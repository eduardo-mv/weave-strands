#include "tests/TestEntryPoints.h"

#include "weave/scenegraph/SceneGraph.h"
#include "weave/system/math/Transform.h"
#include "weave/system/math/VectorMath.h"

namespace weave::tests::scenegraph {

TestReport RunSceneGraphSelfTest() {
	using weave::scenegraph::SceneGraph;
	using weave::algebra::equivalent;

	TestReport report;

	SceneGraph graph;

	auto rootChildIndex = graph.CreateNode("root_child");
	[[maybe_unused]] auto branchIndex = graph.CreateNode("branch", rootChildIndex);
	auto leafIndex = graph.CreateNode("leaf", "branch");

	report.Expect(graph.FindNode("leaf") == leafIndex, "Leaf lookup mismatch");

	Transform myTransform;
	int branchPayloadTag = 1;
	const bool touchedRootChild = graph.TouchNode(rootChildIndex, [](Transform& transform, SceneGraph::NodeExtension&) {
		transform.Translate(1.0f, 0.0f, 0.0f);
	});
	const bool touchedBranch = graph.TouchNode("branch", [&](Transform& transform, SceneGraph::NodeExtension& extension) {
		transform.Translate(0.0f, 2.0f, 0.0f);
		extension = SceneGraph::NodeExtension(branchPayloadTag);
	});
	const bool touchedLeaf = graph.TouchNode(leafIndex, [&](Transform& transform, SceneGraph::NodeExtension& extension) {
		transform.Translate(0.0f, 0.0f, 3.0f);
		extension.linkedLocalTransform = &myTransform;
	});

	report.Expect(touchedRootChild && touchedBranch && touchedLeaf, "Failed to touch expected nodes");

	int payloadOnlyTag = 11;
	SceneGraph::NodeExtension payloadOnlyExtension(payloadOnlyTag);
	graph.CreateNode("payload_only", rootChildIndex, payloadOnlyExtension);

	Transform transformOnlyLinked;
	transformOnlyLinked.Translate(0.0f, 5.0f, 0.0f);
	SceneGraph::NodeExtension transformOnlyExtension(transformOnlyLinked);
	graph.CreateNode("transform_only", "payload_only", transformOnlyExtension);

	int extensionPayloadTag = 7;
	Transform extensionLinkedTransform;
	extensionLinkedTransform.Translate(5.0f, 0.0f, 0.0f);
	SceneGraph::NodeExtension linkedExtension(extensionLinkedTransform, extensionPayloadTag);
	graph.CreateNode("extension_child", "root_child", linkedExtension);

	graph.UpdateWorldTransforms();

	Vector3 rootChildWorld{};
	Vector3 branchWorld{};
	Vector3 leafWorld{};
	Vector3 extensionChildWorld{};
	Vector3 transformOnlyWorld{};
	bool rootChildSeen = false;
	bool branchSeen = false;
	bool leafSeen = false;
	bool extensionChildSeen = false;
	bool payloadOnlySeen = false;
	bool transformOnlySeen = false;
	bool branchExtensionStored = false;
	bool leafExtensionLinked = false;
	bool extensionChildPayloadOk = false;
	bool extensionChildLinkedOk = false;
	bool payloadOnlyPayloadOk = false;
	bool transformOnlyLinkedOk = false;

	graph.Traverse([&](SceneGraph::NodeIndex, SceneGraph::Node const& node) {
		if (node.name.empty()) {
			return;
		}

		Vector3 position(node.worldTransform.W.x, node.worldTransform.W.y, node.worldTransform.W.z);
		if (node.name == "root_child") {
			rootChildWorld = position;
			rootChildSeen = true;
		} else if (node.name == "branch") {
			branchWorld = position;
			branchSeen = true;
			branchExtensionStored = node.extension.obj == &branchPayloadTag;
		} else if (node.name == "leaf") {
			leafWorld = position;
			leafSeen = true;
			leafExtensionLinked = node.extension.linkedLocalTransform == &myTransform;
		} else if (node.name == "extension_child") {
			extensionChildWorld = position;
			extensionChildSeen = true;
			extensionChildPayloadOk = node.extension.obj == &extensionPayloadTag;
			extensionChildLinkedOk = node.extension.linkedLocalTransform == &extensionLinkedTransform;
		} else if (node.name == "payload_only") {
			payloadOnlySeen = true;
			payloadOnlyPayloadOk = node.extension.obj == &payloadOnlyTag;
		} else if (node.name == "transform_only") {
			transformOnlyWorld = position;
			transformOnlySeen = true;
			transformOnlyLinkedOk = node.extension.linkedLocalTransform == &transformOnlyLinked;
		}
	});

	report.Expect(rootChildSeen, "root_child not seen during traversal");
	report.Expect(branchSeen, "branch not seen during traversal");
	report.Expect(leafSeen, "leaf not seen during traversal");

	const bool transformsOk = rootChildSeen && branchSeen && leafSeen
		&& equivalent(rootChildWorld, Vector3(1.0f, 0.0f, 0.0f))
		&& equivalent(branchWorld, Vector3(1.0f, 2.0f, 0.0f))
		&& equivalent(leafWorld, Vector3(1.0f, 2.0f, 3.0f));
	report.Expect(transformsOk, "Unexpected world transforms");

	const bool extensionChildTransformOk = extensionChildSeen
		&& equivalent(extensionChildWorld, Vector3(6.0f, 0.0f, 0.0f));
	report.Expect(extensionChildTransformOk, "Extension child transform mismatch");
	report.Expect(payloadOnlySeen && payloadOnlyPayloadOk, "Payload-only node missing or payload mismatch");
	report.Expect(transformOnlySeen && transformOnlyLinkedOk, "Transform-only node missing or link mismatch");
	const bool transformOnlyTransformOk = transformOnlySeen
		&& equivalent(transformOnlyWorld, Vector3(1.0f, 5.0f, 0.0f));
	report.Expect(transformOnlyTransformOk, "Transform-only world transform mismatch");
	report.Expect(branchExtensionStored, "Branch extension payload not stored");
	report.Expect(leafExtensionLinked, "Leaf extension not linked");
	report.Expect(extensionChildPayloadOk && extensionChildLinkedOk, "Extension child linkage mismatch");

	const bool removeLeafByIndex = graph.RemoveNode(leafIndex);
	report.Expect(removeLeafByIndex, "Failed to remove leaf by index");
	const bool lookupAfterLeafRemoval = graph.FindNode("leaf") == SceneGraph::Node::kInvalidIndex;
	report.Expect(lookupAfterLeafRemoval, "Leaf lookup succeeded after removal");

	auto reusedLeafIndex = graph.CreateNode("reused_leaf", "branch");
	report.Expect(reusedLeafIndex == leafIndex, "Leaf slot was not reused");

	const bool removeReusedByName = graph.RemoveNode("reused_leaf");
	report.Expect(removeReusedByName, "Failed to remove reused leaf by name");
	const bool removeBranchByName = graph.RemoveNode("branch");
	report.Expect(removeBranchByName, "Failed to remove branch by name");
	const bool branchMissing = graph.FindNode("branch") == SceneGraph::Node::kInvalidIndex;
	report.Expect(branchMissing, "Branch lookup succeeded after removal");
	const bool missingNodeLookup = graph.FindNode("missing") == SceneGraph::Node::kInvalidIndex;
	report.Expect(missingNodeLookup, "Missing node lookup returned valid index");

	const bool removeRootFails = !graph.RemoveNode(0);
	report.Expect(removeRootFails, "Removing root unexpectedly succeeded");
	const bool touchInvalidIndex = !graph.TouchNode(SceneGraph::Node::kInvalidIndex, [](Transform& transform, SceneGraph::NodeExtension&) {
		transform.Translate(42.0f, 0.0f, 0.0f);
	});
	report.Expect(touchInvalidIndex, "Touching invalid index unexpectedly succeeded");
	const bool touchMissingName = !graph.TouchNode("missing_name", [](Transform& transform, SceneGraph::NodeExtension&) {
		transform.Translate(0.0f, 42.0f, 0.0f);
	});
	report.Expect(touchMissingName, "Touching missing name unexpectedly succeeded");

	graph.Reset();
	const bool resetClearsNames = graph.FindNode("root_child") == SceneGraph::Node::kInvalidIndex
		&& graph.FindNode("branch") == SceneGraph::Node::kInvalidIndex;
	report.Expect(resetClearsNames, "Reset did not clear names");
	const auto postResetIndex = graph.CreateNode("post_reset");
	report.Expect(postResetIndex != SceneGraph::Node::kInvalidIndex, "Failed to create node after reset");

	return report;
}

} // namespace weave::tests::scenegraph
