#include "tests/TestEntryPoints.h"

#include "weave/particles/core/ParticleBuffer.h"
#include "weave/particles/core/ParticleLayout.h"
#include "weave/particles/core/ParticleMachine.h"
#include "weave/particles/core/ParticleContext.h"
#include "weave/particles/core/nodes/CommitEmissionNode.h"
#include "weave/particles/core/nodes/ParticleEmitter.h"
#include "weave/particles/core/nodes/ParticleNode.h"
#include "weave/particles/core/nodes/ParticleBufferInjector.h"

#include <cmath>
#include <cstddef>
#include <memory>

namespace weave::tests::particles {
namespace {

namespace wp = ::weave::particles;
namespace wb = ::weave::blender;

struct ParticleTestWriter : wp::ParticleNode<wb::Flow<wp::EmissionRange>> {
	ParticleTestWriter(float lifeValue, float maxLifeValue)
		: lifeValue(lifeValue), maxLifeValue(maxLifeValue) {}

	void ExecuteNode() override {
		wroteCount = 0;
		for (auto const& emissionRange : flow.Iterate<wp::EmissionRange>()) {
			if (!emissionRange) {
				continue;
			}

			for (auto&& [life, maxLife] : emissionRange.EmissionSpan<wp::layout::LifeTime, wp::layout::MaxLifeTime>()) {
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

struct ParticleTestInspector : wp::ParticleNode<wb::Flow<wp::ParticleBuffer*>> {
	ParticleTestInspector(float lifeValue, float maxLifeValue)
		: lifeValue(lifeValue), maxLifeValue(maxLifeValue) {}

	void ExecuteNode() override {
		verifiedCount = 0;
		mismatchCount = 0;
		for (auto* buffer : flow.Iterate<wp::ParticleBuffer*>()) {
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
	auto buffer = std::make_shared<wp::ParticleBuffer>(0, wp::ParticleLayout::BuildStdParticleLayout());
	machine.AddBuffer(buffer);
	machine.SetSamplingData(1.0f, 1, true);

	auto bufferInjector = machine.Graph().CreateNode<wp::ParticleBufferInjector>();
	bufferInjector->input.SetDefaultValue<wp::ParticleBufferInjector::TargetBufferIndex>(wp::ParticleBufferInjector::kAllBuffers);

	auto emitter = machine.Graph().CreateNode<wp::ParticleEmitter>();
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MinEmitHz>(static_cast<float>(kParticlesToEmit));
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MaxEmitHz>(static_cast<float>(kParticlesToEmit));
	emitter->input.SetDefaultValue<wp::ParticleEmitter::Rate>(1.0f);
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MinFrequency>(0.0f);
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MaxFrequency>(0.0f);
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MaxRuntime>(-1.0);
	emitter->input.SetDefaultValue<wp::ParticleEmitter::MaxParticles>(kParticlesToEmit);

	auto writer = machine.Graph().CreateNode<ParticleTestWriter>(kLifeValue, kMaxLifeValue);
	auto commit = machine.Graph().CreateNode<wp::CommitEmissionNode>();
	auto inspector = machine.Graph().CreateNode<ParticleTestInspector>(kLifeValue, kMaxLifeValue);

	machine.Graph().AddRootFlowLink(bufferInjector);
	bufferInjector->ConnectOutflowLink(emitter);
	emitter->ConnectOutflowLink(writer);
	writer->ConnectOutflowLink(commit);
	commit->ConnectOutflowLink(inspector);

	machine.Execute();

	auto editableSpan = buffer->EditableSpan<wp::layout::LifeTime, wp::layout::MaxLifeTime>();
	const size_t editableCount = editableSpan.size();

	report.Expect(writer->wroteCount == kParticlesToEmit, "Writer node did not emit expected particle count");
	report.Expect(inspector->verifiedCount == kParticlesToEmit, "Inspector verified count mismatch");
	report.Expect(inspector->mismatchCount == 0, "Inspector reported mismatched lifetimes");
	report.Expect(editableCount == kParticlesToEmit, "Editable span size mismatch");

	return report;
}

} // namespace weave::tests::particles
