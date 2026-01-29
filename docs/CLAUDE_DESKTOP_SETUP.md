# Forge2 Claude Desktop Setup Guide

This guide provides detailed instructions for setting up Forge2 with Claude Desktop.

## Prerequisites

Before you begin, ensure you have:

1. **Node.js 18 or later**
   - Download from [nodejs.org](https://nodejs.org/)
   - Verify: `node --version`

2. **Claude Desktop**
   - Download from [claude.ai/download](https://claude.ai/download)
   - Create an Anthropic account if you don't have one

3. **Unreal Engine 5.0-5.7**
   - Download from [unrealengine.com](https://www.unrealengine.com/)
   - The McpAutomationBridge plugin supports UE 5.0 through 5.7

## Step 1: Install Forge2

```bash
# Clone the repository
git clone https://github.com/yourusername/Forge2.git
cd Forge2/Forge2_Unreal_mcp

# Install dependencies
npm install

# Build the project
npm run build
```

## Step 2: Install Unreal Plugin

Copy the McpAutomationBridge plugin to your Unreal project:

### Windows
```
Copy from: Forge2\Forge2_Unreal_mcp\plugins\McpAutomationBridge\
Copy to:   YourProject\Plugins\McpAutomationBridge\
```

### macOS
```
Copy from: Forge2/Forge2_Unreal_mcp/plugins/McpAutomationBridge/
Copy to:   YourProject/Plugins/McpAutomationBridge/
```

Then in Unreal Editor:
1. Go to **Edit → Plugins**
2. Search for "MCP" or "McpAutomationBridge"
3. Enable the plugin
4. Restart the editor when prompted

## Step 3: Configure Claude Desktop

### Automatic Setup (Recommended)

Run the setup script:

```bash
npm run setup
```

This will:
- Detect your operating system
- Locate the Claude Desktop config file
- Create a backup of any existing configuration
- Add the Forge2 server configuration

### Manual Setup

If you prefer manual configuration or the script doesn't work:

#### Windows

1. Open File Explorer
2. Navigate to `%APPDATA%\Claude\`
3. Create or edit `claude_desktop_config.json`

```json
{
  "mcpServers": {
    "forge2": {
      "command": "node",
      "args": ["C:\path\to\Forge2\Forge2_Unreal_mcp\dist\index.js"],
      "env": {
        "UE_PROJECT_PATH": "C:\path\to\YourProject\YourProject.uproject",
        "MCP_AUTOMATION_HOST": "127.0.0.1",
        "MCP_AUTOMATION_PORT": "8091",
        "LOG_LEVEL": "info",
        "APPDATA": "%APPDATA%"
      }
    }
  }
}
```

#### macOS

1. Open Finder
2. Press `Cmd+Shift+G` and enter `~/Library/Application Support/Claude/`
3. Create or edit `claude_desktop_config.json`

```json
{
  "mcpServers": {
    "forge2": {
      "command": "node",
      "args": ["/path/to/Forge2/Forge2_Unreal_mcp/dist/index.js"],
      "env": {
        "UE_PROJECT_PATH": "/path/to/YourProject/YourProject.uproject",
        "MCP_AUTOMATION_HOST": "127.0.0.1",
        "MCP_AUTOMATION_PORT": "8091",
        "LOG_LEVEL": "info"
      }
    }
  }
}
```

## Step 4: Verify Setup

Run the verification script:

```bash
npm run verify
```

This checks:
- Node.js version
- Forge2 build status
- Claude Desktop configuration
- Connection settings

## Step 5: Start Using Forge2

1. **Start Unreal Engine** with your project (the McpAutomationBridge plugin will start automatically)

2. **Completely quit Claude Desktop** (not just close the window - actually quit the app)

3. **Restart Claude Desktop**

4. **Look for the MCP indicator** - a hammer/tools icon in the bottom-right of the chat input

5. **Click the indicator** to see all available Forge2 tools

6. **Select Claude Opus 4.5** from the model dropdown for best results

7. **Start creating!** Try a prompt like:
   ```
   Create a medieval fantasy village with 5 buildings, a central market square,
   and patrol guard NPCs using Forge2 templates.
   ```

## Troubleshooting

### Claude Desktop doesn't show the MCP indicator

1. Verify the config file is valid JSON (use a JSON validator)
2. Check that the path to `dist/index.js` is correct
3. Ensure Node.js is in your system PATH
4. Check Claude Desktop logs:
   - Windows: `%APPDATA%\Claude\logs\`
   - macOS: `~/Library/Logs/Claude/`

### Connection to Unreal fails

1. Ensure Unreal Engine is running with the project loaded
2. Verify the McpAutomationBridge plugin is enabled
3. Check the port isn't blocked by firewall
4. Default port is 8091 - ensure it matches your config

### "Command not found: node"

1. Ensure Node.js is installed
2. Add Node.js to your system PATH:
   - Windows: Add `C:\Program Files\nodejs\` to PATH
   - macOS: Should be automatic with installer

### Tools appear but don't work

1. Check that Unreal Engine is running
2. Verify the WebSocket connection in Unreal (Window → Developer Tools → Output Log, search for "MCP")
3. Restart both Claude Desktop and Unreal Engine

## Configuration Reference

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `UE_PROJECT_PATH` | Path to your Unreal project file | Required |
| `MCP_AUTOMATION_HOST` | Host for Unreal connection | `127.0.0.1` |
| `MCP_AUTOMATION_PORT` | Port for Unreal connection | `8091` |
| `LOG_LEVEL` | Logging level (debug, info, warn, error) | `info` |
| `WASM_ENABLED` | Enable WebAssembly acceleration | `true` |

### Log Files

- **Forge2 server logs**: Check stderr output in Claude Desktop logs
- **Unreal plugin logs**: Window → Developer Tools → Output Log, filter by "MCP"

## Next Steps

- Read [OPUS_4_5_BEST_PRACTICES.md](./OPUS_4_5_BEST_PRACTICES.md) for tips on using Claude Opus 4.5 effectively
- Explore the 50+ Forge2 tools by clicking the MCP indicator
- Check out example prompts in the main README
