/**
 * FORGE2: Puzzle Genre Handlers
 * Complete puzzle game mechanics (Portal, The Witness, Baba Is You style)
 *
 * Supports: Puzzle elements, logic systems, physics puzzles
 * Systems: Sequence puzzles, environmental puzzles, hints, validation
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

export type PuzzleElementType = 'button' | 'lever' | 'pressure_plate' | 'laser' | 'portal' | 'mirror' | 'cube' | 'ball' | 'gate' | 'platform';
export type LogicGateType = 'and' | 'or' | 'not' | 'xor' | 'nand' | 'nor' | 'timer' | 'toggle' | 'counter';
export type PuzzleType = 'physics' | 'logic' | 'sequence' | 'environmental' | 'pattern' | 'navigation' | 'time';
export type HintLevel = 'subtle' | 'moderate' | 'direct' | 'solution';

// ============================================================================
// Constants
// ============================================================================

const PUZZLE_ELEMENT_TYPES = ['button', 'lever', 'pressure_plate', 'laser', 'portal', 'mirror', 'cube', 'ball', 'gate', 'platform'] as const;
const LOGIC_GATE_TYPES = ['and', 'or', 'not', 'xor', 'nand', 'nor', 'timer', 'toggle', 'counter'] as const;
const PUZZLE_TYPES = ['physics', 'logic', 'sequence', 'environmental', 'pattern', 'navigation', 'time'] as const;
const HINT_LEVELS = ['subtle', 'moderate', 'direct', 'solution'] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create puzzle element
 */
const handleCreatePuzzleElement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const elementId = validateRequiredString(params.element_id || params.elementId, 'element_id');

  const elementType = validateRequiredEnum(
    params.element_type || params.elementType,
    PUZZLE_ELEMENT_TYPES,
    'element_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'create_puzzle_element', {
    elementId,
    elementType,
    meshPath: params.mesh_path || params.meshPath,
    location: params.location,
    rotation: params.rotation,
    interactable: params.interactable ?? true,
    initialState: params.initial_state ?? params.initialState ?? false,
    canReset: params.can_reset ?? params.canReset ?? true,
    activationSound: params.activation_sound || params.activationSound,
    deactivationSound: params.deactivation_sound || params.deactivationSound,
    vfxOnActivate: params.vfx_on_activate || params.vfxOnActivate
  });
};

/**
 * Configure logic system
 */
const handleConfigureLogicSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'configure_logic_system', {
    tickRate: params.tick_rate ?? params.tickRate ?? 0.1,
    signalDelay: params.signal_delay ?? params.signalDelay ?? 0,
    visualizeConnections: params.visualize_connections ?? params.visualizeConnections ?? false,
    connectionColor: params.connection_color || params.connectionColor,
    activeConnectionColor: params.active_connection_color || params.activeConnectionColor,
    debugMode: params.debug_mode ?? params.debugMode ?? false
  });
};

/**
 * Create logic gate
 */
const handleCreateLogicGate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const gateId = validateRequiredString(params.gate_id || params.gateId, 'gate_id');

  const gateType = validateRequiredEnum(
    params.gate_type || params.gateType,
    LOGIC_GATE_TYPES,
    'gate_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'create_logic_gate', {
    gateId,
    gateType,
    location: params.location,
    inputCount: params.input_count ?? params.inputCount ?? 2,
    timerDuration: params.timer_duration ?? params.timerDuration ?? 1.0,
    counterTarget: params.counter_target ?? params.counterTarget ?? 1,
    invertOutput: params.invert_output ?? params.invertOutput ?? false,
    hidden: params.hidden ?? true
  });
};

/**
 * Connect puzzle elements
 */
const handleConnectPuzzleElements: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const sourceId = validateRequiredString(params.source_id || params.sourceId, 'source_id');
  const targetId = validateRequiredString(params.target_id || params.targetId, 'target_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'connect_puzzle_elements', {
    sourceId,
    targetId,
    inputIndex: params.input_index ?? params.inputIndex ?? 0,
    delay: params.delay ?? 0,
    inverted: params.inverted ?? false,
    showConnection: params.show_connection ?? params.showConnection ?? false
  });
};

/**
 * Setup physics puzzles
 */
const handleSetupPhysicsPuzzles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'setup_physics_puzzles', {
    gravityScale: params.gravity_scale ?? params.gravityScale ?? 1.0,
    frictionDefault: params.friction_default ?? params.frictionDefault ?? 0.5,
    bouncinessDefault: params.bounciness_default ?? params.bouncinessDefault ?? 0.3,
    physicsObjectsRespawn: params.physics_objects_respawn ?? params.physicsObjectsRespawn ?? true,
    respawnDelay: params.respawn_delay ?? params.respawnDelay ?? 3,
    massAffectsPressurePlates: params.mass_affects_pressure_plates ?? params.massAffectsPressurePlates ?? true,
    trajectoryPreview: params.trajectory_preview ?? params.trajectoryPreview ?? false
  });
};

/**
 * Create physics object
 */
const handleCreatePhysicsObject: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const objectId = validateRequiredString(params.object_id || params.objectId, 'object_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'create_physics_object', {
    objectId,
    meshPath: params.mesh_path || params.meshPath,
    location: params.location,
    mass: params.mass ?? 10,
    friction: params.friction ?? 0.5,
    bounciness: params.bounciness ?? 0.3,
    canBePickedUp: params.can_be_picked_up ?? params.canBePickedUp ?? true,
    canBeThrown: params.can_be_thrown ?? params.canBeThrown ?? true,
    throwForce: params.throw_force ?? params.throwForce ?? 1000,
    respawnLocation: params.respawn_location || params.respawnLocation,
    affectedByLaser: params.affected_by_laser ?? params.affectedByLaser ?? false
  });
};

/**
 * Create sequence puzzle
 */
const handleCreateSequencePuzzle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const puzzleId = validateRequiredString(params.puzzle_id || params.puzzleId, 'puzzle_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'create_sequence_puzzle', {
    puzzleId,
    sequence: params.sequence,
    allowPartialProgress: params.allow_partial_progress ?? params.allowPartialProgress ?? true,
    resetOnMistake: params.reset_on_mistake ?? params.resetOnMistake ?? true,
    showProgress: params.show_progress ?? params.showProgress ?? true,
    timeLimit: params.time_limit ?? params.timeLimit,
    correctSound: params.correct_sound || params.correctSound,
    wrongSound: params.wrong_sound || params.wrongSound,
    completionSound: params.completion_sound || params.completionSound
  });
};

/**
 * Configure environmental puzzle
 */
const handleConfigureEnvironmentalPuzzle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const puzzleId = validateRequiredString(params.puzzle_id || params.puzzleId, 'puzzle_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'configure_environmental_puzzle', {
    puzzleId,
    puzzleType: params.puzzle_type || params.puzzleType,
    usePerspective: params.use_perspective ?? params.usePerspective ?? false,
    useShadows: params.use_shadows ?? params.useShadows ?? false,
    useReflection: params.use_reflection ?? params.useReflection ?? false,
    viewpoint: params.viewpoint,
    solutionTolerance: params.solution_tolerance ?? params.solutionTolerance ?? 5
  });
};

/**
 * Setup hint system
 */
const handleSetupHintSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'setup_hint_system', {
    enabled: params.enabled ?? true,
    hintsPerPuzzle: params.hints_per_puzzle ?? params.hintsPerPuzzle ?? 3,
    hintCooldown: params.hint_cooldown ?? params.hintCooldown ?? 60,
    progressiveHints: params.progressive_hints ?? params.progressiveHints ?? true,
    hintCost: params.hint_cost ?? params.hintCost ?? 0,
    autoHintAfter: params.auto_hint_after ?? params.autoHintAfter ?? 0,
    skipEnabled: params.skip_enabled ?? params.skipEnabled ?? false,
    skipPenalty: params.skip_penalty ?? params.skipPenalty ?? 0
  });
};

/**
 * Add hint
 */
const handleAddHint: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const puzzleId = validateRequiredString(params.puzzle_id || params.puzzleId, 'puzzle_id');

  const hintLevel = validateRequiredEnum(
    params.hint_level || params.hintLevel,
    HINT_LEVELS,
    'hint_level'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'add_hint', {
    puzzleId,
    hintLevel,
    hintText: params.hint_text || params.hintText,
    highlightElements: params.highlight_elements || params.highlightElements,
    showArrow: params.show_arrow ?? params.showArrow ?? false,
    arrowTarget: params.arrow_target || params.arrowTarget,
    voiceOver: params.voice_over || params.voiceOver
  });
};

/**
 * Create puzzle validation
 */
const handleCreatePuzzleValidation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const puzzleId = validateRequiredString(params.puzzle_id || params.puzzleId, 'puzzle_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'create_puzzle_validation', {
    puzzleId,
    validationMode: params.validation_mode || params.validationMode || 'automatic',
    requiredStates: params.required_states || params.requiredStates,
    allowMultipleSolutions: params.allow_multiple_solutions ?? params.allowMultipleSolutions ?? false,
    alternativeSolutions: params.alternative_solutions || params.alternativeSolutions,
    validationDelay: params.validation_delay ?? params.validationDelay ?? 0.5,
    onSolved: params.on_solved || params.onSolved,
    reward: params.reward
  });
};

/**
 * Create laser system
 */
const handleCreateLaserSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'create_laser_system', {
    laserColor: params.laser_color || params.laserColor,
    laserWidth: params.laser_width ?? params.laserWidth ?? 5,
    maxBounces: params.max_bounces ?? params.maxBounces ?? 10,
    maxDistance: params.max_distance ?? params.maxDistance ?? 10000,
    canActivateReceivers: params.can_activate_receivers ?? params.canActivateReceivers ?? true,
    damagesPlayer: params.damages_player ?? params.damagesPlayer ?? false,
    damageAmount: params.damage_amount ?? params.damageAmount ?? 10
  });
};

/**
 * Create portal system
 */
const handleCreatePortalSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'create_portal_system', {
    portalColor1: params.portal_color_1 || params.portalColor1,
    portalColor2: params.portal_color_2 || params.portalColor2,
    maxPortals: params.max_portals ?? params.maxPortals ?? 2,
    preserveMomentum: params.preserve_momentum ?? params.preserveMomentum ?? true,
    portalableSurfaces: params.portalable_surfaces || params.portalableSurfaces,
    physicsObjectsCanPass: params.physics_objects_can_pass ?? params.physicsObjectsCanPass ?? true,
    lasersCanPass: params.lasers_can_pass ?? params.lasersCanPass ?? true,
    seamlessTransition: params.seamless_transition ?? params.seamlessTransition ?? true
  });
};

/**
 * Check puzzle state
 */
const handleCheckPuzzleState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const puzzleId = validateRequiredString(params.puzzle_id || params.puzzleId, 'puzzle_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'check_puzzle_state', {
    puzzleId,
    includeElementStates: params.include_element_states ?? params.includeElementStates ?? true
  });
};

/**
 * Reset puzzle
 */
const handleResetPuzzle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const puzzleId = validateRequiredString(params.puzzle_id || params.puzzleId, 'puzzle_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'reset_puzzle', {
    puzzleId,
    resetPhysicsObjects: params.reset_physics_objects ?? params.resetPhysicsObjects ?? true,
    resetProgress: params.reset_progress ?? params.resetProgress ?? true
  });
};

/**
 * List puzzles
 */
const handleListPuzzles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const puzzleType = validateOptionalEnum(
    params.puzzle_type || params.puzzleType,
    PUZZLE_TYPES,
    'puzzle_type'
  );

  return executeForge2Action(tools, Forge2Categories.GENRE_PUZZLE, 'list_puzzles', {
    puzzleType,
    includeSolved: params.include_solved ?? params.includeSolved ?? true
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const genrePuzzleHandlers: ActionHandlerMap = {
  create_puzzle_element: handleCreatePuzzleElement,
  configure_logic_system: handleConfigureLogicSystem,
  create_logic_gate: handleCreateLogicGate,
  connect_puzzle_elements: handleConnectPuzzleElements,
  setup_physics_puzzles: handleSetupPhysicsPuzzles,
  create_physics_object: handleCreatePhysicsObject,
  create_sequence_puzzle: handleCreateSequencePuzzle,
  configure_environmental_puzzle: handleConfigureEnvironmentalPuzzle,
  setup_hint_system: handleSetupHintSystem,
  add_hint: handleAddHint,
  create_puzzle_validation: handleCreatePuzzleValidation,
  create_laser_system: handleCreateLaserSystem,
  create_portal_system: handleCreatePortalSystem,
  check_puzzle_state: handleCheckPuzzleState,
  reset_puzzle: handleResetPuzzle,
  list_puzzles: handleListPuzzles
};

/**
 * Main genre puzzle handler - routes to specific action handlers
 */
export const handleGenrePuzzleTools = createCategoryHandler(Forge2Categories.GENRE_PUZZLE, genrePuzzleHandlers);

/**
 * Get available genre puzzle actions
 */
export function getGenrePuzzleActions(): string[] {
  return Object.keys(genrePuzzleHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeGenrePuzzleHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = genrePuzzleHandlers[action];
  if (!handler) {
    throw new Error(`Unknown genre puzzle action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
