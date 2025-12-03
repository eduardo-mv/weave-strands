#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <vector>

#include <weave/particles/core/ParticleLayout.h>

namespace weave::particles {

// particle_span is a specialized span-like class designed for managing particles with
// varying sizes and offsets within a memory buffer. It provides an iterator for easy
// access and modification of particles, as well as a set of utility functions for
// working with the underlying memory.
template<typename... Types>
class particle_span {
public:
    using OffsetArray = std::array<size_t, sizeof...(Types)>;

    particle_span() = default;

    particle_span(std::byte* begin, std::byte* end, size_t particleStride, OffsetArray offsets)
        : data(begin)
        , particleCount(particleStride ? (end - begin) / particleStride : 0)
        , stride(particleStride)
        , cachedOffsets(std::move(offsets)) {}

    struct iterator {
        std::byte* ptr{};
        size_t stride{};
        OffsetArray const* offsets{};

        iterator& operator++() { ptr += stride; return *this; }

        auto operator*() const {
            return deref(ptr, std::index_sequence_for<Types...>{});
        }

        friend bool operator!=(iterator const& a, iterator const& b) { return a.ptr != b.ptr; }

    private:
        template<size_t... I>
        auto deref(std::byte* base, std::index_sequence<I...>) const {
            return std::tie(*reinterpret_cast<Types*>(base + (*offsets)[I])...);
        }
    };

    iterator begin() { return { data, stride, &cachedOffsets }; }
    iterator end()   { return { data + particleCount * stride, stride, &cachedOffsets }; }
    iterator begin() const { return { data, stride, &cachedOffsets }; }
    iterator end()   const { return { data + particleCount * stride, stride, &cachedOffsets }; }

    size_t size() const { return particleCount; }

    auto operator[](size_t i) {
        return deref(data + i * stride, std::index_sequence_for<Types...>{});
    }
    auto operator[](size_t i) const {
        return deref(data + i * stride, std::index_sequence_for<Types...>{});
    }

private:
    template<size_t... I>
    auto deref(std::byte* base, std::index_sequence<I...>) const {
        return std::tie(*reinterpret_cast<Types*>(base + cachedOffsets[I])...);
    }

    std::byte* data{};
    size_t particleCount{};
    size_t stride{};
    OffsetArray cachedOffsets{};
};



class ParticleBuffer {
private:
	std::vector<std::byte> memoryBuffer;
	std::ptrdiff_t editableEndOffset = 0; // Editable part starts at 0 always
	std::ptrdiff_t emissionStartOffset = 0; // Emission part ends at the buffer's tail
	
	ParticleLayout layout;

public:
	ParticleBuffer() = default;
	ParticleBuffer(size_t amount, ParticleLayout layout);

	void SetLayout(ParticleLayout layout);
	ParticleLayout const& GetLayout() const { return layout; }
	ParticleLayout& GetLayout() { return layout; }
	size_t GetParticleByteSize() const { return layout.particleByteSize; }
	size_t GetBufferByteSize() const { return memoryBuffer.size(); }

	void AddEmissionParticles(uint64_t amount);
	void KillParticles(uint64_t offsetStart, uint64_t amount);
	
	//Moves all particles in the emitter section of the buffer into the editable section
	void CommitEmittedParticles();

	template<typename ...Types>
	particle_span<Types...> AddEmissionParticles(uint64_t amount) {
		AddEmissionParticles(amount);
		auto *begin = memoryBuffer.data() + memoryBuffer.size() - amount * layout.particleByteSize;
		auto *end = memoryBuffer.data() + std::ptrdiff_t(memoryBuffer.size());
		return { begin, end, layout.particleByteSize, layout.GetOffsets<Types...>() };
	}

	particle_span<const std::byte> ActiveSpan(size_t offsetItems = 0) {
		auto *begin = memoryBuffer.data() + offsetItems * layout.particleByteSize;
		auto *end = memoryBuffer.data() + editableEndOffset;

		return particle_span<const std::byte>{ begin, end, layout.particleByteSize, ParticleOffsetArray<std::byte>{0} };
	}

	template<typename ...Types>
	particle_span<Types...> EditableSpan(size_t offsetItems = 0) {
		return EditableSpan<Types...>(offsetItems, layout.GetOffsets<Types...>());
	}

	template<typename ...Types>
	particle_span<Types...> EditableSpan(size_t offsetItems, ParticleOffsetArray<Types...> offsets) {
		auto *begin = memoryBuffer.data() + offsetItems * layout.particleByteSize;
		auto *end = memoryBuffer.data() + editableEndOffset;
		return {begin , end, layout.particleByteSize, std::move(offsets)};
	}

	template<typename ...Types>
	particle_span<Types...> EmissionSpan(size_t offsetItems = 0) {
		return EmissionSpan<Types...>(offsetItems, layout.GetOffsets<Types...>());
	}

	template<typename ...Types>
	particle_span<Types...> EmissionSpan(size_t offsetItems, ParticleOffsetArray<Types...> offsets) {
		auto *begin = memoryBuffer.data() + emissionStartOffset + offsetItems * layout.particleByteSize;
		auto *end = memoryBuffer.data() + std::ptrdiff_t(memoryBuffer.size());
		return {begin , end, layout.particleByteSize, std::move(offsets)};
	}

private:
	void ExpandParticleByteSize(size_t size);
	void ReserveBytes(size_t bytes);

};

}
