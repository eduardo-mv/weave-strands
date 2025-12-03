#include "ParticleAttractorSim.h"

#include <algorithm>
#include <cmath>

#include "weave/system/math/VectorMath.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

namespace {
constexpr float kEpsilon = 1e-5f;

Vector3 NormalizeSafe(Vector3 const& v) {
	const float len = algebra::length(v);
	if (len <= kEpsilon) {
		return Vector3{};
	}
	return v / len;
}
}

ParticleAttractorSim::ParticleAttractorSim() {
	Transform identity{};
	identity.Identity();

	this->input.SetDefaultValues(
		identity,         // transform
		1.0f,             // force magnitude
		0.0f,             // gravity scale
		5.0f,             // radius
		1.0f,             // decay
		Vector3{}         // direction (zero => spherical)
	);
}

void ParticleAttractorSim::ExecuteNode() {
	auto& context = GetContext();
	if (context.buffers.empty()) {
		return;
	}

	const Transform attractor = this->input.template Ref<AttractorTransform>();
	const Vector3 attractorPos = attractor.GetPosition();
	const float attractorScale = std::max(kEpsilon, attractor.GetScaling().x);

	const float forceMagnitude = this->input.template Ref<ForceMagnitude>();
	const float gravityScale = this->input.template Ref<GravityScale>();
	const float radius = std::max(kEpsilon, this->input.template Ref<Radius>() * attractorScale);
	const float decay = std::max(0.0f, this->input.template Ref<Decay>());
	const Vector3 directionInput = this->input.template Ref<Direction>();

	const bool directional = std::abs(directionInput.x) > kEpsilon ||
		std::abs(directionInput.y) > kEpsilon ||
		std::abs(directionInput.z) > kEpsilon;

	Vector3 worldDirection{};
	if (directional) {
		worldDirection = attractor.GetTransformMatrix().TransformNormal(directionInput);
	}

	for (auto* buffer : context.buffers) {
		if (!buffer) {
			continue;
		}

		auto& layout = buffer->GetLayout();
		auto offsets = layout.GetOffsets<layout::Position, layout::Force, layout::Mass>();
		if (ParticleLayout::HasInvalidOffsets(offsets)) {
			continue;
		}

		auto editableSpan = buffer->EditableSpan<layout::Position, layout::Force, layout::Mass>(0, offsets);
		for (auto&& [position, force, mass] : editableSpan) {
			Vector3& pos = position.pos;
			Vector3& appliedForce = force.force;
			const float particleMass = mass.mass;

			if (!directional) {
				Vector3 diff = pos - attractorPos;
				const float distance = algebra::length(diff);
				if (distance <= kEpsilon) {
					continue;
				}

				const float normalized = std::max(0.0f, 1.0f - distance / radius);
				const float magnitude = decay > 0.0f ? std::pow(normalized, decay) : normalized;
				if (magnitude <= 0.0f) {
					continue;
				}

				const float totalForce = -(forceMagnitude + gravityScale * particleMass) * magnitude / distance;
				appliedForce += diff * totalForce;
			} else {
				Vector3 dir = NormalizeSafe(worldDirection);
				if (algebra::length(dir) <= kEpsilon) {
					continue;
				}

				float magnitude = 1.0f;
				if (decay > 0.0f) {
					const float distance = algebra::length(pos - attractorPos);
					const float normalized = std::max(0.0f, 1.0f - distance / radius);
					magnitude = std::pow(normalized, decay);
				}

				if (magnitude <= 0.0f) {
					continue;
				}

				appliedForce += dir * ((forceMagnitude + gravityScale * particleMass) * magnitude);
			}
		}
	}
}

} // namespace weave::particles
