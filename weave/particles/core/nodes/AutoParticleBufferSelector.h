#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

// Automatically selects buffers based on trigger index order.
class AutoParticleBufferSelector : public ParticleNode<> {
public:
	void ExecuteNode() override;
	bool TriggerFired(size_t index) override;
};

} // namespace weave::particles
