#include "MeshUploader.h"
#include <cassert>
#include <unordered_map>

using namespace weave;
using namespace opengl;
using namespace graphics;

GLenum weave::opengl::MeshUploader::GetGLPrimitive(MeshPrimitive prim)
{
	//Static table with the GL enums indexed by converted MeshPrimitive enums
	static GLenum glEnums[] = {gl::POINTS, gl::LINES, gl::LINE_STRIP, gl::TRIANGLES, gl::TRIANGLE_STRIP};
	return glEnums[static_cast<uint32_t>(prim)];
}

MeshPrimitive weave::opengl::MeshUploader::GetPrimitiveType(std::string const& name)
{
	static std::unordered_map<std::string, MeshPrimitive> map = {
		{"triangles", MeshPrimitive::Triangles},
		{"triangle_strip", MeshPrimitive::TriStrip},
		{"trianglestrip", MeshPrimitive::TriStrip},
		{"points", MeshPrimitive::Points},
		{"line_strip", MeshPrimitive::LineStrip},
		{"linestrip", MeshPrimitive::LineStrip},
		{"lines", MeshPrimitive::Lines}
	};

	auto it = map.find(name);
	if (it != map.end())
		return it->second;
	else
		return MeshPrimitive::Triangles;
}

weave::opengl::Mesh weave::opengl::MeshUploader::BuildMesh(MeshData const& mesh)
{
	auto glmesh = BuildMeshBuffers(mesh);
	glmesh.vaoBinder = BuildVaoBinder(mesh, glmesh);

	return glmesh;
}

weave::opengl::Mesh weave::opengl::MeshUploader::BuildMeshBuffers(MeshData const& mesh)
{
	weave::opengl::Mesh glmesh;

	//Loop through the buffers and create the GL VBOs
	for (auto const& buffer : mesh.GetBuffers()) {
		GLuint gl_buffer = 0;
		gl::GenBuffers(1, &gl_buffer);
		gl::BindBuffer(gl::ARRAY_BUFFER, gl_buffer);
		gl::BufferData(gl::ARRAY_BUFFER, GLsizeiptr(buffer.Size()), buffer.Memory(0), gl::STATIC_DRAW);
		assert(gl_buffer); //GL is probably not initialized or this thread has no context access

		glmesh.gl_buffers.emplace_back(gl_buffer);
	}

	glmesh.sections = mesh.GetSections();
	glmesh.gl_primitive = GetGLPrimitive(mesh.Primitive());
	glmesh.indexCount = mesh.GetIndex().count;
	if (mesh.GetIndex().type != types::DataType::UserExtended) {
		glmesh.gl_indexDataType = mesh.GetIndex().type == types::DataType::UInt16 ? gl::UNSIGNED_SHORT : gl::UNSIGNED_INT;
	}
	return glmesh;
}

std::function<GLuint()> weave::opengl::MeshUploader::BuildVaoBinder(MeshData const& mesh, weave::opengl::Mesh const& glmesh)
{
	if (glmesh.gl_buffers.size() != mesh.GetBuffers().size())
		return []() { return 0; };
	
	//Helper lambda to find the related gl_buffer for a buffer
	auto FindGlBuffer = [&mesh, &glmesh](MeshBufferView const& meshBuffer) -> GLuint {
		size_t i = 0;
		for (auto& buffer : mesh.GetBuffers()) {
			if (buffer == meshBuffer.buffer) {
				return glmesh.gl_buffers[i];
			}
			++i;
		}

		return 0;
	};

	std::vector<std::function<void()>> vaoBindings;

	//Loop through all the attributes to bind them with their buffers
	for (auto const& att : mesh.GetAttributes()) {
		auto delayedBinding = [att, bufferId = FindGlBuffer(att.stream.bufferView)]() {
			//Bind the associated buffer to be used for the attribute channel
			gl::BindBuffer(gl::ARRAY_BUFFER, bufferId);
			//Set the vertex property channel
			auto type = types::GetRuntimeTypeTraits(att.type).underlyingDataTypeValue;
			GLint elementSize = GLint(types::GetRuntimeTypeTraits(att.type).vectorSize);

			if (type == types::DataType::Float)
				gl::VertexAttribPointer(GLuint(att.channelId), elementSize, gl::FLOAT, gl::FALSE_, GLsizei(att.stream.elemByteStride), reinterpret_cast<void*>(static_cast<size_t>(att.strideOffset)));
			else if (type == types::DataType::Double)
				gl::VertexAttribPointer(GLuint(att.channelId), elementSize, gl::DOUBLE, gl::FALSE_, GLsizei(att.stream.elemByteStride), reinterpret_cast<void*>(static_cast<size_t>(att.strideOffset)));
			else if (type == types::DataType::Int32)
				gl::VertexAttribIPointer(GLuint(att.channelId), elementSize, gl::INT, GLsizei(att.stream.elemByteStride), reinterpret_cast<void*>(static_cast<size_t>(att.strideOffset)));
			else if (type == types::DataType::UInt32)
				gl::VertexAttribIPointer(GLuint(att.channelId), elementSize, gl::UNSIGNED_INT, GLsizei(att.stream.elemByteStride), reinterpret_cast<void*>(static_cast<size_t>(att.strideOffset)));
			else if (type == types::DataType::Int8)
				gl::VertexAttribPointer(GLuint(att.channelId), elementSize, gl::BYTE, gl::TRUE_, GLsizei(att.stream.elemByteStride), reinterpret_cast<void*>(static_cast<size_t>(att.strideOffset)));
			else if (type == types::DataType::UInt8)
				gl::VertexAttribPointer(GLuint(att.channelId), elementSize, gl::UNSIGNED_BYTE, gl::TRUE_, GLsizei(att.stream.elemByteStride), reinterpret_cast<void*>(static_cast<size_t>(att.strideOffset)));
			else
				assert(false); //Unsuported data type
			//And enable it
			gl::EnableVertexAttribArray(GLuint(att.channelId));
		};

		vaoBindings.emplace_back(std::move(delayedBinding));
	}

	//Bind the index buffer to the ELEMENT_ARRAY_BUFFER slot -> this binding point is specific to the current VAO and is stored as VAO state
	if (!mesh.GetIndex().IsIndexless()) {
		auto gl_buffer = FindGlBuffer(mesh.GetIndex().bufferView);
		auto delayedBinding = [gl_buffer]() {
			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, gl_buffer);
		};
		
		vaoBindings.emplace_back(std::move(delayedBinding));
	}

	auto vaoBinder = [vaoBindings = std::move(vaoBindings)]() {
		GLuint gl_vao = 0;
		gl::GenVertexArrays(1, &gl_vao);
		gl::BindVertexArray(gl_vao);

		if (gl_vao) {
			for (auto& binder : vaoBindings) {
				binder();
			}
		}

		return gl_vao;
	};

	return vaoBinder;
}


