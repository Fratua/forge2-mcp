/**
 * FORGE2: Weather & Time Handlers
 * Handles day/night cycle, weather systems, seasonal changes, and atmospheric effects
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
// Day/Night Cycle Action Handlers
// ============================================================================

/**
 * Create day/night cycle
 */
const handleCreateDayNightCycle: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'create_day_night_cycle', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    dayLengthMinutes: validateOptionalNumber(params.day_length_minutes || params.dayLengthMinutes) ?? 24,
    startTime: validateOptionalNumber(params.start_time || params.startTime) ?? 12.0,
    sunriseHour: validateOptionalNumber(params.sunrise_hour || params.sunriseHour) ?? 6.0,
    sunsetHour: validateOptionalNumber(params.sunset_hour || params.sunsetHour) ?? 18.0,
    timeMultiplier: validateOptionalNumber(params.time_multiplier || params.timeMultiplier) ?? 1.0
  });
};

/**
 * Configure sun movement
 */
const handleConfigureSunMovement: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'configure_sun_movement', {
    latitude: validateOptionalNumber(params.latitude) ?? 45.0,
    sunIntensityCurve: validateOptionalString(params.sun_intensity_curve || params.sunIntensityCurve),
    moonEnabled: validateOptionalBoolean(params.moon_enabled || params.moonEnabled) ?? true,
    moonPhaseEnabled: validateOptionalBoolean(params.moon_phase_enabled || params.moonPhaseEnabled) ?? true,
    starsEnabled: validateOptionalBoolean(params.stars_enabled || params.starsEnabled) ?? true
  });
};

/**
 * Configure sky colors
 */
const handleConfigureSkyColors: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'configure_sky_colors', {
    dayZenithColor: validateOptionalObject(params.day_zenith_color || params.dayZenithColor),
    dayHorizonColor: validateOptionalObject(params.day_horizon_color || params.dayHorizonColor),
    sunsetColor: validateOptionalObject(params.sunset_color || params.sunsetColor),
    nightZenithColor: validateOptionalObject(params.night_zenith_color || params.nightZenithColor),
    nightHorizonColor: validateOptionalObject(params.night_horizon_color || params.nightHorizonColor),
    transitionDuration: validateOptionalNumber(params.transition_duration || params.transitionDuration) ?? 1.0
  });
};

// ============================================================================
// Weather System Action Handlers
// ============================================================================

/**
 * Setup weather system
 */
const handleSetupWeatherSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'setup_weather_system', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    weatherTypes: validateOptionalArray(params.weather_types || params.weatherTypes) ?? ['clear', 'cloudy', 'rain', 'storm', 'snow', 'fog'],
    transitionDuration: validateOptionalNumber(params.transition_duration || params.transitionDuration) ?? 30.0,
    randomWeatherEnabled: validateOptionalBoolean(params.random_weather_enabled || params.randomWeatherEnabled) ?? true,
    weatherChangeInterval: validateOptionalNumber(params.weather_change_interval || params.weatherChangeInterval) ?? 300.0
  });
};

/**
 * Create weather preset
 */
const handleCreateWeatherPreset: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const presetName = validateRequiredString(params.preset_name || params.presetName, 'preset_name');

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'create_weather_preset', {
    presetName,
    cloudDensity: validateOptionalNumber(params.cloud_density || params.cloudDensity) ?? 0.5,
    cloudSpeed: validateOptionalNumber(params.cloud_speed || params.cloudSpeed) ?? 1.0,
    precipitationType: validateOptionalString(params.precipitation_type || params.precipitationType) ?? 'none',
    precipitationIntensity: validateOptionalNumber(params.precipitation_intensity || params.precipitationIntensity) ?? 0.0,
    fogDensity: validateOptionalNumber(params.fog_density || params.fogDensity) ?? 0.0,
    fogDistance: validateOptionalNumber(params.fog_distance || params.fogDistance) ?? 10000.0,
    windSpeed: validateOptionalNumber(params.wind_speed || params.windSpeed) ?? 0.0,
    windDirection: validateOptionalNumber(params.wind_direction || params.windDirection) ?? 0.0,
    lightningEnabled: validateOptionalBoolean(params.lightning_enabled || params.lightningEnabled) ?? false,
    thunderEnabled: validateOptionalBoolean(params.thunder_enabled || params.thunderEnabled) ?? false,
    ambientSoundPath: validateOptionalString(params.ambient_sound_path || params.ambientSoundPath)
  });
};

/**
 * Configure weather effects
 */
const handleConfigureWeatherEffects: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'configure_weather_effects', {
    rainParticleSystem: validateOptionalString(params.rain_particle_system || params.rainParticleSystem),
    snowParticleSystem: validateOptionalString(params.snow_particle_system || params.snowParticleSystem),
    dustParticleSystem: validateOptionalString(params.dust_particle_system || params.dustParticleSystem),
    groundWetness: validateOptionalBoolean(params.ground_wetness || params.groundWetness) ?? true,
    snowAccumulation: validateOptionalBoolean(params.snow_accumulation || params.snowAccumulation) ?? true,
    puddleFormation: validateOptionalBoolean(params.puddle_formation || params.puddleFormation) ?? true
  });
};

// ============================================================================
// Seasonal Action Handlers
// ============================================================================

/**
 * Setup seasonal changes
 */
const handleSetupSeasonalChanges: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'setup_seasonal_changes', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    seasons: validateOptionalArray(params.seasons) ?? ['spring', 'summer', 'autumn', 'winter'],
    seasonLengthDays: validateOptionalNumber(params.season_length_days || params.seasonLengthDays) ?? 30,
    startingSeason: validateOptionalString(params.starting_season || params.startingSeason) ?? 'spring',
    transitionDays: validateOptionalNumber(params.transition_days || params.transitionDays) ?? 5
  });
};

/**
 * Create season preset
 */
const handleCreateSeasonPreset: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const seasonName = validateRequiredString(params.season_name || params.seasonName, 'season_name');

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'create_season_preset', {
    seasonName,
    foliageColor: validateOptionalObject(params.foliage_color || params.foliageColor),
    grassColor: validateOptionalObject(params.grass_color || params.grassColor),
    temperatureRange: validateOptionalObject(params.temperature_range || params.temperatureRange) ?? { min: 15, max: 25 },
    dayLengthModifier: validateOptionalNumber(params.day_length_modifier || params.dayLengthModifier) ?? 1.0,
    weatherProbabilities: validateOptionalObject(params.weather_probabilities || params.weatherProbabilities),
    sunIntensityModifier: validateOptionalNumber(params.sun_intensity_modifier || params.sunIntensityModifier) ?? 1.0
  });
};

// ============================================================================
// Indoor/Outdoor Action Handlers
// ============================================================================

/**
 * Configure indoor outdoor detection
 */
const handleConfigureIndoorOutdoor: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'configure_indoor_outdoor', {
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    useVolumes: validateOptionalBoolean(params.use_volumes || params.useVolumes) ?? true,
    useRaycast: validateOptionalBoolean(params.use_raycast || params.useRaycast) ?? false,
    transitionTime: validateOptionalNumber(params.transition_time || params.transitionTime) ?? 0.5,
    indoorAmbientSound: validateOptionalString(params.indoor_ambient_sound || params.indoorAmbientSound),
    shelterFromWeather: validateOptionalBoolean(params.shelter_from_weather || params.shelterFromWeather) ?? true
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get weather info
 */
const handleGetWeatherInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'get_weather_info', {
    includeCurrentWeather: validateOptionalBoolean(params.include_current_weather || params.includeCurrentWeather) ?? true,
    includeForecast: validateOptionalBoolean(params.include_forecast || params.includeForecast) ?? false,
    includeTimeOfDay: validateOptionalBoolean(params.include_time_of_day || params.includeTimeOfDay) ?? true,
    includeSeason: validateOptionalBoolean(params.include_season || params.includeSeason) ?? true
  });
};

/**
 * List weather presets
 */
const handleListWeatherPresets: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TIME_WEATHER, 'list_weather_presets', {
    includeCustom: validateOptionalBoolean(params.include_custom || params.includeCustom) ?? true,
    includeBuiltIn: validateOptionalBoolean(params.include_built_in || params.includeBuiltIn) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const weatherHandlers: ActionHandlerMap = {
  // Day/Night
  create_day_night_cycle: handleCreateDayNightCycle,
  configure_sun_movement: handleConfigureSunMovement,
  configure_sky_colors: handleConfigureSkyColors,

  // Weather
  setup_weather_system: handleSetupWeatherSystem,
  create_weather_preset: handleCreateWeatherPreset,
  configure_weather_effects: handleConfigureWeatherEffects,

  // Seasonal
  setup_seasonal_changes: handleSetupSeasonalChanges,
  create_season_preset: handleCreateSeasonPreset,

  // Indoor/Outdoor
  configure_indoor_outdoor: handleConfigureIndoorOutdoor,

  // Query
  get_weather_info: handleGetWeatherInfo,
  list_weather_presets: handleListWeatherPresets
};

export const handleWeatherTools = createCategoryHandler(Forge2Categories.TIME_WEATHER, weatherHandlers);

export default weatherHandlers;
