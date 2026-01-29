/**
 * Genre-specific handlers for Battle Royale games (Fortnite, PUBG, Apex style)
 * Handles shrinking zones, loot spawns, drop systems, pings, and large-scale combat
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
// Local Validator Helpers
// ============================================================================

function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value.trim();
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

function validateRequiredEnum<T extends string>(value: unknown, validValues: readonly T[], fieldName: string): T {
  const strValue = validateRequiredString(value, fieldName);
  if (!validValues.includes(strValue as T)) {
    throw new Error(`${fieldName} must be one of: ${validValues.join(', ')}`);
  }
  return strValue as T;
}

function validateOptionalEnum<T extends string>(value: unknown, validValues: readonly T[]): T | undefined {
  if (value === undefined || value === null) return undefined;
  const strValue = String(value).trim();
  if (!validValues.includes(strValue as T)) return undefined;
  return strValue as T;
}

function validateOptionalArray(value: unknown): unknown[] | undefined {
  if (value === undefined || value === null) return undefined;
  if (Array.isArray(value)) return value;
  return undefined;
}

function validateOptionalObject(value: unknown): Record<string, unknown> | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value === 'object' && !Array.isArray(value)) return value as Record<string, unknown>;
  return undefined;
}

// ============================================================================
// Type Definitions
// ============================================================================

const ZONE_TYPES = ['circle', 'gas', 'storm', 'radiation', 'fire'] as const;
const LOOT_RARITIES = ['common', 'uncommon', 'rare', 'epic', 'legendary', 'mythic'] as const;
const CONTAINER_TYPES = ['chest', 'supply_drop', 'floor_loot', 'crate', 'vending_machine'] as const;
const DROP_METHODS = ['plane', 'drop_pod', 'parachute', 'glider', 'teleport'] as const;
const PING_TYPES = ['location', 'enemy', 'loot', 'danger', 'going_here', 'watching', 'defending', 'attacking', 'need_item'] as const;
const VEHICLE_TYPES = ['car', 'truck', 'boat', 'helicopter', 'atv', 'motorcycle'] as const;
const KNOCKDOWN_STATES = ['standing', 'downed', 'reviving', 'dead', 'spectating'] as const;
const MATCH_PHASES = ['lobby', 'warmup', 'dropping', 'early_game', 'mid_game', 'late_game', 'final_circle', 'victory'] as const;
const INVENTORY_SLOTS = ['weapon_1', 'weapon_2', 'weapon_3', 'tactical', 'ultimate', 'healing', 'utility'] as const;
const ATTACHMENT_TYPES = ['scope', 'barrel', 'stock', 'magazine', 'grip', 'hop_up'] as const;

// Reserve constants for future validation (type validation uses arrays directly)
void PING_TYPES;
void KNOCKDOWN_STATES;
void MATCH_PHASES;
void INVENTORY_SLOTS;
void ATTACHMENT_TYPES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create shrinking zone system
 */
const handleCreateShrinkingZone: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const zoneType = validateRequiredEnum(params.zone_type || params.zoneType, ZONE_TYPES, 'zone_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'create_shrinking_zone', {
    zoneType,
    initialRadius: validateOptionalNumber(params.initial_radius || params.initialRadius) ?? 5000,
    damagePerSecond: validateOptionalNumber(params.damage_per_second || params.damagePerSecond) ?? 1,
    damageScaling: validateOptionalNumber(params.damage_scaling || params.damageScaling) ?? 1.5,
    visualEffect: validateOptionalString(params.visual_effect || params.visualEffect),
    audioEffect: validateOptionalString(params.audio_effect || params.audioEffect),
    warningTime: validateOptionalNumber(params.warning_time || params.warningTime) ?? 60,
    showOnMinimap: validateOptionalBoolean(params.show_on_minimap || params.showOnMinimap) ?? true,
    showNextZone: validateOptionalBoolean(params.show_next_zone || params.showNextZone) ?? true
  });
};

/**
 * Configure zone phases
 */
const handleConfigureZonePhases: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'configure_zone_phases', {
    phases: validateOptionalArray(params.phases) ?? [
      { phase: 1, waitTime: 120, shrinkTime: 120, targetRadius: 3500, damage: 1 },
      { phase: 2, waitTime: 90, shrinkTime: 90, targetRadius: 2500, damage: 2 },
      { phase: 3, waitTime: 60, shrinkTime: 60, targetRadius: 1500, damage: 5 },
      { phase: 4, waitTime: 45, shrinkTime: 45, targetRadius: 800, damage: 8 },
      { phase: 5, waitTime: 30, shrinkTime: 30, targetRadius: 300, damage: 12 },
      { phase: 6, waitTime: 20, shrinkTime: 20, targetRadius: 50, damage: 20 }
    ],
    randomizeCenter: validateOptionalBoolean(params.randomize_center || params.randomizeCenter) ?? true,
    biasTowardsPoi: validateOptionalBoolean(params.bias_towards_poi || params.biasTowardsPoi) ?? true,
    healingPhaseEnabled: validateOptionalBoolean(params.healing_phase_enabled || params.healingPhaseEnabled) ?? false
  });
};

/**
 * Configure loot spawns
 */
const handleConfigureLootSpawns: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'configure_loot_spawns', {
    floorLootDensity: validateOptionalNumber(params.floor_loot_density || params.floorLootDensity) ?? 0.3,
    chestDensity: validateOptionalNumber(params.chest_density || params.chestDensity) ?? 0.5,
    supplyDropInterval: validateOptionalNumber(params.supply_drop_interval || params.supplyDropInterval) ?? 120,
    supplyDropCount: validateOptionalNumber(params.supply_drop_count || params.supplyDropCount) ?? 3,
    hotZoneEnabled: validateOptionalBoolean(params.hot_zone_enabled || params.hotZoneEnabled) ?? true,
    hotZoneLootMultiplier: validateOptionalNumber(params.hot_zone_loot_multiplier || params.hotZoneLootMultiplier) ?? 2.0,
    lootRefreshEnabled: validateOptionalBoolean(params.loot_refresh_enabled || params.lootRefreshEnabled) ?? false,
    lootRefreshInterval: validateOptionalNumber(params.loot_refresh_interval || params.lootRefreshInterval)
  });
};

/**
 * Create loot container type
 */
const handleCreateLootContainer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const containerId = validateRequiredString(params.container_id || params.containerId, 'container_id');
  const containerType = validateRequiredEnum(params.container_type || params.containerType, CONTAINER_TYPES, 'container_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'create_loot_container', {
    containerId,
    containerType,
    guaranteedItemCount: validateOptionalNumber(params.guaranteed_item_count || params.guaranteedItemCount) ?? 3,
    maxItemCount: validateOptionalNumber(params.max_item_count || params.maxItemCount) ?? 5,
    minRarity: validateOptionalEnum(params.min_rarity || params.minRarity, LOOT_RARITIES),
    maxRarity: validateOptionalEnum(params.max_rarity || params.maxRarity, LOOT_RARITIES),
    openTime: validateOptionalNumber(params.open_time || params.openTime) ?? 1.5,
    openSound: validateOptionalString(params.open_sound || params.openSound),
    glowColor: validateOptionalString(params.glow_color || params.glowColor),
    canBeDestroyed: validateOptionalBoolean(params.can_be_destroyed || params.canBeDestroyed) ?? true,
    spawnChance: validateOptionalNumber(params.spawn_chance || params.spawnChance) ?? 1.0,
    lootTable: validateOptionalString(params.loot_table || params.lootTable)
  });
};

/**
 * Configure loot rarity weights
 */
const handleConfigureRarityWeights: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'configure_rarity_weights', {
    common: validateOptionalNumber(params.common) ?? 40,
    uncommon: validateOptionalNumber(params.uncommon) ?? 30,
    rare: validateOptionalNumber(params.rare) ?? 18,
    epic: validateOptionalNumber(params.epic) ?? 9,
    legendary: validateOptionalNumber(params.legendary) ?? 2.5,
    mythic: validateOptionalNumber(params.mythic) ?? 0.5,
    hotZoneModifier: validateOptionalObject(params.hot_zone_modifier || params.hotZoneModifier),
    supplyDropModifier: validateOptionalObject(params.supply_drop_modifier || params.supplyDropModifier)
  });
};

/**
 * Setup drop system
 */
const handleSetupDropSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const dropMethod = validateRequiredEnum(params.drop_method || params.dropMethod, DROP_METHODS, 'drop_method');

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'setup_drop_system', {
    dropMethod,
    flightPathRandomization: validateOptionalNumber(params.flight_path_randomization || params.flightPathRandomization) ?? 0.3,
    dropAltitude: validateOptionalNumber(params.drop_altitude || params.dropAltitude) ?? 2000,
    dropSpeed: validateOptionalNumber(params.drop_speed || params.dropSpeed) ?? 50,
    freeDropDuration: validateOptionalNumber(params.free_drop_duration || params.freeDroppingDuration) ?? 5,
    glideSpeed: validateOptionalNumber(params.glide_speed || params.glideSpeed) ?? 25,
    landingCooldown: validateOptionalNumber(params.landing_cooldown || params.landingCooldown) ?? 0.5,
    autoDeployAltitude: validateOptionalNumber(params.auto_deploy_altitude || params.autoDeployAltitude) ?? 200,
    canCancelDeploy: validateOptionalBoolean(params.can_cancel_deploy || params.canCancelDeploy) ?? false,
    warmupDuration: validateOptionalNumber(params.warmup_duration || params.warmupDuration) ?? 60
  });
};

/**
 * Create ping system
 */
const handleCreatePingSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'create_ping_system', {
    pingDuration: validateOptionalNumber(params.ping_duration || params.pingDuration) ?? 8,
    maxPingsPerPlayer: validateOptionalNumber(params.max_pings_per_player || params.maxPingsPerPlayer) ?? 3,
    pingCooldown: validateOptionalNumber(params.ping_cooldown || params.pingCooldown) ?? 1,
    doubleTapEnemyPing: validateOptionalBoolean(params.double_tap_enemy_ping || params.doubleTapEnemyPing) ?? true,
    autoItemPing: validateOptionalBoolean(params.auto_item_ping || params.autoItemPing) ?? true,
    pingWheelEnabled: validateOptionalBoolean(params.ping_wheel_enabled || params.pingWheelEnabled) ?? true,
    voiceLineEnabled: validateOptionalBoolean(params.voice_line_enabled || params.voiceLineEnabled) ?? true,
    enemyPingTracking: validateOptionalBoolean(params.enemy_ping_tracking || params.enemyPingTracking) ?? false,
    pingThroughWalls: validateOptionalBoolean(params.ping_through_walls || params.pingThroughWalls) ?? false
  });
};

/**
 * Configure knockdown/DBNO system
 */
const handleConfigureKnockdown: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'configure_knockdown', {
    knockdownEnabled: validateOptionalBoolean(params.knockdown_enabled || params.knockdownEnabled) ?? true,
    bleedoutTime: validateOptionalNumber(params.bleedout_time || params.bleedoutTime) ?? 90,
    bleedoutSpeedInZone: validateOptionalNumber(params.bleedout_speed_in_zone || params.bleedoutSpeedInZone) ?? 2.0,
    reviveTime: validateOptionalNumber(params.revive_time || params.reviveTime) ?? 5,
    reviveCancelOnDamage: validateOptionalBoolean(params.revive_cancel_on_damage || params.reviveCancelOnDamage) ?? true,
    selfReviveEnabled: validateOptionalBoolean(params.self_revive_enabled || params.selfReviveEnabled) ?? false,
    selfReviveTime: validateOptionalNumber(params.self_revive_time || params.selfReviveTime) ?? 10,
    knockdownShield: validateOptionalBoolean(params.knockdown_shield || params.knockdownShield) ?? true,
    knockdownShieldHealth: validateOptionalNumber(params.knockdown_shield_health || params.knockdownShieldHealth) ?? 100,
    crawlSpeed: validateOptionalNumber(params.crawl_speed || params.crawlSpeed) ?? 100,
    finishDamage: validateOptionalNumber(params.finish_damage || params.finishDamage) ?? 100
  });
};

/**
 * Setup vehicle spawns
 */
const handleSetupVehicleSpawns: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'setup_vehicle_spawns', {
    vehicleSpawnChance: validateOptionalNumber(params.vehicle_spawn_chance || params.vehicleSpawnChance) ?? 0.7,
    vehicleTypes: validateOptionalArray(params.vehicle_types || params.vehicleTypes),
    fuelEnabled: validateOptionalBoolean(params.fuel_enabled || params.fuelEnabled) ?? true,
    vehicleDamageEnabled: validateOptionalBoolean(params.vehicle_damage_enabled || params.vehicleDamageEnabled) ?? true,
    explosionOnDestroy: validateOptionalBoolean(params.explosion_on_destroy || params.explosionOnDestroy) ?? true,
    roadSpawnBias: validateOptionalNumber(params.road_spawn_bias || params.roadSpawnBias) ?? 0.8,
    garageSpawnBias: validateOptionalNumber(params.garage_spawn_bias || params.garageSpawnBias) ?? 0.95
  });
};

/**
 * Create vehicle type
 */
const handleCreateVehicle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const vehicleId = validateRequiredString(params.vehicle_id || params.vehicleId, 'vehicle_id');
  const vehicleType = validateRequiredEnum(params.vehicle_type || params.vehicleType, VEHICLE_TYPES, 'vehicle_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'create_vehicle', {
    vehicleId,
    vehicleType,
    vehicleName: validateOptionalString(params.vehicle_name || params.vehicleName),
    maxHealth: validateOptionalNumber(params.max_health || params.maxHealth) ?? 1000,
    maxSpeed: validateOptionalNumber(params.max_speed || params.maxSpeed) ?? 120,
    acceleration: validateOptionalNumber(params.acceleration) ?? 15,
    seatCount: validateOptionalNumber(params.seat_count || params.seatCount) ?? 4,
    fuelCapacity: validateOptionalNumber(params.fuel_capacity || params.fuelCapacity) ?? 100,
    fuelConsumption: validateOptionalNumber(params.fuel_consumption || params.fuelConsumption) ?? 5,
    ramDamage: validateOptionalNumber(params.ram_damage || params.ramDamage) ?? 50,
    noiseLoudness: validateOptionalNumber(params.noise_loudness || params.noiseLoudness) ?? 1.0,
    canShootFrom: validateOptionalBoolean(params.can_shoot_from || params.canShootFrom) ?? true,
    hasBoost: validateOptionalBoolean(params.has_boost || params.hasBoost) ?? false
  });
};

/**
 * Create spectator mode
 */
const handleCreateSpectatorMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'create_spectator_mode', {
    deathCamDuration: validateOptionalNumber(params.death_cam_duration || params.deathCamDuration) ?? 5,
    followKillerEnabled: validateOptionalBoolean(params.follow_killer_enabled || params.followKillerEnabled) ?? true,
    freeCamEnabled: validateOptionalBoolean(params.free_cam_enabled || params.freeCamEnabled) ?? true,
    freeCamDelay: validateOptionalNumber(params.free_cam_delay || params.freeCamDelay) ?? 60,
    teamOnlySpectate: validateOptionalBoolean(params.team_only_spectate || params.teamOnlySpectate) ?? true,
    mapViewEnabled: validateOptionalBoolean(params.map_view_enabled || params.mapViewEnabled) ?? true,
    statsOverlay: validateOptionalBoolean(params.stats_overlay || params.statsOverlay) ?? true,
    xrayEnabled: validateOptionalBoolean(params.xray_enabled || params.xrayEnabled) ?? false
  });
};

/**
 * Configure inventory system
 */
const handleConfigureInventory: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'configure_inventory', {
    weaponSlots: validateOptionalNumber(params.weapon_slots || params.weaponSlots) ?? 2,
    backpackSizes: validateOptionalArray(params.backpack_sizes || params.backpackSizes) ?? [8, 10, 12, 14],
    stackSizes: validateOptionalObject(params.stack_sizes || params.stackSizes),
    armorSlots: validateOptionalBoolean(params.armor_slots || params.armorSlots) ?? true,
    helmetSlots: validateOptionalBoolean(params.helmet_slots || params.helmetSlots) ?? true,
    quickUseSlots: validateOptionalNumber(params.quick_use_slots || params.quickUseSlots) ?? 4,
    dropOnDeath: validateOptionalBoolean(params.drop_on_death || params.dropOnDeath) ?? true,
    deathBoxDuration: validateOptionalNumber(params.death_box_duration || params.deathBoxDuration) ?? 300,
    autoPickupEnabled: validateOptionalBoolean(params.auto_pickup_enabled || params.autoPickupEnabled) ?? true
  });
};

/**
 * Configure armor/shield system
 */
const handleConfigureArmorSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'configure_armor_system', {
    armorTiers: validateOptionalArray(params.armor_tiers || params.armorTiers) ?? [
      { tier: 1, color: 'white', health: 50 },
      { tier: 2, color: 'blue', health: 75 },
      { tier: 3, color: 'purple', health: 100 },
      { tier: 4, color: 'gold', health: 100, regen: true }
    ],
    helmetDamageReduction: validateOptionalArray(params.helmet_damage_reduction || params.helmetDamageReduction),
    armorDegrades: validateOptionalBoolean(params.armor_degrades || params.armorDegrades) ?? true,
    canRepairArmor: validateOptionalBoolean(params.can_repair_armor || params.canRepairArmor) ?? true,
    armorSwapTime: validateOptionalNumber(params.armor_swap_time || params.armorSwapTime) ?? 1.5,
    evoArmorEnabled: validateOptionalBoolean(params.evo_armor_enabled || params.evoArmorEnabled) ?? false,
    evoDamageThresholds: validateOptionalArray(params.evo_damage_thresholds || params.evoDamageThresholds)
  });
};

/**
 * Create weapon attachment system
 */
const handleCreateAttachmentSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'create_attachment_system', {
    scopeTypes: validateOptionalArray(params.scope_types || params.scopeTypes) ?? ['1x', '2x', '3x', '4x', '6x', '8x'],
    barrelTypes: validateOptionalArray(params.barrel_types || params.barrelTypes) ?? ['suppressor', 'compensator', 'flash_hider', 'extended_barrel'],
    stockTypes: validateOptionalArray(params.stock_types || params.stockTypes) ?? ['standard', 'tactical', 'sniper'],
    magazineTypes: validateOptionalArray(params.magazine_types || params.magazineTypes) ?? ['standard', 'extended', 'quick_draw', 'extended_quick_draw'],
    attachmentRarities: validateOptionalBoolean(params.attachment_rarities || params.attachmentRarities) ?? true,
    autoAttach: validateOptionalBoolean(params.auto_attach || params.autoAttach) ?? true,
    goldWeaponsFullyKitted: validateOptionalBoolean(params.gold_weapons_fully_kitted || params.goldWeaponsFullyKitted) ?? true
  });
};

/**
 * Configure match settings
 */
const handleConfigureMatchSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'configure_match_settings', {
    maxPlayers: validateOptionalNumber(params.max_players || params.maxPlayers) ?? 100,
    teamSize: validateOptionalNumber(params.team_size || params.teamSize) ?? 1,
    fillEnabled: validateOptionalBoolean(params.fill_enabled || params.fillEnabled) ?? true,
    respawnEnabled: validateOptionalBoolean(params.respawn_enabled || params.respawnEnabled) ?? false,
    respawnMethod: validateOptionalString(params.respawn_method || params.respawnMethod),
    respawnTokens: validateOptionalNumber(params.respawn_tokens || params.respawnTokens),
    gulagEnabled: validateOptionalBoolean(params.gulag_enabled || params.gulagEnabled) ?? false,
    rebootVanEnabled: validateOptionalBoolean(params.reboot_van_enabled || params.rebootVanEnabled) ?? false,
    matchDurationCap: validateOptionalNumber(params.match_duration_cap || params.matchDurationCap)
  });
};

/**
 * Get match state
 */
const handleGetMatchState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'get_match_state', {
    includeZoneInfo: validateOptionalBoolean(params.include_zone_info || params.includeZoneInfo) ?? true,
    includePlayerCount: validateOptionalBoolean(params.include_player_count || params.includePlayerCount) ?? true,
    includeTeamInfo: validateOptionalBoolean(params.include_team_info || params.includeTeamInfo) ?? true,
    includeKillFeed: validateOptionalBoolean(params.include_kill_feed || params.includeKillFeed) ?? false
  });
};

/**
 * Get player stats
 */
const handleGetPlayerStats: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const playerId = validateRequiredString(params.player_id || params.playerId, 'player_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'get_player_stats', {
    playerId,
    includeInventory: validateOptionalBoolean(params.include_inventory || params.includeInventory) ?? true,
    includePosition: validateOptionalBoolean(params.include_position || params.includePosition) ?? true,
    includeKills: validateOptionalBoolean(params.include_kills || params.includeKills) ?? true
  });
};

/**
 * Trigger supply drop
 */
const handleTriggerSupplyDrop: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'trigger_supply_drop', {
    location: validateOptionalObject(params.location),
    randomizeLocation: validateOptionalBoolean(params.randomize_location || params.randomizeLocation) ?? true,
    announceToAll: validateOptionalBoolean(params.announce_to_all || params.announceToAll) ?? true,
    lootTier: validateOptionalEnum(params.loot_tier || params.lootTier, LOOT_RARITIES),
    dropCount: validateOptionalNumber(params.drop_count || params.dropCount) ?? 1
  });
};

/**
 * List all vehicles
 */
const handleListVehicles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_BATTLE_ROYALE, 'list_vehicles', {
    filterType: validateOptionalEnum(params.filter_type || params.filterType, VEHICLE_TYPES),
    includeSpawned: validateOptionalBoolean(params.include_spawned || params.includeSpawned) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const genreBattleRoyaleHandlers: ActionHandlerMap = {
  // Zone system
  create_shrinking_zone: handleCreateShrinkingZone,
  configure_zone_phases: handleConfigureZonePhases,

  // Loot system
  configure_loot_spawns: handleConfigureLootSpawns,
  create_loot_container: handleCreateLootContainer,
  configure_rarity_weights: handleConfigureRarityWeights,

  // Drop system
  setup_drop_system: handleSetupDropSystem,

  // Communication
  create_ping_system: handleCreatePingSystem,

  // Combat
  configure_knockdown: handleConfigureKnockdown,

  // Vehicles
  setup_vehicle_spawns: handleSetupVehicleSpawns,
  create_vehicle: handleCreateVehicle,

  // Spectator
  create_spectator_mode: handleCreateSpectatorMode,

  // Inventory
  configure_inventory: handleConfigureInventory,
  configure_armor_system: handleConfigureArmorSystem,
  create_attachment_system: handleCreateAttachmentSystem,

  // Match
  configure_match_settings: handleConfigureMatchSettings,
  get_match_state: handleGetMatchState,
  get_player_stats: handleGetPlayerStats,
  trigger_supply_drop: handleTriggerSupplyDrop,
  list_vehicles: handleListVehicles
};

// Create the main handler function for this category
export const handleGenreBattleRoyaleTools = createCategoryHandler(Forge2Categories.GENRE_BATTLE_ROYALE, genreBattleRoyaleHandlers);

export default genreBattleRoyaleHandlers;
