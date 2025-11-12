/*
Title: "Transform"
File: Transform.h
Author(s): Eduardo Martínez Vidal

Abstract:
	
Update Log:
	06 Jan 2017:
	Weave creation
	20 July 2012:
	File creation

*/
#pragma once

#include "VectorMath.h"
#include "Interpolation.h"
#include "weave/system/parallel/Spinlock.h"
#include <mutex>

namespace weave {

namespace component {
	class Hierarchy;
}

//Defines the raw data used by a Transform and a few conversion methods between them. 
//This structure removes any internal cache and sync features and is meant to be used as a raw data storage that can be converted easily into the more flexible Transform
struct TransformData {
	weave::Vector3 position;
	weave::Vector3 scaling{1.0f, 1.0f, 1.0f};
	weave::Quaternion orientation;

	TransformData() {}
	TransformData(Matrix4x4 const &trans) { *this = trans; }
	TransformData(Transform const &trans) { *this = trans; }
	//Sets the transformation based on the supplied matrix
	void FromMatrix(Matrix4x4 const &trans);
	//Converts the transform data to a matrix
	Matrix4x4 ToMatrix() const;

	void operator = (Matrix4x4 const &trans) { this->FromMatrix(trans); }
	void operator = (Transform const &trans);
};

//Defines a transformation for a primitive and provides several servicing methods, including conversions from and to Matrix4x4
class Transform {
	friend class weave::component::Hierarchy;
protected:
	weave::Vector3 position;
	weave::Vector3 scaling{ 1.0f, 1.0f, 1.0f };
	weave::Quaternion orientation;
	
	//Cache
	mutable weave::Matrix3x3 cacheAxes; //Holds the three axes transformed. They are defined on the cache matrix but provide a faster access since they aren't affected by scaling
	mutable weave::Matrix4x4 matrix; //Holds a cached transformation matrix representation of the transform
	mutable weave::Matrix4x4 inverseMatrix;

	//Synchronization counters
	mutable uint64_t lastLocalChange = 1; //Incremented every time the transform is accessed via write access
	mutable uint64_t lastLocalSync = 0; //Indicates local matrix cache needs synchronizing if != lastLocalChange
	mutable uint64_t lastInverseSync = 0; //Indicates the inverse matrix cache is not updated if != lastLocalSync
	
	//mutable std::mutex cacheMutex;
	mutable weave::mrsw_spinlock spinlock; //Unsed to ensure the cache only generates once when shared through threads


public:
	Transform() = default;
	Transform(Transform const &other);
	Transform(Transform&& other);
	Transform(Vector3 const &pos, Vector3 const &scale, Quaternion const &orient) : position(pos), scaling(scale), orientation(orient) {}
	Transform(Matrix4x4 const &trans);
	~Transform() = default;

	//Sync queries. These can be used to synchronize external objects with transform changes
	//Returns the last change value for the transform's internals
	inline uint64_t LastLocalChange() const { return lastLocalChange; }
	//Returns the last sync value for the local cache
	inline uint64_t LastLocalSync() const { return lastInverseSync; }
	//Returns the last sync value for the local inverse matrix cache
	inline uint64_t LastInverseSync() const { return lastInverseSync; }

	//Transformation functions
	//Resets the transform to the default values
	void Identity();
	
	//Translates the transform by the given values. Previous position is taken as the starting point of the new position.
	void Translate(Vector3 const &v) { position += v; ++lastLocalChange; }
	void Translate(Vector4 const &v) { position += v.xyz; ++lastLocalChange; }
	void Translate(float x, float y, float z) { position += Vector3(x, y, z); ++lastLocalChange; }

	//Rotates the transform by the given values. Previous orientation is taken as the starting point of the new rotation.
	void RotateScaled(Vector3 const &scaledVector) { orientation.RotateScaled(scaledVector); ++lastLocalChange; }
	void Rotate(float radians, float x, float y, float z, bool normalize = true) { orientation.Rotate(radians, x, y, z, normalize); ++lastLocalChange; }
	void Rotate(float radians, weave::Vector3 const &axis, bool normalize = true) { orientation.Rotate(radians, axis.x, axis.y, axis.z, normalize); ++lastLocalChange; }
	//Rotates the transform by the given euler angles
	void Rotate(float pitch, float yaw, float roll) { orientation.Rotate(pitch, yaw, roll); ++lastLocalChange; }
	void Rotate(weave::Vector3 const &pitchYawRoll) { orientation.Rotate(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); ++lastLocalChange; }
	//Scales the transform by the given values. Previous scaling is taken as the starting point of the new scaling.
	void Scale(Vector3 const &v) { scaling *= v; ++lastLocalChange; }
	void Scale(float x, float y, float z) { scaling *= Vector3(x, y, z); ++lastLocalChange; }

	//Rotates applying only a pitch rotation
	void RotatePitch(float pitch) { orientation.RotatePitch(pitch); ++lastLocalChange; }
	//Rotates applying only a yaw rotation
	void RotateYaw(float yaw) { orientation.RotateYaw(yaw); ++lastLocalChange; }
	//Rotates applying only a roll rotation
	void RotateRoll(float roll) { orientation.RotateRoll(roll); ++lastLocalChange; }

	//Sets the current position for the transform.
	void Position(Vector3 const &v) { position = v; ++lastLocalChange; }
	void Position(Vector4 const &v) { position = v.xyz; ++lastLocalChange; }
	void Position(float x, float y, float z) { position.Set(x,y,z); ++lastLocalChange; }
	Vector3& Position() { ++lastLocalChange; return position; }

	//Sets the current orientation for the transform.
	void Orientation(float radians, float x, float y, float z, bool normalize = true) { orientation.Orientation(radians,x,y,z,normalize); ++lastLocalChange; }
	void Orientation(float radians, Vector3 const &axis, bool normalize = true) { orientation.Orientation(radians,axis.x,axis.y,axis.z,normalize); ++lastLocalChange; }
	Quaternion& Orientation() { ++lastLocalChange; return orientation; }

	//Sets the orientation given euler angles in radians
	void Orientation(float pitch, float yaw, float roll) { orientation.Orientation(pitch,yaw,roll); ++lastLocalChange; }
	void Orientation(weave::Vector3 const &pitchYawRoll) { orientation.Orientation(pitchYawRoll.x,pitchYawRoll.y,pitchYawRoll.z); ++lastLocalChange; }

	//Sets the current scaling for the transform.
	void Scaling(Vector3 const &v) { scaling = v; ++lastLocalChange; }
	void Scaling(float x, float y, float z) { scaling.Set(x,y,z); ++lastLocalChange;}
	Vector3& Scaling() { ++lastLocalChange; return scaling; }

	//Aligns the transform with the provided right axis
	void AlignRight(Vector3 const &vec);
	//Aligns the transform with the provided up axis
	void AlignUp(Vector3 const &vec);
	//Aligns the transform with the provided front axis
	void AlignFront(Vector3 const &vec);

	//Rotate the transform such that the front axis (Z) points to the specified point. The world up axis can be specified (defaults to Y = 1) and it's used to keep
	//the transform in level with the "ground" concept. This is the usual desired effect of looking at an object. For an alternative that does not correct rotation use LookAlign
	//The method assumes the provided position and vector are on the same space as the Transform
	//@frontIsZPositive: when set to true, the positive direction of the Z vector is taken as the front vector. This is usually what is wanted for objects, but leaves the X vector facing negative (such that Right = -X). Set to false when using this to handle a transform that represents a Camera.
	void LookAt(Vector3 const &point, Vector3 const &worldUp = Vector3(.0f, 1.0f, .0f), bool frontIsZPositive = true);
	void LookAt(float x, float y, float z, float upx = 0.0f, float upy = 1.0f, float upz = 0.0f, bool frontIsZPositive = true) { LookAt(Vector3(x, y, z), Vector3(upx, upy, upz), frontIsZPositive); }

	//Rotate the transform such that the front axis (Z) points to the specified point.
	//No world Up axis is supplied, which can result in an alignment which is not in level with the "ground".
	//The method assumes the provided position and vector are on the same space as the Transform
	void LookAlign(Vector3 const &point, bool frontIsZPositive = true);
	void LookAlign(float x, float y, float z, bool frontIsZPositive = true) { LookAlign(Vector3(x, y, z), frontIsZPositive); }

	//Interpolates two Transforms and stores the result
	void Interpolate(Transform const &t1, Transform const &t2, float u);
	void Interpolate(TransformData const &t1, TransformData const &t2, float u);
	//Weights two Transforms and stores the result
	void Weight(Transform const &t1, Transform const &t2, float u, float v);
	//Weights three Transforms and stores the result
	void Weight(Transform const &t1, Transform const &t2, Transform const &t3, float u, float v, float w);

	//Returns the position/orientation/scaling of the transform in its native format.
	Vector3 const &  GetPosition() const { return position; }
	Vector3 const & GetScaling() const { return scaling; }
	Quaternion const & GetOrientation() const { return orientation; }

	//Returns the "Right" axis. This is equivalent to -X when +Z is considered the "Front" axis.
	Vector3 GetRightAxis() const { return -GetAxis(0); }
	//Returns the "Up" axis (+Y)
	Vector3 const & GetUpAxis() const { return GetAxis(1); }
	//Returns the "Front" axis (+Z)
	Vector3 const & GetFrontAxis() const { return GetAxis(2); }

	//Returns the X axis
	Vector3 const & GetXAxis() const { return GetAxis(0); }
	//Returns the Y axis
	Vector3 const & GetYAxis() const { return GetAxis(1); }
	//Returns the Z axis
	Vector3 const & GetZAxis() const { return GetAxis(2); }

	//Returns the transformed world axis x = 0, y = 1, z >= 2
	template<typename Index>
	Vector3 const & GetAxis(Index axis) const { 
		CacheTransformMatrix(); 
		return cacheAxes.columns[std::min<Index>(Index(2), axis)];
	}

	//Returns a plane on the direction of the specified axis x = 0, y = 1, z >= 2
	Plane AxisPlane(unsigned int axis) const {
		return algebra::makeplane(GetAxis(axis),position);
	}

	//Returns the transformation matrix. The matrix is cached internally and is only generated if the transform is manipulated, so
	//this function can be called safely without incurring in overhead. The matrix is returned as a reference to evade copy.
	Matrix4x4 const& GetTransformMatrix() const { CacheTransformMatrix(); return matrix; } 
	//Returns the inverted transformation matrix. The matrix is cached internally in the same way the TransformMatrix is handled, so
	//no overhead is added to repeated calls.
	Matrix4x4 const& GetInverseTransformMatrix() const { CacheInverseTransformMatrix(); return inverseMatrix; }
	//Returns the inverted transformation matrix. The matrix is cached internally in the same way the TransformMatrix is handled, so
	//no overhead is added to repeated calls. This version performs a faster inversion assuming the matrix is orthogonal
	Matrix4x4 const& GetInverseTransformMatrixOrtho() const { CacheInverseTransformMatrixOrtho(); return inverseMatrix; }
	//Caches the transform matrix if necesary
	void CacheTransformMatrix() const { if(IsDirty()) ForceCacheTransformMatrix(); }
	//Caches the inversed transform matrix if necesary
	void CacheInverseTransformMatrix() const;
	//Caches the inversed transform matrix if necesary. This version performs a faster orthogonal inversion
	void CacheInverseTransformMatrixOrtho() const;
	//Forces the cache of the transform matrix, wether it's necesary or not
	void ForceCacheTransformMatrix() const;
	//Forces the cache of the inversed transform matrix, wether it's necesary or not
	void ForceCacheInverseTransformMatrix() const;
	//Forces the cache of the inversed transform matrix, wether it's necesary or not. This version performs a faster orthogonal inversion
	void ForceCacheInverseTransformMatrixOrtho() const;

	//Sets the transformation based on the supplied matrix
	void FromMatrix(Matrix4x4 const &trans);
	//Sets a transform based on 3 axes that define an axis base.
	//The axes are assumed to be orthonormal
	void FromAxes(Vector3 const &x, Vector3 const &y, Vector3 const &z);

	//Transforms a given point in local coordinates to global coordinates (applies the transform to a point)
	Vector3 ToGlobal(Vector3 const &localPoint) const { return GetTransformMatrix() * localPoint; }
	//Transforms a given point in global coordinates to local by applying the inverse transform
	Vector3 ToLocal(Vector3 const &globalPoint) const { return GetInverseTransformMatrix() * globalPoint;	}

	//Operator = copies all members
	void operator = (Transform const &tin);
	void operator = (TransformData const &tin);
	void operator = (Matrix4x4 const &trans) { this->FromMatrix(trans); }
	//The * operand can be used to concatenate two transformations, in a similar was as it can be used with matrices. This is the
	//equivalent of multiplying the resulting matrix transforms. It's not the same as the + operator.
	Matrix4x4 operator * (Transform const &tin) const { return this->GetTransformMatrix() * tin.GetTransformMatrix(); }
	
	//The + operand concatenates two transformations by adding the position and rotation together and multiplying the scaling.
	//This is not the same as the * operant which actualy accounts for the transformation sequence
	Transform operator + (Transform const &tin) const;

	Vector3 operator * (Vector3 const &vIn) const { return GetTransformMatrix() * vIn; }

	Vector4 operator * (Vector4 const &vIn) const { return GetTransformMatrix() * vIn; }

	//Returns if the transform cache is updated 
	inline bool IsDirty() const { 
		return lastLocalChange != lastLocalSync;
	}

};

//Transform to Matrix methods that were not implemented in VectorMath.h
template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(weave::Transform const &tIn) { *this = (tIn.GetTransformMatrix()); }
template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator = (weave::Transform const &tIn) { *this = (tIn.GetTransformMatrix()); return *this; }

template<typename Type>
inline Matrix4x4T<Type>::Matrix4x4T(weave::TransformData const &tIn) { *this = (tIn.ToMatrix()); }
template<typename Type>
inline Matrix4x4T<Type>& Matrix4x4T<Type>::operator = (weave::TransformData const &tIn) { *this = (tIn.ToMatrix()); return *this; }

namespace interpolation {
	using namespace algebra;

	//Linear interpolation (lerp) specialization for Transform types
	template<>
	inline Transform lerp<Transform, float>(Transform const &a, Transform const &b, float p) {
		Transform t;
		t.Interpolate(a, b, p);
		return t;
	}

	//Linear interpolation (lerp) specialization for TransformData types
	template<>
	inline TransformData lerp<TransformData, float>(TransformData const &t1, TransformData const &t2, float u) {
		TransformData t;

		t.position = interpolation::lerp(t1.position, t2.position, u);
		t.scaling = interpolation::lerp(t1.scaling, t2.scaling, u);
		t.orientation = interpolation::slerp(t1.orientation, t2.orientation, u);

		return t;
	}
}

}
