# Forge2: Ultimate AAA Game Development MCP Extension

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Claude Desktop](https://img.shields.io/badge/Claude%20Desktop-Required-blue)](https://claude.ai/download)
[![Claude Opus 4.5](https://img.shields.io/badge/Claude%20Opus%204.5-Recommended-purple)](https://claude.ai)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.0--5.7-orange)](https://www.unrealengine.com/)

**Forge2** is the most comprehensive AI-assisted game development system ever created for Unreal Engine. Designed exclusively for Claude Desktop with Claude Opus 4.5, it enables you to create ANY game of ANY genre, complexity, style, or depth through natural language.

---

## What Can Forge2 Do?

| Category | Capabilities |
|----------|-------------|
| **World Generation** | Procedural terrain, biomes, dungeons, cities, buildings with full interiors |
| **15+ Game Genres** | Racing, Fighting, Platformer, Stealth, Puzzle, Strategy, Sports, Horror, Survival, MOBA, Battle Royale, Rhythm, Card games |
| **Complete RPG Stack** | Quests, dialogue trees, skill trees, inventory, crafting, economy |
| **Combat Systems** | Melee, ranged, targeting, damage types, status effects, combos |
| **AAA Tech Stack** | Nanite, Lumen, MetaSounds, Chaos Physics, Control Rig, GAS |
| **Multiplayer** | Replication, RPCs, dedicated servers, MMO infrastructure |
| **Polish Features** | Photo mode, accessibility, localization, achievements |

## Quick Start

### Prerequisites

- **Node.js 18+** - [nodejs.org](https://nodejs.org/)
- **Claude Desktop** - [claude.ai/download](https://claude.ai/download)
- **Unreal Engine 5.0-5.7** - [unrealengine.com](https://www.unrealengine.com/)

### Installation

```bash
# Clone the repository
git clone https://github.com/Fratua/forge2-mcp.git
cd forge2-mcp/Forge2_Unreal_mcp

# Install dependencies and build
npm install
npm run build

# Configure Claude Desktop (automatic)
npm run setup

# Verify setup
npm run verify
```

### Install Unreal Plugin

Copy the McpAutomationBridge plugin to your project:

```
From: Forge2_Unreal_mcp/plugins/McpAutomationBridge/
To:   YourUnrealProject/Plugins/McpAutomationBridge/
```

Enable in Unreal: **Edit → Plugins → search "MCP" → Enable → Restart**

### Configure Claude Desktop

After running `npm run setup`, restart Claude Desktop and look for the **MCP tools icon** in the chat input.

**Manual Configuration (Windows):**

Edit `%APPDATA%\Claude\claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "forge2": {
      "command": "node",
      "args": ["C:\\path\\to\\Forge2_Unreal_mcp\\dist\\index.js"],
      "env": {
        "UE_PROJECT_PATH": "C:\\path\\to\\YourProject",
        "MCP_AUTOMATION_PORT": "8091"
      }
    }
  }
}
```

**Manual Configuration (macOS):**

Edit `~/Library/Application Support/Claude/claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "forge2": {
      "command": "node",
      "args": ["/path/to/Forge2_Unreal_mcp/dist/index.js"],
      "env": {
        "UE_PROJECT_PATH": "/path/to/YourProject",
        "MCP_AUTOMATION_PORT": "8091"
      }
    }
  }
}
```

---

## Using Claude Opus 4.5

For the best experience, select **Claude Opus 4.5** in Claude Desktop:

1. Click the model selector dropdown in the chat
2. Select "Claude Opus 4.5"
3. Start creating!

Opus 4.5 provides:
- **200K context window** for complex game systems
- **Superior reasoning** for game architecture
- **Better tool orchestration** for multi-step workflows

---

## Available Tools (50+)

### Core Tools
| Tool | Description |
|------|-------------|
| `manage_asset` | Assets, materials, render targets |
| `manage_blueprint` | Blueprint creation and graph editing |
| `control_actor` | Spawn, transform, physics, components |
| `control_editor` | PIE, camera, viewport, screenshots |

### World Building
| Tool | Description |
|------|-------------|
| `manage_worldgen` | Procedural terrain, biomes, structures |
| `build_environment` | Landscape, foliage, procedural content |
| `manage_level` | Level streaming, World Partition |

### Game Systems
| Tool | Description |
|------|-------------|
| `manage_quest` | Quest definitions, objectives, rewards |
| `manage_dialogue` | Dialogue trees, conditions, consequences |
| `manage_skill_tree` | Skill trees, classes, progression |
| `manage_inventory` | Items, equipment, loot, crafting |
| `manage_economy` | Currency, shops, trading, auction |
| `manage_combat` | Combat systems, damage, targeting |

### Genre-Specific
| Tool | Description |
|------|-------------|
| `manage_genre_racing` | Vehicles, tracks, race modes |
| `manage_genre_fighting` | Movesets, combos, frame data |
| `manage_genre_platformer` | Jump physics, platforms, checkpoints |
| `manage_genre_stealth` | Detection, AI awareness, disguises |
| `manage_genre_puzzle` | Logic puzzles, physics puzzles |
| `manage_genre_strategy` | Units, buildings, resources |
| ...and 7 more genres |

### AAA Features
| Tool | Description |
|------|-------------|
| `manage_nanite` | Nanite mesh configuration |
| `manage_lumen` | Lumen GI and reflections |
| `manage_metasounds` | Procedural audio |
| `manage_chaos` | Physics and destruction |
| `manage_cinematics` | Sequencer and cutscenes |

---

## Example Workflows

### Create a Procedural Dungeon

```
"Use manage_worldgen to generate a dungeon with:
- BSP algorithm
- 15 rooms ranging from 10x10 to 30x30
- Room types: treasure, boss, trap, rest
- Corridor width of 3
- Include loops for multiple paths"
```

### Set Up an RPG Quest System

```
"Create a complete quest system:
1. Use manage_quest to create a main quest 'The Lost Artifact'
2. Add objectives: talk to NPC, explore dungeon, defeat boss, return
3. Use manage_dialogue to create conversation trees for quest givers
4. Use manage_inventory to create the artifact as a quest reward"
```

### Build a Racing Game

```
"Set up a racing game:
1. Use manage_genre_racing to create a sports car vehicle
2. Configure arcade-style physics with drift support
3. Generate a procedural track with 12 checkpoints
4. Set up time trial and circuit race modes"
```

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│              Claude Desktop (Claude Opus 4.5)                    │
│  • Native MCP support  • 200K context  • Multi-tool execution   │
└─────────────────────────────────────────────────────────────────┘
                              │ MCP Protocol (stdio)
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                  TypeScript MCP Server (Forge2)                  │
│  • 50+ tools  • Context engineering  • Handler routing          │
└─────────────────────────────────────────────────────────────────┘
                              │ WebSocket
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│              C++ Unreal Plugin (McpAutomationBridge)             │
│  • Native UE integration  • Editor automation  • Game systems   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Unreal Engine 5.0-5.7                         │
│  PCG • Geometry Script • GAS • Nanite • Lumen • Chaos          │
└─────────────────────────────────────────────────────────────────┘
```

---

## Documentation

| Document | Description |
|----------|-------------|
| [Claude Desktop Setup](docs/CLAUDE_DESKTOP_SETUP.md) | Detailed setup guide |
| [Opus 4.5 Best Practices](docs/OPUS_4_5_BEST_PRACTICES.md) | Optimization guide |
| [Handler Mappings](docs/handler-mapping.md) | TypeScript to C++ routing |
| [Plugin Extension](docs/editor-plugin-extension.md) | C++ plugin architecture |

---

## Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `UE_PROJECT_PATH` | Path to your .uproject file | - |
| `MCP_AUTOMATION_HOST` | Plugin host address | `127.0.0.1` |
| `MCP_AUTOMATION_PORT` | Plugin port | `8091` |
| `LOG_LEVEL` | Logging verbosity | `info` |

---

## Troubleshooting

### Tools not appearing in Claude Desktop

1. Ensure `npm run build` completed successfully
2. Check the config path is correct (use absolute paths)
3. **Completely quit** Claude Desktop and restart
4. Check logs: `%APPDATA%\Claude\logs\` (Windows) or `~/Library/Logs/Claude/` (macOS)

### Connection to Unreal fails

1. Start Unreal Editor first
2. Ensure McpAutomationBridge plugin is enabled
3. Check the port matches (default: 8091)
4. Connection is lazy - it connects on first tool use

### Run verification

```bash
npm run verify
```

---

## Development

```bash
npm run build        # Build TypeScript
npm run build:watch  # Watch mode
npm run lint         # Run ESLint
npm run test         # Run tests
npm run verify       # Verify setup
```

---

## Contributing

Contributions welcome! Please:
- Include reproduction steps for bugs
- Keep PRs focused and small
- Follow existing code style
- Test with Claude Desktop before submitting

---

## License

MIT — See [LICENSE](LICENSE)
