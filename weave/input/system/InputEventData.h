#pragma once

#include "weave/input/system/VirtualKeyPayloads.h"
#include "weave/input/system/VirtualKeys.h"
#include "weave/input/system/VirtualDevices.h"
#include <variant>
#include <cstdint>

namespace weave::input {

struct DeviceStateEvent {
	DeviceState state;
};

struct KeyStateEvent {
	VirtualKeyState state;
};

struct CursorPositionEvent {
    Vector3 position;
};

struct CursorDeltaEvent {
    Vector3 delta;
};

struct CursorPosDeltaEvent {
    Vector3 position;
    Vector3 delta;
};

struct CursorNormalizationEvent {
    Vector3 normalization;
};

struct PressureEvent {
    float pressure;
};

struct PressureDeltaEvent {
    float delta;
};

struct PressureAndDeltaEvent {
	float pressure;
	float delta;	
};

struct PressureNormalizationEvent {
    float normalization;
};

struct MidiNoteEvent {
	uint8_t note;
	uint8_t velocity;
	uint8_t channel;
	bool pressed;
};

struct MidiControlEvent {
	uint8_t control;
	uint8_t value;
	uint8_t channel;
};

struct MidiPitchBendEvent {
	uint8_t channel;
	int value;
};

struct MidiProgramEvent {
	uint8_t channel;
	uint8_t program;
};

struct MidiChannelPressureEvent {
	uint8_t channel;
	uint8_t pressure;
};

struct MidiPolyPressureEvent {
	uint8_t note;
	uint8_t channel;
	uint8_t pressure;
};

struct InputEventData {
	VirtualDevice device;
	VirtualKey key{ VirtualKey::None };

	std::variant<
		std::monostate,
		DeviceStateEvent,
		KeyStateEvent,
		CursorPositionEvent,
		CursorDeltaEvent,
		CursorPosDeltaEvent,
		CursorNormalizationEvent,
		PressureEvent,
		PressureDeltaEvent,
		PressureAndDeltaEvent,
		PressureNormalizationEvent,
		MidiNoteEvent,
		MidiControlEvent,
		MidiPitchBendEvent,
		MidiProgramEvent,
		MidiChannelPressureEvent,
		MidiPolyPressureEvent
	> eventData;

	template<typename EventType>
	InputEventData(VirtualDevice device, VirtualKey key, EventType&& event) 
		: device(device)
		, key(key)
		, eventData(event)
	{}

	InputEventData(VirtualDevice device, DeviceState state)
		: device(device)
		, eventData(DeviceStateEvent{ state })
	{}
	
	bool HasDeviceState() const {
		return std::holds_alternative<DeviceStateEvent>(eventData);
	}

	bool HasVirtualKeyState() const {
		return std::holds_alternative<KeyStateEvent>(eventData);
	}

	bool HasCursorPositionAndDelta() const {
		return std::holds_alternative<CursorPosDeltaEvent>(eventData);
	}

	bool HasCursorPosition() const {
		return std::holds_alternative<CursorPositionEvent>(eventData) ||
		       std::holds_alternative<CursorPosDeltaEvent>(eventData);
	}

	bool HasCursorDelta() const {
		return std::holds_alternative<CursorDeltaEvent>(eventData) ||
		       std::holds_alternative<CursorPosDeltaEvent>(eventData);
	}

	bool HasCursorNormalization() const {
		return std::holds_alternative<CursorNormalizationEvent>(eventData);
	}

	bool HasPressure() const {
		return std::holds_alternative<PressureEvent>(eventData) ||
		       std::holds_alternative<PressureAndDeltaEvent>(eventData);
	}

	bool HasPressureDelta() const {
		return std::holds_alternative<PressureDeltaEvent>(eventData) ||
		       std::holds_alternative<PressureAndDeltaEvent>(eventData);
	}

	bool HasPressureAndDelta() const {
		return std::holds_alternative<PressureAndDeltaEvent>(eventData);
	}

	bool HasPressureNormalization() const {
		return std::holds_alternative<PressureNormalizationEvent>(eventData);
	}

	bool HasMidiNote() const {
		return std::holds_alternative<MidiNoteEvent>(eventData);
	}

	bool HasMidiControl() const {
		return std::holds_alternative<MidiControlEvent>(eventData);
	}

	bool HasMidiPitchBend() const {
		return std::holds_alternative<MidiPitchBendEvent>(eventData);
	}

	bool HasMidiProgram() const {
		return std::holds_alternative<MidiProgramEvent>(eventData);
	}
	bool HasMidiChannelPressure() const {
		return std::holds_alternative<MidiChannelPressureEvent>(eventData);
	}

	bool HasMidiPolyPressure() const {
		return std::holds_alternative<MidiPolyPressureEvent>(eventData);
	}

	DeviceState GetDeviceState() const {
		return std::get<DeviceStateEvent>(eventData).state;
	}

	VirtualKeyState GetVirtualKeyState() const {
		return std::get<KeyStateEvent>(eventData).state;
	}

	std::pair<Vector3, Vector3> GetCursorPositionAndDelta() const {
		auto const& cursor = std::get<CursorPosDeltaEvent>(eventData);
		return { cursor.position, cursor.delta };
	}

	Vector3 GetCursorPosition() const {
		if (auto pos = std::get_if<CursorPositionEvent>(&eventData)) {
			return pos->position;
		}
		return std::get<CursorPosDeltaEvent>(eventData).position;
	}

	Vector3 GetCursorDelta() const {
		if (auto delta = std::get_if<CursorDeltaEvent>(&eventData)) {
			return delta->delta;
		}
		return std::get<CursorPosDeltaEvent>(eventData).delta;
	}

	Vector3 GetCursorNormalization() const {
		return std::get<CursorNormalizationEvent>(eventData).normalization;
	}

	float GetPressure() const {
		if (auto pressure = std::get_if<PressureEvent>(&eventData)) {
			return pressure->pressure;
		}
		return std::get<PressureAndDeltaEvent>(eventData).pressure;
	}

	float GetPressureDelta() const {
		if (auto delta = std::get_if<PressureDeltaEvent>(&eventData)) {
			return delta->delta;
		}
		return std::get<PressureAndDeltaEvent>(eventData).delta;
	}

	PressureAndDeltaEvent GetPressureAndDelta() const {
		return std::get<PressureAndDeltaEvent>(eventData);
	}

	float GetPressureNormalization() const {
		return std::get<PressureNormalizationEvent>(eventData).normalization;
	}

	MidiNoteEvent GetMidiNote() const {
		return std::get<MidiNoteEvent>(eventData);
	}

	MidiControlEvent GetMidiControl() const {
		return std::get<MidiControlEvent>(eventData);
	}

	MidiPitchBendEvent GetMidiPitchBend() const {
		return std::get<MidiPitchBendEvent>(eventData);
	}

	MidiProgramEvent GetMidiProgram() const {
		return std::get<MidiProgramEvent>(eventData);
	}
	MidiChannelPressureEvent GetMidiChannelPressure() const {
		return std::get<MidiChannelPressureEvent>(eventData);
	}

	MidiPolyPressureEvent GetMidiPolyPressure() const {
		return std::get<MidiPolyPressureEvent>(eventData);
	}

};

}
