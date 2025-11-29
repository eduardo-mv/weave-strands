/*
OpenGL Shader Loader
Creates a Shader object from source code or SPIR-V binaries
*/
#pragma once

#include "weave/graphics/gl/gl46.h"
#include "Shader.h"
#include <filesystem>
#include <span>

namespace weave::opengl {

class ShaderLoader {

	ShaderLoader() = delete;
	ShaderLoader(ShaderLoader const& other) = delete;
	ShaderLoader(ShaderLoader&& other) = delete;

	ShaderLoader& operator=(ShaderLoader const& other) = delete;
	ShaderLoader& operator=(ShaderLoader&& other) = delete;

	~ShaderLoader() = delete;

public:
	static Shader BuildSource(GLenum shaderType, char const* source, std::filesystem::path const& includeBasePath = {});
	static Shader BuildSource(GLenum shaderType, std::filesystem::path const& filepath, std::filesystem::path const& includeBasePath = {});

	static Shader BuildSpirv(GLenum shaderType, std::span<char const> binary, std::string entryPointName = "main");
	static Shader BuildSpirv(GLenum shaderType, std::filesystem::path const& filepath, std::string entryPointName = "main");

private:
	//Goes through the provided source code looking for pragma LZ entries and executing the commands. Returns the modified source code. This method is recursive for appended code.
	static std::string PreparseSource(char const *source, std::filesystem::path const& includeBasePath);

};

}
