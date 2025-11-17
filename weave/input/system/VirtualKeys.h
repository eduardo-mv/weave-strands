/*
Weave Input Virtual Keys
VirtualKeys.h

Defines the enums used for the virtual keys supported by the key mapping facilities
*/
#pragma once

#include <array>
#include <string>
#include <string_view>
#include <limits>
#include <cstdint>
#include "VirtualDevices.h"


namespace weave::input {

	enum class VirtualKeyState : uint32_t {
		Up,
		Down,
		Hold,
		Double //Handles states like double click
	};
	
	enum class KeyDataCapability : uint8_t {
		None = 0,
		State = 1 << 0,
		Pressure = 1 << 1,
		Cursor = 1 << 2,
		Midi = 1 << 3
	};

#define WEAVE_VIRTUAL_KEY_CANONICAL_ENTRIES(VK) \
		VK(None, KeyDataCapability::State) \
		VK(Cursor, KeyDataCapability::Cursor) \
		VK(Click_Left, KeyDataCapability::State) \
		VK(Click_Right, KeyDataCapability::State) \
		VK(Click_Middle, KeyDataCapability::State) \
		VK(Click_X4, KeyDataCapability::State) \
		VK(Click_X5, KeyDataCapability::State) \
		VK(Click_Wheel, KeyDataCapability::Cursor) \
		VK(Lstick_North, KeyDataCapability::State) \
		VK(Lstick_East, KeyDataCapability::State) \
		VK(Lstick_South, KeyDataCapability::State) \
		VK(Lstick_West, KeyDataCapability::State) \
		VK(Rstick_North, KeyDataCapability::State) \
		VK(Rstick_East, KeyDataCapability::State) \
		VK(Rstick_South, KeyDataCapability::State) \
		VK(Rstick_West, KeyDataCapability::State) \
		VK(Lstick, KeyDataCapability::Cursor) \
		VK(Rstick, KeyDataCapability::Cursor) \
		VK(Trigger0, KeyDataCapability::Pressure) \
		VK(Trigger1, KeyDataCapability::Pressure) \
		VK(Button0, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button1, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button2, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button3, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button4, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button5, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button6, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button7, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button8, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button9, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button10, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button11, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button12, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button13, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button14, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button15, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button16, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button17, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button18, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(Button19, KeyDataCapability::State | KeyDataCapability::Pressure) \
		VK(North, KeyDataCapability::State) \
		VK(Northeast, KeyDataCapability::State) \
		VK(East, KeyDataCapability::State) \
		VK(Southeast, KeyDataCapability::State) \
		VK(South, KeyDataCapability::State) \
		VK(Southwest, KeyDataCapability::State) \
		VK(West, KeyDataCapability::State) \
		VK(Northwest, KeyDataCapability::State) \
		VK(Nx_LeftSL, KeyDataCapability::State) \
		VK(Nx_LeftSR, KeyDataCapability::State) \
		VK(Nx_RightSL, KeyDataCapability::State) \
		VK(Nx_RightSR, KeyDataCapability::State) \
		VK(Control, KeyDataCapability::State) \
		VK(Alt, KeyDataCapability::State) \
		VK(Altgr, KeyDataCapability::State) \
		VK(Shift, KeyDataCapability::State) \
		VK(Tab, KeyDataCapability::State) \
		VK(Enter, KeyDataCapability::State) \
		VK(A, KeyDataCapability::State) \
		VK(B, KeyDataCapability::State) \
		VK(C, KeyDataCapability::State) \
		VK(D, KeyDataCapability::State) \
		VK(E, KeyDataCapability::State) \
		VK(F, KeyDataCapability::State) \
		VK(G, KeyDataCapability::State) \
		VK(H, KeyDataCapability::State) \
		VK(I, KeyDataCapability::State) \
		VK(J, KeyDataCapability::State) \
		VK(K, KeyDataCapability::State) \
		VK(L, KeyDataCapability::State) \
		VK(M, KeyDataCapability::State) \
		VK(N, KeyDataCapability::State) \
		VK(O, KeyDataCapability::State) \
		VK(P, KeyDataCapability::State) \
		VK(Q, KeyDataCapability::State) \
		VK(R, KeyDataCapability::State) \
		VK(S, KeyDataCapability::State) \
		VK(T, KeyDataCapability::State) \
		VK(U, KeyDataCapability::State) \
		VK(V, KeyDataCapability::State) \
		VK(W, KeyDataCapability::State) \
		VK(X, KeyDataCapability::State) \
		VK(Y, KeyDataCapability::State) \
		VK(Z, KeyDataCapability::State) \
		VK(F1, KeyDataCapability::State) \
		VK(F2, KeyDataCapability::State) \
		VK(F3, KeyDataCapability::State) \
		VK(F4, KeyDataCapability::State) \
		VK(F5, KeyDataCapability::State) \
		VK(F6, KeyDataCapability::State) \
		VK(F7, KeyDataCapability::State) \
		VK(F8, KeyDataCapability::State) \
		VK(F9, KeyDataCapability::State) \
		VK(F10, KeyDataCapability::State) \
		VK(F11, KeyDataCapability::State) \
		VK(F12, KeyDataCapability::State) \
		VK(Key_0, KeyDataCapability::State) \
		VK(Key_1, KeyDataCapability::State) \
		VK(Key_2, KeyDataCapability::State) \
		VK(Key_3, KeyDataCapability::State) \
		VK(Key_4, KeyDataCapability::State) \
		VK(Key_5, KeyDataCapability::State) \
		VK(Key_6, KeyDataCapability::State) \
		VK(Key_7, KeyDataCapability::State) \
		VK(Key_8, KeyDataCapability::State) \
		VK(Key_9, KeyDataCapability::State) \
		VK(Plus, KeyDataCapability::State) \
		VK(Minus, KeyDataCapability::State) \
		VK(Period, KeyDataCapability::State) \
		VK(Comma, KeyDataCapability::State) \
		VK(Vary1, KeyDataCapability::State) \
		VK(Vary2, KeyDataCapability::State) \
		VK(Vary3, KeyDataCapability::State) \
		VK(Vary4, KeyDataCapability::State) \
		VK(Vary5, KeyDataCapability::State) \
		VK(Vary6, KeyDataCapability::State) \
		VK(Vary7, KeyDataCapability::State) \
		VK(Vary8, KeyDataCapability::State) \
		VK(Vary102, KeyDataCapability::State) \
		VK(Numpad0, KeyDataCapability::State) \
		VK(Numpad1, KeyDataCapability::State) \
		VK(Numpad2, KeyDataCapability::State) \
		VK(Numpad3, KeyDataCapability::State) \
		VK(Numpad4, KeyDataCapability::State) \
		VK(Numpad5, KeyDataCapability::State) \
		VK(Numpad6, KeyDataCapability::State) \
		VK(Numpad7, KeyDataCapability::State) \
		VK(Numpad8, KeyDataCapability::State) \
		VK(Numpad9, KeyDataCapability::State) \
		VK(Multiply, KeyDataCapability::State) \
		VK(Add, KeyDataCapability::State) \
		VK(Separator, KeyDataCapability::State) \
		VK(Subtract, KeyDataCapability::State) \
		VK(Decimal, KeyDataCapability::State) \
		VK(Divide, KeyDataCapability::State) \
		VK(Backspace, KeyDataCapability::State) \
		VK(Capslock, KeyDataCapability::State) \
		VK(Esc, KeyDataCapability::State) \
		VK(Space, KeyDataCapability::State) \
		VK(Pageup, KeyDataCapability::State) \
		VK(Pagedown, KeyDataCapability::State) \
		VK(End, KeyDataCapability::State) \
		VK(Home, KeyDataCapability::State) \
		VK(Left, KeyDataCapability::State) \
		VK(Right, KeyDataCapability::State) \
		VK(Up, KeyDataCapability::State) \
		VK(Down, KeyDataCapability::State) \
		VK(Select, KeyDataCapability::State) \
		VK(Printscreen, KeyDataCapability::State) \
		VK(Insert, KeyDataCapability::State) \
		VK(Delete, KeyDataCapability::State) \
		VK(Numlock, KeyDataCapability::State) \
		VK(Scroll, KeyDataCapability::State) \
		VK(Pause, KeyDataCapability::State) \
		VK(Winapps, KeyDataCapability::State) \
		VK(Lwin, KeyDataCapability::State) \
		VK(Rwin, KeyDataCapability::State) \
		VK(Clear, KeyDataCapability::State) \
		VK(Lshift, KeyDataCapability::State) \
		VK(Rshift, KeyDataCapability::State) \
		VK(Lcontrol, KeyDataCapability::State) \
		VK(Rcontrol, KeyDataCapability::State) \
		VK(Volumemute, KeyDataCapability::State) \
		VK(Volumedown, KeyDataCapability::State) \
		VK(Volumeup, KeyDataCapability::State) \
		VK(Medianext, KeyDataCapability::State) \
		VK(Mediaprev, KeyDataCapability::State) \
		VK(Mediastop, KeyDataCapability::State) \
		VK(Mediaplaypause, KeyDataCapability::State) \
		VK(MidiNote, KeyDataCapability::Midi) \
		VK(MidiControl, KeyDataCapability::Midi) \
		VK(MidiPitchBend, KeyDataCapability::Midi) \
		VK(MidiProgramChange, KeyDataCapability::Midi) \
		VK(MidiChannelPressure, KeyDataCapability::Midi) \
		VK(MidiPolyPressure, KeyDataCapability::Midi)

#define WEAVE_VIRTUAL_KEY_ALIAS_LIST(VK) \
		VK(Stick0, Lstick) \
		VK(Stick1, Rstick) \
		VK(Zaxis, Trigger0) \
		VK(Wheel, Click_Wheel) \
		VK(Stick0_North, Lstick_North) \
		VK(Stick0_East, Lstick_East) \
		VK(Stick0_South, Lstick_South) \
		VK(Stick0_West, Lstick_West) \
		VK(Stick1_North, Rstick_North) \
		VK(Stick1_East, Rstick_East) \
		VK(Stick1_South, Rstick_South) \
		VK(Stick1_West, Rstick_West) \
		VK(Crossup, North) \
		VK(Crossupright, Northeast) \
		VK(Crossright, East) \
		VK(Crossdownright, Southeast) \
		VK(Crossdown, South) \
		VK(Crossdownleft, Southwest) \
		VK(Crossleft, West) \
		VK(Crossupleft, Northwest) \
		VK(Leftclick, Click_Left) \
		VK(X1click, Click_Left) \
		VK(Click_X1, Click_Left) \
		VK(Rightclick, Click_Right) \
		VK(X2click, Click_Right) \
		VK(Click_X2, Click_Right) \
		VK(Middleclick, Click_Middle) \
		VK(X3click, Click_Middle) \
		VK(Click_X3, Click_Middle) \
		VK(X4click, Click_X4) \
		VK(X5click, Click_X5) \
		VK(Ctrl, Control) \
		VK(PS_Square, Button0) \
		VK(PS_Cross, Button1) \
		VK(PS_Circle, Button2) \
		VK(PS_Triangle, Button3) \
		VK(PS_L1, Button4) \
		VK(PS_R1, Button5) \
		VK(PS_L2, Button6) \
		VK(PS_R2, Button7) \
		VK(PS_L2Trigger, Trigger0) \
		VK(PS_R2Trigger, Trigger1) \
		VK(PS_TrackpadKey, Button8) \
		VK(PS_Options, Button9) \
		VK(PS_Start, Button9) \
		VK(PS_L3, Button10) \
		VK(PS_R3, Button11) \
		VK(PS_System, Button12) \
		VK(PS_Share, Button13) \
		VK(PS_Select, Button13) \
		VK(PS_Microphone, Button14) \
		VK(PS_Up, North) \
		VK(PS_Down, South) \
		VK(PS_Left, West) \
		VK(PS_Right, East) \
		VK(PS_UpRight, Northeast) \
		VK(PS_DownRight, Southwest) \
		VK(PS_DownLeft, Southeast) \
		VK(PS_UpLeft, Northwest) \
		VK(XBox_A, Button1) \
		VK(XBox_B, Button2) \
		VK(XBox_X, Button0) \
		VK(XBox_Y, Button3) \
		VK(XBox_LB, Button4) \
		VK(XBox_RB, Button5) \
		VK(XBox_Back, Button8) \
		VK(XBox_Start, Button9) \
		VK(XBox_LT, Button6) \
		VK(XBox_RT, Button7) \
		VK(XBox_LTrigger, Trigger0) \
		VK(XBox_RTrigger, Trigger1) \
		VK(XBox_L3, Button10) \
		VK(XBox_R3, Button11) \
		VK(XBox_Up, North) \
		VK(XBox_Down, South) \
		VK(XBox_Left, West) \
		VK(XBox_Right, East) \
		VK(XBox_UpRight, Northeast) \
		VK(XBox_DownRight, Southwest) \
		VK(XBox_DownLeft, Southeast) \
		VK(XBox_UpLeft, Northwest) \
		VK(Wii_A, Button2) \
		VK(Wii_B, Button1) \
		VK(Wii_X, Button3) \
		VK(Wii_Y, Button0) \
		VK(Wii_L, Button4) \
		VK(Wii_R, Button5) \
		VK(Wii_Select, Button8) \
		VK(Wii_Start, Button9) \
		VK(Wii_ZL, Button6) \
		VK(Wii_ZR, Button7) \
		VK(Wii_LTrigger, Trigger0) \
		VK(Wii_RTrigger, Trigger1) \
		VK(Nx_A, Button2) \
		VK(Nx_B, Button1) \
		VK(Nx_X, Button3) \
		VK(Nx_Y, Button0) \
		VK(Nx_L, Button4) \
		VK(Nx_R, Button5) \
		VK(Nx_Minus, Button8) \
		VK(Nx_Plus, Button9) \
		VK(Nx_ZL, Button6) \
		VK(Nx_ZR, Button7) \
		VK(Nx_LTrigger, Trigger0) \
		VK(Nx_RTrigger, Trigger1) \
		VK(Nx_LS, Button10) \
		VK(Nx_RS, Button11) \
		VK(Nx_Up, North) \
		VK(Nx_Down, South) \
		VK(Nx_Left, West) \
		VK(Nx_Right, East) \
		VK(Nx_UpRight, Northeast) \
		VK(Nx_DownRight, Southwest) \
		VK(Nx_DownLeft, Southeast) \
		VK(Nx_UpLeft, Northwest)

	enum class VirtualKey : uint32_t {
#define VK_ENUM_ENTRY(key, ...) key,
		WEAVE_VIRTUAL_KEY_CANONICAL_ENTRIES(VK_ENUM_ENTRY)
#undef VK_ENUM_ENTRY

#define VK_ALIAS_ENTRY(name, target) name = target,
		WEAVE_VIRTUAL_KEY_ALIAS_LIST(VK_ALIAS_ENTRY)
#undef VK_ALIAS_ENTRY

		_FirstKey =	None,
		_LastKey = MidiPolyPressure,
		_CountKeys = (_LastKey - _FirstKey) + 1,
	};

	constexpr KeyDataCapability operator|(KeyDataCapability lhs, KeyDataCapability rhs) {
		return static_cast<KeyDataCapability>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
	}

	constexpr KeyDataCapability operator&(KeyDataCapability lhs, KeyDataCapability rhs) {
		return static_cast<KeyDataCapability>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
	}

	constexpr KeyDataCapability& operator|=(KeyDataCapability& lhs, KeyDataCapability rhs) {
		return lhs = (lhs | rhs);
	}

	constexpr bool HasCapability(KeyDataCapability caps, KeyDataCapability capability) {
		return static_cast<uint8_t>(caps & capability) != 0;
	}

	namespace detail {
		
	struct VirtualKeyDescriptor {
		VirtualKey key;
		std::string_view name;
		KeyDataCapability capability;
	};	

	struct VirtualKeyAlias {
		std::string_view name;
		VirtualKey key;
	};

	consteval auto BuildVirtualKeyDescriptors(){
	return std::array{
	#define VK_ENTRY(key, caps) VirtualKeyDescriptor{ VirtualKey::key, #key, caps },
				WEAVE_VIRTUAL_KEY_CANONICAL_ENTRIES(VK_ENTRY)
	#undef VK_ENTRY
			};
	}

	consteval auto BuildVirtualKeyAliases() {
	return std::array{
	#define VK_ENTRY(name, target) VirtualKeyAlias{ #name, VirtualKey::target },
				WEAVE_VIRTUAL_KEY_ALIAS_LIST(VK_ENTRY)
	#undef VK_ENTRY
			};
	}

	template<KeyDataCapability capability, typename Descriptors>
	consteval size_t CountCapabilityKeys(Descriptors const& descriptors) {
		size_t count = 0;
		for (auto const& descriptor : descriptors) {
			if (HasCapability(descriptor.capability, capability)) {
				++count;
			}
		}
		return count;
	}

	template<KeyDataCapability capability>
	consteval auto BuildCapabilityKeyList() {
		constexpr auto descriptors = BuildVirtualKeyDescriptors();
		std::array<VirtualKey, CountCapabilityKeys<capability>(descriptors)> keys{};
		size_t index = 0;
		for (auto const& descriptor : descriptors) {
			if (HasCapability(descriptor.capability, capability)) {
				keys[index++] = descriptor.key;
			}
		}
		return keys;
	}

	template<typename CapabilityArray>
	consteval auto BuildCapabilityIndex(CapabilityArray const& keys) {
		std::array<uint16_t, static_cast<size_t>(VirtualKey::_CountKeys)> indices{};
		indices.fill(std::numeric_limits<uint16_t>::max());
		for (size_t i = 0; i < keys.size(); ++i) {
			indices[static_cast<size_t>(keys[i])] = static_cast<uint16_t>(i);
		}
		return indices;
	}

	struct VirtualKeyCapabilityMetadata {
		using VirtualKeyArray = decltype(BuildVirtualKeyDescriptors());
		using VirtualKeyAliasesArray = decltype(BuildVirtualKeyAliases());
		using StateKeyArray = decltype(BuildCapabilityKeyList<KeyDataCapability::State>());
		using PressureKeyArray = decltype(BuildCapabilityKeyList<KeyDataCapability::Pressure>());
		using CursorKeyArray = decltype(BuildCapabilityKeyList<KeyDataCapability::Cursor>());
		using MidiKeyArray = decltype(BuildCapabilityKeyList<KeyDataCapability::Midi>());
		using VirtualKeyIndex = std::array<uint16_t, static_cast<size_t>(VirtualKey::_CountKeys)>;

		VirtualKeyArray descriptors{};
		VirtualKeyAliasesArray aliases{};
		StateKeyArray stateKeys{};
		PressureKeyArray pressureKeys{};
		CursorKeyArray cursorKeys{};
		MidiKeyArray midiKeys{};
		VirtualKeyIndex stateIndex{};
		VirtualKeyIndex pressureIndex{};
		VirtualKeyIndex cursorIndex{};
	};

	consteval VirtualKeyCapabilityMetadata BuildVirtualKeyMetadata() {
		VirtualKeyCapabilityMetadata data;
		data.descriptors = BuildVirtualKeyDescriptors();
		data.aliases = BuildVirtualKeyAliases();
		data.stateKeys = BuildCapabilityKeyList<KeyDataCapability::State>();
		data.pressureKeys = BuildCapabilityKeyList<KeyDataCapability::Pressure>();
		data.cursorKeys = BuildCapabilityKeyList<KeyDataCapability::Cursor>();
		data.midiKeys = BuildCapabilityKeyList<KeyDataCapability::Midi>();
		data.stateIndex = BuildCapabilityIndex(data.stateKeys);
		data.pressureIndex = BuildCapabilityIndex(data.pressureKeys);
		data.cursorIndex = BuildCapabilityIndex(data.cursorKeys);
		return data;
	}

	} // namespace detail

	inline constexpr auto kVirtualKeyMetadata = detail::BuildVirtualKeyMetadata();


	inline constexpr uint16_t GetStateKeyIndex(VirtualKey key) {
		return kVirtualKeyMetadata.stateIndex[static_cast<size_t>(key)];
	}

	inline constexpr uint16_t GetPressureKeyIndex(VirtualKey key) {
		return kVirtualKeyMetadata.pressureIndex[static_cast<size_t>(key)];
	}

	inline constexpr uint16_t GetCursorKeyIndex(VirtualKey key) {
		return kVirtualKeyMetadata.cursorIndex[static_cast<size_t>(key)];
	}

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
	inline constexpr std::string_view VirtualKeyName(VirtualKey key) {
		return kVirtualKeyMetadata.descriptors[static_cast<size_t>(key)].name;
	}

	inline constexpr KeyDataCapability KeyCapabilities(VirtualKey key) {
		return kVirtualKeyMetadata.descriptors[static_cast<size_t>(key)].capability;
	}

	inline constexpr bool SupportsState(VirtualKey key) {
		return HasCapability(KeyCapabilities(key), KeyDataCapability::State);
	}

	inline constexpr bool SupportsPressure(VirtualKey key) {
		return HasCapability(KeyCapabilities(key), KeyDataCapability::Pressure);
	}

	inline constexpr bool SupportsCursor(VirtualKey key) {
		return HasCapability(KeyCapabilities(key), KeyDataCapability::Cursor);
	}

	inline constexpr bool SupportsMidi(VirtualKey key) {
		return HasCapability(KeyCapabilities(key), KeyDataCapability::Midi);
	}

	//Returns the unicode UTF8 character associated to a VirtualKey
	std::string CharacterUTF8(VirtualKey key);

	//Offsets a keyvalue by a number by casting it to uint32_t and then back to VirtualKey. Use with caution.
	inline VirtualKey OffsetKey(VirtualKey key, uint32_t offset) {
		return static_cast<VirtualKey>(static_cast<uint32_t>(key) + offset);
	}
}
