/**
 * FORGE2: Response Formatting Utilities
 * Standardized response creation for FORGE2 tool handlers
 */

// ============================================================================
// Response Types
// ============================================================================

export interface Forge2SuccessResponse {
  success: true;
  message: string;
  data?: Record<string, unknown>;
}

export interface Forge2ErrorResponse {
  success: false;
  error: string;
  errorCode?: string;
  details?: Record<string, unknown>;
}

export interface Forge2ProgressResponse {
  success: true;
  message: string;
  progress: number; // 0-100
  step: string;
  data?: Record<string, unknown>;
}

export interface Forge2AssetResponse extends Forge2SuccessResponse {
  data: {
    assetPath: string;
    assetName: string;
    assetType: string;
    [key: string]: unknown;
  };
}

export interface Forge2ActorResponse extends Forge2SuccessResponse {
  data: {
    actorPath: string;
    actorName: string;
    actorClass: string;
    [key: string]: unknown;
  };
}

export type Forge2Response = Forge2SuccessResponse | Forge2ErrorResponse;

// ============================================================================
// Response Builders
// ============================================================================

/**
 * Create a success response
 */
export function createSuccessResponse(
  message: string,
  data?: Record<string, unknown>
): Forge2SuccessResponse {
  const response: Forge2SuccessResponse = {
    success: true,
    message
  };
  if (data) {
    response.data = data;
  }
  return response;
}

/**
 * Create an error response
 */
export function createErrorResponse(
  error: string,
  errorCode?: string,
  details?: Record<string, unknown>
): Forge2ErrorResponse {
  const response: Forge2ErrorResponse = {
    success: false,
    error
  };
  if (errorCode) {
    response.errorCode = errorCode;
  }
  if (details) {
    response.details = details;
  }
  return response;
}

/**
 * Create a progress response for long-running operations
 */
export function createProgressResponse(
  message: string,
  progress: number,
  step: string,
  data?: Record<string, unknown>
): Forge2ProgressResponse {
  return {
    success: true,
    message,
    progress: Math.min(100, Math.max(0, progress)),
    step,
    data
  };
}

/**
 * Create an asset creation response
 */
export function createAssetResponse(
  assetPath: string,
  assetName: string,
  assetType: string,
  additionalData?: Record<string, unknown>
): Forge2AssetResponse {
  return {
    success: true,
    message: `${assetType} '${assetName}' created successfully`,
    data: {
      assetPath,
      assetName,
      assetType,
      ...additionalData
    }
  };
}

/**
 * Create an actor spawn response
 */
export function createActorResponse(
  actorPath: string,
  actorName: string,
  actorClass: string,
  additionalData?: Record<string, unknown>
): Forge2ActorResponse {
  return {
    success: true,
    message: `Actor '${actorName}' spawned successfully`,
    data: {
      actorPath,
      actorName,
      actorClass,
      ...additionalData
    }
  };
}

// ============================================================================
// Error Codes for FORGE2
// ============================================================================

export const ErrorCodes = {
  // General errors
  UNKNOWN_ERROR: 'UNKNOWN_ERROR',
  INVALID_PARAMS: 'INVALID_PARAMS',
  MISSING_REQUIRED: 'MISSING_REQUIRED',
  INVALID_ACTION: 'INVALID_ACTION',

  // Connection errors
  BRIDGE_NOT_CONNECTED: 'BRIDGE_NOT_CONNECTED',
  BRIDGE_TIMEOUT: 'BRIDGE_TIMEOUT',
  BRIDGE_ERROR: 'BRIDGE_ERROR',

  // Asset errors
  ASSET_NOT_FOUND: 'ASSET_NOT_FOUND',
  ASSET_CREATE_FAILED: 'ASSET_CREATE_FAILED',
  ASSET_INVALID_PATH: 'ASSET_INVALID_PATH',
  ASSET_ALREADY_EXISTS: 'ASSET_ALREADY_EXISTS',

  // Actor errors
  ACTOR_NOT_FOUND: 'ACTOR_NOT_FOUND',
  ACTOR_SPAWN_FAILED: 'ACTOR_SPAWN_FAILED',
  ACTOR_INVALID_CLASS: 'ACTOR_INVALID_CLASS',

  // World generation errors
  WORLDGEN_FAILED: 'WORLDGEN_FAILED',
  WORLDGEN_INVALID_PARAMS: 'WORLDGEN_INVALID_PARAMS',
  WORLDGEN_OUT_OF_BOUNDS: 'WORLDGEN_OUT_OF_BOUNDS',

  // Game system errors
  QUEST_NOT_FOUND: 'QUEST_NOT_FOUND',
  DIALOGUE_NOT_FOUND: 'DIALOGUE_NOT_FOUND',
  SKILL_NOT_FOUND: 'SKILL_NOT_FOUND',
  ITEM_NOT_FOUND: 'ITEM_NOT_FOUND',

  // PCG errors
  PCG_GRAPH_NOT_FOUND: 'PCG_GRAPH_NOT_FOUND',
  PCG_NODE_INVALID: 'PCG_NODE_INVALID',
  PCG_EXECUTION_FAILED: 'PCG_EXECUTION_FAILED'
} as const;

export type ErrorCode = typeof ErrorCodes[keyof typeof ErrorCodes];

// ============================================================================
// Response Helpers
// ============================================================================

/**
 * Wrap a handler function with error handling
 */
export async function withErrorHandling<T>(
  fn: () => Promise<T>,
  context: string
): Promise<T | Forge2ErrorResponse> {
  try {
    return await fn();
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error);
    return createErrorResponse(
      `${context}: ${message}`,
      ErrorCodes.UNKNOWN_ERROR,
      { originalError: message }
    );
  }
}

/**
 * Check if a response is an error
 */
export function isErrorResponse(response: unknown): response is Forge2ErrorResponse {
  return (
    typeof response === 'object' &&
    response !== null &&
    'success' in response &&
    (response as Forge2Response).success === false
  );
}

/**
 * Check if a response is a success
 */
export function isSuccessResponse(response: unknown): response is Forge2SuccessResponse {
  return (
    typeof response === 'object' &&
    response !== null &&
    'success' in response &&
    (response as Forge2Response).success === true
  );
}

/**
 * Create a "not implemented" response for placeholder actions
 */
export function createNotImplementedResponse(action: string): Forge2ErrorResponse {
  return createErrorResponse(
    `Action '${action}' is not yet implemented`,
    'NOT_IMPLEMENTED',
    { action }
  );
}

/**
 * Create a response for when Unreal connection is required but not available
 */
export function createBridgeNotConnectedResponse(): Forge2ErrorResponse {
  return createErrorResponse(
    'Automation bridge is not connected to Unreal Engine. Please ensure the editor is running with the McpAutomationBridge plugin enabled.',
    ErrorCodes.BRIDGE_NOT_CONNECTED
  );
}

// ============================================================================
// List/Query Response Helpers
// ============================================================================

export interface ListResponse<T> extends Forge2SuccessResponse {
  data: {
    items: T[];
    count: number;
    totalCount?: number;
    page?: number;
    pageSize?: number;
  };
}

/**
 * Create a paginated list response
 */
export function createListResponse<T>(
  items: T[],
  options?: {
    totalCount?: number;
    page?: number;
    pageSize?: number;
  }
): ListResponse<T> {
  return {
    success: true,
    message: `Found ${items.length} item(s)`,
    data: {
      items,
      count: items.length,
      ...options
    }
  };
}

/**
 * Create a response for a single item query
 */
export function createItemResponse<T>(
  item: T,
  itemType: string
): Forge2SuccessResponse {
  return {
    success: true,
    message: `${itemType} found`,
    data: { item }
  };
}

/**
 * Create a response for item not found
 */
export function createNotFoundResponse(
  itemType: string,
  identifier: string
): Forge2ErrorResponse {
  return createErrorResponse(
    `${itemType} '${identifier}' not found`,
    ErrorCodes.ASSET_NOT_FOUND,
    { itemType, identifier }
  );
}
