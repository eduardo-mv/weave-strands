#include "GpuGeometryAtlasBackend.h"

#include "weave/graphics/core/MeshLoader.h"
#include "weave/graphics/core/MeshUtilities.h"
#include "weave/system/memory/DataType.h"

#include <utility>
#include <limits>

namespace weave::graphics::gl::gpu {

namespace {
size_t SafeByteSize(size_t count, size_t stride) {
	if (stride == 0 || count == 0) {
		return 0;
	}

	const size_t maxCount = std::numeric_limits<size_t>::max() / stride;
	return std::min(count, maxCount) * stride;
}
} // namespace

GpuGeometryAtlasBackend::GpuGeometryAtlasBackend(graphics::MeshLayout layout)
    : targetLayout(FlattenLayoutToSingleBuffer(std::move(layout))) {
    targetLayout.SetIndexType(weave::types::DataType::UInt32);
}

std::pair<bool, std::string> GpuGeometryAtlasBackend::UploadEntry(Entry& entry, Payload&& payload) {

	 graphics::MeshData converted;
	if(auto *path = std::get_if<std::filesystem::path>(&payload)) {
		converted = ConvertMeshToLayout(MeshLoader::Load(*path), targetLayout);
	}
	else if(auto *meshShared = std::get_if<std::shared_ptr<const MeshData>>(&payload))  {
		converted = ConvertMeshToLayout(**meshShared, targetLayout);
	}
	else {
		return { false, "Mesh payload expired" };
	}

	// 1. Convert incoming mesh to the target layout so that vertex buffers
	//    are already flattened and attributes match shader expectations.
	auto streams = converted.GetUniqueStreams();
	if(streams.empty() || converted.GetIndexCount() == 0) {
		return {false, "Empty mesh object"};
	}

	if(streams.size() > 1) {
		return {false, "Malformed mesh object after conversion: more than one stream found."};
	}

	// 2. Compute vertex/index byte requirements based on the converted mesh
	//    (vertexCount * stride, indexCount * element size).
	auto [vertexByteSize, indexByteSize] = ComputeRequiredByteSize(converted);

	// 3. Try allocating slices from range allocators. If allocation fails
	//    due to fragmentation/capacity, grow the buffers (and rebuild allocations)
	//    before retrying the allocation.
	auto [vertexTargetOffset, indexTargetOffset] = AllocateBufferBytes(vertexByteSize, indexByteSize);

	// 4. Once ranges are available, stage offsets/counts inside the entry.
	entry.vertexRange = {vertexTargetOffset, vertexByteSize};
	entry.vertexCount = converted.GetUniqueVertexCount();
    entry.indexRange = {indexTargetOffset, indexByteSize};
    entry.indexCount = converted.GetIndexCount();
	
	// 5. Upload vertex/index data via Buffer::UpdateBuffer.
	auto vertexMeshBuffer = streams.front().bufferView.buffer;
	vertexBuffer.UpdateBuffer(vertexTargetOffset, vertexMeshBuffer.Memory<void const*>(), vertexByteSize);

	auto indexMeshBuffer = converted.GetIndex().bufferView.buffer;
	indexBuffer.UpdateBuffer(indexTargetOffset, indexMeshBuffer.Memory<void const*>(), indexByteSize);

	// 6. Copy section metadata or other entry bookkeeping as required.
	// TODO: Figure out if sections are needed (they are mostly material related sections, which means gpu pipeline changes - thus likely not supported by backend)

	return { true, {} };
}

std::pair<bool, std::string> GpuGeometryAtlasBackend::RemoveEntry(Entry& entry) {
	
	if(entry.vertexRange.size) {
		vertexAllocator.Free(entry.vertexRange.offset, entry.vertexRange.size);
	}

	if(entry.indexRange.size) {
		indexAllocator.Free(entry.indexRange.offset, entry.indexRange.size);
	}

	entry = {};
	return { true, {} };
}

void GpuGeometryAtlasBackend::ReserveBytes(size_t vertexBytes, size_t indexBytes) {
	using namespace weave::opengl;

	if (vertexBytes > vertexBuffer.GetBufferByteSize()) {
		vertexBuffer.Create(nullptr, vertexBytes, BufferUsage::Write);
		vertexAllocator = weave::memory::RangeAllocator(vertexBytes);
	}

	if (indexBytes > indexBuffer.GetBufferByteSize()) {
		indexBuffer.Create(nullptr, indexBytes, BufferUsage::Write);
		indexAllocator = weave::memory::RangeAllocator(indexBytes);
	}
}

void GpuGeometryAtlasBackend::ReserveGeometry(size_t vertexCount, size_t indexCount, size_t vertexStrideBytes) {
    const size_t vertexBytes = SafeByteSize(vertexCount, vertexStrideBytes);

    size_t indexStride = sizeof(uint32_t);
    const size_t indexBytes = SafeByteSize(indexCount, indexStride);

    ReserveBytes(vertexBytes, indexBytes);
}

std::pair<size_t, size_t> GpuGeometryAtlasBackend::ComputeRequiredByteSize(MeshData const& mesh) const {
	auto vertexStride = targetLayout.buffers[0].stride;
    auto vertexCount = mesh.GetUniqueVertexCount();
    auto indexStride = 0;
    auto indexCount = 0;

	indexStride = sizeof(uint32_t);
    indexCount = mesh.GetIndexCount();

    auto vertexByteSize = vertexStride * vertexCount;
    auto indexByteSize = indexStride * indexCount;

	return {vertexByteSize, indexByteSize};
}

std::pair<size_t, size_t> GpuGeometryAtlasBackend::AllocateBufferBytes(size_t vertexBytes, size_t indexBytes) {
	constexpr auto kInvalidOffset = size_t(-1);
	
	size_t vertexTargetOffset = 0;
    if (vertexBytes) {
        std::scoped_lock lock(bufferMutex);
        vertexTargetOffset = vertexAllocator.Allocate(vertexBytes);
        if(vertexTargetOffset == kInvalidOffset) {
            vertexAllocator.Grow(vertexBytes);
            vertexBuffer.GrowBuffer(vertexBytes);
            buffersDirty.store(true, std::memory_order_release);
            vertexTargetOffset = vertexAllocator.Allocate(vertexBytes);
        }
    }

    size_t indexTargetOffset = 0;
    if(indexBytes) {
        std::scoped_lock lock(bufferMutex);
        indexTargetOffset = indexAllocator.Allocate(indexBytes);
        if(indexTargetOffset == kInvalidOffset) {
            indexAllocator.Grow(indexBytes);
            indexBuffer.GrowBuffer(indexBytes);
            buffersDirty.store(true, std::memory_order_release);
            indexTargetOffset = indexAllocator.Allocate(indexBytes);
        }
    }

	return {vertexTargetOffset, indexTargetOffset};
}

std::unique_lock<std::mutex> GpuGeometryAtlasBackend::LockBufferAccess() const {
    return std::unique_lock<std::mutex>(bufferMutex);
}

} // namespace weave::graphics::gl::gpu
