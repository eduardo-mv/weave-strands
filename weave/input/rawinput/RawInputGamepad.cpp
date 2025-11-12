#include "RawInputGamepad.h"
#include <cassert>
#include <iostream>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated wstring_convert in C++17
#endif

//#define _DEBUG_GAMEPAD_ANALOG_OUTPUT 1
//#define _DEBUG_GAMEPAD_DIGITAL_OUTPUT 1
#if defined(_DEBUG_GAMEPAD_ANALOG_OUTPUT) || defined(_DEBUG_GAMEPAD_DIGITAL_OUTPUT)
#include "system/log/WLog.h"
#endif // DEBUG

#include "weave/system/utf/Utf.h"
#include "weave/input/system/GamepadLayouts.h"

using namespace weave::input;
using weave::input::gamepad::kStandardLayouts;
static_assert(static_cast<size_t>(RawInputGamepad::GamepadLayout::_LastLayout) == weave::input::gamepad::kLayoutCount,
	"RawInputGamepad layout enum out of sync with shared layout table");

namespace {
	//Helper functions
	template<typename ...Params>
	void FeedCursorPositionAndDelta(InputPipeline& inputPipeline, std::vector<VirtualDevice> const& devices, Params&&... params) {
		for (auto const& dev : devices) {
			inputPipeline.FeedCursorPositionAndDelta(dev, params...);
		}
	}

	template<typename ...Params>
	void FeedCursorPosition(InputPipeline& inputPipeline, std::vector<VirtualDevice> const& devices, Params&&... params) {
		for (auto const& dev : devices) {
			inputPipeline.FeedCursorPosition(dev, params...);
		}
	}

	template<typename ...Params>
	void FeedKeyEvent(InputPipeline& inputPipeline, std::vector<VirtualDevice> const& devices, Params&&... params) {
		for (auto const& dev : devices) {
			inputPipeline.FeedKeyEvent(dev, params...);
		}
	}
}

RawInputGamepad::HidInfo& RawInputGamepad::HidInfo::operator=(HidInfo&& other)
{
	std::swap(hDevice, other.hDevice);
	std::swap(hidDevice, other.hidDevice);
	std::swap(preparsedData, other.preparsedData);
	std::swap(generalCaps, other.generalCaps);
	std::swap(buttonCaps, other.buttonCaps);
	std::swap(valueCaps, other.valueCaps);
	std::swap(outputValueCaps, other.outputValueCaps);

	return *this;
}

RawInputGamepad::HidInfo::HidInfo(HidInfo&& other)
{
	std::swap(hDevice, other.hDevice);
	std::swap(hidDevice, other.hidDevice);
	std::swap(preparsedData, other.preparsedData);
	std::swap(generalCaps, other.generalCaps);
	std::swap(buttonCaps, other.buttonCaps);
	std::swap(valueCaps, other.valueCaps);
	std::swap(outputValueCaps, other.outputValueCaps);
}


RawInputGamepad::HidInfo::~HidInfo() {
	if (preparsedData) {
		HidD_FreePreparsedData(preparsedData);
	}
	if (hidDevice) {
		CloseHandle(hidDevice);
	}
}

//Initializes the data structures given a device handle. Checks if the device is indeed a gamepad, builds the data structures and maps the data to the required sources before returning.
bool RawInputGamepad::GamepadDeviceData::Initialize(HANDLE h) {
	//A structure for the information data of the device, used to check if the usage page and type are correct
	RID_DEVICE_INFO devInfo;
	UINT devInfoSize = sizeof(devInfo);
	devInfo.cbSize = devInfoSize;

	//Get the general device information where we can find the usage page and and usage type
	GetRawInputDeviceInfo(h, RIDI_DEVICEINFO, &devInfo, &devInfoSize);
	if(!(devInfo.hid.usUsagePage == 1 && (devInfo.hid.usUsage == 4 || devInfo.hid.usUsage == 5))) {
		return false; //Not a joystic or gamepad
	}
	
	//We have found a gamepad pointed by hDevice. We proceed to build the information required to parse the data packets sent by the device
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
	if(hidInfo.hidDevice == INVALID_HANDLE_VALUE)
		return false;
	
		//TODO: study this for Joycon detection
		/*
		HIDD_ATTRIBUTES attribs;
		HidD_GetAttributes(hidInfo.hidDevice, &attribs);
		wchar_t longString[maxDeviceNameSize] = {};
		HidD_GetManufacturerString(hidInfo.hidDevice, longString, maxDeviceNameSize);
		HidD_GetSerialNumberString(hidInfo.hidDevice, longString, maxDeviceNameSize);
		*/
	info.osDeviceName = weave::utf::Utf16toUtf8(rawInputDeviceName);
	info.osProductName = weave::utf::Utf16toUtf8(
		HidD_GetProductString(hidInfo.hidDevice, productString, sizeof(wchar_t) * maxDeviceNameSize) ? productString : L"[Unnamed]"
	);
	
	//Get the preparsed data structure used by all HidP_* functions. This can be done either via RawInput or via Hid, depending on the handle we want to use. 
	if(HidD_GetPreparsedData(hidInfo.hidDevice, &hidInfo.preparsedData) == FALSE)
		return false;

	//Query the size of the preparsed data structure and create it
	//UINT bufferSize = 0;
	//if(GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, NULL, &bufferSize) != 0)
	//	return false;
	////Create the structure and request the data
	//preparsedData = (PHIDP_PREPARSED_DATA) new uint8_t[bufferSize];
	//GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, preparsedData, &bufferSize);


	//Get the general caps for the device
	//The general caps contains information about the number of buttons on the device as well as the number of data feeds (values) that come from it
	if(HidP_GetCaps(hidInfo.preparsedData, &hidInfo.generalCaps) != HIDP_STATUS_SUCCESS) {
		return false;
	}

	//Prepare the buffer that will be used to hold the translated information about button and value status from the gamepad's report
	auto hidreportMapSize = HidP_MaxDataListLength(HidP_Input, hidInfo.preparsedData);
	if(hidreportMapSize == 0) {
		return false;
	}
	hidReportData.resize(hidreportMapSize);
	reportMap.resize(hidreportMapSize);
	reportMapPrev.resize(hidreportMapSize);
	
	//Prepare the buffer data that will be used when reading the reports
	rawbuff.resize(hidInfo.generalCaps.InputReportByteLength + sizeof(RAWINPUT) + 1);

	//Create space for the button caps and request them
	USHORT capsLen = hidInfo.generalCaps.NumberInputButtonCaps;
	hidInfo.buttonCaps.resize(capsLen);
	if(HidP_GetButtonCaps(HidP_Input, hidInfo.buttonCaps.data(), &capsLen, hidInfo.preparsedData) != HIDP_STATUS_SUCCESS) {
		return false;
	}
	numButtons = hidInfo.buttonCaps[0].Range.DataIndexMax - hidInfo.buttonCaps[0].Range.DataIndexMin + 1;
	firstButtonIndex = hidInfo.buttonCaps[0].Range.DataIndexMin;
	lastButtonIndex = hidInfo.buttonCaps[0].Range.DataIndexMax;
		
	//Create space for the value caps structure (one per value!) and request them
	capsLen = hidInfo.generalCaps.NumberInputValueCaps;
	hidInfo.valueCaps.resize(capsLen);
	if(HidP_GetValueCaps(HidP_Input, hidInfo.valueCaps.data(), &capsLen, hidInfo.preparsedData) != HIDP_STATUS_SUCCESS) {
		return false;
	}
	numValues = hidInfo.generalCaps.NumberInputValueCaps;

	//Create space for the output value caps (these are related to rumble packs)
	capsLen = hidInfo.generalCaps.NumberOutputValueCaps;
	if(capsLen) {
		hidInfo.outputValueCaps.resize(capsLen);
		if(HidP_GetValueCaps(HidP_Output, hidInfo.outputValueCaps.data(), &capsLen, hidInfo.preparsedData) != HIDP_STATUS_SUCCESS) {
			return false;
		}
	}

	//Helper function to return a valid axis normalization value given a valueCaps extracted axis
	auto NormValue = [](PHIDP_VALUE_CAPS axis) -> float {
		float max = ((USHORT)axis->LogicalMax > 0) ? ((USHORT)axis->LogicalMax) : ((USHORT)axis->PhysicalMax > 0) ? ((USHORT)axis->PhysicalMax) : 255.0f;
		float min = ((USHORT)axis->LogicalMin > 0) ? ((USHORT)axis->LogicalMin) : ((USHORT)axis->PhysicalMin > 0) ? ((USHORT)axis->PhysicalMin) : 0.0f;
		return 2.0f / (max - min);
	};
	//The value caps store information about each provided value on a gamepad report. We look for supported values to keep track of them.
	for(unsigned int i = 0; i < numValues; ++i) {
		switch(hidInfo.valueCaps[i].Range.UsageMin) {
		case 0x30:	//Left stick X axis
		if (!leftX) {
			leftX = &hidInfo.valueCaps[i];
			leftNorm.x = NormValue(leftX);
		}
		break;

		case 0x31:	//Left stick Y axis
		if (!leftY) {
			leftY = &hidInfo.valueCaps[i];
			leftNorm.y = NormValue(leftY);
		}
		break;

		case 0x32: //Left Z axis
		if (!leftZ) {
			leftZ = &hidInfo.valueCaps[i];
			leftNorm.z = NormValue(leftZ);
		}
		break;

		case 0x33: //Right stick X axis
		if (!rightX) {
			rightX = &hidInfo.valueCaps[i];
			rightNorm.x = NormValue(rightX);
		}
		break;

		case 0x34: //Right stick Y axis
		if (!rightY) {
			rightY = &hidInfo.valueCaps[i];
			rightNorm.y = NormValue(rightY);
		}
		break;

		case 0x35: //Right Z axis
		if (!rightZ) {
			rightZ = &hidInfo.valueCaps[i];
			rightNorm.z = NormValue(rightZ);
		}
		break;

		case 0x39:	//Hat Switch
		if (!hatSwitch) {
			hatSwitch = &hidInfo.valueCaps[i];
		}
		break;

		//Legacy digital pad
		case 0x90:
		dpadUp = &hidInfo.valueCaps[i];
		break;
		case 0x91:
		dpadDown = &hidInfo.valueCaps[i];
		break;
		case 0x92:
		dpadRight = &hidInfo.valueCaps[i];
		break;
		case 0x93:
		dpadLeft = &hidInfo.valueCaps[i];
		break;

		default:
			break;
		}
	}
	
	//Set the default dead zone values
	leftDeadZone = 0.2f;
	rightDeadZone = 0.2f;
	leftDeadZoneSqr = leftDeadZone * leftDeadZone;
	rightDeadZoneSqr = rightDeadZone * rightDeadZone;

	//Select the layout of the controller automatically
	AutoLayout();

	info.hardwareId.uniqueId = (uint64_t)hidInfo.hDevice;

	return true;
}

//Automatically selects a layout based on known values for the device
void RawInputGamepad::GamepadDeviceData::AutoLayout() {
	
	std::string lcdevname = info.osProductName;
	std::transform(lcdevname.begin(), lcdevname.end(), lcdevname.begin(), [](char c) { return (char)::tolower((int)c); });

	//XBox layout
	//Product string identifying it or buttons == 10, numValues = 6, leftZ but no rightZ
	if((numButtons == 10 && numValues == 6 && leftZ && !rightZ) || lcdevname.find("xbox") != std::string::npos || lcdevname.find("360") != std::string::npos) {
		info.detectedLayout = ((lcdevname.find("360") != std::string::npos) ? GamepadLayout::XBox360Gamepad : GamepadLayout::XBoxOneGamepad);
	}
	//PS5 layout
	//Wirless Controller product name, numButtons == 15, numValues == 9, leftZ && rightZ
	else if (numButtons == 15 && numValues == 9 && rightX && rightY && leftZ && rightZ && lcdevname.find("wireless") != std::string::npos) {
		info.detectedLayout = GamepadLayout::PlayStation5;
	}

	//PS4 layout
	//Unknown product name, numButtons == 14, numValues == 9, leftZ && rightZ
	else if(numButtons == 14 && numValues == 9 && rightX && rightY && leftZ && rightZ) {
		info.detectedLayout = GamepadLayout::PlayStation4;
	}

	//PS3 clone layout
	else if(numButtons == 13 && rightX && rightY) {
		info.detectedLayout = GamepadLayout::PlayStation3;
	}

	//Switch Joy Con
	else if (lcdevname.find("wireless gamepad") != std::string::npos &&
		(numButtons == 16 && numValues == 10 && !rightZ)) {
		info.detectedLayout = GamepadLayout::Generic;
	}

	else {
		//Generic layout
		info.detectedLayout = GamepadLayout::Generic;
	}

	info.currentLayout = info.detectedLayout;
	info.deviceName = weave::input::gamepad::LayoutName(info.detectedLayout);

	//Some gamepads report the secondary analog stick as the  Rx and Ry while others seem to report it as Lz and Rz, so we make sure that Rx and Ry are set if we find Lz and Rz set
	//Generic swapped gamepad where rightX and Y are not defined
	if (!rightX && rightZ && !rightY && leftZ) {
		rightX = leftZ;
		rightY = rightZ;
		rightNorm.x = leftNorm.z;
		rightNorm.y = rightNorm.z;
		leftZ = nullptr;
		rightZ = nullptr;
	}
	//PS4 / PS5 gamepad where rightXY is swaped with leftZ/rightZ respectively
	else if (leftZ && rightZ && rightX && rightY && 
		(info.detectedLayout == GamepadLayout::PlayStation4 || info.detectedLayout == GamepadLayout::PlayStation5)) {
		//Swap the leftZ with rightX and rightZ into rightY
		std::swap(leftZ, rightX);
		std::swap(leftNorm.z, rightNorm.x);
		std::swap(rightZ, rightY);
		std::swap(rightNorm.z, rightNorm.y);

		leftNorm.z *= 0.5f;
		rightNorm.z *= 0.5f;
	}
}

//Helper inline function to calculate stick deltas and positions.
//Takes into account current position, previous and deadzone for the stick. 
//Deadzone calculations are performed to ensure the step that crosses the boundary does not report a big delta.
//Returns true if an event should be issued with the change
namespace {
	bool GetStickPositionDelta(ULONG stick_currX, ULONG stick_currY, ULONG stick_prevX, ULONG stick_prevY,
		float normX, float normY, float deadZone, float deadZoneSqr,
		float& x, float& y, float& deltax, float& deltay) {
		//Check if an update is really needed
		if (stick_currX != stick_prevX || stick_currY != stick_prevY) {
			//Calculate previous and current positions
			float prevx = (stick_prevX * normX) - 1.0f;
			float prevy = -(stick_prevY * normY) + 1.0f;
			x = (stick_currX * normX) - 1.0f;
			y = -(stick_currY * normY) + 1.0f;
			//Calculate the delta before deadzone adjustment so that we don't show a big jump when crossing the deadzone boundary 
			deltax = x - prevx;
			deltay = y - prevy;
			//Clamp to deadzone values and normalize the position of the stick into the active range 
			x = (x * x > deadZoneSqr) ? (x - copysignf(deadZone, x)) / (1.0f - deadZone) : 0.0f;
			y = (y * y > deadZoneSqr) ? (y - copysignf(deadZone, y)) / (1.0f - deadZone) : 0.0f;
			prevx = (prevx * prevx > deadZoneSqr) ? (prevx - copysignf(deadZone, x)) / (1.0f - deadZone) : 0.0f;
			prevy = (prevy * prevy > deadZoneSqr) ? (prevy - copysignf(deadZone, y)) / (1.0f - deadZone) : 0.0f;

			//We only need to emit an event if previous position and current are different
			return (prevx != x || prevy != y);
		}

		return false;
	}
}

//Processes raw input data and generates input events from it
int RawInputGamepad::GamepadDeviceData::FeedSyncInput(HRAWINPUT lparam, InputPipeline& inputPipeline) {
	//This will hold the actual buffer size that GetRawInputData returns

	//Read the raw data
	auto rawSize = static_cast<UINT>(rawbuff.size()); 
	auto rawInputHeader = reinterpret_cast<RAWINPUT*>(rawbuff.data());
	auto readResult = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, rawInputHeader, &rawSize, sizeof(RAWINPUTHEADER));
	if (readResult == static_cast<UINT>(-1)) {
		return 0;
	}

	//Request the complete data the gamepad has to offer, in a parsed form of data index / value
	ULONG dataSize = static_cast<ULONG>(hidReportData.size());
	if(HidP_GetData(HidP_Input, hidReportData.data(), &dataSize, hidInfo.preparsedData, (PCHAR)rawInputHeader->data.hid.bRawData, hidInfo.generalCaps.InputReportByteLength) != HIDP_STATUS_SUCCESS) {
		return 0;
	}

	//Go through the resulting data and index it into the actual buffer we use to extract the data from. This buffer is used as a middle man so that we can logically parse data by indexing into it
	//instead of having to do an if/loop
	ZeroMemory(reportMap.data(), sizeof(ULONG) * reportMap.size());
	for(ULONG i = 0; i < dataSize; ++i) {
		USHORT idx = hidReportData[i].DataIndex;
		reportMap[idx] = hidReportData[i].RawValue;
	}

	//With the current report extracted and linearly set, we can logically index the report to find out if we must send any events or not. Phew!
	//Go through the buttons
	for(USHORT i = firstButtonIndex; i <= lastButtonIndex; ++i) {
		//Send only an event if the reports are different between calls
		if(reportMapPrev[i] != reportMap[i]) {
			//Use the key index to determine if we can use the layout to map the button or just get an offset button
			auto layout = kStandardLayouts[static_cast<uint32_t>(info.currentLayout)];
			uint32_t keyIndex = (i - firstButtonIndex);
			VirtualKey button = (keyIndex < 20 ? layout[keyIndex] : input::OffsetKey(VirtualKey::Button0, keyIndex));
			inputPipeline.FeedKeyEvent(info.virtualDevice, button, (reportMap[i] > 0 ? VirtualKeyState::Down : VirtualKeyState::Up));
		}
	}

	//Check each stick axes and generate an event for them, if required
	//Left stick
	USHORT idx0, idx1, idx2; //index used to access the reportMap, for readability purposes
	if(leftX && leftY) {
		idx0 = leftX->Range.DataIndexMin;
		idx1 = leftY->Range.DataIndexMin;

		float x, y, deltax, deltay;
		bool sendEvent =
			GetStickPositionDelta(reportMap[idx0], reportMap[idx1], //Current position of the stick
								  reportMapPrev[idx0], reportMapPrev[idx1], //Previous position of the stick
								  leftNorm.x, leftNorm.y, //Normalization values
								  leftDeadZone, leftDeadZoneSqr,//Deadzone values
								  x, y, deltax, deltay); //The output
		if(sendEvent) {
#ifdef _DEBUG_GAMEPAD_ANALOG_OUTPUT
			WLOG(WLog::Info) << "L : " << reportMap[idx0] << ", " << reportMap[idx1];
			WLOG(WLog::Info) << "Lf: " << x << ", " << y;
#endif
			inputPipeline.FeedCursorPositionAndDelta(info.virtualDevice, VirtualKey::Lstick, x, y, 0.0f, deltax, deltay, 0.0f);
			EmulateDigitalPad(0, x, y, deltax, deltay, inputPipeline);
		}
	}

	//Right stick
	if(rightX && rightY) {
		idx0 = rightX->Range.DataIndexMin;
		idx1 = rightY->Range.DataIndexMin;

		float x, y, deltax, deltay;
		bool sendEvent =
			GetStickPositionDelta(reportMap[idx0], reportMap[idx1], //Current position of the stick
								  reportMapPrev[idx0], reportMapPrev[idx1], //Previous position of the stick
								  rightNorm.x, rightNorm.y, //Normalization values
								  rightDeadZone, rightDeadZoneSqr,//Deadzone values
								  x, y, deltax, deltay); //The output
		if(sendEvent) {
#ifdef _DEBUG_GAMEPAD_ANALOG_OUTPUT
			WLOG(WLog::Info) << "R : " << reportMap[idx0] << ", " << reportMap[idx1];
			WLOG(WLog::Info) << "Rf: " << x << ", " << y;
#endif
			inputPipeline.FeedCursorPositionAndDelta(info.virtualDevice, VirtualKey::Rstick, x, y, 0.0f, deltax, deltay, 0.0f);
			EmulateDigitalPad(1, x, y, deltax, deltay, inputPipeline);
		}
	}

	//Left Z axis (Trigger)
	if(leftZ) {
		idx2 = leftZ->Range.DataIndexMin;
		if(reportMapPrev[idx2] != reportMap[idx2]) {
#ifdef _DEBUG_GAMEPAD_ANALOG_OUTPUT
			WLOG(WLog::Info) << "ZL :" << reportMap[idx2];
			WLOG(WLog::Info) << "ZLf:" << reportMap[idx2] * leftNorm.z;
#endif
			//The XBox gamepad has special functionality when it comes to the LZ trigger data because both LT and RT are reported throught he same axis
			//This data is attempted to solve into something that can work both for analog input and digital emulation
			if(
				info.detectedLayout == GamepadLayout::XBox360Gamepad ||
				info.detectedLayout == GamepadLayout::XBoxOneGamepad) {

				float x, y, deltax, deltay;
				bool sendEvent =
					GetStickPositionDelta(reportMap[idx2], 0, //Current position of the stick
										  reportMapPrev[idx2], 0, //Previous position of the stick
										  leftNorm.z, 1.0f, //Normalization values
										  leftDeadZone, leftDeadZoneSqr,//Deadzone values
										  x, y, deltax, deltay); //The output

				if(sendEvent) {
					//Check analog/digital emulation for XBox
					XBoxTriggerEmulation(x, deltax, inputPipeline);
				}
			}
			else {
				//Default work is to report as trigger data
				float z = (reportMap[idx2] * leftNorm.z);
				inputPipeline.FeedCursorPosition(info.virtualDevice, VirtualKey::Trigger0, z, z, z);
			}
		}
	}

	//Right Z axis (Trigger)
	if(rightZ) {
		idx2 = rightZ->Range.DataIndexMin;
		if(reportMapPrev[idx2] != reportMap[idx2]) {
			float z = (reportMap[idx2] * leftNorm.z);

#ifdef _DEBUG_GAMEPAD_ANALOG_OUTPUT
			WLOG(WLog::Info) << "ZR :" << reportMap[idx2];
			WLOG(WLog::Info) << "ZRf:" << z;
#endif
			inputPipeline.FeedCursorPosition(info.virtualDevice, VirtualKey::Trigger1, z, z, z);
		}

	}

	//The hat switch (cross)
	if(hatSwitch) {
		idx0 = hatSwitch->Range.DataIndexMin;
		if(reportMapPrev[idx0] != reportMap[idx0]) {
			//For the hatswitch, offset the value by the minimun value defined in the caps to get a range of 0-7. Anything outside this value will be considered "Released"
			//Since the hat switch can only have one state at any time, we send a release event on the previous key first and then a press event for the new key
			LONG state = reportMapPrev[idx0] - hatSwitch->LogicalMin;
			if(state >= 0 && state <= 7)
				inputPipeline.FeedKeyEvent(info.virtualDevice, input::OffsetKey(VirtualKey::North, (uint8_t)state), VirtualKeyState::Up);

			state = reportMap[idx0] - hatSwitch->LogicalMin;
			if(state >= 0 && state <= 7)
				inputPipeline.FeedKeyEvent(info.virtualDevice, input::OffsetKey(VirtualKey::North, (uint8_t)state), VirtualKeyState::Down);
		}
	}

	//The legacy digital pad. We check for Up first, then check the rest if Up exists
	if(dpadUp) {
		idx0 = dpadUp->Range.DataIndexMin;
		if(reportMapPrev[idx0] != reportMap[idx0]) {
			inputPipeline.FeedKeyEvent(info.virtualDevice, VirtualKey::North, reportMap[idx0] ? VirtualKeyState::Down : VirtualKeyState::Up);
		}

		if(dpadDown) {
			idx0 = dpadDown->Range.DataIndexMin;
			if(reportMapPrev[idx0] != reportMap[idx0]) {
				inputPipeline.FeedKeyEvent(info.virtualDevice, VirtualKey::South, reportMap[idx0] ? VirtualKeyState::Down : VirtualKeyState::Up);
			}
		}

		if(dpadLeft) {
			idx0 = dpadLeft->Range.DataIndexMin;
			if(reportMapPrev[idx0] != reportMap[idx0]) {
				inputPipeline.FeedKeyEvent(info.virtualDevice, VirtualKey::West, reportMap[idx0] ? VirtualKeyState::Down : VirtualKeyState::Up);
			}
		}

		if(dpadRight) {
			idx0 = dpadRight->Range.DataIndexMin;
			if(reportMapPrev[idx0] != reportMap[idx0]) {
				inputPipeline.FeedKeyEvent(info.virtualDevice, VirtualKey::East, reportMap[idx0] ? VirtualKeyState::Down : VirtualKeyState::Up);
			}
		}
	}

	reportMapPrev = reportMap;
	
	return 1;
}

//Helper function that emulates digital style response with analog sticks and generates the required binary events
//Stick: 0 -> Left, 1 -> Right
void RawInputGamepad::GamepadDeviceData::EmulateDigitalPad(int stick, float posx, float posy, float deltax, float deltay, InputPipeline& inputPipeline) {

	static const float deadLimit = 0.50f; //Minimun boundary the analog stick needs to cross to consider the stick press
	static const float responseDelta = 0.15f; //Minimun delta to consider a stick binary change

	//Prepare the stick data
	IVector2 &dir = stick == 0 ? leftDir : rightDir;
	Vector2 &cap = stick == 0 ? leftCap : rightCap;

	//Y axis
	if(deltay > 0.0f) {
		//Pullup detection
		if(dir.y == 0 && posy - cap.y > responseDelta && posy > deadLimit) {
			//Send North Down event
			inputPipeline.FeedKeyEvent(info.virtualDevice, stick == 0 ? VirtualKey::Lstick_North : VirtualKey::Rstick_North, VirtualKeyState::Down);
			dir.y = 1;
		}
		if(dir.y == 1 && posy > cap.y) {
			cap.y = posy;
		}

		//Release pulldown detection
		if(dir.y == -1 && posy - cap.y > responseDelta) {
			//Send South release (Up) event here
			inputPipeline.FeedKeyEvent(info.virtualDevice, stick == 0 ? VirtualKey::Lstick_South : VirtualKey::Rstick_South, VirtualKeyState::Up);
			dir.y = 0;
			cap.y = 0.0f;
		}

		if(dir.y == 0 && posy > cap.y && posy < 0.0f) {
			cap.y = posy;
		}
	}

	if(deltay < 0.0f) {
		//Pulldown detection
		if(dir.y == 0 && cap.y - posy > responseDelta && posy < -deadLimit) {
			//Send South Down event
			inputPipeline.FeedKeyEvent(info.virtualDevice, stick == 0 ? VirtualKey::Lstick_South : VirtualKey::Rstick_South, VirtualKeyState::Down);
			dir.y = -1;
		}

		if(dir.y == -1 && posy < cap.y) {
			cap.y = posy;
		}

		//Release pullup detection
		if(dir.y == 1 && cap.y - posy > responseDelta) {
			//Send North release (Up) event here
			inputPipeline.FeedKeyEvent(info.virtualDevice, stick == 0 ? VirtualKey::Lstick_North : VirtualKey::Rstick_North, VirtualKeyState::Up);
			dir.y = 0;
			cap.y = 0.0f;
		}
		if(dir.y == 0 && cap.y > posy && posy > 0.0f) {
			cap.y = posy;
		}
	}


	//X axis
	if(deltax > 0.0f) {
		//Pullup detection
		if(dir.x == 0 && posx - cap.x > responseDelta && posx > deadLimit) {
			//Send East Down event
			inputPipeline.FeedKeyEvent(info.virtualDevice, stick == 0 ? VirtualKey::Lstick_East : VirtualKey::Rstick_East, VirtualKeyState::Down);
			dir.x = 1;
		}
		if(dir.x == 1 && posx > cap.x) {
			cap.x = posx;
		}

		//Release pulldown detection
		if(dir.x == -1 && posx - cap.x > responseDelta) {
			//Send West release (Up) event here
			inputPipeline.FeedKeyEvent(info.virtualDevice, stick == 0 ? VirtualKey::Lstick_West : VirtualKey::Rstick_West, VirtualKeyState::Up);
			dir.x = 0;
			cap.x = 0.0f;
		}

		if(dir.x == 0 && posx > cap.x && posx < 0.0f) {
			cap.x = posx;
		}
	}

	if(deltax < 0.0f) {
		//Pulldown detection
		if(dir.x == 0 && cap.x - posx > responseDelta && posx < -deadLimit) {
			//Send West Down event
			inputPipeline.FeedKeyEvent(info.virtualDevice, stick == 0 ? VirtualKey::Lstick_West : VirtualKey::Rstick_West, VirtualKeyState::Down);
			dir.x = -1;
		}

		if(dir.x == -1 && posx < cap.x) {
			cap.x = posx;
		}

		//Release pullup detection
		if(dir.x == 1 && cap.x - posx > responseDelta) {
			//Send East release (Up) event here
			inputPipeline.FeedKeyEvent(info.virtualDevice, stick == 0 ? VirtualKey::Lstick_East : VirtualKey::Rstick_East, VirtualKeyState::Up);
			dir.x = 0;
			cap.x = 0.0f;
		}
		if(dir.x == 0 && cap.x > posx && posx > 0.0f) {
			cap.x = posx;
		}
	}
}

//Emulates digital interactions for the XBox gamepad.
//This gamepad reports both LT and RT analog triggers on the same axis, which makes it impossible to get an accurte reading. Emulation is implemented to make the data useful.
void RawInputGamepad::GamepadDeviceData::XBoxTriggerEmulation(float posx, float deltax, InputPipeline& inputPipeline) {
	//Digital emulation
#ifdef _DEBUG_GAMEPAD_ANALOG_OUTPUT
	WLOG(WLog::Info) << "XBox: Pos: " << posx << " Delta: " << deltax;
#endif

	if(posx == 0.0f) {
		xBoxTrigger = 0; //Neutral, LT and RT released, no event generated
	}
	else if(deltax > 0.0f) {
		//Is it neutral?
		if(xBoxTrigger == 0) {
			xBoxTrigger = 1; //LT pressed
			inputPipeline.FeedKeyEvent(info.virtualDevice, VirtualKey::XBox_LT, VirtualKeyState::Down);
#ifdef _DEBUG_GAMEPAD_DIGITAL_OUTPUT
			WLOG(WLog::Info) << "LT Press";
#endif
		}

		//Is RT pressed?
		if(xBoxTrigger == -1) { 
			xBoxTrigger = -2; //Disabled until posx = 0
			inputPipeline.FeedKeyEvent(info.virtualDevice, VirtualKey::XBox_RT, VirtualKeyState::Up);
#ifdef _DEBUG_GAMEPAD_DIGITAL_OUTPUT
			WLOG(WLog::Info) << "RT Release";
#endif
		}
	}
	else if(deltax < 0.0f) {
		//Is it neutral?
		if(xBoxTrigger == 0) {
			xBoxTrigger = -1; //RT pressed
			inputPipeline.FeedKeyEvent(info.virtualDevice, VirtualKey::XBox_RT, VirtualKeyState::Down);
#ifdef _DEBUG_GAMEPAD_DIGITAL_OUTPUT
			WLOG(WLog::Info) << "RT Press";
#endif
		}

		//Is LT pressed?
		if(xBoxTrigger == 1) {
			xBoxTrigger = 2; //Disabled until posx = 0
			inputPipeline.FeedKeyEvent(info.virtualDevice, VirtualKey::XBox_LT, VirtualKeyState::Up);
#ifdef _DEBUG_GAMEPAD_DIGITAL_OUTPUT
			WLOG(WLog::Info) << "LT Release";
#endif
		}
	}

	//Analog emulation
	if(posx < 0.0f) {
		posx = -posx;
		deltax = -deltax;
		inputPipeline.FeedCursorPositionAndDelta(info.virtualDevice, VirtualKey::XBox_RTrigger, posx, posx, posx, deltax, deltax, deltax);
	}
	else if(posx > 0.0f) {
		inputPipeline.FeedCursorPositionAndDelta(info.virtualDevice, VirtualKey::XBox_LTrigger, posx, posx, posx, deltax, deltax, deltax);
	}
	else if(posx == 0.0f) {
		inputPipeline.FeedCursorPositionAndDelta(info.virtualDevice, VirtualKey::XBox_LTrigger, posx, posx, posx, deltax, deltax, deltax);
		inputPipeline.FeedCursorPositionAndDelta(info.virtualDevice, VirtualKey::XBox_RTrigger, posx, posx, posx, deltax, deltax, deltax);
	}
}

//Creates a write packet sent to the gamepad that makes it rumble, if compatible
//@heavy, weak: Intensity values for low freq and high freq rumble, normalized 0.0 .. 1.0
//@left, right: Special rumble for XBox pads on left and right triggers. Not used on PS4
//@time: Amount of time to rumble, normalized
//@delay: Delay between rumbles, normalized
//@repeat: Amount of repetitions of the rumble pattern. Not normalized.
void RawInputGamepad::GamepadDeviceData::Rumble(float heavy, float weak, float left, float right, float time, float delay, uint8_t repeat) {
	WriteRumblePkg(heavy, weak, left, right, time, delay, repeat, ledColor, stayOnFlash, stayOffFlash);
}

//Creates a write packet sent to the gamepad that makes it change LED color, if compatible
//RGB intensity values are normalized 0.0 .. 1.0
void RawInputGamepad::GamepadDeviceData::Light(Vector3 const & rgb) {
	ledColor = rgb;
	WriteRumblePkg(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, ledColor, stayOnFlash, stayOffFlash);
}

//Creates a write packet sent to the gamepad that makes it flash the LED, if compatible
//@stayOn: Normalized time (0.0 .. 1.0) for the light to be stay on when flashing
//@stayOff: Normalized time (0.0 .. 1.0) for the light to be stay off when flashing
void RawInputGamepad::GamepadDeviceData::Flash(float stayOn, float stayOff) {
	stayOnFlash = stayOn;
	stayOffFlash = stayOff;
	WriteRumblePkg(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, ledColor, stayOnFlash, stayOffFlash);
}

//Creates the full rumble / light / flash package and sends it to the gamepad
void RawInputGamepad::GamepadDeviceData::WriteRumblePkg(float heavy, float weak, float left, float right, float time, float delay, uint8_t repeat, Vector3 const & rgb, float stayOn, float /*stayOff*/)  {
	//Some very rough documentation on this packet can be found at:
	//https://stackoverflow.com/questions/25506101/gamepad-force-feedback-vibration-on-windows-using-raw-input/27437973#27437973
	//https://github.com/ros-drivers/joystick_drivers/blob/indigo-devel/ps3joy/scripts/ps3joy_node.py#L274
	//https://github.com/chrippa/ds4drv/blob/master/ds4drv/device.py#L117
	uint8_t data[32]; //The PS4 pad needs 32 bytes of data. Most of it is not documented.
	std::memset(data, 0, sizeof(data));

	data[0] = 0x05; //This seems to be the report type id.  The PS4 example code cited seems to indicate that Bluetooth connected devices should use 0x11. Can't test. Works on XBox.
	if(
		info.detectedLayout == GamepadLayout::PlayStation3 ||
		info.detectedLayout == GamepadLayout::PlayStation4 ||
		info.detectedLayout == GamepadLayout::PlayStation5 ||
		info.detectedLayout == GamepadLayout::Generic) {

		data[1] = 0xFF; //Arbitrary? 128 for bluetooth? No apparent effect
		data[2] = 0x00; //Arbitrary? When set to 0xFF the PS4 controller turns the LED off after flashing.
		data[3] = 0x00; //Arbitrary? 255 for bluetooth? No apparent effect
		data[4] = uint8_t(weak * 255.0f); //right_motor_strength;  // 0-255
		data[5] = uint8_t(heavy * 255.0f); //left_motor_strength;   // 0-255
		//RGB LED
		data[6] = uint8_t(rgb.r * 255.0f); //led_red_level;         // 0-255
		data[7] = uint8_t(rgb.g * 255.0f); // led_green_level;       // 0-255
		data[8] = uint8_t(rgb.b * 255.0f); // led_blue_level;        // 0-255
		data[9] = uint8_t(stayOn * 255.0f); //Flash on time. Cap is 2.5 seconds
		data[10] = uint8_t(stayOn * 255.0f); //Flash off time. Cap is 2.5 seconds
	}
	else if(
		info.detectedLayout == GamepadLayout::XBox360Gamepad ||
		info.detectedLayout == GamepadLayout::XBoxOneGamepad) {

		data[1] = 0xFF; //Arbitrary? No apparent effect
		data[2] = uint8_t(left * 255.0f); //Left trigger rumble
		data[3] = uint8_t(right * 255.0f); //Right trigger rumble
		data[4] = uint8_t(heavy * 255.0f); //Low frequency rumble
		data[5] = uint8_t(weak * 255.0f); //High frequency rumble
		data[6] = uint8_t(time * 255.0f); //Rumble time. Max 2.5 seconds
		data[7] = uint8_t(delay * 255.0f); //Delay of activation. Max 2.5 seconds
		data[8] = repeat; //Number of repetitions. 0x00 = rumble once only, no repeats
	}

	DWORD bytesWritten;
	WriteFile(hidInfo.hidDevice, data, sizeof(data), &bytesWritten, NULL);
}

void RawInputGamepad::GamepadDeviceData::SetDeadzones(float left, float right) {
	leftDeadZone = left;
	leftDeadZoneSqr = leftDeadZone * leftDeadZone;
	rightDeadZone = right;
	rightDeadZoneSqr = rightDeadZone * rightDeadZone;
}

bool RawInputGamepad::GamepadDeviceData::IsLinked(VirtualDevice vDev) const {
	return info.virtualDevice == vDev;
}

void RawInputGamepad::GamepadDeviceData::Link(VirtualDevice vDev) {
	info.virtualDevice = vDev;
}
void  RawInputGamepad::GamepadDeviceData::Unlink() {
	info.virtualDevice = VirtualDevice::None;
}

//------------------------

std::vector<RawInputGamepad::GamepadId> RawInputGamepad::GetLinkedGamepadIds(VirtualDevice vDev) const {
	std::vector<GamepadId> links;
	for (auto const& pad : gamepadDevices) {
		if (pad.IsLinked(vDev)) {
			links.emplace_back(pad.info.hardwareId);
		}
	}

	return links;
}

RawInputGamepad::GamepadInfo RawInputGamepad::GetGamepadInfo(GamepadId gamepadId) const {
	for (auto const& pad : gamepadDevices) {
		if (pad.info.hardwareId == gamepadId) {
			return pad.info;
		}
	}

	return {};
}

std::vector<RawInputGamepad::GamepadInfo> RawInputGamepad::GetEnumeratedGamepadInfo() const {
	std::vector<GamepadInfo> info;
	for (auto const& pad : gamepadDevices) {
		info.emplace_back(pad.info);
	}

	return info;
}

bool RawInputGamepad::InitializeGamepad(HANDLE hDevice) {
	if (!hDevice)
		return false;

	//See if the device is already allocated in the gamepad vector
	for (auto const& pad : gamepadDevices) {
		if (pad.hidInfo.hDevice == hDevice) {
			//The pad is already allocated, all good.
			return true;
		}
	}

	//Build a temporary gamepad before inserting it on the available gamepads list
	GamepadDeviceData gamepad;
	if (!gamepad.Initialize(hDevice))
		return false;


	//Find a free virtual device to assign
	for (auto vDev = static_cast<unsigned short>(VirtualDevice::_First_Gamepad); vDev <= static_cast<unsigned short>(VirtualDevice::_Last_Gamepad); ++vDev) {
		//Look for the virtual device on the registered device list
		bool found = false;
		for (auto const& pad : gamepadDevices) {
			if (pad.IsLinked(static_cast<VirtualDevice>(vDev))) {
				found = true;
				break;
			}
		}

		//If the device was not found, we've found a free device to assign!
		if (!found) {
			gamepad.Link(static_cast<VirtualDevice>(vDev));
			break;
		}
	}

	//With the gamepad structure created, add it to the gamepad vector
	deviceEvents.emplace_back(gamepad.info.virtualDevice, DeviceState::Idle);
	gamepadDevices.emplace_back(std::move(gamepad));

	return true;
}

size_t RawInputGamepad::EnumerateGamepads(HWND hwnd) {
	
	if (hwnd) {

		//Register to receive data from joysticks (page 1, usage 4) and gamepads (page 1, usage 5)
		RAWINPUTDEVICE Rid[2];
		Rid[0].usUsagePage = 1;
		Rid[0].usUsage = 4; //Joystic HID
		Rid[0].dwFlags = RIDEV_DEVNOTIFY; //Rid[0].dwFlags = RIDEV_INPUTSINK;   
		Rid[0].hwndTarget = hwnd;

		Rid[1].usUsagePage = 1;
		Rid[1].usUsage = 5; //Gamepad HID
		Rid[1].dwFlags = RIDEV_DEVNOTIFY; //Rid[0].dwFlags = RIDEV_INPUTSINK;   
		Rid[1].hwndTarget = hwnd;

		RegisterRawInputDevices(Rid, 2, sizeof(Rid[0]));
		
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
		//Skip keyboards and mice prematurely
		if (dev.dwType == RIM_TYPEHID) {
			InitializeGamepad(dev.hDevice);
		}
	}

	return gamepadDevices.size();
}

void RawInputGamepad::LinkGamepad(GamepadId gamepadId, VirtualDevice vDev) {
	if (auto* pad = FindGamepad(gamepadId); pad) {
		deviceEvents.emplace_back(vDev, DeviceState::Idle);
		pad->Link(vDev);
	}
}

void RawInputGamepad::UnlinkGamepad(GamepadId gamepadId) {
	if (auto* pad = FindGamepad(gamepadId); pad) {
		deviceEvents.emplace_back(pad->info.virtualDevice, DeviceState::Disconnected);
		pad->Unlink();
	}
}

void RawInputGamepad::UnlinkVirtualDevice(VirtualDevice vDev) {
	auto ids = GetLinkedGamepadIds(vDev);
	for (auto id : ids) {
		UnlinkGamepad(id);
	}
}

void RawInputGamepad::SetDeadzones(GamepadId gamepadId, float leftDeadzone, float rightDeadzone) {
	if (auto* pad = FindGamepad(gamepadId); pad) {
		pad->SetDeadzones(leftDeadzone, rightDeadzone);
	}
}

void RawInputGamepad::SetDeadzones(float leftDeadzone, float rightDeadzone) {
	for (auto& pad : gamepadDevices) {
		pad.SetDeadzones(leftDeadzone, rightDeadzone);
	}
}

void RawInputGamepad::SetLayout(GamepadId gamepadId, GamepadLayout layout) {
	if (auto* pad = FindGamepad(gamepadId); pad) {
		pad->info.currentLayout = layout;
	}
}

size_t RawInputGamepad::ReleaseDeadGamepads() {
	//Go through all registered gamepads and do a fake probe for some device information. If the probe fails, the device has been disconected so we remove it.
	std::erase_if(gamepadDevices, [this](auto const& pad) {
		UINT dataSize;
		bool isDead = (GetRawInputDeviceInfo(pad.hidInfo.hDevice, RIDI_DEVICENAME, nullptr, &dataSize) == static_cast<UINT>(-1));
		if (isDead) {
			deviceEvents.emplace_back(pad.info.virtualDevice, DeviceState::Idle);
		}
		return isDead;
	});

	return gamepadDevices.size();
}

void RawInputGamepad::ReleaseAllGamepads() {
	for (auto const& gamepad : gamepadDevices) {
		deviceEvents.emplace_back(gamepad.info.virtualDevice, DeviceState::Idle);
	}
	gamepadDevices.clear();
}

//Feeds synchronous input events to the supplied mapper. wparam and lparam are to be taken directly from the Win32 message loop.
int RawInputGamepad::FeedSyncInput(UINT msg, WPARAM wparam, LPARAM lparam, InputPipeline& inputPipeline) {
	
	FlushDeviceEvents(inputPipeline);

	switch(msg) {
	case WM_ACTIVATE:
	{
		//Window has been deactivated. Release any held keys.
		if(LOWORD(wparam) == WA_INACTIVE) {
			for(auto const &pad : gamepadDevices) {
				inputPipeline.FeedDeviceEvent(pad.info.virtualDevice, DeviceState::FocusLost);
			}
		}
		else if (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE) {
			for (auto const& pad : gamepadDevices) {
				inputPipeline.FeedDeviceEvent(pad.info.virtualDevice, DeviceState::FocusGained);
			}
		}
		//Don't tell that the message has been processed. Someone outside may want it too!
		return 0;
	}
	
	case WM_INPUT_DEVICE_CHANGE:
	case WM_DEVICECHANGE:
	{
		if(wparam == GIDC_ARRIVAL) {
			EnumerateGamepads(nullptr);
		}
		else if(wparam == GIDC_REMOVAL) {
			ReleaseDeadGamepads();
		}
		else if(wparam == 0x0007 /*DBT_DEVNODES_CHANGED*/) {
			ReleaseDeadGamepads();
			EnumerateGamepads(nullptr);
		}

		FlushDeviceEvents(inputPipeline);
		break;
	}

	case WM_INPUT:
	{
		//Read the header of the raw input data only
		RAWINPUTHEADER rawHeader{};
		UINT headerSize = sizeof(RAWINPUTHEADER);
		GetRawInputData((HRAWINPUT)lparam, RID_HEADER, &rawHeader, &headerSize, sizeof(RAWINPUTHEADER));
					 
		if(rawHeader.dwType == RIM_TYPEHID) {
			//Find the gamepad data that matches the device sending the report
			for(auto &pad : gamepadDevices) {
				if(pad.hidInfo.hDevice == rawHeader.hDevice) {
					return pad.FeedSyncInput((HRAWINPUT)lparam, inputPipeline);
				}
			}
		}

		return 0;
	}
	
	default:
		return 0;
	}

	return 0;
}

RawInputGamepad::GamepadDeviceData* RawInputGamepad::FindGamepad(GamepadId gamepadId) {
	for (auto& pad : gamepadDevices) {
		if (pad.info.hardwareId == gamepadId) {
			return &pad;
		}
	}

	return nullptr;
}

void RawInputGamepad::FlushDeviceEvents(InputPipeline& inputPipeline) {
	for (auto& event : deviceEvents) {
		inputPipeline.FeedDeviceEvent(event.first, event.second);
	}
	deviceEvents.clear();
}



#ifdef _MSC_VER
#pragma warning( pop )
#endif
