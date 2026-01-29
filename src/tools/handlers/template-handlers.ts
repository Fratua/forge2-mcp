/**
 * FORGE2: Template Handlers
 * Handles generation templates for characters, weapons, NPCs, quests, dungeons, and open world chunks
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
// Character Template Action Handlers
// ============================================================================

/**
 * Generate character from template
 */
const handleGenerateCharacterTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const characterName = validateRequiredString(params.character_name || params.characterName, 'character_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'generate_character_template', {
    characterName,
    characterType: validateOptionalString(params.character_type || params.characterType) ?? 'player',
    baseClass: validateOptionalString(params.base_class || params.baseClass) ?? 'Character',
    includeMovement: validateOptionalBoolean(params.include_movement || params.includeMovement) ?? true,
    includeInput: validateOptionalBoolean(params.include_input || params.includeInput) ?? true,
    includeCamera: validateOptionalBoolean(params.include_camera || params.includeCamera) ?? true,
    includeCombat: validateOptionalBoolean(params.include_combat || params.includeCombat) ?? false,
    includeInventory: validateOptionalBoolean(params.include_inventory || params.includeInventory) ?? false,
    skeletalMesh: validateOptionalString(params.skeletal_mesh || params.skeletalMesh),
    animBlueprint: validateOptionalString(params.anim_blueprint || params.animBlueprint),
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Characters/${characterName}`
  });
};

/**
 * Configure character template
 */
const handleConfigureCharacterTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'configure_character_template', {
    templateName,
    movementSpeed: validateOptionalNumber(params.movement_speed || params.movementSpeed) ?? 600,
    jumpHeight: validateOptionalNumber(params.jump_height || params.jumpHeight) ?? 420,
    capsuleRadius: validateOptionalNumber(params.capsule_radius || params.capsuleRadius) ?? 42,
    capsuleHalfHeight: validateOptionalNumber(params.capsule_half_height || params.capsuleHalfHeight) ?? 96,
    canCrouch: validateOptionalBoolean(params.can_crouch || params.canCrouch) ?? true,
    canSwim: validateOptionalBoolean(params.can_swim || params.canSwim) ?? true,
    canFly: validateOptionalBoolean(params.can_fly || params.canFly) ?? false
  });
};

/**
 * Add character abilities
 */
const handleAddCharacterAbilities: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'add_character_abilities', {
    templateName,
    abilities: validateOptionalArray(params.abilities) ?? [],
    useGAS: validateOptionalBoolean(params.use_gas || params.useGAS) ?? true,
    abilitySystemComponent: validateOptionalString(params.ability_system_component || params.abilitySystemComponent)
  });
};

/**
 * Finalize character template
 */
const handleFinalizeCharacterTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'finalize_character_template', {
    templateName,
    compileBlueprint: validateOptionalBoolean(params.compile_blueprint || params.compileBlueprint) ?? true,
    createDefaultInstance: validateOptionalBoolean(params.create_default_instance || params.createDefaultInstance) ?? false
  });
};

// ============================================================================
// Weapon System Template Action Handlers
// ============================================================================

/**
 * Generate weapon system template
 */
const handleGenerateWeaponTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const weaponName = validateRequiredString(params.weapon_name || params.weaponName, 'weapon_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'generate_weapon_template', {
    weaponName,
    weaponType: validateOptionalString(params.weapon_type || params.weaponType) ?? 'melee',
    damageType: validateOptionalString(params.damage_type || params.damageType) ?? 'physical',
    baseDamage: validateOptionalNumber(params.base_damage || params.baseDamage) ?? 10,
    attackSpeed: validateOptionalNumber(params.attack_speed || params.attackSpeed) ?? 1.0,
    range: validateOptionalNumber(params.range) ?? 150,
    includeAbility: validateOptionalBoolean(params.include_ability || params.includeAbility) ?? true,
    includeDamageComponent: validateOptionalBoolean(params.include_damage_component || params.includeDamageComponent) ?? true,
    mesh: validateOptionalString(params.mesh),
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Weapons/${weaponName}`
  });
};

/**
 * Configure weapon template
 */
const handleConfigureWeaponTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'configure_weapon_template', {
    templateName,
    criticalChance: validateOptionalNumber(params.critical_chance || params.criticalChance) ?? 0.05,
    criticalMultiplier: validateOptionalNumber(params.critical_multiplier || params.criticalMultiplier) ?? 2.0,
    knockback: validateOptionalNumber(params.knockback) ?? 0,
    stagger: validateOptionalBoolean(params.stagger) ?? false,
    canBlock: validateOptionalBoolean(params.can_block || params.canBlock) ?? false,
    blockDamageReduction: validateOptionalNumber(params.block_damage_reduction || params.blockDamageReduction) ?? 0.5
  });
};

/**
 * Add weapon effects
 */
const handleAddWeaponEffects: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'add_weapon_effects', {
    templateName,
    hitEffects: validateOptionalArray(params.hit_effects || params.hitEffects) ?? [],
    swingEffects: validateOptionalArray(params.swing_effects || params.swingEffects) ?? [],
    trailEffect: validateOptionalString(params.trail_effect || params.trailEffect),
    hitSound: validateOptionalString(params.hit_sound || params.hitSound),
    swingSound: validateOptionalString(params.swing_sound || params.swingSound)
  });
};

/**
 * Finalize weapon template
 */
const handleFinalizeWeaponTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'finalize_weapon_template', {
    templateName,
    createDataAsset: validateOptionalBoolean(params.create_data_asset || params.createDataAsset) ?? true,
    createBlueprint: validateOptionalBoolean(params.create_blueprint || params.createBlueprint) ?? true
  });
};

// ============================================================================
// NPC AI Template Action Handlers
// ============================================================================

/**
 * Generate NPC AI template
 */
const handleGenerateNPCTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const npcName = validateRequiredString(params.npc_name || params.npcName, 'npc_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'generate_npc_template', {
    npcName,
    npcType: validateOptionalString(params.npc_type || params.npcType) ?? 'friendly',
    includeBehaviorTree: validateOptionalBoolean(params.include_behavior_tree || params.includeBehaviorTree) ?? true,
    includeDialogue: validateOptionalBoolean(params.include_dialogue || params.includeDialogue) ?? true,
    includePerception: validateOptionalBoolean(params.include_perception || params.includePerception) ?? true,
    includeCombat: validateOptionalBoolean(params.include_combat || params.includeCombat) ?? false,
    includeSchedule: validateOptionalBoolean(params.include_schedule || params.includeSchedule) ?? false,
    skeletalMesh: validateOptionalString(params.skeletal_mesh || params.skeletalMesh),
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/NPCs/${npcName}`
  });
};

/**
 * Configure NPC AI template
 */
const handleConfigureNPCTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'configure_npc_template', {
    templateName,
    sightRadius: validateOptionalNumber(params.sight_radius || params.sightRadius) ?? 1500,
    hearingRange: validateOptionalNumber(params.hearing_range || params.hearingRange) ?? 1000,
    patrolRadius: validateOptionalNumber(params.patrol_radius || params.patrolRadius) ?? 500,
    aggressionLevel: validateOptionalNumber(params.aggression_level || params.aggressionLevel) ?? 0,
    fleeHealthPercent: validateOptionalNumber(params.flee_health_percent || params.fleeHealthPercent) ?? 0.2
  });
};

/**
 * Add NPC behaviors
 */
const handleAddNPCBehaviors: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'add_npc_behaviors', {
    templateName,
    behaviors: validateOptionalArray(params.behaviors) ?? ['idle', 'patrol', 'interact'],
    customTasks: validateOptionalArray(params.custom_tasks || params.customTasks) ?? [],
    dialogueTrees: validateOptionalArray(params.dialogue_trees || params.dialogueTrees) ?? []
  });
};

/**
 * Finalize NPC template
 */
const handleFinalizeNPCTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'finalize_npc_template', {
    templateName,
    createAIController: validateOptionalBoolean(params.create_ai_controller || params.createAIController) ?? true,
    createBlackboard: validateOptionalBoolean(params.create_blackboard || params.createBlackboard) ?? true
  });
};

// ============================================================================
// Quest Chain Template Action Handlers
// ============================================================================

/**
 * Generate quest chain template
 */
const handleGenerateQuestChainTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const chainName = validateRequiredString(params.chain_name || params.chainName, 'chain_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'generate_quest_chain_template', {
    chainName,
    questCount: validateOptionalNumber(params.quest_count || params.questCount) ?? 3,
    chainType: validateOptionalString(params.chain_type || params.chainType) ?? 'linear',
    category: validateOptionalString(params.category) ?? 'main',
    levelRange: validateOptionalArray(params.level_range || params.levelRange) ?? [1, 10],
    rewardScaling: validateOptionalNumber(params.reward_scaling || params.rewardScaling) ?? 1.5,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Quests/Chains/${chainName}`
  });
};

/**
 * Configure quest chain template
 */
const handleConfigureQuestChainTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'configure_quest_chain_template', {
    templateName,
    objectiveTypes: validateOptionalArray(params.objective_types || params.objectiveTypes) ?? ['kill', 'collect', 'talk_to'],
    includeDialogue: validateOptionalBoolean(params.include_dialogue || params.includeDialogue) ?? true,
    includeRewards: validateOptionalBoolean(params.include_rewards || params.includeRewards) ?? true,
    allowBranching: validateOptionalBoolean(params.allow_branching || params.allowBranching) ?? false
  });
};

/**
 * Add quest chain quests
 */
const handleAddQuestChainQuests: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'add_quest_chain_quests', {
    templateName,
    quests: validateOptionalArray(params.quests) ?? [],
    connections: validateOptionalArray(params.connections) ?? []
  });
};

/**
 * Finalize quest chain template
 */
const handleFinalizeQuestChainTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'finalize_quest_chain_template', {
    templateName,
    createDataAssets: validateOptionalBoolean(params.create_data_assets || params.createDataAssets) ?? true,
    registerWithSubsystem: validateOptionalBoolean(params.register_with_subsystem || params.registerWithSubsystem) ?? true
  });
};

// ============================================================================
// Dungeon Level Template Action Handlers
// ============================================================================

/**
 * Generate dungeon level template
 */
const handleGenerateDungeonTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const dungeonName = validateRequiredString(params.dungeon_name || params.dungeonName, 'dungeon_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'generate_dungeon_template', {
    dungeonName,
    algorithm: validateOptionalString(params.algorithm) ?? 'bsp',
    roomCount: validateOptionalNumber(params.room_count || params.roomCount) ?? 10,
    dungeonSize: validateOptionalObject(params.dungeon_size || params.dungeonSize) ?? { width: 5000, height: 5000 },
    roomSizeRange: validateOptionalArray(params.room_size_range || params.roomSizeRange) ?? [400, 800],
    corridorWidth: validateOptionalNumber(params.corridor_width || params.corridorWidth) ?? 200,
    includeLoops: validateOptionalBoolean(params.include_loops || params.includeLoops) ?? true,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Dungeons/${dungeonName}`
  });
};

/**
 * Configure dungeon template
 */
const handleConfigureDungeonTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'configure_dungeon_template', {
    templateName,
    roomTypes: validateOptionalArray(params.room_types || params.roomTypes) ?? ['normal', 'treasure', 'boss', 'trap', 'rest'],
    enemyDensity: validateOptionalNumber(params.enemy_density || params.enemyDensity) ?? 0.5,
    lootDensity: validateOptionalNumber(params.loot_density || params.lootDensity) ?? 0.3,
    trapDensity: validateOptionalNumber(params.trap_density || params.trapDensity) ?? 0.2,
    lightingStyle: validateOptionalString(params.lighting_style || params.lightingStyle) ?? 'torchlit'
  });
};

/**
 * Add dungeon features
 */
const handleAddDungeonFeatures: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'add_dungeon_features', {
    templateName,
    doors: validateOptionalBoolean(params.doors) ?? true,
    secretPassages: validateOptionalBoolean(params.secret_passages || params.secretPassages) ?? true,
    puzzles: validateOptionalBoolean(params.puzzles) ?? false,
    hazards: validateOptionalArray(params.hazards) ?? ['spikes', 'pits'],
    decorations: validateOptionalArray(params.decorations) ?? ['torches', 'debris', 'cobwebs']
  });
};

/**
 * Finalize dungeon template
 */
const handleFinalizeDungeonTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'finalize_dungeon_template', {
    templateName,
    generateLevel: validateOptionalBoolean(params.generate_level || params.generateLevel) ?? true,
    createNavMesh: validateOptionalBoolean(params.create_nav_mesh || params.createNavMesh) ?? true,
    spawnEnemies: validateOptionalBoolean(params.spawn_enemies || params.spawnEnemies) ?? true,
    placeLoot: validateOptionalBoolean(params.place_loot || params.placeLoot) ?? true
  });
};

// ============================================================================
// Open World Chunk Template Action Handlers
// ============================================================================

/**
 * Generate open world chunk template
 */
const handleGenerateOpenWorldChunkTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const chunkName = validateRequiredString(params.chunk_name || params.chunkName, 'chunk_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'generate_open_world_chunk_template', {
    chunkName,
    chunkSize: validateOptionalNumber(params.chunk_size || params.chunkSize) ?? 2048,
    biome: validateOptionalString(params.biome) ?? 'temperate_forest',
    terrainType: validateOptionalString(params.terrain_type || params.terrainType) ?? 'rolling_hills',
    includeVegetation: validateOptionalBoolean(params.include_vegetation || params.includeVegetation) ?? true,
    includeProps: validateOptionalBoolean(params.include_props || params.includeProps) ?? true,
    includeStructures: validateOptionalBoolean(params.include_structures || params.includeStructures) ?? false,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/WorldChunks/${chunkName}`
  });
};

/**
 * Configure open world chunk template
 */
const handleConfigureOpenWorldChunkTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'configure_open_world_chunk_template', {
    templateName,
    heightRange: validateOptionalArray(params.height_range || params.heightRange) ?? [0, 500],
    vegetationDensity: validateOptionalNumber(params.vegetation_density || params.vegetationDensity) ?? 0.7,
    propDensity: validateOptionalNumber(params.prop_density || params.propDensity) ?? 0.3,
    waterBodies: validateOptionalArray(params.water_bodies || params.waterBodies) ?? [],
    roads: validateOptionalArray(params.roads) ?? []
  });
};

/**
 * Add chunk features
 */
const handleAddChunkFeatures: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'add_chunk_features', {
    templateName,
    pointsOfInterest: validateOptionalArray(params.points_of_interest || params.pointsOfInterest) ?? [],
    spawnPoints: validateOptionalArray(params.spawn_points || params.spawnPoints) ?? [],
    collectibles: validateOptionalArray(params.collectibles) ?? [],
    ambientSounds: validateOptionalArray(params.ambient_sounds || params.ambientSounds) ?? []
  });
};

/**
 * Finalize open world chunk template
 */
const handleFinalizeOpenWorldChunkTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'finalize_open_world_chunk_template', {
    templateName,
    generateTerrain: validateOptionalBoolean(params.generate_terrain || params.generateTerrain) ?? true,
    placeVegetation: validateOptionalBoolean(params.place_vegetation || params.placeVegetation) ?? true,
    createStreamingLevel: validateOptionalBoolean(params.create_streaming_level || params.createStreamingLevel) ?? true,
    buildNavMesh: validateOptionalBoolean(params.build_nav_mesh || params.buildNavMesh) ?? true
  });
};

// ============================================================================
// Shop/Vendor Template Action Handlers
// ============================================================================

/**
 * Generate shop template
 */
const handleGenerateShopTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const shopName = validateRequiredString(params.shop_name || params.shopName, 'shop_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'generate_shop_template', {
    shopName,
    shopType: validateOptionalString(params.shop_type || params.shopType) ?? 'general',
    vendorNPC: validateOptionalBoolean(params.vendor_npc || params.vendorNPC) ?? true,
    includeUI: validateOptionalBoolean(params.include_ui || params.includeUI) ?? true,
    includeInventory: validateOptionalBoolean(params.include_inventory || params.includeInventory) ?? true,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Shops/${shopName}`
  });
};

/**
 * Configure shop template
 */
const handleConfigureShopTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'configure_shop_template', {
    templateName,
    buyPriceMultiplier: validateOptionalNumber(params.buy_price_multiplier || params.buyPriceMultiplier) ?? 1.0,
    sellPriceMultiplier: validateOptionalNumber(params.sell_price_multiplier || params.sellPriceMultiplier) ?? 0.5,
    stockRefreshTime: validateOptionalNumber(params.stock_refresh_time || params.stockRefreshTime) ?? 86400,
    reputationRequired: validateOptionalString(params.reputation_required || params.reputationRequired),
    currencyTypes: validateOptionalArray(params.currency_types || params.currencyTypes) ?? ['gold']
  });
};

// ============================================================================
// Crafting Station Template Action Handlers
// ============================================================================

/**
 * Generate crafting station template
 */
const handleGenerateCraftingStationTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const stationName = validateRequiredString(params.station_name || params.stationName, 'station_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'generate_crafting_station_template', {
    stationName,
    stationType: validateOptionalString(params.station_type || params.stationType) ?? 'workbench',
    includeUI: validateOptionalBoolean(params.include_ui || params.includeUI) ?? true,
    includeRecipeSystem: validateOptionalBoolean(params.include_recipe_system || params.includeRecipeSystem) ?? true,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Crafting/${stationName}`
  });
};

/**
 * Configure crafting station template
 */
const handleConfigureCraftingStationTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'configure_crafting_station_template', {
    templateName,
    recipeCategories: validateOptionalArray(params.recipe_categories || params.recipeCategories) ?? [],
    skillRequirements: validateOptionalObject(params.skill_requirements || params.skillRequirements),
    craftingTimeMultiplier: validateOptionalNumber(params.crafting_time_multiplier || params.craftingTimeMultiplier) ?? 1.0,
    successChanceBonus: validateOptionalNumber(params.success_chance_bonus || params.successChanceBonus) ?? 0
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * List available templates
 */
const handleListTemplates: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'list_templates', {
    category: validateOptionalString(params.category),
    includeBuiltIn: validateOptionalBoolean(params.include_built_in || params.includeBuiltIn) ?? true,
    includeCustom: validateOptionalBoolean(params.include_custom || params.includeCustom) ?? true
  });
};

/**
 * Get template info
 */
const handleGetTemplateInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'get_template_info', {
    templateName,
    includeParameters: validateOptionalBoolean(params.include_parameters || params.includeParameters) ?? true,
    includePreview: validateOptionalBoolean(params.include_preview || params.includePreview) ?? false
  });
};

/**
 * Validate template
 */
const handleValidateTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.TEMPLATES, 'validate_template', {
    templateName,
    checkReferences: validateOptionalBoolean(params.check_references || params.checkReferences) ?? true,
    checkCompilation: validateOptionalBoolean(params.check_compilation || params.checkCompilation) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const templateHandlers: ActionHandlerMap = {
  // Character Template
  generate_character_template: handleGenerateCharacterTemplate,
  configure_character_template: handleConfigureCharacterTemplate,
  add_character_abilities: handleAddCharacterAbilities,
  finalize_character_template: handleFinalizeCharacterTemplate,

  // Weapon System Template
  generate_weapon_template: handleGenerateWeaponTemplate,
  configure_weapon_template: handleConfigureWeaponTemplate,
  add_weapon_effects: handleAddWeaponEffects,
  finalize_weapon_template: handleFinalizeWeaponTemplate,

  // NPC AI Template
  generate_npc_template: handleGenerateNPCTemplate,
  configure_npc_template: handleConfigureNPCTemplate,
  add_npc_behaviors: handleAddNPCBehaviors,
  finalize_npc_template: handleFinalizeNPCTemplate,

  // Quest Chain Template
  generate_quest_chain_template: handleGenerateQuestChainTemplate,
  configure_quest_chain_template: handleConfigureQuestChainTemplate,
  add_quest_chain_quests: handleAddQuestChainQuests,
  finalize_quest_chain_template: handleFinalizeQuestChainTemplate,

  // Dungeon Level Template
  generate_dungeon_template: handleGenerateDungeonTemplate,
  configure_dungeon_template: handleConfigureDungeonTemplate,
  add_dungeon_features: handleAddDungeonFeatures,
  finalize_dungeon_template: handleFinalizeDungeonTemplate,

  // Open World Chunk Template
  generate_open_world_chunk_template: handleGenerateOpenWorldChunkTemplate,
  configure_open_world_chunk_template: handleConfigureOpenWorldChunkTemplate,
  add_chunk_features: handleAddChunkFeatures,
  finalize_open_world_chunk_template: handleFinalizeOpenWorldChunkTemplate,

  // Shop/Vendor Template
  generate_shop_template: handleGenerateShopTemplate,
  configure_shop_template: handleConfigureShopTemplate,

  // Crafting Station Template
  generate_crafting_station_template: handleGenerateCraftingStationTemplate,
  configure_crafting_station_template: handleConfigureCraftingStationTemplate,

  // Query
  list_templates: handleListTemplates,
  get_template_info: handleGetTemplateInfo,
  validate_template: handleValidateTemplate
};

export const handleTemplateTools = createCategoryHandler(Forge2Categories.TEMPLATES, templateHandlers);

export default templateHandlers;
