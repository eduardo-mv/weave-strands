#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleDragSim : public ParticleNode<
	blender::In<float, float, float, float, float>,
	blender::Out<>,
	blender::Flow<ParticleBuffer*>> {
public:
	ParticleDragSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		LinearDrag,      // Constant damping applied each frame
		ExponentialDrag, // Damping proportional to current velocity
		MinVelocityCap,  // Lower clamp for particle speed after drag
		MaxVelocityCap,  // Upper clamp for particle speed after drag
		BreakVelocity    // Threshold where particles come to rest
	};
};

} // namespace weave::particles
