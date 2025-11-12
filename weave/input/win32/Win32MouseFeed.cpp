#include "Win32MouseFeed.h"
using namespace weave::input;

void Win32MouseFeed::FeedAsyncInput(HWND hwnd, InputPipeline& inputPipeline) {
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(hwnd, &point);

	inputPipeline.FeedCursorPosition(device, VirtualKey::Cursor, point.x, point.y, 0);

	auto state = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Left, state);
	
	state = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Right, state);
	
	state = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Middle, state);
}

int Win32MouseFeed::FeedSyncInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline) {
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
	case WM_MOUSEMOVE: {
		POINT point{ LOWORD(lparam), HIWORD(lparam) };
		ScreenToClient(hwnd, &point);
		inputPipeline.FeedCursorPosition(device, VirtualKey::Cursor, point.x, point.y, 0);
		return 1;
	}

	case WM_LBUTTONDOWN:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Left, VirtualKeyState::Down);
		return 1;

	case WM_LBUTTONUP:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Left, VirtualKeyState::Up);
		return 1;

	case WM_LBUTTONDBLCLK:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Left, VirtualKeyState::Double);
		return 1;

	case WM_RBUTTONDOWN:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Right, VirtualKeyState::Down);
		return 1;

	case WM_RBUTTONUP:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Right, VirtualKeyState::Up);
		return 1;

	case WM_RBUTTONDBLCLK:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Right, VirtualKeyState::Double);
		return 1;

	case WM_MBUTTONDOWN:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Middle, VirtualKeyState::Down);
		return 1;

	case WM_MBUTTONUP:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Middle, VirtualKeyState::Up);
		return 1;

	case WM_MBUTTONDBLCLK:
		inputPipeline.FeedKeyEvent(device, VirtualKey::Click_Middle, VirtualKeyState::Double);
		return 1;
	
	default:
		return 0;
	}
}


