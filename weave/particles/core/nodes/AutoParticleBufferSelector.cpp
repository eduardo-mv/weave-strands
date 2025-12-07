#include "AutoParticleBufferSelector.h"

namespace weave::particles {

void AutoParticleBufferSelector::ExecuteNode() {
	// NOOP
}

bool AutoParticleBufferSelector::TriggerFired(size_t index) {
	GetContext().SetCurrentTargetBufferIndex(index);
	return true;
}

} // namespace weave::particles
