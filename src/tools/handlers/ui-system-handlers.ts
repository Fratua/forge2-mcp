/**
 * FORGE2: UI System Handlers
 * Handles HUD, menus, notifications, tutorials, and UI management
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
// Constants
// ============================================================================

const HUD_ELEMENTS = ['health', 'mana', 'stamina', 'minimap', 'compass', 'crosshair', 'ammo', 'objectives', 'buffs'] as const;
const MENU_TYPES = ['main', 'pause', 'settings', 'inventory', 'character', 'map', 'quest', 'social'] as const;
const NOTIFICATION_TYPES = ['toast', 'banner', 'popup', 'alert', 'achievement'] as const;

// Reserve constants for future use
void HUD_ELEMENTS;
void MENU_TYPES;
void NOTIFICATION_TYPES;

// ============================================================================
// HUD Action Handlers
// ============================================================================

/**
 * Create HUD
 */
const handleCreateHUD: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const hudName = validateRequiredString(params.hud_name || params.hudName, 'hud_name');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'create_hud', {
    hudName,
    elements: validateOptionalArray(params.elements) ?? ['health', 'minimap', 'objectives'],
    scalingMode: validateOptionalString(params.scaling_mode || params.scalingMode) ?? 'scale_to_fit',
    safeZoneEnabled: validateOptionalBoolean(params.safe_zone_enabled || params.safeZoneEnabled) ?? true,
    hideOnMenu: validateOptionalBoolean(params.hide_on_menu || params.hideOnMenu) ?? true,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add HUD element
 */
const handleAddHUDElement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const hudPath = validateRequiredString(params.hud_path || params.hudPath, 'hud_path');
  const elementType = validateRequiredString(params.element_type || params.elementType, 'element_type');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'add_hud_element', {
    hudPath,
    elementType,
    position: validateOptionalObject(params.position),
    size: validateOptionalObject(params.size),
    anchor: validateOptionalString(params.anchor) ?? 'top_left',
    alignment: validateOptionalString(params.alignment) ?? 'left',
    visibility: validateOptionalString(params.visibility) ?? 'always',
    binding: validateOptionalString(params.binding)
  });
};

/**
 * Configure minimap
 */
const handleConfigureMinimap: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const hudPath = validateRequiredString(params.hud_path || params.hudPath, 'hud_path');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'configure_minimap', {
    hudPath,
    shape: validateOptionalString(params.shape) ?? 'circle',
    size: validateOptionalNumber(params.size) ?? 200,
    zoom: validateOptionalNumber(params.zoom) ?? 1.0,
    rotateWithPlayer: validateOptionalBoolean(params.rotate_with_player || params.rotateWithPlayer) ?? true,
    showIcons: validateOptionalBoolean(params.show_icons || params.showIcons) ?? true,
    iconTypes: validateOptionalArray(params.icon_types || params.iconTypes) ?? ['enemy', 'ally', 'objective', 'poi'],
    fogOfWar: validateOptionalBoolean(params.fog_of_war || params.fogOfWar) ?? false
  });
};

/**
 * Configure compass
 */
const handleConfigureCompass: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const hudPath = validateRequiredString(params.hud_path || params.hudPath, 'hud_path');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'configure_compass', {
    hudPath,
    style: validateOptionalString(params.style) ?? 'bar',
    showMarkers: validateOptionalBoolean(params.show_markers || params.showMarkers) ?? true,
    showDistance: validateOptionalBoolean(params.show_distance || params.showDistance) ?? true,
    maxMarkerDistance: validateOptionalNumber(params.max_marker_distance || params.maxMarkerDistance) ?? 10000.0,
    markerTypes: validateOptionalArray(params.marker_types || params.markerTypes) ?? ['quest', 'poi', 'enemy']
  });
};

// ============================================================================
// Menu Action Handlers
// ============================================================================

/**
 * Create menu
 */
const handleCreateMenu: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const menuName = validateRequiredString(params.menu_name || params.menuName, 'menu_name');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'create_menu', {
    menuName,
    menuType: validateOptionalString(params.menu_type || params.menuType) ?? 'pause',
    pauseGame: validateOptionalBoolean(params.pause_game || params.pauseGame) ?? true,
    showCursor: validateOptionalBoolean(params.show_cursor || params.showCursor) ?? true,
    blurBackground: validateOptionalBoolean(params.blur_background || params.blurBackground) ?? true,
    closeOnEscape: validateOptionalBoolean(params.close_on_escape || params.closeOnEscape) ?? true,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add menu section
 */
const handleAddMenuSection: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const menuPath = validateRequiredString(params.menu_path || params.menuPath, 'menu_path');
  const sectionName = validateRequiredString(params.section_name || params.sectionName, 'section_name');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'add_menu_section', {
    menuPath,
    sectionName,
    sectionType: validateOptionalString(params.section_type || params.sectionType) ?? 'vertical',
    title: validateOptionalString(params.title),
    icon: validateOptionalString(params.icon),
    collapsible: validateOptionalBoolean(params.collapsible) ?? false
  });
};

/**
 * Add menu option
 */
const handleAddMenuOption: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const menuPath = validateRequiredString(params.menu_path || params.menuPath, 'menu_path');
  const optionName = validateRequiredString(params.option_name || params.optionName, 'option_name');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'add_menu_option', {
    menuPath,
    optionName,
    optionType: validateOptionalString(params.option_type || params.optionType) ?? 'button',
    label: validateOptionalString(params.label),
    tooltip: validateOptionalString(params.tooltip),
    binding: validateOptionalString(params.binding),
    defaultValue: params.default_value ?? params.defaultValue,
    options: validateOptionalArray(params.options),
    range: validateOptionalObject(params.range)
  });
};

// ============================================================================
// Notification System Action Handlers
// ============================================================================

/**
 * Setup notification system
 */
const handleSetupNotificationSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'setup_notification_system', {
    maxNotifications: validateOptionalNumber(params.max_notifications || params.maxNotifications) ?? 5,
    defaultDuration: validateOptionalNumber(params.default_duration || params.defaultDuration) ?? 3.0,
    stackBehavior: validateOptionalString(params.stack_behavior || params.stackBehavior) ?? 'queue',
    position: validateOptionalString(params.position) ?? 'top_right',
    animationIn: validateOptionalString(params.animation_in || params.animationIn) ?? 'slide_in',
    animationOut: validateOptionalString(params.animation_out || params.animationOut) ?? 'fade_out'
  });
};

/**
 * Create notification template
 */
const handleCreateNotificationTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'create_notification_template', {
    templateName,
    notificationType: validateOptionalString(params.notification_type || params.notificationType) ?? 'toast',
    showIcon: validateOptionalBoolean(params.show_icon || params.showIcon) ?? true,
    showProgress: validateOptionalBoolean(params.show_progress || params.showProgress) ?? false,
    interactive: validateOptionalBoolean(params.interactive) ?? false,
    sound: validateOptionalString(params.sound),
    duration: validateOptionalNumber(params.duration) ?? 3.0,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Configure input hints
 */
const handleConfigureInputHints: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'configure_input_hints', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    autoSwitch: validateOptionalBoolean(params.auto_switch || params.autoSwitch) ?? true,
    position: validateOptionalString(params.position) ?? 'bottom',
    showOnInteraction: validateOptionalBoolean(params.show_on_interaction || params.showOnInteraction) ?? true,
    fadeDelay: validateOptionalNumber(params.fade_delay || params.fadeDelay) ?? 3.0,
    keyboardStyle: validateOptionalString(params.keyboard_style || params.keyboardStyle) ?? 'xbox',
    playstationStyle: validateOptionalBoolean(params.playstation_style || params.playstationStyle) ?? false
  });
};

// ============================================================================
// Tutorial System Action Handlers
// ============================================================================

/**
 * Create tutorial system
 */
const handleCreateTutorialSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'create_tutorial_system', {
    showOnFirstPlay: validateOptionalBoolean(params.show_on_first_play || params.showOnFirstPlay) ?? true,
    skippable: validateOptionalBoolean(params.skippable) ?? true,
    replayable: validateOptionalBoolean(params.replayable) ?? true,
    pauseGameDuring: validateOptionalBoolean(params.pause_game_during || params.pauseGameDuring) ?? true,
    progressSaved: validateOptionalBoolean(params.progress_saved || params.progressSaved) ?? true
  });
};

/**
 * Add tutorial step
 */
const handleAddTutorialStep: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tutorialId = validateRequiredString(params.tutorial_id || params.tutorialId, 'tutorial_id');
  const stepId = validateRequiredString(params.step_id || params.stepId, 'step_id');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'add_tutorial_step', {
    tutorialId,
    stepId,
    stepType: validateOptionalString(params.step_type || params.stepType) ?? 'tooltip',
    title: validateOptionalString(params.title),
    description: validateOptionalString(params.description),
    targetWidget: validateOptionalString(params.target_widget || params.targetWidget),
    targetActor: validateOptionalString(params.target_actor || params.targetActor),
    highlightTarget: validateOptionalBoolean(params.highlight_target || params.highlightTarget) ?? true,
    requireAction: validateOptionalBoolean(params.require_action || params.requireAction) ?? false,
    requiredAction: validateOptionalString(params.required_action || params.requiredAction),
    voiceOver: validateOptionalString(params.voice_over || params.voiceOver)
  });
};

/**
 * Configure tutorial highlight
 */
const handleConfigureTutorialHighlight: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'configure_tutorial_highlight', {
    highlightColor: validateOptionalObject(params.highlight_color || params.highlightColor),
    pulseAnimation: validateOptionalBoolean(params.pulse_animation || params.pulseAnimation) ?? true,
    dimBackground: validateOptionalBoolean(params.dim_background || params.dimBackground) ?? true,
    dimOpacity: validateOptionalNumber(params.dim_opacity || params.dimOpacity) ?? 0.7,
    arrowEnabled: validateOptionalBoolean(params.arrow_enabled || params.arrowEnabled) ?? true
  });
};

// ============================================================================
// Dialog System Action Handlers
// ============================================================================

/**
 * Create dialog box
 */
const handleCreateDialogBox: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const dialogName = validateRequiredString(params.dialog_name || params.dialogName, 'dialog_name');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'create_dialog_box', {
    dialogName,
    dialogType: validateOptionalString(params.dialog_type || params.dialogType) ?? 'confirm',
    title: validateOptionalString(params.title),
    message: validateOptionalString(params.message),
    buttons: validateOptionalArray(params.buttons) ?? ['OK', 'Cancel'],
    defaultButton: validateOptionalNumber(params.default_button || params.defaultButton) ?? 0,
    modal: validateOptionalBoolean(params.modal) ?? true,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

// ============================================================================
// Loading Screen Action Handlers
// ============================================================================

/**
 * Create loading screen
 */
const handleCreateLoadingScreen: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const screenName = validateRequiredString(params.screen_name || params.screenName, 'screen_name');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'create_loading_screen', {
    screenName,
    showProgressBar: validateOptionalBoolean(params.show_progress_bar || params.showProgressBar) ?? true,
    showTips: validateOptionalBoolean(params.show_tips || params.showTips) ?? true,
    tipRotationTime: validateOptionalNumber(params.tip_rotation_time || params.tipRotationTime) ?? 5.0,
    backgroundType: validateOptionalString(params.background_type || params.backgroundType) ?? 'static',
    backgroundImages: validateOptionalArray(params.background_images || params.backgroundImages),
    minimumDuration: validateOptionalNumber(params.minimum_duration || params.minimumDuration) ?? 0.5,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add loading tip
 */
const handleAddLoadingTip: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const screenPath = validateRequiredString(params.screen_path || params.screenPath, 'screen_path');
  const tipText = validateRequiredString(params.tip_text || params.tipText, 'tip_text');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'add_loading_tip', {
    screenPath,
    tipText,
    category: validateOptionalString(params.category),
    icon: validateOptionalString(params.icon),
    minLevel: validateOptionalNumber(params.min_level || params.minLevel),
    maxLevel: validateOptionalNumber(params.max_level || params.maxLevel)
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get UI system info
 */
const handleGetUISystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const widgetPath = validateRequiredString(params.widget_path || params.widgetPath, 'widget_path');

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'get_ui_system_info', {
    widgetPath,
    includeChildren: validateOptionalBoolean(params.include_children || params.includeChildren) ?? true,
    includeBindings: validateOptionalBoolean(params.include_bindings || params.includeBindings) ?? true
  });
};

/**
 * List UI widgets
 */
const handleListUIWidgets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.UI_SYSTEM, 'list_ui_widgets', {
    widgetType: validateOptionalString(params.widget_type || params.widgetType),
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const uiSystemHandlers: ActionHandlerMap = {
  // HUD
  create_hud: handleCreateHUD,
  add_hud_element: handleAddHUDElement,
  configure_minimap: handleConfigureMinimap,
  configure_compass: handleConfigureCompass,

  // Menus
  create_menu: handleCreateMenu,
  add_menu_section: handleAddMenuSection,
  add_menu_option: handleAddMenuOption,

  // Notifications
  setup_notification_system: handleSetupNotificationSystem,
  create_notification_template: handleCreateNotificationTemplate,
  configure_input_hints: handleConfigureInputHints,

  // Tutorials
  create_tutorial_system: handleCreateTutorialSystem,
  add_tutorial_step: handleAddTutorialStep,
  configure_tutorial_highlight: handleConfigureTutorialHighlight,

  // Dialogs
  create_dialog_box: handleCreateDialogBox,

  // Loading
  create_loading_screen: handleCreateLoadingScreen,
  add_loading_tip: handleAddLoadingTip,

  // Query
  get_ui_system_info: handleGetUISystemInfo,
  list_ui_widgets: handleListUIWidgets
};

export const handleUISystemTools = createCategoryHandler(Forge2Categories.UI_SYSTEM, uiSystemHandlers);

export default uiSystemHandlers;
