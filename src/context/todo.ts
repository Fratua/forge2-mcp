/**
 * FORGE2: Todo List Management
 * File-based todo list that persists across sessions
 *
 * Following Manus pattern: todo list is recited at end of context
 * to maintain focus on current tasks.
 */

import { promises as fs } from 'node:fs';
import { join } from 'node:path';
import { getMcpFolderPath } from './memory.js';

// ============================================================================
// Types
// ============================================================================

export type TodoPriority = 'low' | 'medium' | 'high' | 'critical';
export type TodoStatus = 'pending' | 'in_progress' | 'completed' | 'cancelled' | 'blocked';
export type TodoCategory =
  | 'worldgen'
  | 'quest'
  | 'dialogue'
  | 'combat'
  | 'ui'
  | 'audio'
  | 'vfx'
  | 'blueprint'
  | 'asset'
  | 'bug'
  | 'feature'
  | 'optimization'
  | 'other';

export interface TodoItem {
  id: string;
  title: string;
  description?: string;
  status: TodoStatus;
  priority: TodoPriority;
  category: TodoCategory;
  createdAt: string;
  updatedAt: string;
  completedAt?: string;
  dueDate?: string;
  tags?: string[];
  blockedBy?: string[];
  relatedAssets?: string[];
  notes?: string;
}

export interface TodoList {
  version: string;
  items: TodoItem[];
  lastUpdated: string;
}

// ============================================================================
// Constants
// ============================================================================

const TODO_FILE_NAME = 'todo.json';
const TODO_VERSION = '1.0.0';

// ============================================================================
// File Operations
// ============================================================================

/**
 * Get the path to the todo file
 */
export function getTodoFilePath(projectPath: string): string {
  return join(getMcpFolderPath(projectPath), TODO_FILE_NAME);
}

/**
 * Load todo list from disk
 */
export async function loadTodoList(projectPath: string): Promise<TodoList> {
  const todoPath = getTodoFilePath(projectPath);

  try {
    const content = await fs.readFile(todoPath, 'utf-8');
    return JSON.parse(content) as TodoList;
  } catch (error) {
    const err = error as NodeJS.ErrnoException;
    if (err.code === 'ENOENT') {
      return {
        version: TODO_VERSION,
        items: [],
        lastUpdated: new Date().toISOString()
      };
    }
    throw error;
  }
}

/**
 * Save todo list to disk
 */
export async function saveTodoList(projectPath: string, todoList: TodoList): Promise<void> {
  const todoPath = getTodoFilePath(projectPath);
  const mcpPath = getMcpFolderPath(projectPath);

  // Ensure .mcp folder exists
  await fs.mkdir(mcpPath, { recursive: true });

  todoList.lastUpdated = new Date().toISOString();
  const content = JSON.stringify(todoList, null, 2);
  await fs.writeFile(todoPath, content, 'utf-8');
}

// ============================================================================
// Todo Operations
// ============================================================================

/**
 * Generate a unique todo ID
 */
function generateTodoId(): string {
  return `todo_${Date.now()}_${Math.random().toString(36).substring(2, 8)}`;
}

/**
 * Add a new todo item
 */
export async function addTodoItem(
  projectPath: string,
  title: string,
  options: {
    description?: string;
    priority?: TodoPriority;
    category?: TodoCategory;
    dueDate?: string;
    tags?: string[];
    relatedAssets?: string[];
  } = {}
): Promise<TodoItem> {
  const todoList = await loadTodoList(projectPath);

  const newItem: TodoItem = {
    id: generateTodoId(),
    title,
    description: options.description,
    status: 'pending',
    priority: options.priority ?? 'medium',
    category: options.category ?? 'other',
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString(),
    dueDate: options.dueDate,
    tags: options.tags,
    relatedAssets: options.relatedAssets
  };

  todoList.items.push(newItem);
  await saveTodoList(projectPath, todoList);

  return newItem;
}

/**
 * Update a todo item
 */
export async function updateTodoItem(
  projectPath: string,
  itemId: string,
  updates: Partial<Omit<TodoItem, 'id' | 'createdAt'>>
): Promise<TodoItem | null> {
  const todoList = await loadTodoList(projectPath);
  const item = todoList.items.find((i) => i.id === itemId);

  if (!item) {
    return null;
  }

  Object.assign(item, updates, { updatedAt: new Date().toISOString() });

  // Set completedAt when status changes to completed
  if (updates.status === 'completed' && !item.completedAt) {
    item.completedAt = new Date().toISOString();
  }

  await saveTodoList(projectPath, todoList);
  return item;
}

/**
 * Complete a todo item
 */
export async function completeTodoItem(
  projectPath: string,
  itemId: string,
  notes?: string
): Promise<TodoItem | null> {
  return updateTodoItem(projectPath, itemId, {
    status: 'completed',
    notes: notes
  });
}

/**
 * Remove a todo item
 */
export async function removeTodoItem(
  projectPath: string,
  itemId: string
): Promise<boolean> {
  const todoList = await loadTodoList(projectPath);
  const initialLength = todoList.items.length;
  todoList.items = todoList.items.filter((i) => i.id !== itemId);

  if (todoList.items.length < initialLength) {
    await saveTodoList(projectPath, todoList);
    return true;
  }

  return false;
}

/**
 * Get a todo item by ID
 */
export async function getTodoItem(
  projectPath: string,
  itemId: string
): Promise<TodoItem | null> {
  const todoList = await loadTodoList(projectPath);
  return todoList.items.find((i) => i.id === itemId) ?? null;
}

/**
 * Get all todo items
 */
export async function getAllTodos(projectPath: string): Promise<TodoItem[]> {
  const todoList = await loadTodoList(projectPath);
  return todoList.items;
}

/**
 * Get todo items by status
 */
export async function getTodosByStatus(
  projectPath: string,
  status: TodoStatus
): Promise<TodoItem[]> {
  const todoList = await loadTodoList(projectPath);
  return todoList.items.filter((i) => i.status === status);
}

/**
 * Get todo items by category
 */
export async function getTodosByCategory(
  projectPath: string,
  category: TodoCategory
): Promise<TodoItem[]> {
  const todoList = await loadTodoList(projectPath);
  return todoList.items.filter((i) => i.category === category);
}

/**
 * Get todo items by priority
 */
export async function getTodosByPriority(
  projectPath: string,
  priority: TodoPriority
): Promise<TodoItem[]> {
  const todoList = await loadTodoList(projectPath);
  return todoList.items.filter((i) => i.priority === priority);
}

/**
 * Get pending todos (not completed or cancelled)
 */
export async function getPendingTodos(projectPath: string): Promise<TodoItem[]> {
  const todoList = await loadTodoList(projectPath);
  return todoList.items.filter(
    (i) => i.status !== 'completed' && i.status !== 'cancelled'
  );
}

/**
 * Get blocked todos
 */
export async function getBlockedTodos(projectPath: string): Promise<TodoItem[]> {
  const todoList = await loadTodoList(projectPath);
  return todoList.items.filter((i) => i.status === 'blocked');
}

/**
 * Set todo as blocked by another todo
 */
export async function setTodoBlocked(
  projectPath: string,
  itemId: string,
  blockedByIds: string[]
): Promise<TodoItem | null> {
  return updateTodoItem(projectPath, itemId, {
    status: 'blocked',
    blockedBy: blockedByIds
  });
}

/**
 * Clear completed todos older than specified days
 */
export async function clearOldCompletedTodos(
  projectPath: string,
  olderThanDays: number = 30
): Promise<number> {
  const todoList = await loadTodoList(projectPath);
  const cutoffDate = new Date();
  cutoffDate.setDate(cutoffDate.getDate() - olderThanDays);
  const cutoffString = cutoffDate.toISOString();

  const initialLength = todoList.items.length;
  todoList.items = todoList.items.filter((i) => {
    if (i.status !== 'completed') return true;
    if (!i.completedAt) return true;
    return i.completedAt > cutoffString;
  });

  const removed = initialLength - todoList.items.length;
  if (removed > 0) {
    await saveTodoList(projectPath, todoList);
  }

  return removed;
}

// ============================================================================
// Summary Generation
// ============================================================================

/**
 * Generate a markdown todo list summary
 * Suitable for inclusion at end of context to maintain focus
 */
export async function getTodoSummary(projectPath: string): Promise<string> {
  const todoList = await loadTodoList(projectPath);
  const pending = todoList.items.filter(
    (i) => i.status !== 'completed' && i.status !== 'cancelled'
  );

  if (pending.length === 0) {
    return '## Todo List\n\nNo pending tasks.';
  }

  // Sort by priority
  const priorityOrder: Record<TodoPriority, number> = {
    critical: 0,
    high: 1,
    medium: 2,
    low: 3
  };
  pending.sort((a, b) => priorityOrder[a.priority] - priorityOrder[b.priority]);

  const lines: string[] = ['## Todo List', ''];

  // Group by priority
  const critical = pending.filter((i) => i.priority === 'critical');
  const high = pending.filter((i) => i.priority === 'high');
  const medium = pending.filter((i) => i.priority === 'medium');
  const low = pending.filter((i) => i.priority === 'low');

  const addItems = (items: TodoItem[], label: string, emoji: string) => {
    if (items.length === 0) return;
    lines.push(`### ${emoji} ${label}`);
    for (const item of items) {
      const statusIcon =
        item.status === 'in_progress' ? '🔄' : item.status === 'blocked' ? '🚫' : '⬜';
      lines.push(`- ${statusIcon} ${item.title}`);
      if (item.description) {
        lines.push(`  _${item.description}_`);
      }
    }
    lines.push('');
  };

  addItems(critical, 'Critical', '🔴');
  addItems(high, 'High Priority', '🟠');
  addItems(medium, 'Medium Priority', '🟡');
  addItems(low, 'Low Priority', '🟢');

  // Summary stats
  const inProgress = pending.filter((i) => i.status === 'in_progress').length;
  const blocked = pending.filter((i) => i.status === 'blocked').length;
  lines.push(`_${pending.length} total | ${inProgress} in progress | ${blocked} blocked_`);

  return lines.join('\n');
}

/**
 * Export todo list to markdown file
 */
export async function exportTodoToMarkdown(projectPath: string): Promise<string> {
  const summary = await getTodoSummary(projectPath);
  const mdPath = join(getMcpFolderPath(projectPath), 'todo.md');
  await fs.writeFile(mdPath, summary, 'utf-8');
  return mdPath;
}
