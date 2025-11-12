/*
Input Processor:
Single VirtualDevice merger

Input processor node that convers all devices of a given device type into the provider virtual device.
e.g. Can merge all Gamepads into the given gamepad device.

If no device is specified, it will merge all device types individually (all keyboards into one, etc)

This can be used to allow a system with multiple keyboards, mice or gamepads to act as a single source of input.

*/
#pragma once

#include "weave/input/system/InputProcessor.h"

namespace weave::input {
class SingleDevice : public InputProcessor {
	VirtualDevice deviceTarget{ VirtualDevice::None };
public:
	SingleDevice() = default;
	SingleDevice(VirtualDevice deviceTarget) : deviceTarget(deviceTarget) {}
	void InputEvent(InputEventData, InputStateMap&) override;
};
}