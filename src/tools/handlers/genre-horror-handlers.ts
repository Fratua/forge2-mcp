/**
 * FORGE2: Horror Genre Handlers
 * Complete horror game mechanics (Resident Evil, Silent Hill, Amnesia style)
 *
 * Supports: Fear systems, sanity meters, monster AI
 * Systems: Atmosphere, resource scarcity, save rooms, jumpscares
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

export type FearEffectType = 'visual' | 'audio' | 'control' | 'hallucination' | 'camera';
export type MonsterBehavior = 'patrol' | 'stalk' | 'hunt' | 'guard' | 'scripted' | 'random';
export type ScareType = 'jumpscare' | 'ambient' | 'chase' | 'reveal' | 'environmental';
export type AtmosphereType = 'tense' | 'calm' | 'chase' | 'safe' | 'otherworld';
export type SaveType = 'save_room' | 'checkpoint' | 'typewriter' | 'unlimited';

// ============================================================================
// Constants
// ============================================================================

const FEAR_EFFECT_TYPES = ['visual', 'audio', 'control', 'hallucination', 'camera'] as const;
const MONSTER_BEHAVIORS = ['patrol', 'stalk', 'hunt', 'guard', 'scripted', 'random'] as const;
const SCARE_TYPES = ['jumpscare', 'ambient', 'chase', 'reveal', 'environmental'] as const;
const ATMOSPHERE_TYPES = ['tense', 'calm', 'chase', 'safe', 'otherworld'] as const;
const SAVE_TYPES = ['save_room', 'checkpoint', 'typewriter', 'unlimited'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create fear system
 */
const handleCreateFearSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'create_fear_system', {
    maxFear: params.max_fear ?? params.maxFear ?? 100,
    startFear: params.start_fear ?? params.startFear ?? 0,
    fearDecayRate: params.fear_decay_rate ?? params.fearDecayRate ?? 0.5,
    fearDecayDelay: params.fear_decay_delay ?? params.fearDecayDelay ?? 5,
    lightReducesFear: params.light_reduces_fear ?? params.lightReducesFear ?? true,
    safeZonesReduceFear: params.safe_zones_reduce_fear ?? params.safeZonesReduceFear ?? true,
    showFearMeter: params.show_fear_meter ?? params.showFearMeter ?? true,
    fearAffectsGameplay: params.fear_affects_gameplay ?? params.fearAffectsGameplay ?? true
  });
};

/**
 * Configure fear effects
 */
const handleConfigureFearEffects: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const effectType = validateRequiredEnum(
    params.effect_type || params.effectType,
    FEAR_EFFECT_TYPES,
    'effect_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'configure_fear_effects', {
    effectType,
    fearThreshold: params.fear_threshold ?? params.fearThreshold ?? 50,
    intensity: params.intensity ?? 1.0,
    postProcessProfile: params.post_process_profile || params.postProcessProfile,
    cameraShakeIntensity: params.camera_shake_intensity ?? params.cameraShakeIntensity ?? 0,
    audioDistortion: params.audio_distortion ?? params.audioDistortion ?? 0,
    inputLag: params.input_lag ?? params.inputLag ?? 0,
    staminaPenalty: params.stamina_penalty ?? params.staminaPenalty ?? 0
  });
};

/**
 * Create sanity system
 */
const handleCreateSanitySystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'create_sanity_system', {
    maxSanity: params.max_sanity ?? params.maxSanity ?? 100,
    startSanity: params.start_sanity ?? params.startSanity ?? 100,
    sanityDecayRate: params.sanity_decay_rate ?? params.sanityDecayRate ?? 0.1,
    sanityRecoveryRate: params.sanity_recovery_rate ?? params.sanityRecoveryRate ?? 0.05,
    darknessDrain: params.darkness_drain ?? params.darknessDrain ?? true,
    monsterViewDrain: params.monster_view_drain ?? params.monsterViewDrain ?? true,
    corpseViewDrain: params.corpse_view_drain ?? params.corpseViewDrain ?? true,
    lowSanityEffects: params.low_sanity_effects || params.lowSanityEffects,
    zeroSanityConsequence: params.zero_sanity_consequence || params.zeroSanityConsequence || 'debuff'
  });
};

/**
 * Configure atmosphere
 */
const handleConfigureAtmosphere: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const atmosphereType = validateRequiredEnum(
    params.atmosphere_type || params.atmosphereType,
    ATMOSPHERE_TYPES,
    'atmosphere_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'configure_atmosphere', {
    atmosphereType,
    fogDensity: params.fog_density ?? params.fogDensity ?? 0.02,
    fogColor: params.fog_color || params.fogColor,
    ambientLightIntensity: params.ambient_light_intensity ?? params.ambientLightIntensity ?? 0.1,
    ambientLightColor: params.ambient_light_color || params.ambientLightColor,
    musicTrack: params.music_track || params.musicTrack,
    ambientSounds: params.ambient_sounds || params.ambientSounds,
    postProcessVolume: params.post_process_volume || params.postProcessVolume,
    colorGrading: params.color_grading || params.colorGrading
  });
};

/**
 * Setup monster AI
 */
const handleSetupMonsterAI: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const monsterId = validateRequiredString(params.monster_id || params.monsterId, 'monster_id');

  const behavior = validateRequiredEnum(
    params.behavior,
    MONSTER_BEHAVIORS,
    'behavior'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'setup_monster_ai', {
    monsterId,
    behavior,
    meshPath: params.mesh_path || params.meshPath,
    detectionRange: params.detection_range ?? params.detectionRange ?? 1500,
    hearingRange: params.hearing_range ?? params.hearingRange ?? 2000,
    chaseSpeed: params.chase_speed ?? params.chaseSpeed ?? 500,
    patrolSpeed: params.patrol_speed ?? params.patrolSpeed ?? 200,
    damage: params.damage ?? 50,
    instantKill: params.instant_kill ?? params.instantKill ?? false,
    canBeKilled: params.can_be_killed ?? params.canBeKilled ?? false,
    fearOnSight: params.fear_on_sight ?? params.fearOnSight ?? 30,
    fearOnProximity: params.fear_on_proximity ?? params.fearOnProximity ?? 10,
    footstepSound: params.footstep_sound || params.footstepSound,
    chaseMusicCue: params.chase_music_cue || params.chaseMusicCue
  });
};

/**
 * Configure monster behavior
 */
const handleConfigureMonsterBehavior: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const monsterId = validateRequiredString(params.monster_id || params.monsterId, 'monster_id');

  const behavior = validateOptionalEnum(
    params.behavior,
    MONSTER_BEHAVIORS,
    'behavior'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'configure_monster_behavior', {
    monsterId,
    behavior,
    stalkDuration: params.stalk_duration ?? params.stalkDuration ?? 30,
    huntDuration: params.hunt_duration ?? params.huntDuration ?? 60,
    searchTime: params.search_time ?? params.searchTime ?? 15,
    loseSightTime: params.lose_sight_time ?? params.loseSightTime ?? 5,
    rubberBanding: params.rubber_banding ?? params.rubberBanding ?? true,
    minPlayerDistance: params.min_player_distance ?? params.minPlayerDistance ?? 500,
    teleportWhenUnseen: params.teleport_when_unseen ?? params.teleportWhenUnseen ?? false
  });
};

/**
 * Configure resource scarcity
 */
const handleConfigureResourceScarcity: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'configure_resource_scarcity', {
    ammoScarcity: params.ammo_scarcity ?? params.ammoScarcity ?? 0.5,
    healthScarcity: params.health_scarcity ?? params.healthScarcity ?? 0.3,
    saveItemScarcity: params.save_item_scarcity ?? params.saveItemScarcity ?? 0.2,
    keyItemGuaranteed: params.key_item_guaranteed ?? params.keyItemGuaranteed ?? true,
    adaptiveDrops: params.adaptive_drops ?? params.adaptiveDrops ?? false,
    adaptiveThreshold: params.adaptive_threshold ?? params.adaptiveThreshold ?? 0.25
  });
};

/**
 * Configure save system
 */
const handleConfigureSaveSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const saveType = validateRequiredEnum(
    params.save_type || params.saveType,
    SAVE_TYPES,
    'save_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'configure_save_system', {
    saveType,
    requiresSaveItem: params.requires_save_item ?? params.requiresSaveItem ?? false,
    saveItemId: params.save_item_id || params.saveItemId,
    limitedSaves: params.limited_saves ?? params.limitedSaves ?? false,
    maxSaves: params.max_saves ?? params.maxSaves ?? 99,
    saveRoomMusic: params.save_room_music || params.saveRoomMusic,
    saveRoomIsSafe: params.save_room_is_safe ?? params.saveRoomIsSafe ?? true
  });
};

/**
 * Create safe zone
 */
const handleCreateSafeZone: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const zoneId = validateRequiredString(params.zone_id || params.zoneId, 'zone_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'create_safe_zone', {
    zoneId,
    location: params.location,
    radius: params.radius ?? 500,
    monstersCannotEnter: params.monsters_cannot_enter ?? params.monstersCannotEnter ?? true,
    fearDecayMultiplier: params.fear_decay_multiplier ?? params.fearDecayMultiplier ?? 3.0,
    sanityRecoveryMultiplier: params.sanity_recovery_multiplier ?? params.sanityRecoveryMultiplier ?? 2.0,
    safeZoneMusic: params.safe_zone_music || params.safeZoneMusic,
    canSaveHere: params.can_save_here ?? params.canSaveHere ?? true,
    hasStorage: params.has_storage ?? params.hasStorage ?? true
  });
};

/**
 * Setup jumpscare system
 */
const handleSetupJumpscareSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'setup_jumpscare_system', {
    globalCooldown: params.global_cooldown ?? params.globalCooldown ?? 60,
    randomChance: params.random_chance ?? params.randomChance ?? 0.05,
    maxPerSession: params.max_per_session ?? params.maxPerSession ?? 10,
    scaleWithFear: params.scale_with_fear ?? params.scaleWithFear ?? true,
    fearGainOnJumpscare: params.fear_gain_on_jumpscare ?? params.fearGainOnJumpscare ?? 25,
    cameraShake: params.camera_shake ?? params.cameraShake ?? true,
    flashEffect: params.flash_effect ?? params.flashEffect ?? false
  });
};

/**
 * Create scare event
 */
const handleCreateScareEvent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const scareId = validateRequiredString(params.scare_id || params.scareId, 'scare_id');

  const scareType = validateRequiredEnum(
    params.scare_type || params.scareType,
    SCARE_TYPES,
    'scare_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'create_scare_event', {
    scareId,
    scareType,
    triggerLocation: params.trigger_location || params.triggerLocation,
    triggerRadius: params.trigger_radius ?? params.triggerRadius ?? 200,
    oneShot: params.one_shot ?? params.oneShot ?? true,
    fearGain: params.fear_gain ?? params.fearGain ?? 20,
    soundCue: params.sound_cue || params.soundCue,
    visualEffect: params.visual_effect || params.visualEffect,
    spawnActor: params.spawn_actor || params.spawnActor,
    cameraAnimation: params.camera_animation || params.cameraAnimation,
    delay: params.delay ?? 0,
    duration: params.duration ?? 2
  });
};

/**
 * Configure lighting horror
 */
const handleConfigureLightingHorror: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'configure_lighting_horror', {
    playerFlashlight: params.player_flashlight ?? params.playerFlashlight ?? true,
    flashlightBattery: params.flashlight_battery ?? params.flashlightBattery ?? true,
    batteryDrainRate: params.battery_drain_rate ?? params.batteryDrainRate ?? 0.5,
    flickeringLights: params.flickering_lights ?? params.flickeringLights ?? true,
    flickerOnMonsterNear: params.flicker_on_monster_near ?? params.flickerOnMonsterNear ?? true,
    darknessFearRate: params.darkness_fear_rate ?? params.darknessFearRate ?? 1.0,
    lightSources: params.light_sources || params.lightSources
  });
};

/**
 * Get player fear level
 */
const handleGetFearLevel: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'get_fear_level', {
    playerId: params.player_id || params.playerId
  });
};

/**
 * Modify fear level
 */
const handleModifyFearLevel: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const amount = params.amount;
  if (typeof amount !== 'number') {
    throw new Error('amount is required and must be a number');
  }

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'modify_fear_level', {
    amount,
    playerId: params.player_id || params.playerId,
    triggerEffects: params.trigger_effects ?? params.triggerEffects ?? true
  });
};

/**
 * Spawn monster
 */
const handleSpawnMonster: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const monsterId = validateRequiredString(params.monster_id || params.monsterId, 'monster_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_HORROR, 'spawn_monster', {
    monsterId,
    location: params.location,
    initialBehavior: params.initial_behavior || params.initialBehavior,
    targetPlayer: params.target_player ?? params.targetPlayer ?? true
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const genreHorrorHandlers: ActionHandlerMap = {
  create_fear_system: handleCreateFearSystem,
  configure_fear_effects: handleConfigureFearEffects,
  create_sanity_system: handleCreateSanitySystem,
  configure_atmosphere: handleConfigureAtmosphere,
  setup_monster_ai: handleSetupMonsterAI,
  configure_monster_behavior: handleConfigureMonsterBehavior,
  configure_resource_scarcity: handleConfigureResourceScarcity,
  configure_save_system: handleConfigureSaveSystem,
  create_safe_zone: handleCreateSafeZone,
  setup_jumpscare_system: handleSetupJumpscareSystem,
  create_scare_event: handleCreateScareEvent,
  configure_lighting_horror: handleConfigureLightingHorror,
  get_fear_level: handleGetFearLevel,
  modify_fear_level: handleModifyFearLevel,
  spawn_monster: handleSpawnMonster
};

/**
 * Main genre horror handler - routes to specific action handlers
 */
export const handleGenreHorrorTools = createCategoryHandler(Forge2Categories.GENRE_HORROR, genreHorrorHandlers);

/**
 * Get available genre horror actions
 */
export function getGenreHorrorActions(): string[] {
  return Object.keys(genreHorrorHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeGenreHorrorHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = genreHorrorHandlers[action];
  if (!handler) {
    throw new Error(`Unknown genre horror action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
