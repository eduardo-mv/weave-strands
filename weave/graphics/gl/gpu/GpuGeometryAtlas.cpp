#include "GpuGeometryAtlas.h"

#include "weave/graphics/gl/gl46.h"
#include "weave/system/memory/DataType.h"

#include <utility>

namespace weave::graphics::gl::gpu {

namespace {
GLenum ToGlEnum(weave::types::DataType type) {
    using weave::types::DataType;
    switch (type) {
    case DataType::Float:
        return ::gl::FLOAT;
    case DataType::Double:
        return ::gl::DOUBLE;
    case DataType::Int8:
        return ::gl::BYTE;
    case DataType::UInt8:
        return ::gl::UNSIGNED_BYTE;
    case DataType::Int16:
        return ::gl::SHORT;
    case DataType::UInt16:
        return ::gl::UNSIGNED_SHORT;
    case DataType::Int32:
        return ::gl::INT;
    case DataType::UInt32:
        return ::gl::UNSIGNED_INT;
    default:
        return ::gl::FLOAT;
    }
}

bool IsIntegerAttrib(weave::types::DataType type) {
    using weave::types::DataType;
    return type == DataType::Int32 || type == DataType::UInt32;
}

bool UseNormalization(weave::types::DataType type) {
    using weave::types::DataType;
    return type == DataType::Int8 || type == DataType::UInt8 ||
           type == DataType::Int16 || type == DataType::UInt16;
}

bool IsDoubleAttrib(weave::types::DataType type) {
    using weave::types::DataType;
    return type == DataType::Double;
}
} // namespace

GpuGeometryAtlas::GpuGeometryAtlas(graphics::MeshLayout layout)
    : backend(std::move(layout))
    , forge(backend) {
}

GpuGeometryAtlas::~GpuGeometryAtlas() {
    if (vao != 0) {
        ::gl::DeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

GpuGeometryAtlas::StreamingTicket GpuGeometryAtlas::StreamMesh(std::shared_ptr<const graphics::MeshData> meshData) {
    if (!meshData) {
        return {};
    }

    Backend::Payload payload;
    payload.mesh = std::move(meshData);
    return forge.StreamPayload(std::move(payload));
}

GpuGeometryAtlas::StreamingTicket GpuGeometryAtlas::RemoveMesh(Handle handle) {
    return forge.RemovePayload(handle);
}

GpuGeometryAtlas::StreamStatus GpuGeometryAtlas::QueryStreamingStatus(Handle handle) const {
    return forge.QueryStreamingStatus(handle);
}

std::optional<GpuGeometryAtlas::EntryInfo> GpuGeometryAtlas::QueryEntry(Handle handle) const {
    return forge.QueryEntry(handle);
}

size_t GpuGeometryAtlas::ProcessStreamingQueue(size_t maxJobs, std::chrono::milliseconds blockTimeout) {
    return forge.ProcessStreamingQueue(maxJobs, blockTimeout);
}

size_t GpuGeometryAtlas::PeekStreamingQueue() const {
    return forge.PeekStreamingQueue();
}

void GpuGeometryAtlas::ReserveBytes(size_t vertexBytes, size_t indexBytes) {
    backend.ReserveBytes(vertexBytes, indexBytes);
}

void GpuGeometryAtlas::ReserveGeometry(size_t vertexCount, size_t indexCount, size_t vertexStrideBytes) {
    backend.ReserveGeometry(vertexCount, indexCount, vertexStrideBytes);
}

GpuGeometryAtlas::Backend& GpuGeometryAtlas::BackendAccess() {
    return backend;
}

GpuGeometryAtlas::Backend const& GpuGeometryAtlas::BackendAccess() const {
    return backend;
}

unsigned int GpuGeometryAtlas::GetOrBuildVao() {
    if (vao == 0 || backend.ConsumeBufferGrowFlag()) {
        RebuildVao();
    }

    return vao;
}

void GpuGeometryAtlas::RebuildVao() {
    auto lock = backend.LockBufferAccess();

    const GLuint vertexBufferId = backend.VertexBufferId();
    if (vertexBufferId == 0) {
        return;
    }

    if (vao != 0) {
        ::gl::DeleteVertexArrays(1, &vao);
        vao = 0;
    }

    ::gl::GenVertexArrays(1, &vao);
    ::gl::BindVertexArray(vao);

    ::gl::BindBuffer(::gl::ARRAY_BUFFER, vertexBufferId);
    ::gl::BindBuffer(::gl::ELEMENT_ARRAY_BUFFER, backend.IndexBufferId());

    const auto& layout = backend.Layout();
    GLuint attribIndex = 0;
    for (auto const& buffer : layout.buffers) {
        for (auto const& attribute : buffer.attributes) {
            auto traits = weave::types::GetRuntimeTypeTraits(attribute.type);
            const GLint componentCount = static_cast<GLint>(traits.vectorSize);
            const GLenum glType = ToGlEnum(traits.underlyingDataTypeValue);
            const GLsizei stride = static_cast<GLsizei>(buffer.stride);
            const void* offset = reinterpret_cast<void*>(static_cast<uintptr_t>(attribute.strideOffset));

            ::gl::EnableVertexAttribArray(attribIndex);

            if (IsDoubleAttrib(traits.underlyingDataTypeValue)) {
                ::gl::VertexAttribLPointer(attribIndex, componentCount, glType, stride, offset);
            } else if (IsIntegerAttrib(traits.underlyingDataTypeValue)) {
                ::gl::VertexAttribIPointer(attribIndex, componentCount, glType, stride, offset);
            } else {
                const GLboolean normalized = UseNormalization(traits.underlyingDataTypeValue) ? ::gl::TRUE_ : ::gl::FALSE_;
                ::gl::VertexAttribPointer(attribIndex, componentCount, glType, normalized, stride, offset);
            }

            ++attribIndex;
        }
    }

    ::gl::BindVertexArray(0);
}

std::span<const GpuGeometryAtlas::DrawElementsIndirectCommand> GpuGeometryAtlas::BuildDrawCommands(std::span<const Handle> handles) {
    cachedCommands.clear();

    const auto& layout = backend.Layout();
    if (handles.empty() || layout.buffers.empty()) {
        return cachedCommands;
    }

    const uint32_t vertexStride = layout.buffers[0].stride;
    if (vertexStride == 0) {
        return cachedCommands;
    }

    constexpr size_t indexStride = sizeof(uint32_t);

    cachedCommands.reserve(handles.size());

    for (auto handle : handles) {
        auto entryOpt = QueryEntry(handle);
        if (!entryOpt.has_value() || entryOpt->indexCount == 0 || entryOpt->indexRange.size == 0) {
            continue;
        }

        DrawElementsIndirectCommand cmd{};
        cmd.count = entryOpt->indexCount;
        cmd.instanceCount = 1;
        cmd.firstIndex = static_cast<GLuint>(entryOpt->indexRange.offset / indexStride);
        cmd.baseVertex = static_cast<GLint>(entryOpt->vertexRange.offset / vertexStride);
        cmd.baseInstance = 0;
        cachedCommands.push_back(cmd);
    }

    return cachedCommands;
}

} // namespace weave::graphics::gl::gpu
