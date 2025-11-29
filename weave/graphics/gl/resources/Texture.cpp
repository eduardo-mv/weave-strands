#include "Texture.h"
#include "Texture.h"
#include <algorithm>
#include <sstream>
#include "ApiHelpers.h"

using namespace weave::opengl;

Texture::Texture(Texture&& other) 
	: gl_tid(other.gl_tid) {
	other.gl_tid = 0;
}

Texture& Texture::operator=(Texture&& other) {
	std::swap(gl_tid, other.gl_tid);
	return *this;
}


Texture::~Texture() {
	gl::DeleteTextures(1, &gl_tid);
}

weave::opengl::Texture::Texture(GLenum glTexTarget, unsigned int glId)
	: gl_texTarget(glTexTarget)
	, gl_tid(glId)
{
	GetTextureInfo();
	SetSamplerDefaults();
}

void Texture::WrapGLTexture(GLenum glTexTarget, unsigned int glId) {
	gl::DeleteTextures(1, &gl_tid); 
	gl_texTarget = glTexTarget;
	gl_tid = glId;
	
	GetTextureInfo();
}

void Texture::SetSamplerDefaults() {
	//Set the default sampler parameters
	gl::TextureParameteri(gl_tid, gl::TEXTURE_WRAP_S, gl::REPEAT);
	gl::TextureParameteri(gl_tid, gl::TEXTURE_WRAP_T, gl::REPEAT);
	gl::TextureParameteri(gl_tid, gl::TEXTURE_WRAP_R, gl::REPEAT);
	gl::TextureParameteri(gl_tid, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
	gl::TextureParameteri(gl_tid, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
	gl::TextureParameterf(gl_tid, gl::TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
	gl::TextureParameteri(gl_tid, gl::TEXTURE_COMPARE_MODE, gl::NONE);
	//gl::TexParameteri(target, gl::TEXTURE_COMPARE_FUNC, gl::NOTEQUAL);
	//gl::TexParameterfv(target, gl::TEXTURE_BORDER_COLOR, {0.0f,0.0f,0.0f,0.0f});
}

void Texture::Bind(unsigned int tiu) {
	gl::ActiveTexture(gl::TEXTURE0 + tiu);
	gl::BindTexture(GLTextureTarget(), gl_tid);	
}

void Texture::GenerateMipmaps() {
	gl::BindTexture(GLTextureTarget(), gl_tid);
	gl::GenerateMipmap(GLTextureTarget());
}

void Texture::GetTextureInfo() {

	gl::GetTextureLevelParameteriv(gl_tid, 0, gl::TEXTURE_WIDTH, (GLint*)&width);
	gl::GetTextureLevelParameteriv(gl_tid, 0, gl::TEXTURE_HEIGHT, (GLint*)&height);
	gl::GetTextureLevelParameteriv(gl_tid, 0, gl::TEXTURE_DEPTH, (GLint*)&depth);

	int r, g, b, a;
	gl::GetTextureLevelParameteriv(gl_tid, 0, gl::TEXTURE_RED_SIZE, (GLint*)&r);
	gl::GetTextureLevelParameteriv(gl_tid, 0, gl::TEXTURE_GREEN_SIZE, (GLint*)&g);
	gl::GetTextureLevelParameteriv(gl_tid, 0, gl::TEXTURE_BLUE_SIZE, (GLint*)&b);
	gl::GetTextureLevelParameteriv(gl_tid, 0, gl::TEXTURE_ALPHA_SIZE, (GLint*)&a);
	if (r) ++numChannels;
	if (g) ++numChannels;
	if (b) ++numChannels;
	if (a) ++numChannels;

	numLayers = depth;
}

