/*
OpenGL Buffer wrapper

GL based interface for buffer objects.
*/
#pragma once

#include "weave/graphics/gl/gl46.h"
#include <cstddef>

namespace weave::opengl {

enum class BufferUsage {
	ReadWrite, //RW hinted to the API
	Write, //Write only (multiple) hinted to the API
	Static //Write once hinted to the API
};

class Buffer {
	
protected:
	GLuint gl_bufferId = 0;

	size_t bufferByteSize = 0;
	BufferUsage bufferUsage = BufferUsage::Write;
public:
	Buffer() = default;
	Buffer(Buffer const& other) = delete;
	Buffer(Buffer&& other) noexcept;

	Buffer& operator=(Buffer const& other) = delete;
	Buffer& operator=(Buffer&& other) noexcept;

	~Buffer();

	GLuint GLId() const { return gl_bufferId; }

	size_t GetBufferByteSize() const { return bufferByteSize; }

	template<typename T>
	void Create(T&& data, BufferUsage usage = BufferUsage::Write) {
		Create(&data, sizeof(data), usage);
	}
	void Create(void const* mem, size_t memSize, BufferUsage usage = BufferUsage::Write);

	template<typename T>
	void UpdateBuffer(T&& data) {
		UpdateBuffer(&data, sizeof(data));
	}
	template<typename T>
	void UpdateBuffer(size_t internalOffset, T&& data) {
		UpdateBuffer(internalOffset , &data, sizeof(data));
	}

	void UpdateBuffer(void const* mem, size_t memSize, size_t minBufferIncrement = 1);
	void UpdateBuffer(size_t internalOffset, void const* mem, size_t memSize, size_t minBufferIncrement = 1);

	void BindToUniforms(GLuint gl_uniformBufferIndex) const;
	void BindToShader(GLuint gl_shaderBufferIndex) const;
	void BindToTexture(GLuint gl_index, GLuint gl_textureId) const;
	void BindToTarget(GLenum gl_bufferTarget) const;

private:
	void ResizeToFit(size_t memSize, size_t minBufferIncrement);

};

template<typename T>
class BufferData : public Buffer {
public:
	T data{};

	void Create(BufferUsage usage = BufferUsage::Write) {
		Buffer::Create(data, usage);
	}

	void UpdateBuffer() {
		Buffer::UpdateBuffer(data);
	}
};

}
