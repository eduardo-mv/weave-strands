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
	template<typename T = void>
	T* Memory() const {
		return buffer.Memory<T>(byteOffset);
	}

	bool operator==(MeshBufferView const& other) const {
		return
			byteOffset == other.byteOffset &&
			byteSize == other.byteSize &&
			buffer == other.buffer;
	};

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
};

//Index buffer definition
//An index is an array of 16 or 32bit values for indexed geometry. For geom without index, the index count value represents the amount of vertices in the geometry data
struct MeshIndex {
	uint32_t count = 0; //Total amount of indices in the buffer or number of vertices in the data if no index
	types::DataType type = types::DataType::UInt32; //Data type of the index (16 or 32bit uint)
	MeshBufferView bufferView; //The data buffer with the indices
	bool primitiveRestart = false; //Indicates if primitive restart is used for this index

	size_t TypeSize() const { return types::GetRuntimeTypeTraits(type).byteSizeUnit; }
	void Indexless(uint32_t vertexCount) {
		count = vertexCount;
		type = types::DataType::UserExtended;
		bufferView = {};
	}

	bool IsIndexless() const { return type == types::DataType::UserExtended; }
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

	std::vector<MeshAttribute> const& GetAttributes() const;
	MeshAttribute const& GetAttribute(uint32_t channelId) const;
	MeshAttribute const& GetAttribute(MeshAttribute::Label label, uint32_t skip = 0) const;
	bool UpdateAttribute(MeshAttribute const& attribute);

	//Section access
	std::vector<MeshSection> const& GetSections() const;
	MeshSection const& GetSection(uint32_t sectionId) const;
	MeshSection GetSectionByMaterial(uint32_t matId, uint32_t offset = 0) const;
	MeshSection GetSectionByLabel(uint32_t label, uint32_t offset = 0) const;
	
	//Returns a mesh primitive given a name
	static MeshPrimitive GetPrimitiveType(std::string const &name);

private:
	bool IsValidBuffer(MeshBuffer buffer) const;
	uint32_t FindFreeAttributeChannel() const;

	//TODO: Utility functions
	//FuseMesh: Fuse one mesh after another into the same Mesh instance
	//SubMesh: Generate a new mesh from a specified section of the mesh
	//SwapYZ: Apply a 90º X rotation to swap Y and Z. Use sematic labels to detect correct channels
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
}