/**
 * FORGE2: Project State Memory System
 * File-based persistent memory for project state across sessions
 *
 * Based on Anthropic's context engineering patterns:
 * - Uses file system as memory (like Manus approach)
 * - Stores high-level project statistics
 * - Tracks recent changes and active work context
 */

import { promises as fs } from 'node:fs';
import { join } from 'node:path';

// ============================================================================
// Types
// ============================================================================

export interface AssetSummary {
  path: string;
  type: string;
  createdAt: string;
  modifiedAt: string;
}

export interface ProjectStatistics {
  totalAssets: number;
  assetsByType: Record<string, number>;
  totalActors: number;
  totalBlueprints: number;
  totalMaterials: number;
  totalLevels: number;
}

export interface RecentChange {
  timestamp: string;
  action: string;
  target: string;
  details?: string;
}

export interface ActiveContext {
  currentLevel?: string;
  selectedActors?: string[];
  activeBlueprint?: string;
  lastAction?: string;
  workingDirectory?: string;
}

export interface GameSystemState {
  quests?: {
    total: number;
    active: number;
    completed: number;
  };
  dialogues?: {
    total: number;
    trees: number;
  };
  skills?: {
    total: number;
    trees: number;
  };
  items?: {
    total: number;
    byRarity: Record<string, number>;
  };
  npcs?: {
    total: number;
    withDialogue: number;
  };
}

export interface ProjectState {
  version: string;
  projectName: string;
  projectPath: string;
  lastUpdated: string;
  statistics: ProjectStatistics;
  gameSystems: GameSystemState;
  recentChanges: RecentChange[];
  activeContext: ActiveContext;
  customData: Record<string, unknown>;
}

// ============================================================================
// Constants
// ============================================================================

const STATE_FILE_NAME = 'project-state.json';
const MCP_FOLDER = '.mcp';
const MAX_RECENT_CHANGES = 50;
const STATE_VERSION = '1.0.0';

// ============================================================================
// File Operations
// ============================================================================

/**
 * Get the path to the .mcp folder for a project
 */
export function getMcpFolderPath(projectPath: string): string {
  return join(projectPath, MCP_FOLDER);
}

/**
 * Get the path to the project state file
 */
export function getStateFilePath(projectPath: string): string {
  return join(getMcpFolderPath(projectPath), STATE_FILE_NAME);
}

/**
 * Ensure the .mcp folder exists
 */
async function ensureMcpFolder(projectPath: string): Promise<void> {
  const mcpPath = getMcpFolderPath(projectPath);
  try {
    await fs.mkdir(mcpPath, { recursive: true });
  } catch (error) {
    // Folder might already exist, that's fine
    const err = error as NodeJS.ErrnoException;
    if (err.code !== 'EEXIST') {
      throw error;
    }
  }
}

// ============================================================================
// State Management
// ============================================================================

/**
 * Create a default empty project state
 */
export function createDefaultState(projectName: string, projectPath: string): ProjectState {
  return {
    version: STATE_VERSION,
    projectName,
    projectPath,
    lastUpdated: new Date().toISOString(),
    statistics: {
      totalAssets: 0,
      assetsByType: {},
      totalActors: 0,
      totalBlueprints: 0,
      totalMaterials: 0,
      totalLevels: 0
    },
    gameSystems: {},
    recentChanges: [],
    activeContext: {},
    customData: {}
  };
}

/**
 * Load project state from disk
 * Returns default state if file doesn't exist
 */
export async function loadProjectState(projectPath: string): Promise<ProjectState> {
  const statePath = getStateFilePath(projectPath);

  try {
    const content = await fs.readFile(statePath, 'utf-8');
    const state = JSON.parse(content) as ProjectState;

    // Validate version and migrate if needed
    if (state.version !== STATE_VERSION) {
      return migrateState(state, projectPath);
    }

    return state;
  } catch (error) {
    const err = error as NodeJS.ErrnoException;
    if (err.code === 'ENOENT') {
      // File doesn't exist, create default state
      const projectName = projectPath.split(/[/\\]/).pop() ?? 'Unknown Project';
      return createDefaultState(projectName, projectPath);
    }
    throw error;
  }
}

/**
 * Save project state to disk
 */
export async function saveProjectState(projectPath: string, state: ProjectState): Promise<void> {
  await ensureMcpFolder(projectPath);
  const statePath = getStateFilePath(projectPath);

  state.lastUpdated = new Date().toISOString();

  // Trim recent changes to max limit
  if (state.recentChanges.length > MAX_RECENT_CHANGES) {
    state.recentChanges = state.recentChanges.slice(-MAX_RECENT_CHANGES);
  }

  const content = JSON.stringify(state, null, 2);
  await fs.writeFile(statePath, content, 'utf-8');
}

/**
 * Initialize project state (creates file if it doesn't exist)
 */
export async function initializeProjectState(
  projectPath: string,
  projectName?: string
): Promise<ProjectState> {
  await ensureMcpFolder(projectPath);

  try {
    // Try to load existing state
    const existing = await loadProjectState(projectPath);
    return existing;
  } catch {
    // Create new state
    const name = projectName ?? projectPath.split(/[/\\]/).pop() ?? 'Unknown Project';
    const state = createDefaultState(name, projectPath);
    await saveProjectState(projectPath, state);
    return state;
  }
}

/**
 * Migrate state from older versions
 */
function migrateState(oldState: ProjectState, projectPath: string): ProjectState {
  // Currently just update version, add missing fields
  const projectName = projectPath.split(/[/\\]/).pop() ?? 'Unknown Project';
  const defaultState = createDefaultState(projectName, projectPath);

  return {
    ...defaultState,
    ...oldState,
    version: STATE_VERSION,
    lastUpdated: new Date().toISOString(),
    // Ensure all required fields exist
    statistics: { ...defaultState.statistics, ...oldState.statistics },
    gameSystems: { ...defaultState.gameSystems, ...oldState.gameSystems },
    activeContext: { ...defaultState.activeContext, ...oldState.activeContext }
  };
}

// ============================================================================
// State Update Functions
// ============================================================================

/**
 * Record a change in the project
 */
export async function recordChange(
  projectPath: string,
  action: string,
  target: string,
  details?: string
): Promise<void> {
  const state = await loadProjectState(projectPath);

  state.recentChanges.push({
    timestamp: new Date().toISOString(),
    action,
    target,
    details
  });

  await saveProjectState(projectPath, state);
}

/**
 * Update project statistics
 */
export async function updateStatistics(
  projectPath: string,
  updates: Partial<ProjectStatistics>
): Promise<void> {
  const state = await loadProjectState(projectPath);
  state.statistics = { ...state.statistics, ...updates };
  await saveProjectState(projectPath, state);
}

/**
 * Update game system state
 */
export async function updateGameSystems(
  projectPath: string,
  updates: Partial<GameSystemState>
): Promise<void> {
  const state = await loadProjectState(projectPath);
  state.gameSystems = { ...state.gameSystems, ...updates };
  await saveProjectState(projectPath, state);
}

/**
 * Update active context
 */
export async function updateActiveContext(
  projectPath: string,
  updates: Partial<ActiveContext>
): Promise<void> {
  const state = await loadProjectState(projectPath);
  state.activeContext = { ...state.activeContext, ...updates };
  await saveProjectState(projectPath, state);
}

/**
 * Set custom data field
 */
export async function setCustomData(
  projectPath: string,
  key: string,
  value: unknown
): Promise<void> {
  const state = await loadProjectState(projectPath);
  state.customData[key] = value;
  await saveProjectState(projectPath, state);
}

/**
 * Get custom data field
 */
export async function getCustomData<T = unknown>(
  projectPath: string,
  key: string
): Promise<T | undefined> {
  const state = await loadProjectState(projectPath);
  return state.customData[key] as T | undefined;
}

// ============================================================================
// Summary Generation
// ============================================================================

/**
 * Generate a compact project summary (optimized for context window)
 * Approximately 500 tokens
 */
export async function getProjectSummary(projectPath: string): Promise<string> {
  const state = await loadProjectState(projectPath);

  const lines: string[] = [
    `# ${state.projectName}`,
    `Last updated: ${state.lastUpdated}`,
    '',
    '## Statistics',
    `- Assets: ${state.statistics.totalAssets}`,
    `- Actors: ${state.statistics.totalActors}`,
    `- Blueprints: ${state.statistics.totalBlueprints}`,
    `- Materials: ${state.statistics.totalMaterials}`,
    `- Levels: ${state.statistics.totalLevels}`
  ];

  // Asset type breakdown
  const assetTypes = Object.entries(state.statistics.assetsByType);
  if (assetTypes.length > 0) {
    lines.push('', '### Asset Types');
    for (const [type, count] of assetTypes.slice(0, 10)) {
      lines.push(`- ${type}: ${count}`);
    }
  }

  // Game systems
  if (Object.keys(state.gameSystems).length > 0) {
    lines.push('', '## Game Systems');
    if (state.gameSystems.quests) {
      lines.push(`- Quests: ${state.gameSystems.quests.total} (${state.gameSystems.quests.active} active)`);
    }
    if (state.gameSystems.dialogues) {
      lines.push(`- Dialogues: ${state.gameSystems.dialogues.total} nodes in ${state.gameSystems.dialogues.trees} trees`);
    }
    if (state.gameSystems.skills) {
      lines.push(`- Skills: ${state.gameSystems.skills.total} in ${state.gameSystems.skills.trees} trees`);
    }
    if (state.gameSystems.items) {
      lines.push(`- Items: ${state.gameSystems.items.total}`);
    }
    if (state.gameSystems.npcs) {
      lines.push(`- NPCs: ${state.gameSystems.npcs.total} (${state.gameSystems.npcs.withDialogue} with dialogue)`);
    }
  }

  // Recent changes (last 5)
  if (state.recentChanges.length > 0) {
    lines.push('', '## Recent Changes');
    const recent = state.recentChanges.slice(-5);
    for (const change of recent) {
      lines.push(`- ${change.action}: ${change.target}`);
    }
  }

  // Active context
  if (state.activeContext.currentLevel || state.activeContext.activeBlueprint) {
    lines.push('', '## Active Context');
    if (state.activeContext.currentLevel) {
      lines.push(`- Level: ${state.activeContext.currentLevel}`);
    }
    if (state.activeContext.activeBlueprint) {
      lines.push(`- Blueprint: ${state.activeContext.activeBlueprint}`);
    }
    if (state.activeContext.lastAction) {
      lines.push(`- Last action: ${state.activeContext.lastAction}`);
    }
  }

  return lines.join('\n');
}

/**
 * Get a category index of assets
 */
export async function getCategoryIndex(
  projectPath: string,
  category?: string
): Promise<Record<string, string[]>> {
  const state = await loadProjectState(projectPath);

  // This would normally query the actual assets
  // For now, return what we have in custom data
  const assetIndex = (state.customData.assetIndex as Record<string, string[]>) ?? {};

  if (category) {
    return { [category]: assetIndex[category] ?? [] };
  }

  return assetIndex;
}
