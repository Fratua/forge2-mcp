/**
 * Genre-specific handlers for MOBA games (League of Legends, Dota, Smite style)
 * Handles heroes, lanes, minions, towers, jungle, items, and team objectives
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

const HERO_ROLES = ['tank', 'fighter', 'assassin', 'mage', 'marksman', 'support'] as const;
const ABILITY_TYPES = ['passive', 'basic', 'ultimate', 'innate'] as const;
const DAMAGE_TYPES = ['physical', 'magical', 'true', 'pure'] as const;
const LANE_TYPES = ['top', 'mid', 'bot', 'jungle'] as const;
const MINION_TYPES = ['melee', 'caster', 'siege', 'super'] as const;
const TOWER_TYPES = ['outer', 'inner', 'inhibitor', 'nexus'] as const;
const JUNGLE_CAMP_TYPES = ['small', 'medium', 'large', 'epic'] as const;
const BUFF_TYPES = ['red', 'blue', 'baron', 'dragon', 'elder', 'herald'] as const;
const ITEM_CATEGORIES = ['starter', 'basic', 'epic', 'legendary', 'mythic', 'boots', 'consumable'] as const;
const OBJECTIVE_TYPES = ['tower', 'inhibitor', 'nexus', 'baron', 'dragon', 'herald', 'elder'] as const;
const SCALING_STATS = ['ad', 'ap', 'health', 'armor', 'mr', 'attack_speed', 'crit', 'lethality'] as const;

// Reserve constants for future validation (type validation uses arrays directly)
void LANE_TYPES;
void SCALING_STATS;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a new hero/champion
 */
const handleCreateHero: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const heroId = validateRequiredString(params.hero_id || params.heroId, 'hero_id');
  const heroName = validateRequiredString(params.hero_name || params.heroName, 'hero_name');
  const primaryRole = validateRequiredEnum(params.primary_role || params.primaryRole, HERO_ROLES, 'primary_role');

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'create_hero', {
    heroId,
    heroName,
    primaryRole,
    secondaryRole: validateOptionalEnum(params.secondary_role || params.secondaryRole, HERO_ROLES),
    baseHealth: validateOptionalNumber(params.base_health || params.baseHealth) ?? 580,
    healthPerLevel: validateOptionalNumber(params.health_per_level || params.healthPerLevel) ?? 95,
    baseMana: validateOptionalNumber(params.base_mana || params.baseMana) ?? 300,
    manaPerLevel: validateOptionalNumber(params.mana_per_level || params.manaPerLevel) ?? 40,
    baseAd: validateOptionalNumber(params.base_ad || params.baseAd) ?? 60,
    adPerLevel: validateOptionalNumber(params.ad_per_level || params.adPerLevel) ?? 3,
    baseArmor: validateOptionalNumber(params.base_armor || params.baseArmor) ?? 30,
    baseMr: validateOptionalNumber(params.base_mr || params.baseMr) ?? 30,
    attackRange: validateOptionalNumber(params.attack_range || params.attackRange) ?? 125,
    movementSpeed: validateOptionalNumber(params.movement_speed || params.movementSpeed) ?? 335,
    attackSpeed: validateOptionalNumber(params.attack_speed || params.attackSpeed) ?? 0.625,
    resourceType: validateOptionalString(params.resource_type || params.resourceType) ?? 'mana',
    description: validateOptionalString(params.description),
    lore: validateOptionalString(params.lore)
  });
};

/**
 * Create an ability for a hero
 */
const handleCreateAbility: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const heroId = validateRequiredString(params.hero_id || params.heroId, 'hero_id');
  const abilityId = validateRequiredString(params.ability_id || params.abilityId, 'ability_id');
  const abilityName = validateRequiredString(params.ability_name || params.abilityName, 'ability_name');
  const abilityType = validateRequiredEnum(params.ability_type || params.abilityType, ABILITY_TYPES, 'ability_type');
  const slot = validateRequiredString(params.slot, 'slot'); // Q, W, E, R, Passive

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'create_ability', {
    heroId,
    abilityId,
    abilityName,
    abilityType,
    slot,
    damageType: validateOptionalEnum(params.damage_type || params.damageType, DAMAGE_TYPES),
    baseDamage: validateOptionalArray(params.base_damage || params.baseDamage), // Per rank
    scalingRatios: validateOptionalObject(params.scaling_ratios || params.scalingRatios),
    manaCost: validateOptionalArray(params.mana_cost || params.manaCost),
    cooldown: validateOptionalArray(params.cooldown),
    range: validateOptionalNumber(params.range),
    radius: validateOptionalNumber(params.radius),
    castTime: validateOptionalNumber(params.cast_time || params.castTime),
    maxRanks: validateOptionalNumber(params.max_ranks || params.maxRanks) ?? 5,
    description: validateOptionalString(params.description),
    isSkillshot: validateOptionalBoolean(params.is_skillshot || params.isSkillshot),
    isTargeted: validateOptionalBoolean(params.is_targeted || params.isTargeted),
    isAoe: validateOptionalBoolean(params.is_aoe || params.isAoe),
    ccType: validateOptionalString(params.cc_type || params.ccType),
    ccDuration: validateOptionalNumber(params.cc_duration || params.ccDuration)
  });
};

/**
 * Configure lane system
 */
const handleConfigureLaneSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'configure_lane_system', {
    laneCount: validateOptionalNumber(params.lane_count || params.laneCount) ?? 3,
    laneWidth: validateOptionalNumber(params.lane_width || params.laneWidth) ?? 1500,
    jungleEnabled: validateOptionalBoolean(params.jungle_enabled || params.jungleEnabled) ?? true,
    riverEnabled: validateOptionalBoolean(params.river_enabled || params.riverEnabled) ?? true,
    brushCount: validateOptionalNumber(params.brush_count || params.brushCount) ?? 20,
    brushVisionBlock: validateOptionalBoolean(params.brush_vision_block || params.brushVisionBlock) ?? true
  });
};

/**
 * Setup minion waves
 */
const handleSetupMinionWaves: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'setup_minion_waves', {
    spawnInterval: validateOptionalNumber(params.spawn_interval || params.spawnInterval) ?? 30,
    firstWaveTime: validateOptionalNumber(params.first_wave_time || params.firstWaveTime) ?? 90,
    meleeCount: validateOptionalNumber(params.melee_count || params.meleeCount) ?? 3,
    casterCount: validateOptionalNumber(params.caster_count || params.casterCount) ?? 3,
    siegeInterval: validateOptionalNumber(params.siege_interval || params.siegeInterval) ?? 3, // Every N waves
    superMinionTrigger: validateOptionalString(params.super_minion_trigger || params.superMinionTrigger) ?? 'inhibitor_destroyed',
    minionGoldBase: validateOptionalNumber(params.minion_gold_base || params.minionGoldBase) ?? 21,
    minionGoldScaling: validateOptionalNumber(params.minion_gold_scaling || params.minionGoldScaling) ?? 0.125,
    minionXpBase: validateOptionalNumber(params.minion_xp_base || params.minionXpBase) ?? 60
  });
};

/**
 * Configure minion type stats
 */
const handleConfigureMinionType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const minionType = validateRequiredEnum(params.minion_type || params.minionType, MINION_TYPES, 'minion_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'configure_minion_type', {
    minionType,
    baseHealth: validateOptionalNumber(params.base_health || params.baseHealth),
    healthScaling: validateOptionalNumber(params.health_scaling || params.healthScaling),
    baseDamage: validateOptionalNumber(params.base_damage || params.baseDamage),
    damageScaling: validateOptionalNumber(params.damage_scaling || params.damageScaling),
    armor: validateOptionalNumber(params.armor),
    magicResist: validateOptionalNumber(params.magic_resist || params.magicResist),
    goldReward: validateOptionalNumber(params.gold_reward || params.goldReward),
    xpReward: validateOptionalNumber(params.xp_reward || params.xpReward),
    attackRange: validateOptionalNumber(params.attack_range || params.attackRange),
    movementSpeed: validateOptionalNumber(params.movement_speed || params.movementSpeed)
  });
};

/**
 * Create tower system
 */
const handleCreateTowerSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'create_tower_system', {
    towersPerLane: validateOptionalNumber(params.towers_per_lane || params.towersPerLane) ?? 3,
    towerTargetPriority: validateOptionalArray(params.tower_target_priority || params.towerTargetPriority) ?? ['minion', 'pet', 'champion'],
    towerAggroRange: validateOptionalNumber(params.tower_aggro_range || params.towerAggroRange) ?? 775,
    towerAttackSpeed: validateOptionalNumber(params.tower_attack_speed || params.towerAttackSpeed) ?? 0.83,
    platingsEnabled: validateOptionalBoolean(params.platings_enabled || params.platingsEnabled) ?? true,
    platingCount: validateOptionalNumber(params.plating_count || params.platingCount) ?? 5,
    platingGold: validateOptionalNumber(params.plating_gold || params.platingGold) ?? 160,
    platingFallTime: validateOptionalNumber(params.plating_fall_time || params.platingFallTime) ?? 840 // 14 minutes
  });
};

/**
 * Configure tower type stats
 */
const handleConfigureTowerType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const towerType = validateRequiredEnum(params.tower_type || params.towerType, TOWER_TYPES, 'tower_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'configure_tower_type', {
    towerType,
    baseHealth: validateOptionalNumber(params.base_health || params.baseHealth),
    armor: validateOptionalNumber(params.armor),
    magicResist: validateOptionalNumber(params.magic_resist || params.magicResist),
    baseDamage: validateOptionalNumber(params.base_damage || params.baseDamage),
    damageScaling: validateOptionalNumber(params.damage_scaling || params.damageScaling),
    goldReward: validateOptionalNumber(params.gold_reward || params.goldReward),
    xpReward: validateOptionalNumber(params.xp_reward || params.xpReward),
    trueVision: validateOptionalBoolean(params.true_vision || params.trueVision),
    visionRange: validateOptionalNumber(params.vision_range || params.visionRange)
  });
};

/**
 * Configure jungle camps
 */
const handleConfigureJungleCamps: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'configure_jungle_camps', {
    campCount: validateOptionalNumber(params.camp_count || params.campCount) ?? 6,
    respawnTime: validateOptionalNumber(params.respawn_time || params.respawnTime) ?? 120,
    firstSpawnTime: validateOptionalNumber(params.first_spawn_time || params.firstSpawnTime) ?? 90,
    camps: validateOptionalArray(params.camps)
  });
};

/**
 * Create a jungle camp
 */
const handleCreateJungleCamp: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const campId = validateRequiredString(params.camp_id || params.campId, 'camp_id');
  const campName = validateRequiredString(params.camp_name || params.campName, 'camp_name');
  const campType = validateRequiredEnum(params.camp_type || params.campType, JUNGLE_CAMP_TYPES, 'camp_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'create_jungle_camp', {
    campId,
    campName,
    campType,
    monsterCount: validateOptionalNumber(params.monster_count || params.monsterCount) ?? 1,
    mainMonsterHealth: validateOptionalNumber(params.main_monster_health || params.mainMonsterHealth),
    mainMonsterDamage: validateOptionalNumber(params.main_monster_damage || params.mainMonsterDamage),
    goldReward: validateOptionalNumber(params.gold_reward || params.goldReward),
    xpReward: validateOptionalNumber(params.xp_reward || params.xpReward),
    respawnTime: validateOptionalNumber(params.respawn_time || params.respawnTime),
    firstSpawnTime: validateOptionalNumber(params.first_spawn_time || params.firstSpawnTime),
    buffType: validateOptionalEnum(params.buff_type || params.buffType, BUFF_TYPES),
    buffDuration: validateOptionalNumber(params.buff_duration || params.buffDuration),
    location: validateOptionalObject(params.location),
    side: validateOptionalString(params.side) // 'blue', 'red', 'neutral'
  });
};

/**
 * Setup item shop
 */
const handleSetupItemShop: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'setup_item_shop', {
    shopLocation: validateOptionalString(params.shop_location || params.shopLocation) ?? 'fountain',
    buyAnywhere: validateOptionalBoolean(params.buy_anywhere || params.buyAnywhere) ?? false,
    sellRatio: validateOptionalNumber(params.sell_ratio || params.sellRatio) ?? 0.7,
    undoWindow: validateOptionalNumber(params.undo_window || params.undoWindow) ?? 10,
    starterItemLimit: validateOptionalNumber(params.starter_item_limit || params.starterItemLimit) ?? 1,
    bootsLimit: validateOptionalNumber(params.boots_limit || params.bootsLimit) ?? 1,
    mythicLimit: validateOptionalNumber(params.mythic_limit || params.mythicLimit) ?? 1,
    recommendedEnabled: validateOptionalBoolean(params.recommended_enabled || params.recommendedEnabled) ?? true
  });
};

/**
 * Create an item
 */
const handleCreateItem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const itemId = validateRequiredString(params.item_id || params.itemId, 'item_id');
  const itemName = validateRequiredString(params.item_name || params.itemName, 'item_name');
  const category = validateRequiredEnum(params.category, ITEM_CATEGORIES, 'category');
  const cost = validateOptionalNumber(params.cost) ?? 0;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'create_item', {
    itemId,
    itemName,
    category,
    cost,
    stats: validateOptionalObject(params.stats), // { ad: 50, ap: 0, health: 300, ... }
    buildPath: validateOptionalArray(params.build_path || params.buildPath), // Component item IDs
    passiveName: validateOptionalString(params.passive_name || params.passiveName),
    passiveDescription: validateOptionalString(params.passive_description || params.passiveDescription),
    passiveEffect: validateOptionalObject(params.passive_effect || params.passiveEffect),
    activeName: validateOptionalString(params.active_name || params.activeName),
    activeDescription: validateOptionalString(params.active_description || params.activeDescription),
    activeCooldown: validateOptionalNumber(params.active_cooldown || params.activeCooldown),
    isMythic: validateOptionalBoolean(params.is_mythic || params.isMythic),
    mythicPassive: validateOptionalObject(params.mythic_passive || params.mythicPassive),
    restrictedTo: validateOptionalArray(params.restricted_to || params.restrictedTo), // Role restrictions
    description: validateOptionalString(params.description)
  });
};

/**
 * Create team objectives (Baron, Dragon, etc.)
 */
const handleCreateTeamObjective: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const objectiveId = validateRequiredString(params.objective_id || params.objectiveId, 'objective_id');
  const objectiveName = validateRequiredString(params.objective_name || params.objectiveName, 'objective_name');
  const objectiveType = validateRequiredEnum(params.objective_type || params.objectiveType, OBJECTIVE_TYPES, 'objective_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'create_team_objective', {
    objectiveId,
    objectiveName,
    objectiveType,
    health: validateOptionalNumber(params.health),
    damage: validateOptionalNumber(params.damage),
    armor: validateOptionalNumber(params.armor),
    magicResist: validateOptionalNumber(params.magic_resist || params.magicResist),
    spawnTime: validateOptionalNumber(params.spawn_time || params.spawnTime),
    respawnTime: validateOptionalNumber(params.respawn_time || params.respawnTime),
    buffName: validateOptionalString(params.buff_name || params.buffName),
    buffDuration: validateOptionalNumber(params.buff_duration || params.buffDuration),
    buffEffect: validateOptionalObject(params.buff_effect || params.buffEffect),
    teamwideReward: validateOptionalBoolean(params.teamwide_reward || params.teamwideReward),
    goldReward: validateOptionalNumber(params.gold_reward || params.goldReward),
    xpReward: validateOptionalNumber(params.xp_reward || params.xpReward),
    location: validateOptionalObject(params.location),
    announceSpawn: validateOptionalBoolean(params.announce_spawn || params.announceSpawn) ?? true
  });
};

/**
 * Configure vision system
 */
const handleConfigureVisionSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'configure_vision_system', {
    baseVisionRange: validateOptionalNumber(params.base_vision_range || params.baseVisionRange) ?? 1200,
    brushBlocksVision: validateOptionalBoolean(params.brush_blocks_vision || params.brushBlocksVision) ?? true,
    fogOfWarEnabled: validateOptionalBoolean(params.fog_of_war_enabled || params.fogOfWarEnabled) ?? true,
    wardLimit: validateOptionalNumber(params.ward_limit || params.wardLimit) ?? 3,
    controlWardLimit: validateOptionalNumber(params.control_ward_limit || params.controlWardLimit) ?? 1,
    wardVisionRange: validateOptionalNumber(params.ward_vision_range || params.wardVisionRange) ?? 900,
    wardDuration: validateOptionalNumber(params.ward_duration || params.wardDuration) ?? 90,
    controlWardDuration: validateOptionalNumber(params.control_ward_duration || params.controlWardDuration), // Infinite until destroyed
    sweepingLensRange: validateOptionalNumber(params.sweeping_lens_range || params.sweepingLensRange) ?? 750,
    oracleEnabled: validateOptionalBoolean(params.oracle_enabled || params.oracleEnabled) ?? true
  });
};

/**
 * Configure kill/assist rewards
 */
const handleConfigureKillRewards: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'configure_kill_rewards', {
    baseKillGold: validateOptionalNumber(params.base_kill_gold || params.baseKillGold) ?? 300,
    assistGoldRatio: validateOptionalNumber(params.assist_gold_ratio || params.assistGoldRatio) ?? 0.5,
    killStreakBonus: validateOptionalArray(params.kill_streak_bonus || params.killStreakBonus),
    shutdownGold: validateOptionalArray(params.shutdown_gold || params.shutdownGold),
    firstBloodBonus: validateOptionalNumber(params.first_blood_bonus || params.firstBloodBonus) ?? 100,
    deathTimerBase: validateOptionalNumber(params.death_timer_base || params.deathTimerBase) ?? 10,
    deathTimerScaling: validateOptionalNumber(params.death_timer_scaling || params.deathTimerScaling) ?? 2,
    maxDeathTimer: validateOptionalNumber(params.max_death_timer || params.maxDeathTimer) ?? 60,
    killXp: validateOptionalNumber(params.kill_xp || params.killXp),
    assistXp: validateOptionalNumber(params.assist_xp || params.assistXp)
  });
};

/**
 * Configure leveling and scaling
 */
const handleConfigureLeveling: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'configure_leveling', {
    maxLevel: validateOptionalNumber(params.max_level || params.maxLevel) ?? 18,
    xpCurve: validateOptionalArray(params.xp_curve || params.xpCurve),
    ultimateUnlockLevel: validateOptionalNumber(params.ultimate_unlock_level || params.ultimateUnlockLevel) ?? 6,
    ultimateUpgradeLevels: validateOptionalArray(params.ultimate_upgrade_levels || params.ultimateUpgradeLevels) ?? [11, 16],
    skillPointsPerLevel: validateOptionalNumber(params.skill_points_per_level || params.skillPointsPerLevel) ?? 1,
    passiveGoldPerSecond: validateOptionalNumber(params.passive_gold_per_second || params.passiveGoldPerSecond) ?? 2,
    startingGold: validateOptionalNumber(params.starting_gold || params.startingGold) ?? 500
  });
};

/**
 * Get hero data
 */
const handleGetHeroData: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const heroId = validateRequiredString(params.hero_id || params.heroId, 'hero_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'get_hero_data', {
    heroId,
    includeAbilities: validateOptionalBoolean(params.include_abilities || params.includeAbilities) ?? true,
    includeScaling: validateOptionalBoolean(params.include_scaling || params.includeScaling) ?? false
  });
};

/**
 * Get match state
 */
const handleGetMatchState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'get_match_state', {
    includeScoreboard: validateOptionalBoolean(params.include_scoreboard || params.includeScoreboard) ?? true,
    includeObjectives: validateOptionalBoolean(params.include_objectives || params.includeObjectives) ?? true,
    includeTowers: validateOptionalBoolean(params.include_towers || params.includeTowers) ?? true
  });
};

/**
 * List all heroes
 */
const handleListHeroes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'list_heroes', {
    filterRole: validateOptionalEnum(params.filter_role || params.filterRole, HERO_ROLES),
    includeStats: validateOptionalBoolean(params.include_stats || params.includeStats) ?? false
  });
};

/**
 * List all items
 */
const handleListItems: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_MOBA, 'list_items', {
    filterCategory: validateOptionalEnum(params.filter_category || params.filterCategory, ITEM_CATEGORIES),
    includeBuildPaths: validateOptionalBoolean(params.include_build_paths || params.includeBuildPaths) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const genreMobaHandlers: ActionHandlerMap = {
  // Hero management
  create_hero: handleCreateHero,
  create_ability: handleCreateAbility,

  // Lane and map
  configure_lane_system: handleConfigureLaneSystem,
  setup_minion_waves: handleSetupMinionWaves,
  configure_minion_type: handleConfigureMinionType,

  // Towers
  create_tower_system: handleCreateTowerSystem,
  configure_tower_type: handleConfigureTowerType,

  // Jungle
  configure_jungle_camps: handleConfigureJungleCamps,
  create_jungle_camp: handleCreateJungleCamp,

  // Items
  setup_item_shop: handleSetupItemShop,
  create_item: handleCreateItem,

  // Objectives
  create_team_objective: handleCreateTeamObjective,

  // Systems
  configure_vision_system: handleConfigureVisionSystem,
  configure_kill_rewards: handleConfigureKillRewards,
  configure_leveling: handleConfigureLeveling,

  // Query
  get_hero_data: handleGetHeroData,
  get_match_state: handleGetMatchState,
  list_heroes: handleListHeroes,
  list_items: handleListItems
};

// Create the main handler function for this category
export const handleGenreMobaTools = createCategoryHandler(Forge2Categories.GENRE_MOBA, genreMobaHandlers);

export default genreMobaHandlers;
