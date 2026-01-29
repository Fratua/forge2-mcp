/**
 * FORGE2: Validation System Handlers
 * Handles asset validation, blueprint errors, performance analysis, and naming conventions
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
// Asset Validation Action Handlers
// ============================================================================

/**
 * Validate assets
 */
const handleValidateAssets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'validate_assets', {
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game',
    assetTypes: validateOptionalArray(params.asset_types || params.assetTypes),
    checkMissingRefs: validateOptionalBoolean(params.check_missing_refs || params.checkMissingRefs) ?? true,
    checkCircularRefs: validateOptionalBoolean(params.check_circular_refs || params.checkCircularRefs) ?? true,
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

/**
 * Check missing references
 */
const handleCheckMissingReferences: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'check_missing_references', {
    assetPath: validateOptionalString(params.asset_path || params.assetPath),
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game',
    includeRedirectors: validateOptionalBoolean(params.include_redirectors || params.includeRedirectors) ?? true,
    fixRedirectors: validateOptionalBoolean(params.fix_redirectors || params.fixRedirectors) ?? false
  });
};

/**
 * Validate asset references
 */
const handleValidateAssetReferences: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetPath = validateRequiredString(params.asset_path || params.assetPath, 'asset_path');

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'validate_asset_references', {
    assetPath,
    checkHardRefs: validateOptionalBoolean(params.check_hard_refs || params.checkHardRefs) ?? true,
    checkSoftRefs: validateOptionalBoolean(params.check_soft_refs || params.checkSoftRefs) ?? true,
    includeDependencies: validateOptionalBoolean(params.include_dependencies || params.includeDependencies) ?? false
  });
};

// ============================================================================
// Blueprint Validation Action Handlers
// ============================================================================

/**
 * Check blueprint errors
 */
const handleCheckBlueprintErrors: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'check_blueprint_errors', {
    blueprintPath: validateOptionalString(params.blueprint_path || params.blueprintPath),
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game',
    compileAll: validateOptionalBoolean(params.compile_all || params.compileAll) ?? false,
    includeWarnings: validateOptionalBoolean(params.include_warnings || params.includeWarnings) ?? true,
    includeInfo: validateOptionalBoolean(params.include_info || params.includeInfo) ?? false
  });
};

/**
 * Validate blueprint nodes
 */
const handleValidateBlueprintNodes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const blueprintPath = validateRequiredString(params.blueprint_path || params.blueprintPath, 'blueprint_path');

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'validate_blueprint_nodes', {
    blueprintPath,
    checkDeprecated: validateOptionalBoolean(params.check_deprecated || params.checkDeprecated) ?? true,
    checkUnconnected: validateOptionalBoolean(params.check_unconnected || params.checkUnconnected) ?? true,
    checkOrphaned: validateOptionalBoolean(params.check_orphaned || params.checkOrphaned) ?? true
  });
};

/**
 * Check blueprint complexity
 */
const handleCheckBlueprintComplexity: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const blueprintPath = validateRequiredString(params.blueprint_path || params.blueprintPath, 'blueprint_path');

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'check_blueprint_complexity', {
    blueprintPath,
    maxNodes: validateOptionalNumber(params.max_nodes || params.maxNodes) ?? 500,
    maxFunctions: validateOptionalNumber(params.max_functions || params.maxFunctions) ?? 50,
    maxVariables: validateOptionalNumber(params.max_variables || params.maxVariables) ?? 100,
    warnOnTick: validateOptionalBoolean(params.warn_on_tick || params.warnOnTick) ?? true
  });
};

// ============================================================================
// Performance Validation Action Handlers
// ============================================================================

/**
 * Analyze performance
 */
const handleAnalyzePerformance: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'analyze_performance', {
    targetMap: validateOptionalString(params.target_map || params.targetMap),
    analyzeGPU: validateOptionalBoolean(params.analyze_gpu || params.analyzeGPU) ?? true,
    analyzeCPU: validateOptionalBoolean(params.analyze_cpu || params.analyzeCPU) ?? true,
    analyzeMemory: validateOptionalBoolean(params.analyze_memory || params.analyzeMemory) ?? true,
    duration: validateOptionalNumber(params.duration) ?? 30.0
  });
};

/**
 * Check draw calls
 */
const handleCheckDrawCalls: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'check_draw_calls', {
    targetMap: validateOptionalString(params.target_map || params.targetMap),
    maxDrawCalls: validateOptionalNumber(params.max_draw_calls || params.maxDrawCalls) ?? 2000,
    checkInstancing: validateOptionalBoolean(params.check_instancing || params.checkInstancing) ?? true,
    checkMaterials: validateOptionalBoolean(params.check_materials || params.checkMaterials) ?? true
  });
};

/**
 * Validate texture sizes
 */
const handleValidateTextureSizes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'validate_texture_sizes', {
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game',
    maxSize: validateOptionalNumber(params.max_size || params.maxSize) ?? 4096,
    checkPowerOfTwo: validateOptionalBoolean(params.check_power_of_two || params.checkPowerOfTwo) ?? true,
    checkCompression: validateOptionalBoolean(params.check_compression || params.checkCompression) ?? true
  });
};

// ============================================================================
// Data Integrity Action Handlers
// ============================================================================

/**
 * Validate data integrity
 */
const handleValidateDataIntegrity: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'validate_data_integrity', {
    dataTablePath: validateOptionalString(params.data_table_path || params.dataTablePath),
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game/Data',
    checkNullValues: validateOptionalBoolean(params.check_null_values || params.checkNullValues) ?? true,
    checkRanges: validateOptionalBoolean(params.check_ranges || params.checkRanges) ?? true,
    checkDuplicates: validateOptionalBoolean(params.check_duplicates || params.checkDuplicates) ?? true
  });
};

/**
 * Validate data table schema
 */
const handleValidateDataTableSchema: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'validate_data_table_schema', {
    tablePath,
    expectedColumns: validateOptionalArray(params.expected_columns || params.expectedColumns),
    checkTypes: validateOptionalBoolean(params.check_types || params.checkTypes) ?? true
  });
};

// ============================================================================
// Naming Convention Action Handlers
// ============================================================================

/**
 * Check naming conventions
 */
const handleCheckNamingConventions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'check_naming_conventions', {
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game',
    conventions: validateOptionalObject(params.conventions) ?? {
      Blueprint: 'BP_',
      Material: 'M_',
      MaterialInstance: 'MI_',
      Texture: 'T_',
      StaticMesh: 'SM_',
      SkeletalMesh: 'SK_',
      Widget: 'WBP_',
      DataTable: 'DT_',
      DataAsset: 'DA_'
    },
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

/**
 * Add naming rule
 */
const handleAddNamingRule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetType = validateRequiredString(params.asset_type || params.assetType, 'asset_type');
  const prefix = validateRequiredString(params.prefix, 'prefix');

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'add_naming_rule', {
    assetType,
    prefix,
    suffix: validateOptionalString(params.suffix),
    pattern: validateOptionalString(params.pattern),
    caseSensitive: validateOptionalBoolean(params.case_sensitive || params.caseSensitive) ?? true
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get validation report
 */
const handleGetValidationReport: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'get_validation_report', {
    includeErrors: validateOptionalBoolean(params.include_errors || params.includeErrors) ?? true,
    includeWarnings: validateOptionalBoolean(params.include_warnings || params.includeWarnings) ?? true,
    includePassed: validateOptionalBoolean(params.include_passed || params.includePassed) ?? false,
    groupByCategory: validateOptionalBoolean(params.group_by_category || params.groupByCategory) ?? true
  });
};

/**
 * List validation rules
 */
const handleListValidationRules: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'list_validation_rules', {
    category: validateOptionalString(params.category),
    enabledOnly: validateOptionalBoolean(params.enabled_only || params.enabledOnly) ?? false
  });
};

/**
 * Get asset health
 */
const handleGetAssetHealth: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetPath = validateRequiredString(params.asset_path || params.assetPath, 'asset_path');

  return executeForge2Action(tools, Forge2Categories.VALIDATION, 'get_asset_health', {
    assetPath,
    includeMetrics: validateOptionalBoolean(params.include_metrics || params.includeMetrics) ?? true,
    includeRecommendations: validateOptionalBoolean(params.include_recommendations || params.includeRecommendations) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const validationHandlers: ActionHandlerMap = {
  // Asset Validation
  validate_assets: handleValidateAssets,
  check_missing_references: handleCheckMissingReferences,
  validate_asset_references: handleValidateAssetReferences,

  // Blueprint Validation
  check_blueprint_errors: handleCheckBlueprintErrors,
  validate_blueprint_nodes: handleValidateBlueprintNodes,
  check_blueprint_complexity: handleCheckBlueprintComplexity,

  // Performance Validation
  analyze_performance: handleAnalyzePerformance,
  check_draw_calls: handleCheckDrawCalls,
  validate_texture_sizes: handleValidateTextureSizes,

  // Data Integrity
  validate_data_integrity: handleValidateDataIntegrity,
  validate_data_table_schema: handleValidateDataTableSchema,

  // Naming Conventions
  check_naming_conventions: handleCheckNamingConventions,
  add_naming_rule: handleAddNamingRule,

  // Query
  get_validation_report: handleGetValidationReport,
  list_validation_rules: handleListValidationRules,
  get_asset_health: handleGetAssetHealth
};

export const handleValidationTools = createCategoryHandler(Forge2Categories.VALIDATION, validationHandlers);

export default validationHandlers;
