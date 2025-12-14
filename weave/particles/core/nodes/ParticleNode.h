#pragma once

#include "weave/system/blender/BlenderNode.h"

#include "weave/particles/core/ParticleContext.h"

namespace weave::particles {

// Convenience alias for blender uniforms used by particle nodes.
using ParticleUniform = blender::Uniform<ParticleContext>;

// Shared base template for particle nodes.
template<typename ...InOuFlowtTypes>
class ParticleNode : public blender::BlenderNode<InOuFlowtTypes..., ParticleUniform> {
protected:
	ParticleContext& GetContext() {
		return this->uniform.template Ref<ParticleContext>();
	}

	ParticleContext const& GetContext() const {
		return this->uniform.template Ref<ParticleContext>();
	}
};

} // namespace weave::particles

