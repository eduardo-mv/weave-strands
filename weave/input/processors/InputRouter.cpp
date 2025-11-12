#include "InputRouter.h"

using namespace weave::input;

void InputRouter::AddRoute(Route input, Route output) {
	routes.insert({ input, output });
}

void InputRouter::InputEvent(InputEventData inputData, InputStateMap& inputState) {

	std::pair in{ inputData.device, inputData.key };

	auto routeInput = [&, this](auto range) {
		if (range.first != range.second) {
			for (auto const& out : std::ranges::subrange(range.first, range.second)) {
				inputData.device = out.second.device.value_or(out.first.device.value_or(in.first));
				inputData.key = out.second.key.value_or(out.first.key.value_or(in.second));
				NextProcessor(inputData, inputState);
			}

			return true;
		}

		return false;
	};

	auto range = routes.equal_range(Route{ inputData.device, inputData.key });
	if (routeInput(range))
		return;

	range = routes.equal_range(Route{ inputData.device, {} });
	if (routeInput(range))
		return;

	range = routes.equal_range(Route{ {}, inputData.key });
	if (routeInput(range))
		return;

	NextProcessor(inputData, inputState);
}