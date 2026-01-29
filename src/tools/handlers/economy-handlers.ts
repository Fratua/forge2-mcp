/**
 * FORGE2: Economy System Handlers
 * Complete economy management including currencies, shops, trading, and auction house
 *
 * Supports currency types: standard, premium, event, bound
 * Shop systems with dynamic pricing and reputation integration
 * Trading and auction house for MMO-style games
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

/**
 * Validate required string parameter
 */
function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value;
}

/**
 * Validate optional enum parameter
 */
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

// ============================================================================
// Types
// ============================================================================

export type CurrencyType = 'standard' | 'premium' | 'event' | 'bound' | 'reputation';

export interface CurrencyDefinition {
  currencyId: string;
  currencyName: string;
  currencyType: CurrencyType;
  iconPath?: string;
  maxAmount?: number;
  tradeable?: boolean;
}

export interface ShopItem {
  itemId: string;
  price: number;
  currencyId: string;
  stock?: number;
  refreshRate?: number;
}

// ============================================================================
// Constants
// ============================================================================

const CURRENCY_TYPES = ['standard', 'premium', 'event', 'bound', 'reputation'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a currency type
 */
const handleCreateCurrency: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const currencyId = validateRequiredString(params.currency_id || params.currencyId, 'currency_id');
  const currencyName = validateRequiredString(params.currency_name || params.currencyName, 'currency_name');

  const currencyType = validateOptionalEnum(
    params.currency_type || params.currencyType,
    CURRENCY_TYPES,
    'currency_type'
  );

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'create_currency', {
    currencyId,
    currencyName,
    currencyType: currencyType || 'standard',
    description: params.description as string | undefined,
    iconPath: params.icon_path || params.iconPath,
    maxAmount: params.max_amount || params.maxAmount,
    tradeable: params.tradeable ?? true,
    displayFormat: params.display_format || params.displayFormat,
    decimalPlaces: params.decimal_places ?? params.decimalPlaces ?? 0
  });
};

/**
 * Create a shop/vendor
 */
const handleCreateShop: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const shopId = validateRequiredString(params.shop_id || params.shopId, 'shop_id');
  const shopName = validateRequiredString(params.shop_name || params.shopName, 'shop_name');

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'create_shop', {
    shopId,
    shopName,
    description: params.description as string | undefined,
    vendorNpcId: params.vendor_npc_id || params.vendorNpcId,
    defaultCurrencyId: params.default_currency_id || params.defaultCurrencyId || 'gold',
    buyPriceMultiplier: params.buy_price_multiplier ?? params.buyPriceMultiplier ?? 1.0,
    sellPriceMultiplier: params.sell_price_multiplier ?? params.sellPriceMultiplier ?? 0.5,
    reputationRequired: params.reputation_required || params.reputationRequired,
    levelRequired: params.level_required || params.levelRequired,
    categories: params.categories as string[] | undefined
  });
};

/**
 * Add item to shop inventory
 */
const handleAddShopItem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const shopId = validateRequiredString(params.shop_id || params.shopId, 'shop_id');
  const itemId = validateRequiredString(params.item_id || params.itemId, 'item_id');

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'add_shop_item', {
    shopId,
    itemId,
    price: params.price,
    currencyId: params.currency_id || params.currencyId,
    stock: params.stock,
    maxStock: params.max_stock || params.maxStock,
    refreshTime: params.refresh_time || params.refreshTime,
    reputationDiscount: params.reputation_discount || params.reputationDiscount,
    requiredReputation: params.required_reputation || params.requiredReputation,
    limitPerPlayer: params.limit_per_player || params.limitPerPlayer
  });
};

/**
 * Configure reputation-based pricing
 */
const handleConfigureReputationPricing: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const shopId = validateRequiredString(params.shop_id || params.shopId, 'shop_id');
  const factionId = validateRequiredString(params.faction_id || params.factionId, 'faction_id');

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'configure_reputation_pricing', {
    shopId,
    factionId,
    discountTiers: params.discount_tiers || params.discountTiers,
    exclusiveItems: params.exclusive_items || params.exclusiveItems
  });
};

/**
 * Create trading system
 */
const handleCreateTradingSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'create_trading_system', {
    enabled: params.enabled ?? true,
    tradeRange: params.trade_range || params.tradeRange,
    tradeTax: params.trade_tax ?? params.tradeTax ?? 0,
    taxCurrencyId: params.tax_currency_id || params.taxCurrencyId,
    requireConfirmation: params.require_confirmation ?? params.requireConfirmation ?? true,
    tradeTimeout: params.trade_timeout ?? params.tradeTimeout ?? 60,
    maxItemsPerTrade: params.max_items_per_trade ?? params.maxItemsPerTrade ?? 10,
    allowCurrencyTrade: params.allow_currency_trade ?? params.allowCurrencyTrade ?? true
  });
};

/**
 * Create auction house system
 */
const handleCreateAuctionHouse: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const auctionId = validateRequiredString(params.auction_id || params.auctionId, 'auction_id');

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'create_auction_house', {
    auctionId,
    name: params.name as string | undefined,
    listingFeePercent: params.listing_fee_percent ?? params.listingFeePercent ?? 5,
    saleFeePercent: params.sale_fee_percent ?? params.saleFeePercent ?? 5,
    defaultDuration: params.default_duration ?? params.defaultDuration ?? 24,
    maxDuration: params.max_duration ?? params.maxDuration ?? 48,
    minBidIncrement: params.min_bid_increment ?? params.minBidIncrement ?? 1,
    allowBuyout: params.allow_buyout ?? params.allowBuyout ?? true,
    currencyId: params.currency_id || params.currencyId || 'gold',
    maxActiveListings: params.max_active_listings ?? params.maxActiveListings ?? 50,
    searchCategories: params.search_categories || params.searchCategories
  });
};

/**
 * Configure gold sinks
 */
const handleConfigureGoldSinks: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'configure_gold_sinks', {
    repairCostMultiplier: params.repair_cost_multiplier ?? params.repairCostMultiplier ?? 1.0,
    travelCostBase: params.travel_cost_base ?? params.travelCostBase ?? 10,
    respecCostBase: params.respec_cost_base ?? params.respecCostBase ?? 100,
    respecCostScaling: params.respec_cost_scaling || params.respecCostScaling,
    craftingFees: params.crafting_fees ?? params.craftingFees ?? true,
    auctionFees: params.auction_fees ?? params.auctionFees ?? true,
    guildBankFees: params.guild_bank_fees ?? params.guildBankFees ?? false,
    vanityItemCosts: params.vanity_item_costs || params.vanityItemCosts
  });
};

/**
 * Give currency to player
 */
const handleGiveCurrency: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const currencyId = validateRequiredString(params.currency_id || params.currencyId, 'currency_id');
  const amount = params.amount;

  if (typeof amount !== 'number' || amount <= 0) {
    throw new Error('amount is required and must be a positive number');
  }

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'give_currency', {
    currencyId,
    amount,
    playerId: params.player_id || params.playerId,
    reason: params.reason as string | undefined
  });
};

/**
 * Take currency from player
 */
const handleTakeCurrency: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const currencyId = validateRequiredString(params.currency_id || params.currencyId, 'currency_id');
  const amount = params.amount;

  if (typeof amount !== 'number' || amount <= 0) {
    throw new Error('amount is required and must be a positive number');
  }

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'take_currency', {
    currencyId,
    amount,
    playerId: params.player_id || params.playerId,
    reason: params.reason as string | undefined,
    allowNegative: params.allow_negative ?? params.allowNegative ?? false
  });
};

/**
 * Get player currency
 */
const handleGetPlayerCurrency: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'get_player_currency', {
    playerId: params.player_id || params.playerId,
    currencyId: params.currency_id || params.currencyId
  });
};

/**
 * Buy item from shop
 */
const handleBuyItem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const shopId = validateRequiredString(params.shop_id || params.shopId, 'shop_id');
  const itemId = validateRequiredString(params.item_id || params.itemId, 'item_id');

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'buy_item', {
    shopId,
    itemId,
    quantity: params.quantity ?? 1,
    playerId: params.player_id || params.playerId
  });
};

/**
 * Sell item to shop
 */
const handleSellItem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const shopId = validateRequiredString(params.shop_id || params.shopId, 'shop_id');
  const itemId = validateRequiredString(params.item_id || params.itemId, 'item_id');

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'sell_item', {
    shopId,
    itemId,
    quantity: params.quantity ?? 1,
    playerId: params.player_id || params.playerId
  });
};

/**
 * Create auction listing
 */
const handleCreateAuctionListing: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const auctionId = validateRequiredString(params.auction_id || params.auctionId, 'auction_id');
  const itemId = validateRequiredString(params.item_id || params.itemId, 'item_id');

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'create_auction_listing', {
    auctionId,
    itemId,
    startingBid: params.starting_bid || params.startingBid,
    buyoutPrice: params.buyout_price || params.buyoutPrice,
    duration: params.duration,
    quantity: params.quantity ?? 1,
    playerId: params.player_id || params.playerId
  });
};

/**
 * Place auction bid
 */
const handlePlaceAuctionBid: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const listingId = validateRequiredString(params.listing_id || params.listingId, 'listing_id');
  const bidAmount = params.bid_amount || params.bidAmount;

  if (typeof bidAmount !== 'number' || bidAmount <= 0) {
    throw new Error('bid_amount is required and must be a positive number');
  }

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'place_auction_bid', {
    listingId,
    bidAmount,
    playerId: params.player_id || params.playerId
  });
};

/**
 * Search auction listings
 */
const handleSearchAuction: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const auctionId = validateRequiredString(params.auction_id || params.auctionId, 'auction_id');

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'search_auction', {
    auctionId,
    searchTerm: params.search_term || params.searchTerm,
    category: params.category as string | undefined,
    minPrice: params.min_price || params.minPrice,
    maxPrice: params.max_price || params.maxPrice,
    rarity: params.rarity as string | undefined,
    sortBy: params.sort_by || params.sortBy || 'time_left',
    page: params.page ?? 1,
    pageSize: params.page_size ?? params.pageSize ?? 20
  });
};

/**
 * List all currencies
 */
const handleListCurrencies: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'list_currencies', {
    includeHidden: params.include_hidden ?? params.includeHidden ?? false
  });
};

/**
 * List all shops
 */
const handleListShops: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ECONOMY, 'list_shops', {
    category: params.category as string | undefined
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const economyHandlers: ActionHandlerMap = {
  create_currency: handleCreateCurrency,
  create_shop: handleCreateShop,
  add_shop_item: handleAddShopItem,
  configure_reputation_pricing: handleConfigureReputationPricing,
  create_trading_system: handleCreateTradingSystem,
  create_auction_house: handleCreateAuctionHouse,
  configure_gold_sinks: handleConfigureGoldSinks,
  give_currency: handleGiveCurrency,
  take_currency: handleTakeCurrency,
  get_player_currency: handleGetPlayerCurrency,
  buy_item: handleBuyItem,
  sell_item: handleSellItem,
  create_auction_listing: handleCreateAuctionListing,
  place_auction_bid: handlePlaceAuctionBid,
  search_auction: handleSearchAuction,
  list_currencies: handleListCurrencies,
  list_shops: handleListShops
};

/**
 * Main economy handler - routes to specific action handlers
 */
export const handleEconomyTools = createCategoryHandler(Forge2Categories.ECONOMY, economyHandlers);

/**
 * Get available economy actions
 */
export function getEconomyActions(): string[] {
  return Object.keys(economyHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeEconomyHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = economyHandlers[action];
  if (!handler) {
    throw new Error(`Unknown economy action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
