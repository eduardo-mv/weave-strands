#include "ParticleRngVelocityInit.h"

#include <algorithm>

#include "weave/system/math/Random.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

namespace {
Vector3 RandomDirection() {
	Vector3 dir{
		weave::rng::uniformMirror<float>(),
		weave::rng::uniformMirror<float>(),
		weave::rng::uniformMirror<float>()
	};
	return algebra::normalize(dir);
}

float ApplyLimit(int limitFlag, float value) {
	if (limitFlag == 0) {
		return value;
	}
	if (limitFlag > 0) {
		return std::abs(value);
	}
	return -std::abs(value);
}
}

ParticleRngVelocityInit::ParticleRngVelocityInit() {
	this->input.SetDefaultValues(
		0.0f, // min velocity
		1.0f, // max velocity
		0,    // limit X
		0,    // limit Y
		0,    // limit Z
		false // radial flag
	);
}

void ParticleRngVelocityInit::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}

	const float minVel = std::max(0.0f, this->input.template Ref<MinVelocity>());
	const float maxVel = std::max(minVel, this->input.template Ref<MaxVelocity>());
	const float velDiff = maxVel - minVel;
	const int limitX = this->input.template Ref<LimitX>();
	const int limitY = this->input.template Ref<LimitY>();
	const int limitZ = this->input.template Ref<LimitZ>();
	const bool radial = this->input.template Ref<Radial>();
	const bool hasLimits = limitX || limitY || limitZ;

	for (auto buffer : context.IterateEmissionRanges(triggerCountTarget)) {
		if (!buffer) {
			continue;
		}

		auto& layout = buffer.GetLayout();
		auto offsets = layout.GetOffsets<layout::Position, layout::Velocity>();
		if (ParticleLayout::HasInvalidOffsets(offsets)) {
			continue;
		}

		auto emissionSpan = buffer.EmissionSpan<layout::Position, layout::Velocity>(offsets);
		for (auto&& [pos, vel] : emissionSpan) {
			Vector3 direction{};

			if (radial) {
				const float magnitude = algebra::length(pos.pos);
				if (magnitude == 0.0f) {
					direction = RandomDirection();
				} else {
					direction = pos.pos / magnitude;
				}
			} else {
				direction = RandomDirection();
				if (hasLimits) {
					direction.x = ApplyLimit(limitX, direction.x);
					direction.y = ApplyLimit(limitY, direction.y);
					direction.z = ApplyLimit(limitZ, direction.z);
					direction = algebra::normalize(direction);
				}
			}

			const float magnitude = weave::rng::uniform<float>() * velDiff + minVel;
			vel.vel = direction * magnitude;
		}
	}
}

} // namespace weave::particles
