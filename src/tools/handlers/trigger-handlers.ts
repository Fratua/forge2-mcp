/**
 * FORGE2: Trigger & Event Handlers
 * Handles trigger volumes, gameplay events, and event dispatchers
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
// Trigger Volume Action Handlers
// ============================================================================

/**
 * Create trigger volume
 */
const handleCreateTriggerVolume: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const triggerId = validateRequiredString(params.trigger_id || params.triggerId, 'trigger_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_trigger_volume', {
    triggerId,
    triggerName: validateOptionalString(params.trigger_name || params.triggerName) ?? triggerId,
    volumeShape: validateOptionalString(params.volume_shape || params.volumeShape) ?? 'box',
    location: validateOptionalObject(params.location) ?? { x: 0, y: 0, z: 0 },
    size: validateOptionalObject(params.size) ?? { x: 200, y: 200, z: 200 },
    triggerOnce: validateOptionalBoolean(params.trigger_once || params.triggerOnce) ?? false,
    enabled: validateOptionalBoolean(params.enabled) ?? true
  });
};

/**
 * Configure trigger events
 */
const handleConfigureTriggerEvents: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const triggerId = validateRequiredString(params.trigger_id || params.triggerId, 'trigger_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'configure_trigger_events', {
    triggerId,
    onBeginOverlap: validateOptionalBoolean(params.on_begin_overlap || params.onBeginOverlap) ?? true,
    onEndOverlap: validateOptionalBoolean(params.on_end_overlap || params.onEndOverlap) ?? true,
    onHit: validateOptionalBoolean(params.on_hit || params.onHit) ?? false,
    actorFilter: validateOptionalArray(params.actor_filter || params.actorFilter),
    tagFilter: validateOptionalArray(params.tag_filter || params.tagFilter),
    classFilter: validateOptionalString(params.class_filter || params.classFilter)
  });
};

/**
 * Add trigger action
 */
const handleAddTriggerAction: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const triggerId = validateRequiredString(params.trigger_id || params.triggerId, 'trigger_id');
  const actionType = validateRequiredString(params.action_type || params.actionType, 'action_type');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'add_trigger_action', {
    triggerId,
    actionType,
    eventType: validateOptionalString(params.event_type || params.eventType) ?? 'begin_overlap',
    targetActor: validateOptionalString(params.target_actor || params.targetActor),
    functionName: validateOptionalString(params.function_name || params.functionName),
    parameters: validateOptionalObject(params.parameters),
    delay: validateOptionalNumber(params.delay) ?? 0
  });
};

// ============================================================================
// Event Dispatcher Action Handlers
// ============================================================================

/**
 * Create event dispatcher
 */
const handleCreateEventDispatcher: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const eventId = validateRequiredString(params.event_id || params.eventId, 'event_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_event_dispatcher', {
    eventId,
    eventName: validateOptionalString(params.event_name || params.eventName) ?? eventId,
    multicast: validateOptionalBoolean(params.multicast) ?? true,
    parameters: validateOptionalArray(params.parameters) ?? [],
    blueprintClass: validateOptionalString(params.blueprint_class || params.blueprintClass)
  });
};

/**
 * Bind event listener
 */
const handleBindEventListener: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const eventId = validateRequiredString(params.event_id || params.eventId, 'event_id');
  const listenerActor = validateRequiredString(params.listener_actor || params.listenerActor, 'listener_actor');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'bind_event_listener', {
    eventId,
    listenerActor,
    functionName: validateRequiredString(params.function_name || params.functionName, 'function_name'),
    bindOnce: validateOptionalBoolean(params.bind_once || params.bindOnce) ?? false
  });
};

// ============================================================================
// Gameplay Tags Action Handlers
// ============================================================================

/**
 * Create gameplay tag
 */
const handleCreateGameplayTag: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const tagName = validateRequiredString(params.tag_name || params.tagName, 'tag_name');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_gameplay_tag', {
    tagName,
    description: validateOptionalString(params.description),
    parentTag: validateOptionalString(params.parent_tag || params.parentTag),
    devComment: validateOptionalString(params.dev_comment || params.devComment)
  });
};

/**
 * Configure tag container
 */
const handleConfigureTagContainer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const actorPath = validateRequiredString(params.actor_path || params.actorPath, 'actor_path');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'configure_tag_container', {
    actorPath,
    ownedTags: validateOptionalArray(params.owned_tags || params.ownedTags) ?? [],
    blockedTags: validateOptionalArray(params.blocked_tags || params.blockedTags) ?? [],
    requiredTags: validateOptionalArray(params.required_tags || params.requiredTags) ?? []
  });
};

// ============================================================================
// Gameplay Cue Action Handlers
// ============================================================================

/**
 * Create gameplay cue
 */
const handleCreateGameplayCue: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const cueTag = validateRequiredString(params.cue_tag || params.cueTag, 'cue_tag');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_gameplay_cue', {
    cueTag,
    cueType: validateOptionalString(params.cue_type || params.cueType) ?? 'burst',
    vfxPath: validateOptionalString(params.vfx_path || params.vfxPath),
    soundPath: validateOptionalString(params.sound_path || params.soundPath),
    cameraShake: validateOptionalString(params.camera_shake || params.cameraShake),
    forceFeedback: validateOptionalString(params.force_feedback || params.forceFeedback)
  });
};

/**
 * Configure cue parameters
 */
const handleConfigureCueParameters: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const cueTag = validateRequiredString(params.cue_tag || params.cueTag, 'cue_tag');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'configure_cue_parameters', {
    cueTag,
    attachToActor: validateOptionalBoolean(params.attach_to_actor || params.attachToActor) ?? true,
    socketName: validateOptionalString(params.socket_name || params.socketName),
    locationOffset: validateOptionalObject(params.location_offset || params.locationOffset),
    rotationOffset: validateOptionalObject(params.rotation_offset || params.rotationOffset),
    scaleMultiplier: validateOptionalNumber(params.scale_multiplier || params.scaleMultiplier) ?? 1.0
  });
};

// ============================================================================
// Game Events Action Handlers
// ============================================================================

/**
 * Setup global events
 */
const handleSetupGlobalEvents: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'setup_global_events', {
    eventCategories: validateOptionalArray(params.event_categories || params.eventCategories) ?? ['game', 'player', 'world', 'combat', 'ui'],
    persistEvents: validateOptionalBoolean(params.persist_events || params.persistEvents) ?? false,
    eventHistory: validateOptionalBoolean(params.event_history || params.eventHistory) ?? true,
    maxHistorySize: validateOptionalNumber(params.max_history_size || params.maxHistorySize) ?? 100
  });
};

/**
 * Create game event
 */
const handleCreateGameEvent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const eventId = validateRequiredString(params.event_id || params.eventId, 'event_id');

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'create_game_event', {
    eventId,
    eventName: validateOptionalString(params.event_name || params.eventName) ?? eventId,
    category: validateOptionalString(params.category) ?? 'game',
    priority: validateOptionalNumber(params.priority) ?? 0,
    canCancel: validateOptionalBoolean(params.can_cancel || params.canCancel) ?? false,
    payload: validateOptionalObject(params.payload)
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get trigger info
 */
const handleGetTriggerInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'get_trigger_info', {
    triggerId: validateOptionalString(params.trigger_id || params.triggerId),
    includeActions: validateOptionalBoolean(params.include_actions || params.includeActions) ?? true,
    includeOverlapping: validateOptionalBoolean(params.include_overlapping || params.includeOverlapping) ?? false
  });
};

/**
 * List triggers
 */
const handleListTriggers: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'list_triggers', {
    volumeShape: validateOptionalString(params.volume_shape || params.volumeShape),
    enabledOnly: validateOptionalBoolean(params.enabled_only || params.enabledOnly) ?? false
  });
};

/**
 * List gameplay tags
 */
const handleListGameplayTags: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.LEVEL, 'list_gameplay_tags', {
    parentTag: validateOptionalString(params.parent_tag || params.parentTag),
    includeChildren: validateOptionalBoolean(params.include_children || params.includeChildren) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const triggerHandlers: ActionHandlerMap = {
  // Trigger Volumes
  create_trigger_volume: handleCreateTriggerVolume,
  configure_trigger_events: handleConfigureTriggerEvents,
  add_trigger_action: handleAddTriggerAction,

  // Event Dispatchers
  create_event_dispatcher: handleCreateEventDispatcher,
  bind_event_listener: handleBindEventListener,

  // Gameplay Tags
  create_gameplay_tag: handleCreateGameplayTag,
  configure_tag_container: handleConfigureTagContainer,

  // Gameplay Cues
  create_gameplay_cue: handleCreateGameplayCue,
  configure_cue_parameters: handleConfigureCueParameters,

  // Game Events
  setup_global_events: handleSetupGlobalEvents,
  create_game_event: handleCreateGameEvent,

  // Query
  get_trigger_info: handleGetTriggerInfo,
  list_triggers: handleListTriggers,
  list_gameplay_tags: handleListGameplayTags
};

export const handleTriggerTools = createCategoryHandler(Forge2Categories.LEVEL, triggerHandlers);

export default triggerHandlers;
