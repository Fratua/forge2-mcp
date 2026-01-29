/**
 * FORGE2: Extended Validation Utilities
 * Additional validation helpers for FORGE2 tool parameters
 */

import type { HandlerArgs } from '../../../types/handler-types.js';

// ============================================================================
// FORGE2-Specific Validators
// ============================================================================

/**
 * Validate that a value is one of the allowed enum values
 */
export function validateEnum<T extends string>(
  value: unknown,
  allowedValues: readonly T[],
  fieldName: string
): T {
  if (typeof value !== 'string') {
    throw new Error(`${fieldName} must be a string, got ${typeof value}`);
  }
  if (!allowedValues.includes(value as T)) {
    throw new Error(`${fieldName} must be one of: ${allowedValues.join(', ')}. Got: ${value}`);
  }
  return value as T;
}

/**
 * Validate that a value is within a numeric range
 */
export function validateRange(
  value: unknown,
  min: number,
  max: number,
  fieldName: string
): number {
  if (typeof value !== 'number' || isNaN(value)) {
    throw new Error(`${fieldName} must be a number, got ${typeof value}`);
  }
  if (value < min || value > max) {
    throw new Error(`${fieldName} must be between ${min} and ${max}. Got: ${value}`);
  }
  return value;
}

/**
 * Validate that a value is a positive integer
 */
export function validatePositiveInteger(value: unknown, fieldName: string): number {
  if (typeof value !== 'number' || !Number.isInteger(value) || value < 1) {
    throw new Error(`${fieldName} must be a positive integer. Got: ${value}`);
  }
  return value;
}

/**
 * Validate that a value is a non-negative number
 */
export function validateNonNegative(value: unknown, fieldName: string): number {
  if (typeof value !== 'number' || isNaN(value) || value < 0) {
    throw new Error(`${fieldName} must be a non-negative number. Got: ${value}`);
  }
  return value;
}

/**
 * Validate a seed value (integer)
 */
export function validateSeed(value: unknown): number {
  if (value === undefined || value === null) {
    return Math.floor(Math.random() * 2147483647);
  }
  if (typeof value !== 'number' || !Number.isInteger(value)) {
    throw new Error(`Seed must be an integer. Got: ${value}`);
  }
  return value;
}

/**
 * Validate an ID string (alphanumeric with underscores)
 */
export function validateId(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} must be a non-empty string`);
  }
  if (!/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(value)) {
    throw new Error(`${fieldName} must be alphanumeric starting with a letter or underscore. Got: ${value}`);
  }
  return value;
}

/**
 * Validate an asset path (Unreal Engine path format)
 */
export function validateAssetPath(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} must be a non-empty string`);
  }
  // Basic UE path validation: should start with /Game/ or /Engine/ or plugin paths
  if (!value.startsWith('/') && !value.includes(':')) {
    throw new Error(`${fieldName} should be a valid Unreal asset path. Got: ${value}`);
  }
  return value;
}

/**
 * Validate an optional asset path (allows undefined)
 */
export function validateOptionalAssetPath(value: unknown, fieldName: string): string | undefined {
  if (value === undefined || value === null) return undefined;
  return validateAssetPath(value, fieldName);
}

// ============================================================================
// Array Validators
// ============================================================================

/**
 * Validate an array has at least a minimum number of elements
 */
export function validateArrayMinLength<T>(
  value: unknown,
  minLength: number,
  fieldName: string
): T[] {
  if (!Array.isArray(value)) {
    throw new Error(`${fieldName} must be an array`);
  }
  if (value.length < minLength) {
    throw new Error(`${fieldName} must have at least ${minLength} elements. Got: ${value.length}`);
  }
  return value as T[];
}

/**
 * Validate an array of numbers
 */
export function validateNumberArray(value: unknown, fieldName: string): number[] {
  if (!Array.isArray(value)) {
    throw new Error(`${fieldName} must be an array`);
  }
  for (let i = 0; i < value.length; i++) {
    if (typeof value[i] !== 'number' || isNaN(value[i])) {
      throw new Error(`${fieldName}[${i}] must be a number`);
    }
  }
  return value as number[];
}

/**
 * Validate an array of strings
 */
export function validateStringArray(value: unknown, fieldName: string): string[] {
  if (!Array.isArray(value)) {
    throw new Error(`${fieldName} must be an array`);
  }
  for (let i = 0; i < value.length; i++) {
    if (typeof value[i] !== 'string') {
      throw new Error(`${fieldName}[${i}] must be a string`);
    }
  }
  return value as string[];
}

// ============================================================================
// Object Validators
// ============================================================================

/**
 * Validate a location object or array
 */
export interface Location3D {
  x: number;
  y: number;
  z: number;
}

export function validateLocation(value: unknown, fieldName: string): Location3D {
  if (Array.isArray(value)) {
    if (value.length < 3) {
      throw new Error(`${fieldName} array must have at least 3 elements (x, y, z)`);
    }
    return { x: Number(value[0]), y: Number(value[1]), z: Number(value[2]) };
  }
  if (typeof value === 'object' && value !== null) {
    const obj = value as Record<string, unknown>;
    return {
      x: Number(obj.x ?? 0),
      y: Number(obj.y ?? 0),
      z: Number(obj.z ?? 0)
    };
  }
  throw new Error(`${fieldName} must be an object {x, y, z} or array [x, y, z]`);
}

/**
 * Validate an optional location
 */
export function validateOptionalLocation(value: unknown, fieldName: string): Location3D | undefined {
  if (value === undefined || value === null) return undefined;
  return validateLocation(value, fieldName);
}

/**
 * Validate bounds (min/max location pair)
 */
export interface Bounds3D {
  min: Location3D;
  max: Location3D;
}

export function validateBounds(value: unknown, fieldName: string): Bounds3D {
  if (typeof value !== 'object' || value === null) {
    throw new Error(`${fieldName} must be an object with min and max`);
  }
  const obj = value as Record<string, unknown>;
  return {
    min: validateLocation(obj.min, `${fieldName}.min`),
    max: validateLocation(obj.max, `${fieldName}.max`)
  };
}

// ============================================================================
// FORGE2 Domain-Specific Validators
// ============================================================================

/**
 * Noise algorithm types
 */
export const NOISE_TYPES = ['perlin', 'simplex', 'voronoi', 'fbm', 'ridged', 'worley'] as const;
export type NoiseType = typeof NOISE_TYPES[number];

export function validateNoiseType(value: unknown): NoiseType {
  return validateEnum(value, NOISE_TYPES, 'noiseType');
}

/**
 * Dungeon algorithm types
 */
export const DUNGEON_ALGORITHMS = ['bsp', 'wfc', 'cellular_automata', 'drunkards_walk'] as const;
export type DungeonAlgorithm = typeof DUNGEON_ALGORITHMS[number];

export function validateDungeonAlgorithm(value: unknown): DungeonAlgorithm {
  return validateEnum(value, DUNGEON_ALGORITHMS, 'dungeonAlgorithm');
}

/**
 * Building style types
 */
export const BUILDING_STYLES = ['medieval', 'fantasy', 'modern', 'scifi', 'ancient', 'victorian', 'japanese', 'arabic'] as const;
export type BuildingStyle = typeof BUILDING_STYLES[number];

export function validateBuildingStyle(value: unknown): BuildingStyle {
  return validateEnum(value, BUILDING_STYLES, 'buildingStyle');
}

/**
 * Item rarity types
 */
export const ITEM_RARITIES = ['common', 'uncommon', 'rare', 'epic', 'legendary', 'mythic', 'unique'] as const;
export type ItemRarity = typeof ITEM_RARITIES[number];

export function validateItemRarity(value: unknown): ItemRarity {
  return validateEnum(value, ITEM_RARITIES, 'rarity');
}

/**
 * Validate noise generation parameters
 */
export interface NoiseParams {
  noiseType: NoiseType;
  seed: number;
  scale: number;
  octaves: number;
  persistence: number;
  lacunarity: number;
}

export function validateNoiseParams(args: HandlerArgs): NoiseParams {
  return {
    noiseType: validateNoiseType(args.noiseType ?? 'perlin'),
    seed: validateSeed(args.seed),
    scale: validateNonNegative(args.scale ?? 100, 'scale'),
    octaves: validateRange(args.octaves ?? 4, 1, 16, 'octaves'),
    persistence: validateRange(args.persistence ?? 0.5, 0, 1, 'persistence'),
    lacunarity: validateRange(args.lacunarity ?? 2.0, 1, 4, 'lacunarity')
  };
}

/**
 * Validate dungeon generation parameters
 */
export interface DungeonParams {
  algorithm: DungeonAlgorithm;
  seed: number;
  roomCount: number;
  roomSizeMin: number;
  roomSizeMax: number;
  corridorWidth: number;
  includeLoops: boolean;
}

export function validateDungeonParams(args: HandlerArgs): DungeonParams {
  const roomSizeRange = Array.isArray(args.roomSizeRange) ? args.roomSizeRange : [10, 30];

  return {
    algorithm: validateDungeonAlgorithm(args.dungeonAlgorithm ?? 'bsp'),
    seed: validateSeed(args.seed),
    roomCount: validateRange(args.roomCount ?? 10, 1, 100, 'roomCount'),
    roomSizeMin: validateNonNegative(roomSizeRange[0] ?? 10, 'roomSizeMin'),
    roomSizeMax: validateNonNegative(roomSizeRange[1] ?? 30, 'roomSizeMax'),
    corridorWidth: validateRange(args.corridorWidth ?? 3, 1, 20, 'corridorWidth'),
    includeLoops: Boolean(args.includeLoops ?? false)
  };
}
