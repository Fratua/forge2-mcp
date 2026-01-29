/**
 * FORGE2: Advanced AI Handlers
 * Handles GOAP (Goal-Oriented Action Planning), Utility AI, Influence Maps, and Flocking
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

const GOAP_ACTION_TYPES = ['move_to', 'attack', 'defend', 'heal', 'gather', 'build', 'interact', 'custom'] as const;
const UTILITY_CONSIDERATION_TYPES = ['linear', 'quadratic', 'sigmoid', 'exponential', 'inverse', 'custom'] as const;
const INFLUENCE_MAP_TYPES = ['threat', 'resource', 'terrain', 'visibility', 'objective', 'combined'] as const;
const FLOCKING_BEHAVIORS = ['separation', 'alignment', 'cohesion', 'obstacle_avoidance', 'seek', 'flee'] as const;

// Reserve constants for future use
void GOAP_ACTION_TYPES;
void UTILITY_CONSIDERATION_TYPES;
void INFLUENCE_MAP_TYPES;
void FLOCKING_BEHAVIORS;

// ============================================================================
// GOAP Action Handlers
// ============================================================================

/**
 * Create GOAP agent
 */
const handleCreateGOAPAgent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const agentName = validateRequiredString(params.agent_name || params.agentName, 'agent_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'create_goap_agent', {
    agentName,
    maxPlanningDepth: validateOptionalNumber(params.max_planning_depth || params.maxPlanningDepth) ?? 10,
    planningTimeout: validateOptionalNumber(params.planning_timeout || params.planningTimeout) ?? 0.1,
    replanningInterval: validateOptionalNumber(params.replanning_interval || params.replanningInterval) ?? 1.0,
    worldStateClass: validateOptionalString(params.world_state_class || params.worldStateClass),
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Define GOAP goal
 */
const handleDefineGOAPGoal: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const agentPath = validateRequiredString(params.agent_path || params.agentPath, 'agent_path');
  const goalName = validateRequiredString(params.goal_name || params.goalName, 'goal_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'define_goap_goal', {
    agentPath,
    goalName,
    priority: validateOptionalNumber(params.priority) ?? 1.0,
    desiredState: validateOptionalObject(params.desired_state || params.desiredState) ?? {},
    preconditions: validateOptionalObject(params.preconditions) ?? {},
    dynamicPriority: validateOptionalBoolean(params.dynamic_priority || params.dynamicPriority) ?? false,
    priorityFunction: validateOptionalString(params.priority_function || params.priorityFunction)
  });
};

/**
 * Create GOAP action
 */
const handleCreateGOAPAction: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const agentPath = validateRequiredString(params.agent_path || params.agentPath, 'agent_path');
  const actionName = validateRequiredString(params.action_name || params.actionName, 'action_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'create_goap_action', {
    agentPath,
    actionName,
    cost: validateOptionalNumber(params.cost) ?? 1.0,
    preconditions: validateOptionalObject(params.preconditions) ?? {},
    effects: validateOptionalObject(params.effects) ?? {},
    interruptible: validateOptionalBoolean(params.interruptible) ?? true,
    executionClass: validateOptionalString(params.execution_class || params.executionClass)
  });
};

/**
 * Configure GOAP world state
 */
const handleConfigureGOAPWorldState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const agentPath = validateRequiredString(params.agent_path || params.agentPath, 'agent_path');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'configure_goap_world_state', {
    agentPath,
    stateProperties: validateOptionalArray(params.state_properties || params.stateProperties) ?? [],
    sensors: validateOptionalArray(params.sensors) ?? [],
    updateFrequency: validateOptionalNumber(params.update_frequency || params.updateFrequency) ?? 0.1
  });
};

// ============================================================================
// Utility AI Action Handlers
// ============================================================================

/**
 * Create Utility AI brain
 */
const handleCreateUtilityBrain: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const brainName = validateRequiredString(params.brain_name || params.brainName, 'brain_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'create_utility_brain', {
    brainName,
    decisionInterval: validateOptionalNumber(params.decision_interval || params.decisionInterval) ?? 0.5,
    inertiaFactor: validateOptionalNumber(params.inertia_factor || params.inertiaFactor) ?? 0.2,
    randomization: validateOptionalNumber(params.randomization) ?? 0.0,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add Utility action
 */
const handleAddUtilityAction: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const brainPath = validateRequiredString(params.brain_path || params.brainPath, 'brain_path');
  const actionName = validateRequiredString(params.action_name || params.actionName, 'action_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'add_utility_action', {
    brainPath,
    actionName,
    baseScore: validateOptionalNumber(params.base_score || params.baseScore) ?? 1.0,
    weight: validateOptionalNumber(params.weight) ?? 1.0,
    cooldown: validateOptionalNumber(params.cooldown) ?? 0.0,
    executionClass: validateOptionalString(params.execution_class || params.executionClass)
  });
};

/**
 * Add consideration to action
 */
const handleAddConsideration: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const brainPath = validateRequiredString(params.brain_path || params.brainPath, 'brain_path');
  const actionName = validateRequiredString(params.action_name || params.actionName, 'action_name');
  const considerationName = validateRequiredString(params.consideration_name || params.considerationName, 'consideration_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'add_consideration', {
    brainPath,
    actionName,
    considerationName,
    inputSource: validateRequiredString(params.input_source || params.inputSource, 'input_source'),
    curveType: validateOptionalString(params.curve_type || params.curveType) ?? 'linear',
    curveParams: validateOptionalObject(params.curve_params || params.curveParams),
    weight: validateOptionalNumber(params.weight) ?? 1.0
  });
};

/**
 * Configure utility response curve
 */
const handleConfigureResponseCurve: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const brainPath = validateRequiredString(params.brain_path || params.brainPath, 'brain_path');
  const curveName = validateRequiredString(params.curve_name || params.curveName, 'curve_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'configure_response_curve', {
    brainPath,
    curveName,
    curveType: validateOptionalString(params.curve_type || params.curveType) ?? 'linear',
    slope: validateOptionalNumber(params.slope) ?? 1.0,
    exponent: validateOptionalNumber(params.exponent) ?? 2.0,
    xShift: validateOptionalNumber(params.x_shift || params.xShift) ?? 0.0,
    yShift: validateOptionalNumber(params.y_shift || params.yShift) ?? 0.0,
    customCurve: validateOptionalArray(params.custom_curve || params.customCurve)
  });
};

// ============================================================================
// Influence Map Action Handlers
// ============================================================================

/**
 * Create influence map
 */
const handleCreateInfluenceMap: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mapName = validateRequiredString(params.map_name || params.mapName, 'map_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'create_influence_map', {
    mapName,
    resolution: validateOptionalNumber(params.resolution) ?? 100.0,
    updateFrequency: validateOptionalNumber(params.update_frequency || params.updateFrequency) ?? 0.5,
    decayRate: validateOptionalNumber(params.decay_rate || params.decayRate) ?? 0.1,
    propagationIterations: validateOptionalNumber(params.propagation_iterations || params.propagationIterations) ?? 3,
    bounds: validateOptionalObject(params.bounds),
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add influence source
 */
const handleAddInfluenceSource: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mapPath = validateRequiredString(params.map_path || params.mapPath, 'map_path');
  const sourceName = validateRequiredString(params.source_name || params.sourceName, 'source_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'add_influence_source', {
    mapPath,
    sourceName,
    sourceType: validateOptionalString(params.source_type || params.sourceType) ?? 'threat',
    baseValue: validateOptionalNumber(params.base_value || params.baseValue) ?? 1.0,
    radius: validateOptionalNumber(params.radius) ?? 500.0,
    falloff: validateOptionalString(params.falloff) ?? 'linear',
    dynamic: validateOptionalBoolean(params.dynamic) ?? true,
    actorFilter: validateOptionalString(params.actor_filter || params.actorFilter)
  });
};

/**
 * Query influence map
 */
const handleQueryInfluenceMap: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mapPath = validateRequiredString(params.map_path || params.mapPath, 'map_path');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'query_influence_map', {
    mapPath,
    queryLocation: validateOptionalObject(params.query_location || params.queryLocation),
    queryRadius: validateOptionalNumber(params.query_radius || params.queryRadius) ?? 100.0,
    queryType: validateOptionalString(params.query_type || params.queryType) ?? 'highest',
    includeTypes: validateOptionalArray(params.include_types || params.includeTypes)
  });
};

/**
 * Combine influence maps
 */
const handleCombineInfluenceMaps: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const outputName = validateRequiredString(params.output_name || params.outputName, 'output_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'combine_influence_maps', {
    outputName,
    sourceMaps: validateOptionalArray(params.source_maps || params.sourceMaps) ?? [],
    weights: validateOptionalArray(params.weights) ?? [],
    combineOperation: validateOptionalString(params.combine_operation || params.combineOperation) ?? 'weighted_sum',
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

// ============================================================================
// Flocking/Swarm Action Handlers
// ============================================================================

/**
 * Create flocking system
 */
const handleCreateFlockingSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemName = validateRequiredString(params.system_name || params.systemName, 'system_name');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'create_flocking_system', {
    systemName,
    maxAgents: validateOptionalNumber(params.max_agents || params.maxAgents) ?? 100,
    spatialPartitionSize: validateOptionalNumber(params.spatial_partition_size || params.spatialPartitionSize) ?? 200.0,
    updateFrequency: validateOptionalNumber(params.update_frequency || params.updateFrequency) ?? 0.033,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Configure flocking behavior
 */
const handleConfigureFlockingBehavior: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'configure_flocking_behavior', {
    systemPath,
    separationWeight: validateOptionalNumber(params.separation_weight || params.separationWeight) ?? 1.0,
    separationRadius: validateOptionalNumber(params.separation_radius || params.separationRadius) ?? 100.0,
    alignmentWeight: validateOptionalNumber(params.alignment_weight || params.alignmentWeight) ?? 1.0,
    alignmentRadius: validateOptionalNumber(params.alignment_radius || params.alignmentRadius) ?? 200.0,
    cohesionWeight: validateOptionalNumber(params.cohesion_weight || params.cohesionWeight) ?? 1.0,
    cohesionRadius: validateOptionalNumber(params.cohesion_radius || params.cohesionRadius) ?? 300.0,
    maxSpeed: validateOptionalNumber(params.max_speed || params.maxSpeed) ?? 500.0,
    maxForce: validateOptionalNumber(params.max_force || params.maxForce) ?? 100.0
  });
};

/**
 * Add flocking obstacle avoidance
 */
const handleAddFlockingObstacleAvoidance: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'add_flocking_obstacle_avoidance', {
    systemPath,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    lookAheadDistance: validateOptionalNumber(params.look_ahead_distance || params.lookAheadDistance) ?? 300.0,
    avoidanceWeight: validateOptionalNumber(params.avoidance_weight || params.avoidanceWeight) ?? 2.0,
    rayCount: validateOptionalNumber(params.ray_count || params.rayCount) ?? 8,
    collisionChannel: validateOptionalString(params.collision_channel || params.collisionChannel)
  });
};

/**
 * Set flocking target
 */
const handleSetFlockingTarget: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'set_flocking_target', {
    systemPath,
    targetType: validateOptionalString(params.target_type || params.targetType) ?? 'point',
    targetLocation: validateOptionalObject(params.target_location || params.targetLocation),
    targetActor: validateOptionalString(params.target_actor || params.targetActor),
    seekWeight: validateOptionalNumber(params.seek_weight || params.seekWeight) ?? 1.0,
    arrivalRadius: validateOptionalNumber(params.arrival_radius || params.arrivalRadius) ?? 200.0
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get advanced AI info
 */
const handleGetAdvancedAIInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetPath = validateRequiredString(params.asset_path || params.assetPath, 'asset_path');

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'get_advanced_ai_info', {
    assetPath,
    includeGoals: validateOptionalBoolean(params.include_goals || params.includeGoals) ?? true,
    includeActions: validateOptionalBoolean(params.include_actions || params.includeActions) ?? true,
    includeConsiderations: validateOptionalBoolean(params.include_considerations || params.includeConsiderations) ?? true
  });
};

/**
 * List advanced AI assets
 */
const handleListAdvancedAIAssets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.AI_ADVANCED, 'list_advanced_ai_assets', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    assetType: validateOptionalString(params.asset_type || params.assetType),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const advancedAIHandlers: ActionHandlerMap = {
  // GOAP
  create_goap_agent: handleCreateGOAPAgent,
  define_goap_goal: handleDefineGOAPGoal,
  create_goap_action: handleCreateGOAPAction,
  configure_goap_world_state: handleConfigureGOAPWorldState,

  // Utility AI
  create_utility_brain: handleCreateUtilityBrain,
  add_utility_action: handleAddUtilityAction,
  add_consideration: handleAddConsideration,
  configure_response_curve: handleConfigureResponseCurve,

  // Influence Maps
  create_influence_map: handleCreateInfluenceMap,
  add_influence_source: handleAddInfluenceSource,
  query_influence_map: handleQueryInfluenceMap,
  combine_influence_maps: handleCombineInfluenceMaps,

  // Flocking
  create_flocking_system: handleCreateFlockingSystem,
  configure_flocking_behavior: handleConfigureFlockingBehavior,
  add_flocking_obstacle_avoidance: handleAddFlockingObstacleAvoidance,
  set_flocking_target: handleSetFlockingTarget,

  // Query
  get_advanced_ai_info: handleGetAdvancedAIInfo,
  list_advanced_ai_assets: handleListAdvancedAIAssets
};

export const handleAdvancedAITools = createCategoryHandler(Forge2Categories.AI_ADVANCED, advancedAIHandlers);

export default advancedAIHandlers;
