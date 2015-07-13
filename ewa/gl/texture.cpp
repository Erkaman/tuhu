#include "texture.hpp"
#include "math/math_common.hpp"

#include "lodepng.h"

#include "log.hpp"



GLfloat Texture::cachedMaxAnisotropic = 0.0f;

Texture::operator std::string() const {
    return "Texture ID: " + std::to_string(m_textureHandle) + ", " + "Width: "
	+ std::to_string(m_width) + ", " + "Height: "
	+ std::to_string(m_height) + ", ";
}



float Texture::GetMaxAnisotropic() {
    if(FloatEquals(cachedMaxAnisotropic, 0)) {
	GL_C(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &cachedMaxAnisotropic));
    }
    return cachedMaxAnisotropic;
}



Texture::Texture(const GLenum target, const GLsizei width, const GLsizei height, const GLint internalFormat, const GLenum type):
    Texture(target) {

    Bind();

    m_width = width;
    m_height = height;

    GL_C(glTexImage2D(m_target, 0, internalFormat, width, height, 0, GL_RGBA, type, NULL));

    Unbind();
}

void Texture::SetBestMagMinFilters(const bool generateMipmap) {
    if(generateMipmap)
	GenerateMipmap();

    EnableAnisotropicFiltering();

    GL_C(glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_C(glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
}

void Texture::SetMinFilter(const GLint filter) {
    GL_C(glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, filter ));
}


void Texture::SetMagFilter(const GLint filter) {
    GL_C(glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER,  filter));
}


void Texture::SetMagMinFilters(const GLint filter) {
    SetMagFilter(filter);
    SetMinFilter(filter);
}

void Texture::GenerateMipmap() {
    GL_C(glGenerateMipmap(m_target));
}

void Texture::SetActiveTextureUnit(const GLenum textureUnit) {
    GL_C(glActiveTexture(GL_TEXTURE0 + textureUnit));
}

void Texture::Bind() {
    if(m_alreadyBound)
	return;

    GL_C(glBindTexture(m_target, m_textureHandle));
    m_alreadyBound = true;
}


void Texture::Unbind() {
    if(!m_alreadyBound)
	return;

    GL_C(glBindTexture(m_target, 0));
    m_alreadyBound = false;
}


void Texture::SetTextureRepeat() {
    SetTextureWrap(GL_REPEAT);
}

void Texture::SetTextureWrap(const GLenum s, const GLenum t) {
    GL_C(glTexParameteri(m_target, GL_TEXTURE_WRAP_S, s));
    GL_C(glTexParameteri(m_target, GL_TEXTURE_WRAP_T, t));
}


void Texture::SetTextureWrap(const GLenum sAndT) {
    SetTextureWrap(sAndT, sAndT);
}

Texture::Texture(const GLenum target):m_alreadyBound(false), m_target(target){
    GL_C(glGenTextures(1, &m_textureHandle));
}


Texture::~Texture()  {
    GL_C(glDeleteTextures(1, &m_textureHandle));
}


void Texture::SetTextureClamping() {
    SetTextureWrap(GL_CLAMP_TO_EDGE);
}


void Texture::EnableAnisotropicFiltering() {
    GL_C(glTexParameterf(m_target,GL_TEXTURE_MAX_ANISOTROPY_EXT, GetMaxAnisotropic()));
}

void Texture::SetBestMagMinFilters() {
    SetBestMagMinFilters(true);
}


GLsizei Texture::GetWidth() const{
    return m_width;
}

GLsizei Texture::GetHeight() const{
    return m_height;
}

GLenum Texture::GetTarget() const{
    return m_target;
}

GLuint Texture::GetHandle() const{
    return m_textureHandle;
}

void Texture::WriteToFile(const std::string& filename) {

    // first, get the raw pixel data from opengl
    Bind();
    size_t size = m_width* m_height * 4;
    unsigned char* pixels = new unsigned char[size];
    GL_C(glGetTexImage(m_target, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
    Unbind();

    // flip the image.

    unsigned int* intPixels = (unsigned int*)pixels;

    for (int i=0;i<m_width;++i){
	for (int j=0;j<m_height/2;++j){
	    unsigned int temp = intPixels[j * m_width + i];

	    intPixels[j * m_width + i] = intPixels[ (m_height-j-1)*m_width + i];
	    intPixels[(m_height-j-1)*m_width + i] = temp;
	}
    }

    unsigned char* charPixels = (unsigned char*)intPixels;

    //Encode the image
    unsigned error = lodepng::encode(filename, charPixels, m_width, m_height);

    delete pixels;

  //if there's an error, display it
    if(error)
      LOG_E("PNG encoder error: %d: %s", error, lodepng_error_text(error) );
}