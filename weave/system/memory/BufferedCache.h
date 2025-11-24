#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace weave {

template<typename Cache>
class BufferedCache final {
public:
	template<typename FilterLambda>
	auto GetCache(FilterLambda const& filter) {
		while (true) {
			uint64_t gen = generation.load(std::memory_order_acquire);
			size_t indexCandidate = (gen & kActiveBufferBit) ? 1 : 0;
			auto candidate = filter(buffers[indexCandidate]); // Retrieval copy, user defined
			if (gen == generation.load(std::memory_order_acquire)) {
				return candidate;
			}
		}
	}

	template<typename CacheOperationLambda>
	void UpdateCache(uint64_t revision, CacheOperationLambda const& operation) const {
		uint64_t gen = generation.load(std::memory_order_acquire);

		// Acquire writing rights
		while (true) {
			gen = generation.load(std::memory_order_acquire);
			if ((gen & kRevisionMask) >= revision) {
				return; // Revision is older than currently recorded, nothing to do
			}

			if (gen & kWritingBit) {
				continue; // Another writer has already acquired writing rights, try again
			}

			uint64_t nextGen = revision;
			nextGen |= (gen & kActiveBufferBit); // Keep the active buffer for readers (will be flipped later when committed)

			if (generation.compare_exchange_weak(gen, nextGen | kWritingBit, std::memory_order_acq_rel, std::memory_order_acquire)) {
				gen = nextGen;
				break; // Buffer slot acquired and locked
			}
		}

		size_t bufferIndex = (gen & kActiveBufferBit) ? 0 : 1;
		auto& targetBuffer = buffers[bufferIndex];

		operation(targetBuffer); // Cache operation, user defined

		gen = (gen ^ kActiveBufferBit); // Flip the active buffer bit
		generation.store(gen, std::memory_order_release);
	}

	private:
		mutable std::atomic_uint64_t generation { 0 };
	mutable Cache buffers[2];

	static constexpr uint64_t kWritingBit      = 1ull << 63;
	static constexpr uint64_t kActiveBufferBit = 1ull << 62;
	static constexpr uint64_t kRevisionMask    = ~(kWritingBit | kActiveBufferBit);
};

}
