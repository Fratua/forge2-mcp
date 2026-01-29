/**
 * FORGE2: Analytics Handlers
 * Handles event tracking, funnel analysis, heatmaps, session tracking, and A/B testing
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
// Event Tracking Action Handlers
// ============================================================================

/**
 * Setup event tracking
 */
const handleSetupEventTracking: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'setup_event_tracking', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    batchEvents: validateOptionalBoolean(params.batch_events || params.batchEvents) ?? true,
    batchInterval: validateOptionalNumber(params.batch_interval || params.batchInterval) ?? 60,
    maxBatchSize: validateOptionalNumber(params.max_batch_size || params.maxBatchSize) ?? 100,
    offlineStorage: validateOptionalBoolean(params.offline_storage || params.offlineStorage) ?? true,
    privacyMode: validateOptionalString(params.privacy_mode || params.privacyMode) ?? 'standard'
  });
};

/**
 * Create custom event
 */
const handleCreateCustomEvent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const eventName = validateRequiredString(params.event_name || params.eventName, 'event_name');

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'create_custom_event', {
    eventName,
    category: validateOptionalString(params.category) ?? 'gameplay',
    parameters: validateOptionalArray(params.parameters) ?? [],
    description: validateOptionalString(params.description),
    isImportant: validateOptionalBoolean(params.is_important || params.isImportant) ?? false,
    sampleRate: validateOptionalNumber(params.sample_rate || params.sampleRate) ?? 1.0
  });
};

/**
 * Add event parameter
 */
const handleAddEventParameter: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const eventName = validateRequiredString(params.event_name || params.eventName, 'event_name');
  const paramName = validateRequiredString(params.param_name || params.paramName, 'param_name');

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'add_event_parameter', {
    eventName,
    paramName,
    paramType: validateOptionalString(params.param_type || params.paramType) ?? 'string',
    required: validateOptionalBoolean(params.required) ?? false,
    defaultValue: params.default_value ?? params.defaultValue,
    validation: validateOptionalString(params.validation)
  });
};

// ============================================================================
// Funnel Analysis Action Handlers
// ============================================================================

/**
 * Configure funnel analysis
 */
const handleConfigureFunnelAnalysis: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'configure_funnel_analysis', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    trackOnboarding: validateOptionalBoolean(params.track_onboarding || params.trackOnboarding) ?? true,
    trackProgression: validateOptionalBoolean(params.track_progression || params.trackProgression) ?? true,
    trackMonetization: validateOptionalBoolean(params.track_monetization || params.trackMonetization) ?? true,
    customFunnels: validateOptionalArray(params.custom_funnels || params.customFunnels) ?? []
  });
};

/**
 * Create funnel
 */
const handleCreateFunnel: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const funnelId = validateRequiredString(params.funnel_id || params.funnelId, 'funnel_id');
  const funnelName = validateRequiredString(params.funnel_name || params.funnelName, 'funnel_name');

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'create_funnel', {
    funnelId,
    funnelName,
    steps: validateOptionalArray(params.steps) ?? [],
    timeoutMinutes: validateOptionalNumber(params.timeout_minutes || params.timeoutMinutes) ?? 1440,
    allowReentry: validateOptionalBoolean(params.allow_reentry || params.allowReentry) ?? false
  });
};

/**
 * Add funnel step
 */
const handleAddFunnelStep: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const funnelId = validateRequiredString(params.funnel_id || params.funnelId, 'funnel_id');
  const stepName = validateRequiredString(params.step_name || params.stepName, 'step_name');

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'add_funnel_step', {
    funnelId,
    stepName,
    stepOrder: validateOptionalNumber(params.step_order || params.stepOrder) ?? 0,
    triggerEvent: validateRequiredString(params.trigger_event || params.triggerEvent, 'trigger_event'),
    conditions: validateOptionalArray(params.conditions),
    isOptional: validateOptionalBoolean(params.is_optional || params.isOptional) ?? false
  });
};

// ============================================================================
// Heatmap Action Handlers
// ============================================================================

/**
 * Create heatmap
 */
const handleCreateHeatmap: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const heatmapId = validateRequiredString(params.heatmap_id || params.heatmapId, 'heatmap_id');

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'create_heatmap', {
    heatmapId,
    heatmapType: validateOptionalString(params.heatmap_type || params.heatmapType) ?? 'position',
    levelName: validateOptionalString(params.level_name || params.levelName),
    resolution: validateOptionalNumber(params.resolution) ?? 100,
    updateInterval: validateOptionalNumber(params.update_interval || params.updateInterval) ?? 1.0,
    maxDataPoints: validateOptionalNumber(params.max_data_points || params.maxDataPoints) ?? 100000
  });
};

/**
 * Configure death heatmap
 */
const handleConfigureDeathHeatmap: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'configure_death_heatmap', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    trackCauseOfDeath: validateOptionalBoolean(params.track_cause_of_death || params.trackCauseOfDeath) ?? true,
    trackKillerInfo: validateOptionalBoolean(params.track_killer_info || params.trackKillerInfo) ?? true,
    trackPlayerLevel: validateOptionalBoolean(params.track_player_level || params.trackPlayerLevel) ?? true,
    aggregatePeriod: validateOptionalString(params.aggregate_period || params.aggregatePeriod) ?? 'daily'
  });
};

/**
 * Configure movement heatmap
 */
const handleConfigureMovementHeatmap: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'configure_movement_heatmap', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    sampleRate: validateOptionalNumber(params.sample_rate || params.sampleRate) ?? 1.0,
    trackVelocity: validateOptionalBoolean(params.track_velocity || params.trackVelocity) ?? false,
    trackTime: validateOptionalBoolean(params.track_time || params.trackTime) ?? true,
    ignorePaused: validateOptionalBoolean(params.ignore_paused || params.ignorePaused) ?? true
  });
};

// ============================================================================
// Session Tracking Action Handlers
// ============================================================================

/**
 * Setup session tracking
 */
const handleSetupSessionTracking: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'setup_session_tracking', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    trackPlaytime: validateOptionalBoolean(params.track_playtime || params.trackPlaytime) ?? true,
    trackSessionCount: validateOptionalBoolean(params.track_session_count || params.trackSessionCount) ?? true,
    trackLastPlayed: validateOptionalBoolean(params.track_last_played || params.trackLastPlayed) ?? true,
    sessionTimeoutMinutes: validateOptionalNumber(params.session_timeout_minutes || params.sessionTimeoutMinutes) ?? 30,
    trackRetention: validateOptionalBoolean(params.track_retention || params.trackRetention) ?? true,
    retentionDays: validateOptionalArray(params.retention_days || params.retentionDays) ?? [1, 7, 14, 30]
  });
};

/**
 * Configure retention tracking
 */
const handleConfigureRetentionTracking: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'configure_retention_tracking', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    cohortDays: validateOptionalArray(params.cohort_days || params.cohortDays) ?? [1, 3, 7, 14, 30, 60, 90],
    trackBySource: validateOptionalBoolean(params.track_by_source || params.trackBySource) ?? true,
    trackByPlatform: validateOptionalBoolean(params.track_by_platform || params.trackByPlatform) ?? true
  });
};

// ============================================================================
// A/B Testing Action Handlers
// ============================================================================

/**
 * Configure A/B testing
 */
const handleConfigureABTesting: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'configure_ab_testing', {
    enabled: validateOptionalBoolean(params.enabled) ?? false,
    assignmentMethod: validateOptionalString(params.assignment_method || params.assignmentMethod) ?? 'random',
    persistAssignment: validateOptionalBoolean(params.persist_assignment || params.persistAssignment) ?? true,
    allowMultipleTests: validateOptionalBoolean(params.allow_multiple_tests || params.allowMultipleTests) ?? true
  });
};

/**
 * Create experiment
 */
const handleCreateExperiment: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const experimentId = validateRequiredString(params.experiment_id || params.experimentId, 'experiment_id');
  const experimentName = validateRequiredString(params.experiment_name || params.experimentName, 'experiment_name');

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'create_experiment', {
    experimentId,
    experimentName,
    description: validateOptionalString(params.description),
    variants: validateOptionalArray(params.variants) ?? [
      { name: 'control', weight: 50 },
      { name: 'treatment', weight: 50 }
    ],
    targetAudience: validateOptionalObject(params.target_audience || params.targetAudience),
    startDate: validateOptionalString(params.start_date || params.startDate),
    endDate: validateOptionalString(params.end_date || params.endDate),
    primaryMetric: validateOptionalString(params.primary_metric || params.primaryMetric)
  });
};

/**
 * Add experiment variant
 */
const handleAddExperimentVariant: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const experimentId = validateRequiredString(params.experiment_id || params.experimentId, 'experiment_id');
  const variantName = validateRequiredString(params.variant_name || params.variantName, 'variant_name');

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'add_experiment_variant', {
    experimentId,
    variantName,
    weight: validateOptionalNumber(params.weight) ?? 50,
    featureFlags: validateOptionalObject(params.feature_flags || params.featureFlags),
    parameters: validateOptionalObject(params.parameters)
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get analytics info
 */
const handleGetAnalyticsInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'get_analytics_info', {
    includeEvents: validateOptionalBoolean(params.include_events || params.includeEvents) ?? true,
    includeFunnels: validateOptionalBoolean(params.include_funnels || params.includeFunnels) ?? true,
    includeHeatmaps: validateOptionalBoolean(params.include_heatmaps || params.includeHeatmaps) ?? true,
    includeExperiments: validateOptionalBoolean(params.include_experiments || params.includeExperiments) ?? true
  });
};

/**
 * List custom events
 */
const handleListCustomEvents: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'list_custom_events', {
    category: validateOptionalString(params.category),
    includeParameters: validateOptionalBoolean(params.include_parameters || params.includeParameters) ?? true
  });
};

/**
 * List experiments
 */
const handleListExperiments: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.ANALYTICS, 'list_experiments', {
    status: validateOptionalString(params.status) ?? 'all',
    includeResults: validateOptionalBoolean(params.include_results || params.includeResults) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const analyticsHandlers: ActionHandlerMap = {
  // Event Tracking
  setup_event_tracking: handleSetupEventTracking,
  create_custom_event: handleCreateCustomEvent,
  add_event_parameter: handleAddEventParameter,

  // Funnel Analysis
  configure_funnel_analysis: handleConfigureFunnelAnalysis,
  create_funnel: handleCreateFunnel,
  add_funnel_step: handleAddFunnelStep,

  // Heatmaps
  create_heatmap: handleCreateHeatmap,
  configure_death_heatmap: handleConfigureDeathHeatmap,
  configure_movement_heatmap: handleConfigureMovementHeatmap,

  // Session Tracking
  setup_session_tracking: handleSetupSessionTracking,
  configure_retention_tracking: handleConfigureRetentionTracking,

  // A/B Testing
  configure_ab_testing: handleConfigureABTesting,
  create_experiment: handleCreateExperiment,
  add_experiment_variant: handleAddExperimentVariant,

  // Query
  get_analytics_info: handleGetAnalyticsInfo,
  list_custom_events: handleListCustomEvents,
  list_experiments: handleListExperiments
};

export const handleAnalyticsTools = createCategoryHandler(Forge2Categories.ANALYTICS, analyticsHandlers);

export default analyticsHandlers;
