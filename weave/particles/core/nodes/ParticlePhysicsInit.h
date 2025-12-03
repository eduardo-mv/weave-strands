#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticlePhysicsInit : public ParticleNode<
	blender::In<float, float, float, float>,
	blender::Out<>> {
public:
	ParticlePhysicsInit();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		MinAgeInput,
		MaxAgeInput,
		MinMassInput,
		MaxMassInput
	};
};

} // namespace weave::particles

