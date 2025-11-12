/*
Title: "Random support library based on C++11 random"
File: Random.h
Author(s): Eduardo Martínez Vidal

Abstract:
	Random number generation support based on C++11 random

Update Log:
	

*/

#pragma once

#include "VectorMath.h"
#include <random>

namespace weave {

namespace rng {
	//Extern global device and engine 
	extern std::random_device rngdev;
	extern std::mt19937 engine;
	
	//Uniform distribution RNG (between 0 and 1 for floating point values)
	template<typename Type>
	Type uniform() {
		static std::uniform_int_distribution<Type> rng;
		return rng(engine);
	}
	//Uniform distribution RNG specializations for float and double (they use a different provider)
	template<>
	float uniform<float>();
	template<>
	double uniform<double>();

	//Uniform distribution RNG between -1 and 1
	template<typename Type>
	Type uniformMirror() {
		static std::uniform_int_distribution<Type> rng;
		return rng(engine) * Type(2) - Type(1);
	}
	//Uniform distribution RNG specializations for float and double (they use a different provider)
	template<>
	float uniformMirror<float>();
	template<>
	double uniformMirror<double>();

	//Normal distribution RNG (only usable with reals)
	template<typename Type>
	Type normal() {
		static std::normal_distribution<Type> rng;
		return static_cast<Type>(rng(engine));
	}

	//Returns a random 3D point on the positive quadrant of the sphere defined by max and min points
	Vector3 pointQuadrant(Vector3 const &minPower, Vector3 const &maxPower);

	//Returns a random 2D point on the positive quadrant of the sphere defined by max and min points
	Vector2 pointQuadrant(Vector2 const &minPower, Vector2 const &maxPower);

	//Returns a random 3D point within the hyperdisc defined by minPower and maxPower
	Vector3 point(Vector3 const &minPower, Vector3 const &maxPower);

	//Returns a random 2D point within the hyperdisc defined by minPower and maxPower
	Vector2 point(Vector2 const &minPower, Vector2 const &maxPower);
}

} //namespace weave
