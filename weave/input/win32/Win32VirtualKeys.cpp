#include "Win32VirtualKeys.h"
#include "weave/system/utf/Utf.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated wstring_convert in C++17
#endif

using namespace weave::input;

//Returns the associated VirtualKey to the supplied VK_
VirtualKey weave::input::Win32VkToVirtualKey(WPARAM vk) {
	#define macroMapKb(n1,n2) case n1: return VirtualKey::n2;

	switch(vk) {
		macroMapKb('A',A);
		macroMapKb('B',B);
		macroMapKb('C',C);
		macroMapKb('D',D);
		macroMapKb('E',E);
		macroMapKb('F',F);
		macroMapKb('G',G);
		macroMapKb('H',H);
		macroMapKb('I',I);
		macroMapKb('J',J);
		macroMapKb('K',K);
		macroMapKb('L',L);
		macroMapKb('M',M);
		macroMapKb('N',N);
		macroMapKb('O',O);
		macroMapKb('P',P);
		macroMapKb('Q',Q);
		macroMapKb('R',R);
		macroMapKb('S',S);
		macroMapKb('T',T);
		macroMapKb('U',U);
		macroMapKb('V',V);
		macroMapKb('W',W);
		macroMapKb('X',X);
		macroMapKb('Y',Y);
		macroMapKb('Z',Z);
		macroMapKb('0',Key_0);
		macroMapKb('1',Key_1);
		macroMapKb('2',Key_2);
		macroMapKb('3',Key_3);
		macroMapKb('4',Key_4);
		macroMapKb('5',Key_5);
		macroMapKb('6',Key_6);
		macroMapKb('7',Key_7);
		macroMapKb('8',Key_8);
		macroMapKb('9',Key_9);
		macroMapKb(VK_TAB,Tab);
		macroMapKb(VK_RETURN,Enter);
		macroMapKb(VK_CONTROL,Control);
		macroMapKb(VK_SHIFT,Shift);
		macroMapKb(VK_SPACE,Space);
		macroMapKb(VK_BACK,Backspace);
		macroMapKb(VK_ESCAPE,Esc);
		macroMapKb(VK_PRIOR,Pageup);
		macroMapKb(VK_NEXT,Pagedown);
		macroMapKb(VK_END,End);
		macroMapKb(VK_HOME,Home);
		macroMapKb(VK_UP,Up);
		macroMapKb(VK_DOWN,Down);
		macroMapKb(VK_LEFT,Left);
		macroMapKb(VK_RIGHT,Right);
		macroMapKb(VK_SELECT,Select);
		macroMapKb(VK_SNAPSHOT,Printscreen);
		macroMapKb(VK_INSERT,Insert);
		macroMapKb(VK_DELETE,Delete);
		macroMapKb(VK_OEM_PLUS,Plus);
		macroMapKb(VK_OEM_MINUS,Minus);
		macroMapKb(VK_OEM_COMMA,Comma);
		macroMapKb(VK_OEM_PERIOD,Period);
		macroMapKb(VK_OEM_1,Vary1);
		macroMapKb(VK_OEM_2,Vary2);
		macroMapKb(VK_OEM_3,Vary3);
		macroMapKb(VK_OEM_4,Vary4);
		macroMapKb(VK_OEM_5,Vary5);
		macroMapKb(VK_OEM_6,Vary6);
		macroMapKb(VK_OEM_7,Vary7);
		macroMapKb(VK_OEM_8,Vary8);
		macroMapKb(VK_OEM_102,Vary102);
		macroMapKb(VK_NUMPAD0,Numpad0);
		macroMapKb(VK_NUMPAD1,Numpad1);
		macroMapKb(VK_NUMPAD2,Numpad2);
		macroMapKb(VK_NUMPAD3,Numpad3);
		macroMapKb(VK_NUMPAD4,Numpad4);
		macroMapKb(VK_NUMPAD5,Numpad5);
		macroMapKb(VK_NUMPAD6,Numpad6);
		macroMapKb(VK_NUMPAD7,Numpad7);
		macroMapKb(VK_NUMPAD8,Numpad8);
		macroMapKb(VK_NUMPAD9,Numpad9);
		macroMapKb(VK_MULTIPLY,Multiply);
		macroMapKb(VK_ADD,Add);
		macroMapKb(VK_SUBTRACT,Subtract);
		macroMapKb(VK_SEPARATOR,Separator);
		macroMapKb(VK_DECIMAL,Decimal);
		macroMapKb(VK_DIVIDE,Divide);
		macroMapKb(VK_F1,F1);
		macroMapKb(VK_F2,F2);
		macroMapKb(VK_F3,F3);
		macroMapKb(VK_F4,F4);
		macroMapKb(VK_F5,F5);
		macroMapKb(VK_F6,F6);
		macroMapKb(VK_F7,F7);
		macroMapKb(VK_F8,F8);
		macroMapKb(VK_F9,F9);
		macroMapKb(VK_F10,F10);
		macroMapKb(VK_F11,F11);
		macroMapKb(VK_F12,F12);
		macroMapKb(VK_CAPITAL,Capslock);
		macroMapKb(VK_NUMLOCK,Numlock);
		macroMapKb(VK_LSHIFT,Lshift);
		macroMapKb(VK_RSHIFT,Rshift);
		macroMapKb(VK_LCONTROL,Lcontrol);
		macroMapKb(VK_RCONTROL,Rcontrol);
		macroMapKb(VK_SCROLL, Scroll);
		macroMapKb(VK_PAUSE,Pause);
		macroMapKb(VK_MENU,Alt);
		macroMapKb(VK_LMENU, Alt);
		macroMapKb(VK_RMENU, Altgr);
		macroMapKb(VK_APPS,Winapps);
		macroMapKb(VK_LWIN,Lwin);
		macroMapKb(VK_RWIN,Rwin);
		macroMapKb(VK_CLEAR, Clear);
		macroMapKb(VK_LBUTTON, Leftclick);
		macroMapKb(VK_RBUTTON, Rightclick);
		macroMapKb(VK_MBUTTON, Middleclick);
		macroMapKb(VK_VOLUME_MUTE, Volumemute);
		macroMapKb(VK_VOLUME_DOWN, Volumedown);
		macroMapKb(VK_VOLUME_UP, Volumeup);
		macroMapKb(VK_MEDIA_NEXT_TRACK, Medianext);
		macroMapKb(VK_MEDIA_PREV_TRACK, Mediaprev);
		macroMapKb(VK_MEDIA_STOP, Mediastop);
		macroMapKb(VK_MEDIA_PLAY_PAUSE, Mediaplaypause);

	default:
		return VirtualKey::None;
	}

	#undef macroMapKb
}


//Returns the associated Win32 VK_ to the supplied VirtualKey
WPARAM weave::input::Win32VirtualKeyToVk(VirtualKey vk) {
#define macroMapKb(n1,n2) case VirtualKey::n2: return n1;

	switch(vk) {
		macroMapKb('A', A);
		macroMapKb('B', B);
		macroMapKb('C', C);
		macroMapKb('D', D);
		macroMapKb('E', E);
		macroMapKb('F', F);
		macroMapKb('G', G);
		macroMapKb('H', H);
		macroMapKb('I', I);
		macroMapKb('J', J);
		macroMapKb('K', K);
		macroMapKb('L', L);
		macroMapKb('M', M);
		macroMapKb('N', N);
		macroMapKb('O', O);
		macroMapKb('P', P);
		macroMapKb('Q', Q);
		macroMapKb('R', R);
		macroMapKb('S', S);
		macroMapKb('T', T);
		macroMapKb('U', U);
		macroMapKb('V', V);
		macroMapKb('W', W);
		macroMapKb('X', X);
		macroMapKb('Y', Y);
		macroMapKb('Z', Z);
		macroMapKb('0', Key_0);
		macroMapKb('1', Key_1);
		macroMapKb('2', Key_2);
		macroMapKb('3', Key_3);
		macroMapKb('4', Key_4);
		macroMapKb('5', Key_5);
		macroMapKb('6', Key_6);
		macroMapKb('7', Key_7);
		macroMapKb('8', Key_8);
		macroMapKb('9', Key_9);
		macroMapKb(VK_TAB, Tab);
		macroMapKb(VK_RETURN, Enter);
		macroMapKb(VK_CONTROL, Control);
		macroMapKb(VK_SHIFT, Shift);
		macroMapKb(VK_SPACE, Space);
		macroMapKb(VK_BACK, Backspace);
		macroMapKb(VK_ESCAPE, Esc);
		macroMapKb(VK_PRIOR, Pageup);
		macroMapKb(VK_NEXT, Pagedown);
		macroMapKb(VK_END, End);
		macroMapKb(VK_HOME, Home);
		macroMapKb(VK_UP, Up);
		macroMapKb(VK_DOWN, Down);
		macroMapKb(VK_LEFT, Left);
		macroMapKb(VK_RIGHT, Right);
		macroMapKb(VK_SELECT, Select);
		macroMapKb(VK_SNAPSHOT, Printscreen);
		macroMapKb(VK_INSERT, Insert);
		macroMapKb(VK_DELETE, Delete);
		macroMapKb(VK_OEM_PLUS, Plus);
		macroMapKb(VK_OEM_MINUS, Minus);
		macroMapKb(VK_OEM_COMMA, Comma);
		macroMapKb(VK_OEM_PERIOD, Period);
		macroMapKb(VK_OEM_1, Vary1);
		macroMapKb(VK_OEM_2, Vary2);
		macroMapKb(VK_OEM_3, Vary3);
		macroMapKb(VK_OEM_4, Vary4);
		macroMapKb(VK_OEM_5, Vary5);
		macroMapKb(VK_OEM_6, Vary6);
		macroMapKb(VK_OEM_7, Vary7);
		macroMapKb(VK_OEM_8, Vary8);
		macroMapKb(VK_OEM_102, Vary102);
		macroMapKb(VK_NUMPAD0, Numpad0);
		macroMapKb(VK_NUMPAD1, Numpad1);
		macroMapKb(VK_NUMPAD2, Numpad2);
		macroMapKb(VK_NUMPAD3, Numpad3);
		macroMapKb(VK_NUMPAD4, Numpad4);
		macroMapKb(VK_NUMPAD5, Numpad5);
		macroMapKb(VK_NUMPAD6, Numpad6);
		macroMapKb(VK_NUMPAD7, Numpad7);
		macroMapKb(VK_NUMPAD8, Numpad8);
		macroMapKb(VK_NUMPAD9, Numpad9);
		macroMapKb(VK_MULTIPLY, Multiply);
		macroMapKb(VK_ADD, Add);
		macroMapKb(VK_SUBTRACT, Subtract);
		macroMapKb(VK_SEPARATOR, Separator);
		macroMapKb(VK_DECIMAL, Decimal);
		macroMapKb(VK_DIVIDE, Divide);
		macroMapKb(VK_F1, F1);
		macroMapKb(VK_F2, F2);
		macroMapKb(VK_F3, F3);
		macroMapKb(VK_F4, F4);
		macroMapKb(VK_F5, F5);
		macroMapKb(VK_F6, F6);
		macroMapKb(VK_F7, F7);
		macroMapKb(VK_F8, F8);
		macroMapKb(VK_F9, F9);
		macroMapKb(VK_F10, F10);
		macroMapKb(VK_F11, F11);
		macroMapKb(VK_F12, F12);
		macroMapKb(VK_CAPITAL, Capslock);
		macroMapKb(VK_NUMLOCK, Numlock);
		macroMapKb(VK_LSHIFT, Lshift);
		macroMapKb(VK_RSHIFT, Rshift);
		macroMapKb(VK_LCONTROL, Lcontrol);
		macroMapKb(VK_RCONTROL, Rcontrol);
		macroMapKb(VK_SCROLL, Scroll);
		macroMapKb(VK_PAUSE, Pause);
		macroMapKb(VK_LMENU, Alt);
		macroMapKb(VK_RMENU, Altgr);
		macroMapKb(VK_APPS, Winapps);
		macroMapKb(VK_LWIN, Lwin);
		macroMapKb(VK_RWIN, Rwin);
		macroMapKb(VK_CLEAR, Clear);
		macroMapKb(VK_LBUTTON, Leftclick);
		macroMapKb(VK_RBUTTON, Rightclick);
		macroMapKb(VK_MBUTTON, Middleclick);
		macroMapKb(VK_VOLUME_MUTE, Volumemute);
		macroMapKb(VK_VOLUME_DOWN, Volumedown);
		macroMapKb(VK_VOLUME_UP, Volumeup);
		macroMapKb(VK_MEDIA_NEXT_TRACK, Medianext);
		macroMapKb(VK_MEDIA_PREV_TRACK, Mediaprev);
		macroMapKb(VK_MEDIA_STOP, Mediastop);
		macroMapKb(VK_MEDIA_PLAY_PAUSE, Mediaplaypause);

		default:
			return 0;
	}

#undef macroMapKb
}

std::string weave::input::Win32CharacterUTF8(VirtualKey vk) {
	static BYTE kblayout[256] = {};
	WCHAR chars[2] = {};

	//Can't call this because it is thread dependent, very annoying
	//GetKeyboardState(kblayout);
	//So we manually fill in the table as a bit of a hacky version
	kblayout[VK_SHIFT] = (GetAsyncKeyState(VK_SHIFT) == 0 ? 0 : 128);
	kblayout[VK_LSHIFT] = kblayout[VK_SHIFT];
	kblayout[VK_CONTROL] = (GetAsyncKeyState(VK_CONTROL) == 0 ? 0 : 128);
	kblayout[VK_LCONTROL] = kblayout[VK_LCONTROL];
	kblayout[VK_CONTROL] = (GetAsyncKeyState(VK_CONTROL) == 0 ? 0 : 128);
	kblayout[VK_LMENU] = (GetAsyncKeyState(VK_LMENU) == 0 ? 0 : 128);
	kblayout[VK_RMENU] = (GetAsyncKeyState(VK_RMENU) == 0 ? 0 : 128);
	kblayout[VK_MENU] = (kblayout[VK_LMENU] ? 128 : (kblayout[VK_RMENU] ? 129 : 0));

	ToUnicode(UINT(input::Win32VirtualKeyToVk(vk)), 0, kblayout, chars, 2, 0);
	
	return utf::Utf16toUtf8(std::wstring(chars));
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
