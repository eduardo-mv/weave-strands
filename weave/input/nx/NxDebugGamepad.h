/*
Weave NX Debug Gamepad Feeder
NxDebugGamepad.h

Implements NX Debug Gamepad support.
Debug gamepad is the debug Wii Gamepad port on the Nx sdev and only has support for a single pad, with very simple input mechanics.
*/
#pragma once
#if __has_include(<nn/hid.h>)
#include "input/system/VirtualKeys.h"
#include "input/system/VirtualDevices.h"
#include "input/system/InputPipeline.h"
#include <nn/hid.h>
#include <vector>

namespace weave {

class NxDebugGamepad  {
protected:
	//The data structure used for each gamepad device
	struct GamepadDeviceData {
		VirtualDevice vDevice = VirtualDevice::None; //The framework's virtual device assigned to this gamepad
		
		nn::hid::DebugPadState padData[2]; //Data feed structure to pool data from the system. A ring is used to keep previous and current data.
		uint32_t padDataRingIndex = 0; //Index points to the new data. Index-1 % 2 to the old data

		//Normalization values for left and right sticks
		Vector3 leftNorm {1.0f / float(nn::hid::AnalogStickMax), 1.0f / float(nn::hid::AnalogStickMax), 1.0f / float(nn::hid::AnalogStickMax)}, rightNorm {1.0f / float(nn::hid::AnalogStickMax), 1.0f / float(nn::hid::AnalogStickMax), 1.0f / float(nn::hid::AnalogStickMax)};
		float leftDeadZone = 0.10f, rightDeadZone = 0.10f;
		float leftDeadZoneSqr = 0.10f * 0.10f, rightDeadZoneSqr = 0.10f * 0.10f; //Kept for convenience

		//Cached values for digital emulation of analog sticks. This is used to send binary state directional keys emulated through the analog gamepad
		mutable Vector2 leftCap; //Tracker values to hold previous information from the pad
		mutable Vector2 rightCap;
		mutable IVector2 leftDir; //Tracker values holding the last digital position (-1, 0, 1)
		mutable IVector2 rightDir;

		GamepadDeviceData() {
		}

		//Processes pooled data from the system and generates events
		int FeedPooledInput(InputPipeline& inputPipeline);

		//Helper function that emulates digital style response with analog sticks and generates the required binary events
		//Stick: 0 -> Left, 1 -> Right
		void EmulateDigitalPad(int stick, float posx, float posy, float deltax, float deltay, VirtualDevice vDevice, InputPipeline& inputPipeline) const;
		
		//Debug functionality
		void PrintButtonPress(VirtualKey button, VirtualKeyState state) const;
	};

	//A vector with all gamepad devices found
	GamepadDeviceData pad;

public:
	NxDebugGamepad();
	virtual ~NxDebugGamepad() = default;

	//Sets the deadzone values for the gamepad sticks
	void SetDeadzones(float leftDeadzone, float rightDeadzone);

	//Sets the associated VirtualDevice for the Debug Gamepad. Only one gamepad of this type is ever present.
	int SetVirtualDevice(VirtualDevice vDev);
	//Returns the VirtualDevice name for the Debug Gamepad
	VirtualDevice GetVirtualDevice() const { return pad.vDevice; }

	//Returns the number of devices registered
	size_t GetDeviceCount() const { return 1; }

	//Initializes the related Nx library for the device. Only one debug device is ever present
	void InitializeDebugGamepad();
	
	//Feeds input from the gamepad pool
	int FeedPooledInput(InputPipeline& inputPipeline);
};

}

#endif
