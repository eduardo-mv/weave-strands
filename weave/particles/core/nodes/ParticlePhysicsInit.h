#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticlePhysicsInit : public ParticleNode<
	blender::In<float, float, float, float>,
	blender::Out<>,
	blender::Flow<EmissionRange>> {
public:
	ParticlePhysicsInit();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		MinAgeInput,  // Minimum random lifetime assigned to particles
		MaxAgeInput,  // Maximum random lifetime assigned to particles
		MinMassInput, // Minimum random mass
		MaxMassInput  // Maximum random mass
	};
};

} // namespace weave::particles
