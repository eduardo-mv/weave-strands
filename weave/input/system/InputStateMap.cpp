#include "InputStateMap.h"
#include "InputStateMap.h"

using namespace weave::input;

InputStateMap::InputStateMap() {
	uint32_t deviceIndex = 0;
	for (auto& deviceKeyData : deviceData) {
		deviceKeyData.device = weave::input::OffsetDevice(VirtualDevice::_First, deviceIndex++);
		ResetDeviceState(deviceKeyData);
	}
}


bool KeyStateData::SetState(VirtualKeyState stt) {
	previous = current;
	if (current != VirtualKeyState::Up && stt == VirtualKeyState::Down) {
		current = VirtualKeyState::Hold;
		return true;
	}
	else if (current != stt) {
		current = stt;
		return true;
	}
	else
		return false;
}

bool KeyPressureData::SetPressure(float newPressure) {
	if (newPressure == pressure)
		return false;

	pressureDelta = newPressure - pressure;
	pressure = newPressure;

	return true;
}

bool CursorData::SetPosition(float x, float y, float z) {
	if (x == position.x && y == position.y && z == position.z)
		return false;
	prevPosition = position;
	position.Set(x, y, z);

	positiontDelta = position - prevPosition;

	return true;
}

bool CursorData::SetPositionDelta(float x, float y, float z) {
	if (x == 0.0f && y == 0.0f && z == 0.0f)
		return false;
	positiontDelta.Set(x, y, z);

	prevPosition = position;
	position += (positiontDelta);

	return true;
}

bool CursorData::SetData(float px, float py, float pz, float mx, float my, float mz) {
	positiontDelta.Set(mx, my, mz);

	prevPosition = position;
	position.Set(px, py, pz);

	return true;
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
	KeyData keyData{ device , key};

	if (deviceIndex < deviceData.size() && key != VirtualKey::None) {
		auto& deviceKeyData = deviceData[deviceIndex];
		if (weave::input::IsCursorKey(key)) {
			keyData.cursor = deviceKeyData.GetCursorData(key);
		}
		else {
			if (weave::input::IsPressureKey(key)) {
				keyData.pressure = deviceKeyData.GetKeyPressureData(key);
			}

			keyData.state = deviceKeyData.GetKeyStateData(key);
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

	if(deviceKeyData.state == state) {
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

void InputStateMap::WriteKeyData(VirtualDevice device, VirtualKey key, KeyStateData const& data) {
	if (weave::input::IsCursorKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex < deviceData.size()) {
		auto& deviceKeyData = deviceData[deviceIndex];
		deviceKeyData.SetKeyStateData(key, data);
	}
}

void InputStateMap::WriteKeyData(VirtualDevice device, VirtualKey key, KeyPressureData const& data) {
	if (!weave::input::IsPressureKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	if (deviceIndex < deviceData.size()) {
		auto& deviceKeyData = deviceData[deviceIndex];
		deviceKeyData.SetKeyPressureData(key, data);
	}
}

void InputStateMap::WriteKeyData(VirtualDevice device, VirtualKey key, CursorData const& data) {
	if (!weave::input::IsCursorKey(key)) {
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
	if (!weave::input::IsPushKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.KeyStateDataRef(key).SetState(state);
}

void InputStateMap::WriteKeyPressure(VirtualDevice device, VirtualKey key, float pressure) {
	if (!weave::input::IsPressureKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.KeyPressureDataRef(key).pressure = pressure;
}

void InputStateMap::WriteKeyPressureDelta(VirtualDevice device, VirtualKey key, float pressure) {
	if (!weave::input::IsPressureKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.KeyPressureDataRef(key).pressureDelta = pressure;
}

void InputStateMap::WriteKeyPressureNormalization(VirtualDevice device, VirtualKey key, float x) {
	if (!weave::input::IsPressureKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.KeyPressureDataRef(key).pressureNormalization = x;
}

void InputStateMap::WriteCursorPosition(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	if (!weave::input::IsCursorKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.CursorDataRef(key).SetPosition(x, y, z);
}

void InputStateMap::WriteCursorDelta(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	if (!weave::input::IsCursorKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.CursorDataRef(key).SetPositionDelta(x, y, z);
}

void InputStateMap::WriteCursorPositionAndDelta(VirtualDevice device, VirtualKey key, float px, float py, float pz, float dx, float dy, float dz) {
	if (!weave::input::IsCursorKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[static_cast<uint32_t>(device)];
	deviceKeyData.CursorDataRef(key).SetData(px, py, pz, dx, dy, dz);
}

void InputStateMap::WriteCursorNormalization(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	if (!weave::input::IsCursorKey(key)) {
		return;
	}
	auto deviceIndex = static_cast<uint32_t>(device);
	std::unique_lock<std::shared_mutex> lock(deviceDataMutex[deviceIndex]);
	auto& deviceKeyData = deviceData[deviceIndex];
	deviceKeyData.CursorDataRef(key).normalizationValues.Set(x, y, z);
}

InputMessage InputStateMap::CreateMessage(uint64_t id, size_t reportCount) {
	std::lock_guard<std::mutex> lock(ribbonMutex);
	return InputMessage{ id, { ribbonAllocator.AllocateMany<KeyData>(reportCount) , reportCount} };
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
	}
	for (auto& cursor : data.cursors) {
		cursor.position.Set(0.0f, 0.0f, 0.0f);
		cursor.prevPosition.Set(0.0f, 0.0f, 0.0f);
		cursor.positiontDelta.Set(0.0f, 0.0f, 0.0f);
	}
}


