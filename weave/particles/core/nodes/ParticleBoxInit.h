#pragma once

#include <cstddef>

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleBoxInit : public ParticleNode<
	blender::In<float, float, float, float, float, float>,
	blender::Out<>> {
public:
	ParticleBoxInit();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		MinX,
		MinY,
		MinZ,
		MaxX,
		MaxY,
		MaxZ
	};

private:
	int faceOut = 0;
};

} // namespace weave::particles
