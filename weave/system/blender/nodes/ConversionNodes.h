#pragma once

#include <cstddef>
#include <concepts>
#include <type_traits>
#include <utility>

#include "weave/system/blender/Blender.h"
#include "weave/system/math/Transform.h"
#include "weave/system/math/VectorMath.h"

namespace weave::blender {

/**
 * Converts a single input value to the desired output type using the Converter helper.
 */
template<typename FromType, typename ToType>
class ConvertNode : public BlenderNode<
    In<FromType>,
    Out<ToType>> {
public:
    enum InputIndex : size_t {
        ValueInput // Source value to convert
    };

    enum OutputIndex : size_t {
        ResultOutput // Converted value
    };

    ConvertNode() = default;

    explicit ConvertNode(FromType value) {
        this->input.SetDefaultValues(value);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = Convert<ToType>(this->input.template Ref<ValueInput>());
    }
private: 
    template<class To, class From>
    requires (std::is_arithmetic_v<From> && std::is_arithmetic_v<To>)
    To Convert(From const& value) {
        return static_cast<To>(value);
    }

    template<class To, class From>
    requires (std::is_arithmetic_v<From> && !std::is_arithmetic_v<To>)
    To Convert(From const& value) {
        return To{ static_cast<float>(value) };
    }

    template<class To, class From>
    requires std::same_as<To, IVector2>
    To Convert(From const& value) {
        return To(
            static_cast<int>(value.x),
            static_cast<int>(value.y));
    }

    template<class To, class From>
    requires std::same_as<To, IVector3>
    To Convert(From const& value) {
        return To(
            static_cast<int>(value.x),
            static_cast<int>(value.y),
            static_cast<int>(value.z));
    }

    template<class To, class From>
    requires std::same_as<To, IVector4>
    To Convert(From const& value) {
        return To(
            static_cast<int>(value.x),
            static_cast<int>(value.y),
            static_cast<int>(value.z),
            static_cast<int>(value.w));
    }

    // ---- unsigned int vectors ----
    template<class To, class From>
    requires std::same_as<To, UVector2>
    To Convert(From const& value) {
        return To(
            static_cast<unsigned int>(value.x),
            static_cast<unsigned int>(value.y));
    }

    template<class To, class From>
    requires std::same_as<To, UVector3>
    To Convert(From const& value) {
        return To(
            static_cast<unsigned int>(value.x),
            static_cast<unsigned int>(value.y),
            static_cast<unsigned int>(value.z));
    }

    template<class To, class From>
    requires std::same_as<To, UVector4>
    To Convert(From const& value) {
        return To(
            static_cast<unsigned int>(value.x),
            static_cast<unsigned int>(value.y),
            static_cast<unsigned int>(value.z),
            static_cast<unsigned int>(value.w));
    }
  
};

class ComposeVector2Node : public BlenderNode<
    In<float, float>,
    Out<Vector2>> {
public:
    enum InputIndex : size_t {
        XInput,
        YInput
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    ComposeVector2Node() = default;

    ComposeVector2Node(float x, float y) {
        this->input.SetDefaultValues(x, y);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = Vector2(
            this->input.template Ref<XInput>(),
            this->input.template Ref<YInput>());
    }
};

class ComposeVector3Node : public BlenderNode<
    In<float, float, float>,
    Out<Vector3>> {
public:
    enum InputIndex : size_t {
        XInput,
        YInput,
        ZInput
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    ComposeVector3Node() = default;

    ComposeVector3Node(float x, float y, float z) {
        this->input.SetDefaultValues(x, y, z);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = Vector3(
            this->input.template Ref<XInput>(),
            this->input.template Ref<YInput>(),
            this->input.template Ref<ZInput>());
    }
};

class ComposeVector4Node : public BlenderNode<
    In<float, float, float, float>,
    Out<Vector4>> {
public:
    enum InputIndex : size_t {
        XInput,
        YInput,
        ZInput,
        WInput
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    ComposeVector4Node() = default;

    ComposeVector4Node(float x, float y, float z, float w) {
        this->input.SetDefaultValues(x, y, z, w);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = Vector4(
            this->input.template Ref<XInput>(),
            this->input.template Ref<YInput>(),
            this->input.template Ref<ZInput>(),
            this->input.template Ref<WInput>());
    }
};

class DecomposeVector2Node : public BlenderNode<
    In<Vector2>,
    Out<float, float>> {
public:
    enum InputIndex : size_t {
        ValueInput
    };

    enum OutputIndex : size_t {
        XOutput,
        YOutput
    };

    DecomposeVector2Node() = default;

    explicit DecomposeVector2Node(Vector2 value) {
        this->input.SetDefaultValues(value);
    }

    void ExecuteNode() override {
        const Vector2 value = this->input.template Ref<ValueInput>();
        this->output.template Ref<XOutput>() = value.x;
        this->output.template Ref<YOutput>() = value.y;
    }
};

class DecomposeVector3Node : public BlenderNode<
    In<Vector3>,
    Out<float, float, float>> {
public:
    enum InputIndex : size_t {
        ValueInput
    };

    enum OutputIndex : size_t {
        XOutput,
        YOutput,
        ZOutput
    };

    DecomposeVector3Node() = default;

    explicit DecomposeVector3Node(Vector3 value) {
        this->input.SetDefaultValues(value);
    }

    void ExecuteNode() override {
        const Vector3 value = this->input.template Ref<ValueInput>();
        this->output.template Ref<XOutput>() = value.x;
        this->output.template Ref<YOutput>() = value.y;
        this->output.template Ref<ZOutput>() = value.z;
    }
};

class DecomposeVector4Node : public BlenderNode<
    In<Vector4>,
    Out<float, float, float, float>> {
public:
    enum InputIndex : size_t {
        ValueInput
    };

    enum OutputIndex : size_t {
        XOutput,
        YOutput,
        ZOutput,
        WOutput
    };

    DecomposeVector4Node() = default;

    explicit DecomposeVector4Node(Vector4 value) {
        this->input.SetDefaultValues(value);
    }

    void ExecuteNode() override {
        const Vector4 value = this->input.template Ref<ValueInput>();
        this->output.template Ref<XOutput>() = value.x;
        this->output.template Ref<YOutput>() = value.y;
        this->output.template Ref<ZOutput>() = value.z;
        this->output.template Ref<WOutput>() = value.w;
    }
};

class ComposeTransformNode : public BlenderNode<
    In<Vector3, Quaternion, Vector3, Vector3>,
    Out<Transform>> {
public:
    enum InputIndex : size_t {
        PositionInput, // World-space translation value
        QuaternionInput, // Preferred quaternion rotation input
        EulerInput, // Optional Euler rotation fallback (radians)
        ScaleInput // Non-uniform scaling factors
    };

    enum OutputIndex : size_t {
        ResultOutput // Composed transform
    };

    ComposeTransformNode() {
        this->input.SetDefaultValues(
            Vector3{ 0.0f, 0.0f, 0.0f },
            Quaternion(Vector3{ 0.0f, 0.0f, 0.0f }),
            Vector3{ 0.0f, 0.0f, 0.0f },
            Vector3{ 1.0f, 1.0f, 1.0f });
    }

    ComposeTransformNode(Vector3 position, Quaternion rotation, Vector3 eulerFallback, Vector3 scale) {
        this->input.SetDefaultValues(position, rotation, eulerFallback, scale);
    }

    void ExecuteNode() override {
        Transform transform;
        transform.SetPosition(this->input.template Ref<PositionInput>());
        transform.SetScaling(this->input.template Ref<ScaleInput>());

        Quaternion orientation;
        if (this->input.template IsConnected<QuaternionInput>()) {
            orientation = this->input.template Ref<QuaternionInput>();
        }
        else {
            orientation = Quaternion(this->input.template Ref<EulerInput>());
        }
        transform.SetOrientation(orientation);

        this->output.template Ref<ResultOutput>() = transform;
    }
};

class DecomposeTransformNode : public BlenderNode<
    In<Transform>,
    Out<Vector3, Quaternion, Vector3, Vector3>> {
public:
    enum InputIndex : size_t {
        TransformInput // Transform to decompose
    };

    enum OutputIndex : size_t {
        PositionOutput, // Extracted translation
        QuaternionOutput, // Extracted quaternion rotation
        EulerOutput, // Rotation expressed as Euler angles (radians)
        ScaleOutput // Extracted scale
    };

    DecomposeTransformNode() = default;

    explicit DecomposeTransformNode(Transform transform) {
        this->input.SetDefaultValues(transform);
    }

    void ExecuteNode() override {
        Transform const& transform = this->input.template Ref<TransformInput>();
        const Vector3 position = transform.GetPosition();
        const Quaternion orientation = transform.GetOrientation();
        const Vector3 euler = orientation.ToEuler();
        const Vector3 scale = transform.GetScaling();

        this->output.template Ref<PositionOutput>() = position;
        this->output.template Ref<QuaternionOutput>() = orientation;
        this->output.template Ref<EulerOutput>() = euler;
        this->output.template Ref<ScaleOutput>() = scale;
    }
};

} // namespace weave::blender
