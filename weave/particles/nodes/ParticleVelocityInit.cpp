#include "ParticleVelocityInit.h"

#include <algorithm>

#include "weave/system/math/Interpolation.h"
#include "weave/system/math/Random.h"

#include "weave/particles/ParticleBuffer.h"

namespace weave::particles {

ParticleVelocityInit::ParticleVelocityInit() {
	this->input.SetDefaultValues(
		Vector3{ 0.0f, 0.0f, 1.0f }, // direction
		0.0f, // up angle (degrees)
		0.0f, // side angle (degrees)
		0.0f, // min velocity
		1.0f  // max velocity
	);
}

void ParticleVelocityInit::ExecuteNode() {
	auto& context = GetContext();
	if (context.buffers.empty()) {
		return;
	}

	const Vector3 directionInput = algebra::normalize(this->input.template Ref<Direction>());
	const float upAngle = algebra::deg2rad(std::max(0.0f, this->input.template Ref<UpAngle>())) * 0.5f;
	const float sideAngle = algebra::deg2rad(std::max(0.0f, this->input.template Ref<SideAngle>())) * 0.5f;
	const float minVel = std::max(0.0f, this->input.template Ref<MinVelocity>());
	const float maxVel = std::max(minVel, this->input.template Ref<MaxVelocity>());
	const float velDiff = maxVel - minVel;

	for (auto* buffer : context.buffers) {
		if (!buffer) {
			continue;
		}

		auto& layout = buffer->GetLayout();
		const size_t velocityOffset = layout.GetOffset<layout::Velocity>();
		if (velocityOffset == ParticleLayout::kInvalidOffset) {
			continue;
		}

		Vector3 up(0.0f, 1.0f, 0.0f);
		if (std::abs(algebra::dot(directionInput, up)) >= 1.0f) {
			up = { 0.0f, 0.0f, 1.0f };
		}
		Vector3 side = algebra::orthogonalize(directionInput, up);

		const Vector3 limitUpA = Quaternion(upAngle, side, false) * directionInput;
		const Vector3 limitUpB = algebra::reflect(-limitUpA, directionInput);
		const Vector3 limitSideA = Quaternion(sideAngle, up, false) * directionInput;
		const Vector3 limitSideB = algebra::reflect(-limitSideA, directionInput);

		auto emissionSpan = buffer->EmissionSpan<layout::Velocity>();
		for (auto&& [vel] : emissionSpan) {
			const float tA = weave::rng::uniform<float>();
			const float tB = weave::rng::uniform<float>();
			const float tC = weave::rng::uniform<float>();

			Vector3 dir = interpolation::lerp(
				interpolation::lerp(limitUpA, limitUpB, tA),
				interpolation::lerp(limitSideA, limitSideB, tB),
				tC);

			float magnitude = algebra::length(dir);
			if (magnitude == 0.0f) {
				dir = directionInput;
				magnitude = 1.0f;
			}

			const float finalMagnitude = weave::rng::uniform<float>() * velDiff + minVel;
			vel.vel = dir * (finalMagnitude / magnitude);
		}
	}
}

} // namespace weave::particles
