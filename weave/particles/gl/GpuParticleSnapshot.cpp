#include "GpuParticleSnapshot.h"

#include <utility>

namespace weave::particles::gl {

GpuParticleSnapshot::GpuParticleSnapshot(size_t particleStrideBytes)
    : backend(particleStrideBytes)
    , queue(backend) {
}

void GpuParticleSnapshot::UploadParticleBuffer(std::shared_ptr<const particles::ParticleBuffer> buffer) {
    if (!buffer) {
        return;
    }

    queue.Enqueue(Payload(std::move(buffer)));
}

void GpuParticleSnapshot::UploadParticleSpan(ParticleSpan span) {
    queue.Enqueue(Payload(span));
}

void GpuParticleSnapshot::CommitSnapshot() {
    queue.Enqueue(Payload(Fence::Commit));
}

size_t GpuParticleSnapshot::ProcessStreamingQueue(size_t maxTasks, std::chrono::milliseconds blockTimeout) {
    return queue.ProcessStreamingQueue(maxTasks, blockTimeout);
}

size_t GpuParticleSnapshot::PeekStreamingQueue() const {
    return queue.PeekQueuedCount();
}

void GpuParticleSnapshot::ReserveBytes(size_t byteCount) {
    backend.ReserveBytes(byteCount);
}

void GpuParticleSnapshot::ReserveParticles(size_t particleCount) {
    backend.ReserveParticles(particleCount);
}

std::pair<weave::opengl::Buffer const&, size_t> GpuParticleSnapshot::GetReadSnapshot() const {
    return backend.GetReadSnapshot();
}

void GpuParticleSnapshot::ConsumeReadSnapshot() {
    backend.ConsumeReadSnapshot();
}

} // namespace weave::particles::gl
