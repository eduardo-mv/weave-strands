/*
================================================================================================

This file was modified for usage on Weave into namespaced C++ version.
The original license text and information is bellow.

Description	:	Vulkan format properties and conversion from OpenGL.
Author		:	J.M.P. van Waveren
Date		:	07/17/2016
Language	:	C99
Format		:	Real tabs with the tab size equal to 4 spaces.
Copyright	:	Copyright (c) 2016 Oculus VR, LLC. All Rights reserved.


LICENSE
=======

Copyright (c) 2016 Oculus VR, LLC.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


DESCRIPTION
===========

This header implements several support routines to convert OpenGL formats/types
to Vulkan formats. These routines are particularly useful for loading file
formats that store OpenGL formats/types such as KTX and glTF.

The functions in this header file convert the format, internalFormat and type
that are used as parameters to the following OpenGL functions:

void glTexImage2D( GLenum target, GLint level, GLint internalFormat,
	GLsizei width, GLsizei height, GLint border,
	GLenum format, GLenum type, const GLvoid * data );
void glTexImage3D( GLenum target, GLint level, GLint internalFormat,
	GLsizei width, GLsizei height, GLsizei depth, GLint border,
	GLenum format, GLenum type, const GLvoid * data );
void glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLint border,
	GLsizei imageSize, const GLvoid * data );
void glCompressedTexImage3D( GLenum target, GLint level, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth, GLint border,
	GLsizei imageSize, const GLvoid * data );
void glTexStorage2D( GLenum target, GLsizei levels, GLenum internalformat,
	GLsizei width, GLsizei height );
void glTexStorage3D( GLenum target, GLsizei levels, GLenum internalformat,
	GLsizei width, GLsizei height, GLsizei depth );
void glVertexAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized,
	GLsizei stride, const GLvoid * pointer);


IMPLEMENTATION
==============

This file does not include OpenGL / OpenGL ES headers because:

  1. Including OpenGL / OpenGL ES headers is platform dependent and
     may require a separate installation of an OpenGL SDK.
  2. The OpenGL format/type constants are the same between extensions and core.
  3. The OpenGL format/type constants are the same between OpenGL and OpenGL ES.
  4. File formats like KTX and glTF may use OpenGL formats and types that
     are not supported by the OpenGL implementation on the platform but are
     supported by the Vulkan implementation.


ENTRY POINTS
============

static inline VkFormat vkGetFormatFromOpenGLFormat( const GLenum format, const GLenum type );
static inline VkFormat vkGetFormatFromOpenGLType( const GLenum type, const GLuint numComponents, const GLboolean normalized );
static inline VkFormat vkGetFormatFromOpenGLInternalFormat( const GLenum internalFormat );
static inline void vkGetFormatSize( const VkFormat format, FormatSize * pFormatSize );

================================================================================================
*/

#pragma once

#include "../gl/gl_format.h"
#include <cassert>

namespace weave {
namespace vulkan {

	//Extracted from Vulkan
	enum class ImageFormat : unsigned int {
		UNDEFINED = 0,
		R4G4_UNORM_PACK8 = 1,
		R4G4B4A4_UNORM_PACK16 = 2,
		B4G4R4A4_UNORM_PACK16 = 3,
		R5G6B5_UNORM_PACK16 = 4,
		B5G6R5_UNORM_PACK16 = 5,
		R5G5B5A1_UNORM_PACK16 = 6,
		B5G5R5A1_UNORM_PACK16 = 7,
		A1R5G5B5_UNORM_PACK16 = 8,
		R8_UNORM = 9,
		R8_SNORM = 10,
		R8_USCALED = 11,
		R8_SSCALED = 12,
		R8_UINT = 13,
		R8_SINT = 14,
		R8_SRGB = 15,
		R8G8_UNORM = 16,
		R8G8_SNORM = 17,
		R8G8_USCALED = 18,
		R8G8_SSCALED = 19,
		R8G8_UINT = 20,
		R8G8_SINT = 21,
		R8G8_SRGB = 22,
		R8G8B8_UNORM = 23,
		R8G8B8_SNORM = 24,
		R8G8B8_USCALED = 25,
		R8G8B8_SSCALED = 26,
		R8G8B8_UINT = 27,
		R8G8B8_SINT = 28,
		R8G8B8_SRGB = 29,
		B8G8R8_UNORM = 30,
		B8G8R8_SNORM = 31,
		B8G8R8_USCALED = 32,
		B8G8R8_SSCALED = 33,
		B8G8R8_UINT = 34,
		B8G8R8_SINT = 35,
		B8G8R8_SRGB = 36,
		R8G8B8A8_UNORM = 37,
		R8G8B8A8_SNORM = 38,
		R8G8B8A8_USCALED = 39,
		R8G8B8A8_SSCALED = 40,
		R8G8B8A8_UINT = 41,
		R8G8B8A8_SINT = 42,
		R8G8B8A8_SRGB = 43,
		B8G8R8A8_UNORM = 44,
		B8G8R8A8_SNORM = 45,
		B8G8R8A8_USCALED = 46,
		B8G8R8A8_SSCALED = 47,
		B8G8R8A8_UINT = 48,
		B8G8R8A8_SINT = 49,
		B8G8R8A8_SRGB = 50,
		A8B8G8R8_UNORM_PACK32 = 51,
		A8B8G8R8_SNORM_PACK32 = 52,
		A8B8G8R8_USCALED_PACK32 = 53,
		A8B8G8R8_SSCALED_PACK32 = 54,
		A8B8G8R8_UINT_PACK32 = 55,
		A8B8G8R8_SINT_PACK32 = 56,
		A8B8G8R8_SRGB_PACK32 = 57,
		A2R10G10B10_UNORM_PACK32 = 58,
		A2R10G10B10_SNORM_PACK32 = 59,
		A2R10G10B10_USCALED_PACK32 = 60,
		A2R10G10B10_SSCALED_PACK32 = 61,
		A2R10G10B10_UINT_PACK32 = 62,
		A2R10G10B10_SINT_PACK32 = 63,
		A2B10G10R10_UNORM_PACK32 = 64,
		A2B10G10R10_SNORM_PACK32 = 65,
		A2B10G10R10_USCALED_PACK32 = 66,
		A2B10G10R10_SSCALED_PACK32 = 67,
		A2B10G10R10_UINT_PACK32 = 68,
		A2B10G10R10_SINT_PACK32 = 69,
		R16_UNORM = 70,
		R16_SNORM = 71,
		R16_USCALED = 72,
		R16_SSCALED = 73,
		R16_UINT = 74,
		R16_SINT = 75,
		R16_SFLOAT = 76,
		R16G16_UNORM = 77,
		R16G16_SNORM = 78,
		R16G16_USCALED = 79,
		R16G16_SSCALED = 80,
		R16G16_UINT = 81,
		R16G16_SINT = 82,
		R16G16_SFLOAT = 83,
		R16G16B16_UNORM = 84,
		R16G16B16_SNORM = 85,
		R16G16B16_USCALED = 86,
		R16G16B16_SSCALED = 87,
		R16G16B16_UINT = 88,
		R16G16B16_SINT = 89,
		R16G16B16_SFLOAT = 90,
		R16G16B16A16_UNORM = 91,
		R16G16B16A16_SNORM = 92,
		R16G16B16A16_USCALED = 93,
		R16G16B16A16_SSCALED = 94,
		R16G16B16A16_UINT = 95,
		R16G16B16A16_SINT = 96,
		R16G16B16A16_SFLOAT = 97,
		R32_UINT = 98,
		R32_SINT = 99,
		R32_SFLOAT = 100,
		R32G32_UINT = 101,
		R32G32_SINT = 102,
		R32G32_SFLOAT = 103,
		R32G32B32_UINT = 104,
		R32G32B32_SINT = 105,
		R32G32B32_SFLOAT = 106,
		R32G32B32A32_UINT = 107,
		R32G32B32A32_SINT = 108,
		R32G32B32A32_SFLOAT = 109,
		R64_UINT = 110,
		R64_SINT = 111,
		R64_SFLOAT = 112,
		R64G64_UINT = 113,
		R64G64_SINT = 114,
		R64G64_SFLOAT = 115,
		R64G64B64_UINT = 116,
		R64G64B64_SINT = 117,
		R64G64B64_SFLOAT = 118,
		R64G64B64A64_UINT = 119,
		R64G64B64A64_SINT = 120,
		R64G64B64A64_SFLOAT = 121,
		B10G11R11_UFLOAT_PACK32 = 122,
		E5B9G9R9_UFLOAT_PACK32 = 123,
		D16_UNORM = 124,
		X8_D24_UNORM_PACK32 = 125,
		D32_SFLOAT = 126,
		S8_UINT = 127,
		D16_UNORM_S8_UINT = 128,
		D24_UNORM_S8_UINT = 129,
		D32_SFLOAT_S8_UINT = 130,
		BC1_RGB_UNORM_BLOCK = 131,
		BC1_RGB_SRGB_BLOCK = 132,
		BC1_RGBA_UNORM_BLOCK = 133,
		BC1_RGBA_SRGB_BLOCK = 134,
		BC2_UNORM_BLOCK = 135,
		BC2_SRGB_BLOCK = 136,
		BC3_UNORM_BLOCK = 137,
		BC3_SRGB_BLOCK = 138,
		BC4_UNORM_BLOCK = 139,
		BC4_SNORM_BLOCK = 140,
		BC5_UNORM_BLOCK = 141,
		BC5_SNORM_BLOCK = 142,
		BC6H_UFLOAT_BLOCK = 143,
		BC6H_SFLOAT_BLOCK = 144,
		BC7_UNORM_BLOCK = 145,
		BC7_SRGB_BLOCK = 146,
		ETC2_R8G8B8_UNORM_BLOCK = 147,
		ETC2_R8G8B8_SRGB_BLOCK = 148,
		ETC2_R8G8B8A1_UNORM_BLOCK = 149,
		ETC2_R8G8B8A1_SRGB_BLOCK = 150,
		ETC2_R8G8B8A8_UNORM_BLOCK = 151,
		ETC2_R8G8B8A8_SRGB_BLOCK = 152,
		EAC_R11_UNORM_BLOCK = 153,
		EAC_R11_SNORM_BLOCK = 154,
		EAC_R11G11_UNORM_BLOCK = 155,
		EAC_R11G11_SNORM_BLOCK = 156,
		ASTC_4x4_UNORM_BLOCK = 157,
		ASTC_4x4_SRGB_BLOCK = 158,
		ASTC_5x4_UNORM_BLOCK = 159,
		ASTC_5x4_SRGB_BLOCK = 160,
		ASTC_5x5_UNORM_BLOCK = 161,
		ASTC_5x5_SRGB_BLOCK = 162,
		ASTC_6x5_UNORM_BLOCK = 163,
		ASTC_6x5_SRGB_BLOCK = 164,
		ASTC_6x6_UNORM_BLOCK = 165,
		ASTC_6x6_SRGB_BLOCK = 166,
		ASTC_8x5_UNORM_BLOCK = 167,
		ASTC_8x5_SRGB_BLOCK = 168,
		ASTC_8x6_UNORM_BLOCK = 169,
		ASTC_8x6_SRGB_BLOCK = 170,
		ASTC_8x8_UNORM_BLOCK = 171,
		ASTC_8x8_SRGB_BLOCK = 172,
		ASTC_10x5_UNORM_BLOCK = 173,
		ASTC_10x5_SRGB_BLOCK = 174,
		ASTC_10x6_UNORM_BLOCK = 175,
		ASTC_10x6_SRGB_BLOCK = 176,
		ASTC_10x8_UNORM_BLOCK = 177,
		ASTC_10x8_SRGB_BLOCK = 178,
		ASTC_10x10_UNORM_BLOCK = 179,
		ASTC_10x10_SRGB_BLOCK = 180,
		ASTC_12x10_UNORM_BLOCK = 181,
		ASTC_12x10_SRGB_BLOCK = 182,
		ASTC_12x12_UNORM_BLOCK = 183,
		ASTC_12x12_SRGB_BLOCK = 184,
		G8B8G8R8_422_UNORM = 1000156000,
		B8G8R8G8_422_UNORM = 1000156001,
		G8_B8_R8_3PLANE_420_UNORM = 1000156002,
		G8_B8R8_2PLANE_420_UNORM = 1000156003,
		G8_B8_R8_3PLANE_422_UNORM = 1000156004,
		G8_B8R8_2PLANE_422_UNORM = 1000156005,
		G8_B8_R8_3PLANE_444_UNORM = 1000156006,
		R10X6_UNORM_PACK16 = 1000156007,
		R10X6G10X6_UNORM_2PACK16 = 1000156008,
		R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
		G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
		B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
		G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
		G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
		G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
		G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
		G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
		R12X4_UNORM_PACK16 = 1000156017,
		R12X4G12X4_UNORM_2PACK16 = 1000156018,
		R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
		G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
		B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
		G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
		G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
		G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
		G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
		G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
		G16B16G16R16_422_UNORM = 1000156027,
		B16G16R16G16_422_UNORM = 1000156028,
		G16_B16_R16_3PLANE_420_UNORM = 1000156029,
		G16_B16R16_2PLANE_420_UNORM = 1000156030,
		G16_B16_R16_3PLANE_422_UNORM = 1000156031,
		G16_B16R16_2PLANE_422_UNORM = 1000156032,
		G16_B16_R16_3PLANE_444_UNORM = 1000156033,
		PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
		PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
		PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
		PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
		PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
		PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
		PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
		PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
		G8B8G8R8_422_UNORM_KHR = G8B8G8R8_422_UNORM,
		B8G8R8G8_422_UNORM_KHR = B8G8R8G8_422_UNORM,
		G8_B8_R8_3PLANE_420_UNORM_KHR = G8_B8_R8_3PLANE_420_UNORM,
		G8_B8R8_2PLANE_420_UNORM_KHR = G8_B8R8_2PLANE_420_UNORM,
		G8_B8_R8_3PLANE_422_UNORM_KHR = G8_B8_R8_3PLANE_422_UNORM,
		G8_B8R8_2PLANE_422_UNORM_KHR = G8_B8R8_2PLANE_422_UNORM,
		G8_B8_R8_3PLANE_444_UNORM_KHR = G8_B8_R8_3PLANE_444_UNORM,
		R10X6_UNORM_PACK16_KHR = R10X6_UNORM_PACK16,
		R10X6G10X6_UNORM_2PACK16_KHR = R10X6G10X6_UNORM_2PACK16,
		R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR = R10X6G10X6B10X6A10X6_UNORM_4PACK16,
		G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR = G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
		B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR = B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
		G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
		G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
		G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
		G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
		G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
		R12X4_UNORM_PACK16_KHR = R12X4_UNORM_PACK16,
		R12X4G12X4_UNORM_2PACK16_KHR = R12X4G12X4_UNORM_2PACK16,
		R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR = R12X4G12X4B12X4A12X4_UNORM_4PACK16,
		G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR = G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
		B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR = B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
		G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
		G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
		G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
		G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
		G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
		G16B16G16R16_422_UNORM_KHR = G16B16G16R16_422_UNORM,
		B16G16R16G16_422_UNORM_KHR = B16G16R16G16_422_UNORM,
		G16_B16_R16_3PLANE_420_UNORM_KHR = G16_B16_R16_3PLANE_420_UNORM,
		G16_B16R16_2PLANE_420_UNORM_KHR = G16_B16R16_2PLANE_420_UNORM,
		G16_B16_R16_3PLANE_422_UNORM_KHR = G16_B16_R16_3PLANE_422_UNORM,
		G16_B16R16_2PLANE_422_UNORM_KHR = G16_B16R16_2PLANE_422_UNORM,
		G16_B16_R16_3PLANE_444_UNORM_KHR = G16_B16_R16_3PLANE_444_UNORM,
		BEGIN_RANGE = UNDEFINED,
		END_RANGE = ASTC_12x12_SRGB_BLOCK,
		RANGE_SIZE = (ASTC_12x12_SRGB_BLOCK - UNDEFINED + 1),
		MAX_ENUM = 0x7FFFFFFF
	};


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif
static inline ImageFormat vkGetFormatFromOpenGLFormat( const weave::opengl::GlFormat format, const weave::opengl::GlFormat type )
{
	switch ( type )
	{
		//
		// 8 bits per component
		//
		case weave::opengl::GlFormat::UNSIGNED_BYTE:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return  ImageFormat::R8_UNORM;
				case weave::opengl::GlFormat::RG:					return  ImageFormat::R8G8_UNORM;
				case weave::opengl::GlFormat::RGB:					return  ImageFormat::R8G8B8_UNORM;
				case weave::opengl::GlFormat::BGR:					return  ImageFormat::B8G8R8_UNORM;
				case weave::opengl::GlFormat::RGBA:					return  ImageFormat::R8G8B8A8_UNORM;
				case weave::opengl::GlFormat::BGRA:					return  ImageFormat::B8G8R8A8_UNORM;
				case weave::opengl::GlFormat::RED_INTEGER:			return  ImageFormat::R8_UINT;
				case weave::opengl::GlFormat::RG_INTEGER:			return  ImageFormat::R8G8_UINT;
				case weave::opengl::GlFormat::RGB_INTEGER:			return  ImageFormat::R8G8B8_UINT;
				case weave::opengl::GlFormat::BGR_INTEGER:			return  ImageFormat::B8G8R8_UINT;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return  ImageFormat::R8G8B8A8_UINT;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return  ImageFormat::B8G8R8A8_UINT;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return  ImageFormat::S8_UINT;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return  ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return  ImageFormat::UNDEFINED;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::BYTE:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R8_SNORM;
				case weave::opengl::GlFormat::RG:					return ImageFormat::R8G8_SNORM;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R8G8B8_SNORM;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::B8G8R8_SNORM;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R8G8B8A8_SNORM;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::B8G8R8A8_SNORM;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::R8_SINT;
				case weave::opengl::GlFormat::RG_INTEGER:			return ImageFormat::R8G8_SINT;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::R8G8B8_SINT;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::B8G8R8_SINT;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::R8G8B8A8_SINT;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::B8G8R8A8_SINT;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::UNDEFINED;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}

		//
		// 16 bits per component
		//
		case weave::opengl::GlFormat::UNSIGNED_SHORT:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R16_UNORM;
				case weave::opengl::GlFormat::RG:					return ImageFormat::R16G16_UNORM;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R16G16B16_UNORM;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R16G16B16A16_UNORM;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::R16_UINT;
				case weave::opengl::GlFormat::RG_INTEGER:			return ImageFormat::R16G16_UINT;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::R16G16B16_UINT;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::R16G16B16A16_UINT;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::D16_UNORM;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::D16_UNORM_S8_UINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::SHORT:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R16_SNORM;
				case weave::opengl::GlFormat::RG:						return ImageFormat::R16G16_SNORM;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R16G16B16_SNORM;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R16G16B16A16_SNORM;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::R16_SINT;
				case weave::opengl::GlFormat::RG_INTEGER:				return ImageFormat::R16G16_SINT;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::R16G16B16_SINT;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::R16G16B16A16_SINT;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::UNDEFINED;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::HALF_FLOAT:
		case weave::opengl::GlFormat::HALF_FLOAT_OES:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R16_SFLOAT;
				case weave::opengl::GlFormat::RG:						return ImageFormat::R16G16_SFLOAT;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R16G16B16_SFLOAT;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R16G16B16A16_SFLOAT;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RG_INTEGER:				return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::UNDEFINED;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}

		//
		// 32 bits per component
		//
		case weave::opengl::GlFormat::UNSIGNED_INT:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R32_UINT;
				case weave::opengl::GlFormat::RG:						return ImageFormat::R32G32_UINT;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R32G32B32_UINT;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R32G32B32A32_UINT;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::R32_UINT;
				case weave::opengl::GlFormat::RG_INTEGER:				return ImageFormat::R32G32_UINT;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::R32G32B32_UINT;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::R32G32B32A32_UINT;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::X8_D24_UNORM_PACK32;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::D24_UNORM_S8_UINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::INT:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R32_SINT;
				case weave::opengl::GlFormat::RG:						return ImageFormat::R32G32_SINT;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R32G32B32_SINT;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R32G32B32A32_SINT;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::R32_SINT;
				case weave::opengl::GlFormat::RG_INTEGER:				return ImageFormat::R32G32_SINT;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::R32G32B32_SINT;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::R32G32B32A32_SINT;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::UNDEFINED;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::FLOAT:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R32_SFLOAT;
				case weave::opengl::GlFormat::RG:						return ImageFormat::R32G32_SFLOAT;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R32G32B32_SFLOAT;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R32G32B32A32_SFLOAT;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RG_INTEGER:				return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::D32_SFLOAT;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::D32_SFLOAT_S8_UINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}

		//
		// 64 bits per component
		//
		case weave::opengl::GlFormat::UNSIGNED_INT64:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R64_UINT;
				case weave::opengl::GlFormat::RG:						return ImageFormat::R64G64_UINT;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R64G64B64_UINT;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R64G64B64A64_UINT;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RG_INTEGER:				return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::UNDEFINED;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::INT64:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R64_SINT;
				case weave::opengl::GlFormat::RG:						return ImageFormat::R64G64_SINT;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R64G64B64_SINT;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R64G64B64A64_SINT;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::R64_SINT;
				case weave::opengl::GlFormat::RG_INTEGER:				return ImageFormat::R64G64_SINT;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::R64G64B64_SINT;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::R64G64B64A64_SINT;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::UNDEFINED;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::DOUBLE:
		{
			switch ( format )
			{
				case weave::opengl::GlFormat::RED:					return ImageFormat::R64_SFLOAT;
				case weave::opengl::GlFormat::RG:						return ImageFormat::R64G64_SFLOAT;
				case weave::opengl::GlFormat::RGB:					return ImageFormat::R64G64B64_SFLOAT;
				case weave::opengl::GlFormat::BGR:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA:					return ImageFormat::R64G64B64A64_SFLOAT;
				case weave::opengl::GlFormat::BGRA:					return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RED_INTEGER:			return ImageFormat::R64_SFLOAT;
				case weave::opengl::GlFormat::RG_INTEGER:				return ImageFormat::R64G64_SFLOAT;
				case weave::opengl::GlFormat::RGB_INTEGER:			return ImageFormat::R64G64B64_SFLOAT;
				case weave::opengl::GlFormat::BGR_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::RGBA_INTEGER:			return ImageFormat::R64G64B64A64_SFLOAT;
				case weave::opengl::GlFormat::BGRA_INTEGER:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::STENCIL_INDEX:			return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_COMPONENT:		return ImageFormat::UNDEFINED;
				case weave::opengl::GlFormat::DEPTH_STENCIL:			return ImageFormat::UNDEFINED;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}

		//
		// Packed
		//
		case weave::opengl::GlFormat::UNSIGNED_BYTE_3_3_2:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::RGB_INTEGER );
			return ImageFormat::UNDEFINED;
		case weave::opengl::GlFormat::UNSIGNED_BYTE_2_3_3_REV:
			assert( format == weave::opengl::GlFormat::BGR || format == weave::opengl::GlFormat::BGR_INTEGER );
			return ImageFormat::UNDEFINED;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_5_6_5:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::RGB_INTEGER );
			return ImageFormat::R5G6B5_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_5_6_5_REV:
			assert( format == weave::opengl::GlFormat::BGR || format == weave::opengl::GlFormat::BGR_INTEGER );
			return ImageFormat::B5G6R5_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_4_4_4_4:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGRA || format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER );
			return ImageFormat::R4G4B4A4_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_4_4_4_4_REV:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGRA || format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER );
			return ImageFormat::B4G4R4A4_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_5_5_5_1:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGRA || format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER );
			return ImageFormat::R5G5B5A1_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_1_5_5_5_REV:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGRA || format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER );
			return ImageFormat::A1R5G5B5_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_INT_8_8_8_8:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGRA || format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER );
			return ( format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER ) ? ImageFormat::R8G8B8A8_UINT : ImageFormat::R8G8B8A8_UNORM;
		case weave::opengl::GlFormat::UNSIGNED_INT_8_8_8_8_REV:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGRA || format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER );
			return ( format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER ) ? ImageFormat::A8B8G8R8_UINT_PACK32 : ImageFormat::A8B8G8R8_UNORM_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_10_10_10_2:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGRA || format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER );
			return ( format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER ) ? ImageFormat::A2R10G10B10_UINT_PACK32 : ImageFormat::A2R10G10B10_UNORM_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_2_10_10_10_REV:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGRA || format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER );
			return ( format == weave::opengl::GlFormat::RGB_INTEGER || format == weave::opengl::GlFormat::BGRA_INTEGER ) ? ImageFormat::A2B10G10R10_UINT_PACK32 : ImageFormat::A2B10G10R10_UNORM_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_10F_11F_11F_REV:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGR );
			return ImageFormat::B10G11R11_UFLOAT_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_5_9_9_9_REV:
			assert( format == weave::opengl::GlFormat::RGB || format == weave::opengl::GlFormat::BGR );
			return ImageFormat::E5B9G9R9_UFLOAT_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_24_8:
			assert( format == weave::opengl::GlFormat::DEPTH_STENCIL );
			return ImageFormat::D24_UNORM_S8_UINT;
		case weave::opengl::GlFormat::FLOAT_32_UNSIGNED_INT_24_8_REV:
			assert( format == weave::opengl::GlFormat::DEPTH_STENCIL );
			return ImageFormat::D32_SFLOAT_S8_UINT;
	}

	return ImageFormat::UNDEFINED;
}


static inline ImageFormat vkGetFormatFromOpenGLType( const weave::opengl::GlFormat type, const unsigned int numComponents, const bool normalized )
{
	switch ( type )
	{
		//
		// 8 bits per component
		//
		case weave::opengl::GlFormat::UNSIGNED_BYTE:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? ImageFormat::R8_UNORM : ImageFormat::R8_UINT;
				case 2:							return normalized ? ImageFormat::R8G8_UNORM : ImageFormat::R8G8_UINT;
				case 3:							return normalized ? ImageFormat::R8G8B8_UNORM : ImageFormat::R8G8B8_UINT;
				case 4:							return normalized ? ImageFormat::R8G8B8A8_UNORM : ImageFormat::R8G8B8A8_UINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::BYTE:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? ImageFormat::R8_SNORM : ImageFormat::R8_SINT;
				case 2:							return normalized ? ImageFormat::R8G8_SNORM : ImageFormat::R8G8_SINT;
				case 3:							return normalized ? ImageFormat::R8G8B8_SNORM : ImageFormat::R8G8B8_SINT;
				case 4:							return normalized ? ImageFormat::R8G8B8A8_SNORM : ImageFormat::R8G8B8A8_SINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}

		//
		// 16 bits per component
		//
		case weave::opengl::GlFormat::UNSIGNED_SHORT:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? ImageFormat::R16_UNORM : ImageFormat::R16_UINT;
				case 2:							return normalized ? ImageFormat::R16G16_UNORM : ImageFormat::R16G16_UINT;
				case 3:							return normalized ? ImageFormat::R16G16B16_UNORM : ImageFormat::R16G16B16_UINT;
				case 4:							return normalized ? ImageFormat::R16G16B16A16_UNORM : ImageFormat::R16G16B16A16_UINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::SHORT:
		{
			switch ( numComponents )
			{
				case 1:							return normalized ? ImageFormat::R16_SNORM : ImageFormat::R16_SINT;
				case 2:							return normalized ? ImageFormat::R16G16_SNORM : ImageFormat::R16G16_SINT;
				case 3:							return normalized ? ImageFormat::R16G16B16_SNORM : ImageFormat::R16G16B16_SINT;
				case 4:							return normalized ? ImageFormat::R16G16B16A16_SNORM : ImageFormat::R16G16B16A16_SINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::HALF_FLOAT:
		case weave::opengl::GlFormat::HALF_FLOAT_OES:
		{
			switch ( numComponents )
			{
				case 1:							return ImageFormat::R16_SFLOAT;
				case 2:							return ImageFormat::R16G16_SFLOAT;
				case 3:							return ImageFormat::R16G16B16_SFLOAT;
				case 4:							return ImageFormat::R16G16B16A16_SFLOAT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}

		//
		// 32 bits per component
		//
		case weave::opengl::GlFormat::UNSIGNED_INT:
		{
			switch ( numComponents )
			{
				case 1:							return ImageFormat::R32_UINT;
				case 2:							return ImageFormat::R32G32_UINT;
				case 3:							return ImageFormat::R32G32B32_UINT;
				case 4:							return ImageFormat::R32G32B32A32_UINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::INT:
		{
			switch ( numComponents )
			{
				case 1:							return ImageFormat::R32_SINT;
				case 2:							return ImageFormat::R32G32_SINT;
				case 3:							return ImageFormat::R32G32B32_SINT;
				case 4:							return ImageFormat::R32G32B32A32_SINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::FLOAT:
		{
			switch ( numComponents )
			{
				case 1:							return ImageFormat::R32_SFLOAT;
				case 2:							return ImageFormat::R32G32_SFLOAT;
				case 3:							return ImageFormat::R32G32B32_SFLOAT;
				case 4:							return ImageFormat::R32G32B32A32_SFLOAT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}

		//
		// 64 bits per component
		//
		case weave::opengl::GlFormat::UNSIGNED_INT64:
		{
			switch ( numComponents )
			{
				case 1:							return ImageFormat::R64_UINT;
				case 2:							return ImageFormat::R64G64_UINT;
				case 3:							return ImageFormat::R64G64B64_UINT;
				case 4:							return ImageFormat::R64G64B64A64_UINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::INT64:
		{
			switch ( numComponents )
			{
				case 1:							return ImageFormat::R64_SINT;
				case 2:							return ImageFormat::R64G64_SINT;
				case 3:							return ImageFormat::R64G64B64_SINT;
				case 4:							return ImageFormat::R64G64B64A64_SINT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}
		case weave::opengl::GlFormat::DOUBLE:
		{
			switch ( numComponents )
			{
				case 1:							return ImageFormat::R64_SFLOAT;
				case 2:							return ImageFormat::R64G64_SFLOAT;
				case 3:							return ImageFormat::R64G64B64_SFLOAT;
				case 4:							return ImageFormat::R64G64B64A64_SFLOAT;
			default: return ImageFormat::UNDEFINED;
			}
			break;
		}

		//
		// Packed
		//
		case weave::opengl::GlFormat::UNSIGNED_BYTE_3_3_2:			return ImageFormat::UNDEFINED;
		case weave::opengl::GlFormat::UNSIGNED_BYTE_2_3_3_REV:		return ImageFormat::UNDEFINED;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_5_6_5:			return ImageFormat::R5G6B5_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_5_6_5_REV:		return ImageFormat::B5G6R5_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_4_4_4_4:			return ImageFormat::R4G4B4A4_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_4_4_4_4_REV:		return ImageFormat::B4G4R4A4_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_5_5_5_1:			return ImageFormat::R5G5B5A1_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_SHORT_1_5_5_5_REV:		return ImageFormat::A1R5G5B5_UNORM_PACK16;
		case weave::opengl::GlFormat::UNSIGNED_INT_8_8_8_8:			return normalized ? ImageFormat::R8G8B8A8_UNORM : ImageFormat::R8G8B8A8_UINT;
		case weave::opengl::GlFormat::UNSIGNED_INT_8_8_8_8_REV:		return normalized ? ImageFormat::A8B8G8R8_UNORM_PACK32 : ImageFormat::A8B8G8R8_UINT_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_10_10_10_2:		return normalized ? ImageFormat::A2R10G10B10_UNORM_PACK32 : ImageFormat::A2R10G10B10_UINT_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_2_10_10_10_REV:	return normalized ? ImageFormat::A2B10G10R10_UNORM_PACK32 : ImageFormat::A2B10G10R10_UINT_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_10F_11F_11F_REV:	return ImageFormat::B10G11R11_UFLOAT_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_5_9_9_9_REV:		return ImageFormat::E5B9G9R9_UFLOAT_PACK32;
		case weave::opengl::GlFormat::UNSIGNED_INT_24_8:				return ImageFormat::D24_UNORM_S8_UINT;
		case weave::opengl::GlFormat::FLOAT_32_UNSIGNED_INT_24_8_REV:	return ImageFormat::D32_SFLOAT_S8_UINT;
	}

	return ImageFormat::UNDEFINED;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


static inline ImageFormat vkGetFormatFromOpenGLInternalFormat( const weave::opengl::GlFormat internalFormat )
{
	switch ( internalFormat )
	{
		//
		// 8 bits per component
		//
		case weave::opengl::GlFormat::R8:												return ImageFormat::R8_UNORM;					// 1-component, 8-bit unsigned normalized
		case weave::opengl::GlFormat::RG8:											return ImageFormat::R8G8_UNORM;				// 2-component, 8-bit unsigned normalized
		case weave::opengl::GlFormat::RGB8:											return ImageFormat::R8G8B8_UNORM;				// 3-component, 8-bit unsigned normalized
		case weave::opengl::GlFormat::RGBA8:											return ImageFormat::R8G8B8A8_UNORM;			// 4-component, 8-bit unsigned normalized

		case weave::opengl::GlFormat::R8_SNORM:										return ImageFormat::R8_SNORM;					// 1-component, 8-bit signed normalized
		case weave::opengl::GlFormat::RG8_SNORM:										return ImageFormat::R8G8_SNORM;				// 2-component, 8-bit signed normalized
		case weave::opengl::GlFormat::RGB8_SNORM:										return ImageFormat::R8G8B8_SNORM;				// 3-component, 8-bit signed normalized
		case weave::opengl::GlFormat::RGBA8_SNORM:									return ImageFormat::R8G8B8A8_SNORM;			// 4-component, 8-bit signed normalized

		case weave::opengl::GlFormat::R8UI:											return ImageFormat::R8_UINT;					// 1-component, 8-bit unsigned integer
		case weave::opengl::GlFormat::RG8UI:											return ImageFormat::R8G8_UINT;					// 2-component, 8-bit unsigned integer
		case weave::opengl::GlFormat::RGB8UI:											return ImageFormat::R8G8B8_UINT;				// 3-component, 8-bit unsigned integer
		case weave::opengl::GlFormat::RGBA8UI:										return ImageFormat::R8G8B8A8_UINT;				// 4-component, 8-bit unsigned integer

		case weave::opengl::GlFormat::R8I:											return ImageFormat::R8_SINT;					// 1-component, 8-bit signed integer
		case weave::opengl::GlFormat::RG8I:											return ImageFormat::R8G8_SINT;					// 2-component, 8-bit signed integer
		case weave::opengl::GlFormat::RGB8I:											return ImageFormat::R8G8B8_SINT;				// 3-component, 8-bit signed integer
		case weave::opengl::GlFormat::RGBA8I:											return ImageFormat::R8G8B8A8_SINT;				// 4-component, 8-bit signed integer

		case weave::opengl::GlFormat::SR8:											return ImageFormat::R8_SRGB;					// 1-component, 8-bit sRGB
		case weave::opengl::GlFormat::SRG8:											return ImageFormat::R8G8_SRGB;					// 2-component, 8-bit sRGB
		case weave::opengl::GlFormat::SRGB8:											return ImageFormat::R8G8B8_SRGB;				// 3-component, 8-bit sRGB
		case weave::opengl::GlFormat::SRGB8_ALPHA8:									return ImageFormat::R8G8B8A8_SRGB;				// 4-component, 8-bit sRGB

		//
		// 16 bits per component
		//
		case weave::opengl::GlFormat::R16:											return ImageFormat::R16_UNORM;					// 1-component, 16-bit unsigned normalized
		case weave::opengl::GlFormat::RG16:											return ImageFormat::R16G16_UNORM;				// 2-component, 16-bit unsigned normalized
		case weave::opengl::GlFormat::RGB16:											return ImageFormat::R16G16B16_UNORM;			// 3-component, 16-bit unsigned normalized
		case weave::opengl::GlFormat::RGBA16:											return ImageFormat::R16G16B16A16_UNORM;		// 4-component, 16-bit unsigned normalized

		case weave::opengl::GlFormat::R16_SNORM:										return ImageFormat::R16_SNORM;					// 1-component, 16-bit signed normalized
		case weave::opengl::GlFormat::RG16_SNORM:										return ImageFormat::R16G16_SNORM;				// 2-component, 16-bit signed normalized
		case weave::opengl::GlFormat::RGB16_SNORM:									return ImageFormat::R16G16B16_SNORM;			// 3-component, 16-bit signed normalized
		case weave::opengl::GlFormat::RGBA16_SNORM:									return ImageFormat::R16G16B16A16_SNORM;		// 4-component, 16-bit signed normalized

		case weave::opengl::GlFormat::R16UI:											return ImageFormat::R16_UINT;					// 1-component, 16-bit unsigned integer
		case weave::opengl::GlFormat::RG16UI:											return ImageFormat::R16G16_UINT;				// 2-component, 16-bit unsigned integer
		case weave::opengl::GlFormat::RGB16UI:										return ImageFormat::R16G16B16_UINT;			// 3-component, 16-bit unsigned integer
		case weave::opengl::GlFormat::RGBA16UI:										return ImageFormat::R16G16B16A16_UINT;			// 4-component, 16-bit unsigned integer

		case weave::opengl::GlFormat::R16I:											return ImageFormat::R16_SINT;					// 1-component, 16-bit signed integer
		case weave::opengl::GlFormat::RG16I:											return ImageFormat::R16G16_SINT;				// 2-component, 16-bit signed integer
		case weave::opengl::GlFormat::RGB16I:											return ImageFormat::R16G16B16_SINT;			// 3-component, 16-bit signed integer
		case weave::opengl::GlFormat::RGBA16I:										return ImageFormat::R16G16B16A16_SINT;			// 4-component, 16-bit signed integer

		case weave::opengl::GlFormat::R16F:											return ImageFormat::R16_SFLOAT;				// 1-component, 16-bit floating-point
		case weave::opengl::GlFormat::RG16F:											return ImageFormat::R16G16_SFLOAT;				// 2-component, 16-bit floating-point
		case weave::opengl::GlFormat::RGB16F:											return ImageFormat::R16G16B16_SFLOAT;			// 3-component, 16-bit floating-point
		case weave::opengl::GlFormat::RGBA16F:										return ImageFormat::R16G16B16A16_SFLOAT;		// 4-component, 16-bit floating-point

		//
		// 32 bits per component
		//
		case weave::opengl::GlFormat::R32UI:											return ImageFormat::R32_UINT;					// 1-component, 32-bit unsigned integer
		case weave::opengl::GlFormat::RG32UI:											return ImageFormat::R32G32_UINT;				// 2-component, 32-bit unsigned integer
		case weave::opengl::GlFormat::RGB32UI:										return ImageFormat::R32G32B32_UINT;			// 3-component, 32-bit unsigned integer
		case weave::opengl::GlFormat::RGBA32UI:										return ImageFormat::R32G32B32A32_UINT;			// 4-component, 32-bit unsigned integer

		case weave::opengl::GlFormat::R32I:											return ImageFormat::R32_SINT;					// 1-component, 32-bit signed integer
		case weave::opengl::GlFormat::RG32I:											return ImageFormat::R32G32_SINT;				// 2-component, 32-bit signed integer
		case weave::opengl::GlFormat::RGB32I:											return ImageFormat::R32G32B32_SINT;			// 3-component, 32-bit signed integer
		case weave::opengl::GlFormat::RGBA32I:										return ImageFormat::R32G32B32A32_SINT;			// 4-component, 32-bit signed integer

		case weave::opengl::GlFormat::R32F:											return ImageFormat::R32_SFLOAT;				// 1-component, 32-bit floating-point
		case weave::opengl::GlFormat::RG32F:											return ImageFormat::R32G32_SFLOAT;				// 2-component, 32-bit floating-point
		case weave::opengl::GlFormat::RGB32F:											return ImageFormat::R32G32B32_SFLOAT;			// 3-component, 32-bit floating-point
		case weave::opengl::GlFormat::RGBA32F:										return ImageFormat::R32G32B32A32_SFLOAT;		// 4-component, 32-bit floating-point

		//
		// Packed
		//
		case weave::opengl::GlFormat::R3_G3_B2:										return ImageFormat::UNDEFINED;					// 3-component 3:3:2,       unsigned normalized
		case weave::opengl::GlFormat::RGB4:											return ImageFormat::UNDEFINED;					// 3-component 4:4:4,       unsigned normalized
		case weave::opengl::GlFormat::RGB5:											return ImageFormat::R5G5B5A1_UNORM_PACK16;		// 3-component 5:5:5,       unsigned normalized
		case weave::opengl::GlFormat::RGB565:											return ImageFormat::R5G6B5_UNORM_PACK16;		// 3-component 5:6:5,       unsigned normalized
		case weave::opengl::GlFormat::RGB10:											return ImageFormat::A2R10G10B10_UNORM_PACK32;	// 3-component 10:10:10,    unsigned normalized
		case weave::opengl::GlFormat::RGB12:											return ImageFormat::UNDEFINED;					// 3-component 12:12:12,    unsigned normalized
		case weave::opengl::GlFormat::RGBA2:											return ImageFormat::UNDEFINED;					// 4-component 2:2:2:2,     unsigned normalized
		case weave::opengl::GlFormat::RGBA4:											return ImageFormat::R4G4B4A4_UNORM_PACK16;		// 4-component 4:4:4:4,     unsigned normalized
		case weave::opengl::GlFormat::RGBA12:											return ImageFormat::UNDEFINED;					// 4-component 12:12:12:12, unsigned normalized
		case weave::opengl::GlFormat::RGB5_A1:										return ImageFormat::A1R5G5B5_UNORM_PACK16;		// 4-component 5:5:5:1,     unsigned normalized
		case weave::opengl::GlFormat::RGB10_A2:										return ImageFormat::A2R10G10B10_UNORM_PACK32;	// 4-component 10:10:10:2,  unsigned normalized
		case weave::opengl::GlFormat::RGB10_A2UI:										return ImageFormat::A2R10G10B10_UINT_PACK32;	// 4-component 10:10:10:2,  unsigned integer
		case weave::opengl::GlFormat::R11F_G11F_B10F:									return ImageFormat::B10G11R11_UFLOAT_PACK32;	// 3-component 11:11:10,    floating-point
		case weave::opengl::GlFormat::RGB9_E5:										return ImageFormat::E5B9G9R9_UFLOAT_PACK32;	// 3-component/exp 9:9:9/5, floating-point

		//
		// S3TC/DXT/BC
		//

		case weave::opengl::GlFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:					return ImageFormat::BC1_RGB_UNORM_BLOCK;		// line through 3D space, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:					return ImageFormat::BC1_RGBA_UNORM_BLOCK;		// line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:					return ImageFormat::BC2_UNORM_BLOCK;			// line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:					return ImageFormat::BC3_UNORM_BLOCK;			// line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

		case weave::opengl::GlFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:					return ImageFormat::BC1_RGB_SRGB_BLOCK;		// line through 3D space, 4x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:			return ImageFormat::BC1_RGBA_SRGB_BLOCK;		// line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:			return ImageFormat::BC2_SRGB_BLOCK;			// line through 3D space plus line through 1D space, 4x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:			return ImageFormat::BC3_SRGB_BLOCK;			// line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

		case weave::opengl::GlFormat::COMPRESSED_LUMINANCE_LATC1_EXT:					return ImageFormat::BC4_UNORM_BLOCK;			// line through 1D space, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:			return ImageFormat::BC5_UNORM_BLOCK;			// two lines through 1D space, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:			return ImageFormat::BC4_SNORM_BLOCK;			// line through 1D space, 4x4 blocks, signed normalized
		case weave::opengl::GlFormat::COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:	return ImageFormat::BC5_SNORM_BLOCK;			// two lines through 1D space, 4x4 blocks, signed normalized

		case weave::opengl::GlFormat::COMPRESSED_RED_RGTC1:							return ImageFormat::BC4_UNORM_BLOCK;			// line through 1D space, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RG_RGTC2:							return ImageFormat::BC5_UNORM_BLOCK;			// two lines through 1D space, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_SIGNED_RED_RGTC1:					return ImageFormat::BC4_SNORM_BLOCK;			// line through 1D space, 4x4 blocks, signed normalized
		case weave::opengl::GlFormat::COMPRESSED_SIGNED_RG_RGTC2:						return ImageFormat::BC5_SNORM_BLOCK;			// two lines through 1D space, 4x4 blocks, signed normalized

		case weave::opengl::GlFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:				return ImageFormat::BC6H_UFLOAT_BLOCK;			// 3-component, 4x4 blocks, unsigned floating-point
		case weave::opengl::GlFormat::COMPRESSED_RGB_BPTC_SIGNED_FLOAT:				return ImageFormat::BC6H_SFLOAT_BLOCK;			// 3-component, 4x4 blocks, signed floating-point
		case weave::opengl::GlFormat::COMPRESSED_RGBA_BPTC_UNORM:						return ImageFormat::BC7_UNORM_BLOCK;			// 4-component, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM:				return ImageFormat::BC7_SRGB_BLOCK;			// 4-component, 4x4 blocks, sRGB

		//
		// ETC
		//
		case weave::opengl::GlFormat::ETC1_RGB8_OES:									return ImageFormat::ETC2_R8G8B8_UNORM_BLOCK;	// 3-component ETC1, 4x4 blocks, unsigned normalized

		case weave::opengl::GlFormat::COMPRESSED_RGB8_ETC2:							return ImageFormat::ETC2_R8G8B8_UNORM_BLOCK;	// 3-component ETC2, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return ImageFormat::ETC2_R8G8B8A1_UNORM_BLOCK;	// 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA8_ETC2_EAC:						return ImageFormat::ETC2_R8G8B8A8_UNORM_BLOCK;	// 4-component ETC2, 4x4 blocks, unsigned normalized

		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ETC2:							return ImageFormat::ETC2_R8G8B8_SRGB_BLOCK;	// 3-component ETC2, 4x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return ImageFormat::ETC2_R8G8B8A1_SRGB_BLOCK;	// 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:				return ImageFormat::ETC2_R8G8B8A8_SRGB_BLOCK;	// 4-component ETC2, 4x4 blocks, sRGB

		case weave::opengl::GlFormat::COMPRESSED_R11_EAC:								return ImageFormat::EAC_R11_UNORM_BLOCK;		// 1-component ETC, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RG11_EAC:							return ImageFormat::EAC_R11G11_UNORM_BLOCK;	// 2-component ETC, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_SIGNED_R11_EAC:						return ImageFormat::EAC_R11_SNORM_BLOCK;		// 1-component ETC, 4x4 blocks, signed normalized
		case weave::opengl::GlFormat::COMPRESSED_SIGNED_RG11_EAC:						return ImageFormat::EAC_R11G11_SNORM_BLOCK;	// 2-component ETC, 4x4 blocks, signed normalized

		//
		// PVRTC
		//
		case weave::opengl::GlFormat::COMPRESSED_RGB_PVRTC_2BPPV1_IMG:				return ImageFormat::UNDEFINED;					// 3-component PVRTC, 16x8 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGB_PVRTC_4BPPV1_IMG:				return ImageFormat::UNDEFINED;					// 3-component PVRTC,  8x8 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:				return ImageFormat::UNDEFINED;					// 4-component PVRTC, 16x8 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:				return ImageFormat::UNDEFINED;					// 4-component PVRTC,  8x8 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:				return ImageFormat::UNDEFINED;					// 4-component PVRTC,  8x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:				return ImageFormat::UNDEFINED;					// 4-component PVRTC,  4x4 blocks, unsigned normalized

		case weave::opengl::GlFormat::COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:				return ImageFormat::UNDEFINED;					// 3-component PVRTC, 16x8 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:				return ImageFormat::UNDEFINED;					// 3-component PVRTC,  8x8 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:			return ImageFormat::UNDEFINED;					// 4-component PVRTC, 16x8 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:			return ImageFormat::UNDEFINED;					// 4-component PVRTC,  8x8 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:			return ImageFormat::UNDEFINED;					// 4-component PVRTC,  8x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:			return ImageFormat::UNDEFINED;					// 4-component PVRTC,  4x4 blocks, sRGB

		//
		// ASTC
		//
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_4x4_KHR:					return ImageFormat::ASTC_4x4_UNORM_BLOCK;		// 4-component ASTC, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_5x4_KHR:					return ImageFormat::ASTC_5x4_UNORM_BLOCK;		// 4-component ASTC, 5x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_5x5_KHR:					return ImageFormat::ASTC_5x5_UNORM_BLOCK;		// 4-component ASTC, 5x5 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_6x5_KHR:					return ImageFormat::ASTC_6x5_UNORM_BLOCK;		// 4-component ASTC, 6x5 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_6x6_KHR:					return ImageFormat::ASTC_6x6_UNORM_BLOCK;		// 4-component ASTC, 6x6 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_8x5_KHR:					return ImageFormat::ASTC_8x5_UNORM_BLOCK;		// 4-component ASTC, 8x5 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_8x6_KHR:					return ImageFormat::ASTC_8x6_UNORM_BLOCK;		// 4-component ASTC, 8x6 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_8x8_KHR:					return ImageFormat::ASTC_8x8_UNORM_BLOCK;		// 4-component ASTC, 8x8 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_10x5_KHR:					return ImageFormat::ASTC_10x5_UNORM_BLOCK;		// 4-component ASTC, 10x5 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_10x6_KHR:					return ImageFormat::ASTC_10x6_UNORM_BLOCK;		// 4-component ASTC, 10x6 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_10x8_KHR:					return ImageFormat::ASTC_10x8_UNORM_BLOCK;		// 4-component ASTC, 10x8 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_10x10_KHR:					return ImageFormat::ASTC_10x10_UNORM_BLOCK;	// 4-component ASTC, 10x10 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_12x10_KHR:					return ImageFormat::ASTC_12x10_UNORM_BLOCK;	// 4-component ASTC, 12x10 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_12x12_KHR:					return ImageFormat::ASTC_12x12_UNORM_BLOCK;	// 4-component ASTC, 12x12 blocks, unsigned normalized

		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:			return ImageFormat::ASTC_4x4_SRGB_BLOCK;		// 4-component ASTC, 4x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:			return ImageFormat::ASTC_5x4_SRGB_BLOCK;		// 4-component ASTC, 5x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:			return ImageFormat::ASTC_5x5_SRGB_BLOCK;		// 4-component ASTC, 5x5 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:			return ImageFormat::ASTC_6x5_SRGB_BLOCK;		// 4-component ASTC, 6x5 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:			return ImageFormat::ASTC_6x6_SRGB_BLOCK;		// 4-component ASTC, 6x6 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:			return ImageFormat::ASTC_8x5_SRGB_BLOCK;		// 4-component ASTC, 8x5 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:			return ImageFormat::ASTC_8x6_SRGB_BLOCK;		// 4-component ASTC, 8x6 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:			return ImageFormat::ASTC_8x8_SRGB_BLOCK;		// 4-component ASTC, 8x8 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:			return ImageFormat::ASTC_10x5_SRGB_BLOCK;		// 4-component ASTC, 10x5 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:			return ImageFormat::ASTC_10x6_SRGB_BLOCK;		// 4-component ASTC, 10x6 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:			return ImageFormat::ASTC_10x8_SRGB_BLOCK;		// 4-component ASTC, 10x8 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:			return ImageFormat::ASTC_10x10_SRGB_BLOCK;		// 4-component ASTC, 10x10 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:			return ImageFormat::ASTC_12x10_SRGB_BLOCK;		// 4-component ASTC, 12x10 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:			return ImageFormat::ASTC_12x12_SRGB_BLOCK;		// 4-component ASTC, 12x12 blocks, sRGB

		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_3x3x3_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 3x3x3 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_4x3x3_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 4x3x3 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_4x4x3_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 4x4x3 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_4x4x4_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 4x4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_5x4x4_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 5x4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_5x5x4_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 5x5x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_5x5x5_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 5x5x5 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_6x5x5_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 6x5x5 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_6x6x5_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 6x6x5 blocks, unsigned normalized
		case weave::opengl::GlFormat::COMPRESSED_RGBA_ASTC_6x6x6_OES:					return ImageFormat::UNDEFINED;					// 4-component ASTC, 6x6x6 blocks, unsigned normalized

		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 3x3x3 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 4x3x3 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 4x4x3 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 4x4x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 5x4x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 5x5x4 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 5x5x5 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 6x5x5 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 6x6x5 blocks, sRGB
		case weave::opengl::GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:			return ImageFormat::UNDEFINED;					// 4-component ASTC, 6x6x6 blocks, sRGB

		//
		// ATC
		//
		case weave::opengl::GlFormat::ATC_RGB_AMD:									return ImageFormat::UNDEFINED;					// 3-component, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::ATC_RGBA_EXPLICIT_ALPHA_AMD:					return ImageFormat::UNDEFINED;					// 4-component, 4x4 blocks, unsigned normalized
		case weave::opengl::GlFormat::ATC_RGBA_INTERPOLATED_ALPHA_AMD:				return ImageFormat::UNDEFINED;					// 4-component, 4x4 blocks, unsigned normalized

		//
		// Palletized
		//
		case weave::opengl::GlFormat::PALETTE4_RGB8_OES:								return ImageFormat::UNDEFINED;					// 3-component 8:8:8,   4-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE4_RGBA8_OES:								return ImageFormat::UNDEFINED;					// 4-component 8:8:8:8, 4-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE4_R5_G6_B5_OES:							return ImageFormat::UNDEFINED;					// 3-component 5:6:5,   4-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE4_RGBA4_OES:								return ImageFormat::UNDEFINED;					// 4-component 4:4:4:4, 4-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE4_RGB5_A1_OES:							return ImageFormat::UNDEFINED;					// 4-component 5:5:5:1, 4-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE8_RGB8_OES:								return ImageFormat::UNDEFINED;					// 3-component 8:8:8,   8-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE8_RGBA8_OES:								return ImageFormat::UNDEFINED;					// 4-component 8:8:8:8, 8-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE8_R5_G6_B5_OES:							return ImageFormat::UNDEFINED;					// 3-component 5:6:5,   8-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE8_RGBA4_OES:								return ImageFormat::UNDEFINED;					// 4-component 4:4:4:4, 8-bit palette, unsigned normalized
		case weave::opengl::GlFormat::PALETTE8_RGB5_A1_OES:							return ImageFormat::UNDEFINED;					// 4-component 5:5:5:1, 8-bit palette, unsigned normalized

		//
		// Depth/stencil
		//
		case weave::opengl::GlFormat::DEPTH_COMPONENT16:								return ImageFormat::D16_UNORM;
		case weave::opengl::GlFormat::DEPTH_COMPONENT24:								return ImageFormat::X8_D24_UNORM_PACK32;
		case weave::opengl::GlFormat::DEPTH_COMPONENT32:								return ImageFormat::UNDEFINED;
		case weave::opengl::GlFormat::DEPTH_COMPONENT32F:								return ImageFormat::D32_SFLOAT;
		case weave::opengl::GlFormat::DEPTH_COMPONENT32F_NV:							return ImageFormat::D32_SFLOAT;
		case weave::opengl::GlFormat::STENCIL_INDEX1:									return ImageFormat::UNDEFINED;
		case weave::opengl::GlFormat::STENCIL_INDEX4:									return ImageFormat::UNDEFINED;
		case weave::opengl::GlFormat::STENCIL_INDEX8:									return ImageFormat::S8_UINT;
		case weave::opengl::GlFormat::STENCIL_INDEX16:								return ImageFormat::UNDEFINED;
		case weave::opengl::GlFormat::DEPTH24_STENCIL8:								return ImageFormat::D24_UNORM_S8_UINT;
		case weave::opengl::GlFormat::DEPTH32F_STENCIL8:								return ImageFormat::D32_SFLOAT_S8_UINT;
		case weave::opengl::GlFormat::DEPTH32F_STENCIL8_NV:							return ImageFormat::D32_SFLOAT_S8_UINT;

		default:												return ImageFormat::UNDEFINED;
	}
}

enum FormatSizeFlagBits {
	NONE					= 0,
	PACKED_BIT				= 0x00000001,
	COMPRESSED_BIT			= 0x00000002,
	PALETTIZED_BIT			= 0x00000004,
	DEPTH_BIT				= 0x00000008,
	STENCIL_BIT				= 0x00000010,
};

struct FormatSize {
	FormatSizeFlagBits	flags;
	unsigned int		paletteSizeInBits;
	unsigned int		blockSizeInBits;
	unsigned int		blockWidth;			// in texels
	unsigned int		blockHeight;		// in texels
	unsigned int		blockDepth;			// in texels
};

static inline FormatSize vkGetFormatSize( const ImageFormat format)
{
	FormatSize formatSize = {};

	switch ( format )
	{
		case ImageFormat::R4G4_UNORM_PACK8:
			formatSize.flags = FormatSizeFlagBits::PACKED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 1 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R4G4B4A4_UNORM_PACK16:
		case ImageFormat::B4G4R4A4_UNORM_PACK16:
		case ImageFormat::R5G6B5_UNORM_PACK16:
		case ImageFormat::B5G6R5_UNORM_PACK16:
		case ImageFormat::R5G5B5A1_UNORM_PACK16:
		case ImageFormat::B5G5R5A1_UNORM_PACK16:
		case ImageFormat::A1R5G5B5_UNORM_PACK16:
			formatSize.flags = FormatSizeFlagBits::PACKED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 2 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R8_UNORM:
		case ImageFormat::R8_SNORM:
		case ImageFormat::R8_USCALED:
		case ImageFormat::R8_SSCALED:
		case ImageFormat::R8_UINT:
		case ImageFormat::R8_SINT:
		case ImageFormat::R8_SRGB:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 1 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R8G8_UNORM:
		case ImageFormat::R8G8_SNORM:
		case ImageFormat::R8G8_USCALED:
		case ImageFormat::R8G8_SSCALED:
		case ImageFormat::R8G8_UINT:
		case ImageFormat::R8G8_SINT:
		case ImageFormat::R8G8_SRGB:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 2 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R8G8B8_UNORM:
		case ImageFormat::R8G8B8_SNORM:
		case ImageFormat::R8G8B8_USCALED:
		case ImageFormat::R8G8B8_SSCALED:
		case ImageFormat::R8G8B8_UINT:
		case ImageFormat::R8G8B8_SINT:
		case ImageFormat::R8G8B8_SRGB:
		case ImageFormat::B8G8R8_UNORM:
		case ImageFormat::B8G8R8_SNORM:
		case ImageFormat::B8G8R8_USCALED:
		case ImageFormat::B8G8R8_SSCALED:
		case ImageFormat::B8G8R8_UINT:
		case ImageFormat::B8G8R8_SINT:
		case ImageFormat::B8G8R8_SRGB:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 3 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R8G8B8A8_UNORM:
		case ImageFormat::R8G8B8A8_SNORM:
		case ImageFormat::R8G8B8A8_USCALED:
		case ImageFormat::R8G8B8A8_SSCALED:
		case ImageFormat::R8G8B8A8_UINT:
		case ImageFormat::R8G8B8A8_SINT:
		case ImageFormat::R8G8B8A8_SRGB:
		case ImageFormat::B8G8R8A8_UNORM:
		case ImageFormat::B8G8R8A8_SNORM:
		case ImageFormat::B8G8R8A8_USCALED:
		case ImageFormat::B8G8R8A8_SSCALED:
		case ImageFormat::B8G8R8A8_UINT:
		case ImageFormat::B8G8R8A8_SINT:
		case ImageFormat::B8G8R8A8_SRGB:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::A8B8G8R8_UNORM_PACK32:
		case ImageFormat::A8B8G8R8_SNORM_PACK32:
		case ImageFormat::A8B8G8R8_USCALED_PACK32:
		case ImageFormat::A8B8G8R8_SSCALED_PACK32:
		case ImageFormat::A8B8G8R8_UINT_PACK32:
		case ImageFormat::A8B8G8R8_SINT_PACK32:
		case ImageFormat::A8B8G8R8_SRGB_PACK32:
			formatSize.flags = FormatSizeFlagBits::PACKED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::A2R10G10B10_UNORM_PACK32:
		case ImageFormat::A2R10G10B10_SNORM_PACK32:
		case ImageFormat::A2R10G10B10_USCALED_PACK32:
		case ImageFormat::A2R10G10B10_SSCALED_PACK32:
		case ImageFormat::A2R10G10B10_UINT_PACK32:
		case ImageFormat::A2R10G10B10_SINT_PACK32:
		case ImageFormat::A2B10G10R10_UNORM_PACK32:
		case ImageFormat::A2B10G10R10_SNORM_PACK32:
		case ImageFormat::A2B10G10R10_USCALED_PACK32:
		case ImageFormat::A2B10G10R10_SSCALED_PACK32:
		case ImageFormat::A2B10G10R10_UINT_PACK32:
		case ImageFormat::A2B10G10R10_SINT_PACK32:
			formatSize.flags = FormatSizeFlagBits::PACKED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R16_UNORM:
		case ImageFormat::R16_SNORM:
		case ImageFormat::R16_USCALED:
		case ImageFormat::R16_SSCALED:
		case ImageFormat::R16_UINT:
		case ImageFormat::R16_SINT:
		case ImageFormat::R16_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 2 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R16G16_UNORM:
		case ImageFormat::R16G16_SNORM:
		case ImageFormat::R16G16_USCALED:
		case ImageFormat::R16G16_SSCALED:
		case ImageFormat::R16G16_UINT:
		case ImageFormat::R16G16_SINT:
		case ImageFormat::R16G16_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R16G16B16_UNORM:
		case ImageFormat::R16G16B16_SNORM:
		case ImageFormat::R16G16B16_USCALED:
		case ImageFormat::R16G16B16_SSCALED:
		case ImageFormat::R16G16B16_UINT:
		case ImageFormat::R16G16B16_SINT:
		case ImageFormat::R16G16B16_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 6 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R16G16B16A16_UNORM:
		case ImageFormat::R16G16B16A16_SNORM:
		case ImageFormat::R16G16B16A16_USCALED:
		case ImageFormat::R16G16B16A16_SSCALED:
		case ImageFormat::R16G16B16A16_UINT:
		case ImageFormat::R16G16B16A16_SINT:
		case ImageFormat::R16G16B16A16_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 8 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R32_UINT:
		case ImageFormat::R32_SINT:
		case ImageFormat::R32_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R32G32_UINT:
		case ImageFormat::R32G32_SINT:
		case ImageFormat::R32G32_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 8 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R32G32B32_UINT:
		case ImageFormat::R32G32B32_SINT:
		case ImageFormat::R32G32B32_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 12 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R32G32B32A32_UINT:
		case ImageFormat::R32G32B32A32_SINT:
		case ImageFormat::R32G32B32A32_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R64_UINT:
		case ImageFormat::R64_SINT:
		case ImageFormat::R64_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 8 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R64G64_UINT:
		case ImageFormat::R64G64_SINT:
		case ImageFormat::R64G64_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R64G64B64_UINT:
		case ImageFormat::R64G64B64_SINT:
		case ImageFormat::R64G64B64_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 24 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::R64G64B64A64_UINT:
		case ImageFormat::R64G64B64A64_SINT:
		case ImageFormat::R64G64B64A64_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 32 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::B10G11R11_UFLOAT_PACK32:
		case ImageFormat::E5B9G9R9_UFLOAT_PACK32:
			formatSize.flags = FormatSizeFlagBits::PACKED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::D16_UNORM:
			formatSize.flags = FormatSizeFlagBits::DEPTH_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 2 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::X8_D24_UNORM_PACK32:
			formatSize.flags = static_cast<FormatSizeFlagBits>(FormatSizeFlagBits::PACKED_BIT | FormatSizeFlagBits::DEPTH_BIT);
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::D32_SFLOAT:
			formatSize.flags = FormatSizeFlagBits::DEPTH_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::S8_UINT:
			formatSize.flags = FormatSizeFlagBits::STENCIL_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 1 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::D16_UNORM_S8_UINT:
			formatSize.flags = static_cast<FormatSizeFlagBits>(FormatSizeFlagBits::DEPTH_BIT | FormatSizeFlagBits::STENCIL_BIT);
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 3 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::D24_UNORM_S8_UINT:
			formatSize.flags = static_cast<FormatSizeFlagBits>(FormatSizeFlagBits::DEPTH_BIT | FormatSizeFlagBits::STENCIL_BIT);
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 4 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::D32_SFLOAT_S8_UINT:
			formatSize.flags = static_cast<FormatSizeFlagBits>(FormatSizeFlagBits::DEPTH_BIT | FormatSizeFlagBits::STENCIL_BIT);
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 8 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::BC1_RGB_UNORM_BLOCK:
		case ImageFormat::BC1_RGB_SRGB_BLOCK:
		case ImageFormat::BC1_RGBA_UNORM_BLOCK:
		case ImageFormat::BC1_RGBA_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 8 * 8;
			formatSize.blockWidth = 4;
			formatSize.blockHeight = 4;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::BC2_UNORM_BLOCK:
		case ImageFormat::BC2_SRGB_BLOCK:
		case ImageFormat::BC3_UNORM_BLOCK:
		case ImageFormat::BC3_SRGB_BLOCK:
		case ImageFormat::BC4_UNORM_BLOCK:
		case ImageFormat::BC4_SNORM_BLOCK:
		case ImageFormat::BC5_UNORM_BLOCK:
		case ImageFormat::BC5_SNORM_BLOCK:
		case ImageFormat::BC6H_UFLOAT_BLOCK:
		case ImageFormat::BC6H_SFLOAT_BLOCK:
		case ImageFormat::BC7_UNORM_BLOCK:
		case ImageFormat::BC7_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 4;
			formatSize.blockHeight = 4;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ETC2_R8G8B8_UNORM_BLOCK:
		case ImageFormat::ETC2_R8G8B8_SRGB_BLOCK:
		case ImageFormat::ETC2_R8G8B8A1_UNORM_BLOCK:
		case ImageFormat::ETC2_R8G8B8A1_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 8 * 8;
			formatSize.blockWidth = 4;
			formatSize.blockHeight = 4;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ETC2_R8G8B8A8_UNORM_BLOCK:
		case ImageFormat::ETC2_R8G8B8A8_SRGB_BLOCK:
		case ImageFormat::EAC_R11_UNORM_BLOCK:
		case ImageFormat::EAC_R11_SNORM_BLOCK:
		case ImageFormat::EAC_R11G11_UNORM_BLOCK:
		case ImageFormat::EAC_R11G11_SNORM_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 4;
			formatSize.blockHeight = 4;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_4x4_UNORM_BLOCK:
		case ImageFormat::ASTC_4x4_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 4;
			formatSize.blockHeight = 4;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_5x4_UNORM_BLOCK:
		case ImageFormat::ASTC_5x4_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 5;
			formatSize.blockHeight = 4;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_5x5_UNORM_BLOCK:
		case ImageFormat::ASTC_5x5_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 5;
			formatSize.blockHeight = 5;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_6x5_UNORM_BLOCK:
		case ImageFormat::ASTC_6x5_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 6;
			formatSize.blockHeight = 5;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_6x6_UNORM_BLOCK:
		case ImageFormat::ASTC_6x6_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 6;
			formatSize.blockHeight = 6;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_8x5_UNORM_BLOCK:
		case ImageFormat::ASTC_8x5_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 8;
			formatSize.blockHeight = 5;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_8x6_UNORM_BLOCK:
		case ImageFormat::ASTC_8x6_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 8;
			formatSize.blockHeight = 6;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_8x8_UNORM_BLOCK:
		case ImageFormat::ASTC_8x8_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 8;
			formatSize.blockHeight = 8;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_10x5_UNORM_BLOCK:
		case ImageFormat::ASTC_10x5_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 10;
			formatSize.blockHeight = 5;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_10x6_UNORM_BLOCK:
		case ImageFormat::ASTC_10x6_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 10;
			formatSize.blockHeight = 6;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_10x8_UNORM_BLOCK: 
		case ImageFormat::ASTC_10x8_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 10;
			formatSize.blockHeight = 8;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_10x10_UNORM_BLOCK:
		case ImageFormat::ASTC_10x10_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 10;
			formatSize.blockHeight = 10;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_12x10_UNORM_BLOCK:
		case ImageFormat::ASTC_12x10_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 12;
			formatSize.blockHeight = 10;
			formatSize.blockDepth = 1;
			break;
		case ImageFormat::ASTC_12x12_UNORM_BLOCK:
		case ImageFormat::ASTC_12x12_SRGB_BLOCK:
			formatSize.flags = FormatSizeFlagBits::COMPRESSED_BIT;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 16 * 8;
			formatSize.blockWidth = 12;
			formatSize.blockHeight = 12;
			formatSize.blockDepth = 1;
			break;
		default:
			formatSize.flags = FormatSizeFlagBits::NONE;
			formatSize.paletteSizeInBits = 0;
			formatSize.blockSizeInBits = 0 * 8;
			formatSize.blockWidth = 1;
			formatSize.blockHeight = 1;
			formatSize.blockDepth = 1;
			break;
	}

	return formatSize;
}

} //namespace vulkan
} //namespace weave