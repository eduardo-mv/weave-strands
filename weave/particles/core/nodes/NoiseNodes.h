#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <type_traits>

#include "weave/system/blender/Blender.h"
#include "weave/system/math/Noise.h"
#include "weave/system/math/VectorMath.h"

namespace weave::blender {

namespace detail {

template<typename VectorType>
struct VectorTraits;

template<>
struct VectorTraits<Vector2> {
	static constexpr size_t kComponents = 2;
};

template<>
struct VectorTraits<Vector3> {
	static constexpr size_t kComponents = 3;
};

template<>
struct VectorTraits<Vector4> {
	static constexpr size_t kComponents = 4;
};

template<typename VectorType>
VectorType Broadcast(float value) {
	return VectorType(value);
}

inline float EvaluatePerlin(Vector2 const& sample) {
	return noise::perlin(sample.x, sample.y, 0.0f);
}

inline float EvaluatePerlin(Vector3 const& sample) {
	return noise::perlin(sample.x, sample.y, sample.z);
}

inline float EvaluatePerlin(Vector4 const& sample) {
	return noise::perlin(sample.x, sample.y, sample.z);
}

inline float EvaluateSimplex(Vector2 const& sample) {
	return noise::simplex(sample.x, sample.y);
}

inline float EvaluateSimplex(Vector3 const& sample) {
	return noise::simplex(sample.x, sample.y, sample.z);
}

inline float EvaluateSimplex(Vector4 const& sample) {
	return noise::simplex(sample.x, sample.y, sample.z, sample.w);
}

inline float EvaluateSimplexLoop(Vector2 const& sample, float freq) {
	return noise::simplexLoop(sample.x, sample.y, freq);
}

inline float EvaluateSimplexLoop(Vector3 const& sample, float freq) {
	return noise::simplexLoop(sample.x, sample.y, sample.z, freq);
}

inline float EvaluateSimplexLoop(Vector4 const& sample, float freq) {
	return noise::simplexLoop(sample.x, sample.y, sample.z, sample.w, freq);
}

inline float EvaluatePerlinLoop(Vector2 const& sample) {
	return noise::perlinLoop(sample.x, sample.y);
}

inline float EvaluatePerlinLoop(Vector3 const& sample) {
	return noise::perlinLoop(sample.x, sample.y, sample.z);
}

template<typename VectorType, typename EvalFunc>
VectorType SampleNoiseVector(VectorType const& position, VectorType const& offset,
	VectorType const& frequency, VectorType const& seed, EvalFunc&& eval) {
	VectorType result{};
	for (size_t i = 0; i < VectorTraits<VectorType>::kComponents; ++i) {
		const float freqScalar = frequency[i];
		const float seedScalar = seed[i];
		VectorType componentSample = position * freqScalar + offset + Broadcast<VectorType>(seedScalar);
		result[i] = static_cast<float>(eval(componentSample));
	}
	return result;
}

template<typename VectorType, typename EvalFunc>
VectorType SampleLoopNoiseVector(VectorType const& position, VectorType const& offset,
	VectorType const& frequency, VectorType const& seed, float loopFrequency, EvalFunc&& eval) {
	VectorType result{};
	for (size_t i = 0; i < VectorTraits<VectorType>::kComponents; ++i) {
		const float freqScalar = frequency[i];
		const float seedScalar = seed[i];
		VectorType componentSample = position * freqScalar + offset + Broadcast<VectorType>(seedScalar);
		result[i] = static_cast<float>(eval(componentSample, loopFrequency));
	}
	return result;
}

} // namespace detail

template<typename VectorType>
class PerlinNoiseNode : public BlenderNode<
	In<VectorType, VectorType, VectorType, VectorType>,
	Out<VectorType>> {
public:
	enum InputIndex : size_t {
		PositionInput,
		OffsetInput,
        FrequencyInput,
        SeedInput
    };

	PerlinNoiseNode() {
		this->input.SetDefaultValues(
			VectorType{},
			VectorType{},
			VectorType(1.0f),
			VectorType{}
		);
	}

	void ExecuteNode() override {
		const VectorType position = this->input.template Ref<PositionInput>();
		const VectorType offset = this->input.template Ref<OffsetInput>();
		const VectorType frequency = this->input.template Ref<FrequencyInput>();
		const VectorType seed = this->input.template Ref<SeedInput>();

		auto result = detail::SampleNoiseVector<VectorType>(
			position, offset, frequency, seed,
			[](VectorType const& sample) {
				return detail::EvaluatePerlin(sample);
			});

		this->output.template Ref<0>() = result;
	}
};

template<typename VectorType>
class SimplexNoiseNode : public BlenderNode<
	In<VectorType, VectorType, VectorType, VectorType>,
	Out<VectorType>> {
public:
	enum InputIndex : size_t {
		PositionInput,
		OffsetInput,
        FrequencyInput,
        SeedInput
    };

	SimplexNoiseNode() {
		this->input.SetDefaultValues(
			VectorType{},
			VectorType{},
			VectorType(1.0f),
			VectorType{}
		);
	}

	void ExecuteNode() override {
		const VectorType position = this->input.template Ref<PositionInput>();
		const VectorType offset = this->input.template Ref<OffsetInput>();
		const VectorType frequency = this->input.template Ref<FrequencyInput>();
		const VectorType seed = this->input.template Ref<SeedInput>();

		auto result = detail::SampleNoiseVector<VectorType>(
			position, offset, frequency, seed,
			[](VectorType const& sample) {
				return detail::EvaluateSimplex(sample);
			});

		this->output.template Ref<0>() = result;
	}
};

template<typename VectorType>
class TurbulenceNoiseNode : public BlenderNode<
	In<VectorType, VectorType, VectorType, VectorType, int32_t, float, float>,
	Out<VectorType>> {
public:
	enum InputIndex : size_t {
		PositionInput,
		OffsetInput,
		FrequencyInput,
		SeedInput,
		OctavesInput,
		PersistenceInput,
		LacunarityInput
	};

	TurbulenceNoiseNode() {
		this->input.SetDefaultValues(
			VectorType{},
			VectorType{},
			VectorType(1.0f),
			VectorType{},
			3,
			0.5f,
			2.0f
		);
	}

	void ExecuteNode() override {
		const VectorType position = this->input.template Ref<PositionInput>();
		const VectorType offset = this->input.template Ref<OffsetInput>();
		const VectorType frequency = this->input.template Ref<FrequencyInput>();
		const VectorType seed = this->input.template Ref<SeedInput>();
		const int octaves = std::max(1, this->input.template Ref<OctavesInput>());
		const float persistence = std::clamp(this->input.template Ref<PersistenceInput>(), 0.0f, 1.0f);
		const float lacunarity = std::max(0.0001f, this->input.template Ref<LacunarityInput>());

		auto result = detail::SampleNoiseVector<VectorType>(
			position, offset, frequency, seed,
			[&](VectorType const& componentSample, size_t componentIndex) {
				float amplitude = 1.0f;
				float frequencyMul = 1.0f;
				float value = 0.0f;
				for (int octave = 0; octave < octaves; ++octave) {
					VectorType octaveSample = componentSample * frequencyMul
						+ detail::Broadcast<VectorType>(static_cast<float>(componentIndex) * 17.0f)
						+ detail::Broadcast<VectorType>(static_cast<float>(octave) * 23.0f);
					value += amplitude * std::abs(detail::EvaluateSimplex(octaveSample));
					amplitude *= persistence;
					frequencyMul *= lacunarity;
				}
				return value;
			});

		this->output.template Ref<0>() = result;
	}
};

template<typename VectorType>
class SimplexLoopNoiseNode : public BlenderNode<
	In<VectorType, VectorType, VectorType, VectorType, float>,
	Out<VectorType>> {
public:
	enum InputIndex : size_t {
		PositionInput,
		OffsetInput,
		FrequencyInput,
		SeedInput,
		LoopFrequencyInput
	};

	SimplexLoopNoiseNode() {
		this->input.SetDefaultValues(
			VectorType{},
			VectorType{},
			VectorType(1.0f),
			VectorType{},
			1.0f
		);
	}

	void ExecuteNode() override {
		const VectorType position = this->input.template Ref<PositionInput>();
		const VectorType offset = this->input.template Ref<OffsetInput>();
		const VectorType frequency = this->input.template Ref<FrequencyInput>();
		const VectorType seed = this->input.template Ref<SeedInput>();
		const float loopFrequency = this->input.template Ref<LoopFrequencyInput>();

		auto result = detail::SampleLoopNoiseVector<VectorType>(
			position, offset, frequency, seed, loopFrequency,
			[](VectorType const& sample, float freq) {
				return detail::EvaluateSimplexLoop(sample, freq);
			});

		this->output.template Ref<0>() = result;
	}
};

template<typename VectorType>
class PerlinLoopNoiseNode : public BlenderNode<
	In<VectorType, VectorType, VectorType, VectorType, float>,
	Out<VectorType>> {
public:
	enum InputIndex : size_t {
		PositionInput,
		OffsetInput,
		FrequencyInput,
		SeedInput,
		LoopFrequencyInput
	};

	PerlinLoopNoiseNode() {
		this->input.SetDefaultValues(
			VectorType{},
			VectorType{},
			VectorType(1.0f),
			VectorType{},
			1.0f
		);
	}

	void ExecuteNode() override {
		const VectorType position = this->input.template Ref<PositionInput>();
		const VectorType offset = this->input.template Ref<OffsetInput>();
		const VectorType frequency = this->input.template Ref<FrequencyInput>();
		const VectorType seed = this->input.template Ref<SeedInput>();
		const float loopFrequency = this->input.template Ref<LoopFrequencyInput>();

		auto result = detail::SampleLoopNoiseVector<VectorType>(
			position, offset, frequency, seed, loopFrequency,
			[](VectorType const& sample, float) {
				return detail::EvaluatePerlinLoop(sample);
			});

		this->output.template Ref<0>() = result;
	}
};

class CircularNoiseNode : public BlenderNode<
	In<Vector2, Vector2, Vector2, Vector2, float>,
	Out<Vector2>> {
public:
	enum InputIndex : size_t {
		PositionInput,
		OffsetInput,
		FrequencyInput,
		SeedInput,
		LoopFrequencyInput
	};

	CircularNoiseNode() {
		this->input.SetDefaultValues(
			Vector2{},
			Vector2{},
			Vector2(1.0f),
			Vector2{},
			1.0f
		);
	}

	void ExecuteNode() override {
		const Vector2 position = this->input.template Ref<PositionInput>();
		const Vector2 offset = this->input.template Ref<OffsetInput>();
		const Vector2 frequency = this->input.template Ref<FrequencyInput>();
		const Vector2 seed = this->input.template Ref<SeedInput>();
		const float loopFrequency = this->input.template Ref<LoopFrequencyInput>();

		auto result = detail::SampleLoopNoiseVector<Vector2>(
			position, offset, frequency, seed, loopFrequency,
			[](Vector2 const& sample, float freq) {
				return noise::circular(sample.x, sample.y, freq);
			});

		this->output.template Ref<0>() = result;
	}
};

using PerlinNoise2DNode = PerlinNoiseNode<Vector2>;
using PerlinNoise3DNode = PerlinNoiseNode<Vector3>;
using PerlinNoise4DNode = PerlinNoiseNode<Vector4>;

using SimplexNoise2DNode = SimplexNoiseNode<Vector2>;
using SimplexNoise3DNode = SimplexNoiseNode<Vector3>;
using SimplexNoise4DNode = SimplexNoiseNode<Vector4>;

using TurbulenceNoise2DNode = TurbulenceNoiseNode<Vector2>;
using TurbulenceNoise3DNode = TurbulenceNoiseNode<Vector3>;
using TurbulenceNoise4DNode = TurbulenceNoiseNode<Vector4>;

using SimplexLoopNoise2DNode = SimplexLoopNoiseNode<Vector2>;
using SimplexLoopNoise3DNode = SimplexLoopNoiseNode<Vector3>;
using SimplexLoopNoise4DNode = SimplexLoopNoiseNode<Vector4>;

using PerlinLoopNoise2DNode = PerlinLoopNoiseNode<Vector2>;
using PerlinLoopNoise3DNode = PerlinLoopNoiseNode<Vector3>;

using CircularLoopNoiseNode = CircularNoiseNode;

} // namespace weave::blender
