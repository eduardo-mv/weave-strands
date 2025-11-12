/*
Weave Mesh Loader

MeshData loader routines
*/

#pragma once
#include "MeshData.h"
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace weave::graphics {

class MeshLoader {
public:
	MeshLoader() = delete;
	MeshLoader(MeshLoader const& other) = delete;
	MeshLoader(MeshLoader && other) = delete;

	MeshLoader& operator=(MeshLoader const& other) = delete;
	MeshLoader& operator=(MeshLoader && other) = delete;

	~MeshLoader() = delete;

	//Saves the mesh data to a binary file. The version flag can be used to select a specific binary version (where 100 = 1.00)
	static bool WriteFile(MeshData const& mesh, std::filesystem::path const& filename, uint32_t version = 0);
	
	//Helper read methods
	static MeshData Load(std::filesystem::path const& filename);
	static MeshData Load(std::istream& file, bool skipHeader);
	
private:
	//Supported versions
	static bool WriteFileV101(MeshData const& mesh, std::filesystem::path const& filename);
	static bool WriteFileV100(MeshData const& mesh, std::filesystem::path const& filename);

	static MeshData LoadFileV101(std::istream& file);
	static MeshData LoadFileV100(std::istream &file);
	static MeshData LoadFileV099(std::istream &file);
};

}