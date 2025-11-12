#include "VirtualKeys.h"
#include <unordered_map>
#include <algorithm>
#ifdef _WIN32
#include "weave/input/win32/Win32VirtualKeys.h"
#endif

using namespace weave::input;


namespace {
//Convert all strings into lowercase
std::string fixAttribute(std::string attribute) {
	if(attribute.empty()) {
		return "none";
	}

	std::replace(attribute.begin(), attribute.end(), ',', ' ');
	std::transform(attribute.begin(), attribute.end(), attribute.begin(), [](char c) { return (char)::tolower((int)c); });
	return attribute;
};
}

//Translates state names into virtual state codes
VirtualKeyState weave::input::VirtualKeyStateValue(std::string const &name) {
#define macroTable(n) {fixAttribute(#n), VirtualKeyState::n},

	static std::unordered_map<std::string, VirtualKeyState> map = {
		macroTable(Up)
		macroTable(Down)
		macroTable(Hold)
		macroTable(Double)
	};

#undef macroTable
	
	return map[fixAttribute(name)];
}



//Helps translate key names into virtual key codes
VirtualKey weave::input::VirtualKeyValue(std::string const &name) {
#define macroTable(n) {fixAttribute(#n), VirtualKey::n},
	
	static std::unordered_map<std::string, VirtualKey> map = {
		//Generic mouse cursor
		macroTable(None)
		macroTable(Cursor)

		//Generic gamepad analogs
		macroTable(Lstick)
		macroTable(Rstick)
		macroTable(Stick0)
		macroTable(Stick1)
		macroTable(Zaxis)
		macroTable(Trigger0)
		macroTable(Trigger1)

		//Generic gamepad digital emulation for analog sticks
		macroTable(Lstick_North)
		macroTable(Lstick_East)
		macroTable(Lstick_South)
		macroTable(Lstick_West)

		macroTable(Stick0_North)
		macroTable(Stick0_East)
		macroTable(Stick0_South)
		macroTable(Stick0_West)

		macroTable(Rstick_North)
		macroTable(Rstick_East)
		macroTable(Rstick_South)
		macroTable(Rstick_West)

		macroTable(Stick1_North)
		macroTable(Stick1_East)
		macroTable(Stick1_South)
		macroTable(Stick1_West)

		//Generic gamepad buttons
		macroTable(Button0)
		macroTable(Button1)
		macroTable(Button2)
		macroTable(Button3)
		macroTable(Button4)
		macroTable(Button5)
		macroTable(Button6)
		macroTable(Button7)
		macroTable(Button8)
		macroTable(Button9)
		macroTable(Button10)
		macroTable(Button11)
		macroTable(Button12)
		macroTable(Button13)
		macroTable(Button14)
		macroTable(Button15)
		macroTable(Button16)
		macroTable(Button17)
		macroTable(Button18)
		macroTable(Button19)
		macroTable(North)
		macroTable(Northeast)
		macroTable(East)
		macroTable(Southeast)
		macroTable(South)
		macroTable(Southwest)
		macroTable(West)
		macroTable(Northwest)

		//PS4 remaps
		macroTable(PS_Square)
		macroTable(PS_Cross)
		macroTable(PS_Circle)
		macroTable(PS_Triangle)
		macroTable(PS_L1)
		macroTable(PS_R1)
		macroTable(PS_L2)
		macroTable(PS_R2)
		macroTable(PS_L2Trigger)
		macroTable(PS_R2Trigger)
		macroTable(PS_Share)
		macroTable(PS_Options)
		macroTable(PS_L3)
		macroTable(PS_R3)
		macroTable(PS_System)
		macroTable(PS_TrackpadKey)
		macroTable(PS_Up)
		macroTable(PS_Down)
		macroTable(PS_Left)
		macroTable(PS_Right)
		macroTable(PS_UpLeft)
		macroTable(PS_UpRight)
		macroTable(PS_DownRight)
		macroTable(PS_DownLeft)

		//XBox remaps
		macroTable(XBox_A)
		macroTable(XBox_B)
		macroTable(XBox_X)
		macroTable(XBox_Y)
		macroTable(XBox_LB)
		macroTable(XBox_RB)
		macroTable(XBox_Back)
		macroTable(XBox_Start)
		macroTable(XBox_LT)
		macroTable(XBox_RT)
		macroTable(XBox_L3)
		macroTable(XBox_R3)
		macroTable(XBox_Up)
		macroTable(XBox_Down)
		macroTable(XBox_Left)
		macroTable(XBox_Right)
		macroTable(XBox_UpLeft)
		macroTable(XBox_UpRight)
		macroTable(XBox_DownRight)
		macroTable(XBox_DownLeft)

		//Wii (debug Nx) remaps
		macroTable(Wii_A)
		macroTable(Wii_B)
		macroTable(Wii_X)
		macroTable(Wii_Y)
		macroTable(Wii_L)
		macroTable(Wii_R)
		macroTable(Wii_Select)
		macroTable(Wii_Start)
		macroTable(Wii_ZL)
		macroTable(Wii_ZR)
		macroTable(Wii_LTrigger)
		macroTable(Wii_RTrigger)
		macroTable(Wii_Up)
		macroTable(Wii_Down)
		macroTable(Wii_Left)
		macroTable(Wii_Right)
		macroTable(Wii_UpRight)
		macroTable(Wii_DownRight)
		macroTable(Wii_DownLeft)
		macroTable(Wii_UpLeft)

		//Nx remaps + exclusives
		macroTable(Nx_A)
		macroTable(Nx_B)
		macroTable(Nx_X)
		macroTable(Nx_Y)
		macroTable(Nx_L)
		macroTable(Nx_R)
		macroTable(Nx_Minus)
		macroTable(Nx_Plus)
		macroTable(Nx_ZL)
		macroTable(Nx_ZR)
		macroTable(Nx_LTrigger)
		macroTable(Nx_RTrigger)
		macroTable(Nx_LS)
		macroTable(Nx_RS)
		macroTable(Nx_Up)
		macroTable(Nx_Down)
		macroTable(Nx_Left)
		macroTable(Nx_Right)
		macroTable(Nx_UpRight)
		macroTable(Nx_DownRight)
		macroTable(Nx_DownLeft)
		macroTable(Nx_UpLeft)

		macroTable(Nx_LeftSL)
		macroTable(Nx_LeftSR)
		macroTable(Nx_RightSL)
		macroTable(Nx_RightSR)
		
		//Generic mouse keys
		macroTable(Leftclick)
		macroTable(Rightclick)
		macroTable(Middleclick)
		macroTable(X1click)
		macroTable(X2click)
		macroTable(X3click)
		macroTable(X4click)
		macroTable(X5click)
		macroTable(Wheel)
		macroTable(Click_Left)
		macroTable(Click_Right)
		macroTable(Click_Middle)
		macroTable(Click_X1)
		macroTable(Click_X2)
		macroTable(Click_X3)
		macroTable(Click_X4)
		macroTable(Click_X5)
		macroTable(Click_Wheel)

		//Generic keyboard keys
		macroTable(Control)
		macroTable(Ctrl)
		macroTable(Alt)
		macroTable(Altgr)
		macroTable(Shift)
		macroTable(Tab)
		macroTable(Enter)
		macroTable(A)
		macroTable(B)
		macroTable(C)
		macroTable(D)
		macroTable(E)
		macroTable(F)
		macroTable(G)
		macroTable(H)
		macroTable(I)
		macroTable(J)
		macroTable(K)
		macroTable(L)
		macroTable(M)
		macroTable(N)
		macroTable(O)
		macroTable(P)
		macroTable(Q)
		macroTable(R)
		macroTable(S)
		macroTable(T)
		macroTable(U)
		macroTable(V)
		macroTable(W)
		macroTable(X)
		macroTable(Y)
		macroTable(Z)
		macroTable(F1)
		macroTable(F2)
		macroTable(F3)
		macroTable(F4)
		macroTable(F5)
		macroTable(F6)
		macroTable(F7)
		macroTable(F8)
		macroTable(F9)
		macroTable(F10)
		macroTable(F11)
		macroTable(F12)
		macroTable(Key_0)
		macroTable(Key_1)
		macroTable(Key_2)
		macroTable(Key_3)
		macroTable(Key_4)
		macroTable(Key_5)
		macroTable(Key_6)
		macroTable(Key_7)
		macroTable(Key_8)
		macroTable(Key_9)
		macroTable(Plus)
		macroTable(Minus)
		macroTable(Period)
		macroTable(Comma)
		macroTable(Vary1)
		macroTable(Vary2)
		macroTable(Vary3)
		macroTable(Vary4)
		macroTable(Vary5)
		macroTable(Vary6)
		macroTable(Vary7)
		macroTable(Vary8)
		macroTable(Vary102)
		macroTable(Numpad0)
		macroTable(Numpad1)
		macroTable(Numpad2)
		macroTable(Numpad3)
		macroTable(Numpad4)
		macroTable(Numpad5)
		macroTable(Numpad6)
		macroTable(Numpad7)
		macroTable(Numpad8)
		macroTable(Numpad9)
		macroTable(Multiply)
		macroTable(Add)
		macroTable(Separator)
		macroTable(Subtract)
		macroTable(Decimal)
		macroTable(Divide)
		macroTable(Backspace)
		macroTable(Capslock)
		macroTable(Esc)
		macroTable(Space)
		macroTable(Pageup)
		macroTable(Pagedown)
		macroTable(End)
		macroTable(Home)
		macroTable(Left)
		macroTable(Right)
		macroTable(Up)
		macroTable(Down)
		macroTable(Select)
		macroTable(Printscreen)
		macroTable(Insert)
		macroTable(Delete)
		macroTable(Numlock)
		macroTable(Scroll)
		macroTable(Pause)
		macroTable(Winapps)
		macroTable(Lwin)
		macroTable(Rwin)
		macroTable(Clear)
		macroTable(Lshift)
		macroTable(Rshift)
		macroTable(Lcontrol)
		macroTable(Rcontrol)
		macroTable(Middleclick)
		macroTable(Volumemute)
		macroTable(Volumedown)
		macroTable(Volumeup)
		macroTable(Medianext)
		macroTable(Mediaprev)
		macroTable(Mediastop)
		macroTable(Mediaplaypause)
	};

#undef macroTable

	return map[fixAttribute(name.c_str())];
}




//Translates virtual key codes into strings
std::string weave::input::VirtualKeyStateName(VirtualKeyState key) {
#define macroTable(n) case VirtualKeyState::n: return #n;

	switch(key) {
		macroTable(Up)
		macroTable(Down)
		macroTable(Hold)
		macroTable(Double)
	default:
		return "";
	}

#undef macroTable
}



//Translates virtual key codes into strings
std::string weave::input::VirtualKeyName(VirtualKey key) {
#define macroTable(n) case VirtualKey::n: return #n;

	switch(key) {
		macroTable(None)
		macroTable(Cursor)
		macroTable(Click_Left)
		macroTable(Click_Right)
		macroTable(Click_Middle)
		macroTable(Click_X4)
		macroTable(Click_X5)
		macroTable(Click_Wheel)

		macroTable(Lstick_North)
		macroTable(Lstick_East)
		macroTable(Lstick_South)
		macroTable(Lstick_West)

		macroTable(Rstick_North)
		macroTable(Rstick_East)
		macroTable(Rstick_South)
		macroTable(Rstick_West)

		macroTable(Lstick)
		macroTable(Rstick)
		macroTable(Trigger0)
		macroTable(Trigger1)
		macroTable(Button0)
		macroTable(Button1)
		macroTable(Button2)
		macroTable(Button3)
		macroTable(Button4)
		macroTable(Button5)
		macroTable(Button6)
		macroTable(Button7)
		macroTable(Button8)
		macroTable(Button9)
		macroTable(Button10)
		macroTable(Button11)
		macroTable(Button12)
		macroTable(Button13)
		macroTable(Button14)
		macroTable(Button15)
		macroTable(Button16)
		macroTable(Button17)
		macroTable(Button18)
		macroTable(Button19)
		macroTable(North)
		macroTable(Northeast)
		macroTable(East)
		macroTable(Southeast)
		macroTable(South)
		macroTable(Southwest)
		macroTable(West)
		macroTable(Northwest)
		macroTable(Nx_LeftSL)
		macroTable(Nx_LeftSR)
		macroTable(Nx_RightSL)
		macroTable(Nx_RightSR)

		macroTable(Control)
		macroTable(Alt)
		macroTable(Altgr)
		macroTable(Shift)
		macroTable(Tab)
		macroTable(Enter)
		macroTable(A)
		macroTable(B)
		macroTable(C)
		macroTable(D)
		macroTable(E)
		macroTable(F)		
		macroTable(G)
		macroTable(H)
		macroTable(I)
		macroTable(J)
		macroTable(K)
		macroTable(L)
		macroTable(M)
		macroTable(N)
		macroTable(O)
		macroTable(P)
		macroTable(Q)
		macroTable(R)
		macroTable(S)
		macroTable(T)
		macroTable(U)
		macroTable(V)
		macroTable(W)
		macroTable(X)
		macroTable(Y)
		macroTable(Z)
		macroTable(F1)
		macroTable(F2)
		macroTable(F3)
		macroTable(F4)
		macroTable(F5)
		macroTable(F6)
		macroTable(F7)
		macroTable(F8)
		macroTable(F9)
		macroTable(F10)
		macroTable(F11)
		macroTable(F12)
		macroTable(Key_0)
		macroTable(Key_1)
		macroTable(Key_2)
		macroTable(Key_3)
		macroTable(Key_4)
		macroTable(Key_5)
		macroTable(Key_6)
		macroTable(Key_7)
		macroTable(Key_8)
		macroTable(Key_9)
		macroTable(Plus)
		macroTable(Minus)
		macroTable(Period)
		macroTable(Comma)
		macroTable(Vary1)
		macroTable(Vary2)
		macroTable(Vary3)
		macroTable(Vary4)
		macroTable(Vary5)
		macroTable(Vary6)
		macroTable(Vary7)
		macroTable(Vary8)
		macroTable(Vary102)
		macroTable(Numpad0)
		macroTable(Numpad1)
		macroTable(Numpad2)
		macroTable(Numpad3)
		macroTable(Numpad4)
		macroTable(Numpad5)
		macroTable(Numpad6)
		macroTable(Numpad7)
		macroTable(Numpad8)
		macroTable(Numpad9)
		macroTable(Multiply)
		macroTable(Add)
		macroTable(Separator)
		macroTable(Subtract)
		macroTable(Decimal)
		macroTable(Divide)
		macroTable(Backspace)
		macroTable(Capslock)
		macroTable(Esc)
		macroTable(Space)
		macroTable(Pageup)
		macroTable(Pagedown)
		macroTable(End)
		macroTable(Home)
		macroTable(Left)
		macroTable(Right)
		macroTable(Up)
		macroTable(Down)
		macroTable(Select)
		macroTable(Printscreen)
		macroTable(Insert)
		macroTable(Delete)
		macroTable(Numlock)
		macroTable(Scroll)
		macroTable(Pause)
		macroTable(Winapps)
		macroTable(Lwin)
		macroTable(Rwin)
		macroTable(Clear)
		macroTable(Lshift)
		macroTable(Rshift)
		macroTable(Lcontrol)
		macroTable(Rcontrol)
		macroTable(Volumemute)
		macroTable(Volumedown)
		macroTable(Volumeup)
		macroTable(Medianext)
		macroTable(Mediaprev)
		macroTable(Mediastop)
		macroTable(Mediaplaypause)
		default:
			return "";
	}

#undef macroTable
}

bool weave::input::IsPushKey(VirtualKey key) {
	return
		static_cast<uint32_t>(key) >= static_cast<uint32_t>(VirtualKey::_FirstKey) &&
		static_cast<uint32_t>(key) <= static_cast<uint32_t>(VirtualKey::_LastKey);
}

bool weave::input::IsCursorKey(VirtualKey key) {
	return
		static_cast<uint32_t>(key) >= static_cast<uint32_t>(VirtualKey::_FirstCursor) &&
		static_cast<uint32_t>(key) <= static_cast<uint32_t>(VirtualKey::_LastCursor);
}

bool weave::input::IsPressureKey(VirtualKey key) {
	return
		static_cast<uint32_t>(key) >= static_cast<uint32_t>(VirtualKey::_FirstPressureKey) &&
		static_cast<uint32_t>(key) <= static_cast<uint32_t>(VirtualKey::_LastPressureKey);
}

bool weave::input::IsGamepadKey(VirtualKey key) {
	return
		static_cast<uint32_t>(key) >= static_cast<uint32_t>(VirtualKey::_FirstGamepadKey) &&
		static_cast<uint32_t>(key) <= static_cast<uint32_t>(VirtualKey::_LastGamepadKey);
}

bool weave::input::IsKeyboardKey(VirtualKey key) {
	return
		static_cast<uint32_t>(key) >= static_cast<uint32_t>(VirtualKey::_FirstKeyboardKey) &&
		static_cast<uint32_t>(key) <= static_cast<uint32_t>(VirtualKey::_LastKeyboardKey);
}

bool weave::input::IsMouseKey(VirtualKey key) {
	return
		static_cast<uint32_t>(key) >= static_cast<uint32_t>(VirtualKey::_FirstMouseKey) &&
		static_cast<uint32_t>(key) <= static_cast<uint32_t>(VirtualKey::_LastMouseKey);
}

//Returns the unicode UTF8 character associated to a VirtualKey
std::string weave::input::CharacterUTF8(VirtualKey key) {
	(void)key;
#ifdef _WIN32
	return input::Win32CharacterUTF8(key);
#else
	//TODO non Win32 versions
	return {};
#endif
}

