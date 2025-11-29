#include "ApiHelpers.h"
#include <unordered_map>
#include <algorithm>
#include <sstream>


#if __has_include(<gl/GLU.h>)
#include <gl/GLU.h>
#endif

using namespace weave;
using namespace opengl;

//Returns the current OpenGL version as a single uint32
uint32_t weave::opengl::GetGLVersion() {
	uint32_t version = GLVersion.major * 10 + GLVersion.minor;
	return version;
}

//Utility functions for all texture derivatives
//Returns the GL compression flag ID based on simple string codes determined by the texture file format
#define glEnumMacro(glenum) {#glenum, gl::glenum}
GLenum weave::opengl::EnumStringToGL(std::string str) {
	//Use a static map initialized first time the function runs
	static std::unordered_map<std::string, unsigned int> glmap = {
		//Manual equivalences created as shortcuts
		{"AUTO", gl::COMPRESSED_RGBA_S3TC_DXT5_EXT},
		{"RGBA", gl::RGBA8},
		{"RGB", gl::RGB8},
		{"RED", gl::R8},
		{"BC1", gl::COMPRESSED_RGB_S3TC_DXT1_EXT},
		{"BC2", gl::COMPRESSED_RGBA_S3TC_DXT5_EXT},
		{"DXT1", gl::COMPRESSED_RGB_S3TC_DXT1_EXT},
		{"DXT5", gl::COMPRESSED_RGBA_S3TC_DXT5_EXT},
		{"SRGBA", gl::SRGB8_ALPHA8},
		{"SRGB", gl::SRGB8},
		{"SBC1", gl::COMPRESSED_SRGB_S3TC_DXT1_EXT},
		{"SBC2", gl::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT},
		{"SDXT1", gl::COMPRESSED_SRGB_S3TC_DXT1_EXT},
		{"SDXT5", gl::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT},
		{"DEPTH", gl::DEPTH_COMPONENT},
		{"DEPTH16", gl::DEPTH_COMPONENT16},
		{"DEPTH24", gl::DEPTH_COMPONENT24},
		{"DEPTH32", gl::DEPTH_COMPONENT32},
		{"DEPTH32F", gl::DEPTH_COMPONENT32F},
		{"EDGE", gl::CLAMP_TO_EDGE},
		{"BORDER", gl::CLAMP_TO_BORDER},
		{"MIRROR", gl::MIRRORED_REPEAT},
		{"MIRRORED", gl::MIRRORED_REPEAT},
		{"EDGEMIRROR", gl::MIRROR_CLAMP_TO_EDGE},
		{"EDGEMIRRORED", gl::MIRROR_CLAMP_TO_EDGE},
		{"REPEAT", gl::REPEAT},

		//OpenGL enum literals
		glEnumMacro(COMPRESSED_RGB_S3TC_DXT1_EXT),
		glEnumMacro(COMPRESSED_RGBA_S3TC_DXT5_EXT),
		glEnumMacro(SRGB8_ALPHA8),
		glEnumMacro(SRGB8),
		glEnumMacro(COMPRESSED_SRGB_S3TC_DXT1_EXT),
		glEnumMacro(COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT),
		glEnumMacro(COMPRESSED_SRGB_S3TC_DXT1_EXT),
		glEnumMacro(COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT),

		glEnumMacro(DEPTH_COMPONENT),
		glEnumMacro(DEPTH_COMPONENT16),
		glEnumMacro(DEPTH_COMPONENT24),
		glEnumMacro(DEPTH_COMPONENT32),
		glEnumMacro(DEPTH_COMPONENT32F),
		glEnumMacro(DEPTH_STENCIL),

		glEnumMacro(RG),
		glEnumMacro(R8),
		glEnumMacro(R8_SNORM),
		glEnumMacro(R16),
		glEnumMacro(R16_SNORM),
		glEnumMacro(RG8),
		glEnumMacro(RG16),
		glEnumMacro(RG16_SNORM),
		glEnumMacro(R3_G3_B2),
		glEnumMacro(RGB4),
		glEnumMacro(RGB5),
		glEnumMacro(RGB8),
		glEnumMacro(RGB8_SNORM),
		glEnumMacro(RGB10),
		glEnumMacro(RGB12),
		glEnumMacro(RGB16_SNORM),
		glEnumMacro(RGBA2),
		glEnumMacro(RGBA4),
		glEnumMacro(RGB5_A1),
		glEnumMacro(RGBA8),
		glEnumMacro(RGBA8_SNORM),
		glEnumMacro(RGB10_A2),
		glEnumMacro(RGB10_A2UI),
		glEnumMacro(RGBA12),
		glEnumMacro(RGBA16),
		glEnumMacro(SRGB8),
		glEnumMacro(SRGB8_ALPHA8),
		glEnumMacro(R16F),
		glEnumMacro(RG16F),
		glEnumMacro(RGB16F),
		glEnumMacro(RGBA16F),
		glEnumMacro(R32F),
		glEnumMacro(RG32F),
		glEnumMacro(RGB32F),
		glEnumMacro(RGBA32F),
		glEnumMacro(R11F_G11F_B10F),
		glEnumMacro(RGB9_E5),
		glEnumMacro(R8I),
		glEnumMacro(R8UI),
		glEnumMacro(R16I),
		glEnumMacro(R16UI),
		glEnumMacro(R32I),
		glEnumMacro(R32UI),
		glEnumMacro(RG8I),
		glEnumMacro(RG8UI),
		glEnumMacro(RG16I),
		glEnumMacro(RG16UI),
		glEnumMacro(RG32I),
		glEnumMacro(RG32UI),
		glEnumMacro(RGB8I),
		glEnumMacro(RGB8UI),
		glEnumMacro(RGB16I),
		glEnumMacro(RGB16UI),
		glEnumMacro(RGB32I),
		glEnumMacro(RGB32UI),
		glEnumMacro(RGBA8I),
		glEnumMacro(RGBA8UI),
		glEnumMacro(RGBA16I),
		glEnumMacro(RGBA16UI),
		glEnumMacro(RGBA32I),
		glEnumMacro(RGBA32UI),

		glEnumMacro(NONE),
		glEnumMacro(LEQUAL),
		glEnumMacro(GEQUAL),
		glEnumMacro(LESS),
		glEnumMacro(GREATER),
		glEnumMacro(EQUAL),
		glEnumMacro(NOTEQUAL),
		glEnumMacro(ALWAYS),
		glEnumMacro(NEVER),

		glEnumMacro(LINEAR),
		glEnumMacro(NEAREST),

		glEnumMacro(CLAMP_TO_EDGE),
		glEnumMacro(CLAMP_TO_BORDER),
		glEnumMacro(MIRRORED_REPEAT),
		glEnumMacro(MIRROR_CLAMP_TO_EDGE),
		glEnumMacro(REPEAT),
	};


	//Transform to uppercase
	std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)::toupper((int)c); });
	//Remove a possible GL_ line at the start of the string
	if (str.substr(0, 3) == "GL_")
		str = str.substr(3);
	//Return the correct value for the associated gl string
	return glmap[str];
}
#undef glEnumMacro

#define glEnumMacro(glenum) {gl::glenum, #glenum}
std::string weave::opengl::EnumGLToString(GLenum gl) {
	static std::unordered_map<unsigned int, std::string> glmap = {
		//OpenGL enum literals
		glEnumMacro(COMPRESSED_RGB_S3TC_DXT1_EXT),
		glEnumMacro(COMPRESSED_RGBA_S3TC_DXT5_EXT),
		glEnumMacro(SRGB8_ALPHA8),
		glEnumMacro(SRGB8),
		glEnumMacro(COMPRESSED_SRGB_S3TC_DXT1_EXT),
		glEnumMacro(COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT),
		glEnumMacro(COMPRESSED_SRGB_S3TC_DXT1_EXT),
		glEnumMacro(COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT),

		glEnumMacro(DEPTH_COMPONENT),
		glEnumMacro(DEPTH_COMPONENT16),
		glEnumMacro(DEPTH_COMPONENT24),
		glEnumMacro(DEPTH_COMPONENT32),
		glEnumMacro(DEPTH_COMPONENT32F),
		glEnumMacro(DEPTH_STENCIL),

		glEnumMacro(RG),
		glEnumMacro(R8),
		glEnumMacro(R8_SNORM),
		glEnumMacro(R16),
		glEnumMacro(R16_SNORM),
		glEnumMacro(RG8),
		glEnumMacro(RG16),
		glEnumMacro(RG16_SNORM),
		glEnumMacro(R3_G3_B2),
		glEnumMacro(RGB4),
		glEnumMacro(RGB5),
		glEnumMacro(RGB8),
		glEnumMacro(RGB8_SNORM),
		glEnumMacro(RGB10),
		glEnumMacro(RGB12),
		glEnumMacro(RGB16_SNORM),
		glEnumMacro(RGBA2),
		glEnumMacro(RGBA4),
		glEnumMacro(RGB5_A1),
		glEnumMacro(RGBA8),
		glEnumMacro(RGBA8_SNORM),
		glEnumMacro(RGB10_A2),
		glEnumMacro(RGB10_A2UI),
		glEnumMacro(RGBA12),
		glEnumMacro(RGBA16),
		glEnumMacro(SRGB8),
		glEnumMacro(SRGB8_ALPHA8),
		glEnumMacro(R16F),
		glEnumMacro(RG16F),
		glEnumMacro(RGB16F),
		glEnumMacro(RGBA16F),
		glEnumMacro(R32F),
		glEnumMacro(RG32F),
		glEnumMacro(RGB32F),
		glEnumMacro(RGBA32F),
		glEnumMacro(R11F_G11F_B10F),
		glEnumMacro(RGB9_E5),
		glEnumMacro(R8I),
		glEnumMacro(R8UI),
		glEnumMacro(R16I),
		glEnumMacro(R16UI),
		glEnumMacro(R32I),
		glEnumMacro(R32UI),
		glEnumMacro(RG8I),
		glEnumMacro(RG8UI),
		glEnumMacro(RG16I),
		glEnumMacro(RG16UI),
		glEnumMacro(RG32I),
		glEnumMacro(RG32UI),
		glEnumMacro(RGB8I),
		glEnumMacro(RGB8UI),
		glEnumMacro(RGB16I),
		glEnumMacro(RGB16UI),
		glEnumMacro(RGB32I),
		glEnumMacro(RGB32UI),
		glEnumMacro(RGBA8I),
		glEnumMacro(RGBA8UI),
		glEnumMacro(RGBA16I),
		glEnumMacro(RGBA16UI),
		glEnumMacro(RGBA32I),
		glEnumMacro(RGBA32UI),

		glEnumMacro(NONE),
		glEnumMacro(LEQUAL),
		glEnumMacro(GEQUAL),
		glEnumMacro(LESS),
		glEnumMacro(GREATER),
		glEnumMacro(EQUAL),
		glEnumMacro(NOTEQUAL),
		glEnumMacro(ALWAYS),
		glEnumMacro(NEVER),
	};

	return glmap[gl];
}

//Support function that returns a Blending enum based on the name
Blending weave::opengl::GetBlendingFromName(std::string str) {
	static std::unordered_map<std::string, Blending> typemap = {
		//Manual equivalences created as shortcuts
		{"none", Blending::None},
		{"transparent", Blending::Translucent},
		{"translucent", Blending::Translucent},
		{"standard", Blending::Translucent},
		{"additive", Blending::Additive},
		{"custom", Blending::Custom},
	};

	//Transform to uppercase
	std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)::tolower((int)c); });
	//Return the correct value for the associated string
	return typemap[str];
}

//Support function that returns a FaceCulling enum based on the name
FaceCulling weave::opengl::GetFaceCullingFromName(std::string str) {
	static std::unordered_map<std::string, FaceCulling> typemap = {
		//Manual equivalences created as shortcuts
		{"none", FaceCulling::Default},
		{"default", FaceCulling::Default},
		{"front", FaceCulling::ShowFront},
		{"showfront", FaceCulling::ShowFront},
		{"back", FaceCulling::ShowBack},
		{"showback", FaceCulling::ShowBack},
		{"both", FaceCulling::ShowBoth},
		{"showboth", FaceCulling::ShowBoth},
		{"none", FaceCulling::ShowNone},
		{"shownone", FaceCulling::ShowNone},
	};

	//Transform to uppercase
	std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)::tolower((int)c); });
	//Return the correct value for the associated string
	return typemap[str];
}

//Sets the current blending mode
void weave::opengl::SetBlending(Blending blending) {
	static Blending lastBlending = Blending::None;
	if(lastBlending == blending)
		return;
	lastBlending = blending;

	switch(blending) {
		case Blending::None:
			break;
		case Blending::Translucent:
			gl::BlendEquationi(0, gl::FUNC_ADD);
			gl::BlendEquationi(1, gl::FUNC_ADD);
			gl::BlendEquationi(2, gl::FUNC_ADD);
			gl::BlendFunci(0, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
			gl::BlendFunci(1, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
			gl::BlendFunci(2, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
			//gl::BlendFuncSeparatei(0, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA, gl::ONE, gl::ONE);
			//gl::BlendFuncSeparatei(1, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA, gl::ONE, gl::ONE);
			//gl::BlendFuncSeparatei(2, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA, gl::ONE, gl::ONE);
			break;
		case Blending::Additive:
			gl::BlendEquationi(0, gl::FUNC_ADD);
			gl::BlendEquationi(1, gl::FUNC_ADD);
			gl::BlendEquationi(2, gl::FUNC_ADD);
			gl::BlendFunci(0, gl::SRC_ALPHA, gl::ONE);
			gl::BlendFunci(1, gl::SRC_ALPHA, gl::ONE);
			gl::BlendFunci(2, gl::SRC_ALPHA, gl::ONE);
			//gl::BlendFuncSeparatei(0, gl::SRC_ALPHA, gl::ONE, gl::ONE, gl::ONE);
			//gl::BlendFuncSeparatei(1, gl::SRC_ALPHA, gl::ONE, gl::ONE, gl::ONE);
			//gl::BlendFuncSeparatei(2, gl::SRC_ALPHA, gl::ONE, gl::ONE, gl::ONE);
			break;
		case Blending::Custom: //TODO Custom Blending
		default:
			break;
	}
}

void weave::opengl::SetBlending(uint32_t buffer, Blending blending)
{
	switch (blending) {
	case Blending::None:
		break;
	case Blending::Translucent:
		gl::BlendEquationi(buffer, gl::FUNC_ADD);
		gl::BlendFunci(buffer, gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
		break;
	case Blending::Additive:
		gl::BlendEquationi(buffer, gl::FUNC_ADD);
		gl::BlendFunci(buffer, gl::SRC_ALPHA, gl::ONE);
		break;
	case Blending::Custom: //TODO Custom Blending
	default:
		break;
	}
}

//Set the face culling mode
void weave::opengl::SetFaceCulling(FaceCulling culling) {
	static FaceCulling lastFC = FaceCulling::Default;
	if(culling == lastFC)
		return;
	lastFC = culling;

	switch(culling) {
		case FaceCulling::Default: //No change
			break;
		case FaceCulling::ShowFront: {
			gl::Enable(gl::CULL_FACE);
			gl::CullFace(gl::BACK);
			break;
		}
		case FaceCulling::ShowBack: {
			gl::Enable(gl::CULL_FACE);
			gl::CullFace(gl::FRONT);
			break;
		}

		case FaceCulling::ShowBoth: {
			gl::Disable(gl::CULL_FACE);
			break;
		}
		case FaceCulling::ShowNone: {
			gl::Enable(gl::CULL_FACE);
			gl::CullFace(gl::FRONT_AND_BACK);
			break;
		}
		default:
			break;
	}
}

//Sets the primitive restart index for triangle strips
void weave::opengl::SetPrimitiveRestart(uint32_t index, bool flag) {
	static uint32_t oldIndex = 0;
	static bool oldFlag = false;

	if(oldIndex != index || oldFlag != flag) {
		gl::Enable(gl::PRIMITIVE_RESTART);
		gl::PrimitiveRestartIndex(index);
		oldIndex = index;
		oldFlag = flag;
	}
}

//Turns face filling on or off (default on, triangles are filled)
void weave::opengl::SetFaceFill(bool flag) {
	gl::PolygonMode(gl::FRONT_AND_BACK, flag ? gl::FILL : gl::LINE);
}

//Turns standard VSync on or off
void weave::opengl::SetVSync(bool flag) {
#ifdef WIN32
	wglSwapIntervalEXT(flag ? -1 : 0);
#elif NN_NINTENDO_SDK
	eglSwapInterval(GLContextPool::GetDisplayHandle(), flag ? -1 : 0);
#else
	(void)flag;
#endif
}

//Calls the Present functionality of the underlying API
void weave::opengl::PresentScreen() {
#ifdef WIN32
	SwapBuffers(opengl::ContextPool::GetWindowsHDC());
#elif NN_NINTENDO_SDK
	eglSwapBuffers(opengl::ContextPool::GetDisplayHandle(), opengl::ContextPool::GetSurfaceHandle());
#endif
}

bool weave::opengl::IsLayeredTarget(GLenum target) {
	return (target == gl::TEXTURE_1D_ARRAY || target == gl::TEXTURE_2D_ARRAY || target == gl::TEXTURE_3D ||
		target == gl::TEXTURE_2D_MULTISAMPLE_ARRAY || target == gl::TEXTURE_CUBE_MAP_ARRAY);
}


bool weave::opengl::HasTexStorageExtension() {
	bool hasTexStorage = weave::opengl::GetGLVersion() >= 42;
	return hasTexStorage;
}

bool weave::opengl::HasTextureStorageExtension() {
	bool hasTexStorage = weave::opengl::GetGLVersion() >= 45;
	return hasTexStorage;
}

std::string weave::opengl::GetFrameBufferStatusString(GLenum statusValue) {
	switch (statusValue) {
	case gl::FRAMEBUFFER_COMPLETE:
		return "FRAMEBUFFER_COMPLETE";
	case gl::FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		return "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
	case gl::FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		return "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
	case gl::FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		return "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
	default:
		return "UNKNOWN";
	}
}

std::string_view weave::opengl::GetErrorString(GLenum error) {
#if __has_include(<gl/GLU.h>)
	auto errorStr = gluErrorString(error);
	return std::string_view{ reinterpret_cast<char const*>(errorStr) };
#else
	(void)error;
	return "gluErrorString not available";
#endif
}

namespace {
	void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* userCallback) {
			std::stringstream out;

		out << "\nGL Debug ID: " << id << "\n";
		switch (source) {
		case gl::DEBUG_SOURCE_API:
			out << "Source: API";
			break;
		case gl::DEBUG_SOURCE_SHADER_COMPILER:
			out << "Source: Shader Compiler";
			break;
		case gl::DEBUG_SOURCE_WINDOW_SYSTEM:
			out << "Source: Windows";
			break;
		case gl::DEBUG_SOURCE_THIRD_PARTY:
			out << "Source: Third party";
			break;
		case gl::DEBUG_SOURCE_APPLICATION:
			out << "Source: Application";
			break;
		case gl::DEBUG_SOURCE_OTHER:
			out << "Source: Other";
			break;
		}

		out << "\n";

		switch (type) {
		case gl::DEBUG_TYPE_ERROR:
			out << "Type: Error";
			break;
		case gl::DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			out << "Type: Deprecation";
			break;
		case gl::DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			out << "Type: Undefined behavior";
			break;
		case gl::DEBUG_TYPE_PERFORMANCE:
			out << "Type: Performance";
			break;
		case gl::DEBUG_TYPE_PORTABILITY:
			out << "Type: Portability";
			break;
		case gl::DEBUG_TYPE_MARKER:
			out << "Type: Marker";
			break;
		case gl::DEBUG_TYPE_PUSH_GROUP:
			out << "Type: PushGroup";
			break;
		case gl::DEBUG_TYPE_POP_GROUP:
			out << "Type: PopGroup";
			break;
		case gl::DEBUG_TYPE_OTHER:
			out << "Type: Other";
			break;
		}

		out << "\n";

		switch (severity) {
		case gl::DEBUG_SEVERITY_NOTIFICATION:
			out << "Severity: Notification";
			break;
		case gl::DEBUG_SEVERITY_LOW:
			out << "Severity: Low";
			break;
		case gl::DEBUG_SEVERITY_MEDIUM:
			out << "Severity: Med";
			break;
		case gl::DEBUG_SEVERITY_HIGH:
			out << "Severity: High";
			break;
		}

		out << "\n" << message;

		auto cb = reinterpret_cast<std::function<void(std::string message)>const*>(userCallback);
		(*cb)(out.str());
	}
}

void weave::opengl::BindDebugCallback(GLDEBUGPROC callback, void* user_info)
{
	gl::DebugMessageCallback(callback, user_info);
}

void weave::opengl::BindDebugCallback(std::function<void(std::string message)> callback)
{
	static auto cb = callback;
	BindDebugCallback(GLDebugCallback, &cb);
}

void weave::opengl::EnableDebugMessage(bool enable, bool sync)
{
	enable ? gl::Enable(gl::DEBUG_OUTPUT) : gl::Disable(gl::DEBUG_OUTPUT);
	sync ? gl::Enable(gl::DEBUG_OUTPUT_SYNCHRONOUS) : gl::Disable(gl::DEBUG_OUTPUT_SYNCHRONOUS);
}

