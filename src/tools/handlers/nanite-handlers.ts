/**
 * FORGE2: Nanite Handlers
 * Handles UE5 Nanite virtualized geometry configuration, optimization, and management
 * Supports mesh conversion, fallback settings, displacement, and LOD configuration
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

// Note: Enum validators defined but reserved for future use when validation is needed
// function validateRequiredEnum / validateOptionalEnum - see other handlers for pattern

function validateOptionalArray(value: unknown): unknown[] | undefined {
  if (value === undefined || value === null) return undefined;
  if (Array.isArray(value)) return value;
  return undefined;
}

// ============================================================================
// Constants
// ============================================================================

const FALLBACK_MODES = ['none', 'proxies', 'static_mesh'] as const;
const DISPLACEMENT_MODES = ['none', 'vertex', 'tessellation'] as const;
const NANITE_QUALITY_LEVELS = ['low', 'medium', 'high', 'epic', 'cinematic'] as const;
const TRIM_METHODS = ['none', 'percentile', 'threshold'] as const;

// Reserve constants for future use
void FALLBACK_MODES;
void DISPLACEMENT_MODES;
void NANITE_QUALITY_LEVELS;
void TRIM_METHODS;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Enable Nanite on a static mesh
 */
const handleEnableNanite: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const meshPath = validateRequiredString(params.mesh_path || params.meshPath, 'mesh_path');

  return executeForge2Action(tools, Forge2Categories.NANITE, 'enable_nanite', {
    meshPath,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    preserveArea: validateOptionalBoolean(params.preserve_area || params.preserveArea) ?? true,
    positionPrecision: validateOptionalNumber(params.position_precision || params.positionPrecision),
    percentTriangles: validateOptionalNumber(params.percent_triangles || params.percentTriangles) ?? 100,
    maxEdgeLength: validateOptionalNumber(params.max_edge_length || params.maxEdgeLength)
  });
};

/**
 * Configure Nanite settings for a mesh
 */
const handleConfigureNaniteSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const meshPath = validateRequiredString(params.mesh_path || params.meshPath, 'mesh_path');

  return executeForge2Action(tools, Forge2Categories.NANITE, 'configure_nanite_settings', {
    meshPath,
    fallbackRelativeError: validateOptionalNumber(params.fallback_relative_error || params.fallbackRelativeError) ?? 1.0,
    fallbackPercentTriangles: validateOptionalNumber(params.fallback_percent_triangles || params.fallbackPercentTriangles) ?? 100,
    trimRelativeError: validateOptionalNumber(params.trim_relative_error || params.trimRelativeError),
    displacementMagnitude: validateOptionalNumber(params.displacement_magnitude || params.displacementMagnitude),
    maxVertexCount: validateOptionalNumber(params.max_vertex_count || params.maxVertexCount),
    keepPercentTriangles: validateOptionalNumber(params.keep_percent_triangles || params.keepPercentTriangles)
  });
};

/**
 * Setup Nanite foliage (UE 5.7+ voxel foliage)
 */
const handleSetupNaniteFoliage: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const foliageTypePath = validateRequiredString(params.foliage_type_path || params.foliageTypePath, 'foliage_type_path');

  return executeForge2Action(tools, Forge2Categories.NANITE, 'setup_nanite_foliage', {
    foliageTypePath,
    enableNanite: validateOptionalBoolean(params.enable_nanite || params.enableNanite) ?? true,
    voxelFoliageEnabled: validateOptionalBoolean(params.voxel_foliage_enabled || params.voxelFoliageEnabled) ?? false,
    cullDistance: validateOptionalNumber(params.cull_distance || params.cullDistance),
    maxDrawDistance: validateOptionalNumber(params.max_draw_distance || params.maxDrawDistance),
    minLodIndex: validateOptionalNumber(params.min_lod_index || params.minLodIndex),
    maxLodIndex: validateOptionalNumber(params.max_lod_index || params.maxLodIndex),
    lodBias: validateOptionalNumber(params.lod_bias || params.lodBias)
  });
};

/**
 * Optimize Nanite scene
 */
const handleOptimizeNaniteScene: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NANITE, 'optimize_nanite_scene', {
    levelPath: validateOptionalString(params.level_path || params.levelPath),
    reduceOverdraw: validateOptionalBoolean(params.reduce_overdraw || params.reduceOverdraw) ?? true,
    optimizeClusters: validateOptionalBoolean(params.optimize_clusters || params.optimizeClusters) ?? true,
    targetFrameBudget: validateOptionalNumber(params.target_frame_budget || params.targetFrameBudget),
    maxInstancesPerCluster: validateOptionalNumber(params.max_instances_per_cluster || params.maxInstancesPerCluster),
    clusterPixelErrorThreshold: validateOptionalNumber(params.cluster_pixel_error_threshold || params.clusterPixelErrorThreshold)
  });
};

/**
 * Configure Nanite render settings
 */
const handleConfigureNaniteRender: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NANITE, 'configure_nanite_render', {
    maxPixelsPerEdge: validateOptionalNumber(params.max_pixels_per_edge || params.maxPixelsPerEdge) ?? 1.0,
    minPixelsPerEdgeHigh: validateOptionalNumber(params.min_pixels_per_edge_high || params.minPixelsPerEdgeHigh),
    maxCandidateClusters: validateOptionalNumber(params.max_candidate_clusters || params.maxCandidateClusters),
    maxVisibleClusters: validateOptionalNumber(params.max_visible_clusters || params.maxVisibleClusters),
    streamingPoolSize: validateOptionalNumber(params.streaming_pool_size || params.streamingPoolSize),
    debugMode: validateOptionalString(params.debug_mode || params.debugMode), // 'off', 'clusters', 'triangles', 'overdraw'
    forceFullUpdate: validateOptionalBoolean(params.force_full_update || params.forceFullUpdate)
  });
};

/**
 * Batch convert meshes to Nanite
 */
const handleBatchConvertToNanite: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const meshPaths = validateOptionalArray(params.mesh_paths || params.meshPaths) ?? [];
  const folderPath = validateOptionalString(params.folder_path || params.folderPath);

  if (meshPaths.length === 0 && !folderPath) {
    throw new Error('Either mesh_paths or folder_path is required');
  }

  return executeForge2Action(tools, Forge2Categories.NANITE, 'batch_convert_to_nanite', {
    meshPaths,
    folderPath,
    recursive: validateOptionalBoolean(params.recursive) ?? true,
    overwriteExisting: validateOptionalBoolean(params.overwrite_existing || params.overwriteExisting) ?? false,
    preserveArea: validateOptionalBoolean(params.preserve_area || params.preserveArea) ?? true,
    positionPrecision: validateOptionalNumber(params.position_precision || params.positionPrecision)
  });
};

/**
 * Check Nanite status of a mesh
 */
const handleGetNaniteStatus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const meshPath = validateRequiredString(params.mesh_path || params.meshPath, 'mesh_path');

  return executeForge2Action(tools, Forge2Categories.NANITE, 'get_nanite_status', {
    meshPath,
    includeStats: validateOptionalBoolean(params.include_stats || params.includeStats) ?? true,
    includeClusterInfo: validateOptionalBoolean(params.include_cluster_info || params.includeClusterInfo) ?? false
  });
};

/**
 * Configure Nanite displacement
 */
const handleConfigureDisplacement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const meshPath = validateRequiredString(params.mesh_path || params.meshPath, 'mesh_path');

  return executeForge2Action(tools, Forge2Categories.NANITE, 'configure_displacement', {
    meshPath,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    magnitude: validateOptionalNumber(params.magnitude) ?? 1.0,
    center: validateOptionalNumber(params.center) ?? 0.5,
    displacementMapPath: validateOptionalString(params.displacement_map_path || params.displacementMapPath),
    uvChannel: validateOptionalNumber(params.uv_channel || params.uvChannel) ?? 0
  });
};

/**
 * Get Nanite scene statistics
 */
const handleGetNaniteSceneStats: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NANITE, 'get_nanite_scene_stats', {
    levelPath: validateOptionalString(params.level_path || params.levelPath),
    includePerMeshStats: validateOptionalBoolean(params.include_per_mesh_stats || params.includePerMeshStats) ?? false,
    includeClusterStats: validateOptionalBoolean(params.include_cluster_stats || params.includeClusterStats) ?? false,
    includeStreamingStats: validateOptionalBoolean(params.include_streaming_stats || params.includeStreamingStats) ?? true
  });
};

/**
 * List Nanite-enabled meshes
 */
const handleListNaniteMeshes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NANITE, 'list_nanite_meshes', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true,
    includeStats: validateOptionalBoolean(params.include_stats || params.includeStats) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const naniteHandlers: ActionHandlerMap = {
  // Core Nanite
  enable_nanite: handleEnableNanite,
  configure_nanite_settings: handleConfigureNaniteSettings,
  configure_displacement: handleConfigureDisplacement,

  // Foliage
  setup_nanite_foliage: handleSetupNaniteFoliage,

  // Optimization
  optimize_nanite_scene: handleOptimizeNaniteScene,
  configure_nanite_render: handleConfigureNaniteRender,

  // Batch operations
  batch_convert_to_nanite: handleBatchConvertToNanite,

  // Query
  get_nanite_status: handleGetNaniteStatus,
  get_nanite_scene_stats: handleGetNaniteSceneStats,
  list_nanite_meshes: handleListNaniteMeshes
};

export const handleNaniteTools = createCategoryHandler(Forge2Categories.NANITE, naniteHandlers);

export default naniteHandlers;
