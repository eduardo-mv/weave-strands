#pragma once

#include "weave/system/math/Transform.h"

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleAgingSim : public ParticleNode<
	blender::In<Transform, float, float>,
	blender::Out<>> {
public:
	ParticleAgingSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		TransformInput,
		RadiusInput,
		AgingMultiplierInput
	};

private:
	void AgeParticles(float deltaTime);
	void AgeParticlesInRadius(float deltaTime, float radiusSq);
};

} // namespace weave::particles

