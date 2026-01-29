/**
 * FORGE2: Survival Genre Handlers
 * Complete survival game mechanics (Rust, Valheim, The Forest style)
 *
 * Supports: Needs systems (hunger, thirst, temperature)
 * Systems: Harvesting, building, crafting tiers, decay, wildlife, weather
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

export type NeedType = 'hunger' | 'thirst' | 'sleep' | 'temperature' | 'stamina' | 'sanity' | 'oxygen';
export type ResourceType = 'wood' | 'stone' | 'metal' | 'fiber' | 'food' | 'water' | 'fuel' | 'custom';
export type BuildingPieceType = 'foundation' | 'wall' | 'floor' | 'ceiling' | 'roof' | 'door' | 'window' | 'stairs' | 'fence' | 'pillar';
export type CraftingTier = 'hand' | 'workbench' | 'forge' | 'chemistry' | 'electronics' | 'advanced';
export type WildlifeType = 'passive' | 'defensive' | 'aggressive' | 'predator' | 'tameable';
export type WeatherType = 'clear' | 'rain' | 'storm' | 'snow' | 'heat' | 'fog' | 'sandstorm';

// ============================================================================
// Constants
// ============================================================================

const NEED_TYPES = ['hunger', 'thirst', 'sleep', 'temperature', 'stamina', 'sanity', 'oxygen'] as const;
const RESOURCE_TYPES = ['wood', 'stone', 'metal', 'fiber', 'food', 'water', 'fuel', 'custom'] as const;
const BUILDING_PIECE_TYPES = ['foundation', 'wall', 'floor', 'ceiling', 'roof', 'door', 'window', 'stairs', 'fence', 'pillar'] as const;
const CRAFTING_TIERS = ['hand', 'workbench', 'forge', 'chemistry', 'electronics', 'advanced'] as const;
const WILDLIFE_TYPES = ['passive', 'defensive', 'aggressive', 'predator', 'tameable'] as const;
const WEATHER_TYPES = ['clear', 'rain', 'storm', 'snow', 'heat', 'fog', 'sandstorm'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create needs system
 */
const handleCreateNeedsSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'create_needs_system', {
    enabledNeeds: params.enabled_needs || params.enabledNeeds || ['hunger', 'thirst'],
    tickRate: params.tick_rate ?? params.tickRate ?? 1.0,
    deathOnZero: params.death_on_zero ?? params.deathOnZero ?? true,
    penaltiesBeforeDeath: params.penalties_before_death ?? params.penaltiesBeforeDeath ?? true,
    showHUD: params.show_hud ?? params.showHUD ?? true
  });
};

/**
 * Configure need
 */
const handleConfigureNeed: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const needType = validateRequiredEnum(
    params.need_type || params.needType,
    NEED_TYPES,
    'need_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'configure_need', {
    needType,
    maxValue: params.max_value ?? params.maxValue ?? 100,
    startValue: params.start_value ?? params.startValue ?? 100,
    decayRate: params.decay_rate ?? params.decayRate ?? 0.1,
    decayMultiplierOnActivity: params.decay_multiplier_on_activity ?? params.decayMultiplierOnActivity ?? 1.5,
    warningThreshold: params.warning_threshold ?? params.warningThreshold ?? 30,
    criticalThreshold: params.critical_threshold ?? params.criticalThreshold ?? 10,
    penaltyEffects: params.penalty_effects || params.penaltyEffects,
    iconPath: params.icon_path || params.iconPath
  });
};

/**
 * Configure temperature system
 */
const handleConfigureTemperature: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'configure_temperature', {
    enabled: params.enabled ?? true,
    minTemperature: params.min_temperature ?? params.minTemperature ?? -40,
    maxTemperature: params.max_temperature ?? params.maxTemperature ?? 50,
    comfortMin: params.comfort_min ?? params.comfortMin ?? 15,
    comfortMax: params.comfort_max ?? params.comfortMax ?? 30,
    hypothermiaRate: params.hypothermia_rate ?? params.hypothermiaRate ?? 1,
    hyperthermiaRate: params.hyperthermia_rate ?? params.hyperthermiaRate ?? 1,
    clothingEffect: params.clothing_effect ?? params.clothingEffect ?? true,
    shelterEffect: params.shelter_effect ?? params.shelterEffect ?? true,
    fireEffect: params.fire_effect ?? params.fireEffect ?? true,
    waterCooling: params.water_cooling ?? params.waterCooling ?? true
  });
};

/**
 * Configure harvesting
 */
const handleConfigureHarvesting: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'configure_harvesting', {
    toolRequired: params.tool_required ?? params.toolRequired ?? true,
    toolEfficiency: params.tool_efficiency ?? params.toolEfficiency ?? true,
    harvestTime: params.harvest_time ?? params.harvestTime ?? 2.0,
    harvestAnimation: params.harvest_animation || params.harvestAnimation,
    gatheringXP: params.gathering_xp ?? params.gatheringXP ?? true,
    respawnEnabled: params.respawn_enabled ?? params.respawnEnabled ?? true,
    defaultRespawnTime: params.default_respawn_time ?? params.defaultRespawnTime ?? 300
  });
};

/**
 * Create harvestable resource
 */
const handleCreateHarvestableResource: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const resourceId = validateRequiredString(params.resource_id || params.resourceId, 'resource_id');

  const resourceType = validateRequiredEnum(
    params.resource_type || params.resourceType,
    RESOURCE_TYPES,
    'resource_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'create_harvestable_resource', {
    resourceId,
    resourceType,
    resourceName: params.resource_name || params.resourceName,
    meshPath: params.mesh_path || params.meshPath,
    health: params.health ?? 100,
    harvestAmount: params.harvest_amount ?? params.harvestAmount ?? 10,
    harvestPerHit: params.harvest_per_hit ?? params.harvestPerHit ?? 1,
    requiredTool: params.required_tool || params.requiredTool,
    bonusTool: params.bonus_tool || params.bonusTool,
    bonusMultiplier: params.bonus_multiplier ?? params.bonusMultiplier ?? 2.0,
    respawnTime: params.respawn_time ?? params.respawnTime ?? 300,
    harvestSound: params.harvest_sound || params.harvestSound,
    harvestVFX: params.harvest_vfx || params.harvestVFX
  });
};

/**
 * Setup building system
 */
const handleSetupBuildingSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'setup_building_system', {
    snapEnabled: params.snap_enabled ?? params.snapEnabled ?? true,
    snapDistance: params.snap_distance ?? params.snapDistance ?? 50,
    rotationSnap: params.rotation_snap ?? params.rotationSnap ?? 45,
    requireFoundation: params.require_foundation ?? params.requireFoundation ?? true,
    requireSupport: params.require_support ?? params.requireSupport ?? true,
    maxBuildDistance: params.max_build_distance ?? params.maxBuildDistance ?? 500,
    ghostPreview: params.ghost_preview ?? params.ghostPreview ?? true,
    instantBuild: params.instant_build ?? params.instantBuild ?? true,
    buildTime: params.build_time ?? params.buildTime ?? 0,
    upgradeEnabled: params.upgrade_enabled ?? params.upgradeEnabled ?? true
  });
};

/**
 * Create building piece
 */
const handleCreateBuildingPiece: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const pieceId = validateRequiredString(params.piece_id || params.pieceId, 'piece_id');

  const pieceType = validateRequiredEnum(
    params.piece_type || params.pieceType,
    BUILDING_PIECE_TYPES,
    'piece_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'create_building_piece', {
    pieceId,
    pieceType,
    pieceName: params.piece_name || params.pieceName,
    meshPath: params.mesh_path || params.meshPath,
    buildCost: params.build_cost || params.buildCost,
    health: params.health ?? 500,
    materialTier: params.material_tier ?? params.materialTier ?? 1,
    snapPoints: params.snap_points || params.snapPoints,
    supportValue: params.support_value ?? params.supportValue ?? 0,
    stabilityRequired: params.stability_required ?? params.stabilityRequired ?? 0,
    weatherProtection: params.weather_protection ?? params.weatherProtection ?? 0,
    insulation: params.insulation ?? 0
  });
};

/**
 * Configure decay system
 */
const handleConfigureDecaySystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'configure_decay_system', {
    enabled: params.enabled ?? true,
    decayRate: params.decay_rate ?? params.decayRate ?? 0.01,
    decayInterval: params.decay_interval ?? params.decayInterval ?? 3600,
    requireToolCupboard: params.require_tool_cupboard ?? params.requireToolCupboard ?? false,
    toolCupboardRadius: params.tool_cupboard_radius ?? params.toolCupboardRadius ?? 5000,
    weatherAcceleration: params.weather_acceleration ?? params.weatherAcceleration ?? 2.0,
    repairEnabled: params.repair_enabled ?? params.repairEnabled ?? true,
    repairCostMultiplier: params.repair_cost_multiplier ?? params.repairCostMultiplier ?? 0.5
  });
};

/**
 * Create crafting tier
 */
const handleCreateCraftingTier: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const tier = validateRequiredEnum(
    params.tier,
    CRAFTING_TIERS,
    'tier'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'create_crafting_tier', {
    tier,
    stationName: params.station_name || params.stationName,
    stationMesh: params.station_mesh || params.stationMesh,
    buildCost: params.build_cost || params.buildCost,
    craftingSpeedMultiplier: params.crafting_speed_multiplier ?? params.craftingSpeedMultiplier ?? 1.0,
    fuelRequired: params.fuel_required ?? params.fuelRequired ?? false,
    fuelConsumption: params.fuel_consumption ?? params.fuelConsumption ?? 1,
    maxQueue: params.max_queue ?? params.maxQueue ?? 5,
    unlockedRecipes: params.unlocked_recipes || params.unlockedRecipes
  });
};

/**
 * Create survival crafting recipe
 */
const handleCreateSurvivalRecipe: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const recipeId = validateRequiredString(params.recipe_id || params.recipeId, 'recipe_id');

  const requiredTier = validateOptionalEnum(
    params.required_tier || params.requiredTier,
    CRAFTING_TIERS,
    'required_tier'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'create_survival_recipe', {
    recipeId,
    outputItem: params.output_item || params.outputItem,
    outputCount: params.output_count ?? params.outputCount ?? 1,
    ingredients: params.ingredients,
    requiredTier: requiredTier || 'hand',
    craftTime: params.craft_time ?? params.craftTime ?? 5,
    requiresBlueprint: params.requires_blueprint ?? params.requiresBlueprint ?? false,
    xpReward: params.xp_reward ?? params.xpReward ?? 10
  });
};

/**
 * Setup wildlife
 */
const handleSetupWildlife: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const wildlifeId = validateRequiredString(params.wildlife_id || params.wildlifeId, 'wildlife_id');

  const wildlifeType = validateRequiredEnum(
    params.wildlife_type || params.wildlifeType,
    WILDLIFE_TYPES,
    'wildlife_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'setup_wildlife', {
    wildlifeId,
    wildlifeType,
    name: params.name as string | undefined,
    meshPath: params.mesh_path || params.meshPath,
    health: params.health ?? 100,
    damage: params.damage ?? 10,
    speed: params.speed ?? 400,
    aggroRange: params.aggro_range ?? params.aggroRange ?? 1000,
    fleeHealth: params.flee_health ?? params.fleeHealth ?? 0.2,
    lootTable: params.loot_table || params.lootTable,
    spawnBiomes: params.spawn_biomes || params.spawnBiomes,
    activeTimeOfDay: params.active_time_of_day || params.activeTimeOfDay
  });
};

/**
 * Configure taming system
 */
const handleConfigureTamingSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'configure_taming_system', {
    enabled: params.enabled ?? true,
    tamingMethod: params.taming_method || params.tamingMethod || 'passive',
    tamingTime: params.taming_time ?? params.tamingTime ?? 60,
    requiredFood: params.required_food || params.requiredFood,
    maxTamedCreatures: params.max_tamed_creatures ?? params.maxTamedCreatures ?? 10,
    loyaltyDecay: params.loyalty_decay ?? params.loyaltyDecay ?? 0,
    breedingEnabled: params.breeding_enabled ?? params.breedingEnabled ?? false,
    ridingEnabled: params.riding_enabled ?? params.ridingEnabled ?? false
  });
};

/**
 * Configure weather effects
 */
const handleConfigureWeatherEffects: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const weatherType = validateRequiredEnum(
    params.weather_type || params.weatherType,
    WEATHER_TYPES,
    'weather_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'configure_weather_effects', {
    weatherType,
    temperatureModifier: params.temperature_modifier ?? params.temperatureModifier ?? 0,
    visibilityMultiplier: params.visibility_multiplier ?? params.visibilityMultiplier ?? 1.0,
    moveSpeedMultiplier: params.move_speed_multiplier ?? params.moveSpeedMultiplier ?? 1.0,
    wetEffect: params.wet_effect ?? params.wetEffect ?? false,
    wetDuration: params.wet_duration ?? params.wetDuration ?? 60,
    damagePerSecond: params.damage_per_second ?? params.damagePerSecond ?? 0,
    shelterProtection: params.shelter_protection ?? params.shelterProtection ?? true,
    decayMultiplier: params.decay_multiplier ?? params.decayMultiplier ?? 1.0
  });
};

/**
 * Create storage container
 */
const handleCreateStorageContainer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const containerId = validateRequiredString(params.container_id || params.containerId, 'container_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'create_storage_container', {
    containerId,
    containerName: params.container_name || params.containerName,
    meshPath: params.mesh_path || params.meshPath,
    slotCount: params.slot_count ?? params.slotCount ?? 30,
    maxStackSize: params.max_stack_size ?? params.maxStackSize ?? 100,
    filterItems: params.filter_items || params.filterItems,
    preservesFood: params.preserves_food ?? params.preservesFood ?? false,
    requiresLock: params.requires_lock ?? params.requiresLock ?? false,
    buildCost: params.build_cost || params.buildCost
  });
};

/**
 * Get player status
 */
const handleGetPlayerStatus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'get_player_status', {
    playerId: params.player_id || params.playerId,
    includeNeeds: params.include_needs ?? params.includeNeeds ?? true,
    includeStats: params.include_stats ?? params.includeStats ?? true,
    includeEffects: params.include_effects ?? params.includeEffects ?? true
  });
};

/**
 * Modify player need
 */
const handleModifyPlayerNeed: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const needType = validateRequiredEnum(
    params.need_type || params.needType,
    NEED_TYPES,
    'need_type'
  );

  const amount = params.amount;
  if (typeof amount !== 'number') {
    throw new Error('amount is required and must be a number');
  }

  return executeForge2Action(tools, Forge2Categories.GENRE_SURVIVAL, 'modify_player_need', {
    needType,
    amount,
    playerId: params.player_id || params.playerId,
    isPercentage: params.is_percentage ?? params.isPercentage ?? false
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const genreSurvivalHandlers: ActionHandlerMap = {
  create_needs_system: handleCreateNeedsSystem,
  configure_need: handleConfigureNeed,
  configure_temperature: handleConfigureTemperature,
  configure_harvesting: handleConfigureHarvesting,
  create_harvestable_resource: handleCreateHarvestableResource,
  setup_building_system: handleSetupBuildingSystem,
  create_building_piece: handleCreateBuildingPiece,
  configure_decay_system: handleConfigureDecaySystem,
  create_crafting_tier: handleCreateCraftingTier,
  create_survival_recipe: handleCreateSurvivalRecipe,
  setup_wildlife: handleSetupWildlife,
  configure_taming_system: handleConfigureTamingSystem,
  configure_weather_effects: handleConfigureWeatherEffects,
  create_storage_container: handleCreateStorageContainer,
  get_player_status: handleGetPlayerStatus,
  modify_player_need: handleModifyPlayerNeed
};

/**
 * Main genre survival handler - routes to specific action handlers
 */
export const handleGenreSurvivalTools = createCategoryHandler(Forge2Categories.GENRE_SURVIVAL, genreSurvivalHandlers);

/**
 * Get available genre survival actions
 */
export function getGenreSurvivalActions(): string[] {
  return Object.keys(genreSurvivalHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeGenreSurvivalHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = genreSurvivalHandlers[action];
  if (!handler) {
    throw new Error(`Unknown genre survival action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
