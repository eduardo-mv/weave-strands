#pragma once

#include <array>
#include <string>
#include "weave/input/system/VirtualKeys.h"

namespace weave::input::gamepad {

enum class GamepadLayout {
	Generic,
	PlayStation3,
	PlayStation4,
	PlayStation5,
	XBox360Gamepad,
	XBoxOneGamepad,
	_LastLayout
};

constexpr std::size_t kLayoutButtonCount = 20;
constexpr std::size_t kLayoutCount = 6;

using LayoutArray = std::array<VirtualKey, kLayoutButtonCount>;
using LayoutTable = std::array<LayoutArray, kLayoutCount>;

constexpr LayoutTable kStandardLayouts{{
	// Generic
	LayoutArray{
		VirtualKey::Button0,  VirtualKey::Button1,  VirtualKey::Button2,  VirtualKey::Button3,
		VirtualKey::Button4,  VirtualKey::Button5,  VirtualKey::Button6,  VirtualKey::Button7,
		VirtualKey::Button8,  VirtualKey::Button9,  VirtualKey::Button10, VirtualKey::Button11,
		VirtualKey::Button12, VirtualKey::Button13, VirtualKey::Button14, VirtualKey::Button15,
		VirtualKey::Button16, VirtualKey::Button17, VirtualKey::Button18, VirtualKey::Button19
	},
	// PlayStation 3
	LayoutArray{
		VirtualKey::PS_Square, VirtualKey::PS_Cross, VirtualKey::PS_Circle, VirtualKey::PS_Triangle,
		VirtualKey::PS_L1,     VirtualKey::PS_R1,    VirtualKey::PS_L2,     VirtualKey::PS_R2,
		VirtualKey::PS_Select, VirtualKey::PS_Start, VirtualKey::PS_L3,     VirtualKey::PS_R3,
		VirtualKey::Button12,  VirtualKey::Button13, VirtualKey::Button14,  VirtualKey::Button15,
		VirtualKey::Button16,  VirtualKey::Button17, VirtualKey::Button18,  VirtualKey::Button19
	},
	// PlayStation 4
	LayoutArray{
		VirtualKey::PS_Square,       VirtualKey::PS_Cross,       VirtualKey::PS_Circle,        VirtualKey::PS_Triangle,
		VirtualKey::PS_L1,           VirtualKey::PS_R1,          VirtualKey::PS_L2,            VirtualKey::PS_R2,
		VirtualKey::PS_Share,        VirtualKey::PS_Options,     VirtualKey::PS_L3,            VirtualKey::PS_R3,
		VirtualKey::PS_System,       VirtualKey::PS_TrackpadKey, VirtualKey::Button14,         VirtualKey::Button15,
		VirtualKey::Button16,        VirtualKey::Button17,       VirtualKey::Button18,         VirtualKey::Button19
	},
	// PlayStation 5
	LayoutArray{
		VirtualKey::PS_Square,       VirtualKey::PS_Cross,       VirtualKey::PS_Circle,        VirtualKey::PS_Triangle,
		VirtualKey::PS_L1,           VirtualKey::PS_R1,          VirtualKey::PS_L2,            VirtualKey::PS_R2,
		VirtualKey::PS_Share,        VirtualKey::PS_Options,     VirtualKey::PS_L3,            VirtualKey::PS_R3,
		VirtualKey::PS_System,       VirtualKey::PS_TrackpadKey, VirtualKey::PS_Microphone,    VirtualKey::Button15,
		VirtualKey::Button16,        VirtualKey::Button17,       VirtualKey::Button18,         VirtualKey::Button19
	},
	// Xbox 360
	LayoutArray{
		VirtualKey::XBox_A,   VirtualKey::XBox_B,   VirtualKey::XBox_X,    VirtualKey::XBox_Y,
		VirtualKey::XBox_LB,  VirtualKey::XBox_RB,  VirtualKey::XBox_Back, VirtualKey::XBox_Start,
		VirtualKey::XBox_L3,  VirtualKey::XBox_R3,  VirtualKey::Button10,  VirtualKey::Button11,
		VirtualKey::Button12, VirtualKey::Button13, VirtualKey::Button14,  VirtualKey::Button15,
		VirtualKey::Button16, VirtualKey::Button17, VirtualKey::Button18,  VirtualKey::Button19
	},
	// Xbox One
	LayoutArray{
		VirtualKey::XBox_A,   VirtualKey::XBox_B,   VirtualKey::XBox_X,    VirtualKey::XBox_Y,
		VirtualKey::XBox_LB,  VirtualKey::XBox_RB,  VirtualKey::XBox_Back, VirtualKey::XBox_Start,
		VirtualKey::XBox_L3,  VirtualKey::XBox_R3,  VirtualKey::Button10,  VirtualKey::Button11,
		VirtualKey::Button12, VirtualKey::Button13, VirtualKey::Button14,  VirtualKey::Button15,
		VirtualKey::Button16, VirtualKey::Button17, VirtualKey::Button18,  VirtualKey::Button19
	}
}};

constexpr std::array<const char*, kLayoutCount> kLayoutNames{
	"Generic",
	"PlayStation 3",
	"PlayStation 4",
	"PlayStation 5",
	"Xbox 360",
	"Xbox One"
};

inline std::string LayoutName(GamepadLayout layout) {
	size_t idx = static_cast<size_t>(layout);
	if (idx >= kLayoutNames.size()) {
		idx = 0;
	}
	return kLayoutNames[idx];
}

} // namespace weave::input::gamepad
