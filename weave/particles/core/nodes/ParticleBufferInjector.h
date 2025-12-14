#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleBufferInjector : public ParticleNode<
	blender::In<size_t>,
	blender::Flow<ParticleBuffer*>> {
public:
	
	ParticleBufferInjector();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		TargetBufferIndex,
		InputCount
	};

	constexpr static size_t kAllBuffers = size_t(-1);
};

} // namespace weave::particles
