#pragma once

#include "weave/system/math/Transform.h"
#include "weave/system/blender/nodes/TurbulenceFieldTypes.h"

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleTurbulenceSim : public ParticleNode<
	blender::In<Transform, blender::TurbulenceFieldList, float, float>,
	blender::Out<>> {
public:
	ParticleTurbulenceSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		TransformInput, // Emitter transform to evaluate fields in local space
		FieldListInput, // List of turbulence fields to apply
		RadiusInput,    // Optional influence radius
		DecayInput      // Falloff applied as particles leave the radius
	};

private:
	void ApplyTurbulence(blender::TurbulenceFieldList const& fields, Transform const& transform,
		float radiusInput, float decayInput);
};

} // namespace weave::particles
