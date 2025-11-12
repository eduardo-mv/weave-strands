#include "WaylandOpenGL.h"

#include <array>
#include <sstream>

namespace weave::wayland {

namespace {

EGLint ToColorComponentBits(uint32_t totalBits) {
	if (totalBits >= 32) {
		return 8;
	}
	if (totalBits >= 24) {
		return 8;
	}
	if (totalBits >= 16) {
		return 5;
	}
	return 8;
}

} // namespace

WaylandOpenGL::~WaylandOpenGL() {
	DestroyContexts();
}

bool WaylandOpenGL::InitGL(WaylandWindow& window) {
	return InitGL(window, InitParams{});
}

bool WaylandOpenGL::InitGL(WaylandWindow& window, InitParams params) {
	owner = &window;

	eglDisplay = eglGetDisplay(static_cast<EGLNativeDisplayType>(window.GetDisplay()));
	if (eglDisplay == EGL_NO_DISPLAY) {
		throw WaylandError("eglGetDisplay failed.");
	}

	if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
		throw WaylandError("eglInitialize failed.");
	}

	usesGLES = !BindAPI();

	EGLConfig config{};
	if (!ChooseConfig(params, config)) {
		throw WaylandError("No suitable EGL config found.");
	}

	int width = params.surfaceWidth;
	int height = params.surfaceHeight;
	if (width <= 0 || height <= 0) {
		auto [w, h] = window.GetSize();
		if (width <= 0) {
			width = w;
		}
		if (height <= 0) {
			height = h;
		}
	}

	eglWindow = window.CreateEGLWindow(width, height);

	eglSurface = eglCreateWindowSurface(eglDisplay, config, eglWindow, nullptr);
	if (eglSurface == EGL_NO_SURFACE) {
		throw WaylandError("eglCreateWindowSurface failed.");
	}

	if (!CreateEGLContext(params, config)) {
		throw WaylandError("Failed to create EGL context.");
	}

	// Make the first context current so that loaders can initialize
	if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContexts.front())) {
		throw WaylandError("eglMakeCurrent failed.");
	}

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(eglGetProcAddress))) {
		throw WaylandError("Failed to load OpenGL functions.");
	}

	opengl::ContextPool::RegisterContexts(eglDisplay, eglSurface, eglContexts);
	opengl::ContextPool::ClaimContext();

	auto [w, h] = window.GetSize();
	gl::Viewport(0, 0, w, h);

	return true;
}

void WaylandOpenGL::DestroyContexts() {
	if (eglDisplay == EGL_NO_DISPLAY) {
		return;
	}

	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	for (auto ctx : eglContexts) {
		if (ctx != EGL_NO_CONTEXT) {
			eglDestroyContext(eglDisplay, ctx);
		}
	}
	eglContexts.clear();

	if (eglSurface != EGL_NO_SURFACE) {
		eglDestroySurface(eglDisplay, eglSurface);
		eglSurface = EGL_NO_SURFACE;
	}

	if (eglWindow && owner) {
		owner->DestroyEGLWindow(eglWindow);
		eglWindow = nullptr;
	}

	eglTerminate(eglDisplay);
	eglDisplay = EGL_NO_DISPLAY;
	owner = nullptr;
}

std::pair<int, int> WaylandOpenGL::GetGLVersion() const {
	if (eglContexts.empty()) {
		return {0, 0};
	}

	int major = 0;
	int minor = 0;
	gl::GetIntegerv(gl::MAJOR_VERSION, &major);
	gl::GetIntegerv(gl::MINOR_VERSION, &minor);
	return {major, minor};
}

void WaylandOpenGL::SwapBuffers() {
	if (eglDisplay != EGL_NO_DISPLAY && eglSurface != EGL_NO_SURFACE) {
		eglSwapBuffers(eglDisplay, eglSurface);
	}
}

void WaylandOpenGL::Resize(int width, int height) {
	if (eglWindow) {
		wl_egl_window_resize(eglWindow, width, height, 0, 0);
	}
}

bool WaylandOpenGL::BindAPI() {
	if (eglBindAPI(EGL_OPENGL_API) == EGL_TRUE) {
		usesGLES = false;
		return true;
	}

	if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE) {
		usesGLES = true;
		return true;
	}

	throw WaylandError("Neither EGL_OPENGL_API nor EGL_OPENGL_ES_API is available.");
}

bool WaylandOpenGL::ChooseConfig(const InitParams& params, EGLConfig& config) {
	const EGLint redBits = ToColorComponentBits(params.colorBits);
	const EGLint alphaBits = params.colorBits >= 32 ? 8 : 0;

	const EGLint sampleBuffers = params.msaaSamples > 0 ? 1 : 0;

	const EGLint renderableType = usesGLES ? EGL_OPENGL_ES2_BIT : EGL_OPENGL_BIT;

	std::array<EGLint, 32> attribs{};
	int idx = 0;
	attribs[idx++] = EGL_SURFACE_TYPE;
	attribs[idx++] = EGL_WINDOW_BIT;
	attribs[idx++] = EGL_RED_SIZE;
	attribs[idx++] = redBits;
	attribs[idx++] = EGL_GREEN_SIZE;
	attribs[idx++] = redBits;
	attribs[idx++] = EGL_BLUE_SIZE;
	attribs[idx++] = redBits;
	attribs[idx++] = EGL_ALPHA_SIZE;
	attribs[idx++] = alphaBits;
	attribs[idx++] = EGL_DEPTH_SIZE;
	attribs[idx++] = static_cast<EGLint>(params.depthBits);
	attribs[idx++] = EGL_STENCIL_SIZE;
	attribs[idx++] = static_cast<EGLint>(params.stencilBits);
	attribs[idx++] = EGL_SAMPLE_BUFFERS;
	attribs[idx++] = sampleBuffers;
	attribs[idx++] = EGL_SAMPLES;
	attribs[idx++] = static_cast<EGLint>(params.msaaSamples);
	attribs[idx++] = EGL_RENDERABLE_TYPE;
	attribs[idx++] = renderableType;
	attribs[idx++] = EGL_NONE;

	EGLint numConfigs = 0;
	if (eglChooseConfig(eglDisplay, attribs.data(), &config, 1, &numConfigs) == EGL_FALSE || numConfigs == 0) {
		return false;
	}

	return true;
}

bool WaylandOpenGL::CreateEGLContext(const InitParams& params, EGLConfig config) {
	EGLint major = params.majorVersion;
	EGLint minor = params.minorVersion;

	if (major <= 0 || minor < 0) {
		major = usesGLES ? 3 : 4;
		minor = usesGLES ? 0 : 5;
	}

	std::vector<EGLint> contextAttribs;

	if (usesGLES) {
		contextAttribs = {
			EGL_CONTEXT_CLIENT_VERSION, major,
			EGL_NONE
		};
	} else {
		contextAttribs = {
			EGL_CONTEXT_MAJOR_VERSION, major,
			EGL_CONTEXT_MINOR_VERSION, minor,
			EGL_NONE
		};
	}

	eglContexts.clear();
	eglContexts.reserve(params.numGLContexts);

	EGLContext shared = EGL_NO_CONTEXT;
	for (uint32_t i = 0; i < params.numGLContexts; ++i) {
		EGLContext ctx = eglCreateContext(eglDisplay, config, shared, contextAttribs.data());
		if (ctx == EGL_NO_CONTEXT) {
			return false;
		}
		if (shared == EGL_NO_CONTEXT) {
			shared = ctx;
		}
		eglContexts.push_back(ctx);
	}

	return true;
}

} // namespace weave::wayland
