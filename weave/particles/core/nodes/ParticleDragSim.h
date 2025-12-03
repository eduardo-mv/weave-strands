#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleDragSim : public ParticleNode<
	blender::In<float, float, float, float, float>,
	blender::Out<>> {
public:
	ParticleDragSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		LinearDrag,
		ExponentialDrag,
		MinVelocityCap,
		MaxVelocityCap,
		BreakVelocity
	};
};

} // namespace weave::particles

