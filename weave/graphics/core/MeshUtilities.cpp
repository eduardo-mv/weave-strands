#include "MeshUtilities.h"
#include "weave/system/memory/DataType.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>
#include <vector>
#include <unordered_map>

namespace weave::graphics {
namespace wt = weave::types;
namespace {

wt::DataType DeduceIndexType(uint32_t vertexCount) {
    return vertexCount <= std::numeric_limits<uint16_t>::max()
        ? wt::DataType::UInt16
        : wt::DataType::UInt32;
}

void CopyVertexAttributes(MeshData::VertexProxy const& sourceVertex,
                          MeshData::MutableVertexProxy const& targetVertex,
                          std::vector<std::pair<MeshAttribute::Label, uint32_t>> const& mappings) {
    for (auto const& [label, skip] : mappings) {
        auto destElement = targetVertex.Attribute(label, skip);
        if (!destElement) {
            continue;
        }

        auto sourceElement = sourceVertex.Attribute(label, skip);
        if (!sourceElement) {
            std::memset(destElement.Bytes().data(), 0, destElement.Size());
            continue;
        }

        auto copySize = std::min(destElement.Size(), sourceElement.Size());
        std::memcpy(destElement.Bytes().data(), sourceElement.Data(), copySize);
        if (copySize < destElement.Size()) {
            std::memset(destElement.Bytes().data() + copySize, 0, destElement.Size() - copySize);
        }
    }
}

std::string BuildVertexKey(MeshData::VertexProxy const& vertex,
                           MeshData const& mesh,
                           std::vector<std::pair<MeshAttribute::Label, uint32_t>> const& mappings) {
    std::string key;
    for (auto const& [label, skip] : mappings) {
        auto const& attribute = mesh.GetAttribute(label, skip);
        auto elementSize = attribute.ElementByteSize();
        if (elementSize == 0) {
            continue;
        }

        auto element = vertex.Attribute(label, skip);
        if (element) {
            auto bytes = element.Bytes();
            key.append(reinterpret_cast<char const*>(bytes.data()), bytes.size());
        }
        else {
            key.append(elementSize, '\0');
        }
    }

    return key;
}

void CopyIndexData(MeshIndex const& sourceIndex, MeshIndex const& targetIndex) {
    auto count = std::min(sourceIndex.count, targetIndex.count);
    if (count == 0) {
        return;
    }

    for (uint32_t i = 0; i < count; ++i) {
        targetIndex.Write(i, sourceIndex.Read<uint32_t>(i));
    }
}

} // namespace

MeshData ConvertMeshToLayout(MeshData const& source, MeshLayout const& targetLayout) {
    MeshLayout sourceLayout = MeshLayout::FromMeshData(source);
    if (sourceLayout == targetLayout) {
        return source;
    }

    MeshData working;
    bool sourceHasIndex = source.HasIndex();
    bool targetHasIndex = targetLayout.HasIndex();

    if (targetHasIndex && !sourceHasIndex) {
        working = ConvertMeshToIndexedBloated(source);
    }
    else if (!targetHasIndex && sourceHasIndex) {
        working = ConvertMeshToIndexless(source);
    }
    else {
        working = source;
    }

    uint32_t vertexCount = working.GetUniqueVertexCount();
    uint32_t indexCount = targetHasIndex ? working.GetIndexCount() : 0;

    MeshData converted = targetLayout.Build(vertexCount, indexCount);
    for (auto const& section : working.GetSections()) {
        converted.AddSection(section);
    }

    auto attributeMappings = converted.GetAttributeLabels();
    MeshData const& workingConst = working;
    auto sourceVertices = workingConst.UniqueVertices();
    auto targetVertices = converted.UniqueVertices();
    auto sourceIt = sourceVertices.begin();
    auto targetIt = targetVertices.begin();
    auto sourceEnd = sourceVertices.end();
    auto targetEnd = targetVertices.end();

    for (; sourceIt != sourceEnd && targetIt != targetEnd; ++sourceIt, ++targetIt) {
        CopyVertexAttributes(*sourceIt, *targetIt, attributeMappings);
    }

    if (targetHasIndex && working.HasIndex()) {
        CopyIndexData(working.GetIndex(), converted.GetIndex());
    }

    return converted;
}

MeshData ConvertMeshToIndexless(MeshData const& source) {
    if(!source.HasIndex()) {
        return source;
    }

    auto const& sourceIndex = source.GetIndex();
    const uint32_t indexCount = sourceIndex.count;

    MeshLayout layout = MeshLayout::FromMeshData(source);
    layout.SetIndexType(wt::DataType::Void);

    MeshData converted = layout.Build(indexCount, 0);
    for (auto const& section : source.GetSections()) {
        converted.AddSection(section);
    }

	
	auto attributeMappings = converted.GetAttributeLabels();
	auto sourceVertices = source.IndexedVertices();
	auto sourceIt = sourceVertices.begin();
	auto sourceEnd = sourceVertices.end();

	for (auto targetVertex : converted.UniqueVertices()) {
        if(sourceIt == sourceEnd) {
            break;
        }

		CopyVertexAttributes(*sourceIt, targetVertex, attributeMappings);
		++sourceIt;
	}

    return converted;
}

MeshData ConvertMeshToIndexedBloated(MeshData const& source) {
    if (source.HasIndex()) {
        return source;
    }

    auto const vertexCount = source.GetUniqueVertexCount();

    MeshLayout layout = MeshLayout::FromMeshData(source);
    layout.SetIndexType(DeduceIndexType(vertexCount));
    layout.primitiveRestart = false;

    MeshData converted = layout.Build(vertexCount, vertexCount);
    for (auto const& section : source.GetSections()) {
        converted.AddSection(section);
    }

    auto attributeMappings = converted.GetAttributeLabels();
    auto sourceVertices = source.UniqueVertices();
    auto sourceIt = sourceVertices.begin();
    auto const sourceEnd = sourceVertices.end();
    for (auto targetVertex : converted.UniqueVertices()) {
        if (sourceIt == sourceEnd) {
            break;
        }

        CopyVertexAttributes(*sourceIt, targetVertex, attributeMappings);
        ++sourceIt;
    }

    auto const& index = converted.GetIndex();
    for (uint32_t i = 0; i < vertexCount; ++i) {
        index.Write(i, i);
    }

    return converted;
}

MeshData ConvertMeshToIndexed(MeshData const& source) {
    if (source.HasIndex()) {
        return source;
    }

    auto const vertexCount = source.GetUniqueVertexCount();
    if (vertexCount == 0) {
        return source;
    }

    auto attributeMappings = source.GetAttributeLabels();
    std::unordered_map<std::string, uint32_t> vertexLookup;
    vertexLookup.reserve(vertexCount);

    std::vector<uint32_t> remap(vertexCount);
    std::vector<MeshData::VertexProxy> uniqueVertices;
    uniqueVertices.reserve(vertexCount);

    uint32_t originalIndex = 0;
    for (auto vertex : source.UniqueVertices()) {
        auto key = BuildVertexKey(vertex, source, attributeMappings);
        auto [it, inserted] = vertexLookup.try_emplace(std::move(key), static_cast<uint32_t>(uniqueVertices.size()));
        if (inserted) {
            uniqueVertices.push_back(vertex);
        }

        remap[originalIndex++] = it->second;
    }

    auto const uniqueCount = static_cast<uint32_t>(uniqueVertices.size());

    MeshLayout layout = MeshLayout::FromMeshData(source);
    layout.SetIndexType(DeduceIndexType(uniqueCount));
    layout.primitiveRestart = false;

    MeshData converted = layout.Build(uniqueCount, vertexCount);
    for (auto const& section : source.GetSections()) {
        converted.AddSection(section);
    }

    auto convertedAttributeMappings = converted.GetAttributeLabels();
    auto uniqueIt = uniqueVertices.begin();
    for (auto targetVertex : converted.UniqueVertices()) {
        if (uniqueIt == uniqueVertices.end()) {
            break;
        }

        CopyVertexAttributes(*uniqueIt, targetVertex, convertedAttributeMappings);
        ++uniqueIt;
    }

    auto const& index = converted.GetIndex();
    for (uint32_t i = 0; i < vertexCount; ++i) {
        index.Write(i, remap[i]);
    }

	return converted;
}

MeshLayout FlattenLayoutToSingleBuffer(MeshLayout layout) {
	if (layout.buffers.empty()) {
		return layout;
	}

	MeshLayout flat;
	flat.indexType = layout.indexType;
	flat.primitive = layout.primitive;
	flat.primitiveRestart = layout.primitiveRestart;

	auto& buffer = flat.AddBuffer(0);
	uint32_t currentOffset = 0;
	for (auto const& sourceBuffer : layout.buffers) {
		for (auto const& attribute : sourceBuffer.attributes) {
			buffer.attributes.push_back(MeshLayout::Buffer::Attribute{
				attribute.label,
				attribute.type,
				currentOffset
			});

			auto const& traits = weave::types::GetRuntimeTypeTraits(attribute.type);
			currentOffset += static_cast<uint32_t>(traits.byteSizeVector);
		}
	}

	buffer.stride = currentOffset;
	return flat;
}

MeshData AddMeshes(MeshData const& lhs, MeshData const& rhs) {
    MeshLayout lhsLayout = MeshLayout::FromMeshData(lhs);
    MeshData convertedRhs = ConvertMeshToLayout(rhs, lhsLayout);

    bool hasIndex = lhsLayout.HasIndex();
    uint32_t lhsVertexCount = lhs.GetUniqueVertexCount();
    uint32_t rhsVertexCount = convertedRhs.GetUniqueVertexCount();
    uint32_t totalVertexCount = lhsVertexCount + rhsVertexCount;

    uint32_t lhsIndexCount = hasIndex ? lhs.GetIndexCount() : 0;
    uint32_t rhsIndexCount = hasIndex ? convertedRhs.GetIndexCount() : 0;
    uint32_t totalIndexCount = hasIndex ? lhsIndexCount + rhsIndexCount : 0;

    MeshData combined = lhsLayout.Build(totalVertexCount, totalIndexCount);

    auto addSectionsWithOffset = [&](MeshData const& mesh, uint32_t offset) {
        for (auto const& section : mesh.GetSections()) {
            MeshSection adjusted = section;
            adjusted.indexStart += offset;
            combined.AddSection(adjusted);
        }
    };

    addSectionsWithOffset(lhs, 0);
    uint32_t sectionOffset = hasIndex ? lhsIndexCount : lhsVertexCount;
    addSectionsWithOffset(convertedRhs, sectionOffset);

    auto attributeMappings = combined.GetAttributeLabels();
    auto targetVertices = combined.UniqueVertices();
    auto targetIt = targetVertices.begin();

    for (auto sourceVertex : lhs.UniqueVertices()) {
        if (targetIt == targetVertices.end()) {
            break;
        }
        CopyVertexAttributes(sourceVertex, *targetIt, attributeMappings);
        ++targetIt;
    }

    MeshData const& convertedRhsConst = convertedRhs;
    for (auto sourceVertex : convertedRhsConst.UniqueVertices()) {
        if (targetIt == targetVertices.end()) {
            break;
        }
        CopyVertexAttributes(sourceVertex, *targetIt, attributeMappings);
        ++targetIt;
    }

    if (hasIndex) {
        auto const& combinedIndex = combined.GetIndex();
        auto const& lhsIndex = lhs.GetIndex();
        auto const& rhsIndex = convertedRhs.GetIndex();

        for (uint32_t i = 0; i < lhsIndexCount; ++i) {
            combinedIndex.Write(i, lhsIndex.Read<uint32_t>(i));
        }

        for (uint32_t i = 0; i < rhsIndexCount; ++i) {
            combinedIndex.Write(lhsIndexCount + i, rhsIndex.Read<uint32_t>(i) + lhsVertexCount);
        }
    }

    return combined;
}

MeshData ExtractSubmesh(MeshData const& source, uint32_t vertexStart, uint32_t vertexCount) {
    MeshLayout layout = MeshLayout::FromMeshData(source);
    uint32_t totalVertices = source.GetUniqueVertexCount();
    if (totalVertices == 0) {
        return layout.Build(0, 0);
    }

    uint32_t clampedStart = std::min(vertexStart, totalVertices);
    uint32_t available = totalVertices - clampedStart;
    uint32_t clampedCount = std::min(vertexCount, available);
    if (clampedCount == 0) {
        return layout.Build(0, layout.HasIndex() ? 0 : 0);
    }

    uint32_t vertexRangeEnd = clampedStart + clampedCount;
    bool hasIndex = source.HasIndex();

    std::vector<uint32_t> filteredIndices;
    std::vector<uint32_t> indexPositionMap;
    constexpr uint32_t invalidIndex = std::numeric_limits<uint32_t>::max();
    if (hasIndex) {
        uint32_t sourceIndexCount = source.GetIndexCount();
        filteredIndices.reserve(sourceIndexCount);
        indexPositionMap.assign(sourceIndexCount, invalidIndex);

        auto const& srcIndex = source.GetIndex();
        for (uint32_t idxPos = 0; idxPos < sourceIndexCount; ++idxPos) {
            uint32_t value = srcIndex.Read<uint32_t>(idxPos);
            if (value >= clampedStart && value < vertexRangeEnd) {
                uint32_t newPos = static_cast<uint32_t>(filteredIndices.size());
                filteredIndices.push_back(value - clampedStart);
                indexPositionMap[idxPos] = newPos;
            }
        }
    }

    uint32_t targetIndexCount = hasIndex ? static_cast<uint32_t>(filteredIndices.size()) : 0;
    MeshData submesh = layout.Build(clampedCount, targetIndexCount);

    if (hasIndex) {
        for (auto const& section : source.GetSections()) {
            uint32_t newStart = invalidIndex;
            uint32_t newCount = 0;
            for (uint32_t offset = 0; offset < section.indexCount; ++offset) {
                uint32_t srcPos = section.indexStart + offset;
                if (srcPos >= indexPositionMap.size()) {
                    break;
                }

                uint32_t mapped = indexPositionMap[srcPos];
                if (mapped != invalidIndex) {
                    if (newStart == invalidIndex) {
                        newStart = mapped;
                    }
                    ++newCount;
                }
            }

            if (newCount > 0) {
                MeshSection adjusted = section;
                adjusted.indexStart = newStart;
                adjusted.indexCount = newCount;
                submesh.AddSection(adjusted);
            }
        }
    }
    else {
        for (auto const& section : source.GetSections()) {
            uint32_t sectionStart = section.indexStart;
            uint32_t sectionEnd = section.indexStart + section.indexCount;
            uint32_t overlapStart = std::max(sectionStart, clampedStart);
            uint32_t overlapEnd = std::min(sectionEnd, vertexRangeEnd);
            if (overlapStart < overlapEnd) {
                MeshSection adjusted = section;
                adjusted.indexStart = overlapStart - clampedStart;
                adjusted.indexCount = overlapEnd - overlapStart;
                submesh.AddSection(adjusted);
            }
        }
    }

    auto attributeMappings = submesh.GetAttributeLabels();
    auto sourceVertices = source.UniqueVertices(clampedStart, clampedCount);
    auto targetVertices = submesh.UniqueVertices();
    auto sourceIt = sourceVertices.begin();
    for (auto targetVertex : targetVertices) {
        if (sourceIt == sourceVertices.end()) {
            break;
        }

        CopyVertexAttributes(*sourceIt, targetVertex, attributeMappings);
        ++sourceIt;
    }

    if (hasIndex && targetIndexCount > 0) {
        auto const& targetIndex = submesh.GetIndex();
        for (uint32_t i = 0; i < targetIndexCount; ++i) {
            targetIndex.Write(i, filteredIndices[i]);
        }
    }

    return submesh;
}

} // namespace weave::graphics
