/**
 * FORGE2: Context Engineering Handlers
 * Handles project state, todo tracking, decision logging, and session management
 * for AI context persistence
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
// Project State Action Handlers
// ============================================================================

/**
 * Get project summary
 */
const handleGetProjectSummary: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'get_project_summary', {
    includeStatistics: validateOptionalBoolean(params.include_statistics || params.includeStatistics) ?? true,
    includeRecentChanges: validateOptionalBoolean(params.include_recent_changes || params.includeRecentChanges) ?? true,
    includeActiveWork: validateOptionalBoolean(params.include_active_work || params.includeActiveWork) ?? true,
    maxTokens: validateOptionalNumber(params.max_tokens || params.maxTokens) ?? 500
  });
};

/**
 * Initialize project state
 */
const handleInitializeProjectState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'initialize_project_state', {
    projectName: validateOptionalString(params.project_name || params.projectName),
    projectType: validateOptionalString(params.project_type || params.projectType) ?? 'game',
    targetPlatforms: validateOptionalArray(params.target_platforms || params.targetPlatforms) ?? ['Windows'],
    genre: validateOptionalString(params.genre),
    description: validateOptionalString(params.description)
  });
};

/**
 * Update project state
 */
const handleUpdateProjectState: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'update_project_state', {
    updates: validateOptionalObject(params.updates) ?? {},
    addToHistory: validateOptionalBoolean(params.add_to_history || params.addToHistory) ?? true
  });
};

// ============================================================================
// Category Index Action Handlers
// ============================================================================

/**
 * Get category index
 */
const handleGetCategoryIndex: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'get_category_index', {
    category: validateOptionalString(params.category),
    includeAssets: validateOptionalBoolean(params.include_assets || params.includeAssets) ?? true,
    includeBlueprints: validateOptionalBoolean(params.include_blueprints || params.includeBlueprints) ?? true,
    sortBy: validateOptionalString(params.sort_by || params.sortBy) ?? 'modified'
  });
};

/**
 * Drill down to path
 */
const handleDrillDown: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const path = validateRequiredString(params.path, 'path');

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'drill_down', {
    path,
    depth: validateOptionalNumber(params.depth) ?? 1,
    includeDetails: validateOptionalBoolean(params.include_details || params.includeDetails) ?? true
  });
};

// ============================================================================
// Todo Management Action Handlers
// ============================================================================

/**
 * Get todo list
 */
const handleGetTodoList: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'get_todo_list', {
    status: validateOptionalString(params.status),
    priority: validateOptionalString(params.priority),
    category: validateOptionalString(params.category),
    includeCompleted: validateOptionalBoolean(params.include_completed || params.includeCompleted) ?? false
  });
};

/**
 * Add todo item
 */
const handleAddTodoItem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const title = validateRequiredString(params.title, 'title');

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'add_todo_item', {
    title,
    description: validateOptionalString(params.description),
    priority: validateOptionalString(params.priority) ?? 'medium',
    category: validateOptionalString(params.category),
    dueDate: validateOptionalString(params.due_date || params.dueDate),
    tags: validateOptionalArray(params.tags) ?? []
  });
};

/**
 * Update todo item
 */
const handleUpdateTodoItem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const todoId = validateRequiredString(params.todo_id || params.todoId, 'todo_id');

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'update_todo_item', {
    todoId,
    title: validateOptionalString(params.title),
    description: validateOptionalString(params.description),
    status: validateOptionalString(params.status),
    priority: validateOptionalString(params.priority)
  });
};

/**
 * Complete todo item
 */
const handleCompleteTodoItem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const todoId = validateRequiredString(params.todo_id || params.todoId, 'todo_id');

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'complete_todo_item', {
    todoId,
    notes: validateOptionalString(params.notes)
  });
};

/**
 * Remove todo item
 */
const handleRemoveTodoItem: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const todoId = validateRequiredString(params.todo_id || params.todoId, 'todo_id');

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'remove_todo_item', {
    todoId
  });
};

// ============================================================================
// Decision Log Action Handlers
// ============================================================================

/**
 * Log decision
 */
const handleLogDecision: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const title = validateRequiredString(params.title, 'title');

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'log_decision', {
    title,
    description: validateOptionalString(params.description),
    rationale: validateOptionalString(params.rationale),
    alternatives: validateOptionalArray(params.alternatives) ?? [],
    category: validateOptionalString(params.category) ?? 'architecture',
    impact: validateOptionalString(params.impact) ?? 'medium',
    tags: validateOptionalArray(params.tags) ?? []
  });
};

/**
 * Get decisions
 */
const handleGetDecisions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'get_decisions', {
    category: validateOptionalString(params.category),
    impact: validateOptionalString(params.impact),
    searchTerm: validateOptionalString(params.search_term || params.searchTerm),
    limit: validateOptionalNumber(params.limit) ?? 20
  });
};

// ============================================================================
// Session Management Action Handlers
// ============================================================================

/**
 * Save session
 */
const handleSaveSession: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'save_session', {
    sessionName: validateOptionalString(params.session_name || params.sessionName),
    notes: validateOptionalString(params.notes),
    includeState: validateOptionalBoolean(params.include_state || params.includeState) ?? true,
    includeTodos: validateOptionalBoolean(params.include_todos || params.includeTodos) ?? true,
    includeDecisions: validateOptionalBoolean(params.include_decisions || params.includeDecisions) ?? true
  });
};

/**
 * Load session
 */
const handleLoadSession: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'load_session', {
    sessionName: validateOptionalString(params.session_name || params.sessionName),
    sessionId: validateOptionalString(params.session_id || params.sessionId),
    mergeWithCurrent: validateOptionalBoolean(params.merge_with_current || params.mergeWithCurrent) ?? false
  });
};

/**
 * List sessions
 */
const handleListSessions: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'list_sessions', {
    limit: validateOptionalNumber(params.limit) ?? 10,
    sortBy: validateOptionalString(params.sort_by || params.sortBy) ?? 'date'
  });
};

// ============================================================================
// Memory/Notes Action Handlers
// ============================================================================

/**
 * Add note
 */
const handleAddNote: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const content = validateRequiredString(params.content, 'content');

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'add_note', {
    content,
    title: validateOptionalString(params.title),
    category: validateOptionalString(params.category) ?? 'general',
    tags: validateOptionalArray(params.tags) ?? [],
    relatedAssets: validateOptionalArray(params.related_assets || params.relatedAssets) ?? []
  });
};

/**
 * Search notes
 */
const handleSearchNotes: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'search_notes', {
    searchTerm: validateOptionalString(params.search_term || params.searchTerm),
    category: validateOptionalString(params.category),
    tags: validateOptionalArray(params.tags),
    limit: validateOptionalNumber(params.limit) ?? 20
  });
};

// ============================================================================
// Template Action Handlers
// ============================================================================

/**
 * List templates
 */
const handleListTemplates: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'list_templates', {
    category: validateOptionalString(params.category),
    includeBuiltIn: validateOptionalBoolean(params.include_built_in || params.includeBuiltIn) ?? true,
    includeCustom: validateOptionalBoolean(params.include_custom || params.includeCustom) ?? true
  });
};

/**
 * Get template
 */
const handleGetTemplate: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const templateName = validateRequiredString(params.template_name || params.templateName, 'template_name');

  return executeForge2Action(tools, Forge2Categories.CONTEXT, 'get_template', {
    templateName,
    includeParameters: validateOptionalBoolean(params.include_parameters || params.includeParameters) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const contextHandlers: ActionHandlerMap = {
  // Project State
  get_project_summary: handleGetProjectSummary,
  initialize_project_state: handleInitializeProjectState,
  update_project_state: handleUpdateProjectState,

  // Category Index
  get_category_index: handleGetCategoryIndex,
  drill_down: handleDrillDown,

  // Todo Management
  get_todo_list: handleGetTodoList,
  add_todo_item: handleAddTodoItem,
  update_todo_item: handleUpdateTodoItem,
  complete_todo_item: handleCompleteTodoItem,
  remove_todo_item: handleRemoveTodoItem,

  // Decision Log
  log_decision: handleLogDecision,
  get_decisions: handleGetDecisions,

  // Session Management
  save_session: handleSaveSession,
  load_session: handleLoadSession,
  list_sessions: handleListSessions,

  // Memory/Notes
  add_note: handleAddNote,
  search_notes: handleSearchNotes,

  // Templates
  list_templates: handleListTemplates,
  get_template: handleGetTemplate
};

export const handleContextTools = createCategoryHandler(Forge2Categories.CONTEXT, contextHandlers);

export default contextHandlers;
