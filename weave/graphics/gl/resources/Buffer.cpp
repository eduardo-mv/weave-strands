#include "Buffer.h"
#include "ApiHelpers.h"
#include <cassert>

using namespace weave;
using namespace weave::opengl;

namespace {
//Helper function to return a GL enum from a Usage enum
[[maybe_unused]] GLbitfield GetGLUsage(BufferUsage usage) {
	switch(usage) {
		case BufferUsage::ReadWrite:
			return gl::DYNAMIC_STORAGE_BIT | gl::MAP_READ_BIT | gl::MAP_WRITE_BIT | gl::MAP_PERSISTENT_BIT | gl::MAP_COHERENT_BIT;
		case BufferUsage::Static:
			return 0;
		case BufferUsage::Write:
			return gl::DYNAMIC_STORAGE_BIT | gl::MAP_WRITE_BIT | gl::MAP_PERSISTENT_BIT | gl::MAP_COHERENT_BIT;
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
	gl::CreateBuffers(1, &gl_bufferId);
	gl::NamedBufferStorage(gl_bufferId, memSize, mem, GetGLUsage(usage));
	bufferByteSize = memSize;
	bufferUsage = usage;

	// Documenting for posterity
	//std::byte* mapped = static_cast<std::byte*>(gl::MapNamedBufferRange(gl_bufferId, 0, memSize, GetGLUsage(usage) & ~gl::DYNAMIC_STORAGE_BIT));
}

void Buffer::UpdateBuffer(void const *mem, size_t memSize, size_t minBufferIncrement) {
	UpdateBuffer(0, mem, memSize, minBufferIncrement);
}

void Buffer::UpdateBuffer(size_t internalOffset, void const* mem, size_t memSize, size_t minBufferIncrement)
{
	ResizeToFit(internalOffset + memSize, minBufferIncrement);
	gl::NamedBufferSubData(gl_bufferId, GLintptr(internalOffset), memSize, mem);
}

void Buffer::GrowBuffer(size_t additionalBytes) {
	if (additionalBytes == 0) {
		return;
	}

	const size_t newSize = bufferByteSize + additionalBytes;
	if (gl_bufferId == 0) {
		Create(nullptr, newSize, bufferUsage);
		return;
	}

	GLuint newBuffer = 0;
	gl::CreateBuffers(1, &newBuffer);
	gl::NamedBufferStorage(newBuffer, newSize, nullptr, GetGLUsage(bufferUsage));

	if (bufferByteSize > 0) {
		gl::CopyNamedBufferSubData(gl_bufferId, newBuffer, 0, 0, bufferByteSize);
	}

	gl::DeleteBuffers(1, &gl_bufferId);
	gl_bufferId = newBuffer;
	bufferByteSize = newSize;
}

void Buffer::BindToUniforms(GLuint gl_uniformBufferIndex) const {
	if (gl_bufferId == 0) {
		return;
	}
	gl::BindBufferBase(gl::UNIFORM_BUFFER, gl_uniformBufferIndex, gl_bufferId);
}

void Buffer::BindRangeToUniforms(GLuint gl_uniformBufferIndex, size_t offsetBytes, size_t sizeBytes) const {
	if (gl_bufferId == 0 || sizeBytes == 0) {
		return;
	}

	assert(offsetBytes <= bufferByteSize && "Uniform buffer range offset exceeds buffer size");
	assert(sizeBytes <= (bufferByteSize - offsetBytes) && "Uniform buffer range exceeds buffer size");

	gl::BindBufferRange(
		gl::UNIFORM_BUFFER,
		gl_uniformBufferIndex,
		gl_bufferId,
		static_cast<GLintptr>(offsetBytes),
		static_cast<GLsizeiptr>(sizeBytes));
}

void Buffer::BindToShader(GLuint gl_shaderBufferIndex) const{
	if(gl_bufferId == 0) {
		return;
	}
	gl::BindBufferBase(gl::SHADER_STORAGE_BUFFER, gl_shaderBufferIndex, gl_bufferId);
}

void Buffer::BindRangeToShader(GLuint gl_shaderBufferIndex, size_t offsetBytes, size_t sizeBytes) const {
	if (gl_bufferId == 0 || sizeBytes == 0) {
		return;
	}

	assert(offsetBytes <= bufferByteSize && "Shader buffer range offset exceeds buffer size");
	assert(sizeBytes <= (bufferByteSize - offsetBytes) && "Shader buffer range exceeds buffer size");

	gl::BindBufferRange(
		gl::SHADER_STORAGE_BUFFER,
		gl_shaderBufferIndex,
		gl_bufferId,
		static_cast<GLintptr>(offsetBytes),
		static_cast<GLsizeiptr>(sizeBytes));
}

void Buffer::BindToTexture(GLuint gl_index, GLuint gl_textureId, GLuint gl_internalFormat) const {
	if (gl_bufferId == 0) {
		return;
	}
	//Activate the related texture unit according to the binding index and bind the texture
	gl::ActiveTexture(gl::TEXTURE0 + gl_index);
	gl::BindTexture(gl::TEXTURE_BUFFER, gl_textureId);
	gl::TexBuffer(gl::TEXTURE_BUFFER, gl_internalFormat, gl_bufferId);
}

void Buffer::BindRangeToTexture(GLuint gl_index, GLuint gl_textureId, size_t offsetBytes, size_t sizeBytes, GLuint gl_internalFormat) const {
	if (gl_bufferId == 0 || sizeBytes == 0) {
		return;
	}

	assert(offsetBytes <= bufferByteSize && "Texture buffer range offset exceeds buffer size");
	assert(sizeBytes <= (bufferByteSize - offsetBytes) && "Texture buffer range exceeds buffer size");

	gl::ActiveTexture(gl::TEXTURE0 + gl_index);
	gl::BindTexture(gl::TEXTURE_BUFFER, gl_textureId);
	gl::TexBufferRange(
		gl::TEXTURE_BUFFER,
		gl_internalFormat,
		gl_bufferId,
		static_cast<GLintptr>(offsetBytes),
		static_cast<GLsizeiptr>(sizeBytes));
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
