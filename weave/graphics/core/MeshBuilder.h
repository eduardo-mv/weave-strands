#pragma once

#include "MeshData.h"

#include <vector>

namespace weave::graphics {

class MeshBuilder {
private:
    MeshPrimitive mprimitive{ MeshPrimitive::Triangles };
    MeshIndex index{};
    void const* indexToCopy = nullptr;

    struct AttribData {
        MeshAttribute::Label label{};
        types::DataType type{};
        uint64_t byteSize{};
        void const* dataToCopy{};
        bool isMutable{};
    };

    std::vector<AttribData> attribs;

    uint32_t singleSectionLabel = 0;
    uint32_t singleMaterialChannel = 0;
    std::vector<MeshSection> sections;

public:
    MeshBuilder& Primitive(MeshPrimitive mprimitive);

    template<typename Type, size_t count>
    MeshBuilder& AddIndex(Type const (&dataToCopy)[count], bool primitiveRestart) {
        return AddIndex(types::TypeTraits<Type>::dataTypeValue, static_cast<uint32_t>(count), dataToCopy, primitiveRestart);
    }

    template<typename Type>
    MeshBuilder& AddIndex(uint32_t indicesCount, Type const* dataToCopy, bool primitiveRestart) {
        return AddIndex(types::TypeTraits<Type>::dataTypeValue, indicesCount, dataToCopy, primitiveRestart);
    }

    MeshBuilder& AddIndex(types::DataType type, uint32_t indicesCount, bool primitiveRestart, void const* dataToCopy);
    MeshBuilder& NoIndex(uint32_t vertexCount = ~0u);

    //TODO: Remove these templates and rework. They lead to wrong typing of attributes that is hard to spot
    template<typename Type, size_t count>
    MeshBuilder& AddAttribute(MeshAttribute::Label label, Type const (&dataToCopy)[count]) {
        return AddAttribute(label, types::TypeTraits<Type>::dataTypeValue, sizeof(dataToCopy), dataToCopy);
    }

    //TODO: Remove these templates and rework. They lead to wrong typing of attributes that is hard to spot
    template<typename Type>
    MeshBuilder& AddAttribute(MeshAttribute::Label label, uint32_t elementCount, Type const* dataToCopy) {
        return AddAttribute(label, types::TypeTraits<Type>::dataTypeValue, sizeof(Type) * elementCount, dataToCopy);
    }

    MeshBuilder& AddAttribute(MeshAttribute::Label label, types::DataType type, uint64_t byteSize, void const* dataToCopy);

    MeshBuilder& AddSection(MeshSection const& section);
    MeshBuilder& SingleSection(uint32_t label, uint32_t materialChannel);

    MeshData Build(bool multiBuffer = false) const;
};

}
