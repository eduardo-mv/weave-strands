/*
Input Processor:
Router

Allows routing of devices and keys to other devices and keys.
Device + Key pairs can defined as From and To routes. 
No device or no key is also allowed, acting as a wild card that routes "any device" or "any key"

e.g. Routing a Mouse3 into Mouse1

AddRoute({VirtualDevice::Mouse3, {}}, {VirtualDevice::Mouse1, {}});

*/
#pragma once

#include "weave/input/system/InputProcessor.h"

namespace weave::input {

class InputRouter : public InputProcessor {
public:
	struct Route {
		std::optional<VirtualDevice> device;
		std::optional<VirtualKey> key;
	};

private:

	struct PairHash {
		std::size_t operator()(Route value) const {
			uint64_t i64value =
				static_cast<uint64_t>(value.device.value_or(VirtualDevice::None)) << 32
				| static_cast<uint64_t>(value.key.value_or(VirtualKey::None));
			return size_t(i64value);
		}

		constexpr bool operator()(const Route& lhs, const Route& rhs) const {
			return lhs.device.value_or(VirtualDevice::None) == rhs.device.value_or(VirtualDevice::None)
				&& lhs.key.value_or(VirtualKey::None) == rhs.key.value_or(VirtualKey::None);
		}
	};


	std::unordered_multimap<Route, Route, PairHash, PairHash> routes;

public:
	InputRouter() = default;

	void AddRoute(Route from, Route to);

private:
	void InputEvent(InputEventData inputData, InputStateMap& inputState) override;
};

}