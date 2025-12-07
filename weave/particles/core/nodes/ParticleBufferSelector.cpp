#include "ParticleBufferSelector.h"

#include <algorithm>

namespace weave::particles {

void ParticleBufferSelector::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}

	const size_t requestedIndex = this->input.template Ref<TargetBufferIndex>();
	const size_t selectedIndex = std::min(requestedIndex, context.BufferCount() - 1);
	context.SetCurrentTargetBufferIndex(selectedIndex);
}

} // namespace weave::particles
