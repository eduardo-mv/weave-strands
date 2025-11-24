#pragma once

#include <cstddef>

#include "weave/particles/nodes/ParticleNode.h"

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
		MinRadius,
		MaxRadius,
		Alpha,
		Beta
	};

private:
	bool alphaHemisphere = false;
	bool betaHemisphere = false;
};

} // namespace weave::particles
