/**
 * FORGE2: Data Table & Curve Handlers
 * Handles data tables, curve tables, curve assets, and composite tables
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
// Data Table Action Handlers
// ============================================================================

/**
 * Create data table
 */
const handleCreateDataTable: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tableName = validateRequiredString(params.table_name || params.tableName, 'table_name');
  const rowStruct = validateRequiredString(params.row_struct || params.rowStruct, 'row_struct');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'create_data_table', {
    tableName,
    rowStruct,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Data/Tables/${tableName}`,
    description: validateOptionalString(params.description)
  });
};

/**
 * Add data row
 */
const handleAddDataRow: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');
  const rowName = validateRequiredString(params.row_name || params.rowName, 'row_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'add_data_row', {
    tablePath,
    rowName,
    rowData: validateOptionalObject(params.row_data || params.rowData) ?? {}
  });
};

/**
 * Update data row
 */
const handleUpdateDataRow: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');
  const rowName = validateRequiredString(params.row_name || params.rowName, 'row_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'update_data_row', {
    tablePath,
    rowName,
    rowData: validateOptionalObject(params.row_data || params.rowData) ?? {},
    mergeData: validateOptionalBoolean(params.merge_data || params.mergeData) ?? true
  });
};

/**
 * Remove data row
 */
const handleRemoveDataRow: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');
  const rowName = validateRequiredString(params.row_name || params.rowName, 'row_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'remove_data_row', {
    tablePath,
    rowName
  });
};

/**
 * Import CSV to table
 */
const handleImportCSVToTable: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');
  const csvPath = validateRequiredString(params.csv_path || params.csvPath, 'csv_path');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'import_csv_to_table', {
    tablePath,
    csvPath,
    replaceExisting: validateOptionalBoolean(params.replace_existing || params.replaceExisting) ?? false,
    skipFirstRow: validateOptionalBoolean(params.skip_first_row || params.skipFirstRow) ?? true
  });
};

/**
 * Export table to CSV
 */
const handleExportTableToCSV: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'export_table_to_csv', {
    tablePath,
    outputPath: validateOptionalString(params.output_path || params.outputPath),
    includeRowNames: validateOptionalBoolean(params.include_row_names || params.includeRowNames) ?? true
  });
};

// ============================================================================
// Curve Table Action Handlers
// ============================================================================

/**
 * Create curve table
 */
const handleCreateCurveTable: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tableName = validateRequiredString(params.table_name || params.tableName, 'table_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'create_curve_table', {
    tableName,
    curveType: validateOptionalString(params.curve_type || params.curveType) ?? 'float',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Data/Curves/${tableName}`,
    description: validateOptionalString(params.description)
  });
};

/**
 * Add curve row
 */
const handleAddCurveRow: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');
  const rowName = validateRequiredString(params.row_name || params.rowName, 'row_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'add_curve_row', {
    tablePath,
    rowName,
    keys: validateOptionalArray(params.keys) ?? [],
    interpolation: validateOptionalString(params.interpolation) ?? 'linear'
  });
};

/**
 * Add curve key
 */
const handleAddCurveKey: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');
  const rowName = validateRequiredString(params.row_name || params.rowName, 'row_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'add_curve_key', {
    tablePath,
    rowName,
    time: validateOptionalNumber(params.time) ?? 0,
    value: validateOptionalNumber(params.value) ?? 0,
    interpMode: validateOptionalString(params.interp_mode || params.interpMode) ?? 'linear',
    tangentMode: validateOptionalString(params.tangent_mode || params.tangentMode) ?? 'auto'
  });
};

// ============================================================================
// Curve Asset Action Handlers
// ============================================================================

/**
 * Create curve asset
 */
const handleCreateCurveAsset: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const curveName = validateRequiredString(params.curve_name || params.curveName, 'curve_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'create_curve_asset', {
    curveName,
    curveType: validateOptionalString(params.curve_type || params.curveType) ?? 'float',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Data/Curves/${curveName}`,
    presetType: validateOptionalString(params.preset_type || params.presetType)
  });
};

/**
 * Configure curve keys
 */
const handleConfigureCurveKeys: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const curvePath = validateRequiredString(params.curve_path || params.curvePath, 'curve_path');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'configure_curve_keys', {
    curvePath,
    keys: validateOptionalArray(params.keys) ?? [],
    defaultInterpMode: validateOptionalString(params.default_interp_mode || params.defaultInterpMode) ?? 'cubic',
    cycleMode: validateOptionalString(params.cycle_mode || params.cycleMode) ?? 'clamp'
  });
};

/**
 * Create vector curve
 */
const handleCreateVectorCurve: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const curveName = validateRequiredString(params.curve_name || params.curveName, 'curve_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'create_vector_curve', {
    curveName,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Data/Curves/${curveName}`,
    xKeys: validateOptionalArray(params.x_keys || params.xKeys) ?? [],
    yKeys: validateOptionalArray(params.y_keys || params.yKeys) ?? [],
    zKeys: validateOptionalArray(params.z_keys || params.zKeys) ?? []
  });
};

/**
 * Create color curve
 */
const handleCreateColorCurve: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const curveName = validateRequiredString(params.curve_name || params.curveName, 'curve_name');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'create_color_curve', {
    curveName,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Data/Curves/${curveName}`,
    colorKeys: validateOptionalArray(params.color_keys || params.colorKeys) ?? [],
    useHSV: validateOptionalBoolean(params.use_hsv || params.useHSV) ?? false
  });
};

// ============================================================================
// Composite Table Action Handlers
// ============================================================================

/**
 * Create composite table
 */
const handleCreateCompositeTable: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tableName = validateRequiredString(params.table_name || params.tableName, 'table_name');
  const rowStruct = validateRequiredString(params.row_struct || params.rowStruct, 'row_struct');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'create_composite_table', {
    tableName,
    rowStruct,
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Data/Tables/${tableName}`,
    parentTables: validateOptionalArray(params.parent_tables || params.parentTables) ?? []
  });
};

/**
 * Add table reference
 */
const handleAddTableReference: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');
  const referencedTable = validateRequiredString(params.referenced_table || params.referencedTable, 'referenced_table');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'add_table_reference', {
    tablePath,
    referencedTable,
    priority: validateOptionalNumber(params.priority) ?? 0
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get table info
 */
const handleGetTableInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'get_table_info', {
    tablePath,
    includeRows: validateOptionalBoolean(params.include_rows || params.includeRows) ?? false,
    includeStructure: validateOptionalBoolean(params.include_structure || params.includeStructure) ?? true
  });
};

/**
 * Get row data
 */
const handleGetRowData: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'get_row_data', {
    tablePath,
    rowName: validateOptionalString(params.row_name || params.rowName),
    rowFilter: validateOptionalObject(params.row_filter || params.rowFilter)
  });
};

/**
 * List data tables
 */
const handleListDataTables: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'list_data_tables', {
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game/Data',
    rowStruct: validateOptionalString(params.row_struct || params.rowStruct),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

/**
 * List curve assets
 */
const handleListCurveAssets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.DATA_TABLES, 'list_curve_assets', {
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game/Data/Curves',
    curveType: validateOptionalString(params.curve_type || params.curveType),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const datatableHandlers: ActionHandlerMap = {
  // Data Table
  create_data_table: handleCreateDataTable,
  add_data_row: handleAddDataRow,
  update_data_row: handleUpdateDataRow,
  remove_data_row: handleRemoveDataRow,
  import_csv_to_table: handleImportCSVToTable,
  export_table_to_csv: handleExportTableToCSV,

  // Curve Table
  create_curve_table: handleCreateCurveTable,
  add_curve_row: handleAddCurveRow,
  add_curve_key: handleAddCurveKey,

  // Curve Asset
  create_curve_asset: handleCreateCurveAsset,
  configure_curve_keys: handleConfigureCurveKeys,
  create_vector_curve: handleCreateVectorCurve,
  create_color_curve: handleCreateColorCurve,

  // Composite Table
  create_composite_table: handleCreateCompositeTable,
  add_table_reference: handleAddTableReference,

  // Query
  get_table_info: handleGetTableInfo,
  get_row_data: handleGetRowData,
  list_data_tables: handleListDataTables,
  list_curve_assets: handleListCurveAssets
};

export const handleDataTableTools = createCategoryHandler(Forge2Categories.DATA_TABLES, datatableHandlers);

export default datatableHandlers;
