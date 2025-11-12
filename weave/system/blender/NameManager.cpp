#include "Blender.h"

using namespace weave;
using namespace weave::blender;

bool NameManager::RegisterNodeName(BlenderNodeBase* node, const std::string& name) {
    if (nodeNameMap.find(name) != nodeNameMap.end()) {
        return false;
    }
    nodeNameMap[name] = node;
    nodeInfoMap[node].nodeName = name;
    return true;
}

BlenderNodeBase* NameManager::GetNodeByName(const std::string& name) const {
    auto it = nodeNameMap.find(name);
    if (it == nodeNameMap.end()) {
        return nullptr;
    }
    return it->second;
}

std::string NameManager::GetNodeName(BlenderNodeBase* node) const {
    auto it = nodeInfoMap.find(node);
    if (it == nodeInfoMap.end()) {
        return "";
    }
    return it->second.nodeName;
}

bool NameManager::RemoveNodeName(const std::string& name) {
    auto node_it = nodeNameMap.find(name);
    if (node_it == nodeNameMap.end()) {
        return false;
    }
    BlenderNodeBase* node = node_it->second;
    nodeNameMap.erase(node_it);
    nodeInfoMap.erase(node);
    return true;
}

bool NameManager::RegisterInputName(BlenderNodeBase* node, size_t index, const std::string& name) {
    NodeInfo& nodeInfo = nodeInfoMap[node];
    if (nodeInfo.inputReverseNames.find(name) != nodeInfo.inputReverseNames.end()) {
        return false;
    }
    nodeInfo.inputNames[index] = name;
    nodeInfo.inputReverseNames[name] = index;
    return true;
}

bool NameManager::RegisterOutputName(BlenderNodeBase* node, size_t index, const std::string& name) {
    NodeInfo& nodeInfo = nodeInfoMap[node];
    if (nodeInfo.outputReverseNames.find(name) != nodeInfo.outputReverseNames.end()) {
        return false;
    }
    nodeInfo.outputNames[index] = name;
    nodeInfo.outputReverseNames[name] = index;
    return true;
}

std::string NameManager::GetInputName(BlenderNodeBase* node, size_t index) const {
    auto it = nodeInfoMap.find(node);
    if (it == nodeInfoMap.end()) {
        return "";
    }
    const NodeInfo& nodeInfo = it->second;
    auto name_it = nodeInfo.inputNames.find(index);
    if (name_it == nodeInfo.inputNames.end()) {
        return "";
    }
    return name_it->second;
}

std::string NameManager::GetOutputName(BlenderNodeBase* node, size_t index) const {
    auto it = nodeInfoMap.find(node);
    if (it == nodeInfoMap.end()) {
        return "";
    }
    const NodeInfo& nodeInfo = it->second;
    auto name_it = nodeInfo.outputNames.find(index);
    if (name_it == nodeInfo.outputNames.end()) {
        return "";
    }
    return name_it->second;
}

size_t NameManager::GetInputIndexByName(BlenderNodeBase* node, const std::string& name) const {
    auto it = nodeInfoMap.find(node);
    if (it == nodeInfoMap.end()) {
        return SIZE_MAX;
    }
    const NodeInfo& nodeInfo = it->second;
    auto index_it = nodeInfo.inputReverseNames.find(name);
    if (index_it == nodeInfo.inputReverseNames.end()) {
        return SIZE_MAX;
    }
    return index_it->second;
}

size_t NameManager::GetOutputIndexByName(BlenderNodeBase* node, const std::string& name) const {
    auto it = nodeInfoMap.find(node);
    if (it == nodeInfoMap.end()) {
        return SIZE_MAX;
    }
    const NodeInfo& nodeInfo = it->second;
    auto index_it = nodeInfo.outputReverseNames.find(name);
    if (index_it == nodeInfo.outputReverseNames.end()) {
        return SIZE_MAX;
    }
    return index_it->second;
}

bool NameManager::RemoveInputName(BlenderNodeBase* node, const std::string& name) {
    auto it = nodeInfoMap.find(node);
    if (it == nodeInfoMap.end()) {
        return false;
    }
    NodeInfo& nodeInfo = it->second;
    auto index_it = nodeInfo.inputReverseNames.find(name);
    if (index_it == nodeInfo.inputReverseNames.end()) {
        return false;
    }
    size_t index = index_it->second;
    nodeInfo.inputNames.erase(index);
    nodeInfo.inputReverseNames.erase(name);
    return true;
}

bool NameManager::RemoveOutputName(BlenderNodeBase* node, const std::string& name) {
    auto it = nodeInfoMap.find(node);
    if (it == nodeInfoMap.end()) {
        return false;
    }
    NodeInfo& nodeInfo = it->second;
    auto index_it = nodeInfo.outputReverseNames.find(name);
    if (index_it == nodeInfo.outputReverseNames.end()) {
        return false;
    }
    size_t index = index_it->second;
    nodeInfo.outputNames.erase(index);
    nodeInfo.outputReverseNames.erase(name);
    return true;
}

bool NameManager::RegisterUniqueInputName(size_t index, const std::string& name) {
    auto it = blenderInputReverseNames.find(name);
    if (it != blenderInputReverseNames.end()) {
        return false;
    }
    blenderInputNames[index] = name;
    blenderInputReverseNames[name] = index;
    return true;
}

bool NameManager::RegisterUniqueOutputName(size_t index, const std::string& name) {
    auto it = blenderOutputReverseNames.find(name);
    if (it != blenderOutputReverseNames.end()) {
        return false;
    }
    blenderOutputNames[index] = name;
    blenderOutputReverseNames[name] = index;
    return true;
}

std::string NameManager::GetBlenderInputName(size_t index) const {
    auto it = blenderInputNames.find(index);
    if (it == blenderInputNames.end()) {
        return "";
    }
    return it->second;
}

std::string NameManager::GetBlenderOutputName(size_t index) const {
    auto it = blenderOutputNames.find(index);
    if (it == blenderOutputNames.end()) {
        return "";
    }
    return it->second;
}

size_t NameManager::GetBlenderInputIndexByName(const std::string& name) const {
    auto it = blenderInputReverseNames.find(name);
    if (it == blenderInputReverseNames.end()) {
        return SIZE_MAX;
    }
    return it->second;
}

size_t NameManager::GetBlenderOutputIndexByName(const std::string& name) const {
    auto it = blenderOutputReverseNames.find(name);
    if (it == blenderOutputReverseNames.end()) {
        return SIZE_MAX;
    }
    return it->second;
}

bool NameManager::RemoveBlenderInputName(const std::string& name) {
    auto it = blenderInputReverseNames.find(name);
    if (it == blenderInputReverseNames.end()) {
        return false;
    }
    size_t index = it->second;
    blenderInputNames.erase(index);
    blenderInputReverseNames.erase(name);
    return true;
}

bool NameManager::RemoveBlenderOutputName(const std::string& name) {
    auto it = blenderOutputReverseNames.find(name);
    if (it == blenderOutputReverseNames.end()) {
        return false;
    }
    size_t index = it->second;
    blenderOutputNames.erase(index);
    blenderOutputReverseNames.erase(name);
    return true;
}
 