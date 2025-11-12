#include "ContextPool.h"
#include <algorithm>

using namespace weave;
using namespace opengl;
//Static declaration for the vector holding all the registered contexts
std::vector<ContextPool::ContextData> ContextPool::contexts;
std::mutex ContextPool::contextMutex;

#ifdef WIN32

void weave::opengl::ContextPool::ContextData::Activate() {
	threadId = std::this_thread::get_id();
	int retries = 100;
	do {
		wglMakeCurrent(hdc, glrc);
		std::this_thread::yield();
	} while (wglGetCurrentContext() != glrc && (--retries) > 0);
}

void weave::opengl::ContextPool::ContextData::Release() {
	gl::Flush();
	wglMakeCurrent(nullptr, nullptr);
	threadId = {};
}

void weave::opengl::ContextPool::RegisterContext(HDC hdc, HGLRC glrc) {
	contexts.emplace_back(hdc, glrc);
}

void weave::opengl::ContextPool::RegisterContexts(HDC hdc, std::vector<HGLRC> const & glrc) {
	for (auto& rc : glrc) {
		contexts.emplace_back(hdc, rc);
	}
}

HDC weave::opengl::ContextPool::GetWindowsHDC() {
	return (contexts.size() ? contexts[0].hdc : HDC());
}

#elif defined(NN_NINTENDO_SDK) || defined(WEAVE_PLATFORM_WAYLAND)

void weave::opengl::ContextPool::ContextData::Activate() {
	threadId = std::this_thread::get_id();
	int retries = 100;
	do {
		eglMakeCurrent(hDisplay, hSurface, hSurface, hContext);
		std::this_thread::yield();
	} while (eglGetCurrentContext() != hContext && (--retries) > 0);
}

void weave::opengl::ContextPool::ContextData::Release() {
	eglMakeCurrent(hDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	threadId = std::thread::id();
}

//Registers a context. This should be used to populate contexts during initialization of the system
void weave::opengl::ContextPool::RegisterContext(EGLDisplay display, EGLSurface surface, EGLContext context) {
	contexts.emplace_back(display, surface, context);
}

void weave::opengl::ContextPool::RegisterContexts(EGLDisplay display, EGLSurface surface, std::vector<EGLContext> const &context) {
	int i = 0;
	for(auto &ctx : context) {
		contexts.emplace_back(display, i++ == 0 ? surface : EGL_NO_SURFACE, ctx);
	}
}

//Returns the Display of the contexts
EGLDisplay weave::opengl::ContextPool::GetDisplayHandle() {
	return (contexts.size() ? contexts[0].hDisplay : EGLDisplay());
}

//Returns the Surface of the contexts
EGLSurface weave::opengl::ContextPool::GetSurfaceHandle() {
	return (contexts.size() ? contexts[0].hSurface : EGLSurface());
}
#endif

uint32_t weave::opengl::ContextPool::FreeContextsCount()
{
	std::thread::id noID;

	auto count = std::count_if(std::begin(contexts), std::end(contexts), [&noID](auto& item) {
		return item.threadId == noID;
	});

	return uint32_t(count);
}

void weave::opengl::ContextPool::ClaimContext() {
	auto thisThread = std::this_thread::get_id();
	std::lock_guard lock(contextMutex);

	auto it = std::find_if(std::begin(contexts), std::end(contexts), [&](auto const& c) {
		return c.threadId == thisThread;
	});

	if (it == std::end(contexts)) {
		std::thread::id noID;
		it = std::find_if(std::begin(contexts), std::end(contexts), [&](auto const& c) {
			return c.threadId == noID;
		});
	}

	if (it != std::end(contexts)) {
		it->Activate();
	}
}

void weave::opengl::ContextPool::ReleaseContext() {
	auto thisThread = std::this_thread::get_id();
	std::lock_guard lock(contextMutex);

	auto it = std::find_if(std::begin(contexts), std::end(contexts), [&](auto const& c) {
		return c.threadId == thisThread;
	});

	if (it != std::end(contexts)) {
		it->Release();
	}
}

bool weave::opengl::ContextPool::HasClaimedContext() {
	auto thisThread = std::this_thread::get_id();
	std::lock_guard lock(contextMutex);

	auto it = std::find_if(std::begin(contexts), std::end(contexts), [&](auto const& c) {
		return c.threadId == thisThread;
	});

	return (it != std::end(contexts));
}

Context::Context() {
	Claim();
}


Context::~Context() {
	Release();
}

bool weave::opengl::Context::IsValid() {
	return ContextPool::HasClaimedContext();
}

void weave::opengl::Context::Claim() {
	ContextPool::ClaimContext();
}

void weave::opengl::Context::Release() {
	ContextPool::ReleaseContext();
}


