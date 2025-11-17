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
	inputPipeline.FeedEvent(device, input::Win32VkToVirtualKey(vk), KeyStateEvent{ state });
}

void Win32KeyboardFeed::FeedAsyncInput(VirtualKey virtualKey, InputPipeline& inputPipeline) {
	auto vk = input::Win32VirtualKeyToVk(virtualKey);
	auto state = (GetAsyncKeyState(INT(vk)) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedEvent(device, virtualKey, KeyStateEvent{ state });
}

void Win32KeyboardFeed::FeedAsyncInput(InputPipeline& inputPipeline) {
	for (auto const& i : input) {
		auto state = (GetAsyncKeyState(i.first) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
		inputPipeline.FeedEvent(device, i.second, KeyStateEvent{ state });
	}
}

int Win32KeyboardFeed::FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline) {
	(void)lparam;

	switch(msg) {
	case WM_ACTIVATE:
	{
		if (LOWORD(wparam) == WA_INACTIVE) {
			inputPipeline.FeedEvent(device, VirtualKey::None, DeviceStateEvent{ DeviceState::FocusLost });
		} else if (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE) {
			inputPipeline.FeedEvent(device, VirtualKey::None, DeviceStateEvent{ DeviceState::FocusGained });
		}
		return 0;
	}
	case WM_KEYDOWN:
		{
		VirtualKey vk = input::Win32VkToVirtualKey(wparam);
		inputPipeline.FeedEvent(device, vk, KeyStateEvent{ VirtualKeyState::Down });
		return 1;
		}

	case WM_KEYUP:
		{
		VirtualKey vk = input::Win32VkToVirtualKey(wparam);
		inputPipeline.FeedEvent(device, vk, KeyStateEvent{ VirtualKeyState::Up });
		return 1;
		}
	
	default:
		return 0;
	}
}
