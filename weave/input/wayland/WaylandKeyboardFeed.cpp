#include "WaylandKeyboardFeed.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <sys/mman.h>
#include <unistd.h>

#include <xkbcommon/xkbcommon-keysyms.h>

namespace weave::input::wayland {

namespace {

const wl_keyboard_listener kKeyboardListener{
	WaylandKeyboardFeed::HandleKeymap,
	WaylandKeyboardFeed::HandleEnter,
	WaylandKeyboardFeed::HandleLeave,
	WaylandKeyboardFeed::HandleKey,
	WaylandKeyboardFeed::HandleModifiers,
	WaylandKeyboardFeed::HandleRepeatInfo
};

uint32_t ToXkbKeycode(uint32_t key) {
	return key + 8;
}

VirtualKeyState ToVirtualKeyState(uint32_t state) {
	return (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? VirtualKeyState::Down : VirtualKeyState::Up;
}

} // namespace

WaylandKeyboardFeed::~WaylandKeyboardFeed() {
	Unbind();
}

bool WaylandKeyboardFeed::Bind(wl_seat* seat, wl_surface* targetSurface, xkb_context* xkbContext, InputPipeline* pipelinePtr, VirtualDevice keyboardDevice) {
	if (!seat || !targetSurface || !xkbContext || !pipelinePtr) {
		return false;
	}
	if (keyboard) {
		return true;
	}

	pipeline = pipelinePtr;
	device = keyboardDevice;
	surface = targetSurface;
	context = xkbContext;

	keyboard = wl_seat_get_keyboard(seat);
	if (!keyboard) {
		pipeline = nullptr;
		return false;
	}

	wl_keyboard_add_listener(keyboard, &kKeyboardListener, this);
	pipeline->FeedDeviceEvent(device, DeviceState::Idle);
	return true;
}

void WaylandKeyboardFeed::Unbind() {
	if (keyboard) {
		wl_keyboard_release(keyboard);
		keyboard = nullptr;
	}
	if (state) {
		xkb_state_unref(state);
		state = nullptr;
	}
	if (keymap) {
		xkb_keymap_unref(keymap);
		keymap = nullptr;
	}
	if (pipeline) {
		pipeline->FeedDeviceEvent(device, DeviceState::Disconnected);
	}
	focused = false;
	surface = nullptr;
	context = nullptr;
}

void WaylandKeyboardFeed::SetVirtualDevice(VirtualDevice newDevice) {
	if (device == newDevice) {
		return;
	}
	const bool wasFocused = focused;
	if (pipeline && keyboard) {
		pipeline->FeedDeviceEvent(device, DeviceState::Disconnected);
	}
	device = newDevice;
	if (pipeline && keyboard) {
		pipeline->FeedDeviceEvent(device, DeviceState::Idle);
		if (wasFocused) {
			pipeline->FeedDeviceEvent(device, DeviceState::FocusGained);
		}
	}
}

void WaylandKeyboardFeed::HandleKeymap(void* data, wl_keyboard*, uint32_t format, int32_t fd, uint32_t size) {
	auto* self = static_cast<WaylandKeyboardFeed*>(data);
	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		close(fd);
		return;
	}

	char* map = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
	if (map == MAP_FAILED) {
		close(fd);
		throw std::runtime_error("Failed to mmap keyboard keymap");
	}

	if (self->keymap) {
		xkb_keymap_unref(self->keymap);
		self->keymap = nullptr;
	}
	if (self->state) {
		xkb_state_unref(self->state);
		self->state = nullptr;
	}

	self->keymap = xkb_keymap_new_from_string(self->context, map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(map, size);
	close(fd);

	if (!self->keymap) {
		throw std::runtime_error("Failed to compile xkb keymap");
	}

	self->state = xkb_state_new(self->keymap);
	if (!self->state) {
		throw std::runtime_error("Failed to create xkb state");
	}
}

void WaylandKeyboardFeed::HandleEnter(void* data, wl_keyboard*, uint32_t, wl_surface* surface, wl_array* keys) {
	auto* self = static_cast<WaylandKeyboardFeed*>(data);
	if (surface != self->surface) {
		return;
	}
	self->SetFocus(true);

	if (!self->pipeline || !self->state || !keys) {
		return;
	}

	const uint32_t* keycodes = static_cast<const uint32_t*>(keys->data);
	const std::size_t count = keys->size / sizeof(uint32_t);

	for (std::size_t i = 0; i < count; ++i) {
		const uint32_t keycode = ToXkbKeycode(keycodes[i]);
		xkb_state_update_key(self->state, keycode, XKB_KEY_DOWN);
		const auto sym = xkb_state_key_get_one_sym(self->state, keycode);
		const auto vk = self->TranslateKeysym(sym);
		if (vk != VirtualKey::None) {
			self->pipeline->FeedKeyEvent(self->device, vk, VirtualKeyState::Down);
		}
	}
}

void WaylandKeyboardFeed::HandleLeave(void* data, wl_keyboard*, uint32_t, wl_surface* surface) {
	auto* self = static_cast<WaylandKeyboardFeed*>(data);
	if (surface != self->surface) {
		return;
	}
	self->SetFocus(false);
	self->ResetState();
}

void WaylandKeyboardFeed::HandleKey(void* data, wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state) {
	auto* self = static_cast<WaylandKeyboardFeed*>(data);
	if (!self->focused || !self->state || !self->pipeline) {
		return;
	}

	const uint32_t keycode = ToXkbKeycode(key);
	const xkb_key_direction direction = (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? XKB_KEY_DOWN : XKB_KEY_UP;
	xkb_state_update_key(self->state, keycode, direction);

	const auto sym = xkb_state_key_get_one_sym(self->state, keycode);
	const auto vk = self->TranslateKeysym(sym);
	if (vk != VirtualKey::None) {
		self->pipeline->FeedKeyEvent(self->device, vk, ToVirtualKeyState(state));
	}
}

void WaylandKeyboardFeed::HandleModifiers(void* data, wl_keyboard*, uint32_t, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
	auto* self = static_cast<WaylandKeyboardFeed*>(data);
	if (!self->state) {
		return;
	}
	xkb_state_update_mask(self->state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

void WaylandKeyboardFeed::HandleRepeatInfo(void*, wl_keyboard*, int32_t, int32_t) {
	// Repeats handled by compositor; nothing to do.
}

void WaylandKeyboardFeed::ResetState() {
	if (!keymap) {
		return;
	}
	if (state) {
		xkb_state_unref(state);
	}
	state = xkb_state_new(keymap);
}

void WaylandKeyboardFeed::SetFocus(bool focusedState) {
	if (focused == focusedState) {
		return;
	}
	focused = focusedState;
	if (pipeline) {
		pipeline->FeedDeviceEvent(device, focused ? DeviceState::FocusGained : DeviceState::FocusLost);
	}
	if (!focused) {
		ResetState();
	}
}

VirtualKey WaylandKeyboardFeed::TranslateKeysym(xkb_keysym_t sym) const {
	if (sym >= XKB_KEY_A && sym <= XKB_KEY_Z) {
		
		return static_cast<VirtualKey>(weave::input::OffsetKey(VirtualKey::A, sym - XKB_KEY_A));
	}
	if (sym >= XKB_KEY_a && sym <= XKB_KEY_z) {
		return static_cast<VirtualKey>(weave::input::OffsetKey(VirtualKey::A, sym - XKB_KEY_a));
	}
	if (sym >= XKB_KEY_0 && sym <= XKB_KEY_9) {
		return static_cast<VirtualKey>(weave::input::OffsetKey(VirtualKey::Key_0, sym - XKB_KEY_0));
	}
	if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F12) {
		return static_cast<VirtualKey>(weave::input::OffsetKey(VirtualKey::F1, sym - XKB_KEY_F1));
	}
	if (sym >= XKB_KEY_KP_0 && sym <= XKB_KEY_KP_9) {
		return static_cast<VirtualKey>(weave::input::OffsetKey(VirtualKey::Numpad0, sym - XKB_KEY_KP_0));
	}

	switch (sym) {
	case XKB_KEY_Return:
	case XKB_KEY_KP_Enter: return VirtualKey::Enter;
	case XKB_KEY_Escape: return VirtualKey::Esc;
	case XKB_KEY_Tab:
	case XKB_KEY_ISO_Left_Tab: return VirtualKey::Tab;
	case XKB_KEY_space: return VirtualKey::Space;
	case XKB_KEY_BackSpace: return VirtualKey::Backspace;
	case XKB_KEY_Delete: return VirtualKey::Delete;
	case XKB_KEY_Insert: return VirtualKey::Insert;
	case XKB_KEY_Home: return VirtualKey::Home;
	case XKB_KEY_End: return VirtualKey::End;
	case XKB_KEY_Page_Up: return VirtualKey::Pageup;
	case XKB_KEY_Page_Down: return VirtualKey::Pagedown;
	case XKB_KEY_Left: return VirtualKey::Left;
	case XKB_KEY_Right: return VirtualKey::Right;
	case XKB_KEY_Up: return VirtualKey::Up;
	case XKB_KEY_Down: return VirtualKey::Down;
	case XKB_KEY_Control_L: return VirtualKey::Lcontrol;
	case XKB_KEY_Control_R: return VirtualKey::Rcontrol;
	case XKB_KEY_Shift_L: return VirtualKey::Lshift;
	case XKB_KEY_Shift_R: return VirtualKey::Rshift;
	case XKB_KEY_Alt_L: return VirtualKey::Alt;
	case XKB_KEY_Alt_R: return VirtualKey::Altgr;
	case XKB_KEY_Super_L: return VirtualKey::Lwin;
	case XKB_KEY_Super_R: return VirtualKey::Rwin;
	case XKB_KEY_Caps_Lock: return VirtualKey::Capslock;
	case XKB_KEY_Num_Lock: return VirtualKey::Numlock;
	case XKB_KEY_Scroll_Lock: return VirtualKey::Scroll;
	case XKB_KEY_Print: return VirtualKey::Printscreen;
	case XKB_KEY_Pause: return VirtualKey::Pause;
	case XKB_KEY_minus: return VirtualKey::Minus;
	case XKB_KEY_equal:
	case XKB_KEY_plus: return VirtualKey::Plus;
	case XKB_KEY_comma: return VirtualKey::Comma;
	case XKB_KEY_period: return VirtualKey::Period;
	case XKB_KEY_slash: return VirtualKey::Vary6;
	case XKB_KEY_backslash:
	case XKB_KEY_bar: return VirtualKey::Vary7;
	case XKB_KEY_section:
	case XKB_KEY_brokenbar: return VirtualKey::Vary8;
	case XKB_KEY_less:
	case XKB_KEY_greater:
	case XKB_KEY_guillemotleft:
	case XKB_KEY_guillemotright: return VirtualKey::Vary102;
	case XKB_KEY_semicolon:
	case XKB_KEY_ccedilla:
	case XKB_KEY_Ccedilla: return VirtualKey::Vary3;
	case XKB_KEY_apostrophe:
	case XKB_KEY_dead_acute: return VirtualKey::Vary4;
	case XKB_KEY_bracketleft: return VirtualKey::Vary1;
	case XKB_KEY_bracketright: return VirtualKey::Vary2;
	case XKB_KEY_grave:
	case XKB_KEY_asciitilde:
	case XKB_KEY_dead_grave:
	case XKB_KEY_dead_tilde:
	case XKB_KEY_masculine: return VirtualKey::Vary5;
	case XKB_KEY_KP_Add: return VirtualKey::Add;
	case XKB_KEY_KP_Subtract: return VirtualKey::Subtract;
	case XKB_KEY_KP_Multiply: return VirtualKey::Multiply;
	case XKB_KEY_KP_Divide: return VirtualKey::Divide;
	case XKB_KEY_KP_Decimal: return VirtualKey::Decimal;
	case XKB_KEY_KP_Separator: return VirtualKey::Separator;
	case XKB_KEY_Menu: return VirtualKey::Winapps;
	case XKB_KEY_Select: return VirtualKey::Select;
	case XKB_KEY_Clear: return VirtualKey::Clear;
	case XKB_KEY_XF86AudioMute: return VirtualKey::Volumemute;
	case XKB_KEY_XF86AudioLowerVolume: return VirtualKey::Volumedown;
	case XKB_KEY_XF86AudioRaiseVolume: return VirtualKey::Volumeup;
	case XKB_KEY_XF86AudioNext: return VirtualKey::Medianext;
	case XKB_KEY_XF86AudioPrev: return VirtualKey::Mediaprev;
	case XKB_KEY_XF86AudioStop: return VirtualKey::Mediastop;
	case XKB_KEY_XF86AudioPlay:
	case XKB_KEY_XF86AudioPause: return VirtualKey::Mediaplaypause;
	default:
		break;
	}

	return VirtualKey::None;
}

} // namespace weave::input::wayland
