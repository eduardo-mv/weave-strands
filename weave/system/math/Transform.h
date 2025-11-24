#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>

#include "weave/system/memory/BufferedCache.h"
#include "VectorMath.h"

namespace weave {

// Holds the raw TRS state for a transform and keeps a revision counter that
// changes whenever the state mutates. Copying the state copies the current
// revision value so that snapshots remain self-contained.
struct TransformState final {
	Vector3 position{ 0.0f, 0.0f, 0.0f };
	Vector3 scaling{ 1.0f, 1.0f, 1.0f };
	Quaternion orientation{};

	constexpr TransformState() = default;
	TransformState(Vector3 const &pos, Vector3 const &scale, Quaternion const &orient);
	explicit TransformState(Matrix4x4 const &matrix);
	TransformState(TransformState const &other) = default;
	TransformState &operator=(TransformState const &other) = default;
	TransformState &operator=(Matrix4x4 const &matrix);

	void Identity();

	void SetPosition(Vector3 const &value);
	constexpr Vector3 GetPosition() const noexcept { return position; }
	void Translate(Vector3 const &offset);
	void Translate(Vector4 const &offset);
	void Translate(float x, float y, float z);

	void SetScaling(Vector3 const &value);
	constexpr Vector3 GetScaling() const noexcept { return scaling; }
	void Scale(Vector3 const &factor);
	void Scale(float x, float y, float z);

	void SetOrientation(Quaternion const &value);
	constexpr Quaternion GetOrientation() const noexcept { return orientation; }
	void RotateScaled(Vector3 const &scaledAxis);
	void Rotate(float radians, float x, float y, float z, bool normalize = true);
	void Rotate(float radians, Vector3 const &axis, bool normalize = true);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(Vector3 const &pitchYawRoll);
	void RotatePitch(float pitch);
	void RotateYaw(float yaw);
	void RotateRoll(float roll);

	bool AlignRight(Vector3 const &desiredRight);
	bool AlignUp(Vector3 const &desiredUp);
	bool AlignFront(Vector3 const &desiredFront);
	bool LookAt(Vector3 const &point, Vector3 const &worldUp = Vector3(0.0f, 1.0f, 0.0f), bool frontIsZPositive = true);
	bool LookAlign(Vector3 const &point, bool frontIsZPositive = true);
};

class MatrixCache final {
public:

	Matrix4x4 GetMatrix(TransformState const &state, uint64_t revision) const;
	Matrix4x4 GetInverseMatrix(TransformState const &state, uint64_t revision) const;

private:
	void UpdateCache(TransformState const &state, uint64_t revision) const;

	struct Cache {
		Matrix4x4 matrix;
		Matrix4x4 inverse;
	};

	mutable BufferedCache<Cache> cache;
};

class TransformStateCache final {
public:
	TransformState GetState(TransformState const &state, uint64_t revision) const;
	Vector3 GetPosition(TransformState const &state, uint64_t revision) const;
	Vector3 GetScaling(TransformState const &state, uint64_t revision) const;
	Quaternion GetOrientation(TransformState const &state, uint64_t revision) const;

private:
	void UpdateCache(TransformState const &state, uint64_t revision) const;
	mutable BufferedCache<TransformState> cache;
};

// Public facing transform object composed from a state + cache pair.
class Transform final {
public:
	Transform();
	explicit Transform(Vector3 position);
	Transform(Vector3 position, Vector3 scaling, Quaternion orientation);
	explicit Transform(Matrix4x4 const &matrix);
	Transform(Transform const &other);
	Transform &operator=(Transform const &other);
	Transform(Transform &&other) noexcept;
	Transform &operator=(Transform &&other) noexcept;
	Transform &operator=(Matrix4x4 const &matrix);
	Transform &operator=(TransformState const &state);

	void Identity();

	TransformState GetState() const;
	void SetState(TransformState const &state);

	void SetPosition(Vector3 const &position);
	Vector3 GetPosition() const;
	void Translate(Vector3 const &offset);
	void Translate(Vector4 const &offset);
	void Translate(float x, float y, float z);

	void SetScaling(Vector3 const &scale);
	Vector3 GetScaling() const;
	void Scale(Vector3 const &factor);
	void Scale(float x, float y, float z);

	void SetOrientation(Quaternion const &orientation);
	Quaternion GetOrientation() const;
	void RotateScaled(Vector3 const &scaledAxis);
	void Rotate(float radians, float x, float y, float z, bool normalize = true);
	void Rotate(float radians, Vector3 const &axis, bool normalize = true);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(Vector3 const &pitchYawRoll);
	void RotatePitch(float pitch);
	void RotateYaw(float yaw);
	void RotateRoll(float roll);
	void AlignRight(Vector3 const &desiredRight);
	void AlignUp(Vector3 const &desiredUp);
	void AlignFront(Vector3 const &desiredFront);
	void LookAt(Vector3 const &point, Vector3 const &worldUp = Vector3(0.0f, 1.0f, 0.0f), bool frontIsZPositive = true);
	void LookAlign(Vector3 const &point, bool frontIsZPositive = true);

	Matrix4x4 GetTransformMatrix() const;
	Matrix4x4 GetInverseTransformMatrix() const;
	Matrix4x4 operator*(Transform const &other) const;
	Vector3 operator*(Vector3 const &vector) const;
	Vector4 operator*(Vector4 const &vector) const;
	Transform operator+(Transform const &other) const;
	operator TransformState() const { return GetState(); }

private:
	void MarkStateDirty();

	TransformState state;
	mutable TransformStateCache trsCache;
	mutable MatrixCache matrixCache;
	std::atomic_uint64_t stateRevision { 1 };
	std::mutex stateMutex;
};

}
