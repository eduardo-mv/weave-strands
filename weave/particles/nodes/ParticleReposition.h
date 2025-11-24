#pragma once

#include "weave/particles/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleReposition : public ParticleNode<
	blender::In<float, float, float, float, float, float, float, float>,
	blender::Out<>> {
public:
	ParticleReposition();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		DelayedInit,
		IdleDistance,
		ForceFieldDistance,
		VelocityFieldDistance,
		ForceMagnitude,
		IdleToActiveTime,
		ActiveToIdleTime,
		Damping
	};

private:
	void ProcessEmissionBuffers(float delayedInit, float idleToActiveTime);
	void ProcessEditableBuffers(float delayedInit, float idleDistance, float forceFieldDistance,
		float velocityFieldDistance, float forceMagnitude, float idleToActiveTime,
		float activeToIdleTime, float damping);
};

} // namespace weave::particles

