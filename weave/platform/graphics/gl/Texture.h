/*
OpenGL 4.5 Texture Object
Helper class for OpenGL 4.5 textures
*/
#pragma once

#include "gl46.h"
#include "weave/platform/graphics/ImageData.h"
#include <string>
#include <cmath>

namespace weave::opengl {


class Texture {
protected:
	//GL Texture buffer object id
	GLenum gl_texTarget = gl::TEXTURE_2D;
	GLuint gl_tid = 0;

	//3D size of the image
	uint32_t width = 0, height = 0, depth = 0;
	//Amount of surfaces in the texture
	uint32_t numLayers = 0;
	//Amount of channels per texel
	uint32_t numChannels = 0;

public:
	Texture() = default;
	Texture(Texture const&) = delete;
	Texture(Texture &&other);
	Texture& operator=(Texture const&) = delete;
	Texture& operator=(Texture&& other);
	~Texture();
	
	Texture(GLenum glTexTarget, unsigned int glId);


	uint32_t Width() const { return width; }
	uint32_t Height() const { return height; }
	uint32_t Depth() const { return depth; }
	uint32_t NumLayers() const { return numLayers; }
	uint32_t NumChannels() const { return numChannels; }

	//Returns the GL target of the texture object (gl::TEXTURE_2D, etc)
	GLenum GLTextureTarget() const { return gl_texTarget; }
	//Returns the GL object id
	unsigned int GLId() const { return gl_tid; }

	std::pair<GLenum, unsigned int> GLTargetAndId() const { return { gl_texTarget, gl_tid }; }
	
	//Wraps an externally generated GL texture
	void WrapGLTexture(GLenum glTexTarget, unsigned int glId);

	//Sets the default sampler values
	void SetSamplerDefaults();

	//Binds the texture to the given texture unit via GL
	void Bind(unsigned int tiu);

	//Generates all mipmaps for the texture
	void GenerateMipmaps();

	//Returns the max number of mipmaps the texture can have given its size
	GLsizei MaxMipLevels() const {
		return Texture::MaxMipLevels(width, height);
	}

	static inline GLsizei MaxMipLevels(unsigned int width, unsigned int height) {
		return (GLsizei)std::floor(std::log2(std::max<unsigned int>(width, height))) + 1;
	}

private:
	void GetTextureInfo();
};

}