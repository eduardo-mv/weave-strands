const ARITHMETIC_HEADER = "weave/system/blender/nodes/ArithmeticNodes.h";
const CONVERSION_HEADER = "weave/system/blender/nodes/ConversionNodes.h";
const CONDITIONAL_HEADER = "weave/system/blender/nodes/ConditionalNodes.h";
const EASING_NODES_HEADER = "weave/system/blender/nodes/EasingNodes.h";
const EASING_HEADER = "weave/system/math/Easing.h";
const RNG_HEADER = "weave/system/blender/nodes/RngNodes.h";
const NOISE_HEADER = "weave/system/blender/nodes/NoiseNodes.h";
const SIGNAL_SAMPLER_HEADER = "weave/animation/blender/samplers/SignalSampler.h";

const TYPE_VARIANTS = [
  { id: "float", suffix: "Float", varSuffix: "Float", label: "Float", typeGroup: "Float", typeName: "float", zero: "0.0f", one: "1.0f" },
  { id: "vec2", suffix: "Vec2", varSuffix: "Vec2", label: "Vector2", typeGroup: "Vector2", typeName: "Vector2", zero: "Vector2{}", one: "Vector2{1.0f, 1.0f}" },
  { id: "vec3", suffix: "Vec3", varSuffix: "Vec3", label: "Vector3", typeGroup: "Vector3", typeName: "Vector3", zero: "Vector3{}", one: "Vector3{1.0f, 1.0f, 1.0f}" },
  { id: "vec4", suffix: "Vec4", varSuffix: "Vec4", label: "Vector4", typeGroup: "Vector4", typeName: "Vector4", zero: "Vector4{}", one: "Vector4{1.0f, 1.0f, 1.0f, 1.0f}" }
];

const TYPE_VARIANT_MAP = Object.fromEntries(TYPE_VARIANTS.map(variant => [variant.id, variant]));
const TRIGGER_PORT_KEY = "__trigger__";

function normalizeDataType(value) {
  if (value == null) {
    return null;
  }
  const normalized = String(value).trim().toLowerCase();
  if (!normalized) {
    return null;
  }
  switch (normalized) {
    case "vector2":
      return "vec2";
    case "vector3":
      return "vec3";
    case "vector4":
      return "vec4";
    case "int32":
    case "i32":
    case "int":
      return "int";
    case "uint32":
    case "u32":
      return "uint";
    default:
      return normalized;
  }
}

function formatDataTypeLabel(dataType) {
  if (!dataType) {
    return null;
  }
  const variant = TYPE_VARIANT_MAP[dataType];
  if (variant?.label) {
    return variant.label;
  }
  return dataType.charAt(0).toUpperCase() + dataType.slice(1);
}

function buildPortTooltip(label, typeLabel, hint) {
  const base = typeLabel ? `${label} (${typeLabel})` : label;
  if (hint) {
    return `${base} — ${hint}`;
  }
  return base;
}

const PARTICLE_NODES = [
  {
    typeId: "ParticleEmitter",
    label: "Particle Emitter",
    description: "Spawns new particles using emission counts and frequency windows.",
    cppType: "wp::ParticleEmitter",
    typeGroup: "Emitter",
    include: "weave/particles/core/nodes/ParticleEmitter.h",
    varPrefix: "emitter",
    inputs: [
      { key: "MinEmit", label: "Min Emit", cppAccessor: "MinEmit", defaultValue: "1.0f", dataType: "float" },
      { key: "MaxEmit", label: "Max Emit", cppAccessor: "MaxEmit", defaultValue: "5.0f", dataType: "float" },
      { key: "Rate", label: "Rate Multiplier", cppAccessor: "Rate", defaultValue: "1.0f", dataType: "float" },
      { key: "MinFrequency", label: "Min Frequency", cppAccessor: "MinFrequency", defaultValue: "0.0f", dataType: "float" },
      { key: "MaxFrequency", label: "Max Frequency", cppAccessor: "MaxFrequency", defaultValue: "0.0f", dataType: "float" },
      { key: "MaxRuntime", label: "Max Runtime (seconds)", cppAccessor: "MaxRuntime", defaultValue: "-1.0", dataType: "float" },
      { key: "MaxParticles", label: "Max Particles", cppAccessor: "MaxParticles", defaultValue: "2048", dataType: "float" },
      { key: "ResetSignal", label: "Reset Signal", cppAccessor: "ResetSignal", defaultValue: "0u" }
    ]
  },
  {
    typeId: "ParticleBoxInit",
    label: "Box Init",
    description: "Places particle positions along the faces of a box volume.",
    cppType: "wp::ParticleBoxInit",
    typeGroup: "Initializer",
    include: "weave/particles/core/nodes/ParticleBoxInit.h",
    varPrefix: "boxInit",
    inputs: [
      { key: "MinX", label: "Min X", cppAccessor: "MinX", defaultValue: "0.0f", dataType: "float" },
      { key: "MinY", label: "Min Y", cppAccessor: "MinY", defaultValue: "0.0f", dataType: "float" },
      { key: "MinZ", label: "Min Z", cppAccessor: "MinZ", defaultValue: "0.0f", dataType: "float" },
      { key: "MaxX", label: "Max X", cppAccessor: "MaxX", defaultValue: "1.0f", dataType: "float" },
      { key: "MaxY", label: "Max Y", cppAccessor: "MaxY", defaultValue: "1.0f", dataType: "float" },
      { key: "MaxZ", label: "Max Z", cppAccessor: "MaxZ", defaultValue: "1.0f", dataType: "float" }
    ]
  },
  {
    typeId: "ParticleSphereInit",
    label: "Sphere Init",
    description: "Initializes particle positions within a sphere.",
    cppType: "wp::ParticleSphereInit",
    typeGroup: "Initializer",
    include: "weave/particles/core/nodes/ParticleSphereInit.h",
    varPrefix: "sphere",
    inputs: [
      { key: "MinRadius", label: "Min Radius", cppAccessor: "MinRadius", defaultValue: "0.0f", dataType: "float" },
      { key: "MaxRadius", label: "Max Radius", cppAccessor: "MaxRadius", defaultValue: "0.5f", dataType: "float" },
      { key: "Alpha", label: "Alpha", cppAccessor: "Alpha", defaultValue: "weave::algebra::F_2PI", dataType: "float" },
      { key: "Beta", label: "Beta", cppAccessor: "Beta", defaultValue: "weave::algebra::F_2PI", dataType: "float" }
    ],
    options: [
      { key: "alphaHemisphere", label: "Clamp Alpha to Hemisphere", type: "bool", defaultValue: false, method: "SetAlphaHemisphere" },
      { key: "betaHemisphere", label: "Clamp Beta to Hemisphere", type: "bool", defaultValue: false, method: "SetBetaHemisphere" }
    ]
  },
  {
    typeId: "ParticleVelocityInit",
    label: "Velocity Init",
    description: "Configures spawn velocity cones.",
    cppType: "wp::ParticleVelocityInit",
    typeGroup: "Initializer",
    include: "weave/particles/core/nodes/ParticleVelocityInit.h",
    varPrefix: "velocity",
    inputs: [
      { key: "Direction", label: "Direction", cppAccessor: "Direction", defaultValue: "Vector3{0.0f, 1.0f, 0.0f}", dataType: "vec3" },
      { key: "UpAngle", label: "Up Angle", cppAccessor: "UpAngle", defaultValue: "30.0f", dataType: "float" },
      { key: "SideAngle", label: "Side Angle", cppAccessor: "SideAngle", defaultValue: "20.0f", dataType: "float" },
      { key: "MinVelocity", label: "Min Velocity", cppAccessor: "MinVelocity", defaultValue: "0.4f", dataType: "float" },
      { key: "MaxVelocity", label: "Max Velocity", cppAccessor: "MaxVelocity", defaultValue: "1.1f", dataType: "float" }
    ]
  },
  {
    typeId: "ParticlePhysicsInit",
    label: "Physics Init",
    description: "Seeds lifetime and mass attributes.",
    cppType: "wp::ParticlePhysicsInit",
    typeGroup: "Initializer",
    include: "weave/particles/core/nodes/ParticlePhysicsInit.h",
    varPrefix: "physicsInit",
    inputs: [
      { key: "MinAgeInput", label: "Min Lifetime", cppAccessor: "MinAgeInput", defaultValue: "0.8f", dataType: "float" },
      { key: "MaxAgeInput", label: "Max Lifetime", cppAccessor: "MaxAgeInput", defaultValue: "2.5f", dataType: "float" },
      { key: "MinMassInput", label: "Min Mass", cppAccessor: "MinMassInput", defaultValue: "0.2f", dataType: "float" },
      { key: "MaxMassInput", label: "Max Mass", cppAccessor: "MaxMassInput", defaultValue: "1.0f", dataType: "float" }
    ]
  },
  {
    typeId: "ParticleTransformInit",
    label: "Transform Init",
    description: "Applies emitter transform offsets.",
    cppType: "wp::ParticleTransformInit",
    typeGroup: "Initializer",
    include: "weave/particles/core/nodes/ParticleTransformInit.h",
    varPrefix: "transform",
    inputs: [
      { key: "TransformInput", label: "Transform", cppAccessor: "TransformInput", defaultValue: "Transform{}", dataType: "transform" },
      { key: "ApplyEmission", label: "Apply Emission", cppAccessor: "ApplyEmission", defaultValue: "true" },
      { key: "ApplyEditable", label: "Apply Editable", cppAccessor: "ApplyEditable", defaultValue: "true" },
      { key: "TranslatePosition", label: "Translate Position", cppAccessor: "TranslatePosition", defaultValue: "true" },
      { key: "RotatePosition", label: "Rotate Position", cppAccessor: "RotatePosition", defaultValue: "true" },
      { key: "ScalePosition", label: "Scale Position", cppAccessor: "ScalePosition", defaultValue: "false" },
      { key: "RotateVelocity", label: "Rotate Velocity", cppAccessor: "RotateVelocity", defaultValue: "true" },
      { key: "ScaleVelocity", label: "Scale Velocity", cppAccessor: "ScaleVelocity", defaultValue: "false" }
    ]
  },
  {
    typeId: "ParticleRngVelocityInit",
    label: "RNG Velocity Init",
    description: "Assigns random velocity vectors with optional axis limits or radial mode.",
    cppType: "wp::ParticleRngVelocityInit",
    typeGroup: "Initializer",
    include: "weave/particles/core/nodes/ParticleRngVelocityInit.h",
    varPrefix: "rngVelocity",
    inputs: [
      { key: "MinVelocity", label: "Min Velocity", cppAccessor: "MinVelocity", defaultValue: "0.0f", dataType: "float" },
      { key: "MaxVelocity", label: "Max Velocity", cppAccessor: "MaxVelocity", defaultValue: "1.0f", dataType: "float" },
      { key: "LimitX", label: "Limit X (-1..1)", cppAccessor: "LimitX", defaultValue: "0", dataType: "float" },
      { key: "LimitY", label: "Limit Y (-1..1)", cppAccessor: "LimitY", defaultValue: "0", dataType: "float" },
      { key: "LimitZ", label: "Limit Z (-1..1)", cppAccessor: "LimitZ", defaultValue: "0", dataType: "float" },
      { key: "Radial", label: "Radial From Position", cppAccessor: "Radial", defaultValue: "false" }
    ]
  },
  {
    typeId: "CommitEmissionNode",
    label: "Commit Emission",
    description: "Pushes staged emission buffers into the live set.",
    cppType: "wp::CommitEmissionNode",
    typeGroup: "Initializer",
    include: "weave/particles/core/nodes/CommitEmissionNode.h",
    varPrefix: "commit",
    inputs: []
  },
  {
    typeId: "ParticleAgingSim",
    label: "Aging Sim",
    description: "Applies lifetime decay each frame.",
    cppType: "wp::ParticleAgingSim",
    typeGroup: "Simulator",
    typeGroup: "Simulator",
    include: "weave/particles/core/nodes/ParticleAgingSim.h",
    varPrefix: "aging",
    inputs: [
      { key: "TransformInput", label: "Transform", cppAccessor: "TransformInput", defaultValue: "Transform{}", dataType: "transform" },
      { key: "RadiusInput", label: "Radius", cppAccessor: "RadiusInput", defaultValue: "-1.0f", dataType: "float" },
      { key: "AgingMultiplierInput", label: "Aging Multiplier", cppAccessor: "AgingMultiplierInput", defaultValue: "1.0f", dataType: "float" }
    ]
  },
  {
    typeId: "ParticlePhysicsSim",
    label: "Physics Sim",
    description: "Integrates particle forces over time.",
    cppType: "wp::ParticlePhysicsSim",
    typeGroup: "Simulator",
    typeGroup: "Simulator",
    include: "weave/particles/core/nodes/ParticlePhysicsSim.h",
    varPrefix: "physics",
    inputs: [
      { key: "LinearDamping", label: "Linear Damping", cppAccessor: "LinearDamping", defaultValue: "0.985f", dataType: "float" },
      { key: "Gravity", label: "Gravity", cppAccessor: "Gravity", defaultValue: "Vector3{0.0f, -1.0f, 0.0f}", dataType: "vec3" }
    ]
  },
  {
    typeId: "ParticleAttractorSim",
    label: "Attractor Sim",
    description: "Pulls particles toward a transform or along a direction with falloff.",
    cppType: "wp::ParticleAttractorSim",
    typeGroup: "Simulator",
    include: "weave/particles/core/nodes/ParticleAttractorSim.h",
    varPrefix: "attractor",
    inputs: [
      { key: "AttractorTransform", label: "Transform", cppAccessor: "AttractorTransform", defaultValue: "Transform{}", dataType: "transform" },
      { key: "ForceMagnitude", label: "Force Magnitude", cppAccessor: "ForceMagnitude", defaultValue: "1.0f", dataType: "float" },
      { key: "GravityScale", label: "Gravity Scale", cppAccessor: "GravityScale", defaultValue: "0.0f", dataType: "float" },
      { key: "Radius", label: "Radius", cppAccessor: "Radius", defaultValue: "5.0f", dataType: "float" },
      { key: "Decay", label: "Decay", cppAccessor: "Decay", defaultValue: "1.0f", dataType: "float" },
      { key: "Direction", label: "Direction (optional)", cppAccessor: "Direction", defaultValue: "Vector3{}", dataType: "vec3" }
    ]
  },
  {
    typeId: "ParticleDragSim",
    label: "Drag Sim",
    description: "Applies linear/exponential drag with velocity caps.",
    cppType: "wp::ParticleDragSim",
    typeGroup: "Simulator",
    include: "weave/particles/core/nodes/ParticleDragSim.h",
    varPrefix: "drag",
    inputs: [
      { key: "LinearDrag", label: "Linear Drag", cppAccessor: "LinearDrag", defaultValue: "1.0f", dataType: "float" },
      { key: "ExponentialDrag", label: "Exponential Drag", cppAccessor: "ExponentialDrag", defaultValue: "0.0f", dataType: "float" },
      { key: "MinVelocityCap", label: "Min Velocity Cap", cppAccessor: "MinVelocityCap", defaultValue: "0.0f", dataType: "float" },
      { key: "MaxVelocityCap", label: "Max Velocity Cap", cppAccessor: "MaxVelocityCap", defaultValue: "-1.0f", dataType: "float" },
      { key: "BreakVelocity", label: "Break Velocity", cppAccessor: "BreakVelocity", defaultValue: "0.0f", dataType: "float" }
    ]
  },
  {
    typeId: "ParticleLifeAdjustSim",
    label: "Life Adjust Sim",
    description: "Clamps particle lifetimes, optionally inside a radius.",
    cppType: "wp::ParticleLifeAdjustSim",
    typeGroup: "Simulator",
    include: "weave/particles/core/nodes/ParticleLifeAdjustSim.h",
    varPrefix: "lifeAdjust",
    inputs: [
      { key: "TransformInput", label: "Transform", cppAccessor: "TransformInput", defaultValue: "Transform{}", dataType: "transform" },
      { key: "RadiusInput", label: "Radius", cppAccessor: "RadiusInput", defaultValue: "5.0f", dataType: "float" },
      { key: "MinLifeInput", label: "Min Life", cppAccessor: "MinLifeInput", defaultValue: "0.25f", dataType: "float" },
      { key: "MaxLifeInput", label: "Max Life", cppAccessor: "MaxLifeInput", defaultValue: "0.75f", dataType: "float" },
      { key: "RelativeMinInput", label: "Relative Min", cppAccessor: "RelativeMinInput", defaultValue: "true" },
      { key: "RelativeMaxInput", label: "Relative Max", cppAccessor: "RelativeMaxInput", defaultValue: "true" },
      { key: "ClampUpperInput", label: "Clamp Upper", cppAccessor: "ClampUpperInput", defaultValue: "true" }
    ]
  },
  {
    typeId: "ParticleReposition",
    label: "Reposition Sim",
    description: "Animates particles toward stored targets with idle/active fields.",
    cppType: "wp::ParticleReposition",
    typeGroup: "Simulator",
    include: "weave/particles/core/nodes/ParticleReposition.h",
    varPrefix: "reposition",
    inputs: [
      { key: "DelayedInit", label: "Delayed Init", cppAccessor: "DelayedInit", defaultValue: "0.0f", dataType: "float" },
      { key: "IdleDistance", label: "Idle Distance", cppAccessor: "IdleDistance", defaultValue: "0.1f", dataType: "float" },
      { key: "ForceFieldDistance", label: "Force Field Distance", cppAccessor: "ForceFieldDistance", defaultValue: "1.5f", dataType: "float" },
      { key: "VelocityFieldDistance", label: "Velocity Field Distance", cppAccessor: "VelocityFieldDistance", defaultValue: "0.5f", dataType: "float" },
      { key: "ForceMagnitude", label: "Force Magnitude", cppAccessor: "ForceMagnitude", defaultValue: "1.0f", dataType: "float" },
      { key: "IdleToActiveTime", label: "Idle → Active Time", cppAccessor: "IdleToActiveTime", defaultValue: "2.0f", dataType: "float" },
      { key: "ActiveToIdleTime", label: "Active → Idle Time", cppAccessor: "ActiveToIdleTime", defaultValue: "0.2f", dataType: "float" },
      { key: "Damping", label: "Velocity Damping", cppAccessor: "Damping", defaultValue: "0.8f", dataType: "float" }
    ]
  },
  {
    typeId: "ParticleTurbulenceFieldGenerator",
    label: "Turbulence Fields",
    description: "Generates a list of turbulence directions and magnitudes.",
    cppType: "wb::TurbulenceFieldGeneratorNode",
    typeGroup: "Initializer",
    include: "weave/system/blender/nodes/TurbulenceFieldGeneratorNode.h",
    varPrefix: "turbulenceFields",
    inputs: [
      { key: "MinForceInput", label: "Min Force", cppAccessor: "MinForceInput", defaultValue: "0.0f", dataType: "float" },
      { key: "MaxForceInput", label: "Max Force", cppAccessor: "MaxForceInput", defaultValue: "1.0f", dataType: "float" },
      { key: "FrequencyInput", label: "Update Frequency (s)", cppAccessor: "FrequencyInput", defaultValue: "0.5f", dataType: "float" },
      { key: "VariationInput", label: "Variation Blend", cppAccessor: "VariationInput", defaultValue: "0.5f", dataType: "float" },
      { key: "FieldCountInput", label: "Field Count", cppAccessor: "FieldCountInput", defaultValue: "1u", dataType: "float" }
    ],
    outputs: [
      { key: "Fields", label: "Field List", cppAccessor: "FieldsOutput", accessorType: "enum", dataType: "turbulence", hint: "Generated turbulence field list" }
    ]
  },
  {
    typeId: "ParticleTurbulenceSim",
    label: "Turbulence Sim",
    description: "Applies turbulence fields within a radius with decay.",
    cppType: "wp::ParticleTurbulenceSim",
    typeGroup: "Simulator",
    include: "weave/particles/core/nodes/ParticleTurbulenceSim.h",
    varPrefix: "turbulence",
    inputs: [
      { key: "TransformInput", label: "Transform", cppAccessor: "TransformInput", defaultValue: "Transform{}", dataType: "transform" },
      { key: "FieldListInput", label: "Field List", cppAccessor: "FieldListInput", defaultValue: "wb::TurbulenceFieldList{}", dataType: "turbulence" },
      { key: "RadiusInput", label: "Radius", cppAccessor: "RadiusInput", defaultValue: "-1.0f", dataType: "float" },
      { key: "DecayInput", label: "Decay", cppAccessor: "DecayInput", defaultValue: "1.0f", dataType: "float" }
    ]
  }
];

const PORT_HINTS = {
  ParticleEmitter: {
    inputs: {
      MinEmit: "Minimum particles emitted per burst",
      MaxEmit: "Maximum particles emitted per burst",
      Rate: "Scalar multiplier applied to emission counts",
      MinFrequency: "Shortest delay between emission windows (seconds)",
      MaxFrequency: "Longest delay between emission windows (seconds)",
      MaxRuntime: "Optional lifetime cap for the emitter (seconds)",
      MaxParticles: "Hard cap for particles managed by this emitter",
      ResetSignal: "Optional external reset trigger value"
    }
  },
  ParticleBoxInit: {
    inputs: {
      MinX: "Minimum X extent of the spawn box",
      MinY: "Minimum Y extent of the spawn box",
      MinZ: "Minimum Z extent of the spawn box",
      MaxX: "Maximum X extent of the spawn box",
      MaxY: "Maximum Y extent of the spawn box",
      MaxZ: "Maximum Z extent of the spawn box"
    }
  },
  ParticleSphereInit: {
    inputs: {
      MinRadius: "Minimum spawn radius from the center",
      MaxRadius: "Maximum spawn radius from the center",
      Alpha: "Longitudinal angle span",
      Beta: "Latitudinal angle span"
    }
  },
  ParticleVelocityInit: {
    inputs: {
      Direction: "Central axis of the spawn cone",
      UpAngle: "Vertical spread angle",
      SideAngle: "Horizontal spread angle",
      MinVelocity: "Minimum launch speed",
      MaxVelocity: "Maximum launch speed"
    }
  },
  ParticlePhysicsInit: {
    inputs: {
      MinAgeInput: "Minimum random lifetime assigned to particles",
      MaxAgeInput: "Maximum random lifetime assigned to particles",
      MinMassInput: "Minimum random mass",
      MaxMassInput: "Maximum random mass"
    }
  },
  ParticleTransformInit: {
    inputs: {
      TransformInput: "Transform applied to particles during init",
      ApplyEmission: "Whether to affect the emission buffer",
      ApplyEditable: "Whether to affect the editable buffer",
      TranslatePosition: "Translates particle positions by the transform",
      RotatePosition: "Rotates particle positions by the transform",
      ScalePosition: "Scales particle positions by the transform",
      RotateVelocity: "Rotates particle velocities",
      ScaleVelocity: "Scales particle velocities"
    }
  },
  ParticleRngVelocityInit: {
    inputs: {
      MinVelocity: "Minimum randomized launch speed",
      MaxVelocity: "Maximum randomized launch speed",
      LimitX: "Cosine clamp on X direction (-1..1)",
      LimitY: "Cosine clamp on Y direction (-1..1)",
      LimitZ: "Cosine clamp on Z direction (-1..1)",
      Radial: "Toggles outward velocity from particle position"
    }
  },
  ParticleAgingSim: {
    inputs: {
      TransformInput: "Reference transform used for radius tests",
      RadiusInput: "Optional radius in which to age particles faster",
      AgingMultiplierInput: "Scalar applied to the lifetime decay"
    }
  },
  ParticlePhysicsSim: {
    inputs: {
      LinearDamping: "Scalar applied to velocities each frame",
      Gravity: "World gravity vector applied to all particles"
    }
  },
  ParticleAttractorSim: {
    inputs: {
      AttractorTransform: "World transform the particles move toward",
      ForceMagnitude: "Base force applied toward the attractor",
      GravityScale: "Additional gravity-like pull strength",
      Radius: "Distance where the attractor stops influencing",
      Decay: "Falloff applied as particles move away",
      Direction: "Optional direction override instead of transform"
    }
  },
  ParticleDragSim: {
    inputs: {
      LinearDrag: "Constant damping applied each frame",
      ExponentialDrag: "Damping proportional to current velocity",
      MinVelocityCap: "Lower clamp for particle speed after drag",
      MaxVelocityCap: "Upper clamp for particle speed after drag",
      BreakVelocity: "Threshold where particles come to rest"
    }
  },
  ParticleLifeAdjustSim: {
    inputs: {
      TransformInput: "Reference transform for optional radius queries",
      RadiusInput: "Radius to limit lifetime adjustments",
      MinLifeInput: "Minimum lifetime to enforce",
      MaxLifeInput: "Maximum lifetime to enforce",
      RelativeMinInput: "Treat min life as a relative fraction of current life",
      RelativeMaxInput: "Treat max life as a relative fraction of current life",
      ClampUpperInput: "Whether to clamp lifetimes that exceed the max"
    }
  },
  ParticleReposition: {
    inputs: {
      DelayedInit: "Time before newly spawned particles start following targets",
      IdleDistance: "Radius that keeps particles in the idle region",
      ForceFieldDistance: "Radius of the positional force field",
      VelocityFieldDistance: "Radius of the velocity field that steers travel",
      ForceMagnitude: "Strength of the positional force toward the target",
      IdleToActiveTime: "Duration to ramp from idle to chasing",
      ActiveToIdleTime: "Duration to relax back to idle",
      Damping: "Velocity damping while homing"
    }
  },
  ParticleTurbulenceFieldGenerator: {
    inputs: {
      MinForceInput: "Minimum force per generated field",
      MaxForceInput: "Maximum force per generated field",
      FrequencyInput: "Time between regeneration (seconds)",
      VariationInput: "Blend between configured and random directions",
      FieldCountInput: "Number of fields to synthesize"
    },
    outputs: {
      Fields: "Generated turbulence field list"
    }
  },
  ParticleTurbulenceSim: {
    inputs: {
      TransformInput: "Emitter transform to evaluate fields in local space",
      FieldListInput: "List of turbulence fields to apply",
      RadiusInput: "Optional influence radius",
      DecayInput: "Falloff applied as particles leave the radius"
    }
  },
  SignalSampler: {
    inputs: {
      TimeScaleInput: "Playback speed multiplier",
      TimeOffsetInput: "Seconds to offset the playback window",
      MinValueInput: "Lower bound used for interpolation",
      MaxValueInput: "Upper bound used for interpolation",
      SignalLengthInput: "Duration of the easing segment (seconds)",
      SignalLoopsInput: "Loop count before clamping (0 = infinite)",
      MirrorInput: "Mirror factor (1 = off, 2 = mirrored)"
    },
    outputs: {
      Value: "Sampled easing value at the current local time"
    }
  }
};

function applyPortHints(nodes) {
  nodes.forEach(node => {
    const hints = PORT_HINTS[node.typeId];
    if (!hints) {
      return;
    }
    (node.inputs || []).forEach(input => {
      if (hints.inputs?.[input.key]) {
        input.hint = hints.inputs[input.key];
      }
    });
    (node.outputs || []).forEach(output => {
      if (hints.outputs?.[output.key]) {
        output.hint = hints.outputs[output.key];
      }
    });
  });
}

applyPortHints(PARTICLE_NODES);

function buildAnimationSamplerNodes() {
  return [
    {
      typeId: "SignalSampler",
      category: "Animation",
      typeGroup: "Samplers",
      label: "Signal Sampler",
      description: "Samples an easing curve over time using GraphTime.",
      cppType: "wb::data::SignalSampler",
      include: SIGNAL_SAMPLER_HEADER,
      varPrefix: "signalSampler",
      constructorArgs: [
        { key: "Curve", label: "Curve Function", defaultValue: "weave::easing::linear", placeholder: "weave::easing::expInOut" },
      ],
      inputs: [
        { key: "TimeScaleInput", label: "Time Scale", cppAccessor: "TimeScaleInput", accessorType: "enum", dataType: "float", defaultValue: "1.0f", hint: "Speed multiplier" },
        { key: "TimeOffsetInput", label: "Time Offset", cppAccessor: "TimeOffsetInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Phase offset (seconds)" },
        { key: "MinValueInput", label: "Min Value", cppAccessor: "MinValueInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Interpolation start" },
        { key: "MaxValueInput", label: "Max Value", cppAccessor: "MaxValueInput", accessorType: "enum", dataType: "float", defaultValue: "1.0f", hint: "Interpolation end" },
        { key: "SignalLengthInput", label: "Signal Length", cppAccessor: "SignalLengthInput", accessorType: "enum", dataType: "float", defaultValue: "1.0f", hint: "Total duration (seconds)" },
        { key: "SignalLoopsInput", label: "Signal Loops", cppAccessor: "SignalLoopsInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Loop count before clamping" },
        { key: "MirrorInput", label: "Mirror Factor", cppAccessor: "MirrorInput", accessorType: "enum", dataType: "float", defaultValue: "1.0f", hint: "1 = play once, 2 = mirror back" }
      ],
      outputs: [
        { key: "Value", label: "Value", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float", hint: "Sampled output value" }
      ]
    }
  ];
}

const ANIMATION_NODES = buildAnimationSamplerNodes();
applyPortHints(ANIMATION_NODES);

function buildConstNodes() {
  return TYPE_VARIANTS.map(type => ({
    typeId: `Const${type.suffix}`,
    category: "Math",
    typeGroup: type.typeGroup,
    label: `Const (${type.label})`,
    description: `Outputs a constant ${type.label.toLowerCase()} value.`,
    cppType: `wb::ConstNode<${type.typeName}>`,
    include: ARITHMETIC_HEADER,
    varPrefix: `const${type.suffix}`,
    constructorArgs: [
      { key: "Value0", label: "Value", defaultValue: type.zero }
    ],
    outputs: [
      { key: "Value", label: `${type.label} Value`, cppAccessor: "0", accessorType: "index", dataType: type.id }
    ]
  }));
}

const BINARY_MATH_CONFIGS = [
  {
    id: "Add",
    label: "Add",
    description: "Adds two values.",
    varPrefix: "add",
    resultLabel: "Sum",
    defaults: type => ({ A: type.zero, B: type.zero }),
    cppType: type => `wb::AddNode<${type.typeName}, ${type.typeName}>`
  },
  {
    id: "Subtract",
    label: "Subtract",
    description: "Subtracts B from A.",
    varPrefix: "sub",
    resultLabel: "Difference",
    defaults: type => ({ A: type.zero, B: type.zero }),
    cppType: type => `wb::SubtractNode<${type.typeName}, ${type.typeName}>`
  },
  {
    id: "Multiply",
    label: "Multiply",
    description: "Multiplies two values.",
    varPrefix: "mul",
    resultLabel: "Product",
    defaults: type => ({ A: type.one, B: type.one }),
    cppType: type => `wb::MultiplyNode<${type.typeName}, ${type.typeName}>`
  },
  {
    id: "Divide",
    label: "Divide",
    description: "Divides A by B.",
    varPrefix: "div",
    resultLabel: "Quotient",
    defaults: type => ({ A: type.one, B: type.one }),
    cppType: type => `wb::DivideNode<${type.typeName}, ${type.typeName}>`
  }
];

function buildBinaryMathNodes() {
  const nodes = [];
  BINARY_MATH_CONFIGS.forEach(config => {
    TYPE_VARIANTS.forEach(type => {
      const defaults = config.defaults(type);
      nodes.push({
        typeId: `${config.id}${type.suffix}`,
        category: "Math",
        typeGroup: type.typeGroup,
        label: `${config.label} (${type.label})`,
        description: config.description,
        cppType: config.cppType(type),
        include: ARITHMETIC_HEADER,
        varPrefix: `${config.varPrefix}${type.suffix}`,
        inputs: [
          { key: "A", label: config.inputLabels?.A ?? "A", cppAccessor: "0", defaultValue: defaults.A, accessorType: "index", dataType: type.id },
          { key: "B", label: config.inputLabels?.B ?? "B", cppAccessor: "1", defaultValue: defaults.B, accessorType: "index", dataType: type.id }
        ],
        outputs: [
          { key: "Result", label: config.resultLabel, cppAccessor: "ResultOutput", accessorType: "enum", dataType: type.id }
        ]
      });
    });
  });
  return nodes;
}

function buildScalarMathNodes() {
  return [
    {
      typeId: "ModuloFloat",
      category: "Math",
      typeGroup: "Float",
      label: "Modulo (float)",
      description: "Computes the remainder of A % B.",
      cppType: "wb::ModuloNode<float, float>",
      include: ARITHMETIC_HEADER,
      varPrefix: "mod",
      inputs: [
        { key: "A", label: "A", cppAccessor: "0", defaultValue: "0.0f", accessorType: "index", dataType: "float" },
        { key: "B", label: "B", cppAccessor: "1", defaultValue: "1.0f", accessorType: "index", dataType: "float" }
      ],
      outputs: [
        { key: "Result", label: "Remainder", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float" }
      ]
    },
    {
      typeId: "AbsFloat",
      category: "Math",
      typeGroup: "Float",
      label: "Absolute (float)",
      description: "Outputs the absolute value of the input.",
      cppType: "wb::AbsoluteNode<float>",
      include: ARITHMETIC_HEADER,
      varPrefix: "abs",
      inputs: [
        { key: "Value", label: "Value", cppAccessor: "ValueInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Operand to abs()" }
      ],
      outputs: [
        { key: "Result", label: "Absolute", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float" }
      ]
    },
    {
      typeId: "PowerFloat",
      category: "Math",
      typeGroup: "Float",
      label: "Power (float)",
      description: "Raises A to the B power.",
      cppType: "wb::PowerNode<float, float>",
      include: ARITHMETIC_HEADER,
      varPrefix: "pow",
      inputs: [
        { key: "Base", label: "Base", cppAccessor: "BaseInput", defaultValue: "1.0f", accessorType: "enum", dataType: "float", hint: "Value being raised to a power" },
        { key: "Exponent", label: "Exponent", cppAccessor: "ExponentInput", defaultValue: "2.0f", accessorType: "enum", dataType: "float", hint: "Exponent applied to the base" }
      ],
      outputs: [
        { key: "Result", label: "Value", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float" }
      ]
    },
    {
      typeId: "SqrtFloat",
      category: "Math",
      typeGroup: "Float",
      label: "Square Root (float)",
      description: "Outputs the square root of the input.",
      cppType: "wb::SquareRootNode<float>",
      include: ARITHMETIC_HEADER,
      varPrefix: "sqrt",
      inputs: [
        { key: "Value", label: "Value", cppAccessor: "ValueInput", defaultValue: "1.0f", accessorType: "enum", dataType: "float", hint: "Operand to square-root" }
      ],
      outputs: [
        { key: "Result", label: "Root", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float" }
      ]
    },
    {
      typeId: "ClampFloat",
      category: "Math",
      typeGroup: "Float",
      label: "Clamp (float)",
      description: "Clamps the value between min and max.",
      cppType: "wb::ClampNode<float>",
      include: ARITHMETIC_HEADER,
      varPrefix: "clamp",
      inputs: [
        { key: "Value", label: "Value", cppAccessor: "ValueInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Value to clamp" },
        { key: "Min", label: "Min", cppAccessor: "MinInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Lower bound" },
        { key: "Max", label: "Max", cppAccessor: "MaxInput", defaultValue: "1.0f", accessorType: "enum", dataType: "float", hint: "Upper bound" }
      ],
      outputs: [
        { key: "Result", label: "Clamped", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float" }
      ]
    },
    {
      typeId: "LerpFloat",
      category: "Math",
      typeGroup: "Float",
      label: "Lerp (float)",
      description: "Linearly interpolates between A and B using T.",
      cppType: "wb::LerpNode<float, float>",
      include: ARITHMETIC_HEADER,
      varPrefix: "lerp",
      inputs: [
        { key: "A", label: "A", cppAccessor: "AInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Starting value" },
        { key: "B", label: "B", cppAccessor: "BInput", defaultValue: "1.0f", accessorType: "enum", dataType: "float", hint: "Ending value" },
        { key: "T", label: "T", cppAccessor: "WeightInput", defaultValue: "0.5f", accessorType: "enum", dataType: "float", hint: "Interpolation factor (0..1)" }
      ],
      outputs: [
        { key: "Result", label: "Value", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float" }
      ]
    },
    {
      typeId: "SmoothstepFloat",
      category: "Math",
      typeGroup: "Float",
      label: "Smoothstep (float)",
      description: "Smoothly interpolates between A and B using T.",
      cppType: "wb::SmoothstepNode<float>",
      include: ARITHMETIC_HEADER,
      varPrefix: "smooth",
      inputs: [
        { key: "A", label: "A", cppAccessor: "AInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Starting value" },
        { key: "B", label: "B", cppAccessor: "BInput", defaultValue: "1.0f", accessorType: "enum", dataType: "float", hint: "Ending value" },
        { key: "T", label: "T", cppAccessor: "WeightInput", defaultValue: "0.5f", accessorType: "enum", dataType: "float", hint: "Interpolation factor before smoothstep" }
      ],
      outputs: [
        { key: "Result", label: "Value", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float" }
      ]
    },
    {
      typeId: "MinMaxFloat",
      category: "Math",
      typeGroup: "Float",
      label: "Min/Max (float)",
      description: "Outputs the min and max across three samples.",
      cppType: "wb::MinMaxNode<float, float, float>",
      include: ARITHMETIC_HEADER,
      varPrefix: "minmax",
      inputs: [
        { key: "A", label: "A", cppAccessor: "0", defaultValue: "0.0f", accessorType: "index", dataType: "float" },
        { key: "B", label: "B", cppAccessor: "1", defaultValue: "0.5f", accessorType: "index", dataType: "float" },
        { key: "C", label: "C", cppAccessor: "2", defaultValue: "1.0f", accessorType: "index", dataType: "float" }
      ],
      outputs: [
        { key: "Min", label: "Min", cppAccessor: "MinOutput", accessorType: "enum", dataType: "float", hint: "Minimum value across all inputs" },
        { key: "Max", label: "Max", cppAccessor: "MaxOutput", accessorType: "enum", dataType: "float", hint: "Maximum value across all inputs" }
      ]
    }
  ];
}

function buildRandomRangeNodes() {
  return TYPE_VARIANTS.map(type => {
    const suffix = type.id === "float" ? "Float" : type.label;
    const varPrefix = type.id === "float" ? "randFloat" : `randVec${type.label.slice(-1)}`;
    return {
      typeId: `RandomRange${suffix}`,
      category: "Random",
      typeGroup: type.typeGroup,
      label: `Random Range (${type.label})`,
      description: `Outputs a random ${type.label.toLowerCase()} between Min and Max.`,
      cppType: `wb::RandomRangeNode<${type.typeName}>`,
      include: RNG_HEADER,
      varPrefix,
      inputs: [
        { key: "Min", label: "Min", cppAccessor: "MinInput", defaultValue: type.zero, accessorType: "enum", dataType: type.id, hint: "Lower bound for the random pick" },
        { key: "Max", label: "Max", cppAccessor: "MaxInput", defaultValue: type.one, accessorType: "enum", dataType: type.id, hint: "Upper bound for the random pick" }
      ],
      outputs: [
        { key: "Value", label: "Random Value", cppAccessor: "ResultOutput", accessorType: "enum", dataType: type.id, hint: "Random value between Min and Max" }
      ]
    };
  });
}

function formatPoolValues(rawValue, setter) {
  const trimmed = (rawValue || "").trim();
  if (!trimmed.length) {
    return null;
  }
  let contents = trimmed;
  if (contents.startsWith("{") && contents.endsWith("}")) {
    contents = contents.slice(1, -1).trim();
  }
  return `std::vector<${setter.valueType}>{ ${contents} }`;
}

function buildRandomPoolNodes() {
  return TYPE_VARIANTS.map(type => {
    const suffix = type.id === "float" ? "Float" : type.label;
    return {
      typeId: `RandomPool${suffix}`,
      category: "Random",
      typeGroup: type.typeGroup,
      label: `Random Pool (${type.label})`,
      description: `Chooses a random ${type.label.toLowerCase()} from a pool.`,
      cppType: `wb::RandomPoolNode<${type.typeName}>`,
      include: RNG_HEADER,
      varPrefix: `randPool${suffix}`,
      outputs: [
        { key: "Value", label: "Random Value", cppAccessor: "ValueOutput", accessorType: "enum", dataType: type.id, hint: "Random value selected from the pool" }
      ],
      customSetters: [
        {
          key: "poolValues",
          label: "Pool Values",
          defaultValue: "",
          method: "SetPool",
          inputType: "textarea",
          helperText: "Comma-separated list, e.g. 0.2f, 0.5f, 0.9f",
          valueType: type.typeName,
          formatValue: (value, _def, setter) => formatPoolValues(value, setter)
        }
      ]
    };
  });
}

const NOISE_TEMPLATES = [
  {
    id: "PerlinNoise",
    label: "Perlin Noise",
    description: "Samples Perlin noise.",
    varPrefix: "perlin",
    alias: {
      vec2: "wb::PerlinNoise2DNode",
      vec3: "wb::PerlinNoise3DNode",
      vec4: "wb::PerlinNoise4DNode"
    },
    inputsFactory: type => createNoiseInputs(type)
  },
  {
    id: "SimplexNoise",
    label: "Simplex Noise",
    description: "Samples Simplex noise.",
    varPrefix: "simplex",
    alias: {
      vec2: "wb::SimplexNoise2DNode",
      vec3: "wb::SimplexNoise3DNode",
      vec4: "wb::SimplexNoise4DNode"
    },
    inputsFactory: type => createNoiseInputs(type)
  },
  {
    id: "TurbulenceNoise",
    label: "Turbulence Noise",
    description: "Generates fractal turbulence noise.",
    varPrefix: "turbNoise",
    alias: {
      vec2: "wb::TurbulenceNoise2DNode",
      vec3: "wb::TurbulenceNoise3DNode",
      vec4: "wb::TurbulenceNoise4DNode"
    },
    inputsFactory: type => createTurbulenceInputs(type)
  },
  {
    id: "SimplexLoopNoise",
    label: "Simplex Loop Noise",
    description: "Generates looping Simplex noise.",
    varPrefix: "simplexLoop",
    alias: {
      vec2: "wb::SimplexLoopNoise2DNode",
      vec3: "wb::SimplexLoopNoise3DNode",
      vec4: "wb::SimplexLoopNoise4DNode"
    },
    inputsFactory: type => createLoopNoiseInputs(type)
  },
  {
    id: "PerlinLoopNoise",
    label: "Perlin Loop Noise",
    description: "Generates looping Perlin noise.",
    varPrefix: "perlinLoop",
    alias: {
      vec2: "wb::PerlinLoopNoise2DNode",
      vec3: "wb::PerlinLoopNoise3DNode"
    },
    inputsFactory: type => createLoopNoiseInputs(type)
  },
  {
    id: "CircularNoise",
    label: "Circular Noise",
    description: "Generates looping circular noise.",
    varPrefix: "circularNoise",
    alias: {
      vec2: "wb::CircularLoopNoiseNode"
    },
    inputsFactory: type => createLoopNoiseInputs(type)
  }
];

function buildNoiseNodes() {
  const nodes = [];
  NOISE_TEMPLATES.forEach(template => {
    Object.entries(template.alias).forEach(([typeId, cppType]) => {
      const type = TYPE_VARIANT_MAP[typeId];
      if (!type) {
        return;
      }
      nodes.push({
        typeId: `${template.id}${type.suffix}`,
        category: "Noise",
        typeGroup: type.typeGroup,
        label: `${template.label} (${type.label})`,
        description: template.description,
        cppType,
        include: NOISE_HEADER,
        varPrefix: template.varPrefix,
        inputs: template.inputsFactory(type),
        outputs: [
          { key: "Noise", label: "Vector", cppAccessor: "NoiseOutput", accessorType: "enum", dataType: type.id, hint: "Sampled noise vector" }
        ]
      });
    });
  });
  return nodes;
}

function createNoiseInputs(type) {
  return [
    { key: "PositionInput", label: "Position", cppAccessor: "PositionInput", defaultValue: type.zero, accessorType: "enum", dataType: type.id, hint: "Coordinates being sampled" },
    { key: "OffsetInput", label: "Offset", cppAccessor: "OffsetInput", defaultValue: type.zero, accessorType: "enum", dataType: type.id, hint: "Additive offset applied to the sample" },
    { key: "FrequencyInput", label: "Frequency", cppAccessor: "FrequencyInput", defaultValue: type.one, accessorType: "enum", dataType: type.id, hint: "Per-axis frequency multipliers" },
    { key: "SeedInput", label: "Seed", cppAccessor: "SeedInput", defaultValue: type.zero, accessorType: "enum", dataType: type.id, hint: "Per-axis random seed offsets" }
  ];
}

function createTurbulenceInputs(type) {
  return [
    ...createNoiseInputs(type),
    { key: "OctavesInput", label: "Octaves", cppAccessor: "OctavesInput", defaultValue: "4", accessorType: "enum", dataType: "float", hint: "Number of noise layers to accumulate" },
    { key: "PersistenceInput", label: "Persistence", cppAccessor: "PersistenceInput", defaultValue: "0.5f", accessorType: "enum", dataType: "float", hint: "Amplitude falloff between octaves" },
    { key: "LacunarityInput", label: "Lacunarity", cppAccessor: "LacunarityInput", defaultValue: "2.0f", accessorType: "enum", dataType: "float", hint: "Frequency multiplier between octaves" }
  ];
}

function createLoopNoiseInputs(type) {
  return [
    ...createNoiseInputs(type),
    { key: "LoopFrequencyInput", label: "Loop Frequency", cppAccessor: "LoopFrequencyInput", defaultValue: "1.0f", accessorType: "enum", dataType: "float", hint: "How quickly the loop repeats in time" }
  ];
}

function stripEasingPrefix(name) {
  if (name.startsWith("ri")) {
    return { prefix: "Reverse Inverse", base: name.slice(2) };
  }
  if (name.startsWith("r")) {
    return { prefix: "Reverse", base: name.slice(1) };
  }
  if (name.startsWith("i")) {
    return { prefix: "Inverse", base: name.slice(1) };
  }
  return { prefix: "", base: name };
}

function camelToWords(value) {
  if (!value) {
    return "";
  }
  return value
    .replace(/([a-z])([A-Z])/g, "$1 $2")
    .replace(/^./, c => c.toUpperCase());
}

function easingLabelFromName(name) {
  const { prefix, base } = stripEasingPrefix(name);
  const baseLabel = camelToWords(base || name);
  return prefix ? `${prefix} ${baseLabel}` : baseLabel;
}

function easingGroupFromName(name) {
  const { base } = stripEasingPrefix(name);
  const trimmed = base.replace(/(InOut|In|Out)$/i, "") || base;
  const label = camelToWords(trimmed);
  return label || "General";
}

function easingPascalFromName(name) {
  return easingLabelFromName(name).replace(/\s+/g, "");
}

const EASING_FUNCTIONS = [
  "flat",
  "linear",
  "step",
  "sin",
  "cos",
  "quadraticIn",
  "quadraticOut",
  "quadraticInOut",
  "cubicIn",
  "cubicOut",
  "cubicInOut",
  "cubicElasticIn",
  "cubicElasticOut",
  "quarticIn",
  "quarticOut",
  "quarticInOut",
  "quarticElasticIn",
  "quarticElasticOut",
  "quinticIn",
  "quinticOut",
  "quinticInOut",
  "sinusIn",
  "sinusOut",
  "sinusInOut",
  "expIn",
  "expOut",
  "expInOut",
  "circIn",
  "circOut",
  "circInOut",
  "elasticSoftOut",
  "elasticSoftIn",
  "elasticHardOut",
  "elasticHardIn",
  "bounceOut",
  "bounceIn",
  "bounceInOut",
  "ilinear",
  "istep",
  "isin",
  "icos",
  "iquadraticIn",
  "iquadraticOut",
  "iquadraticInOut",
  "icubicIn",
  "icubicOut",
  "icubicInOut",
  "icubicElasticIn",
  "icubicElasticOut",
  "iquarticIn",
  "iquarticOut",
  "iquarticInOut",
  "iquarticElasticIn",
  "iquarticElasticOut",
  "iquinticIn",
  "iquinticOut",
  "iquinticInOut",
  "isinusIn",
  "isinusOut",
  "isinusInOut",
  "iexpIn",
  "iexpOut",
  "iexpInOut",
  "icircIn",
  "icircOut",
  "icircInOut",
  "ielasticSoftOut",
  "ielasticSoftIn",
  "ielasticHardOut",
  "ielasticHardIn",
  "ibounceOut",
  "ibounceIn",
  "ibounceInOut",
  "rlinear",
  "rstep",
  "rsin",
  "rcos",
  "rquadraticIn",
  "rquadraticOut",
  "rquadraticInOut",
  "rcubicIn",
  "rcubicOut",
  "rcubicInOut",
  "rcubicElasticIn",
  "rcubicElasticOut",
  "rquarticIn",
  "rquarticOut",
  "rquarticInOut",
  "rquarticElasticIn",
  "rquarticElasticOut",
  "rquinticIn",
  "rquinticOut",
  "rquinticInOut",
  "rsinusIn",
  "rsinusOut",
  "rsinusInOut",
  "rexpIn",
  "rexpOut",
  "rexpInOut",
  "rcircIn",
  "rcircOut",
  "rcircInOut",
  "relasticSoftOut",
  "relasticSoftIn",
  "relasticHardOut",
  "relasticHardIn",
  "rbounceOut",
  "rbounceIn",
  "rbounceInOut",
  "rilinear",
  "ristep",
  "risin",
  "ricos",
  "riquadraticIn",
  "riquadraticOut",
  "riquadraticInOut",
  "ricubicIn",
  "ricubicOut",
  "ricubicInOut",
  "ricubicElasticIn",
  "ricubicElasticOut",
  "riquarticIn",
  "riquarticOut",
  "riquarticInOut",
  "riquarticElasticIn",
  "riquarticElasticOut",
  "riquinticIn",
  "riquinticOut",
  "riquinticInOut",
  "risinusIn",
  "risinusOut",
  "risinusInOut",
  "riexpIn",
  "riexpOut",
  "riexpInOut",
  "ricircIn",
  "ricircOut",
  "ricircInOut",
  "rielasticSoftOut",
  "rielasticSoftIn",
  "rielasticHardOut",
  "rielasticHardIn",
  "ribounceOut",
  "ribounceIn",
  "ribounceInOut"
];


function buildConversionNodes() {
  const nodes = [
    {
      typeId: "ConvertFloatToUInt",
      category: "Conversion",
      typeGroup: "Scalar",
      label: "Float → UInt",
      description: "Casts a float value to an unsigned integer.",
      cppType: "wb::ConvertNode<float, uint32_t>",
      include: CONVERSION_HEADER,
      varPrefix: "f2u",
      inputs: [
        { key: "Value", label: "Float", cppAccessor: "ValueInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Float source value" }
      ],
      outputs: [
        { key: "Result", label: "UInt", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "uint", hint: "Converted unsigned value" }
      ]
    },
    {
      typeId: "ConvertFloatToInt",
      category: "Conversion",
      typeGroup: "Scalar",
      label: "Float → Int",
      description: "Casts a float value to a signed integer.",
      cppType: "wb::ConvertNode<float, int32_t>",
      include: CONVERSION_HEADER,
      varPrefix: "f2i",
      inputs: [
        { key: "Value", label: "Float", cppAccessor: "ValueInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Float source value" }
      ],
      outputs: [
        { key: "Result", label: "Int", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "int", hint: "Converted signed value" }
      ]
    },
    {
      typeId: "ConvertIntToFloat",
      category: "Conversion",
      typeGroup: "Scalar",
      label: "Int → Float",
      description: "Casts a signed integer to float.",
      cppType: "wb::ConvertNode<int32_t, float>",
      include: CONVERSION_HEADER,
      varPrefix: "i2f",
      inputs: [
        { key: "Value", label: "Int", cppAccessor: "ValueInput", defaultValue: "0", accessorType: "enum", dataType: "int", hint: "Signed integer source value" }
      ],
      outputs: [
        { key: "Result", label: "Float", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float", hint: "Converted float value" }
      ]
    },
    {
      typeId: "ConvertUIntToFloat",
      category: "Conversion",
      typeGroup: "Scalar",
      label: "UInt → Float",
      description: "Casts an unsigned integer to float.",
      cppType: "wb::ConvertNode<uint32_t, float>",
      include: CONVERSION_HEADER,
      varPrefix: "u2f",
      inputs: [
        { key: "Value", label: "UInt", cppAccessor: "ValueInput", defaultValue: "0u", accessorType: "enum", dataType: "uint", hint: "Unsigned integer source value" }
      ],
      outputs: [
        { key: "Result", label: "Float", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float", hint: "Converted float value" }
      ]
    },
    {
      typeId: "ConvertFloatToVec2",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Float → Vec2",
      description: "Broadcasts a float into all Vector2 components.",
      cppType: "wb::ConvertNode<float, Vector2>",
      include: CONVERSION_HEADER,
      varPrefix: "f2v2",
      inputs: [
        { key: "Value", label: "Float", cppAccessor: "ValueInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Float source value" }
      ],
      outputs: [
        { key: "Result", label: "Vector2", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "vec2", hint: "Converted Vector2 value" }
      ]
    },
    {
      typeId: "ConvertFloatToVec3",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Float → Vec3",
      description: "Broadcasts a float into all Vector3 components.",
      cppType: "wb::ConvertNode<float, Vector3>",
      include: CONVERSION_HEADER,
      varPrefix: "f2v3",
      inputs: [
        { key: "Value", label: "Float", cppAccessor: "ValueInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Float source value" }
      ],
      outputs: [
        { key: "Result", label: "Vector3", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "vec3", hint: "Converted Vector3 value" }
      ]
    },
    {
      typeId: "ConvertFloatToVec4",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Float → Vec4",
      description: "Broadcasts a float into all Vector4 components.",
      cppType: "wb::ConvertNode<float, Vector4>",
      include: CONVERSION_HEADER,
      varPrefix: "f2v4",
      inputs: [
        { key: "Value", label: "Float", cppAccessor: "ValueInput", defaultValue: "0.0f", accessorType: "enum", dataType: "float", hint: "Float source value" }
      ],
      outputs: [
        { key: "Result", label: "Vector4", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "vec4", hint: "Converted Vector4 value" }
      ]
    }
  ];

  nodes.push(
    {
      typeId: "ComposeVec2",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Compose Vec2",
      description: "Builds a Vector2 from two floats.",
      cppType: "wb::ComposeVector2Node",
      include: CONVERSION_HEADER,
      varPrefix: "composeV2",
      inputs: [
        { key: "X", label: "X", cppAccessor: "XInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "X component" },
        { key: "Y", label: "Y", cppAccessor: "YInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Y component" }
      ],
      outputs: [
        { key: "Vector", label: "Vector2", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "vec2", hint: "Composed Vector2" }
      ]
    },
    {
      typeId: "ComposeVec3",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Compose Vec3",
      description: "Builds a Vector3 from three floats.",
      cppType: "wb::ComposeVector3Node",
      include: CONVERSION_HEADER,
      varPrefix: "composeV3",
      inputs: [
        { key: "X", label: "X", cppAccessor: "XInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "X component" },
        { key: "Y", label: "Y", cppAccessor: "YInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Y component" },
        { key: "Z", label: "Z", cppAccessor: "ZInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Z component" }
      ],
      outputs: [
        { key: "Vector", label: "Vector3", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "vec3", hint: "Composed Vector3" }
      ]
    },
    {
      typeId: "ComposeVec4",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Compose Vec4",
      description: "Builds a Vector4 from four floats.",
      cppType: "wb::ComposeVector4Node",
      include: CONVERSION_HEADER,
      varPrefix: "composeV4",
      inputs: [
        { key: "X", label: "X", cppAccessor: "XInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "X component" },
        { key: "Y", label: "Y", cppAccessor: "YInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Y component" },
        { key: "Z", label: "Z", cppAccessor: "ZInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Z component" },
        { key: "W", label: "W", cppAccessor: "WInput", accessorType: "enum", dataType: "float", defaultValue: "1.0f", hint: "W component" }
      ],
      outputs: [
        { key: "Vector", label: "Vector4", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "vec4", hint: "Composed Vector4" }
      ]
    },
    {
      typeId: "DecomposeVec2",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Decompose Vec2",
      description: "Splits a Vector2 into its components.",
      cppType: "wb::DecomposeVector2Node",
      include: CONVERSION_HEADER,
      varPrefix: "decomposeV2",
      inputs: [
        { key: "Vector", label: "Vector2", cppAccessor: "ValueInput", accessorType: "enum", dataType: "vec2", defaultValue: "Vector2{}", hint: "Vector2 value to split" }
      ],
      outputs: [
        { key: "X", label: "X", cppAccessor: "XOutput", accessorType: "enum", dataType: "float", hint: "X component" },
        { key: "Y", label: "Y", cppAccessor: "YOutput", accessorType: "enum", dataType: "float", hint: "Y component" }
      ]
    },
    {
      typeId: "DecomposeVec3",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Decompose Vec3",
      description: "Splits a Vector3 into its components.",
      cppType: "wb::DecomposeVector3Node",
      include: CONVERSION_HEADER,
      varPrefix: "decomposeV3",
      inputs: [
        { key: "Vector", label: "Vector3", cppAccessor: "ValueInput", accessorType: "enum", dataType: "vec3", defaultValue: "Vector3{}", hint: "Vector3 value to split" }
      ],
      outputs: [
        { key: "X", label: "X", cppAccessor: "XOutput", accessorType: "enum", dataType: "float", hint: "X component" },
        { key: "Y", label: "Y", cppAccessor: "YOutput", accessorType: "enum", dataType: "float", hint: "Y component" },
        { key: "Z", label: "Z", cppAccessor: "ZOutput", accessorType: "enum", dataType: "float", hint: "Z component" }
      ]
    },
    {
      typeId: "DecomposeVec4",
      category: "Conversion",
      typeGroup: "Vector",
      label: "Decompose Vec4",
      description: "Splits a Vector4 into its components.",
      cppType: "wb::DecomposeVector4Node",
      include: CONVERSION_HEADER,
      varPrefix: "decomposeV4",
      inputs: [
        { key: "Vector", label: "Vector4", cppAccessor: "ValueInput", accessorType: "enum", dataType: "vec4", defaultValue: "Vector4{}", hint: "Vector4 value to split" }
      ],
      outputs: [
        { key: "X", label: "X", cppAccessor: "XOutput", accessorType: "enum", dataType: "float", hint: "X component" },
        { key: "Y", label: "Y", cppAccessor: "YOutput", accessorType: "enum", dataType: "float", hint: "Y component" },
        { key: "Z", label: "Z", cppAccessor: "ZOutput", accessorType: "enum", dataType: "float", hint: "Z component" },
        { key: "W", label: "W", cppAccessor: "WOutput", accessorType: "enum", dataType: "float", hint: "W component" }
      ]
    }
  );

  return nodes;
}

function buildEasingNodes() {
  const curveOptions = EASING_FUNCTIONS.map(name => ({
    label: easingLabelFromName(name),
    value: `weave::easing::${name}`,
    group: easingGroupFromName(name)
  }));
  return [
    {
      typeId: "EasingFunction",
      category: "Easing",
      typeGroup: "Curves",
      label: "Easing Function",
      description: "Evaluates a selected easing curve.",
      cppType: "wb::EasingFunctionNode",
      includes: [EASING_NODES_HEADER, EASING_HEADER],
      varPrefix: "easeFn",
      inputs: [
        { key: "T", label: "T", cppAccessor: "TimeInput", accessorType: "enum", dataType: "float", defaultValue: "0.0f", hint: "Normalized time input (0-1)" }
      ],
      outputs: [
        { key: "Value", label: "Value", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "float", hint: "Curve-adjusted output value" }
      ],
      customSetters: [
        {
          key: "curve",
          label: "Curve",
          method: "SetCurve",
          defaultValue: "weave::easing::linear",
          inputType: "select",
          options: curveOptions.map(opt => ({ label: opt.label, value: opt.value })),
          displayLabel: true,
          displayLabelPrefix: "Curve: "
        }
      ]
    }
  ];
}

function repeatTypeList(typeName, count) {
  return Array.from({ length: count }, () => typeName).join(", ");
}

const BOOLEAN_INPUT_VARIANTS = [2, 3, 4];

const BOOLEAN_LOGIC_CONFIGS = [
  {
    id: "BoolAnd",
    label: "AND",
    description: "Outputs true only when every input is true.",
    cppClass: "AndNode",
    varPrefix: "boolAnd",
    inputHint: "Boolean operand",
    outputHint: "True when every input is true"
  },
  {
    id: "BoolOr",
    label: "OR",
    description: "Outputs true when at least one input is true.",
    cppClass: "OrNode",
    varPrefix: "boolOr",
    inputHint: "Boolean operand",
    outputHint: "True when any input is true"
  },
  {
    id: "BoolXor",
    label: "XOR",
    description: "Outputs true when an odd number of inputs are true.",
    cppClass: "XorNode",
    varPrefix: "boolXor",
    inputHint: "Boolean operand",
    outputHint: "True when an odd number of inputs are true"
  }
];

function buildBooleanLogicNodes() {
  const nodes = [];
  BOOLEAN_LOGIC_CONFIGS.forEach(config => {
    BOOLEAN_INPUT_VARIANTS.forEach(count => {
      const templateArgs = repeatTypeList("bool", count);
      const inputs = Array.from({ length: count }, (_, idx) => ({
        key: `Value${idx + 1}`,
        label: `Value ${idx + 1}`,
        cppAccessor: `${idx}`,
        accessorType: "index",
        dataType: "bool",
        defaultValue: idx === 0 ? "true" : "false",
        hint: config.inputHint
      }));
      nodes.push({
        typeId: `${config.id}${count}`,
        category: "Conditional",
        typeGroup: "Bool",
        label: `${config.label} (${count} inputs)`,
        description: config.description,
        cppType: `wb::${config.cppClass}<${templateArgs}>`,
        include: CONDITIONAL_HEADER,
        varPrefix: `${config.varPrefix}${count}`,
        inputs,
        outputs: [
          { key: "Result", label: "Result", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "bool", hint: config.outputHint }
        ]
      });
    });
  });

  nodes.push({
    typeId: "BoolNot",
    category: "Conditional",
    typeGroup: "Bool",
    label: "NOT",
    description: "Outputs the inverse of the input.",
    cppType: "wb::NotNode",
    include: CONDITIONAL_HEADER,
    varPrefix: "boolNot",
    inputs: [
      { key: "Value", label: "Value", cppAccessor: "ValueInput", accessorType: "enum", dataType: "bool", defaultValue: "false", hint: "Boolean operand to invert" }
    ],
    outputs: [
      { key: "Result", label: "Result", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "bool", hint: "Logical NOT of the operand" }
    ]
  });

  return nodes;
}

const COMPARISON_CONFIGS = [
  {
    id: "LessThan",
    label: "Less Than",
    description: "Outputs true when A is less than B.",
    cppClass: "LessThanNode",
    varPrefix: "lt",
    resultHint: "True when A < B"
  },
  {
    id: "LessEqual",
    label: "Less Or Equal",
    description: "Outputs true when A is less than or equal to B.",
    cppClass: "LessEqualNode",
    varPrefix: "le",
    resultHint: "True when A ≤ B"
  },
  {
    id: "GreaterThan",
    label: "Greater Than",
    description: "Outputs true when A is greater than B.",
    cppClass: "GreaterThanNode",
    varPrefix: "gt",
    resultHint: "True when A > B"
  },
  {
    id: "GreaterEqual",
    label: "Greater Or Equal",
    description: "Outputs true when A is greater than or equal to B.",
    cppClass: "GreaterEqualNode",
    varPrefix: "ge",
    resultHint: "True when A ≥ B"
  },
  {
    id: "Equal",
    label: "Equal",
    description: "Outputs true when A equals B.",
    cppClass: "EqualNode",
    varPrefix: "eq",
    resultHint: "True when A == B"
  },
  {
    id: "NotEqual",
    label: "Not Equal",
    description: "Outputs true when A differs from B.",
    cppClass: "NotEqualNode",
    varPrefix: "neq",
    resultHint: "True when A != B"
  }
];

function buildComparisonNodes() {
  const nodes = [];
  COMPARISON_CONFIGS.forEach(config => {
    TYPE_VARIANTS.forEach(type => {
      nodes.push({
        typeId: `${config.id}${type.suffix}`,
        category: "Conditional",
        typeGroup: type.typeGroup,
        label: `${config.label} (${type.label})`,
        description: config.description,
        cppType: `wb::${config.cppClass}<${type.typeName}>`,
        include: CONDITIONAL_HEADER,
        varPrefix: `${config.varPrefix}${type.varSuffix}`,
        inputs: [
          { key: "A", label: "A", cppAccessor: "AInput", accessorType: "enum", dataType: type.id, defaultValue: type.zero, hint: "Left operand" },
          { key: "B", label: "B", cppAccessor: "BInput", accessorType: "enum", dataType: type.id, defaultValue: type.one, hint: "Right operand" }
        ],
        outputs: [
          { key: "Result", label: "Result", cppAccessor: "ResultOutput", accessorType: "enum", dataType: "bool", hint: config.resultHint }
        ]
      });
    });
  });
  return nodes;
}

const GENERAL_NODES = [
  ...buildConstNodes(),
  ...buildBinaryMathNodes(),
  ...buildScalarMathNodes(),
  ...buildRandomRangeNodes(),
  ...buildRandomPoolNodes(),
  ...buildNoiseNodes(),
  ...buildBooleanLogicNodes(),
  ...buildComparisonNodes(),
  ...buildConversionNodes(),
  ...buildEasingNodes()
];

applyPortHints(GENERAL_NODES);

const NODE_LIBRARY = [...PARTICLE_NODES, ...ANIMATION_NODES, ...GENERAL_NODES];

const CATEGORY_ORDER = ["Particles", "Animation", "Math", "Conditional", "Conversion", "Easing", "Noise", "Random"];

const NODE_LOOKUP = new Map();
const CATEGORY_MAP = new Map();

NODE_LIBRARY.forEach(def => {
  def.category = def.category || "Particles";
  def.inputs = (def.inputs || []).map(input => ({
    ...input,
    accessorType: input.accessorType || "enum",
    dataType: input.dataType || null
  }));
  def.outputs = (def.outputs || []).map(output => ({
    ...output,
    accessorType: output.accessorType || "index",
    dataType: output.dataType || null
  }));
  NODE_LOOKUP.set(def.typeId, def);
  if (!CATEGORY_MAP.has(def.category)) {
    CATEGORY_MAP.set(def.category, []);
  }
  CATEGORY_MAP.get(def.category).push(def);
});

function createCategoryEntry(id, nodes) {
  const groupsMap = new Map();
  nodes.forEach(node => {
    const groupId = node.typeGroup || "__default__";
    if (!groupsMap.has(groupId)) {
      groupsMap.set(groupId, {
        id: groupId,
        label: node.typeGroup || "",
        nodes: []
      });
    }
    groupsMap.get(groupId).nodes.push(node);
  });
  const groups = Array.from(groupsMap.values()).map(group => ({
    ...group,
    nodes: group.nodes.sort((a, b) => a.label.localeCompare(b.label))
  }));
  groups.sort((a, b) => {
    if (!a.label && b.label) {
      return -1;
    }
    if (a.label && !b.label) {
      return 1;
    }
    return a.label.localeCompare(b.label);
  });
  return { id, label: id, groups };
}

const NODE_CATEGORIES = [];
CATEGORY_ORDER.forEach(cat => {
  if (CATEGORY_MAP.has(cat)) {
    NODE_CATEGORIES.push(createCategoryEntry(cat, CATEGORY_MAP.get(cat)));
  }
});
CATEGORY_MAP.forEach((nodes, cat) => {
  if (!CATEGORY_ORDER.includes(cat)) {
    NODE_CATEGORIES.push(createCategoryEntry(cat, nodes));
  }
});

const state = {
  nodes: [],
  connections: [],
  dataConnections: [],
  selectedNodeId: null,
  typeCounters: {},
  nextNodeId: 1,
  paletteCollapse: {}
};

const paletteListEl = document.getElementById("paletteList");
const nodesLayerEl = document.getElementById("nodesLayer");
const connectionLayerEl = document.getElementById("connectionLayer");
const inspectorContentEl = document.getElementById("inspectorContent");
const codeOutputEl = document.getElementById("codeOutput");
const canvasEl = document.getElementById("graphCanvas");
const generateBtn = document.getElementById("generateCodeBtn");
const resetBtn = document.getElementById("resetGraphBtn");

let dragState = null;
let connectionDrag = null;
let ioDrag = null;
let connectionUpdateScheduled = false;

function createId() {
  if (typeof crypto !== "undefined" && typeof crypto.randomUUID === "function") {
    return crypto.randomUUID();
  }
  return `conn-${Math.random().toString(16).slice(2)}`;
}

function init() {
  renderPalette();
  canvasEl.addEventListener("click", () => selectNode(null));
  window.addEventListener("resize", scheduleConnectionUpdate);
  generateBtn.addEventListener("click", () => {
    codeOutputEl.value = generateCpp();
    codeOutputEl.focus();
    codeOutputEl.select();
  });
  resetBtn.addEventListener("click", () => {
    if (state.nodes.length === 0) {
      return;
    }
    if (confirm("Clear the current graph?")) {
      state.nodes = [];
      state.connections = [];
      state.dataConnections = [];
      state.selectedNodeId = null;
      state.typeCounters = {};
      state.nextNodeId = 1;
      renderAll();
      codeOutputEl.value = "";
    }
  });
  renderAll();
}

document.addEventListener("DOMContentLoaded", init);

function ensurePaletteCollapseState() {
  NODE_CATEGORIES.forEach(cat => {
    if (!(cat.id in state.paletteCollapse)) {
      state.paletteCollapse[cat.id] = true;
    }
    (cat.groups || []).forEach(group => {
      if (!group.label) {
        return;
      }
      const key = `${cat.id}::${group.id}`;
      if (!(key in state.paletteCollapse)) {
        state.paletteCollapse[key] = true;
      }
    });
  });
}

function renderPalette() {
  ensurePaletteCollapseState();
  paletteListEl.innerHTML = "";
  NODE_CATEGORIES.forEach(category => {
    const section = document.createElement("div");
    section.className = "palette-category";

    const headerBtn = document.createElement("button");
    headerBtn.type = "button";
    headerBtn.className = "category-header";
    const collapsed = !!state.paletteCollapse[category.id];
    const chevron = collapsed ? "▸" : "▾";
    headerBtn.innerHTML = `<span>${category.label}</span><span class="chevron">${chevron}</span>`;
    headerBtn.addEventListener("click", () => {
      state.paletteCollapse[category.id] = !state.paletteCollapse[category.id];
      renderPalette();
    });
    section.appendChild(headerBtn);

    if (!collapsed) {
      const groups = category.groups || [];
      if (groups.length === 1 && !groups[0].label) {
        section.appendChild(createPaletteNodeList(groups[0].nodes));
      } else {
        groups.forEach(group => {
          if (!group.nodes.length) {
            return;
          }
          if (!group.label) {
            section.appendChild(createPaletteNodeList(group.nodes));
            return;
          }
          const groupKey = `${category.id}::${group.id}`;
          const groupCollapsed = !!state.paletteCollapse[groupKey];
          const subgroup = document.createElement("div");
          subgroup.className = "palette-subcategory";
          const subHeader = document.createElement("button");
          subHeader.type = "button";
          subHeader.className = "subcategory-header";
          const subChevron = groupCollapsed ? "▸" : "▾";
          subHeader.innerHTML = `<span>${group.label}</span><span class="chevron">${subChevron}</span>`;
          subHeader.addEventListener("click", () => {
            state.paletteCollapse[groupKey] = !groupCollapsed;
            renderPalette();
          });
          subgroup.appendChild(subHeader);
          if (!groupCollapsed) {
            subgroup.appendChild(createPaletteNodeList(group.nodes));
          }
          section.appendChild(subgroup);
        });
      }
    }

    paletteListEl.appendChild(section);
  });
}

function createPaletteNodeList(nodes) {
  const list = document.createElement("div");
  list.className = "palette-group";
  nodes.forEach(def => {
    const item = document.createElement("div");
    item.className = "palette-item";

    const heading = document.createElement("h3");
    heading.textContent = def.label;
    item.appendChild(heading);

    const desc = document.createElement("p");
    desc.textContent = def.description;
    item.appendChild(desc);

    const addBtn = document.createElement("button");
    addBtn.textContent = "Add";
    addBtn.addEventListener("click", () => addNode(def.typeId));
    item.appendChild(addBtn);

    list.appendChild(item);
  });
  return list;
}

function addNode(typeId) {
  const def = getNodeDefinition(typeId);
  if (!def) {
    return;
  }
  const nodeId = `node-${state.nextNodeId++}`;
  const count = (state.typeCounters[typeId] ?? 0) + 1;
  state.typeCounters[typeId] = count;
  const label = `${def.label} ${count}`;
  const position = {
    x: 40 + (state.nodes.length * 32) % Math.max(canvasEl.clientWidth - 260, 60),
    y: 40 + (state.nodes.length * 24) % Math.max(canvasEl.clientHeight - 160, 60)
  };

  const inputs = {};
  def.inputs.forEach(input => {
    inputs[input.key] = input.defaultValue ?? "";
  });

  const options = {};
  (def.options ?? []).forEach(option => {
    options[option.key] = option.defaultValue ?? false;
  });

  const constructorArgs = {};
  (def.constructorArgs ?? []).forEach(arg => {
    constructorArgs[arg.key] = arg.defaultValue ?? "";
  });

  const customValues = {};
  (def.customSetters ?? []).forEach(setter => {
    customValues[setter.key] = setter.defaultValue ?? "";
  });

  const variableName = makeUniqueIdentifier(def.varPrefix || def.typeId.toLowerCase());

  state.nodes.push({
    id: nodeId,
    typeId,
    label,
    variableName,
    position,
    inputs,
    options,
    constructorArgs,
    customValues,
    isRoot: state.nodes.length === 0, // first node defaults to root
    createdAt: Date.now()
  });

  state.selectedNodeId = nodeId;
  renderAll();
}

function getNodeDefinition(typeId) {
  return NODE_LOOKUP.get(typeId);
}

function getNodeById(id) {
  return state.nodes.find(n => n.id === id);
}

function findInputConnection(nodeId, portKey) {
  return state.dataConnections.find(conn => conn.to === nodeId && conn.toPort === portKey);
}

function findDataConnectionById(id) {
  return state.dataConnections.find(conn => conn.id === id);
}

function getOutputConnections(nodeId) {
  return state.dataConnections.filter(conn => conn.from === nodeId);
}

function getInputConnections(nodeId) {
  return state.dataConnections.filter(conn => conn.to === nodeId);
}

function applyPortDataset(element, nodeId, portKey, role) {
  if (!element) {
    return;
  }
  element.dataset.nodeId = nodeId;
  element.dataset.portKey = portKey;
  element.dataset.portRole = role;
}

function createPortRowPointerHandler(nodeId, portKey, role) {
  return event => {
    if (event.target.closest(".port-knob")) {
      return;
    }
    startIoDrag(event, nodeId, portKey, role);
  };
}

function renderAll() {
  renderNodes();
  renderInspector();
  scheduleConnectionUpdate();
}

function renderNodes() {
  nodesLayerEl.innerHTML = "";
  state.nodes.forEach(node => {
    const def = getNodeDefinition(node.typeId);
    if (!def) {
      return;
    }
    const card = document.createElement("div");
    card.className = `node-card${node.id === state.selectedNodeId ? " selected" : ""}`;
    card.style.left = `${node.position.x}px`;
    card.style.top = `${node.position.y}px`;
    card.dataset.nodeId = node.id;

    const header = document.createElement("div");
    header.className = "node-header";
    const title = document.createElement("span");
    title.className = "node-title";
    title.textContent = node.label;
    header.appendChild(title);
    if (node.isRoot) {
      const badge = document.createElement("span");
      badge.className = "badge-root";
      badge.textContent = "Root";
      header.appendChild(badge);
    }
    const closeBtn = document.createElement("button");
    closeBtn.type = "button";
    closeBtn.className = "node-close-btn";
    closeBtn.innerHTML = "×";
    closeBtn.title = "Delete node";
    closeBtn.addEventListener("pointerdown", event => event.stopPropagation());
    closeBtn.addEventListener("click", event => {
      event.stopPropagation();
      deleteNode(node.id);
    });
    header.appendChild(closeBtn);

    const body = document.createElement("div");
    body.className = "node-body";
    body.textContent = def.description ?? "";

    const summaryTexts = [];
    (def.customSetters || []).forEach(setter => {
      if (!setter.displayLabel) {
        return;
      }
      const currentValue = node.customValues?.[setter.key] ?? setter.defaultValue ?? "";
      if (!currentValue) {
        return;
      }
      let labelText = currentValue;
      if (setter.options?.length) {
        const option = setter.options.find(opt => opt.value === currentValue);
        if (option?.label) {
          labelText = option.label;
        }
      }
      if (setter.displayLabelPrefix) {
        labelText = `${setter.displayLabelPrefix}${labelText}`;
      }
      summaryTexts.push(labelText);
    });

    card.appendChild(header);
    if (summaryTexts.length) {
      const subtitle = document.createElement("div");
      subtitle.className = "node-subtitle";
      subtitle.textContent = summaryTexts.join(" • ");
      card.appendChild(subtitle);
      subtitle.addEventListener("pointerdown", event => startDrag(event, node.id));
    }
    card.appendChild(body);

    const ports = document.createElement("div");
    ports.className = "node-ports";

    if (def.inputs?.length) {
      const inputCol = document.createElement("div");
      inputCol.className = "ports-column inputs";
      def.inputs.forEach(inputDef => {
        const row = document.createElement("div");
        row.className = "port-row input";
        const dataType = normalizeDataType(inputDef.dataType);
        if (dataType) {
          row.dataset.dataType = dataType;
        }
        applyPortDataset(row, node.id, inputDef.key, "input");
        const typeLabel = formatDataTypeLabel(dataType);
        const knob = document.createElement("button");
        knob.type = "button";
        knob.className = "port-knob input";
        knob.dataset.nodeId = node.id;
        knob.dataset.portKey = inputDef.key;
        knob.dataset.portRole = "input";
        knob.dataset.dataType = dataType;
        const tooltip = buildPortTooltip(inputDef.label, typeLabel, inputDef.hint);
        knob.title = tooltip;
        row.title = tooltip;
        knob.addEventListener("pointerdown", event => startIoDrag(event, node.id, inputDef.key, "input"));
        const connection = findInputConnection(node.id, inputDef.key);
        if (connection) {
          knob.classList.add("connected");
          row.classList.add("connected");
        }
        const label = document.createElement("span");
        label.className = "port-label";
        label.textContent = inputDef.label;
        label.title = tooltip;
        applyPortDataset(label, node.id, inputDef.key, "input");
        const handlePointerDown = createPortRowPointerHandler(node.id, inputDef.key, "input");
        row.addEventListener("pointerdown", handlePointerDown);
        label.addEventListener("pointerdown", handlePointerDown);
        row.append(knob, label);
        inputCol.appendChild(row);
      });
      ports.appendChild(inputCol);
    }

    const outputCol = document.createElement("div");
    outputCol.className = "ports-column outputs";
    (def.outputs || []).forEach(outputDef => {
      const row = document.createElement("div");
      row.className = "port-row output";
      const dataType = normalizeDataType(outputDef.dataType);
      if (dataType) {
        row.dataset.dataType = dataType;
      }
      applyPortDataset(row, node.id, outputDef.key, "output");
      const typeLabel = formatDataTypeLabel(dataType);
      const tooltip = buildPortTooltip(outputDef.label, typeLabel, outputDef.hint);
      const label = document.createElement("span");
      label.className = "port-label";
      label.textContent = outputDef.label;
      label.title = tooltip;
      applyPortDataset(label, node.id, outputDef.key, "output");
      const knob = document.createElement("button");
      knob.type = "button";
      knob.className = "port-knob output";
      knob.dataset.nodeId = node.id;
      knob.dataset.portKey = outputDef.key;
      knob.dataset.portRole = "output";
      knob.dataset.dataType = dataType;
      knob.title = tooltip;
      knob.addEventListener("pointerdown", event => startIoDrag(event, node.id, outputDef.key, "output"));
      row.title = tooltip;
      const handlePointerDown = createPortRowPointerHandler(node.id, outputDef.key, "output");
      row.addEventListener("pointerdown", handlePointerDown);
      label.addEventListener("pointerdown", handlePointerDown);
      const hasOutgoing = state.dataConnections.some(conn => conn.from === node.id && conn.fromPort === outputDef.key);
      if (hasOutgoing) {
        knob.classList.add("connected");
        row.classList.add("connected");
      }
      row.append(label, knob);
      outputCol.appendChild(row);
    });

    const triggerRow = document.createElement("div");
    triggerRow.className = "port-row output trigger-row";
    const triggerLabel = document.createElement("span");
    triggerLabel.className = "port-label";
    triggerLabel.textContent = "Trigger";
    const triggerKnob = document.createElement("button");
    triggerKnob.type = "button";
    triggerKnob.className = "port-knob output trigger";
    triggerKnob.dataset.nodeId = node.id;
    triggerKnob.dataset.portKey = TRIGGER_PORT_KEY;
    triggerKnob.dataset.portRole = "trigger";
    triggerKnob.textContent = "➔";
    triggerKnob.title = "Drag to connect trigger";
    triggerKnob.addEventListener("pointerdown", event => startConnectionDrag(event, node.id));
    triggerRow.append(triggerLabel, triggerKnob);
    outputCol.appendChild(triggerRow);

    ports.appendChild(outputCol);
    card.appendChild(ports);

    card.addEventListener("click", event => {
      event.stopPropagation();
      selectNode(node.id);
    });

    header.addEventListener("pointerdown", event => startDrag(event, node.id));
    body.addEventListener("pointerdown", event => startDrag(event, node.id));

    nodesLayerEl.appendChild(card);
  });

  if (connectionDrag?.hoverTargetId) {
    const hoveredCard = nodesLayerEl.querySelector(`[data-node-id="${connectionDrag.hoverTargetId}"]`);
    hoveredCard?.classList.add("drop-target");
  }
}

function selectNode(nodeId) {
  state.selectedNodeId = nodeId;
  renderNodes();
  renderInspector();
}

function startDrag(event, nodeId) {
  event.preventDefault();
  const node = getNodeById(nodeId);
  if (!node) {
    return;
  }
  const canvasRect = canvasEl.getBoundingClientRect();
  dragState = {
    nodeId,
    offsetX: event.clientX - canvasRect.left - node.position.x,
    offsetY: event.clientY - canvasRect.top - node.position.y
  };
  document.addEventListener("pointermove", onDragMove);
  document.addEventListener("pointerup", endDrag);
}

function onDragMove(event) {
  if (!dragState) {
    return;
  }
  const node = getNodeById(dragState.nodeId);
  if (!node) {
    return;
  }
  const canvasRect = canvasEl.getBoundingClientRect();
  const newX = event.clientX - canvasRect.left - dragState.offsetX;
  const newY = event.clientY - canvasRect.top - dragState.offsetY;
  node.position.x = clamp(newX, 0, Math.max(canvasRect.width - 220, 0));
  node.position.y = clamp(newY, 0, Math.max(canvasRect.height - 120, 0));

  const card = nodesLayerEl.querySelector(`[data-node-id="${node.id}"]`);
  if (card) {
    card.style.left = `${node.position.x}px`;
    card.style.top = `${node.position.y}px`;
  }
  scheduleConnectionUpdate();
}

function endDrag() {
  document.removeEventListener("pointermove", onDragMove);
  document.removeEventListener("pointerup", endDrag);
  dragState = null;
}

function renderInspector() {
  const node = getNodeById(state.selectedNodeId);
  if (!node) {
    inspectorContentEl.innerHTML = "<p>Select a node to edit its inputs, naming, and root status.</p>";
    return;
  }
  const def = getNodeDefinition(node.typeId);
  const fragment = document.createDocumentFragment();
  node.constructorArgs = node.constructorArgs || {};
  node.customValues = node.customValues || {};

  // General section
  const general = document.createElement("div");
  general.className = "inspector-section";
  const generalHeading = document.createElement("h3");
  generalHeading.textContent = "Node";
  general.appendChild(generalHeading);

  const nameRow = createFormRow("Display Name", node.label, value => {
    node.label = value;
    renderNodes();
  });
  general.appendChild(nameRow);

  const varRow = createFormRow("Variable Name", node.variableName, value => {
    node.variableName = sanitizeIdentifier(value) || node.variableName;
    renderNodes();
  });
  general.appendChild(varRow);

  const rootRow = document.createElement("div");
  rootRow.className = "checkbox-row";
  const rootInput = document.createElement("input");
  rootInput.type = "checkbox";
  rootInput.checked = node.isRoot;
  rootInput.addEventListener("change", () => {
    if (rootInput.checked) {
      state.nodes.forEach(entry => {
        entry.isRoot = entry.id === node.id;
      });
    } else {
      node.isRoot = false;
    }
    renderNodes();
  });
  const rootLabel = document.createElement("label");
  rootLabel.textContent = "Root Trigger";
  rootRow.append(rootInput, rootLabel);
  general.appendChild(rootRow);

  fragment.appendChild(general);

  if (def?.constructorArgs?.length) {
    const ctorSection = document.createElement("div");
    ctorSection.className = "inspector-section";
    const ctorTitle = document.createElement("h3");
    ctorTitle.textContent = "Constructor";
    ctorSection.appendChild(ctorTitle);
    def.constructorArgs.forEach(arg => {
      const currentValue = node.constructorArgs[arg.key] ?? arg.defaultValue ?? "";
      const row = createFormRow(arg.label, currentValue, value => {
        node.constructorArgs[arg.key] = value;
      }, { placeholder: arg.placeholder });
      ctorSection.appendChild(row);
    });
    fragment.appendChild(ctorSection);
  }

  if (def?.inputs?.length) {
    const inputsSection = document.createElement("div");
    inputsSection.className = "inspector-section";
    const title = document.createElement("h3");
    title.textContent = "Inputs";
    inputsSection.appendChild(title);

    def.inputs.forEach(inputDef => {
      const currentValue = node.inputs[inputDef.key] ?? "";
      const row = createFormRow(inputDef.label, currentValue, value => {
        node.inputs[inputDef.key] = value;
      });
      const dataLink = findInputConnection(node.id, inputDef.key);
      if (dataLink) {
        const chip = document.createElement("div");
        chip.className = "connection-chip";
        const sourceNode = getNodeById(dataLink.from);
        const sourceDef = getNodeDefinition(sourceNode?.typeId ?? "");
        const outputDef = sourceDef?.outputs?.find(out => out.key === dataLink.fromPort);
        const sourceLabel = sourceNode?.label ?? dataLink.from;
        const portLabel = outputDef?.label ?? dataLink.fromPort;
        chip.textContent = `Connected to ${sourceLabel} • ${portLabel}`;
        const unlink = document.createElement("button");
        unlink.className = "secondary";
        unlink.textContent = "Disconnect";
        unlink.addEventListener("click", () => removeDataConnection(dataLink.id));
        chip.appendChild(unlink);
        row.appendChild(chip);
      }
      inputsSection.appendChild(row);
    });

    fragment.appendChild(inputsSection);
  }

  if (def?.options?.length) {
    const optSection = document.createElement("div");
    optSection.className = "inspector-section";
    const optTitle = document.createElement("h3");
    optTitle.textContent = "Options";
    optSection.appendChild(optTitle);

    def.options.forEach(option => {
      const row = document.createElement("div");
      row.className = "checkbox-row";
      const checkbox = document.createElement("input");
      checkbox.type = "checkbox";
      checkbox.checked = !!node.options[option.key];
      checkbox.addEventListener("change", () => {
        node.options[option.key] = checkbox.checked;
      });
      const label = document.createElement("label");
      label.textContent = option.label;
      row.append(checkbox, label);
      optSection.appendChild(row);
    });

    fragment.appendChild(optSection);
  }

  const triggerSection = document.createElement("div");
  triggerSection.className = "inspector-section";
  const connTitle = document.createElement("h3");
  connTitle.textContent = "Trigger Links";
  triggerSection.appendChild(connTitle);

  const outgoingList = document.createElement("ul");
  outgoingList.className = "connection-list";
  const outgoing = state.connections.filter(conn => conn.from === node.id);
  if (outgoing.length === 0) {
    const empty = document.createElement("p");
    empty.textContent = "No outgoing triggers";
    empty.className = "muted";
    triggerSection.appendChild(empty);
  } else {
    outgoing.forEach(conn => {
      const item = document.createElement("li");
      const target = getNodeById(conn.to);
      item.textContent = `→ ${target?.label ?? conn.to}`;
      const removeBtn = document.createElement("button");
      removeBtn.className = "secondary";
      removeBtn.textContent = "Remove";
      removeBtn.addEventListener("click", () => removeConnection(conn.id));
      item.appendChild(removeBtn);
      outgoingList.appendChild(item);
    });
    triggerSection.appendChild(outgoingList);
  }

  if (state.connections.some(conn => conn.to === node.id)) {
    const incomingTitle = document.createElement("h3");
    incomingTitle.textContent = "Incoming";
    triggerSection.appendChild(incomingTitle);
    const incomingList = document.createElement("ul");
    incomingList.className = "connection-list";
    state.connections
      .filter(conn => conn.to === node.id)
      .forEach(conn => {
        const item = document.createElement("li");
        const source = getNodeById(conn.from);
        item.textContent = `${source?.label ?? conn.from} →`;
        const removeBtn = document.createElement("button");
        removeBtn.className = "secondary";
        removeBtn.textContent = "Remove";
        removeBtn.addEventListener("click", () => removeConnection(conn.id));
        item.appendChild(removeBtn);
        incomingList.appendChild(item);
      });
    triggerSection.appendChild(incomingList);
  }

  fragment.appendChild(triggerSection);

  const dataSection = document.createElement("div");
  dataSection.className = "inspector-section";
  const dataTitle = document.createElement("h3");
  dataTitle.textContent = "Data Links";
  dataSection.appendChild(dataTitle);

  const dataOutgoing = getOutputConnections(node.id);
  const dataIncoming = getInputConnections(node.id);
  if (dataOutgoing.length) {
    const list = document.createElement("ul");
    list.className = "connection-list";
    dataOutgoing.forEach(conn => {
      const target = getNodeById(conn.to);
      const targetDef = getNodeDefinition(target?.typeId ?? "");
      const targetInput = targetDef?.inputs?.find(i => i.key === conn.toPort);
      const item = document.createElement("li");
      item.textContent = `${target?.label ?? conn.to} • ${targetInput?.label ?? conn.toPort}`;
      const removeBtn = document.createElement("button");
      removeBtn.className = "secondary";
      removeBtn.textContent = "Remove";
      removeBtn.addEventListener("click", () => removeDataConnection(conn.id));
      item.appendChild(removeBtn);
      list.appendChild(item);
    });
    dataSection.appendChild(list);
  } else if (!dataIncoming.length) {
    const empty = document.createElement("p");
    empty.className = "muted";
    empty.textContent = "No outgoing data links";
    dataSection.appendChild(empty);
  }

  if (dataIncoming.length) {
    const incomingTitle = document.createElement("h3");
    incomingTitle.textContent = "Incoming";
    dataSection.appendChild(incomingTitle);
    const list = document.createElement("ul");
    list.className = "connection-list";
    dataIncoming.forEach(conn => {
      const source = getNodeById(conn.from);
      const sourceDef = getNodeDefinition(source?.typeId ?? "");
      const outputDef = sourceDef?.outputs?.find(out => out.key === conn.fromPort);
      const item = document.createElement("li");
      item.textContent = `${source?.label ?? conn.from} • ${outputDef?.label ?? conn.fromPort}`;
      const removeBtn = document.createElement("button");
      removeBtn.className = "secondary";
      removeBtn.textContent = "Remove";
      removeBtn.addEventListener("click", () => removeDataConnection(conn.id));
      item.appendChild(removeBtn);
      list.appendChild(item);
    });
    dataSection.appendChild(list);
  }

  fragment.appendChild(dataSection);

  if (def?.customSetters?.length) {
    const customSection = document.createElement("div");
    customSection.className = "inspector-section";
    const title = document.createElement("h3");
    title.textContent = "Advanced";
    customSection.appendChild(title);
    def.customSetters.forEach(setter => {
      const currentValue = node.customValues[setter.key] ?? setter.defaultValue ?? "";
      const row = createFormRow(
        setter.label,
        currentValue,
        value => {
          node.customValues[setter.key] = value;
          renderNodes();
        },
        {
          placeholder: setter.placeholder,
          helperText: setter.helperText,
          multiline: setter.inputType === "textarea",
          rows: setter.rows,
          inputType: setter.inputType,
          options: setter.options
        }
      );
      customSection.appendChild(row);
    });
    fragment.appendChild(customSection);
  }

  const deleteBtn = document.createElement("button");
  deleteBtn.style.background = "var(--danger)";
  deleteBtn.style.color = "#0b0404";
  deleteBtn.textContent = "Delete Node";
  deleteBtn.addEventListener("click", () => deleteNode(node.id));
  fragment.appendChild(deleteBtn);

  inspectorContentEl.innerHTML = "";
  inspectorContentEl.appendChild(fragment);
}

function createFormRow(labelText, value, onChange, options = {}) {
  const row = document.createElement("div");
  row.className = "form-row";
  const label = document.createElement("label");
  label.textContent = labelText;
  let input;
  const inputType = options.inputType || (options.multiline ? "textarea" : "text");
  if (inputType === "textarea") {
    input = document.createElement("textarea");
    if (options.rows) {
      input.rows = options.rows;
    }
  } else if (inputType === "select") {
    input = document.createElement("select");
    (options.options || []).forEach(opt => {
      const optionEl = document.createElement("option");
      optionEl.value = opt.value ?? opt.label ?? "";
      optionEl.textContent = opt.label ?? opt.value ?? "";
      input.appendChild(optionEl);
    });
  } else {
    input = document.createElement("input");
    input.type = options.type || "text";
  }
  if (options.placeholder) {
    input.placeholder = options.placeholder;
  }
  input.value = value ?? "";
  const updateHandler = inputType === "select" ? "change" : "input";
  input.addEventListener(updateHandler, () => onChange(input.value));
  row.append(label, input);
  if (options.helperText) {
    const helper = document.createElement("p");
    helper.className = "muted";
    helper.textContent = options.helperText;
    row.appendChild(helper);
  }
  return row;
}

function deleteNode(nodeId) {
  state.nodes = state.nodes.filter(n => n.id !== nodeId);
  state.connections = state.connections.filter(conn => conn.from !== nodeId && conn.to !== nodeId);
  state.dataConnections = state.dataConnections.filter(conn => conn.from !== nodeId && conn.to !== nodeId);
  if (state.selectedNodeId === nodeId) {
    state.selectedNodeId = null;
  }
  renderAll();
}

function pruneConnections(predicate) {
  const before = state.connections.length;
  state.connections = state.connections.filter(conn => !predicate(conn));
  const removed = before - state.connections.length;
  if (removed > 0) {
    renderAll();
    return true;
  }
  return false;
}

function removeConnection(connectionId) {
  pruneConnections(conn => conn.id === connectionId);
}

function disconnectTriggerPort(nodeId) {
  return pruneConnections(conn => conn.from === nodeId);
}

function scheduleConnectionUpdate() {
  if (connectionUpdateScheduled) {
    return;
  }
  connectionUpdateScheduled = true;
  requestAnimationFrame(() => {
    connectionUpdateScheduled = false;
    updateConnectionLines();
  });
}

function updateConnectionLines() {
  const canvasRect = canvasEl.getBoundingClientRect();
  connectionLayerEl.setAttribute("viewBox", `0 0 ${canvasRect.width} ${canvasRect.height}`);
  const triggerPreview = connectionDrag?.previewPath ?? null;
  const dataPreview = ioDrag?.previewPath ?? null;
  if (triggerPreview && triggerPreview.parentElement === connectionLayerEl) {
    connectionLayerEl.removeChild(triggerPreview);
  }
  if (dataPreview && dataPreview.parentElement === connectionLayerEl) {
    connectionLayerEl.removeChild(dataPreview);
  }
  connectionLayerEl.innerHTML = "";

  state.dataConnections.forEach(conn => {
    const startPoint = getOutputPortPosition(conn.from, conn.fromPort);
    const endPoint = getInputPortPosition(conn.to, conn.toPort);
    if (!startPoint || !endPoint) {
      return;
    }
    const path = document.createElementNS("http://www.w3.org/2000/svg", "path");
    path.setAttribute("d", computeConnectionPath(startPoint.x, startPoint.y, endPoint.x, endPoint.y));
    path.setAttribute("fill", "none");
    path.setAttribute("stroke", "var(--connection-data)");
    path.setAttribute("stroke-width", "2");
    path.setAttribute("stroke-linecap", "round");
    path.setAttribute("opacity", "0.9");
    connectionLayerEl.appendChild(path);
  });

  state.connections.forEach(conn => {
    const startPoint = getTriggerPortPosition(conn.from);
    const endPoint = getNodeLeftDock(conn.to);
    if (!startPoint || !endPoint) {
      return;
    }
    const path = document.createElementNS("http://www.w3.org/2000/svg", "path");
    path.setAttribute("d", computeConnectionPath(startPoint.x, startPoint.y, endPoint.x, endPoint.y));
    path.setAttribute("fill", "none");
    path.setAttribute("stroke", "var(--connection-trigger)");
    path.setAttribute("stroke-width", "2");
    path.setAttribute("stroke-linecap", "round");
    path.setAttribute("opacity", "0.9");
    connectionLayerEl.appendChild(path);
  });

  if (dataPreview) {
    connectionLayerEl.appendChild(dataPreview);
  }
  if (triggerPreview) {
    connectionLayerEl.appendChild(triggerPreview);
  }
}

function startConnectionDrag(event, fromNodeId) {
  if (event.ctrlKey) {
    const disconnected = disconnectTriggerPort(fromNodeId);
    if (disconnected) {
      event.stopPropagation();
      event.preventDefault();
      return;
    }
  }
  event.stopPropagation();
  event.preventDefault();
  const triggerAnchor = getTriggerPortPosition(fromNodeId);
  if (!triggerAnchor) {
    return;
  }

  const previewPath = document.createElementNS("http://www.w3.org/2000/svg", "path");
  previewPath.setAttribute("fill", "none");
  previewPath.setAttribute("stroke", "var(--connection-trigger)");
  previewPath.setAttribute("stroke-width", "2");
  previewPath.setAttribute("stroke-linecap", "round");
  previewPath.setAttribute("opacity", "0.65");
  connectionLayerEl.appendChild(previewPath);

  connectionDrag = {
    fromNodeId,
    startX: triggerAnchor.x,
    startY: triggerAnchor.y,
    previewPath,
    hoverTargetId: null
  };

  document.addEventListener("pointermove", onConnectionDragMove);
  document.addEventListener("pointerup", endConnectionDrag);
  updateConnectionPreview(event);
}

function onConnectionDragMove(event) {
  if (!connectionDrag) {
    return;
  }
  updateConnectionPreview(event);
}

function updateConnectionPreview(event) {
  if (!connectionDrag) {
    return;
  }
  const canvasRect = canvasEl.getBoundingClientRect();
  let endX = event.clientX - canvasRect.left;
  let endY = event.clientY - canvasRect.top;

  const hoveredCard = findNodeCardAtPoint(event.clientX, event.clientY);
  const hoveredId = hoveredCard?.dataset.nodeId ?? null;
  if (hoveredId === connectionDrag.fromNodeId) {
    setDragHoverTarget(null);
  } else {
    setDragHoverTarget(hoveredId);
    if (hoveredId) {
      const dock = getNodeLeftDock(hoveredId);
      if (dock) {
        endX = dock.x;
        endY = dock.y;
      }
    }
  }

  connectionDrag.previewPath.setAttribute("d", computeConnectionPath(connectionDrag.startX, connectionDrag.startY, endX, endY));
}

function endConnectionDrag(event) {
  if (!connectionDrag) {
    return;
  }
  document.removeEventListener("pointermove", onConnectionDragMove);
  document.removeEventListener("pointerup", endConnectionDrag);

  const { fromNodeId, previewPath, hoverTargetId } = connectionDrag;
  if (previewPath && previewPath.parentElement === connectionLayerEl) {
    connectionLayerEl.removeChild(previewPath);
  }
  const hoveredCard = hoverTargetId ? nodesLayerEl.querySelector(`[data-node-id="${hoverTargetId}"]`) : findNodeCardAtPoint(event.clientX, event.clientY);
  const hoveredId = hoveredCard?.dataset?.nodeId;

  if (hoveredId && hoveredId !== fromNodeId) {
    const exists = state.connections.some(conn => conn.from === fromNodeId && conn.to === hoveredId);
    if (!exists) {
      state.connections.push({ id: createId(), from: fromNodeId, to: hoveredId });
      renderAll();
    }
  }

  setDragHoverTarget(null);
  connectionDrag = null;
}

function setDragHoverTarget(nodeId) {
  if (!connectionDrag) {
    return;
  }
  if (connectionDrag.hoverTargetId && connectionDrag.hoverTargetId !== nodeId) {
    const prev = nodesLayerEl.querySelector(`[data-node-id="${connectionDrag.hoverTargetId}"]`);
    prev?.classList.remove("drop-target");
  }
  connectionDrag.hoverTargetId = nodeId ?? null;
  if (connectionDrag.hoverTargetId) {
    const next = nodesLayerEl.querySelector(`[data-node-id="${connectionDrag.hoverTargetId}"]`);
    next?.classList.add("drop-target");
  }
}

function startIoDrag(event, nodeId, portKey, role) {
  if (event.ctrlKey) {
    const disconnected = disconnectIoPort(nodeId, portKey, role);
    if (disconnected) {
      event.stopPropagation();
      event.preventDefault();
      return;
    }
  }
  event.stopPropagation();
  event.preventDefault();
  const element = getPortElement(nodeId, portKey, role);
  if (!element) {
    return;
  }
  const anchor = getPortPosition(nodeId, portKey, role);
  if (!anchor) {
    return;
  }

  const previewPath = document.createElementNS("http://www.w3.org/2000/svg", "path");
  previewPath.setAttribute("fill", "none");
  previewPath.setAttribute("stroke", "var(--connection-data)");
  previewPath.setAttribute("stroke-width", "2");
  previewPath.setAttribute("stroke-linecap", "round");
  previewPath.setAttribute("opacity", "0.9");
  connectionLayerEl.appendChild(previewPath);

  ioDrag = {
    nodeId,
    portKey,
    role,
    anchor,
    previewPath,
    hoverPort: null,
    dataType: normalizeDataType(element.dataset.dataType)
  };

  document.addEventListener("pointermove", onIoDragMove);
  document.addEventListener("pointerup", endIoDrag);
  updateIoPreview(event);
}

function onIoDragMove(event) {
  if (!ioDrag) {
    return;
  }
  updateIoPreview(event);
}

function updateIoPreview(event) {
  if (!ioDrag) {
    return;
  }
  const canvasRect = canvasEl.getBoundingClientRect();
  let pointer = {
    x: event.clientX - canvasRect.left,
    y: event.clientY - canvasRect.top
  };

  const portHit = findPortElementAtPoint(event.clientX, event.clientY);
  let hoverPort = null;
  if (portHit && portsCompatibleForDrag(ioDrag, portHit)) {
    const targetPos = getPortPosition(portHit.nodeId, portHit.portKey, portHit.role);
    if (targetPos) {
      pointer = targetPos;
    }
    hoverPort = portHit;
  }
  setPortHoverTarget(hoverPort);

  let startX = ioDrag.anchor.x;
  let startY = ioDrag.anchor.y;
  let endX = pointer.x;
  let endY = pointer.y;

  if (ioDrag.role === "input") {
    startX = pointer.x;
    startY = pointer.y;
    endX = ioDrag.anchor.x;
    endY = ioDrag.anchor.y;
  }

  ioDrag.previewPath.setAttribute("d", computeConnectionPath(startX, startY, endX, endY));
}

function endIoDrag(event) {
  if (!ioDrag) {
    return;
  }
  document.removeEventListener("pointermove", onIoDragMove);
  document.removeEventListener("pointerup", endIoDrag);

  if (ioDrag.previewPath && ioDrag.previewPath.parentElement === connectionLayerEl) {
    connectionLayerEl.removeChild(ioDrag.previewPath);
  }

  const portHit = ioDrag.hoverPort || findPortElementAtPoint(event.clientX, event.clientY);
  if (portHit && portHit.role !== ioDrag.role) {
    const from = ioDrag.role === "output"
      ? { nodeId: ioDrag.nodeId, portKey: ioDrag.portKey }
      : { nodeId: portHit.nodeId, portKey: portHit.portKey };
    const to = ioDrag.role === "output"
      ? { nodeId: portHit.nodeId, portKey: portHit.portKey }
      : { nodeId: ioDrag.nodeId, portKey: ioDrag.portKey };
    addDataConnection(from.nodeId, from.portKey, to.nodeId, to.portKey);
  }

  setPortHoverTarget(null);
  ioDrag = null;
  scheduleConnectionUpdate();
}

function setPortHoverTarget(portInfo) {
  const current = ioDrag?.hoverPort;
  if (current && (!portInfo || current.element !== portInfo.element)) {
    current.element.classList.remove("port-hover");
  }
  if (ioDrag) {
    ioDrag.hoverPort = portInfo ? { ...portInfo } : null;
  }
  if (portInfo?.element) {
    portInfo.element.classList.add("port-hover");
  }
}

function findPortElementAtPoint(clientX, clientY) {
  const element = document.elementFromPoint(clientX, clientY);
  const target = element?.closest(".port-knob, .port-row, .port-label");
  if (!target) {
    return null;
  }
  const role = target.dataset.portRole;
  if (!role || role === "trigger") {
    return null;
  }
  const nodeId = target.dataset.nodeId;
  const portKey = target.dataset.portKey;
  if (!nodeId || !portKey) {
    return null;
  }
  const knob = target.classList.contains("port-knob") ? target : target.querySelector(".port-knob");
  return {
    nodeId,
    portKey,
    role,
    element: knob || target
  };
}

function portsCompatibleForDrag(dragState, portInfo) {
  if (!dragState || !portInfo) {
    return false;
  }
  if (portInfo.role === "trigger" || portInfo.role === dragState.role) {
    return false;
  }
  const portType = normalizeDataType(portInfo.element?.dataset?.dataType);
  if (!dragState.dataType || !portType) {
    return true;
  }
  return dragState.dataType === portType;
}

function addDataConnection(fromNodeId, fromPortKey, toNodeId, toPortKey) {
  if (fromNodeId === toNodeId) {
    return;
  }
  const fromNode = getNodeById(fromNodeId);
  const toNode = getNodeById(toNodeId);
  if (!fromNode || !toNode) {
    return;
  }
  const fromDef = getNodeDefinition(fromNode.typeId);
  const toDef = getNodeDefinition(toNode.typeId);
  if (!fromDef || !toDef) {
    return;
  }
  const outputDef = (fromDef.outputs || []).find(out => out.key === fromPortKey);
  const inputDef = (toDef.inputs || []).find(input => input.key === toPortKey);
  if (!outputDef || !inputDef) {
    return;
  }
  const fromType = normalizeDataType(outputDef.dataType);
  const toType = normalizeDataType(inputDef.dataType);
  if (fromType && toType && fromType !== toType) {
    return;
  }
  const existingIndex = state.dataConnections.findIndex(conn => conn.to === toNodeId && conn.toPort === toPortKey);
  if (existingIndex >= 0) {
    state.dataConnections.splice(existingIndex, 1);
  }
  state.dataConnections.push({
    id: createId(),
    from: fromNodeId,
    fromPort: fromPortKey,
    to: toNodeId,
    toPort: toPortKey
  });
  renderAll();
}

function pruneDataConnections(predicate) {
  const before = state.dataConnections.length;
  state.dataConnections = state.dataConnections.filter(conn => !predicate(conn));
  const removed = before - state.dataConnections.length;
  if (removed > 0) {
    renderAll();
    return true;
  }
  return false;
}

function removeDataConnection(connectionId) {
  pruneDataConnections(conn => conn.id === connectionId);
}

function disconnectIoPort(nodeId, portKey, role) {
  if (role === "input") {
    return pruneDataConnections(conn => conn.to === nodeId && conn.toPort === portKey);
  }
  if (role === "output") {
    return pruneDataConnections(conn => conn.from === nodeId && conn.fromPort === portKey);
  }
  return false;
}

function getPortPosition(nodeId, portKey, role) {
  const selector = `.port-knob[data-node-id="${nodeId}"][data-port-key="${portKey}"][data-port-role="${role}"]`;
  const element = nodesLayerEl.querySelector(selector);
  if (!element) {
    return null;
  }
  const rect = element.getBoundingClientRect();
  const canvasRect = canvasEl.getBoundingClientRect();
  return {
    x: rect.left - canvasRect.left + rect.width / 2,
    y: rect.top - canvasRect.top + rect.height / 2
  };
}

function getPortElement(nodeId, portKey, role) {
  const selector = `.port-knob[data-node-id="${nodeId}"][data-port-key="${portKey}"][data-port-role="${role}"]`;
  return nodesLayerEl.querySelector(selector);
}

function getInputPortPosition(nodeId, portKey) {
  return getPortPosition(nodeId, portKey, "input");
}

function getOutputPortPosition(nodeId, portKey) {
  return getPortPosition(nodeId, portKey, "output");
}

function findNodeCardAtPoint(clientX, clientY) {
  const element = document.elementFromPoint(clientX, clientY);
  return element?.closest(".node-card") ?? null;
}

function getTriggerPortPosition(nodeId) {
  const knobPosition = getPortPosition(nodeId, TRIGGER_PORT_KEY, "trigger");
  if (knobPosition) {
    return knobPosition;
  }
  const card = nodesLayerEl.querySelector(`[data-node-id="${nodeId}"]`);
  if (!card) {
    return null;
  }
  const canvasRect = canvasEl.getBoundingClientRect();
  const rect = card.getBoundingClientRect();
  return {
    x: rect.right - canvasRect.left,
    y: rect.top - canvasRect.top + rect.height / 2
  };
}

function getNodeLeftDock(nodeId) {
  const card = nodesLayerEl.querySelector(`[data-node-id="${nodeId}"]`);
  if (!card) {
    return null;
  }
  const canvasRect = canvasEl.getBoundingClientRect();
  const rect = card.getBoundingClientRect();
  return {
    x: rect.left - canvasRect.left - 6,
    y: rect.top - canvasRect.top + rect.height / 2
  };
}

function computeConnectionPath(startX, startY, endX, endY) {
  const dx = endX - startX;
  const curvature = Math.min(Math.abs(dx) * 0.5, 140);
  const controlOffset = dx >= 0 ? curvature : -curvature;
  const c1x = startX + controlOffset;
  const c1y = startY;
  const c2x = endX - controlOffset;
  const c2y = endY;
  return `M ${startX} ${startY} C ${c1x} ${c1y}, ${c2x} ${c2y}, ${endX} ${endY}`;
}

function sanitizeIdentifier(value) {
  const cleaned = (value || "")
    .replace(/[^0-9a-zA-Z_]/g, "")
    .replace(/^[^a-zA-Z_]+/, "");
  return cleaned || "nodeVar";
}

function makeUniqueIdentifier(base) {
  const cleanedBase = sanitizeIdentifier(base);
  let attempt = cleanedBase || "node";
  let counter = 1;
  const existing = new Set(state.nodes.map(n => n.variableName));
  while (existing.has(attempt)) {
    attempt = `${cleanedBase}${counter++}`;
  }
  return attempt;
}

function clamp(value, min, max) {
  return Math.min(Math.max(value, min), max);
}

function formatInputAccessor(def, inputMeta) {
  if (!def || !inputMeta) {
    return "0";
  }
  if (inputMeta.accessorType === "index") {
    return inputMeta.cppAccessor ?? "0";
  }
  return `${def.cppType}::${inputMeta.cppAccessor}`;
}

function formatOutputAccessor(def, outputMeta) {
  if (!def || !outputMeta) {
    return "0";
  }
  if (outputMeta.accessorType === "enum") {
    return `${def.cppType}::${outputMeta.cppAccessor}`;
  }
  return outputMeta.cppAccessor ?? "0";
}

function generateCpp() {
  if (state.nodes.length === 0) {
    return "// Add nodes to the graph to generate code.";
  }
  const sortedNodes = [...state.nodes].sort((a, b) => a.createdAt - b.createdAt);
  const hasParticleNodes = sortedNodes.some(node => {
    const def = getNodeDefinition(node.typeId);
    return (def?.category || "Particles") === "Particles";
  });
  const includes = new Set();
  sortedNodes.forEach(node => {
    const def = getNodeDefinition(node.typeId);
    if (!def) {
      return;
    }
    const includeList = [];
    if (def.include) {
      includeList.push(def.include);
    }
    if (Array.isArray(def.includes)) {
      includeList.push(...def.includes);
    }
    includeList.forEach(inc => {
      if (inc) {
        includes.add(inc);
      }
    });
  });
  if (hasParticleNodes) {
    includes.add("weave/particles/core/ParticleMachine.h");
  } else {
    includes.add("weave/system/blender/Blender.h");
  }

  const lines = [];
  lines.push("// --- Generated with Loom Blender ---");
  if (includes.size) {
    includes.forEach(inc => lines.push(`#include "${inc}"`));
    lines.push("");
  }
  if (hasParticleNodes) {
    lines.push("namespace wp = weave::particles;");
  }
  lines.push("namespace wb = weave::blender;");
  lines.push("");
  if (hasParticleNodes) {
    lines.push("wp::ParticleMachine particleMachine;");
    lines.push("auto& graph = particleMachine.Graph();");
  } else {
    lines.push("wb::Blender blender;");
    lines.push("auto& graph = blender;");
  }

  const variableMap = new Map();
  const usedNames = new Set();
  const nodeById = new Map(sortedNodes.map(node => [node.id, node]));

  sortedNodes.forEach(node => {
    let candidate = sanitizeIdentifier(node.variableName || node.label || "node");
    if (!candidate) {
      candidate = "node";
    }
    let uniqueName = candidate;
    let suffix = 1;
    while (usedNames.has(uniqueName)) {
      uniqueName = `${candidate}${suffix++}`;
    }
    usedNames.add(uniqueName);
    variableMap.set(node.id, uniqueName);

    const def = getNodeDefinition(node.typeId);
    const ctorValues = [];
    (def.constructorArgs || []).forEach(arg => {
      const stored = (node.constructorArgs?.[arg.key] ?? "").trim();
      const fallback = (arg.defaultValue ?? "").trim();
      const finalValue = stored || fallback;
      if (finalValue) {
        ctorValues.push(finalValue);
      }
    });
    const ctorCall = ctorValues.length ? `(${ctorValues.join(", ")})` : "()";
    lines.push("");
    lines.push(`auto ${uniqueName} = graph.CreateNode<${def.cppType}>${ctorCall};`);
    (def.inputs || []).forEach(input => {
      const value = (node.inputs?.[input.key] ?? "").trim();
      if (value) {
        const templateArg = formatInputAccessor(def, input);
        lines.push(`${uniqueName}->input.SetDefaultValue<${templateArg}>(${value});`);
      }
    });
    (def.options || []).forEach(option => {
      const current = !!node.options?.[option.key];
      if (current !== (option.defaultValue ?? false)) {
        const boolValue = current ? "true" : "false";
        lines.push(`${uniqueName}->${option.method}(${boolValue});`);
      }
    });
    (def.customSetters || []).forEach(setter => {
      const rawValue = (node.customValues?.[setter.key] ?? "").trim();
      let formatted = rawValue;
      if (setter.formatValue) {
        formatted = setter.formatValue(rawValue, def, setter) ?? "";
      }
      if (formatted) {
        lines.push(`${uniqueName}->${setter.method}(${formatted});`);
      }
    });
  });

  const rootNodes = sortedNodes.filter(node => node.isRoot);
  if (rootNodes.length) {
    lines.push("");
    rootNodes.forEach(node => {
      const varName = variableMap.get(node.id);
      lines.push(`graph.AddRootTrigger(${varName});`);
    });
  }

  if (state.dataConnections.length) {
    lines.push("");
    lines.push("// Data connections");
    state.dataConnections.forEach(conn => {
      const fromVar = variableMap.get(conn.from);
      const toVar = variableMap.get(conn.to);
      const fromNode = nodeById.get(conn.from);
      const toNode = nodeById.get(conn.to);
      const fromDef = getNodeDefinition(fromNode?.typeId ?? "");
      const toDef = getNodeDefinition(toNode?.typeId ?? "");
      if (!fromVar || !toVar || !fromDef || !toDef) {
        return;
      }
      const inputMeta = (toDef.inputs || []).find(input => input.key === conn.toPort);
      const outputMeta = (fromDef.outputs || []).find(output => output.key === conn.fromPort);
      if (!inputMeta || !outputMeta) {
        return;
      }
      const inputAccessor = formatInputAccessor(toDef, inputMeta);
      const outputAccessor = formatOutputAccessor(fromDef, outputMeta);
      lines.push(`${toVar}->ConnectInputTo(${inputAccessor}, ${fromVar}, ${outputAccessor});`);
    });
  }

  if (state.connections.length) {
    lines.push("");
    state.connections.forEach(conn => {
      const fromVar = variableMap.get(conn.from);
      const toVar = variableMap.get(conn.to);
      if (fromVar && toVar) {
        lines.push(`${fromVar}->ConnectTrigger(${toVar});`);
      }
    });
  }

  lines.push("");
  if (hasParticleNodes) {
    lines.push("// particleMachine.SetSamplingData(deltaTime, iteration, isVisible);");
    lines.push("// particleMachine.Execute();");
  } else {
    lines.push("// blender.Execute();");
  }
  return lines.join("\n");
}
