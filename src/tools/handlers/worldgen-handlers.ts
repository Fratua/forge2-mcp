/**
 * FORGE2: World Generation Handlers
 * Procedural world generation using UE native systems (PCG, Geometry Script, Landscape)
 *
 * Handles terrain generation, biomes, structures, dungeons, cities, water bodies,
 * and environment population.
 */

import { ITools } from '../../types/tool-interfaces.js';
import type { HandlerArgs } from '../../types/handler-types.js';
import { cleanObject } from '../../utils/safe-json.js';
import {
  Forge2Categories,
  executeForge2Action,
  createCategoryHandler,
  normalizeAssetPath,
  type ActionHandler,
  type ActionHandlerMap
} from './utils/forge2-bridge.js';
// Note: Using local validator helpers for worldgen-specific validation

// ============================================================================
// Helper Functions - Required Validators
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
 * Validate optional enum parameter - returns undefined if value is not provided
 */
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

/**
 * Validate optional number parameter
 * @internal Reserved for future use
 */
function _validateOptionalNumber(value: unknown, fieldName: string): number | undefined {
  if (value === undefined || value === null) {
    return undefined;
  }
  const num = typeof value === 'number' ? value : Number(value);
  if (isNaN(num)) {
    throw new Error(`${fieldName} must be a valid number`);
  }
  return num;
}
// Suppress unused warning - reserved for future handlers
void _validateOptionalNumber;

/**
 * Validate optional boolean parameter
 * @internal Reserved for future use
 */
function _validateOptionalBoolean(value: unknown): boolean | undefined {
  if (value === undefined || value === null) {
    return undefined;
  }
  return Boolean(value);
}
// Suppress unused warning - reserved for future handlers
void _validateOptionalBoolean;

/**
 * Validate optional array parameter
 * @internal Reserved for future use
 */
function _validateOptionalArray<T>(value: unknown): T[] | undefined {
  if (value === undefined || value === null) {
    return undefined;
  }
  if (!Array.isArray(value)) {
    return [value] as T[];
  }
  return value as T[];
}
// Suppress unused warning - reserved for future handlers
void _validateOptionalArray;

// ============================================================================
// Types
// ============================================================================

export interface NoiseSettings {
  noiseType: 'perlin' | 'simplex' | 'voronoi' | 'fbm' | 'ridged' | 'worley';
  seed?: number;
  scale?: number;
  octaves?: number;
  persistence?: number;
  lacunarity?: number;
  amplitude?: number;
  frequency?: number;
}

export interface ErosionSettings {
  iterations?: number;
  strength?: number;
  sedimentCapacity?: number;
  evaporationRate?: number;
}

export interface BiomeDefinition {
  biomeName: string;
  temperatureRange?: [number, number];
  moistureRange?: [number, number];
  elevationRange?: [number, number];
  vegetationDensity?: number;
  materialPath?: string;
  foliageTypes?: string[];
  ambientSounds?: string[];
}

export interface BuildingSettings {
  style: 'medieval' | 'fantasy' | 'modern' | 'scifi' | 'ancient' | 'victorian' | 'japanese' | 'arabic';
  size?: 'small' | 'medium' | 'large' | 'massive';
  floorCount?: number;
  includeInterior?: boolean;
  interiorStyle?: string;
  furnitureDensity?: number;
}

export interface DungeonSettings {
  algorithm: 'bsp' | 'wfc' | 'cellular_automata' | 'drunkards_walk';
  roomCount?: number;
  roomSizeRange?: [number, number];
  corridorWidth?: number;
  includeLoops?: boolean;
  roomTypes?: string[];
}

export interface CitySettings {
  districtTypes?: string[];
  buildingDensity?: number;
  roadStyle?: string;
  includeWalls?: boolean;
  populationTarget?: number;
}

// ============================================================================
// Helper Functions
// ============================================================================

function normalizeVector3(v: unknown): [number, number, number] | undefined {
  if (!v) return undefined;
  if (Array.isArray(v) && v.length >= 3) {
    return [Number(v[0]) || 0, Number(v[1]) || 0, Number(v[2]) || 0];
  }
  if (typeof v === 'object' && v !== null) {
    const vec = v as Record<string, unknown>;
    return [Number(vec.x) || 0, Number(vec.y) || 0, Number(vec.z) || 0];
  }
  return undefined;
}

function extractNoiseSettings(args: Record<string, unknown>): NoiseSettings {
  return {
    noiseType: (args.noise_type || args.noiseType || 'perlin') as NoiseSettings['noiseType'],
    seed: args.seed as number | undefined,
    scale: args.scale as number | undefined,
    octaves: args.octaves as number | undefined,
    persistence: args.persistence as number | undefined,
    lacunarity: args.lacunarity as number | undefined,
    amplitude: args.amplitude as number | undefined,
    frequency: args.frequency as number | undefined
  };
}

function extractErosionSettings(args: Record<string, unknown>): ErosionSettings {
  return {
    iterations: (args.erosion_iterations || args.iterations) as number | undefined,
    strength: (args.erosion_strength || args.strength) as number | undefined,
    sedimentCapacity: (args.sediment_capacity || args.sedimentCapacity) as number | undefined,
    evaporationRate: (args.evaporation_rate || args.evaporationRate) as number | undefined
  };
}

// ============================================================================
// Terrain Generation Handlers
// ============================================================================

const handleGenerateHeightmap: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;
  const noise = extractNoiseSettings(params);

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_heightmap', {
    ...noise,
    width: params.width || 1024,
    height: params.height || 1024,
    outputPath: params.output_path || params.outputPath
  });
};

const handleGenerateNoiseTerrain: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;
  const noise = extractNoiseSettings(params);

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_noise_terrain', {
    name: params.name || 'GeneratedTerrain',
    location: normalizeVector3(params.location),
    ...noise,
    size: params.size || 10000,
    resolution: params.resolution || 256
  });
};

const handleApplyThermalErosion: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;
  const erosion = extractErosionSettings(params);

  return executeForge2Action(tools, Forge2Categories.WORLD, 'apply_thermal_erosion', {
    landscapeName: params.landscape_name || params.landscapeName,
    ...erosion,
    talusAngle: params.talus_angle || params.talusAngle || 45
  });
};

const handleApplyHydraulicErosion: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;
  const erosion = extractErosionSettings(params);

  return executeForge2Action(tools, Forge2Categories.WORLD, 'apply_hydraulic_erosion', {
    landscapeName: params.landscape_name || params.landscapeName,
    ...erosion,
    rainAmount: params.rain_amount || params.rainAmount || 1.0,
    solubility: params.solubility || 0.01
  });
};

const handleImportHeightmap: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const filePath = validateRequiredString(params.file_path || params.filePath, 'file_path');

  return executeForge2Action(tools, Forge2Categories.WORLD, 'import_heightmap', {
    filePath,
    landscapeName: params.landscape_name || params.landscapeName,
    scale: params.scale || 1.0
  });
};

const handleExportHeightmap: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const landscapeName = validateRequiredString(
    params.landscape_name || params.landscapeName,
    'landscape_name'
  );

  return executeForge2Action(tools, Forge2Categories.WORLD, 'export_heightmap', {
    landscapeName,
    outputPath: params.output_path || params.outputPath,
    format: params.format || 'png'
  });
};

// ============================================================================
// Biome System Handlers
// ============================================================================

const handleDefineBiome: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const biomeName = validateRequiredString(params.biome_name || params.biomeName, 'biome_name');

  return executeForge2Action(tools, Forge2Categories.WORLD, 'define_biome', {
    biomeName,
    temperatureRange: (params.temperature_range || params.temperatureRange) as [number, number] | undefined,
    moistureRange: (params.moisture_range || params.moistureRange) as [number, number] | undefined,
    elevationRange: (params.elevation_range || params.elevationRange) as [number, number] | undefined,
    vegetationDensity: (params.vegetation_density || params.vegetationDensity) as number | undefined,
    materialPath: normalizeAssetPath(String(params.material_path || params.materialPath || '')),
    foliageTypes: (params.foliage_types || params.foliageTypes) as string[] | undefined,
    ambientSounds: (params.ambient_sounds || params.ambientSounds) as string[] | undefined
  });
};

const handleCreateBiomeMap: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'create_biome_map', {
    landscapeName: params.landscape_name || params.landscapeName,
    seed: params.seed,
    temperatureNoiseScale: params.temperature_noise_scale || params.temperatureNoiseScale || 0.001,
    moistureNoiseScale: params.moisture_noise_scale || params.moistureNoiseScale || 0.002
  });
};

const handleApplyBiome: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const biomeName = validateRequiredString(params.biome_name || params.biomeName, 'biome_name');

  return executeForge2Action(tools, Forge2Categories.WORLD, 'apply_biome', {
    biomeName,
    region: params.region,
    blendDistance: params.blend_distance || params.blendDistance || 100
  });
};

const handleBlendBiomeEdges: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'blend_biome_edges', {
    landscapeName: params.landscape_name || params.landscapeName,
    blendWidth: params.blend_width || params.blendWidth || 50
  });
};

// ============================================================================
// Water Body Handlers
// ============================================================================

const handleCreateRiver: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'create_river', {
    name: params.name || 'River',
    splinePoints: params.spline_points || params.splinePoints,
    width: params.width || 100,
    depth: params.depth || 50,
    flowRate: params.flow_rate || params.flowRate || 1.0,
    carveTerrain: params.carve_terrain !== false,
    materialPath: normalizeAssetPath(String(params.material_path || params.materialPath || ''))
  });
};

const handleCreateLake: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'create_lake', {
    name: params.name || 'Lake',
    location: normalizeVector3(params.location),
    radius: params.radius || 500,
    depth: params.depth || 100,
    shorelineIrregularity: params.shoreline_irregularity || params.shorelineIrregularity || 0.3,
    materialPath: normalizeAssetPath(String(params.material_path || params.materialPath || ''))
  });
};

const handleCreateOcean: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'create_ocean', {
    name: params.name || 'Ocean',
    seaLevel: params.sea_level || params.seaLevel || 0,
    waveIntensity: params.wave_intensity || params.waveIntensity || 1.0,
    materialPath: normalizeAssetPath(String(params.material_path || params.materialPath || ''))
  });
};

const handleSimulateWaterFlow: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'simulate_water_flow', {
    landscapeName: params.landscape_name || params.landscapeName,
    rainAmount: params.rain_amount || params.rainAmount || 1.0,
    iterations: params.iterations || 100
  });
};

// ============================================================================
// Structure Generation Handlers
// ============================================================================

const handleGenerateBuilding: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const style = validateOptionalEnum(
    params.building_style || params.style,
    ['medieval', 'fantasy', 'modern', 'scifi', 'ancient', 'victorian', 'japanese', 'arabic'],
    'building_style'
  ) || 'medieval';

  const settings: BuildingSettings = {
    style: style as BuildingSettings['style'],
    size: (params.building_size || params.size) as BuildingSettings['size'],
    floorCount: (params.floor_count || params.floorCount) as number | undefined,
    includeInterior: (params.include_interior ?? params.includeInterior ?? true) as boolean,
    interiorStyle: (params.interior_style || params.interiorStyle) as string | undefined,
    furnitureDensity: (params.furniture_density || params.furnitureDensity) as number | undefined
  };

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_building', {
    name: params.name || 'GeneratedBuilding',
    location: normalizeVector3(params.location),
    rotation: normalizeVector3(params.rotation),
    ...settings
  });
};

const handleGenerateDungeon: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const algorithm = validateOptionalEnum(
    params.dungeon_algorithm || params.algorithm,
    ['bsp', 'wfc', 'cellular_automata', 'drunkards_walk'],
    'dungeon_algorithm'
  ) || 'bsp';

  const settings: DungeonSettings = {
    algorithm: algorithm as DungeonSettings['algorithm'],
    roomCount: (params.room_count || params.roomCount) as number | undefined,
    roomSizeRange: (params.room_size_range || params.roomSizeRange) as [number, number] | undefined,
    corridorWidth: (params.corridor_width || params.corridorWidth) as number | undefined,
    includeLoops: (params.include_loops ?? params.includeLoops) as boolean | undefined,
    roomTypes: (params.room_types || params.roomTypes) as string[] | undefined
  };

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_dungeon', {
    name: params.name || 'GeneratedDungeon',
    location: normalizeVector3(params.location),
    seed: params.seed,
    ...settings
  });
};

const handleGenerateCity: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const settings: CitySettings = {
    districtTypes: (params.district_types || params.districtTypes) as string[] | undefined,
    buildingDensity: (params.building_density || params.buildingDensity) as number | undefined,
    roadStyle: (params.road_style || params.roadStyle) as string | undefined,
    includeWalls: (params.include_walls ?? params.includeWalls) as boolean | undefined,
    populationTarget: (params.population_target || params.populationTarget) as number | undefined
  };

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_city', {
    name: params.name || 'GeneratedCity',
    location: normalizeVector3(params.location),
    size: params.size || 5000,
    seed: params.seed,
    ...settings
  });
};

const handleGenerateVillage: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_village', {
    name: params.name || 'GeneratedVillage',
    location: normalizeVector3(params.location),
    size: params.size || 1000,
    buildingCount: params.building_count || params.buildingCount || 10,
    style: params.style || 'medieval',
    includeFarms: params.include_farms ?? params.includeFarms ?? true,
    seed: params.seed
  });
};

const handleGenerateRoadNetwork: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_road_network', {
    name: params.name || 'RoadNetwork',
    waypoints: params.waypoints,
    width: params.width || 500,
    material: normalizeAssetPath(String(params.material || params.materialPath || '')),
    createSpline: params.create_spline ?? params.createSpline ?? true
  });
};

const handleGenerateBridge: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_bridge', {
    name: params.name || 'Bridge',
    startPoint: normalizeVector3(params.start_point || params.startPoint),
    endPoint: normalizeVector3(params.end_point || params.endPoint),
    style: params.style || 'stone',
    width: params.width || 500,
    includeRailings: params.include_railings ?? params.includeRailings ?? true
  });
};

const handleGenerateWall: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_wall', {
    name: params.name || 'Wall',
    splinePoints: params.spline_points || params.splinePoints,
    height: params.height || 500,
    thickness: params.thickness || 100,
    style: params.style || 'stone',
    includeGates: params.include_gates ?? params.includeGates ?? true,
    includeTowers: params.include_towers ?? params.includeTowers ?? true
  });
};

const handleGenerateTower: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_tower', {
    name: params.name || 'Tower',
    location: normalizeVector3(params.location),
    style: params.style || 'medieval',
    height: params.height || 1500,
    radius: params.radius || 400,
    floorCount: params.floor_count || params.floorCount || 5,
    includeInterior: params.include_interior ?? params.includeInterior ?? true
  });
};

const handleGenerateInterior: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const buildingName = validateRequiredString(
    params.building_name || params.buildingName,
    'building_name'
  );

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_interior', {
    buildingName,
    roomTypes: params.room_types || params.roomTypes,
    furnitureDensity: params.furniture_density || params.furnitureDensity || 0.5,
    style: params.style || 'default'
  });
};

const handleFurnishRoom: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const roomType = validateRequiredString(params.room_type || params.roomType, 'room_type');

  return executeForge2Action(tools, Forge2Categories.WORLD, 'furnish_room', {
    roomType,
    bounds: params.bounds,
    style: params.style || 'default',
    density: params.density || 0.5,
    includeLighting: params.include_lighting ?? params.includeLighting ?? true
  });
};

// ============================================================================
// PCG Framework Handlers
// ============================================================================

const handleCreatePCGGraph: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const name = validateRequiredString(params.name, 'name');

  return executeForge2Action(tools, Forge2Categories.WORLD, 'create_pcg_graph', {
    name,
    path: normalizeAssetPath(String(params.path || `/Game/FORGE2/PCG/${name}`))
  });
};

const handleAddPCGNode: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const graphPath = validateRequiredString(
    params.graph_path || params.graphPath,
    'graph_path'
  );
  const nodeType = validateRequiredString(params.node_type || params.nodeType, 'node_type');

  return executeForge2Action(tools, Forge2Categories.WORLD, 'add_pcg_node', {
    graphPath: normalizeAssetPath(graphPath),
    nodeType,
    nodeId: params.node_id || params.nodeId,
    settings: params.settings || {}
  });
};

const handleConnectPCGNodes: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const graphPath = validateRequiredString(
    params.graph_path || params.graphPath,
    'graph_path'
  );
  const sourceNodeId = validateRequiredString(
    params.source_node_id || params.sourceNodeId,
    'source_node_id'
  );
  const targetNodeId = validateRequiredString(
    params.target_node_id || params.targetNodeId,
    'target_node_id'
  );

  return executeForge2Action(tools, Forge2Categories.WORLD, 'connect_pcg_nodes', {
    graphPath: normalizeAssetPath(graphPath),
    sourceNodeId,
    targetNodeId,
    outputPin: params.output_pin || params.outputPin || 'Out',
    inputPin: params.input_pin || params.inputPin || 'In'
  });
};

const handleExecutePCGGraph: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const graphPath = validateRequiredString(
    params.graph_path || params.graphPath,
    'graph_path'
  );

  return executeForge2Action(tools, Forge2Categories.WORLD, 'execute_pcg_graph', {
    graphPath: normalizeAssetPath(graphPath),
    location: normalizeVector3(params.location),
    seed: params.seed
  });
};

const handleSetPCGSettings: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  const graphPath = validateRequiredString(
    params.graph_path || params.graphPath,
    'graph_path'
  );

  return executeForge2Action(tools, Forge2Categories.WORLD, 'set_pcg_settings', {
    graphPath: normalizeAssetPath(graphPath),
    settings: params.settings || {}
  });
};

// ============================================================================
// Environment Population Handlers
// ============================================================================

const handleScatterProps: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'scatter_props', {
    meshes: params.meshes,
    bounds: params.bounds,
    density: params.density || 0.5,
    randomRotation: params.random_rotation ?? params.randomRotation ?? true,
    randomScaleRange: params.random_scale_range || params.randomScaleRange || [0.8, 1.2],
    alignToSurface: params.align_to_surface ?? params.alignToSurface ?? true,
    seed: params.seed
  });
};

const handlePlaceVegetation: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'place_vegetation', {
    biome: params.biome,
    bounds: params.bounds,
    density: params.density || 1.0,
    respectElevation: params.respect_elevation ?? params.respectElevation ?? true,
    respectSlope: params.respect_slope ?? params.respectSlope ?? true,
    seed: params.seed
  });
};

const handleGenerateRocks: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'generate_rocks', {
    meshes: params.meshes,
    bounds: params.bounds,
    density: params.density || 0.3,
    sizeRange: params.size_range || params.sizeRange || [0.5, 2.0],
    embedInTerrain: params.embed_in_terrain ?? params.embedInTerrain ?? true,
    seed: params.seed
  });
};

const handlePlaceDecals: ActionHandler = async (tools, args) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.WORLD, 'place_decals', {
    decalMaterials: params.decal_materials || params.decalMaterials,
    bounds: params.bounds,
    density: params.density || 0.2,
    sizeRange: params.size_range || params.sizeRange || [100, 500],
    seed: params.seed
  });
};

// ============================================================================
// Action Handler Map
// ============================================================================

const worldgenHandlers: ActionHandlerMap = {
  // Terrain Generation
  'generate_heightmap': handleGenerateHeightmap,
  'generate_noise_terrain': handleGenerateNoiseTerrain,
  'apply_thermal_erosion': handleApplyThermalErosion,
  'apply_hydraulic_erosion': handleApplyHydraulicErosion,
  'import_heightmap': handleImportHeightmap,
  'export_heightmap': handleExportHeightmap,

  // Biome System
  'define_biome': handleDefineBiome,
  'create_biome_map': handleCreateBiomeMap,
  'apply_biome': handleApplyBiome,
  'blend_biome_edges': handleBlendBiomeEdges,

  // Water Bodies
  'create_river': handleCreateRiver,
  'create_lake': handleCreateLake,
  'create_ocean': handleCreateOcean,
  'simulate_water_flow': handleSimulateWaterFlow,

  // Structure Generation
  'generate_building': handleGenerateBuilding,
  'generate_dungeon': handleGenerateDungeon,
  'generate_city': handleGenerateCity,
  'generate_village': handleGenerateVillage,
  'generate_road_network': handleGenerateRoadNetwork,
  'generate_bridge': handleGenerateBridge,
  'generate_wall': handleGenerateWall,
  'generate_tower': handleGenerateTower,
  'generate_interior': handleGenerateInterior,
  'furnish_room': handleFurnishRoom,

  // PCG Framework
  'create_pcg_graph': handleCreatePCGGraph,
  'add_pcg_node': handleAddPCGNode,
  'connect_pcg_nodes': handleConnectPCGNodes,
  'execute_pcg_graph': handleExecutePCGGraph,
  'set_pcg_settings': handleSetPCGSettings,

  // Environment Population
  'scatter_props': handleScatterProps,
  'place_vegetation': handlePlaceVegetation,
  'generate_rocks': handleGenerateRocks,
  'place_decals': handlePlaceDecals
};

// ============================================================================
// Main Handler Export
// ============================================================================

/**
 * Handle worldgen tool actions
 */
export const handleWorldgenTools = createCategoryHandler(Forge2Categories.WORLD, worldgenHandlers);

/**
 * Legacy handler for direct use
 */
export async function handleWorldGeneration(
  action: string,
  args: HandlerArgs,
  tools: ITools
): Promise<Record<string, unknown>> {
  const response = await handleWorldgenTools(action, args as Record<string, unknown>, tools);
  return cleanObject(response as unknown) as Record<string, unknown>;
}
