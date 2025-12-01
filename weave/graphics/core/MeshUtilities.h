#pragma once

#include "MeshData.h"
#include "MeshLayout.h"

namespace weave::graphics {

MeshData ConvertMeshToLayout(MeshData const& source, MeshLayout const& targetLayout);
MeshData ConvertMeshToIndexless(MeshData const& source);
MeshData ConvertMeshToIndexedBloated(MeshData const& source);
MeshData ConvertMeshToIndexed(MeshData const& source);
MeshData AddMeshes(MeshData const& lhs, MeshData const& rhs);
MeshData ExtractSubmesh(MeshData const& source, uint32_t vertexStart, uint32_t vertexCount);


}
