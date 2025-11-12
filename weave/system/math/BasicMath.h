/*
Title: "Basic math definitions"
File: BasicMath.h
Author(s): Eduardo Martínez Vidal

Abstract:
	Definition of basic math constants and functions

Update Log:

*/

#pragma once

#include <limits>
#include <memory>
#include <cmath>
#include <algorithm>

#undef min
#undef max

namespace weave {

//Inline utilities
namespace algebra {
	//A small value that can be used as a general epsilon
	const constexpr float epsilonFloat = std::numeric_limits<float>::epsilon();
	const constexpr double epsilonDouble = std::numeric_limits<double>::epsilon();
	const constexpr float minFloat = std::numeric_limits<float>::min();
	const constexpr double minDouble = std::numeric_limits<double>::min();
		  
	const constexpr double D_2PI = 6.283185307179586476925286766559;
	const constexpr double D_PI = 3.1415926535897932384626433832795;
	const constexpr double D_PIH = 1.5707963267948966192313216916398;
		  
	const constexpr float F_2PI = 6.283185307179586476925286766559f;
	const constexpr float F_PI = 3.1415926535897932384626433832795f;
	const constexpr float F_PIH = 1.5707963267948966192313216916398f;

	inline constexpr double rad2deg(double rad) { return rad*180.0 / D_PI; };
	inline constexpr double deg2rad(double deg) { return D_PI*deg / 180.0; };

	inline constexpr float rad2deg(float rad) { return rad*180.0f / F_PI; };
	inline constexpr float deg2rad(float deg) { return F_PI*deg / 180.0f; };

	template<typename T>
	inline constexpr T rad2deg(T const &rad) { return rad*180.0f / F_PI; };
	template<typename T>
	inline constexpr T deg2rad(T const &deg) { return F_PI*deg / 180.0f; };

	#define RAD2DEG(x) (((x)*180.0f)/weave::algebra::F_PI)
	#define DEG2RAD(x) (((x)*weave::algebra::F_PI)/180.0f)

	//Solves the quadratic equation defined by the three terms a,b,c and returns the result in t0 and t1, where t0 is the smallest of the two solutions. Returns false if no solution can be found.
	bool constexpr solveQuadratic(float a, float b, float c, float &t0, float &t1);
	//Solves the quadratic equation defined by the three terms a,b,c and returns the result in t0, where t0 is the smallest of the two solutions that is higher or equal to thLow and lower or equal to thHigh. Returns false if no solution can be found that matches the criteria.
	bool constexpr solveQuadraticCapped(float a, float b, float c, float thLow, float thHigh, float &t0);
	

	//Returns the nearest upper power of two
	inline constexpr unsigned int upperPowerOfTwo(unsigned int v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;

	}

	inline constexpr unsigned long long upperPowerOfTwo(unsigned long long v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		v++;
		return v;
	}
}


} //namespace weave
