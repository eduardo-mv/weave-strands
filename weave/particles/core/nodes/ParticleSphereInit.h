#pragma once

#include <cstddef>

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleSphereInit : public ParticleNode<
	blender::In<float, float, float, float>,
	blender::Out<>> {
public:
	ParticleSphereInit();

	void ExecuteNode() override;

	void SetAlphaHemisphere(bool value) { alphaHemisphere = value; }
	void SetBetaHemisphere(bool value) { betaHemisphere = value; }

	enum InputIndex : size_t {
		MinRadius, // Minimum spawn radius from the center
		MaxRadius, // Maximum spawn radius from the center
		Alpha,     // Longitudinal angle span
		Beta       // Latitudinal angle span
	};

private:
	bool alphaHemisphere = false;
	bool betaHemisphere = false;
};

} // namespace weave::particles
