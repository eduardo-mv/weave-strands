#pragma once

#include <vector>

#include "weave/system/math/VectorMath.h"

namespace weave::blender {

struct TurbulenceField {
    Vector3 direction{};
    float forceMagnitude{ 0.0f };
};

using TurbulenceFieldList = std::vector<TurbulenceField>;

} // namespace weave::blender
