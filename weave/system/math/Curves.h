/*
Title: "Curves Math library"
File: Curves.h
Author(s): Eduardo Martínez Vidal

Abstract:
Curve and Splines definitions

Update Log:

*/

#pragma once
#ifdef _MSC_VER
#pragma warning( push )
//4201: Non standard nameless struct. 
#pragma warning(disable: 4201) 
#endif

#include "VectorMath.h"

namespace weave {

	//Forward declaration of curve template
template<typename Type>
class CurveT;

//Forward declaration of spline template
template<uint32_t, typename Type>
class SplineT;

template<typename Type>
class DynamicSplineT;

//Long name defines
using Curve = CurveT<float>;

template<uint32_t curveCount>
using Spline = SplineT<curveCount, float>;

using DynamicSpline = DynamicSplineT<float>;

//Template declarations
/*
Curve
Represents a cubic curve with the formula:

Curve = a*u^3 + b*u^2 + c*u^1 + d;

Where a,b,c and d are Vector4(x,y,z,1) (vertices) in space representing the control points.
The internal matrix represents the four control points.

Curve implements three interpolation methods for the curve.

Linear:
The curve will go through all 4 control points (p0,p1,p2,p3), such that:

d = p0;   (u=0);
a*(1/3)^3 + b*(1/3)^2 + c*(1/3)^1 + d = p1;	  (u=1/3);
a*(2/3)^3 + b*(2/3)^2 + c*(2/3)^1 + d = p2;   (u=2/3);
a         + b         + c         + d = p3;   (u=1);

Matrix form:
(A)				    (Coef)              (Control points)
(0   0   0   1  )   (a.x a.y a.z a.1)   (p0.x p0.y p0.z p0.1)
(1/3 1/3 1/3 1/3) * (b.x b.y b.z b.1) = (p1.x p1.y p1.z p1.1)
(2/3 2/3 2/3 2/3)   (c.x c.y c.z c.1)   (p2.x p2.y p2.z p2.1)
(1   1   1   1  )   (d.x d.y d.z d.1)   (p3.x p3.y p3.z p3.1)

Isolating (Coef):

(A')(A)(Coef)=(A')(Points)

(Coef)=(A')(Points)



Hermite:
Two points, p0 y p1 (start and end) and two derivatives of the curve on u=0 y u=1, p2 y p3. Placed into the matrix such as:
p0,p1,p2,p3

d             =p0;
a + b + c + d =p1;
c             =p2;  //Derivada, en u=0
3a + 2b + c   =p3;  //Derivada, en u=1

Isolation of (Coef) is the same as previously stated.


Bezier:
Two points, p0 y p3 (start and end) and two control points, one for p0 and one for p3, which we'll call p1 and p2.
Placed into the matrix such as:
p0,p1,p2,p3

p0'=p0;
p1'=p3;
p2'=3(p1-p0);  //3 es un factor de escalaje.
p3'=3(p3-p2);

hermite(p0',p1',p2',p3');

Bezier is implemented in two different versions that should give the same results: as a precalculated matrix or calculating the derivatives and using hermite.
*/

template<typename Type>
class CurveT {
private:
	Matrix4x4T<Type> points; //Points a,b,c,d defined according to the interpolation used
	Matrix4x4T<Type> scalars; //Coeficients for the chosen interpolation
	Matrix4x4T<Type> solution; //Solution composing the chosen interpolation method and the path

public:
	CurveT() { Bezier(); }
	CurveT(CurveT const &) = default;
	CurveT(CurveT &&) = default;
	CurveT(Vector3T<Type> const &a, Vector3T<Type> const &b, Vector3T<Type> const &c, Vector3T<Type> const &d);
	~CurveT() = default;

	//Returns the current control points
	Matrix4x4T<Type> const& GetPathPoints() const { return points; }

	//Sets the 4 path points to be used for the interpolation. Automaticaly creates a solution based on the current
	//coeficients.
	void SetPath(Vector3T<Type> const &a, Vector3T<Type> const &b, Vector3T<Type> const &c, Vector3T<Type> const &d);
	//Sets the first and second points of the path
	void SetPathStart(Vector3T<Type> const &a, Vector3T<Type> const &b);
	//Sets the third and fourth points of the path
	void SetPathEnd(Vector3T<Type> const &c, Vector3T<Type> const &d);

	//Sets the path start point using bezier (point + control point)
	void SetBezierStart(Vector3T<Type> const &point, Vector3T<Type> const &ctrlPoint);
	//Sets the path end point using bezier (point + control point)
	void SetBezierEnd(Vector3T<Type> const &point, Vector3T<Type> const &ctrlPoint);

	//Sets the interpolation method to Hermite and recalculates the solution according to the path points set
	void Hermite();
	//Sets the interpolation method to Linear and recalculates the solution according to the path points set
	void Linear();
	//Sets the interpolation method to Bezier and recalculates the solution according to the path points set.
	//Method flag forces using hermite as a final step instead.
	void Bezier(bool hermite = false);
	//Returns a point within the curve determined by the given factor. Valid values are between 0 and 1
	Vector3T<Type> SamplePoint(Type u) const;
};

//A sequence of bezier curves that can ensure continuity between control points
template<std::uint32_t curveCount, typename Type>
class SplineT {
private:
	//Array of curves that form the spline
	CurveT<Type> bcurves[curveCount];
	//Lengths of each curve, normalized. This is calculated by sampling the curve and adding the distances
	//This is used to sample the curve with a normalized value
	mutable Type lengths[curveCount] = {};
	//Cache flag that indicates the lengths cache is dirty and needs recalculating
	mutable bool isCacheDirty = true;

public:
	SplineT() = default;
	SplineT(SplineT const &) = default;
	SplineT(SplineT &&) = default;
	~SplineT() = default;

	//Returns the maximum amount of points allowed for this spline
	uint32_t GetMaxPoints() const { return curveCount + 1; }

	//Sets a continuous path point and its control point
	//The control point is specified from the point of view of the starting curve and the opposite control point for the previous curve is calculated automatically
	void SetPathPoint(uint32_t numPoint, Vector3T<Type> const &point, Vector3T<Type> const &control);

	//Sets a continuos path point with two discontinuos control points.
	//@controlStart: Control point on the starting curve
	//@controlEnd: Control point on the ending (previous) curve
	void SetPathPoint(uint32_t numPoint, Vector3T<Type> const &point, Vector3T<Type> const &controlStart, Vector3T<Type> const &controlEnd);

	//Sets two discontinuos path points with two discontinuos control points.
	//@pointStart: The point for the starting curve
	//@controlStart: The control point for the starting curve
	//@pointEnd: The point for the ending curve
	//@controlEnd: The control point for the ending curve
	void SetPathPoint(uint32_t numPoint, Vector3T<Type> const &pointStart, Vector3T<Type> const &controlStart, Vector3T<Type> const &pointEnd, Vector3T<Type> const &controlEnd);

	//Returns a point within the spline determined by the given factor. Valid values are between 0 and 1
	Vector3T<Type> SamplePoint(Type u) const;

private:
	//Recalculates the lengths cache
	void CalculateLenghts() const;
};


//A Spline that can be resized
template<typename Type>
class DynamicSplineT {
private:
	//Array of curves that form the spline
	mutable struct CurveStage {
		CurveT<Type> curve; //The curve for the stage
		Type length = 0.0f; //Lengths of each curve, normalized. This is calculated by sampling the curve and adding the distances
	} *bcurves = nullptr;

	//Counter for the number of points added to the spline. The amount of curves will by the same - 1
	size_t pointCount = 0;
	//Amount of curve stage objects allocated
	size_t stageSize = 0;

	//Cache flag that indicates the lengths cache is dirty and needs recalculating
	mutable bool isCacheDirty = true;

public:
	DynamicSplineT();
	DynamicSplineT(DynamicSplineT const &other);
	DynamicSplineT(DynamicSplineT &&other);
	~DynamicSplineT();

	DynamicSplineT& operator=(DynamicSplineT const &other);
	DynamicSplineT& operator=(DynamicSplineT &&other);

	//Resets the point count
	void ResetPoints();

	//Returns the maximum amount of points allowed for this spline
	size_t GetMaxPoints() const { return pointCount; }

	//Add a continuous path point and its control point
	//The control point is specified from the point of view of the starting curve and the opposite control point for the previous curve is calculated automatically
	void AddPathPoint(Vector3T<Type> const &point, Vector3T<Type> const &control);

	//Adds a continuos path point with two discontinuos control points.
	//@controlStart: Control point on the starting curve
	//@controlEnd: Control point on the ending (previous) curve
	void AddPathPoint(Vector3T<Type> const &point, Vector3T<Type> const &controlStart, Vector3T<Type> const &controlEnd);

	//Adds two discontinuos path points with two discontinuos control points.
	//@pointStart: The point for the starting curve
	//@controlStart: The control point for the starting curve
	//@pointEnd: The point for the ending curve
	//@controlEnd: The control point for the ending curve
	void AddPathPoint(Vector3T<Type> const &pointStart, Vector3T<Type> const &controlStart, Vector3T<Type> const &pointEnd, Vector3T<Type> const &controlEnd);

	//Sets a continuous path point and its control point
	//The control point is specified from the point of view of the starting curve and the opposite control point for the previous curve is calculated automatically
	void SetPathPoint(size_t numPoint, Vector3T<Type> const &point, Vector3T<Type> const &control);

	//Sets a continuos path point with two discontinuos control points.
	//@controlStart: Control point on the starting curve
	//@controlEnd: Control point on the ending (previous) curve
	void SetPathPoint(size_t numPoint, Vector3T<Type> const &point, Vector3T<Type> const &controlStart, Vector3T<Type> const &controlEnd);

	//Sets two discontinuos path points with two discontinuos control points.
	//@pointStart: The point for the starting curve
	//@controlStart: The control point for the starting curve
	//@pointEnd: The point for the ending curve
	//@controlEnd: The control point for the ending curve
	void SetPathPoint(size_t numPoint, Vector3T<Type> const &pointStart, Vector3T<Type> const &controlStart, Vector3T<Type> const &pointEnd, Vector3T<Type> const &controlEnd);

	//Returns the start, end and both control points of a given curve number
	void GetPathPoint(size_t numPoint, Vector3T<Type> &pointStart, Vector3T<Type> &controlStart, Vector3T<Type> &pointEnd, Vector3T<Type> &controlEnd) const;

	//Returns a point within the spline determined by the given factor. Valid values are between 0 and 1
	Vector3T<Type> SamplePoint(Type u) const;

	//Returns the path point number for a given factor. Valid values are between 0 and 1
	size_t FindPathPoint(Type u) const;

private:
	//Recalculates the lengths cache
	void CalculateLenghts() const;

	//Resizes the stage cache to fit the amount of points specified
	void ResizeStage(size_t numPoints);
};

//************************
//Template implementations
//************************

//---------
//CurveT impl

template<typename Type>
inline CurveT<Type>::CurveT(Vector3T<Type> const &a, Vector3T<Type> const &b, Vector3T<Type> const &c, Vector3T<Type> const &d) {
	points.SetRows(a, b, c, d);
	Bezier();
}

//Sets the 4 path points to be used for the interpolation. Automaticaly creates a solution based on the current
//coeficients.
template<typename Type>
inline void CurveT<Type>::SetPath(Vector3T<Type> const &a, Vector3T<Type> const &b, Vector3T<Type> const &c, Vector3T<Type> const &d) {
	points.SetRows(a, b, c, d);
	solution = scalars * points;
}
template<typename Type>
inline void CurveT<Type>::SetPathStart(Vector3T<Type> const & a, Vector3T<Type> const & b) {
	points.SetRow(0, a);
	points.SetRow(1, b);
	solution = scalars * points;
}

template<typename Type>
inline void CurveT<Type>::SetPathEnd(Vector3T<Type> const & c, Vector3T<Type> const & d) {
	points.SetRow(2, c);
	points.SetRow(3, d);
	solution = scalars * points;
}

template<typename Type>
inline void CurveT<Type>::SetBezierStart(Vector3T<Type> const & point, Vector3T<Type> const & ctrlPoint) {
	points.SetRow(0, point);
	points.SetRow(1, ctrlPoint);
	solution = scalars * points;
}

template<typename Type>
inline void CurveT<Type>::SetBezierEnd(Vector3T<Type> const & point, Vector3T<Type> const & ctrlPoint) {
	points.SetRow(2, ctrlPoint);
	points.SetRow(3, point);
	solution = scalars * points;
}

//Sets the interpolation method to Hermite and recalculates the solution according to the path points set
template<typename Type>
inline void CurveT<Type>::Hermite() {
	scalars = Matrix4x4T<Type>(
		1.0f, 0.0f, -3.0f, 2.0f,
		0.0f, 0.0f, 3.0f, -2.0f,
		0.0f, 1.0f, -2.0f, 1.0f,
		0.0f, 0.0f, -1.0f, 1.0f);

	solution = scalars * points;
}
//Sets the interpolation method to Linear and recalculates the solution according to the path points set
template<typename Type>
inline void CurveT<Type>::Linear() {
	scalars = Matrix4x4T<Type>(
		1.0f, -5.5f, 9.0f, -4.5f,
		0.0f, 9.0f, -22.5f, 13.5f,
		0.0f, -4.5f, 18.0f, -13.5f,
		0.0f, 1.0f, -4.5f, 4.5f);

	solution = scalars * points;
}
//Sets the interpolation method to Bezier and recalculates the solution according to the path points set.
//Method flag forces using hermite as a final step instead.
template<typename Type>
inline void CurveT<Type>::Bezier(bool hermite) {
	if(hermite) {
		//Calculate derived points before hermite
		//TODO: Fix this. This will fail with multiple Bezier(true) calls as the points will be shuffled around!
		Vector4T<Type> ap = points.GetRow(0);
		Vector4T<Type> bp = points.GetRow(3);
		Vector4T<Type> cp = 3.0f*(points.GetRow(1) - points.GetRow(0));
		Vector4T<Type> dp = 3.0f*(points.GetRow(3) - points.GetRow(2));

		points.SetRows(ap, bp, cp, dp);
		Hermite();
	}
	else {
		scalars = Matrix4x4T<Type>(
			1.0f, -3.0f, 3.0f, -1.0f,
			0.0f, 3.0f, -6.0f, 3.0f,
			0.0f, 0.0f, 3.0f, -3.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		solution = scalars * points;
	}
}

template<typename Type>
inline Vector3T<Type> CurveT<Type>::SamplePoint(Type u) const {
	float u2 = u * u;
	float u3 = u2 * u;

	return Vector3T<Type>(
		solution[0] + solution[1] * u + solution[2] * u2 + solution[3] * u3,
		solution[4] + solution[5] * u + solution[6] * u2 + solution[7] * u3,
		solution[8] + solution[9] * u + solution[10] * u2 + solution[11] * u3
	);
}

//End CurveT impl

//----------
//SplineT impl

template<uint32_t curveCount, typename Type>
inline void SplineT<curveCount, Type>::SetPathPoint(uint32_t numPoint, Vector3T<Type> const & point, Vector3T<Type> const & control) {
	if(numPoint < curveCount) {
		{
			//Set the starting point of the related curve
			CurveT<Type> &curve = bcurves[numPoint];
			curve.SetBezierStart(point, control);
		}

		if(numPoint > 0) {
			//Set the ending point of the previous curve to be the same, with an opposite control point
			Vector3T<Type> opCtrl = point + point - control;
			CurveT<Type> &curve = bcurves[numPoint - 1];
			curve.SetBezierEnd(point, opCtrl);
		}
	}
	else if(numPoint == curveCount) {
		//Set the ending point of the spline
		CurveT<Type> &curve = bcurves[numPoint - 1];
		curve.SetBezierEnd(point, control);
	}

	isCacheDirty = true;
}

template<uint32_t curveCount, typename Type>
inline void SplineT<curveCount, Type>::SetPathPoint(uint32_t numPoint, Vector3T<Type> const &point, Vector3T<Type> const &controlStart, Vector3T<Type> const &controlEnd) {
	if(numPoint < curveCount) {
		{
			//Set the starting point of the related curve with its starting control
			CurveT<Type> &curve = bcurves[numPoint];
			curve.SetBezierStart(point, controlStart);
		}

		if(numPoint > 0) {
			//Set the ending point of the previous curve to be the same and set the control point to the ending control
			CurveT<Type> &curve = bcurves[numPoint - 1];
			curve.SetBezierEnd(point, controlEnd);
		}
	}
	else if(numPoint == curveCount) {
		//Set the ending point of the spline
		CurveT<Type> &curve = bcurves[numPoint - 1];
		curve.SetBezierEnd(point, controlEnd);
	}

	isCacheDirty = true;
}

template<uint32_t curveCount, typename Type>
inline void SplineT<curveCount, Type>::SetPathPoint(uint32_t numPoint, Vector3T<Type> const &pointStart, Vector3T<Type> const &controlStart, Vector3T<Type> const &pointEnd, Vector3T<Type> const &controlEnd) {
	if(numPoint < curveCount) {
		{
			//Set the starting point of the related curve with its starting control
			CurveT<Type> &curve = bcurves[numPoint];
			curve.SetBezierStart(pointStart, controlStart);
		}

		if(numPoint > 0) {
			//Set the ending point of the previous curve to be the same and set the control point to the ending control
			CurveT<Type> &curve = bcurves[numPoint - 1];
			curve.SetBezierEnd(pointEnd, controlEnd);
		}
	}
	else if(numPoint == curveCount) {
		//Set the ending point of the spline
		CurveT<Type> &curve = bcurves[numPoint - 1];
		curve.SetBezierEnd(pointEnd, controlEnd);
	}

	isCacheDirty = true;

}

template<uint32_t curveCount, typename Type>
inline Vector3T<Type> SplineT<curveCount, Type>::SamplePoint(Type u) const {
	//Make sure the lenghts cache is ready
	CalculateLenghts();
	//Figure out which curve needs to be sampled
	for(uint32_t c = 0; c < curveCount; ++c) {
		if(u < lengths[c]) {
			//Convert the sampling parameter
			Type prevLen = (c > 0 ? lengths[c - 1] : 0.0f);
			Type uLocal = (u - prevLen) / (lengths[c] - prevLen);
			return bcurves[c].SamplePoint(uLocal);
		}
	}


	return Vector3T<Type>();
}

template<uint32_t curveCount, typename Type>
inline void SplineT<curveCount, Type>::CalculateLenghts() const {
	if(isCacheDirty) {
		Type totalLen = {};

		//Sample each curve a number of times and add up the length
		for(uint32_t c = 0; c < curveCount; ++c) {
			CurveT<Type> const &curve = bcurves[c];
			Type u = {};
			lengths[c] = {};

			Vector3T<Type> prevPoint = curve.SamplePoint(u);

			for(size_t i = 1; i < 10; ++i) {
				u += Type(1.0 / 10.0);
				Vector3T<Type> point = curve.SamplePoint(u);
				lengths[c] += weave::algebra::length(point - prevPoint);
			}

			//Accumulate the length
			totalLen += lengths[c];
		}

		//Normalize the lengths
		if(totalLen > 0.0f) {
			for(uint32_t c = 0; c < curveCount; ++c) {
				lengths[c] /= totalLen;
				if(c > 0)
					lengths[c] += lengths[c - 1];
			}
		}

		isCacheDirty = false;
	}
}

//End SplineT impl

//----------
//DynamicSplineT impl

template<typename Type>
DynamicSplineT<Type>::DynamicSplineT() : stageSize(2) {
	//Default buffer size of two curves
	bcurves = new CurveStage[stageSize];
}

template<typename Type>
DynamicSplineT<Type>::DynamicSplineT(DynamicSplineT const & other) {
	bcurves = new CurveStage[other.stageSize];
	std::memcpy(bcurves, other.bcurves, sizeof(CurveStage) * other.stageSize);
	pointCount = other.pointCount;
	stageSize = other.stageSize;
	isCacheDirty = other.isCacheDirty;
}


template<typename Type>
DynamicSplineT<Type>::DynamicSplineT(DynamicSplineT && other) {
	bcurves = other.bcurves;
	pointCount = other.pointCount;
	stageSize = other.stageSize;
	isCacheDirty = other.isCacheDirty;

	other.bcurves = nullptr;
	other.pointCount = 0;
	other.stageSize = 0;
	other.isCacheDirty = true;
}


template<typename Type>
DynamicSplineT<Type>::~DynamicSplineT() {
	delete[] bcurves;
}

template<typename Type>
DynamicSplineT<Type>& DynamicSplineT<Type>::operator=(DynamicSplineT<Type> const & other) {
	delete[] bcurves;

	bcurves = new CurveStage[other.stageSize];
	std::memcpy(bcurves, other.bcurves, sizeof(CurveStage) * other.stageSize);
	pointCount = other.pointCount;
	stageSize = other.stageSize;
	isCacheDirty = other.isCacheDirty;

	return *this;
}

template<typename Type>
DynamicSplineT<Type>& DynamicSplineT<Type>::operator=(DynamicSplineT<Type> && other) {
	delete[] bcurves;

	bcurves = other.bcurves;
	pointCount = other.pointCount;
	stageSize = other.stageSize;
	isCacheDirty = other.isCacheDirty;

	other.bcurves = nullptr;
	other.pointCount = 0;
	other.stageSize = 0;
	other.isCacheDirty = true;

	return *this;
}


template<typename Type>
inline void DynamicSplineT<Type>::ResetPoints() {
	pointCount = 0;
	isCacheDirty = true;
}

template<typename Type>
void DynamicSplineT<Type>::AddPathPoint(Vector3T<Type> const & point, Vector3T<Type> const & control) {
	//Make sure we have enough space to store the points and edit the new point
	++pointCount;
	ResizeStage(pointCount);
	SetPathPoint(pointCount - 1, point, control);
}

template<typename Type>
void DynamicSplineT<Type>::AddPathPoint(Vector3T<Type> const & point, Vector3T<Type> const & controlStart, Vector3T<Type> const & controlEnd) {
	//Make sure we have enough space to store the points and edit the new point
	++pointCount;
	ResizeStage(pointCount);
	SetPathPoint(pointCount - 1, point, controlStart, controlEnd);
}

template<typename Type>
void DynamicSplineT<Type>::AddPathPoint(Vector3T<Type> const & pointStart, Vector3T<Type> const & controlStart, Vector3T<Type> const & pointEnd, Vector3T<Type> const & controlEnd) {
	//Make sure we have enough space to store the points and edit the new point
	++pointCount;
	ResizeStage(pointCount);
	SetPathPoint(pointCount - 1, pointStart, controlStart, pointEnd, controlEnd);
}

template<typename Type>
inline void DynamicSplineT<Type>::SetPathPoint(size_t numPoint, Vector3T<Type> const & point, Vector3T<Type> const & control) {
	if(numPoint < pointCount) {
		//Add the new point into the corresponding curve
		{
			//Set the starting point of the related curve
			CurveT<Type> &curve = bcurves[numPoint].curve;
			curve.SetBezierStart(point, control);
		}

		if(numPoint > 0) {
			//Set the ending point of the previous curve to be the same, with an opposite control point
			Vector3T<Type> opCtrl = point + point - control;
			CurveT<Type> &curve = bcurves[numPoint - 1].curve;
			curve.SetBezierEnd(point, opCtrl);
		}

		isCacheDirty = true;
	}
}

template<typename Type>
inline void DynamicSplineT<Type>::SetPathPoint(size_t numPoint, Vector3T<Type> const &point, Vector3T<Type> const &controlStart, Vector3T<Type> const &controlEnd) {
	if(numPoint < pointCount) {
		//Add the new point into the corresponding curve
		{
			//Set the starting point of the related curve
			CurveT<Type> &curve = bcurves[numPoint].curve;
			curve.SetBezierStart(point, controlStart);
		}

		if(numPoint > 0) {
			//Set the ending point of the previous curve to be the same, with an opposite control point
			CurveT<Type> &curve = bcurves[numPoint - 1].curve;
			curve.SetBezierEnd(point, controlEnd);
		}

		isCacheDirty = true;
	}
}

template<typename Type>
inline void DynamicSplineT<Type>::SetPathPoint(size_t numPoint, Vector3T<Type> const &pointStart, Vector3T<Type> const &controlStart, Vector3T<Type> const &pointEnd, Vector3T<Type> const &controlEnd) {
	if(numPoint < pointCount) {
		//Add the new point into the corresponding curve
		{
			//Set the starting point of the related curve
			CurveT<Type> &curve = bcurves[numPoint].curve;
			curve.SetBezierStart(pointStart, controlStart);
		}

		if(numPoint > 0) {
			//Set the ending point of the previous curve to be the same, with an opposite control point
			CurveT<Type> &curve = bcurves[numPoint - 1].curve;
			curve.SetBezierEnd(pointEnd, controlEnd);
		}

		isCacheDirty = true;
	}
}

template<typename Type>
inline void DynamicSplineT<Type>::GetPathPoint(size_t numPoint, Vector3T<Type>& pointStart, Vector3T<Type>& controlStart, Vector3T<Type>& pointEnd, Vector3T<Type>& controlEnd) const {
	if(numPoint < pointCount) {
		//Set the starting point of the related curve
		CurveT<Type> &curve = bcurves[numPoint].curve;
		pointStart = curve.GetPathPoints().GetRow(0).xyz;
		controlStart = curve.GetPathPoints().GetRow(1).xyz;
		controlEnd = curve.GetPathPoints().GetRow(2).xyz;
		pointEnd = curve.GetPathPoints().GetRow(3).xyz;
	}
}

template<typename Type>
inline Vector3T<Type> DynamicSplineT<Type>::SamplePoint(Type u) const {
	//Make sure the lenghts cache is ready
	CalculateLenghts();
	//Figure out which curve needs to be sampled
	for(uint32_t c = 1; c < pointCount; ++c) {
		CurveStage &bcurve = bcurves[c - 1];

		if(u <= bcurve.length) {
			//Convert the sampling parameter
			Type prevLen = (c > 1 ? bcurves[c - 2].length : 0.0f);
			Type uLocal = (u - prevLen) / (bcurve.length - prevLen);
			return bcurve.curve.SamplePoint(uLocal);
		}
	}


	return Vector3T<Type>();
}

template<typename Type>
inline size_t DynamicSplineT<Type>::FindPathPoint(Type u) const {
	//Make sure the lenghts cache is ready
	CalculateLenghts();
	//Figure out which curve needs to be sampled
	for(uint32_t c = 1; c < pointCount; ++c) {
		CurveStage &bcurve = bcurves[c - 1];

		if(u <= bcurve.length) {
			//Convert the sampling parameter
			Type prevLen = (c > 1 ? bcurves[c - 2].length : 0.0f);
			Type uLocal = (u - prevLen) / (bcurve.length - prevLen);
			return (uLocal > 0.5f ? c : c-1);
		}
	}

	return 0;
}

template<typename Type>
inline void DynamicSplineT<Type>::CalculateLenghts() const {
	if(isCacheDirty) {
		Type totalLen = {};

		//Sample each curve a number of times and add up the length
		for(uint32_t c = 1; c < pointCount; ++c) {
			CurveStage &bcurve = bcurves[c - 1];
			Type u = {};

			Vector3T<Type> prevPoint = bcurve.curve.SamplePoint(u);

			for(size_t i = 1; i < 10; ++i) {
				u += Type(1.0 / 10.0);
				Vector3T<Type> point = bcurve.curve.SamplePoint(u);
				bcurve.length += weave::algebra::length(point - prevPoint);
			}

			//Accumulate the length
			totalLen += bcurve.length;
		}

		//Normalize the lengths
		if(totalLen > 0.0f) {
			for(uint32_t c = 0; c < pointCount; ++c) {
				CurveStage &bcurve = bcurves[c];
				bcurve.length /= totalLen;
				if(c > 0)
					bcurve.length += bcurves[c - 1].length;
			}

			//Make sure the last curve is set at 1.0f
			if(pointCount > 1)
				bcurves[pointCount - 2].length = 1.0f;
		}

		isCacheDirty = false;
	}
}

//Resizes the stage cache to fit the amount of points specified
template<typename Type>
void DynamicSplineT<Type>::ResizeStage(size_t numPoints) {
	if(numPoints > stageSize) {
		auto *oldcurves = bcurves;
		bcurves = new CurveStage[numPoints];
		std::memcpy(bcurves, oldcurves, sizeof(CurveStage) * stageSize);
		stageSize = numPoints;
		delete[] oldcurves;
	}
}

//End DynamicSplineT impl

} //namespace weave

#ifdef _MSC_VER
#pragma warning( pop )
#endif
