#include "VectorMath.h"
#include "Interpolation.h"

using namespace weave;
using namespace weave::algebra;

//Helper functions
//Returns a 32bit RGBA value of a Vector4. The values of the vector are clamped between 0 and 1
unsigned int weave::algebra::vec2rgba(Vector4 const & v) {
	unsigned int rgba = uint32_t(weave::algebra::clamp(v.r, 0.0f, 1.0f) * 255.0f) << 24;
	rgba |= uint32_t(weave::algebra::clamp(v.g, 0.0f, 1.0f) * 255.0f) << 16;
	rgba |= uint32_t(weave::algebra::clamp(v.b, 0.0f, 1.0f) * 255.0f) << 8;
	rgba |= uint32_t(weave::algebra::clamp(v.a, 0.0f, 1.0f) * 255.0f);
	return rgba;
}

//Returns a 32bit RGBA value of a Vector3. The values of the vector are clamped between 0 and 1
unsigned int weave::algebra::vec2rgba(Vector3 const & v) {
	unsigned int rgba = uint32_t(weave::algebra::clamp(v.r, 0.0f, 1.0f) * 255.0f) << 24;
	rgba |= uint32_t(weave::algebra::clamp(v.g, 0.0f, 1.0f) * 255.0f) << 16;
	rgba |= uint32_t(weave::algebra::clamp(v.b, 0.0f, 1.0f) * 255.0f) << 8;
	rgba |= uint32_t(255);
	return rgba;
}

//Converts a 32bit RGBA value to a Vector4. The values of the vector are clamped between 0 and 1
Vector4 weave::algebra::rgba2vec(unsigned int rgba) {
	return Vector4((rgba >> 24) / 255.0f, ((rgba & (uint32_t(255) << 16)) >> 16) / 255.0f, ((rgba & (uint32_t(255) << 8)) >> 8) / 255.0f, (rgba & (uint32_t(255))) / 255.0f);
}
