#include "CommitEmissionNode.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

void CommitEmissionNode::ExecuteNode() {
	for(auto* buffer : flow.Iterate<ParticleBuffer*>()) {
		buffer->CommitEmittedParticles();
	}

	flow.Flush<EmissionRange>();
}

} // namespace weave::particles
