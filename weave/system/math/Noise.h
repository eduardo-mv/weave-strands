/*
Title: "Noise library"
File: Noise.h
Author(s): Eduardo Martínez Vidal

Abstract:
	Noise implementations for perlin and simplex, with multi dimensional variations and loopign support

Update Log:
	

*/

#pragma once

#include <limits>
#include <algorithm>
#include <cmath>

namespace weave {

namespace noise {
	//Implements 3D perlin noise with floating point values. Returns a value within [-1,1]
	double perlin(double x, double y, double z);
	float perlin(float x, float y, float z);

	//Implements 2D,3D and 4D simplex noise. Values returned are within [-1,1]
	double simplex(double x, double y);
	double simplex(double x, double y, double z);
	double simplex(double x, double y, double z, double w);

	inline float simplex(float x, float y) { return (float)simplex((double)x,(double)y); }
	inline float simplex(float x, float y, float z) { return (float)simplex((double)x,(double)y,(double)z); }
	inline float simplex(float x, float y, float z, float w) { return (float)simplex((double)x,(double)y,(double)z,(double)w); }

	//Implements looping seamless 2D noise using cirular mapping and 4D simplex noise
	double circular(double x, double y, double freq);
	inline float circular(float x, float y, float freq) { return (float)circular((double)x, (double)y, (double)freq); }

	//Implements 2D, 3D and 4D seamless looping noise using a wrapping function and simplex noise
	double simplexLoop(double x, double y, double freq);
	double simplexLoop(double x, double y, double z, double freq);
	double simplexLoop(double x, double y, double z, double w, double freq);
	inline float simplexLoop(float x, float y, float freq) { return (float)simplexLoop((double)x,(double)y, (double)freq); }
	inline float simplexLoop(float x, float y, float z, float freq) { return (float)simplexLoop((double)x,(double)y,(double)z,(double)freq); }
	inline float simplexLoop(float x, float y, float z, float w, float freq) { return (float)simplexLoop((double)x,(double)y,(double)z,(double)w,(double)freq); }

	//Implements 2D and 3D seamless looping noise using a wrapping function and perlin classic noise
	double perlinLoop(double x, double y);
	double perlinLoop(double x, double y, double z);
	inline float perlinLoop(float x, float y) { return (float)perlinLoop((double)x,(double)y); }
	inline float perlinLoop(float x, float y, float z) { return (float)perlinLoop((double)x,(double)y,(double)z); }
}

} //namespace weave
