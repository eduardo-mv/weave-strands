#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleVelocityInit : public ParticleNode<
	blender::In<Vector3, float, float, float, float>,
	blender::Out<>> {
public:
	ParticleVelocityInit();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		Direction,   // Central axis of the spawn cone
		UpAngle,     // Vertical spread angle
		SideAngle,   // Horizontal spread angle
		MinVelocity, // Minimum launch speed
		MaxVelocity  // Maximum launch speed
	};
};

} // namespace weave::particles
