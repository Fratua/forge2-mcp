#!/usr/bin/env node

/**
 * Forge2 Claude Desktop Setup Script
 *
 * This script helps configure Claude Desktop to use the Forge2 MCP server.
 * It detects the operating system, locates the Claude Desktop config file,
 * and merges the Forge2 configuration into it.
 */

import { readFileSync, writeFileSync, existsSync, mkdirSync, copyFileSync } from 'fs';
import { join, dirname } from 'path';
import { homedir, platform } from 'os';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Colors for console output
const colors = {
  reset: '\x1b[0m',
  bright: '\x1b[1m',
  red: '\x1b[31m',
  green: '\x1b[32m',
  yellow: '\x1b[33m',
  blue: '\x1b[34m',
  cyan: '\x1b[36m'
};

function log(message, color = colors.reset) {
  console.log(`${color}${message}${colors.reset}`);
}

function logSuccess(message) {
  log(`[OK] ${message}`, colors.green);
}

function logError(message) {
  log(`[ERROR] ${message}`, colors.red);
}

function logInfo(message) {
  log(`[INFO] ${message}`, colors.cyan);
}

function logWarning(message) {
  log(`[WARN] ${message}`, colors.yellow);
}

/**
 * Get the Claude Desktop config file path based on OS
 */
function getClaudeDesktopConfigPath() {
  const os = platform();

  if (os === 'win32') {
    const appData = process.env.APPDATA || join(homedir(), 'AppData', 'Roaming');
    return join(appData, 'Claude', 'claude_desktop_config.json');
  } else if (os === 'darwin') {
    return join(homedir(), 'Library', 'Application Support', 'Claude', 'claude_desktop_config.json');
  } else {
    return join(homedir(), '.config', 'Claude', 'claude_desktop_config.json');
  }
}

/**
 * Get the Forge2 project directory
 */
function getForge2Dir() {
  return join(__dirname, '..');
}

/**
 * Get the Forge2 dist/index.js path
 */
function getForge2EntryPoint() {
  const forge2Dir = getForge2Dir();
  return join(forge2Dir, 'dist', 'index.js');
}

/**
 * Create the Forge2 MCP server configuration
 */
function createForge2Config() {
  const entryPoint = getForge2EntryPoint();
  const os = platform();

  const formattedPath = os === 'win32'
    ? entryPoint.replace(/\//g, '\\')
    : entryPoint;

  const config = {
    command: 'node',
    args: [formattedPath],
    env: {
      UNREAL_HOST: 'localhost',
      UNREAL_PORT: '8080',
      LOG_LEVEL: 'info'
    }
  };

  if (os === 'win32') {
    config.env.APPDATA = process.env.APPDATA || '';
  }

  return config;
}

/**
 * Read and parse existing config, or create empty config
 */
function readExistingConfig(configPath) {
  if (existsSync(configPath)) {
    try {
      const content = readFileSync(configPath, 'utf8');
      return JSON.parse(content);
    } catch (error) {
      logWarning(`Could not parse existing config: ${error.message}`);
      logInfo('Creating new configuration...');
      return { mcpServers: {} };
    }
  }
  return { mcpServers: {} };
}

/**
 * Backup existing config file
 */
function backupConfig(configPath) {
  if (existsSync(configPath)) {
    const backupPath = `${configPath}.backup.${Date.now()}`;
    copyFileSync(configPath, backupPath);
    logInfo(`Backed up existing config to: ${backupPath}`);
    return backupPath;
  }
  return null;
}

/**
 * Main setup function
 */
async function setup() {
  log('\n========================================================', colors.bright);
  log('       Forge2 Claude Desktop Setup                      ', colors.bright);
  log('========================================================\n', colors.bright);

  const os = platform();
  logInfo(`Detected OS: ${os}`);

  const entryPoint = getForge2EntryPoint();
  if (!existsSync(entryPoint)) {
    logError(`Forge2 entry point not found: ${entryPoint}`);
    logInfo('Please run "npm run build" first to build the project.');
    process.exit(1);
  }
  logSuccess(`Forge2 entry point found: ${entryPoint}`);

  const configPath = getClaudeDesktopConfigPath();
  logInfo(`Claude Desktop config path: ${configPath}`);

  const configDir = dirname(configPath);
  if (!existsSync(configDir)) {
    mkdirSync(configDir, { recursive: true });
    logInfo(`Created config directory: ${configDir}`);
  }

  backupConfig(configPath);

  const existingConfig = readExistingConfig(configPath);
  const forge2Config = createForge2Config();

  existingConfig.mcpServers = existingConfig.mcpServers || {};

  if (existingConfig.mcpServers.forge2) {
    logWarning('Forge2 is already configured. Updating configuration...');
  }

  existingConfig.mcpServers.forge2 = forge2Config;

  try {
    writeFileSync(configPath, JSON.stringify(existingConfig, null, 2), 'utf8');
    logSuccess(`Configuration written to: ${configPath}`);
  } catch (error) {
    logError(`Failed to write config: ${error.message}`);
    process.exit(1);
  }

  log('\n========================================================', colors.green);
  log('                    Setup Complete!                      ', colors.green);
  log('========================================================\n', colors.green);

  log('Next steps:', colors.bright);
  log('1. Completely quit Claude Desktop (not just close the window)', colors.cyan);
  log('2. Start Unreal Engine with the McpAutomationBridge plugin loaded', colors.cyan);
  log('3. Restart Claude Desktop', colors.cyan);
  log('4. Look for the MCP server indicator (hammer icon) in the chat input', colors.cyan);
  log('5. Click the indicator to see Forge2 tools\n', colors.cyan);

  logInfo('Run "npm run verify" to verify your setup');
}

setup().catch(error => {
  logError(`Setup failed: ${error.message}`);
  process.exit(1);
});
