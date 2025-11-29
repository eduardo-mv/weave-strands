#include "MeshBuilder.h"

#include <cstring>

using namespace weave;
using namespace weave::graphics;

MeshBuilder& MeshBuilder::Primitive(MeshPrimitive mprimitive_) {
    mprimitive = mprimitive_;
    return *this;
}

MeshBuilder& MeshBuilder::AddIndex(types::DataType type, uint32_t indicesCount, bool primitiveRestart, void const* dataToCopy) {
    if (type == types::DataType::UInt32 || type == types::DataType::UInt16) {
        index.type = type;
        index.primitiveRestart = primitiveRestart;
        index.count = indicesCount;
        indexToCopy = dataToCopy;
    }

    return *this;
}

MeshBuilder& MeshBuilder::NoIndex(uint32_t vertexCount) {
    index.Indexless(vertexCount);
    return *this;
}

MeshBuilder& MeshBuilder::AddAttribute(MeshAttribute::Label label, types::DataType type, uint64_t byteSize, void const* dataToCopy) {
    attribs.emplace_back(AttribData{ label, type, byteSize, dataToCopy, false });
    return *this;
}

MeshBuilder& MeshBuilder::AddSection(MeshSection const& section) {
    sections.push_back(section);
    return *this;
}

MeshBuilder& MeshBuilder::SingleSection(uint32_t label, uint32_t materialChannel) {
    sections.clear();
    singleSectionLabel = label;
    singleMaterialChannel = materialChannel;
    return *this;
}

MeshData MeshBuilder::Build(bool multiBuffer) const {
    MeshData mesh;

    if (multiBuffer) {
        for (auto const& attrib : attribs) {
            auto stream = mesh.AllocateStream(static_cast<uint32_t>(types::GetRuntimeTypeTraits(attrib.type).byteSizeVector), attrib.byteSize, attrib.dataToCopy, attrib.isMutable);
            mesh.AddAttribute({ attrib.label, ~0u, attrib.type, 0, stream });
        }

        if (index.IsIndexless()) {
            mesh.NoIndex(index.count);
        }
        else {
            mesh.SetIndex(mesh.AllocateIndex(index.type, index.count, index.primitiveRestart, indexToCopy, false));
        }
    }
    else {
        uint64_t byteSize = 0;

        if (!index.IsIndexless()) {
            byteSize += index.count * types::GetRuntimeTypeTraits(index.type).byteSizeVector;
        }

        for (auto const& attrib : attribs) {
            byteSize += attrib.byteSize;
        }

        auto buffer = mesh.AllocateMemory(byteSize, nullptr, false);

        uint64_t byteOffset = 0;
        if (!index.IsIndexless()) {
            auto size = index.count * types::GetRuntimeTypeTraits(index.type).byteSizeVector;
            MeshIndex idx = index;
            idx.bufferView = MeshBufferView{ byteOffset, size, buffer };
            mesh.SetIndex(idx);
            byteOffset += size;

            std::memcpy(idx.bufferView.Memory(), indexToCopy, size);
        }

        for (auto const& attrib : attribs) {
            MeshBufferView bufferView{ byteOffset, attrib.byteSize, buffer };
            mesh.AddAttribute({ attrib.label, ~0u, attrib.type, 0, MeshDataStream{ static_cast<uint32_t>(types::GetRuntimeTypeTraits(attrib.type).byteSizeVector), bufferView } });
            byteOffset += attrib.byteSize;

            std::memcpy(bufferView.Memory(), attrib.dataToCopy, attrib.byteSize);
        }

        if (index.IsIndexless()) {
            mesh.NoIndex(index.count);
        }
    }

    if (sections.empty()) {
        mesh.SingleSection(singleSectionLabel, singleMaterialChannel);
    }
    else {
        for (auto const& section : sections) {
            mesh.AddSection(section);
        }
    }

    mesh.SetPrimitive(mprimitive);

    return mesh;
}

