#ifndef PARTICLE_SYSTEM_H
#define	PARTICLE_SYSTEM_H

#include "ewa/gl/gl_common.hpp"

#include "ewa/math/vector3f.hpp"

class Vector3f;
class Matrix4f;
class Matrix4f;
class Texture;
class ShaderProgram;
class VBO;
class RandomTexture;

class ParticleSystem
{
public:
    ParticleSystem();
    void Init();

    virtual ~ParticleSystem();

    virtual void Render(const Matrix4f& VP, const Vector3f& CameraPos);
    virtual void Update(float delta);

    void SetMinVelocity(const Vector3f& vel);
    void SetMaxVelocity(const Vector3f& vel);
    void SetMaxParticles(size_t maxParticles);
    void SetTexture(Texture* texture);
    void SetEmitRate(float emitRate);
    void SetParticleLifetime(float particleLifetime);
    void SetBillboardSize(float billboardSize);
    void SetEmitPosition(const Vector3f& emitPosition);
    void SetEmitRange(const Vector3f& emitRange);
    void SetWarmupFrames(const int warmupFrames);
    void SetEmitCount(const int emitCount);

private:

    void UpdateParticles(float delta);
    void RenderParticles(const Matrix4f& VP, const Vector3f& CameraPos);


    bool m_isFirst;
    unsigned int m_currVB;
    unsigned int m_currTFB;
    VBO* m_particleBuffer[2];
    GLuint m_transformFeedback[2];

    ShaderProgram* m_particleUpdateShader;
    ShaderProgram* m_particleBillboardShader;
    RandomTexture* m_randomTexture;

    float m_time;

    Vector3f m_minVelocity;
    Vector3f m_maxVelocity;
    size_t m_maxParticles;
    Texture* m_texture;
    float m_emitRate;
    Vector3f m_emitPosition;
    Vector3f m_emitRange;
    float m_particleLifetime;
    float m_billboardSize;
    int m_warmupFrames;
    int m_emitCount;
};


#endif	/* PARTICLE_SYSTEM_H */