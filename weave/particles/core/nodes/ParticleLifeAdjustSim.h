#pragma once

#include "weave/system/math/Transform.h"

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleLifeAdjustSim : public ParticleNode<
	blender::In<Transform, float, float, float, bool, bool, bool>,
	blender::Out<>> {
public:
	ParticleLifeAdjustSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		TransformInput,
		RadiusInput,
		MinLifeInput,
		MaxLifeInput,
		RelativeMinInput,
		RelativeMaxInput,
		ClampUpperInput
	};

private:
	void AdjustAllParticles(float minInput, float maxInput, bool relativeMin, bool relativeMax, bool clampUpper);
	void AdjustParticlesInRadius(float radiusSq,
		float minInput, float maxInput, bool relativeMin, bool relativeMax, bool clampUpper);

};

} // namespace weave::particles
