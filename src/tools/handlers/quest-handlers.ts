/**
 * FORGE2: Quest System Handlers
 * Complete quest management including objectives, rewards, prerequisites, chains, and markers
 *
 * Supports quest types: main, side, daily, weekly, hidden, faction
 * Objective types: kill, collect, interact, talk_to, escort, defend, reach_location, survive, craft, discover, photograph, custom
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

export type QuestCategory = 'main' | 'side' | 'daily' | 'weekly' | 'hidden' | 'faction';

export type ObjectiveType =
  | 'kill'
  | 'collect'
  | 'interact'
  | 'talk_to'
  | 'escort'
  | 'defend'
  | 'reach_location'
  | 'survive'
  | 'craft'
  | 'discover'
  | 'photograph'
  | 'custom';

export interface QuestDefinition {
  questId: string;
  questName: string;
  description?: string;
  category?: QuestCategory;
  levelRequirement?: number;
  repeatable?: boolean;
}

export interface QuestObjective {
  objectiveId: string;
  objectiveType: ObjectiveType;
  target?: string;
  targetCount?: number;
  targetLocation?: { x: number; y: number; z: number };
  timeLimit?: number;
  optional?: boolean;
  hiddenUntilProgress?: number;
}

export interface QuestRewards {
  xpReward?: number;
  currencyRewards?: Array<{ type: string; amount: number }>;
  itemRewards?: Array<{ itemId: string; count: number; choice?: boolean }>;
  reputationRewards?: Array<{ faction: string; amount: number }>;
  abilityUnlock?: string;
  titleUnlock?: string;
}

export interface QuestPrerequisite {
  requiredQuests?: string[];
  requiredLevel?: number;
  requiredReputation?: { faction: string; rank: string };
  requiredItems?: string[];
  requiredClass?: string;
}

// ============================================================================
// Constants
// ============================================================================

const QUEST_CATEGORIES = ['main', 'side', 'daily', 'weekly', 'hidden', 'faction'] as const;

const OBJECTIVE_TYPES = [
  'kill', 'collect', 'interact', 'talk_to', 'escort', 'defend',
  'reach_location', 'survive', 'craft', 'discover', 'photograph', 'custom'
] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a new quest definition
 */
const handleCreateQuest: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');
  const questName = validateRequiredString(params.quest_name || params.questName, 'quest_name');

  const category = validateOptionalEnum(
    params.category,
    QUEST_CATEGORIES,
    'category'
  );

  return executeForge2Action(tools, Forge2Categories.QUEST, 'create_quest', {
    questId,
    questName,
    description: params.description as string | undefined,
    category: category || 'side',
    levelRequirement: params.level_requirement || params.levelRequirement,
    repeatable: params.repeatable ?? false
  });
};

/**
 * Add an objective to a quest
 */
const handleAddObjective: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');
  const objectiveId = validateRequiredString(params.objective_id || params.objectiveId, 'objective_id');

  const objectiveType = validateRequiredEnum(
    params.objective_type || params.objectiveType,
    OBJECTIVE_TYPES,
    'objective_type'
  );

  return executeForge2Action(tools, Forge2Categories.QUEST, 'add_objective', {
    questId,
    objectiveId,
    objectiveType,
    target: params.target as string | undefined,
    targetCount: params.target_count || params.targetCount,
    targetLocation: params.target_location || params.targetLocation,
    timeLimit: params.time_limit || params.timeLimit,
    optional: params.optional ?? false,
    hiddenUntilProgress: params.hidden_until_progress || params.hiddenUntilProgress
  });
};

/**
 * Set rewards for a quest
 */
const handleSetRewards: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'set_rewards', {
    questId,
    xpReward: params.xp_reward || params.xpReward,
    currencyRewards: params.currency_rewards || params.currencyRewards,
    itemRewards: params.item_rewards || params.itemRewards,
    reputationRewards: params.reputation_rewards || params.reputationRewards,
    abilityUnlock: params.ability_unlock || params.abilityUnlock,
    titleUnlock: params.title_unlock || params.titleUnlock
  });
};

/**
 * Add a prerequisite to a quest
 */
const handleAddPrerequisite: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'add_prerequisite', {
    questId,
    requiredQuests: params.required_quests || params.requiredQuests,
    requiredLevel: params.required_level || params.requiredLevel,
    requiredReputation: params.required_reputation || params.requiredReputation,
    requiredItems: params.required_items || params.requiredItems,
    requiredClass: params.required_class || params.requiredClass
  });
};

/**
 * Create a quest chain linking multiple quests
 */
const handleCreateQuestChain: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const chainId = validateRequiredString(params.chain_id || params.chainId, 'chain_id');
  const chainName = validateRequiredString(params.chain_name || params.chainName, 'chain_name');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'create_quest_chain', {
    chainId,
    chainName,
    description: params.description as string | undefined,
    quests: params.quests as string[] | undefined,
    category: validateOptionalEnum(params.category, QUEST_CATEGORIES, 'category')
  });
};

/**
 * Add a branch point to a quest chain
 */
const handleAddBranch: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const chainId = validateRequiredString(params.chain_id || params.chainId, 'chain_id');
  const branchId = validateRequiredString(params.branch_id || params.branchId, 'branch_id');
  const sourceQuestId = validateRequiredString(params.source_quest_id || params.sourceQuestId, 'source_quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'add_branch', {
    chainId,
    branchId,
    sourceQuestId,
    branchConditions: params.branch_conditions || params.branchConditions,
    targetQuests: params.target_quests || params.targetQuests
  });
};

/**
 * Set dialogue trigger for quest
 */
const handleSetDialogueTrigger: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');
  const dialogueTreeId = validateRequiredString(params.dialogue_tree_id || params.dialogueTreeId, 'dialogue_tree_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'set_dialogue_trigger', {
    questId,
    dialogueTreeId,
    triggerOnStart: params.trigger_on_start || params.triggerOnStart,
    triggerOnComplete: params.trigger_on_complete || params.triggerOnComplete,
    npcId: params.npc_id || params.npcId
  });
};

/**
 * Configure quest markers and compass integration
 */
const handleConfigureMarkers: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'configure_markers', {
    questId,
    objectiveId: params.objective_id || params.objectiveId,
    showCompassMarker: params.show_compass_marker ?? params.showCompassMarker ?? true,
    showMapMarker: params.show_map_marker ?? params.showMapMarker ?? true,
    markerIcon: params.marker_icon || params.markerIcon,
    markerColor: params.marker_color || params.markerColor,
    trackingDistance: params.tracking_distance || params.trackingDistance
  });
};

/**
 * Set fail conditions for a quest
 */
const handleSetFailConditions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'set_fail_conditions', {
    questId,
    timeExpired: params.time_expired ?? params.timeExpired,
    npcDeath: params.npc_death ?? params.npcDeath,
    playerDeath: params.player_death ?? params.playerDeath,
    customConditions: params.custom_conditions || params.customConditions,
    allowRestart: params.allow_restart ?? params.allowRestart ?? true
  });
};

/**
 * Create an optional objective
 */
const handleCreateOptionalObjective: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');
  const objectiveId = validateRequiredString(params.objective_id || params.objectiveId, 'objective_id');

  const objectiveType = validateRequiredEnum(
    params.objective_type || params.objectiveType,
    OBJECTIVE_TYPES,
    'objective_type'
  );

  return executeForge2Action(tools, Forge2Categories.QUEST, 'create_optional_objective', {
    questId,
    objectiveId,
    objectiveType,
    target: params.target as string | undefined,
    targetCount: params.target_count || params.targetCount,
    bonusRewards: params.bonus_rewards || params.bonusRewards,
    description: params.description as string | undefined
  });
};

/**
 * Add a timed objective with countdown
 */
const handleAddTimedObjective: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');
  const objectiveId = validateRequiredString(params.objective_id || params.objectiveId, 'objective_id');

  const objectiveType = validateRequiredEnum(
    params.objective_type || params.objectiveType,
    OBJECTIVE_TYPES,
    'objective_type'
  );

  const timeLimit = params.time_limit || params.timeLimit;
  if (typeof timeLimit !== 'number' || timeLimit <= 0) {
    throw new Error('time_limit is required and must be a positive number');
  }

  return executeForge2Action(tools, Forge2Categories.QUEST, 'add_timed_objective', {
    questId,
    objectiveId,
    objectiveType,
    target: params.target as string | undefined,
    targetCount: params.target_count || params.targetCount,
    timeLimit,
    showCountdown: params.show_countdown ?? params.showCountdown ?? true,
    failOnTimeout: params.fail_on_timeout ?? params.failOnTimeout ?? true
  });
};

/**
 * Configure auto-tracking for a quest
 */
const handleSetAutoTrack: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'set_auto_track', {
    questId,
    autoTrack: params.auto_track ?? params.autoTrack ?? true,
    trackPriority: params.track_priority || params.trackPriority,
    trackOnAccept: params.track_on_accept ?? params.trackOnAccept ?? true
  });
};

/**
 * Get quest state/progress
 */
const handleGetQuestState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'get_quest_state', {
    questId,
    includeObjectives: params.include_objectives ?? params.includeObjectives ?? true
  });
};

/**
 * Update quest progress
 */
const handleUpdateQuestProgress: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');
  const objectiveId = validateRequiredString(params.objective_id || params.objectiveId, 'objective_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'update_quest_progress', {
    questId,
    objectiveId,
    progressAmount: params.progress_amount || params.progressAmount || 1,
    setAbsolute: params.set_absolute ?? params.setAbsolute ?? false
  });
};

/**
 * Complete a quest
 */
const handleCompleteQuest: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'complete_quest', {
    questId,
    grantRewards: params.grant_rewards ?? params.grantRewards ?? true,
    triggerEvents: params.trigger_events ?? params.triggerEvents ?? true
  });
};

/**
 * Fail a quest
 */
const handleFailQuest: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'fail_quest', {
    questId,
    reason: params.reason as string | undefined,
    allowRestart: params.allow_restart ?? params.allowRestart ?? true
  });
};

/**
 * Abandon a quest
 */
const handleAbandonQuest: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.QUEST, 'abandon_quest', {
    questId,
    resetProgress: params.reset_progress ?? params.resetProgress ?? true
  });
};

/**
 * List all quests
 */
const handleListQuests: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.QUEST, 'list_quests', {
    category: validateOptionalEnum(params.category, QUEST_CATEGORIES, 'category'),
    status: params.status as string | undefined,
    includeHidden: params.include_hidden ?? params.includeHidden ?? false
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const questHandlers: ActionHandlerMap = {
  create_quest: handleCreateQuest,
  add_objective: handleAddObjective,
  set_rewards: handleSetRewards,
  add_prerequisite: handleAddPrerequisite,
  create_quest_chain: handleCreateQuestChain,
  add_branch: handleAddBranch,
  set_dialogue_trigger: handleSetDialogueTrigger,
  configure_markers: handleConfigureMarkers,
  set_fail_conditions: handleSetFailConditions,
  create_optional_objective: handleCreateOptionalObjective,
  add_timed_objective: handleAddTimedObjective,
  set_auto_track: handleSetAutoTrack,
  get_quest_state: handleGetQuestState,
  update_quest_progress: handleUpdateQuestProgress,
  complete_quest: handleCompleteQuest,
  fail_quest: handleFailQuest,
  abandon_quest: handleAbandonQuest,
  list_quests: handleListQuests
};

/**
 * Main quest handler - routes to specific action handlers
 */
export const handleQuestTools = createCategoryHandler(Forge2Categories.QUEST, questHandlers);

/**
 * Get available quest actions
 */
export function getQuestActions(): string[] {
  return Object.keys(questHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeQuestHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = questHandlers[action];
  if (!handler) {
    throw new Error(`Unknown quest action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
