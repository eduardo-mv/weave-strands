#pragma once

#include <cstdint>

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleEmitter : public ParticleNode<
	blender::In<float, float, float, float, float, double, uint64_t, uint32_t>,
	blender::Out<>> {
public:
	ParticleEmitter();

	void ExecuteNode() override;

	void Reset();

	enum InputIndex : size_t {
		MinEmit,
		MaxEmit,
		Rate,
		MinFrequency,
		MaxFrequency,
		MaxRuntime,
		MaxParticles,
		ResetSignal,
		InputCount
	};

private:
	float accumTime = 0.0f;
	float nextFreq = 0.0f;
	double runTime = 0.0;
	double fractionalCarry = 0.0;
	uint64_t emittedTotal = 0;
    uint32_t lastResetValue = 0;

    float RandomFrequency(float minFreq, float maxFreq) const;
};

}
