#include "ParticleTransformInit.h"

#include <algorithm>

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

ParticleTransformInit::ParticleTransformInit() {
	Transform identity{};
	identity.Identity();
	this->input.SetDefaultValues(
		identity,
		true,  // apply on emission range
		false, // apply on editable range
		true,  // translate positions
		true,  // rotate positions
		true,  // scale positions
		true,  // rotate velocities
		true   // scale velocities
	);
}

void ParticleTransformInit::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}

	const Transform transform = this->input.template Ref<TransformInput>();
	const bool applyEmission = this->input.template Ref<ApplyEmission>();
	const bool applyEditable = this->input.template Ref<ApplyEditable>();
	const bool translatePosition = this->input.template Ref<TranslatePosition>();
	const bool rotatePosition = this->input.template Ref<RotatePosition>();
	const bool scalePosition = this->input.template Ref<ScalePosition>();
	const bool rotateVelocity = this->input.template Ref<RotateVelocity>();
	const bool scaleVelocity = this->input.template Ref<ScaleVelocity>();

	const bool modifyPositions = translatePosition || rotatePosition || scalePosition;
	const bool modifyVelocities = rotateVelocity || scaleVelocity;

	if (!modifyPositions && !modifyVelocities) {
		return;
	}

	const Vector3 translation = transform.GetPosition();
	const Vector3 scaling = transform.GetScaling();
	const Quaternion rotation = transform.GetOrientation();
	Matrix4x4 rotationMatrix(rotation);

	auto transformPosition = [&](Vector3& pos) {
		Vector3 result = pos;
		if (scalePosition) {
			result.x *= scaling.x;
			result.y *= scaling.y;
			result.z *= scaling.z;
		}
		if (rotatePosition) {
			result = rotationMatrix.TransformNormal(result);
		}
		if (translatePosition) {
			result += translation;
		}
		pos = result;
	};

	auto transformVelocity = [&](Vector3& vel) {
		Vector3 result = vel;
		if (scaleVelocity) {
			result.x *= scaling.x;
			result.y *= scaling.y;
			result.z *= scaling.z;
		}
		if (rotateVelocity) {
			result = rotationMatrix.TransformNormal(result);
		}
		vel = result;
	};

	if (applyEmission) {
		for (auto const& emissionRange : flow.Iterate<EmissionRange>()) {
			if (!emissionRange) {
				continue;
			}

			auto& layout = emissionRange.GetLayout();

			if (modifyPositions) {
				const auto positionOffsets = layout.GetOffsets<layout::Position>();
				if (!ParticleLayout::HasInvalidOffsets(positionOffsets)) {
					auto emissionSpan = emissionRange.EmissionSpan<layout::Position>(positionOffsets);
					for (auto&& [position] : emissionSpan) {
						transformPosition(position.pos);
					}
				}
			}

			if (modifyVelocities) {
				const auto velocityOffsets = layout.GetOffsets<layout::Velocity>();
				if (!ParticleLayout::HasInvalidOffsets(velocityOffsets)) {
					auto emissionSpan = emissionRange.EmissionSpan<layout::Velocity>(velocityOffsets);
					for (auto&& [velocity] : emissionSpan) {
						transformVelocity(velocity.vel);
					}
				}
			}
		}
	}

	if (applyEditable) {
		for (auto* buffer : flow.Iterate<ParticleBuffer*>()) {
			if (!buffer) {
				continue;
			}

			auto& layout = buffer->GetLayout();

			if (modifyPositions) {
				const auto positionOffsets = layout.GetOffsets<layout::Position>();
				if (!ParticleLayout::HasInvalidOffsets(positionOffsets)) {
					auto editableSpan = buffer->EditableSpan<layout::Position>(0, positionOffsets);
					for (auto&& [position] : editableSpan) {
						transformPosition(position.pos);
					}
				}
			}

			if (modifyVelocities) {
				const auto velocityOffsets = layout.GetOffsets<layout::Velocity>();
				if (!ParticleLayout::HasInvalidOffsets(velocityOffsets)) {
					auto editableSpan = buffer->EditableSpan<layout::Velocity>(0, velocityOffsets);
					for (auto&& [velocity] : editableSpan) {
						transformVelocity(velocity.vel);
					}
				}
			}
		}
	}
}

} // namespace weave::particles
