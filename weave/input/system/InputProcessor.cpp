#include "InputProcessor.h"
#include "InputPipeline.h"

using namespace weave::input;

void InputProcessor::SetPriority(int prio) {
	priority = prio;
	if (ownerPipeline) {
		ownerPipeline->SortPipeline();
	}
}

void InputProcessor::Enable() {
	enabled.store(true, std::memory_order_release);
}

void InputProcessor::Disable() {
	enabled.store(false, std::memory_order_release);
}


