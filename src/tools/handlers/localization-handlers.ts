/**
 * FORGE2: Localization Handlers
 * Handles string tables, text formatting, audio localization, font fallbacks, RTL support
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
// String Table Action Handlers
// ============================================================================

/**
 * Setup string tables
 */
const handleSetupStringTables: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tableName = validateRequiredString(params.table_name || params.tableName, 'table_name');

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'setup_string_tables', {
    tableName,
    namespace: validateOptionalString(params.namespace) ?? 'Game',
    defaultLanguage: validateOptionalString(params.default_language || params.defaultLanguage) ?? 'en',
    supportedLanguages: validateOptionalArray(params.supported_languages || params.supportedLanguages) ?? ['en', 'es', 'fr', 'de', 'ja', 'ko', 'zh'],
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add string entry
 */
const handleAddStringEntry: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');
  const key = validateRequiredString(params.key, 'key');

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'add_string_entry', {
    tablePath,
    key,
    sourceText: validateRequiredString(params.source_text || params.sourceText, 'source_text'),
    comment: validateOptionalString(params.comment),
    category: validateOptionalString(params.category),
    translations: validateOptionalObject(params.translations)
  });
};

/**
 * Import string translations
 */
const handleImportStringTranslations: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tablePath = validateRequiredString(params.table_path || params.tablePath, 'table_path');

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'import_string_translations', {
    tablePath,
    importPath: validateRequiredString(params.import_path || params.importPath, 'import_path'),
    format: validateOptionalString(params.format) ?? 'csv',
    language: validateOptionalString(params.language),
    overwrite: validateOptionalBoolean(params.overwrite) ?? false
  });
};

// ============================================================================
// Text Formatting Action Handlers
// ============================================================================

/**
 * Configure text formatting
 */
const handleConfigureTextFormatting: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'configure_text_formatting', {
    pluralRulesEnabled: validateOptionalBoolean(params.plural_rules_enabled || params.pluralRulesEnabled) ?? true,
    genderRulesEnabled: validateOptionalBoolean(params.gender_rules_enabled || params.genderRulesEnabled) ?? true,
    numberFormatting: validateOptionalBoolean(params.number_formatting || params.numberFormatting) ?? true,
    dateFormatting: validateOptionalBoolean(params.date_formatting || params.dateFormatting) ?? true,
    currencyFormatting: validateOptionalBoolean(params.currency_formatting || params.currencyFormatting) ?? true,
    argumentSyntax: validateOptionalString(params.argument_syntax || params.argumentSyntax) ?? 'icu'
  });
};

/**
 * Add plural rule
 */
const handleAddPluralRule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const language = validateRequiredString(params.language, 'language');
  const key = validateRequiredString(params.key, 'key');

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'add_plural_rule', {
    language,
    key,
    zero: validateOptionalString(params.zero),
    one: validateOptionalString(params.one),
    two: validateOptionalString(params.two),
    few: validateOptionalString(params.few),
    many: validateOptionalString(params.many),
    other: validateRequiredString(params.other, 'other')
  });
};

// ============================================================================
// Audio Localization Action Handlers
// ============================================================================

/**
 * Setup audio localization
 */
const handleSetupAudioLocalization: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'setup_audio_localization', {
    dubbedLanguages: validateOptionalArray(params.dubbed_languages || params.dubbedLanguages) ?? ['en'],
    subtitleLanguages: validateOptionalArray(params.subtitle_languages || params.subtitleLanguages) ?? ['en', 'es', 'fr', 'de', 'ja', 'ko', 'zh'],
    defaultAudioLanguage: validateOptionalString(params.default_audio_language || params.defaultAudioLanguage) ?? 'en',
    audioFallback: validateOptionalString(params.audio_fallback || params.audioFallback) ?? 'en',
    streamingEnabled: validateOptionalBoolean(params.streaming_enabled || params.streamingEnabled) ?? true
  });
};

/**
 * Add localized audio
 */
const handleAddLocalizedAudio: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const audioKey = validateRequiredString(params.audio_key || params.audioKey, 'audio_key');

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'add_localized_audio', {
    audioKey,
    language: validateRequiredString(params.language, 'language'),
    audioPath: validateRequiredString(params.audio_path || params.audioPath, 'audio_path'),
    subtitleKey: validateOptionalString(params.subtitle_key || params.subtitleKey),
    lipSyncData: validateOptionalString(params.lip_sync_data || params.lipSyncData)
  });
};

// ============================================================================
// Font Action Handlers
// ============================================================================

/**
 * Create font fallbacks
 */
const handleCreateFontFallbacks: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const fontFamilyName = validateRequiredString(params.font_family_name || params.fontFamilyName, 'font_family_name');

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'create_font_fallbacks', {
    fontFamilyName,
    primaryFont: validateRequiredString(params.primary_font || params.primaryFont, 'primary_font'),
    cjkFallback: validateOptionalString(params.cjk_fallback || params.cjkFallback),
    arabicFallback: validateOptionalString(params.arabic_fallback || params.arabicFallback),
    cyrillicFallback: validateOptionalString(params.cyrillic_fallback || params.cyrillicFallback),
    greekFallback: validateOptionalString(params.greek_fallback || params.greekFallback),
    hebrewFallback: validateOptionalString(params.hebrew_fallback || params.hebrewFallback),
    thaiFallback: validateOptionalString(params.thai_fallback || params.thaiFallback),
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Configure font scaling
 */
const handleConfigureFontScaling: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'configure_font_scaling', {
    baseSize: validateOptionalNumber(params.base_size || params.baseSize) ?? 16,
    scalingPerLanguage: validateOptionalObject(params.scaling_per_language || params.scalingPerLanguage),
    minSize: validateOptionalNumber(params.min_size || params.minSize) ?? 12,
    maxSize: validateOptionalNumber(params.max_size || params.maxSize) ?? 32,
    autoScale: validateOptionalBoolean(params.auto_scale || params.autoScale) ?? true
  });
};

// ============================================================================
// RTL Support Action Handlers
// ============================================================================

/**
 * Configure RTL support
 */
const handleConfigureRTLSupport: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'configure_rtl_support', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    rtlLanguages: validateOptionalArray(params.rtl_languages || params.rtlLanguages) ?? ['ar', 'he', 'fa', 'ur'],
    mirrorUI: validateOptionalBoolean(params.mirror_ui || params.mirrorUI) ?? true,
    mirrorAnimations: validateOptionalBoolean(params.mirror_animations || params.mirrorAnimations) ?? true,
    preserveNumbers: validateOptionalBoolean(params.preserve_numbers || params.preserveNumbers) ?? true,
    preserveIcons: validateOptionalBoolean(params.preserve_icons || params.preserveIcons) ?? false
  });
};

// ============================================================================
// Regional Content Action Handlers
// ============================================================================

/**
 * Setup regional content
 */
const handleSetupRegionalContent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'setup_regional_content', {
    regions: validateOptionalArray(params.regions) ?? ['NA', 'EU', 'JP', 'KR', 'CN'],
    regionDetection: validateOptionalString(params.region_detection || params.regionDetection) ?? 'auto',
    allowOverride: validateOptionalBoolean(params.allow_override || params.allowOverride) ?? true,
    contentDifferences: validateOptionalObject(params.content_differences || params.contentDifferences)
  });
};

/**
 * Add regional variant
 */
const handleAddRegionalVariant: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const assetPath = validateRequiredString(params.asset_path || params.assetPath, 'asset_path');
  const region = validateRequiredString(params.region, 'region');

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'add_regional_variant', {
    assetPath,
    region,
    variantPath: validateRequiredString(params.variant_path || params.variantPath, 'variant_path'),
    reason: validateOptionalString(params.reason)
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get localization info
 */
const handleGetLocalizationInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'get_localization_info', {
    tablePath: validateOptionalString(params.table_path || params.tablePath),
    language: validateOptionalString(params.language),
    includeStats: validateOptionalBoolean(params.include_stats || params.includeStats) ?? true
  });
};

/**
 * List string tables
 */
const handleListStringTables: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LOCALIZATION, 'list_string_tables', {
    namespace: validateOptionalString(params.namespace),
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const localizationHandlers: ActionHandlerMap = {
  // String Tables
  setup_string_tables: handleSetupStringTables,
  add_string_entry: handleAddStringEntry,
  import_string_translations: handleImportStringTranslations,

  // Text Formatting
  configure_text_formatting: handleConfigureTextFormatting,
  add_plural_rule: handleAddPluralRule,

  // Audio
  setup_audio_localization: handleSetupAudioLocalization,
  add_localized_audio: handleAddLocalizedAudio,

  // Fonts
  create_font_fallbacks: handleCreateFontFallbacks,
  configure_font_scaling: handleConfigureFontScaling,

  // RTL
  configure_rtl_support: handleConfigureRTLSupport,

  // Regional
  setup_regional_content: handleSetupRegionalContent,
  add_regional_variant: handleAddRegionalVariant,

  // Query
  get_localization_info: handleGetLocalizationInfo,
  list_string_tables: handleListStringTables
};

export const handleLocalizationTools = createCategoryHandler(Forge2Categories.LOCALIZATION, localizationHandlers);

export default localizationHandlers;
