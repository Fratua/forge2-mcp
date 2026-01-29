/**
 * FORGE2: Save System Handlers
 * Handles save/load, autosave, save slots, cloud saves, and save migration
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
// Save Game Structure Action Handlers
// ============================================================================

/**
 * Create save game structure
 */
const handleCreateSaveGame: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const saveGameName = validateRequiredString(params.save_game_name || params.saveGameName, 'save_game_name');

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'create_save_game', {
    saveGameName,
    version: validateOptionalNumber(params.version) ?? 1,
    compression: validateOptionalBoolean(params.compression) ?? true,
    encryption: validateOptionalBoolean(params.encryption) ?? false,
    includeThumbnail: validateOptionalBoolean(params.include_thumbnail || params.includeThumbnail) ?? true,
    thumbnailSize: validateOptionalObject(params.thumbnail_size || params.thumbnailSize) ?? { width: 256, height: 144 },
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add save property
 */
const handleAddSaveProperty: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const saveGamePath = validateRequiredString(params.save_game_path || params.saveGamePath, 'save_game_path');
  const propertyName = validateRequiredString(params.property_name || params.propertyName, 'property_name');

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'add_save_property', {
    saveGamePath,
    propertyName,
    propertyType: validateRequiredString(params.property_type || params.propertyType, 'property_type'),
    defaultValue: params.default_value ?? params.defaultValue,
    category: validateOptionalString(params.category),
    saveToCloud: validateOptionalBoolean(params.save_to_cloud || params.saveToCloud) ?? true
  });
};

// ============================================================================
// Autosave Action Handlers
// ============================================================================

/**
 * Configure autosave
 */
const handleConfigureAutosave: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'configure_autosave', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    interval: validateOptionalNumber(params.interval) ?? 300,
    maxAutosaves: validateOptionalNumber(params.max_autosaves || params.maxAutosaves) ?? 3,
    saveOnQuit: validateOptionalBoolean(params.save_on_quit || params.saveOnQuit) ?? true,
    saveOnLevelTransition: validateOptionalBoolean(params.save_on_level_transition || params.saveOnLevelTransition) ?? true,
    triggers: validateOptionalArray(params.triggers) ?? ['checkpoint', 'quest_complete', 'boss_defeat'],
    showIndicator: validateOptionalBoolean(params.show_indicator || params.showIndicator) ?? true
  });
};

/**
 * Add autosave trigger
 */
const handleAddAutosaveTrigger: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const triggerName = validateRequiredString(params.trigger_name || params.triggerName, 'trigger_name');

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'add_autosave_trigger', {
    triggerName,
    triggerType: validateOptionalString(params.trigger_type || params.triggerType) ?? 'event',
    eventName: validateOptionalString(params.event_name || params.eventName),
    cooldown: validateOptionalNumber(params.cooldown) ?? 60,
    condition: validateOptionalString(params.condition)
  });
};

// ============================================================================
// Save Slots Action Handlers
// ============================================================================

/**
 * Setup save slots
 */
const handleSetupSaveSlots: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'setup_save_slots', {
    maxSlots: validateOptionalNumber(params.max_slots || params.maxSlots) ?? 10,
    quickSaveSlot: validateOptionalBoolean(params.quick_save_slot || params.quickSaveSlot) ?? true,
    showThumbnails: validateOptionalBoolean(params.show_thumbnails || params.showThumbnails) ?? true,
    showPlaytime: validateOptionalBoolean(params.show_playtime || params.showPlaytime) ?? true,
    showDate: validateOptionalBoolean(params.show_date || params.showDate) ?? true,
    showLocation: validateOptionalBoolean(params.show_location || params.showLocation) ?? true,
    confirmOverwrite: validateOptionalBoolean(params.confirm_overwrite || params.confirmOverwrite) ?? true
  });
};

/**
 * Configure slot display
 */
const handleConfigureSlotDisplay: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'configure_slot_display', {
    sortOrder: validateOptionalString(params.sort_order || params.sortOrder) ?? 'date_descending',
    displayFields: validateOptionalArray(params.display_fields || params.displayFields) ?? ['character_name', 'level', 'playtime', 'date'],
    emptySlotText: validateOptionalString(params.empty_slot_text || params.emptySlotText),
    dateFormat: validateOptionalString(params.date_format || params.dateFormat) ?? 'YYYY-MM-DD HH:mm'
  });
};

// ============================================================================
// Cloud Save Action Handlers
// ============================================================================

/**
 * Create cloud save
 */
const handleCreateCloudSave: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'create_cloud_save', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    platform: validateOptionalString(params.platform) ?? 'auto',
    syncOnStartup: validateOptionalBoolean(params.sync_on_startup || params.syncOnStartup) ?? true,
    syncOnSave: validateOptionalBoolean(params.sync_on_save || params.syncOnSave) ?? true,
    conflictResolution: validateOptionalString(params.conflict_resolution || params.conflictResolution) ?? 'ask',
    maxCloudSaves: validateOptionalNumber(params.max_cloud_saves || params.maxCloudSaves) ?? 5
  });
};

/**
 * Configure cloud sync
 */
const handleConfigureCloudSync: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'configure_cloud_sync', {
    syncInterval: validateOptionalNumber(params.sync_interval || params.syncInterval) ?? 60,
    syncOnBackground: validateOptionalBoolean(params.sync_on_background || params.syncOnBackground) ?? true,
    offlineMode: validateOptionalBoolean(params.offline_mode || params.offlineMode) ?? true,
    showSyncIndicator: validateOptionalBoolean(params.show_sync_indicator || params.showSyncIndicator) ?? true,
    retryOnFailure: validateOptionalBoolean(params.retry_on_failure || params.retryOnFailure) ?? true,
    maxRetries: validateOptionalNumber(params.max_retries || params.maxRetries) ?? 3
  });
};

// ============================================================================
// Save Migration Action Handlers
// ============================================================================

/**
 * Configure save migration
 */
const handleConfigureSaveMigration: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'configure_save_migration', {
    autoMigrate: validateOptionalBoolean(params.auto_migrate || params.autoMigrate) ?? true,
    backupBeforeMigration: validateOptionalBoolean(params.backup_before_migration || params.backupBeforeMigration) ?? true,
    migrationVersions: validateOptionalArray(params.migration_versions || params.migrationVersions) ?? [],
    showMigrationPrompt: validateOptionalBoolean(params.show_migration_prompt || params.showMigrationPrompt) ?? false
  });
};

/**
 * Add migration rule
 */
const handleAddMigrationRule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const fromVersion = validateRequiredString(params.from_version || params.fromVersion, 'from_version');
  const toVersion = validateRequiredString(params.to_version || params.toVersion, 'to_version');

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'add_migration_rule', {
    fromVersion,
    toVersion,
    migrationFunction: validateOptionalString(params.migration_function || params.migrationFunction),
    renameFields: validateOptionalObject(params.rename_fields || params.renameFields),
    removeFields: validateOptionalArray(params.remove_fields || params.removeFields),
    addFields: validateOptionalObject(params.add_fields || params.addFields)
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get save system info
 */
const handleGetSaveSystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'get_save_system_info', {
    saveGamePath: validateOptionalString(params.save_game_path || params.saveGamePath),
    includeSlots: validateOptionalBoolean(params.include_slots || params.includeSlots) ?? true,
    includeCloudStatus: validateOptionalBoolean(params.include_cloud_status || params.includeCloudStatus) ?? true
  });
};

/**
 * List save files
 */
const handleListSaveFiles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SAVE_SYSTEM, 'list_save_files', {
    includeAutosaves: validateOptionalBoolean(params.include_autosaves || params.includeAutosaves) ?? true,
    includeCloudSaves: validateOptionalBoolean(params.include_cloud_saves || params.includeCloudSaves) ?? true,
    sortBy: validateOptionalString(params.sort_by || params.sortBy) ?? 'date'
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const saveHandlers: ActionHandlerMap = {
  // Save Structure
  create_save_game: handleCreateSaveGame,
  add_save_property: handleAddSaveProperty,

  // Autosave
  configure_autosave: handleConfigureAutosave,
  add_autosave_trigger: handleAddAutosaveTrigger,

  // Save Slots
  setup_save_slots: handleSetupSaveSlots,
  configure_slot_display: handleConfigureSlotDisplay,

  // Cloud Save
  create_cloud_save: handleCreateCloudSave,
  configure_cloud_sync: handleConfigureCloudSync,

  // Migration
  configure_save_migration: handleConfigureSaveMigration,
  add_migration_rule: handleAddMigrationRule,

  // Query
  get_save_system_info: handleGetSaveSystemInfo,
  list_save_files: handleListSaveFiles
};

export const handleSaveTools = createCategoryHandler(Forge2Categories.SAVE_SYSTEM, saveHandlers);

export default saveHandlers;
