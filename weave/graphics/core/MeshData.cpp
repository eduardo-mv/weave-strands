#include "MeshData.h"
#include <cstring>
#include <algorithm>
#include <unordered_map>

using namespace weave;
using namespace graphics;

MeshData::MeshData(MeshData const& other)
{
	*this = other;
}

MeshData::~MeshData()
{
	for (auto& buffer : buffers) {
		std::free(buffer.data);
	}
}

MeshData& MeshData::operator=(MeshData const& other)
{
	for (auto& buffer : buffers) {
		std::free(buffer.data);
	}

	primitive = other.primitive;
	index = other.index;
	sections = other.sections;
	attributes = other.attributes;
	buffers = other.buffers;

	//Allocate new memory for the buffers
	for (auto& buffer : buffers) {
		void* mem = std::malloc(buffer.byteSize);
		std::memcpy(mem, buffer.data, buffer.byteSize);

		//Rebind any attributes or index that use this buffer
		for (auto& attr : attributes) {
			if (attr.stream.bufferView.buffer.data == buffer.data) {
				attr.stream.bufferView.buffer.data = mem;
			}
		}

		if (index.bufferView.buffer.data == buffer.data) {
			index.bufferView.buffer.data = mem;
		}

		//Rebind the buffer itself
		buffer.data = mem;
	}

	return *this;
}

MeshBuffer MeshData::AllocateMemory(uint64_t byteSize, void const* dataToCopy, bool isMutable)
{
	MeshBuffer buffer{ byteSize, std::malloc(byteSize), isMutable ? MeshBuffer::Usage::Write : MeshBuffer::Usage::Read };
	if (dataToCopy) {
		std::memcpy(buffer.Memory(0), dataToCopy, byteSize);
	}
	buffers.emplace_back(buffer);
	return buffer;
}

MeshDataStream MeshData::AllocateStream(uint32_t elemByteStride, uint64_t byteSize, void const* dataToCopy, bool isMutable)
{
	auto buffer = AllocateMemory(byteSize, dataToCopy, isMutable);

	return MeshDataStream{ elemByteStride, 0, byteSize, buffer };
}

MeshIndex MeshData::AllocateIndex(types::DataType type, uint32_t indicesCount, bool primitiveRestart, void const* dataToCopy, bool isMutable)
{
	auto byteSize = indicesCount * types::GetRuntimeTypeTraits(type).byteSizeVector;
	index.count = indicesCount;
	index.type = type;
	index.primitiveRestart = primitiveRestart;
	index.bufferView = {
		0, //byteOffset
		byteSize,
		AllocateMemory(byteSize, dataToCopy, isMutable)
	};

	return index;
}

bool MeshData::AddAttribute(MeshAttribute const& attribute)
{
	//Make sure this attribute isn't already present (the channel id is not taken)
	uint32_t autoChannel = attribute.channelId;

	if (autoChannel != ~0u) {
		for (auto const& attrib : attributes) {
			if (attrib.channelId == autoChannel)
				return false;
		}
	}
	else {
		//Attempt to deduce the target channel if not set
		autoChannel = static_cast<uint32_t>(attribute.label);
		for (auto const& attrib : attributes) {
			if (attrib.channelId == autoChannel) {
				autoChannel = FindFreeAttributeChannel();
			}
		}
	}
	
	//Make sure this attribute refers to a real existing buffer
	if (IsValidBuffer(attribute.stream.bufferView.buffer)) {
		attributes.emplace_back(attribute);
		attributes.back().channelId = autoChannel;

		std::sort(std::begin(attributes), std::end(attributes), [](MeshAttribute const& a, MeshAttribute const& b) {
			return a.channelId < b.channelId;
		});

		return true;
	}


	return false;
}

bool MeshData::SetIndex(MeshIndex const& indexIn)
{
	if (IsValidBuffer(indexIn.bufferView.buffer)) {
		index = indexIn;
		return true;	
	}

	return false;
}

//Removes the index and sets the amount of vertices for the mesh. If no value is passed, the expected vertex count will be automatically calculated based on the byteStride of the buffers
void MeshData::NoIndex(uint32_t vertexCount) {

	if(vertexCount == ~0u) {
		//Attempt to figure out the amount of vertices by looking at the strides on the attribute streams
		for(auto const &attrib : attributes) {
			vertexCount = attrib.stream.VertexCount();
			if(vertexCount != 0) {
				break;
			}
		}
	}

	index.Indexless(vertexCount);
}

bool MeshData::AddSection(MeshSection const& section)
{
	uint32_t autoLabel = 0;

	for (auto const& i : sections) {
		if (i == section)
			return false;

		autoLabel = std::max(i.label, autoLabel);
	}
	
	sections.emplace_back(section);

	if (section.label == ~0u) {
		sections.back().label = autoLabel + 1;
	}

	return true;
}

void MeshData::SingleSection(uint32_t label, uint32_t materialChannel)
{
	sections.clear();
	sections.emplace_back(MeshSection{ label,  materialChannel, 0, index.count });
}

bool MeshData::HasIndex() const
{
	return !index.IsIndexless();
}

std::vector<MeshBuffer> const& MeshData::GetBuffers() const
{
	return buffers;
}

MeshBuffer const& MeshData::GetBuffer(uint32_t bufferId) const
{
	static MeshBuffer empty;
	return buffers.size() > bufferId ? buffers[bufferId] : empty;
}

uint32_t MeshData::GetBufferId(MeshBuffer const& buffer) const
{
	uint32_t bufferId = 0;
	for (auto const& b : buffers) {
		if (b == buffer) {
			return bufferId;
		}
		++bufferId;
	}

	return ~0u;
}

std::vector<MeshDataStream> MeshData::GetUniqueStreams() const
{
	std::vector<MeshDataStream> streams;

	for (auto const& att : attributes) {
		bool found = false;
		for (auto const& s : streams) {
			if (s == att.stream) {
				found = true;
				break;
			}
		}
		if (!found) {
			streams.emplace_back(att.stream);
		}
	}

	return streams;
}

std::vector<MeshAttribute> const& MeshData::GetAttributes() const
{
	return attributes;
}

MeshAttribute const& MeshData::GetAttribute(uint32_t channelId) const
{
	static MeshAttribute empty;
	return attributes.size() > channelId ? attributes[channelId] : empty;
}

MeshAttribute const& MeshData::GetAttribute(MeshAttribute::Label label, uint32_t skip) const
{
	uint32_t count = 0;
	for (auto const& attrib : attributes) {
		if (attrib.label == label) {
			if (count == skip) {
				return attrib;
			}
			++count;
		}
	}

	static MeshAttribute empty;
	return empty;
}

bool MeshData::UpdateAttribute(MeshAttribute const& attribute)
{
	for (auto & attrib : attributes) {
		if (attrib.channelId == attribute.channelId) {
			attrib = attribute;
			return true;
		}
	}
	
	return false;
}

std::vector<MeshSection> const& MeshData::GetSections() const
{
	return sections;
}

MeshSection const& MeshData::GetSection(uint32_t sectionId) const
{
	static MeshSection empty;
	return sections.size() > sectionId ? sections[sectionId] :empty;
}

MeshSection MeshData::GetSectionByMaterial(uint32_t matId, uint32_t offset) const
{
	uint32_t count = 0;
	for (auto const& section : sections) {
		if (section.materialChannel == matId) {
			if (count == offset) {
				return section;
			}
			++count;
		}
	}

	return {};
}

MeshSection MeshData::GetSectionByLabel(uint32_t label, uint32_t offset) const
{
	uint32_t count = 0;
	for (auto const& section : sections) {
		if (section.label == label) {
			if (count == offset) {
				return section;
			}
			++count;
		}
	}

	return {};
}

//Returns a mesh primitive given a name
MeshPrimitive MeshData::GetPrimitiveType(std::string const & name) {
	static std::unordered_map<std::string, MeshPrimitive> map = {
		{"triangles", MeshPrimitive::Triangles},
		{"tristrip", MeshPrimitive::TriStrip},
		{"triangle_strip", MeshPrimitive::TriStrip},
		{"trianglestrip", MeshPrimitive::TriStrip},
		{"points", MeshPrimitive::Points},
		{"line_strip", MeshPrimitive::LineStrip},
		{"linestrip", MeshPrimitive::LineStrip},
		{"lines", MeshPrimitive::Lines}
	};

	auto it = map.find(name);
	if(it != map.end())
		return it->second;
	else
		return MeshPrimitive::Triangles;
}

bool MeshData::IsValidBuffer(MeshBuffer buffer) const
{
	for (auto const& b : buffers) {
		if (b == buffer) {
			return true;
		}
	}

	return false;
}

uint32_t MeshData::FindFreeAttributeChannel() const
{
	uint32_t channel = static_cast<uint32_t>(MeshAttribute::Label::Generic);

	for (auto const& attrib : attributes) {
		if (attrib.channelId > channel) {
			return channel;
		}
		else if (attrib.channelId == channel) {
			++channel;
		}
	}

	return channel;
}
