/**
 * FORGE2: Spawning System Handlers
 * Handles spawn points, spawn rules, wave spawning, and object pooling
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
// Spawn Point Action Handlers
// ============================================================================

/**
 * Create spawn point
 */
const handleCreateSpawnPoint: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const spawnId = validateRequiredString(params.spawn_id || params.spawnId, 'spawn_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_spawn_point', {
    spawnId,
    spawnName: validateOptionalString(params.spawn_name || params.spawnName) ?? spawnId,
    location: validateOptionalObject(params.location) ?? { x: 0, y: 0, z: 0 },
    rotation: validateOptionalObject(params.rotation) ?? { pitch: 0, yaw: 0, roll: 0 },
    spawnType: validateOptionalString(params.spawn_type || params.spawnType) ?? 'player',
    tags: validateOptionalArray(params.tags) ?? [],
    enabled: validateOptionalBoolean(params.enabled) ?? true
  });
};

/**
 * Configure spawn rules
 */
const handleConfigureSpawnRules: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const spawnId = validateRequiredString(params.spawn_id || params.spawnId, 'spawn_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'configure_spawn_rules', {
    spawnId,
    cooldown: validateOptionalNumber(params.cooldown) ?? 0,
    maxSpawns: validateOptionalNumber(params.max_spawns || params.maxSpawns) ?? -1,
    spawnRadius: validateOptionalNumber(params.spawn_radius || params.spawnRadius) ?? 0,
    checkCollision: validateOptionalBoolean(params.check_collision || params.checkCollision) ?? true,
    conditions: validateOptionalArray(params.conditions) ?? []
  });
};

/**
 * Create spawn group
 */
const handleCreateSpawnGroup: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const groupId = validateRequiredString(params.group_id || params.groupId, 'group_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_spawn_group', {
    groupId,
    groupName: validateOptionalString(params.group_name || params.groupName) ?? groupId,
    spawnPoints: validateOptionalArray(params.spawn_points || params.spawnPoints) ?? [],
    selectionMode: validateOptionalString(params.selection_mode || params.selectionMode) ?? 'random',
    enabled: validateOptionalBoolean(params.enabled) ?? true
  });
};

// ============================================================================
// Wave Spawner Action Handlers
// ============================================================================

/**
 * Create wave spawner
 */
const handleCreateWaveSpawner: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const spawnerId = validateRequiredString(params.spawner_id || params.spawnerId, 'spawner_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_wave_spawner', {
    spawnerId,
    spawnerName: validateOptionalString(params.spawner_name || params.spawnerName) ?? spawnerId,
    spawnGroup: validateOptionalString(params.spawn_group || params.spawnGroup),
    autoStart: validateOptionalBoolean(params.auto_start || params.autoStart) ?? false,
    loopWaves: validateOptionalBoolean(params.loop_waves || params.loopWaves) ?? false,
    enabled: validateOptionalBoolean(params.enabled) ?? true
  });
};

/**
 * Add spawn wave
 */
const handleAddSpawnWave: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const spawnerId = validateRequiredString(params.spawner_id || params.spawnerId, 'spawner_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'add_spawn_wave', {
    spawnerId,
    waveIndex: validateOptionalNumber(params.wave_index || params.waveIndex),
    enemyTypes: validateOptionalArray(params.enemy_types || params.enemyTypes) ?? [],
    spawnCount: validateOptionalNumber(params.spawn_count || params.spawnCount) ?? 5,
    spawnDelay: validateOptionalNumber(params.spawn_delay || params.spawnDelay) ?? 1.0,
    waveDuration: validateOptionalNumber(params.wave_duration || params.waveDuration),
    delayAfterWave: validateOptionalNumber(params.delay_after_wave || params.delayAfterWave) ?? 5.0
  });
};

/**
 * Configure wave scaling
 */
const handleConfigureWaveScaling: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const spawnerId = validateRequiredString(params.spawner_id || params.spawnerId, 'spawner_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'configure_wave_scaling', {
    spawnerId,
    scaleWithPlayers: validateOptionalBoolean(params.scale_with_players || params.scaleWithPlayers) ?? false,
    playerCountMultiplier: validateOptionalNumber(params.player_count_multiplier || params.playerCountMultiplier) ?? 0.5,
    difficultyScaling: validateOptionalBoolean(params.difficulty_scaling || params.difficultyScaling) ?? true,
    healthScalePerWave: validateOptionalNumber(params.health_scale_per_wave || params.healthScalePerWave) ?? 1.1,
    damageScalePerWave: validateOptionalNumber(params.damage_scale_per_wave || params.damageScalePerWave) ?? 1.05
  });
};

// ============================================================================
// Object Pool Action Handlers
// ============================================================================

/**
 * Create object pool
 */
const handleCreateObjectPool: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const poolId = validateRequiredString(params.pool_id || params.poolId, 'pool_id');
  const actorClass = validateRequiredString(params.actor_class || params.actorClass, 'actor_class');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_object_pool', {
    poolId,
    actorClass,
    initialSize: validateOptionalNumber(params.initial_size || params.initialSize) ?? 10,
    maxSize: validateOptionalNumber(params.max_size || params.maxSize) ?? 50,
    growSize: validateOptionalNumber(params.grow_size || params.growSize) ?? 5,
    shrinkEnabled: validateOptionalBoolean(params.shrink_enabled || params.shrinkEnabled) ?? false,
    shrinkThreshold: validateOptionalNumber(params.shrink_threshold || params.shrinkThreshold) ?? 0.25
  });
};

/**
 * Configure pool behavior
 */
const handleConfigurePoolBehavior: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const poolId = validateRequiredString(params.pool_id || params.poolId, 'pool_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'configure_pool_behavior', {
    poolId,
    prewarmOnLoad: validateOptionalBoolean(params.prewarm_on_load || params.prewarmOnLoad) ?? true,
    recycleDelay: validateOptionalNumber(params.recycle_delay || params.recycleDelay) ?? 0,
    resetOnRecycle: validateOptionalBoolean(params.reset_on_recycle || params.resetOnRecycle) ?? true,
    hideOnRecycle: validateOptionalBoolean(params.hide_on_recycle || params.hideOnRecycle) ?? true,
    disableCollisionOnRecycle: validateOptionalBoolean(params.disable_collision_on_recycle || params.disableCollisionOnRecycle) ?? true
  });
};

/**
 * Setup global pooling
 */
const handleSetupGlobalPooling: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'setup_global_pooling', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    projectilePoolSize: validateOptionalNumber(params.projectile_pool_size || params.projectilePoolSize) ?? 100,
    particlePoolSize: validateOptionalNumber(params.particle_pool_size || params.particlePoolSize) ?? 50,
    audioPoolSize: validateOptionalNumber(params.audio_pool_size || params.audioPoolSize) ?? 30,
    decalPoolSize: validateOptionalNumber(params.decal_pool_size || params.decalPoolSize) ?? 50
  });
};

// ============================================================================
// Respawn Action Handlers
// ============================================================================

/**
 * Configure respawn system
 */
const handleConfigureRespawnSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'configure_respawn_system', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    respawnDelay: validateOptionalNumber(params.respawn_delay || params.respawnDelay) ?? 3.0,
    respawnMode: validateOptionalString(params.respawn_mode || params.respawnMode) ?? 'spawn_point',
    invincibilityTime: validateOptionalNumber(params.invincibility_time || params.invincibilityTime) ?? 2.0,
    respawnLimit: validateOptionalNumber(params.respawn_limit || params.respawnLimit) ?? -1,
    penaltyOnDeath: validateOptionalObject(params.penalty_on_death || params.penaltyOnDeath)
  });
};

/**
 * Create checkpoint
 */
const handleCreateCheckpoint: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const checkpointId = validateRequiredString(params.checkpoint_id || params.checkpointId, 'checkpoint_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_checkpoint', {
    checkpointId,
    checkpointName: validateOptionalString(params.checkpoint_name || params.checkpointName) ?? checkpointId,
    location: validateOptionalObject(params.location) ?? { x: 0, y: 0, z: 0 },
    rotation: validateOptionalObject(params.rotation) ?? { pitch: 0, yaw: 0, roll: 0 },
    autoActivate: validateOptionalBoolean(params.auto_activate || params.autoActivate) ?? true,
    activationRadius: validateOptionalNumber(params.activation_radius || params.activationRadius) ?? 200,
    saveGameState: validateOptionalBoolean(params.save_game_state || params.saveGameState) ?? false
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get spawn system info
 */
const handleGetSpawnSystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'get_spawn_system_info', {
    includeSpawnPoints: validateOptionalBoolean(params.include_spawn_points || params.includeSpawnPoints) ?? true,
    includeWaveSpawners: validateOptionalBoolean(params.include_wave_spawners || params.includeWaveSpawners) ?? true,
    includePools: validateOptionalBoolean(params.include_pools || params.includePools) ?? true
  });
};

/**
 * List spawn points
 */
const handleListSpawnPoints: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'list_spawn_points', {
    spawnType: validateOptionalString(params.spawn_type || params.spawnType),
    enabledOnly: validateOptionalBoolean(params.enabled_only || params.enabledOnly) ?? false,
    withTag: validateOptionalString(params.with_tag || params.withTag)
  });
};

/**
 * Get pool statistics
 */
const handleGetPoolStatistics: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'get_pool_statistics', {
    poolId: validateOptionalString(params.pool_id || params.poolId),
    includeUsage: validateOptionalBoolean(params.include_usage || params.includeUsage) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const spawningHandlers: ActionHandlerMap = {
  // Spawn Points
  create_spawn_point: handleCreateSpawnPoint,
  configure_spawn_rules: handleConfigureSpawnRules,
  create_spawn_group: handleCreateSpawnGroup,

  // Wave Spawner
  create_wave_spawner: handleCreateWaveSpawner,
  add_spawn_wave: handleAddSpawnWave,
  configure_wave_scaling: handleConfigureWaveScaling,

  // Object Pool
  create_object_pool: handleCreateObjectPool,
  configure_pool_behavior: handleConfigurePoolBehavior,
  setup_global_pooling: handleSetupGlobalPooling,

  // Respawn
  configure_respawn_system: handleConfigureRespawnSystem,
  create_checkpoint: handleCreateCheckpoint,

  // Query
  get_spawn_system_info: handleGetSpawnSystemInfo,
  list_spawn_points: handleListSpawnPoints,
  get_pool_statistics: handleGetPoolStatistics
};

export const handleSpawningTools = createCategoryHandler(Forge2Categories.LEVEL, spawningHandlers);

export default spawningHandlers;
