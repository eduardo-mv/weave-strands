#pragma once

#include "weave/system/math/Transform.h"

#include "weave/particles/nodes/ParticleNode.h"

namespace weave::particles {

struct TurbulenceField {
	Vector3 direction;
	float forceMagnitude{ 0.0f };
};

using TurbulenceFieldList = std::vector<TurbulenceField>;

class ParticleTurbulenceSim : public ParticleNode<
	blender::In<Transform, TurbulenceFieldList, float, float>,
	blender::Out<>> {
public:
	ParticleTurbulenceSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		TransformInput,
		FieldListInput,
		RadiusInput,
		DecayInput
	};

private:
	void ApplyTurbulence(TurbulenceFieldList const& fields, Transform const& transform,
		float radiusInput, float decayInput);
};

} // namespace weave::particles
