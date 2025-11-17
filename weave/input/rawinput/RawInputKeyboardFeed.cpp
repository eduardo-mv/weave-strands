#include "RawInputKeyboardFeed.h"
#include <cassert>

#include "weave/system/utf/Utf.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated wstring_convert in C++17
#endif

using namespace weave::input;

RawInputKeyboardFeed::HidInfo& RawInputKeyboardFeed::HidInfo::operator=(HidInfo&& other)
{
	std::swap(hDevice, other.hDevice);
	std::swap(hidDevice, other.hidDevice);
	return *this;
}

RawInputKeyboardFeed::HidInfo::HidInfo(HidInfo&& other)
{
	std::swap(hDevice, other.hDevice);
	std::swap(hidDevice, other.hidDevice);
}


RawInputKeyboardFeed::HidInfo::~HidInfo() {
	if (hidDevice) {
		CloseHandle(hidDevice);
	}
}

bool RawInputKeyboardFeed::KeyboardDeviceData::Initialize(HANDLE h) {

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

	info.deviceName = "Generic Keyboard";
	info.osDeviceName = weave::utf::Utf16toUtf8(rawInputDeviceName);
	info.osProductName = weave::utf::Utf16toUtf8(
		HidD_GetProductString(hidInfo.hidDevice, productString, sizeof(wchar_t) * maxDeviceNameSize) ? productString : L"[Unnamed]"
	);

	info.hardwareId.uniqueId = (uint64_t)hidInfo.hDevice;

	return true;
}

int RawInputKeyboardFeed::KeyboardDeviceData::FeedSyncInput(HRAWINPUT lparam, InputPipeline& inputPipeline) {
	//This will hold the actual buffer size that GetRawInputData returns
	UINT rawSize = bufferSize;

	//Read the raw data
	GetRawInputData((HRAWINPUT)lparam, RID_INPUT, rawbuff, &rawSize, sizeof(RAWINPUTHEADER));
	//Cast it to a RAWINPUT structure
	RAWINPUT* raw = (RAWINPUT*)rawbuff;

	//Map the key correctly from VK_ values to VirtualKeys
	VirtualKey vk = input::Win32VkToVirtualKey(raw->data.keyboard.VKey);
	//Run checks to determine left or right ctrl, shift and alt. In the case of shift and ctrl, both the generic key and the specific ones are sent
	switch (vk) {
	case VirtualKey::Control:
		inputPipeline.FeedEvent(info.virtualDevice,
			((raw->data.keyboard.Flags & RI_KEY_E0) ? VirtualKey::Rcontrol : VirtualKey::Lcontrol),
			KeyStateEvent{ (raw->data.keyboard.Flags & RI_KEY_BREAK) ? VirtualKeyState::Up : VirtualKeyState::Down });
		break;
	case VirtualKey::Shift:
		inputPipeline.FeedEvent(info.virtualDevice,
			((raw->data.keyboard.MakeCode == 0x36) ? VirtualKey::Rshift : VirtualKey::Lshift),
			KeyStateEvent{ (raw->data.keyboard.Flags & RI_KEY_BREAK) ? VirtualKeyState::Up : VirtualKeyState::Down });
		break;
	case VirtualKey::Alt:
		vk = ((raw->data.keyboard.Flags & RI_KEY_E0) ? VirtualKey::Altgr : VirtualKey::Alt);
		break;
	case VirtualKey::None:
		return 0;
	default:
		break;
	}

	inputPipeline.FeedEvent(info.virtualDevice, vk,
		KeyStateEvent{ (raw->data.keyboard.Flags & RI_KEY_BREAK) ? VirtualKeyState::Up : VirtualKeyState::Down });

	return 1;
}

bool RawInputKeyboardFeed::KeyboardDeviceData::IsLinked(VirtualDevice vDev) const {
	return info.virtualDevice == vDev;
}

void RawInputKeyboardFeed::KeyboardDeviceData::Link(VirtualDevice vDev) {
	info.virtualDevice = vDev;
}
void  RawInputKeyboardFeed::KeyboardDeviceData::Unlink() {
	info.virtualDevice = VirtualDevice::None;
}

//--------

std::vector<RawInputKeyboardFeed::KeyboardId> RawInputKeyboardFeed::GetLinkedKeyboardIds(VirtualDevice vDev) const {
	std::vector<KeyboardId> links;
	for (auto const& pad : keyboardDevices) {
		if (pad.IsLinked(vDev)) {
			links.emplace_back(pad.info.hardwareId);
		}
	}

	return links;
}

RawInputKeyboardFeed::KeyboardInfo RawInputKeyboardFeed::GetKeyboardInfo(KeyboardId gamepadId) const {
	for (auto const& pad : keyboardDevices) {
		if (pad.info.hardwareId == gamepadId) {
			return pad.info;
		}
	}

	return {};
}

std::vector<RawInputKeyboardFeed::KeyboardInfo> RawInputKeyboardFeed::GetEnumeratedKeyboardInfo() const {
	std::vector<KeyboardInfo> info;
	for (auto const& pad : keyboardDevices) {
		info.emplace_back(pad.info);
	}

	return info;
}

size_t RawInputKeyboardFeed::EnumerateKeyboards(HWND hwnd) {
	//Register the input device notifications to this window so we can be notified when a device is connected or disconnected
	if(hwnd) {
		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = 1;
		Rid[0].usUsage = 6; //Keyboard HID
		Rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
		Rid[0].hwndTarget = hwnd;
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

		hwndEnumerator = hwnd;
	}

	//Get the amount of RawHIDs
	UINT number = 0;
	GetRawInputDeviceList(nullptr, &number, sizeof(RAWINPUTDEVICELIST));

	//Query the information for all devices
	std::vector<RAWINPUTDEVICELIST> devs;
	devs.resize(number);
	GetRawInputDeviceList(devs.data(), &number, sizeof(RAWINPUTDEVICELIST));
	for (auto dev : devs) {
		if (dev.dwType == RIM_TYPEKEYBOARD) {
			InitializeKeyboard(dev.hDevice);
		}
	}

	return keyboardDevices.size();
}

//Initializes a keyboard's structure given its device handle
bool RawInputKeyboardFeed::InitializeKeyboard(HANDLE hDevice) {
	//Look for the keyboard on the current list of keyboards
	for(auto const &kb : keyboardDevices) {
		if(kb.hidInfo.hDevice == hDevice)
			return true;
	}

	//Create a new keyboard structure 
	KeyboardDeviceData kbDevice;
	if (!kbDevice.Initialize(hDevice))
		return false;

	//Find a free virtual device to assign
	for(unsigned short vDev = (unsigned short)VirtualDevice::_First_Keyboard; vDev <= (unsigned short)VirtualDevice::_Last_Keyboard; ++vDev) {
		//Look for the virtual device on the registered device list
		bool found = false;
		for(auto const &kb : keyboardDevices) {
			if (kb.IsLinked(static_cast<VirtualDevice>(vDev))) {
				found = true;
				break;
			}
		}
		//If the device was not found, we've found a free device to assign!
		if(!found) {
			kbDevice.Link(static_cast<VirtualDevice>(vDev));
			break;
		}
	}

	//With the keyboard structure created, add it to the keyboard vector and maps
	deviceEvents.emplace_back(kbDevice.info.virtualDevice, DeviceState::Idle);
	keyboardDevices.emplace_back(std::move(kbDevice));

	return true;
}

void RawInputKeyboardFeed::LinkKeyboard(KeyboardId gamepadId, VirtualDevice vDev) {
	if (auto* pad = FindKeyboard(gamepadId); pad) {
		deviceEvents.emplace_back(vDev, DeviceState::Idle);
		pad->Link(vDev);
	}
}

void RawInputKeyboardFeed::UnlinkKeyboard(KeyboardId gamepadId) {
	if (auto* pad = FindKeyboard(gamepadId); pad) {
		deviceEvents.emplace_back(pad->info.virtualDevice, DeviceState::Disconnected);
		pad->Unlink();
	}
}

void RawInputKeyboardFeed::UnlinkVirtualDevice(VirtualDevice vDev) {
	auto ids = GetLinkedKeyboardIds(vDev);
	for (auto id : ids) {
		UnlinkKeyboard(id);
	}
}

//Releases all keyboard data and virtual device bindings from keyboards that are no longer present in the system
size_t RawInputKeyboardFeed::ReleaseDeadKeyboards() {
	std::erase_if(keyboardDevices, [this](auto const& dev) {
		UINT dataSize;
		bool isDead = (GetRawInputDeviceInfo(dev.hidInfo.hDevice, RIDI_DEVICENAME, nullptr, &dataSize) == static_cast<UINT>(-1));
		if (isDead) {
			deviceEvents.emplace_back(dev.info.virtualDevice, DeviceState::Disconnected);
		}
		return isDead;
		});

	return keyboardDevices.size();
}

//Feeds synchronous input events to the supplied mapper. wparam and lparam are to be taken directly from the Win32 message loop.
int RawInputKeyboardFeed::FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline) {
	switch(msg) {
	case WM_ACTIVATE:
		{
			//Window has been deactivated. Release any held keys.
			if(LOWORD(wparam) == WA_INACTIVE) {
				for (auto const& kb : keyboardDevices) {
					inputPipeline.FeedEvent(kb.info.virtualDevice, VirtualKey::None,
						DeviceStateEvent{ DeviceState::FocusLost });
				}
			}
			else if (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE) {
				for (auto const& kb : keyboardDevices) {
					inputPipeline.FeedEvent(kb.info.virtualDevice, VirtualKey::None,
						DeviceStateEvent{ DeviceState::FocusGained });
				}
			}
			//Don't tell that the message has been processed. Someone outside may want it too!
			return 0;
		}
	
	case WM_INPUT_DEVICE_CHANGE:
	{
			if(wparam == GIDC_ARRIVAL) {
				EnumerateKeyboards();
			}
			else if(wparam == GIDC_REMOVAL) {
				ReleaseDeadKeyboards();
			}

			return 0;
	}

	case WM_INPUT:
		{
			//Read the header of the raw input data only
			RAWINPUTHEADER rawHeader;
			UINT headerSize = sizeof(RAWINPUTHEADER);
			GetRawInputData((HRAWINPUT)lparam, RID_HEADER, &rawHeader, &headerSize, sizeof(RAWINPUTHEADER));
			//Check if the data comes from a keyboard, which is what we registered
			if(rawHeader.dwType == RIM_TYPEKEYBOARD) {
				//Check if the keyboards have been enumerated. If so, the keyboard device structure must exist, which will provide the virtual device ID
				for(auto &kbDevice : keyboardDevices) {
					if(kbDevice.hidInfo.hDevice == rawHeader.hDevice) {
						return kbDevice.FeedSyncInput((HRAWINPUT)lparam, inputPipeline);
					}
				}

				//We're getting keyboard input from an unknown device, which we'll map to the first device available
				if (!rawHeader.hDevice && !keyboardDevices.empty()) {
					return keyboardDevices[0].FeedSyncInput((HRAWINPUT)lparam, inputPipeline);
				}
			} 

			return 0;
		}
	
	default:
		return 0;
	}
}

RawInputKeyboardFeed::KeyboardDeviceData* RawInputKeyboardFeed::FindKeyboard(KeyboardId keyboardId) {
	for (auto& pad : keyboardDevices) {
		if (pad.info.hardwareId == keyboardId) {
			return &pad;
		}
	}

	return nullptr;
}

void RawInputKeyboardFeed::FlushDeviceEvents(InputPipeline& inputPipeline) {
	for (auto& event : deviceEvents) {
		inputPipeline.FeedEvent(event.first, VirtualKey::None, DeviceStateEvent{ event.second });
	}
	deviceEvents.clear();
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
