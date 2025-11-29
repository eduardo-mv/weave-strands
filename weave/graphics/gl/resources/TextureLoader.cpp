#include "TextureLoader.h"
#include "ApiHelpers.h"
#include <algorithm>
#include <sstream>

using namespace weave::opengl;
using namespace weave::graphics;

Texture TextureLoader::LoadImage2D(ImageData const& img, unsigned int gl_compressionFlag) {

	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};

	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1);
	gl::BindTexture(gl::TEXTURE_2D, gl_tid);

	auto surface = img.GetSurface(0, 0);

	//Transfer the texture data via a PBO. This should allow GL to delay the transfer
	//The data pointer is passed through into the PBO with BufferData and the call to TexImage2D assumes it has to use a PBO if called with nullptr as data
	auto internalFormat = img.GetGLDataType();
	auto compressionFlag = CompressionCodeAdjust(gl_compressionFlag, internalFormat);

	GLuint pixelBufferID{};
	for (unsigned int i = 0; i < img.GetNumMipmaps(); ++i) {
		surface = img.GetSurface(0, i);
		gl::GenBuffers(1, &pixelBufferID);
		gl::BindBuffer(gl::PIXEL_UNPACK_BUFFER, pixelBufferID);
		gl::BufferData(gl::PIXEL_UNPACK_BUFFER, surface.data.size(), surface.data.data(), gl::STREAM_DRAW);

		if (internalFormat == gl::COMPRESSED_RGB_S3TC_DXT1_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT5_EXT) {
			//Compressed texture data
			gl::CompressedTexImage2D(gl::TEXTURE_2D, i, internalFormat, surface.width, surface.height, 0, (GLsizei)surface.data.size(), nullptr);
		}
		else {
			//Uncompressed texture data
			gl::TexImage2D(gl::TEXTURE_2D, i, compressionFlag, surface.width, surface.height, 0, internalFormat, gl::UNSIGNED_BYTE, nullptr);
		}

		gl::DeleteBuffers(1, &pixelBufferID);
		gl::BindBuffer(gl::PIXEL_UNPACK_BUFFER, 0);
	}

	if (img.GetNumMipmaps() == 1) {
		gl::GenerateMipmap(gl::TEXTURE_2D);
	}

	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment);

	return Texture(gl::TEXTURE_2D, gl_tid);
}

Texture TextureLoader::LoadImage2D(uint32_t imgWidth, uint32_t imgHeight, unsigned int gl_internalFormat) {

	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};

	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1);
	gl::BindTexture(gl::TEXTURE_2D, gl_tid);

	//Create the empty texture surface
	if (weave::opengl::HasTextureStorageExtension()) {
		//Prepare the storage for the texture
		GLsizei levels = Texture::MaxMipLevels(imgWidth, imgHeight);
		//gl::TexStorage2D(gl::TEXTURE_2D, levels, gl_internalFormat, width, height);
		gl::TextureStorage2D(gl_tid, levels, gl_internalFormat, imgWidth, imgHeight);
		float black[4] = { 0,0,0,0 };
		auto gl_dataFormat = GetGLDataFormatFromInternalFormat(gl_internalFormat);
		gl::ClearTexImage(gl_tid, 0, gl_dataFormat, gl::UNSIGNED_BYTE, black);
	}
	else {
		//Set the internal data format to some suitable value. GL seems to need a valid value even if there's no data sent
		auto gl_dataFormat = GetGLDataFormatFromInternalFormat(gl_internalFormat);
		//Upload the data
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl_internalFormat, imgWidth, imgHeight, 0, gl_dataFormat, gl::UNSIGNED_BYTE, nullptr);
	}

	gl::GenerateMipmap(gl::TEXTURE_2D);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment);


	return Texture(gl::TEXTURE_2D, gl_tid);
}

Texture TextureLoader::LoadImage2DMultiSample(uint32_t imgWidth, uint32_t imgHeight, uint32_t samples, unsigned int gl_internalFormat, bool fixedSamples) {
	
	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};
	
	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1);
	gl::BindTexture(gl::TEXTURE_2D_MULTISAMPLE, gl_tid);

	//Create the empty texture surface
	if (weave::opengl::HasTexStorageExtension()) {
		gl::TexStorage2DMultisample(gl::TEXTURE_2D_MULTISAMPLE, samples, gl_internalFormat, imgWidth, imgHeight, GLboolean(fixedSamples));
	}
	else {
		gl::TexImage2DMultisample(gl::TEXTURE_2D_MULTISAMPLE, samples, gl_internalFormat, imgWidth, imgHeight, GLboolean(fixedSamples));
	}

	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment);

	return Texture(gl::TEXTURE_2D_MULTISAMPLE, gl_tid);
}

Texture TextureLoader::LoadImage2DArray(ImageData const& img, unsigned int gl_compressionFlag) {

	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};
	
	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1);
	gl::BindTexture(gl::TEXTURE_2D_ARRAY, gl_tid);

	auto surface = img.GetSurface(0, 0);
	auto numLayers = img.GetNumLayers();

	//Loop through all the mipmaps, storing each level in the buffer and uploading it after
	for (uint32_t mipmap = 0; mipmap < img.GetNumMipmaps(); ++mipmap) {
		//Transfer the texture data via a PBO. All textures are stacked in the same buffer and then uploaded via pixel transfers
		GLuint pixelBufferID{};
		gl::GenBuffers(1, &pixelBufferID);
		gl::BindBuffer(gl::PIXEL_UNPACK_BUFFER, pixelBufferID);
		gl::BufferData(gl::PIXEL_UNPACK_BUFFER, img.GetByteSize((unsigned int)mipmap) * numLayers, nullptr, gl::STREAM_DRAW);

		//Dump the current mipmap into the buffer for all the layers
		for (uint32_t i = 0; i < img.GetNumLayers(); ++i) {
			surface = img.GetSurface(i, mipmap);
			gl::BufferSubData(gl::PIXEL_UNPACK_BUFFER, surface.data.size() * i, surface.data.size(), surface.data.data());
		}

		//Upload the data in the pixel buffer
		auto internalFormat = img.GetGLDataType();
		if (internalFormat == gl::COMPRESSED_RGB_S3TC_DXT1_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT5_EXT) {
			gl::CompressedTexImage3D(gl::TEXTURE_2D_ARRAY, (GLint)mipmap, internalFormat, surface.width, surface.height, (GLsizei)numLayers, 0, (GLsizei)surface.data.size() * (GLsizei)numLayers, nullptr);
		}
		else {
			auto compressionFlag = CompressionCodeAdjust(gl_compressionFlag, internalFormat);
			gl::TexImage3D(gl::TEXTURE_2D_ARRAY, (GLint)mipmap, compressionFlag, surface.width, surface.height, (GLsizei)numLayers, 0, internalFormat, gl::UNSIGNED_BYTE, nullptr);
		}

		//Delete the buffer
		gl::BindBuffer(gl::PIXEL_UNPACK_BUFFER, 0);
		gl::DeleteBuffers(1, &pixelBufferID);
	}
	
	if (img.GetNumMipmaps() == 1) {
		gl::GenerateMipmap(gl::TEXTURE_2D_ARRAY);
	}

	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment);

	return Texture(gl::TEXTURE_2D_ARRAY, gl_tid);
}

Texture TextureLoader::LoadImage2DArray(uint32_t imgWidth, uint32_t imgHeight, uint32_t layerCount, unsigned int gl_compressionFlag) {
	
	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};
	
	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1);
	gl::BindTexture(gl::TEXTURE_2D_ARRAY, gl_tid);

	//GL requires the data format of the provided data for the texture to be defined even if we are creating an empty texture. 
	auto gl_dataFormat = GetGLDataFormatFromInternalFormat(gl_compressionFlag);

	//Create the empty texture surface
	gl::TexImage3D(gl::TEXTURE_2D_ARRAY, 0, gl_compressionFlag, imgWidth, imgHeight, layerCount, 0, gl_dataFormat, gl::UNSIGNED_BYTE, nullptr);

	gl::GenerateMipmap(gl::TEXTURE_2D_ARRAY);

	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment);

	return Texture(gl::TEXTURE_2D_ARRAY, gl_tid);
}

Texture TextureLoader::LoadImage3D(ImageData const& img, unsigned int gl_compressionFlag) {
	
	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};
	
	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1);
	gl::BindTexture(gl::TEXTURE_3D, gl_tid);

	auto surface = img.GetSurface(0, 0);
	
	//Upload all images to GL
	auto internalFormat = img.GetGLDataType();
	auto compressionFlag = CompressionCodeAdjust(gl_compressionFlag, internalFormat);
	
	GLuint pixelBufferID{};

	if (internalFormat == gl::COMPRESSED_RGB_S3TC_DXT1_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT5_EXT) {
		gl::CompressedTexImage3D(gl::TEXTURE_3D, 0, internalFormat, surface.width, surface.height, surface.depth, 0, (GLsizei)surface.data.size(), nullptr);
	}
	else {
		gl::TexImage3D(gl::TEXTURE_3D, 0, compressionFlag, surface.width, surface.height, surface.depth, 0, internalFormat, gl::UNSIGNED_BYTE, nullptr);
	}

	for (uint32_t mip = 0; mip < img.GetNumMipmaps(); ++mip) {
		surface = img.GetSurface(0, mip);

		for (uint32_t i = 0; i < surface.depth; ++i) {
			gl::GenBuffers(1, &pixelBufferID);
			gl::BindBuffer(gl::PIXEL_UNPACK_BUFFER, pixelBufferID);
			gl::BufferData(gl::PIXEL_UNPACK_BUFFER, (GLsizei)surface.data.size(), surface.data.data(), gl::STREAM_DRAW);

			if (internalFormat == gl::COMPRESSED_RGB_S3TC_DXT1_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT5_EXT) {
				gl::CompressedTexSubImage3D(gl::TEXTURE_3D, mip, 0, 0, i, surface.width, surface.height, 1, internalFormat, (GLsizei)surface.data.size(), nullptr);
			}
			else {
				gl::TexSubImage3D(gl::TEXTURE_3D, mip, 0, 0, i, surface.width, surface.height, 1, internalFormat, gl::UNSIGNED_BYTE, nullptr);
			}

			gl::DeleteBuffers(1, &pixelBufferID);
		}
	}

	if (img.GetNumMipmaps() == 1) {
		gl::GenerateMipmap(gl::TEXTURE_3D);
	}

	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment);

	return Texture(gl::TEXTURE_3D, gl_tid);
}

Texture TextureLoader::LoadImage3D(uint32_t imgWidth, uint32_t imgHeight, uint32_t imgDepth, unsigned int gl_internalFormat) {
	
	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};
	
	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1); 
	gl::BindTexture(gl::TEXTURE_3D, gl_tid);

	//Create the empty texture surface
	if (weave::opengl::HasTextureStorageExtension()) {
		//Prepare the storage for the texture
		GLsizei levels = Texture::MaxMipLevels(imgWidth, imgHeight);
		//gl::TexStorage3D(gl::TEXTURE_3D, levels, gl_internalFormat, imgWidth, imgHeight, imgDepth);
		gl::TextureStorage3D(gl_tid, levels, gl_internalFormat, imgWidth, imgHeight, imgDepth);
		float black[4] = { 0,0,0,0 };
		auto gl_dataFormat = GetGLDataFormatFromInternalFormat(gl_internalFormat);
		gl::ClearTexImage(gl_tid, 0, gl_dataFormat, gl::UNSIGNED_BYTE, black);
	}
	else {
		//Set the internal data format to some suitable value. GL seems to need a valid value even if there's no data sent
		auto gl_dataFormat = GetGLDataFormatFromInternalFormat(gl_internalFormat);
		//Create the empty texture surface
		gl::TexImage3D(gl::TEXTURE_3D, 0, gl_internalFormat, imgWidth, imgHeight, imgDepth, 0, gl_dataFormat, gl::UNSIGNED_BYTE, nullptr);
	}
	
	gl::GenerateMipmap(gl::TEXTURE_3D);

	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment); 

	return Texture(gl::TEXTURE_3D, gl_tid);
}

Texture TextureLoader::LoadImageCubemap(ImageData const& img, unsigned int gl_compressionFlag) {
	const unsigned int gl_cubetargets[] = { gl::TEXTURE_CUBE_MAP_POSITIVE_X, gl::TEXTURE_CUBE_MAP_NEGATIVE_X,
										   gl::TEXTURE_CUBE_MAP_POSITIVE_Y, gl::TEXTURE_CUBE_MAP_NEGATIVE_Y,
										   gl::TEXTURE_CUBE_MAP_POSITIVE_Z, gl::TEXTURE_CUBE_MAP_NEGATIVE_Z };

	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};
	
	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1); 
	gl::BindTexture(gl::TEXTURE_CUBE_MAP, gl_tid);

	auto surface = img.GetSurface(0, 0);

	gl::Enable(gl::TEXTURE_CUBE_MAP_SEAMLESS);

	//Upload all 6 images to GL
	auto internalFormat = img.GetGLDataType();
	auto compressionFlag = CompressionCodeAdjust(gl_compressionFlag, internalFormat);

	GLuint pixelBufferID;

	for (unsigned int i = 0; i < 6; ++i) {
		for (unsigned int j = 0; j < img.GetNumMipmaps(); ++j) {
			surface = img.GetSurface(std::min(i, img.GetNumLayers() - 1), j);

			gl::GenBuffers(1, &pixelBufferID);
			gl::BindBuffer(gl::PIXEL_UNPACK_BUFFER, pixelBufferID);
			gl::BufferData(gl::PIXEL_UNPACK_BUFFER, (GLsizei)surface.data.size(), surface.data.data(), gl::STREAM_DRAW);
			
			if (internalFormat == gl::COMPRESSED_RGB_S3TC_DXT1_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT || internalFormat == gl::COMPRESSED_RGBA_S3TC_DXT5_EXT) {
				gl::CompressedTexImage2D(gl_cubetargets[i], j, internalFormat, surface.width, surface.height, 0, (GLsizei)surface.data.size(), nullptr);
			}
			else {
				gl::TexImage2D(gl_cubetargets[i], j, compressionFlag, surface.width, surface.height, 0, (GLsizei)surface.data.size(), gl::UNSIGNED_BYTE, nullptr);
			}

			gl::BindBuffer(gl::PIXEL_UNPACK_BUFFER, 0);
			gl::DeleteBuffers(1, &pixelBufferID);
		}

	}

	if (img.GetNumMipmaps() == 1) {
		gl::GenerateMipmap(gl::TEXTURE_CUBE_MAP);
	}

	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment); 
	
	return Texture(gl::TEXTURE_CUBE_MAP, gl_tid);
}

Texture TextureLoader::LoadImageCubemap(uint32_t imgWidth, uint32_t imgHeight, unsigned int gl_compressionFlag) {
	
	GLuint gl_tid;
	gl::GenTextures(1, &gl_tid);

	if (!gl_tid)
		return {};
	
	//Set the pixel storage alignment to 1
	int tmp_alignment = 1;
	gl::GetIntegerv(gl::UNPACK_ALIGNMENT, &tmp_alignment);
	gl::PixelStorei(gl::UNPACK_ALIGNMENT, 1); 
	gl::BindTexture(gl::TEXTURE_CUBE_MAP, gl_tid);

	gl::Enable(gl::TEXTURE_CUBE_MAP_SEAMLESS);

	//Prepare the storage for the texture maps
	GLsizei numLevels = 1;
	numLevels += (GLsizei)std::floor(std::log2(std::max(imgWidth, imgHeight)));

	gl::TexStorage2D(gl::TEXTURE_CUBE_MAP, numLevels, gl_compressionFlag, imgWidth, imgHeight);

	float black[4] = { 0,0,0,0 };
	gl::ClearTexImage(gl_tid, 0, gl_compressionFlag, gl::UNSIGNED_BYTE, black);

	gl::GenerateMipmap(gl::TEXTURE_CUBE_MAP);

	gl::PixelStorei(gl::UNPACK_ALIGNMENT, tmp_alignment); 
	
	return Texture(gl::TEXTURE_CUBE_MAP, gl_tid);
}



unsigned int TextureLoader::CompressionCodeAdjust(unsigned int gl_target_code, unsigned int gl_image_code) {
	if(gl_target_code == gl_image_code)
		return gl_target_code;
	
	if(gl_target_code == 0) {
		if(gl_image_code == gl::RGBA)
			return gl::RGBA8;
		else if(gl_image_code == gl::RGB)
			return gl::RGB8;
		else if(gl_image_code == gl::RED)
			return gl::R8;
		else if(gl_image_code == gl::BGRA)
			return gl::RGBA8;
		else if(gl_image_code == gl::BGR)
			return gl::RGB8;
		if(gl_image_code == gl::RG)
			return gl::RG8;
		else
			return gl_image_code;
	}

	if(gl_target_code == gl::COMPRESSED_SRGB_S3TC_DXT1_EXT || gl_target_code == gl::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT || 
	   gl_target_code == gl::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT || gl_target_code == gl::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT) {
			
		if(gl_image_code == gl::COMPRESSED_RGB_S3TC_DXT1_EXT){ 
			return gl::COMPRESSED_SRGB_S3TC_DXT1_EXT;
		}
	
		if(gl_image_code == gl::COMPRESSED_RGBA_S3TC_DXT1_EXT){
			return gl::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
		} 

		if(gl_image_code == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT){
			return gl::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
		} 

		if(gl_image_code == gl::COMPRESSED_RGBA_S3TC_DXT5_EXT){
			return gl::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
		} 
	}

	if((gl_target_code == gl::COMPRESSED_RGB_S3TC_DXT1_EXT || gl_target_code == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT || 
	    gl_target_code == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT || gl_target_code == gl::COMPRESSED_RGBA_S3TC_DXT5_EXT) 
	   &&
	   (gl_image_code == gl::COMPRESSED_RGB_S3TC_DXT1_EXT || gl_image_code == gl::COMPRESSED_RGBA_S3TC_DXT1_EXT || 
        gl_image_code == gl::COMPRESSED_RGBA_S3TC_DXT3_EXT || gl_image_code == gl::COMPRESSED_RGBA_S3TC_DXT5_EXT)){ 
		
		return gl_image_code;
	}
	
	
	return gl_target_code;
}

//Returns the equivalent data format GL enum based on the supplied internalFormat enum. This is used for gl::TexImage* calls to automatically set the format paramter based on the internalFormat parameter
//when no user data is supplied.
GLenum TextureLoader::GetGLDataFormatFromInternalFormat(GLenum internalFormat) {
	//GL requires the data format of the provided data for the texture to be defined even if we are creating an empty texture.
	switch(internalFormat) {
		case gl::DEPTH_COMPONENT:
		case gl::DEPTH_COMPONENT16:
		case gl::DEPTH_COMPONENT24:
		case gl::DEPTH_COMPONENT32:
		case gl::DEPTH_COMPONENT32F:
		case gl::DEPTH24_STENCIL8:
		case gl::DEPTH32F_STENCIL8:
			return gl::DEPTH_COMPONENT;
		case gl::RGB10_A2UI:
		case gl::R8I:
		case gl::R8UI:
		case gl::R16I:
		case gl::R16UI:
		case gl::R32I:
		case gl::R32UI:
		case gl::RG8I:
		case gl::RG8UI:
		case gl::RG16I:
		case gl::RG16UI:
		case gl::RG32I:
		case gl::RG32UI:
		case gl::RGB8I:
		case gl::RGB8UI:
		case gl::RGB16I:
		case gl::RGB16UI:
		case gl::RGB32I:
		case gl::RGB32UI:
		case gl::RGBA8I:
		case gl::RGBA8UI:
		case gl::RGBA16I:
		case gl::RGBA16UI:
		case gl::RGBA32I:
		case gl::RGBA32UI:
			return gl::RGBA_INTEGER;
		default:
			return gl::RGBA;
	}
	
}

