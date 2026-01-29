/**
 * FORGE2: Handler Utilities
 * Barrel export for all FORGE2 handler utility modules
 */

// Validation utilities
export {
  // Enum and range validators
  validateEnum,
  validateRange,
  validatePositiveInteger,
  validateNonNegative,
  validateSeed,
  validateId,
  validateAssetPath,
  validateOptionalAssetPath,

  // Array validators
  validateArrayMinLength,
  validateNumberArray,
  validateStringArray,

  // Object validators
  validateLocation,
  validateOptionalLocation,
  validateBounds,
  type Location3D,
  type Bounds3D,

  // Domain-specific validators
  NOISE_TYPES,
  validateNoiseType,
  type NoiseType,

  DUNGEON_ALGORITHMS,
  validateDungeonAlgorithm,
  type DungeonAlgorithm,

  BUILDING_STYLES,
  validateBuildingStyle,
  type BuildingStyle,

  ITEM_RARITIES,
  validateItemRarity,
  type ItemRarity,

  // Composite validators
  validateNoiseParams,
  validateDungeonParams,
  type NoiseParams,
  type DungeonParams
} from './validation.js';

// Response utilities
export {
  // Response types
  type Forge2SuccessResponse,
  type Forge2ErrorResponse,
  type Forge2ProgressResponse,
  type Forge2AssetResponse,
  type Forge2ActorResponse,
  type Forge2Response,
  type ListResponse,

  // Response builders
  createSuccessResponse,
  createErrorResponse,
  createProgressResponse,
  createAssetResponse,
  createActorResponse,
  createListResponse,
  createItemResponse,
  createNotFoundResponse,
  createNotImplementedResponse,
  createBridgeNotConnectedResponse,

  // Error codes
  ErrorCodes,
  type ErrorCode,

  // Helper functions
  withErrorHandling,
  isErrorResponse,
  isSuccessResponse
} from './response.js';

// Bridge utilities
export {
  // Types
  type Forge2BridgeRequest,
  type Forge2BridgeResponse,
  type BridgeRequestOptions,
  type BatchOperation,
  type BatchResult,
  type ActionHandler,
  type ActionHandlerMap,
  type ProgressTracker,

  // Categories
  Forge2Categories,
  type Forge2Category,

  // Connection functions
  isBridgeConnected,
  getBridgeStatus,

  // Request execution
  sendForge2Request,
  executeForge2Action,
  executeBatch,

  // Handler creation
  createCategoryHandler,
  createPassthroughHandler,

  // Asset path utilities
  normalizeAssetPath,
  getAssetNameFromPath,
  getAssetFolderFromPath,

  // Progress tracking
  createProgressTracker
} from './forge2-bridge.js';
