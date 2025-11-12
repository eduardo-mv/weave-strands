/*
Weave Win32 Raw Input Mouse Feeder
RawInputMouseFeed.h

Implements RawInput through the WM_INPUT message for high resolution mice control.
Provides support for multiple mice sources.
Adding library "hid.lib" is required.

*/
#pragma once

#include "weave/input/system/InputPipeline.h"
#include "weave/input/win32/Win32VirtualKeys.h"
#include <hidsdi.h>
#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "hid.lib")
#endif

namespace weave::input {

class RawInputMouseFeed {
public:
	struct MouseId {
		uint64_t uniqueId{};

		bool operator==(MouseId other) const { return uniqueId == other.uniqueId; }
	};

	struct MouseInfo {
		MouseId hardwareId; //Unique hardware id
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

	struct MouseDeviceData {
		MouseInfo info;
		HidInfo hidInfo;

		//Raw input is written to a buffer with enough size to hold the raw data.
		static const constexpr UINT bufferSize = 80;
		BYTE rawbuff[bufferSize];

		bool Initialize(HANDLE hDevice);

		int FeedSyncInput(HRAWINPUT lparam, POINT point, bool onlyMouseUp, InputPipeline& inputPipeline);

		bool IsLinked(VirtualDevice vDev) const;
		void Link(VirtualDevice vDev);
		void Unlink();
	};

	std::vector<MouseDeviceData> mouseDevices;
	std::vector<std::pair<VirtualDevice, DeviceState>> deviceEvents;
	HWND hwndEnumerator{};
	
	//Flags mouse functionality to be constrained to the supplied area
	RECT constrains{};
	bool constrainToArea = true;
	bool constrainIsWindowArea = true;
	//Sets the mouse cursor to be trapped inside the window
	bool trapCursor = false;
	//Sets the cursor to be hidden while the window has focus
	bool hideCursor = false;
	//Keeps track of the focus of the window so that mouse trapping can work
	bool windowFocus = false;

public:
	//Returns the unique hardware mouse identifiers linked to a virtual device
	std::vector<MouseId> GetLinkedMouseIds(VirtualDevice vDev) const;

	MouseInfo GetMouseInfo(MouseId gamepadId) const;
	std::vector<MouseInfo> GetEnumeratedMiceInfo() const;
	size_t GetEnumeratedMiceCount() const { return mouseDevices.size(); }

	//Sets the hardware/OS controlled cursor's position in normalized coordinates
	void SetOsCursorPosition(float x, float y);

	void SetWindowAsConstrainArea();
	void SetConstrainArea(long x, long y, long width, long height);
	void ConstrainToAreaToggle(bool constrain);
	void TrapCursor(bool trap);
	void HideCursor(bool hide);

	//Initializes all individual mouse devices attached to the system and assigns a virtual mouse device to each of them.
	size_t EnumerateMice(HWND hwnd);

	void LinkMouse(MouseId gamepadId, VirtualDevice vDev);
	void UnlinkMouse(MouseId gamepadId);
	void UnlinkVirtualDevice(VirtualDevice vDev);

	//Releases all mouse data and virtual device bindings from mice that are no longer present in the system
	size_t ReleaseDeadMice();

	//Releases all mouse data and virtual device bindings
	void ReleaseAllMice();

	//Feeds synchronous input events to the system pipeline. wparam and lparam are to be taken directly from the Win32 message loop.
	int FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline);

private:
	//Initializes a mouse's structure given its device handle
	bool InitializeMouse(HANDLE hDevice);

	MouseDeviceData* FindMouse(MouseId mouseId);

	void FlushDeviceEvents(InputPipeline& inputPipeline);
};
	

}
