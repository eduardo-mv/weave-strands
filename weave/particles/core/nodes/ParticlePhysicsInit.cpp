#include "ParticlePhysicsInit.h"

#include <algorithm>
#include <iostream>

#include "weave/particles/core/ParticleBuffer.h"
#include "weave/system/math/Random.h"

namespace weave::particles {

namespace {
float RandomBetween(float minValue, float maxValue) {
	const float range = maxValue - minValue;
	if (range <= 0.0f) {
		return minValue;
	}
	return weave::rng::uniform<float>() * range + minValue;
}
} // namespace

ParticlePhysicsInit::ParticlePhysicsInit() {
	this->input.SetDefaultValues(
		0.1f, // min age
		1.0f, // max age
		0.1f, // min mass
		1.0f  // max mass
	);
}

void ParticlePhysicsInit::ExecuteNode() {
	const float inputMinAge = this->input.template Ref<MinAgeInput>();
	const float inputMaxAge = this->input.template Ref<MaxAgeInput>();
	const float inputMinMass = this->input.template Ref<MinMassInput>();
	const float inputMaxMass = this->input.template Ref<MaxMassInput>();

	const float minAge = std::min(inputMinAge, inputMaxAge);
	const float maxAge = std::max(inputMinAge, inputMaxAge);
	const float minMass = std::min(inputMinMass, inputMaxMass);
	const float maxMass = std::max(inputMinMass, inputMaxMass);

	for(auto const& emissionRange : flow.Iterate<EmissionRange>()) {
		if(!emissionRange.buffer)
			continue;

		auto& layout = emissionRange.buffer->GetLayout();
		auto offsets = layout.GetOffsets<layout::Force, layout::Mass, layout::LifeTime, layout::MaxLifeTime>();
		if (ParticleLayout::HasInvalidOffsets(offsets)) {
			return;
		}

		auto span = emissionRange.EmissionSpan<layout::Force, layout::Mass, layout::LifeTime, layout::MaxLifeTime>(offsets);
		for (auto&& [force, mass, life, maxLife] : span) {
			force.force = Vector3{};
			mass.mass = RandomBetween(minMass, maxMass);
			maxLife.maxLifeTime = RandomBetween(minAge, maxAge);
			life.lifeTime = 0.0f;
		}
	}
}

} // namespace weave::particles
