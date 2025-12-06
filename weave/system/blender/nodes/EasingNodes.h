#pragma once

#include <cstddef>
#include <functional>

#include "weave/system/blender/Blender.h"

namespace weave::blender {

class EasingFunctionNode : public BlenderNode<
    In<float>,
    Out<float>> {
public:
    using Callback = std::function<float(float)>;

    enum InputIndex : size_t {
        TimeInput // Normalized time value
    };

    enum OutputIndex : size_t {
        ResultOutput // Curve-adjusted time value
    };

    EasingFunctionNode() = default;

    explicit EasingFunctionNode(Callback curve)
        : curve(std::move(curve)) { }

    void SetCurve(Callback fn) {
        curve = std::move(fn);
    }

    void ExecuteNode() override {
        const float t = this->input.template Ref<TimeInput>();
        this->output.template Ref<ResultOutput>() = curve(t);
    }

private:
    Callback curve = [](float t){ return t; };
};

} // namespace weave::blender

