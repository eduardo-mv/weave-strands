#pragma once

#include "tests/TestReport.h"

namespace weave::tests::blender {
TestReport TestNodes();
}

namespace weave::tests::particles {
TestReport TestParticleBuffer();
TestReport RunParticleMachineSelfTest();
}

namespace weave::tests::scenegraph {
TestReport RunSceneGraphSelfTest();
}
