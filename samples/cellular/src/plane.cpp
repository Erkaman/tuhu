#include "plane.hpp"

#include "ewa/perlin_seed.hpp"
#include "ewa/common.hpp"

#include "ewa/gl/shader_program.hpp"
#include "ewa/gl/texture2d.hpp"
#include "ewa/gl/vbo.hpp"

#include "ewa/math/matrix4f.hpp"
#include "ewa/math/vector2f.hpp"
#include "ewa/math/vector3f.hpp"

#include "ewa/eob_file.hpp"
#include "ewa/file.hpp"
#include "ewa/camera.hpp"


using std::vector;

Plane::Plane(const Vector3f& position, const bool isCellular ): m_modelMatrix(Matrix4f::CreateTranslation(position)){

    m_shader = isCellular ? new ShaderProgram("shader/cellular") : new ShaderProgram("shader/perlin");

    GeometryObjectData data;

    data.m_vertexAttribsSizes = vector<GLuint>{3,3,2};
    data.m_indexType = GL_UNSIGNED_SHORT;

    GeometryObjectData::Chunk* chunk = new GeometryObjectData::Chunk;
    chunk->m_material->m_textureFilename = "";
    chunk->m_material->m_normalMapFilename = "";
    chunk->m_material->m_specularMapFilename = "";

    chunk->m_material->m_materialName = "empty_mat";


    data.m_chunks.push_back(chunk);

    const size_t verticesSize = 4 * (3+3+2); // four corners,
    chunk->m_verticesSize = verticesSize * sizeof(float);
    vector<GLfloat> vertices;
    vertices.reserve(verticesSize);

    const size_t indicesSize = 6; // 6 indiices
    chunk->m_indicesSize = indicesSize * sizeof(GLushort);
    vector<GLushort> indices;
    indices.reserve(indicesSize);


    const float SCALE = 10.0f;
    // compute the 4 corners.
    Vector3f n = Vector3f(0,1,0);

    Vector3f(0,0,0).Add(vertices);
    n.Add(vertices);
    Vector2f(0,0).Add(vertices);

    Vector3f(SCALE,0,0).Add(vertices);
    n.Add(vertices);
    Vector2f(SCALE,0).Add(vertices);


    Vector3f(SCALE,0,SCALE).Add(vertices);
    n.Add(vertices);
    Vector2f(SCALE,SCALE).Add(vertices);

    Vector3f(0,0,SCALE).Add(vertices);
    n.Add(vertices);
    Vector2f(0,SCALE).Add(vertices);

    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(0);

    indices.push_back(0);
    indices.push_back(3);
    indices.push_back(2);


    chunk->m_numTriangles = 2;


    chunk->m_vertices = &vertices[0];
    chunk->m_indices = &indices[0];

    m_perlinSeed = new PerlinSeed(1);

    EobFile::Write(data, "obj/plane.eob");
    GeometryObjectData obj =  EobFile::Read("obj/plane.eob");

    GeometryObject::Init(obj, true);


}

void Plane::Render(const Camera& camera, const Vector4f& lightPosition) {

    m_shader->Bind();

    m_perlinSeed->Bind(*m_shader);



    const Matrix4f modelViewMatrix = camera.GetModelViewMatrix(
	m_modelMatrix);

    const Matrix4f mvp = camera.GetMvp(modelViewMatrix);

    m_shader->SetUniform("mvp", mvp);



    GeometryObject::RenderVertices(*m_shader);

    m_perlinSeed->Unbind();


    m_shader->Unbind();
}

Plane::~Plane() {
    MY_DELETE(m_shader);
    MY_DELETE(m_perlinSeed);
}
