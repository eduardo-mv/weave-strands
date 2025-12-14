#pragma once

#include <cstdint>
#include <iostream>
#include <format>

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleEmitter : public ParticleNode<
	blender::In<float, float, float, float, float, double, uint64_t, uint32_t>,
	blender::Out<>, 
	blender::Flow<ParticleBuffer*, EmissionRange>> {
public:
	ParticleEmitter();

	void ExecuteNode() override;

	void Reset();

	enum InputIndex : size_t {
		MinEmitHz,        // Minimum particles emitted per second
		MaxEmitHz,        // Maximum particles emitted per second
		Rate,           // Scalar multiplier applied to emission counts
		MinFrequency,   // Shortest delay between emission windows (seconds)
		MaxFrequency,   // Longest delay between emission windows (seconds)
		MaxRuntime,     // Optional lifetime cap for the emitter (seconds)
		MaxParticles,   // Hard cap for particles managed by this emitter
		ResetSignal,    // Optional external reset trigger value
		InputCount      // Sentinel entry for total emitter inputs
	};

private:
	float accumTime = 0.0f;
	float nextFreq = 0.0f;
	double runTime = 0.0;
	double fractionalCarry = 0.0;
	uint64_t emittedTotal = 0;
    uint32_t lastResetValue = 0;

    float RandomFrequency(float minFreq, float maxFreq) const;

	uint64_t GenerateParticles();
};

}
