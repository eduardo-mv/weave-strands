#include "MeshLoader.h"
#include "weave/system/utf/Utf.h"
#include "weave/system/hash/Hash.h"

using namespace weave;
using namespace graphics;
using namespace types;

bool MeshLoader::WriteFile(MeshData const& mesh, std::filesystem::path const& filename, uint32_t version)
{
	switch (version) {
	case 0:
	case 101:
		return WriteFileV101(mesh, filename);
	case 100:
		return WriteFileV100(mesh, filename);
	default:
		return false;
	}
}

bool MeshLoader::WriteFileV101(MeshData const& mesh, std::filesystem::path const& filename)
{
	//Open the output file
	std::ofstream f(filename, std::ios::binary);
	if (!f.good())
		return false;
	//Store the standard header
	uint64_t lzid = *((uint64_t*)"WEAVEBIN");
	f.write((char*)& lzid, sizeof(lzid));
	
	uint64_t id_type = weave::HashString("Mesh");
	f.write((char*)& id_type, sizeof(id_type));

	//Store the file version
	uint32_t version = 101;
	f.write((char*)& version, sizeof(version));
	
	//Write all buffers
	{
		auto& buffers = mesh.GetBuffers();
		uint32_t bufferCount = uint32_t(buffers.size());
		f.write((char*)& bufferCount, sizeof(uint32_t));

		for (auto& buffer : buffers) {
			uint32_t usage = static_cast<uint32_t>(buffer.usage);
			uint64_t size = buffer.Size();
			f.write((char*)& usage, sizeof(uint32_t));
			f.write((char*)& size, sizeof(uint64_t));
			f.write(buffer.Memory<char>(0), buffer.Size());
		}
	}
	//Write all attributes with the related streams
	//References to buffers are done via internal ids
	{
		auto& attributes = mesh.GetAttributes();
		uint32_t attribCount = uint32_t(attributes.size());
		f.write((char*)& attribCount, sizeof(uint32_t));

		for (auto& attr : attributes) {
			//Attribute data
			uint32_t label = static_cast<uint32_t>(attr.label);
			f.write((char*)& label, sizeof(uint32_t));
			f.write((char*)& attr.channelId, sizeof(uint32_t));
			uint32_t dataType = static_cast<uint32_t>(attr.type);
			f.write((char*)& dataType, sizeof(uint32_t));
			f.write((char*)& attr.strideOffset, sizeof(uint32_t));
			//Stream data
			f.write((char*)& attr.stream.elemByteStride, sizeof(uint32_t));
			f.write((char*)& attr.stream.bufferView.byteOffset, sizeof(uint64_t));
			f.write((char*)& attr.stream.bufferView.byteSize, sizeof(uint64_t));
			uint32_t bufferId = mesh.GetBufferId(attr.stream.bufferView.buffer);
			f.write((char*)& bufferId, sizeof(uint32_t));
		}
	}

	//Primitive
	{
		uint32_t primitive = static_cast<uint32_t>(mesh.Primitive());
		f.write((char*)& primitive, sizeof(uint32_t));
	}

	//Index data
	{
		auto& index = mesh.GetIndex();
		f.write((char*)& index.count, sizeof(uint32_t));
		uint32_t dataType = static_cast<uint32_t>(index.type);
		f.write((char*)& dataType, sizeof(uint32_t));
		f.write((char*)& index.bufferView.byteOffset, sizeof(uint64_t));
		uint32_t bufferId = mesh.GetBufferId(index.bufferView.buffer);
		f.write((char*)& bufferId, sizeof(uint32_t));
		uint32_t restart = index.primitiveRestart ? 1 : 0;
		f.write((char*)& restart, sizeof(uint32_t));
	}

	//Sections
	{
		auto& sections = mesh.GetSections();
		uint32_t sectionCount = uint32_t(sections.size());
		f.write((char*)& sectionCount, sizeof(uint32_t));

		for (auto& section : sections) {
			f.write((char*)& section.label, sizeof(uint32_t));
			f.write((char*)& section.materialChannel, sizeof(uint32_t));
			f.write((char*)& section.indexStart, sizeof(uint32_t));
			f.write((char*)& section.indexCount, sizeof(uint32_t));
		}
	}

	return true;
}

bool MeshLoader::WriteFileV100(MeshData const& mesh, std::filesystem::path const& filename)
{
	//Open the output file
	std::ofstream f(filename, std::ios::binary);
	if (!f.good())
		return false;
	//Store the standard header
	uint64_t lzid = *((uint64_t*)"WEAVEBIN");
	f.write((char*)& lzid, sizeof(lzid));
	//RID
	uint64_t id_type = weave::HashString("Mesh");
	f.write((char*)& id_type, sizeof(id_type));
	

	//Store the file version
	uint32_t version = 100;
	f.write((char*)& version, sizeof(version));

	//Store the index
	{
		//Index size
		MeshIndex index = mesh.GetIndex();
		f.write((char*)& index.count, sizeof(index.count));
		//Usage flag
		if (index.bufferView.buffer.Size()) {
			int8_t usage = 0; //Deprecated
			f.write((char*)& usage, sizeof(usage));
			f.write(index.bufferView.buffer.Memory<char>(0), index.bufferView.buffer.Size());
		}
		else {
			int8_t usage = -1;
			f.write((char*)& usage, sizeof(usage));
		}
	}

	auto attributes = mesh.GetAttributes();

	//Store the amount of buffers in the mesh
	uint64_t count = attributes.size();
	f.write((char*)& count, sizeof(count));

	//Store the data buffers
	for (auto const& attr : attributes) {
		//Usage flag
		int8_t usage = 0; //Deprecated
		f.write((char*)& usage, sizeof(usage));
		//Byte stride
		uint64_t byteStride = attr.stream.elemByteStride;
		f.write((char*)& byteStride, sizeof(byteStride));
		//The byte size of the buffer
		f.write((char*)& attr.stream.bufferView.byteSize, sizeof(attr.stream.bufferView.byteSize));
		//The buffer data
		f.write(attr.stream.bufferView.buffer.Memory<char>(attr.stream.bufferView.byteOffset), attr.stream.bufferView.byteSize);
	}

	//Store the amount of attributes in the mesh
	count = attributes.size();
	f.write((char*)& count, sizeof(count));

	//Store the attributes
	uint64_t internalId = 0;
	for (auto const& att : attributes) {
		//The channel id
		uint64_t channelId = att.channelId;
		f.write((char*)& channelId, sizeof(channelId));
		//The semantic label
		int8_t byte = 0;
		f.write((char*)& byte, sizeof(byte));
		//The data type
		byte = int8_t(GetRuntimeTypeTraits(att.type).underlyingDataTypeValue );
		f.write((char*)& byte, sizeof(byte));
		//Buffer internal Id
		f.write((char*)& internalId, sizeof(internalId));
		//Element count and stride offset
		uint64_t elementCount = GetRuntimeTypeTraits(att.type).vectorSize;
		uint64_t strideOffset = att.strideOffset;
		f.write((char*)& elementCount, sizeof(elementCount));
		f.write((char*)& strideOffset, sizeof(strideOffset));

		++internalId;
	}

	//Store the section count
	auto sections = mesh.GetSections();
	count = sections.size();
	f.write((char*)& count, sizeof(count));

	//Store the sections
	for (auto const& section : sections) {
		uint64_t label = section.label;
		f.write((char*)& label, sizeof(label));
		uint64_t materialChannel = section.materialChannel;
		f.write((char*)& materialChannel, sizeof(materialChannel));
		uint64_t indexStart = section.indexStart;
		f.write((char*)& indexStart, sizeof(indexStart));
		uint64_t indexCount = section.indexCount;
		f.write((char*)& indexCount, sizeof(indexCount));
	}

	return true;
}


MeshData MeshLoader::Load(std::filesystem::path const& filepath)
{
	//Read the standard header to determine which version of the loader to use
	if (!std::filesystem::exists(filepath))
		return {};
	
	std::ifstream f(filepath, std::ifstream::binary);
	if (!f.is_open()) {
		return {};
	}

	return Load(f, false);
}

MeshData MeshLoader::Load(std::istream& f, bool skipHeader)
{
	if (!skipHeader) {
		//8 bytes for the main header
		//Must be a standard binary
		uint64_t lzid = 0;
		f.read((char*)& lzid, sizeof(lzid));
		if (lzid != *((uint64_t*)"WEAVEBIN")) {
			return {};
		}

		//Read the first 8 bytes of the standard header which contain the RID
		f.read((char*)& lzid, sizeof(lzid));
		if (lzid != 0 && lzid != weave::HashString("Mesh"))
			return {};
	}

	//Read the file version
	uint32_t version = 0;
	f.read((char*)& version, sizeof(version));
	switch (version) {
	case 101:
		return LoadFileV101(f);
	case 100:
		return LoadFileV100(f);
	case 99:
		return LoadFileV099(f);
	default:
		return {};
	}
}


MeshData MeshLoader::LoadFileV101(std::istream& f)
{
	MeshData mesh;

	//Read all buffers
	{
		uint32_t bufferCount = 0;
		f.read((char*)& bufferCount, sizeof(uint32_t));

		for (uint32_t i = 0; i < bufferCount; ++i) {
			uint32_t usage = 0;
			uint64_t size = 0;
			f.read((char*)& usage, sizeof(uint32_t));
			f.read((char*)& size, sizeof(uint64_t));
			auto buffer = mesh.AllocateMemory(size, nullptr, usage != 0);

			f.read(buffer.Memory<char>(0), buffer.Size());
		}
	}
	//Write all attributes with the related streams
	//References to buffers are done via internal ids
	{
		uint32_t attribCount = 0;
		f.read((char*)& attribCount, sizeof(uint32_t));

		for (uint32_t i = 0; i < attribCount; ++i) {
			MeshAttribute attr;

			//Attribute data
			uint32_t label = 0;
			f.read((char*)& label, sizeof(uint32_t));
			attr.label = static_cast<MeshAttribute::Label>(label);
			
			f.read((char*)& attr.channelId, sizeof(uint32_t));
			
			uint32_t dataType = 0;
			f.read((char*)& dataType, sizeof(uint32_t));
			attr.type = static_cast<DataType>(dataType);

			f.read((char*)& attr.strideOffset, sizeof(uint32_t));

			//Stream data
			f.read((char*)& attr.stream.elemByteStride, sizeof(uint32_t));
			f.read((char*)& attr.stream.bufferView.byteOffset, sizeof(uint64_t));
			f.read((char*)& attr.stream.bufferView.byteSize, sizeof(uint64_t));
			uint32_t bufferId = 0;
			f.read((char*)& bufferId, sizeof(uint32_t));
			attr.stream.bufferView.buffer = mesh.GetBuffer(bufferId);

			mesh.AddAttribute(attr);
		}
	}

	//Primitive
	{
		uint32_t primitive = 0;
		f.read((char*)& primitive, sizeof(uint32_t));
		mesh.SetPrimitive(static_cast<MeshPrimitive>(primitive));
	}

	//Index data
	{
		MeshIndex index;

		f.read((char*)& index.count, sizeof(uint32_t));
		
		uint32_t dataType = 0;
		f.read((char*)& dataType, sizeof(uint32_t));
		index.type = static_cast<DataType>(dataType);
		
		f.read((char*)& index.bufferView.byteOffset, sizeof(uint64_t));
		
		uint32_t bufferId = 0;
		f.read((char*)& bufferId, sizeof(uint32_t));
		index.bufferView.buffer = mesh.GetBuffer(bufferId);
		
		uint32_t restart = 0;
		f.read((char*)& restart, sizeof(uint32_t));
		index.primitiveRestart = (restart != 0);

		mesh.SetIndex(index);
	}

	//Sections
	{
		uint32_t sectionCount = 0;
		f.read((char*)& sectionCount, sizeof(uint32_t));

		for (uint32_t i = 0; i < sectionCount; ++i) {
			MeshSection section;

			f.read((char*)& section.label, sizeof(uint32_t));
			f.read((char*)& section.materialChannel, sizeof(uint32_t));
			f.read((char*)& section.indexStart, sizeof(uint32_t));
			f.read((char*)& section.indexCount, sizeof(uint32_t));

			mesh.AddSection(section);
		}
	}

	if (mesh.GetSections().empty())
		mesh.SingleSection(0, 0);

	return mesh;
}

MeshData MeshLoader::LoadFileV100(std::istream& f)
{
	MeshData mesh;

	//Read the index
	uint64_t count = 0;

	{
		//Index element count
		f.read((char*)& count, sizeof(count));
		//Usage flag. Ignored when creating buffers
		int8_t usage = -1;
		f.read((char*)& usage, sizeof(usage));

		MeshIndex index;
		index.bufferView.byteOffset = 0;
		index.count = (uint32_t)count;
		index.type = DataType::UInt32;

		if (usage != -1) {
			//Contains index data
			index.bufferView.buffer = mesh.AllocateMemory(count * sizeof(uint32_t));
			f.read(index.bufferView.buffer.Memory<char>(0), index.bufferView.buffer.Size());
		}

		mesh.SetIndex(index);
	}

	//Read the amount of buffers in the mesh
	f.read((char*)& count, sizeof(count));
	std::vector<MeshDataStream> streams;

	//Create and read the buffer data
	for (uint64_t i = 0; i < count; ++i) {
		//Usage flag. Ignored when creating buffers
		int8_t usage = 0;
		f.read((char*)& usage, sizeof(usage));
		//Byte stride
		uint64_t stride = 0;
		f.read((char*)& stride, sizeof(stride));
		//The byte size of the buffer
		uint64_t size = 0;
		f.read((char*)& size, sizeof(size));

		MeshDataStream stream;
		stream.bufferView.byteOffset = 0;
		stream.bufferView.byteSize = size;
		stream.elemByteStride = uint32_t(stride);
		stream.bufferView.buffer = mesh.AllocateMemory(size);
		streams.emplace_back(stream);

		//Data
		f.read(stream.bufferView.buffer.Memory<char>(0), stream.bufferView.buffer.Size());
	}


	//Read the amount of attributes in the mesh
	f.read((char*)& count, sizeof(count));
	//Create all attributes
	for (uint64_t i = 0; i < count; ++i) {
		//The channel id
		uint64_t channel = 0;
		f.read((char*)& channel, sizeof(channel));
		//The semantic label
		int8_t label = 0;
		f.read((char*)& label, sizeof(label));
		//The data type
		int8_t type = 0;
		f.read((char*)& type, sizeof(type));
		//Buffer internal Id
		uint64_t id = 0;
		f.read((char*)& id, sizeof(id));
		//Element count and stride offset
		uint64_t ecount = 0;
		f.read((char*)& ecount, sizeof(ecount));
		uint64_t stride = 0;
		f.read((char*)& stride, sizeof(stride));

		MeshAttribute attrib;
		attrib.channelId = uint32_t(channel);
		attrib.label = MeshAttribute::Label(label);
		attrib.strideOffset = uint32_t(stride);
		attrib.type = DataType(type);
		if (attrib.type == DataType::Float) {
			DataType realType[] = { DataType::Float, DataType::Float_2, DataType::Float_3, DataType::Float_4 };
			attrib.type = realType[ecount - 1];
		}
		if (attrib.type == DataType::UInt32) {
			DataType realType[] = { DataType::UInt32, DataType::UInt32_2, DataType::UInt32_3, DataType::UInt32_4 };
			attrib.type = realType[ecount - 1];
		}

		attrib.stream = streams[id];
		attrib.stream.elemByteStride = uint32_t(GetRuntimeTypeTraits(attrib.type).byteSizeVector);

		mesh.AddAttribute(attrib);
	}

	//Read the section count
	f.read((char*)& count, sizeof(count));
	//Create all sections
	for (uint64_t i = 0; i < count; ++i) {
		uint64_t label = 0;
		f.read((char*)& label, sizeof(label));
		uint64_t material = 0;
		f.read((char*)& material, sizeof(material));
		uint64_t start = 0;
		f.read((char*)& start, sizeof(start));
		uint64_t ecount = 0;
		f.read((char*)& ecount, sizeof(ecount));

		MeshSection section;
		section.indexCount = uint32_t(ecount);
		section.indexStart = uint32_t(start);
		section.label = uint32_t(label);
		section.materialChannel = uint32_t(material);

		mesh.AddSection(section);
	}
	
	if (mesh.GetSections().empty())
		mesh.SingleSection(0, 0);

	//Primitive was not stored under this format, but it's likely triangles
	mesh.SetPrimitive(MeshPrimitive::Triangles);
	

	return mesh;
}

MeshData MeshLoader::LoadFileV099(std::istream& f)
{
	MeshData mesh;

	//Old attribute codes mapped to current label codes
	static MeshAttribute::Label labels[] = { MeshAttribute::Label::Position, MeshAttribute::Label::Color, MeshAttribute::Label::Normal, MeshAttribute::Label::UVCoord,
		MeshAttribute::Label::Tangent, MeshAttribute::Label::Bitangent, MeshAttribute::Label::Generic, MeshAttribute::Label::JointWeight, MeshAttribute::Label::JointIndex };

	//Read the float attribute count
	uint32_t uint = 0;
	f.read((char*)& uint, sizeof(uint));

	//Read each attribute information
	for (unsigned int i = 0; i < uint; ++i) {
		uint32_t labelu = 0;
		uint32_t strideu = 0;

		f.read((char*)& labelu, sizeof(uint));
		f.read((char*)& strideu, sizeof(uint));

		//Dump the data buffer
		uint64_t size;
		f.read((char*)& size, sizeof(size));
		if (!size)
			continue;
		auto buffer = mesh.AllocateMemory(size * sizeof(float));

		f.read(buffer.Memory<char>(0), buffer.Size());
		if (f.eof())
			return {};

		//Create the attribute channel
		MeshAttribute attrib;
		attrib.label = labels[labelu];
		attrib.strideOffset = 0;
		switch (strideu) {
		case 1:
			attrib.type = DataType::Float;
			break;
		case 2:
			attrib.type = DataType::Float_2;
			break;
		case 3:
			attrib.type = DataType::Float_3;
			break;
		case 4:
			attrib.type = DataType::Float_4;
			break;
		}
		attrib.stream.bufferView.buffer = buffer;
		attrib.stream.elemByteStride = uint32_t(GetRuntimeTypeTraits(attrib.type).byteSizeVector);
		attrib.stream.bufferView.byteOffset = 0;
		attrib.stream.bufferView.byteSize = buffer.Size();
		mesh.AddAttribute(attrib);
	}

	//Read the int attribute count
	f.read((char*)& uint, sizeof(uint));

	for (unsigned int i = 0; i < uint; ++i) {
		uint32_t labelu = 0;
		uint32_t strideu = 0;

		f.read((char*)& labelu, sizeof(uint));
		f.read((char*)& strideu, sizeof(uint));

		//Dump the data buffer
		uint64_t size;
		f.read((char*)& size, sizeof(size));
		if (!size)
			continue;
		auto buffer = mesh.AllocateMemory(size * sizeof(int));
		f.read(buffer.Memory<char>(0), buffer.Size());
		if (f.eof())
			return {};

		//Create the attribute channel
		MeshAttribute attrib;
		attrib.label = labels[labelu];
		attrib.strideOffset = 0;
		switch (strideu) {
		case 1:
			attrib.type = DataType::Int32;
			break;
		case 2:
			attrib.type = DataType::Int32_2;
			break;
		case 3:
			attrib.type = DataType::Int32_3;
			break;
		case 4:
			attrib.type = DataType::Int32_4;
			break;
		}
		attrib.stream.bufferView.buffer = buffer;
		attrib.stream.elemByteStride = uint32_t(GetRuntimeTypeTraits(attrib.type).byteSizeVector);
		attrib.stream.bufferView.byteOffset = 0;
		attrib.stream.bufferView.byteSize = buffer.Size();
		mesh.AddAttribute(attrib);
	}

	//Read the uint attribute count
	f.read((char*)& uint, sizeof(uint));

	for (unsigned int i = 0; i < uint; ++i) {
		uint32_t labelu = 0;
		uint32_t strideu = 0;

		f.read((char*)& labelu, sizeof(uint));
		f.read((char*)& strideu, sizeof(uint));

		//Dump the data buffer
		uint64_t size;
		f.read((char*)& size, sizeof(size));
		if (!size)
			continue;

		if (labelu == 6) {
			//Face index
			MeshIndex index;
			index.bufferView.buffer = mesh.AllocateMemory(size * sizeof(uint32_t));
			index.count = uint32_t(size);
			index.type = DataType::UInt32;
			f.read(index.bufferView.buffer.Memory<char>(0), index.bufferView.buffer.Size());

			mesh.SetIndex(index);
		}
		else {
			auto buffer = mesh.AllocateMemory(size * sizeof(uint32_t));
			f.read(buffer.Memory<char>(0), buffer.Size());
			if (f.eof())
				return {};

			//Create the attribute channel
			MeshAttribute attrib;
			attrib.label = labels[labelu];
			attrib.strideOffset = 0;
			switch (strideu) {
			case 1:
				attrib.type = DataType::UInt32;
				break;
			case 2:
				attrib.type = DataType::UInt32_2;
				break;
			case 3:
				attrib.type = DataType::UInt32_3;
				break;
			case 4:
				attrib.type = DataType::UInt32_4;
				break;
			}
			attrib.stream.bufferView.buffer = buffer;
			attrib.stream.elemByteStride = uint32_t(GetRuntimeTypeTraits(attrib.type).byteSizeVector);
			attrib.stream.bufferView.byteOffset = 0;
			attrib.stream.bufferView.byteSize = buffer.Size();
			mesh.AddAttribute(attrib);
		}
	}

	//Read the section count
	f.read((char*)& uint, sizeof(uint));

	//Read the information about each section
	for (unsigned int i = 0; i < uint; ++i) {
		MeshSection section;

		uint32_t value32 = 0;
		f.read((char*)& value32, sizeof(uint));
		section.label = value32;
		f.read((char*)& value32, sizeof(uint));
		section.indexStart = value32;
		f.read((char*)& value32, sizeof(uint));
		section.indexCount = value32;
		f.read((char*)& value32, sizeof(uint));
		section.materialChannel = value32;
		if (f.eof())
			return {};

		mesh.AddSection(section);
	}

	if (mesh.GetSections().empty())
		mesh.SingleSection(0, 0);

	//Primitive was not stored under this format, but it's likely triangles
	mesh.SetPrimitive(MeshPrimitive::Triangles);

	return mesh;
}
