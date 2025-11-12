#pragma once

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "weave/input/system/InputPipeline.h"
#include "weave/input/system/VirtualDevices.h"
#include "weave/input/system/VirtualKeys.h"

namespace weave::input::wayland {

class WaylandKeyboardFeed {
public:
	WaylandKeyboardFeed() = default;
	~WaylandKeyboardFeed();

	WaylandKeyboardFeed(WaylandKeyboardFeed const&) = delete;
	WaylandKeyboardFeed& operator=(WaylandKeyboardFeed const&) = delete;
	WaylandKeyboardFeed(WaylandKeyboardFeed&&) = delete;
	WaylandKeyboardFeed& operator=(WaylandKeyboardFeed&&) = delete;

	bool Bind(wl_seat* seat, wl_surface* surface, xkb_context* context, InputPipeline* pipeline, VirtualDevice keyboardDevice);
	void Unbind();
	void SetInputPipeline(InputPipeline* pipeline);
	void SetVirtualDevice(VirtualDevice device);

	static void HandleKeymap(void* data, wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size);
	static void HandleEnter(void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface, wl_array* keys);
	static void HandleLeave(void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface);
	static void HandleKey(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
	static void HandleModifiers(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
	static void HandleRepeatInfo(void* data, wl_keyboard* keyboard, int32_t rate, int32_t delay);

private:
	void ResetState();
	VirtualKey TranslateKeysym(xkb_keysym_t sym) const;
	void SetFocus(bool focused);

	InputPipeline* pipeline = nullptr;
	VirtualDevice device = VirtualDevice::Keyboard0;
	wl_surface* surface = nullptr;
	wl_keyboard* keyboard = nullptr;
	xkb_context* context = nullptr;
	xkb_keymap* keymap = nullptr;
	xkb_state* state = nullptr;
	bool focused = false;
};

} // namespace weave::input::wayland
