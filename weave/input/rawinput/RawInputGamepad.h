/*
Weave Win32 Raw Gamepad Feeder
RawInputGamepad.h

Implements WM_INPUT data parsing and event generation for Gamepads and Joysticks.
Adding library "hid.lib" is required.

RawInput gamepad support has been developed based on the work found here:
http://www.codeproject.com/Articles/185522/Using-the-Raw-Input-API-to-Process-Joystick-Input?msg=3866406

As well as the documentation for RawInput and HID:
http://msdn.microsoft.com/en-us/library/windows/desktop/ff468895%28v=vs.85%29.aspx
http://msdn.microsoft.com/en-us/library/windows/hardware/ff539956%28v=vs.85%29.aspx

Usage codes for USB devices can be found here:
https://usb.org/sites/default/files/hut1_3_0.pdf

*/
#pragma once

#include "weave/input/system/VirtualKeys.h"
#include "weave/input/system/VirtualDevices.h"
#include "weave/input/system/InputPipeline.h"
#include "weave/input/system/GamepadLayouts.h"

#ifndef NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif // !NOMINMAX

#include <windows.h>
#include <hidsdi.h>
#include <vector>
#include <unordered_map>

#ifdef _MSC_VER
#pragma comment(lib, "hid.lib")
#endif

namespace weave::input {

class RawInputGamepad  {
public:
	using GamepadLayout = weave::input::gamepad::GamepadLayout;

	struct GamepadId {
		uint64_t uniqueId{};

		bool operator==(GamepadId other) const { return uniqueId == other.uniqueId; }
	};

	struct GamepadInfo {
		GamepadId hardwareId; //Unique hardware id
		GamepadLayout detectedLayout{}; //System detected layout based on internal heuristics
		GamepadLayout currentLayout{}; //Assigned layout
		std::string deviceName; //Weave system device name (not OS provided)
		std::string osDeviceName; //Device name provided by the OS
		std::string osProductName; //Product name provided by the OS
		VirtualDevice virtualDevice; //Virtual device linked
	};

protected:
	struct HidInfo {
		HANDLE hDevice{}; //hDevice handle taken from the RawInput reports. Indentifies the device through RawInput
		HANDLE hidDevice{}; //Handle created with CreateFile for the HID device
		PHIDP_PREPARSED_DATA preparsedData{}; //Allocated block with the preparsed data information, used by the Hid library methods. This is not used directly
		HIDP_CAPS generalCaps{}; //Capabilities of the device
		std::vector<HIDP_BUTTON_CAPS> buttonCaps; //One or multiple structures detailing the button capabilities of the device
		std::vector<HIDP_VALUE_CAPS> valueCaps; //Structures detailing the ranged capabilities of the device (this includes axes, pressure controls, etc. Anything with a value associated)
		std::vector<HIDP_VALUE_CAPS> outputValueCaps; //Structures detailing the writable ranged capabilities of the device (rumble, lights, etc)

		HidInfo() = default;
		HidInfo(HidInfo const&) = delete;
		HidInfo(HidInfo&&);
		HidInfo& operator=(HidInfo const&) = delete;
		HidInfo& operator=(HidInfo&&);
		~HidInfo();
	};

	//The data structure used for each gamepad device
	struct GamepadDeviceData {
		//Basic information
		GamepadInfo info; 
		//OS information
		HidInfo hidInfo;

		//Manageable data is extracted from the data structures and held apart for easier reference
		//Button data
		ULONG numButtons{}; //The number of buttons the device reports
		USHORT firstButtonIndex{}; //Same information as in buttonCaps->Range.DataIndexMin. It holds the offset to the first button index on the data
		USHORT lastButtonIndex{}; //Same information as in buttonCaps->Range.DataIndexMax. It holds the offset to the first button index on the data

		//Value data is mapped to specific data we look for (like joystick axes or the hat switch), by keepign a record of which item in the valueCaps structure holds it. 
		ULONG numValues{}; //The number of values the device provides
		PHIDP_VALUE_CAPS leftX{}, leftY{}, leftZ{}, rightX{}, rightY{}, rightZ{}; //Left and right stick axes
		PHIDP_VALUE_CAPS hatSwitch{}; //The hat switch cross pad
		PHIDP_VALUE_CAPS dpadUp{}, dpadDown{}, dpadRight{}, dpadLeft{}; //Directional pad buttons (legacy)
		PHIDP_VALUE_CAPS *pressureData{}; //Pressure data for each button. The size of this is numButtons and is ordered to mimic the button order
		
		//Runtime feed reusable data
		std::vector<std::byte> rawbuff; //Byte buffer for the reports supplied by RawInput on each WM_INPUT message. This is sized accordingly to the report size for each device
		 								//The size of the report buffer can be found in the generalCaps structure's InputReportByteLength member added to sizeof(RAWINPUT) + 1

		std::vector<HIDP_DATA> hidReportData; //This array of structures is where the parsed report will be placed by HidP_GetData

		//These two buffers hold the information returned by HidP_GetData in a predictable order of values. GetData returns the data relevant to values and buttons, but does not include the state
		//of buttons that are down; these two buffers will include those states as well, so the positions of the data in the buffer coincide one to one with the data indices on the caps.
		//The size of these buffers is the size reported by HidP_MaxDataListLength, which should be the same as numButtons + numValues.
		std::vector<ULONG> reportMap; //Holds the data of the current report. This is wiped each time a new report arrives
		std::vector<ULONG> reportMapPrev; //Holds the data of the previous report

		//Normalization and deadzone values for axes
		Vector3 leftNorm, rightNorm;
		float leftDeadZone{}, rightDeadZone{};
		float leftDeadZoneSqr{}, rightDeadZoneSqr{}; //Kept for convenience
		
		//Cached values for digital emulation of analog sticks. This is used to send binary state directional keys emulated through the analog gamepad
		Vector2 leftCap; //Tracker values to hold previous information from the pad
		Vector2 rightCap;
		IVector2 leftDir; //Tracker values holding the last digital position (-1, 0, 1)
		IVector2 rightDir;

		//X360 digital trigger emulation
		//+1 = LT, -1 = RT, 0 = neutral position. The value must go back to neutral before LT or RT can be pressed again. 
		//This implies that RT and LT cannot be pressed at the same time
		int xBoxTrigger{};

		//Current LED color and flashing state
		Vector3 ledColor;
		float stayOnFlash{}, stayOffFlash{};
		
		//Initializes the data structures given a device handle. Checks if the device is indeed a gamepad, builds the data structures and maps the data to the required sources before returning.
		bool Initialize(HANDLE hDevice);

		//Automatically selects a layout based on known values for the device
		void AutoLayout();

		//Processes raw input data and generates input events from it
		int FeedSyncInput(HRAWINPUT lparam, InputPipeline &inputPipeline);

		//Helper function that emulates digital style response with analog sticks and generates the required binary events
		//Stick: 0 -> Left, 1 -> Right
		void EmulateDigitalPad(int stick, float posx, float posy, float deltax, float deltay, InputPipeline& inputPipeline);

		//Emulates digital interactions for the X360 gamepad.
		//This gamepad reports both LT and RT analog triggers on the same axis, which makes it impossible to get an accurte reading. Emulation is implemented to make the data useful.
		void XBoxTriggerEmulation(float posx, float deltax, InputPipeline& inputPipeline);

		//Creates a write packet sent to the gamepad that makes it rumble, if compatible
		//@heavy, weak: Intensity values for low freq and high freq rumble, normalized 0.0 .. 1.0
		//@left, right: Special rumble for XBox pads on left and right triggers. Not used on PS4
		//@time: Amount of time to rumble, normalized
		//@delay: Delay between rumbles, normalized
		//@repeat: Amount of repetitions of the rumble pattern. Not normalized.
		void Rumble(float heavy, float weak, float left, float right, float time, float delay, uint8_t repeat);
		//Creates a write packet sent to the gamepad that makes it change LED color, if compatible
		//RGB intensity values are normalized 0.0 .. 1.0
		void Light(Vector3 const &rgb);
		//Creates a write packet sent to the gamepad that makes it flash the LED, if compatible
		//@stayOn: Normalized time (0.0 .. 1.0) for the light to be stay on when flashing
		//@stayOff: Normalized time (0.0 .. 1.0) for the light to be stay off when flashing
		void Flash(float stayOn, float stayOff);

		//Creates the full rumble / light / flash package and sends it to the gamepad
		void WriteRumblePkg(float heavy, float weak, float left, float right, float time, float delay, uint8_t repeat, Vector3 const &rgb, float stayOn, float stayOff);

		void SetDeadzones(float leftDeadzone, float rightDeadzone);
		void SetLayout(GamepadLayout layout);

		bool IsLinked(VirtualDevice vDev) const;
		void Link(VirtualDevice vDev);
		void Unlink();
	};

	std::vector<GamepadDeviceData> gamepadDevices;
	std::vector<std::pair<VirtualDevice, DeviceState>> deviceEvents;
	HWND hwndEnumerator{};

public:

	//Returns the unique hardware gamepad identifiers linked to a virtual device
	std::vector<GamepadId> GetLinkedGamepadIds(VirtualDevice vDev) const;

	GamepadInfo GetGamepadInfo(GamepadId gamepadId) const;
	std::vector<GamepadInfo> GetEnumeratedGamepadInfo() const;
	size_t GetEnumeratedGamepadCount() const { return gamepadDevices.size(); }

	//Initializes all attached devices by enumerating them and probing the capabilities of each device. 
	//Gamepads are automatically linked to an unused sequential virtual device
	//Returns the amount of devices successfully enumerated
	size_t EnumerateGamepads(HWND hwnd);

	void LinkGamepad(GamepadId gamepadId, VirtualDevice vDev);
	void UnlinkGamepad(GamepadId gamepadId);
	void UnlinkVirtualDevice(VirtualDevice vDev);

	void SetDeadzones(float leftDeadzone, float rightDeadzone);
	void SetDeadzones(GamepadId gamepadId, float leftDeadzone, float rightDeadzone);
	void SetLayout(GamepadId gamepadId, GamepadLayout layout);

	//Releases the data held for gamepads that are no longer connected to the system, cleaning up the available gamepad slots
	size_t ReleaseDeadGamepads();

	//Releases all allocated gamepads regardless of their status as connected
	void ReleaseAllGamepads();

	//Feeds synchronous input events to the system pipeline. wparam and lparam are to be taken directly from the Win32 message loop.
	int FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline);

private:
	//Initializes a gamepad's structure given its device handle
	bool InitializeGamepad(HANDLE hDevice);

	GamepadDeviceData* FindGamepad(GamepadId gamepadId);

	void FlushDeviceEvents(InputPipeline& inputPipeline);
};
	

}
