#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <typeindex>

#include "weave/system/math/Transform.h"
namespace weave::scenegraph {



class SceneGraph final {
public:
	using NodeIndex = size_t;

	struct NodeExtension {
		Transform *linkedLocalTransform{};
		void *obj{};
		std::type_index type{typeid(void)};

		NodeExtension() = default;

		template<typename T>
		NodeExtension(Transform &transform, T &object)
			: linkedLocalTransform(&transform)
			, obj(&object)
			, type(typeid(T)) {}

		template<typename T>
		explicit NodeExtension(T &object)
			: linkedLocalTransform(nullptr)
			, obj(&object)
			, type(typeid(T)) {}

		explicit NodeExtension(Transform &transform)
			: linkedLocalTransform(&transform)
			, obj(nullptr)
			, type(typeid(void)) {}

		template<typename T>
		T *GetObj() {
			return typeid(T) == type ? static_cast<T*>(obj) : nullptr;
		}

		template<typename T>
		T const *GetObj() const {
			return typeid(T) == type ? static_cast<T const*>(obj) : nullptr;
		}

	};

	struct Node {
		static constexpr NodeIndex kInvalidIndex = static_cast<NodeIndex>(-1);

		std::string name;
		Transform localTransform;
		Matrix4x4 worldTransform;
		NodeIndex self = kInvalidIndex;
		NodeIndex parent = kInvalidIndex;
		std::vector<NodeIndex> children;
		NodeExtension extension;

		bool IsActive() const { return self != kInvalidIndex; }
	};


	SceneGraph();
	~SceneGraph() = default;

	SceneGraph(SceneGraph const &) = delete;
	SceneGraph &operator=(SceneGraph const &) = delete;
	SceneGraph(SceneGraph &&) noexcept = default;
	SceneGraph &operator=(SceneGraph &&) noexcept = default;

	NodeIndex CreateNode(std::string_view name) { return CreateNode(name, 0, NodeExtension{}); }
	NodeIndex CreateNode(std::string_view name, NodeIndex parent);
	NodeIndex CreateNode(std::string_view name, std::string_view parentName);
	NodeIndex CreateNode(std::string_view name, NodeExtension extension) { return CreateNode(name, 0, std::move(extension)); }
	NodeIndex CreateNode(std::string_view name, NodeIndex parent, NodeExtension extension);
	NodeIndex CreateNode(std::string_view name, std::string_view parentName, NodeExtension extension);
	NodeIndex FindNode(std::string_view name) const;
	bool RemoveNode(NodeIndex index);
	bool RemoveNode(std::string_view name);

	void Reset();

	void UpdateWorldTransforms();
	
	template<typename NodeVisitor>
	void Traverse(NodeVisitor const &visitor) const {
		BuildTraversalCache();

		for (NodeIndex index : traversalCache) {
			auto const &node = nodes[index];
			visitor(index, node);
		}
	}

	template<typename NodeFn>
	bool TouchNode(NodeIndex index, NodeFn &&fn) {
		if (auto *node = AccessNode(index)) {
			fn(node->localTransform, node->extension);
			return true;
		}
		return false;
	}

	template<typename NodeFn>
	bool TouchNode(std::string_view name, NodeFn &&fn) {
		return TouchNode(FindNode(name), std::forward<NodeFn>(fn));
	}

private:
	std::vector<Node> nodes;
	std::vector<NodeIndex> freeList;
	std::unordered_map<std::string, NodeIndex> nameToIndex;
	mutable std::vector<NodeIndex> traversalCache;
	mutable std::vector<NodeIndex> traverseStack;

	NodeIndex FindNodeIndex(std::string_view name) const;
	Node *AccessNode(NodeIndex index);
	Node const *AccessNode(NodeIndex index) const;
	void RemoveSingleNode(NodeIndex index);
	void BuildTraversalCache() const;

	template<typename Visitor>
	void TraverseNonRecursive(NodeIndex startNode, Visitor&& visitor) const {
		traverseStack.clear();

		if (startNode == Node::kInvalidIndex || startNode >= nodes.size() || !nodes[startNode].IsActive()) {
			return;
		}

		for (auto child : nodes[startNode].children) {
			if (nodes[child].self != Node::kInvalidIndex) {
				traverseStack.push_back(child);
			}
		}

		while (!traverseStack.empty()) {
			NodeIndex index = traverseStack.back();
			traverseStack.pop_back();
			for (auto child : nodes[index].children) {
				if (nodes[child].self != Node::kInvalidIndex) {
					traverseStack.push_back(child);
				}
			}

			visitor(index);
		}
	}
};





}
