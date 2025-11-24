#pragma once

#include "VectorMath.h"

namespace weave {

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

}

