#include "InputStateMap.h"

using namespace weave::input;

InputStateMap::InputStateMap() {
	uint32_t deviceIndex = 0;
	for (auto& deviceKeyData : deviceData) {
		deviceKeyData.device = weave::input::OffsetDevice(VirtualDevice::_First, deviceIndex++);
		ResetDeviceState(deviceKeyData);
	}
}

DeviceState InputStateMap::QueryDeviceState(VirtualDevice device) const {
	auto deviceIndex = static_cast<uint32_t>(device);
	std::shared_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex < deviceData.size()) {
		auto& deviceKeyData = deviceData[deviceIndex];
		return deviceKeyData.state;
	}

	return DeviceState::Disconnected;
}

KeyData InputStateMap::QueryKeyData(VirtualDevice device, VirtualKey key) const {
	auto deviceIndex = static_cast<uint32_t>(device);
	std::shared_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	KeyData keyData{ device , key };

	if (deviceIndex < deviceData.size() && key != VirtualKey::None) {
		auto& deviceKeyData = deviceData[deviceIndex];
		if (SupportsCursor(key)) {
			keyData.cursor = deviceKeyData.GetCursorData(key);
		}
		if (SupportsPressure(key)) {
			keyData.pressure = deviceKeyData.GetKeyPressureData(key);
		}
		if (SupportsState(key)) {
			keyData.state = deviceKeyData.GetKeyStateData(key);
		}
		if (SupportsMidi(key) && deviceKeyData.midi) {
			keyData.midi = deviceKeyData.midi;
		}
	}

	return keyData;
}

void InputStateMap::WriteDeviceState(VirtualDevice device, DeviceState state) {
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex >= deviceData.size()) {
		return;
	}

	auto& deviceKeyData = deviceData[deviceIndex];

	if (deviceKeyData.state == state) {
		return;
	}

	if (state == DeviceState::FocusGained) {
		if (deviceKeyData.state == DeviceState::FocusLost) {
			state = deviceKeyData.previousState;
		}
	}

	deviceKeyData.previousState = deviceKeyData.state;
	deviceKeyData.state = state;
}

void InputStateMap::WriteKeyData(VirtualDevice device, VirtualKey key, KeyStatePayload const& data) {
	if (!SupportsState(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex < deviceData.size()) {
		auto& deviceKeyData = deviceData[deviceIndex];
		deviceKeyData.SetKeyStateData(key, data);
	}
}

void InputStateMap::WriteKeyData(VirtualDevice device, VirtualKey key, KeyPressurePayload const& data) {
	if (!SupportsPressure(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex < deviceData.size()) {
		auto& deviceKeyData = deviceData[deviceIndex];
		deviceKeyData.SetKeyPressureData(key, data);
	}
}

void InputStateMap::WriteKeyData(VirtualDevice device, VirtualKey key, CursorPayload const& data) {
	if (!SupportsCursor(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex < deviceData.size()) {
		auto& deviceKeyData = deviceData[deviceIndex];
		deviceKeyData.SetCursorData(key, data);
	}
}

void InputStateMap::WriteKeyState(VirtualDevice device, VirtualKey key, VirtualKeyState state) {
	if (!SupportsState(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.KeyStateDataRef(key).SetState(state);
}

void InputStateMap::WriteKeyPressure(VirtualDevice device, VirtualKey key, float pressure) {
	if (!SupportsPressure(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.KeyPressureDataRef(key).pressure = pressure;
}

void InputStateMap::WriteKeyPressureDelta(VirtualDevice device, VirtualKey key, float pressure) {
	if (!SupportsPressure(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.KeyPressureDataRef(key).pressureDelta = pressure;
}

void InputStateMap::WriteKeyPressureAndDelta(VirtualDevice device, VirtualKey key, float pressure, float delta) {
	if (!SupportsPressure(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	auto& payload = deviceKeyData.KeyPressureDataRef(key);
	payload.pressure = pressure;
	payload.pressureDelta = delta;
}

void InputStateMap::WriteKeyPressureNormalization(VirtualDevice device, VirtualKey key, float x) {
	if (!SupportsPressure(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.KeyPressureDataRef(key).pressureNormalization = x;
}

void InputStateMap::WriteCursorPosition(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	if (!SupportsCursor(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.CursorDataRef(key).SetPosition(x, y, z);
}

void InputStateMap::WriteCursorDelta(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	if (!SupportsCursor(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.CursorDataRef(key).SetPositionDelta(x, y, z);
}

void InputStateMap::WriteCursorPositionAndDelta(VirtualDevice device, VirtualKey key, float px, float py, float pz, float dx, float dy, float dz) {
	if (!SupportsCursor(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.CursorDataRef(key).SetData(px, py, pz, dx, dy, dz);
}

void InputStateMap::WriteCursorNormalization(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	if (!SupportsCursor(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.CursorDataRef(key).normalizationValues.Set(x, y, z);
}

void InputStateMap::WriteMidiNote(VirtualDevice device, uint8_t note, uint8_t channel, uint8_t velocity, bool pressed) {
	if (!IsMidiDevice(device)) {
		return;
	}
	if (note >= MidiPayload::kNoteCount) {
		return;
	}
	channel &= 0x0F;
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex >= deviceData.size()) {
		return;
	}
	auto& deviceKeyData = deviceData[deviceIndex];
	if (!deviceKeyData.midi) {
		deviceKeyData.midi.emplace();
	}
	auto& entry = deviceKeyData.midi->notes[note];
	entry.active = pressed;
	entry.velocity = velocity;
	entry.channel = channel;
	if (!pressed) {
		entry.aftertouch = 0;
	}
}

void InputStateMap::WriteMidiControl(VirtualDevice device, uint8_t control, uint8_t channel, uint8_t value) {
	if (!IsMidiDevice(device)) {
		return;
	}
	if (control >= MidiPayload::kControlCount) {
		return;
	}
	channel &= 0x0F;
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex >= deviceData.size()) {
		return;
	}
	auto& deviceKeyData = deviceData[deviceIndex];
	if (!deviceKeyData.midi) {
		deviceKeyData.midi.emplace();
	}
	auto& entry = deviceKeyData.midi->controls[control];
	entry.value = value;
	entry.channel = channel;
}

void InputStateMap::WriteMidiPitchBend(VirtualDevice device, uint8_t channel, int value) {
	if (!IsMidiDevice(device)) {
		return;
	}
	channel &= 0x0F;
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex >= deviceData.size()) {
		return;
	}
	auto& deviceKeyData = deviceData[deviceIndex];
	if (!deviceKeyData.midi) {
		deviceKeyData.midi.emplace();
	}
	auto& entry = deviceKeyData.midi->pitch[channel];
	entry.value = value;
	entry.channel = channel;
}

void InputStateMap::WriteMidiProgram(VirtualDevice device, uint8_t channel, uint8_t program) {
	if (!IsMidiDevice(device)) {
		return;
	}
	channel &= 0x0F;
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex >= deviceData.size()) {
		return;
	}
	auto& deviceKeyData = deviceData[deviceIndex];
	if (!deviceKeyData.midi) {
		deviceKeyData.midi.emplace();
	}
	auto& entry = deviceKeyData.midi->programs[channel];
	entry.program = program;
	entry.channel = channel;
}

void InputStateMap::WriteMidiChannelPressure(VirtualDevice device, uint8_t channel, uint8_t pressure) {
	if (!IsMidiDevice(device)) {
		return;
	}
	channel &= 0x0F;
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex >= deviceData.size()) {
		return;
	}
	auto& deviceKeyData = deviceData[deviceIndex];
	if (!deviceKeyData.midi) {
		deviceKeyData.midi.emplace();
	}
	auto& entry = deviceKeyData.midi->channelPressure[channel];
	entry.pressure = pressure;
	entry.channel = channel;
}

void InputStateMap::WriteMidiPolyPressure(VirtualDevice device, uint8_t note, uint8_t channel, uint8_t pressure) {
	if (!IsMidiDevice(device)) {
		return;
	}
	if (note >= MidiPayload::kNoteCount) {
		return;
	}
	channel &= 0x0F;
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex >= deviceData.size()) {
		return;
	}
	auto& deviceKeyData = deviceData[deviceIndex];
	if (!deviceKeyData.midi) {
		deviceKeyData.midi.emplace();
	}
	auto& entry = deviceKeyData.midi->notes[note];
	entry.aftertouch = pressure;
	entry.channel = channel;
}

InputMessage InputStateMap::CreateMessage(uint64_t id, size_t reportCount) {
	std::lock_guard<std::mutex> lock(ribbonMutex);
	return InputMessage{ id, { ribbonAllocator.AllocateMany<KeyData>(reportCount), reportCount } };
}

void InputStateMap::WriteMessage(InputMessage message) {
	std::unique_lock lock(messagesMutex);
	messages.emplace_back(std::move(message));
}

std::vector<InputMessage> InputStateMap::FlushMessages() {
	std::unique_lock lockSwap(messagesSwapMutex);
	{
		std::unique_lock lockMessages(messagesMutex);
		std::swap(messages, messagesSwap);
		messages.clear();
	}
	return messagesSwap;
}

void InputStateMap::ResetDevice(VirtualDevice device) {
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex >= deviceData.size()) {
		return;
	}
	ResetDeviceState(deviceData[deviceIndex]);
}

void InputStateMap::ResetDeviceState(DeviceKeyData& data) {
	for (auto& keyState : data.keys) {
		keyState.current = VirtualKeyState::Up;
		keyState.previous = VirtualKeyState::Up;
	}
	for (auto& pressureState : data.pressure) {
		pressureState.pressure = 0.0f;
		pressureState.pressureDelta = 0.0f;
		pressureState.pressureNormalization = 1.0f;
	}
	for (auto& cursor : data.cursors) {
		cursor.position.Set(0.0f, 0.0f, 0.0f);
		cursor.prevPosition.Set(0.0f, 0.0f, 0.0f);
		cursor.positionDelta.Set(0.0f, 0.0f, 0.0f);
		cursor.normalizationValues.Set(0.0f, 0.0f, 0.0f);
	}
		if (IsMidiDevice(data.device)) {
			if (!data.midi) {
				data.midi.emplace();
			}
			for (auto& note : data.midi->notes) {
				note = MidiNoteState{};
			}
			for (auto& entry : data.midi->channelPressure) {
				entry = MidiChannelPressureState{};
			}
		} else {
			data.midi.reset();
		}
	data.state = DeviceState::Disconnected;
	data.previousState = DeviceState::Disconnected;
}
