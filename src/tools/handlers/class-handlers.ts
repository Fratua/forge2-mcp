/**
 * FORGE2: Class System Handlers
 * Handles character classes, subclasses, roles, and multiclassing
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
// Class Definition Action Handlers
// ============================================================================

/**
 * Create class
 */
const handleCreateClass: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const classId = validateRequiredString(params.class_id || params.classId, 'class_id');
  const className = validateRequiredString(params.class_name || params.className, 'class_name');

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'create_class', {
    classId,
    className,
    description: validateOptionalString(params.description),
    iconPath: validateOptionalString(params.icon_path || params.iconPath),
    role: validateOptionalString(params.role) ?? 'damage',
    primaryStat: validateOptionalString(params.primary_stat || params.primaryStat) ?? 'strength',
    secondaryStat: validateOptionalString(params.secondary_stat || params.secondaryStat),
    healthPerLevel: validateOptionalNumber(params.health_per_level || params.healthPerLevel) ?? 10,
    resourceType: validateOptionalString(params.resource_type || params.resourceType) ?? 'mana',
    resourcePerLevel: validateOptionalNumber(params.resource_per_level || params.resourcePerLevel) ?? 5
  });
};

/**
 * Configure class base stats
 */
const handleConfigureClassBaseStats: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const classId = validateRequiredString(params.class_id || params.classId, 'class_id');

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'configure_class_base_stats', {
    classId,
    baseHealth: validateOptionalNumber(params.base_health || params.baseHealth) ?? 100,
    baseResource: validateOptionalNumber(params.base_resource || params.baseResource) ?? 50,
    baseStats: validateOptionalObject(params.base_stats || params.baseStats) ?? {
      strength: 10,
      dexterity: 10,
      intelligence: 10,
      constitution: 10
    },
    statGrowth: validateOptionalObject(params.stat_growth || params.statGrowth)
  });
};

/**
 * Add class ability
 */
const handleAddClassAbility: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const classId = validateRequiredString(params.class_id || params.classId, 'class_id');
  const abilityId = validateRequiredString(params.ability_id || params.abilityId, 'ability_id');

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'add_class_ability', {
    classId,
    abilityId,
    unlockLevel: validateOptionalNumber(params.unlock_level || params.unlockLevel) ?? 1,
    isSignature: validateOptionalBoolean(params.is_signature || params.isSignature) ?? false,
    prerequisiteAbility: validateOptionalString(params.prerequisite_ability || params.prerequisiteAbility)
  });
};

/**
 * Configure equipment restrictions
 */
const handleConfigureEquipmentRestrictions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const classId = validateRequiredString(params.class_id || params.classId, 'class_id');

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'configure_equipment_restrictions', {
    classId,
    allowedArmorTypes: validateOptionalArray(params.allowed_armor_types || params.allowedArmorTypes) ?? ['cloth', 'leather'],
    allowedWeaponTypes: validateOptionalArray(params.allowed_weapon_types || params.allowedWeaponTypes) ?? ['sword', 'staff'],
    disallowedSlots: validateOptionalArray(params.disallowed_slots || params.disallowedSlots) ?? []
  });
};

// ============================================================================
// Subclass Action Handlers
// ============================================================================

/**
 * Create subclass
 */
const handleCreateSubclass: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const subclassId = validateRequiredString(params.subclass_id || params.subclassId, 'subclass_id');
  const subclassName = validateRequiredString(params.subclass_name || params.subclassName, 'subclass_name');
  const parentClassId = validateRequiredString(params.parent_class_id || params.parentClassId, 'parent_class_id');

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'create_subclass', {
    subclassId,
    subclassName,
    parentClassId,
    description: validateOptionalString(params.description),
    iconPath: validateOptionalString(params.icon_path || params.iconPath),
    unlockLevel: validateOptionalNumber(params.unlock_level || params.unlockLevel) ?? 10,
    unlockQuestId: validateOptionalString(params.unlock_quest_id || params.unlockQuestId)
  });
};

/**
 * Add subclass bonus
 */
const handleAddSubclassBonus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const subclassId = validateRequiredString(params.subclass_id || params.subclassId, 'subclass_id');

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'add_subclass_bonus', {
    subclassId,
    bonusType: validateRequiredString(params.bonus_type || params.bonusType, 'bonus_type'),
    bonusValue: validateOptionalNumber(params.bonus_value || params.bonusValue) ?? 0,
    bonusStat: validateOptionalString(params.bonus_stat || params.bonusStat),
    bonusAbility: validateOptionalString(params.bonus_ability || params.bonusAbility),
    description: validateOptionalString(params.description)
  });
};

// ============================================================================
// Role System Action Handlers
// ============================================================================

/**
 * Setup role system
 */
const handleSetupRoleSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'setup_role_system', {
    roles: validateOptionalArray(params.roles) ?? ['tank', 'healer', 'damage', 'support'],
    roleQueueEnabled: validateOptionalBoolean(params.role_queue_enabled || params.roleQueueEnabled) ?? false,
    hybridRolesAllowed: validateOptionalBoolean(params.hybrid_roles_allowed || params.hybridRolesAllowed) ?? true,
    roleBonus: validateOptionalBoolean(params.role_bonus || params.roleBonus) ?? true
  });
};

/**
 * Configure role bonuses
 */
const handleConfigureRoleBonuses: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const role = validateRequiredString(params.role, 'role');

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'configure_role_bonuses', {
    role,
    threatModifier: validateOptionalNumber(params.threat_modifier || params.threatModifier) ?? 1.0,
    healingModifier: validateOptionalNumber(params.healing_modifier || params.healingModifier) ?? 1.0,
    damageModifier: validateOptionalNumber(params.damage_modifier || params.damageModifier) ?? 1.0,
    defensiveModifier: validateOptionalNumber(params.defensive_modifier || params.defensiveModifier) ?? 1.0,
    specialBonuses: validateOptionalObject(params.special_bonuses || params.specialBonuses)
  });
};

// ============================================================================
// Multiclass Action Handlers
// ============================================================================

/**
 * Setup multiclassing
 */
const handleSetupMulticlassing: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'setup_multiclassing', {
    enabled: validateOptionalBoolean(params.enabled) ?? false,
    maxClasses: validateOptionalNumber(params.max_classes || params.maxClasses) ?? 2,
    minimumLevelToMulticlass: validateOptionalNumber(params.minimum_level_to_multiclass || params.minimumLevelToMulticlass) ?? 10,
    sharedAbilities: validateOptionalBoolean(params.shared_abilities || params.sharedAbilities) ?? true,
    statPenalty: validateOptionalBoolean(params.stat_penalty || params.statPenalty) ?? false
  });
};

/**
 * Configure multiclass restrictions
 */
const handleConfigureMulticlassRestrictions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'configure_multiclass_restrictions', {
    incompatibleClasses: validateOptionalArray(params.incompatible_classes || params.incompatibleClasses) ?? [],
    requiredStatMinimums: validateOptionalObject(params.required_stat_minimums || params.requiredStatMinimums),
    questRequirement: validateOptionalString(params.quest_requirement || params.questRequirement)
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get class system info
 */
const handleGetClassSystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'get_class_system_info', {
    classId: validateOptionalString(params.class_id || params.classId),
    includeAbilities: validateOptionalBoolean(params.include_abilities || params.includeAbilities) ?? true,
    includeSubclasses: validateOptionalBoolean(params.include_subclasses || params.includeSubclasses) ?? true
  });
};

/**
 * List classes
 */
const handleListClasses: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CLASS_SYSTEM, 'list_classes', {
    role: validateOptionalString(params.role),
    includeSubclasses: validateOptionalBoolean(params.include_subclasses || params.includeSubclasses) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const classHandlers: ActionHandlerMap = {
  // Class Definition
  create_class: handleCreateClass,
  configure_class_base_stats: handleConfigureClassBaseStats,
  add_class_ability: handleAddClassAbility,
  configure_equipment_restrictions: handleConfigureEquipmentRestrictions,

  // Subclass
  create_subclass: handleCreateSubclass,
  add_subclass_bonus: handleAddSubclassBonus,

  // Roles
  setup_role_system: handleSetupRoleSystem,
  configure_role_bonuses: handleConfigureRoleBonuses,

  // Multiclass
  setup_multiclassing: handleSetupMulticlassing,
  configure_multiclass_restrictions: handleConfigureMulticlassRestrictions,

  // Query
  get_class_system_info: handleGetClassSystemInfo,
  list_classes: handleListClasses
};

export const handleClassTools = createCategoryHandler(Forge2Categories.CLASS_SYSTEM, classHandlers);

export default classHandlers;
