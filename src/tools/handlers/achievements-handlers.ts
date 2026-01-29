/**
 * FORGE2: Achievements Handlers
 * Handles achievements, trophies, platform integration, progress tracking, and rewards
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
// Achievement Definition Action Handlers
// ============================================================================

/**
 * Create achievement
 */
const handleCreateAchievement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const achievementId = validateRequiredString(params.achievement_id || params.achievementId, 'achievement_id');
  const achievementName = validateRequiredString(params.achievement_name || params.achievementName, 'achievement_name');

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'create_achievement', {
    achievementId,
    achievementName,
    description: validateOptionalString(params.description) ?? '',
    iconPath: validateOptionalString(params.icon_path || params.iconPath),
    iconLockedPath: validateOptionalString(params.icon_locked_path || params.iconLockedPath),
    points: validateOptionalNumber(params.points) ?? 10,
    category: validateOptionalString(params.category) ?? 'general',
    isSecret: validateOptionalBoolean(params.is_secret || params.isSecret) ?? false,
    isHidden: validateOptionalBoolean(params.is_hidden || params.isHidden) ?? false,
    rarity: validateOptionalString(params.rarity) ?? 'common'
  });
};

/**
 * Configure achievement criteria
 */
const handleConfigureAchievementCriteria: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const achievementId = validateRequiredString(params.achievement_id || params.achievementId, 'achievement_id');

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'configure_achievement_criteria', {
    achievementId,
    criteriaType: validateRequiredString(params.criteria_type || params.criteriaType, 'criteria_type'),
    targetValue: validateOptionalNumber(params.target_value || params.targetValue) ?? 1,
    targetId: validateOptionalString(params.target_id || params.targetId),
    conditions: validateOptionalArray(params.conditions),
    trackProgress: validateOptionalBoolean(params.track_progress || params.trackProgress) ?? true
  });
};

/**
 * Add achievement reward
 */
const handleAddAchievementReward: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const achievementId = validateRequiredString(params.achievement_id || params.achievementId, 'achievement_id');

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'add_achievement_reward', {
    achievementId,
    rewardType: validateRequiredString(params.reward_type || params.rewardType, 'reward_type'),
    rewardId: validateOptionalString(params.reward_id || params.rewardId),
    rewardAmount: validateOptionalNumber(params.reward_amount || params.rewardAmount) ?? 1,
    rewardData: validateOptionalObject(params.reward_data || params.rewardData)
  });
};

// ============================================================================
// Achievement Categories Action Handlers
// ============================================================================

/**
 * Create achievement category
 */
const handleCreateAchievementCategory: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const categoryId = validateRequiredString(params.category_id || params.categoryId, 'category_id');
  const categoryName = validateRequiredString(params.category_name || params.categoryName, 'category_name');

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'create_achievement_category', {
    categoryId,
    categoryName,
    description: validateOptionalString(params.description),
    iconPath: validateOptionalString(params.icon_path || params.iconPath),
    sortOrder: validateOptionalNumber(params.sort_order || params.sortOrder) ?? 0,
    isExpansion: validateOptionalBoolean(params.is_expansion || params.isExpansion) ?? false
  });
};

/**
 * Create achievement tier
 */
const handleCreateAchievementTier: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const baseAchievementId = validateRequiredString(params.base_achievement_id || params.baseAchievementId, 'base_achievement_id');

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'create_achievement_tier', {
    baseAchievementId,
    tiers: validateOptionalArray(params.tiers) ?? [
      { level: 1, target: 10, suffix: 'I' },
      { level: 2, target: 50, suffix: 'II' },
      { level: 3, target: 100, suffix: 'III' }
    ],
    scalingRewards: validateOptionalBoolean(params.scaling_rewards || params.scalingRewards) ?? true
  });
};

// ============================================================================
// Platform Integration Action Handlers
// ============================================================================

/**
 * Configure platform integration
 */
const handleConfigurePlatformIntegration: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'configure_platform_integration', {
    steamEnabled: validateOptionalBoolean(params.steam_enabled || params.steamEnabled) ?? true,
    psnEnabled: validateOptionalBoolean(params.psn_enabled || params.psnEnabled) ?? true,
    xboxEnabled: validateOptionalBoolean(params.xbox_enabled || params.xboxEnabled) ?? true,
    epicEnabled: validateOptionalBoolean(params.epic_enabled || params.epicEnabled) ?? false,
    gogEnabled: validateOptionalBoolean(params.gog_enabled || params.gogEnabled) ?? false,
    syncOnStartup: validateOptionalBoolean(params.sync_on_startup || params.syncOnStartup) ?? true,
    offlineSupport: validateOptionalBoolean(params.offline_support || params.offlineSupport) ?? true
  });
};

/**
 * Map platform achievement
 */
const handleMapPlatformAchievement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const achievementId = validateRequiredString(params.achievement_id || params.achievementId, 'achievement_id');

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'map_platform_achievement', {
    achievementId,
    steamAchievementId: validateOptionalString(params.steam_achievement_id || params.steamAchievementId),
    psnTrophyId: validateOptionalString(params.psn_trophy_id || params.psnTrophyId),
    xboxAchievementId: validateOptionalString(params.xbox_achievement_id || params.xboxAchievementId),
    epicAchievementId: validateOptionalString(params.epic_achievement_id || params.epicAchievementId)
  });
};

// ============================================================================
// Progress Tracking Action Handlers
// ============================================================================

/**
 * Setup progress tracking
 */
const handleSetupProgressTracking: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'setup_progress_tracking', {
    showProgressNotifications: validateOptionalBoolean(params.show_progress_notifications || params.showProgressNotifications) ?? true,
    progressMilestones: validateOptionalArray(params.progress_milestones || params.progressMilestones) ?? [25, 50, 75, 90],
    trackStatistics: validateOptionalBoolean(params.track_statistics || params.trackStatistics) ?? true,
    persistProgress: validateOptionalBoolean(params.persist_progress || params.persistProgress) ?? true
  });
};

/**
 * Configure achievement UI
 */
const handleConfigureAchievementUI: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'configure_achievement_ui', {
    popupEnabled: validateOptionalBoolean(params.popup_enabled || params.popupEnabled) ?? true,
    popupDuration: validateOptionalNumber(params.popup_duration || params.popupDuration) ?? 5.0,
    popupPosition: validateOptionalString(params.popup_position || params.popupPosition) ?? 'top_right',
    soundEnabled: validateOptionalBoolean(params.sound_enabled || params.soundEnabled) ?? true,
    soundPath: validateOptionalString(params.sound_path || params.soundPath),
    showRarity: validateOptionalBoolean(params.show_rarity || params.showRarity) ?? true,
    showProgress: validateOptionalBoolean(params.show_progress || params.showProgress) ?? true
  });
};

// ============================================================================
// Statistics Action Handlers
// ============================================================================

/**
 * Create statistic
 */
const handleCreateStatistic: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const statId = validateRequiredString(params.stat_id || params.statId, 'stat_id');
  const statName = validateRequiredString(params.stat_name || params.statName, 'stat_name');

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'create_statistic', {
    statId,
    statName,
    statType: validateOptionalString(params.stat_type || params.statType) ?? 'int',
    defaultValue: validateOptionalNumber(params.default_value || params.defaultValue) ?? 0,
    minValue: validateOptionalNumber(params.min_value || params.minValue),
    maxValue: validateOptionalNumber(params.max_value || params.maxValue),
    aggregationType: validateOptionalString(params.aggregation_type || params.aggregationType) ?? 'sum',
    displayFormat: validateOptionalString(params.display_format || params.displayFormat)
  });
};

/**
 * Link statistic to achievement
 */
const handleLinkStatisticToAchievement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const statId = validateRequiredString(params.stat_id || params.statId, 'stat_id');
  const achievementId = validateRequiredString(params.achievement_id || params.achievementId, 'achievement_id');

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'link_statistic_to_achievement', {
    statId,
    achievementId,
    targetValue: validateRequiredString(params.target_value || params.targetValue, 'target_value'),
    comparison: validateOptionalString(params.comparison) ?? 'greater_or_equal'
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get achievement info
 */
const handleGetAchievementInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'get_achievement_info', {
    achievementId: validateOptionalString(params.achievement_id || params.achievementId),
    includeProgress: validateOptionalBoolean(params.include_progress || params.includeProgress) ?? true,
    includePlatformStatus: validateOptionalBoolean(params.include_platform_status || params.includePlatformStatus) ?? false
  });
};

/**
 * List achievements
 */
const handleListAchievements: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'list_achievements', {
    category: validateOptionalString(params.category),
    includeSecret: validateOptionalBoolean(params.include_secret || params.includeSecret) ?? false,
    includeUnlocked: validateOptionalBoolean(params.include_unlocked || params.includeUnlocked) ?? true,
    includeLocked: validateOptionalBoolean(params.include_locked || params.includeLocked) ?? true,
    sortBy: validateOptionalString(params.sort_by || params.sortBy) ?? 'category'
  });
};

/**
 * Get achievement statistics
 */
const handleGetAchievementStatistics: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ACHIEVEMENTS, 'get_achievement_statistics', {
    includeGlobalStats: validateOptionalBoolean(params.include_global_stats || params.includeGlobalStats) ?? false,
    includePlayerStats: validateOptionalBoolean(params.include_player_stats || params.includePlayerStats) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const achievementsHandlers: ActionHandlerMap = {
  // Definition
  create_achievement: handleCreateAchievement,
  configure_achievement_criteria: handleConfigureAchievementCriteria,
  add_achievement_reward: handleAddAchievementReward,

  // Categories
  create_achievement_category: handleCreateAchievementCategory,
  create_achievement_tier: handleCreateAchievementTier,

  // Platform
  configure_platform_integration: handleConfigurePlatformIntegration,
  map_platform_achievement: handleMapPlatformAchievement,

  // Progress
  setup_progress_tracking: handleSetupProgressTracking,
  configure_achievement_ui: handleConfigureAchievementUI,

  // Statistics
  create_statistic: handleCreateStatistic,
  link_statistic_to_achievement: handleLinkStatisticToAchievement,

  // Query
  get_achievement_info: handleGetAchievementInfo,
  list_achievements: handleListAchievements,
  get_achievement_statistics: handleGetAchievementStatistics
};

export const handleAchievementsTools = createCategoryHandler(Forge2Categories.ACHIEVEMENTS, achievementsHandlers);

export default achievementsHandlers;
