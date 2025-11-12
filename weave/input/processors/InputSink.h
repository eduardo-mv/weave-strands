/*
Input Processor:
Sink

Processor node that stops input prograpation

*/

#pragma once

#include "weave/input/system/InputProcessor.h"

namespace weave::input {
class InputSink : public InputProcessor {
public:
	void InputEvent(InputEventData, InputStateMap&) override {}
};
}