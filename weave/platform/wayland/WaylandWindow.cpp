#include "WaylandWindow.h"

#include "xdg-shell-client-protocol.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace weave::wayland {

namespace {

const wl_registry_listener RegistryListener = {
	WaylandWindow::HandleRegistryGlobal,
	WaylandWindow::HandleRegistryRemove
};

const xdg_wm_base_listener WmBaseListener = {
	WaylandWindow::HandleWmBasePing
};

const xdg_surface_listener XdgSurfaceListener = {
	WaylandWindow::HandleXdgSurfaceConfigure
};

const xdg_toplevel_listener XdgToplevelListener = {
	WaylandWindow::HandleXdgToplevelConfigure,
	WaylandWindow::HandleXdgToplevelClose
};

} // namespace

WaylandWindow::WaylandWindow() = default;

WaylandWindow::~WaylandWindow() {
	Destroy();
}

bool WaylandWindow::Create() {
	return Create(InitParams{});
}

bool WaylandWindow::Create(InitParams params) {
	if (display) {
		return true;
	}

	display = wl_display_connect(nullptr);
	if (!display) {
		throw WaylandError("Failed to connect to Wayland display.");
	}

	registry = wl_display_get_registry(display);
	if (!registry) {
		throw WaylandError("Failed to obtain Wayland registry.");
	}

	wl_registry_add_listener(registry, &RegistryListener, this);

	// Roundtrip to ensure globals are received
	wl_display_roundtrip(display);

	if (!compositor) {
		throw WaylandError("Wayland compositor is unavailable.");
	}
	if (!wmBase) {
		throw WaylandError("Wayland xdg_wm_base interface is unavailable.");
	}

	surface = wl_compositor_create_surface(compositor);
	if (!surface) {
		throw WaylandError("Failed to create Wayland surface.");
	}

	xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, surface);
	if (!xdgSurface) {
		throw WaylandError("Failed to create xdg_surface.");
	}
	xdg_surface_add_listener(xdgSurface, &XdgSurfaceListener, this);

	toplevel = xdg_surface_get_toplevel(xdgSurface);
	if (!toplevel) {
		throw WaylandError("Failed to create xdg_toplevel.");
	}
	xdg_toplevel_add_listener(toplevel, &XdgToplevelListener, this);

	if (decorationManager) {
		decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(decorationManager, toplevel);
		if (decoration) {
			zxdg_toplevel_decoration_v1_set_mode(decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
		}
	}

	SetTitle(params.title);
	if (params.fullscreen) {
		xdg_toplevel_set_fullscreen(toplevel, nullptr);
	}

	width = params.width;
	height = params.height;
	windowedWidth = static_cast<uint32_t>(width);
	windowedHeight = static_cast<uint32_t>(height);
	isFullscreen = params.fullscreen;

	wl_surface_commit(surface);
	wl_display_roundtrip(display);
	EnsureConfigured();

	running = true;
	shouldClose = false;

	return true;
}

void WaylandWindow::Destroy() {
	for (auto* eglWindow : eglWindows) {
		if (eglWindow) {
			wl_egl_window_destroy(eglWindow);
		}
	}
	eglWindows.clear();

	if (decoration) {
		zxdg_toplevel_decoration_v1_destroy(decoration);
		decoration = nullptr;
	}
	if (decorationManager) {
		zxdg_decoration_manager_v1_destroy(decorationManager);
		decorationManager = nullptr;
	}
	if (toplevel) {
		xdg_toplevel_destroy(toplevel);
		toplevel = nullptr;
	}
	if (xdgSurface) {
		xdg_surface_destroy(xdgSurface);
		xdgSurface = nullptr;
	}
	if (surface) {
		wl_surface_destroy(surface);
		surface = nullptr;
	}
	if (wmBase) {
		xdg_wm_base_destroy(wmBase);
		wmBase = nullptr;
	}
	if (compositor) {
		wl_compositor_destroy(compositor);
		compositor = nullptr;
	}
	compositorName = 0;
	if (registry) {
		wl_registry_destroy(registry);
		registry = nullptr;
	}
	wmBaseName = 0;
	decorationManagerName = 0;
	if (display) {
		wl_display_disconnect(display);
		display = nullptr;
	}

	windowedWidth = 0;
	windowedHeight = 0;

	running = false;
	shouldClose = false;
	configured = false;
	isFullscreen = false;
}

bool WaylandWindow::Fullscreen(uint32_t widthHint, uint32_t heightHint, uint32_t) {
	(void)widthHint;
	(void)heightHint;

	if (!toplevel) {
		return false;
	}

	xdg_toplevel_set_fullscreen(toplevel, nullptr);
	isFullscreen = true;
	return true;
}

void WaylandWindow::Windowed(int, int, uint32_t widthHint, uint32_t heightHint, bool, bool borderless, bool show) {
	if (!toplevel) {
		return;
	}

	if (isFullscreen) {
		xdg_toplevel_unset_fullscreen(toplevel);
		isFullscreen = false;
	}

	if (decoration) {
		zxdg_toplevel_decoration_v1_set_mode(decoration, borderless ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE
		                                                           : ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
	}

	if (widthHint > 0) {
		windowedWidth = widthHint;
		width = static_cast<int>(widthHint);
	}
	if (heightHint > 0) {
		windowedHeight = heightHint;
		height = static_cast<int>(heightHint);
	}

	if (xdgSurface && windowedWidth > 0 && windowedHeight > 0) {
		xdg_surface_set_window_geometry(
			xdgSurface, 0, 0, static_cast<int32_t>(windowedWidth), static_cast<int32_t>(windowedHeight));
	}

	if (toplevel) {
		xdg_toplevel_unset_maximized(toplevel);
	}

	UpdateEGLWindowSizes();
	Show(show);
}

void WaylandWindow::Maximize() {
	if (!toplevel) {
		return;
	}
	xdg_toplevel_set_maximized(toplevel);
}

void WaylandWindow::Minimize() {
	if (!toplevel) {
		return;
	}
	xdg_toplevel_set_minimized(toplevel);
}

void WaylandWindow::Show() {
	if (!surface) {
		return;
	}
	EnsureConfigured();
	wl_surface_commit(surface);
	wl_display_flush(display);
}

void WaylandWindow::Show(bool show) {
	if (!surface) {
		return;
	}
	if (show) {
		Show();
		return;
	}

	// Wayland does not offer a direct equivalent to Win32's ShowWindow(SW_HIDE).
	// Minimizing is the closest cross-platform behaviour.
	Minimize();
}

std::pair<uint32_t, uint32_t> WaylandWindow::GetClientArea() const {
	return {static_cast<uint32_t>(std::max(width, 0)), static_cast<uint32_t>(std::max(height, 0))};
}

std::pair<uint32_t, uint32_t> WaylandWindow::GetWindowArea() const {
	return GetClientArea();
}

void WaylandWindow::UpdateEGLWindowSizes() {
	if (!surface) {
		return;
	}

	const int targetWidth = std::max(width, 1);
	const int targetHeight = std::max(height, 1);

	for (auto* eglWindow : eglWindows) {
		if (eglWindow) {
			wl_egl_window_resize(eglWindow, targetWidth, targetHeight, 0, 0);
		}
	}
	wl_surface_commit(surface);
	wl_display_flush(display);
}

void WaylandWindow::SetTitle(const std::string& title) {
	if (toplevel) {
		xdg_toplevel_set_title(toplevel, title.c_str());
	}
}

void WaylandWindow::MessagePump() {
	if (!display) {
		return;
	}

	running = true;
	while (running && !shouldClose) {
		if (wl_display_dispatch(display) < 0) {
			throw WaylandError("Wayland dispatch failed.");
		}
	}
}

void WaylandWindow::MessagePeekPump(const std::function<void()>& idleCallback) {
	if (!display) {
		return;
	}

	running = true;
	while (running && !shouldClose) {
		if (wl_display_prepare_read(display) == 0) {
			wl_display_flush(display);
			if (wl_display_read_events(display) < 0) {
				wl_display_cancel_read(display);
				throw WaylandError("Wayland read events failed.");
			}
		} else {
			wl_display_dispatch_pending(display);
		}

		wl_display_dispatch_pending(display);

		if (idleCallback) {
			idleCallback();
		}
	}
}

void WaylandWindow::StopMessagePump() {
	running = false;
}

wl_egl_window* WaylandWindow::CreateEGLWindow(int w, int h) {
	if (!surface) {
		return nullptr;
	}

	if (w <= 0) {
		w = std::max(width, 1);
	}
	if (h <= 0) {
		h = std::max(height, 1);
	}

	auto* window = wl_egl_window_create(surface, w, h);
	if (!window) {
		throw WaylandError("Failed to create wl_egl_window.");
	}

	eglWindows.push_back(window);
	return window;
}

void WaylandWindow::DestroyEGLWindow(wl_egl_window* window) {
	if (!window) {
		return;
	}
	auto it = std::find(eglWindows.begin(), eglWindows.end(), window);
	if (it != eglWindows.end()) {
		wl_egl_window_destroy(*it);
		eglWindows.erase(it);
	}
}

void WaylandWindow::DispatchPending() {
	if (!display) {
		return;
	}
	wl_display_dispatch_pending(display);
	wl_display_flush(display);
}

void WaylandWindow::HandleRegistryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
	auto* window = static_cast<WaylandWindow*>(data);
	window->BindInterfaces(registry, name, interface, version);
}

void WaylandWindow::HandleRegistryRemove(void* data, wl_registry* registry, uint32_t name) {
	(void)registry;
	auto* window = static_cast<WaylandWindow*>(data);
	// Clear pointers if globals disappear
	if (window->compositorName == name) {
		window->compositor = nullptr;
		window->compositorName = 0;
	}
	if (window->wmBaseName == name) {
		window->wmBase = nullptr;
		window->wmBaseName = 0;
	}
	if (window->decorationManagerName == name) {
		window->decorationManager = nullptr;
		window->decorationManagerName = 0;
	}
}

void WaylandWindow::HandleWmBasePing(void* data, xdg_wm_base* wm_base, uint32_t serial) {
	(void)data;
	xdg_wm_base_pong(wm_base, serial);
}

void WaylandWindow::HandleXdgSurfaceConfigure(void* data, xdg_surface* surface, uint32_t serial) {
	auto* window = static_cast<WaylandWindow*>(data);
	xdg_surface_ack_configure(surface, serial);
	window->configured = true;
}

void WaylandWindow::HandleXdgToplevelConfigure(void* data, xdg_toplevel*, int32_t width, int32_t height, wl_array*) {
	auto* window = static_cast<WaylandWindow*>(data);

	if (width > 0) {
		window->width = width;
		if (!window->isFullscreen) {
			window->windowedWidth = static_cast<uint32_t>(width);
		}
	}
	if (height > 0) {
		window->height = height;
		if (!window->isFullscreen) {
			window->windowedHeight = static_cast<uint32_t>(height);
		}
	}

	window->UpdateEGLWindowSizes();
}

void WaylandWindow::HandleXdgToplevelClose(void* data, xdg_toplevel*) {
	auto* window = static_cast<WaylandWindow*>(data);
	window->shouldClose = true;
	window->StopMessagePump();
}

void WaylandWindow::BindInterfaces(wl_registry* registryObj, uint32_t name, const char* interface, uint32_t version) {
	if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
		compositor = static_cast<wl_compositor*>(wl_registry_bind(registryObj, name, &wl_compositor_interface, std::min<uint32_t>(version, 4)));
		compositorName = name;
	} else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0) {
		wmBase = static_cast<xdg_wm_base*>(wl_registry_bind(registryObj, name, &xdg_wm_base_interface, 2));
		xdg_wm_base_add_listener(wmBase, &WmBaseListener, this);
		wmBaseName = name;
	} else if (std::strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		decorationManager = static_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(registryObj, name, &zxdg_decoration_manager_v1_interface, 1));
		decorationManagerName = name;
	}
}

void WaylandWindow::EnsureConfigured() {
	if (!configured && display) {
		wl_display_roundtrip(display);
	}
	if (!configured) {
		configured = true;
		if (width == 0) width = 1280;
		if (height == 0) height = 720;
	}
}

} // namespace weave::wayland
