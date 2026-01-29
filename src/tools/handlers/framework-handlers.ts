/**
 * FORGE2: Game Framework Handlers
 * Handles game mode, game state, player state, player controller, character, and game instance
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
// Game Mode Action Handlers
// ============================================================================

/**
 * Create game mode
 */
const handleCreateGameMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const modeName = validateRequiredString(params.mode_name || params.modeName, 'mode_name');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'create_game_mode', {
    modeName,
    parentClass: validateOptionalString(params.parent_class || params.parentClass) ?? 'McpGameMode',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Blueprints/GameModes/${modeName}`,
    defaultPawnClass: validateOptionalString(params.default_pawn_class || params.defaultPawnClass),
    playerControllerClass: validateOptionalString(params.player_controller_class || params.playerControllerClass)
  });
};

/**
 * Configure game mode
 */
const handleConfigureGameMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const modePath = validateRequiredString(params.mode_path || params.modePath, 'mode_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_game_mode', {
    modePath,
    defaultMapName: validateOptionalString(params.default_map_name || params.defaultMapName),
    matchDuration: validateOptionalNumber(params.match_duration || params.matchDuration),
    respawnDelay: validateOptionalNumber(params.respawn_delay || params.respawnDelay) ?? 5.0,
    spectatorClass: validateOptionalString(params.spectator_class || params.spectatorClass),
    useSeamlessTravel: validateOptionalBoolean(params.use_seamless_travel || params.useSeamlessTravel) ?? false
  });
};

/**
 * Configure match settings
 */
const handleConfigureMatchSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const modePath = validateRequiredString(params.mode_path || params.modePath, 'mode_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_match_settings', {
    modePath,
    minPlayers: validateOptionalNumber(params.min_players || params.minPlayers) ?? 1,
    maxPlayers: validateOptionalNumber(params.max_players || params.maxPlayers) ?? 16,
    teamBased: validateOptionalBoolean(params.team_based || params.teamBased) ?? false,
    numTeams: validateOptionalNumber(params.num_teams || params.numTeams) ?? 2,
    winConditions: validateOptionalArray(params.win_conditions || params.winConditions) ?? []
  });
};

// ============================================================================
// Game State Action Handlers
// ============================================================================

/**
 * Create game state
 */
const handleCreateGameState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const stateName = validateRequiredString(params.state_name || params.stateName, 'state_name');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'create_game_state', {
    stateName,
    parentClass: validateOptionalString(params.parent_class || params.parentClass) ?? 'McpGameState',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Blueprints/GameState/${stateName}`
  });
};

/**
 * Configure game state
 */
const handleConfigureGameState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const statePath = validateRequiredString(params.state_path || params.statePath, 'state_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_game_state', {
    statePath,
    replicatedVariables: validateOptionalArray(params.replicated_variables || params.replicatedVariables) ?? [],
    matchStateEnum: validateOptionalString(params.match_state_enum || params.matchStateEnum),
    trackPlayerScores: validateOptionalBoolean(params.track_player_scores || params.trackPlayerScores) ?? true,
    trackTeamScores: validateOptionalBoolean(params.track_team_scores || params.trackTeamScores) ?? false
  });
};

/**
 * Add game state variable
 */
const handleAddGameStateVariable: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const statePath = validateRequiredString(params.state_path || params.statePath, 'state_path');
  const variableName = validateRequiredString(params.variable_name || params.variableName, 'variable_name');
  const variableType = validateRequiredString(params.variable_type || params.variableType, 'variable_type');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'add_game_state_variable', {
    statePath,
    variableName,
    variableType,
    replicated: validateOptionalBoolean(params.replicated) ?? true,
    replicationCondition: validateOptionalString(params.replication_condition || params.replicationCondition) ?? 'always',
    defaultValue: validateOptionalString(params.default_value || params.defaultValue)
  });
};

// ============================================================================
// Player State Action Handlers
// ============================================================================

/**
 * Create player state
 */
const handleCreatePlayerState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const stateName = validateRequiredString(params.state_name || params.stateName, 'state_name');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'create_player_state', {
    stateName,
    parentClass: validateOptionalString(params.parent_class || params.parentClass) ?? 'McpPlayerState',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Blueprints/PlayerState/${stateName}`
  });
};

/**
 * Configure player state
 */
const handleConfigurePlayerState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const statePath = validateRequiredString(params.state_path || params.statePath, 'state_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_player_state', {
    statePath,
    trackKills: validateOptionalBoolean(params.track_kills || params.trackKills) ?? true,
    trackDeaths: validateOptionalBoolean(params.track_deaths || params.trackDeaths) ?? true,
    trackScore: validateOptionalBoolean(params.track_score || params.trackScore) ?? true,
    customStats: validateOptionalArray(params.custom_stats || params.customStats) ?? []
  });
};

// ============================================================================
// Player Controller Action Handlers
// ============================================================================

/**
 * Create player controller
 */
const handleCreatePlayerController: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const controllerName = validateRequiredString(params.controller_name || params.controllerName, 'controller_name');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'create_player_controller', {
    controllerName,
    parentClass: validateOptionalString(params.parent_class || params.parentClass) ?? 'McpPlayerController',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Blueprints/PlayerController/${controllerName}`
  });
};

/**
 * Configure player controller
 */
const handleConfigurePlayerController: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const controllerPath = validateRequiredString(params.controller_path || params.controllerPath, 'controller_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_player_controller', {
    controllerPath,
    inputMappingContext: validateOptionalString(params.input_mapping_context || params.inputMappingContext),
    hudClass: validateOptionalString(params.hud_class || params.hudClass),
    cameraManagerClass: validateOptionalString(params.camera_manager_class || params.cameraManagerClass),
    autoManageActiveCameraTarget: validateOptionalBoolean(params.auto_manage_active_camera_target || params.autoManageActiveCameraTarget) ?? true
  });
};

// ============================================================================
// Character Action Handlers
// ============================================================================

/**
 * Create character class
 */
const handleCreateCharacterClass: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const characterName = validateRequiredString(params.character_name || params.characterName, 'character_name');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'create_character_class', {
    characterName,
    parentClass: validateOptionalString(params.parent_class || params.parentClass) ?? 'McpCharacter',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Blueprints/Characters/${characterName}`,
    skeletalMesh: validateOptionalString(params.skeletal_mesh || params.skeletalMesh),
    animBlueprint: validateOptionalString(params.anim_blueprint || params.animBlueprint)
  });
};

/**
 * Configure character movement
 */
const handleConfigureCharacterMovement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const characterPath = validateRequiredString(params.character_path || params.characterPath, 'character_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_character_movement', {
    characterPath,
    maxWalkSpeed: validateOptionalNumber(params.max_walk_speed || params.maxWalkSpeed) ?? 600.0,
    maxSprintSpeed: validateOptionalNumber(params.max_sprint_speed || params.maxSprintSpeed) ?? 1000.0,
    jumpZVelocity: validateOptionalNumber(params.jump_z_velocity || params.jumpZVelocity) ?? 600.0,
    airControl: validateOptionalNumber(params.air_control || params.airControl) ?? 0.35,
    gravityScale: validateOptionalNumber(params.gravity_scale || params.gravityScale) ?? 1.0,
    canCrouch: validateOptionalBoolean(params.can_crouch || params.canCrouch) ?? true,
    canSwim: validateOptionalBoolean(params.can_swim || params.canSwim) ?? true,
    canFly: validateOptionalBoolean(params.can_fly || params.canFly) ?? false
  });
};

/**
 * Configure character abilities
 */
const handleConfigureCharacterAbilities: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const characterPath = validateRequiredString(params.character_path || params.characterPath, 'character_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_character_abilities', {
    characterPath,
    abilitySystemComponent: validateOptionalBoolean(params.ability_system_component || params.abilitySystemComponent) ?? true,
    startingAbilities: validateOptionalArray(params.starting_abilities || params.startingAbilities) ?? [],
    startingEffects: validateOptionalArray(params.starting_effects || params.startingEffects) ?? [],
    attributeSetClass: validateOptionalString(params.attribute_set_class || params.attributeSetClass)
  });
};

// ============================================================================
// Game Instance Action Handlers
// ============================================================================

/**
 * Create game instance
 */
const handleCreateGameInstance: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const instanceName = validateRequiredString(params.instance_name || params.instanceName, 'instance_name');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'create_game_instance', {
    instanceName,
    parentClass: validateOptionalString(params.parent_class || params.parentClass) ?? 'McpGameInstance',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Blueprints/GameInstance/${instanceName}`
  });
};

/**
 * Configure game instance
 */
const handleConfigureGameInstance: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const instancePath = validateRequiredString(params.instance_path || params.instancePath, 'instance_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_game_instance', {
    instancePath,
    saveGameClass: validateOptionalString(params.save_game_class || params.saveGameClass),
    defaultSettings: validateOptionalObject(params.default_settings || params.defaultSettings) ?? {},
    persistentData: validateOptionalArray(params.persistent_data || params.persistentData) ?? []
  });
};

// ============================================================================
// Save Game Action Handlers
// ============================================================================

/**
 * Create save game class
 */
const handleCreateSaveGameClass: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const className = validateRequiredString(params.class_name || params.className, 'class_name');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'create_save_game_class', {
    className,
    parentClass: validateOptionalString(params.parent_class || params.parentClass) ?? 'McpSaveGame',
    savePath: validateOptionalString(params.save_path || params.savePath) ?? `/Game/Blueprints/SaveGame/${className}`
  });
};

/**
 * Configure save game
 */
const handleConfigureSaveGame: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const classPath = validateRequiredString(params.class_path || params.classPath, 'class_path');

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'configure_save_game', {
    classPath,
    saveSlotName: validateOptionalString(params.save_slot_name || params.saveSlotName) ?? 'SaveSlot',
    userIndex: validateOptionalNumber(params.user_index || params.userIndex) ?? 0,
    savedVariables: validateOptionalArray(params.saved_variables || params.savedVariables) ?? [],
    compressSave: validateOptionalBoolean(params.compress_save || params.compressSave) ?? true
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get framework info
 */
const handleGetFrameworkInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'get_framework_info', {
    includeGameMode: validateOptionalBoolean(params.include_game_mode || params.includeGameMode) ?? true,
    includeGameState: validateOptionalBoolean(params.include_game_state || params.includeGameState) ?? true,
    includePlayerState: validateOptionalBoolean(params.include_player_state || params.includePlayerState) ?? true,
    includeControllers: validateOptionalBoolean(params.include_controllers || params.includeControllers) ?? true
  });
};

/**
 * List framework classes
 */
const handleListFrameworkClasses: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.FRAMEWORK, 'list_framework_classes', {
    classType: validateOptionalString(params.class_type || params.classType),
    searchPath: validateOptionalString(params.search_path || params.searchPath) ?? '/Game/Blueprints',
    mcpClassesOnly: validateOptionalBoolean(params.mcp_classes_only || params.mcpClassesOnly) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const frameworkHandlers: ActionHandlerMap = {
  // Game Mode
  create_game_mode: handleCreateGameMode,
  configure_game_mode: handleConfigureGameMode,
  configure_match_settings: handleConfigureMatchSettings,

  // Game State
  create_game_state: handleCreateGameState,
  configure_game_state: handleConfigureGameState,
  add_game_state_variable: handleAddGameStateVariable,

  // Player State
  create_player_state: handleCreatePlayerState,
  configure_player_state: handleConfigurePlayerState,

  // Player Controller
  create_player_controller: handleCreatePlayerController,
  configure_player_controller: handleConfigurePlayerController,

  // Character
  create_character_class: handleCreateCharacterClass,
  configure_character_movement: handleConfigureCharacterMovement,
  configure_character_abilities: handleConfigureCharacterAbilities,

  // Game Instance
  create_game_instance: handleCreateGameInstance,
  configure_game_instance: handleConfigureGameInstance,

  // Save Game
  create_save_game_class: handleCreateSaveGameClass,
  configure_save_game: handleConfigureSaveGame,

  // Query
  get_framework_info: handleGetFrameworkInfo,
  list_framework_classes: handleListFrameworkClasses
};

export const handleFrameworkTools = createCategoryHandler(Forge2Categories.FRAMEWORK, frameworkHandlers);

export default frameworkHandlers;
