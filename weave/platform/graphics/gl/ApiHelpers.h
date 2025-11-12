/*
OpenGL 4.5 GFX API helpers
Convinience OpenGL implementations of generic GFX API functions
*/

#pragma once
#include <string>
#include <string_view>
#include <functional>
#include "gl46.h"
#include "ContextPool.h"

namespace weave::opengl {
	//Returns the current OpenGL version as a single uint32
	uint32_t GetGLVersion();

	//Returns the corresponding GL enum to a given string representation. The string should not include the GL_ prefix.
	GLenum EnumStringToGL(std::string str);
	//Returns a string representing the GL enum provided
	std::string EnumGLToString(GLenum gl);

	//Enum indicating the type of blending to use with the material
	enum class Blending {
		None,
		Translucent,
		Additive,
		Custom
	};
	//Enum indicating the type of face culling to apply when using he material
	enum class FaceCulling {
		Default,
		ShowFront,
		ShowBack,
		ShowBoth,
		ShowNone
	};

	//Support function that returns a Blending enum based on the name
	Blending GetBlendingFromName(std::string name);
	//Support function that returns a FaceCulling enum based on the name
	FaceCulling GetFaceCullingFromName(std::string name);

	//Sets the current blending mode
	void SetBlending(Blending blending);
	void SetBlending(uint32_t buffer, Blending blending);
	//Set the face culling mode
	void SetFaceCulling(FaceCulling culling);

	//Sets the primitive restart index for triangle strips
	void SetPrimitiveRestart(uint32_t index, bool flag);

	//Turns face filling on or off (default on, triangles are filled)
	void SetFaceFill(bool flag);

	//Turns standard VSync on or off
	void SetVSync(bool flag);

	//Calls the Present functionality of the underlying API
	void PresentScreen();

	//Returns true if the given gl texture target is layered 
	bool IsLayeredTarget(GLenum target);

	//Helper function that returns true if TexStorage extension is available
	bool HasTexStorageExtension();
	//Helper function that returns true if TextureStorage extension is available
	bool HasTextureStorageExtension();

	//Returns a string associated to a glCheckFramebufferStatus enum
	std::string GetFrameBufferStatusString(GLenum statusValue);

	//Returns a string from a glGetError enum
	//Calls gluErrorString internally if available
	std::string_view GetErrorString(GLenum error);

	//Binds a callback for GL debugging
	void BindDebugCallback(GLDEBUGPROC callback, void *user_info);
	void BindDebugCallback(std::function<void(std::string message)> callback);

	void EnableDebugMessage(bool enable, bool sync = true);
}