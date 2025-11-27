#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>

#include "weave/system/memory/BufferedCache.h"
#include "weave/system/memory/ProtectedState.h"
#include "TransformState.h"

namespace weave {

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
	Vector3 TransformDirection(Vector3 const &localDir) const;
	Vector3 InverseTransformDirection(Vector3 const &globalDir) const;
	uint64_t GetRevision() const;

private:
	struct MatrixCache {
		Matrix4x4 matrix;
		Matrix4x4 inverse;
	};

	void UpdateMatrixCache() const;

	ProtectedState<TransformState> transformState;
	BufferedCache<MatrixCache> matrixCache;
};

}
