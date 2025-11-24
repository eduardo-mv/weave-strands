#pragma once

#include <cstdint>

#include "weave/particles/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleRngVelocityInit : public ParticleNode<
	blender::In<float, float, int32_t, int32_t, int32_t, bool>,
	blender::Out<>> {
public:
	ParticleRngVelocityInit();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		MinVelocity,
		MaxVelocity,
		LimitX,
		LimitY,
		LimitZ,
		Radial
	};
};

} // namespace weave::particles
