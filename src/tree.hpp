#pragma once

#include "gl/gl_common.hpp"
#include "math/vector3f.hpp"
#include "colonization.hpp"


class ShaderProgram;
class VBO;
class Camera;
class Texture;
class Vector4f;

class Tree {

private:

    /*
      INSTANCE VARIABLES.
    */

    FloatVector m_leavesVertices;
    UshortVector m_leavesIndices;
    GLushort m_leavesNumTriangles;

    FloatVector m_treeVertices;
    UshortVector m_treeIndices;
    GLushort m_treeNumTriangles;



    // the position of the beginning of the stem.
    Vector3f m_stemPosition;

    std::unique_ptr<ShaderProgram> m_phongShader;

    std::unique_ptr<VBO> m_leavesVertexBuffer;
    std::unique_ptr<VBO> m_leavesIndexBuffer;

    std::unique_ptr<VBO> m_treeVertexBuffer;
    std::unique_ptr<VBO> m_treeIndexBuffer;


    std::unique_ptr<Texture> m_leafTexture;

    Colonization m_colonization;

    /*
      PRIVATE INSTANCE METHODS
    */

    // position is with respect to m_stemPosition
    void AddLeaf(const Vector3f& position);

    void  AddLeafFace(
	const Vector3f& leafPosition,
	const float ax, const float ay, const float az,
	const float bx, const float by, const float bz,
	const float cx, const float cy, const float cz,
	const float dx, const float dy, const float dz,

	const float nx, const float ny, const float nz

	);

    void DrawLeaves(const Camera& camera, const Vector4f& lightPosition);
    void DrawTree(const Camera& camera, const Vector4f& lightPosition);

    void AddBranch(const std::vector<float>& branchWidths, const std::vector<Vector3f>& branchPositions, const int steps = 5);

public:

    /*
      CONSTRUCTORS
    */

    Tree(const Vector3f& position);

    void Draw(const Camera& camera, const Vector4f& lightPosition);

};
