#pragma once

#include <wayland-client.h>
#include <wayland-egl.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "WaylandExceptions.h"

struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;
struct zxdg_decoration_manager_v1;
struct zxdg_toplevel_decoration_v1;

namespace weave::wayland {

class WaylandWindow {
public:
	struct InitParams {
		int width = 1280;
		int height = 720;
		std::string title = "Weave Wayland Window";
		bool fullscreen = false;
	};

	WaylandWindow();
	~WaylandWindow();

	WaylandWindow(WaylandWindow const&) = delete;
	WaylandWindow& operator=(WaylandWindow const&) = delete;
	WaylandWindow(WaylandWindow&&) = delete;
	WaylandWindow& operator=(WaylandWindow&&) = delete;

	bool Create();
	bool Create(InitParams params);
	void Destroy();

	bool Fullscreen(uint32_t width = 0, uint32_t height = 0, uint32_t hz = 0);
	void Windowed(uint32_t width, uint32_t height, bool sizeIsWindow, bool borderless, bool show) { Windowed(-1, -1, width, height, sizeIsWindow, borderless, show); }
	void Windowed(int x, int y, uint32_t width, uint32_t height, bool sizeIsWindow, bool borderless, bool show);
	void Maximize();
	void Minimize();

	void Show();
	void Show(bool show);
	void SetTitle(const std::string& title);

	void MessagePump();
	void MessagePeekPump(const std::function<void()>& idleCallback);
	void StopMessagePump();

	std::pair<int, int> GetSize() const { return {width, height}; }
	std::pair<uint32_t, uint32_t> GetClientArea() const;
	std::pair<uint32_t, uint32_t> GetWindowArea() const;
	bool ShouldClose() const { return shouldClose; }

	// Wayland/EGL helpers
	wl_display* GetDisplay() const { return display; }
	wl_surface* GetSurface() const { return surface; }
	xdg_wm_base* GetWMBase() const { return wmBase; }

	wl_egl_window* CreateEGLWindow(int w, int h);
	void DestroyEGLWindow(wl_egl_window* window);

	void DispatchPending();

	static void HandleRegistryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
	static void HandleRegistryRemove(void* data, wl_registry* registry, uint32_t name);

	static void HandleWmBasePing(void* data, xdg_wm_base* wm_base, uint32_t serial);
	static void HandleXdgSurfaceConfigure(void* data, xdg_surface* surface, uint32_t serial);
	static void HandleXdgToplevelConfigure(void* data, xdg_toplevel* toplevel, int32_t width, int32_t height, wl_array* states);
	static void HandleXdgToplevelClose(void* data, xdg_toplevel* toplevel);

private:
	void BindInterfaces(wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
	void EnsureConfigured();

	wl_display* display = nullptr;
	wl_registry* registry = nullptr;
    wl_compositor* compositor = nullptr;
    xdg_wm_base* wmBase = nullptr;
    zxdg_decoration_manager_v1* decorationManager = nullptr;
    zxdg_toplevel_decoration_v1* decoration = nullptr;
	wl_surface* surface = nullptr;
	xdg_surface* xdgSurface = nullptr;
	xdg_toplevel* toplevel = nullptr;

	std::vector<wl_egl_window*> eglWindows;

	int width = 0;
	int height = 0;
	bool running = false;
	bool shouldClose = false;
	bool configured = false;
	bool isFullscreen = false;

    uint32_t compositorName = 0;
    uint32_t wmBaseName = 0;
    uint32_t decorationManagerName = 0;

	uint32_t windowedWidth = 0;
	uint32_t windowedHeight = 0;

	void UpdateEGLWindowSizes();
};

} // namespace weave::wayland
