#pragma once

#include "weave/system/math/Transform.h"

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleTransformInit : public ParticleNode<
	blender::In<Transform, bool, bool, bool, bool, bool, bool, bool>,
	blender::Out<>> {
public:
	ParticleTransformInit();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		TransformInput,    // Transform applied to particles during init
		ApplyEmission,     // Whether to affect the emission buffer
		ApplyEditable,     // Whether to affect the editable buffer
		TranslatePosition, // Translates particle positions by the transform
		RotatePosition,    // Rotates particle positions by the transform
		ScalePosition,     // Scales particle positions by the transform
		RotateVelocity,    // Rotates particle velocities
		ScaleVelocity      // Scales particle velocities
	};
};

} // namespace weave::particles
