// http://math.stackexchange.com/questions/943383/determine-circle-of-intersection-of-plane-and-sphere


#include "geometry_object.hpp"

#include "math/vector3f.hpp"
#include "math/vector4f.hpp"
#include "gl/dual_paraboloid_map.hpp"

#include "ewa/gl/vbo.hpp"
#include "ewa/gl/shader_program.hpp"
#include "ewa/gl/texture2d.hpp"
#include "ewa/gl/array_texture.hpp"
#include "ewa/gl/cube_map_texture.hpp"

#include "ewa/bt_util.hpp"

#include "ewa/common.hpp"
#include "ewa/view_frustum.hpp"
#include "ewa/icamera.hpp"
#include "ewa/file.hpp"
#include "ewa/cube.hpp"
#include "eob_file.hpp"
#include "resource_manager.hpp"
#include "config.hpp"
#include "physics_world.hpp"
#include "gl/depth_fbo.hpp"
#include "ewa/gl/color_fbo.hpp"
#include "ewa/gl/color_depth_fbo.hpp"

#include <btBulletDynamicsCommon.h>

#include <map>

using std::string;
using std::vector;
using std::map;

struct Chunk {
    VBO* m_indexBuffer;
    GLuint m_numTriangles;

    float m_shininess;
    Vector3f m_specularColor;
    Vector3f m_diffuseColor;

    // the material.
    GLint m_texture;
    GLint m_normalMap;
    GLint m_specularMap;

    bool m_hasHeightMap;
};

// contains the info needed to render a batch of GeoObjs
struct GeoObjBatch{
public:

    VBO* m_vertexBuffer;

    std::vector<Chunk*> m_chunks;

    GeometryObjectData* m_data;

    ShaderProgram* m_defaultShader;
    ShaderProgram* m_depthShader; //outputs only the depth. Used for shadow mapping.



    // the geoObjs that needs the info of this struct to be rendered.
    vector<GeometryObject*> m_geoObjs;

    GeoObjBatch() {
	m_defaultShader = NULL;
    }

};

class GeoObjManager {

private:




    GeoObjManager() {



	m_outputIdShader = ShaderProgram::Load("shader/geo_obj_output_id");

	m_outlineShader = ShaderProgram::Load("shader/geo_obj_draw_outline");

	m_outputDepthShader = ShaderProgram::Load("shader/geo_obj_output_depth");

	// used for env mapping.

	{
	    string shaderName = "shader/geo_obj_render";

	    vector<string> defines;

	    defines.push_back("DIFF_MAPPING");
	    defines.push_back("ALPHA_MAPPING");
	    defines.push_back("PARABOLOID");

	    m_envShader = ResourceManager::LoadShader(
		shaderName + "_vs.glsl", shaderName + "_fs.glsl", defines);
	}


	{
	    string shaderName = "shader/geo_obj_render";

	    vector<string> defines;

	    defines.push_back("DIFF_MAPPING");
	    defines.push_back("ALPHA_MAPPING");
	    defines.push_back("REFLECT");

	    m_reflectionShader = ResourceManager::LoadShader(
		shaderName + "_vs.glsl", shaderName + "_fs.glsl", defines);
	}


	{
	    string shaderName = "shader/geo_obj_render";

	    vector<string> defines;

	    defines.push_back("DIFF_MAPPING");
	    defines.push_back("ALPHA_MAPPING");

	}

	m_aabbWireframe = Cube::Load();


	/*
	  Find all PNG files in the obj/ directory, and then load them all into a texture array.
	*/

        vector<string> files = File::EnumerateDirectory("obj");

	if(files.size() == 0) {
	    // try again!

	    files = File::EnumerateDirectory("../game/obj");

	}

        vector<string> pngFiles;

	for(const string& file: files) {

	    if(file.size() > 4) { // must be long enough to fit a ".png extension.

		if(file.substr(file.size()-3).c_str() == string("png") ) {

		    string fullpathFile = File::AppendPaths("obj", file);

		    LOG_I("res: %s", fullpathFile.c_str());

		    pngFiles.push_back(fullpathFile);


		}
	    }
	}

	m_arrayTexture = ArrayTexture::Load(pngFiles);

	m_arrayTexture->Bind();
	m_arrayTexture->SetTextureRepeat();
	m_arrayTexture->GenerateMipmap();
	m_arrayTexture->SetMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	m_arrayTexture->SetMagFilter(GL_LINEAR);
	m_arrayTexture->Unbind();


	if(!m_arrayTexture) {
	    PrintErrorExit();
	}

	m_dudvMap =
	    m_arrayTexture->GetTexture(File::AppendPaths("obj/", "waterDUDV.png"));

	m_normalMap =
	    m_arrayTexture->GetTexture(File::AppendPaths("obj/", "waterNormalMap.png"));
    }

public:
    Cube* m_aabbWireframe;

    map<string, GeoObjBatch*> m_batches;

    vector<GeometryObject*> m_torches;

    ShaderProgram* m_outputIdShader;

    ShaderProgram* m_outlineShader;

    ShaderProgram* m_outputDepthShader;

    ShaderProgram* m_envShader;
    ShaderProgram* m_reflectionShader;

    ArrayTexture* m_arrayTexture;

    GeometryObjectListener* m_listener;


    GLint m_normalMap;
    GLint m_dudvMap;

    float m_totalDelta;

    static GeoObjManager& GetInstance(){
	static GeoObjManager instance;

	return instance;
    }

    GeometryObjectData* LoadObj(const std::string& filename, GeometryObject* geoObj) {

	// check if already a batch for this object has been created.
	map<string, GeoObjBatch*>::iterator it = m_batches.find(filename);
	if(it != m_batches.end() ) {

	    // add it as an object to be batched
	    it->second->m_geoObjs.push_back(geoObj);



	    return it->second->m_data;
	}

	// else, we load the object and create a batch for that object.
	GeoObjBatch* geoObjBatch = new GeoObjBatch();
	GeometryObjectData* data = EobFile::Read(filename);
	geoObjBatch->m_data = data;


	m_batches[filename] = geoObjBatch;
	geoObjBatch->m_geoObjs.push_back(geoObj);



	if(!data) {
	    return NULL;
	}

	geoObjBatch->m_vertexBuffer = VBO::CreateInterleaved(
	    data->m_vertexAttribsSizes);
	geoObjBatch->m_vertexBuffer->Bind();
	geoObjBatch->m_vertexBuffer->SetBufferData(data->m_verticesSize, data->m_vertices);
	geoObjBatch->m_vertexBuffer->Unbind();

	bool isAo = false;

	if(data->m_vertexAttribsSizes[0] == 4 ) {
	    isAo = true;
	}



	vector<string> defaultDefines;

	if(isAo)
	    defaultDefines.push_back("AO");




	/*
	  Next, we create VBOs from the vertex data in the chunks.
	*/
	for(size_t i = 0; i < data->m_chunks.size(); ++i) {

	    GeometryObjectData::Chunk* baseChunk = data->m_chunks[i];
	    Chunk* newChunk = new Chunk;

	    /*
	      Load the textures of the object.
	    */

	    string basePath = File::GetFilePath(filename);

	    newChunk->m_hasHeightMap = false;

	    newChunk->m_texture = -1;
	    newChunk->m_normalMap = -1;
	    newChunk->m_specularMap = -1;


	    Material* mat = data->m_chunks[i]->m_material;

	    if(mat->m_textureFilename != ""){ // empty textures should remain empty.

		newChunk->m_texture =
		    m_arrayTexture->GetTexture(File::AppendPaths(basePath, mat->m_textureFilename));


	    } else {
		newChunk->m_texture = -1;
	    }

	    if(mat->m_normalMapFilename != ""){ // empty textures should remain empty->


		newChunk->m_normalMap =
		    m_arrayTexture->GetTexture(File::AppendPaths(basePath, mat->m_normalMapFilename));


		newChunk->m_hasHeightMap = mat->m_hasHeightMap;
	    }

	    if(mat->m_specularMapFilename != ""){ // empty textures should remain empty->



		newChunk->m_specularMap =
		    m_arrayTexture->GetTexture(File::AppendPaths(basePath, mat->m_specularMapFilename));
	    }
	    newChunk->m_indexBuffer = VBO::CreateIndex(data->m_indexType);


	    newChunk->m_numTriangles = baseChunk->m_numTriangles;


	    newChunk->m_indexBuffer->Bind();
	    newChunk->m_indexBuffer->SetBufferData(baseChunk->m_indicesSize, baseChunk->m_indices);
	    newChunk->m_indexBuffer->Unbind();

	    newChunk->m_shininess = baseChunk->m_material->m_specularExponent;

	    float EPS = 0.0001f;
	    if(newChunk->m_shininess < EPS) {
		newChunk->m_shininess = EPS;
	    }

	    newChunk->m_specularColor = baseChunk->m_material->m_specularColor;
	    newChunk->m_diffuseColor = baseChunk->m_material->m_diffuseColor;

	    string shaderName = "shader/geo_obj_render";

	    if(geoObjBatch->m_defaultShader == NULL) {

		if(filename == "obj/car_blend.eob") {

		    vector<string> defines(defaultDefines);

		    defines.push_back("ALPHA_MAPPING");
		    defines.push_back("ENV_MAPPING");
		    defines.push_back("SPECULAR_LIGHT");
		    defines.push_back("DIFFUSE_LIGHT");
		    defines.push_back("FRESNEL");
		    defines.push_back("DEFERRED");


		    geoObjBatch->m_defaultShader = ResourceManager::LoadShader(
			shaderName + "_vs.glsl", shaderName + "_fs.glsl", defines);

		}else if(filename == "obj/water.eob") {


		    vector<string> defines(defaultDefines);

		    defines.push_back("ALPHA_MAPPING");
		    defines.push_back("ENV_MAPPING");
		    defines.push_back("SPECULAR_LIGHT");
		    defines.push_back("DIFFUSE_LIGHT");
		    defines.push_back("FRESNEL");
		    defines.push_back("DEFERRED");


		    string shaderName = "shader/water_render";


		    geoObjBatch->m_defaultShader = ResourceManager::LoadShader(
			shaderName + "_vs.glsl", shaderName + "_fs.glsl", defines);


		}else {

		    /*
		      Next, we create a shader that supports all the texture types.
		    */

		    vector<string> defines(defaultDefines);

		    defines.push_back("SHADOW_MAPPING");
		    defines.push_back("DIFF_MAPPING");
		    defines.push_back("ALPHA_MAPPING");
		    defines.push_back("SPECULAR_LIGHT");
		    defines.push_back("DIFFUSE_LIGHT");
		    defines.push_back("DEFERRED");



		    if(newChunk->m_specularMap != -1) {
			defines.push_back("SPEC_MAPPING");
		    }

		    if(newChunk->m_hasHeightMap) {
			defines.push_back("HEIGHT_MAPPING");
		    } else if(newChunk->m_normalMap != -1) { // only a normal map, no height map.
			defines.push_back("NORMAL_MAPPING");
		    }

		    geoObjBatch->m_defaultShader = ResourceManager::LoadShader(shaderName + "_vs.glsl", shaderName + "_fs.glsl", defines);

		}

	    }

	    geoObjBatch->m_chunks.push_back(newChunk);
	}	return data;
    }

};


Matrix4f fromBtMat(const btMatrix3x3& m) {

    return Matrix4f(
	m[0].x(),m[0].y(),m[0].z(),0,
	m[1].x(),m[1].y(),m[1].z(),0,
	m[2].x(),m[2].y(),m[2].z(),0,
	0            , 0           , 0           ,1
	);
}



ATTRIBUTE_ALIGNED16(class) MyMotionState : public btMotionState
{
protected:
    GeometryObject* m_obj;
    btTransform mInitialPosition;

public:

    BT_DECLARE_ALIGNED_ALLOCATOR();

    MyMotionState(const btTransform& initialPosition, GeometryObject *obj)
    {
        m_obj = obj;
	mInitialPosition = initialPosition;
    }

    virtual ~MyMotionState()
    {
    }

    virtual void getWorldTransform(btTransform &worldTrans) const
    {
        worldTrans = mInitialPosition;
    }

    virtual void setWorldTransform(const btTransform &worldTrans)
    {
        if(m_obj == NULL)
            return; // silently return before we set a node

        btVector3 pos = worldTrans.getOrigin();
        m_obj->SetPosition(Vector3f(
			       pos.x() / WORLD_SCALE,
			       pos.y() / WORLD_SCALE,
			       pos.z() / WORLD_SCALE) );

	// TODO: also scale the rotation?
	m_obj->SetRotation(/*rot*/ worldTrans.getRotation() );
    }
};

bool GeometryObject::Init(
    const std::string& filename,
    const Vector3f& position,
    const btQuaternion& rotation,
    float scale,
    unsigned int id,

    short physicsGroup,
    short physicsMask    ) {

    /*
     if filename is torch, add position to light list.
     */
    if(filename == "obj/torch.eob" ) {
	GeoObjManager::GetInstance().m_torches.push_back(this);

	if(GeoObjManager::GetInstance().m_listener != NULL)
	    GeoObjManager::GetInstance().m_listener->LightUpdate();

    }


    m_physicsGroup = physicsGroup;
    m_physicsMask = physicsMask;

    m_id = id;
    m_filename = filename;

    m_inCameraFrustum = true;
    m_inLightFrustum = true;
    m_inReflectionFrustum = true;
    SetSelected(false);

    for(int i = 0; i < 2; ++i) {
	m_inEnvLightFrustums[i] = true;
    }

    SetPosition(position);
    SetEditPosition( Vector3f(0.0f) );

    SetRotation( rotation  );
    SetEditRotation( btQuaternion::getIdentity() );

    SetScale(scale);
    SetEditScale( 1.0f );


    GeometryObjectData* data = GeoObjManager::GetInstance().LoadObj(filename, this);



    if(!data) {
	return false;
    }

    /*
      Bounding Volume
    */
    m_aabb = data->aabb;


    m_data = data;

    return true;
}

GeometryObject::~GeometryObject() {
/*
  for(size_t i = 0; i < m_chunks.size(); ++i) {
  Chunk* chunk = m_chunks[i];

  MY_DELETE(chunk->m_vertexBuffer);
  MY_DELETE(chunk->m_indexBuffer);

  MY_DELETE(chunk);
  }*/
    // should instead do this in GeoObjManager.
}

void GeometryObject::RenderShadowMapAll(const Matrix4f& lightVp) {

    auto& batches = GeoObjManager::GetInstance().m_batches;

    ShaderProgram* outputDepthShader = GeoObjManager::GetInstance().m_outputDepthShader;

    // bind shader of all the batches.
    outputDepthShader->Bind();

    // render all batches, one after one.
    for(auto& itBatch : batches) {

	const GeoObjBatch* batch = itBatch.second;

	batch->m_vertexBuffer->EnableVertexAttribInterleavedWithBind();

	outputDepthShader->SetUniform("textureArray", 0);
	Texture::SetActiveTextureUnit(0);
	GeoObjManager::GetInstance().m_arrayTexture->Bind();


	// render the objects of the batch, one after one.
	for(GeometryObject* geoObj : batch->m_geoObjs ) {

	    if(!geoObj->m_inLightFrustum) {
		continue; // if culled, do nothing.
	    }

	    Matrix4f modelMatrix = geoObj->GetModelMatrix( );

	    const Matrix4f mvp = lightVp * modelMatrix;
	    outputDepthShader->SetUniform("mvp", mvp  );

	    for(size_t i = 0; i < batch->m_chunks.size(); ++i) {

		Chunk* chunk = batch->m_chunks[i];


		outputDepthShader->SetUniform("diffMap", (float)chunk->m_texture  );


		chunk->m_indexBuffer->Bind();
		chunk->m_indexBuffer->DrawIndices(GL_TRIANGLES, (chunk->m_numTriangles)*3);
		chunk->m_indexBuffer->Unbind();

	    }
	}

	batch->m_vertexBuffer->DisableVertexAttribInterleavedWithBind();


    }

    outputDepthShader->Unbind();

/*
  m_depthShader->Bind();

  Matrix4f modelMatrix = GetModelMatrix();

  const Matrix4f mvp = lightVp * modelMatrix;
  m_depthShader->SetUniform("mvp", mvp  );

  for(size_t i = 0; i < m_chunks.size(); ++i) {
  Chunk* chunk = m_chunks[i];

  VBO::DrawIndices(*chunk->m_vertexBuffer, *chunk->m_indexBuffer, GL_TRIANGLES, (chunk->m_numTriangles)*3);
  }

  m_depthShader->Unbind();
*/
}


void GeometryObject::RenderIdAll(
    const ICamera* camera) {

    map<string, GeoObjBatch*>& batches = GeoObjManager::GetInstance().m_batches;

    ShaderProgram* outputIdShader = GeoObjManager::GetInstance().m_outputIdShader;

    // bind shader of all the batches.
    outputIdShader->Bind();

    // render all batches, one after one.
    for(auto& itBatch : batches) {

	const GeoObjBatch* batch = itBatch.second;

	batch->m_vertexBuffer->EnableVertexAttribInterleavedWithBind();


	// render the objects of the batch, one after one.
	for(GeometryObject* geoObj : batch->m_geoObjs ) {

	    if(!geoObj->m_inCameraFrustum) {
		continue; // if culled, do nothing.
	    }

	    Matrix4f modelMatrix = geoObj->GetModelMatrix( );

	    const Matrix4f mvp = camera->GetVp() * modelMatrix;
	    outputIdShader->SetUniform("mvp", mvp  );
	    outputIdShader->SetUniform("id", (float)geoObj->m_id  );

	    for(size_t i = 0; i < batch->m_chunks.size(); ++i) {

		Chunk* chunk = batch->m_chunks[i];

		chunk->m_indexBuffer->Bind();
		chunk->m_indexBuffer->DrawIndices(GL_TRIANGLES, (chunk->m_numTriangles)*3);
		chunk->m_indexBuffer->Unbind();

	    }

	}

	batch->m_vertexBuffer->DisableVertexAttribInterleavedWithBind();


    }

    outputIdShader->Unbind();

}








void GeometryObject::RenderAllEnv(const Vector4f& lightPosition, int i, Paraboloid& paraboloid){

    int total = 0;
    int nonCulled = 0;


    auto& batches = GeoObjManager::GetInstance().m_batches;

    ShaderProgram* envShader = GeoObjManager::GetInstance().m_envShader;


    // render all batches, one after one.
    for(auto& itBatch : batches) {

	const GeoObjBatch* batch = itBatch.second;

	// bind shader of the batch.
	envShader->Bind();

	batch->m_vertexBuffer->EnableVertexAttribInterleavedWithBind();

	envShader->SetUniform("textureArray", 0);
	Texture::SetActiveTextureUnit(0);
	GeoObjManager::GetInstance().m_arrayTexture->Bind();

	// render the objects of the batch, one after one.
	for(GeometryObject* geoObj : batch->m_geoObjs ) {

	    ++total;

	    if(!geoObj->m_inEnvLightFrustums[i]) { // not in frustum?
		continue; // if culled, do nothing.
	    }

	    ++nonCulled;

	    Matrix4f modelMatrix = geoObj->GetModelMatrix();

	    /*
	    envShader->SetPhongUniforms(
		modelMatrix
		, camera, lightPosition);
	    */

	    paraboloid.SetParaboloidUniforms(
		*envShader,
//		Matrix4f::CreateTranslation(0,0,0),
		modelMatrix,

		paraboloid.m_viewMatrix,
		Matrix4f::CreateIdentity(),
		paraboloid.m_position,
		lightPosition);


	    GL_C(glEnable(GL_CLIP_DISTANCE0));



	    for(size_t i = 0; i < batch->m_chunks.size(); ++i) {

		Chunk* chunk = batch->m_chunks[i];

		if(chunk->m_specularMap == -1) {
		    // if no spec map, the chunk has the same specular color all over the texture.
		    envShader->SetUniform("specColor", chunk->m_specularColor);
		}

		envShader->SetUniform("specShiny", chunk->m_shininess);

		if(chunk->m_texture != -1) {
		    envShader->SetUniform("diffMap", (float)chunk->m_texture  );
		}

		chunk->m_indexBuffer->Bind();
		chunk->m_indexBuffer->DrawIndices(GL_TRIANGLES, (chunk->m_numTriangles)*3);
		chunk->m_indexBuffer->Unbind();
	    }

	    GL_C(glDisable(GL_CLIP_DISTANCE0));

	}

	batch->m_vertexBuffer->DisableVertexAttribInterleavedWithBind();

	GeoObjManager::GetInstance().m_arrayTexture->Unbind();

    }
}




void GeometryObject::RenderReflection(
    ICamera* camera, const Vector4f& lightPosition){

    auto& batches = GeoObjManager::GetInstance().m_batches;

    ShaderProgram* envShader = GeoObjManager::GetInstance().m_reflectionShader;


    // render all batches, one after one.
    for(auto& itBatch : batches) {

	const GeoObjBatch* batch = itBatch.second;

	// bind shader of the batch.
	envShader->Bind();

	batch->m_vertexBuffer->EnableVertexAttribInterleavedWithBind();

	envShader->SetUniform("textureArray", 0);
	Texture::SetActiveTextureUnit(0);
	GeoObjManager::GetInstance().m_arrayTexture->Bind();

	// render the objects of the batch, one after one.
	for(GeometryObject* geoObj : batch->m_geoObjs ) {

	    if(!geoObj->m_inReflectionFrustum) { // not in frustum?
		continue; // if culled, do nothing.
	    }

	    Matrix4f modelMatrix = geoObj->GetModelMatrix();

	    envShader->SetPhongUniforms(
		modelMatrix
		, camera, lightPosition);

	    for(size_t i = 0; i < batch->m_chunks.size(); ++i) {

		Chunk* chunk = batch->m_chunks[i];

		if(chunk->m_specularMap == -1) {
		    // if no spec map, the chunk has the same specular color all over the texture.
		    envShader->SetUniform("specColor", chunk->m_specularColor);
		}

		envShader->SetUniform("specShiny", chunk->m_shininess);

		if(chunk->m_texture != -1) {
		    envShader->SetUniform("diffMap", (float)chunk->m_texture  );
		}

		chunk->m_indexBuffer->Bind();
		chunk->m_indexBuffer->DrawIndices(GL_TRIANGLES, (chunk->m_numTriangles)*3);
		chunk->m_indexBuffer->Unbind();
	    }
	}

	batch->m_vertexBuffer->DisableVertexAttribInterleavedWithBind();

	GeoObjManager::GetInstance().m_arrayTexture->Unbind();

    }
}



void GeometryObject::RenderAll(const ICamera* camera, const Vector4f& lightPosition, const Matrix4f& lightVp, const DepthFBO& shadowMap, CubeMapTexture* cubeMapTexture, ColorDepthFbo& refractionMap, const ColorFBO& reflectionMap) {

    int total = 0;
    int nonCulled = 0;

    auto& batches = GeoObjManager::GetInstance().m_batches;

    GeometryObject* selectedObj = NULL;
    const GeoObjBatch* selectedBatch = NULL;

    // render all batches, one after one.
    for(auto& itBatch : batches) {

	const GeoObjBatch* batch = itBatch.second;



	// bind shader of the batch.
	batch->m_defaultShader->Bind();

	batch->m_vertexBuffer->EnableVertexAttribInterleavedWithBind();

	batch->m_defaultShader->SetUniform("textureArray", 0);
	Texture::SetActiveTextureUnit(0);
	GeoObjManager::GetInstance().m_arrayTexture->Bind();


	batch->m_defaultShader->SetUniform("shadowMap", (int)shadowMap.GetTargetTextureUnit() );
	Texture::SetActiveTextureUnit(shadowMap.GetTargetTextureUnit());
	shadowMap.GetRenderTargetTexture().Bind();

/*
  if(batch->m_hasHeightMap) {
  Config& config = Config::GetInstance();

  batch->m_defaultShader->SetUniform("zNear", config.GetZNear());
  batch->m_defaultShader->SetUniform("zFar", config.GetZFar());
  }
*/

	// render the objects of the batch, one after one.
	for(GeometryObject* geoObj : batch->m_geoObjs ) {

	    ++total;

	    if(!geoObj->m_inCameraFrustum) {
		continue; // if culled, do nothing.
	    }



	    ++nonCulled;

	    Matrix4f modelMatrix = geoObj->GetModelMatrix();

	    batch->m_defaultShader->SetPhongUniforms(
		modelMatrix
		, camera, lightPosition,
		lightVp);

	    if(geoObj->IsSelected() ) {
		// for the selected object, we also render to the stencil buffer
		// so that we later can draw an outline.


		GL_C(glEnable(GL_STENCIL_TEST));
		GL_C(glStencilFunc(GL_ALWAYS,1,1));
		GL_C(glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE));
		GL_C(glStencilMask(1));
		GL_C(glClearStencil(0));
		GL_C(glClear(GL_STENCIL_BUFFER_BIT));

		selectedObj = geoObj;
		selectedBatch = batch;
	    }


	    if(geoObj->GetFilename() == "obj/car_blend.eob") {
		batch->m_defaultShader->SetUniform("envMap", 7);
		Texture::SetActiveTextureUnit(7);
		cubeMapTexture->Bind();

		batch->m_defaultShader->SetUniform("inverseViewNormalMatrix",
						   camera->GetViewMatrix().Transpose()  );


		batch->m_defaultShader->SetUniform("id", 1.0f);

	    } else if(geoObj->GetFilename() == "obj/water.eob") {

		batch->m_defaultShader->SetUniform("refractionMap", 8);
		Texture::SetActiveTextureUnit(8);
		refractionMap.GetColorTexture()->Bind();

		batch->m_defaultShader->SetUniform("reflectionMap", 9);
		Texture::SetActiveTextureUnit(9);
		reflectionMap.GetRenderTargetTexture().Bind();


		batch->m_defaultShader->SetUniform("depthMap", 10);
		Texture::SetActiveTextureUnit(10);
		refractionMap.GetDepthTexture()->Bind();


		batch->m_defaultShader->SetUniform("dudvMap", (float)GeoObjManager::GetInstance().m_dudvMap);
		batch->m_defaultShader->SetUniform("normalMap", (float)GeoObjManager::GetInstance().m_normalMap);


		batch->m_defaultShader->SetUniform("totalDelta", (float)GeoObjManager::GetInstance().m_totalDelta);

		// remove this for the time being.
/*
		GL_C(glEnable(GL_BLEND));
		GL_C(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
*/

	    } else {
		batch->m_defaultShader->SetUniform("id", 0.0f);
	    }



	    for(size_t i = 0; i < batch->m_chunks.size(); ++i) {

		Chunk* chunk = batch->m_chunks[i];

		if(chunk->m_specularMap == -1) {
		    // if no spec map, the chunk has the same specular color all over the texture.
		    batch->m_defaultShader->SetUniform("specColor", chunk->m_specularColor);
		}

		batch->m_defaultShader->SetUniform("specShiny", chunk->m_shininess);

		if(chunk->m_texture != -1) {
		    batch->m_defaultShader->SetUniform("diffMap", (float)chunk->m_texture  );
		}else {
		    batch->m_defaultShader->SetUniform("diffColor", chunk->m_diffuseColor);
		}

		if(chunk->m_normalMap != -1) {
		    batch->m_defaultShader->SetUniform("normalMap", (float)chunk->m_normalMap  );
		}

		if(chunk->m_specularMap != -1) {
		    batch->m_defaultShader->SetUniform("specMap", (float)chunk->m_specularMap  );
		}

		chunk->m_indexBuffer->Bind();
		chunk->m_indexBuffer->DrawIndices(GL_TRIANGLES, (chunk->m_numTriangles)*3);
		chunk->m_indexBuffer->Unbind();
	    }


	    if(geoObj->GetFilename() == "obj/car_blend.eob") {
		cubeMapTexture->Unbind();
	    }else if(geoObj->GetFilename() == "obj/water.eob") {

		GL_C(glDisable(GL_BLEND));
		refractionMap.GetColorTexture()->Unbind();

		refractionMap.GetDepthTexture()->Unbind();
	    }

	    if(geoObj->IsSelected() ) {

		// dont render to stencil buffer for remaining objects.
		GL_C(glStencilMask(0));
	    }
	}

	batch->m_vertexBuffer->DisableVertexAttribInterleavedWithBind();

	shadowMap.GetRenderTargetTexture().Unbind();
	batch->m_defaultShader->Unbind();

	GeoObjManager::GetInstance().m_arrayTexture->Unbind();

    }

    if(selectedObj != NULL) {

	// also render outline for selected object.
	GL_C(glStencilFunc(GL_EQUAL,0,1));
	GL_C(glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP));
	GL_C(glStencilMask(0x00));


	ShaderProgram* outlineShader = GeoObjManager::GetInstance().m_outlineShader;

	outlineShader->Bind();

	Matrix4f modelMatrix = selectedObj->GetModelMatrix( Matrix4f::CreateScale(Vector3f(1.05)) );

	const Matrix4f mvp = camera->GetVp() * modelMatrix;
	outlineShader->SetUniform("mvp", mvp  );

	for(size_t i = 0; i < selectedBatch->m_chunks.size(); ++i) {
	    Chunk* chunk = selectedBatch->m_chunks[i];

	    VBO::DrawIndices(*selectedBatch->m_vertexBuffer, *chunk->m_indexBuffer, GL_TRIANGLES, (chunk->m_numTriangles)*3);
	}

	outlineShader->Unbind();

	GL_C(glDisable(GL_STENCIL_TEST));
    }

    /*
      Render the AABBs of all objects
    */
/*
    // render all batches, one after one.
    for(auto& itBatch : batches) {

	const GeoObjBatch* batch = itBatch.second;

	// render the objects of the batch, one after one.
	for(GeometryObject* geoObj : batch->m_geoObjs ) {

	    Vector3f center = (geoObj->m_aabb.min + geoObj->m_aabb.max) * 0.5f;
	    Vector3f radius = geoObj->m_aabb.max - center;
	    Matrix4f modelMatrix = geoObj->GetModelMatrix();

	    Cube* aabbWireframe = GeoObjManager::GetInstance().m_aabbWireframe;

	    aabbWireframe->SetModelMatrix(
		modelMatrix *
		Matrix4f::CreateTranslation(center) *
		Matrix4f::CreateScale(radius)
		);


	    aabbWireframe->Render(camera->GetVp());

	}
    }
*/
}

AABB GeometryObject::GetModelSpaceAABB()const {

    AABB temp;

    Matrix4f modelMatrix = GetModelMatrix();

    temp.m_min = Vector3f((modelMatrix * Vector4f(m_aabb.m_min, 1.0f)));
    temp.m_max = Vector3f((modelMatrix * Vector4f(m_aabb.m_max, 1.0f)));

    return temp;
}

void GeometryObject::SetPosition(const Vector3f& position) {
    this->m_position = position;
}

void GeometryObject::SetRotation(const btQuaternion& rotation) {

    m_rotation = rotation;
}

/*
  void GeometryObject::ApplyCentralForce(const Vector3f& force) {
  if(m_rigidBody)
  m_rigidBody->applyCentralForce(toBtVec(force));
  }

  void GeometryObject::ApplyForce(const Vector3f& force, const Vector3f& relPos) {

  if(m_rigidBody)
  m_rigidBody->applyForce(toBtVec(force), toBtVec(relPos) );
  }
*/

btRigidBody* GeometryObject::GetRigidBody() const {
    return m_rigidBody;
}


btMotionState* GeometryObject::GetMotionState() const {
    return m_motionState;
}

Vector3f GeometryObject::GetPosition() const {
    return m_position;
}

void GeometryObject::AddToPhysicsWorld(PhysicsWorld* physicsWorld) {

    CollisionShape* colShape = m_data->m_collisionShape;
    EntityInfo* entityInfo = m_data->m_entityInfo;

    if(!colShape)
	return; // do nothing for nothing.

    btCollisionShape* btShape = NULL;

    /*
      create collison shape.
    */
    if(colShape->m_shape == BoxShape) {
	btShape = new btBoxShape(toBtVec(colShape->m_halfExtents) );
    } else if(colShape->m_shape == SphereShape) {
	btShape = new btSphereShape(colShape->m_radius);
    } else if(colShape->m_shape == CylinderShape) {
	btShape = new btCylinderShape(toBtVec(colShape->m_halfExtents) );
    } else {

	LOG_E("undefined collision shape: %d", colShape->m_shape );

	return;
    }

    // set scaling.
    LOG_I("scale: %f", m_scale );
    btShape->setLocalScaling(
	btVector3(m_scale, m_scale, m_scale) * WORLD_SCALE
	);

    /*
      Create motion state
    */
    btTransform transform(m_rotation* toBtQuat(colShape->m_rotate),
			  toBtVec(
			      (m_position + colShape->m_origin) * WORLD_SCALE

			      ));
    m_motionState = new MyMotionState(transform, this);

    btVector3 inertia(0, 0, 0);
    // static objects dont move, so they have no intertia
    if(!entityInfo->m_isStatic) {
        btShape->calculateLocalInertia(entityInfo->m_mass, inertia);
    }

    btRigidBody::btRigidBodyConstructionInfo ci(entityInfo->m_mass, m_motionState, btShape, inertia);


    if(entityInfo->m_isStatic) {
	ci.m_friction=2.0f;
    }

    m_rigidBody = new btRigidBody(ci);

    if(!entityInfo->m_isStatic) {
	m_rigidBody->setCollisionFlags(btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

    }

    physicsWorld->AddRigidBody(m_rigidBody, m_physicsGroup, m_physicsMask);
}

Matrix4f GeometryObject::GetModelMatrix(const Matrix4f& scaling)const {
    return
	Matrix4f::CreateTranslation(m_position + m_editPosition) * // translate
	fromBtMat( btMatrix3x3(m_rotation * m_editRotation) ) * // rotate

    	scaling *

	Matrix4f::CreateScale(m_scale * m_editScale)
	;
}

void GeometryObject::SetEditPosition(const Vector3f& editPosition) {
    m_editPosition = editPosition;
}

void GeometryObject::SetEditRotation(const btQuaternion& editRotation) {
    m_editRotation = editRotation;
}

btQuaternion GeometryObject::GetRotation() const {
    return m_rotation;
}

std::string GeometryObject::GetFilename() const {
    return m_filename;
}

bool GeometryObject::IsSelected()const {
    return m_selected;
}

void GeometryObject::SetSelected(bool selected) {
    m_selected = selected;
}

void GeometryObject::Update(const ViewFrustum* cameraFrustum, const ViewFrustum* lightFrustum,
			    DualParaboloidMap& dualParaboloidMap, const ViewFrustum* reflectionFrustum) {
    m_inCameraFrustum = cameraFrustum->IsAABBInFrustum(GetModelSpaceAABB());



    if(m_filename == "obj/water.eob" ) {
	m_inLightFrustum = false;
    } else {
    m_inLightFrustum = lightFrustum->IsAABBInFrustum(GetModelSpaceAABB());

    }
    m_inReflectionFrustum = reflectionFrustum->IsAABBInFrustum(GetModelSpaceAABB());


/*
    if(m_filename == "obj/car_blend.eob" ) {
	m_inReflectionFrustum = false;
	m_inLightFrustum = false;
	m_inCameraFrustum = false;
    }*/

/*
    if(m_filename != "obj/water.eob" && m_filename != "obj/car_blend.eob" ) {
	m_inReflectionFrustum = false;
	m_inLightFrustum = false;
	m_inCameraFrustum = false;
    }
*/

    for(int i = 0; i < 2; ++i) {

	if(m_filename == "obj/car_blend.eob" ) {
	    // car is never in environment map.
	    m_inEnvLightFrustums[i] = false;
	} else {
	    m_inEnvLightFrustums[i] =
		dualParaboloidMap.GetParaboloid(i).InFrustum(GetModelSpaceAABB());
	}


/*
	if(m_filename != "obj/water.eob" && m_filename != "obj/car_blend.eob" ) {
	    m_inEnvLightFrustums[i] = false;
	}
*/
    }



}

IGeometryObject* GeometryObject::Duplicate(unsigned int id) {
    GeometryObject* newObj = new GeometryObject();

    newObj->Init(m_filename, m_position, m_rotation, m_scale, id, m_physicsGroup, m_physicsMask);

    return newObj;
}


unsigned int GeometryObject::GetId() {
    return m_id;
}

void GeometryObject::Delete(IGeometryObject* geoObj) {

    /*
      if torch, remove form light list.
     */
    if(geoObj->GetFilename() == "obj/torch.eob" ) {
	vector<GeometryObject*>::iterator it;
	for(it = GeoObjManager::GetInstance().m_torches.begin();

	    it != GeoObjManager::GetInstance().m_torches.end();
	    ++it
	    ) {

	    if(*it == geoObj ) {
		break;
	    }

	}

	GeoObjManager::GetInstance().m_torches.erase(it);

	if(GeoObjManager::GetInstance().m_listener != NULL)
	    GeoObjManager::GetInstance().m_listener->LightUpdate();
    }

    // remove the obj from its corresponding batch:

    // find the batch:x
    GeoObjBatch* batch = GeoObjManager::GetInstance().m_batches[geoObj->GetFilename()];


    vector<GeometryObject*>::iterator it;

    for(it = batch->m_geoObjs.begin(); it !=batch->m_geoObjs.end(); ++it ) {

	if(*it == geoObj) {
	    break;
	}
    }

    batch->m_geoObjs.erase(it);

}


float GeometryObject::GetScale() const {
    return m_scale;
}

void GeometryObject::SetScale(const float scale) {
    m_scale = scale;
}

void GeometryObject::SetEditScale(const float editScale) {
    m_editScale = editScale;
}


//how many rocks are we drawing per iteration? i think we are drawing them twice!!


void GeometryObject::SetTotalDelta(float totalDelta) {
    GeoObjManager::GetInstance().m_totalDelta = totalDelta;
}


std::vector<Vector3f> GeometryObject::GetTorches() {

    vector<Vector3f> torches;

    for(GeometryObject* torch : GeoObjManager::GetInstance().m_torches) {
	torches.push_back(  Vector3f(torch->GetModelMatrix(  ) * Vector4f(0,2.05,0,1)) );
    }

    return torches;
}


void GeometryObject::SetListener(GeometryObjectListener* listener) {

    GeoObjManager::GetInstance().m_listener = listener;
}
