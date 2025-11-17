#include "VirtualKeyPayloads.h"

using namespace weave::input;

bool KeyStatePayload::SetState(VirtualKeyState stt) {
	previous = current;
	if (current != VirtualKeyState::Up && stt == VirtualKeyState::Down) {
		current = VirtualKeyState::Hold;
		return true;
	} else if (current != stt) {
		current = stt;
		return true;
	}
	return false;
}

bool KeyPressurePayload::SetPressure(float newPressure) {
	if (newPressure == pressure) {
		return false;
	}

	pressureDelta = newPressure - pressure;
	pressure = newPressure;

	return true;
}

bool CursorPayload::SetPosition(float x, float y, float z) {
	if (x == position.x && y == position.y && z == position.z) {
		return false;
	}
	prevPosition = position;
	position.Set(x, y, z);

	positionDelta = position - prevPosition;

	return true;
}

bool CursorPayload::SetPositionDelta(float x, float y, float z) {
	if (x == 0.0f && y == 0.0f && z == 0.0f) {
		return false;
	}
	positionDelta.Set(x, y, z);

	prevPosition = position;
	position += positionDelta;

	return true;
}

bool CursorPayload::SetData(float px, float py, float pz, float mx, float my, float mz) {
	positionDelta.Set(mx, my, mz);

	prevPosition = position;
	position.Set(px, py, pz);

	return true;
}

