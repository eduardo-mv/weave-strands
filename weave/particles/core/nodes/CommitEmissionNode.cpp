#include "CommitEmissionNode.h"

#include "weave/particles/core/ParticleBuffer.h"

namespace weave::particles {

void CommitEmissionNode::ExecuteNode() {
	GetContext().CommitEmissionparticles();
}

} // namespace weave::particles
