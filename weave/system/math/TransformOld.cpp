#include "Transform.h"
using namespace weave;

Transform::Transform(Matrix4x4 const &trans) { 
	FromMatrix(trans);
}

Transform::Transform(Transform const &other)
	: position(other.position)
	, scaling(other.scaling)
	, orientation(other.orientation)
{
}

weave::Transform::Transform(Transform&& other)
	: position(other.position)
	, scaling(other.scaling)
	, orientation(other.orientation)
	, lastLocalChange(other.lastLocalChange)
	, lastLocalSync(other.lastLocalSync)
	, lastInverseSync(other.lastInverseSync)
{
	if (lastLocalChange == lastLocalSync) {
		cacheAxes = other.cacheAxes;
		matrix = other.matrix;
	}

	if (lastLocalChange == lastInverseSync) {
		inverseMatrix = other.inverseMatrix;
	}
}

void Transform::operator = (Transform const &other) {
	position = other.position;
	scaling = other.scaling;
	orientation = other.orientation;

	++lastLocalChange;
}

void Transform::operator = (TransformData const &other) {
	position = other.position;
	scaling = other.scaling;
	orientation = other.orientation;

	++lastLocalChange;
}

//Resets the transform to the default values
void Transform::Identity() {
	position.Set(0, 0, 0);
	scaling.Set(1.0f, 1.0f, 1.0f);
	orientation.Identity();

	++lastLocalChange;
}

//Aligns the transform with the provided right axis
void Transform::AlignRight(Vector3 const &rightIn) {
	using namespace weave::algebra;

	//First rotate the side vector so that it's aligned with the rightIn supplied
	vec3 const &side = GetAxis(0);
	float ang = anglenormal(side, rightIn);

	if(std::abs(algebra::F_PI - ang) < 0.001f) {
		orientation.Rotate(ang, GetAxis(1));
	}
	else if(ang > 0.00001f) {
		orientation.Rotate(ang, normalize(cross(side, rightIn)));
	}

	++lastLocalChange;
}

//Aligns the transform with the provided up axis
void Transform::AlignUp(Vector3 const &upIn) {
	using namespace weave::algebra;

	//First rotate the up vector so that it's aligned with the upIn supplied
	vec3 const &up = GetAxis(1);
	float ang = anglenormal(up, upIn);

	if(std::abs(algebra::F_PI - ang) < 0.001f) {
		orientation.Rotate(ang, GetAxis(0));
	}
	else if(ang > 0.00001f) {
		orientation.Rotate(ang, normalize(cross(up, upIn)));
	}

	++lastLocalChange;
}

//Aligns the transform with the provided front axis
void Transform::AlignFront(Vector3 const &frontIn) {
	using namespace weave::algebra;

	//First rotate the front vector so that it's aligned with the frontIn supplied
	vec3 const &front = GetAxis(2);
	float ang = anglenormal(front, frontIn);

	if(std::abs(algebra::F_PI - ang) < 0.001f) {
		orientation.Rotate(ang, GetAxis(1));
	}
	else if(ang > 0.00001f) {
		orientation.Rotate(ang, normalize(cross(front, frontIn)));
	}

	++lastLocalChange;
}

//Rotate the transform such that the front axis (Z) points to the specified point. The world up axis can be specified (defaults to Y = 1) and it's used to keep
//the transform in level with the "ground" concept. This is the usual desired effect of looking at an object. For an alternative that does not correct rotation use LookAlign
//The method assumes the provided position and vector are on the same space as the Transform
void weave::Transform::LookAt(Vector3 const & point, Vector3 const & worldUp, bool frontIsZPositive) {
	using namespace weave::algebra;
	//Find the new view vector based on the position of the target and the transform's position
	vec3 forward = (frontIsZPositive ? point - position : position - point);
	if(forward.x == 0.0f && forward.y == 0.0 && forward.z == 0.0f)
		return;
	forward = normalize(forward);
	float upDot = dot(worldUp, forward);
	vec3 up = (std::abs(upDot) >= 0.999f ? (frontIsZPositive ? Vector3(0, 0, 1) : Vector3(0, 0, -1)) : worldUp); //For when the point is right on top of the transform along Y
	//Create an orthonormal base from the supplied forward and worldUp vectors. All vectors will be normalized, up will be rotated so that it's orthogonal to forward and a the side vector will be returned.
	vec3 right = orthonormalize(forward, up);
	//Place the orthonormal base on a 3x3 matrix and transform it into an orientation quaternion
	orientation = mat3(right, up, forward);

	++lastLocalChange;
}

//Rotate the transform such that the front axis (Z) points to the specified point.
//No world Up axis is supplied, which can result in an alignment which is not in level with the "ground".
//The method assumes the provided position and vector are on the same space as the Transform
void weave::Transform::LookAlign(Vector3 const & point, bool frontIsZPositive) {
	using namespace weave::algebra;
	//Find the new view vector based on the position of the target and the transform's position
	vec3 forward = (frontIsZPositive ? point - position : position - point);
	if(forward.x == 0.0f && forward.y == 0.0 && forward.z == 0.0f)
		return;
	forward = normalize(forward);

	//Use the alignment functionality
	AlignFront(forward);
}

//Interpolates two Transforms and stores the result
void Transform::Interpolate(Transform const &t1, Transform const &t2, float u) {
	this->position = interpolation::lerp(t1.position,t2.position,u);
	this->scaling = interpolation::lerp(t1.scaling,t2.scaling,u);
	this->orientation = interpolation::slerp(t1.orientation,t2.orientation,u);
	//this->orientation = interpolation::lerp(t1.orientation,t2.orientation,u);

	++lastLocalChange;
}

void weave::Transform::Interpolate(TransformData const & t1, TransformData const & t2, float u) {
	this->position = interpolation::lerp(t1.position, t2.position, u);
	this->scaling = interpolation::lerp(t1.scaling, t2.scaling, u);
	this->orientation = interpolation::slerp(t1.orientation, t2.orientation, u);

	++lastLocalChange;
}

//Weights two Transforms and stores the result
void Transform::Weight(Transform const &t1, Transform const &t2, float u, float v) {
	this->position = interpolation::weight(t1.position,t2.position,u,v);
	this->scaling = interpolation::weight(t1.scaling,t2.scaling,u,v);
	this->orientation = interpolation::weight(t1.orientation,t2.orientation,u,v);

	++lastLocalChange;
}

//Weights three Transforms and stores the result
void Transform::Weight(Transform const &t1, Transform const &t2, Transform const &t3, float u, float v, float w) {
	this->position = interpolation::weight(t1.position,t2.position,t3.position,u,v,w);
	this->scaling = interpolation::weight(t1.scaling,t2.scaling,t3.scaling,u,v,w);
	this->orientation = interpolation::weight(t1.orientation,t2.orientation,t3.orientation,u,v,w);
	
	++lastLocalChange;
}

void Transform::CacheInverseTransformMatrix() const {
	if(IsDirty() || lastInverseSync != lastLocalSync) {
		ForceCacheInverseTransformMatrix();
	}
}

void Transform::CacheInverseTransformMatrixOrtho() const {
	if(IsDirty() || lastInverseSync != lastLocalSync) {
		ForceCacheInverseTransformMatrixOrtho();
	}
}

//Forces the cache of the transform matrix, wether it's necesary or not
void Transform::ForceCacheTransformMatrix() const {
	//Lock the cache
	weave::mrsw_writelock lock(spinlock);
	//std::lock_guard lock(cacheMutex);
	if (IsDirty()) {
		//Normalize the quaternion
		const_cast<Quaternion&>(orientation).Normalize();
		//Quaternion to matrix transformation
		Matrix4x4 tmpMat(orientation, position);
		//Extract the axis to keep a cache copy of them
		cacheAxes = tmpMat;
		//And apply the scaling
		tmpMat.Scale(scaling);
		//Copy to the local cache. This is done last to make sure matrix is only touched once without intermediate processing
		//It helps keeping data correctly set with multi threaded access
		matrix = tmpMat;
		//Synchronize the counters
		lastLocalSync = lastLocalChange;
	}
}

//Forces the cache of the inverse transform matrix, wether it's necesary or not
void Transform::ForceCacheInverseTransformMatrix() const {
	CacheTransformMatrix();
	inverseMatrix = weave::algebra::invert(matrix);

	lastInverseSync = lastLocalSync;
}

//Forces the cache of the inverse transform matrix, wether it's necesary or not
void Transform::ForceCacheInverseTransformMatrixOrtho() const {
	CacheTransformMatrix();
	inverseMatrix = weave::algebra::orthoinvert(matrix);

	lastInverseSync = lastLocalSync;
}

//Sets the transformation based on the supplied matrix
void Transform::FromMatrix(Matrix4x4 const &trans) {
	//Rotation matrix
	Matrix3x3 rot = trans;
	//Extract the scaling. The scaling factor must be removed prior to extracting the rotation for the quaternion
	this->scaling = rot.RemoveScalingFactor();
	//Matrix to quaternion with the already removed scaling
	this->orientation = rot;
	//Extract the position
	position = trans.W.xyz;
	//Increase the change counter
	++lastLocalChange;

	//Since we've been given the matrix, we can cache it safely and synch
	cacheAxes = rot;
	matrix = trans;

	lastLocalSync = lastLocalChange;

}

//Sets a transform based on 3 axes that define an axis base.
//The axes are assumed to be orthonormal
void weave::Transform::FromAxes(Vector3 const & x, Vector3 const & y, Vector3 const & z) {
	FromMatrix(Matrix4x4(x, y, z));
}
	
//The + operand concatenates two transformations by adding the position and rotation together and multiplying the scaling.
//This is not the same as the * operant which actualy accounts for the transformation sequence
Transform Transform::operator + (Transform const &tin) const {
	Transform out(this->position + tin.position, this->scaling * (tin.scaling), this->orientation + tin.orientation);

	//Make sure the output values are correct
	out.orientation.Normalize();
	//out.position.w = 1.0;

	return out;
}

//Sets the transformation based on the supplied matrix
void weave::TransformData::FromMatrix(Matrix4x4 const & trans) {
	//Rotation matrix
	Matrix3x3 rot = trans;
	//Extract the scaling. The scaling factor must be removed prior to extracting the rotation for the quaternion
	this->scaling = rot.RemoveScalingFactor();
	//Matrix to quaternion with the already removed scaling
	this->orientation = rot;
	//Extract the position
	position = trans.W.xyz;
}

//Forces the cache of the transform matrix, wether it's necesary or not
Matrix4x4 weave::TransformData::ToMatrix() const {
	Matrix4x4 matrix;
	//Normalize the quaternion
	const_cast<Quaternion&>(orientation).Normalize();
	//Quaternion to matrix transformation
	matrix.Set(orientation, position, scaling);

	return matrix;
}

void weave::TransformData::operator=(Transform const & trans) {
	position = trans.GetPosition();
	scaling = trans.GetScaling();
	orientation = trans.GetOrientation();
}

