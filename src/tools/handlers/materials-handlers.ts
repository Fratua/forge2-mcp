/**
 * FORGE2: Materials Handlers
 * Handles UE5 Material creation, instances, parameters, and landscape materials
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

const MATERIAL_DOMAINS = ['surface', 'deferred_decal', 'light_function', 'volume', 'post_process', 'ui'] as const;
const BLEND_MODES = ['opaque', 'masked', 'translucent', 'additive', 'modulate'] as const;
const SHADING_MODELS = ['default_lit', 'unlit', 'subsurface', 'clear_coat', 'hair', 'cloth', 'eye', 'two_sided_foliage'] as const;
const PARAMETER_TYPES = ['scalar', 'vector', 'texture', 'static_switch', 'component_mask'] as const;

// Reserve constants for future use
void MATERIAL_DOMAINS;
void BLEND_MODES;
void SHADING_MODELS;
void PARAMETER_TYPES;

// ============================================================================
// Action Handlers
// ============================================================================

/**
 * Create a new material
 */
const handleCreateMaterial: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialName = validateRequiredString(params.material_name || params.materialName, 'material_name');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'create_material', {
    materialName,
    materialDomain: validateOptionalEnum(params.material_domain || params.materialDomain, MATERIAL_DOMAINS) ?? 'surface',
    blendMode: validateOptionalEnum(params.blend_mode || params.blendMode, BLEND_MODES) ?? 'opaque',
    shadingModel: validateOptionalEnum(params.shading_model || params.shadingModel, SHADING_MODELS) ?? 'default_lit',
    twoSided: validateOptionalBoolean(params.two_sided || params.twoSided) ?? false,
    useNanite: validateOptionalBoolean(params.use_nanite || params.useNanite) ?? true,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Create a material instance
 */
const handleCreateMaterialInstance: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const parentMaterialPath = validateRequiredString(params.parent_material_path || params.parentMaterialPath, 'parent_material_path');
  const instanceName = validateRequiredString(params.instance_name || params.instanceName, 'instance_name');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'create_material_instance', {
    parentMaterialPath,
    instanceName,
    scalarParameters: validateOptionalObject(params.scalar_parameters || params.scalarParameters),
    vectorParameters: validateOptionalObject(params.vector_parameters || params.vectorParameters),
    textureParameters: validateOptionalObject(params.texture_parameters || params.textureParameters),
    staticSwitchParameters: validateOptionalObject(params.static_switch_parameters || params.staticSwitchParameters),
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Set material parameter
 */
const handleSetMaterialParameter: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialPath = validateRequiredString(params.material_path || params.materialPath, 'material_path');
  const parameterName = validateRequiredString(params.parameter_name || params.parameterName, 'parameter_name');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'set_material_parameter', {
    materialPath,
    parameterName,
    parameterType: validateOptionalEnum(params.parameter_type || params.parameterType, PARAMETER_TYPES) ?? 'scalar',
    value: params.value,
    group: validateOptionalString(params.group),
    sortPriority: validateOptionalNumber(params.sort_priority || params.sortPriority)
  });
};

/**
 * Configure material parameters
 */
const handleConfigureMaterialParameters: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialPath = validateRequiredString(params.material_path || params.materialPath, 'material_path');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'configure_material_parameters', {
    materialPath,
    baseColor: validateOptionalObject(params.base_color || params.baseColor),
    metallic: validateOptionalNumber(params.metallic),
    specular: validateOptionalNumber(params.specular),
    roughness: validateOptionalNumber(params.roughness),
    emissive: validateOptionalObject(params.emissive),
    normal: validateOptionalString(params.normal),
    opacity: validateOptionalNumber(params.opacity),
    opacityMask: validateOptionalNumber(params.opacity_mask || params.opacityMask),
    worldPositionOffset: validateOptionalObject(params.world_position_offset || params.worldPositionOffset),
    ambientOcclusion: validateOptionalNumber(params.ambient_occlusion || params.ambientOcclusion),
    pixelDepthOffset: validateOptionalNumber(params.pixel_depth_offset || params.pixelDepthOffset)
  });
};

/**
 * Create material function
 */
const handleCreateMaterialFunction: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const functionName = validateRequiredString(params.function_name || params.functionName, 'function_name');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'create_material_function', {
    functionName,
    description: validateOptionalString(params.description),
    exposeToLibrary: validateOptionalBoolean(params.expose_to_library || params.exposeToLibrary) ?? true,
    inputs: validateOptionalArray(params.inputs) ?? [],
    outputs: validateOptionalArray(params.outputs) ?? [],
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Create landscape material
 */
const handleCreateLandscapeMaterial: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialName = validateRequiredString(params.material_name || params.materialName, 'material_name');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'create_landscape_material', {
    materialName,
    layers: validateOptionalArray(params.layers) ?? [],
    useWeightBlend: validateOptionalBoolean(params.use_weight_blend || params.useWeightBlend) ?? true,
    useHeightBlend: validateOptionalBoolean(params.use_height_blend || params.useHeightBlend) ?? false,
    heightBlendSharpness: validateOptionalNumber(params.height_blend_sharpness || params.heightBlendSharpness) ?? 0.5,
    tessellationEnabled: validateOptionalBoolean(params.tessellation_enabled || params.tessellationEnabled) ?? false,
    virtualTexturingEnabled: validateOptionalBoolean(params.virtual_texturing_enabled || params.virtualTexturingEnabled) ?? true,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Add landscape layer
 */
const handleAddLandscapeLayer: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialPath = validateRequiredString(params.material_path || params.materialPath, 'material_path');
  const layerName = validateRequiredString(params.layer_name || params.layerName, 'layer_name');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'add_landscape_layer', {
    materialPath,
    layerName,
    baseColorTexture: validateOptionalString(params.base_color_texture || params.baseColorTexture),
    normalTexture: validateOptionalString(params.normal_texture || params.normalTexture),
    roughnessTexture: validateOptionalString(params.roughness_texture || params.roughnessTexture),
    heightTexture: validateOptionalString(params.height_texture || params.heightTexture),
    tiling: validateOptionalNumber(params.tiling) ?? 1.0,
    roughnessMultiplier: validateOptionalNumber(params.roughness_multiplier || params.roughnessMultiplier) ?? 1.0
  });
};

/**
 * Setup procedural textures
 */
const handleSetupProceduralTextures: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialPath = validateRequiredString(params.material_path || params.materialPath, 'material_path');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'setup_procedural_textures', {
    materialPath,
    noiseType: validateOptionalString(params.noise_type || params.noiseType) ?? 'perlin', // 'perlin', 'simplex', 'voronoi', 'worley'
    scale: validateOptionalNumber(params.scale) ?? 1.0,
    octaves: validateOptionalNumber(params.octaves) ?? 4,
    persistence: validateOptionalNumber(params.persistence) ?? 0.5,
    tiling: validateOptionalBoolean(params.tiling) ?? true,
    seed: validateOptionalNumber(params.seed)
  });
};

/**
 * Configure subsurface scattering
 */
const handleConfigureSubsurfaceScattering: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialPath = validateRequiredString(params.material_path || params.materialPath, 'material_path');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'configure_subsurface_scattering', {
    materialPath,
    subsurfaceColor: validateOptionalObject(params.subsurface_color || params.subsurfaceColor),
    subsurfaceProfilePath: validateOptionalString(params.subsurface_profile_path || params.subsurfaceProfilePath),
    opacity: validateOptionalNumber(params.opacity) ?? 1.0,
    scatterRadius: validateOptionalNumber(params.scatter_radius || params.scatterRadius) ?? 1.0
  });
};

/**
 * Create decal material
 */
const handleCreateDecalMaterial: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialName = validateRequiredString(params.material_name || params.materialName, 'material_name');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'create_decal_material', {
    materialName,
    decalBlendMode: validateOptionalString(params.decal_blend_mode || params.decalBlendMode) ?? 'translucent', // 'translucent', 'stain', 'normal', 'emissive'
    sortOrder: validateOptionalNumber(params.sort_order || params.sortOrder) ?? 0,
    receivesDecals: validateOptionalBoolean(params.receives_decals || params.receivesDecals) ?? false,
    outputPath: validateOptionalString(params.output_path || params.outputPath)
  });
};

/**
 * Get material info
 */
const handleGetMaterialInfo: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const materialPath = validateRequiredString(params.material_path || params.materialPath, 'material_path');

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'get_material_info', {
    materialPath,
    includeParameters: validateOptionalBoolean(params.include_parameters || params.includeParameters) ?? true,
    includeTextures: validateOptionalBoolean(params.include_textures || params.includeTextures) ?? true,
    includeFunctions: validateOptionalBoolean(params.include_functions || params.includeFunctions) ?? false
  });
};

/**
 * List materials
 */
const handleListMaterials: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.MATERIALS, 'list_materials', {
    folderPath: validateOptionalString(params.folder_path || params.folderPath),
    recursive: validateOptionalBoolean(params.recursive) ?? true,
    includeInstances: validateOptionalBoolean(params.include_instances || params.includeInstances) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const materialsHandlers: ActionHandlerMap = {
  // Material Creation
  create_material: handleCreateMaterial,
  create_material_instance: handleCreateMaterialInstance,
  create_material_function: handleCreateMaterialFunction,
  create_decal_material: handleCreateDecalMaterial,

  // Parameters
  set_material_parameter: handleSetMaterialParameter,
  configure_material_parameters: handleConfigureMaterialParameters,

  // Landscape
  create_landscape_material: handleCreateLandscapeMaterial,
  add_landscape_layer: handleAddLandscapeLayer,

  // Advanced
  setup_procedural_textures: handleSetupProceduralTextures,
  configure_subsurface_scattering: handleConfigureSubsurfaceScattering,

  // Query
  get_material_info: handleGetMaterialInfo,
  list_materials: handleListMaterials
};

export const handleMaterialsTools = createCategoryHandler(Forge2Categories.MATERIALS, materialsHandlers);

export default materialsHandlers;
