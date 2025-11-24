#pragma once

#include "weave/particles/nodes/ParticleNode.h"

namespace weave::particles {

class ParticlePhysicsSim : public ParticleNode<
	blender::In<float, Vector3>,
	blender::Out<>> {
public:
	ParticlePhysicsSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		LinearDamping,
		Gravity
	};
};

} // namespace weave::particles

