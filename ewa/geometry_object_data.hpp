#pragma once

#include "gl/gl_common.hpp"
#include "math/vector3f.hpp"
#include "math/vector4f.hpp"

#include "aabb.hpp"

#include <string>

struct Material{
    std::string m_materialName;

    std::string m_textureFilename;
    std::string m_normalMapFilename;
    std::string m_specularMapFilename;

    float m_specularExponent;
    Vector3f m_specularColor;
    Vector3f m_diffuseColor;
    bool m_hasHeightMap;

    Material() {
	this->m_textureFilename = "";
	this->m_normalMapFilename = "";
	this->m_specularMapFilename = "";

	this->m_specularExponent = 1; // default shininess.
	this->m_hasHeightMap = false;

	// default specular color is black
	// (which means there is no specular lighting at all)
	this->m_specularColor = Vector3f(0);
	this->m_diffuseColor = Vector3f(1);
	this->m_specularExponent = 1.0;


    }
};

struct EntityInfo{
public:
    float m_mass;

    // a static object wont move even if you collide into it.
    bool m_isStatic;

    // descriptive name.
    std::string m_name;

    EntityInfo() {
	m_mass = 1.0f;
	m_isStatic = true;
	m_name = "Unnamed Object";
    }
};

enum CollisionShapeEnum {
    CylinderShape,
    SphereShape,
    ConeShape,
    BoxShape
};

struct CollisionShape {
    CollisionShapeEnum m_shape;
    Vector3f m_origin;
    Vector4f m_rotate; // quaternion rotation
    float m_radius;
    Vector3f m_halfExtents;

    CollisionShape() {
	m_origin = Vector3f(0.0f);
	m_rotate = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
	m_radius = 1.0;
	m_halfExtents = Vector3f(1.0f);
    }
};

struct GeometryObjectData {

    GLsizeiptr m_verticesSize;
    GLvoid* m_vertices;

    struct Chunk{

	GLsizeiptr m_indicesSize;
	GLvoid* m_indices;

	GLuint m_numTriangles;

	Material* m_material; // store texture in material.
    };

    // every chunk is its own VBO, and has its own shader.
    std::vector<Chunk*> m_chunks;

    // how much data required for each vertex in the chunk.
    std::vector<GLuint> m_vertexAttribsSizes;

    // type of the integer used to store vertex indices.
    // But right now we only allow GL_UNSIGNED_SHORT.
    GLenum m_indexType;

    AABB aabb;

    EntityInfo* m_entityInfo;

    CollisionShape* m_collisionShape;
};
