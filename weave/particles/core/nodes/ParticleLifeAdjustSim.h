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
		TransformInput,   // Reference transform for optional radius queries
		RadiusInput,      // Radius to limit lifetime adjustments
		MinLifeInput,     // Minimum lifetime to enforce
		MaxLifeInput,     // Maximum lifetime to enforce
		RelativeMinInput, // Treat min life as a relative fraction of current life
		RelativeMaxInput, // Treat max life as a relative fraction of current life
		ClampUpperInput   // Whether to clamp lifetimes that exceed the max
	};

private:
	void AdjustAllParticles(float minInput, float maxInput, bool relativeMin, bool relativeMax, bool clampUpper);
	void AdjustParticlesInRadius(float radiusSq,
		float minInput, float maxInput, bool relativeMin, bool relativeMax, bool clampUpper);

};

} // namespace weave::particles
