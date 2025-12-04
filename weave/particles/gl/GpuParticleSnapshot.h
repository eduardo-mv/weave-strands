#pragma once

#include "weave/particles/gl/GpuParticleSnapshotBackend.h"
#include "weave/system/memory/StreamingQueue.h"

#include <chrono>
#include <memory>

namespace weave::particles::gl {

class GpuParticleSnapshot {
public:
    using Backend = GpuParticleSnapshotBackend;
    using Queue = weave::system::memory::StreamingQueue<Backend>;
    using Payload = Backend::Payload;
    using ParticleSpan = Backend::ParticleSpan;
    using Fence = Backend::Fence;

    explicit GpuParticleSnapshot(size_t particleStrideBytes);

    void UploadParticleBuffer(std::shared_ptr<const particles::ParticleBuffer> buffer);
    void UploadParticleSpan(ParticleSpan span);
    void CommitSnapshot();

    size_t ProcessStreamingQueue(size_t maxTasks,
                                 std::chrono::milliseconds blockTimeout = std::chrono::milliseconds::zero());
    size_t PeekStreamingQueue() const;

    void ReserveBytes(size_t byteCount);
    void ReserveParticles(size_t particleCount);

    size_t ParticleStride() const { return backend.ParticleStride(); }
    weave::opengl::Buffer const& GetReadSnapshot() const;
    void ConsumeReadSnapshot();

    Backend& BackendAccess() { return backend; }
    Backend const& BackendAccess() const { return backend; }

private:
    Backend backend;
    Queue queue;
};

} // namespace weave::particles::gl
