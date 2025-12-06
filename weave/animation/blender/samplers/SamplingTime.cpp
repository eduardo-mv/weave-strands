#include "SamplingTime.h"

#include <algorithm>
#include <cmath>

namespace weave::blender {

float GlobalToLocalTimeLoop(float clipLength, float globalTime, float rate, float offset) {
	float globalRelativeTime = rate * globalTime + offset;
	return std::fmod(globalRelativeTime, clipLength);
}

float GlobalToLocalTime(float clipLength, float globalTime, float rate, float offset, float loops) {
	if (loops == 0.0f) {
		return GlobalToLocalTimeLoop(clipLength, globalTime, rate, offset);
	}

	float loopedLength = loops * clipLength;
	float globalRelativeClampedTime = std::clamp(rate * globalTime + offset, 0.0f, loopedLength);
	float wrappedTime = std::fmod(globalRelativeClampedTime, clipLength);
	return (globalRelativeClampedTime == loopedLength) ? clipLength : wrappedTime;
}

float NormalizedToLocalTime(float clipLength, float normalizedTime, float loops) {
	float loopedLength = loops * clipLength;
	return std::fmod(normalizedTime * loopedLength, clipLength);
}

float NormalizedToLocalTimeLoop(float clipLength, float normalizedTime) {
	return normalizedTime * clipLength;
}

}

