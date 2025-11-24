#include "Transform.h"

#include <utility>

#include "TransformUtils.h"

namespace weave {

Matrix4x4 MatrixCache::GetMatrix(TransformState const &state, uint64_t revision) const {
	UpdateCache(state, revision);
	return cache.GetCache([](Cache &cacheTarget) {
		return cacheTarget.matrix;
	});
}

Matrix4x4 MatrixCache::GetInverseMatrix(TransformState const &state, uint64_t revision) const {
	UpdateCache(state, revision);
	return cache.GetCache([](Cache &cacheTarget) {
		return cacheTarget.inverse;
	});
}

void MatrixCache::UpdateCache(TransformState const &state, uint64_t revision) const {
	cache.UpdateCache(revision, [&](Cache &cacheTarget) {
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
	cache.UpdateCache(revision, [&](TransformState &cacheTarget) {
		cacheTarget = state;
	});
}

Transform::Transform() = default;

Transform::Transform(Vector3 position) {
	std::lock_guard<std::mutex> lock(stateMutex);
	state = TransformState(position, Vector3(1.0f, 1.0f, 1.0f), Quaternion());
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

Transform::Transform(Transform const &other)
	: Transform() {
	*this = other;
}

Transform::Transform(Transform &&other) noexcept
	: Transform() {
	*this = std::move(other);
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

void Transform::Identity() {
	std::lock_guard<std::mutex> lock(stateMutex);
	state.Identity();
	MarkStateDirty();
}

TransformState Transform::GetState() const {
	uint64_t revision = stateRevision.load(std::memory_order_acquire);
	return trsCache.GetState(state, revision);
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

Vector3 Transform::TransformDirection(Vector3 const &localDir) const {
	TransformState snapshot = GetState();
	return transform_utils::ToGlobalDirection(snapshot, localDir);
}

Vector3 Transform::InverseTransformDirection(Vector3 const &globalDir) const {
	TransformState snapshot = GetState();
	return transform_utils::ToLocalDirection(snapshot, globalDir);
}

void Transform::MarkStateDirty() {
	stateRevision.fetch_add(1, std::memory_order_release);
}

}
