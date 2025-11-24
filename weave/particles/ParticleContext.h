#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
#include <utility>
#include <weave/particles/ParticleLayout.h>

namespace weave::particles {


class ParticleBuffer;


/**
 * ParticleContext acts as the shared uniform block for blender-based particle nodes.
 * It carries per-iteration sampling data along with the set of buffers the particle
 * machine will operate on.
 */
struct ParticleContext {
	struct SamplingData {
		float deltaTime{ 0.0f };
		uint64_t iteration{ 0 };
		bool isVisible{ true };
	};

	SamplingData sampling{};
	std::vector<ParticleBuffer*> buffers;

	void ClearBuffers() { buffers.clear(); }

	void AddBuffer(ParticleBuffer& buffer) { buffers.push_back(&buffer); }

	void SetBuffers(std::vector<ParticleBuffer*> newBuffers) { buffers = std::move(newBuffers); }
};

} // namespace weave::particles
