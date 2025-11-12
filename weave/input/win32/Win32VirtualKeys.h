/*
Weave Win32 Virtual Keys
Win32VirtualKeys.h

Provides a method to translate Win32 VK_* codes into VirtualKey::* codes
*/
#pragma once

#include "weave/input/system/VirtualKeys.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

namespace weave::input {
//Returns the associated VirtualKey to the supplied VK_
VirtualKey Win32VkToVirtualKey(WPARAM vk);
//Returns the associated Win32 VK_ to the supplied VirtualKey
WPARAM Win32VirtualKeyToVk(VirtualKey vk);
//Returns the UTF8 character for a VirtualKey
std::string Win32CharacterUTF8(VirtualKey vk);
}
