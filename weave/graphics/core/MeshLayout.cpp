#include "MeshLayout.h"

using namespace weave::graphics;


MeshLayout::Buffer& MeshLayout::Buffer::AddAttribute(MeshAttribute::Label label, types::DataType type, uint32_t strideOffset) {
    attributes.push_back(Buffer::Attribute{label, type, strideOffset});
    return *this;
}

MeshLayout MeshLayout::FromMeshData(MeshData const& mesh) {
    MeshLayout layout;

    std::vector<MeshDataStream> uniqueStreams = mesh.GetUniqueStreams();
    layout.buffers.resize(uniqueStreams.size());
    for (size_t i = 0; i < uniqueStreams.size(); ++i) {
        layout.buffers[i].stride = uniqueStreams[i].elemByteStride;
    }

    for (auto const& attr : mesh.GetAttributes()) {
        uint32_t bufferIdx = 0;
        for (size_t i = 0; i < uniqueStreams.size(); ++i) {
            if (uniqueStreams[i] == attr.stream) {
                bufferIdx = static_cast<uint32_t>(i);
                break;
            }
        }

        layout.buffers[bufferIdx].attributes.push_back({
            attr.label,
            attr.type,
            attr.strideOffset
        });
    }

    layout.indexType = mesh.HasIndex() ? mesh.GetIndex().type : types::DataType::Void;
    layout.primitive = mesh.Primitive();
    layout.primitiveRestart = mesh.GetIndex().primitiveRestart;

    return layout;
}

MeshLayout::Buffer& MeshLayout::AddBuffer(uint32_t stride) {
    buffers.push_back(Buffer{});
    buffers.back().stride = stride;
    return buffers.back();
}

MeshLayout& MeshLayout::AddAttribute(size_t bufferIndex, MeshAttribute::Label label, types::DataType type, uint32_t strideOffset) {
    if (bufferIndex < buffers.size()) {
        buffers[bufferIndex].AddAttribute(label, type, strideOffset);
    }
    return *this;
}

MeshLayout& MeshLayout::SetIndexType(types::DataType type) {
    indexType = type;
    return *this;
}

MeshData MeshLayout::Build(uint32_t vertexCount, uint32_t indexCount) const {
    MeshData mesh;
    mesh.SetPrimitive(primitive);

    for (size_t i = 0; i < buffers.size(); ++i) {
        auto const& buf = buffers[i];
        uint64_t dataSize = static_cast<uint64_t>(buf.stride) * vertexCount;
        auto stream = mesh.AllocateStream(buf.stride, dataSize, nullptr, false);
        for (auto const& attr : buf.attributes) {
            mesh.AddAttribute(MeshAttribute{
                attr.label,
                ~0u,
                attr.type,
                attr.strideOffset,
                stream
            });
        }
    }

    if (HasIndex()) {
        mesh.SetIndex(mesh.AllocateIndex(indexType, indexCount, primitiveRestart, nullptr, false));
    }
    else {
        mesh.NoIndex(vertexCount);
    }

    return mesh;
}

bool weave::graphics::operator==(MeshLayout const& lhs, MeshLayout const& rhs) {
    if (lhs.indexType != rhs.indexType || lhs.buffers.size() != rhs.buffers.size()) {
        return false;
    }

    for (size_t i = 0; i < lhs.buffers.size(); ++i) {
        auto const& la = lhs.buffers[i];
        auto const& ra = rhs.buffers[i];
        if (la.stride != ra.stride || la.attributes.size() != ra.attributes.size()) {
            return false;
        }

        for (size_t j = 0; j < la.attributes.size(); ++j) {
            auto const& lattr = la.attributes[j];
            auto const& rattr = ra.attributes[j];
            if (lattr.label != rattr.label ||
                lattr.type != rattr.type ||
                lattr.strideOffset != rattr.strideOffset) {
                return false;
            }
        }
    }

    return true;
}
