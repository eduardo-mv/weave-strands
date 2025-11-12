#pragma once

#include "weave/system/time/Clock.h"
#include <cstdint>

namespace weave::blender {

struct SamplingTime {
	weave::time::Clock clock;

	float globalTimeStart{};
	float globalTimeNow{};
	float delta{};
};

//Time conversions
float GlobalToLocalTimeLoop(float clipLength, float globalStart, float globalNow, float rate, float offset);
float GlobalToLocalTime(float clipLength, float globalStart, float globalNow, float rate, float offset, float loops);
float NormalizedToLocalTimeLoop(float clipLength, float normalizedTime);
float NormalizedToLocalTime(float clipLength, float normalizedTime, float loops);

}