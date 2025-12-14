#include "ParticleReposition.h"

#include <algorithm>

#include "weave/system/math/VectorMath.h"
#include "weave/system/math/Interpolation.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

ParticleReposition::ParticleReposition() {
	this->input.SetDefaultValues(
		0.0f,  // delayed init
		0.1f,  // idle distance
		1.5f,  // force field distance
		0.5f,  // velocity field distance
		1.0f,  // force magnitude
		2.0f,  // idle-to-active time
		0.2f,  // active-to-idle time
		0.8f   // damping
	);
}

void ParticleReposition::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}

	const float delayedInit = this->input.template Ref<DelayedInit>();
	const float idleDistance = std::max(0.0f, this->input.template Ref<IdleDistance>());
	const float forceFieldDistance = std::max(0.0001f, this->input.template Ref<ForceFieldDistance>());
	const float velocityFieldDistance = std::max(0.0001f, this->input.template Ref<VelocityFieldDistance>());
	const float forceMagnitude = this->input.template Ref<ForceMagnitude>();
	const float idleToActiveTime = std::max(0.0f, this->input.template Ref<IdleToActiveTime>());
	const float activeToIdleTime = std::max(0.0f, this->input.template Ref<ActiveToIdleTime>());
	const float damping = std::clamp(this->input.template Ref<Damping>(), 0.0f, 1.0f);

	for (auto const& emissionRange : flow.Iterate<EmissionRange>()) {
		if (!emissionRange) {
			continue;
		}
		ProcessEmissionRange(emissionRange, delayedInit, idleToActiveTime);
	}

	for (auto* buffer : flow.Iterate<ParticleBuffer*>()) {
		if (!buffer) {
			continue;
		}
		ProcessEditableBuffer(buffer, delayedInit, idleDistance, forceFieldDistance, velocityFieldDistance,
			forceMagnitude, idleToActiveTime, activeToIdleTime, damping);
	}
}

void ParticleReposition::ProcessEmissionRange(EmissionRange const& emissionRange, float delayedInit, float idleToActiveTime) {
	auto& layout = emissionRange.GetLayout();
	auto offsets = layout.GetOffsets<layout::Position, layout::LifeTime, layout::Target>();
	if (ParticleLayout::HasInvalidOffsets(offsets)) {
		return;
	}

	auto emissionSpan = emissionRange.EmissionSpan<layout::Position, layout::LifeTime, layout::Target>(offsets);
	for (auto&& [position, lifeTime, target] : emissionSpan) {
		target.time = idleToActiveTime;
		if (delayedInit == 0.0f) {
			target.targetPosition = position.pos;
		}
	}
}

void ParticleReposition::ProcessEditableBuffer(ParticleBuffer* buffer, float delayedInit, float idleDistance, float forceFieldDistance,
	float velocityFieldDistance, float forceMagnitude, float idleToActiveTime,
	float activeToIdleTime, float damping) {
	auto& context = GetContext();
	const float deltaTime = std::max(0.0f, context.sampling.deltaTime);

	if (!buffer) {
		return;
	}

	auto& layout = buffer->GetLayout();
	auto offsets = layout.GetOffsets<
		layout::Position,
		layout::LifeTime,
		layout::Velocity,
		layout::Force,
		layout::Target>();

	if (ParticleLayout::HasInvalidOffsets(offsets)) {
		return;
	}

	auto editableSpan = buffer->EditableSpan<
		layout::Position,
		layout::LifeTime,
		layout::Velocity,
		layout::Force,
		layout::Target>(0, offsets);

	for (auto&& [position, lifeTime, velocity, force, target] : editableSpan) {
		Vector3& pos = position.pos;
		const float life = lifeTime.lifeTime;
		Vector3& vel = velocity.vel;
		Vector3& appliedForce = force.force;
		Vector3& targetPos = target.targetPosition;
		float& timer = target.time;

		if (delayedInit > 0.0f && life < delayedInit) {
			targetPos = pos;
		}

		Vector3 dir = targetPos - pos;
		const float distance = algebra::length(dir);

		if (distance > idleDistance) {
			if (life >= timer && distance > 0.0f) {
				const float mForce = std::min(1.0f, distance / forceFieldDistance);
				appliedForce += (mForce * forceMagnitude / distance) * dir;

				const float eForce = std::min(1.0f, distance / velocityFieldDistance);
				const float currentSpeed = algebra::length(vel);
				Vector3 blended = interpolation::lerp(algebra::normalizesafe(dir), algebra::normalizesafe(vel), eForce);
				vel = algebra::normalizesafe(blended) * currentSpeed;
			}
		}
		else {
			if (timer < 0.0f) {
				timer += deltaTime;
				vel *= damping;
				if (timer > 0.0f) {
					timer = life + idleToActiveTime;
				}
			}
			else if (life >= timer) {
				timer = -activeToIdleTime;
			}
			else {
				timer = std::max(timer, life + idleToActiveTime);
			}
		}
	}
}

} // namespace weave::particles
