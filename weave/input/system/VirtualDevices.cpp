#include "VirtualDevices.h"
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <string_view>

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

//Helps translate key names into virtual key codes
VirtualDevice weave::input::VirtualDeviceValue(std::string const &name) {

#define macroTable(n) {fixAttribute(#n), VirtualDevice::n},
	
	static std::unordered_map<std::string, VirtualDevice> map {
		macroTable(None)
		macroTable(Mouse)
		macroTable(Mouse0)
		macroTable(Mouse1)
		macroTable(Mouse2)
		macroTable(Mouse3)
		macroTable(Mouse4)
		macroTable(Mouse5)
		macroTable(Mouse6)
		macroTable(Mouse7)
		macroTable(Mouse8)
		macroTable(Mouse9)
		macroTable(Mouse10)
		macroTable(Mouse11)
		macroTable(Mouse12)
		macroTable(Mouse13)
		macroTable(Mouse14)
		macroTable(Mouse15)
		macroTable(Keyboard)
		macroTable(Keyboard0)
		macroTable(Keyboard1)
		macroTable(Keyboard2)
		macroTable(Keyboard3)
		macroTable(Keyboard4)
		macroTable(Keyboard5)
		macroTable(Keyboard6)
		macroTable(Keyboard7)
		macroTable(Keyboard8)
		macroTable(Keyboard9)
		macroTable(Keyboard10)
		macroTable(Keyboard11)
		macroTable(Keyboard12)
		macroTable(Keyboard13)
		macroTable(Keyboard14)
		macroTable(Keyboard15)
		macroTable(Gamepad)
		macroTable(Gamepad0)
		macroTable(Gamepad1)
		macroTable(Gamepad2)
		macroTable(Gamepad3)
		macroTable(Gamepad4)
		macroTable(Gamepad5)
		macroTable(Gamepad6)
		macroTable(Gamepad7)
		macroTable(Gamepad8)
		macroTable(Gamepad9)
		macroTable(Gamepad10)
		macroTable(Gamepad11)
		macroTable(Gamepad12)
		macroTable(Gamepad13)
		macroTable(Gamepad14)
		macroTable(Gamepad15)
	};

#undef macroTable
	
	return map[fixAttribute(name)];
}



//Translates virtual device codes into strings
std::string weave::input::VirtualDeviceName(VirtualDevice dev) {
#define macroTable(n) case VirtualDevice::n: return #n;

	switch(dev) {
		macroTable(None)
			macroTable(Mouse0)
			macroTable(Mouse1)
			macroTable(Mouse2)
			macroTable(Mouse3)
			macroTable(Mouse4)
			macroTable(Mouse5)
			macroTable(Mouse6)
			macroTable(Mouse7)
			macroTable(Mouse8)
			macroTable(Mouse9)
			macroTable(Mouse10)
			macroTable(Mouse11)
			macroTable(Mouse12)
			macroTable(Mouse13)
			macroTable(Mouse14)
			macroTable(Mouse15)
			macroTable(Keyboard0)
			macroTable(Keyboard1)
			macroTable(Keyboard2)
			macroTable(Keyboard3)
			macroTable(Keyboard4)
			macroTable(Keyboard5)
			macroTable(Keyboard6)
			macroTable(Keyboard7)
			macroTable(Keyboard8)
			macroTable(Keyboard9)
			macroTable(Keyboard10)
			macroTable(Keyboard11)
			macroTable(Keyboard12)
			macroTable(Keyboard13)
			macroTable(Keyboard14)
			macroTable(Keyboard15)
			macroTable(Gamepad0)
			macroTable(Gamepad1)
			macroTable(Gamepad2)
			macroTable(Gamepad3)
			macroTable(Gamepad4)
			macroTable(Gamepad5)
			macroTable(Gamepad6)
			macroTable(Gamepad7)
			macroTable(Gamepad8)
			macroTable(Gamepad9)
			macroTable(Gamepad10)
			macroTable(Gamepad11)
			macroTable(Gamepad12)
			macroTable(Gamepad13)
			macroTable(Gamepad14)
			macroTable(Gamepad15)
		default:
			break;
	}

#undef macroTable
	
	return "";
}

DeviceState weave::input::DeviceStateValue(std::string const& name) {
#define macroTable(n) {fixAttribute(#n), DeviceState::n},

static std::unordered_map<std::string, DeviceState> map{
		macroTable(Disconnected)
		macroTable(Idle)
		macroTable(Active)
		macroTable(FocusLost)
		macroTable(FocusGained)
	};

#undef macroTable

	return map[fixAttribute(name)];
}

//Translates device state codes into strings
std::string weave::input::DeviceStateName(DeviceState dev) {
#define macroTable(n) case DeviceState::n: return #n;

	switch (dev) {
		macroTable(Disconnected)
		macroTable(Idle)
		macroTable(Active)
		macroTable(FocusLost)
		macroTable(FocusGained)
	default:
		break;
	}

#undef macroTable

	return "";
}


bool weave::input::IsDeviceCursor(VirtualDevice dev) {
	switch(dev) {
		case VirtualDevice::Keyboard0:
		case VirtualDevice::Keyboard1:
		case VirtualDevice::Keyboard2:
		case VirtualDevice::Keyboard3:
		case VirtualDevice::Keyboard4:
		case VirtualDevice::Keyboard5:
		case VirtualDevice::Keyboard6:
		case VirtualDevice::Keyboard7:
		case VirtualDevice::Keyboard8:
		case VirtualDevice::Keyboard9:
		case VirtualDevice::Keyboard10:
		case VirtualDevice::Keyboard11:
		case VirtualDevice::Keyboard12:
		case VirtualDevice::Keyboard13:
		case VirtualDevice::Keyboard14:
		case VirtualDevice::Keyboard15:
			return false;
		default:
			return true;
	}
}

bool weave::input::IsMouseDevice(VirtualDevice dev) {
	return dev >= VirtualDevice::_First_Mouse && dev <= VirtualDevice::_Last_Mouse;
}

bool weave::input::IsKeyboardDevice(VirtualDevice dev) {
	return dev >= VirtualDevice::_First_Keyboard && dev <= VirtualDevice::_Last_Keyboard;
}

bool weave::input::IsGamepadDevice(VirtualDevice dev) {
	return dev >= VirtualDevice::_First_Gamepad && dev <= VirtualDevice::_Last_Gamepad;
}

std::pair<VirtualDevice, uint32_t> weave::input::GetDeviceAndOffset(VirtualDevice dev) {
	auto deviceValue = static_cast<std::underlying_type_t<VirtualDevice>>(dev);

	constexpr auto mouseRange = std::pair{
		static_cast<std::underlying_type_t<VirtualDevice>>(VirtualDevice::_First_Mouse),
		static_cast<std::underlying_type_t<VirtualDevice>>(VirtualDevice::_Last_Mouse) };

	constexpr auto keyboardRange = std::pair{
		static_cast<std::underlying_type_t<VirtualDevice>>(VirtualDevice::_First_Keyboard),
		static_cast<std::underlying_type_t<VirtualDevice>>(VirtualDevice::_Last_Keyboard) };

	constexpr auto gamepadRange = std::pair{
		static_cast<std::underlying_type_t<VirtualDevice>>(VirtualDevice::_First_Gamepad),
		static_cast<std::underlying_type_t<VirtualDevice>>(VirtualDevice::_Last_Gamepad) };

	auto inRange = [deviceValue](auto range) {
		return (deviceValue >= range.first && deviceValue <= range.second);
	};

	if (inRange(mouseRange)) {
		return { VirtualDevice::Mouse, deviceValue - mouseRange.first };
	}

	if (inRange(keyboardRange)) {
		return { VirtualDevice::Keyboard, deviceValue - keyboardRange.first };
	}

	if (inRange(gamepadRange)) {
		return { VirtualDevice::Gamepad, deviceValue - gamepadRange.first };
	}
	return { VirtualDevice::None, 0 };

}
