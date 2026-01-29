/**
 * FORGE2: Photo Mode Handlers
 * Handles camera controls, filters, poses, frames, depth of field, and sharing
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
// Photo Mode Core Action Handlers
// ============================================================================

/**
 * Create photo mode system
 */
const handleCreatePhotoMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'create_photo_mode', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    pauseGame: validateOptionalBoolean(params.pause_game || params.pauseGame) ?? true,
    hideUI: validateOptionalBoolean(params.hide_ui || params.hideUI) ?? true,
    hidePlayer: validateOptionalBoolean(params.hide_player || params.hidePlayer) ?? false,
    maxCameraDistance: validateOptionalNumber(params.max_camera_distance || params.maxCameraDistance) ?? 1000.0,
    allowNPCControl: validateOptionalBoolean(params.allow_npc_control || params.allowNPCControl) ?? false
  });
};

/**
 * Configure camera controls
 */
const handleConfigureCameraControls: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'configure_camera_controls', {
    fovRange: validateOptionalObject(params.fov_range || params.fovRange) ?? { min: 20, max: 120 },
    rollEnabled: validateOptionalBoolean(params.roll_enabled || params.rollEnabled) ?? true,
    rollRange: validateOptionalObject(params.roll_range || params.rollRange) ?? { min: -180, max: 180 },
    orbitSpeed: validateOptionalNumber(params.orbit_speed || params.orbitSpeed) ?? 1.0,
    panSpeed: validateOptionalNumber(params.pan_speed || params.panSpeed) ?? 1.0,
    zoomSpeed: validateOptionalNumber(params.zoom_speed || params.zoomSpeed) ?? 1.0,
    smoothing: validateOptionalNumber(params.smoothing) ?? 0.5
  });
};

// ============================================================================
// Filter Action Handlers
// ============================================================================

/**
 * Setup filters
 */
const handleSetupFilters: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'setup_filters', {
    presetFilters: validateOptionalArray(params.preset_filters || params.presetFilters) ?? ['none', 'vintage', 'noir', 'sepia', 'vibrant', 'cool', 'warm', 'dramatic'],
    customFiltersEnabled: validateOptionalBoolean(params.custom_filters_enabled || params.customFiltersEnabled) ?? true,
    lutSupport: validateOptionalBoolean(params.lut_support || params.lutSupport) ?? true
  });
};

/**
 * Add filter preset
 */
const handleAddFilterPreset: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const filterName = validateRequiredString(params.filter_name || params.filterName, 'filter_name');

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'add_filter_preset', {
    filterName,
    saturation: validateOptionalNumber(params.saturation) ?? 1.0,
    contrast: validateOptionalNumber(params.contrast) ?? 1.0,
    brightness: validateOptionalNumber(params.brightness) ?? 1.0,
    temperature: validateOptionalNumber(params.temperature) ?? 0.0,
    tint: validateOptionalNumber(params.tint) ?? 0.0,
    vignetteIntensity: validateOptionalNumber(params.vignette_intensity || params.vignetteIntensity) ?? 0.0,
    grainIntensity: validateOptionalNumber(params.grain_intensity || params.grainIntensity) ?? 0.0,
    lutTexture: validateOptionalString(params.lut_texture || params.lutTexture)
  });
};

/**
 * Configure color grading
 */
const handleConfigureColorGrading: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'configure_color_grading', {
    exposureRange: validateOptionalObject(params.exposure_range || params.exposureRange) ?? { min: -4, max: 4 },
    saturationRange: validateOptionalObject(params.saturation_range || params.saturationRange) ?? { min: 0, max: 2 },
    contrastRange: validateOptionalObject(params.contrast_range || params.contrastRange) ?? { min: 0.5, max: 2 },
    highlightsEnabled: validateOptionalBoolean(params.highlights_enabled || params.highlightsEnabled) ?? true,
    shadowsEnabled: validateOptionalBoolean(params.shadows_enabled || params.shadowsEnabled) ?? true,
    midtonesEnabled: validateOptionalBoolean(params.midtones_enabled || params.midtonesEnabled) ?? true
  });
};

// ============================================================================
// Pose Action Handlers
// ============================================================================

/**
 * Create pose library
 */
const handleCreatePoseLibrary: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'create_pose_library', {
    libraryName: validateOptionalString(params.library_name || params.libraryName) ?? 'DefaultPoses',
    categories: validateOptionalArray(params.categories) ?? ['idle', 'action', 'emotion', 'victory', 'defeat'],
    npcPosesEnabled: validateOptionalBoolean(params.npc_poses_enabled || params.npcPosesEnabled) ?? false,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add pose
 */
const handleAddPose: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const poseName = validateRequiredString(params.pose_name || params.poseName, 'pose_name');

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'add_pose', {
    poseName,
    category: validateOptionalString(params.category) ?? 'idle',
    animationPath: validateOptionalString(params.animation_path || params.animationPath),
    thumbnailPath: validateOptionalString(params.thumbnail_path || params.thumbnailPath),
    faceExpression: validateOptionalString(params.face_expression || params.faceExpression),
    unlockRequirement: validateOptionalString(params.unlock_requirement || params.unlockRequirement)
  });
};

// ============================================================================
// Depth of Field Action Handlers
// ============================================================================

/**
 * Setup depth of field
 */
const handleSetupDepthOfField: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'setup_depth_of_field', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    method: validateOptionalString(params.method) ?? 'cinematic',
    focalDistanceRange: validateOptionalObject(params.focal_distance_range || params.focalDistanceRange) ?? { min: 10, max: 10000 },
    apertureRange: validateOptionalObject(params.aperture_range || params.apertureRange) ?? { min: 1.4, max: 22 },
    autoFocusEnabled: validateOptionalBoolean(params.auto_focus_enabled || params.autoFocusEnabled) ?? true,
    bokehShape: validateOptionalString(params.bokeh_shape || params.bokehShape) ?? 'circle'
  });
};

// ============================================================================
// Frame Action Handlers
// ============================================================================

/**
 * Create frame options
 */
const handleCreateFrameOptions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'create_frame_options', {
    borderStyles: validateOptionalArray(params.border_styles || params.borderStyles) ?? ['none', 'simple', 'ornate', 'vintage', 'modern'],
    aspectRatios: validateOptionalArray(params.aspect_ratios || params.aspectRatios) ?? ['16:9', '4:3', '1:1', '21:9', '9:16'],
    watermarkEnabled: validateOptionalBoolean(params.watermark_enabled || params.watermarkEnabled) ?? true,
    watermarkPath: validateOptionalString(params.watermark_path || params.watermarkPath),
    timestampEnabled: validateOptionalBoolean(params.timestamp_enabled || params.timestampEnabled) ?? false,
    locationTagEnabled: validateOptionalBoolean(params.location_tag_enabled || params.locationTagEnabled) ?? false
  });
};

/**
 * Add frame style
 */
const handleAddFrameStyle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const styleName = validateRequiredString(params.style_name || params.styleName, 'style_name');

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'add_frame_style', {
    styleName,
    borderTexture: validateOptionalString(params.border_texture || params.borderTexture),
    borderWidth: validateOptionalNumber(params.border_width || params.borderWidth) ?? 10,
    borderColor: validateOptionalObject(params.border_color || params.borderColor),
    cornerStyle: validateOptionalString(params.corner_style || params.cornerStyle) ?? 'square',
    unlockRequirement: validateOptionalString(params.unlock_requirement || params.unlockRequirement)
  });
};

// ============================================================================
// Output Action Handlers
// ============================================================================

/**
 * Configure screenshot settings
 */
const handleConfigureScreenshotSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'configure_screenshot_settings', {
    defaultResolution: validateOptionalString(params.default_resolution || params.defaultResolution) ?? '1920x1080',
    resolutionOptions: validateOptionalArray(params.resolution_options || params.resolutionOptions) ?? ['1920x1080', '2560x1440', '3840x2160', '7680x4320'],
    format: validateOptionalString(params.format) ?? 'png',
    quality: validateOptionalNumber(params.quality) ?? 100,
    superSampling: validateOptionalNumber(params.super_sampling || params.superSampling) ?? 1,
    saveLocation: validateOptionalString(params.save_location || params.saveLocation)
  });
};

/**
 * Setup sharing options
 */
const handleSetupSharingOptions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'setup_sharing_options', {
    directShare: validateOptionalBoolean(params.direct_share || params.directShare) ?? true,
    platforms: validateOptionalArray(params.platforms) ?? ['steam', 'twitter', 'clipboard'],
    galleryEnabled: validateOptionalBoolean(params.gallery_enabled || params.galleryEnabled) ?? true,
    maxGallerySize: validateOptionalNumber(params.max_gallery_size || params.maxGallerySize) ?? 100,
    cloudBackup: validateOptionalBoolean(params.cloud_backup || params.cloudBackup) ?? false
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get photo mode info
 */
const handleGetPhotoModeInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'get_photo_mode_info', {
    includeFilters: validateOptionalBoolean(params.include_filters || params.includeFilters) ?? true,
    includePoses: validateOptionalBoolean(params.include_poses || params.includePoses) ?? true,
    includeFrames: validateOptionalBoolean(params.include_frames || params.includeFrames) ?? true
  });
};

/**
 * List photo mode assets
 */
const handleListPhotoModeAssets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PHOTO_MODE, 'list_photo_mode_assets', {
    assetType: validateOptionalString(params.asset_type || params.assetType),
    includeUnlocked: validateOptionalBoolean(params.include_unlocked || params.includeUnlocked) ?? true,
    includeLocked: validateOptionalBoolean(params.include_locked || params.includeLocked) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const photoModeHandlers: ActionHandlerMap = {
  // Core
  create_photo_mode: handleCreatePhotoMode,
  configure_camera_controls: handleConfigureCameraControls,

  // Filters
  setup_filters: handleSetupFilters,
  add_filter_preset: handleAddFilterPreset,
  configure_color_grading: handleConfigureColorGrading,

  // Poses
  create_pose_library: handleCreatePoseLibrary,
  add_pose: handleAddPose,

  // DOF
  setup_depth_of_field: handleSetupDepthOfField,

  // Frames
  create_frame_options: handleCreateFrameOptions,
  add_frame_style: handleAddFrameStyle,

  // Output
  configure_screenshot_settings: handleConfigureScreenshotSettings,
  setup_sharing_options: handleSetupSharingOptions,

  // Query
  get_photo_mode_info: handleGetPhotoModeInfo,
  list_photo_mode_assets: handleListPhotoModeAssets
};

export const handlePhotoModeTools = createCategoryHandler(Forge2Categories.PHOTO_MODE, photoModeHandlers);

export default photoModeHandlers;
