/*
Platform agnostic mesh data
The structure of the mesh contains buffers, streams, attributes, sections and index.

Buffers: Memory blocks with the actual data. The mesh can have multiple blocks where data is stored either interleaved or in sequential blocks.
Streams: Define how to access a memory block, serving as a view into a buffer. Contains the buffer address, the byte offset, byte stride and size pointed by the stream.
Attributes: Defines the actual type of an attribute (float, float3, int...) and uses a stream to determine how to access this attribute within a buffer
Index: A specialized attribute with information on which buffer the index can be found and with what configuration. Works as a simplified attribute + stream combination. 
Sections: A subdivision of the Index, tagged with a label and material id.

This format allows creating a versitile mesh system where data is well separated from its concrete definition.
For example, a few buffers can contain the different data blocks for a mesh (positions, colors, normals...).
Attributes will indicate which buffers to use, defining how to access them using streams. 
The streams allow completely different attributes to access the same buffer, even then same memory section of the buffer, and interpret the section differently (e.g. reuse normal data as color...)
The stream abstraction enables exploiting similar mechanisms found in gfx APIs like Vulkan.

*/

#pragma once

#include "weave/system/memory/DataType.h"
#include <vector>
#include <span>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace weave::graphics {
	
	class Mesh;

//Buffer definition that holds a memory block
//Stream will point at offsets within a memory block
//The memory is owned
class MeshBuffer {
public:
	friend class MeshLoader;
	friend class MeshData;
	friend struct MeshDataStream;
	friend struct MeshIndex;
	
protected:
	enum class Usage {
		Read,
		Write
	};

	uint64_t byteSize = 0;
	void *data = nullptr; 
	Usage usage = Usage::Read;

	MeshBuffer(uint64_t bufferSize, void* data, Usage usage = Usage::Read)
		: byteSize(bufferSize)
		, data(data)
		, usage(usage) {}

public:
	MeshBuffer() = default;
	MeshBuffer(MeshBuffer &&) noexcept = default;
	MeshBuffer(MeshBuffer const &) = default;
	MeshBuffer& operator=(MeshBuffer const&) = default;
	MeshBuffer& operator=(MeshBuffer &&) = default;
	
	bool operator==(MeshBuffer const& other) const { return byteSize == other.byteSize && data == other.data; }

	//Gets a pointer to the data
	template<typename T = void>
	T* Memory(uint64_t byteOffset = 0) const {
		return reinterpret_cast<T*>(static_cast<uint8_t*>(data) + byteOffset);
	}

	uint64_t Size() const { return byteSize; }

	bool IsReadOnly() const { return usage == Usage::Read; }
};

struct MeshBufferView {
	uint64_t byteOffset = 0; //Offset in bytes related to the memory buffer where this view starts
	uint64_t byteSize = 0; //Byte size of the view
	MeshBuffer buffer; //Buffer which the view refers to

	//Gets a pointer to the data
	template<typename T = std::byte>
	T* Memory(uint64_t offset = 0) const {
		return buffer.Memory<T>(byteOffset + offset);
	}

	bool operator==(MeshBufferView const& other) const {
		return
			byteOffset == other.byteOffset &&
			byteSize == other.byteSize &&
			buffer == other.buffer;
	};

	template<typename BytePtr>
	struct StridedIterator {
		using iterator_category = std::forward_iterator_tag;
		using value_type = BytePtr;
		using difference_type = std::ptrdiff_t;
		using pointer = BytePtr;
		using reference = BytePtr;

		BytePtr current{};
		BytePtr end{};
		size_t stride = 1;

		StridedIterator() = default;
		StridedIterator(BytePtr start, BytePtr finish, size_t step)
			: current(start)
			, end(finish)
			, stride(step ? step : 1) {}

		reference operator*() const { return current; }

		StridedIterator& operator++() {
			if (current < end) {
				const auto next = current + stride;
				current = next < end ? next : end;
			}
			return *this;
		}

		friend bool operator==(StridedIterator const& a, StridedIterator const& b) {
			return a.current == b.current;
		}

		friend bool operator!=(StridedIterator const& a, StridedIterator const& b) {
			return !(a == b);
		}
	};

	using iterator = StridedIterator<std::byte*>;
	using const_iterator = StridedIterator<std::byte const*>;

	iterator begin() { return BeginStrided(0, 1); }
	iterator end() { return EndStrided(1); }
	const_iterator begin() const { return BeginStrided(0, 1); }
	const_iterator end() const { return EndStrided(1); }

	iterator BeginStrided(uint64_t offset, uint32_t stride) {
		auto start = buffer.Memory<std::byte>(byteOffset + offset);
		auto finish = buffer.Memory<std::byte>(byteOffset + byteSize);
		return iterator{ start, finish, stride };
	}
	iterator EndStrided(uint32_t stride) {
		auto finish = buffer.Memory<std::byte>(byteOffset + byteSize);
		return iterator{ finish, finish, stride };
	}
	const_iterator BeginStrided(uint64_t offset, uint32_t stride) const {
		auto start = buffer.Memory<std::byte const>(byteOffset + offset);
		auto finish = buffer.Memory<std::byte const>(byteOffset + byteSize);
		return const_iterator{ start, finish, stride };
	}
	const_iterator EndStrided(uint32_t stride) const {
		auto finish = buffer.Memory<std::byte const>(byteOffset + byteSize);
		return const_iterator{ finish, finish, stride };
	}

};

//Data stream definition
struct MeshDataStream {
	uint32_t elemByteStride = 0; //Size in bytes of each element block within the stream. For interleaved data this indicates the size of the interleaved block
	MeshBufferView bufferView;

	//Return the vertex count calculated as byteSize / elemByteStride. Returns 0 if byteStride is 0.
	uint32_t VertexCount() const { return static_cast<uint32_t>(bufferView.byteSize / elemByteStride); }

	bool operator==(MeshDataStream const& other) const { 
		return
			elemByteStride == other.elemByteStride &&
			bufferView == other.bufferView;
	}

    auto begin() { return bufferView.BeginStrided(0, elemByteStride); }
    auto end()   { return bufferView.EndStrided(elemByteStride); }
    auto begin() const { return bufferView.BeginStrided(0, elemByteStride); }
    auto end()   const { return bufferView.EndStrided(elemByteStride); }

};

//Attribute channel definition
//Defines the attribute structure to access a related data buffer
struct MeshAttribute {
	enum class Label {
		Position,
		UVCoord,
		Color,
		Normal,
		Tangent,
		Bitangent,
		JointWeight,
		JointIndex,
		Generic
	};

	MeshAttribute::Label label = MeshAttribute::Label::Generic; //Semantic label for the attribute
	uint32_t channelId = ~0u; //The attribute channel id, related to the shader input. If not set, autodeduction from label will be applied
	types::DataType type = types::DataType::Void; //Data type of the attribute
	uint32_t strideOffset = 0; //Byte offset within the related stream's stride. For interleaved data, indicates the offset applied to each interleaved stride to locate this attribute
	MeshDataStream stream; //The related stream that this attribute feeds from

	//Returns the size of the type associated with this attribute
	size_t TypeSize() const { return types::GetRuntimeTypeTraits(type).byteSizeUnit; }
	size_t ElementByteSize() const { return types::GetRuntimeTypeTraits(type).byteSizeVector; }

	uint32_t ElementStride() const {
	    auto stride = stream.elemByteStride;
	    if (stride == 0) {
	        stride = static_cast<uint32_t>(types::GetRuntimeTypeTraits(type).byteSizeVector);
	    }
	    return stride;
	}

	template<typename BytePtr>
	struct ElementRef {
		using ByteType = std::conditional_t<
			std::is_const_v<std::remove_pointer_t<BytePtr>>,
			const std::byte,
			std::byte
		>;

		BytePtr data = nullptr;
		types::DataType dataType = types::DataType::Void;
		size_t byteSize = 0;

		template<typename T>
		T Read() const {
			return types::DynamicTypeConvert<T>(reinterpret_cast<std::byte const*>(data), dataType);
		}

		template<typename T>
		void Write(T const& value) const {
			types::DynamicTypeConvert(value, reinterpret_cast<std::byte*>(data), dataType);
		}

		std::span<ByteType> Bytes() const { return { data, byteSize }; }
		template<typename T = std::byte>
		T const* Data() const { return reinterpret_cast<T const*>(data); }
		size_t Size() const { return byteSize; }
		types::DataType AttributeType() const { return dataType; }
		bool IsValid() const { return data != nullptr && byteSize > 0; }
		explicit operator bool() const { return IsValid(); }
	};

	template<typename IteratorType, typename BytePtr>
	struct ElementIterator {
		using iterator_category = std::forward_iterator_tag;
		using value_type = ElementRef<BytePtr>;
		using difference_type = std::ptrdiff_t;

		IteratorType it{};
		types::DataType dataType = types::DataType::Void;
		size_t byteSize = 0;

		value_type operator*() const { return { *it, dataType, byteSize }; }
		ElementIterator& operator++() { ++it; return *this; }
		friend bool operator==(ElementIterator const& a, ElementIterator const& b) { return a.it == b.it; }
		friend bool operator!=(ElementIterator const& a, ElementIterator const& b) { return !(a == b); }
	};

	template<typename IteratorType, typename BytePtr>
	struct ElementRange {
		using iterator = ElementIterator<IteratorType, BytePtr>;

		iterator first;
		iterator last;

		iterator begin() const { return first; }
		iterator end() const { return last; }
		bool empty() const { return first == last; }
	};

	using ElementRangeType = ElementRange<MeshBufferView::iterator, std::byte*>;
	using ConstElementRangeType = ElementRange<MeshBufferView::const_iterator, std::byte const*>;
	using ElementRefType = ElementRef<std::byte*>;
	using ConstElementRefType = ElementRef<std::byte const*>;

	ElementRefType Element(uint32_t vertexIndex) {
		return MakeElementRef<std::byte>(vertexIndex);
	}

	ConstElementRefType Element(uint32_t vertexIndex) const {
		return MakeElementRef<const std::byte>(vertexIndex);
	}

	ElementRangeType Elements() {
		auto stride = ElementStride();
		auto byteSize = ElementByteSize();
		return ElementRangeType{
			{ stream.bufferView.BeginStrided(strideOffset, stride), type, byteSize },
			{ stream.bufferView.EndStrided(stride), type, byteSize }
		};
	}

	ConstElementRangeType Elements() const {
		auto stride = ElementStride();
		auto byteSize = ElementByteSize();
		return ConstElementRangeType{
			{ stream.bufferView.BeginStrided(strideOffset, stride), type, byteSize },
			{ stream.bufferView.EndStrided(stride), type, byteSize }
		};
	}

    auto begin() { return stream.bufferView.BeginStrided(strideOffset, ElementStride()); }
    auto end()   { return stream.bufferView.EndStrided(ElementStride()); }
	auto begin() const { return stream.bufferView.BeginStrided(strideOffset, ElementStride()); }
	auto end()   const { return stream.bufferView.EndStrided(ElementStride()); }

private:
	template<typename ByteType>
	ElementRef<ByteType*> MakeElementRef(uint32_t vertexIndex) const {
		auto stride = ElementStride();
		auto byteSize = ElementByteSize();
		if (stride == 0 || byteSize == 0) {
			return {};
		}

		uint64_t offset = strideOffset + static_cast<uint64_t>(vertexIndex) * stride;
		if (offset + byteSize > stream.bufferView.byteSize) {
			return {};
		}

		auto bufferOffset = stream.bufferView.byteOffset + offset;
		return { stream.bufferView.buffer.Memory<ByteType>(bufferOffset), type, byteSize };
	}
};

//Index buffer definition
//An index is an array of 16 or 32bit values for indexed geometry. For geom without index, the index count value represents the amount of vertices in the geometry data
struct MeshIndex {
	uint32_t count = 0; //Total amount of indices in the buffer or number of vertices in the data if no index
	types::DataType type = types::DataType::UInt32; //Data type of the index (16 or 32bit uint)
	MeshBufferView bufferView; //The data buffer with the indices
	bool primitiveRestart = false; //Indicates if primitive restart is used for this index

	size_t TypeSize() const { return types::GetRuntimeTypeTraits(type).byteSizeUnit; }
	uint32_t ElementStride() const { return static_cast<uint32_t>(TypeSize()); }
	void Indexless(uint32_t vertexCount) {
		count = vertexCount;
		type = types::DataType::UserExtended;
		bufferView = {};
	}

	bool IsIndexless() const { return type == types::DataType::UserExtended; }

	template<typename T = uint32_t>
	T Read(uint32_t position) const {
		if (position >= count || IsIndexless() || type == types::DataType::Void) {
			return T{};
		}

		auto stride = ElementStride();
		if (stride == 0) {
			return T{};
		}

		auto offset = static_cast<uint64_t>(position) * stride;
		if (offset + stride > bufferView.byteSize) {
			return T{};
		}

		auto data = bufferView.Memory<std::byte const>(offset);
		if (!data) {
			return T{};
		}

		return types::DynamicTypeConvert<T>(data, type);
	}

	template<typename T>
	bool Write(uint32_t position, T const& value) const {
		if (position >= count || IsIndexless() || type == types::DataType::Void) {
			return false;
		}

		auto stride = ElementStride();
		if (stride == 0) {
			return false;
		}

		auto offset = static_cast<uint64_t>(position) * stride;
		if (offset + stride > bufferView.byteSize) {
			return false;
		}

		auto data = bufferView.Memory<std::byte>(offset);
		if (!data) {
			return false;
		}

		types::DynamicTypeConvert(value, data, type);
		return true;
	}
};

//Section definition
//A section is used to delimit a piece of the mesh index with external information, like related material
//If the mesh has no real index, the start and end markers indicate the vertex numbers
struct MeshSection {
	uint32_t label = ~0u; //User defined label
	uint32_t materialChannel = 0; //User defined material channel
	uint32_t indexStart = 0; //First index entry of the section
	uint32_t indexCount = 0; //Amount of indices to use

	bool operator==(MeshSection const& other) const {
		return
			label == other.label && materialChannel == other.materialChannel &&
			indexStart == other.indexStart && indexCount == other.indexCount;
	}
};

//Render primitive enum
//This is used for standarized Render commands so that we can encapsulate Mesh rendering calls
enum class MeshPrimitive {
	Points,
	Lines,
	LineStrip,
	Triangles,
	TriStrip,
};

//Mesh definition
class MeshData {
private:
	MeshPrimitive primitive{}; //The primite represented by the mesh data
	MeshIndex index; //The index of vertices
	std::vector<MeshSection> sections; //Sections of the mesh from the index
	std::vector<MeshAttribute> attributes; //The vertex atributes fetched from streams
	std::vector<MeshBuffer> buffers; //The memory storage

public:
	template<typename MeshT>
	class VertexProxyBase;
	using VertexProxy = VertexProxyBase<MeshData const>;
	using MutableVertexProxy = VertexProxyBase<MeshData>;

	template<typename MeshT>
	class VertexRangeBase;
	using VertexRange = VertexRangeBase<MeshData const>;
	using MutableVertexRange = VertexRangeBase<MeshData>;

	MeshData() = default;
	MeshData(MeshData const &other);
	MeshData(MeshData&&) noexcept = default;
	~MeshData();

	MeshData& operator=(MeshData const& other);
	MeshData& operator=(MeshData&&) noexcept = default;

	void SetPrimitive(MeshPrimitive mprimitive) { primitive = mprimitive; }
	MeshPrimitive Primitive() const { return primitive; }

	//Memory allocation
	MeshBuffer AllocateMemory(uint64_t byteSize, void const *dataToCopy = nullptr, bool isMutable = false);
	//Calls AllocateMemory and creates a stream with the added information
	MeshDataStream AllocateStream(uint32_t elemByteStride, uint64_t byteSize, void const* dataToCopy = nullptr, bool isMutable = false);
	//Calls AllocateMemory and creates a buffer and an index that uses that buffer
	MeshIndex AllocateIndex(types::DataType type, uint32_t indicesCount, bool primitiveRestart, void const* dataToCopy = nullptr, bool isMutable = false);

	//Attribute creation
	bool AddAttribute(MeshAttribute const &attribute);
	//Index creation. Only one index is allowed
	bool SetIndex(MeshIndex const &index);
	//Removes the index and sets the amount of vertices for the mesh. If no value is passed, the expected vertex count will be automatically guessed based on the current streams
	void NoIndex(uint32_t vertexCount = ~0u);

	//Section creation
	bool AddSection(MeshSection const& section);
	void SingleSection(uint32_t label, uint32_t materialChannel);

	//Returns the index
	MeshIndex const& GetIndex() const { return index; }
	//Returns true if the mesh has a real index
	bool HasIndex() const;

	//Data access
	std::vector<MeshBuffer> const& GetBuffers() const;
	MeshBuffer const& GetBuffer(uint32_t bufferId) const;
	uint32_t GetBufferId(MeshBuffer const& buffer) const;

	std::vector<MeshDataStream> GetUniqueStreams() const;

	std::vector<std::pair<MeshAttribute::Label, uint32_t>> GetAttributeLabels() const;

	std::vector<MeshAttribute> const& GetAttributes() const;
	MeshAttribute const& GetAttribute(uint32_t channelId) const;
	MeshAttribute const& GetAttribute(MeshAttribute::Label label, uint32_t skip = 0) const;
	MeshAttribute const* FindAttribute(uint32_t channelId) const;
	MeshAttribute const* FindAttribute(MeshAttribute::Label label, uint32_t skip = 0) const;
	MeshAttribute* FindAttributeMutable(uint32_t channelId);
	MeshAttribute* FindAttributeMutable(MeshAttribute::Label label, uint32_t skip = 0);

	//Section access
	std::vector<MeshSection> const& GetSections() const;
	MeshSection const& GetSection(uint32_t sectionId) const;
	MeshSection GetSectionByMaterial(uint32_t matId, uint32_t offset = 0) const;
	MeshSection GetSectionByLabel(uint32_t label, uint32_t offset = 0) const;
	
	//Returns a mesh primitive given a name
	static MeshPrimitive GetPrimitiveType(std::string const &name);

	//Returns the amount of vertices based on the index or attributes set
	uint32_t GetUniqueVertexCount() const;

	//Returns the real count of indices (0 if HasIndex is false)
	uint32_t GetIndexCount() const;

	//Vertex traversal
	MutableVertexRange UniqueVertices();
	VertexRange UniqueVertices() const;
	MutableVertexRange UniqueVertices(uint32_t start, uint32_t count);
	VertexRange UniqueVertices(uint32_t start, uint32_t count) const;

	MutableVertexRange IndexedVertices();
	VertexRange IndexedVertices() const;
	MutableVertexRange IndexedVertices(uint32_t start, uint32_t count);
	VertexRange IndexedVertices(uint32_t start, uint32_t count) const;

private:
	bool IsValidBuffer(MeshBuffer buffer) const;
	uint32_t FindFreeAttributeChannel() const;

	//TODO: Utility functions
	//FuseMesh: Fuse one mesh after another into the same Mesh instance
	//SubMesh: Generate a new mesh from a specified section of the mesh
	//SwapYZ: Apply a 90� X rotation to swap Y and Z. Use sematic labels to detect correct channels
	//InvertWinding: Invert the winding of the mesh data or the index
	//Transform: Apply a given transform to the chosen attribute
	//TransformGeom: Apply a given transform using the semantic labels to detect the actual effect. Positions are tranformed, normals, tangents anb bt are only rotated
	//GenNormals: Generate vertex normals
	//GenTangents: Generate vertex tangents and bitangents for the chosen UV channel
	//Collapse: Cleans the internal data by looking for identical vertices and removing them
	//Expand: Expands the geometry data by cloning vertices according to the index. The index is then discarded
	//MergeByMaterial: Restructure the data so that all sections with the same material are fused into a single contiguous section
	//SortSpherical: Sort triangles in respect to a point in space. Respect section limits
	//SortProjection: Sort according to the projected triangles on the given matrix
	//Saves a binary file for version 100
};

template<typename MeshT>
class MeshData::VertexProxyBase {
private:
	using AttributeType = std::conditional_t<std::is_const_v<MeshT>, MeshAttribute const, MeshAttribute>;
	using AttributeRefType = std::conditional_t<std::is_const_v<MeshT>, MeshAttribute::ConstElementRefType, MeshAttribute::ElementRefType>;

public:
	VertexProxyBase() = default;
	VertexProxyBase(MeshT* meshData, uint32_t indexPos, uint32_t vertex)
		: mesh(meshData)
		, indexPosition(indexPos)
		, vertexIndex(vertex) {}

	uint32_t IndexPosition() const { return indexPosition; }
	uint32_t VertexIndex() const { return vertexIndex; }

	AttributeRefType Attribute(AttributeType& attribute) const {
		return attribute.Element(vertexIndex);
	}

	AttributeRefType Attribute(MeshAttribute::Label label, uint32_t skip = 0) const {
		auto attr = FindAttribute(label, skip);
		return attr ? Attribute(*attr) : AttributeRefType{};
	}

	bool HasAttribute(MeshAttribute::Label label, uint32_t skip = 0) const {
		return FindAttribute(label, skip) != nullptr;
	}

	template<typename T>
	void WriteAttribute(MeshAttribute::Label label, T const& value, uint32_t skip = 0) const {
		if constexpr (!std::is_const_v<MeshT>) {
			auto attr = FindAttribute(label, skip);
			if (attr) {
				auto element = attr->Element(vertexIndex);
				if (element) {
					element.Write(value);
				}
			}
		}
	}

private:
	AttributeType* FindAttribute(uint32_t channelId) const {
		if (!mesh) {
			return nullptr;
		}

		if constexpr (std::is_const_v<MeshT>) {
			return mesh->FindAttribute(channelId);
		}
		else {
			return mesh->FindAttributeMutable(channelId);
		}
	}

	AttributeType* FindAttribute(MeshAttribute::Label label, uint32_t skip) const {
		if (!mesh) {
			return nullptr;
		}

		if constexpr (std::is_const_v<MeshT>) {
			return mesh->FindAttribute(label, skip);
		}
		else {
			return mesh->FindAttributeMutable(label, skip);
		}
	}

	MeshT* mesh = nullptr;
	uint32_t indexPosition = 0;
	uint32_t vertexIndex = 0;
};

template<typename MeshT>
class MeshData::VertexRangeBase {
public:
	using ProxyType = VertexProxyBase<MeshT>;

	struct iterator {
		using iterator_category = std::forward_iterator_tag;
		using value_type = ProxyType;
		using difference_type = std::ptrdiff_t;

		VertexRangeBase const* range = nullptr;
		uint32_t indexPositionOffset = 0;

		value_type operator*() const { return range->MakeVertex(indexPositionOffset); }
		iterator& operator++() { ++indexPositionOffset; return *this; }
		friend bool operator==(iterator const& a, iterator const& b) {
			return a.range == b.range && a.indexPositionOffset == b.indexPositionOffset;
		}
		friend bool operator!=(iterator const& a, iterator const& b) {
			return !(a == b);
		}
	};

	iterator begin() const { return iterator{ this, 0 }; }
	iterator end() const { return iterator{ this, count }; }
	bool empty() const { return count == 0; }
	uint32_t size() const { return count; }

private:
	friend class MeshData;

	VertexRangeBase(MeshT& meshData, uint32_t startIndex, uint32_t vertexCount, bool useIndex)
		: mesh(&meshData)
		, start(startIndex)
		, count(vertexCount)
		, useIndex(useIndex) {}

	ProxyType MakeVertex(uint32_t indexPositionOffset) const {
		uint32_t indexPosition = start + indexPositionOffset;
		return { mesh, indexPosition, ResolveVertexIndex(indexPosition) };
	}

	uint32_t ResolveVertexIndex(uint32_t indexPosition) const {
		if (!mesh) {
			return 0;
		}

		if (!useIndex || !mesh->HasIndex()) {
			return indexPosition;
		}

		auto const& meshIndex = mesh->GetIndex();
		auto stride = static_cast<uint32_t>(meshIndex.TypeSize());
		if (stride == 0) {
			return 0;
		}

		auto relativeOffset = static_cast<uint64_t>(indexPosition) * stride;
		if (relativeOffset + stride > meshIndex.bufferView.byteSize) {
			return 0;
		}

		auto data = meshIndex.bufferView.template Memory<std::byte const>(relativeOffset);
		return types::DynamicTypeConvert<uint32_t>(data, meshIndex.type);
	}

	MeshT* mesh{};
	uint32_t start{};
	uint32_t count{};
	bool useIndex{};
};
}
