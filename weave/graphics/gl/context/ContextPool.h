/*
Weave GL Context Pool
Helper global allocator for multi threaded OpenGL resource context claiming
*/
#pragma once

#include "weave/graphics/gl/gl46.h"
#include <vector>
#include <thread>
#include <mutex>

#if defined(NN_NINTENDO_SDK) || defined(WEAVE_PLATFORM_WAYLAND)
#include <EGL/egl.h>
#endif

namespace weave::opengl {

	class Context;

class ContextPool {
	friend class Context;
private:
	struct ContextData {
		std::thread::id threadId; //Thread identifier associated with this context. {} if free.

#ifdef _WIN32
		HDC hdc = nullptr; //Win32 window handle
		HGLRC glrc = nullptr; //Win32 GL handle

		ContextData(HDC hdc, HGLRC glrc) : hdc(hdc), glrc(glrc){}
#elif defined(NN_NINTENDO_SDK) || defined(WEAVE_PLATFORM_WAYLAND)
		EGLDisplay hDisplay; //NX display handle
		EGLSurface hSurface; //NX GL surface handle
		EGLContext hContext; //NX GL Context Handle

		ContextData(EGLDisplay display, EGLSurface surface, EGLContext context) : hDisplay(display), hSurface(surface), hContext(context) {}
#endif
		ContextData() = default;
		void Activate();
		void Release();
	};

	//List of available contexts
	static std::vector<ContextData> contexts;
	static std::mutex contextMutex;

public:
#ifdef _WIN32
	static void RegisterContext(HDC hdc, HGLRC glrc);
	static void RegisterContexts(HDC hdc, std::vector<HGLRC> const &glrc);
	
	static HDC GetWindowsHDC();
#elif defined(NN_NINTENDO_SDK) || defined(WEAVE_PLATFORM_WAYLAND)
	static void RegisterContext(EGLDisplay display, EGLSurface surface, EGLContext context);
	static void RegisterContexts(EGLDisplay display, EGLSurface surface, std::vector<EGLContext> const &context);

	static EGLDisplay GetDisplayHandle();
	static EGLSurface GetSurfaceHandle();
#endif
	
	static uint32_t FreeContextsCount();

	//Thread specific context claiming, releasing and accessing
	static void ClaimContext();
	static void ReleaseContext();

	//Returns true if the caller thread has a valid context assigned
	static bool HasClaimedContext();
};

class Context {

public:
	//Constructor automatically claims a context for the constructing thread
	Context();
	Context(Context const& other) = delete;
	Context(Context &&other) = delete;
	Context& operator=(Context const& other) = delete;
	Context& operator=(Context&& other) = delete;
	//Destructor automatically releases a context claimed
	~Context();

	bool IsValid();
	void Claim();
	void Release();

};

}
