/*
Weave Input State Map
InputStateMap.h

Processor that stores the state of keys
*/
#pragma once

#include "weave/system/math/VectorMath.h"
#include "weave/system/memory/RibbonAllocator.h"
#include "VirtualKeys.h"
#include "VirtualDevices.h"
#include <vector>
#include <array>
#include <optional>
#include <shared_mutex>
#include <span>
#include <mutex>

namespace weave::input {

struct KeyStateData {
	VirtualKeyState current{ VirtualKeyState::Up };
	VirtualKeyState previous{ VirtualKeyState::Up };

	bool SetState(VirtualKeyState stt);
};

struct KeyPressureData {
	//Pressure applied to the key. This is only relevant for pressure sensitive devices, like gamepads. The value is likely to be normalized.
	float pressure = 0.0f;
	float pressureDelta = 0.0f;
	//The normalization factor for the pressure
	float pressureNormalization = 1.0f;

	bool SetPressure(float newPressure);
};

struct CursorData {
	Vector3 position;
	Vector3 prevPosition;
	Vector3 positiontDelta;
	//The normalization values used for the cursor position, usually synched with the window size
	Vector3 normalizationValues;

	//Sets positional data and infers movement data
	bool SetPosition(float x, float y, float z);
	//Sets movement data and infers positional data
	bool SetPositionDelta(float x, float y, float z);
	
	bool SetData(float px, float py, float pz, float mx, float my, float mz);
};

struct KeyData {
	VirtualDevice device{ VirtualDevice::None };
	VirtualKey key{ VirtualKey::None };

	std::optional<KeyStateData> state;
	std::optional<KeyPressureData> pressure;
	std::optional<CursorData> cursor;

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

		std::array<KeyStateData, static_cast<uint32_t>(VirtualKey::_CountKeys)> keys;
		std::array<KeyPressureData, static_cast<uint32_t>(VirtualKey::_CountPressureKeys)> pressure;
		std::array<CursorData, static_cast<uint32_t>(VirtualKey::_CountCursors)> cursors;

		static constexpr const int32_t keyOffset = -static_cast<int32_t>(VirtualKey::_FirstKey);
		static constexpr const int32_t pressureOffset = -static_cast<int32_t>(VirtualKey::_FirstPressureKey);
		static constexpr const int32_t cursorOffset = -static_cast<int32_t>(VirtualKey::_FirstCursor);

		KeyStateData GetKeyStateData(VirtualKey key) const {
			return keys[static_cast<uint32_t>(key) + keyOffset];
		}

		KeyPressureData GetKeyPressureData(VirtualKey key) const {
			return pressure[static_cast<uint32_t>(key) + pressureOffset];
		}

		CursorData GetCursorData(VirtualKey key) const {
			return cursors[static_cast<uint32_t>(key) + cursorOffset];
		}

		void SetKeyStateData(VirtualKey key, KeyStateData const& data) {
			keys[static_cast<uint32_t>(key) + keyOffset] = data;
		}

		void SetKeyPressureData(VirtualKey key, KeyPressureData const& data) {
			pressure[static_cast<uint32_t>(key) + pressureOffset] = data;
		}

		void SetCursorData(VirtualKey key, CursorData const& data) {
			cursors[static_cast<uint32_t>(key) + cursorOffset] = data;
		}

		KeyStateData& KeyStateDataRef(VirtualKey key) {
			return keys[static_cast<uint32_t>(key) + keyOffset];
		}

		KeyPressureData& KeyPressureDataRef(VirtualKey key) {
			return pressure[static_cast<uint32_t>(key) + pressureOffset];
		}

		CursorData& CursorDataRef(VirtualKey key) {
			return cursors[static_cast<uint32_t>(key) + cursorOffset];
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
	
	void WriteKeyData(VirtualDevice device, VirtualKey key, KeyStateData const& data);
	void WriteKeyData(VirtualDevice device, VirtualKey key, KeyPressureData const& data);
	void WriteKeyData(VirtualDevice device, VirtualKey key, CursorData const& data);

	void WriteKeyState(VirtualDevice device, VirtualKey key, VirtualKeyState state);
	
	void WriteKeyPressure(VirtualDevice device, VirtualKey key, float pressure);
	void WriteKeyPressureDelta(VirtualDevice device, VirtualKey key, float pressure);
	void WriteKeyPressureNormalization(VirtualDevice device, VirtualKey key, float x);
	
	void WriteCursorPosition(VirtualDevice device, VirtualKey key, float x, float y, float z);
	void WriteCursorDelta(VirtualDevice device, VirtualKey key, float x, float y, float z);
	void WriteCursorPositionAndDelta(VirtualDevice device, VirtualKey key, float px, float py, float pz, float dx, float dy, float dz);
	void WriteCursorNormalization(VirtualDevice device, VirtualKey key, float x, float y, float z);

	void ResetDevice(VirtualDevice device);
	
	InputMessage CreateMessage(uint64_t id, size_t reportCount);
	void WriteMessage(InputMessage message);

	std::vector<InputMessage> FlushMessages();


};

}
