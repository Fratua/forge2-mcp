/**
 * FORGE2: Skill Tree System Handlers
 * Complete skill tree management including skills, prerequisites, classes, and respecs
 *
 * Supports skill types: passive, active, toggle, aura, ultimate
 * Class system with specializations and multiclassing
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

export type SkillType = 'passive' | 'active' | 'toggle' | 'aura' | 'ultimate';
export type SkillCategory = 'offensive' | 'defensive' | 'utility' | 'support' | 'movement';

export interface SkillDefinition {
  skillId: string;
  skillName: string;
  skillType: SkillType;
  description?: string;
  iconPath?: string;
  maxRanks?: number;
  pointCost?: number;
}

export interface SkillEffect {
  effectType: string;
  stat?: string;
  value?: number;
  valuePerRank?: number;
  duration?: number;
}

export interface SkillPrerequisite {
  skillId?: string;
  minRank?: number;
  requiredLevel?: number;
  requiredStat?: { stat: string; value: number };
}

export interface ClassDefinition {
  classId: string;
  className: string;
  description?: string;
  baseStats?: Record<string, number>;
  startingAbilities?: string[];
  equipmentRestrictions?: string[];
}

// ============================================================================
// Constants
// ============================================================================

const SKILL_TYPES = ['passive', 'active', 'toggle', 'aura', 'ultimate'] as const;
const SKILL_CATEGORIES = ['offensive', 'defensive', 'utility', 'support', 'movement'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a new skill tree
 */
const handleCreateSkillTree: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const treeName = validateRequiredString(params.tree_name || params.treeName, 'tree_name');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'create_skill_tree', {
    treeId,
    treeName,
    description: params.description as string | undefined,
    iconPath: params.icon_path || params.iconPath,
    classRestriction: params.class_restriction || params.classRestriction,
    maxPoints: params.max_points || params.maxPoints,
    layoutType: params.layout_type || params.layoutType || 'vertical'
  });
};

/**
 * Add a skill node to a tree
 */
const handleAddSkillNode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const skillId = validateRequiredString(params.skill_id || params.skillId, 'skill_id');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'add_skill_node', {
    treeId,
    skillId,
    positionX: params.position_x ?? params.positionX ?? 0,
    positionY: params.position_y ?? params.positionY ?? 0,
    tier: params.tier ?? 0,
    branchId: params.branch_id || params.branchId
  });
};

/**
 * Connect skill nodes (set prerequisites)
 */
const handleConnectSkillNodes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const fromSkillId = validateRequiredString(params.from_skill_id || params.fromSkillId, 'from_skill_id');
  const toSkillId = validateRequiredString(params.to_skill_id || params.toSkillId, 'to_skill_id');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'connect_skill_nodes', {
    treeId,
    fromSkillId,
    toSkillId,
    requiredRank: params.required_rank ?? params.requiredRank ?? 1,
    connectionType: params.connection_type || params.connectionType || 'required'
  });
};

/**
 * Create a skill definition
 */
const handleCreateSkill: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const skillId = validateRequiredString(params.skill_id || params.skillId, 'skill_id');
  const skillName = validateRequiredString(params.skill_name || params.skillName, 'skill_name');

  const skillType = validateRequiredEnum(
    params.skill_type || params.skillType,
    SKILL_TYPES,
    'skill_type'
  );

  const category = validateOptionalEnum(
    params.category,
    SKILL_CATEGORIES,
    'category'
  );

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'create_skill', {
    skillId,
    skillName,
    skillType,
    category,
    description: params.description as string | undefined,
    iconPath: params.icon_path || params.iconPath,
    maxRanks: params.max_ranks ?? params.maxRanks ?? 1,
    pointCost: params.point_cost ?? params.pointCost ?? 1,
    cooldown: params.cooldown,
    manaCost: params.mana_cost || params.manaCost,
    castTime: params.cast_time || params.castTime,
    range: params.range,
    areaOfEffect: params.area_of_effect || params.areaOfEffect
  });
};

/**
 * Add effect to a skill
 */
const handleAddSkillEffect: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const skillId = validateRequiredString(params.skill_id || params.skillId, 'skill_id');
  const effectType = validateRequiredString(params.effect_type || params.effectType, 'effect_type');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'add_skill_effect', {
    skillId,
    effectType,
    stat: params.stat as string | undefined,
    value: params.value,
    valuePerRank: params.value_per_rank || params.valuePerRank,
    duration: params.duration,
    tickRate: params.tick_rate || params.tickRate,
    stackable: params.stackable ?? false,
    maxStacks: params.max_stacks || params.maxStacks
  });
};

/**
 * Add prerequisite to a skill
 */
const handleAddSkillPrerequisite: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const skillId = validateRequiredString(params.skill_id || params.skillId, 'skill_id');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'add_skill_prerequisite', {
    skillId,
    requiredSkillId: params.required_skill_id || params.requiredSkillId,
    requiredSkillRank: params.required_skill_rank ?? params.requiredSkillRank ?? 1,
    requiredLevel: params.required_level || params.requiredLevel,
    requiredStat: params.required_stat || params.requiredStat,
    requiredStatValue: params.required_stat_value || params.requiredStatValue,
    eitherOr: params.either_or ?? params.eitherOr ?? false
  });
};

/**
 * Create a character class
 */
const handleCreateClass: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const classId = validateRequiredString(params.class_id || params.classId, 'class_id');
  const className = validateRequiredString(params.class_name || params.className, 'class_name');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'create_class', {
    classId,
    className,
    description: params.description as string | undefined,
    iconPath: params.icon_path || params.iconPath,
    baseStats: params.base_stats || params.baseStats,
    statGrowth: params.stat_growth || params.statGrowth,
    startingAbilities: params.starting_abilities || params.startingAbilities,
    equipmentRestrictions: params.equipment_restrictions || params.equipmentRestrictions,
    skillTrees: params.skill_trees || params.skillTrees,
    role: params.role as string | undefined
  });
};

/**
 * Configure subclass/specialization
 */
const handleConfigureSubclass: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const classId = validateRequiredString(params.class_id || params.classId, 'class_id');
  const subclassId = validateRequiredString(params.subclass_id || params.subclassId, 'subclass_id');
  const subclassName = validateRequiredString(params.subclass_name || params.subclassName, 'subclass_name');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'configure_subclass', {
    classId,
    subclassId,
    subclassName,
    description: params.description as string | undefined,
    unlockLevel: params.unlock_level || params.unlockLevel,
    bonusStats: params.bonus_stats || params.bonusStats,
    bonusAbilities: params.bonus_abilities || params.bonusAbilities,
    exclusiveSkillTree: params.exclusive_skill_tree || params.exclusiveSkillTree,
    requirements: params.requirements
  });
};

/**
 * Setup multiclassing rules
 */
const handleSetupMulticlass: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'setup_multiclass', {
    enabled: params.enabled ?? true,
    maxClasses: params.max_classes ?? params.maxClasses ?? 2,
    primaryClassRequired: params.primary_class_required ?? params.primaryClassRequired ?? true,
    levelRequirement: params.level_requirement || params.levelRequirement,
    restrictedCombinations: params.restricted_combinations || params.restrictedCombinations,
    statPenalty: params.stat_penalty ?? params.statPenalty
  });
};

/**
 * Configure respec system
 */
const handleConfigureRespec: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'configure_respec', {
    enabled: params.enabled ?? true,
    costType: params.cost_type || params.costType || 'currency',
    baseCost: params.base_cost ?? params.baseCost ?? 100,
    costScaling: params.cost_scaling || params.costScaling,
    partialRespec: params.partial_respec ?? params.partialRespec ?? false,
    freeRespecs: params.free_respecs ?? params.freeRespecs ?? 0,
    cooldown: params.cooldown,
    npcRequired: params.npc_required ?? params.npcRequired ?? false,
    npcId: params.npc_id || params.npcId
  });
};

/**
 * Unlock a skill for a player
 */
const handleUnlockSkill: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const skillId = validateRequiredString(params.skill_id || params.skillId, 'skill_id');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'unlock_skill', {
    skillId,
    playerId: params.player_id || params.playerId,
    rank: params.rank ?? 1,
    ignorePrerequisites: params.ignore_prerequisites ?? params.ignorePrerequisites ?? false,
    ignoreCost: params.ignore_cost ?? params.ignoreCost ?? false
  });
};

/**
 * Upgrade a skill rank
 */
const handleUpgradeSkill: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const skillId = validateRequiredString(params.skill_id || params.skillId, 'skill_id');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'upgrade_skill', {
    skillId,
    playerId: params.player_id || params.playerId,
    ranks: params.ranks ?? 1,
    ignoreCost: params.ignore_cost ?? params.ignoreCost ?? false
  });
};

/**
 * Respec skills (reset points)
 */
const handleRespecSkills: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'respec_skills', {
    playerId: params.player_id || params.playerId,
    treeId: params.tree_id || params.treeId,
    fullRespec: params.full_respec ?? params.fullRespec ?? true,
    refundPercentage: params.refund_percentage ?? params.refundPercentage ?? 100
  });
};

/**
 * Get skill tree data
 */
const handleGetSkillTree: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'get_skill_tree', {
    treeId,
    includeSkills: params.include_skills ?? params.includeSkills ?? true,
    includeConnections: params.include_connections ?? params.includeConnections ?? true
  });
};

/**
 * Get player's skill progress
 */
const handleGetPlayerSkills: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'get_player_skills', {
    playerId: params.player_id || params.playerId,
    treeId: params.tree_id || params.treeId,
    includeAvailable: params.include_available ?? params.includeAvailable ?? true
  });
};

/**
 * List all skill trees
 */
const handleListSkillTrees: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'list_skill_trees', {
    classId: params.class_id || params.classId,
    includeHidden: params.include_hidden ?? params.includeHidden ?? false
  });
};

/**
 * List all classes
 */
const handleListClasses: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SKILL_TREE, 'list_classes', {
    includeSubclasses: params.include_subclasses ?? params.includeSubclasses ?? true,
    includeStats: params.include_stats ?? params.includeStats ?? false
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const skillTreeHandlers: ActionHandlerMap = {
  create_skill_tree: handleCreateSkillTree,
  add_skill_node: handleAddSkillNode,
  connect_skill_nodes: handleConnectSkillNodes,
  create_skill: handleCreateSkill,
  add_skill_effect: handleAddSkillEffect,
  add_skill_prerequisite: handleAddSkillPrerequisite,
  create_class: handleCreateClass,
  configure_subclass: handleConfigureSubclass,
  setup_multiclass: handleSetupMulticlass,
  configure_respec: handleConfigureRespec,
  unlock_skill: handleUnlockSkill,
  upgrade_skill: handleUpgradeSkill,
  respec_skills: handleRespecSkills,
  get_skill_tree: handleGetSkillTree,
  get_player_skills: handleGetPlayerSkills,
  list_skill_trees: handleListSkillTrees,
  list_classes: handleListClasses
};

/**
 * Main skill tree handler - routes to specific action handlers
 */
export const handleSkillTreeTools = createCategoryHandler(Forge2Categories.SKILL_TREE, skillTreeHandlers);

/**
 * Get available skill tree actions
 */
export function getSkillTreeActions(): string[] {
  return Object.keys(skillTreeHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeSkillTreeHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = skillTreeHandlers[action];
  if (!handler) {
    throw new Error(`Unknown skill tree action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
