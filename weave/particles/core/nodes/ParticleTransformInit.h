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
		TransformInput,
		ApplyEmission,
		ApplyEditable,
		TranslatePosition,
		RotatePosition,
		ScalePosition,
		RotateVelocity,
		ScaleVelocity
	};
};

} // namespace weave::particles
