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

private:
	struct EmissionRange {
		ParticleBuffer* buffer{};
		uint64_t offsetItems{};
		uint64_t countItems{};

		operator bool() const { return buffer != nullptr; }

		template<typename ...Types>
		auto EmissionSpan() { 
			return buffer->EmissionSpan<Types...>(offsetItems, countItems);
		}

		template<typename ...Types>
		auto EmissionSpan(ParticleOffsetArray<Types...> offsets) { 
			return buffer->EmissionSpan<Types...>(offsetItems, countItems, offsets);
		}

		auto const& GetLayout() const {
			return buffer->GetLayout();
		}
	};

	struct BufferData {
		std::shared_ptr<ParticleBuffer> buffer;
		std::vector<uint64_t> emissionStack;
		uint64_t totalEmitted{ 0 };

		void CommitEmission() {
			buffer->CommitEmittedParticles();
			totalEmitted = 0;
			emissionStack.clear();
		}

		void PushEmission(uint64_t amount) {
			if(buffer) {
				if(amount > 0) {
					buffer->AddEmissionParticles(amount);
				}
				emissionStack.emplace_back(amount);
				totalEmitted += amount;
			}
		}

		EmissionRange GetEmissionRange(size_t backSteps) {
			if(!buffer || emissionStack.empty() || backSteps == 0) {
				return EmissionRange{ buffer.get(), 0, 0 };
			}

			backSteps = std::min(backSteps, emissionStack.size());
			const size_t mergeStart = emissionStack.size() - backSteps;
			uint64_t amount = 0;

			for(size_t i = mergeStart; i < emissionStack.size(); ++i) {
				amount += emissionStack[i];
			}

			emissionStack[mergeStart] = amount;
			emissionStack.resize(mergeStart + 1);

			const uint64_t startOffset = totalEmitted - amount;
			return EmissionRange{ buffer.get(), startOffset, amount };
		}
	};

	

	std::vector<BufferData> buffers;

public:
	void ClearBuffers() { buffers.clear(); }

	void AddBuffer(std::shared_ptr<ParticleBuffer> buffer) { 
		buffers.emplace_back(std::move(buffer), std::vector<uint64_t>{}); 
	}

	void SetBuffers(std::vector<std::shared_ptr<ParticleBuffer>> newBuffers) { 
		ClearBuffers();
		for(auto & buffer : newBuffers) {
			AddBuffer(std::move(buffer));
		}
	}

	size_t BufferCount() const { return buffers.size(); }

	void AddEmissionParticles(uint64_t amount) {
		for(auto& buffer : buffers){
			buffer.PushEmission(amount);
		}
	}

	void CommitEmissionparticles() {
		for(auto& buffer : buffers){
			buffer.CommitEmission();
		}
	}

	auto IterateEmissionRanges(size_t backSteps) {
		return buffers | std::views::transform([backSteps](BufferData& data) {
			return data.GetEmissionRange(backSteps);
		});
	}

	auto IterateSimulationRanges() {
		return buffers | std::views::transform([](BufferData& data) {
			return data.buffer.get();
		});
	}
};

} // namespace weave::particles
