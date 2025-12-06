#pragma once

#include "weave/particles/core/nodes/ParticleNode.h"

namespace weave::particles {

class ParticleReposition : public ParticleNode<
	blender::In<float, float, float, float, float, float, float, float>,
	blender::Out<>> {
public:
	ParticleReposition();

	void ExecuteNode() override;

	enum InputIndex : size_t {
		DelayedInit,           // Time before newly spawned particles start following targets
		IdleDistance,          // Radius that keeps particles in the idle region
		ForceFieldDistance,    // Radius of the positional force field
		VelocityFieldDistance, // Radius of the velocity field that steers travel
		ForceMagnitude,        // Strength of the positional force toward the target
		IdleToActiveTime,      // Duration to ramp from idle to chasing
		ActiveToIdleTime,      // Duration to relax back to idle
		Damping                // Velocity damping while homing
	};

private:
	void ProcessEmissionBuffers(float delayedInit, float idleToActiveTime);
	void ProcessEditableBuffers(float delayedInit, float idleDistance, float forceFieldDistance,
		float velocityFieldDistance, float forceMagnitude, float idleToActiveTime,
		float activeToIdleTime, float damping);
};

} // namespace weave::particles
