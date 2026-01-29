# Claude Opus 4.5 Best Practices for Forge2

This guide covers best practices for using Claude Opus 4.5 with Forge2 to maximize productivity and results.

## Why Claude Opus 4.5?

Claude Opus 4.5 is the recommended model for Forge2 because:

1. **200K Context Window** - Handle massive game systems without context overflow
2. **Superior Reasoning** - Best model for complex game logic and architecture decisions
3. **Auto-Compaction** - Built-in context management at 100K tokens
4. **Parallel Tool Execution** - Execute multiple MCP tools simultaneously
5. **Native MCP Support** - First-class integration in Claude Desktop

## Selecting the Model

1. Open Claude Desktop
2. Click the model selector dropdown (top of chat)
3. Select **"Claude Opus 4.5"** (claude-opus-4-5-20251101)

The model selection persists for your session.

## Prompt Engineering for Forge2

### Be Specific with Requirements

**Good:**
```
Create a medieval village with:
- 5 residential buildings (2-story half-timber style)
- 1 blacksmith shop with forge
- 1 tavern with interior seating
- Central market square with 4 vendor stalls
- Stone perimeter wall with 2 gates
```

**Less Effective:**
```
Create a village
```

### Use Forge2 Template Names

Reference template types directly for faster results:

```
Using Forge2 templates, create:
- PlayerCharacter template with double jump and dash abilities
- MeleeEnemy template with patrol AI
- FetchQuest template for gathering 10 iron ore
```

### Request Multiple Systems Together

Opus 4.5 can handle complex multi-system requests:

```
Create a complete dungeon experience:
1. Generate a BossDungeon template with 8 rooms using BSP algorithm
2. Add combat rooms with EnemyMelee spawns
3. Create a treasure room with LootTable for rare items
4. Add the boss "IceDragon" with multi-phase AI
5. Set up quest objectives for clearing the dungeon
```

### Leverage Context for Iteration

The 200K context window allows for iterative refinement:

```
Session:
1. "Create a forest biome chunk"
2. "Add a river running through the eastern side"
3. "Place a small village near the river"
4. "Add patrol guards along the village perimeter"
5. "Create a hidden dungeon entrance in the northwest forest"
```

## Managing Large Projects

### Use Session Notes

For complex projects spanning multiple sessions, ask Claude to maintain notes:

```
Please maintain a session log for this project including:
- All assets created
- Configuration decisions made
- What still needs to be done
```

### Break Down Large Tasks

For massive systems, break into phases:

```
Phase 1: Create the base terrain and biomes
Phase 2: Generate cities and road networks
Phase 3: Add NPCs and AI systems
Phase 4: Implement quest chains
Phase 5: Polish and testing
```

### Reference Previous Work

When continuing work:
```
In the last session, we created:
- Forest biome at coordinates (0, 0)
- Village "Riverdale" at coordinates (1000, 500)

Now let's add a quest giver NPC in the village tavern.
```

## Performance Tips

### Parallel Tool Requests

Opus 4.5 can execute independent tools simultaneously. Structure requests to enable parallelism:

```
Create these independently:
1. A sword weapon template
2. A bow weapon template
3. A staff weapon template
```

### Batch Operations

For repetitive tasks, request batches:

```
Create 10 NPC villagers with:
- Random names from medieval naming pool
- Varied appearance templates
- Daily schedules (home, work, tavern)
```

### Avoid Over-Specification

Let Forge2 defaults handle common settings:

**Over-specified:**
```
Create a sword with:
- BaseDamage: 25
- DamageVariance: 5
- AttackSpeed: 1.0
- Range: 150
- StaggerPower: 1.0
- [... 20 more properties]
```

**Better:**
```
Create a MeleeSword template with slightly higher damage than default
```

## Common Patterns

### World Building Pattern

1. Start with terrain and biomes
2. Add water features
3. Generate settlements
4. Connect with roads
5. Populate with NPCs and enemies
6. Add quests and objectives

### Character Development Pattern

1. Define character template (stats, abilities)
2. Create associated AI behavior
3. Add dialogue trees
4. Connect to quests
5. Set up faction relationships

### Combat System Pattern

1. Define weapon templates
2. Configure damage types
3. Set up status effects
4. Create enemy AI profiles
5. Balance and test

## Troubleshooting

### Context Overflow

If you get context warnings:
1. Start a new session for independent work
2. Reference previous sessions briefly
3. Ask Claude to summarize key details

### Slow Responses

For complex operations:
1. Break into smaller requests
2. Use templates instead of full specifications
3. Allow time for large generations (dungeons, cities)

### Unexpected Results

If output doesn't match expectations:
1. Be more specific in requirements
2. Reference exact template types
3. Provide examples of desired outcome

## Integration with Unreal Workflow

### Iterative Development

1. Generate content with Forge2
2. Review in Unreal Editor
3. Request adjustments through Claude
4. Repeat until satisfied

### Combining with Manual Work

```
I've manually placed some buildings in my level.
Please use Forge2 to:
1. Add vegetation around the existing buildings
2. Create NPC schedules that use the buildings
3. Generate patrol paths between buildings
```

### Testing Generated Content

```
Please validate the dungeon layout we created:
1. Check all rooms are reachable
2. Verify boss room is at the end
3. Ensure treasure room has proper loot tables
```

## Example Workflows

### Creating an RPG Village

```
1. "Create a PlainsChunk template at origin"
2. "Generate a village with 8 buildings in the center"
3. "Add a blacksmith, tavern, and general store"
4. "Create 15 villager NPCs with schedules"
5. "Add a merchant NPC with equipment shop"
6. "Create a quest giver with 3 fetch quests"
7. "Add guard patrol paths around the perimeter"
```

### Building a Boss Encounter

```
1. "Create a BossDungeon template with fire theme"
2. "Generate the boss 'FireLord' with 3 phases"
3. "Add phase 1: basic melee attacks"
4. "Add phase 2: fire breath and minion spawns"
5. "Add phase 3: enrage mode with AoE attacks"
6. "Create legendary loot table for boss drops"
7. "Add achievement for defeating the boss"
```

### Setting Up Multiplayer

```
1. "Configure dedicated server settings"
2. "Set up player replication for 16 players"
3. "Create team-based spawn points"
4. "Add matchmaking session system"
5. "Configure voice chat integration"
```

## Conclusion

Claude Opus 4.5 + Forge2 is a powerful combination for game development. Use specific prompts, leverage templates, and take advantage of the large context window for complex, iterative development workflows.
