#include "WaylandSeat.h"

#include "WaylandKeyboardFeed.h"
#include "WaylandMouseFeed.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace weave::input::wayland {

namespace {

constexpr uint32_t kSeatVersion = 7;

const wl_registry_listener RegistryListener{
	WaylandSeat::HandleRegistryGlobal,
	WaylandSeat::HandleRegistryRemove
};

const wl_seat_listener SeatListener{
	WaylandSeat::HandleSeatCapabilities,
	WaylandSeat::HandleSeatName
};

} // namespace

WaylandSeat::~WaylandSeat() {
	DestroyMouse();
	DestroyKeyboard();

	if (seat) {
		wl_seat_destroy(seat);
		seat = nullptr;
	}
	if (registry) {
		wl_registry_destroy(registry);
		registry = nullptr;
	}
	if (xkbContext) {
		xkb_context_unref(xkbContext);
		xkbContext = nullptr;
	}
}

void WaylandSeat::EnsureXkbContext() {
	if (!xkbContext) {
		xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		if (!xkbContext) {
			throw std::runtime_error("Failed to create xkb context");
		}
	}
}

void WaylandSeat::HandleRegistryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
	auto* self = static_cast<WaylandSeat*>(data);
	(void)registry;
	if (std::strcmp(interface, wl_seat_interface.name) == 0) {
		self->BindSeat(name, std::min<uint32_t>(version, kSeatVersion));
	}
}

void WaylandSeat::HandleRegistryRemove(void* data, wl_registry*, uint32_t name) {
	auto* self = static_cast<WaylandSeat*>(data);
	self->UnbindSeat(name);
}

void WaylandSeat::BindSeat(uint32_t name, uint32_t version) {
	if (seat) {
		return;
	}
	if (!registry) {
		return;
	}
	seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, version));
	if (!seat) {
		throw std::runtime_error("Failed to bind wl_seat");
	}
	wl_seat_add_listener(seat, &SeatListener, this);
}

void WaylandSeat::UnbindSeat(uint32_t) {
	if (!seat) {
		return;
	}
	DestroyMouse();
	DestroyKeyboard();
	wl_seat_destroy(seat);
	seat = nullptr;
}

void WaylandSeat::HandleSeatCapabilities(void* data, wl_seat* seat, uint32_t capabilities) {
	auto* self = static_cast<WaylandSeat*>(data);
	if (seat != self->seat) {
		return;
	}
	self->UpdateSeatCapabilities(capabilities);
}

void WaylandSeat::HandleSeatName(void*, wl_seat*, const char*) {
	// Seat name unused.
}

void WaylandSeat::UpdateSeatCapabilities(uint32_t capabilities) {
	const bool havePointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
	const bool haveKeyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

	if (havePointer) {
		CreateMouse();
	} else {
		DestroyMouse();
	}

	if (haveKeyboard) {
		CreateKeyboard();
	} else {
		DestroyKeyboard();
	}
}

void WaylandSeat::CreateKeyboard() {
	if (!seat || keyboardBound) {
		return;
	}
	EnsureXkbContext();
	if (!keyboardFeed.Bind(seat, surface, xkbContext, pipeline, keyboardDevice)) {
		throw std::runtime_error("Failed to bind Wayland keyboard feed");
	}
	keyboardBound = true;
}

void WaylandSeat::DestroyKeyboard() {
	if (!keyboardBound) {
		return;
	}
	keyboardFeed.Unbind();
	keyboardBound = false;
}

void WaylandSeat::CreateMouse() {
	if (!seat || mouseBound) {
		return;
	}
	if (!mouseFeed.Bind(seat, surface, pipeline, mouseDevice)) {
		throw std::runtime_error("Failed to bind Wayland mouse feed");
	}
	mouseBound = true;
}

void WaylandSeat::DestroyMouse() {
	if (!mouseBound) {
		return;
	}
	mouseFeed.Unbind();
	mouseBound = false;
}

bool WaylandSeat::InitializeSeat(wl_display* displayIn, wl_surface* surfaceIn, InputPipeline& pipelineRef, VirtualDevice keyboardDev, VirtualDevice mouseDev) {
	if (registry) {
		return true;
	}
	if (!displayIn || !surfaceIn) {
		return false;
	}

	display = displayIn;
	surface = surfaceIn;
	pipeline = &pipelineRef;
	keyboardDevice = keyboardDev;
	mouseDevice = mouseDev;

	registry = wl_display_get_registry(display);
	if (!registry) {
		return false;
	}
	wl_registry_add_listener(registry, &RegistryListener, this);

	wl_display_roundtrip(display);
	wl_display_roundtrip(display);

	return true;
}

void WaylandSeat::SetKeyboardDevice(VirtualDevice device) {
	keyboardDevice = device;
	if (keyboardBound) {
		keyboardFeed.SetVirtualDevice(device);
	}
}

void WaylandSeat::SetMouseDevice(VirtualDevice device) {
	mouseDevice = device;
	if (mouseBound) {
		mouseFeed.SetVirtualDevice(device);
	}
}

} // namespace weave::input::wayland
