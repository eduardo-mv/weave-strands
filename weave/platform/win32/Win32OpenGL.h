/*
Win32 OpenGL Context Creator

Win32 window creation helper with OpenGL 3.0+ support with multiple shared contexts
*/


#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

#include "weave/platform/graphics/gl/gl46.h"
#include "weave/platform/graphics/gl/ContextPool.h"

namespace weave {
namespace win32 {

class Win32OpenGL {
public:
	struct InitParams {
		int majorVersion = 0;
		int minorVersion = 0;
		uint32_t numGLContexts = 4;
		uint32_t numColorBuffers = 2;
		uint32_t colorBits = 32;
		uint32_t depthBits = 24;
		uint32_t stencilBits = 8;
		uint32_t msaaSamples = 0;
		bool sRGB = true;
		bool debugContext = false;
	};

protected:
	HWND hwnd{}; //Related hwnd
	HDC hdc{}; //Device context taken from the HWND
	std::vector<HGLRC> glrc; //OpenGL contexts

public:
	Win32OpenGL() = default;
	~Win32OpenGL();

	//Initialize OpenGL for the provided hwnd
	bool InitGL(HWND hwnd);
	bool InitGL(HWND hwnd, InitParams initParams);
	
	void DestroyContexts(); 

	std::vector<std::string> EnumPixelFormats() const;
	
	std::pair<int, int> GetGLVersion() const;

	//Returns the handle to the device context HDC
	HDC GetHDC() const { return hdc; }
	
	//Returns one of the OpenGL shared resource contexts
	HGLRC GetHGLRC(unsigned int i = 0) const { return glrc[i]; }
	std::vector<HGLRC> GetHGLRCs() const { return glrc; }
	size_t GetContextCount() const { return glrc.size(); }
};

}
}
