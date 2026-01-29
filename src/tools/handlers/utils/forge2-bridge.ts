/**
 * FORGE2: Unreal Engine Bridge Utilities
 * Extended utilities for FORGE2 tool handlers communicating with Unreal Engine
 */

import { ITools } from '../../../types/tool-interfaces.js';
import type { HandlerArgs } from '../../../types/handler-types.js';
import {
  createErrorResponse,
  createSuccessResponse,
  createBridgeNotConnectedResponse,
  ErrorCodes,
  type Forge2Response
} from './response.js';

// ============================================================================
// Bridge Communication Types
// ============================================================================

export interface Forge2BridgeRequest {
  category: string;
  action: string;
  params: Record<string, unknown>;
  [key: string]: unknown;
}

export interface Forge2BridgeResponse {
  success: boolean;
  message?: string;
  data?: Record<string, unknown>;
  error?: string;
  errorCode?: string;
}

export interface BridgeRequestOptions {
  /** Timeout in milliseconds (default: 30000) */
  timeoutMs?: number;
  /** Whether to retry on timeout (default: false) */
  retryOnTimeout?: boolean;
  /** Max retry attempts (default: 2) */
  maxRetries?: number;
}

// ============================================================================
// FORGE2 Tool Categories
// ============================================================================

export const Forge2Categories = {
  // Core Systems
  WORLD: 'worldgen',
  QUEST: 'quest',
  DIALOGUE: 'dialogue',
  SKILL_TREE: 'skill_tree',
  INVENTORY: 'inventory',
  ECONOMY: 'economy',
  PROGRESSION: 'progression',
  COMBAT: 'combat',
  COMBAT_STATS: 'combat_stats',
  INPUT: 'input',
  CAMERA: 'camera',
  AI: 'ai_behavior',
  AI_ADVANCED: 'ai_advanced',
  PHYSICS: 'physics',
  DAMAGE: 'damage',

  // Genre-Specific
  GENRE_RACING: 'genre_racing',
  GENRE_FIGHTING: 'genre_fighting',
  GENRE_PLATFORMER: 'genre_platformer',
  GENRE_STEALTH: 'genre_stealth',
  GENRE_PUZZLE: 'genre_puzzle',
  GENRE_STRATEGY: 'genre_strategy',
  GENRE_SPORTS: 'genre_sports',
  GENRE_HORROR: 'genre_horror',
  GENRE_SURVIVAL: 'genre_survival',
  GENRE_MOBA: 'genre_moba',
  GENRE_BATTLE_ROYALE: 'genre_battle_royale',
  GENRE_RHYTHM: 'genre_rhythm',
  GENRE_CARD: 'genre_card',

  // AAA Tech Stack
  NANITE: 'nanite',
  LUMEN: 'lumen',
  METASOUNDS: 'metasounds',
  CONTROL_RIG: 'control_rig',
  CHAOS: 'chaos',
  CINEMATICS: 'cinematics',

  // Materials & VFX
  MATERIALS: 'materials',
  VFX: 'vfx',
  POST_PROCESS: 'post_process',

  // Audio
  AUDIO: 'audio',
  MUSIC: 'music',

  // Level & Environment
  LEVEL: 'level',
  TRIGGERS: 'triggers',
  SPAWNING: 'spawning',
  TIME_WEATHER: 'time_weather',
  ECOSYSTEM: 'ecosystem',
  DESTRUCTION: 'destruction',
  NPC_SYSTEMS: 'npc_systems',

  // Character
  CHARACTER_CREATOR: 'character_creator',
  CLASS_SYSTEM: 'class_system',
  MOUNT_SYSTEM: 'mount_system',

  // Data
  DATA_TABLES: 'data_tables',
  DATA_ASSETS: 'data_assets',

  // Blueprint
  BLUEPRINT: 'blueprint',
  WIDGET: 'widget',

  // Multiplayer
  NETWORKING: 'networking',
  MMO: 'mmo',
  SOCIAL: 'social',

  // Polish
  SAVE_SYSTEM: 'save_system',
  UI_SYSTEM: 'ui_system',
  ACCESSIBILITY: 'accessibility',
  LOCALIZATION: 'localization',
  PHOTO_MODE: 'photo_mode',
  ACHIEVEMENTS: 'achievements',
  ANALYTICS: 'analytics',

  // Build & Testing
  TESTING: 'testing',
  VALIDATION: 'validation',
  BUILD: 'build',
  PLATFORMS: 'platforms',

  // Context Engineering
  CONTEXT: 'context',

  // Runtime Systems (Phase 18-21)
  COMPONENTS: 'components',
  FRAMEWORK: 'framework',
  SUBSYSTEMS: 'subsystems',
  TEMPLATES: 'templates'
} as const;

export type Forge2Category = typeof Forge2Categories[keyof typeof Forge2Categories];

// ============================================================================
// Bridge Connection Check
// ============================================================================

/**
 * Check if the automation bridge is connected to Unreal Engine
 */
export function isBridgeConnected(tools: ITools): boolean {
  const bridge = tools.automationBridge;
  return !!(bridge && typeof bridge.isConnected === 'function' && bridge.isConnected());
}

/**
 * Get the automation bridge status
 */
export function getBridgeStatus(tools: ITools): { connected: boolean; status?: unknown } {
  const bridge = tools.automationBridge;
  if (!bridge || typeof bridge.getStatus !== 'function') {
    return { connected: false };
  }
  try {
    const status = bridge.getStatus();
    return { connected: bridge.isConnected?.() ?? false, status };
  } catch {
    return { connected: false };
  }
}

// ============================================================================
// FORGE2 Request Execution
// ============================================================================

/**
 * Send a FORGE2-specific automation request to Unreal Engine
 * This is the primary function for FORGE2 handlers to communicate with the C++ plugin
 */
export async function sendForge2Request(
  tools: ITools,
  category: Forge2Category,
  action: string,
  params: Record<string, unknown> = {},
  options: BridgeRequestOptions = {}
): Promise<Forge2BridgeResponse> {
  const bridge = tools.automationBridge;

  if (!bridge || typeof bridge.sendAutomationRequest !== 'function') {
    return {
      success: false,
      error: 'Automation bridge not available',
      errorCode: ErrorCodes.BRIDGE_NOT_CONNECTED
    };
  }

  if (!bridge.isConnected()) {
    return {
      success: false,
      error: 'Automation bridge is not connected to Unreal Engine',
      errorCode: ErrorCodes.BRIDGE_NOT_CONNECTED
    };
  }

  const request: Forge2BridgeRequest = {
    category,
    action,
    params
  };

  const timeoutMs = options.timeoutMs ?? 30000;
  const maxRetries = options.retryOnTimeout ? (options.maxRetries ?? 2) : 0;

  let lastError: Error | null = null;
  for (let attempt = 0; attempt <= maxRetries; attempt++) {
    try {
      const response = await bridge.sendAutomationRequest(
        `forge2_${category}`,
        request,
        { timeoutMs }
      );
      return response as Forge2BridgeResponse;
    } catch (error) {
      lastError = error instanceof Error ? error : new Error(String(error));
      if (attempt < maxRetries && lastError.message.includes('timeout')) {
        // Retry on timeout
        continue;
      }
      break;
    }
  }

  return {
    success: false,
    error: lastError?.message ?? 'Unknown error',
    errorCode: ErrorCodes.BRIDGE_ERROR
  };
}

/**
 * Execute a FORGE2 handler action and return a standardized response
 */
export async function executeForge2Action(
  tools: ITools,
  category: Forge2Category,
  action: string,
  params: Record<string, unknown>,
  options: BridgeRequestOptions = {}
): Promise<Forge2Response> {
  if (!isBridgeConnected(tools)) {
    return createBridgeNotConnectedResponse();
  }

  try {
    const response = await sendForge2Request(tools, category, action, params, options);

    if (response.success) {
      return createSuccessResponse(
        response.message ?? `Action '${action}' completed successfully`,
        response.data
      );
    } else {
      return createErrorResponse(
        response.error ?? `Action '${action}' failed`,
        response.errorCode ?? ErrorCodes.UNKNOWN_ERROR,
        response.data
      );
    }
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error);
    return createErrorResponse(
      `Failed to execute ${category}.${action}: ${message}`,
      ErrorCodes.BRIDGE_ERROR,
      { originalError: message }
    );
  }
}

// ============================================================================
// Batch Operations
// ============================================================================

export interface BatchOperation {
  action: string;
  params: Record<string, unknown>;
}

export interface BatchResult {
  success: boolean;
  results: Array<{
    action: string;
    success: boolean;
    data?: Record<string, unknown>;
    error?: string;
  }>;
  successCount: number;
  failureCount: number;
}

/**
 * Execute multiple FORGE2 actions in sequence
 */
export async function executeBatch(
  tools: ITools,
  category: Forge2Category,
  operations: BatchOperation[],
  options: BridgeRequestOptions & { stopOnError?: boolean } = {}
): Promise<BatchResult> {
  const results: BatchResult['results'] = [];
  let successCount = 0;
  let failureCount = 0;

  for (const op of operations) {
    const response = await executeForge2Action(tools, category, op.action, op.params, options);

    const result = {
      action: op.action,
      success: response.success,
      data: 'data' in response ? response.data : undefined,
      error: 'error' in response ? response.error : undefined
    };
    results.push(result);

    if (response.success) {
      successCount++;
    } else {
      failureCount++;
      if (options.stopOnError) {
        break;
      }
    }
  }

  return {
    success: failureCount === 0,
    results,
    successCount,
    failureCount
  };
}

// ============================================================================
// Handler Creation Utilities
// ============================================================================

export type ActionHandler = (
  tools: ITools,
  args: HandlerArgs
) => Promise<Forge2Response>;

export type ActionHandlerMap = Record<string, ActionHandler>;

/**
 * Create a category handler that routes to action-specific handlers
 * Returns a function with signature (action, args, tools) to match consolidated-tool-handlers pattern
 */
export function createCategoryHandler(
  category: Forge2Category,
  handlers: ActionHandlerMap
): (action: string, args: Record<string, unknown>, tools: ITools) => Promise<Forge2Response> {
  return async (action: string, args: Record<string, unknown>, tools: ITools): Promise<Forge2Response> => {
    if (typeof action !== 'string' || action.trim() === '') {
      return createErrorResponse(
        'Missing required parameter: action',
        ErrorCodes.MISSING_REQUIRED,
        { category }
      );
    }

    const handler = handlers[action];
    if (!handler) {
      return createErrorResponse(
        `Unknown action '${action}' for category '${category}'`,
        ErrorCodes.INVALID_ACTION,
        { category, action, availableActions: Object.keys(handlers) }
      );
    }

    try {
      // Merge action into args for handler
      const handlerArgs = { ...args, action } as HandlerArgs;
      return await handler(tools, handlerArgs);
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error);
      return createErrorResponse(
        `Error in ${category}.${action}: ${message}`,
        ErrorCodes.UNKNOWN_ERROR,
        { category, action, originalError: message }
      );
    }
  };
}

/**
 * Create a simple passthrough handler that forwards to Unreal Engine
 */
export function createPassthroughHandler(
  category: Forge2Category,
  action: string,
  paramExtractor?: (args: HandlerArgs) => Record<string, unknown>
): ActionHandler {
  return async (tools: ITools, args: HandlerArgs): Promise<Forge2Response> => {
    const params = paramExtractor ? paramExtractor(args) : { ...args };
    delete (params as Record<string, unknown>).action;
    return executeForge2Action(tools, category, action, params);
  };
}

// ============================================================================
// Asset Path Utilities
// ============================================================================

/**
 * Ensure an asset path has the correct format for Unreal Engine
 */
export function normalizeAssetPath(path: string): string {
  if (!path) return path;

  // Already a valid UE path
  if (path.startsWith('/Game/') || path.startsWith('/Engine/') || path.startsWith('/Script/')) {
    return path;
  }

  // Convert relative paths to /Game/
  if (path.startsWith('Content/')) {
    return '/Game/' + path.substring('Content/'.length);
  }

  // Add /Game/ prefix if not present
  if (!path.startsWith('/')) {
    return '/Game/' + path;
  }

  return path;
}

/**
 * Extract asset name from a full path
 */
export function getAssetNameFromPath(path: string): string {
  if (!path) return '';
  const parts = path.split('/');
  const lastPart = parts[parts.length - 1];
  // Remove .uasset extension if present
  return lastPart.replace(/\.uasset$/i, '');
}

/**
 * Get the folder path from a full asset path
 */
export function getAssetFolderFromPath(path: string): string {
  if (!path) return '/Game/';
  const lastSlash = path.lastIndexOf('/');
  return lastSlash > 0 ? path.substring(0, lastSlash) : '/Game/';
}

// ============================================================================
// Progress Tracking
// ============================================================================

export interface ProgressTracker {
  update: (progress: number, step: string) => void;
  complete: () => void;
  fail: (error: string) => void;
}

/**
 * Create a progress tracker for long-running operations
 * Note: This is a placeholder - actual progress reporting would need
 * to be implemented via a streaming mechanism or polling
 */
export function createProgressTracker(
  operationName: string,
  onProgress?: (progress: number, step: string) => void
): ProgressTracker {
  let completed = false;
  let failed = false;

  return {
    update: (progress: number, step: string) => {
      if (completed || failed) return;
      onProgress?.(Math.min(100, Math.max(0, progress)), step);
    },
    complete: () => {
      if (completed || failed) return;
      completed = true;
      onProgress?.(100, `${operationName} completed`);
    },
    fail: (error: string) => {
      if (completed || failed) return;
      failed = true;
      onProgress?.(-1, `${operationName} failed: ${error}`);
    }
  };
}
