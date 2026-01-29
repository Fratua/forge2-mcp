/**
 * FORGE2: Data Asset Handlers
 * Handles primary data assets, asset manager configuration, and asset registry
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
// Data Asset Action Handlers
// ============================================================================

/**
 * Create data asset
 */
const handleCreateDataAsset: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetName = validateRequiredString(params.asset_name || params.assetName, 'asset_name');
  const assetClass = validateRequiredString(params.asset_class || params.assetClass, 'asset_class');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'create_data_asset', {
    assetName,
    assetClass,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Data/Assets/${assetName}`,
    initialData: validateOptionalObject(params.initial_data || params.initialData)
  });
};

/**
 * Configure data asset
 */
const handleConfigureDataAsset: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetPath = validateRequiredString(params.asset_path || params.assetPath, 'asset_path');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'configure_data_asset', {
    assetPath,
    properties: validateOptionalObject(params.properties) ?? {},
    tags: validateOptionalArray(params.tags) ?? [],
    metadata: validateOptionalObject(params.metadata)
  });
};

/**
 * Duplicate data asset
 */
const handleDuplicateDataAsset: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sourceAsset = validateRequiredString(params.source_asset || params.sourceAsset, 'source_asset');
  const newName = validateRequiredString(params.new_name || params.newName, 'new_name');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'duplicate_data_asset', {
    sourceAsset,
    newName,
    savePath: validateOptionalString(params.save_path || params.savePath),
    overrideProperties: validateOptionalObject(params.override_properties || params.overrideProperties)
  });
};

// ============================================================================
// Asset Manager Action Handlers
// ============================================================================

/**
 * Configure asset manager
 */
const handleConfigureAssetManager: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'configure_asset_manager', {
    primaryAssetTypes: validateOptionalArray(params.primary_asset_types || params.primaryAssetTypes) ?? [],
    assetBundles: validateOptionalArray(params.asset_bundles || params.assetBundles) ?? [],
    defaultBundleState: validateOptionalString(params.default_bundle_state || params.defaultBundleState) ?? 'unloaded'
  });
};

/**
 * Register primary asset type
 */
const handleRegisterPrimaryAssetType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const typeName = validateRequiredString(params.type_name || params.typeName, 'type_name');
  const assetClass = validateRequiredString(params.asset_class || params.assetClass, 'asset_class');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'register_primary_asset_type', {
    typeName,
    assetClass,
    directories: validateOptionalArray(params.directories) ?? [],
    hasBlueprintClass: validateOptionalBoolean(params.has_blueprint_class || params.hasBlueprintClass) ?? false,
    isEditorOnly: validateOptionalBoolean(params.is_editor_only || params.isEditorOnly) ?? false,
    rules: validateOptionalObject(params.rules)
  });
};

/**
 * Create asset bundle
 */
const handleCreateAssetBundle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const bundleName = validateRequiredString(params.bundle_name || params.bundleName, 'bundle_name');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'create_asset_bundle', {
    bundleName,
    assets: validateOptionalArray(params.assets) ?? [],
    loadPriority: validateOptionalNumber(params.load_priority || params.loadPriority) ?? 0,
    alwaysLoaded: validateOptionalBoolean(params.always_loaded || params.alwaysLoaded) ?? false,
    preloadInEditor: validateOptionalBoolean(params.preload_in_editor || params.preloadInEditor) ?? false
  });
};

/**
 * Configure bundle rules
 */
const handleConfigureBundleRules: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const bundleName = validateRequiredString(params.bundle_name || params.bundleName, 'bundle_name');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'configure_bundle_rules', {
    bundleName,
    chunkId: validateOptionalNumber(params.chunk_id || params.chunkId),
    cookRule: validateOptionalString(params.cook_rule || params.cookRule) ?? 'always_cook',
    applyRecursively: validateOptionalBoolean(params.apply_recursively || params.applyRecursively) ?? true
  });
};

// ============================================================================
// Asset Registry Action Handlers
// ============================================================================

/**
 * Setup asset registry
 */
const handleSetupAssetRegistry: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'setup_asset_registry', {
    scanPaths: validateOptionalArray(params.scan_paths || params.scanPaths) ?? ['/Game'],
    asyncScan: validateOptionalBoolean(params.async_scan || params.asyncScan) ?? true,
    cacheEnabled: validateOptionalBoolean(params.cache_enabled || params.cacheEnabled) ?? true
  });
};

/**
 * Add asset tag
 */
const handleAddAssetTag: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetPath = validateRequiredString(params.asset_path || params.assetPath, 'asset_path');
  const tagKey = validateRequiredString(params.tag_key || params.tagKey, 'tag_key');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'add_asset_tag', {
    assetPath,
    tagKey,
    tagValue: validateOptionalString(params.tag_value || params.tagValue) ?? '',
    saveAsset: validateOptionalBoolean(params.save_asset || params.saveAsset) ?? true
  });
};

/**
 * Search assets by tag
 */
const handleSearchAssetsByTag: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'search_assets_by_tag', {
    tagKey: validateOptionalString(params.tag_key || params.tagKey),
    tagValue: validateOptionalString(params.tag_value || params.tagValue),
    assetClass: validateOptionalString(params.asset_class || params.assetClass),
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game',
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Soft Reference Action Handlers
// ============================================================================

/**
 * Create soft reference
 */
const handleCreateSoftReference: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const targetAsset = validateRequiredString(params.target_asset || params.targetAsset, 'target_asset');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'create_soft_reference', {
    targetAsset,
    referenceType: validateOptionalString(params.reference_type || params.referenceType) ?? 'soft_object'
  });
};

/**
 * Configure async loading
 */
const handleConfigureAsyncLoading: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetPath = validateRequiredString(params.asset_path || params.assetPath, 'asset_path');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'configure_async_loading', {
    assetPath,
    loadPriority: validateOptionalNumber(params.load_priority || params.loadPriority) ?? 0,
    keepInMemory: validateOptionalBoolean(params.keep_in_memory || params.keepInMemory) ?? false,
    loadAsync: validateOptionalBoolean(params.load_async || params.loadAsync) ?? true,
    unloadDelay: validateOptionalNumber(params.unload_delay || params.unloadDelay) ?? 30.0
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get asset info
 */
const handleGetAssetInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetPath = validateRequiredString(params.asset_path || params.assetPath, 'asset_path');

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'get_asset_info', {
    assetPath,
    includeProperties: validateOptionalBoolean(params.include_properties || params.includeProperties) ?? true,
    includeTags: validateOptionalBoolean(params.include_tags || params.includeTags) ?? true,
    includeDependencies: validateOptionalBoolean(params.include_dependencies || params.includeDependencies) ?? false
  });
};

/**
 * List data assets
 */
const handleListDataAssets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'list_data_assets', {
    assetClass: validateOptionalString(params.asset_class || params.assetClass),
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game/Data',
    recursive: validateOptionalBoolean(params.recursive) ?? true,
    includeBlueprints: validateOptionalBoolean(params.include_blueprints || params.includeBlueprints) ?? false
  });
};

/**
 * Get bundle info
 */
const handleGetBundleInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.DATA_ASSETS, 'get_bundle_info', {
    bundleName: validateOptionalString(params.bundle_name || params.bundleName),
    includeAssets: validateOptionalBoolean(params.include_assets || params.includeAssets) ?? true,
    includeLoadState: validateOptionalBoolean(params.include_load_state || params.includeLoadState) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const dataassetHandlers: ActionHandlerMap = {
  // Data Asset
  create_data_asset: handleCreateDataAsset,
  configure_data_asset: handleConfigureDataAsset,
  duplicate_data_asset: handleDuplicateDataAsset,

  // Asset Manager
  configure_asset_manager: handleConfigureAssetManager,
  register_primary_asset_type: handleRegisterPrimaryAssetType,
  create_asset_bundle: handleCreateAssetBundle,
  configure_bundle_rules: handleConfigureBundleRules,

  // Asset Registry
  setup_asset_registry: handleSetupAssetRegistry,
  add_asset_tag: handleAddAssetTag,
  search_assets_by_tag: handleSearchAssetsByTag,

  // Soft References
  create_soft_reference: handleCreateSoftReference,
  configure_async_loading: handleConfigureAsyncLoading,

  // Query
  get_asset_info: handleGetAssetInfo,
  list_data_assets: handleListDataAssets,
  get_bundle_info: handleGetBundleInfo
};

export const handleDataAssetTools = createCategoryHandler(Forge2Categories.DATA_ASSETS, dataassetHandlers);

export default dataassetHandlers;
