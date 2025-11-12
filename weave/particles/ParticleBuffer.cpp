#include "ParticleBuffer.h"
#include <cstring>
#include <algorithm>

using namespace weave;
using namespace weave::particles;

ParticleBuffer::ParticleBuffer(size_t amount, size_t particleByteSize)
	: particleByteSize(particleByteSize)
{
	ReserveBytes(amount * particleByteSize);
}

void ParticleBuffer::ExpandParticleByteSize(size_t size)
{
	if (size <= particleByteSize) {
		return;
	}

	if (memoryBuffer.empty()) {
		particleByteSize = size;
		return;
	}

	auto newBufferSize = (memoryBuffer.size() / particleByteSize) * size;
	auto oldBufferSize = memoryBuffer.size();

	memoryBuffer.resize(newBufferSize);

	//Backwards copy so that we don't lose any data
	for (std::ptrdiff_t writeIndex = newBufferSize - size, readIndex = oldBufferSize - particleByteSize; writeIndex >= 0; readIndex -= particleByteSize, writeIndex -= size) {
		std::memcpy(memoryBuffer.data() + writeIndex, memoryBuffer.data() + readIndex, particleByteSize);
		std::memset(memoryBuffer.data() + writeIndex + particleByteSize, 0, size - particleByteSize); //zero padding between particles
	}


	editableEndOffset = (editableEndOffset / particleByteSize) * size;
	emissionStartOffset = (emissionStartOffset / particleByteSize) * size;

	particleByteSize = size;
}

void ParticleBuffer::ReserveBytes(size_t bytes)
{
	memoryBuffer.reserve(bytes);
	if (editableEndOffset > std::ptrdiff_t(bytes)) {
		editableEndOffset = std::ptrdiff_t(bytes);
	}
	if (emissionStartOffset > std::ptrdiff_t(bytes)) {
		emissionStartOffset = std::ptrdiff_t(bytes);
	}
}

void ParticleBuffer::AddEmissionParticles(uint64_t amount)
{
	memoryBuffer.resize(memoryBuffer.size() + amount * particleByteSize);
}

void ParticleBuffer::KillParticles(uint64_t offsetStart, uint64_t amount)
{
	// A block equivalent to the killed size is copied from the back of the buffer
	// The amount of particles actually killed will be capped to the available ones in the buffer and the copied will be limited to the remaining ones
	std::ptrdiff_t startBytesOffset = offsetStart * particleByteSize;
	auto difference = std::min(size_t(particleByteSize) * amount, size_t(editableEndOffset - startBytesOffset));
	auto copySize = std::min(difference, editableEndOffset - startBytesOffset - difference);

	std::memcpy(memoryBuffer.data() + startBytesOffset, memoryBuffer.data() + editableEndOffset - copySize, copySize);
	editableEndOffset -= difference;
}

void ParticleBuffer::PublishEmittedParticles()
{
	// Particles in the emission section are moved to be packed tightly against the last editable particle, then the buffer is resized downwards
	auto difference = emissionStartOffset - editableEndOffset;
	if (difference > 0) {
		//std::copy(memoryBuffer.data() + emissionStartOffset, memoryBuffer.data() + std::ptrdiff_t(memoryBuffer.size()), memoryBuffer.data() + editableEndOffset);
		std::memcpy(memoryBuffer.data() + editableEndOffset, memoryBuffer.data() + std::ptrdiff_t(memoryBuffer.size() - difference), difference);
	}

	memoryBuffer.resize(memoryBuffer.size() - difference);

	editableEndOffset = std::ptrdiff_t(memoryBuffer.size());
	emissionStartOffset = editableEndOffset;
}
