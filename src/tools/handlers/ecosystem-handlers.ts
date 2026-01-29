/**
 * FORGE2: Ecosystem Handlers
 * Handles wildlife, animal behaviors, spawning, taming, and ecosystem simulation
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
// Animal Species Action Handlers
// ============================================================================

/**
 * Create animal species
 */
const handleCreateAnimalSpecies: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const speciesId = validateRequiredString(params.species_id || params.speciesId, 'species_id');
  const speciesName = validateRequiredString(params.species_name || params.speciesName, 'species_name');

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'create_animal_species', {
    speciesId,
    speciesName,
    meshPath: validateOptionalString(params.mesh_path || params.meshPath),
    animBlueprintPath: validateOptionalString(params.anim_blueprint_path || params.animBlueprintPath),
    behaviorType: validateOptionalString(params.behavior_type || params.behaviorType) ?? 'passive',
    dietType: validateOptionalString(params.diet_type || params.dietType) ?? 'herbivore',
    size: validateOptionalString(params.size) ?? 'medium',
    speed: validateOptionalNumber(params.speed) ?? 400.0,
    health: validateOptionalNumber(params.health) ?? 100.0,
    damage: validateOptionalNumber(params.damage) ?? 10.0
  });
};

/**
 * Configure animal behavior
 */
const handleConfigureAnimalBehavior: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const speciesId = validateRequiredString(params.species_id || params.speciesId, 'species_id');

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'configure_animal_behavior', {
    speciesId,
    fearLevel: validateOptionalNumber(params.fear_level || params.fearLevel) ?? 0.5,
    curiosityLevel: validateOptionalNumber(params.curiosity_level || params.curiosityLevel) ?? 0.3,
    aggressionLevel: validateOptionalNumber(params.aggression_level || params.aggressionLevel) ?? 0.1,
    fleeDistance: validateOptionalNumber(params.flee_distance || params.fleeDistance) ?? 1000.0,
    attackDistance: validateOptionalNumber(params.attack_distance || params.attackDistance) ?? 200.0,
    roamRadius: validateOptionalNumber(params.roam_radius || params.roamRadius) ?? 2000.0,
    restHours: validateOptionalArray(params.rest_hours || params.restHours) ?? [22, 23, 0, 1, 2, 3, 4, 5],
    nocturnalBehavior: validateOptionalBoolean(params.nocturnal_behavior || params.nocturnalBehavior) ?? false
  });
};

/**
 * Configure animal habitat
 */
const handleConfigureAnimalHabitat: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const speciesId = validateRequiredString(params.species_id || params.speciesId, 'species_id');

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'configure_animal_habitat', {
    speciesId,
    preferredBiomes: validateOptionalArray(params.preferred_biomes || params.preferredBiomes) ?? ['forest'],
    avoidedBiomes: validateOptionalArray(params.avoided_biomes || params.avoidedBiomes) ?? [],
    elevationRange: validateOptionalObject(params.elevation_range || params.elevationRange) ?? { min: 0, max: 5000 },
    waterProximity: validateOptionalBoolean(params.water_proximity || params.waterProximity) ?? false,
    shelterRequired: validateOptionalBoolean(params.shelter_required || params.shelterRequired) ?? false
  });
};

// ============================================================================
// Herd/Flock Behavior Action Handlers
// ============================================================================

/**
 * Configure herd behavior
 */
const handleConfigureHerdBehavior: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const speciesId = validateRequiredString(params.species_id || params.speciesId, 'species_id');

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'configure_herd_behavior', {
    speciesId,
    herdsEnabled: validateOptionalBoolean(params.herds_enabled || params.herdsEnabled) ?? true,
    minHerdSize: validateOptionalNumber(params.min_herd_size || params.minHerdSize) ?? 3,
    maxHerdSize: validateOptionalNumber(params.max_herd_size || params.maxHerdSize) ?? 12,
    herdSpacing: validateOptionalNumber(params.herd_spacing || params.herdSpacing) ?? 300.0,
    leaderFollowing: validateOptionalBoolean(params.leader_following || params.leaderFollowing) ?? true,
    flockingEnabled: validateOptionalBoolean(params.flocking_enabled || params.flockingEnabled) ?? true
  });
};

/**
 * Setup migration patterns
 */
const handleSetupMigrationPatterns: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const speciesId = validateRequiredString(params.species_id || params.speciesId, 'species_id');

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'setup_migration_patterns', {
    speciesId,
    migrationEnabled: validateOptionalBoolean(params.migration_enabled || params.migrationEnabled) ?? false,
    migrationRoutes: validateOptionalArray(params.migration_routes || params.migrationRoutes) ?? [],
    seasonalMigration: validateOptionalBoolean(params.seasonal_migration || params.seasonalMigration) ?? true,
    migrationSeasons: validateOptionalArray(params.migration_seasons || params.migrationSeasons) ?? ['spring', 'autumn'],
    migrationSpeed: validateOptionalNumber(params.migration_speed || params.migrationSpeed) ?? 1.0
  });
};

// ============================================================================
// Predator/Prey Action Handlers
// ============================================================================

/**
 * Setup predator prey relationships
 */
const handleSetupPredatorPreyRelationships: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'setup_predator_prey_relationships', {
    predatorId: validateRequiredString(params.predator_id || params.predatorId, 'predator_id'),
    preyIds: validateOptionalArray(params.prey_ids || params.preyIds) ?? [],
    huntingBehavior: validateOptionalString(params.hunting_behavior || params.huntingBehavior) ?? 'stalk',
    huntSuccessRate: validateOptionalNumber(params.hunt_success_rate || params.huntSuccessRate) ?? 0.5,
    packHunting: validateOptionalBoolean(params.pack_hunting || params.packHunting) ?? false,
    territorialRange: validateOptionalNumber(params.territorial_range || params.territorialRange) ?? 5000.0
  });
};

// ============================================================================
// Spawning Action Handlers
// ============================================================================

/**
 * Create spawning system
 */
const handleCreateSpawningSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'create_spawning_system', {
    maxWildlife: validateOptionalNumber(params.max_wildlife || params.maxWildlife) ?? 100,
    spawnRadius: validateOptionalNumber(params.spawn_radius || params.spawnRadius) ?? 5000.0,
    despawnRadius: validateOptionalNumber(params.despawn_radius || params.despawnRadius) ?? 8000.0,
    respawnTime: validateOptionalNumber(params.respawn_time || params.respawnTime) ?? 300.0,
    densityControl: validateOptionalBoolean(params.density_control || params.densityControl) ?? true,
    biomeAwareSpawning: validateOptionalBoolean(params.biome_aware_spawning || params.biomeAwareSpawning) ?? true
  });
};

/**
 * Add spawn rule
 */
const handleAddSpawnRule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const speciesId = validateRequiredString(params.species_id || params.speciesId, 'species_id');

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'add_spawn_rule', {
    speciesId,
    spawnWeight: validateOptionalNumber(params.spawn_weight || params.spawnWeight) ?? 1.0,
    minLevel: validateOptionalNumber(params.min_level || params.minLevel),
    maxLevel: validateOptionalNumber(params.max_level || params.maxLevel),
    timeOfDay: validateOptionalArray(params.time_of_day || params.timeOfDay),
    weatherConditions: validateOptionalArray(params.weather_conditions || params.weatherConditions),
    biomeRestrictions: validateOptionalArray(params.biome_restrictions || params.biomeRestrictions)
  });
};

// ============================================================================
// Taming Action Handlers
// ============================================================================

/**
 * Setup taming system
 */
const handleSetupTamingSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'setup_taming_system', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    tamingMethod: validateOptionalString(params.taming_method || params.tamingMethod) ?? 'passive',
    maxTamedAnimals: validateOptionalNumber(params.max_tamed_animals || params.maxTamedAnimals) ?? 5,
    loyaltySystem: validateOptionalBoolean(params.loyalty_system || params.loyaltySystem) ?? true,
    feedingRequired: validateOptionalBoolean(params.feeding_required || params.feedingRequired) ?? true
  });
};

/**
 * Configure taming for species
 */
const handleConfigureTamingForSpecies: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const speciesId = validateRequiredString(params.species_id || params.speciesId, 'species_id');

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'configure_taming_for_species', {
    speciesId,
    tamable: validateOptionalBoolean(params.tamable) ?? true,
    tamingDifficulty: validateOptionalNumber(params.taming_difficulty || params.tamingDifficulty) ?? 1.0,
    preferredFood: validateOptionalArray(params.preferred_food || params.preferredFood) ?? [],
    tamingTime: validateOptionalNumber(params.taming_time || params.tamingTime) ?? 60.0,
    rideable: validateOptionalBoolean(params.rideable) ?? false,
    combatCapable: validateOptionalBoolean(params.combat_capable || params.combatCapable) ?? false,
    carryCapacity: validateOptionalNumber(params.carry_capacity || params.carryCapacity)
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get ecosystem info
 */
const handleGetEcosystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'get_ecosystem_info', {
    includeSpecies: validateOptionalBoolean(params.include_species || params.includeSpecies) ?? true,
    includePopulation: validateOptionalBoolean(params.include_population || params.includePopulation) ?? true,
    includeRelationships: validateOptionalBoolean(params.include_relationships || params.includeRelationships) ?? false
  });
};

/**
 * List species
 */
const handleListSpecies: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECOSYSTEM, 'list_species', {
    behaviorType: validateOptionalString(params.behavior_type || params.behaviorType),
    dietType: validateOptionalString(params.diet_type || params.dietType),
    tamableOnly: validateOptionalBoolean(params.tamable_only || params.tamableOnly) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const ecosystemHandlers: ActionHandlerMap = {
  // Species
  create_animal_species: handleCreateAnimalSpecies,
  configure_animal_behavior: handleConfigureAnimalBehavior,
  configure_animal_habitat: handleConfigureAnimalHabitat,

  // Herd/Flock
  configure_herd_behavior: handleConfigureHerdBehavior,
  setup_migration_patterns: handleSetupMigrationPatterns,

  // Predator/Prey
  setup_predator_prey_relationships: handleSetupPredatorPreyRelationships,

  // Spawning
  create_spawning_system: handleCreateSpawningSystem,
  add_spawn_rule: handleAddSpawnRule,

  // Taming
  setup_taming_system: handleSetupTamingSystem,
  configure_taming_for_species: handleConfigureTamingForSpecies,

  // Query
  get_ecosystem_info: handleGetEcosystemInfo,
  list_species: handleListSpecies
};

export const handleEcosystemTools = createCategoryHandler(Forge2Categories.ECOSYSTEM, ecosystemHandlers);

export default ecosystemHandlers;
