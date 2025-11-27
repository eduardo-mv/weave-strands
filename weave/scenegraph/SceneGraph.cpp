#include "SceneGraph.h"

#include <stdexcept>
#include <algorithm>

namespace weave::scenegraph {

SceneGraph::SceneGraph() {
	Reset();
}

SceneGraph::NodeIndex SceneGraph::CreateNode(std::string_view name, NodeIndex parent) {
	return CreateNode(name, parent, NodeExtension{});
}

SceneGraph::NodeIndex SceneGraph::CreateNode(std::string_view name, std::string_view parentName) {
	return CreateNode(name, FindNode(parentName), NodeExtension{});
}

SceneGraph::NodeIndex SceneGraph::CreateNode(std::string_view name, NodeIndex parent, NodeExtension extension) {
	if (parent == Node::kInvalidIndex || parent >= nodes.size() || !nodes[parent].IsActive()) {
		throw std::runtime_error("Invalid parent index for SceneGraph::CreateNode");
	}

	std::string nameCopy(name);
	if (!nameCopy.empty() && nameToIndex.contains(nameCopy)) {
		throw std::runtime_error("SceneGraph::CreateNode duplicate name");
	}

	NodeIndex index = Node::kInvalidIndex;
	if (!freeList.empty()) {
		index = freeList.back();
		freeList.pop_back();
	} else {
		nodes.emplace_back();
		index = nodes.size() - 1;
	}

	Node &node = index >= nodes.size() ? nodes.back() : nodes[index];
	node.name = std::move(nameCopy);
	node.self = index;
	node.parent = parent;
	node.children.clear();
	node.localTransform = Transform();
	node.extension = std::move(extension);
	node.worldTransform.Identity();

	nodes[parent].children.push_back(index);
	if (!node.name.empty()) {
		nameToIndex.emplace(node.name, index);
	}

	traversalCache.clear();
	return index;
}

SceneGraph::NodeIndex SceneGraph::CreateNode(std::string_view name, std::string_view parentName, NodeExtension extension) {
	return CreateNode(name, FindNode(parentName), std::move(extension));
}

SceneGraph::NodeIndex SceneGraph::FindNode(std::string_view name) const {
	return FindNodeIndex(name);
}

bool SceneGraph::RemoveNode(NodeIndex index) {
	if (index == 0 || index == Node::kInvalidIndex || index >= nodes.size()) {
		return false;
	}

	Node &node = nodes[index];
	if (!node.IsActive()) {
		return false;
	}

	NodeIndex parentIndex = node.parent;
	if (parentIndex != Node::kInvalidIndex && parentIndex < nodes.size()) {
		auto &siblings = nodes[parentIndex].children;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), index), siblings.end());
	}

	RemoveSingleNode(index);
	TraverseNonRecursive(index, [&](NodeIndex index) {
		RemoveSingleNode(index);
	});

	traversalCache.clear();
	BuildTraversalCache();
	return true;
}

bool SceneGraph::RemoveNode(std::string_view name) {
	return RemoveNode(FindNode(name));
}

void SceneGraph::Reset() {
	nodes.clear();
	nameToIndex.clear();
	freeList.clear();
	traversalCache.clear();
	traverseStack.clear();

	nodes.emplace_back();
	Node &root = nodes.back();
	root.self = 0;
	root.parent = Node::kInvalidIndex;
	root.children.clear();
	root.name.clear();
	root.localTransform = Transform();
	root.extension = NodeExtension();
	root.worldTransform.Identity();
}

void SceneGraph::UpdateWorldTransforms() {
	BuildTraversalCache();

	for (NodeIndex index : traversalCache) {
		Node &node = nodes[index];
		Matrix4x4 local = 
			node.extension.linkedLocalTransform 
			? node.localTransform.GetTransformMatrix() * node.extension.linkedLocalTransform->GetTransformMatrix() 
			: node.localTransform.GetTransformMatrix();
		node.worldTransform = nodes[node.parent].worldTransform * local;
	}
}

void SceneGraph::BuildTraversalCache() const {
	if (!traversalCache.empty()) {
		return;
	}

	traversalCache.reserve(nodes.size());

	TraverseNonRecursive(0, [&](NodeIndex index) {
		traversalCache.push_back(index);
	});
}

SceneGraph::NodeIndex SceneGraph::FindNodeIndex(std::string_view name) const {
	if (name.empty()) {
		return Node::kInvalidIndex;
	}

	auto it = nameToIndex.find(std::string(name));
	if (it == nameToIndex.end()) {
		return Node::kInvalidIndex;
	}
	return it->second;
}

SceneGraph::Node *SceneGraph::AccessNode(NodeIndex index) {
	if (index >= nodes.size()) {
		return nullptr;
	}
	Node &node = nodes[index];
	if (!node.IsActive()) {
		return nullptr;
	}
	return &node;
}

SceneGraph::Node const *SceneGraph::AccessNode(NodeIndex index) const {
	if (index >= nodes.size()) {
		return nullptr;
	}
	Node const &node = nodes[index];
	if (!node.IsActive()) {
		return nullptr;
	}
	return &node;
}

void SceneGraph::RemoveSingleNode(NodeIndex index) {
	if (index == Node::kInvalidIndex || index >= nodes.size()) {
		return;
	}

	Node &node = nodes[index];
	if (!node.IsActive()) {
		return;
	}

	if (!node.name.empty()) {
		nameToIndex.erase(node.name);
	}

	node.children.clear();
	node.name.clear();
	node.self = Node::kInvalidIndex;
	node.parent = Node::kInvalidIndex;
	node.localTransform = Transform();
	node.extension = NodeExtension();
	node.worldTransform.Identity();
	freeList.push_back(index);
}

}
