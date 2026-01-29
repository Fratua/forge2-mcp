/**
 * FORGE2: Context Engineering Module
 * File-based persistent memory and session management
 *
 * Based on Anthropic's context engineering patterns and Manus approach:
 * - File system as memory
 * - Todo list recited at end of context
 * - Decision log to prevent repeated discussions
 * - Session state for recovery on context reset
 */

// Memory/Project State
export {
  // Types
  type AssetSummary,
  type ProjectStatistics,
  type RecentChange,
  type ActiveContext,
  type GameSystemState,
  type ProjectState,

  // Path utilities
  getMcpFolderPath,
  getStateFilePath,

  // State operations
  createDefaultState,
  loadProjectState,
  saveProjectState,
  initializeProjectState,

  // Update functions
  recordChange,
  updateStatistics,
  updateGameSystems,
  updateActiveContext,
  setCustomData,
  getCustomData,

  // Summary generation
  getProjectSummary,
  getCategoryIndex
} from './memory.js';

// Todo List
export {
  // Types
  type TodoPriority,
  type TodoStatus,
  type TodoCategory,
  type TodoItem,
  type TodoList,

  // Path utilities
  getTodoFilePath,

  // Operations
  loadTodoList,
  saveTodoList,
  addTodoItem,
  updateTodoItem,
  completeTodoItem,
  removeTodoItem,
  getTodoItem,
  getAllTodos,
  getTodosByStatus,
  getTodosByCategory,
  getTodosByPriority,
  getPendingTodos,
  getBlockedTodos,
  setTodoBlocked,
  clearOldCompletedTodos,

  // Summary
  getTodoSummary,
  exportTodoToMarkdown
} from './todo.js';

// Decision Log
export {
  // Types
  type DecisionCategory,
  type DecisionStatus,
  type DecisionOption,
  type Decision,
  type DecisionLog,

  // Path utilities
  getDecisionsFilePath,

  // Operations
  loadDecisionLog,
  saveDecisionLog,
  logDecision,
  updateDecision,
  supersedeDecision,
  getDecision,
  getAllDecisions,
  getActiveDecisions,
  getDecisionsByCategory,
  searchDecisions,
  getDecisionsForAsset,

  // Summary
  getDecisionsSummary,
  exportDecisionsToMarkdown
} from './decisions.js';

// Session Management
export {
  // Types
  type SessionStatus,
  type SessionTask,
  type SessionCheckpoint,
  type Session,
  type SessionStore,

  // Path utilities
  getSessionFilePath,

  // Operations
  loadSessionStore,
  saveSessionStore,
  createSession,
  getCurrentSession,
  getSession,
  updateSession,
  updateProgress,
  completeTask,
  failTask,
  saveCheckpoint,
  storeIntermediateResult,
  getIntermediateResult,
  completeSession,
  failSession,
  resumeSession,
  abandonSession,

  // Summary
  getSessionSummary,
  getResumeInstructions
} from './session.js';

// ============================================================================
// Combined Context Summary
// ============================================================================

import { getProjectSummary } from './memory.js';
import { getTodoSummary } from './todo.js';
import { getDecisionsSummary } from './decisions.js';
import { getSessionSummary } from './session.js';

/**
 * Generate a complete context summary (for end of response)
 * Approximately 1000-1500 tokens total
 */
export async function getFullContextSummary(projectPath: string): Promise<string> {
  const [project, todos, decisions, session] = await Promise.all([
    getProjectSummary(projectPath),
    getTodoSummary(projectPath),
    getDecisionsSummary(projectPath),
    getSessionSummary(projectPath)
  ]);

  return [
    '---',
    '# Context Summary',
    '',
    project,
    '',
    session,
    '',
    todos,
    '',
    decisions,
    '---'
  ].join('\n');
}

/**
 * Generate a minimal context reminder (for token efficiency)
 * Approximately 200-300 tokens
 */
export async function getMinimalContextReminder(projectPath: string): Promise<string> {
  const [todos, session] = await Promise.all([
    getTodoSummary(projectPath),
    getSessionSummary(projectPath)
  ]);

  return [
    '---',
    '## Current Context',
    '',
    session,
    '',
    todos,
    '---'
  ].join('\n');
}
