#include "InputStateWriter.h"

using namespace weave::input;

void InputStateWriter::InputEvent(InputEventData inputData, InputStateMap& inputState) {
	if (inputData.HasVirtualKeyState()) {
		inputState.WriteKeyState(inputData.device, inputData.key, inputData.GetVirtualKeyState());
	}
		
	else if (inputData.HasCursorPositionAndDelta()) {
		auto [position, delta] = inputData.GetCursorPositionAndDelta();
		inputState.WriteCursorPositionAndDelta(inputData.device, inputData.key, 
			position.x, 
			position.y, 
			position.z, 
			delta.x, 
			delta.y, 
			delta.z);
	}

	else if (inputData.HasCursorPosition()) {
		auto position = inputData.GetCursorPosition();
		inputState.WriteCursorPosition(inputData.device, inputData.key,
			position.x,
			position.y,
			position.z);
	}
	else if (inputData.HasCursorDelta()) {
		auto delta = inputData.GetCursorDelta();
		inputState.WriteCursorDelta(inputData.device, inputData.key,
			delta.x,
			delta.y,
			delta.z);
	}

	else if (inputData.HasDeviceState()) {
		auto deviceState = inputData.GetDeviceState();
		if (deviceState == DeviceState::FocusLost) {
			inputState.ResetDevice(inputData.device);
		}
		inputState.WriteDeviceState(inputData.device, deviceState);
	}

	else if (inputData.HasPressure()) {
		inputState.WriteKeyPressure(inputData.device, inputData.key, inputData.GetPressure());
	}
	else if (inputData.HasPressureDelta()) {
		inputState.WriteKeyPressureDelta(inputData.device, inputData.key, inputData.GetPressureDelta());
	}

	else if (inputData.HasCursorNormalization()) {
		auto normalization = inputData.GetCursorNormalization();
		inputState.WriteCursorNormalization(inputData.device, inputData.key, 
			normalization.x, 
			normalization.y, 
			normalization.z);
	}
	else if (inputData.HasPressureNormalization()) {
		inputState.WriteKeyPressureNormalization(inputData.device, inputData.key, inputData.GetPressureNormalization());
	}

	NextProcessor(inputData, inputState);
}
