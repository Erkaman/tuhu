#pragma once

#include "math/matrix4f.hpp"

#include "geometry_object_data.hpp"

#include "igeometry_object.hpp"

#include <LinearMath/btQuaternion.h>

class Vector3f;
class VBO;
class ShaderProgram;
class Texture;
class Cube;
class PhysicsWorld;
class btRigidBody;
class btMotionState;

class GeometryObject : public IGeometryObject {
private:

    // the AABB of the object. Used for view-frustrum culling.
    AABB m_aabb;

    // wireframe cube used to visualize the AABB of the object.
    Cube* m_aabbWireframe;

    btRigidBody* m_rigidBody;
    btMotionState* m_motionState;

    // the id of this object. Used for object picking.
    unsigned int m_id;

    GeometryObjectData* m_data;


    Vector3f m_position;
    btQuaternion m_rotation;

    Vector3f m_editPosition;
    btQuaternion m_editRotation;

    std::string m_filename;

    Matrix4f GetModelMatrix(const Matrix4f& scaling = Matrix4f::CreateIdentity() )const;

    AABB GetModelSpaceAABB()const;

    void RenderVertices(ShaderProgram& shader);

public:

    GeometryObject() {}

    bool Init(
    const std::string& filename,
    const Vector3f& position,
    const btQuaternion& rotation,
    unsigned int id);


    virtual ~GeometryObject();

    static void RenderAll(

	const ICamera* camera, const Vector4f& lightPosition, const Matrix4f& lightVp, const DepthFBO& shadowMap);

    virtual void RenderShadowMap(const Matrix4f& lightVp); // vp = view projection matrix.

    virtual void RenderWithOutlines(
	const ICamera* camera,
	const Vector4f& lightPosition,
	const Matrix4f& lightVp,
	const DepthFBO& shadowMap);

    virtual void RenderId(
	const ICamera* camera);


    virtual void SetPosition(const Vector3f& position);
    virtual void SetRotation(const btQuaternion& rotation);
    virtual void SetEditPosition(const Vector3f& editPosition);
    virtual void SetEditRotation(const btQuaternion& editRotation);

    virtual void ApplyCentralForce(const Vector3f& force);

    virtual void ApplyForce(const Vector3f& force, const Vector3f& relPos);

    virtual btRigidBody* GetRigidBody() const;

    virtual btMotionState* GetMotionState() const;

    virtual Vector3f GetPosition() const;
    virtual btQuaternion GetRotation() const;

    virtual void AddToPhysicsWorld(PhysicsWorld* physicsWorld);

    virtual std::string GetFilename() const;

};
