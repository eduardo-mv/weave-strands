#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <vector>
#include <utility>
#include <weave/particles/core/ParticleLayout.h>
#include <weave/particles/core/ParticleBuffer.h>

namespace weave::particles {

struct EmissionRange {
	ParticleBuffer* buffer{};
	uint64_t offsetItems{};
	uint64_t countItems{};

	operator bool() const { return buffer != nullptr; }

	template<typename ...Types>
	auto EmissionSpan() const { 
		return buffer->EmissionSpan<Types...>(offsetItems, countItems);
	}

	template<typename ...Types>
	auto EmissionSpan(ParticleOffsetArray<Types...> offsets) const { 
		return buffer->EmissionSpan<Types...>(offsetItems, countItems, offsets);
	}

	auto const& GetLayout() const {
		return buffer->GetLayout();
	}
};

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
	std::vector<std::shared_ptr<ParticleBuffer>> buffers;


public:
	void ClearBuffers() { buffers.clear(); }

	void AddBuffer(std::shared_ptr<ParticleBuffer> buffer) { 
		buffers.emplace_back(std::move(buffer)); 
	}

	void SetBuffers(std::vector<std::shared_ptr<ParticleBuffer>> newBuffers) { 
		buffers = std::move(newBuffers);
	}

	size_t BufferCount() const { return buffers.size(); }
};

} // namespace weave::particles
