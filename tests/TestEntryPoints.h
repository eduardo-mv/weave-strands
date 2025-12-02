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

namespace weave::tests::graphics::core {
TestReport TestImageData();
TestReport TestImageLoader();
TestReport TestMeshBuilder();
TestReport TestMeshData();
TestReport TestMeshLayout();
TestReport TestMeshLoader();
TestReport TestMeshUtilities();
TestReport TestPrimitives3D();
}

namespace weave::tests::system::memory {
TestReport TestDataType();
TestReport TestStreamingForge();
}
