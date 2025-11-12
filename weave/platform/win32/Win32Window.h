/*
Win32 Window wrapper

Simplified wrapper to control Win32 window creation.
This is mostly meant for single window applications without any Windows GUI elements (e.g. Games).
*/
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <utility>

namespace weave {
namespace win32 {

class Win32Window {
	static constexpr const wchar_t DefaultWinClass[] = L"Win32 Window Class";
	static constexpr const wchar_t DefaultCaption[] = L"Win32 Application";

protected:
	HWND hwnd{};
	HWND hparent{};
	WNDCLASSEX wclass{};
	unsigned long style = 0, exstyle = 0;
	bool pumpLoop = false;

	std::function<bool(HWND, UINT, WPARAM, LPARAM)> exWinProcCallback = [](HWND, UINT, WPARAM, LPARAM) { return false; };

public:
	Win32Window();
	~Win32Window();

	HWND GetHWND() const { return hwnd; }
	HWND GetParentHWND() const { return hparent; }

	//Overrides default window class values
	void InitWindowClass(int bigIcon, int smallIcon, int cursor, HINSTANCE hinstance); 
	
	//Sets the WinProc user side message loop for this window
	void SetWinProc(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> const & winProcCallback);
	
	bool Create(wchar_t const* caption = nullptr, unsigned long style = 0, unsigned long exstyle = 0);
	
	//Sets fullscreen mode on an already created window
	//Set values to zero for max screen size and refresh rate
	bool Fullscreen(uint32_t width = 0, uint32_t height = 0, uint32_t hz = 0);
	
	//Sets windowed mode on an already created window
	//Set width and height to zero for max screen size
	void Windowed(uint32_t width, uint32_t height, bool sizeIsWindow, bool borderless, bool show) { Windowed(-1, -1, width, height, sizeIsWindow, borderless, show); }
	void Windowed(int x, int y, uint32_t width, uint32_t height, bool sizeIsWindow, bool borderless, bool show);
	
	void Maximize();
	void Minimize();

	void Show(bool show);

	//Returns the client area (inside of window)
	std::pair<uint32_t, uint32_t> GetClientArea();
	//Returns the window area (including frame)
	std::pair<uint32_t, uint32_t> GetWindowArea();

	void Destroy(); 

	//Starts the Win32 message pump for the window
	void MessagePump();
	void MessagePeekPump(std::function<void()> idleCallback);
	void StopMessagePump() { pumpLoop = false; }
private:
	//Extended WinProc which calls the user supplied callback
	LRESULT ExWinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	//Controlling DefWinProc to handle the message pump
	static LRESULT CALLBACK DefWinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); 
};

}
}
