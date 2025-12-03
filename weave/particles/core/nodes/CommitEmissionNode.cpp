#include "CommitEmissionNode.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

void CommitEmissionNode::ExecuteNode() {
	for (auto* buffer : GetContext().buffers) {
		if (buffer) {
			buffer->CommitEmittedParticles();
		}
	}
}

} // namespace weave::particles

