#pragma once

#include "weave/graphics/core/ImageLoader.h"
#include "weave/graphics/core/MeshBuilder.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <random>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace weave::tests::graphics::core::detail {

#pragma pack(push, 1)
struct TgaHeader {
	uint8_t identlength{};
	uint8_t colormaptype{};
	uint8_t datatype{};
	uint8_t b1{};
	uint8_t b2{};
	uint8_t b3{};
	uint8_t b4{};
	uint8_t b5{};
	uint16_t originx{};
	uint16_t originy{};
	uint16_t width{};
	uint16_t height{};
	uint8_t bitsperpixel{};
	uint8_t info{};
};
#pragma pack(pop)
static_assert(sizeof(TgaHeader) == 18, "Unexpected TGA header packing");

inline std::vector<uint8_t> BuildUncompressedTga(uint16_t width, uint16_t height, std::span<const std::array<uint8_t, 4>> pixels, bool includeAlpha) {
	const uint32_t pixelCount = width * height;
	const uint32_t componentCount = includeAlpha ? 4 : 3;
	std::vector<uint8_t> blob;
	blob.resize(sizeof(TgaHeader) + pixelCount * componentCount);

	TgaHeader header{};
	header.datatype = 2;
	header.width = width;
	header.height = height;
	header.bitsperpixel = static_cast<uint8_t>(componentCount * 8);
	header.info = 32; // Top-left origin so FlipVertical is skipped.
	std::memcpy(blob.data(), &header, sizeof(header));

	uint8_t* writePtr = blob.data() + sizeof(header);
	for (uint32_t i = 0; i < pixelCount; ++i) {
		const auto& px = pixels[i];
		*writePtr++ = px[2]; // B
		*writePtr++ = px[1]; // G
		*writePtr++ = px[0]; // R
		if (includeAlpha) {
			*writePtr++ = px[3];
		}
	}

	return blob;
}

inline weave::graphics::ImageData LoadImageFromMemory(std::span<const uint8_t> blob) {
	std::string buffer(reinterpret_cast<const char*>(blob.data()), blob.size());
	std::istringstream stream(buffer, std::ios::binary);
	return weave::graphics::ImageLoader::Load(stream);
}

inline weave::graphics::MeshData CreateBasicMeshData() {
	using namespace weave::graphics;
	using weave::types::DataType;

	const std::array<float, 9> positions{
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
	const std::array<float, 6> uvs{
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f
	};
	const std::array<uint16_t, 3> indices{ 0, 1, 2 };

	MeshBuilder builder;
        builder.AddIndex(weave::types::DataType::UInt16, static_cast<uint32_t>(indices.size()), false, indices.data());
	builder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
	builder.AddAttribute(MeshAttribute::Label::UVCoord, DataType::Float_2, sizeof(uvs), uvs.data());
	builder.SingleSection(0, 0);
	return builder.Build(true);
}

inline std::filesystem::path MakeTempFilePath(std::string_view prefix, std::string_view extension) {
	static std::random_device rd;
	static std::mt19937 rng(rd());
	std::uniform_int_distribution<uint64_t> dist;
	const auto unique = dist(rng);
	std::string filename(prefix);
	filename += "_";
	filename += std::to_string(unique);
	filename += extension;
	return std::filesystem::temp_directory_path() / filename;
}

inline std::vector<uint8_t> MakeBottomUpBgrData(uint32_t width, uint32_t height, std::span<const std::array<uint8_t, 3>> topDownPixels) {
	const uint32_t pixelCount = width * height;
	std::vector<uint8_t> data(pixelCount * 3);
	for (uint32_t row = 0; row < height; ++row) {
		const uint32_t sourceRow = height - 1 - row;
		for (uint32_t col = 0; col < width; ++col) {
			const uint32_t dstIndex = (row * width + col) * 3;
			const uint32_t srcIndex = sourceRow * width + col;
			const auto& pixel = topDownPixels[srcIndex];
			data[dstIndex + 0] = pixel[2];
			data[dstIndex + 1] = pixel[1];
			data[dstIndex + 2] = pixel[0];
		}
	}
	return data;
}

} // namespace weave::tests::graphics::core::detail
