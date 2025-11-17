#include "Win32MouseFeed.h"
using namespace weave::input;

void Win32MouseFeed::FeedAsyncInput(HWND hwnd, InputPipeline& inputPipeline) {
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(hwnd, &point);

	inputPipeline.FeedEvent(device, VirtualKey::Cursor,
		CursorPositionEvent{ Vector3{ static_cast<float>(point.x), static_cast<float>(point.y), 0.0f } });

	auto state = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedEvent(device, VirtualKey::Click_Left, KeyStateEvent{ state });
	
	state = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedEvent(device, VirtualKey::Click_Right, KeyStateEvent{ state });
	
	state = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? (VirtualKeyState::Down) : (VirtualKeyState::Up);
	inputPipeline.FeedEvent(device, VirtualKey::Click_Middle, KeyStateEvent{ state });
}

int Win32MouseFeed::FeedSyncInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline) {
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
	case WM_MOUSEMOVE: {
		POINT point{ LOWORD(lparam), HIWORD(lparam) };
		ScreenToClient(hwnd, &point);
		inputPipeline.FeedEvent(device, VirtualKey::Cursor,
			CursorPositionEvent{ Vector3{ static_cast<float>(point.x), static_cast<float>(point.y), 0.0f } });
		return 1;
	}

	case WM_LBUTTONDOWN:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Left, KeyStateEvent{ VirtualKeyState::Down });
		return 1;

	case WM_LBUTTONUP:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Left, KeyStateEvent{ VirtualKeyState::Up });
		return 1;

	case WM_LBUTTONDBLCLK:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Left, KeyStateEvent{ VirtualKeyState::Double });
		return 1;

	case WM_RBUTTONDOWN:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Right, KeyStateEvent{ VirtualKeyState::Down });
		return 1;

	case WM_RBUTTONUP:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Right, KeyStateEvent{ VirtualKeyState::Up });
		return 1;

	case WM_RBUTTONDBLCLK:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Right, KeyStateEvent{ VirtualKeyState::Double });
		return 1;

	case WM_MBUTTONDOWN:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Middle, KeyStateEvent{ VirtualKeyState::Down });
		return 1;

	case WM_MBUTTONUP:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Middle, KeyStateEvent{ VirtualKeyState::Up });
		return 1;

	case WM_MBUTTONDBLCLK:
		inputPipeline.FeedEvent(device, VirtualKey::Click_Middle, KeyStateEvent{ VirtualKeyState::Double });
		return 1;
	
	default:
		return 0;
	}
}


