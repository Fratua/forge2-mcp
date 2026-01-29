/**
 * FORGE2: Accessibility Handlers
 * Handles colorblind modes, subtitles, audio cues, input remapping, motor accessibility
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
// Colorblind Mode Action Handlers
// ============================================================================

/**
 * Configure colorblind modes
 */
const handleConfigureColorblindModes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'configure_colorblind_modes', {
    protanopiaEnabled: validateOptionalBoolean(params.protanopia_enabled || params.protanopiaEnabled) ?? true,
    deuteranopiaEnabled: validateOptionalBoolean(params.deuteranopia_enabled || params.deuteranopiaEnabled) ?? true,
    tritanopiaEnabled: validateOptionalBoolean(params.tritanopia_enabled || params.tritanopiaEnabled) ?? true,
    customFilters: validateOptionalArray(params.custom_filters || params.customFilters),
    intensity: validateOptionalNumber(params.intensity) ?? 1.0,
    affectUI: validateOptionalBoolean(params.affect_ui || params.affectUI) ?? true,
    affectWorld: validateOptionalBoolean(params.affect_world || params.affectWorld) ?? true
  });
};

/**
 * Add colorblind filter
 */
const handleAddColorblindFilter: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const filterName = validateRequiredString(params.filter_name || params.filterName, 'filter_name');

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'add_colorblind_filter', {
    filterName,
    colorMatrix: validateOptionalArray(params.color_matrix || params.colorMatrix),
    lutTexture: validateOptionalString(params.lut_texture || params.lutTexture),
    description: validateOptionalString(params.description)
  });
};

// ============================================================================
// Subtitle Action Handlers
// ============================================================================

/**
 * Setup subtitle options
 */
const handleSetupSubtitleOptions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'setup_subtitle_options', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    defaultSize: validateOptionalString(params.default_size || params.defaultSize) ?? 'medium',
    sizes: validateOptionalArray(params.sizes) ?? ['small', 'medium', 'large', 'extra_large'],
    backgroundEnabled: validateOptionalBoolean(params.background_enabled || params.backgroundEnabled) ?? true,
    backgroundOpacity: validateOptionalNumber(params.background_opacity || params.backgroundOpacity) ?? 0.7,
    speakerNames: validateOptionalBoolean(params.speaker_names || params.speakerNames) ?? true,
    speakerColors: validateOptionalBoolean(params.speaker_colors || params.speakerColors) ?? true,
    soundDescriptions: validateOptionalBoolean(params.sound_descriptions || params.soundDescriptions) ?? false,
    directionIndicators: validateOptionalBoolean(params.direction_indicators || params.directionIndicators) ?? false
  });
};

/**
 * Configure subtitle appearance
 */
const handleConfigureSubtitleAppearance: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'configure_subtitle_appearance', {
    fontFamily: validateOptionalString(params.font_family || params.fontFamily),
    textColor: validateOptionalObject(params.text_color || params.textColor),
    outlineEnabled: validateOptionalBoolean(params.outline_enabled || params.outlineEnabled) ?? true,
    outlineColor: validateOptionalObject(params.outline_color || params.outlineColor),
    outlineWidth: validateOptionalNumber(params.outline_width || params.outlineWidth) ?? 2,
    position: validateOptionalString(params.position) ?? 'bottom',
    maxLines: validateOptionalNumber(params.max_lines || params.maxLines) ?? 3,
    lineSpacing: validateOptionalNumber(params.line_spacing || params.lineSpacing) ?? 1.2
  });
};

// ============================================================================
// Audio Cue Action Handlers
// ============================================================================

/**
 * Create audio cues
 */
const handleCreateAudioCues: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'create_audio_cues', {
    enabled: validateOptionalBoolean(params.enabled) ?? false,
    cueTypes: validateOptionalArray(params.cue_types || params.cueTypes) ?? ['item_pickup', 'enemy_nearby', 'objective', 'hazard'],
    spatialAudio: validateOptionalBoolean(params.spatial_audio || params.spatialAudio) ?? true,
    volumeMultiplier: validateOptionalNumber(params.volume_multiplier || params.volumeMultiplier) ?? 1.0
  });
};

/**
 * Add audio cue
 */
const handleAddAudioCue: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const cueName = validateRequiredString(params.cue_name || params.cueName, 'cue_name');

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'add_audio_cue', {
    cueName,
    triggerEvent: validateRequiredString(params.trigger_event || params.triggerEvent, 'trigger_event'),
    soundPath: validateOptionalString(params.sound_path || params.soundPath),
    pitch: validateOptionalNumber(params.pitch) ?? 1.0,
    volume: validateOptionalNumber(params.volume) ?? 1.0,
    cooldown: validateOptionalNumber(params.cooldown) ?? 0.5,
    description: validateOptionalString(params.description)
  });
};

// ============================================================================
// Input Accessibility Action Handlers
// ============================================================================

/**
 * Configure input remapping
 */
const handleConfigureInputRemapping: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'configure_input_remapping', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    allowDuplicates: validateOptionalBoolean(params.allow_duplicates || params.allowDuplicates) ?? false,
    separateProfiles: validateOptionalBoolean(params.separate_profiles || params.separateProfiles) ?? true,
    maxProfiles: validateOptionalNumber(params.max_profiles || params.maxProfiles) ?? 5,
    resetToDefault: validateOptionalBoolean(params.reset_to_default || params.resetToDefault) ?? true
  });
};

/**
 * Setup difficulty options
 */
const handleSetupDifficultyOptions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'setup_difficulty_options', {
    assistModes: validateOptionalArray(params.assist_modes || params.assistModes) ?? ['auto_aim', 'combat_assist', 'puzzle_hints'],
    godModeEnabled: validateOptionalBoolean(params.god_mode_enabled || params.godModeEnabled) ?? true,
    skipEnabled: validateOptionalBoolean(params.skip_enabled || params.skipEnabled) ?? true,
    skipTypes: validateOptionalArray(params.skip_types || params.skipTypes) ?? ['puzzle', 'combat', 'cutscene'],
    damageScaling: validateOptionalBoolean(params.damage_scaling || params.damageScaling) ?? true,
    resourceMultiplier: validateOptionalBoolean(params.resource_multiplier || params.resourceMultiplier) ?? true
  });
};

// ============================================================================
// Motor Accessibility Action Handlers
// ============================================================================

/**
 * Configure motor accessibility
 */
const handleConfigureMotorAccessibility: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'configure_motor_accessibility', {
    holdVsToggle: validateOptionalBoolean(params.hold_vs_toggle || params.holdVsToggle) ?? true,
    autoAimEnabled: validateOptionalBoolean(params.auto_aim_enabled || params.autoAimEnabled) ?? true,
    autoAimStrength: validateOptionalNumber(params.auto_aim_strength || params.autoAimStrength) ?? 0.5,
    qteAssist: validateOptionalBoolean(params.qte_assist || params.qteAssist) ?? true,
    qteTimingMultiplier: validateOptionalNumber(params.qte_timing_multiplier || params.qteTimingMultiplier) ?? 1.5,
    repeatDelay: validateOptionalNumber(params.repeat_delay || params.repeatDelay) ?? 0.0,
    stickyKeys: validateOptionalBoolean(params.sticky_keys || params.stickyKeys) ?? false
  });
};

/**
 * Setup one-handed mode
 */
const handleSetupOneHandedMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'setup_one_handed_mode', {
    enabled: validateOptionalBoolean(params.enabled) ?? false,
    preferredHand: validateOptionalString(params.preferred_hand || params.preferredHand) ?? 'right',
    autoMovement: validateOptionalBoolean(params.auto_movement || params.autoMovement) ?? true,
    simplifiedControls: validateOptionalBoolean(params.simplified_controls || params.simplifiedControls) ?? true,
    customLayout: validateOptionalObject(params.custom_layout || params.customLayout)
  });
};

// ============================================================================
// Screen Reader Action Handlers
// ============================================================================

/**
 * Create screen reader support
 */
const handleCreateScreenReaderSupport: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'create_screen_reader_support', {
    enabled: validateOptionalBoolean(params.enabled) ?? false,
    narrateUI: validateOptionalBoolean(params.narrate_ui || params.narrateUI) ?? true,
    narrateWorld: validateOptionalBoolean(params.narrate_world || params.narrateWorld) ?? true,
    narrateDialogue: validateOptionalBoolean(params.narrate_dialogue || params.narrateDialogue) ?? true,
    speechRate: validateOptionalNumber(params.speech_rate || params.speechRate) ?? 1.0,
    voice: validateOptionalString(params.voice)
  });
};

// ============================================================================
// Visual Accessibility Action Handlers
// ============================================================================

/**
 * Configure visual accessibility
 */
const handleConfigureVisualAccessibility: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'configure_visual_accessibility', {
    highContrastMode: validateOptionalBoolean(params.high_contrast_mode || params.highContrastMode) ?? false,
    largeTextMode: validateOptionalBoolean(params.large_text_mode || params.largeTextMode) ?? false,
    textScaling: validateOptionalNumber(params.text_scaling || params.textScaling) ?? 1.0,
    reduceMotion: validateOptionalBoolean(params.reduce_motion || params.reduceMotion) ?? false,
    flashingEffects: validateOptionalBoolean(params.flashing_effects || params.flashingEffects) ?? true,
    screenShake: validateOptionalBoolean(params.screen_shake || params.screenShake) ?? true,
    motionBlur: validateOptionalBoolean(params.motion_blur || params.motionBlur) ?? true
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get accessibility info
 */
const handleGetAccessibilityInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'get_accessibility_info', {
    category: validateOptionalString(params.category),
    includeAllSettings: validateOptionalBoolean(params.include_all_settings || params.includeAllSettings) ?? true
  });
};

/**
 * List accessibility presets
 */
const handleListAccessibilityPresets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACCESSIBILITY, 'list_accessibility_presets', {
    category: validateOptionalString(params.category)
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const accessibilityHandlers: ActionHandlerMap = {
  // Colorblind
  configure_colorblind_modes: handleConfigureColorblindModes,
  add_colorblind_filter: handleAddColorblindFilter,

  // Subtitles
  setup_subtitle_options: handleSetupSubtitleOptions,
  configure_subtitle_appearance: handleConfigureSubtitleAppearance,

  // Audio Cues
  create_audio_cues: handleCreateAudioCues,
  add_audio_cue: handleAddAudioCue,

  // Input
  configure_input_remapping: handleConfigureInputRemapping,
  setup_difficulty_options: handleSetupDifficultyOptions,

  // Motor
  configure_motor_accessibility: handleConfigureMotorAccessibility,
  setup_one_handed_mode: handleSetupOneHandedMode,

  // Screen Reader
  create_screen_reader_support: handleCreateScreenReaderSupport,

  // Visual
  configure_visual_accessibility: handleConfigureVisualAccessibility,

  // Query
  get_accessibility_info: handleGetAccessibilityInfo,
  list_accessibility_presets: handleListAccessibilityPresets
};

export const handleAccessibilityTools = createCategoryHandler(Forge2Categories.ACCESSIBILITY, accessibilityHandlers);

export default accessibilityHandlers;
