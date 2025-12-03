#include "ParticleLifeAdjustSim.h"

#include <algorithm>

#include "weave/system/math/Interpolation.h"
#include "weave/system/math/Random.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

namespace {
constexpr float kEpsilon = 1e-5f;

float RandomRange(float minValue, float maxValue) {
	if (maxValue <= minValue) {
		return minValue;
	}
	return interpolation::lerp(minValue, maxValue, weave::rng::uniform<float>());
}

void ApplyClamp(float& lifeValue, float maxLife, float lower, float upper, bool clampUpper) {
	if (upper < lower) {
		std::swap(upper, lower);
	}

	if (clampUpper) {
		if (lifeValue > upper) {
			lifeValue = algebra::clamp(RandomRange(lower, upper), 0.0f, maxLife);
		}
	} else {
		if (lifeValue < lower) {
			lifeValue = algebra::clamp(RandomRange(lower, upper), 0.0f, maxLife);
		}
	}
}
} // namespace

ParticleLifeAdjustSim::ParticleLifeAdjustSim() {
	Transform identity{};
	identity.Identity();

	this->input.SetDefaultValues(
		identity,   // transform
		5.0f,       // radius
		0.25f,      // min life
		0.75f,      // max life
		true,       // relative min
		true,       // relative max
		true        // clamp upper by default
	);
}

void ParticleLifeAdjustSim::ExecuteNode() {
	auto& context = GetContext();
	if (context.buffers.empty()) {
		return;
	}

	const float radiusInput = this->input.template Ref<RadiusInput>();
	const float minInput = this->input.template Ref<MinLifeInput>();
	const float maxInput = this->input.template Ref<MaxLifeInput>();
	const bool relativeMin = this->input.template Ref<RelativeMinInput>();
	const bool relativeMax = this->input.template Ref<RelativeMaxInput>();
	const bool clampUpper = this->input.template Ref<ClampUpperInput>();

	if (radiusInput < 0.0f) {
		AdjustAllParticles(minInput, maxInput, relativeMin, relativeMax, clampUpper);
	} else {
		const float radiusSq = radiusInput * radiusInput;
		AdjustParticlesInRadius(radiusSq, minInput, maxInput, relativeMin, relativeMax, clampUpper);
	}
}

void ParticleLifeAdjustSim::AdjustAllParticles(float minInput, float maxInput, bool relativeMin, bool relativeMax, bool clampUpper) {
	auto& context = GetContext();

	for (auto* buffer : context.buffers) {
		if (!buffer) {
			continue;
		}

		auto& layout = buffer->GetLayout();
		auto offsets = layout.GetOffsets<layout::LifeTime, layout::MaxLifeTime>();
		if (ParticleLayout::HasInvalidOffsets(offsets)) {
			continue;
		}

		auto span = buffer->EditableSpan<layout::LifeTime, layout::MaxLifeTime>(0, offsets);
		for (auto&& [life, maxLife] : span) {
			float lower = relativeMin ? minInput * maxLife.maxLifeTime : minInput;
			float upper = relativeMax ? maxInput * maxLife.maxLifeTime : maxInput;
			ApplyClamp(life.lifeTime, maxLife.maxLifeTime, lower, upper, clampUpper);
		}
	}
}

void ParticleLifeAdjustSim::AdjustParticlesInRadius(float radiusSq,
	float minInput, float maxInput, bool relativeMin, bool relativeMax, bool clampUpper) {
	const Transform transform = this->input.template Ref<TransformInput>();
	const Vector3 center = transform.GetPosition();

	auto& context = GetContext();

	for (auto* buffer : context.buffers) {
		if (!buffer) {
			continue;
		}

		auto& layout = buffer->GetLayout();
		auto offsets = layout.GetOffsets<layout::Position, layout::LifeTime, layout::MaxLifeTime>();
		if (ParticleLayout::HasInvalidOffsets(offsets)) {
			continue;
		}

		auto span = buffer->EditableSpan<layout::Position, layout::LifeTime, layout::MaxLifeTime>(0, offsets);
		for (auto&& [position, life, maxLife] : span) {
			Vector3 diff = position.pos - center;
			if (algebra::lengthSqr(diff) > radiusSq) {
				continue;
			}

			float lower = relativeMin ? minInput * maxLife.maxLifeTime : minInput;
			float upper = relativeMax ? maxInput * maxLife.maxLifeTime : maxInput;
			ApplyClamp(life.lifeTime, maxLife.maxLifeTime, lower, upper, clampUpper);
		}
	}
}

} // namespace weave::particles
