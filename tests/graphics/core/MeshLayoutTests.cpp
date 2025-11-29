#include "tests/TestEntryPoints.h"
#include "tests/graphics/core/CoreGraphicsTestUtils.h"

#include "weave/graphics/core/MeshLayout.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <vector>

namespace weave::tests::graphics::core {

TestReport TestMeshLayout() {
	using namespace weave::graphics;

	TestReport report;
	auto referenceMesh = detail::CreateBasicMeshData();
	auto layout = MeshLayout::FromMeshData(referenceMesh);

	report.Expect(layout.buffers.size() == referenceMesh.GetUniqueStreams().size(),
		"Layout buffers should match mesh unique streams");
	report.Expect(layout.indexType == referenceMesh.GetIndex().type, "Layout should capture index type");

	for (auto const& attr : referenceMesh.GetAttributes()) {
		bool found = false;
		for (auto const& buffer : layout.buffers) {
			auto it = std::find_if(buffer.attributes.begin(), buffer.attributes.end(),
				[&](MeshLayout::Buffer::Attribute const& candidate) {
					return candidate.label == attr.label
						&& candidate.type == attr.type
						&& candidate.strideOffset == attr.strideOffset;
				});
			if (it != buffer.attributes.end()) {
				found = true;
				break;
			}
		}
		report.Expect(found, "Every mesh attribute should be represented inside the layout");
	}

	MeshLayout manual;
	manual.AddBuffer(static_cast<uint32_t>(sizeof(float) * 3))
		.AddAttribute(MeshAttribute::Label::Position, weave::types::DataType::Float_3, 0);
	manual.AddBuffer(static_cast<uint32_t>(sizeof(float) * 2))
		.AddAttribute(MeshAttribute::Label::UVCoord, weave::types::DataType::Float_2, 0);
	manual.SetIndexType(weave::types::DataType::UInt16);

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

	std::array<std::byte, sizeof(positions)> posBytes{};
	std::memcpy(posBytes.data(), positions.data(), posBytes.size());
	std::array<std::byte, sizeof(uvs)> uvBytes{};
	std::memcpy(uvBytes.data(), uvs.data(), uvBytes.size());

	std::vector<std::span<const std::byte>> bufferData{
		std::span<const std::byte>(posBytes.data(), posBytes.size()),
		std::span<const std::byte>(uvBytes.data(), uvBytes.size())
	};

	auto builtMesh = manual.Build(bufferData);
	report.Expect(builtMesh.GetAttributes().size() == 2, "Manual layout should generate two attributes");
	report.Expect(builtMesh.HasIndex(), "Manual layout with index type set should create an index buffer");
	report.Expect(builtMesh.GetIndex().count == 0, "Layout build uses empty index placeholder by default");

	auto builtPosition = builtMesh.GetAttribute(MeshAttribute::Label::Position);
	auto builtPositionFloats = builtPosition.stream.bufferView.Memory<const float>();
	report.Expect(std::equal(builtPositionFloats, builtPositionFloats + positions.size(), positions.begin()),
		"Position data built from layout should match supplied bytes");

	auto layoutCopy = layout;
	report.Expect(layout == layoutCopy, "Copied layout should compare equal");
	layoutCopy.buffers[0].stride += 4;
	report.Expect(layout != layoutCopy, "Stride change should make layouts differ");

	return report;
}

} // namespace weave::tests::graphics::core
