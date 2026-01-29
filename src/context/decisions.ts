/**
 * FORGE2: Decision Log
 * Records architectural and design decisions to prevent repeated discussions
 *
 * Following Anthropic's context engineering patterns:
 * - Persist "why" behind choices
 * - Prevent context loss on architectural decisions
 * - Enable resuming work with full decision history
 */

import { promises as fs } from 'node:fs';
import { join } from 'node:path';
import { getMcpFolderPath } from './memory.js';

// ============================================================================
// Types
// ============================================================================

export type DecisionCategory =
  | 'architecture'
  | 'design'
  | 'implementation'
  | 'performance'
  | 'ux'
  | 'compatibility'
  | 'security'
  | 'technical_debt'
  | 'scope'
  | 'tooling'
  | 'other';

export type DecisionStatus = 'active' | 'superseded' | 'deprecated' | 'proposed';

export interface DecisionOption {
  name: string;
  description: string;
  pros?: string[];
  cons?: string[];
  rejected?: boolean;
  rejectionReason?: string;
}

export interface Decision {
  id: string;
  title: string;
  category: DecisionCategory;
  status: DecisionStatus;
  date: string;
  context: string;
  decision: string;
  rationale: string;
  options?: DecisionOption[];
  consequences?: string[];
  alternatives?: string[];
  relatedDecisions?: string[];
  relatedAssets?: string[];
  tags?: string[];
  supersededBy?: string;
  notes?: string;
}

export interface DecisionLog {
  version: string;
  decisions: Decision[];
  lastUpdated: string;
}

// ============================================================================
// Constants
// ============================================================================

const DECISIONS_FILE_NAME = 'decisions.json';
const DECISIONS_MD_NAME = 'decisions.md';
const DECISIONS_VERSION = '1.0.0';

// ============================================================================
// File Operations
// ============================================================================

/**
 * Get the path to the decisions file
 */
export function getDecisionsFilePath(projectPath: string): string {
  return join(getMcpFolderPath(projectPath), DECISIONS_FILE_NAME);
}

/**
 * Load decision log from disk
 */
export async function loadDecisionLog(projectPath: string): Promise<DecisionLog> {
  const decPath = getDecisionsFilePath(projectPath);

  try {
    const content = await fs.readFile(decPath, 'utf-8');
    return JSON.parse(content) as DecisionLog;
  } catch (error) {
    const err = error as NodeJS.ErrnoException;
    if (err.code === 'ENOENT') {
      return {
        version: DECISIONS_VERSION,
        decisions: [],
        lastUpdated: new Date().toISOString()
      };
    }
    throw error;
  }
}

/**
 * Save decision log to disk
 */
export async function saveDecisionLog(projectPath: string, log: DecisionLog): Promise<void> {
  const decPath = getDecisionsFilePath(projectPath);
  const mcpPath = getMcpFolderPath(projectPath);

  await fs.mkdir(mcpPath, { recursive: true });

  log.lastUpdated = new Date().toISOString();
  const content = JSON.stringify(log, null, 2);
  await fs.writeFile(decPath, content, 'utf-8');
}

// ============================================================================
// Decision Operations
// ============================================================================

/**
 * Generate a unique decision ID
 */
function generateDecisionId(): string {
  const timestamp = Date.now();
  const random = Math.random().toString(36).substring(2, 6);
  return `DEC-${timestamp}-${random}`.toUpperCase();
}

/**
 * Log a new decision
 */
export async function logDecision(
  projectPath: string,
  title: string,
  context: string,
  decision: string,
  rationale: string,
  options: {
    category?: DecisionCategory;
    options?: DecisionOption[];
    consequences?: string[];
    alternatives?: string[];
    relatedDecisions?: string[];
    relatedAssets?: string[];
    tags?: string[];
    notes?: string;
  } = {}
): Promise<Decision> {
  const log = await loadDecisionLog(projectPath);

  const newDecision: Decision = {
    id: generateDecisionId(),
    title,
    category: options.category ?? 'other',
    status: 'active',
    date: new Date().toISOString(),
    context,
    decision,
    rationale,
    options: options.options,
    consequences: options.consequences,
    alternatives: options.alternatives,
    relatedDecisions: options.relatedDecisions,
    relatedAssets: options.relatedAssets,
    tags: options.tags,
    notes: options.notes
  };

  log.decisions.push(newDecision);
  await saveDecisionLog(projectPath, log);

  return newDecision;
}

/**
 * Update a decision
 */
export async function updateDecision(
  projectPath: string,
  decisionId: string,
  updates: Partial<Omit<Decision, 'id' | 'date'>>
): Promise<Decision | null> {
  const log = await loadDecisionLog(projectPath);
  const decision = log.decisions.find((d) => d.id === decisionId);

  if (!decision) {
    return null;
  }

  Object.assign(decision, updates);
  await saveDecisionLog(projectPath, log);

  return decision;
}

/**
 * Supersede a decision with a new one
 */
export async function supersedeDecision(
  projectPath: string,
  oldDecisionId: string,
  newDecision: {
    title: string;
    context: string;
    decision: string;
    rationale: string;
    category?: DecisionCategory;
  }
): Promise<{ oldDecision: Decision; newDecision: Decision } | null> {
  const log = await loadDecisionLog(projectPath);
  const oldDec = log.decisions.find((d) => d.id === oldDecisionId);

  if (!oldDec) {
    return null;
  }

  // Create new decision
  const newDec: Decision = {
    id: generateDecisionId(),
    title: newDecision.title,
    category: newDecision.category ?? oldDec.category,
    status: 'active',
    date: new Date().toISOString(),
    context: newDecision.context,
    decision: newDecision.decision,
    rationale: newDecision.rationale,
    relatedDecisions: [oldDecisionId]
  };

  // Mark old as superseded
  oldDec.status = 'superseded';
  oldDec.supersededBy = newDec.id;

  log.decisions.push(newDec);
  await saveDecisionLog(projectPath, log);

  return { oldDecision: oldDec, newDecision: newDec };
}

/**
 * Get a decision by ID
 */
export async function getDecision(
  projectPath: string,
  decisionId: string
): Promise<Decision | null> {
  const log = await loadDecisionLog(projectPath);
  return log.decisions.find((d) => d.id === decisionId) ?? null;
}

/**
 * Get all decisions
 */
export async function getAllDecisions(projectPath: string): Promise<Decision[]> {
  const log = await loadDecisionLog(projectPath);
  return log.decisions;
}

/**
 * Get active decisions only
 */
export async function getActiveDecisions(projectPath: string): Promise<Decision[]> {
  const log = await loadDecisionLog(projectPath);
  return log.decisions.filter((d) => d.status === 'active');
}

/**
 * Get decisions by category
 */
export async function getDecisionsByCategory(
  projectPath: string,
  category: DecisionCategory
): Promise<Decision[]> {
  const log = await loadDecisionLog(projectPath);
  return log.decisions.filter((d) => d.category === category);
}

/**
 * Search decisions by keyword
 */
export async function searchDecisions(
  projectPath: string,
  keyword: string
): Promise<Decision[]> {
  const log = await loadDecisionLog(projectPath);
  const lowerKeyword = keyword.toLowerCase();

  return log.decisions.filter((d) => {
    return (
      d.title.toLowerCase().includes(lowerKeyword) ||
      d.context.toLowerCase().includes(lowerKeyword) ||
      d.decision.toLowerCase().includes(lowerKeyword) ||
      d.rationale.toLowerCase().includes(lowerKeyword) ||
      d.tags?.some((t) => t.toLowerCase().includes(lowerKeyword))
    );
  });
}

/**
 * Get decisions related to an asset
 */
export async function getDecisionsForAsset(
  projectPath: string,
  assetPath: string
): Promise<Decision[]> {
  const log = await loadDecisionLog(projectPath);
  return log.decisions.filter((d) => d.relatedAssets?.includes(assetPath));
}

// ============================================================================
// Summary Generation
// ============================================================================

/**
 * Generate a compact decisions summary
 */
export async function getDecisionsSummary(projectPath: string): Promise<string> {
  const log = await loadDecisionLog(projectPath);
  const active = log.decisions.filter((d) => d.status === 'active');

  if (active.length === 0) {
    return '## Decisions\n\nNo architectural decisions recorded.';
  }

  const lines: string[] = ['## Architectural Decisions', ''];

  // Group by category
  const byCategory = new Map<DecisionCategory, Decision[]>();
  for (const dec of active) {
    const existing = byCategory.get(dec.category) ?? [];
    existing.push(dec);
    byCategory.set(dec.category, existing);
  }

  for (const [category, decisions] of byCategory) {
    lines.push(`### ${formatCategory(category)}`);
    for (const dec of decisions.slice(0, 5)) {
      lines.push(`- **${dec.id}**: ${dec.title}`);
      lines.push(`  _${truncate(dec.decision, 100)}_`);
    }
    if (decisions.length > 5) {
      lines.push(`  _...and ${decisions.length - 5} more_`);
    }
    lines.push('');
  }

  lines.push(`_${active.length} active decisions_`);

  return lines.join('\n');
}

/**
 * Export decisions to markdown (ADR format)
 */
export async function exportDecisionsToMarkdown(projectPath: string): Promise<string> {
  const log = await loadDecisionLog(projectPath);
  const lines: string[] = ['# Architectural Decision Records', '', `_Last updated: ${log.lastUpdated}_`, ''];

  for (const dec of log.decisions) {
    lines.push(`## ${dec.id}: ${dec.title}`);
    lines.push('');
    lines.push(`**Status:** ${dec.status}`);
    lines.push(`**Category:** ${formatCategory(dec.category)}`);
    lines.push(`**Date:** ${dec.date.split('T')[0]}`);
    lines.push('');
    lines.push('### Context');
    lines.push(dec.context);
    lines.push('');
    lines.push('### Decision');
    lines.push(dec.decision);
    lines.push('');
    lines.push('### Rationale');
    lines.push(dec.rationale);

    if (dec.options && dec.options.length > 0) {
      lines.push('');
      lines.push('### Options Considered');
      for (const opt of dec.options) {
        const rejected = opt.rejected ? ' (rejected)' : '';
        lines.push(`- **${opt.name}**${rejected}: ${opt.description}`);
        if (opt.pros) {
          lines.push(`  - Pros: ${opt.pros.join(', ')}`);
        }
        if (opt.cons) {
          lines.push(`  - Cons: ${opt.cons.join(', ')}`);
        }
        if (opt.rejectionReason) {
          lines.push(`  - Rejection reason: ${opt.rejectionReason}`);
        }
      }
    }

    if (dec.consequences && dec.consequences.length > 0) {
      lines.push('');
      lines.push('### Consequences');
      for (const con of dec.consequences) {
        lines.push(`- ${con}`);
      }
    }

    if (dec.supersededBy) {
      lines.push('');
      lines.push(`**Superseded by:** ${dec.supersededBy}`);
    }

    lines.push('');
    lines.push('---');
    lines.push('');
  }

  const mdPath = join(getMcpFolderPath(projectPath), DECISIONS_MD_NAME);
  await fs.writeFile(mdPath, lines.join('\n'), 'utf-8');

  return mdPath;
}

// ============================================================================
// Helpers
// ============================================================================

function formatCategory(category: DecisionCategory): string {
  return category
    .split('_')
    .map((w) => w.charAt(0).toUpperCase() + w.slice(1))
    .join(' ');
}

function truncate(str: string, maxLength: number): string {
  if (str.length <= maxLength) return str;
  return str.substring(0, maxLength - 3) + '...';
}
