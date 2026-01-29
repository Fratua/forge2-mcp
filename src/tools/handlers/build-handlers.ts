/**
 * FORGE2: Build System Handlers
 * Handles build settings, build targets, packaging, and DLC content
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
// Build Settings Action Handlers
// ============================================================================

/**
 * Configure build settings
 */
const handleConfigureBuildSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.BUILD, 'configure_build_settings', {
    platform: validateOptionalString(params.platform) ?? 'Windows',
    configuration: validateOptionalString(params.configuration) ?? 'Development',
    targetName: validateOptionalString(params.target_name || params.targetName),
    buildCookRun: validateOptionalBoolean(params.build_cook_run || params.buildCookRun) ?? true,
    compressedPak: validateOptionalBoolean(params.compressed_pak || params.compressedPak) ?? true,
    iterativeCooking: validateOptionalBoolean(params.iterative_cooking || params.iterativeCooking) ?? true
  });
};

/**
 * Setup build target
 */
const handleSetupBuildTarget: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const targetName = validateRequiredString(params.target_name || params.targetName, 'target_name');

  return executeForge2Action(tools, Forge2Categories.BUILD, 'setup_build_target', {
    targetName,
    targetType: validateOptionalString(params.target_type || params.targetType) ?? 'Game',
    buildConfiguration: validateOptionalString(params.build_configuration || params.buildConfiguration) ?? 'Development',
    platform: validateOptionalString(params.platform) ?? 'Win64',
    extraModules: validateOptionalArray(params.extra_modules || params.extraModules) ?? [],
    globalDefinitions: validateOptionalArray(params.global_definitions || params.globalDefinitions) ?? []
  });
};

/**
 * Create build profile
 */
const handleCreateBuildProfile: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const profileName = validateRequiredString(params.profile_name || params.profileName, 'profile_name');

  return executeForge2Action(tools, Forge2Categories.BUILD, 'create_build_profile', {
    profileName,
    platform: validateOptionalString(params.platform) ?? 'Windows',
    configuration: validateOptionalString(params.configuration) ?? 'Shipping',
    mapsToInclude: validateOptionalArray(params.maps_to_include || params.mapsToInclude) ?? [],
    mapsToExclude: validateOptionalArray(params.maps_to_exclude || params.mapsToExclude) ?? [],
    cookCultures: validateOptionalArray(params.cook_cultures || params.cookCultures) ?? ['en'],
    cookFlavor: validateOptionalString(params.cook_flavor || params.cookFlavor)
  });
};

// ============================================================================
// PAK File Action Handlers
// ============================================================================

/**
 * Configure PAK files
 */
const handleConfigurePakFiles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.BUILD, 'configure_pak_files', {
    usePakFile: validateOptionalBoolean(params.use_pak_file || params.usePakFile) ?? true,
    signPak: validateOptionalBoolean(params.sign_pak || params.signPak) ?? false,
    encryptPak: validateOptionalBoolean(params.encrypt_pak || params.encryptPak) ?? false,
    encryptionKey: validateOptionalString(params.encryption_key || params.encryptionKey),
    compressionFormats: validateOptionalArray(params.compression_formats || params.compressionFormats) ?? ['Zlib'],
    maxChunkSize: validateOptionalNumber(params.max_chunk_size || params.maxChunkSize)
  });
};

/**
 * Create PAK chunk
 */
const handleCreatePakChunk: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const chunkId = validateRequiredString(params.chunk_id || params.chunkId, 'chunk_id');

  return executeForge2Action(tools, Forge2Categories.BUILD, 'create_pak_chunk', {
    chunkId,
    chunkName: validateOptionalString(params.chunk_name || params.chunkName) ?? chunkId,
    assets: validateOptionalArray(params.assets) ?? [],
    directories: validateOptionalArray(params.directories) ?? [],
    priority: validateOptionalNumber(params.priority) ?? 0
  });
};

// ============================================================================
// DLC/Plugin Content Action Handlers
// ============================================================================

/**
 * Setup DLC content
 */
const handleSetupDLCContent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const dlcName = validateRequiredString(params.dlc_name || params.dlcName, 'dlc_name');

  return executeForge2Action(tools, Forge2Categories.BUILD, 'setup_dlc_content', {
    dlcName,
    contentPath: validateOptionalString(params.content_path || params.contentPath) ?? `/Game/DLC/${dlcName}`,
    chunkId: validateOptionalNumber(params.chunk_id || params.chunkId),
    releasedWithBase: validateOptionalBoolean(params.released_with_base || params.releasedWithBase) ?? false,
    maps: validateOptionalArray(params.maps) ?? [],
    requiredDLC: validateOptionalArray(params.required_dlc || params.requiredDLC) ?? []
  });
};

/**
 * Configure DLC mounting
 */
const handleConfigureDLCMounting: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const dlcName = validateRequiredString(params.dlc_name || params.dlcName, 'dlc_name');

  return executeForge2Action(tools, Forge2Categories.BUILD, 'configure_dlc_mounting', {
    dlcName,
    autoMount: validateOptionalBoolean(params.auto_mount || params.autoMount) ?? true,
    mountPriority: validateOptionalNumber(params.mount_priority || params.mountPriority) ?? 0,
    requireOwnership: validateOptionalBoolean(params.require_ownership || params.requireOwnership) ?? true
  });
};

// ============================================================================
// Cooking Action Handlers
// ============================================================================

/**
 * Configure cooking
 */
const handleConfigureCooking: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.BUILD, 'configure_cooking', {
    cookOnTheFly: validateOptionalBoolean(params.cook_on_the_fly || params.cookOnTheFly) ?? false,
    iterativeCooking: validateOptionalBoolean(params.iterative_cooking || params.iterativeCooking) ?? true,
    unversionedContent: validateOptionalBoolean(params.unversioned_content || params.unversionedContent) ?? false,
    createReleaseVersion: validateOptionalBoolean(params.create_release_version || params.createReleaseVersion) ?? false,
    additionalCookerOptions: validateOptionalArray(params.additional_cooker_options || params.additionalCookerOptions) ?? []
  });
};

/**
 * Add cooking rule
 */
const handleAddCookingRule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const ruleName = validateRequiredString(params.rule_name || params.ruleName, 'rule_name');

  return executeForge2Action(tools, Forge2Categories.BUILD, 'add_cooking_rule', {
    ruleName,
    assetPath: validateOptionalString(params.asset_path || params.assetPath),
    assetClass: validateOptionalString(params.asset_class || params.assetClass),
    cookRule: validateOptionalString(params.cook_rule || params.cookRule) ?? 'always_cook',
    platforms: validateOptionalArray(params.platforms) ?? []
  });
};

// ============================================================================
// Packaging Action Handlers
// ============================================================================

/**
 * Start packaging
 */
const handleStartPackaging: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.BUILD, 'start_packaging', {
    profileName: validateOptionalString(params.profile_name || params.profileName),
    platform: validateOptionalString(params.platform) ?? 'Windows',
    configuration: validateOptionalString(params.configuration) ?? 'Shipping',
    outputDirectory: validateOptionalString(params.output_directory || params.outputDirectory),
    archive: validateOptionalBoolean(params.archive) ?? false,
    staged: validateOptionalBoolean(params.staged) ?? true
  });
};

/**
 * Configure staging
 */
const handleConfigureStaging: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.BUILD, 'configure_staging', {
    stageDirectory: validateOptionalString(params.stage_directory || params.stageDirectory),
    stageNonUFS: validateOptionalBoolean(params.stage_non_ufs || params.stageNonUFS) ?? true,
    additionalFiles: validateOptionalArray(params.additional_files || params.additionalFiles) ?? [],
    excludedFiles: validateOptionalArray(params.excluded_files || params.excludedFiles) ?? []
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get build info
 */
const handleGetBuildInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.BUILD, 'get_build_info', {
    includeProfiles: validateOptionalBoolean(params.include_profiles || params.includeProfiles) ?? true,
    includeTargets: validateOptionalBoolean(params.include_targets || params.includeTargets) ?? true,
    includeDLC: validateOptionalBoolean(params.include_dlc || params.includeDLC) ?? true
  });
};

/**
 * List build profiles
 */
const handleListBuildProfiles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.BUILD, 'list_build_profiles', {
    platform: validateOptionalString(params.platform),
    configuration: validateOptionalString(params.configuration)
  });
};

/**
 * Get packaging status
 */
const handleGetPackagingStatus: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.BUILD, 'get_packaging_status', {
    includeProgress: validateOptionalBoolean(params.include_progress || params.includeProgress) ?? true,
    includeErrors: validateOptionalBoolean(params.include_errors || params.includeErrors) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const buildHandlers: ActionHandlerMap = {
  // Build Settings
  configure_build_settings: handleConfigureBuildSettings,
  setup_build_target: handleSetupBuildTarget,
  create_build_profile: handleCreateBuildProfile,

  // PAK Files
  configure_pak_files: handleConfigurePakFiles,
  create_pak_chunk: handleCreatePakChunk,

  // DLC Content
  setup_dlc_content: handleSetupDLCContent,
  configure_dlc_mounting: handleConfigureDLCMounting,

  // Cooking
  configure_cooking: handleConfigureCooking,
  add_cooking_rule: handleAddCookingRule,

  // Packaging
  start_packaging: handleStartPackaging,
  configure_staging: handleConfigureStaging,

  // Query
  get_build_info: handleGetBuildInfo,
  list_build_profiles: handleListBuildProfiles,
  get_packaging_status: handleGetPackagingStatus
};

export const handleBuildTools = createCategoryHandler(Forge2Categories.BUILD, buildHandlers);

export default buildHandlers;
