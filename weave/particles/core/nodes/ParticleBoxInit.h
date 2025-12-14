#pragma once

#include <cstddef>

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleBoxInit : public ParticleNode<
	blender::In<float, float, float, float, float, float>,
	blender::Out<>,
	blender::Flow<EmissionRange>> {
public:
	ParticleBoxInit();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		MinX, // Minimum X extent of the spawn box
		MinY, // Minimum Y extent of the spawn box
		MinZ, // Minimum Z extent of the spawn box
		MaxX, // Maximum X extent of the spawn box
		MaxY, // Maximum Y extent of the spawn box
		MaxZ  // Maximum Z extent of the spawn box
	};

private:
	int faceOut = 0;
};

} // namespace weave::particles
