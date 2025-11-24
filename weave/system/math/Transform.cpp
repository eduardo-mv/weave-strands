#include "Transform.h"

#include <algorithm>
#include <utility>

#include "TransformUtils.h"
#include "VectorMath.h"

namespace weave {

namespace {

Vector3 AxisFromOrientation(Quaternion const &orientation, uint32_t axis) {
	static constexpr Vector3 bases[3] = {
		Vector3(1.0f, 0.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, 0.0f, 1.0f)
	};
	uint32_t index = std::min<uint32_t>(axis, 2u);
	return orientation * bases[index];
}

bool NormalizeVector(Vector3 &vector) {
	return weave::algebra::normalizecond(vector);
}

bool ApplyRotation(Quaternion &orientation, float radians, Vector3 axis) {
	if (radians <= 0.0f) {
		return false;
	}

	if (!NormalizeVector(axis)) {
		return false;
	}

	orientation.Rotate(radians, axis);
	orientation.Normalize();
	return true;
}

bool AlignAxis(Quaternion &orientation, Vector3 current, Vector3 desired, Vector3 fallbackAxis) {
	using namespace weave::algebra;
	if (!NormalizeVector(current) || !NormalizeVector(desired)) {
		return false;
	}

	float dotValue = clamp(dot(current, desired), -1.0f, 1.0f);
	float angle = std::acos(dotValue);
	bool rotated = false;

	if (std::abs(F_PI - angle) < 0.001f) {
		if (!NormalizeVector(fallbackAxis)) {
			fallbackAxis = Vector3(0.0f, 1.0f, 0.0f);
		}
		rotated = ApplyRotation(orientation, angle, fallbackAxis);
	} else if (angle > 0.00001f) {
		Vector3 rotationAxis = cross(current, desired);
		rotated = ApplyRotation(orientation, angle, rotationAxis);
	}

	return rotated;
}

bool BuildLookOrientation(Vector3 forward, Vector3 upInput, bool frontIsZPositive, Quaternion &outOrientation) {
	using namespace weave::algebra;

	if (!NormalizeVector(forward)) {
		return false;
	}

	Vector3 up = upInput;
	if (!NormalizeVector(up) || std::abs(dot(up, forward)) >= 0.999f) {
		up = frontIsZPositive ? Vector3(0.0f, 0.0f, 1.0f) : Vector3(0.0f, 0.0f, -1.0f);
	}

	Vector3 right = orthonormalize(forward, up);
	up = normalize(cross(right, forward));

	Matrix3x3 basis(right, up, forward);
	outOrientation = basis;
	outOrientation.Normalize();
	return true;
}

}

TransformState::TransformState(Vector3 const &pos, Vector3 const &scale, Quaternion const &orient)
	: position(pos)
	, scaling(scale)
	, orientation(orient) {
	orientation.Normalize();
}

TransformState::TransformState(Matrix4x4 const &matrix) {
	*this = matrix;
}

TransformState &TransformState::operator=(Matrix4x4 const &matrix) {
	*this = transform_utils::FromMatrix(matrix);
	return *this;
}

void TransformState::Identity() {
	position = Vector3(0.0f, 0.0f, 0.0f);
	scaling = Vector3(1.0f, 1.0f, 1.0f);
	orientation.Identity();
}

void TransformState::SetPosition(Vector3 const &value) {
	position = value;
}

void TransformState::Translate(Vector3 const &offset) {
	position += offset;
}

void TransformState::Translate(Vector4 const &offset) {
	Translate(offset.xyz);
}

void TransformState::Translate(float x, float y, float z) {
	Translate(Vector3(x, y, z));
}

void TransformState::SetScaling(Vector3 const &value) {
	scaling = value;
}

void TransformState::Scale(Vector3 const &factor) {
	scaling *= factor;
}

void TransformState::Scale(float x, float y, float z) {
	Scale(Vector3(x, y, z));
}

void TransformState::SetOrientation(Quaternion const &value) {
	orientation = value;
	orientation.Normalize();
}

void TransformState::RotateScaled(Vector3 const &scaledAxis) {
	orientation.RotateScaled(scaledAxis);
	orientation.Normalize();
}

void TransformState::Rotate(float radians, float x, float y, float z, bool normalize) {
	orientation.Rotate(radians, x, y, z, normalize);
	orientation.Normalize();
}

void TransformState::Rotate(float radians, Vector3 const &axis, bool normalize) {
	Rotate(radians, axis.x, axis.y, axis.z, normalize);
}

void TransformState::Rotate(float pitch, float yaw, float roll) {
	orientation.Rotate(pitch, yaw, roll);
	orientation.Normalize();
}

void TransformState::Rotate(Vector3 const &pitchYawRoll) {
	Rotate(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
}

void TransformState::RotatePitch(float pitch) {
	orientation.RotatePitch(pitch);
	orientation.Normalize();
}

void TransformState::RotateYaw(float yaw) {
	orientation.RotateYaw(yaw);
	orientation.Normalize();
}

void TransformState::RotateRoll(float roll) {
	orientation.RotateRoll(roll);
	orientation.Normalize();
}

bool TransformState::AlignRight(Vector3 const &desiredRight) {
	Quaternion newOrientation = orientation;
	Vector3 currentRight = -AxisFromOrientation(newOrientation, 0);
	Vector3 fallbackAxis = AxisFromOrientation(newOrientation, 1);
	if (!AlignAxis(newOrientation, currentRight, desiredRight, fallbackAxis)) {
		return false;
	}

	orientation = newOrientation;
	return true;
}

bool TransformState::AlignUp(Vector3 const &desiredUp) {
	Quaternion newOrientation = orientation;
	Vector3 currentUp = AxisFromOrientation(newOrientation, 1);
	Vector3 fallbackAxis = AxisFromOrientation(newOrientation, 0);
	if (!AlignAxis(newOrientation, currentUp, desiredUp, fallbackAxis)) {
		return false;
	}

	orientation = newOrientation;
	return true;
}

bool TransformState::AlignFront(Vector3 const &desiredFront) {
	Quaternion newOrientation = orientation;
	Vector3 currentFront = AxisFromOrientation(newOrientation, 2);
	Vector3 fallbackAxis = AxisFromOrientation(newOrientation, 1);
	if (!AlignAxis(newOrientation, currentFront, desiredFront, fallbackAxis)) {
		return false;
	}

	orientation = newOrientation;
	return true;
}

bool TransformState::LookAt(Vector3 const &point, Vector3 const &worldUp, bool frontIsZPositive) {
	Vector3 forward = frontIsZPositive ? point - position : position - point;
	Quaternion newOrientation = orientation;
	if (!BuildLookOrientation(forward, worldUp, frontIsZPositive, newOrientation)) {
		return false;
	}

	orientation = newOrientation;
	return true;
}

bool TransformState::LookAlign(Vector3 const &point, bool frontIsZPositive) {
	Vector3 forward = frontIsZPositive ? point - position : position - point;
	Quaternion newOrientation = orientation;
	Vector3 currentFront = AxisFromOrientation(newOrientation, 2);
	Vector3 fallbackAxis = AxisFromOrientation(newOrientation, 1);
	if (!AlignAxis(newOrientation, currentFront, forward, fallbackAxis)) {
		return false;
	}

	orientation = newOrientation;
	return true;
}

Matrix4x4 MatrixCache::GetMatrix(TransformState const &state, uint64_t revision) const {
	UpdateCache(state, revision);	
	return cache.GetCache([](Cache& cacheTarget) {
		return cacheTarget.matrix;
	});
}

Matrix4x4 MatrixCache::GetInverseMatrix(TransformState const &state, uint64_t revision) const {
	UpdateCache(state, revision);	
	return cache.GetCache([](Cache& cacheTarget) {
		return cacheTarget.inverse;
	});
}

void MatrixCache::UpdateCache(TransformState const &state, uint64_t revision) const {
	cache.UpdateCache(revision, [&](Cache& cacheTarget){
		cacheTarget.matrix = transform_utils::ComposeTRS(state.position, state.orientation, state.scaling);
		cacheTarget.inverse = cacheTarget.matrix.Inverted();
	});
}

TransformState TransformStateCache::GetState(TransformState const &state, uint64_t revision) const {
	UpdateCache(state, revision);
	return cache.GetCache([](TransformState const &cacheTarget) {
		return cacheTarget;
	});
}

Vector3 TransformStateCache::GetPosition(TransformState const &state, uint64_t revision) const {
	UpdateCache(state, revision);
	return cache.GetCache([](TransformState const &cacheTarget) {
		return cacheTarget.position;
	});
}

Vector3 TransformStateCache::GetScaling(TransformState const &state, uint64_t revision) const {
	UpdateCache(state, revision);
	return cache.GetCache([](TransformState const &cacheTarget) {
		return cacheTarget.scaling;
	});
}

Quaternion TransformStateCache::GetOrientation(TransformState const &state, uint64_t revision) const {
	UpdateCache(state, revision);
	return cache.GetCache([](TransformState const &cacheTarget) {
		return cacheTarget.orientation;
	});
}

void TransformStateCache::UpdateCache(TransformState const &state, uint64_t revision) const {
	cache.UpdateCache(revision, [&](TransformState& cacheTarget) {
		cacheTarget = state;
	});
}


Transform::Transform() = default;

Transform::Transform(Transform const &other)
	: Transform() {
	*this = other;
}

Transform::Transform(Transform &&other) noexcept
	: Transform() {
	*this = std::move(other);
}


Transform::Transform(Vector3 position) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state = TransformState(position, Vector3(1.0f, 1.0f, 1.0f), Quaternion());
}

void Transform::Identity() {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.Identity();
	MarkStateDirty();
}

Transform::Transform(Vector3 position, Vector3 scaling, Quaternion orientation)
	: state(position, scaling, orientation) {
	std::lock_guard<std::mutex> lock(stateMutex);
}

Transform::Transform(Matrix4x4 const &matrix) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state = matrix;
	MarkStateDirty();
}
TransformState Transform::GetState() const {
	uint64_t revision = stateRevision.load(std::memory_order_acquire);
	return trsCache.GetState(state, revision);
}

Transform &Transform::operator=(Transform const &other) {
	if (this == &other) {
		return *this;
	}

	SetState(other.GetState());

	return *this;
}

Transform &Transform::operator=(Transform &&other) noexcept {
	if (this == &other) {
		return *this;
	}

	SetState(other.GetState());

	other.Identity();
	return *this;
}

Transform &Transform::operator=(Matrix4x4 const &matrix) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state = matrix;
	MarkStateDirty();
	return *this;
}

Transform &Transform::operator=(TransformState const &state_) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state = state_;
	MarkStateDirty();
	return *this;
}

void Transform::SetState(TransformState const &state_) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state = state_;
	MarkStateDirty();
}

void Transform::SetPosition(Vector3 const &position) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.SetPosition(position);
	MarkStateDirty();
}

Vector3 Transform::GetPosition() const {
	uint64_t revision = stateRevision.load(std::memory_order_acquire);
	return trsCache.GetPosition(state, revision);
}

void Transform::Translate(Vector3 const &offset) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.Translate(offset);
	MarkStateDirty();
}

void Transform::Translate(Vector4 const &offset) {
	Translate(offset.xyz);
}

void Transform::Translate(float x, float y, float z) {
	Translate(Vector3(x, y, z));
}

void Transform::SetScaling(Vector3 const &scale) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.SetScaling(scale);
	MarkStateDirty();
}

Vector3 Transform::GetScaling() const {
	uint64_t revision = stateRevision.load(std::memory_order_acquire);
	return trsCache.GetScaling(state, revision);
}

void Transform::Scale(Vector3 const &factor) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.Scale(factor);
	MarkStateDirty();
}

void Transform::Scale(float x, float y, float z) {
	Scale(Vector3(x, y, z));
}

void Transform::SetOrientation(Quaternion const &orientation) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.SetOrientation(orientation);
	MarkStateDirty();
}

Quaternion Transform::GetOrientation() const {
	uint64_t revision = stateRevision.load(std::memory_order_acquire);
	return trsCache.GetOrientation(state, revision);
}

void Transform::RotateScaled(Vector3 const &scaledAxis) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.RotateScaled(scaledAxis);
	MarkStateDirty();
}

void Transform::Rotate(float radians, float x, float y, float z, bool normalize) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.Rotate(radians, x, y, z, normalize);
	MarkStateDirty();
}

void Transform::Rotate(float radians, Vector3 const &axis, bool normalize) {
	Rotate(radians, axis.x, axis.y, axis.z, normalize);
}

void Transform::Rotate(float pitch, float yaw, float roll) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.Rotate(pitch, yaw, roll);
	MarkStateDirty();
}

void Transform::Rotate(Vector3 const &pitchYawRoll) {
	Rotate(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
}

void Transform::RotatePitch(float pitch) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.RotatePitch(pitch);
	MarkStateDirty();
}

void Transform::RotateYaw(float yaw) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.RotateYaw(yaw);
	MarkStateDirty();
}

void Transform::RotateRoll(float roll) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.RotateRoll(roll);
	MarkStateDirty();
}

void Transform::AlignRight(Vector3 const &desiredRight) {
	std::lock_guard<std::mutex> lock(stateMutex);
	if (state.AlignRight(desiredRight)) {
		MarkStateDirty();
	}
}

void Transform::AlignUp(Vector3 const &desiredUp) {
	std::lock_guard<std::mutex> lock(stateMutex);
	if (state.AlignUp(desiredUp)) {
		MarkStateDirty();
	}
}

void Transform::AlignFront(Vector3 const &desiredFront) {
	std::lock_guard<std::mutex> lock(stateMutex);
	if (state.AlignFront(desiredFront)) {
		MarkStateDirty();
	}
}

void Transform::LookAt(Vector3 const &point, Vector3 const &worldUp, bool frontIsZPositive) {
	std::lock_guard<std::mutex> lock(stateMutex);
	if (state.LookAt(point, worldUp, frontIsZPositive)) {
		MarkStateDirty();
	}
}

void Transform::LookAlign(Vector3 const &point, bool frontIsZPositive) {
	std::lock_guard<std::mutex> lock(stateMutex);
	if (state.LookAlign(point, frontIsZPositive)) {
		MarkStateDirty();
	}
}

Matrix4x4 Transform::GetTransformMatrix() const {
	uint64_t revision = stateRevision.load(std::memory_order_acquire);
	return matrixCache.GetMatrix(state, revision);
}

Matrix4x4 Transform::GetInverseTransformMatrix() const {
	uint64_t revision = stateRevision.load(std::memory_order_acquire);
	return matrixCache.GetInverseMatrix(state, revision);
}

Matrix4x4 Transform::operator*(Transform const &other) const {
	return GetTransformMatrix() * other.GetTransformMatrix();
}

Vector3 Transform::operator*(Vector3 const &vector) const {
	return GetTransformMatrix() * vector;
}

Vector4 Transform::operator*(Vector4 const &vector) const {
	return GetTransformMatrix() * vector;
}

Transform Transform::operator+(Transform const &other) const {
	TransformState lhs = GetState();
	TransformState rhs = other.GetState();

	Quaternion orientation = lhs.orientation + rhs.orientation;
	orientation.Normalize();

	return Transform(lhs.position + rhs.position, lhs.scaling * rhs.scaling, orientation);
}

void Transform::MarkStateDirty() {
	stateRevision.fetch_add(1, std::memory_order_release);
}

}
