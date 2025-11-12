/*
Input Processor:
State Writter

Writes the input state data provided into the InputStateMap.

This node will usually be present only once in a processor chain, located after any filtering desired.

*/
#pragma once

#include "weave/input/system/InputProcessor.h"

namespace weave::input {

class InputStateWriter : public InputProcessor {
	void InputEvent(InputEventData inputData, InputStateMap& inputState) override;
};

}