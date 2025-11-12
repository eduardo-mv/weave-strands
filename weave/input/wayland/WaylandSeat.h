#pragma once

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "weave/input/system/InputPipeline.h"
#include "weave/input/system/VirtualDevices.h"
#include "WaylandKeyboardFeed.h"
#include "WaylandMouseFeed.h"

namespace weave::input::wayland {

class WaylandSeat {
public:
	WaylandSeat() = default;
	~WaylandSeat();

	WaylandSeat(WaylandSeat const&) = delete;
	WaylandSeat& operator=(WaylandSeat const&) = delete;
	WaylandSeat(WaylandSeat&&) = delete;
	WaylandSeat& operator=(WaylandSeat&&) = delete;

	bool InitializeSeat(wl_display* display, wl_surface* surface, InputPipeline& pipeline, VirtualDevice keyboardDevice = VirtualDevice::Keyboard0, VirtualDevice mouseDevice = VirtualDevice::Mouse0);
	void SetKeyboardDevice(VirtualDevice device);
	void SetMouseDevice(VirtualDevice device);

	wl_surface* GetSurface() const { return surface; }
	xkb_context* GetXkbContext() const { return xkbContext; }

	static void HandleRegistryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
	static void HandleRegistryRemove(void* data, wl_registry* registry, uint32_t name);

	static void HandleSeatCapabilities(void* data, wl_seat* seat, uint32_t capabilities);
	static void HandleSeatName(void* data, wl_seat* seat, const char* name);

private:
	void BindSeat(uint32_t name, uint32_t version);
	void UnbindSeat(uint32_t name);
	void UpdateSeatCapabilities(uint32_t capabilities);

	void CreateKeyboard();
	void DestroyKeyboard();
	void CreateMouse();
	void DestroyMouse();

	void EnsureXkbContext();

	wl_display* display = nullptr;
	wl_surface* surface = nullptr;
	wl_registry* registry = nullptr;
	wl_seat* seat = nullptr;
	xkb_context* xkbContext = nullptr;
	InputPipeline* pipeline = nullptr;
	VirtualDevice keyboardDevice = VirtualDevice::Keyboard0;
	VirtualDevice mouseDevice = VirtualDevice::Mouse0;
	bool keyboardBound = false;
	bool mouseBound = false;

	WaylandKeyboardFeed keyboardFeed;
	WaylandMouseFeed mouseFeed;
};

} // namespace weave::input::wayland
