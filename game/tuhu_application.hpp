#pragma once

#include "application.hpp"
#include "common.hpp"

class Texture;
struct TextureDeleter{void operator()(Texture *p);};

class Camera;
class HeightMap;
class Skybox;

class TuhuApplication : public Application{

private:

    std::unique_ptr<Camera> camera;
    std::unique_ptr<HeightMap> heightMap;
    std::unique_ptr<Skybox> skybox;


public:

    TuhuApplication();
    ~TuhuApplication();

    void Init() override;
    void Render() override;
    void Update(const float delta) override;

};