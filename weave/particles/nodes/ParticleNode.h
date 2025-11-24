#pragma once

#include "weave/system/blender/BlenderNode.h"

#include "weave/particles/ParticleContext.h"

namespace weave::particles {

// Convenience alias for blender uniforms used by particle nodes.
using ParticleUniform = blender::Uniform<ParticleContext>;

// Shared base template for particle nodes.
template<typename ...InOutTypes>
class ParticleNode : public blender::BlenderNode<InOutTypes..., ParticleUniform> {
protected:
	ParticleContext& GetContext() {
		return this->uniform.template Ref<ParticleContext>();
	}

	ParticleContext const& GetContext() const {
		return this->uniform.template Ref<ParticleContext>();
	}
};

} // namespace weave::particles

