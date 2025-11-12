#include "Mesh.h"
#include <cassert>

using namespace weave;
using namespace opengl;
using namespace graphics;


namespace {
	//Helper conversion for gl draw commands
	void* GlIndexStart(MeshSection const& section, GLenum dataType) {
		return reinterpret_cast<void*>(section.indexStart * (dataType == gl::UNSIGNED_SHORT ? sizeof(uint16_t) : sizeof(uint32_t)));
	}
}

weave::opengl::Mesh::Mesh(Mesh&& other)
	: gl_vao(other.gl_vao)
	, gl_primitive(other.gl_primitive)
	, gl_indexDataType(other.gl_indexDataType)
	, indexCount(other.indexCount)
	, gl_buffers(std::move(other.gl_buffers))
	, sections(std::move(other.sections))
	, vaoBinder(std::move(other.vaoBinder))
{
	other.gl_vao = 0;
}

weave::opengl::Mesh& weave::opengl::Mesh::operator=(Mesh&& other)
{
	gl::DeleteBuffers(GLsizei(gl_buffers.size()), gl_buffers.data());
	gl::DeleteVertexArrays(1, &gl_vao);

	gl_vao = other.gl_vao;
	gl_primitive = other.gl_primitive;
	gl_indexDataType = other.gl_indexDataType;
	indexCount = other.indexCount;
	gl_buffers = std::move(other.gl_buffers);
	sections = std::move(other.sections);
	vaoBinder = std::move(other.vaoBinder);
	other.gl_vao = 0;

	return *this;
}

weave::opengl::Mesh::~Mesh()
{
	gl::DeleteBuffers(GLsizei(gl_buffers.size()), gl_buffers.data());
	gl::DeleteVertexArrays(1, &gl_vao);
}

MeshSection weave::opengl::Mesh::GetSectionByMaterial(uint32_t matId, uint32_t offset) const
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

//Renders the mesh in index mode if the index exists or array mode if not
void weave::opengl::Mesh::RenderInstances(uint32_t instanceCount) const
{
	//Render all sections or the whole array
	gl_indexDataType ?
		RenderIndex(instanceCount) : RenderArray(instanceCount);
}

void weave::opengl::Mesh::RenderInstances(uint32_t instanceCount, uint32_t sectionId) const 
{
	gl_indexDataType ?
		RenderIndex(instanceCount, sectionId) : RenderArray(instanceCount, sectionId);
}

void weave::opengl::Mesh::RenderInstances(uint32_t instanceCount, uint32_t sectionStart, uint32_t sectionCount) const 
{
	gl_indexDataType ?
		RenderIndex(instanceCount, sectionStart, sectionCount) : RenderArray(instanceCount, sectionStart, sectionCount);
}


//Renders the mesh using the index data. Index data must exist or an assertion failure will trigger
void weave::opengl::Mesh::RenderIndex(uint32_t instanceCount) const
{
	Bind();

	for (auto const& section : sections) {
		gl::DrawElementsInstanced(gl_primitive, GLsizei(section.indexCount), gl_indexDataType, GlIndexStart(section, gl_indexDataType), GLsizei(instanceCount));
	}
}

void weave::opengl::Mesh::RenderIndex(uint32_t instanceCount, uint32_t sectionId) const
{
	assert(sectionId < sections.size());
	auto& section = sections[sectionId];
	Bind();
	gl::DrawElementsInstanced(gl_primitive, GLsizei(section.indexCount), gl_indexDataType, GlIndexStart(section, gl_indexDataType), GLsizei(instanceCount));
}

void weave::opengl::Mesh::RenderIndex(uint32_t instanceCount, uint32_t sectionStart, uint32_t sectionCount) const 
{
	Bind();
	gl::DrawElementsInstanced(gl_primitive, GLsizei(std::min(sectionCount, indexCount - sectionStart)), gl::UNSIGNED_INT, reinterpret_cast<void*>(std::min(sectionStart, indexCount) * sizeof(uint32_t)), GLsizei(instanceCount));
}

void weave::opengl::Mesh::RenderArray(uint32_t instanceCount) const 
{
	Bind();

	for (auto const& section : sections) {
		gl::DrawArraysInstanced(gl_primitive, GLint(section.indexStart), GLsizei(section.indexCount), GLsizei(instanceCount));
	}
}

void weave::opengl::Mesh::RenderArray(uint32_t instanceCount, uint32_t sectionId) const 
{
	assert(sectionId < sections.size());
	auto& section = sections[sectionId];

	Bind();
	gl::DrawArraysInstanced(gl_primitive, GLint(section.indexStart), GLsizei(section.indexCount), GLsizei(instanceCount));
}

void weave::opengl::Mesh::RenderArray(uint32_t instanceCount, uint32_t sectionStart, uint32_t sectionCount) const 
{
	Bind();
	gl::DrawArraysInstanced(gl_primitive, GLint(sectionStart), GLint(sectionCount), GLsizei(instanceCount));
}

void weave::opengl::Mesh::Bind() const 
{
	if (!gl_vao) {
		const_cast<Mesh*>(this)->gl_vao = vaoBinder();
	}

	gl::BindVertexArray(gl_vao);
}


