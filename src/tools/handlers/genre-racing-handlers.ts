/**
 * FORGE2: Racing Genre Handlers
 * Complete racing/driving game mechanics
 *
 * Supports: Cars, bikes, boats, aircraft
 * Vehicle physics: Arcade vs simulation
 * Race modes: Time trial, circuit, elimination, drift
 * Systems: Tires, damage, powerups, AI racers, leaderboards
 */

import { ITools } from '../../types/tool-interfaces.js';
import type { HandlerArgs } from '../../types/handler-types.js';
import { cleanObject } from '../../utils/safe-json.js';
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

function validateOptionalEnum<T extends string>(
  value: unknown,
  allowedValues: readonly T[],
  fieldName: string
): T | undefined {
  if (value === undefined || value === null || value === '') {
    return undefined;
  }
  if (typeof value !== 'string' || !allowedValues.includes(value as T)) {
    throw new Error(`${fieldName} must be one of: ${allowedValues.join(', ')}`);
  }
  return value as T;
}

function validateRequiredEnum<T extends string>(
  value: unknown,
  allowedValues: readonly T[],
  fieldName: string
): T {
  if (value === undefined || value === null || value === '') {
    throw new Error(`${fieldName} is required`);
  }
  if (typeof value !== 'string' || !allowedValues.includes(value as T)) {
    throw new Error(`${fieldName} must be one of: ${allowedValues.join(', ')}`);
  }
  return value as T;
}

// ============================================================================
// Types
// ============================================================================

export type VehicleType = 'car' | 'bike' | 'boat' | 'aircraft' | 'hovercraft' | 'kart';
export type PhysicsPreset = 'arcade' | 'simcade' | 'simulation';
export type TireCompound = 'soft' | 'medium' | 'hard' | 'wet' | 'intermediate';
export type DamageType = 'visual' | 'mechanical' | 'both' | 'none';
export type RaceMode = 'time_trial' | 'circuit' | 'sprint' | 'elimination' | 'drift' | 'drag' | 'rally';
export type PowerupType = 'boost' | 'shield' | 'weapon' | 'slowdown' | 'repair';

// ============================================================================
// Constants
// ============================================================================

const VEHICLE_TYPES = ['car', 'bike', 'boat', 'aircraft', 'hovercraft', 'kart'] as const;
const PHYSICS_PRESETS = ['arcade', 'simcade', 'simulation'] as const;
const TIRE_COMPOUNDS = ['soft', 'medium', 'hard', 'wet', 'intermediate'] as const;
const DAMAGE_TYPES = ['visual', 'mechanical', 'both', 'none'] as const;
const RACE_MODES = ['time_trial', 'circuit', 'sprint', 'elimination', 'drift', 'drag', 'rally'] as const;
const POWERUP_TYPES = ['boost', 'shield', 'weapon', 'slowdown', 'repair'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a vehicle
 */
const handleCreateVehicle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const vehicleId = validateRequiredString(params.vehicle_id || params.vehicleId, 'vehicle_id');
  const vehicleName = validateRequiredString(params.vehicle_name || params.vehicleName, 'vehicle_name');

  const vehicleType = validateRequiredEnum(
    params.vehicle_type || params.vehicleType,
    VEHICLE_TYPES,
    'vehicle_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'create_vehicle', {
    vehicleId,
    vehicleName,
    vehicleType,
    meshPath: params.mesh_path || params.meshPath,
    skeletonPath: params.skeleton_path || params.skeletonPath,
    baseTopSpeed: params.base_top_speed ?? params.baseTopSpeed ?? 200,
    baseAcceleration: params.base_acceleration ?? params.baseAcceleration ?? 10,
    baseBraking: params.base_braking ?? params.baseBraking ?? 15,
    baseHandling: params.base_handling ?? params.baseHandling ?? 5,
    mass: params.mass ?? 1500,
    wheelCount: params.wheel_count ?? params.wheelCount ?? 4,
    driveType: params.drive_type || params.driveType || 'rwd',
    category: params.category as string | undefined
  });
};

/**
 * Configure vehicle physics
 */
const handleConfigureVehiclePhysics: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const vehicleId = validateRequiredString(params.vehicle_id || params.vehicleId, 'vehicle_id');

  const physicsPreset = validateOptionalEnum(
    params.physics_preset || params.physicsPreset,
    PHYSICS_PRESETS,
    'physics_preset'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'configure_vehicle_physics', {
    vehicleId,
    physicsPreset: physicsPreset || 'arcade',
    accelerationCurve: params.acceleration_curve || params.accelerationCurve,
    brakingForce: params.braking_force ?? params.brakingForce ?? 3000,
    steeringAngle: params.steering_angle ?? params.steeringAngle ?? 35,
    steeringSpeed: params.steering_speed ?? params.steeringSpeed ?? 5,
    downforce: params.downforce ?? 0,
    dragCoefficient: params.drag_coefficient ?? params.dragCoefficient ?? 0.3,
    centerOfMass: params.center_of_mass || params.centerOfMass,
    weightDistribution: params.weight_distribution ?? params.weightDistribution ?? 0.5,
    antiRollBar: params.anti_roll_bar ?? params.antiRollBar ?? 0.5
  });
};

/**
 * Configure tire system
 */
const handleConfigureTireSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const vehicleId = validateRequiredString(params.vehicle_id || params.vehicleId, 'vehicle_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'configure_tire_system', {
    vehicleId,
    enableTireWear: params.enable_tire_wear ?? params.enableTireWear ?? false,
    enableTireTemperature: params.enable_tire_temperature ?? params.enableTireTemperature ?? false,
    gripMultiplier: params.grip_multiplier ?? params.gripMultiplier ?? 1.0,
    lateralGrip: params.lateral_grip ?? params.lateralGrip ?? 1.0,
    longitudinalGrip: params.longitudinal_grip ?? params.longitudinalGrip ?? 1.0,
    optimalTemperature: params.optimal_temperature ?? params.optimalTemperature ?? 90,
    wearRate: params.wear_rate ?? params.wearRate ?? 0.01,
    compounds: params.compounds as string[] | undefined
  });
};

/**
 * Configure tire compound
 */
const handleConfigureTireCompound: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const compound = validateRequiredEnum(
    params.compound,
    TIRE_COMPOUNDS,
    'compound'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'configure_tire_compound', {
    compound,
    gripBonus: params.grip_bonus ?? params.gripBonus ?? 0,
    wearMultiplier: params.wear_multiplier ?? params.wearMultiplier ?? 1.0,
    temperatureRange: params.temperature_range || params.temperatureRange,
    wetPerformance: params.wet_performance ?? params.wetPerformance ?? 0.5
  });
};

/**
 * Setup vehicle damage system
 */
const handleSetupVehicleDamage: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const vehicleId = validateRequiredString(params.vehicle_id || params.vehicleId, 'vehicle_id');

  const damageType = validateOptionalEnum(
    params.damage_type || params.damageType,
    DAMAGE_TYPES,
    'damage_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'setup_vehicle_damage', {
    vehicleId,
    damageType: damageType || 'visual',
    enableDeformation: params.enable_deformation ?? params.enableDeformation ?? true,
    deformationIntensity: params.deformation_intensity ?? params.deformationIntensity ?? 1.0,
    engineDamageThreshold: params.engine_damage_threshold ?? params.engineDamageThreshold ?? 0.7,
    suspensionDamageThreshold: params.suspension_damage_threshold ?? params.suspensionDamageThreshold ?? 0.5,
    tirePunctureChance: params.tire_puncture_chance ?? params.tirePunctureChance ?? 0.1,
    speedPenaltyPerDamage: params.speed_penalty_per_damage ?? params.speedPenaltyPerDamage ?? 0.1,
    handlingPenaltyPerDamage: params.handling_penalty_per_damage ?? params.handlingPenaltyPerDamage ?? 0.2
  });
};

/**
 * Create race track
 */
const handleCreateTrack: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const trackId = validateRequiredString(params.track_id || params.trackId, 'track_id');
  const trackName = validateRequiredString(params.track_name || params.trackName, 'track_name');

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'create_track', {
    trackId,
    trackName,
    splinePath: params.spline_path || params.splinePath,
    trackWidth: params.track_width ?? params.trackWidth ?? 12,
    lapCount: params.lap_count ?? params.lapCount ?? 3,
    checkpointCount: params.checkpoint_count ?? params.checkpointCount ?? 10,
    startGridSize: params.start_grid_size ?? params.startGridSize ?? 20,
    includePitLane: params.include_pit_lane ?? params.includePitLane ?? false,
    timeOfDay: params.time_of_day || params.timeOfDay,
    weatherPreset: params.weather_preset || params.weatherPreset,
    surface: params.surface || 'asphalt'
  });
};

/**
 * Add track checkpoint
 */
const handleAddTrackCheckpoint: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const trackId = validateRequiredString(params.track_id || params.trackId, 'track_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'add_track_checkpoint', {
    trackId,
    location: params.location,
    rotation: params.rotation,
    width: params.width ?? 15,
    isStartFinish: params.is_start_finish ?? params.isStartFinish ?? false,
    sectorNumber: params.sector_number ?? params.sectorNumber
  });
};

/**
 * Setup race mode
 */
const handleSetupRaceMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const raceMode = validateRequiredEnum(
    params.race_mode || params.raceMode,
    RACE_MODES,
    'race_mode'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'setup_race_mode', {
    raceMode,
    lapCount: params.lap_count ?? params.lapCount ?? 3,
    timeLimit: params.time_limit ?? params.timeLimit,
    eliminationInterval: params.elimination_interval ?? params.eliminationInterval ?? 30,
    driftScoreMultiplier: params.drift_score_multiplier ?? params.driftScoreMultiplier ?? 1.0,
    ghostEnabled: params.ghost_enabled ?? params.ghostEnabled ?? true,
    collisionEnabled: params.collision_enabled ?? params.collisionEnabled ?? true,
    catchUpEnabled: params.catch_up_enabled ?? params.catchUpEnabled ?? false,
    slipstreamEnabled: params.slipstream_enabled ?? params.slipstreamEnabled ?? true
  });
};

/**
 * Create powerup system (arcade)
 */
const handleCreatePowerupSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'create_powerup_system', {
    enabled: params.enabled ?? true,
    spawnInterval: params.spawn_interval ?? params.spawnInterval ?? 15,
    maxActivePerPlayer: params.max_active_per_player ?? params.maxActivePerPlayer ?? 1,
    powerupDuration: params.powerup_duration ?? params.powerupDuration ?? 5,
    respawnTime: params.respawn_time ?? params.respawnTime ?? 10
  });
};

/**
 * Add powerup type
 */
const handleAddPowerupType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const powerupType = validateRequiredEnum(
    params.powerup_type || params.powerupType,
    POWERUP_TYPES,
    'powerup_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'add_powerup_type', {
    powerupType,
    duration: params.duration ?? 5,
    effectStrength: params.effect_strength ?? params.effectStrength ?? 1.0,
    spawnWeight: params.spawn_weight ?? params.spawnWeight ?? 1.0,
    meshPath: params.mesh_path || params.meshPath,
    iconPath: params.icon_path || params.iconPath,
    sfxPath: params.sfx_path || params.sfxPath
  });
};

/**
 * Configure AI racers
 */
const handleConfigureAIRacers: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'configure_ai_racers', {
    difficultyLevel: params.difficulty_level ?? params.difficultyLevel ?? 'medium',
    rubberBandingEnabled: params.rubber_banding_enabled ?? params.rubberBandingEnabled ?? true,
    rubberBandingStrength: params.rubber_banding_strength ?? params.rubberBandingStrength ?? 0.5,
    aggressionLevel: params.aggression_level ?? params.aggressionLevel ?? 0.5,
    mistakeFrequency: params.mistake_frequency ?? params.mistakeFrequency ?? 0.1,
    racingLineAdherence: params.racing_line_adherence ?? params.racingLineAdherence ?? 0.8,
    overtakingChance: params.overtaking_chance ?? params.overtakingChance ?? 0.5,
    defensiveChance: params.defensive_chance ?? params.defensiveChance ?? 0.5
  });
};

/**
 * Create AI racer profile
 */
const handleCreateAIRacerProfile: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const profileId = validateRequiredString(params.profile_id || params.profileId, 'profile_id');
  const profileName = validateRequiredString(params.profile_name || params.profileName, 'profile_name');

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'create_ai_racer_profile', {
    profileId,
    profileName,
    skill: params.skill ?? 0.7,
    aggression: params.aggression ?? 0.5,
    consistency: params.consistency ?? 0.8,
    wetSkill: params.wet_skill ?? params.wetSkill ?? 0.6,
    tireManagement: params.tire_management ?? params.tireManagement ?? 0.5,
    preferredVehicleId: params.preferred_vehicle_id || params.preferredVehicleId
  });
};

/**
 * Setup leaderboards
 */
const handleSetupLeaderboards: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const trackId = validateRequiredString(params.track_id || params.trackId, 'track_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'setup_leaderboards', {
    trackId,
    enableGhostData: params.enable_ghost_data ?? params.enableGhostData ?? true,
    enableLapTimes: params.enable_lap_times ?? params.enableLapTimes ?? true,
    enableTotalTimes: params.enable_total_times ?? params.enableTotalTimes ?? true,
    enableDriftScores: params.enable_drift_scores ?? params.enableDriftScores ?? false,
    maxEntriesPerPlayer: params.max_entries_per_player ?? params.maxEntriesPerPlayer ?? 10,
    ghostDataMaxSize: params.ghost_data_max_size ?? params.ghostDataMaxSize ?? 1024
  });
};

/**
 * Configure suspension
 */
const handleConfigureSuspension: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const vehicleId = validateRequiredString(params.vehicle_id || params.vehicleId, 'vehicle_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'configure_suspension', {
    vehicleId,
    frontSpringRate: params.front_spring_rate ?? params.frontSpringRate ?? 50000,
    rearSpringRate: params.rear_spring_rate ?? params.rearSpringRate ?? 45000,
    frontDamping: params.front_damping ?? params.frontDamping ?? 4000,
    rearDamping: params.rear_damping ?? params.rearDamping ?? 4000,
    frontRideHeight: params.front_ride_height ?? params.frontRideHeight ?? 0.15,
    rearRideHeight: params.rear_ride_height ?? params.rearRideHeight ?? 0.15,
    maxCompression: params.max_compression ?? params.maxCompression ?? 0.1,
    maxDroop: params.max_droop ?? params.maxDroop ?? 0.1
  });
};

/**
 * Configure transmission
 */
const handleConfigureTransmission: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const vehicleId = validateRequiredString(params.vehicle_id || params.vehicleId, 'vehicle_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'configure_transmission', {
    vehicleId,
    transmissionType: params.transmission_type || params.transmissionType || 'automatic',
    gearCount: params.gear_count ?? params.gearCount ?? 6,
    gearRatios: params.gear_ratios || params.gearRatios,
    finalDriveRatio: params.final_drive_ratio ?? params.finalDriveRatio ?? 3.5,
    shiftTime: params.shift_time ?? params.shiftTime ?? 0.2,
    autoShiftUp: params.auto_shift_up ?? params.autoShiftUp ?? 0.9,
    autoShiftDown: params.auto_shift_down ?? params.autoShiftDown ?? 0.3,
    clutchStrength: params.clutch_strength ?? params.clutchStrength ?? 1.0
  });
};

/**
 * Record lap time
 */
const handleRecordLapTime: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const trackId = validateRequiredString(params.track_id || params.trackId, 'track_id');

  const lapTime = params.lap_time || params.lapTime;
  if (typeof lapTime !== 'number' || lapTime <= 0) {
    throw new Error('lap_time is required and must be a positive number');
  }

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'record_lap_time', {
    trackId,
    lapTime,
    playerId: params.player_id || params.playerId,
    vehicleId: params.vehicle_id || params.vehicleId,
    ghostData: params.ghost_data || params.ghostData,
    conditions: params.conditions
  });
};

/**
 * Get leaderboard entries
 */
const handleGetLeaderboard: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const trackId = validateRequiredString(params.track_id || params.trackId, 'track_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'get_leaderboard', {
    trackId,
    leaderboardType: params.leaderboard_type || params.leaderboardType || 'lap_time',
    limit: params.limit ?? 100,
    offset: params.offset ?? 0,
    vehicleFilter: params.vehicle_filter || params.vehicleFilter,
    includeGhostData: params.include_ghost_data ?? params.includeGhostData ?? false
  });
};

/**
 * List all vehicles
 */
const handleListVehicles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const vehicleType = validateOptionalEnum(
    params.vehicle_type || params.vehicleType,
    VEHICLE_TYPES,
    'vehicle_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'list_vehicles', {
    vehicleType,
    category: params.category as string | undefined
  });
};

/**
 * List all tracks
 */
const handleListTracks: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RACING, 'list_tracks', {
    surface: params.surface as string | undefined,
    minLapCount: params.min_lap_count ?? params.minLapCount,
    maxLapCount: params.max_lap_count ?? params.maxLapCount
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const genreRacingHandlers: ActionHandlerMap = {
  create_vehicle: handleCreateVehicle,
  configure_vehicle_physics: handleConfigureVehiclePhysics,
  configure_tire_system: handleConfigureTireSystem,
  configure_tire_compound: handleConfigureTireCompound,
  setup_vehicle_damage: handleSetupVehicleDamage,
  create_track: handleCreateTrack,
  add_track_checkpoint: handleAddTrackCheckpoint,
  setup_race_mode: handleSetupRaceMode,
  create_powerup_system: handleCreatePowerupSystem,
  add_powerup_type: handleAddPowerupType,
  configure_ai_racers: handleConfigureAIRacers,
  create_ai_racer_profile: handleCreateAIRacerProfile,
  setup_leaderboards: handleSetupLeaderboards,
  configure_suspension: handleConfigureSuspension,
  configure_transmission: handleConfigureTransmission,
  record_lap_time: handleRecordLapTime,
  get_leaderboard: handleGetLeaderboard,
  list_vehicles: handleListVehicles,
  list_tracks: handleListTracks
};

/**
 * Main genre racing handler - routes to specific action handlers
 */
export const handleGenreRacingTools = createCategoryHandler(Forge2Categories.GENRE_RACING, genreRacingHandlers);

/**
 * Get available genre racing actions
 */
export function getGenreRacingActions(): string[] {
  return Object.keys(genreRacingHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeGenreRacingHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = genreRacingHandlers[action];
  if (!handler) {
    throw new Error(`Unknown genre racing action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
