/**
 * FORGE2: Social Features Handlers
 * Handles friends, presence, voice chat, emotes, player housing, trading
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

// ============================================================================
// Constants
// ============================================================================

const PRESENCE_STATUSES = ['online', 'away', 'busy', 'invisible', 'offline'] as const;
const EMOTE_CATEGORIES = ['greetings', 'dance', 'combat', 'sit', 'expression', 'special'] as const;
const HOUSING_TYPES = ['apartment', 'house', 'guild_hall', 'personal_instance'] as const;

// Reserve constants for future use
void PRESENCE_STATUSES;
void EMOTE_CATEGORIES;
void HOUSING_TYPES;

// ============================================================================
// Friends System Action Handlers
// ============================================================================

/**
 * Create friends system
 */
const handleCreateFriendsSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'create_friends_system', {
    maxFriends: validateOptionalNumber(params.max_friends || params.maxFriends) ?? 200,
    maxBlocked: validateOptionalNumber(params.max_blocked || params.maxBlocked) ?? 50,
    maxPending: validateOptionalNumber(params.max_pending || params.maxPending) ?? 25,
    confirmationRequired: validateOptionalBoolean(params.confirmation_required || params.confirmationRequired) ?? true,
    notesEnabled: validateOptionalBoolean(params.notes_enabled || params.notesEnabled) ?? true,
    groupsEnabled: validateOptionalBoolean(params.groups_enabled || params.groupsEnabled) ?? true
  });
};

/**
 * Configure friend notifications
 */
const handleConfigureFriendNotifications: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'configure_friend_notifications', {
    notifyOnLogin: validateOptionalBoolean(params.notify_on_login || params.notifyOnLogin) ?? true,
    notifyOnLogout: validateOptionalBoolean(params.notify_on_logout || params.notifyOnLogout) ?? false,
    notifyOnLevelUp: validateOptionalBoolean(params.notify_on_level_up || params.notifyOnLevelUp) ?? true,
    notifyOnAchievement: validateOptionalBoolean(params.notify_on_achievement || params.notifyOnAchievement) ?? false,
    notifySound: validateOptionalString(params.notify_sound || params.notifySound),
    consolidateNotifications: validateOptionalBoolean(params.consolidate_notifications || params.consolidateNotifications) ?? true
  });
};

// ============================================================================
// Presence System Action Handlers
// ============================================================================

/**
 * Setup presence system
 */
const handleSetupPresence: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'setup_presence', {
    updateInterval: validateOptionalNumber(params.update_interval || params.updateInterval) ?? 30,
    showActivity: validateOptionalBoolean(params.show_activity || params.showActivity) ?? true,
    showLocation: validateOptionalBoolean(params.show_location || params.showLocation) ?? true,
    showParty: validateOptionalBoolean(params.show_party || params.showParty) ?? true,
    richPresenceEnabled: validateOptionalBoolean(params.rich_presence_enabled || params.richPresenceEnabled) ?? true,
    platformIntegration: validateOptionalBoolean(params.platform_integration || params.platformIntegration) ?? true
  });
};

/**
 * Configure status options
 */
const handleConfigureStatusOptions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'configure_status_options', {
    availableStatuses: validateOptionalArray(params.available_statuses || params.availableStatuses) ?? ['online', 'away', 'busy', 'invisible'],
    customStatusEnabled: validateOptionalBoolean(params.custom_status_enabled || params.customStatusEnabled) ?? true,
    maxStatusLength: validateOptionalNumber(params.max_status_length || params.maxStatusLength) ?? 100,
    autoAwayEnabled: validateOptionalBoolean(params.auto_away_enabled || params.autoAwayEnabled) ?? true,
    autoAwayTime: validateOptionalNumber(params.auto_away_time || params.autoAwayTime) ?? 300
  });
};

// ============================================================================
// Voice Chat Action Handlers
// ============================================================================

/**
 * Configure voice chat
 */
const handleConfigureVoiceChat: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'configure_voice_chat', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    defaultChannel: validateOptionalString(params.default_channel || params.defaultChannel) ?? 'party',
    pushToTalk: validateOptionalBoolean(params.push_to_talk || params.pushToTalk) ?? true,
    voiceActivation: validateOptionalBoolean(params.voice_activation || params.voiceActivation) ?? false,
    spatialAudio: validateOptionalBoolean(params.spatial_audio || params.spatialAudio) ?? true,
    proximityEnabled: validateOptionalBoolean(params.proximity_enabled || params.proximityEnabled) ?? true,
    proximityRadius: validateOptionalNumber(params.proximity_radius || params.proximityRadius) ?? 2000.0
  });
};

/**
 * Setup voice channels
 */
const handleSetupVoiceChannels: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'setup_voice_channels', {
    partyVoice: validateOptionalBoolean(params.party_voice || params.partyVoice) ?? true,
    guildVoice: validateOptionalBoolean(params.guild_voice || params.guildVoice) ?? true,
    raidVoice: validateOptionalBoolean(params.raid_voice || params.raidVoice) ?? true,
    customChannels: validateOptionalBoolean(params.custom_channels || params.customChannels) ?? true,
    maxCustomChannels: validateOptionalNumber(params.max_custom_channels || params.maxCustomChannels) ?? 10
  });
};

// ============================================================================
// Emote System Action Handlers
// ============================================================================

/**
 * Create emote system
 */
const handleCreateEmoteSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'create_emote_system', {
    emoteWheelSize: validateOptionalNumber(params.emote_wheel_size || params.emoteWheelSize) ?? 8,
    quickEmoteSlots: validateOptionalNumber(params.quick_emote_slots || params.quickEmoteSlots) ?? 4,
    emoteQueue: validateOptionalBoolean(params.emote_queue || params.emoteQueue) ?? false,
    emoteCooldown: validateOptionalNumber(params.emote_cooldown || params.emoteCooldown) ?? 1.0,
    interruptible: validateOptionalBoolean(params.interruptible) ?? true
  });
};

/**
 * Add emote
 */
const handleAddEmote: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const emoteName = validateRequiredString(params.emote_name || params.emoteName, 'emote_name');

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'add_emote', {
    emoteName,
    category: validateOptionalString(params.category) ?? 'expression',
    animationPath: validateOptionalString(params.animation_path || params.animationPath),
    soundPath: validateOptionalString(params.sound_path || params.soundPath),
    duration: validateOptionalNumber(params.duration) ?? 2.0,
    looping: validateOptionalBoolean(params.looping) ?? false,
    targetRequired: validateOptionalBoolean(params.target_required || params.targetRequired) ?? false,
    chatMessage: validateOptionalString(params.chat_message || params.chatMessage),
    unlockRequirement: validateOptionalString(params.unlock_requirement || params.unlockRequirement)
  });
};

// ============================================================================
// Player Housing Action Handlers
// ============================================================================

/**
 * Setup player housing
 */
const handleSetupPlayerHousing: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'setup_player_housing', {
    housingType: validateOptionalString(params.housing_type || params.housingType) ?? 'personal_instance',
    visitingEnabled: validateOptionalBoolean(params.visiting_enabled || params.visitingEnabled) ?? true,
    decorationLimit: validateOptionalNumber(params.decoration_limit || params.decorationLimit) ?? 500,
    storageEnabled: validateOptionalBoolean(params.storage_enabled || params.storageEnabled) ?? true,
    guestPermissions: validateOptionalArray(params.guest_permissions || params.guestPermissions) ?? ['view'],
    neighborhoodEnabled: validateOptionalBoolean(params.neighborhood_enabled || params.neighborhoodEnabled) ?? false
  });
};

/**
 * Configure housing permissions
 */
const handleConfigureHousingPermissions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'configure_housing_permissions', {
    defaultVisibility: validateOptionalString(params.default_visibility || params.defaultVisibility) ?? 'friends',
    allowDecorating: validateOptionalArray(params.allow_decorating || params.allowDecorating) ?? ['owner'],
    allowStorage: validateOptionalArray(params.allow_storage || params.allowStorage) ?? ['owner'],
    allowCrafting: validateOptionalArray(params.allow_crafting || params.allowCrafting) ?? ['owner', 'friends'],
    allowTeleport: validateOptionalBoolean(params.allow_teleport || params.allowTeleport) ?? true,
    permissionLevels: validateOptionalArray(params.permission_levels || params.permissionLevels) ?? ['owner', 'friends', 'guild', 'public']
  });
};

// ============================================================================
// Trading System Action Handlers
// ============================================================================

/**
 * Create trading system
 */
const handleCreateTradingSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'create_trading_system', {
    maxTradeSlots: validateOptionalNumber(params.max_trade_slots || params.maxTradeSlots) ?? 6,
    currencyEnabled: validateOptionalBoolean(params.currency_enabled || params.currencyEnabled) ?? true,
    confirmationRequired: validateOptionalBoolean(params.confirmation_required || params.confirmationRequired) ?? true,
    tradeRange: validateOptionalNumber(params.trade_range || params.tradeRange) ?? 500.0,
    tradeTax: validateOptionalNumber(params.trade_tax || params.tradeTax) ?? 0.0,
    antiFraudDelay: validateOptionalNumber(params.anti_fraud_delay || params.antiFraudDelay) ?? 3.0
  });
};

/**
 * Configure trade restrictions
 */
const handleConfigureTradeRestrictions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'configure_trade_restrictions', {
    levelRequirement: validateOptionalNumber(params.level_requirement || params.levelRequirement) ?? 10,
    bindOnTrade: validateOptionalBoolean(params.bind_on_trade || params.bindOnTrade) ?? false,
    untradableTypes: validateOptionalArray(params.untradable_types || params.untradableTypes) ?? ['quest', 'bound'],
    tradeCooldown: validateOptionalNumber(params.trade_cooldown || params.tradeCooldown) ?? 0,
    maxCurrencyPerTrade: validateOptionalNumber(params.max_currency_per_trade || params.maxCurrencyPerTrade)
  });
};

// ============================================================================
// Inspection System Action Handlers
// ============================================================================

/**
 * Setup inspection system
 */
const handleSetupInspectionSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'setup_inspection_system', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    showEquipment: validateOptionalBoolean(params.show_equipment || params.showEquipment) ?? true,
    showStats: validateOptionalBoolean(params.show_stats || params.showStats) ?? true,
    showTalents: validateOptionalBoolean(params.show_talents || params.showTalents) ?? true,
    showAchievements: validateOptionalBoolean(params.show_achievements || params.showAchievements) ?? true,
    showGuild: validateOptionalBoolean(params.show_guild || params.showGuild) ?? true,
    privacyOptions: validateOptionalBoolean(params.privacy_options || params.privacyOptions) ?? true
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get social system info
 */
const handleGetSocialSystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'get_social_system_info', {
    systemType: validateOptionalString(params.system_type || params.systemType),
    includeConfiguration: validateOptionalBoolean(params.include_configuration || params.includeConfiguration) ?? true
  });
};

/**
 * List emotes
 */
const handleListEmotes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.SOCIAL, 'list_emotes', {
    category: validateOptionalString(params.category),
    includeUnlocked: validateOptionalBoolean(params.include_unlocked || params.includeUnlocked) ?? true,
    includeLocked: validateOptionalBoolean(params.include_locked || params.includeLocked) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const socialHandlers: ActionHandlerMap = {
  // Friends
  create_friends_system: handleCreateFriendsSystem,
  configure_friend_notifications: handleConfigureFriendNotifications,

  // Presence
  setup_presence: handleSetupPresence,
  configure_status_options: handleConfigureStatusOptions,

  // Voice Chat
  configure_voice_chat: handleConfigureVoiceChat,
  setup_voice_channels: handleSetupVoiceChannels,

  // Emotes
  create_emote_system: handleCreateEmoteSystem,
  add_emote: handleAddEmote,

  // Housing
  setup_player_housing: handleSetupPlayerHousing,
  configure_housing_permissions: handleConfigureHousingPermissions,

  // Trading
  create_trading_system: handleCreateTradingSystem,
  configure_trade_restrictions: handleConfigureTradeRestrictions,

  // Inspection
  setup_inspection_system: handleSetupInspectionSystem,

  // Query
  get_social_system_info: handleGetSocialSystemInfo,
  list_emotes: handleListEmotes
};

export const handleSocialTools = createCategoryHandler(Forge2Categories.SOCIAL, socialHandlers);

export default socialHandlers;
