#include "SingleDevice.h"

using namespace weave::input;

void SingleDevice::InputEvent(InputEventData inputData, InputStateMap& inputState) {

	auto [device, i] = weave::input::GetDeviceAndOffset(inputData.device);
	auto [target, t] = weave::input::GetDeviceAndOffset(deviceTarget);
	
	if (target == VirtualDevice::None) {
		inputData.device = device;
	}
	else if (target == device) {
		inputData.device = deviceTarget;
	}

	NextProcessor(inputData, inputState);
}