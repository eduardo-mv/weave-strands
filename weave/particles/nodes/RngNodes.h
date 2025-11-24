#pragma once

#include <vector>

#include "weave/system/blender/Blender.h"
#include "weave/system/math/Random.h"
#include "weave/system/math/VectorMath.h"

namespace weave::blender {

namespace detail {

template<typename T>
struct RngDefaultRange {
    static constexpr T Min() { return T{}; }
    static constexpr T Max() { return T{ 1 }; }
};

template<>
struct RngDefaultRange<Vector2> {
    static Vector2 Min() { return Vector2(0.0f, 0.0f); }
    static Vector2 Max() { return Vector2(1.0f, 1.0f); }
};

template<>
struct RngDefaultRange<Vector3> {
    static Vector3 Min() { return Vector3(0.0f, 0.0f, 0.0f); }
    static Vector3 Max() { return Vector3(1.0f, 1.0f, 1.0f); }
};

template<>
struct RngDefaultRange<Vector4> {
    static Vector4 Min() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }
    static Vector4 Max() { return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
};

template<typename T>
T RandomScalarBetween(T minValue, T maxValue) {
    if (maxValue < minValue) {
        std::swap(minValue, maxValue);
    }
    const T range = maxValue - minValue;
    if (range == T{}) {
        return minValue;
    }
    return weave::rng::uniform<T>() * range + minValue;
}

template<typename T>
struct RngRangeGenerator {
    static T Generate(T minValue, T maxValue) {
        return RandomScalarBetween(minValue, maxValue);
    }
};

template<>
struct RngRangeGenerator<Vector2> {
    static Vector2 Generate(Vector2 minValue, Vector2 maxValue) {
        return Vector2{
            RandomScalarBetween(minValue.x, maxValue.x),
            RandomScalarBetween(minValue.y, maxValue.y)
        };
    }
};

template<>
struct RngRangeGenerator<Vector3> {
    static Vector3 Generate(Vector3 minValue, Vector3 maxValue) {
        return Vector3{
            RandomScalarBetween(minValue.x, maxValue.x),
            RandomScalarBetween(minValue.y, maxValue.y),
            RandomScalarBetween(minValue.z, maxValue.z)
        };
    }
};

template<>
struct RngRangeGenerator<Vector4> {
    static Vector4 Generate(Vector4 minValue, Vector4 maxValue) {
        return Vector4{
            RandomScalarBetween(minValue.x, maxValue.x),
            RandomScalarBetween(minValue.y, maxValue.y),
            RandomScalarBetween(minValue.z, maxValue.z),
            RandomScalarBetween(minValue.w, maxValue.w)
        };
    }
};

} // namespace detail

template<typename Type>
class RandomRangeNode : public BlenderNode<
    In<Type, Type>,
    Out<Type>> {
public:
    RandomRangeNode() {
        this->input.SetDefaultValues(
            detail::RngDefaultRange<Type>::Min(),
            detail::RngDefaultRange<Type>::Max()
        );
    }

    RandomRangeNode(Type minValue, Type maxValue) {
        this->input.SetDefaultValues(minValue, maxValue);
    }

    void ExecuteNode() override {
        const Type minValue = this->input.template Ref<0>();
        const Type maxValue = this->input.template Ref<1>();
        this->output.template Ref<0>() = detail::RngRangeGenerator<Type>::Generate(minValue, maxValue);
    }
};

template<typename Type>
class RandomPoolNode : public BlenderNode<
    Out<Type>> {
public:
    RandomPoolNode() = default;

    RandomPoolNode(std::initializer_list<Type> values) {
        SetPool(values);
    }

    template<typename Container>
    explicit RandomPoolNode(Container const& values) {
        SetPool(values);
    }

    void ExecuteNode() override {
        if (pool.empty()) {
            return;
        }
        const size_t index = weave::rng::uniform<size_t>() % pool.size();
        this->output.template Ref<0>() = pool[index];
    }

    void SetPool(std::initializer_list<Type> values) {
        pool.assign(values.begin(), values.end());
    }

    template<typename Container>
    void SetPool(Container const& values) {
        pool.assign(values.begin(), values.end());
    }

private:
    std::vector<Type> pool;
};

} // namespace weave::blender

