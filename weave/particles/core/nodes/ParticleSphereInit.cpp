#include "ParticleSphereInit.h"

#include <algorithm>
#include <cmath>

#include "weave/system/math/VectorMath.h"
#include "weave/system/math/Random.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

namespace {

float RandomAngle(float minAngle, float maxAngle) {
	return weave::rng::uniform<float>() * (maxAngle - minAngle) + minAngle;
}

float RandomRadius(float minRadius, float maxRadius) {
	return weave::rng::uniform<float>() * (maxRadius - minRadius) + minRadius;
}
}

ParticleSphereInit::ParticleSphereInit() {
	this->input.SetDefaultValues(
		0.0f,  // min radius
		1.0f,  // max radius
		weave::algebra::F_2PI, // alpha angle
		weave::algebra::F_2PI  // beta angle
	);
}

void ParticleSphereInit::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}

	float minRadiusInput = this->input.template Ref<MinRadius>();
	float maxRadiusInput = this->input.template Ref<MaxRadius>();
	float alphaInput = this->input.template Ref<Alpha>();
	float betaInput = this->input.template Ref<Beta>();

	const float minRadius = std::min(minRadiusInput, maxRadiusInput);
	const float maxRadius = std::max(minRadiusInput, maxRadiusInput);

	const float alphaHalf = std::clamp(alphaInput, 0.0f, weave::algebra::F_2PI);
	const float betaHalf = std::clamp(betaInput, 0.0f, weave::algebra::F_2PI);
	const float minAlpha = -alphaHalf * 0.5f;
	const float minBeta = -betaHalf * 0.5f;

	auto emissionRange = context.GetCurrentEmissionRange(triggerCountTarget);
	if (!emissionRange) {
		return;
	}

	auto const& layout = emissionRange.GetLayout();
	const size_t particleSize = layout.particleByteSize;
	if (particleSize == 0) {
		return;
	}

	const auto layoutPositionOffset = layout.GetOffsets<layout::Position>();
	if (ParticleLayout::HasInvalidOffsets(layoutPositionOffset)) {
		return;
	}

	if (layoutPositionOffset[0] + sizeof(Vector3) > particleSize) {
		return;
	}

	for (auto&& [position] : emissionRange.EmissionSpan<layout::Position>(layoutPositionOffset)) {
		Vector3& pos = position.pos;
		const float radius = RandomRadius(minRadius, maxRadius);
		const float theta = RandomAngle(minAlpha, minAlpha + alphaHalf);
		const float phi = RandomAngle(minBeta, minBeta + betaHalf);

		const float sinTheta = std::sin(theta);
		pos.x = radius * sinTheta * std::cos(phi);
		pos.y = radius * sinTheta * std::sin(phi);
		pos.z = radius * std::cos(theta);

		if (alphaHemisphere) {
			pos.x = std::abs(pos.x);
		}
		if (betaHemisphere) {
			pos.y = std::abs(pos.y);
		}
	}
}

} // namespace weave::particles
