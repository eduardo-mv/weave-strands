#include "WaylandMouseFeed.h"

#include <algorithm>
#include <stdexcept>

#include <linux/input-event-codes.h>

namespace weave::input::wayland {

namespace {

const wl_pointer_listener kPointerListener{
	WaylandMouseFeed::HandleEnter,
	WaylandMouseFeed::HandleLeave,
	WaylandMouseFeed::HandleMotion,
	WaylandMouseFeed::HandleButton,
	WaylandMouseFeed::HandleAxis,
	WaylandMouseFeed::HandleFrame,
	WaylandMouseFeed::HandleAxisSource,
	WaylandMouseFeed::HandleAxisStop,
	WaylandMouseFeed::HandleAxisDiscrete,
	WaylandMouseFeed::HandleAxisValue120,
	WaylandMouseFeed::HandleAxisRelativeDirection
};

} // namespace

WaylandMouseFeed::~WaylandMouseFeed() {
	Unbind();
}

void WaylandMouseFeed::SetInputPipeline(InputPipeline* pipelinePtr) {
	if (pipeline == pipelinePtr) {
		return;
	}

	const bool wasFocused = focused;
	if (pipeline && pointer) {
		pipeline->FeedDeviceEvent(device, DeviceState::Disconnected);
	}

	pipeline = pipelinePtr;

	if (pipeline && pointer) {
		pipeline->FeedDeviceEvent(device, DeviceState::Idle);
		if (wasFocused) {
			pipeline->FeedDeviceEvent(device, DeviceState::FocusGained);
		}
	}
}

void WaylandMouseFeed::SetVirtualDevice(VirtualDevice newDevice) {
	if (device == newDevice) {
		return;
	}
	if (pipeline && pointer) {
		pipeline->FeedDeviceEvent(device, DeviceState::Disconnected);
	}
	device = newDevice;
	if (pipeline && pointer) {
		pipeline->FeedDeviceEvent(device, DeviceState::Idle);
		if (focused) {
			pipeline->FeedDeviceEvent(device, DeviceState::FocusGained);
		}
	}
}

bool WaylandMouseFeed::Bind(wl_seat* seat, wl_surface* targetSurface, InputPipeline* pipelinePtr, VirtualDevice mouseDevice) {
	if (!seat || !targetSurface || !pipelinePtr) {
		return false;
	}
	if (pointer) {
		return true;
	}

	pipeline = pipelinePtr;
	device = mouseDevice;
	surface = targetSurface;
	pointer = wl_seat_get_pointer(seat);
	if (!pointer) {
		pipeline = nullptr;
		surface = nullptr;
		return false;
	}
	wl_pointer_add_listener(pointer, &kPointerListener, this);
	pipeline->FeedDeviceEvent(device, DeviceState::Idle);
	return true;
}

void WaylandMouseFeed::Unbind() {
	if (pointer) {
		wl_pointer_release(pointer);
		pointer = nullptr;
	}
	if (pipeline) {
		pipeline->FeedDeviceEvent(device, DeviceState::Disconnected);
	}
	focused = false;
	surface = nullptr;
	ClearAxisState();
	pipeline = nullptr;
}

void WaylandMouseFeed::HandleEnter(void* data, wl_pointer*, uint32_t, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (surface != self->surface) {
		return;
	}
	self->position[0] = wl_fixed_to_double(sx);
	self->position[1] = wl_fixed_to_double(sy);
	self->SetFocus(true);
	if (self->pipeline) {
		self->pipeline->FeedCursorPosition(self->device, VirtualKey::Cursor,
			static_cast<float>(self->position[0]),
			static_cast<float>(self->position[1]),
			0.0f);
	}
}

void WaylandMouseFeed::HandleLeave(void* data, wl_pointer*, uint32_t, wl_surface* surface) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (surface != self->surface) {
		return;
	}
	self->SetFocus(false);
}

void WaylandMouseFeed::HandleMotion(void* data, wl_pointer*, uint32_t, wl_fixed_t sx, wl_fixed_t sy) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (!self->focused || !self->pipeline) {
		return;
	}

	const double newX = wl_fixed_to_double(sx);
	const double newY = wl_fixed_to_double(sy);
	const float deltaX = static_cast<float>(newX - self->position[0]);
	const float deltaY = static_cast<float>(newY - self->position[1]);
	self->position[0] = newX;
	self->position[1] = newY;

	self->pipeline->FeedCursorPosition(self->device, VirtualKey::Cursor,
		static_cast<float>(self->position[0]),
		static_cast<float>(self->position[1]),
		0.0f);

	if (deltaX != 0.0f || deltaY != 0.0f) {
		self->pipeline->FeedCursorDelta(self->device, VirtualKey::Cursor, deltaX, deltaY, 0.0f);
	}
}

void WaylandMouseFeed::HandleButton(void* data, wl_pointer*, uint32_t, uint32_t, uint32_t button, uint32_t state) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (!self->focused || !self->pipeline) {
		return;
	}
	const auto vk = ButtonToVirtualKey(button);
	if (vk == VirtualKey::None) {
		return;
	}
	self->pipeline->FeedKeyEvent(self->device, vk,
		(state == WL_POINTER_BUTTON_STATE_PRESSED) ? VirtualKeyState::Down : VirtualKeyState::Up);
}

void WaylandMouseFeed::HandleAxis(void* data, wl_pointer*, uint32_t, uint32_t axis, wl_fixed_t value) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (!self->focused || axis > 1) {
		return;
	}
	self->axisAcc[axis] += wl_fixed_to_double(value);
	self->axisDirty[axis] = true;
}

void WaylandMouseFeed::HandleAxisSource(void* data, wl_pointer*, uint32_t axis_source) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	self->axisSource = axis_source;
}

void WaylandMouseFeed::HandleAxisStop(void* data, wl_pointer*, uint32_t, uint32_t axis) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (axis > 1) {
		return;
	}
	self->axisAcc[axis] = 0.0;
	self->axisDiscrete[axis] = 0;
	self->axisValue120[axis] = 0;
	self->axisDirty[axis] = false;
}

void WaylandMouseFeed::HandleAxisDiscrete(void* data, wl_pointer*, uint32_t axis, int32_t discrete) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (!self->focused || axis > 1) {
		return;
	}
	self->axisDiscrete[axis] += discrete;
	self->axisDirty[axis] = true;
}

void WaylandMouseFeed::HandleAxisValue120(void* data, wl_pointer*, uint32_t axis, int32_t value120) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (!self->focused || axis > 1) {
		return;
	}
	self->axisValue120[axis] += value120;
	self->axisDirty[axis] = true;
}

void WaylandMouseFeed::HandleAxisRelativeDirection(void* data, wl_pointer*, uint32_t axis, uint32_t) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (axis > 1) {
		return;
	}
	self->axisDirty[axis] = true;
}

void WaylandMouseFeed::HandleFrame(void* data, wl_pointer*) {
	auto* self = static_cast<WaylandMouseFeed*>(data);
	if (!self->focused || !self->pipeline) {
		self->ClearAxisState();
		return;
	}

	auto computeScroll = [](double value, int discrete, int value120) -> float {
		if (value120 != 0) {
			return static_cast<float>(value120);
		}
		if (discrete != 0) {
			return static_cast<float>(discrete * 120);
		}
		return static_cast<float>(value * 120.0);
	};

	if (self->axisDirty[WL_POINTER_AXIS_VERTICAL_SCROLL]) {
		const float scroll = computeScroll(self->axisAcc[WL_POINTER_AXIS_VERTICAL_SCROLL],
			self->axisDiscrete[WL_POINTER_AXIS_VERTICAL_SCROLL],
			self->axisValue120[WL_POINTER_AXIS_VERTICAL_SCROLL]);
		if (scroll != 0.0f) {
			self->pipeline->FeedCursorDelta(self->device, VirtualKey::Click_Wheel, scroll, 0.0f, 0.0f);
		}
	}

	if (self->axisDirty[WL_POINTER_AXIS_HORIZONTAL_SCROLL]) {
		const float scroll = computeScroll(self->axisAcc[WL_POINTER_AXIS_HORIZONTAL_SCROLL],
			self->axisDiscrete[WL_POINTER_AXIS_HORIZONTAL_SCROLL],
			self->axisValue120[WL_POINTER_AXIS_HORIZONTAL_SCROLL]);
		if (scroll != 0.0f) {
			self->pipeline->FeedCursorDelta(self->device, VirtualKey::Click_Wheel, 0.0f, scroll, 0.0f);
		}
	}

	self->ClearAxisState();
}

void WaylandMouseFeed::SetFocus(bool focusedState) {
	if (focused == focusedState) {
		return;
	}
	focused = focusedState;
	if (pipeline) {
		pipeline->FeedDeviceEvent(device, focused ? DeviceState::FocusGained : DeviceState::FocusLost);
	}
	if (!focused) {
		ClearAxisState();
	}
}

void WaylandMouseFeed::ClearAxisState() {
	axisAcc.fill(0.0);
	axisDiscrete.fill(0);
	axisValue120.fill(0);
	axisDirty.fill(false);
}

VirtualKey WaylandMouseFeed::ButtonToVirtualKey(uint32_t button) {
	switch (button) {
	case BTN_LEFT: return VirtualKey::Click_Left;
	case BTN_RIGHT: return VirtualKey::Click_Right;
	case BTN_MIDDLE: return VirtualKey::Click_Middle;
	case BTN_SIDE: return VirtualKey::Click_X4;
	case BTN_EXTRA: return VirtualKey::Click_X5;
	default: return VirtualKey::None;
	}
}

} // namespace weave::input::wayland
