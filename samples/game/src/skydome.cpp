#include "skydome.hpp"

#include "ewa/gl/vbo.hpp"
#include "ewa/gl/texture2d.hpp"
#include "ewa/gl/shader_program.hpp"

#include "ewa/math/vector2f.hpp"
#include "ewa/math/vector3f.hpp"
#include "ewa/math/vector4f.hpp"
#include "ewa/math/matrix4f.hpp"

#include "ewa/value_noise_seed.hpp"
#include "ewa/camera.hpp"

#include "ewa/common.hpp"
#include "ewa/util.hpp"


using std::vector;

constexpr GLushort NUM_BILLBOARD_TRIANGLES = 2;

struct CloudInfo {
public:

    float m_orientation;
    float m_elevation;
    float m_rotation;

    VBO* m_vertexBuffer;
    VBO* m_indexBuffer;
};

struct CloudGroup{
public:

    Texture* m_cloudTexture;
    vector<CloudInfo> m_clouds;
};


void GenerateBillboardVertices(VBO* m_vertexBuffer, VBO* m_indexBuffer, const double hsize);

Skydome::Skydome(const float radius, const int slices, const int stacks): GeometryObject(Vector3f(0), Vector3f(1)), m_delta(0) {

    /*
      Create the skydome.
    */

    MakeSky(radius, slices, stacks);
    MakeSun();
    MakeClouds();

}

void Skydome::MakeSky(const float radius, const int slices, const int stacks) {
    m_domeShader = new ShaderProgram("shader/skydome");

    m_domeVertexBuffer = VBO::CreateInterleaved(
	vector<GLuint>{
	    VBO_POSITION_ATTRIB_INDEX,
		VBO_TEX_COORD_ATTRIB_INDEX },
	vector<GLuint>{3,3}
	);
    m_domeIndexBuffer = VBO::CreateIndex(GL_UNSIGNED_SHORT);
    m_domeNumTriangles = GenerateSphereVertices(radius, slices, stacks, m_domeVertexBuffer, m_domeIndexBuffer);

}

void Skydome::MakeClouds() {


    CloudGroup* cloudGroup = new CloudGroup();

    Texture* cloudTexture= new Texture2D("img/cloud1.png");
    cloudTexture->Bind();
    cloudTexture->SetTextureClamping();
    cloudTexture->SetMinFilter(GL_LINEAR);
    cloudTexture->SetMagFilter(GL_NEAREST);
    cloudTexture->Unbind();

    cloudGroup->m_cloudTexture= cloudTexture;

    CloudInfo cloudInfo;

    cloudInfo.m_vertexBuffer = VBO::CreateInterleaved(
	vector<GLuint>{
	    VBO_POSITION_ATTRIB_INDEX,
		VBO_TEX_COORD_ATTRIB_INDEX },
	vector<GLuint>{3,3}
	);
    cloudInfo.m_indexBuffer = VBO::CreateIndex(GL_UNSIGNED_SHORT);
    GenerateBillboardVertices(cloudInfo.m_vertexBuffer, cloudInfo.m_indexBuffer, 0.06);

    cloudInfo.m_elevation = 5;
    cloudInfo.m_rotation = 0;
    cloudInfo.m_orientation = 0;

    cloudGroup->m_clouds.push_back(cloudInfo);

    m_clouds.push_back(cloudGroup);

}

void Skydome::MakeSun() {

    m_billboardShader = new ShaderProgram("shader/billboard");

    m_sunVertexBuffer = VBO::CreateInterleaved(
	vector<GLuint>{
	    VBO_POSITION_ATTRIB_INDEX,
		VBO_TEX_COORD_ATTRIB_INDEX },
	vector<GLuint>{3,2}
	);

    m_sunIndexBuffer = VBO::CreateIndex(GL_UNSIGNED_SHORT);

    GenerateBillboardVertices(m_sunVertexBuffer, m_sunIndexBuffer, 0.06);

    m_sunTexture = new Texture2D("img/sun.png");

    m_sunTexture->Bind();
    m_sunTexture->SetTextureClamping();
    m_sunTexture->SetMinFilter(GL_LINEAR);
    m_sunTexture->SetMagFilter(GL_NEAREST);
    m_sunTexture->Unbind();

}

Skydome::~Skydome() {
    MY_DELETE(m_domeShader);
    MY_DELETE(m_domeVertexBuffer);
    MY_DELETE(m_domeIndexBuffer);

    MY_DELETE(m_billboardShader);
    MY_DELETE(m_sunVertexBuffer);
    MY_DELETE(m_sunIndexBuffer);
    MY_DELETE(m_sunTexture);

}

void Skydome::Draw(const Camera& camera) {

    SetDepthTest(false);


    DrawDome(camera);


    // next we'll draw all the billboards:

    m_billboardShader->Bind();
    GL_C(glEnable(GL_BLEND)); // all the billboards use alpha blending.
    GL_C(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    DrawSun(camera);

    DrawClouds(camera);



    // done drawing billboards.
    GL_C(glDisable(GL_BLEND));
    m_billboardShader->Unbind();


    SetDepthTest(true);
}

void Skydome::Update(const float delta) {
    m_delta += delta;
}

void GenerateBillboardVertices(VBO* m_vertexBuffer, VBO* m_indexBuffer, const double hsize) {

    FloatVector vertices;
    UshortVector indices;

    Vector3f sunPoint(0, 0, 1);
    Vector3f xaxis(1, 0, 0);
    Vector3f yaxis(0, 1, 0);

//    double hsize = 0.06; // 0.3


    (sunPoint - hsize * xaxis + hsize * yaxis).Add(vertices);
    Vector2f(0.0f,1.0f).Add(vertices);

    (sunPoint + hsize * xaxis + hsize * yaxis).Add(vertices);
    Vector2f(1.0f,1.0f).Add(vertices);

    (sunPoint - hsize * xaxis - hsize * yaxis).Add(vertices);
    Vector2f(0.0f,0.0f).Add(vertices);

    (sunPoint + hsize * xaxis - hsize * yaxis).Add(vertices);
    Vector2f(1.0f,0.0f).Add(vertices);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    indices.push_back(1);
    indices.push_back(3);
    indices.push_back(2);

    m_vertexBuffer->Bind();
    m_vertexBuffer->SetBufferData(vertices);
    m_vertexBuffer->Unbind();

    m_indexBuffer->Bind();
    m_indexBuffer->SetBufferData(indices);
    m_indexBuffer->Unbind();
}

void Skydome::DrawDome(const Camera& camera) {
    m_domeShader->Bind();

    Matrix4f modelView =  camera.GetViewMatrix();
    modelView.m03 = 0;
    modelView.m13 = 0;
    modelView.m23 = 0;

    Matrix4f mvp = camera.GetProjectionMatrix() * modelView;

    m_domeShader->SetUniform("mvp", mvp);
    m_domeShader->SetUniform("delta", m_delta);

    VBO::DrawIndices(*m_domeVertexBuffer, *m_domeIndexBuffer, GL_TRIANGLES, (m_domeNumTriangles)*3);

    m_domeShader->Unbind();


}

void Skydome::DrawSun(const Camera& camera) {

    m_sunTexture->Bind();

    DrawBillboard(camera, m_sunVertexBuffer, m_sunIndexBuffer, -10.0f,5.0f,0.0f);

    m_sunTexture->Unbind();
}


void Skydome::DrawBillboard(const Camera& camera, VBO* m_vertexBuffer, VBO* m_indexBuffer,
		   const float orientation, const float elevation, const float rotation) {


    const Matrix4f orientationMatrix =Matrix4f::CreateRotate(orientation, Vector3f(0,0,1) );
    const Matrix4f elevationMatrix =Matrix4f::CreateRotate(-elevation, Vector3f(1,0,0) );
    const Matrix4f rotationMatrix = Matrix4f::CreateRotate(rotation, Vector3f(0,1,0) );

    const Matrix4f sunModelView = rotationMatrix * elevationMatrix * orientationMatrix;


    Matrix4f modelView =  camera.GetModelViewMatrix(sunModelView);
    modelView.m03 = 0;
    modelView.m13 = 0;
    modelView.m23 = 0;

    const Matrix4f mvp = camera.GetProjectionMatrix() * modelView;

    m_billboardShader->SetUniform("mvp", mvp);

    m_billboardShader->SetUniform("tex", 0);
    Texture::SetActiveTextureUnit(0);

    // the texture format is somehow corrupt?

    VBO::DrawIndices(*m_vertexBuffer, *m_indexBuffer, GL_TRIANGLES, (NUM_BILLBOARD_TRIANGLES)*3);

}

void Skydome::DrawClouds(const Camera& camera) {


    for(const CloudGroup* cloudGroup : m_clouds ) {

	cloudGroup->m_cloudTexture->Bind();

	for(const CloudInfo& cloud : cloudGroup->m_clouds) {


	    DrawBillboard(camera, m_sunVertexBuffer, m_sunIndexBuffer,
			  cloud.m_rotation, cloud.m_elevation, cloud.m_rotation);


	}

	cloudGroup->m_cloudTexture->Unbind();


    }

/*    m_sunTexture->Bind();

    DrawBillboard(camera, m_sunVertexBuffer, m_sunIndexBuffer, -10.0f,5.0f,0.0f);

    m_sunTexture->Unbind();*/
}
