#if __has_include(<nn/hid.h>)

#include "NxDebugGamepad.h"
#include <cassert>

//#define _DEBUG_GAMEPAD_ANALOG_OUTPUT 1
//#define _DEBUG_GAMEPAD_DIGITAL_OUTPUT 1

using namespace weave;

NxDebugGamepad::NxDebugGamepad() {
	
}

//Sets the deadzone values for the gamepad sticks
void weave::NxDebugGamepad::SetDeadzones(float leftDeadzone, float rightDeadzone) {
	pad.leftDeadZone = leftDeadzone;
	pad.leftDeadZoneSqr = pad.leftDeadZone * pad.leftDeadZone;
	pad.rightDeadZone = rightDeadzone;
	pad.rightDeadZoneSqr = pad.rightDeadZone * pad.rightDeadZone;
}

//Sets the associated VirtualDevice for the Debug Gamepad. Only one gamepad of this type is ever present.
int weave::NxDebugGamepad::SetVirtualDevice(VirtualDevice vDev) {
	pad.vDevice = vDev;

	return 1;
}

//Initializes the related Nx library for the device. Only one debug device is ever present
void NxDebugGamepad::InitializeDebugGamepad() {
	nn::hid::InitializeDebugPad();
	pad.vDevice = VirtualDevice::Gamepad0;
}

//Helper inline function to calculate stick deltas and positions.
//Takes into account current position, previous and deadzone for the stick. 
//Deadzone calculations are performed to ensure the step that crosses the boundary does not report a big delta.
//Returns true if an event should be issued with the change
inline
bool GetStickPositionDelta(int32_t stick_currX, int32_t stick_currY, int32_t stick_prevX, int32_t stick_prevY,
						   float normX, float normY, float deadZone, float deadZoneSqr,
						   float &x, float &y, float &deltax, float &deltay) {
	//Check if an update is really needed
	if(stick_currX != stick_prevX || stick_currY != stick_prevY) {
		//Calculate previous and current positions
		float prevx = (stick_prevX * normX);
		float prevy = (stick_prevY * normY);
		x = (stick_currX * normX);
		y = (stick_currY * normY);
		//Calculate the delta before deadzone adjustment so that we don't show a big jump when crossing the deadzone boundary 
		deltax = x - prevx;
		deltay = y - prevy;
		//Clamp to deadzone values and normalize the position of the stick into the active range 
		x = (x*x > deadZoneSqr) ? (x - copysignf(deadZone, x)) / (1.0f - deadZone) : 0.0f;
		y = (y*y > deadZoneSqr) ? (y - copysignf(deadZone, y)) / (1.0f - deadZone) : 0.0f;
		prevx = (prevx*prevx > deadZoneSqr) ? (prevx - copysignf(deadZone, x)) / (1.0f - deadZone) : 0.0f;
		prevy = (prevy*prevy > deadZoneSqr) ? (prevy - copysignf(deadZone, y)) / (1.0f - deadZone) : 0.0f;

		//We only need to emit an event if previous position and current are different
		return (prevx != x || prevy != y);
	}

	return false;
}

//Processes raw input data and generates input events from it
int NxDebugGamepad::GamepadDeviceData::FeedPooledInput(InputPipeline& inputPipeline) {

	//Convinience references into the ring
	auto &currData = padData[padDataRingIndex];
	padDataRingIndex = (padDataRingIndex + 1) % 2;
	auto &prevData = padData[padDataRingIndex];

	//Fetch the input state
	nn::hid::GetDebugPadState(&currData);
	//Make sure we have a new sample and the pad is connected
	if(currData.attributes.Test<nn::hid::DebugPadAttribute::IsConnected>() &&
	   currData.samplingNumber != prevData.samplingNumber) {
		
		//Digital buttons feedback
		if(currData.buttons != prevData.buttons) {
			//14 sequential digital buttons, defined as:
			static const VirtualKey vButtons[] = {VirtualKey::Wii_A, VirtualKey::Wii_B, VirtualKey::Wii_X, VirtualKey::Wii_Y, VirtualKey::Wii_L, VirtualKey::Wii_R, VirtualKey::Wii_ZL, VirtualKey::Wii_ZR, VirtualKey::Wii_Start, VirtualKey::Wii_Select, VirtualKey::Wii_Left, VirtualKey::Wii_Up, VirtualKey::Wii_Right, VirtualKey::Wii_Down};
			static const int numButtons = sizeof(vButtons) / sizeof(VirtualKey);

			for(int i = 0; i < numButtons; ++i) {
				if(currData.buttons[i] != prevData.buttons[i]) {
					//Button state changed
					inputPipeline.FeedKeyEvent(vButtons[i], (currData.buttons.Test(i) ? VirtualKeyState::Down : VirtualKeyState::Up), vDevice);
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
			inputPipeline.FeedCursorPositionAndDelta(x, y, 0.0f, deltax, deltay, 0.0f, VirtualKey::Lstick, vDevice);
			EmulateDigitalPad(0, x, y, deltax, deltay, vDevice, inputPipeline);
		}

		//Right
		sendEvent =
			GetStickPositionDelta(currData.analogStickR.x, currData.analogStickR.y, //Current position of the stick
								  prevData.analogStickR.x, prevData.analogStickR.y, //Previous position
								  rightNorm.x, rightNorm.y, //Normalization values
								  rightDeadZone, rightDeadZoneSqr, //Deadzone value
								  x, y, deltax, deltay); //Output position and delta

		if(sendEvent) {
			inputPipeline.FeedCursorPositionAndDelta(x, y, 0.0f, deltax, deltay, 0.0f, VirtualKey::Rstick, vDevice);
			EmulateDigitalPad(1, x, y, deltax, deltay, vDevice, inputPipeline);
		}
	}
	
	return 1;
}

//Helper function that emulates digital style response with analog sticks and generates the required binary events
//Stick: 0 -> Left, 1 -> Right
void weave::NxDebugGamepad::GamepadDeviceData::EmulateDigitalPad(int stick, float posx, float posy, float deltax, float deltay, VirtualDevice vDevice, InputPipeline& inputPipeline) const {

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
			inputPipeline.FeedKeyEvent(stick == 0 ? VirtualKey::Lstick_North : VirtualKey::Rstick_North, VirtualKeyState::Down, vDevice);
			dir.y = 1;
		}
		if(dir.y == 1 && posy > cap.y) {
			cap.y = posy;
		}

		//Release pulldown detection
		if(dir.y == -1 && posy - cap.y > responseDelta) {
			//Send South release (Up) event here
			inputPipeline.FeedKeyEvent(stick == 0 ? VirtualKey::Lstick_South : VirtualKey::Rstick_South, VirtualKeyState::Up, vDevice);
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
			inputPipeline.FeedKeyEvent(stick == 0 ? VirtualKey::Lstick_South : VirtualKey::Rstick_South, VirtualKeyState::Down, vDevice);
			dir.y = -1;
		}

		if(dir.y == -1 && posy < cap.y) {
			cap.y = posy;
		}

		//Release pullup detection
		if(dir.y == 1 && cap.y - posy > responseDelta) {
			//Send North release (Up) event here
			inputPipeline.FeedKeyEvent(stick == 0 ? VirtualKey::Lstick_North : VirtualKey::Rstick_North, VirtualKeyState::Up, vDevice);
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
			inputPipeline.FeedKeyEvent(stick == 0 ? VirtualKey::Lstick_East : VirtualKey::Rstick_East, VirtualKeyState::Down, vDevice);
			dir.x = 1;
		}
		if(dir.x == 1 && posx > cap.x) {
			cap.x = posx;
		}

		//Release pulldown detection
		if(dir.x == -1 && posx - cap.x > responseDelta) {
			//Send West release (Up) event here
			inputPipeline.FeedKeyEvent(stick == 0 ? VirtualKey::Lstick_West : VirtualKey::Rstick_West, VirtualKeyState::Up, vDevice);
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
			inputPipeline.FeedKeyEvent(stick == 0 ? VirtualKey::Lstick_West : VirtualKey::Rstick_West, VirtualKeyState::Down, vDevice);
			dir.x = -1;
		}

		if(dir.x == -1 && posx < cap.x) {
			cap.x = posx;
		}

		//Release pullup detection
		if(dir.x == 1 && cap.x - posx > responseDelta) {
			//Send East release (Up) event here
			inputPipeline.FeedKeyEvent(stick == 0 ? VirtualKey::Lstick_East : VirtualKey::Rstick_East, VirtualKeyState::Up, vDevice);
			dir.x = 0;
			cap.x = 0.0f;
		}
		if(dir.x == 0 && cap.x > posx && posx > 0.0f) {
			cap.x = posx;
		}
	}
}

//Debug functionality
void weave::NxDebugGamepad::GamepadDeviceData::PrintButtonPress(VirtualKey button, VirtualKeyState state) const {
	//TODO: Figure this out
	//WLOG(WLog::Info) << input::VirtualKeyName(button) << " [" << input::VirtualKeyStateName(state) << "]";
}


//------------------------
//Feeds input from the gamepad pool
int NxDebugGamepad::FeedPooledInput(InputPipeline& inputPipeline) {
	return pad.FeedPooledInput(inputPipeline);
}

#endif
