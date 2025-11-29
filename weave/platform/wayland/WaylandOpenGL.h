#pragma once

#include "WaylandWindow.h"

#include "weave/graphics/gl/context/ContextPool.h"
#include "weave/graphics/gl/gl46.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-egl.h>

#include <string>
#include <utility>
#include <vector>

namespace weave::wayland {

class WaylandOpenGL {
public:
	struct InitParams {
		int majorVersion = 0;
		int minorVersion = 0;
		uint32_t numGLContexts = 2;
		uint32_t colorBits = 32;
		uint32_t depthBits = 24;
		uint32_t stencilBits = 8;
		uint32_t msaaSamples = 0;
		bool debugContext = false;
		bool sRGB = true;
		int surfaceWidth = 0;
		int surfaceHeight = 0;
	};

	WaylandOpenGL() = default;
	~WaylandOpenGL();

	WaylandOpenGL(WaylandOpenGL const&) = delete;
	WaylandOpenGL& operator=(WaylandOpenGL const&) = delete;
	WaylandOpenGL(WaylandOpenGL&&) = delete;
	WaylandOpenGL& operator=(WaylandOpenGL&&) = delete;

	bool InitGL(WaylandWindow& window);
	bool InitGL(WaylandWindow& window, InitParams params);
	void DestroyContexts();

	std::pair<int, int> GetGLVersion() const;

	EGLDisplay GetEGLDisplay() const { return eglDisplay; }
	EGLSurface GetEGLSurface() const { return eglSurface; }
	EGLContext GetEGLContext(std::size_t idx = 0) const { return eglContexts[idx]; }
	std::size_t GetContextCount() const { return eglContexts.size(); }

	void SwapBuffers();
	void Resize(int width, int height);

private:
	bool BindAPI();
	bool ChooseConfig(const InitParams& params, EGLConfig& config);
	bool CreateEGLContext(const InitParams& params, EGLConfig config);

	WaylandWindow* owner = nullptr;
	wl_egl_window* eglWindow = nullptr;

	EGLDisplay eglDisplay = EGL_NO_DISPLAY;
	EGLSurface eglSurface = EGL_NO_SURFACE;
	std::vector<EGLContext> eglContexts;

	bool usesGLES = false;
};

} // namespace weave::wayland
