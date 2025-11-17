/*
Weave Input Pipeline
InputPipeline.h

Pipelines control chains of InputProcessors and are fed virtual input events which are passed to every processor in the pipeline.
Contains an internal memory map with the state of all virtual devices.
*/
#pragma once

#include "VirtualKeys.h"
#include "VirtualDevices.h"
#include "InputProcessor.h"
#include "InputStateMap.h"
#include <memory>
#include <shared_mutex>
#include <vector>

namespace weave::input {

class InputPipeline {
	friend class InputProcessor;

	class InputSink : public weave::input::InputProcessor {
		void InputEvent(weave::input::InputEventData, weave::input::InputStateMap&) override {}
	};

protected:
	InputStateMap inputState;
	std::vector<std::shared_ptr<InputProcessor>> pipeline;
	InputSink inputSink;
	mutable std::shared_mutex pipelineMutex;

public:
	InputStateMap& GetInputState() { return inputState; }
	InputStateMap const& GetInputState() const { return inputState; }

	void AddProcessor(std::shared_ptr<InputProcessor> processor);
	void RemoveProcessor(std::shared_ptr<InputProcessor> processor);

	template<typename EventType>
	void FeedEvent(VirtualDevice device, VirtualKey key, EventType&& event) {
		FeedEvent(InputEventData{device, key, std::forward<EventType>(event)});
	}

	void FeedEvent(InputEventData event);

private:
	void SortPipeline();
	void SortPipelineLocked();
	InputProcessor* GetFrontProcessor();
};

}
