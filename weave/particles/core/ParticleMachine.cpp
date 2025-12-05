#include "ParticleMachine.h"

#include "ParticleContext.h"

namespace weave::particles {

ParticleMachine::ParticleMachine() {
	context = blenderGraph.GetUniform<ParticleContext>();
	graphTime = blenderGraph.GetUniform<blender::GraphTime>();
}

ParticleMachine::~ParticleMachine() = default;

ParticleContext& ParticleMachine::Context() {
	return *context;
}

ParticleContext const& ParticleMachine::Context() const {
	return *context;
}

void ParticleMachine::ClearBuffers() {
	context->ClearBuffers();
}

void ParticleMachine::SetBuffers(std::vector<ParticleBuffer*> buffers) {
	context->SetBuffers(std::move(buffers));
}

void ParticleMachine::AddBuffer(ParticleBuffer& buffer) {
	context->AddBuffer(buffer);
}

void ParticleMachine::SetSamplingData(float deltaTime, uint64_t iteration, bool isVisible) {
	context->sampling.deltaTime = deltaTime;
	context->sampling.iteration = iteration;
	context->sampling.isVisible = isVisible;
	if (graphTime) {
		graphTime->deltaSeconds = deltaTime;
		graphTime->totalSeconds += static_cast<double>(deltaTime);
	}
}

void ParticleMachine::Execute() {
	blenderGraph.Execute();
}

} // namespace weave::particles
