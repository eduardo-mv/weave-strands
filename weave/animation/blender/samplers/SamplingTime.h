#pragma once

#include <cstddef>

namespace weave::blender {

float GlobalToLocalTimeLoop(float clipLength, float globalTime, float rate, float offset);
float GlobalToLocalTime(float clipLength, float globalTime, float rate, float offset, float loops);
float NormalizedToLocalTimeLoop(float clipLength, float normalizedTime);
float NormalizedToLocalTime(float clipLength, float normalizedTime, float loops);

}

