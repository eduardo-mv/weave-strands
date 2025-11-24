#pragma once

#include "VectorMath.h"

namespace weave {
class Transform;
struct TransformState;
}

namespace weave::transform_utils {

Matrix4x4 ComposeTRS(Vector3 const &position, Quaternion const &orientation, Vector3 const &scale);
Matrix4x4 ComposeInverseTRS(Vector3 const &position, Quaternion const &orientation, Vector3 const &scale);
TransformState FromMatrix(Matrix4x4 const &matrix);
Matrix4x4 ToMatrix(TransformState const &state);

Vector3 GetAxis(TransformState const &state, uint32_t axis);
Vector3 GetXAxis(TransformState const &state);
Vector3 GetYAxis(TransformState const &state);
Vector3 GetZAxis(TransformState const &state);
Vector3 GetRightAxis(TransformState const &state);
Vector3 GetUpAxis(TransformState const &state);
Vector3 GetFrontAxis(TransformState const &state);

Plane AxisPlane(TransformState const &state, unsigned int axis);
Vector3 ToGlobal(TransformState const &state, Vector3 const &localPoint);
Vector3 ToLocal(TransformState const &state, Vector3 const &globalPoint);

TransformState Interpolate(TransformState const &a, TransformState const &b, float t);

TransformState Weight(TransformState const &a, TransformState const &b, float u, float v);
TransformState Weight(TransformState const &a, TransformState const &b, TransformState const &c, float u, float v, float w);

TransformState AlignRight(TransformState const &state, Vector3 const &desiredRight);
TransformState AlignUp(TransformState const &state, Vector3 const &desiredUp);
TransformState AlignFront(TransformState const &state, Vector3 const &desiredFront);
TransformState LookAt(TransformState const &state, Vector3 const &point, Vector3 const &worldUp = Vector3(0.0f, 1.0f, 0.0f), bool frontIsZPositive = true);
TransformState LookAlign(TransformState const &state, Vector3 const &point, bool frontIsZPositive = true);

TransformState FromAxes(Vector3 const &xAxis, Vector3 const &yAxis, Vector3 const &zAxis, Vector3 const &position = Vector3(0.0f, 0.0f, 0.0f));

}
