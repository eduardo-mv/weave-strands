#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <span>

namespace weave::particles {

// particle_span is a specialized span-like class designed for managing particles with
// varying sizes and offsets within a memory buffer. It provides an iterator for easy
// access and modification of particles, as well as a set of utility functions for
// working with the underlying memory.
template<typename T>
struct particle_span {
	using PtrType = std::conditional_t<std::is_const<T>::value, std::byte const*, std::byte*>;
	PtrType data{};
	size_t particleCount{};
	size_t particleByteSize{};
	size_t particleByteOffset{};

	particle_span() = default;
	particle_span(particle_span const&) = default;
	particle_span(particle_span&&) = default;
	particle_span& operator=(particle_span const&) = default;
	particle_span& operator=(particle_span&&) = default;

	particle_span(T* data, size_t size, size_t particleByteSize, size_t particleByteOffset)
		: data(reinterpret_cast<PtrType>(data))
		, particleCount(size)
		, particleByteSize(particleByteSize)
		, particleByteOffset(particleByteOffset) {}

	particle_span(T* data, T* end, size_t particleByteSize, size_t particleByteOffset)
		: data(reinterpret_cast<PtrType>(data))
		, particleCount((reinterpret_cast<PtrType>(end) - reinterpret_cast<PtrType>(data)) / particleByteSize)
		, particleByteSize(particleByteSize) 
		, particleByteOffset(particleByteOffset) {}

	struct iterator {
		T* object{};
		size_t particleByteSize{};

		void operator++() { object = reinterpret_cast<T*>(reinterpret_cast<std::byte*>(object) + particleByteSize); }
		T& operator*() { return *object; }
		T const& operator*() const { return *object; }

		friend bool operator!=(iterator const& a, iterator const& b) {
			return a.object != b.object;
		}
	};

	iterator begin() const { return { reinterpret_cast<T*>(data + particleByteOffset), particleByteSize }; }
	iterator end() const { return { reinterpret_cast<T*>(data + particleByteOffset + particleCount * particleByteSize), particleByteSize }; }

	iterator begin() { return { reinterpret_cast<T*>(data + particleByteOffset), particleByteSize }; }
	iterator end() { return { reinterpret_cast<T*>(data + particleByteOffset + particleCount * particleByteSize), particleByteSize }; }

	size_t size() const { return particleCount; }

	template<typename Int>
	T& operator[](Int i) { return *reinterpret_cast<T*>(data + particleByteOffset + i * particleByteSize); }
	template<typename Int>
	T const& operator[](Int i) const { return *reinterpret_cast<T const*>(data + particleByteOffset + i * particleByteSize); }

	template<typename CastedType>
	CastedType& CastOffset(void* p, size_t byteOffset) { return *reinterpret_cast<CastedType*>(reinterpret_cast<std::byte*>(p) + byteOffset); }
};


class ParticleBuffer {
private:
	std::vector<std::byte> memoryBuffer;
	std::ptrdiff_t editableEndOffset = 0; // Editable part starts at 0 always
	std::ptrdiff_t emissionStartOffset = 0; // Emission part ends at the buffer's tail

	// The expected size of the particles stored in the buffer. 
	size_t particleByteSize = 1;

public:
	ParticleBuffer() = default;
	ParticleBuffer(size_t amount, size_t particleByteSize);

	void ExpandParticleByteSize(size_t size);
	size_t GetParticleByteSize() const { return particleByteSize; }

	size_t GetBufferByteSize() const { return memoryBuffer.size(); }
	void ReserveBytes(size_t bytes);

	void AddEmissionParticles(uint64_t amount);
	void KillParticles(uint64_t offsetStart, uint64_t amount);
	
	//Moves all particles in the emitter section of the buffer into the editable section
	void PublishEmittedParticles();

	template<typename T>
	particle_span<T> AddEmissionParticles(uint64_t amount) {
		AddEmissionParticles(amount);
		return particle_span<T>{ reinterpret_cast<T*>(memoryBuffer.data() + memoryBuffer.size() - amount * particleByteSize), reinterpret_cast<T*>(memoryBuffer.data() + std::ptrdiff_t(memoryBuffer.size())), particleByteSize, 0 };
	}

	particle_span<const std::byte> ReadableBuffer(size_t offsetItems = 0) const {
		return particle_span<const std::byte>{ reinterpret_cast<std::byte const*>(memoryBuffer.data() + offsetItems * particleByteSize), reinterpret_cast<std::byte const*>(memoryBuffer.data() + editableEndOffset), particleByteSize, 0 };
	}

	template<typename T = std::byte>
	particle_span<T> EditableBuffer(size_t offsetItems = 0, size_t particleByteOffset = 0) {
		return particle_span<T>{ reinterpret_cast<T*>(memoryBuffer.data() + offsetItems * particleByteSize), reinterpret_cast<T*>(memoryBuffer.data() + editableEndOffset), particleByteSize, particleByteOffset };
	}

	template<typename T = std::byte>
	particle_span<T> EmissionBuffer(size_t offsetItems = 0, size_t particleByteOffset = 0) {
		return particle_span<T>{ reinterpret_cast<T*>(memoryBuffer.data() + emissionStartOffset + offsetItems * particleByteSize), reinterpret_cast<T*>(memoryBuffer.data() + std::ptrdiff_t(memoryBuffer.size())), particleByteSize, particleByteOffset };
	}

};

}