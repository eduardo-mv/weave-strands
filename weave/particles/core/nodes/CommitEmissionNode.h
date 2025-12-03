#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class CommitEmissionNode : public ParticleNode<> {
public:
	void ExecuteNode() override;
};

} // namespace weave::particles

