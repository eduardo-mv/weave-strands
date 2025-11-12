#include "weave/system/math/Math.h"
#include "weave/input/system/VirtualKeys.h"
#include "weave/input/system/VirtualDevices.h"
#include <optional>
#include <variant>

namespace weave::input {

struct InputEventData {
	VirtualDevice device;
	VirtualKey key{ VirtualKey::None };

	std::variant<
		DeviceState,
		VirtualKeyState,
		std::pair<Vector3, Vector3>, //Cursor Position & Delta
		Vector3, //Cursor Position
		Vector3, //C. Delta
		Vector3, //C. Normalization
		std::pair<float, float> , //Pressure & Delta
		float, //Pressure
		float, //P. Delta
		float  //P. Normalization
	> eventData;
	
	InputEventData(VirtualDevice device)
		: device(device) {}

	InputEventData(VirtualDevice device, VirtualKey key)
		: device(device)
		, key(key) {}

	InputEventData(VirtualDevice device, DeviceState deviceState)
		: device(device)
		, eventData(deviceState) {
	}

	InputEventData(VirtualDevice device, VirtualKey key, VirtualKeyState keyState)
		: device(device)
		, key(key)
		, eventData(keyState) {
	}

	InputEventData(VirtualDevice device, VirtualKey key, Vector3 position, Vector3 delta)
		: device(device)
		, key(key)
		, eventData(std::pair{position, delta}) {
	}

	InputEventData(VirtualDevice device, VirtualKey key, float pressure, float delta)
		: device(device)
		, key(key)
		, eventData(std::pair{ pressure, delta }) {
	}

	template<typename ...Args>
	InputEventData(VirtualDevice device, VirtualKey key, Args &&... args)
		: device(device)
		, key(key)
		, eventData(args...) {
	}

	static InputEventData BuildCursorPosition(VirtualDevice device, VirtualKey key, Vector3 position) {
		return InputEventData (device, key, std::in_place_index<3>, position);
	}

	static InputEventData BuildCursorDelta(VirtualDevice device, VirtualKey key, Vector3 delta) {
		return InputEventData(device, key, std::in_place_index<4>, delta);
	}

	static InputEventData BuildCursorNormalization(VirtualDevice device, VirtualKey key, Vector3 norm) {
		return InputEventData(device, key, std::in_place_index<5>, norm);
	}

	static InputEventData BuildPressure(VirtualDevice device, VirtualKey key, float pressure) {
		return InputEventData(device, key, std::in_place_index<7>, pressure);
	}

	static InputEventData BuildPressureDelta(VirtualDevice device, VirtualKey key, float delta) {
		return InputEventData(device, key, std::in_place_index<8>, delta);
	}

	static InputEventData BuildPressureNormalization(VirtualDevice device, VirtualKey key, float norm) {
		return InputEventData(device, key, std::in_place_index<9>, norm);
	}

	bool HasDeviceState() const {
		return std::holds_alternative<DeviceState>(eventData);
	}

	bool HasVirtualKeyState() const {
		return std::holds_alternative<VirtualKeyState>(eventData);
	}

	bool HasCursorPositionAndDelta() const {
		return std::holds_alternative<std::pair<Vector3, Vector3>>(eventData);
	}

	bool HasCursorPosition() const {
		return eventData.index() == 3;
	}

	bool HasCursorDelta() const {
		return eventData.index() == 4;
	}

	bool HasCursorNormalization() const {
		return eventData.index() == 5;
	}

	bool HasCursorPressureAndDelta() const {
		return std::holds_alternative<std::pair<float, float>>(eventData);
	}

	bool HasPressure() const {
		return eventData.index() == 7;
	}

	bool HasPressureDelta() const {
		return eventData.index() == 8;
	}

	bool HasPressureNormalization() const {
		return eventData.index() == 9;
	}

	auto GetDeviceState() const {
		return std::get<DeviceState>(eventData);
	}

	auto GetVirtualKeyState() const {
		return std::get<VirtualKeyState>(eventData);
	}

	auto GetCursorPositionAndDelta() const {
		return std::get<std::pair<Vector3, Vector3>>(eventData);
	}

	auto GetCursorPosition() const {
		return std::get<3>(eventData);
	}

	auto GetCursorDelta() const {
		return std::get<4>(eventData);
	}

	auto GetCursorNormalization() const {
		return std::get<5>(eventData);
	}

	auto GetCursorPressureAndDelta() const {
		return std::get<std::pair<float, float>>(eventData);
	}

	auto GetPressure() const {
		return std::get<7>(eventData);
	}

	auto GetPressureDelta() const {
		return std::get<8>(eventData);
	}

	auto GetPressureNormalization() const {
		return std::get<9>(eventData);
	}

};

}