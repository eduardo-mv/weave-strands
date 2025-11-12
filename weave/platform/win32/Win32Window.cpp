#ifdef _WIN32

#include "Win32Window.h"	
#include <unordered_map>

weave::win32::Win32Window::Win32Window(){
	InitWindowClass(0,0,0,nullptr);
}

weave::win32::Win32Window::~Win32Window(){
	Destroy();
	UnregisterClass(wclass.lpszClassName, nullptr);
	ChangeDisplaySettings(nullptr, 0);
}

void weave::win32::Win32Window::InitWindowClass(int bigIcon, int smallIcon, int cursor, HINSTANCE hinstance) {
	//Set default values for the window class
	LPWSTR cur = (cursor == 0) ? IDC_ARROW : MAKEINTRESOURCE(cursor);
	LPWSTR iconb = (bigIcon == 0) ? IDI_WINLOGO : MAKEINTRESOURCE(bigIcon);
	LPWSTR iconsm = (smallIcon == 0) ? IDI_WINLOGO : MAKEINTRESOURCE(smallIcon);

	wclass.cbSize     = sizeof(wclass);
	wclass.cbClsExtra = 0;
	wclass.cbWndExtra = 0;
	wclass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wclass.hCursor = LoadCursor((cursor == 0 ? nullptr : hinstance), cur);
	wclass.hIcon = LoadIcon((bigIcon == 0 ? nullptr : hinstance), iconb);
	wclass.hIconSm = LoadIcon((smallIcon == 0 ? nullptr : hinstance), iconsm);
	wclass.hInstance = hinstance;
	wclass.lpszMenuName = nullptr;
	wclass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	wclass.lpfnWndProc = DefWinProc;
	wclass.lpszClassName = DefaultWinClass;
}

void weave::win32::Win32Window::SetWinProc(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> const& winProcCallback) {
	exWinProcCallback = winProcCallback;
}

bool weave::win32::Win32Window::Create(wchar_t const* caption, unsigned long style_, unsigned long exstyle_) {
	if(hwnd)
		DestroyWindow(hwnd);

	if (!RegisterClassEx(&wclass))
		return false;

	style = style_;
	exstyle = exstyle_;

	if(!style)
		style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	
	hwnd = CreateWindowEx(
		exstyle,
		wclass.lpszClassName,
		caption ? caption : DefaultCaption,
		style,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr, nullptr, nullptr, this);

	if(!hwnd)
		return false;
	
	UpdateWindow(hwnd);
	SetWindowTextW(hwnd, caption ? caption : DefaultCaption);
	
	UnregisterClass(wclass.lpszClassName, nullptr);

	return true;
}

bool weave::win32::Win32Window::Fullscreen(uint32_t width, uint32_t height, uint32_t hz)
{
	//Screen size
	DEVMODE current = { };
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &current);

	if (width == 0) width = current.dmPelsWidth;
	if (height == 0) height = current.dmPelsHeight;
	if (hz == 0) hz = current.dmDisplayFrequency;

	//Change the resolution of the desktop temporarily
	DEVMODE	dm = {};
	dm.dmSize = sizeof(dm);
	dm.dmPelsWidth = width;
	dm.dmPelsHeight = height;
	dm.dmBitsPerPel = 32;
	dm.dmDisplayFrequency = hz;
	//dm.dmFields	= DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
	dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		return false;

	//Set the window to borderless
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
	//Set the position and size to the target screen size
	SetWindowPos(hwnd, nullptr, 0, 0, width, height, 0);

	//Show the window
	ShowWindow(hwnd, SW_SHOWMAXIMIZED);

	return true;
}

void weave::win32::Win32Window::Windowed(int x, int y, uint32_t width, uint32_t height, bool sizeIsWindow, bool borderless, bool show) {

	//Current size
	DEVMODE current{};
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &current);

	//Set the resolution of the desktop back to normal
	ChangeDisplaySettings(nullptr, 0);

	//Set the window to borderless or border
	if(borderless)
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
	else
		SetWindowLongPtr(hwnd, GWL_STYLE, style);

	//Screen size
	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (width == 0) width = nScreenWidth;
	if (height == 0) height = nScreenHeight;
	if (x == -1) x = (nScreenWidth - width) / 2;
	if (y == -1) y = (nScreenHeight - height) / 2;

	//Window size
	RECT rect{};
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;

	if (!sizeIsWindow) {
		if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle)) {
			rect.left = 0;
			rect.top = 0;
		}
	}

	//Set the position and size to the target screen size
	SetWindowPos(hwnd, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
	//Show the window
	Show(show);
}

void weave::win32::Win32Window::Maximize() {
	ShowWindow(hwnd, SW_SHOWMAXIMIZED);
}

void weave::win32::Win32Window::Minimize() {
	ShowWindow(hwnd, SW_SHOWMINIMIZED);
}

void weave::win32::Win32Window::Show(bool show) {
	ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
}

std::pair<uint32_t, uint32_t> weave::win32::Win32Window::GetClientArea() {
	RECT rect;
	GetClientRect(hwnd, &rect);
	return { uint32_t(rect.right - rect.left), uint32_t(rect.bottom - rect.top) };
}

std::pair<uint32_t, uint32_t> weave::win32::Win32Window::GetWindowArea() {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	return { uint32_t(rect.right - rect.left), uint32_t(rect.bottom - rect.top) };
}

void weave::win32::Win32Window::Destroy(){
	if(hwnd) {
		DestroyWindow(hwnd);
		//Destroying the window generates a WM_QUIT message that must be retrived or else the next window created may fail (for example a dialog box)
		MSG msg;
		PeekMessage(&msg,0,WM_QUIT,WM_QUIT,PM_REMOVE);
	}
	hwnd = nullptr;
}

void weave::win32::Win32Window::MessagePump(){
	pumpLoop = true;

	MSG msg;
	while (pumpLoop) {
		//Check Event
		if (GetMessage(&msg, hwnd, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void weave::win32::Win32Window::MessagePeekPump(std::function<void()> idleCallback) {
	pumpLoop = true;
	
	MSG msg;
	while (pumpLoop) {
		//Check Event
		if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		idleCallback();
	}
}

LRESULT weave::win32::Win32Window::ExWinProc(HWND hwndIn, UINT msg, WPARAM wparam, LPARAM lparam) {
	return exWinProcCallback(hwndIn, msg, wparam, lparam);
}

LRESULT CALLBACK weave::win32::Win32Window::DefWinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	static std::unordered_map<HWND, Win32Window*> hwndMap;

	Win32Window* _this = hwndMap[hwnd];
	if (_this && _this->ExWinProc(hwnd, msg, wparam, lparam)) {
		return TRUE;
	}

    switch(msg)
    {
		case WM_NCCREATE: 
		{
			LPCREATESTRUCT info = reinterpret_cast<LPCREATESTRUCT>(lparam);
			hwndMap[hwnd] = static_cast<Win32Window*>(info->lpCreateParams);
			return TRUE;
		}

		case WM_QUIT:
			_this->StopMessagePump();
			return TRUE;

        case WM_CLOSE:
            DestroyWindow(hwnd);
			_this->StopMessagePump();
			hwndMap[hwnd] = nullptr;
			return TRUE;

        case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;

		default: {
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
    }
}

#endif
