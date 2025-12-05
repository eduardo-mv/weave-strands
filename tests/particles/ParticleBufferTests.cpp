#include "tests/TestEntryPoints.h"

#include "weave/particles/core/ParticleBuffer.h"
#include "weave/particles/core/ParticleLayout.h"

#include <cstddef>
#include <cstdint>
#include <tuple>

namespace weave::tests::particles {

TestReport TestParticleBuffer() {
	using namespace weave::particles;

	TestReport report;

	struct TestFieldA { std::byte value; };
	struct TestFieldB { std::byte value; };
	static_assert(sizeof(TestFieldA) == 1);
	static_assert(sizeof(TestFieldB) == 1);

	ParticleLayout layout;
	layout.particleByteSize = sizeof(TestFieldA) + sizeof(TestFieldB);
	layout.SetOffset<TestFieldA>(0);
	layout.SetOffset<TestFieldB>(sizeof(TestFieldA));

	ParticleBuffer buffer(0, layout);

	buffer.AddEmissionParticles(10);
	{
		auto emissionBuffer = buffer.EmissionSpan<TestFieldA, TestFieldB>();
		size_t index = 0;
		for (auto&& [fieldA, fieldB] : emissionBuffer) {
			fieldA.value = std::byte(0);
			fieldB.value = std::byte(0);
			++index;
		}
	}	
	buffer.CommitEmittedParticles();

	report.Expect(buffer.GetBufferByteSize() == 10 * layout.particleByteSize, "Unexpected buffer byte size after initial commit");

	auto editableBuffer = buffer.EditableSpan<TestFieldA, TestFieldB>();

	size_t index = 0;
	for (auto&& [fieldA, fieldB] : editableBuffer) {
		fieldA.value = static_cast<std::byte>(index);
		fieldB.value = static_cast<std::byte>(index + 1);
		++index;
	}

	index = 0;
	for (auto&& [fieldA, fieldB] : editableBuffer) {
		report.Expect(std::to_integer<int>(fieldA.value) == static_cast<int>(index), "FieldA mismatch after write");
		report.Expect(std::to_integer<int>(fieldB.value) == static_cast<int>(index + 1), "FieldB mismatch after write");
		++index;
	}

	buffer.KillParticles(1, 3);

	auto newEditableBuffer = buffer.EditableSpan<TestFieldA, TestFieldB>();
	report.Expect(newEditableBuffer.size() == 7, "Unexpected size after killing particles");

	auto offsets = buffer.GetLayout().GetOffsets<TestFieldA, TestFieldB>();
	auto skippedEditable = buffer.EditableSpan<TestFieldA, TestFieldB>(1, offsets);
	report.Expect(skippedEditable.size() == newEditableBuffer.size() - 1, "Skipped span size mismatch");
	auto firstSkipped = skippedEditable[0];
	auto referenceTuple = newEditableBuffer[1];
	report.Expect(std::to_integer<int>(std::get<0>(firstSkipped).value) ==
		std::to_integer<int>(std::get<0>(referenceTuple).value), "Offset span FieldA mismatch");
	report.Expect(std::to_integer<int>(std::get<1>(firstSkipped).value) ==
		std::to_integer<int>(std::get<1>(referenceTuple).value), "Offset span FieldB mismatch");

	auto emissionView = buffer.AddEmissionParticles<TestFieldA, TestFieldB>(2);
	report.Expect(emissionView.size() == 2, "Emission view size mismatch");
	int emissionIndex = 0;
	for (auto&& [fieldA, fieldB] : emissionView) {
		fieldA.value = static_cast<std::byte>(10 + emissionIndex);
		fieldB.value = static_cast<std::byte>(20 + emissionIndex);
		++emissionIndex;
	}

	auto pendingEmission = buffer.EmissionSpan<TestFieldA, TestFieldB>();
	report.Expect(pendingEmission.size() == 2, "Pending emission span size mismatch");

	{
		auto emissionBuffer = buffer.EmissionSpan<TestFieldA, TestFieldB>();
		size_t index = 0;
		for (auto&& [fieldA, fieldB] : emissionBuffer) {
			fieldA.value = std::byte(0);
			fieldB.value = std::byte(0);
			++index;
		}
	}
	buffer.CommitEmittedParticles();

	auto committedEditable = buffer.EditableSpan<TestFieldA, TestFieldB>();
	report.Expect(committedEditable.size() == 9, "Committed editable span size mismatch");
	
	auto activeSpan = buffer.ActiveByteSpan();
	report.Expect(activeSpan.size() / buffer.GetParticleByteSize() == committedEditable.size(), "Active span size mismatch");
	
	return report;
}

} // namespace weave::tests::particles
