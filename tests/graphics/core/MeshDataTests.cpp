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

	const std::array<uint16_t, 3> indices{ 2, 1, 0 };
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

	std::array<uint32_t, 3> expectedVertexOrder{ 2u, 1u, 0u };
	uint32_t logicalVertex = 0;
	for (auto vertex : mesh.IndexedVertices()) {
		report.Expect(vertex.IndexPosition() == logicalVertex, "Index position should match iteration order");
		report.Expect(vertex.VertexIndex() == expectedVertexOrder[logicalVertex], "Indexed vertex should resolve to expected source vertex");

		auto positionSample = vertex.Attribute(MeshAttribute::Label::Position);
		report.Expect(positionSample.IsValid(), "Vertex proxy should expose position attribute");
		auto expectedBase = expectedVertexOrder[logicalVertex] * 3;
		auto position = positionSample.Read<std::array<float, 3>>();
		report.Expect(position[0] == positions[expectedBase + 0], "Vertex X value mismatch");
		report.Expect(position[1] == positions[expectedBase + 1], "Vertex Y value mismatch");
		report.Expect(position[2] == positions[expectedBase + 2], "Vertex Z value mismatch");
		++logicalVertex;
	}

	mesh.NoIndex(static_cast<uint32_t>(positions.size() / 3));
	report.Expect(!mesh.HasIndex(), "Mesh should report no index after NoIndex()");
	report.Expect(mesh.GetIndex().count == positions.size() / 3, "Indexless mesh should store vertex count");

	uint32_t vertexId = 0;
	for (auto vertex : mesh.UniqueVertices()) {
		report.Expect(vertex.VertexIndex() == vertexId, "Indexless vertex index should match iteration order");
		auto normalSample = vertex.Attribute(MeshAttribute::Label::Normal);
		report.Expect(normalSample.IsValid(), "Indexless vertex should expose normal attribute");
		report.Expect(normalSample.AttributeType() == DataType::Float_3, "Normal attribute type mismatch");
		auto values = normalSample.Read<std::array<float, 3>>();
		report.Expect(values[2] == 1.0f, "Indexless vertex normal should match stored data");
		++vertexId;
	}

	uint32_t writeCursor = 0;
	for (auto vertex : mesh.UniqueVertices()) {
		std::array<float, 3> override{
			static_cast<float>(writeCursor + 10),
			static_cast<float>(writeCursor + 11),
			static_cast<float>(writeCursor + 12)
		};
		vertex.WriteAttribute(MeshAttribute::Label::Position, override);
		auto updated = vertex.Attribute(MeshAttribute::Label::Position);
		auto values = updated.Read<std::array<float, 3>>();
		report.Expect(values == override, "Mutable vertex attribute write should persist");
		++writeCursor;
	}

	report.Expect(MeshData::GetPrimitiveType("triangle_strip") == MeshPrimitive::TriStrip, "Primitive lookup failed for triangle_strip");
	report.Expect(MeshData::GetPrimitiveType("unknown") == MeshPrimitive::Triangles, "Unknown primitive should fall back to triangles");

	for(auto elem : normalAttr.Elements()) {
		float f = elem.Read<float>();
		report.Expect(f == *reinterpret_cast<float const*>(elem.Data()), "DynamicTypeConvert must report same value conversions (float)");

		std::array<float,3> f3 = elem.Read<std::array<float,3>>();
		report.Expect(
			f3[0] == reinterpret_cast<float const*>(elem.Data())[0] &&
			f3[1] == reinterpret_cast<float const*>(elem.Data())[1] &&
			f3[2] == reinterpret_cast<float const*>(elem.Data())[2], 
			"DynamicTypeConvert must report same value conversions (float)");
	}

	return report;
}

} // namespace weave::tests::graphics::core
