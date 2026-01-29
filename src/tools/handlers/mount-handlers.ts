/**
 * FORGE2: Mount System Handlers
 * Handles mounts, mount abilities, customization, and mounted combat
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
// Mount Definition Action Handlers
// ============================================================================

/**
 * Create mount
 */
const handleCreateMount: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mountId = validateRequiredString(params.mount_id || params.mountId, 'mount_id');
  const mountName = validateRequiredString(params.mount_name || params.mountName, 'mount_name');

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'create_mount', {
    mountId,
    mountName,
    mountType: validateOptionalString(params.mount_type || params.mountType) ?? 'ground',
    meshPath: validateOptionalString(params.mesh_path || params.meshPath),
    animBlueprintPath: validateOptionalString(params.anim_blueprint_path || params.animBlueprintPath),
    rarity: validateOptionalString(params.rarity) ?? 'common',
    description: validateOptionalString(params.description)
  });
};

/**
 * Configure mount stats
 */
const handleConfigureMountStats: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mountId = validateRequiredString(params.mount_id || params.mountId, 'mount_id');

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'configure_mount_stats', {
    mountId,
    baseSpeed: validateOptionalNumber(params.base_speed || params.baseSpeed) ?? 600.0,
    sprintSpeed: validateOptionalNumber(params.sprint_speed || params.sprintSpeed) ?? 1000.0,
    acceleration: validateOptionalNumber(params.acceleration) ?? 1000.0,
    turnRate: validateOptionalNumber(params.turn_rate || params.turnRate) ?? 180.0,
    health: validateOptionalNumber(params.health) ?? 500,
    stamina: validateOptionalNumber(params.stamina) ?? 100,
    staminaRegenRate: validateOptionalNumber(params.stamina_regen_rate || params.staminaRegenRate) ?? 10.0
  });
};

/**
 * Configure mount abilities
 */
const handleConfigureMountAbilities: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mountId = validateRequiredString(params.mount_id || params.mountId, 'mount_id');

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'configure_mount_abilities', {
    mountId,
    canJump: validateOptionalBoolean(params.can_jump || params.canJump) ?? true,
    jumpHeight: validateOptionalNumber(params.jump_height || params.jumpHeight) ?? 400.0,
    canSprint: validateOptionalBoolean(params.can_sprint || params.canSprint) ?? true,
    canFly: validateOptionalBoolean(params.can_fly || params.canFly) ?? false,
    flySpeed: validateOptionalNumber(params.fly_speed || params.flySpeed) ?? 800.0,
    canSwim: validateOptionalBoolean(params.can_swim || params.canSwim) ?? true,
    swimSpeed: validateOptionalNumber(params.swim_speed || params.swimSpeed) ?? 400.0,
    canClimb: validateOptionalBoolean(params.can_climb || params.canClimb) ?? false
  });
};

// ============================================================================
// Mount Customization Action Handlers
// ============================================================================

/**
 * Setup mount customization
 */
const handleSetupMountCustomization: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'setup_mount_customization', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    armorSlots: validateOptionalArray(params.armor_slots || params.armorSlots) ?? ['head', 'body', 'legs', 'saddle'],
    dyeEnabled: validateOptionalBoolean(params.dye_enabled || params.dyeEnabled) ?? true,
    skinSystem: validateOptionalBoolean(params.skin_system || params.skinSystem) ?? true
  });
};

/**
 * Add mount armor
 */
const handleAddMountArmor: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const armorId = validateRequiredString(params.armor_id || params.armorId, 'armor_id');
  const armorName = validateRequiredString(params.armor_name || params.armorName, 'armor_name');

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'add_mount_armor', {
    armorId,
    armorName,
    slot: validateOptionalString(params.slot) ?? 'body',
    meshPath: validateOptionalString(params.mesh_path || params.meshPath),
    armorValue: validateOptionalNumber(params.armor_value || params.armorValue) ?? 10,
    speedModifier: validateOptionalNumber(params.speed_modifier || params.speedModifier) ?? 1.0,
    statBonuses: validateOptionalObject(params.stat_bonuses || params.statBonuses)
  });
};

// ============================================================================
// Mount Storage Action Handlers
// ============================================================================

/**
 * Configure mount storage
 */
const handleConfigureMountStorage: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mountId = validateRequiredString(params.mount_id || params.mountId, 'mount_id');

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'configure_mount_storage', {
    mountId,
    storageEnabled: validateOptionalBoolean(params.storage_enabled || params.storageEnabled) ?? true,
    inventorySlots: validateOptionalNumber(params.inventory_slots || params.inventorySlots) ?? 20,
    weightCapacity: validateOptionalNumber(params.weight_capacity || params.weightCapacity) ?? 200.0,
    accessWhileMounted: validateOptionalBoolean(params.access_while_mounted || params.accessWhileMounted) ?? false
  });
};

// ============================================================================
// Mounted Combat Action Handlers
// ============================================================================

/**
 * Setup mounted combat
 */
const handleSetupMountedCombat: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'setup_mounted_combat', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    meleeEnabled: validateOptionalBoolean(params.melee_enabled || params.meleeEnabled) ?? true,
    rangedEnabled: validateOptionalBoolean(params.ranged_enabled || params.rangedEnabled) ?? true,
    spellcastingEnabled: validateOptionalBoolean(params.spellcasting_enabled || params.spellcastingEnabled) ?? false,
    chargeAttack: validateOptionalBoolean(params.charge_attack || params.chargeAttack) ?? true,
    chargeDamageMultiplier: validateOptionalNumber(params.charge_damage_multiplier || params.chargeDamageMultiplier) ?? 2.0,
    trampleDamage: validateOptionalNumber(params.trample_damage || params.trampleDamage) ?? 50.0
  });
};

/**
 * Add mount ability
 */
const handleAddMountAbility: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mountId = validateRequiredString(params.mount_id || params.mountId, 'mount_id');
  const abilityId = validateRequiredString(params.ability_id || params.abilityId, 'ability_id');

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'add_mount_ability', {
    mountId,
    abilityId,
    abilityName: validateOptionalString(params.ability_name || params.abilityName) ?? abilityId,
    cooldown: validateOptionalNumber(params.cooldown) ?? 10.0,
    staminaCost: validateOptionalNumber(params.stamina_cost || params.staminaCost) ?? 20,
    damage: validateOptionalNumber(params.damage),
    effectPath: validateOptionalString(params.effect_path || params.effectPath)
  });
};

// ============================================================================
// Mount Management Action Handlers
// ============================================================================

/**
 * Setup mount stable
 */
const handleSetupMountStable: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'setup_mount_stable', {
    maxMounts: validateOptionalNumber(params.max_mounts || params.maxMounts) ?? 5,
    stableLocations: validateOptionalArray(params.stable_locations || params.stableLocations) ?? [],
    rentalSystem: validateOptionalBoolean(params.rental_system || params.rentalSystem) ?? false,
    breedingEnabled: validateOptionalBoolean(params.breeding_enabled || params.breedingEnabled) ?? false
  });
};

/**
 * Configure mount summon
 */
const handleConfigureMountSummon: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'configure_mount_summon', {
    summonTime: validateOptionalNumber(params.summon_time || params.summonTime) ?? 2.0,
    dismissTime: validateOptionalNumber(params.dismiss_time || params.dismissTime) ?? 1.0,
    summonRange: validateOptionalNumber(params.summon_range || params.summonRange),
    combatRestrictions: validateOptionalBoolean(params.combat_restrictions || params.combatRestrictions) ?? true,
    indoorRestrictions: validateOptionalBoolean(params.indoor_restrictions || params.indoorRestrictions) ?? true,
    summonEffect: validateOptionalString(params.summon_effect || params.summonEffect),
    dismissEffect: validateOptionalString(params.dismiss_effect || params.dismissEffect)
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get mount system info
 */
const handleGetMountSystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'get_mount_system_info', {
    mountId: validateOptionalString(params.mount_id || params.mountId),
    includeStats: validateOptionalBoolean(params.include_stats || params.includeStats) ?? true,
    includeAbilities: validateOptionalBoolean(params.include_abilities || params.includeAbilities) ?? true
  });
};

/**
 * List mounts
 */
const handleListMounts: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MOUNT_SYSTEM, 'list_mounts', {
    mountType: validateOptionalString(params.mount_type || params.mountType),
    rarity: validateOptionalString(params.rarity),
    includeUnlocked: validateOptionalBoolean(params.include_unlocked || params.includeUnlocked) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const mountHandlers: ActionHandlerMap = {
  // Definition
  create_mount: handleCreateMount,
  configure_mount_stats: handleConfigureMountStats,
  configure_mount_abilities: handleConfigureMountAbilities,

  // Customization
  setup_mount_customization: handleSetupMountCustomization,
  add_mount_armor: handleAddMountArmor,

  // Storage
  configure_mount_storage: handleConfigureMountStorage,

  // Combat
  setup_mounted_combat: handleSetupMountedCombat,
  add_mount_ability: handleAddMountAbility,

  // Management
  setup_mount_stable: handleSetupMountStable,
  configure_mount_summon: handleConfigureMountSummon,

  // Query
  get_mount_system_info: handleGetMountSystemInfo,
  list_mounts: handleListMounts
};

export const handleMountTools = createCategoryHandler(Forge2Categories.MOUNT_SYSTEM, mountHandlers);

export default mountHandlers;
