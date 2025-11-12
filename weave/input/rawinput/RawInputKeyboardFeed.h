/*
Weave Win32 Raw Input Keyboard Feeder
RawInputKeyboardFeed.h

Implements RawInput through the WM_INPUT message instead of WM_KEYDOWN/WM_KEYUP to report keyboard state changes.
Provides support for multiple keyboard sources.
Adding library "hid.lib" is required.

*/
#pragma once

#include "weave/input/system/InputPipeline.h"
#include "weave/input/win32/Win32VirtualKeys.h"
#include <vector>
#include <hidsdi.h>
#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "hid.lib")
#endif

namespace weave::input {

class RawInputKeyboardFeed {
public:
	struct KeyboardId {
		uint64_t uniqueId{};

		bool operator==(KeyboardId other) const { return uniqueId == other.uniqueId; }
	};

	struct KeyboardInfo {
		KeyboardId hardwareId; //Unique hardware id
		std::string deviceName; //Weave system device name (not OS provided)
		std::string osDeviceName; //Device name provided by the OS
		std::string osProductName; //Product name provided by the OS
		VirtualDevice virtualDevice; //Virtual device linked
	};

protected:
	struct HidInfo {
		HANDLE hDevice{}; //hDevice handle taken from the RawInput reports. Indentifies the device through RawInput
		HANDLE hidDevice{}; //Handle created with CreateFile for the HID device

		HidInfo() = default;
		HidInfo(HidInfo const&) = delete;
		HidInfo(HidInfo&&);
		HidInfo& operator=(HidInfo const&) = delete;
		HidInfo& operator=(HidInfo&&);
		~HidInfo();
	};

	struct KeyboardDeviceData {
		KeyboardInfo info;
		HidInfo hidInfo;

		//Raw input is written to a buffer with enough size to hold the raw data.
		static const constexpr UINT bufferSize = 80;
		BYTE rawbuff[bufferSize];

		bool Initialize(HANDLE hDevice);

		int FeedSyncInput(HRAWINPUT lparam, InputPipeline& inputPipeline);

		bool IsLinked(VirtualDevice vDev) const;
		void Link(VirtualDevice vDev);
		void Unlink();
	};

	//The list of keyboard devices present in the system. If the list is empty, all keyboard input is considered to be a single keyboard
	std::vector<KeyboardDeviceData> keyboardDevices;
	std::vector<std::pair<VirtualDevice, DeviceState>> deviceEvents;
	HWND hwndEnumerator{};

public:

	//Returns the unique hardware keyboard identifiers linked to a virtual device
	std::vector<KeyboardId> GetLinkedKeyboardIds(VirtualDevice vDev) const;

	KeyboardInfo GetKeyboardInfo(KeyboardId gamepadId) const;
	std::vector<KeyboardInfo> GetEnumeratedKeyboardInfo() const;
	size_t GetEnumeratedKeyboardCount() const { return keyboardDevices.size(); }

	//Initializes all individual keyboard devices attached to the system and assigns a virtual keyboard device to each of them. 
	size_t EnumerateKeyboards(HWND hwnd = nullptr);

	void LinkKeyboard(KeyboardId gamepadId, VirtualDevice vDev);
	void UnlinkKeyboard(KeyboardId gamepadId);
	void UnlinkVirtualDevice(VirtualDevice vDev);

	//Releases all keyboard data and virtual device bindings from keyboards that are no longer present in the system
	size_t ReleaseDeadKeyboards();

	//Feeds synchronous input events to the system mapper. wparam and lparam are to be taken directly from the Win32 message loop.
	int FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline);

private:
	//Initializes a keyboard's structure given its device handle
	bool InitializeKeyboard(HANDLE hDevice);

	KeyboardDeviceData* FindKeyboard(KeyboardId mouseId);

	void FlushDeviceEvents(InputPipeline& inputPipeline);
};
	

}
