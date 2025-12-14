#include "ParticleBufferInjector.h"

#include <algorithm>

namespace weave::particles {

ParticleBufferInjector::ParticleBufferInjector() {
	this->input.SetDefaultValues(kAllBuffers);
}

void ParticleBufferInjector::ExecuteNode() {
	auto& context = GetContext();
	if (context.BufferCount() == 0) {
		return;
	}

	const size_t requestedIndex = this->input.template Ref<TargetBufferIndex>();
	if(requestedIndex == kAllBuffers) {
		for(auto& buffer : context.buffers) {
			flow.Push(buffer.get());
		}
	}
	else if(requestedIndex < context.BufferCount()) {
		flow.Push(context.buffers[requestedIndex].get());
	}
	
}

} // namespace weave::particles
