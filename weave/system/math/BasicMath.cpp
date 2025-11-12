#include "BasicMath.h"

using namespace weave;

//Solves the quadratic equation defined by the three terms a,b,c and returns the result in t0 and t1, where t0 is the smallest of the two solutions. Returns false if no solution can be found.
constexpr bool algebra::solveQuadratic(float a, float b, float c, float &t0, float &t1) {
	if(a == 0.0f)
		return false;
	//2 * A
	float a2 = 2.0f * a;

	//This goes inside the sqrt. If this is negative, the equation has no solution
	float n = b*b - 2.0f*a2*c;
	if(n < 0.0f) {
		return false;
	}

	//These are the two solutions for the system
	float rootn = std::sqrt(n);
	t0 = (-b + rootn) / a2;
	t1 = (-b - rootn) / a2;

	//Swap them so that t1 is always the biggest value
	if(t0 > t1)
		std::swap(t0, t1);

	return true;

}

//Solves the quadratic equation defined by the three terms a,b,c and returns the result in t0, where t0 is the smallest of the two solutions that is higher than 0.0f and lower than the supplied th. Returns false if no solution can be found that matches the criteria.
constexpr bool algebra::solveQuadraticCapped(float a, float b, float c, float thLow, float thHigh, float &tout) {
	if(a == 0.0f)
		return false;

	//This goes inside the sqrt. If this is negative, the equation has no solution
	float n = b*b - 4.0f*a*c;
	if(n < 0.0f) {
		return false;
	}

	//These are the two solutions for the system
	float rootn = std::sqrt(n);
	float t0 = (-b + rootn) / (2.0f * a);
	float t1 = (-b - rootn) / (2.0f * a);

	//Swap them so that t1 is always the biggest value
	if(t0 > t1)
		std::swap(t0, t1);

	//Cap the results to the criterial asked
	if(t0 > thLow && t0 < thHigh) {
		tout = t0;
		return true;
	}

	if(t1 > thLow && t1 < thHigh) {
		tout = t1;
		return true;
	}

	return false;
}