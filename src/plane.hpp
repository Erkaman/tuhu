#pragma once

class ShaderProgram;
class VBO;
class Camera;
class Texture;
class Vector4f;

#include "gl/gl_common.hpp"
#include "math/vector3f.hpp"

#include <memory>

class ShaderProgram;
class VBO;

class Plane {

    private:

    /*
      INSTANCE VARIABLES.
    */

    GLushort m_numTriangles;

    std::unique_ptr<ShaderProgram> m_noiseShader;

    Vector3f m_position;

    std::unique_ptr<VBO> m_vertexBuffer;
    std::unique_ptr<VBO> m_indexBuffer;

public:

    Plane(const Vector3f& position);

    void Draw(const Camera& camera, const Vector4f& lightPosition);

};
