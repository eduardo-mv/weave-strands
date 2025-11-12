/*
Title: "Interpolation library"
File: Interpolation.h
Author(s): Eduardo Martínez Vidal

Abstract:
	General interpolation library with implementations for linear, spherical, cubic hermite and some variations

Update Log:

*/

#pragma once

#include "VectorMath.h"

namespace weave {


namespace interpolation {
	//using namespace algebra;

	//Linear interpolation (lerp)
	template<class T, class real>
	inline T lerp(T const &a, T const &b, real p) { return a + p*(b-a); }
	//Interger lerp
	template<class T = unsigned int, class real>
	inline unsigned int lerp(unsigned int const &a, unsigned int const &b, real p) { return (p >= 1.0f ?  b : a); }
	template<class T = int, class real>
	inline int lerp(int const &a, int const &b, real p) { return (p >= 1.0f ? b : a); }
	template<class T = uint64_t, class real>
	inline uint64_t lerp(uint64_t const &a, uint64_t const &b, real p) { return (p >= 1.0f ? b : a); }
	template<class T = int64_t, class real>
	inline int64_t lerp(int64_t const &a, int64_t const &b, real p) { return (p >= 1.0f ? b : a); }


	//Linear interpolation specialization for quaternions
	template <>
	inline quat lerp<quat,float>(quat const &q1, quat const &q2, float interp) {
		float dp = algebra::dot(q1,q2) > 0.0f ? 1.0f : -1.0f;
		//quat q = q1 + ((q2*dp)-q1)*interp;
		quat q = q1*(1-interp) + q2*(interp*dp);
		q.Normalize();
		return q;
	}

	//Smoothstep interpolation
	template<typename T>
	inline T smoothstep(T value) {
		if (value <= T(0)) {
			return T(0);
		}
		else if (value >= T(1)) {
			return T(1);
		}
		else {
			return value * value * (T(3) - T(2) * value);
		}
	}

	//Linear interpolation specialization for quaternions without ensuring a shortest path
	template <class real>
	inline quat longlerp(quat const &q1, quat const &q2, real interp) {
		quat q = q1*(1 - interp) + q2*(interp);
		q.Normalize();
		return q;
	}

	//Weighted interpolation
	template <class T, class real>
	inline T weight(T const &a, T const &b, real wa, real wb){ return a*wa + b*wb; }
	template <class T, class real>
	inline T weight(T const &a, T const &b, T const &c, real wa, real wb, real wc){ return a*wa + b*wb + c*wc; }

	//Weight specialization for quaternions
	template <>
	inline quat weight<quat,float>(quat const &q1, quat const &q2, float wa, float wb) {
		float dp = algebra::dot(q1,q2) > 0.0f ? 1.0f : -1.0f;
		quat q = q1*wa + q2*(wb*dp);
		q.Normalize();
		return q;
	}

	template <>
	inline quat weight<quat,float>(quat const &q1, quat const &q2, quat const &q3, float wa, float wb, float wc) {
		float dp = algebra::dot(q1,q2) > 0.0f ? 1.0f : -1.0f;
		quat q = q1*wa + q2*wb*dp;
		dp = algebra::dot(q,q3) > 0.0f ? 1.0f : -1.0f;
		q += q3*wc*dp;
		q.Normalize();
		return q;
	}

	//Weighted interpolation for multiple values
	template <class T, class real>
	inline T weight(T *values, real *weights, unsigned int count){ 
		T res = values[0]*weights[0];
		for(unsigned int i=1; i<count; ++i) 
			res += values[i]*weights[i]; 
		return res;
	}

	//Cosine interpolation
	template<class T, class real>
	inline T cosine(T const &a, T const &b, real p) { 	
		real mu = (1.0f-std::cos(p*algebra::F_PI))/2.0f;
		return(a*(1.0f-mu)+b*mu);
	}

	//Spherical linear interpolation of quaternions
	inline quat slerp(quat const &q, quat const &r, float t) {
		float qdot = algebra::dot(q,r);
		float absqdot = std::abs(qdot);
		if(absqdot < 0.00001f || absqdot > 0.999999f)
			return lerp(q, r, t);

		//if(qdot >= 1.0f || qdot <= -1.0f || std::abs(sinAngle) <= 0.0001f)
		//	return lerp(q, r, t);

		float dp = qdot >= 0.0f ? 1.0f : -1.0f;
		float angle = 2.0f * std::acos(qdot*dp);
		float sinAngle = sinf(angle);
		
		float a = sinf(angle*(1.0f-t)) / sinAngle;
		float b = sinf(angle*t) / sinAngle;

		//return algebra::normalize(q*a + r*(b*dp));
		return (q*a + r*(b*dp));
	}

	//Cubic interpolation. 
	//Points to interpolate: y1, y2. 
	//Control points: y0, y3
	template <class T, class real>
	inline T cubic(T y0, T y1, T y2, T y3, real t) {
		real t2 = t*t;
		T a0 = y3 - y2 - y0 + y1;
		T a1 = y0 - y1 - a0;
		T a2 = y2 - y0;
		T a3 = y1;

		return(a0*t*t2+a1*t2+a2*t+a3);
	}

	//Cubic interpolation witn Catmun-Rom coeficients.
	//Points to interpolate: y1, y2. 
	//Control points: y0, y3
	template <class T, class real>
	inline T catmunrom(T y0, T y1, T y2, T y3, real t) {
		real t2 = t*t;
		T a0 = -real(0.5)*y0 + real(1.5)*y1 - real(1.5)*y2 + real(0.5)*y3;
		T a1 = y0 - real(2.5)*y1 + real(2)*y2 - real(0.5)*y3;
		T a2 = -real(0.5)*y0 + real(0.5)*y2;
		T a3 = y1;

		return(a0*t*t2+a1*t2+a2*t+a3);
	}

	//Hermite interpolation. 
	//Tension: 1 is high, 0 normal, -1 is low
	//Bias: 0 is even, positive is towards first segment, negative towards the other
	//Points to interpolate: y1, y2. 
	//Control points: y0, y3
	//Implementation adapted from: http://paulbourke.net/miscellaneous/interpolation/
	template <class T, class real>
	inline T hermite(T y0, T y1, T y2, T y3, real mu, real tension, real bias) {
		real mu2 = mu * mu;
		real mu3 = mu2 * mu;

		real a0 =  2*mu3 - 3*mu2 + 1;
		real a1 =    mu3 - 2*mu2 + mu;
		real a2 =    mu3 -   mu2;
		real a3 = -2*mu3 + 3*mu2;

		real f = (1-tension)*(real)0.5;
		T m0 = (y1-y0)*((1+bias)*f) + (y2-y1)*((1-bias)*f);
		T m1 = (y2-y1)*((1+bias)*f) + (y3-y2)*((1-bias)*f);

		return(a0*y1+a1*m0+a2*m1+a3*y2);
	}
}

//Interpolate between two matrices and store the result
//Note that SRT matrix interpolation must be done by separating the scale, rotation and translation components, otherwise the result will be incorrect within the 3D gfx context
//This interpolation is thus generating wrong values if the matrix represents a SRT. Use Transforms and interpolate them.
template<typename Type>
inline void Matrix4x4T<Type>::Interpolate(Matrix4x4T const & m1, Matrix4x4T const & m2, Type p) {
	for(int i = 0; i<16; i++)
		this->data[i] = interpolation::lerp(m1[i], m2[i], p);
}

//Weights two matrices and stores the result
template<typename Type>
inline void Matrix4x4T<Type>::Weight(Matrix4x4T const & m1, Matrix4x4T const & m2, Type w1, Type w2) {
	for(int i = 0; i<16; i++)
		this->data[i] = interpolation::weight(m1[i], m2[i], w1, w2);
}

template<typename Type>
inline void Matrix3x3T<Type>::Interpolate(Matrix3x3T const & m1, Matrix3x3T const & m2, Type p) {
	for(int i = 0; i<9; i++)
		this->data[i] = interpolation::lerp(m1[i], m2[i], p);
}
template<typename Type>
inline void Matrix3x3T<Type>::Weight(Matrix3x3T const & m1, Matrix3x3T const & m2, Type w1, Type w2) {
	for(int i = 0; i<9; i++)
		this->data[i] = interpolation::weight(m1[i], m2[i], w1, w2);
}

template<typename Type>
inline void Matrix2x2T<Type>::Interpolate(Matrix2x2T const & m1, Matrix2x2T const & m2, Type p) {
	for(int i = 0; i<4; i++)
		this->data[i] = interpolation::lerp(m1[i], m2[i], p);
}

template<typename Type>
inline void Matrix2x2T<Type>::Weight(Matrix2x2T const & m1, Matrix2x2T const & m2, Type w1, Type w2) {
	for(int i = 0; i<4; i++)
		this->data[i] = interpolation::weight(m1[i], m2[i], w1, w2);
}


} //namespace weave
