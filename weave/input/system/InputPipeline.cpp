#include "InputPipeline.h"
#include <algorithm>
#include <functional>
#include <cassert>

using namespace weave::input;

void InputPipeline::AddProcessor(std::shared_ptr<InputProcessor> processor) {
	if (processor->ownerPipeline == this) {
		return;
	}

	std::unique_lock lock(pipelineMutex);

	if (processor->ownerPipeline != nullptr) {
		processor->ownerPipeline->RemoveProcessor(processor);
	}

	processor->ownerPipeline = this;
	processor->nextProcessor.store(std::shared_ptr<InputProcessor>{}, std::memory_order_release);
	
	pipeline.emplace_back(processor);
	
	SortPipelineLocked();
}

void InputPipeline::RemoveProcessor(std::shared_ptr<InputProcessor> processor) {
	if (processor->ownerPipeline != this) {
		return;
	}

	std::unique_lock lock(pipelineMutex);
	
	processor->ownerPipeline = nullptr;
	std::erase(pipeline, processor);
	processor->nextProcessor.store(std::shared_ptr<InputProcessor>{}, std::memory_order_release);

	SortPipelineLocked();
}

InputProcessor* InputPipeline::GetFrontProcessor() {
	if (pipeline.empty()) {
		return &inputSink;
	}
	return pipeline.front().get();
}

void InputPipeline::FeedEvent(InputEventData event) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(std::move(event), inputState);
	}
}

void InputPipeline::SortPipeline() {
	std::unique_lock lock(pipelineMutex);
	SortPipelineLocked();
}

void InputPipeline::SortPipelineLocked() {
	if (pipeline.empty()) {
		return;
	}

	std::sort(pipeline.begin(), pipeline.end(), [](auto const& a, auto const& b) {
		return a->priority > b->priority;
	});

	for (size_t i = 1, end = pipeline.size(); i < end; ++i) {
		pipeline[i - 1]->nextProcessor.store(pipeline[i], std::memory_order_release);
	}
}
