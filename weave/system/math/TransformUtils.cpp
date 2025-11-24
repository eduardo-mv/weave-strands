#include "TransformUtils.h"

#include <algorithm>
#include <cmath>

#include "Interpolation.h"
#include "Transform.h"
#include "VectorMath.h"

namespace weave::transform_utils {

namespace {

Vector3 AxisFromState(TransformState const &state, uint32_t axis) {
	static constexpr Vector3 bases[3] = {
		Vector3(1.0f, 0.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, 0.0f, 1.0f)
	};
	uint32_t index = std::min<uint32_t>(axis, 2u);
	return state.orientation * bases[index];
}

TransformState LerpState(TransformState const &a, TransformState const &b, float t) {
	TransformState result;
	result.position = interpolation::lerp(a.position, b.position, t);
	result.scaling = interpolation::lerp(a.scaling, b.scaling, t);
	result.orientation = interpolation::slerp(a.orientation, b.orientation, t);
	return result;
}

TransformState WeightState(TransformState const &a, TransformState const &b, float u, float v) {
	TransformState result;
	result.position = interpolation::weight(a.position, b.position, u, v);
	result.scaling = interpolation::weight(a.scaling, b.scaling, u, v);
	result.orientation = interpolation::weight(a.orientation, b.orientation, u, v);
	return result;
}

TransformState WeightState(TransformState const &a, TransformState const &b, TransformState const &c, float u, float v, float w) {
	TransformState result;
	result.position = interpolation::weight(a.position, b.position, c.position, u, v, w);
	result.scaling = interpolation::weight(a.scaling, b.scaling, c.scaling, u, v, w);
	result.orientation = interpolation::weight(a.orientation, b.orientation, c.orientation, u, v, w);
	return result;
}

}

Matrix4x4 ComposeTRS(Vector3 const &position, Quaternion const &orientation, Vector3 const &scale) {
	Quaternion normalized = weave::algebra::normalize(orientation);
	Matrix4x4 matrix(normalized, position);
	matrix.Scale(scale);
	return matrix;
}

Matrix4x4 ComposeInverseTRS(Vector3 const &position, Quaternion const &orientation, Vector3 const &scale) {
	Matrix4x4 matrix = ComposeTRS(position, orientation, scale);
	return weave::algebra::invert(matrix);
}

TransformState FromMatrix(Matrix4x4 const &matrix) {
	TransformState state;
	Matrix3x3 rotation = matrix;
	state.scaling = rotation.RemoveScalingFactor();
	state.orientation = rotation;
	state.position = matrix.W.xyz;
	return state;
}

Matrix4x4 ToMatrix(TransformState const &state) {
	return ComposeTRS(state.position, state.orientation, state.scaling);
}

Vector3 GetAxis(TransformState const &state, uint32_t axis) {
	return AxisFromState(state, axis);
}

Vector3 GetXAxis(TransformState const &state) {
	return GetAxis(state, 0);
}

Vector3 GetYAxis(TransformState const &state) {
	return GetAxis(state, 1);
}

Vector3 GetZAxis(TransformState const &state) {
	return GetAxis(state, 2);
}

Vector3 GetRightAxis(TransformState const &state) {
	return -GetXAxis(state);
}

Vector3 GetUpAxis(TransformState const &state) {
	return GetYAxis(state);
}

Vector3 GetFrontAxis(TransformState const &state) {
	return GetZAxis(state);
}

Plane AxisPlane(TransformState const &state, unsigned int axis) {
	Vector3 axisVector = AxisFromState(state, axis);
	return weave::algebra::makeplane(axisVector, state.position);
}

Vector3 ToGlobal(TransformState const &state, Vector3 const &localPoint) {
	Matrix4x4 matrix = ComposeTRS(state.position, state.orientation, state.scaling);
	return matrix * localPoint;
}

Vector3 ToLocal(TransformState const &state, Vector3 const &globalPoint) {
	Matrix4x4 matrix = ComposeInverseTRS(state.position, state.orientation, state.scaling);
	return matrix * globalPoint;
}

Vector3 ToGlobalDirection(TransformState const &state, Vector3 const &localDir) {
	Matrix4x4 matrix = ComposeTRS(Vector3(0.0f, 0.0f, 0.0f), state.orientation, state.scaling);
	return matrix * localDir;
}

Vector3 ToLocalDirection(TransformState const &state, Vector3 const &globalDir) {
	Matrix4x4 matrix = ComposeInverseTRS(Vector3(0.0f, 0.0f, 0.0f), state.orientation, state.scaling);
	return matrix * globalDir;
}

TransformState Interpolate(TransformState const &a, TransformState const &b, float t) {
	return LerpState(a, b, t);
}

TransformState Weight(TransformState const &a, TransformState const &b, float u, float v) {
	return WeightState(a, b, u, v);
}

TransformState Weight(TransformState const &a, TransformState const &b, TransformState const &c, float u, float v, float w) {
	return WeightState(a, b, c, u, v, w);
}

TransformState FromAxes(Vector3 const &xAxis, Vector3 const &yAxis, Vector3 const &zAxis, Vector3 const &position) {
	Matrix4x4 matrix(xAxis, yAxis, zAxis);
	matrix.W.xyz = position;
	return TransformState(matrix);
}

}
