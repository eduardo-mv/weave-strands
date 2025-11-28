#include "tests/TestEntryPoints.h"

#include "weave/particles/ParticleBuffer.h"
#include "weave/particles/ParticleLayout.h"
#include "weave/particles/ParticleMachine.h"
#include "weave/particles/nodes/CommitEmissionNode.h"
#include "weave/particles/nodes/ParticleEmitter.h"
#include "weave/particles/nodes/ParticleNode.h"

#include <cmath>
#include <cstddef>

namespace weave::tests::particles {
namespace {

namespace wp = ::weave::particles;

struct ParticleTestWriter : wp::ParticleNode<> {
	ParticleTestWriter(float lifeValue, float maxLifeValue)
		: lifeValue(lifeValue), maxLifeValue(maxLifeValue) {}

	void ExecuteNode() override {
		wroteCount = 0;
		auto& context = GetContext();
		for (auto* buffer : context.buffers) {
			if (!buffer) {
				continue;
			}
			for (auto&& [life, maxLife] : buffer->EmissionSpan<wp::layout::LifeTime, wp::layout::MaxLifeTime>()) {
				life.lifeTime = lifeValue;
				maxLife.maxLifeTime = maxLifeValue;
				++wroteCount;
			}
		}
	}

	float lifeValue{};
	float maxLifeValue{};
	size_t wroteCount{};
};

struct ParticleTestInspector : wp::ParticleNode<> {
	ParticleTestInspector(float lifeValue, float maxLifeValue)
		: lifeValue(lifeValue), maxLifeValue(maxLifeValue) {}

	void ExecuteNode() override {
		verifiedCount = 0;
		mismatchCount = 0;
		auto& context = GetContext();
		for (auto* buffer : context.buffers) {
			if (!buffer) {
				continue;
			}
			for (auto&& [life, maxLife] : buffer->EditableSpan<wp::layout::LifeTime, wp::layout::MaxLifeTime>()) {
				const bool matches = std::abs(life.lifeTime - lifeValue) < 1e-5f
					&& std::abs(maxLife.maxLifeTime - maxLifeValue) < 1e-5f;
				if (matches) {
					++verifiedCount;
				} else {
					++mismatchCount;
				}
			}
		}
	}

	float lifeValue{};
	float maxLifeValue{};
	size_t verifiedCount{};
	size_t mismatchCount{};
};

} // namespace

TestReport RunParticleMachineSelfTest() {
	constexpr uint64_t kParticlesToEmit = 4;
	constexpr float kLifeValue = 0.5f;
	constexpr float kMaxLifeValue = 5.0f;

	TestReport report;

	wp::ParticleMachine machine;
	wp::ParticleBuffer buffer(0, wp::ParticleLayout::BuildStdParticleLayout());
	machine.AddBuffer(buffer);
	machine.SetSamplingData(1.0f, 1, true);

	auto emitter = machine.Graph().CreateNode<wp::ParticleEmitter>();
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MinEmit>(static_cast<float>(kParticlesToEmit));
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MaxEmit>(static_cast<float>(kParticlesToEmit));
	emitter->input.SetDefaultValue<wp::ParticleEmitter::Rate>(1.0f);
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MinFrequency>(0.0f);
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MaxFrequency>(0.0f);
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MaxRuntime>(-1.0);
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MaxParticles>(kParticlesToEmit);

	auto writer = machine.Graph().CreateNode<ParticleTestWriter>(kLifeValue, kMaxLifeValue);
	auto commit = machine.Graph().CreateNode<wp::CommitEmissionNode>();
	auto inspector = machine.Graph().CreateNode<ParticleTestInspector>(kLifeValue, kMaxLifeValue);

	machine.Graph().AddRootTrigger(emitter);
	emitter->ConnectTrigger(writer);
	writer->ConnectTrigger(commit);
	commit->ConnectTrigger(inspector);

	machine.Execute();

	auto editableSpan = buffer.EditableSpan<wp::layout::LifeTime, wp::layout::MaxLifeTime>();
	const size_t editableCount = editableSpan.size();

	report.Expect(writer->wroteCount == kParticlesToEmit, "Writer node did not emit expected particle count");
	report.Expect(inspector->verifiedCount == kParticlesToEmit, "Inspector verified count mismatch");
	report.Expect(inspector->mismatchCount == 0, "Inspector reported mismatched lifetimes");
	report.Expect(editableCount == kParticlesToEmit, "Editable span size mismatch");

	return report;
}

} // namespace weave::tests::particles
