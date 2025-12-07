#include "ParticlePhysicsSim.h"

#include <algorithm>
#include <cmath>

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

ParticlePhysicsSim::ParticlePhysicsSim() {
	this->input.SetDefaultValues(
		0.995f,                 // linear damping base
		Vector3{ 0.0f, -9.8f, 0.0f } // gravity
	);
}

void ParticlePhysicsSim::ExecuteNode() {
	auto& context = GetContext();
	auto* buffer = context.GetCurrentBuffer();
	if (!buffer) {
		return;
	}

	const float deltaTime = context.sampling.deltaTime;
	if (deltaTime <= 0.0f) {
		return;
	}

	auto& layout = buffer->GetLayout();
	auto offsets = layout.GetOffsets<layout::Position, layout::Velocity, layout::Force, layout::Mass>();
	if (ParticleLayout::HasInvalidOffsets(offsets)) {
		return;
	}

	const float dampingBase = std::clamp(this->input.template Ref<LinearDamping>(), 0.0f, 1.0f);
	const float damping = dampingBase > 0.0f ? std::pow(dampingBase, deltaTime) : 0.0f;
	const Vector3 gravity = this->input.template Ref<Gravity>() * deltaTime;

	auto editableSpan = buffer->EditableSpan<layout::Position, layout::Velocity, layout::Force, layout::Mass>(0, offsets);
	for (auto&& [position, velocity, force, mass] : editableSpan) {
		Vector3& pos = position.pos;
		Vector3& vel = velocity.vel;
		Vector3& appliedForce = force.force;
		const float particleMass = std::max(mass.mass, 0.0001f);

		pos += vel * deltaTime;

		const Vector3 acceleration = appliedForce * (deltaTime / particleMass) + gravity;
		vel = (vel + acceleration) * damping;

		appliedForce = Vector3{};
	}
}

} // namespace weave::particles
