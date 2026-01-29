/**
 * FORGE2: Session Management
 * Saves and restores session progress for recovery on context reset
 *
 * Sessions capture:
 * - Current task/workflow in progress
 * - Intermediate results
 * - Pending operations
 * - Work context
 */

import { promises as fs } from 'node:fs';
import { join } from 'node:path';
import { getMcpFolderPath } from './memory.js';

// ============================================================================
// Types
// ============================================================================

export type SessionStatus = 'active' | 'paused' | 'completed' | 'failed' | 'abandoned';

export interface SessionTask {
  id: string;
  name: string;
  status: 'pending' | 'running' | 'completed' | 'failed' | 'skipped';
  startedAt?: string;
  completedAt?: string;
  result?: unknown;
  error?: string;
}

export interface SessionCheckpoint {
  timestamp: string;
  description: string;
  data: Record<string, unknown>;
}

export interface Session {
  id: string;
  name: string;
  status: SessionStatus;
  startedAt: string;
  updatedAt: string;
  completedAt?: string;

  // Current work
  currentTask?: string;
  currentStep?: string;
  progress?: number;

  // Task tracking
  tasks: SessionTask[];
  completedTasks: number;
  totalTasks: number;

  // Data storage
  context: Record<string, unknown>;
  intermediateResults: Record<string, unknown>;
  checkpoints: SessionCheckpoint[];

  // Error tracking
  lastError?: {
    message: string;
    task?: string;
    timestamp: string;
  };

  // Recovery info
  canResume: boolean;
  resumeInstructions?: string;
}

export interface SessionStore {
  version: string;
  currentSessionId?: string;
  sessions: Session[];
  lastUpdated: string;
}

// ============================================================================
// Constants
// ============================================================================

const SESSION_FILE_NAME = 'sessions.json';
const SESSION_VERSION = '1.0.0';
const MAX_SESSIONS = 10;
const MAX_CHECKPOINTS_PER_SESSION = 20;

// ============================================================================
// File Operations
// ============================================================================

/**
 * Get the path to the sessions file
 */
export function getSessionFilePath(projectPath: string): string {
  return join(getMcpFolderPath(projectPath), SESSION_FILE_NAME);
}

/**
 * Load session store from disk
 */
export async function loadSessionStore(projectPath: string): Promise<SessionStore> {
  const sessionPath = getSessionFilePath(projectPath);

  try {
    const content = await fs.readFile(sessionPath, 'utf-8');
    return JSON.parse(content) as SessionStore;
  } catch (error) {
    const err = error as NodeJS.ErrnoException;
    if (err.code === 'ENOENT') {
      return {
        version: SESSION_VERSION,
        sessions: [],
        lastUpdated: new Date().toISOString()
      };
    }
    throw error;
  }
}

/**
 * Save session store to disk
 */
export async function saveSessionStore(projectPath: string, store: SessionStore): Promise<void> {
  const sessionPath = getSessionFilePath(projectPath);
  const mcpPath = getMcpFolderPath(projectPath);

  await fs.mkdir(mcpPath, { recursive: true });

  // Limit number of sessions
  if (store.sessions.length > MAX_SESSIONS) {
    // Keep active session and most recent completed ones
    const active = store.sessions.filter((s) => s.status === 'active' || s.status === 'paused');
    const others = store.sessions
      .filter((s) => s.status !== 'active' && s.status !== 'paused')
      .sort((a, b) => new Date(b.updatedAt).getTime() - new Date(a.updatedAt).getTime())
      .slice(0, MAX_SESSIONS - active.length);
    store.sessions = [...active, ...others];
  }

  store.lastUpdated = new Date().toISOString();
  const content = JSON.stringify(store, null, 2);
  await fs.writeFile(sessionPath, content, 'utf-8');
}

// ============================================================================
// Session Operations
// ============================================================================

/**
 * Generate a unique session ID
 */
function generateSessionId(): string {
  const timestamp = Date.now();
  const random = Math.random().toString(36).substring(2, 8);
  return `session_${timestamp}_${random}`;
}

/**
 * Create a new session
 */
export async function createSession(
  projectPath: string,
  name: string,
  options: {
    tasks?: Array<{ id: string; name: string }>;
    context?: Record<string, unknown>;
  } = {}
): Promise<Session> {
  const store = await loadSessionStore(projectPath);

  // Pause any existing active session
  for (const session of store.sessions) {
    if (session.status === 'active') {
      session.status = 'paused';
      session.updatedAt = new Date().toISOString();
    }
  }

  const tasks: SessionTask[] = (options.tasks ?? []).map((t) => ({
    id: t.id,
    name: t.name,
    status: 'pending'
  }));

  const newSession: Session = {
    id: generateSessionId(),
    name,
    status: 'active',
    startedAt: new Date().toISOString(),
    updatedAt: new Date().toISOString(),
    tasks,
    completedTasks: 0,
    totalTasks: tasks.length,
    context: options.context ?? {},
    intermediateResults: {},
    checkpoints: [],
    canResume: true
  };

  store.sessions.push(newSession);
  store.currentSessionId = newSession.id;
  await saveSessionStore(projectPath, store);

  return newSession;
}

/**
 * Get the current active session
 */
export async function getCurrentSession(projectPath: string): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);

  if (store.currentSessionId) {
    return store.sessions.find((s) => s.id === store.currentSessionId) ?? null;
  }

  // Find any active session
  return store.sessions.find((s) => s.status === 'active') ?? null;
}

/**
 * Get a session by ID
 */
export async function getSession(projectPath: string, sessionId: string): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  return store.sessions.find((s) => s.id === sessionId) ?? null;
}

/**
 * Update session
 */
export async function updateSession(
  projectPath: string,
  sessionId: string,
  updates: Partial<Omit<Session, 'id' | 'startedAt'>>
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session) {
    return null;
  }

  Object.assign(session, updates, { updatedAt: new Date().toISOString() });
  await saveSessionStore(projectPath, store);

  return session;
}

/**
 * Update current task progress
 */
export async function updateProgress(
  projectPath: string,
  sessionId: string,
  currentTask: string,
  currentStep: string,
  progress: number
): Promise<Session | null> {
  return updateSession(projectPath, sessionId, {
    currentTask,
    currentStep,
    progress: Math.min(100, Math.max(0, progress))
  });
}

/**
 * Complete a task in the session
 */
export async function completeTask(
  projectPath: string,
  sessionId: string,
  taskId: string,
  result?: unknown
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session) {
    return null;
  }

  const task = session.tasks.find((t) => t.id === taskId);
  if (task) {
    task.status = 'completed';
    task.completedAt = new Date().toISOString();
    task.result = result;
    session.completedTasks = session.tasks.filter((t) => t.status === 'completed').length;
  }

  session.updatedAt = new Date().toISOString();
  await saveSessionStore(projectPath, store);

  return session;
}

/**
 * Mark a task as failed
 */
export async function failTask(
  projectPath: string,
  sessionId: string,
  taskId: string,
  error: string
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session) {
    return null;
  }

  const task = session.tasks.find((t) => t.id === taskId);
  if (task) {
    task.status = 'failed';
    task.completedAt = new Date().toISOString();
    task.error = error;
  }

  session.lastError = {
    message: error,
    task: taskId,
    timestamp: new Date().toISOString()
  };

  session.updatedAt = new Date().toISOString();
  await saveSessionStore(projectPath, store);

  return session;
}

/**
 * Save a checkpoint
 */
export async function saveCheckpoint(
  projectPath: string,
  sessionId: string,
  description: string,
  data: Record<string, unknown>
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session) {
    return null;
  }

  session.checkpoints.push({
    timestamp: new Date().toISOString(),
    description,
    data
  });

  // Limit checkpoints
  if (session.checkpoints.length > MAX_CHECKPOINTS_PER_SESSION) {
    session.checkpoints = session.checkpoints.slice(-MAX_CHECKPOINTS_PER_SESSION);
  }

  session.updatedAt = new Date().toISOString();
  await saveSessionStore(projectPath, store);

  return session;
}

/**
 * Store intermediate result
 */
export async function storeIntermediateResult(
  projectPath: string,
  sessionId: string,
  key: string,
  value: unknown
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session) {
    return null;
  }

  session.intermediateResults[key] = value;
  session.updatedAt = new Date().toISOString();
  await saveSessionStore(projectPath, store);

  return session;
}

/**
 * Get intermediate result
 */
export async function getIntermediateResult<T = unknown>(
  projectPath: string,
  sessionId: string,
  key: string
): Promise<T | undefined> {
  const session = await getSession(projectPath, sessionId);
  return session?.intermediateResults[key] as T | undefined;
}

/**
 * Complete a session
 */
export async function completeSession(
  projectPath: string,
  sessionId: string
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session) {
    return null;
  }

  session.status = 'completed';
  session.completedAt = new Date().toISOString();
  session.updatedAt = new Date().toISOString();
  session.canResume = false;

  if (store.currentSessionId === sessionId) {
    store.currentSessionId = undefined;
  }

  await saveSessionStore(projectPath, store);
  return session;
}

/**
 * Fail a session
 */
export async function failSession(
  projectPath: string,
  sessionId: string,
  error: string
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session) {
    return null;
  }

  session.status = 'failed';
  session.completedAt = new Date().toISOString();
  session.updatedAt = new Date().toISOString();
  session.lastError = {
    message: error,
    timestamp: new Date().toISOString()
  };

  if (store.currentSessionId === sessionId) {
    store.currentSessionId = undefined;
  }

  await saveSessionStore(projectPath, store);
  return session;
}

/**
 * Resume a paused session
 */
export async function resumeSession(
  projectPath: string,
  sessionId: string
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session || !session.canResume) {
    return null;
  }

  // Pause any currently active session
  for (const s of store.sessions) {
    if (s.status === 'active' && s.id !== sessionId) {
      s.status = 'paused';
      s.updatedAt = new Date().toISOString();
    }
  }

  session.status = 'active';
  session.updatedAt = new Date().toISOString();
  store.currentSessionId = sessionId;

  await saveSessionStore(projectPath, store);
  return session;
}

/**
 * Abandon a session (cannot be resumed)
 */
export async function abandonSession(
  projectPath: string,
  sessionId: string
): Promise<Session | null> {
  const store = await loadSessionStore(projectPath);
  const session = store.sessions.find((s) => s.id === sessionId);

  if (!session) {
    return null;
  }

  session.status = 'abandoned';
  session.completedAt = new Date().toISOString();
  session.updatedAt = new Date().toISOString();
  session.canResume = false;

  if (store.currentSessionId === sessionId) {
    store.currentSessionId = undefined;
  }

  await saveSessionStore(projectPath, store);
  return session;
}

// ============================================================================
// Summary Generation
// ============================================================================

/**
 * Generate session summary for context
 */
export async function getSessionSummary(projectPath: string): Promise<string> {
  const store = await loadSessionStore(projectPath);
  const current = store.currentSessionId
    ? store.sessions.find((s) => s.id === store.currentSessionId)
    : null;

  const lines: string[] = ['## Session'];

  if (!current) {
    lines.push('', 'No active session.');
    return lines.join('\n');
  }

  lines.push('', `**${current.name}** (${current.status})`);
  lines.push(`Progress: ${current.completedTasks}/${current.totalTasks} tasks`);

  if (current.currentTask) {
    lines.push(`Current: ${current.currentTask}`);
    if (current.currentStep) {
      lines.push(`Step: ${current.currentStep}`);
    }
    if (current.progress !== undefined) {
      lines.push(`Progress: ${current.progress}%`);
    }
  }

  if (current.lastError) {
    lines.push('', `⚠️ Last error: ${current.lastError.message}`);
  }

  // Pending tasks
  const pending = current.tasks.filter((t) => t.status === 'pending');
  if (pending.length > 0) {
    lines.push('', '### Pending Tasks');
    for (const task of pending.slice(0, 5)) {
      lines.push(`- ${task.name}`);
    }
    if (pending.length > 5) {
      lines.push(`_...and ${pending.length - 5} more_`);
    }
  }

  // Last checkpoint
  if (current.checkpoints.length > 0) {
    const lastCheckpoint = current.checkpoints[current.checkpoints.length - 1];
    lines.push('', `Last checkpoint: ${lastCheckpoint.description}`);
  }

  return lines.join('\n');
}

/**
 * Get resume instructions for a session
 */
export async function getResumeInstructions(
  projectPath: string,
  sessionId: string
): Promise<string | null> {
  const session = await getSession(projectPath, sessionId);
  if (!session || !session.canResume) {
    return null;
  }

  const lines: string[] = [
    `## Resume Session: ${session.name}`,
    '',
    `Started: ${session.startedAt}`,
    `Progress: ${session.completedTasks}/${session.totalTasks} tasks completed`,
    ''
  ];

  // Current state
  if (session.currentTask) {
    lines.push(`### Current Task: ${session.currentTask}`);
    if (session.currentStep) {
      lines.push(`Step: ${session.currentStep}`);
    }
    lines.push('');
  }

  // Pending tasks
  const pending = session.tasks.filter((t) => t.status === 'pending' || t.status === 'running');
  if (pending.length > 0) {
    lines.push('### Remaining Tasks');
    for (const task of pending) {
      const status = task.status === 'running' ? '🔄' : '⬜';
      lines.push(`${status} ${task.name}`);
    }
    lines.push('');
  }

  // Last checkpoint
  if (session.checkpoints.length > 0) {
    const lastCheckpoint = session.checkpoints[session.checkpoints.length - 1];
    lines.push('### Last Checkpoint');
    lines.push(`${lastCheckpoint.description}`);
    lines.push(`Time: ${lastCheckpoint.timestamp}`);
  }

  // Custom resume instructions
  if (session.resumeInstructions) {
    lines.push('', '### Resume Instructions');
    lines.push(session.resumeInstructions);
  }

  return lines.join('\n');
}
