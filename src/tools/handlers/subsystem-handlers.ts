/**
 * FORGE2: Subsystem Handlers
 * Handles Unreal Engine subsystems: quest, economy, spawn, weather, worldgen, dialogue, audio, analytics
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
// Quest Subsystem Action Handlers
// ============================================================================

/**
 * Create quest subsystem
 */
const handleCreateQuestSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'create_quest_subsystem', {
    persistQuestState: validateOptionalBoolean(params.persist_quest_state || params.persistQuestState) ?? true,
    autoTrackQuests: validateOptionalBoolean(params.auto_track_quests || params.autoTrackQuests) ?? true,
    maxTrackedQuests: validateOptionalNumber(params.max_tracked_quests || params.maxTrackedQuests) ?? 3,
    enableQuestMarkers: validateOptionalBoolean(params.enable_quest_markers || params.enableQuestMarkers) ?? true
  });
};

/**
 * Configure quest subsystem
 */
const handleConfigureQuestSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'configure_quest_subsystem', {
    questUpdateFrequency: validateOptionalNumber(params.quest_update_frequency || params.questUpdateFrequency) ?? 0.5,
    enableQuestLog: validateOptionalBoolean(params.enable_quest_log || params.enableQuestLog) ?? true,
    enableNotifications: validateOptionalBoolean(params.enable_notifications || params.enableNotifications) ?? true,
    questCategories: validateOptionalArray(params.quest_categories || params.questCategories) ?? ['main', 'side', 'daily', 'weekly']
  });
};

/**
 * Register quest definition
 */
const handleRegisterQuestDefinition: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'register_quest_definition', {
    questId,
    questDataAsset: validateOptionalString(params.quest_data_asset || params.questDataAsset),
    autoActivate: validateOptionalBoolean(params.auto_activate || params.autoActivate) ?? false
  });
};

/**
 * Get quest subsystem status
 */
const handleGetQuestSubsystemStatus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'get_quest_subsystem_status', {
    includeActiveQuests: validateOptionalBoolean(params.include_active_quests || params.includeActiveQuests) ?? true,
    includeCompletedQuests: validateOptionalBoolean(params.include_completed_quests || params.includeCompletedQuests) ?? false,
    includeFailedQuests: validateOptionalBoolean(params.include_failed_quests || params.includeFailedQuests) ?? false
  });
};

// ============================================================================
// Economy Subsystem Action Handlers
// ============================================================================

/**
 * Create economy subsystem
 */
const handleCreateEconomySubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'create_economy_subsystem', {
    enableDynamicPricing: validateOptionalBoolean(params.enable_dynamic_pricing || params.enableDynamicPricing) ?? false,
    inflationRate: validateOptionalNumber(params.inflation_rate || params.inflationRate) ?? 0,
    taxRate: validateOptionalNumber(params.tax_rate || params.taxRate) ?? 0,
    enableAuctionHouse: validateOptionalBoolean(params.enable_auction_house || params.enableAuctionHouse) ?? false
  });
};

/**
 * Configure economy subsystem
 */
const handleConfigureEconomySubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'configure_economy_subsystem', {
    currencyTypes: validateOptionalArray(params.currency_types || params.currencyTypes) ?? ['gold'],
    priceUpdateInterval: validateOptionalNumber(params.price_update_interval || params.priceUpdateInterval) ?? 3600,
    supplyDemandFactor: validateOptionalNumber(params.supply_demand_factor || params.supplyDemandFactor) ?? 0.1,
    goldSinkMultiplier: validateOptionalNumber(params.gold_sink_multiplier || params.goldSinkMultiplier) ?? 1.0
  });
};

/**
 * Register shop
 */
const handleRegisterShop: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const shopId = validateRequiredString(params.shop_id || params.shopId, 'shop_id');

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'register_shop', {
    shopId,
    shopDataAsset: validateOptionalString(params.shop_data_asset || params.shopDataAsset),
    vendorActor: validateOptionalString(params.vendor_actor || params.vendorActor),
    refreshInterval: validateOptionalNumber(params.refresh_interval || params.refreshInterval) ?? 86400
  });
};

/**
 * Get economy status
 */
const handleGetEconomyStatus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'get_economy_status', {
    includeCurrencyStats: validateOptionalBoolean(params.include_currency_stats || params.includeCurrencyStats) ?? true,
    includeShopStats: validateOptionalBoolean(params.include_shop_stats || params.includeShopStats) ?? true,
    includeTransactionHistory: validateOptionalBoolean(params.include_transaction_history || params.includeTransactionHistory) ?? false
  });
};

// ============================================================================
// Spawn Subsystem Action Handlers
// ============================================================================

/**
 * Create spawn subsystem
 */
const handleCreateSpawnSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'create_spawn_subsystem', {
    useObjectPooling: validateOptionalBoolean(params.use_object_pooling || params.useObjectPooling) ?? true,
    defaultPoolSize: validateOptionalNumber(params.default_pool_size || params.defaultPoolSize) ?? 50,
    maxActiveEntities: validateOptionalNumber(params.max_active_entities || params.maxActiveEntities) ?? 500,
    enableDynamicSpawning: validateOptionalBoolean(params.enable_dynamic_spawning || params.enableDynamicSpawning) ?? true
  });
};

/**
 * Configure spawn subsystem
 */
const handleConfigureSpawnSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'configure_spawn_subsystem', {
    spawnTickRate: validateOptionalNumber(params.spawn_tick_rate || params.spawnTickRate) ?? 0.5,
    despawnDistance: validateOptionalNumber(params.despawn_distance || params.despawnDistance) ?? 10000,
    respawnDelay: validateOptionalNumber(params.respawn_delay || params.respawnDelay) ?? 30,
    levelBasedScaling: validateOptionalBoolean(params.level_based_scaling || params.levelBasedScaling) ?? true
  });
};

/**
 * Register spawn pool
 */
const handleRegisterSpawnPool: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const poolId = validateRequiredString(params.pool_id || params.poolId, 'pool_id');
  const actorClass = validateRequiredString(params.actor_class || params.actorClass, 'actor_class');

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'register_spawn_pool', {
    poolId,
    actorClass,
    initialSize: validateOptionalNumber(params.initial_size || params.initialSize) ?? 10,
    maxSize: validateOptionalNumber(params.max_size || params.maxSize) ?? 50,
    growthRate: validateOptionalNumber(params.growth_rate || params.growthRate) ?? 5
  });
};

/**
 * Get spawn subsystem status
 */
const handleGetSpawnSubsystemStatus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'get_spawn_subsystem_status', {
    includePoolStats: validateOptionalBoolean(params.include_pool_stats || params.includePoolStats) ?? true,
    includeActiveEntities: validateOptionalBoolean(params.include_active_entities || params.includeActiveEntities) ?? true,
    includeSpawnPoints: validateOptionalBoolean(params.include_spawn_points || params.includeSpawnPoints) ?? false
  });
};

// ============================================================================
// Weather Subsystem Action Handlers
// ============================================================================

/**
 * Create weather subsystem
 */
const handleCreateWeatherSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'create_weather_subsystem', {
    enableDayNightCycle: validateOptionalBoolean(params.enable_day_night_cycle || params.enableDayNightCycle) ?? true,
    dayLengthMinutes: validateOptionalNumber(params.day_length_minutes || params.dayLengthMinutes) ?? 24,
    enableWeatherSystem: validateOptionalBoolean(params.enable_weather_system || params.enableWeatherSystem) ?? true,
    enableSeasons: validateOptionalBoolean(params.enable_seasons || params.enableSeasons) ?? false
  });
};

/**
 * Configure weather subsystem
 */
const handleConfigureWeatherSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'configure_weather_subsystem', {
    weatherTransitionDuration: validateOptionalNumber(params.weather_transition_duration || params.weatherTransitionDuration) ?? 60,
    weatherTypes: validateOptionalArray(params.weather_types || params.weatherTypes) ?? ['clear', 'cloudy', 'rain', 'storm', 'snow', 'fog'],
    weatherWeights: validateOptionalObject(params.weather_weights || params.weatherWeights),
    temperatureRange: validateOptionalArray(params.temperature_range || params.temperatureRange) ?? [-10, 40]
  });
};

/**
 * Register weather zone
 */
const handleRegisterWeatherZone: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const zoneId = validateRequiredString(params.zone_id || params.zoneId, 'zone_id');

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'register_weather_zone', {
    zoneId,
    zoneBounds: validateOptionalObject(params.zone_bounds || params.zoneBounds),
    defaultWeather: validateOptionalString(params.default_weather || params.defaultWeather) ?? 'clear',
    weatherOverrides: validateOptionalObject(params.weather_overrides || params.weatherOverrides)
  });
};

/**
 * Get weather subsystem status
 */
const handleGetWeatherSubsystemStatus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'get_weather_subsystem_status', {
    includeCurrentWeather: validateOptionalBoolean(params.include_current_weather || params.includeCurrentWeather) ?? true,
    includeTimeOfDay: validateOptionalBoolean(params.include_time_of_day || params.includeTimeOfDay) ?? true,
    includeForecast: validateOptionalBoolean(params.include_forecast || params.includeForecast) ?? false
  });
};

// ============================================================================
// WorldGen Subsystem Action Handlers
// ============================================================================

/**
 * Create worldgen subsystem
 */
const handleCreateWorldGenSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'create_worldgen_subsystem', {
    enableRuntimeGeneration: validateOptionalBoolean(params.enable_runtime_generation || params.enableRuntimeGeneration) ?? false,
    chunkSize: validateOptionalNumber(params.chunk_size || params.chunkSize) ?? 1000,
    loadDistance: validateOptionalNumber(params.load_distance || params.loadDistance) ?? 5000,
    unloadDistance: validateOptionalNumber(params.unload_distance || params.unloadDistance) ?? 7000
  });
};

/**
 * Configure worldgen subsystem
 */
const handleConfigureWorldGenSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'configure_worldgen_subsystem', {
    worldSeed: validateOptionalNumber(params.world_seed || params.worldSeed),
    generationQuality: validateOptionalString(params.generation_quality || params.generationQuality) ?? 'high',
    asyncGeneration: validateOptionalBoolean(params.async_generation || params.asyncGeneration) ?? true,
    maxConcurrentChunks: validateOptionalNumber(params.max_concurrent_chunks || params.maxConcurrentChunks) ?? 4
  });
};

/**
 * Register worldgen preset
 */
const handleRegisterWorldGenPreset: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const presetId = validateRequiredString(params.preset_id || params.presetId, 'preset_id');

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'register_worldgen_preset', {
    presetId,
    presetDataAsset: validateOptionalString(params.preset_data_asset || params.presetDataAsset),
    biomeTypes: validateOptionalArray(params.biome_types || params.biomeTypes),
    structureTypes: validateOptionalArray(params.structure_types || params.structureTypes)
  });
};

/**
 * Get worldgen subsystem status
 */
const handleGetWorldGenSubsystemStatus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'get_worldgen_subsystem_status', {
    includeLoadedChunks: validateOptionalBoolean(params.include_loaded_chunks || params.includeLoadedChunks) ?? true,
    includeGenerationQueue: validateOptionalBoolean(params.include_generation_queue || params.includeGenerationQueue) ?? true,
    includeMemoryUsage: validateOptionalBoolean(params.include_memory_usage || params.includeMemoryUsage) ?? false
  });
};

// ============================================================================
// Dialogue Subsystem Action Handlers
// ============================================================================

/**
 * Create dialogue subsystem
 */
const handleCreateDialogueSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'create_dialogue_subsystem', {
    enableVoiceover: validateOptionalBoolean(params.enable_voiceover || params.enableVoiceover) ?? true,
    enableSubtitles: validateOptionalBoolean(params.enable_subtitles || params.enableSubtitles) ?? true,
    dialogueSpeed: validateOptionalNumber(params.dialogue_speed || params.dialogueSpeed) ?? 1.0,
    autoAdvanceDelay: validateOptionalNumber(params.auto_advance_delay || params.autoAdvanceDelay) ?? 3.0
  });
};

/**
 * Configure dialogue subsystem
 */
const handleConfigureDialogueSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'configure_dialogue_subsystem', {
    typewriterEffect: validateOptionalBoolean(params.typewriter_effect || params.typewriterEffect) ?? true,
    typewriterSpeed: validateOptionalNumber(params.typewriter_speed || params.typewriterSpeed) ?? 50,
    enableSkipping: validateOptionalBoolean(params.enable_skipping || params.enableSkipping) ?? true,
    enableHistory: validateOptionalBoolean(params.enable_history || params.enableHistory) ?? true
  });
};

// ============================================================================
// Audio Subsystem Action Handlers
// ============================================================================

/**
 * Create audio subsystem
 */
const handleCreateAudioSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'create_audio_subsystem', {
    enableDynamicMusic: validateOptionalBoolean(params.enable_dynamic_music || params.enableDynamicMusic) ?? true,
    enable3DAudio: validateOptionalBoolean(params.enable_3d_audio || params.enable3DAudio) ?? true,
    maxConcurrentSounds: validateOptionalNumber(params.max_concurrent_sounds || params.maxConcurrentSounds) ?? 32,
    enableOcclusion: validateOptionalBoolean(params.enable_occlusion || params.enableOcclusion) ?? true
  });
};

/**
 * Configure audio subsystem
 */
const handleConfigureAudioSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'configure_audio_subsystem', {
    masterVolume: validateOptionalNumber(params.master_volume || params.masterVolume) ?? 1.0,
    musicVolume: validateOptionalNumber(params.music_volume || params.musicVolume) ?? 0.8,
    sfxVolume: validateOptionalNumber(params.sfx_volume || params.sfxVolume) ?? 1.0,
    voiceVolume: validateOptionalNumber(params.voice_volume || params.voiceVolume) ?? 1.0,
    ambienceVolume: validateOptionalNumber(params.ambience_volume || params.ambienceVolume) ?? 0.7
  });
};

// ============================================================================
// Analytics Subsystem Action Handlers
// ============================================================================

/**
 * Create analytics subsystem
 */
const handleCreateAnalyticsSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'create_analytics_subsystem', {
    enableEventTracking: validateOptionalBoolean(params.enable_event_tracking || params.enableEventTracking) ?? true,
    enableHeatmaps: validateOptionalBoolean(params.enable_heatmaps || params.enableHeatmaps) ?? false,
    enableSessionTracking: validateOptionalBoolean(params.enable_session_tracking || params.enableSessionTracking) ?? true,
    batchSize: validateOptionalNumber(params.batch_size || params.batchSize) ?? 100
  });
};

/**
 * Configure analytics subsystem
 */
const handleConfigureAnalyticsSubsystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'configure_analytics_subsystem', {
    flushInterval: validateOptionalNumber(params.flush_interval || params.flushInterval) ?? 60,
    enableDebugLogging: validateOptionalBoolean(params.enable_debug_logging || params.enableDebugLogging) ?? false,
    eventCategories: validateOptionalArray(params.event_categories || params.eventCategories) ?? ['gameplay', 'ui', 'progression', 'economy']
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * List all subsystems
 */
const handleListSubsystems: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'list_subsystems', {
    includeStatus: validateOptionalBoolean(params.include_status || params.includeStatus) ?? true,
    includeConfig: validateOptionalBoolean(params.include_config || params.includeConfig) ?? false
  });
};

/**
 * Get subsystem info
 */
const handleGetSubsystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const subsystemType = validateRequiredString(params.subsystem_type || params.subsystemType, 'subsystem_type');

  return executeForge2Action(tools, Forge2Categories.SUBSYSTEMS, 'get_subsystem_info', {
    subsystemType,
    includeMetrics: validateOptionalBoolean(params.include_metrics || params.includeMetrics) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const subsystemHandlers: ActionHandlerMap = {
  // Quest Subsystem
  create_quest_subsystem: handleCreateQuestSubsystem,
  configure_quest_subsystem: handleConfigureQuestSubsystem,
  register_quest_definition: handleRegisterQuestDefinition,
  get_quest_subsystem_status: handleGetQuestSubsystemStatus,

  // Economy Subsystem
  create_economy_subsystem: handleCreateEconomySubsystem,
  configure_economy_subsystem: handleConfigureEconomySubsystem,
  register_shop: handleRegisterShop,
  get_economy_status: handleGetEconomyStatus,

  // Spawn Subsystem
  create_spawn_subsystem: handleCreateSpawnSubsystem,
  configure_spawn_subsystem: handleConfigureSpawnSubsystem,
  register_spawn_pool: handleRegisterSpawnPool,
  get_spawn_subsystem_status: handleGetSpawnSubsystemStatus,

  // Weather Subsystem
  create_weather_subsystem: handleCreateWeatherSubsystem,
  configure_weather_subsystem: handleConfigureWeatherSubsystem,
  register_weather_zone: handleRegisterWeatherZone,
  get_weather_subsystem_status: handleGetWeatherSubsystemStatus,

  // WorldGen Subsystem
  create_worldgen_subsystem: handleCreateWorldGenSubsystem,
  configure_worldgen_subsystem: handleConfigureWorldGenSubsystem,
  register_worldgen_preset: handleRegisterWorldGenPreset,
  get_worldgen_subsystem_status: handleGetWorldGenSubsystemStatus,

  // Dialogue Subsystem
  create_dialogue_subsystem: handleCreateDialogueSubsystem,
  configure_dialogue_subsystem: handleConfigureDialogueSubsystem,

  // Audio Subsystem
  create_audio_subsystem: handleCreateAudioSubsystem,
  configure_audio_subsystem: handleConfigureAudioSubsystem,

  // Analytics Subsystem
  create_analytics_subsystem: handleCreateAnalyticsSubsystem,
  configure_analytics_subsystem: handleConfigureAnalyticsSubsystem,

  // Query
  list_subsystems: handleListSubsystems,
  get_subsystem_info: handleGetSubsystemInfo
};

export const handleSubsystemTools = createCategoryHandler(Forge2Categories.SUBSYSTEMS, subsystemHandlers);

export default subsystemHandlers;
