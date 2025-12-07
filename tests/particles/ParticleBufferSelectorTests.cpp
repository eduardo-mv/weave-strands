#include "tests/TestEntryPoints.h"

#include "weave/particles/core/ParticleLayout.h"
#include "weave/particles/core/ParticleMachine.h"
#include "weave/particles/core/ParticleBuffer.h"
#include "weave/particles/core/nodes/CommitEmissionNode.h"
#include "weave/particles/core/nodes/AutoParticleBufferSelector.h"
#include "weave/particles/core/nodes/ParticleBufferSelector.h"
#include "weave/particles/core/nodes/ParticleEmitter.h"

#include <memory>

namespace weave::tests::particles {

namespace {

void ConfigureEmitter(weave::particles::ParticleEmitter* emitter, uint64_t count) {
	using weave::particles::ParticleEmitter;
	emitter->input.SetDefaultValue<ParticleEmitter::MinEmit>(static_cast<float>(count));
	emitter->input.SetDefaultValue<ParticleEmitter::MaxEmit>(static_cast<float>(count));
	emitter->input.SetDefaultValue<ParticleEmitter::Rate>(1.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MinFrequency>(0.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxFrequency>(0.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxRuntime>(-1.0);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxParticles>(count);
}

} // namespace

TestReport TestAutoParticleBufferSelector() {
	using namespace weave::particles;

	constexpr uint64_t kFirstBufferCount = 3;
	constexpr uint64_t kSecondBufferCount = 5;

	TestReport report;

	ParticleMachine machine;
	auto layout = ParticleLayout::BuildStdParticleLayout();
	auto bufferA = std::make_shared<ParticleBuffer>(0, layout);
	auto bufferB = std::make_shared<ParticleBuffer>(0, layout);
	machine.AddBuffer(bufferA);
	machine.AddBuffer(bufferB);
	machine.SetSamplingData(1.0f, 0u, true);

	auto selector = machine.Graph().CreateNode<AutoParticleBufferSelector>();
	auto emitterA = machine.Graph().CreateNode<ParticleEmitter>();
	auto emitterB = machine.Graph().CreateNode<ParticleEmitter>();
	auto commitA = machine.Graph().CreateNode<CommitEmissionNode>();
	auto commitB = machine.Graph().CreateNode<CommitEmissionNode>();

	ConfigureEmitter(emitterA, kFirstBufferCount);
	ConfigureEmitter(emitterB, kSecondBufferCount);

	machine.Graph().AddRootTrigger(selector);
	selector->ConnectTrigger(emitterA);
	selector->ConnectTrigger(emitterB);
	emitterA->ConnectTrigger(commitA);
	emitterB->ConnectTrigger(commitB);

	machine.Execute();

	auto firstBufferSpan = bufferA->EditableSpan<layout::LifeTime>();
	auto secondBufferSpan = bufferB->EditableSpan<layout::LifeTime>();

	report.Expect(firstBufferSpan.size() == kFirstBufferCount, "First buffer particle count mismatch");
	report.Expect(secondBufferSpan.size() == kSecondBufferCount, "Second buffer particle count mismatch");

	return report;
}

TestReport TestParticleBufferSelector() {
	using namespace weave::particles;

	constexpr uint64_t kEmitCount = 4;

	TestReport report;

	ParticleMachine machine;
	auto layout = ParticleLayout::BuildStdParticleLayout();
	auto bufferA = std::make_shared<ParticleBuffer>(0, layout);
	auto bufferB = std::make_shared<ParticleBuffer>(0, layout);
	machine.AddBuffer(bufferA);
	machine.AddBuffer(bufferB);
	machine.SetSamplingData(1.0f, 0u, true);

	auto selector = machine.Graph().CreateNode<ParticleBufferSelector>();
	auto emitter = machine.Graph().CreateNode<ParticleEmitter>();
	auto commit = machine.Graph().CreateNode<CommitEmissionNode>();
	ConfigureEmitter(emitter, kEmitCount);

	machine.Graph().AddRootTrigger(selector);
	selector->ConnectTrigger(emitter);
	emitter->ConnectTrigger(commit);

	selector->input.SetDefaultValue<ParticleBufferSelector::TargetBufferIndex>(1);
	machine.Execute();

	auto firstBufferSpan = bufferA->EditableSpan<layout::LifeTime>();
	auto secondBufferSpan = bufferB->EditableSpan<layout::LifeTime>();
	report.Expect(firstBufferSpan.empty(), "Manual selector wrote to unexpected buffer");
	report.Expect(secondBufferSpan.size() == kEmitCount, "Manual selector failed to select requested buffer");

	selector->input.SetDefaultValue<ParticleBufferSelector::TargetBufferIndex>(0);
	emitter->input.SetDefaultValue<ParticleEmitter::ResetSignal>(1);
	machine.Execute();

	firstBufferSpan = bufferA->EditableSpan<layout::LifeTime>();
	secondBufferSpan = bufferB->EditableSpan<layout::LifeTime>();
	report.Expect(firstBufferSpan.size() == kEmitCount, "Manual selector failed to switch buffers");
	report.Expect(secondBufferSpan.size() == kEmitCount, "Manual selector mutated previous buffer unexpectedly");

	return report;
}

} // namespace weave::tests::particles
