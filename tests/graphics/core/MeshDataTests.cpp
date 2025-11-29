#include "tests/TestEntryPoints.h"

#include "weave/graphics/core/MeshData.h"

#include <array>
#include <cstdint>

namespace weave::tests::graphics::core {

TestReport TestMeshData() {
	using namespace weave::graphics;
	using weave::types::DataType;

	TestReport report;

	MeshData mesh;
	mesh.SetPrimitive(MeshPrimitive::Triangles);

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

	auto posStream = mesh.AllocateStream(sizeof(float) * 3, sizeof(positions), positions.data());
	MeshAttribute positionAttr{ MeshAttribute::Label::Position, ~0u, DataType::Float_3, 0, posStream };
	report.Expect(mesh.AddAttribute(positionAttr), "Position attribute should be added");

	const auto& storedPosition = mesh.GetAttribute(MeshAttribute::Label::Position);
	report.Expect(storedPosition.channelId == static_cast<uint32_t>(MeshAttribute::Label::Position),
		"Auto channel assignment should match label for first attribute");

	auto normalStream = mesh.AllocateStream(sizeof(float) * 3, sizeof(normals), normals.data());
	MeshAttribute normalAttr{ MeshAttribute::Label::Normal, 5u, DataType::Float_3, 0, normalStream };
	report.Expect(mesh.AddAttribute(normalAttr), "Normal attribute should be added");

	report.Expect(!mesh.AddAttribute(normalAttr), "Duplicate channel id should be rejected");

	auto uniqueStreams = mesh.GetUniqueStreams();
	report.Expect(uniqueStreams.size() == 2, "Expected unique streams to match attribute count when buffers differ");

	const std::array<uint16_t, 3> indices{ 0, 1, 2 };
	auto meshIndex = mesh.AllocateIndex(DataType::UInt16, static_cast<uint32_t>(indices.size()), false, indices.data());
	report.Expect(mesh.SetIndex(meshIndex), "SetIndex should accept an index with a known buffer");
	report.Expect(mesh.HasIndex(), "Mesh should report it has an index");
	report.Expect(mesh.GetIndex().count == indices.size(), "Index count mismatch after allocation");

	report.Expect(mesh.AddSection({ ~0u, 7u, 0u, 3u }), "First section should be accepted");
	auto section = mesh.GetSection(0);
	report.Expect(section.label == 1, "Auto-labeled section should increment from zero");

	mesh.AddSection({ 99u, 7u, 1u, 2u });
	report.Expect(mesh.GetSectionByMaterial(7).label == 1, "Material lookup should return first matching section");
	auto materialOffset = mesh.GetSectionByMaterial(7, 1);
	report.Expect(materialOffset.label == 99, "Material lookup with offset should return second section");
	auto labelLookup = mesh.GetSectionByLabel(99);
	report.Expect(labelLookup.indexStart == 1, "Label lookup should return stored index start");

	mesh.SingleSection(42, 5);
	auto singleSection = mesh.GetSection(0);
	report.Expect(singleSection.label == 42 && singleSection.materialChannel == 5, "SingleSection should override labels/material");
	report.Expect(singleSection.indexCount == mesh.GetIndex().count, "SingleSection should use current index count");

	auto bufferId = mesh.GetBufferId(posStream.bufferView.buffer);
	report.Expect(bufferId != ~0u, "Position buffer should be discoverable via GetBufferId");
	report.Expect(mesh.GetBuffer(bufferId).Size() == posStream.bufferView.buffer.Size(), "Buffer lookup should return same size");
	report.Expect(mesh.GetBuffers().size() == 3, "Expected two vertex buffers plus index buffer");

	MeshAttribute updatedNormal = normalAttr;
	updatedNormal.strideOffset = 4;
	report.Expect(mesh.UpdateAttribute(updatedNormal), "UpdateAttribute should succeed for known channel");
	const auto& reloadedNormal = mesh.GetAttribute(MeshAttribute::Label::Normal);
	report.Expect(reloadedNormal.strideOffset == 4, "Updated normal should reflect stride offset change");

	mesh.NoIndex(static_cast<uint32_t>(positions.size() / 3));
	report.Expect(!mesh.HasIndex(), "Mesh should report no index after NoIndex()");
	report.Expect(mesh.GetIndex().count == positions.size() / 3, "Indexless mesh should store vertex count");

	report.Expect(MeshData::GetPrimitiveType("triangle_strip") == MeshPrimitive::TriStrip, "Primitive lookup failed for triangle_strip");
	report.Expect(MeshData::GetPrimitiveType("unknown") == MeshPrimitive::Triangles, "Unknown primitive should fall back to triangles");

	return report;
}

} // namespace weave::tests::graphics::core
