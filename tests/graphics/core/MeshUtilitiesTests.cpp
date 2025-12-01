#include "tests/TestEntryPoints.h"

#include "tests/graphics/core/CoreGraphicsTestUtils.h"
#include "weave/graphics/core/MeshBuilder.h"
#include "weave/graphics/core/MeshUtilities.h"

#include <array>
#include <cmath>
#include <string>
#include <cstddef>
#include <vector>

namespace weave::tests::graphics::core {
namespace {

using namespace weave::graphics;
using weave::types::DataType;

std::array<float, 3> ExtractVec3(const MeshAttribute& attr, uint32_t vertexIndex) {
	std::array<float, 3> value{};
	if (attr.type == DataType::Void || attr.stream.elemByteStride == 0) {
		return value;
	}

	auto const* base = attr.stream.bufferView.Memory<const std::byte>();
	auto const* floats = reinterpret_cast<const float*>(
		base + vertexIndex * attr.stream.elemByteStride + attr.strideOffset);

	value[0] = floats[0];
	value[1] = floats[1];
	value[2] = floats[2];
	return value;
}

bool Vec3Equal(std::array<float, 3> lhs, std::array<float, 3> rhs) {
	const float epsilon = 1e-5f;
	for (size_t i = 0; i < 3; ++i) {
		if (std::abs(lhs[i] - rhs[i]) > epsilon) {
			return false;
		}
	}
	return true;
}

} // namespace

TestReport TestMeshUtilities() {
	TestReport report;

	// ConvertMeshToLayout should repack buffers into the requested layout and keep sections + primitive.
	{
		MeshBuilder builder;
		builder.Primitive(MeshPrimitive::Triangles);

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
		const std::array<uint16_t, 3> indices{ 0, 1, 2 };

		builder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
		builder.AddAttribute(MeshAttribute::Label::Normal, DataType::Float_3, sizeof(normals), normals.data());
		builder.AddIndex(DataType::UInt16, static_cast<uint32_t>(indices.size()), false, indices.data());
		builder.SingleSection(5, 2);

		auto source = builder.Build(true); // distinct buffers for each attribute

		MeshLayout targetLayout;
		targetLayout.AddBuffer(static_cast<uint32_t>(sizeof(float) * 6))
			.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, 0)
			.AddAttribute(MeshAttribute::Label::Normal, DataType::Float_3, static_cast<uint32_t>(sizeof(float) * 3));
		targetLayout.SetIndexType(DataType::UInt16);
		targetLayout.primitive = MeshPrimitive::Triangles;

		auto converted = ConvertMeshToLayout(source, targetLayout);
		auto convertedLayout = MeshLayout::FromMeshData(converted);

		report.Expect(convertedLayout == targetLayout, "Converted layout does not match requested target layout");
		report.Expect(converted.Primitive() == source.Primitive(), "Primitive should be preserved");
		report.Expect(converted.GetSections().size() == source.GetSections().size(), "Sections should be preserved");

		for (uint32_t vertex = 0; vertex < 3; ++vertex) {
			auto sourcePosAttr = source.GetAttribute(MeshAttribute::Label::Position);
			auto convertedPosAttr = converted.GetAttribute(MeshAttribute::Label::Position);
			auto sourceNormalAttr = source.GetAttribute(MeshAttribute::Label::Normal);
			auto convertedNormalAttr = converted.GetAttribute(MeshAttribute::Label::Normal);

			auto sourcePos = ExtractVec3(sourcePosAttr, vertex);
			auto convertedPos = ExtractVec3(convertedPosAttr, vertex);
			auto sourceNormal = ExtractVec3(sourceNormalAttr, vertex);
			auto convertedNormal = ExtractVec3(convertedNormalAttr, vertex);

			report.Expect(Vec3Equal(sourcePos, convertedPos), "Position mismatch after layout conversion");
			report.Expect(Vec3Equal(sourceNormal, convertedNormal), "Normal mismatch after layout conversion");
		}
	}

	// ConvertMeshToIndexless should expand indexed geometry and zero-fill out-of-range vertices.
	{
		MeshBuilder builder;
		const std::array<float, 6> positions{
			10.0f, 11.0f, 12.0f,
			20.0f, 21.0f, 22.0f
		};
		const std::array<uint16_t, 4> indices{ 0, 1, 0, 2 };

		builder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
		builder.AddIndex(DataType::UInt16, static_cast<uint32_t>(indices.size()), false, indices.data());
		builder.Primitive(MeshPrimitive::LineStrip);
		builder.SingleSection(7, 1);

		auto indexedMesh = builder.Build(true);
		auto indexlessMesh = ConvertMeshToIndexless(indexedMesh);

		report.Expect(!indexlessMesh.HasIndex(), "Indexless conversion should remove index buffer");
		report.Expect(indexlessMesh.GetIndex().count == indices.size(), "Indexless mesh vertex count mismatch");
		report.Expect(indexlessMesh.Primitive() == indexedMesh.Primitive(), "Primitive should be preserved after indexless conversion");
		report.Expect(indexlessMesh.GetSections().size() == indexedMesh.GetSections().size(), "Sections should be preserved after indexless conversion");

		auto convertedAttr = indexlessMesh.GetAttribute(MeshAttribute::Label::Position);
		std::array<std::array<float, 3>, 4> expected{
			std::array<float, 3>{10.0f, 11.0f, 12.0f},
			std::array<float, 3>{20.0f, 21.0f, 22.0f},
			std::array<float, 3>{10.0f, 11.0f, 12.0f},
			std::array<float, 3>{0.0f, 0.0f, 0.0f}
		};

		for (size_t vertex = 0; vertex < expected.size(); ++vertex) {
			auto value = ExtractVec3(convertedAttr, static_cast<uint32_t>(vertex));
			report.Expect(Vec3Equal(expected[vertex], value),
				"Vertex data mismatch after exploding index at position " + std::to_string(vertex));
		}
	}

	// ConvertMeshToIndexedBloated should create a sequential index for indexless meshes.
	{
		MeshBuilder builder;
		builder.Primitive(MeshPrimitive::Triangles);

		const std::array<float, 9> positions{
			5.0f, 6.0f, 7.0f,
			8.0f, 9.0f, 10.0f,
			11.0f, 12.0f, 13.0f
		};

		builder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
		builder.NoIndex(static_cast<uint32_t>(positions.size() / 3));
		builder.SingleSection(3, 4);

		auto indexlessMesh = builder.Build(true);
		auto indexedMesh = ConvertMeshToIndexedBloated(indexlessMesh);

		report.Expect(indexedMesh.HasIndex(), "Bloated conversion should create an index buffer");
		report.Expect(indexedMesh.GetIndex().count == indexlessMesh.GetUniqueVertexCount(),
			"Bloated conversion should emit one index per vertex");
		report.Expect(indexedMesh.GetSections().size() == indexlessMesh.GetSections().size(),
			"Sections should be preserved after bloated conversion");
		report.Expect(indexedMesh.Primitive() == indexlessMesh.Primitive(),
			"Primitive should be preserved after bloated conversion");
		report.Expect(indexedMesh.GetIndex().type == DataType::UInt16,
			"Bloated conversion should pick the narrowest compatible index type");

		auto sourcePosAttr = indexlessMesh.GetAttribute(MeshAttribute::Label::Position);
		auto convertedPosAttr = indexedMesh.GetAttribute(MeshAttribute::Label::Position);
		for (uint32_t vertex = 0; vertex < indexlessMesh.GetUniqueVertexCount(); ++vertex) {
			auto srcValue = ExtractVec3(sourcePosAttr, vertex);
			auto dstValue = ExtractVec3(convertedPosAttr, vertex);
			report.Expect(Vec3Equal(srcValue, dstValue),
				"Vertex attributes should be copied during bloated conversion at vertex " + std::to_string(vertex));
		}

		std::vector<uint32_t> indices(indexedMesh.GetIndex().count);
		auto const& indexBuffer = indexedMesh.GetIndex();
		if (indexBuffer.type == DataType::UInt16) {
			auto data = indexBuffer.bufferView.Memory<const uint16_t>();
			for (uint32_t i = 0; i < indexBuffer.count; ++i) {
				indices[i] = data[i];
			}
		} else {
			auto data = indexBuffer.bufferView.Memory<const uint32_t>();
			for (uint32_t i = 0; i < indexBuffer.count; ++i) {
				indices[i] = data[i];
			}
		}

		for (uint32_t i = 0; i < indices.size(); ++i) {
			report.Expect(indices[i] == i, "Bloated indices should be sequential starting at zero");
		}
	}

	// ConvertMeshToIndexed should trim duplicate vertices.
	{
		MeshBuilder builder;
		builder.Primitive(MeshPrimitive::Lines);

		const std::array<float, 12> positions{
			0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 1.0f,
			0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 1.0f
		};

		builder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
		builder.NoIndex(static_cast<uint32_t>(positions.size() / 3));
		builder.SingleSection(11, 6);

		auto indexlessMesh = builder.Build(true);
		auto trimmedMesh = ConvertMeshToIndexed(indexlessMesh);

		report.Expect(trimmedMesh.HasIndex(), "Trimmed conversion should create an index buffer");
		report.Expect(trimmedMesh.GetIndex().count == indexlessMesh.GetUniqueVertexCount(),
			"Trimmed conversion should emit one index entry per source vertex");
		report.Expect(trimmedMesh.GetSections().size() == indexlessMesh.GetSections().size(),
			"Sections should be preserved after trimmed conversion");
		report.Expect(trimmedMesh.Primitive() == indexlessMesh.Primitive(),
			"Primitive should be preserved after trimmed conversion");
		report.Expect(trimmedMesh.GetUniqueVertexCount() == 2,
			"Trimmed conversion should remove duplicate vertices");

		auto trimmedAttr = trimmedMesh.GetAttribute(MeshAttribute::Label::Position);
		std::array<std::array<float, 3>, 2> expectedUnique{
			std::array<float, 3>{0.0f, 0.0f, 0.0f},
			std::array<float, 3>{1.0f, 1.0f, 1.0f}
		};
		const uint32_t uniqueExpectedCount = static_cast<uint32_t>(expectedUnique.size());
		for (uint32_t vertex = 0; vertex < uniqueExpectedCount; ++vertex) {
			auto value = ExtractVec3(trimmedAttr, vertex);
			report.Expect(Vec3Equal(expectedUnique[vertex], value),
				"Trimmed conversion should keep canonical vertex data");
		}

		std::vector<uint32_t> indices(trimmedMesh.GetIndex().count);
		auto const& indexBuffer = trimmedMesh.GetIndex();
		if (indexBuffer.type == DataType::UInt16) {
			auto data = indexBuffer.bufferView.Memory<const uint16_t>();
			for (uint32_t i = 0; i < indexBuffer.count; ++i) {
				indices[i] = data[i];
			}
		} else {
			auto data = indexBuffer.bufferView.Memory<const uint32_t>();
			for (uint32_t i = 0; i < indexBuffer.count; ++i) {
				indices[i] = data[i];
			}
		}

		std::array<uint32_t, 4> expectedIndices{ 0, 1, 0, 1 };
		const uint32_t expectedIndexCount = static_cast<uint32_t>(expectedIndices.size());
		for (uint32_t i = 0; i < expectedIndexCount; ++i) {
			report.Expect(indices[i] == expectedIndices[i],
				"Trimmed indices should point to deduplicated vertices");
		}
	}

	// AddMeshes should append indexless meshes and offset subsequent sections.
	{
		MeshBuilder meshABuilder;
		meshABuilder.Primitive(MeshPrimitive::LineStrip);
		const std::array<float, 6> positionsA{
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f
		};
		meshABuilder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positionsA), positionsA.data());
		meshABuilder.NoIndex(2);
		meshABuilder.SingleSection(21, 9);
		auto meshA = meshABuilder.Build(true);

		MeshBuilder meshBBuilder;
		meshBBuilder.Primitive(MeshPrimitive::LineStrip);
		const std::array<float, 9> positionsB{
			2.0f, 0.0f, 0.0f,
			3.0f, 0.0f, 0.0f,
			4.0f, 0.0f, 0.0f
		};
		meshBBuilder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positionsB), positionsB.data());
		meshBBuilder.NoIndex(3);
		meshBBuilder.SingleSection(22, 10);
		auto meshB = meshBBuilder.Build(true);

		auto combined = AddMeshes(meshA, meshB);

		report.Expect(!combined.HasIndex(), "Indexless mesh addition should remain indexless");
		report.Expect(combined.GetUniqueVertexCount() == 5, "Combined mesh should contain all vertices");
		report.Expect(combined.GetSections().size() == 2, "Combined mesh should preserve section count");
		const auto& combinedSections = combined.GetSections();
		report.Expect(combinedSections[0].indexStart == 0 && combinedSections[0].indexCount == 2,
			"First section should remain unchanged");
		report.Expect(combinedSections[1].indexStart == 2 && combinedSections[1].indexCount == 3,
			"Second section should be offset by the first mesh vertex count");

		auto combinedAttr = combined.GetAttribute(MeshAttribute::Label::Position);
		std::array<std::array<float, 3>, 5> expectedPositions{
			std::array<float, 3>{0.0f, 0.0f, 0.0f},
			std::array<float, 3>{1.0f, 0.0f, 0.0f},
			std::array<float, 3>{2.0f, 0.0f, 0.0f},
			std::array<float, 3>{3.0f, 0.0f, 0.0f},
			std::array<float, 3>{4.0f, 0.0f, 0.0f}
		};
		const uint32_t expectedVertexCount = static_cast<uint32_t>(expectedPositions.size());
		for (uint32_t vertex = 0; vertex < expectedVertexCount; ++vertex) {
			auto value = ExtractVec3(combinedAttr, vertex);
			report.Expect(Vec3Equal(expectedPositions[vertex], value),
				"Vertex order should be preserved when appending indexless meshes");
		}
	}

	// AddMeshes should append indexed meshes and offset indices.
	{
		MeshBuilder indexedABuilder;
		indexedABuilder.Primitive(MeshPrimitive::Triangles);
		const std::array<float, 9> positionsA{
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f
		};
		const std::array<uint16_t, 3> indicesA{ 0, 1, 2 };
		indexedABuilder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positionsA), positionsA.data());
		indexedABuilder.AddIndex(DataType::UInt16, static_cast<uint32_t>(indicesA.size()), false, indicesA.data());
		indexedABuilder.SingleSection(31, 2);
		auto meshIndexedA = indexedABuilder.Build(true);

		MeshBuilder indexedBBuilder;
		indexedBBuilder.Primitive(MeshPrimitive::Triangles);
		const std::array<float, 6> positionsB{
			2.0f, 2.0f, 0.0f,
			3.0f, 3.0f, 0.0f
		};
		const std::array<uint16_t, 3> indicesB{ 0, 1, 0 };
		indexedBBuilder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positionsB), positionsB.data());
		indexedBBuilder.AddIndex(DataType::UInt16, static_cast<uint32_t>(indicesB.size()), false, indicesB.data());
		indexedBBuilder.SingleSection(32, 3);
		auto meshIndexedB = indexedBBuilder.Build(true);

		auto combined = AddMeshes(meshIndexedA, meshIndexedB);

		report.Expect(combined.HasIndex(), "Indexed mesh addition should keep indices");
		report.Expect(combined.GetIndex().count == indicesA.size() + indicesB.size(),
			"Combined mesh should concatenate indices");
		report.Expect(combined.GetUniqueVertexCount() == positionsA.size() / 3 + positionsB.size() / 3,
			"Combined mesh should append vertex data");
		report.Expect(combined.GetSections().size() == 2,
			"Combined mesh should include both section ranges");
		report.Expect(combined.GetSections()[1].indexStart == indicesA.size(),
			"Second section should start after the first mesh indices");

		auto combinedAttr = combined.GetAttribute(MeshAttribute::Label::Position);
		std::array<std::array<float, 3>, 5> expectedPositions{
			std::array<float, 3>{0.0f, 0.0f, 0.0f},
			std::array<float, 3>{1.0f, 0.0f, 0.0f},
			std::array<float, 3>{0.0f, 1.0f, 0.0f},
			std::array<float, 3>{2.0f, 2.0f, 0.0f},
			std::array<float, 3>{3.0f, 3.0f, 0.0f}
		};
		const uint32_t expectedIndexedVertices = static_cast<uint32_t>(expectedPositions.size());
		for (uint32_t vertex = 0; vertex < expectedIndexedVertices; ++vertex) {
			auto value = ExtractVec3(combinedAttr, vertex);
			report.Expect(Vec3Equal(expectedPositions[vertex], value),
				"Combined mesh should keep vertex data ordering");
		}

		std::vector<uint32_t> combinedIndices(combined.GetIndex().count);
		auto const& combinedIndex = combined.GetIndex();
		auto const* idx16 = combinedIndex.bufferView.Memory<const uint16_t>();
		if (idx16) {
			for (uint32_t i = 0; i < combinedIndex.count; ++i) {
				combinedIndices[i] = idx16[i];
			}
		} else {
			auto const* idx32 = combinedIndex.bufferView.Memory<const uint32_t>();
			if (idx32) {
				for (uint32_t i = 0; i < combinedIndex.count; ++i) {
					combinedIndices[i] = idx32[i];
				}
			}
		}

		std::array<uint32_t, 6> expectedIndices{
			0, 1, 2,
			3, 4, 3
		};
		for (uint32_t i = 0; i < expectedIndices.size(); ++i) {
			report.Expect(combinedIndices[i] == expectedIndices[i],
				"Combined indices should offset the second mesh");
		}
	}

	// ExtractSubmesh should clip vertex ranges for indexless meshes.
	{
		MeshBuilder builder;
		builder.Primitive(MeshPrimitive::LineStrip);
		const std::array<float, 15> positions{
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			2.0f, 0.0f, 0.0f,
			3.0f, 0.0f, 0.0f,
			4.0f, 0.0f, 0.0f
		};
		builder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
		builder.NoIndex(static_cast<uint32_t>(positions.size() / 3));
		builder.AddSection(MeshSection{ 101, 0, 0, 3 });
		builder.AddSection(MeshSection{ 102, 0, 3, 2 });

		auto mesh = builder.Build(true);
		auto submesh = ExtractSubmesh(mesh, 1, 3);

		report.Expect(!submesh.HasIndex(), "Indexless submesh should remain indexless");
		report.Expect(submesh.GetUniqueVertexCount() == 3, "Submesh should contain the requested vertex range");
		auto const& sections = submesh.GetSections();
		report.Expect(sections.size() == 2, "Submesh should clamp overlapping sections");
		report.Expect(sections[0].indexStart == 0 && sections[0].indexCount == 2, "First section should be trimmed to overlap");
		report.Expect(sections[1].indexStart == 2 && sections[1].indexCount == 1, "Second section should be offset by the cropped vertices");

		auto attr = submesh.GetAttribute(MeshAttribute::Label::Position);
		std::array<std::array<float, 3>, 3> expected{
			std::array<float, 3>{1.0f, 0.0f, 0.0f},
			std::array<float, 3>{2.0f, 0.0f, 0.0f},
			std::array<float, 3>{3.0f, 0.0f, 0.0f}
		};
		for (uint32_t vertex = 0; vertex < expected.size(); ++vertex) {
			auto value = ExtractVec3(attr, vertex);
			report.Expect(Vec3Equal(expected[vertex], value), "Submesh should copy vertex data for the requested range");
		}
	}

	// ExtractSubmesh should filter indices and sections for indexed meshes.
	{
		MeshBuilder builder;
		builder.Primitive(MeshPrimitive::Triangles);
		const std::array<float, 15> positions{
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			2.0f, 0.0f, 0.0f,
			3.0f, 0.0f, 0.0f,
			4.0f, 0.0f, 0.0f
		};
		const std::array<uint16_t, 6> indices{ 0, 1, 2, 2, 3, 4 };

		builder.AddAttribute(MeshAttribute::Label::Position, DataType::Float_3, sizeof(positions), positions.data());
		builder.AddIndex(DataType::UInt16, static_cast<uint32_t>(indices.size()), false, indices.data());
		builder.AddSection(MeshSection{ 201, 0, 0, 3 });
		builder.AddSection(MeshSection{ 202, 0, 3, 3 });

		auto mesh = builder.Build(true);
		auto submesh = ExtractSubmesh(mesh, 1, 3);

		report.Expect(submesh.HasIndex(), "Indexed submesh should retain indexing");
		report.Expect(submesh.GetUniqueVertexCount() == 3, "Indexed submesh should only keep requested vertices");
		report.Expect(submesh.GetIndex().count == 4, "Indexed submesh should only keep indices referencing the range");
		auto const& sections = submesh.GetSections();
		report.Expect(sections.size() == 2, "Indexed submesh should clamp sections");
		report.Expect(sections[0].indexStart == 0 && sections[0].indexCount == 2, "First indexed section should shrink to overlap");
		report.Expect(sections[1].indexStart == 2 && sections[1].indexCount == 2, "Second indexed section should offset after filtering");

		auto attr = submesh.GetAttribute(MeshAttribute::Label::Position);
		std::array<std::array<float, 3>, 3> expectedVertices{
			std::array<float, 3>{1.0f, 0.0f, 0.0f},
			std::array<float, 3>{2.0f, 0.0f, 0.0f},
			std::array<float, 3>{3.0f, 0.0f, 0.0f}
		};
		for (uint32_t vertex = 0; vertex < expectedVertices.size(); ++vertex) {
			auto value = ExtractVec3(attr, vertex);
			report.Expect(Vec3Equal(expectedVertices[vertex], value), "Indexed submesh should copy vertex attributes");
		}

		std::array<uint32_t, 4> expectedIndices{ 0, 1, 1, 2 };
		std::vector<uint32_t> actual(submesh.GetIndex().count);
		auto const& idx = submesh.GetIndex();
		auto const* idx16 = idx.bufferView.Memory<const uint16_t>();
		if (idx16) {
			for (uint32_t i = 0; i < idx.count; ++i) {
				actual[i] = idx16[i];
			}
		} else {
			auto const* idx32 = idx.bufferView.Memory<const uint32_t>();
			for (uint32_t i = 0; i < idx.count; ++i) {
				actual[i] = idx32[i];
			}
		}
		for (uint32_t i = 0; i < expectedIndices.size(); ++i) {
			report.Expect(actual[i] == expectedIndices[i], "Indexed submesh should remap indices relative to the new vertex range");
		}
	}

	return report;
}

} // namespace weave::tests::graphics::core
