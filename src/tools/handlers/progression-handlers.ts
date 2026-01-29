/**
 * FORGE2: Progression System Handlers
 * Complete progression management including XP, levels, achievements, and factions
 *
 * Supports XP curves: linear, exponential, polynomial, custom
 * Achievement system with various criteria types
 * Faction/reputation system with ranks and rewards
 */

import { ITools } from '../../types/tool-interfaces.js';
import type { HandlerArgs } from '../../types/handler-types.js';
import { cleanObject } from '../../utils/safe-json.js';
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

/**
 * Validate required string parameter
 */
function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value;
}

/**
 * Validate optional enum parameter
 */
function validateOptionalEnum<T extends string>(
  value: unknown,
  allowedValues: readonly T[],
  fieldName: string
): T | undefined {
  if (value === undefined || value === null || value === '') {
    return undefined;
  }
  if (typeof value !== 'string' || !allowedValues.includes(value as T)) {
    throw new Error(`${fieldName} must be one of: ${allowedValues.join(', ')}`);
  }
  return value as T;
}

/**
 * Validate required enum parameter
 */
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

// ============================================================================
// Types
// ============================================================================

export type XPCurveType = 'linear' | 'exponential' | 'polynomial' | 'custom';
export type AchievementCriteriaType = 'kill' | 'collect' | 'reach' | 'discover' | 'craft' | 'complete' | 'survive' | 'custom';
export type ReputationRank = 'hated' | 'hostile' | 'unfriendly' | 'neutral' | 'friendly' | 'honored' | 'revered' | 'exalted';

export interface XPCurve {
  curveType: XPCurveType;
  baseXP?: number;
  multiplier?: number;
  exponent?: number;
  customValues?: number[];
}

export interface AchievementDefinition {
  achievementId: string;
  achievementName: string;
  description?: string;
  criteriaType: AchievementCriteriaType;
  criteriaTarget?: string;
  criteriaValue?: number;
  points?: number;
  hidden?: boolean;
}

export interface FactionDefinition {
  factionId: string;
  factionName: string;
  description?: string;
  ranks: Array<{ name: string; threshold: number; rewards?: unknown[] }>;
}

// ============================================================================
// Constants
// ============================================================================

const XP_CURVE_TYPES = ['linear', 'exponential', 'polynomial', 'custom'] as const;
const ACHIEVEMENT_CRITERIA_TYPES = ['kill', 'collect', 'reach', 'discover', 'craft', 'complete', 'survive', 'custom'] as const;
const REPUTATION_RANKS = ['hated', 'hostile', 'unfriendly', 'neutral', 'friendly', 'honored', 'revered', 'exalted'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Configure XP curve for leveling
 */
const handleConfigureXPCurve: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const curveType = validateOptionalEnum(
    params.curve_type || params.curveType,
    XP_CURVE_TYPES,
    'curve_type'
  );

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'configure_xp_curve', {
    curveType: curveType || 'exponential',
    baseXP: params.base_xp ?? params.baseXP ?? 100,
    multiplier: params.multiplier ?? 1.5,
    exponent: params.exponent ?? 2,
    maxLevel: params.max_level ?? params.maxLevel ?? 100,
    customValues: params.custom_values || params.customValues
  });
};

/**
 * Configure level rewards
 */
const handleConfigureLevelRewards: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'configure_level_rewards', {
    skillPointsPerLevel: params.skill_points_per_level ?? params.skillPointsPerLevel ?? 1,
    attributePointsPerLevel: params.attribute_points_per_level ?? params.attributePointsPerLevel ?? 0,
    healthPerLevel: params.health_per_level ?? params.healthPerLevel,
    manaPerLevel: params.mana_per_level ?? params.manaPerLevel,
    statIncreases: params.stat_increases || params.statIncreases,
    abilityUnlocks: params.ability_unlocks || params.abilityUnlocks,
    milestoneRewards: params.milestone_rewards || params.milestoneRewards
  });
};

/**
 * Configure XP sources
 */
const handleConfigureXPSources: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'configure_xp_sources', {
    killXPEnabled: params.kill_xp_enabled ?? params.killXPEnabled ?? true,
    questXPEnabled: params.quest_xp_enabled ?? params.questXPEnabled ?? true,
    explorationXPEnabled: params.exploration_xp_enabled ?? params.explorationXPEnabled ?? true,
    craftingXPEnabled: params.crafting_xp_enabled ?? params.craftingXPEnabled ?? true,
    killXPFormula: params.kill_xp_formula || params.killXPFormula,
    questXPMultiplier: params.quest_xp_multiplier ?? params.questXPMultiplier ?? 1.0,
    restedXPMultiplier: params.rested_xp_multiplier ?? params.restedXPMultiplier ?? 1.5,
    groupXPBonus: params.group_xp_bonus ?? params.groupXPBonus ?? 0.1,
    levelDifferenceModifier: params.level_difference_modifier ?? params.levelDifferenceModifier ?? true
  });
};

/**
 * Create an achievement
 */
const handleCreateAchievement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const achievementId = validateRequiredString(params.achievement_id || params.achievementId, 'achievement_id');
  const achievementName = validateRequiredString(params.achievement_name || params.achievementName, 'achievement_name');

  const criteriaType = validateRequiredEnum(
    params.criteria_type || params.criteriaType,
    ACHIEVEMENT_CRITERIA_TYPES,
    'criteria_type'
  );

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'create_achievement', {
    achievementId,
    achievementName,
    description: params.description as string | undefined,
    criteriaType,
    criteriaTarget: params.criteria_target || params.criteriaTarget,
    criteriaValue: params.criteria_value ?? params.criteriaValue ?? 1,
    points: params.points ?? 10,
    iconPath: params.icon_path || params.iconPath,
    hidden: params.hidden ?? false,
    category: params.category as string | undefined,
    rewards: params.rewards
  });
};

/**
 * Create achievement category
 */
const handleCreateAchievementCategory: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const categoryId = validateRequiredString(params.category_id || params.categoryId, 'category_id');
  const categoryName = validateRequiredString(params.category_name || params.categoryName, 'category_name');

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'create_achievement_category', {
    categoryId,
    categoryName,
    description: params.description as string | undefined,
    iconPath: params.icon_path || params.iconPath,
    sortOrder: params.sort_order ?? params.sortOrder ?? 0
  });
};

/**
 * Create a faction
 */
const handleCreateFaction: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const factionId = validateRequiredString(params.faction_id || params.factionId, 'faction_id');
  const factionName = validateRequiredString(params.faction_name || params.factionName, 'faction_name');

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'create_faction', {
    factionId,
    factionName,
    description: params.description as string | undefined,
    iconPath: params.icon_path || params.iconPath,
    defaultReputation: params.default_reputation ?? params.defaultReputation ?? 0,
    minReputation: params.min_reputation ?? params.minReputation ?? -42000,
    maxReputation: params.max_reputation ?? params.maxReputation ?? 42000,
    ranks: params.ranks
  });
};

/**
 * Configure faction ranks
 */
const handleConfigureFactionRanks: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const factionId = validateRequiredString(params.faction_id || params.factionId, 'faction_id');

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'configure_faction_ranks', {
    factionId,
    ranks: params.ranks
  });
};

/**
 * Add faction rank rewards
 */
const handleAddFactionRankRewards: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const factionId = validateRequiredString(params.faction_id || params.factionId, 'faction_id');
  const rankName = validateRequiredString(params.rank_name || params.rankName, 'rank_name');

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'add_faction_rank_rewards', {
    factionId,
    rankName,
    shopDiscounts: params.shop_discounts || params.shopDiscounts,
    exclusiveItems: params.exclusive_items || params.exclusiveItems,
    titleUnlock: params.title_unlock || params.titleUnlock,
    abilityUnlock: params.ability_unlock || params.abilityUnlock,
    questUnlocks: params.quest_unlocks || params.questUnlocks
  });
};

/**
 * Setup faction relationships
 */
const handleSetupFactionRelationships: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const factionId = validateRequiredString(params.faction_id || params.factionId, 'faction_id');

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'setup_faction_relationships', {
    factionId,
    allies: params.allies as string[] | undefined,
    enemies: params.enemies as string[] | undefined,
    neutral: params.neutral as string[] | undefined,
    allyGainMultiplier: params.ally_gain_multiplier ?? params.allyGainMultiplier ?? 0.5,
    enemyLossMultiplier: params.enemy_loss_multiplier ?? params.enemyLossMultiplier ?? 0.5
  });
};

/**
 * Give XP to player
 */
const handleGiveXP: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const amount = params.amount;
  if (typeof amount !== 'number' || amount <= 0) {
    throw new Error('amount is required and must be a positive number');
  }

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'give_xp', {
    amount,
    playerId: params.player_id || params.playerId,
    source: params.source as string | undefined,
    applyModifiers: params.apply_modifiers ?? params.applyModifiers ?? true
  });
};

/**
 * Set player level
 */
const handleSetLevel: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const level = params.level;
  if (typeof level !== 'number' || level < 1) {
    throw new Error('level is required and must be a positive integer');
  }

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'set_level', {
    level,
    playerId: params.player_id || params.playerId,
    grantRewards: params.grant_rewards ?? params.grantRewards ?? true
  });
};

/**
 * Unlock achievement
 */
const handleUnlockAchievement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const achievementId = validateRequiredString(params.achievement_id || params.achievementId, 'achievement_id');

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'unlock_achievement', {
    achievementId,
    playerId: params.player_id || params.playerId,
    grantRewards: params.grant_rewards ?? params.grantRewards ?? true,
    showNotification: params.show_notification ?? params.showNotification ?? true
  });
};

/**
 * Update achievement progress
 */
const handleUpdateAchievementProgress: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const achievementId = validateRequiredString(params.achievement_id || params.achievementId, 'achievement_id');

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'update_achievement_progress', {
    achievementId,
    playerId: params.player_id || params.playerId,
    progressAmount: params.progress_amount ?? params.progressAmount ?? 1,
    setAbsolute: params.set_absolute ?? params.setAbsolute ?? false
  });
};

/**
 * Change faction reputation
 */
const handleChangeReputation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const factionId = validateRequiredString(params.faction_id || params.factionId, 'faction_id');
  const amount = params.amount;

  if (typeof amount !== 'number') {
    throw new Error('amount is required and must be a number');
  }

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'change_reputation', {
    factionId,
    amount,
    playerId: params.player_id || params.playerId,
    applyRelationships: params.apply_relationships ?? params.applyRelationships ?? true
  });
};

/**
 * Set faction reputation to a specific rank
 */
const handleSetReputationRank: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const factionId = validateRequiredString(params.faction_id || params.factionId, 'faction_id');
  const rank = validateRequiredEnum(
    params.rank,
    REPUTATION_RANKS,
    'rank'
  );

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'set_reputation_rank', {
    factionId,
    rank,
    playerId: params.player_id || params.playerId,
    grantRewards: params.grant_rewards ?? params.grantRewards ?? true
  });
};

/**
 * Get player progression data
 */
const handleGetPlayerProgression: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'get_player_progression', {
    playerId: params.player_id || params.playerId,
    includeAchievements: params.include_achievements ?? params.includeAchievements ?? true,
    includeFactions: params.include_factions ?? params.includeFactions ?? true,
    includeStats: params.include_stats ?? params.includeStats ?? true
  });
};

/**
 * List all achievements
 */
const handleListAchievements: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'list_achievements', {
    category: params.category as string | undefined,
    includeHidden: params.include_hidden ?? params.includeHidden ?? false,
    playerId: params.player_id || params.playerId
  });
};

/**
 * List all factions
 */
const handleListFactions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.PROGRESSION, 'list_factions', {
    playerId: params.player_id || params.playerId,
    includeReputation: params.include_reputation ?? params.includeReputation ?? true
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const progressionHandlers: ActionHandlerMap = {
  configure_xp_curve: handleConfigureXPCurve,
  configure_level_rewards: handleConfigureLevelRewards,
  configure_xp_sources: handleConfigureXPSources,
  create_achievement: handleCreateAchievement,
  create_achievement_category: handleCreateAchievementCategory,
  create_faction: handleCreateFaction,
  configure_faction_ranks: handleConfigureFactionRanks,
  add_faction_rank_rewards: handleAddFactionRankRewards,
  setup_faction_relationships: handleSetupFactionRelationships,
  give_xp: handleGiveXP,
  set_level: handleSetLevel,
  unlock_achievement: handleUnlockAchievement,
  update_achievement_progress: handleUpdateAchievementProgress,
  change_reputation: handleChangeReputation,
  set_reputation_rank: handleSetReputationRank,
  get_player_progression: handleGetPlayerProgression,
  list_achievements: handleListAchievements,
  list_factions: handleListFactions
};

/**
 * Main progression handler - routes to specific action handlers
 */
export const handleProgressionTools = createCategoryHandler(Forge2Categories.PROGRESSION, progressionHandlers);

/**
 * Get available progression actions
 */
export function getProgressionActions(): string[] {
  return Object.keys(progressionHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeProgressionHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = progressionHandlers[action];
  if (!handler) {
    throw new Error(`Unknown progression action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
