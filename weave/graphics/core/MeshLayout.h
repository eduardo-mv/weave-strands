#pragma once

#include "weave/graphics/core/MeshData.h"
#include <span>

namespace weave::graphics {

class MeshLayout {
public:
    struct Buffer {
        struct Attribute {
            MeshAttribute::Label label = MeshAttribute::Label::Generic;
            types::DataType type = types::DataType::Void;
            uint32_t strideOffset = 0;
        };

        uint32_t stride = 0;
        std::vector<Attribute> attributes;

        Buffer& AddAttribute(MeshAttribute::Label label, types::DataType type, uint32_t strideOffset);

    };

    static MeshLayout FromMeshData(MeshData const& mesh);
    Buffer& AddBuffer(uint32_t stride);
    MeshLayout& AddAttribute(size_t bufferIndex, MeshAttribute::Label label, types::DataType type, uint32_t strideOffset);
    MeshLayout& SetIndexType(types::DataType type);

    MeshData Build(std::vector<std::span<const std::byte>> bufferData = {}) const;

    std::vector<Buffer> buffers;
    types::DataType indexType = types::DataType::Void;
};

bool operator==(MeshLayout const& lhs, MeshLayout const& rhs);
inline bool operator!=(MeshLayout const& lhs, MeshLayout const& rhs) { return !(lhs == rhs); }

}
