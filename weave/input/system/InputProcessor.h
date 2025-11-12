/*
Weave Input Processor
InputProcessor.h

Input processors run inside an InputPipeline in sequence, allowing to affect the next stage of the pipeline
*/
#pragma once

#include "InputStateMap.h"
#include "InputEventData.h"
#include <atomic>
#include <memory>

namespace weave::input {

class InputPipeline;

class InputProcessor {
	friend class InputPipeline;

private:
	InputPipeline* ownerPipeline{};
	std::atomic<std::shared_ptr<InputProcessor>> nextProcessor{ std::shared_ptr<InputProcessor>{} };

protected:
	int32_t priority = 0;
	std::atomic<bool> enabled{ true };
public:
	virtual ~InputProcessor() = default;
	

	int GetPriority() const { return priority; }
	void SetPriority(int prio);

	void Enable();
	void Disable();

	bool IsEnabled() const { return enabled.load(std::memory_order_acquire); }

	virtual void InputEvent(InputEventData inputData, InputStateMap& inputState) { NextProcessor(inputData, inputState); }

protected:
	void NextProcessor(InputEventData inputData, InputStateMap& inputState) const {
		auto next = nextProcessor.load(std::memory_order_acquire);
		while (next && !next->IsEnabled()) {
			next = next->nextProcessor.load(std::memory_order_acquire);
		}
		
		if (next) {
			next->InputEvent(inputData, inputState);
		}
}
};

}
