#include "ParticleDragSim.h"

#include <algorithm>
#include <limits>

#include "weave/system/math/VectorMath.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

ParticleDragSim::ParticleDragSim() {
	this->input.SetDefaultValues(
		1.0f,  // linear drag (k1)
		0.0f,  // exponential drag (k2)
		0.0f,  // min velocity cap
		-1.0f, // max velocity cap (negative disables)
		0.0f   // break velocity
	);
}

void ParticleDragSim::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}

	const float linearDrag = std::max(0.0f, this->input.template Ref<LinearDrag>());
	const float exponentialDrag = std::max(0.0f, this->input.template Ref<ExponentialDrag>());
	const float minCap = std::max(0.0f, this->input.template Ref<MinVelocityCap>());
	const float maxCapInput = this->input.template Ref<MaxVelocityCap>();
	const float maxCap = maxCapInput < 0.0f ? std::numeric_limits<float>::max() : maxCapInput;
	const float breakCap = std::max(0.0f, this->input.template Ref<BreakVelocity>());

	for (auto* buffer : flow.Iterate<ParticleBuffer*>()) {
		if (!buffer) {
			continue;
		}

		auto& layout = buffer->GetLayout();
		auto offsets = layout.GetOffsets<layout::Velocity, layout::Force>();
		if (ParticleLayout::HasInvalidOffsets(offsets)) {
			return;
		}

		auto editableSpan = buffer->EditableSpan<layout::Velocity, layout::Force>(0, offsets);
		for (auto&& [velocity, force] : editableSpan) {
			Vector3& vel = velocity.vel;
			Vector3& appliedForce = force.force;

			float speed = algebra::length(vel);
			if (speed < breakCap) {
				vel = Vector3{};
				continue;
			}

			if (speed > 0.0f) {
				if (speed > maxCap) {
					vel *= (maxCap / speed);
					speed = maxCap;
				}
				if (speed < minCap) {
					vel *= (minCap / speed);
					speed = minCap;
				}

				const float drag = -(linearDrag * speed + exponentialDrag * speed * speed) / speed;
				appliedForce += vel * drag;
			}
		}
	}
}

} // namespace weave::particles
