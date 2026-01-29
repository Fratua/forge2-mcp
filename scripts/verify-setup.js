#!/usr/bin/env node

/**
 * Forge2 Setup Verification Script
 *
 * This script verifies that Forge2 is correctly configured for Claude Desktop.
 */

import { readFileSync, existsSync } from 'fs';
import { join, dirname } from 'path';
import { homedir, platform } from 'os';
import { fileURLToPath } from 'url';
import { execSync } from 'child_process';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Colors for console output
const colors = {
  reset: '\x1b[0m',
  bright: '\x1b[1m',
  red: '\x1b[31m',
  green: '\x1b[32m',
  yellow: '\x1b[33m',
  cyan: '\x1b[36m'
};

function log(message, color = colors.reset) {
  console.log(`${color}${message}${colors.reset}`);
}

function logSuccess(message) {
  log(`[PASS] ${message}`, colors.green);
}

function logError(message) {
  log(`[FAIL] ${message}`, colors.red);
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
 * Check Node.js version
 */
function checkNodeVersion() {
  try {
    const version = process.version;
    const major = parseInt(version.slice(1).split('.')[0], 10);
    
    if (major >= 18) {
      logSuccess(`Node.js version: ${version} (>= 18 required)`);
      return true;
    } else {
      logError(`Node.js version: ${version} (>= 18 required)`);
      return false;
    }
  } catch (error) {
    logError(`Could not check Node.js version: ${error.message}`);
    return false;
  }
}

/**
 * Check if Forge2 is built
 */
function checkBuild() {
  const entryPoint = join(__dirname, '..', 'dist', 'index.js');
  
  if (existsSync(entryPoint)) {
    logSuccess(`Forge2 build found: ${entryPoint}`);
    return true;
  } else {
    logError(`Forge2 build not found: ${entryPoint}`);
    logInfo('Run "npm run build" to build the project');
    return false;
  }
}

/**
 * Check Claude Desktop config
 */
function checkClaudeDesktopConfig() {
  const configPath = getClaudeDesktopConfigPath();
  
  if (!existsSync(configPath)) {
    logError(`Claude Desktop config not found: ${configPath}`);
    logInfo('Run "npm run setup" to configure Claude Desktop');
    return false;
  }
  
  try {
    const content = readFileSync(configPath, 'utf8');
    const config = JSON.parse(content);
    
    if (!config.mcpServers) {
      logError('Claude Desktop config missing mcpServers section');
      return false;
    }
    
    if (!config.mcpServers.forge2) {
      logError('Forge2 not configured in Claude Desktop');
      logInfo('Run "npm run setup" to configure Forge2');
      return false;
    }
    
    const forge2Config = config.mcpServers.forge2;
    
    // Check command
    if (forge2Config.command !== 'node') {
      logWarning(`Unexpected command: ${forge2Config.command} (expected: node)`);
    }
    
    // Check args
    if (!forge2Config.args || forge2Config.args.length === 0) {
      logError('Forge2 config missing args');
      return false;
    }
    
    const entryPointPath = forge2Config.args[0];
    if (!existsSync(entryPointPath.replace(/\\/g, '\'))) {
      logWarning(`Entry point path may not exist: ${entryPointPath}`);
    }
    
    logSuccess(`Forge2 configured in Claude Desktop`);
    logInfo(`Entry point: ${forge2Config.args[0]}`);
    
    return true;
  } catch (error) {
    logError(`Could not parse Claude Desktop config: ${error.message}`);
    return false;
  }
}

/**
 * Check if Unreal Engine is running with the plugin
 */
function checkUnrealConnection() {
  const host = process.env.UNREAL_HOST || 'localhost';
  const port = process.env.UNREAL_PORT || '8080';
  
  logInfo(`Checking Unreal Engine connection at ${host}:${port}...`);
  logWarning('Note: Unreal Engine must be running with McpAutomationBridge plugin loaded');
  logInfo('This check is informational - the connection is made at runtime');
  
  return true;
}

/**
 * Main verification function
 */
async function verify() {
  log('\n========================================================', colors.bright);
  log('       Forge2 Setup Verification                        ', colors.bright);
  log('========================================================\n', colors.bright);

  let allPassed = true;

  // Check Node.js version
  log('Checking Node.js version...', colors.cyan);
  if (!checkNodeVersion()) allPassed = false;
  log('');

  // Check Forge2 build
  log('Checking Forge2 build...', colors.cyan);
  if (!checkBuild()) allPassed = false;
  log('');

  // Check Claude Desktop config
  log('Checking Claude Desktop configuration...', colors.cyan);
  if (!checkClaudeDesktopConfig()) allPassed = false;
  log('');

  // Check Unreal connection info
  log('Checking Unreal Engine connection info...', colors.cyan);
  checkUnrealConnection();
  log('');

  // Summary
  log('========================================================', allPassed ? colors.green : colors.red);
  if (allPassed) {
    log('           All checks passed!                           ', colors.green);
    log('========================================================\n', colors.green);
    log('Forge2 is ready to use with Claude Desktop.', colors.bright);
    log('1. Start Unreal Engine with McpAutomationBridge plugin', colors.cyan);
    log('2. Open Claude Desktop', colors.cyan);
    log('3. Look for the MCP indicator and start using Forge2 tools!\n', colors.cyan);
  } else {
    log('           Some checks failed                           ', colors.red);
    log('========================================================\n', colors.red);
    log('Please address the issues above before using Forge2.', colors.bright);
    log('Run "npm run setup" to configure Claude Desktop.\n', colors.cyan);
  }

  process.exit(allPassed ? 0 : 1);
}

verify().catch(error => {
  logError(`Verification failed: ${error.message}`);
  process.exit(1);
});
