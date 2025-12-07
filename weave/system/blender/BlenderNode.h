#pragma once

#include <vector>
#include <memory>
#include <typeinfo>
#include <algorithm>
#include <tuple>
#include <variant>
#include <string>
#include <unordered_map>

#include "BlenderIo.h"

namespace weave::blender {

struct BlenderNodeBase {
protected:
	int64_t executionStamp{};
	int32_t triggerCounter{};
	int32_t triggerCountTarget{}; //Amount of times the node needs to be triggered to allow execution as a trigger. Increased every time a node is connected to another node's trigger
	std::vector<BlenderNodeBase*> triggers;

public:
	void Execute(int64_t executionCounter) {
		if (executionStamp != executionCounter) {
			executionStamp = executionCounter;
			ExecuteNode();
			ExecuteTriggers(executionCounter);
		}
	}

	void ExecuteTriggers(int64_t executionCounter) {
		size_t index = 0;
		for (auto* trigger : triggers) {
			if (trigger) {
				trigger->triggerCounter++;
				if(trigger->triggerCounter >= trigger->triggerCountTarget) {
					trigger->triggerCounter = 0;
					if(TriggerFired(index)) {
						trigger->Execute(executionCounter);
					}
				}
			}
			index++;
		}
	}

	void ConnectTrigger(BlenderNodeBase* trigger) {
		if (trigger) {
			if (std::find(triggers.begin(), triggers.end(), trigger) == triggers.end()) {
				trigger->triggerCountTarget++;
				triggers.push_back(trigger);
			}
		}
	}

	void DisconnectTrigger(BlenderNodeBase* trigger) {
		triggers.erase(std::remove(triggers.begin(), triggers.end(), trigger), triggers.end());
	}

	void DisconnectTriggers() {
		triggers.clear();
	}

	void DisableCache() {
		executionStamp = 0;
	}

	virtual ~BlenderNodeBase() {}

	virtual void ExecuteNode() = 0;

	virtual bool TriggerFired([[maybe_unused]] size_t index) { return true; }

	virtual void ConnectInputTo(size_t inputNum, BlenderNodeBase* n, size_t nodeOutput) = 0;
	virtual void ConnectInputTo(size_t inputNum, std::pair<std::type_index, std::shared_ptr<void>> typedPtr) = 0;
	virtual void DisconnectInput(size_t inputNum) = 0;
	
	virtual void LinkUniforms(DynamicInterface& uniformInterface) = 0;

	virtual std::pair<std::type_index, std::shared_ptr<void>> GetSharedPtrOutput(size_t nodeOutput) = 0;

	virtual size_t GetInputCount() const = 0;
	virtual size_t GetOutputCount() const = 0;
	
	virtual std::type_index GetInputType(size_t inputNum) const = 0;
	virtual std::type_index GetOutputType(size_t outputNum) const = 0;
	virtual std::vector<std::type_index> GetInputTypes() const = 0;
	virtual std::vector<std::type_index> GetOutputTypes() const = 0;
};

template<typename ...InOutUniformBaseTypes>
struct BlenderNode : weave::first_base_of_t<BlenderNodeBase, InOutUniformBaseTypes...> {
	using InType = weave::first_specialization_of_t<In, InOutUniformBaseTypes...>;
	using OutType = weave::first_specialization_of_t<Out, InOutUniformBaseTypes...>;
	using UniformType = weave::first_specialization_of_t<Uniform, InOutUniformBaseTypes...>;
	using BaseType = weave::first_base_of_t<BlenderNodeBase, InOutUniformBaseTypes...>;

	static_assert(std::is_base_of_v<BlenderNodeBase, BaseType>, "Base class must derive from BlenderNodeBase.");

	static_assert(
		weave::count_specializations_of_v<In, InOutUniformBaseTypes...> <= 1 &&
		weave::count_specializations_of_v<Out, InOutUniformBaseTypes...> <= 1 &&
		weave::count_specializations_of_v<Uniform, InOutUniformBaseTypes...> <= 1,
		"Input, Output or Uniform objects incorrectly set. Ensure at most one object of each is defined.");

	InType input{ BaseType::executionStamp };
	OutType output{};
	UniformType uniform;

	
	BlenderNode() = default;
	BlenderNode(BlenderNode const& other) = default;
	BlenderNode(BlenderNode&& other) noexcept = default;
	BlenderNode& operator=(BlenderNode const& other) = default;
	BlenderNode& operator=(BlenderNode&& other) noexcept = default;
	
	template<size_t inputNum, size_t nodeOutput, typename OutType>
	void ConnectInputTo(OutType&& out) {
		ConnectInputTo(inputNum, std::forward<OutType>(out), nodeOutput);
	}

	template<size_t inputNum, typename OutType>
	void ConnectInputTo(OutType&& out) {
		ConnectInputTo(inputNum, std::forward<OutType>(out));
	}

	template<typename OutType>
	void ConnectInputTo(size_t inputNum, OutType&& out, size_t nodeOutput) {
		if constexpr (std::is_pointer_v<std::remove_reference_t<OutType>>) {
			input.Connect(inputNum, *out, nodeOutput);
		}
		else {
			input.Connect(inputNum, std::forward<OutType>(out), nodeOutput);
		}
	}


	void ConnectInputTo(size_t inputNum, BlenderNodeBase* n, size_t nodeOutput) override {
		input.Connect(inputNum, *n, nodeOutput);
	}

	void ConnectInputTo(size_t inputNum, std::pair<std::type_index, std::shared_ptr<void>> typedPtr) override {
		input.Connect(inputNum, typedPtr);
	}

	template<typename T>
	void ConnectInputTo(size_t inputNum, std::shared_ptr<T> ptr) {
		input.Connect(inputNum, std::pair<std::type_index, std::shared_ptr<void>>{ std::type_index(typeid(T)), ptr });
	}

	void DisconnectInput(size_t inputNum) override {
		input.Disconnect(inputNum);
	}

	std::pair<std::type_index, std::shared_ptr<void>> GetSharedPtrOutput(size_t nodeOutput) override {
		return output.GetSharedPtrOutput(nodeOutput);
	}

	size_t GetInputCount() const override {
		return std::tuple_size_v<decltype(InType::inputTuple)>;
	}
	size_t GetOutputCount() const override {
		return std::tuple_size_v<decltype(OutType::outputTuple)>;
	}

	std::type_index GetInputType(size_t inputNum) const override {
		return input.GetInputType(inputNum);
	}

	std::type_index GetOutputType(size_t outputNum) const override {
		return output.GetOutputType(outputNum);
	}

	std::vector<std::type_index> GetInputTypes() const override {
		std::vector<std::type_index> types(GetInputCount(), typeid(void));
		for (size_t i = 0; i < types.size(); ++i) {
			types[i] = GetInputType(i);
		}

		return types;
	}

	std::vector<std::type_index> GetOutputTypes() const override {
		std::vector<std::type_index> types(GetOutputCount(), typeid(void));
		for (size_t i = 0; i < types.size(); ++i) {
			types[i] = GetOutputType(i);
		}

		return types;
	}

	void LinkUniforms(DynamicInterface& uniformInterface) override {
		uniform.LinkUniforms(uniformInterface);
	}

};

}