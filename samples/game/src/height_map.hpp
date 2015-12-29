#pragma once

#include <string>
#include "math/color.hpp"
#include "math/vector4f.hpp"
#include "math/vector2f.hpp"
#include "ewa/mult_array.hpp"
#include "math/vector2i.hpp"

class VBO;
class ShaderProgram;
class ICamera;
class Texture;
class PerlinSeed;
class Texture;
class Texture2D;
class ShaderProgra;
class Config;
class ICamera;
class PhysicsWorld;
class ValueNoise;

// the data associated with every triangle in the heightmap mesh.
struct Cell {
    Vector3f position;
    float id;
    Vector2f texCoord;
};

struct SplatColor{
public:
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

};

class HeightMap {


private:

    unsigned int m_numTriangles;
    bool m_isWireframe;

    VBO* m_vertexBuffer;
    VBO* m_indexBuffer;

    VBO* m_cursorVertexBuffer;
    unsigned short m_numCursorPoints;

    ShaderProgram* m_shader;
    ShaderProgram* m_depthShader; //outputs only the depth. Used for shadow mapping.
    ShaderProgram* m_idShader; //outputs only the id. Used for triangle picking in the height map.
    ShaderProgram* m_cursorShader;

    Texture* m_grassTexture;
    Texture* m_dirtTexture;
    Texture* m_rockTexture;

    Texture2D* m_heightMap;
    Texture2D* m_splatMap;

    MultArray<Cell> *m_map;
    MultArray<unsigned short>* m_heightData;
    MultArray<SplatColor>* m_splatData;

    // used to store temp data in SmoothTerrain()
    MultArray<unsigned short>* m_tempData;

    Config* m_config;

    Vector2i m_cursorPosition;
    bool m_cursorPositionWasUpdated;

    Vector3f m_offset;
    float m_xzScale;
    float m_yScale;
    int m_resolution;
    float m_textureScale;
    int HEIGHT_MAP_SIZE;

    int m_cursorSize;

    ValueNoise* m_noise;

    unsigned int pboIds[2];

    // if true, update heightmap using FBO this frame.
    int m_updateHeightMap;

    static const float ComputeY(const unsigned char heightMapData );
    static const float ScaleXZ(const int x);
    static const Color VertexColoring(const float y);

    void CreateHeightmap(const std::string& heightMapFilename, bool guiMode);
    void CreateCursor();
    void CreateSplatMap(const std::string& splatMapFilename, bool guiMode);

    void LoadHeightmap(const std::string& heightMapFilename);
    void LoadSplatMap(const std::string& splatMapFilename);

    void RenderHeightMap(const ICamera* camera, const Vector4f& lightPosition);
    void RenderCursor(const ICamera* camera);


    void Render(ShaderProgram* shader);

    void RenderSetup(ShaderProgram* shader);
    void RenderUnsetup();

    void UpdateCursor(ICamera* camera,
		      const float framebufferWidth,
		      const float framebufferHeight);

    void Init(const std::string& heightMapFilename, const std::string& splatMapFilename, bool guiMode );

    bool InBounds(int x, int z);

    void UpdateHeightMap();

public:

    HeightMap(const std::string& heightMapFilename, const std::string& splatMapFilename, bool guiMode );
    HeightMap(bool guiMode );


    ~HeightMap();

    void Render(const ICamera* camera, const Vector4f& lightPosition);
    void RenderShadowMap(const ICamera* camera);

    // render, but instead of outputting colors for every triangle, we output the id of the frontmost triangles.
    void RenderId(const ICamera* camera);

    void SetWireframe(const bool wireframe);

    float GetHeightAt(float x, float z)const;

    void Update(const float delta, ICamera* camera,
		const float framebufferWidth,
		const float framebufferHeight);

    void ModifyTerrain(const float delta, const float strength);
    void DistortTerrain(const float delta, const float strength, float noiseScale);
    void SmoothTerrain(const float delta, const int smoothRadius);
    void LevelTerrain(const float delta, const float strength);

    void DrawTexture(const float delta, int drawTextureType);


    void SaveHeightMap(const std::string& filename);
    void SaveSplatMap(const std::string& filename);


    void AddToPhysicsWorld(PhysicsWorld* physicsWorld);

    void SetCursorSize(int cursorSize);

};
