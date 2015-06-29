#include "height_map.hpp"

#include <vector>
#include "lodepng.h"

#include "log.hpp"

#include "gl/vbo.hpp"
#include "gl/shader_program.hpp"
#include "math/matrix4f.hpp"
#include "camera.hpp"
#include "math/vector3f.hpp"
#include "math/vector4f.hpp"
#include "math/color.hpp"
#include "mult_array.hpp"

//#include "common.hpp"

using std::unique_ptr;
using std::vector;

struct Cell {
    Vector3f position;
    Vector3f normal;
    Color color;
};

static Vector3f CalculateNormal (float north, float south, float east, float west)
{
    return Vector3f(
	west - east,
	2.0f,
	north - south).Normalize();
}


HeightMap::HeightMap(const std::string& path): m_isWireframe(false) {

    /*
      load the shader
     */
    shader = make_unique<ShaderProgram>("shader/height_map");


    /*
      Load the heightmap data.
     */

    std::vector<unsigned char> buffer;
    lodepng::load_file(buffer, path);

    lodepng::State state;
    std::vector<unsigned char> imageData;
    unsigned error = lodepng::decode(imageData, m_width, m_depth, state, buffer);

    if(error != 0){
	LOG_E("could not load height map: %s", lodepng_error_text(error));
    }

    /*
      Next we create the vertex buffer.
     */


    vertexBuffer = unique_ptr<VBO>(VBO::CreateInterleaved(
				       vector<GLuint>{
					   VBO_POSITION_ATTRIB_INDEX,
					       VBO_NORMAL_ATTRIB_INDEX,
					       VBO_COLOR_ATTRIB_INDEX},
				       vector<GLuint>{3,3,4}
				       ));

    UintVector indices;


    m_numTriangles = 0;

    LOG_I("width: %d", m_width);
    LOG_I("depth: %d", m_depth);
    LOG_I("imageData size: %ld", imageData.size());


    MultArray<Cell> map(m_width, m_depth);

    unsigned int xpos = 0;
    unsigned int zpos = 0;

    for(size_t i = 0; i < imageData.size(); i+=4) {

	map.Get(xpos, zpos).position = Vector3f(xpos, ComputeY(imageData[i]), zpos);
//	LOG_I("pos: %s", tos(map.Get(xpos, zpos).position).c_str() );

	++xpos;
	if(xpos != 0 && ( xpos % (m_width) == 0)) {
	    xpos = 0;
	    ++zpos;
	}
    }

    /*
      TODO: SMOOTH OUT THE VERTEX DATA.
     */

    for(size_t x = 0; x < m_width; ++x) {
	for(size_t z = 0; z < m_depth; ++z) {
	    Cell& c = map.Get(x,z);

	    c.normal = CalculateNormal(
		map.GetWrap(x,z-1).position.y,
		map.GetWrap(x,z+1).position.y,
		map.GetWrap(x+1,z).position.y,
		map.GetWrap(x-1,z).position.y);

	    c.color = VertexColoring(c.position.y);
	}
    }

    vertexBuffer->Bind();
    vertexBuffer->SetBufferData(map);
    vertexBuffer->Unbind();

    GLuint baseIndex = 0;

    for(size_t x = 0; x < (m_width-1); ++x) {
	for(size_t z = 0; z < (m_depth-1); ++z) {

	    indices.push_back(baseIndex+m_width);
	    indices.push_back(baseIndex+1);
	    indices.push_back(baseIndex+0);

	    indices.push_back(baseIndex+m_width+1);
	    indices.push_back(baseIndex+1);
	    indices.push_back(baseIndex+m_width);

	    m_numTriangles += 2;
	    baseIndex += 1;
	}
	baseIndex += 1;
    }

    indexBuffer = unique_ptr<VBO>(VBO::CreateIndex(GL_UNSIGNED_INT));


    indexBuffer->Bind();
    indexBuffer->SetBufferData(indices);
    indexBuffer->Unbind();


}


void HeightMap::Draw(const Camera& camera)  {

    shader->Bind();
    shader->SetUniform("mvp", camera.GetMvp());
    Matrix4f viewMatrix = camera.GetViewMatrix();
    shader->SetUniform("modelViewMatrix", viewMatrix);
    Matrix4f normalMatrix(viewMatrix);
    viewMatrix.Transpose().Inverse();
    shader->SetUniform("normalMatrix", normalMatrix);

    Vector4f lightPosition(0,10,0,1);

    shader->SetUniform("viewSpaceLightPosition", Vector3f(viewMatrix * lightPosition) );

    if(m_isWireframe)
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );


    vertexBuffer->Bind();
    vertexBuffer->EnableVertexAttribInterleaved();
    vertexBuffer->Bind();

    indexBuffer->Bind();

    indexBuffer->DrawIndices(GL_TRIANGLES, (m_numTriangles)*3);

    indexBuffer->Unbind();

    vertexBuffer->Bind();
    vertexBuffer->DisableVertexAttribInterleaved();
    vertexBuffer->Bind();

    if(m_isWireframe)
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    shader->Unbind();
}

const float HeightMap::ComputeY(const unsigned char heightMapData ) {
//    return ((float)heightMapData  / 255.0f) * 0.2;
    return ((float)heightMapData  / 255.0f) * 1.0;

}

void HeightMap::SetWireframe(const bool wireframe) {
    m_isWireframe = wireframe;
}

const float HeightMap::ScaleXZ(const float x) {
//    return 0.03f * x;
    return x;

}

const Color HeightMap::VertexColoring(const float y) {
    return Color(0.33,0.33,0.33);
    /*
    if(y < 0.5f) {
	Color lower = Color::FromInt(237, 201, 175);
	Color higher = Color::FromInt(0, 255, 0);
	return Color::Lerp(lower, higher, y / 0.5f);
    } else {
	Color lower = Color::FromInt(0, 255, 0);
	Color higher = Color::FromInt(100, 100, 100);
	return Color::Lerp(lower, higher, (y-0.5f) / 0.5f);
	}*/
}
