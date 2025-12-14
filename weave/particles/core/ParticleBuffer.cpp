#include "ParticleBuffer.h"
#include <cstring>
#include <algorithm>

using namespace weave;
using namespace weave::particles;

ParticleBuffer::ParticleBuffer(size_t amount, ParticleLayout layout)
	: layout(std::move(layout))
{
	ReserveBytes(amount * layout.particleByteSize);
}

void ParticleBuffer::SetLayout(ParticleLayout newLayout) {
	ExpandParticleByteSize(newLayout.particleByteSize);
	layout = std::move(newLayout);
}

uint64_t ParticleBuffer::AddEmissionParticles(uint64_t amount)
{
	uint64_t prevAmount = (memoryBuffer.size() - emissionStartOffset)/layout.particleByteSize;
	memoryBuffer.resize(memoryBuffer.size() + amount * layout.particleByteSize);
	return prevAmount;
}

void ParticleBuffer::KillParticles(uint64_t offsetStart, uint64_t amount)
{
	// A block equivalent to the killed size is copied from the back of the buffer
	// The amount of particles actually killed will be capped to the available ones in the buffer and the copied will be limited to the remaining ones
	std::ptrdiff_t startBytesOffset = offsetStart * layout.particleByteSize;
	auto difference = std::min(size_t(layout.particleByteSize) * amount, size_t(editableEndOffset - startBytesOffset));
	auto copySize = std::min(difference, editableEndOffset - startBytesOffset - difference);

	std::memcpy(memoryBuffer.data() + startBytesOffset, memoryBuffer.data() + editableEndOffset - copySize, copySize);
	editableEndOffset -= difference;
}

void ParticleBuffer::CommitEmittedParticles()
{
	auto difference = emissionStartOffset - editableEndOffset;
    if (difference > 0) {
        const size_t gapBytes = static_cast<size_t>(difference);
        const size_t emissionBytes = memoryBuffer.size() - static_cast<size_t>(emissionStartOffset);
        if (emissionBytes > 0) {
            auto* dest = memoryBuffer.data() + editableEndOffset;
            auto* src = memoryBuffer.data() + emissionStartOffset;
            const size_t copyBytes = std::min(gapBytes, emissionBytes);
            if (copyBytes > 0) {
                std::memcpy(dest, src, copyBytes);
            }
        }
    }

	memoryBuffer.resize(memoryBuffer.size() - difference);

	editableEndOffset = std::ptrdiff_t(memoryBuffer.size());
	emissionStartOffset = editableEndOffset;
}

void ParticleBuffer::ExpandParticleByteSize(size_t size)
{
	if (size <= layout.particleByteSize) {
		return;
	}

	if (memoryBuffer.empty()) {
		return;
	}

	auto newBufferSize = (memoryBuffer.size() / layout.particleByteSize) * size;
	auto oldBufferSize = memoryBuffer.size();

	memoryBuffer.resize(newBufferSize);

	//Backwards copy so that we don't lose any data
	for (std::ptrdiff_t writeIndex = newBufferSize - size, readIndex = oldBufferSize - layout.particleByteSize; writeIndex >= 0; readIndex -= layout.particleByteSize, writeIndex -= size) {
		std::memcpy(memoryBuffer.data() + writeIndex, memoryBuffer.data() + readIndex, layout.particleByteSize);
		std::memset(memoryBuffer.data() + writeIndex + layout.particleByteSize, 0, size - layout.particleByteSize); //zero padding between particles
	}


	editableEndOffset = (editableEndOffset / layout.particleByteSize) * size;
	emissionStartOffset = (emissionStartOffset / layout.particleByteSize) * size;
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

