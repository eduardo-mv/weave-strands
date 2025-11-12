#pragma once

#include <string>
#include <unordered_map>

namespace weave::blender {

struct BlenderNodeBase;

class NameManager {
public:
	// Node naming
	bool RegisterNodeName(BlenderNodeBase* node, const std::string& name);
	BlenderNodeBase* GetNodeByName(const std::string& name) const;
	std::string GetNodeName(BlenderNodeBase* node) const;
	bool RemoveNodeName(const std::string& name);

	// Per-node Input/Output naming
	bool RegisterInputName(BlenderNodeBase* node, size_t index, const std::string& name);
	bool RegisterOutputName(BlenderNodeBase* node, size_t index, const std::string& name);
	std::string GetInputName(BlenderNodeBase* node, size_t index) const;
	std::string GetOutputName(BlenderNodeBase* node, size_t index) const;
	size_t GetInputIndexByName(BlenderNodeBase* node, const std::string& name) const;
	size_t GetOutputIndexByName(BlenderNodeBase* node, const std::string& name) const;
	bool RemoveInputName(BlenderNodeBase* node, const std::string& name);
	bool RemoveOutputName(BlenderNodeBase* node, const std::string& name);

	// Unique Input/Output naming
	bool RegisterUniqueInputName(size_t index, const std::string& name);
	bool RegisterUniqueOutputName(size_t index, const std::string& name);
	std::string GetBlenderInputName(size_t index) const;
	std::string GetBlenderOutputName(size_t index) const;
	size_t GetBlenderInputIndexByName(const std::string& name) const;
	size_t GetBlenderOutputIndexByName(const std::string& name) const;
	bool RemoveBlenderInputName(const std::string& name);
	bool RemoveBlenderOutputName(const std::string& name);

private:
	struct NodeInfo {
		std::string nodeName;
		std::unordered_map<size_t, std::string> inputNames;
		std::unordered_map<size_t, std::string> outputNames;
		std::unordered_map<std::string, size_t> inputReverseNames;
		std::unordered_map<std::string, size_t> outputReverseNames;
	};

	std::unordered_map<std::string, BlenderNodeBase*> nodeNameMap;
	std::unordered_map<BlenderNodeBase*, NodeInfo> nodeInfoMap;

	std::unordered_map<size_t, std::string> blenderInputNames;
	std::unordered_map<size_t, std::string> blenderOutputNames;
	std::unordered_map<std::string, size_t> blenderInputReverseNames;
	std::unordered_map<std::string, size_t> blenderOutputReverseNames;
};

}