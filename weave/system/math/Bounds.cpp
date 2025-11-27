#include "Bounds.h"

#include <algorithm>

#include "TransformState.h"
#include "TransformUtils.h"
#include "VectorMath.h"

namespace weave::bounds {

namespace {

constexpr int kNumFrustumCorners = 8;

void ExtractFrustumCorners(Matrix4x4 const &viewProj, Vector3 corners[8]) {
	Matrix4x4 inv = viewProj.Inverted();
	static Vector3 ndcCorners[kNumFrustumCorners] = {
		{ -1.0f, -1.0f, 0.0f },
		{ 1.0f, -1.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ -1.0f, 1.0f, 0.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f }
	};

	for (int i = 0; i < kNumFrustumCorners; ++i) {
		Vector4 corner(ndcCorners[i], 1.0f);
		Vector4 worldCorner = inv * corner;
		worldCorner /= worldCorner.w;
		corners[i] = worldCorner.xyz;
	}
}

Sphere SphereFromFrustumCorners(Vector3 const corners[8]) {
	Vector3 center = Vector3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < kNumFrustumCorners; ++i) {
		center += corners[i];
	}
	center /= static_cast<float>(kNumFrustumCorners);

	float radius = 0.0f;
	for (int i = 0; i < kNumFrustumCorners; ++i) {
		float dist = weave::algebra::length(corners[i] - center);
		radius = std::max(radius, dist);
	}

	return { center, radius };
}

void BuildFrustumPlanes(Vector3 const corners[8], Plane planes[6]) {
	using weave::algebra::makeplane;
	planes[0] = makeplane(corners[0], corners[1], corners[2]); // Near
	planes[1] = makeplane(corners[5], corners[4], corners[7]); // Far
	planes[2] = makeplane(corners[4], corners[0], corners[3]); // Left
	planes[3] = makeplane(corners[1], corners[5], corners[6]); // Right
	planes[4] = makeplane(corners[3], corners[2], corners[6]); // Top
	planes[5] = makeplane(corners[4], corners[5], corners[1]); // Bottom
}

}

Frustum BuildFrustum(Matrix4x4 const &viewProjection) {
	Frustum frustum {};
	ExtractFrustumCorners(viewProjection, frustum.corners);
	BuildFrustumPlanes(frustum.corners, frustum.planes);
	return frustum;
}

Sphere BuildBoundingSphere(Matrix4x4 const &viewProjection) {
	Vector3 corners[kNumFrustumCorners];
	ExtractFrustumCorners(viewProjection, corners);
	return SphereFromFrustumCorners(corners);
}

AABB BuildAABBForPoints(Vector3 const *points, size_t count) {
	if (count == 0) {
		return {};
	}

	Vector3 min = points[0];
	Vector3 max = points[0];
	for (size_t i = 1; i < count; ++i) {
		min = weave::algebra::mincomps(min, points[i]);
		max = weave::algebra::maxcomps(max, points[i]);
	}

	return { min, max };
}

OrthoVolume BuildOrthoVolume(TransformState const &state, float width, float height, float zNear, float zFar) {
	Vector3 extents(width * 0.5f, height * 0.5f, (zFar - zNear) * 0.5f);
	Vector3 forward = state.orientation * Vector3(0.0f, 0.0f, -1.0f);
	if (!weave::algebra::normalizecond(forward)) {
		forward = Vector3(0.0f, 0.0f, -1.0f);
	}
	Vector3 center = state.position + forward * (zNear + extents.z);
	return { center, extents };
}

}
