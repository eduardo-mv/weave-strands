#pragma once

#include "weave/system/math/Transform.h"

#include "weave/particles/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleAttractorSim : public ParticleNode<
	blender::In<Transform, float, float, float, float, Vector3>,
	blender::Out<>> {
public:
	ParticleAttractorSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		AttractorTransform,
		ForceMagnitude,
		GravityScale,
		Radius,
		Decay,
		Direction
	};
};

} // namespace weave::particles

