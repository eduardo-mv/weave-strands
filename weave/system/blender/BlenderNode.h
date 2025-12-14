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
	friend class Blender;
protected:
	int64_t executionStamp{};
	std::vector<BlenderNodeBase*> outflowLinks;
	std::vector<BlenderNodeBase*> inflowLinks;

	FlowId flowId{ kInvalidFlowId };

public:
	void Execute(int64_t executionCounter, FlowId incomingFlow) {
		if (executionStamp != executionCounter) {
			if(FlowArrival(incomingFlow)) {
				executionStamp = executionCounter;
				ExecuteNode();
				PropagateOutflow(executionCounter);
			}
		}
	}

	void PropagateOutflow(int64_t executionCounter) {
		size_t index = 0;
		for (auto* link : outflowLinks) {
			if (link && FlowDeparture(index)) {
				link->Execute(executionCounter, flowId);
			}
			index++;
		}
	}

	virtual bool FlowArrival(FlowId ncomingFlow) = 0;
	virtual bool FlowDeparture(size_t outflowIndex) = 0;

	void ConnectOutflowLink(BlenderNodeBase* link) {
		if (link) {
			if (std::find(outflowLinks.begin(), outflowLinks.end(), link) == outflowLinks.end()) {
				link->RegisterInflowLink(this);
				outflowLinks.push_back(link);
			}
		}
	}

	void DisconnectOutflowLink(BlenderNodeBase* link) {
		if (link) {
			link->UnregisterInflowLink(this);
		}
		outflowLinks.erase(std::remove(outflowLinks.begin(), outflowLinks.end(), link), outflowLinks.end());
	}

	void DisconnectOutflowLinks() {
		for (auto* link : outflowLinks) {
			if (link) {
				link->UnregisterInflowLink(this);
			}
		}
		outflowLinks.clear();
	}

	void DisableCache() {
		executionStamp = 0;
	}

	virtual ~BlenderNodeBase() = default;

	virtual void ExecuteNode() = 0;

	virtual void ConnectInputTo(size_t inputNum, BlenderNodeBase* n, size_t nodeOutput) = 0;
	virtual void ConnectInputTo(size_t inputNum, std::pair<std::type_index, std::shared_ptr<void>> typedPtr) = 0;
	virtual void DisconnectInput(size_t inputNum) = 0;
	
	virtual void LinkUniforms(DynamicInterface& uniformInterface) = 0;
	virtual void LinkFlow(FlowInterface& flowInterface) = 0;
	virtual void PrepareFlowData(FlowInterface& flowInterface) = 0;

	virtual std::pair<std::type_index, std::shared_ptr<void>> GetSharedPtrOutput(size_t nodeOutput) = 0;

	virtual size_t GetInputCount() const = 0;
	virtual size_t GetOutputCount() const = 0;
	
	virtual std::type_index GetInputType(size_t inputNum) const = 0;
	virtual std::type_index GetOutputType(size_t outputNum) const = 0;
	virtual std::vector<std::type_index> GetInputTypes() const = 0;
	virtual std::vector<std::type_index> GetOutputTypes() const = 0;

protected:
	void RegisterInflowLink(BlenderNodeBase* parentLink) {
		if (parentLink) {
			if (std::find(inflowLinks.begin(), inflowLinks.end(), parentLink) == inflowLinks.end()) {
				inflowLinks.push_back(parentLink);
			}
		}
	}

	void UnregisterInflowLink(BlenderNodeBase* parentLink) {
		if (!parentLink) {
			return;
		}
		inflowLinks.erase(std::remove(inflowLinks.begin(), inflowLinks.end(), parentLink), inflowLinks.end());
	}

	void ClearInflowLinks() {
		inflowLinks.clear();
	}

	bool HasInflowLinks() const {
		return std::any_of(inflowLinks.begin(), inflowLinks.end(), [](auto* parent) { return parent != nullptr; });
	}

	bool HasOutflowLinks() const {
		return std::any_of(outflowLinks.begin(), outflowLinks.end(), [](auto* child) { return child != nullptr; });
	}

	size_t CountActiveOutflows() const {
		return std::count_if(outflowLinks.begin(), outflowLinks.end(), [](auto* child) { return child != nullptr; });
	}

	FlowId EnsureFlowCompiled(FlowInterface& flowInterface) {
		if (flowId != kInvalidFlowId) {
			return flowId; // FlowId already set
		}

		const bool hasInflows = HasInflowLinks();
		const bool hasOutflows = HasOutflowLinks();

		if (!hasInflows && !hasOutflows) {
			return flowId; // No in or out flow, there's no flow on this node so it returns invalid
		}

		if (!hasInflows) {
			flowId = flowInterface.CreateFlowId();
			auto& block = flowInterface.GetBlock(flowId);
			block.ancestry.clear();
			return flowId; // No parent inflows, but has outflows, so a new flowid is created
		}


		if (inflowLinks.size() == 1) {
			auto* parentNode = inflowLinks.front();
			const bool parentBranches = parentNode && parentNode->CountActiveOutflows() > 1;
			if (!parentBranches) {
				flowId = parentNode->EnsureFlowCompiled(flowInterface);
				return flowId; // Node has a single parent that isn't branching, so the parent flowid is directly inherited
			}
		}
		
		// Node has multiple parents or a single parent that is branching, so a new flowid is needed alongside the ancestry
		std::vector<FlowId> parentIds;
		parentIds.reserve(inflowLinks.size());
		for (auto* parent : inflowLinks) {
			if (!parent) {
				continue;
			}
			auto parentFlowId = parent->EnsureFlowCompiled(flowInterface);
			if (parentFlowId != kInvalidFlowId) {
				parentIds.emplace_back(parentFlowId);
			}
		}

		flowId = flowInterface.CreateFlowId();
		flowInterface.MergeAncestry(flowId, parentIds);

		return flowId;
	}
};

template<typename ...InOutUniformBaseTypes>
struct BlenderNode : weave::first_base_of_t<BlenderNodeBase, InOutUniformBaseTypes...> {
	using InType = weave::first_specialization_of_t<In, InOutUniformBaseTypes...>;
	using OutType = weave::first_specialization_of_t<Out, InOutUniformBaseTypes...>;
	using UniformType = weave::first_specialization_of_t<Uniform, InOutUniformBaseTypes...>;
	using FlowType = weave::first_specialization_of_t<Flow, InOutUniformBaseTypes...>;
	using BaseType = weave::first_base_of_t<BlenderNodeBase, InOutUniformBaseTypes...>;
	using BlenderNodeBase::flowId;
	using BlenderNodeBase::inflowLinks;

	static_assert(std::is_base_of_v<BlenderNodeBase, BaseType>, "Base class must derive from BlenderNodeBase.");

	static_assert(
		weave::count_specializations_of_v<In, InOutUniformBaseTypes...> <= 1 &&
		weave::count_specializations_of_v<Out, InOutUniformBaseTypes...> <= 1 &&
		weave::count_specializations_of_v<Uniform, InOutUniformBaseTypes...> <= 1,
		"Input, Output or Uniform objects incorrectly set. Ensure at most one object of each is defined.");

	InType input{ BaseType::executionStamp };
	OutType output{};
	UniformType uniform;
	FlowType flow; 

	
	BlenderNode() = default;
	BlenderNode(BlenderNode const& other) = default;
	BlenderNode(BlenderNode&& other) noexcept = default;
	BlenderNode& operator=(BlenderNode const& other) = default;
	BlenderNode& operator=(BlenderNode&& other) noexcept = default;
	

	bool FlowArrival(FlowId incomingFlow) override {
		flow.inflowStage.emplace_back(incomingFlow);
		if(flow.inflowStage.size() >= inflowLinks.size()) {
			flow.inflowStage.clear();
			return true;
		}

		return false;
	}

	bool FlowDeparture([[maybe_unused]] size_t outflowIndex) override {
		return true;
	}

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

	void LinkFlow(FlowInterface& flowInterface) override {
		flow.LinkFlow(flowId, flowInterface);
	}

	void PrepareFlowData(FlowInterface& flowInterface) override {
		flow.PrepareFlowData(flowId, flowInterface);
	}


};

}
