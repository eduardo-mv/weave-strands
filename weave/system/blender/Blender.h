#pragma once
/*
Blender system. TODO: Description
*/


#include <vector>
#include <memory>
#include <typeinfo>
#include <algorithm>
#include <tuple>
#include <variant>
#include <string>
#include <unordered_map>

#include "TupleTraits.h"
#include "BlenderIo.h"
#include "BlenderNode.h"
#include "NameManager.h"

namespace weave::blender {

struct Connection {
	std::variant<BlenderNodeBase*, std::string> fromOutputNode{};
	std::variant<BlenderNodeBase*, std::string> toInputNode{};

	std::variant<size_t, std::string> outputIndex{};
	std::variant<size_t, std::string> inputIndex{};

	Connection() = default;
	
	template<typename T1, typename T2, typename T3, typename T4>
	Connection(T1&& fromOutputNode, T2&& outputIndexValue, T3&& toInputNode, T4&& inputIndexValue)
		: fromOutputNode(std::forward<T1>(fromOutputNode))
		, toInputNode(std::forward<T3>(toInputNode))
		, outputIndex(static_cast<std::conditional_t<std::is_scalar_v<T2>, size_t, T2>>(outputIndexValue))
		, inputIndex(static_cast<std::conditional_t<std::is_scalar_v<T4>, size_t, T4>>(inputIndexValue)) {}

	bool IsUsingNodeOutputIndex() const {
		return std::holds_alternative<size_t>(outputIndex);
	}

	bool IsUsingNodeInputIndex() const {
		return std::holds_alternative<size_t>(inputIndex);
	}

	bool IsUsingFromOutputNodePointer() const {
		return std::holds_alternative<BlenderNodeBase*>(fromOutputNode);
	}

	bool IsUsingToInputNodePointer() const {
		return std::holds_alternative<BlenderNodeBase*>(toInputNode);
	}
};

struct NodeIoNames {
	std::vector<std::pair<std::string, size_t>> inputNames;
	std::vector<std::pair<std::string, size_t>> outputNames;
};

class FlowEntryNode : public BlenderNode<> {
public:
	void ExecuteNode() override {};
};

class Blender {
protected:
	int64_t executionCounter{};
	DynamicInterface ioInterface;
	FlowInterface flowInterface;
	FlowEntryNode rootNode;
	std::vector<std::unique_ptr<BlenderNodeBase>> nodes;
	bool flowGraphDirty{ true };
	
	NameManager nameManager;

public:
	template<typename NodeType, typename... Args>
	NodeType* CreateNamedNode(std::string const& name, Args&&... args) {
		auto node = CreateNode<NodeType>(std::forward<Args>(args)...);
		RegisterNodeName(node, name);
		return node;
	}

	template<typename NodeType, typename... Args>
	NodeType* CreateNamedNode(std::pair<std::string, NodeIoNames> nodeAndIoNames, Args&&... args) {
		auto node = CreateNode<NodeType>(std::forward<Args>(args)...);
		RegisterNodeNameAndIo(node, nodeAndIoNames.first, nodeAndIoNames.second);
		return node;
	}


	template<typename NodeType, typename... Args>
	NodeType* CreateNode(Args&&... args) {
		
		static_assert(std::is_base_of_v<BlenderNodeBase, NodeType>, "Node must be derived from the BlenderNodeBase base class.");

		std::unique_ptr<NodeType> node;
		node = std::make_unique<NodeType>(std::forward<Args>(args)...);
		node->LinkUniforms(ioInterface);

		NodeType* nodePtr = node.get();
		nodes.push_back(std::move(node));
		MarkFlowGraphDirty();
		
		return nodePtr;
	}

	void AddRootFlowLink(BlenderNodeBase* node) {
		rootNode.ConnectOutflowLink(node);
	}

	void RemoveRootFlowLink(BlenderNodeBase* node) {
		rootNode.DisconnectOutflowLink(node);
	}

	// Node names
	bool RegisterNodeName(BlenderNodeBase* node, const std::string& name) {
		return nameManager.RegisterNodeName(node, name);
	}

	bool RegisterNodeNameAndIo(BlenderNodeBase* node, const std::string& nodeName, NodeIoNames const& ioNames) {
		bool nodeRegistered = nameManager.RegisterNodeName(node, nodeName);

		if (!nodeRegistered) {
			return false;
		}

		for (const auto& inputName : ioNames.inputNames) {
			size_t index = inputName.second;
			const std::string& name = inputName.first;
			if (!nameManager.RegisterInputName(node, index, name)) {
				// Failed to register an input name, remove previously registered input names and the node name
				for (const auto& registeredInputName : ioNames.inputNames) {
					if (registeredInputName.second >= index) {
						break;
					}
					nameManager.RemoveInputName(node, registeredInputName.first);
				}
				nameManager.RemoveNodeName(name);
				return false;
			}
		}

		for (const auto& outputName : ioNames.outputNames) {
			size_t index = outputName.second;
			const std::string& name = outputName.first;
			if (!nameManager.RegisterOutputName(node, index, name)) {
				// Failed to register an output name, remove previously registered output/input names and the node name
				for (const auto& registeredOutputName : ioNames.outputNames) {
					if (registeredOutputName.second >= index) {
						break;
					}
					nameManager.RemoveOutputName(node, registeredOutputName.first);
				}
				for (const auto& registeredInputName : ioNames.inputNames) {
					nameManager.RemoveInputName(node, registeredInputName.first);
				}
				nameManager.RemoveNodeName(name);
				return false;
			}
		}

		return true;
	}

	BlenderNodeBase* GetNodeByName(const std::string& name) const {
		return nameManager.GetNodeByName(name);
	}

	std::string GetNodeName(BlenderNodeBase* node) const {
		return nameManager.GetNodeName(node);
	}

	bool RemoveNodeName(const std::string& name) {
		return nameManager.RemoveNodeName(name);
	}

	// Per node Input/Output names
	bool RegisterInputName(BlenderNodeBase* node, size_t index, const std::string& name) {
		return nameManager.RegisterInputName(node, index, name);
	}

	bool RegisterOutputName(BlenderNodeBase* node, size_t index, const std::string& name) {
		return nameManager.RegisterOutputName(node, index, name);
	}

	std::string GetInputName(BlenderNodeBase* node, size_t index) const {
		return nameManager.GetInputName(node, index);
	}

	std::string GetOutputName(BlenderNodeBase* node, size_t index) const {
		return nameManager.GetOutputName(node, index);
	}

	size_t GetInputIndexByName(BlenderNodeBase* node, const std::string& name) const {
		return nameManager.GetInputIndexByName(node, name);
	}

	size_t GetOutputIndexByName(BlenderNodeBase* node, const std::string& name) const {
		return nameManager.GetOutputIndexByName(node, name);
	}

	bool RemoveInputName(BlenderNodeBase* node, const std::string& name) {
		return nameManager.RemoveInputName(node, name);
	}

	bool RemoveOutputName(BlenderNodeBase* node, const std::string& name) {
		return nameManager.RemoveOutputName(node, name);
	}

	// Blender Input/Output names
	bool RegisterBlenderInputName(size_t index, const std::string& name) {
		return nameManager.RegisterUniqueInputName(index, name);
	}

	bool RegisterBlenderOutputName(size_t index, const std::string& name) {
		return nameManager.RegisterUniqueOutputName(index, name);
	}

	std::string GetBlenderInputName(size_t index) const {
		return nameManager.GetBlenderInputName(index);
	}

	std::string GetBlenderOutputName(size_t index) const {
		return nameManager.GetBlenderOutputName(index);
	}

	size_t GetBlenderInputIndexByName(const std::string& name) const {
		return nameManager.GetBlenderInputIndexByName(name);
	}

	size_t GetBlenderOutputIndexByName(const std::string& name) const {
		return nameManager.GetBlenderOutputIndexByName(name);
	}

	bool RemoveBlenderInputName(const std::string& name) {
		return nameManager.RemoveBlenderInputName(name);
	}

	bool RemoveBlenderOutputName(const std::string& name) {
		return nameManager.RemoveBlenderOutputName(name);
	}

	template<typename T>
	auto GetOutput(std::string const& outputName) const {
		return ioInterface.GetOutputSharedPtr<T>(GetBlenderOutputIndexByName(outputName));
	}

	template<typename T>
	auto GetOutput(size_t index) const {
		return ioInterface.GetOutputSharedPtr<T>(index);
	}

	template<typename T>
	auto GetInput(std::string const& inputName) const {
		return ioInterface.GetInputSharedPtr<T>(GetBlenderInputIndexByName(inputName));
	}

	template<typename T>
	auto GetInput(size_t index) const {
		return ioInterface.GetInputSharedPtr<T>(index);
	}

	template<typename T>
	auto GetUniform() {
		return ioInterface.GetUniformSharedPtr<T>();
	}

	template<typename ...T>
	auto GetUniforms() {
		return ioInterface.GetUniformsTuple<T...>();
	}



	template<int outputNum, typename NodeType>
	auto ExposeOutput(NodeType* node, std::string const& outputName) {
		if constexpr (outputNum >= node->output.GetOutputCount()) {
			return std::shared_ptr<void>{};
		}
		else {
			auto ptr = node->output.template SharedPtr<outputNum>();
			auto ioNum = ioInterface.RegisterOutput(ptr);
			nameManager.RegisterUniqueOutputName(ioNum, outputName);
			return ptr;
		}
	}

	template<typename Type, typename NodeType>
	auto ExposeOutput(NodeType* node, std::string const& outputName) {
		auto ptr = node->output.template SharedPtr<Type>();
		auto ioNum = ioInterface.RegisterOutput(ptr);
		nameManager.RegisterUniqueOutputName(ioNum, outputName);
		return ptr;
	}

	template<int inputNum, typename NodeType>
	auto ExposeInput(NodeType* node, std::string const& inputName) {
		if constexpr (inputNum >= node->input.GetInputCount()) {
			return std::shared_ptr<void>{};
		}
		else {
			auto ptr = node->input.template SharedPtr<inputNum>();
			auto ioNum = ioInterface.RegisterInput(ptr);
			nameManager.RegisterUniqueInputName(ioNum, inputName);
			return ptr;
		}
	}

	template<typename Type, typename NodeType>
	auto ExposeInput(NodeType* node, std::string const& inputName) {
		auto ptr = node->input.template SharedPtr<Type>();
		auto ioNum = ioInterface.RegisterInput(ptr);
		nameManager.RegisterUniqueInputName(ioNum, inputName);
		return ptr;
	}


	bool Connect(Connection const& connection) {
		BlenderNodeBase* fromOutputNode = connection.IsUsingFromOutputNodePointer()
			? std::get<BlenderNodeBase*>(connection.fromOutputNode)
			: GetNodeByName(std::get<std::string>(connection.fromOutputNode));

		BlenderNodeBase* toInputNode = connection.IsUsingToInputNodePointer()
			? std::get<BlenderNodeBase*>(connection.toInputNode)
			: GetNodeByName(std::get<std::string>(connection.toInputNode));

		if (!fromOutputNode || !toInputNode) {
			return false;
		}

		size_t outputIndex = connection.IsUsingNodeOutputIndex()
			? std::get<size_t>(connection.outputIndex)
			: GetOutputIndexByName(fromOutputNode, std::get<std::string>(connection.outputIndex));

		size_t inputIndex = connection.IsUsingNodeInputIndex()
			? std::get<size_t>(connection.inputIndex)
			: GetInputIndexByName(toInputNode, std::get<std::string>(connection.inputIndex));

		if (outputIndex == size_t(-1) || inputIndex == size_t(-1)
			|| outputIndex >= fromOutputNode->GetOutputCount() || inputIndex >= toInputNode->GetInputCount()) {
			return false;
		}

		toInputNode->ConnectInputTo(inputIndex, fromOutputNode, outputIndex);

		return true;
	}

	void Execute() {
		CompileFlowGraph();
		flowInterface.FlushFlow();
		executionCounter++;
		rootNode.PropagateOutflow(executionCounter);
	}

	void CompileFlowGraph() {
		if (!flowGraphDirty) {
			return;
		}
		
		flowInterface.Reset();
		rootNode.flowId = kInvalidFlowId;
		for (auto& node : nodes) {
			if (node) {
				node->flowId = kInvalidFlowId;
			}
		}

		rootNode.EnsureFlowCompiled(flowInterface);
		for (auto& node : nodes) {
			if (node) {
				node->EnsureFlowCompiled(flowInterface);
				node->LinkFlow(flowInterface);
			}
		}

		flowGraphDirty = false;
	}

private:
	void MarkFlowGraphDirty() {
		flowGraphDirty = true;
	}
	
};

}
