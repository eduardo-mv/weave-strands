#include "tests/TestEntryPoints.h"
#include "tests/graphics/core/CoreGraphicsTestUtils.h"

#include "weave/graphics/core/ImageData.h"
#include "weave/graphics/gl/gl_format.h"
#include "weave/graphics/vulkan/vk_format.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace weave::tests::graphics::core {
namespace {

constexpr uint32_t kTestWidth = 2;
constexpr uint32_t kTestHeight = 2;

weave::graphics::ImageData MakeSampleImage() {
	const std::array<std::array<uint8_t, 4>, kTestWidth * kTestHeight> pixels{ {
		{ 10, 20, 30, 255 }, // (0,0)
		{ 40, 50, 60, 255 }, // (1,0)
		{ 70, 80, 90, 255 }, // (0,1)
		{ 100, 110, 120, 255 } // (1,1)
	} };
	auto blob = detail::BuildUncompressedTga(static_cast<uint16_t>(kTestWidth), static_cast<uint16_t>(kTestHeight), pixels, false);
	return detail::LoadImageFromMemory(blob);
}

std::array<uint8_t, 4> PixelAt(uint32_t x, uint32_t y) {
	constexpr std::array<std::array<uint8_t, 4>, kTestWidth * kTestHeight> pixels{ {
		{ 10, 20, 30, 255 },
		{ 40, 50, 60, 255 },
		{ 70, 80, 90, 255 },
		{ 100, 110, 120, 255 }
	} };
	return pixels[y * kTestWidth + x];
}

} // namespace

TestReport TestImageData() {
	TestReport report;
	auto image = MakeSampleImage();

	report.Expect(image.IsValid(), "ImageData produced from helper should be valid");
	report.Expect(image.GetWidth(0) == kTestWidth, "Width mismatch on base mip");
	report.Expect(image.GetHeight(0) == kTestHeight, "Height mismatch on base mip");
	report.Expect(image.GetDepth(0) == 1, "Depth mismatch on base mip");
	report.Expect(image.GetNumLayers() == 1, "Expected single layer image");
	report.Expect(image.GetNumMipmaps() == 1, "Helper creates single mip");
	report.Expect(image.GetComponentCount() == 3, "Component count mismatch");
	report.Expect(image.GetComponentByteSize() == 1, "Component byte size mismatch");

	const uint32_t expectedBytes = kTestWidth * kTestHeight * image.GetComponentCount();
	report.Expect(image.GetFullByteSize() == expectedBytes, "Full byte size mismatch");
	report.Expect(image.GetLayerByteSize() == expectedBytes, "Layer byte size mismatch");
	report.Expect(image.GetByteSize(0) == expectedBytes, "Byte size per mip mismatch");

	auto surface = image.GetSurface(0, 0);
	report.Expect(surface.data.size() == expectedBytes, "Surface byte span mismatch");
	auto invalidLayerSurface = image.GetSurface(1, 0);
	report.Expect(invalidLayerSurface.data.empty(), "Surface query with invalid layer should be empty");
	auto invalidMipSurface = image.GetSurface(0, 1);
	report.Expect(invalidMipSurface.data.empty(), "Surface query with invalid mip should be empty");

	const auto glType = image.GetGLDataType();
	const auto expectedGl = static_cast<uint32_t>(weave::opengl::GlFormat::BGR);
	report.Expect(glType == expectedGl, "Unexpected GL data type");
	const auto vkFormat = image.GetVkFormat();
	const auto expectedVk = static_cast<uint32_t>(weave::vulkan::ImageFormat::B8G8R8_UNORM);
	report.Expect(vkFormat == expectedVk, "Unexpected Vulkan format value");
	report.Expect(image.MaxMipmaps() == 2, "Unexpected max mip count for 2x2 image");

	uint8_t rgba[4]{};
	bool gotPixel = image.GetRGBAPixel(0, 0, 1, 0, 0, rgba);
	report.Expect(gotPixel, "GetRGBAPixel should succeed for valid coords");
	auto expectedPixel = PixelAt(1, 0);
	report.Expect(rgba[0] == expectedPixel[2] && rgba[1] == expectedPixel[1] && rgba[2] == expectedPixel[0],
		"RGB values read from surface mismatch stored data");

	uint8_t fastPixel[4]{};
	image.GetRGBAPixelFast(0, 0, fastPixel);
	auto firstPixel = PixelAt(0, 0);
	report.Expect(fastPixel[0] == firstPixel[2] && fastPixel[1] == firstPixel[1] && fastPixel[2] == firstPixel[0],
		"Fast pixel accessor mismatch");

	auto originalSurface = image.GetSurface(0, 0);
	std::vector<uint8_t> beforeFlip(originalSurface.data.begin(), originalSurface.data.end());
	image.FlipVertical();
	auto flippedSurface = image.GetSurface(0, 0);
	const uint32_t rowSize = kTestWidth * image.GetComponentCount();
	bool rowsSwapped = std::equal(flippedSurface.data.begin(), flippedSurface.data.begin() + rowSize,
		beforeFlip.begin() + rowSize);
	rowsSwapped = rowsSwapped && std::equal(flippedSurface.data.begin() + rowSize, flippedSurface.data.end(),
		beforeFlip.begin());
	report.Expect(rowsSwapped, "FlipVertical did not swap top and bottom rows");

	auto layered = image;
	const bool merged = layered.MergeWith(image);
	report.Expect(merged, "MergeWith should succeed for identical images");
	report.Expect(layered.GetNumLayers() == 2, "Layer count should double after merge");
	layered.ConvertLayersTo3D();
	report.Expect(layered.GetNumLayers() == 1, "Layer count should drop to 1 after converting to 3D");
	report.Expect(layered.GetDepth(0) == 2, "Depth should match previous layer count after converting to 3D");

	return report;
}

} // namespace weave::tests::graphics::core
