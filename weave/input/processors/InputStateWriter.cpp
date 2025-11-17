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
	else if (inputData.HasMidiNote()) {
		auto event = inputData.GetMidiNote();
		inputState.WriteMidiNote(inputData.device, event.note, event.channel, event.velocity, event.pressed);
	}
	else if (inputData.HasMidiControl()) {
		auto event = inputData.GetMidiControl();
		inputState.WriteMidiControl(inputData.device, event.control, event.channel, event.value);
	}
	else if (inputData.HasMidiPitchBend()) {
		auto event = inputData.GetMidiPitchBend();
		inputState.WriteMidiPitchBend(inputData.device, event.channel, event.value);
	}
	else if (inputData.HasMidiProgram()) {
		auto event = inputData.GetMidiProgram();
		inputState.WriteMidiProgram(inputData.device, event.channel, event.program);
	}
	else if (inputData.HasMidiChannelPressure()) {
		auto event = inputData.GetMidiChannelPressure();
		inputState.WriteMidiChannelPressure(inputData.device, event.channel, event.pressure);
	}
	else if (inputData.HasMidiPolyPressure()) {
		auto event = inputData.GetMidiPolyPressure();
		inputState.WriteMidiPolyPressure(inputData.device, event.note, event.channel, event.pressure);
	}
	else if (inputData.HasPressureAndDelta()) {
		inputState.WriteKeyPressureAndDelta(inputData.device, inputData.key, inputData.GetPressure(), inputData.GetPressureDelta());
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
