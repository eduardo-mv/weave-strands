#pragma once

#include "weave/particles/core/ParticleBuffer.h"
#include "weave/graphics/gl/resources/Buffer.h"
#include "weave/system/memory/RangeAllocator.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <span>
#include <string>
#include <utility>
#include <variant>

namespace weave::particles::gl {

class GpuParticleSnapshotBackend {
public:

    enum class Fence {
        Commit
    };

    struct ParticleSpan {
        std::span<const std::byte> data;
        size_t particleStride = 0;
    };

    using Payload = std::variant<
        std::shared_ptr<const ParticleBuffer>,
        ParticleSpan,
        Fence>;


    explicit GpuParticleSnapshotBackend(size_t particleStrideBytes);

    std::pair<bool, std::string> UploadEntry(Payload&& payload);

    void ReserveBytes(size_t byteCount);
    void ReserveParticles(size_t particleCount);

    size_t ParticleStride() const { return particleStride; }
    weave::opengl::Buffer const& GetReadSnapshot() const;
    void ConsumeReadSnapshot();

private:
    std::pair<bool, std::string> ProcessFence(Fence fence);
    std::pair<bool, std::string> ProcessSpan(ParticleSpan span);
    std::pair<weave::opengl::Buffer&, size_t&> GetWriteSnapshotOffset();

    constexpr static const size_t kBufferCount = 3;
    std::array<weave::opengl::Buffer, kBufferCount> storageBuffers;
    std::array<size_t, kBufferCount> writeOffset{};

    size_t particleStride = 0;
    std::atomic_uint32_t readSnapshotIndex {0};
    std::atomic_uint32_t writeSnapshotIndex{1};
};

} // namespace weave::particles::gl
