#pragma once

#include <array>

#include <wayland-client.h>

#include "weave/input/system/InputPipeline.h"
#include "weave/input/system/VirtualDevices.h"
#include "weave/input/system/VirtualKeys.h"

namespace weave::input::wayland {

class WaylandMouseFeed {
public:
	WaylandMouseFeed() = default;
	~WaylandMouseFeed();

	WaylandMouseFeed(WaylandMouseFeed const&) = delete;
	WaylandMouseFeed& operator=(WaylandMouseFeed const&) = delete;
	WaylandMouseFeed(WaylandMouseFeed&&) = delete;
	WaylandMouseFeed& operator=(WaylandMouseFeed&&) = delete;

	bool Bind(wl_seat* seat, wl_surface* surface, InputPipeline* pipeline, VirtualDevice mouseDevice);
	void Unbind();

	void SetInputPipeline(InputPipeline* pipeline);
	void SetVirtualDevice(VirtualDevice device);

	static void HandleEnter(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
	static void HandleLeave(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface);
	static void HandleMotion(void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
	static void HandleButton(void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void HandleAxis(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
	static void HandleFrame(void* data, wl_pointer* pointer);
	static void HandleAxisSource(void* data, wl_pointer* pointer, uint32_t axis_source);
	static void HandleAxisStop(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis);
	static void HandleAxisDiscrete(void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete);
	static void HandleAxisValue120(void* data, wl_pointer* pointer, uint32_t axis, int32_t value120);
	static void HandleAxisRelativeDirection(void* data, wl_pointer* pointer, uint32_t axis, uint32_t direction);

private:
	void SetFocus(bool focused);
	void ClearAxisState();
	static VirtualKey ButtonToVirtualKey(uint32_t button);

	InputPipeline* pipeline = nullptr;
	VirtualDevice device = VirtualDevice::Mouse0;
	wl_surface* surface = nullptr;
	wl_pointer* pointer = nullptr;
	bool focused = false;

	double position[2] = {0.0, 0.0};
	std::array<double, 2> axisAcc{};
	std::array<int, 2> axisDiscrete{};
	std::array<int, 2> axisValue120{};
	std::array<bool, 2> axisDirty{};
	uint32_t axisSource = WL_POINTER_AXIS_SOURCE_WHEEL;
};

} // namespace weave::input::wayland
