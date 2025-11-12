/*
OpenGL 4.6 Texture Object Loader
Creates texture objects based on ImageData or raw empty data
*/
#pragma once

#include "gl46.h"
#include "weave/platform/graphics/ImageData.h"
#include "Texture.h"

namespace weave::opengl {

class TextureLoader {
	
	TextureLoader() = default;
	TextureLoader(TextureLoader const&) = delete;
	TextureLoader(TextureLoader &&other) = delete;
	TextureLoader& operator=(TextureLoader const&) = delete;
	TextureLoader& operator=(TextureLoader&& other) = delete;
	~TextureLoader() = delete;

public:
	static Texture LoadImage2D(weave::graphics::ImageData const& imageData, unsigned int gl_compressionFlag = gl::RGBA8);
	static Texture LoadImage2D(uint32_t imgWidth, uint32_t imgHeight, unsigned int gl_internalFormat = gl::RGBA8);
	static Texture LoadImage2DMultiSample(uint32_t imgWidth, uint32_t imgHeight, uint32_t samples, unsigned int gl_internalFormat = gl::RGBA8, bool fixedSamples = false);

	static Texture LoadImage2DArray(weave::graphics::ImageData const& imageData, unsigned int gl_compressionFlag = gl::RGBA8);
	static Texture LoadImage2DArray(uint32_t imgWidth, uint32_t imgHeight, uint32_t layerCount, unsigned int gl_compressionFlag = gl::RGBA8);
	
	static Texture LoadImage3D(weave::graphics::ImageData const& imageData, unsigned int gl_compressionFlag = gl::RGBA8);
	static Texture LoadImage3D(uint32_t imgWidth, uint32_t imgHeight, uint32_t imgDepth, unsigned int gl_internalFormat = gl::RGBA8);
	
	static Texture LoadImageCubemap(weave::graphics::ImageData const& imageData, unsigned int gl_compressionFlag = gl::RGBA8);
	static Texture LoadImageCubemap(uint32_t imgWidth, uint32_t imgHeight, unsigned int gl_compressionFlag = gl::RGBA8);

private:
	//Support functions
	//Given two GL format IDs returns the most suitable ID. gl_target_code is the ID desired and gl_image_code the actual ID for the image.
	//This helps with creating correct sRGB images for compressed types and evaluating if the original data format will be accepted by GL
	//to convert the data into the target format.
	static unsigned int CompressionCodeAdjust(unsigned int gl_target_code, unsigned int gl_image_code);
	//Returns the equivalent data format GL enum based on the supplied internalFormat enum. This is used for gl::TexImage* calls to automatically set the format paramter based on the internalFormat parameter
	//when no user data is supplied.
	static GLenum GetGLDataFormatFromInternalFormat(GLenum internalFormat);
};

}