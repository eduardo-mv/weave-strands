#pragma once

#include <cstdint>

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleRngVelocityInit : public ParticleNode<
	blender::In<float, float, int32_t, int32_t, int32_t, bool>,
	blender::Out<>> {
public:
	ParticleRngVelocityInit();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		MinVelocity, // Minimum randomized launch speed
		MaxVelocity, // Maximum randomized launch speed
		LimitX,      // Cosine clamp on X direction (-1..1)
		LimitY,      // Cosine clamp on Y direction (-1..1)
		LimitZ,      // Cosine clamp on Z direction (-1..1)
		Radial       // Toggles outward velocity from particle position
	};
};

} // namespace weave::particles
