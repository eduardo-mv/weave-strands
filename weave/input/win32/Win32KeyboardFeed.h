/*
Weave Win32 Keyboard Feeder
Win32KeyboardFeed.h

Provides a standard input feed for keyboards based on the Win32 API.
Supports synchronos and async operations. 
Async input will query the Win32 API for the state of a key before generating an state change event.
Sync is intended to parse WParam and LParam from the Win32 message loop.
*/
#pragma once

#include "weave/input/system/InputPipeline.h"
#include "Win32VirtualKeys.h"
#include <vector>

namespace weave::input {

class Win32KeyboardFeed {
private:
	VirtualDevice device{ VirtualDevice::Keyboard };
	std::vector<std::pair<int, VirtualKey>> input;

public:

	Win32KeyboardFeed() = default;
	Win32KeyboardFeed(VirtualDevice device) : device(device) {}

	void SetVirtualDevice(VirtualDevice dev) { device = dev; }
	VirtualDevice GetVirtualDevice() const { return device; }

	void AddAsyncKey(int vk);
	void AddAsyncKey(VirtualKey virtualKey);
	void AddAsyncKey(int vk, VirtualKey virtualKey);

	void ClearAyncKeys() { input.clear(); }

	void FeedAsyncInput(int vk, InputPipeline& inputPipeline);
	void FeedAsyncInput(VirtualKey virtualKey, InputPipeline& inputPipeline);
	void FeedAsyncInput(InputPipeline& inputPipeline);

	int FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline);
};
	
}
