#include "tests/TestEntryPoints.h"

#include "weave/graphics/core/MeshBuilder.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace weave::tests::graphics::core {

TestReport TestMeshBuilder() {
	using namespace weave::graphics;
	using weave::types::DataType;

	TestReport report;

	const std::array<uint16_t, 3> indices{ 0, 1, 2 };
	const std::array<float, 9> positions{
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
	const std::array<float, 9> normals{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};

	MeshBuilder builder;
	builder.Primitive(MeshPrimitive::Lines);
    builder.AddIndex(DataType::UInt16, static_cast<uint32_t>(indices.size()), false, indices.data());
	builder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
	builder.AddAttribute(MeshAttribute::Label::Normal, DataType::Float_3, sizeof(normals), normals.data());
	builder.SingleSection(5, 2);

	auto singleBufferMesh = builder.Build(false);
	report.Expect(singleBufferMesh.Primitive() == MeshPrimitive::Lines, "Primitive choice should propagate to mesh");
	report.Expect(singleBufferMesh.HasIndex(), "Mesh built with index data should expose an index");
	report.Expect(singleBufferMesh.GetIndex().count == indices.size(), "Index count mismatch in single buffer build");
	report.Expect(singleBufferMesh.GetBuffers().size() == 1, "Single-buffer build should allocate one backing buffer");

	auto indexValues = singleBufferMesh.GetIndex().bufferView.Memory<const uint16_t>();
	report.Expect(std::equal(indexValues, indexValues + indices.size(), indices.begin()), "Index data not copied correctly");

	auto positionAttr = singleBufferMesh.GetAttribute(MeshAttribute::Label::Position);
	auto posValues = positionAttr.stream.bufferView.Memory<const float>();
	report.Expect(std::equal(posValues, posValues + positions.size(), positions.begin()), "Position data not copied correctly");

	auto section = singleBufferMesh.GetSection(0);
	report.Expect(section.label == 5 && section.materialChannel == 2, "Sections should mirror SingleSection configuration");

	auto multiBufferMesh = builder.Build(true);
	report.Expect(multiBufferMesh.GetBuffers().size() == 3, "Multi-buffer build should allocate per-attribute buffers plus index");
	const auto& multiIndex = multiBufferMesh.GetIndex();
	auto multiPosAttr = multiBufferMesh.GetAttribute(MeshAttribute::Label::Position);
	auto multiNormalAttr = multiBufferMesh.GetAttribute(MeshAttribute::Label::Normal);
    const auto indexBufferId = multiBufferMesh.GetBufferId(multiIndex.bufferView.buffer);
    const auto positionBufferId = multiBufferMesh.GetBufferId(multiPosAttr.stream.bufferView.buffer);
    const auto normalBufferId = multiBufferMesh.GetBufferId(multiNormalAttr.stream.bufferView.buffer);
    report.Expect(indexBufferId != positionBufferId && indexBufferId != normalBufferId,
        "Index buffer should be unique when multiBuffer=true");
    report.Expect(positionBufferId != normalBufferId,
        "Distinct attributes should land on separate buffers when multiBuffer=true");

	MeshBuilder noIndexBuilder;
	noIndexBuilder.Primitive(MeshPrimitive::Triangles);
	noIndexBuilder.NoIndex(static_cast<uint32_t>(positions.size() / 3));
	noIndexBuilder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
	noIndexBuilder.AddAttribute(MeshAttribute::Label::Normal, DataType::Float_3, sizeof(normals), normals.data());
	noIndexBuilder.AddSection({ 7u, 3u, 0u, 3u });

	auto noIndexMesh = noIndexBuilder.Build();
	report.Expect(!noIndexMesh.HasIndex(), "NoIndex() builder path should produce indexless mesh");
	report.Expect(noIndexMesh.GetIndex().count == positions.size() / 3, "Indexless mesh should track vertex count");
	report.Expect(noIndexMesh.GetSection(0).label == 7, "Explicit section should be preserved");

	return report;
}

} // namespace weave::tests::graphics::core
