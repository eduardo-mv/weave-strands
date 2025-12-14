#pragma once

#include <limits>

#include "weave/system/blender/Blender.h"

namespace weave::blender {

// Propagates flow only when the boolean input evaluates to true.
class FlowConditional : public BlenderNode<
	In<bool>> {
public:
	enum InputIndex : size_t {
		ConditionInput
	};

	void ExecuteNode() override {}

	bool FlowDeparture(size_t) override {
		auto allowOutflow = this->input.template Ref<ConditionInput>();
		return allowOutflow;
	}
};

// Allows propagation only for the selected incoming flow index.
class FlowSelector : public BlenderNode<
	In<size_t>> {

public:
	enum InputIndex : size_t {
		SelectedIndex
	};

	void ExecuteNode() override {}

	bool FlowDeparture(size_t outflowIndex) override {
		const size_t selected = this->input.template Ref<SelectedIndex>();
		return outflowIndex == selected;
	}
};

template<typename ...Types>
class FlushFlow : public BlenderNode<
	In<size_t>,
	Flow<Types...>> {

public:
	enum InputIndex : size_t {
		SelectedIndex
	};

	void ExecuteNode() override {
		(flow.Flush<Types>(), ...);
	}

};

} // namespace weave::blender
