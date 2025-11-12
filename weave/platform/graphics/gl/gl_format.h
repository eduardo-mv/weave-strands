/*
================================================================================================

This file was modified for usage on Weave into namespaced C++ version. 
The original license text and information is bellow.

Description	:	OpenGL formats/types and properties.
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

This header stores the OpenGL formats/types and two simple routines
to derive the format/type from an internal format. These routines
are useful to verify the data in a KTX container files. The OpenGL
constants are generally useful to convert files like KTX and glTF
to different graphics APIs.

This header stores the OpenGL formats/types that are used as parameters
to the following OpenGL functions:

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
  4. The OpenGL constants in this header are also used to derive Vulkan formats
     from the OpenGL formats/types stored in files like KTX and glTF. These file
     formats may use OpenGL formats/types that are not supported by the OpenGL
     implementation on the platform but are supported by the Vulkan implementation.


ENTRY POINTS
============

static inline GLenum glGetFormatFromInternalFormat( const GLenum internalFormat );
static inline GLenum glGetTypeFromInternalFormat( const GLenum internalFormat );
static inline void glGetFormatSize( const GLenum internalFormat, GlFormatSize * pFormatSize );
static inline unsigned int glGetTypeSizeFromType( const GLenum type );
 
MODIFICATIONS for use in libktx
===============================
 
2018.3.23 Added glGetTypeSizeFromType. Mark Callow, Edgewise Consulting.
2019.3.09 #if 0 around GL type declarations. Mark Callow,     〃

================================================================================================
*/

#pragma once
#include <cstdint>

namespace weave {
namespace opengl {

enum class GlFormat : unsigned int {

INVALID_VALUE								= 0x0501,
														
		/*												
		================================================================================================================================
														
		Format to glTexImage2D and glTexImage3D.		
														
		================================================================================================================================
		*/												
														
RED											= 0x1903,	// same as RED_EXT
GREEN										= 0x1904,	// deprecated
BLUE											= 0x1905,	// deprecated
ALPHA										= 0x1906,	// deprecated
LUMINANCE									= 0x1909,	// deprecated
SLUMINANCE									= 0x8C46,	// deprecated, same as SLUMINANCE_EXT
LUMINANCE_ALPHA								= 0x190A,	// deprecated
SLUMINANCE_ALPHA								= 0x8C44,	// deprecated, same as SLUMINANCE_ALPHA_EXT
INTENSITY									= 0x8049,	// deprecated, same as INTENSITY_EXT
RG											= 0x8227,	// same as RG_EXT
RGB											= 0x1907,
BGR											= 0x80E0,	// same as BGR_EXT
RGBA											= 0x1908,
BGRA											= 0x80E1,	// same as BGRA_EXT
RED_INTEGER									= 0x8D94,	// same as RED_INTEGER_EXT
GREEN_INTEGER								= 0x8D95,	// deprecated, same as GREEN_INTEGER_EXT
BLUE_INTEGER									= 0x8D96,	// deprecated, same as BLUE_INTEGER_EXT
ALPHA_INTEGER								= 0x8D97,	// deprecated, same as ALPHA_INTEGER_EXT
LUMINANCE_INTEGER							= 0x8D9C,	// deprecated, same as LUMINANCE_INTEGER_EXT
LUMINANCE_ALPHA_INTEGER						= 0x8D9D,	// deprecated, same as LUMINANCE_ALPHA_INTEGER_EXT
RG_INTEGER									= 0x8228,	// same as RG_INTEGER_EXT
RGB_INTEGER									= 0x8D98,	// same as RGB_INTEGER_EXT
BGR_INTEGER									= 0x8D9A,	// same as BGR_INTEGER_EXT
RGBA_INTEGER									= 0x8D99,	// same as RGBA_INTEGER_EXT
BGRA_INTEGER									= 0x8D9B,	// same as BGRA_INTEGER_EXT
COLOR_INDEX									= 0x1900,	// deprecated
STENCIL_INDEX								= 0x1901,
DEPTH_COMPONENT								= 0x1902,
DEPTH_STENCIL								= 0x84F9,	// same as DEPTH_STENCIL_NV and DEPTH_STENCIL_EXT and DEPTH_STENCIL_OES

		/*
		================================================================================================================================

		Type to glTexImage2D, glTexImage3D and glVertexAttribPointer.

		================================================================================================================================
		*/

BYTE											= 0x1400,
UNSIGNED_BYTE								= 0x1401,
SHORT										= 0x1402,
UNSIGNED_SHORT								= 0x1403,
INT											= 0x1404,
UNSIGNED_INT									= 0x1405,
INT64										= 0x140E,	// same as INT64_NV and INT64_ARB
UNSIGNED_INT64								= 0x140F,	// same as UNSIGNED_INT64_NV and UNSIGNED_INT64_ARB
HALF_FLOAT									= 0x140B,	// same as HALF_FLOAT_NV and HALF_FLOAT_ARB
HALF_FLOAT_OES								= 0x8D61,	// Note that this different from HALF_FLOAT.
FLOAT										= 0x1406,
DOUBLE										= 0x140A,	// same as DOUBLE_EXT
UNSIGNED_BYTE_3_3_2							= 0x8032,	// same as UNSIGNED_BYTE_3_3_2_EXT
UNSIGNED_BYTE_2_3_3_REV						= 0x8362,	// same as UNSIGNED_BYTE_2_3_3_REV_EXT
UNSIGNED_SHORT_5_6_5							= 0x8363,	// same as UNSIGNED_SHORT_5_6_5_EXT
UNSIGNED_SHORT_5_6_5_REV						= 0x8364,	// same as UNSIGNED_SHORT_5_6_5_REV_EXT
UNSIGNED_SHORT_4_4_4_4						= 0x8033,	// same as UNSIGNED_SHORT_4_4_4_4_EXT
UNSIGNED_SHORT_4_4_4_4_REV					= 0x8365,	// same as UNSIGNED_SHORT_4_4_4_4_REV_IMG and UNSIGNED_SHORT_4_4_4_4_REV_EXT
UNSIGNED_SHORT_5_5_5_1						= 0x8034,	// same as UNSIGNED_SHORT_5_5_5_1_EXT
UNSIGNED_SHORT_1_5_5_5_REV					= 0x8366,	// same as UNSIGNED_SHORT_1_5_5_5_REV_EXT
UNSIGNED_INT_8_8_8_8							= 0x8035,	// same as UNSIGNED_INT_8_8_8_8_EXT
UNSIGNED_INT_8_8_8_8_REV						= 0x8367,	// same as UNSIGNED_INT_8_8_8_8_REV_EXT
UNSIGNED_INT_10_10_10_2						= 0x8036,	// same as UNSIGNED_INT_10_10_10_2_EXT
UNSIGNED_INT_2_10_10_10_REV					= 0x8368,	// same as UNSIGNED_INT_2_10_10_10_REV_EXT
UNSIGNED_INT_10F_11F_11F_REV					= 0x8C3B,	// same as UNSIGNED_INT_10F_11F_11F_REV_EXT
UNSIGNED_INT_5_9_9_9_REV						= 0x8C3E,	// same as UNSIGNED_INT_5_9_9_9_REV_EXT
UNSIGNED_INT_24_8							= 0x84FA,	// same as UNSIGNED_INT_24_8_NV and UNSIGNED_INT_24_8_EXT and UNSIGNED_INT_24_8_OES
FLOAT_32_UNSIGNED_INT_24_8_REV				= 0x8DAD,	// same as FLOAT_32_UNSIGNED_INT_24_8_REV_NV and FLOAT_32_UNSIGNED_INT_24_8_REV_ARB

		/*
		================================================================================================================================

		Internal format to glTexImage2D, glTexImage3D, glCompressedTexImage2D, glCompressedTexImage3D, glTexStorage2D, glTexStorage3D

		================================================================================================================================
		*/

		//
		// 8 bits per component
		//

R8											= 0x8229,	// same as R8_EXT
RG8											= 0x822B,	// same as RG8_EXT
RGB8											= 0x8051,	// same as RGB8_EXT and RGB8_OES
RGBA8										= 0x8058,	// same as RGBA8_EXT and RGBA8_OES
R8_SNORM										= 0x8F94,
RG8_SNORM									= 0x8F95,
RGB8_SNORM									= 0x8F96,
RGBA8_SNORM									= 0x8F97,
R8UI											= 0x8232,
RG8UI										= 0x8238,
RGB8UI										= 0x8D7D,	// same as RGB8UI_EXT
RGBA8UI										= 0x8D7C,	// same as RGBA8UI_EXT
R8I											= 0x8231,
RG8I											= 0x8237,
RGB8I										= 0x8D8F,	// same as RGB8I_EXT
RGBA8I										= 0x8D8E,	// same as RGBA8I_EXT
SR8											= 0x8FBD,	// same as SR8_EXT
SRG8											= 0x8FBE,	// same as SRG8_EXT
SRGB8										= 0x8C41,	// same as SRGB8_EXT
SRGB8_ALPHA8									= 0x8C43,	// same as SRGB8_ALPHA8_EXT

//
// 16 bits per component
//
R16											= 0x822A,	// same as R16_EXT
RG16											= 0x822C,	// same as RG16_EXT
RGB16										= 0x8054,	// same as RGB16_EXT
RGBA16										= 0x805B,	// same as RGBA16_EXT
R16_SNORM									= 0x8F98,	// same as R16_SNORM_EXT
RG16_SNORM									= 0x8F99,	// same as RG16_SNORM_EXT
RGB16_SNORM									= 0x8F9A,	// same as RGB16_SNORM_EXT
RGBA16_SNORM									= 0x8F9B,	// same as RGBA16_SNORM_EXT
R16UI										= 0x8234,
RG16UI										= 0x823A,
RGB16UI										= 0x8D77,	// same as RGB16UI_EXT
RGBA16UI										= 0x8D76,	// same as RGBA16UI_EXT
R16I											= 0x8233,
RG16I										= 0x8239,
RGB16I										= 0x8D89,	// same as RGB16I_EXT
RGBA16I										= 0x8D88,	// same as RGBA16I_EXT
R16F											= 0x822D,	// same as R16F_EXT
RG16F										= 0x822F,	// same as RG16F_EXT
RGB16F										= 0x881B,	// same as RGB16F_EXT and RGB16F_ARB
RGBA16F										= 0x881A,	// sama as RGBA16F_EXT and RGBA16F_ARB

//
// 32 bits per component
//
R32UI										= 0x8236,
RG32UI										= 0x823C,
RGB32UI										= 0x8D71,	// same as RGB32UI_EXT
RGBA32UI										= 0x8D70,	// same as RGBA32UI_EXT
R32I											= 0x8235,
RG32I										= 0x823B,
RGB32I										= 0x8D83,	// same as RGB32I_EXT 
RGBA32I										= 0x8D82,	// same as RGBA32I_EXT
R32F											= 0x822E,	// same as R32F_EXT
RG32F										= 0x8230,	// same as RG32F_EXT
RGB32F										= 0x8815,	// same as RGB32F_EXT and RGB32F_ARB
RGBA32F										= 0x8814,	// same as RGBA32F_EXT and RGBA32F_ARB

//
// Packed
//
R3_G3_B2										= 0x2A10,
RGB4											= 0x804F,	// same as RGB4_EXT
RGB5											= 0x8050,	// same as RGB5_EXT
RGB565										= 0x8D62,	// same as RGB565_EXT and RGB565_OES
RGB10										= 0x8052,	// same as RGB10_EXT
RGB12										= 0x8053,	// same as RGB12_EXT
RGBA2										= 0x8055,	// same as RGBA2_EXT
RGBA4										= 0x8056,	// same as RGBA4_EXT and RGBA4_OES
RGBA12										= 0x805A,	// same as RGBA12_EXT
RGB5_A1										= 0x8057,	// same as RGB5_A1_EXT and RGB5_A1_OES
RGB10_A2										= 0x8059,	// same as RGB10_A2_EXT
RGB10_A2UI									= 0x906F,
R11F_G11F_B10F								= 0x8C3A,	// same as R11F_G11F_B10F_APPLE and R11F_G11F_B10F_EXT
RGB9_E5										= 0x8C3D,	// same as RGB9_E5_APPLE and RGB9_E5_EXT

//
// Alpha
//
ALPHA4										= 0x803B,	// deprecated, same as ALPHA4_EXT
ALPHA8										= 0x803C,	// deprecated, same as ALPHA8_EXT
ALPHA8_SNORM									= 0x9014,	// deprecated
ALPHA8UI_EXT									= 0x8D7E,	// deprecated
ALPHA8I_EXT									= 0x8D90,	// deprecated
ALPHA12										= 0x803D,	// deprecated, same as ALPHA12_EXT
ALPHA16										= 0x803E,	// deprecated, same as ALPHA16_EXT
ALPHA16_SNORM								= 0x9018,	// deprecated
ALPHA16UI_EXT								= 0x8D78,	// deprecated
ALPHA16I_EXT									= 0x8D8A,	// deprecated
ALPHA16F_ARB									= 0x881C,	// deprecated, same as ALPHA_FLOAT16_APPLE and ALPHA_FLOAT16_ATI
ALPHA32UI_EXT								= 0x8D72,	// deprecated
ALPHA32I_EXT									= 0x8D84,	// deprecated
ALPHA32F_ARB									= 0x8816,	// deprecated, same as ALPHA_FLOAT32_APPLE and ALPHA_FLOAT32_ATI

//
// Luminance
//
LUMINANCE4									= 0x803F,	// deprecated, same as LUMINANCE4_EXT
LUMINANCE8									= 0x8040,	// deprecated, same as LUMINANCE8_EXT
LUMINANCE8_SNORM								= 0x9015,	// deprecated
SLUMINANCE8									= 0x8C47,	// deprecated, same as SLUMINANCE8_EXT
LUMINANCE8UI_EXT								= 0x8D80,	// deprecated
LUMINANCE8I_EXT								= 0x8D92,	// deprecated
LUMINANCE12									= 0x8041,	// deprecated, same as LUMINANCE12_EXT
LUMINANCE16									= 0x8042,	// deprecated, same as LUMINANCE16_EXT
LUMINANCE16_SNORM							= 0x9019,	// deprecated
LUMINANCE16UI_EXT							= 0x8D7A,	// deprecated
LUMINANCE16I_EXT								= 0x8D8C,	// deprecated
LUMINANCE16F_ARB								= 0x881E,	// deprecated, same as LUMINANCE_FLOAT16_APPLE and LUMINANCE_FLOAT16_ATI
LUMINANCE32UI_EXT							= 0x8D74,	// deprecated
LUMINANCE32I_EXT								= 0x8D86,	// deprecated
LUMINANCE32F_ARB								= 0x8818,	// deprecated, same as LUMINANCE_FLOAT32_APPLE and LUMINANCE_FLOAT32_ATI

//
// Luminance/Alpha
//
LUMINANCE4_ALPHA4							= 0x8043,	// deprecated, same as LUMINANCE4_ALPHA4_EXT
LUMINANCE6_ALPHA2							= 0x8044,	// deprecated, same as LUMINANCE6_ALPHA2_EXT
LUMINANCE8_ALPHA8							= 0x8045,	// deprecated, same as LUMINANCE8_ALPHA8_EXT
LUMINANCE8_ALPHA8_SNORM						= 0x9016,	// deprecated
SLUMINANCE8_ALPHA8							= 0x8C45,	// deprecated, same as SLUMINANCE8_ALPHA8_EXT
LUMINANCE_ALPHA8UI_EXT						= 0x8D81,	// deprecated
LUMINANCE_ALPHA8I_EXT						= 0x8D93,	// deprecated
LUMINANCE12_ALPHA4							= 0x8046,	// deprecated, same as LUMINANCE12_ALPHA4_EXT
LUMINANCE12_ALPHA12							= 0x8047,	// deprecated, same as LUMINANCE12_ALPHA12_EXT
LUMINANCE16_ALPHA16							= 0x8048,	// deprecated, same as LUMINANCE16_ALPHA16_EXT
LUMINANCE16_ALPHA16_SNORM					= 0x901A,	// deprecated
LUMINANCE_ALPHA16UI_EXT						= 0x8D7B,	// deprecated
LUMINANCE_ALPHA16I_EXT						= 0x8D8D,	// deprecated
LUMINANCE_ALPHA16F_ARB						= 0x881F,	// deprecated, same as LUMINANCE_ALPHA_FLOAT16_APPLE and LUMINANCE_ALPHA_FLOAT16_ATI
LUMINANCE_ALPHA32UI_EXT						= 0x8D75,	// deprecated
LUMINANCE_ALPHA32I_EXT						= 0x8D87,	// deprecated
LUMINANCE_ALPHA32F_ARB						= 0x8819,	// deprecated, same as LUMINANCE_ALPHA_FLOAT32_APPLE and LUMINANCE_ALPHA_FLOAT32_ATI

//
// Intensity
//
INTENSITY4									= 0x804A,	// deprecated, same as INTENSITY4_EXT
INTENSITY8									= 0x804B,	// deprecated, same as INTENSITY8_EXT
INTENSITY8_SNORM								= 0x9017,	// deprecated
INTENSITY8UI_EXT								= 0x8D7F,	// deprecated
INTENSITY8I_EXT								= 0x8D91,	// deprecated
INTENSITY12									= 0x804C,	// deprecated, same as INTENSITY12_EXT
INTENSITY16									= 0x804D,	// deprecated, same as INTENSITY16_EXT
INTENSITY16_SNORM							= 0x901B,	// deprecated
INTENSITY16UI_EXT							= 0x8D79,	// deprecated
INTENSITY16I_EXT								= 0x8D8B,	// deprecated
INTENSITY16F_ARB								= 0x881D,	// deprecated, same as INTENSITY_FLOAT16_APPLE and INTENSITY_FLOAT16_ATI
INTENSITY32UI_EXT							= 0x8D73,	// deprecated
INTENSITY32I_EXT								= 0x8D85,	// deprecated
INTENSITY32F_ARB								= 0x8817,	// deprecated, same as INTENSITY_FLOAT32_APPLE and INTENSITY_FLOAT32_ATI

//
// Generic compression
//
COMPRESSED_RED								= 0x8225,
COMPRESSED_ALPHA								= 0x84E9,	// deprecated, same as COMPRESSED_ALPHA_ARB
COMPRESSED_LUMINANCE							= 0x84EA,	// deprecated, same as COMPRESSED_LUMINANCE_ARB
COMPRESSED_SLUMINANCE						= 0x8C4A,	// deprecated, same as COMPRESSED_SLUMINANCE_EXT
COMPRESSED_LUMINANCE_ALPHA					= 0x84EB,	// deprecated, same as COMPRESSED_LUMINANCE_ALPHA_ARB
COMPRESSED_SLUMINANCE_ALPHA					= 0x8C4B,	// deprecated, same as COMPRESSED_SLUMINANCE_ALPHA_EXT
COMPRESSED_INTENSITY							= 0x84EC,	// deprecated, same as COMPRESSED_INTENSITY_ARB
COMPRESSED_RG								= 0x8226,
COMPRESSED_RGB								= 0x84ED,	// same as COMPRESSED_RGB_ARB
COMPRESSED_RGBA								= 0x84EE,	// same as COMPRESSED_RGBA_ARB
COMPRESSED_SRGB								= 0x8C48,	// same as COMPRESSED_SRGB_EXT
COMPRESSED_SRGB_ALPHA						= 0x8C49,	// same as COMPRESSED_SRGB_ALPHA_EXT

//
// FXT1
//
COMPRESSED_RGB_FXT1_3DFX						= 0x86B0,	// deprecated
COMPRESSED_RGBA_FXT1_3DFX					= 0x86B1,	// deprecated

//
// S3TC/DXT/BC
//
COMPRESSED_RGB_S3TC_DXT1_EXT					= 0x83F0,
COMPRESSED_RGBA_S3TC_DXT1_EXT				= 0x83F1,
COMPRESSED_RGBA_S3TC_DXT3_EXT				= 0x83F2,
COMPRESSED_RGBA_S3TC_DXT5_EXT				= 0x83F3,
COMPRESSED_SRGB_S3TC_DXT1_EXT				= 0x8C4C,
COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT			= 0x8C4D,
COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT			= 0x8C4E,
COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT			= 0x8C4F,
COMPRESSED_LUMINANCE_LATC1_EXT				= 0x8C70,
COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT			= 0x8C72,
COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT		= 0x8C71,
COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT	= 0x8C73,
COMPRESSED_RED_RGTC1							= 0x8DBB,	// same as COMPRESSED_RED_RGTC1_EXT
COMPRESSED_RG_RGTC2							= 0x8DBD,	// same as COMPRESSED_RG_RGTC2_EXT
COMPRESSED_SIGNED_RED_RGTC1					= 0x8DBC,	// same as COMPRESSED_SIGNED_RED_RGTC1_EXT
COMPRESSED_SIGNED_RG_RGTC2					= 0x8DBE,	// same as COMPRESSED_SIGNED_RG_RGTC2_EXT
COMPRESSED_RGB_BPTC_SIGNED_FLOAT				= 0x8E8E,	// same as COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT			= 0x8E8F,	// same as COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
COMPRESSED_RGBA_BPTC_UNORM					= 0x8E8C,	// same as COMPRESSED_RGBA_BPTC_UNORM_ARB	
COMPRESSED_SRGB_ALPHA_BPTC_UNORM				= 0x8E8D,	// same as COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB

//
// ETC
//
ETC1_RGB8_OES								= 0x8D64,
COMPRESSED_RGB8_ETC2							= 0x9274,
COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2		= 0x9276,
COMPRESSED_RGBA8_ETC2_EAC					= 0x9278,
COMPRESSED_SRGB8_ETC2						= 0x9275,
COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2	= 0x9277,
COMPRESSED_SRGB8_ALPHA8_ETC2_EAC				= 0x9279,
COMPRESSED_R11_EAC							= 0x9270,
COMPRESSED_RG11_EAC							= 0x9272,
COMPRESSED_SIGNED_R11_EAC					= 0x9271,
COMPRESSED_SIGNED_RG11_EAC					= 0x9273,

//
// PVRTC
//
COMPRESSED_RGB_PVRTC_2BPPV1_IMG				= 0x8C01,
COMPRESSED_RGB_PVRTC_4BPPV1_IMG				= 0x8C00,
COMPRESSED_RGBA_PVRTC_2BPPV1_IMG				= 0x8C03,
COMPRESSED_RGBA_PVRTC_4BPPV1_IMG				= 0x8C02,
COMPRESSED_RGBA_PVRTC_2BPPV2_IMG				= 0x9137,
COMPRESSED_RGBA_PVRTC_4BPPV2_IMG				= 0x9138,
COMPRESSED_SRGB_PVRTC_2BPPV1_EXT				= 0x8A54,
COMPRESSED_SRGB_PVRTC_4BPPV1_EXT				= 0x8A55,
COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT		= 0x8A56,
COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT		= 0x8A57,
COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG		= 0x93F0,
COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG		= 0x93F1,

//
// ASTC
//
COMPRESSED_RGBA_ASTC_4x4_KHR					= 0x93B0,
COMPRESSED_RGBA_ASTC_5x4_KHR					= 0x93B1,
COMPRESSED_RGBA_ASTC_5x5_KHR					= 0x93B2,
COMPRESSED_RGBA_ASTC_6x5_KHR					= 0x93B3,
COMPRESSED_RGBA_ASTC_6x6_KHR					= 0x93B4,
COMPRESSED_RGBA_ASTC_8x5_KHR					= 0x93B5,
COMPRESSED_RGBA_ASTC_8x6_KHR					= 0x93B6,
COMPRESSED_RGBA_ASTC_8x8_KHR					= 0x93B7,
COMPRESSED_RGBA_ASTC_10x5_KHR				= 0x93B8,
COMPRESSED_RGBA_ASTC_10x6_KHR				= 0x93B9,
COMPRESSED_RGBA_ASTC_10x8_KHR				= 0x93BA,
COMPRESSED_RGBA_ASTC_10x10_KHR				= 0x93BB,
COMPRESSED_RGBA_ASTC_12x10_KHR				= 0x93BC,
COMPRESSED_RGBA_ASTC_12x12_KHR				= 0x93BD,
COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR			= 0x93D0,
COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR			= 0x93D1,
COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR			= 0x93D2,
COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR			= 0x93D3,
COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR			= 0x93D4,
COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR			= 0x93D5,
COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR			= 0x93D6,
COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR			= 0x93D7,
COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR		= 0x93D8,
COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR		= 0x93D9,
COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR		= 0x93DA,
COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR		= 0x93DB,
COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR		= 0x93DC,
COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR		= 0x93DD,
COMPRESSED_RGBA_ASTC_3x3x3_OES				= 0x93C0,
COMPRESSED_RGBA_ASTC_4x3x3_OES				= 0x93C1,
COMPRESSED_RGBA_ASTC_4x4x3_OES				= 0x93C2,
COMPRESSED_RGBA_ASTC_4x4x4_OES				= 0x93C3,
COMPRESSED_RGBA_ASTC_5x4x4_OES				= 0x93C4,
COMPRESSED_RGBA_ASTC_5x5x4_OES				= 0x93C5,
COMPRESSED_RGBA_ASTC_5x5x5_OES				= 0x93C6,
COMPRESSED_RGBA_ASTC_6x5x5_OES				= 0x93C7,
COMPRESSED_RGBA_ASTC_6x6x5_OES				= 0x93C8,
COMPRESSED_RGBA_ASTC_6x6x6_OES				= 0x93C9,
COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES		= 0x93E0,
COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES		= 0x93E1,
COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES		= 0x93E2,
COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES		= 0x93E3,
COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES		= 0x93E4,
COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES		= 0x93E5,
COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES		= 0x93E6,
COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES		= 0x93E7,
COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES		= 0x93E8,
COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES		= 0x93E9,

//
// ATC
//
ATC_RGB_AMD									= 0x8C92,
ATC_RGBA_EXPLICIT_ALPHA_AMD					= 0x8C93,
ATC_RGBA_INTERPOLATED_ALPHA_AMD				= 0x87EE,

//
// Palletized (combined palette)
//
PALETTE4_RGB8_OES							= 0x8B90,
PALETTE4_RGBA8_OES							= 0x8B91,
PALETTE4_R5_G6_B5_OES						= 0x8B92,
PALETTE4_RGBA4_OES							= 0x8B93,
PALETTE4_RGB5_A1_OES							= 0x8B94,
PALETTE8_RGB8_OES							= 0x8B95,
PALETTE8_RGBA8_OES							= 0x8B96,
PALETTE8_R5_G6_B5_OES						= 0x8B97,
PALETTE8_RGBA4_OES							= 0x8B98,
PALETTE8_RGB5_A1_OES							= 0x8B99,

//
// Palletized (separate palette)
//
COLOR_INDEX1_EXT								= 0x80E2,	// deprecated
COLOR_INDEX2_EXT								= 0x80E3,	// deprecated
COLOR_INDEX4_EXT								= 0x80E4,	// deprecated
COLOR_INDEX8_EXT								= 0x80E5,	// deprecated
COLOR_INDEX12_EXT							= 0x80E6,	// deprecated
COLOR_INDEX16_EXT							= 0x80E7,	// deprecated

//
// Depth/stencil
//
DEPTH_COMPONENT16							= 0x81A5,	// same as DEPTH_COMPONENT16_SGIX and DEPTH_COMPONENT16_ARB
DEPTH_COMPONENT24							= 0x81A6,	// same as DEPTH_COMPONENT24_SGIX and DEPTH_COMPONENT24_ARB
DEPTH_COMPONENT32							= 0x81A7,	// same as DEPTH_COMPONENT32_SGIX and DEPTH_COMPONENT32_ARB and DEPTH_COMPONENT32_OES
DEPTH_COMPONENT32F							= 0x8CAC,	// same as DEPTH_COMPONENT32F_ARB
DEPTH_COMPONENT32F_NV						= 0x8DAB,	// note that this is different from DEPTH_COMPONENT32F
STENCIL_INDEX1								= 0x8D46,	// same as STENCIL_INDEX1_EXT
STENCIL_INDEX4								= 0x8D47,	// same as STENCIL_INDEX4_EXT
STENCIL_INDEX8								= 0x8D48,	// same as STENCIL_INDEX8_EXT
STENCIL_INDEX16								= 0x8D49,	// same as STENCIL_INDEX16_EXT
DEPTH24_STENCIL8								= 0x88F0,	// same as DEPTH24_STENCIL8_EXT and DEPTH24_STENCIL8_OES
DEPTH32F_STENCIL8							= 0x8CAD,	// same as DEPTH32F_STENCIL8_ARB
DEPTH32F_STENCIL8_NV							= 0x8DAC,	// note that this is different from DEPTH32F_STENCIL8
};

static inline GlFormat GetFormatFromInternalFormat( const GlFormat internalFormat )
{
	switch ( internalFormat )
	{
		//
		// 8 bits per component
		//
		case GlFormat::R8:												return GlFormat::RED;		// 1-component, 8-bit unsigned normalized
		case GlFormat::RG8:											return GlFormat::RG;		// 2-component, 8-bit unsigned normalized
		case GlFormat::RGB8:											return GlFormat::RGB;		// 3-component, 8-bit unsigned normalized
		case GlFormat::RGBA8:											return GlFormat::RGBA;		// 4-component, 8-bit unsigned normalized

		case GlFormat::R8_SNORM:										return GlFormat::RED;		// 1-component, 8-bit signed normalized
		case GlFormat::RG8_SNORM:										return GlFormat::RG;		// 2-component, 8-bit signed normalized
		case GlFormat::RGB8_SNORM:										return GlFormat::RGB;		// 3-component, 8-bit signed normalized
		case GlFormat::RGBA8_SNORM:									return GlFormat::RGBA;		// 4-component, 8-bit signed normalized

		case GlFormat::R8UI:											return GlFormat::RED;		// 1-component, 8-bit unsigned integer
		case GlFormat::RG8UI:											return GlFormat::RG;		// 2-component, 8-bit unsigned integer
		case GlFormat::RGB8UI:											return GlFormat::RGB;		// 3-component, 8-bit unsigned integer
		case GlFormat::RGBA8UI:										return GlFormat::RGBA;		// 4-component, 8-bit unsigned integer

		case GlFormat::R8I:											return GlFormat::RED;		// 1-component, 8-bit signed integer
		case GlFormat::RG8I:											return GlFormat::RG;		// 2-component, 8-bit signed integer
		case GlFormat::RGB8I:											return GlFormat::RGB;		// 3-component, 8-bit signed integer
		case GlFormat::RGBA8I:											return GlFormat::RGBA;		// 4-component, 8-bit signed integer

		case GlFormat::SR8:											return GlFormat::RED;		// 1-component, 8-bit sRGB
		case GlFormat::SRG8:											return GlFormat::RG;		// 2-component, 8-bit sRGB
		case GlFormat::SRGB8:											return GlFormat::RGB;		// 3-component, 8-bit sRGB
		case GlFormat::SRGB8_ALPHA8:									return GlFormat::RGBA;		// 4-component, 8-bit sRGB

		//
		// 16 bits per component
		//
		case GlFormat::R16:											return GlFormat::RED;		// 1-component, 16-bit unsigned normalized
		case GlFormat::RG16:											return GlFormat::RG;		// 2-component, 16-bit unsigned normalized
		case GlFormat::RGB16:											return GlFormat::RGB;		// 3-component, 16-bit unsigned normalized
		case GlFormat::RGBA16:											return GlFormat::RGBA;		// 4-component, 16-bit unsigned normalized

		case GlFormat::R16_SNORM:										return GlFormat::RED;		// 1-component, 16-bit signed normalized
		case GlFormat::RG16_SNORM:										return GlFormat::RG;		// 2-component, 16-bit signed normalized
		case GlFormat::RGB16_SNORM:									return GlFormat::RGB;		// 3-component, 16-bit signed normalized
		case GlFormat::RGBA16_SNORM:									return GlFormat::RGBA;		// 4-component, 16-bit signed normalized

		case GlFormat::R16UI:											return GlFormat::RED;		// 1-component, 16-bit unsigned integer
		case GlFormat::RG16UI:											return GlFormat::RG;		// 2-component, 16-bit unsigned integer
		case GlFormat::RGB16UI:										return GlFormat::RGB;		// 3-component, 16-bit unsigned integer
		case GlFormat::RGBA16UI:										return GlFormat::RGBA;		// 4-component, 16-bit unsigned integer

		case GlFormat::R16I:											return GlFormat::RED;		// 1-component, 16-bit signed integer
		case GlFormat::RG16I:											return GlFormat::RG;		// 2-component, 16-bit signed integer
		case GlFormat::RGB16I:											return GlFormat::RGB;		// 3-component, 16-bit signed integer
		case GlFormat::RGBA16I:										return GlFormat::RGBA;		// 4-component, 16-bit signed integer

		case GlFormat::R16F:											return GlFormat::RED;		// 1-component, 16-bit floating-point
		case GlFormat::RG16F:											return GlFormat::RG;		// 2-component, 16-bit floating-point
		case GlFormat::RGB16F:											return GlFormat::RGB;		// 3-component, 16-bit floating-point
		case GlFormat::RGBA16F:										return GlFormat::RGBA;		// 4-component, 16-bit floating-point

		//
		// 32 bits per component
		//
		case GlFormat::R32UI:											return GlFormat::RED;		// 1-component, 32-bit unsigned integer
		case GlFormat::RG32UI:											return GlFormat::RG;		// 2-component, 32-bit unsigned integer
		case GlFormat::RGB32UI:										return GlFormat::RGB;		// 3-component, 32-bit unsigned integer
		case GlFormat::RGBA32UI:										return GlFormat::RGBA;		// 4-component, 32-bit unsigned integer

		case GlFormat::R32I:											return GlFormat::RED;		// 1-component, 32-bit signed integer
		case GlFormat::RG32I:											return GlFormat::RG;		// 2-component, 32-bit signed integer
		case GlFormat::RGB32I:											return GlFormat::RGB;		// 3-component, 32-bit signed integer
		case GlFormat::RGBA32I:										return GlFormat::RGBA;		// 4-component, 32-bit signed integer

		case GlFormat::R32F:											return GlFormat::RED;		// 1-component, 32-bit floating-point
		case GlFormat::RG32F:											return GlFormat::RG;		// 2-component, 32-bit floating-point
		case GlFormat::RGB32F:											return GlFormat::RGB;		// 3-component, 32-bit floating-point
		case GlFormat::RGBA32F:										return GlFormat::RGBA;		// 4-component, 32-bit floating-point

		//
		// Packed
		//
		case GlFormat::R3_G3_B2:										return GlFormat::RGB;		// 3-component 3:3:2,       unsigned normalized
		case GlFormat::RGB4:											return GlFormat::RGB;		// 3-component 4:4:4,       unsigned normalized
		case GlFormat::RGB5:											return GlFormat::RGB;		// 3-component 5:5:5,       unsigned normalized
		case GlFormat::RGB565:											return GlFormat::RGB;		// 3-component 5:6:5,       unsigned normalized
		case GlFormat::RGB10:											return GlFormat::RGB;		// 3-component 10:10:10,    unsigned normalized
		case GlFormat::RGB12:											return GlFormat::RGB;		// 3-component 12:12:12,    unsigned normalized
		case GlFormat::RGBA2:											return GlFormat::RGBA;		// 4-component 2:2:2:2,     unsigned normalized
		case GlFormat::RGBA4:											return GlFormat::RGBA;		// 4-component 4:4:4:4,     unsigned normalized
		case GlFormat::RGBA12:											return GlFormat::RGBA;		// 4-component 12:12:12:12, unsigned normalized
		case GlFormat::RGB5_A1:										return GlFormat::RGBA;		// 4-component 5:5:5:1,     unsigned normalized
		case GlFormat::RGB10_A2:										return GlFormat::RGBA;		// 4-component 10:10:10:2,  unsigned normalized
		case GlFormat::RGB10_A2UI:										return GlFormat::RGBA;		// 4-component 10:10:10:2,  unsigned integer
		case GlFormat::R11F_G11F_B10F:									return GlFormat::RGB;		// 3-component 11:11:10,    floating-point
		case GlFormat::RGB9_E5:										return GlFormat::RGB;		// 3-component/exp 9:9:9/5, floating-point

		//
		// S3TC/DXT/BC
		//

		case GlFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:					return GlFormat::RGB;		// line through 3D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:					return GlFormat::RGBA;		// line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:					return GlFormat::RGBA;		// line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:					return GlFormat::RGBA;		// line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:					return GlFormat::RGB;		// line through 3D space, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:			return GlFormat::RGBA;		// line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:			return GlFormat::RGBA;		// line through 3D space plus line through 1D space, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:			return GlFormat::RGBA;		// line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

		case GlFormat::COMPRESSED_LUMINANCE_LATC1_EXT:					return GlFormat::RED;		// line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:			return GlFormat::RG;		// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:			return GlFormat::RED;		// line through 1D space, 4x4 blocks, signed normalized
		case GlFormat::COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:	return GlFormat::RG;		// two lines through 1D space, 4x4 blocks, signed normalized

		case GlFormat::COMPRESSED_RED_RGTC1:							return GlFormat::RED;		// line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RG_RGTC2:							return GlFormat::RG;		// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_RED_RGTC1:					return GlFormat::RED;		// line through 1D space, 4x4 blocks, signed normalized
		case GlFormat::COMPRESSED_SIGNED_RG_RGTC2:						return GlFormat::RG;		// two lines through 1D space, 4x4 blocks, signed normalized

		case GlFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:				return GlFormat::RGB;		// 3-component, 4x4 blocks, unsigned floating-point
		case GlFormat::COMPRESSED_RGB_BPTC_SIGNED_FLOAT:				return GlFormat::RGB;		// 3-component, 4x4 blocks, signed floating-point
		case GlFormat::COMPRESSED_RGBA_BPTC_UNORM:						return GlFormat::RGBA;		// 4-component, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM:				return GlFormat::RGBA;		// 4-component, 4x4 blocks, sRGB

		//
		// ETC
		//
		case GlFormat::ETC1_RGB8_OES:									return GlFormat::RGB;		// 3-component ETC1, 4x4 blocks, unsigned normalized

		case GlFormat::COMPRESSED_RGB8_ETC2:							return GlFormat::RGB;		// 3-component ETC2, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return GlFormat::RGBA;		// 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA8_ETC2_EAC:						return GlFormat::RGBA;		// 4-component ETC2, 4x4 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB8_ETC2:							return GlFormat::RGB;		// 3-component ETC2, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return GlFormat::RGBA;		// 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:				return GlFormat::RGBA;		// 4-component ETC2, 4x4 blocks, sRGB

		case GlFormat::COMPRESSED_R11_EAC:								return GlFormat::RED;		// 1-component ETC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RG11_EAC:							return GlFormat::RG;		// 2-component ETC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_R11_EAC:						return GlFormat::RED;		// 1-component ETC, 4x4 blocks, signed normalized
		case GlFormat::COMPRESSED_SIGNED_RG11_EAC:						return GlFormat::RG;		// 2-component ETC, 4x4 blocks, signed normalized

		//
		// PVRTC
		//
		case GlFormat::COMPRESSED_RGB_PVRTC_2BPPV1_IMG:				return GlFormat::RGB;		// 3-component PVRTC, 16x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGB_PVRTC_4BPPV1_IMG:				return GlFormat::RGB;		// 3-component PVRTC,  8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:				return GlFormat::RGBA;		// 4-component PVRTC, 16x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:				return GlFormat::RGBA;		// 4-component PVRTC,  8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:				return GlFormat::RGBA;		// 4-component PVRTC,  8x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:				return GlFormat::RGBA;		// 4-component PVRTC,  4x4 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:				return GlFormat::RGB;		// 3-component PVRTC, 16x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:				return GlFormat::RGB;		// 3-component PVRTC,  8x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:			return GlFormat::RGBA;		// 4-component PVRTC, 16x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:			return GlFormat::RGBA;		// 4-component PVRTC,  8x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:			return GlFormat::RGBA;		// 4-component PVRTC,  8x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:			return GlFormat::RGBA;		// 4-component PVRTC,  4x4 blocks, sRGB

		//
		// ASTC
		//
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x4_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 5x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x5_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 6x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 6x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_8x5_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 8x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_8x6_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 8x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_8x8_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_10x5_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 10x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_10x6_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 10x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_10x8_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 10x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_10x10_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 10x10 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_12x10_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 12x10 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_12x12_KHR:					return GlFormat::RGBA;		// 4-component ASTC, 12x12 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 5x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 5x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 6x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 6x6 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 8x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 8x6 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 8x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 10x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 10x6 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 10x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 10x10 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 12x10 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:			return GlFormat::RGBA;		// 4-component ASTC, 12x12 blocks, sRGB

		case GlFormat::COMPRESSED_RGBA_ASTC_3x3x3_OES:					return GlFormat::RGBA;		// 4-component ASTC, 3x3x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_4x3x3_OES:					return GlFormat::RGBA;		// 4-component ASTC, 4x3x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4x3_OES:					return GlFormat::RGBA;		// 4-component ASTC, 4x4x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4x4_OES:					return GlFormat::RGBA;		// 4-component ASTC, 4x4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x4x4_OES:					return GlFormat::RGBA;		// 4-component ASTC, 5x4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5x4_OES:					return GlFormat::RGBA;		// 4-component ASTC, 5x5x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5x5_OES:					return GlFormat::RGBA;		// 4-component ASTC, 5x5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x5x5_OES:					return GlFormat::RGBA;		// 4-component ASTC, 6x5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6x5_OES:					return GlFormat::RGBA;		// 4-component ASTC, 6x6x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6x6_OES:					return GlFormat::RGBA;		// 4-component ASTC, 6x6x6 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:			return GlFormat::RGBA;		// 4-component ASTC, 3x3x3 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:			return GlFormat::RGBA;		// 4-component ASTC, 4x3x3 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:			return GlFormat::RGBA;		// 4-component ASTC, 4x4x3 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:			return GlFormat::RGBA;		// 4-component ASTC, 4x4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:			return GlFormat::RGBA;		// 4-component ASTC, 5x4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:			return GlFormat::RGBA;		// 4-component ASTC, 5x5x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:			return GlFormat::RGBA;		// 4-component ASTC, 5x5x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:			return GlFormat::RGBA;		// 4-component ASTC, 6x5x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:			return GlFormat::RGBA;		// 4-component ASTC, 6x6x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:			return GlFormat::RGBA;		// 4-component ASTC, 6x6x6 blocks, sRGB

		//
		// ATC
		//
		case GlFormat::ATC_RGB_AMD:									return GlFormat::RGB;		// 3-component, 4x4 blocks, unsigned normalized
		case GlFormat::ATC_RGBA_EXPLICIT_ALPHA_AMD:					return GlFormat::RGBA;		// 4-component, 4x4 blocks, unsigned normalized
		case GlFormat::ATC_RGBA_INTERPOLATED_ALPHA_AMD:				return GlFormat::RGBA;		// 4-component, 4x4 blocks, unsigned normalized

		//
		// Palletized
		//
		case GlFormat::PALETTE4_RGB8_OES:								return GlFormat::RGB;		// 3-component 8:8:8,   4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_RGBA8_OES:								return GlFormat::RGBA;		// 4-component 8:8:8:8, 4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_R5_G6_B5_OES:							return GlFormat::RGB;		// 3-component 5:6:5,   4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_RGBA4_OES:								return GlFormat::RGBA;		// 4-component 4:4:4:4, 4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_RGB5_A1_OES:							return GlFormat::RGBA;		// 4-component 5:5:5:1, 4-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGB8_OES:								return GlFormat::RGB;		// 3-component 8:8:8,   8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGBA8_OES:								return GlFormat::RGBA;		// 4-component 8:8:8:8, 8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_R5_G6_B5_OES:							return GlFormat::RGB;		// 3-component 5:6:5,   8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGBA4_OES:								return GlFormat::RGBA;		// 4-component 4:4:4:4, 8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGB5_A1_OES:							return GlFormat::RGBA;		// 4-component 5:5:5:1, 8-bit palette, unsigned normalized

		//
		// Depth/stencil
		//
		case GlFormat::DEPTH_COMPONENT16:								return GlFormat::DEPTH_COMPONENT;
		case GlFormat::DEPTH_COMPONENT24:								return GlFormat::DEPTH_COMPONENT;
		case GlFormat::DEPTH_COMPONENT32:								return GlFormat::DEPTH_COMPONENT;
		case GlFormat::DEPTH_COMPONENT32F:								return GlFormat::DEPTH_COMPONENT;
		case GlFormat::DEPTH_COMPONENT32F_NV:							return GlFormat::DEPTH_COMPONENT;
		case GlFormat::STENCIL_INDEX1:									return GlFormat::STENCIL_INDEX;
		case GlFormat::STENCIL_INDEX4:									return GlFormat::STENCIL_INDEX;
		case GlFormat::STENCIL_INDEX8:									return GlFormat::STENCIL_INDEX;
		case GlFormat::STENCIL_INDEX16:								return GlFormat::STENCIL_INDEX;
		case GlFormat::DEPTH24_STENCIL8:								return GlFormat::DEPTH_STENCIL;
		case GlFormat::DEPTH32F_STENCIL8:								return GlFormat::DEPTH_STENCIL;
		case GlFormat::DEPTH32F_STENCIL8_NV:							return GlFormat::DEPTH_STENCIL;

		default:												return GlFormat::INVALID_VALUE;
	}
}

static inline GlFormat GetTypeFromInternalFormat( const GlFormat internalFormat )
{
	switch ( internalFormat )
	{
		//
		// 8 bits per component
		//
		case GlFormat::R8:												return GlFormat::UNSIGNED_BYTE;				// 1-component, 8-bit unsigned normalized
		case GlFormat::RG8:											return GlFormat::UNSIGNED_BYTE;				// 2-component, 8-bit unsigned normalized
		case GlFormat::RGB8:											return GlFormat::UNSIGNED_BYTE;				// 3-component, 8-bit unsigned normalized
		case GlFormat::RGBA8:											return GlFormat::UNSIGNED_BYTE;				// 4-component, 8-bit unsigned normalized

		case GlFormat::R8_SNORM:										return GlFormat::BYTE;							// 1-component, 8-bit signed normalized
		case GlFormat::RG8_SNORM:										return GlFormat::BYTE;							// 2-component, 8-bit signed normalized
		case GlFormat::RGB8_SNORM:										return GlFormat::BYTE;							// 3-component, 8-bit signed normalized
		case GlFormat::RGBA8_SNORM:									return GlFormat::BYTE;							// 4-component, 8-bit signed normalized

		case GlFormat::R8UI:											return GlFormat::UNSIGNED_BYTE;				// 1-component, 8-bit unsigned integer
		case GlFormat::RG8UI:											return GlFormat::UNSIGNED_BYTE;				// 2-component, 8-bit unsigned integer
		case GlFormat::RGB8UI:											return GlFormat::UNSIGNED_BYTE;				// 3-component, 8-bit unsigned integer
		case GlFormat::RGBA8UI:										return GlFormat::UNSIGNED_BYTE;				// 4-component, 8-bit unsigned integer

		case GlFormat::R8I:											return GlFormat::BYTE;							// 1-component, 8-bit signed integer
		case GlFormat::RG8I:											return GlFormat::BYTE;							// 2-component, 8-bit signed integer
		case GlFormat::RGB8I:											return GlFormat::BYTE;							// 3-component, 8-bit signed integer
		case GlFormat::RGBA8I:											return GlFormat::BYTE;							// 4-component, 8-bit signed integer

		case GlFormat::SR8:											return GlFormat::UNSIGNED_BYTE;				// 1-component, 8-bit sRGB
		case GlFormat::SRG8:											return GlFormat::UNSIGNED_BYTE;				// 2-component, 8-bit sRGB
		case GlFormat::SRGB8:											return GlFormat::UNSIGNED_BYTE;				// 3-component, 8-bit sRGB
		case GlFormat::SRGB8_ALPHA8:									return GlFormat::UNSIGNED_BYTE;				// 4-component, 8-bit sRGB

		//
		// 16 bits per component
		//
		case GlFormat::R16:											return GlFormat::UNSIGNED_SHORT;				// 1-component, 16-bit unsigned normalized
		case GlFormat::RG16:											return GlFormat::UNSIGNED_SHORT;				// 2-component, 16-bit unsigned normalized
		case GlFormat::RGB16:											return GlFormat::UNSIGNED_SHORT;				// 3-component, 16-bit unsigned normalized
		case GlFormat::RGBA16:											return GlFormat::UNSIGNED_SHORT;				// 4-component, 16-bit unsigned normalized

		case GlFormat::R16_SNORM:										return GlFormat::SHORT;						// 1-component, 16-bit signed normalized
		case GlFormat::RG16_SNORM:										return GlFormat::SHORT;						// 2-component, 16-bit signed normalized
		case GlFormat::RGB16_SNORM:									return GlFormat::SHORT;						// 3-component, 16-bit signed normalized
		case GlFormat::RGBA16_SNORM:									return GlFormat::SHORT;						// 4-component, 16-bit signed normalized

		case GlFormat::R16UI:											return GlFormat::UNSIGNED_SHORT;				// 1-component, 16-bit unsigned integer
		case GlFormat::RG16UI:											return GlFormat::UNSIGNED_SHORT;				// 2-component, 16-bit unsigned integer
		case GlFormat::RGB16UI:										return GlFormat::UNSIGNED_SHORT;				// 3-component, 16-bit unsigned integer
		case GlFormat::RGBA16UI:										return GlFormat::UNSIGNED_SHORT;				// 4-component, 16-bit unsigned integer

		case GlFormat::R16I:											return GlFormat::SHORT;						// 1-component, 16-bit signed integer
		case GlFormat::RG16I:											return GlFormat::SHORT;						// 2-component, 16-bit signed integer
		case GlFormat::RGB16I:											return GlFormat::SHORT;						// 3-component, 16-bit signed integer
		case GlFormat::RGBA16I:										return GlFormat::SHORT;						// 4-component, 16-bit signed integer

		case GlFormat::R16F:											return GlFormat::HALF_FLOAT;					// 1-component, 16-bit floating-point
		case GlFormat::RG16F:											return GlFormat::HALF_FLOAT;					// 2-component, 16-bit floating-point
		case GlFormat::RGB16F:											return GlFormat::HALF_FLOAT;					// 3-component, 16-bit floating-point
		case GlFormat::RGBA16F:										return GlFormat::HALF_FLOAT;					// 4-component, 16-bit floating-point

		//
		// 32 bits per component
		//
		case GlFormat::R32UI:											return GlFormat::UNSIGNED_INT;					// 1-component, 32-bit unsigned integer
		case GlFormat::RG32UI:											return GlFormat::UNSIGNED_INT;					// 2-component, 32-bit unsigned integer
		case GlFormat::RGB32UI:										return GlFormat::UNSIGNED_INT;					// 3-component, 32-bit unsigned integer
		case GlFormat::RGBA32UI:										return GlFormat::UNSIGNED_INT;					// 4-component, 32-bit unsigned integer

		case GlFormat::R32I:											return GlFormat::INT;							// 1-component, 32-bit signed integer
		case GlFormat::RG32I:											return GlFormat::INT;							// 2-component, 32-bit signed integer
		case GlFormat::RGB32I:											return GlFormat::INT;							// 3-component, 32-bit signed integer
		case GlFormat::RGBA32I:										return GlFormat::INT;							// 4-component, 32-bit signed integer

		case GlFormat::R32F:											return GlFormat::FLOAT;						// 1-component, 32-bit floating-point
		case GlFormat::RG32F:											return GlFormat::FLOAT;						// 2-component, 32-bit floating-point
		case GlFormat::RGB32F:											return GlFormat::FLOAT;						// 3-component, 32-bit floating-point
		case GlFormat::RGBA32F:										return GlFormat::FLOAT;						// 4-component, 32-bit floating-point

		//
		// Packed
		//
		case GlFormat::R3_G3_B2:										return GlFormat::UNSIGNED_BYTE_2_3_3_REV;		// 3-component 3:3:2,       unsigned normalized
		case GlFormat::RGB4:											return GlFormat::UNSIGNED_SHORT_4_4_4_4;		// 3-component 4:4:4,       unsigned normalized
		case GlFormat::RGB5:											return GlFormat::UNSIGNED_SHORT_5_5_5_1;		// 3-component 5:5:5,       unsigned normalized
		case GlFormat::RGB565:											return GlFormat::UNSIGNED_SHORT_5_6_5;			// 3-component 5:6:5,       unsigned normalized
		case GlFormat::RGB10:											return GlFormat::UNSIGNED_INT_10_10_10_2;		// 3-component 10:10:10,    unsigned normalized
		case GlFormat::RGB12:											return GlFormat::UNSIGNED_SHORT;				// 3-component 12:12:12,    unsigned normalized
		case GlFormat::RGBA2:											return GlFormat::UNSIGNED_BYTE;				// 4-component 2:2:2:2,     unsigned normalized
		case GlFormat::RGBA4:											return GlFormat::UNSIGNED_SHORT_4_4_4_4;		// 4-component 4:4:4:4,     unsigned normalized
		case GlFormat::RGBA12:											return GlFormat::UNSIGNED_SHORT;				// 4-component 12:12:12:12, unsigned normalized
		case GlFormat::RGB5_A1:										return GlFormat::UNSIGNED_SHORT_5_5_5_1;		// 4-component 5:5:5:1,     unsigned normalized
		case GlFormat::RGB10_A2:										return GlFormat::UNSIGNED_INT_2_10_10_10_REV;	// 4-component 10:10:10:2,  unsigned normalized
		case GlFormat::RGB10_A2UI:										return GlFormat::UNSIGNED_INT_2_10_10_10_REV;	// 4-component 10:10:10:2,  unsigned integer
		case GlFormat::R11F_G11F_B10F:									return GlFormat::UNSIGNED_INT_10F_11F_11F_REV;	// 3-component 11:11:10,    floating-point
		case GlFormat::RGB9_E5:										return GlFormat::UNSIGNED_INT_5_9_9_9_REV;		// 3-component/exp 9:9:9/5, floating-point

		//
		// S3TC/DXT/BC
		//

		case GlFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:					return GlFormat::UNSIGNED_BYTE;				// line through 3D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:					return GlFormat::UNSIGNED_BYTE;				// line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:					return GlFormat::UNSIGNED_BYTE;				// line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:					return GlFormat::UNSIGNED_BYTE;				// line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:					return GlFormat::UNSIGNED_BYTE;				// line through 3D space, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:			return GlFormat::UNSIGNED_BYTE;				// line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:			return GlFormat::UNSIGNED_BYTE;				// line through 3D space plus line through 1D space, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:			return GlFormat::UNSIGNED_BYTE;				// line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

		case GlFormat::COMPRESSED_LUMINANCE_LATC1_EXT:					return GlFormat::UNSIGNED_BYTE;				// line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:			return GlFormat::UNSIGNED_BYTE;				// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:			return GlFormat::UNSIGNED_BYTE;				// line through 1D space, 4x4 blocks, signed normalized
		case GlFormat::COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:	return GlFormat::UNSIGNED_BYTE;				// two lines through 1D space, 4x4 blocks, signed normalized

		case GlFormat::COMPRESSED_RED_RGTC1:							return GlFormat::UNSIGNED_BYTE;				// line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RG_RGTC2:							return GlFormat::UNSIGNED_BYTE;				// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_RED_RGTC1:					return GlFormat::UNSIGNED_BYTE;				// line through 1D space, 4x4 blocks, signed normalized
		case GlFormat::COMPRESSED_SIGNED_RG_RGTC2:						return GlFormat::UNSIGNED_BYTE;				// two lines through 1D space, 4x4 blocks, signed normalized

		case GlFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:				return GlFormat::FLOAT;						// 3-component, 4x4 blocks, unsigned floating-point
		case GlFormat::COMPRESSED_RGB_BPTC_SIGNED_FLOAT:				return GlFormat::FLOAT;						// 3-component, 4x4 blocks, signed floating-point
		case GlFormat::COMPRESSED_RGBA_BPTC_UNORM:						return GlFormat::UNSIGNED_BYTE;				// 4-component, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM:				return GlFormat::UNSIGNED_BYTE;				// 4-component, 4x4 blocks, sRGB

		//
		// ETC
		//
		case GlFormat::ETC1_RGB8_OES:									return GlFormat::UNSIGNED_BYTE;				// 3-component ETC1, 4x4 blocks, unsigned normalized" ),

		case GlFormat::COMPRESSED_RGB8_ETC2:							return GlFormat::UNSIGNED_BYTE;				// 3-component ETC2, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return GlFormat::UNSIGNED_BYTE;				// 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA8_ETC2_EAC:						return GlFormat::UNSIGNED_BYTE;				// 4-component ETC2, 4x4 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB8_ETC2:							return GlFormat::UNSIGNED_BYTE;				// 3-component ETC2, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return GlFormat::UNSIGNED_BYTE;				// 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:				return GlFormat::UNSIGNED_BYTE;				// 4-component ETC2, 4x4 blocks, sRGB

		case GlFormat::COMPRESSED_R11_EAC:								return GlFormat::UNSIGNED_BYTE;				// 1-component ETC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RG11_EAC:							return GlFormat::UNSIGNED_BYTE;				// 2-component ETC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_R11_EAC:						return GlFormat::UNSIGNED_BYTE;				// 1-component ETC, 4x4 blocks, signed normalized
		case GlFormat::COMPRESSED_SIGNED_RG11_EAC:						return GlFormat::UNSIGNED_BYTE;				// 2-component ETC, 4x4 blocks, signed normalized

		//
		// PVRTC
		//
		case GlFormat::COMPRESSED_RGB_PVRTC_2BPPV1_IMG:				return GlFormat::UNSIGNED_BYTE;				// 3-component PVRTC, 16x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGB_PVRTC_4BPPV1_IMG:				return GlFormat::UNSIGNED_BYTE;				// 3-component PVRTC,  8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:				return GlFormat::UNSIGNED_BYTE;				// 4-component PVRTC, 16x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:				return GlFormat::UNSIGNED_BYTE;				// 4-component PVRTC,  8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:				return GlFormat::UNSIGNED_BYTE;				// 4-component PVRTC,  8x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:				return GlFormat::UNSIGNED_BYTE;				// 4-component PVRTC,  4x4 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:				return GlFormat::UNSIGNED_BYTE;				// 3-component PVRTC, 16x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:				return GlFormat::UNSIGNED_BYTE;				// 3-component PVRTC,  8x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:			return GlFormat::UNSIGNED_BYTE;				// 4-component PVRTC, 16x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:			return GlFormat::UNSIGNED_BYTE;				// 4-component PVRTC,  8x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:			return GlFormat::UNSIGNED_BYTE;				// 4-component PVRTC,  8x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:			return GlFormat::UNSIGNED_BYTE;				// 4-component PVRTC,  4x4 blocks, sRGB

		//
		// ASTC
		//
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x4_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x5_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_8x5_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 8x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_8x6_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 8x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_8x8_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_10x5_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 10x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_10x6_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 10x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_10x8_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 10x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_10x10_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 10x10 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_12x10_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 12x10 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_12x12_KHR:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 12x12 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x6 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 8x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 8x6 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 8x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 10x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 10x6 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 10x8 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 10x10 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 12x10 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 12x12 blocks, sRGB

		case GlFormat::COMPRESSED_RGBA_ASTC_3x3x3_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 3x3x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_4x3x3_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 4x3x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4x3_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 4x4x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4x4_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 4x4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x4x4_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5x4_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x5x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5x5_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x5x5_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6x5_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x6x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6x6_OES:					return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x6x6 blocks, unsigned normalized

		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 3x3x3 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 4x3x3 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 4x4x3 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 4x4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x5x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 5x5x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x5x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x6x5 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:			return GlFormat::UNSIGNED_BYTE;				// 4-component ASTC, 6x6x6 blocks, sRGB

		//
		// ATC
		//
		case GlFormat::ATC_RGB_AMD:									return GlFormat::UNSIGNED_BYTE;				// 3-component, 4x4 blocks, unsigned normalized
		case GlFormat::ATC_RGBA_EXPLICIT_ALPHA_AMD:					return GlFormat::UNSIGNED_BYTE;				// 4-component, 4x4 blocks, unsigned normalized
		case GlFormat::ATC_RGBA_INTERPOLATED_ALPHA_AMD:				return GlFormat::UNSIGNED_BYTE;				// 4-component, 4x4 blocks, unsigned normalized

		//
		// Palletized
		//
		case GlFormat::PALETTE4_RGB8_OES:								return GlFormat::UNSIGNED_BYTE;				// 3-component 8:8:8,   4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_RGBA8_OES:								return GlFormat::UNSIGNED_BYTE;				// 4-component 8:8:8:8, 4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_R5_G6_B5_OES:							return GlFormat::UNSIGNED_SHORT_5_6_5;			// 3-component 5:6:5,   4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_RGBA4_OES:								return GlFormat::UNSIGNED_SHORT_4_4_4_4;		// 4-component 4:4:4:4, 4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_RGB5_A1_OES:							return GlFormat::UNSIGNED_SHORT_5_5_5_1;		// 4-component 5:5:5:1, 4-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGB8_OES:								return GlFormat::UNSIGNED_BYTE;				// 3-component 8:8:8,   8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGBA8_OES:								return GlFormat::UNSIGNED_BYTE;				// 4-component 8:8:8:8, 8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_R5_G6_B5_OES:							return GlFormat::UNSIGNED_SHORT_5_6_5;			// 3-component 5:6:5,   8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGBA4_OES:								return GlFormat::UNSIGNED_SHORT_4_4_4_4;		// 4-component 4:4:4:4, 8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGB5_A1_OES:							return GlFormat::UNSIGNED_SHORT_5_5_5_1;		// 4-component 5:5:5:1, 8-bit palette, unsigned normalized

		//
		// Depth/stencil
		//
		case GlFormat::DEPTH_COMPONENT16:								return GlFormat::UNSIGNED_SHORT;
		case GlFormat::DEPTH_COMPONENT24:								return GlFormat::UNSIGNED_INT_24_8;
		case GlFormat::DEPTH_COMPONENT32:								return GlFormat::UNSIGNED_INT;
		case GlFormat::DEPTH_COMPONENT32F:								return GlFormat::FLOAT;
		case GlFormat::DEPTH_COMPONENT32F_NV:							return GlFormat::FLOAT;
		case GlFormat::STENCIL_INDEX1:									return GlFormat::UNSIGNED_BYTE;
		case GlFormat::STENCIL_INDEX4:									return GlFormat::UNSIGNED_BYTE;
		case GlFormat::STENCIL_INDEX8:									return GlFormat::UNSIGNED_BYTE;
		case GlFormat::STENCIL_INDEX16:								return GlFormat::UNSIGNED_SHORT;
		case GlFormat::DEPTH24_STENCIL8:								return GlFormat::UNSIGNED_INT_24_8;
		case GlFormat::DEPTH32F_STENCIL8:								return GlFormat::FLOAT_32_UNSIGNED_INT_24_8_REV;
		case GlFormat::DEPTH32F_STENCIL8_NV:							return GlFormat::FLOAT_32_UNSIGNED_INT_24_8_REV;

		default:												return GlFormat::INVALID_VALUE;
	}
}

static inline unsigned int GetTypeSizeFromType(GlFormat type)
{
    switch (type) {
       case GlFormat::BYTE:
       case GlFormat::UNSIGNED_BYTE:
       case GlFormat::UNSIGNED_BYTE_3_3_2:
       case GlFormat::UNSIGNED_BYTE_2_3_3_REV:
            return 1;
            
       case GlFormat::SHORT:
       case GlFormat::UNSIGNED_SHORT:
       case GlFormat::UNSIGNED_SHORT_5_6_5:
       case GlFormat::UNSIGNED_SHORT_4_4_4_4:
       case GlFormat::UNSIGNED_SHORT_5_5_5_1:
       case GlFormat::UNSIGNED_SHORT_5_6_5_REV:
       case GlFormat::UNSIGNED_SHORT_4_4_4_4_REV:
       case GlFormat::UNSIGNED_SHORT_1_5_5_5_REV:
       case GlFormat::HALF_FLOAT:
            return 2;
            
       case GlFormat::INT:
       case GlFormat::UNSIGNED_INT:
       case GlFormat::UNSIGNED_INT_8_8_8_8:
       case GlFormat::UNSIGNED_INT_8_8_8_8_REV:
       case GlFormat::UNSIGNED_INT_10_10_10_2:
       case GlFormat::UNSIGNED_INT_2_10_10_10_REV:
       case GlFormat::UNSIGNED_INT_24_8:
       case GlFormat::UNSIGNED_INT_10F_11F_11F_REV:
       case GlFormat::UNSIGNED_INT_5_9_9_9_REV:
       case GlFormat::FLOAT:
       case GlFormat::FLOAT_32_UNSIGNED_INT_24_8_REV:
            return 4;

        default:
            return 0;
    }
}

typedef enum GlFormatSizeFlagBits {
	FORMAT_SIZE_PACKED_BIT				= 0x00000001,
	FORMAT_SIZE_COMPRESSED_BIT			= 0x00000002,
	FORMAT_SIZE_PALETTIZED_BIT			= 0x00000004,
	FORMAT_SIZE_DEPTH_BIT				= 0x00000008,
	FORMAT_SIZE_STENCIL_BIT				= 0x00000010,
} GlFormatSizeFlagBits;

typedef unsigned int GlFormatSizeFlags;

typedef struct GlFormatSize {
	GlFormatSizeFlags	flags;
	unsigned int		paletteSizeInBits;
	unsigned int		blockSizeInBits;
	unsigned int		blockWidth;			// in texels
	unsigned int		blockHeight;		// in texels
	unsigned int		blockDepth;			// in texels
} GlFormatSize;

static inline void GetFormatSize( const GlFormat internalFormat, GlFormatSize * pFormatSize )
{
	switch ( internalFormat )
	{
		//
		// 8 bits per component
		//
		case GlFormat::R8:												// 1-component, 8-bit unsigned normalized
		case GlFormat::R8_SNORM:										// 1-component, 8-bit signed normalized
		case GlFormat::R8UI:											// 1-component, 8-bit unsigned integer
		case GlFormat::R8I:											// 1-component, 8-bit signed integer
		case GlFormat::SR8:											// 1-component, 8-bit sRGB
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 1 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RG8:											// 2-component, 8-bit unsigned normalized
		case GlFormat::RG8_SNORM:										// 2-component, 8-bit signed normalized
		case GlFormat::RG8UI:											// 2-component, 8-bit unsigned integer
		case GlFormat::RG8I:											// 2-component, 8-bit signed integer
		case GlFormat::SRG8:											// 2-component, 8-bit sRGB
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB8:											// 3-component, 8-bit unsigned normalized
		case GlFormat::RGB8_SNORM:										// 3-component, 8-bit signed normalized
		case GlFormat::RGB8UI:											// 3-component, 8-bit unsigned integer
		case GlFormat::RGB8I:											// 3-component, 8-bit signed integer
		case GlFormat::SRGB8:											// 3-component, 8-bit sRGB
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 3 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGBA8:											// 4-component, 8-bit unsigned normalized
		case GlFormat::RGBA8_SNORM:									// 4-component, 8-bit signed normalized
		case GlFormat::RGBA8UI:										// 4-component, 8-bit unsigned integer
		case GlFormat::RGBA8I:											// 4-component, 8-bit signed integer
		case GlFormat::SRGB8_ALPHA8:									// 4-component, 8-bit sRGB
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;

		//
		// 16 bits per component
		//
		case GlFormat::R16:											// 1-component, 16-bit unsigned normalized
		case GlFormat::R16_SNORM:										// 1-component, 16-bit signed normalized
		case GlFormat::R16UI:											// 1-component, 16-bit unsigned integer
		case GlFormat::R16I:											// 1-component, 16-bit signed integer
		case GlFormat::R16F:											// 1-component, 16-bit floating-point
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 2 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RG16:											// 2-component, 16-bit unsigned normalized
		case GlFormat::RG16_SNORM:										// 2-component, 16-bit signed normalized
		case GlFormat::RG16UI:											// 2-component, 16-bit unsigned integer
		case GlFormat::RG16I:											// 2-component, 16-bit signed integer
		case GlFormat::RG16F:											// 2-component, 16-bit floating-point
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB16:											// 3-component, 16-bit unsigned normalized
		case GlFormat::RGB16_SNORM:									// 3-component, 16-bit signed normalized
		case GlFormat::RGB16UI:										// 3-component, 16-bit unsigned integer
		case GlFormat::RGB16I:											// 3-component, 16-bit signed integer
		case GlFormat::RGB16F:											// 3-component, 16-bit floating-point
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 6 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGBA16:											// 4-component, 16-bit unsigned normalized
		case GlFormat::RGBA16_SNORM:									// 4-component, 16-bit signed normalized
		case GlFormat::RGBA16UI:										// 4-component, 16-bit unsigned integer
		case GlFormat::RGBA16I:										// 4-component, 16-bit signed integer
		case GlFormat::RGBA16F:										// 4-component, 16-bit floating-point
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;

		//
		// 32 bits per component
		//
		case GlFormat::R32UI:											// 1-component, 32-bit unsigned integer
		case GlFormat::R32I:											// 1-component, 32-bit signed integer
		case GlFormat::R32F:											// 1-component, 32-bit floating-point
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RG32UI:											// 2-component, 32-bit unsigned integer
		case GlFormat::RG32I:											// 2-component, 32-bit signed integer
		case GlFormat::RG32F:											// 2-component, 32-bit floating-point
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB32UI:										// 3-component, 32-bit unsigned integer
		case GlFormat::RGB32I:											// 3-component, 32-bit signed integer
		case GlFormat::RGB32F:											// 3-component, 32-bit floating-point
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 12 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGBA32UI:										// 4-component, 32-bit unsigned integer
		case GlFormat::RGBA32I:										// 4-component, 32-bit signed integer
		case GlFormat::RGBA32F:										// 4-component, 32-bit floating-point
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16 * 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;

		//
		// Packed
		//
		case GlFormat::R3_G3_B2:										// 3-component 3:3:2, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB4:											// 3-component 4:4:4, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 12;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB5:											// 3-component 5:5:5, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB565:											// 3-component 5:6:5, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB10:											// 3-component 10:10:10, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB12:											// 3-component 12:12:12, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 36;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGBA2:											// 4-component 2:2:2:2, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGBA4:											// 4-component 4:4:4:4, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGBA12:											// 4-component 12:12:12:12, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 48;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB5_A1:										// 4-component 5:5:5:1, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB10_A2:										// 4-component 10:10:10:2, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::RGB10_A2UI:										// 4-component 10:10:10:2, unsigned integer
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::R11F_G11F_B10F:									// 3-component 11:11:10, floating-point
		case GlFormat::RGB9_E5:										// 3-component/exp 9:9:9/5, floating-point
			pFormatSize->flags = FORMAT_SIZE_PACKED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;

		//
		// S3TC/DXT/BC
		//
		case GlFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:					// line through 3D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:					// line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:					// line through 3D space, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:			// line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:					// line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:					// line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:			// line through 3D space plus line through 1D space, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:			// line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;

		case GlFormat::COMPRESSED_LUMINANCE_LATC1_EXT:					// line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:			// line through 1D space, 4x4 blocks, signed normalized
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:			// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:	// two lines through 1D space, 4x4 blocks, signed normalized
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;

		case GlFormat::COMPRESSED_RED_RGTC1:							// line through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_RED_RGTC1:					// line through 1D space, 4x4 blocks, signed normalized
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RG_RGTC2:							// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_RG_RGTC2:						// two lines through 1D space, 4x4 blocks, signed normalized
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;

		case GlFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:				// 3-component, 4x4 blocks, unsigned floating-point
		case GlFormat::COMPRESSED_RGB_BPTC_SIGNED_FLOAT:				// 3-component, 4x4 blocks, signed floating-point
		case GlFormat::COMPRESSED_RGBA_BPTC_UNORM:						// 4-component, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM:				// 4-component, 4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;

		//
		// ETC
		//
		case GlFormat::ETC1_RGB8_OES:									// 3-component ETC1, 4x4 blocks, unsigned normalized" ),
		case GlFormat::COMPRESSED_RGB8_ETC2:							// 3-component ETC2, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ETC2:							// 3-component ETC2, 4x4 blocks, sRGB
		case GlFormat::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:		// 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:		// 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA8_ETC2_EAC:						// 4-component ETC2, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:				// 4-component ETC2, 4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;

		case GlFormat::COMPRESSED_R11_EAC:								// 1-component ETC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_R11_EAC:						// 1-component ETC, 4x4 blocks, signed normalized
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RG11_EAC:							// 2-component ETC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SIGNED_RG11_EAC:						// 2-component ETC, 4x4 blocks, signed normalized
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;

		//
		// PVRTC
		//
		case GlFormat::COMPRESSED_RGB_PVRTC_2BPPV1_IMG:				// 3-component PVRTC, 16x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:				// 3-component PVRTC, 16x8 blocks, sRGB
		case GlFormat::COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:				// 4-component PVRTC, 16x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:			// 4-component PVRTC, 16x8 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 16;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGB_PVRTC_4BPPV1_IMG:				// 3-component PVRTC, 8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:				// 3-component PVRTC, 8x8 blocks, sRGB
		case GlFormat::COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:				// 4-component PVRTC, 8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:			// 4-component PVRTC, 8x8 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:				// 4-component PVRTC, 8x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:			// 4-component PVRTC, 8x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:				// 4-component PVRTC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:			// 4-component PVRTC, 4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;

		//
		// ASTC
		//
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4_KHR:					// 4-component ASTC, 4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:			// 4-component ASTC, 4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_5x4_KHR:					// 4-component ASTC, 5x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:			// 4-component ASTC, 5x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 5;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5_KHR:					// 4-component ASTC, 5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:			// 4-component ASTC, 5x5 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 5;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_6x5_KHR:					// 4-component ASTC, 6x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:			// 4-component ASTC, 6x5 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 6;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6_KHR:					// 4-component ASTC, 6x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:			// 4-component ASTC, 6x6 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 6;
			pFormatSize->blockHeight = 6;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_8x5_KHR:					// 4-component ASTC, 8x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:			// 4-component ASTC, 8x5 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_8x6_KHR:					// 4-component ASTC, 8x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:			// 4-component ASTC, 8x6 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 6;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_8x8_KHR:					// 4-component ASTC, 8x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:			// 4-component ASTC, 8x8 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 8;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_10x5_KHR:					// 4-component ASTC, 10x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:			// 4-component ASTC, 10x5 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 10;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_10x6_KHR:					// 4-component ASTC, 10x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:			// 4-component ASTC, 10x6 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 10;
			pFormatSize->blockHeight = 6;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_10x8_KHR:					// 4-component ASTC, 10x8 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:			// 4-component ASTC, 10x8 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 10;
			pFormatSize->blockHeight = 8;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_10x10_KHR:					// 4-component ASTC, 10x10 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:			// 4-component ASTC, 10x10 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 10;
			pFormatSize->blockHeight = 10;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_12x10_KHR:					// 4-component ASTC, 12x10 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:			// 4-component ASTC, 12x10 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 12;
			pFormatSize->blockHeight = 10;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_12x12_KHR:					// 4-component ASTC, 12x12 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:			// 4-component ASTC, 12x12 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 12;
			pFormatSize->blockHeight = 12;
			pFormatSize->blockDepth = 1;
			break;

		case GlFormat::COMPRESSED_RGBA_ASTC_3x3x3_OES:					// 4-component ASTC, 3x3x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:			// 4-component ASTC, 3x3x3 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 3;
			pFormatSize->blockHeight = 3;
			pFormatSize->blockDepth = 3;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_4x3x3_OES:					// 4-component ASTC, 4x3x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:			// 4-component ASTC, 4x3x3 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 3;
			pFormatSize->blockDepth = 3;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4x3_OES:					// 4-component ASTC, 4x4x3 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:			// 4-component ASTC, 4x4x3 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 3;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_4x4x4_OES:					// 4-component ASTC, 4x4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:			// 4-component ASTC, 4x4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 4;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_5x4x4_OES:					// 4-component ASTC, 5x4x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:			// 4-component ASTC, 5x4x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 5;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 4;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5x4_OES:					// 4-component ASTC, 5x5x4 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:			// 4-component ASTC, 5x5x4 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 5;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 4;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_5x5x5_OES:					// 4-component ASTC, 5x5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:			// 4-component ASTC, 5x5x5 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 5;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 5;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_6x5x5_OES:					// 4-component ASTC, 6x5x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:			// 4-component ASTC, 6x5x5 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 6;
			pFormatSize->blockHeight = 5;
			pFormatSize->blockDepth = 5;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6x5_OES:					// 4-component ASTC, 6x6x5 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:			// 4-component ASTC, 6x6x5 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 6;
			pFormatSize->blockHeight = 6;
			pFormatSize->blockDepth = 5;
			break;
		case GlFormat::COMPRESSED_RGBA_ASTC_6x6x6_OES:					// 4-component ASTC, 6x6x6 blocks, unsigned normalized
		case GlFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:			// 4-component ASTC, 6x6x6 blocks, sRGB
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 6;
			pFormatSize->blockHeight = 6;
			pFormatSize->blockDepth = 6;
			break;

		//
		// ATC
		//
		case GlFormat::ATC_RGB_AMD:									// 3-component, 4x4 blocks, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::ATC_RGBA_EXPLICIT_ALPHA_AMD:					// 4-component, 4x4 blocks, unsigned normalized
		case GlFormat::ATC_RGBA_INTERPOLATED_ALPHA_AMD:				// 4-component, 4x4 blocks, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_COMPRESSED_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 128;
			pFormatSize->blockWidth = 4;
			pFormatSize->blockHeight = 4;
			pFormatSize->blockDepth = 1;
			break;

		//
		// Palletized
		//
		case GlFormat::PALETTE4_RGB8_OES:								// 3-component 8:8:8,   4-bit palette, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PALETTIZED_BIT;
			pFormatSize->paletteSizeInBits = 16 * 24;
			pFormatSize->blockSizeInBits = 4;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::PALETTE4_RGBA8_OES:								// 4-component 8:8:8:8, 4-bit palette, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PALETTIZED_BIT;
			pFormatSize->paletteSizeInBits = 16 * 32;
			pFormatSize->blockSizeInBits = 4;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::PALETTE4_R5_G6_B5_OES:							// 3-component 5:6:5,   4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_RGBA4_OES:								// 4-component 4:4:4:4, 4-bit palette, unsigned normalized
		case GlFormat::PALETTE4_RGB5_A1_OES:							// 4-component 5:5:5:1, 4-bit palette, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PALETTIZED_BIT;
			pFormatSize->paletteSizeInBits = 16 * 16;
			pFormatSize->blockSizeInBits = 4;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::PALETTE8_RGB8_OES:								// 3-component 8:8:8,   8-bit palette, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PALETTIZED_BIT;
			pFormatSize->paletteSizeInBits = 256 * 24;
			pFormatSize->blockSizeInBits = 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::PALETTE8_RGBA8_OES:								// 4-component 8:8:8:8, 8-bit palette, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PALETTIZED_BIT;
			pFormatSize->paletteSizeInBits = 256 * 32;
			pFormatSize->blockSizeInBits = 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::PALETTE8_R5_G6_B5_OES:							// 3-component 5:6:5,   8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGBA4_OES:								// 4-component 4:4:4:4, 8-bit palette, unsigned normalized
		case GlFormat::PALETTE8_RGB5_A1_OES:							// 4-component 5:5:5:1, 8-bit palette, unsigned normalized
			pFormatSize->flags = FORMAT_SIZE_PALETTIZED_BIT;
			pFormatSize->paletteSizeInBits = 256 * 16;
			pFormatSize->blockSizeInBits = 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;

		//
		// Depth/stencil
		//
		case GlFormat::DEPTH_COMPONENT16:
			pFormatSize->flags = FORMAT_SIZE_DEPTH_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::DEPTH_COMPONENT24:
		case GlFormat::DEPTH_COMPONENT32:
		case GlFormat::DEPTH_COMPONENT32F:
		case GlFormat::DEPTH_COMPONENT32F_NV:
			pFormatSize->flags = FORMAT_SIZE_DEPTH_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::STENCIL_INDEX1:
			pFormatSize->flags = FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 1;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::STENCIL_INDEX4:
			pFormatSize->flags = FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 4;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::STENCIL_INDEX8:
			pFormatSize->flags = FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::STENCIL_INDEX16:
			pFormatSize->flags = FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 16;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::DEPTH24_STENCIL8:
			pFormatSize->flags = FORMAT_SIZE_DEPTH_BIT | FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 32;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
		case GlFormat::DEPTH32F_STENCIL8:
		case GlFormat::DEPTH32F_STENCIL8_NV:
			pFormatSize->flags = FORMAT_SIZE_DEPTH_BIT | FORMAT_SIZE_STENCIL_BIT;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 64;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;

		default:
			pFormatSize->flags = 0;
			pFormatSize->paletteSizeInBits = 0;
			pFormatSize->blockSizeInBits = 8;
			pFormatSize->blockWidth = 1;
			pFormatSize->blockHeight = 1;
			pFormatSize->blockDepth = 1;
			break;
	}
}

} //namespace gl
} //namespace weave