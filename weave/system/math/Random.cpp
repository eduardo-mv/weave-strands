#include "Random.h"

using namespace weave;
using namespace weave::algebra;

//RNG globals
std::random_device weave::rng::rngdev;
std::mt19937 weave::rng::engine(weave::rng::rngdev());

//Uniform distribution RNG specializations for float and double (they use a different provider)
template<>
float weave::rng::uniform<float>() {
	static std::uniform_real_distribution<float> rng;
	return rng(engine);
}

template<>
double weave::rng::uniform<double>() {
	static std::uniform_real_distribution<double> rng;
	return rng(engine);
}

template<>
float weave::rng::uniformMirror<float>() {
	static std::uniform_real_distribution<float> rng;
	return rng(engine) * 2.0f - 1.0f;
}

template<>
double weave::rng::uniformMirror<double>() {
	static std::uniform_real_distribution<double> rng;
	return rng(engine) * 2.0 - 1.0;
}

//Returns a random 3D point on the positive quadrant of the sphere defined by max and min points
 Vector3 weave::rng::pointQuadrant(Vector3 const &minPower, Vector3 const &maxPower) {
	Vector3 radius = maxPower - minPower;
	radius = radius * (Vector3(weave::rng::uniform<float>(), weave::rng::uniform<float>(), weave::rng::uniform<float>()));
	radius += minPower;

	return radius;
}

//Returns a random 2D point on the positive quadrant of the sphere defined by max and min points
 Vector2 weave::rng::pointQuadrant(Vector2 const &minPower, Vector2 const &maxPower) {
	Vector2 radius = maxPower - minPower;
	radius = radius * (Vector2(weave::rng::uniform<float>(), weave::rng::uniform<float>()));
	radius += minPower;

	return radius;
}

//Returns a random 3D point within the hyperdisc defined by minPower and maxPower
 Vector3 weave::rng::point(Vector3 const &minPower, Vector3 const &maxPower) {
	Vector3 radius = maxPower - minPower;
	radius = radius * (Vector3(weave::rng::uniformMirror<float>(), weave::rng::uniformMirror<float>(), weave::rng::uniformMirror<float>()));
	radius += Vector3(std::copysign(minPower.x, radius.x), std::copysign(minPower.y, radius.y), std::copysign(minPower.z, radius.z));

	return radius;
}

//Returns a random 2D point within the hyperdisc defined by minPower and maxPower
 Vector2 weave::rng::point(Vector2 const &minPower, Vector2 const &maxPower) {
	Vector2 radius = maxPower - minPower;
	radius = radius * (Vector2(weave::rng::uniformMirror<float>(), weave::rng::uniformMirror<float>()));
	radius += Vector2(std::copysign(minPower.x, radius.x), std::copysign(minPower.y, radius.y));

	return radius;
}

