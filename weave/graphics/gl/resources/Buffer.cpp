#include "Buffer.h"
#include "ApiHelpers.h"

using namespace weave;
using namespace weave::opengl;

namespace {
//Helper function to return a GL enum from a Usage enum
[[maybe_unused]] GLbitfield GetGLUsage(BufferUsage usage) {
	switch(usage) {
		case BufferUsage::ReadWrite:
			return gl::DYNAMIC_STORAGE_BIT | gl::MAP_READ_BIT | gl::MAP_WRITE_BIT;
		case BufferUsage::Static:
			return 0;
		case BufferUsage::Write:
			return gl::DYNAMIC_STORAGE_BIT;
		default:
			return 0;
	}
}
}

Buffer::Buffer(Buffer &&other) noexcept
	: gl_bufferId(other.gl_bufferId)
	, bufferByteSize(other.bufferByteSize)
	, bufferUsage(other.bufferUsage)
{
	other.gl_bufferId = 0;
	other.bufferByteSize = 0;
}

Buffer& Buffer::operator=(Buffer &&other) noexcept
{
	gl_bufferId = other.gl_bufferId;
	bufferByteSize = other.bufferByteSize;
	bufferUsage = other.bufferUsage;

	other.gl_bufferId = 0;
	other.bufferByteSize = 0;

	return *this;
}

Buffer::~Buffer() {
	gl::DeleteBuffers(1, &gl_bufferId);
}

void Buffer::Create(void const* mem, size_t memSize, BufferUsage usage) {
	gl::DeleteBuffers(1, &gl_bufferId);
	gl::GenBuffers(1, &gl_bufferId);
	gl::BindBuffer(gl::COPY_WRITE_BUFFER, gl_bufferId);
	gl::NamedBufferStorage(gl_bufferId, memSize, mem, GetGLUsage(usage));
	gl::BindBuffer(gl::COPY_WRITE_BUFFER, 0);
	bufferByteSize = memSize;
	bufferUsage = usage;
}

void Buffer::UpdateBuffer(void const *mem, size_t memSize, size_t minBufferIncrement) {
	UpdateBuffer(0, mem, memSize, minBufferIncrement);
}

void Buffer::UpdateBuffer(size_t internalOffset, void const* mem, size_t memSize, size_t minBufferIncrement)
{
	ResizeToFit(memSize, minBufferIncrement);
	gl::NamedBufferSubData(gl_bufferId, GLintptr(internalOffset), memSize, mem);
}

void Buffer::BindToUniforms(GLuint gl_uniformBufferIndex) const {
	if (gl_bufferId == 0) {
		return;
	}
	gl::BindBufferBase(gl::UNIFORM_BUFFER, gl_uniformBufferIndex, gl_bufferId);
}

void Buffer::BindToShader(GLuint gl_shaderBufferIndex) const{
	if(gl_bufferId == 0) {
		return;
	}
	gl::BindBufferBase(gl::SHADER_STORAGE_BUFFER, gl_shaderBufferIndex, gl_bufferId);
}

void Buffer::BindToTexture(GLuint gl_index, GLuint gl_textureId) const {
	if (gl_bufferId == 0) {
		return;
	}
	//Activate the related texture unit according to the binding index and bind the texture
	gl::ActiveTexture(gl::TEXTURE0 + gl_index);
	gl::BindTexture(gl::TEXTURE_BUFFER, gl_textureId);
	gl::TexBuffer(gl::TEXTURE_BUFFER, gl::RGBA32F, gl_bufferId);
}

void Buffer::BindToTarget(GLenum gl_bufferTarget) const {
	if (gl_bufferId == 0) {
		return;
	}

	gl::BindBuffer(gl_bufferTarget, gl_bufferId);
}

void Buffer::ResizeToFit(size_t memSize, size_t minBufferIncrement)
{
	if (memSize > bufferByteSize) {
		size_t expandTo = ((memSize / minBufferIncrement) + 1) * minBufferIncrement;
		Create(nullptr, expandTo, bufferUsage);
	}
}

