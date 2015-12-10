#pragma once

#include "ewa/application.hpp"
#include "ewa/common.hpp"

#include "ewa/math/vector4f.hpp"
#include "ewa/math/matrix4f.hpp"

class Camera;
class HeightMap;
class Skydome;
class Grass;
class Sound;
class ParticleSystem;
class GeometryObject;
class DepthFBO;
class Line;
class Points;
class ViewFrustum;
class PhysicsWorld;

class TuhuApplication : public Application{

private:

    Camera* m_camera;
    HeightMap* m_heightMap;

    Skydome* m_skydome;

    Grass* m_grass;

    Sound* m_windSound;

    ParticleSystem* m_smoke;
    ParticleSystem* m_snow;
    ParticleSystem* m_fire;


    GeometryObject* m_stoneFloor;
    GeometryObject* m_flatWoodFloor;
    GeometryObject* m_woodFloor;
    GeometryObject* m_sphere;
    GeometryObject* m_plane;
    GeometryObject* m_tree;
    GeometryObject* m_wall;
    GeometryObject* m_wall2;
    GeometryObject* m_ball2;

    GeometryObject* m_car;


    DepthFBO* m_depthFbo;

    Line* m_line;
    Points* m_points;

    Vector4f m_lightDirection;

    // used in shadow mapping for rendering all object from the perspective of the light source.
    Matrix4f m_lightProjectionMatrix;
    Matrix4f m_lightViewMatrix;

    float m_totalDelta;

    ViewFrustum* m_viewFrustum;

    std::vector<GeometryObject*> m_geoObjs;

    PhysicsWorld* m_physicsWorld;

    GeometryObject* LoadObj(const std::string& path, const Vector3f& position);

    Matrix4f MakeLightProj()const;

public:

    TuhuApplication(int argc, char *argv[]);
    ~TuhuApplication();

    void Init() override;
    void Render() override;
    void RenderText() override;
    void Update(const float delta) override;

    void RenderShadowMap();
    void RenderScene();

};
