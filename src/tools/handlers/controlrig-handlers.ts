/**
 * FORGE2: Control Rig Handlers
 * Handles UE5 Control Rig animation systems including IK, physics-driven animation,
 * procedural animation, and motion matching
 */

import { ITools } from '../../types/tool-interfaces.js';
import type { HandlerArgs } from '../../types/handler-types.js';
import {
  Forge2Categories,
  executeForge2Action,
  createCategoryHandler,
  type ActionHandler,
  type ActionHandlerMap
} from './utils/forge2-bridge.js';

// ============================================================================
// Helper Functions - Validators
// ============================================================================

function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value;
}

function validateOptionalString(value: unknown): string | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value !== 'string') return undefined;
  return value.trim() || undefined;
}

function validateOptionalNumber(value: unknown): number | undefined {
  if (value === undefined || value === null) return undefined;
  const num = Number(value);
  return isNaN(num) ? undefined : num;
}

function validateOptionalBoolean(value: unknown): boolean | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value === 'boolean') return value;
  if (value === 'true') return true;
  if (value === 'false') return false;
  return undefined;
}

function validateOptionalEnum<T extends string>(
  value: unknown,
  allowedValues: readonly T[]
): T | undefined {
  if (value === undefined || value === null || value === '') {
    return undefined;
  }
  if (typeof value !== 'string' || !allowedValues.includes(value as T)) {
    return undefined;
  }
  return value as T;
}

function validateOptionalArray(value: unknown): unknown[] | undefined {
  if (value === undefined || value === null) return undefined;
  if (Array.isArray(value)) return value;
  return undefined;
}

function validateOptionalObject(value: unknown): Record<string, unknown> | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value === 'object' && !Array.isArray(value)) {
    return value as Record<string, unknown>;
  }
  return undefined;
}

// ============================================================================
// Constants
// ============================================================================

const IK_TYPES = ['two_bone', 'full_body', 'limb', 'spline', 'aim', 'ccdik', 'fabrik'] as const;
const CONTROL_TYPES = ['transform', 'float', 'vector', 'bool'] as const;
const SPACE_TYPES = ['world', 'component', 'local', 'bone'] as const;
const BLEND_MODES = ['replace', 'additive', 'layered'] as const;
const DRIVE_SOURCES = ['animation', 'physics', 'procedural', 'user'] as const;

// Reserve constants for future use
void IK_TYPES;
void CONTROL_TYPES;
void SPACE_TYPES;
void BLEND_MODES;
void DRIVE_SOURCES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a Control Rig asset
 */
const handleCreateControlRig: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigName = validateRequiredString(params.rig_name || params.rigName, 'rig_name');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'create_control_rig', {
    rigName,
    skeletonPath: validateOptionalString(params.skeleton_path || params.skeletonPath),
    skeletalMeshPath: validateOptionalString(params.skeletal_mesh_path || params.skeletalMeshPath),
    parentRigPath: validateOptionalString(params.parent_rig_path || params.parentRigPath),
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add IK chain to Control Rig
 */
const handleAddIKChain: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');
  const chainName = validateRequiredString(params.chain_name || params.chainName, 'chain_name');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'add_ik_chain', {
    rigPath,
    chainName,
    ikType: validateOptionalEnum(params.ik_type || params.ikType, IK_TYPES) ?? 'two_bone',
    rootBone: validateRequiredString(params.root_bone || params.rootBone, 'root_bone'),
    endBone: validateRequiredString(params.end_bone || params.endBone, 'end_bone'),
    poleBone: validateOptionalString(params.pole_bone || params.poleBone),
    goalBone: validateOptionalString(params.goal_bone || params.goalBone),
    precision: validateOptionalNumber(params.precision) ?? 0.01,
    maxIterations: validateOptionalNumber(params.max_iterations || params.maxIterations) ?? 15,
    stretchEnabled: validateOptionalBoolean(params.stretch_enabled || params.stretchEnabled) ?? false,
    stretchLimits: validateOptionalArray(params.stretch_limits || params.stretchLimits)
  });
};

/**
 * Add FK control to Control Rig
 */
const handleAddFKControl: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');
  const controlName = validateRequiredString(params.control_name || params.controlName, 'control_name');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'add_fk_control', {
    rigPath,
    controlName,
    driveBone: validateRequiredString(params.drive_bone || params.driveBone, 'drive_bone'),
    controlType: validateOptionalEnum(params.control_type || params.controlType, CONTROL_TYPES) ?? 'transform',
    controlShape: validateOptionalString(params.control_shape || params.controlShape) ?? 'box',
    controlSize: validateOptionalNumber(params.control_size || params.controlSize) ?? 10.0,
    controlColor: validateOptionalString(params.control_color || params.controlColor),
    space: validateOptionalEnum(params.space, SPACE_TYPES) ?? 'local',
    parentControl: validateOptionalString(params.parent_control || params.parentControl)
  });
};

/**
 * Setup physics-driven animation
 */
const handleSetupPhysicsAnimation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'setup_physics_animation', {
    rigPath,
    bones: validateOptionalArray(params.bones) ?? [],
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    mass: validateOptionalNumber(params.mass) ?? 1.0,
    stiffness: validateOptionalNumber(params.stiffness) ?? 100.0,
    damping: validateOptionalNumber(params.damping) ?? 5.0,
    gravityScale: validateOptionalNumber(params.gravity_scale || params.gravityScale) ?? 1.0,
    windScale: validateOptionalNumber(params.wind_scale || params.windScale) ?? 0.0,
    collisionEnabled: validateOptionalBoolean(params.collision_enabled || params.collisionEnabled) ?? false,
    simulateInComponentSpace: validateOptionalBoolean(params.simulate_in_component_space || params.simulateInComponentSpace) ?? true
  });
};

/**
 * Configure Full Body IK
 */
const handleConfigureFullBodyIK: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'configure_full_body_ik', {
    rigPath,
    rootBone: validateRequiredString(params.root_bone || params.rootBone, 'root_bone'),
    solverSettings: validateOptionalObject(params.solver_settings || params.solverSettings),
    effectors: validateOptionalArray(params.effectors) ?? [],
    constraints: validateOptionalArray(params.constraints) ?? [],
    pullChainEnabled: validateOptionalBoolean(params.pull_chain_enabled || params.pullChainEnabled) ?? true,
    rootBehavior: validateOptionalString(params.root_behavior || params.rootBehavior) ?? 'pin_to_input',
    massMultiplier: validateOptionalNumber(params.mass_multiplier || params.massMultiplier) ?? 1.0,
    maxIterations: validateOptionalNumber(params.max_iterations || params.maxIterations) ?? 20
  });
};

/**
 * Create procedural animation
 */
const handleCreateProceduralAnimation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');
  const animationType = validateRequiredString(params.animation_type || params.animationType, 'animation_type');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'create_procedural_animation', {
    rigPath,
    animationType, // 'breathing', 'look_at', 'lean', 'sway', 'idle_micro'
    targetBones: validateOptionalArray(params.target_bones || params.targetBones) ?? [],
    amplitude: validateOptionalNumber(params.amplitude) ?? 1.0,
    frequency: validateOptionalNumber(params.frequency) ?? 1.0,
    phase: validateOptionalNumber(params.phase) ?? 0.0,
    blendWeight: validateOptionalNumber(params.blend_weight || params.blendWeight) ?? 1.0,
    noiseScale: validateOptionalNumber(params.noise_scale || params.noiseScale),
    axis: validateOptionalString(params.axis) ?? 'z',
    useNoise: validateOptionalBoolean(params.use_noise || params.useNoise) ?? false
  });
};

/**
 * Setup motion matching
 */
const handleSetupMotionMatching: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'setup_motion_matching', {
    rigPath,
    databasePath: validateOptionalString(params.database_path || params.databasePath),
    trajectoryBones: validateOptionalArray(params.trajectory_bones || params.trajectoryBones) ?? [],
    poseBones: validateOptionalArray(params.pose_bones || params.poseBones) ?? [],
    trajectoryHistoryCount: validateOptionalNumber(params.trajectory_history_count || params.trajectoryHistoryCount) ?? 4,
    trajectoryPredictionCount: validateOptionalNumber(params.trajectory_prediction_count || params.trajectoryPredictionCount) ?? 3,
    blendTime: validateOptionalNumber(params.blend_time || params.blendTime) ?? 0.2,
    searchThrottle: validateOptionalNumber(params.search_throttle || params.searchThrottle) ?? 0.1
  });
};

/**
 * Configure ragdoll blend
 */
const handleConfigureRagdollBlend: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'configure_ragdoll_blend', {
    rigPath,
    physicsAssetPath: validateOptionalString(params.physics_asset_path || params.physicsAssetPath),
    blendWeight: validateOptionalNumber(params.blend_weight || params.blendWeight) ?? 0.0,
    bonesForPhysics: validateOptionalArray(params.bones_for_physics || params.bonesForPhysics) ?? [],
    driveStiffness: validateOptionalNumber(params.drive_stiffness || params.driveStiffness) ?? 100.0,
    driveDamping: validateOptionalNumber(params.drive_damping || params.driveDamping) ?? 10.0,
    velocityBlendEnabled: validateOptionalBoolean(params.velocity_blend_enabled || params.velocityBlendEnabled) ?? true
  });
};

/**
 * Add constraint to Control Rig
 */
const handleAddConstraint: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');
  const constraintName = validateRequiredString(params.constraint_name || params.constraintName, 'constraint_name');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'add_constraint', {
    rigPath,
    constraintName,
    constraintType: validateRequiredString(params.constraint_type || params.constraintType, 'constraint_type'), // 'aim', 'parent', 'position', 'rotation', 'scale'
    drivenBone: validateRequiredString(params.driven_bone || params.drivenBone, 'driven_bone'),
    driverBone: validateOptionalString(params.driver_bone || params.driverBone),
    driverControl: validateOptionalString(params.driver_control || params.driverControl),
    maintainOffset: validateOptionalBoolean(params.maintain_offset || params.maintainOffset) ?? true,
    weight: validateOptionalNumber(params.weight) ?? 1.0,
    constraintSpace: validateOptionalEnum(params.constraint_space || params.constraintSpace, SPACE_TYPES) ?? 'world'
  });
};

/**
 * Setup foot placement IK
 */
const handleSetupFootPlacement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'setup_foot_placement', {
    rigPath,
    leftFootBone: validateRequiredString(params.left_foot_bone || params.leftFootBone, 'left_foot_bone'),
    rightFootBone: validateRequiredString(params.right_foot_bone || params.rightFootBone, 'right_foot_bone'),
    hipBone: validateRequiredString(params.hip_bone || params.hipBone, 'hip_bone'),
    traceDistance: validateOptionalNumber(params.trace_distance || params.traceDistance) ?? 50.0,
    footOffset: validateOptionalNumber(params.foot_offset || params.footOffset) ?? 0.0,
    hipAdjustment: validateOptionalBoolean(params.hip_adjustment || params.hipAdjustment) ?? true,
    maxHipOffset: validateOptionalNumber(params.max_hip_offset || params.maxHipOffset) ?? 30.0,
    interpSpeed: validateOptionalNumber(params.interp_speed || params.interpSpeed) ?? 15.0,
    rotateToSlope: validateOptionalBoolean(params.rotate_to_slope || params.rotateToSlope) ?? true
  });
};

/**
 * Setup hand IK for weapons/tools
 */
const handleSetupHandIK: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'setup_hand_ik', {
    rigPath,
    handBone: validateRequiredString(params.hand_bone || params.handBone, 'hand_bone'),
    side: validateRequiredString(params.side, 'side'), // 'left', 'right'
    effectorType: validateOptionalString(params.effector_type || params.effectorType) ?? 'socket', // 'socket', 'bone', 'transform'
    targetSocket: validateOptionalString(params.target_socket || params.targetSocket),
    targetBone: validateOptionalString(params.target_bone || params.targetBone),
    blendWeight: validateOptionalNumber(params.blend_weight || params.blendWeight) ?? 1.0,
    interpSpeed: validateOptionalNumber(params.interp_speed || params.interpSpeed) ?? 15.0
  });
};

/**
 * Get Control Rig information
 */
const handleGetControlRigInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const rigPath = validateRequiredString(params.rig_path || params.rigPath, 'rig_path');

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'get_control_rig_info', {
    rigPath,
    includeControls: validateOptionalBoolean(params.include_controls || params.includeControls) ?? true,
    includeChains: validateOptionalBoolean(params.include_chains || params.includeChains) ?? true,
    includeConstraints: validateOptionalBoolean(params.include_constraints || params.includeConstraints) ?? true
  });
};

/**
 * List Control Rigs
 */
const handleListControlRigs: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTROL_RIG, 'list_control_rigs', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const controlrigHandlers: ActionHandlerMap = {
  // Creation
  create_control_rig: handleCreateControlRig,
  add_ik_chain: handleAddIKChain,
  add_fk_control: handleAddFKControl,
  add_constraint: handleAddConstraint,

  // Full Body IK
  configure_full_body_ik: handleConfigureFullBodyIK,
  setup_foot_placement: handleSetupFootPlacement,
  setup_hand_ik: handleSetupHandIK,

  // Physics & Procedural
  setup_physics_animation: handleSetupPhysicsAnimation,
  create_procedural_animation: handleCreateProceduralAnimation,
  configure_ragdoll_blend: handleConfigureRagdollBlend,

  // Motion Matching
  setup_motion_matching: handleSetupMotionMatching,

  // Query
  get_control_rig_info: handleGetControlRigInfo,
  list_control_rigs: handleListControlRigs
};

export const handleControlRigTools = createCategoryHandler(Forge2Categories.CONTROL_RIG, controlrigHandlers);

export default controlrigHandlers;
