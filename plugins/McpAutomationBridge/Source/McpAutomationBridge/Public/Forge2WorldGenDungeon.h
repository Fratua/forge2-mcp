// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Dungeon Generation Algorithms

#pragma once

#include "CoreMinimal.h"
#include "Forge2WorldGenDungeon.generated.h"

/**
 * Room purpose types for dungeon rooms
 */
UENUM(BlueprintType)
enum class EForge2RoomPurpose : uint8
{
    Normal UMETA(DisplayName = "Normal"),
    Entrance UMETA(DisplayName = "Entrance"),
    Exit UMETA(DisplayName = "Exit"),
    Treasure UMETA(DisplayName = "Treasure"),
    Boss UMETA(DisplayName = "Boss"),
    Trap UMETA(DisplayName = "Trap"),
    Rest UMETA(DisplayName = "Rest/Safe"),
    Secret UMETA(DisplayName = "Secret"),
    Shop UMETA(DisplayName = "Shop"),
    Puzzle UMETA(DisplayName = "Puzzle")
};

/**
 * Dungeon tile types
 */
UENUM(BlueprintType)
enum class EForge2DungeonTile : uint8
{
    Empty UMETA(DisplayName = "Empty/Void"),
    Floor UMETA(DisplayName = "Floor"),
    Wall UMETA(DisplayName = "Wall"),
    Door UMETA(DisplayName = "Door"),
    SecretDoor UMETA(DisplayName = "Secret Door"),
    Corridor UMETA(DisplayName = "Corridor"),
    Stairs UMETA(DisplayName = "Stairs"),
    Water UMETA(DisplayName = "Water"),
    Pit UMETA(DisplayName = "Pit/Hole"),
    Decoration UMETA(DisplayName = "Decoration")
};

/**
 * Represents a dungeon room
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DungeonRoom
{
    GENERATED_BODY()

    /** Room bounds (min X, min Y, max X, max Y) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FIntRect Bounds;

    /** Room center point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FIntPoint Center;

    /** Room purpose */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EForge2RoomPurpose Purpose = EForge2RoomPurpose::Normal;

    /** Connected room indices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<int32> ConnectedRooms;

    /** Room ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 RoomID = -1;

    /** Depth in the dungeon (distance from entrance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Depth = 0;

    FForge2DungeonRoom()
    {
        Bounds = FIntRect(0, 0, 0, 0);
        Center = FIntPoint(0, 0);
    }

    FForge2DungeonRoom(int32 X, int32 Y, int32 Width, int32 Height)
    {
        Bounds = FIntRect(X, Y, X + Width, Y + Height);
        Center = FIntPoint(X + Width / 2, Y + Height / 2);
    }

    int32 GetWidth() const { return Bounds.Max.X - Bounds.Min.X; }
    int32 GetHeight() const { return Bounds.Max.Y - Bounds.Min.Y; }
    int32 GetArea() const { return GetWidth() * GetHeight(); }

    bool Intersects(const FForge2DungeonRoom& Other, int32 Padding = 0) const
    {
        return !(Bounds.Max.X + Padding <= Other.Bounds.Min.X ||
                 Other.Bounds.Max.X + Padding <= Bounds.Min.X ||
                 Bounds.Max.Y + Padding <= Other.Bounds.Min.Y ||
                 Other.Bounds.Max.Y + Padding <= Bounds.Min.Y);
    }
};

/**
 * Represents a corridor connecting rooms
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DungeonCorridor
{
    GENERATED_BODY()

    /** Start point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FIntPoint Start;

    /** End point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FIntPoint End;

    /** Corridor width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Width = 1;

    /** Connected room indices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 RoomA = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 RoomB = -1;

    /** Waypoints for L-shaped or complex corridors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FIntPoint> Waypoints;
};

/**
 * Dungeon generation result
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DungeonData
{
    GENERATED_BODY()

    /** 2D tile map */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<EForge2DungeonTile> Tiles;

    /** Width of the dungeon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Width = 0;

    /** Height of the dungeon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Height = 0;

    /** All rooms in the dungeon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FForge2DungeonRoom> Rooms;

    /** All corridors in the dungeon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FForge2DungeonCorridor> Corridors;

    /** Seed used for generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Seed = 0;

    /** Get tile at position */
    EForge2DungeonTile GetTile(int32 X, int32 Y) const
    {
        if (X < 0 || X >= Width || Y < 0 || Y >= Height) return EForge2DungeonTile::Empty;
        return Tiles[Y * Width + X];
    }

    /** Set tile at position */
    void SetTile(int32 X, int32 Y, EForge2DungeonTile Tile)
    {
        if (X >= 0 && X < Width && Y >= 0 && Y < Height)
        {
            Tiles[Y * Width + X] = Tile;
        }
    }

    /** Initialize with dimensions */
    void Initialize(int32 InWidth, int32 InHeight, EForge2DungeonTile FillTile = EForge2DungeonTile::Empty)
    {
        Width = InWidth;
        Height = InHeight;
        Tiles.SetNum(Width * Height);
        for (int32 i = 0; i < Tiles.Num(); ++i)
        {
            Tiles[i] = FillTile;
        }
    }
};

/**
 * BSP Dungeon generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BSPDungeonParams
{
    GENERATED_BODY()

    /** Dungeon width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "20", ClampMax = "500"))
    int32 Width = 80;

    /** Dungeon height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "20", ClampMax = "500"))
    int32 Height = 60;

    /** Minimum room width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "4", ClampMax = "50"))
    int32 MinRoomSize = 6;

    /** Maximum room width/height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "8", ClampMax = "100"))
    int32 MaxRoomSize = 15;

    /** Minimum partition size before stopping subdivision */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "10", ClampMax = "100"))
    int32 MinPartitionSize = 12;

    /** Maximum recursion depth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxDepth = 5;

    /** Corridor width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1", ClampMax = "5"))
    int32 CorridorWidth = 2;

    /** Random seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Seed = 0;

    /** Room padding from partition edges */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1", ClampMax = "5"))
    int32 RoomPadding = 1;
};

/**
 * Cellular Automata parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CellularAutomataParams
{
    GENERATED_BODY()

    /** Cave width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "20", ClampMax = "500"))
    int32 Width = 80;

    /** Cave height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "20", ClampMax = "500"))
    int32 Height = 60;

    /** Initial fill percentage (0-100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "30", ClampMax = "70"))
    int32 FillPercent = 45;

    /** Number of smoothing iterations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1", ClampMax = "20"))
    int32 Iterations = 5;

    /** Neighbor count threshold for birth (cell becomes floor) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1", ClampMax = "8"))
    int32 BirthLimit = 4;

    /** Neighbor count threshold for death (cell becomes wall) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1", ClampMax = "8"))
    int32 DeathLimit = 3;

    /** Random seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Seed = 0;

    /** Connect disconnected regions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    bool bConnectRegions = true;

    /** Minimum region size to keep (smaller regions are filled) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1"))
    int32 MinRegionSize = 50;
};

/**
 * Drunkard's Walk parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DrunkardWalkParams
{
    GENERATED_BODY()

    /** Cave width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "20", ClampMax = "500"))
    int32 Width = 80;

    /** Cave height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "20", ClampMax = "500"))
    int32 Height = 60;

    /** Target fill percentage (0-100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "20", ClampMax = "80"))
    int32 FillPercent = 40;

    /** Number of walkers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1", ClampMax = "20"))
    int32 WalkerCount = 3;

    /** Maximum steps per walker (0 = unlimited until target fill) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "0"))
    int32 MaxSteps = 0;

    /** Random seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Seed = 0;

    /** Walker carve radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "1", ClampMax = "5"))
    int32 CarveRadius = 1;

    /** Start from center */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    bool bStartFromCenter = true;

    /** Weighted direction (tends to continue in same direction) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirectionWeight = 0.25f;
};

/**
 * WFC (Wave Function Collapse) tile definition
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WFCTile
{
    GENERATED_BODY()

    /** Tile ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    int32 TileID = 0;

    /** Tile type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    EForge2DungeonTile TileType = EForge2DungeonTile::Empty;

    /** Weight for selection probability */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    float Weight = 1.0f;

    /** Valid neighbor tiles for each direction (North, East, South, West) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    TArray<int32> ValidNorth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    TArray<int32> ValidEast;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    TArray<int32> ValidSouth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    TArray<int32> ValidWest;
};

/**
 * WFC parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WFCParams
{
    GENERATED_BODY()

    /** Output width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC", meta = (ClampMin = "5", ClampMax = "200"))
    int32 Width = 40;

    /** Output height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC", meta = (ClampMin = "5", ClampMax = "200"))
    int32 Height = 30;

    /** Tile set to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    TArray<FForge2WFCTile> Tiles;

    /** Random seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
    int32 Seed = 0;

    /** Maximum attempts before giving up */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxAttempts = 10;
};

/**
 * BSP Tree Node for dungeon generation
 */
struct MCPAUTOMATIONBRIDGE_API FForge2BSPNode
{
    FIntRect Bounds;
    TSharedPtr<FForge2BSPNode> Left;
    TSharedPtr<FForge2BSPNode> Right;
    FForge2DungeonRoom Room;
    bool bHasRoom = false;
    int32 Depth = 0;

    FForge2BSPNode() : Bounds(0, 0, 0, 0) {}
    FForge2BSPNode(const FIntRect& InBounds) : Bounds(InBounds) {}

    bool IsLeaf() const { return !Left.IsValid() && !Right.IsValid(); }
    int32 GetWidth() const { return Bounds.Max.X - Bounds.Min.X; }
    int32 GetHeight() const { return Bounds.Max.Y - Bounds.Min.Y; }
};

/**
 * BSP Dungeon Generator
 * Uses Binary Space Partitioning to create room-based dungeons
 */
class MCPAUTOMATIONBRIDGE_API FForge2BSPDungeon
{
public:
    FForge2BSPDungeon() = default;

    /**
     * Generate a BSP dungeon
     * @param Params - Generation parameters
     * @return Generated dungeon data
     */
    FForge2DungeonData Generate(const FForge2BSPDungeonParams& Params);

private:
    /** Recursively subdivide the space */
    void Subdivide(TSharedPtr<FForge2BSPNode> Node, int32 Depth, const FForge2BSPDungeonParams& Params, FRandomStream& Random);

    /** Create rooms in leaf nodes */
    void CreateRooms(TSharedPtr<FForge2BSPNode> Node, FForge2DungeonData& Dungeon, const FForge2BSPDungeonParams& Params, FRandomStream& Random);

    /** Connect sibling rooms */
    void ConnectRooms(TSharedPtr<FForge2BSPNode> Node, FForge2DungeonData& Dungeon, const FForge2BSPDungeonParams& Params, FRandomStream& Random);

    /** Get a room from a node (either its own or a child's) */
    FForge2DungeonRoom* GetRoom(TSharedPtr<FForge2BSPNode> Node);

    /** Carve a corridor between two points */
    void CarveCorridor(FForge2DungeonData& Dungeon, const FIntPoint& Start, const FIntPoint& End, int32 Width);

    /** Carve a room into the dungeon */
    void CarveRoom(FForge2DungeonData& Dungeon, const FForge2DungeonRoom& Room);
};

/**
 * Cellular Automata Cave Generator
 * Creates organic cave systems using cellular automata rules
 */
class MCPAUTOMATIONBRIDGE_API FForge2CellularAutomata
{
public:
    FForge2CellularAutomata() = default;

    /**
     * Generate a cave using cellular automata
     * @param Params - Generation parameters
     * @return Generated dungeon data
     */
    FForge2DungeonData Generate(const FForge2CellularAutomataParams& Params);

private:
    /** Apply one iteration of the automata rules */
    void ApplyIteration(TArray<EForge2DungeonTile>& Tiles, int32 Width, int32 Height, const FForge2CellularAutomataParams& Params);

    /** Count floor neighbors */
    int32 CountFloorNeighbors(const TArray<EForge2DungeonTile>& Tiles, int32 X, int32 Y, int32 Width, int32 Height);

    /** Find all regions using flood fill */
    TArray<TArray<FIntPoint>> FindRegions(const TArray<EForge2DungeonTile>& Tiles, int32 Width, int32 Height, EForge2DungeonTile TileType);

    /** Flood fill to find a region */
    TArray<FIntPoint> FloodFill(const TArray<EForge2DungeonTile>& Tiles, int32 StartX, int32 StartY, int32 Width, int32 Height, EForge2DungeonTile TileType, TArray<bool>& Visited);

    /** Connect regions with corridors */
    void ConnectRegions(FForge2DungeonData& Dungeon, const TArray<TArray<FIntPoint>>& Regions, FRandomStream& Random);

    /** Find closest points between two regions */
    void FindClosestPoints(const TArray<FIntPoint>& RegionA, const TArray<FIntPoint>& RegionB, FIntPoint& OutPointA, FIntPoint& OutPointB);

    /** Carve a corridor between two points */
    void CarveCorridor(FForge2DungeonData& Dungeon, const FIntPoint& Start, const FIntPoint& End);

    /** Identify rooms from floor regions */
    void IdentifyRooms(FForge2DungeonData& Dungeon);
};

/**
 * Drunkard's Walk Cave Generator
 * Creates organic caves by simulating random walkers
 */
class MCPAUTOMATIONBRIDGE_API FForge2DrunkardWalk
{
public:
    FForge2DrunkardWalk() = default;

    /**
     * Generate a cave using drunkard's walk
     * @param Params - Generation parameters
     * @return Generated dungeon data
     */
    FForge2DungeonData Generate(const FForge2DrunkardWalkParams& Params);

private:
    /** Simulate a single walker */
    void SimulateWalker(FForge2DungeonData& Dungeon, FIntPoint& Position, const FForge2DrunkardWalkParams& Params, FRandomStream& Random, int32& FilledCount, int32 TargetFill, FIntPoint& LastDirection);

    /** Carve at position with radius */
    int32 CarveAt(FForge2DungeonData& Dungeon, int32 X, int32 Y, int32 Radius);

    /** Get random direction, weighted towards previous direction */
    FIntPoint GetRandomDirection(FRandomStream& Random, const FIntPoint& LastDir, float Weight);

    /** Identify rooms from carved areas */
    void IdentifyRooms(FForge2DungeonData& Dungeon);
};

/**
 * Wave Function Collapse Dungeon Generator
 * Creates tile-based dungeons using constraint propagation
 */
class MCPAUTOMATIONBRIDGE_API FForge2WFCDungeon
{
public:
    FForge2WFCDungeon() = default;

    /**
     * Generate a dungeon using WFC
     * @param Params - Generation parameters
     * @return Generated dungeon data
     */
    FForge2DungeonData Generate(const FForge2WFCParams& Params);

    /** Create a default dungeon tile set */
    static TArray<FForge2WFCTile> CreateDefaultTileSet();

private:
    /** Internal cell state */
    struct FWFCCell
    {
        TArray<int32> PossibleTiles;
        int32 CollapsedTile = -1;
        bool bCollapsed = false;

        float GetEntropy() const { return static_cast<float>(PossibleTiles.Num()); }
    };

    /** Initialize the wave */
    void InitializeWave(TArray<FWFCCell>& Wave, int32 Width, int32 Height, const TArray<FForge2WFCTile>& Tiles);

    /** Find cell with minimum entropy */
    int32 FindMinEntropyCell(const TArray<FWFCCell>& Wave, FRandomStream& Random);

    /** Collapse a cell to a single tile */
    bool CollapseCell(TArray<FWFCCell>& Wave, int32 CellIndex, const TArray<FForge2WFCTile>& Tiles, FRandomStream& Random);

    /** Propagate constraints after a collapse */
    bool Propagate(TArray<FWFCCell>& Wave, int32 Width, int32 Height, const TArray<FForge2WFCTile>& Tiles);

    /** Get valid tiles for a direction */
    TArray<int32> GetValidNeighbors(int32 TileID, int32 Direction, const TArray<FForge2WFCTile>& Tiles);

    /** Check if WFC is complete */
    bool IsComplete(const TArray<FWFCCell>& Wave);

    /** Check if WFC has failed (contradiction) */
    bool HasContradiction(const TArray<FWFCCell>& Wave);

    /** Convert wave to dungeon data */
    void ConvertToDungeon(const TArray<FWFCCell>& Wave, int32 Width, int32 Height, const TArray<FForge2WFCTile>& Tiles, FForge2DungeonData& Dungeon);

    /** Direction constants */
    static constexpr int32 DIR_NORTH = 0;
    static constexpr int32 DIR_EAST = 1;
    static constexpr int32 DIR_SOUTH = 2;
    static constexpr int32 DIR_WEST = 3;
};

/**
 * Combined dungeon processor
 * Can apply multiple algorithms and post-processing
 */
class MCPAUTOMATIONBRIDGE_API FForge2DungeonProcessor
{
public:
    /**
     * Assign room purposes based on distance from entrance
     */
    static void AssignRoomPurposes(FForge2DungeonData& Dungeon, FRandomStream& Random);

    /**
     * Place doors at corridor-room intersections
     */
    static void PlaceDoors(FForge2DungeonData& Dungeon, FRandomStream& Random, float DoorChance = 0.8f);

    /**
     * Add secret passages between nearby rooms
     */
    static void AddSecretPassages(FForge2DungeonData& Dungeon, FRandomStream& Random, float SecretChance = 0.1f);

    /**
     * Generate spawn points for entities
     */
    static TArray<FIntPoint> GenerateSpawnPoints(const FForge2DungeonData& Dungeon, EForge2RoomPurpose RoomType, int32 PointsPerRoom = 3);

    /**
     * Add walls around floor tiles
     */
    static void GenerateWalls(FForge2DungeonData& Dungeon);

    /**
     * Smooth dungeon edges
     */
    static void SmoothEdges(FForge2DungeonData& Dungeon, int32 Iterations = 1);

    /**
     * Validate dungeon connectivity (all rooms reachable)
     */
    static bool ValidateConnectivity(const FForge2DungeonData& Dungeon);

    /**
     * Find path between two points (A* pathfinding)
     */
    static TArray<FIntPoint> FindPath(const FForge2DungeonData& Dungeon, const FIntPoint& Start, const FIntPoint& End);
};
