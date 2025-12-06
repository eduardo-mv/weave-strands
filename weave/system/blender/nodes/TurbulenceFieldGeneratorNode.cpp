#include "TurbulenceFieldGeneratorNode.h"

#include <algorithm>

#include "weave/system/math/Interpolation.h"
#include "weave/system/math/Random.h"

namespace weave::blender {
namespace {
Vector3 RandomDirection() {
    Vector3 dir{
        weave::rng::uniformMirror<float>() + 0.0001f,
        weave::rng::uniformMirror<float>(),
        weave::rng::uniformMirror<float>()
    };
    return algebra::normalize(dir);
}
}

TurbulenceFieldGeneratorNode::TurbulenceFieldGeneratorNode() {
    this->input.SetDefaultValues(
        0.0f,  // min force
        1.0f,  // max force
        0.5f,  // frequency (seconds between updates)
        0.5f,  // variation blend factor
        1u     // field count
    );

    SetFields({});
}

void TurbulenceFieldGeneratorNode::SetFields(std::vector<FieldConfig> configs) {
    fields.clear();
    if (configs.empty()) {
        fields.push_back(RuntimeField{ RandomDirection(), -1.0f, -1.0f, -1.0f });
        return;
    }

    fields.reserve(configs.size());
    for (auto const& cfg : configs) {
        RuntimeField runtime{};
        runtime.direction = algebra::normalize(cfg.direction);
        runtime.minForce = cfg.minForce;
        runtime.maxForce = cfg.maxForce;
        runtime.variation = cfg.variation;
        fields.push_back(runtime);
    }
}

void TurbulenceFieldGeneratorNode::EnsureFieldCount(uint32_t desiredCount) {
    if (desiredCount == 0) {
        desiredCount = 1;
    }
    while (fields.size() < desiredCount) {
        fields.push_back(RuntimeField{ RandomDirection(), -1.0f, -1.0f, -1.0f });
    }
    if (fields.size() > desiredCount) {
        fields.resize(desiredCount);
    }
}

void TurbulenceFieldGeneratorNode::ExecuteNode() {
    const GraphTime& graphTime = this->uniform.template Ref<GraphTime>();
    const float deltaTime = graphTime.deltaSeconds;

    const float minForceInput = this->input.template Ref<MinForceInput>();
    const float maxForceInput = this->input.template Ref<MaxForceInput>();
    const float frequency = std::max(0.0f, this->input.template Ref<FrequencyInput>());
    const float variationInput = std::clamp(this->input.template Ref<VariationInput>(), 0.0f, 1.0f);
    const uint32_t fieldCount = this->input.template Ref<FieldCountInput>();

    EnsureFieldCount(fieldCount);

    nextUpdate -= deltaTime;
    if (frequency <= 0.0f || nextUpdate <= 0.0f) {
        nextUpdate = frequency;
        for (auto& field : fields) {
            Vector3 rng = RandomDirection();
            const float blend = field.variation >= 0.0f ? field.variation : variationInput;
            field.direction = algebra::normalize(interpolation::lerp(field.direction, rng, blend));
        }
    }

    auto& fieldList = this->output.template Ref<FieldsOutput>();
    fieldList.resize(fields.size());
    for (size_t i = 0; i < fields.size(); ++i) {
        const float minForce = fields[i].minForce >= 0.0f ? fields[i].minForce : minForceInput;
        const float maxForce = fields[i].maxForce >= 0.0f ? fields[i].maxForce : maxForceInput;
        const float magnitude = weave::rng::uniform<float>() * (maxForce - minForce) + minForce;
        fieldList[i] = TurbulenceField{ fields[i].direction, magnitude };
    }
}

} // namespace weave::blender
