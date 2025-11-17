#include "RawInputMouseFeed.h"
#include <limits>
#include <cassert>

#include "weave/system/utf/Utf.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated wstring_convert in C++17
#endif

using namespace weave::input;


RawInputMouseFeed::HidInfo& RawInputMouseFeed::HidInfo::operator=(HidInfo&& other)
{
	std::swap(hDevice, other.hDevice);
	std::swap(hidDevice, other.hidDevice);
	return *this;
}

RawInputMouseFeed::HidInfo::HidInfo(HidInfo&& other)
{
	std::swap(hDevice, other.hDevice);
	std::swap(hidDevice, other.hidDevice);
}


RawInputMouseFeed::HidInfo::~HidInfo() {
	if (hidDevice) {
		CloseHandle(hidDevice);
	}
}

bool RawInputMouseFeed::MouseDeviceData::Initialize(HANDLE h) {

	hidInfo.hDevice = h;

	//Attempt to fetch a vendor name. For this the HID device name must be retrieved with the RawInput API and with this name we can open the device as a file, which
	//gives us access to the HidD_* style functions
	static const uint32_t maxDeviceNameSize = 256;
	wchar_t rawInputDeviceName[maxDeviceNameSize] = {};
	wchar_t productString[maxDeviceNameSize] = {};
	UINT nameSize = maxDeviceNameSize;
	if (!GetRawInputDeviceInfo(hidInfo.hDevice, RIDI_DEVICENAME, &rawInputDeviceName, &nameSize))
		return false;

	hidInfo.hidDevice = CreateFileW(rawInputDeviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
	if (hidInfo.hidDevice == INVALID_HANDLE_VALUE)
		return false;

	info.deviceName = "Generic Mouse";
	info.osDeviceName = weave::utf::Utf16toUtf8(rawInputDeviceName);
	info.osProductName = weave::utf::Utf16toUtf8(
		HidD_GetProductString(hidInfo.hidDevice, productString, sizeof(wchar_t) * maxDeviceNameSize) ? productString : L"[Unnamed]"
	);

	info.hardwareId.uniqueId = (uint64_t)hidInfo.hDevice;

	return true;
}

int RawInputMouseFeed::MouseDeviceData::FeedSyncInput(HRAWINPUT lparam, POINT point, bool onlyMouseUp, InputPipeline& inputPipeline) {
	UINT rawSize = bufferSize;

	//Read the raw data
	GetRawInputData((HRAWINPUT)lparam, RID_INPUT, rawbuff, &rawSize, sizeof(RAWINPUTHEADER));
	//Cast it to a RAWINPUT structure
	RAWINPUT* raw = (RAWINPUT*)rawbuff;

	//Up events
	//Check for button pressed
	constexpr auto upButtons = RI_MOUSE_LEFT_BUTTON_UP | RI_MOUSE_MIDDLE_BUTTON_UP | RI_MOUSE_RIGHT_BUTTON_UP | RI_MOUSE_BUTTON_4_UP | RI_MOUSE_BUTTON_5_UP;
	constexpr auto downButtons = RI_MOUSE_WHEEL | RI_MOUSE_LEFT_BUTTON_DOWN | RI_MOUSE_MIDDLE_BUTTON_DOWN | RI_MOUSE_RIGHT_BUTTON_DOWN | RI_MOUSE_BUTTON_4_DOWN | RI_MOUSE_BUTTON_5_DOWN;

	if (raw->data.mouse.usButtonFlags & upButtons) {
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_Left, KeyStateEvent{ VirtualKeyState::Up });
		}
		
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_Middle, KeyStateEvent{ VirtualKeyState::Up });
		}
		
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_Right, KeyStateEvent{ VirtualKeyState::Up });
		}

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_X4, KeyStateEvent{ VirtualKeyState::Up });
		}

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_X5, KeyStateEvent{ VirtualKeyState::Up });
		}
	}
	
	if (onlyMouseUp)
		return raw->data.mouse.usButtonFlags & upButtons ? 1 : 0;

	//Down events
	if (raw->data.mouse.usButtonFlags & downButtons) {
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_Left, KeyStateEvent{ VirtualKeyState::Down });
		}

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_Middle, KeyStateEvent{ VirtualKeyState::Down });
		}

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_Right, KeyStateEvent{ VirtualKeyState::Down });
		}

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_X4, KeyStateEvent{ VirtualKeyState::Down });
		}

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_X5, KeyStateEvent{ VirtualKeyState::Down });
		}

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
			auto x = static_cast<short>(raw->data.mouse.usButtonData);
			inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Click_Wheel,
				CursorDeltaEvent{ Vector3{ static_cast<float>(x), 0.0f, 0.0f } });
		}
	}

	//Check for movement
	if (raw->data.mouse.lLastX || raw->data.mouse.lLastY) {
		//lLastX and lLastY members of the mouse structure represent the movement of the mouse (not the position), pack and send the position and the movement
		inputPipeline.FeedEvent(info.virtualDevice, VirtualKey::Cursor,
			CursorPosDeltaEvent{ Vector3{ static_cast<float>(point.x), static_cast<float>(point.y), 0.0f },
								 Vector3{ static_cast<float>(raw->data.mouse.lLastX), static_cast<float>(raw->data.mouse.lLastY), 0.0f } });
	}
	
	return raw->data.mouse.usButtonFlags || raw->data.mouse.lLastX || raw->data.mouse.lLastY ? 1 : 0;
}

bool RawInputMouseFeed::MouseDeviceData::IsLinked(VirtualDevice vDev) const {
	return info.virtualDevice == vDev;
}

void RawInputMouseFeed::MouseDeviceData::Link(VirtualDevice vDev) {
	info.virtualDevice = vDev;
}
void  RawInputMouseFeed::MouseDeviceData::Unlink() {
	info.virtualDevice = VirtualDevice::None;
}

//--------

std::vector<RawInputMouseFeed::MouseId> RawInputMouseFeed::GetLinkedMouseIds(VirtualDevice vDev) const {
	std::vector<MouseId> links;
	for (auto const& pad : mouseDevices) {
		if (pad.IsLinked(vDev)) {
			links.emplace_back(pad.info.hardwareId);
		}
	}

	return links;
}

RawInputMouseFeed::MouseInfo RawInputMouseFeed::GetMouseInfo(MouseId gamepadId) const {
	for (auto const& pad : mouseDevices) {
		if (pad.info.hardwareId == gamepadId) {
			return pad.info;
		}
	}

	return {};
}

std::vector<RawInputMouseFeed::MouseInfo> RawInputMouseFeed::GetEnumeratedMiceInfo() const {
	std::vector<MouseInfo> info;
	for (auto const& pad : mouseDevices) {
		info.emplace_back(pad.info);
	}

	return info;
}

void RawInputMouseFeed::SetOsCursorPosition(float x, float y) {
	RECT screen;
	GetClientRect(hwndEnumerator, &screen);

	POINT point;
	point.x = screen.left + LONG((screen.right - screen.left)*x);
	point.y = screen.top + LONG((screen.bottom - screen.top)*(1.0f - y));
	POINT screenpoint = point;
	ClientToScreen(hwndEnumerator, &screenpoint);
	SetCursorPos(screenpoint.x, screenpoint.y);
}

void RawInputMouseFeed::SetWindowAsConstrainArea() {
	constrainIsWindowArea = true;
	GetClientRect(hwndEnumerator, &constrains);

	TrapCursor(trapCursor);
}

//Limits the mouse to a rectangle
void RawInputMouseFeed::SetConstrainArea(long x, long y, long width, long height) {
	constrains.left = x;
	constrains.top = y;
	constrains.right = x + width;
	constrains.bottom = y + height;
	constrainIsWindowArea = false;

	TrapCursor(trapCursor);
}

//Toggles the area/window constrain on and off
void RawInputMouseFeed::ConstrainToAreaToggle(bool constrain) {
	constrainToArea = constrain;
}

void RawInputMouseFeed::TrapCursor(bool trap) {
	trapCursor = trap;

	//Use windows' cliping system to constrain the cursor
	if(trapCursor) {
		POINT point{};
		ClientToScreen(hwndEnumerator, &point);
		
		RECT clip;
		clip.left = point.x;
		clip.top = point.y;

		point.x = constrains.right;
		point.y = constrains.bottom;
		ClientToScreen(hwndEnumerator, &point);
		
		clip.right = point.x;
		clip.bottom = point.y;

		ClipCursor(&clip);
	}
	else {
		ClipCursor(NULL);
	}
}

//Sets the cursor to be hidden while the window has focus. The cursor will be shown again when the window focus is lost
void RawInputMouseFeed::HideCursor(bool hide) {
	hideCursor = hide;
}

size_t RawInputMouseFeed::EnumerateMice(HWND handle) {
	
	if (handle) {
		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = 1;
		Rid[0].usUsage = 2; //Mouse HID
		Rid[0].dwFlags = RIDEV_DEVNOTIFY;
		Rid[0].hwndTarget = handle;
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

		hwndEnumerator = handle;
	}

	//Get the amount of RawHIDs
	UINT number = 0;
	GetRawInputDeviceList(nullptr, &number, sizeof(RAWINPUTDEVICELIST));

	//Query the information for all devices
	std::vector<RAWINPUTDEVICELIST> devs;
	devs.resize(number);
	GetRawInputDeviceList(devs.data(), &number, sizeof(RAWINPUTDEVICELIST));
	for (auto dev : devs) {
		//Mice only
		if (dev.dwType == RIM_TYPEMOUSE) {
			InitializeMouse(dev.hDevice);
		}
	}

	if (constrainIsWindowArea) {
		SetWindowAsConstrainArea();
	}

	return mouseDevices.size();
}

bool RawInputMouseFeed::InitializeMouse(HANDLE hDevice) {
	if (!hDevice)
		return false;
	
	//Look for the mouse on the current list of mice
	for(auto const &mouse : mouseDevices) {
		if(mouse.hidInfo.hDevice == hDevice) {
			return true;
		}
	}

	//Create a new mouse structure 
	MouseDeviceData mouse;
	if (!mouse.Initialize(hDevice))
		return false;
	
	//Find a free virtual device to assign
	for (auto vDev = static_cast<unsigned short>(VirtualDevice::_First_Mouse); vDev <= static_cast<unsigned short>(VirtualDevice::_Last_Mouse); ++vDev) {
		//Look for the virtual device on the registered device list
		bool found = false;
		for (auto const& mdev : mouseDevices) {
			if (mdev.IsLinked(static_cast<VirtualDevice>(vDev))) {
				found = true;
				break;
			}
		}

		//If the device was not found, we've found a free device to assign!
		if (!found) {
			mouse.Link(static_cast<VirtualDevice>(vDev));
			break;
		}
	}

	//With the mouse structure created, add it to the mouse vector
	deviceEvents.emplace_back(mouse.info.virtualDevice, DeviceState::Idle);
	mouseDevices.emplace_back(std::move(mouse));

	return true;
}

void RawInputMouseFeed::LinkMouse(MouseId gamepadId, VirtualDevice vDev) {
	if (auto* pad = FindMouse(gamepadId); pad) {
		deviceEvents.emplace_back(vDev, DeviceState::Idle);
		pad->Link(vDev);
	}
}

void RawInputMouseFeed::UnlinkMouse(MouseId gamepadId) {
	if (auto* pad = FindMouse(gamepadId); pad) {
		deviceEvents.emplace_back(pad->info.virtualDevice, DeviceState::Disconnected);
		pad->Unlink();
	}
}

void RawInputMouseFeed::UnlinkVirtualDevice(VirtualDevice vDev) {
	auto ids = GetLinkedMouseIds(vDev);
	for (auto id : ids) {
		UnlinkMouse(id);
	}
}

//Releases all mouse data and virtual device bindings from mice that are no longer present in the system
size_t RawInputMouseFeed::ReleaseDeadMice() {
	std::erase_if(mouseDevices, [this](auto const& dev) {
		UINT dataSize = 0;
		UINT queryResult = GetRawInputDeviceInfo(dev.hidInfo.hDevice, RIDI_DEVICENAME, nullptr, &dataSize);
		bool isDead = (queryResult == static_cast<UINT>(-1));
		if (isDead) {
			deviceEvents.emplace_back(dev.info.virtualDevice, DeviceState::Disconnected);
		}
		return isDead;
	});

	return mouseDevices.size();
}

//Releases all mouse data and virtual device bindings, reverting to the default behaviour of accepting all mouse data as a single mouse device.
void RawInputMouseFeed::ReleaseAllMice() {
	for (auto const& dev : mouseDevices) {
		deviceEvents.emplace_back(dev.info.virtualDevice, DeviceState::Disconnected);
	}

	mouseDevices.clear();
}

//Feeds synchronous input events to the supplied mapper. wparam and lparam are to be taken directly from the Win32 message loop.
int RawInputMouseFeed::FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline) {
	
	FlushDeviceEvents(inputPipeline);
	
	switch(msg) {
	case WM_ACTIVATE:
		{
			//Window has been deactivated. Release any held keys.
			if(LOWORD(wparam) == WA_INACTIVE) {
				for (auto const& mouse : mouseDevices) {
					inputPipeline.FeedEvent(mouse.info.virtualDevice, VirtualKey::None,
						DeviceStateEvent{ DeviceState::FocusLost });
				}
				windowFocus = false;
				//if(hideCursor)
					//ShowCursor(true);
			}
			else if (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE) {
				for (auto const& mouse : mouseDevices) {
					inputPipeline.FeedEvent(mouse.info.virtualDevice, VirtualKey::None,
						DeviceStateEvent{ DeviceState::FocusGained });
				}
				windowFocus = true;
				//if(hideCursor)
				//	ShowCursor(false);
			}
			//Don't tell that the message has been processed. Someone outside may want it too!
			return 0;
		}
	case WM_INPUT_DEVICE_CHANGE:
	{
		if(wparam == GIDC_ARRIVAL) {
			EnumerateMice(nullptr);
		}
		else if(wparam == GIDC_REMOVAL) {
			ReleaseDeadMice();
		}
		//Don't confirm the message has been processed
		return 0;
	}
	case WM_SIZE:
	{
		//The window's size has changed, which concerns us if we are limiting mouse input
		if(hwndEnumerator && constrainIsWindowArea)
			GetClientRect(hwndEnumerator, &constrains);
		//Don't confirm the message has been processed
		return 0;
	}
	case WM_SETCURSOR:
	{
		if(hideCursor) {
			//Check if the mouse is on the window bar or the borders. 
			//This bar is considered inside the window so the cursor will remain hidden, but we actually want the cursor to be visible in this area
			//The area will still be detected as negative when getting the client coordinates or outside the actual window rect
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(hwndEnumerator, &point);
			
			RECT winRect;
			GetClientRect(hwndEnumerator, &winRect);

			if(point.y >= 0 && point.x >= 0 && point.x <= winRect.right && point.y <= winRect.bottom) {
				SetCursor(NULL);
				return 1;
			}
		}
		return 0;
	}
	case WM_INPUT:
		{
			//Read the header of the raw input data only
			RAWINPUTHEADER rawHeader;
			UINT headerSize = sizeof(RAWINPUTHEADER);
			GetRawInputData((HRAWINPUT)lparam, RID_HEADER, &rawHeader, &headerSize, sizeof(RAWINPUTHEADER));
			//Check if the data comes from a mouse, which is what we registered
			if (rawHeader.dwType == RIM_TYPEMOUSE) {
				//Query the current cursor position (the raw mouse data only provides movement)
				POINT point;
				GetCursorPos(&point);
				ScreenToClient(hwndEnumerator, &point);
				//If hwnd is set, make sure the coordinates of the cursor are within the constrains and ignore any input from outside
				bool feedOnlyMouseUp = false;
				if (constrainToArea &&
					(point.x < constrains.left || point.x > constrains.right ||
					point.y < constrains.top || point.y > constrains.bottom)) {
					//See if we want the cursor trapped
					if (trapCursor) {
						//Set a valid position for the cursor
						point.x = std::max<LONG>(std::min<LONG>(point.x, constrains.right), constrains.left);
						point.y = std::max<LONG>(std::min<LONG>(point.y, constrains.bottom), constrains.top);
						POINT screenpoint = point;
						ClientToScreen(hwndEnumerator, &screenpoint);

						SetCursorPos(screenpoint.x, screenpoint.y);

						//Reactivate windows' cliping
						TrapCursor(true);
					}
					else {
						//Only check for mouse ups, to allow releasing keys
						feedOnlyMouseUp = true;
					}
				}

				for (auto& mouseDevice : mouseDevices) {
					if (mouseDevice.hidInfo.hDevice == rawHeader.hDevice) {
						return mouseDevice.FeedSyncInput((HRAWINPUT)lparam, point, feedOnlyMouseUp, inputPipeline);
					}
				}
			}
			return 0;
		}
	
	default:
		return 0;
	}
}

RawInputMouseFeed::MouseDeviceData* RawInputMouseFeed::FindMouse(MouseId mouseId) {
	for (auto& pad : mouseDevices) {
		if (pad.info.hardwareId == mouseId) {
			return &pad;
		}
	}

	return nullptr;
}

void RawInputMouseFeed::FlushDeviceEvents(InputPipeline& inputPipeline) {
	for (auto& event : deviceEvents) {
		inputPipeline.FeedEvent(event.first, VirtualKey::None, DeviceStateEvent{ event.second });
	}
	deviceEvents.clear();
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
