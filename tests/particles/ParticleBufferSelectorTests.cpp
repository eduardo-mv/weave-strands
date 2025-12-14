#include "tests/TestEntryPoints.h"

#include "weave/particles/core/ParticleLayout.h"
#include "weave/particles/core/ParticleMachine.h"
#include "weave/particles/core/ParticleBuffer.h"
#include "weave/particles/core/nodes/CommitEmissionNode.h"
#include "weave/particles/core/nodes/ParticleBufferInjector.h"
#include "weave/particles/core/nodes/ParticleEmitter.h"

#include <memory>

namespace weave::tests::particles {

namespace {

void ConfigureEmitter(weave::particles::ParticleEmitter* emitter, uint64_t count) {
	using weave::particles::ParticleEmitter;
	emitter->input.SetDefaultValue<ParticleEmitter::MinEmitHz>(static_cast<float>(count));
	emitter->input.SetDefaultValue<ParticleEmitter::MaxEmitHz>(static_cast<float>(count));
	emitter->input.SetDefaultValue<ParticleEmitter::Rate>(1.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MinFrequency>(0.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxFrequency>(0.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxRuntime>(-1.0);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxParticles>(count);
}

} // namespace

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

	auto selector = machine.Graph().CreateNode<ParticleBufferInjector>();
	auto emitter = machine.Graph().CreateNode<ParticleEmitter>();
	auto commit = machine.Graph().CreateNode<CommitEmissionNode>();
	ConfigureEmitter(emitter, kEmitCount);

	machine.Graph().AddRootFlowLink(selector);
	selector->ConnectOutflowLink(emitter);
	emitter->ConnectOutflowLink(commit);

	selector->input.SetDefaultValue<ParticleBufferInjector::TargetBufferIndex>(1);
	machine.Execute();

	auto firstBufferSpan = bufferA->EditableSpan<layout::LifeTime>();
	auto secondBufferSpan = bufferB->EditableSpan<layout::LifeTime>();
	report.Expect(firstBufferSpan.empty(), "Manual selector wrote to unexpected buffer");
	report.Expect(secondBufferSpan.size() == kEmitCount, "Manual selector failed to select requested buffer");

	selector->input.SetDefaultValue<ParticleBufferInjector::TargetBufferIndex>(0);
	emitter->input.SetDefaultValue<ParticleEmitter::ResetSignal>(1);
	machine.Execute();

	firstBufferSpan = bufferA->EditableSpan<layout::LifeTime>();
	secondBufferSpan = bufferB->EditableSpan<layout::LifeTime>();
	report.Expect(firstBufferSpan.size() == kEmitCount, "Manual selector failed to switch buffers");
	report.Expect(secondBufferSpan.size() == kEmitCount, "Manual selector mutated previous buffer unexpectedly");

	return report;
}

} // namespace weave::tests::particles
