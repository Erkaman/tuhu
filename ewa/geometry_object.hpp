#pragma once

#include "math/matrix4f.hpp"

#include "geometry_object_data.hpp"

#include "igeometry_object.hpp"
#include "geometry_object_listener.hpp"

#include <LinearMath/btQuaternion.h>

class Vector3f;
class VBO;
class ShaderProgram;
class Texture;
class Cube;
class PhysicsWorld;
class btRigidBody;
class btMotionState;
class CubeMapTexture;
class ColorFBO;
class ColorDepthFbo;
class Paraboloid;

class GeometryObject : public IGeometryObject {
private:

    // the AABB of the object. Used for view-frustrum culling.
    AABB m_aabb;

    btRigidBody* m_rigidBody;
    btMotionState* m_motionState;

    // the id of this object. Used for object picking.
    unsigned int m_id;

    GeometryObjectData* m_data;


    Vector3f m_position;
    btQuaternion m_rotation;

    Vector3f m_editPosition;
    btQuaternion m_editRotation;

    float m_scale;
    float m_editScale;


    std::string m_filename;

    bool m_selected;

    bool m_inCameraFrustum;
    bool m_inLightFrustum;
    bool m_inReflectionFrustum;

    bool m_inEnvLightFrustums[2];

    short m_physicsGroup;
    short m_physicsMask;

    Matrix4f GetModelMatrix(const Matrix4f& scaling = Matrix4f::CreateIdentity() )const;

    AABB GetModelSpaceAABB()const;

    void RenderVertices(ShaderProgram& shader);

public:

    GeometryObject() {}

    bool Init(
    const std::string& filename,
    const Vector3f& position,
    const btQuaternion& rotation,
    float scale,
    unsigned int id,

    short physicsGroup,
    short physicsMask
	);

    virtual IGeometryObject* Duplicate(unsigned int id);


    virtual ~GeometryObject();

    static void RenderAll(
	const ICamera* camera, const Vector4f& lightPosition, const Matrix4f& lightVp, const DepthFBO& shadowMap,
	CubeMapTexture* cubeMapTexture, ColorDepthFbo& refractionMap, const ColorFBO& reflectionMap);

    static void RenderAllEnv(const Vector4f& lightPosition, int i, Paraboloid& paraboloid);

    static std::vector<Vector3f> GetTorches();


    static void RenderReflection(ICamera* camera, const Vector4f& lightPosition);


    static void RenderIdAll(
	const ICamera* camera);


    static void Delete(IGeometryObject* geoObj);

    static void RenderShadowMapAll(const Matrix4f& lightVp); // vp = view projection matrix.

    static void SetTotalDelta(float totalDelta);

    virtual void Update(const ViewFrustum* cameraFrustum, const ViewFrustum* lightFrustum, DualParaboloidMap& dualParaboloidMap, const ViewFrustum* reflectionFrustum);

    /*
    virtual void RenderId(
	const ICamera* camera);
    */

    virtual void SetPosition(const Vector3f& position);
    virtual void SetRotation(const btQuaternion& rotation);
    virtual void SetEditPosition(const Vector3f& editPosition);
    virtual void SetEditRotation(const btQuaternion& editRotation);

    /*
    virtual void ApplyCentralForce(const Vector3f& force);
    virtual void ApplyForce(const Vector3f& force, const Vector3f& relPos);
*/

    virtual btRigidBody* GetRigidBody() const;

    virtual btMotionState* GetMotionState() const;

    virtual Vector3f GetPosition() const;
    virtual btQuaternion GetRotation() const;

    virtual void AddToPhysicsWorld(PhysicsWorld* physicsWorld);

    virtual std::string GetFilename() const;


    virtual bool IsSelected()const;
    virtual void SetSelected(bool selected);


    virtual unsigned int GetId();


    virtual float GetScale() const;
    virtual void SetScale(const float scale);
    virtual void SetEditScale(const float editScale);

    static void SetListener(GeometryObjectListener* listener);
};
