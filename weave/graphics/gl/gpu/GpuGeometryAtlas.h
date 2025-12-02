#pragma once

#include "weave/graphics/gl/gpu/GpuGeometryAtlasBackend.h"
#include "weave/graphics/gl/gl46.h"
#include "weave/system/memory/StreamingForge.h"

#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace weave::graphics::gl::gpu {

class GpuGeometryAtlas {
public:
    struct DrawElementsIndirectCommand {
        GLuint count = 0;
        GLuint instanceCount = 0;
        GLuint firstIndex = 0;
        GLuint baseVertex = 0;
        GLuint baseInstance = 0;
    };

    using Backend = GpuGeometryAtlasBackend;
    using Forge = weave::system::memory::StreamingForge<Backend>;

    using Handle = typename Forge::Handle;
    using StreamingTicket = typename Forge::StreamingTicket;
    using StreamingResult = typename Forge::StreamingResult;
    using StreamStatus = typename Forge::StreamStatus;
    using EntryInfo = typename Forge::Entry;

    explicit GpuGeometryAtlas(graphics::MeshLayout layout);
    ~GpuGeometryAtlas();

    GpuGeometryAtlas(GpuGeometryAtlas const&) = delete;
    GpuGeometryAtlas& operator=(GpuGeometryAtlas const&) = delete;
    GpuGeometryAtlas(GpuGeometryAtlas&&) noexcept = delete;
    GpuGeometryAtlas& operator=(GpuGeometryAtlas&&) noexcept = delete;

    StreamingTicket StreamMesh(std::shared_ptr<const graphics::MeshData> meshData);
    StreamingTicket RemoveMesh(Handle handle);

    StreamStatus QueryStreamingStatus(Handle handle) const;
    std::optional<EntryInfo> QueryEntry(Handle handle) const;

    size_t ProcessStreamingQueue(size_t maxJobs, std::chrono::milliseconds blockTimeout);
    size_t PeekStreamingQueue() const;

    void ReserveBytes(size_t vertexBytes, size_t indexBytes);
    void ReserveGeometry(size_t vertexCount, size_t indexCount, size_t vertexStrideBytes);

    Backend& BackendAccess();
    Backend const& BackendAccess() const;
    unsigned int GetOrBuildVao();
    std::span<const DrawElementsIndirectCommand> BuildDrawCommands(std::span<const Handle> handles);

private:
    void RebuildVao();

    Backend backend;
    Forge forge;
    unsigned int vao = 0;
    std::vector<DrawElementsIndirectCommand> cachedCommands;
};

} // namespace weave::graphics::gl::gpu
