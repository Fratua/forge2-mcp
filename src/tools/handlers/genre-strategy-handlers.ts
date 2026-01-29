/**
 * FORGE2: Strategy Genre Handlers
 * Complete RTS/strategy mechanics (StarCraft, Age of Empires, Civilization style)
 *
 * Supports: Units, buildings, resources, tech trees, fog of war
 * Systems: Formation, command, AI opponents, production queues
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

function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value;
}

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

export type UnitType = 'infantry' | 'vehicle' | 'air' | 'naval' | 'siege' | 'support' | 'hero';
export type BuildingType = 'base' | 'production' | 'tech' | 'defense' | 'resource' | 'support';
export type ResourceType = 'primary' | 'secondary' | 'tertiary' | 'population' | 'special';
export type CommandType = 'move' | 'attack' | 'patrol' | 'hold' | 'stop' | 'guard' | 'follow' | 'gather';
export type FormationType = 'line' | 'column' | 'wedge' | 'box' | 'circle' | 'spread' | 'custom';
export type AIStrategy = 'rush' | 'turtle' | 'boom' | 'harass' | 'balanced' | 'adaptive';

// ============================================================================
// Constants
// ============================================================================

const UNIT_TYPES = ['infantry', 'vehicle', 'air', 'naval', 'siege', 'support', 'hero'] as const;
const BUILDING_TYPES = ['base', 'production', 'tech', 'defense', 'resource', 'support'] as const;
const RESOURCE_TYPES = ['primary', 'secondary', 'tertiary', 'population', 'special'] as const;
const COMMAND_TYPES = ['move', 'attack', 'patrol', 'hold', 'stop', 'guard', 'follow', 'gather'] as const;
const FORMATION_TYPES = ['line', 'column', 'wedge', 'box', 'circle', 'spread', 'custom'] as const;
const AI_STRATEGIES = ['rush', 'turtle', 'boom', 'harass', 'balanced', 'adaptive'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create unit type
 */
const handleCreateUnitType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const unitId = validateRequiredString(params.unit_id || params.unitId, 'unit_id');
  const unitName = validateRequiredString(params.unit_name || params.unitName, 'unit_name');

  const unitType = validateRequiredEnum(
    params.unit_type || params.unitType,
    UNIT_TYPES,
    'unit_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'create_unit_type', {
    unitId,
    unitName,
    unitType,
    meshPath: params.mesh_path || params.meshPath,
    health: params.health ?? 100,
    armor: params.armor ?? 0,
    armorType: params.armor_type || params.armorType || 'normal',
    damage: params.damage ?? 10,
    damageType: params.damage_type || params.damageType || 'normal',
    attackSpeed: params.attack_speed ?? params.attackSpeed ?? 1.0,
    attackRange: params.attack_range ?? params.attackRange ?? 100,
    moveSpeed: params.move_speed ?? params.moveSpeed ?? 300,
    sightRange: params.sight_range ?? params.sightRange ?? 800,
    buildTime: params.build_time ?? params.buildTime ?? 15,
    cost: params.cost,
    populationCost: params.population_cost ?? params.populationCost ?? 1,
    abilities: params.abilities as string[] | undefined,
    prerequisites: params.prerequisites as string[] | undefined
  });
};

/**
 * Configure building system
 */
const handleConfigureBuildingSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'configure_building_system', {
    requireBuilder: params.require_builder ?? params.requireBuilder ?? true,
    builderUnitId: params.builder_unit_id || params.builderUnitId,
    foundationRequired: params.foundation_required ?? params.foundationRequired ?? false,
    buildingPlacement: params.building_placement || params.buildingPlacement || 'grid',
    gridSize: params.grid_size ?? params.gridSize ?? 100,
    canBuildOnResources: params.can_build_on_resources ?? params.canBuildOnResources ?? false,
    repairEnabled: params.repair_enabled ?? params.repairEnabled ?? true,
    sellEnabled: params.sell_enabled ?? params.sellEnabled ?? true,
    sellRefundPercent: params.sell_refund_percent ?? params.sellRefundPercent ?? 0.5
  });
};

/**
 * Create building type
 */
const handleCreateBuildingType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const buildingId = validateRequiredString(params.building_id || params.buildingId, 'building_id');
  const buildingName = validateRequiredString(params.building_name || params.buildingName, 'building_name');

  const buildingType = validateRequiredEnum(
    params.building_type || params.buildingType,
    BUILDING_TYPES,
    'building_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'create_building_type', {
    buildingId,
    buildingName,
    buildingType,
    meshPath: params.mesh_path || params.meshPath,
    health: params.health ?? 1000,
    armor: params.armor ?? 0,
    sightRange: params.sight_range ?? params.sightRange ?? 500,
    buildTime: params.build_time ?? params.buildTime ?? 30,
    cost: params.cost,
    size: params.size,
    producesUnits: params.produces_units || params.producesUnits,
    researchesTech: params.researches_tech || params.researchesTech,
    providesPopulation: params.provides_population ?? params.providesPopulation ?? 0,
    generatesResources: params.generates_resources || params.generatesResources,
    prerequisites: params.prerequisites as string[] | undefined,
    abilities: params.abilities as string[] | undefined
  });
};

/**
 * Setup resource system
 */
const handleSetupResourceSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'setup_resource_system', {
    resources: params.resources,
    startingResources: params.starting_resources || params.startingResources,
    maxStorage: params.max_storage || params.maxStorage,
    gatherRate: params.gather_rate ?? params.gatherRate ?? 1,
    refinementRequired: params.refinement_required ?? params.refinementRequired ?? false,
    dropoffRequired: params.dropoff_required ?? params.dropoffRequired ?? true,
    resourceNodes: params.resource_nodes || params.resourceNodes
  });
};

/**
 * Create resource type
 */
const handleCreateResourceType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const resourceId = validateRequiredString(params.resource_id || params.resourceId, 'resource_id');
  const resourceName = validateRequiredString(params.resource_name || params.resourceName, 'resource_name');

  const resourceType = validateRequiredEnum(
    params.resource_type || params.resourceType,
    RESOURCE_TYPES,
    'resource_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'create_resource_type', {
    resourceId,
    resourceName,
    resourceType,
    iconPath: params.icon_path || params.iconPath,
    color: params.color,
    startAmount: params.start_amount ?? params.startAmount ?? 0,
    maxAmount: params.max_amount ?? params.maxAmount ?? 99999,
    gatherRate: params.gather_rate ?? params.gatherRate ?? 1,
    refinementRatio: params.refinement_ratio ?? params.refinementRatio ?? 1.0,
    nodeMeshPath: params.node_mesh_path || params.nodeMeshPath
  });
};

/**
 * Create tech tree
 */
const handleCreateTechTree: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const factionId = validateRequiredString(params.faction_id || params.factionId, 'faction_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'create_tech_tree', {
    factionId,
    technologies: params.technologies,
    ages: params.ages,
    ageUpRequirements: params.age_up_requirements || params.ageUpRequirements
  });
};

/**
 * Create technology
 */
const handleCreateTechnology: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const techId = validateRequiredString(params.tech_id || params.techId, 'tech_id');
  const techName = validateRequiredString(params.tech_name || params.techName, 'tech_name');

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'create_technology', {
    techId,
    techName,
    description: params.description as string | undefined,
    iconPath: params.icon_path || params.iconPath,
    researchTime: params.research_time ?? params.researchTime ?? 30,
    cost: params.cost,
    researchBuilding: params.research_building || params.researchBuilding,
    prerequisites: params.prerequisites as string[] | undefined,
    effects: params.effects,
    unlocksUnits: params.unlocks_units || params.unlocksUnits,
    unlocksBuildings: params.unlocks_buildings || params.unlocksBuildings,
    unlocksAbilities: params.unlocks_abilities || params.unlocksAbilities
  });
};

/**
 * Configure fog of war
 */
const handleConfigureFogOfWar: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'configure_fog_of_war', {
    enabled: params.enabled ?? true,
    fogStyle: params.fog_style || params.fogStyle || 'explored',
    revealOnExplore: params.reveal_on_explore ?? params.revealOnExplore ?? true,
    shroudUnexplored: params.shroud_unexplored ?? params.shroudUnexplored ?? true,
    fogColor: params.fog_color || params.fogColor,
    shroudColor: params.shroud_color || params.shroudColor,
    updateRate: params.update_rate ?? params.updateRate ?? 0.2,
    shareVision: params.share_vision ?? params.shareVision ?? true
  });
};

/**
 * Setup formation system
 */
const handleSetupFormationSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'setup_formation_system', {
    enabled: params.enabled ?? true,
    defaultFormation: params.default_formation || params.defaultFormation || 'line',
    autoFormation: params.auto_formation ?? params.autoFormation ?? true,
    formationSpacing: params.formation_spacing ?? params.formationSpacing ?? 100,
    rotationEnabled: params.rotation_enabled ?? params.rotationEnabled ?? true,
    formations: params.formations
  });
};

/**
 * Create formation
 */
const handleCreateFormation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const formationType = validateRequiredEnum(
    params.formation_type || params.formationType,
    FORMATION_TYPES,
    'formation_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'create_formation', {
    formationType,
    spacing: params.spacing ?? 100,
    ranks: params.ranks ?? 3,
    moveSpeedMultiplier: params.move_speed_multiplier ?? params.moveSpeedMultiplier ?? 1.0,
    attackBonus: params.attack_bonus ?? params.attackBonus ?? 0,
    defenseBonus: params.defense_bonus ?? params.defenseBonus ?? 0,
    positionOffsets: params.position_offsets || params.positionOffsets
  });
};

/**
 * Create command system
 */
const handleCreateCommandSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'create_command_system', {
    enabledCommands: params.enabled_commands || params.enabledCommands || ['move', 'attack', 'stop'],
    rightClickAction: params.right_click_action || params.rightClickAction || 'smart',
    attackMoveEnabled: params.attack_move_enabled ?? params.attackMoveEnabled ?? true,
    waypointsEnabled: params.waypoints_enabled ?? params.waypointsEnabled ?? true,
    maxWaypoints: params.max_waypoints ?? params.maxWaypoints ?? 20,
    patrolBehavior: params.patrol_behavior || params.patrolBehavior || 'loop',
    guardRange: params.guard_range ?? params.guardRange ?? 500
  });
};

/**
 * Configure AI opponent
 */
const handleConfigureAIOpponent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const strategy = validateOptionalEnum(
    params.strategy,
    AI_STRATEGIES,
    'strategy'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'configure_ai_opponent', {
    difficulty: params.difficulty || 'medium',
    strategy: strategy || 'balanced',
    cheating: params.cheating ?? false,
    cheatResourceMultiplier: params.cheat_resource_multiplier ?? params.cheatResourceMultiplier ?? 1.0,
    cheatBuildSpeedMultiplier: params.cheat_build_speed_multiplier ?? params.cheatBuildSpeedMultiplier ?? 1.0,
    aggressionLevel: params.aggression_level ?? params.aggressionLevel ?? 0.5,
    expansionPriority: params.expansion_priority ?? params.expansionPriority ?? 0.5,
    scoutingFrequency: params.scouting_frequency ?? params.scoutingFrequency ?? 60,
    adaptToPlayer: params.adapt_to_player ?? params.adaptToPlayer ?? true
  });
};

/**
 * Create production queue
 */
const handleCreateProductionQueue: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const buildingId = validateRequiredString(params.building_id || params.buildingId, 'building_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'create_production_queue', {
    buildingId,
    maxQueueSize: params.max_queue_size ?? params.maxQueueSize ?? 5,
    simultaneousProduction: params.simultaneous_production ?? params.simultaneousProduction ?? 1,
    rallyPointEnabled: params.rally_point_enabled ?? params.rallyPointEnabled ?? true,
    autoQueueEnabled: params.auto_queue_enabled ?? params.autoQueueEnabled ?? false,
    cancelRefundPercent: params.cancel_refund_percent ?? params.cancelRefundPercent ?? 1.0
  });
};

/**
 * Issue command
 */
const handleIssueCommand: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const commandType = validateRequiredEnum(
    params.command_type || params.commandType,
    COMMAND_TYPES,
    'command_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'issue_command', {
    commandType,
    unitIds: params.unit_ids || params.unitIds,
    targetLocation: params.target_location || params.targetLocation,
    targetUnitId: params.target_unit_id || params.targetUnitId,
    targetBuildingId: params.target_building_id || params.targetBuildingId,
    queued: params.queued ?? false,
    playerId: params.player_id || params.playerId
  });
};

/**
 * Get player resources
 */
const handleGetPlayerResources: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'get_player_resources', {
    playerId: params.player_id || params.playerId
  });
};

/**
 * List unit types
 */
const handleListUnitTypes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const unitType = validateOptionalEnum(
    params.unit_type || params.unitType,
    UNIT_TYPES,
    'unit_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STRATEGY, 'list_unit_types', {
    unitType,
    factionId: params.faction_id || params.factionId
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const genreStrategyHandlers: ActionHandlerMap = {
  create_unit_type: handleCreateUnitType,
  configure_building_system: handleConfigureBuildingSystem,
  create_building_type: handleCreateBuildingType,
  setup_resource_system: handleSetupResourceSystem,
  create_resource_type: handleCreateResourceType,
  create_tech_tree: handleCreateTechTree,
  create_technology: handleCreateTechnology,
  configure_fog_of_war: handleConfigureFogOfWar,
  setup_formation_system: handleSetupFormationSystem,
  create_formation: handleCreateFormation,
  create_command_system: handleCreateCommandSystem,
  configure_ai_opponent: handleConfigureAIOpponent,
  create_production_queue: handleCreateProductionQueue,
  issue_command: handleIssueCommand,
  get_player_resources: handleGetPlayerResources,
  list_unit_types: handleListUnitTypes
};

/**
 * Main genre strategy handler - routes to specific action handlers
 */
export const handleGenreStrategyTools = createCategoryHandler(Forge2Categories.GENRE_STRATEGY, genreStrategyHandlers);

/**
 * Get available genre strategy actions
 */
export function getGenreStrategyActions(): string[] {
  return Object.keys(genreStrategyHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeGenreStrategyHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = genreStrategyHandlers[action];
  if (!handler) {
    throw new Error(`Unknown genre strategy action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
