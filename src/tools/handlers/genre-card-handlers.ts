/**
 * Genre-specific handlers for Card/Deck-building games (Hearthstone, Slay the Spire style)
 * Handles cards, decks, turn systems, mana, and card effects
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

const CARD_TYPES = ['minion', 'spell', 'weapon', 'artifact', 'hero', 'power', 'curse', 'status'] as const;
const CARD_RARITIES = ['common', 'uncommon', 'rare', 'epic', 'legendary', 'token'] as const;
const CARD_CLASSES = ['neutral', 'warrior', 'mage', 'rogue', 'hunter', 'priest', 'paladin', 'warlock', 'druid', 'shaman', 'demon_hunter'] as const;
const EFFECT_TYPES = ['battlecry', 'deathrattle', 'aura', 'triggered', 'passive', 'ongoing', 'combo', 'choose_one'] as const;
const TARGET_TYPES = ['none', 'minion', 'enemy_minion', 'friendly_minion', 'hero', 'enemy_hero', 'any', 'random', 'all', 'adjacent'] as const;
const KEYWORD_TYPES = ['taunt', 'charge', 'divine_shield', 'windfury', 'stealth', 'lifesteal', 'rush', 'reborn', 'poisonous', 'spell_damage', 'overload'] as const;
const GAME_MODES = ['standard', 'wild', 'arena', 'draft', 'roguelike', 'puzzle'] as const;
const TURN_PHASES = ['start', 'draw', 'main', 'combat', 'end'] as const;
const ZONE_TYPES = ['deck', 'hand', 'board', 'graveyard', 'exile', 'secret'] as const;

// Reserve constants for future validation (type validation uses arrays directly)
void TURN_PHASES;
void ZONE_TYPES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a new card
 */
const handleCreateCard: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const cardId = validateRequiredString(params.card_id || params.cardId, 'card_id');
  const cardName = validateRequiredString(params.card_name || params.cardName, 'card_name');
  const cardType = validateRequiredEnum(params.card_type || params.cardType, CARD_TYPES, 'card_type');
  const cost = validateOptionalNumber(params.cost) ?? 0;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'create_card', {
    cardId,
    cardName,
    cardType,
    cost,
    rarity: validateOptionalEnum(params.rarity, CARD_RARITIES) ?? 'common',
    cardClass: validateOptionalEnum(params.card_class || params.cardClass, CARD_CLASSES) ?? 'neutral',
    description: validateOptionalString(params.description),
    flavorText: validateOptionalString(params.flavor_text || params.flavorText),
    artPath: validateOptionalString(params.art_path || params.artPath),
    attack: validateOptionalNumber(params.attack), // For minions/weapons
    health: validateOptionalNumber(params.health), // For minions
    durability: validateOptionalNumber(params.durability), // For weapons
    collectible: validateOptionalBoolean(params.collectible) ?? true,
    keywords: validateOptionalArray(params.keywords),
    tribe: validateOptionalString(params.tribe), // Beast, Dragon, Murloc, etc.
    spellSchool: validateOptionalString(params.spell_school || params.spellSchool) // Fire, Frost, Nature, etc.
  });
};

/**
 * Add effect to a card
 */
const handleAddCardEffect: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const cardId = validateRequiredString(params.card_id || params.cardId, 'card_id');
  const effectType = validateRequiredEnum(params.effect_type || params.effectType, EFFECT_TYPES, 'effect_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'add_card_effect', {
    cardId,
    effectType,
    effectId: validateOptionalString(params.effect_id || params.effectId),
    targetType: validateOptionalEnum(params.target_type || params.targetType, TARGET_TYPES),
    effectValue: validateOptionalNumber(params.effect_value || params.effectValue),
    effectAction: validateOptionalString(params.effect_action || params.effectAction), // 'deal_damage', 'draw_cards', 'summon', etc.
    effectParams: validateOptionalObject(params.effect_params || params.effectParams),
    condition: validateOptionalString(params.condition),
    triggerEvent: validateOptionalString(params.trigger_event || params.triggerEvent)
  });
};

/**
 * Configure deck rules
 */
const handleConfigureDeckRules: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'configure_deck_rules', {
    minDeckSize: validateOptionalNumber(params.min_deck_size || params.minDeckSize) ?? 30,
    maxDeckSize: validateOptionalNumber(params.max_deck_size || params.maxDeckSize) ?? 30,
    maxCopiesPerCard: validateOptionalNumber(params.max_copies_per_card || params.maxCopiesPerCard) ?? 2,
    maxLegendaryCopies: validateOptionalNumber(params.max_legendary_copies || params.maxLegendaryCopies) ?? 1,
    classRestriction: validateOptionalBoolean(params.class_restriction || params.classRestriction) ?? true,
    allowNeutral: validateOptionalBoolean(params.allow_neutral || params.allowNeutral) ?? true,
    bannedCards: validateOptionalArray(params.banned_cards || params.bannedCards),
    requiredCards: validateOptionalArray(params.required_cards || params.requiredCards)
  });
};

/**
 * Create a deck
 */
const handleCreateDeck: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const deckId = validateRequiredString(params.deck_id || params.deckId, 'deck_id');
  const deckName = validateRequiredString(params.deck_name || params.deckName, 'deck_name');
  const deckClass = validateRequiredEnum(params.deck_class || params.deckClass, CARD_CLASSES, 'deck_class');

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'create_deck', {
    deckId,
    deckName,
    deckClass,
    cards: validateOptionalArray(params.cards) ?? [], // Array of { cardId, count }
    heroCard: validateOptionalString(params.hero_card || params.heroCard),
    heroPower: validateOptionalString(params.hero_power || params.heroPower),
    format: validateOptionalEnum(params.format, GAME_MODES)
  });
};

/**
 * Add cards to deck
 */
const handleAddCardsToDeck: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const deckId = validateRequiredString(params.deck_id || params.deckId, 'deck_id');
  const cards = validateOptionalArray(params.cards) ?? [];

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'add_cards_to_deck', {
    deckId,
    cards, // Array of { cardId, count }
    validateDeck: validateOptionalBoolean(params.validate_deck || params.validateDeck) ?? true
  });
};

/**
 * Setup turn system
 */
const handleSetupTurnSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'setup_turn_system', {
    startingHandSize: validateOptionalNumber(params.starting_hand_size || params.startingHandSize) ?? 3,
    mulliganEnabled: validateOptionalBoolean(params.mulligan_enabled || params.mulliganEnabled) ?? true,
    drawPerTurn: validateOptionalNumber(params.draw_per_turn || params.drawPerTurn) ?? 1,
    maxHandSize: validateOptionalNumber(params.max_hand_size || params.maxHandSize) ?? 10,
    burnExcessCards: validateOptionalBoolean(params.burn_excess_cards || params.burnExcessCards) ?? true,
    turnTimeLimit: validateOptionalNumber(params.turn_time_limit || params.turnTimeLimit) ?? 75,
    ropeWarningTime: validateOptionalNumber(params.rope_warning_time || params.ropeWarningTime) ?? 15,
    simultaneousTurns: validateOptionalBoolean(params.simultaneous_turns || params.simultaneousTurns) ?? false
  });
};

/**
 * Create mana system
 */
const handleCreateManaSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'create_mana_system', {
    startingMana: validateOptionalNumber(params.starting_mana || params.startingMana) ?? 0,
    manaPerTurn: validateOptionalNumber(params.mana_per_turn || params.manaPerTurn) ?? 1,
    maxMana: validateOptionalNumber(params.max_mana || params.maxMana) ?? 10,
    refillOnTurnStart: validateOptionalBoolean(params.refill_on_turn_start || params.refillOnTurnStart) ?? true,
    excessManaDraws: validateOptionalBoolean(params.excess_mana_draws || params.excessManaDraws) ?? false,
    manaColors: validateOptionalArray(params.mana_colors || params.manaColors), // For MTG-style colored mana
    overloadEnabled: validateOptionalBoolean(params.overload_enabled || params.overloadEnabled) ?? false,
    rampEnabled: validateOptionalBoolean(params.ramp_enabled || params.rampEnabled) ?? true
  });
};

/**
 * Configure card effects
 */
const handleConfigureCardEffects: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'configure_card_effects', {
    damageEffect: validateOptionalObject(params.damage_effect || params.damageEffect),
    healEffect: validateOptionalObject(params.heal_effect || params.healEffect),
    drawEffect: validateOptionalObject(params.draw_effect || params.drawEffect),
    summonEffect: validateOptionalObject(params.summon_effect || params.summonEffect),
    buffEffect: validateOptionalObject(params.buff_effect || params.buffEffect),
    destroyEffect: validateOptionalObject(params.destroy_effect || params.destroyEffect),
    silenceEffect: validateOptionalObject(params.silence_effect || params.silenceEffect),
    discoverEffect: validateOptionalObject(params.discover_effect || params.discoverEffect),
    transformEffect: validateOptionalObject(params.transform_effect || params.transformEffect)
  });
};

/**
 * Configure keyword abilities
 */
const handleConfigureKeywords: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const keyword = validateRequiredEnum(params.keyword, KEYWORD_TYPES, 'keyword');

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'configure_keyword', {
    keyword,
    description: validateOptionalString(params.description),
    stackable: validateOptionalBoolean(params.stackable) ?? false,
    persistOnTransform: validateOptionalBoolean(params.persist_on_transform || params.persistOnTransform) ?? false,
    visualEffect: validateOptionalString(params.visual_effect || params.visualEffect),
    audioEffect: validateOptionalString(params.audio_effect || params.audioEffect)
  });
};

/**
 * Setup deck builder
 */
const handleSetupDeckBuilder: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'setup_deck_builder', {
    filterByClass: validateOptionalBoolean(params.filter_by_class || params.filterByClass) ?? true,
    filterByMana: validateOptionalBoolean(params.filter_by_mana || params.filterByMana) ?? true,
    filterByRarity: validateOptionalBoolean(params.filter_by_rarity || params.filterByRarity) ?? true,
    filterByType: validateOptionalBoolean(params.filter_by_type || params.filterByType) ?? true,
    searchEnabled: validateOptionalBoolean(params.search_enabled || params.searchEnabled) ?? true,
    sortOptions: validateOptionalArray(params.sort_options || params.sortOptions) ?? ['mana', 'name', 'rarity', 'type'],
    showManaCurve: validateOptionalBoolean(params.show_mana_curve || params.showManaCurve) ?? true,
    showCardCount: validateOptionalBoolean(params.show_card_count || params.showCardCount) ?? true,
    autoComplete: validateOptionalBoolean(params.auto_complete || params.autoComplete) ?? false,
    importExport: validateOptionalBoolean(params.import_export || params.importExport) ?? true
  });
};

/**
 * Configure board layout
 */
const handleConfigureBoardLayout: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'configure_board_layout', {
    boardSlots: validateOptionalNumber(params.board_slots || params.boardSlots) ?? 7,
    heroPosition: validateOptionalString(params.hero_position || params.heroPosition) ?? 'center',
    weaponSlot: validateOptionalBoolean(params.weapon_slot || params.weaponSlot) ?? true,
    secretSlots: validateOptionalNumber(params.secret_slots || params.secretSlots) ?? 5,
    questSlot: validateOptionalBoolean(params.quest_slot || params.questSlot) ?? true,
    heroPowerPosition: validateOptionalString(params.hero_power_position || params.heroPowerPosition),
    graveyardVisible: validateOptionalBoolean(params.graveyard_visible || params.graveyardVisible) ?? true,
    deckCountVisible: validateOptionalBoolean(params.deck_count_visible || params.deckCountVisible) ?? true
  });
};

/**
 * Configure roguelike mode (Slay the Spire style)
 */
const handleConfigureRoguelikeMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'configure_roguelike_mode', {
    startingDeckSize: validateOptionalNumber(params.starting_deck_size || params.startingDeckSize) ?? 10,
    startingHealth: validateOptionalNumber(params.starting_health || params.startingHealth) ?? 80,
    cardRewardsPerCombat: validateOptionalNumber(params.card_rewards_per_combat || params.cardRewardsPerCombat) ?? 3,
    goldPerCombat: validateOptionalNumber(params.gold_per_combat || params.goldPerCombat) ?? 25,
    relicsEnabled: validateOptionalBoolean(params.relics_enabled || params.relicsEnabled) ?? true,
    potionsEnabled: validateOptionalBoolean(params.potions_enabled || params.potionsEnabled) ?? true,
    restSitesEnabled: validateOptionalBoolean(params.rest_sites_enabled || params.restSitesEnabled) ?? true,
    shopEnabled: validateOptionalBoolean(params.shop_enabled || params.shopEnabled) ?? true,
    eliteEncounters: validateOptionalBoolean(params.elite_encounters || params.eliteEncounters) ?? true,
    bossEveryNFloors: validateOptionalNumber(params.boss_every_n_floors || params.bossEveryNFloors) ?? 17,
    cardRemovalCost: validateOptionalNumber(params.card_removal_cost || params.cardRemovalCost) ?? 75
  });
};

/**
 * Get game state
 */
const handleGetGameState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'get_game_state', {
    includeHands: validateOptionalBoolean(params.include_hands || params.includeHands) ?? false,
    includeBoard: validateOptionalBoolean(params.include_board || params.includeBoard) ?? true,
    includeGraveyard: validateOptionalBoolean(params.include_graveyard || params.includeGraveyard) ?? false,
    includeSecrets: validateOptionalBoolean(params.include_secrets || params.includeSecrets) ?? false
  });
};

/**
 * Play a card
 */
const handlePlayCard: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const cardIndex = validateOptionalNumber(params.card_index || params.cardIndex) ?? 0;
  const playerId = validateRequiredString(params.player_id || params.playerId, 'player_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'play_card', {
    playerId,
    cardIndex,
    targetId: validateOptionalString(params.target_id || params.targetId),
    boardPosition: validateOptionalNumber(params.board_position || params.boardPosition),
    chooseOneOption: validateOptionalNumber(params.choose_one_option || params.chooseOneOption)
  });
};

/**
 * List all cards
 */
const handleListCards: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'list_cards', {
    filterType: validateOptionalEnum(params.filter_type || params.filterType, CARD_TYPES),
    filterClass: validateOptionalEnum(params.filter_class || params.filterClass, CARD_CLASSES),
    filterRarity: validateOptionalEnum(params.filter_rarity || params.filterRarity, CARD_RARITIES),
    filterCollectible: validateOptionalBoolean(params.filter_collectible || params.filterCollectible),
    includeEffects: validateOptionalBoolean(params.include_effects || params.includeEffects) ?? false
  });
};

/**
 * List all decks
 */
const handleListDecks: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_CARD, 'list_decks', {
    filterClass: validateOptionalEnum(params.filter_class || params.filterClass, CARD_CLASSES),
    filterFormat: validateOptionalEnum(params.filter_format || params.filterFormat, GAME_MODES),
    includeCardList: validateOptionalBoolean(params.include_card_list || params.includeCardList) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const genreCardHandlers: ActionHandlerMap = {
  // Card creation
  create_card: handleCreateCard,
  add_card_effect: handleAddCardEffect,
  configure_keywords: handleConfigureKeywords,

  // Deck management
  configure_deck_rules: handleConfigureDeckRules,
  create_deck: handleCreateDeck,
  add_cards_to_deck: handleAddCardsToDeck,
  setup_deck_builder: handleSetupDeckBuilder,

  // Game systems
  setup_turn_system: handleSetupTurnSystem,
  create_mana_system: handleCreateManaSystem,
  configure_card_effects: handleConfigureCardEffects,
  configure_board_layout: handleConfigureBoardLayout,

  // Roguelike
  configure_roguelike_mode: handleConfigureRoguelikeMode,

  // Query
  get_game_state: handleGetGameState,
  play_card: handlePlayCard,
  list_cards: handleListCards,
  list_decks: handleListDecks
};

// Create the main handler function for this category
export const handleGenreCardTools = createCategoryHandler(Forge2Categories.GENRE_CARD, genreCardHandlers);

export default genreCardHandlers;
