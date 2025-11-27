#pragma once

#include "VectorMath.h"
#include "TransformState.h"

namespace weave::bounds {

struct Sphere {
	Vector3 center;
	float radius = 0.0f;
};

struct AABB {
	Vector3 min;
	Vector3 max;
};

struct Frustum {
	Plane planes[6];
	Vector3 corners[8];
};

struct OrthoVolume {
	Vector3 center;
	Vector3 extents;
};

Frustum BuildFrustum(Matrix4x4 const &viewProjection);
Sphere BuildBoundingSphere(Matrix4x4 const &viewProjection);
AABB BuildAABBForPoints(Vector3 const *points, size_t count);
OrthoVolume BuildOrthoVolume(TransformState const &state, float width, float height, float zNear, float zFar);

}
