/**
 * FORGE2: Platformer Genre Handlers
 * Complete platformer mechanics (Mario, Celeste, Hollow Knight style)
 *
 * Supports: 2D and 3D platformer controllers
 * Movement: Run, jump, wall jump, dash, variable height
 * Systems: Platforms, collectibles, checkpoints, hazards, powerups
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

export type PlatformerStyle = '2d_sidescroller' | '2d_topdown' | '3d_platformer' | '3d_collectathon';
export type PlatformType = 'static' | 'moving' | 'falling' | 'bouncy' | 'ice' | 'conveyor' | 'oneway' | 'crumbling' | 'rotating';
export type HazardType = 'spike' | 'pit' | 'enemy' | 'fire' | 'water' | 'saw' | 'laser' | 'crusher' | 'projectile';
export type PowerupType = 'double_jump' | 'wall_jump' | 'dash' | 'glide' | 'shrink' | 'grow' | 'invincibility' | 'speed';
export type CollectibleType = 'coin' | 'gem' | 'star' | 'key' | 'heart' | 'custom';

// ============================================================================
// Constants
// ============================================================================

const PLATFORMER_STYLES = ['2d_sidescroller', '2d_topdown', '3d_platformer', '3d_collectathon'] as const;
const PLATFORM_TYPES = ['static', 'moving', 'falling', 'bouncy', 'ice', 'conveyor', 'oneway', 'crumbling', 'rotating'] as const;
const HAZARD_TYPES = ['spike', 'pit', 'enemy', 'fire', 'water', 'saw', 'laser', 'crusher', 'projectile'] as const;
const POWERUP_TYPES = ['double_jump', 'wall_jump', 'dash', 'glide', 'shrink', 'grow', 'invincibility', 'speed'] as const;
const COLLECTIBLE_TYPES = ['coin', 'gem', 'star', 'key', 'heart', 'custom'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create 2D player controller
 */
const handleCreatePlayerController2D: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'create_player_controller_2d', {
    moveSpeed: params.move_speed ?? params.moveSpeed ?? 400,
    acceleration: params.acceleration ?? 2000,
    deceleration: params.deceleration ?? 2500,
    airControl: params.air_control ?? params.airControl ?? 0.8,
    jumpForce: params.jump_force ?? params.jumpForce ?? 600,
    gravity: params.gravity ?? 1500,
    terminalVelocity: params.terminal_velocity ?? params.terminalVelocity ?? 1200,
    enableWallSlide: params.enable_wall_slide ?? params.enableWallSlide ?? false,
    wallSlideSpeed: params.wall_slide_speed ?? params.wallSlideSpeed ?? 100,
    enableWallJump: params.enable_wall_jump ?? params.enableWallJump ?? false,
    enableDash: params.enable_dash ?? params.enableDash ?? false,
    dashSpeed: params.dash_speed ?? params.dashSpeed ?? 800,
    dashDuration: params.dash_duration ?? params.dashDuration ?? 0.15
  });
};

/**
 * Create 3D player controller
 */
const handleCreatePlayerController3D: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const style = validateOptionalEnum(
    params.style,
    PLATFORMER_STYLES,
    'style'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'create_player_controller_3d', {
    style: style || '3d_platformer',
    moveSpeed: params.move_speed ?? params.moveSpeed ?? 600,
    rotationSpeed: params.rotation_speed ?? params.rotationSpeed ?? 540,
    acceleration: params.acceleration ?? 2000,
    deceleration: params.deceleration ?? 3000,
    airControl: params.air_control ?? params.airControl ?? 0.4,
    jumpForce: params.jump_force ?? params.jumpForce ?? 700,
    gravity: params.gravity ?? 2000,
    enableLongJump: params.enable_long_jump ?? params.enableLongJump ?? false,
    enableBackflip: params.enable_backflip ?? params.enableBackflip ?? false,
    enableSideFlip: params.enable_side_flip ?? params.enableSideFlip ?? false,
    enableGroundPound: params.enable_ground_pound ?? params.enableGroundPound ?? false,
    enableTripleJump: params.enable_triple_jump ?? params.enableTripleJump ?? false
  });
};

/**
 * Configure jump physics
 */
const handleConfigureJumpPhysics: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'configure_jump_physics', {
    coyoteTime: params.coyote_time ?? params.coyoteTime ?? 0.1,
    jumpBufferTime: params.jump_buffer_time ?? params.jumpBufferTime ?? 0.15,
    variableJumpHeight: params.variable_jump_height ?? params.variableJumpHeight ?? true,
    minJumpMultiplier: params.min_jump_multiplier ?? params.minJumpMultiplier ?? 0.4,
    maxJumpCount: params.max_jump_count ?? params.maxJumpCount ?? 1,
    jumpCutMultiplier: params.jump_cut_multiplier ?? params.jumpCutMultiplier ?? 0.5,
    apexHangTime: params.apex_hang_time ?? params.apexHangTime ?? 0,
    apexGravityMultiplier: params.apex_gravity_multiplier ?? params.apexGravityMultiplier ?? 0.5,
    fallingGravityMultiplier: params.falling_gravity_multiplier ?? params.fallingGravityMultiplier ?? 1.5
  });
};

/**
 * Create platform type
 */
const handleCreatePlatformType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const platformType = validateRequiredEnum(
    params.platform_type || params.platformType,
    PLATFORM_TYPES,
    'platform_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'create_platform_type', {
    platformType,
    meshPath: params.mesh_path || params.meshPath,
    materialPath: params.material_path || params.materialPath,
    friction: params.friction ?? 1.0,
    bounceForce: params.bounce_force ?? params.bounceForce ?? 0,
    moveSpeed: params.move_speed ?? params.moveSpeed ?? 100,
    movePath: params.move_path || params.movePath,
    fallDelay: params.fall_delay ?? params.fallDelay ?? 0.5,
    crumbleTime: params.crumble_time ?? params.crumbleTime ?? 1.0,
    respawnTime: params.respawn_time ?? params.respawnTime ?? 3.0,
    conveyorSpeed: params.conveyor_speed ?? params.conveyorSpeed ?? 200,
    conveyorDirection: params.conveyor_direction || params.conveyorDirection,
    rotationSpeed: params.rotation_speed ?? params.rotationSpeed ?? 30
  });
};

/**
 * Place platform
 */
const handlePlacePlatform: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const platformType = validateRequiredEnum(
    params.platform_type || params.platformType,
    PLATFORM_TYPES,
    'platform_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'place_platform', {
    platformType,
    location: params.location,
    rotation: params.rotation,
    scale: params.scale,
    movePath: params.move_path || params.movePath,
    moveSpeed: params.move_speed ?? params.moveSpeed,
    startDelay: params.start_delay ?? params.startDelay ?? 0,
    pingPong: params.ping_pong ?? params.pingPong ?? true
  });
};

/**
 * Setup collectibles
 */
const handleSetupCollectibles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const collectibleType = validateRequiredEnum(
    params.collectible_type || params.collectibleType,
    COLLECTIBLE_TYPES,
    'collectible_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'setup_collectibles', {
    collectibleType,
    value: params.value ?? 1,
    meshPath: params.mesh_path || params.meshPath,
    iconPath: params.icon_path || params.iconPath,
    collectSound: params.collect_sound || params.collectSound,
    collectVFX: params.collect_vfx || params.collectVFX,
    floatAnimation: params.float_animation ?? params.floatAnimation ?? true,
    rotateSpeed: params.rotate_speed ?? params.rotateSpeed ?? 90,
    magnetRange: params.magnet_range ?? params.magnetRange ?? 0,
    respawns: params.respawns ?? false,
    respawnTime: params.respawn_time ?? params.respawnTime ?? 0
  });
};

/**
 * Place collectible
 */
const handlePlaceCollectible: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const collectibleType = validateRequiredEnum(
    params.collectible_type || params.collectibleType,
    COLLECTIBLE_TYPES,
    'collectible_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'place_collectible', {
    collectibleType,
    location: params.location,
    value: params.value,
    uniqueId: params.unique_id || params.uniqueId
  });
};

/**
 * Create checkpoint system
 */
const handleCreateCheckpointSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'create_checkpoint_system', {
    autoActivate: params.auto_activate ?? params.autoActivate ?? true,
    activationRadius: params.activation_radius ?? params.activationRadius ?? 100,
    saveCollectibles: params.save_collectibles ?? params.saveCollectibles ?? false,
    saveProgress: params.save_progress ?? params.saveProgress ?? true,
    visualFeedback: params.visual_feedback ?? params.visualFeedback ?? true,
    activatedMeshPath: params.activated_mesh_path || params.activatedMeshPath,
    inactiveMeshPath: params.inactive_mesh_path || params.inactiveMeshPath,
    activationSound: params.activation_sound || params.activationSound,
    activationVFX: params.activation_vfx || params.activationVFX
  });
};

/**
 * Place checkpoint
 */
const handlePlaceCheckpoint: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const checkpointId = validateRequiredString(params.checkpoint_id || params.checkpointId, 'checkpoint_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'place_checkpoint', {
    checkpointId,
    location: params.location,
    rotation: params.rotation,
    respawnOffset: params.respawn_offset || params.respawnOffset,
    isStartPoint: params.is_start_point ?? params.isStartPoint ?? false,
    isEndPoint: params.is_end_point ?? params.isEndPoint ?? false
  });
};

/**
 * Configure hazards
 */
const handleConfigureHazards: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const hazardType = validateRequiredEnum(
    params.hazard_type || params.hazardType,
    HAZARD_TYPES,
    'hazard_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'configure_hazards', {
    hazardType,
    damage: params.damage ?? 1,
    instantKill: params.instant_kill ?? params.instantKill ?? false,
    knockback: params.knockback ?? 200,
    knockbackDirection: params.knockback_direction || params.knockbackDirection,
    invincibilityTime: params.invincibility_time ?? params.invincibilityTime ?? 1.0,
    meshPath: params.mesh_path || params.meshPath,
    hitSound: params.hit_sound || params.hitSound,
    hitVFX: params.hit_vfx || params.hitVFX
  });
};

/**
 * Place hazard
 */
const handlePlaceHazard: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const hazardType = validateRequiredEnum(
    params.hazard_type || params.hazardType,
    HAZARD_TYPES,
    'hazard_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'place_hazard', {
    hazardType,
    location: params.location,
    rotation: params.rotation,
    scale: params.scale,
    movePath: params.move_path || params.movePath,
    activationDelay: params.activation_delay ?? params.activationDelay,
    activeTime: params.active_time ?? params.activeTime,
    inactiveTime: params.inactive_time ?? params.inactiveTime
  });
};

/**
 * Setup powerups
 */
const handleSetupPowerups: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const powerupType = validateRequiredEnum(
    params.powerup_type || params.powerupType,
    POWERUP_TYPES,
    'powerup_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'setup_powerups', {
    powerupType,
    duration: params.duration ?? 10,
    permanent: params.permanent ?? false,
    stackable: params.stackable ?? false,
    meshPath: params.mesh_path || params.meshPath,
    iconPath: params.icon_path || params.iconPath,
    collectSound: params.collect_sound || params.collectSound,
    activeVFX: params.active_vfx || params.activeVFX,
    effectStrength: params.effect_strength ?? params.effectStrength ?? 1.0
  });
};

/**
 * Place powerup
 */
const handlePlacePowerup: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const powerupType = validateRequiredEnum(
    params.powerup_type || params.powerupType,
    POWERUP_TYPES,
    'powerup_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'place_powerup', {
    powerupType,
    location: params.location,
    respawns: params.respawns ?? true,
    respawnTime: params.respawn_time ?? params.respawnTime ?? 30
  });
};

/**
 * Configure death and respawn
 */
const handleConfigureDeathRespawn: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'configure_death_respawn', {
    lives: params.lives ?? -1,
    deathDelay: params.death_delay ?? params.deathDelay ?? 1.0,
    respawnDelay: params.respawn_delay ?? params.respawnDelay ?? 0.5,
    resetCollectibles: params.reset_collectibles ?? params.resetCollectibles ?? false,
    resetEnemies: params.reset_enemies ?? params.resetEnemies ?? true,
    invincibilityOnRespawn: params.invincibility_on_respawn ?? params.invincibilityOnRespawn ?? 2.0,
    deathAnimation: params.death_animation || params.deathAnimation,
    deathSound: params.death_sound || params.deathSound,
    respawnVFX: params.respawn_vfx || params.respawnVFX
  });
};

/**
 * Setup level completion
 */
const handleSetupLevelCompletion: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'setup_level_completion', {
    goalType: params.goal_type || params.goalType || 'reach_end',
    requiredCollectibles: params.required_collectibles ?? params.requiredCollectibles ?? 0,
    requiredPercentage: params.required_percentage ?? params.requiredPercentage,
    timeLimit: params.time_limit ?? params.timeLimit,
    bonusObjectives: params.bonus_objectives || params.bonusObjectives,
    completionAnimation: params.completion_animation || params.completionAnimation,
    completionSound: params.completion_sound || params.completionSound,
    nextLevel: params.next_level || params.nextLevel
  });
};

/**
 * Get level statistics
 */
const handleGetLevelStats: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const levelId = validateRequiredString(params.level_id || params.levelId, 'level_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'get_level_stats', {
    levelId,
    playerId: params.player_id || params.playerId
  });
};

/**
 * List platform types
 */
const handleListPlatformTypes: ActionHandler = async (tools: ITools, _args: HandlerArgs) => {
  return executeForge2Action(tools, Forge2Categories.GENRE_PLATFORMER, 'list_platform_types', {});
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const genrePlatformerHandlers: ActionHandlerMap = {
  create_player_controller_2d: handleCreatePlayerController2D,
  create_player_controller_3d: handleCreatePlayerController3D,
  configure_jump_physics: handleConfigureJumpPhysics,
  create_platform_type: handleCreatePlatformType,
  place_platform: handlePlacePlatform,
  setup_collectibles: handleSetupCollectibles,
  place_collectible: handlePlaceCollectible,
  create_checkpoint_system: handleCreateCheckpointSystem,
  place_checkpoint: handlePlaceCheckpoint,
  configure_hazards: handleConfigureHazards,
  place_hazard: handlePlaceHazard,
  setup_powerups: handleSetupPowerups,
  place_powerup: handlePlacePowerup,
  configure_death_respawn: handleConfigureDeathRespawn,
  setup_level_completion: handleSetupLevelCompletion,
  get_level_stats: handleGetLevelStats,
  list_platform_types: handleListPlatformTypes
};

/**
 * Main genre platformer handler - routes to specific action handlers
 */
export const handleGenrePlatformerTools = createCategoryHandler(Forge2Categories.GENRE_PLATFORMER, genrePlatformerHandlers);

/**
 * Get available genre platformer actions
 */
export function getGenrePlatformerActions(): string[] {
  return Object.keys(genrePlatformerHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeGenrePlatformerHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = genrePlatformerHandlers[action];
  if (!handler) {
    throw new Error(`Unknown genre platformer action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
