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

void InputPipeline::FeedKeyEvent(VirtualDevice device, VirtualKey key, VirtualKeyState state) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData{ device, key, state }, inputState);
	}
}

void InputPipeline::FeedKeyPressure(VirtualDevice device, VirtualKey key, float pressure) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData::BuildPressure(device, key, pressure), inputState);
	}
}

void InputPipeline::FeedKeyPressureDelta(VirtualDevice device, VirtualKey key, float pressure) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData::BuildPressureDelta(device, key, pressure), inputState);
	}
}

void InputPipeline::FeedCursorPosition(VirtualDevice device, VirtualKey key, int x, int y, int z) {
	FeedCursorPosition(device, key, (float)x,(float)y,(float)z);
}

void InputPipeline::FeedCursorPosition(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData::BuildCursorPosition(device, key, Vector3{x, y, z}), inputState);
	}
}


void InputPipeline::FeedCursorDelta(VirtualDevice device, VirtualKey key, int x, int y, int z) {
	FeedCursorDelta(device, key, (float)x,(float)y,(float)z);
}

void InputPipeline::FeedCursorDelta(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData::BuildCursorDelta(device, key, Vector3(x, y, z)), inputState);
	}
}

void InputPipeline::FeedCursorPositionAndDelta(VirtualDevice device, VirtualKey key, int px, int py, int pz, int mx, int my, int mz) {
	FeedCursorPositionAndDelta(device, key, (float)px,(float)py,(float)pz,(float)mx,(float)my,(float)mz);
}

void InputPipeline::FeedCursorPositionAndDelta(VirtualDevice device, VirtualKey key, float px, float py, float pz, float dx, float dy, float dz) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData{ device, key, Vector3(px, py, pz), Vector3(dx, dy, dz) }, inputState);
	}
}

void InputPipeline::SetCursorNormalization(VirtualDevice device, VirtualKey key, float x, float y, float z) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData::BuildCursorNormalization(device, key, Vector3(x, y, z)), inputState);
	}
}

void InputPipeline::SetKeyPressureNormalization(VirtualDevice device, VirtualKey key, float x) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData::BuildPressureNormalization(device, key, x), inputState);
	}
}

void InputPipeline::FeedDeviceEvent(VirtualDevice device, DeviceState state) {
	std::shared_lock lock(pipelineMutex);
	if (auto head = GetFrontProcessor()) {
		head->InputEvent(InputEventData{ device, state }, inputState);
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
