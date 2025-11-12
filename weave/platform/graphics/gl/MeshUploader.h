/*
OpenGL 4.6 VAO mesh loader from MeshData
*/
#pragma once
#include "gl46.h"
#include "Mesh.h"

namespace weave::opengl {

class MeshUploader {
private:
	MeshUploader() = delete;
	MeshUploader(MeshUploader const&) = delete;
	MeshUploader(MeshUploader&& other) = delete;

	MeshUploader& operator=(MeshUploader const&) = delete;
	MeshUploader& operator=(MeshUploader&& other) = delete;

	~MeshUploader() = delete;

public:
	//Returns a the related GL enum for a primitive
	static GLenum GetGLPrimitive(weave::graphics::MeshPrimitive prim);

	//Returns a mesh primitive given a name
	static weave::graphics::MeshPrimitive GetPrimitiveType(std::string const& name);

	//Uploads buffer data and creates a GL VAO for the mesh. 
	//This can only be called from the rendering thread because VAOs can not be shared between GL contexts
	//BuildMeshBuffers and BuildMeshVao are provided to split the work load
	static Mesh BuildMesh(weave::graphics::MeshData const& mesh);

private:
	//Builds a partial GL Mesh (without the VAO) which uploads the internal mesh data buffers to GL. This can be called from any thread as long as there's free GLContexts available
	static Mesh BuildMeshBuffers(weave::graphics::MeshData const& mesh);
	//Creates a lambda that can be called from the rendering thread to create the appropriate VAO
	static std::function<GLuint()> BuildVaoBinder(weave::graphics::MeshData const& mesh, weave::opengl::Mesh const& glmesh);

};
}
