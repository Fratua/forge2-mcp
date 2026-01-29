/**
 * FORGE2: Cinematics Handlers
 * Handles UE5 Sequencer, cutscenes, and cinematic camera systems
 * Supports level sequences, camera rigs, actor tracks, and cinematic modes
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

function validateOptionalEnum<T extends string>(
  value: unknown,
  allowedValues: readonly T[]
): T | undefined {
  if (value === undefined || value === null || value === '') {
    return undefined;
  }
  if (typeof value !== 'string' || !allowedValues.includes(value as T)) {
    return undefined;
  }
  return value as T;
}

// Note: validateOptionalArray reserved for future use
// function validateOptionalArray - see other handlers for pattern

function validateOptionalObject(value: unknown): Record<string, unknown> | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value === 'object' && !Array.isArray(value)) {
    return value as Record<string, unknown>;
  }
  return undefined;
}

// ============================================================================
// Constants
// ============================================================================

const TRACK_TYPES = ['transform', 'animation', 'visibility', 'event', 'property', 'fade', 'audio', 'camera_cut'] as const;
const BLEND_TYPES = ['linear', 'cubic', 'ease_in', 'ease_out', 'ease_in_out', 'step'] as const;
const CAMERA_RIG_TYPES = ['static', 'rail', 'crane', 'handheld', 'orbit', 'dolly'] as const;
const CINEMATIC_MODES = ['letterbox', 'full_screen', 'split_screen'] as const;
const EVALUATION_TYPES = ['with_sub_sequences', 'as_is', 'signed', 'unsigned'] as const;

// Reserve constants for future use
void TRACK_TYPES;
void BLEND_TYPES;
void CAMERA_RIG_TYPES;
void CINEMATIC_MODES;
void EVALUATION_TYPES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a level sequence
 */
const handleCreateLevelSequence: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequenceName = validateRequiredString(params.sequence_name || params.sequenceName, 'sequence_name');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'create_level_sequence', {
    sequenceName,
    frameRate: validateOptionalNumber(params.frame_rate || params.frameRate) ?? 30,
    durationSeconds: validateOptionalNumber(params.duration_seconds || params.durationSeconds) ?? 10.0,
    playbackStart: validateOptionalNumber(params.playback_start || params.playbackStart) ?? 0,
    looping: validateOptionalBoolean(params.looping) ?? false,
    autoPlay: validateOptionalBoolean(params.auto_play || params.autoPlay) ?? false,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add camera track to sequence
 */
const handleAddCameraTrack: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequencePath = validateRequiredString(params.sequence_path || params.sequencePath, 'sequence_path');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'add_camera_track', {
    sequencePath,
    cameraActorName: validateOptionalString(params.camera_actor_name || params.cameraActorName),
    createCamera: validateOptionalBoolean(params.create_camera || params.createCamera) ?? true,
    cameraName: validateOptionalString(params.camera_name || params.cameraName) ?? 'CineCamera',
    focalLength: validateOptionalNumber(params.focal_length || params.focalLength) ?? 35.0,
    sensorWidth: validateOptionalNumber(params.sensor_width || params.sensorWidth) ?? 24.89,
    focusMethod: validateOptionalString(params.focus_method || params.focusMethod) ?? 'manual', // 'manual', 'tracking', 'disable'
    aperture: validateOptionalNumber(params.aperture) ?? 2.8
  });
};

/**
 * Add camera cut to sequence
 */
const handleAddCameraCut: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequencePath = validateRequiredString(params.sequence_path || params.sequencePath, 'sequence_path');
  const cameraActorName = validateRequiredString(params.camera_actor_name || params.cameraActorName, 'camera_actor_name');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'add_camera_cut', {
    sequencePath,
    cameraActorName,
    startFrame: validateOptionalNumber(params.start_frame || params.startFrame) ?? 0,
    endFrame: validateOptionalNumber(params.end_frame || params.endFrame),
    blendTime: validateOptionalNumber(params.blend_time || params.blendTime) ?? 0.0,
    blendType: validateOptionalEnum(params.blend_type || params.blendType, BLEND_TYPES) ?? 'linear'
  });
};

/**
 * Add actor track to sequence
 */
const handleAddActorTrack: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequencePath = validateRequiredString(params.sequence_path || params.sequencePath, 'sequence_path');
  const actorName = validateRequiredString(params.actor_name || params.actorName, 'actor_name');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'add_actor_track', {
    sequencePath,
    actorName,
    trackType: validateOptionalEnum(params.track_type || params.trackType, TRACK_TYPES) ?? 'transform',
    spawnableTemplate: validateOptionalString(params.spawnable_template || params.spawnableTemplate),
    addAnimationTrack: validateOptionalBoolean(params.add_animation_track || params.addAnimationTrack) ?? false,
    addVisibilityTrack: validateOptionalBoolean(params.add_visibility_track || params.addVisibilityTrack) ?? false
  });
};

/**
 * Add keyframe to track
 */
const handleAddKeyframe: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequencePath = validateRequiredString(params.sequence_path || params.sequencePath, 'sequence_path');
  const trackName = validateRequiredString(params.track_name || params.trackName, 'track_name');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'add_keyframe', {
    sequencePath,
    trackName,
    frame: validateOptionalNumber(params.frame) ?? 0,
    time: validateOptionalNumber(params.time),
    value: params.value, // Can be various types
    interpMode: validateOptionalEnum(params.interp_mode || params.interpMode, BLEND_TYPES) ?? 'cubic',
    tangentMode: validateOptionalString(params.tangent_mode || params.tangentMode) ?? 'auto' // 'auto', 'user', 'break', 'none'
  });
};

/**
 * Add audio track to sequence
 */
const handleAddAudioTrack: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequencePath = validateRequiredString(params.sequence_path || params.sequencePath, 'sequence_path');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'add_audio_track', {
    sequencePath,
    soundWavePath: validateOptionalString(params.sound_wave_path || params.soundWavePath),
    soundCuePath: validateOptionalString(params.sound_cue_path || params.soundCuePath),
    startFrame: validateOptionalNumber(params.start_frame || params.startFrame) ?? 0,
    rowIndex: validateOptionalNumber(params.row_index || params.rowIndex) ?? 0,
    volumeMultiplier: validateOptionalNumber(params.volume_multiplier || params.volumeMultiplier) ?? 1.0,
    pitchMultiplier: validateOptionalNumber(params.pitch_multiplier || params.pitchMultiplier) ?? 1.0,
    looping: validateOptionalBoolean(params.looping) ?? false
  });
};

/**
 * Add event track to sequence
 */
const handleAddEventTrack: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequencePath = validateRequiredString(params.sequence_path || params.sequencePath, 'sequence_path');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'add_event_track', {
    sequencePath,
    eventName: validateRequiredString(params.event_name || params.eventName, 'event_name'),
    frame: validateOptionalNumber(params.frame) ?? 0,
    receiverActorName: validateOptionalString(params.receiver_actor_name || params.receiverActorName),
    functionName: validateOptionalString(params.function_name || params.functionName),
    eventData: validateOptionalObject(params.event_data || params.eventData)
  });
};

/**
 * Configure camera rig
 */
const handleConfigureCameraRig: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const cameraActorName = validateRequiredString(params.camera_actor_name || params.cameraActorName, 'camera_actor_name');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'configure_camera_rig', {
    cameraActorName,
    rigType: validateOptionalEnum(params.rig_type || params.rigType, CAMERA_RIG_TYPES) ?? 'static',
    railPath: validateOptionalString(params.rail_path || params.railPath),
    craneArmLength: validateOptionalNumber(params.crane_arm_length || params.craneArmLength) ?? 300.0,
    craneYaw: validateOptionalNumber(params.crane_yaw || params.craneYaw),
    cranePitch: validateOptionalNumber(params.crane_pitch || params.cranePitch),
    handheldIntensity: validateOptionalNumber(params.handheld_intensity || params.handheldIntensity) ?? 1.0,
    handheldFrequency: validateOptionalNumber(params.handheld_frequency || params.handheldFrequency) ?? 1.0,
    lookAtActor: validateOptionalString(params.look_at_actor || params.lookAtActor),
    orbitRadius: validateOptionalNumber(params.orbit_radius || params.orbitRadius),
    orbitSpeed: validateOptionalNumber(params.orbit_speed || params.orbitSpeed)
  });
};

/**
 * Setup cinematic mode
 */
const handleSetupCinematicMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'setup_cinematic_mode', {
    mode: validateOptionalEnum(params.mode, CINEMATIC_MODES) ?? 'letterbox',
    letterboxRatio: validateOptionalNumber(params.letterbox_ratio || params.letterboxRatio) ?? 2.35,
    letterboxColor: validateOptionalString(params.letterbox_color || params.letterboxColor) ?? '#000000',
    fadeInTime: validateOptionalNumber(params.fade_in_time || params.fadeInTime) ?? 0.5,
    fadeOutTime: validateOptionalNumber(params.fade_out_time || params.fadeOutTime) ?? 0.5,
    hideHUD: validateOptionalBoolean(params.hide_hud || params.hideHUD) ?? true,
    hidePlayer: validateOptionalBoolean(params.hide_player || params.hidePlayer) ?? false,
    disableInput: validateOptionalBoolean(params.disable_input || params.disableInput) ?? true
  });
};

/**
 * Configure depth of field
 */
const handleConfigureDepthOfField: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const cameraActorName = validateRequiredString(params.camera_actor_name || params.cameraActorName, 'camera_actor_name');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'configure_depth_of_field', {
    cameraActorName,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    method: validateOptionalString(params.method) ?? 'cinematic', // 'gaussian', 'bokeh', 'cinematic'
    focalDistance: validateOptionalNumber(params.focal_distance || params.focalDistance) ?? 1000.0,
    depthBlurRadius: validateOptionalNumber(params.depth_blur_radius || params.depthBlurRadius) ?? 0.0,
    depthBlurAmount: validateOptionalNumber(params.depth_blur_amount || params.depthBlurAmount) ?? 1.0,
    aperture: validateOptionalNumber(params.aperture) ?? 4.0,
    bladeCount: validateOptionalNumber(params.blade_count || params.bladeCount) ?? 5,
    focusTarget: validateOptionalString(params.focus_target || params.focusTarget)
  });
};

/**
 * Add motion blur settings
 */
const handleConfigureMotionBlur: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'configure_motion_blur', {
    sequencePath: validateOptionalString(params.sequence_path || params.sequencePath),
    cameraActorName: validateOptionalString(params.camera_actor_name || params.cameraActorName),
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    amount: validateOptionalNumber(params.amount) ?? 0.5,
    max: validateOptionalNumber(params.max) ?? 5.0,
    perObjectSize: validateOptionalNumber(params.per_object_size || params.perObjectSize) ?? 0.5,
    targetFPS: validateOptionalNumber(params.target_fps || params.targetFPS) ?? 30
  });
};

/**
 * Create sub-sequence
 */
const handleCreateSubSequence: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const parentSequencePath = validateRequiredString(params.parent_sequence_path || params.parentSequencePath, 'parent_sequence_path');
  const subSequenceName = validateRequiredString(params.sub_sequence_name || params.subSequenceName, 'sub_sequence_name');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'create_sub_sequence', {
    parentSequencePath,
    subSequenceName,
    startFrame: validateOptionalNumber(params.start_frame || params.startFrame) ?? 0,
    duration: validateOptionalNumber(params.duration),
    timeScale: validateOptionalNumber(params.time_scale || params.timeScale) ?? 1.0,
    hierarchicalBias: validateOptionalNumber(params.hierarchical_bias || params.hierarchicalBias) ?? 0,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Play sequence
 */
const handlePlaySequence: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequencePath = validateRequiredString(params.sequence_path || params.sequencePath, 'sequence_path');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'play_sequence', {
    sequencePath,
    playRate: validateOptionalNumber(params.play_rate || params.playRate) ?? 1.0,
    startTime: validateOptionalNumber(params.start_time || params.startTime) ?? 0.0,
    loop: validateOptionalBoolean(params.loop) ?? false,
    restoreState: validateOptionalBoolean(params.restore_state || params.restoreState) ?? true,
    hideHUD: validateOptionalBoolean(params.hide_hud || params.hideHUD) ?? true,
    pauseAtEnd: validateOptionalBoolean(params.pause_at_end || params.pauseAtEnd) ?? false
  });
};

/**
 * Stop sequence
 */
const handleStopSequence: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'stop_sequence', {
    sequencePath: validateOptionalString(params.sequence_path || params.sequencePath),
    stopAll: validateOptionalBoolean(params.stop_all || params.stopAll) ?? false,
    restoreState: validateOptionalBoolean(params.restore_state || params.restoreState) ?? true
  });
};

/**
 * Get sequence info
 */
const handleGetSequenceInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sequencePath = validateRequiredString(params.sequence_path || params.sequencePath, 'sequence_path');

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'get_sequence_info', {
    sequencePath,
    includeTracks: validateOptionalBoolean(params.include_tracks || params.includeTracks) ?? true,
    includeKeyframes: validateOptionalBoolean(params.include_keyframes || params.includeKeyframes) ?? false,
    includeSubSequences: validateOptionalBoolean(params.include_sub_sequences || params.includeSubSequences) ?? true
  });
};

/**
 * List sequences
 */
const handleListSequences: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CINEMATICS, 'list_sequences', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const cinematicsHandlers: ActionHandlerMap = {
  // Sequence Creation
  create_level_sequence: handleCreateLevelSequence,
  create_sub_sequence: handleCreateSubSequence,

  // Tracks
  add_camera_track: handleAddCameraTrack,
  add_camera_cut: handleAddCameraCut,
  add_actor_track: handleAddActorTrack,
  add_audio_track: handleAddAudioTrack,
  add_event_track: handleAddEventTrack,
  add_keyframe: handleAddKeyframe,

  // Camera Configuration
  configure_camera_rig: handleConfigureCameraRig,
  configure_depth_of_field: handleConfigureDepthOfField,
  configure_motion_blur: handleConfigureMotionBlur,

  // Cinematic Mode
  setup_cinematic_mode: handleSetupCinematicMode,

  // Playback
  play_sequence: handlePlaySequence,
  stop_sequence: handleStopSequence,

  // Query
  get_sequence_info: handleGetSequenceInfo,
  list_sequences: handleListSequences
};

export const handleCinematicsTools = createCategoryHandler(Forge2Categories.CINEMATICS, cinematicsHandlers);

export default cinematicsHandlers;
