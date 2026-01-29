/**
 * FORGE2: MetaSounds Handlers
 * Handles UE5 MetaSounds procedural audio creation and configuration
 * Supports oscillators, filters, envelopes, procedural SFX, and dynamic music
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
  if (typeof value === 'object' && !Array.isArray(value)) return value as Record<string, unknown>;
  return undefined;
}

// ============================================================================
// Constants
// ============================================================================

const OSCILLATOR_TYPES = ['sine', 'saw', 'square', 'triangle', 'noise', 'white_noise', 'pink_noise'] as const;
const FILTER_TYPES = ['lowpass', 'highpass', 'bandpass', 'notch', 'allpass', 'peak', 'lowshelf', 'highshelf'] as const;
const ENVELOPE_TYPES = ['adsr', 'ar', 'custom'] as const;
const SPATIALIZATION_MODES = ['off', 'binaural', 'surround', 'object_based'] as const;
const ATTENUATION_SHAPES = ['linear', 'logarithmic', 'inverse', 'natural', 'custom'] as const;
const TRIGGER_MODES = ['one_shot', 'looping', 'retriggerable', 'gate'] as const;

// Reserve constants for future use
void OSCILLATOR_TYPES;
void FILTER_TYPES;
void ENVELOPE_TYPES;
void SPATIALIZATION_MODES;
void ATTENUATION_SHAPES;
void TRIGGER_MODES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a new MetaSound source
 */
const handleCreateMetaSound: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const metaSoundName = validateRequiredString(params.metasound_name || params.metaSoundName, 'metasound_name');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'create_metasound', {
    metaSoundName,
    outputPath: validateOptionalString(params.output_path || params.outputPath) ?? '/Game/Audio/MetaSounds',
    description: validateOptionalString(params.description),
    isSource: validateOptionalBoolean(params.is_source || params.isSource) ?? true,
    numChannels: validateOptionalNumber(params.num_channels || params.numChannels) ?? 2,
    sampleRate: validateOptionalNumber(params.sample_rate || params.sampleRate) ?? 48000,
    blockSize: validateOptionalNumber(params.block_size || params.blockSize) ?? 256
  });
};

/**
 * Add an oscillator to a MetaSound
 */
const handleAddOscillator: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const metaSoundPath = validateRequiredString(params.metasound_path || params.metaSoundPath, 'metasound_path');
  const oscillatorType = validateRequiredEnum(params.oscillator_type || params.oscillatorType, OSCILLATOR_TYPES, 'oscillator_type');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'add_oscillator', {
    metaSoundPath,
    oscillatorType,
    nodeName: validateOptionalString(params.node_name || params.nodeName),
    frequency: validateOptionalNumber(params.frequency) ?? 440,
    amplitude: validateOptionalNumber(params.amplitude) ?? 1.0,
    phase: validateOptionalNumber(params.phase) ?? 0,
    pulseWidth: validateOptionalNumber(params.pulse_width || params.pulseWidth) ?? 0.5, // For square wave
    detuning: validateOptionalNumber(params.detuning) ?? 0,
    frequencyModulation: validateOptionalBoolean(params.frequency_modulation || params.frequencyModulation) ?? false,
    fmDepth: validateOptionalNumber(params.fm_depth || params.fmDepth)
  });
};

/**
 * Add a filter to a MetaSound
 */
const handleAddFilter: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const metaSoundPath = validateRequiredString(params.metasound_path || params.metaSoundPath, 'metasound_path');
  const filterType = validateRequiredEnum(params.filter_type || params.filterType, FILTER_TYPES, 'filter_type');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'add_filter', {
    metaSoundPath,
    filterType,
    nodeName: validateOptionalString(params.node_name || params.nodeName),
    cutoffFrequency: validateOptionalNumber(params.cutoff_frequency || params.cutoffFrequency) ?? 1000,
    resonance: validateOptionalNumber(params.resonance) ?? 0.7,
    gain: validateOptionalNumber(params.gain) ?? 0, // For shelf/peak filters
    bandwidth: validateOptionalNumber(params.bandwidth) ?? 1.0,
    filterOrder: validateOptionalNumber(params.filter_order || params.filterOrder) ?? 2
  });
};

/**
 * Add an envelope to a MetaSound
 */
const handleAddEnvelope: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const metaSoundPath = validateRequiredString(params.metasound_path || params.metaSoundPath, 'metasound_path');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'add_envelope', {
    metaSoundPath,
    envelopeType: validateOptionalEnum(params.envelope_type || params.envelopeType, ENVELOPE_TYPES) ?? 'adsr',
    nodeName: validateOptionalString(params.node_name || params.nodeName),
    attack: validateOptionalNumber(params.attack) ?? 0.01,
    decay: validateOptionalNumber(params.decay) ?? 0.1,
    sustain: validateOptionalNumber(params.sustain) ?? 0.7,
    release: validateOptionalNumber(params.release) ?? 0.3,
    attackCurve: validateOptionalNumber(params.attack_curve || params.attackCurve) ?? 1.0,
    decayCurve: validateOptionalNumber(params.decay_curve || params.decayCurve) ?? 1.0,
    releaseCurve: validateOptionalNumber(params.release_curve || params.releaseCurve) ?? 1.0,
    customCurve: validateOptionalArray(params.custom_curve || params.customCurve)
  });
};

/**
 * Create procedural SFX preset
 */
const handleCreateProceduralSFX: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sfxName = validateRequiredString(params.sfx_name || params.sfxName, 'sfx_name');
  const sfxType = validateRequiredString(params.sfx_type || params.sfxType, 'sfx_type');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'create_procedural_sfx', {
    sfxName,
    sfxType, // 'footstep', 'impact', 'explosion', 'weapon', 'ambient', 'ui', 'creature'
    outputPath: validateOptionalString(params.output_path || params.outputPath),
    // Footstep params
    surface: validateOptionalString(params.surface), // 'concrete', 'grass', 'metal', 'wood', 'water'
    // Impact params
    materialHardness: validateOptionalNumber(params.material_hardness || params.materialHardness),
    impactForce: validateOptionalNumber(params.impact_force || params.impactForce),
    // Explosion params
    size: validateOptionalString(params.size), // 'small', 'medium', 'large', 'huge'
    // Ambient params
    ambientType: validateOptionalString(params.ambient_type || params.ambientType), // 'wind', 'rain', 'forest', 'city', 'cave'
    // Common params
    pitchVariation: validateOptionalNumber(params.pitch_variation || params.pitchVariation) ?? 0.1,
    volumeVariation: validateOptionalNumber(params.volume_variation || params.volumeVariation) ?? 0.1,
    layerCount: validateOptionalNumber(params.layer_count || params.layerCount) ?? 3
  });
};

/**
 * Create dynamic/adaptive music system
 */
const handleCreateDynamicMusic: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const musicSystemName = validateRequiredString(params.music_system_name || params.musicSystemName, 'music_system_name');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'create_dynamic_music', {
    musicSystemName,
    outputPath: validateOptionalString(params.output_path || params.outputPath),
    bpm: validateOptionalNumber(params.bpm) ?? 120,
    timeSignatureNumerator: validateOptionalNumber(params.time_signature_numerator || params.timeSignatureNumerator) ?? 4,
    timeSignatureDenominator: validateOptionalNumber(params.time_signature_denominator || params.timeSignatureDenominator) ?? 4,
    layers: validateOptionalArray(params.layers), // Array of layer configurations
    transitionBars: validateOptionalNumber(params.transition_bars || params.transitionBars) ?? 4,
    crossfadeType: validateOptionalString(params.crossfade_type || params.crossfadeType) ?? 'equal_power',
    quantizeTransitions: validateOptionalBoolean(params.quantize_transitions || params.quantizeTransitions) ?? true,
    stingerSupport: validateOptionalBoolean(params.stinger_support || params.stingerSupport) ?? true
  });
};

/**
 * Add music layer
 */
const handleAddMusicLayer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const musicSystemPath = validateRequiredString(params.music_system_path || params.musicSystemPath, 'music_system_path');
  const layerName = validateRequiredString(params.layer_name || params.layerName, 'layer_name');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'add_music_layer', {
    musicSystemPath,
    layerName,
    audioAssetPath: validateOptionalString(params.audio_asset_path || params.audioAssetPath),
    volumeDefault: validateOptionalNumber(params.volume_default || params.volumeDefault) ?? 1.0,
    fadeInTime: validateOptionalNumber(params.fade_in_time || params.fadeInTime) ?? 2.0,
    fadeOutTime: validateOptionalNumber(params.fade_out_time || params.fadeOutTime) ?? 2.0,
    priority: validateOptionalNumber(params.priority) ?? 0,
    conditions: validateOptionalObject(params.conditions), // When this layer should play
    isPercussion: validateOptionalBoolean(params.is_percussion || params.isPercussion) ?? false,
    isMelody: validateOptionalBoolean(params.is_melody || params.isMelody) ?? false,
    isHarmony: validateOptionalBoolean(params.is_harmony || params.isHarmony) ?? false
  });
};

/**
 * Configure spatialization
 */
const handleConfigureSpatialization: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const metaSoundPath = validateRequiredString(params.metasound_path || params.metaSoundPath, 'metasound_path');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'configure_spatialization', {
    metaSoundPath,
    spatializationMode: validateOptionalEnum(params.spatialization_mode || params.spatializationMode, SPATIALIZATION_MODES) ?? 'binaural',
    attenuationShape: validateOptionalEnum(params.attenuation_shape || params.attenuationShape, ATTENUATION_SHAPES) ?? 'natural',
    innerRadius: validateOptionalNumber(params.inner_radius || params.innerRadius) ?? 200,
    falloffDistance: validateOptionalNumber(params.falloff_distance || params.falloffDistance) ?? 2000,
    lpfRadiusMin: validateOptionalNumber(params.lpf_radius_min || params.lpfRadiusMin),
    lpfRadiusMax: validateOptionalNumber(params.lpf_radius_max || params.lpfRadiusMax),
    occlusionEnabled: validateOptionalBoolean(params.occlusion_enabled || params.occlusionEnabled) ?? true,
    reverbSend: validateOptionalNumber(params.reverb_send || params.reverbSend) ?? 0.5
  });
};

/**
 * Connect MetaSound nodes
 */
const handleConnectNodes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const metaSoundPath = validateRequiredString(params.metasound_path || params.metaSoundPath, 'metasound_path');
  const sourceNode = validateRequiredString(params.source_node || params.sourceNode, 'source_node');
  const targetNode = validateRequiredString(params.target_node || params.targetNode, 'target_node');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'connect_nodes', {
    metaSoundPath,
    sourceNode,
    sourcePin: validateOptionalString(params.source_pin || params.sourcePin) ?? 'Output',
    targetNode,
    targetPin: validateOptionalString(params.target_pin || params.targetPin) ?? 'Input'
  });
};

/**
 * Add MetaSound input parameter
 */
const handleAddInputParameter: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const metaSoundPath = validateRequiredString(params.metasound_path || params.metaSoundPath, 'metasound_path');
  const parameterName = validateRequiredString(params.parameter_name || params.parameterName, 'parameter_name');
  const parameterType = validateRequiredString(params.parameter_type || params.parameterType, 'parameter_type');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'add_input_parameter', {
    metaSoundPath,
    parameterName,
    parameterType, // 'float', 'int', 'bool', 'trigger', 'audio', 'string'
    defaultValue: params.default_value ?? params.defaultValue,
    minValue: validateOptionalNumber(params.min_value || params.minValue),
    maxValue: validateOptionalNumber(params.max_value || params.maxValue),
    displayName: validateOptionalString(params.display_name || params.displayName),
    description: validateOptionalString(params.description)
  });
};

/**
 * Get MetaSound info
 */
const handleGetMetaSoundInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const metaSoundPath = validateRequiredString(params.metasound_path || params.metaSoundPath, 'metasound_path');

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'get_metasound_info', {
    metaSoundPath,
    includeNodes: validateOptionalBoolean(params.include_nodes || params.includeNodes) ?? true,
    includeConnections: validateOptionalBoolean(params.include_connections || params.includeConnections) ?? true,
    includeParameters: validateOptionalBoolean(params.include_parameters || params.includeParameters) ?? true
  });
};

/**
 * List MetaSounds
 */
const handleListMetaSounds: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.METASOUNDS, 'list_metasounds', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true,
    includePresets: validateOptionalBoolean(params.include_presets || params.includePresets) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const metaSoundsHandlers: ActionHandlerMap = {
  // Core MetaSound creation
  create_metasound: handleCreateMetaSound,
  add_oscillator: handleAddOscillator,
  add_filter: handleAddFilter,
  add_envelope: handleAddEnvelope,

  // Procedural audio
  create_procedural_sfx: handleCreateProceduralSFX,

  // Dynamic music
  create_dynamic_music: handleCreateDynamicMusic,
  add_music_layer: handleAddMusicLayer,

  // Configuration
  configure_spatialization: handleConfigureSpatialization,

  // Node operations
  connect_nodes: handleConnectNodes,
  add_input_parameter: handleAddInputParameter,

  // Query
  get_metasound_info: handleGetMetaSoundInfo,
  list_metasounds: handleListMetaSounds
};

export const handleMetaSoundsTools = createCategoryHandler(Forge2Categories.METASOUNDS, metaSoundsHandlers);

export default metaSoundsHandlers;
