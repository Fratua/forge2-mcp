/**
 * Genre-specific handlers for Rhythm games (Guitar Hero, Beat Saber, Osu style)
 * Handles note tracks, scoring, combos, beat mapping, and audio synchronization
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

function _validateOptionalObject(value: unknown): Record<string, unknown> | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value === 'object' && !Array.isArray(value)) return value as Record<string, unknown>;
  return undefined;
}
void _validateOptionalObject; // Reserved for future use

// ============================================================================
// Type Definitions
// ============================================================================

const NOTE_TYPES = ['tap', 'hold', 'slide', 'flick', 'swipe', 'scratch', 'bomb'] as const;
const TIMING_GRADES = ['perfect', 'great', 'good', 'ok', 'miss'] as const;
const DIFFICULTY_LEVELS = ['easy', 'normal', 'hard', 'expert', 'master'] as const;
const LANE_COUNTS = [4, 5, 6, 7, 8, 9] as const;
const INPUT_MODES = ['button', 'touch', 'motion', 'vr', 'keyboard', 'drum', 'guitar'] as const;
const GAME_MODES = ['standard', 'marathon', 'endless', 'challenge', 'practice', 'versus'] as const;
const MODIFIER_TYPES = ['speed', 'hidden', 'sudden', 'mirror', 'random', 'no_fail', 'auto'] as const;
const VISUAL_STYLES = ['falling', 'rising', 'horizontal', 'radial', 'vr_blocks'] as const;

// Reserve constants for future validation (type validation uses arrays directly)
void LANE_COUNTS;
void GAME_MODES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a note track/chart
 */
const handleCreateNoteTrack: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const trackId = validateRequiredString(params.track_id || params.trackId, 'track_id');
  const songId = validateRequiredString(params.song_id || params.songId, 'song_id');
  const difficulty = validateRequiredEnum(params.difficulty, DIFFICULTY_LEVELS, 'difficulty');

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'create_note_track', {
    trackId,
    songId,
    difficulty,
    laneCount: validateOptionalNumber(params.lane_count || params.laneCount) ?? 4,
    bpm: validateOptionalNumber(params.bpm) ?? 120,
    offset: validateOptionalNumber(params.offset) ?? 0,
    visualStyle: validateOptionalEnum(params.visual_style || params.visualStyle, VISUAL_STYLES) ?? 'falling',
    inputMode: validateOptionalEnum(params.input_mode || params.inputMode, INPUT_MODES) ?? 'button',
    description: validateOptionalString(params.description),
    author: validateOptionalString(params.author)
  });
};

/**
 * Add notes to a track
 */
const handleAddNotes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const trackId = validateRequiredString(params.track_id || params.trackId, 'track_id');
  const notes = validateOptionalArray(params.notes) ?? [];

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'add_notes', {
    trackId,
    notes, // Array of { time, lane, type, duration, direction }
    clearExisting: validateOptionalBoolean(params.clear_existing || params.clearExisting) ?? false
  });
};

/**
 * Add a single note
 */
const handleAddNote: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const trackId = validateRequiredString(params.track_id || params.trackId, 'track_id');
  const time = validateOptionalNumber(params.time) ?? 0;
  const lane = validateOptionalNumber(params.lane) ?? 0;
  const noteType = validateRequiredEnum(params.note_type || params.noteType, NOTE_TYPES, 'note_type');

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'add_note', {
    trackId,
    time, // In milliseconds or beats
    lane,
    noteType,
    duration: validateOptionalNumber(params.duration), // For hold notes
    direction: validateOptionalString(params.direction), // For swipe/flick notes
    timeUnit: validateOptionalString(params.time_unit || params.timeUnit) ?? 'ms' // 'ms' or 'beat'
  });
};

/**
 * Configure scoring system
 */
const handleConfigureScoring: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'configure_scoring', {
    perfectScore: validateOptionalNumber(params.perfect_score || params.perfectScore) ?? 100,
    greatScore: validateOptionalNumber(params.great_score || params.greatScore) ?? 75,
    goodScore: validateOptionalNumber(params.good_score || params.goodScore) ?? 50,
    okScore: validateOptionalNumber(params.ok_score || params.okScore) ?? 25,
    missScore: validateOptionalNumber(params.miss_score || params.missScore) ?? 0,
    holdScorePerBeat: validateOptionalNumber(params.hold_score_per_beat || params.holdScorePerBeat) ?? 10,
    maxComboBonus: validateOptionalNumber(params.max_combo_bonus || params.maxComboBonus) ?? 2.0,
    comboGrowthRate: validateOptionalNumber(params.combo_growth_rate || params.comboGrowthRate) ?? 0.01
  });
};

/**
 * Configure timing windows
 */
const handleConfigureTimingWindows: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'configure_timing_windows', {
    perfectWindow: validateOptionalNumber(params.perfect_window || params.perfectWindow) ?? 25, // milliseconds
    greatWindow: validateOptionalNumber(params.great_window || params.greatWindow) ?? 50,
    goodWindow: validateOptionalNumber(params.good_window || params.goodWindow) ?? 100,
    okWindow: validateOptionalNumber(params.ok_window || params.okWindow) ?? 150,
    scaleWithDifficulty: validateOptionalBoolean(params.scale_with_difficulty || params.scaleWithDifficulty) ?? true,
    easyMultiplier: validateOptionalNumber(params.easy_multiplier || params.easyMultiplier) ?? 1.5,
    hardMultiplier: validateOptionalNumber(params.hard_multiplier || params.hardMultiplier) ?? 0.8,
    expertMultiplier: validateOptionalNumber(params.expert_multiplier || params.expertMultiplier) ?? 0.6,
    masterMultiplier: validateOptionalNumber(params.master_multiplier || params.masterMultiplier) ?? 0.5
  });
};

/**
 * Setup combo system
 */
const handleSetupComboSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'setup_combo_system', {
    comboEnabled: validateOptionalBoolean(params.combo_enabled || params.comboEnabled) ?? true,
    minimumGradeForCombo: validateOptionalEnum(params.minimum_grade_for_combo || params.minimumGradeForCombo, TIMING_GRADES) ?? 'ok',
    comboMilestones: validateOptionalArray(params.combo_milestones || params.comboMilestones) ?? [10, 25, 50, 100, 200, 500],
    milestoneEffects: validateOptionalBoolean(params.milestone_effects || params.milestoneEffects) ?? true,
    streakDisplay: validateOptionalBoolean(params.streak_display || params.streakDisplay) ?? true,
    comboBreakPenalty: validateOptionalNumber(params.combo_break_penalty || params.comboBreakPenalty) ?? 0
  });
};

/**
 * Create beat map editor settings
 */
const handleCreateBeatMapEditor: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'create_beat_map_editor', {
    gridSnapEnabled: validateOptionalBoolean(params.grid_snap_enabled || params.gridSnapEnabled) ?? true,
    gridDivisions: validateOptionalArray(params.grid_divisions || params.gridDivisions) ?? [4, 8, 12, 16, 24, 32],
    defaultDivision: validateOptionalNumber(params.default_division || params.defaultDivision) ?? 4,
    waveformDisplay: validateOptionalBoolean(params.waveform_display || params.waveformDisplay) ?? true,
    metronomeEnabled: validateOptionalBoolean(params.metronome_enabled || params.metronomeEnabled) ?? true,
    playbackSpeeds: validateOptionalArray(params.playback_speeds || params.playbackSpeeds) ?? [0.25, 0.5, 0.75, 1.0],
    autoSaveInterval: validateOptionalNumber(params.auto_save_interval || params.autoSaveInterval) ?? 60,
    undoHistorySize: validateOptionalNumber(params.undo_history_size || params.undoHistorySize) ?? 100
  });
};

/**
 * Configure difficulty scaling
 */
const handleConfigureDifficulty: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const difficulty = validateRequiredEnum(params.difficulty, DIFFICULTY_LEVELS, 'difficulty');

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'configure_difficulty', {
    difficulty,
    noteSpeed: validateOptionalNumber(params.note_speed || params.noteSpeed),
    noteDensity: validateOptionalNumber(params.note_density || params.noteDensity),
    holdNoteFrequency: validateOptionalNumber(params.hold_note_frequency || params.holdNoteFrequency),
    complexPatterns: validateOptionalBoolean(params.complex_patterns || params.complexPatterns),
    starRating: validateOptionalNumber(params.star_rating || params.starRating),
    healthDrain: validateOptionalNumber(params.health_drain || params.healthDrain),
    healthRecovery: validateOptionalNumber(params.health_recovery || params.healthRecovery)
  });
};

/**
 * Setup visual feedback
 */
const handleSetupVisualFeedback: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'setup_visual_feedback', {
    hitEffectsEnabled: validateOptionalBoolean(params.hit_effects_enabled || params.hitEffectsEnabled) ?? true,
    perfectEffect: validateOptionalString(params.perfect_effect || params.perfectEffect),
    greatEffect: validateOptionalString(params.great_effect || params.greatEffect),
    missEffect: validateOptionalString(params.miss_effect || params.missEffect),
    laneHighlighting: validateOptionalBoolean(params.lane_highlighting || params.laneHighlighting) ?? true,
    judgmentTextDisplay: validateOptionalBoolean(params.judgment_text_display || params.judgmentTextDisplay) ?? true,
    comboDisplay: validateOptionalBoolean(params.combo_display || params.comboDisplay) ?? true,
    accuracyDisplay: validateOptionalBoolean(params.accuracy_display || params.accuracyDisplay) ?? true,
    progressBar: validateOptionalBoolean(params.progress_bar || params.progressBar) ?? true,
    backgroundVideoEnabled: validateOptionalBoolean(params.background_video_enabled || params.backgroundVideoEnabled) ?? false
  });
};

/**
 * Configure game modifiers
 */
const handleConfigureModifiers: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'configure_modifiers', {
    availableModifiers: validateOptionalArray(params.available_modifiers || params.availableModifiers) ?? MODIFIER_TYPES,
    speedOptions: validateOptionalArray(params.speed_options || params.speedOptions) ?? [0.5, 0.75, 1.0, 1.25, 1.5, 2.0],
    hiddenFadePoint: validateOptionalNumber(params.hidden_fade_point || params.hiddenFadePoint) ?? 0.5, // % from hit point
    suddenFadePoint: validateOptionalNumber(params.sudden_fade_point || params.suddenFadePoint) ?? 0.5,
    mirrorSwapLanes: validateOptionalBoolean(params.mirror_swap_lanes || params.mirrorSwapLanes) ?? true,
    randomizeLanes: validateOptionalBoolean(params.randomize_lanes || params.randomizeLanes) ?? true,
    noFailDrainRate: validateOptionalNumber(params.no_fail_drain_rate || params.noFailDrainRate) ?? 0,
    autoPlayEnabled: validateOptionalBoolean(params.auto_play_enabled || params.autoPlayEnabled) ?? true
  });
};

/**
 * Configure audio synchronization
 */
const handleConfigureAudioSync: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'configure_audio_sync', {
    globalOffset: validateOptionalNumber(params.global_offset || params.globalOffset) ?? 0,
    audioLatencyCalibration: validateOptionalBoolean(params.audio_latency_calibration || params.audioLatencyCalibration) ?? true,
    visualOffset: validateOptionalNumber(params.visual_offset || params.visualOffset) ?? 0,
    hitSoundEnabled: validateOptionalBoolean(params.hit_sound_enabled || params.hitSoundEnabled) ?? true,
    hitSoundVolume: validateOptionalNumber(params.hit_sound_volume || params.hitSoundVolume) ?? 0.5,
    musicVolume: validateOptionalNumber(params.music_volume || params.musicVolume) ?? 1.0,
    effectsVolume: validateOptionalNumber(params.effects_volume || params.effectsVolume) ?? 0.7
  });
};

/**
 * Create song entry
 */
const handleCreateSong: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const songId = validateRequiredString(params.song_id || params.songId, 'song_id');
  const songTitle = validateRequiredString(params.song_title || params.songTitle, 'song_title');
  const audioPath = validateRequiredString(params.audio_path || params.audioPath, 'audio_path');

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'create_song', {
    songId,
    songTitle,
    audioPath,
    artist: validateOptionalString(params.artist),
    album: validateOptionalString(params.album),
    bpm: validateOptionalNumber(params.bpm) ?? 120,
    duration: validateOptionalNumber(params.duration), // seconds
    previewStart: validateOptionalNumber(params.preview_start || params.previewStart) ?? 30,
    previewDuration: validateOptionalNumber(params.preview_duration || params.previewDuration) ?? 15,
    genre: validateOptionalString(params.genre),
    coverArt: validateOptionalString(params.cover_art || params.coverArt),
    backgroundImage: validateOptionalString(params.background_image || params.backgroundImage),
    unlockCondition: validateOptionalString(params.unlock_condition || params.unlockCondition)
  });
};

/**
 * Configure health/fail system
 */
const handleConfigureHealthSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'configure_health_system', {
    healthEnabled: validateOptionalBoolean(params.health_enabled || params.healthEnabled) ?? true,
    maxHealth: validateOptionalNumber(params.max_health || params.maxHealth) ?? 100,
    startingHealth: validateOptionalNumber(params.starting_health || params.startingHealth) ?? 100,
    perfectHealthGain: validateOptionalNumber(params.perfect_health_gain || params.perfectHealthGain) ?? 3,
    greatHealthGain: validateOptionalNumber(params.great_health_gain || params.greatHealthGain) ?? 2,
    goodHealthGain: validateOptionalNumber(params.good_health_gain || params.goodHealthGain) ?? 1,
    missHealthLoss: validateOptionalNumber(params.miss_health_loss || params.missHealthLoss) ?? 10,
    failOnEmpty: validateOptionalBoolean(params.fail_on_empty || params.failOnEmpty) ?? true,
    instantDeathMisses: validateOptionalNumber(params.instant_death_misses || params.instantDeathMisses) // For hardcore mode
  });
};

/**
 * Get song performance
 */
const handleGetPerformance: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const songId = validateRequiredString(params.song_id || params.songId, 'song_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'get_performance', {
    songId,
    difficulty: validateOptionalEnum(params.difficulty, DIFFICULTY_LEVELS),
    includeHitData: validateOptionalBoolean(params.include_hit_data || params.includeHitData) ?? false,
    includeHighScores: validateOptionalBoolean(params.include_high_scores || params.includeHighScores) ?? true
  });
};

/**
 * Submit score
 */
const handleSubmitScore: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const songId = validateRequiredString(params.song_id || params.songId, 'song_id');
  const difficulty = validateRequiredEnum(params.difficulty, DIFFICULTY_LEVELS, 'difficulty');
  const score = validateOptionalNumber(params.score) ?? 0;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'submit_score', {
    songId,
    difficulty,
    score,
    maxCombo: validateOptionalNumber(params.max_combo || params.maxCombo),
    accuracy: validateOptionalNumber(params.accuracy),
    perfectCount: validateOptionalNumber(params.perfect_count || params.perfectCount),
    greatCount: validateOptionalNumber(params.great_count || params.greatCount),
    goodCount: validateOptionalNumber(params.good_count || params.goodCount),
    okCount: validateOptionalNumber(params.ok_count || params.okCount),
    missCount: validateOptionalNumber(params.miss_count || params.missCount),
    modifiers: validateOptionalArray(params.modifiers),
    fullCombo: validateOptionalBoolean(params.full_combo || params.fullCombo),
    allPerfect: validateOptionalBoolean(params.all_perfect || params.allPerfect)
  });
};

/**
 * List all songs
 */
const handleListSongs: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'list_songs', {
    filterGenre: validateOptionalString(params.filter_genre || params.filterGenre),
    filterDifficulty: validateOptionalEnum(params.filter_difficulty || params.filterDifficulty, DIFFICULTY_LEVELS),
    sortBy: validateOptionalString(params.sort_by || params.sortBy) ?? 'title',
    includeHighScores: validateOptionalBoolean(params.include_high_scores || params.includeHighScores) ?? false
  });
};

/**
 * List tracks for a song
 */
const handleListTracks: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const songId = validateRequiredString(params.song_id || params.songId, 'song_id');

  return executeForge2Action(tools, Forge2Categories.GENRE_RHYTHM, 'list_tracks', {
    songId,
    includeNoteCount: validateOptionalBoolean(params.include_note_count || params.includeNoteCount) ?? true,
    includeStats: validateOptionalBoolean(params.include_stats || params.includeStats) ?? false
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const genreRhythmHandlers: ActionHandlerMap = {
  // Track creation
  create_note_track: handleCreateNoteTrack,
  add_notes: handleAddNotes,
  add_note: handleAddNote,

  // Scoring
  configure_scoring: handleConfigureScoring,
  configure_timing_windows: handleConfigureTimingWindows,
  setup_combo_system: handleSetupComboSystem,

  // Editor
  create_beat_map_editor: handleCreateBeatMapEditor,
  configure_difficulty: handleConfigureDifficulty,

  // Visual/Audio
  setup_visual_feedback: handleSetupVisualFeedback,
  configure_modifiers: handleConfigureModifiers,
  configure_audio_sync: handleConfigureAudioSync,

  // Songs
  create_song: handleCreateSong,
  configure_health_system: handleConfigureHealthSystem,

  // Query
  get_performance: handleGetPerformance,
  submit_score: handleSubmitScore,
  list_songs: handleListSongs,
  list_tracks: handleListTracks
};

// Create the main handler function for this category
export const handleGenreRhythmTools = createCategoryHandler(Forge2Categories.GENRE_RHYTHM, genreRhythmHandlers);

export default genreRhythmHandlers;
