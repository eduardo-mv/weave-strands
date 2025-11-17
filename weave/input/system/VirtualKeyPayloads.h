/*
Weave Input Virtual Key Payloads
VirtualKeyPayloads.h

Defines the data payload structures associated with VirtualKey capabilities so they
can be shared across the entire input pipeline.
*/
#pragma once

#include <array>
#include "weave/system/math/VectorMath.h"
#include "VirtualKeys.h"

namespace weave::input {

struct KeyStatePayload {
	VirtualKeyState current{ VirtualKeyState::Up };
	VirtualKeyState previous{ VirtualKeyState::Up };

	bool SetState(VirtualKeyState stt);
};

struct KeyPressurePayload {
	// Pressure applied to the key. Relevant for pressure sensitive devices like triggers.
	float pressure = 0.0f;
	float pressureDelta = 0.0f;
	// The normalization factor for the pressure
	float pressureNormalization = 1.0f;

	bool SetPressure(float newPressure);
};

struct CursorPayload {
	Vector3 position;
	Vector3 prevPosition;
	Vector3 positionDelta;
	// Normalization values (typically window dimensions) for the cursor position
	Vector3 normalizationValues;

	// Sets positional data and infers movement data
	bool SetPosition(float x, float y, float z);
	// Sets movement data and infers positional data
	bool SetPositionDelta(float x, float y, float z);

	bool SetData(float px, float py, float pz, float mx, float my, float mz);
};

struct MidiNoteState {
	bool active{ false };
	uint8_t velocity{ 0 };
	uint8_t channel{ 0 };
	uint8_t aftertouch{ 0 };
};

struct MidiControlState {
	uint8_t value{ 0 };
	uint8_t channel{ 0 };
};

struct MidiPitchBendState {
	int value{ 0 }; // centered 0, positive/negative range
	uint8_t channel{ 0 };
};

struct MidiProgramState {
	uint8_t program{ 0 };
	uint8_t channel{ 0 };
};

struct MidiChannelPressureState {
	uint8_t pressure{ 0 };
	uint8_t channel{ 0 };
};

struct MidiPayload {
	static constexpr size_t kNoteCount = 128;
	static constexpr size_t kControlCount = 128;
	static constexpr size_t kChannelCount = 16;

	std::array<MidiNoteState, kNoteCount> notes{};
	std::array<MidiControlState, kControlCount> controls{};
	std::array<MidiPitchBendState, kChannelCount> pitch{};
	std::array<MidiProgramState, kChannelCount> programs{};
	std::array<MidiChannelPressureState, kChannelCount> channelPressure{};
};

} // namespace weave::input
