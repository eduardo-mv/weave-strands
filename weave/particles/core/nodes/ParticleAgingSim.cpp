#include "ParticleAgingSim.h"

#include <algorithm>

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

ParticleAgingSim::ParticleAgingSim() {
	Transform identity{};
	identity.Identity();
	this->input.SetDefaultValues(
		identity,
		-1.0f, // infinite radius
		1.0f   // aging multiplier
	);
}

void ParticleAgingSim::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}
	
	const float multiplier = this->input.template Ref<AgingMultiplierInput>();
	const float deltaTime = context.sampling.deltaTime * multiplier;
	if (deltaTime == 0.0f) {
		return;
	}
	
	const float radiusInput = this->input.template Ref<RadiusInput>();
	if(radiusInput < 0.0f) {
		AgeParticles(deltaTime);
	}
	else {
		const float radiusSq = radiusInput * radiusInput;
		AgeParticlesInRadius(deltaTime, radiusSq);
	}
}

void ParticleAgingSim::AgeParticles(float deltaTime) {
	auto& context = GetContext();

	for (auto buffer : context.IterateSimulationRanges()) {
		if (!buffer) {
			continue;
		}

		auto& layout = buffer->GetLayout();
		auto offsets = layout.GetOffsets<layout::LifeTime, layout::MaxLifeTime>();

		if (ParticleLayout::HasInvalidOffsets(offsets)) {
			continue;
		}

		auto editableSpan = buffer->EditableSpan<layout::LifeTime, layout::MaxLifeTime>(0, offsets);
		size_t index = 0;
		while (index < editableSpan.size()) {
			auto&& [life, maxLife] = editableSpan[index];
			life.lifeTime += deltaTime;

			if (life.lifeTime > maxLife.maxLifeTime) {
				buffer->KillParticles(index, 1);
				editableSpan = buffer->EditableSpan<layout::LifeTime, layout::MaxLifeTime>(0, offsets);
				continue;
			}

			if (life.lifeTime < 0.0f) {
				life.lifeTime = 0.0f;
			}

			++index;
		}
	}
}

void ParticleAgingSim::AgeParticlesInRadius(float deltaTime, float radiusSq) {
	const Transform transform = this->input.template Ref<TransformInput>();
	const Vector3 center = transform.GetPosition();
	
	auto& context = GetContext();

	for (auto buffer : context.IterateSimulationRanges()) {
		if (!buffer) {
			continue;
		}

		auto& layout = buffer->GetLayout();
		auto offsets = layout.GetOffsets<layout::Position, layout::LifeTime, layout::MaxLifeTime>();
		if (ParticleLayout::HasInvalidOffsets(offsets)) {
			continue;
		}

		auto editableSpan = buffer->EditableSpan<layout::Position, layout::LifeTime, layout::MaxLifeTime>(0, offsets);
		size_t index = 0;
		while (index < editableSpan.size()) {
			auto&& [position, life, maxLife] = editableSpan[index];
			Vector3 diff = position.pos - center;
			if (algebra::lengthSqr(diff) > radiusSq) {
				++index;
				continue;
			}

			life.lifeTime += deltaTime;

			if (life.lifeTime > maxLife.maxLifeTime) {
				buffer->KillParticles(index, 1);
				editableSpan = buffer->EditableSpan<layout::Position, layout::LifeTime, layout::MaxLifeTime>(0, offsets);
				continue;
			}

			if (life.lifeTime < 0.0f) {
				life.lifeTime = 0.0f;
			}

			++index;
		}
	}
}

} // namespace weave::particles
