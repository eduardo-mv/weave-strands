#pragma once

#include "weave/system/math/Transform.h"
#include "weave/system/math/Bounds.h"
#include "weave/system/memory/BufferedCache.h"
#include "weave/system/memory/ProtectedState.h"

namespace weave::scene {

class Camera final {
public:
	enum class ProjectionMode {
		Perspective,
		Orthographic,
		Custom,
	};

	struct Viewport {
		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
	};

	Camera();

	void SetPerspective(float fieldOfViewDegrees, float aspect, float zNear, float zFar);
	void SetOrthographic(float width, float aspect, float zNear, float zFar);
	void SetCustomProjection(Matrix4x4 const &matrix);

	void SetProjectionMode(ProjectionMode mode);
	ProjectionMode GetProjectionMode() const;

	void SetViewport(Viewport const &viewport);
	Viewport GetViewport() const;

	void SetAspectRatio(float aspect);
	float GetAspectRatio() const;

	void SetClipPlanes(float zNear, float zFar);
	float GetNearPlane() const;
	float GetFarPlane() const;

	void SetFieldOfView(float degrees);
	float GetFieldOfView() const;

	void SetOrthographicWidth(float width);
	float GetOrthographicWidth() const;

	void SetZoom(float zoomFactor);
	float GetZoom() const;

	Matrix4x4 GetProjectionMatrix() const;
	Matrix4x4 GetViewMatrix() const;
	Matrix4x4 GetViewProjectionMatrix() const;
	Matrix4x4 CalculateUnprojectionMatrix(float screenWidth, float screenHeight) const;

	Transform const &GetTransform() const { return transform; }
	Transform &GetTransform() { return transform; }

	void LookAt(Vector3 const &target, Vector3 const &worldUp = Vector3(0.0f, 1.0f, 0.0f), bool frontIsZPositive = true);
	void Reflect(Plane const &plane);

	bounds::Frustum GetFrustumBounds() const;
	bounds::Sphere GetBoundingSphere() const;
	bounds::AABB GetFrustumAABB() const;
	bounds::OrthoVolume GetOrthoVolume() const;

private:
	struct CameraState {
		ProjectionMode projectionMode = ProjectionMode::Perspective;
		Viewport viewport{};
		float zNear = 0.1f;
		float zFar = 1000.0f;
		float fov = 65.0f;
		float orthoWidth = 1000.0f;
		float aspectRatio = 16.0f / 9.0f;
		float zoomScale = 1.0f;
		Matrix4x4 customProjection;
	};

	class CameraMatrixCache final {
	public:
		Matrix4x4 GetProjectionMatrix(CameraState const &state, uint64_t cameraRevision) const;
		Matrix4x4 GetInverseProjectionMatrix(CameraState const &state, uint64_t cameraRevision) const;
		Matrix4x4 GetViewProjectionMatrix(Matrix4x4 const &projection, Matrix4x4 const &viewMatrix, uint64_t combinedRevision) const;

	private:
		struct ProjectionCacheEntry {
			Matrix4x4 projection;
			Matrix4x4 inverseProjection;
		};

		struct ViewProjectionCacheEntry {
			Matrix4x4 viewProjection;
		};

		Matrix4x4 BuildProjectionMatrix(CameraState const &state) const;
		void UpdateProjectionCache(CameraState const &state, uint64_t revision) const;
		void UpdateViewProjectionCache(Matrix4x4 const &projection, Matrix4x4 const &viewMatrix, uint64_t revision) const;

		mutable weave::BufferedCache<ProjectionCacheEntry> projectionCache;
		mutable weave::BufferedCache<ViewProjectionCacheEntry> viewProjectionCache;
	};

		uint64_t CombineRevisions(uint64_t cameraRevision, uint64_t transformRevision) const;

private:
	Transform transform;
	ProtectedState<CameraState> cameraState;
	mutable CameraMatrixCache matrixCache;
};

}
