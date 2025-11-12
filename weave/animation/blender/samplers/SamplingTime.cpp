#include "SamplingTime.h"
#include <cmath>

namespace weave::blender {

	float GlobalToLocalTimeLoop(float clipLength, float globalStart, float globalNow, float rate, float offset) {
		float globalRelativeTime = rate * (globalNow - globalStart) + offset;
		return std::fmod(globalRelativeTime, clipLength);
	}

	float GlobalToLocalTime(float clipLength, float globalStart, float globalNow, float rate, float offset, float loops) {
		if (loops == 0.0f) {
			return GlobalToLocalTimeLoop(clipLength, globalStart, globalNow, rate, offset);
		}

		// Adjust clip length based on loops and clamp the relative global time to that length
		float loopedLength = loops * clipLength;
		float globalRelativeClampedTime = std::clamp(rate * (globalNow - globalStart) + offset, 0.0f, loopedLength);

		// Time wrap around the non looped length
		float wrappedTime = std::fmod(globalRelativeClampedTime, clipLength);
		
		// Ensure that the value does not wrap to 0 at the end of the last loop
		return (globalRelativeClampedTime == loopedLength) ? clipLength : wrappedTime;
	}

	float NormalizedToLocalTime(float clipLength, float normalizedTime, float loops) {
		// Adjust clip length based on loops
		float loopedLength = loops * clipLength;
		//Time wrap around the non looped length
		return std::fmod(normalizedTime * loopedLength, clipLength);
	}

	float NormalizedToLocalTimeLoop(float clipLength, float normalizedTime) {
		return normalizedTime * clipLength;
	}
}