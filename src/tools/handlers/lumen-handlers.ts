/**
 * FORGE2: Lumen Handlers
 * Handles UE5 Lumen Global Illumination and reflections configuration
 * Supports quality settings, hardware ray tracing, and performance optimization
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

// Note: validateRequiredString reserved for future use when required params are added
// function validateRequiredString - see other handlers for pattern

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

// ============================================================================
// Constants
// ============================================================================

const LUMEN_QUALITY_LEVELS = ['low', 'medium', 'high', 'epic'] as const;
const REFLECTION_METHODS = ['screen_space', 'lumen', 'ray_traced'] as const;
const GI_METHODS = ['none', 'lumen', 'ray_traced'] as const;
const TRACE_METHODS = ['surface_cache', 'hit_lighting', 'detail_tracing'] as const;

// Reserve constants for future use
void LUMEN_QUALITY_LEVELS;
void REFLECTION_METHODS;
void GI_METHODS;
void TRACE_METHODS;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Configure Lumen Global Illumination
 */
const handleConfigureLumenGI: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'configure_lumen_gi', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    quality: validateOptionalEnum(params.quality, LUMEN_QUALITY_LEVELS),
    sceneViewDistance: validateOptionalNumber(params.scene_view_distance || params.sceneViewDistance) ?? 20000,
    sceneLightingQuality: validateOptionalNumber(params.scene_lighting_quality || params.sceneLightingQuality) ?? 1.0,
    sceneDetail: validateOptionalNumber(params.scene_detail || params.sceneDetail) ?? 1.0,
    finalGatherQuality: validateOptionalNumber(params.final_gather_quality || params.finalGatherQuality) ?? 1.0,
    finalGatherLighting: validateOptionalNumber(params.final_gather_lighting || params.finalGatherLighting) ?? 1.0,
    maxTraceDistance: validateOptionalNumber(params.max_trace_distance || params.maxTraceDistance),
    surfaceCacheResolution: validateOptionalNumber(params.surface_cache_resolution || params.surfaceCacheResolution),
    useHardwareRayTracing: validateOptionalBoolean(params.use_hardware_ray_tracing || params.useHardwareRayTracing)
  });
};

/**
 * Configure Lumen reflections
 */
const handleConfigureLumenReflections: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'configure_lumen_reflections', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    quality: validateOptionalEnum(params.quality, LUMEN_QUALITY_LEVELS),
    method: validateOptionalEnum(params.method, REFLECTION_METHODS),
    useHardwareRayTracing: validateOptionalBoolean(params.use_hardware_ray_tracing || params.useHardwareRayTracing),
    rayLightingMode: validateOptionalString(params.ray_lighting_mode || params.rayLightingMode), // 'surface_cache', 'hit_lighting'
    maxRoughness: validateOptionalNumber(params.max_roughness || params.maxRoughness) ?? 0.4,
    refractionQuality: validateOptionalNumber(params.refraction_quality || params.refractionQuality),
    screenSpaceEnabled: validateOptionalBoolean(params.screen_space_enabled || params.screenSpaceEnabled) ?? true,
    maxScreenSpaceRoughness: validateOptionalNumber(params.max_screen_space_roughness || params.maxScreenSpaceRoughness)
  });
};

/**
 * Configure emissive lighting for Lumen
 */
const handleConfigureEmissiveLighting: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'configure_emissive_lighting', {
    meshPath: validateOptionalString(params.mesh_path || params.meshPath),
    actorName: validateOptionalString(params.actor_name || params.actorName),
    emissiveBoost: validateOptionalNumber(params.emissive_boost || params.emissiveBoost) ?? 1.0,
    useAsLightSource: validateOptionalBoolean(params.use_as_light_source || params.useAsLightSource) ?? true,
    maxLuminanceForGI: validateOptionalNumber(params.max_luminance_for_gi || params.maxLuminanceForGI),
    affectDirectLighting: validateOptionalBoolean(params.affect_direct_lighting || params.affectDirectLighting) ?? false,
    affectIndirectLighting: validateOptionalBoolean(params.affect_indirect_lighting || params.affectIndirectLighting) ?? true
  });
};

/**
 * Optimize Lumen performance
 */
const handleOptimizeLumenPerformance: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'optimize_lumen_performance', {
    rayBudget: validateOptionalNumber(params.ray_budget || params.rayBudget),
    detailTracing: validateOptionalBoolean(params.detail_tracing || params.detailTracing) ?? true,
    farFieldDistanceThreshold: validateOptionalNumber(params.far_field_distance_threshold || params.farFieldDistanceThreshold),
    surfaceCacheUpdatesPerFrame: validateOptionalNumber(params.surface_cache_updates_per_frame || params.surfaceCacheUpdatesPerFrame),
    maxReflectionBounces: validateOptionalNumber(params.max_reflection_bounces || params.maxReflectionBounces) ?? 1,
    maxRefractionBounces: validateOptionalNumber(params.max_refraction_bounces || params.maxRefractionBounces) ?? 1,
    asyncCompute: validateOptionalBoolean(params.async_compute || params.asyncCompute) ?? true,
    useOcclusionCulling: validateOptionalBoolean(params.use_occlusion_culling || params.useOcclusionCulling) ?? true
  });
};

/**
 * Configure hardware ray tracing for Lumen
 */
const handleConfigureHardwareRayTracing: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'configure_hardware_ray_tracing', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    giEnabled: validateOptionalBoolean(params.gi_enabled || params.giEnabled) ?? true,
    reflectionsEnabled: validateOptionalBoolean(params.reflections_enabled || params.reflectionsEnabled) ?? true,
    shadowsEnabled: validateOptionalBoolean(params.shadows_enabled || params.shadowsEnabled) ?? false,
    aoEnabled: validateOptionalBoolean(params.ao_enabled || params.aoEnabled) ?? false,
    translucencyEnabled: validateOptionalBoolean(params.translucency_enabled || params.translucencyEnabled) ?? false,
    skyLightEnabled: validateOptionalBoolean(params.sky_light_enabled || params.skyLightEnabled) ?? true,
    maxRayHits: validateOptionalNumber(params.max_ray_hits || params.maxRayHits),
    rayCountScale: validateOptionalNumber(params.ray_count_scale || params.rayCountScale)
  });
};

/**
 * Configure Lumen scene capture
 */
const handleConfigureSceneCapture: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'configure_scene_capture', {
    surfaceCacheEnabled: validateOptionalBoolean(params.surface_cache_enabled || params.surfaceCacheEnabled) ?? true,
    cardUpdatePerFrame: validateOptionalNumber(params.card_update_per_frame || params.cardUpdatePerFrame),
    cardMaxResolution: validateOptionalNumber(params.card_max_resolution || params.cardMaxResolution),
    distanceFieldEnabled: validateOptionalBoolean(params.distance_field_enabled || params.distanceFieldEnabled) ?? true,
    distanceFieldResolution: validateOptionalNumber(params.distance_field_resolution || params.distanceFieldResolution),
    meshCardCoverage: validateOptionalNumber(params.mesh_card_coverage || params.meshCardCoverage)
  });
};

/**
 * Setup Lumen for interior lighting
 */
const handleSetupInteriorLighting: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'setup_interior_lighting', {
    roomBoundsActorName: validateOptionalString(params.room_bounds_actor_name || params.roomBoundsActorName),
    lightmassImportanceVolume: validateOptionalBoolean(params.lightmass_importance_volume || params.lightmassImportanceVolume) ?? true,
    indirectLightingIntensity: validateOptionalNumber(params.indirect_lighting_intensity || params.indirectLightingIntensity) ?? 1.0,
    skylightLeaking: validateOptionalBoolean(params.skylight_leaking || params.skylightLeaking) ?? false,
    surfaceCacheBias: validateOptionalNumber(params.surface_cache_bias || params.surfaceCacheBias),
    localExposure: validateOptionalBoolean(params.local_exposure || params.localExposure) ?? true
  });
};

/**
 * Get Lumen debug visualization
 */
const handleGetLumenDebugVisualization: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'get_lumen_debug_visualization', {
    mode: validateOptionalString(params.mode) ?? 'off', // 'off', 'lumen_scene', 'surface_cache', 'radiance_cache', 'final_gather'
    showTraceRays: validateOptionalBoolean(params.show_trace_rays || params.showTraceRays) ?? false,
    showProbes: validateOptionalBoolean(params.show_probes || params.showProbes) ?? false,
    showCards: validateOptionalBoolean(params.show_cards || params.showCards) ?? false
  });
};

/**
 * Get Lumen statistics
 */
const handleGetLumenStats: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'get_lumen_stats', {
    includeGIStats: validateOptionalBoolean(params.include_gi_stats || params.includeGIStats) ?? true,
    includeReflectionStats: validateOptionalBoolean(params.include_reflection_stats || params.includeReflectionStats) ?? true,
    includeSurfaceCacheStats: validateOptionalBoolean(params.include_surface_cache_stats || params.includeSurfaceCacheStats) ?? true,
    includePerformanceStats: validateOptionalBoolean(params.include_performance_stats || params.includePerformanceStats) ?? true
  });
};

/**
 * Configure Lumen sky lighting
 */
const handleConfigureSkyLighting: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LUMEN, 'configure_sky_lighting', {
    skyLightActorName: validateOptionalString(params.sky_light_actor_name || params.skyLightActorName),
    realTimeCapture: validateOptionalBoolean(params.real_time_capture || params.realTimeCapture) ?? false,
    lowerHemisphereColor: validateOptionalString(params.lower_hemisphere_color || params.lowerHemisphereColor),
    castRayTracedShadows: validateOptionalBoolean(params.cast_ray_traced_shadows || params.castRayTracedShadows),
    cloudAmbientOcclusion: validateOptionalBoolean(params.cloud_ambient_occlusion || params.cloudAmbientOcclusion),
    intensity: validateOptionalNumber(params.intensity) ?? 1.0
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const lumenHandlers: ActionHandlerMap = {
  // Core Lumen
  configure_lumen_gi: handleConfigureLumenGI,
  configure_lumen_reflections: handleConfigureLumenReflections,
  configure_emissive_lighting: handleConfigureEmissiveLighting,

  // Performance
  optimize_lumen_performance: handleOptimizeLumenPerformance,
  configure_hardware_ray_tracing: handleConfigureHardwareRayTracing,
  configure_scene_capture: handleConfigureSceneCapture,

  // Lighting setups
  setup_interior_lighting: handleSetupInteriorLighting,
  configure_sky_lighting: handleConfigureSkyLighting,

  // Debug and stats
  get_lumen_debug_visualization: handleGetLumenDebugVisualization,
  get_lumen_stats: handleGetLumenStats
};

export const handleLumenTools = createCategoryHandler(Forge2Categories.LUMEN, lumenHandlers);

export default lumenHandlers;
