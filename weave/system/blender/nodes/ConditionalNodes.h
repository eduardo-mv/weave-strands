#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "weave/system/blender/Blender.h"

namespace weave::blender {

namespace detail {

template<typename... Types>
struct AllBools {
    static constexpr bool value = (std::is_same_v<std::decay_t<Types>, bool> && ...);
};

template<typename... Types>
struct AtLeastTwo {
    static constexpr bool value = sizeof...(Types) >= 2;
};

} // namespace detail

template<typename... BoolInputs>
class AndNode : public BlenderNode<
    In<BoolInputs...>,
    Out<bool>> {
    static_assert(detail::AtLeastTwo<BoolInputs...>::value, "AndNode requires at least two inputs");
    static_assert(detail::AllBools<BoolInputs...>::value, "AndNode inputs must be bool");
public:
    enum OutputIndex : size_t {
        ResultOutput // True when every input evaluates to true
    };

    AndNode() = default;

    AndNode(BoolInputs... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = Evaluate(std::index_sequence_for<BoolInputs...>());
    }

private:
    template<std::size_t... I>
    bool Evaluate(std::index_sequence<I...>) const {
        bool result = true;
        ((result &= this->input.template Ref<I>()), ...);
        return result;
    }
};

template<typename... BoolInputs>
class OrNode : public BlenderNode<
    In<BoolInputs...>,
    Out<bool>> {
    static_assert(detail::AtLeastTwo<BoolInputs...>::value, "OrNode requires at least two inputs");
    static_assert(detail::AllBools<BoolInputs...>::value, "OrNode inputs must be bool");
public:
    enum OutputIndex : size_t {
        ResultOutput // True when at least one input evaluates to true
    };

    OrNode() = default;

    OrNode(BoolInputs... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = Evaluate(std::index_sequence_for<BoolInputs...>());
    }

private:
    template<std::size_t... I>
    bool Evaluate(std::index_sequence<I...>) const {
        bool result = false;
        ((result |= this->input.template Ref<I>()), ...);
        return result;
    }
};

template<typename... BoolInputs>
class XorNode : public BlenderNode<
    In<BoolInputs...>,
    Out<bool>> {
    static_assert(detail::AtLeastTwo<BoolInputs...>::value, "XorNode requires at least two inputs");
    static_assert(detail::AllBools<BoolInputs...>::value, "XorNode inputs must be bool");
public:
    enum OutputIndex : size_t {
        ResultOutput // True when an odd number of inputs are true
    };

    XorNode() = default;

    XorNode(BoolInputs... values) {
        this->input.SetDefaultValues(values...);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = Evaluate(std::index_sequence_for<BoolInputs...>());
    }

private:
    template<std::size_t... I>
    bool Evaluate(std::index_sequence<I...>) const {
        bool result = false;
        ((result ^= this->input.template Ref<I>()), ...);
        return result;
    }
};

class NotNode : public BlenderNode<
    In<bool>,
    Out<bool>> {
public:
    enum InputIndex : size_t {
        ValueInput // Boolean operand to invert
    };

    enum OutputIndex : size_t {
        ResultOutput // Logical NOT of the operand
    };

    NotNode() = default;

    explicit NotNode(bool value) {
        this->input.SetDefaultValues(value);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = !this->input.template Ref<ValueInput>();
    }
};

// Comparison nodes

template<typename Type>
class LessThanNode : public BlenderNode<
    In<Type, Type>,
    Out<bool>> {
public:
    enum InputIndex : size_t {
        AInput, // Left operand
        BInput  // Right operand
    };

    enum OutputIndex : size_t {
        ResultOutput // True when A < B
    };

    LessThanNode() = default;

    LessThanNode(Type a, Type b) {
        this->input.SetDefaultValues(a, b);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = this->input.template Ref<AInput>() < this->input.template Ref<BInput>();
    }
};

template<typename Type>
class LessEqualNode : public BlenderNode<
    In<Type, Type>,
    Out<bool>> {
public:
    enum InputIndex : size_t {
        AInput, // Left operand
        BInput  // Right operand
    };

    enum OutputIndex : size_t {
        ResultOutput // True when A <= B
    };

    LessEqualNode() = default;

    LessEqualNode(Type a, Type b) {
        this->input.SetDefaultValues(a, b);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = this->input.template Ref<AInput>() <= this->input.template Ref<BInput>();
    }
};

template<typename Type>
class GreaterThanNode : public BlenderNode<
    In<Type, Type>,
    Out<bool>> {
public:
    enum InputIndex : size_t {
        AInput, // Left operand
        BInput  // Right operand
    };

    enum OutputIndex : size_t {
        ResultOutput // True when A > B
    };

    GreaterThanNode() = default;

    GreaterThanNode(Type a, Type b) {
        this->input.SetDefaultValues(a, b);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = this->input.template Ref<AInput>() > this->input.template Ref<BInput>();
    }
};

template<typename Type>
class GreaterEqualNode : public BlenderNode<
    In<Type, Type>,
    Out<bool>> {
public:
    enum InputIndex : size_t {
        AInput, // Left operand
        BInput  // Right operand
    };

    enum OutputIndex : size_t {
        ResultOutput // True when A >= B
    };

    GreaterEqualNode() = default;

    GreaterEqualNode(Type a, Type b) {
        this->input.SetDefaultValues(a, b);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = this->input.template Ref<AInput>() >= this->input.template Ref<BInput>();
    }
};

template<typename Type>
class EqualNode : public BlenderNode<
    In<Type, Type>,
    Out<bool>> {
public:
    enum InputIndex : size_t {
        AInput, // Left operand
        BInput  // Right operand
    };

    enum OutputIndex : size_t {
        ResultOutput // True when A == B
    };

    EqualNode() = default;

    EqualNode(Type a, Type b) {
        this->input.SetDefaultValues(a, b);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = this->input.template Ref<AInput>() == this->input.template Ref<BInput>();
    }
};

template<typename Type>
class NotEqualNode : public BlenderNode<
    In<Type, Type>,
    Out<bool>> {
public:
    enum InputIndex : size_t {
        AInput, // Left operand
        BInput  // Right operand
    };

    enum OutputIndex : size_t {
        ResultOutput // True when A != B
    };

    NotEqualNode() = default;

    NotEqualNode(Type a, Type b) {
        this->input.SetDefaultValues(a, b);
    }

    void ExecuteNode() override {
        this->output.template Ref<ResultOutput>() = this->input.template Ref<AInput>() != this->input.template Ref<BInput>();
    }
};

} // namespace weave::blender
