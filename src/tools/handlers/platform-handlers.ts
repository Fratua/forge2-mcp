/**
 * FORGE2: Platform Configuration Handlers
 * Handles platform settings, platform input, and cross-platform configuration
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
// Platform Settings Action Handlers
// ============================================================================

/**
 * Configure platform settings
 */
const handleConfigurePlatformSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const platform = validateRequiredString(params.platform, 'platform');

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_platform_settings', {
    platform,
    targetFrameRate: validateOptionalNumber(params.target_frame_rate || params.targetFrameRate) ?? 60,
    vsyncEnabled: validateOptionalBoolean(params.vsync_enabled || params.vsyncEnabled) ?? true,
    maxTextureSize: validateOptionalNumber(params.max_texture_size || params.maxTextureSize),
    audioQuality: validateOptionalString(params.audio_quality || params.audioQuality) ?? 'high',
    memoryBudget: validateOptionalNumber(params.memory_budget || params.memoryBudget)
  });
};

/**
 * Configure console settings
 */
const handleConfigureConsoleSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const console = validateRequiredString(params.console, 'console');

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_console_settings', {
    console,
    supportHDR: validateOptionalBoolean(params.support_hdr || params.supportHDR) ?? true,
    support4K: validateOptionalBoolean(params.support_4k || params.support4K) ?? true,
    performanceMode: validateOptionalBoolean(params.performance_mode || params.performanceMode) ?? false,
    qualityMode: validateOptionalBoolean(params.quality_mode || params.qualityMode) ?? true,
    vrSupport: validateOptionalBoolean(params.vr_support || params.vrSupport) ?? false
  });
};

/**
 * Configure mobile settings
 */
const handleConfigureMobileSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const platform = validateRequiredString(params.platform, 'platform');

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_mobile_settings', {
    platform,
    supportedOrientations: validateOptionalArray(params.supported_orientations || params.supportedOrientations) ?? ['landscape'],
    minOSVersion: validateOptionalString(params.min_os_version || params.minOSVersion),
    requiresFullscreen: validateOptionalBoolean(params.requires_fullscreen || params.requiresFullscreen) ?? true,
    supportBackgroundAudio: validateOptionalBoolean(params.support_background_audio || params.supportBackgroundAudio) ?? false,
    appStoreCategory: validateOptionalString(params.app_store_category || params.appStoreCategory) ?? 'games'
  });
};

// ============================================================================
// Platform Input Action Handlers
// ============================================================================

/**
 * Configure platform input
 */
const handleConfigurePlatformInput: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const platform = validateRequiredString(params.platform, 'platform');

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_platform_input', {
    platform,
    inputDevice: validateOptionalString(params.input_device || params.inputDevice) ?? 'auto',
    controllerSupport: validateOptionalBoolean(params.controller_support || params.controllerSupport) ?? true,
    keyboardMouseSupport: validateOptionalBoolean(params.keyboard_mouse_support || params.keyboardMouseSupport) ?? true,
    touchSupport: validateOptionalBoolean(params.touch_support || params.touchSupport) ?? false
  });
};

/**
 * Configure touch input
 */
const handleConfigureTouchInput: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_touch_input', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    virtualJoystick: validateOptionalBoolean(params.virtual_joystick || params.virtualJoystick) ?? true,
    gestureSupport: validateOptionalBoolean(params.gesture_support || params.gestureSupport) ?? true,
    multiTouch: validateOptionalBoolean(params.multi_touch || params.multiTouch) ?? true,
    hapticFeedback: validateOptionalBoolean(params.haptic_feedback || params.hapticFeedback) ?? true,
    touchRadius: validateOptionalNumber(params.touch_radius || params.touchRadius) ?? 30
  });
};

/**
 * Configure controller settings
 */
const handleConfigureControllerSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_controller_settings', {
    deadZone: validateOptionalNumber(params.dead_zone || params.deadZone) ?? 0.25,
    sensitivity: validateOptionalNumber(params.sensitivity) ?? 1.0,
    invertYAxis: validateOptionalBoolean(params.invert_y_axis || params.invertYAxis) ?? false,
    vibrationEnabled: validateOptionalBoolean(params.vibration_enabled || params.vibrationEnabled) ?? true,
    triggerDeadZone: validateOptionalNumber(params.trigger_dead_zone || params.triggerDeadZone) ?? 0.1,
    adaptiveTriggers: validateOptionalBoolean(params.adaptive_triggers || params.adaptiveTriggers) ?? true
  });
};

// ============================================================================
// Platform Overrides Action Handlers
// ============================================================================

/**
 * Create platform override
 */
const handleCreatePlatformOverride: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const platform = validateRequiredString(params.platform, 'platform');
  const settingCategory = validateRequiredString(params.setting_category || params.settingCategory, 'setting_category');

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'create_platform_override', {
    platform,
    settingCategory,
    overrides: validateOptionalObject(params.overrides) ?? {}
  });
};

/**
 * Configure scalability
 */
const handleConfigureScalability: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const platform = validateRequiredString(params.platform, 'platform');

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_scalability', {
    platform,
    overallQuality: validateOptionalString(params.overall_quality || params.overallQuality) ?? 'high',
    resolutionScale: validateOptionalNumber(params.resolution_scale || params.resolutionScale) ?? 100,
    viewDistance: validateOptionalString(params.view_distance || params.viewDistance) ?? 'epic',
    antiAliasing: validateOptionalString(params.anti_aliasing || params.antiAliasing) ?? 'epic',
    shadows: validateOptionalString(params.shadows) ?? 'high',
    postProcess: validateOptionalString(params.post_process || params.postProcess) ?? 'high',
    textures: validateOptionalString(params.textures) ?? 'high',
    effects: validateOptionalString(params.effects) ?? 'high',
    foliage: validateOptionalString(params.foliage) ?? 'high'
  });
};

// ============================================================================
// Cross-Platform Action Handlers
// ============================================================================

/**
 * Configure cross-platform
 */
const handleConfigureCrossPlatform: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_cross_platform', {
    crossPlayEnabled: validateOptionalBoolean(params.cross_play_enabled || params.crossPlayEnabled) ?? false,
    crossSaveEnabled: validateOptionalBoolean(params.cross_save_enabled || params.crossSaveEnabled) ?? false,
    crossProgressionEnabled: validateOptionalBoolean(params.cross_progression_enabled || params.crossProgressionEnabled) ?? false,
    supportedPlatforms: validateOptionalArray(params.supported_platforms || params.supportedPlatforms) ?? [],
    inputBasedMatchmaking: validateOptionalBoolean(params.input_based_matchmaking || params.inputBasedMatchmaking) ?? true
  });
};

/**
 * Configure cloud saves
 */
const handleConfigureCloudSaves: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_cloud_saves', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    autoSync: validateOptionalBoolean(params.auto_sync || params.autoSync) ?? true,
    syncOnStart: validateOptionalBoolean(params.sync_on_start || params.syncOnStart) ?? true,
    conflictResolution: validateOptionalString(params.conflict_resolution || params.conflictResolution) ?? 'newest',
    maxSaveSize: validateOptionalNumber(params.max_save_size || params.maxSaveSize) ?? 10485760
  });
};

// ============================================================================
// Store Integration Action Handlers
// ============================================================================

/**
 * Configure store integration
 */
const handleConfigureStoreIntegration: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const store = validateRequiredString(params.store, 'store');

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'configure_store_integration', {
    store,
    achievementsEnabled: validateOptionalBoolean(params.achievements_enabled || params.achievementsEnabled) ?? true,
    leaderboardsEnabled: validateOptionalBoolean(params.leaderboards_enabled || params.leaderboardsEnabled) ?? true,
    richPresenceEnabled: validateOptionalBoolean(params.rich_presence_enabled || params.richPresenceEnabled) ?? true,
    dlcSupport: validateOptionalBoolean(params.dlc_support || params.dlcSupport) ?? true,
    workshopSupport: validateOptionalBoolean(params.workshop_support || params.workshopSupport) ?? false
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get platform info
 */
const handleGetPlatformInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'get_platform_info', {
    platform: validateOptionalString(params.platform),
    includeSettings: validateOptionalBoolean(params.include_settings || params.includeSettings) ?? true,
    includeOverrides: validateOptionalBoolean(params.include_overrides || params.includeOverrides) ?? true
  });
};

/**
 * List supported platforms
 */
const handleListSupportedPlatforms: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'list_supported_platforms', {
    includeConsoles: validateOptionalBoolean(params.include_consoles || params.includeConsoles) ?? true,
    includeMobile: validateOptionalBoolean(params.include_mobile || params.includeMobile) ?? true,
    includePC: validateOptionalBoolean(params.include_pc || params.includePC) ?? true
  });
};

/**
 * Get scalability settings
 */
const handleGetScalabilitySettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PLATFORMS, 'get_scalability_settings', {
    platform: validateOptionalString(params.platform),
    qualityLevel: validateOptionalString(params.quality_level || params.qualityLevel)
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const platformHandlers: ActionHandlerMap = {
  // Platform Settings
  configure_platform_settings: handleConfigurePlatformSettings,
  configure_console_settings: handleConfigureConsoleSettings,
  configure_mobile_settings: handleConfigureMobileSettings,

  // Platform Input
  configure_platform_input: handleConfigurePlatformInput,
  configure_touch_input: handleConfigureTouchInput,
  configure_controller_settings: handleConfigureControllerSettings,

  // Platform Overrides
  create_platform_override: handleCreatePlatformOverride,
  configure_scalability: handleConfigureScalability,

  // Cross-Platform
  configure_cross_platform: handleConfigureCrossPlatform,
  configure_cloud_saves: handleConfigureCloudSaves,

  // Store Integration
  configure_store_integration: handleConfigureStoreIntegration,

  // Query
  get_platform_info: handleGetPlatformInfo,
  list_supported_platforms: handleListSupportedPlatforms,
  get_scalability_settings: handleGetScalabilitySettings
};

export const handlePlatformTools = createCategoryHandler(Forge2Categories.PLATFORMS, platformHandlers);

export default platformHandlers;
