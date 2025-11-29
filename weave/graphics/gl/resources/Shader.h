/*
OpenGL Shader
Shader and ShaderProgram wrappers for GL
*/
#pragma once

#include "weave/graphics/gl/gl46.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace weave::opengl {

class ShaderLoader;

class Shader {
	friend ShaderLoader;

private:
	GLuint gl_shaderId = 0;
	GLenum format = gl::NONE;

public:
	Shader() = default;
	Shader(Shader const&) = delete;
	Shader(Shader &&other);
	Shader& operator=(Shader const&) = delete;
	Shader& operator=(Shader&& other);
	~Shader();

	GLuint GLId() const { return gl_shaderId; }
	GLenum GetShaderType() const;

	//Returns a verbose string from the compiler with the status of the last operations
	std::pair<GLenum, std::string> CompilerLog() const;

};

class Program {
private:
	GLuint gl_programId = 0;
	GLint numUniforms = 0;
	GLint numUniformBlocks = 0;
	GLint numStorageBlocks = 0;
	GLint numAttributes = 0;
	GLbitfield stageFlags = 0;

	struct Uniform {
		GLint location{};
		GLint index{};
		GLint arrayLength{};
		GLenum dataType{};
		std::function<void(GLuint, GLint, GLsizei, void const*)> glUniformUpload;
		std::string name;

		void Upload(GLuint program, GLsizei count, void const *data) const {
			glUniformUpload(program, location, count, data);
		}
	};
	
	std::unordered_map<std::string, Uniform> uniforms;

public:
	~Program();

	GLuint GLId() const { return gl_programId; }

	//Calls gl::BindAttribLocation on "id" and "name"
	void BindAttribute(GLuint index, char const* name) const;
	//Calls gl::GetUniformBlockIndex on "name" and gl::UniformBlockBinding to bind the name to the index
	void BindUniformBlock(char const* name, unsigned int index) const;
	
	template<typename T>
	void UploadUniformValue(std::string const& uniformName, T&& value) {
		if (auto it = uniforms.find(uniformName);  it != uniforms.end()) {
			if constexpr (std::is_array<T>::value || std::is_pointer<T>::value) {
				it->second.Upload(gl_programId, 1, value);
			}
			else {
				it->second.Upload(gl_programId, 1, &value);
			}
		}
	}

	//Links the program as a separable shader program (only one shader). Requires a ProgramPipeline to function.
	GLuint LinkShaderProgram(GLuint separableShader);

	//Links the program. Returns the program object given by GL or 0 if any error ocurred. The build log can be inspected for further information.
	GLuint LinkProgram(std::vector<GLuint> const& shaders, bool separable = false);

	//Returns a verbose string from the compiler with the status of the last operations
	std::pair<GLenum, std::string> CompilerLog() const;

	void Use();
	void UseStages(GLuint pipeline);
	void UseStages(GLuint pipeline, GLbitfield stages);

};

class ProgramPipeline {
private:
	GLuint gl_programPipelineId = 0;
public:
	~ProgramPipeline();

	GLuint GLId() const { return gl_programPipelineId; }

	void Create();

	void Use();
};

}
