/**
 * FORGE2: Chaos Handlers
 * Handles UE5 Chaos Physics and Destruction systems including destructibles,
 * geometry collections, physics fields, vehicles, and cloth simulation
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

const FRACTURE_MODES = ['voronoi', 'uniform', 'clustered', 'radial', 'planar', 'brick'] as const;
const DAMAGE_TYPES = ['impact', 'explosion', 'slice', 'shatter'] as const;
const FIELD_TYPES = ['radial_force', 'vector_force', 'noise', 'kill', 'disable', 'sleep'] as const;
const VEHICLE_TYPES = ['wheeled', 'tracked', 'hover', 'boat', 'aircraft'] as const;
const CLOTH_TYPES = ['standard', 'nv_cloth', 'chaos_cloth'] as const;

// Reserve constants for future use
void FRACTURE_MODES;
void DAMAGE_TYPES;
void FIELD_TYPES;
void VEHICLE_TYPES;
void CLOTH_TYPES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a destructible from static mesh
 */
const handleCreateDestructible: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const meshPath = validateRequiredString(params.mesh_path || params.meshPath, 'mesh_path');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'create_destructible', {
    meshPath,
    fractureMode: validateOptionalEnum(params.fracture_mode || params.fractureMode, FRACTURE_MODES) ?? 'voronoi',
    fractureCount: validateOptionalNumber(params.fracture_count || params.fractureCount) ?? 10,
    randomSeed: validateOptionalNumber(params.random_seed || params.randomSeed),
    clusterCount: validateOptionalNumber(params.cluster_count || params.clusterCount),
    clusterRadius: validateOptionalNumber(params.cluster_radius || params.clusterRadius),
    minimumSize: validateOptionalNumber(params.minimum_size || params.minimumSize) ?? 1.0,
    grout: validateOptionalNumber(params.grout) ?? 0.0,
    collisionEnabled: validateOptionalBoolean(params.collision_enabled || params.collisionEnabled) ?? true,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Configure destruction levels
 */
const handleConfigureDestructionLevels: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const geometryCollectionPath = validateRequiredString(params.geometry_collection_path || params.geometryCollectionPath, 'geometry_collection_path');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'configure_destruction_levels', {
    geometryCollectionPath,
    damageThresholds: validateOptionalArray(params.damage_thresholds || params.damageThresholds) ?? [100, 500, 1000],
    levelCount: validateOptionalNumber(params.level_count || params.levelCount) ?? 3,
    clusterUnionEnabled: validateOptionalBoolean(params.cluster_union_enabled || params.clusterUnionEnabled) ?? true,
    removeOnSleep: validateOptionalBoolean(params.remove_on_sleep || params.removeOnSleep) ?? true,
    removeOnSleepTime: validateOptionalNumber(params.remove_on_sleep_time || params.removeOnSleepTime) ?? 5.0,
    debrisLifetime: validateOptionalNumber(params.debris_lifetime || params.debrisLifetime) ?? 10.0,
    debrisFadeTime: validateOptionalNumber(params.debris_fade_time || params.debrisFadeTime) ?? 2.0
  });
};

/**
 * Setup debris system
 */
const handleSetupDebrisSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const geometryCollectionPath = validateRequiredString(params.geometry_collection_path || params.geometryCollectionPath, 'geometry_collection_path');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'setup_debris_system', {
    geometryCollectionPath,
    debrisMeshes: validateOptionalArray(params.debris_meshes || params.debrisMeshes) ?? [],
    spawnOnBreak: validateOptionalBoolean(params.spawn_on_break || params.spawnOnBreak) ?? true,
    debrisCount: validateOptionalNumber(params.debris_count || params.debrisCount) ?? 5,
    debrisLifetime: validateOptionalNumber(params.debris_lifetime || params.debrisLifetime) ?? 10.0,
    inheritVelocity: validateOptionalBoolean(params.inherit_velocity || params.inheritVelocity) ?? true,
    velocityScale: validateOptionalNumber(params.velocity_scale || params.velocityScale) ?? 1.0,
    enableCollision: validateOptionalBoolean(params.enable_collision || params.enableCollision) ?? true,
    collisionProfile: validateOptionalString(params.collision_profile || params.collisionProfile)
  });
};

/**
 * Create Chaos vehicle
 */
const handleCreateChaosVehicle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const vehicleName = validateRequiredString(params.vehicle_name || params.vehicleName, 'vehicle_name');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'create_chaos_vehicle', {
    vehicleName,
    vehicleType: validateOptionalEnum(params.vehicle_type || params.vehicleType, VEHICLE_TYPES) ?? 'wheeled',
    bodyMeshPath: validateOptionalString(params.body_mesh_path || params.bodyMeshPath),
    wheelMeshPath: validateOptionalString(params.wheel_mesh_path || params.wheelMeshPath),
    wheelCount: validateOptionalNumber(params.wheel_count || params.wheelCount) ?? 4,
    mass: validateOptionalNumber(params.mass) ?? 1500.0,
    chassisWidth: validateOptionalNumber(params.chassis_width || params.chassisWidth) ?? 200.0,
    chassisHeight: validateOptionalNumber(params.chassis_height || params.chassisHeight) ?? 100.0,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Configure vehicle wheels
 */
const handleConfigureVehicleWheels: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const vehiclePath = validateRequiredString(params.vehicle_path || params.vehiclePath, 'vehicle_path');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'configure_vehicle_wheels', {
    vehiclePath,
    wheelIndex: validateOptionalNumber(params.wheel_index || params.wheelIndex),
    radius: validateOptionalNumber(params.radius) ?? 35.0,
    width: validateOptionalNumber(params.width) ?? 20.0,
    mass: validateOptionalNumber(params.mass) ?? 20.0,
    frictionMultiplier: validateOptionalNumber(params.friction_multiplier || params.frictionMultiplier) ?? 1.0,
    lateralSlipAngle: validateOptionalNumber(params.lateral_slip_angle || params.lateralSlipAngle) ?? 10.0,
    longitudinalSlipRatio: validateOptionalNumber(params.longitudinal_slip_ratio || params.longitudinalSlipRatio) ?? 0.1,
    suspensionMaxRaise: validateOptionalNumber(params.suspension_max_raise || params.suspensionMaxRaise) ?? 10.0,
    suspensionMaxDrop: validateOptionalNumber(params.suspension_max_drop || params.suspensionMaxDrop) ?? 10.0,
    suspensionDampingRatio: validateOptionalNumber(params.suspension_damping_ratio || params.suspensionDampingRatio) ?? 0.5,
    springRate: validateOptionalNumber(params.spring_rate || params.springRate) ?? 250.0
  });
};

/**
 * Configure vehicle engine
 */
const handleConfigureVehicleEngine: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const vehiclePath = validateRequiredString(params.vehicle_path || params.vehiclePath, 'vehicle_path');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'configure_vehicle_engine', {
    vehiclePath,
    maxRPM: validateOptionalNumber(params.max_rpm || params.maxRPM) ?? 6000,
    idleRPM: validateOptionalNumber(params.idle_rpm || params.idleRPM) ?? 1000,
    maxTorque: validateOptionalNumber(params.max_torque || params.maxTorque) ?? 500.0,
    torqueCurve: validateOptionalArray(params.torque_curve || params.torqueCurve),
    gearRatios: validateOptionalArray(params.gear_ratios || params.gearRatios) ?? [-3.0, 3.0, 2.0, 1.5, 1.0, 0.8],
    finalGearRatio: validateOptionalNumber(params.final_gear_ratio || params.finalGearRatio) ?? 4.0,
    clutchStrength: validateOptionalNumber(params.clutch_strength || params.clutchStrength) ?? 10.0,
    differentialType: validateOptionalString(params.differential_type || params.differentialType) ?? 'limited_slip'
  });
};

/**
 * Configure Chaos cloth
 */
const handleConfigureChaosCloth: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const skeletalMeshPath = validateRequiredString(params.skeletal_mesh_path || params.skeletalMeshPath, 'skeletal_mesh_path');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'configure_chaos_cloth', {
    skeletalMeshPath,
    clothType: validateOptionalEnum(params.cloth_type || params.clothType, CLOTH_TYPES) ?? 'chaos_cloth',
    clothSection: validateOptionalNumber(params.cloth_section || params.clothSection) ?? 0,
    mass: validateOptionalNumber(params.mass) ?? 1.0,
    edgeStiffness: validateOptionalNumber(params.edge_stiffness || params.edgeStiffness) ?? 1.0,
    bendStiffness: validateOptionalNumber(params.bend_stiffness || params.bendStiffness) ?? 1.0,
    areaStiffness: validateOptionalNumber(params.area_stiffness || params.areaStiffness) ?? 1.0,
    damping: validateOptionalNumber(params.damping) ?? 0.01,
    friction: validateOptionalNumber(params.friction) ?? 0.5,
    selfCollision: validateOptionalBoolean(params.self_collision || params.selfCollision) ?? false,
    gravityScale: validateOptionalNumber(params.gravity_scale || params.gravityScale) ?? 1.0,
    windEnabled: validateOptionalBoolean(params.wind_enabled || params.windEnabled) ?? true,
    windScale: validateOptionalNumber(params.wind_scale || params.windScale) ?? 1.0
  });
};

/**
 * Create physics field
 */
const handleCreatePhysicsField: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const fieldName = validateRequiredString(params.field_name || params.fieldName, 'field_name');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'create_physics_field', {
    fieldName,
    fieldType: validateOptionalEnum(params.field_type || params.fieldType, FIELD_TYPES) ?? 'radial_force',
    magnitude: validateOptionalNumber(params.magnitude) ?? 1000.0,
    radius: validateOptionalNumber(params.radius) ?? 500.0,
    falloff: validateOptionalString(params.falloff) ?? 'linear', // 'linear', 'inverse', 'squared'
    position: validateOptionalObject(params.position),
    direction: validateOptionalObject(params.direction),
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    affectsRigidBodies: validateOptionalBoolean(params.affects_rigid_bodies || params.affectsRigidBodies) ?? true,
    affectsCloth: validateOptionalBoolean(params.affects_cloth || params.affectsCloth) ?? true,
    affectsDestructibles: validateOptionalBoolean(params.affects_destructibles || params.affectsDestructibles) ?? true
  });
};

/**
 * Setup networked physics
 */
const handleSetupNetworkedPhysics: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'setup_networked_physics', {
    actorPath,
    replicationEnabled: validateOptionalBoolean(params.replication_enabled || params.replicationEnabled) ?? true,
    predictionEnabled: validateOptionalBoolean(params.prediction_enabled || params.predictionEnabled) ?? true,
    correctionEnabled: validateOptionalBoolean(params.correction_enabled || params.correctionEnabled) ?? true,
    correctionThreshold: validateOptionalNumber(params.correction_threshold || params.correctionThreshold) ?? 10.0,
    smoothingTime: validateOptionalNumber(params.smoothing_time || params.smoothingTime) ?? 0.1,
    extrapolationEnabled: validateOptionalBoolean(params.extrapolation_enabled || params.extrapolationEnabled) ?? false,
    asyncPhysics: validateOptionalBoolean(params.async_physics || params.asyncPhysics) ?? true
  });
};

/**
 * Apply destruction damage
 */
const handleApplyDestructionDamage: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const targetActor = validateRequiredString(params.target_actor || params.targetActor, 'target_actor');

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'apply_destruction_damage', {
    targetActor,
    damageType: validateOptionalEnum(params.damage_type || params.damageType, DAMAGE_TYPES) ?? 'impact',
    damageAmount: validateOptionalNumber(params.damage_amount || params.damageAmount) ?? 100.0,
    damageRadius: validateOptionalNumber(params.damage_radius || params.damageRadius) ?? 100.0,
    damageLocation: validateOptionalObject(params.damage_location || params.damageLocation),
    impulse: validateOptionalNumber(params.impulse) ?? 1000.0,
    impulseDirection: validateOptionalObject(params.impulse_direction || params.impulseDirection)
  });
};

/**
 * Get destruction statistics
 */
const handleGetDestructionStats: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'get_destruction_stats', {
    actorName: validateOptionalString(params.actor_name || params.actorName),
    includePerformance: validateOptionalBoolean(params.include_performance || params.includePerformance) ?? true,
    includeFragmentCount: validateOptionalBoolean(params.include_fragment_count || params.includeFragmentCount) ?? true
  });
};

/**
 * List destructible assets
 */
const handleListDestructibleAssets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CHAOS, 'list_destructible_assets', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const chaosHandlers: ActionHandlerMap = {
  // Destructibles
  create_destructible: handleCreateDestructible,
  configure_destruction_levels: handleConfigureDestructionLevels,
  setup_debris_system: handleSetupDebrisSystem,
  apply_destruction_damage: handleApplyDestructionDamage,

  // Vehicles
  create_chaos_vehicle: handleCreateChaosVehicle,
  configure_vehicle_wheels: handleConfigureVehicleWheels,
  configure_vehicle_engine: handleConfigureVehicleEngine,

  // Cloth
  configure_chaos_cloth: handleConfigureChaosCloth,

  // Physics Fields
  create_physics_field: handleCreatePhysicsField,

  // Networking
  setup_networked_physics: handleSetupNetworkedPhysics,

  // Query
  get_destruction_stats: handleGetDestructionStats,
  list_destructible_assets: handleListDestructibleAssets
};

export const handleChaosTools = createCategoryHandler(Forge2Categories.CHAOS, chaosHandlers);

export default chaosHandlers;
