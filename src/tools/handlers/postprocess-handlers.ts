/**
 * FORGE2: Post-Process Handlers
 * Handles UE5 post-processing volumes, effects, and color grading
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

const DOF_METHODS = ['gaussian', 'bokeh', 'cinematic'] as const;
const BLOOM_METHODS = ['standard', 'convolution'] as const;
const AO_METHODS = ['ssao', 'gtao', 'hbao'] as const;
const TONEMAP_METHODS = ['filmic', 'aces', 'linear', 'none'] as const;

// Reserve constants for future use
void DOF_METHODS;
void BLOOM_METHODS;
void AO_METHODS;
void TONEMAP_METHODS;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a post-process volume
 */
const handleCreatePostProcessVolume: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'create_post_process_volume', {
    actorName: validateOptionalString(params.actor_name || params.actorName),
    location: validateOptionalObject(params.location),
    bounds: validateOptionalObject(params.bounds),
    unbound: validateOptionalBoolean(params.unbound) ?? false,
    priority: validateOptionalNumber(params.priority) ?? 0,
    blendRadius: validateOptionalNumber(params.blend_radius || params.blendRadius) ?? 100.0,
    blendWeight: validateOptionalNumber(params.blend_weight || params.blendWeight) ?? 1.0,
    enabled: validateOptionalBoolean(params.enabled) ?? true
  });
};

/**
 * Configure bloom
 */
const handleConfigureBloom: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'configure_bloom', {
    volumeActorName,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    method: validateOptionalEnum(params.method, BLOOM_METHODS) ?? 'standard',
    intensity: validateOptionalNumber(params.intensity) ?? 0.675,
    threshold: validateOptionalNumber(params.threshold) ?? -1.0,
    sizeScale: validateOptionalNumber(params.size_scale || params.sizeScale) ?? 4.0,
    tint: validateOptionalObject(params.tint), // { r, g, b }
    convolutionTexture: validateOptionalString(params.convolution_texture || params.convolutionTexture)
  });
};

/**
 * Configure color grading
 */
const handleConfigureColorGrading: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'configure_color_grading', {
    volumeActorName,
    lutTexturePath: validateOptionalString(params.lut_texture_path || params.lutTexturePath),
    lutIntensity: validateOptionalNumber(params.lut_intensity || params.lutIntensity) ?? 1.0,
    whiteTemp: validateOptionalNumber(params.white_temp || params.whiteTemp) ?? 6500,
    whiteTint: validateOptionalNumber(params.white_tint || params.whiteTint) ?? 0,
    saturation: validateOptionalObject(params.saturation), // { r, g, b, a }
    contrast: validateOptionalObject(params.contrast),
    gamma: validateOptionalObject(params.gamma),
    gain: validateOptionalObject(params.gain),
    offset: validateOptionalObject(params.offset),
    shadowsMax: validateOptionalNumber(params.shadows_max || params.shadowsMax),
    highlightsMin: validateOptionalNumber(params.highlights_min || params.highlightsMin),
    tonemapMethod: validateOptionalEnum(params.tonemap_method || params.tonemapMethod, TONEMAP_METHODS) ?? 'aces'
  });
};

/**
 * Configure depth of field
 */
const handleConfigureDepthOfField: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'configure_depth_of_field', {
    volumeActorName,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    method: validateOptionalEnum(params.method, DOF_METHODS) ?? 'cinematic',
    focalDistance: validateOptionalNumber(params.focal_distance || params.focalDistance) ?? 1000.0,
    depthBlurRadius: validateOptionalNumber(params.depth_blur_radius || params.depthBlurRadius) ?? 0.0,
    depthBlurAmount: validateOptionalNumber(params.depth_blur_amount || params.depthBlurAmount) ?? 1.0,
    sensorWidth: validateOptionalNumber(params.sensor_width || params.sensorWidth) ?? 24.89,
    fstop: validateOptionalNumber(params.fstop) ?? 4.0,
    bladeCount: validateOptionalNumber(params.blade_count || params.bladeCount) ?? 5,
    minFstop: validateOptionalNumber(params.min_fstop || params.minFstop) ?? 1.2,
    nearTransitionRegion: validateOptionalNumber(params.near_transition_region || params.nearTransitionRegion) ?? 300,
    farTransitionRegion: validateOptionalNumber(params.far_transition_region || params.farTransitionRegion) ?? 500
  });
};

/**
 * Configure motion blur
 */
const handleConfigureMotionBlur: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'configure_motion_blur', {
    volumeActorName,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    amount: validateOptionalNumber(params.amount) ?? 0.5,
    max: validateOptionalNumber(params.max) ?? 5.0,
    perObjectSize: validateOptionalNumber(params.per_object_size || params.perObjectSize) ?? 0.5,
    targetFPS: validateOptionalNumber(params.target_fps || params.targetFPS) ?? 30
  });
};

/**
 * Configure ambient occlusion
 */
const handleConfigureAmbientOcclusion: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'configure_ambient_occlusion', {
    volumeActorName,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    method: validateOptionalEnum(params.method, AO_METHODS) ?? 'gtao',
    intensity: validateOptionalNumber(params.intensity) ?? 0.5,
    radius: validateOptionalNumber(params.radius) ?? 200.0,
    staticFraction: validateOptionalNumber(params.static_fraction || params.staticFraction) ?? 0.1,
    fadeOutDistance: validateOptionalNumber(params.fade_out_distance || params.fadeOutDistance) ?? 8000.0,
    fadeOutRadius: validateOptionalNumber(params.fade_out_radius || params.fadeOutRadius) ?? 5000.0,
    power: validateOptionalNumber(params.power) ?? 2.0,
    bias: validateOptionalNumber(params.bias) ?? 3.0
  });
};

/**
 * Configure screen effects
 */
const handleConfigureScreenEffects: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'configure_screen_effects', {
    volumeActorName,
    // Vignette
    vignetteIntensity: validateOptionalNumber(params.vignette_intensity || params.vignetteIntensity) ?? 0.4,
    // Film Grain
    grainIntensity: validateOptionalNumber(params.grain_intensity || params.grainIntensity) ?? 0.0,
    grainJitter: validateOptionalNumber(params.grain_jitter || params.grainJitter) ?? 0.5,
    // Chromatic Aberration
    chromaticAberrationIntensity: validateOptionalNumber(params.chromatic_aberration_intensity || params.chromaticAberrationIntensity) ?? 0.0,
    chromaticAberrationOffset: validateOptionalNumber(params.chromatic_aberration_offset || params.chromaticAberrationOffset) ?? 0.0,
    // Lens Flare
    lensFlareIntensity: validateOptionalNumber(params.lens_flare_intensity || params.lensFlareIntensity) ?? 1.0,
    lensFlareTint: validateOptionalObject(params.lens_flare_tint || params.lensFlareTint),
    lensFlareBokehSize: validateOptionalNumber(params.lens_flare_bokeh_size || params.lensFlareBokehSize) ?? 3.0
  });
};

/**
 * Configure fog
 */
const handleConfigureFog: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'configure_fog', {
    volumeActorName,
    // Exponential Fog
    exponentialFogEnabled: validateOptionalBoolean(params.exponential_fog_enabled || params.exponentialFogEnabled) ?? false,
    fogDensity: validateOptionalNumber(params.fog_density || params.fogDensity) ?? 0.02,
    fogHeightFalloff: validateOptionalNumber(params.fog_height_falloff || params.fogHeightFalloff) ?? 0.2,
    fogInscatteringColor: validateOptionalObject(params.fog_inscattering_color || params.fogInscatteringColor),
    fogMaxOpacity: validateOptionalNumber(params.fog_max_opacity || params.fogMaxOpacity) ?? 1.0,
    fogStartDistance: validateOptionalNumber(params.fog_start_distance || params.fogStartDistance) ?? 0,
    // Volumetric Fog
    volumetricFogEnabled: validateOptionalBoolean(params.volumetric_fog_enabled || params.volumetricFogEnabled) ?? false,
    volumetricFogDistance: validateOptionalNumber(params.volumetric_fog_distance || params.volumetricFogDistance) ?? 6000.0,
    volumetricFogExtinctionScale: validateOptionalNumber(params.volumetric_fog_extinction_scale || params.volumetricFogExtinctionScale) ?? 1.0,
    volumetricFogAlbedo: validateOptionalObject(params.volumetric_fog_albedo || params.volumetricFogAlbedo),
    volumetricFogEmissive: validateOptionalObject(params.volumetric_fog_emissive || params.volumetricFogEmissive)
  });
};

/**
 * Create outline effect
 */
const handleCreateOutlineEffect: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'create_outline_effect', {
    volumeActorName,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    outlineColor: validateOptionalObject(params.outline_color || params.outlineColor) ?? { r: 1, g: 1, b: 0, a: 1 },
    outlineThickness: validateOptionalNumber(params.outline_thickness || params.outlineThickness) ?? 2.0,
    depthThreshold: validateOptionalNumber(params.depth_threshold || params.depthThreshold) ?? 0.001,
    normalThreshold: validateOptionalNumber(params.normal_threshold || params.normalThreshold) ?? 0.5,
    edgeGlow: validateOptionalBoolean(params.edge_glow || params.edgeGlow) ?? false,
    glowIntensity: validateOptionalNumber(params.glow_intensity || params.glowIntensity) ?? 1.0
  });
};

/**
 * Get post-process volume info
 */
const handleGetPostProcessVolumeInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const volumeActorName = validateRequiredString(params.volume_actor_name || params.volumeActorName, 'volume_actor_name');

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'get_post_process_volume_info', {
    volumeActorName,
    includeAllSettings: validateOptionalBoolean(params.include_all_settings || params.includeAllSettings) ?? true
  });
};

/**
 * List post-process volumes
 */
const handleListPostProcessVolumes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.POST_PROCESS, 'list_post_process_volumes', {
    levelName: validateOptionalString(params.level_name || params.levelName)
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const postprocessHandlers: ActionHandlerMap = {
  // Volume
  create_post_process_volume: handleCreatePostProcessVolume,

  // Effects
  configure_bloom: handleConfigureBloom,
  configure_color_grading: handleConfigureColorGrading,
  configure_depth_of_field: handleConfigureDepthOfField,
  configure_motion_blur: handleConfigureMotionBlur,
  configure_ambient_occlusion: handleConfigureAmbientOcclusion,
  configure_screen_effects: handleConfigureScreenEffects,
  configure_fog: handleConfigureFog,
  create_outline_effect: handleCreateOutlineEffect,

  // Query
  get_post_process_volume_info: handleGetPostProcessVolumeInfo,
  list_post_process_volumes: handleListPostProcessVolumes
};

export const handlePostProcessTools = createCategoryHandler(Forge2Categories.POST_PROCESS, postprocessHandlers);

export default postprocessHandlers;
