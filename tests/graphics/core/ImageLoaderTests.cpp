#include "tests/TestEntryPoints.h"
#include "tests/graphics/core/CoreGraphicsTestUtils.h"

#include "weave/graphics/core/ImageLoader.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <sstream>
#include <vector>

namespace weave::tests::graphics::core {
namespace {

constexpr uint32_t MakeFourCC(char a, char b, char c, char d) {
	return static_cast<uint32_t>(static_cast<uint8_t>(a))
		| (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 8)
		| (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 16)
		| (static_cast<uint32_t>(static_cast<uint8_t>(d)) << 24);
}

} // namespace

TestReport TestImageLoader() {
	TestReport report;
	using weave::graphics::ImageLoader;

	constexpr uint32_t ddsMagic = MakeFourCC('D', 'D', 'S', ' ');
	constexpr uint32_t tgaMagicGuess = 0x00020000; // datatype 2 + trailing zero byte
	constexpr uint32_t invalidMagic = 0xDEADBEEF;

	report.Expect(ImageLoader::IsCompatibleImage({}, ddsMagic), "DDS magic should be reported as compatible");
	report.Expect(ImageLoader::IsCompatibleImage({}, tgaMagicGuess), "TGA guess should report as compatible");
	report.Expect(!ImageLoader::IsCompatibleImage({}, invalidMagic), "Invalid magic should not be reported as compatible");

	std::string badData("not_an_image");
	std::istringstream badStream(badData, std::ios::binary);
	auto badImage = ImageLoader::Load(badStream);
	report.Expect(!badImage.IsValid(), "Invalid stream should yield invalid ImageData");

	constexpr uint32_t width = 2;
	constexpr uint32_t height = 2;
	const std::array<std::array<uint8_t, 3>, width * height> topDownPixels{ {
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 },
		{ 10, 11, 12 }
	} };

	std::filesystem::path tempFile = detail::MakeTempFilePath("ImageLoader", ".tga");
	auto rawData = detail::MakeBottomUpBgrData(width, height, topDownPixels);
	ImageLoader::WriteTGA(width, height, 24, rawData.data(), tempFile);

	auto loadedFromFile = ImageLoader::Load(tempFile);
	std::error_code removeEc;
	std::filesystem::remove(tempFile, removeEc);

	report.Expect(loadedFromFile.IsValid(), "ImageLoader::Load should read back file written via WriteTGA");
	report.Expect(loadedFromFile.GetWidth(0) == width, "Width mismatch after roundtrip");
	report.Expect(loadedFromFile.GetHeight(0) == height, "Height mismatch after roundtrip");

	auto surface = loadedFromFile.GetSurface(0, 0);
	std::vector<uint8_t> expectedTopDown(surface.data.size());
	for (uint32_t i = 0; i < width * height; ++i) {
		const auto& rgb = topDownPixels[i];
		expectedTopDown[i * 3 + 0] = rgb[2];
		expectedTopDown[i * 3 + 1] = rgb[1];
		expectedTopDown[i * 3 + 2] = rgb[0];
	}

	const bool surfaceMatches = std::equal(surface.data.begin(), surface.data.end(), expectedTopDown.begin());
	report.Expect(surfaceMatches, "Loaded pixel data mismatch after WriteTGA roundtrip");

	return report;
}

} // namespace weave::tests::graphics::core
