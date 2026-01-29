/**
 * Genre-specific handlers for Sports games (FIFA, NBA 2K, Madden style)
 * Handles player stats, teams, ball physics, AI, seasons, and commentary
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
// Local Validator Helpers
// ============================================================================

function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value.trim();
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

function validateRequiredEnum<T extends string>(value: unknown, validValues: readonly T[], fieldName: string): T {
  const strValue = validateRequiredString(value, fieldName);
  if (!validValues.includes(strValue as T)) {
    throw new Error(`${fieldName} must be one of: ${validValues.join(', ')}`);
  }
  return strValue as T;
}

function validateOptionalEnum<T extends string>(value: unknown, validValues: readonly T[]): T | undefined {
  if (value === undefined || value === null) return undefined;
  const strValue = String(value).trim();
  if (!validValues.includes(strValue as T)) return undefined;
  return strValue as T;
}

function validateOptionalArray(value: unknown): unknown[] | undefined {
  if (value === undefined || value === null) return undefined;
  if (Array.isArray(value)) return value;
  return undefined;
}

function validateOptionalObject(value: unknown): Record<string, unknown> | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value === 'object' && !Array.isArray(value)) return value as Record<string, unknown>;
  return undefined;
}

// ============================================================================
// Type Definitions
// ============================================================================

const SPORT_TYPES = ['soccer', 'basketball', 'american_football', 'baseball', 'hockey', 'tennis', 'golf', 'volleyball'] as const;
const POSITION_CATEGORIES = ['offense', 'defense', 'midfield', 'goalkeeper', 'special_teams'] as const;
const STAT_CATEGORIES = ['physical', 'technical', 'mental', 'goalkeeping'] as const;
const FORMATION_TYPES = ['4-4-2', '4-3-3', '3-5-2', '4-2-3-1', '5-3-2', 'custom'] as const;
const PLAY_TYPES = ['offense', 'defense', 'special'] as const;
const FOUL_TYPES = ['minor', 'major', 'flagrant', 'technical', 'ejection'] as const;
const MATCH_TYPES = ['friendly', 'league', 'cup', 'tournament', 'playoff', 'final'] as const;
const DIFFICULTY_LEVELS = ['rookie', 'amateur', 'semi_pro', 'professional', 'world_class', 'legendary'] as const;
const WEATHER_CONDITIONS = ['clear', 'rain', 'snow', 'wind', 'fog', 'extreme_heat'] as const;
const CAMERA_MODES = ['broadcast', 'end_to_end', 'dynamic', 'player_lock', 'pro', 'custom'] as const;

// Reserve constants for future validation (type validation uses arrays directly)
void STAT_CATEGORIES;
void FOUL_TYPES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Configure sport type
 */
const handleConfigureSportType: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const sportType = validateRequiredEnum(params.sport_type || params.sportType, SPORT_TYPES, 'sport_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'configure_sport_type', {
    sportType,
    teamSize: validateOptionalNumber(params.team_size || params.teamSize),
    fieldDimensions: validateOptionalObject(params.field_dimensions || params.fieldDimensions),
    matchDuration: validateOptionalNumber(params.match_duration || params.matchDuration),
    periods: validateOptionalNumber(params.periods),
    overtimeEnabled: validateOptionalBoolean(params.overtime_enabled || params.overtimeEnabled),
    tieBreaker: validateOptionalString(params.tie_breaker || params.tieBreaker)
  });
};

/**
 * Create player with stats
 */
const handleCreatePlayerStats: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const playerId = validateRequiredString(params.player_id || params.playerId, 'player_id');
  const playerName = validateRequiredString(params.player_name || params.playerName, 'player_name');

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'create_player_stats', {
    playerId,
    playerName,
    position: validateOptionalString(params.position),
    positionCategory: validateOptionalEnum(params.position_category || params.positionCategory, POSITION_CATEGORIES),
    overallRating: validateOptionalNumber(params.overall_rating || params.overallRating) ?? 75,
    potential: validateOptionalNumber(params.potential) ?? 80,
    age: validateOptionalNumber(params.age) ?? 25,
    // Physical stats
    speed: validateOptionalNumber(params.speed) ?? 70,
    acceleration: validateOptionalNumber(params.acceleration) ?? 70,
    strength: validateOptionalNumber(params.strength) ?? 70,
    stamina: validateOptionalNumber(params.stamina) ?? 70,
    jumping: validateOptionalNumber(params.jumping) ?? 70,
    agility: validateOptionalNumber(params.agility) ?? 70,
    // Technical stats
    shooting: validateOptionalNumber(params.shooting) ?? 70,
    passing: validateOptionalNumber(params.passing) ?? 70,
    dribbling: validateOptionalNumber(params.dribbling) ?? 70,
    defending: validateOptionalNumber(params.defending) ?? 70,
    // Mental stats
    awareness: validateOptionalNumber(params.awareness) ?? 70,
    composure: validateOptionalNumber(params.composure) ?? 70,
    aggression: validateOptionalNumber(params.aggression) ?? 70,
    // Custom stats
    customStats: validateOptionalObject(params.custom_stats || params.customStats),
    traits: validateOptionalArray(params.traits),
    weakFoot: validateOptionalNumber(params.weak_foot || params.weakFoot) ?? 3,
    preferredFoot: validateOptionalString(params.preferred_foot || params.preferredFoot) ?? 'right'
  });
};

/**
 * Configure ball physics
 */
const handleConfigureBallPhysics: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'configure_ball_physics', {
    mass: validateOptionalNumber(params.mass) ?? 0.43,
    radius: validateOptionalNumber(params.radius) ?? 11,
    bounceCoefficient: validateOptionalNumber(params.bounce_coefficient || params.bounceCoefficient) ?? 0.7,
    friction: validateOptionalNumber(params.friction) ?? 0.4,
    airResistance: validateOptionalNumber(params.air_resistance || params.airResistance) ?? 0.3,
    spinEffect: validateOptionalNumber(params.spin_effect || params.spinEffect) ?? 1.0,
    magnusEffect: validateOptionalBoolean(params.magnus_effect || params.magnusEffect) ?? true,
    windAffected: validateOptionalBoolean(params.wind_affected || params.windAffected) ?? true,
    trajectoryPrediction: validateOptionalBoolean(params.trajectory_prediction || params.trajectoryPrediction) ?? true
  });
};

/**
 * Setup team AI
 */
const handleSetupTeamAI: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const teamId = validateRequiredString(params.team_id || params.teamId, 'team_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'setup_team_ai', {
    teamId,
    formation: validateOptionalEnum(params.formation, FORMATION_TYPES) ?? '4-4-2',
    playStyle: validateOptionalString(params.play_style || params.playStyle) ?? 'balanced',
    attackingTactic: validateOptionalString(params.attacking_tactic || params.attackingTactic),
    defendingTactic: validateOptionalString(params.defending_tactic || params.defendingTactic),
    pressingIntensity: validateOptionalNumber(params.pressing_intensity || params.pressingIntensity) ?? 50,
    passingStyle: validateOptionalString(params.passing_style || params.passingStyle) ?? 'mixed',
    buildUpSpeed: validateOptionalNumber(params.build_up_speed || params.buildUpSpeed) ?? 50,
    defenseWidth: validateOptionalNumber(params.defense_width || params.defenseWidth) ?? 50,
    defenseDepth: validateOptionalNumber(params.defense_depth || params.defenseDepth) ?? 50,
    playerInstructions: validateOptionalObject(params.player_instructions || params.playerInstructions)
  });
};

/**
 * Create play/tactic
 */
const handleCreatePlay: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const playId = validateRequiredString(params.play_id || params.playId, 'play_id');
  const playName = validateRequiredString(params.play_name || params.playName, 'play_name');
  const playType = validateRequiredEnum(params.play_type || params.playType, PLAY_TYPES, 'play_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'create_play', {
    playId,
    playName,
    playType,
    formation: validateOptionalEnum(params.formation, FORMATION_TYPES),
    positions: validateOptionalArray(params.positions), // Array of { playerId, x, y, route }
    triggers: validateOptionalArray(params.triggers),
    duration: validateOptionalNumber(params.duration),
    situational: validateOptionalString(params.situational) // 'normal', 'red_zone', 'goal_line', etc.
  });
};

/**
 * Create season mode
 */
const handleCreateSeasonMode: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const seasonId = validateRequiredString(params.season_id || params.seasonId, 'season_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'create_season_mode', {
    seasonId,
    leagueName: validateOptionalString(params.league_name || params.leagueName),
    teamCount: validateOptionalNumber(params.team_count || params.teamCount) ?? 20,
    gamesPerTeam: validateOptionalNumber(params.games_per_team || params.gamesPerTeam) ?? 38,
    homeAndAway: validateOptionalBoolean(params.home_and_away || params.homeAndAway) ?? true,
    playoffsEnabled: validateOptionalBoolean(params.playoffs_enabled || params.playoffsEnabled) ?? false,
    playoffTeams: validateOptionalNumber(params.playoff_teams || params.playoffTeams),
    relegation: validateOptionalBoolean(params.relegation) ?? false,
    relegationSpots: validateOptionalNumber(params.relegation_spots || params.relegationSpots),
    cupCompetitions: validateOptionalArray(params.cup_competitions || params.cupCompetitions),
    transferWindows: validateOptionalArray(params.transfer_windows || params.transferWindows)
  });
};

/**
 * Configure referee system
 */
const handleConfigureRefereeSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'configure_referee_system', {
    strictness: validateOptionalNumber(params.strictness) ?? 50,
    varEnabled: validateOptionalBoolean(params.var_enabled || params.varEnabled) ?? true,
    foulDetection: validateOptionalBoolean(params.foul_detection || params.foulDetection) ?? true,
    cardSystem: validateOptionalBoolean(params.card_system || params.cardSystem) ?? true,
    yellowCardThreshold: validateOptionalNumber(params.yellow_card_threshold || params.yellowCardThreshold) ?? 3,
    redCardThreshold: validateOptionalNumber(params.red_card_threshold || params.redCardThreshold) ?? 6,
    offsideEnabled: validateOptionalBoolean(params.offside_enabled || params.offsideEnabled) ?? true,
    penaltyBoxRules: validateOptionalBoolean(params.penalty_box_rules || params.penaltyBoxRules) ?? true,
    advantageRule: validateOptionalBoolean(params.advantage_rule || params.advantageRule) ?? true,
    injuryTime: validateOptionalBoolean(params.injury_time || params.injuryTime) ?? true
  });
};

/**
 * Setup commentary
 */
const handleSetupCommentary: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'setup_commentary', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    commentatorCount: validateOptionalNumber(params.commentator_count || params.commentatorCount) ?? 2,
    excitementLevel: validateOptionalNumber(params.excitement_level || params.excitementLevel) ?? 50,
    triggerEvents: validateOptionalArray(params.trigger_events || params.triggerEvents) ?? [
      'goal', 'near_miss', 'save', 'foul', 'card', 'substitution', 'halftime', 'fulltime'
    ],
    playerNameCallouts: validateOptionalBoolean(params.player_name_callouts || params.playerNameCallouts) ?? true,
    statisticsCommentary: validateOptionalBoolean(params.statistics_commentary || params.statisticsCommentary) ?? true,
    crowdReactions: validateOptionalBoolean(params.crowd_reactions || params.crowdReactions) ?? true,
    delayBetweenLines: validateOptionalNumber(params.delay_between_lines || params.delayBetweenLines) ?? 2
  });
};

/**
 * Create replay system
 */
const handleCreateReplaySystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'create_replay_system', {
    autoReplayEnabled: validateOptionalBoolean(params.auto_replay_enabled || params.autoReplayEnabled) ?? true,
    autoReplayEvents: validateOptionalArray(params.auto_replay_events || params.autoReplayEvents) ?? ['goal', 'save', 'near_miss'],
    replayDuration: validateOptionalNumber(params.replay_duration || params.replayDuration) ?? 10,
    cameraAngles: validateOptionalArray(params.camera_angles || params.cameraAngles) ?? ['broadcast', 'behind_goal', 'side', 'aerial'],
    slowMotionSpeeds: validateOptionalArray(params.slow_motion_speeds || params.slowMotionSpeeds) ?? [0.25, 0.5, 0.75, 1.0],
    pauseEnabled: validateOptionalBoolean(params.pause_enabled || params.pauseEnabled) ?? true,
    skipEnabled: validateOptionalBoolean(params.skip_enabled || params.skipEnabled) ?? true,
    manualReplayEnabled: validateOptionalBoolean(params.manual_replay_enabled || params.manualReplayEnabled) ?? true
  });
};

/**
 * Create team
 */
const handleCreateTeam: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const teamId = validateRequiredString(params.team_id || params.teamId, 'team_id');
  const teamName = validateRequiredString(params.team_name || params.teamName, 'team_name');

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'create_team', {
    teamId,
    teamName,
    shortName: validateOptionalString(params.short_name || params.shortName),
    overallRating: validateOptionalNumber(params.overall_rating || params.overallRating) ?? 75,
    attackRating: validateOptionalNumber(params.attack_rating || params.attackRating) ?? 75,
    defenseRating: validateOptionalNumber(params.defense_rating || params.defenseRating) ?? 75,
    midfieldRating: validateOptionalNumber(params.midfield_rating || params.midfieldRating) ?? 75,
    homeStadium: validateOptionalString(params.home_stadium || params.homeStadium),
    homeKitColors: validateOptionalObject(params.home_kit_colors || params.homeKitColors),
    awayKitColors: validateOptionalObject(params.away_kit_colors || params.awayKitColors),
    budget: validateOptionalNumber(params.budget),
    fanBase: validateOptionalNumber(params.fan_base || params.fanBase),
    leagueId: validateOptionalString(params.league_id || params.leagueId)
  });
};

/**
 * Configure stadium
 */
const handleConfigureStadium: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const stadiumId = validateRequiredString(params.stadium_id || params.stadiumId, 'stadium_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'configure_stadium', {
    stadiumId,
    stadiumName: validateOptionalString(params.stadium_name || params.stadiumName),
    capacity: validateOptionalNumber(params.capacity) ?? 50000,
    fieldType: validateOptionalString(params.field_type || params.fieldType) ?? 'grass',
    roofType: validateOptionalString(params.roof_type || params.roofType) ?? 'open',
    ambientSound: validateOptionalString(params.ambient_sound || params.ambientSound),
    crowdDensity: validateOptionalNumber(params.crowd_density || params.crowdDensity) ?? 0.9,
    homeAdvantage: validateOptionalNumber(params.home_advantage || params.homeAdvantage) ?? 5,
    weatherEnabled: validateOptionalBoolean(params.weather_enabled || params.weatherEnabled) ?? true
  });
};

/**
 * Configure match settings
 */
const handleConfigureMatchSettings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'configure_match_settings', {
    matchType: validateOptionalEnum(params.match_type || params.matchType, MATCH_TYPES) ?? 'friendly',
    difficulty: validateOptionalEnum(params.difficulty, DIFFICULTY_LEVELS) ?? 'professional',
    halfLength: validateOptionalNumber(params.half_length || params.halfLength) ?? 6,
    gameSpeed: validateOptionalNumber(params.game_speed || params.gameSpeed) ?? 0,
    injuries: validateOptionalBoolean(params.injuries) ?? true,
    fatigue: validateOptionalBoolean(params.fatigue) ?? true,
    substitutionLimit: validateOptionalNumber(params.substitution_limit || params.substitutionLimit) ?? 3,
    cameraMode: validateOptionalEnum(params.camera_mode || params.cameraMode, CAMERA_MODES) ?? 'broadcast',
    weather: validateOptionalEnum(params.weather, WEATHER_CONDITIONS),
    timeOfDay: validateOptionalString(params.time_of_day || params.timeOfDay) ?? 'afternoon'
  });
};

/**
 * Get match state
 */
const handleGetMatchState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'get_match_state', {
    includeScore: validateOptionalBoolean(params.include_score || params.includeScore) ?? true,
    includeTime: validateOptionalBoolean(params.include_time || params.includeTime) ?? true,
    includeStats: validateOptionalBoolean(params.include_stats || params.includeStats) ?? true,
    includeFormations: validateOptionalBoolean(params.include_formations || params.includeFormations) ?? false,
    includePlayerStats: validateOptionalBoolean(params.include_player_stats || params.includePlayerStats) ?? false
  });
};

/**
 * Get standings
 */
const handleGetStandings: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const seasonId = validateRequiredString(params.season_id || params.seasonId, 'season_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'get_standings', {
    seasonId,
    includeGoalDifference: validateOptionalBoolean(params.include_goal_difference || params.includeGoalDifference) ?? true,
    includeForm: validateOptionalBoolean(params.include_form || params.includeForm) ?? true,
    includeTopScorers: validateOptionalBoolean(params.include_top_scorers || params.includeTopScorers) ?? false
  });
};

/**
 * List teams
 */
const handleListTeams: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'list_teams', {
    leagueId: validateOptionalString(params.league_id || params.leagueId),
    includeRatings: validateOptionalBoolean(params.include_ratings || params.includeRatings) ?? true,
    includeRosters: validateOptionalBoolean(params.include_rosters || params.includeRosters) ?? false
  });
};

/**
 * List players
 */
const handleListPlayers: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_SPORTS, 'list_players', {
    teamId: validateOptionalString(params.team_id || params.teamId),
    position: validateOptionalString(params.position),
    minRating: validateOptionalNumber(params.min_rating || params.minRating),
    maxRating: validateOptionalNumber(params.max_rating || params.maxRating),
    includeStats: validateOptionalBoolean(params.include_stats || params.includeStats) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const genreSportsHandlers: ActionHandlerMap = {
  // Sport configuration
  configure_sport_type: handleConfigureSportType,
  configure_ball_physics: handleConfigureBallPhysics,

  // Players and teams
  create_player_stats: handleCreatePlayerStats,
  create_team: handleCreateTeam,
  setup_team_ai: handleSetupTeamAI,
  create_play: handleCreatePlay,

  // Season/competition
  create_season_mode: handleCreateSeasonMode,
  configure_stadium: handleConfigureStadium,

  // Match systems
  configure_referee_system: handleConfigureRefereeSystem,
  setup_commentary: handleSetupCommentary,
  create_replay_system: handleCreateReplaySystem,
  configure_match_settings: handleConfigureMatchSettings,

  // Query
  get_match_state: handleGetMatchState,
  get_standings: handleGetStandings,
  list_teams: handleListTeams,
  list_players: handleListPlayers
};

// Create the main handler function for this category
export const handleGenreSportsTools = createCategoryHandler(Forge2Categories.GENRE_SPORTS, genreSportsHandlers);

export default genreSportsHandlers;
