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

	void FeedKeyEvent(VirtualDevice device, VirtualKey keyId, VirtualKeyState state);
	void FeedKeyPressure(VirtualDevice device, VirtualKey keyId, float pressure);
	void FeedKeyPressureDelta(VirtualDevice device, VirtualKey keyId, float pressure);
	void SetKeyPressureNormalization(VirtualDevice device, VirtualKey keyId, float x);

	void FeedCursorPosition(VirtualDevice device, VirtualKey keyId, int x, int y, int z);
	void FeedCursorPosition(VirtualDevice device, VirtualKey keyId, float x, float y, float z);

	void FeedCursorDelta(VirtualDevice device, VirtualKey keyId, int x, int y, int z);
	void FeedCursorDelta(VirtualDevice device, VirtualKey keyId, float x, float y, float z);

	void FeedCursorPositionAndDelta(VirtualDevice device, VirtualKey keyId, int px, int py, int pz, int dx, int dy, int dz);
	void FeedCursorPositionAndDelta(VirtualDevice device, VirtualKey keyId, float px, float py, float pz, float dx, float dy, float dz);
	void SetCursorNormalization(VirtualDevice device, VirtualKey keyId, float x, float y, float z);

	void FeedDeviceEvent(VirtualDevice device, DeviceState state);

private:
	void SortPipeline();
	void SortPipelineLocked();
	InputProcessor* GetFrontProcessor();
};

}
