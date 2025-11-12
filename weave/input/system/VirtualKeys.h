/*
Weave Input Virtual Keys
VirtualKeys.h

Defines the enums used for the virtual keys supported by the key mapping facilities
*/
#pragma once

#include <string>
#include <cstdint>
#include "VirtualDevices.h"


namespace weave::input {

	enum class VirtualKeyState : uint32_t {
		Up,
		Down,
		Hold,
		Double //Handles states like double click
	};

	enum class VirtualKey : uint32_t {
		None,
		//Cursor keys start
		//*****************
		Cursor, //Generic cursor
		Lstick, //Gamepad left stick
		Stick0 = Lstick,
		Rstick, //Gamepad right stick
		Stick1 = Rstick,
		Zaxis, //Gamepad Z axis 
		Trigger0 = Zaxis, //Gamepad generic trigger 0
		Trigger1, //Gamepad generic trigger 1
		Wheel, //Mouse wheel
		Click_Wheel = Wheel,

		_FirstCursor = Cursor,
		_LastCursor = Wheel,
		//***************
		//Cursor keys end
		

		//State keys start
		//****************
		//Gamepad
		//Gamepad analog stick digital emulation
		Lstick_North,
		Lstick_East,
		Lstick_South,
		Lstick_West,

		Stick0_North = Lstick_North,
		Stick0_East = Lstick_East,
		Stick0_South = Lstick_South,
		Stick0_West = Lstick_West,

		Rstick_North,
		Rstick_East,
		Rstick_South,
		Rstick_West,

		Stick1_North = Rstick_North,
		Stick1_East = Rstick_East,
		Stick1_South = Rstick_South,
		Stick1_West = Rstick_West,

		//Generic gamepad buttons
		Button0,
		Button1,
		Button2,
		Button3,
		Button4,
		Button5,
		Button6,
		Button7,
		Button8,
		Button9,
		Button10,
		Button11,
		Button12,
		Button13,
		Button14,
		Button15,
		Button16,
		Button17,
		Button18,
		Button19,

		_FirstPressureKey = Button0,
		_LastPressureKey = Button19,
				
		//Cross directional buttons
		Crossup, 
		North = Crossup,
		Crossupright,
		Northeast = Crossupright,
		Crossright,
		East = Crossright,
		Crossdownright,
		Southeast = Crossdownright,
		Crossdown,
		South = Crossdown,
		Crossdownleft,
		Southwest = Crossdownleft,
		Crossleft,
		West = Crossleft,
		Crossupleft,
		Northwest = Crossupleft,

		//PlayStation 4 gamepad remaps
		PS_Square = Button0,
		PS_Cross = Button1,
		PS_Circle = Button2,
		PS_Triangle = Button3,
		PS_L1 = Button4,
		PS_R1 = Button5,
		PS_L2 = Button6,
		PS_R2 = Button7,
		PS_L2Trigger = Trigger0,
		PS_R2Trigger = Trigger1,
		PS_TrackpadKey = Button8,
		PS_Options = Button9,
		PS_Start = Button9,
		PS_L3 = Button10,
		PS_R3 = Button11,
		PS_System = Button12,
		PS_Share = Button13, //Not accessible to devs
		PS_Select = Button13, //Not accessible to devs
		PS_Microphone = Button14, //PS5 mic
		PS_Up = North,
		PS_Down = South,
		PS_Left = West,
		PS_Right = East,
		PS_UpRight = Northeast,
		PS_DownRight = Southwest,
		PS_DownLeft = Southeast,
		PS_UpLeft = Northwest, //Notice the last of the list is Northwest because that's the last real value generated on the previous list before remapping

		//XBox360 gamepad remaps
		XBox_A = PS_Cross,
		XBox_B = PS_Circle,
		XBox_X = PS_Square,
		XBox_Y = PS_Triangle,
		XBox_LB = PS_L1,
		XBox_RB = PS_R1,
		XBox_Back = PS_TrackpadKey,
		XBox_Start = PS_Options,
		XBox_LT = PS_L2, //XBox analog triggers are not supported because the information is not provided correctly. Digital emulation is implemented.
		XBox_RT = PS_R2,
		XBox_LTrigger = Trigger0, //These are added for completition purposes. The system will report data for each trigger, but it will mostly be unusable.
		XBox_RTrigger = Trigger1,
		XBox_L3 = PS_L3,
		XBox_R3 = PS_R3,
		XBox_Up = North,
		XBox_Down = South,
		XBox_Left = West,
		XBox_Right = East,
		XBox_UpRight = Northeast,
		XBox_DownRight = Southwest,
		XBox_DownLeft = Southeast,
		XBox_UpLeft = Northwest, //Notice the last of the list is Northwest because that's the last real value generated on the previous list before remapping

		//NX Wii debug gamepad remaps
		Wii_A = PS_Circle,
		Wii_B = PS_Cross,
		Wii_X = PS_Triangle,
		Wii_Y = PS_Square,
		Wii_L = PS_L1,
		Wii_R = PS_R1,
		Wii_Select = PS_TrackpadKey,
		Wii_Start = PS_Options,
		Wii_ZL = PS_L2, //Wii pad has no analog trigger buttons
		Wii_ZR = PS_R2,
		Wii_LTrigger = Trigger0, //These are added for completition purposes. The system will report data for each trigger, but it will mostly be unusable.
		Wii_RTrigger = Trigger1,
		Wii_Up = North,
		Wii_Down = South,
		Wii_Left = West,
		Wii_Right = East,
		Wii_UpRight = Northeast,
		Wii_DownRight = Southwest,
		Wii_DownLeft = Southeast,
		Wii_UpLeft = Northwest, //Notice the last of the list is Northwest because that's the last real value generated on the previous list before remapping

		//NX Npad gamepad remaps. Note not all buttons from the joycons match directly. More buttons at added after the remap.
		Nx_A = PS_Circle,
		Nx_B = PS_Cross,
		Nx_X = PS_Triangle,
		Nx_Y = PS_Square,
		Nx_L = PS_L1,
		Nx_R = PS_R1,
		Nx_Minus = PS_TrackpadKey,
		Nx_Plus = PS_Options,
		Nx_ZL = PS_L2, //Nx pad has no analog trigger buttons
		Nx_ZR = PS_R2,
		Nx_LTrigger = Trigger0, //These are added for completition purposes. The system will report data for each trigger, but it will mostly be unusable.
		Nx_RTrigger = Trigger1,
		Nx_LS = PS_L3,
		Nx_RS = PS_R3,
		Nx_Up = North,
		Nx_Down = South,
		Nx_Left = West,
		Nx_Right = East,
		Nx_UpRight = Northeast,
		Nx_DownRight = Southwest,
		Nx_DownLeft = Southeast,
		Nx_UpLeft = Northwest, //Notice the last of the list is Northwest because that's the last real value generated on the previous list before remapping
		Nx_LeftSL, //Nx Npad button additions. This are not mapped to anything on other gamepads and represent the SNES style L and R buttons
		Nx_LeftSR,
		Nx_RightSL,
		Nx_RightSR,
			
		_FirstGamepadKey = Lstick_North,
		_LastGamepadKey = Nx_RightSR,
		
		//Mouse
		Leftclick,
		X1click = Leftclick,
		Click_Left = Leftclick,
		Click_X1 = Leftclick,
		Rightclick,
		X2click = Rightclick,
		Click_Right = Rightclick,
		Click_X2 = Rightclick,
		Middleclick,
		X3click = Middleclick,
		Click_Middle = Middleclick,
		Click_X3 = Middleclick,
		X4click,
		Click_X4 = X4click,
		X5click,
		Click_X5 = X5click,

		_FirstMouseKey = Leftclick,
		_LastMouseKey = Click_X5,

		//Keyboard
		Control,
		Ctrl = Control,
		Alt,
		Altgr,
		Shift,
		Tab,
		Enter,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		Key_0,
		Key_1,
		Key_2,
		Key_3,
		Key_4,
		Key_5,
		Key_6,
		Key_7,
		Key_8,
		Key_9,
		Plus,
		Minus,
		Period,
		Comma,
		Vary1,
		Vary2,
		Vary3,
		Vary4,
		Vary5,
		Vary6,
		Vary7,
		Vary8,
		Vary102,
		Numpad0,
		Numpad1,
		Numpad2,
		Numpad3,
		Numpad4,
		Numpad5,
		Numpad6,
		Numpad7,
		Numpad8,
		Numpad9,
		Multiply,
		Add,
		Separator,
		Subtract,
		Decimal,
		Divide,
		Backspace,
		Capslock,
		Esc,
		Space,
		Pageup,
		Pagedown,
		End,
		Home,
		Left,
		Right,
		Up,
		Down,
		Select,
		Printscreen,
		Insert,
		Delete,
		Numlock,
		Scroll,
		Pause,
		Winapps,
		Lwin,
		Rwin,
		Clear,
		Lshift,
		Rshift,
		Lcontrol,
		Rcontrol,
		Volumemute,
		Volumedown,
		Volumeup,
		Medianext,
		Mediaprev,
		Mediastop,
		Mediaplaypause,

		_FirstKeyboardKey = Control,
		_LastKeyboardKey = Mediaplaypause,
		//State keys end

		_FirstKey = _FirstGamepadKey,
		_LastKey = _LastKeyboardKey,

		_CountKeys = (_LastKey - _FirstKey) + 1,
		_CountCursors = (_LastCursor - _FirstCursor) + 1,
		_CountPressureKeys = (_LastPressureKey - _FirstPressureKey) + 1,
		_CountGamepadKeys = (_LastGamepadKey - _FirstGamepadKey) + 1,
		_CountMouseKeys = (_LastMouseKey - _FirstMouseKey) + 1,
		_CountKeyboardKeys = (_LastKeyboardKey - _FirstKeyboardKey) + 1,
	};

	//Returns a 64bit compound of a VirtualDevice and a VirtualKey. Mostly used for hash map keys.
	inline uint64_t VirtualDeviceKeyPair(VirtualDevice dev, VirtualKey key) {
		return ((static_cast<uint64_t>(dev) << 32) | static_cast<uint64_t>(key));
	}

	//Converts a VirtualKeyState into a 64bit bitfield where the corresponding bit for that KeyState is set
	inline uint64_t VirtualKeyStateBit(VirtualKeyState state) {
		return (uint64_t(1) << static_cast<uint32_t>(state));
	}

	//Translates state names into virtual state codes
	VirtualKeyState VirtualKeyStateValue(std::string const &name);

	//Translates virtual key codes into strings
	std::string VirtualKeyStateName(VirtualKeyState key);


	//Helps translate key names into virtual key codes
	VirtualKey VirtualKeyValue(std::string const &name);

	//Translates virtual key codes into strings
	std::string VirtualKeyName(VirtualKey key);

	bool IsPushKey(VirtualKey key);
	bool IsCursorKey(VirtualKey key);
	bool IsPressureKey(VirtualKey key);
	bool IsGamepadKey(VirtualKey key);
	bool IsKeyboardKey(VirtualKey key);
	bool IsMouseKey(VirtualKey key);


	//Returns the unicode UTF8 character associated to a VirtualKey
	std::string CharacterUTF8(VirtualKey key);

	//Offsets a keyvalue by a number by casting it to uint32_t and then back to VirtualKey. Use with caution.
	inline VirtualKey OffsetKey(VirtualKey key, uint32_t offset) {
		return static_cast<VirtualKey>(static_cast<uint32_t>(key) + offset);
	}
}
