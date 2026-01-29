/**
 * FORGE2: Music Handlers
 * Handles UE5 dynamic music systems, adaptive music, layers, and transitions
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

const MUSIC_STATES = ['exploration', 'combat', 'stealth', 'menu', 'victory', 'defeat', 'boss', 'ambient'] as const;
const TRANSITION_TYPES = ['crossfade', 'sync_point', 'beat_sync', 'immediate', 'fade_out_in'] as const;
const LAYER_TYPES = ['base', 'melody', 'percussion', 'bass', 'ambient', 'stinger'] as const;
const INTENSITY_LEVELS = ['low', 'medium', 'high', 'very_high', 'climax'] as const;

// Reserve constants for future use
void MUSIC_STATES;
void TRANSITION_TYPES;
void LAYER_TYPES;
void INTENSITY_LEVELS;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create music layer system
 */
const handleCreateMusicLayerSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemName = validateRequiredString(params.system_name || params.systemName, 'system_name');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'create_music_layer_system', {
    systemName,
    bpm: validateOptionalNumber(params.bpm) ?? 120,
    timeSignatureNumerator: validateOptionalNumber(params.time_signature_numerator || params.timeSignatureNumerator) ?? 4,
    timeSignatureDenominator: validateOptionalNumber(params.time_signature_denominator || params.timeSignatureDenominator) ?? 4,
    defaultTransitionType: validateOptionalEnum(params.default_transition_type || params.defaultTransitionType, TRANSITION_TYPES) ?? 'crossfade',
    defaultTransitionDuration: validateOptionalNumber(params.default_transition_duration || params.defaultTransitionDuration) ?? 2.0,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add music layer
 */
const handleAddMusicLayer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const layerName = validateRequiredString(params.layer_name || params.layerName, 'layer_name');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'add_music_layer', {
    systemPath,
    layerName,
    layerType: validateOptionalEnum(params.layer_type || params.layerType, LAYER_TYPES) ?? 'base',
    soundWavePath: validateOptionalString(params.sound_wave_path || params.soundWavePath),
    looping: validateOptionalBoolean(params.looping) ?? true,
    volume: validateOptionalNumber(params.volume) ?? 1.0,
    pitch: validateOptionalNumber(params.pitch) ?? 1.0,
    fadeInTime: validateOptionalNumber(params.fade_in_time || params.fadeInTime) ?? 0.5,
    fadeOutTime: validateOptionalNumber(params.fade_out_time || params.fadeOutTime) ?? 0.5,
    enabledByDefault: validateOptionalBoolean(params.enabled_by_default || params.enabledByDefault) ?? true
  });
};

/**
 * Configure music state
 */
const handleConfigureMusicState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const stateName = validateRequiredString(params.state_name || params.stateName, 'state_name');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'configure_music_state', {
    systemPath,
    stateName,
    stateType: validateOptionalEnum(params.state_type || params.stateType, MUSIC_STATES) ?? 'exploration',
    activeLayers: validateOptionalArray(params.active_layers || params.activeLayers) ?? [],
    layerVolumes: validateOptionalObject(params.layer_volumes || params.layerVolumes),
    transitionType: validateOptionalEnum(params.transition_type || params.transitionType, TRANSITION_TYPES) ?? 'crossfade',
    transitionDuration: validateOptionalNumber(params.transition_duration || params.transitionDuration) ?? 2.0,
    priority: validateOptionalNumber(params.priority) ?? 0
  });
};

/**
 * Setup stinger system
 */
const handleSetupStingerSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'setup_stinger_system', {
    systemPath,
    stingers: validateOptionalArray(params.stingers) ?? [], // Array of { name, soundPath, trigger, volume }
    cooldownTime: validateOptionalNumber(params.cooldown_time || params.cooldownTime) ?? 5.0,
    maxConcurrentStingers: validateOptionalNumber(params.max_concurrent_stingers || params.maxConcurrentStingers) ?? 1,
    duckMainMusic: validateOptionalBoolean(params.duck_main_music || params.duckMainMusic) ?? true,
    duckAmount: validateOptionalNumber(params.duck_amount || params.duckAmount) ?? 0.3
  });
};

/**
 * Create adaptive music rule
 */
const handleCreateAdaptiveMusicRule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const ruleName = validateRequiredString(params.rule_name || params.ruleName, 'rule_name');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'create_adaptive_music_rule', {
    systemPath,
    ruleName,
    condition: validateRequiredString(params.condition, 'condition'), // e.g., 'enemy_count > 3', 'health < 25%'
    targetState: validateOptionalString(params.target_state || params.targetState),
    targetLayers: validateOptionalArray(params.target_layers || params.targetLayers),
    intensityChange: validateOptionalEnum(params.intensity_change || params.intensityChange, INTENSITY_LEVELS),
    priority: validateOptionalNumber(params.priority) ?? 0
  });
};

/**
 * Configure music transitions
 */
const handleConfigureMusicTransitions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'configure_music_transitions', {
    systemPath,
    fromState: validateOptionalString(params.from_state || params.fromState),
    toState: validateOptionalString(params.to_state || params.toState),
    transitionType: validateOptionalEnum(params.transition_type || params.transitionType, TRANSITION_TYPES) ?? 'crossfade',
    duration: validateOptionalNumber(params.duration) ?? 2.0,
    syncPoints: validateOptionalArray(params.sync_points || params.syncPoints), // Beat numbers to sync on
    quantizeToBeat: validateOptionalBoolean(params.quantize_to_beat || params.quantizeToBeat) ?? true
  });
};

/**
 * Setup rhythm synchronization
 */
const handleSetupRhythmSync: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'setup_rhythm_sync', {
    systemPath,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    bpm: validateOptionalNumber(params.bpm) ?? 120,
    beatsPerBar: validateOptionalNumber(params.beats_per_bar || params.beatsPerBar) ?? 4,
    beatEventEnabled: validateOptionalBoolean(params.beat_event_enabled || params.beatEventEnabled) ?? false,
    barEventEnabled: validateOptionalBoolean(params.bar_event_enabled || params.barEventEnabled) ?? false,
    lookaheadMs: validateOptionalNumber(params.lookahead_ms || params.lookaheadMs) ?? 100
  });
};

/**
 * Set music intensity
 */
const handleSetMusicIntensity: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'set_music_intensity', {
    systemPath,
    intensity: validateOptionalEnum(params.intensity, INTENSITY_LEVELS) ?? 'medium',
    intensityValue: validateOptionalNumber(params.intensity_value || params.intensityValue), // 0.0 to 1.0
    transitionTime: validateOptionalNumber(params.transition_time || params.transitionTime) ?? 1.0
  });
};

/**
 * Transition to music state
 */
const handleTransitionToMusicState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const targetState = validateRequiredString(params.target_state || params.targetState, 'target_state');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'transition_to_music_state', {
    systemPath,
    targetState,
    overrideTransitionType: validateOptionalEnum(params.override_transition_type || params.overrideTransitionType, TRANSITION_TYPES),
    overrideTransitionDuration: validateOptionalNumber(params.override_transition_duration || params.overrideTransitionDuration),
    immediate: validateOptionalBoolean(params.immediate) ?? false
  });
};

/**
 * Play stinger
 */
const handlePlayStinger: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const stingerName = validateRequiredString(params.stinger_name || params.stingerName, 'stinger_name');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'play_stinger', {
    systemPath,
    stingerName,
    volumeMultiplier: validateOptionalNumber(params.volume_multiplier || params.volumeMultiplier) ?? 1.0,
    ignoreCooldown: validateOptionalBoolean(params.ignore_cooldown || params.ignoreCooldown) ?? false
  });
};

/**
 * Get music system state
 */
const handleGetMusicSystemState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'get_music_system_state', {
    systemPath,
    includeLayers: validateOptionalBoolean(params.include_layers || params.includeLayers) ?? true,
    includeRules: validateOptionalBoolean(params.include_rules || params.includeRules) ?? true,
    includeTransitions: validateOptionalBoolean(params.include_transitions || params.includeTransitions) ?? true
  });
};

/**
 * List music systems
 */
const handleListMusicSystems: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MUSIC, 'list_music_systems', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const musicHandlers: ActionHandlerMap = {
  // System Creation
  create_music_layer_system: handleCreateMusicLayerSystem,
  add_music_layer: handleAddMusicLayer,

  // State Configuration
  configure_music_state: handleConfigureMusicState,
  configure_music_transitions: handleConfigureMusicTransitions,
  create_adaptive_music_rule: handleCreateAdaptiveMusicRule,

  // Stingers
  setup_stinger_system: handleSetupStingerSystem,
  play_stinger: handlePlayStinger,

  // Rhythm
  setup_rhythm_sync: handleSetupRhythmSync,

  // Playback Control
  set_music_intensity: handleSetMusicIntensity,
  transition_to_music_state: handleTransitionToMusicState,

  // Query
  get_music_system_state: handleGetMusicSystemState,
  list_music_systems: handleListMusicSystems
};

export const handleMusicTools = createCategoryHandler(Forge2Categories.MUSIC, musicHandlers);

export default musicHandlers;
