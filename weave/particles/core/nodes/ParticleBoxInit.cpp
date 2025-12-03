#include "ParticleBoxInit.h"

#include <algorithm>
#include <cmath>

#include "weave/system/math/Random.h"
#include "weave/system/math/VectorMath.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {
namespace {
float RandomSigned(float magnitude) {
	return weave::rng::uniformMirror<float>() * std::abs(magnitude);
}

float FaceCoordinate(float minValue, float diff) {
	const float rand = weave::rng::uniformMirror<float>();
	const float sign = (rand == 0.0f) ? 1.0f : rand;
	return std::copysign(std::abs(rand) * diff + minValue, sign);
}
}

ParticleBoxInit::ParticleBoxInit() {
	this->input.SetDefaultValues(
		0.0f, // min X
		0.0f, // min Y
		0.0f, // min Z
		1.0f, // max X
		1.0f, // max Y
		1.0f  // max Z
	);
}

void ParticleBoxInit::ExecuteNode() {
	auto& context = GetContext();
	if (context.buffers.empty()) {
		return;
	}

	const float minX = this->input.template Ref<MinX>();
	const float minY = this->input.template Ref<MinY>();
	const float minZ = this->input.template Ref<MinZ>();
	const float maxX = this->input.template Ref<MaxX>();
	const float maxY = this->input.template Ref<MaxY>();
	const float maxZ = this->input.template Ref<MaxZ>();

	const float normMinX = std::min(minX, maxX);
	const float normMinY = std::min(minY, maxY);
	const float normMinZ = std::min(minZ, maxZ);
	const float normMaxX = std::max(minX, maxX);
	const float normMaxY = std::max(minY, maxY);
	const float normMaxZ = std::max(minZ, maxZ);

	const float diffX = std::max(0.0f, normMaxX - normMinX);
	const float diffY = std::max(0.0f, normMaxY - normMinY);
	const float diffZ = std::max(0.0f, normMaxZ - normMinZ);

	for (auto* buffer : context.buffers) {
		if (!buffer) {
			continue;
		}

		auto const& layout = buffer->GetLayout();
		const size_t particleSize = layout.particleByteSize;
		if (particleSize == 0) {
			continue;
		}

		const auto layoutPositionOffset = layout.GetOffsets<layout::Position>();
		if (ParticleLayout::HasInvalidOffsets(layoutPositionOffset)) {
			continue;
		}

		if (layoutPositionOffset[0] + sizeof(Vector3) > particleSize) {
			continue;
		}

		for (auto&& [position] : buffer->EmissionSpan<layout::Position>(0, layoutPositionOffset)) {
			Vector3& pos = position.pos;
			switch (faceOut) {
			case 0: {
				pos.x = FaceCoordinate(normMinX, diffX);
				pos.y = RandomSigned(normMaxY);
				pos.z = RandomSigned(normMaxZ);
				break;
			}
			case 1: {
				pos.x = RandomSigned(normMaxX);
				pos.y = FaceCoordinate(normMinY, diffY);
				pos.z = RandomSigned(normMaxZ);
				break;
			}
			case 2:
			default: {
				pos.x = RandomSigned(normMaxX);
				pos.y = RandomSigned(normMaxY);
				pos.z = FaceCoordinate(normMinZ, diffZ);
				break;
			}
			}

			faceOut = (faceOut + 1) % 3;
		}
	}
}

} // namespace weave::particles
