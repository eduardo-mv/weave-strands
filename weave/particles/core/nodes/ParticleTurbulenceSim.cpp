#include "ParticleTurbulenceSim.h"

#include <algorithm>

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

namespace {
Vector3 TurbulenceComponent(Vector3 const& dir, Vector3 const& velocity) {
	const float speed = algebra::length(velocity);
	if (speed <= 0.0f) {
		return Vector3{};
	}

	Vector3 projected = algebra::project(dir, velocity / speed);
	return dir - projected;
}
} // namespace

ParticleTurbulenceSim::ParticleTurbulenceSim() {
	this->input.SetDefaultValues(
		Transform{},
		blender::TurbulenceFieldList{},
		-1.0f, // radius
		1.0f   // decay
	);
}

void ParticleTurbulenceSim::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}

	const Transform transform = this->input.template Ref<TransformInput>();
	const auto& fieldList = this->input.template Ref<FieldListInput>();
	if (fieldList.empty()) {
		return;
	}

	const float radiusInput = this->input.template Ref<RadiusInput>();
	const float decayInput = std::max(0.0f, this->input.template Ref<DecayInput>());

	for (auto* buffer : flow.Iterate<ParticleBuffer*>()) {
		if (!buffer) {
			continue;
		}
		ApplyTurbulence(fieldList, transform, radiusInput, decayInput, *buffer);
	}
}

void ParticleTurbulenceSim::ApplyTurbulence(blender::TurbulenceFieldList const& fields, Transform const& transform,
	float radiusInput, float decayInput, ParticleBuffer& buffer) {
	const Vector3 center = transform.GetPosition();
	const bool useRadius = radiusInput >= 0.0f;
	const float scaledRadius = radiusInput * transform.GetScaling().x;

	auto& layout = buffer.GetLayout();
	auto offsets = layout.GetOffsets<layout::Position, layout::Velocity, layout::Force>();
	if (ParticleLayout::HasInvalidOffsets(offsets)) {
		return;
	}

	auto span = buffer.EditableSpan<layout::Position, layout::Velocity, layout::Force>(0, offsets);
	size_t fieldIndex = 0;
	for (auto&& [position, velocity, force] : span) {
		Vector3& vel = velocity.vel;
		Vector3& appliedForce = force.force;

		const float speed = algebra::length(vel);
		if (speed <= 0.0f) {
			continue;
		}

		float factor = 1.0f;
		if (useRadius) {
			Vector3 diff = position.pos - center;
			const float distance = algebra::length(diff);
			if (distance > scaledRadius) {
				++fieldIndex;
				continue;
			}
			factor = std::pow(std::max(0.0f, 1.0f - distance / scaledRadius), decayInput);
			if (factor <= 0.0f) {
				++fieldIndex;
				continue;
			}
		}

		const blender::TurbulenceField& field = fields[fieldIndex % fields.size()];
		Vector3 worldDir = transform.GetTransformMatrix().TransformNormal(field.direction);
		Vector3 turbulence = TurbulenceComponent(worldDir, vel);
		appliedForce += turbulence * (field.forceMagnitude * speed * factor);

		++fieldIndex;
	}
}

} // namespace weave::particles
