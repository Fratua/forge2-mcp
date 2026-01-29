/**
 * FORGE2: Runtime Component Handlers
 * Handles actor components for quest tracking, dialogue, inventory, combat, and more
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
// Quest Tracker Component Handlers
// ============================================================================

/**
 * Add quest tracker component
 */
const handleAddQuestTrackerComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_quest_tracker_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'QuestTracker',
    autoRegister: validateOptionalBoolean(params.auto_register || params.autoRegister) ?? true,
    trackAllQuests: validateOptionalBoolean(params.track_all_quests || params.trackAllQuests) ?? false
  });
};

/**
 * Configure quest tracker
 */
const handleConfigureQuestTracker: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_quest_tracker', {
    actorPath,
    maxTrackedQuests: validateOptionalNumber(params.max_tracked_quests || params.maxTrackedQuests) ?? 5,
    autoAcceptQuests: validateOptionalBoolean(params.auto_accept_quests || params.autoAcceptQuests) ?? false,
    showObjectiveMarkers: validateOptionalBoolean(params.show_objective_markers || params.showObjectiveMarkers) ?? true
  });
};

// ============================================================================
// Dialogue Component Handlers
// ============================================================================

/**
 * Add dialogue component
 */
const handleAddDialogueComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_dialogue_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Dialogue',
    dialogueTreePath: validateOptionalString(params.dialogue_tree_path || params.dialogueTreePath),
    speakerProfile: validateOptionalString(params.speaker_profile || params.speakerProfile)
  });
};

/**
 * Configure dialogue component
 */
const handleConfigureDialogueComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_dialogue_component', {
    actorPath,
    interactionRadius: validateOptionalNumber(params.interaction_radius || params.interactionRadius) ?? 200.0,
    autoPlayOnInteract: validateOptionalBoolean(params.auto_play_on_interact || params.autoPlayOnInteract) ?? true,
    showIndicator: validateOptionalBoolean(params.show_indicator || params.showIndicator) ?? true
  });
};

// ============================================================================
// Inventory Component Handlers
// ============================================================================

/**
 * Add inventory component
 */
const handleAddInventoryComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_inventory_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Inventory',
    maxSlots: validateOptionalNumber(params.max_slots || params.maxSlots) ?? 50,
    maxWeight: validateOptionalNumber(params.max_weight || params.maxWeight) ?? 100.0
  });
};

/**
 * Configure inventory component
 */
const handleConfigureInventoryComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_inventory_component', {
    actorPath,
    enableStacking: validateOptionalBoolean(params.enable_stacking || params.enableStacking) ?? true,
    enableSorting: validateOptionalBoolean(params.enable_sorting || params.enableSorting) ?? true,
    persistOnSave: validateOptionalBoolean(params.persist_on_save || params.persistOnSave) ?? true,
    restrictedCategories: validateOptionalArray(params.restricted_categories || params.restrictedCategories) ?? []
  });
};

// ============================================================================
// Combat Component Handlers
// ============================================================================

/**
 * Add combat component
 */
const handleAddCombatComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_combat_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Combat',
    baseHealth: validateOptionalNumber(params.base_health || params.baseHealth) ?? 100.0,
    baseDamage: validateOptionalNumber(params.base_damage || params.baseDamage) ?? 10.0
  });
};

/**
 * Configure combat component
 */
const handleConfigureCombatComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_combat_component', {
    actorPath,
    enableTargetLock: validateOptionalBoolean(params.enable_target_lock || params.enableTargetLock) ?? true,
    enableCombos: validateOptionalBoolean(params.enable_combos || params.enableCombos) ?? true,
    combatStyle: validateOptionalString(params.combat_style || params.combatStyle) ?? 'melee',
    damageTypes: validateOptionalArray(params.damage_types || params.damageTypes) ?? []
  });
};

// ============================================================================
// Interaction Component Handlers
// ============================================================================

/**
 * Add interaction component
 */
const handleAddInteractionComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_interaction_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Interaction',
    interactionType: validateOptionalString(params.interaction_type || params.interactionType) ?? 'use',
    interactionRadius: validateOptionalNumber(params.interaction_radius || params.interactionRadius) ?? 150.0
  });
};

/**
 * Configure interaction component
 */
const handleConfigureInteractionComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_interaction_component', {
    actorPath,
    promptText: validateOptionalString(params.prompt_text || params.promptText) ?? 'Press E to interact',
    requireLineOfSight: validateOptionalBoolean(params.require_line_of_sight || params.requireLineOfSight) ?? true,
    cooldown: validateOptionalNumber(params.cooldown) ?? 0.0,
    oneTimeUse: validateOptionalBoolean(params.one_time_use || params.oneTimeUse) ?? false
  });
};

// ============================================================================
// Status Effect Component Handlers
// ============================================================================

/**
 * Add status effect component
 */
const handleAddStatusEffectComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_status_effect_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'StatusEffects',
    maxEffects: validateOptionalNumber(params.max_effects || params.maxEffects) ?? 10
  });
};

/**
 * Configure status effect component
 */
const handleConfigureStatusEffectComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_status_effect_component', {
    actorPath,
    allowStacking: validateOptionalBoolean(params.allow_stacking || params.allowStacking) ?? true,
    immuneEffects: validateOptionalArray(params.immune_effects || params.immuneEffects) ?? [],
    effectMultipliers: validateOptionalObject(params.effect_multipliers || params.effectMultipliers) ?? {}
  });
};

// ============================================================================
// Equipment Component Handlers
// ============================================================================

/**
 * Add equipment component
 */
const handleAddEquipmentComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_equipment_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Equipment',
    equipmentSlots: validateOptionalArray(params.equipment_slots || params.equipmentSlots) ?? ['head', 'chest', 'hands', 'legs', 'feet', 'mainhand', 'offhand']
  });
};

/**
 * Configure equipment component
 */
const handleConfigureEquipmentComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_equipment_component', {
    actorPath,
    visualEquipment: validateOptionalBoolean(params.visual_equipment || params.visualEquipment) ?? true,
    autoEquipBest: validateOptionalBoolean(params.auto_equip_best || params.autoEquipBest) ?? false,
    classRestrictions: validateOptionalBoolean(params.class_restrictions || params.classRestrictions) ?? true
  });
};

// ============================================================================
// Progression Component Handlers
// ============================================================================

/**
 * Add progression component
 */
const handleAddProgressionComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_progression_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Progression',
    startingLevel: validateOptionalNumber(params.starting_level || params.startingLevel) ?? 1,
    maxLevel: validateOptionalNumber(params.max_level || params.maxLevel) ?? 100
  });
};

/**
 * Configure progression component
 */
const handleConfigureProgressionComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_progression_component', {
    actorPath,
    xpCurveType: validateOptionalString(params.xp_curve_type || params.xpCurveType) ?? 'exponential',
    skillPointsPerLevel: validateOptionalNumber(params.skill_points_per_level || params.skillPointsPerLevel) ?? 1,
    attributePointsPerLevel: validateOptionalNumber(params.attribute_points_per_level || params.attributePointsPerLevel) ?? 5
  });
};

// ============================================================================
// Faction Component Handlers
// ============================================================================

/**
 * Add faction component
 */
const handleAddFactionComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_faction_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Faction',
    defaultFaction: validateOptionalString(params.default_faction || params.defaultFaction)
  });
};

/**
 * Configure faction component
 */
const handleConfigureFactionComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_faction_component', {
    actorPath,
    trackReputation: validateOptionalBoolean(params.track_reputation || params.trackReputation) ?? true,
    factionStandings: validateOptionalObject(params.faction_standings || params.factionStandings) ?? {},
    hostileFactions: validateOptionalArray(params.hostile_factions || params.hostileFactions) ?? []
  });
};

// ============================================================================
// Stealth Component Handlers
// ============================================================================

/**
 * Add stealth component
 */
const handleAddStealthComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_stealth_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Stealth',
    baseVisibility: validateOptionalNumber(params.base_visibility || params.baseVisibility) ?? 1.0
  });
};

/**
 * Configure stealth component
 */
const handleConfigureStealthComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_stealth_component', {
    actorPath,
    lightAffectsVisibility: validateOptionalBoolean(params.light_affects_visibility || params.lightAffectsVisibility) ?? true,
    movementAffectsVisibility: validateOptionalBoolean(params.movement_affects_visibility || params.movementAffectsVisibility) ?? true,
    crouchModifier: validateOptionalNumber(params.crouch_modifier || params.crouchModifier) ?? 0.5,
    proneModifier: validateOptionalNumber(params.prone_modifier || params.proneModifier) ?? 0.2
  });
};

// ============================================================================
// Needs Component Handlers
// ============================================================================

/**
 * Add needs component
 */
const handleAddNeedsComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'add_needs_component', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName) ?? 'Needs',
    needs: validateOptionalArray(params.needs) ?? ['hunger', 'thirst', 'sleep']
  });
};

/**
 * Configure needs component
 */
const handleConfigureNeedsComponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'configure_needs_component', {
    actorPath,
    needSettings: validateOptionalObject(params.need_settings || params.needSettings) ?? {},
    decayRates: validateOptionalObject(params.decay_rates || params.decayRates) ?? {},
    criticalThresholds: validateOptionalObject(params.critical_thresholds || params.criticalThresholds) ?? {}
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get component info
 */
const handleGetComponentInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'get_component_info', {
    actorPath,
    componentName: validateOptionalString(params.component_name || params.componentName),
    includeConfiguration: validateOptionalBoolean(params.include_configuration || params.includeConfiguration) ?? true
  });
};

/**
 * List actor components
 */
const handleListActorComponents: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.COMPONENTS, 'list_actor_components', {
    actorPath,
    componentType: validateOptionalString(params.component_type || params.componentType),
    mcpComponentsOnly: validateOptionalBoolean(params.mcp_components_only || params.mcpComponentsOnly) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const componentHandlers: ActionHandlerMap = {
  // Quest Tracker
  add_quest_tracker_component: handleAddQuestTrackerComponent,
  configure_quest_tracker: handleConfigureQuestTracker,

  // Dialogue
  add_dialogue_component: handleAddDialogueComponent,
  configure_dialogue_component: handleConfigureDialogueComponent,

  // Inventory
  add_inventory_component: handleAddInventoryComponent,
  configure_inventory_component: handleConfigureInventoryComponent,

  // Combat
  add_combat_component: handleAddCombatComponent,
  configure_combat_component: handleConfigureCombatComponent,

  // Interaction
  add_interaction_component: handleAddInteractionComponent,
  configure_interaction_component: handleConfigureInteractionComponent,

  // Status Effects
  add_status_effect_component: handleAddStatusEffectComponent,
  configure_status_effect_component: handleConfigureStatusEffectComponent,

  // Equipment
  add_equipment_component: handleAddEquipmentComponent,
  configure_equipment_component: handleConfigureEquipmentComponent,

  // Progression
  add_progression_component: handleAddProgressionComponent,
  configure_progression_component: handleConfigureProgressionComponent,

  // Faction
  add_faction_component: handleAddFactionComponent,
  configure_faction_component: handleConfigureFactionComponent,

  // Stealth
  add_stealth_component: handleAddStealthComponent,
  configure_stealth_component: handleConfigureStealthComponent,

  // Needs
  add_needs_component: handleAddNeedsComponent,
  configure_needs_component: handleConfigureNeedsComponent,

  // Query
  get_component_info: handleGetComponentInfo,
  list_actor_components: handleListActorComponents
};

export const handleComponentTools = createCategoryHandler(Forge2Categories.COMPONENTS, componentHandlers);

export default componentHandlers;
