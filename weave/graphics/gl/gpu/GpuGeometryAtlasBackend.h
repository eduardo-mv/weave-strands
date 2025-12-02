#pragma once

#include "weave/graphics/core/MeshData.h"
#include "weave/graphics/core/MeshLayout.h"
#include "weave/graphics/gl/resources/Buffer.h"
#include "weave/system/memory/RangeAllocator.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace weave::graphics::gl::gpu {

class GpuGeometryAtlasBackend {
public:
    struct Entry {
        weave::memory::RangeAllocator::Range vertexRange{};
        weave::memory::RangeAllocator::Range indexRange{};
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
    };

    struct Payload {
        std::shared_ptr<const graphics::MeshData> mesh;
    };

    explicit GpuGeometryAtlasBackend(graphics::MeshLayout layout);

    std::pair<bool, std::string> UploadEntry(Entry& entry, Payload&& payload);
    std::pair<bool, std::string> RemoveEntry(Entry& entry);

    void ReserveBytes(size_t vertexBytes, size_t indexBytes);
    void ReserveGeometry(size_t vertexCount, size_t indexCount, size_t vertexStrideBytes);

    graphics::MeshLayout const& Layout() const { return targetLayout; }
    bool ConsumeBufferGrowFlag() { return buffersDirty.exchange(false, std::memory_order_acq_rel); }
    GLuint VertexBufferId() const { return vertexBuffer.GLId(); }
    GLuint IndexBufferId() const { return indexBuffer.GLId(); }
    std::unique_lock<std::mutex> LockBufferAccess() const;

private:

    std::pair<size_t, size_t> ComputeRequiredByteSize(MeshData const& mesh) const;
    std::pair<size_t, size_t> AllocateBufferBytes(size_t vertexBytes, size_t indexBytes);

    graphics::MeshLayout targetLayout;
    weave::opengl::Buffer vertexBuffer;
    weave::opengl::Buffer indexBuffer;
    weave::memory::RangeAllocator vertexAllocator;
    weave::memory::RangeAllocator indexAllocator;
    std::atomic<bool> buffersDirty{false};
    mutable std::mutex bufferMutex;
};

} // namespace weave::graphics::gl::gpu
