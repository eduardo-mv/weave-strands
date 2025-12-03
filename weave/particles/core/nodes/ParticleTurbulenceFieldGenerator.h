#pragma once

#include "ParticleTurbulenceSim.h"
#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleTurbulenceFieldGenerator : public ParticleNode<
	blender::In<float, float, float, float, uint32_t>,
	blender::Out<TurbulenceFieldList>> {
public:
	ParticleTurbulenceFieldGenerator();

	void ExecuteNode() override;

	struct FieldConfig {
		Vector3 direction{ 0.0f, 0.0f, 1.0f };
		float minForce{ -1.0f };
		float maxForce{ -1.0f };
		float variation{ -1.0f };
	};

	void SetFields(std::vector<FieldConfig> configs);

	enum InputIndex : size_t {
		MinForceInput,
		MaxForceInput,
		FrequencyInput,
		VariationInput,
		FieldCountInput
	};

private:
	struct RuntimeField {
		Vector3 direction;
		float minForce{ -1.0f };
		float maxForce{ -1.0f };
		float variation{ -1.0f };
	};

	void EnsureFieldCount(uint32_t desiredCount);

	std::vector<RuntimeField> fields;
	float nextUpdate{ 0.0f };
};

} // namespace weave::particles
