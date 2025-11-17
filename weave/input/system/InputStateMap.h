/*
Weave Input State Map
InputStateMap.h

Processor that stores the state of keys
*/
#pragma once

#include "weave/system/memory/RibbonAllocator.h"
#include "VirtualKeys.h"
#include "VirtualKeyPayloads.h"
#include "VirtualDevices.h"
#include <vector>
#include <array>
#include <optional>
#include <shared_mutex>
#include <span>
#include <mutex>

namespace weave::input {

struct KeyData {
	VirtualDevice device{ VirtualDevice::None };
	VirtualKey key{ VirtualKey::None };

	std::optional<KeyStatePayload> state;
	std::optional<KeyPressurePayload> pressure;
	std::optional<CursorPayload> cursor;
	std::optional<MidiPayload> midi;

	KeyData() = default;
	KeyData(VirtualDevice device, VirtualKey key) : device(device), key(key) {}
};

struct InputMessage {
	uint64_t messageId{};
	std::span<KeyData> report;
};

class InputStateMap {
	struct DeviceKeyData {
		VirtualDevice device;
		DeviceState state{ DeviceState::Disconnected };
		DeviceState previousState{ DeviceState::Disconnected };

		std::array<KeyStatePayload, kVirtualKeyMetadata.stateKeys.size()> keys{};
		std::array<KeyPressurePayload, kVirtualKeyMetadata.pressureKeys.size()> pressure{};
		std::array<CursorPayload, kVirtualKeyMetadata.cursorKeys.size()> cursors{};
		std::optional<MidiPayload> midi{};

		KeyStatePayload GetKeyStateData(VirtualKey key) const {
			return keys[static_cast<size_t>(GetStateKeyIndex(key))];
		}

		KeyPressurePayload GetKeyPressureData(VirtualKey key) const {
			return pressure[static_cast<size_t>(GetPressureKeyIndex(key))];
		}

		CursorPayload GetCursorData(VirtualKey key) const {
			return cursors[static_cast<size_t>(GetCursorKeyIndex(key))];
		}

		void SetKeyStateData(VirtualKey key, KeyStatePayload const& data) {
			keys[static_cast<size_t>(GetStateKeyIndex(key))] = data;
		}

		void SetKeyPressureData(VirtualKey key, KeyPressurePayload const& data) {
			pressure[static_cast<size_t>(GetPressureKeyIndex(key))] = data;
		}

		void SetCursorData(VirtualKey key, CursorPayload const& data) {
			cursors[static_cast<size_t>(GetCursorKeyIndex(key))] = data;
		}

		KeyStatePayload& KeyStateDataRef(VirtualKey key) {
			return keys[static_cast<size_t>(GetStateKeyIndex(key))];
		}

		KeyPressurePayload& KeyPressureDataRef(VirtualKey key) {
			return pressure[static_cast<size_t>(GetPressureKeyIndex(key))];
		}

		CursorPayload& CursorDataRef(VirtualKey key) {
			return cursors[static_cast<size_t>(GetCursorKeyIndex(key))];
		}
	};

	void ResetDeviceState(DeviceKeyData& data);
	
	std::array<DeviceKeyData, static_cast<uint32_t>(VirtualDevice::_Count)> deviceData;
	std::vector<InputMessage> messages, messagesSwap;

	static constexpr size_t ribbonBuffer = 2 * 1024 * 1024; //2MB
	RibbonAllocator ribbonAllocator{ ribbonBuffer };

	mutable std::array<std::shared_mutex, static_cast<uint32_t>(VirtualDevice::_Count)> deviceDataMutex;
	std::mutex messagesMutex;
	std::mutex messagesSwapMutex;
	mutable std::mutex ribbonMutex;

public:

	InputStateMap();

	DeviceState QueryDeviceState(VirtualDevice device) const;
	KeyData QueryKeyData(VirtualDevice device, VirtualKey key) const;

	void WriteDeviceState(VirtualDevice device, DeviceState state);
	
	void WriteKeyData(VirtualDevice device, VirtualKey key, KeyStatePayload const& data);
	void WriteKeyData(VirtualDevice device, VirtualKey key, KeyPressurePayload const& data);
	void WriteKeyData(VirtualDevice device, VirtualKey key, CursorPayload const& data);

	void WriteKeyState(VirtualDevice device, VirtualKey key, VirtualKeyState state);
	
	void WriteKeyPressure(VirtualDevice device, VirtualKey key, float pressure);
	void WriteKeyPressureDelta(VirtualDevice device, VirtualKey key, float pressure);
	void WriteKeyPressureAndDelta(VirtualDevice device, VirtualKey key, float pressure, float delta);
	void WriteKeyPressureNormalization(VirtualDevice device, VirtualKey key, float x);
	
	void WriteCursorPosition(VirtualDevice device, VirtualKey key, float x, float y, float z);
	void WriteCursorDelta(VirtualDevice device, VirtualKey key, float x, float y, float z);
	void WriteCursorPositionAndDelta(VirtualDevice device, VirtualKey key, float px, float py, float pz, float dx, float dy, float dz);
	void WriteCursorNormalization(VirtualDevice device, VirtualKey key, float x, float y, float z);

	void WriteMidiNote(VirtualDevice device, uint8_t note, uint8_t channel, uint8_t velocity, bool pressed);
	void WriteMidiControl(VirtualDevice device, uint8_t control, uint8_t channel, uint8_t value);
	void WriteMidiPitchBend(VirtualDevice device, uint8_t channel, int value);
	void WriteMidiProgram(VirtualDevice device, uint8_t channel, uint8_t program);
	void WriteMidiChannelPressure(VirtualDevice device, uint8_t channel, uint8_t pressure);
	void WriteMidiPolyPressure(VirtualDevice device, uint8_t note, uint8_t channel, uint8_t pressure);

	void ResetDevice(VirtualDevice device);
	
	InputMessage CreateMessage(uint64_t id, size_t reportCount);
	void WriteMessage(InputMessage message);

	std::vector<InputMessage> FlushMessages();


};

}
