/**
 * FORGE2: Stealth Genre Handlers
 * Complete stealth game mechanics (Metal Gear, Hitman, Thief style)
 *
 * Supports: Detection systems, AI awareness, patrol paths
 * Systems: Distractions, cover, disguises, security, shadows
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

function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value;
}

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

function validateRequiredEnum<T extends string>(
  value: unknown,
  allowedValues: readonly T[],
  fieldName: string
): T {
  if (value === undefined || value === null || value === '') {
    throw new Error(`${fieldName} is required`);
  }
  if (typeof value !== 'string' || !allowedValues.includes(value as T)) {
    throw new Error(`${fieldName} must be one of: ${allowedValues.join(', ')}`);
  }
  return value as T;
}

// ============================================================================
// Types
// ============================================================================

export type AlertState = 'unaware' | 'suspicious' | 'investigating' | 'alert' | 'searching' | 'hostile';
export type DistractionType = 'noise' | 'thrown_object' | 'decoy' | 'electronics' | 'body' | 'alarm';
export type CoverType = 'crouch' | 'stand' | 'lean' | 'corner';
export type SecuritySystemType = 'camera' | 'laser' | 'motion_sensor' | 'pressure_plate' | 'alarm' | 'turret';
export type LightLevel = 'bright' | 'dim' | 'shadow' | 'dark';

// ============================================================================
// Constants
// ============================================================================

const ALERT_STATES = ['unaware', 'suspicious', 'investigating', 'alert', 'searching', 'hostile'] as const;
const DISTRACTION_TYPES = ['noise', 'thrown_object', 'decoy', 'electronics', 'body', 'alarm'] as const;
const COVER_TYPES = ['crouch', 'stand', 'lean', 'corner'] as const;
const SECURITY_SYSTEM_TYPES = ['camera', 'laser', 'motion_sensor', 'pressure_plate', 'alarm', 'turret'] as const;
const LIGHT_LEVELS = ['bright', 'dim', 'shadow', 'dark'] as const;
void LIGHT_LEVELS; // Used for type reference, reserved for future light level validation

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create detection system
 */
const handleCreateDetectionSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'create_detection_system', {
    visionConeAngle: params.vision_cone_angle ?? params.visionConeAngle ?? 90,
    visionConeDistance: params.vision_cone_distance ?? params.visionConeDistance ?? 2000,
    peripheralAngle: params.peripheral_angle ?? params.peripheralAngle ?? 120,
    peripheralDistance: params.peripheral_distance ?? params.peripheralDistance ?? 500,
    hearingRadius: params.hearing_radius ?? params.hearingRadius ?? 1000,
    detectionSpeed: params.detection_speed ?? params.detectionSpeed ?? 1.0,
    loseTargetTime: params.lose_target_time ?? params.loseTargetTime ?? 5.0,
    suspicionDecay: params.suspicion_decay ?? params.suspicionDecay ?? 0.1,
    lightAffectsDetection: params.light_affects_detection ?? params.lightAffectsDetection ?? true,
    movementAffectsDetection: params.movement_affects_detection ?? params.movementAffectsDetection ?? true
  });
};

/**
 * Configure AI awareness
 */
const handleConfigureAIAwareness: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const initialState = validateOptionalEnum(
    params.initial_state || params.initialState,
    ALERT_STATES,
    'initial_state'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'configure_ai_awareness', {
    initialState: initialState || 'unaware',
    suspicionThreshold: params.suspicion_threshold ?? params.suspicionThreshold ?? 50,
    alertThreshold: params.alert_threshold ?? params.alertThreshold ?? 100,
    investigationTime: params.investigation_time ?? params.investigationTime ?? 10,
    searchTime: params.search_time ?? params.searchTime ?? 30,
    returnToPatrolTime: params.return_to_patrol_time ?? params.returnToPatrolTime ?? 15,
    callForBackup: params.call_for_backup ?? params.callForBackup ?? true,
    backupRadius: params.backup_radius ?? params.backupRadius ?? 1500,
    shareKnowledge: params.share_knowledge ?? params.shareKnowledge ?? true
  });
};

/**
 * Configure alert state behavior
 */
const handleConfigureAlertState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const alertState = validateRequiredEnum(
    params.alert_state || params.alertState,
    ALERT_STATES,
    'alert_state'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'configure_alert_state', {
    alertState,
    moveSpeed: params.move_speed ?? params.moveSpeed,
    visionMultiplier: params.vision_multiplier ?? params.visionMultiplier ?? 1.0,
    hearingMultiplier: params.hearing_multiplier ?? params.hearingMultiplier ?? 1.0,
    canAttack: params.can_attack ?? params.canAttack ?? false,
    behaviorTree: params.behavior_tree || params.behaviorTree,
    animationState: params.animation_state || params.animationState,
    musicCue: params.music_cue || params.musicCue
  });
};

/**
 * Setup patrol paths
 */
const handleSetupPatrolPaths: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const patrolId = validateRequiredString(params.patrol_id || params.patrolId, 'patrol_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'setup_patrol_paths', {
    patrolId,
    waypoints: params.waypoints,
    loopMode: params.loop_mode || params.loopMode || 'loop',
    waitTimes: params.wait_times || params.waitTimes,
    lookDirections: params.look_directions || params.lookDirections,
    walkSpeed: params.walk_speed ?? params.walkSpeed ?? 200,
    randomization: params.randomization ?? 0,
    timeOfDayVariant: params.time_of_day_variant ?? params.timeOfDayVariant
  });
};

/**
 * Assign patrol to AI
 */
const handleAssignPatrol: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const aiId = validateRequiredString(params.ai_id || params.aiId, 'ai_id');
  const patrolId = validateRequiredString(params.patrol_id || params.patrolId, 'patrol_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'assign_patrol', {
    aiId,
    patrolId,
    startWaypoint: params.start_waypoint ?? params.startWaypoint ?? 0,
    startDelay: params.start_delay ?? params.startDelay ?? 0
  });
};

/**
 * Create distraction system
 */
const handleCreateDistractionSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'create_distraction_system', {
    maxSimultaneousDistractions: params.max_simultaneous_distractions ?? params.maxSimultaneousDistractions ?? 3,
    distractionDecayTime: params.distraction_decay_time ?? params.distractionDecayTime ?? 10,
    aiResponseRadius: params.ai_response_radius ?? params.aiResponseRadius ?? 1000,
    maxRespondingAI: params.max_responding_ai ?? params.maxRespondingAI ?? 2
  });
};

/**
 * Create distraction type
 */
const handleCreateDistractionType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const distractionType = validateRequiredEnum(
    params.distraction_type || params.distractionType,
    DISTRACTION_TYPES,
    'distraction_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'create_distraction_type', {
    distractionType,
    noiseRadius: params.noise_radius ?? params.noiseRadius ?? 500,
    duration: params.duration ?? 5,
    priority: params.priority ?? 1,
    interruptPatrol: params.interrupt_patrol ?? params.interruptPatrol ?? true,
    causeSuspicion: params.cause_suspicion ?? params.causeSuspicion ?? true,
    suspicionAmount: params.suspicion_amount ?? params.suspicionAmount ?? 30,
    meshPath: params.mesh_path || params.meshPath,
    soundPath: params.sound_path || params.soundPath
  });
};

/**
 * Configure cover system
 */
const handleConfigureCoverSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'configure_cover_system', {
    autoDetectCover: params.auto_detect_cover ?? params.autoDetectCover ?? true,
    coverDetectionRadius: params.cover_detection_radius ?? params.coverDetectionRadius ?? 100,
    snapToCover: params.snap_to_cover ?? params.snapToCover ?? true,
    peekExposure: params.peek_exposure ?? params.peekExposure ?? 0.5,
    coverTakedownRange: params.cover_takedown_range ?? params.coverTakedownRange ?? 150,
    cornerTakedownRange: params.corner_takedown_range ?? params.cornerTakedownRange ?? 200,
    coverMoveSpeed: params.cover_move_speed ?? params.coverMoveSpeed ?? 150,
    transitionTime: params.transition_time ?? params.transitionTime ?? 0.3
  });
};

/**
 * Setup cover point
 */
const handleSetupCoverPoint: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const coverType = validateRequiredEnum(
    params.cover_type || params.coverType,
    COVER_TYPES,
    'cover_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'setup_cover_point', {
    coverType,
    location: params.location,
    rotation: params.rotation,
    coverHeight: params.cover_height ?? params.coverHeight ?? 100,
    canPeekLeft: params.can_peek_left ?? params.canPeekLeft ?? true,
    canPeekRight: params.can_peek_right ?? params.canPeekRight ?? true,
    canVault: params.can_vault ?? params.canVault ?? false,
    linkedCoverPoints: params.linked_cover_points || params.linkedCoverPoints
  });
};

/**
 * Setup disguise system
 */
const handleSetupDisguiseSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'setup_disguise_system', {
    enabled: params.enabled ?? true,
    disguiseChangeTime: params.disguise_change_time ?? params.disguiseChangeTime ?? 3,
    suspicionOnChange: params.suspicion_on_change ?? params.suspicionOnChange ?? true,
    requireHiddenSpot: params.require_hidden_spot ?? params.requireHiddenSpot ?? true,
    enforcerSystem: params.enforcer_system ?? params.enforcerSystem ?? true,
    enforcerSuspicionRate: params.enforcer_suspicion_rate ?? params.enforcerSuspicionRate ?? 2.0
  });
};

/**
 * Create disguise
 */
const handleCreateDisguise: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const disguiseId = validateRequiredString(params.disguise_id || params.disguiseId, 'disguise_id');
  const disguiseName = validateRequiredString(params.disguise_name || params.disguiseName, 'disguise_name');

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'create_disguise', {
    disguiseId,
    disguiseName,
    accessLevel: params.access_level ?? params.accessLevel ?? 1,
    accessibleAreas: params.accessible_areas || params.accessibleAreas,
    restrictedActions: params.restricted_actions || params.restrictedActions,
    enforcerTypes: params.enforcer_types || params.enforcerTypes,
    meshOverride: params.mesh_override || params.meshOverride,
    iconPath: params.icon_path || params.iconPath
  });
};

/**
 * Create security system
 */
const handleCreateSecuritySystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const systemType = validateRequiredEnum(
    params.system_type || params.systemType,
    SECURITY_SYSTEM_TYPES,
    'system_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'create_security_system', {
    systemType,
    detectionRadius: params.detection_radius ?? params.detectionRadius ?? 500,
    rotationSpeed: params.rotation_speed ?? params.rotationSpeed ?? 30,
    rotationArc: params.rotation_arc ?? params.rotationArc ?? 180,
    alertDelay: params.alert_delay ?? params.alertDelay ?? 2,
    canBeDisabled: params.can_be_disabled ?? params.canBeDisabled ?? true,
    disableTime: params.disable_time ?? params.disableTime ?? 30,
    linkedAlarm: params.linked_alarm || params.linkedAlarm,
    meshPath: params.mesh_path || params.meshPath
  });
};

/**
 * Place security system
 */
const handlePlaceSecuritySystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const systemType = validateRequiredEnum(
    params.system_type || params.systemType,
    SECURITY_SYSTEM_TYPES,
    'system_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'place_security_system', {
    systemType,
    location: params.location,
    rotation: params.rotation,
    networkId: params.network_id || params.networkId,
    initiallyActive: params.initially_active ?? params.initiallyActive ?? true
  });
};

/**
 * Configure shadow system
 */
const handleConfigureShadowSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'configure_shadow_system', {
    enabled: params.enabled ?? true,
    lightThresholds: params.light_thresholds || params.lightThresholds,
    visibilityInBright: params.visibility_in_bright ?? params.visibilityInBright ?? 1.0,
    visibilityInDim: params.visibility_in_dim ?? params.visibilityInDim ?? 0.7,
    visibilityInShadow: params.visibility_in_shadow ?? params.visibilityInShadow ?? 0.3,
    visibilityInDark: params.visibility_in_dark ?? params.visibilityInDark ?? 0.1,
    movementPenalty: params.movement_penalty ?? params.movementPenalty ?? true,
    crouchBonus: params.crouch_bonus ?? params.crouchBonus ?? 0.5,
    showLightMeter: params.show_light_meter ?? params.showLightMeter ?? true
  });
};

/**
 * Get current light level at location
 */
const handleGetLightLevel: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'get_light_level', {
    location: params.location,
    actorId: params.actor_id || params.actorId
  });
};

/**
 * Configure takedown system
 */
const handleConfigureTakedownSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'configure_takedown_system', {
    lethalTakedownEnabled: params.lethal_takedown_enabled ?? params.lethalTakedownEnabled ?? true,
    nonLethalTakedownEnabled: params.non_lethal_takedown_enabled ?? params.nonLethalTakedownEnabled ?? true,
    takedownRange: params.takedown_range ?? params.takedownRange ?? 150,
    takedownAngle: params.takedown_angle ?? params.takedownAngle ?? 60,
    takedownDuration: params.takedown_duration ?? params.takedownDuration ?? 2,
    noiseOnTakedown: params.noise_on_takedown ?? params.noiseOnTakedown ?? 100,
    canDragBodies: params.can_drag_bodies ?? params.canDragBodies ?? true,
    bodyHideSpots: params.body_hide_spots ?? params.bodyHideSpots ?? true,
    unconsciousWakeTime: params.unconscious_wake_time ?? params.unconsciousWakeTime ?? 300
  });
};

/**
 * Get AI alert state
 */
const handleGetAlertState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const aiId = validateRequiredString(params.ai_id || params.aiId, 'ai_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'get_alert_state', {
    aiId
  });
};

/**
 * Trigger alarm
 */
const handleTriggerAlarm: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_STEALTH, 'trigger_alarm', {
    alarmId: params.alarm_id || params.alarmId,
    location: params.location,
    alertRadius: params.alert_radius ?? params.alertRadius ?? 2000
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const genreStealthHandlers: ActionHandlerMap = {
  create_detection_system: handleCreateDetectionSystem,
  configure_ai_awareness: handleConfigureAIAwareness,
  configure_alert_state: handleConfigureAlertState,
  setup_patrol_paths: handleSetupPatrolPaths,
  assign_patrol: handleAssignPatrol,
  create_distraction_system: handleCreateDistractionSystem,
  create_distraction_type: handleCreateDistractionType,
  configure_cover_system: handleConfigureCoverSystem,
  setup_cover_point: handleSetupCoverPoint,
  setup_disguise_system: handleSetupDisguiseSystem,
  create_disguise: handleCreateDisguise,
  create_security_system: handleCreateSecuritySystem,
  place_security_system: handlePlaceSecuritySystem,
  configure_shadow_system: handleConfigureShadowSystem,
  get_light_level: handleGetLightLevel,
  configure_takedown_system: handleConfigureTakedownSystem,
  get_alert_state: handleGetAlertState,
  trigger_alarm: handleTriggerAlarm
};

/**
 * Main genre stealth handler - routes to specific action handlers
 */
export const handleGenreStealthTools = createCategoryHandler(Forge2Categories.GENRE_STEALTH, genreStealthHandlers);

/**
 * Get available genre stealth actions
 */
export function getGenreStealthActions(): string[] {
  return Object.keys(genreStealthHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeGenreStealthHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = genreStealthHandlers[action];
  if (!handler) {
    throw new Error(`Unknown genre stealth action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
