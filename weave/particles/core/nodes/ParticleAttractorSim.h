#pragma once

#include "weave/system/math/Transform.h"

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleAttractorSim : public ParticleNode<
	blender::In<Transform, float, float, float, float, Vector3>,
	blender::Out<>,
	blender::Flow<ParticleBuffer*>> {
public:
	ParticleAttractorSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		AttractorTransform, // World transform the particles move toward
		ForceMagnitude,     // Base force applied toward the attractor
		GravityScale,       // Additional gravity-like pull strength
		Radius,             // Distance where the attractor stops influencing
		Decay,              // Falloff applied as particles move away
		Direction           // Optional direction override instead of transform
	};
};

} // namespace weave::particles
