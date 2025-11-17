/*
Weave Input Virtual Devices
VirtualDevices.h

Defines the enums used for the virtual devices supported by the key mapping facilities, used to identify the source of the key pressed or movement
*/
#pragma once

#include <string>
#include <cstdint>

namespace weave::input {

	enum class DeviceState : uint32_t {
		Disconnected,
		Idle,
		Active,
		FocusLost,
		FocusGained
	};

	enum class VirtualDevice : unsigned short {
		None,
		Mouse,
		_First_Mouse = Mouse,
		Mouse0 = Mouse,
		Mouse1,
		Mouse2,
		Mouse3,
		Mouse4,
		Mouse5,
		Mouse6,
		Mouse7,
		Mouse8,
		Mouse9,
		Mouse10,
		Mouse11,
		Mouse12,
		Mouse13,
		Mouse14,
		Mouse15,
		_Last_Mouse = Mouse15,
		Keyboard,
		_First_Keyboard = Keyboard,
		Keyboard0 = Keyboard,
		Keyboard1,
		Keyboard2,
		Keyboard3,
		Keyboard4,
		Keyboard5,
		Keyboard6,
		Keyboard7,
		Keyboard8,
		Keyboard9,
		Keyboard10,
		Keyboard11,
		Keyboard12,
		Keyboard13,
		Keyboard14,
		Keyboard15,
		_Last_Keyboard = Keyboard15,
		Gamepad,
		_First_Gamepad = Gamepad,
		Gamepad0 = Gamepad,
		Gamepad1,
		Gamepad2,
		Gamepad3,
		Gamepad4,
		Gamepad5,
		Gamepad6,
		Gamepad7,
		Gamepad8,
		Gamepad9,
		Gamepad10,
		Gamepad11,
		Gamepad12,
		Gamepad13,
		Gamepad14,
		Gamepad15,
		_Last_Gamepad = Gamepad15,
		Midi,
		_First_Midi = Midi,
		Midi0 = Midi,
		Midi1,
		Midi2,
		Midi3,
		Midi4,
		Midi5,
		Midi6,
		Midi7,
		Midi8,
		Midi9,
		Midi10,
		Midi11,
		Midi12,
		Midi13,
		Midi14,
		Midi15,
		_Last_Midi = Midi15,
		_MouseCount = (_Last_Mouse - _First_Mouse) + 1,
		_KeyboardCount = (_Last_Keyboard - _First_Keyboard) + 1,
		_GamepadCount = (_Last_Gamepad - _First_Gamepad) + 1,
		_MidiCount = (_Last_Midi - _First_Midi) + 1,
		_First = None,
		_Last = Midi15,
		_Count = (_Last - _First) + 1
	};
	
	//Helps translate device names into virtual device codes
	VirtualDevice VirtualDeviceValue(std::string const &name);

	//Translates virtual device codes into strings
	std::string VirtualDeviceName(VirtualDevice dev);

	//Helps translate device state names into device state codes
	DeviceState DeviceStateValue(std::string const& name);

	//Translates device state codes into strings
	std::string DeviceStateName(DeviceState dev);

	//Returns if the device is a cursor device
	bool IsDeviceCursor(VirtualDevice dev);
	
	//Returns true if the device is a keyboard
	bool IsMouseDevice(VirtualDevice dev);

	//Returns true if the device is a keyboard
	bool IsKeyboardDevice(VirtualDevice dev);
	
	//Returns true if the device is a gamepad
	bool IsGamepadDevice(VirtualDevice dev);
	
	//Returns true if the device is a midi device
	bool IsMidiDevice(VirtualDevice dev);

	//Offsets a keyvalue by a number by casting it to uint32_t and then back to VirtualKey. Use with caution.
	inline VirtualDevice OffsetDevice(VirtualDevice dev, uint32_t offset) {
		return static_cast<VirtualDevice>(static_cast<uint32_t>(dev) + offset);
	}

	std::pair<VirtualDevice, uint32_t> GetDeviceAndOffset(VirtualDevice dev);

}
