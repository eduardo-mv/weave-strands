#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class CommitEmissionNode : public ParticleNode<blender::Flow<ParticleBuffer*, EmissionRange>> {
public:
	void ExecuteNode() override;
};

} // namespace weave::particles

