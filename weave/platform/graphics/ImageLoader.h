/*
Weave Image Loader

ImageData loader routines
Support for KTX, DDS, TGA and BMP file formats with propietary implementations.
Support for JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC and PNM file formats via stb image.
	
*/

#pragma once
#include "ImageData.h"
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace weave::graphics {

class ImageLoader {
public:
	ImageLoader() = delete;
	ImageLoader(ImageLoader const& other) = delete;
	ImageLoader(ImageLoader && other) = delete;

	ImageLoader& operator=(ImageLoader const& other) = delete;
	ImageLoader& operator=(ImageLoader && other) = delete;

	~ImageLoader() = delete;

	//Indentifies if this loader is capable of loading a file with the specified magic ID or filepath
	static bool IsCompatibleImage(std::filesystem::path const& filepath, uint32_t magic);

	//Static method to write uncompressed TGA files
	static void WriteTGA(uint32_t width, uint32_t height, uint32_t bits, uint8_t *data, std::filesystem::path const &filepathOut);
	
	//Helper read methods
	static ImageData Load(std::filesystem::path const& filename);
	static ImageData Load(std::istream& file);
	
private:
	//Supported native formats
	static ImageData ReadDDS(std::istream &file);
	static ImageData ReadBMP(std::istream &file);
	static ImageData ReadTGA(std::istream &file);
	static ImageData ReadKTX(std::istream &file);

	//STBI external loader support for other formats
	static ImageData ReadSTBI(char const * const fname);
};

}