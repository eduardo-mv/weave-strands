/*
Weave Nx Npad Gamepad Feeder
NxNpadGamepad.h

Implements support for Nx Npad gamepads, including Full, Handheld, Dual and Single joycon modes on Switch.

*/
#pragma once
#if __has_include(<nn/hid.h>)
#include "input/system/VirtualKeys.h"
#include "input/system/VirtualDevices.h"
#include "input/system/InputPipeline.h"

#include <nn/hid.h>
#include <nn/hid/hid_Npad.h>
#include <nn/hid/hid_NpadJoy.h>
#include <nn/hid/hid_Vibration.h>
#include <vector>

namespace weave {

class NxNpadGamepad  {
protected:
	//The data structure used for each gamepad device
	struct GamepadDeviceData {
		nn::hid::NpadIdType npadId = nn::hid::NpadId::Handheld; //The related Npad for this gamepad
		nn::hid::VibrationDeviceHandle hVibration[8]; //8 Handles for all operating modes
		VirtualDevice vDevice = VirtualDevice::None; //The framework's virtual device assigned to this gamepad

		nn::hid::NpadFullKeyState padData[2]; //Data feed structure to pool data from the system. A ring is used to keep previous and current data.
		uint32_t padDataRingIndex = 0; //Index points to the new data. Index-1 % 2 to the old data
				
		//Sets the pad in horizontal mode, swapping the movement axis and button reports accordingly. 
		//Only works if the Npad is detected as single
		bool horizontalMode = true;

		//Normalization values for left and right sticks
		Vector3 leftNorm {1.0f / float(nn::hid::AnalogStickMax), 1.0f / float(nn::hid::AnalogStickMax), 1.0f / float(nn::hid::AnalogStickMax)}, rightNorm {1.0f / float(nn::hid::AnalogStickMax), 1.0f / float(nn::hid::AnalogStickMax), 1.0f / float(nn::hid::AnalogStickMax)};
		float leftDeadZone = 0.10f, rightDeadZone = 0.10f;
		float leftDeadZoneSqr = 0.10f * 0.10f, rightDeadZoneSqr = 0.10f * 0.10f; //Kept for convenience

		//Zero out all the values
		GamepadDeviceData() = default;

		//Initializes the data structures given an NpadId
		bool Initialize(nn::hid::NpadIdType npadId, VirtualDevice vDevice);

		//Processes pooled data from the system and generates events
		int FeedPooledInput(InputPipeline& inputPipeline);
		
		//Makes the left gamepad vibrate
		void RumbleLeft(float amplitudeLow, float freqLow, float amplitudeHigh, float freqHigh) const;
		//Makes the right gamepad vibrate
		void RumbleRight(float amplitudeLow, float freqLow, float amplitudeHigh, float freqHigh) const;
				
		//Debug functionality
		void PrintButtonPress(VirtualKey button, VirtualKeyState state) const;
	};

	//A vector with all gamepad devices found.
	std::vector<GamepadDeviceData> gamepadDevices;

public:
	NxNpadGamepad() = default;
	virtual ~NxNpadGamepad() = default;

	//Sets the deadzone values for the gamepad sticks
	void SetDeadzones(float leftDeadzone, float rightDeadzone);
	int SetDeviceDeadzones(VirtualDevice vDev, float leftDeadzone, float rightDeadzone);
	//Sets all gamepads in or out of horizontal mode
	void SetHorizontalMode(bool flag);
	//Sets a specific gamepad in or out of horizontal mode 
	void SetHorizontalMode(VirtualDevice vDev, bool flag);

	//Returns the internal numbering of a given VirtualDevice or GetDeviceCount if not found
	size_t GetDevicePosition(VirtualDevice vDev) const;
	//Returns the VirtualDevice name for the enumerated device
	VirtualDevice GetVirtualDevice(size_t num) const;
	//Returns the related NpadId for a VirtualDevice
	nn::hid::NpadIdType GetDeviceNpadId(VirtualDevice vDev) const;
	//Returns the VirtualDevice name for the related NpadId device
	VirtualDevice GetVirtualDevice(nn::hid::NpadIdType npadId) const;

	//Returns the number of devices registered
	size_t GetDeviceCount() { return gamepadDevices.size(); }

	//Makes the left gamepad vibrate
	void RumbleLeft(VirtualDevice vDev, float amplitudeLow, float freqLow, float amplitudeHigh, float freqHigh) const;
	//Makes the right gamepad vibrate
	void RumbleRight(VirtualDevice vDev, float amplitudeLow, float freqLow, float amplitudeHigh, float freqHigh) const;

	//Swaps two virtual device IDs so that their hardware counterparts report events through those names
	void SwapVirtualDevices(VirtualDevice vDeviceA, VirtualDevice vDeviceB);

	//Initializes the related Nx library for the devices and creates the Nx data structures and Npad bindings
	size_t EnumerateGamepads(bool handHeld = true, uint32_t padCount = 8, nn::hid::NpadStyleSet supportedStyles = 
							 nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleJoyDual::Mask | nn::hid::NpadStyleHandheld::Mask | nn::hid::NpadStyleJoyLeft::Mask | nn::hid::NpadStyleJoyRight::Mask);

	//Feeds pooled input for all Nx gamepads
	int FeedPooledInput(InputPipeline& inputPipeline);
};
	

}
#endif
