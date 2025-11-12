#if __has_include(<nn/hid.h>)

#include "NxNpadGamepad.h"
#include <cassert>

//#define _DEBUG_GAMEPAD_ANALOG_OUTPUT 1
//#define _DEBUG_GAMEPAD_DIGITAL_OUTPUT 1

using namespace weave;

//Sets the deadzone values for the gamepad sticks
int weave::NxNpadGamepad::SetDeviceDeadzones(VirtualDevice vDev, float leftDeadzone, float rightDeadzone) {
	//Find the device
	for(auto &pad : gamepadDevices) {
		if(pad.vDevice == vDev) {
			pad.leftDeadZone = leftDeadzone;
			pad.leftDeadZoneSqr = pad.leftDeadZone * pad.leftDeadZone;
			pad.rightDeadZone = rightDeadzone;
			pad.rightDeadZoneSqr = pad.rightDeadZone * pad.rightDeadZone;
			return 1;
		}
	}

	return 0;
}

//Sets all gamepads in or out of horizontal mode
void weave::NxNpadGamepad::SetHorizontalMode(bool flag) {
	for(auto &pad : gamepadDevices) {
		pad.horizontalMode = flag;
	}

	nn::hid::SetNpadJoyHoldType(flag ? nn::hid::NpadJoyHoldType::NpadJoyHoldType_Horizontal : nn::hid::NpadJoyHoldType::NpadJoyHoldType_Vertical);
}

//Sets a specific gamepad in or out of horizontal mode 
void weave::NxNpadGamepad::SetHorizontalMode(VirtualDevice vDev, bool flag) {
	for(auto &pad : gamepadDevices) {
		if(pad.vDevice == vDev) {
			pad.horizontalMode = flag;
			break;
		}
	}
}

void weave::NxNpadGamepad::SetDeadzones(float leftDeadzone, float rightDeadzone) {
	//Find the device
	for(auto &pad : gamepadDevices) {
		pad.leftDeadZone = leftDeadzone;
		pad.leftDeadZoneSqr = pad.leftDeadZone * pad.leftDeadZone;
		pad.rightDeadZone = rightDeadzone;
		pad.rightDeadZoneSqr = pad.rightDeadZone * pad.rightDeadZone;
	}
}

//Returns the internal numbering of a given VirtualDevice or -1 if not found
size_t NxNpadGamepad::GetDevicePosition(VirtualDevice vDev) const {
	size_t i = 0;
	for(auto const &pad : gamepadDevices) {
		if(pad.vDevice == vDev) {
			return i;
		}
		++i;
	}

	return i;
}

//Returns the VirtualDevice name for the enumerated device
VirtualDevice NxNpadGamepad::GetVirtualDevice(size_t num) const {
	if(num >= gamepadDevices.size()) {
		return VirtualDevice::None;
	}

	return gamepadDevices[num].vDevice;
}

//Returns the related NpadId for a VirtualDevice
nn::hid::NpadIdType NxNpadGamepad::GetDeviceNpadId(VirtualDevice vDev) const {
	for(auto const &pad : gamepadDevices) {
		if(pad.vDevice == vDev) {
			return pad.npadId;
		}
	}

	return nn::hid::NpadId::Handheld; //Should we return an invalid handle? It's dangerous if later used on Nx libs
	//return nn::hid::NpadIdType {uint32_t(-1)};
}

//Returns the VirtualDevice name for the related NpadId device
VirtualDevice NxNpadGamepad::GetVirtualDevice(nn::hid::NpadIdType npadId) const {
	for(auto const &pad : gamepadDevices) {
		if(pad.npadId == npadId) {
			return pad.vDevice;
		}
	}

	return VirtualDevice::None;
}

//Makes the left gamepad vibrate
void weave::NxNpadGamepad::RumbleLeft(VirtualDevice vDev, float amplitudeLow, float freqLow, float amplitudeHigh, float freqHigh) const {
	for(auto const &pad : gamepadDevices) {
		if(pad.vDevice == vDev) {
			pad.RumbleLeft(amplitudeLow, freqLow, amplitudeHigh, freqHigh);
		}
	}
}

//Makes the right gamepad vibrate
void weave::NxNpadGamepad::RumbleRight(VirtualDevice vDev, float amplitudeLow, float freqLow, float amplitudeHigh, float freqHigh) const {
	for(auto const &pad : gamepadDevices) {
		if(pad.vDevice == vDev) {
			pad.RumbleRight(amplitudeLow, freqLow, amplitudeHigh, freqHigh);
		}
	}
}

//Swaps two virtual device IDs so that their hardware counterparts report events through those names
//If one of the two virtual devices is not assigned the swap will release the virtual device and acquire the second one
void NxNpadGamepad::SwapVirtualDevices(VirtualDevice vDeviceA, VirtualDevice vDeviceB) {
	//Look for the mouse on the current list of mice for the devices
	size_t locA = gamepadDevices.size();
	size_t locB = gamepadDevices.size();

	size_t i = 0;
	for(auto const &dev : gamepadDevices) {
		if(dev.vDevice == vDeviceA) {
			locA = i;
		}
		else if(dev.vDevice == vDeviceB) {
			locB = i;
		}
		++i;
	}

	//Swap the devices, freeing any device not found
	if(locA < gamepadDevices.size()) {
		gamepadDevices[locA].vDevice = vDeviceB;
	}

	if(locB < gamepadDevices.size()) {
		gamepadDevices[locB].vDevice = vDeviceA;
	}
}

//Initializes the related Nx library for the devices and creates the Nx data structures and Npad bindings
size_t NxNpadGamepad::EnumerateGamepads(bool handHeld, uint32_t padCount, nn::hid::NpadStyleSet supportedStyles) {
	//Initializes the Npad library
	nn::hid::InitializeNpad();
	//Set the style of operation to use. We support all of them
	nn::hid::SetSupportedNpadStyleSet(supportedStyles);
	//Set the support Npads. We support all of them
	nn::hid::NpadIdType npadIds[] =
	{
		nn::hid::NpadId::No1, nn::hid::NpadId::No2, nn::hid::NpadId::No3, nn::hid::NpadId::No4,
		nn::hid::NpadId::No5, nn::hid::NpadId::No6, nn::hid::NpadId::No7, nn::hid::NpadId::No8,
		nn::hid::NpadId::Handheld
	};
	
	//Prepare the desired amount of gamepads
	padCount = std::min(padCount, 8u); //Max 8 pads + handheld
	if(handHeld) {
		npadIds[padCount] = nn::hid::NpadId::Handheld;
		++padCount;
	}

	nn::hid::SetSupportedNpadIdType(npadIds, padCount);

	//Create the data for all npads and assign their Ids
	gamepadDevices.resize(padCount);
	uint32_t i = 0; 
	for(auto &npad : gamepadDevices) {
		npad.Initialize(npadIds[i], input::OffsetDevice(VirtualDevice::Gamepad, i));
		++i;
	}

	//TODO: Should we fuse all single joycons upon init?

	return gamepadDevices.size();
}

//Initializes the data structures given an NpadId
bool NxNpadGamepad::GamepadDeviceData::Initialize(nn::hid::NpadIdType npadId, VirtualDevice vDevice) {
	this->npadId = npadId;
	this->vDevice = vDevice;

	if(this->npadId == nn::hid::NpadId::Handheld) {
		this->vDevice = VirtualDevice::None; //Gamepad will be forced to Gamepad0 when pooling
	}

	//Initialize the vibration functionality
	int maxCount = 8;
	auto count = nn::hid::GetVibrationDeviceHandles(hVibration, maxCount, npadId, nn::hid::NpadStyleFullKey::Mask);
	count += nn::hid::GetVibrationDeviceHandles(&hVibration[count], maxCount - count, npadId, nn::hid::NpadStyleJoyDual::Mask);
	count += nn::hid::GetVibrationDeviceHandles(&hVibration[count], maxCount - count, npadId, nn::hid::NpadStyleHandheld::Mask);
	count += nn::hid::GetVibrationDeviceHandles(&hVibration[count], maxCount - count, npadId, nn::hid::NpadStyleJoyLeft::Mask);
	count += nn::hid::GetVibrationDeviceHandles(&hVibration[count], maxCount - count, npadId, nn::hid::NpadStyleJoyRight::Mask);
	
	nn::hid::VibrationValue v0 = nn::hid::VibrationValue::Make();
	for(int i = 0; i < count; i++) {
		nn::hid::InitializeVibrationDevice(hVibration[i]);
		nn::hid::SendVibrationValue(hVibration[i], v0);
	}

	return true;
}

namespace {
	//Helper inline function to calculate stick deltas and positions.
	//Takes into account current position, previous and deadzone for the stick. 
	//Deadzone calculations are performed to ensure the step that crosses the boundary does not report a big delta.
	//Returns true if an event should be issued with the change
	bool GetStickPositionDelta(int32_t stick_currX, int32_t stick_currY, int32_t stick_prevX, int32_t stick_prevY,
		float normX, float normY, float deadZone, float deadZoneSqr,
		float& x, float& y, float& deltax, float& deltay) {
		//Check if an update is really needed
		if (stick_currX != stick_prevX || stick_currY != stick_prevY) {
			//Calculate previous and current positions
			float prevx = (stick_prevX * normX);
			float prevy = (stick_prevY * normY);
			x = (stick_currX * normX);
			y = (stick_currY * normY);
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

	//Helper function for gamepad data extraction.
	//The Nx library seems to use a standard structure when it comes to data, but renames the structure according to the gamepad style
	//which also comes with a different overloaded pooling function. With this we simply cast to the correct mode to call the right function
	void GetNpadState(nn::hid::NpadStyleSet const& style, nn::hid::NpadFullKeyState* pOutValue, nn::hid::NpadIdType const& id) {
		if (style.Test<nn::hid::NpadStyleFullKey>()) {
			nn::hid::GetNpadState(pOutValue, id);
		}
		else if (style.Test<nn::hid::NpadStyleJoyDual>()) {
			nn::hid::GetNpadState(reinterpret_cast<nn::hid::NpadJoyDualState*>(pOutValue), id);
		}
		else if (style.Test<nn::hid::NpadStyleHandheld>()) {
			nn::hid::GetNpadState(reinterpret_cast<nn::hid::NpadHandheldState*>(pOutValue), id);
		}
		else if (style.Test<nn::hid::NpadStyleJoyLeft>()) {
			nn::hid::GetNpadState(reinterpret_cast<nn::hid::NpadJoyLeftState*>(pOutValue), id);
		}
		else if (style.Test<nn::hid::NpadStyleJoyRight>()) {
			nn::hid::GetNpadState(reinterpret_cast<nn::hid::NpadJoyRightState*>(pOutValue), id);
		}
	}
}

//Processes raw input data and generates input events from it
int NxNpadGamepad::GamepadDeviceData::FeedPooledInput(InputPipeline& inputPipeline) {

	//Get the currently enabled style of operation for this npad
	nn::hid::NpadStyleSet style = nn::hid::GetNpadStyleSet(npadId);
	if(style.IsAllOff())
		return 1;

	//Convinience references into the ring
	auto &currData = padData[padDataRingIndex];
	padDataRingIndex = (padDataRingIndex + 1) % 2;
	auto &prevData = padData[padDataRingIndex];

	//Handheld mode support is equivalent to the first gamepad
	if(this->npadId == nn::hid::NpadId::Handheld) {
		this->vDevice = VirtualDevice::Gamepad0;
	}

	//Fetch the input state
	GetNpadState(style, &currData, npadId);

	//Make sure we have a new sample and the pad is connected
	if(currData.attributes.Test<nn::hid::NpadAttribute::IsConnected>() &&
	   currData.samplingNumber != prevData.samplingNumber) {

		//Digital buttons feedback
		if(currData.buttons != prevData.buttons) {
			//28 sequential digital buttons, defined as:
			static const VirtualKey vButtons[] = {
				//Standard
				VirtualKey::Nx_A, VirtualKey::Nx_B, VirtualKey::Nx_X, VirtualKey::Nx_Y, VirtualKey::Nx_LS, VirtualKey::Nx_RS, VirtualKey::Nx_L, VirtualKey::Nx_R, VirtualKey::Nx_ZL, VirtualKey::Nx_ZR, VirtualKey::Nx_Plus, VirtualKey::Nx_Minus, 
				VirtualKey::Nx_Left, VirtualKey::Nx_Up, VirtualKey::Nx_Right, VirtualKey::Nx_Down,
				//Stick digital control pad emulation (Nx has this already embedded)
				VirtualKey::Lstick_West, VirtualKey::Lstick_North, VirtualKey::Lstick_East, VirtualKey::Lstick_South,
				VirtualKey::Rstick_West, VirtualKey::Rstick_North, VirtualKey::Rstick_East, VirtualKey::Rstick_South,
				//Extended Nx SNES style L and R for both controllers
				VirtualKey::Nx_LeftSL, VirtualKey::Nx_LeftSR, VirtualKey::Nx_RightSL, VirtualKey::Nx_RightSR
			};
			//Single joycon mode: horizontal remaps
			static const VirtualKey vButtonsSingleHorizontal[] = {
				//Standard (horizontal remap)
				VirtualKey::Nx_B, VirtualKey::Nx_Y, VirtualKey::Nx_A, VirtualKey::Nx_X, VirtualKey::Nx_LS, VirtualKey::Nx_LS, VirtualKey::Nx_L, VirtualKey::Nx_L, VirtualKey::Nx_ZL, VirtualKey::Nx_ZL, VirtualKey::Nx_Minus, VirtualKey::Nx_Minus, 
				VirtualKey::Nx_B, VirtualKey::Nx_Y, VirtualKey::Nx_X, VirtualKey::Nx_A,
				//Stick digital control pad emulation (Nx has this already embedded)
				VirtualKey::Lstick_South, VirtualKey::Lstick_West, VirtualKey::Lstick_North, VirtualKey::Lstick_East,
				VirtualKey::Lstick_North, VirtualKey::Lstick_East, VirtualKey::Lstick_South, VirtualKey::Lstick_West,
				//Extended Nx SNES style L and R for both controllers
				VirtualKey::Nx_LeftSL, VirtualKey::Nx_LeftSR, VirtualKey::Nx_LeftSL, VirtualKey::Nx_LeftSR
			};

			static const int numButtons = sizeof(vButtons) / sizeof(VirtualKey);

			for(int i = 0; i < numButtons; ++i) {
				if(currData.buttons[i] != prevData.buttons[i]) {
					VirtualKey key = (horizontalMode && (style.Test<nn::hid::NpadStyleJoyLeft>() || style.Test<nn::hid::NpadStyleJoyRight>()) ? vButtonsSingleHorizontal[i] : vButtons[i]);
					//Button state changed
					inputPipeline.FeedKeyEvent(key, (currData.buttons.Test(i) ? VirtualKeyState::Down : VirtualKeyState::Up), vDevice);
#ifdef _DEBUG_GAMEPAD_DIGITAL_OUTPUT
					PrintButtonPress(key, (currData.buttons.Test(i) ? VirtualKeyState::Down : VirtualKeyState::Up));
#endif
				}
			}
		}

		//Stick movement
		float x, y, deltax, deltay;
		//Left
		bool sendEvent =
			GetStickPositionDelta(currData.analogStickL.x, currData.analogStickL.y, //Current position of the stick
								  prevData.analogStickL.x, prevData.analogStickL.y, //Previous position
								  leftNorm.x, leftNorm.y, //Normalization values
								  leftDeadZone, leftDeadZoneSqr, //Deadzone values
								  x, y, deltax, deltay); //Output position and delta

		if(sendEvent) {
			//In horizontal mode we must swap one axis (only used in single mode)
			if(horizontalMode && style.Test<nn::hid::NpadStyleJoyLeft>()) {
				std::swap(x, y);
				std::swap(deltax, deltay);
				x = -x;
				deltax = -deltax;
			}
			inputPipeline.FeedCursorPositionAndDelta(x, y, 0.0f, deltax, deltay, 0.0f, VirtualKey::Lstick, vDevice);
		}

		//Right
		sendEvent =
			GetStickPositionDelta(currData.analogStickR.x, currData.analogStickR.y, //Current position of the stick
								  prevData.analogStickR.x, prevData.analogStickR.y, //Previous position
								  rightNorm.x, rightNorm.y, //Normalization values
								  rightDeadZone, rightDeadZoneSqr, //Deadzone value
								  x, y, deltax, deltay); //Output position and delta

		if(sendEvent) {
			//In single joycon mode, we report Lstick instead of Rstick
			VirtualKey stick = VirtualKey::Rstick;
			if(style.Test<nn::hid::NpadStyleJoyRight>()) {
				stick = VirtualKey::Lstick;

				//In horizontal mode we must swap one axis
				if(horizontalMode) {
					std::swap(x, y);
					std::swap(deltax, deltay);
					y = -y;
					deltay = -deltay;
				}
			}
			inputPipeline.FeedCursorPositionAndDelta(x, y, 0.0f, deltax, deltay, 0.0f, stick, vDevice);
		}
	}

	if(this->npadId == nn::hid::NpadId::Handheld) {
		this->vDevice = VirtualDevice::None;
	}

	return 1;
}


//Makes the left gamepad vibrate
void weave::NxNpadGamepad::GamepadDeviceData::RumbleLeft(float amplitudeLow, float freqLow, float amplitudeHigh, float freqHigh) const {
	nn::hid::VibrationValue rumble = nn::hid::VibrationValue::Make(amplitudeLow, freqLow, amplitudeHigh, freqHigh);
	nn::hid::VibrationDeviceInfo info;
	
	for(int i = 0; i < 8; ++i) {
		nn::hid::GetVibrationDeviceInfo(&info, hVibration[i]);
		if(info.position == nn::hid::VibrationDevicePosition_Left) {
			nn::hid::SendVibrationValue(hVibration[i], rumble);
		}
	}
}

//Makes the right gamepad vibrate
void weave::NxNpadGamepad::GamepadDeviceData::RumbleRight(float amplitudeLow, float freqLow, float amplitudeHigh, float freqHigh) const {
	nn::hid::VibrationValue rumble = nn::hid::VibrationValue::Make(amplitudeLow, freqLow, amplitudeHigh, freqHigh);
	nn::hid::VibrationDeviceInfo info;

	for(int i = 0; i < 8; ++i) {
		nn::hid::GetVibrationDeviceInfo(&info, hVibration[i]);
		if(info.position == nn::hid::VibrationDevicePosition_Right) {
			nn::hid::SendVibrationValue(hVibration[i], rumble);
		}
	}
}

//Debug functionality
void weave::NxNpadGamepad::GamepadDeviceData::PrintButtonPress(VirtualKey button, VirtualKeyState state) const {
	WLOG(WLog::Info) << input::VirtualKeyName(button) << " [" << input::VirtualKeyStateName(state) << "]";
}


//------------------------
//Feeds synchronous input events to the supplied mapper. wparam and lparam are to be taken directly from the Win32 message loop.
int NxNpadGamepad::FeedPooledInput(InputPipeline& inputPipeline) {
	for(auto &pad : gamepadDevices) {
		pad.FeedPooledInput(inputPipeline);
	}

	return 1;
}

#endif
