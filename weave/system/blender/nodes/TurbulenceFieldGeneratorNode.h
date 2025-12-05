#pragma once

#include <cstdint>
#include <vector>

#include "weave/system/blender/BlenderNode.h"
#include "weave/system/blender/GraphTime.h"
#include "weave/system/blender/nodes/TurbulenceFieldTypes.h"
#include "weave/system/math/VectorMath.h"

namespace weave::blender {

class TurbulenceFieldGeneratorNode : public BlenderNode<
    Uniform<GraphTime>,
    In<float, float, float, float, uint32_t>,
    Out<TurbulenceFieldList>> {
public:
    TurbulenceFieldGeneratorNode();

    void ExecuteNode() override;

    struct FieldConfig {
        Vector3 direction{ 0.0f, 0.0f, 1.0f };
        float minForce{ -1.0f };
        float maxForce{ -1.0f };
        float variation{ -1.0f };
    };

    void SetFields(std::vector<FieldConfig> configs);

    enum InputIndex : size_t {
        MinForceInput,
        MaxForceInput,
        FrequencyInput,
        VariationInput,
        FieldCountInput
    };

private:
    struct RuntimeField {
        Vector3 direction;
        float minForce{ -1.0f };
        float maxForce{ -1.0f };
        float variation{ -1.0f };
    };

    void EnsureFieldCount(uint32_t desiredCount);

    std::vector<RuntimeField> fields;
    float nextUpdate{ 0.0f };
};

} // namespace weave::blender
