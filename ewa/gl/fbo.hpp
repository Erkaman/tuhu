#pragma once

#include "gl_common.hpp"

class RenderBuffer;
class Texture;


class FBO{

protected:

    FBO(const FBO&);
    FBO& operator=(const FBO&);

    GLuint m_fboHandle;
    const GLenum m_target;
    GLenum m_targetTextureUnit;

    RenderBuffer* m_depthBuffer;

    Texture* m_renderTargetTexture;

    void Attach(const  GLenum attachment, const Texture& texture);
    void Attach(const  GLenum attachment, GLuint textureHandle, GLenum textureTarget);


    static void CheckFramebufferStatus(const GLenum target);

    void CheckFramebufferStatus();


public:

    FBO();
    void Init(const GLenum targetTextureUnit, const GLsizei width, const GLsizei height);

    virtual ~FBO();

    GLenum GetTargetTextureUnit() const;

    /**
     * This method MUST be called every time the window is resized!
     */
    virtual void RecreateBuffers(const GLsizei width, const GLsizei height) = 0;

    void BindForWriting() {
	GL_C(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboHandle));
    }

    void UnbindForWriting() {
	GL_C(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
    }

    void Bind() {
	GL_C(glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle));
    }

    void Unbind() {
	GL_C(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

/*    void blitFrameBuffer() {
	glBlitFramebuffer(0, 0, Display.getWidth(), Display.getHeight(), 0, 0, Display.getWidth(), Display.getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}*/

    void BindForReading() {
	GL_C(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboHandle));
    }

    void UnbindForReading() {
	GL_C(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
    }

    Texture& GetRenderTargetTexture() const{
	return *m_renderTargetTexture;
    }



};
