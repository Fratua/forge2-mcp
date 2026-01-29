/**
 * FORGE2: Dialogue System Handlers
 * Complete dialogue tree management including nodes, speakers, conditions, and consequences
 *
 * Supports dialogue trees with branching conversations, speaker profiles,
 * choice conditions, and gameplay consequences.
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

/**
 * Validate required string parameter
 */
function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value;
}

/**
 * Validate optional enum parameter
 * @internal Reserved for future use
 */
function _validateOptionalEnum<T extends string>(
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
// Suppress unused warning
void _validateOptionalEnum;

/**
 * Validate required enum parameter
 */
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

export type DialogueNodeType = 'npc_speech' | 'player_choice' | 'condition' | 'event' | 'random' | 'branch';

export type ConditionType =
  | 'has_item'
  | 'quest_complete'
  | 'quest_active'
  | 'skill_check'
  | 'reputation_check'
  | 'stat_check'
  | 'variable_check'
  | 'custom';

export type ConsequenceType =
  | 'set_variable'
  | 'give_item'
  | 'take_item'
  | 'start_quest'
  | 'complete_objective'
  | 'change_reputation'
  | 'trigger_event'
  | 'play_animation'
  | 'play_sound'
  | 'change_camera'
  | 'custom';

export interface DialogueNode {
  nodeId: string;
  nodeType: DialogueNodeType;
  speakerId?: string;
  text?: string;
  audioPath?: string;
  animationPath?: string;
  duration?: number;
}

export interface SpeakerProfile {
  speakerId: string;
  speakerName: string;
  portraitPath?: string;
  voicePath?: string;
  defaultAnimation?: string;
}

export interface DialogueCondition {
  conditionType: ConditionType;
  target?: string;
  value?: unknown;
  operator?: string;
}

export interface DialogueConsequence {
  consequenceType: ConsequenceType;
  target?: string;
  value?: unknown;
}

// ============================================================================
// Constants
// ============================================================================

const NODE_TYPES = ['npc_speech', 'player_choice', 'condition', 'event', 'random', 'branch'] as const;

const CONDITION_TYPES = [
  'has_item', 'quest_complete', 'quest_active', 'skill_check',
  'reputation_check', 'stat_check', 'variable_check', 'custom'
] as const;

const CONSEQUENCE_TYPES = [
  'set_variable', 'give_item', 'take_item', 'start_quest',
  'complete_objective', 'change_reputation', 'trigger_event',
  'play_animation', 'play_sound', 'change_camera', 'custom'
] as const;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a new dialogue tree
 */
const handleCreateDialogueTree: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const treeName = validateRequiredString(params.tree_name || params.treeName, 'tree_name');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'create_dialogue_tree', {
    treeId,
    treeName,
    description: params.description as string | undefined,
    startNodeId: params.start_node_id || params.startNodeId,
    defaultSpeakerId: params.default_speaker_id || params.defaultSpeakerId,
    tags: params.tags as string[] | undefined
  });
};

/**
 * Add a dialogue node to a tree
 */
const handleAddDialogueNode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const nodeId = validateRequiredString(params.node_id || params.nodeId, 'node_id');

  const nodeType = validateRequiredEnum(
    params.node_type || params.nodeType,
    NODE_TYPES,
    'node_type'
  );

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'add_dialogue_node', {
    treeId,
    nodeId,
    nodeType,
    speakerId: params.speaker_id || params.speakerId,
    text: params.text as string | undefined,
    localizedTextKey: params.localized_text_key || params.localizedTextKey,
    audioPath: params.audio_path || params.audioPath,
    animationPath: params.animation_path || params.animationPath,
    duration: params.duration,
    autoAdvance: params.auto_advance ?? params.autoAdvance,
    skippable: params.skippable ?? true
  });
};

/**
 * Connect dialogue nodes
 */
const handleConnectDialogueNodes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const fromNodeId = validateRequiredString(params.from_node_id || params.fromNodeId, 'from_node_id');
  const toNodeId = validateRequiredString(params.to_node_id || params.toNodeId, 'to_node_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'connect_dialogue_nodes', {
    treeId,
    fromNodeId,
    toNodeId,
    connectionLabel: params.connection_label || params.connectionLabel,
    isDefault: params.is_default ?? params.isDefault ?? false,
    priority: params.priority
  });
};

/**
 * Create a speaker profile
 */
const handleCreateSpeakerProfile: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const speakerId = validateRequiredString(params.speaker_id || params.speakerId, 'speaker_id');
  const speakerName = validateRequiredString(params.speaker_name || params.speakerName, 'speaker_name');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'create_speaker_profile', {
    speakerId,
    speakerName,
    displayName: params.display_name || params.displayName,
    portraitPath: params.portrait_path || params.portraitPath,
    voicePath: params.voice_path || params.voicePath,
    defaultAnimation: params.default_animation || params.defaultAnimation,
    textColor: params.text_color || params.textColor,
    nameColor: params.name_color || params.nameColor
  });
};

/**
 * Add a choice condition to a dialogue node
 */
const handleAddChoiceCondition: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const nodeId = validateRequiredString(params.node_id || params.nodeId, 'node_id');
  const choiceIndex = params.choice_index ?? params.choiceIndex;

  const conditionType = validateRequiredEnum(
    params.condition_type || params.conditionType,
    CONDITION_TYPES,
    'condition_type'
  );

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'add_choice_condition', {
    treeId,
    nodeId,
    choiceIndex,
    conditionType,
    target: params.target as string | undefined,
    value: params.value,
    operator: params.operator,
    hideIfFailed: params.hide_if_failed ?? params.hideIfFailed ?? false,
    failureText: params.failure_text || params.failureText
  });
};

/**
 * Add a consequence to a dialogue node
 */
const handleAddConsequence: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const nodeId = validateRequiredString(params.node_id || params.nodeId, 'node_id');

  const consequenceType = validateRequiredEnum(
    params.consequence_type || params.consequenceType,
    CONSEQUENCE_TYPES,
    'consequence_type'
  );

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'add_consequence', {
    treeId,
    nodeId,
    consequenceType,
    target: params.target as string | undefined,
    value: params.value,
    delay: params.delay,
    executeOnEntry: params.execute_on_entry ?? params.executeOnEntry ?? true
  });
};

/**
 * Add player choice options to a choice node
 */
const handleAddPlayerChoice: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const nodeId = validateRequiredString(params.node_id || params.nodeId, 'node_id');
  const choiceText = validateRequiredString(params.choice_text || params.choiceText, 'choice_text');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'add_player_choice', {
    treeId,
    nodeId,
    choiceText,
    localizedKey: params.localized_key || params.localizedKey,
    nextNodeId: params.next_node_id || params.nextNodeId,
    tooltip: params.tooltip as string | undefined,
    isBackOption: params.is_back_option ?? params.isBackOption ?? false,
    isExitOption: params.is_exit_option ?? params.isExitOption ?? false
  });
};

/**
 * Configure dialogue audio playback
 */
const handleConfigureDialogueAudio: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'configure_dialogue_audio', {
    treeId,
    nodeId: params.node_id || params.nodeId,
    audioPath: params.audio_path || params.audioPath,
    volume: params.volume,
    pitch: params.pitch,
    subtitlesEnabled: params.subtitles_enabled ?? params.subtitlesEnabled ?? true,
    lipSyncEnabled: params.lip_sync_enabled ?? params.lipSyncEnabled ?? true
  });
};

/**
 * Configure dialogue camera
 */
const handleConfigureDialogueCamera: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'configure_dialogue_camera', {
    treeId,
    nodeId: params.node_id || params.nodeId,
    cameraMode: params.camera_mode || params.cameraMode,
    focusActor: params.focus_actor || params.focusActor,
    cameraOffset: params.camera_offset || params.cameraOffset,
    blendTime: params.blend_time || params.blendTime
  });
};

/**
 * Set dialogue trigger for NPC
 */
const handleSetDialogueTrigger: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const npcId = validateRequiredString(params.npc_id || params.npcId, 'npc_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'set_dialogue_trigger', {
    treeId,
    npcId,
    triggerRadius: params.trigger_radius || params.triggerRadius,
    requireInteraction: params.require_interaction ?? params.requireInteraction ?? true,
    cooldown: params.cooldown,
    oneTime: params.one_time ?? params.oneTime ?? false,
    conditions: params.conditions
  });
};

/**
 * Link dialogue to quest
 */
const handleLinkToQuest: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');
  const questId = validateRequiredString(params.quest_id || params.questId, 'quest_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'link_to_quest', {
    treeId,
    questId,
    triggerOnQuestStart: params.trigger_on_quest_start ?? params.triggerOnQuestStart ?? false,
    triggerOnQuestComplete: params.trigger_on_quest_complete ?? params.triggerOnQuestComplete ?? false,
    objectiveId: params.objective_id || params.objectiveId
  });
};

/**
 * Get dialogue tree data
 */
const handleGetDialogueTree: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'get_dialogue_tree', {
    treeId,
    includeNodes: params.include_nodes ?? params.includeNodes ?? true,
    includeConnections: params.include_connections ?? params.includeConnections ?? true
  });
};

/**
 * Start dialogue
 */
const handleStartDialogue: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'start_dialogue', {
    treeId,
    startNodeId: params.start_node_id || params.startNodeId,
    participantIds: params.participant_ids || params.participantIds,
    context: params.context
  });
};

/**
 * Advance dialogue to next node
 */
const handleAdvanceDialogue: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'advance_dialogue', {
    treeId,
    choiceIndex: params.choice_index ?? params.choiceIndex,
    forceSkip: params.force_skip ?? params.forceSkip ?? false
  });
};

/**
 * End dialogue
 */
const handleEndDialogue: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'end_dialogue', {
    treeId,
    triggerConsequences: params.trigger_consequences ?? params.triggerConsequences ?? true
  });
};

/**
 * List all dialogue trees
 */
const handleListDialogueTrees: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'list_dialogue_trees', {
    speakerId: params.speaker_id || params.speakerId,
    tags: params.tags as string[] | undefined,
    includeNodeCount: params.include_node_count ?? params.includeNodeCount ?? false
  });
};

/**
 * Delete dialogue tree
 */
const handleDeleteDialogueTree: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'delete_dialogue_tree', {
    treeId
  });
};

/**
 * Clone dialogue tree
 */
const handleCloneDialogueTree: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const sourceTreeId = validateRequiredString(params.source_tree_id || params.sourceTreeId, 'source_tree_id');
  const newTreeId = validateRequiredString(params.new_tree_id || params.newTreeId, 'new_tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'clone_dialogue_tree', {
    sourceTreeId,
    newTreeId,
    newTreeName: params.new_tree_name || params.newTreeName
  });
};

/**
 * Validate dialogue tree structure
 */
const handleValidateDialogueTree: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  const treeId = validateRequiredString(params.tree_id || params.treeId, 'tree_id');

  return executeForge2Action(tools, Forge2Categories.DIALOGUE, 'validate_dialogue_tree', {
    treeId,
    checkDeadEnds: params.check_dead_ends ?? params.checkDeadEnds ?? true,
    checkOrphans: params.check_orphans ?? params.checkOrphans ?? true,
    checkConditions: params.check_conditions ?? params.checkConditions ?? true
  });
};

// ============================================================================
// Handler Map and Export
// ============================================================================

const dialogueHandlers: ActionHandlerMap = {
  create_dialogue_tree: handleCreateDialogueTree,
  add_dialogue_node: handleAddDialogueNode,
  connect_dialogue_nodes: handleConnectDialogueNodes,
  create_speaker_profile: handleCreateSpeakerProfile,
  add_choice_condition: handleAddChoiceCondition,
  add_consequence: handleAddConsequence,
  add_player_choice: handleAddPlayerChoice,
  configure_dialogue_audio: handleConfigureDialogueAudio,
  configure_dialogue_camera: handleConfigureDialogueCamera,
  set_dialogue_trigger: handleSetDialogueTrigger,
  link_to_quest: handleLinkToQuest,
  get_dialogue_tree: handleGetDialogueTree,
  start_dialogue: handleStartDialogue,
  advance_dialogue: handleAdvanceDialogue,
  end_dialogue: handleEndDialogue,
  list_dialogue_trees: handleListDialogueTrees,
  delete_dialogue_tree: handleDeleteDialogueTree,
  clone_dialogue_tree: handleCloneDialogueTree,
  validate_dialogue_tree: handleValidateDialogueTree
};

/**
 * Main dialogue handler - routes to specific action handlers
 */
export const handleDialogueTools = createCategoryHandler(Forge2Categories.DIALOGUE, dialogueHandlers);

/**
 * Get available dialogue actions
 */
export function getDialogueActions(): string[] {
  return Object.keys(dialogueHandlers);
}

/**
 * Direct handler execution for testing/internal use
 */
export async function executeDialogueHandler(
  tools: ITools,
  action: string,
  args: HandlerArgs
): Promise<Record<string, unknown>> {
  const handler = dialogueHandlers[action];
  if (!handler) {
    throw new Error(`Unknown dialogue action: ${action}`);
  }
  const response = await handler(tools, args);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
