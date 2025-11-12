#include "Shader.h"
#include "Shader.h"
#include "Shader.h"
#include <fstream>
#include <format>
#include <sstream>

using namespace weave;
using namespace weave::opengl;

namespace {
	std::function<void(GLuint, GLint, GLsizei, void const*)> CreateUniformFunc(GLenum type) {
		switch (type) {
		case gl::FLOAT: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform1fv(program, loc, count, (GLfloat const*)data); };
		case gl::FLOAT_VEC2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform2fv(program, loc, count, (GLfloat const*)data); };
		case gl::FLOAT_VEC3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform3fv(program, loc, count, (GLfloat const*)data); };
		case gl::FLOAT_VEC4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform4fv(program, loc, count, (GLfloat const*)data); };
		case gl::DOUBLE: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform1dv(program, loc, count, (GLdouble const*)data); };
		case gl::DOUBLE_VEC2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform2dv(program, loc, count, (GLdouble const*)data); };
		case gl::DOUBLE_VEC3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform3dv(program, loc, count, (GLdouble const*)data); };
		case gl::DOUBLE_VEC4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform4dv(program, loc, count, (GLdouble const*)data); };
		case gl::INT: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform1iv(program, loc, count, (GLint const*)data); };
		case gl::INT_VEC2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform2iv(program, loc, count, (GLint const*)data); };
		case gl::INT_VEC3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform3iv(program, loc, count, (GLint const*)data); };
		case gl::INT_VEC4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform4iv(program, loc, count, (GLint const*)data); };
		case gl::UNSIGNED_INT: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform1uiv(program, loc, count, (GLuint const*)data); };
		case gl::UNSIGNED_INT_VEC2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform2uiv(program, loc, count, (GLuint const*)data); };
		case gl::UNSIGNED_INT_VEC3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform3uiv(program, loc, count, (GLuint const*)data); };
		case gl::UNSIGNED_INT_VEC4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform4uiv(program, loc, count, (GLuint const*)data); };
		case gl::BOOL:      return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform1iv(program, loc, count, (GLint const*)data); };
		case gl::BOOL_VEC2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform2iv(program, loc, count, (GLint const*)data); };
		case gl::BOOL_VEC3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform3iv(program, loc, count, (GLint const*)data); };
		case gl::BOOL_VEC4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform4iv(program, loc, count, (GLint const*)data); };
		case gl::FLOAT_MAT2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix2fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::FLOAT_MAT3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix3fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::FLOAT_MAT4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix4fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::FLOAT_MAT2x3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix2x3fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::FLOAT_MAT2x4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix2x4fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::FLOAT_MAT3x2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix3x2fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::FLOAT_MAT3x4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix3x4fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::FLOAT_MAT4x2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix4x2fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::FLOAT_MAT4x3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix4x3fv(program, loc, count, 0, (GLfloat const*)data); };
		case gl::DOUBLE_MAT2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix2dv(program, loc, count, 0, (GLdouble const*)data); };
		case gl::DOUBLE_MAT3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix3dv(program, loc, count, 0, (GLdouble const*)data); };
		case gl::DOUBLE_MAT4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix4dv(program, loc, count, 0, (GLdouble const*)data); };
		case gl::DOUBLE_MAT2x3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix2x3dv(program, loc, count, 0, (GLdouble const*)data); };
		case gl::DOUBLE_MAT2x4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix2x4dv(program, loc, count, 0, (GLdouble const*)data); };
		case gl::DOUBLE_MAT3x2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix3x2dv(program, loc, count, 0, (GLdouble const*)data); };
		case gl::DOUBLE_MAT3x4: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix3x4dv(program, loc, count, 0, (GLdouble const*)data); };
		case gl::DOUBLE_MAT4x2: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix4x2dv(program, loc, count, 0, (GLdouble const*)data); };
		case gl::DOUBLE_MAT4x3: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniformMatrix4x3dv(program, loc, count, 0, (GLdouble const*)data); };
		default: return [](GLuint program, GLint loc, GLsizei count, void const* data) { gl::ProgramUniform1iv(program, loc, count, (GLint const*)data); };
		}
	}

	GLbitfield GetShaderBitFlag(GLuint shaderId) {
		GLint type = 0;
		gl::GetShaderiv(shaderId, gl::SHADER_TYPE, &type);

		switch (type) {
		case gl::VERTEX_SHADER:
			return gl::VERTEX_SHADER_BIT;
		case gl::GEOMETRY_SHADER:
			return gl::GEOMETRY_SHADER_BIT;
		case gl::FRAGMENT_SHADER:
			return gl::FRAGMENT_SHADER_BIT;
		case gl::TESS_CONTROL_SHADER:
			return gl::TESS_CONTROL_SHADER_BIT;
		case gl::TESS_EVALUATION_SHADER:
			return gl::TESS_EVALUATION_SHADER_BIT;
		case gl::COMPUTE_SHADER:
			return gl::COMPUTE_SHADER_BIT;
		default:
			return 0;
		}
	}
}

Shader::Shader(Shader&& other)
	: gl_shaderId(other.gl_shaderId)
	, format (other.format)
{
	other.gl_shaderId = 0;
	other.format = gl::NONE;
}

Shader& Shader::operator=(Shader&& other)
{
	std::swap(gl_shaderId, other.gl_shaderId);
	std::swap(format, other.format);
	return *this;
}

Shader::~Shader(){
	gl::DeleteShader(gl_shaderId);
}

GLenum weave::opengl::Shader::GetShaderType() const
{
	GLint type = 0;
	gl::GetShaderiv(gl_shaderId, gl::SHADER_TYPE, &type);

	return type;
}

std::pair<GLenum, std::string> Shader::CompilerLog() const {
	std::ostringstream log;

	switch(GetShaderType()) {
		case gl::VERTEX_SHADER:
			log << "Vertex Shader #";
			break;
		case gl::GEOMETRY_SHADER:
			log << "Geometry Shader #";
			break;
		case gl::TESS_CONTROL_SHADER:
			log << "Tess Ctrl Shader #";
			break;
		case gl::TESS_EVALUATION_SHADER:
			log << "Tess Eval Shader #";
			break;
		case gl::FRAGMENT_SHADER:
			log << "Fragment Shader #";
			break;
		case gl::COMPUTE_SHADER:
			log << "Compute Shader #";
			break;
		default:
			log << "Unknown Shader #";
			break;
	}

	log << gl_shaderId;

	int test;
	int length;
	gl::GetShaderiv(gl_shaderId, gl::COMPILE_STATUS, &test);
	log << (test ? ": OK" : ": FAILED");
	gl::GetShaderiv(gl_shaderId, gl::INFO_LOG_LENGTH, &length);
	if(length && gl_shaderId) {
		std::string buff;
		buff.resize(length + 1);
		gl::GetShaderInfoLog(gl_shaderId, length, &length, buff.data());
		buff[length] = 0;
		log << "\n" << buff;
	}
		
	return { test, log.str() };
}

weave::opengl::Program::~Program()
{
	gl::DeleteProgram(gl_programId);
}

void weave::opengl::Program::BindAttribute(GLuint index, char const* name) const
{
	gl::BindAttribLocation(gl_programId, index, name);
}

void weave::opengl::Program::BindUniformBlock(char const* name, unsigned int index) const
{
	gl::UniformBlockBinding(gl_programId, gl::GetUniformBlockIndex(gl_programId, name), index);
}

GLuint weave::opengl::Program::LinkShaderProgram(GLuint separableShader)
{
	return LinkProgram({separableShader}, true);
}

GLuint weave::opengl::Program::LinkProgram(std::vector<GLuint> const& shaders, bool separable)
{
	if (gl_programId)
		gl::DeleteProgram(gl_programId);
	gl_programId = gl::CreateProgram();

	for (auto shaderId : shaders) {
		gl::AttachShader(gl_programId, shaderId);
	}

	if (separable) {
		gl::ProgramParameteri(gl_programId, gl::PROGRAM_SEPARABLE, gl::TRUE_);
	}

	gl::LinkProgram(gl_programId);

	//Check for errors
	int test = 0;
	gl::GetProgramiv(gl_programId, gl::LINK_STATUS, &test);
	if (test == 0) {
		return 0;
	}

	for (auto shaderId : shaders) {
		gl::DetachShader(gl_programId, shaderId);
	}

	//Fetch information of all active uniforms and build the map
	gl::GetProgramInterfaceiv(gl_programId, gl::UNIFORM, gl::ACTIVE_RESOURCES, &numUniforms);
	gl::GetProgramInterfaceiv(gl_programId, gl::UNIFORM_BLOCK, gl::ACTIVE_RESOURCES, &numUniformBlocks);
	gl::GetProgramInterfaceiv(gl_programId, gl::SHADER_STORAGE_BLOCK, gl::ACTIVE_RESOURCES, &numStorageBlocks);
	gl::GetProgramInterfaceiv(gl_programId, gl::PROGRAM_INPUT, gl::ACTIVE_RESOURCES, &numAttributes);

	if (numUniforms > 0) {
		std::string buffer(128, 0);
		for (int i = 0; i < numUniforms; ++i) {
			Uniform uniform;
			GLsizei written = 0;
			gl::GetActiveUniform(gl_programId, i, GLsizei(buffer.size()), &written, &uniform.arrayLength, &uniform.dataType, buffer.data());
			uniform.name = buffer.substr(0, written);
			uniform.location = uniform.name.size() ? gl::GetUniformLocation(gl_programId, uniform.name.c_str()) : uniform.index;
			uniform.index = i;
			uniform.glUniformUpload = CreateUniformFunc(uniform.dataType);

			uniforms[uniform.name] = std::move(uniform);
		}
	}

	for (auto shaderId : shaders) {
		stageFlags |= GetShaderBitFlag(shaderId);
	}
	
	return gl_programId;
}

std::pair<GLenum, std::string> weave::opengl::Program::CompilerLog() const
{
	std::ostringstream log;

	log << "Shader Program #" << gl_programId;

	int test = 0;
	int length = 0;
	gl::GetProgramiv(gl_programId, gl::LINK_STATUS, &test);
	log << (test ? ": OK" : ": FAILED");
	gl::GetProgramiv(gl_programId, gl::INFO_LOG_LENGTH, &length);
	if (length && gl_programId) {
		std::string buff;
		buff.resize(length + 1);
		gl::GetProgramInfoLog(gl_programId, length, &length, buff.data());
		buff[length] = 0;
		log << "\n" << buff;
	}
	else {
		log << "\nNo link performed";
	}

	return { test, log.str() };
}

void weave::opengl::Program::Use()
{
	gl::UseProgram(gl_programId);
}

void weave::opengl::Program::UseStages(GLuint pipeline)
{
	gl::UseProgramStages(pipeline, stageFlags, gl_programId);
}

void weave::opengl::Program::UseStages(GLuint pipeline, GLbitfield stages)
{
	gl::UseProgramStages(pipeline, stages, gl_programId);
}

ProgramPipeline::~ProgramPipeline() {
	gl::DeleteProgramPipelines(1, &gl_programPipelineId);
}

void ProgramPipeline::Create() {
	gl::CreateProgramPipelines(1, &gl_programPipelineId);
}

void ProgramPipeline::Use() {
	gl::BindProgramPipeline(gl_programPipelineId);
}
