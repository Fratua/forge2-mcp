/**
 * FORGE2: VFX Handlers
 * Handles UE5 Niagara visual effects, particle systems, emitters, and renderers
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
// Constants
// ============================================================================

const EMITTER_TYPES = ['sprite', 'mesh', 'ribbon', 'light', 'component'] as const;
const RENDERER_TYPES = ['sprite', 'mesh', 'ribbon', 'light', 'component', 'decal'] as const;
const SPAWN_MODES = ['continuous', 'burst', 'event'] as const;
const FORCE_TYPES = ['drag', 'noise', 'curl', 'vortex', 'point_force', 'vector_field'] as const;
const SIM_STAGES = ['spawn', 'update', 'render'] as const;

// Reserve constants for future use
void EMITTER_TYPES;
void RENDERER_TYPES;
void SPAWN_MODES;
void FORCE_TYPES;
void SIM_STAGES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a Niagara system
 */
const handleCreateNiagaraSystem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemName = validateRequiredString(params.system_name || params.systemName, 'system_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'create_niagara_system', {
    systemName,
    templatePath: validateOptionalString(params.template_path || params.templatePath),
    warmupTime: validateOptionalNumber(params.warmup_time || params.warmupTime) ?? 0,
    looping: validateOptionalBoolean(params.looping) ?? false,
    autoActivate: validateOptionalBoolean(params.auto_activate || params.autoActivate) ?? true,
    poolingEnabled: validateOptionalBoolean(params.pooling_enabled || params.poolingEnabled) ?? false,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add emitter to Niagara system
 */
const handleAddEmitter: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const emitterName = validateRequiredString(params.emitter_name || params.emitterName, 'emitter_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'add_emitter', {
    systemPath,
    emitterName,
    emitterType: validateOptionalEnum(params.emitter_type || params.emitterType, EMITTER_TYPES) ?? 'sprite',
    templatePath: validateOptionalString(params.template_path || params.templatePath),
    gpuEnabled: validateOptionalBoolean(params.gpu_enabled || params.gpuEnabled) ?? true,
    localSpace: validateOptionalBoolean(params.local_space || params.localSpace) ?? false,
    sortOrder: validateOptionalNumber(params.sort_order || params.sortOrder)
  });
};

/**
 * Add Niagara module
 */
const handleAddNiagaraModule: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const emitterName = validateRequiredString(params.emitter_name || params.emitterName, 'emitter_name');
  const moduleName = validateRequiredString(params.module_name || params.moduleName, 'module_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'add_niagara_module', {
    systemPath,
    emitterName,
    moduleName,
    stage: validateOptionalEnum(params.stage, SIM_STAGES) ?? 'update',
    modulePath: validateOptionalString(params.module_path || params.modulePath),
    parameters: validateOptionalObject(params.parameters)
  });
};

/**
 * Configure particle spawn
 */
const handleConfigureParticleSpawn: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const emitterName = validateRequiredString(params.emitter_name || params.emitterName, 'emitter_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'configure_particle_spawn', {
    systemPath,
    emitterName,
    spawnMode: validateOptionalEnum(params.spawn_mode || params.spawnMode, SPAWN_MODES) ?? 'continuous',
    spawnRate: validateOptionalNumber(params.spawn_rate || params.spawnRate) ?? 100,
    burstCount: validateOptionalNumber(params.burst_count || params.burstCount),
    burstTime: validateOptionalNumber(params.burst_time || params.burstTime),
    maxParticles: validateOptionalNumber(params.max_particles || params.maxParticles) ?? 1000,
    spawnLocation: validateOptionalObject(params.spawn_location || params.spawnLocation),
    spawnShape: validateOptionalString(params.spawn_shape || params.spawnShape) // 'point', 'sphere', 'box', 'cylinder', 'mesh'
  });
};

/**
 * Setup particle attributes
 */
const handleSetupParticleAttributes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const emitterName = validateRequiredString(params.emitter_name || params.emitterName, 'emitter_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'setup_particle_attributes', {
    systemPath,
    emitterName,
    lifetime: validateOptionalObject(params.lifetime), // { min, max }
    size: validateOptionalObject(params.size), // { min, max } or curve
    color: validateOptionalObject(params.color), // { r, g, b, a } or curve
    velocity: validateOptionalObject(params.velocity), // { x, y, z } ranges
    rotation: validateOptionalObject(params.rotation),
    mass: validateOptionalNumber(params.mass) ?? 1.0,
    spriteSize: validateOptionalObject(params.sprite_size || params.spriteSize),
    subUVIndex: validateOptionalNumber(params.sub_uv_index || params.subUVIndex)
  });
};

/**
 * Create particle forces
 */
const handleCreateParticleForces: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const emitterName = validateRequiredString(params.emitter_name || params.emitterName, 'emitter_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'create_particle_forces', {
    systemPath,
    emitterName,
    forceType: validateOptionalEnum(params.force_type || params.forceType, FORCE_TYPES) ?? 'drag',
    strength: validateOptionalNumber(params.strength) ?? 1.0,
    direction: validateOptionalObject(params.direction),
    noiseScale: validateOptionalNumber(params.noise_scale || params.noiseScale),
    noiseFrequency: validateOptionalNumber(params.noise_frequency || params.noiseFrequency),
    curlStrength: validateOptionalNumber(params.curl_strength || params.curlStrength),
    vectorFieldPath: validateOptionalString(params.vector_field_path || params.vectorFieldPath)
  });
};

/**
 * Configure collision
 */
const handleConfigureCollision: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const emitterName = validateRequiredString(params.emitter_name || params.emitterName, 'emitter_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'configure_collision', {
    systemPath,
    emitterName,
    enabled: validateOptionalBoolean(params.enabled) ?? true,
    collisionMode: validateOptionalString(params.collision_mode || params.collisionMode) ?? 'distance_field', // 'distance_field', 'depth_buffer', 'analytical'
    restitution: validateOptionalNumber(params.restitution) ?? 0.5,
    friction: validateOptionalNumber(params.friction) ?? 0.2,
    killOnCollision: validateOptionalBoolean(params.kill_on_collision || params.killOnCollision) ?? false,
    spawnOnCollision: validateOptionalBoolean(params.spawn_on_collision || params.spawnOnCollision) ?? false,
    collisionResponse: validateOptionalString(params.collision_response || params.collisionResponse) // 'bounce', 'kill', 'spawn'
  });
};

/**
 * Setup renderer
 */
const handleSetupRenderer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');
  const emitterName = validateRequiredString(params.emitter_name || params.emitterName, 'emitter_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'setup_renderer', {
    systemPath,
    emitterName,
    rendererType: validateOptionalEnum(params.renderer_type || params.rendererType, RENDERER_TYPES) ?? 'sprite',
    materialPath: validateOptionalString(params.material_path || params.materialPath),
    meshPath: validateOptionalString(params.mesh_path || params.meshPath),
    alignment: validateOptionalString(params.alignment) ?? 'velocity', // 'velocity', 'camera', 'custom'
    sortMode: validateOptionalString(params.sort_mode || params.sortMode) ?? 'view_depth',
    subUVAnimation: validateOptionalObject(params.sub_uv_animation || params.subUVAnimation),
    ribbonSettings: validateOptionalObject(params.ribbon_settings || params.ribbonSettings)
  });
};

/**
 * Create VFX event (impact, muzzle flash, etc.)
 */
const handleCreateVFXEvent: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const eventName = validateRequiredString(params.event_name || params.eventName, 'event_name');

  return executeForge2Action(tools, Forge2Categories.VFX, 'create_vfx_event', {
    eventName,
    eventType: validateRequiredString(params.event_type || params.eventType, 'event_type'), // 'impact', 'muzzle_flash', 'blood_splatter', 'explosion', 'dust', 'sparks'
    systemPath: validateOptionalString(params.system_path || params.systemPath),
    attachToActor: validateOptionalBoolean(params.attach_to_actor || params.attachToActor) ?? false,
    autoDestroy: validateOptionalBoolean(params.auto_destroy || params.autoDestroy) ?? true,
    poolSize: validateOptionalNumber(params.pool_size || params.poolSize) ?? 10,
    surfaceTypes: validateOptionalArray(params.surface_types || params.surfaceTypes)
  });
};

/**
 * Configure GPU particles
 */
const handleConfigureGPUParticles: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.VFX, 'configure_gpu_particles', {
    systemPath,
    emitterName: validateOptionalString(params.emitter_name || params.emitterName),
    computeShaderEnabled: validateOptionalBoolean(params.compute_shader_enabled || params.computeShaderEnabled) ?? true,
    fixedBounds: validateOptionalObject(params.fixed_bounds || params.fixedBounds),
    gpuSpawnEnabled: validateOptionalBoolean(params.gpu_spawn_enabled || params.gpuSpawnEnabled) ?? true,
    simulationStagesPerFrame: validateOptionalNumber(params.simulation_stages_per_frame || params.simulationStagesPerFrame) ?? 1
  });
};

/**
 * Configure VFX pooling
 */
const handleConfigureVFXPooling: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.VFX, 'configure_vfx_pooling', {
    systemPath,
    poolEnabled: validateOptionalBoolean(params.pool_enabled || params.poolEnabled) ?? true,
    poolSize: validateOptionalNumber(params.pool_size || params.poolSize) ?? 10,
    prewarmEnabled: validateOptionalBoolean(params.prewarm_enabled || params.prewarmEnabled) ?? false,
    prewarmTime: validateOptionalNumber(params.prewarm_time || params.prewarmTime) ?? 1.0,
    recycleOnComplete: validateOptionalBoolean(params.recycle_on_complete || params.recycleOnComplete) ?? true
  });
};

/**
 * Get Niagara system info
 */
const handleGetNiagaraSystemInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const systemPath = validateRequiredString(params.system_path || params.systemPath, 'system_path');

  return executeForge2Action(tools, Forge2Categories.VFX, 'get_niagara_system_info', {
    systemPath,
    includeEmitters: validateOptionalBoolean(params.include_emitters || params.includeEmitters) ?? true,
    includeModules: validateOptionalBoolean(params.include_modules || params.includeModules) ?? true,
    includeParameters: validateOptionalBoolean(params.include_parameters || params.includeParameters) ?? false
  });
};

/**
 * List Niagara systems
 */
const handleListNiagaraSystems: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.VFX, 'list_niagara_systems', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const vfxHandlers: ActionHandlerMap = {
  // System Creation
  create_niagara_system: handleCreateNiagaraSystem,
  add_emitter: handleAddEmitter,
  add_niagara_module: handleAddNiagaraModule,

  // Particle Configuration
  configure_particle_spawn: handleConfigureParticleSpawn,
  setup_particle_attributes: handleSetupParticleAttributes,
  create_particle_forces: handleCreateParticleForces,
  configure_collision: handleConfigureCollision,

  // Renderers
  setup_renderer: handleSetupRenderer,

  // GPU & Performance
  configure_gpu_particles: handleConfigureGPUParticles,
  configure_vfx_pooling: handleConfigureVFXPooling,

  // Events
  create_vfx_event: handleCreateVFXEvent,

  // Query
  get_niagara_system_info: handleGetNiagaraSystemInfo,
  list_niagara_systems: handleListNiagaraSystems
};

export const handleVFXTools = createCategoryHandler(Forge2Categories.VFX, vfxHandlers);

export default vfxHandlers;
