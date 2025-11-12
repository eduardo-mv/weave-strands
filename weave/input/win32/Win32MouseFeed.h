/*
Weave Win32 Mouse Feeder
Win32MouseFeed.h

Provides a standard input feed for mice based on the Win32 API.
Supports synchronous and async operations.
Async input will query the Win32 API for the state of a key before generating an state change event.
Sync is intended to parse WParam and LParam from the Win32 message loop.
*/
#pragma once

#include "weave/input/system/InputPipeline.h"
#include "Win32VirtualKeys.h"
#include <vector>

namespace weave::input {

class Win32MouseFeed {
protected:
	VirtualDevice device{ VirtualDevice::Mouse };
public:

	Win32MouseFeed() = default;
	Win32MouseFeed(VirtualDevice device) : device(device) {}

	void FeedAsyncInput(HWND hwnd, InputPipeline& inputPipeline);

	int FeedSyncInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline);
};
	

}
