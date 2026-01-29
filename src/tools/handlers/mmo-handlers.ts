/**
 * FORGE2: MMO Infrastructure Handlers
 * Handles MMO-specific systems: sharding, instances, guilds, parties, chat, mail
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

const SHARD_TYPES = ['realm', 'channel', 'layer', 'phase'] as const;
const INSTANCE_TYPES = ['dungeon', 'raid', 'arena', 'battleground', 'scenario', 'housing'] as const;
const CHAT_CHANNELS = ['global', 'zone', 'party', 'guild', 'raid', 'whisper', 'trade', 'lfg'] as const;
const GUILD_RANKS = ['leader', 'officer', 'veteran', 'member', 'initiate'] as const;

// Reserve constants for future use
void SHARD_TYPES;
void INSTANCE_TYPES;
void CHAT_CHANNELS;
void GUILD_RANKS;

// ============================================================================
// World Server/Sharding Action Handlers
// ============================================================================

/**
 * Setup world server
 */
const handleSetupWorldServer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const serverName = validateRequiredString(params.server_name || params.serverName, 'server_name');

  return executeForge2Action(tools, Forge2Categories.MMO, 'setup_world_server', {
    serverName,
    maxPlayers: validateOptionalNumber(params.max_players || params.maxPlayers) ?? 5000,
    tickRate: validateOptionalNumber(params.tick_rate || params.tickRate) ?? 30,
    persistentWorld: validateOptionalBoolean(params.persistent_world || params.persistentWorld) ?? true,
    worldPartitionEnabled: validateOptionalBoolean(params.world_partition_enabled || params.worldPartitionEnabled) ?? true,
    streamingEnabled: validateOptionalBoolean(params.streaming_enabled || params.streamingEnabled) ?? true
  });
};

/**
 * Configure sharding
 */
const handleConfigureSharding: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'configure_sharding', {
    shardType: validateOptionalString(params.shard_type || params.shardType) ?? 'channel',
    maxPlayersPerShard: validateOptionalNumber(params.max_players_per_shard || params.maxPlayersPerShard) ?? 200,
    autoBalancing: validateOptionalBoolean(params.auto_balancing || params.autoBalancing) ?? true,
    crossShardEnabled: validateOptionalBoolean(params.cross_shard_enabled || params.crossShardEnabled) ?? true,
    shardMigrationEnabled: validateOptionalBoolean(params.shard_migration_enabled || params.shardMigrationEnabled) ?? true,
    visibilityRadius: validateOptionalNumber(params.visibility_radius || params.visibilityRadius) ?? 5000.0
  });
};

/**
 * Create instance template
 */
const handleCreateInstanceTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const instanceName = validateRequiredString(params.instance_name || params.instanceName, 'instance_name');

  return executeForge2Action(tools, Forge2Categories.MMO, 'create_instance_template', {
    instanceName,
    instanceType: validateOptionalString(params.instance_type || params.instanceType) ?? 'dungeon',
    levelPath: validateOptionalString(params.level_path || params.levelPath),
    minPlayers: validateOptionalNumber(params.min_players || params.minPlayers) ?? 1,
    maxPlayers: validateOptionalNumber(params.max_players || params.maxPlayers) ?? 5,
    levelRequirement: validateOptionalNumber(params.level_requirement || params.levelRequirement),
    lockoutDuration: validateOptionalNumber(params.lockout_duration || params.lockoutDuration) ?? 0,
    resetInterval: validateOptionalNumber(params.reset_interval || params.resetInterval) ?? 86400,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Configure instance settings
 */
const handleConfigureInstanceSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const instancePath = validateRequiredString(params.instance_path || params.instancePath, 'instance_path');

  return executeForge2Action(tools, Forge2Categories.MMO, 'configure_instance_settings', {
    instancePath,
    difficultyModes: validateOptionalArray(params.difficulty_modes || params.difficultyModes),
    lootLockout: validateOptionalBoolean(params.loot_lockout || params.lootLockout) ?? true,
    savedProgress: validateOptionalBoolean(params.saved_progress || params.savedProgress) ?? true,
    teleportOnComplete: validateOptionalBoolean(params.teleport_on_complete || params.teleportOnComplete) ?? true,
    timeLimit: validateOptionalNumber(params.time_limit || params.timeLimit)
  });
};

// ============================================================================
// Login/Character Server Action Handlers
// ============================================================================

/**
 * Setup login server
 */
const handleSetupLoginServer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'setup_login_server', {
    authenticationMethod: validateOptionalString(params.authentication_method || params.authenticationMethod) ?? 'session',
    sessionTimeout: validateOptionalNumber(params.session_timeout || params.sessionTimeout) ?? 3600,
    maxCharactersPerAccount: validateOptionalNumber(params.max_characters_per_account || params.maxCharactersPerAccount) ?? 10,
    characterDeleteDelay: validateOptionalNumber(params.character_delete_delay || params.characterDeleteDelay) ?? 86400,
    queueEnabled: validateOptionalBoolean(params.queue_enabled || params.queueEnabled) ?? true
  });
};

/**
 * Configure character select
 */
const handleConfigureCharacterSelect: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'configure_character_select', {
    showLevel: validateOptionalBoolean(params.show_level || params.showLevel) ?? true,
    showGuild: validateOptionalBoolean(params.show_guild || params.showGuild) ?? true,
    showLocation: validateOptionalBoolean(params.show_location || params.showLocation) ?? true,
    showPlaytime: validateOptionalBoolean(params.show_playtime || params.showPlaytime) ?? false,
    characterSortOrder: validateOptionalString(params.character_sort_order || params.characterSortOrder) ?? 'last_played',
    previewEnvironment: validateOptionalString(params.preview_environment || params.previewEnvironment)
  });
};

// ============================================================================
// Chat System Action Handlers
// ============================================================================

/**
 * Configure chat system
 */
const handleConfigureChatSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'configure_chat_system', {
    enabledChannels: validateOptionalArray(params.enabled_channels || params.enabledChannels) ?? ['global', 'zone', 'party', 'guild', 'whisper'],
    profanityFilter: validateOptionalBoolean(params.profanity_filter || params.profanityFilter) ?? true,
    spamProtection: validateOptionalBoolean(params.spam_protection || params.spamProtection) ?? true,
    messageRateLimit: validateOptionalNumber(params.message_rate_limit || params.messageRateLimit) ?? 5,
    historyLength: validateOptionalNumber(params.history_length || params.historyLength) ?? 100,
    crossRealmChat: validateOptionalBoolean(params.cross_realm_chat || params.crossRealmChat) ?? false
  });
};

/**
 * Create chat channel
 */
const handleCreateChatChannel: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const channelName = validateRequiredString(params.channel_name || params.channelName, 'channel_name');

  return executeForge2Action(tools, Forge2Categories.MMO, 'create_chat_channel', {
    channelName,
    channelType: validateOptionalString(params.channel_type || params.channelType) ?? 'custom',
    passwordProtected: validateOptionalBoolean(params.password_protected || params.passwordProtected) ?? false,
    moderated: validateOptionalBoolean(params.moderated) ?? false,
    persistent: validateOptionalBoolean(params.persistent) ?? false,
    maxMembers: validateOptionalNumber(params.max_members || params.maxMembers) ?? 100
  });
};

// ============================================================================
// Mail System Action Handlers
// ============================================================================

/**
 * Setup mail system
 */
const handleSetupMailSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'setup_mail_system', {
    maxMailboxSize: validateOptionalNumber(params.max_mailbox_size || params.maxMailboxSize) ?? 100,
    attachmentLimit: validateOptionalNumber(params.attachment_limit || params.attachmentLimit) ?? 12,
    currencyLimit: validateOptionalNumber(params.currency_limit || params.currencyLimit) ?? 1000000,
    expirationDays: validateOptionalNumber(params.expiration_days || params.expirationDays) ?? 30,
    sendCost: validateOptionalNumber(params.send_cost || params.sendCost) ?? 30,
    codEnabled: validateOptionalBoolean(params.cod_enabled || params.codEnabled) ?? true
  });
};

/**
 * Configure mail notifications
 */
const handleConfigureMailNotifications: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'configure_mail_notifications', {
    notifyOnReceive: validateOptionalBoolean(params.notify_on_receive || params.notifyOnReceive) ?? true,
    notifySound: validateOptionalString(params.notify_sound || params.notifySound),
    showPreview: validateOptionalBoolean(params.show_preview || params.showPreview) ?? true,
    autoOpenOnLogin: validateOptionalBoolean(params.auto_open_on_login || params.autoOpenOnLogin) ?? false
  });
};

// ============================================================================
// Guild System Action Handlers
// ============================================================================

/**
 * Create guild system
 */
const handleCreateGuildSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'create_guild_system', {
    maxMembers: validateOptionalNumber(params.max_members || params.maxMembers) ?? 500,
    creationCost: validateOptionalNumber(params.creation_cost || params.creationCost) ?? 1000,
    minMembersToCreate: validateOptionalNumber(params.min_members_to_create || params.minMembersToCreate) ?? 1,
    defaultRanks: validateOptionalArray(params.default_ranks || params.defaultRanks) ?? ['Leader', 'Officer', 'Member'],
    guildBankEnabled: validateOptionalBoolean(params.guild_bank_enabled || params.guildBankEnabled) ?? true,
    guildBankTabs: validateOptionalNumber(params.guild_bank_tabs || params.guildBankTabs) ?? 8
  });
};

/**
 * Configure guild permissions
 */
const handleConfigureGuildPermissions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'configure_guild_permissions', {
    permissions: validateOptionalArray(params.permissions) ?? [],
    rankPermissions: validateOptionalObject(params.rank_permissions || params.rankPermissions),
    defaultOfficerPermissions: validateOptionalArray(params.default_officer_permissions || params.defaultOfficerPermissions),
    defaultMemberPermissions: validateOptionalArray(params.default_member_permissions || params.defaultMemberPermissions)
  });
};

/**
 * Setup guild features
 */
const handleSetupGuildFeatures: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'setup_guild_features', {
    motdEnabled: validateOptionalBoolean(params.motd_enabled || params.motdEnabled) ?? true,
    newsEnabled: validateOptionalBoolean(params.news_enabled || params.newsEnabled) ?? true,
    calendarEnabled: validateOptionalBoolean(params.calendar_enabled || params.calendarEnabled) ?? true,
    achievementsEnabled: validateOptionalBoolean(params.achievements_enabled || params.achievementsEnabled) ?? true,
    perksEnabled: validateOptionalBoolean(params.perks_enabled || params.perksEnabled) ?? true,
    levelsEnabled: validateOptionalBoolean(params.levels_enabled || params.levelsEnabled) ?? true
  });
};

// ============================================================================
// Party System Action Handlers
// ============================================================================

/**
 * Configure party system
 */
const handleConfigurePartySystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'configure_party_system', {
    maxPartySize: validateOptionalNumber(params.max_party_size || params.maxPartySize) ?? 5,
    maxRaidSize: validateOptionalNumber(params.max_raid_size || params.maxRaidSize) ?? 40,
    lootMethods: validateOptionalArray(params.loot_methods || params.lootMethods) ?? ['group', 'need_before_greed', 'master_looter', 'round_robin'],
    defaultLootMethod: validateOptionalString(params.default_loot_method || params.defaultLootMethod) ?? 'group',
    roleCheckEnabled: validateOptionalBoolean(params.role_check_enabled || params.roleCheckEnabled) ?? true,
    readyCheckEnabled: validateOptionalBoolean(params.ready_check_enabled || params.readyCheckEnabled) ?? true
  });
};

/**
 * Setup group finder
 */
const handleSetupGroupFinder: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'setup_group_finder', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    activities: validateOptionalArray(params.activities) ?? [],
    roleRequirements: validateOptionalObject(params.role_requirements || params.roleRequirements),
    queueTimeout: validateOptionalNumber(params.queue_timeout || params.queueTimeout) ?? 3600,
    crossRealmEnabled: validateOptionalBoolean(params.cross_realm_enabled || params.crossRealmEnabled) ?? true,
    ilvlRequirements: validateOptionalBoolean(params.ilvl_requirements || params.ilvlRequirements) ?? true
  });
};

// ============================================================================
// Raid System Action Handlers
// ============================================================================

/**
 * Setup raid system
 */
const handleSetupRaidSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'setup_raid_system', {
    maxRaidGroups: validateOptionalNumber(params.max_raid_groups || params.maxRaidGroups) ?? 8,
    groupSize: validateOptionalNumber(params.group_size || params.groupSize) ?? 5,
    markersEnabled: validateOptionalBoolean(params.markers_enabled || params.markersEnabled) ?? true,
    markerCount: validateOptionalNumber(params.marker_count || params.markerCount) ?? 8,
    assistEnabled: validateOptionalBoolean(params.assist_enabled || params.assistEnabled) ?? true,
    mainTankEnabled: validateOptionalBoolean(params.main_tank_enabled || params.mainTankEnabled) ?? true
  });
};

/**
 * Configure raid lockout
 */
const handleConfigureRaidLockout: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const instancePath = validateRequiredString(params.instance_path || params.instancePath, 'instance_path');

  return executeForge2Action(tools, Forge2Categories.MMO, 'configure_raid_lockout', {
    instancePath,
    lockoutType: validateOptionalString(params.lockout_type || params.lockoutType) ?? 'weekly',
    lockoutDuration: validateOptionalNumber(params.lockout_duration || params.lockoutDuration) ?? 604800,
    extendable: validateOptionalBoolean(params.extendable) ?? true,
    perDifficulty: validateOptionalBoolean(params.per_difficulty || params.perDifficulty) ?? true,
    resetDay: validateOptionalNumber(params.reset_day || params.resetDay) ?? 2
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get MMO system info
 */
const handleGetMMOSystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'get_mmo_system_info', {
    systemType: validateOptionalString(params.system_type || params.systemType),
    includeConfiguration: validateOptionalBoolean(params.include_configuration || params.includeConfiguration) ?? true,
    includeStatistics: validateOptionalBoolean(params.include_statistics || params.includeStatistics) ?? false
  });
};

/**
 * List instance templates
 */
const handleListInstanceTemplates: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MMO, 'list_instance_templates', {
    instanceType: validateOptionalString(params.instance_type || params.instanceType),
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const mmoHandlers: ActionHandlerMap = {
  // World Server/Sharding
  setup_world_server: handleSetupWorldServer,
  configure_sharding: handleConfigureSharding,
  create_instance_template: handleCreateInstanceTemplate,
  configure_instance_settings: handleConfigureInstanceSettings,

  // Login/Character
  setup_login_server: handleSetupLoginServer,
  configure_character_select: handleConfigureCharacterSelect,

  // Chat
  configure_chat_system: handleConfigureChatSystem,
  create_chat_channel: handleCreateChatChannel,

  // Mail
  setup_mail_system: handleSetupMailSystem,
  configure_mail_notifications: handleConfigureMailNotifications,

  // Guild
  create_guild_system: handleCreateGuildSystem,
  configure_guild_permissions: handleConfigureGuildPermissions,
  setup_guild_features: handleSetupGuildFeatures,

  // Party
  configure_party_system: handleConfigurePartySystem,
  setup_group_finder: handleSetupGroupFinder,

  // Raid
  setup_raid_system: handleSetupRaidSystem,
  configure_raid_lockout: handleConfigureRaidLockout,

  // Query
  get_mmo_system_info: handleGetMMOSystemInfo,
  list_instance_templates: handleListInstanceTemplates
};

export const handleMMOTools = createCategoryHandler(Forge2Categories.MMO, mmoHandlers);

export default mmoHandlers;
