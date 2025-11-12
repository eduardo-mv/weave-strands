#include "Win32KeyboardFeed.h"

using namespace weave::input;

void Win32KeyboardFeed::AddAsyncKey(int vk) {
	input.emplace_back(vk, input::Win32VkToVirtualKey(vk));
}

void Win32KeyboardFeed::AddAsyncKey(VirtualKey virtualKey) {
	input.emplace_back(INT(input::Win32VirtualKeyToVk(virtualKey)), virtualKey);
}
void Win32KeyboardFeed::AddAsyncKey(int vk, VirtualKey virtualKey) {
	input.emplace_back(vk, virtualKey);
}

void Win32KeyboardFeed::FeedAsyncInput(int vk, InputPipeline& inputPipeline) {
	auto state = (GetAsyncKeyState(vk) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedKeyEvent(device, input::Win32VkToVirtualKey(vk), state);
}

void Win32KeyboardFeed::FeedAsyncInput(VirtualKey virtualKey, InputPipeline& inputPipeline) {
	auto vk = input::Win32VirtualKeyToVk(virtualKey);
	auto state = (GetAsyncKeyState(INT(vk)) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedKeyEvent(device, virtualKey, state);
}

void Win32KeyboardFeed::FeedAsyncInput(InputPipeline& inputPipeline) {
	for (auto const& i : input) {
		auto state = (GetAsyncKeyState(i.first) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
		inputPipeline.FeedKeyEvent(device, i.second, state);
	}
}

int Win32KeyboardFeed::FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline) {
	(void)lparam;

	switch(msg) {
	case WM_ACTIVATE:
	{
		if (LOWORD(wparam) == WA_INACTIVE) {
			inputPipeline.FeedDeviceEvent(device, DeviceState::FocusLost);
		} else if (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE) {
			inputPipeline.FeedDeviceEvent(device, DeviceState::FocusGained);
		}
		return 0;
	}
	case WM_KEYDOWN:
		{
		VirtualKey vk = input::Win32VkToVirtualKey(wparam);
		inputPipeline.FeedKeyEvent(device, vk,VirtualKeyState::Down);
		return 1;
		}

	case WM_KEYUP:
		{
		VirtualKey vk = input::Win32VkToVirtualKey(wparam);
		inputPipeline.FeedKeyEvent(device, vk,VirtualKeyState::Up);
		return 1;
		}
	
	default:
		return 0;
	}
}
