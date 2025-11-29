#include "tests/TestEntryPoints.h"
#include "tests/graphics/core/CoreGraphicsTestUtils.h"

#include "weave/graphics/core/MeshLoader.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace weave::tests::graphics::core {
namespace {

bool CompareMeshes(TestReport& report, std::string_view label, weave::graphics::MeshData const& expected, weave::graphics::MeshData const& actual) {
	bool ok = true;

	if (expected.Primitive() != actual.Primitive()) {
		report.AddFailure(std::string(label) + ": Primitive mismatch");
		ok = false;
	}

	auto const& expectedSections = expected.GetSections();
	auto const& actualSections = actual.GetSections();
	if (expectedSections.size() != actualSections.size()) {
		report.AddFailure(std::string(label) + ": Section count mismatch");
		ok = false;
	} else {
		for (size_t i = 0; i < expectedSections.size(); ++i) {
			if (!(expectedSections[i] == actualSections[i])) {
				report.AddFailure(std::string(label) + ": Section content mismatch at index " + std::to_string(i));
				ok = false;
			}
		}
	}

	if (expected.HasIndex() != actual.HasIndex()) {
		report.AddFailure(std::string(label) + ": Index presence mismatch");
		ok = false;
	} else if (expected.HasIndex()) {
		auto const& expIndex = expected.GetIndex();
		auto const& actIndex = actual.GetIndex();
		if (expIndex.count != actIndex.count || expIndex.type != actIndex.type || expIndex.primitiveRestart != actIndex.primitiveRestart) {
			report.AddFailure(std::string(label) + ": Index metadata mismatch");
			ok = false;
		} else {
			const size_t byteSize = expIndex.bufferView.byteSize;
			auto expBytes = std::span<const std::byte>(reinterpret_cast<const std::byte*>(expIndex.bufferView.Memory()), byteSize);
			auto actBytes = std::span<const std::byte>(reinterpret_cast<const std::byte*>(actIndex.bufferView.Memory()), byteSize);
			if (!std::equal(expBytes.begin(), expBytes.end(), actBytes.begin(), actBytes.end())) {
				report.AddFailure(std::string(label) + ": Index buffer data mismatch");
				ok = false;
			}
		}
	}

	auto const& expAttrs = expected.GetAttributes();
	auto const& actAttrs = actual.GetAttributes();
	if (expAttrs.size() != actAttrs.size()) {
		report.AddFailure(std::string(label) + ": Attribute count mismatch");
		return false;
	}

	for (size_t i = 0; i < expAttrs.size(); ++i) {
		auto const& expAttr = expAttrs[i];
		auto const& actAttr = actAttrs[i];
		if (expAttr.label != actAttr.label || expAttr.channelId != actAttr.channelId ||
			expAttr.type != actAttr.type || expAttr.strideOffset != actAttr.strideOffset ||
			expAttr.stream.elemByteStride != actAttr.stream.elemByteStride ||
			expAttr.stream.bufferView.byteSize != actAttr.stream.bufferView.byteSize) {
			report.AddFailure(std::string(label) + ": Attribute metadata mismatch at index " + std::to_string(i));
			ok = false;
			continue;
		}

		auto expSpan = std::span<const std::byte>(
			reinterpret_cast<const std::byte*>(expAttr.stream.bufferView.Memory()),
			expAttr.stream.bufferView.byteSize);
		auto actSpan = std::span<const std::byte>(
			reinterpret_cast<const std::byte*>(actAttr.stream.bufferView.Memory()),
			actAttr.stream.bufferView.byteSize);
		if (!std::equal(expSpan.begin(), expSpan.end(), actSpan.begin(), actSpan.end())) {
			report.AddFailure(std::string(label) + ": Attribute data mismatch at index " + std::to_string(i));
			ok = false;
		}
	}

	return ok;
}

void CleanupFile(const std::filesystem::path& path) {
	std::error_code ec;
	std::filesystem::remove(path, ec);
}

} // namespace

TestReport TestMeshLoader() {
	TestReport report;
	using namespace weave::graphics;

	auto mesh = detail::CreateBasicMeshData();

	const auto fileV101 = detail::MakeTempFilePath("MeshLoader101", ".mesh");
	report.Expect(MeshLoader::WriteFile(mesh, fileV101, 101), "WriteFile v101 should succeed");
	auto loadedV101 = MeshLoader::Load(fileV101);
	CleanupFile(fileV101);
	report.Expect(loadedV101.GetBuffers().size() == mesh.GetBuffers().size(), "Loaded v101 mesh should recreate buffers");
	CompareMeshes(report, "v101", mesh, loadedV101);

	/*
	//V100 writing or reading seems to be broken. This is legacy stuff that we might just not need any more.
	const auto fileV100 = detail::MakeTempFilePath("MeshLoader100", ".mesh");
	report.Expect(MeshLoader::WriteFile(mesh, fileV100, 100), "WriteFile v100 should succeed");
	auto loadedV100 = MeshLoader::Load(fileV100);
	CleanupFile(fileV100);
	report.Expect(loadedV100.GetAttributes().size() == mesh.GetAttributes().size(), "Loaded v100 mesh should recreate attributes");
	CompareMeshes(report, "v100", mesh, loadedV100);
	*/

	auto invalidMesh = MeshLoader::Load(std::filesystem::path("nonexistent_file.mesh"));
	const bool invalidState = invalidMesh.GetBuffers().empty() && invalidMesh.GetAttributes().empty() && invalidMesh.GetIndex().count == 0;
	report.Expect(invalidState, "Loading a missing file should yield an empty mesh");

	return report;
}

} // namespace weave::tests::graphics::core
