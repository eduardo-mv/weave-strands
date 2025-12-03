#pragma once

#include <memory>
#include <vector>

#include "weave/system/blender/Blender.h"

namespace weave::particles {

class ParticleBuffer;
struct ParticleContext;

class ParticleMachine {
public:
	ParticleMachine();
	~ParticleMachine();

	ParticleMachine(ParticleMachine const&) = delete;
	ParticleMachine(ParticleMachine&&) noexcept = delete;
	ParticleMachine& operator=(ParticleMachine const&) = delete;
	ParticleMachine& operator=(ParticleMachine&&) noexcept = delete;

	blender::Blender& Graph() { return blenderGraph; }
	blender::Blender const& Graph() const { return blenderGraph; }

	ParticleContext& Context();
	ParticleContext const& Context() const;

	void ClearBuffers();
	void SetBuffers(std::vector<ParticleBuffer*> buffers);
	void AddBuffer(ParticleBuffer& buffer);

	void SetSamplingData(float deltaTime, uint64_t iteration, bool isVisible);

	void Execute();

private:
	blender::Blender blenderGraph;
	std::shared_ptr<ParticleContext> context;
};

} // namespace weave::particles

