#ifndef PARTICLE_SYSTEM_H
#define	PARTICLE_SYSTEM_H

#include "ewa/gl/gl_common.hpp"

#include "ewa/math/vector3f.hpp"
#include "ewa/math/color.hpp"

class Vector3f;
class Matrix4f;
class Matrix4f;
class Texture;
class ShaderProgram;
class VBO;
class RandomTexture;
class Gbuffer;

enum ColorBlendingMode {
    ALPHA_BLENDING_MODE,
    ADDITIVE_BLENDING_MODE
};

class ParticleSystem
{
public:
    ParticleSystem();
    void Init();

    virtual ~ParticleSystem();

    virtual void Render(Gbuffer* gbuffer, const Matrix4f& VP, const Vector3f& CameraPos, int windowWidth, int windowHeight, const Vector3f& emitPosition);

    virtual void RenderSetup(Gbuffer* gbuffer, const Matrix4f& VP, const Vector3f& CameraPos, int windowWidth, int windowHeight);

    virtual void RenderUnsetup(Gbuffer* gbuffer, const Matrix4f& VP, const Vector3f& CameraPos, int windowWidth, int windowHeight);



    virtual void Update(float delta);

    void SetMinVelocity(const Vector3f& vel);
    void SetMaxVelocity(const Vector3f& vel);
    void SetVelocity(const Vector3f& vel);

    void SetMaxParticles(size_t maxParticles);
    void SetTexture(Texture* texture);
    void SetEmitRate(float emitRate);

    void SetBaseEmitPosition(const Vector3f& emitPosition);
    void SetEmitPosition(const Vector3f& emitPosition);
    void SetEmitPositionVariance(const Vector3f& emitPositionVariance);

    void SetWarmupFrames(const int warmupFrames);
    void SetEmitCount(const int emitCount);

    void SetBaseStartSize(const float startSize);
    void SetBaseEndSize(const float endSize);
    void SetSize(const float size);

    void SetStartSize(const float startSize);
    void SetEndSize(const float endSize);


    void SetStartColor(const Color& startColor);
    void SetEndColor(const Color& endColor);
    void SetColor(const Color& color);

    void SetBlendingMode(const ColorBlendingMode blendingMode);


    void SetBaseParticleLifetime(float particleLifetime);
    void SetParticleLifetimeVariance(const float particleLifetimeVariance);
    void SetParticleLifetime(float particleLifetime);

    void SetStartSizeVariance(const float startSizeVariance);
    void SetEndSizeVariance(const float endSizeVariance);

private:

    void UpdateParticles(float delta);


    ShaderProgram* m_particleBillboardShader;
    RandomTexture* m_randomTexture;

    float m_time;


    Texture* m_texture;

    /*
      Minimum and maximum velocity
     */
    Vector3f m_minVelocity;
    Vector3f m_maxVelocity;

    size_t m_maxParticles;

    /*
      How often particles are emitted.
     */
    float m_emitRate;

    /*
      How many particles are emitted each emit.
     */
    int m_emitCount;

    /*
      The position from which the particles are emitted.
     */
    Vector3f m_emitPosition;
    Vector3f m_emitPositionVariance;


    /*
      The lifetime of a particle.
     */
    float m_particleLifetime;

    /*
      The initial and final sizes.
     */
    float m_startSize;
    float m_endSize;
    float m_startSizeVariance;
    float m_endSizeVariance;

    int m_warmupFrames;

    Color m_startColor;
    Color m_endColor;

    GLenum m_sfactor;
    GLenum m_dfactor;

    float m_particleLifetimeVariance;
};


#endif	/* PARTICLE_SYSTEM_H */
