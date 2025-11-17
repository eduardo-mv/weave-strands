#include "VirtualKeys.h"
#include <unordered_map>
#include <algorithm>
#include <array>
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
	static const std::unordered_map<std::string, VirtualKey> map = [] {
		std::unordered_map<std::string, VirtualKey> table;
		table.reserve(kVirtualKeyMetadata.descriptors.size() + kVirtualKeyMetadata.aliases.size());

		auto addEntry = [&table](std::string_view entryName, VirtualKey key) {
			auto normalized = fixAttribute(std::string(entryName));
			table.emplace(std::move(normalized), key);
		};

		for (auto const& descriptor : kVirtualKeyMetadata.descriptors) {
			addEntry(descriptor.name, descriptor.key);
		}

		for (auto const& alias : kVirtualKeyMetadata.aliases) {
			addEntry(alias.name, alias.key);
		}

		return table;
	}();

	auto normalized = fixAttribute(std::string(name));
	if (auto it = map.find(normalized); it != map.end()) {
		return it->second;
	}

	return VirtualKey::None;
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
