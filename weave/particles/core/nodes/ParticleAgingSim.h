#pragma once

#include "weave/system/math/Transform.h"

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleAgingSim : public ParticleNode<
	blender::In<Transform, float, float>,
	blender::Out<>,
	blender::Flow<ParticleBuffer*>> {
public:
	ParticleAgingSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		TransformInput,      // Reference transform used for radius tests
		RadiusInput,         // Optional radius in which to age particles faster
		AgingMultiplierInput // Scalar applied to the lifetime decay
	};

private:
	void AgeParticles(ParticleBuffer* buffer, float deltaTime);
	void AgeParticlesInRadius(ParticleBuffer* buffer, float deltaTime, float radiusSq);
};

} // namespace weave::particles
