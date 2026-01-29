/**
 * FORGE2: NPC Systems Handlers
 * Handles NPC schedules, relationships, conversation, memory, and crowd simulation
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
// NPC Schedule Action Handlers
// ============================================================================

/**
 * Create NPC schedule
 */
const handleCreateNPCSchedule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const scheduleId = validateRequiredString(params.schedule_id || params.scheduleId, 'schedule_id');

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'create_npc_schedule', {
    scheduleId,
    name: validateOptionalString(params.name) ?? scheduleId,
    dailyRoutine: validateOptionalBoolean(params.daily_routine || params.dailyRoutine) ?? true,
    weeklyVariation: validateOptionalBoolean(params.weekly_variation || params.weeklyVariation) ?? false,
    weatherAware: validateOptionalBoolean(params.weather_aware || params.weatherAware) ?? true,
    interruptible: validateOptionalBoolean(params.interruptible) ?? true
  });
};

/**
 * Add schedule activity
 */
const handleAddScheduleActivity: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const scheduleId = validateRequiredString(params.schedule_id || params.scheduleId, 'schedule_id');
  const activityName = validateRequiredString(params.activity_name || params.activityName, 'activity_name');

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'add_schedule_activity', {
    scheduleId,
    activityName,
    startHour: validateOptionalNumber(params.start_hour || params.startHour) ?? 8,
    endHour: validateOptionalNumber(params.end_hour || params.endHour) ?? 17,
    locationId: validateOptionalString(params.location_id || params.locationId),
    animationState: validateOptionalString(params.animation_state || params.animationState),
    priority: validateOptionalNumber(params.priority) ?? 0,
    daysOfWeek: validateOptionalArray(params.days_of_week || params.daysOfWeek)
  });
};

/**
 * Configure sleep schedule
 */
const handleConfigureSleepSchedule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const scheduleId = validateRequiredString(params.schedule_id || params.scheduleId, 'schedule_id');

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'configure_sleep_schedule', {
    scheduleId,
    sleepEnabled: validateOptionalBoolean(params.sleep_enabled || params.sleepEnabled) ?? true,
    bedtimeHour: validateOptionalNumber(params.bedtime_hour || params.bedtimeHour) ?? 22,
    wakeHour: validateOptionalNumber(params.wake_hour || params.wakeHour) ?? 6,
    homeLocationId: validateOptionalString(params.home_location_id || params.homeLocationId),
    sleepAnimation: validateOptionalString(params.sleep_animation || params.sleepAnimation)
  });
};

// ============================================================================
// NPC Relationships Action Handlers
// ============================================================================

/**
 * Setup NPC relationships
 */
const handleSetupNPCRelationships: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'setup_npc_relationships', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    relationshipTypes: validateOptionalArray(params.relationship_types || params.relationshipTypes) ?? ['friend', 'enemy', 'family', 'romantic', 'business', 'rival'],
    dynamicRelationships: validateOptionalBoolean(params.dynamic_relationships || params.dynamicRelationships) ?? true,
    relationshipDecay: validateOptionalBoolean(params.relationship_decay || params.relationshipDecay) ?? true,
    decayRate: validateOptionalNumber(params.decay_rate || params.decayRate) ?? 0.01
  });
};

/**
 * Create NPC relationship
 */
const handleCreateNPCRelationship: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const npcAId = validateRequiredString(params.npc_a_id || params.npcAId, 'npc_a_id');
  const npcBId = validateRequiredString(params.npc_b_id || params.npcBId, 'npc_b_id');

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'create_npc_relationship', {
    npcAId,
    npcBId,
    relationshipType: validateOptionalString(params.relationship_type || params.relationshipType) ?? 'neutral',
    initialValue: validateOptionalNumber(params.initial_value || params.initialValue) ?? 50,
    bidirectional: validateOptionalBoolean(params.bidirectional) ?? true
  });
};

// ============================================================================
// NPC Conversation Action Handlers
// ============================================================================

/**
 * Setup NPC conversation
 */
const handleSetupNPCConversation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'setup_npc_conversation', {
    idleChatterEnabled: validateOptionalBoolean(params.idle_chatter_enabled || params.idleChatterEnabled) ?? true,
    greetingsEnabled: validateOptionalBoolean(params.greetings_enabled || params.greetingsEnabled) ?? true,
    contextAwareDialogue: validateOptionalBoolean(params.context_aware_dialogue || params.contextAwareDialogue) ?? true,
    weatherComments: validateOptionalBoolean(params.weather_comments || params.weatherComments) ?? true,
    timeOfDayComments: validateOptionalBoolean(params.time_of_day_comments || params.timeOfDayComments) ?? true,
    cooldownSeconds: validateOptionalNumber(params.cooldown_seconds || params.cooldownSeconds) ?? 60
  });
};

/**
 * Add NPC greeting
 */
const handleAddNPCGreeting: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const npcId = validateRequiredString(params.npc_id || params.npcId, 'npc_id');

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'add_npc_greeting', {
    npcId,
    greetingText: validateRequiredString(params.greeting_text || params.greetingText, 'greeting_text'),
    conditions: validateOptionalObject(params.conditions),
    relationshipThreshold: validateOptionalNumber(params.relationship_threshold || params.relationshipThreshold),
    timeOfDay: validateOptionalString(params.time_of_day || params.timeOfDay),
    voicePath: validateOptionalString(params.voice_path || params.voicePath)
  });
};

// ============================================================================
// NPC Memory Action Handlers
// ============================================================================

/**
 * Setup NPC memory
 */
const handleSetupNPCMemory: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'setup_npc_memory', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    rememberPlayerActions: validateOptionalBoolean(params.remember_player_actions || params.rememberPlayerActions) ?? true,
    memoryDuration: validateOptionalNumber(params.memory_duration || params.memoryDuration) ?? 604800,
    maxMemories: validateOptionalNumber(params.max_memories || params.maxMemories) ?? 50,
    persistAcrossSessions: validateOptionalBoolean(params.persist_across_sessions || params.persistAcrossSessions) ?? true
  });
};

/**
 * Add memory type
 */
const handleAddMemoryType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const memoryType = validateRequiredString(params.memory_type || params.memoryType, 'memory_type');

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'add_memory_type', {
    memoryType,
    importance: validateOptionalNumber(params.importance) ?? 1.0,
    decayRate: validateOptionalNumber(params.decay_rate || params.decayRate) ?? 0.1,
    affectsRelationship: validateOptionalBoolean(params.affects_relationship || params.affectsRelationship) ?? true,
    relationshipImpact: validateOptionalNumber(params.relationship_impact || params.relationshipImpact) ?? 0
  });
};

// ============================================================================
// Crowd Simulation Action Handlers
// ============================================================================

/**
 * Setup crowd simulation
 */
const handleSetupCrowdSimulation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'setup_crowd_simulation', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    maxCrowdSize: validateOptionalNumber(params.max_crowd_size || params.maxCrowdSize) ?? 50,
    spawnRadius: validateOptionalNumber(params.spawn_radius || params.spawnRadius) ?? 3000.0,
    despawnRadius: validateOptionalNumber(params.despawn_radius || params.despawnRadius) ?? 5000.0,
    lodEnabled: validateOptionalBoolean(params.lod_enabled || params.lodEnabled) ?? true,
    avoidanceEnabled: validateOptionalBoolean(params.avoidance_enabled || params.avoidanceEnabled) ?? true
  });
};

/**
 * Create crowd zone
 */
const handleCreateCrowdZone: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const zoneId = validateRequiredString(params.zone_id || params.zoneId, 'zone_id');

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'create_crowd_zone', {
    zoneId,
    zoneName: validateOptionalString(params.zone_name || params.zoneName) ?? zoneId,
    crowdDensity: validateOptionalNumber(params.crowd_density || params.crowdDensity) ?? 0.5,
    npcTypes: validateOptionalArray(params.npc_types || params.npcTypes) ?? ['civilian'],
    behaviorType: validateOptionalString(params.behavior_type || params.behaviorType) ?? 'wander',
    activeHours: validateOptionalObject(params.active_hours || params.activeHours) ?? { start: 6, end: 22 },
    weatherSensitive: validateOptionalBoolean(params.weather_sensitive || params.weatherSensitive) ?? true
  });
};

/**
 * Configure traffic simulation
 */
const handleConfigureTrafficSimulation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'configure_traffic_simulation', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    vehicleTypes: validateOptionalArray(params.vehicle_types || params.vehicleTypes) ?? ['car', 'truck', 'bicycle'],
    trafficDensity: validateOptionalNumber(params.traffic_density || params.trafficDensity) ?? 0.5,
    obeyTrafficRules: validateOptionalBoolean(params.obey_traffic_rules || params.obeyTrafficRules) ?? true,
    parkingEnabled: validateOptionalBoolean(params.parking_enabled || params.parkingEnabled) ?? true
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get NPC system info
 */
const handleGetNPCSystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'get_npc_system_info', {
    includeSchedules: validateOptionalBoolean(params.include_schedules || params.includeSchedules) ?? true,
    includeRelationships: validateOptionalBoolean(params.include_relationships || params.includeRelationships) ?? true,
    includeCrowdZones: validateOptionalBoolean(params.include_crowd_zones || params.includeCrowdZones) ?? true
  });
};

/**
 * List NPC schedules
 */
const handleListNPCSchedules: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.NPC_SYSTEMS, 'list_npc_schedules', {
    includeActivities: validateOptionalBoolean(params.include_activities || params.includeActivities) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const npcHandlers: ActionHandlerMap = {
  // Schedule
  create_npc_schedule: handleCreateNPCSchedule,
  add_schedule_activity: handleAddScheduleActivity,
  configure_sleep_schedule: handleConfigureSleepSchedule,

  // Relationships
  setup_npc_relationships: handleSetupNPCRelationships,
  create_npc_relationship: handleCreateNPCRelationship,

  // Conversation
  setup_npc_conversation: handleSetupNPCConversation,
  add_npc_greeting: handleAddNPCGreeting,

  // Memory
  setup_npc_memory: handleSetupNPCMemory,
  add_memory_type: handleAddMemoryType,

  // Crowd
  setup_crowd_simulation: handleSetupCrowdSimulation,
  create_crowd_zone: handleCreateCrowdZone,
  configure_traffic_simulation: handleConfigureTrafficSimulation,

  // Query
  get_npc_system_info: handleGetNPCSystemInfo,
  list_npc_schedules: handleListNPCSchedules
};

export const handleNPCTools = createCategoryHandler(Forge2Categories.NPC_SYSTEMS, npcHandlers);

export default npcHandlers;
