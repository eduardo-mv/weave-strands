#include "Transform.h"

#include <utility>

#include "TransformUtils.h"

namespace weave {

Transform::Transform() = default;

Transform::Transform(Vector3 position) {
	transformState.Touch([&](auto& mutableState){
		mutableState = TransformState(position, Quaternion(), Vector3(1.0f, 1.0f, 1.0f));
	});
}

Transform::Transform(Vector3 position, Vector3 scaling, Quaternion orientation) {
	transformState.Touch([&](auto& mutableState){
		mutableState = TransformState(position, orientation, scaling);
	});
}

Transform::Transform(Matrix4x4 const &matrix) {
	*this = matrix;
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
	transformState.Touch([&](auto& mutableState){
		mutableState = matrix;
	});
	return *this;
}

Transform &Transform::operator=(TransformState const &state_) {
	SetState(state_);
	return *this;
}

void Transform::Identity() {
	transformState.Touch([&](auto& mutableState){
		mutableState.Identity();
	});
}

TransformState Transform::GetState() const {
	return transformState.Snapshot();
}

void Transform::SetState(TransformState const &state_) {
	transformState.Touch([&](auto& mutableState){
		mutableState = state_;
	});
}

void Transform::SetPosition(Vector3 const &position) {
	transformState.Touch([&](auto& mutableState){
		mutableState.SetPosition(position);
	});
}

Vector3 Transform::GetPosition() const {
	return transformState.Snapshot([](auto const& snapshot) {
		return snapshot.GetPosition();
	});
}

void Transform::Translate(Vector3 const &offset) {
	transformState.Touch([&](auto& mutableState){
		mutableState.Translate(offset);
	});
}

void Transform::Translate(Vector4 const &offset) {
	Translate(offset.xyz);
}

void Transform::Translate(float x, float y, float z) {
	Translate(Vector3(x, y, z));
}

void Transform::SetScaling(Vector3 const &scale) {
	transformState.Touch([&](auto& mutableState){
		mutableState.SetScaling(scale);
	});
}

Vector3 Transform::GetScaling() const {
	return transformState.Snapshot([](auto const& snapshot) {
		return snapshot.GetScaling();
	});
}

void Transform::Scale(Vector3 const &factor) {
	transformState.Touch([&](auto& mutableState){
		mutableState.Scale(factor);
	});
}

void Transform::Scale(float x, float y, float z) {
	Scale(Vector3(x, y, z));
}

void Transform::SetOrientation(Quaternion const &orientation) {
	transformState.Touch([&](auto& mutableState){
		mutableState.SetOrientation(orientation);
	});
}

Quaternion Transform::GetOrientation() const {
	return transformState.Snapshot([](auto const& snapshot) {
		return snapshot.GetOrientation();
	});
}

void Transform::RotateScaled(Vector3 const &scaledAxis) {
	transformState.Touch([&](auto& mutableState){
		mutableState.RotateScaled(scaledAxis);
	});
}

void Transform::Rotate(float radians, float x, float y, float z, bool normalize) {
	transformState.Touch([&](auto& mutableState){
		mutableState.Rotate(radians, x, y, z, normalize);
	});
}

void Transform::Rotate(float radians, Vector3 const &axis, bool normalize) {
	Rotate(radians, axis.x, axis.y, axis.z, normalize);
}

void Transform::Rotate(float pitch, float yaw, float roll) {
	transformState.Touch([&](auto& mutableState){
		mutableState.Rotate(pitch, yaw, roll);
	});
}

void Transform::Rotate(Vector3 const &pitchYawRoll) {
	Rotate(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
}

void Transform::RotatePitch(float pitch) {
	transformState.Touch([&](auto& mutableState){
		mutableState.RotatePitch(pitch);
	});
}

void Transform::RotateYaw(float yaw) {
	transformState.Touch([&](auto& mutableState){
		mutableState.RotateYaw(yaw);
	});
}

void Transform::RotateRoll(float roll) {
	transformState.Touch([&](auto& mutableState){
		mutableState.RotateRoll(roll);
	});
}

void Transform::AlignRight(Vector3 const &desiredRight) {
	transformState.Touch([&](auto& mutableState){
		mutableState.AlignRight(desiredRight);
	});
}

void Transform::AlignUp(Vector3 const &desiredUp) {
	transformState.Touch([&](auto& mutableState){
		mutableState.AlignUp(desiredUp);
	});
}

void Transform::AlignFront(Vector3 const &desiredFront) {
	transformState.Touch([&](auto& mutableState){
		mutableState.AlignFront(desiredFront);
	});
}

void Transform::LookAt(Vector3 const &point, Vector3 const &worldUp, bool frontIsZPositive) {
	transformState.Touch([&](auto& mutableState){
		mutableState.LookAt(point, worldUp, frontIsZPositive);
	});
}

void Transform::LookAlign(Vector3 const &point, bool frontIsZPositive) {
	transformState.Touch([&](auto& mutableState){
		mutableState.LookAlign(point, frontIsZPositive);
	});
}

Matrix4x4 Transform::GetTransformMatrix() const {
	UpdateMatrixCache();
	return matrixCache.GetCache([](MatrixCache &cacheTarget) {
		return cacheTarget.matrix;
	});
}

Matrix4x4 Transform::GetInverseTransformMatrix() const {
	UpdateMatrixCache();
	return matrixCache.GetCache([](MatrixCache &cacheTarget) {
		return cacheTarget.inverse;
	});

}

void Transform::UpdateMatrixCache() const {
	auto [snapshot, snapRevision] = transformState.SnapshotAndRevision();
	matrixCache.UpdateCache(snapRevision, [&](MatrixCache &cacheTarget, uint64_t& revision) {
		cacheTarget.matrix = transform_utils::ComposeTRS(snapshot.position, snapshot.orientation, snapshot.scaling);
		cacheTarget.inverse = cacheTarget.matrix.Inverted();
		revision = snapRevision;
	});
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

uint64_t Transform::GetRevision() const {
	return transformState.Revision();
}

}
