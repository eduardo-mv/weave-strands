#pragma once

/*
* Generic arithmetic nodes to use with a Blender
*/

#include "weave/system/blender/Blender.h"
#include "weave/system/math/Easing.h"
#include "weave/system/math/Interpolation.h"
#include <cstddef>
#include <tuple>

namespace weave::blender {

template<typename... Types>
class ConstNode : public BlenderNode<
    Out<Types...>> {
public:
    ConstNode() = default;

    ConstNode(Types... values) {
        this->output.Set(values...);
    }

    void ExecuteNode() override { }
};

template<typename Type>
class AbsNode : public BlenderNode<
    In<Type>,
    Out<Type>> {
public:
    enum InputIndex : size_t {
        ValueInput // Operand to abs()
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    AbsNode() = default;

    AbsNode(Type value) {
        this->input.SetDefaultValues(value);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = std::abs(this->input.template Ref<ValueInput>());
    }
};

template<typename... Types>
class AddNode : public BlenderNode<
    In<Types...>,
    Out<typename std::common_type<Types...>::type>> {
public:
    enum OutputIndex : size_t {
        ResultOutput
    };

    AddNode() = default;

    AddNode(Types ... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        AddInputs(std::index_sequence_for<Types...>());
    }

private:
    template<std::size_t... I>
    void AddInputs(std::index_sequence<I...>) {
        using ReturnType = typename std::common_type<Types...>::type;
        this->output.template Ref<ResultOutput>() = (static_cast<ReturnType>(this->input.template Ref<I>()) + ...);
    }
};



template<typename... Types>
class SubtractNode : public BlenderNode<
    In<Types...>,
    Out<typename std::common_type<Types...>::type>> {
public:
    enum OutputIndex : size_t {
        ResultOutput
    };

    SubtractNode() = default;

    SubtractNode(Types ... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        SubtractInputs(std::index_sequence_for<Types...>());
    }

private:
    template<std::size_t... I>
    void SubtractInputs(std::index_sequence<I...>) {
        using ReturnType = typename std::common_type<Types...>::type;
        this->output.template Ref<ResultOutput>() = ((static_cast<ReturnType>(this->input.template Ref<I>()) - ...));
    }
};


template<typename... Types>
class MultiplyNode : public BlenderNode<
    In<Types...>,
    Out<typename std::common_type<Types...>::type>> {
public:
    enum OutputIndex : size_t {
        ResultOutput
    };

    MultiplyNode() = default;

    MultiplyNode(Types ... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        MultiplyInputs(std::index_sequence_for<Types...>());
    }

private:
    template<std::size_t... I>
    void MultiplyInputs(std::index_sequence<I...>) {
        using ReturnType = typename std::common_type<Types...>::type;
        this->output.template Ref<ResultOutput>() = ((static_cast<ReturnType>(this->input.template Ref<I>()) * ...));
    }
};


template<typename... Types>
class DivideNode : public BlenderNode<
    In<Types...>,
    Out<typename std::common_type<Types...>::type>> {
public:
    enum OutputIndex : size_t {
        ResultOutput
    };

    DivideNode() = default;

    DivideNode(Types ... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        DivideInputs(std::index_sequence_for<Types...>());
    }

private:
    template<std::size_t... I>
    void DivideInputs(std::index_sequence<I...>) {
        using ReturnType = typename std::common_type<Types...>::type;
        this->output.template Ref<ResultOutput>() = ((static_cast<ReturnType>(this->input.template Ref<I>()) / ...));
    }
};


template<typename... Types>
class ModuloNode : public BlenderNode<
    In<Types...>,
    Out<typename std::common_type<Types...>::type>> {
public:
    enum OutputIndex : size_t {
        ResultOutput
    };

    ModuloNode() = default;

    ModuloNode(Types ... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        ModuloInputs(std::index_sequence_for<Types...>());
    }

private:
    template<std::size_t... I>
    void ModuloInputs(std::index_sequence<I...>) {
        using ReturnType = typename std::common_type<Types...>::type;
        this->output.template Ref<ResultOutput>() = ((static_cast<ReturnType>(this->input.template Ref<I>()) % ...));
    }
};

template<typename BaseType = float, typename ExponentType = BaseType>
class PowerNode : public BlenderNode<
    In<BaseType, ExponentType>,
    Out<typename std::common_type<BaseType, ExponentType>::type>> {
public:
    enum InputIndex : size_t {
        BaseInput,     // Value being raised to a power
        ExponentInput  // Exponent applied to the base
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    PowerNode() = default;

    PowerNode(BaseType base, ExponentType exponent) {
        this->input.SetDefaultValues(base, exponent);
    }

    void ExecuteNode() override {
        using ReturnType = typename std::common_type<BaseType, ExponentType>::type;
        this->output.template Ref<ResultOutput>() = std::pow(
            static_cast<ReturnType>(this->input.template Ref<BaseInput>()),
            static_cast<ReturnType>(this->input.template Ref<ExponentInput>()));
    }
};

template<typename Type = float>
class SquareRootNode : public BlenderNode<
    In<Type>,
    Out<typename std::common_type<Type>::type>> {
public:
    enum InputIndex : size_t {
        ValueInput // Operand to square-root
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    SquareRootNode() = default;

    SquareRootNode(Type value) {
        this->input.SetDefaultValues(value);
    }

    void ExecuteNode() override {
        using ReturnType = typename std::common_type<Type>::type;
        this->output.template Ref<ResultOutput>() = std::sqrt(static_cast<ReturnType>(this->input.template Ref<ValueInput>()));
    }
};

template<typename Type = float>
class AbsoluteNode : public BlenderNode<
    In<Type>,
    Out<typename std::common_type<Type>::type>> {
public:
    enum InputIndex : size_t {
        ValueInput // Operand to abs()
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    AbsoluteNode() = default;

    AbsoluteNode(Type value) {
        this->input.SetDefaultValues(value);
    }

    void ExecuteNode() override {
        using ReturnType = typename std::common_type<Type>::type;
        this->output.template Ref<ResultOutput>() = std::abs(static_cast<ReturnType>(this->input.template Ref<ValueInput>()));
    }
};

template<typename Type = float>
class ClampNode : public BlenderNode<
    In<Type, Type, Type>,
    Out<Type>> {
public:
    enum InputIndex : size_t {
        ValueInput, // Value to clamp
        MinInput,   // Lower bound
        MaxInput    // Upper bound
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    ClampNode() = default;

    ClampNode(Type value, Type minValue, Type maxValue) {
        this->input.SetDefaultValues(value, minValue, maxValue);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = std::clamp(
            this->input.template Ref<ValueInput>(),
            this->input.template Ref<MinInput>(),
            this->input.template Ref<MaxInput>());
    }
};

template<typename... Types>
class MinMaxNode : public BlenderNode<
    In<Types...>,
    Out<typename std::common_type<Types...>::type, typename std::common_type<Types...>::type>> {
public:
    enum OutputIndex : size_t {
        MinOutput, // Minimum value across all inputs
        MaxOutput  // Maximum value across all inputs
    };

    MinMaxNode() = default;

    MinMaxNode(Types ... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        ComputeMinMax(std::index_sequence_for<Types...>());
    }

private:
    template<std::size_t... I>
    void ComputeMinMax(std::index_sequence<I...>) {
        using ReturnType = typename std::common_type<Types...>::type;
        this->output.template Ref<MinOutput>() = (std::min({ static_cast<ReturnType>(this->input.template Ref<I>())... }));
        this->output.template Ref<MaxOutput>() = (std::max({ static_cast<ReturnType>(this->input.template Ref<I>())... }));
    }
};

template<typename Type = float, typename WeightType = Type>
class LerpNode : public BlenderNode<
    In<Type, Type, WeightType>,
    Out<Type>> {
private:
    weave::easing::EasingCurve curve = weave::easing::linear;
public:
    enum InputIndex : size_t {
        AInput,       // Starting value
        BInput,       // Ending value
        WeightInput   // Interpolation factor (0..1)
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    LerpNode() = default;

    LerpNode(weave::easing::EasingCurve curve, Type valueA, Type valueB, WeightType weight)
    : curve(curve) {
        this->input.SetDefaultValues(valueA, valueB, weight);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = weave::interpolation::lerp(
            this->input.template Ref<AInput>(),
            this->input.template Ref<BInput>(),
            this->input.template Ref<WeightInput>());
    }
};

template<typename Type = float, typename WeightType = Type>
class SmoothstepNode : public BlenderNode<
    In<Type, Type, WeightType>,
    Out<Type>> {
public:
    enum InputIndex : size_t {
        AInput,       // Starting value
        BInput,       // Ending value
        WeightInput   // Interpolation factor before smoothstep
    };

    enum OutputIndex : size_t {
        ResultOutput
    };

    SmoothstepNode() = default;

    SmoothstepNode(Type valueA, Type valueB, WeightType weight) {
        this->input.SetDefaultValues(valueA, valueB, weight);
    }

    void ExecuteNode() override {
        WeightType t = weave::interpolation::smoothstep(this->input.template Ref<WeightInput>());
        this->output.template Ref<ResultOutput>() = weave::interpolation::lerp(
            this->input.template Ref<AInput>(),
            this->input.template Ref<BInput>(),
            t);
    }
};


} // namespace weave::blender
