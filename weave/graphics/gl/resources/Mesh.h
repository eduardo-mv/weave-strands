/*
OpenGL 4.6 VAO mesh encapsulation
*/

#pragma once
#include "weave/graphics/gl/gl46.h"
#include "weave/graphics/core/MeshData.h"
#include <functional>

namespace weave::opengl {

class MeshUploader;

class Mesh {
	friend MeshUploader;
private:
	GLuint gl_vao = 0;
	GLenum gl_primitive = 0;
	GLenum gl_indexDataType = 0;
	uint32_t indexCount = 0;
	std::vector<GLuint> gl_buffers; //Includes the index buffer, if any
	std::vector<weave::graphics::MeshSection> sections;
	
	//This a lambda required create a VAO for a context
	//It can be called on any context to generate an appropriate VAO
	std::function<GLuint()> vaoBinder;

public:
	Mesh() = default;
	Mesh(Mesh const&) = delete;
	Mesh(Mesh&& other);

	Mesh& operator=(Mesh const&) = delete;
	Mesh& operator=(Mesh&& other);

	~Mesh();

	weave::graphics::MeshSection GetSectionByMaterial(uint32_t matId, uint32_t offset = 0) const;

	void Bind() const;

	//Standard Render interface
	//Renders the mesh in index mode if the index exists or array mode if not
	void Render() const { RenderInstances(1); }
	void Render(uint32_t sectionId) const { RenderInstances(1, sectionId); }
	void Render(uint32_t sectionStart, uint32_t sectionCount) const { RenderInstances(1, sectionStart, sectionCount); }
	//Renders the mesh in index mode if the index exists or array mode if not
	void RenderInstances(uint32_t instanceCount) const;
	void RenderInstances(uint32_t instanceCount, uint32_t sectionId) const;
	void RenderInstances(uint32_t instanceCount, uint32_t sectionStart, uint32_t sectionCount) const;

	//Renders the mesh using the index data. Index data must exist or an assertion failure will trigger
	void RenderIndex(uint32_t instanceCount) const;
	void RenderIndex(uint32_t instanceCount, uint32_t sectionId) const;
	void RenderIndex(uint32_t instanceCount, uint32_t sectionStart, uint32_t sectionCount) const;
	//Renders the mesh using array data (ignoring the index even if it exists)
	void RenderArray(uint32_t instanceCount) const;
	void RenderArray(uint32_t instanceCount, uint32_t sectionId) const;
	void RenderArray(uint32_t instanceCount, uint32_t sectionStart, uint32_t sectionCount) const;
};

}
