#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleBufferSelector : public ParticleNode<
	blender::In<size_t>,
	blender::Out<>> {
public:
	void ExecuteNode() override;

	enum InputIndex : size_t {
		TargetBufferIndex,
		InputCount
	};
};

} // namespace weave::particles
