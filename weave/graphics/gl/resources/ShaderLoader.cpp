#include "ShaderLoader.h"
#include <fstream>
#include <format>

using namespace weave;
using namespace weave::opengl;

Shader ShaderLoader::BuildSource(GLenum shaderType, char const * code, std::filesystem::path const& includeBasePath) {
	if(!code || code[0] == 0)
		return {};

	auto gl_shaderId = gl::CreateShader(shaderType);
	if (!gl_shaderId) {
		return {};
	}
	

	//Pre-parse the shader code looking for pragma weave entries
	std::string completeCode = PreparseSource(code, includeBasePath);
	char const * sourceCode[1];
	sourceCode[0] = completeCode.c_str();

	//Pass the final code to GL
	gl::ShaderSource(gl_shaderId, 1, (const GLchar **)sourceCode, nullptr);
	gl::CompileShader(gl_shaderId);

	int test = 0;
	gl::GetShaderiv(gl_shaderId, gl::COMPILE_STATUS, &test);
	if (test == 0) {
		return {};
	}

	Shader shader;
	shader.gl_shaderId = gl_shaderId;
	shader.format = gl::SHADER;

	return shader;
}

Shader ShaderLoader::BuildSource(GLenum shaderType, std::filesystem::path const& filepath, std::filesystem::path const& includeBasePath) {
	std::ifstream file(filepath);

	if(file.good()) {
		std::stringstream buffer;
		buffer << file.rdbuf();
		return BuildSource(shaderType, buffer.str().c_str(), includeBasePath);
	}
	
	return {};
}

Shader ShaderLoader::BuildSpirv(GLenum shaderType, std::span<char const> binary, std::string entryPointName)
{
	if (!binary.data() || !binary.size())
		return {};

	auto gl_shaderId = gl::CreateShader(shaderType);
	if (!gl_shaderId) {
		return {};
	}

	//Pass the final code to GL
	gl::ShaderBinary(1, &gl_shaderId, gl::SHADER_BINARY_FORMAT_SPIR_V, binary.data(), (GLsizei)binary.size());
	gl::SpecializeShader(gl_shaderId, entryPointName.c_str(), 0, nullptr, nullptr);

	int test = 0;
	gl::GetShaderiv(gl_shaderId, gl::COMPILE_STATUS, &test);
	if (test == 0) {
		return {};
	}

	Shader shader;
	shader.gl_shaderId = gl_shaderId;
	shader.format = gl::SHADER_BINARY_FORMAT_SPIR_V;

	return shader;
}

Shader ShaderLoader::BuildSpirv(GLenum shaderType, std::filesystem::path const& filepath, std::string entryPointName)
{
	std::ifstream file(filepath, std::ios_base::binary);

	if (file.good()) {
		std::error_code error;
		auto fileSize = std::filesystem::file_size(filepath, error);
		if (!error) {
			std::vector<char> buffer(fileSize);
			file.read(buffer.data(), fileSize);
			return BuildSpirv(shaderType, buffer, entryPointName);
		}
	}

	return {};
}

std::string ShaderLoader::PreparseSource(char const * sourceCode, std::filesystem::path const& includeBasePath) {
	std::stringstream out;
	std::stringstream in(sourceCode);

	//Go through the code and look for #pragma weave lines
	std::string buffer;

	bool loop = true;
	while(loop && !in.eof() && in.good()) {
		if (!std::getline(in, buffer)) {
			break;
		}
		constexpr std::string_view includeWord{ "//#include" };
		if (auto pos = buffer.find(includeWord); pos != buffer.npos) {
			std::string_view includeFile{ &buffer[pos + includeWord.size()] };
			std::erase_if(buffer, [](auto& c) { return c == '"'; });
			auto targetPath = includeBasePath / includeFile;
			std::ifstream file(targetPath);
			if ((file.bad() || !file.is_open())) {
				buffer = std::format("//Weave error including file: {}\n", (char const*)targetPath.u8string().c_str());
			}
			else {
				std::stringstream includeSource;
				includeSource << file.rdbuf();
				file.close();
				buffer = PreparseSource(includeSource.str().c_str(), targetPath.parent_path());
			}
		}
		
		out << buffer << "\n";
	}
	
	return out.str();
}
