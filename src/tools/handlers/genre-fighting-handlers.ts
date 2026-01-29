/**
 * FORGE2: Fighting Genre Handlers
 * Complete fighting game mechanics (Street Fighter, Tekken, Smash style)
 *
 * Supports: Fighters with hitboxes/hurtboxes, movesets
 * Frame data: Startup, active, recovery frames
 * Systems: Combos, special meters, blocking, stages, training mode
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

export type MoveType = 'normal' | 'special' | 'super' | 'ultra' | 'command_normal' | 'throw' | 'universal';
export type AttackStrength = 'light' | 'medium' | 'heavy' | 'unique';
export type AttackHeight = 'high' | 'mid' | 'low' | 'overhead' | 'unblockable';
export type BlockType = 'stand' | 'crouch' | 'air' | 'parry' | 'just_defend';
export type MeterType = 'super' | 'rage' | 'burst' | 'guard' | 'custom';
export type StageTransitionType = 'wall_break' | 'floor_break' | 'balcony' | 'ring_out';

// ============================================================================
// Constants
// ============================================================================

const MOVE_TYPES = ['normal', 'special', 'super', 'ultra', 'command_normal', 'throw', 'universal'] as const;
const ATTACK_STRENGTHS = ['light', 'medium', 'heavy', 'unique'] as const;
const ATTACK_HEIGHTS = ['high', 'mid', 'low', 'overhead', 'unblockable'] as const;
const BLOCK_TYPES = ['stand', 'crouch', 'air', 'parry', 'just_defend'] as const;
const METER_TYPES = ['super', 'rage', 'burst', 'guard', 'custom'] as const;
const STAGE_TRANSITION_TYPES = ['wall_break', 'floor_break', 'balcony', 'ring_out'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a fighter
 */
const handleCreateFighter: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const fighterId = validateRequiredString(params.fighter_id || params.fighterId, 'fighter_id');
  const fighterName = validateRequiredString(params.fighter_name || params.fighterName, 'fighter_name');

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'create_fighter', {
    fighterId,
    fighterName,
    skeletonPath: params.skeleton_path || params.skeletonPath,
    meshPath: params.mesh_path || params.meshPath,
    health: params.health ?? 1000,
    walkSpeed: params.walk_speed ?? params.walkSpeed ?? 4.0,
    backWalkSpeed: params.back_walk_speed ?? params.backWalkSpeed ?? 3.0,
    dashSpeed: params.dash_speed ?? params.dashSpeed ?? 8.0,
    jumpHeight: params.jump_height ?? params.jumpHeight ?? 200,
    jumpForwardSpeed: params.jump_forward_speed ?? params.jumpForwardSpeed ?? 5.0,
    weight: params.weight ?? 100,
    archetype: params.archetype || 'balanced'
  });
};

/**
 * Setup fighter hitboxes/hurtboxes
 */
const handleSetupFighterBoxes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const fighterId = validateRequiredString(params.fighter_id || params.fighterId, 'fighter_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'setup_fighter_boxes', {
    fighterId,
    hurtboxes: params.hurtboxes,
    pushbox: params.pushbox,
    throwBox: params.throw_box || params.throwBox,
    hitboxPresets: params.hitbox_presets || params.hitboxPresets
  });
};

/**
 * Define moveset
 */
const handleDefineMoveSet: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const fighterId = validateRequiredString(params.fighter_id || params.fighterId, 'fighter_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'define_moveset', {
    fighterId,
    standNormals: params.stand_normals || params.standNormals,
    crouchNormals: params.crouch_normals || params.crouchNormals,
    airNormals: params.air_normals || params.airNormals,
    commandNormals: params.command_normals || params.commandNormals,
    specialMoves: params.special_moves || params.specialMoves,
    superMoves: params.super_moves || params.superMoves,
    throws: params.throws
  });
};

/**
 * Create a move
 */
const handleCreateMove: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const moveId = validateRequiredString(params.move_id || params.moveId, 'move_id');
  const moveName = validateRequiredString(params.move_name || params.moveName, 'move_name');

  const moveType = validateRequiredEnum(
    params.move_type || params.moveType,
    MOVE_TYPES,
    'move_type'
  );

  const attackStrength = validateOptionalEnum(
    params.attack_strength || params.attackStrength,
    ATTACK_STRENGTHS,
    'attack_strength'
  );

  const attackHeight = validateOptionalEnum(
    params.attack_height || params.attackHeight,
    ATTACK_HEIGHTS,
    'attack_height'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'create_move', {
    moveId,
    moveName,
    moveType,
    attackStrength: attackStrength || 'medium',
    attackHeight: attackHeight || 'mid',
    damage: params.damage ?? 100,
    chipDamage: params.chip_damage ?? params.chipDamage ?? 0,
    startupFrames: params.startup_frames ?? params.startupFrames ?? 8,
    activeFrames: params.active_frames ?? params.activeFrames ?? 4,
    recoveryFrames: params.recovery_frames ?? params.recoveryFrames ?? 16,
    blockAdvantage: params.block_advantage ?? params.blockAdvantage ?? -5,
    hitAdvantage: params.hit_advantage ?? params.hitAdvantage ?? 5,
    counterHitAdvantage: params.counter_hit_advantage ?? params.counterHitAdvantage ?? 8,
    animationPath: params.animation_path || params.animationPath,
    inputCommand: params.input_command || params.inputCommand,
    meterCost: params.meter_cost ?? params.meterCost ?? 0,
    meterGainOnHit: params.meter_gain_on_hit ?? params.meterGainOnHit ?? 10,
    meterGainOnBlock: params.meter_gain_on_block ?? params.meterGainOnBlock ?? 5
  });
};

/**
 * Setup frame data
 */
const handleSetupFrameData: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const moveId = validateRequiredString(params.move_id || params.moveId, 'move_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'setup_frame_data', {
    moveId,
    startupFrames: params.startup_frames ?? params.startupFrames,
    activeFrames: params.active_frames ?? params.activeFrames,
    recoveryFrames: params.recovery_frames ?? params.recoveryFrames,
    totalFrames: params.total_frames ?? params.totalFrames,
    blockstun: params.blockstun,
    hitstun: params.hitstun,
    cancelWindows: params.cancel_windows || params.cancelWindows,
    invincibilityFrames: params.invincibility_frames || params.invincibilityFrames,
    armorFrames: params.armor_frames || params.armorFrames
  });
};

/**
 * Configure combo system
 */
const handleConfigureComboSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'configure_combo_system', {
    maxJuggleCount: params.max_juggle_count ?? params.maxJuggleCount ?? 15,
    juggleDecay: params.juggle_decay ?? params.juggleDecay ?? 0.1,
    damageScaling: params.damage_scaling ?? params.damageScaling ?? 0.9,
    minDamagePercent: params.min_damage_percent ?? params.minDamagePercent ?? 0.1,
    hitstunDecay: params.hitstun_decay ?? params.hitstunDecay ?? 0.05,
    groundBounceLimit: params.ground_bounce_limit ?? params.groundBounceLimit ?? 1,
    wallBounceLimit: params.wall_bounce_limit ?? params.wallBounceLimit ?? 1,
    comboBreakerEnabled: params.combo_breaker_enabled ?? params.comboBreakerEnabled ?? false,
    comboBreakerCost: params.combo_breaker_cost ?? params.comboBreakerCost ?? 50
  });
};

/**
 * Create special meter
 */
const handleCreateSpecialMeter: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const meterType = validateRequiredEnum(
    params.meter_type || params.meterType,
    METER_TYPES,
    'meter_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'create_special_meter', {
    meterType,
    maxValue: params.max_value ?? params.maxValue ?? 100,
    segments: params.segments ?? 3,
    gainOnHit: params.gain_on_hit ?? params.gainOnHit ?? 10,
    gainOnDamage: params.gain_on_damage ?? params.gainOnDamage ?? 15,
    gainOnBlock: params.gain_on_block ?? params.gainOnBlock ?? 5,
    decayRate: params.decay_rate ?? params.decayRate ?? 0,
    preservesBetweenRounds: params.preserves_between_rounds ?? params.preservesBetweenRounds ?? false,
    activationThreshold: params.activation_threshold ?? params.activationThreshold ?? 100,
    activationEffect: params.activation_effect || params.activationEffect
  });
};

/**
 * Configure blocking
 */
const handleConfigureBlocking: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'configure_blocking', {
    standBlockHigh: params.stand_block_high ?? params.standBlockHigh ?? true,
    standBlockMid: params.stand_block_mid ?? params.standBlockMid ?? true,
    standBlockLow: params.stand_block_low ?? params.standBlockLow ?? false,
    crouchBlockHigh: params.crouch_block_high ?? params.crouchBlockHigh ?? false,
    crouchBlockMid: params.crouch_block_mid ?? params.crouchBlockMid ?? true,
    crouchBlockLow: params.crouch_block_low ?? params.crouchBlockLow ?? true,
    chipDamagePercent: params.chip_damage_percent ?? params.chipDamagePercent ?? 0.1,
    guardCrushEnabled: params.guard_crush_enabled ?? params.guardCrushEnabled ?? false,
    guardCrushThreshold: params.guard_crush_threshold ?? params.guardCrushThreshold ?? 100,
    pushbackOnBlock: params.pushback_on_block ?? params.pushbackOnBlock ?? 50
  });
};

/**
 * Setup parry system
 */
const handleSetupParrySystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const blockType = validateOptionalEnum(
    params.block_type || params.blockType,
    BLOCK_TYPES,
    'block_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'setup_parry_system', {
    enabled: params.enabled ?? true,
    blockType: blockType || 'parry',
    parryWindow: params.parry_window ?? params.parryWindow ?? 8,
    parryReward: params.parry_reward || params.parryReward || 'advantage',
    parryAdvantage: params.parry_advantage ?? params.parryAdvantage ?? 15,
    meterGainOnParry: params.meter_gain_on_parry ?? params.meterGainOnParry ?? 20,
    requiresInput: params.requires_input ?? params.requiresInput ?? true,
    inputCommand: params.input_command || params.inputCommand
  });
};

/**
 * Setup stage
 */
const handleSetupStage: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const stageId = validateRequiredString(params.stage_id || params.stageId, 'stage_id');
  const stageName = validateRequiredString(params.stage_name || params.stageName, 'stage_name');

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'setup_stage', {
    stageId,
    stageName,
    arenaWidth: params.arena_width ?? params.arenaWidth ?? 1000,
    arenaDepth: params.arena_depth ?? params.arenaDepth ?? 200,
    hasWalls: params.has_walls ?? params.hasWalls ?? true,
    wallBounceEnabled: params.wall_bounce_enabled ?? params.wallBounceEnabled ?? true,
    wallSplatEnabled: params.wall_splat_enabled ?? params.wallSplatEnabled ?? true,
    ringOutEnabled: params.ring_out_enabled ?? params.ringOutEnabled ?? false,
    cornerDistance: params.corner_distance ?? params.cornerDistance ?? 450,
    levelPath: params.level_path || params.levelPath
  });
};

/**
 * Add stage hazard
 */
const handleAddStageHazard: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const stageId = validateRequiredString(params.stage_id || params.stageId, 'stage_id');
  const hazardId = validateRequiredString(params.hazard_id || params.hazardId, 'hazard_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'add_stage_hazard', {
    stageId,
    hazardId,
    hazardType: params.hazard_type || params.hazardType,
    damage: params.damage ?? 50,
    triggerCondition: params.trigger_condition || params.triggerCondition,
    location: params.location,
    cooldown: params.cooldown ?? 10
  });
};

/**
 * Setup stage transition
 */
const handleSetupStageTransition: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const stageId = validateRequiredString(params.stage_id || params.stageId, 'stage_id');

  const transitionType = validateRequiredEnum(
    params.transition_type || params.transitionType,
    STAGE_TRANSITION_TYPES,
    'transition_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'setup_stage_transition', {
    stageId,
    transitionType,
    damageThreshold: params.damage_threshold ?? params.damageThreshold ?? 100,
    transitionDamage: params.transition_damage ?? params.transitionDamage ?? 50,
    targetStageId: params.target_stage_id || params.targetStageId,
    animationPath: params.animation_path || params.animationPath
  });
};

/**
 * Create training mode
 */
const handleCreateTrainingMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'create_training_mode', {
    hitboxDisplayEnabled: params.hitbox_display_enabled ?? params.hitboxDisplayEnabled ?? true,
    frameDataDisplayEnabled: params.frame_data_display_enabled ?? params.frameDataDisplayEnabled ?? true,
    inputDisplayEnabled: params.input_display_enabled ?? params.inputDisplayEnabled ?? true,
    damageDisplayEnabled: params.damage_display_enabled ?? params.damageDisplayEnabled ?? true,
    comboCounterEnabled: params.combo_counter_enabled ?? params.comboCounterEnabled ?? true,
    healthRegenEnabled: params.health_regen_enabled ?? params.healthRegenEnabled ?? true,
    meterRegenEnabled: params.meter_regen_enabled ?? params.meterRegenEnabled ?? true,
    recordPlaybackEnabled: params.record_playback_enabled ?? params.recordPlaybackEnabled ?? true,
    dummyOptions: params.dummy_options || params.dummyOptions
  });
};

/**
 * Configure dummy AI in training
 */
const handleConfigureDummyAI: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'configure_dummy_ai', {
    state: params.state || 'standing',
    blockMode: params.block_mode || params.blockMode || 'none',
    recoveryAction: params.recovery_action || params.recoveryAction || 'none',
    counterAttack: params.counter_attack || params.counterAttack || 'none',
    reversal: params.reversal,
    techOnKnockdown: params.tech_on_knockdown ?? params.techOnKnockdown ?? false,
    randomBlock: params.random_block ?? params.randomBlock ?? false
  });
};

/**
 * Add combo route
 */
const handleAddComboRoute: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const fighterId = validateRequiredString(params.fighter_id || params.fighterId, 'fighter_id');
  const comboId = validateRequiredString(params.combo_id || params.comboId, 'combo_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'add_combo_route', {
    fighterId,
    comboId,
    comboName: params.combo_name || params.comboName,
    moveSequence: params.move_sequence || params.moveSequence,
    difficulty: params.difficulty || 'medium',
    damage: params.damage,
    starterMoves: params.starter_moves || params.starterMoves,
    enderMoves: params.ender_moves || params.enderMoves,
    meterRequired: params.meter_required ?? params.meterRequired ?? 0
  });
};

/**
 * Get fighter frame data
 */
const handleGetFrameData: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const fighterId = validateRequiredString(params.fighter_id || params.fighterId, 'fighter_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'get_frame_data', {
    fighterId,
    moveId: params.move_id || params.moveId,
    includeAllMoves: params.include_all_moves ?? params.includeAllMoves ?? false
  });
};

/**
 * List all fighters
 */
const handleListFighters: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_FIGHTING, 'list_fighters', {
    archetype: params.archetype as string | undefined,
    includeFrameData: params.include_frame_data ?? params.includeFrameData ?? false
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const genreFightingHandlers: ActionHandlerMap = {
  create_fighter: handleCreateFighter,
  setup_fighter_boxes: handleSetupFighterBoxes,
  define_moveset: handleDefineMoveSet,
  create_move: handleCreateMove,
  setup_frame_data: handleSetupFrameData,
  configure_combo_system: handleConfigureComboSystem,
  create_special_meter: handleCreateSpecialMeter,
  configure_blocking: handleConfigureBlocking,
  setup_parry_system: handleSetupParrySystem,
  setup_stage: handleSetupStage,
  add_stage_hazard: handleAddStageHazard,
  setup_stage_transition: handleSetupStageTransition,
  create_training_mode: handleCreateTrainingMode,
  configure_dummy_ai: handleConfigureDummyAI,
  add_combo_route: handleAddComboRoute,
  get_frame_data: handleGetFrameData,
  list_fighters: handleListFighters
};

/**
 * Main genre fighting handler - routes to specific action handlers
 */
export const handleGenreFightingTools = createCategoryHandler(Forge2Categories.GENRE_FIGHTING, genreFightingHandlers);

/**
 * Get available genre fighting actions
 */
export function getGenreFightingActions(): string[] {
  return Object.keys(genreFightingHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeGenreFightingHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = genreFightingHandlers[action];
  if (!handler) {
    throw new Error(`Unknown genre fighting action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
