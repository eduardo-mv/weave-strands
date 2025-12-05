#include "GpuParticleSnapshotBackend.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <thread>

namespace weave::particles::gl {

namespace {
size_t SafeByteSize(size_t count, size_t stride) {
    if (count == 0 || stride == 0) {
        return 0;
    }

    const size_t maxCount = std::numeric_limits<size_t>::max() / stride;
    return std::min(count, maxCount) * stride;
}
} // namespace

GpuParticleSnapshotBackend::GpuParticleSnapshotBackend(size_t particleStrideBytes)
    : particleStride(particleStrideBytes) {
    assert(particleStride > 0 && "Particle stride must be greater than zero");
    writeOffset.fill(0);
}

std::pair<bool, std::string> GpuParticleSnapshotBackend::UploadEntry(Payload&& payload) {

    if(auto *fence = std::get_if<Fence>(&payload)) {
        return ProcessFence(*fence);
    }
    else {
        ParticleSpan span;

        if (auto *payloadSpan = std::get_if<ParticleSpan>(&payload)) {
            span = *payloadSpan;
        }
        else if (auto *payloadBuffer = std::get_if<std::shared_ptr<const ParticleBuffer>>(&payload)) {
            span = { (**payloadBuffer).ActiveByteSpan(), (**payloadBuffer).GetParticleByteSize() };
        }
        else {
            return {false, "Invalid particle payload"};
        }

        return ProcessSpan(span);
    }
}

void GpuParticleSnapshotBackend::ReserveBytes(size_t byteCount) {
    using namespace weave::opengl;

    if (byteCount == 0) {
        return;
    }

    for(auto &buffer : storageBuffers) {
        buffer.GrowBufferToFit(byteCount);
    }
}

void GpuParticleSnapshotBackend::ReserveParticles(size_t particleCount) {
    const size_t bytes = SafeByteSize(particleCount, particleStride);
    ReserveBytes(bytes);
}

std::pair<weave::opengl::Buffer const&, size_t> GpuParticleSnapshotBackend::GetReadSnapshot() const { 
    uint32_t readTarget = readSnapshotIndex.load(std::memory_order::acquire);
    uint32_t writeTarget = 0;

    while(true) {
        writeTarget = writeSnapshotIndex.load(std::memory_order::acquire);

        if(readTarget != writeTarget) {
            break;
        }

        // Spinlock, we're overlapping read and write buffers
        // The triple buffering should make this case a very rare one
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    readTarget = readTarget % storageBuffers.size();
    size_t particleCount = writeOffset[readTarget] / particleStride;

    return { storageBuffers[readTarget], particleCount }; 
}

void GpuParticleSnapshotBackend::ConsumeReadSnapshot() {
    uint32_t readTarget = readSnapshotIndex.load(std::memory_order::acquire);
    uint32_t writeTarget = writeSnapshotIndex.load(std::memory_order::acquire);

    if(readTarget != writeTarget) {
        readSnapshotIndex.store(readTarget + 1, std::memory_order::release);
    }
}

std::pair<bool, std::string> GpuParticleSnapshotBackend::ProcessFence(Fence fence) {
    if(fence == Fence::Commit) {
        uint32_t writeTarget = writeSnapshotIndex.fetch_add(1, std::memory_order::acquire);
        uint32_t readTarget = 0;

        const auto nextWriteIndex = writeTarget + 1;
        const auto nextBuffer = nextWriteIndex % storageBuffers.size();
        writeOffset[nextBuffer] = 0;

        while(true) {
            readTarget = readSnapshotIndex.load(std::memory_order::acquire);

            if(writeTarget != readTarget) {
                break;
            }

            // Spinlock, we're overlapping read and write buffers
            // The triple buffering should make this case a very rare one
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return {true, ""};
}

std::pair<bool, std::string> GpuParticleSnapshotBackend::ProcessSpan(ParticleSpan span) {
    if (span.particleStride == 0 || particleStride == 0) {
        return {false, "Invalid particle stride"};
    }

    assert(span.particleStride == particleStride && "Particle stride mismatch");
    if (span.particleStride != particleStride) {
        return {false, "Particle stride mismatch"};
    }

    if (span.data.size() % particleStride != 0) {
        return {false, "Payload byte size is not a multiple of the particle stride"};
    }

    const size_t particleCount = span.data.size() / particleStride;
    if (particleCount == 0) {
        return {false, "Payload does not contain complete particles"};
    }

    const size_t byteSize = particleCount * particleStride;

    auto [writeBuffer, writeOffset] = GetWriteSnapshotOffset();
    writeBuffer.GrowBufferToFit(writeOffset + byteSize);
    writeBuffer.UpdateBuffer(writeOffset, span.data.data(), byteSize);
    writeOffset += byteSize;

    return {true, {}};
}

std::pair<weave::opengl::Buffer&, size_t&> GpuParticleSnapshotBackend::GetWriteSnapshotOffset() {  
    auto idx = writeSnapshotIndex.load(std::memory_order::acquire) % storageBuffers.size();
    return {storageBuffers[idx], writeOffset[idx]}; 
}

} // namespace weave::particles::gl
