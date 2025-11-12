/*
Title: "Ray picking support library"
File: RayPicking.h
Author(s): Eduardo Martínez Vidal

Abstract:
	Support functions for generic ray picking

*/

#pragma once

#include "VectorMath.h"
#include "Transform.h"

namespace weave {

namespace raypicking {

	struct Ray {
		Vector3 origin;
		Vector3 direction;
		float length = 0.0f; //If positive, ray is considered a segment
	};

	struct PickResult {
		Vector3 surfaceNormal;
		float pointAlongRay = -1.0f;

		operator bool() const { return pointAlongRay >= 0.0f; }
	};

	inline bool isSegment(Ray const& ray) { return ray.length > 0.0f; }

	inline Vector3 pointAlongRay(Ray const& ray, float t) { 
		return ray.origin + ray.direction * t;	
	}

	//Returns the ray casted by the normalized window coordinate with the origin set to the near plan
	Ray pickingRay(weave::Matrix4x4 const &unprojection, weave::Matrix4x4 const &invCamera, float cursorx, float cursory);

	PickResult pickSphere(Ray const& ray, Vector3 const& spherePosition, float sphereRadius);
	PickResult pickBox(Ray const& ray, Transform const& boxTransform, Vector3 const& boxHalfSize);
	std::pair<PickResult, bool> pickFrustum(Ray const& ray, Plane const frustumPlanes[6], bool exitOnFirstHit);

}

} //namespace weave
