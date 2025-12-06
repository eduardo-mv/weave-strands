#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticlePhysicsSim : public ParticleNode<
	blender::In<float, Vector3>,
	blender::Out<>> {
public:
	ParticlePhysicsSim();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		LinearDamping, // Scalar applied to velocities each frame
		Gravity       // World gravity vector applied to all particles
	};
};

} // namespace weave::particles
