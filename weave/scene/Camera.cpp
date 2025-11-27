#include "Camera.h"

#include <iterator>

#include "weave/system/math/TransformUtils.h"
#include "weave/system/math/VectorMath.h"

namespace weave::scene {

namespace {
Plane MirrorPlane() {
	return Plane(0.0f, 1.0f, 0.0f, 0.0f);
}
}

Camera::Camera() {
	cameraState.Touch([](CameraState &state) {
		state.customProjection.Identity();
	});
}

void Camera::SetPerspective(float fieldOfViewDegrees, float aspect, float nearPlane, float farPlane) {
	cameraState.Touch([&](CameraState &state) {
		state.projectionMode = ProjectionMode::Perspective;
		state.fov = fieldOfViewDegrees;
		state.aspectRatio = aspect;
		state.zNear = nearPlane;
		state.zFar = farPlane;
	});
}

void Camera::SetOrthographic(float width, float aspect, float nearPlane, float farPlane) {
	cameraState.Touch([&](CameraState &state) {
		state.projectionMode = ProjectionMode::Orthographic;
		state.orthoWidth = width;
		state.aspectRatio = aspect;
		state.zNear = nearPlane;
		state.zFar = farPlane;
	});
}

void Camera::SetCustomProjection(Matrix4x4 const &matrix) {
	cameraState.Touch([&](CameraState &state) {
		state.projectionMode = ProjectionMode::Custom;
		state.customProjection = matrix;
	});
}

void Camera::SetProjectionMode(ProjectionMode mode) {
		ProjectionMode current = cameraState.Snapshot([](CameraState const &snapshot) {
			return snapshot.projectionMode;
		});
	if (current == mode) {
		return;
	}

	cameraState.Touch([&](CameraState &state) {
		state.projectionMode = mode;
	});
}

Camera::ProjectionMode Camera::GetProjectionMode() const {
	return cameraState.Snapshot([](CameraState const &snapshot) {
		return snapshot.projectionMode;
	});
}

void Camera::SetViewport(Viewport const &viewport_) {
	cameraState.Touch([&](CameraState &state) {
		state.viewport = viewport_;
	});
}

	Camera::Viewport Camera::GetViewport() const {
		return cameraState.Snapshot([](CameraState const &snapshot) {
			return snapshot.viewport;
		});
}

void Camera::SetAspectRatio(float aspect) {
	cameraState.Touch([&](CameraState &state) {
		state.aspectRatio = aspect;
	});
}

	float Camera::GetAspectRatio() const {
		return cameraState.Snapshot([](CameraState const &snapshot) {
			return snapshot.aspectRatio;
		});
}

void Camera::SetClipPlanes(float nearPlane, float farPlane) {
	cameraState.Touch([&](CameraState &state) {
		state.zNear = nearPlane;
		state.zFar = farPlane;
	});
}

	float Camera::GetNearPlane() const {
		return cameraState.Snapshot([](CameraState const &snapshot) {
			return snapshot.zNear;
		});
}

float Camera::GetFarPlane() const {
	return cameraState.Snapshot([](CameraState const &snapshot) {
		return snapshot.zFar;
	});
}

void Camera::SetFieldOfView(float degrees) {
	cameraState.Touch([&](CameraState &state) {
		state.fov = degrees;
	});
}

float Camera::GetFieldOfView() const {
	return cameraState.Snapshot([](CameraState const &snapshot) {
		return snapshot.fov;
	});
}

void Camera::SetOrthographicWidth(float width) {
	cameraState.Touch([&](CameraState &state) {
		state.orthoWidth = width;
	});
}

float Camera::GetOrthographicWidth() const {
	return cameraState.Snapshot([](CameraState const &snapshot) {
		return snapshot.orthoWidth;
	});
}

void Camera::SetZoom(float zoomFactor) {
	cameraState.Touch([&](CameraState &state) {
		state.zoomScale = zoomFactor;
	});
}

float Camera::GetZoom() const {
	return cameraState.Snapshot([](CameraState const &snapshot) {
		return snapshot.zoomScale;
	});
}

Matrix4x4 Camera::GetProjectionMatrix() const {
	auto [snapshot, revision] = cameraState.SnapshotAndRevision();
	return matrixCache.GetProjectionMatrix(snapshot, revision);
}

Matrix4x4 Camera::GetViewMatrix() const {
	return transform.GetInverseTransformMatrix();
}

Matrix4x4 Camera::GetViewProjectionMatrix() const {
	auto [snapshot, cameraRevision] = cameraState.SnapshotAndRevision();
	Matrix4x4 projection = matrixCache.GetProjectionMatrix(snapshot, cameraRevision);

	Matrix4x4 viewMatrix = GetViewMatrix();
	uint64_t transformRevision = transform.GetRevision();
	uint64_t combinedRevision = CombineRevisions(cameraRevision, transformRevision);
	return matrixCache.GetViewProjectionMatrix(projection, viewMatrix, combinedRevision);
}

Matrix4x4 Camera::CalculateUnprojectionMatrix(float screenWidth, float screenHeight) const {
	Matrix4x4 projection = GetProjectionMatrix();
	Matrix4x4 screenbias(
		0.5f * screenWidth, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f * screenHeight, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f * screenWidth, 0.5f * screenHeight, 0.5f, 1.0f
	);

	return algebra::invert(screenbias * projection);
}

void Camera::LookAt(Vector3 const &target, Vector3 const &worldUp, bool frontIsZPositive) {
	transform.LookAt(target, worldUp, frontIsZPositive);
}

void Camera::Reflect(Plane const &plane) {
	Matrix4x4 reflection = Matrix4x4::ReflectionCtor(plane);
	Matrix4x4 mirror = Matrix4x4::ReflectionCtor(MirrorPlane());
	Matrix4x4 world = transform.GetTransformMatrix();
	Matrix4x4 reflectedMatrix = reflection * world * mirror;
	transform = transform_utils::FromMatrix(reflectedMatrix);
}

bounds::Frustum Camera::GetFrustumBounds() const {
	return bounds::BuildFrustum(GetViewProjectionMatrix());
}

bounds::Sphere Camera::GetBoundingSphere() const {
	return bounds::BuildBoundingSphere(GetViewProjectionMatrix());
}

bounds::AABB Camera::GetFrustumAABB() const {
	auto frustum = GetFrustumBounds();
	return bounds::BuildAABBForPoints(frustum.corners, std::size(frustum.corners));
}

bounds::OrthoVolume Camera::GetOrthoVolume() const {
	CameraState snapshot = cameraState.Snapshot();
	if (snapshot.projectionMode != ProjectionMode::Orthographic) {
		return {};
	}

	float width = snapshot.orthoWidth * snapshot.zoomScale;
	float height = (snapshot.aspectRatio > 0.0f) ? (width / snapshot.aspectRatio) : width;
	return bounds::BuildOrthoVolume(transform.GetState(), width, height, snapshot.zNear, snapshot.zFar);
}

uint64_t Camera::CombineRevisions(uint64_t cameraRevision, uint64_t transformRevision) const {
	constexpr uint64_t mask = 0xffffffffull;
	return (cameraRevision << 32) | (transformRevision & mask);
}

Matrix4x4 Camera::CameraMatrixCache::GetProjectionMatrix(CameraState const &state, uint64_t cameraRevision) const {
	UpdateProjectionCache(state, cameraRevision);
	return projectionCache.GetCache([](ProjectionCacheEntry const &entry) {
		return entry.projection;
	});
}

Matrix4x4 Camera::CameraMatrixCache::GetInverseProjectionMatrix(CameraState const &state, uint64_t cameraRevision) const {
	UpdateProjectionCache(state, cameraRevision);
	return projectionCache.GetCache([](ProjectionCacheEntry const &entry) {
		return entry.inverseProjection;
	});
}

Matrix4x4 Camera::CameraMatrixCache::GetViewProjectionMatrix(Matrix4x4 const &projection, Matrix4x4 const &viewMatrix, uint64_t combinedRevision) const {
	UpdateViewProjectionCache(projection, viewMatrix, combinedRevision);
	return viewProjectionCache.GetCache([](ViewProjectionCacheEntry const &entry) {
		return entry.viewProjection;
	});
}

Matrix4x4 Camera::CameraMatrixCache::BuildProjectionMatrix(CameraState const &state) const {
	switch (state.projectionMode) {
	case ProjectionMode::Perspective: {
		float fovRadians = algebra::deg2rad(state.fov * state.zoomScale);
		return Matrix4x4::PerspectiveRadCtor(fovRadians, state.aspectRatio, state.zNear, state.zFar);
	}
	case ProjectionMode::Orthographic: {
		float width = state.orthoWidth * state.zoomScale;
		float height = (state.aspectRatio > 0.0f) ? (width / state.aspectRatio) : width;
		float halfWidth = width * 0.5f;
		float halfHeight = height * 0.5f;
		return Matrix4x4::OrthoCtor(-halfWidth, halfWidth, -halfHeight, halfHeight, state.zNear, state.zFar);
	}
	case ProjectionMode::Custom:
	default:
		return state.customProjection;
	}
}

void Camera::CameraMatrixCache::UpdateProjectionCache(CameraState const &state, uint64_t revision) const {
	projectionCache.UpdateCache(revision, [&](ProjectionCacheEntry &entry, uint64_t &entryRevision) {
		entry.projection = BuildProjectionMatrix(state);
		entry.inverseProjection = entry.projection.Inverted();
		entryRevision = revision;
	});
}

void Camera::CameraMatrixCache::UpdateViewProjectionCache(Matrix4x4 const &projection, Matrix4x4 const &viewMatrix, uint64_t revision) const {
	viewProjectionCache.UpdateCache(revision, [&](ViewProjectionCacheEntry &entry, uint64_t &entryRevision) {
		entry.viewProjection = projection * viewMatrix;
		entryRevision = revision;
	});
}

}
