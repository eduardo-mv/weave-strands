#pragma once

#include "weave/system/blender/Blender.h"

namespace weave::blender {

// Passive node used to merge or split trigger flows, also exposes trigger count.
class TriggerFunnel : public BlenderNode<
	Out<size_t>> {
public:
	enum OutputIndex : size_t {
		TriggerCountOutput
	};

	void ExecuteNode() override {
		this->output.template Ref<TriggerCountOutput>() = static_cast<size_t>(this->triggerCountTarget);
	}
};

// Propagates triggers only when the boolean input evaluates to true.
class TriggerConditional : public BlenderNode<
	In<bool>> {
public:
	enum InputIndex : size_t {
		ConditionInput
	};

	void ExecuteNode() override {}

	bool TriggerFired(size_t) override {
		return this->input.template Ref<ConditionInput>();
	}
};

// Allows trigger propagation only for the selected trigger index.
class TriggerSelector : public BlenderNode<
	In<size_t>> {
public:
	enum InputIndex : size_t {
		SelectedIndex
	};

	void ExecuteNode() override {}

	bool TriggerFired(size_t incomingIndex) override {
		return incomingIndex == this->input.template Ref<SelectedIndex>();
	}
};

} // namespace weave::blender
