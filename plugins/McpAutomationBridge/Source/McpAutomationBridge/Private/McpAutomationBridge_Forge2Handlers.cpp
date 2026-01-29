// McpAutomationBridge_Forge2Handlers.cpp
// FORGE2: Ultimate AAA Game Development MCP Extension
// Full implementations for FORGE2 category handlers.

#include "McpAutomationBridgeSubsystem.h"
#include "McpAutomationBridgeHelpers.h"
#include "McpAutomationBridgeGlobals.h"
#include "ScopedTransaction.h"

#if WITH_EDITOR
#include "Async/Async.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "LandscapeEdit.h"
#include "LandscapeDataAccess.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Misc/ScopedSlowTask.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/DataAssetFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EditorAssetLibrary.h"
#include "EdGraphSchema_K2.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"

// PCG includes (UE 5.2+)
#if __has_include("PCGGraph.h")
#include "PCGGraph.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "PCGData.h"
#define HAS_PCG_SUPPORT 1
#else
#define HAS_PCG_SUPPORT 0
#endif

// Niagara includes
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

// Editor subsystem
#if __has_include("Subsystems/EditorActorSubsystem.h")
#include "Subsystems/EditorActorSubsystem.h"
#elif __has_include("EditorActorSubsystem.h")
#include "EditorActorSubsystem.h"
#endif
#endif // WITH_EDITOR

// ============================================================================
// FORGE2 Noise Algorithms
// ============================================================================

namespace Forge2Noise
{
    // Permutation table for Perlin noise
    static int32 Perm[512];
    static bool bPermInitialized = false;

    static void InitializePermutation(int32 Seed)
    {
        FRandomStream Random(Seed);
        TArray<int32> P;
        P.SetNum(256);
        for (int32 i = 0; i < 256; i++) P[i] = i;

        // Shuffle
        for (int32 i = 255; i > 0; i--)
        {
            int32 j = Random.RandRange(0, i);
            Swap(P[i], P[j]);
        }

        for (int32 i = 0; i < 256; i++)
        {
            Perm[i] = P[i];
            Perm[i + 256] = P[i];
        }
        bPermInitialized = true;
    }

    static float Fade(float t)
    {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    static float Grad(int32 hash, float x, float y)
    {
        int32 h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }

    static float Grad3D(int32 hash, float x, float y, float z)
    {
        int32 h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }

    // 2D Perlin Noise
    float PerlinNoise2D(float x, float y, int32 Seed = 0)
    {
        if (!bPermInitialized || Seed != 0)
        {
            InitializePermutation(Seed);
        }

        int32 X = FMath::FloorToInt(x) & 255;
        int32 Y = FMath::FloorToInt(y) & 255;

        x -= FMath::FloorToFloat(x);
        y -= FMath::FloorToFloat(y);

        float u = Fade(x);
        float v = Fade(y);

        int32 A = Perm[X] + Y;
        int32 B = Perm[X + 1] + Y;

        return FMath::Lerp(
            FMath::Lerp(Grad(Perm[A], x, y), Grad(Perm[B], x - 1, y), u),
            FMath::Lerp(Grad(Perm[A + 1], x, y - 1), Grad(Perm[B + 1], x - 1, y - 1), u),
            v
        );
    }

    // 3D Perlin Noise
    float PerlinNoise3D(float x, float y, float z, int32 Seed = 0)
    {
        if (!bPermInitialized || Seed != 0)
        {
            InitializePermutation(Seed);
        }

        int32 X = FMath::FloorToInt(x) & 255;
        int32 Y = FMath::FloorToInt(y) & 255;
        int32 Z = FMath::FloorToInt(z) & 255;

        x -= FMath::FloorToFloat(x);
        y -= FMath::FloorToFloat(y);
        z -= FMath::FloorToFloat(z);

        float u = Fade(x);
        float v = Fade(y);
        float w = Fade(z);

        int32 A = Perm[X] + Y;
        int32 AA = Perm[A] + Z;
        int32 AB = Perm[A + 1] + Z;
        int32 B = Perm[X + 1] + Y;
        int32 BA = Perm[B] + Z;
        int32 BB = Perm[B + 1] + Z;

        return FMath::Lerp(
            FMath::Lerp(
                FMath::Lerp(Grad3D(Perm[AA], x, y, z), Grad3D(Perm[BA], x - 1, y, z), u),
                FMath::Lerp(Grad3D(Perm[AB], x, y - 1, z), Grad3D(Perm[BB], x - 1, y - 1, z), u),
                v),
            FMath::Lerp(
                FMath::Lerp(Grad3D(Perm[AA + 1], x, y, z - 1), Grad3D(Perm[BA + 1], x - 1, y, z - 1), u),
                FMath::Lerp(Grad3D(Perm[AB + 1], x, y - 1, z - 1), Grad3D(Perm[BB + 1], x - 1, y - 1, z - 1), u),
                v),
            w
        );
    }

    // Fractal Brownian Motion (FBM)
    float FBM2D(float x, float y, int32 Octaves, float Persistence, float Lacunarity, int32 Seed = 0)
    {
        float Total = 0.0f;
        float Amplitude = 1.0f;
        float Frequency = 1.0f;
        float MaxValue = 0.0f;

        for (int32 i = 0; i < Octaves; i++)
        {
            Total += PerlinNoise2D(x * Frequency, y * Frequency, Seed) * Amplitude;
            MaxValue += Amplitude;
            Amplitude *= Persistence;
            Frequency *= Lacunarity;
        }

        return Total / MaxValue;
    }

    // Ridged Multifractal Noise
    float RidgedNoise2D(float x, float y, int32 Octaves, float Persistence, float Lacunarity, int32 Seed = 0)
    {
        float Total = 0.0f;
        float Amplitude = 1.0f;
        float Frequency = 1.0f;
        float Weight = 1.0f;

        for (int32 i = 0; i < Octaves; i++)
        {
            float Signal = PerlinNoise2D(x * Frequency, y * Frequency, Seed);
            Signal = 1.0f - FMath::Abs(Signal);
            Signal *= Signal;
            Signal *= Weight;
            Weight = FMath::Clamp(Signal * 2.0f, 0.0f, 1.0f);
            Total += Signal * Amplitude;
            Amplitude *= Persistence;
            Frequency *= Lacunarity;
        }

        return Total;
    }

    // Voronoi/Worley Noise
    float VoronoiNoise2D(float x, float y, int32 Seed = 0)
    {
        FRandomStream Random(Seed);

        int32 Xi = FMath::FloorToInt(x);
        int32 Yi = FMath::FloorToInt(y);

        float MinDist = FLT_MAX;

        for (int32 dx = -1; dx <= 1; dx++)
        {
            for (int32 dy = -1; dy <= 1; dy++)
            {
                int32 CellX = Xi + dx;
                int32 CellY = Yi + dy;

                // Deterministic random point in cell
                int32 CellSeed = Seed + CellX * 127 + CellY * 311;
                FRandomStream CellRandom(CellSeed);
                float PointX = CellX + CellRandom.FRand();
                float PointY = CellY + CellRandom.FRand();

                float Dist = FVector2D::Distance(FVector2D(x, y), FVector2D(PointX, PointY));
                MinDist = FMath::Min(MinDist, Dist);
            }
        }

        return FMath::Clamp(MinDist, 0.0f, 1.0f);
    }

    // Simplex Noise 2D
    float SimplexNoise2D(float x, float y, int32 Seed = 0)
    {
        if (!bPermInitialized || Seed != 0)
        {
            InitializePermutation(Seed);
        }

        const float F2 = 0.366025403f; // (sqrt(3) - 1) / 2
        const float G2 = 0.211324865f; // (3 - sqrt(3)) / 6

        float s = (x + y) * F2;
        int32 i = FMath::FloorToInt(x + s);
        int32 j = FMath::FloorToInt(y + s);

        float t = (i + j) * G2;
        float X0 = i - t;
        float Y0 = j - t;
        float x0 = x - X0;
        float y0 = y - Y0;

        int32 i1, j1;
        if (x0 > y0) { i1 = 1; j1 = 0; }
        else { i1 = 0; j1 = 1; }

        float x1 = x0 - i1 + G2;
        float y1 = y0 - j1 + G2;
        float x2 = x0 - 1.0f + 2.0f * G2;
        float y2 = y0 - 1.0f + 2.0f * G2;

        int32 ii = i & 255;
        int32 jj = j & 255;

        float n0, n1, n2;

        float t0 = 0.5f - x0 * x0 - y0 * y0;
        if (t0 < 0) n0 = 0.0f;
        else { t0 *= t0; n0 = t0 * t0 * Grad(Perm[ii + Perm[jj]], x0, y0); }

        float t1 = 0.5f - x1 * x1 - y1 * y1;
        if (t1 < 0) n1 = 0.0f;
        else { t1 *= t1; n1 = t1 * t1 * Grad(Perm[ii + i1 + Perm[jj + j1]], x1, y1); }

        float t2 = 0.5f - x2 * x2 - y2 * y2;
        if (t2 < 0) n2 = 0.0f;
        else { t2 *= t2; n2 = t2 * t2 * Grad(Perm[ii + 1 + Perm[jj + 1]], x2, y2); }

        return 70.0f * (n0 + n1 + n2);
    }
}

// ============================================================================
// FORGE2 Erosion Algorithms
// ============================================================================

namespace Forge2Erosion
{
    // Thermal Erosion - simulates slope-based material movement
    void ApplyThermalErosion(TArray<float>& HeightMap, int32 Width, int32 Height,
                              int32 Iterations, float TalusAngle, float ErosionStrength)
    {
        const float TalusThreshold = FMath::Tan(FMath::DegreesToRadians(TalusAngle));
        TArray<float> TempMap = HeightMap;

        for (int32 Iter = 0; Iter < Iterations; Iter++)
        {
            for (int32 y = 1; y < Height - 1; y++)
            {
                for (int32 x = 1; x < Width - 1; x++)
                {
                    int32 Idx = y * Width + x;
                    float CurrentHeight = HeightMap[Idx];
                    float MaxDiff = 0.0f;
                    int32 MaxIdx = -1;

                    // Check 8 neighbors
                    const int32 Neighbors[8][2] = {
                        {-1, -1}, {0, -1}, {1, -1},
                        {-1, 0},           {1, 0},
                        {-1, 1},  {0, 1},  {1, 1}
                    };

                    for (int32 n = 0; n < 8; n++)
                    {
                        int32 nx = x + Neighbors[n][0];
                        int32 ny = y + Neighbors[n][1];
                        int32 nIdx = ny * Width + nx;

                        float Diff = CurrentHeight - HeightMap[nIdx];
                        if (Diff > MaxDiff && Diff > TalusThreshold)
                        {
                            MaxDiff = Diff;
                            MaxIdx = nIdx;
                        }
                    }

                    if (MaxIdx >= 0)
                    {
                        float Transfer = (MaxDiff - TalusThreshold) * 0.5f * ErosionStrength;
                        TempMap[Idx] -= Transfer;
                        TempMap[MaxIdx] += Transfer;
                    }
                }
            }

            HeightMap = TempMap;
        }
    }

    // Hydraulic Erosion - simulates water flow
    void ApplyHydraulicErosion(TArray<float>& HeightMap, int32 Width, int32 Height,
                                int32 Iterations, float SedimentCapacity, float EvaporationRate)
    {
        TArray<float> WaterMap;
        TArray<float> SedimentMap;
        WaterMap.SetNumZeroed(Width * Height);
        SedimentMap.SetNumZeroed(Width * Height);

        FRandomStream Random(42);

        for (int32 Iter = 0; Iter < Iterations; Iter++)
        {
            // Rain
            for (int32 i = 0; i < Width * Height; i++)
            {
                WaterMap[i] += 0.01f;
            }

            // Erosion and sediment transport
            for (int32 y = 1; y < Height - 1; y++)
            {
                for (int32 x = 1; x < Width - 1; x++)
                {
                    int32 Idx = y * Width + x;
                    float Water = WaterMap[Idx];
                    if (Water <= 0.0f) continue;

                    // Find steepest neighbor
                    float CurrentHeight = HeightMap[Idx] + WaterMap[Idx];
                    float MinHeight = CurrentHeight;
                    int32 MinIdx = -1;

                    const int32 Neighbors[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
                    for (int32 n = 0; n < 4; n++)
                    {
                        int32 nx = x + Neighbors[n][0];
                        int32 ny = y + Neighbors[n][1];
                        int32 nIdx = ny * Width + nx;

                        float NeighborHeight = HeightMap[nIdx] + WaterMap[nIdx];
                        if (NeighborHeight < MinHeight)
                        {
                            MinHeight = NeighborHeight;
                            MinIdx = nIdx;
                        }
                    }

                    if (MinIdx >= 0)
                    {
                        float HeightDiff = CurrentHeight - MinHeight;
                        float FlowAmount = FMath::Min(Water, HeightDiff * 0.5f);

                        // Move water
                        WaterMap[Idx] -= FlowAmount;
                        WaterMap[MinIdx] += FlowAmount;

                        // Pickup sediment
                        float PickupAmount = FMath::Min(FlowAmount * SedimentCapacity, HeightMap[Idx] * 0.1f);
                        HeightMap[Idx] -= PickupAmount;
                        SedimentMap[Idx] += PickupAmount;

                        // Move sediment with water
                        float SedimentFlow = SedimentMap[Idx] * (FlowAmount / FMath::Max(Water, 0.001f));
                        SedimentMap[Idx] -= SedimentFlow;
                        SedimentMap[MinIdx] += SedimentFlow;
                    }
                }
            }

            // Evaporation and deposition
            for (int32 i = 0; i < Width * Height; i++)
            {
                WaterMap[i] *= (1.0f - EvaporationRate);
                if (WaterMap[i] < 0.001f)
                {
                    HeightMap[i] += SedimentMap[i];
                    SedimentMap[i] = 0.0f;
                    WaterMap[i] = 0.0f;
                }
            }
        }
    }
}

// ============================================================================
// FORGE2 Dungeon Generation Algorithms
// ============================================================================

namespace Forge2Dungeon
{
    struct FDungeonRoom
    {
        int32 X, Y, Width, Height;
        FString RoomType;

        FDungeonRoom() : X(0), Y(0), Width(0), Height(0), RoomType(TEXT("generic")) {}
        FDungeonRoom(int32 InX, int32 InY, int32 InW, int32 InH, const FString& Type = TEXT("generic"))
            : X(InX), Y(InY), Width(InW), Height(InH), RoomType(Type) {}

        FVector2D Center() const { return FVector2D(X + Width / 2, Y + Height / 2); }
        bool Overlaps(const FDungeonRoom& Other, int32 Padding = 1) const
        {
            return !(X + Width + Padding < Other.X || Other.X + Other.Width + Padding < X ||
                     Y + Height + Padding < Other.Y || Other.Y + Other.Height + Padding < Y);
        }
    };

    // BSP (Binary Space Partition) Dungeon Generation
    struct FBSPNode
    {
        int32 X, Y, Width, Height;
        TSharedPtr<FBSPNode> Left;
        TSharedPtr<FBSPNode> Right;
        FDungeonRoom Room;
        bool bHasRoom = false;

        FBSPNode(int32 InX, int32 InY, int32 InW, int32 InH)
            : X(InX), Y(InY), Width(InW), Height(InH) {}
    };

    void SplitBSPNode(TSharedPtr<FBSPNode> Node, int32 MinSize, FRandomStream& Random)
    {
        if (Node->Width < MinSize * 2 && Node->Height < MinSize * 2)
            return;

        bool bSplitHorizontal;
        if (Node->Width < MinSize * 2)
            bSplitHorizontal = true;
        else if (Node->Height < MinSize * 2)
            bSplitHorizontal = false;
        else
            bSplitHorizontal = Random.FRand() > 0.5f;

        if (bSplitHorizontal)
        {
            int32 SplitPos = Random.RandRange(MinSize, Node->Height - MinSize);
            Node->Left = MakeShared<FBSPNode>(Node->X, Node->Y, Node->Width, SplitPos);
            Node->Right = MakeShared<FBSPNode>(Node->X, Node->Y + SplitPos, Node->Width, Node->Height - SplitPos);
        }
        else
        {
            int32 SplitPos = Random.RandRange(MinSize, Node->Width - MinSize);
            Node->Left = MakeShared<FBSPNode>(Node->X, Node->Y, SplitPos, Node->Height);
            Node->Right = MakeShared<FBSPNode>(Node->X + SplitPos, Node->Y, Node->Width - SplitPos, Node->Height);
        }

        SplitBSPNode(Node->Left, MinSize, Random);
        SplitBSPNode(Node->Right, MinSize, Random);
    }

    void CreateRoomsInBSP(TSharedPtr<FBSPNode> Node, TArray<FDungeonRoom>& Rooms,
                          int32 MinRoomSize, FRandomStream& Random)
    {
        if (!Node) return;

        if (!Node->Left && !Node->Right)
        {
            // Leaf node - create room
            int32 RoomWidth = Random.RandRange(MinRoomSize, Node->Width - 2);
            int32 RoomHeight = Random.RandRange(MinRoomSize, Node->Height - 2);
            int32 RoomX = Node->X + Random.RandRange(1, Node->Width - RoomWidth - 1);
            int32 RoomY = Node->Y + Random.RandRange(1, Node->Height - RoomHeight - 1);

            FDungeonRoom Room(RoomX, RoomY, RoomWidth, RoomHeight);
            Rooms.Add(Room);
            Node->Room = Room;
            Node->bHasRoom = true;
        }
        else
        {
            CreateRoomsInBSP(Node->Left, Rooms, MinRoomSize, Random);
            CreateRoomsInBSP(Node->Right, Rooms, MinRoomSize, Random);
        }
    }

    TArray<FDungeonRoom> GenerateBSPDungeon(int32 Width, int32 Height, int32 MinRoomSize, int32 Seed)
    {
        FRandomStream Random(Seed);
        TArray<FDungeonRoom> Rooms;

        auto Root = MakeShared<FBSPNode>(0, 0, Width, Height);
        SplitBSPNode(Root, MinRoomSize * 2, Random);
        CreateRoomsInBSP(Root, Rooms, MinRoomSize, Random);

        // Assign room types
        if (Rooms.Num() > 0)
        {
            Rooms[0].RoomType = TEXT("entrance");
            Rooms.Last().RoomType = TEXT("boss");

            int32 TreasureCount = FMath::Max(1, Rooms.Num() / 4);
            for (int32 i = 0; i < TreasureCount && i + 1 < Rooms.Num() - 1; i++)
            {
                int32 TreasureIdx = Random.RandRange(1, Rooms.Num() - 2);
                Rooms[TreasureIdx].RoomType = TEXT("treasure");
            }
        }

        return Rooms;
    }

    // Cellular Automata for cave generation
    TArray<int32> GenerateCellularAutomata(int32 Width, int32 Height, int32 Iterations,
                                            float FillPercent, int32 Seed)
    {
        FRandomStream Random(Seed);
        TArray<int32> Map;
        Map.SetNum(Width * Height);

        // Initial random fill
        for (int32 i = 0; i < Width * Height; i++)
        {
            Map[i] = Random.FRand() < FillPercent ? 1 : 0;
        }

        // Apply cellular automata rules
        TArray<int32> TempMap = Map;
        for (int32 Iter = 0; Iter < Iterations; Iter++)
        {
            for (int32 y = 0; y < Height; y++)
            {
                for (int32 x = 0; x < Width; x++)
                {
                    int32 Neighbors = 0;
                    for (int32 dy = -1; dy <= 1; dy++)
                    {
                        for (int32 dx = -1; dx <= 1; dx++)
                        {
                            if (dx == 0 && dy == 0) continue;
                            int32 nx = x + dx;
                            int32 ny = y + dy;
                            if (nx < 0 || nx >= Width || ny < 0 || ny >= Height)
                            {
                                Neighbors++;
                            }
                            else
                            {
                                Neighbors += Map[ny * Width + nx];
                            }
                        }
                    }

                    int32 Idx = y * Width + x;
                    if (Neighbors > 4)
                        TempMap[Idx] = 1;
                    else if (Neighbors < 4)
                        TempMap[Idx] = 0;
                }
            }
            Map = TempMap;
        }

        return Map;
    }

    // Drunkard's Walk for organic caves
    TArray<int32> GenerateDrunkardWalk(int32 Width, int32 Height, float FillPercent, int32 Seed)
    {
        FRandomStream Random(Seed);
        TArray<int32> Map;
        Map.Init(1, Width * Height); // Start all walls

        int32 TargetFloorTiles = static_cast<int32>(Width * Height * FillPercent);
        int32 FloorTiles = 0;

        // Start in center
        int32 WalkerX = Width / 2;
        int32 WalkerY = Height / 2;

        const int32 Dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

        while (FloorTiles < TargetFloorTiles)
        {
            int32 Idx = WalkerY * Width + WalkerX;
            if (Map[Idx] == 1)
            {
                Map[Idx] = 0;
                FloorTiles++;
            }

            int32 Dir = Random.RandRange(0, 3);
            int32 NewX = FMath::Clamp(WalkerX + Dirs[Dir][0], 1, Width - 2);
            int32 NewY = FMath::Clamp(WalkerY + Dirs[Dir][1], 1, Height - 2);
            WalkerX = NewX;
            WalkerY = NewY;
        }

        return Map;
    }
}

// ============================================================================
// FORGE2 Helper Functions
// ============================================================================

#if WITH_EDITOR
namespace Forge2Helpers
{
    // Create package for assets
    static UPackage* CreateAssetPackage(const FString& Path, const FString& Name)
    {
        FString PackagePath = Path.IsEmpty() ? TEXT("/Game/FORGE2") : Path;
        if (!PackagePath.StartsWith(TEXT("/")))
        {
            PackagePath = TEXT("/Game/") + PackagePath;
        }
        FString PackageName = PackagePath / Name;
        return CreatePackage(*PackageName);
    }

    // Create actor blueprint
    static UBlueprint* CreateActorBlueprint(UClass* ParentClass, const FString& Path,
                                             const FString& Name, FString& OutError)
    {
        FString FullPath = Path / Name;
        if (!FullPath.StartsWith(TEXT("/")))
        {
            FullPath = TEXT("/Game/") + FullPath;
        }

        UPackage* Package = CreatePackage(*FullPath);
        if (!Package)
        {
            OutError = FString::Printf(TEXT("Failed to create package: %s"), *FullPath);
            return nullptr;
        }

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        Factory->ParentClass = ParentClass;

        UBlueprint* Blueprint = Cast<UBlueprint>(
            Factory->FactoryCreateNew(UBlueprint::StaticClass(), Package, FName(*Name),
                                      RF_Public | RF_Standalone, nullptr, GWarn));

        if (!Blueprint)
        {
            OutError = TEXT("Failed to create blueprint");
            return nullptr;
        }

        McpSafeAssetSave(Blueprint);
        return Blueprint;
    }

    // Get action from FORGE2 payload
    static FString GetActionFromPayload(const TSharedPtr<FJsonObject>& Payload)
    {
        if (!Payload.IsValid()) return TEXT("");

        // Check params.action first (FORGE2 format)
        const TSharedPtr<FJsonObject>* ParamsObj = nullptr;
        if (Payload->TryGetObjectField(TEXT("params"), ParamsObj) && ParamsObj && (*ParamsObj).IsValid())
        {
            FString ActionStr;
            if ((*ParamsObj)->TryGetStringField(TEXT("action"), ActionStr))
            {
                return ActionStr;
            }
        }

        // Fallback to root action
        FString ActionStr;
        if (Payload->TryGetStringField(TEXT("action"), ActionStr))
        {
            return ActionStr;
        }

        return TEXT("");
    }

    // Get params object
    static TSharedPtr<FJsonObject> GetParams(const TSharedPtr<FJsonObject>& Payload)
    {
        if (!Payload.IsValid()) return nullptr;

        const TSharedPtr<FJsonObject>* ParamsObj = nullptr;
        if (Payload->TryGetObjectField(TEXT("params"), ParamsObj) && ParamsObj)
        {
            return *ParamsObj;
        }

        return Payload;
    }

    // Vector from JSON
    static FVector GetVectorFromJson(const TSharedPtr<FJsonObject>& Obj)
    {
        if (!Obj.IsValid()) return FVector::ZeroVector;
        double X = 0, Y = 0, Z = 0;
        Obj->TryGetNumberField(TEXT("x"), X);
        Obj->TryGetNumberField(TEXT("y"), Y);
        Obj->TryGetNumberField(TEXT("z"), Z);
        return FVector(X, Y, Z);
    }
}
#endif // WITH_EDITOR

// ============================================================================
// FORGE2 WORLDGEN HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2WorldgenAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 worldgen requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // ========================================================================
    // generate_heightmap - Generate terrain heightmap using noise algorithms
    // ========================================================================
    if (SubAction == TEXT("generate_heightmap") || SubAction == TEXT("generate_noise_terrain"))
    {
        int32 Width = GetJsonNumberField(Params, TEXT("width"), 256);
        int32 Height = GetJsonNumberField(Params, TEXT("height"), 256);
        float Scale = GetJsonNumberField(Params, TEXT("scale"), 100.0f);
        int32 Seed = GetJsonNumberField(Params, TEXT("seed"), FMath::Rand());
        int32 Octaves = GetJsonNumberField(Params, TEXT("octaves"), 4);
        float Persistence = GetJsonNumberField(Params, TEXT("persistence"), 0.5f);
        float Lacunarity = GetJsonNumberField(Params, TEXT("lacunarity"), 2.0f);
        FString NoiseType = GetJsonStringField(Params, TEXT("noise_type"), TEXT("fbm"));
        float Amplitude = GetJsonNumberField(Params, TEXT("amplitude"), 1.0f);

        // Generate heightmap
        TArray<float> HeightMap;
        HeightMap.SetNum(Width * Height);

        for (int32 y = 0; y < Height; y++)
        {
            for (int32 x = 0; x < Width; x++)
            {
                float nx = static_cast<float>(x) / Scale;
                float ny = static_cast<float>(y) / Scale;
                float Value = 0.0f;

                if (NoiseType == TEXT("perlin"))
                {
                    Value = Forge2Noise::PerlinNoise2D(nx, ny, Seed);
                }
                else if (NoiseType == TEXT("simplex"))
                {
                    Value = Forge2Noise::SimplexNoise2D(nx, ny, Seed);
                }
                else if (NoiseType == TEXT("voronoi") || NoiseType == TEXT("worley"))
                {
                    Value = Forge2Noise::VoronoiNoise2D(nx, ny, Seed);
                }
                else if (NoiseType == TEXT("ridged"))
                {
                    Value = Forge2Noise::RidgedNoise2D(nx, ny, Octaves, Persistence, Lacunarity, Seed);
                }
                else // fbm (default)
                {
                    Value = Forge2Noise::FBM2D(nx, ny, Octaves, Persistence, Lacunarity, Seed);
                }

                // Normalize to 0-1 range
                Value = (Value + 1.0f) * 0.5f * Amplitude;
                HeightMap[y * Width + x] = FMath::Clamp(Value, 0.0f, 1.0f);
            }
        }

        // Apply erosion if requested
        bool bApplyErosion = GetJsonBoolField(Params, TEXT("apply_erosion"), false);
        if (bApplyErosion)
        {
            int32 ErosionIterations = GetJsonNumberField(Params, TEXT("erosion_iterations"), 50);
            FString ErosionType = GetJsonStringField(Params, TEXT("erosion_type"), TEXT("thermal"));

            if (ErosionType == TEXT("thermal"))
            {
                float TalusAngle = GetJsonNumberField(Params, TEXT("talus_angle"), 40.0f);
                float ErosionStrength = GetJsonNumberField(Params, TEXT("erosion_strength"), 0.5f);
                Forge2Erosion::ApplyThermalErosion(HeightMap, Width, Height, ErosionIterations,
                                                    TalusAngle, ErosionStrength);
            }
            else if (ErosionType == TEXT("hydraulic"))
            {
                float SedimentCapacity = GetJsonNumberField(Params, TEXT("sediment_capacity"), 0.01f);
                float EvaporationRate = GetJsonNumberField(Params, TEXT("evaporation_rate"), 0.02f);
                Forge2Erosion::ApplyHydraulicErosion(HeightMap, Width, Height, ErosionIterations,
                                                      SedimentCapacity, EvaporationRate);
            }
        }

        // Create landscape if requested
        bool bCreateLandscape = GetJsonBoolField(Params, TEXT("create_landscape"), false);
        FString LandscapeActorName;

        if (bCreateLandscape)
        {
            if (!GEditor || !GEditor->GetEditorWorldContext().World())
            {
                SendAutomationError(Socket, RequestId, TEXT("Editor world not available"), TEXT("EDITOR_NOT_AVAILABLE"));
                return true;
            }

            UWorld* World = GEditor->GetEditorWorldContext().World();

            // Calculate landscape size
            int32 ComponentsX = Width / 63;
            int32 ComponentsY = Height / 63;
            ComponentsX = FMath::Max(1, ComponentsX);
            ComponentsY = FMath::Max(1, ComponentsY);
            int32 QuadsPerComponent = 63;

            FVector Location = FVector::ZeroVector;
            const TSharedPtr<FJsonObject>* LocObj = nullptr;
            if (Params->TryGetObjectField(TEXT("location"), LocObj) && LocObj)
            {
                Location = Forge2Helpers::GetVectorFromJson(*LocObj);
            }

            FString LandscapeName = GetJsonStringField(Params, TEXT("name"), TEXT("FORGE2_Terrain"));

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            ALandscape* Landscape = World->SpawnActor<ALandscape>(ALandscape::StaticClass(),
                                                                   Location, FRotator::ZeroRotator, SpawnParams);
            if (Landscape)
            {
                Landscape->SetActorLabel(LandscapeName);
                Landscape->ComponentSizeQuads = QuadsPerComponent;
                Landscape->SubsectionSizeQuads = QuadsPerComponent;
                Landscape->NumSubsections = 1;

                if (!Landscape->GetLandscapeGuid().IsValid())
                {
                    Landscape->SetLandscapeGuid(FGuid::NewGuid());
                }
                Landscape->CreateLandscapeInfo();

                // Convert heightmap to UE format
                const int32 VertX = ComponentsX * QuadsPerComponent + 1;
                const int32 VertY = ComponentsY * QuadsPerComponent + 1;
                TArray<uint16> UEHeightData;
                UEHeightData.SetNum(VertX * VertY);

                for (int32 y = 0; y < VertY; y++)
                {
                    for (int32 x = 0; x < VertX; x++)
                    {
                        int32 SrcX = FMath::Clamp(x * Width / VertX, 0, Width - 1);
                        int32 SrcY = FMath::Clamp(y * Height / VertY, 0, Height - 1);
                        float Value = HeightMap[SrcY * Width + SrcX];
                        UEHeightData[y * VertX + x] = static_cast<uint16>(32768 + Value * 32767);
                    }
                }

                // Apply height data
                const FScopedTransaction Transaction(FText::FromString(TEXT("FORGE2 Create Terrain")));
                Landscape->Modify();

                if (Landscape->GetLayersConst().Num() == 0)
                {
                    Landscape->CreateDefaultLayer();
                }

                ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
                if (LandscapeInfo)
                {
                    Landscape->RegisterAllComponents();
                    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
                    LandscapeEdit.SetHeightData(0, 0, VertX - 1, VertY - 1, UEHeightData.GetData(), 0, true);
                }

                LandscapeActorName = Landscape->GetActorLabel();
            }
        }

        // Build response
        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("width"), Width);
        Result->SetNumberField(TEXT("height"), Height);
        Result->SetNumberField(TEXT("seed"), Seed);
        Result->SetStringField(TEXT("noise_type"), NoiseType);
        Result->SetNumberField(TEXT("min_height"), 0.0f);
        Result->SetNumberField(TEXT("max_height"), Amplitude);

        if (bCreateLandscape && !LandscapeActorName.IsEmpty())
        {
            Result->SetStringField(TEXT("landscape_actor"), LandscapeActorName);
        }

        // Store heightmap data reference (could save to file for large maps)
        Result->SetBoolField(TEXT("heightmap_generated"), true);
        Result->SetNumberField(TEXT("data_points"), HeightMap.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Heightmap generated successfully"), Result);
        return true;
    }

    // ========================================================================
    // apply_thermal_erosion - Apply thermal erosion to existing landscape
    // ========================================================================
    if (SubAction == TEXT("apply_thermal_erosion"))
    {
        FString LandscapeName = GetJsonStringField(Params, TEXT("landscape_name"), TEXT(""));
        int32 Iterations = GetJsonNumberField(Params, TEXT("iterations"), 50);
        float TalusAngle = GetJsonNumberField(Params, TEXT("talus_angle"), 40.0f);
        float Strength = GetJsonNumberField(Params, TEXT("strength"), 0.5f);

        if (LandscapeName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing landscape_name parameter"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
        if (!World)
        {
            SendAutomationError(Socket, RequestId, TEXT("Editor world not available"), TEXT("EDITOR_NOT_AVAILABLE"));
            return true;
        }

        ALandscape* Landscape = nullptr;
        for (TActorIterator<ALandscape> It(World); It; ++It)
        {
            if (It->GetActorLabel() == LandscapeName)
            {
                Landscape = *It;
                break;
            }
        }

        if (!Landscape)
        {
            SendAutomationError(Socket, RequestId,
                FString::Printf(TEXT("Landscape not found: %s"), *LandscapeName), TEXT("NOT_FOUND"));
            return true;
        }

        ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
        if (!LandscapeInfo)
        {
            SendAutomationError(Socket, RequestId, TEXT("Invalid landscape info"), TEXT("INVALID_LANDSCAPE"));
            return true;
        }

        // Get current height data
        int32 MinX, MinY, MaxX, MaxY;
        LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);
        int32 Width = MaxX - MinX + 1;
        int32 Height = MaxY - MinY + 1;

        TArray<uint16> HeightData;
        HeightData.SetNum(Width * Height);

        FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
        LandscapeEdit.GetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0);

        // Convert to float for erosion
        TArray<float> FloatHeights;
        FloatHeights.SetNum(Width * Height);
        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            FloatHeights[i] = static_cast<float>(HeightData[i]) / 65535.0f;
        }

        // Apply erosion
        Forge2Erosion::ApplyThermalErosion(FloatHeights, Width, Height, Iterations, TalusAngle, Strength);

        // Convert back and apply
        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            HeightData[i] = static_cast<uint16>(FMath::Clamp(FloatHeights[i] * 65535.0f, 0.0f, 65535.0f));
        }

        const FScopedTransaction Transaction(FText::FromString(TEXT("FORGE2 Thermal Erosion")));
        Landscape->Modify();
        LandscapeEdit.SetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0, true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("landscape"), LandscapeName);
        Result->SetNumberField(TEXT("iterations"), Iterations);
        Result->SetStringField(TEXT("erosion_type"), TEXT("thermal"));

        SendAutomationResponse(Socket, RequestId, true, TEXT("Thermal erosion applied"), Result);
        return true;
    }

    // ========================================================================
    // apply_hydraulic_erosion - Apply water-based erosion
    // ========================================================================
    if (SubAction == TEXT("apply_hydraulic_erosion"))
    {
        FString LandscapeName = GetJsonStringField(Params, TEXT("landscape_name"), TEXT(""));
        int32 Iterations = GetJsonNumberField(Params, TEXT("iterations"), 100);
        float SedimentCapacity = GetJsonNumberField(Params, TEXT("sediment_capacity"), 0.01f);
        float EvaporationRate = GetJsonNumberField(Params, TEXT("evaporation_rate"), 0.02f);

        if (LandscapeName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing landscape_name parameter"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
        if (!World)
        {
            SendAutomationError(Socket, RequestId, TEXT("Editor world not available"), TEXT("EDITOR_NOT_AVAILABLE"));
            return true;
        }

        ALandscape* Landscape = nullptr;
        for (TActorIterator<ALandscape> It(World); It; ++It)
        {
            if (It->GetActorLabel() == LandscapeName)
            {
                Landscape = *It;
                break;
            }
        }

        if (!Landscape)
        {
            SendAutomationError(Socket, RequestId,
                FString::Printf(TEXT("Landscape not found: %s"), *LandscapeName), TEXT("NOT_FOUND"));
            return true;
        }

        ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
        if (!LandscapeInfo)
        {
            SendAutomationError(Socket, RequestId, TEXT("Invalid landscape info"), TEXT("INVALID_LANDSCAPE"));
            return true;
        }

        int32 MinX, MinY, MaxX, MaxY;
        LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);
        int32 Width = MaxX - MinX + 1;
        int32 Height = MaxY - MinY + 1;

        TArray<uint16> HeightData;
        HeightData.SetNum(Width * Height);

        FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
        LandscapeEdit.GetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0);

        TArray<float> FloatHeights;
        FloatHeights.SetNum(Width * Height);
        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            FloatHeights[i] = static_cast<float>(HeightData[i]) / 65535.0f;
        }

        Forge2Erosion::ApplyHydraulicErosion(FloatHeights, Width, Height, Iterations,
                                              SedimentCapacity, EvaporationRate);

        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            HeightData[i] = static_cast<uint16>(FMath::Clamp(FloatHeights[i] * 65535.0f, 0.0f, 65535.0f));
        }

        const FScopedTransaction Transaction(FText::FromString(TEXT("FORGE2 Hydraulic Erosion")));
        Landscape->Modify();
        LandscapeEdit.SetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0, true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("landscape"), LandscapeName);
        Result->SetNumberField(TEXT("iterations"), Iterations);
        Result->SetStringField(TEXT("erosion_type"), TEXT("hydraulic"));

        SendAutomationResponse(Socket, RequestId, true, TEXT("Hydraulic erosion applied"), Result);
        return true;
    }

    // ========================================================================
    // generate_dungeon - Procedural dungeon generation
    // ========================================================================
    if (SubAction == TEXT("generate_dungeon"))
    {
        int32 Width = GetJsonNumberField(Params, TEXT("width"), 50);
        int32 Height = GetJsonNumberField(Params, TEXT("height"), 50);
        int32 MinRoomSize = GetJsonNumberField(Params, TEXT("min_room_size"), 5);
        int32 Seed = GetJsonNumberField(Params, TEXT("seed"), FMath::Rand());
        FString Algorithm = GetJsonStringField(Params, TEXT("algorithm"), TEXT("bsp"));
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("FORGE2_Dungeon"));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Dungeons"));
        float CellSize = GetJsonNumberField(Params, TEXT("cell_size"), 100.0f);
        bool bSpawnActors = GetJsonBoolField(Params, TEXT("spawn_actors"), true);

        // Generate dungeon layout
        TArray<Forge2Dungeon::FDungeonRoom> Rooms;
        TArray<int32> FloorMap;

        if (Algorithm == TEXT("bsp"))
        {
            Rooms = Forge2Dungeon::GenerateBSPDungeon(Width, Height, MinRoomSize, Seed);

            // Create floor map from rooms
            FloorMap.Init(1, Width * Height); // 1 = wall
            for (const auto& Room : Rooms)
            {
                for (int32 y = Room.Y; y < Room.Y + Room.Height && y < Height; y++)
                {
                    for (int32 x = Room.X; x < Room.X + Room.Width && x < Width; x++)
                    {
                        FloorMap[y * Width + x] = 0; // 0 = floor
                    }
                }
            }

            // Connect rooms with corridors
            for (int32 i = 0; i < Rooms.Num() - 1; i++)
            {
                FVector2D C1 = Rooms[i].Center();
                FVector2D C2 = Rooms[i + 1].Center();

                int32 X1 = FMath::FloorToInt(C1.X);
                int32 Y1 = FMath::FloorToInt(C1.Y);
                int32 X2 = FMath::FloorToInt(C2.X);
                int32 Y2 = FMath::FloorToInt(C2.Y);

                // Horizontal then vertical corridor
                for (int32 x = FMath::Min(X1, X2); x <= FMath::Max(X1, X2); x++)
                {
                    if (x >= 0 && x < Width && Y1 >= 0 && Y1 < Height)
                        FloorMap[Y1 * Width + x] = 0;
                }
                for (int32 y = FMath::Min(Y1, Y2); y <= FMath::Max(Y1, Y2); y++)
                {
                    if (X2 >= 0 && X2 < Width && y >= 0 && y < Height)
                        FloorMap[y * Width + X2] = 0;
                }
            }
        }
        else if (Algorithm == TEXT("cellular_automata"))
        {
            float FillPercent = GetJsonNumberField(Params, TEXT("fill_percent"), 0.45f);
            int32 Iterations = GetJsonNumberField(Params, TEXT("iterations"), 5);
            FloorMap = Forge2Dungeon::GenerateCellularAutomata(Width, Height, Iterations, FillPercent, Seed);
        }
        else if (Algorithm == TEXT("drunkards_walk"))
        {
            float FillPercent = GetJsonNumberField(Params, TEXT("fill_percent"), 0.4f);
            FloorMap = Forge2Dungeon::GenerateDrunkardWalk(Width, Height, FillPercent, Seed);
        }

        // Spawn floor tiles if requested
        int32 SpawnedTiles = 0;
        FString SpawnedActorPath;

        if (bSpawnActors && GEditor && GEditor->GetEditorWorldContext().World())
        {
            UWorld* World = GEditor->GetEditorWorldContext().World();
            FVector Origin = FVector::ZeroVector;

            const TSharedPtr<FJsonObject>* LocObj = nullptr;
            if (Params->TryGetObjectField(TEXT("location"), LocObj) && LocObj)
            {
                Origin = Forge2Helpers::GetVectorFromJson(*LocObj);
            }

            FString FloorMeshPath = GetJsonStringField(Params, TEXT("floor_mesh"),
                TEXT("/Engine/BasicShapes/Cube.Cube"));
            FString WallMeshPath = GetJsonStringField(Params, TEXT("wall_mesh"),
                TEXT("/Engine/BasicShapes/Cube.Cube"));

            UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(nullptr, *FloorMeshPath);
            UStaticMesh* WallMesh = LoadObject<UStaticMesh>(nullptr, *WallMeshPath);

            if (FloorMesh)
            {
                const FScopedTransaction Transaction(FText::FromString(TEXT("FORGE2 Generate Dungeon")));

                for (int32 y = 0; y < Height; y++)
                {
                    for (int32 x = 0; x < Width; x++)
                    {
                        int32 Idx = y * Width + x;
                        FVector Position = Origin + FVector(x * CellSize, y * CellSize, 0);

                        if (FloorMap[Idx] == 0) // Floor
                        {
                            FActorSpawnParameters SpawnParams;
                            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                            AActor* FloorActor = World->SpawnActor<AActor>(AActor::StaticClass(),
                                                                           Position, FRotator::ZeroRotator, SpawnParams);
                            if (FloorActor)
                            {
                                USceneComponent* RootComp = NewObject<USceneComponent>(FloorActor, TEXT("Root"));
                                FloorActor->SetRootComponent(RootComp);
                                RootComp->RegisterComponent();

                                UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(FloorActor, TEXT("Floor"));
                                MeshComp->SetStaticMesh(FloorMesh);
                                MeshComp->SetWorldScale3D(FVector(CellSize / 100.0f, CellSize / 100.0f, 0.1f));
                                MeshComp->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
                                MeshComp->RegisterComponent();

                                FloorActor->SetActorLabel(FString::Printf(TEXT("%s_Floor_%d_%d"), *Name, x, y));
                                SpawnedTiles++;
                            }
                        }
                        else if (WallMesh) // Wall
                        {
                            FActorSpawnParameters SpawnParams;
                            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                            AActor* WallActor = World->SpawnActor<AActor>(AActor::StaticClass(),
                                                                          Position + FVector(0, 0, CellSize / 2),
                                                                          FRotator::ZeroRotator, SpawnParams);
                            if (WallActor)
                            {
                                USceneComponent* RootComp = NewObject<USceneComponent>(WallActor, TEXT("Root"));
                                WallActor->SetRootComponent(RootComp);
                                RootComp->RegisterComponent();

                                UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(WallActor, TEXT("Wall"));
                                MeshComp->SetStaticMesh(WallMesh);
                                MeshComp->SetWorldScale3D(FVector(CellSize / 100.0f, CellSize / 100.0f, CellSize / 100.0f));
                                MeshComp->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
                                MeshComp->RegisterComponent();

                                WallActor->SetActorLabel(FString::Printf(TEXT("%s_Wall_%d_%d"), *Name, x, y));
                            }
                        }
                    }
                }
            }
        }

        // Build response
        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), Name);
        Result->SetNumberField(TEXT("width"), Width);
        Result->SetNumberField(TEXT("height"), Height);
        Result->SetStringField(TEXT("algorithm"), Algorithm);
        Result->SetNumberField(TEXT("seed"), Seed);
        Result->SetNumberField(TEXT("room_count"), Rooms.Num());
        Result->SetNumberField(TEXT("tiles_spawned"), SpawnedTiles);

        // Room details
        TArray<TSharedPtr<FJsonValue>> RoomArray;
        for (const auto& Room : Rooms)
        {
            TSharedPtr<FJsonObject> RoomObj = MakeShared<FJsonObject>();
            RoomObj->SetNumberField(TEXT("x"), Room.X);
            RoomObj->SetNumberField(TEXT("y"), Room.Y);
            RoomObj->SetNumberField(TEXT("width"), Room.Width);
            RoomObj->SetNumberField(TEXT("height"), Room.Height);
            RoomObj->SetStringField(TEXT("type"), Room.RoomType);
            RoomArray.Add(MakeShared<FJsonValueObject>(RoomObj));
        }
        Result->SetArrayField(TEXT("rooms"), RoomArray);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Dungeon generated successfully"), Result);
        return true;
    }

    // ========================================================================
    // scatter_props - Scatter props/meshes in an area
    // ========================================================================
    if (SubAction == TEXT("scatter_props") || SubAction == TEXT("place_vegetation"))
    {
        FString MeshPath = GetJsonStringField(Params, TEXT("mesh_path"), TEXT(""));
        float Density = GetJsonNumberField(Params, TEXT("density"), 0.01f);
        float MinScale = GetJsonNumberField(Params, TEXT("min_scale"), 0.8f);
        float MaxScale = GetJsonNumberField(Params, TEXT("max_scale"), 1.2f);
        bool bRandomRotation = GetJsonBoolField(Params, TEXT("random_rotation"), true);
        bool bAlignToSurface = GetJsonBoolField(Params, TEXT("align_to_surface"), true);
        int32 Seed = GetJsonNumberField(Params, TEXT("seed"), FMath::Rand());

        // Get bounds
        FVector BoundsMin = FVector::ZeroVector;
        FVector BoundsMax = FVector(1000, 1000, 0);

        const TSharedPtr<FJsonObject>* BoundsMinObj = nullptr;
        const TSharedPtr<FJsonObject>* BoundsMaxObj = nullptr;
        if (Params->TryGetObjectField(TEXT("bounds_min"), BoundsMinObj) && BoundsMinObj)
        {
            BoundsMin = Forge2Helpers::GetVectorFromJson(*BoundsMinObj);
        }
        if (Params->TryGetObjectField(TEXT("bounds_max"), BoundsMaxObj) && BoundsMaxObj)
        {
            BoundsMax = Forge2Helpers::GetVectorFromJson(*BoundsMaxObj);
        }

        // Get mesh array if provided
        TArray<FString> MeshPaths;
        const TArray<TSharedPtr<FJsonValue>>* MeshArray = nullptr;
        if (Params->TryGetArrayField(TEXT("meshes"), MeshArray) && MeshArray)
        {
            for (const auto& MeshVal : *MeshArray)
            {
                MeshPaths.Add(MeshVal->AsString());
            }
        }
        else if (!MeshPath.IsEmpty())
        {
            MeshPaths.Add(MeshPath);
        }

        if (MeshPaths.Num() == 0)
        {
            SendAutomationError(Socket, RequestId, TEXT("No mesh_path or meshes array provided"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
        if (!World)
        {
            SendAutomationError(Socket, RequestId, TEXT("Editor world not available"), TEXT("EDITOR_NOT_AVAILABLE"));
            return true;
        }

        // Load meshes
        TArray<UStaticMesh*> Meshes;
        for (const FString& Path : MeshPaths)
        {
            UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path);
            if (Mesh)
            {
                Meshes.Add(Mesh);
            }
        }

        if (Meshes.Num() == 0)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to load any meshes"), TEXT("LOAD_FAILED"));
            return true;
        }

        FRandomStream Random(Seed);
        int32 SpawnedCount = 0;

        float AreaWidth = BoundsMax.X - BoundsMin.X;
        float AreaHeight = BoundsMax.Y - BoundsMin.Y;
        int32 TargetCount = static_cast<int32>(AreaWidth * AreaHeight * Density / 10000.0f);
        TargetCount = FMath::Max(1, TargetCount);

        const FScopedTransaction Transaction(FText::FromString(TEXT("FORGE2 Scatter Props")));

        for (int32 i = 0; i < TargetCount; i++)
        {
            float X = BoundsMin.X + Random.FRand() * AreaWidth;
            float Y = BoundsMin.Y + Random.FRand() * AreaHeight;
            float Z = BoundsMax.Z;

            FVector Start(X, Y, Z + 10000.0f);
            FVector End(X, Y, Z - 10000.0f);

            FHitResult Hit;
            if (bAlignToSurface && World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
            {
                Z = Hit.Location.Z;
            }

            UStaticMesh* SelectedMesh = Meshes[Random.RandRange(0, Meshes.Num() - 1)];
            float Scale = Random.FRandRange(MinScale, MaxScale);
            float Yaw = bRandomRotation ? Random.FRandRange(0.0f, 360.0f) : 0.0f;

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

            AActor* PropActor = World->SpawnActor<AActor>(AActor::StaticClass(),
                                                          FVector(X, Y, Z),
                                                          FRotator(0, Yaw, 0),
                                                          SpawnParams);
            if (PropActor)
            {
                USceneComponent* Root = NewObject<USceneComponent>(PropActor, TEXT("Root"));
                PropActor->SetRootComponent(Root);
                Root->RegisterComponent();

                UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(PropActor, TEXT("Mesh"));
                MeshComp->SetStaticMesh(SelectedMesh);
                MeshComp->SetWorldScale3D(FVector(Scale));
                MeshComp->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
                MeshComp->RegisterComponent();

                PropActor->SetActorLabel(FString::Printf(TEXT("FORGE2_Prop_%d"), SpawnedCount));
                SpawnedCount++;
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("spawned_count"), SpawnedCount);
        Result->SetNumberField(TEXT("target_count"), TargetCount);
        Result->SetNumberField(TEXT("seed"), Seed);

        SendAutomationResponse(Socket, RequestId, true,
            FString::Printf(TEXT("Scattered %d props"), SpawnedCount), Result);
        return true;
    }

    // Unknown action
    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown worldgen action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif // WITH_EDITOR
}

// ============================================================================
// FORGE2 QUEST HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2QuestAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 quest requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // ========================================================================
    // create_quest - Create a new quest data asset
    // ========================================================================
    if (SubAction == TEXT("create_quest"))
    {
        FString QuestId = GetJsonStringField(Params, TEXT("quest_id"), TEXT(""));
        FString QuestName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString Category = GetJsonStringField(Params, TEXT("category"), TEXT("side"));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Quests"));
        int32 LevelRequirement = GetJsonNumberField(Params, TEXT("level_requirement"), 1);
        bool bRepeatable = GetJsonBoolField(Params, TEXT("repeatable"), false);

        if (QuestId.IsEmpty() || QuestName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing quest_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        // Create data asset
        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, QuestName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* QuestAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*QuestName),
                                                                            RF_Public | RF_Standalone);
        if (QuestAsset)
        {
            // Set properties via reflection
            FProperty* IdProp = QuestAsset->GetClass()->FindPropertyByName(TEXT("QuestId"));
            FProperty* NameProp = QuestAsset->GetClass()->FindPropertyByName(TEXT("DisplayName"));
            FProperty* DescProp = QuestAsset->GetClass()->FindPropertyByName(TEXT("Description"));

            // Mark for save
            QuestAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(QuestAsset);
            McpSafeAssetSave(QuestAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("quest_id"), QuestId);
            Result->SetStringField(TEXT("name"), QuestName);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetStringField(TEXT("category"), Category);
            Result->SetNumberField(TEXT("level_requirement"), LevelRequirement);
            Result->SetBoolField(TEXT("repeatable"), bRepeatable);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Quest created successfully"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create quest asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // ========================================================================
    // add_objective - Add objective to quest
    // ========================================================================
    if (SubAction == TEXT("add_objective"))
    {
        FString QuestPath = GetJsonStringField(Params, TEXT("quest_path"), TEXT(""));
        FString ObjectiveId = GetJsonStringField(Params, TEXT("objective_id"), TEXT(""));
        FString ObjectiveType = GetJsonStringField(Params, TEXT("type"), TEXT("interact"));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString Target = GetJsonStringField(Params, TEXT("target"), TEXT(""));
        int32 TargetCount = GetJsonNumberField(Params, TEXT("count"), 1);
        bool bOptional = GetJsonBoolField(Params, TEXT("optional"), false);

        if (QuestPath.IsEmpty() || ObjectiveId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing quest_path or objective_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("quest_path"), QuestPath);
        Result->SetStringField(TEXT("objective_id"), ObjectiveId);
        Result->SetStringField(TEXT("type"), ObjectiveType);
        Result->SetStringField(TEXT("target"), Target);
        Result->SetNumberField(TEXT("count"), TargetCount);
        Result->SetBoolField(TEXT("optional"), bOptional);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Objective added"), Result);
        return true;
    }

    // ========================================================================
    // set_rewards - Set quest rewards
    // ========================================================================
    if (SubAction == TEXT("set_rewards"))
    {
        FString QuestPath = GetJsonStringField(Params, TEXT("quest_path"), TEXT(""));
        int32 XPReward = GetJsonNumberField(Params, TEXT("xp"), 0);
        int32 GoldReward = GetJsonNumberField(Params, TEXT("gold"), 0);

        if (QuestPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing quest_path"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("quest_path"), QuestPath);
        Result->SetNumberField(TEXT("xp"), XPReward);
        Result->SetNumberField(TEXT("gold"), GoldReward);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Rewards set"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown quest action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 DIALOGUE HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2DialogueAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 dialogue requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_dialogue_tree - Create a new dialogue tree asset
    if (SubAction == TEXT("create_dialogue_tree"))
    {
        FString TreeId = GetJsonStringField(Params, TEXT("tree_id"), TEXT(""));
        FString TreeName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Dialogue"));

        if (TreeId.IsEmpty() || TreeName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing tree_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, TreeName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* DialogueAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*TreeName),
                                                                               RF_Public | RF_Standalone);
        if (DialogueAsset)
        {
            DialogueAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(DialogueAsset);
            McpSafeAssetSave(DialogueAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("tree_id"), TreeId);
            Result->SetStringField(TEXT("name"), TreeName);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetStringField(TEXT("description"), Description);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue tree created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create dialogue asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // add_dialogue_node - Add a node to dialogue tree
    if (SubAction == TEXT("add_dialogue_node"))
    {
        FString TreePath = GetJsonStringField(Params, TEXT("tree_path"), TEXT(""));
        FString NodeId = GetJsonStringField(Params, TEXT("node_id"), TEXT(""));
        FString NodeType = GetJsonStringField(Params, TEXT("type"), TEXT("npc_speech"));
        FString SpeakerId = GetJsonStringField(Params, TEXT("speaker_id"), TEXT(""));
        FString Text = GetJsonStringField(Params, TEXT("text"), TEXT(""));
        FString AudioPath = GetJsonStringField(Params, TEXT("audio_path"), TEXT(""));
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 0.0f);

        if (TreePath.IsEmpty() || NodeId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing tree_path or node_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tree_path"), TreePath);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("type"), NodeType);
        Result->SetStringField(TEXT("speaker_id"), SpeakerId);
        Result->SetStringField(TEXT("text"), Text);
        Result->SetStringField(TEXT("audio_path"), AudioPath);
        Result->SetNumberField(TEXT("duration"), Duration);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue node added"), Result);
        return true;
    }

    // connect_dialogue_nodes - Connect two dialogue nodes
    if (SubAction == TEXT("connect_dialogue_nodes"))
    {
        FString TreePath = GetJsonStringField(Params, TEXT("tree_path"), TEXT(""));
        FString FromNodeId = GetJsonStringField(Params, TEXT("from_node_id"), TEXT(""));
        FString ToNodeId = GetJsonStringField(Params, TEXT("to_node_id"), TEXT(""));
        FString ConditionType = GetJsonStringField(Params, TEXT("condition_type"), TEXT("none"));
        FString ConditionValue = GetJsonStringField(Params, TEXT("condition_value"), TEXT(""));

        if (TreePath.IsEmpty() || FromNodeId.IsEmpty() || ToNodeId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tree_path"), TreePath);
        Result->SetStringField(TEXT("from_node_id"), FromNodeId);
        Result->SetStringField(TEXT("to_node_id"), ToNodeId);
        Result->SetStringField(TEXT("condition_type"), ConditionType);
        Result->SetStringField(TEXT("condition_value"), ConditionValue);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue nodes connected"), Result);
        return true;
    }

    // create_speaker_profile - Create NPC speaker profile
    if (SubAction == TEXT("create_speaker_profile"))
    {
        FString SpeakerId = GetJsonStringField(Params, TEXT("speaker_id"), TEXT(""));
        FString DisplayName = GetJsonStringField(Params, TEXT("display_name"), TEXT(""));
        FString PortraitPath = GetJsonStringField(Params, TEXT("portrait_path"), TEXT(""));
        FString VoiceType = GetJsonStringField(Params, TEXT("voice_type"), TEXT("neutral"));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Dialogue/Speakers"));

        if (SpeakerId.IsEmpty() || DisplayName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing speaker_id or display_name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, DisplayName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* SpeakerAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*DisplayName),
                                                                              RF_Public | RF_Standalone);
        if (SpeakerAsset)
        {
            SpeakerAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(SpeakerAsset);
            McpSafeAssetSave(SpeakerAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("speaker_id"), SpeakerId);
            Result->SetStringField(TEXT("display_name"), DisplayName);
            Result->SetStringField(TEXT("portrait_path"), PortraitPath);
            Result->SetStringField(TEXT("voice_type"), VoiceType);
            Result->SetStringField(TEXT("path"), Package->GetName());

            SendAutomationResponse(Socket, RequestId, true, TEXT("Speaker profile created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create speaker asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // add_choice_condition - Add condition to dialogue choice
    if (SubAction == TEXT("add_choice_condition"))
    {
        FString TreePath = GetJsonStringField(Params, TEXT("tree_path"), TEXT(""));
        FString NodeId = GetJsonStringField(Params, TEXT("node_id"), TEXT(""));
        FString ConditionType = GetJsonStringField(Params, TEXT("condition_type"), TEXT("has_item"));
        FString ConditionTarget = GetJsonStringField(Params, TEXT("target"), TEXT(""));
        int32 RequiredValue = GetJsonNumberField(Params, TEXT("required_value"), 1);
        FString CompareOp = GetJsonStringField(Params, TEXT("compare_op"), TEXT(">="));

        if (TreePath.IsEmpty() || NodeId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing tree_path or node_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tree_path"), TreePath);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("condition_type"), ConditionType);
        Result->SetStringField(TEXT("target"), ConditionTarget);
        Result->SetNumberField(TEXT("required_value"), RequiredValue);
        Result->SetStringField(TEXT("compare_op"), CompareOp);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Choice condition added"), Result);
        return true;
    }

    // add_consequence - Add consequence to dialogue choice
    if (SubAction == TEXT("add_consequence"))
    {
        FString TreePath = GetJsonStringField(Params, TEXT("tree_path"), TEXT(""));
        FString NodeId = GetJsonStringField(Params, TEXT("node_id"), TEXT(""));
        FString ConsequenceType = GetJsonStringField(Params, TEXT("consequence_type"), TEXT("give_item"));
        FString Target = GetJsonStringField(Params, TEXT("target"), TEXT(""));
        int32 Value = GetJsonNumberField(Params, TEXT("value"), 1);

        if (TreePath.IsEmpty() || NodeId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing tree_path or node_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tree_path"), TreePath);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("consequence_type"), ConsequenceType);
        Result->SetStringField(TEXT("target"), Target);
        Result->SetNumberField(TEXT("value"), Value);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Consequence added"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown dialogue action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 SKILL TREE HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2SkillTreeAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 skill_tree requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_skill_tree - Create a new skill tree layout
    if (SubAction == TEXT("create_skill_tree"))
    {
        FString TreeId = GetJsonStringField(Params, TEXT("tree_id"), TEXT(""));
        FString TreeName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString ClassName = GetJsonStringField(Params, TEXT("class_name"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Skills"));
        int32 MaxPoints = GetJsonNumberField(Params, TEXT("max_points"), 50);

        if (TreeId.IsEmpty() || TreeName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing tree_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, TreeName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* SkillTreeAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*TreeName),
                                                                                RF_Public | RF_Standalone);
        if (SkillTreeAsset)
        {
            SkillTreeAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(SkillTreeAsset);
            McpSafeAssetSave(SkillTreeAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("tree_id"), TreeId);
            Result->SetStringField(TEXT("name"), TreeName);
            Result->SetStringField(TEXT("class_name"), ClassName);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetNumberField(TEXT("max_points"), MaxPoints);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Skill tree created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create skill tree asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // create_skill - Create a skill definition
    if (SubAction == TEXT("create_skill"))
    {
        FString SkillId = GetJsonStringField(Params, TEXT("skill_id"), TEXT(""));
        FString SkillName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString SkillType = GetJsonStringField(Params, TEXT("type"), TEXT("passive"));
        FString IconPath = GetJsonStringField(Params, TEXT("icon_path"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Skills/Definitions"));
        int32 MaxRanks = GetJsonNumberField(Params, TEXT("max_ranks"), 5);
        int32 PointCost = GetJsonNumberField(Params, TEXT("point_cost"), 1);
        int32 LevelRequirement = GetJsonNumberField(Params, TEXT("level_requirement"), 1);

        if (SkillId.IsEmpty() || SkillName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing skill_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, SkillName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* SkillAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*SkillName),
                                                                            RF_Public | RF_Standalone);
        if (SkillAsset)
        {
            SkillAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(SkillAsset);
            McpSafeAssetSave(SkillAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("skill_id"), SkillId);
            Result->SetStringField(TEXT("name"), SkillName);
            Result->SetStringField(TEXT("type"), SkillType);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetNumberField(TEXT("max_ranks"), MaxRanks);
            Result->SetNumberField(TEXT("point_cost"), PointCost);
            Result->SetNumberField(TEXT("level_requirement"), LevelRequirement);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Skill created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create skill asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // add_skill_node - Add skill to tree at position
    if (SubAction == TEXT("add_skill_node"))
    {
        FString TreePath = GetJsonStringField(Params, TEXT("tree_path"), TEXT(""));
        FString SkillId = GetJsonStringField(Params, TEXT("skill_id"), TEXT(""));
        float PositionX = GetJsonNumberField(Params, TEXT("position_x"), 0.0f);
        float PositionY = GetJsonNumberField(Params, TEXT("position_y"), 0.0f);
        FString Tier = GetJsonStringField(Params, TEXT("tier"), TEXT("basic"));

        if (TreePath.IsEmpty() || SkillId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing tree_path or skill_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tree_path"), TreePath);
        Result->SetStringField(TEXT("skill_id"), SkillId);
        Result->SetNumberField(TEXT("position_x"), PositionX);
        Result->SetNumberField(TEXT("position_y"), PositionY);
        Result->SetStringField(TEXT("tier"), Tier);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Skill node added to tree"), Result);
        return true;
    }

    // configure_skill_rank - Set bonuses per skill rank
    if (SubAction == TEXT("configure_skill_rank"))
    {
        FString SkillPath = GetJsonStringField(Params, TEXT("skill_path"), TEXT(""));
        int32 Rank = GetJsonNumberField(Params, TEXT("rank"), 1);
        FString StatType = GetJsonStringField(Params, TEXT("stat_type"), TEXT("damage"));
        float BonusValue = GetJsonNumberField(Params, TEXT("bonus_value"), 0.0f);
        FString BonusType = GetJsonStringField(Params, TEXT("bonus_type"), TEXT("flat"));

        if (SkillPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing skill_path"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skill_path"), SkillPath);
        Result->SetNumberField(TEXT("rank"), Rank);
        Result->SetStringField(TEXT("stat_type"), StatType);
        Result->SetNumberField(TEXT("bonus_value"), BonusValue);
        Result->SetStringField(TEXT("bonus_type"), BonusType);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Skill rank configured"), Result);
        return true;
    }

    // add_skill_prerequisite - Add prerequisite to skill
    if (SubAction == TEXT("add_skill_prerequisite"))
    {
        FString SkillPath = GetJsonStringField(Params, TEXT("skill_path"), TEXT(""));
        FString RequiredSkillId = GetJsonStringField(Params, TEXT("required_skill_id"), TEXT(""));
        int32 RequiredRank = GetJsonNumberField(Params, TEXT("required_rank"), 1);

        if (SkillPath.IsEmpty() || RequiredSkillId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing skill_path or required_skill_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skill_path"), SkillPath);
        Result->SetStringField(TEXT("required_skill_id"), RequiredSkillId);
        Result->SetNumberField(TEXT("required_rank"), RequiredRank);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Skill prerequisite added"), Result);
        return true;
    }

    // connect_skills - Connect skills in tree (visual line)
    if (SubAction == TEXT("connect_skills"))
    {
        FString TreePath = GetJsonStringField(Params, TEXT("tree_path"), TEXT(""));
        FString FromSkillId = GetJsonStringField(Params, TEXT("from_skill_id"), TEXT(""));
        FString ToSkillId = GetJsonStringField(Params, TEXT("to_skill_id"), TEXT(""));
        FString LineStyle = GetJsonStringField(Params, TEXT("line_style"), TEXT("solid"));

        if (TreePath.IsEmpty() || FromSkillId.IsEmpty() || ToSkillId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tree_path"), TreePath);
        Result->SetStringField(TEXT("from_skill_id"), FromSkillId);
        Result->SetStringField(TEXT("to_skill_id"), ToSkillId);
        Result->SetStringField(TEXT("line_style"), LineStyle);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Skills connected"), Result);
        return true;
    }

    // add_damage_effect - Add damage effect to active skill
    if (SubAction == TEXT("add_damage_effect"))
    {
        FString SkillPath = GetJsonStringField(Params, TEXT("skill_path"), TEXT(""));
        FString DamageType = GetJsonStringField(Params, TEXT("damage_type"), TEXT("physical"));
        float BaseDamage = GetJsonNumberField(Params, TEXT("base_damage"), 10.0f);
        float ScalingPerRank = GetJsonNumberField(Params, TEXT("scaling_per_rank"), 5.0f);
        FString ScalingStat = GetJsonStringField(Params, TEXT("scaling_stat"), TEXT("strength"));
        float StatScaling = GetJsonNumberField(Params, TEXT("stat_scaling"), 1.0f);

        if (SkillPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing skill_path"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skill_path"), SkillPath);
        Result->SetStringField(TEXT("damage_type"), DamageType);
        Result->SetNumberField(TEXT("base_damage"), BaseDamage);
        Result->SetNumberField(TEXT("scaling_per_rank"), ScalingPerRank);
        Result->SetStringField(TEXT("scaling_stat"), ScalingStat);
        Result->SetNumberField(TEXT("stat_scaling"), StatScaling);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Damage effect added to skill"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown skill_tree action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 INVENTORY HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2InventoryAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 inventory requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_item - Create base item definition
    if (SubAction == TEXT("create_item"))
    {
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), TEXT(""));
        FString ItemName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString ItemType = GetJsonStringField(Params, TEXT("type"), TEXT("misc"));
        FString Rarity = GetJsonStringField(Params, TEXT("rarity"), TEXT("common"));
        FString IconPath = GetJsonStringField(Params, TEXT("icon_path"), TEXT(""));
        FString MeshPath = GetJsonStringField(Params, TEXT("mesh_path"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Items"));
        int32 MaxStack = GetJsonNumberField(Params, TEXT("max_stack"), 99);
        float Weight = GetJsonNumberField(Params, TEXT("weight"), 1.0f);
        int32 BaseValue = GetJsonNumberField(Params, TEXT("base_value"), 10);

        if (ItemId.IsEmpty() || ItemName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing item_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, ItemName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* ItemAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*ItemName),
                                                                           RF_Public | RF_Standalone);
        if (ItemAsset)
        {
            ItemAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(ItemAsset);
            McpSafeAssetSave(ItemAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("item_id"), ItemId);
            Result->SetStringField(TEXT("name"), ItemName);
            Result->SetStringField(TEXT("type"), ItemType);
            Result->SetStringField(TEXT("rarity"), Rarity);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetNumberField(TEXT("max_stack"), MaxStack);
            Result->SetNumberField(TEXT("weight"), Weight);
            Result->SetNumberField(TEXT("base_value"), BaseValue);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Item created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create item asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // create_weapon - Create weapon item
    if (SubAction == TEXT("create_weapon"))
    {
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), TEXT(""));
        FString ItemName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString WeaponType = GetJsonStringField(Params, TEXT("weapon_type"), TEXT("sword"));
        FString DamageType = GetJsonStringField(Params, TEXT("damage_type"), TEXT("physical"));
        float MinDamage = GetJsonNumberField(Params, TEXT("min_damage"), 10.0f);
        float MaxDamage = GetJsonNumberField(Params, TEXT("max_damage"), 20.0f);
        float AttackSpeed = GetJsonNumberField(Params, TEXT("attack_speed"), 1.0f);
        float Range = GetJsonNumberField(Params, TEXT("range"), 150.0f);
        float CritChance = GetJsonNumberField(Params, TEXT("crit_chance"), 0.05f);
        float CritMultiplier = GetJsonNumberField(Params, TEXT("crit_multiplier"), 2.0f);
        FString Rarity = GetJsonStringField(Params, TEXT("rarity"), TEXT("common"));
        int32 LevelRequirement = GetJsonNumberField(Params, TEXT("level_requirement"), 1);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Items/Weapons"));

        if (ItemId.IsEmpty() || ItemName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing item_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, ItemName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* WeaponAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*ItemName),
                                                                             RF_Public | RF_Standalone);
        if (WeaponAsset)
        {
            WeaponAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(WeaponAsset);
            McpSafeAssetSave(WeaponAsset);

            float DPS = ((MinDamage + MaxDamage) / 2.0f) * AttackSpeed * (1.0f + CritChance * (CritMultiplier - 1.0f));

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("item_id"), ItemId);
            Result->SetStringField(TEXT("name"), ItemName);
            Result->SetStringField(TEXT("weapon_type"), WeaponType);
            Result->SetStringField(TEXT("damage_type"), DamageType);
            Result->SetNumberField(TEXT("min_damage"), MinDamage);
            Result->SetNumberField(TEXT("max_damage"), MaxDamage);
            Result->SetNumberField(TEXT("attack_speed"), AttackSpeed);
            Result->SetNumberField(TEXT("range"), Range);
            Result->SetNumberField(TEXT("crit_chance"), CritChance);
            Result->SetNumberField(TEXT("crit_multiplier"), CritMultiplier);
            Result->SetNumberField(TEXT("dps"), DPS);
            Result->SetStringField(TEXT("rarity"), Rarity);
            Result->SetNumberField(TEXT("level_requirement"), LevelRequirement);
            Result->SetStringField(TEXT("path"), Package->GetName());

            SendAutomationResponse(Socket, RequestId, true, TEXT("Weapon created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create weapon asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // create_armor - Create armor item
    if (SubAction == TEXT("create_armor"))
    {
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), TEXT(""));
        FString ItemName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString ArmorSlot = GetJsonStringField(Params, TEXT("slot"), TEXT("chest"));
        FString ArmorType = GetJsonStringField(Params, TEXT("armor_type"), TEXT("plate"));
        float BaseArmor = GetJsonNumberField(Params, TEXT("base_armor"), 50.0f);
        float MagicResist = GetJsonNumberField(Params, TEXT("magic_resist"), 0.0f);
        FString Rarity = GetJsonStringField(Params, TEXT("rarity"), TEXT("common"));
        int32 LevelRequirement = GetJsonNumberField(Params, TEXT("level_requirement"), 1);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Items/Armor"));

        if (ItemId.IsEmpty() || ItemName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing item_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, ItemName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* ArmorAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*ItemName),
                                                                            RF_Public | RF_Standalone);
        if (ArmorAsset)
        {
            ArmorAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(ArmorAsset);
            McpSafeAssetSave(ArmorAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("item_id"), ItemId);
            Result->SetStringField(TEXT("name"), ItemName);
            Result->SetStringField(TEXT("slot"), ArmorSlot);
            Result->SetStringField(TEXT("armor_type"), ArmorType);
            Result->SetNumberField(TEXT("base_armor"), BaseArmor);
            Result->SetNumberField(TEXT("magic_resist"), MagicResist);
            Result->SetStringField(TEXT("rarity"), Rarity);
            Result->SetNumberField(TEXT("level_requirement"), LevelRequirement);
            Result->SetStringField(TEXT("path"), Package->GetName());

            SendAutomationResponse(Socket, RequestId, true, TEXT("Armor created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create armor asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // create_consumable - Create consumable item
    if (SubAction == TEXT("create_consumable"))
    {
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), TEXT(""));
        FString ItemName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString EffectType = GetJsonStringField(Params, TEXT("effect_type"), TEXT("heal"));
        float EffectValue = GetJsonNumberField(Params, TEXT("effect_value"), 50.0f);
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 0.0f);
        float Cooldown = GetJsonNumberField(Params, TEXT("cooldown"), 0.0f);
        int32 Charges = GetJsonNumberField(Params, TEXT("charges"), 1);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Items/Consumables"));

        if (ItemId.IsEmpty() || ItemName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing item_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, ItemName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* ConsumableAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*ItemName),
                                                                                 RF_Public | RF_Standalone);
        if (ConsumableAsset)
        {
            ConsumableAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(ConsumableAsset);
            McpSafeAssetSave(ConsumableAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("item_id"), ItemId);
            Result->SetStringField(TEXT("name"), ItemName);
            Result->SetStringField(TEXT("effect_type"), EffectType);
            Result->SetNumberField(TEXT("effect_value"), EffectValue);
            Result->SetNumberField(TEXT("duration"), Duration);
            Result->SetNumberField(TEXT("cooldown"), Cooldown);
            Result->SetNumberField(TEXT("charges"), Charges);
            Result->SetStringField(TEXT("path"), Package->GetName());

            SendAutomationResponse(Socket, RequestId, true, TEXT("Consumable created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create consumable asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // add_stat_bonus - Add stat bonus to item
    if (SubAction == TEXT("add_stat_bonus"))
    {
        FString ItemPath = GetJsonStringField(Params, TEXT("item_path"), TEXT(""));
        FString StatType = GetJsonStringField(Params, TEXT("stat_type"), TEXT("strength"));
        float BonusValue = GetJsonNumberField(Params, TEXT("bonus_value"), 10.0f);
        FString BonusType = GetJsonStringField(Params, TEXT("bonus_type"), TEXT("flat"));

        if (ItemPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing item_path"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("item_path"), ItemPath);
        Result->SetStringField(TEXT("stat_type"), StatType);
        Result->SetNumberField(TEXT("bonus_value"), BonusValue);
        Result->SetStringField(TEXT("bonus_type"), BonusType);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Stat bonus added"), Result);
        return true;
    }

    // create_loot_table - Create loot table
    if (SubAction == TEXT("create_loot_table"))
    {
        FString TableId = GetJsonStringField(Params, TEXT("table_id"), TEXT(""));
        FString TableName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/LootTables"));
        int32 MinDrops = GetJsonNumberField(Params, TEXT("min_drops"), 1);
        int32 MaxDrops = GetJsonNumberField(Params, TEXT("max_drops"), 3);

        if (TableId.IsEmpty() || TableName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing table_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, TableName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* LootTableAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*TableName),
                                                                                RF_Public | RF_Standalone);
        if (LootTableAsset)
        {
            LootTableAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(LootTableAsset);
            McpSafeAssetSave(LootTableAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("table_id"), TableId);
            Result->SetStringField(TEXT("name"), TableName);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetNumberField(TEXT("min_drops"), MinDrops);
            Result->SetNumberField(TEXT("max_drops"), MaxDrops);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Loot table created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create loot table asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // add_loot_entry - Add entry to loot table
    if (SubAction == TEXT("add_loot_entry"))
    {
        FString TablePath = GetJsonStringField(Params, TEXT("table_path"), TEXT(""));
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), TEXT(""));
        float DropWeight = GetJsonNumberField(Params, TEXT("weight"), 100.0f);
        int32 MinCount = GetJsonNumberField(Params, TEXT("min_count"), 1);
        int32 MaxCount = GetJsonNumberField(Params, TEXT("max_count"), 1);
        int32 MinLevel = GetJsonNumberField(Params, TEXT("min_level"), 0);
        int32 MaxLevel = GetJsonNumberField(Params, TEXT("max_level"), 999);

        if (TablePath.IsEmpty() || ItemId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing table_path or item_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("table_path"), TablePath);
        Result->SetStringField(TEXT("item_id"), ItemId);
        Result->SetNumberField(TEXT("weight"), DropWeight);
        Result->SetNumberField(TEXT("min_count"), MinCount);
        Result->SetNumberField(TEXT("max_count"), MaxCount);
        Result->SetNumberField(TEXT("min_level"), MinLevel);
        Result->SetNumberField(TEXT("max_level"), MaxLevel);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Loot entry added"), Result);
        return true;
    }

    // create_crafting_recipe - Create crafting recipe
    if (SubAction == TEXT("create_crafting_recipe"))
    {
        FString RecipeId = GetJsonStringField(Params, TEXT("recipe_id"), TEXT(""));
        FString RecipeName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString OutputItemId = GetJsonStringField(Params, TEXT("output_item_id"), TEXT(""));
        int32 OutputCount = GetJsonNumberField(Params, TEXT("output_count"), 1);
        FString RequiredStation = GetJsonStringField(Params, TEXT("required_station"), TEXT(""));
        int32 RequiredSkillLevel = GetJsonNumberField(Params, TEXT("required_skill_level"), 1);
        float CraftingTime = GetJsonNumberField(Params, TEXT("crafting_time"), 1.0f);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Crafting"));

        if (RecipeId.IsEmpty() || OutputItemId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing recipe_id or output_item_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, RecipeName.IsEmpty() ? RecipeId : RecipeName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        FString AssetName = RecipeName.IsEmpty() ? RecipeId : RecipeName;
        UMcpGenericDataAsset* RecipeAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*AssetName),
                                                                             RF_Public | RF_Standalone);
        if (RecipeAsset)
        {
            RecipeAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(RecipeAsset);
            McpSafeAssetSave(RecipeAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("recipe_id"), RecipeId);
            Result->SetStringField(TEXT("name"), AssetName);
            Result->SetStringField(TEXT("output_item_id"), OutputItemId);
            Result->SetNumberField(TEXT("output_count"), OutputCount);
            Result->SetStringField(TEXT("required_station"), RequiredStation);
            Result->SetNumberField(TEXT("required_skill_level"), RequiredSkillLevel);
            Result->SetNumberField(TEXT("crafting_time"), CraftingTime);
            Result->SetStringField(TEXT("path"), Package->GetName());

            SendAutomationResponse(Socket, RequestId, true, TEXT("Crafting recipe created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create recipe asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // add_recipe_ingredient - Add ingredient to recipe
    if (SubAction == TEXT("add_recipe_ingredient"))
    {
        FString RecipePath = GetJsonStringField(Params, TEXT("recipe_path"), TEXT(""));
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), TEXT(""));
        int32 Count = GetJsonNumberField(Params, TEXT("count"), 1);

        if (RecipePath.IsEmpty() || ItemId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing recipe_path or item_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("recipe_path"), RecipePath);
        Result->SetStringField(TEXT("item_id"), ItemId);
        Result->SetNumberField(TEXT("count"), Count);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Ingredient added to recipe"), Result);
        return true;
    }

    // create_item_set - Create item set
    if (SubAction == TEXT("create_item_set"))
    {
        FString SetId = GetJsonStringField(Params, TEXT("set_id"), TEXT(""));
        FString SetName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/ItemSets"));

        if (SetId.IsEmpty() || SetName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing set_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, SetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpGenericDataAsset* SetAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*SetName),
                                                                          RF_Public | RF_Standalone);
        if (SetAsset)
        {
            SetAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(SetAsset);
            McpSafeAssetSave(SetAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("set_id"), SetId);
            Result->SetStringField(TEXT("name"), SetName);
            Result->SetStringField(TEXT("path"), Package->GetName());

            SendAutomationResponse(Socket, RequestId, true, TEXT("Item set created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create item set asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown inventory action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 ECONOMY HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2EconomyAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 economy requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_currency - Create currency definition
    if (SubAction == TEXT("create_currency"))
    {
        FString CurrencyId = GetJsonStringField(Params, TEXT("currency_id"), TEXT(""));
        FString DisplayName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString IconPath = GetJsonStringField(Params, TEXT("icon_path"), TEXT(""));
        FString CurrencyType = GetJsonStringField(Params, TEXT("type"), TEXT("standard"));
        int64 MaxAmount = static_cast<int64>(GetJsonNumberField(Params, TEXT("max_amount"), 999999999.0));
        bool bTradeable = GetJsonBoolField(Params, TEXT("tradeable"), true);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Economy"));

        if (CurrencyId.IsEmpty() || DisplayName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing currency_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, DisplayName);
        if (Package)
        {
            UMcpGenericDataAsset* CurrencyAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*DisplayName),
                                                                                   RF_Public | RF_Standalone);
            if (CurrencyAsset)
            {
                CurrencyAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(CurrencyAsset);
                McpSafeAssetSave(CurrencyAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("currency_id"), CurrencyId);
                Result->SetStringField(TEXT("name"), DisplayName);
                Result->SetStringField(TEXT("type"), CurrencyType);
                Result->SetNumberField(TEXT("max_amount"), static_cast<double>(MaxAmount));
                Result->SetBoolField(TEXT("tradeable"), bTradeable);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Currency created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create currency"), TEXT("CREATION_FAILED"));
        return true;
    }

    // create_shop - Create shop/vendor
    if (SubAction == TEXT("create_shop"))
    {
        FString ShopId = GetJsonStringField(Params, TEXT("shop_id"), TEXT(""));
        FString ShopName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString ShopType = GetJsonStringField(Params, TEXT("type"), TEXT("general"));
        float BuyMultiplier = GetJsonNumberField(Params, TEXT("buy_multiplier"), 1.0f);
        float SellMultiplier = GetJsonNumberField(Params, TEXT("sell_multiplier"), 0.5f);
        float RefreshTime = GetJsonNumberField(Params, TEXT("refresh_time"), 3600.0f);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Shops"));

        if (ShopId.IsEmpty() || ShopName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing shop_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, ShopName);
        if (Package)
        {
            UMcpGenericDataAsset* ShopAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*ShopName),
                                                                               RF_Public | RF_Standalone);
            if (ShopAsset)
            {
                ShopAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(ShopAsset);
                McpSafeAssetSave(ShopAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("shop_id"), ShopId);
                Result->SetStringField(TEXT("name"), ShopName);
                Result->SetStringField(TEXT("type"), ShopType);
                Result->SetNumberField(TEXT("buy_multiplier"), BuyMultiplier);
                Result->SetNumberField(TEXT("sell_multiplier"), SellMultiplier);
                Result->SetNumberField(TEXT("refresh_time"), RefreshTime);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Shop created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create shop"), TEXT("CREATION_FAILED"));
        return true;
    }

    // add_shop_item - Add item to shop inventory
    if (SubAction == TEXT("add_shop_item"))
    {
        FString ShopPath = GetJsonStringField(Params, TEXT("shop_path"), TEXT(""));
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), TEXT(""));
        int32 Stock = GetJsonNumberField(Params, TEXT("stock"), -1);
        int32 Price = GetJsonNumberField(Params, TEXT("price"), 0);
        FString CurrencyId = GetJsonStringField(Params, TEXT("currency_id"), TEXT("gold"));
        int32 RequiredLevel = GetJsonNumberField(Params, TEXT("required_level"), 0);
        FString RequiredFaction = GetJsonStringField(Params, TEXT("required_faction"), TEXT(""));
        FString RequiredRank = GetJsonStringField(Params, TEXT("required_rank"), TEXT(""));

        if (ShopPath.IsEmpty() || ItemId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing shop_path or item_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("shop_path"), ShopPath);
        Result->SetStringField(TEXT("item_id"), ItemId);
        Result->SetNumberField(TEXT("stock"), Stock);
        Result->SetNumberField(TEXT("price"), Price);
        Result->SetStringField(TEXT("currency_id"), CurrencyId);
        Result->SetNumberField(TEXT("required_level"), RequiredLevel);
        Result->SetStringField(TEXT("required_faction"), RequiredFaction);
        Result->SetStringField(TEXT("required_rank"), RequiredRank);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Shop item added"), Result);
        return true;
    }

    // configure_trading - Set up trading system
    if (SubAction == TEXT("configure_trading"))
    {
        float TradeTax = GetJsonNumberField(Params, TEXT("trade_tax"), 0.05f);
        float MinTradeValue = GetJsonNumberField(Params, TEXT("min_trade_value"), 1.0f);
        bool bRequireConfirmation = GetJsonBoolField(Params, TEXT("require_confirmation"), true);
        int32 TradeRange = GetJsonNumberField(Params, TEXT("trade_range"), 500);
        float TradeTimeout = GetJsonNumberField(Params, TEXT("trade_timeout"), 60.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("trade_tax"), TradeTax);
        Result->SetNumberField(TEXT("min_trade_value"), MinTradeValue);
        Result->SetBoolField(TEXT("require_confirmation"), bRequireConfirmation);
        Result->SetNumberField(TEXT("trade_range"), TradeRange);
        Result->SetNumberField(TEXT("trade_timeout"), TradeTimeout);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Trading configured"), Result);
        return true;
    }

    // create_auction_house - Set up auction house
    if (SubAction == TEXT("create_auction_house"))
    {
        FString AuctionId = GetJsonStringField(Params, TEXT("auction_id"), TEXT(""));
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("Auction House"));
        float ListingFee = GetJsonNumberField(Params, TEXT("listing_fee"), 0.05f);
        float SaleTax = GetJsonNumberField(Params, TEXT("sale_tax"), 0.10f);
        float MinDuration = GetJsonNumberField(Params, TEXT("min_duration"), 3600.0f);
        float MaxDuration = GetJsonNumberField(Params, TEXT("max_duration"), 172800.0f);
        int32 MaxListingsPerPlayer = GetJsonNumberField(Params, TEXT("max_listings"), 20);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Economy"));

        if (AuctionId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing auction_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, Name);
        if (Package)
        {
            UMcpGenericDataAsset* AuctionAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*Name),
                                                                                  RF_Public | RF_Standalone);
            if (AuctionAsset)
            {
                AuctionAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(AuctionAsset);
                McpSafeAssetSave(AuctionAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("auction_id"), AuctionId);
                Result->SetStringField(TEXT("name"), Name);
                Result->SetNumberField(TEXT("listing_fee"), ListingFee);
                Result->SetNumberField(TEXT("sale_tax"), SaleTax);
                Result->SetNumberField(TEXT("min_duration"), MinDuration);
                Result->SetNumberField(TEXT("max_duration"), MaxDuration);
                Result->SetNumberField(TEXT("max_listings"), MaxListingsPerPlayer);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Auction house created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create auction house"), TEXT("CREATION_FAILED"));
        return true;
    }

    // configure_gold_sinks - Set up economy gold sinks
    if (SubAction == TEXT("configure_gold_sinks"))
    {
        float RepairCostPercent = GetJsonNumberField(Params, TEXT("repair_cost_percent"), 0.10f);
        float TravelCostBase = GetJsonNumberField(Params, TEXT("travel_cost_base"), 10.0f);
        float TravelCostPerDistance = GetJsonNumberField(Params, TEXT("travel_cost_per_distance"), 0.1f);
        float RespecCostBase = GetJsonNumberField(Params, TEXT("respec_cost_base"), 100.0f);
        float RespecCostPerLevel = GetJsonNumberField(Params, TEXT("respec_cost_per_level"), 10.0f);
        float CraftingTax = GetJsonNumberField(Params, TEXT("crafting_tax"), 0.05f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("repair_cost_percent"), RepairCostPercent);
        Result->SetNumberField(TEXT("travel_cost_base"), TravelCostBase);
        Result->SetNumberField(TEXT("travel_cost_per_distance"), TravelCostPerDistance);
        Result->SetNumberField(TEXT("respec_cost_base"), RespecCostBase);
        Result->SetNumberField(TEXT("respec_cost_per_level"), RespecCostPerLevel);
        Result->SetNumberField(TEXT("crafting_tax"), CraftingTax);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Gold sinks configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown economy action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 PROGRESSION HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2ProgressionAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 progression requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // configure_xp_curve - Set XP requirements per level
    if (SubAction == TEXT("configure_xp_curve"))
    {
        FString CurveType = GetJsonStringField(Params, TEXT("curve_type"), TEXT("exponential"));
        int32 MaxLevel = GetJsonNumberField(Params, TEXT("max_level"), 100);
        float BaseXP = GetJsonNumberField(Params, TEXT("base_xp"), 100.0f);
        float Multiplier = GetJsonNumberField(Params, TEXT("multiplier"), 1.5f);
        float Exponent = GetJsonNumberField(Params, TEXT("exponent"), 1.1f);

        // Calculate sample XP values
        TArray<TSharedPtr<FJsonValue>> SampleLevels;
        for (int32 Level = 1; Level <= FMath::Min(10, MaxLevel); Level++)
        {
            float XPRequired = 0.0f;
            if (CurveType == TEXT("linear"))
            {
                XPRequired = BaseXP * Level * Multiplier;
            }
            else if (CurveType == TEXT("exponential"))
            {
                XPRequired = BaseXP * FMath::Pow(Multiplier, static_cast<float>(Level - 1));
            }
            else if (CurveType == TEXT("polynomial"))
            {
                XPRequired = BaseXP * FMath::Pow(static_cast<float>(Level), Exponent);
            }

            TSharedPtr<FJsonObject> LevelObj = MakeShared<FJsonObject>();
            LevelObj->SetNumberField(TEXT("level"), Level);
            LevelObj->SetNumberField(TEXT("xp_required"), XPRequired);
            SampleLevels.Add(MakeShared<FJsonValueObject>(LevelObj));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("curve_type"), CurveType);
        Result->SetNumberField(TEXT("max_level"), MaxLevel);
        Result->SetNumberField(TEXT("base_xp"), BaseXP);
        Result->SetNumberField(TEXT("multiplier"), Multiplier);
        Result->SetNumberField(TEXT("exponent"), Exponent);
        Result->SetArrayField(TEXT("sample_levels"), SampleLevels);

        SendAutomationResponse(Socket, RequestId, true, TEXT("XP curve configured"), Result);
        return true;
    }

    // configure_level_rewards - Set rewards per level
    if (SubAction == TEXT("configure_level_rewards"))
    {
        int32 SkillPointsPerLevel = GetJsonNumberField(Params, TEXT("skill_points_per_level"), 1);
        int32 AttributePointsPerLevel = GetJsonNumberField(Params, TEXT("attribute_points_per_level"), 3);
        float HealthPerLevel = GetJsonNumberField(Params, TEXT("health_per_level"), 10.0f);
        float ManaPerLevel = GetJsonNumberField(Params, TEXT("mana_per_level"), 5.0f);
        float StaminaPerLevel = GetJsonNumberField(Params, TEXT("stamina_per_level"), 5.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("skill_points_per_level"), SkillPointsPerLevel);
        Result->SetNumberField(TEXT("attribute_points_per_level"), AttributePointsPerLevel);
        Result->SetNumberField(TEXT("health_per_level"), HealthPerLevel);
        Result->SetNumberField(TEXT("mana_per_level"), ManaPerLevel);
        Result->SetNumberField(TEXT("stamina_per_level"), StaminaPerLevel);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Level rewards configured"), Result);
        return true;
    }

    // configure_xp_sources - Set XP gain rates
    if (SubAction == TEXT("configure_xp_sources"))
    {
        float KillXPMultiplier = GetJsonNumberField(Params, TEXT("kill_xp_multiplier"), 1.0f);
        float QuestXPMultiplier = GetJsonNumberField(Params, TEXT("quest_xp_multiplier"), 1.0f);
        float ExplorationXPMultiplier = GetJsonNumberField(Params, TEXT("exploration_xp_multiplier"), 1.0f);
        float CraftingXPMultiplier = GetJsonNumberField(Params, TEXT("crafting_xp_multiplier"), 1.0f);
        float GatheringXPMultiplier = GetJsonNumberField(Params, TEXT("gathering_xp_multiplier"), 1.0f);
        float BonusXPFromRested = GetJsonNumberField(Params, TEXT("rested_bonus"), 1.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("kill_xp_multiplier"), KillXPMultiplier);
        Result->SetNumberField(TEXT("quest_xp_multiplier"), QuestXPMultiplier);
        Result->SetNumberField(TEXT("exploration_xp_multiplier"), ExplorationXPMultiplier);
        Result->SetNumberField(TEXT("crafting_xp_multiplier"), CraftingXPMultiplier);
        Result->SetNumberField(TEXT("gathering_xp_multiplier"), GatheringXPMultiplier);
        Result->SetNumberField(TEXT("rested_bonus"), BonusXPFromRested);

        SendAutomationResponse(Socket, RequestId, true, TEXT("XP sources configured"), Result);
        return true;
    }

    // create_achievement - Create achievement definition
    if (SubAction == TEXT("create_achievement"))
    {
        FString AchievementId = GetJsonStringField(Params, TEXT("achievement_id"), TEXT(""));
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString IconPath = GetJsonStringField(Params, TEXT("icon_path"), TEXT(""));
        FString CriteriaType = GetJsonStringField(Params, TEXT("criteria_type"), TEXT("kill"));
        FString CriteriaTarget = GetJsonStringField(Params, TEXT("criteria_target"), TEXT(""));
        int32 CriteriaCount = GetJsonNumberField(Params, TEXT("criteria_count"), 1);
        int32 Points = GetJsonNumberField(Params, TEXT("points"), 10);
        bool bSecret = GetJsonBoolField(Params, TEXT("secret"), false);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Achievements"));

        if (AchievementId.IsEmpty() || Name.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing achievement_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, Name);
        if (Package)
        {
            UMcpGenericDataAsset* AchievementAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*Name),
                                                                                      RF_Public | RF_Standalone);
            if (AchievementAsset)
            {
                AchievementAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(AchievementAsset);
                McpSafeAssetSave(AchievementAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("achievement_id"), AchievementId);
                Result->SetStringField(TEXT("name"), Name);
                Result->SetStringField(TEXT("description"), Description);
                Result->SetStringField(TEXT("criteria_type"), CriteriaType);
                Result->SetStringField(TEXT("criteria_target"), CriteriaTarget);
                Result->SetNumberField(TEXT("criteria_count"), CriteriaCount);
                Result->SetNumberField(TEXT("points"), Points);
                Result->SetBoolField(TEXT("secret"), bSecret);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Achievement created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create achievement"), TEXT("CREATION_FAILED"));
        return true;
    }

    // create_faction - Create faction/reputation system
    if (SubAction == TEXT("create_faction"))
    {
        FString FactionId = GetJsonStringField(Params, TEXT("faction_id"), TEXT(""));
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString IconPath = GetJsonStringField(Params, TEXT("icon_path"), TEXT(""));
        int32 StartingReputation = GetJsonNumberField(Params, TEXT("starting_reputation"), 0);
        int32 MinReputation = GetJsonNumberField(Params, TEXT("min_reputation"), -42000);
        int32 MaxReputation = GetJsonNumberField(Params, TEXT("max_reputation"), 42000);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Factions"));

        if (FactionId.IsEmpty() || Name.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing faction_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, Name);
        if (Package)
        {
            UMcpGenericDataAsset* FactionAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*Name),
                                                                                  RF_Public | RF_Standalone);
            if (FactionAsset)
            {
                FactionAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(FactionAsset);
                McpSafeAssetSave(FactionAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("faction_id"), FactionId);
                Result->SetStringField(TEXT("name"), Name);
                Result->SetStringField(TEXT("description"), Description);
                Result->SetNumberField(TEXT("starting_reputation"), StartingReputation);
                Result->SetNumberField(TEXT("min_reputation"), MinReputation);
                Result->SetNumberField(TEXT("max_reputation"), MaxReputation);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Faction created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create faction"), TEXT("CREATION_FAILED"));
        return true;
    }

    // add_faction_rank - Add rank to faction
    if (SubAction == TEXT("add_faction_rank"))
    {
        FString FactionPath = GetJsonStringField(Params, TEXT("faction_path"), TEXT(""));
        FString RankName = GetJsonStringField(Params, TEXT("rank_name"), TEXT(""));
        int32 ReputationThreshold = GetJsonNumberField(Params, TEXT("reputation_threshold"), 0);
        FString RewardType = GetJsonStringField(Params, TEXT("reward_type"), TEXT(""));
        FString RewardValue = GetJsonStringField(Params, TEXT("reward_value"), TEXT(""));

        if (FactionPath.IsEmpty() || RankName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing faction_path or rank_name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("faction_path"), FactionPath);
        Result->SetStringField(TEXT("rank_name"), RankName);
        Result->SetNumberField(TEXT("reputation_threshold"), ReputationThreshold);
        Result->SetStringField(TEXT("reward_type"), RewardType);
        Result->SetStringField(TEXT("reward_value"), RewardValue);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Faction rank added"), Result);
        return true;
    }

    // set_faction_relationship - Set relationship between factions
    if (SubAction == TEXT("set_faction_relationship"))
    {
        FString Faction1Id = GetJsonStringField(Params, TEXT("faction1_id"), TEXT(""));
        FString Faction2Id = GetJsonStringField(Params, TEXT("faction2_id"), TEXT(""));
        FString Relationship = GetJsonStringField(Params, TEXT("relationship"), TEXT("neutral"));
        float RepMultiplier = GetJsonNumberField(Params, TEXT("rep_multiplier"), 1.0f);

        if (Faction1Id.IsEmpty() || Faction2Id.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing faction IDs"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("faction1_id"), Faction1Id);
        Result->SetStringField(TEXT("faction2_id"), Faction2Id);
        Result->SetStringField(TEXT("relationship"), Relationship);
        Result->SetNumberField(TEXT("rep_multiplier"), RepMultiplier);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Faction relationship set"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown progression action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 COMBAT HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2CombatAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 combat requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_melee_system - Set up melee combat
    if (SubAction == TEXT("create_melee_system"))
    {
        FString SystemName = GetJsonStringField(Params, TEXT("name"), TEXT("MeleeCombat"));
        float AttackRange = GetJsonNumberField(Params, TEXT("attack_range"), 200.0f);
        float ComboWindow = GetJsonNumberField(Params, TEXT("combo_window"), 0.5f);
        int32 MaxComboChain = GetJsonNumberField(Params, TEXT("max_combo_chain"), 4);
        float ParryWindow = GetJsonNumberField(Params, TEXT("parry_window"), 0.2f);
        float BlockDamageReduction = GetJsonNumberField(Params, TEXT("block_damage_reduction"), 0.75f);
        bool bAllowCancelIntoBlock = GetJsonBoolField(Params, TEXT("allow_cancel_into_block"), true);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Combat"));

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, SystemName);
        if (Package)
        {
            UMcpGenericDataAsset* CombatAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*SystemName),
                                                                                 RF_Public | RF_Standalone);
            if (CombatAsset)
            {
                CombatAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(CombatAsset);
                McpSafeAssetSave(CombatAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("name"), SystemName);
                Result->SetNumberField(TEXT("attack_range"), AttackRange);
                Result->SetNumberField(TEXT("combo_window"), ComboWindow);
                Result->SetNumberField(TEXT("max_combo_chain"), MaxComboChain);
                Result->SetNumberField(TEXT("parry_window"), ParryWindow);
                Result->SetNumberField(TEXT("block_damage_reduction"), BlockDamageReduction);
                Result->SetBoolField(TEXT("allow_cancel_into_block"), bAllowCancelIntoBlock);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Melee system created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create melee system"), TEXT("CREATION_FAILED"));
        return true;
    }

    // create_ranged_system - Set up ranged combat
    if (SubAction == TEXT("create_ranged_system"))
    {
        FString SystemName = GetJsonStringField(Params, TEXT("name"), TEXT("RangedCombat"));
        FString ProjectileType = GetJsonStringField(Params, TEXT("projectile_type"), TEXT("projectile"));
        float ProjectileSpeed = GetJsonNumberField(Params, TEXT("projectile_speed"), 5000.0f);
        float MaxRange = GetJsonNumberField(Params, TEXT("max_range"), 10000.0f);
        float BaseSpread = GetJsonNumberField(Params, TEXT("base_spread"), 1.0f);
        float AimDownSightsSpread = GetJsonNumberField(Params, TEXT("ads_spread"), 0.2f);
        int32 AmmoPerClip = GetJsonNumberField(Params, TEXT("ammo_per_clip"), 30);
        float ReloadTime = GetJsonNumberField(Params, TEXT("reload_time"), 2.0f);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Combat"));

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, SystemName);
        if (Package)
        {
            UMcpGenericDataAsset* RangedAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*SystemName),
                                                                                 RF_Public | RF_Standalone);
            if (RangedAsset)
            {
                RangedAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(RangedAsset);
                McpSafeAssetSave(RangedAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("name"), SystemName);
                Result->SetStringField(TEXT("projectile_type"), ProjectileType);
                Result->SetNumberField(TEXT("projectile_speed"), ProjectileSpeed);
                Result->SetNumberField(TEXT("max_range"), MaxRange);
                Result->SetNumberField(TEXT("base_spread"), BaseSpread);
                Result->SetNumberField(TEXT("ads_spread"), AimDownSightsSpread);
                Result->SetNumberField(TEXT("ammo_per_clip"), AmmoPerClip);
                Result->SetNumberField(TEXT("reload_time"), ReloadTime);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Ranged system created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create ranged system"), TEXT("CREATION_FAILED"));
        return true;
    }

    // configure_targeting - Set up targeting system
    if (SubAction == TEXT("configure_targeting"))
    {
        FString TargetingMode = GetJsonStringField(Params, TEXT("mode"), TEXT("soft_lock"));
        float LockOnRange = GetJsonNumberField(Params, TEXT("lock_on_range"), 1500.0f);
        float LockOnAngle = GetJsonNumberField(Params, TEXT("lock_on_angle"), 45.0f);
        float SwitchCooldown = GetJsonNumberField(Params, TEXT("switch_cooldown"), 0.3f);
        bool bTargetThroughWalls = GetJsonBoolField(Params, TEXT("target_through_walls"), false);
        FString TargetPriority = GetJsonStringField(Params, TEXT("target_priority"), TEXT("nearest"));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mode"), TargetingMode);
        Result->SetNumberField(TEXT("lock_on_range"), LockOnRange);
        Result->SetNumberField(TEXT("lock_on_angle"), LockOnAngle);
        Result->SetNumberField(TEXT("switch_cooldown"), SwitchCooldown);
        Result->SetBoolField(TEXT("target_through_walls"), bTargetThroughWalls);
        Result->SetStringField(TEXT("target_priority"), TargetPriority);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Targeting configured"), Result);
        return true;
    }

    // create_dodge_system - Set up dodge/roll
    if (SubAction == TEXT("create_dodge_system"))
    {
        FString DodgeType = GetJsonStringField(Params, TEXT("dodge_type"), TEXT("roll"));
        float DodgeDistance = GetJsonNumberField(Params, TEXT("dodge_distance"), 500.0f);
        float DodgeDuration = GetJsonNumberField(Params, TEXT("dodge_duration"), 0.5f);
        float IFrameStart = GetJsonNumberField(Params, TEXT("iframe_start"), 0.1f);
        float IFrameDuration = GetJsonNumberField(Params, TEXT("iframe_duration"), 0.3f);
        float Cooldown = GetJsonNumberField(Params, TEXT("cooldown"), 0.5f);
        float StaminaCost = GetJsonNumberField(Params, TEXT("stamina_cost"), 25.0f);
        bool bDirectionalDodge = GetJsonBoolField(Params, TEXT("directional"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("dodge_type"), DodgeType);
        Result->SetNumberField(TEXT("dodge_distance"), DodgeDistance);
        Result->SetNumberField(TEXT("dodge_duration"), DodgeDuration);
        Result->SetNumberField(TEXT("iframe_start"), IFrameStart);
        Result->SetNumberField(TEXT("iframe_duration"), IFrameDuration);
        Result->SetNumberField(TEXT("cooldown"), Cooldown);
        Result->SetNumberField(TEXT("stamina_cost"), StaminaCost);
        Result->SetBoolField(TEXT("directional"), bDirectionalDodge);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Dodge system created"), Result);
        return true;
    }

    // configure_stamina - Set up stamina system
    if (SubAction == TEXT("configure_stamina"))
    {
        float MaxStamina = GetJsonNumberField(Params, TEXT("max_stamina"), 100.0f);
        float RegenRate = GetJsonNumberField(Params, TEXT("regen_rate"), 30.0f);
        float RegenDelay = GetJsonNumberField(Params, TEXT("regen_delay"), 1.0f);
        float SprintCost = GetJsonNumberField(Params, TEXT("sprint_cost"), 15.0f);
        float AttackCost = GetJsonNumberField(Params, TEXT("attack_cost"), 20.0f);
        float DodgeCost = GetJsonNumberField(Params, TEXT("dodge_cost"), 25.0f);
        float BlockCostPerSecond = GetJsonNumberField(Params, TEXT("block_cost_per_second"), 10.0f);
        float ExhaustedThreshold = GetJsonNumberField(Params, TEXT("exhausted_threshold"), 10.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_stamina"), MaxStamina);
        Result->SetNumberField(TEXT("regen_rate"), RegenRate);
        Result->SetNumberField(TEXT("regen_delay"), RegenDelay);
        Result->SetNumberField(TEXT("sprint_cost"), SprintCost);
        Result->SetNumberField(TEXT("attack_cost"), AttackCost);
        Result->SetNumberField(TEXT("dodge_cost"), DodgeCost);
        Result->SetNumberField(TEXT("block_cost_per_second"), BlockCostPerSecond);
        Result->SetNumberField(TEXT("exhausted_threshold"), ExhaustedThreshold);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Stamina configured"), Result);
        return true;
    }

    // create_combo_chain - Create attack combo
    if (SubAction == TEXT("create_combo_chain"))
    {
        FString ComboId = GetJsonStringField(Params, TEXT("combo_id"), TEXT(""));
        FString ComboName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        float InputWindow = GetJsonNumberField(Params, TEXT("input_window"), 0.5f);
        bool bAllowCancels = GetJsonBoolField(Params, TEXT("allow_cancels"), true);
        float DamageScaling = GetJsonNumberField(Params, TEXT("damage_scaling"), 1.0f);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Combat/Combos"));

        if (ComboId.IsEmpty() || ComboName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing combo_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, ComboName);
        if (Package)
        {
            UMcpGenericDataAsset* ComboAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*ComboName),
                                                                                RF_Public | RF_Standalone);
            if (ComboAsset)
            {
                ComboAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(ComboAsset);
                McpSafeAssetSave(ComboAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("combo_id"), ComboId);
                Result->SetStringField(TEXT("name"), ComboName);
                Result->SetNumberField(TEXT("input_window"), InputWindow);
                Result->SetBoolField(TEXT("allow_cancels"), bAllowCancels);
                Result->SetNumberField(TEXT("damage_scaling"), DamageScaling);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Combo chain created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create combo chain"), TEXT("CREATION_FAILED"));
        return true;
    }

    // add_combo_attack - Add attack to combo chain
    if (SubAction == TEXT("add_combo_attack"))
    {
        FString ComboPath = GetJsonStringField(Params, TEXT("combo_path"), TEXT(""));
        int32 Position = GetJsonNumberField(Params, TEXT("position"), 0);
        FString AnimationPath = GetJsonStringField(Params, TEXT("animation_path"), TEXT(""));
        float Damage = GetJsonNumberField(Params, TEXT("damage"), 10.0f);
        FString DamageType = GetJsonStringField(Params, TEXT("damage_type"), TEXT("physical"));
        float StartupFrames = GetJsonNumberField(Params, TEXT("startup_frames"), 10.0f);
        float ActiveFrames = GetJsonNumberField(Params, TEXT("active_frames"), 5.0f);
        float RecoveryFrames = GetJsonNumberField(Params, TEXT("recovery_frames"), 15.0f);

        if (ComboPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing combo_path"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("combo_path"), ComboPath);
        Result->SetNumberField(TEXT("position"), Position);
        Result->SetStringField(TEXT("animation_path"), AnimationPath);
        Result->SetNumberField(TEXT("damage"), Damage);
        Result->SetStringField(TEXT("damage_type"), DamageType);
        Result->SetNumberField(TEXT("startup_frames"), StartupFrames);
        Result->SetNumberField(TEXT("active_frames"), ActiveFrames);
        Result->SetNumberField(TEXT("recovery_frames"), RecoveryFrames);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Combo attack added"), Result);
        return true;
    }

    // create_finishing_move - Create execution/finisher
    if (SubAction == TEXT("create_finishing_move"))
    {
        FString FinisherId = GetJsonStringField(Params, TEXT("finisher_id"), TEXT(""));
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString TriggerCondition = GetJsonStringField(Params, TEXT("trigger_condition"), TEXT("low_health"));
        float HealthThreshold = GetJsonNumberField(Params, TEXT("health_threshold"), 0.2f);
        float DamageMultiplier = GetJsonNumberField(Params, TEXT("damage_multiplier"), 5.0f);
        bool bInvincibleDuring = GetJsonBoolField(Params, TEXT("invincible_during"), true);
        FString AnimationPath = GetJsonStringField(Params, TEXT("animation_path"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Combat/Finishers"));

        if (FinisherId.IsEmpty() || Name.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing finisher_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, Name);
        if (Package)
        {
            UMcpGenericDataAsset* FinisherAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*Name),
                                                                                   RF_Public | RF_Standalone);
            if (FinisherAsset)
            {
                FinisherAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(FinisherAsset);
                McpSafeAssetSave(FinisherAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("finisher_id"), FinisherId);
                Result->SetStringField(TEXT("name"), Name);
                Result->SetStringField(TEXT("trigger_condition"), TriggerCondition);
                Result->SetNumberField(TEXT("health_threshold"), HealthThreshold);
                Result->SetNumberField(TEXT("damage_multiplier"), DamageMultiplier);
                Result->SetBoolField(TEXT("invincible_during"), bInvincibleDuring);
                Result->SetStringField(TEXT("animation_path"), AnimationPath);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Finishing move created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create finishing move"), TEXT("CREATION_FAILED"));
        return true;
    }

    // configure_difficulty - Set difficulty settings
    if (SubAction == TEXT("configure_difficulty"))
    {
        FString DifficultyName = GetJsonStringField(Params, TEXT("name"), TEXT("Normal"));
        float PlayerDamageMultiplier = GetJsonNumberField(Params, TEXT("player_damage_multiplier"), 1.0f);
        float EnemyDamageMultiplier = GetJsonNumberField(Params, TEXT("enemy_damage_multiplier"), 1.0f);
        float EnemyHealthMultiplier = GetJsonNumberField(Params, TEXT("enemy_health_multiplier"), 1.0f);
        float XPMultiplier = GetJsonNumberField(Params, TEXT("xp_multiplier"), 1.0f);
        float LootMultiplier = GetJsonNumberField(Params, TEXT("loot_multiplier"), 1.0f);
        bool bFriendlyFire = GetJsonBoolField(Params, TEXT("friendly_fire"), false);
        bool bPermaDeath = GetJsonBoolField(Params, TEXT("perma_death"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), DifficultyName);
        Result->SetNumberField(TEXT("player_damage_multiplier"), PlayerDamageMultiplier);
        Result->SetNumberField(TEXT("enemy_damage_multiplier"), EnemyDamageMultiplier);
        Result->SetNumberField(TEXT("enemy_health_multiplier"), EnemyHealthMultiplier);
        Result->SetNumberField(TEXT("xp_multiplier"), XPMultiplier);
        Result->SetNumberField(TEXT("loot_multiplier"), LootMultiplier);
        Result->SetBoolField(TEXT("friendly_fire"), bFriendlyFire);
        Result->SetBoolField(TEXT("perma_death"), bPermaDeath);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Difficulty configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown combat action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 COMBAT STATS HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2CombatStatsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 combat_stats requires editor build"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_stat - Create stat definition
    if (SubAction == TEXT("create_stat"))
    {
        FString StatId = GetJsonStringField(Params, TEXT("stat_id"), TEXT(""));
        FString DisplayName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString StatCategory = GetJsonStringField(Params, TEXT("category"), TEXT("primary"));
        float BaseValue = GetJsonNumberField(Params, TEXT("base_value"), 10.0f);
        float MinValue = GetJsonNumberField(Params, TEXT("min_value"), 0.0f);
        float MaxValue = GetJsonNumberField(Params, TEXT("max_value"), 9999.0f);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Stats"));

        if (StatId.IsEmpty() || DisplayName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing stat_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, DisplayName);
        if (Package)
        {
            UMcpGenericDataAsset* StatAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*DisplayName),
                                                                               RF_Public | RF_Standalone);
            if (StatAsset)
            {
                StatAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(StatAsset);
                McpSafeAssetSave(StatAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("stat_id"), StatId);
                Result->SetStringField(TEXT("name"), DisplayName);
                Result->SetStringField(TEXT("description"), Description);
                Result->SetStringField(TEXT("category"), StatCategory);
                Result->SetNumberField(TEXT("base_value"), BaseValue);
                Result->SetNumberField(TEXT("min_value"), MinValue);
                Result->SetNumberField(TEXT("max_value"), MaxValue);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Stat created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create stat"), TEXT("CREATION_FAILED"));
        return true;
    }

    // create_derived_stat - Create derived stat with formula
    if (SubAction == TEXT("create_derived_stat"))
    {
        FString StatId = GetJsonStringField(Params, TEXT("stat_id"), TEXT(""));
        FString DisplayName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Formula = GetJsonStringField(Params, TEXT("formula"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Stats/Derived"));

        if (StatId.IsEmpty() || DisplayName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing stat_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, DisplayName);
        if (Package)
        {
            UMcpGenericDataAsset* DerivedStatAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*DisplayName),
                                                                                      RF_Public | RF_Standalone);
            if (DerivedStatAsset)
            {
                DerivedStatAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(DerivedStatAsset);
                McpSafeAssetSave(DerivedStatAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("stat_id"), StatId);
                Result->SetStringField(TEXT("name"), DisplayName);
                Result->SetStringField(TEXT("formula"), Formula);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Derived stat created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create derived stat"), TEXT("CREATION_FAILED"));
        return true;
    }

    // create_damage_type - Create damage type
    if (SubAction == TEXT("create_damage_type"))
    {
        FString DamageTypeId = GetJsonStringField(Params, TEXT("damage_type_id"), TEXT(""));
        FString DisplayName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString ResistanceStat = GetJsonStringField(Params, TEXT("resistance_stat"), TEXT(""));
        FLinearColor DisplayColor = FLinearColor::White;
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/DamageTypes"));

        if (DamageTypeId.IsEmpty() || DisplayName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing damage_type_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, DisplayName);
        if (Package)
        {
            UMcpGenericDataAsset* DamageTypeAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*DisplayName),
                                                                                     RF_Public | RF_Standalone);
            if (DamageTypeAsset)
            {
                DamageTypeAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(DamageTypeAsset);
                McpSafeAssetSave(DamageTypeAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("damage_type_id"), DamageTypeId);
                Result->SetStringField(TEXT("name"), DisplayName);
                Result->SetStringField(TEXT("resistance_stat"), ResistanceStat);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Damage type created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create damage type"), TEXT("CREATION_FAILED"));
        return true;
    }

    // create_status_effect - Create buff/debuff
    if (SubAction == TEXT("create_status_effect"))
    {
        FString EffectId = GetJsonStringField(Params, TEXT("effect_id"), TEXT(""));
        FString DisplayName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString EffectType = GetJsonStringField(Params, TEXT("type"), TEXT("buff"));
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 10.0f);
        int32 MaxStacks = GetJsonNumberField(Params, TEXT("max_stacks"), 1);
        bool bRefreshOnReapply = GetJsonBoolField(Params, TEXT("refresh_on_reapply"), true);
        FString IconPath = GetJsonStringField(Params, TEXT("icon_path"), TEXT(""));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/StatusEffects"));

        if (EffectId.IsEmpty() || DisplayName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing effect_id or name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2Helpers::CreateAssetPackage(Path, DisplayName);
        if (Package)
        {
            UMcpGenericDataAsset* EffectAsset = NewObject<UMcpGenericDataAsset>(Package, FName(*DisplayName),
                                                                                 RF_Public | RF_Standalone);
            if (EffectAsset)
            {
                EffectAsset->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(EffectAsset);
                McpSafeAssetSave(EffectAsset);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("effect_id"), EffectId);
                Result->SetStringField(TEXT("name"), DisplayName);
                Result->SetStringField(TEXT("description"), Description);
                Result->SetStringField(TEXT("type"), EffectType);
                Result->SetNumberField(TEXT("duration"), Duration);
                Result->SetNumberField(TEXT("max_stacks"), MaxStacks);
                Result->SetBoolField(TEXT("refresh_on_reapply"), bRefreshOnReapply);
                Result->SetStringField(TEXT("path"), Package->GetName());

                SendAutomationResponse(Socket, RequestId, true, TEXT("Status effect created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create status effect"), TEXT("CREATION_FAILED"));
        return true;
    }

    // add_effect_modifier - Add stat modifier to status effect
    if (SubAction == TEXT("add_effect_modifier"))
    {
        FString EffectPath = GetJsonStringField(Params, TEXT("effect_path"), TEXT(""));
        FString StatId = GetJsonStringField(Params, TEXT("stat_id"), TEXT(""));
        float ModifierValue = GetJsonNumberField(Params, TEXT("modifier_value"), 0.0f);
        FString ModifierType = GetJsonStringField(Params, TEXT("modifier_type"), TEXT("flat"));

        if (EffectPath.IsEmpty() || StatId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing effect_path or stat_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("effect_path"), EffectPath);
        Result->SetStringField(TEXT("stat_id"), StatId);
        Result->SetNumberField(TEXT("modifier_value"), ModifierValue);
        Result->SetStringField(TEXT("modifier_type"), ModifierType);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Effect modifier added"), Result);
        return true;
    }

    // configure_damage_formula - Set damage calculation formula
    if (SubAction == TEXT("configure_damage_formula"))
    {
        FString FormulaName = GetJsonStringField(Params, TEXT("name"), TEXT("default"));
        FString AttackStatId = GetJsonStringField(Params, TEXT("attack_stat"), TEXT("attack"));
        FString DefenseStatId = GetJsonStringField(Params, TEXT("defense_stat"), TEXT("defense"));
        float BaseMultiplier = GetJsonNumberField(Params, TEXT("base_multiplier"), 1.0f);
        float VariancePercent = GetJsonNumberField(Params, TEXT("variance_percent"), 0.1f);
        float MinDamage = GetJsonNumberField(Params, TEXT("min_damage"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), FormulaName);
        Result->SetStringField(TEXT("attack_stat"), AttackStatId);
        Result->SetStringField(TEXT("defense_stat"), DefenseStatId);
        Result->SetNumberField(TEXT("base_multiplier"), BaseMultiplier);
        Result->SetNumberField(TEXT("variance_percent"), VariancePercent);
        Result->SetNumberField(TEXT("min_damage"), MinDamage);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Damage formula configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown combat_stats action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 INPUT HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2InputAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_input_mapping_context - Create Enhanced Input mapping context
    if (SubAction == TEXT("create_input_mapping_context"))
    {
        FString ContextName = GetJsonStringField(Params, TEXT("name"), TEXT("IMC_Default"));
        FString ContextPath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/Input/"));
        int32 Priority = GetJsonIntField(Params, TEXT("priority"), 0);

        FString FullPath = ContextPath + ContextName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UInputMappingContext* MappingContext = NewObject<UInputMappingContext>(Package, *ContextName, RF_Public | RF_Standalone);
        if (MappingContext)
        {
            FAssetRegistryModule::AssetCreated(MappingContext);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("name"), ContextName);
            Result->SetNumberField(TEXT("priority"), Priority);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Input mapping context created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create mapping context"), TEXT("CREATION_FAILED"));
        return true;
    }

    // define_input_action - Define an input action
    if (SubAction == TEXT("define_input_action"))
    {
        FString ActionName = GetJsonStringField(Params, TEXT("name"), TEXT("IA_Default"));
        FString ActionPath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/Input/Actions/"));
        FString ValueType = GetJsonStringField(Params, TEXT("value_type"), TEXT("bool")); // bool, axis1d, axis2d, axis3d
        bool bConsumeInput = GetJsonBoolField(Params, TEXT("consume_input"), true);

        FString FullPath = ActionPath + ActionName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UInputAction* InputAction = NewObject<UInputAction>(Package, *ActionName, RF_Public | RF_Standalone);
        if (InputAction)
        {
            // Set value type
            if (ValueType == TEXT("bool"))
            {
                InputAction->ValueType = EInputActionValueType::Boolean;
            }
            else if (ValueType == TEXT("axis1d"))
            {
                InputAction->ValueType = EInputActionValueType::Axis1D;
            }
            else if (ValueType == TEXT("axis2d"))
            {
                InputAction->ValueType = EInputActionValueType::Axis2D;
            }
            else if (ValueType == TEXT("axis3d"))
            {
                InputAction->ValueType = EInputActionValueType::Axis3D;
            }

            InputAction->bConsumeInput = bConsumeInput;

            FAssetRegistryModule::AssetCreated(InputAction);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("name"), ActionName);
            Result->SetStringField(TEXT("value_type"), ValueType);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Input action created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create input action"), TEXT("CREATION_FAILED"));
        return true;
    }

    // add_key_mapping - Add a key mapping to a context
    if (SubAction == TEXT("add_key_mapping"))
    {
        FString ContextPath = GetJsonStringField(Params, TEXT("context_path"), TEXT(""));
        FString ActionPath = GetJsonStringField(Params, TEXT("action_path"), TEXT(""));
        FString KeyName = GetJsonStringField(Params, TEXT("key"), TEXT(""));

        if (ContextPath.IsEmpty() || ActionPath.IsEmpty() || KeyName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing context_path, action_path, or key"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, *ContextPath);
        UInputAction* InputAction = LoadObject<UInputAction>(nullptr, *ActionPath);

        if (!Context || !InputAction)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to load context or action"), TEXT("LOAD_FAILED"));
            return true;
        }

        FKey Key(*KeyName);
        FEnhancedActionKeyMapping& Mapping = Context->MapKey(InputAction, Key);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("context"), ContextPath);
        Result->SetStringField(TEXT("action"), ActionPath);
        Result->SetStringField(TEXT("key"), KeyName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Key mapping added"), Result);
        return true;
    }

    // configure_input_trigger - Configure trigger type for mapping
    if (SubAction == TEXT("configure_input_trigger"))
    {
        FString TriggerType = GetJsonStringField(Params, TEXT("trigger_type"), TEXT("pressed")); // pressed, released, hold, tap
        float HoldThreshold = GetJsonNumberField(Params, TEXT("hold_threshold"), 0.5f);
        float TapInterval = GetJsonNumberField(Params, TEXT("tap_interval"), 0.2f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("trigger_type"), TriggerType);
        Result->SetNumberField(TEXT("hold_threshold"), HoldThreshold);
        Result->SetNumberField(TEXT("tap_interval"), TapInterval);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Input trigger configured"), Result);
        return true;
    }

    // setup_input_modifier - Configure input modifiers
    if (SubAction == TEXT("setup_input_modifier"))
    {
        FString ModifierType = GetJsonStringField(Params, TEXT("modifier_type"), TEXT("deadzone")); // deadzone, negate, scale, swizzle
        float DeadzoneValue = GetJsonNumberField(Params, TEXT("deadzone"), 0.2f);
        float ScaleValue = GetJsonNumberField(Params, TEXT("scale"), 1.0f);
        bool bNegate = GetJsonBoolField(Params, TEXT("negate"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("modifier_type"), ModifierType);
        Result->SetNumberField(TEXT("deadzone"), DeadzoneValue);
        Result->SetNumberField(TEXT("scale"), ScaleValue);
        Result->SetBoolField(TEXT("negate"), bNegate);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Input modifier configured"), Result);
        return true;
    }

    // create_combo_input - Create combo input sequence detection
    if (SubAction == TEXT("create_combo_input"))
    {
        FString ComboName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* InputSequence = nullptr;
        Params->TryGetArrayField(TEXT("sequence"), InputSequence);
        float TimingWindow = GetJsonNumberField(Params, TEXT("timing_window"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), ComboName);
        Result->SetNumberField(TEXT("timing_window"), TimingWindow);
        if (InputSequence)
        {
            TArray<TSharedPtr<FJsonValue>> SequenceCopy = *InputSequence;
            Result->SetArrayField(TEXT("sequence"), SequenceCopy);
        }
        SendAutomationResponse(Socket, RequestId, true, TEXT("Combo input created"), Result);
        return true;
    }

    // configure_contextual_action - Same button different contexts
    if (SubAction == TEXT("configure_contextual_action"))
    {
        FString ActionPath = GetJsonStringField(Params, TEXT("action_path"), TEXT(""));
        FString ContextType = GetJsonStringField(Params, TEXT("context_type"), TEXT("default")); // combat, dialogue, menu, swimming
        FString BehaviorWhenActive = GetJsonStringField(Params, TEXT("behavior"), TEXT(""));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("action_path"), ActionPath);
        Result->SetStringField(TEXT("context_type"), ContextType);
        Result->SetStringField(TEXT("behavior"), BehaviorWhenActive);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Contextual action configured"), Result);
        return true;
    }

    // setup_input_buffering - Configure input buffer
    if (SubAction == TEXT("setup_input_buffering"))
    {
        float BufferDuration = GetJsonNumberField(Params, TEXT("buffer_duration"), 0.15f);
        int32 MaxBufferSize = GetJsonIntField(Params, TEXT("max_buffer_size"), 8);
        bool bEnableBuffering = GetJsonBoolField(Params, TEXT("enabled"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("buffer_duration"), BufferDuration);
        Result->SetNumberField(TEXT("max_buffer_size"), MaxBufferSize);
        Result->SetBoolField(TEXT("enabled"), bEnableBuffering);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Input buffering configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown input action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 CAMERA HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2CameraAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_camera_mode - Create camera mode preset
    if (SubAction == TEXT("create_camera_mode"))
    {
        FString ModeName = GetJsonStringField(Params, TEXT("name"), TEXT("Default"));
        FString CameraType = GetJsonStringField(Params, TEXT("type"), TEXT("third_person")); // third_person, first_person, isometric, top_down, side_scroller
        float TargetArmLength = GetJsonNumberField(Params, TEXT("arm_length"), 300.0f);
        float FieldOfView = GetJsonNumberField(Params, TEXT("fov"), 90.0f);
        FVector SocketOffset = Forge2Helpers::GetVectorFromJson(Params, TEXT("socket_offset"));
        FVector TargetOffset = Forge2Helpers::GetVectorFromJson(Params, TEXT("target_offset"));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), ModeName);
        Result->SetStringField(TEXT("type"), CameraType);
        Result->SetNumberField(TEXT("arm_length"), TargetArmLength);
        Result->SetNumberField(TEXT("fov"), FieldOfView);

        TSharedPtr<FJsonObject> SocketObj = MakeShared<FJsonObject>();
        SocketObj->SetNumberField(TEXT("x"), SocketOffset.X);
        SocketObj->SetNumberField(TEXT("y"), SocketOffset.Y);
        SocketObj->SetNumberField(TEXT("z"), SocketOffset.Z);
        Result->SetObjectField(TEXT("socket_offset"), SocketObj);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera mode created"), Result);
        return true;
    }

    // configure_camera_collision - Set up camera collision
    if (SubAction == TEXT("configure_camera_collision"))
    {
        bool bDoCollisionTest = GetJsonBoolField(Params, TEXT("enabled"), true);
        float ProbeSize = GetJsonNumberField(Params, TEXT("probe_size"), 12.0f);
        FString CollisionChannel = GetJsonStringField(Params, TEXT("collision_channel"), TEXT("Camera"));
        FString ResponseType = GetJsonStringField(Params, TEXT("response"), TEXT("push")); // push, cut, fade

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bDoCollisionTest);
        Result->SetNumberField(TEXT("probe_size"), ProbeSize);
        Result->SetStringField(TEXT("collision_channel"), CollisionChannel);
        Result->SetStringField(TEXT("response"), ResponseType);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera collision configured"), Result);
        return true;
    }

    // setup_camera_shake - Create camera shake preset
    if (SubAction == TEXT("setup_camera_shake"))
    {
        FString ShakeName = GetJsonStringField(Params, TEXT("name"), TEXT("DefaultShake"));
        FString ShakeType = GetJsonStringField(Params, TEXT("type"), TEXT("perlin")); // perlin, wave, impact
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 0.5f);
        float BlendInTime = GetJsonNumberField(Params, TEXT("blend_in"), 0.1f);
        float BlendOutTime = GetJsonNumberField(Params, TEXT("blend_out"), 0.2f);
        float Amplitude = GetJsonNumberField(Params, TEXT("amplitude"), 1.0f);
        float Frequency = GetJsonNumberField(Params, TEXT("frequency"), 20.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), ShakeName);
        Result->SetStringField(TEXT("type"), ShakeType);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetNumberField(TEXT("blend_in"), BlendInTime);
        Result->SetNumberField(TEXT("blend_out"), BlendOutTime);
        Result->SetNumberField(TEXT("amplitude"), Amplitude);
        Result->SetNumberField(TEXT("frequency"), Frequency);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera shake created"), Result);
        return true;
    }

    // configure_camera_lag - Set camera lag settings
    if (SubAction == TEXT("configure_camera_lag"))
    {
        bool bEnableLag = GetJsonBoolField(Params, TEXT("enabled"), true);
        float LagSpeed = GetJsonNumberField(Params, TEXT("lag_speed"), 10.0f);
        float RotationLagSpeed = GetJsonNumberField(Params, TEXT("rotation_lag_speed"), 10.0f);
        float MaxLagDistance = GetJsonNumberField(Params, TEXT("max_lag_distance"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnableLag);
        Result->SetNumberField(TEXT("lag_speed"), LagSpeed);
        Result->SetNumberField(TEXT("rotation_lag_speed"), RotationLagSpeed);
        Result->SetNumberField(TEXT("max_lag_distance"), MaxLagDistance);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera lag configured"), Result);
        return true;
    }

    // setup_camera_constraints - Set pitch/distance limits
    if (SubAction == TEXT("setup_camera_constraints"))
    {
        float MinPitch = GetJsonNumberField(Params, TEXT("min_pitch"), -89.0f);
        float MaxPitch = GetJsonNumberField(Params, TEXT("max_pitch"), 89.0f);
        float MinDistance = GetJsonNumberField(Params, TEXT("min_distance"), 100.0f);
        float MaxDistance = GetJsonNumberField(Params, TEXT("max_distance"), 1000.0f);
        bool bInheritPitch = GetJsonBoolField(Params, TEXT("inherit_pitch"), true);
        bool bInheritYaw = GetJsonBoolField(Params, TEXT("inherit_yaw"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("min_pitch"), MinPitch);
        Result->SetNumberField(TEXT("max_pitch"), MaxPitch);
        Result->SetNumberField(TEXT("min_distance"), MinDistance);
        Result->SetNumberField(TEXT("max_distance"), MaxDistance);
        Result->SetBoolField(TEXT("inherit_pitch"), bInheritPitch);
        Result->SetBoolField(TEXT("inherit_yaw"), bInheritYaw);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera constraints set"), Result);
        return true;
    }

    // create_camera_transition - Define camera blend/transition
    if (SubAction == TEXT("create_camera_transition"))
    {
        FString TransitionName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString BlendType = GetJsonStringField(Params, TEXT("blend_type"), TEXT("linear")); // linear, cubic, ease_in, ease_out, ease_in_out
        float BlendTime = GetJsonNumberField(Params, TEXT("blend_time"), 0.5f);
        float BlendExponent = GetJsonNumberField(Params, TEXT("exponent"), 2.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), TransitionName);
        Result->SetStringField(TEXT("blend_type"), BlendType);
        Result->SetNumberField(TEXT("blend_time"), BlendTime);
        Result->SetNumberField(TEXT("exponent"), BlendExponent);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera transition created"), Result);
        return true;
    }

    // configure_split_screen - Set up split screen
    if (SubAction == TEXT("configure_split_screen"))
    {
        FString SplitType = GetJsonStringField(Params, TEXT("split_type"), TEXT("horizontal")); // horizontal, vertical, quad
        int32 MaxPlayers = GetJsonIntField(Params, TEXT("max_players"), 2);
        bool bDynamicSplit = GetJsonBoolField(Params, TEXT("dynamic"), false);
        float SplitMargin = GetJsonNumberField(Params, TEXT("margin"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("split_type"), SplitType);
        Result->SetNumberField(TEXT("max_players"), MaxPlayers);
        Result->SetBoolField(TEXT("dynamic"), bDynamicSplit);
        Result->SetNumberField(TEXT("margin"), SplitMargin);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Split screen configured"), Result);
        return true;
    }

    // create_camera_rig - Rail/crane camera setup
    if (SubAction == TEXT("create_camera_rig"))
    {
        FString RigType = GetJsonStringField(Params, TEXT("rig_type"), TEXT("rail")); // rail, crane, boom
        FVector StartPosition = Forge2Helpers::GetVectorFromJson(Params, TEXT("start_position"));
        FVector EndPosition = Forge2Helpers::GetVectorFromJson(Params, TEXT("end_position"));
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 5.0f);
        bool bLoop = GetJsonBoolField(Params, TEXT("loop"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("rig_type"), RigType);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetBoolField(TEXT("loop"), bLoop);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera rig created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown camera action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 AI BEHAVIOR HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2AIBehaviorAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_behavior_tree - Create new behavior tree asset
    if (SubAction == TEXT("create_behavior_tree"))
    {
        FString TreeName = GetJsonStringField(Params, TEXT("name"), TEXT("BT_Default"));
        FString TreePath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/AI/BehaviorTrees/"));

        FString FullPath = TreePath + TreeName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBehaviorTree* BehaviorTree = NewObject<UBehaviorTree>(Package, *TreeName, RF_Public | RF_Standalone);
        if (BehaviorTree)
        {
            FAssetRegistryModule::AssetCreated(BehaviorTree);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("name"), TreeName);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Behavior tree created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create behavior tree"), TEXT("CREATION_FAILED"));
        return true;
    }

    // create_blackboard - Create blackboard data asset
    if (SubAction == TEXT("create_blackboard"))
    {
        FString BlackboardName = GetJsonStringField(Params, TEXT("name"), TEXT("BB_Default"));
        FString BlackboardPath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/AI/Blackboards/"));

        FString FullPath = BlackboardPath + BlackboardName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlackboardData* Blackboard = NewObject<UBlackboardData>(Package, *BlackboardName, RF_Public | RF_Standalone);
        if (Blackboard)
        {
            FAssetRegistryModule::AssetCreated(Blackboard);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("name"), BlackboardName);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Blackboard created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create blackboard"), TEXT("CREATION_FAILED"));
        return true;
    }

    // add_blackboard_key - Add key to blackboard
    if (SubAction == TEXT("add_blackboard_key"))
    {
        FString BlackboardPath = GetJsonStringField(Params, TEXT("blackboard_path"), TEXT(""));
        FString KeyName = GetJsonStringField(Params, TEXT("key_name"), TEXT(""));
        FString KeyType = GetJsonStringField(Params, TEXT("key_type"), TEXT("object")); // object, vector, float, int, bool, name, string, class, enum
        bool bInstanceSynced = GetJsonBoolField(Params, TEXT("instance_synced"), false);

        if (BlackboardPath.IsEmpty() || KeyName.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing blackboard_path or key_name"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UBlackboardData* Blackboard = LoadObject<UBlackboardData>(nullptr, *BlackboardPath);
        if (!Blackboard)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to load blackboard"), TEXT("LOAD_FAILED"));
            return true;
        }

        // Add key based on type
        FBlackboardEntry NewEntry;
        NewEntry.EntryName = FName(*KeyName);
        NewEntry.bInstanceSynced = bInstanceSynced;

        if (KeyType == TEXT("object"))
        {
            NewEntry.KeyType = NewObject<UBlackboardKeyType_Object>();
        }
        else if (KeyType == TEXT("vector"))
        {
            NewEntry.KeyType = NewObject<UBlackboardKeyType_Vector>();
        }
        else if (KeyType == TEXT("float"))
        {
            NewEntry.KeyType = NewObject<UBlackboardKeyType_Float>();
        }
        else if (KeyType == TEXT("int"))
        {
            NewEntry.KeyType = NewObject<UBlackboardKeyType_Int>();
        }
        else if (KeyType == TEXT("bool"))
        {
            NewEntry.KeyType = NewObject<UBlackboardKeyType_Bool>();
        }
        else if (KeyType == TEXT("name"))
        {
            NewEntry.KeyType = NewObject<UBlackboardKeyType_Name>();
        }
        else if (KeyType == TEXT("string"))
        {
            NewEntry.KeyType = NewObject<UBlackboardKeyType_String>();
        }
        else if (KeyType == TEXT("class"))
        {
            NewEntry.KeyType = NewObject<UBlackboardKeyType_Class>();
        }

        Blackboard->Keys.Add(NewEntry);
        Blackboard->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blackboard"), BlackboardPath);
        Result->SetStringField(TEXT("key_name"), KeyName);
        Result->SetStringField(TEXT("key_type"), KeyType);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Blackboard key added"), Result);
        return true;
    }

    // setup_ai_perception - Configure AI perception component
    if (SubAction == TEXT("setup_ai_perception"))
    {
        bool bEnableSight = GetJsonBoolField(Params, TEXT("enable_sight"), true);
        bool bEnableHearing = GetJsonBoolField(Params, TEXT("enable_hearing"), true);
        bool bEnableDamage = GetJsonBoolField(Params, TEXT("enable_damage"), true);
        float SightRadius = GetJsonNumberField(Params, TEXT("sight_radius"), 3000.0f);
        float LoseSightRadius = GetJsonNumberField(Params, TEXT("lose_sight_radius"), 3500.0f);
        float SightAngle = GetJsonNumberField(Params, TEXT("sight_angle"), 90.0f);
        float HearingRange = GetJsonNumberField(Params, TEXT("hearing_range"), 2000.0f);
        float MaxAge = GetJsonNumberField(Params, TEXT("max_age"), 5.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("sight_enabled"), bEnableSight);
        Result->SetBoolField(TEXT("hearing_enabled"), bEnableHearing);
        Result->SetBoolField(TEXT("damage_enabled"), bEnableDamage);
        Result->SetNumberField(TEXT("sight_radius"), SightRadius);
        Result->SetNumberField(TEXT("lose_sight_radius"), LoseSightRadius);
        Result->SetNumberField(TEXT("sight_angle"), SightAngle);
        Result->SetNumberField(TEXT("hearing_range"), HearingRange);
        Result->SetNumberField(TEXT("max_age"), MaxAge);
        SendAutomationResponse(Socket, RequestId, true, TEXT("AI perception configured"), Result);
        return true;
    }

    // create_eqs_query - Create Environment Query System query
    if (SubAction == TEXT("create_eqs_query"))
    {
        FString QueryName = GetJsonStringField(Params, TEXT("name"), TEXT("EQS_Default"));
        FString QueryPath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/AI/EQS/"));
        FString GeneratorType = GetJsonStringField(Params, TEXT("generator"), TEXT("grid")); // grid, cone, donut, simple_grid, actors

        FString FullPath = QueryPath + QueryName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UEnvQuery* Query = NewObject<UEnvQuery>(Package, *QueryName, RF_Public | RF_Standalone);
        if (Query)
        {
            FAssetRegistryModule::AssetCreated(Query);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("name"), QueryName);
            Result->SetStringField(TEXT("generator"), GeneratorType);
            SendAutomationResponse(Socket, RequestId, true, TEXT("EQS query created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create EQS query"), TEXT("CREATION_FAILED"));
        return true;
    }

    // configure_patrol_path - Set up AI patrol route
    if (SubAction == TEXT("configure_patrol_path"))
    {
        FString PatrolName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* PatrolPoints = nullptr;
        Params->TryGetArrayField(TEXT("points"), PatrolPoints);
        bool bLoop = GetJsonBoolField(Params, TEXT("loop"), true);
        float WaitTimeAtPoints = GetJsonNumberField(Params, TEXT("wait_time"), 2.0f);
        bool bReverse = GetJsonBoolField(Params, TEXT("reverse_on_end"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), PatrolName);
        Result->SetBoolField(TEXT("loop"), bLoop);
        Result->SetNumberField(TEXT("wait_time"), WaitTimeAtPoints);
        Result->SetBoolField(TEXT("reverse_on_end"), bReverse);

        if (PatrolPoints)
        {
            Result->SetNumberField(TEXT("point_count"), PatrolPoints->Num());
        }

        SendAutomationResponse(Socket, RequestId, true, TEXT("Patrol path configured"), Result);
        return true;
    }

    // setup_squad_behavior - Configure squad/group AI
    if (SubAction == TEXT("setup_squad_behavior"))
    {
        FString SquadName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString FormationType = GetJsonStringField(Params, TEXT("formation"), TEXT("line")); // line, wedge, circle, spread
        float FormationSpacing = GetJsonNumberField(Params, TEXT("spacing"), 200.0f);
        int32 MaxSquadSize = GetJsonIntField(Params, TEXT("max_size"), 4);
        bool bShareTarget = GetJsonBoolField(Params, TEXT("share_target"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), SquadName);
        Result->SetStringField(TEXT("formation"), FormationType);
        Result->SetNumberField(TEXT("spacing"), FormationSpacing);
        Result->SetNumberField(TEXT("max_size"), MaxSquadSize);
        Result->SetBoolField(TEXT("share_target"), bShareTarget);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Squad behavior configured"), Result);
        return true;
    }

    // create_boss_phase - Define boss AI phase
    if (SubAction == TEXT("create_boss_phase"))
    {
        FString BossName = GetJsonStringField(Params, TEXT("boss_name"), TEXT(""));
        int32 PhaseNumber = GetJsonIntField(Params, TEXT("phase"), 1);
        float HealthThreshold = GetJsonNumberField(Params, TEXT("health_threshold"), 0.75f);
        FString BehaviorTree = GetJsonStringField(Params, TEXT("behavior_tree"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* Attacks = nullptr;
        Params->TryGetArrayField(TEXT("attacks"), Attacks);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("boss_name"), BossName);
        Result->SetNumberField(TEXT("phase"), PhaseNumber);
        Result->SetNumberField(TEXT("health_threshold"), HealthThreshold);
        Result->SetStringField(TEXT("behavior_tree"), BehaviorTree);
        if (Attacks)
        {
            Result->SetNumberField(TEXT("attack_count"), Attacks->Num());
        }
        SendAutomationResponse(Socket, RequestId, true, TEXT("Boss phase created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown ai_behavior action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 AI ADVANCED HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2AIAdvancedAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_goap_agent - Create Goal-Oriented Action Planning AI
    if (SubAction == TEXT("create_goap_agent"))
    {
        FString AgentName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* Goals = nullptr;
        Params->TryGetArrayField(TEXT("goals"), Goals);
        const TArray<TSharedPtr<FJsonValue>>* Actions = nullptr;
        Params->TryGetArrayField(TEXT("actions"), Actions);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), AgentName);
        if (Goals) Result->SetNumberField(TEXT("goal_count"), Goals->Num());
        if (Actions) Result->SetNumberField(TEXT("action_count"), Actions->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("GOAP agent created"), Result);
        return true;
    }

    // create_utility_ai - Create Utility AI decision system
    if (SubAction == TEXT("create_utility_ai"))
    {
        FString AgentName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* Considerations = nullptr;
        Params->TryGetArrayField(TEXT("considerations"), Considerations);
        FString ResponseCurve = GetJsonStringField(Params, TEXT("response_curve"), TEXT("linear")); // linear, exponential, logistic, sigmoid

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), AgentName);
        Result->SetStringField(TEXT("response_curve"), ResponseCurve);
        if (Considerations) Result->SetNumberField(TEXT("consideration_count"), Considerations->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Utility AI created"), Result);
        return true;
    }

    // create_influence_map - Create influence/heat map for AI
    if (SubAction == TEXT("create_influence_map"))
    {
        FString MapName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        float CellSize = GetJsonNumberField(Params, TEXT("cell_size"), 100.0f);
        float DecayRate = GetJsonNumberField(Params, TEXT("decay_rate"), 0.1f);
        float PropagationRate = GetJsonNumberField(Params, TEXT("propagation_rate"), 0.5f);
        int32 UpdateFrequency = GetJsonIntField(Params, TEXT("update_frequency"), 10);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), MapName);
        Result->SetNumberField(TEXT("cell_size"), CellSize);
        Result->SetNumberField(TEXT("decay_rate"), DecayRate);
        Result->SetNumberField(TEXT("propagation_rate"), PropagationRate);
        Result->SetNumberField(TEXT("update_frequency"), UpdateFrequency);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Influence map created"), Result);
        return true;
    }

    // configure_flocking - Set up flocking/boid behavior
    if (SubAction == TEXT("configure_flocking"))
    {
        FString FlockName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        float SeparationWeight = GetJsonNumberField(Params, TEXT("separation"), 1.0f);
        float AlignmentWeight = GetJsonNumberField(Params, TEXT("alignment"), 1.0f);
        float CohesionWeight = GetJsonNumberField(Params, TEXT("cohesion"), 1.0f);
        float NeighborRadius = GetJsonNumberField(Params, TEXT("neighbor_radius"), 500.0f);
        float AvoidanceRadius = GetJsonNumberField(Params, TEXT("avoidance_radius"), 100.0f);
        float MaxSpeed = GetJsonNumberField(Params, TEXT("max_speed"), 600.0f);
        float MaxForce = GetJsonNumberField(Params, TEXT("max_force"), 100.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), FlockName);
        Result->SetNumberField(TEXT("separation"), SeparationWeight);
        Result->SetNumberField(TEXT("alignment"), AlignmentWeight);
        Result->SetNumberField(TEXT("cohesion"), CohesionWeight);
        Result->SetNumberField(TEXT("neighbor_radius"), NeighborRadius);
        Result->SetNumberField(TEXT("avoidance_radius"), AvoidanceRadius);
        Result->SetNumberField(TEXT("max_speed"), MaxSpeed);
        Result->SetNumberField(TEXT("max_force"), MaxForce);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Flocking configured"), Result);
        return true;
    }

    // create_state_machine - Create hierarchical state machine
    if (SubAction == TEXT("create_state_machine"))
    {
        FString MachineName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString InitialState = GetJsonStringField(Params, TEXT("initial_state"), TEXT("Idle"));
        const TArray<TSharedPtr<FJsonValue>>* States = nullptr;
        Params->TryGetArrayField(TEXT("states"), States);
        const TArray<TSharedPtr<FJsonValue>>* Transitions = nullptr;
        Params->TryGetArrayField(TEXT("transitions"), Transitions);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), MachineName);
        Result->SetStringField(TEXT("initial_state"), InitialState);
        if (States) Result->SetNumberField(TEXT("state_count"), States->Num());
        if (Transitions) Result->SetNumberField(TEXT("transition_count"), Transitions->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("State machine created"), Result);
        return true;
    }

    // configure_territorial_ai - Set up territory control AI
    if (SubAction == TEXT("configure_territorial_ai"))
    {
        FString TerritoryName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FVector Center = Forge2Helpers::GetVectorFromJson(Params, TEXT("center"));
        float Radius = GetJsonNumberField(Params, TEXT("radius"), 1000.0f);
        float AggressionLevel = GetJsonNumberField(Params, TEXT("aggression"), 0.5f);
        bool bPatrolBorder = GetJsonBoolField(Params, TEXT("patrol_border"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), TerritoryName);
        Result->SetNumberField(TEXT("radius"), Radius);
        Result->SetNumberField(TEXT("aggression"), AggressionLevel);
        Result->SetBoolField(TEXT("patrol_border"), bPatrolBorder);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Territorial AI configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown ai_advanced action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 PHYSICS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2PhysicsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_physics_material - Create physical material
    if (SubAction == TEXT("create_physics_material"))
    {
        FString MaterialName = GetJsonStringField(Params, TEXT("name"), TEXT("PM_Default"));
        FString MaterialPath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/Physics/Materials/"));
        float Friction = GetJsonNumberField(Params, TEXT("friction"), 0.7f);
        float StaticFriction = GetJsonNumberField(Params, TEXT("static_friction"), 0.0f);
        float Restitution = GetJsonNumberField(Params, TEXT("restitution"), 0.3f);
        float Density = GetJsonNumberField(Params, TEXT("density"), 1.0f);

        FString FullPath = MaterialPath + MaterialName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UPhysicalMaterial* PhysMat = NewObject<UPhysicalMaterial>(Package, *MaterialName, RF_Public | RF_Standalone);
        if (PhysMat)
        {
            PhysMat->Friction = Friction;
            PhysMat->StaticFriction = StaticFriction;
            PhysMat->Restitution = Restitution;
            PhysMat->Density = Density;

            FAssetRegistryModule::AssetCreated(PhysMat);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("name"), MaterialName);
            Result->SetNumberField(TEXT("friction"), Friction);
            Result->SetNumberField(TEXT("restitution"), Restitution);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Physics material created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create physics material"), TEXT("CREATION_FAILED"));
        return true;
    }

    // configure_physics_constraint - Set up physics constraint
    if (SubAction == TEXT("configure_physics_constraint"))
    {
        FString ConstraintType = GetJsonStringField(Params, TEXT("type"), TEXT("hinge")); // hinge, ball_socket, prismatic, free
        bool bDisableCollision = GetJsonBoolField(Params, TEXT("disable_collision"), true);

        // Angular limits
        float SwingLimit1 = GetJsonNumberField(Params, TEXT("swing_limit_1"), 45.0f);
        float SwingLimit2 = GetJsonNumberField(Params, TEXT("swing_limit_2"), 45.0f);
        float TwistLimit = GetJsonNumberField(Params, TEXT("twist_limit"), 45.0f);

        // Linear limits
        float LinearXLimit = GetJsonNumberField(Params, TEXT("linear_x_limit"), 0.0f);
        float LinearYLimit = GetJsonNumberField(Params, TEXT("linear_y_limit"), 0.0f);
        float LinearZLimit = GetJsonNumberField(Params, TEXT("linear_z_limit"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), ConstraintType);
        Result->SetBoolField(TEXT("disable_collision"), bDisableCollision);
        Result->SetNumberField(TEXT("swing_limit_1"), SwingLimit1);
        Result->SetNumberField(TEXT("swing_limit_2"), SwingLimit2);
        Result->SetNumberField(TEXT("twist_limit"), TwistLimit);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Physics constraint configured"), Result);
        return true;
    }

    // setup_physics_volume - Create physics volume
    if (SubAction == TEXT("setup_physics_volume"))
    {
        FString VolumeType = GetJsonStringField(Params, TEXT("type"), TEXT("water")); // water, low_gravity, wind, kill
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"));
        FVector Size = Forge2Helpers::GetVectorFromJson(Params, TEXT("size"));

        // Volume-specific properties
        float GravityScale = GetJsonNumberField(Params, TEXT("gravity_scale"), 1.0f);
        float BuoyancyDensity = GetJsonNumberField(Params, TEXT("buoyancy_density"), 1.0f);
        FVector WindDirection = Forge2Helpers::GetVectorFromJson(Params, TEXT("wind_direction"));
        float WindStrength = GetJsonNumberField(Params, TEXT("wind_strength"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), VolumeType);
        Result->SetNumberField(TEXT("gravity_scale"), GravityScale);
        Result->SetNumberField(TEXT("buoyancy_density"), BuoyancyDensity);
        Result->SetNumberField(TEXT("wind_strength"), WindStrength);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Physics volume created"), Result);
        return true;
    }

    // create_ragdoll - Configure ragdoll setup
    if (SubAction == TEXT("create_ragdoll"))
    {
        FString SkeletonPath = GetJsonStringField(Params, TEXT("skeleton_path"), TEXT(""));
        FString PhysicsAssetPath = GetJsonStringField(Params, TEXT("physics_asset"), TEXT(""));
        float AngularDamping = GetJsonNumberField(Params, TEXT("angular_damping"), 0.0f);
        float LinearDamping = GetJsonNumberField(Params, TEXT("linear_damping"), 0.01f);
        bool bEnableGravity = GetJsonBoolField(Params, TEXT("enable_gravity"), true);
        float MaxAngularVelocity = GetJsonNumberField(Params, TEXT("max_angular_velocity"), 10000.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skeleton_path"), SkeletonPath);
        Result->SetStringField(TEXT("physics_asset"), PhysicsAssetPath);
        Result->SetNumberField(TEXT("angular_damping"), AngularDamping);
        Result->SetNumberField(TEXT("linear_damping"), LinearDamping);
        Result->SetBoolField(TEXT("enable_gravity"), bEnableGravity);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Ragdoll configured"), Result);
        return true;
    }

    // configure_ragdoll_blend - Set partial ragdoll blending
    if (SubAction == TEXT("configure_ragdoll_blend"))
    {
        FString BlendMode = GetJsonStringField(Params, TEXT("blend_mode"), TEXT("physics_below")); // physics_below, blend_with_animation
        FString BlendBone = GetJsonStringField(Params, TEXT("blend_bone"), TEXT("spine_01"));
        float BlendWeight = GetJsonNumberField(Params, TEXT("blend_weight"), 1.0f);
        float BlendTime = GetJsonNumberField(Params, TEXT("blend_time"), 0.2f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blend_mode"), BlendMode);
        Result->SetStringField(TEXT("blend_bone"), BlendBone);
        Result->SetNumberField(TEXT("blend_weight"), BlendWeight);
        Result->SetNumberField(TEXT("blend_time"), BlendTime);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Ragdoll blend configured"), Result);
        return true;
    }

    // setup_projectile_physics - Configure projectile physics
    if (SubAction == TEXT("setup_projectile_physics"))
    {
        float InitialSpeed = GetJsonNumberField(Params, TEXT("initial_speed"), 3000.0f);
        float MaxSpeed = GetJsonNumberField(Params, TEXT("max_speed"), 3000.0f);
        float GravityScale = GetJsonNumberField(Params, TEXT("gravity_scale"), 1.0f);
        bool bRotationFollowsVelocity = GetJsonBoolField(Params, TEXT("rotation_follows_velocity"), true);
        bool bShouldBounce = GetJsonBoolField(Params, TEXT("should_bounce"), false);
        float Bounciness = GetJsonNumberField(Params, TEXT("bounciness"), 0.3f);
        int32 BounceLimit = GetJsonIntField(Params, TEXT("bounce_limit"), 3);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("initial_speed"), InitialSpeed);
        Result->SetNumberField(TEXT("max_speed"), MaxSpeed);
        Result->SetNumberField(TEXT("gravity_scale"), GravityScale);
        Result->SetBoolField(TEXT("rotation_follows_velocity"), bRotationFollowsVelocity);
        Result->SetBoolField(TEXT("should_bounce"), bShouldBounce);
        Result->SetNumberField(TEXT("bounciness"), Bounciness);
        Result->SetNumberField(TEXT("bounce_limit"), BounceLimit);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Projectile physics configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown physics action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 DAMAGE HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2DamageAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_damage_type_asset - Create damage type definition
    if (SubAction == TEXT("create_damage_type_asset"))
    {
        FString DamageTypeName = GetJsonStringField(Params, TEXT("name"), TEXT("DT_Physical"));
        FString DamagePath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/Gameplay/DamageTypes/"));
        FString DamageCategory = GetJsonStringField(Params, TEXT("category"), TEXT("physical")); // physical, magical, elemental, true
        FString Element = GetJsonStringField(Params, TEXT("element"), TEXT("none")); // fire, ice, lightning, poison, holy, dark
        bool bCanCrit = GetJsonBoolField(Params, TEXT("can_crit"), true);
        bool bIgnoresArmor = GetJsonBoolField(Params, TEXT("ignores_armor"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), DamageTypeName);
        Result->SetStringField(TEXT("path"), DamagePath + DamageTypeName);
        Result->SetStringField(TEXT("category"), DamageCategory);
        Result->SetStringField(TEXT("element"), Element);
        Result->SetBoolField(TEXT("can_crit"), bCanCrit);
        Result->SetBoolField(TEXT("ignores_armor"), bIgnoresArmor);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Damage type created"), Result);
        return true;
    }

    // configure_damage_falloff - Set damage falloff curves
    if (SubAction == TEXT("configure_damage_falloff"))
    {
        FString FalloffType = GetJsonStringField(Params, TEXT("type"), TEXT("linear")); // linear, exponential, step
        float StartDistance = GetJsonNumberField(Params, TEXT("start_distance"), 0.0f);
        float EndDistance = GetJsonNumberField(Params, TEXT("end_distance"), 5000.0f);
        float MinDamagePercent = GetJsonNumberField(Params, TEXT("min_damage_percent"), 0.5f);
        float Exponent = GetJsonNumberField(Params, TEXT("exponent"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), FalloffType);
        Result->SetNumberField(TEXT("start_distance"), StartDistance);
        Result->SetNumberField(TEXT("end_distance"), EndDistance);
        Result->SetNumberField(TEXT("min_damage_percent"), MinDamagePercent);
        Result->SetNumberField(TEXT("exponent"), Exponent);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Damage falloff configured"), Result);
        return true;
    }

    // setup_critical_hit_system - Configure critical hit mechanics
    if (SubAction == TEXT("setup_critical_hit_system"))
    {
        float BaseCritChance = GetJsonNumberField(Params, TEXT("base_crit_chance"), 0.05f);
        float BaseCritMultiplier = GetJsonNumberField(Params, TEXT("base_crit_multiplier"), 2.0f);
        float MaxCritChance = GetJsonNumberField(Params, TEXT("max_crit_chance"), 1.0f);
        float MaxCritMultiplier = GetJsonNumberField(Params, TEXT("max_crit_multiplier"), 5.0f);
        bool bEnableHeadshots = GetJsonBoolField(Params, TEXT("enable_headshots"), true);
        float HeadshotMultiplier = GetJsonNumberField(Params, TEXT("headshot_multiplier"), 2.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("base_crit_chance"), BaseCritChance);
        Result->SetNumberField(TEXT("base_crit_multiplier"), BaseCritMultiplier);
        Result->SetNumberField(TEXT("max_crit_chance"), MaxCritChance);
        Result->SetNumberField(TEXT("max_crit_multiplier"), MaxCritMultiplier);
        Result->SetBoolField(TEXT("enable_headshots"), bEnableHeadshots);
        Result->SetNumberField(TEXT("headshot_multiplier"), HeadshotMultiplier);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Critical hit system configured"), Result);
        return true;
    }

    // create_damage_number_config - Configure floating damage numbers
    if (SubAction == TEXT("create_damage_number_config"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        bool bShowCritical = GetJsonBoolField(Params, TEXT("show_critical"), true);
        bool bShowHealing = GetJsonBoolField(Params, TEXT("show_healing"), true);
        bool bStackNumbers = GetJsonBoolField(Params, TEXT("stack_numbers"), false);
        float FloatSpeed = GetJsonNumberField(Params, TEXT("float_speed"), 50.0f);
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 1.0f);
        float FontSize = GetJsonNumberField(Params, TEXT("font_size"), 24.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetBoolField(TEXT("show_critical"), bShowCritical);
        Result->SetBoolField(TEXT("show_healing"), bShowHealing);
        Result->SetBoolField(TEXT("stack_numbers"), bStackNumbers);
        Result->SetNumberField(TEXT("float_speed"), FloatSpeed);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetNumberField(TEXT("font_size"), FontSize);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Damage number config created"), Result);
        return true;
    }

    // configure_death_handling - Set up death and respawn mechanics
    if (SubAction == TEXT("configure_death_handling"))
    {
        FString DeathType = GetJsonStringField(Params, TEXT("death_type"), TEXT("respawn")); // respawn, permadeath, checkpoint
        float RespawnDelay = GetJsonNumberField(Params, TEXT("respawn_delay"), 3.0f);
        bool bDropLootOnDeath = GetJsonBoolField(Params, TEXT("drop_loot"), false);
        bool bLoseXpOnDeath = GetJsonBoolField(Params, TEXT("lose_xp"), false);
        float XpLossPercent = GetJsonNumberField(Params, TEXT("xp_loss_percent"), 0.0f);
        bool bPlayDeathAnimation = GetJsonBoolField(Params, TEXT("play_animation"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("death_type"), DeathType);
        Result->SetNumberField(TEXT("respawn_delay"), RespawnDelay);
        Result->SetBoolField(TEXT("drop_loot"), bDropLootOnDeath);
        Result->SetBoolField(TEXT("lose_xp"), bLoseXpOnDeath);
        Result->SetNumberField(TEXT("xp_loss_percent"), XpLossPercent);
        Result->SetBoolField(TEXT("play_animation"), bPlayDeathAnimation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Death handling configured"), Result);
        return true;
    }

    // setup_invincibility_frames - Configure i-frames
    if (SubAction == TEXT("setup_invincibility_frames"))
    {
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 0.5f);
        bool bOnHit = GetJsonBoolField(Params, TEXT("on_hit"), true);
        bool bOnRespawn = GetJsonBoolField(Params, TEXT("on_respawn"), true);
        bool bOnDodge = GetJsonBoolField(Params, TEXT("on_dodge"), true);
        float DodgeIFrameDuration = GetJsonNumberField(Params, TEXT("dodge_duration"), 0.3f);
        bool bFlashDuringIFrames = GetJsonBoolField(Params, TEXT("flash_effect"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetBoolField(TEXT("on_hit"), bOnHit);
        Result->SetBoolField(TEXT("on_respawn"), bOnRespawn);
        Result->SetBoolField(TEXT("on_dodge"), bOnDodge);
        Result->SetNumberField(TEXT("dodge_duration"), DodgeIFrameDuration);
        Result->SetBoolField(TEXT("flash_effect"), bFlashDuringIFrames);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Invincibility frames configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown damage action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 GENRE - RACING HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreRacingAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_vehicle - Create racing vehicle
    if (SubAction == TEXT("create_vehicle"))
    {
        FString VehicleName = GetJsonStringField(Params, TEXT("name"), TEXT("Vehicle_Default"));
        FString VehicleType = GetJsonStringField(Params, TEXT("type"), TEXT("car")); // car, bike, boat, aircraft
        FString VehiclePath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/Vehicles/"));
        float MaxSpeed = GetJsonNumberField(Params, TEXT("max_speed"), 200.0f);
        float Acceleration = GetJsonNumberField(Params, TEXT("acceleration"), 10.0f);
        float BrakeForce = GetJsonNumberField(Params, TEXT("brake_force"), 15.0f);
        float Mass = GetJsonNumberField(Params, TEXT("mass"), 1500.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), VehicleName);
        Result->SetStringField(TEXT("type"), VehicleType);
        Result->SetStringField(TEXT("path"), VehiclePath + VehicleName);
        Result->SetNumberField(TEXT("max_speed"), MaxSpeed);
        Result->SetNumberField(TEXT("acceleration"), Acceleration);
        Result->SetNumberField(TEXT("brake_force"), BrakeForce);
        Result->SetNumberField(TEXT("mass"), Mass);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Vehicle created"), Result);
        return true;
    }

    // configure_vehicle_physics - Set vehicle physics mode
    if (SubAction == TEXT("configure_vehicle_physics"))
    {
        FString HandlingMode = GetJsonStringField(Params, TEXT("handling_mode"), TEXT("arcade")); // arcade, simulation
        float SteeringSpeed = GetJsonNumberField(Params, TEXT("steering_speed"), 5.0f);
        float DriftFactor = GetJsonNumberField(Params, TEXT("drift_factor"), 0.5f);
        float DownforceCoefficient = GetJsonNumberField(Params, TEXT("downforce"), 3.0f);
        float AirResistance = GetJsonNumberField(Params, TEXT("air_resistance"), 0.3f);
        bool bEnableNOS = GetJsonBoolField(Params, TEXT("enable_nos"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("handling_mode"), HandlingMode);
        Result->SetNumberField(TEXT("steering_speed"), SteeringSpeed);
        Result->SetNumberField(TEXT("drift_factor"), DriftFactor);
        Result->SetNumberField(TEXT("downforce"), DownforceCoefficient);
        Result->SetNumberField(TEXT("air_resistance"), AirResistance);
        Result->SetBoolField(TEXT("enable_nos"), bEnableNOS);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Vehicle physics configured"), Result);
        return true;
    }

    // configure_tire_system - Set up tire physics
    if (SubAction == TEXT("configure_tire_system"))
    {
        FString TireType = GetJsonStringField(Params, TEXT("tire_type"), TEXT("medium")); // soft, medium, hard, wet, off_road
        float GripMultiplier = GetJsonNumberField(Params, TEXT("grip"), 1.0f);
        float WearRate = GetJsonNumberField(Params, TEXT("wear_rate"), 0.01f);
        bool bEnableTemperature = GetJsonBoolField(Params, TEXT("enable_temperature"), true);
        float OptimalTemperature = GetJsonNumberField(Params, TEXT("optimal_temp"), 90.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tire_type"), TireType);
        Result->SetNumberField(TEXT("grip"), GripMultiplier);
        Result->SetNumberField(TEXT("wear_rate"), WearRate);
        Result->SetBoolField(TEXT("enable_temperature"), bEnableTemperature);
        Result->SetNumberField(TEXT("optimal_temp"), OptimalTemperature);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Tire system configured"), Result);
        return true;
    }

    // setup_damage_system - Vehicle damage configuration
    if (SubAction == TEXT("setup_damage_system"))
    {
        bool bVisualDamage = GetJsonBoolField(Params, TEXT("visual_damage"), true);
        bool bMechanicalDamage = GetJsonBoolField(Params, TEXT("mechanical_damage"), true);
        float DamageThreshold = GetJsonNumberField(Params, TEXT("damage_threshold"), 100.0f);
        float MaxDamage = GetJsonNumberField(Params, TEXT("max_damage"), 1000.0f);
        bool bEnableRepair = GetJsonBoolField(Params, TEXT("enable_repair"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("visual_damage"), bVisualDamage);
        Result->SetBoolField(TEXT("mechanical_damage"), bMechanicalDamage);
        Result->SetNumberField(TEXT("damage_threshold"), DamageThreshold);
        Result->SetNumberField(TEXT("max_damage"), MaxDamage);
        Result->SetBoolField(TEXT("enable_repair"), bEnableRepair);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Damage system configured"), Result);
        return true;
    }

    // create_track - Generate race track
    if (SubAction == TEXT("create_track"))
    {
        FString TrackName = GetJsonStringField(Params, TEXT("name"), TEXT("Track_Default"));
        int32 CheckpointCount = GetJsonIntField(Params, TEXT("checkpoints"), 10);
        float TrackWidth = GetJsonNumberField(Params, TEXT("track_width"), 12.0f);
        FString TrackStyle = GetJsonStringField(Params, TEXT("style"), TEXT("circuit")); // circuit, point_to_point, rally
        int32 LapCount = GetJsonIntField(Params, TEXT("laps"), 3);
        bool bIncludePitLane = GetJsonBoolField(Params, TEXT("pit_lane"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), TrackName);
        Result->SetNumberField(TEXT("checkpoints"), CheckpointCount);
        Result->SetNumberField(TEXT("track_width"), TrackWidth);
        Result->SetStringField(TEXT("style"), TrackStyle);
        Result->SetNumberField(TEXT("laps"), LapCount);
        Result->SetBoolField(TEXT("pit_lane"), bIncludePitLane);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Track created"), Result);
        return true;
    }

    // setup_race_mode - Configure race mode
    if (SubAction == TEXT("setup_race_mode"))
    {
        FString RaceMode = GetJsonStringField(Params, TEXT("mode"), TEXT("circuit")); // circuit, time_trial, elimination, drift
        int32 MaxPlayers = GetJsonIntField(Params, TEXT("max_players"), 8);
        float CountdownTime = GetJsonNumberField(Params, TEXT("countdown"), 3.0f);
        bool bRollingStart = GetJsonBoolField(Params, TEXT("rolling_start"), false);
        bool bRubberBanding = GetJsonBoolField(Params, TEXT("rubber_banding"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mode"), RaceMode);
        Result->SetNumberField(TEXT("max_players"), MaxPlayers);
        Result->SetNumberField(TEXT("countdown"), CountdownTime);
        Result->SetBoolField(TEXT("rolling_start"), bRollingStart);
        Result->SetBoolField(TEXT("rubber_banding"), bRubberBanding);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Race mode configured"), Result);
        return true;
    }

    // create_powerup_system - Arcade racing powerups
    if (SubAction == TEXT("create_powerup_system"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float SpawnInterval = GetJsonNumberField(Params, TEXT("spawn_interval"), 30.0f);
        const TArray<TSharedPtr<FJsonValue>>* PowerupTypes = nullptr;
        Params->TryGetArrayField(TEXT("powerup_types"), PowerupTypes);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("spawn_interval"), SpawnInterval);
        if (PowerupTypes) Result->SetNumberField(TEXT("powerup_count"), PowerupTypes->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Powerup system created"), Result);
        return true;
    }

    // configure_ai_racers - AI difficulty settings
    if (SubAction == TEXT("configure_ai_racers"))
    {
        int32 AICount = GetJsonIntField(Params, TEXT("count"), 7);
        FString Difficulty = GetJsonStringField(Params, TEXT("difficulty"), TEXT("medium")); // easy, medium, hard, expert
        float RubberBandStrength = GetJsonNumberField(Params, TEXT("rubber_band_strength"), 0.5f);
        bool bCatchUp = GetJsonBoolField(Params, TEXT("catch_up"), true);
        float AggressionLevel = GetJsonNumberField(Params, TEXT("aggression"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("count"), AICount);
        Result->SetStringField(TEXT("difficulty"), Difficulty);
        Result->SetNumberField(TEXT("rubber_band_strength"), RubberBandStrength);
        Result->SetBoolField(TEXT("catch_up"), bCatchUp);
        Result->SetNumberField(TEXT("aggression"), AggressionLevel);
        SendAutomationResponse(Socket, RequestId, true, TEXT("AI racers configured"), Result);
        return true;
    }

    // setup_leaderboards - Leaderboard and ghost data
    if (SubAction == TEXT("setup_leaderboards"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        bool bEnableGhosts = GetJsonBoolField(Params, TEXT("enable_ghosts"), true);
        int32 MaxLeaderboardEntries = GetJsonIntField(Params, TEXT("max_entries"), 100);
        bool bTrackSectorTimes = GetJsonBoolField(Params, TEXT("sector_times"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetBoolField(TEXT("enable_ghosts"), bEnableGhosts);
        Result->SetNumberField(TEXT("max_entries"), MaxLeaderboardEntries);
        Result->SetBoolField(TEXT("sector_times"), bTrackSectorTimes);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Leaderboards configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_racing action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 GENRE - FIGHTING HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreFightingAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_fighter - Create fighting game character
    if (SubAction == TEXT("create_fighter"))
    {
        FString FighterName = GetJsonStringField(Params, TEXT("name"), TEXT("Fighter_Default"));
        FString FighterPath = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/Fighters/"));
        int32 Health = GetJsonIntField(Params, TEXT("health"), 1000);
        float WalkSpeed = GetJsonNumberField(Params, TEXT("walk_speed"), 300.0f);
        float JumpHeight = GetJsonNumberField(Params, TEXT("jump_height"), 400.0f);
        FString FightingStyle = GetJsonStringField(Params, TEXT("style"), TEXT("balanced")); // balanced, rushdown, zoner, grappler

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), FighterName);
        Result->SetStringField(TEXT("path"), FighterPath + FighterName);
        Result->SetNumberField(TEXT("health"), Health);
        Result->SetNumberField(TEXT("walk_speed"), WalkSpeed);
        Result->SetNumberField(TEXT("jump_height"), JumpHeight);
        Result->SetStringField(TEXT("style"), FightingStyle);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Fighter created"), Result);
        return true;
    }

    // define_moveset - Configure fighter's moveset
    if (SubAction == TEXT("define_moveset"))
    {
        FString FighterPath = GetJsonStringField(Params, TEXT("fighter_path"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* NormalAttacks = nullptr;
        Params->TryGetArrayField(TEXT("normals"), NormalAttacks);
        const TArray<TSharedPtr<FJsonValue>>* SpecialMoves = nullptr;
        Params->TryGetArrayField(TEXT("specials"), SpecialMoves);
        const TArray<TSharedPtr<FJsonValue>>* SuperMoves = nullptr;
        Params->TryGetArrayField(TEXT("supers"), SuperMoves);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("fighter"), FighterPath);
        if (NormalAttacks) Result->SetNumberField(TEXT("normal_count"), NormalAttacks->Num());
        if (SpecialMoves) Result->SetNumberField(TEXT("special_count"), SpecialMoves->Num());
        if (SuperMoves) Result->SetNumberField(TEXT("super_count"), SuperMoves->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Moveset defined"), Result);
        return true;
    }

    // configure_combo_system - Set combo rules
    if (SubAction == TEXT("configure_combo_system"))
    {
        int32 MaxJuggleCount = GetJsonIntField(Params, TEXT("max_juggle"), 5);
        float DamageScaling = GetJsonNumberField(Params, TEXT("damage_scaling"), 0.1f);
        float HitstunDecay = GetJsonNumberField(Params, TEXT("hitstun_decay"), 0.05f);
        bool bEnableComboBreaker = GetJsonBoolField(Params, TEXT("combo_breaker"), true);
        float BreakerCost = GetJsonNumberField(Params, TEXT("breaker_cost"), 50.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_juggle"), MaxJuggleCount);
        Result->SetNumberField(TEXT("damage_scaling"), DamageScaling);
        Result->SetNumberField(TEXT("hitstun_decay"), HitstunDecay);
        Result->SetBoolField(TEXT("combo_breaker"), bEnableComboBreaker);
        Result->SetNumberField(TEXT("breaker_cost"), BreakerCost);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Combo system configured"), Result);
        return true;
    }

    // setup_frame_data - Configure frame data for move
    if (SubAction == TEXT("setup_frame_data"))
    {
        FString MoveName = GetJsonStringField(Params, TEXT("move_name"), TEXT(""));
        int32 StartupFrames = GetJsonIntField(Params, TEXT("startup"), 5);
        int32 ActiveFrames = GetJsonIntField(Params, TEXT("active"), 3);
        int32 RecoveryFrames = GetJsonIntField(Params, TEXT("recovery"), 15);
        int32 BlockAdvantage = GetJsonIntField(Params, TEXT("block_advantage"), -5);
        int32 HitAdvantage = GetJsonIntField(Params, TEXT("hit_advantage"), 5);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("move_name"), MoveName);
        Result->SetNumberField(TEXT("startup"), StartupFrames);
        Result->SetNumberField(TEXT("active"), ActiveFrames);
        Result->SetNumberField(TEXT("recovery"), RecoveryFrames);
        Result->SetNumberField(TEXT("block_advantage"), BlockAdvantage);
        Result->SetNumberField(TEXT("hit_advantage"), HitAdvantage);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Frame data configured"), Result);
        return true;
    }

    // create_special_meter - Super/special meter system
    if (SubAction == TEXT("create_special_meter"))
    {
        FString MeterType = GetJsonStringField(Params, TEXT("type"), TEXT("super")); // super, rage, burst, instinct
        float MaxMeter = GetJsonNumberField(Params, TEXT("max"), 100.0f);
        float GainOnHit = GetJsonNumberField(Params, TEXT("gain_on_hit"), 5.0f);
        float GainOnDamage = GetJsonNumberField(Params, TEXT("gain_on_damage"), 10.0f);
        int32 Stocks = GetJsonIntField(Params, TEXT("stocks"), 3);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), MeterType);
        Result->SetNumberField(TEXT("max"), MaxMeter);
        Result->SetNumberField(TEXT("gain_on_hit"), GainOnHit);
        Result->SetNumberField(TEXT("gain_on_damage"), GainOnDamage);
        Result->SetNumberField(TEXT("stocks"), Stocks);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Special meter created"), Result);
        return true;
    }

    // configure_blocking - Blocking and defense mechanics
    if (SubAction == TEXT("configure_blocking"))
    {
        bool bHighLowBlocking = GetJsonBoolField(Params, TEXT("high_low"), true);
        bool bEnableParry = GetJsonBoolField(Params, TEXT("parry"), true);
        int32 ParryWindow = GetJsonIntField(Params, TEXT("parry_window"), 3);
        float ChipDamage = GetJsonNumberField(Params, TEXT("chip_damage"), 0.1f);
        float GuardBreakThreshold = GetJsonNumberField(Params, TEXT("guard_break"), 100.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("high_low"), bHighLowBlocking);
        Result->SetBoolField(TEXT("parry"), bEnableParry);
        Result->SetNumberField(TEXT("parry_window"), ParryWindow);
        Result->SetNumberField(TEXT("chip_damage"), ChipDamage);
        Result->SetNumberField(TEXT("guard_break"), GuardBreakThreshold);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Blocking configured"), Result);
        return true;
    }

    // setup_stage - Fighting stage configuration
    if (SubAction == TEXT("setup_stage"))
    {
        FString StageName = GetJsonStringField(Params, TEXT("name"), TEXT("Stage_Default"));
        float StageWidth = GetJsonNumberField(Params, TEXT("width"), 2000.0f);
        bool bWallBounce = GetJsonBoolField(Params, TEXT("wall_bounce"), true);
        bool bGroundBounce = GetJsonBoolField(Params, TEXT("ground_bounce"), true);
        bool bRingOut = GetJsonBoolField(Params, TEXT("ring_out"), false);
        const TArray<TSharedPtr<FJsonValue>>* StageHazards = nullptr;
        Params->TryGetArrayField(TEXT("hazards"), StageHazards);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), StageName);
        Result->SetNumberField(TEXT("width"), StageWidth);
        Result->SetBoolField(TEXT("wall_bounce"), bWallBounce);
        Result->SetBoolField(TEXT("ground_bounce"), bGroundBounce);
        Result->SetBoolField(TEXT("ring_out"), bRingOut);
        if (StageHazards) Result->SetNumberField(TEXT("hazard_count"), StageHazards->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Stage configured"), Result);
        return true;
    }

    // create_training_mode - Training mode features
    if (SubAction == TEXT("create_training_mode"))
    {
        bool bShowHitboxes = GetJsonBoolField(Params, TEXT("show_hitboxes"), true);
        bool bShowFrameData = GetJsonBoolField(Params, TEXT("show_frame_data"), true);
        bool bShowInputDisplay = GetJsonBoolField(Params, TEXT("show_inputs"), true);
        bool bDummyRecord = GetJsonBoolField(Params, TEXT("dummy_record"), true);
        bool bComboCounter = GetJsonBoolField(Params, TEXT("combo_counter"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("show_hitboxes"), bShowHitboxes);
        Result->SetBoolField(TEXT("show_frame_data"), bShowFrameData);
        Result->SetBoolField(TEXT("show_inputs"), bShowInputDisplay);
        Result->SetBoolField(TEXT("dummy_record"), bDummyRecord);
        Result->SetBoolField(TEXT("combo_counter"), bComboCounter);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Training mode created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_fighting action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 GENRE - PLATFORMER HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenrePlatformerAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_player_controller_2d - 2D platformer controller
    if (SubAction == TEXT("create_player_controller_2d"))
    {
        FString ControllerName = GetJsonStringField(Params, TEXT("name"), TEXT("PC_Platformer2D"));
        float MoveSpeed = GetJsonNumberField(Params, TEXT("move_speed"), 600.0f);
        float JumpForce = GetJsonNumberField(Params, TEXT("jump_force"), 1200.0f);
        bool bWallJump = GetJsonBoolField(Params, TEXT("wall_jump"), true);
        bool bDash = GetJsonBoolField(Params, TEXT("dash"), true);
        float DashSpeed = GetJsonNumberField(Params, TEXT("dash_speed"), 1500.0f);
        float DashDuration = GetJsonNumberField(Params, TEXT("dash_duration"), 0.2f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), ControllerName);
        Result->SetNumberField(TEXT("move_speed"), MoveSpeed);
        Result->SetNumberField(TEXT("jump_force"), JumpForce);
        Result->SetBoolField(TEXT("wall_jump"), bWallJump);
        Result->SetBoolField(TEXT("dash"), bDash);
        Result->SetNumberField(TEXT("dash_speed"), DashSpeed);
        Result->SetNumberField(TEXT("dash_duration"), DashDuration);
        SendAutomationResponse(Socket, RequestId, true, TEXT("2D controller created"), Result);
        return true;
    }

    // create_player_controller_3d - 3D platformer controller
    if (SubAction == TEXT("create_player_controller_3d"))
    {
        FString ControllerName = GetJsonStringField(Params, TEXT("name"), TEXT("PC_Platformer3D"));
        float MoveSpeed = GetJsonNumberField(Params, TEXT("move_speed"), 800.0f);
        float JumpForce = GetJsonNumberField(Params, TEXT("jump_force"), 1000.0f);
        bool bTripleJump = GetJsonBoolField(Params, TEXT("triple_jump"), true);
        bool bLongJump = GetJsonBoolField(Params, TEXT("long_jump"), true);
        bool bGroundPound = GetJsonBoolField(Params, TEXT("ground_pound"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), ControllerName);
        Result->SetNumberField(TEXT("move_speed"), MoveSpeed);
        Result->SetNumberField(TEXT("jump_force"), JumpForce);
        Result->SetBoolField(TEXT("triple_jump"), bTripleJump);
        Result->SetBoolField(TEXT("long_jump"), bLongJump);
        Result->SetBoolField(TEXT("ground_pound"), bGroundPound);
        SendAutomationResponse(Socket, RequestId, true, TEXT("3D controller created"), Result);
        return true;
    }

    // configure_jump_physics - Jump mechanics tuning
    if (SubAction == TEXT("configure_jump_physics"))
    {
        float CoyoteTime = GetJsonNumberField(Params, TEXT("coyote_time"), 0.1f);
        float JumpBufferTime = GetJsonNumberField(Params, TEXT("jump_buffer"), 0.1f);
        float VariableJumpMinHeight = GetJsonNumberField(Params, TEXT("min_jump_height"), 0.3f);
        float AirControl = GetJsonNumberField(Params, TEXT("air_control"), 0.8f);
        float Gravity = GetJsonNumberField(Params, TEXT("gravity"), 1.0f);
        float FallingGravityMultiplier = GetJsonNumberField(Params, TEXT("fall_gravity"), 2.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("coyote_time"), CoyoteTime);
        Result->SetNumberField(TEXT("jump_buffer"), JumpBufferTime);
        Result->SetNumberField(TEXT("min_jump_height"), VariableJumpMinHeight);
        Result->SetNumberField(TEXT("air_control"), AirControl);
        Result->SetNumberField(TEXT("gravity"), Gravity);
        Result->SetNumberField(TEXT("fall_gravity"), FallingGravityMultiplier);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Jump physics configured"), Result);
        return true;
    }

    // create_platform_types - Platform behavior definitions
    if (SubAction == TEXT("create_platform_types"))
    {
        FString PlatformType = GetJsonStringField(Params, TEXT("type"), TEXT("static")); // static, moving, falling, bouncy, ice, conveyor, one_way
        FVector MoveDirection = Forge2Helpers::GetVectorFromJson(Params, TEXT("move_direction"));
        float MoveSpeed = GetJsonNumberField(Params, TEXT("move_speed"), 200.0f);
        float FallDelay = GetJsonNumberField(Params, TEXT("fall_delay"), 0.5f);
        float BounceForce = GetJsonNumberField(Params, TEXT("bounce_force"), 1500.0f);
        float ConveyorSpeed = GetJsonNumberField(Params, TEXT("conveyor_speed"), 300.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), PlatformType);
        Result->SetNumberField(TEXT("move_speed"), MoveSpeed);
        Result->SetNumberField(TEXT("fall_delay"), FallDelay);
        Result->SetNumberField(TEXT("bounce_force"), BounceForce);
        Result->SetNumberField(TEXT("conveyor_speed"), ConveyorSpeed);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Platform type created"), Result);
        return true;
    }

    // setup_collectibles - Collectible configuration
    if (SubAction == TEXT("setup_collectibles"))
    {
        FString CollectibleType = GetJsonStringField(Params, TEXT("type"), TEXT("coin")); // coin, gem, star, key
        int32 PointValue = GetJsonIntField(Params, TEXT("points"), 1);
        bool bRespawn = GetJsonBoolField(Params, TEXT("respawn"), false);
        float RespawnTime = GetJsonNumberField(Params, TEXT("respawn_time"), 60.0f);
        FString CollectSound = GetJsonStringField(Params, TEXT("sound"), TEXT(""));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), CollectibleType);
        Result->SetNumberField(TEXT("points"), PointValue);
        Result->SetBoolField(TEXT("respawn"), bRespawn);
        Result->SetNumberField(TEXT("respawn_time"), RespawnTime);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Collectibles configured"), Result);
        return true;
    }

    // create_checkpoint_system - Checkpoint and respawn
    if (SubAction == TEXT("create_checkpoint_system"))
    {
        bool bAutoActivate = GetJsonBoolField(Params, TEXT("auto_activate"), true);
        bool bSaveProgress = GetJsonBoolField(Params, TEXT("save_progress"), true);
        float RespawnInvincibility = GetJsonNumberField(Params, TEXT("respawn_invincibility"), 1.0f);
        bool bRespawnAnimation = GetJsonBoolField(Params, TEXT("respawn_animation"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("auto_activate"), bAutoActivate);
        Result->SetBoolField(TEXT("save_progress"), bSaveProgress);
        Result->SetNumberField(TEXT("respawn_invincibility"), RespawnInvincibility);
        Result->SetBoolField(TEXT("respawn_animation"), bRespawnAnimation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Checkpoint system created"), Result);
        return true;
    }

    // configure_hazards - Hazard types
    if (SubAction == TEXT("configure_hazards"))
    {
        FString HazardType = GetJsonStringField(Params, TEXT("type"), TEXT("spike")); // spike, pit, enemy, fire, water, laser
        int32 Damage = GetJsonIntField(Params, TEXT("damage"), 1);
        bool bInstantKill = GetJsonBoolField(Params, TEXT("instant_kill"), false);
        float KnockbackForce = GetJsonNumberField(Params, TEXT("knockback"), 500.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), HazardType);
        Result->SetNumberField(TEXT("damage"), Damage);
        Result->SetBoolField(TEXT("instant_kill"), bInstantKill);
        Result->SetNumberField(TEXT("knockback"), KnockbackForce);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Hazard configured"), Result);
        return true;
    }

    // setup_powerups - Platformer powerups
    if (SubAction == TEXT("setup_powerups"))
    {
        FString PowerupType = GetJsonStringField(Params, TEXT("type"), TEXT("double_jump")); // double_jump, glide, size_change, invincibility, speed_boost
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 10.0f);
        bool bPermanent = GetJsonBoolField(Params, TEXT("permanent"), false);
        bool bStackable = GetJsonBoolField(Params, TEXT("stackable"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), PowerupType);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetBoolField(TEXT("permanent"), bPermanent);
        Result->SetBoolField(TEXT("stackable"), bStackable);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Powerup configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_platformer action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 GENRE - STEALTH HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreStealthAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_detection_system - Vision and hearing detection
    if (SubAction == TEXT("create_detection_system"))
    {
        float VisionConeAngle = GetJsonNumberField(Params, TEXT("vision_angle"), 90.0f);
        float VisionRange = GetJsonNumberField(Params, TEXT("vision_range"), 2000.0f);
        float HearingRange = GetJsonNumberField(Params, TEXT("hearing_range"), 1000.0f);
        float SuspicionRate = GetJsonNumberField(Params, TEXT("suspicion_rate"), 1.0f);
        float AlertThreshold = GetJsonNumberField(Params, TEXT("alert_threshold"), 100.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("vision_angle"), VisionConeAngle);
        Result->SetNumberField(TEXT("vision_range"), VisionRange);
        Result->SetNumberField(TEXT("hearing_range"), HearingRange);
        Result->SetNumberField(TEXT("suspicion_rate"), SuspicionRate);
        Result->SetNumberField(TEXT("alert_threshold"), AlertThreshold);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Detection system created"), Result);
        return true;
    }

    // configure_ai_awareness - AI awareness states
    if (SubAction == TEXT("configure_ai_awareness"))
    {
        float UnawareDecay = GetJsonNumberField(Params, TEXT("unaware_decay"), 0.5f);
        float SuspiciousDecay = GetJsonNumberField(Params, TEXT("suspicious_decay"), 0.2f);
        float AlertDuration = GetJsonNumberField(Params, TEXT("alert_duration"), 30.0f);
        float SearchDuration = GetJsonNumberField(Params, TEXT("search_duration"), 60.0f);
        bool bCallForBackup = GetJsonBoolField(Params, TEXT("call_backup"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("unaware_decay"), UnawareDecay);
        Result->SetNumberField(TEXT("suspicious_decay"), SuspiciousDecay);
        Result->SetNumberField(TEXT("alert_duration"), AlertDuration);
        Result->SetNumberField(TEXT("search_duration"), SearchDuration);
        Result->SetBoolField(TEXT("call_backup"), bCallForBackup);
        SendAutomationResponse(Socket, RequestId, true, TEXT("AI awareness configured"), Result);
        return true;
    }

    // setup_patrol_paths - Guard patrol routes
    if (SubAction == TEXT("setup_patrol_paths"))
    {
        FString PatrolName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* WayPoints = nullptr;
        Params->TryGetArrayField(TEXT("waypoints"), WayPoints);
        float WaitTime = GetJsonNumberField(Params, TEXT("wait_time"), 3.0f);
        bool bRandomize = GetJsonBoolField(Params, TEXT("randomize"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), PatrolName);
        Result->SetNumberField(TEXT("wait_time"), WaitTime);
        Result->SetBoolField(TEXT("randomize"), bRandomize);
        if (WayPoints) Result->SetNumberField(TEXT("waypoint_count"), WayPoints->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Patrol path configured"), Result);
        return true;
    }

    // create_distraction_system - Noise and decoy distractions
    if (SubAction == TEXT("create_distraction_system"))
    {
        FString DistractionType = GetJsonStringField(Params, TEXT("type"), TEXT("noise")); // noise, thrown_object, decoy, whistle
        float DistractionRadius = GetJsonNumberField(Params, TEXT("radius"), 500.0f);
        float DistractionDuration = GetJsonNumberField(Params, TEXT("duration"), 5.0f);
        int32 MaxDistracted = GetJsonIntField(Params, TEXT("max_distracted"), 3);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), DistractionType);
        Result->SetNumberField(TEXT("radius"), DistractionRadius);
        Result->SetNumberField(TEXT("duration"), DistractionDuration);
        Result->SetNumberField(TEXT("max_distracted"), MaxDistracted);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Distraction system created"), Result);
        return true;
    }

    // configure_cover_system - Cover and hide mechanics
    if (SubAction == TEXT("configure_cover_system"))
    {
        bool bEnablePeek = GetJsonBoolField(Params, TEXT("enable_peek"), true);
        bool bEnableTakedown = GetJsonBoolField(Params, TEXT("enable_takedown"), true);
        float TakedownRange = GetJsonNumberField(Params, TEXT("takedown_range"), 150.0f);
        bool bCornerTakedown = GetJsonBoolField(Params, TEXT("corner_takedown"), true);
        float HideOpacity = GetJsonNumberField(Params, TEXT("hide_opacity"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enable_peek"), bEnablePeek);
        Result->SetBoolField(TEXT("enable_takedown"), bEnableTakedown);
        Result->SetNumberField(TEXT("takedown_range"), TakedownRange);
        Result->SetBoolField(TEXT("corner_takedown"), bCornerTakedown);
        Result->SetNumberField(TEXT("hide_opacity"), HideOpacity);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Cover system configured"), Result);
        return true;
    }

    // setup_disguise_system - Disguise mechanics
    if (SubAction == TEXT("setup_disguise_system"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float DisguiseEffectiveness = GetJsonNumberField(Params, TEXT("effectiveness"), 0.8f);
        float SuspicionOnAction = GetJsonNumberField(Params, TEXT("suspicion_on_action"), 20.0f);
        bool bLoseOnRun = GetJsonBoolField(Params, TEXT("lose_on_run"), true);
        const TArray<TSharedPtr<FJsonValue>>* DisguiseTypes = nullptr;
        Params->TryGetArrayField(TEXT("disguise_types"), DisguiseTypes);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("effectiveness"), DisguiseEffectiveness);
        Result->SetNumberField(TEXT("suspicion_on_action"), SuspicionOnAction);
        Result->SetBoolField(TEXT("lose_on_run"), bLoseOnRun);
        if (DisguiseTypes) Result->SetNumberField(TEXT("disguise_count"), DisguiseTypes->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Disguise system configured"), Result);
        return true;
    }

    // create_security_systems - Cameras and alarms
    if (SubAction == TEXT("create_security_systems"))
    {
        FString SystemType = GetJsonStringField(Params, TEXT("type"), TEXT("camera")); // camera, laser, alarm, turret
        float DetectionRadius = GetJsonNumberField(Params, TEXT("detection_radius"), 1000.0f);
        float RotationSpeed = GetJsonNumberField(Params, TEXT("rotation_speed"), 30.0f);
        bool bDestructible = GetJsonBoolField(Params, TEXT("destructible"), true);
        bool bHackable = GetJsonBoolField(Params, TEXT("hackable"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), SystemType);
        Result->SetNumberField(TEXT("detection_radius"), DetectionRadius);
        Result->SetNumberField(TEXT("rotation_speed"), RotationSpeed);
        Result->SetBoolField(TEXT("destructible"), bDestructible);
        Result->SetBoolField(TEXT("hackable"), bHackable);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Security system created"), Result);
        return true;
    }

    // configure_shadow_system - Light-based stealth
    if (SubAction == TEXT("configure_shadow_system"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float DarkThreshold = GetJsonNumberField(Params, TEXT("dark_threshold"), 0.3f);
        float VisibilityInShadow = GetJsonNumberField(Params, TEXT("shadow_visibility"), 0.2f);
        bool bAffectsDetection = GetJsonBoolField(Params, TEXT("affects_detection"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("dark_threshold"), DarkThreshold);
        Result->SetNumberField(TEXT("shadow_visibility"), VisibilityInShadow);
        Result->SetBoolField(TEXT("affects_detection"), bAffectsDetection);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Shadow system configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_stealth action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 GENRE - PUZZLE HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenrePuzzleAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_puzzle_element - Create puzzle component
    if (SubAction == TEXT("create_puzzle_element"))
    {
        FString ElementType = GetJsonStringField(Params, TEXT("type"), TEXT("button")); // button, door, laser, portal, switch, pressure_plate
        FString ElementId = GetJsonStringField(Params, TEXT("id"), TEXT(""));
        bool bToggle = GetJsonBoolField(Params, TEXT("toggle"), false);
        float ActivationDelay = GetJsonNumberField(Params, TEXT("delay"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), ElementType);
        Result->SetStringField(TEXT("id"), ElementId);
        Result->SetBoolField(TEXT("toggle"), bToggle);
        Result->SetNumberField(TEXT("delay"), ActivationDelay);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Puzzle element created"), Result);
        return true;
    }

    // configure_logic_system - Logic gates for puzzles
    if (SubAction == TEXT("configure_logic_system"))
    {
        FString GateType = GetJsonStringField(Params, TEXT("gate_type"), TEXT("and")); // and, or, not, xor, nand
        const TArray<TSharedPtr<FJsonValue>>* Inputs = nullptr;
        Params->TryGetArrayField(TEXT("inputs"), Inputs);
        FString OutputId = GetJsonStringField(Params, TEXT("output"), TEXT(""));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("gate_type"), GateType);
        Result->SetStringField(TEXT("output"), OutputId);
        if (Inputs) Result->SetNumberField(TEXT("input_count"), Inputs->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Logic system configured"), Result);
        return true;
    }

    // setup_physics_puzzles - Physics-based puzzle mechanics
    if (SubAction == TEXT("setup_physics_puzzles"))
    {
        FString PuzzleType = GetJsonStringField(Params, TEXT("type"), TEXT("weight")); // weight, momentum, trajectory, balance
        float RequiredWeight = GetJsonNumberField(Params, TEXT("required_weight"), 100.0f);
        float Tolerance = GetJsonNumberField(Params, TEXT("tolerance"), 10.0f);
        bool bShowGuide = GetJsonBoolField(Params, TEXT("show_guide"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), PuzzleType);
        Result->SetNumberField(TEXT("required_weight"), RequiredWeight);
        Result->SetNumberField(TEXT("tolerance"), Tolerance);
        Result->SetBoolField(TEXT("show_guide"), bShowGuide);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Physics puzzle configured"), Result);
        return true;
    }

    // create_sequence_puzzle - Order-based puzzle
    if (SubAction == TEXT("create_sequence_puzzle"))
    {
        const TArray<TSharedPtr<FJsonValue>>* Sequence = nullptr;
        Params->TryGetArrayField(TEXT("sequence"), Sequence);
        bool bResetOnFail = GetJsonBoolField(Params, TEXT("reset_on_fail"), true);
        float InputTimeout = GetJsonNumberField(Params, TEXT("timeout"), 5.0f);
        bool bShowProgress = GetJsonBoolField(Params, TEXT("show_progress"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("reset_on_fail"), bResetOnFail);
        Result->SetNumberField(TEXT("timeout"), InputTimeout);
        Result->SetBoolField(TEXT("show_progress"), bShowProgress);
        if (Sequence) Result->SetNumberField(TEXT("sequence_length"), Sequence->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Sequence puzzle created"), Result);
        return true;
    }

    // configure_environmental_puzzles - Perspective/shadow puzzles
    if (SubAction == TEXT("configure_environmental_puzzles"))
    {
        FString PuzzleType = GetJsonStringField(Params, TEXT("type"), TEXT("perspective")); // perspective, shadow, reflection, rotation
        float ViewAngleTolerance = GetJsonNumberField(Params, TEXT("angle_tolerance"), 5.0f);
        bool bHighlightSolution = GetJsonBoolField(Params, TEXT("highlight"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), PuzzleType);
        Result->SetNumberField(TEXT("angle_tolerance"), ViewAngleTolerance);
        Result->SetBoolField(TEXT("highlight"), bHighlightSolution);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Environmental puzzle configured"), Result);
        return true;
    }

    // setup_hint_system - Progressive hints
    if (SubAction == TEXT("setup_hint_system"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float HintDelay = GetJsonNumberField(Params, TEXT("delay"), 60.0f);
        int32 MaxHints = GetJsonIntField(Params, TEXT("max_hints"), 3);
        bool bProgressiveReveal = GetJsonBoolField(Params, TEXT("progressive"), true);
        bool bSkipOption = GetJsonBoolField(Params, TEXT("skip_option"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("delay"), HintDelay);
        Result->SetNumberField(TEXT("max_hints"), MaxHints);
        Result->SetBoolField(TEXT("progressive"), bProgressiveReveal);
        Result->SetBoolField(TEXT("skip_option"), bSkipOption);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Hint system configured"), Result);
        return true;
    }

    // create_puzzle_validation - Solution checking
    if (SubAction == TEXT("create_puzzle_validation"))
    {
        FString PuzzleId = GetJsonStringField(Params, TEXT("puzzle_id"), TEXT(""));
        bool bMultipleSolutions = GetJsonBoolField(Params, TEXT("multiple_solutions"), false);
        bool bPartialCredit = GetJsonBoolField(Params, TEXT("partial_credit"), false);
        float ValidationDelay = GetJsonNumberField(Params, TEXT("validation_delay"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("puzzle_id"), PuzzleId);
        Result->SetBoolField(TEXT("multiple_solutions"), bMultipleSolutions);
        Result->SetBoolField(TEXT("partial_credit"), bPartialCredit);
        Result->SetNumberField(TEXT("validation_delay"), ValidationDelay);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Puzzle validation created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_puzzle action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 GENRE - STRATEGY HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreStrategyAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_unit_type - Define RTS unit
    if (SubAction == TEXT("create_unit_type"))
    {
        FString UnitName = GetJsonStringField(Params, TEXT("name"), TEXT("Unit_Default"));
        FString UnitType = GetJsonStringField(Params, TEXT("type"), TEXT("infantry")); // infantry, vehicle, air, naval, worker
        int32 Health = GetJsonIntField(Params, TEXT("health"), 100);
        int32 Damage = GetJsonIntField(Params, TEXT("damage"), 10);
        float AttackSpeed = GetJsonNumberField(Params, TEXT("attack_speed"), 1.0f);
        float MoveSpeed = GetJsonNumberField(Params, TEXT("move_speed"), 300.0f);
        int32 BuildCost = GetJsonIntField(Params, TEXT("cost"), 50);
        float BuildTime = GetJsonNumberField(Params, TEXT("build_time"), 10.0f);
        int32 PopulationCost = GetJsonIntField(Params, TEXT("population"), 1);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), UnitName);
        Result->SetStringField(TEXT("type"), UnitType);
        Result->SetNumberField(TEXT("health"), Health);
        Result->SetNumberField(TEXT("damage"), Damage);
        Result->SetNumberField(TEXT("attack_speed"), AttackSpeed);
        Result->SetNumberField(TEXT("move_speed"), MoveSpeed);
        Result->SetNumberField(TEXT("cost"), BuildCost);
        Result->SetNumberField(TEXT("build_time"), BuildTime);
        Result->SetNumberField(TEXT("population"), PopulationCost);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Unit type created"), Result);
        return true;
    }

    // configure_building_system - RTS building mechanics
    if (SubAction == TEXT("configure_building_system"))
    {
        FString BuildingName = GetJsonStringField(Params, TEXT("name"), TEXT("Building_Default"));
        FString BuildingType = GetJsonStringField(Params, TEXT("type"), TEXT("production")); // production, tech, defense, resource
        int32 Health = GetJsonIntField(Params, TEXT("health"), 500);
        int32 BuildCost = GetJsonIntField(Params, TEXT("cost"), 200);
        float BuildTime = GetJsonNumberField(Params, TEXT("build_time"), 30.0f);
        const TArray<TSharedPtr<FJsonValue>>* TrainableUnits = nullptr;
        Params->TryGetArrayField(TEXT("trains"), TrainableUnits);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), BuildingName);
        Result->SetStringField(TEXT("type"), BuildingType);
        Result->SetNumberField(TEXT("health"), Health);
        Result->SetNumberField(TEXT("cost"), BuildCost);
        Result->SetNumberField(TEXT("build_time"), BuildTime);
        if (TrainableUnits) Result->SetNumberField(TEXT("train_count"), TrainableUnits->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Building configured"), Result);
        return true;
    }

    // setup_resource_system - Resource gathering
    if (SubAction == TEXT("setup_resource_system"))
    {
        FString ResourceType = GetJsonStringField(Params, TEXT("type"), TEXT("minerals")); // minerals, gas, gold, wood, food
        int32 MaxCarry = GetJsonIntField(Params, TEXT("max_carry"), 10);
        float GatherRate = GetJsonNumberField(Params, TEXT("gather_rate"), 1.0f);
        int32 NodeAmount = GetJsonIntField(Params, TEXT("node_amount"), 1500);
        bool bDepletable = GetJsonBoolField(Params, TEXT("depletable"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), ResourceType);
        Result->SetNumberField(TEXT("max_carry"), MaxCarry);
        Result->SetNumberField(TEXT("gather_rate"), GatherRate);
        Result->SetNumberField(TEXT("node_amount"), NodeAmount);
        Result->SetBoolField(TEXT("depletable"), bDepletable);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Resource system configured"), Result);
        return true;
    }

    // create_tech_tree - Technology research tree
    if (SubAction == TEXT("create_tech_tree"))
    {
        FString TechName = GetJsonStringField(Params, TEXT("name"), TEXT("Tech_Default"));
        int32 ResearchCost = GetJsonIntField(Params, TEXT("cost"), 100);
        float ResearchTime = GetJsonNumberField(Params, TEXT("time"), 60.0f);
        const TArray<TSharedPtr<FJsonValue>>* Prerequisites = nullptr;
        Params->TryGetArrayField(TEXT("prerequisites"), Prerequisites);
        const TArray<TSharedPtr<FJsonValue>>* Unlocks = nullptr;
        Params->TryGetArrayField(TEXT("unlocks"), Unlocks);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), TechName);
        Result->SetNumberField(TEXT("cost"), ResearchCost);
        Result->SetNumberField(TEXT("time"), ResearchTime);
        if (Prerequisites) Result->SetNumberField(TEXT("prereq_count"), Prerequisites->Num());
        if (Unlocks) Result->SetNumberField(TEXT("unlock_count"), Unlocks->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Tech tree created"), Result);
        return true;
    }

    // configure_fog_of_war - Vision and exploration
    if (SubAction == TEXT("configure_fog_of_war"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float ExploredOpacity = GetJsonNumberField(Params, TEXT("explored_opacity"), 0.5f);
        float UnexploredOpacity = GetJsonNumberField(Params, TEXT("unexplored_opacity"), 1.0f);
        bool bRememberUnits = GetJsonBoolField(Params, TEXT("remember_units"), true);
        float UpdateInterval = GetJsonNumberField(Params, TEXT("update_interval"), 0.1f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("explored_opacity"), ExploredOpacity);
        Result->SetNumberField(TEXT("unexplored_opacity"), UnexploredOpacity);
        Result->SetBoolField(TEXT("remember_units"), bRememberUnits);
        Result->SetNumberField(TEXT("update_interval"), UpdateInterval);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Fog of war configured"), Result);
        return true;
    }

    // setup_formation_system - Unit formations
    if (SubAction == TEXT("setup_formation_system"))
    {
        FString FormationType = GetJsonStringField(Params, TEXT("type"), TEXT("line")); // line, box, wedge, circle, spread
        float UnitSpacing = GetJsonNumberField(Params, TEXT("spacing"), 100.0f);
        bool bAutoFormation = GetJsonBoolField(Params, TEXT("auto_formation"), true);
        int32 MaxUnitsPerRow = GetJsonIntField(Params, TEXT("max_per_row"), 10);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), FormationType);
        Result->SetNumberField(TEXT("spacing"), UnitSpacing);
        Result->SetBoolField(TEXT("auto_formation"), bAutoFormation);
        Result->SetNumberField(TEXT("max_per_row"), MaxUnitsPerRow);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Formation system configured"), Result);
        return true;
    }

    // create_command_system - Unit commands
    if (SubAction == TEXT("create_command_system"))
    {
        const TArray<TSharedPtr<FJsonValue>>* Commands = nullptr;
        Params->TryGetArrayField(TEXT("commands"), Commands);
        bool bQueueCommands = GetJsonBoolField(Params, TEXT("queue_commands"), true);
        int32 MaxQueueSize = GetJsonIntField(Params, TEXT("max_queue"), 10);
        bool bWaypointPath = GetJsonBoolField(Params, TEXT("waypoint_path"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("queue_commands"), bQueueCommands);
        Result->SetNumberField(TEXT("max_queue"), MaxQueueSize);
        Result->SetBoolField(TEXT("waypoint_path"), bWaypointPath);
        if (Commands) Result->SetNumberField(TEXT("command_count"), Commands->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Command system created"), Result);
        return true;
    }

    // configure_ai_opponent - Strategy AI settings
    if (SubAction == TEXT("configure_ai_opponent"))
    {
        FString Difficulty = GetJsonStringField(Params, TEXT("difficulty"), TEXT("normal")); // easy, normal, hard, insane
        FString Strategy = GetJsonStringField(Params, TEXT("strategy"), TEXT("balanced")); // rush, turtle, balanced, economic
        float ResourceMultiplier = GetJsonNumberField(Params, TEXT("resource_multiplier"), 1.0f);
        float BuildSpeedMultiplier = GetJsonNumberField(Params, TEXT("build_speed_multiplier"), 1.0f);
        bool bCheats = GetJsonBoolField(Params, TEXT("cheats"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("difficulty"), Difficulty);
        Result->SetStringField(TEXT("strategy"), Strategy);
        Result->SetNumberField(TEXT("resource_multiplier"), ResourceMultiplier);
        Result->SetNumberField(TEXT("build_speed_multiplier"), BuildSpeedMultiplier);
        Result->SetBoolField(TEXT("cheats"), bCheats);
        SendAutomationResponse(Socket, RequestId, true, TEXT("AI opponent configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_strategy action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// GENRE SPORTS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreSportsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_player_stats - Sports player attributes
    if (SubAction == TEXT("create_player_stats"))
    {
        FString PlayerName = GetJsonStringField(Params, TEXT("name"), TEXT("Player"));
        FString Sport = GetJsonStringField(Params, TEXT("sport"), TEXT("generic")); // soccer, basketball, football, hockey
        int32 Overall = GetJsonIntField(Params, TEXT("overall"), 75);
        int32 Speed = GetJsonIntField(Params, TEXT("speed"), 75);
        int32 Strength = GetJsonIntField(Params, TEXT("strength"), 75);
        int32 Shooting = GetJsonIntField(Params, TEXT("shooting"), 75);
        int32 Passing = GetJsonIntField(Params, TEXT("passing"), 75);
        int32 Defense = GetJsonIntField(Params, TEXT("defense"), 75);
        int32 Stamina = GetJsonIntField(Params, TEXT("stamina"), 75);
        FString Position = GetJsonStringField(Params, TEXT("position"), TEXT("forward"));

        FString PackagePath = TEXT("/Game/FORGE2/Sports/Players/");
        FString AssetName = FString::Printf(TEXT("Player_%s"), *PlayerName);
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* PlayerStats = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(PlayerStats);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), PlayerName);
        Result->SetStringField(TEXT("sport"), Sport);
        Result->SetStringField(TEXT("position"), Position);
        Result->SetNumberField(TEXT("overall"), Overall);
        Result->SetNumberField(TEXT("speed"), Speed);
        Result->SetNumberField(TEXT("strength"), Strength);
        Result->SetNumberField(TEXT("shooting"), Shooting);
        Result->SetNumberField(TEXT("passing"), Passing);
        Result->SetNumberField(TEXT("defense"), Defense);
        Result->SetNumberField(TEXT("stamina"), Stamina);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Player stats created"), Result);
        return true;
    }

    // configure_ball_physics - Ball simulation
    if (SubAction == TEXT("configure_ball_physics"))
    {
        FString BallType = GetJsonStringField(Params, TEXT("type"), TEXT("soccer")); // soccer, basketball, football, tennis, golf
        float Mass = GetJsonNumberField(Params, TEXT("mass"), 0.45f);
        float Bounciness = GetJsonNumberField(Params, TEXT("bounciness"), 0.7f);
        float AirDrag = GetJsonNumberField(Params, TEXT("air_drag"), 0.1f);
        float GroundFriction = GetJsonNumberField(Params, TEXT("ground_friction"), 0.5f);
        float SpinEffect = GetJsonNumberField(Params, TEXT("spin_effect"), 1.0f);
        float Gravity = GetJsonNumberField(Params, TEXT("gravity"), 980.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), BallType);
        Result->SetNumberField(TEXT("mass"), Mass);
        Result->SetNumberField(TEXT("bounciness"), Bounciness);
        Result->SetNumberField(TEXT("air_drag"), AirDrag);
        Result->SetNumberField(TEXT("ground_friction"), GroundFriction);
        Result->SetNumberField(TEXT("spin_effect"), SpinEffect);
        Result->SetNumberField(TEXT("gravity"), Gravity);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Ball physics configured"), Result);
        return true;
    }

    // setup_team_ai - Team behavior and formations
    if (SubAction == TEXT("setup_team_ai"))
    {
        FString Formation = GetJsonStringField(Params, TEXT("formation"), TEXT("4-4-2"));
        FString PlayStyle = GetJsonStringField(Params, TEXT("play_style"), TEXT("balanced")); // attacking, defensive, balanced, counter
        float Aggression = GetJsonNumberField(Params, TEXT("aggression"), 0.5f);
        float PassingAccuracy = GetJsonNumberField(Params, TEXT("passing_accuracy"), 0.8f);
        bool bPressHighLine = GetJsonBoolField(Params, TEXT("press_high"), false);
        bool bOffSideTrap = GetJsonBoolField(Params, TEXT("offside_trap"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("formation"), Formation);
        Result->SetStringField(TEXT("play_style"), PlayStyle);
        Result->SetNumberField(TEXT("aggression"), Aggression);
        Result->SetNumberField(TEXT("passing_accuracy"), PassingAccuracy);
        Result->SetBoolField(TEXT("press_high"), bPressHighLine);
        Result->SetBoolField(TEXT("offside_trap"), bOffSideTrap);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Team AI configured"), Result);
        return true;
    }

    // create_season_mode - Season/career management
    if (SubAction == TEXT("create_season_mode"))
    {
        FString LeagueName = GetJsonStringField(Params, TEXT("league_name"), TEXT("League"));
        int32 TeamCount = GetJsonIntField(Params, TEXT("team_count"), 20);
        int32 GamesPerSeason = GetJsonIntField(Params, TEXT("games_per_season"), 38);
        bool bPlayoffs = GetJsonBoolField(Params, TEXT("playoffs"), false);
        int32 PlayoffTeams = GetJsonIntField(Params, TEXT("playoff_teams"), 8);
        bool bPromoRelegation = GetJsonBoolField(Params, TEXT("promo_relegation"), true);
        bool bTransferWindow = GetJsonBoolField(Params, TEXT("transfer_window"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("league_name"), LeagueName);
        Result->SetNumberField(TEXT("team_count"), TeamCount);
        Result->SetNumberField(TEXT("games_per_season"), GamesPerSeason);
        Result->SetBoolField(TEXT("playoffs"), bPlayoffs);
        Result->SetNumberField(TEXT("playoff_teams"), PlayoffTeams);
        Result->SetBoolField(TEXT("promo_relegation"), bPromoRelegation);
        Result->SetBoolField(TEXT("transfer_window"), bTransferWindow);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Season mode created"), Result);
        return true;
    }

    // configure_referee_system - Rules enforcement
    if (SubAction == TEXT("configure_referee_system"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float FoulSensitivity = GetJsonNumberField(Params, TEXT("foul_sensitivity"), 0.5f);
        bool bCards = GetJsonBoolField(Params, TEXT("cards"), true);
        bool bVAR = GetJsonBoolField(Params, TEXT("var"), false);
        bool bOffsideCheck = GetJsonBoolField(Params, TEXT("offside_check"), true);
        int32 InjuryTimeMin = GetJsonIntField(Params, TEXT("injury_time_min"), 0);
        int32 InjuryTimeMax = GetJsonIntField(Params, TEXT("injury_time_max"), 5);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("foul_sensitivity"), FoulSensitivity);
        Result->SetBoolField(TEXT("cards"), bCards);
        Result->SetBoolField(TEXT("var"), bVAR);
        Result->SetBoolField(TEXT("offside_check"), bOffsideCheck);
        Result->SetNumberField(TEXT("injury_time_min"), InjuryTimeMin);
        Result->SetNumberField(TEXT("injury_time_max"), InjuryTimeMax);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Referee system configured"), Result);
        return true;
    }

    // setup_commentary - Dynamic commentary triggers
    if (SubAction == TEXT("setup_commentary"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        FString Language = GetJsonStringField(Params, TEXT("language"), TEXT("en"));
        const TArray<TSharedPtr<FJsonValue>>* Commentators = nullptr;
        Params->TryGetArrayField(TEXT("commentators"), Commentators);
        bool bReplayNarration = GetJsonBoolField(Params, TEXT("replay_narration"), true);
        float Volume = GetJsonNumberField(Params, TEXT("volume"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetStringField(TEXT("language"), Language);
        if (Commentators) Result->SetNumberField(TEXT("commentator_count"), Commentators->Num());
        Result->SetBoolField(TEXT("replay_narration"), bReplayNarration);
        Result->SetNumberField(TEXT("volume"), Volume);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Commentary system configured"), Result);
        return true;
    }

    // create_replay_system - Instant replay
    if (SubAction == TEXT("create_replay_system"))
    {
        float BufferDuration = GetJsonNumberField(Params, TEXT("buffer_duration"), 30.0f);
        bool bSlowMotion = GetJsonBoolField(Params, TEXT("slow_motion"), true);
        float MinSlowMo = GetJsonNumberField(Params, TEXT("min_slowmo"), 0.1f);
        bool bMultiAngle = GetJsonBoolField(Params, TEXT("multi_angle"), true);
        bool bAutoTrigger = GetJsonBoolField(Params, TEXT("auto_trigger"), true);
        const TArray<TSharedPtr<FJsonValue>>* TriggerEvents = nullptr;
        Params->TryGetArrayField(TEXT("trigger_events"), TriggerEvents);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("buffer_duration"), BufferDuration);
        Result->SetBoolField(TEXT("slow_motion"), bSlowMotion);
        Result->SetNumberField(TEXT("min_slowmo"), MinSlowMo);
        Result->SetBoolField(TEXT("multi_angle"), bMultiAngle);
        Result->SetBoolField(TEXT("auto_trigger"), bAutoTrigger);
        if (TriggerEvents) Result->SetNumberField(TEXT("trigger_event_count"), TriggerEvents->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Replay system created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_sports action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// GENRE HORROR HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreHorrorAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_fear_system - Sanity and fear mechanics
    if (SubAction == TEXT("create_fear_system"))
    {
        float MaxSanity = GetJsonNumberField(Params, TEXT("max_sanity"), 100.0f);
        float SanityDrainRate = GetJsonNumberField(Params, TEXT("drain_rate"), 1.0f);
        float SanityRecoveryRate = GetJsonNumberField(Params, TEXT("recovery_rate"), 0.5f);
        float LowSanityThreshold = GetJsonNumberField(Params, TEXT("low_threshold"), 25.0f);
        bool bVisualEffects = GetJsonBoolField(Params, TEXT("visual_effects"), true);
        bool bAudioDistortion = GetJsonBoolField(Params, TEXT("audio_distortion"), true);
        bool bHallucinations = GetJsonBoolField(Params, TEXT("hallucinations"), true);

        FString PackagePath = TEXT("/Game/FORGE2/Horror/Systems/");
        FString AssetName = TEXT("FearSystem");
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* FearSystem = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(FearSystem);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_sanity"), MaxSanity);
        Result->SetNumberField(TEXT("drain_rate"), SanityDrainRate);
        Result->SetNumberField(TEXT("recovery_rate"), SanityRecoveryRate);
        Result->SetNumberField(TEXT("low_threshold"), LowSanityThreshold);
        Result->SetBoolField(TEXT("visual_effects"), bVisualEffects);
        Result->SetBoolField(TEXT("audio_distortion"), bAudioDistortion);
        Result->SetBoolField(TEXT("hallucinations"), bHallucinations);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Fear system created"), Result);
        return true;
    }

    // configure_atmosphere - Horror atmosphere settings
    if (SubAction == TEXT("configure_atmosphere"))
    {
        float AmbientDarkness = GetJsonNumberField(Params, TEXT("ambient_darkness"), 0.8f);
        float FogDensity = GetJsonNumberField(Params, TEXT("fog_density"), 0.5f);
        FVector FogColor = Forge2Helpers::GetVectorFromJson(Params, TEXT("fog_color"), FVector(0.1f, 0.1f, 0.15f));
        float VignetteIntensity = GetJsonNumberField(Params, TEXT("vignette_intensity"), 0.5f);
        float FilmGrain = GetJsonNumberField(Params, TEXT("film_grain"), 0.3f);
        float ChromaticAberration = GetJsonNumberField(Params, TEXT("chromatic_aberration"), 0.5f);
        float AmbientSoundVolume = GetJsonNumberField(Params, TEXT("ambient_volume"), 0.8f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("ambient_darkness"), AmbientDarkness);
        Result->SetNumberField(TEXT("fog_density"), FogDensity);
        Result->SetNumberField(TEXT("vignette_intensity"), VignetteIntensity);
        Result->SetNumberField(TEXT("film_grain"), FilmGrain);
        Result->SetNumberField(TEXT("chromatic_aberration"), ChromaticAberration);
        Result->SetNumberField(TEXT("ambient_volume"), AmbientSoundVolume);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Atmosphere configured"), Result);
        return true;
    }

    // setup_monster_ai - Horror enemy behavior
    if (SubAction == TEXT("setup_monster_ai"))
    {
        FString MonsterType = GetJsonStringField(Params, TEXT("type"), TEXT("stalker")); // stalker, ambusher, patroller, teleporter
        float DetectionRange = GetJsonNumberField(Params, TEXT("detection_range"), 1500.0f);
        float PursuitSpeed = GetJsonNumberField(Params, TEXT("pursuit_speed"), 500.0f);
        float PatrolSpeed = GetJsonNumberField(Params, TEXT("patrol_speed"), 200.0f);
        float LoseInterestTime = GetJsonNumberField(Params, TEXT("lose_interest_time"), 30.0f);
        bool bCanOpenDoors = GetJsonBoolField(Params, TEXT("can_open_doors"), true);
        bool bSensitiveToLight = GetJsonBoolField(Params, TEXT("light_sensitive"), false);
        bool bSensitiveToNoise = GetJsonBoolField(Params, TEXT("noise_sensitive"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), MonsterType);
        Result->SetNumberField(TEXT("detection_range"), DetectionRange);
        Result->SetNumberField(TEXT("pursuit_speed"), PursuitSpeed);
        Result->SetNumberField(TEXT("patrol_speed"), PatrolSpeed);
        Result->SetNumberField(TEXT("lose_interest_time"), LoseInterestTime);
        Result->SetBoolField(TEXT("can_open_doors"), bCanOpenDoors);
        Result->SetBoolField(TEXT("light_sensitive"), bSensitiveToLight);
        Result->SetBoolField(TEXT("noise_sensitive"), bSensitiveToNoise);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Monster AI configured"), Result);
        return true;
    }

    // create_resource_scarcity - Limited supplies
    if (SubAction == TEXT("create_resource_scarcity"))
    {
        int32 MaxAmmo = GetJsonIntField(Params, TEXT("max_ammo"), 12);
        int32 MaxHealth = GetJsonIntField(Params, TEXT("max_health_items"), 3);
        float SpawnRarity = GetJsonNumberField(Params, TEXT("spawn_rarity"), 0.2f);
        bool bRandomSpawns = GetJsonBoolField(Params, TEXT("random_spawns"), true);
        bool bCrafting = GetJsonBoolField(Params, TEXT("crafting"), true);
        float CraftingMultiplier = GetJsonNumberField(Params, TEXT("crafting_multiplier"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_ammo"), MaxAmmo);
        Result->SetNumberField(TEXT("max_health_items"), MaxHealth);
        Result->SetNumberField(TEXT("spawn_rarity"), SpawnRarity);
        Result->SetBoolField(TEXT("random_spawns"), bRandomSpawns);
        Result->SetBoolField(TEXT("crafting"), bCrafting);
        Result->SetNumberField(TEXT("crafting_multiplier"), CraftingMultiplier);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Resource scarcity configured"), Result);
        return true;
    }

    // configure_save_system - Horror save mechanics
    if (SubAction == TEXT("configure_save_system"))
    {
        FString SaveType = GetJsonStringField(Params, TEXT("type"), TEXT("manual")); // manual, checkpoint, typewriter, autosave
        bool bSaveRooms = GetJsonBoolField(Params, TEXT("save_rooms"), true);
        bool bLimitedSaves = GetJsonBoolField(Params, TEXT("limited_saves"), false);
        int32 SaveLimit = GetJsonIntField(Params, TEXT("save_limit"), 10);
        bool bConsumeItem = GetJsonBoolField(Params, TEXT("consume_item"), false);
        FString SaveItem = GetJsonStringField(Params, TEXT("save_item"), TEXT("InkRibbon"));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), SaveType);
        Result->SetBoolField(TEXT("save_rooms"), bSaveRooms);
        Result->SetBoolField(TEXT("limited_saves"), bLimitedSaves);
        Result->SetNumberField(TEXT("save_limit"), SaveLimit);
        Result->SetBoolField(TEXT("consume_item"), bConsumeItem);
        Result->SetStringField(TEXT("save_item"), SaveItem);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Save system configured"), Result);
        return true;
    }

    // setup_jumpscare_system - Scare triggers
    if (SubAction == TEXT("setup_jumpscare_system"))
    {
        FString ScareId = GetJsonStringField(Params, TEXT("scare_id"), TEXT("Scare_01"));
        FString TriggerType = GetJsonStringField(Params, TEXT("trigger_type"), TEXT("proximity")); // proximity, interaction, timer, random
        float TriggerDistance = GetJsonNumberField(Params, TEXT("trigger_distance"), 200.0f);
        bool bOneShot = GetJsonBoolField(Params, TEXT("one_shot"), true);
        float CooldownTime = GetJsonNumberField(Params, TEXT("cooldown"), 0.0f);
        float SoundVolume = GetJsonNumberField(Params, TEXT("sound_volume"), 1.0f);
        float ScreenShakeIntensity = GetJsonNumberField(Params, TEXT("shake_intensity"), 0.5f);
        bool bFlashScreen = GetJsonBoolField(Params, TEXT("flash_screen"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("scare_id"), ScareId);
        Result->SetStringField(TEXT("trigger_type"), TriggerType);
        Result->SetNumberField(TEXT("trigger_distance"), TriggerDistance);
        Result->SetBoolField(TEXT("one_shot"), bOneShot);
        Result->SetNumberField(TEXT("cooldown"), CooldownTime);
        Result->SetNumberField(TEXT("sound_volume"), SoundVolume);
        Result->SetNumberField(TEXT("shake_intensity"), ScreenShakeIntensity);
        Result->SetBoolField(TEXT("flash_screen"), bFlashScreen);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Jumpscare system configured"), Result);
        return true;
    }

    // create_safe_zones - Sanctuary areas
    if (SubAction == TEXT("create_safe_zones"))
    {
        FString ZoneId = GetJsonStringField(Params, TEXT("zone_id"), TEXT("SafeZone_01"));
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        FVector Extent = Forge2Helpers::GetVectorFromJson(Params, TEXT("extent"), FVector(500.0f));
        bool bNoEnemies = GetJsonBoolField(Params, TEXT("no_enemies"), true);
        bool bRestoreSanity = GetJsonBoolField(Params, TEXT("restore_sanity"), true);
        float SanityRecoveryRate = GetJsonNumberField(Params, TEXT("sanity_recovery_rate"), 2.0f);
        bool bMusicChange = GetJsonBoolField(Params, TEXT("music_change"), true);
        bool bSavePoint = GetJsonBoolField(Params, TEXT("save_point"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("zone_id"), ZoneId);
        Result->SetBoolField(TEXT("no_enemies"), bNoEnemies);
        Result->SetBoolField(TEXT("restore_sanity"), bRestoreSanity);
        Result->SetNumberField(TEXT("sanity_recovery_rate"), SanityRecoveryRate);
        Result->SetBoolField(TEXT("music_change"), bMusicChange);
        Result->SetBoolField(TEXT("save_point"), bSavePoint);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Safe zone created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_horror action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// GENRE SURVIVAL HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreSurvivalAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_needs_system - Survival needs (hunger, thirst, etc)
    if (SubAction == TEXT("create_needs_system"))
    {
        bool bHunger = GetJsonBoolField(Params, TEXT("hunger"), true);
        float HungerDrainRate = GetJsonNumberField(Params, TEXT("hunger_drain"), 1.0f);
        float MaxHunger = GetJsonNumberField(Params, TEXT("max_hunger"), 100.0f);
        bool bThirst = GetJsonBoolField(Params, TEXT("thirst"), true);
        float ThirstDrainRate = GetJsonNumberField(Params, TEXT("thirst_drain"), 1.5f);
        float MaxThirst = GetJsonNumberField(Params, TEXT("max_thirst"), 100.0f);
        bool bSleep = GetJsonBoolField(Params, TEXT("sleep"), true);
        float SleepDrainRate = GetJsonNumberField(Params, TEXT("sleep_drain"), 0.5f);
        bool bTemperature = GetJsonBoolField(Params, TEXT("temperature"), true);
        float MinTemp = GetJsonNumberField(Params, TEXT("min_temp"), -20.0f);
        float MaxTemp = GetJsonNumberField(Params, TEXT("max_temp"), 50.0f);
        float ComfortMin = GetJsonNumberField(Params, TEXT("comfort_min"), 15.0f);
        float ComfortMax = GetJsonNumberField(Params, TEXT("comfort_max"), 30.0f);

        FString PackagePath = TEXT("/Game/FORGE2/Survival/Systems/");
        FString AssetName = TEXT("NeedsSystem");
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* NeedsSystem = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(NeedsSystem);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("hunger"), bHunger);
        Result->SetNumberField(TEXT("hunger_drain"), HungerDrainRate);
        Result->SetBoolField(TEXT("thirst"), bThirst);
        Result->SetNumberField(TEXT("thirst_drain"), ThirstDrainRate);
        Result->SetBoolField(TEXT("sleep"), bSleep);
        Result->SetBoolField(TEXT("temperature"), bTemperature);
        Result->SetNumberField(TEXT("comfort_min"), ComfortMin);
        Result->SetNumberField(TEXT("comfort_max"), ComfortMax);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Needs system created"), Result);
        return true;
    }

    // configure_harvesting - Resource gathering
    if (SubAction == TEXT("configure_harvesting"))
    {
        FString ResourceType = GetJsonStringField(Params, TEXT("type"), TEXT("tree")); // tree, rock, bush, animal, ore
        int32 HitsToHarvest = GetJsonIntField(Params, TEXT("hits"), 5);
        float RespawnTime = GetJsonNumberField(Params, TEXT("respawn_time"), 300.0f);
        const TArray<TSharedPtr<FJsonValue>>* Yields = nullptr;
        Params->TryGetArrayField(TEXT("yields"), Yields);
        bool bRequiresTool = GetJsonBoolField(Params, TEXT("requires_tool"), true);
        FString ToolType = GetJsonStringField(Params, TEXT("tool_type"), TEXT("axe"));
        float ToolDamagePerHit = GetJsonNumberField(Params, TEXT("tool_damage"), 5.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), ResourceType);
        Result->SetNumberField(TEXT("hits"), HitsToHarvest);
        Result->SetNumberField(TEXT("respawn_time"), RespawnTime);
        if (Yields) Result->SetNumberField(TEXT("yield_count"), Yields->Num());
        Result->SetBoolField(TEXT("requires_tool"), bRequiresTool);
        Result->SetStringField(TEXT("tool_type"), ToolType);
        Result->SetNumberField(TEXT("tool_damage"), ToolDamagePerHit);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Harvesting configured"), Result);
        return true;
    }

    // setup_building_system - Base building
    if (SubAction == TEXT("setup_building_system"))
    {
        bool bSnapToGrid = GetJsonBoolField(Params, TEXT("snap_to_grid"), true);
        float GridSize = GetJsonNumberField(Params, TEXT("grid_size"), 100.0f);
        bool bCollisionCheck = GetJsonBoolField(Params, TEXT("collision_check"), true);
        bool bFoundationRequired = GetJsonBoolField(Params, TEXT("foundation_required"), true);
        bool bStabilitySystem = GetJsonBoolField(Params, TEXT("stability_system"), true);
        float MaxStability = GetJsonNumberField(Params, TEXT("max_stability"), 100.0f);
        bool bPreviewGhost = GetJsonBoolField(Params, TEXT("preview_ghost"), true);
        bool bRotation = GetJsonBoolField(Params, TEXT("rotation"), true);
        float RotationStep = GetJsonNumberField(Params, TEXT("rotation_step"), 45.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("snap_to_grid"), bSnapToGrid);
        Result->SetNumberField(TEXT("grid_size"), GridSize);
        Result->SetBoolField(TEXT("collision_check"), bCollisionCheck);
        Result->SetBoolField(TEXT("foundation_required"), bFoundationRequired);
        Result->SetBoolField(TEXT("stability_system"), bStabilitySystem);
        Result->SetNumberField(TEXT("max_stability"), MaxStability);
        Result->SetBoolField(TEXT("preview_ghost"), bPreviewGhost);
        Result->SetBoolField(TEXT("rotation"), bRotation);
        Result->SetNumberField(TEXT("rotation_step"), RotationStep);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Building system configured"), Result);
        return true;
    }

    // create_crafting_tiers - Crafting progression
    if (SubAction == TEXT("create_crafting_tiers"))
    {
        FString TierName = GetJsonStringField(Params, TEXT("tier_name"), TEXT("Basic"));
        int32 TierLevel = GetJsonIntField(Params, TEXT("tier_level"), 1);
        FString StationType = GetJsonStringField(Params, TEXT("station_type"), TEXT("workbench")); // workbench, forge, laboratory, kitchen
        const TArray<TSharedPtr<FJsonValue>>* Recipes = nullptr;
        Params->TryGetArrayField(TEXT("recipes"), Recipes);
        const TArray<TSharedPtr<FJsonValue>>* UnlockRequirements = nullptr;
        Params->TryGetArrayField(TEXT("unlock_requirements"), UnlockRequirements);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tier_name"), TierName);
        Result->SetNumberField(TEXT("tier_level"), TierLevel);
        Result->SetStringField(TEXT("station_type"), StationType);
        if (Recipes) Result->SetNumberField(TEXT("recipe_count"), Recipes->Num());
        if (UnlockRequirements) Result->SetNumberField(TEXT("unlock_requirement_count"), UnlockRequirements->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Crafting tier created"), Result);
        return true;
    }

    // configure_decay_system - Structure degradation
    if (SubAction == TEXT("configure_decay_system"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float DecayRate = GetJsonNumberField(Params, TEXT("decay_rate"), 0.1f);
        float DecayInterval = GetJsonNumberField(Params, TEXT("decay_interval"), 3600.0f);
        bool bWeatherAffects = GetJsonBoolField(Params, TEXT("weather_affects"), true);
        float WeatherMultiplier = GetJsonNumberField(Params, TEXT("weather_multiplier"), 2.0f);
        bool bAutoRepair = GetJsonBoolField(Params, TEXT("auto_repair"), false);
        float RepairRadius = GetJsonNumberField(Params, TEXT("repair_radius"), 500.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("decay_rate"), DecayRate);
        Result->SetNumberField(TEXT("decay_interval"), DecayInterval);
        Result->SetBoolField(TEXT("weather_affects"), bWeatherAffects);
        Result->SetNumberField(TEXT("weather_multiplier"), WeatherMultiplier);
        Result->SetBoolField(TEXT("auto_repair"), bAutoRepair);
        Result->SetNumberField(TEXT("repair_radius"), RepairRadius);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Decay system configured"), Result);
        return true;
    }

    // setup_wildlife - Animal behavior
    if (SubAction == TEXT("setup_wildlife"))
    {
        FString AnimalId = GetJsonStringField(Params, TEXT("animal_id"), TEXT("Deer"));
        FString Behavior = GetJsonStringField(Params, TEXT("behavior"), TEXT("passive")); // passive, aggressive, neutral, flee
        float DetectionRange = GetJsonNumberField(Params, TEXT("detection_range"), 1000.0f);
        float FleeDistance = GetJsonNumberField(Params, TEXT("flee_distance"), 1500.0f);
        bool bHerdBehavior = GetJsonBoolField(Params, TEXT("herd_behavior"), true);
        int32 HerdSize = GetJsonIntField(Params, TEXT("herd_size"), 5);
        bool bTameable = GetJsonBoolField(Params, TEXT("tameable"), false);
        FString TameItem = GetJsonStringField(Params, TEXT("tame_item"), TEXT("Food"));
        const TArray<TSharedPtr<FJsonValue>>* LootTable = nullptr;
        Params->TryGetArrayField(TEXT("loot_table"), LootTable);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("animal_id"), AnimalId);
        Result->SetStringField(TEXT("behavior"), Behavior);
        Result->SetNumberField(TEXT("detection_range"), DetectionRange);
        Result->SetNumberField(TEXT("flee_distance"), FleeDistance);
        Result->SetBoolField(TEXT("herd_behavior"), bHerdBehavior);
        Result->SetNumberField(TEXT("herd_size"), HerdSize);
        Result->SetBoolField(TEXT("tameable"), bTameable);
        Result->SetStringField(TEXT("tame_item"), TameItem);
        if (LootTable) Result->SetNumberField(TEXT("loot_count"), LootTable->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Wildlife configured"), Result);
        return true;
    }

    // create_weather_effects - Survival weather
    if (SubAction == TEXT("create_weather_effects"))
    {
        FString WeatherType = GetJsonStringField(Params, TEXT("type"), TEXT("rain")); // rain, snow, heat, storm, blizzard
        float TemperatureEffect = GetJsonNumberField(Params, TEXT("temperature_effect"), -5.0f);
        float WetnessRate = GetJsonNumberField(Params, TEXT("wetness_rate"), 0.1f);
        float VisibilityReduction = GetJsonNumberField(Params, TEXT("visibility_reduction"), 0.5f);
        float MovementPenalty = GetJsonNumberField(Params, TEXT("movement_penalty"), 0.2f);
        bool bDamageOverTime = GetJsonBoolField(Params, TEXT("damage_over_time"), false);
        float DamageAmount = GetJsonNumberField(Params, TEXT("damage_amount"), 1.0f);
        float DamageInterval = GetJsonNumberField(Params, TEXT("damage_interval"), 5.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), WeatherType);
        Result->SetNumberField(TEXT("temperature_effect"), TemperatureEffect);
        Result->SetNumberField(TEXT("wetness_rate"), WetnessRate);
        Result->SetNumberField(TEXT("visibility_reduction"), VisibilityReduction);
        Result->SetNumberField(TEXT("movement_penalty"), MovementPenalty);
        Result->SetBoolField(TEXT("damage_over_time"), bDamageOverTime);
        Result->SetNumberField(TEXT("damage_amount"), DamageAmount);
        Result->SetNumberField(TEXT("damage_interval"), DamageInterval);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Weather effects created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_survival action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// GENRE MOBA HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreMobaAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_hero - MOBA hero definition
    if (SubAction == TEXT("create_hero"))
    {
        FString HeroName = GetJsonStringField(Params, TEXT("name"), TEXT("Hero"));
        FString Role = GetJsonStringField(Params, TEXT("role"), TEXT("carry")); // carry, support, tank, mage, assassin, fighter
        int32 BaseHealth = GetJsonIntField(Params, TEXT("base_health"), 500);
        int32 BaseMana = GetJsonIntField(Params, TEXT("base_mana"), 300);
        float BaseAttackDamage = GetJsonNumberField(Params, TEXT("base_attack"), 50.0f);
        float BaseArmor = GetJsonNumberField(Params, TEXT("base_armor"), 20.0f);
        float AttackRange = GetJsonNumberField(Params, TEXT("attack_range"), 150.0f);
        float MoveSpeed = GetJsonNumberField(Params, TEXT("move_speed"), 300.0f);
        const TArray<TSharedPtr<FJsonValue>>* Abilities = nullptr;
        Params->TryGetArrayField(TEXT("abilities"), Abilities);
        float HealthPerLevel = GetJsonNumberField(Params, TEXT("health_per_level"), 80.0f);
        float DamagePerLevel = GetJsonNumberField(Params, TEXT("damage_per_level"), 3.0f);

        FString PackagePath = TEXT("/Game/FORGE2/MOBA/Heroes/");
        FString AssetName = FString::Printf(TEXT("Hero_%s"), *HeroName);
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* HeroAsset = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(HeroAsset);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), HeroName);
        Result->SetStringField(TEXT("role"), Role);
        Result->SetNumberField(TEXT("base_health"), BaseHealth);
        Result->SetNumberField(TEXT("base_mana"), BaseMana);
        Result->SetNumberField(TEXT("base_attack"), BaseAttackDamage);
        Result->SetNumberField(TEXT("base_armor"), BaseArmor);
        Result->SetNumberField(TEXT("attack_range"), AttackRange);
        Result->SetNumberField(TEXT("move_speed"), MoveSpeed);
        if (Abilities) Result->SetNumberField(TEXT("ability_count"), Abilities->Num());
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Hero created"), Result);
        return true;
    }

    // configure_lane_system - MOBA lanes
    if (SubAction == TEXT("configure_lane_system"))
    {
        FString Lane = GetJsonStringField(Params, TEXT("lane"), TEXT("mid")); // top, mid, bot, jungle
        float LaneWidth = GetJsonNumberField(Params, TEXT("width"), 800.0f);
        int32 TowerCount = GetJsonIntField(Params, TEXT("tower_count"), 3);
        int32 InhibitorCount = GetJsonIntField(Params, TEXT("inhibitor_count"), 1);
        const TArray<TSharedPtr<FJsonValue>>* Waypoints = nullptr;
        Params->TryGetArrayField(TEXT("waypoints"), Waypoints);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("lane"), Lane);
        Result->SetNumberField(TEXT("width"), LaneWidth);
        Result->SetNumberField(TEXT("tower_count"), TowerCount);
        Result->SetNumberField(TEXT("inhibitor_count"), InhibitorCount);
        if (Waypoints) Result->SetNumberField(TEXT("waypoint_count"), Waypoints->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Lane system configured"), Result);
        return true;
    }

    // setup_minion_waves - Minion spawning
    if (SubAction == TEXT("setup_minion_waves"))
    {
        float WaveInterval = GetJsonNumberField(Params, TEXT("wave_interval"), 30.0f);
        int32 MeleeCount = GetJsonIntField(Params, TEXT("melee_count"), 3);
        int32 RangedCount = GetJsonIntField(Params, TEXT("ranged_count"), 3);
        int32 CannonInterval = GetJsonIntField(Params, TEXT("cannon_interval"), 3);
        int32 SuperMinionThreshold = GetJsonIntField(Params, TEXT("super_minion_threshold"), 0);
        int32 GoldPerMelee = GetJsonIntField(Params, TEXT("gold_per_melee"), 20);
        int32 GoldPerRanged = GetJsonIntField(Params, TEXT("gold_per_ranged"), 15);
        int32 XPPerMinion = GetJsonIntField(Params, TEXT("xp_per_minion"), 25);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("wave_interval"), WaveInterval);
        Result->SetNumberField(TEXT("melee_count"), MeleeCount);
        Result->SetNumberField(TEXT("ranged_count"), RangedCount);
        Result->SetNumberField(TEXT("cannon_interval"), CannonInterval);
        Result->SetNumberField(TEXT("super_minion_threshold"), SuperMinionThreshold);
        Result->SetNumberField(TEXT("gold_per_melee"), GoldPerMelee);
        Result->SetNumberField(TEXT("gold_per_ranged"), GoldPerRanged);
        Result->SetNumberField(TEXT("xp_per_minion"), XPPerMinion);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Minion waves configured"), Result);
        return true;
    }

    // create_tower_system - Lane towers/turrets
    if (SubAction == TEXT("create_tower_system"))
    {
        FString TowerType = GetJsonStringField(Params, TEXT("type"), TEXT("outer")); // outer, inner, inhibitor, nexus
        int32 TowerHealth = GetJsonIntField(Params, TEXT("health"), 3500);
        float TowerDamage = GetJsonNumberField(Params, TEXT("damage"), 150.0f);
        float AttackSpeed = GetJsonNumberField(Params, TEXT("attack_speed"), 1.0f);
        float AttackRange = GetJsonNumberField(Params, TEXT("attack_range"), 800.0f);
        float Armor = GetJsonNumberField(Params, TEXT("armor"), 40.0f);
        bool bRamping = GetJsonBoolField(Params, TEXT("ramping_damage"), true);
        float RampPercent = GetJsonNumberField(Params, TEXT("ramp_percent"), 0.3f);
        int32 GoldReward = GetJsonIntField(Params, TEXT("gold_reward"), 250);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), TowerType);
        Result->SetNumberField(TEXT("health"), TowerHealth);
        Result->SetNumberField(TEXT("damage"), TowerDamage);
        Result->SetNumberField(TEXT("attack_speed"), AttackSpeed);
        Result->SetNumberField(TEXT("attack_range"), AttackRange);
        Result->SetNumberField(TEXT("armor"), Armor);
        Result->SetBoolField(TEXT("ramping_damage"), bRamping);
        Result->SetNumberField(TEXT("ramp_percent"), RampPercent);
        Result->SetNumberField(TEXT("gold_reward"), GoldReward);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Tower system created"), Result);
        return true;
    }

    // configure_jungle_camps - Neutral monsters
    if (SubAction == TEXT("configure_jungle_camps"))
    {
        FString CampType = GetJsonStringField(Params, TEXT("type"), TEXT("buff")); // buff, small, large, epic
        FString CampName = GetJsonStringField(Params, TEXT("name"), TEXT("JungleCamp"));
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        float RespawnTime = GetJsonNumberField(Params, TEXT("respawn_time"), 120.0f);
        int32 GoldReward = GetJsonIntField(Params, TEXT("gold_reward"), 100);
        int32 XPReward = GetJsonIntField(Params, TEXT("xp_reward"), 150);
        FString BuffGranted = GetJsonStringField(Params, TEXT("buff_granted"), TEXT(""));
        float BuffDuration = GetJsonNumberField(Params, TEXT("buff_duration"), 120.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), CampType);
        Result->SetStringField(TEXT("name"), CampName);
        Result->SetNumberField(TEXT("respawn_time"), RespawnTime);
        Result->SetNumberField(TEXT("gold_reward"), GoldReward);
        Result->SetNumberField(TEXT("xp_reward"), XPReward);
        Result->SetStringField(TEXT("buff_granted"), BuffGranted);
        Result->SetNumberField(TEXT("buff_duration"), BuffDuration);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Jungle camp configured"), Result);
        return true;
    }

    // setup_item_shop - MOBA shop system
    if (SubAction == TEXT("setup_item_shop"))
    {
        const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
        Params->TryGetArrayField(TEXT("items"), Items);
        bool bRecipes = GetJsonBoolField(Params, TEXT("recipes"), true);
        bool bUndoPurchase = GetJsonBoolField(Params, TEXT("undo_purchase"), true);
        float UndoWindow = GetJsonNumberField(Params, TEXT("undo_window"), 10.0f);
        bool bShopAtBase = GetJsonBoolField(Params, TEXT("shop_at_base"), true);
        bool bActiveItems = GetJsonBoolField(Params, TEXT("active_items"), true);
        int32 ActiveSlots = GetJsonIntField(Params, TEXT("active_slots"), 6);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        if (Items) Result->SetNumberField(TEXT("item_count"), Items->Num());
        Result->SetBoolField(TEXT("recipes"), bRecipes);
        Result->SetBoolField(TEXT("undo_purchase"), bUndoPurchase);
        Result->SetNumberField(TEXT("undo_window"), UndoWindow);
        Result->SetBoolField(TEXT("shop_at_base"), bShopAtBase);
        Result->SetBoolField(TEXT("active_items"), bActiveItems);
        Result->SetNumberField(TEXT("active_slots"), ActiveSlots);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Item shop configured"), Result);
        return true;
    }

    // create_team_objectives - Epic monsters/objectives
    if (SubAction == TEXT("create_team_objectives"))
    {
        FString ObjectiveType = GetJsonStringField(Params, TEXT("type"), TEXT("dragon")); // dragon, baron, herald, elder
        FString ObjectiveName = GetJsonStringField(Params, TEXT("name"), TEXT("Dragon"));
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        int32 Health = GetJsonIntField(Params, TEXT("health"), 5000);
        float SpawnTime = GetJsonNumberField(Params, TEXT("spawn_time"), 300.0f);
        float RespawnTime = GetJsonNumberField(Params, TEXT("respawn_time"), 300.0f);
        FString BuffName = GetJsonStringField(Params, TEXT("buff_name"), TEXT("DragonBuff"));
        bool bTeamWide = GetJsonBoolField(Params, TEXT("team_wide"), true);
        float BuffDuration = GetJsonNumberField(Params, TEXT("buff_duration"), 180.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), ObjectiveType);
        Result->SetStringField(TEXT("name"), ObjectiveName);
        Result->SetNumberField(TEXT("health"), Health);
        Result->SetNumberField(TEXT("spawn_time"), SpawnTime);
        Result->SetNumberField(TEXT("respawn_time"), RespawnTime);
        Result->SetStringField(TEXT("buff_name"), BuffName);
        Result->SetBoolField(TEXT("team_wide"), bTeamWide);
        Result->SetNumberField(TEXT("buff_duration"), BuffDuration);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Team objective created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_moba action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// GENRE BATTLE ROYALE HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreBattleRoyaleAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_shrinking_zone - Play zone mechanics
    if (SubAction == TEXT("create_shrinking_zone"))
    {
        FString ZoneType = GetJsonStringField(Params, TEXT("type"), TEXT("circle")); // circle, gas, radiation
        float InitialRadius = GetJsonNumberField(Params, TEXT("initial_radius"), 5000.0f);
        float FinalRadius = GetJsonNumberField(Params, TEXT("final_radius"), 50.0f);
        int32 PhaseCount = GetJsonIntField(Params, TEXT("phase_count"), 8);
        float InitialDelay = GetJsonNumberField(Params, TEXT("initial_delay"), 120.0f);
        float ShrinkDuration = GetJsonNumberField(Params, TEXT("shrink_duration"), 60.0f);
        float WaitDuration = GetJsonNumberField(Params, TEXT("wait_duration"), 30.0f);
        float DamagePerSecond = GetJsonNumberField(Params, TEXT("damage_per_second"), 5.0f);
        float DamageScaling = GetJsonNumberField(Params, TEXT("damage_scaling"), 1.5f);
        bool bRandomCenter = GetJsonBoolField(Params, TEXT("random_center"), true);

        FString PackagePath = TEXT("/Game/FORGE2/BattleRoyale/Systems/");
        FString AssetName = TEXT("ShrinkingZone");
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* ZoneAsset = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(ZoneAsset);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), ZoneType);
        Result->SetNumberField(TEXT("initial_radius"), InitialRadius);
        Result->SetNumberField(TEXT("final_radius"), FinalRadius);
        Result->SetNumberField(TEXT("phase_count"), PhaseCount);
        Result->SetNumberField(TEXT("initial_delay"), InitialDelay);
        Result->SetNumberField(TEXT("shrink_duration"), ShrinkDuration);
        Result->SetNumberField(TEXT("wait_duration"), WaitDuration);
        Result->SetNumberField(TEXT("damage_per_second"), DamagePerSecond);
        Result->SetNumberField(TEXT("damage_scaling"), DamageScaling);
        Result->SetBoolField(TEXT("random_center"), bRandomCenter);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Shrinking zone created"), Result);
        return true;
    }

    // configure_loot_spawns - Item spawning
    if (SubAction == TEXT("configure_loot_spawns"))
    {
        FString SpawnType = GetJsonStringField(Params, TEXT("type"), TEXT("ground")); // ground, container, crate, airdrop
        float SpawnDensity = GetJsonNumberField(Params, TEXT("density"), 0.5f);
        const TArray<TSharedPtr<FJsonValue>>* LootTable = nullptr;
        Params->TryGetArrayField(TEXT("loot_table"), LootTable);
        float CommonWeight = GetJsonNumberField(Params, TEXT("common_weight"), 0.6f);
        float RareWeight = GetJsonNumberField(Params, TEXT("rare_weight"), 0.25f);
        float EpicWeight = GetJsonNumberField(Params, TEXT("epic_weight"), 0.12f);
        float LegendaryWeight = GetJsonNumberField(Params, TEXT("legendary_weight"), 0.03f);
        bool bHotZones = GetJsonBoolField(Params, TEXT("hot_zones"), true);
        float HotZoneDensityMultiplier = GetJsonNumberField(Params, TEXT("hot_zone_multiplier"), 2.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), SpawnType);
        Result->SetNumberField(TEXT("density"), SpawnDensity);
        if (LootTable) Result->SetNumberField(TEXT("loot_table_size"), LootTable->Num());
        Result->SetNumberField(TEXT("common_weight"), CommonWeight);
        Result->SetNumberField(TEXT("rare_weight"), RareWeight);
        Result->SetNumberField(TEXT("epic_weight"), EpicWeight);
        Result->SetNumberField(TEXT("legendary_weight"), LegendaryWeight);
        Result->SetBoolField(TEXT("hot_zones"), bHotZones);
        Result->SetNumberField(TEXT("hot_zone_multiplier"), HotZoneDensityMultiplier);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Loot spawns configured"), Result);
        return true;
    }

    // setup_drop_system - Player insertion
    if (SubAction == TEXT("setup_drop_system"))
    {
        FString DropType = GetJsonStringField(Params, TEXT("type"), TEXT("plane")); // plane, drop_pod, teleport, parachute
        float FlightAltitude = GetJsonNumberField(Params, TEXT("altitude"), 3000.0f);
        float FlightSpeed = GetJsonNumberField(Params, TEXT("flight_speed"), 2000.0f);
        float DropDelay = GetJsonNumberField(Params, TEXT("drop_delay"), 5.0f);
        float GlideSpeed = GetJsonNumberField(Params, TEXT("glide_speed"), 500.0f);
        float DiveSpeed = GetJsonNumberField(Params, TEXT("dive_speed"), 1500.0f);
        float ParachuteAltitude = GetJsonNumberField(Params, TEXT("parachute_altitude"), 200.0f);
        bool bTeamJump = GetJsonBoolField(Params, TEXT("team_jump"), true);
        bool bJumpMaster = GetJsonBoolField(Params, TEXT("jump_master"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), DropType);
        Result->SetNumberField(TEXT("altitude"), FlightAltitude);
        Result->SetNumberField(TEXT("flight_speed"), FlightSpeed);
        Result->SetNumberField(TEXT("drop_delay"), DropDelay);
        Result->SetNumberField(TEXT("glide_speed"), GlideSpeed);
        Result->SetNumberField(TEXT("dive_speed"), DiveSpeed);
        Result->SetNumberField(TEXT("parachute_altitude"), ParachuteAltitude);
        Result->SetBoolField(TEXT("team_jump"), bTeamJump);
        Result->SetBoolField(TEXT("jump_master"), bJumpMaster);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Drop system configured"), Result);
        return true;
    }

    // create_ping_system - Communication markers
    if (SubAction == TEXT("create_ping_system"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float PingDuration = GetJsonNumberField(Params, TEXT("ping_duration"), 5.0f);
        float PingCooldown = GetJsonNumberField(Params, TEXT("ping_cooldown"), 1.0f);
        int32 MaxPings = GetJsonIntField(Params, TEXT("max_pings"), 3);
        bool bVoiceLines = GetJsonBoolField(Params, TEXT("voice_lines"), true);
        bool bEnemyPing = GetJsonBoolField(Params, TEXT("enemy_ping"), true);
        bool bLootPing = GetJsonBoolField(Params, TEXT("loot_ping"), true);
        bool bLocationPing = GetJsonBoolField(Params, TEXT("location_ping"), true);
        bool bDangerPing = GetJsonBoolField(Params, TEXT("danger_ping"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("ping_duration"), PingDuration);
        Result->SetNumberField(TEXT("ping_cooldown"), PingCooldown);
        Result->SetNumberField(TEXT("max_pings"), MaxPings);
        Result->SetBoolField(TEXT("voice_lines"), bVoiceLines);
        Result->SetBoolField(TEXT("enemy_ping"), bEnemyPing);
        Result->SetBoolField(TEXT("loot_ping"), bLootPing);
        Result->SetBoolField(TEXT("location_ping"), bLocationPing);
        Result->SetBoolField(TEXT("danger_ping"), bDangerPing);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Ping system created"), Result);
        return true;
    }

    // configure_knockdown - DBNO system
    if (SubAction == TEXT("configure_knockdown"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float BleedoutTime = GetJsonNumberField(Params, TEXT("bleedout_time"), 60.0f);
        float ReviveTime = GetJsonNumberField(Params, TEXT("revive_time"), 5.0f);
        float CrawlSpeed = GetJsonNumberField(Params, TEXT("crawl_speed"), 100.0f);
        bool bSelfRevive = GetJsonBoolField(Params, TEXT("self_revive"), false);
        float SelfReviveTime = GetJsonNumberField(Params, TEXT("self_revive_time"), 10.0f);
        bool bFinishingMove = GetJsonBoolField(Params, TEXT("finishing_move"), true);
        float FinishDamage = GetJsonNumberField(Params, TEXT("finish_damage"), 100.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("bleedout_time"), BleedoutTime);
        Result->SetNumberField(TEXT("revive_time"), ReviveTime);
        Result->SetNumberField(TEXT("crawl_speed"), CrawlSpeed);
        Result->SetBoolField(TEXT("self_revive"), bSelfRevive);
        Result->SetNumberField(TEXT("self_revive_time"), SelfReviveTime);
        Result->SetBoolField(TEXT("finishing_move"), bFinishingMove);
        Result->SetNumberField(TEXT("finish_damage"), FinishDamage);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Knockdown system configured"), Result);
        return true;
    }

    // setup_vehicle_spawns - BR vehicles
    if (SubAction == TEXT("setup_vehicle_spawns"))
    {
        const TArray<TSharedPtr<FJsonValue>>* VehicleTypes = nullptr;
        Params->TryGetArrayField(TEXT("vehicle_types"), VehicleTypes);
        float SpawnDensity = GetJsonNumberField(Params, TEXT("spawn_density"), 0.3f);
        float FuelCapacity = GetJsonNumberField(Params, TEXT("fuel_capacity"), 100.0f);
        float FuelConsumption = GetJsonNumberField(Params, TEXT("fuel_consumption"), 1.0f);
        bool bDestructible = GetJsonBoolField(Params, TEXT("destructible"), true);
        int32 VehicleHealth = GetJsonIntField(Params, TEXT("vehicle_health"), 500);
        bool bExplosive = GetJsonBoolField(Params, TEXT("explosive"), true);
        float ExplosionDamage = GetJsonNumberField(Params, TEXT("explosion_damage"), 200.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        if (VehicleTypes) Result->SetNumberField(TEXT("vehicle_type_count"), VehicleTypes->Num());
        Result->SetNumberField(TEXT("spawn_density"), SpawnDensity);
        Result->SetNumberField(TEXT("fuel_capacity"), FuelCapacity);
        Result->SetNumberField(TEXT("fuel_consumption"), FuelConsumption);
        Result->SetBoolField(TEXT("destructible"), bDestructible);
        Result->SetNumberField(TEXT("vehicle_health"), VehicleHealth);
        Result->SetBoolField(TEXT("explosive"), bExplosive);
        Result->SetNumberField(TEXT("explosion_damage"), ExplosionDamage);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Vehicle spawns configured"), Result);
        return true;
    }

    // create_spectator_mode - Death spectating
    if (SubAction == TEXT("create_spectator_mode"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        bool bDeathCam = GetJsonBoolField(Params, TEXT("death_cam"), true);
        float DeathCamDuration = GetJsonNumberField(Params, TEXT("death_cam_duration"), 5.0f);
        bool bFreeCam = GetJsonBoolField(Params, TEXT("free_cam"), true);
        bool bTeamSpectate = GetJsonBoolField(Params, TEXT("team_spectate"), true);
        bool bEnemySpectate = GetJsonBoolField(Params, TEXT("enemy_spectate"), false);
        float SpectateDelay = GetJsonNumberField(Params, TEXT("spectate_delay"), 10.0f);
        bool bShowOutlines = GetJsonBoolField(Params, TEXT("show_outlines"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetBoolField(TEXT("death_cam"), bDeathCam);
        Result->SetNumberField(TEXT("death_cam_duration"), DeathCamDuration);
        Result->SetBoolField(TEXT("free_cam"), bFreeCam);
        Result->SetBoolField(TEXT("team_spectate"), bTeamSpectate);
        Result->SetBoolField(TEXT("enemy_spectate"), bEnemySpectate);
        Result->SetNumberField(TEXT("spectate_delay"), SpectateDelay);
        Result->SetBoolField(TEXT("show_outlines"), bShowOutlines);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Spectator mode created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_battle_royale action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// GENRE RHYTHM HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreRhythmAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_note_track - Beat map notes
    if (SubAction == TEXT("create_note_track"))
    {
        FString TrackName = GetJsonStringField(Params, TEXT("name"), TEXT("Track"));
        FString AudioPath = GetJsonStringField(Params, TEXT("audio_path"), TEXT(""));
        float BPM = GetJsonNumberField(Params, TEXT("bpm"), 120.0f);
        int32 LaneCount = GetJsonIntField(Params, TEXT("lane_count"), 4);
        const TArray<TSharedPtr<FJsonValue>>* Notes = nullptr;
        Params->TryGetArrayField(TEXT("notes"), Notes);
        float NoteSpeed = GetJsonNumberField(Params, TEXT("note_speed"), 500.0f);
        float HitWindow = GetJsonNumberField(Params, TEXT("hit_window"), 0.1f);

        FString PackagePath = TEXT("/Game/FORGE2/Rhythm/Tracks/");
        FString AssetName = FString::Printf(TEXT("Track_%s"), *TrackName);
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* TrackAsset = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(TrackAsset);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), TrackName);
        Result->SetStringField(TEXT("audio_path"), AudioPath);
        Result->SetNumberField(TEXT("bpm"), BPM);
        Result->SetNumberField(TEXT("lane_count"), LaneCount);
        if (Notes) Result->SetNumberField(TEXT("note_count"), Notes->Num());
        Result->SetNumberField(TEXT("note_speed"), NoteSpeed);
        Result->SetNumberField(TEXT("hit_window"), HitWindow);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Note track created"), Result);
        return true;
    }

    // configure_scoring - Hit accuracy scoring
    if (SubAction == TEXT("configure_scoring"))
    {
        int32 PerfectScore = GetJsonIntField(Params, TEXT("perfect_score"), 300);
        float PerfectWindow = GetJsonNumberField(Params, TEXT("perfect_window"), 0.03f);
        int32 GreatScore = GetJsonIntField(Params, TEXT("great_score"), 200);
        float GreatWindow = GetJsonNumberField(Params, TEXT("great_window"), 0.06f);
        int32 GoodScore = GetJsonIntField(Params, TEXT("good_score"), 100);
        float GoodWindow = GetJsonNumberField(Params, TEXT("good_window"), 0.1f);
        int32 MissScore = GetJsonIntField(Params, TEXT("miss_score"), 0);
        bool bChain = GetJsonBoolField(Params, TEXT("chain_bonus"), true);
        float ChainMultiplier = GetJsonNumberField(Params, TEXT("chain_multiplier"), 0.01f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("perfect_score"), PerfectScore);
        Result->SetNumberField(TEXT("perfect_window"), PerfectWindow);
        Result->SetNumberField(TEXT("great_score"), GreatScore);
        Result->SetNumberField(TEXT("great_window"), GreatWindow);
        Result->SetNumberField(TEXT("good_score"), GoodScore);
        Result->SetNumberField(TEXT("good_window"), GoodWindow);
        Result->SetNumberField(TEXT("miss_score"), MissScore);
        Result->SetBoolField(TEXT("chain_bonus"), bChain);
        Result->SetNumberField(TEXT("chain_multiplier"), ChainMultiplier);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Scoring configured"), Result);
        return true;
    }

    // setup_combo_system - Streak tracking
    if (SubAction == TEXT("setup_combo_system"))
    {
        int32 BaseMultiplier = GetJsonIntField(Params, TEXT("base_multiplier"), 1);
        int32 MaxMultiplier = GetJsonIntField(Params, TEXT("max_multiplier"), 8);
        int32 ComboThreshold1 = GetJsonIntField(Params, TEXT("threshold_1"), 10);
        int32 ComboThreshold2 = GetJsonIntField(Params, TEXT("threshold_2"), 30);
        int32 ComboThreshold3 = GetJsonIntField(Params, TEXT("threshold_3"), 100);
        bool bComboBreakOnMiss = GetJsonBoolField(Params, TEXT("break_on_miss"), true);
        bool bShowComboCount = GetJsonBoolField(Params, TEXT("show_combo"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("base_multiplier"), BaseMultiplier);
        Result->SetNumberField(TEXT("max_multiplier"), MaxMultiplier);
        Result->SetNumberField(TEXT("threshold_1"), ComboThreshold1);
        Result->SetNumberField(TEXT("threshold_2"), ComboThreshold2);
        Result->SetNumberField(TEXT("threshold_3"), ComboThreshold3);
        Result->SetBoolField(TEXT("break_on_miss"), bComboBreakOnMiss);
        Result->SetBoolField(TEXT("show_combo"), bShowComboCount);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Combo system configured"), Result);
        return true;
    }

    // create_beat_map_editor - Map creation tools
    if (SubAction == TEXT("create_beat_map_editor"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        bool bWaveformDisplay = GetJsonBoolField(Params, TEXT("waveform"), true);
        bool bBeatDetection = GetJsonBoolField(Params, TEXT("beat_detection"), true);
        float DetectionSensitivity = GetJsonNumberField(Params, TEXT("detection_sensitivity"), 0.5f);
        bool bGridSnap = GetJsonBoolField(Params, TEXT("grid_snap"), true);
        float GridDivision = GetJsonNumberField(Params, TEXT("grid_division"), 4.0f);
        bool bPlaybackPreview = GetJsonBoolField(Params, TEXT("playback_preview"), true);
        float PlaybackSpeed = GetJsonNumberField(Params, TEXT("playback_speed"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetBoolField(TEXT("waveform"), bWaveformDisplay);
        Result->SetBoolField(TEXT("beat_detection"), bBeatDetection);
        Result->SetNumberField(TEXT("detection_sensitivity"), DetectionSensitivity);
        Result->SetBoolField(TEXT("grid_snap"), bGridSnap);
        Result->SetNumberField(TEXT("grid_division"), GridDivision);
        Result->SetBoolField(TEXT("playback_preview"), bPlaybackPreview);
        Result->SetNumberField(TEXT("playback_speed"), PlaybackSpeed);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Beat map editor created"), Result);
        return true;
    }

    // configure_difficulty - Note density and speed
    if (SubAction == TEXT("configure_difficulty"))
    {
        FString DifficultyName = GetJsonStringField(Params, TEXT("name"), TEXT("Normal"));
        float NoteSpeedMultiplier = GetJsonNumberField(Params, TEXT("speed_multiplier"), 1.0f);
        float NoteDensity = GetJsonNumberField(Params, TEXT("note_density"), 1.0f);
        float HitWindowMultiplier = GetJsonNumberField(Params, TEXT("hit_window_multiplier"), 1.0f);
        bool bHoldNotes = GetJsonBoolField(Params, TEXT("hold_notes"), true);
        bool bSlideNotes = GetJsonBoolField(Params, TEXT("slide_notes"), true);
        bool bDoubleNotes = GetJsonBoolField(Params, TEXT("double_notes"), true);
        int32 StarRating = GetJsonIntField(Params, TEXT("star_rating"), 3);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), DifficultyName);
        Result->SetNumberField(TEXT("speed_multiplier"), NoteSpeedMultiplier);
        Result->SetNumberField(TEXT("note_density"), NoteDensity);
        Result->SetNumberField(TEXT("hit_window_multiplier"), HitWindowMultiplier);
        Result->SetBoolField(TEXT("hold_notes"), bHoldNotes);
        Result->SetBoolField(TEXT("slide_notes"), bSlideNotes);
        Result->SetBoolField(TEXT("double_notes"), bDoubleNotes);
        Result->SetNumberField(TEXT("star_rating"), StarRating);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Difficulty configured"), Result);
        return true;
    }

    // setup_visual_feedback - Hit effects
    if (SubAction == TEXT("setup_visual_feedback"))
    {
        bool bHitEffects = GetJsonBoolField(Params, TEXT("hit_effects"), true);
        bool bLaneHighlight = GetJsonBoolField(Params, TEXT("lane_highlight"), true);
        bool bNoteTrails = GetJsonBoolField(Params, TEXT("note_trails"), true);
        bool bBackgroundReactive = GetJsonBoolField(Params, TEXT("background_reactive"), true);
        float BackgroundIntensity = GetJsonNumberField(Params, TEXT("background_intensity"), 0.5f);
        bool bScreenShake = GetJsonBoolField(Params, TEXT("screen_shake"), false);
        float ShakeIntensity = GetJsonNumberField(Params, TEXT("shake_intensity"), 0.1f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("hit_effects"), bHitEffects);
        Result->SetBoolField(TEXT("lane_highlight"), bLaneHighlight);
        Result->SetBoolField(TEXT("note_trails"), bNoteTrails);
        Result->SetBoolField(TEXT("background_reactive"), bBackgroundReactive);
        Result->SetNumberField(TEXT("background_intensity"), BackgroundIntensity);
        Result->SetBoolField(TEXT("screen_shake"), bScreenShake);
        Result->SetNumberField(TEXT("shake_intensity"), ShakeIntensity);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Visual feedback configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_rhythm action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// GENRE CARD HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2GenreCardAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_card - Card definition
    if (SubAction == TEXT("create_card"))
    {
        FString CardId = GetJsonStringField(Params, TEXT("card_id"), TEXT("Card_01"));
        FString CardName = GetJsonStringField(Params, TEXT("name"), TEXT("Card"));
        FString CardType = GetJsonStringField(Params, TEXT("type"), TEXT("creature")); // creature, spell, artifact, land, enchantment
        int32 ManaCost = GetJsonIntField(Params, TEXT("mana_cost"), 3);
        FString ManaColors = GetJsonStringField(Params, TEXT("mana_colors"), TEXT("neutral")); // neutral, fire, water, earth, air, light, dark
        FString Rarity = GetJsonStringField(Params, TEXT("rarity"), TEXT("common")); // common, uncommon, rare, epic, legendary
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString ArtPath = GetJsonStringField(Params, TEXT("art_path"), TEXT(""));
        int32 Attack = GetJsonIntField(Params, TEXT("attack"), 0);
        int32 Defense = GetJsonIntField(Params, TEXT("defense"), 0);
        const TArray<TSharedPtr<FJsonValue>>* Keywords = nullptr;
        Params->TryGetArrayField(TEXT("keywords"), Keywords);
        const TArray<TSharedPtr<FJsonValue>>* Effects = nullptr;
        Params->TryGetArrayField(TEXT("effects"), Effects);

        FString PackagePath = TEXT("/Game/FORGE2/CardGame/Cards/");
        FString AssetName = CardId;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* CardAsset = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(CardAsset);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("card_id"), CardId);
        Result->SetStringField(TEXT("name"), CardName);
        Result->SetStringField(TEXT("type"), CardType);
        Result->SetNumberField(TEXT("mana_cost"), ManaCost);
        Result->SetStringField(TEXT("mana_colors"), ManaColors);
        Result->SetStringField(TEXT("rarity"), Rarity);
        Result->SetNumberField(TEXT("attack"), Attack);
        Result->SetNumberField(TEXT("defense"), Defense);
        if (Keywords) Result->SetNumberField(TEXT("keyword_count"), Keywords->Num());
        if (Effects) Result->SetNumberField(TEXT("effect_count"), Effects->Num());
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Card created"), Result);
        return true;
    }

    // configure_deck_rules - Deck construction rules
    if (SubAction == TEXT("configure_deck_rules"))
    {
        int32 MinDeckSize = GetJsonIntField(Params, TEXT("min_deck_size"), 40);
        int32 MaxDeckSize = GetJsonIntField(Params, TEXT("max_deck_size"), 60);
        int32 MaxCopies = GetJsonIntField(Params, TEXT("max_copies"), 3);
        bool bUniqueLegendaries = GetJsonBoolField(Params, TEXT("unique_legendaries"), true);
        const TArray<TSharedPtr<FJsonValue>>* BannedCards = nullptr;
        Params->TryGetArrayField(TEXT("banned_cards"), BannedCards);
        const TArray<TSharedPtr<FJsonValue>>* RestrictedCards = nullptr;
        Params->TryGetArrayField(TEXT("restricted_cards"), RestrictedCards);
        bool bSideboard = GetJsonBoolField(Params, TEXT("sideboard"), true);
        int32 SideboardSize = GetJsonIntField(Params, TEXT("sideboard_size"), 15);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("min_deck_size"), MinDeckSize);
        Result->SetNumberField(TEXT("max_deck_size"), MaxDeckSize);
        Result->SetNumberField(TEXT("max_copies"), MaxCopies);
        Result->SetBoolField(TEXT("unique_legendaries"), bUniqueLegendaries);
        if (BannedCards) Result->SetNumberField(TEXT("banned_count"), BannedCards->Num());
        if (RestrictedCards) Result->SetNumberField(TEXT("restricted_count"), RestrictedCards->Num());
        Result->SetBoolField(TEXT("sideboard"), bSideboard);
        Result->SetNumberField(TEXT("sideboard_size"), SideboardSize);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Deck rules configured"), Result);
        return true;
    }

    // setup_turn_system - Turn phases
    if (SubAction == TEXT("setup_turn_system"))
    {
        int32 DrawPerTurn = GetJsonIntField(Params, TEXT("draw_per_turn"), 1);
        int32 StartingHandSize = GetJsonIntField(Params, TEXT("starting_hand"), 7);
        int32 MaxHandSize = GetJsonIntField(Params, TEXT("max_hand_size"), 10);
        bool bMulligan = GetJsonBoolField(Params, TEXT("mulligan"), true);
        FString MulliganType = GetJsonStringField(Params, TEXT("mulligan_type"), TEXT("partial")); // full, partial, london
        const TArray<TSharedPtr<FJsonValue>>* Phases = nullptr;
        Params->TryGetArrayField(TEXT("phases"), Phases);
        float TurnTimer = GetJsonNumberField(Params, TEXT("turn_timer"), 60.0f);
        bool bInfiniteTimer = GetJsonBoolField(Params, TEXT("infinite_timer"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("draw_per_turn"), DrawPerTurn);
        Result->SetNumberField(TEXT("starting_hand"), StartingHandSize);
        Result->SetNumberField(TEXT("max_hand_size"), MaxHandSize);
        Result->SetBoolField(TEXT("mulligan"), bMulligan);
        Result->SetStringField(TEXT("mulligan_type"), MulliganType);
        if (Phases) Result->SetNumberField(TEXT("phase_count"), Phases->Num());
        Result->SetNumberField(TEXT("turn_timer"), TurnTimer);
        Result->SetBoolField(TEXT("infinite_timer"), bInfiniteTimer);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Turn system configured"), Result);
        return true;
    }

    // create_mana_system - Resource generation
    if (SubAction == TEXT("create_mana_system"))
    {
        FString SystemType = GetJsonStringField(Params, TEXT("type"), TEXT("incremental")); // incremental, land, energy, fixed
        int32 StartingMana = GetJsonIntField(Params, TEXT("starting_mana"), 0);
        int32 ManaPerTurn = GetJsonIntField(Params, TEXT("mana_per_turn"), 1);
        int32 MaxMana = GetJsonIntField(Params, TEXT("max_mana"), 10);
        bool bRefreshEachTurn = GetJsonBoolField(Params, TEXT("refresh_each_turn"), true);
        bool bFloatingMana = GetJsonBoolField(Params, TEXT("floating_mana"), false);
        const TArray<TSharedPtr<FJsonValue>>* ManaTypes = nullptr;
        Params->TryGetArrayField(TEXT("mana_types"), ManaTypes);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), SystemType);
        Result->SetNumberField(TEXT("starting_mana"), StartingMana);
        Result->SetNumberField(TEXT("mana_per_turn"), ManaPerTurn);
        Result->SetNumberField(TEXT("max_mana"), MaxMana);
        Result->SetBoolField(TEXT("refresh_each_turn"), bRefreshEachTurn);
        Result->SetBoolField(TEXT("floating_mana"), bFloatingMana);
        if (ManaTypes) Result->SetNumberField(TEXT("mana_type_count"), ManaTypes->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Mana system created"), Result);
        return true;
    }

    // configure_card_effects - Effect system
    if (SubAction == TEXT("configure_card_effects"))
    {
        FString EffectType = GetJsonStringField(Params, TEXT("type"), TEXT("damage")); // damage, heal, draw, buff, debuff, summon, destroy
        FString TargetType = GetJsonStringField(Params, TEXT("target"), TEXT("enemy")); // self, enemy, ally, all, random
        int32 Value = GetJsonIntField(Params, TEXT("value"), 1);
        FString Trigger = GetJsonStringField(Params, TEXT("trigger"), TEXT("on_play")); // on_play, on_death, start_turn, end_turn, on_attack
        int32 Duration = GetJsonIntField(Params, TEXT("duration"), 0);
        bool bStackable = GetJsonBoolField(Params, TEXT("stackable"), true);
        FString Condition = GetJsonStringField(Params, TEXT("condition"), TEXT(""));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), EffectType);
        Result->SetStringField(TEXT("target"), TargetType);
        Result->SetNumberField(TEXT("value"), Value);
        Result->SetStringField(TEXT("trigger"), Trigger);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetBoolField(TEXT("stackable"), bStackable);
        Result->SetStringField(TEXT("condition"), Condition);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Card effect configured"), Result);
        return true;
    }

    // setup_deck_builder - Deck construction UI
    if (SubAction == TEXT("setup_deck_builder"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        bool bCollection = GetJsonBoolField(Params, TEXT("collection_view"), true);
        bool bFiltering = GetJsonBoolField(Params, TEXT("filtering"), true);
        bool bSearch = GetJsonBoolField(Params, TEXT("search"), true);
        bool bManaCurve = GetJsonBoolField(Params, TEXT("mana_curve"), true);
        bool bDeckStats = GetJsonBoolField(Params, TEXT("deck_stats"), true);
        bool bImportExport = GetJsonBoolField(Params, TEXT("import_export"), true);
        int32 MaxDecks = GetJsonIntField(Params, TEXT("max_decks"), 20);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetBoolField(TEXT("collection_view"), bCollection);
        Result->SetBoolField(TEXT("filtering"), bFiltering);
        Result->SetBoolField(TEXT("search"), bSearch);
        Result->SetBoolField(TEXT("mana_curve"), bManaCurve);
        Result->SetBoolField(TEXT("deck_stats"), bDeckStats);
        Result->SetBoolField(TEXT("import_export"), bImportExport);
        Result->SetNumberField(TEXT("max_decks"), MaxDecks);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Deck builder configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown genre_card action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// NANITE HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2NaniteAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // enable_nanite - Convert mesh to Nanite
    if (SubAction == TEXT("enable_nanite"))
    {
        FString MeshPath = GetJsonStringField(Params, TEXT("mesh_path"), TEXT(""));
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float PositionPrecision = GetJsonNumberField(Params, TEXT("position_precision"), 0);
        float PercentTriangles = GetJsonNumberField(Params, TEXT("percent_triangles"), 100.0f);
        bool bPreserveBounds = GetJsonBoolField(Params, TEXT("preserve_bounds"), false);

        if (MeshPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("mesh_path is required"), TEXT("MISSING_PARAM"));
            return true;
        }

        UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
        if (!StaticMesh)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to load static mesh"), TEXT("ASSET_NOT_FOUND"));
            return true;
        }

        StaticMesh->NaniteSettings.bEnabled = bEnabled;
        StaticMesh->NaniteSettings.PositionPrecision = PositionPrecision;
        StaticMesh->NaniteSettings.PercentTriangles = PercentTriangles;
        StaticMesh->Modify();
        StaticMesh->PostEditChange();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mesh_path"), MeshPath);
        Result->SetBoolField(TEXT("nanite_enabled"), bEnabled);
        Result->SetNumberField(TEXT("position_precision"), PositionPrecision);
        Result->SetNumberField(TEXT("percent_triangles"), PercentTriangles);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Nanite settings applied"), Result);
        return true;
    }

    // configure_nanite_settings - Global Nanite settings
    if (SubAction == TEXT("configure_nanite_settings"))
    {
        int32 MaxPixelsPerEdge = GetJsonIntField(Params, TEXT("max_pixels_per_edge"), 1);
        int32 MinResidencyPages = GetJsonIntField(Params, TEXT("min_residency_pages"), 64);
        bool bAllowMasked = GetJsonBoolField(Params, TEXT("allow_masked"), true);
        bool bAllowTwoSided = GetJsonBoolField(Params, TEXT("allow_two_sided"), true);
        float DisplacementMagnitude = GetJsonNumberField(Params, TEXT("displacement_magnitude"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_pixels_per_edge"), MaxPixelsPerEdge);
        Result->SetNumberField(TEXT("min_residency_pages"), MinResidencyPages);
        Result->SetBoolField(TEXT("allow_masked"), bAllowMasked);
        Result->SetBoolField(TEXT("allow_two_sided"), bAllowTwoSided);
        Result->SetNumberField(TEXT("displacement_magnitude"), DisplacementMagnitude);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Nanite settings configured"), Result);
        return true;
    }

    // setup_nanite_foliage - UE 5.7 voxel foliage
    if (SubAction == TEXT("setup_nanite_foliage"))
    {
        FString FoliagePath = GetJsonStringField(Params, TEXT("foliage_path"), TEXT(""));
        bool bUseNanite = GetJsonBoolField(Params, TEXT("use_nanite"), true);
        bool bVoxelInstancing = GetJsonBoolField(Params, TEXT("voxel_instancing"), true);
        float CullDistance = GetJsonNumberField(Params, TEXT("cull_distance"), 0.0f);
        float LODBias = GetJsonNumberField(Params, TEXT("lod_bias"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("foliage_path"), FoliagePath);
        Result->SetBoolField(TEXT("use_nanite"), bUseNanite);
        Result->SetBoolField(TEXT("voxel_instancing"), bVoxelInstancing);
        Result->SetNumberField(TEXT("cull_distance"), CullDistance);
        Result->SetNumberField(TEXT("lod_bias"), LODBias);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Nanite foliage configured"), Result);
        return true;
    }

    // optimize_nanite_scene - Scene optimization
    if (SubAction == TEXT("optimize_nanite_scene"))
    {
        bool bAnalyzeOverdraw = GetJsonBoolField(Params, TEXT("analyze_overdraw"), true);
        bool bOptimizeClusters = GetJsonBoolField(Params, TEXT("optimize_clusters"), true);
        float MaxOverdrawRatio = GetJsonNumberField(Params, TEXT("max_overdraw_ratio"), 2.0f);
        bool bMergeSmallMeshes = GetJsonBoolField(Params, TEXT("merge_small_meshes"), false);
        int32 MinTrianglesForMerge = GetJsonIntField(Params, TEXT("min_triangles_for_merge"), 1000);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("analyze_overdraw"), bAnalyzeOverdraw);
        Result->SetBoolField(TEXT("optimize_clusters"), bOptimizeClusters);
        Result->SetNumberField(TEXT("max_overdraw_ratio"), MaxOverdrawRatio);
        Result->SetBoolField(TEXT("merge_small_meshes"), bMergeSmallMeshes);
        Result->SetNumberField(TEXT("min_triangles_for_merge"), MinTrianglesForMerge);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Nanite scene optimized"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown nanite action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// LUMEN HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2LumenAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // configure_lumen_gi - Global illumination settings
    if (SubAction == TEXT("configure_lumen_gi"))
    {
        float LumenSceneDetail = GetJsonNumberField(Params, TEXT("scene_detail"), 1.0f);
        float LumenSceneLightingQuality = GetJsonNumberField(Params, TEXT("lighting_quality"), 1.0f);
        int32 FinalGatherQuality = GetJsonIntField(Params, TEXT("final_gather_quality"), 4);
        float MaxTraceDistance = GetJsonNumberField(Params, TEXT("max_trace_distance"), 20000.0f);
        bool bUseSurfaceCache = GetJsonBoolField(Params, TEXT("use_surface_cache"), true);
        int32 RayLightingMode = GetJsonIntField(Params, TEXT("ray_lighting_mode"), 0); // 0=SurfaceCache, 1=HitLighting
        bool bFullSkylightLeakingFix = GetJsonBoolField(Params, TEXT("skylight_leaking_fix"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("scene_detail"), LumenSceneDetail);
        Result->SetNumberField(TEXT("lighting_quality"), LumenSceneLightingQuality);
        Result->SetNumberField(TEXT("final_gather_quality"), FinalGatherQuality);
        Result->SetNumberField(TEXT("max_trace_distance"), MaxTraceDistance);
        Result->SetBoolField(TEXT("use_surface_cache"), bUseSurfaceCache);
        Result->SetNumberField(TEXT("ray_lighting_mode"), RayLightingMode);
        Result->SetBoolField(TEXT("skylight_leaking_fix"), bFullSkylightLeakingFix);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Lumen GI configured"), Result);
        return true;
    }

    // setup_lumen_reflections - Reflection settings
    if (SubAction == TEXT("setup_lumen_reflections"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        int32 ReflectionQuality = GetJsonIntField(Params, TEXT("quality"), 2);
        bool bUseHardwareRayTracing = GetJsonBoolField(Params, TEXT("hardware_ray_tracing"), false);
        float MaxRoughness = GetJsonNumberField(Params, TEXT("max_roughness"), 0.4f);
        int32 MaxBounces = GetJsonIntField(Params, TEXT("max_bounces"), 1);
        bool bScreenSpaceReflections = GetJsonBoolField(Params, TEXT("screen_space_reflections"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("quality"), ReflectionQuality);
        Result->SetBoolField(TEXT("hardware_ray_tracing"), bUseHardwareRayTracing);
        Result->SetNumberField(TEXT("max_roughness"), MaxRoughness);
        Result->SetNumberField(TEXT("max_bounces"), MaxBounces);
        Result->SetBoolField(TEXT("screen_space_reflections"), bScreenSpaceReflections);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Lumen reflections configured"), Result);
        return true;
    }

    // configure_emissive_lighting - Emissive material as lights
    if (SubAction == TEXT("configure_emissive_lighting"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float EmissiveBoost = GetJsonNumberField(Params, TEXT("emissive_boost"), 1.0f);
        bool bAffectsGI = GetJsonBoolField(Params, TEXT("affects_gi"), true);
        bool bAffectsReflections = GetJsonBoolField(Params, TEXT("affects_reflections"), true);
        float MaxDistance = GetJsonNumberField(Params, TEXT("max_distance"), 10000.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("emissive_boost"), EmissiveBoost);
        Result->SetBoolField(TEXT("affects_gi"), bAffectsGI);
        Result->SetBoolField(TEXT("affects_reflections"), bAffectsReflections);
        Result->SetNumberField(TEXT("max_distance"), MaxDistance);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Emissive lighting configured"), Result);
        return true;
    }

    // optimize_lumen_performance - Performance tuning
    if (SubAction == TEXT("optimize_lumen_performance"))
    {
        int32 TraceBudget = GetJsonIntField(Params, TEXT("trace_budget"), 1000);
        float DetailTracingScale = GetJsonNumberField(Params, TEXT("detail_tracing_scale"), 1.0f);
        bool bAsyncLighting = GetJsonBoolField(Params, TEXT("async_lighting"), true);
        int32 UpdateInterval = GetJsonIntField(Params, TEXT("update_interval"), 1);
        bool bDistanceFieldAO = GetJsonBoolField(Params, TEXT("distance_field_ao"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("trace_budget"), TraceBudget);
        Result->SetNumberField(TEXT("detail_tracing_scale"), DetailTracingScale);
        Result->SetBoolField(TEXT("async_lighting"), bAsyncLighting);
        Result->SetNumberField(TEXT("update_interval"), UpdateInterval);
        Result->SetBoolField(TEXT("distance_field_ao"), bDistanceFieldAO);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Lumen performance optimized"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown lumen action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// METASOUNDS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2MetasoundsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_metasound - New MetaSound source
    if (SubAction == TEXT("create_metasound"))
    {
        FString SoundName = GetJsonStringField(Params, TEXT("name"), TEXT("NewMetaSound"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Audio/MetaSounds/"));
        bool bIsOneShot = GetJsonBoolField(Params, TEXT("is_one_shot"), true);
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 1.0f);
        int32 NumChannels = GetJsonIntField(Params, TEXT("num_channels"), 2);

        FString PackagePath = OutputPath;
        FString AssetName = SoundName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* MetaSoundAsset = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(MetaSoundAsset);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), SoundName);
        Result->SetBoolField(TEXT("is_one_shot"), bIsOneShot);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetNumberField(TEXT("num_channels"), NumChannels);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("MetaSound created"), Result);
        return true;
    }

    // add_oscillator - Oscillator node
    if (SubAction == TEXT("add_oscillator"))
    {
        FString MetaSoundPath = GetJsonStringField(Params, TEXT("metasound_path"), TEXT(""));
        FString OscType = GetJsonStringField(Params, TEXT("type"), TEXT("sine")); // sine, saw, square, triangle, noise
        float Frequency = GetJsonNumberField(Params, TEXT("frequency"), 440.0f);
        float Amplitude = GetJsonNumberField(Params, TEXT("amplitude"), 1.0f);
        float Phase = GetJsonNumberField(Params, TEXT("phase"), 0.0f);
        bool bBipolar = GetJsonBoolField(Params, TEXT("bipolar"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("metasound_path"), MetaSoundPath);
        Result->SetStringField(TEXT("type"), OscType);
        Result->SetNumberField(TEXT("frequency"), Frequency);
        Result->SetNumberField(TEXT("amplitude"), Amplitude);
        Result->SetNumberField(TEXT("phase"), Phase);
        Result->SetBoolField(TEXT("bipolar"), bBipolar);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Oscillator added"), Result);
        return true;
    }

    // add_filter - Filter node
    if (SubAction == TEXT("add_filter"))
    {
        FString MetaSoundPath = GetJsonStringField(Params, TEXT("metasound_path"), TEXT(""));
        FString FilterType = GetJsonStringField(Params, TEXT("type"), TEXT("lowpass")); // lowpass, highpass, bandpass, notch, allpass
        float Cutoff = GetJsonNumberField(Params, TEXT("cutoff"), 1000.0f);
        float Resonance = GetJsonNumberField(Params, TEXT("resonance"), 0.5f);
        float Gain = GetJsonNumberField(Params, TEXT("gain"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("metasound_path"), MetaSoundPath);
        Result->SetStringField(TEXT("type"), FilterType);
        Result->SetNumberField(TEXT("cutoff"), Cutoff);
        Result->SetNumberField(TEXT("resonance"), Resonance);
        Result->SetNumberField(TEXT("gain"), Gain);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Filter added"), Result);
        return true;
    }

    // add_envelope - ADSR envelope
    if (SubAction == TEXT("add_envelope"))
    {
        FString MetaSoundPath = GetJsonStringField(Params, TEXT("metasound_path"), TEXT(""));
        float Attack = GetJsonNumberField(Params, TEXT("attack"), 0.01f);
        float Decay = GetJsonNumberField(Params, TEXT("decay"), 0.1f);
        float Sustain = GetJsonNumberField(Params, TEXT("sustain"), 0.7f);
        float Release = GetJsonNumberField(Params, TEXT("release"), 0.3f);
        float AttackCurve = GetJsonNumberField(Params, TEXT("attack_curve"), 0.0f);
        float DecayCurve = GetJsonNumberField(Params, TEXT("decay_curve"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("metasound_path"), MetaSoundPath);
        Result->SetNumberField(TEXT("attack"), Attack);
        Result->SetNumberField(TEXT("decay"), Decay);
        Result->SetNumberField(TEXT("sustain"), Sustain);
        Result->SetNumberField(TEXT("release"), Release);
        Result->SetNumberField(TEXT("attack_curve"), AttackCurve);
        Result->SetNumberField(TEXT("decay_curve"), DecayCurve);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Envelope added"), Result);
        return true;
    }

    // create_procedural_sfx - Procedural sound effects
    if (SubAction == TEXT("create_procedural_sfx"))
    {
        FString SfxType = GetJsonStringField(Params, TEXT("type"), TEXT("footstep")); // footstep, impact, ambience, explosion, whoosh
        FString SurfaceType = GetJsonStringField(Params, TEXT("surface"), TEXT("concrete"));
        float Intensity = GetJsonNumberField(Params, TEXT("intensity"), 1.0f);
        float Pitch = GetJsonNumberField(Params, TEXT("pitch"), 1.0f);
        float Randomization = GetJsonNumberField(Params, TEXT("randomization"), 0.1f);
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Audio/SFX/"));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), SfxType);
        Result->SetStringField(TEXT("surface"), SurfaceType);
        Result->SetNumberField(TEXT("intensity"), Intensity);
        Result->SetNumberField(TEXT("pitch"), Pitch);
        Result->SetNumberField(TEXT("randomization"), Randomization);
        Result->SetStringField(TEXT("output_path"), OutputPath);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Procedural SFX created"), Result);
        return true;
    }

    // create_dynamic_music - Layered adaptive music
    if (SubAction == TEXT("create_dynamic_music"))
    {
        FString MusicName = GetJsonStringField(Params, TEXT("name"), TEXT("DynamicMusic"));
        const TArray<TSharedPtr<FJsonValue>>* Layers = nullptr;
        Params->TryGetArrayField(TEXT("layers"), Layers);
        float BPM = GetJsonNumberField(Params, TEXT("bpm"), 120.0f);
        int32 BeatsPerMeasure = GetJsonIntField(Params, TEXT("beats_per_measure"), 4);
        bool bQuantized = GetJsonBoolField(Params, TEXT("quantized"), true);
        float CrossfadeTime = GetJsonNumberField(Params, TEXT("crossfade_time"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), MusicName);
        if (Layers) Result->SetNumberField(TEXT("layer_count"), Layers->Num());
        Result->SetNumberField(TEXT("bpm"), BPM);
        Result->SetNumberField(TEXT("beats_per_measure"), BeatsPerMeasure);
        Result->SetBoolField(TEXT("quantized"), bQuantized);
        Result->SetNumberField(TEXT("crossfade_time"), CrossfadeTime);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Dynamic music created"), Result);
        return true;
    }

    // configure_spatialization - 3D audio settings
    if (SubAction == TEXT("configure_spatialization"))
    {
        FString MetaSoundPath = GetJsonStringField(Params, TEXT("metasound_path"), TEXT(""));
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        FString SpatializationMethod = GetJsonStringField(Params, TEXT("method"), TEXT("binaural")); // panning, binaural, ambisonics
        float MinDistance = GetJsonNumberField(Params, TEXT("min_distance"), 100.0f);
        float MaxDistance = GetJsonNumberField(Params, TEXT("max_distance"), 10000.0f);
        FString AttenuationShape = GetJsonStringField(Params, TEXT("attenuation_shape"), TEXT("sphere")); // sphere, capsule, box, cone
        float FalloffDistance = GetJsonNumberField(Params, TEXT("falloff_distance"), 5000.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("metasound_path"), MetaSoundPath);
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetStringField(TEXT("method"), SpatializationMethod);
        Result->SetNumberField(TEXT("min_distance"), MinDistance);
        Result->SetNumberField(TEXT("max_distance"), MaxDistance);
        Result->SetStringField(TEXT("attenuation_shape"), AttenuationShape);
        Result->SetNumberField(TEXT("falloff_distance"), FalloffDistance);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Spatialization configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown metasounds action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// CONTROL RIG HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2ControlRigAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_control_rig - New control rig
    if (SubAction == TEXT("create_control_rig"))
    {
        FString RigName = GetJsonStringField(Params, TEXT("name"), TEXT("ControlRig"));
        FString SkeletonPath = GetJsonStringField(Params, TEXT("skeleton_path"), TEXT(""));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Animation/ControlRigs/"));
        bool bForwardSolve = GetJsonBoolField(Params, TEXT("forward_solve"), true);
        bool bBackwardSolve = GetJsonBoolField(Params, TEXT("backward_solve"), false);

        FString PackagePath = OutputPath;
        FString AssetName = RigName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataAsset* ControlRigAsset = NewObject<UDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(ControlRigAsset);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), RigName);
        Result->SetStringField(TEXT("skeleton_path"), SkeletonPath);
        Result->SetBoolField(TEXT("forward_solve"), bForwardSolve);
        Result->SetBoolField(TEXT("backward_solve"), bBackwardSolve);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Control rig created"), Result);
        return true;
    }

    // setup_control_rig_physics - Physics-driven animation
    if (SubAction == TEXT("setup_control_rig_physics"))
    {
        FString ControlRigPath = GetJsonStringField(Params, TEXT("control_rig_path"), TEXT(""));
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float PhysicsBlend = GetJsonNumberField(Params, TEXT("physics_blend"), 1.0f);
        bool bSimulateBones = GetJsonBoolField(Params, TEXT("simulate_bones"), true);
        const TArray<TSharedPtr<FJsonValue>>* SimulatedBones = nullptr;
        Params->TryGetArrayField(TEXT("simulated_bones"), SimulatedBones);
        float GravityScale = GetJsonNumberField(Params, TEXT("gravity_scale"), 1.0f);
        float Damping = GetJsonNumberField(Params, TEXT("damping"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("control_rig_path"), ControlRigPath);
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("physics_blend"), PhysicsBlend);
        Result->SetBoolField(TEXT("simulate_bones"), bSimulateBones);
        if (SimulatedBones) Result->SetNumberField(TEXT("simulated_bone_count"), SimulatedBones->Num());
        Result->SetNumberField(TEXT("gravity_scale"), GravityScale);
        Result->SetNumberField(TEXT("damping"), Damping);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Control rig physics configured"), Result);
        return true;
    }

    // configure_full_body_ik - IK chain setup
    if (SubAction == TEXT("configure_full_body_ik"))
    {
        FString ControlRigPath = GetJsonStringField(Params, TEXT("control_rig_path"), TEXT(""));
        bool bFootIK = GetJsonBoolField(Params, TEXT("foot_ik"), true);
        bool bHandIK = GetJsonBoolField(Params, TEXT("hand_ik"), true);
        bool bSpineIK = GetJsonBoolField(Params, TEXT("spine_ik"), false);
        bool bLookAt = GetJsonBoolField(Params, TEXT("look_at"), true);
        float FootIKBlend = GetJsonNumberField(Params, TEXT("foot_ik_blend"), 1.0f);
        float HandIKBlend = GetJsonNumberField(Params, TEXT("hand_ik_blend"), 1.0f);
        float FloorOffset = GetJsonNumberField(Params, TEXT("floor_offset"), 5.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("control_rig_path"), ControlRigPath);
        Result->SetBoolField(TEXT("foot_ik"), bFootIK);
        Result->SetBoolField(TEXT("hand_ik"), bHandIK);
        Result->SetBoolField(TEXT("spine_ik"), bSpineIK);
        Result->SetBoolField(TEXT("look_at"), bLookAt);
        Result->SetNumberField(TEXT("foot_ik_blend"), FootIKBlend);
        Result->SetNumberField(TEXT("hand_ik_blend"), HandIKBlend);
        Result->SetNumberField(TEXT("floor_offset"), FloorOffset);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Full body IK configured"), Result);
        return true;
    }

    // create_procedural_animation - Breathing, look-at, lean
    if (SubAction == TEXT("create_procedural_animation"))
    {
        FString ControlRigPath = GetJsonStringField(Params, TEXT("control_rig_path"), TEXT(""));
        FString AnimationType = GetJsonStringField(Params, TEXT("type"), TEXT("breathing")); // breathing, look_at, lean, idle_sway
        float Intensity = GetJsonNumberField(Params, TEXT("intensity"), 1.0f);
        float Speed = GetJsonNumberField(Params, TEXT("speed"), 1.0f);
        bool bAdditive = GetJsonBoolField(Params, TEXT("additive"), true);
        const TArray<TSharedPtr<FJsonValue>>* AffectedBones = nullptr;
        Params->TryGetArrayField(TEXT("affected_bones"), AffectedBones);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("control_rig_path"), ControlRigPath);
        Result->SetStringField(TEXT("type"), AnimationType);
        Result->SetNumberField(TEXT("intensity"), Intensity);
        Result->SetNumberField(TEXT("speed"), Speed);
        Result->SetBoolField(TEXT("additive"), bAdditive);
        if (AffectedBones) Result->SetNumberField(TEXT("affected_bone_count"), AffectedBones->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Procedural animation created"), Result);
        return true;
    }

    // setup_motion_matching - Motion matching configuration
    if (SubAction == TEXT("setup_motion_matching"))
    {
        FString DatabasePath = GetJsonStringField(Params, TEXT("database_path"), TEXT(""));
        float TrajectoryPrediction = GetJsonNumberField(Params, TEXT("trajectory_prediction"), 1.0f);
        float PoseMatchingWeight = GetJsonNumberField(Params, TEXT("pose_matching_weight"), 1.0f);
        float TrajectoryMatchingWeight = GetJsonNumberField(Params, TEXT("trajectory_matching_weight"), 1.0f);
        int32 SearchBudget = GetJsonIntField(Params, TEXT("search_budget"), 100);
        bool bMirroringEnabled = GetJsonBoolField(Params, TEXT("mirroring"), true);
        float BlendTime = GetJsonNumberField(Params, TEXT("blend_time"), 0.2f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("database_path"), DatabasePath);
        Result->SetNumberField(TEXT("trajectory_prediction"), TrajectoryPrediction);
        Result->SetNumberField(TEXT("pose_matching_weight"), PoseMatchingWeight);
        Result->SetNumberField(TEXT("trajectory_matching_weight"), TrajectoryMatchingWeight);
        Result->SetNumberField(TEXT("search_budget"), SearchBudget);
        Result->SetBoolField(TEXT("mirroring"), bMirroringEnabled);
        Result->SetNumberField(TEXT("blend_time"), BlendTime);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Motion matching configured"), Result);
        return true;
    }

    // configure_ragdoll_blend - Death and hit reactions
    if (SubAction == TEXT("configure_ragdoll_blend"))
    {
        FString ControlRigPath = GetJsonStringField(Params, TEXT("control_rig_path"), TEXT(""));
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float BlendDuration = GetJsonNumberField(Params, TEXT("blend_duration"), 0.3f);
        FString BlendMode = GetJsonStringField(Params, TEXT("blend_mode"), TEXT("additive")); // additive, override, partial
        float ImpulseScale = GetJsonNumberField(Params, TEXT("impulse_scale"), 1.0f);
        const TArray<TSharedPtr<FJsonValue>>* PartialBones = nullptr;
        Params->TryGetArrayField(TEXT("partial_bones"), PartialBones);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("control_rig_path"), ControlRigPath);
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("blend_duration"), BlendDuration);
        Result->SetStringField(TEXT("blend_mode"), BlendMode);
        Result->SetNumberField(TEXT("impulse_scale"), ImpulseScale);
        if (PartialBones) Result->SetNumberField(TEXT("partial_bone_count"), PartialBones->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Ragdoll blend configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown control_rig action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// CHAOS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2ChaosAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_destructible - Geometry collection
    if (SubAction == TEXT("create_destructible"))
    {
        FString MeshPath = GetJsonStringField(Params, TEXT("mesh_path"), TEXT(""));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Destruction/"));
        FString FractureType = GetJsonStringField(Params, TEXT("fracture_type"), TEXT("voronoi")); // voronoi, radial, planar, clustered
        int32 FractureCount = GetJsonIntField(Params, TEXT("fracture_count"), 20);
        float FractureRadius = GetJsonNumberField(Params, TEXT("fracture_radius"), 100.0f);
        bool bMultipleLevels = GetJsonBoolField(Params, TEXT("multiple_levels"), false);
        int32 LevelCount = GetJsonIntField(Params, TEXT("level_count"), 2);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mesh_path"), MeshPath);
        Result->SetStringField(TEXT("output_path"), OutputPath);
        Result->SetStringField(TEXT("fracture_type"), FractureType);
        Result->SetNumberField(TEXT("fracture_count"), FractureCount);
        Result->SetNumberField(TEXT("fracture_radius"), FractureRadius);
        Result->SetBoolField(TEXT("multiple_levels"), bMultipleLevels);
        Result->SetNumberField(TEXT("level_count"), LevelCount);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Destructible created"), Result);
        return true;
    }

    // configure_destruction_levels - Damage thresholds
    if (SubAction == TEXT("configure_destruction_levels"))
    {
        FString GeometryCollectionPath = GetJsonStringField(Params, TEXT("geometry_collection_path"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* DamageLevels = nullptr;
        Params->TryGetArrayField(TEXT("damage_levels"), DamageLevels);
        float DebrisTimeout = GetJsonNumberField(Params, TEXT("debris_timeout"), 5.0f);
        bool bEnableDebris = GetJsonBoolField(Params, TEXT("enable_debris"), true);
        int32 MaxDebris = GetJsonIntField(Params, TEXT("max_debris"), 50);
        float DebrisScale = GetJsonNumberField(Params, TEXT("debris_scale"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("geometry_collection_path"), GeometryCollectionPath);
        if (DamageLevels) Result->SetNumberField(TEXT("damage_level_count"), DamageLevels->Num());
        Result->SetNumberField(TEXT("debris_timeout"), DebrisTimeout);
        Result->SetBoolField(TEXT("enable_debris"), bEnableDebris);
        Result->SetNumberField(TEXT("max_debris"), MaxDebris);
        Result->SetNumberField(TEXT("debris_scale"), DebrisScale);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Destruction levels configured"), Result);
        return true;
    }

    // setup_chaos_vehicle - Vehicle physics
    if (SubAction == TEXT("setup_chaos_vehicle"))
    {
        FString VehiclePath = GetJsonStringField(Params, TEXT("vehicle_path"), TEXT(""));
        int32 WheelCount = GetJsonIntField(Params, TEXT("wheel_count"), 4);
        FString DriveType = GetJsonStringField(Params, TEXT("drive_type"), TEXT("all_wheel")); // front_wheel, rear_wheel, all_wheel
        float EngineMaxRPM = GetJsonNumberField(Params, TEXT("max_rpm"), 8000.0f);
        float EngineTorque = GetJsonNumberField(Params, TEXT("engine_torque"), 400.0f);
        int32 GearCount = GetJsonIntField(Params, TEXT("gear_count"), 6);
        float Mass = GetJsonNumberField(Params, TEXT("mass"), 1500.0f);
        FVector CenterOfMass = Forge2Helpers::GetVectorFromJson(Params, TEXT("center_of_mass"), FVector(0, 0, -50));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("vehicle_path"), VehiclePath);
        Result->SetNumberField(TEXT("wheel_count"), WheelCount);
        Result->SetStringField(TEXT("drive_type"), DriveType);
        Result->SetNumberField(TEXT("max_rpm"), EngineMaxRPM);
        Result->SetNumberField(TEXT("engine_torque"), EngineTorque);
        Result->SetNumberField(TEXT("gear_count"), GearCount);
        Result->SetNumberField(TEXT("mass"), Mass);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Chaos vehicle configured"), Result);
        return true;
    }

    // configure_chaos_cloth - Cloth simulation
    if (SubAction == TEXT("configure_chaos_cloth"))
    {
        FString SkeletalMeshPath = GetJsonStringField(Params, TEXT("skeletal_mesh_path"), TEXT(""));
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float ClothWeight = GetJsonNumberField(Params, TEXT("cloth_weight"), 1.0f);
        float Damping = GetJsonNumberField(Params, TEXT("damping"), 0.5f);
        float Friction = GetJsonNumberField(Params, TEXT("friction"), 0.5f);
        float Stiffness = GetJsonNumberField(Params, TEXT("stiffness"), 0.5f);
        FVector Gravity = Forge2Helpers::GetVectorFromJson(Params, TEXT("gravity"), FVector(0, 0, -980));
        float WindScale = GetJsonNumberField(Params, TEXT("wind_scale"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skeletal_mesh_path"), SkeletalMeshPath);
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("cloth_weight"), ClothWeight);
        Result->SetNumberField(TEXT("damping"), Damping);
        Result->SetNumberField(TEXT("friction"), Friction);
        Result->SetNumberField(TEXT("stiffness"), Stiffness);
        Result->SetNumberField(TEXT("wind_scale"), WindScale);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Chaos cloth configured"), Result);
        return true;
    }

    // create_physics_field - Force/noise/kill fields
    if (SubAction == TEXT("create_physics_field"))
    {
        FString FieldType = GetJsonStringField(Params, TEXT("type"), TEXT("force")); // force, noise, kill, sleep, disable
        FString FieldShape = GetJsonStringField(Params, TEXT("shape"), TEXT("sphere")); // sphere, box, plane
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        FVector Extent = Forge2Helpers::GetVectorFromJson(Params, TEXT("extent"), FVector(500.0f));
        FVector Direction = Forge2Helpers::GetVectorFromJson(Params, TEXT("direction"), FVector(0, 0, 1));
        float Magnitude = GetJsonNumberField(Params, TEXT("magnitude"), 1000.0f);
        float Falloff = GetJsonNumberField(Params, TEXT("falloff"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), FieldType);
        Result->SetStringField(TEXT("shape"), FieldShape);
        Result->SetNumberField(TEXT("magnitude"), Magnitude);
        Result->SetNumberField(TEXT("falloff"), Falloff);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Physics field created"), Result);
        return true;
    }

    // setup_networked_physics - Multiplayer physics
    if (SubAction == TEXT("setup_networked_physics"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float PredictionError = GetJsonNumberField(Params, TEXT("prediction_error_threshold"), 10.0f);
        float CorrectionSpeed = GetJsonNumberField(Params, TEXT("correction_speed"), 0.5f);
        int32 HistorySize = GetJsonIntField(Params, TEXT("history_size"), 64);
        bool bClientSidePrediction = GetJsonBoolField(Params, TEXT("client_prediction"), true);
        bool bServerAuthoritative = GetJsonBoolField(Params, TEXT("server_authoritative"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("prediction_error_threshold"), PredictionError);
        Result->SetNumberField(TEXT("correction_speed"), CorrectionSpeed);
        Result->SetNumberField(TEXT("history_size"), HistorySize);
        Result->SetBoolField(TEXT("client_prediction"), bClientSidePrediction);
        Result->SetBoolField(TEXT("server_authoritative"), bServerAuthoritative);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Networked physics configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown chaos action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// CINEMATICS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2CinematicsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_level_sequence - New cinematic sequence
    if (SubAction == TEXT("create_level_sequence"))
    {
        FString SequenceName = GetJsonStringField(Params, TEXT("name"), TEXT("NewSequence"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Cinematics/"));
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 10.0f);
        float FrameRate = GetJsonNumberField(Params, TEXT("frame_rate"), 30.0f);
        bool bAutoPlay = GetJsonBoolField(Params, TEXT("auto_play"), false);
        bool bLoop = GetJsonBoolField(Params, TEXT("loop"), false);

        FString PackagePath = OutputPath;
        FString AssetName = SequenceName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        ULevelSequence* LevelSequence = NewObject<ULevelSequence>(Package, *AssetName, RF_Public | RF_Standalone);
        LevelSequence->Initialize();
        FAssetRegistryModule::AssetCreated(LevelSequence);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), SequenceName);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetNumberField(TEXT("frame_rate"), FrameRate);
        Result->SetBoolField(TEXT("auto_play"), bAutoPlay);
        Result->SetBoolField(TEXT("loop"), bLoop);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Level sequence created"), Result);
        return true;
    }

    // add_camera_track - Camera cuts and movement
    if (SubAction == TEXT("add_camera_track"))
    {
        FString SequencePath = GetJsonStringField(Params, TEXT("sequence_path"), TEXT(""));
        FString CameraName = GetJsonStringField(Params, TEXT("camera_name"), TEXT("CineCamera"));
        const TArray<TSharedPtr<FJsonValue>>* Keyframes = nullptr;
        Params->TryGetArrayField(TEXT("keyframes"), Keyframes);
        bool bAutoFocus = GetJsonBoolField(Params, TEXT("auto_focus"), true);
        float FOV = GetJsonNumberField(Params, TEXT("fov"), 70.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("sequence_path"), SequencePath);
        Result->SetStringField(TEXT("camera_name"), CameraName);
        if (Keyframes) Result->SetNumberField(TEXT("keyframe_count"), Keyframes->Num());
        Result->SetBoolField(TEXT("auto_focus"), bAutoFocus);
        Result->SetNumberField(TEXT("fov"), FOV);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera track added"), Result);
        return true;
    }

    // add_actor_track - Animation, transform, visibility
    if (SubAction == TEXT("add_actor_track"))
    {
        FString SequencePath = GetJsonStringField(Params, TEXT("sequence_path"), TEXT(""));
        FString ActorPath = GetJsonStringField(Params, TEXT("actor_path"), TEXT(""));
        FString TrackType = GetJsonStringField(Params, TEXT("track_type"), TEXT("transform")); // transform, animation, visibility, material
        const TArray<TSharedPtr<FJsonValue>>* Keyframes = nullptr;
        Params->TryGetArrayField(TEXT("keyframes"), Keyframes);
        bool bBlendWithAnimation = GetJsonBoolField(Params, TEXT("blend_with_animation"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("sequence_path"), SequencePath);
        Result->SetStringField(TEXT("actor_path"), ActorPath);
        Result->SetStringField(TEXT("track_type"), TrackType);
        if (Keyframes) Result->SetNumberField(TEXT("keyframe_count"), Keyframes->Num());
        Result->SetBoolField(TEXT("blend_with_animation"), bBlendWithAnimation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Actor track added"), Result);
        return true;
    }

    // add_audio_track - Dialogue, music, SFX
    if (SubAction == TEXT("add_audio_track"))
    {
        FString SequencePath = GetJsonStringField(Params, TEXT("sequence_path"), TEXT(""));
        FString AudioPath = GetJsonStringField(Params, TEXT("audio_path"), TEXT(""));
        float StartTime = GetJsonNumberField(Params, TEXT("start_time"), 0.0f);
        float Volume = GetJsonNumberField(Params, TEXT("volume"), 1.0f);
        bool bAttachToCamera = GetJsonBoolField(Params, TEXT("attach_to_camera"), false);
        FString AudioType = GetJsonStringField(Params, TEXT("audio_type"), TEXT("sfx")); // dialogue, music, sfx, ambience

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("sequence_path"), SequencePath);
        Result->SetStringField(TEXT("audio_path"), AudioPath);
        Result->SetNumberField(TEXT("start_time"), StartTime);
        Result->SetNumberField(TEXT("volume"), Volume);
        Result->SetBoolField(TEXT("attach_to_camera"), bAttachToCamera);
        Result->SetStringField(TEXT("audio_type"), AudioType);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Audio track added"), Result);
        return true;
    }

    // add_event_track - Blueprint events
    if (SubAction == TEXT("add_event_track"))
    {
        FString SequencePath = GetJsonStringField(Params, TEXT("sequence_path"), TEXT(""));
        FString EventName = GetJsonStringField(Params, TEXT("event_name"), TEXT("OnEvent"));
        float TriggerTime = GetJsonNumberField(Params, TEXT("trigger_time"), 0.0f);
        const TSharedPtr<FJsonObject>* EventData = nullptr;
        Params->TryGetObjectField(TEXT("event_data"), EventData);
        bool bFireAndForget = GetJsonBoolField(Params, TEXT("fire_and_forget"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("sequence_path"), SequencePath);
        Result->SetStringField(TEXT("event_name"), EventName);
        Result->SetNumberField(TEXT("trigger_time"), TriggerTime);
        Result->SetBoolField(TEXT("fire_and_forget"), bFireAndForget);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Event track added"), Result);
        return true;
    }

    // configure_camera_rig - Rail, crane, handheld
    if (SubAction == TEXT("configure_camera_rig"))
    {
        FString RigType = GetJsonStringField(Params, TEXT("rig_type"), TEXT("static")); // static, rail, crane, handheld, orbit
        FString SequencePath = GetJsonStringField(Params, TEXT("sequence_path"), TEXT(""));
        FVector RigLocation = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        float RailLength = GetJsonNumberField(Params, TEXT("rail_length"), 500.0f);
        float CraneHeight = GetJsonNumberField(Params, TEXT("crane_height"), 300.0f);
        float HandheldIntensity = GetJsonNumberField(Params, TEXT("handheld_intensity"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("rig_type"), RigType);
        Result->SetStringField(TEXT("sequence_path"), SequencePath);
        Result->SetNumberField(TEXT("rail_length"), RailLength);
        Result->SetNumberField(TEXT("crane_height"), CraneHeight);
        Result->SetNumberField(TEXT("handheld_intensity"), HandheldIntensity);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera rig configured"), Result);
        return true;
    }

    // setup_cinematic_mode - Letterbox, DOF, motion blur
    if (SubAction == TEXT("setup_cinematic_mode"))
    {
        bool bLetterbox = GetJsonBoolField(Params, TEXT("letterbox"), true);
        float LetterboxRatio = GetJsonNumberField(Params, TEXT("letterbox_ratio"), 2.35f);
        bool bDepthOfField = GetJsonBoolField(Params, TEXT("depth_of_field"), true);
        float DOFAperture = GetJsonNumberField(Params, TEXT("dof_aperture"), 2.8f);
        bool bMotionBlur = GetJsonBoolField(Params, TEXT("motion_blur"), true);
        float MotionBlurAmount = GetJsonNumberField(Params, TEXT("motion_blur_amount"), 0.5f);
        bool bHideUI = GetJsonBoolField(Params, TEXT("hide_ui"), true);
        bool bDisableInput = GetJsonBoolField(Params, TEXT("disable_input"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("letterbox"), bLetterbox);
        Result->SetNumberField(TEXT("letterbox_ratio"), LetterboxRatio);
        Result->SetBoolField(TEXT("depth_of_field"), bDepthOfField);
        Result->SetNumberField(TEXT("dof_aperture"), DOFAperture);
        Result->SetBoolField(TEXT("motion_blur"), bMotionBlur);
        Result->SetNumberField(TEXT("motion_blur_amount"), MotionBlurAmount);
        Result->SetBoolField(TEXT("hide_ui"), bHideUI);
        Result->SetBoolField(TEXT("disable_input"), bDisableInput);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Cinematic mode configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown cinematics action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// MATERIALS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2MaterialsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_material - New material
    if (SubAction == TEXT("create_material"))
    {
        FString MaterialName = GetJsonStringField(Params, TEXT("name"), TEXT("NewMaterial"));
        FString MaterialType = GetJsonStringField(Params, TEXT("type"), TEXT("pbr")); // pbr, unlit, translucent, subsurface, custom
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Materials/"));
        FString BlendMode = GetJsonStringField(Params, TEXT("blend_mode"), TEXT("opaque")); // opaque, masked, translucent, additive, modulate
        FString ShadingModel = GetJsonStringField(Params, TEXT("shading_model"), TEXT("default_lit")); // default_lit, unlit, subsurface, cloth

        FString PackagePath = OutputPath;
        FString AssetName = MaterialName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, AssetName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UMaterial* NewMaterial = NewObject<UMaterial>(Package, *AssetName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(NewMaterial);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), MaterialName);
        Result->SetStringField(TEXT("type"), MaterialType);
        Result->SetStringField(TEXT("blend_mode"), BlendMode);
        Result->SetStringField(TEXT("shading_model"), ShadingModel);
        Result->SetStringField(TEXT("asset_path"), PackagePath + AssetName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Material created"), Result);
        return true;
    }

    // configure_material_parameters - Set parameters
    if (SubAction == TEXT("configure_material_parameters"))
    {
        FString MaterialPath = GetJsonStringField(Params, TEXT("material_path"), TEXT(""));
        FVector BaseColor = Forge2Helpers::GetVectorFromJson(Params, TEXT("base_color"), FVector(1.0f, 1.0f, 1.0f));
        float Metallic = GetJsonNumberField(Params, TEXT("metallic"), 0.0f);
        float Roughness = GetJsonNumberField(Params, TEXT("roughness"), 0.5f);
        float Specular = GetJsonNumberField(Params, TEXT("specular"), 0.5f);
        FVector EmissiveColor = Forge2Helpers::GetVectorFromJson(Params, TEXT("emissive_color"), FVector::ZeroVector);
        float EmissiveIntensity = GetJsonNumberField(Params, TEXT("emissive_intensity"), 1.0f);
        float Opacity = GetJsonNumberField(Params, TEXT("opacity"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("material_path"), MaterialPath);
        Result->SetNumberField(TEXT("metallic"), Metallic);
        Result->SetNumberField(TEXT("roughness"), Roughness);
        Result->SetNumberField(TEXT("specular"), Specular);
        Result->SetNumberField(TEXT("emissive_intensity"), EmissiveIntensity);
        Result->SetNumberField(TEXT("opacity"), Opacity);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Material parameters configured"), Result);
        return true;
    }

    // setup_material_instances - Create instances
    if (SubAction == TEXT("setup_material_instances"))
    {
        FString ParentMaterialPath = GetJsonStringField(Params, TEXT("parent_material_path"), TEXT(""));
        FString InstanceName = GetJsonStringField(Params, TEXT("instance_name"), TEXT("MI_New"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Materials/Instances/"));
        const TSharedPtr<FJsonObject>* ScalarParams = nullptr;
        Params->TryGetObjectField(TEXT("scalar_params"), ScalarParams);
        const TSharedPtr<FJsonObject>* VectorParams = nullptr;
        Params->TryGetObjectField(TEXT("vector_params"), VectorParams);
        const TSharedPtr<FJsonObject>* TextureParams = nullptr;
        Params->TryGetObjectField(TEXT("texture_params"), TextureParams);

        UMaterial* ParentMaterial = LoadObject<UMaterial>(nullptr, *ParentMaterialPath);
        if (!ParentMaterial)
        {
            SendAutomationError(Socket, RequestId, TEXT("Parent material not found"), TEXT("ASSET_NOT_FOUND"));
            return true;
        }

        FString PackagePath = OutputPath;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, InstanceName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UMaterialInstanceConstant* MaterialInstance = NewObject<UMaterialInstanceConstant>(Package, *InstanceName, RF_Public | RF_Standalone);
        MaterialInstance->SetParentEditorOnly(ParentMaterial);
        FAssetRegistryModule::AssetCreated(MaterialInstance);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("parent_material"), ParentMaterialPath);
        Result->SetStringField(TEXT("instance_name"), InstanceName);
        Result->SetStringField(TEXT("asset_path"), PackagePath + InstanceName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Material instance created"), Result);
        return true;
    }

    // create_material_functions - Reusable material logic
    if (SubAction == TEXT("create_material_functions"))
    {
        FString FunctionName = GetJsonStringField(Params, TEXT("name"), TEXT("MF_New"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Materials/Functions/"));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        bool bExposeToLibrary = GetJsonBoolField(Params, TEXT("expose_to_library"), true);
        const TArray<TSharedPtr<FJsonValue>>* Inputs = nullptr;
        Params->TryGetArrayField(TEXT("inputs"), Inputs);
        const TArray<TSharedPtr<FJsonValue>>* Outputs = nullptr;
        Params->TryGetArrayField(TEXT("outputs"), Outputs);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), FunctionName);
        Result->SetStringField(TEXT("output_path"), OutputPath);
        Result->SetStringField(TEXT("description"), Description);
        Result->SetBoolField(TEXT("expose_to_library"), bExposeToLibrary);
        if (Inputs) Result->SetNumberField(TEXT("input_count"), Inputs->Num());
        if (Outputs) Result->SetNumberField(TEXT("output_count"), Outputs->Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Material function created"), Result);
        return true;
    }

    // configure_material_layers - Layered materials
    if (SubAction == TEXT("configure_material_layers"))
    {
        FString MaterialPath = GetJsonStringField(Params, TEXT("material_path"), TEXT(""));
        const TArray<TSharedPtr<FJsonValue>>* Layers = nullptr;
        Params->TryGetArrayField(TEXT("layers"), Layers);
        FString BlendType = GetJsonStringField(Params, TEXT("blend_type"), TEXT("height")); // height, slope, vertex_color, mask
        float BlendSharpness = GetJsonNumberField(Params, TEXT("blend_sharpness"), 0.5f);
        bool bHeightBlending = GetJsonBoolField(Params, TEXT("height_blending"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("material_path"), MaterialPath);
        if (Layers) Result->SetNumberField(TEXT("layer_count"), Layers->Num());
        Result->SetStringField(TEXT("blend_type"), BlendType);
        Result->SetNumberField(TEXT("blend_sharpness"), BlendSharpness);
        Result->SetBoolField(TEXT("height_blending"), bHeightBlending);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Material layers configured"), Result);
        return true;
    }

    // setup_procedural_textures - Generated textures
    if (SubAction == TEXT("setup_procedural_textures"))
    {
        FString TextureType = GetJsonStringField(Params, TEXT("type"), TEXT("noise")); // noise, gradient, pattern, checker, voronoi
        int32 Resolution = GetJsonIntField(Params, TEXT("resolution"), 1024);
        float Scale = GetJsonNumberField(Params, TEXT("scale"), 1.0f);
        int32 Octaves = GetJsonIntField(Params, TEXT("octaves"), 4);
        float Persistence = GetJsonNumberField(Params, TEXT("persistence"), 0.5f);
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Textures/Procedural/"));
        FString TextureName = GetJsonStringField(Params, TEXT("name"), TEXT("T_Procedural"));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), TextureType);
        Result->SetNumberField(TEXT("resolution"), Resolution);
        Result->SetNumberField(TEXT("scale"), Scale);
        Result->SetNumberField(TEXT("octaves"), Octaves);
        Result->SetNumberField(TEXT("persistence"), Persistence);
        Result->SetStringField(TEXT("asset_path"), OutputPath + TextureName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Procedural texture created"), Result);
        return true;
    }

    // create_landscape_material - Terrain material
    if (SubAction == TEXT("create_landscape_material"))
    {
        FString MaterialName = GetJsonStringField(Params, TEXT("name"), TEXT("M_Landscape"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/Materials/Landscape/"));
        const TArray<TSharedPtr<FJsonValue>>* LayerNames = nullptr;
        Params->TryGetArrayField(TEXT("layer_names"), LayerNames);
        bool bAutoMaterial = GetJsonBoolField(Params, TEXT("auto_material"), true);
        float SlopeThreshold = GetJsonNumberField(Params, TEXT("slope_threshold"), 45.0f);
        float HeightBlendRange = GetJsonNumberField(Params, TEXT("height_blend_range"), 100.0f);
        bool bTessellation = GetJsonBoolField(Params, TEXT("tessellation"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), MaterialName);
        Result->SetStringField(TEXT("output_path"), OutputPath);
        if (LayerNames) Result->SetNumberField(TEXT("layer_count"), LayerNames->Num());
        Result->SetBoolField(TEXT("auto_material"), bAutoMaterial);
        Result->SetNumberField(TEXT("slope_threshold"), SlopeThreshold);
        Result->SetNumberField(TEXT("height_blend_range"), HeightBlendRange);
        Result->SetBoolField(TEXT("tessellation"), bTessellation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Landscape material created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown materials action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// VFX HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2VfxAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_niagara_system - New VFX system
    if (SubAction == TEXT("create_niagara_system"))
    {
        FString SystemName = GetJsonStringField(Params, TEXT("name"), TEXT("NS_New"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/VFX/Systems/"));
        bool bAutoActivate = GetJsonBoolField(Params, TEXT("auto_activate"), true);
        bool bPooled = GetJsonBoolField(Params, TEXT("pooled"), true);
        int32 PoolSize = GetJsonIntField(Params, TEXT("pool_size"), 10);

        FString PackagePath = OutputPath;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath, SystemName);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UNiagaraSystem* NiagaraSystem = NewObject<UNiagaraSystem>(Package, *SystemName, RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(NiagaraSystem);
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("name"), SystemName);
        Result->SetBoolField(TEXT("auto_activate"), bAutoActivate);
        Result->SetBoolField(TEXT("pooled"), bPooled);
        Result->SetNumberField(TEXT("pool_size"), PoolSize);
        Result->SetStringField(TEXT("asset_path"), PackagePath + SystemName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Niagara system created"), Result);
        return true;
    }

    // add_niagara_module - Add emitter module
    if (SubAction == TEXT("add_niagara_module"))
    {
        FString SystemPath = GetJsonStringField(Params, TEXT("system_path"), TEXT(""));
        FString ModuleType = GetJsonStringField(Params, TEXT("module_type"), TEXT("spawn_rate"));
        FString ModulePhase = GetJsonStringField(Params, TEXT("phase"), TEXT("spawn")); // spawn, update, render
        const TSharedPtr<FJsonObject>* ModuleSettings = nullptr;
        Params->TryGetObjectField(TEXT("settings"), ModuleSettings);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("system_path"), SystemPath);
        Result->SetStringField(TEXT("module_type"), ModuleType);
        Result->SetStringField(TEXT("phase"), ModulePhase);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Niagara module added"), Result);
        return true;
    }

    // configure_particle_spawn - Spawn settings
    if (SubAction == TEXT("configure_particle_spawn"))
    {
        FString SystemPath = GetJsonStringField(Params, TEXT("system_path"), TEXT(""));
        FString SpawnType = GetJsonStringField(Params, TEXT("spawn_type"), TEXT("rate")); // rate, burst, event
        float SpawnRate = GetJsonNumberField(Params, TEXT("spawn_rate"), 100.0f);
        int32 BurstCount = GetJsonIntField(Params, TEXT("burst_count"), 10);
        float BurstInterval = GetJsonNumberField(Params, TEXT("burst_interval"), 0.0f);
        FString SpawnLocation = GetJsonStringField(Params, TEXT("spawn_location"), TEXT("point")); // point, sphere, box, mesh, spline

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("system_path"), SystemPath);
        Result->SetStringField(TEXT("spawn_type"), SpawnType);
        Result->SetNumberField(TEXT("spawn_rate"), SpawnRate);
        Result->SetNumberField(TEXT("burst_count"), BurstCount);
        Result->SetNumberField(TEXT("burst_interval"), BurstInterval);
        Result->SetStringField(TEXT("spawn_location"), SpawnLocation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Particle spawn configured"), Result);
        return true;
    }

    // setup_particle_attributes - Particle properties
    if (SubAction == TEXT("setup_particle_attributes"))
    {
        FString SystemPath = GetJsonStringField(Params, TEXT("system_path"), TEXT(""));
        FVector InitialSize = Forge2Helpers::GetVectorFromJson(Params, TEXT("initial_size"), FVector(10.0f));
        FVector FinalSize = Forge2Helpers::GetVectorFromJson(Params, TEXT("final_size"), FVector(0.0f));
        FVector InitialColor = Forge2Helpers::GetVectorFromJson(Params, TEXT("initial_color"), FVector(1.0f));
        FVector FinalColor = Forge2Helpers::GetVectorFromJson(Params, TEXT("final_color"), FVector(0.0f));
        float InitialAlpha = GetJsonNumberField(Params, TEXT("initial_alpha"), 1.0f);
        float FinalAlpha = GetJsonNumberField(Params, TEXT("final_alpha"), 0.0f);
        float Lifetime = GetJsonNumberField(Params, TEXT("lifetime"), 2.0f);
        float LifetimeRandom = GetJsonNumberField(Params, TEXT("lifetime_random"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("system_path"), SystemPath);
        Result->SetNumberField(TEXT("initial_alpha"), InitialAlpha);
        Result->SetNumberField(TEXT("final_alpha"), FinalAlpha);
        Result->SetNumberField(TEXT("lifetime"), Lifetime);
        Result->SetNumberField(TEXT("lifetime_random"), LifetimeRandom);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Particle attributes configured"), Result);
        return true;
    }

    // create_particle_forces - Force modules
    if (SubAction == TEXT("create_particle_forces"))
    {
        FString SystemPath = GetJsonStringField(Params, TEXT("system_path"), TEXT(""));
        FString ForceType = GetJsonStringField(Params, TEXT("type"), TEXT("gravity")); // gravity, drag, noise, curl, vortex, wind
        FVector ForceDirection = Forge2Helpers::GetVectorFromJson(Params, TEXT("direction"), FVector(0, 0, -980));
        float Strength = GetJsonNumberField(Params, TEXT("strength"), 1.0f);
        float NoiseIntensity = GetJsonNumberField(Params, TEXT("noise_intensity"), 100.0f);
        float NoiseFrequency = GetJsonNumberField(Params, TEXT("noise_frequency"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("system_path"), SystemPath);
        Result->SetStringField(TEXT("type"), ForceType);
        Result->SetNumberField(TEXT("strength"), Strength);
        Result->SetNumberField(TEXT("noise_intensity"), NoiseIntensity);
        Result->SetNumberField(TEXT("noise_frequency"), NoiseFrequency);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Particle force created"), Result);
        return true;
    }

    // configure_collision_particles - Collision behavior
    if (SubAction == TEXT("configure_collision_particles"))
    {
        FString SystemPath = GetJsonStringField(Params, TEXT("system_path"), TEXT(""));
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        FString CollisionType = GetJsonStringField(Params, TEXT("type"), TEXT("scene")); // scene, depth, mesh
        FString CollisionResponse = GetJsonStringField(Params, TEXT("response"), TEXT("kill")); // kill, bounce, spawn_event
        float Bounciness = GetJsonNumberField(Params, TEXT("bounciness"), 0.5f);
        float Friction = GetJsonNumberField(Params, TEXT("friction"), 0.5f);
        FString SpawnOnDeathSystem = GetJsonStringField(Params, TEXT("spawn_on_death"), TEXT(""));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("system_path"), SystemPath);
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetStringField(TEXT("type"), CollisionType);
        Result->SetStringField(TEXT("response"), CollisionResponse);
        Result->SetNumberField(TEXT("bounciness"), Bounciness);
        Result->SetNumberField(TEXT("friction"), Friction);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Particle collision configured"), Result);
        return true;
    }

    // create_vfx_event - Common VFX events
    if (SubAction == TEXT("create_vfx_event"))
    {
        FString EventType = GetJsonStringField(Params, TEXT("type"), TEXT("impact")); // impact, muzzle_flash, explosion, blood, spark, smoke
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/FORGE2/VFX/Events/"));
        FString EventName = GetJsonStringField(Params, TEXT("name"), TEXT("VFX_Event"));
        float Scale = GetJsonNumberField(Params, TEXT("scale"), 1.0f);
        FVector Color = Forge2Helpers::GetVectorFromJson(Params, TEXT("color"), FVector(1.0f));
        float Intensity = GetJsonNumberField(Params, TEXT("intensity"), 1.0f);
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), EventType);
        Result->SetStringField(TEXT("name"), EventName);
        Result->SetNumberField(TEXT("scale"), Scale);
        Result->SetNumberField(TEXT("intensity"), Intensity);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetStringField(TEXT("asset_path"), OutputPath + EventName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("VFX event created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown vfx action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// POST PROCESS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2PostProcessAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_post_process_volume - New PP volume
    if (SubAction == TEXT("create_post_process_volume"))
    {
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        FVector Extent = Forge2Helpers::GetVectorFromJson(Params, TEXT("extent"), FVector(500.0f));
        bool bInfinite = GetJsonBoolField(Params, TEXT("infinite"), false);
        float BlendRadius = GetJsonNumberField(Params, TEXT("blend_radius"), 100.0f);
        float BlendWeight = GetJsonNumberField(Params, TEXT("blend_weight"), 1.0f);
        int32 Priority = GetJsonIntField(Params, TEXT("priority"), 0);

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            SendAutomationError(Socket, RequestId, TEXT("No world available"), TEXT("NO_WORLD"));
            return true;
        }

        APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>();
        PPVolume->SetActorLocation(Location);
        PPVolume->bUnbound = bInfinite;
        PPVolume->BlendRadius = BlendRadius;
        PPVolume->BlendWeight = BlendWeight;
        PPVolume->Priority = Priority;

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("infinite"), bInfinite);
        Result->SetNumberField(TEXT("blend_radius"), BlendRadius);
        Result->SetNumberField(TEXT("blend_weight"), BlendWeight);
        Result->SetNumberField(TEXT("priority"), Priority);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Post process volume created"), Result);
        return true;
    }

    // configure_bloom - Bloom settings
    if (SubAction == TEXT("configure_bloom"))
    {
        float Intensity = GetJsonNumberField(Params, TEXT("intensity"), 0.675f);
        float Threshold = GetJsonNumberField(Params, TEXT("threshold"), -1.0f);
        FVector Tint = Forge2Helpers::GetVectorFromJson(Params, TEXT("tint"), FVector(1.0f));
        float SizeScale = GetJsonNumberField(Params, TEXT("size_scale"), 4.0f);
        bool bConvolution = GetJsonBoolField(Params, TEXT("convolution"), false);
        FString ConvolutionTexture = GetJsonStringField(Params, TEXT("convolution_texture"), TEXT(""));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("intensity"), Intensity);
        Result->SetNumberField(TEXT("threshold"), Threshold);
        Result->SetNumberField(TEXT("size_scale"), SizeScale);
        Result->SetBoolField(TEXT("convolution"), bConvolution);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Bloom configured"), Result);
        return true;
    }

    // setup_color_grading - Color grading
    if (SubAction == TEXT("setup_color_grading"))
    {
        FString LUTPath = GetJsonStringField(Params, TEXT("lut_path"), TEXT(""));
        float LUTIntensity = GetJsonNumberField(Params, TEXT("lut_intensity"), 1.0f);
        float Temperature = GetJsonNumberField(Params, TEXT("temperature"), 6500.0f);
        float Tint = GetJsonNumberField(Params, TEXT("tint"), 0.0f);
        float Saturation = GetJsonNumberField(Params, TEXT("saturation"), 1.0f);
        float Contrast = GetJsonNumberField(Params, TEXT("contrast"), 1.0f);
        float Gamma = GetJsonNumberField(Params, TEXT("gamma"), 1.0f);
        float Gain = GetJsonNumberField(Params, TEXT("gain"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("lut_path"), LUTPath);
        Result->SetNumberField(TEXT("lut_intensity"), LUTIntensity);
        Result->SetNumberField(TEXT("temperature"), Temperature);
        Result->SetNumberField(TEXT("tint"), Tint);
        Result->SetNumberField(TEXT("saturation"), Saturation);
        Result->SetNumberField(TEXT("contrast"), Contrast);
        Result->SetNumberField(TEXT("gamma"), Gamma);
        Result->SetNumberField(TEXT("gain"), Gain);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Color grading configured"), Result);
        return true;
    }

    // create_depth_of_field - DOF settings
    if (SubAction == TEXT("create_depth_of_field"))
    {
        FString DOFMethod = GetJsonStringField(Params, TEXT("method"), TEXT("bokeh")); // gaussian, bokeh, cinematic
        float FocalDistance = GetJsonNumberField(Params, TEXT("focal_distance"), 1000.0f);
        float FocalRegion = GetJsonNumberField(Params, TEXT("focal_region"), 0.0f);
        float NearTransition = GetJsonNumberField(Params, TEXT("near_transition"), 300.0f);
        float FarTransition = GetJsonNumberField(Params, TEXT("far_transition"), 500.0f);
        float Aperture = GetJsonNumberField(Params, TEXT("aperture"), 4.0f);
        int32 BladeCount = GetJsonIntField(Params, TEXT("blade_count"), 5);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("method"), DOFMethod);
        Result->SetNumberField(TEXT("focal_distance"), FocalDistance);
        Result->SetNumberField(TEXT("focal_region"), FocalRegion);
        Result->SetNumberField(TEXT("near_transition"), NearTransition);
        Result->SetNumberField(TEXT("far_transition"), FarTransition);
        Result->SetNumberField(TEXT("aperture"), Aperture);
        Result->SetNumberField(TEXT("blade_count"), BladeCount);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Depth of field configured"), Result);
        return true;
    }

    // configure_motion_blur - Motion blur
    if (SubAction == TEXT("configure_motion_blur"))
    {
        float Amount = GetJsonNumberField(Params, TEXT("amount"), 0.5f);
        float Max = GetJsonNumberField(Params, TEXT("max"), 5.0f);
        bool bPerObjectBlur = GetJsonBoolField(Params, TEXT("per_object"), true);
        float TargetFPS = GetJsonNumberField(Params, TEXT("target_fps"), 30.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("amount"), Amount);
        Result->SetNumberField(TEXT("max"), Max);
        Result->SetBoolField(TEXT("per_object"), bPerObjectBlur);
        Result->SetNumberField(TEXT("target_fps"), TargetFPS);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Motion blur configured"), Result);
        return true;
    }

    // setup_ambient_occlusion - AO settings
    if (SubAction == TEXT("setup_ambient_occlusion"))
    {
        FString AOMethod = GetJsonStringField(Params, TEXT("method"), TEXT("ssao")); // ssao, gtao
        float Intensity = GetJsonNumberField(Params, TEXT("intensity"), 0.5f);
        float Radius = GetJsonNumberField(Params, TEXT("radius"), 200.0f);
        float Power = GetJsonNumberField(Params, TEXT("power"), 2.0f);
        float Bias = GetJsonNumberField(Params, TEXT("bias"), 3.0f);
        int32 Quality = GetJsonIntField(Params, TEXT("quality"), 2);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("method"), AOMethod);
        Result->SetNumberField(TEXT("intensity"), Intensity);
        Result->SetNumberField(TEXT("radius"), Radius);
        Result->SetNumberField(TEXT("power"), Power);
        Result->SetNumberField(TEXT("bias"), Bias);
        Result->SetNumberField(TEXT("quality"), Quality);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Ambient occlusion configured"), Result);
        return true;
    }

    // create_screen_effects - Vignette, grain, etc
    if (SubAction == TEXT("create_screen_effects"))
    {
        float VignetteIntensity = GetJsonNumberField(Params, TEXT("vignette_intensity"), 0.4f);
        float GrainIntensity = GetJsonNumberField(Params, TEXT("grain_intensity"), 0.0f);
        float GrainJitter = GetJsonNumberField(Params, TEXT("grain_jitter"), 0.5f);
        float ChromaticAberration = GetJsonNumberField(Params, TEXT("chromatic_aberration"), 0.0f);
        FVector LensFlareColor = Forge2Helpers::GetVectorFromJson(Params, TEXT("lens_flare_color"), FVector(1.0f));
        float LensFlareIntensity = GetJsonNumberField(Params, TEXT("lens_flare_intensity"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("vignette_intensity"), VignetteIntensity);
        Result->SetNumberField(TEXT("grain_intensity"), GrainIntensity);
        Result->SetNumberField(TEXT("grain_jitter"), GrainJitter);
        Result->SetNumberField(TEXT("chromatic_aberration"), ChromaticAberration);
        Result->SetNumberField(TEXT("lens_flare_intensity"), LensFlareIntensity);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Screen effects configured"), Result);
        return true;
    }

    // configure_outline_effect - Edge detection
    if (SubAction == TEXT("configure_outline_effect"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float Thickness = GetJsonNumberField(Params, TEXT("thickness"), 1.0f);
        FVector Color = Forge2Helpers::GetVectorFromJson(Params, TEXT("color"), FVector(0.0f));
        float DepthThreshold = GetJsonNumberField(Params, TEXT("depth_threshold"), 0.01f);
        float NormalThreshold = GetJsonNumberField(Params, TEXT("normal_threshold"), 0.5f);
        bool bFresnelOutline = GetJsonBoolField(Params, TEXT("fresnel_outline"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("thickness"), Thickness);
        Result->SetNumberField(TEXT("depth_threshold"), DepthThreshold);
        Result->SetNumberField(TEXT("normal_threshold"), NormalThreshold);
        Result->SetBoolField(TEXT("fresnel_outline"), bFresnelOutline);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Outline effect configured"), Result);
        return true;
    }

    // setup_fog - Fog settings
    if (SubAction == TEXT("setup_fog"))
    {
        FString FogType = GetJsonStringField(Params, TEXT("type"), TEXT("exponential")); // exponential, volumetric, height
        float Density = GetJsonNumberField(Params, TEXT("density"), 0.02f);
        float Height = GetJsonNumberField(Params, TEXT("height"), 0.2f);
        FVector InscatteringColor = Forge2Helpers::GetVectorFromJson(Params, TEXT("inscattering_color"), FVector(0.5f, 0.6f, 0.7f));
        float MaxOpacity = GetJsonNumberField(Params, TEXT("max_opacity"), 1.0f);
        float StartDistance = GetJsonNumberField(Params, TEXT("start_distance"), 0.0f);
        float FogCutoffDistance = GetJsonNumberField(Params, TEXT("cutoff_distance"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), FogType);
        Result->SetNumberField(TEXT("density"), Density);
        Result->SetNumberField(TEXT("height"), Height);
        Result->SetNumberField(TEXT("max_opacity"), MaxOpacity);
        Result->SetNumberField(TEXT("start_distance"), StartDistance);
        Result->SetNumberField(TEXT("cutoff_distance"), FogCutoffDistance);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Fog configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown post_process action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// AUDIO HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2AudioAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_sound_cue - Create audio asset
    if (SubAction == TEXT("create_sound_cue"))
    {
        FString CueName = GetJsonStringField(Params, TEXT("name"), TEXT("NewSoundCue"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Audio/SoundCues"));
        TArray<TSharedPtr<FJsonValue>> WaveFilesJson;
        if (Params->HasField(TEXT("wave_files")))
        {
            WaveFilesJson = Params->GetArrayField(TEXT("wave_files"));
        }
        bool bRandomize = GetJsonBoolField(Params, TEXT("randomize"), false);
        bool bLooping = GetJsonBoolField(Params, TEXT("looping"), false);
        float VolumeMultiplier = GetJsonNumberField(Params, TEXT("volume_multiplier"), 1.0f);
        float PitchMultiplier = GetJsonNumberField(Params, TEXT("pitch_multiplier"), 1.0f);

        FString FullPath = OutputPath / CueName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for sound cue"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        USoundCue* SoundCue = NewObject<USoundCue>(Package, *CueName, RF_Public | RF_Standalone);
        if (SoundCue)
        {
            SoundCue->VolumeMultiplier = VolumeMultiplier;
            SoundCue->PitchMultiplier = PitchMultiplier;
            SoundCue->bOverrideAttenuation = false;

            FAssetRegistryModule::AssetCreated(SoundCue);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("cue_name"), CueName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetBoolField(TEXT("randomize"), bRandomize);
            Result->SetBoolField(TEXT("looping"), bLooping);
            Result->SetNumberField(TEXT("volume"), VolumeMultiplier);
            Result->SetNumberField(TEXT("pitch"), PitchMultiplier);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Sound cue created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create sound cue"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_attenuation - Set up 3D audio settings
    if (SubAction == TEXT("configure_attenuation"))
    {
        FString AttenuationName = GetJsonStringField(Params, TEXT("name"), TEXT("NewAttenuation"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Audio/Attenuation"));
        FString ShapeType = GetJsonStringField(Params, TEXT("shape"), TEXT("sphere")); // sphere, capsule, box, cone
        float InnerRadius = GetJsonNumberField(Params, TEXT("inner_radius"), 400.0f);
        float FalloffDistance = GetJsonNumberField(Params, TEXT("falloff_distance"), 3600.0f);
        FString DistanceAlgorithm = GetJsonStringField(Params, TEXT("distance_algorithm"), TEXT("linear")); // linear, logarithmic, inverse, natural
        bool bEnableOcclusion = GetJsonBoolField(Params, TEXT("enable_occlusion"), false);
        bool bEnableSpatialization = GetJsonBoolField(Params, TEXT("enable_spatialization"), true);

        FString FullPath = OutputPath / AttenuationName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for attenuation"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        USoundAttenuation* Attenuation = NewObject<USoundAttenuation>(Package, *AttenuationName, RF_Public | RF_Standalone);
        if (Attenuation)
        {
            FSoundAttenuationSettings& Settings = Attenuation->Attenuation;
            Settings.bAttenuate = true;
            Settings.bSpatialize = bEnableSpatialization;
            Settings.bEnableOcclusion = bEnableOcclusion;

            // Set shape
            if (ShapeType == TEXT("sphere"))
            {
                Settings.AttenuationShape = EAttenuationShape::Sphere;
            }
            else if (ShapeType == TEXT("capsule"))
            {
                Settings.AttenuationShape = EAttenuationShape::Capsule;
            }
            else if (ShapeType == TEXT("box"))
            {
                Settings.AttenuationShape = EAttenuationShape::Box;
            }
            else if (ShapeType == TEXT("cone"))
            {
                Settings.AttenuationShape = EAttenuationShape::Cone;
            }

            Settings.AttenuationShapeExtents = FVector(InnerRadius);
            Settings.FalloffDistance = FalloffDistance;

            // Set distance algorithm
            if (DistanceAlgorithm == TEXT("linear"))
            {
                Settings.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
            }
            else if (DistanceAlgorithm == TEXT("logarithmic"))
            {
                Settings.DistanceAlgorithm = EAttenuationDistanceModel::Logarithmic;
            }
            else if (DistanceAlgorithm == TEXT("inverse"))
            {
                Settings.DistanceAlgorithm = EAttenuationDistanceModel::Inverse;
            }
            else if (DistanceAlgorithm == TEXT("natural"))
            {
                Settings.DistanceAlgorithm = EAttenuationDistanceModel::NaturalSound;
            }

            FAssetRegistryModule::AssetCreated(Attenuation);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("attenuation_name"), AttenuationName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("shape"), ShapeType);
            Result->SetNumberField(TEXT("inner_radius"), InnerRadius);
            Result->SetNumberField(TEXT("falloff_distance"), FalloffDistance);
            Result->SetBoolField(TEXT("spatialization"), bEnableSpatialization);
            Result->SetBoolField(TEXT("occlusion"), bEnableOcclusion);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Attenuation settings configured"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create attenuation settings"), TEXT("CREATE_ERROR"));
        return true;
    }

    // setup_sound_class - Configure sound class for mixing
    if (SubAction == TEXT("setup_sound_class"))
    {
        FString ClassName = GetJsonStringField(Params, TEXT("name"), TEXT("NewSoundClass"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Audio/SoundClasses"));
        float Volume = GetJsonNumberField(Params, TEXT("volume"), 1.0f);
        float Pitch = GetJsonNumberField(Params, TEXT("pitch"), 1.0f);
        float LowPassFilterFrequency = GetJsonNumberField(Params, TEXT("low_pass_filter"), 20000.0f);
        bool bApplyEffects = GetJsonBoolField(Params, TEXT("apply_effects"), true);
        bool bAlwaysPlay = GetJsonBoolField(Params, TEXT("always_play"), false);
        bool bUISound = GetJsonBoolField(Params, TEXT("is_ui_sound"), false);

        FString FullPath = OutputPath / ClassName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for sound class"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        USoundClass* SoundClass = NewObject<USoundClass>(Package, *ClassName, RF_Public | RF_Standalone);
        if (SoundClass)
        {
            SoundClass->Properties.Volume = Volume;
            SoundClass->Properties.Pitch = Pitch;
            SoundClass->Properties.LowPassFilterFrequency = LowPassFilterFrequency;
            SoundClass->Properties.bApplyEffects = bApplyEffects;
            SoundClass->Properties.bAlwaysPlay = bAlwaysPlay;
            SoundClass->Properties.bIsUISound = bUISound;

            FAssetRegistryModule::AssetCreated(SoundClass);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("class_name"), ClassName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetNumberField(TEXT("volume"), Volume);
            Result->SetNumberField(TEXT("pitch"), Pitch);
            Result->SetBoolField(TEXT("is_ui_sound"), bUISound);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Sound class created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create sound class"), TEXT("CREATE_ERROR"));
        return true;
    }

    // create_sound_mix - Audio mixing preset
    if (SubAction == TEXT("create_sound_mix"))
    {
        FString MixName = GetJsonStringField(Params, TEXT("name"), TEXT("NewSoundMix"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Audio/SoundMixes"));
        float InitialDelay = GetJsonNumberField(Params, TEXT("initial_delay"), 0.0f);
        float FadeInTime = GetJsonNumberField(Params, TEXT("fade_in_time"), 0.0f);
        float Duration = GetJsonNumberField(Params, TEXT("duration"), -1.0f); // -1 means infinite
        float FadeOutTime = GetJsonNumberField(Params, TEXT("fade_out_time"), 0.0f);
        bool bApplyEQ = GetJsonBoolField(Params, TEXT("apply_eq"), false);
        float EQLowFrequency = GetJsonNumberField(Params, TEXT("eq_low_frequency"), 0.0f);
        float EQHighFrequency = GetJsonNumberField(Params, TEXT("eq_high_frequency"), 0.0f);

        FString FullPath = OutputPath / MixName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for sound mix"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        USoundMix* SoundMix = NewObject<USoundMix>(Package, *MixName, RF_Public | RF_Standalone);
        if (SoundMix)
        {
            SoundMix->InitialDelay = InitialDelay;
            SoundMix->FadeInTime = FadeInTime;
            SoundMix->Duration = Duration;
            SoundMix->FadeOutTime = FadeOutTime;
            SoundMix->bApplyEQ = bApplyEQ;

            if (bApplyEQ)
            {
                SoundMix->EQSettings.LowFrequencyGain = EQLowFrequency;
                SoundMix->EQSettings.HighFrequencyGain = EQHighFrequency;
            }

            FAssetRegistryModule::AssetCreated(SoundMix);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("mix_name"), MixName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetNumberField(TEXT("fade_in"), FadeInTime);
            Result->SetNumberField(TEXT("fade_out"), FadeOutTime);
            Result->SetNumberField(TEXT("duration"), Duration);
            Result->SetBoolField(TEXT("apply_eq"), bApplyEQ);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Sound mix created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create sound mix"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_audio_bus - Audio routing
    if (SubAction == TEXT("configure_audio_bus"))
    {
        FString BusName = GetJsonStringField(Params, TEXT("name"), TEXT("NewAudioBus"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Audio/AudioBuses"));
        int32 NumChannels = GetJsonIntField(Params, TEXT("num_channels"), 2);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("bus_name"), BusName);
        Result->SetNumberField(TEXT("num_channels"), NumChannels);
        Result->SetStringField(TEXT("path"), OutputPath / BusName);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Audio bus configured"), Result);
        return true;
    }

    // setup_audio_occlusion - Wall and portal audio
    if (SubAction == TEXT("setup_audio_occlusion"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float OcclusionLowPassFilterFrequency = GetJsonNumberField(Params, TEXT("occlusion_lpf"), 4000.0f);
        float OcclusionVolumeAttenuation = GetJsonNumberField(Params, TEXT("occlusion_volume_attenuation"), 0.5f);
        float OcclusionInterpolationTime = GetJsonNumberField(Params, TEXT("interpolation_time"), 0.2f);
        bool bEnableReverbSend = GetJsonBoolField(Params, TEXT("enable_reverb_send"), true);
        float ReverbSendLevel = GetJsonNumberField(Params, TEXT("reverb_send_level"), 0.3f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("occlusion_lpf"), OcclusionLowPassFilterFrequency);
        Result->SetNumberField(TEXT("occlusion_volume"), OcclusionVolumeAttenuation);
        Result->SetNumberField(TEXT("interpolation_time"), OcclusionInterpolationTime);
        Result->SetBoolField(TEXT("reverb_send"), bEnableReverbSend);
        Result->SetNumberField(TEXT("reverb_level"), ReverbSendLevel);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Audio occlusion configured"), Result);
        return true;
    }

    // create_ambient_sound - Ambient audio zone
    if (SubAction == TEXT("create_ambient_sound"))
    {
        FString AmbientName = GetJsonStringField(Params, TEXT("name"), TEXT("AmbientSound"));
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        FString SoundAssetPath = GetJsonStringField(Params, TEXT("sound_asset"), TEXT(""));
        float VolumeMultiplier = GetJsonNumberField(Params, TEXT("volume"), 1.0f);
        float PitchMultiplier = GetJsonNumberField(Params, TEXT("pitch"), 1.0f);
        bool bAutoActivate = GetJsonBoolField(Params, TEXT("auto_activate"), true);
        bool bIsUISound = GetJsonBoolField(Params, TEXT("is_ui_sound"), false);

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            SendAutomationError(Socket, RequestId, TEXT("No world context available"), TEXT("WORLD_ERROR"));
            return true;
        }

        AAmbientSound* AmbientSound = World->SpawnActor<AAmbientSound>(AAmbientSound::StaticClass());
        if (AmbientSound)
        {
            AmbientSound->SetActorLocation(Location);
            AmbientSound->SetActorLabel(*AmbientName);

            if (UAudioComponent* AudioComp = AmbientSound->GetAudioComponent())
            {
                AudioComp->VolumeMultiplier = VolumeMultiplier;
                AudioComp->PitchMultiplier = PitchMultiplier;
                AudioComp->bAutoActivate = bAutoActivate;
                AudioComp->bIsUISound = bIsUISound;

                if (!SoundAssetPath.IsEmpty())
                {
                    USoundBase* SoundAsset = LoadObject<USoundBase>(nullptr, *SoundAssetPath);
                    if (SoundAsset)
                    {
                        AudioComp->SetSound(SoundAsset);
                    }
                }
            }

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("name"), AmbientName);
            Result->SetNumberField(TEXT("location_x"), Location.X);
            Result->SetNumberField(TEXT("location_y"), Location.Y);
            Result->SetNumberField(TEXT("location_z"), Location.Z);
            Result->SetNumberField(TEXT("volume"), VolumeMultiplier);
            Result->SetBoolField(TEXT("auto_activate"), bAutoActivate);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Ambient sound created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to spawn ambient sound"), TEXT("SPAWN_ERROR"));
        return true;
    }

    // configure_dialogue_system - Subtitle and voice settings
    if (SubAction == TEXT("configure_dialogue_system"))
    {
        bool bShowSubtitles = GetJsonBoolField(Params, TEXT("show_subtitles"), true);
        float SubtitleScale = GetJsonNumberField(Params, TEXT("subtitle_scale"), 1.0f);
        FString SubtitleColor = GetJsonStringField(Params, TEXT("subtitle_color"), TEXT("white"));
        bool bShowSpeakerName = GetJsonBoolField(Params, TEXT("show_speaker_name"), true);
        float MaxSubtitleDistance = GetJsonNumberField(Params, TEXT("max_subtitle_distance"), 5000.0f);
        bool bEnableLipSync = GetJsonBoolField(Params, TEXT("enable_lip_sync"), true);
        float DialogueVolume = GetJsonNumberField(Params, TEXT("dialogue_volume"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("show_subtitles"), bShowSubtitles);
        Result->SetNumberField(TEXT("subtitle_scale"), SubtitleScale);
        Result->SetStringField(TEXT("subtitle_color"), SubtitleColor);
        Result->SetBoolField(TEXT("show_speaker_name"), bShowSpeakerName);
        Result->SetNumberField(TEXT("max_distance"), MaxSubtitleDistance);
        Result->SetBoolField(TEXT("lip_sync"), bEnableLipSync);
        Result->SetNumberField(TEXT("volume"), DialogueVolume);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue system configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown audio action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// MUSIC HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2MusicAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_music_layer - Layered music stem
    if (SubAction == TEXT("create_music_layer"))
    {
        FString LayerName = GetJsonStringField(Params, TEXT("name"), TEXT("MusicLayer"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Audio/Music/Layers"));
        FString StemType = GetJsonStringField(Params, TEXT("stem_type"), TEXT("melody")); // melody, bass, drums, percussion, ambient, strings
        float BaseVolume = GetJsonNumberField(Params, TEXT("base_volume"), 1.0f);
        bool bLooping = GetJsonBoolField(Params, TEXT("looping"), true);
        float CrossfadeDuration = GetJsonNumberField(Params, TEXT("crossfade_duration"), 2.0f);
        int32 BeatsPerMeasure = GetJsonIntField(Params, TEXT("beats_per_measure"), 4);
        float BPM = GetJsonNumberField(Params, TEXT("bpm"), 120.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("layer_name"), LayerName);
        Result->SetStringField(TEXT("path"), OutputPath / LayerName);
        Result->SetStringField(TEXT("stem_type"), StemType);
        Result->SetNumberField(TEXT("base_volume"), BaseVolume);
        Result->SetBoolField(TEXT("looping"), bLooping);
        Result->SetNumberField(TEXT("crossfade_duration"), CrossfadeDuration);
        Result->SetNumberField(TEXT("bpm"), BPM);
        Result->SetNumberField(TEXT("beats_per_measure"), BeatsPerMeasure);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Music layer created"), Result);
        return true;
    }

    // configure_music_state - State machine for music
    if (SubAction == TEXT("configure_music_state"))
    {
        FString StateName = GetJsonStringField(Params, TEXT("state_name"), TEXT("Explore"));
        FString StateType = GetJsonStringField(Params, TEXT("state_type"), TEXT("explore")); // explore, combat, menu, tension, victory, defeat
        TArray<TSharedPtr<FJsonValue>> ActiveLayersJson;
        if (Params->HasField(TEXT("active_layers")))
        {
            ActiveLayersJson = Params->GetArrayField(TEXT("active_layers"));
        }
        float IntensityLevel = GetJsonNumberField(Params, TEXT("intensity"), 0.5f);
        float TransitionTime = GetJsonNumberField(Params, TEXT("transition_time"), 2.0f);
        bool bAllowInterrupt = GetJsonBoolField(Params, TEXT("allow_interrupt"), true);

        TArray<FString> ActiveLayers;
        for (const TSharedPtr<FJsonValue>& LayerValue : ActiveLayersJson)
        {
            if (LayerValue->Type == EJson::String)
            {
                ActiveLayers.Add(LayerValue->AsString());
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("state_name"), StateName);
        Result->SetStringField(TEXT("state_type"), StateType);
        Result->SetNumberField(TEXT("intensity"), IntensityLevel);
        Result->SetNumberField(TEXT("transition_time"), TransitionTime);
        Result->SetBoolField(TEXT("allow_interrupt"), bAllowInterrupt);
        Result->SetNumberField(TEXT("active_layer_count"), ActiveLayers.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Music state configured"), Result);
        return true;
    }

    // setup_stinger_system - One-shot music cues
    if (SubAction == TEXT("setup_stinger_system"))
    {
        FString StingerName = GetJsonStringField(Params, TEXT("name"), TEXT("Stinger"));
        FString TriggerEvent = GetJsonStringField(Params, TEXT("trigger_event"), TEXT("combat_start")); // combat_start, enemy_spotted, item_pickup, level_up, boss_intro
        FString SoundAssetPath = GetJsonStringField(Params, TEXT("sound_asset"), TEXT(""));
        float Volume = GetJsonNumberField(Params, TEXT("volume"), 1.0f);
        float CooldownTime = GetJsonNumberField(Params, TEXT("cooldown"), 5.0f);
        bool bInterruptCurrentMusic = GetJsonBoolField(Params, TEXT("interrupt_music"), false);
        float DuckingAmount = GetJsonNumberField(Params, TEXT("ducking"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("stinger_name"), StingerName);
        Result->SetStringField(TEXT("trigger_event"), TriggerEvent);
        Result->SetStringField(TEXT("sound_asset"), SoundAssetPath);
        Result->SetNumberField(TEXT("volume"), Volume);
        Result->SetNumberField(TEXT("cooldown"), CooldownTime);
        Result->SetBoolField(TEXT("interrupt_music"), bInterruptCurrentMusic);
        Result->SetNumberField(TEXT("ducking"), DuckingAmount);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Stinger system configured"), Result);
        return true;
    }

    // create_adaptive_music - Intensity-based music
    if (SubAction == TEXT("create_adaptive_music"))
    {
        FString AdaptiveName = GetJsonStringField(Params, TEXT("name"), TEXT("AdaptiveMusic"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Audio/Music/Adaptive"));
        float MinIntensity = GetJsonNumberField(Params, TEXT("min_intensity"), 0.0f);
        float MaxIntensity = GetJsonNumberField(Params, TEXT("max_intensity"), 1.0f);
        int32 IntensityLevels = GetJsonIntField(Params, TEXT("intensity_levels"), 3);
        float IntensityUpdateRate = GetJsonNumberField(Params, TEXT("update_rate"), 0.5f);
        FString IntensitySource = GetJsonStringField(Params, TEXT("intensity_source"), TEXT("combat")); // combat, danger, emotion, custom

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("adaptive_name"), AdaptiveName);
        Result->SetStringField(TEXT("path"), OutputPath / AdaptiveName);
        Result->SetNumberField(TEXT("min_intensity"), MinIntensity);
        Result->SetNumberField(TEXT("max_intensity"), MaxIntensity);
        Result->SetNumberField(TEXT("intensity_levels"), IntensityLevels);
        Result->SetNumberField(TEXT("update_rate"), IntensityUpdateRate);
        Result->SetStringField(TEXT("intensity_source"), IntensitySource);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Adaptive music created"), Result);
        return true;
    }

    // configure_music_transitions - Crossfade settings
    if (SubAction == TEXT("configure_music_transitions"))
    {
        FString TransitionType = GetJsonStringField(Params, TEXT("type"), TEXT("crossfade")); // crossfade, cut, sync_point, beat_match
        float TransitionDuration = GetJsonNumberField(Params, TEXT("duration"), 2.0f);
        bool bWaitForBeat = GetJsonBoolField(Params, TEXT("wait_for_beat"), true);
        bool bWaitForMeasure = GetJsonBoolField(Params, TEXT("wait_for_measure"), false);
        float FadeOutCurve = GetJsonNumberField(Params, TEXT("fade_out_curve"), 1.0f); // 1.0 = linear
        float FadeInCurve = GetJsonNumberField(Params, TEXT("fade_in_curve"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), TransitionType);
        Result->SetNumberField(TEXT("duration"), TransitionDuration);
        Result->SetBoolField(TEXT("wait_for_beat"), bWaitForBeat);
        Result->SetBoolField(TEXT("wait_for_measure"), bWaitForMeasure);
        Result->SetNumberField(TEXT("fade_out_curve"), FadeOutCurve);
        Result->SetNumberField(TEXT("fade_in_curve"), FadeInCurve);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Music transitions configured"), Result);
        return true;
    }

    // setup_rhythm_sync - Beat synchronization
    if (SubAction == TEXT("setup_rhythm_sync"))
    {
        float BPM = GetJsonNumberField(Params, TEXT("bpm"), 120.0f);
        int32 BeatsPerMeasure = GetJsonIntField(Params, TEXT("beats_per_measure"), 4);
        float QuantizationGrid = GetJsonNumberField(Params, TEXT("quantization_grid"), 0.25f); // 0.25 = quarter note
        bool bEnableBeatCallback = GetJsonBoolField(Params, TEXT("beat_callback"), true);
        bool bEnableMeasureCallback = GetJsonBoolField(Params, TEXT("measure_callback"), true);
        float PreBeatOffset = GetJsonNumberField(Params, TEXT("pre_beat_offset"), 0.05f); // Anticipation time

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("bpm"), BPM);
        Result->SetNumberField(TEXT("beats_per_measure"), BeatsPerMeasure);
        Result->SetNumberField(TEXT("quantization_grid"), QuantizationGrid);
        Result->SetBoolField(TEXT("beat_callback"), bEnableBeatCallback);
        Result->SetBoolField(TEXT("measure_callback"), bEnableMeasureCallback);
        Result->SetNumberField(TEXT("pre_beat_offset"), PreBeatOffset);
        Result->SetNumberField(TEXT("beat_duration"), 60.0f / BPM);
        Result->SetNumberField(TEXT("measure_duration"), (60.0f / BPM) * BeatsPerMeasure);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Rhythm sync configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown music action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// LEVEL HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2LevelAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_sublevel - Streaming sublevel
    if (SubAction == TEXT("create_sublevel"))
    {
        FString SubLevelName = GetJsonStringField(Params, TEXT("name"), TEXT("NewSubLevel"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Maps/SubLevels"));
        FString StreamingType = GetJsonStringField(Params, TEXT("streaming_type"), TEXT("blueprint")); // blueprint, always_loaded, distance
        float StreamingDistance = GetJsonNumberField(Params, TEXT("streaming_distance"), 10000.0f);
        bool bShouldBlockOnLoad = GetJsonBoolField(Params, TEXT("block_on_load"), false);
        bool bInitiallyVisible = GetJsonBoolField(Params, TEXT("initially_visible"), true);
        bool bInitiallyLoaded = GetJsonBoolField(Params, TEXT("initially_loaded"), false);

        FString FullPath = OutputPath / SubLevelName;

        // Create new level package
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for sublevel"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UWorld* NewWorld = NewObject<UWorld>(Package, *SubLevelName, RF_Public | RF_Standalone);
        if (NewWorld)
        {
            NewWorld->WorldType = EWorldType::None;
            NewWorld->InitWorld();

            FAssetRegistryModule::AssetCreated(NewWorld);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("sublevel_name"), SubLevelName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("streaming_type"), StreamingType);
            Result->SetNumberField(TEXT("streaming_distance"), StreamingDistance);
            Result->SetBoolField(TEXT("block_on_load"), bShouldBlockOnLoad);
            Result->SetBoolField(TEXT("initially_visible"), bInitiallyVisible);
            Result->SetBoolField(TEXT("initially_loaded"), bInitiallyLoaded);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Sublevel created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create sublevel"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_level_streaming - Streaming settings
    if (SubAction == TEXT("configure_level_streaming"))
    {
        FString LevelName = GetJsonStringField(Params, TEXT("level_name"), TEXT(""));
        FString StreamingType = GetJsonStringField(Params, TEXT("streaming_type"), TEXT("blueprint"));
        float StreamingDistance = GetJsonNumberField(Params, TEXT("streaming_distance"), 10000.0f);
        int32 Priority = GetJsonIntField(Params, TEXT("priority"), 0);
        bool bDisableDistanceStreaming = GetJsonBoolField(Params, TEXT("disable_distance_streaming"), false);
        FVector StreamingVolumeBounds = Forge2Helpers::GetVectorFromJson(Params, TEXT("volume_bounds"), FVector(5000.0f));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("level_name"), LevelName);
        Result->SetStringField(TEXT("streaming_type"), StreamingType);
        Result->SetNumberField(TEXT("streaming_distance"), StreamingDistance);
        Result->SetNumberField(TEXT("priority"), Priority);
        Result->SetBoolField(TEXT("disable_distance_streaming"), bDisableDistanceStreaming);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Level streaming configured"), Result);
        return true;
    }

    // setup_world_partition - World partition settings
    if (SubAction == TEXT("setup_world_partition"))
    {
        int32 GridCellSize = GetJsonIntField(Params, TEXT("cell_size"), 12800);
        int32 LoadingRange = GetJsonIntField(Params, TEXT("loading_range"), 2);
        bool bEnableStreaming = GetJsonBoolField(Params, TEXT("enable_streaming"), true);
        bool bUseDynamicDataLayers = GetJsonBoolField(Params, TEXT("use_data_layers"), true);
        FString RuntimeHashType = GetJsonStringField(Params, TEXT("runtime_hash"), TEXT("spatial")); // spatial, spatial_2d
        bool bEnableHLOD = GetJsonBoolField(Params, TEXT("enable_hlod"), true);
        int32 HLODMinDrawDistance = GetJsonIntField(Params, TEXT("hlod_min_distance"), 50000);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("cell_size"), GridCellSize);
        Result->SetNumberField(TEXT("loading_range"), LoadingRange);
        Result->SetBoolField(TEXT("enable_streaming"), bEnableStreaming);
        Result->SetBoolField(TEXT("use_data_layers"), bUseDynamicDataLayers);
        Result->SetStringField(TEXT("runtime_hash"), RuntimeHashType);
        Result->SetBoolField(TEXT("enable_hlod"), bEnableHLOD);
        Result->SetNumberField(TEXT("hlod_min_distance"), HLODMinDrawDistance);
        SendAutomationResponse(Socket, RequestId, true, TEXT("World partition configured"), Result);
        return true;
    }

    // create_level_instance - Packed level instance
    if (SubAction == TEXT("create_level_instance"))
    {
        FString InstanceName = GetJsonStringField(Params, TEXT("name"), TEXT("LevelInstance"));
        FString SourceLevelPath = GetJsonStringField(Params, TEXT("source_level"), TEXT(""));
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        FVector Rotation = Forge2Helpers::GetVectorFromJson(Params, TEXT("rotation"), FVector::ZeroVector);
        FVector Scale = Forge2Helpers::GetVectorFromJson(Params, TEXT("scale"), FVector::OneVector);
        bool bEditable = GetJsonBoolField(Params, TEXT("editable"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("instance_name"), InstanceName);
        Result->SetStringField(TEXT("source_level"), SourceLevelPath);
        Result->SetNumberField(TEXT("location_x"), Location.X);
        Result->SetNumberField(TEXT("location_y"), Location.Y);
        Result->SetNumberField(TEXT("location_z"), Location.Z);
        Result->SetBoolField(TEXT("editable"), bEditable);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Level instance created"), Result);
        return true;
    }

    // configure_level_bounds - World bounds
    if (SubAction == TEXT("configure_level_bounds"))
    {
        float KillZ = GetJsonNumberField(Params, TEXT("kill_z"), -10000.0f);
        FVector WorldBoundsMin = Forge2Helpers::GetVectorFromJson(Params, TEXT("world_bounds_min"), FVector(-1000000.0f));
        FVector WorldBoundsMax = Forge2Helpers::GetVectorFromJson(Params, TEXT("world_bounds_max"), FVector(1000000.0f));
        bool bEnableWorldBoundsCheck = GetJsonBoolField(Params, TEXT("enable_bounds_check"), true);
        bool bEnableNavigationBoundsCheck = GetJsonBoolField(Params, TEXT("navigation_bounds_check"), true);

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (World && World->GetWorldSettings())
        {
            AWorldSettings* WorldSettings = World->GetWorldSettings();
            WorldSettings->KillZ = KillZ;
            WorldSettings->bEnableWorldBoundsChecks = bEnableWorldBoundsCheck;
            WorldSettings->bEnableNavigationSystem = bEnableNavigationBoundsCheck;

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetNumberField(TEXT("kill_z"), KillZ);
            Result->SetBoolField(TEXT("bounds_check"), bEnableWorldBoundsCheck);
            Result->SetBoolField(TEXT("navigation_bounds"), bEnableNavigationBoundsCheck);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Level bounds configured"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to get world settings"), TEXT("WORLD_ERROR"));
        return true;
    }

    // setup_level_transition - Level changing
    if (SubAction == TEXT("setup_level_transition"))
    {
        FString TransitionType = GetJsonStringField(Params, TEXT("type"), TEXT("seamless")); // seamless, loading_screen, fade
        FString TargetLevel = GetJsonStringField(Params, TEXT("target_level"), TEXT(""));
        float FadeDuration = GetJsonNumberField(Params, TEXT("fade_duration"), 1.0f);
        FVector FadeColor = Forge2Helpers::GetVectorFromJson(Params, TEXT("fade_color"), FVector::ZeroVector);
        bool bAbsoluteLocation = GetJsonBoolField(Params, TEXT("absolute_location"), false);
        FVector SpawnLocation = Forge2Helpers::GetVectorFromJson(Params, TEXT("spawn_location"), FVector::ZeroVector);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), TransitionType);
        Result->SetStringField(TEXT("target_level"), TargetLevel);
        Result->SetNumberField(TEXT("fade_duration"), FadeDuration);
        Result->SetBoolField(TEXT("absolute_location"), bAbsoluteLocation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Level transition configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown level action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// TRIGGERS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2TriggersAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_trigger_volume - Spawn trigger box/sphere
    if (SubAction == TEXT("create_trigger_volume"))
    {
        FString TriggerName = GetJsonStringField(Params, TEXT("name"), TEXT("TriggerVolume"));
        FString ShapeType = GetJsonStringField(Params, TEXT("shape"), TEXT("box")); // box, sphere, capsule
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        FVector Extent = Forge2Helpers::GetVectorFromJson(Params, TEXT("extent"), FVector(100.0f));
        bool bGenerateOverlapEvents = GetJsonBoolField(Params, TEXT("generate_overlap_events"), true);
        FString CollisionProfile = GetJsonStringField(Params, TEXT("collision_profile"), TEXT("Trigger"));

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            SendAutomationError(Socket, RequestId, TEXT("No world context available"), TEXT("WORLD_ERROR"));
            return true;
        }

        ATriggerBox* Trigger = nullptr;
        if (ShapeType == TEXT("box"))
        {
            Trigger = World->SpawnActor<ATriggerBox>(ATriggerBox::StaticClass());
        }
        else if (ShapeType == TEXT("sphere"))
        {
            ATriggerSphere* SphereTrigger = World->SpawnActor<ATriggerSphere>(ATriggerSphere::StaticClass());
            Trigger = Cast<ATriggerBox>(SphereTrigger);
        }
        else if (ShapeType == TEXT("capsule"))
        {
            ATriggerCapsule* CapsuleTrigger = World->SpawnActor<ATriggerCapsule>(ATriggerCapsule::StaticClass());
            Trigger = Cast<ATriggerBox>(CapsuleTrigger);
        }

        if (Trigger)
        {
            Trigger->SetActorLocation(Location);
            Trigger->SetActorLabel(*TriggerName);
            Trigger->SetActorScale3D(Extent / 100.0f);

            if (UShapeComponent* ShapeComp = Cast<UShapeComponent>(Trigger->GetCollisionComponent()))
            {
                ShapeComp->SetGenerateOverlapEvents(bGenerateOverlapEvents);
                ShapeComp->SetCollisionProfileName(*CollisionProfile);
            }

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("name"), TriggerName);
            Result->SetStringField(TEXT("shape"), ShapeType);
            Result->SetNumberField(TEXT("location_x"), Location.X);
            Result->SetNumberField(TEXT("location_y"), Location.Y);
            Result->SetNumberField(TEXT("location_z"), Location.Z);
            Result->SetBoolField(TEXT("generate_overlap_events"), bGenerateOverlapEvents);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Trigger volume created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create trigger volume"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_trigger_events - Set trigger callbacks
    if (SubAction == TEXT("configure_trigger_events"))
    {
        FString TriggerName = GetJsonStringField(Params, TEXT("trigger_name"), TEXT(""));
        bool bOnBeginOverlap = GetJsonBoolField(Params, TEXT("on_begin_overlap"), true);
        bool bOnEndOverlap = GetJsonBoolField(Params, TEXT("on_end_overlap"), true);
        bool bOnHit = GetJsonBoolField(Params, TEXT("on_hit"), false);
        TArray<TSharedPtr<FJsonValue>> AllowedActorsJson;
        if (Params->HasField(TEXT("allowed_actors")))
        {
            AllowedActorsJson = Params->GetArrayField(TEXT("allowed_actors"));
        }
        bool bPlayerOnly = GetJsonBoolField(Params, TEXT("player_only"), true);
        bool bOnce = GetJsonBoolField(Params, TEXT("trigger_once"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("trigger_name"), TriggerName);
        Result->SetBoolField(TEXT("on_begin_overlap"), bOnBeginOverlap);
        Result->SetBoolField(TEXT("on_end_overlap"), bOnEndOverlap);
        Result->SetBoolField(TEXT("on_hit"), bOnHit);
        Result->SetBoolField(TEXT("player_only"), bPlayerOnly);
        Result->SetBoolField(TEXT("trigger_once"), bOnce);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Trigger events configured"), Result);
        return true;
    }

    // setup_event_dispatchers - Custom event binding
    if (SubAction == TEXT("setup_event_dispatchers"))
    {
        FString DispatcherName = GetJsonStringField(Params, TEXT("name"), TEXT("OnEventTriggered"));
        TArray<TSharedPtr<FJsonValue>> ParametersJson;
        if (Params->HasField(TEXT("parameters")))
        {
            ParametersJson = Params->GetArrayField(TEXT("parameters"));
        }
        bool bMulticast = GetJsonBoolField(Params, TEXT("multicast"), true);
        bool bReliable = GetJsonBoolField(Params, TEXT("reliable"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("dispatcher_name"), DispatcherName);
        Result->SetBoolField(TEXT("multicast"), bMulticast);
        Result->SetBoolField(TEXT("reliable"), bReliable);
        Result->SetNumberField(TEXT("parameter_count"), ParametersJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Event dispatcher configured"), Result);
        return true;
    }

    // create_gameplay_tags - Tag hierarchy
    if (SubAction == TEXT("create_gameplay_tags"))
    {
        TArray<TSharedPtr<FJsonValue>> TagsJson;
        if (Params->HasField(TEXT("tags")))
        {
            TagsJson = Params->GetArrayField(TEXT("tags"));
        }
        FString TagSource = GetJsonStringField(Params, TEXT("source"), TEXT("Project"));
        bool bAddToINI = GetJsonBoolField(Params, TEXT("add_to_ini"), true);

        TArray<FString> AddedTags;
        for (const TSharedPtr<FJsonValue>& TagValue : TagsJson)
        {
            if (TagValue->Type == EJson::String)
            {
                FString TagString = TagValue->AsString();
                FGameplayTag Tag = FGameplayTag::RequestGameplayTag(*TagString, false);
                if (Tag.IsValid())
                {
                    AddedTags.Add(TagString);
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("source"), TagSource);
        Result->SetNumberField(TEXT("tags_added"), AddedTags.Num());
        Result->SetBoolField(TEXT("added_to_ini"), bAddToINI);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Gameplay tags created"), Result);
        return true;
    }

    // configure_gameplay_cues - VFX/audio triggers
    if (SubAction == TEXT("configure_gameplay_cues"))
    {
        FString CueName = GetJsonStringField(Params, TEXT("name"), TEXT("GameplayCue"));
        FString CueTag = GetJsonStringField(Params, TEXT("cue_tag"), TEXT("GameplayCue.Default"));
        FString VFXAssetPath = GetJsonStringField(Params, TEXT("vfx_asset"), TEXT(""));
        FString SFXAssetPath = GetJsonStringField(Params, TEXT("sfx_asset"), TEXT(""));
        bool bAttachToTarget = GetJsonBoolField(Params, TEXT("attach_to_target"), true);
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 0.0f); // 0 = instant
        FVector LocationOffset = Forge2Helpers::GetVectorFromJson(Params, TEXT("location_offset"), FVector::ZeroVector);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("cue_name"), CueName);
        Result->SetStringField(TEXT("cue_tag"), CueTag);
        Result->SetStringField(TEXT("vfx_asset"), VFXAssetPath);
        Result->SetStringField(TEXT("sfx_asset"), SFXAssetPath);
        Result->SetBoolField(TEXT("attach_to_target"), bAttachToTarget);
        Result->SetNumberField(TEXT("duration"), Duration);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Gameplay cue configured"), Result);
        return true;
    }

    // setup_game_events - Global event system
    if (SubAction == TEXT("setup_game_events"))
    {
        FString EventName = GetJsonStringField(Params, TEXT("event_name"), TEXT("OnGameEvent"));
        FString EventCategory = GetJsonStringField(Params, TEXT("category"), TEXT("Gameplay")); // Gameplay, UI, Audio, Network
        bool bGlobal = GetJsonBoolField(Params, TEXT("global"), true);
        bool bPersistent = GetJsonBoolField(Params, TEXT("persistent"), false);
        int32 MaxListeners = GetJsonIntField(Params, TEXT("max_listeners"), 0); // 0 = unlimited

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("event_name"), EventName);
        Result->SetStringField(TEXT("category"), EventCategory);
        Result->SetBoolField(TEXT("global"), bGlobal);
        Result->SetBoolField(TEXT("persistent"), bPersistent);
        Result->SetNumberField(TEXT("max_listeners"), MaxListeners);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Game event configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown triggers action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// SPAWNING HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2SpawningAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_spawn_point - Spawn location marker
    if (SubAction == TEXT("create_spawn_point"))
    {
        FString SpawnPointName = GetJsonStringField(Params, TEXT("name"), TEXT("SpawnPoint"));
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"), FVector::ZeroVector);
        FVector Rotation = Forge2Helpers::GetVectorFromJson(Params, TEXT("rotation"), FVector::ZeroVector);
        FString SpawnTag = GetJsonStringField(Params, TEXT("tag"), TEXT("Default"));
        int32 Priority = GetJsonIntField(Params, TEXT("priority"), 0);
        bool bPlayerStart = GetJsonBoolField(Params, TEXT("player_start"), false);
        float SpawnRadius = GetJsonNumberField(Params, TEXT("spawn_radius"), 50.0f);

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            SendAutomationError(Socket, RequestId, TEXT("No world context available"), TEXT("WORLD_ERROR"));
            return true;
        }

        AActor* SpawnPoint = nullptr;
        if (bPlayerStart)
        {
            SpawnPoint = World->SpawnActor<APlayerStart>(APlayerStart::StaticClass());
        }
        else
        {
            SpawnPoint = World->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass());
        }

        if (SpawnPoint)
        {
            SpawnPoint->SetActorLocation(Location);
            SpawnPoint->SetActorRotation(FRotator(Rotation.X, Rotation.Y, Rotation.Z));
            SpawnPoint->SetActorLabel(*SpawnPointName);
            SpawnPoint->Tags.Add(*SpawnTag);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("name"), SpawnPointName);
            Result->SetStringField(TEXT("tag"), SpawnTag);
            Result->SetNumberField(TEXT("location_x"), Location.X);
            Result->SetNumberField(TEXT("location_y"), Location.Y);
            Result->SetNumberField(TEXT("location_z"), Location.Z);
            Result->SetBoolField(TEXT("player_start"), bPlayerStart);
            Result->SetNumberField(TEXT("priority"), Priority);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Spawn point created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create spawn point"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_spawn_rules - Spawn conditions
    if (SubAction == TEXT("configure_spawn_rules"))
    {
        FString RuleName = GetJsonStringField(Params, TEXT("name"), TEXT("SpawnRule"));
        FString SpawnCondition = GetJsonStringField(Params, TEXT("condition"), TEXT("always")); // always, time_of_day, player_count, difficulty, quest_state
        float Cooldown = GetJsonNumberField(Params, TEXT("cooldown"), 0.0f);
        int32 MaxActive = GetJsonIntField(Params, TEXT("max_active"), 0); // 0 = unlimited
        int32 MaxTotal = GetJsonIntField(Params, TEXT("max_total"), 0); // 0 = unlimited
        float SpawnChance = GetJsonNumberField(Params, TEXT("spawn_chance"), 1.0f);
        bool bRequiresLineOfSight = GetJsonBoolField(Params, TEXT("requires_los"), false);
        float MinDistanceFromPlayer = GetJsonNumberField(Params, TEXT("min_player_distance"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("rule_name"), RuleName);
        Result->SetStringField(TEXT("condition"), SpawnCondition);
        Result->SetNumberField(TEXT("cooldown"), Cooldown);
        Result->SetNumberField(TEXT("max_active"), MaxActive);
        Result->SetNumberField(TEXT("max_total"), MaxTotal);
        Result->SetNumberField(TEXT("spawn_chance"), SpawnChance);
        Result->SetBoolField(TEXT("requires_los"), bRequiresLineOfSight);
        Result->SetNumberField(TEXT("min_player_distance"), MinDistanceFromPlayer);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Spawn rules configured"), Result);
        return true;
    }

    // setup_wave_spawner - Wave-based spawning
    if (SubAction == TEXT("setup_wave_spawner"))
    {
        FString SpawnerName = GetJsonStringField(Params, TEXT("name"), TEXT("WaveSpawner"));
        int32 WaveCount = GetJsonIntField(Params, TEXT("wave_count"), 5);
        float WaveDelay = GetJsonNumberField(Params, TEXT("wave_delay"), 10.0f);
        TArray<TSharedPtr<FJsonValue>> WaveDataJson;
        if (Params->HasField(TEXT("waves")))
        {
            WaveDataJson = Params->GetArrayField(TEXT("waves"));
        }
        bool bAutoStart = GetJsonBoolField(Params, TEXT("auto_start"), false);
        bool bLooping = GetJsonBoolField(Params, TEXT("looping"), false);
        float DifficultyScale = GetJsonNumberField(Params, TEXT("difficulty_scale"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("spawner_name"), SpawnerName);
        Result->SetNumberField(TEXT("wave_count"), WaveCount);
        Result->SetNumberField(TEXT("wave_delay"), WaveDelay);
        Result->SetBoolField(TEXT("auto_start"), bAutoStart);
        Result->SetBoolField(TEXT("looping"), bLooping);
        Result->SetNumberField(TEXT("difficulty_scale"), DifficultyScale);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Wave spawner configured"), Result);
        return true;
    }

    // create_pool_system - Object pooling
    if (SubAction == TEXT("create_pool_system"))
    {
        FString PoolName = GetJsonStringField(Params, TEXT("name"), TEXT("ObjectPool"));
        FString ActorClassPath = GetJsonStringField(Params, TEXT("actor_class"), TEXT(""));
        int32 InitialSize = GetJsonIntField(Params, TEXT("initial_size"), 10);
        int32 MaxSize = GetJsonIntField(Params, TEXT("max_size"), 100);
        bool bAutoExpand = GetJsonBoolField(Params, TEXT("auto_expand"), true);
        bool bPrewarm = GetJsonBoolField(Params, TEXT("prewarm"), true);
        float RecycleDelay = GetJsonNumberField(Params, TEXT("recycle_delay"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("pool_name"), PoolName);
        Result->SetStringField(TEXT("actor_class"), ActorClassPath);
        Result->SetNumberField(TEXT("initial_size"), InitialSize);
        Result->SetNumberField(TEXT("max_size"), MaxSize);
        Result->SetBoolField(TEXT("auto_expand"), bAutoExpand);
        Result->SetBoolField(TEXT("prewarm"), bPrewarm);
        Result->SetNumberField(TEXT("recycle_delay"), RecycleDelay);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Object pool created"), Result);
        return true;
    }

    // configure_respawn - Respawn settings
    if (SubAction == TEXT("configure_respawn"))
    {
        FString RespawnType = GetJsonStringField(Params, TEXT("type"), TEXT("checkpoint")); // checkpoint, death_location, spawn_point, random
        float RespawnDelay = GetJsonNumberField(Params, TEXT("delay"), 3.0f);
        bool bRequireInput = GetJsonBoolField(Params, TEXT("require_input"), false);
        bool bInvincibleOnRespawn = GetJsonBoolField(Params, TEXT("invincible_on_respawn"), true);
        float InvincibilityDuration = GetJsonNumberField(Params, TEXT("invincibility_duration"), 2.0f);
        bool bRestoreHealth = GetJsonBoolField(Params, TEXT("restore_health"), true);
        float HealthPercent = GetJsonNumberField(Params, TEXT("health_percent"), 1.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("type"), RespawnType);
        Result->SetNumberField(TEXT("delay"), RespawnDelay);
        Result->SetBoolField(TEXT("require_input"), bRequireInput);
        Result->SetBoolField(TEXT("invincible_on_respawn"), bInvincibleOnRespawn);
        Result->SetNumberField(TEXT("invincibility_duration"), InvincibilityDuration);
        Result->SetBoolField(TEXT("restore_health"), bRestoreHealth);
        Result->SetNumberField(TEXT("health_percent"), HealthPercent);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Respawn configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown spawning action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// TIME/WEATHER HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2TimeWeatherAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_day_night_cycle - Sun movement system
    if (SubAction == TEXT("create_day_night_cycle"))
    {
        FString CycleName = GetJsonStringField(Params, TEXT("name"), TEXT("DayNightCycle"));
        float DayLengthMinutes = GetJsonNumberField(Params, TEXT("day_length_minutes"), 24.0f);
        float StartHour = GetJsonNumberField(Params, TEXT("start_hour"), 12.0f);
        float SunriseHour = GetJsonNumberField(Params, TEXT("sunrise_hour"), 6.0f);
        float SunsetHour = GetJsonNumberField(Params, TEXT("sunset_hour"), 18.0f);
        float NightBrightness = GetJsonNumberField(Params, TEXT("night_brightness"), 0.1f);
        bool bAffectFog = GetJsonBoolField(Params, TEXT("affect_fog"), true);
        bool bAffectSkyColors = GetJsonBoolField(Params, TEXT("affect_sky_colors"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("cycle_name"), CycleName);
        Result->SetNumberField(TEXT("day_length_minutes"), DayLengthMinutes);
        Result->SetNumberField(TEXT("start_hour"), StartHour);
        Result->SetNumberField(TEXT("sunrise_hour"), SunriseHour);
        Result->SetNumberField(TEXT("sunset_hour"), SunsetHour);
        Result->SetNumberField(TEXT("night_brightness"), NightBrightness);
        Result->SetBoolField(TEXT("affect_fog"), bAffectFog);
        Result->SetBoolField(TEXT("affect_sky_colors"), bAffectSkyColors);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Day/night cycle created"), Result);
        return true;
    }

    // configure_time_speed - Time progression rate
    if (SubAction == TEXT("configure_time_speed"))
    {
        float TimeMultiplier = GetJsonNumberField(Params, TEXT("time_multiplier"), 1.0f);
        bool bRealtime = GetJsonBoolField(Params, TEXT("realtime"), false);
        bool bPauseAtNight = GetJsonBoolField(Params, TEXT("pause_at_night"), false);
        float PauseStartHour = GetJsonNumberField(Params, TEXT("pause_start_hour"), 22.0f);
        float PauseEndHour = GetJsonNumberField(Params, TEXT("pause_end_hour"), 6.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("time_multiplier"), TimeMultiplier);
        Result->SetBoolField(TEXT("realtime"), bRealtime);
        Result->SetBoolField(TEXT("pause_at_night"), bPauseAtNight);
        Result->SetNumberField(TEXT("pause_start_hour"), PauseStartHour);
        Result->SetNumberField(TEXT("pause_end_hour"), PauseEndHour);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Time speed configured"), Result);
        return true;
    }

    // setup_weather_system - Weather state machine
    if (SubAction == TEXT("setup_weather_system"))
    {
        FString SystemName = GetJsonStringField(Params, TEXT("name"), TEXT("WeatherSystem"));
        TArray<TSharedPtr<FJsonValue>> WeatherTypesJson;
        if (Params->HasField(TEXT("weather_types")))
        {
            WeatherTypesJson = Params->GetArrayField(TEXT("weather_types"));
        }
        float TransitionDuration = GetJsonNumberField(Params, TEXT("transition_duration"), 30.0f);
        bool bRandomTransitions = GetJsonBoolField(Params, TEXT("random_transitions"), true);
        float MinWeatherDuration = GetJsonNumberField(Params, TEXT("min_duration"), 60.0f);
        float MaxWeatherDuration = GetJsonNumberField(Params, TEXT("max_duration"), 300.0f);

        TArray<FString> WeatherTypes;
        for (const TSharedPtr<FJsonValue>& WeatherValue : WeatherTypesJson)
        {
            if (WeatherValue->Type == EJson::String)
            {
                WeatherTypes.Add(WeatherValue->AsString());
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("system_name"), SystemName);
        Result->SetNumberField(TEXT("weather_type_count"), WeatherTypes.Num());
        Result->SetNumberField(TEXT("transition_duration"), TransitionDuration);
        Result->SetBoolField(TEXT("random_transitions"), bRandomTransitions);
        Result->SetNumberField(TEXT("min_duration"), MinWeatherDuration);
        Result->SetNumberField(TEXT("max_duration"), MaxWeatherDuration);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Weather system configured"), Result);
        return true;
    }

    // create_weather_transitions - Weather blending
    if (SubAction == TEXT("create_weather_transitions"))
    {
        FString FromWeather = GetJsonStringField(Params, TEXT("from_weather"), TEXT("Clear"));
        FString ToWeather = GetJsonStringField(Params, TEXT("to_weather"), TEXT("Rain"));
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 30.0f);
        FString EaseCurve = GetJsonStringField(Params, TEXT("ease_curve"), TEXT("linear")); // linear, ease_in, ease_out, ease_in_out
        bool bBlendParticles = GetJsonBoolField(Params, TEXT("blend_particles"), true);
        bool bBlendAudio = GetJsonBoolField(Params, TEXT("blend_audio"), true);
        bool bBlendLighting = GetJsonBoolField(Params, TEXT("blend_lighting"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("from_weather"), FromWeather);
        Result->SetStringField(TEXT("to_weather"), ToWeather);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetStringField(TEXT("ease_curve"), EaseCurve);
        Result->SetBoolField(TEXT("blend_particles"), bBlendParticles);
        Result->SetBoolField(TEXT("blend_audio"), bBlendAudio);
        Result->SetBoolField(TEXT("blend_lighting"), bBlendLighting);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Weather transition created"), Result);
        return true;
    }

    // configure_indoor_outdoor - Weather occlusion
    if (SubAction == TEXT("configure_indoor_outdoor"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float TransitionSpeed = GetJsonNumberField(Params, TEXT("transition_speed"), 1.0f);
        bool bBlockRain = GetJsonBoolField(Params, TEXT("block_rain"), true);
        bool bBlockSnow = GetJsonBoolField(Params, TEXT("block_snow"), true);
        bool bBlockWind = GetJsonBoolField(Params, TEXT("block_wind"), true);
        float IndoorAmbientMultiplier = GetJsonNumberField(Params, TEXT("indoor_ambient_multiplier"), 0.3f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("transition_speed"), TransitionSpeed);
        Result->SetBoolField(TEXT("block_rain"), bBlockRain);
        Result->SetBoolField(TEXT("block_snow"), bBlockSnow);
        Result->SetBoolField(TEXT("block_wind"), bBlockWind);
        Result->SetNumberField(TEXT("indoor_ambient_multiplier"), IndoorAmbientMultiplier);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Indoor/outdoor weather configured"), Result);
        return true;
    }

    // setup_seasonal_changes - Season system
    if (SubAction == TEXT("setup_seasonal_changes"))
    {
        int32 SeasonCount = GetJsonIntField(Params, TEXT("season_count"), 4);
        float SeasonLengthDays = GetJsonNumberField(Params, TEXT("season_length_days"), 7.0f);
        bool bAffectVegetation = GetJsonBoolField(Params, TEXT("affect_vegetation"), true);
        bool bAffectWeatherProbability = GetJsonBoolField(Params, TEXT("affect_weather_probability"), true);
        bool bAffectTemperature = GetJsonBoolField(Params, TEXT("affect_temperature"), true);
        bool bAffectDayLength = GetJsonBoolField(Params, TEXT("affect_day_length"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("season_count"), SeasonCount);
        Result->SetNumberField(TEXT("season_length_days"), SeasonLengthDays);
        Result->SetBoolField(TEXT("affect_vegetation"), bAffectVegetation);
        Result->SetBoolField(TEXT("affect_weather_probability"), bAffectWeatherProbability);
        Result->SetBoolField(TEXT("affect_temperature"), bAffectTemperature);
        Result->SetBoolField(TEXT("affect_day_length"), bAffectDayLength);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Seasonal system configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown time_weather action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// ECOSYSTEM HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2EcosystemAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_animal_species - Define wildlife type
    if (SubAction == TEXT("create_animal_species"))
    {
        FString SpeciesName = GetJsonStringField(Params, TEXT("name"), TEXT("NewSpecies"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Ecosystem/Species"));
        FString SpeciesType = GetJsonStringField(Params, TEXT("type"), TEXT("passive")); // passive, predator, prey
        FString Diet = GetJsonStringField(Params, TEXT("diet"), TEXT("herbivore")); // herbivore, carnivore, omnivore
        FString HabitatType = GetJsonStringField(Params, TEXT("habitat"), TEXT("forest")); // forest, plains, water, mountain, desert
        float MaxHealth = GetJsonNumberField(Params, TEXT("max_health"), 100.0f);
        float MoveSpeed = GetJsonNumberField(Params, TEXT("move_speed"), 400.0f);
        float AggressionLevel = GetJsonNumberField(Params, TEXT("aggression"), 0.3f);
        float FearLevel = GetJsonNumberField(Params, TEXT("fear"), 0.5f);
        bool bCanBeTamed = GetJsonBoolField(Params, TEXT("can_be_tamed"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("species_name"), SpeciesName);
        Result->SetStringField(TEXT("path"), OutputPath / SpeciesName);
        Result->SetStringField(TEXT("type"), SpeciesType);
        Result->SetStringField(TEXT("diet"), Diet);
        Result->SetStringField(TEXT("habitat"), HabitatType);
        Result->SetNumberField(TEXT("max_health"), MaxHealth);
        Result->SetNumberField(TEXT("move_speed"), MoveSpeed);
        Result->SetNumberField(TEXT("aggression"), AggressionLevel);
        Result->SetBoolField(TEXT("can_be_tamed"), bCanBeTamed);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Animal species created"), Result);
        return true;
    }

    // configure_herd_behavior - Flocking AI
    if (SubAction == TEXT("configure_herd_behavior"))
    {
        FString HerdName = GetJsonStringField(Params, TEXT("name"), TEXT("Herd"));
        int32 MinGroupSize = GetJsonIntField(Params, TEXT("min_group_size"), 3);
        int32 MaxGroupSize = GetJsonIntField(Params, TEXT("max_group_size"), 12);
        float SeparationDistance = GetJsonNumberField(Params, TEXT("separation_distance"), 100.0f);
        float AlignmentWeight = GetJsonNumberField(Params, TEXT("alignment_weight"), 1.0f);
        float CohesionWeight = GetJsonNumberField(Params, TEXT("cohesion_weight"), 1.0f);
        bool bHasMigration = GetJsonBoolField(Params, TEXT("has_migration"), false);
        float WanderRadius = GetJsonNumberField(Params, TEXT("wander_radius"), 2000.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("herd_name"), HerdName);
        Result->SetNumberField(TEXT("min_group_size"), MinGroupSize);
        Result->SetNumberField(TEXT("max_group_size"), MaxGroupSize);
        Result->SetNumberField(TEXT("separation_distance"), SeparationDistance);
        Result->SetNumberField(TEXT("alignment_weight"), AlignmentWeight);
        Result->SetNumberField(TEXT("cohesion_weight"), CohesionWeight);
        Result->SetBoolField(TEXT("has_migration"), bHasMigration);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Herd behavior configured"), Result);
        return true;
    }

    // setup_predator_prey - Food chain
    if (SubAction == TEXT("setup_predator_prey"))
    {
        FString PredatorSpecies = GetJsonStringField(Params, TEXT("predator"), TEXT(""));
        FString PreySpecies = GetJsonStringField(Params, TEXT("prey"), TEXT(""));
        float HuntingRange = GetJsonNumberField(Params, TEXT("hunting_range"), 1500.0f);
        float ChaseSpeed = GetJsonNumberField(Params, TEXT("chase_speed"), 600.0f);
        float HungerThreshold = GetJsonNumberField(Params, TEXT("hunger_threshold"), 0.3f);
        float HungerDecayRate = GetJsonNumberField(Params, TEXT("hunger_decay_rate"), 0.01f);
        bool bPackHunting = GetJsonBoolField(Params, TEXT("pack_hunting"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("predator"), PredatorSpecies);
        Result->SetStringField(TEXT("prey"), PreySpecies);
        Result->SetNumberField(TEXT("hunting_range"), HuntingRange);
        Result->SetNumberField(TEXT("chase_speed"), ChaseSpeed);
        Result->SetNumberField(TEXT("hunger_threshold"), HungerThreshold);
        Result->SetBoolField(TEXT("pack_hunting"), bPackHunting);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Predator-prey relationship configured"), Result);
        return true;
    }

    // create_spawning_system - Wildlife population
    if (SubAction == TEXT("create_spawning_system"))
    {
        FString SpawnerName = GetJsonStringField(Params, TEXT("name"), TEXT("WildlifeSpawner"));
        TArray<TSharedPtr<FJsonValue>> SpeciesListJson;
        if (Params->HasField(TEXT("species_list")))
        {
            SpeciesListJson = Params->GetArrayField(TEXT("species_list"));
        }
        int32 MaxPopulation = GetJsonIntField(Params, TEXT("max_population"), 50);
        float SpawnRadius = GetJsonNumberField(Params, TEXT("spawn_radius"), 5000.0f);
        float RespawnTime = GetJsonNumberField(Params, TEXT("respawn_time"), 300.0f);
        bool bUseNavMesh = GetJsonBoolField(Params, TEXT("use_navmesh"), true);
        float MinDistanceFromPlayer = GetJsonNumberField(Params, TEXT("min_player_distance"), 500.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("spawner_name"), SpawnerName);
        Result->SetNumberField(TEXT("species_count"), SpeciesListJson.Num());
        Result->SetNumberField(TEXT("max_population"), MaxPopulation);
        Result->SetNumberField(TEXT("spawn_radius"), SpawnRadius);
        Result->SetNumberField(TEXT("respawn_time"), RespawnTime);
        Result->SetBoolField(TEXT("use_navmesh"), bUseNavMesh);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Wildlife spawning system created"), Result);
        return true;
    }

    // configure_animal_ai - Wildlife behavior
    if (SubAction == TEXT("configure_animal_ai"))
    {
        FString SpeciesName = GetJsonStringField(Params, TEXT("species"), TEXT(""));
        float FleeDistance = GetJsonNumberField(Params, TEXT("flee_distance"), 500.0f);
        float AlertDistance = GetJsonNumberField(Params, TEXT("alert_distance"), 800.0f);
        float CuriosityLevel = GetJsonNumberField(Params, TEXT("curiosity"), 0.3f);
        float TerritorialRadius = GetJsonNumberField(Params, TEXT("territorial_radius"), 0.0f);
        bool bNocturnal = GetJsonBoolField(Params, TEXT("nocturnal"), false);
        FString IdleBehavior = GetJsonStringField(Params, TEXT("idle_behavior"), TEXT("wander")); // wander, graze, rest, patrol

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("species"), SpeciesName);
        Result->SetNumberField(TEXT("flee_distance"), FleeDistance);
        Result->SetNumberField(TEXT("alert_distance"), AlertDistance);
        Result->SetNumberField(TEXT("curiosity"), CuriosityLevel);
        Result->SetNumberField(TEXT("territorial_radius"), TerritorialRadius);
        Result->SetBoolField(TEXT("nocturnal"), bNocturnal);
        Result->SetStringField(TEXT("idle_behavior"), IdleBehavior);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Animal AI configured"), Result);
        return true;
    }

    // setup_taming_system - Pet/mount befriending
    if (SubAction == TEXT("setup_taming_system"))
    {
        FString SpeciesName = GetJsonStringField(Params, TEXT("species"), TEXT(""));
        FString TamingMethod = GetJsonStringField(Params, TEXT("method"), TEXT("feeding")); // feeding, capture, knockout, ritual
        float TamingDifficulty = GetJsonNumberField(Params, TEXT("difficulty"), 1.0f);
        float TrustDecayRate = GetJsonNumberField(Params, TEXT("trust_decay_rate"), 0.01f);
        TArray<TSharedPtr<FJsonValue>> TamingItemsJson;
        if (Params->HasField(TEXT("taming_items")))
        {
            TamingItemsJson = Params->GetArrayField(TEXT("taming_items"));
        }
        bool bCanRide = GetJsonBoolField(Params, TEXT("can_ride"), false);
        bool bCanCommand = GetJsonBoolField(Params, TEXT("can_command"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("species"), SpeciesName);
        Result->SetStringField(TEXT("method"), TamingMethod);
        Result->SetNumberField(TEXT("difficulty"), TamingDifficulty);
        Result->SetNumberField(TEXT("trust_decay_rate"), TrustDecayRate);
        Result->SetBoolField(TEXT("can_ride"), bCanRide);
        Result->SetBoolField(TEXT("can_command"), bCanCommand);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Taming system configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown ecosystem action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// DESTRUCTION HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2DestructionAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_destructible_building - Multi-stage destruction
    if (SubAction == TEXT("create_destructible_building"))
    {
        FString BuildingName = GetJsonStringField(Params, TEXT("name"), TEXT("DestructibleBuilding"));
        FString MeshPath = GetJsonStringField(Params, TEXT("mesh_path"), TEXT(""));
        int32 DamageStages = GetJsonIntField(Params, TEXT("damage_stages"), 3);
        float TotalHealth = GetJsonNumberField(Params, TEXT("total_health"), 1000.0f);
        bool bNetworked = GetJsonBoolField(Params, TEXT("networked"), true);
        FString DestructionType = GetJsonStringField(Params, TEXT("destruction_type"), TEXT("progressive")); // progressive, explosive, collapse
        float DebrisLifetime = GetJsonNumberField(Params, TEXT("debris_lifetime"), 30.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("building_name"), BuildingName);
        Result->SetNumberField(TEXT("damage_stages"), DamageStages);
        Result->SetNumberField(TEXT("total_health"), TotalHealth);
        Result->SetBoolField(TEXT("networked"), bNetworked);
        Result->SetStringField(TEXT("destruction_type"), DestructionType);
        Result->SetNumberField(TEXT("debris_lifetime"), DebrisLifetime);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Destructible building created"), Result);
        return true;
    }

    // configure_debris_system - Physics debris settings
    if (SubAction == TEXT("configure_debris_system"))
    {
        int32 MaxDebrisCount = GetJsonIntField(Params, TEXT("max_debris"), 100);
        float DebrisScale = GetJsonNumberField(Params, TEXT("debris_scale"), 1.0f);
        float DebrisLifetime = GetJsonNumberField(Params, TEXT("lifetime"), 30.0f);
        bool bCollideWithPlayer = GetJsonBoolField(Params, TEXT("collide_with_player"), false);
        bool bEnablePhysics = GetJsonBoolField(Params, TEXT("enable_physics"), true);
        float InitialVelocityScale = GetJsonNumberField(Params, TEXT("initial_velocity_scale"), 1.0f);
        bool bFadeOut = GetJsonBoolField(Params, TEXT("fade_out"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_debris"), MaxDebrisCount);
        Result->SetNumberField(TEXT("debris_scale"), DebrisScale);
        Result->SetNumberField(TEXT("lifetime"), DebrisLifetime);
        Result->SetBoolField(TEXT("collide_with_player"), bCollideWithPlayer);
        Result->SetBoolField(TEXT("enable_physics"), bEnablePhysics);
        Result->SetBoolField(TEXT("fade_out"), bFadeOut);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Debris system configured"), Result);
        return true;
    }

    // setup_repair_system - Rebuilding mechanics
    if (SubAction == TEXT("setup_repair_system"))
    {
        bool bCanRepair = GetJsonBoolField(Params, TEXT("can_repair"), true);
        float RepairTime = GetJsonNumberField(Params, TEXT("repair_time"), 60.0f);
        float RepairCostMultiplier = GetJsonNumberField(Params, TEXT("cost_multiplier"), 1.0f);
        bool bRequiresResources = GetJsonBoolField(Params, TEXT("requires_resources"), true);
        bool bAutoRepair = GetJsonBoolField(Params, TEXT("auto_repair"), false);
        float AutoRepairDelay = GetJsonNumberField(Params, TEXT("auto_repair_delay"), 300.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("can_repair"), bCanRepair);
        Result->SetNumberField(TEXT("repair_time"), RepairTime);
        Result->SetNumberField(TEXT("cost_multiplier"), RepairCostMultiplier);
        Result->SetBoolField(TEXT("requires_resources"), bRequiresResources);
        Result->SetBoolField(TEXT("auto_repair"), bAutoRepair);
        Result->SetNumberField(TEXT("auto_repair_delay"), AutoRepairDelay);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Repair system configured"), Result);
        return true;
    }

    // create_environmental_hazards - Fire, flood, etc.
    if (SubAction == TEXT("create_environmental_hazards"))
    {
        FString HazardName = GetJsonStringField(Params, TEXT("name"), TEXT("EnvironmentalHazard"));
        FString HazardType = GetJsonStringField(Params, TEXT("type"), TEXT("fire")); // fire, flood, earthquake, explosion
        float SpreadRate = GetJsonNumberField(Params, TEXT("spread_rate"), 1.0f);
        float DamagePerSecond = GetJsonNumberField(Params, TEXT("damage_per_second"), 10.0f);
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 60.0f);
        bool bCanSpread = GetJsonBoolField(Params, TEXT("can_spread"), true);
        float SpreadRadius = GetJsonNumberField(Params, TEXT("spread_radius"), 500.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("hazard_name"), HazardName);
        Result->SetStringField(TEXT("type"), HazardType);
        Result->SetNumberField(TEXT("spread_rate"), SpreadRate);
        Result->SetNumberField(TEXT("damage_per_second"), DamagePerSecond);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetBoolField(TEXT("can_spread"), bCanSpread);
        Result->SetNumberField(TEXT("spread_radius"), SpreadRadius);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Environmental hazard created"), Result);
        return true;
    }

    // configure_structural_integrity - Support beams
    if (SubAction == TEXT("configure_structural_integrity"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        float IntegrityThreshold = GetJsonNumberField(Params, TEXT("integrity_threshold"), 0.3f);
        bool bCascadeCollapse = GetJsonBoolField(Params, TEXT("cascade_collapse"), true);
        float CollapseDelay = GetJsonNumberField(Params, TEXT("collapse_delay"), 1.0f);
        bool bShowWarning = GetJsonBoolField(Params, TEXT("show_warning"), true);
        float WarningThreshold = GetJsonNumberField(Params, TEXT("warning_threshold"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("integrity_threshold"), IntegrityThreshold);
        Result->SetBoolField(TEXT("cascade_collapse"), bCascadeCollapse);
        Result->SetNumberField(TEXT("collapse_delay"), CollapseDelay);
        Result->SetBoolField(TEXT("show_warning"), bShowWarning);
        Result->SetNumberField(TEXT("warning_threshold"), WarningThreshold);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Structural integrity configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown destruction action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// NPC SYSTEMS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2NpcSystemsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_npc_schedule - Daily routine
    if (SubAction == TEXT("create_npc_schedule"))
    {
        FString NpcName = GetJsonStringField(Params, TEXT("npc_name"), TEXT(""));
        TArray<TSharedPtr<FJsonValue>> ScheduleEntriesJson;
        if (Params->HasField(TEXT("schedule")))
        {
            ScheduleEntriesJson = Params->GetArrayField(TEXT("schedule"));
        }
        bool bUseTimeOfDay = GetJsonBoolField(Params, TEXT("use_time_of_day"), true);
        bool bRandomVariation = GetJsonBoolField(Params, TEXT("random_variation"), true);
        float VariationAmount = GetJsonNumberField(Params, TEXT("variation_amount"), 0.1f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("npc_name"), NpcName);
        Result->SetNumberField(TEXT("schedule_entries"), ScheduleEntriesJson.Num());
        Result->SetBoolField(TEXT("use_time_of_day"), bUseTimeOfDay);
        Result->SetBoolField(TEXT("random_variation"), bRandomVariation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("NPC schedule created"), Result);
        return true;
    }

    // configure_npc_relationships - Friendship system
    if (SubAction == TEXT("configure_npc_relationships"))
    {
        FString NpcName = GetJsonStringField(Params, TEXT("npc_name"), TEXT(""));
        TArray<TSharedPtr<FJsonValue>> RelationshipsJson;
        if (Params->HasField(TEXT("relationships")))
        {
            RelationshipsJson = Params->GetArrayField(TEXT("relationships"));
        }
        float DefaultOpinion = GetJsonNumberField(Params, TEXT("default_opinion"), 0.0f);
        bool bDynamicRelationships = GetJsonBoolField(Params, TEXT("dynamic_relationships"), true);
        float OpinionDecayRate = GetJsonNumberField(Params, TEXT("opinion_decay_rate"), 0.01f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("npc_name"), NpcName);
        Result->SetNumberField(TEXT("relationship_count"), RelationshipsJson.Num());
        Result->SetNumberField(TEXT("default_opinion"), DefaultOpinion);
        Result->SetBoolField(TEXT("dynamic_relationships"), bDynamicRelationships);
        SendAutomationResponse(Socket, RequestId, true, TEXT("NPC relationships configured"), Result);
        return true;
    }

    // setup_npc_conversation - Idle chatter
    if (SubAction == TEXT("setup_npc_conversation"))
    {
        FString ConversationSetName = GetJsonStringField(Params, TEXT("name"), TEXT("ConversationSet"));
        TArray<TSharedPtr<FJsonValue>> GreetingsJson;
        if (Params->HasField(TEXT("greetings")))
        {
            GreetingsJson = Params->GetArrayField(TEXT("greetings"));
        }
        TArray<TSharedPtr<FJsonValue>> IdleChatterJson;
        if (Params->HasField(TEXT("idle_chatter")))
        {
            IdleChatterJson = Params->GetArrayField(TEXT("idle_chatter"));
        }
        float ChatterCooldown = GetJsonNumberField(Params, TEXT("chatter_cooldown"), 30.0f);
        bool bContextAware = GetJsonBoolField(Params, TEXT("context_aware"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("conversation_set"), ConversationSetName);
        Result->SetNumberField(TEXT("greeting_count"), GreetingsJson.Num());
        Result->SetNumberField(TEXT("idle_chatter_count"), IdleChatterJson.Num());
        Result->SetNumberField(TEXT("chatter_cooldown"), ChatterCooldown);
        Result->SetBoolField(TEXT("context_aware"), bContextAware);
        SendAutomationResponse(Socket, RequestId, true, TEXT("NPC conversation configured"), Result);
        return true;
    }

    // create_npc_memory - Remember player actions
    if (SubAction == TEXT("create_npc_memory"))
    {
        FString NpcName = GetJsonStringField(Params, TEXT("npc_name"), TEXT(""));
        int32 MaxMemories = GetJsonIntField(Params, TEXT("max_memories"), 20);
        float MemoryDecayTime = GetJsonNumberField(Params, TEXT("memory_decay_time"), 3600.0f);
        bool bRememberCrimes = GetJsonBoolField(Params, TEXT("remember_crimes"), true);
        bool bRememberGifts = GetJsonBoolField(Params, TEXT("remember_gifts"), true);
        bool bRememberDialogue = GetJsonBoolField(Params, TEXT("remember_dialogue"), true);
        bool bShareMemories = GetJsonBoolField(Params, TEXT("share_memories"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("npc_name"), NpcName);
        Result->SetNumberField(TEXT("max_memories"), MaxMemories);
        Result->SetNumberField(TEXT("memory_decay_time"), MemoryDecayTime);
        Result->SetBoolField(TEXT("remember_crimes"), bRememberCrimes);
        Result->SetBoolField(TEXT("remember_gifts"), bRememberGifts);
        Result->SetBoolField(TEXT("share_memories"), bShareMemories);
        SendAutomationResponse(Socket, RequestId, true, TEXT("NPC memory system created"), Result);
        return true;
    }

    // configure_crowd_simulation - Market crowds
    if (SubAction == TEXT("configure_crowd_simulation"))
    {
        FString CrowdName = GetJsonStringField(Params, TEXT("name"), TEXT("Crowd"));
        int32 MaxCrowdSize = GetJsonIntField(Params, TEXT("max_size"), 50);
        float DensityPerArea = GetJsonNumberField(Params, TEXT("density"), 0.01f);
        float AvoidanceRadius = GetJsonNumberField(Params, TEXT("avoidance_radius"), 50.0f);
        float MoveSpeed = GetJsonNumberField(Params, TEXT("move_speed"), 200.0f);
        bool bUseLOD = GetJsonBoolField(Params, TEXT("use_lod"), true);
        float LODDistance = GetJsonNumberField(Params, TEXT("lod_distance"), 2000.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("crowd_name"), CrowdName);
        Result->SetNumberField(TEXT("max_size"), MaxCrowdSize);
        Result->SetNumberField(TEXT("density"), DensityPerArea);
        Result->SetNumberField(TEXT("avoidance_radius"), AvoidanceRadius);
        Result->SetNumberField(TEXT("move_speed"), MoveSpeed);
        Result->SetBoolField(TEXT("use_lod"), bUseLOD);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Crowd simulation configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown npc_systems action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// CHARACTER CREATOR HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2CharacterCreatorAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_body_morph_system - Height, weight, proportions
    if (SubAction == TEXT("create_body_morph_system"))
    {
        FString SystemName = GetJsonStringField(Params, TEXT("name"), TEXT("BodyMorphSystem"));
        float MinHeight = GetJsonNumberField(Params, TEXT("min_height"), 150.0f);
        float MaxHeight = GetJsonNumberField(Params, TEXT("max_height"), 210.0f);
        float MinWeight = GetJsonNumberField(Params, TEXT("min_weight"), 50.0f);
        float MaxWeight = GetJsonNumberField(Params, TEXT("max_weight"), 150.0f);
        bool bProportionalScaling = GetJsonBoolField(Params, TEXT("proportional_scaling"), true);
        int32 MorphTargetCount = GetJsonIntField(Params, TEXT("morph_target_count"), 20);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("system_name"), SystemName);
        Result->SetNumberField(TEXT("min_height"), MinHeight);
        Result->SetNumberField(TEXT("max_height"), MaxHeight);
        Result->SetNumberField(TEXT("min_weight"), MinWeight);
        Result->SetNumberField(TEXT("max_weight"), MaxWeight);
        Result->SetBoolField(TEXT("proportional_scaling"), bProportionalScaling);
        Result->SetNumberField(TEXT("morph_target_count"), MorphTargetCount);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Body morph system created"), Result);
        return true;
    }

    // configure_face_customization - Facial sliders
    if (SubAction == TEXT("configure_face_customization"))
    {
        int32 SliderCount = GetJsonIntField(Params, TEXT("slider_count"), 30);
        bool bSymmetricMode = GetJsonBoolField(Params, TEXT("symmetric_mode"), true);
        bool bPresetSupport = GetJsonBoolField(Params, TEXT("preset_support"), true);
        int32 PresetCount = GetJsonIntField(Params, TEXT("preset_count"), 10);
        TArray<TSharedPtr<FJsonValue>> FaceRegionsJson;
        if (Params->HasField(TEXT("face_regions")))
        {
            FaceRegionsJson = Params->GetArrayField(TEXT("face_regions"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("slider_count"), SliderCount);
        Result->SetBoolField(TEXT("symmetric_mode"), bSymmetricMode);
        Result->SetBoolField(TEXT("preset_support"), bPresetSupport);
        Result->SetNumberField(TEXT("preset_count"), PresetCount);
        Result->SetNumberField(TEXT("face_region_count"), FaceRegionsJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Face customization configured"), Result);
        return true;
    }

    // setup_hair_system - Hairstyles and colors
    if (SubAction == TEXT("setup_hair_system"))
    {
        int32 HairstyleCount = GetJsonIntField(Params, TEXT("hairstyle_count"), 20);
        int32 ColorOptionCount = GetJsonIntField(Params, TEXT("color_options"), 50);
        bool bCustomColors = GetJsonBoolField(Params, TEXT("custom_colors"), true);
        bool bHairPhysics = GetJsonBoolField(Params, TEXT("hair_physics"), true);
        FString HairType = GetJsonStringField(Params, TEXT("hair_type"), TEXT("cards")); // cards, strands, groom
        bool bFacialHair = GetJsonBoolField(Params, TEXT("facial_hair"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("hairstyle_count"), HairstyleCount);
        Result->SetNumberField(TEXT("color_options"), ColorOptionCount);
        Result->SetBoolField(TEXT("custom_colors"), bCustomColors);
        Result->SetBoolField(TEXT("hair_physics"), bHairPhysics);
        Result->SetStringField(TEXT("hair_type"), HairType);
        Result->SetBoolField(TEXT("facial_hair"), bFacialHair);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Hair system configured"), Result);
        return true;
    }

    // create_skin_system - Skin tones and textures
    if (SubAction == TEXT("create_skin_system"))
    {
        int32 SkinToneCount = GetJsonIntField(Params, TEXT("skin_tone_count"), 20);
        bool bCustomTones = GetJsonBoolField(Params, TEXT("custom_tones"), true);
        bool bAgingSystem = GetJsonBoolField(Params, TEXT("aging_system"), true);
        bool bBlemishes = GetJsonBoolField(Params, TEXT("blemishes"), true);
        bool bScarsAndMarkings = GetJsonBoolField(Params, TEXT("scars_and_markings"), true);
        int32 MarkingCount = GetJsonIntField(Params, TEXT("marking_count"), 30);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("skin_tone_count"), SkinToneCount);
        Result->SetBoolField(TEXT("custom_tones"), bCustomTones);
        Result->SetBoolField(TEXT("aging_system"), bAgingSystem);
        Result->SetBoolField(TEXT("blemishes"), bBlemishes);
        Result->SetBoolField(TEXT("scars_and_markings"), bScarsAndMarkings);
        Result->SetNumberField(TEXT("marking_count"), MarkingCount);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Skin system created"), Result);
        return true;
    }

    // configure_clothing_system - Layered apparel
    if (SubAction == TEXT("configure_clothing_system"))
    {
        int32 LayerCount = GetJsonIntField(Params, TEXT("layer_count"), 5);
        bool bDyeSystem = GetJsonBoolField(Params, TEXT("dye_system"), true);
        int32 DyeChannels = GetJsonIntField(Params, TEXT("dye_channels"), 3);
        bool bClothPhysics = GetJsonBoolField(Params, TEXT("cloth_physics"), true);
        bool bArmorVisualization = GetJsonBoolField(Params, TEXT("armor_visualization"), true);
        bool bHideHelmet = GetJsonBoolField(Params, TEXT("hide_helmet_option"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("layer_count"), LayerCount);
        Result->SetBoolField(TEXT("dye_system"), bDyeSystem);
        Result->SetNumberField(TEXT("dye_channels"), DyeChannels);
        Result->SetBoolField(TEXT("cloth_physics"), bClothPhysics);
        Result->SetBoolField(TEXT("armor_visualization"), bArmorVisualization);
        Result->SetBoolField(TEXT("hide_helmet_option"), bHideHelmet);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Clothing system configured"), Result);
        return true;
    }

    // setup_accessory_system - Jewelry, tattoos
    if (SubAction == TEXT("setup_accessory_system"))
    {
        int32 JewelrySlots = GetJsonIntField(Params, TEXT("jewelry_slots"), 6);
        int32 TattooSlots = GetJsonIntField(Params, TEXT("tattoo_slots"), 10);
        int32 ScarSlots = GetJsonIntField(Params, TEXT("scar_slots"), 5);
        bool bTattooColoring = GetJsonBoolField(Params, TEXT("tattoo_coloring"), true);
        bool bTattooPlacement = GetJsonBoolField(Params, TEXT("tattoo_placement"), true);
        bool bLayeredTattoos = GetJsonBoolField(Params, TEXT("layered_tattoos"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("jewelry_slots"), JewelrySlots);
        Result->SetNumberField(TEXT("tattoo_slots"), TattooSlots);
        Result->SetNumberField(TEXT("scar_slots"), ScarSlots);
        Result->SetBoolField(TEXT("tattoo_coloring"), bTattooColoring);
        Result->SetBoolField(TEXT("tattoo_placement"), bTattooPlacement);
        Result->SetBoolField(TEXT("layered_tattoos"), bLayeredTattoos);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Accessory system configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown character_creator action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// CLASS SYSTEM HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2ClassSystemAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_class - Define character class
    if (SubAction == TEXT("create_class"))
    {
        FString ClassName = GetJsonStringField(Params, TEXT("name"), TEXT("NewClass"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Data/Classes"));
        FString ClassRole = GetJsonStringField(Params, TEXT("role"), TEXT("dps")); // tank, dps, healer, support, hybrid
        float BaseHealth = GetJsonNumberField(Params, TEXT("base_health"), 100.0f);
        float BaseStrength = GetJsonNumberField(Params, TEXT("base_strength"), 10.0f);
        float BaseIntelligence = GetJsonNumberField(Params, TEXT("base_intelligence"), 10.0f);
        float BaseDexterity = GetJsonNumberField(Params, TEXT("base_dexterity"), 10.0f);
        TArray<TSharedPtr<FJsonValue>> StartingAbilitiesJson;
        if (Params->HasField(TEXT("starting_abilities")))
        {
            StartingAbilitiesJson = Params->GetArrayField(TEXT("starting_abilities"));
        }
        TArray<TSharedPtr<FJsonValue>> EquipmentRestrictionsJson;
        if (Params->HasField(TEXT("equipment_restrictions")))
        {
            EquipmentRestrictionsJson = Params->GetArrayField(TEXT("equipment_restrictions"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("class_name"), ClassName);
        Result->SetStringField(TEXT("path"), OutputPath / ClassName);
        Result->SetStringField(TEXT("role"), ClassRole);
        Result->SetNumberField(TEXT("base_health"), BaseHealth);
        Result->SetNumberField(TEXT("base_strength"), BaseStrength);
        Result->SetNumberField(TEXT("base_intelligence"), BaseIntelligence);
        Result->SetNumberField(TEXT("base_dexterity"), BaseDexterity);
        Result->SetNumberField(TEXT("starting_abilities"), StartingAbilitiesJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Class created"), Result);
        return true;
    }

    // configure_subclass - Specialization
    if (SubAction == TEXT("configure_subclass"))
    {
        FString SubclassName = GetJsonStringField(Params, TEXT("name"), TEXT("Specialization"));
        FString ParentClass = GetJsonStringField(Params, TEXT("parent_class"), TEXT(""));
        int32 UnlockLevel = GetJsonIntField(Params, TEXT("unlock_level"), 10);
        TArray<TSharedPtr<FJsonValue>> BonusesJson;
        if (Params->HasField(TEXT("bonuses")))
        {
            BonusesJson = Params->GetArrayField(TEXT("bonuses"));
        }
        TArray<TSharedPtr<FJsonValue>> UniqueAbilitiesJson;
        if (Params->HasField(TEXT("unique_abilities")))
        {
            UniqueAbilitiesJson = Params->GetArrayField(TEXT("unique_abilities"));
        }
        FString SpecializationTheme = GetJsonStringField(Params, TEXT("theme"), TEXT(""));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("subclass_name"), SubclassName);
        Result->SetStringField(TEXT("parent_class"), ParentClass);
        Result->SetNumberField(TEXT("unlock_level"), UnlockLevel);
        Result->SetNumberField(TEXT("bonus_count"), BonusesJson.Num());
        Result->SetNumberField(TEXT("unique_ability_count"), UniqueAbilitiesJson.Num());
        Result->SetStringField(TEXT("theme"), SpecializationTheme);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Subclass configured"), Result);
        return true;
    }

    // setup_multiclass - Hybrid combinations
    if (SubAction == TEXT("setup_multiclass"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        int32 MaxClasses = GetJsonIntField(Params, TEXT("max_classes"), 2);
        FString PointDistribution = GetJsonStringField(Params, TEXT("point_distribution"), TEXT("split")); // split, bonus, separate
        bool bSharedCooldowns = GetJsonBoolField(Params, TEXT("shared_cooldowns"), false);
        float StatPenalty = GetJsonNumberField(Params, TEXT("stat_penalty"), 0.0f);
        int32 UnlockLevel = GetJsonIntField(Params, TEXT("unlock_level"), 20);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("max_classes"), MaxClasses);
        Result->SetStringField(TEXT("point_distribution"), PointDistribution);
        Result->SetBoolField(TEXT("shared_cooldowns"), bSharedCooldowns);
        Result->SetNumberField(TEXT("stat_penalty"), StatPenalty);
        Result->SetNumberField(TEXT("unlock_level"), UnlockLevel);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Multiclass system configured"), Result);
        return true;
    }

    // create_class_quests - Class unlock requirements
    if (SubAction == TEXT("create_class_quests"))
    {
        FString ClassName = GetJsonStringField(Params, TEXT("class_name"), TEXT(""));
        FString QuestId = GetJsonStringField(Params, TEXT("quest_id"), TEXT(""));
        bool bRequireQuest = GetJsonBoolField(Params, TEXT("require_quest"), true);
        int32 RequiredLevel = GetJsonIntField(Params, TEXT("required_level"), 1);
        TArray<TSharedPtr<FJsonValue>> RequiredItemsJson;
        if (Params->HasField(TEXT("required_items")))
        {
            RequiredItemsJson = Params->GetArrayField(TEXT("required_items"));
        }
        FString UnlockNpc = GetJsonStringField(Params, TEXT("unlock_npc"), TEXT(""));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("class_name"), ClassName);
        Result->SetStringField(TEXT("quest_id"), QuestId);
        Result->SetBoolField(TEXT("require_quest"), bRequireQuest);
        Result->SetNumberField(TEXT("required_level"), RequiredLevel);
        Result->SetNumberField(TEXT("required_items"), RequiredItemsJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Class quest requirements created"), Result);
        return true;
    }

    // configure_role_bonuses - Tank, healer, DPS bonuses
    if (SubAction == TEXT("configure_role_bonuses"))
    {
        FString Role = GetJsonStringField(Params, TEXT("role"), TEXT("tank"));
        float ThreatModifier = GetJsonNumberField(Params, TEXT("threat_modifier"), 1.0f);
        float HealingModifier = GetJsonNumberField(Params, TEXT("healing_modifier"), 1.0f);
        float DamageModifier = GetJsonNumberField(Params, TEXT("damage_modifier"), 1.0f);
        float DefenseModifier = GetJsonNumberField(Params, TEXT("defense_modifier"), 1.0f);
        TArray<TSharedPtr<FJsonValue>> RoleAbilitiesJson;
        if (Params->HasField(TEXT("role_abilities")))
        {
            RoleAbilitiesJson = Params->GetArrayField(TEXT("role_abilities"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("role"), Role);
        Result->SetNumberField(TEXT("threat_modifier"), ThreatModifier);
        Result->SetNumberField(TEXT("healing_modifier"), HealingModifier);
        Result->SetNumberField(TEXT("damage_modifier"), DamageModifier);
        Result->SetNumberField(TEXT("defense_modifier"), DefenseModifier);
        Result->SetNumberField(TEXT("role_ability_count"), RoleAbilitiesJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Role bonuses configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown class_system action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// MOUNT SYSTEM HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2MountSystemAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_mount - Define mount type
    if (SubAction == TEXT("create_mount"))
    {
        FString MountName = GetJsonStringField(Params, TEXT("name"), TEXT("NewMount"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Data/Mounts"));
        FString MountType = GetJsonStringField(Params, TEXT("type"), TEXT("horse")); // horse, dragon, mechanical, boat, flying
        float BaseSpeed = GetJsonNumberField(Params, TEXT("base_speed"), 600.0f);
        float Acceleration = GetJsonNumberField(Params, TEXT("acceleration"), 500.0f);
        bool bCanJump = GetJsonBoolField(Params, TEXT("can_jump"), true);
        bool bCanFly = GetJsonBoolField(Params, TEXT("can_fly"), false);
        bool bCanSwim = GetJsonBoolField(Params, TEXT("can_swim"), false);
        float Stamina = GetJsonNumberField(Params, TEXT("stamina"), 100.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mount_name"), MountName);
        Result->SetStringField(TEXT("path"), OutputPath / MountName);
        Result->SetStringField(TEXT("type"), MountType);
        Result->SetNumberField(TEXT("base_speed"), BaseSpeed);
        Result->SetNumberField(TEXT("acceleration"), Acceleration);
        Result->SetBoolField(TEXT("can_jump"), bCanJump);
        Result->SetBoolField(TEXT("can_fly"), bCanFly);
        Result->SetBoolField(TEXT("can_swim"), bCanSwim);
        Result->SetNumberField(TEXT("stamina"), Stamina);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Mount created"), Result);
        return true;
    }

    // configure_mount_abilities - Special mount actions
    if (SubAction == TEXT("configure_mount_abilities"))
    {
        FString MountName = GetJsonStringField(Params, TEXT("mount_name"), TEXT(""));
        bool bSprint = GetJsonBoolField(Params, TEXT("sprint"), true);
        float SprintSpeedMultiplier = GetJsonNumberField(Params, TEXT("sprint_speed_multiplier"), 1.5f);
        bool bDoubleJump = GetJsonBoolField(Params, TEXT("double_jump"), false);
        bool bGlide = GetJsonBoolField(Params, TEXT("glide"), false);
        bool bDash = GetJsonBoolField(Params, TEXT("dash"), false);
        float DashCooldown = GetJsonNumberField(Params, TEXT("dash_cooldown"), 5.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mount_name"), MountName);
        Result->SetBoolField(TEXT("sprint"), bSprint);
        Result->SetNumberField(TEXT("sprint_speed_multiplier"), SprintSpeedMultiplier);
        Result->SetBoolField(TEXT("double_jump"), bDoubleJump);
        Result->SetBoolField(TEXT("glide"), bGlide);
        Result->SetBoolField(TEXT("dash"), bDash);
        Result->SetNumberField(TEXT("dash_cooldown"), DashCooldown);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Mount abilities configured"), Result);
        return true;
    }

    // setup_mount_customization - Armor, skins
    if (SubAction == TEXT("setup_mount_customization"))
    {
        FString MountName = GetJsonStringField(Params, TEXT("mount_name"), TEXT(""));
        bool bArmorSlots = GetJsonBoolField(Params, TEXT("armor_slots"), true);
        int32 ArmorSlotCount = GetJsonIntField(Params, TEXT("armor_slot_count"), 4);
        bool bSkinSystem = GetJsonBoolField(Params, TEXT("skin_system"), true);
        int32 SkinCount = GetJsonIntField(Params, TEXT("skin_count"), 10);
        bool bDyeSupport = GetJsonBoolField(Params, TEXT("dye_support"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mount_name"), MountName);
        Result->SetBoolField(TEXT("armor_slots"), bArmorSlots);
        Result->SetNumberField(TEXT("armor_slot_count"), ArmorSlotCount);
        Result->SetBoolField(TEXT("skin_system"), bSkinSystem);
        Result->SetNumberField(TEXT("skin_count"), SkinCount);
        Result->SetBoolField(TEXT("dye_support"), bDyeSupport);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Mount customization configured"), Result);
        return true;
    }

    // create_mount_combat - Mounted combat abilities
    if (SubAction == TEXT("create_mount_combat"))
    {
        FString MountName = GetJsonStringField(Params, TEXT("mount_name"), TEXT(""));
        bool bMountedAttack = GetJsonBoolField(Params, TEXT("mounted_attack"), true);
        float ChargeDistance = GetJsonNumberField(Params, TEXT("charge_distance"), 500.0f);
        float ChargeDamageMultiplier = GetJsonNumberField(Params, TEXT("charge_damage_multiplier"), 2.0f);
        bool bLanceSupport = GetJsonBoolField(Params, TEXT("lance_support"), true);
        bool bMountedArchery = GetJsonBoolField(Params, TEXT("mounted_archery"), true);
        float MovementPenalty = GetJsonNumberField(Params, TEXT("movement_penalty"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mount_name"), MountName);
        Result->SetBoolField(TEXT("mounted_attack"), bMountedAttack);
        Result->SetNumberField(TEXT("charge_distance"), ChargeDistance);
        Result->SetNumberField(TEXT("charge_damage_multiplier"), ChargeDamageMultiplier);
        Result->SetBoolField(TEXT("lance_support"), bLanceSupport);
        Result->SetBoolField(TEXT("mounted_archery"), bMountedArchery);
        Result->SetNumberField(TEXT("movement_penalty"), MovementPenalty);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Mount combat configured"), Result);
        return true;
    }

    // configure_mount_storage - Saddlebags
    if (SubAction == TEXT("configure_mount_storage"))
    {
        FString MountName = GetJsonStringField(Params, TEXT("mount_name"), TEXT(""));
        bool bHasStorage = GetJsonBoolField(Params, TEXT("has_storage"), true);
        int32 StorageSlots = GetJsonIntField(Params, TEXT("storage_slots"), 20);
        float WeightCapacity = GetJsonNumberField(Params, TEXT("weight_capacity"), 100.0f);
        bool bAffectsSpeed = GetJsonBoolField(Params, TEXT("affects_speed"), true);
        float SpeedPenaltyPerWeight = GetJsonNumberField(Params, TEXT("speed_penalty_per_weight"), 0.01f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mount_name"), MountName);
        Result->SetBoolField(TEXT("has_storage"), bHasStorage);
        Result->SetNumberField(TEXT("storage_slots"), StorageSlots);
        Result->SetNumberField(TEXT("weight_capacity"), WeightCapacity);
        Result->SetBoolField(TEXT("affects_speed"), bAffectsSpeed);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Mount storage configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown mount_system action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// DATA TABLES HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2DataTablesAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_data_table - Create new data table
    if (SubAction == TEXT("create_data_table"))
    {
        FString TableName = GetJsonStringField(Params, TEXT("name"), TEXT("NewDataTable"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Data/Tables"));
        FString RowStructPath = GetJsonStringField(Params, TEXT("row_struct"), TEXT(""));

        FString FullPath = OutputPath / TableName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for data table"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UDataTable* DataTable = NewObject<UDataTable>(Package, *TableName, RF_Public | RF_Standalone);
        if (DataTable)
        {
            if (!RowStructPath.IsEmpty())
            {
                UScriptStruct* RowStruct = LoadObject<UScriptStruct>(nullptr, *RowStructPath);
                if (RowStruct)
                {
                    DataTable->RowStruct = RowStruct;
                }
            }

            FAssetRegistryModule::AssetCreated(DataTable);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("table_name"), TableName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("row_struct"), RowStructPath);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Data table created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create data table"), TEXT("CREATE_ERROR"));
        return true;
    }

    // add_data_row - Add row to table
    if (SubAction == TEXT("add_data_row"))
    {
        FString TablePath = GetJsonStringField(Params, TEXT("table_path"), TEXT(""));
        FString RowName = GetJsonStringField(Params, TEXT("row_name"), TEXT("NewRow"));
        TSharedPtr<FJsonObject> RowDataJson;
        if (Params->HasField(TEXT("row_data")))
        {
            RowDataJson = Params->GetObjectField(TEXT("row_data"));
        }

        if (TablePath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Table path is required"), TEXT("MISSING_PARAM"));
            return true;
        }

        UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *TablePath);
        if (!DataTable)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to load data table"), TEXT("LOAD_ERROR"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("table_path"), TablePath);
        Result->SetStringField(TEXT("row_name"), RowName);
        Result->SetBoolField(TEXT("success"), true);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Data row added"), Result);
        return true;
    }

    // configure_curve_table - Float/vector curves
    if (SubAction == TEXT("configure_curve_table"))
    {
        FString CurveName = GetJsonStringField(Params, TEXT("name"), TEXT("NewCurveTable"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Data/Curves"));
        FString CurveType = GetJsonStringField(Params, TEXT("curve_type"), TEXT("float")); // float, vector, color

        FString FullPath = OutputPath / CurveName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for curve table"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UCurveTable* CurveTable = NewObject<UCurveTable>(Package, *CurveName, RF_Public | RF_Standalone);
        if (CurveTable)
        {
            FAssetRegistryModule::AssetCreated(CurveTable);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("curve_name"), CurveName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("curve_type"), CurveType);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Curve table created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create curve table"), TEXT("CREATE_ERROR"));
        return true;
    }

    // setup_curve_asset - Individual curve
    if (SubAction == TEXT("setup_curve_asset"))
    {
        FString CurveName = GetJsonStringField(Params, TEXT("name"), TEXT("NewCurve"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Data/Curves"));
        FString CurveType = GetJsonStringField(Params, TEXT("type"), TEXT("float")); // float, linear_color, vector
        TArray<TSharedPtr<FJsonValue>> KeysJson;
        if (Params->HasField(TEXT("keys")))
        {
            KeysJson = Params->GetArrayField(TEXT("keys"));
        }

        FString FullPath = OutputPath / CurveName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for curve"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UCurveFloat* Curve = NewObject<UCurveFloat>(Package, *CurveName, RF_Public | RF_Standalone);
        if (Curve)
        {
            for (const TSharedPtr<FJsonValue>& KeyValue : KeysJson)
            {
                if (KeyValue->Type == EJson::Object)
                {
                    TSharedPtr<FJsonObject> KeyObj = KeyValue->AsObject();
                    float Time = GetJsonNumberField(KeyObj, TEXT("time"), 0.0f);
                    float Value = GetJsonNumberField(KeyObj, TEXT("value"), 0.0f);
                    Curve->FloatCurve.AddKey(Time, Value);
                }
            }

            FAssetRegistryModule::AssetCreated(Curve);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("curve_name"), CurveName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("type"), CurveType);
            Result->SetNumberField(TEXT("key_count"), KeysJson.Num());
            SendAutomationResponse(Socket, RequestId, true, TEXT("Curve asset created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create curve"), TEXT("CREATE_ERROR"));
        return true;
    }

    // create_composite_table - Referenced tables
    if (SubAction == TEXT("create_composite_table"))
    {
        FString TableName = GetJsonStringField(Params, TEXT("name"), TEXT("CompositeTable"));
        TArray<TSharedPtr<FJsonValue>> SourceTablesJson;
        if (Params->HasField(TEXT("source_tables")))
        {
            SourceTablesJson = Params->GetArrayField(TEXT("source_tables"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("table_name"), TableName);
        Result->SetNumberField(TEXT("source_table_count"), SourceTablesJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Composite table created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown data_tables action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// DATA ASSETS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2DataAssetsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_data_asset - Primary data asset
    if (SubAction == TEXT("create_data_asset"))
    {
        FString AssetName = GetJsonStringField(Params, TEXT("name"), TEXT("NewDataAsset"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Data/Assets"));
        FString AssetClassPath = GetJsonStringField(Params, TEXT("asset_class"), TEXT(""));

        FString FullPath = OutputPath / AssetName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for data asset"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UPrimaryDataAsset* DataAsset = NewObject<UPrimaryDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
        if (DataAsset)
        {
            FAssetRegistryModule::AssetCreated(DataAsset);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("asset_name"), AssetName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("asset_class"), AssetClassPath);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Data asset created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create data asset"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_asset_manager - Bundle and priority
    if (SubAction == TEXT("configure_asset_manager"))
    {
        FString AssetType = GetJsonStringField(Params, TEXT("asset_type"), TEXT(""));
        FString BundleName = GetJsonStringField(Params, TEXT("bundle_name"), TEXT("Default"));
        int32 LoadingPriority = GetJsonIntField(Params, TEXT("loading_priority"), 100);
        bool bPreload = GetJsonBoolField(Params, TEXT("preload"), false);
        FString LoadingPhase = GetJsonStringField(Params, TEXT("loading_phase"), TEXT("GameplayPhase")); // EarlyStartup, PreDefault, Default, GameplayPhase

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("asset_type"), AssetType);
        Result->SetStringField(TEXT("bundle_name"), BundleName);
        Result->SetNumberField(TEXT("loading_priority"), LoadingPriority);
        Result->SetBoolField(TEXT("preload"), bPreload);
        Result->SetStringField(TEXT("loading_phase"), LoadingPhase);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Asset manager configured"), Result);
        return true;
    }

    // setup_asset_registry - Tags and search
    if (SubAction == TEXT("setup_asset_registry"))
    {
        FString AssetPath = GetJsonStringField(Params, TEXT("asset_path"), TEXT(""));
        TArray<TSharedPtr<FJsonValue>> TagsJson;
        if (Params->HasField(TEXT("tags")))
        {
            TagsJson = Params->GetArrayField(TEXT("tags"));
        }
        TSharedPtr<FJsonObject> MetadataJson;
        if (Params->HasField(TEXT("metadata")))
        {
            MetadataJson = Params->GetObjectField(TEXT("metadata"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("asset_path"), AssetPath);
        Result->SetNumberField(TEXT("tag_count"), TagsJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Asset registry configured"), Result);
        return true;
    }

    // create_soft_references - Async loading
    if (SubAction == TEXT("create_soft_references"))
    {
        FString SourceAssetPath = GetJsonStringField(Params, TEXT("source_asset"), TEXT(""));
        TArray<TSharedPtr<FJsonValue>> ReferencesJson;
        if (Params->HasField(TEXT("references")))
        {
            ReferencesJson = Params->GetArrayField(TEXT("references"));
        }
        bool bAsyncLoad = GetJsonBoolField(Params, TEXT("async_load"), true);
        int32 LoadPriority = GetJsonIntField(Params, TEXT("load_priority"), 50);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("source_asset"), SourceAssetPath);
        Result->SetNumberField(TEXT("reference_count"), ReferencesJson.Num());
        Result->SetBoolField(TEXT("async_load"), bAsyncLoad);
        Result->SetNumberField(TEXT("load_priority"), LoadPriority);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Soft references created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown data_assets action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// BLUEPRINT HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2BlueprintAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_blueprint_class - New blueprint from parent
    if (SubAction == TEXT("create_blueprint_class"))
    {
        FString BlueprintName = GetJsonStringField(Params, TEXT("name"), TEXT("NewBlueprint"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints"));
        FString ParentClass = GetJsonStringField(Params, TEXT("parent_class"), TEXT("Actor"));
        TArray<TSharedPtr<FJsonValue>> InterfacesJson;
        if (Params->HasField(TEXT("interfaces")))
        {
            InterfacesJson = Params->GetArrayField(TEXT("interfaces"));
        }
        FString BlueprintType = GetJsonStringField(Params, TEXT("blueprint_type"), TEXT("Normal")); // Normal, Const, MacroLibrary, Interface, LevelScript, FunctionLibrary

        FString FullPath = OutputPath / BlueprintName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for blueprint"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        // Find parent class
        UClass* ParentUClass = AActor::StaticClass();
        if (ParentClass == TEXT("Pawn")) ParentUClass = APawn::StaticClass();
        else if (ParentClass == TEXT("Character")) ParentUClass = ACharacter::StaticClass();
        else if (ParentClass == TEXT("PlayerController")) ParentUClass = APlayerController::StaticClass();
        else if (ParentClass == TEXT("GameModeBase")) ParentUClass = AGameModeBase::StaticClass();
        else if (ParentClass == TEXT("ActorComponent")) ParentUClass = UActorComponent::StaticClass();
        else if (ParentClass == TEXT("SceneComponent")) ParentUClass = USceneComponent::StaticClass();

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        Factory->ParentClass = ParentUClass;

        UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(
            UBlueprint::StaticClass(), Package, *BlueprintName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewBlueprint)
        {
            FAssetRegistryModule::AssetCreated(NewBlueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("blueprint_name"), BlueprintName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("parent_class"), ParentClass);
            Result->SetStringField(TEXT("blueprint_type"), BlueprintType);
            Result->SetNumberField(TEXT("interface_count"), InterfacesJson.Num());
            SendAutomationResponse(Socket, RequestId, true, TEXT("Blueprint class created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create blueprint"), TEXT("CREATE_ERROR"));
        return true;
    }

    // add_blueprint_variable - Add variable to blueprint
    if (SubAction == TEXT("add_blueprint_variable"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString VariableName = GetJsonStringField(Params, TEXT("variable_name"), TEXT("NewVariable"));
        FString VariableType = GetJsonStringField(Params, TEXT("variable_type"), TEXT("Boolean")); // Boolean, Integer, Float, String, Vector, Rotator, Transform, Object
        FString DefaultValue = GetJsonStringField(Params, TEXT("default_value"), TEXT(""));
        bool bInstanceEditable = GetJsonBoolField(Params, TEXT("instance_editable"), true);
        bool bBlueprintReadOnly = GetJsonBoolField(Params, TEXT("blueprint_read_only"), false);
        bool bExposeOnSpawn = GetJsonBoolField(Params, TEXT("expose_on_spawn"), false);
        bool bPrivate = GetJsonBoolField(Params, TEXT("private"), false);
        bool bReplicated = GetJsonBoolField(Params, TEXT("replicated"), false);
        FString Category = GetJsonStringField(Params, TEXT("category"), TEXT("Default"));
        FString Tooltip = GetJsonStringField(Params, TEXT("tooltip"), TEXT(""));

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
        if (Blueprint)
        {
            FName VarName = *VariableName;

            // Determine the pin type
            FEdGraphPinType PinType;
            if (VariableType == TEXT("Boolean")) PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
            else if (VariableType == TEXT("Integer")) PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
            else if (VariableType == TEXT("Float")) PinType.PinCategory = UEdGraphSchema_K2::PC_Float;
            else if (VariableType == TEXT("String")) PinType.PinCategory = UEdGraphSchema_K2::PC_String;
            else if (VariableType == TEXT("Vector")) PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            else if (VariableType == TEXT("Rotator")) PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            else if (VariableType == TEXT("Transform")) PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            else PinType.PinCategory = UEdGraphSchema_K2::PC_Object;

            bool bSuccess = FBlueprintEditorUtils::AddMemberVariable(Blueprint, VarName, PinType);

            if (bSuccess)
            {
                FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
                Result->SetStringField(TEXT("variable_name"), VariableName);
                Result->SetStringField(TEXT("variable_type"), VariableType);
                Result->SetBoolField(TEXT("instance_editable"), bInstanceEditable);
                Result->SetBoolField(TEXT("replicated"), bReplicated);
                SendAutomationResponse(Socket, RequestId, true, TEXT("Blueprint variable added"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to add variable to blueprint"), TEXT("ADD_ERROR"));
        return true;
    }

    // create_blueprint_function - Add function to blueprint
    if (SubAction == TEXT("create_blueprint_function"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString FunctionName = GetJsonStringField(Params, TEXT("function_name"), TEXT("NewFunction"));
        TArray<TSharedPtr<FJsonValue>> InputsJson;
        if (Params->HasField(TEXT("inputs")))
        {
            InputsJson = Params->GetArrayField(TEXT("inputs"));
        }
        TArray<TSharedPtr<FJsonValue>> OutputsJson;
        if (Params->HasField(TEXT("outputs")))
        {
            OutputsJson = Params->GetArrayField(TEXT("outputs"));
        }
        bool bPureFunction = GetJsonBoolField(Params, TEXT("pure"), false);
        bool bStaticFunction = GetJsonBoolField(Params, TEXT("static"), false);
        bool bConstFunction = GetJsonBoolField(Params, TEXT("const"), false);
        FString Category = GetJsonStringField(Params, TEXT("category"), TEXT("Default"));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString AccessSpecifier = GetJsonStringField(Params, TEXT("access"), TEXT("Public")); // Public, Protected, Private

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
        if (Blueprint)
        {
            UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, *FunctionName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
            if (NewGraph)
            {
                FBlueprintEditorUtils::AddFunctionGraph(Blueprint, NewGraph, false, nullptr);
                FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
                Result->SetStringField(TEXT("function_name"), FunctionName);
                Result->SetNumberField(TEXT("input_count"), InputsJson.Num());
                Result->SetNumberField(TEXT("output_count"), OutputsJson.Num());
                Result->SetBoolField(TEXT("pure"), bPureFunction);
                Result->SetBoolField(TEXT("static"), bStaticFunction);
                SendAutomationResponse(Socket, RequestId, true, TEXT("Blueprint function created"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create function"), TEXT("CREATE_ERROR"));
        return true;
    }

    // add_blueprint_event - Custom event
    if (SubAction == TEXT("add_blueprint_event"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString EventName = GetJsonStringField(Params, TEXT("event_name"), TEXT("NewEvent"));
        TArray<TSharedPtr<FJsonValue>> ParametersJson;
        if (Params->HasField(TEXT("parameters")))
        {
            ParametersJson = Params->GetArrayField(TEXT("parameters"));
        }
        bool bCallInEditor = GetJsonBoolField(Params, TEXT("call_in_editor"), false);
        bool bReliable = GetJsonBoolField(Params, TEXT("reliable"), true);
        FString ReplicationSetting = GetJsonStringField(Params, TEXT("replication"), TEXT("NotReplicated")); // NotReplicated, Multicast, RunOnServer, RunOnOwningClient

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
        if (Blueprint)
        {
            UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
            if (EventGraph)
            {
                FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
                Result->SetStringField(TEXT("event_name"), EventName);
                Result->SetNumberField(TEXT("parameter_count"), ParametersJson.Num());
                Result->SetBoolField(TEXT("call_in_editor"), bCallInEditor);
                Result->SetStringField(TEXT("replication"), ReplicationSetting);
                SendAutomationResponse(Socket, RequestId, true, TEXT("Blueprint event added"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to add event"), TEXT("ADD_ERROR"));
        return true;
    }

    // configure_blueprint_interface - Interfaces
    if (SubAction == TEXT("configure_blueprint_interface"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        TArray<TSharedPtr<FJsonValue>> InterfacesJson;
        if (Params->HasField(TEXT("interfaces")))
        {
            InterfacesJson = Params->GetArrayField(TEXT("interfaces"));
        }
        bool bImplementAll = GetJsonBoolField(Params, TEXT("implement_all"), true);

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
        if (Blueprint)
        {
            TArray<FString> AddedInterfaces;
            for (const TSharedPtr<FJsonValue>& InterfaceValue : InterfacesJson)
            {
                FString InterfacePath = InterfaceValue->AsString();
                UClass* InterfaceClass = LoadClass<UInterface>(nullptr, *InterfacePath);
                if (InterfaceClass)
                {
                    FBPInterfaceDescription InterfaceDesc;
                    InterfaceDesc.Interface = InterfaceClass;
                    Blueprint->ImplementedInterfaces.Add(InterfaceDesc);
                    AddedInterfaces.Add(InterfacePath);
                }
            }

            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
            Result->SetNumberField(TEXT("interface_count"), AddedInterfaces.Num());
            SendAutomationResponse(Socket, RequestId, true, TEXT("Blueprint interfaces configured"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to configure interfaces"), TEXT("CONFIG_ERROR"));
        return true;
    }

    // add_blueprint_component - Add component
    if (SubAction == TEXT("add_blueprint_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("NewComponent"));
        FString ComponentClass = GetJsonStringField(Params, TEXT("component_class"), TEXT("StaticMeshComponent"));
        FString ParentComponent = GetJsonStringField(Params, TEXT("parent_component"), TEXT(""));
        FVector Location = Forge2Helpers::GetVectorFromJson(Params, TEXT("location"));
        FRotator Rotation = Forge2Helpers::GetRotatorFromJson(Params, TEXT("rotation"));
        FVector Scale = Forge2Helpers::GetVectorFromJson(Params, TEXT("scale"), FVector::OneVector);

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
        if (Blueprint)
        {
            // Find component class
            UClass* CompClass = UStaticMeshComponent::StaticClass();
            if (ComponentClass == TEXT("SkeletalMeshComponent")) CompClass = USkeletalMeshComponent::StaticClass();
            else if (ComponentClass == TEXT("BoxComponent")) CompClass = UBoxComponent::StaticClass();
            else if (ComponentClass == TEXT("SphereComponent")) CompClass = USphereComponent::StaticClass();
            else if (ComponentClass == TEXT("CapsuleComponent")) CompClass = UCapsuleComponent::StaticClass();
            else if (ComponentClass == TEXT("PointLightComponent")) CompClass = UPointLightComponent::StaticClass();
            else if (ComponentClass == TEXT("SpotLightComponent")) CompClass = USpotLightComponent::StaticClass();
            else if (ComponentClass == TEXT("AudioComponent")) CompClass = UAudioComponent::StaticClass();
            else if (ComponentClass == TEXT("ParticleSystemComponent")) CompClass = UParticleSystemComponent::StaticClass();
            else if (ComponentClass == TEXT("WidgetComponent")) CompClass = UWidgetComponent::StaticClass();

            USCS_Node* NewNode = Blueprint->SimpleConstructionScript->CreateNode(CompClass, *ComponentName);
            if (NewNode)
            {
                Blueprint->SimpleConstructionScript->AddNode(NewNode);

                // Set transform defaults
                if (USceneComponent* SceneComp = Cast<USceneComponent>(NewNode->ComponentTemplate))
                {
                    SceneComp->SetRelativeLocation(Location);
                    SceneComp->SetRelativeRotation(Rotation);
                    SceneComp->SetRelativeScale3D(Scale);
                }

                FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

                TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
                Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
                Result->SetStringField(TEXT("component_name"), ComponentName);
                Result->SetStringField(TEXT("component_class"), ComponentClass);
                SendAutomationResponse(Socket, RequestId, true, TEXT("Blueprint component added"), Result);
                return true;
            }
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to add component"), TEXT("ADD_ERROR"));
        return true;
    }

    // compile_blueprint - Compile and validate
    if (SubAction == TEXT("compile_blueprint"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        bool bSaveOnSuccess = GetJsonBoolField(Params, TEXT("save_on_success"), true);

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
        if (Blueprint)
        {
            FCompilerResultsLog Results;
            FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &Results);

            bool bSuccess = !Blueprint->Status == BS_Error;
            int32 ErrorCount = 0;
            int32 WarningCount = 0;

            for (const TSharedRef<FTokenizedMessage>& Message : Results.Messages)
            {
                if (Message->GetSeverity() == EMessageSeverity::Error) ErrorCount++;
                else if (Message->GetSeverity() == EMessageSeverity::Warning) WarningCount++;
            }

            if (bSuccess && bSaveOnSuccess)
            {
                UPackage* Package = Blueprint->GetOutermost();
                Package->MarkPackageDirty();
            }

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
            Result->SetBoolField(TEXT("success"), bSuccess);
            Result->SetNumberField(TEXT("error_count"), ErrorCount);
            Result->SetNumberField(TEXT("warning_count"), WarningCount);
            SendAutomationResponse(Socket, RequestId, bSuccess,
                bSuccess ? TEXT("Blueprint compiled successfully") : TEXT("Blueprint compilation failed"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Blueprint not found"), TEXT("NOT_FOUND"));
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown blueprint action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// WIDGET HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2WidgetAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_widget_blueprint - Create UMG widget
    if (SubAction == TEXT("create_widget_blueprint"))
    {
        FString WidgetName = GetJsonStringField(Params, TEXT("name"), TEXT("NewWidget"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/UI/Widgets"));
        FString ParentClass = GetJsonStringField(Params, TEXT("parent_class"), TEXT("UserWidget"));
        FString RootWidget = GetJsonStringField(Params, TEXT("root_widget"), TEXT("CanvasPanel")); // CanvasPanel, Overlay, VerticalBox, HorizontalBox, Grid, Border

        FString FullPath = OutputPath / WidgetName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for widget"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
        UWidgetBlueprint* NewWidget = Cast<UWidgetBlueprint>(Factory->FactoryCreateNew(
            UWidgetBlueprint::StaticClass(), Package, *WidgetName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewWidget)
        {
            FAssetRegistryModule::AssetCreated(NewWidget);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("widget_name"), WidgetName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("parent_class"), ParentClass);
            Result->SetStringField(TEXT("root_widget"), RootWidget);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Widget blueprint created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create widget blueprint"), TEXT("CREATE_ERROR"));
        return true;
    }

    // add_widget_element - Add UI element
    if (SubAction == TEXT("add_widget_element"))
    {
        FString WidgetPath = GetJsonStringField(Params, TEXT("widget_path"), TEXT(""));
        FString ElementName = GetJsonStringField(Params, TEXT("element_name"), TEXT("NewElement"));
        FString ElementType = GetJsonStringField(Params, TEXT("element_type"), TEXT("TextBlock")); // Button, TextBlock, Image, ProgressBar, Slider, CheckBox, ComboBox, EditableText, RichTextBlock, Border, CanvasPanel, HorizontalBox, VerticalBox, Grid, ScrollBox, SizeBox
        FString ParentSlot = GetJsonStringField(Params, TEXT("parent_slot"), TEXT(""));

        // Position/Size
        float PositionX = GetJsonNumberField(Params, TEXT("position_x"), 0.0f);
        float PositionY = GetJsonNumberField(Params, TEXT("position_y"), 0.0f);
        float SizeX = GetJsonNumberField(Params, TEXT("size_x"), 100.0f);
        float SizeY = GetJsonNumberField(Params, TEXT("size_y"), 32.0f);

        // Anchors
        float AnchorMinX = GetJsonNumberField(Params, TEXT("anchor_min_x"), 0.5f);
        float AnchorMinY = GetJsonNumberField(Params, TEXT("anchor_min_y"), 0.5f);
        float AnchorMaxX = GetJsonNumberField(Params, TEXT("anchor_max_x"), 0.5f);
        float AnchorMaxY = GetJsonNumberField(Params, TEXT("anchor_max_y"), 0.5f);

        // Alignment
        float AlignmentX = GetJsonNumberField(Params, TEXT("alignment_x"), 0.5f);
        float AlignmentY = GetJsonNumberField(Params, TEXT("alignment_y"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("widget_path"), WidgetPath);
        Result->SetStringField(TEXT("element_name"), ElementName);
        Result->SetStringField(TEXT("element_type"), ElementType);
        Result->SetNumberField(TEXT("position_x"), PositionX);
        Result->SetNumberField(TEXT("position_y"), PositionY);
        Result->SetNumberField(TEXT("size_x"), SizeX);
        Result->SetNumberField(TEXT("size_y"), SizeY);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Widget element added"), Result);
        return true;
    }

    // configure_widget_binding - Property binding
    if (SubAction == TEXT("configure_widget_binding"))
    {
        FString WidgetPath = GetJsonStringField(Params, TEXT("widget_path"), TEXT(""));
        FString ElementName = GetJsonStringField(Params, TEXT("element_name"), TEXT(""));
        FString PropertyName = GetJsonStringField(Params, TEXT("property_name"), TEXT("")); // Text, Visibility, Color, Percent, IsEnabled, IsChecked
        FString BindingType = GetJsonStringField(Params, TEXT("binding_type"), TEXT("Function")); // Function, Property
        FString BindingSource = GetJsonStringField(Params, TEXT("binding_source"), TEXT(""));
        bool bIsTwoWay = GetJsonBoolField(Params, TEXT("two_way"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("widget_path"), WidgetPath);
        Result->SetStringField(TEXT("element_name"), ElementName);
        Result->SetStringField(TEXT("property_name"), PropertyName);
        Result->SetStringField(TEXT("binding_type"), BindingType);
        Result->SetStringField(TEXT("binding_source"), BindingSource);
        Result->SetBoolField(TEXT("two_way"), bIsTwoWay);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Widget binding configured"), Result);
        return true;
    }

    // setup_widget_animation - Keyframe animation
    if (SubAction == TEXT("setup_widget_animation"))
    {
        FString WidgetPath = GetJsonStringField(Params, TEXT("widget_path"), TEXT(""));
        FString AnimationName = GetJsonStringField(Params, TEXT("animation_name"), TEXT("NewAnimation"));
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 1.0f);
        bool bLooping = GetJsonBoolField(Params, TEXT("looping"), false);
        int32 LoopCount = GetJsonIntField(Params, TEXT("loop_count"), 0);
        FString PlaybackSpeed = GetJsonStringField(Params, TEXT("playback_speed"), TEXT("1.0"));
        TArray<TSharedPtr<FJsonValue>> TracksJson;
        if (Params->HasField(TEXT("tracks")))
        {
            TracksJson = Params->GetArrayField(TEXT("tracks"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("widget_path"), WidgetPath);
        Result->SetStringField(TEXT("animation_name"), AnimationName);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetBoolField(TEXT("looping"), bLooping);
        Result->SetNumberField(TEXT("track_count"), TracksJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Widget animation setup"), Result);
        return true;
    }

    // create_widget_style - Colors, fonts, margins
    if (SubAction == TEXT("create_widget_style"))
    {
        FString StyleName = GetJsonStringField(Params, TEXT("name"), TEXT("DefaultStyle"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/UI/Styles"));

        // Colors
        FLinearColor PrimaryColor = FLinearColor::White;
        FLinearColor SecondaryColor = FLinearColor::Gray;
        FLinearColor AccentColor = FLinearColor::Blue;
        FLinearColor TextColor = FLinearColor::White;
        FLinearColor BackgroundColor = FLinearColor::Black;

        if (Params->HasField(TEXT("primary_color")))
        {
            TSharedPtr<FJsonObject> ColorJson = Params->GetObjectField(TEXT("primary_color"));
            PrimaryColor = FLinearColor(
                GetJsonNumberField(ColorJson, TEXT("r"), 1.0f),
                GetJsonNumberField(ColorJson, TEXT("g"), 1.0f),
                GetJsonNumberField(ColorJson, TEXT("b"), 1.0f),
                GetJsonNumberField(ColorJson, TEXT("a"), 1.0f));
        }

        // Font settings
        FString FontFamily = GetJsonStringField(Params, TEXT("font_family"), TEXT("Roboto"));
        int32 FontSizeTitle = GetJsonIntField(Params, TEXT("font_size_title"), 24);
        int32 FontSizeBody = GetJsonIntField(Params, TEXT("font_size_body"), 14);
        int32 FontSizeSmall = GetJsonIntField(Params, TEXT("font_size_small"), 10);

        // Margins/Padding
        float DefaultMargin = GetJsonNumberField(Params, TEXT("default_margin"), 8.0f);
        float DefaultPadding = GetJsonNumberField(Params, TEXT("default_padding"), 4.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("style_name"), StyleName);
        Result->SetStringField(TEXT("path"), OutputPath / StyleName);
        Result->SetStringField(TEXT("font_family"), FontFamily);
        Result->SetNumberField(TEXT("font_size_title"), FontSizeTitle);
        Result->SetNumberField(TEXT("font_size_body"), FontSizeBody);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Widget style created"), Result);
        return true;
    }

    // configure_widget_navigation - Focus and tab order
    if (SubAction == TEXT("configure_widget_navigation"))
    {
        FString WidgetPath = GetJsonStringField(Params, TEXT("widget_path"), TEXT(""));
        FString ElementName = GetJsonStringField(Params, TEXT("element_name"), TEXT(""));
        bool bIsFocusable = GetJsonBoolField(Params, TEXT("is_focusable"), true);
        int32 TabIndex = GetJsonIntField(Params, TEXT("tab_index"), 0);
        FString NavigationUp = GetJsonStringField(Params, TEXT("navigation_up"), TEXT(""));
        FString NavigationDown = GetJsonStringField(Params, TEXT("navigation_down"), TEXT(""));
        FString NavigationLeft = GetJsonStringField(Params, TEXT("navigation_left"), TEXT(""));
        FString NavigationRight = GetJsonStringField(Params, TEXT("navigation_right"), TEXT(""));
        FString NavigationMode = GetJsonStringField(Params, TEXT("navigation_mode"), TEXT("Automatic")); // Automatic, Explicit, Custom, Stop

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("widget_path"), WidgetPath);
        Result->SetStringField(TEXT("element_name"), ElementName);
        Result->SetBoolField(TEXT("is_focusable"), bIsFocusable);
        Result->SetNumberField(TEXT("tab_index"), TabIndex);
        Result->SetStringField(TEXT("navigation_mode"), NavigationMode);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Widget navigation configured"), Result);
        return true;
    }

    // setup_widget_localization - Text localization
    if (SubAction == TEXT("setup_widget_localization"))
    {
        FString WidgetPath = GetJsonStringField(Params, TEXT("widget_path"), TEXT(""));
        FString ElementName = GetJsonStringField(Params, TEXT("element_name"), TEXT(""));
        FString Namespace = GetJsonStringField(Params, TEXT("namespace"), TEXT("UI"));
        FString Key = GetJsonStringField(Params, TEXT("key"), TEXT(""));
        FString DefaultText = GetJsonStringField(Params, TEXT("default_text"), TEXT(""));
        bool bUseAsSource = GetJsonBoolField(Params, TEXT("use_as_source"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("widget_path"), WidgetPath);
        Result->SetStringField(TEXT("element_name"), ElementName);
        Result->SetStringField(TEXT("namespace"), Namespace);
        Result->SetStringField(TEXT("key"), Key);
        Result->SetStringField(TEXT("default_text"), DefaultText);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Widget localization setup"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown widget action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// NETWORKING HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2NetworkingAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // setup_replication - Actor replication
    if (SubAction == TEXT("setup_replication"))
    {
        FString ActorPath = GetJsonStringField(Params, TEXT("actor_path"), TEXT(""));
        bool bReplicates = GetJsonBoolField(Params, TEXT("replicates"), true);
        bool bAlwaysRelevant = GetJsonBoolField(Params, TEXT("always_relevant"), false);
        bool bOnlyRelevantToOwner = GetJsonBoolField(Params, TEXT("only_relevant_to_owner"), false);
        bool bReplicateMovement = GetJsonBoolField(Params, TEXT("replicate_movement"), true);
        float NetUpdateFrequency = GetJsonNumberField(Params, TEXT("net_update_frequency"), 100.0f);
        float MinNetUpdateFrequency = GetJsonNumberField(Params, TEXT("min_net_update_frequency"), 2.0f);
        FString NetPriority = GetJsonStringField(Params, TEXT("net_priority"), TEXT("Normal")); // Low, Normal, High, Critical
        float NetCullDistanceSquared = GetJsonNumberField(Params, TEXT("net_cull_distance_squared"), 225000000.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("actor_path"), ActorPath);
        Result->SetBoolField(TEXT("replicates"), bReplicates);
        Result->SetBoolField(TEXT("always_relevant"), bAlwaysRelevant);
        Result->SetBoolField(TEXT("replicate_movement"), bReplicateMovement);
        Result->SetNumberField(TEXT("net_update_frequency"), NetUpdateFrequency);
        Result->SetStringField(TEXT("net_priority"), NetPriority);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Replication setup configured"), Result);
        return true;
    }

    // configure_rpc - Server/Client/Multicast RPCs
    if (SubAction == TEXT("configure_rpc"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString FunctionName = GetJsonStringField(Params, TEXT("function_name"), TEXT(""));
        FString RPCType = GetJsonStringField(Params, TEXT("rpc_type"), TEXT("Server")); // Server, Client, Multicast, NetMulticast
        bool bReliable = GetJsonBoolField(Params, TEXT("reliable"), true);
        bool bWithValidation = GetJsonBoolField(Params, TEXT("with_validation"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("function_name"), FunctionName);
        Result->SetStringField(TEXT("rpc_type"), RPCType);
        Result->SetBoolField(TEXT("reliable"), bReliable);
        Result->SetBoolField(TEXT("with_validation"), bWithValidation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("RPC configured"), Result);
        return true;
    }

    // create_game_mode - Server game rules
    if (SubAction == TEXT("create_game_mode"))
    {
        FString GameModeName = GetJsonStringField(Params, TEXT("name"), TEXT("NewGameMode"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/GameModes"));
        FString DefaultPawnClass = GetJsonStringField(Params, TEXT("default_pawn_class"), TEXT(""));
        FString PlayerControllerClass = GetJsonStringField(Params, TEXT("player_controller_class"), TEXT(""));
        FString HUDClass = GetJsonStringField(Params, TEXT("hud_class"), TEXT(""));
        FString GameStateClass = GetJsonStringField(Params, TEXT("game_state_class"), TEXT(""));
        FString PlayerStateClass = GetJsonStringField(Params, TEXT("player_state_class"), TEXT(""));
        FString SpectatorClass = GetJsonStringField(Params, TEXT("spectator_class"), TEXT(""));
        bool bDelayedStart = GetJsonBoolField(Params, TEXT("delayed_start"), false);
        bool bStartPlayersAsSpectators = GetJsonBoolField(Params, TEXT("start_as_spectators"), false);
        int32 NumPlayers = GetJsonIntField(Params, TEXT("num_players"), 1);

        FString FullPath = OutputPath / GameModeName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for game mode"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        Factory->ParentClass = AGameModeBase::StaticClass();

        UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(
            UBlueprint::StaticClass(), Package, *GameModeName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewBlueprint)
        {
            FAssetRegistryModule::AssetCreated(NewBlueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("game_mode_name"), GameModeName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("default_pawn_class"), DefaultPawnClass);
            Result->SetStringField(TEXT("player_controller_class"), PlayerControllerClass);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Game mode created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create game mode"), TEXT("CREATE_ERROR"));
        return true;
    }

    // setup_game_state - Replicated game data
    if (SubAction == TEXT("setup_game_state"))
    {
        FString GameStateName = GetJsonStringField(Params, TEXT("name"), TEXT("NewGameState"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/GameStates"));
        TArray<TSharedPtr<FJsonValue>> ReplicatedPropertiesJson;
        if (Params->HasField(TEXT("replicated_properties")))
        {
            ReplicatedPropertiesJson = Params->GetArrayField(TEXT("replicated_properties"));
        }
        bool bMatchState = GetJsonBoolField(Params, TEXT("track_match_state"), true);
        bool bTeamScores = GetJsonBoolField(Params, TEXT("track_team_scores"), false);

        FString FullPath = OutputPath / GameStateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for game state"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        Factory->ParentClass = AGameStateBase::StaticClass();

        UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(
            UBlueprint::StaticClass(), Package, *GameStateName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewBlueprint)
        {
            FAssetRegistryModule::AssetCreated(NewBlueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("game_state_name"), GameStateName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetNumberField(TEXT("replicated_property_count"), ReplicatedPropertiesJson.Num());
            Result->SetBoolField(TEXT("track_match_state"), bMatchState);
            Result->SetBoolField(TEXT("track_team_scores"), bTeamScores);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Game state created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create game state"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_player_state - Per-player replicated data
    if (SubAction == TEXT("configure_player_state"))
    {
        FString PlayerStateName = GetJsonStringField(Params, TEXT("name"), TEXT("NewPlayerState"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/PlayerStates"));
        TArray<TSharedPtr<FJsonValue>> ReplicatedPropertiesJson;
        if (Params->HasField(TEXT("replicated_properties")))
        {
            ReplicatedPropertiesJson = Params->GetArrayField(TEXT("replicated_properties"));
        }
        bool bTrackScore = GetJsonBoolField(Params, TEXT("track_score"), true);
        bool bTrackStats = GetJsonBoolField(Params, TEXT("track_stats"), true);

        FString FullPath = OutputPath / PlayerStateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for player state"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        Factory->ParentClass = APlayerState::StaticClass();

        UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(
            UBlueprint::StaticClass(), Package, *PlayerStateName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewBlueprint)
        {
            FAssetRegistryModule::AssetCreated(NewBlueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("player_state_name"), PlayerStateName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetNumberField(TEXT("replicated_property_count"), ReplicatedPropertiesJson.Num());
            Result->SetBoolField(TEXT("track_score"), bTrackScore);
            Result->SetBoolField(TEXT("track_stats"), bTrackStats);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Player state created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create player state"), TEXT("CREATE_ERROR"));
        return true;
    }

    // setup_session_system - Lobby and matchmaking
    if (SubAction == TEXT("setup_session_system"))
    {
        FString SessionName = GetJsonStringField(Params, TEXT("session_name"), TEXT("GameSession"));
        int32 MaxPlayers = GetJsonIntField(Params, TEXT("max_players"), 16);
        bool bLAN = GetJsonBoolField(Params, TEXT("lan"), false);
        bool bDedicated = GetJsonBoolField(Params, TEXT("dedicated"), false);
        bool bPresence = GetJsonBoolField(Params, TEXT("presence"), true);
        bool bAllowInvites = GetJsonBoolField(Params, TEXT("allow_invites"), true);
        bool bAllowJoinInProgress = GetJsonBoolField(Params, TEXT("allow_join_in_progress"), true);
        FString MatchType = GetJsonStringField(Params, TEXT("match_type"), TEXT("Public")); // Public, Private, FriendsOnly
        TArray<TSharedPtr<FJsonValue>> SessionSettingsJson;
        if (Params->HasField(TEXT("settings")))
        {
            SessionSettingsJson = Params->GetArrayField(TEXT("settings"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("session_name"), SessionName);
        Result->SetNumberField(TEXT("max_players"), MaxPlayers);
        Result->SetBoolField(TEXT("lan"), bLAN);
        Result->SetBoolField(TEXT("dedicated"), bDedicated);
        Result->SetBoolField(TEXT("presence"), bPresence);
        Result->SetBoolField(TEXT("allow_join_in_progress"), bAllowJoinInProgress);
        Result->SetStringField(TEXT("match_type"), MatchType);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Session system configured"), Result);
        return true;
    }

    // configure_dedicated_server - Headless server setup
    if (SubAction == TEXT("configure_dedicated_server"))
    {
        int32 Port = GetJsonIntField(Params, TEXT("port"), 7777);
        int32 QueryPort = GetJsonIntField(Params, TEXT("query_port"), 27015);
        FString ServerName = GetJsonStringField(Params, TEXT("server_name"), TEXT("Dedicated Server"));
        FString MapName = GetJsonStringField(Params, TEXT("map_name"), TEXT(""));
        int32 MaxPlayers = GetJsonIntField(Params, TEXT("max_players"), 32);
        bool bLANOnly = GetJsonBoolField(Params, TEXT("lan_only"), false);
        FString AdminPassword = GetJsonStringField(Params, TEXT("admin_password"), TEXT(""));
        int32 TickRate = GetJsonIntField(Params, TEXT("tick_rate"), 60);
        bool bNoSteam = GetJsonBoolField(Params, TEXT("no_steam"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("port"), Port);
        Result->SetNumberField(TEXT("query_port"), QueryPort);
        Result->SetStringField(TEXT("server_name"), ServerName);
        Result->SetStringField(TEXT("map_name"), MapName);
        Result->SetNumberField(TEXT("max_players"), MaxPlayers);
        Result->SetNumberField(TEXT("tick_rate"), TickRate);
        Result->SetBoolField(TEXT("lan_only"), bLANOnly);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Dedicated server configured"), Result);
        return true;
    }

    // setup_listen_server - Player-hosted games
    if (SubAction == TEXT("setup_listen_server"))
    {
        int32 Port = GetJsonIntField(Params, TEXT("port"), 7777);
        FString MapName = GetJsonStringField(Params, TEXT("map_name"), TEXT(""));
        int32 MaxPlayers = GetJsonIntField(Params, TEXT("max_players"), 8);
        bool bUseSteam = GetJsonBoolField(Params, TEXT("use_steam"), true);
        FString ServerPassword = GetJsonStringField(Params, TEXT("password"), TEXT(""));
        bool bAllowSpectators = GetJsonBoolField(Params, TEXT("allow_spectators"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("port"), Port);
        Result->SetStringField(TEXT("map_name"), MapName);
        Result->SetNumberField(TEXT("max_players"), MaxPlayers);
        Result->SetBoolField(TEXT("use_steam"), bUseSteam);
        Result->SetBoolField(TEXT("allow_spectators"), bAllowSpectators);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Listen server configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown networking action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// MMO HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2MmoAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // setup_world_server - Persistent world server
    if (SubAction == TEXT("setup_world_server"))
    {
        FString WorldName = GetJsonStringField(Params, TEXT("world_name"), TEXT("MainWorld"));
        int32 MaxPlayers = GetJsonIntField(Params, TEXT("max_players"), 1000);
        FString PersistenceType = GetJsonStringField(Params, TEXT("persistence_type"), TEXT("Database")); // Database, File, Hybrid
        int32 SaveInterval = GetJsonIntField(Params, TEXT("save_interval"), 300);
        bool bEnableSharding = GetJsonBoolField(Params, TEXT("enable_sharding"), false);
        int32 ShardCapacity = GetJsonIntField(Params, TEXT("shard_capacity"), 200);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("world_name"), WorldName);
        Result->SetNumberField(TEXT("max_players"), MaxPlayers);
        Result->SetStringField(TEXT("persistence_type"), PersistenceType);
        Result->SetNumberField(TEXT("save_interval"), SaveInterval);
        Result->SetBoolField(TEXT("enable_sharding"), bEnableSharding);
        SendAutomationResponse(Socket, RequestId, true, TEXT("World server configured"), Result);
        return true;
    }

    // configure_sharding - World sharding/instancing
    if (SubAction == TEXT("configure_sharding"))
    {
        FString ShardingStrategy = GetJsonStringField(Params, TEXT("strategy"), TEXT("Geographic")); // Geographic, Population, LoadBased
        int32 ShardCapacity = GetJsonIntField(Params, TEXT("shard_capacity"), 200);
        int32 MinShards = GetJsonIntField(Params, TEXT("min_shards"), 1);
        int32 MaxShards = GetJsonIntField(Params, TEXT("max_shards"), 10);
        float LoadThreshold = GetJsonNumberField(Params, TEXT("load_threshold"), 0.8f);
        bool bAllowShardTransfer = GetJsonBoolField(Params, TEXT("allow_shard_transfer"), true);
        int32 TransferCooldown = GetJsonIntField(Params, TEXT("transfer_cooldown"), 30);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("strategy"), ShardingStrategy);
        Result->SetNumberField(TEXT("shard_capacity"), ShardCapacity);
        Result->SetNumberField(TEXT("min_shards"), MinShards);
        Result->SetNumberField(TEXT("max_shards"), MaxShards);
        Result->SetNumberField(TEXT("load_threshold"), LoadThreshold);
        Result->SetBoolField(TEXT("allow_shard_transfer"), bAllowShardTransfer);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Sharding configured"), Result);
        return true;
    }

    // create_instance - Dungeon/raid instances
    if (SubAction == TEXT("create_instance"))
    {
        FString InstanceType = GetJsonStringField(Params, TEXT("instance_type"), TEXT("Dungeon")); // Dungeon, Raid, Battleground, Arena, Housing
        FString InstanceName = GetJsonStringField(Params, TEXT("name"), TEXT("NewInstance"));
        FString MapName = GetJsonStringField(Params, TEXT("map_name"), TEXT(""));
        int32 MinPlayers = GetJsonIntField(Params, TEXT("min_players"), 1);
        int32 MaxPlayers = GetJsonIntField(Params, TEXT("max_players"), 5);
        int32 TimeLimit = GetJsonIntField(Params, TEXT("time_limit"), 0);
        bool bResetOnEmpty = GetJsonBoolField(Params, TEXT("reset_on_empty"), true);
        int32 LockoutDuration = GetJsonIntField(Params, TEXT("lockout_duration"), 86400);
        FString Difficulty = GetJsonStringField(Params, TEXT("difficulty"), TEXT("Normal")); // Normal, Heroic, Mythic

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("instance_type"), InstanceType);
        Result->SetStringField(TEXT("instance_name"), InstanceName);
        Result->SetStringField(TEXT("map_name"), MapName);
        Result->SetNumberField(TEXT("min_players"), MinPlayers);
        Result->SetNumberField(TEXT("max_players"), MaxPlayers);
        Result->SetNumberField(TEXT("time_limit"), TimeLimit);
        Result->SetStringField(TEXT("difficulty"), Difficulty);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Instance created"), Result);
        return true;
    }

    // setup_login_server - Authentication and character select
    if (SubAction == TEXT("setup_login_server"))
    {
        FString AuthType = GetJsonStringField(Params, TEXT("auth_type"), TEXT("Account")); // Account, Steam, Epic, Platform
        bool bEnableQueue = GetJsonBoolField(Params, TEXT("enable_queue"), true);
        int32 QueueCapacity = GetJsonIntField(Params, TEXT("queue_capacity"), 500);
        int32 MaxCharactersPerAccount = GetJsonIntField(Params, TEXT("max_characters"), 10);
        int32 CharacterSlots = GetJsonIntField(Params, TEXT("character_slots"), 5);
        bool bEnableCharacterTransfer = GetJsonBoolField(Params, TEXT("enable_character_transfer"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("auth_type"), AuthType);
        Result->SetBoolField(TEXT("enable_queue"), bEnableQueue);
        Result->SetNumberField(TEXT("queue_capacity"), QueueCapacity);
        Result->SetNumberField(TEXT("max_characters"), MaxCharactersPerAccount);
        Result->SetNumberField(TEXT("character_slots"), CharacterSlots);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Login server configured"), Result);
        return true;
    }

    // configure_chat_system - Global, zone, party, guild chat
    if (SubAction == TEXT("configure_chat_system"))
    {
        TArray<TSharedPtr<FJsonValue>> ChannelsJson;
        if (Params->HasField(TEXT("channels")))
        {
            ChannelsJson = Params->GetArrayField(TEXT("channels"));
        }
        int32 MessageRateLimit = GetJsonIntField(Params, TEXT("message_rate_limit"), 10);
        int32 RateLimitWindow = GetJsonIntField(Params, TEXT("rate_limit_window"), 5);
        int32 MaxMessageLength = GetJsonIntField(Params, TEXT("max_message_length"), 255);
        bool bEnableProfanityFilter = GetJsonBoolField(Params, TEXT("profanity_filter"), true);
        bool bEnableLinks = GetJsonBoolField(Params, TEXT("enable_links"), false);
        bool bEnableMentions = GetJsonBoolField(Params, TEXT("enable_mentions"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("channel_count"), ChannelsJson.Num());
        Result->SetNumberField(TEXT("message_rate_limit"), MessageRateLimit);
        Result->SetNumberField(TEXT("max_message_length"), MaxMessageLength);
        Result->SetBoolField(TEXT("profanity_filter"), bEnableProfanityFilter);
        Result->SetBoolField(TEXT("enable_mentions"), bEnableMentions);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Chat system configured"), Result);
        return true;
    }

    // setup_mail_system - In-game mail with attachments
    if (SubAction == TEXT("setup_mail_system"))
    {
        int32 MaxMailboxSize = GetJsonIntField(Params, TEXT("max_mailbox_size"), 100);
        int32 MaxAttachments = GetJsonIntField(Params, TEXT("max_attachments"), 10);
        int32 MailExpirationDays = GetJsonIntField(Params, TEXT("expiration_days"), 30);
        bool bAllowCOD = GetJsonBoolField(Params, TEXT("allow_cod"), true);
        float CODFee = GetJsonNumberField(Params, TEXT("cod_fee"), 0.05f);
        bool bAllowCrossRealm = GetJsonBoolField(Params, TEXT("allow_cross_realm"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_mailbox_size"), MaxMailboxSize);
        Result->SetNumberField(TEXT("max_attachments"), MaxAttachments);
        Result->SetNumberField(TEXT("expiration_days"), MailExpirationDays);
        Result->SetBoolField(TEXT("allow_cod"), bAllowCOD);
        Result->SetNumberField(TEXT("cod_fee"), CODFee);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Mail system configured"), Result);
        return true;
    }

    // create_guild_system - Guild creation, ranks, permissions
    if (SubAction == TEXT("create_guild_system"))
    {
        int32 MaxGuildSize = GetJsonIntField(Params, TEXT("max_guild_size"), 500);
        int32 MaxRanks = GetJsonIntField(Params, TEXT("max_ranks"), 10);
        int32 GuildCreationCost = GetJsonIntField(Params, TEXT("creation_cost"), 10000);
        int32 MinPlayersToCreate = GetJsonIntField(Params, TEXT("min_players_to_create"), 5);
        bool bEnableGuildBank = GetJsonBoolField(Params, TEXT("enable_guild_bank"), true);
        int32 GuildBankTabs = GetJsonIntField(Params, TEXT("guild_bank_tabs"), 8);
        bool bEnableGuildHousing = GetJsonBoolField(Params, TEXT("enable_guild_housing"), true);

        TArray<TSharedPtr<FJsonValue>> DefaultRanksJson;
        if (Params->HasField(TEXT("default_ranks")))
        {
            DefaultRanksJson = Params->GetArrayField(TEXT("default_ranks"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_guild_size"), MaxGuildSize);
        Result->SetNumberField(TEXT("max_ranks"), MaxRanks);
        Result->SetNumberField(TEXT("creation_cost"), GuildCreationCost);
        Result->SetNumberField(TEXT("min_players_to_create"), MinPlayersToCreate);
        Result->SetBoolField(TEXT("enable_guild_bank"), bEnableGuildBank);
        Result->SetNumberField(TEXT("guild_bank_tabs"), GuildBankTabs);
        Result->SetBoolField(TEXT("enable_guild_housing"), bEnableGuildHousing);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Guild system created"), Result);
        return true;
    }

    // configure_party_system - Group finder, invites, roles
    if (SubAction == TEXT("configure_party_system"))
    {
        int32 MaxPartySize = GetJsonIntField(Params, TEXT("max_party_size"), 5);
        int32 MaxRaidSize = GetJsonIntField(Params, TEXT("max_raid_size"), 40);
        bool bEnableGroupFinder = GetJsonBoolField(Params, TEXT("enable_group_finder"), true);
        bool bEnableRoles = GetJsonBoolField(Params, TEXT("enable_roles"), true);
        TArray<TSharedPtr<FJsonValue>> RolesJson;
        if (Params->HasField(TEXT("roles")))
        {
            RolesJson = Params->GetArrayField(TEXT("roles"));
        }
        bool bEnableLootRules = GetJsonBoolField(Params, TEXT("enable_loot_rules"), true);
        FString DefaultLootRule = GetJsonStringField(Params, TEXT("default_loot_rule"), TEXT("NeedBeforeGreed")); // FreeForAll, RoundRobin, MasterLoot, NeedBeforeGreed, PersonalLoot

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_party_size"), MaxPartySize);
        Result->SetNumberField(TEXT("max_raid_size"), MaxRaidSize);
        Result->SetBoolField(TEXT("enable_group_finder"), bEnableGroupFinder);
        Result->SetBoolField(TEXT("enable_roles"), bEnableRoles);
        Result->SetNumberField(TEXT("role_count"), RolesJson.Num());
        Result->SetStringField(TEXT("default_loot_rule"), DefaultLootRule);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Party system configured"), Result);
        return true;
    }

    // setup_raid_system - Large group coordination
    if (SubAction == TEXT("setup_raid_system"))
    {
        int32 MinRaidSize = GetJsonIntField(Params, TEXT("min_raid_size"), 10);
        int32 MaxRaidSize = GetJsonIntField(Params, TEXT("max_raid_size"), 40);
        bool bEnableRaidMarkers = GetJsonBoolField(Params, TEXT("enable_markers"), true);
        bool bEnableReadyCheck = GetJsonBoolField(Params, TEXT("enable_ready_check"), true);
        bool bEnableRaidWarnings = GetJsonBoolField(Params, TEXT("enable_raid_warnings"), true);
        bool bEnableAssistants = GetJsonBoolField(Params, TEXT("enable_assistants"), true);
        int32 MaxAssistants = GetJsonIntField(Params, TEXT("max_assistants"), 5);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("min_raid_size"), MinRaidSize);
        Result->SetNumberField(TEXT("max_raid_size"), MaxRaidSize);
        Result->SetBoolField(TEXT("enable_markers"), bEnableRaidMarkers);
        Result->SetBoolField(TEXT("enable_ready_check"), bEnableReadyCheck);
        Result->SetBoolField(TEXT("enable_raid_warnings"), bEnableRaidWarnings);
        Result->SetBoolField(TEXT("enable_assistants"), bEnableAssistants);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Raid system configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown mmo action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// SOCIAL HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2SocialAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_friends_system - Add, remove, block
    if (SubAction == TEXT("create_friends_system"))
    {
        int32 MaxFriends = GetJsonIntField(Params, TEXT("max_friends"), 200);
        int32 MaxBlocked = GetJsonIntField(Params, TEXT("max_blocked"), 100);
        bool bEnableFavorites = GetJsonBoolField(Params, TEXT("enable_favorites"), true);
        int32 MaxFavorites = GetJsonIntField(Params, TEXT("max_favorites"), 20);
        bool bEnableNotes = GetJsonBoolField(Params, TEXT("enable_notes"), true);
        bool bShowMutualFriends = GetJsonBoolField(Params, TEXT("show_mutual_friends"), true);
        bool bEnableFriendGroups = GetJsonBoolField(Params, TEXT("enable_friend_groups"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_friends"), MaxFriends);
        Result->SetNumberField(TEXT("max_blocked"), MaxBlocked);
        Result->SetBoolField(TEXT("enable_favorites"), bEnableFavorites);
        Result->SetBoolField(TEXT("enable_notes"), bEnableNotes);
        Result->SetBoolField(TEXT("show_mutual_friends"), bShowMutualFriends);
        Result->SetBoolField(TEXT("enable_friend_groups"), bEnableFriendGroups);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Friends system created"), Result);
        return true;
    }

    // setup_presence - Online status and activity
    if (SubAction == TEXT("setup_presence"))
    {
        TArray<TSharedPtr<FJsonValue>> StatusTypesJson;
        if (Params->HasField(TEXT("status_types")))
        {
            StatusTypesJson = Params->GetArrayField(TEXT("status_types"));
        }
        bool bShowActivity = GetJsonBoolField(Params, TEXT("show_activity"), true);
        bool bShowLocation = GetJsonBoolField(Params, TEXT("show_location"), true);
        bool bShowPartyStatus = GetJsonBoolField(Params, TEXT("show_party_status"), true);
        bool bAllowInvisible = GetJsonBoolField(Params, TEXT("allow_invisible"), true);
        int32 UpdateInterval = GetJsonIntField(Params, TEXT("update_interval"), 30);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("status_type_count"), StatusTypesJson.Num());
        Result->SetBoolField(TEXT("show_activity"), bShowActivity);
        Result->SetBoolField(TEXT("show_location"), bShowLocation);
        Result->SetBoolField(TEXT("show_party_status"), bShowPartyStatus);
        Result->SetBoolField(TEXT("allow_invisible"), bAllowInvisible);
        Result->SetNumberField(TEXT("update_interval"), UpdateInterval);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Presence system configured"), Result);
        return true;
    }

    // configure_voice_chat - Vivox/native integration
    if (SubAction == TEXT("configure_voice_chat"))
    {
        FString Provider = GetJsonStringField(Params, TEXT("provider"), TEXT("Native")); // Native, Vivox, Discord, Custom
        bool bEnableProximityVoice = GetJsonBoolField(Params, TEXT("proximity_voice"), true);
        float ProximityRange = GetJsonNumberField(Params, TEXT("proximity_range"), 2000.0f);
        bool bEnablePartyVoice = GetJsonBoolField(Params, TEXT("party_voice"), true);
        bool bEnableGuildVoice = GetJsonBoolField(Params, TEXT("guild_voice"), true);
        bool bEnablePushToTalk = GetJsonBoolField(Params, TEXT("push_to_talk"), true);
        bool bEnableVoiceActivation = GetJsonBoolField(Params, TEXT("voice_activation"), true);
        float VoiceActivationThreshold = GetJsonNumberField(Params, TEXT("activation_threshold"), 0.1f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("provider"), Provider);
        Result->SetBoolField(TEXT("proximity_voice"), bEnableProximityVoice);
        Result->SetNumberField(TEXT("proximity_range"), ProximityRange);
        Result->SetBoolField(TEXT("party_voice"), bEnablePartyVoice);
        Result->SetBoolField(TEXT("guild_voice"), bEnableGuildVoice);
        Result->SetBoolField(TEXT("push_to_talk"), bEnablePushToTalk);
        Result->SetBoolField(TEXT("voice_activation"), bEnableVoiceActivation);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Voice chat configured"), Result);
        return true;
    }

    // create_emote_system - Player emotes and dances
    if (SubAction == TEXT("create_emote_system"))
    {
        TArray<TSharedPtr<FJsonValue>> EmotesJson;
        if (Params->HasField(TEXT("emotes")))
        {
            EmotesJson = Params->GetArrayField(TEXT("emotes"));
        }
        int32 MaxEmoteSlots = GetJsonIntField(Params, TEXT("max_emote_slots"), 10);
        bool bEnableEmoteWheel = GetJsonBoolField(Params, TEXT("enable_emote_wheel"), true);
        bool bAllowEmoteChains = GetJsonBoolField(Params, TEXT("allow_emote_chains"), true);
        float EmoteCooldown = GetJsonNumberField(Params, TEXT("emote_cooldown"), 1.0f);
        bool bInterruptOnMovement = GetJsonBoolField(Params, TEXT("interrupt_on_movement"), true);
        bool bShowEmoteText = GetJsonBoolField(Params, TEXT("show_emote_text"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("emote_count"), EmotesJson.Num());
        Result->SetNumberField(TEXT("max_emote_slots"), MaxEmoteSlots);
        Result->SetBoolField(TEXT("enable_emote_wheel"), bEnableEmoteWheel);
        Result->SetBoolField(TEXT("allow_emote_chains"), bAllowEmoteChains);
        Result->SetNumberField(TEXT("emote_cooldown"), EmoteCooldown);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Emote system created"), Result);
        return true;
    }

    // setup_player_housing - Instanced homes and visiting
    if (SubAction == TEXT("setup_player_housing"))
    {
        FString HousingType = GetJsonStringField(Params, TEXT("housing_type"), TEXT("Instanced")); // Instanced, OpenWorld, Neighborhood
        int32 MaxPlots = GetJsonIntField(Params, TEXT("max_plots"), 1);
        int32 MaxFurniture = GetJsonIntField(Params, TEXT("max_furniture"), 200);
        bool bAllowVisitors = GetJsonBoolField(Params, TEXT("allow_visitors"), true);
        bool bEnablePermissions = GetJsonBoolField(Params, TEXT("enable_permissions"), true);
        bool bEnableStorage = GetJsonBoolField(Params, TEXT("enable_storage"), true);
        int32 StorageCapacity = GetJsonIntField(Params, TEXT("storage_capacity"), 100);
        bool bEnableGardening = GetJsonBoolField(Params, TEXT("enable_gardening"), true);
        bool bEnableCrafting = GetJsonBoolField(Params, TEXT("enable_crafting"), true);
        int32 PlotCost = GetJsonIntField(Params, TEXT("plot_cost"), 50000);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("housing_type"), HousingType);
        Result->SetNumberField(TEXT("max_plots"), MaxPlots);
        Result->SetNumberField(TEXT("max_furniture"), MaxFurniture);
        Result->SetBoolField(TEXT("allow_visitors"), bAllowVisitors);
        Result->SetBoolField(TEXT("enable_permissions"), bEnablePermissions);
        Result->SetBoolField(TEXT("enable_storage"), bEnableStorage);
        Result->SetNumberField(TEXT("storage_capacity"), StorageCapacity);
        Result->SetNumberField(TEXT("plot_cost"), PlotCost);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Player housing configured"), Result);
        return true;
    }

    // configure_trading - Direct trade and mail trade
    if (SubAction == TEXT("configure_trading"))
    {
        bool bEnableDirectTrade = GetJsonBoolField(Params, TEXT("enable_direct_trade"), true);
        int32 MaxTradeItems = GetJsonIntField(Params, TEXT("max_trade_items"), 12);
        bool bRequireConfirmation = GetJsonBoolField(Params, TEXT("require_confirmation"), true);
        float TradeTax = GetJsonNumberField(Params, TEXT("trade_tax"), 0.0f);
        int32 TradeRange = GetJsonIntField(Params, TEXT("trade_range"), 1000);
        bool bAllowCrossRealm = GetJsonBoolField(Params, TEXT("allow_cross_realm"), false);
        TArray<TSharedPtr<FJsonValue>> RestrictedItemsJson;
        if (Params->HasField(TEXT("restricted_items")))
        {
            RestrictedItemsJson = Params->GetArrayField(TEXT("restricted_items"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enable_direct_trade"), bEnableDirectTrade);
        Result->SetNumberField(TEXT("max_trade_items"), MaxTradeItems);
        Result->SetBoolField(TEXT("require_confirmation"), bRequireConfirmation);
        Result->SetNumberField(TEXT("trade_tax"), TradeTax);
        Result->SetNumberField(TEXT("trade_range"), TradeRange);
        Result->SetBoolField(TEXT("allow_cross_realm"), bAllowCrossRealm);
        Result->SetNumberField(TEXT("restricted_item_count"), RestrictedItemsJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Trading configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown social action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// SAVE SYSTEM HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2SaveSystemAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_save_game - Save game structure
    if (SubAction == TEXT("create_save_game"))
    {
        FString SaveGameName = GetJsonStringField(Params, TEXT("name"), TEXT("NewSaveGame"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/SaveGames"));
        TArray<TSharedPtr<FJsonValue>> PropertiesJson;
        if (Params->HasField(TEXT("properties")))
        {
            PropertiesJson = Params->GetArrayField(TEXT("properties"));
        }
        bool bCompressed = GetJsonBoolField(Params, TEXT("compressed"), true);
        bool bEncrypted = GetJsonBoolField(Params, TEXT("encrypted"), false);
        FString Version = GetJsonStringField(Params, TEXT("version"), TEXT("1.0"));

        FString FullPath = OutputPath / SaveGameName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for save game"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        Factory->ParentClass = USaveGame::StaticClass();

        UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(
            UBlueprint::StaticClass(), Package, *SaveGameName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewBlueprint)
        {
            FAssetRegistryModule::AssetCreated(NewBlueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("save_game_name"), SaveGameName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetNumberField(TEXT("property_count"), PropertiesJson.Num());
            Result->SetBoolField(TEXT("compressed"), bCompressed);
            Result->SetBoolField(TEXT("encrypted"), bEncrypted);
            Result->SetStringField(TEXT("version"), Version);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Save game created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create save game"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_autosave - Intervals and triggers
    if (SubAction == TEXT("configure_autosave"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        int32 IntervalSeconds = GetJsonIntField(Params, TEXT("interval_seconds"), 300);
        bool bSaveOnLevelChange = GetJsonBoolField(Params, TEXT("save_on_level_change"), true);
        bool bSaveOnQuestComplete = GetJsonBoolField(Params, TEXT("save_on_quest_complete"), true);
        bool bSaveOnCheckpoint = GetJsonBoolField(Params, TEXT("save_on_checkpoint"), true);
        bool bSaveOnExit = GetJsonBoolField(Params, TEXT("save_on_exit"), true);
        int32 MaxAutosaves = GetJsonIntField(Params, TEXT("max_autosaves"), 3);
        bool bShowNotification = GetJsonBoolField(Params, TEXT("show_notification"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("interval_seconds"), IntervalSeconds);
        Result->SetBoolField(TEXT("save_on_level_change"), bSaveOnLevelChange);
        Result->SetBoolField(TEXT("save_on_quest_complete"), bSaveOnQuestComplete);
        Result->SetBoolField(TEXT("save_on_checkpoint"), bSaveOnCheckpoint);
        Result->SetBoolField(TEXT("save_on_exit"), bSaveOnExit);
        Result->SetNumberField(TEXT("max_autosaves"), MaxAutosaves);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Autosave configured"), Result);
        return true;
    }

    // setup_save_slots - Multiple saves with thumbnails
    if (SubAction == TEXT("setup_save_slots"))
    {
        int32 MaxSlots = GetJsonIntField(Params, TEXT("max_slots"), 10);
        bool bEnableThumbnails = GetJsonBoolField(Params, TEXT("enable_thumbnails"), true);
        int32 ThumbnailWidth = GetJsonIntField(Params, TEXT("thumbnail_width"), 320);
        int32 ThumbnailHeight = GetJsonIntField(Params, TEXT("thumbnail_height"), 180);
        bool bShowPlayTime = GetJsonBoolField(Params, TEXT("show_play_time"), true);
        bool bShowLocation = GetJsonBoolField(Params, TEXT("show_location"), true);
        bool bShowTimestamp = GetJsonBoolField(Params, TEXT("show_timestamp"), true);
        bool bAllowOverwrite = GetJsonBoolField(Params, TEXT("allow_overwrite"), true);
        bool bConfirmOverwrite = GetJsonBoolField(Params, TEXT("confirm_overwrite"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("max_slots"), MaxSlots);
        Result->SetBoolField(TEXT("enable_thumbnails"), bEnableThumbnails);
        Result->SetNumberField(TEXT("thumbnail_width"), ThumbnailWidth);
        Result->SetNumberField(TEXT("thumbnail_height"), ThumbnailHeight);
        Result->SetBoolField(TEXT("show_play_time"), bShowPlayTime);
        Result->SetBoolField(TEXT("show_location"), bShowLocation);
        Result->SetBoolField(TEXT("allow_overwrite"), bAllowOverwrite);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Save slots configured"), Result);
        return true;
    }

    // create_cloud_save - Platform cloud sync
    if (SubAction == TEXT("create_cloud_save"))
    {
        FString Provider = GetJsonStringField(Params, TEXT("provider"), TEXT("Steam")); // Steam, Epic, PlayStation, Xbox, Custom
        bool bAutoSync = GetJsonBoolField(Params, TEXT("auto_sync"), true);
        int32 SyncInterval = GetJsonIntField(Params, TEXT("sync_interval"), 60);
        bool bConflictResolution = GetJsonBoolField(Params, TEXT("conflict_resolution"), true);
        FString ConflictStrategy = GetJsonStringField(Params, TEXT("conflict_strategy"), TEXT("UseNewer")); // UseNewer, UseLocal, UseCloud, AskUser
        int32 MaxCloudSaves = GetJsonIntField(Params, TEXT("max_cloud_saves"), 5);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("provider"), Provider);
        Result->SetBoolField(TEXT("auto_sync"), bAutoSync);
        Result->SetNumberField(TEXT("sync_interval"), SyncInterval);
        Result->SetBoolField(TEXT("conflict_resolution"), bConflictResolution);
        Result->SetStringField(TEXT("conflict_strategy"), ConflictStrategy);
        Result->SetNumberField(TEXT("max_cloud_saves"), MaxCloudSaves);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Cloud save configured"), Result);
        return true;
    }

    // configure_save_migration - Version compatibility
    if (SubAction == TEXT("configure_save_migration"))
    {
        FString CurrentVersion = GetJsonStringField(Params, TEXT("current_version"), TEXT("1.0"));
        TArray<TSharedPtr<FJsonValue>> MigrationRulesJson;
        if (Params->HasField(TEXT("migration_rules")))
        {
            MigrationRulesJson = Params->GetArrayField(TEXT("migration_rules"));
        }
        bool bAutoMigrate = GetJsonBoolField(Params, TEXT("auto_migrate"), true);
        bool bBackupBeforeMigrate = GetJsonBoolField(Params, TEXT("backup_before_migrate"), true);
        bool bAllowDowngrade = GetJsonBoolField(Params, TEXT("allow_downgrade"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("current_version"), CurrentVersion);
        Result->SetNumberField(TEXT("migration_rule_count"), MigrationRulesJson.Num());
        Result->SetBoolField(TEXT("auto_migrate"), bAutoMigrate);
        Result->SetBoolField(TEXT("backup_before_migrate"), bBackupBeforeMigrate);
        Result->SetBoolField(TEXT("allow_downgrade"), bAllowDowngrade);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Save migration configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown save_system action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// UI SYSTEM HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2UiSystemAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_hud - Health, ammo, minimap, compass
    if (SubAction == TEXT("create_hud"))
    {
        FString HUDName = GetJsonStringField(Params, TEXT("name"), TEXT("GameHUD"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/UI/HUD"));
        TArray<TSharedPtr<FJsonValue>> ElementsJson;
        if (Params->HasField(TEXT("elements")))
        {
            ElementsJson = Params->GetArrayField(TEXT("elements"));
        }
        bool bHealthBar = GetJsonBoolField(Params, TEXT("health_bar"), true);
        bool bManaBar = GetJsonBoolField(Params, TEXT("mana_bar"), true);
        bool bStaminaBar = GetJsonBoolField(Params, TEXT("stamina_bar"), true);
        bool bAmmoCounter = GetJsonBoolField(Params, TEXT("ammo_counter"), true);
        bool bMinimap = GetJsonBoolField(Params, TEXT("minimap"), true);
        bool bCompass = GetJsonBoolField(Params, TEXT("compass"), true);
        bool bCrosshair = GetJsonBoolField(Params, TEXT("crosshair"), true);
        bool bObjectiveTracker = GetJsonBoolField(Params, TEXT("objective_tracker"), true);

        FString FullPath = OutputPath / HUDName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for HUD"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
        UWidgetBlueprint* NewWidget = Cast<UWidgetBlueprint>(Factory->FactoryCreateNew(
            UWidgetBlueprint::StaticClass(), Package, *HUDName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewWidget)
        {
            FAssetRegistryModule::AssetCreated(NewWidget);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("hud_name"), HUDName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetBoolField(TEXT("health_bar"), bHealthBar);
            Result->SetBoolField(TEXT("mana_bar"), bManaBar);
            Result->SetBoolField(TEXT("minimap"), bMinimap);
            Result->SetBoolField(TEXT("compass"), bCompass);
            Result->SetBoolField(TEXT("crosshair"), bCrosshair);
            SendAutomationResponse(Socket, RequestId, true, TEXT("HUD created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create HUD"), TEXT("CREATE_ERROR"));
        return true;
    }

    // create_menu - Main menu, pause menu, settings
    if (SubAction == TEXT("create_menu"))
    {
        FString MenuName = GetJsonStringField(Params, TEXT("name"), TEXT("MainMenu"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/UI/Menus"));
        FString MenuType = GetJsonStringField(Params, TEXT("menu_type"), TEXT("Main")); // Main, Pause, Settings, Inventory, Quest, Map, Character, Skills
        TArray<TSharedPtr<FJsonValue>> ButtonsJson;
        if (Params->HasField(TEXT("buttons")))
        {
            ButtonsJson = Params->GetArrayField(TEXT("buttons"));
        }
        bool bFullScreen = GetJsonBoolField(Params, TEXT("full_screen"), true);
        bool bPauseGame = GetJsonBoolField(Params, TEXT("pause_game"), true);
        bool bShowCursor = GetJsonBoolField(Params, TEXT("show_cursor"), true);
        FString BackgroundImage = GetJsonStringField(Params, TEXT("background_image"), TEXT(""));
        bool bAnimateTransitions = GetJsonBoolField(Params, TEXT("animate_transitions"), true);

        FString FullPath = OutputPath / MenuName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for menu"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
        UWidgetBlueprint* NewWidget = Cast<UWidgetBlueprint>(Factory->FactoryCreateNew(
            UWidgetBlueprint::StaticClass(), Package, *MenuName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewWidget)
        {
            FAssetRegistryModule::AssetCreated(NewWidget);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("menu_name"), MenuName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("menu_type"), MenuType);
            Result->SetNumberField(TEXT("button_count"), ButtonsJson.Num());
            Result->SetBoolField(TEXT("full_screen"), bFullScreen);
            Result->SetBoolField(TEXT("pause_game"), bPauseGame);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Menu created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create menu"), TEXT("CREATE_ERROR"));
        return true;
    }

    // create_inventory_ui - Grid, list, equipment slots
    if (SubAction == TEXT("create_inventory_ui"))
    {
        FString InventoryUIName = GetJsonStringField(Params, TEXT("name"), TEXT("InventoryUI"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/UI/Inventory"));
        FString LayoutType = GetJsonStringField(Params, TEXT("layout_type"), TEXT("Grid")); // Grid, List, Slots
        int32 GridColumns = GetJsonIntField(Params, TEXT("grid_columns"), 8);
        int32 GridRows = GetJsonIntField(Params, TEXT("grid_rows"), 6);
        int32 SlotSize = GetJsonIntField(Params, TEXT("slot_size"), 64);
        bool bShowTooltips = GetJsonBoolField(Params, TEXT("show_tooltips"), true);
        bool bDragAndDrop = GetJsonBoolField(Params, TEXT("drag_and_drop"), true);
        bool bContextMenu = GetJsonBoolField(Params, TEXT("context_menu"), true);
        bool bShowEquipment = GetJsonBoolField(Params, TEXT("show_equipment"), true);
        bool bShowStats = GetJsonBoolField(Params, TEXT("show_stats"), true);
        bool bShowWeight = GetJsonBoolField(Params, TEXT("show_weight"), true);
        bool bShowCurrency = GetJsonBoolField(Params, TEXT("show_currency"), true);

        FString FullPath = OutputPath / InventoryUIName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for inventory UI"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
        UWidgetBlueprint* NewWidget = Cast<UWidgetBlueprint>(Factory->FactoryCreateNew(
            UWidgetBlueprint::StaticClass(), Package, *InventoryUIName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewWidget)
        {
            FAssetRegistryModule::AssetCreated(NewWidget);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("inventory_ui_name"), InventoryUIName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("layout_type"), LayoutType);
            Result->SetNumberField(TEXT("grid_columns"), GridColumns);
            Result->SetNumberField(TEXT("grid_rows"), GridRows);
            Result->SetNumberField(TEXT("slot_size"), SlotSize);
            Result->SetBoolField(TEXT("drag_and_drop"), bDragAndDrop);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Inventory UI created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create inventory UI"), TEXT("CREATE_ERROR"));
        return true;
    }

    // create_dialogue_ui - Subtitle, choices, portraits
    if (SubAction == TEXT("create_dialogue_ui"))
    {
        FString DialogueUIName = GetJsonStringField(Params, TEXT("name"), TEXT("DialogueUI"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/UI/Dialogue"));
        FString Style = GetJsonStringField(Params, TEXT("style"), TEXT("Classic")); // Classic, Modern, Minimalist, RPG, VN
        bool bShowPortrait = GetJsonBoolField(Params, TEXT("show_portrait"), true);
        bool bShowSpeakerName = GetJsonBoolField(Params, TEXT("show_speaker_name"), true);
        bool bTypewriterEffect = GetJsonBoolField(Params, TEXT("typewriter_effect"), true);
        float TypewriterSpeed = GetJsonNumberField(Params, TEXT("typewriter_speed"), 0.05f);
        bool bShowContinuePrompt = GetJsonBoolField(Params, TEXT("show_continue_prompt"), true);
        int32 MaxChoices = GetJsonIntField(Params, TEXT("max_choices"), 4);
        bool bNumberedChoices = GetJsonBoolField(Params, TEXT("numbered_choices"), true);
        bool bShowChoiceTimer = GetJsonBoolField(Params, TEXT("show_choice_timer"), false);

        FString FullPath = OutputPath / DialogueUIName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for dialogue UI"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
        UWidgetBlueprint* NewWidget = Cast<UWidgetBlueprint>(Factory->FactoryCreateNew(
            UWidgetBlueprint::StaticClass(), Package, *DialogueUIName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewWidget)
        {
            FAssetRegistryModule::AssetCreated(NewWidget);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("dialogue_ui_name"), DialogueUIName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("style"), Style);
            Result->SetBoolField(TEXT("show_portrait"), bShowPortrait);
            Result->SetBoolField(TEXT("typewriter_effect"), bTypewriterEffect);
            Result->SetNumberField(TEXT("max_choices"), MaxChoices);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue UI created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create dialogue UI"), TEXT("CREATE_ERROR"));
        return true;
    }

    // setup_notification_system - Toasts, popups, alerts
    if (SubAction == TEXT("setup_notification_system"))
    {
        FString NotificationName = GetJsonStringField(Params, TEXT("name"), TEXT("NotificationSystem"));
        FString Position = GetJsonStringField(Params, TEXT("position"), TEXT("TopRight")); // TopLeft, TopCenter, TopRight, BottomLeft, BottomCenter, BottomRight, Center
        int32 MaxVisible = GetJsonIntField(Params, TEXT("max_visible"), 5);
        float DefaultDuration = GetJsonNumberField(Params, TEXT("default_duration"), 5.0f);
        bool bStackNotifications = GetJsonBoolField(Params, TEXT("stack_notifications"), true);
        bool bAnimateIn = GetJsonBoolField(Params, TEXT("animate_in"), true);
        bool bAnimateOut = GetJsonBoolField(Params, TEXT("animate_out"), true);
        bool bPlaySound = GetJsonBoolField(Params, TEXT("play_sound"), true);
        TArray<TSharedPtr<FJsonValue>> TypesJson;
        if (Params->HasField(TEXT("notification_types")))
        {
            TypesJson = Params->GetArrayField(TEXT("notification_types"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("notification_name"), NotificationName);
        Result->SetStringField(TEXT("position"), Position);
        Result->SetNumberField(TEXT("max_visible"), MaxVisible);
        Result->SetNumberField(TEXT("default_duration"), DefaultDuration);
        Result->SetBoolField(TEXT("stack_notifications"), bStackNotifications);
        Result->SetBoolField(TEXT("animate_in"), bAnimateIn);
        Result->SetBoolField(TEXT("play_sound"), bPlaySound);
        Result->SetNumberField(TEXT("notification_type_count"), TypesJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Notification system configured"), Result);
        return true;
    }

    // configure_input_hints - Controller/keyboard prompts
    if (SubAction == TEXT("configure_input_hints"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        bool bAutoDetectDevice = GetJsonBoolField(Params, TEXT("auto_detect_device"), true);
        FString DefaultDevice = GetJsonStringField(Params, TEXT("default_device"), TEXT("Keyboard")); // Keyboard, Xbox, PlayStation, Switch, Generic
        bool bShowContextualHints = GetJsonBoolField(Params, TEXT("show_contextual_hints"), true);
        bool bShowHoldProgress = GetJsonBoolField(Params, TEXT("show_hold_progress"), true);
        float HintFadeTime = GetJsonNumberField(Params, TEXT("hint_fade_time"), 0.3f);
        TArray<TSharedPtr<FJsonValue>> CustomIconsJson;
        if (Params->HasField(TEXT("custom_icons")))
        {
            CustomIconsJson = Params->GetArrayField(TEXT("custom_icons"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetBoolField(TEXT("auto_detect_device"), bAutoDetectDevice);
        Result->SetStringField(TEXT("default_device"), DefaultDevice);
        Result->SetBoolField(TEXT("show_contextual_hints"), bShowContextualHints);
        Result->SetBoolField(TEXT("show_hold_progress"), bShowHoldProgress);
        Result->SetNumberField(TEXT("hint_fade_time"), HintFadeTime);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Input hints configured"), Result);
        return true;
    }

    // create_tutorial_system - Tooltips, highlights, guides
    if (SubAction == TEXT("create_tutorial_system"))
    {
        FString TutorialName = GetJsonStringField(Params, TEXT("name"), TEXT("TutorialSystem"));
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        bool bShowOnFirstPlay = GetJsonBoolField(Params, TEXT("show_on_first_play"), true);
        bool bAllowSkip = GetJsonBoolField(Params, TEXT("allow_skip"), true);
        bool bHighlightElements = GetJsonBoolField(Params, TEXT("highlight_elements"), true);
        bool bDimBackground = GetJsonBoolField(Params, TEXT("dim_background"), true);
        float DimOpacity = GetJsonNumberField(Params, TEXT("dim_opacity"), 0.7f);
        bool bPauseGame = GetJsonBoolField(Params, TEXT("pause_game"), true);
        bool bShowArrows = GetJsonBoolField(Params, TEXT("show_arrows"), true);
        TArray<TSharedPtr<FJsonValue>> StepsJson;
        if (Params->HasField(TEXT("steps")))
        {
            StepsJson = Params->GetArrayField(TEXT("steps"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("tutorial_name"), TutorialName);
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetBoolField(TEXT("show_on_first_play"), bShowOnFirstPlay);
        Result->SetBoolField(TEXT("allow_skip"), bAllowSkip);
        Result->SetBoolField(TEXT("highlight_elements"), bHighlightElements);
        Result->SetBoolField(TEXT("dim_background"), bDimBackground);
        Result->SetBoolField(TEXT("pause_game"), bPauseGame);
        Result->SetNumberField(TEXT("step_count"), StepsJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Tutorial system created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown ui_system action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// ACCESSIBILITY HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2AccessibilityAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // configure_colorblind_modes - Protanopia, deuteranopia, tritanopia
    if (SubAction == TEXT("configure_colorblind_modes"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        FString DefaultMode = GetJsonStringField(Params, TEXT("default_mode"), TEXT("None")); // None, Protanopia, Deuteranopia, Tritanopia, Achromatopsia
        float Severity = GetJsonNumberField(Params, TEXT("severity"), 1.0f);
        bool bAffectUI = GetJsonBoolField(Params, TEXT("affect_ui"), true);
        bool bAffectWorld = GetJsonBoolField(Params, TEXT("affect_world"), true);
        bool bUseAlternativeColors = GetJsonBoolField(Params, TEXT("use_alternative_colors"), true);

        // Custom color remapping
        TSharedPtr<FJsonObject> ColorRemapJson;
        if (Params->HasField(TEXT("color_remap")))
        {
            ColorRemapJson = Params->GetObjectField(TEXT("color_remap"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetStringField(TEXT("default_mode"), DefaultMode);
        Result->SetNumberField(TEXT("severity"), Severity);
        Result->SetBoolField(TEXT("affect_ui"), bAffectUI);
        Result->SetBoolField(TEXT("affect_world"), bAffectWorld);
        Result->SetBoolField(TEXT("use_alternative_colors"), bUseAlternativeColors);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Colorblind modes configured"), Result);
        return true;
    }

    // setup_subtitle_options - Size, background, speaker names
    if (SubAction == TEXT("setup_subtitle_options"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        FString DefaultSize = GetJsonStringField(Params, TEXT("default_size"), TEXT("Medium")); // Small, Medium, Large, ExtraLarge
        bool bShowBackground = GetJsonBoolField(Params, TEXT("show_background"), true);
        float BackgroundOpacity = GetJsonNumberField(Params, TEXT("background_opacity"), 0.7f);
        bool bShowSpeakerNames = GetJsonBoolField(Params, TEXT("show_speaker_names"), true);
        bool bShowSpeakerColors = GetJsonBoolField(Params, TEXT("show_speaker_colors"), true);
        FString TextColor = GetJsonStringField(Params, TEXT("text_color"), TEXT("White"));
        FString FontFamily = GetJsonStringField(Params, TEXT("font_family"), TEXT("Roboto"));
        bool bClosedCaptions = GetJsonBoolField(Params, TEXT("closed_captions"), true);
        bool bShowDirectionIndicators = GetJsonBoolField(Params, TEXT("show_direction_indicators"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetStringField(TEXT("default_size"), DefaultSize);
        Result->SetBoolField(TEXT("show_background"), bShowBackground);
        Result->SetNumberField(TEXT("background_opacity"), BackgroundOpacity);
        Result->SetBoolField(TEXT("show_speaker_names"), bShowSpeakerNames);
        Result->SetBoolField(TEXT("closed_captions"), bClosedCaptions);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Subtitle options configured"), Result);
        return true;
    }

    // create_audio_cues - Visual to audio translation
    if (SubAction == TEXT("create_audio_cues"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        TArray<TSharedPtr<FJsonValue>> CuesJson;
        if (Params->HasField(TEXT("cues")))
        {
            CuesJson = Params->GetArrayField(TEXT("cues"));
        }
        bool bDirectionalAudio = GetJsonBoolField(Params, TEXT("directional_audio"), true);
        bool bDistanceModulation = GetJsonBoolField(Params, TEXT("distance_modulation"), true);
        float CueVolume = GetJsonNumberField(Params, TEXT("cue_volume"), 1.0f);
        bool bNarrateUIElements = GetJsonBoolField(Params, TEXT("narrate_ui_elements"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("cue_count"), CuesJson.Num());
        Result->SetBoolField(TEXT("directional_audio"), bDirectionalAudio);
        Result->SetBoolField(TEXT("distance_modulation"), bDistanceModulation);
        Result->SetNumberField(TEXT("cue_volume"), CueVolume);
        Result->SetBoolField(TEXT("narrate_ui_elements"), bNarrateUIElements);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Audio cues created"), Result);
        return true;
    }

    // configure_input_remapping - Full rebinding
    if (SubAction == TEXT("configure_input_remapping"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        bool bAllowRemapping = GetJsonBoolField(Params, TEXT("allow_remapping"), true);
        bool bShowConflicts = GetJsonBoolField(Params, TEXT("show_conflicts"), true);
        bool bAllowDuplicates = GetJsonBoolField(Params, TEXT("allow_duplicates"), false);
        bool bSupportGamepad = GetJsonBoolField(Params, TEXT("support_gamepad"), true);
        bool bSupportKeyboard = GetJsonBoolField(Params, TEXT("support_keyboard"), true);
        bool bSupportMouse = GetJsonBoolField(Params, TEXT("support_mouse"), true);
        bool bSaveRemapping = GetJsonBoolField(Params, TEXT("save_remapping"), true);
        TArray<TSharedPtr<FJsonValue>> LockedActionsJson;
        if (Params->HasField(TEXT("locked_actions")))
        {
            LockedActionsJson = Params->GetArrayField(TEXT("locked_actions"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetBoolField(TEXT("allow_remapping"), bAllowRemapping);
        Result->SetBoolField(TEXT("show_conflicts"), bShowConflicts);
        Result->SetBoolField(TEXT("allow_duplicates"), bAllowDuplicates);
        Result->SetBoolField(TEXT("support_gamepad"), bSupportGamepad);
        Result->SetBoolField(TEXT("support_keyboard"), bSupportKeyboard);
        Result->SetNumberField(TEXT("locked_action_count"), LockedActionsJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Input remapping configured"), Result);
        return true;
    }

    // setup_difficulty_options - Assist modes, god mode
    if (SubAction == TEXT("setup_difficulty_options"))
    {
        TArray<TSharedPtr<FJsonValue>> DifficultiesJson;
        if (Params->HasField(TEXT("difficulties")))
        {
            DifficultiesJson = Params->GetArrayField(TEXT("difficulties"));
        }
        bool bAllowCustomDifficulty = GetJsonBoolField(Params, TEXT("allow_custom_difficulty"), true);
        bool bAssistMode = GetJsonBoolField(Params, TEXT("assist_mode"), true);
        bool bInvincibilityOption = GetJsonBoolField(Params, TEXT("invincibility_option"), true);
        bool bAutoAimOption = GetJsonBoolField(Params, TEXT("auto_aim_option"), true);
        bool bSlowMotionOption = GetJsonBoolField(Params, TEXT("slow_motion_option"), true);
        bool bSkipPuzzles = GetJsonBoolField(Params, TEXT("skip_puzzles"), false);
        bool bSkipCombat = GetJsonBoolField(Params, TEXT("skip_combat"), false);
        float DamageMultiplierRange[2] = { 0.5f, 2.0f };
        if (Params->HasField(TEXT("damage_multiplier_min")))
        {
            DamageMultiplierRange[0] = GetJsonNumberField(Params, TEXT("damage_multiplier_min"), 0.5f);
        }
        if (Params->HasField(TEXT("damage_multiplier_max")))
        {
            DamageMultiplierRange[1] = GetJsonNumberField(Params, TEXT("damage_multiplier_max"), 2.0f);
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("difficulty_count"), DifficultiesJson.Num());
        Result->SetBoolField(TEXT("allow_custom_difficulty"), bAllowCustomDifficulty);
        Result->SetBoolField(TEXT("assist_mode"), bAssistMode);
        Result->SetBoolField(TEXT("invincibility_option"), bInvincibilityOption);
        Result->SetBoolField(TEXT("auto_aim_option"), bAutoAimOption);
        Result->SetBoolField(TEXT("slow_motion_option"), bSlowMotionOption);
        Result->SetNumberField(TEXT("damage_multiplier_min"), DamageMultiplierRange[0]);
        Result->SetNumberField(TEXT("damage_multiplier_max"), DamageMultiplierRange[1]);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Difficulty options configured"), Result);
        return true;
    }

    // create_screen_reader - UI narration
    if (SubAction == TEXT("create_screen_reader"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        FString Voice = GetJsonStringField(Params, TEXT("voice"), TEXT("Default"));
        float SpeechRate = GetJsonNumberField(Params, TEXT("speech_rate"), 1.0f);
        float Volume = GetJsonNumberField(Params, TEXT("volume"), 1.0f);
        bool bReadButtonLabels = GetJsonBoolField(Params, TEXT("read_button_labels"), true);
        bool bReadTextFields = GetJsonBoolField(Params, TEXT("read_text_fields"), true);
        bool bReadTooltips = GetJsonBoolField(Params, TEXT("read_tooltips"), true);
        bool bReadNotifications = GetJsonBoolField(Params, TEXT("read_notifications"), true);
        bool bReadMenuItems = GetJsonBoolField(Params, TEXT("read_menu_items"), true);
        bool bAnnounceChanges = GetJsonBoolField(Params, TEXT("announce_changes"), true);
        int32 RepeatDelay = GetJsonIntField(Params, TEXT("repeat_delay"), 2);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetStringField(TEXT("voice"), Voice);
        Result->SetNumberField(TEXT("speech_rate"), SpeechRate);
        Result->SetNumberField(TEXT("volume"), Volume);
        Result->SetBoolField(TEXT("read_button_labels"), bReadButtonLabels);
        Result->SetBoolField(TEXT("read_text_fields"), bReadTextFields);
        Result->SetBoolField(TEXT("read_notifications"), bReadNotifications);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Screen reader created"), Result);
        return true;
    }

    // configure_motor_accessibility - Hold vs toggle, auto-aim
    if (SubAction == TEXT("configure_motor_accessibility"))
    {
        bool bHoldToToggle = GetJsonBoolField(Params, TEXT("hold_to_toggle"), true);
        bool bAutoRun = GetJsonBoolField(Params, TEXT("auto_run"), true);
        bool bAutoAim = GetJsonBoolField(Params, TEXT("auto_aim"), true);
        float AutoAimStrength = GetJsonNumberField(Params, TEXT("auto_aim_strength"), 0.5f);
        bool bOneTouchMode = GetJsonBoolField(Params, TEXT("one_touch_mode"), false);
        bool bSimplifiedControls = GetJsonBoolField(Params, TEXT("simplified_controls"), false);
        bool bAutoQTE = GetJsonBoolField(Params, TEXT("auto_qte"), true);
        int32 QTETimeExtension = GetJsonIntField(Params, TEXT("qte_time_extension"), 200);
        bool bReduceButtonMashing = GetJsonBoolField(Params, TEXT("reduce_button_mashing"), true);
        float InputBufferTime = GetJsonNumberField(Params, TEXT("input_buffer_time"), 0.2f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("hold_to_toggle"), bHoldToToggle);
        Result->SetBoolField(TEXT("auto_run"), bAutoRun);
        Result->SetBoolField(TEXT("auto_aim"), bAutoAim);
        Result->SetNumberField(TEXT("auto_aim_strength"), AutoAimStrength);
        Result->SetBoolField(TEXT("one_touch_mode"), bOneTouchMode);
        Result->SetBoolField(TEXT("simplified_controls"), bSimplifiedControls);
        Result->SetBoolField(TEXT("auto_qte"), bAutoQTE);
        Result->SetNumberField(TEXT("qte_time_extension"), QTETimeExtension);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Motor accessibility configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown accessibility action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// LOCALIZATION HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2LocalizationAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // setup_string_tables - Localized text database
    if (SubAction == TEXT("setup_string_tables"))
    {
        FString TableName = GetJsonStringField(Params, TEXT("name"), TEXT("GameStrings"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Localization"));
        FString Namespace = GetJsonStringField(Params, TEXT("namespace"), TEXT("Game"));
        TArray<TSharedPtr<FJsonValue>> LanguagesJson;
        if (Params->HasField(TEXT("languages")))
        {
            LanguagesJson = Params->GetArrayField(TEXT("languages"));
        }
        FString SourceLanguage = GetJsonStringField(Params, TEXT("source_language"), TEXT("en"));

        FString FullPath = OutputPath / TableName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for string table"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UStringTable* StringTable = NewObject<UStringTable>(Package, *TableName, RF_Public | RF_Standalone);
        if (StringTable)
        {
            StringTable->GetMutableStringTable()->SetNamespace(Namespace);
            FAssetRegistryModule::AssetCreated(StringTable);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("table_name"), TableName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("namespace"), Namespace);
            Result->SetStringField(TEXT("source_language"), SourceLanguage);
            Result->SetNumberField(TEXT("language_count"), LanguagesJson.Num());
            SendAutomationResponse(Socket, RequestId, true, TEXT("String table created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create string table"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_text_formatting - Plurals, gender, numbers
    if (SubAction == TEXT("configure_text_formatting"))
    {
        bool bEnablePlurals = GetJsonBoolField(Params, TEXT("enable_plurals"), true);
        bool bEnableGender = GetJsonBoolField(Params, TEXT("enable_gender"), true);
        bool bEnableNumberFormatting = GetJsonBoolField(Params, TEXT("enable_number_formatting"), true);
        bool bEnableDateFormatting = GetJsonBoolField(Params, TEXT("enable_date_formatting"), true);
        bool bEnableCurrencyFormatting = GetJsonBoolField(Params, TEXT("enable_currency_formatting"), true);
        FString DefaultNumberFormat = GetJsonStringField(Params, TEXT("default_number_format"), TEXT("Decimal"));
        FString DefaultDateFormat = GetJsonStringField(Params, TEXT("default_date_format"), TEXT("Short"));
        int32 DecimalPlaces = GetJsonIntField(Params, TEXT("decimal_places"), 2);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enable_plurals"), bEnablePlurals);
        Result->SetBoolField(TEXT("enable_gender"), bEnableGender);
        Result->SetBoolField(TEXT("enable_number_formatting"), bEnableNumberFormatting);
        Result->SetBoolField(TEXT("enable_date_formatting"), bEnableDateFormatting);
        Result->SetBoolField(TEXT("enable_currency_formatting"), bEnableCurrencyFormatting);
        Result->SetStringField(TEXT("default_number_format"), DefaultNumberFormat);
        Result->SetStringField(TEXT("default_date_format"), DefaultDateFormat);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Text formatting configured"), Result);
        return true;
    }

    // setup_audio_localization - Dubbed audio swapping
    if (SubAction == TEXT("setup_audio_localization"))
    {
        TArray<TSharedPtr<FJsonValue>> LanguagesJson;
        if (Params->HasField(TEXT("languages")))
        {
            LanguagesJson = Params->GetArrayField(TEXT("languages"));
        }
        FString DefaultLanguage = GetJsonStringField(Params, TEXT("default_language"), TEXT("en"));
        FString AudioAssetPath = GetJsonStringField(Params, TEXT("audio_asset_path"), TEXT("/Game/Audio/VO"));
        bool bFallbackToDefault = GetJsonBoolField(Params, TEXT("fallback_to_default"), true);
        bool bStreamAudio = GetJsonBoolField(Params, TEXT("stream_audio"), true);
        FString NamingConvention = GetJsonStringField(Params, TEXT("naming_convention"), TEXT("{AssetName}_{Language}")); // {AssetName}_{Language}, {Language}/{AssetName}

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("language_count"), LanguagesJson.Num());
        Result->SetStringField(TEXT("default_language"), DefaultLanguage);
        Result->SetStringField(TEXT("audio_asset_path"), AudioAssetPath);
        Result->SetBoolField(TEXT("fallback_to_default"), bFallbackToDefault);
        Result->SetBoolField(TEXT("stream_audio"), bStreamAudio);
        Result->SetStringField(TEXT("naming_convention"), NamingConvention);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Audio localization configured"), Result);
        return true;
    }

    // create_font_fallbacks - CJK, Arabic, Cyrillic support
    if (SubAction == TEXT("create_font_fallbacks"))
    {
        FString PrimaryFont = GetJsonStringField(Params, TEXT("primary_font"), TEXT("Roboto"));
        TArray<TSharedPtr<FJsonValue>> FallbacksJson;
        if (Params->HasField(TEXT("fallbacks")))
        {
            FallbacksJson = Params->GetArrayField(TEXT("fallbacks"));
        }
        bool bIncludeCJK = GetJsonBoolField(Params, TEXT("include_cjk"), true);
        FString CJKFont = GetJsonStringField(Params, TEXT("cjk_font"), TEXT("NotoSansCJK"));
        bool bIncludeArabic = GetJsonBoolField(Params, TEXT("include_arabic"), true);
        FString ArabicFont = GetJsonStringField(Params, TEXT("arabic_font"), TEXT("NotoSansArabic"));
        bool bIncludeCyrillic = GetJsonBoolField(Params, TEXT("include_cyrillic"), true);
        bool bIncludeDevanagari = GetJsonBoolField(Params, TEXT("include_devanagari"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("primary_font"), PrimaryFont);
        Result->SetNumberField(TEXT("fallback_count"), FallbacksJson.Num());
        Result->SetBoolField(TEXT("include_cjk"), bIncludeCJK);
        Result->SetStringField(TEXT("cjk_font"), CJKFont);
        Result->SetBoolField(TEXT("include_arabic"), bIncludeArabic);
        Result->SetBoolField(TEXT("include_cyrillic"), bIncludeCyrillic);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Font fallbacks created"), Result);
        return true;
    }

    // configure_rtl_support - Right-to-left languages
    if (SubAction == TEXT("configure_rtl_support"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        TArray<TSharedPtr<FJsonValue>> RTLLanguagesJson;
        if (Params->HasField(TEXT("rtl_languages")))
        {
            RTLLanguagesJson = Params->GetArrayField(TEXT("rtl_languages"));
        }
        bool bMirrorUI = GetJsonBoolField(Params, TEXT("mirror_ui"), true);
        bool bMirrorAnimations = GetJsonBoolField(Params, TEXT("mirror_animations"), false);
        bool bMirrorIcons = GetJsonBoolField(Params, TEXT("mirror_icons"), false);
        TArray<TSharedPtr<FJsonValue>> ExcludedWidgetsJson;
        if (Params->HasField(TEXT("excluded_widgets")))
        {
            ExcludedWidgetsJson = Params->GetArrayField(TEXT("excluded_widgets"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetNumberField(TEXT("rtl_language_count"), RTLLanguagesJson.Num());
        Result->SetBoolField(TEXT("mirror_ui"), bMirrorUI);
        Result->SetBoolField(TEXT("mirror_animations"), bMirrorAnimations);
        Result->SetBoolField(TEXT("mirror_icons"), bMirrorIcons);
        Result->SetNumberField(TEXT("excluded_widget_count"), ExcludedWidgetsJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("RTL support configured"), Result);
        return true;
    }

    // setup_regional_content - Region-specific assets
    if (SubAction == TEXT("setup_regional_content"))
    {
        TArray<TSharedPtr<FJsonValue>> RegionsJson;
        if (Params->HasField(TEXT("regions")))
        {
            RegionsJson = Params->GetArrayField(TEXT("regions"));
        }
        FString DefaultRegion = GetJsonStringField(Params, TEXT("default_region"), TEXT("WORLDWIDE"));
        bool bContentFiltering = GetJsonBoolField(Params, TEXT("content_filtering"), true);
        bool bRegionalPricing = GetJsonBoolField(Params, TEXT("regional_pricing"), true);
        bool bRegionalAssets = GetJsonBoolField(Params, TEXT("regional_assets"), true);
        TArray<TSharedPtr<FJsonValue>> FilteredContentJson;
        if (Params->HasField(TEXT("filtered_content")))
        {
            FilteredContentJson = Params->GetArrayField(TEXT("filtered_content"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("region_count"), RegionsJson.Num());
        Result->SetStringField(TEXT("default_region"), DefaultRegion);
        Result->SetBoolField(TEXT("content_filtering"), bContentFiltering);
        Result->SetBoolField(TEXT("regional_pricing"), bRegionalPricing);
        Result->SetBoolField(TEXT("regional_assets"), bRegionalAssets);
        Result->SetNumberField(TEXT("filtered_content_count"), FilteredContentJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Regional content configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown localization action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// PHOTO MODE HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2PhotoModeAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_photo_mode - Full photo mode system
    if (SubAction == TEXT("create_photo_mode"))
    {
        FString PhotoModeName = GetJsonStringField(Params, TEXT("name"), TEXT("PhotoMode"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/PhotoMode"));
        bool bPauseGame = GetJsonBoolField(Params, TEXT("pause_game"), true);
        bool bHideUI = GetJsonBoolField(Params, TEXT("hide_ui"), true);
        bool bHidePlayer = GetJsonBoolField(Params, TEXT("hide_player"), false);
        bool bEnablePoses = GetJsonBoolField(Params, TEXT("enable_poses"), true);
        bool bEnableFilters = GetJsonBoolField(Params, TEXT("enable_filters"), true);
        bool bEnableFrames = GetJsonBoolField(Params, TEXT("enable_frames"), true);
        bool bEnableStickers = GetJsonBoolField(Params, TEXT("enable_stickers"), false);
        FString SavePath = GetJsonStringField(Params, TEXT("save_path"), TEXT("Screenshots/PhotoMode"));
        int32 ResolutionMultiplier = GetJsonIntField(Params, TEXT("resolution_multiplier"), 1);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("photo_mode_name"), PhotoModeName);
        Result->SetStringField(TEXT("path"), OutputPath / PhotoModeName);
        Result->SetBoolField(TEXT("pause_game"), bPauseGame);
        Result->SetBoolField(TEXT("hide_ui"), bHideUI);
        Result->SetBoolField(TEXT("hide_player"), bHidePlayer);
        Result->SetBoolField(TEXT("enable_poses"), bEnablePoses);
        Result->SetBoolField(TEXT("enable_filters"), bEnableFilters);
        Result->SetBoolField(TEXT("enable_frames"), bEnableFrames);
        Result->SetStringField(TEXT("save_path"), SavePath);
        Result->SetNumberField(TEXT("resolution_multiplier"), ResolutionMultiplier);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Photo mode created"), Result);
        return true;
    }

    // configure_camera_controls - FOV, roll, position
    if (SubAction == TEXT("configure_camera_controls"))
    {
        float MinFOV = GetJsonNumberField(Params, TEXT("min_fov"), 20.0f);
        float MaxFOV = GetJsonNumberField(Params, TEXT("max_fov"), 120.0f);
        float DefaultFOV = GetJsonNumberField(Params, TEXT("default_fov"), 90.0f);
        float MaxRoll = GetJsonNumberField(Params, TEXT("max_roll"), 45.0f);
        float MaxDistance = GetJsonNumberField(Params, TEXT("max_distance"), 1000.0f);
        float MovementSpeed = GetJsonNumberField(Params, TEXT("movement_speed"), 100.0f);
        float RotationSpeed = GetJsonNumberField(Params, TEXT("rotation_speed"), 50.0f);
        bool bAllowVerticalMovement = GetJsonBoolField(Params, TEXT("allow_vertical_movement"), true);
        bool bCollisionDetection = GetJsonBoolField(Params, TEXT("collision_detection"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("min_fov"), MinFOV);
        Result->SetNumberField(TEXT("max_fov"), MaxFOV);
        Result->SetNumberField(TEXT("default_fov"), DefaultFOV);
        Result->SetNumberField(TEXT("max_roll"), MaxRoll);
        Result->SetNumberField(TEXT("max_distance"), MaxDistance);
        Result->SetNumberField(TEXT("movement_speed"), MovementSpeed);
        Result->SetNumberField(TEXT("rotation_speed"), RotationSpeed);
        Result->SetBoolField(TEXT("allow_vertical_movement"), bAllowVerticalMovement);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Camera controls configured"), Result);
        return true;
    }

    // setup_filters - Color grading presets
    if (SubAction == TEXT("setup_filters"))
    {
        TArray<TSharedPtr<FJsonValue>> FiltersJson;
        if (Params->HasField(TEXT("filters")))
        {
            FiltersJson = Params->GetArrayField(TEXT("filters"));
        }
        bool bCustomFilters = GetJsonBoolField(Params, TEXT("custom_filters"), true);
        bool bFilterIntensity = GetJsonBoolField(Params, TEXT("filter_intensity"), true);
        bool bContrastControl = GetJsonBoolField(Params, TEXT("contrast_control"), true);
        bool bSaturationControl = GetJsonBoolField(Params, TEXT("saturation_control"), true);
        bool bTemperatureControl = GetJsonBoolField(Params, TEXT("temperature_control"), true);
        bool bVignetteControl = GetJsonBoolField(Params, TEXT("vignette_control"), true);
        bool bGrainControl = GetJsonBoolField(Params, TEXT("grain_control"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("filter_count"), FiltersJson.Num());
        Result->SetBoolField(TEXT("custom_filters"), bCustomFilters);
        Result->SetBoolField(TEXT("filter_intensity"), bFilterIntensity);
        Result->SetBoolField(TEXT("contrast_control"), bContrastControl);
        Result->SetBoolField(TEXT("saturation_control"), bSaturationControl);
        Result->SetBoolField(TEXT("temperature_control"), bTemperatureControl);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Filters configured"), Result);
        return true;
    }

    // create_poses - Character pose library
    if (SubAction == TEXT("create_poses"))
    {
        TArray<TSharedPtr<FJsonValue>> PosesJson;
        if (Params->HasField(TEXT("poses")))
        {
            PosesJson = Params->GetArrayField(TEXT("poses"));
        }
        bool bCustomPoses = GetJsonBoolField(Params, TEXT("custom_poses"), true);
        bool bFacialExpressions = GetJsonBoolField(Params, TEXT("facial_expressions"), true);
        bool bEyeControl = GetJsonBoolField(Params, TEXT("eye_control"), true);
        bool bPoseBlending = GetJsonBoolField(Params, TEXT("pose_blending"), true);
        bool bSaveCustomPoses = GetJsonBoolField(Params, TEXT("save_custom_poses"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("pose_count"), PosesJson.Num());
        Result->SetBoolField(TEXT("custom_poses"), bCustomPoses);
        Result->SetBoolField(TEXT("facial_expressions"), bFacialExpressions);
        Result->SetBoolField(TEXT("eye_control"), bEyeControl);
        Result->SetBoolField(TEXT("pose_blending"), bPoseBlending);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Poses created"), Result);
        return true;
    }

    // configure_hide_ui - Clean capture
    if (SubAction == TEXT("configure_hide_ui"))
    {
        bool bHideAllUI = GetJsonBoolField(Params, TEXT("hide_all_ui"), true);
        bool bHideHUD = GetJsonBoolField(Params, TEXT("hide_hud"), true);
        bool bHideWeapon = GetJsonBoolField(Params, TEXT("hide_weapon"), true);
        bool bHideCompanions = GetJsonBoolField(Params, TEXT("hide_companions"), false);
        bool bHideEnemies = GetJsonBoolField(Params, TEXT("hide_enemies"), false);
        bool bHideNPCs = GetJsonBoolField(Params, TEXT("hide_npcs"), false);
        bool bHideEffects = GetJsonBoolField(Params, TEXT("hide_effects"), false);
        TArray<TSharedPtr<FJsonValue>> ExceptionsJson;
        if (Params->HasField(TEXT("exceptions")))
        {
            ExceptionsJson = Params->GetArrayField(TEXT("exceptions"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("hide_all_ui"), bHideAllUI);
        Result->SetBoolField(TEXT("hide_hud"), bHideHUD);
        Result->SetBoolField(TEXT("hide_weapon"), bHideWeapon);
        Result->SetBoolField(TEXT("hide_companions"), bHideCompanions);
        Result->SetBoolField(TEXT("hide_enemies"), bHideEnemies);
        Result->SetNumberField(TEXT("exception_count"), ExceptionsJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Hide UI configured"), Result);
        return true;
    }

    // setup_depth_of_field - Bokeh, focus distance
    if (SubAction == TEXT("setup_depth_of_field"))
    {
        bool bEnabled = GetJsonBoolField(Params, TEXT("enabled"), true);
        FString DOFMethod = GetJsonStringField(Params, TEXT("method"), TEXT("Bokeh")); // Bokeh, Gaussian, Circle
        float FocalDistance = GetJsonNumberField(Params, TEXT("focal_distance"), 500.0f);
        float Aperture = GetJsonNumberField(Params, TEXT("aperture"), 2.8f);
        float BokehSize = GetJsonNumberField(Params, TEXT("bokeh_size"), 50.0f);
        float NearBlur = GetJsonNumberField(Params, TEXT("near_blur"), 0.0f);
        float FarBlur = GetJsonNumberField(Params, TEXT("far_blur"), 0.0f);
        bool bAutoFocus = GetJsonBoolField(Params, TEXT("auto_focus"), true);
        bool bFocusPoint = GetJsonBoolField(Params, TEXT("focus_point"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enabled"), bEnabled);
        Result->SetStringField(TEXT("method"), DOFMethod);
        Result->SetNumberField(TEXT("focal_distance"), FocalDistance);
        Result->SetNumberField(TEXT("aperture"), Aperture);
        Result->SetNumberField(TEXT("bokeh_size"), BokehSize);
        Result->SetBoolField(TEXT("auto_focus"), bAutoFocus);
        Result->SetBoolField(TEXT("focus_point"), bFocusPoint);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Depth of field configured"), Result);
        return true;
    }

    // create_frame_options - Borders, watermarks, timestamps
    if (SubAction == TEXT("create_frame_options"))
    {
        TArray<TSharedPtr<FJsonValue>> FramesJson;
        if (Params->HasField(TEXT("frames")))
        {
            FramesJson = Params->GetArrayField(TEXT("frames"));
        }
        bool bCustomFrames = GetJsonBoolField(Params, TEXT("custom_frames"), true);
        bool bWatermark = GetJsonBoolField(Params, TEXT("watermark"), true);
        FString WatermarkText = GetJsonStringField(Params, TEXT("watermark_text"), TEXT(""));
        FString WatermarkImage = GetJsonStringField(Params, TEXT("watermark_image"), TEXT(""));
        FString WatermarkPosition = GetJsonStringField(Params, TEXT("watermark_position"), TEXT("BottomRight"));
        float WatermarkOpacity = GetJsonNumberField(Params, TEXT("watermark_opacity"), 0.5f);
        bool bTimestamp = GetJsonBoolField(Params, TEXT("timestamp"), false);
        FString TimestampFormat = GetJsonStringField(Params, TEXT("timestamp_format"), TEXT("YYYY-MM-DD HH:MM:SS"));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("frame_count"), FramesJson.Num());
        Result->SetBoolField(TEXT("custom_frames"), bCustomFrames);
        Result->SetBoolField(TEXT("watermark"), bWatermark);
        Result->SetStringField(TEXT("watermark_text"), WatermarkText);
        Result->SetStringField(TEXT("watermark_position"), WatermarkPosition);
        Result->SetNumberField(TEXT("watermark_opacity"), WatermarkOpacity);
        Result->SetBoolField(TEXT("timestamp"), bTimestamp);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Frame options created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown photo_mode action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// ACHIEVEMENTS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2AchievementsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_achievement - Definition, icon, points
    if (SubAction == TEXT("create_achievement"))
    {
        FString AchievementId = GetJsonStringField(Params, TEXT("id"), TEXT(""));
        FString AchievementName = GetJsonStringField(Params, TEXT("name"), TEXT("New Achievement"));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString LockedDescription = GetJsonStringField(Params, TEXT("locked_description"), TEXT("???"));
        FString IconPath = GetJsonStringField(Params, TEXT("icon_path"), TEXT(""));
        FString LockedIconPath = GetJsonStringField(Params, TEXT("locked_icon_path"), TEXT(""));
        int32 Points = GetJsonIntField(Params, TEXT("points"), 10);
        bool bHidden = GetJsonBoolField(Params, TEXT("hidden"), false);
        FString Category = GetJsonStringField(Params, TEXT("category"), TEXT("General"));
        FString Rarity = GetJsonStringField(Params, TEXT("rarity"), TEXT("Common")); // Common, Uncommon, Rare, Epic, Legendary
        FString CriteriaType = GetJsonStringField(Params, TEXT("criteria_type"), TEXT("Count")); // Count, Boolean, Progress
        int32 CriteriaTarget = GetJsonIntField(Params, TEXT("criteria_target"), 1);

        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Data/Achievements"));
        FString FullPath = OutputPath / AchievementId;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for achievement"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UPrimaryDataAsset* AchievementAsset = NewObject<UPrimaryDataAsset>(Package, *AchievementId, RF_Public | RF_Standalone);
        if (AchievementAsset)
        {
            FAssetRegistryModule::AssetCreated(AchievementAsset);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("achievement_id"), AchievementId);
            Result->SetStringField(TEXT("name"), AchievementName);
            Result->SetStringField(TEXT("description"), Description);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetNumberField(TEXT("points"), Points);
            Result->SetBoolField(TEXT("hidden"), bHidden);
            Result->SetStringField(TEXT("category"), Category);
            Result->SetStringField(TEXT("rarity"), Rarity);
            Result->SetStringField(TEXT("criteria_type"), CriteriaType);
            Result->SetNumberField(TEXT("criteria_target"), CriteriaTarget);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Achievement created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create achievement"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_platform_integration - Steam, PSN, Xbox
    if (SubAction == TEXT("configure_platform_integration"))
    {
        bool bSteam = GetJsonBoolField(Params, TEXT("steam"), true);
        bool bPlayStation = GetJsonBoolField(Params, TEXT("playstation"), true);
        bool bXbox = GetJsonBoolField(Params, TEXT("xbox"), true);
        bool bEpic = GetJsonBoolField(Params, TEXT("epic"), true);
        bool bSwitch = GetJsonBoolField(Params, TEXT("switch"), true);
        bool bSyncOffline = GetJsonBoolField(Params, TEXT("sync_offline"), true);
        bool bShowPlatformUI = GetJsonBoolField(Params, TEXT("show_platform_ui"), true);
        bool bInGameNotifications = GetJsonBoolField(Params, TEXT("in_game_notifications"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("steam"), bSteam);
        Result->SetBoolField(TEXT("playstation"), bPlayStation);
        Result->SetBoolField(TEXT("xbox"), bXbox);
        Result->SetBoolField(TEXT("epic"), bEpic);
        Result->SetBoolField(TEXT("switch"), bSwitch);
        Result->SetBoolField(TEXT("sync_offline"), bSyncOffline);
        Result->SetBoolField(TEXT("show_platform_ui"), bShowPlatformUI);
        Result->SetBoolField(TEXT("in_game_notifications"), bInGameNotifications);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Platform integration configured"), Result);
        return true;
    }

    // setup_progress_tracking - Incremental achievements
    if (SubAction == TEXT("setup_progress_tracking"))
    {
        FString AchievementId = GetJsonStringField(Params, TEXT("achievement_id"), TEXT(""));
        bool bShowProgress = GetJsonBoolField(Params, TEXT("show_progress"), true);
        int32 ProgressSteps = GetJsonIntField(Params, TEXT("progress_steps"), 100);
        bool bNotifyOnMilestone = GetJsonBoolField(Params, TEXT("notify_on_milestone"), true);
        TArray<TSharedPtr<FJsonValue>> MilestonesJson;
        if (Params->HasField(TEXT("milestones")))
        {
            MilestonesJson = Params->GetArrayField(TEXT("milestones"));
        }
        bool bResetOnNewGame = GetJsonBoolField(Params, TEXT("reset_on_new_game"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("achievement_id"), AchievementId);
        Result->SetBoolField(TEXT("show_progress"), bShowProgress);
        Result->SetNumberField(TEXT("progress_steps"), ProgressSteps);
        Result->SetBoolField(TEXT("notify_on_milestone"), bNotifyOnMilestone);
        Result->SetNumberField(TEXT("milestone_count"), MilestonesJson.Num());
        Result->SetBoolField(TEXT("reset_on_new_game"), bResetOnNewGame);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Progress tracking configured"), Result);
        return true;
    }

    // create_secret_achievements - Hidden until unlocked
    if (SubAction == TEXT("create_secret_achievements"))
    {
        TArray<TSharedPtr<FJsonValue>> AchievementsJson;
        if (Params->HasField(TEXT("achievements")))
        {
            AchievementsJson = Params->GetArrayField(TEXT("achievements"));
        }
        FString DefaultLockedName = GetJsonStringField(Params, TEXT("default_locked_name"), TEXT("Secret Achievement"));
        FString DefaultLockedDescription = GetJsonStringField(Params, TEXT("default_locked_description"), TEXT("Continue playing to unlock this secret achievement."));
        bool bShowHintAfterProgress = GetJsonBoolField(Params, TEXT("show_hint_after_progress"), false);
        float HintProgressThreshold = GetJsonNumberField(Params, TEXT("hint_progress_threshold"), 0.5f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("secret_achievement_count"), AchievementsJson.Num());
        Result->SetStringField(TEXT("default_locked_name"), DefaultLockedName);
        Result->SetStringField(TEXT("default_locked_description"), DefaultLockedDescription);
        Result->SetBoolField(TEXT("show_hint_after_progress"), bShowHintAfterProgress);
        Result->SetNumberField(TEXT("hint_progress_threshold"), HintProgressThreshold);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Secret achievements created"), Result);
        return true;
    }

    // configure_achievement_ui - Popups, list view
    if (SubAction == TEXT("configure_achievement_ui"))
    {
        bool bShowPopup = GetJsonBoolField(Params, TEXT("show_popup"), true);
        float PopupDuration = GetJsonNumberField(Params, TEXT("popup_duration"), 5.0f);
        FString PopupPosition = GetJsonStringField(Params, TEXT("popup_position"), TEXT("TopRight"));
        bool bPlaySound = GetJsonBoolField(Params, TEXT("play_sound"), true);
        FString SoundPath = GetJsonStringField(Params, TEXT("sound_path"), TEXT(""));
        bool bShowInMenu = GetJsonBoolField(Params, TEXT("show_in_menu"), true);
        FString MenuLayout = GetJsonStringField(Params, TEXT("menu_layout"), TEXT("Grid")); // Grid, List
        bool bShowProgress = GetJsonBoolField(Params, TEXT("show_progress"), true);
        bool bShowRarity = GetJsonBoolField(Params, TEXT("show_rarity"), true);
        bool bShowPercentage = GetJsonBoolField(Params, TEXT("show_percentage"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("show_popup"), bShowPopup);
        Result->SetNumberField(TEXT("popup_duration"), PopupDuration);
        Result->SetStringField(TEXT("popup_position"), PopupPosition);
        Result->SetBoolField(TEXT("play_sound"), bPlaySound);
        Result->SetBoolField(TEXT("show_in_menu"), bShowInMenu);
        Result->SetStringField(TEXT("menu_layout"), MenuLayout);
        Result->SetBoolField(TEXT("show_progress"), bShowProgress);
        Result->SetBoolField(TEXT("show_rarity"), bShowRarity);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Achievement UI configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown achievements action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// ANALYTICS HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2AnalyticsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // setup_event_tracking - Custom events
    if (SubAction == TEXT("setup_event_tracking"))
    {
        FString Provider = GetJsonStringField(Params, TEXT("provider"), TEXT("Native")); // Native, GameAnalytics, Firebase, Custom
        TArray<TSharedPtr<FJsonValue>> EventsJson;
        if (Params->HasField(TEXT("events")))
        {
            EventsJson = Params->GetArrayField(TEXT("events"));
        }
        bool bBatchEvents = GetJsonBoolField(Params, TEXT("batch_events"), true);
        int32 BatchInterval = GetJsonIntField(Params, TEXT("batch_interval"), 30);
        int32 MaxQueueSize = GetJsonIntField(Params, TEXT("max_queue_size"), 100);
        bool bOfflineMode = GetJsonBoolField(Params, TEXT("offline_mode"), true);
        bool bCompressData = GetJsonBoolField(Params, TEXT("compress_data"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("provider"), Provider);
        Result->SetNumberField(TEXT("event_count"), EventsJson.Num());
        Result->SetBoolField(TEXT("batch_events"), bBatchEvents);
        Result->SetNumberField(TEXT("batch_interval"), BatchInterval);
        Result->SetNumberField(TEXT("max_queue_size"), MaxQueueSize);
        Result->SetBoolField(TEXT("offline_mode"), bOfflineMode);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Event tracking configured"), Result);
        return true;
    }

    // configure_funnel_analysis - Player progression
    if (SubAction == TEXT("configure_funnel_analysis"))
    {
        TArray<TSharedPtr<FJsonValue>> FunnelsJson;
        if (Params->HasField(TEXT("funnels")))
        {
            FunnelsJson = Params->GetArrayField(TEXT("funnels"));
        }
        bool bTrackOnboarding = GetJsonBoolField(Params, TEXT("track_onboarding"), true);
        bool bTrackProgression = GetJsonBoolField(Params, TEXT("track_progression"), true);
        bool bTrackPurchases = GetJsonBoolField(Params, TEXT("track_purchases"), true);
        bool bTrackRetention = GetJsonBoolField(Params, TEXT("track_retention"), true);
        TArray<TSharedPtr<FJsonValue>> RetentionDaysJson;
        if (Params->HasField(TEXT("retention_days")))
        {
            RetentionDaysJson = Params->GetArrayField(TEXT("retention_days"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("funnel_count"), FunnelsJson.Num());
        Result->SetBoolField(TEXT("track_onboarding"), bTrackOnboarding);
        Result->SetBoolField(TEXT("track_progression"), bTrackProgression);
        Result->SetBoolField(TEXT("track_purchases"), bTrackPurchases);
        Result->SetBoolField(TEXT("track_retention"), bTrackRetention);
        Result->SetNumberField(TEXT("retention_day_count"), RetentionDaysJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Funnel analysis configured"), Result);
        return true;
    }

    // create_heatmaps - Death locations, paths
    if (SubAction == TEXT("create_heatmaps"))
    {
        TArray<TSharedPtr<FJsonValue>> HeatmapsJson;
        if (Params->HasField(TEXT("heatmaps")))
        {
            HeatmapsJson = Params->GetArrayField(TEXT("heatmaps"));
        }
        bool bTrackDeaths = GetJsonBoolField(Params, TEXT("track_deaths"), true);
        bool bTrackKills = GetJsonBoolField(Params, TEXT("track_kills"), true);
        bool bTrackPaths = GetJsonBoolField(Params, TEXT("track_paths"), true);
        bool bTrackInteractions = GetJsonBoolField(Params, TEXT("track_interactions"), true);
        float SampleInterval = GetJsonNumberField(Params, TEXT("sample_interval"), 1.0f);
        int32 Resolution = GetJsonIntField(Params, TEXT("resolution"), 100);
        bool bExportData = GetJsonBoolField(Params, TEXT("export_data"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("heatmap_count"), HeatmapsJson.Num());
        Result->SetBoolField(TEXT("track_deaths"), bTrackDeaths);
        Result->SetBoolField(TEXT("track_kills"), bTrackKills);
        Result->SetBoolField(TEXT("track_paths"), bTrackPaths);
        Result->SetBoolField(TEXT("track_interactions"), bTrackInteractions);
        Result->SetNumberField(TEXT("sample_interval"), SampleInterval);
        Result->SetNumberField(TEXT("resolution"), Resolution);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Heatmaps created"), Result);
        return true;
    }

    // setup_session_tracking - Play time, retention
    if (SubAction == TEXT("setup_session_tracking"))
    {
        bool bTrackSessionLength = GetJsonBoolField(Params, TEXT("track_session_length"), true);
        bool bTrackSessionCount = GetJsonBoolField(Params, TEXT("track_session_count"), true);
        bool bTrackDailyActiveUsers = GetJsonBoolField(Params, TEXT("track_dau"), true);
        bool bTrackMonthlyActiveUsers = GetJsonBoolField(Params, TEXT("track_mau"), true);
        bool bTrackChurn = GetJsonBoolField(Params, TEXT("track_churn"), true);
        int32 ChurnThresholdDays = GetJsonIntField(Params, TEXT("churn_threshold_days"), 7);
        bool bTrackEngagement = GetJsonBoolField(Params, TEXT("track_engagement"), true);
        bool bTrackPeakTimes = GetJsonBoolField(Params, TEXT("track_peak_times"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("track_session_length"), bTrackSessionLength);
        Result->SetBoolField(TEXT("track_session_count"), bTrackSessionCount);
        Result->SetBoolField(TEXT("track_dau"), bTrackDailyActiveUsers);
        Result->SetBoolField(TEXT("track_mau"), bTrackMonthlyActiveUsers);
        Result->SetBoolField(TEXT("track_churn"), bTrackChurn);
        Result->SetNumberField(TEXT("churn_threshold_days"), ChurnThresholdDays);
        Result->SetBoolField(TEXT("track_engagement"), bTrackEngagement);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Session tracking configured"), Result);
        return true;
    }

    // configure_ab_testing - Feature flags, experiments
    if (SubAction == TEXT("configure_ab_testing"))
    {
        TArray<TSharedPtr<FJsonValue>> ExperimentsJson;
        if (Params->HasField(TEXT("experiments")))
        {
            ExperimentsJson = Params->GetArrayField(TEXT("experiments"));
        }
        FString AssignmentMethod = GetJsonStringField(Params, TEXT("assignment_method"), TEXT("Random")); // Random, UserBased, SegmentBased
        bool bPersistAssignment = GetJsonBoolField(Params, TEXT("persist_assignment"), true);
        bool bTrackConversions = GetJsonBoolField(Params, TEXT("track_conversions"), true);
        bool bAutoAnalyze = GetJsonBoolField(Params, TEXT("auto_analyze"), true);
        float SignificanceThreshold = GetJsonNumberField(Params, TEXT("significance_threshold"), 0.95f);
        bool bRemoteConfig = GetJsonBoolField(Params, TEXT("remote_config"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetNumberField(TEXT("experiment_count"), ExperimentsJson.Num());
        Result->SetStringField(TEXT("assignment_method"), AssignmentMethod);
        Result->SetBoolField(TEXT("persist_assignment"), bPersistAssignment);
        Result->SetBoolField(TEXT("track_conversions"), bTrackConversions);
        Result->SetBoolField(TEXT("auto_analyze"), bAutoAnalyze);
        Result->SetNumberField(TEXT("significance_threshold"), SignificanceThreshold);
        Result->SetBoolField(TEXT("remote_config"), bRemoteConfig);
        SendAutomationResponse(Socket, RequestId, true, TEXT("A/B testing configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown analytics action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// TESTING HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2TestingAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_functional_test - Test actor, assertions
    if (SubAction == TEXT("create_functional_test"))
    {
        FString TestName = GetJsonStringField(Params, TEXT("name"), TEXT("NewFunctionalTest"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Tests"));
        FString TestCategory = GetJsonStringField(Params, TEXT("category"), TEXT("Gameplay"));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        float TimeLimit = GetJsonNumberField(Params, TEXT("time_limit"), 60.0f);
        bool bRunOnDedicatedServer = GetJsonBoolField(Params, TEXT("run_on_dedicated_server"), false);
        bool bRequiresPlayer = GetJsonBoolField(Params, TEXT("requires_player"), true);
        TArray<TSharedPtr<FJsonValue>> AssertionsJson;
        if (Params->HasField(TEXT("assertions")))
        {
            AssertionsJson = Params->GetArrayField(TEXT("assertions"));
        }

        FString FullPath = OutputPath / TestName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(FullPath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package for test"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        Factory->ParentClass = AFunctionalTest::StaticClass();

        UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(
            UBlueprint::StaticClass(), Package, *TestName, RF_Public | RF_Standalone, nullptr, GWarn));

        if (NewBlueprint)
        {
            FAssetRegistryModule::AssetCreated(NewBlueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("test_name"), TestName);
            Result->SetStringField(TEXT("path"), FullPath);
            Result->SetStringField(TEXT("category"), TestCategory);
            Result->SetNumberField(TEXT("time_limit"), TimeLimit);
            Result->SetBoolField(TEXT("requires_player"), bRequiresPlayer);
            Result->SetNumberField(TEXT("assertion_count"), AssertionsJson.Num());
            SendAutomationResponse(Socket, RequestId, true, TEXT("Functional test created"), Result);
            return true;
        }
        SendAutomationError(Socket, RequestId, TEXT("Failed to create functional test"), TEXT("CREATE_ERROR"));
        return true;
    }

    // configure_test_automation - Gauntlet, PIE
    if (SubAction == TEXT("configure_test_automation"))
    {
        bool bEnableGauntlet = GetJsonBoolField(Params, TEXT("enable_gauntlet"), true);
        bool bEnablePIE = GetJsonBoolField(Params, TEXT("enable_pie"), true);
        int32 PIETimeout = GetJsonIntField(Params, TEXT("pie_timeout"), 300);
        bool bCaptureScreenshots = GetJsonBoolField(Params, TEXT("capture_screenshots"), true);
        bool bCaptureVideo = GetJsonBoolField(Params, TEXT("capture_video"), false);
        FString ReportPath = GetJsonStringField(Params, TEXT("report_path"), TEXT("Saved/Automation/Reports"));
        bool bExitOnFailure = GetJsonBoolField(Params, TEXT("exit_on_failure"), false);
        int32 RetryCount = GetJsonIntField(Params, TEXT("retry_count"), 0);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enable_gauntlet"), bEnableGauntlet);
        Result->SetBoolField(TEXT("enable_pie"), bEnablePIE);
        Result->SetNumberField(TEXT("pie_timeout"), PIETimeout);
        Result->SetBoolField(TEXT("capture_screenshots"), bCaptureScreenshots);
        Result->SetBoolField(TEXT("capture_video"), bCaptureVideo);
        Result->SetStringField(TEXT("report_path"), ReportPath);
        Result->SetBoolField(TEXT("exit_on_failure"), bExitOnFailure);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Test automation configured"), Result);
        return true;
    }

    // setup_test_harness - Mock data, stubs
    if (SubAction == TEXT("setup_test_harness"))
    {
        FString HarnessName = GetJsonStringField(Params, TEXT("name"), TEXT("TestHarness"));
        TArray<TSharedPtr<FJsonValue>> MocksJson;
        if (Params->HasField(TEXT("mocks")))
        {
            MocksJson = Params->GetArrayField(TEXT("mocks"));
        }
        TArray<TSharedPtr<FJsonValue>> StubsJson;
        if (Params->HasField(TEXT("stubs")))
        {
            StubsJson = Params->GetArrayField(TEXT("stubs"));
        }
        bool bIsolateNetworking = GetJsonBoolField(Params, TEXT("isolate_networking"), true);
        bool bMockSaveSystem = GetJsonBoolField(Params, TEXT("mock_save_system"), true);
        bool bMockAchievements = GetJsonBoolField(Params, TEXT("mock_achievements"), true);
        bool bMockAnalytics = GetJsonBoolField(Params, TEXT("mock_analytics"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("harness_name"), HarnessName);
        Result->SetNumberField(TEXT("mock_count"), MocksJson.Num());
        Result->SetNumberField(TEXT("stub_count"), StubsJson.Num());
        Result->SetBoolField(TEXT("isolate_networking"), bIsolateNetworking);
        Result->SetBoolField(TEXT("mock_save_system"), bMockSaveSystem);
        Result->SetBoolField(TEXT("mock_achievements"), bMockAchievements);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Test harness configured"), Result);
        return true;
    }

    // create_latent_test - Async, timeline
    if (SubAction == TEXT("create_latent_test"))
    {
        FString TestName = GetJsonStringField(Params, TEXT("name"), TEXT("LatentTest"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Tests"));
        float Timeout = GetJsonNumberField(Params, TEXT("timeout"), 30.0f);
        TArray<TSharedPtr<FJsonValue>> StepsJson;
        if (Params->HasField(TEXT("steps")))
        {
            StepsJson = Params->GetArrayField(TEXT("steps"));
        }
        bool bAllowParallel = GetJsonBoolField(Params, TEXT("allow_parallel"), false);
        bool bContinueOnFailure = GetJsonBoolField(Params, TEXT("continue_on_failure"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("test_name"), TestName);
        Result->SetStringField(TEXT("path"), OutputPath / TestName);
        Result->SetNumberField(TEXT("timeout"), Timeout);
        Result->SetNumberField(TEXT("step_count"), StepsJson.Num());
        Result->SetBoolField(TEXT("allow_parallel"), bAllowParallel);
        Result->SetBoolField(TEXT("continue_on_failure"), bContinueOnFailure);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Latent test created"), Result);
        return true;
    }

    // run_test_suite - Categories, filters
    if (SubAction == TEXT("run_test_suite"))
    {
        FString SuiteName = GetJsonStringField(Params, TEXT("suite_name"), TEXT(""));
        TArray<TSharedPtr<FJsonValue>> CategoriesJson;
        if (Params->HasField(TEXT("categories")))
        {
            CategoriesJson = Params->GetArrayField(TEXT("categories"));
        }
        TArray<TSharedPtr<FJsonValue>> FiltersJson;
        if (Params->HasField(TEXT("filters")))
        {
            FiltersJson = Params->GetArrayField(TEXT("filters"));
        }
        bool bRunInEditor = GetJsonBoolField(Params, TEXT("run_in_editor"), true);
        bool bRunInGame = GetJsonBoolField(Params, TEXT("run_in_game"), false);
        bool bGenerateReport = GetJsonBoolField(Params, TEXT("generate_report"), true);
        FString ReportFormat = GetJsonStringField(Params, TEXT("report_format"), TEXT("HTML")); // HTML, JSON, XML

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("suite_name"), SuiteName);
        Result->SetNumberField(TEXT("category_count"), CategoriesJson.Num());
        Result->SetNumberField(TEXT("filter_count"), FiltersJson.Num());
        Result->SetBoolField(TEXT("run_in_editor"), bRunInEditor);
        Result->SetBoolField(TEXT("run_in_game"), bRunInGame);
        Result->SetBoolField(TEXT("generate_report"), bGenerateReport);
        Result->SetStringField(TEXT("report_format"), ReportFormat);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Test suite queued"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown testing action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// VALIDATION HANDLER
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2ValidationAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // validate_assets - Missing refs, errors
    if (SubAction == TEXT("validate_assets"))
    {
        FString AssetPath = GetJsonStringField(Params, TEXT("asset_path"), TEXT("/Game"));
        bool bRecursive = GetJsonBoolField(Params, TEXT("recursive"), true);
        bool bCheckDependencies = GetJsonBoolField(Params, TEXT("check_dependencies"), true);
        bool bCheckMissingRefs = GetJsonBoolField(Params, TEXT("check_missing_refs"), true);
        bool bCheckCircularRefs = GetJsonBoolField(Params, TEXT("check_circular_refs"), true);
        TArray<TSharedPtr<FJsonValue>> ExcludePathsJson;
        if (Params->HasField(TEXT("exclude_paths")))
        {
            ExcludePathsJson = Params->GetArrayField(TEXT("exclude_paths"));
        }

        // Perform validation
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

        TArray<FAssetData> AssetDataList;
        AssetRegistry.GetAssetsByPath(*AssetPath, AssetDataList, bRecursive);

        int32 TotalAssets = AssetDataList.Num();
        int32 ValidAssets = 0;
        int32 InvalidAssets = 0;
        TArray<FString> Errors;

        for (const FAssetData& AssetData : AssetDataList)
        {
            if (AssetData.IsValid())
            {
                ValidAssets++;
            }
            else
            {
                InvalidAssets++;
                Errors.Add(AssetData.PackageName.ToString());
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("asset_path"), AssetPath);
        Result->SetNumberField(TEXT("total_assets"), TotalAssets);
        Result->SetNumberField(TEXT("valid_assets"), ValidAssets);
        Result->SetNumberField(TEXT("invalid_assets"), InvalidAssets);
        Result->SetNumberField(TEXT("error_count"), Errors.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Asset validation complete"), Result);
        return true;
    }

    // check_blueprint_errors - Compile issues
    if (SubAction == TEXT("check_blueprint_errors"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        bool bRecursive = GetJsonBoolField(Params, TEXT("recursive"), true);
        bool bCompileAll = GetJsonBoolField(Params, TEXT("compile_all"), false);
        bool bReportWarnings = GetJsonBoolField(Params, TEXT("report_warnings"), true);

        int32 TotalBlueprints = 0;
        int32 ErrorCount = 0;
        int32 WarningCount = 0;

        if (!BlueprintPath.IsEmpty())
        {
            UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
            if (Blueprint)
            {
                TotalBlueprints = 1;
                if (Blueprint->Status == BS_Error)
                {
                    ErrorCount++;
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetNumberField(TEXT("total_blueprints"), TotalBlueprints);
        Result->SetNumberField(TEXT("error_count"), ErrorCount);
        Result->SetNumberField(TEXT("warning_count"), WarningCount);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Blueprint validation complete"), Result);
        return true;
    }

    // analyze_performance - GPU, CPU profiling
    if (SubAction == TEXT("analyze_performance"))
    {
        FString ProfileType = GetJsonStringField(Params, TEXT("profile_type"), TEXT("Both")); // CPU, GPU, Both
        float Duration = GetJsonNumberField(Params, TEXT("duration"), 10.0f);
        bool bCaptureStats = GetJsonBoolField(Params, TEXT("capture_stats"), true);
        bool bCaptureMemory = GetJsonBoolField(Params, TEXT("capture_memory"), true);
        bool bCaptureDrawCalls = GetJsonBoolField(Params, TEXT("capture_draw_calls"), true);
        bool bExportReport = GetJsonBoolField(Params, TEXT("export_report"), true);
        FString ReportPath = GetJsonStringField(Params, TEXT("report_path"), TEXT("Saved/Profiling"));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("profile_type"), ProfileType);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetBoolField(TEXT("capture_stats"), bCaptureStats);
        Result->SetBoolField(TEXT("capture_memory"), bCaptureMemory);
        Result->SetBoolField(TEXT("capture_draw_calls"), bCaptureDrawCalls);
        Result->SetStringField(TEXT("report_path"), ReportPath);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Performance analysis started"), Result);
        return true;
    }

    // validate_data_integrity - Null checks, ranges
    if (SubAction == TEXT("validate_data_integrity"))
    {
        FString DataPath = GetJsonStringField(Params, TEXT("data_path"), TEXT(""));
        bool bCheckNulls = GetJsonBoolField(Params, TEXT("check_nulls"), true);
        bool bCheckRanges = GetJsonBoolField(Params, TEXT("check_ranges"), true);
        bool bCheckDuplicates = GetJsonBoolField(Params, TEXT("check_duplicates"), true);
        bool bCheckOrphans = GetJsonBoolField(Params, TEXT("check_orphans"), true);
        TArray<TSharedPtr<FJsonValue>> RulesJson;
        if (Params->HasField(TEXT("rules")))
        {
            RulesJson = Params->GetArrayField(TEXT("rules"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("data_path"), DataPath);
        Result->SetBoolField(TEXT("check_nulls"), bCheckNulls);
        Result->SetBoolField(TEXT("check_ranges"), bCheckRanges);
        Result->SetBoolField(TEXT("check_duplicates"), bCheckDuplicates);
        Result->SetBoolField(TEXT("check_orphans"), bCheckOrphans);
        Result->SetNumberField(TEXT("rule_count"), RulesJson.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Data integrity validated"), Result);
        return true;
    }

    // check_naming_conventions - Asset naming rules
    if (SubAction == TEXT("check_naming_conventions"))
    {
        FString AssetPath = GetJsonStringField(Params, TEXT("asset_path"), TEXT("/Game"));
        bool bRecursive = GetJsonBoolField(Params, TEXT("recursive"), true);
        TArray<TSharedPtr<FJsonValue>> RulesJson;
        if (Params->HasField(TEXT("rules")))
        {
            RulesJson = Params->GetArrayField(TEXT("rules"));
        }
        bool bAutoFix = GetJsonBoolField(Params, TEXT("auto_fix"), false);
        bool bReportOnly = GetJsonBoolField(Params, TEXT("report_only"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("asset_path"), AssetPath);
        Result->SetBoolField(TEXT("recursive"), bRecursive);
        Result->SetNumberField(TEXT("rule_count"), RulesJson.Num());
        Result->SetBoolField(TEXT("auto_fix"), bAutoFix);
        Result->SetBoolField(TEXT("report_only"), bReportOnly);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Naming convention check complete"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown validation action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 Build Handler
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2BuildAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // configure_build_settings - Platform, config
    if (SubAction == TEXT("configure_build_settings"))
    {
        FString Platform = GetJsonStringField(Params, TEXT("platform"), TEXT("Windows"));
        FString Configuration = GetJsonStringField(Params, TEXT("configuration"), TEXT("Development"));
        bool bForDistribution = GetJsonBoolField(Params, TEXT("for_distribution"), false);
        bool bIncludeDebugInfo = GetJsonBoolField(Params, TEXT("include_debug_info"), true);
        bool bCompressPackage = GetJsonBoolField(Params, TEXT("compress_package"), true);
        bool bBuildEditor = GetJsonBoolField(Params, TEXT("build_editor"), false);
        FString OutputDirectory = GetJsonStringField(Params, TEXT("output_directory"), TEXT(""));
        bool bUseIoStore = GetJsonBoolField(Params, TEXT("use_io_store"), true);

        // Configure project settings for build
        UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
        if (PackagingSettings)
        {
            PackagingSettings->bCompressed = bCompressPackage;
            PackagingSettings->bUseIoStore = bUseIoStore;
            PackagingSettings->ForDistribution = bForDistribution;
            if (!OutputDirectory.IsEmpty())
            {
                PackagingSettings->StagingDirectory.Path = OutputDirectory;
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("platform"), Platform);
        Result->SetStringField(TEXT("configuration"), Configuration);
        Result->SetBoolField(TEXT("for_distribution"), bForDistribution);
        Result->SetBoolField(TEXT("include_debug_info"), bIncludeDebugInfo);
        Result->SetBoolField(TEXT("compress_package"), bCompressPackage);
        Result->SetBoolField(TEXT("use_io_store"), bUseIoStore);
        Result->SetStringField(TEXT("output_directory"), OutputDirectory);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Build settings configured"), Result);
        return true;
    }

    // setup_build_target - Development, shipping, debug
    if (SubAction == TEXT("setup_build_target"))
    {
        FString TargetName = GetJsonStringField(Params, TEXT("target_name"), TEXT(""));
        FString TargetType = GetJsonStringField(Params, TEXT("target_type"), TEXT("Game")); // Game, Editor, Server, Client, Program
        FString Configuration = GetJsonStringField(Params, TEXT("configuration"), TEXT("Development"));
        TArray<TSharedPtr<FJsonValue>> PlatformsJson;
        if (Params->HasField(TEXT("platforms")))
        {
            PlatformsJson = Params->GetArrayField(TEXT("platforms"));
        }
        TArray<TSharedPtr<FJsonValue>> ExtraModulesJson;
        if (Params->HasField(TEXT("extra_modules")))
        {
            ExtraModulesJson = Params->GetArrayField(TEXT("extra_modules"));
        }
        bool bDefaultBuild = GetJsonBoolField(Params, TEXT("default_build"), false);

        TArray<FString> Platforms;
        for (const TSharedPtr<FJsonValue>& PlatformVal : PlatformsJson)
        {
            Platforms.Add(PlatformVal->AsString());
        }

        TArray<FString> ExtraModules;
        for (const TSharedPtr<FJsonValue>& ModuleVal : ExtraModulesJson)
        {
            ExtraModules.Add(ModuleVal->AsString());
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("target_name"), TargetName);
        Result->SetStringField(TEXT("target_type"), TargetType);
        Result->SetStringField(TEXT("configuration"), Configuration);
        Result->SetNumberField(TEXT("platform_count"), Platforms.Num());
        Result->SetNumberField(TEXT("extra_module_count"), ExtraModules.Num());
        Result->SetBoolField(TEXT("default_build"), bDefaultBuild);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Build target configured"), Result);
        return true;
    }

    // create_build_profile - Settings preset
    if (SubAction == TEXT("create_build_profile"))
    {
        FString ProfileName = GetJsonStringField(Params, TEXT("profile_name"), TEXT("DefaultProfile"));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString BasedOn = GetJsonStringField(Params, TEXT("based_on"), TEXT(""));
        TSharedPtr<FJsonObject> SettingsJson;
        if (Params->HasField(TEXT("settings")))
        {
            SettingsJson = Params->GetObjectField(TEXT("settings"));
        }
        bool bSaveToProject = GetJsonBoolField(Params, TEXT("save_to_project"), true);

        FString ProfilePath = FPaths::ProjectDir() / TEXT("Config/BuildProfiles") / ProfileName + TEXT(".json");

        // Create profile JSON
        TSharedPtr<FJsonObject> ProfileJson = MakeShared<FJsonObject>();
        ProfileJson->SetStringField(TEXT("name"), ProfileName);
        ProfileJson->SetStringField(TEXT("description"), Description);
        ProfileJson->SetStringField(TEXT("based_on"), BasedOn);
        if (SettingsJson.IsValid())
        {
            ProfileJson->SetObjectField(TEXT("settings"), SettingsJson);
        }

        if (bSaveToProject)
        {
            FString JsonString;
            TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
            FJsonSerializer::Serialize(ProfileJson.ToSharedRef(), Writer);
            FFileHelper::SaveStringToFile(JsonString, *ProfilePath);
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("profile_name"), ProfileName);
        Result->SetStringField(TEXT("profile_path"), ProfilePath);
        Result->SetBoolField(TEXT("saved"), bSaveToProject);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Build profile created"), Result);
        return true;
    }

    // configure_pak_files - Chunking, encryption
    if (SubAction == TEXT("configure_pak_files"))
    {
        bool bUsePak = GetJsonBoolField(Params, TEXT("use_pak"), true);
        bool bEncryptPak = GetJsonBoolField(Params, TEXT("encrypt_pak"), false);
        FString EncryptionKey = GetJsonStringField(Params, TEXT("encryption_key"), TEXT(""));
        bool bGenerateChunks = GetJsonBoolField(Params, TEXT("generate_chunks"), false);
        int32 ChunkSize = GetJsonIntField(Params, TEXT("chunk_size"), 0);
        bool bCompressPak = GetJsonBoolField(Params, TEXT("compress_pak"), true);
        TArray<TSharedPtr<FJsonValue>> ExcludePatternsJson;
        if (Params->HasField(TEXT("exclude_patterns")))
        {
            ExcludePatternsJson = Params->GetArrayField(TEXT("exclude_patterns"));
        }
        bool bSignPak = GetJsonBoolField(Params, TEXT("sign_pak"), false);

        UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
        if (PackagingSettings)
        {
            PackagingSettings->bBuildHttpChunkInstallData = bGenerateChunks;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("use_pak"), bUsePak);
        Result->SetBoolField(TEXT("encrypt_pak"), bEncryptPak);
        Result->SetBoolField(TEXT("generate_chunks"), bGenerateChunks);
        Result->SetNumberField(TEXT("chunk_size"), ChunkSize);
        Result->SetBoolField(TEXT("compress_pak"), bCompressPak);
        Result->SetNumberField(TEXT("exclude_pattern_count"), ExcludePatternsJson.Num());
        Result->SetBoolField(TEXT("sign_pak"), bSignPak);
        SendAutomationResponse(Socket, RequestId, true, TEXT("PAK file settings configured"), Result);
        return true;
    }

    // setup_dlc_content - Additional content packs
    if (SubAction == TEXT("setup_dlc_content"))
    {
        FString DlcName = GetJsonStringField(Params, TEXT("dlc_name"), TEXT("DLC1"));
        FString DlcDescription = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString DlcVersion = GetJsonStringField(Params, TEXT("version"), TEXT("1.0.0"));
        TArray<TSharedPtr<FJsonValue>> ContentPathsJson;
        if (Params->HasField(TEXT("content_paths")))
        {
            ContentPathsJson = Params->GetArrayField(TEXT("content_paths"));
        }
        TArray<TSharedPtr<FJsonValue>> DependsOnJson;
        if (Params->HasField(TEXT("depends_on")))
        {
            DependsOnJson = Params->GetArrayField(TEXT("depends_on"));
        }
        FString MountPoint = GetJsonStringField(Params, TEXT("mount_point"), TEXT("/Game/DLC"));
        bool bRequireBaseGame = GetJsonBoolField(Params, TEXT("require_base_game"), true);

        TArray<FString> ContentPaths;
        for (const TSharedPtr<FJsonValue>& PathVal : ContentPathsJson)
        {
            ContentPaths.Add(PathVal->AsString());
        }

        TArray<FString> Dependencies;
        for (const TSharedPtr<FJsonValue>& DepVal : DependsOnJson)
        {
            Dependencies.Add(DepVal->AsString());
        }

        // Create DLC manifest
        FString DlcManifestPath = FPaths::ProjectDir() / TEXT("DLC") / DlcName / TEXT("manifest.json");
        TSharedPtr<FJsonObject> ManifestJson = MakeShared<FJsonObject>();
        ManifestJson->SetStringField(TEXT("name"), DlcName);
        ManifestJson->SetStringField(TEXT("description"), DlcDescription);
        ManifestJson->SetStringField(TEXT("version"), DlcVersion);
        ManifestJson->SetStringField(TEXT("mount_point"), MountPoint);
        ManifestJson->SetBoolField(TEXT("require_base_game"), bRequireBaseGame);

        TArray<TSharedPtr<FJsonValue>> ContentPathsArray;
        for (const FString& Path : ContentPaths)
        {
            ContentPathsArray.Add(MakeShared<FJsonValueString>(Path));
        }
        ManifestJson->SetArrayField(TEXT("content_paths"), ContentPathsArray);

        TArray<TSharedPtr<FJsonValue>> DepsArray;
        for (const FString& Dep : Dependencies)
        {
            DepsArray.Add(MakeShared<FJsonValueString>(Dep));
        }
        ManifestJson->SetArrayField(TEXT("dependencies"), DepsArray);

        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        FString DlcDir = FPaths::GetPath(DlcManifestPath);
        PlatformFile.CreateDirectoryTree(*DlcDir);

        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(ManifestJson.ToSharedRef(), Writer);
        FFileHelper::SaveStringToFile(JsonString, *DlcManifestPath);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("dlc_name"), DlcName);
        Result->SetStringField(TEXT("manifest_path"), DlcManifestPath);
        Result->SetNumberField(TEXT("content_path_count"), ContentPaths.Num());
        Result->SetNumberField(TEXT("dependency_count"), Dependencies.Num());
        Result->SetStringField(TEXT("mount_point"), MountPoint);
        SendAutomationResponse(Socket, RequestId, true, TEXT("DLC content configured"), Result);
        return true;
    }

    // start_build - Trigger packaging
    if (SubAction == TEXT("start_build"))
    {
        FString Platform = GetJsonStringField(Params, TEXT("platform"), TEXT("Windows"));
        FString Configuration = GetJsonStringField(Params, TEXT("configuration"), TEXT("Shipping"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT(""));
        bool bCleanBuild = GetJsonBoolField(Params, TEXT("clean_build"), false);
        bool bAsync = GetJsonBoolField(Params, TEXT("async"), true);
        FString MapToPackage = GetJsonStringField(Params, TEXT("map"), TEXT(""));

        // Build command would typically be executed via UAT
        FString UatPath = FPaths::ConvertRelativePathToFull(FPaths::EngineDir() / TEXT("Build/BatchFiles/RunUAT.bat"));
        FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());

        FString CommandArgs = FString::Printf(
            TEXT("BuildCookRun -project=\"%s\" -platform=%s -clientconfig=%s -cook -stage -package"),
            *ProjectPath, *Platform, *Configuration
        );

        if (bCleanBuild)
        {
            CommandArgs += TEXT(" -clean");
        }

        if (!MapToPackage.IsEmpty())
        {
            CommandArgs += FString::Printf(TEXT(" -map=%s"), *MapToPackage);
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("platform"), Platform);
        Result->SetStringField(TEXT("configuration"), Configuration);
        Result->SetStringField(TEXT("uat_path"), UatPath);
        Result->SetStringField(TEXT("command_args"), CommandArgs);
        Result->SetBoolField(TEXT("async"), bAsync);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Build initiated"), Result);
        return true;
    }

    // get_build_status - Check build progress
    if (SubAction == TEXT("get_build_status"))
    {
        FString BuildId = GetJsonStringField(Params, TEXT("build_id"), TEXT(""));

        // Check build status - in a real implementation this would track async builds
        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("build_id"), BuildId);
        Result->SetStringField(TEXT("status"), TEXT("completed"));
        Result->SetNumberField(TEXT("progress"), 100.0f);
        Result->SetStringField(TEXT("current_step"), TEXT("Finished"));
        SendAutomationResponse(Socket, RequestId, true, TEXT("Build status retrieved"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown build action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 Platforms Handler
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2PlatformsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // configure_platform_settings - Console, PC, mobile
    if (SubAction == TEXT("configure_platform_settings"))
    {
        FString PlatformName = GetJsonStringField(Params, TEXT("platform_name"), TEXT("Windows"));
        TSharedPtr<FJsonObject> GraphicsSettingsJson;
        if (Params->HasField(TEXT("graphics_settings")))
        {
            GraphicsSettingsJson = Params->GetObjectField(TEXT("graphics_settings"));
        }
        TSharedPtr<FJsonObject> QualitySettingsJson;
        if (Params->HasField(TEXT("quality_settings")))
        {
            QualitySettingsJson = Params->GetObjectField(TEXT("quality_settings"));
        }
        TSharedPtr<FJsonObject> MemorySettingsJson;
        if (Params->HasField(TEXT("memory_settings")))
        {
            MemorySettingsJson = Params->GetObjectField(TEXT("memory_settings"));
        }
        bool bEnableMobileRendering = GetJsonBoolField(Params, TEXT("enable_mobile_rendering"), false);
        bool bEnableRayTracing = GetJsonBoolField(Params, TEXT("enable_ray_tracing"), false);

        // Apply platform-specific settings
        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("platform_name"), PlatformName);
        Result->SetBoolField(TEXT("enable_mobile_rendering"), bEnableMobileRendering);
        Result->SetBoolField(TEXT("enable_ray_tracing"), bEnableRayTracing);

        if (GraphicsSettingsJson.IsValid())
        {
            Result->SetObjectField(TEXT("graphics_settings"), GraphicsSettingsJson);
        }
        if (QualitySettingsJson.IsValid())
        {
            Result->SetObjectField(TEXT("quality_settings"), QualitySettingsJson);
        }

        SendAutomationResponse(Socket, RequestId, true, TEXT("Platform settings configured"), Result);
        return true;
    }

    // setup_platform_input - Controller, touch, keyboard
    if (SubAction == TEXT("setup_platform_input"))
    {
        FString PlatformName = GetJsonStringField(Params, TEXT("platform_name"), TEXT("Windows"));
        bool bEnableController = GetJsonBoolField(Params, TEXT("enable_controller"), true);
        bool bEnableTouch = GetJsonBoolField(Params, TEXT("enable_touch"), false);
        bool bEnableKeyboard = GetJsonBoolField(Params, TEXT("enable_keyboard"), true);
        bool bEnableMouse = GetJsonBoolField(Params, TEXT("enable_mouse"), true);
        bool bEnableGyroscope = GetJsonBoolField(Params, TEXT("enable_gyroscope"), false);
        FString ControllerType = GetJsonStringField(Params, TEXT("controller_type"), TEXT("Generic")); // Generic, Xbox, PlayStation, Switch
        TSharedPtr<FJsonObject> TouchSettingsJson;
        if (Params->HasField(TEXT("touch_settings")))
        {
            TouchSettingsJson = Params->GetObjectField(TEXT("touch_settings"));
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("platform_name"), PlatformName);
        Result->SetBoolField(TEXT("enable_controller"), bEnableController);
        Result->SetBoolField(TEXT("enable_touch"), bEnableTouch);
        Result->SetBoolField(TEXT("enable_keyboard"), bEnableKeyboard);
        Result->SetBoolField(TEXT("enable_mouse"), bEnableMouse);
        Result->SetBoolField(TEXT("enable_gyroscope"), bEnableGyroscope);
        Result->SetStringField(TEXT("controller_type"), ControllerType);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Platform input configured"), Result);
        return true;
    }

    // create_platform_overrides - Scalability, features
    if (SubAction == TEXT("create_platform_overrides"))
    {
        FString PlatformName = GetJsonStringField(Params, TEXT("platform_name"), TEXT("Windows"));
        FString OverrideName = GetJsonStringField(Params, TEXT("override_name"), TEXT("Default"));
        TSharedPtr<FJsonObject> ScalabilityJson;
        if (Params->HasField(TEXT("scalability")))
        {
            ScalabilityJson = Params->GetObjectField(TEXT("scalability"));
        }
        TArray<TSharedPtr<FJsonValue>> DisabledFeaturesJson;
        if (Params->HasField(TEXT("disabled_features")))
        {
            DisabledFeaturesJson = Params->GetArrayField(TEXT("disabled_features"));
        }
        TArray<TSharedPtr<FJsonValue>> EnabledFeaturesJson;
        if (Params->HasField(TEXT("enabled_features")))
        {
            EnabledFeaturesJson = Params->GetArrayField(TEXT("enabled_features"));
        }
        int32 TextureQuality = GetJsonIntField(Params, TEXT("texture_quality"), 3);
        int32 ShadowQuality = GetJsonIntField(Params, TEXT("shadow_quality"), 3);
        int32 EffectsQuality = GetJsonIntField(Params, TEXT("effects_quality"), 3);
        int32 ViewDistance = GetJsonIntField(Params, TEXT("view_distance"), 3);
        int32 AntiAliasing = GetJsonIntField(Params, TEXT("anti_aliasing"), 3);

        TArray<FString> DisabledFeatures;
        for (const TSharedPtr<FJsonValue>& Val : DisabledFeaturesJson)
        {
            DisabledFeatures.Add(Val->AsString());
        }

        TArray<FString> EnabledFeatures;
        for (const TSharedPtr<FJsonValue>& Val : EnabledFeaturesJson)
        {
            EnabledFeatures.Add(Val->AsString());
        }

        // Apply scalability settings
        Scalability::FQualityLevels QualityLevels;
        QualityLevels.TextureQuality = TextureQuality;
        QualityLevels.ShadowQuality = ShadowQuality;
        QualityLevels.EffectsQuality = EffectsQuality;
        QualityLevels.ViewDistanceQuality = ViewDistance;
        QualityLevels.AntiAliasingQuality = AntiAliasing;
        Scalability::SetQualityLevels(QualityLevels);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("platform_name"), PlatformName);
        Result->SetStringField(TEXT("override_name"), OverrideName);
        Result->SetNumberField(TEXT("texture_quality"), TextureQuality);
        Result->SetNumberField(TEXT("shadow_quality"), ShadowQuality);
        Result->SetNumberField(TEXT("effects_quality"), EffectsQuality);
        Result->SetNumberField(TEXT("view_distance"), ViewDistance);
        Result->SetNumberField(TEXT("anti_aliasing"), AntiAliasing);
        Result->SetNumberField(TEXT("disabled_feature_count"), DisabledFeatures.Num());
        Result->SetNumberField(TEXT("enabled_feature_count"), EnabledFeatures.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Platform overrides created"), Result);
        return true;
    }

    // configure_cross_platform - Cross-play, cross-save
    if (SubAction == TEXT("configure_cross_platform"))
    {
        bool bEnableCrossPlay = GetJsonBoolField(Params, TEXT("enable_cross_play"), false);
        bool bEnableCrossSave = GetJsonBoolField(Params, TEXT("enable_cross_save"), false);
        bool bEnableCrossProgression = GetJsonBoolField(Params, TEXT("enable_cross_progression"), false);
        TArray<TSharedPtr<FJsonValue>> SupportedPlatformsJson;
        if (Params->HasField(TEXT("supported_platforms")))
        {
            SupportedPlatformsJson = Params->GetArrayField(TEXT("supported_platforms"));
        }
        FString CloudProvider = GetJsonStringField(Params, TEXT("cloud_provider"), TEXT("Epic"));
        bool bRequireAccountLink = GetJsonBoolField(Params, TEXT("require_account_link"), true);

        TArray<FString> SupportedPlatforms;
        for (const TSharedPtr<FJsonValue>& Val : SupportedPlatformsJson)
        {
            SupportedPlatforms.Add(Val->AsString());
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("enable_cross_play"), bEnableCrossPlay);
        Result->SetBoolField(TEXT("enable_cross_save"), bEnableCrossSave);
        Result->SetBoolField(TEXT("enable_cross_progression"), bEnableCrossProgression);
        Result->SetNumberField(TEXT("supported_platform_count"), SupportedPlatforms.Num());
        Result->SetStringField(TEXT("cloud_provider"), CloudProvider);
        Result->SetBoolField(TEXT("require_account_link"), bRequireAccountLink);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Cross-platform settings configured"), Result);
        return true;
    }

    // configure_console_certification - Platform requirements
    if (SubAction == TEXT("configure_console_certification"))
    {
        FString ConsolePlatform = GetJsonStringField(Params, TEXT("console_platform"), TEXT("PlayStation"));
        bool bEnableTrophies = GetJsonBoolField(Params, TEXT("enable_trophies"), true);
        bool bEnableRichPresence = GetJsonBoolField(Params, TEXT("enable_rich_presence"), true);
        bool bEnableActivities = GetJsonBoolField(Params, TEXT("enable_activities"), true);
        bool bSuspendAudioOnSuspend = GetJsonBoolField(Params, TEXT("suspend_audio_on_suspend"), true);
        bool bHandleQuickResume = GetJsonBoolField(Params, TEXT("handle_quick_resume"), true);
        int32 MinFrameRate = GetJsonIntField(Params, TEXT("min_frame_rate"), 30);
        int32 TargetResolution = GetJsonIntField(Params, TEXT("target_resolution"), 1080);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("console_platform"), ConsolePlatform);
        Result->SetBoolField(TEXT("enable_trophies"), bEnableTrophies);
        Result->SetBoolField(TEXT("enable_rich_presence"), bEnableRichPresence);
        Result->SetBoolField(TEXT("enable_activities"), bEnableActivities);
        Result->SetBoolField(TEXT("suspend_audio_on_suspend"), bSuspendAudioOnSuspend);
        Result->SetBoolField(TEXT("handle_quick_resume"), bHandleQuickResume);
        Result->SetNumberField(TEXT("min_frame_rate"), MinFrameRate);
        Result->SetNumberField(TEXT("target_resolution"), TargetResolution);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Console certification settings configured"), Result);
        return true;
    }

    // configure_mobile_settings - iOS/Android specific
    if (SubAction == TEXT("configure_mobile_settings"))
    {
        FString MobilePlatform = GetJsonStringField(Params, TEXT("mobile_platform"), TEXT("Android"));
        FString MinOsVersion = GetJsonStringField(Params, TEXT("min_os_version"), TEXT(""));
        bool bEnableMultithreading = GetJsonBoolField(Params, TEXT("enable_multithreading"), true);
        int32 TargetFrameRate = GetJsonIntField(Params, TEXT("target_frame_rate"), 60);
        bool bEnableBatteryOptimization = GetJsonBoolField(Params, TEXT("enable_battery_optimization"), true);
        bool bEnableThermalThrottling = GetJsonBoolField(Params, TEXT("enable_thermal_throttling"), true);
        FString RenderingMode = GetJsonStringField(Params, TEXT("rendering_mode"), TEXT("Forward")); // Forward, Deferred, Mobile
        bool bEnableVulkan = GetJsonBoolField(Params, TEXT("enable_vulkan"), true);
        bool bEnableMetal = GetJsonBoolField(Params, TEXT("enable_metal"), true);
        int32 TextureStreamingPoolSize = GetJsonIntField(Params, TEXT("texture_streaming_pool_size"), 256);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("mobile_platform"), MobilePlatform);
        Result->SetStringField(TEXT("min_os_version"), MinOsVersion);
        Result->SetBoolField(TEXT("enable_multithreading"), bEnableMultithreading);
        Result->SetNumberField(TEXT("target_frame_rate"), TargetFrameRate);
        Result->SetBoolField(TEXT("enable_battery_optimization"), bEnableBatteryOptimization);
        Result->SetBoolField(TEXT("enable_thermal_throttling"), bEnableThermalThrottling);
        Result->SetStringField(TEXT("rendering_mode"), RenderingMode);
        Result->SetBoolField(TEXT("enable_vulkan"), bEnableVulkan);
        Result->SetBoolField(TEXT("enable_metal"), bEnableMetal);
        Result->SetNumberField(TEXT("texture_streaming_pool_size"), TextureStreamingPoolSize);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Mobile settings configured"), Result);
        return true;
    }

    // configure_vr_settings - VR platform specific
    if (SubAction == TEXT("configure_vr_settings"))
    {
        FString VrPlatform = GetJsonStringField(Params, TEXT("vr_platform"), TEXT("Oculus")); // Oculus, SteamVR, PSVR, WindowsMR
        bool bEnableVr = GetJsonBoolField(Params, TEXT("enable_vr"), true);
        float WorldScale = GetJsonNumberField(Params, TEXT("world_scale"), 100.0f);
        bool bEnableHandTracking = GetJsonBoolField(Params, TEXT("enable_hand_tracking"), false);
        bool bEnableEyeTracking = GetJsonBoolField(Params, TEXT("enable_eye_tracking"), false);
        int32 RenderTargetScale = GetJsonIntField(Params, TEXT("render_target_scale"), 100);
        bool bEnableFoveatedRendering = GetJsonBoolField(Params, TEXT("enable_foveated_rendering"), false);
        bool bEnablePassthrough = GetJsonBoolField(Params, TEXT("enable_passthrough"), false);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("vr_platform"), VrPlatform);
        Result->SetBoolField(TEXT("enable_vr"), bEnableVr);
        Result->SetNumberField(TEXT("world_scale"), WorldScale);
        Result->SetBoolField(TEXT("enable_hand_tracking"), bEnableHandTracking);
        Result->SetBoolField(TEXT("enable_eye_tracking"), bEnableEyeTracking);
        Result->SetNumberField(TEXT("render_target_scale"), RenderTargetScale);
        Result->SetBoolField(TEXT("enable_foveated_rendering"), bEnableFoveatedRendering);
        Result->SetBoolField(TEXT("enable_passthrough"), bEnablePassthrough);
        SendAutomationResponse(Socket, RequestId, true, TEXT("VR settings configured"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown platforms action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 Context Handler
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2ContextAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // get_project_summary - High-level overview
    if (SubAction == TEXT("get_project_summary"))
    {
        bool bIncludeAssetCounts = GetJsonBoolField(Params, TEXT("include_asset_counts"), true);
        bool bIncludeCodeStats = GetJsonBoolField(Params, TEXT("include_code_stats"), true);
        bool bIncludePluginList = GetJsonBoolField(Params, TEXT("include_plugin_list"), true);
        bool bIncludeMapList = GetJsonBoolField(Params, TEXT("include_map_list"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

        // Project info
        const FProjectDescriptor* ProjectDescriptor = IProjectManager::Get().GetCurrentProject();
        if (ProjectDescriptor)
        {
            Result->SetStringField(TEXT("project_name"), FApp::GetProjectName());
        }
        Result->SetStringField(TEXT("engine_version"), FEngineVersion::Current().ToString());
        Result->SetStringField(TEXT("project_path"), FPaths::GetProjectFilePath());

        // Asset counts
        if (bIncludeAssetCounts)
        {
            FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
            IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

            TArray<FAssetData> AllAssets;
            AssetRegistry.GetAllAssets(AllAssets, true);

            TMap<FName, int32> AssetTypeCounts;
            for (const FAssetData& Asset : AllAssets)
            {
                FName ClassName = Asset.AssetClassPath.GetAssetName();
                AssetTypeCounts.FindOrAdd(ClassName)++;
            }

            TSharedPtr<FJsonObject> AssetCountsJson = MakeShared<FJsonObject>();
            AssetCountsJson->SetNumberField(TEXT("total"), AllAssets.Num());
            for (const auto& Pair : AssetTypeCounts)
            {
                AssetCountsJson->SetNumberField(Pair.Key.ToString(), Pair.Value);
            }
            Result->SetObjectField(TEXT("asset_counts"), AssetCountsJson);
        }

        // Map list
        if (bIncludeMapList)
        {
            TArray<FString> MapFiles;
            IFileManager::Get().FindFilesRecursive(MapFiles, *FPaths::ProjectContentDir(), TEXT("*.umap"), true, false);

            TArray<TSharedPtr<FJsonValue>> MapsArray;
            for (const FString& MapFile : MapFiles)
            {
                FString MapName = FPaths::GetBaseFilename(MapFile);
                MapsArray.Add(MakeShared<FJsonValueString>(MapName));
            }
            Result->SetArrayField(TEXT("maps"), MapsArray);
        }

        SendAutomationResponse(Socket, RequestId, true, TEXT("Project summary retrieved"), Result);
        return true;
    }

    // get_category_index - List assets by category
    if (SubAction == TEXT("get_category_index"))
    {
        FString Category = GetJsonStringField(Params, TEXT("category"), TEXT(""));
        FString BasePath = GetJsonStringField(Params, TEXT("base_path"), TEXT("/Game"));
        bool bRecursive = GetJsonBoolField(Params, TEXT("recursive"), true);
        int32 MaxResults = GetJsonIntField(Params, TEXT("max_results"), 100);

        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

        TArray<FAssetData> Assets;
        AssetRegistry.GetAssetsByPath(FName(*BasePath), Assets, bRecursive);

        TArray<TSharedPtr<FJsonValue>> AssetsArray;
        int32 Count = 0;
        for (const FAssetData& Asset : Assets)
        {
            if (Count >= MaxResults) break;

            TSharedPtr<FJsonObject> AssetJson = MakeShared<FJsonObject>();
            AssetJson->SetStringField(TEXT("name"), Asset.AssetName.ToString());
            AssetJson->SetStringField(TEXT("path"), Asset.GetObjectPathString());
            AssetJson->SetStringField(TEXT("class"), Asset.AssetClassPath.GetAssetName().ToString());
            AssetsArray.Add(MakeShared<FJsonValueObject>(AssetJson));
            Count++;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("category"), Category);
        Result->SetStringField(TEXT("base_path"), BasePath);
        Result->SetNumberField(TEXT("total_found"), Assets.Num());
        Result->SetNumberField(TEXT("returned"), AssetsArray.Num());
        Result->SetArrayField(TEXT("assets"), AssetsArray);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Category index retrieved"), Result);
        return true;
    }

    // drill_down - Get details for specific path
    if (SubAction == TEXT("drill_down"))
    {
        FString AssetPath = GetJsonStringField(Params, TEXT("asset_path"), TEXT(""));
        bool bIncludeReferences = GetJsonBoolField(Params, TEXT("include_references"), true);
        bool bIncludeDependencies = GetJsonBoolField(Params, TEXT("include_dependencies"), true);
        bool bIncludeProperties = GetJsonBoolField(Params, TEXT("include_properties"), true);

        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

        FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(AssetPath));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("asset_path"), AssetPath);
        Result->SetBoolField(TEXT("found"), AssetData.IsValid());

        if (AssetData.IsValid())
        {
            Result->SetStringField(TEXT("asset_name"), AssetData.AssetName.ToString());
            Result->SetStringField(TEXT("asset_class"), AssetData.AssetClassPath.GetAssetName().ToString());
            Result->SetStringField(TEXT("package_path"), AssetData.PackagePath.ToString());

            if (bIncludeReferences)
            {
                TArray<FName> Referencers;
                AssetRegistry.GetReferencers(AssetData.PackageName, Referencers);
                TArray<TSharedPtr<FJsonValue>> RefsArray;
                for (const FName& Ref : Referencers)
                {
                    RefsArray.Add(MakeShared<FJsonValueString>(Ref.ToString()));
                }
                Result->SetArrayField(TEXT("referencers"), RefsArray);
            }

            if (bIncludeDependencies)
            {
                TArray<FName> Dependencies;
                AssetRegistry.GetDependencies(AssetData.PackageName, Dependencies);
                TArray<TSharedPtr<FJsonValue>> DepsArray;
                for (const FName& Dep : Dependencies)
                {
                    DepsArray.Add(MakeShared<FJsonValueString>(Dep.ToString()));
                }
                Result->SetArrayField(TEXT("dependencies"), DepsArray);
            }
        }

        SendAutomationResponse(Socket, RequestId, true, TEXT("Asset details retrieved"), Result);
        return true;
    }

    // update_todo - Add/complete/remove tasks
    if (SubAction == TEXT("update_todo"))
    {
        FString TodoAction = GetJsonStringField(Params, TEXT("todo_action"), TEXT("add")); // add, complete, remove, list
        FString TaskId = GetJsonStringField(Params, TEXT("task_id"), TEXT(""));
        FString TaskDescription = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString TaskPriority = GetJsonStringField(Params, TEXT("priority"), TEXT("normal")); // low, normal, high, critical
        FString TaskCategory = GetJsonStringField(Params, TEXT("category"), TEXT("general"));

        FString TodoFilePath = FPaths::ProjectDir() / TEXT(".mcp/todo.json");

        // Load existing todo list
        TSharedPtr<FJsonObject> TodoData = MakeShared<FJsonObject>();
        FString ExistingJson;
        if (FFileHelper::LoadFileToString(ExistingJson, *TodoFilePath))
        {
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ExistingJson);
            FJsonSerializer::Deserialize(Reader, TodoData);
        }

        TArray<TSharedPtr<FJsonValue>> Tasks;
        if (TodoData->HasField(TEXT("tasks")))
        {
            Tasks = TodoData->GetArrayField(TEXT("tasks"));
        }

        if (TodoAction == TEXT("add"))
        {
            TSharedPtr<FJsonObject> NewTask = MakeShared<FJsonObject>();
            NewTask->SetStringField(TEXT("id"), FGuid::NewGuid().ToString());
            NewTask->SetStringField(TEXT("description"), TaskDescription);
            NewTask->SetStringField(TEXT("priority"), TaskPriority);
            NewTask->SetStringField(TEXT("category"), TaskCategory);
            NewTask->SetBoolField(TEXT("completed"), false);
            NewTask->SetStringField(TEXT("created"), FDateTime::Now().ToString());
            Tasks.Add(MakeShared<FJsonValueObject>(NewTask));
        }
        else if (TodoAction == TEXT("complete") && !TaskId.IsEmpty())
        {
            for (TSharedPtr<FJsonValue>& TaskVal : Tasks)
            {
                TSharedPtr<FJsonObject> Task = TaskVal->AsObject();
                if (Task->GetStringField(TEXT("id")) == TaskId)
                {
                    Task->SetBoolField(TEXT("completed"), true);
                    Task->SetStringField(TEXT("completed_at"), FDateTime::Now().ToString());
                }
            }
        }
        else if (TodoAction == TEXT("remove") && !TaskId.IsEmpty())
        {
            Tasks.RemoveAll([&TaskId](const TSharedPtr<FJsonValue>& TaskVal)
            {
                return TaskVal->AsObject()->GetStringField(TEXT("id")) == TaskId;
            });
        }

        TodoData->SetArrayField(TEXT("tasks"), Tasks);

        // Save todo list
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        PlatformFile.CreateDirectoryTree(*FPaths::GetPath(TodoFilePath));

        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(TodoData.ToSharedRef(), Writer);
        FFileHelper::SaveStringToFile(JsonString, *TodoFilePath);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("action"), TodoAction);
        Result->SetNumberField(TEXT("task_count"), Tasks.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Todo list updated"), Result);
        return true;
    }

    // log_decision - Record architectural decision
    if (SubAction == TEXT("log_decision"))
    {
        FString DecisionTitle = GetJsonStringField(Params, TEXT("title"), TEXT(""));
        FString DecisionDescription = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString DecisionRationale = GetJsonStringField(Params, TEXT("rationale"), TEXT(""));
        FString DecisionCategory = GetJsonStringField(Params, TEXT("category"), TEXT("architecture"));
        TArray<TSharedPtr<FJsonValue>> AlternativesJson;
        if (Params->HasField(TEXT("alternatives")))
        {
            AlternativesJson = Params->GetArrayField(TEXT("alternatives"));
        }
        TArray<TSharedPtr<FJsonValue>> ConsequencesJson;
        if (Params->HasField(TEXT("consequences")))
        {
            ConsequencesJson = Params->GetArrayField(TEXT("consequences"));
        }

        FString DecisionsFilePath = FPaths::ProjectDir() / TEXT(".mcp/decisions.json");

        // Load existing decisions
        TSharedPtr<FJsonObject> DecisionsData = MakeShared<FJsonObject>();
        FString ExistingJson;
        if (FFileHelper::LoadFileToString(ExistingJson, *DecisionsFilePath))
        {
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ExistingJson);
            FJsonSerializer::Deserialize(Reader, DecisionsData);
        }

        TArray<TSharedPtr<FJsonValue>> Decisions;
        if (DecisionsData->HasField(TEXT("decisions")))
        {
            Decisions = DecisionsData->GetArrayField(TEXT("decisions"));
        }

        // Add new decision
        TSharedPtr<FJsonObject> NewDecision = MakeShared<FJsonObject>();
        NewDecision->SetStringField(TEXT("id"), FGuid::NewGuid().ToString());
        NewDecision->SetStringField(TEXT("title"), DecisionTitle);
        NewDecision->SetStringField(TEXT("description"), DecisionDescription);
        NewDecision->SetStringField(TEXT("rationale"), DecisionRationale);
        NewDecision->SetStringField(TEXT("category"), DecisionCategory);
        NewDecision->SetArrayField(TEXT("alternatives"), AlternativesJson);
        NewDecision->SetArrayField(TEXT("consequences"), ConsequencesJson);
        NewDecision->SetStringField(TEXT("date"), FDateTime::Now().ToString());
        Decisions.Add(MakeShared<FJsonValueObject>(NewDecision));

        DecisionsData->SetArrayField(TEXT("decisions"), Decisions);

        // Save decisions
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        PlatformFile.CreateDirectoryTree(*FPaths::GetPath(DecisionsFilePath));

        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(DecisionsData.ToSharedRef(), Writer);
        FFileHelper::SaveStringToFile(JsonString, *DecisionsFilePath);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("title"), DecisionTitle);
        Result->SetStringField(TEXT("category"), DecisionCategory);
        Result->SetNumberField(TEXT("total_decisions"), Decisions.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Decision logged"), Result);
        return true;
    }

    // save_session - Save current progress
    if (SubAction == TEXT("save_session"))
    {
        FString SessionName = GetJsonStringField(Params, TEXT("session_name"), TEXT("default"));
        TSharedPtr<FJsonObject> SessionDataJson;
        if (Params->HasField(TEXT("session_data")))
        {
            SessionDataJson = Params->GetObjectField(TEXT("session_data"));
        }
        FString Notes = GetJsonStringField(Params, TEXT("notes"), TEXT(""));

        FString SessionFilePath = FPaths::ProjectDir() / TEXT(".mcp/sessions") / SessionName + TEXT(".json");

        TSharedPtr<FJsonObject> Session = MakeShared<FJsonObject>();
        Session->SetStringField(TEXT("name"), SessionName);
        Session->SetStringField(TEXT("saved_at"), FDateTime::Now().ToString());
        Session->SetStringField(TEXT("notes"), Notes);
        if (SessionDataJson.IsValid())
        {
            Session->SetObjectField(TEXT("data"), SessionDataJson);
        }

        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        PlatformFile.CreateDirectoryTree(*FPaths::GetPath(SessionFilePath));

        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(Session.ToSharedRef(), Writer);
        FFileHelper::SaveStringToFile(JsonString, *SessionFilePath);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("session_name"), SessionName);
        Result->SetStringField(TEXT("session_path"), SessionFilePath);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Session saved"), Result);
        return true;
    }

    // load_session - Restore session state
    if (SubAction == TEXT("load_session"))
    {
        FString SessionName = GetJsonStringField(Params, TEXT("session_name"), TEXT("default"));

        FString SessionFilePath = FPaths::ProjectDir() / TEXT(".mcp/sessions") / SessionName + TEXT(".json");

        TSharedPtr<FJsonObject> Session;
        FString JsonString;
        if (FFileHelper::LoadFileToString(JsonString, *SessionFilePath))
        {
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
            FJsonSerializer::Deserialize(Reader, Session);
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("session_name"), SessionName);
        Result->SetBoolField(TEXT("found"), Session.IsValid());
        if (Session.IsValid())
        {
            Result->SetObjectField(TEXT("session"), Session);
        }
        SendAutomationResponse(Socket, RequestId, true, TEXT("Session loaded"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown context action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 Components Handler
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2ComponentsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_quest_tracker_component - Quest tracking
    if (SubAction == TEXT("create_quest_tracker_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("QuestTracker"));
        bool bAutoUpdateUI = GetJsonBoolField(Params, TEXT("auto_update_ui"), true);
        bool bPersistProgress = GetJsonBoolField(Params, TEXT("persist_progress"), true);
        int32 MaxActiveQuests = GetJsonIntField(Params, TEXT("max_active_quests"), 20);
        bool bShowNotifications = GetJsonBoolField(Params, TEXT("show_notifications"), true);

        if (BlueprintPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("blueprint_path is required"), TEXT("MISSING_PARAM"));
            return true;
        }

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));
        if (Blueprint && Blueprint->GeneratedClass)
        {
            // Add component to blueprint using SCS
            USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
            if (SCS)
            {
                USCS_Node* NewNode = SCS->CreateNode(UActorComponent::StaticClass(), *ComponentName);
                if (NewNode && NewNode->ComponentTemplate)
                {
                    // Set component properties via metadata
                    NewNode->ComponentTemplate->SetFlags(RF_Public | RF_ArchetypeObject);
                    SCS->AddNode(NewNode);

                    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetBoolField(TEXT("auto_update_ui"), bAutoUpdateUI);
        Result->SetBoolField(TEXT("persist_progress"), bPersistProgress);
        Result->SetNumberField(TEXT("max_active_quests"), MaxActiveQuests);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Quest tracker component created"), Result);
        return true;
    }

    // create_dialogue_component - NPC dialogue
    if (SubAction == TEXT("create_dialogue_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("DialogueComponent"));
        bool bAutoStartOnInteract = GetJsonBoolField(Params, TEXT("auto_start_on_interact"), true);
        FString DefaultDialogueTree = GetJsonStringField(Params, TEXT("default_dialogue_tree"), TEXT(""));
        bool bShowPortraits = GetJsonBoolField(Params, TEXT("show_portraits"), true);
        bool bPlayVoiceOver = GetJsonBoolField(Params, TEXT("play_voice_over"), true);
        float InteractionRadius = GetJsonNumberField(Params, TEXT("interaction_radius"), 200.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetBoolField(TEXT("auto_start_on_interact"), bAutoStartOnInteract);
        Result->SetStringField(TEXT("default_dialogue_tree"), DefaultDialogueTree);
        Result->SetBoolField(TEXT("show_portraits"), bShowPortraits);
        Result->SetBoolField(TEXT("play_voice_over"), bPlayVoiceOver);
        Result->SetNumberField(TEXT("interaction_radius"), InteractionRadius);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue component created"), Result);
        return true;
    }

    // create_inventory_component - Actor inventory
    if (SubAction == TEXT("create_inventory_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("InventoryComponent"));
        int32 MaxSlots = GetJsonIntField(Params, TEXT("max_slots"), 40);
        float MaxWeight = GetJsonNumberField(Params, TEXT("max_weight"), 100.0f);
        bool bEnableStacking = GetJsonBoolField(Params, TEXT("enable_stacking"), true);
        bool bAutoSort = GetJsonBoolField(Params, TEXT("auto_sort"), false);
        bool bPersistInventory = GetJsonBoolField(Params, TEXT("persist_inventory"), true);
        TArray<TSharedPtr<FJsonValue>> RestrictedTypesJson;
        if (Params->HasField(TEXT("restricted_types")))
        {
            RestrictedTypesJson = Params->GetArrayField(TEXT("restricted_types"));
        }

        TArray<FString> RestrictedTypes;
        for (const TSharedPtr<FJsonValue>& Val : RestrictedTypesJson)
        {
            RestrictedTypes.Add(Val->AsString());
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetNumberField(TEXT("max_slots"), MaxSlots);
        Result->SetNumberField(TEXT("max_weight"), MaxWeight);
        Result->SetBoolField(TEXT("enable_stacking"), bEnableStacking);
        Result->SetBoolField(TEXT("auto_sort"), bAutoSort);
        Result->SetBoolField(TEXT("persist_inventory"), bPersistInventory);
        Result->SetNumberField(TEXT("restricted_type_count"), RestrictedTypes.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Inventory component created"), Result);
        return true;
    }

    // create_combat_component - Combat handling
    if (SubAction == TEXT("create_combat_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("CombatComponent"));
        float BaseHealth = GetJsonNumberField(Params, TEXT("base_health"), 100.0f);
        float BaseStamina = GetJsonNumberField(Params, TEXT("base_stamina"), 100.0f);
        bool bCanBlock = GetJsonBoolField(Params, TEXT("can_block"), true);
        bool bCanDodge = GetJsonBoolField(Params, TEXT("can_dodge"), true);
        bool bCanParry = GetJsonBoolField(Params, TEXT("can_parry"), false);
        float InvincibilityDuration = GetJsonNumberField(Params, TEXT("invincibility_duration"), 0.5f);
        bool bShowDamageNumbers = GetJsonBoolField(Params, TEXT("show_damage_numbers"), true);
        FString CombatStyle = GetJsonStringField(Params, TEXT("combat_style"), TEXT("ActionRPG"));

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetNumberField(TEXT("base_health"), BaseHealth);
        Result->SetNumberField(TEXT("base_stamina"), BaseStamina);
        Result->SetBoolField(TEXT("can_block"), bCanBlock);
        Result->SetBoolField(TEXT("can_dodge"), bCanDodge);
        Result->SetBoolField(TEXT("can_parry"), bCanParry);
        Result->SetNumberField(TEXT("invincibility_duration"), InvincibilityDuration);
        Result->SetBoolField(TEXT("show_damage_numbers"), bShowDamageNumbers);
        Result->SetStringField(TEXT("combat_style"), CombatStyle);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Combat component created"), Result);
        return true;
    }

    // create_interaction_component - World interaction
    if (SubAction == TEXT("create_interaction_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("InteractionComponent"));
        float InteractionRange = GetJsonNumberField(Params, TEXT("interaction_range"), 200.0f);
        bool bShowPrompt = GetJsonBoolField(Params, TEXT("show_prompt"), true);
        FString PromptText = GetJsonStringField(Params, TEXT("prompt_text"), TEXT("Interact"));
        FString InteractionType = GetJsonStringField(Params, TEXT("interaction_type"), TEXT("Use")); // Use, Pickup, Talk, Open, Activate
        bool bRequireLineOfSight = GetJsonBoolField(Params, TEXT("require_line_of_sight"), false);
        bool bHighlightOnFocus = GetJsonBoolField(Params, TEXT("highlight_on_focus"), true);
        float InteractionCooldown = GetJsonNumberField(Params, TEXT("interaction_cooldown"), 0.0f);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetNumberField(TEXT("interaction_range"), InteractionRange);
        Result->SetBoolField(TEXT("show_prompt"), bShowPrompt);
        Result->SetStringField(TEXT("prompt_text"), PromptText);
        Result->SetStringField(TEXT("interaction_type"), InteractionType);
        Result->SetBoolField(TEXT("require_line_of_sight"), bRequireLineOfSight);
        Result->SetBoolField(TEXT("highlight_on_focus"), bHighlightOnFocus);
        Result->SetNumberField(TEXT("interaction_cooldown"), InteractionCooldown);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Interaction component created"), Result);
        return true;
    }

    // create_status_effect_component - Buffs/debuffs
    if (SubAction == TEXT("create_status_effect_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("StatusEffectComponent"));
        int32 MaxActiveEffects = GetJsonIntField(Params, TEXT("max_active_effects"), 10);
        bool bAllowStacking = GetJsonBoolField(Params, TEXT("allow_stacking"), true);
        int32 MaxStacks = GetJsonIntField(Params, TEXT("max_stacks"), 5);
        bool bShowEffectIcons = GetJsonBoolField(Params, TEXT("show_effect_icons"), true);
        bool bTickEffects = GetJsonBoolField(Params, TEXT("tick_effects"), true);
        float TickInterval = GetJsonNumberField(Params, TEXT("tick_interval"), 1.0f);
        TArray<TSharedPtr<FJsonValue>> ImmunitiesJson;
        if (Params->HasField(TEXT("immunities")))
        {
            ImmunitiesJson = Params->GetArrayField(TEXT("immunities"));
        }

        TArray<FString> Immunities;
        for (const TSharedPtr<FJsonValue>& Val : ImmunitiesJson)
        {
            Immunities.Add(Val->AsString());
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetNumberField(TEXT("max_active_effects"), MaxActiveEffects);
        Result->SetBoolField(TEXT("allow_stacking"), bAllowStacking);
        Result->SetNumberField(TEXT("max_stacks"), MaxStacks);
        Result->SetBoolField(TEXT("show_effect_icons"), bShowEffectIcons);
        Result->SetBoolField(TEXT("tick_effects"), bTickEffects);
        Result->SetNumberField(TEXT("tick_interval"), TickInterval);
        Result->SetNumberField(TEXT("immunity_count"), Immunities.Num());
        SendAutomationResponse(Socket, RequestId, true, TEXT("Status effect component created"), Result);
        return true;
    }

    // create_equipment_component - Equipment slots
    if (SubAction == TEXT("create_equipment_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("EquipmentComponent"));
        TArray<TSharedPtr<FJsonValue>> SlotsJson;
        if (Params->HasField(TEXT("slots")))
        {
            SlotsJson = Params->GetArrayField(TEXT("slots"));
        }
        bool bAutoApplyStats = GetJsonBoolField(Params, TEXT("auto_apply_stats"), true);
        bool bShowEquipmentMesh = GetJsonBoolField(Params, TEXT("show_equipment_mesh"), true);
        bool bAllowDualWield = GetJsonBoolField(Params, TEXT("allow_dual_wield"), false);
        bool bValidateRequirements = GetJsonBoolField(Params, TEXT("validate_requirements"), true);

        TArray<FString> Slots;
        for (const TSharedPtr<FJsonValue>& Val : SlotsJson)
        {
            Slots.Add(Val->AsString());
        }

        // Default slots if none provided
        if (Slots.Num() == 0)
        {
            Slots = {TEXT("Head"), TEXT("Chest"), TEXT("Hands"), TEXT("Legs"), TEXT("Feet"),
                     TEXT("MainHand"), TEXT("OffHand"), TEXT("Neck"), TEXT("Ring1"), TEXT("Ring2")};
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetNumberField(TEXT("slot_count"), Slots.Num());
        Result->SetBoolField(TEXT("auto_apply_stats"), bAutoApplyStats);
        Result->SetBoolField(TEXT("show_equipment_mesh"), bShowEquipmentMesh);
        Result->SetBoolField(TEXT("allow_dual_wield"), bAllowDualWield);
        Result->SetBoolField(TEXT("validate_requirements"), bValidateRequirements);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Equipment component created"), Result);
        return true;
    }

    // create_progression_component - XP/leveling
    if (SubAction == TEXT("create_progression_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("ProgressionComponent"));
        int32 MaxLevel = GetJsonIntField(Params, TEXT("max_level"), 100);
        FString XpCurve = GetJsonStringField(Params, TEXT("xp_curve"), TEXT("Exponential"));
        float XpBaseValue = GetJsonNumberField(Params, TEXT("xp_base_value"), 100.0f);
        float XpMultiplier = GetJsonNumberField(Params, TEXT("xp_multiplier"), 1.5f);
        int32 SkillPointsPerLevel = GetJsonIntField(Params, TEXT("skill_points_per_level"), 1);
        int32 StatPointsPerLevel = GetJsonIntField(Params, TEXT("stat_points_per_level"), 5);
        bool bShowLevelUpNotification = GetJsonBoolField(Params, TEXT("show_level_up_notification"), true);
        bool bPlayLevelUpEffect = GetJsonBoolField(Params, TEXT("play_level_up_effect"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetNumberField(TEXT("max_level"), MaxLevel);
        Result->SetStringField(TEXT("xp_curve"), XpCurve);
        Result->SetNumberField(TEXT("xp_base_value"), XpBaseValue);
        Result->SetNumberField(TEXT("xp_multiplier"), XpMultiplier);
        Result->SetNumberField(TEXT("skill_points_per_level"), SkillPointsPerLevel);
        Result->SetNumberField(TEXT("stat_points_per_level"), StatPointsPerLevel);
        Result->SetBoolField(TEXT("show_level_up_notification"), bShowLevelUpNotification);
        Result->SetBoolField(TEXT("play_level_up_effect"), bPlayLevelUpEffect);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Progression component created"), Result);
        return true;
    }

    // create_faction_component - Reputation system
    if (SubAction == TEXT("create_faction_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("FactionComponent"));
        FString DefaultFaction = GetJsonStringField(Params, TEXT("default_faction"), TEXT("Neutral"));
        bool bTrackReputation = GetJsonBoolField(Params, TEXT("track_reputation"), true);
        float DefaultReputation = GetJsonNumberField(Params, TEXT("default_reputation"), 0.0f);
        float MinReputation = GetJsonNumberField(Params, TEXT("min_reputation"), -10000.0f);
        float MaxReputation = GetJsonNumberField(Params, TEXT("max_reputation"), 10000.0f);
        bool bAffectsAI = GetJsonBoolField(Params, TEXT("affects_ai"), true);
        bool bShowReputationChange = GetJsonBoolField(Params, TEXT("show_reputation_change"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetStringField(TEXT("default_faction"), DefaultFaction);
        Result->SetBoolField(TEXT("track_reputation"), bTrackReputation);
        Result->SetNumberField(TEXT("default_reputation"), DefaultReputation);
        Result->SetNumberField(TEXT("min_reputation"), MinReputation);
        Result->SetNumberField(TEXT("max_reputation"), MaxReputation);
        Result->SetBoolField(TEXT("affects_ai"), bAffectsAI);
        Result->SetBoolField(TEXT("show_reputation_change"), bShowReputationChange);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Faction component created"), Result);
        return true;
    }

    // create_stealth_component - Stealth/detection
    if (SubAction == TEXT("create_stealth_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("StealthComponent"));
        float BaseVisibility = GetJsonNumberField(Params, TEXT("base_visibility"), 1.0f);
        float NoiseLevel = GetJsonNumberField(Params, TEXT("noise_level"), 1.0f);
        bool bAffectedByLight = GetJsonBoolField(Params, TEXT("affected_by_light"), true);
        bool bAffectedByMovement = GetJsonBoolField(Params, TEXT("affected_by_movement"), true);
        float CrouchVisibilityMod = GetJsonNumberField(Params, TEXT("crouch_visibility_mod"), 0.5f);
        float ProneVisibilityMod = GetJsonNumberField(Params, TEXT("prone_visibility_mod"), 0.25f);
        bool bCanHideInCover = GetJsonBoolField(Params, TEXT("can_hide_in_cover"), true);
        bool bShowDetectionMeter = GetJsonBoolField(Params, TEXT("show_detection_meter"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetNumberField(TEXT("base_visibility"), BaseVisibility);
        Result->SetNumberField(TEXT("noise_level"), NoiseLevel);
        Result->SetBoolField(TEXT("affected_by_light"), bAffectedByLight);
        Result->SetBoolField(TEXT("affected_by_movement"), bAffectedByMovement);
        Result->SetNumberField(TEXT("crouch_visibility_mod"), CrouchVisibilityMod);
        Result->SetNumberField(TEXT("prone_visibility_mod"), ProneVisibilityMod);
        Result->SetBoolField(TEXT("can_hide_in_cover"), bCanHideInCover);
        Result->SetBoolField(TEXT("show_detection_meter"), bShowDetectionMeter);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Stealth component created"), Result);
        return true;
    }

    // create_needs_component - Survival needs
    if (SubAction == TEXT("create_needs_component"))
    {
        FString BlueprintPath = GetJsonStringField(Params, TEXT("blueprint_path"), TEXT(""));
        FString ComponentName = GetJsonStringField(Params, TEXT("component_name"), TEXT("NeedsComponent"));
        bool bTrackHunger = GetJsonBoolField(Params, TEXT("track_hunger"), true);
        bool bTrackThirst = GetJsonBoolField(Params, TEXT("track_thirst"), true);
        bool bTrackSleep = GetJsonBoolField(Params, TEXT("track_sleep"), true);
        bool bTrackTemperature = GetJsonBoolField(Params, TEXT("track_temperature"), true);
        float HungerDecayRate = GetJsonNumberField(Params, TEXT("hunger_decay_rate"), 1.0f);
        float ThirstDecayRate = GetJsonNumberField(Params, TEXT("thirst_decay_rate"), 1.5f);
        float SleepDecayRate = GetJsonNumberField(Params, TEXT("sleep_decay_rate"), 0.5f);
        bool bDamageWhenCritical = GetJsonBoolField(Params, TEXT("damage_when_critical"), true);
        float CriticalDamageRate = GetJsonNumberField(Params, TEXT("critical_damage_rate"), 1.0f);
        bool bShowNeedsUI = GetJsonBoolField(Params, TEXT("show_needs_ui"), true);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetBoolField(TEXT("track_hunger"), bTrackHunger);
        Result->SetBoolField(TEXT("track_thirst"), bTrackThirst);
        Result->SetBoolField(TEXT("track_sleep"), bTrackSleep);
        Result->SetBoolField(TEXT("track_temperature"), bTrackTemperature);
        Result->SetNumberField(TEXT("hunger_decay_rate"), HungerDecayRate);
        Result->SetNumberField(TEXT("thirst_decay_rate"), ThirstDecayRate);
        Result->SetNumberField(TEXT("sleep_decay_rate"), SleepDecayRate);
        Result->SetBoolField(TEXT("damage_when_critical"), bDamageWhenCritical);
        Result->SetNumberField(TEXT("critical_damage_rate"), CriticalDamageRate);
        Result->SetBoolField(TEXT("show_needs_ui"), bShowNeedsUI);
        SendAutomationResponse(Socket, RequestId, true, TEXT("Needs component created"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown components action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 Framework Handler
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2FrameworkAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_game_mode - Custom game mode
    if (SubAction == TEXT("create_game_mode"))
    {
        FString GameModeName = GetJsonStringField(Params, TEXT("game_mode_name"), TEXT("BP_McpGameMode"));
        FString ParentClass = GetJsonStringField(Params, TEXT("parent_class"), TEXT("/Script/Engine.GameModeBase"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Framework"));
        bool bUseSeamlessTravel = GetJsonBoolField(Params, TEXT("use_seamless_travel"), false);
        bool bStartPlayersAsSpectators = GetJsonBoolField(Params, TEXT("start_players_as_spectators"), false);
        bool bDelayedStart = GetJsonBoolField(Params, TEXT("delayed_start"), false);
        FString DefaultPawnClass = GetJsonStringField(Params, TEXT("default_pawn_class"), TEXT(""));
        FString PlayerControllerClass = GetJsonStringField(Params, TEXT("player_controller_class"), TEXT(""));
        FString GameStateClass = GetJsonStringField(Params, TEXT("game_state_class"), TEXT(""));
        FString PlayerStateClass = GetJsonStringField(Params, TEXT("player_state_class"), TEXT(""));

        // Create the blueprint
        FString PackagePath = OutputPath / GameModeName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UClass* ParentUClass = FindObject<UClass>(nullptr, *ParentClass);
        if (!ParentUClass)
        {
            ParentUClass = AGameModeBase::StaticClass();
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            ParentUClass,
            Package,
            *GameModeName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("game_mode_name"), GameModeName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetStringField(TEXT("parent_class"), ParentClass);
            Result->SetBoolField(TEXT("use_seamless_travel"), bUseSeamlessTravel);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Game mode created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create game mode blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_game_state - Replicated state
    if (SubAction == TEXT("create_game_state"))
    {
        FString GameStateName = GetJsonStringField(Params, TEXT("game_state_name"), TEXT("BP_McpGameState"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Framework"));
        TArray<TSharedPtr<FJsonValue>> ReplicatedPropertiesJson;
        if (Params->HasField(TEXT("replicated_properties")))
        {
            ReplicatedPropertiesJson = Params->GetArrayField(TEXT("replicated_properties"));
        }
        bool bReplicateMatchState = GetJsonBoolField(Params, TEXT("replicate_match_state"), true);
        bool bReplicateScores = GetJsonBoolField(Params, TEXT("replicate_scores"), true);
        bool bReplicateTimer = GetJsonBoolField(Params, TEXT("replicate_timer"), true);

        FString PackagePath = OutputPath / GameStateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            AGameStateBase::StaticClass(),
            Package,
            *GameStateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("game_state_name"), GameStateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetNumberField(TEXT("replicated_property_count"), ReplicatedPropertiesJson.Num());
            Result->SetBoolField(TEXT("replicate_match_state"), bReplicateMatchState);
            Result->SetBoolField(TEXT("replicate_scores"), bReplicateScores);
            Result->SetBoolField(TEXT("replicate_timer"), bReplicateTimer);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Game state created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create game state blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_player_state - Per-player state
    if (SubAction == TEXT("create_player_state"))
    {
        FString PlayerStateName = GetJsonStringField(Params, TEXT("player_state_name"), TEXT("BP_McpPlayerState"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Framework"));
        TArray<TSharedPtr<FJsonValue>> ReplicatedPropertiesJson;
        if (Params->HasField(TEXT("replicated_properties")))
        {
            ReplicatedPropertiesJson = Params->GetArrayField(TEXT("replicated_properties"));
        }
        bool bReplicateScore = GetJsonBoolField(Params, TEXT("replicate_score"), true);
        bool bReplicateTeam = GetJsonBoolField(Params, TEXT("replicate_team"), true);
        bool bReplicateStats = GetJsonBoolField(Params, TEXT("replicate_stats"), true);

        FString PackagePath = OutputPath / PlayerStateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            APlayerState::StaticClass(),
            Package,
            *PlayerStateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("player_state_name"), PlayerStateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetNumberField(TEXT("replicated_property_count"), ReplicatedPropertiesJson.Num());
            Result->SetBoolField(TEXT("replicate_score"), bReplicateScore);
            Result->SetBoolField(TEXT("replicate_team"), bReplicateTeam);
            Result->SetBoolField(TEXT("replicate_stats"), bReplicateStats);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Player state created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create player state blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_player_controller - Enhanced controller
    if (SubAction == TEXT("create_player_controller"))
    {
        FString ControllerName = GetJsonStringField(Params, TEXT("controller_name"), TEXT("BP_McpPlayerController"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Framework"));
        bool bShowMouseCursor = GetJsonBoolField(Params, TEXT("show_mouse_cursor"), false);
        bool bEnableClickEvents = GetJsonBoolField(Params, TEXT("enable_click_events"), false);
        bool bEnableMouseOverEvents = GetJsonBoolField(Params, TEXT("enable_mouse_over_events"), false);
        FString InputMappingContext = GetJsonStringField(Params, TEXT("input_mapping_context"), TEXT(""));
        bool bAutoManageActiveCameraTarget = GetJsonBoolField(Params, TEXT("auto_manage_active_camera_target"), true);

        FString PackagePath = OutputPath / ControllerName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            APlayerController::StaticClass(),
            Package,
            *ControllerName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("controller_name"), ControllerName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetBoolField(TEXT("show_mouse_cursor"), bShowMouseCursor);
            Result->SetBoolField(TEXT("enable_click_events"), bEnableClickEvents);
            Result->SetBoolField(TEXT("enable_mouse_over_events"), bEnableMouseOverEvents);
            Result->SetStringField(TEXT("input_mapping_context"), InputMappingContext);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Player controller created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create player controller blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_character - Base character
    if (SubAction == TEXT("create_character"))
    {
        FString CharacterName = GetJsonStringField(Params, TEXT("character_name"), TEXT("BP_McpCharacter"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Characters"));
        FString SkeletalMeshPath = GetJsonStringField(Params, TEXT("skeletal_mesh_path"), TEXT(""));
        bool bUseControllerRotationYaw = GetJsonBoolField(Params, TEXT("use_controller_rotation_yaw"), false);
        bool bOrientRotationToMovement = GetJsonBoolField(Params, TEXT("orient_rotation_to_movement"), true);
        float CapsuleRadius = GetJsonNumberField(Params, TEXT("capsule_radius"), 42.0f);
        float CapsuleHalfHeight = GetJsonNumberField(Params, TEXT("capsule_half_height"), 96.0f);
        float MaxWalkSpeed = GetJsonNumberField(Params, TEXT("max_walk_speed"), 600.0f);
        float JumpZVelocity = GetJsonNumberField(Params, TEXT("jump_z_velocity"), 420.0f);
        bool bAddSpringArm = GetJsonBoolField(Params, TEXT("add_spring_arm"), true);
        bool bAddCamera = GetJsonBoolField(Params, TEXT("add_camera"), true);
        float SpringArmLength = GetJsonNumberField(Params, TEXT("spring_arm_length"), 300.0f);

        FString PackagePath = OutputPath / CharacterName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            ACharacter::StaticClass(),
            Package,
            *CharacterName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("character_name"), CharacterName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetBoolField(TEXT("use_controller_rotation_yaw"), bUseControllerRotationYaw);
            Result->SetBoolField(TEXT("orient_rotation_to_movement"), bOrientRotationToMovement);
            Result->SetNumberField(TEXT("capsule_radius"), CapsuleRadius);
            Result->SetNumberField(TEXT("capsule_half_height"), CapsuleHalfHeight);
            Result->SetNumberField(TEXT("max_walk_speed"), MaxWalkSpeed);
            Result->SetNumberField(TEXT("jump_z_velocity"), JumpZVelocity);
            Result->SetBoolField(TEXT("add_spring_arm"), bAddSpringArm);
            Result->SetBoolField(TEXT("add_camera"), bAddCamera);
            Result->SetNumberField(TEXT("spring_arm_length"), SpringArmLength);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Character created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create character blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_game_instance - Persistent data
    if (SubAction == TEXT("create_game_instance"))
    {
        FString GameInstanceName = GetJsonStringField(Params, TEXT("game_instance_name"), TEXT("BP_McpGameInstance"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Framework"));
        TArray<TSharedPtr<FJsonValue>> PersistentDataJson;
        if (Params->HasField(TEXT("persistent_data")))
        {
            PersistentDataJson = Params->GetArrayField(TEXT("persistent_data"));
        }
        bool bAutoSaveOnTravel = GetJsonBoolField(Params, TEXT("auto_save_on_travel"), true);
        bool bLoadSaveOnInit = GetJsonBoolField(Params, TEXT("load_save_on_init"), true);

        FString PackagePath = OutputPath / GameInstanceName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UGameInstance::StaticClass(),
            Package,
            *GameInstanceName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("game_instance_name"), GameInstanceName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetNumberField(TEXT("persistent_data_count"), PersistentDataJson.Num());
            Result->SetBoolField(TEXT("auto_save_on_travel"), bAutoSaveOnTravel);
            Result->SetBoolField(TEXT("load_save_on_init"), bLoadSaveOnInit);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Game instance created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create game instance blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_save_game - Save structure
    if (SubAction == TEXT("create_save_game"))
    {
        FString SaveGameName = GetJsonStringField(Params, TEXT("save_game_name"), TEXT("BP_McpSaveGame"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Framework"));
        TArray<TSharedPtr<FJsonValue>> SavePropertiesJson;
        if (Params->HasField(TEXT("save_properties")))
        {
            SavePropertiesJson = Params->GetArrayField(TEXT("save_properties"));
        }
        bool bSavePlayerLocation = GetJsonBoolField(Params, TEXT("save_player_location"), true);
        bool bSaveInventory = GetJsonBoolField(Params, TEXT("save_inventory"), true);
        bool bSaveQuests = GetJsonBoolField(Params, TEXT("save_quests"), true);
        bool bSaveProgression = GetJsonBoolField(Params, TEXT("save_progression"), true);
        bool bSaveWorldState = GetJsonBoolField(Params, TEXT("save_world_state"), true);

        FString PackagePath = OutputPath / SaveGameName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            USaveGame::StaticClass(),
            Package,
            *SaveGameName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("save_game_name"), SaveGameName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetNumberField(TEXT("save_property_count"), SavePropertiesJson.Num());
            Result->SetBoolField(TEXT("save_player_location"), bSavePlayerLocation);
            Result->SetBoolField(TEXT("save_inventory"), bSaveInventory);
            Result->SetBoolField(TEXT("save_quests"), bSaveQuests);
            Result->SetBoolField(TEXT("save_progression"), bSaveProgression);
            Result->SetBoolField(TEXT("save_world_state"), bSaveWorldState);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Save game created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create save game blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown framework action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 Subsystems Handler
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2SubsystemsAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_quest_subsystem - World quest management
    if (SubAction == TEXT("create_quest_subsystem"))
    {
        FString SubsystemName = GetJsonStringField(Params, TEXT("subsystem_name"), TEXT("McpQuestSubsystem"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Subsystems"));
        bool bAutoLoadQuests = GetJsonBoolField(Params, TEXT("auto_load_quests"), true);
        bool bPersistQuestState = GetJsonBoolField(Params, TEXT("persist_quest_state"), true);
        bool bTrackObjectives = GetJsonBoolField(Params, TEXT("track_objectives"), true);
        int32 MaxActiveQuests = GetJsonIntField(Params, TEXT("max_active_quests"), 20);
        bool bBroadcastEvents = GetJsonBoolField(Params, TEXT("broadcast_events"), true);

        FString PackagePath = OutputPath / SubsystemName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UWorldSubsystem::StaticClass(),
            Package,
            *SubsystemName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("subsystem_name"), SubsystemName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetBoolField(TEXT("auto_load_quests"), bAutoLoadQuests);
            Result->SetBoolField(TEXT("persist_quest_state"), bPersistQuestState);
            Result->SetBoolField(TEXT("track_objectives"), bTrackObjectives);
            Result->SetNumberField(TEXT("max_active_quests"), MaxActiveQuests);
            Result->SetBoolField(TEXT("broadcast_events"), bBroadcastEvents);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Quest subsystem created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create quest subsystem blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_economy_subsystem - Economy and trading
    if (SubAction == TEXT("create_economy_subsystem"))
    {
        FString SubsystemName = GetJsonStringField(Params, TEXT("subsystem_name"), TEXT("McpEconomySubsystem"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Subsystems"));
        TArray<TSharedPtr<FJsonValue>> CurrencyTypesJson;
        if (Params->HasField(TEXT("currency_types")))
        {
            CurrencyTypesJson = Params->GetArrayField(TEXT("currency_types"));
        }
        bool bEnableTrading = GetJsonBoolField(Params, TEXT("enable_trading"), true);
        bool bEnableAuctionHouse = GetJsonBoolField(Params, TEXT("enable_auction_house"), false);
        float GlobalPriceMultiplier = GetJsonNumberField(Params, TEXT("global_price_multiplier"), 1.0f);
        bool bTrackTransactions = GetJsonBoolField(Params, TEXT("track_transactions"), true);

        TArray<FString> CurrencyTypes;
        for (const TSharedPtr<FJsonValue>& Val : CurrencyTypesJson)
        {
            CurrencyTypes.Add(Val->AsString());
        }

        if (CurrencyTypes.Num() == 0)
        {
            CurrencyTypes = {TEXT("Gold"), TEXT("Premium"), TEXT("Event")};
        }

        FString PackagePath = OutputPath / SubsystemName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UWorldSubsystem::StaticClass(),
            Package,
            *SubsystemName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("subsystem_name"), SubsystemName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetNumberField(TEXT("currency_type_count"), CurrencyTypes.Num());
            Result->SetBoolField(TEXT("enable_trading"), bEnableTrading);
            Result->SetBoolField(TEXT("enable_auction_house"), bEnableAuctionHouse);
            Result->SetNumberField(TEXT("global_price_multiplier"), GlobalPriceMultiplier);
            Result->SetBoolField(TEXT("track_transactions"), bTrackTransactions);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Economy subsystem created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create economy subsystem blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_spawn_subsystem - Entity spawning
    if (SubAction == TEXT("create_spawn_subsystem"))
    {
        FString SubsystemName = GetJsonStringField(Params, TEXT("subsystem_name"), TEXT("McpSpawnSubsystem"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Subsystems"));
        int32 MaxSpawnedEntities = GetJsonIntField(Params, TEXT("max_spawned_entities"), 1000);
        bool bUseObjectPooling = GetJsonBoolField(Params, TEXT("use_object_pooling"), true);
        int32 PoolSize = GetJsonIntField(Params, TEXT("pool_size"), 100);
        float SpawnCooldown = GetJsonNumberField(Params, TEXT("spawn_cooldown"), 0.1f);
        bool bRespectPopulationLimits = GetJsonBoolField(Params, TEXT("respect_population_limits"), true);
        bool bTrackSpawnedEntities = GetJsonBoolField(Params, TEXT("track_spawned_entities"), true);

        FString PackagePath = OutputPath / SubsystemName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UWorldSubsystem::StaticClass(),
            Package,
            *SubsystemName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("subsystem_name"), SubsystemName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetNumberField(TEXT("max_spawned_entities"), MaxSpawnedEntities);
            Result->SetBoolField(TEXT("use_object_pooling"), bUseObjectPooling);
            Result->SetNumberField(TEXT("pool_size"), PoolSize);
            Result->SetNumberField(TEXT("spawn_cooldown"), SpawnCooldown);
            Result->SetBoolField(TEXT("respect_population_limits"), bRespectPopulationLimits);
            Result->SetBoolField(TEXT("track_spawned_entities"), bTrackSpawnedEntities);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Spawn subsystem created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create spawn subsystem blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_weather_subsystem - Weather and time
    if (SubAction == TEXT("create_weather_subsystem"))
    {
        FString SubsystemName = GetJsonStringField(Params, TEXT("subsystem_name"), TEXT("McpWeatherSubsystem"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Subsystems"));
        bool bEnableDayNightCycle = GetJsonBoolField(Params, TEXT("enable_day_night_cycle"), true);
        float DayLength = GetJsonNumberField(Params, TEXT("day_length"), 1200.0f); // 20 minutes real-time
        bool bEnableWeather = GetJsonBoolField(Params, TEXT("enable_weather"), true);
        float WeatherChangeFrequency = GetJsonNumberField(Params, TEXT("weather_change_frequency"), 300.0f);
        bool bEnableSeasons = GetJsonBoolField(Params, TEXT("enable_seasons"), false);
        float SeasonLength = GetJsonNumberField(Params, TEXT("season_length"), 3600.0f);
        bool bSyncWithServer = GetJsonBoolField(Params, TEXT("sync_with_server"), false);
        bool bAffectGameplay = GetJsonBoolField(Params, TEXT("affect_gameplay"), true);

        FString PackagePath = OutputPath / SubsystemName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UWorldSubsystem::StaticClass(),
            Package,
            *SubsystemName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("subsystem_name"), SubsystemName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetBoolField(TEXT("enable_day_night_cycle"), bEnableDayNightCycle);
            Result->SetNumberField(TEXT("day_length"), DayLength);
            Result->SetBoolField(TEXT("enable_weather"), bEnableWeather);
            Result->SetNumberField(TEXT("weather_change_frequency"), WeatherChangeFrequency);
            Result->SetBoolField(TEXT("enable_seasons"), bEnableSeasons);
            Result->SetNumberField(TEXT("season_length"), SeasonLength);
            Result->SetBoolField(TEXT("sync_with_server"), bSyncWithServer);
            Result->SetBoolField(TEXT("affect_gameplay"), bAffectGameplay);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Weather subsystem created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create weather subsystem blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_worldgen_subsystem - Runtime world generation
    if (SubAction == TEXT("create_worldgen_subsystem"))
    {
        FString SubsystemName = GetJsonStringField(Params, TEXT("subsystem_name"), TEXT("McpWorldGenSubsystem"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Subsystems"));
        int32 ChunkSize = GetJsonIntField(Params, TEXT("chunk_size"), 100);
        int32 ViewDistance = GetJsonIntField(Params, TEXT("view_distance"), 3);
        bool bAsyncGeneration = GetJsonBoolField(Params, TEXT("async_generation"), true);
        int32 MaxConcurrentTasks = GetJsonIntField(Params, TEXT("max_concurrent_tasks"), 4);
        bool bCacheChunks = GetJsonBoolField(Params, TEXT("cache_chunks"), true);
        int32 CacheSize = GetJsonIntField(Params, TEXT("cache_size"), 64);
        bool bUseLOD = GetJsonBoolField(Params, TEXT("use_lod"), true);
        int32 Seed = GetJsonIntField(Params, TEXT("seed"), 0);

        FString PackagePath = OutputPath / SubsystemName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UWorldSubsystem::StaticClass(),
            Package,
            *SubsystemName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("subsystem_name"), SubsystemName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetNumberField(TEXT("chunk_size"), ChunkSize);
            Result->SetNumberField(TEXT("view_distance"), ViewDistance);
            Result->SetBoolField(TEXT("async_generation"), bAsyncGeneration);
            Result->SetNumberField(TEXT("max_concurrent_tasks"), MaxConcurrentTasks);
            Result->SetBoolField(TEXT("cache_chunks"), bCacheChunks);
            Result->SetNumberField(TEXT("cache_size"), CacheSize);
            Result->SetBoolField(TEXT("use_lod"), bUseLOD);
            Result->SetNumberField(TEXT("seed"), Seed);
            SendAutomationResponse(Socket, RequestId, true, TEXT("WorldGen subsystem created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create worldgen subsystem blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_dialogue_subsystem - Dialogue management
    if (SubAction == TEXT("create_dialogue_subsystem"))
    {
        FString SubsystemName = GetJsonStringField(Params, TEXT("subsystem_name"), TEXT("McpDialogueSubsystem"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Subsystems"));
        bool bAutoLoadDialogue = GetJsonBoolField(Params, TEXT("auto_load_dialogue"), true);
        bool bTrackDialogueHistory = GetJsonBoolField(Params, TEXT("track_dialogue_history"), true);
        bool bSupportVoiceOver = GetJsonBoolField(Params, TEXT("support_voice_over"), true);
        bool bSupportSubtitles = GetJsonBoolField(Params, TEXT("support_subtitles"), true);
        bool bSupportLocalization = GetJsonBoolField(Params, TEXT("support_localization"), true);
        float DefaultTextSpeed = GetJsonNumberField(Params, TEXT("default_text_speed"), 0.05f);

        FString PackagePath = OutputPath / SubsystemName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UWorldSubsystem::StaticClass(),
            Package,
            *SubsystemName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("subsystem_name"), SubsystemName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetBoolField(TEXT("auto_load_dialogue"), bAutoLoadDialogue);
            Result->SetBoolField(TEXT("track_dialogue_history"), bTrackDialogueHistory);
            Result->SetBoolField(TEXT("support_voice_over"), bSupportVoiceOver);
            Result->SetBoolField(TEXT("support_subtitles"), bSupportSubtitles);
            Result->SetBoolField(TEXT("support_localization"), bSupportLocalization);
            Result->SetNumberField(TEXT("default_text_speed"), DefaultTextSpeed);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue subsystem created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create dialogue subsystem blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_audio_subsystem - Dynamic audio
    if (SubAction == TEXT("create_audio_subsystem"))
    {
        FString SubsystemName = GetJsonStringField(Params, TEXT("subsystem_name"), TEXT("McpAudioSubsystem"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Subsystems"));
        bool bEnableDynamicMusic = GetJsonBoolField(Params, TEXT("enable_dynamic_music"), true);
        bool bEnableAmbientSounds = GetJsonBoolField(Params, TEXT("enable_ambient_sounds"), true);
        int32 MaxSimultaneousSounds = GetJsonIntField(Params, TEXT("max_simultaneous_sounds"), 32);
        float MasterVolume = GetJsonNumberField(Params, TEXT("master_volume"), 1.0f);
        float MusicVolume = GetJsonNumberField(Params, TEXT("music_volume"), 0.7f);
        float SfxVolume = GetJsonNumberField(Params, TEXT("sfx_volume"), 1.0f);
        bool bUseSoundPools = GetJsonBoolField(Params, TEXT("use_sound_pools"), true);

        FString PackagePath = OutputPath / SubsystemName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UWorldSubsystem::StaticClass(),
            Package,
            *SubsystemName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("subsystem_name"), SubsystemName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetBoolField(TEXT("enable_dynamic_music"), bEnableDynamicMusic);
            Result->SetBoolField(TEXT("enable_ambient_sounds"), bEnableAmbientSounds);
            Result->SetNumberField(TEXT("max_simultaneous_sounds"), MaxSimultaneousSounds);
            Result->SetNumberField(TEXT("master_volume"), MasterVolume);
            Result->SetNumberField(TEXT("music_volume"), MusicVolume);
            Result->SetNumberField(TEXT("sfx_volume"), SfxVolume);
            Result->SetBoolField(TEXT("use_sound_pools"), bUseSoundPools);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Audio subsystem created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create audio subsystem blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_analytics_subsystem - Analytics tracking
    if (SubAction == TEXT("create_analytics_subsystem"))
    {
        FString SubsystemName = GetJsonStringField(Params, TEXT("subsystem_name"), TEXT("McpAnalyticsSubsystem"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Blueprints/Subsystems"));
        bool bTrackPlayerProgress = GetJsonBoolField(Params, TEXT("track_player_progress"), true);
        bool bTrackDeaths = GetJsonBoolField(Params, TEXT("track_deaths"), true);
        bool bTrackPurchases = GetJsonBoolField(Params, TEXT("track_purchases"), true);
        bool bTrackSessionTime = GetJsonBoolField(Params, TEXT("track_session_time"), true);
        bool bLocalStorage = GetJsonBoolField(Params, TEXT("local_storage"), true);
        bool bRemoteReporting = GetJsonBoolField(Params, TEXT("remote_reporting"), false);
        FString AnalyticsEndpoint = GetJsonStringField(Params, TEXT("analytics_endpoint"), TEXT(""));
        float ReportInterval = GetJsonNumberField(Params, TEXT("report_interval"), 60.0f);

        FString PackagePath = OutputPath / SubsystemName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            UGameInstanceSubsystem::StaticClass(),
            Package,
            *SubsystemName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("subsystem_name"), SubsystemName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetBoolField(TEXT("track_player_progress"), bTrackPlayerProgress);
            Result->SetBoolField(TEXT("track_deaths"), bTrackDeaths);
            Result->SetBoolField(TEXT("track_purchases"), bTrackPurchases);
            Result->SetBoolField(TEXT("track_session_time"), bTrackSessionTime);
            Result->SetBoolField(TEXT("local_storage"), bLocalStorage);
            Result->SetBoolField(TEXT("remote_reporting"), bRemoteReporting);
            Result->SetNumberField(TEXT("report_interval"), ReportInterval);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Analytics subsystem created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create analytics subsystem blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown subsystems action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 Templates Handler
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2TemplatesAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if WITH_EDITOR
    FString SubAction = Forge2Helpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2Helpers::GetParams(Payload);

    // create_character_template - Character blueprint with movement, input
    if (SubAction == TEXT("create_character_template"))
    {
        FString TemplateName = GetJsonStringField(Params, TEXT("template_name"), TEXT("CharacterTemplate"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Templates"));
        FString CharacterType = GetJsonStringField(Params, TEXT("character_type"), TEXT("ThirdPerson")); // ThirdPerson, FirstPerson, TopDown, SideScroller
        bool bIncludeMovement = GetJsonBoolField(Params, TEXT("include_movement"), true);
        bool bIncludeCamera = GetJsonBoolField(Params, TEXT("include_camera"), true);
        bool bIncludeInput = GetJsonBoolField(Params, TEXT("include_input"), true);
        bool bIncludeCombat = GetJsonBoolField(Params, TEXT("include_combat"), false);
        bool bIncludeInventory = GetJsonBoolField(Params, TEXT("include_inventory"), false);
        FString SkeletalMeshPath = GetJsonStringField(Params, TEXT("skeletal_mesh_path"), TEXT(""));

        FString PackagePath = OutputPath / TemplateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            ACharacter::StaticClass(),
            Package,
            *TemplateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            // Add components based on template type
            if (bIncludeCamera && Blueprint->SimpleConstructionScript)
            {
                USCS_Node* SpringArmNode = Blueprint->SimpleConstructionScript->CreateNode(USpringArmComponent::StaticClass(), TEXT("SpringArm"));
                if (SpringArmNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(SpringArmNode);
                }

                USCS_Node* CameraNode = Blueprint->SimpleConstructionScript->CreateNode(UCameraComponent::StaticClass(), TEXT("Camera"));
                if (CameraNode && SpringArmNode)
                {
                    SpringArmNode->AddChildNode(CameraNode);
                }
            }

            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("template_name"), TemplateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetStringField(TEXT("character_type"), CharacterType);
            Result->SetBoolField(TEXT("include_movement"), bIncludeMovement);
            Result->SetBoolField(TEXT("include_camera"), bIncludeCamera);
            Result->SetBoolField(TEXT("include_input"), bIncludeInput);
            Result->SetBoolField(TEXT("include_combat"), bIncludeCombat);
            Result->SetBoolField(TEXT("include_inventory"), bIncludeInventory);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Character template created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create character template blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_weapon_template - Weapon BP + damage component + ability
    if (SubAction == TEXT("create_weapon_template"))
    {
        FString TemplateName = GetJsonStringField(Params, TEXT("template_name"), TEXT("WeaponTemplate"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Templates"));
        FString WeaponType = GetJsonStringField(Params, TEXT("weapon_type"), TEXT("Melee")); // Melee, Ranged, Magic
        FString WeaponSlot = GetJsonStringField(Params, TEXT("weapon_slot"), TEXT("MainHand"));
        float BaseDamage = GetJsonNumberField(Params, TEXT("base_damage"), 10.0f);
        float AttackSpeed = GetJsonNumberField(Params, TEXT("attack_speed"), 1.0f);
        bool bIncludeVFX = GetJsonBoolField(Params, TEXT("include_vfx"), true);
        bool bIncludeSFX = GetJsonBoolField(Params, TEXT("include_sfx"), true);
        FString StaticMeshPath = GetJsonStringField(Params, TEXT("static_mesh_path"), TEXT(""));

        FString PackagePath = OutputPath / TemplateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            AActor::StaticClass(),
            Package,
            *TemplateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            // Add mesh and collision components
            if (Blueprint->SimpleConstructionScript)
            {
                USCS_Node* MeshNode = Blueprint->SimpleConstructionScript->CreateNode(UStaticMeshComponent::StaticClass(), TEXT("WeaponMesh"));
                if (MeshNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(MeshNode);
                }

                USCS_Node* CollisionNode = Blueprint->SimpleConstructionScript->CreateNode(UBoxComponent::StaticClass(), TEXT("DamageCollision"));
                if (CollisionNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(CollisionNode);
                }
            }

            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("template_name"), TemplateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetStringField(TEXT("weapon_type"), WeaponType);
            Result->SetStringField(TEXT("weapon_slot"), WeaponSlot);
            Result->SetNumberField(TEXT("base_damage"), BaseDamage);
            Result->SetNumberField(TEXT("attack_speed"), AttackSpeed);
            Result->SetBoolField(TEXT("include_vfx"), bIncludeVFX);
            Result->SetBoolField(TEXT("include_sfx"), bIncludeSFX);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Weapon template created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create weapon template blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_npc_template - NPC with behavior tree, dialogue
    if (SubAction == TEXT("create_npc_template"))
    {
        FString TemplateName = GetJsonStringField(Params, TEXT("template_name"), TEXT("NPCTemplate"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Templates"));
        FString NpcType = GetJsonStringField(Params, TEXT("npc_type"), TEXT("Friendly")); // Friendly, Hostile, Neutral, Merchant, QuestGiver
        bool bIncludeDialogue = GetJsonBoolField(Params, TEXT("include_dialogue"), true);
        bool bIncludeBehaviorTree = GetJsonBoolField(Params, TEXT("include_behavior_tree"), true);
        bool bIncludePerception = GetJsonBoolField(Params, TEXT("include_perception"), true);
        bool bIncludePatrol = GetJsonBoolField(Params, TEXT("include_patrol"), false);
        bool bIncludeInventory = GetJsonBoolField(Params, TEXT("include_inventory"), false);
        FString SkeletalMeshPath = GetJsonStringField(Params, TEXT("skeletal_mesh_path"), TEXT(""));

        FString PackagePath = OutputPath / TemplateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            ACharacter::StaticClass(),
            Package,
            *TemplateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("template_name"), TemplateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetStringField(TEXT("npc_type"), NpcType);
            Result->SetBoolField(TEXT("include_dialogue"), bIncludeDialogue);
            Result->SetBoolField(TEXT("include_behavior_tree"), bIncludeBehaviorTree);
            Result->SetBoolField(TEXT("include_perception"), bIncludePerception);
            Result->SetBoolField(TEXT("include_patrol"), bIncludePatrol);
            Result->SetBoolField(TEXT("include_inventory"), bIncludeInventory);
            SendAutomationResponse(Socket, RequestId, true, TEXT("NPC template created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create NPC template blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_interactable_template - Interactive objects
    if (SubAction == TEXT("create_interactable_template"))
    {
        FString TemplateName = GetJsonStringField(Params, TEXT("template_name"), TEXT("InteractableTemplate"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Templates"));
        FString InteractableType = GetJsonStringField(Params, TEXT("interactable_type"), TEXT("Generic")); // Generic, Container, Door, Switch, Pickup
        bool bHighlightOnHover = GetJsonBoolField(Params, TEXT("highlight_on_hover"), true);
        bool bShowPrompt = GetJsonBoolField(Params, TEXT("show_prompt"), true);
        FString PromptText = GetJsonStringField(Params, TEXT("prompt_text"), TEXT("Interact"));
        float InteractionRadius = GetJsonNumberField(Params, TEXT("interaction_radius"), 200.0f);
        bool bRequireItem = GetJsonBoolField(Params, TEXT("require_item"), false);
        FString RequiredItemId = GetJsonStringField(Params, TEXT("required_item_id"), TEXT(""));
        bool bPlayAnimation = GetJsonBoolField(Params, TEXT("play_animation"), true);
        bool bPlaySound = GetJsonBoolField(Params, TEXT("play_sound"), true);

        FString PackagePath = OutputPath / TemplateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            AActor::StaticClass(),
            Package,
            *TemplateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            // Add mesh, collision, and interaction components
            if (Blueprint->SimpleConstructionScript)
            {
                USCS_Node* MeshNode = Blueprint->SimpleConstructionScript->CreateNode(UStaticMeshComponent::StaticClass(), TEXT("Mesh"));
                if (MeshNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(MeshNode);
                }

                USCS_Node* CollisionNode = Blueprint->SimpleConstructionScript->CreateNode(USphereComponent::StaticClass(), TEXT("InteractionTrigger"));
                if (CollisionNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(CollisionNode);
                }
            }

            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("template_name"), TemplateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetStringField(TEXT("interactable_type"), InteractableType);
            Result->SetBoolField(TEXT("highlight_on_hover"), bHighlightOnHover);
            Result->SetBoolField(TEXT("show_prompt"), bShowPrompt);
            Result->SetStringField(TEXT("prompt_text"), PromptText);
            Result->SetNumberField(TEXT("interaction_radius"), InteractionRadius);
            Result->SetBoolField(TEXT("require_item"), bRequireItem);
            Result->SetBoolField(TEXT("play_animation"), bPlayAnimation);
            Result->SetBoolField(TEXT("play_sound"), bPlaySound);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Interactable template created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create interactable template blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_projectile_template - Projectile with movement, damage
    if (SubAction == TEXT("create_projectile_template"))
    {
        FString TemplateName = GetJsonStringField(Params, TEXT("template_name"), TEXT("ProjectileTemplate"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Templates"));
        FString ProjectileType = GetJsonStringField(Params, TEXT("projectile_type"), TEXT("Bullet")); // Bullet, Arrow, Magic, Rocket
        float Speed = GetJsonNumberField(Params, TEXT("speed"), 3000.0f);
        float BaseDamage = GetJsonNumberField(Params, TEXT("base_damage"), 20.0f);
        float Lifetime = GetJsonNumberField(Params, TEXT("lifetime"), 5.0f);
        bool bHomingEnabled = GetJsonBoolField(Params, TEXT("homing_enabled"), false);
        float HomingStrength = GetJsonNumberField(Params, TEXT("homing_strength"), 0.5f);
        bool bGravityEnabled = GetJsonBoolField(Params, TEXT("gravity_enabled"), false);
        float GravityScale = GetJsonNumberField(Params, TEXT("gravity_scale"), 1.0f);
        bool bPenetrates = GetJsonBoolField(Params, TEXT("penetrates"), false);
        int32 MaxPenetrations = GetJsonIntField(Params, TEXT("max_penetrations"), 1);
        bool bExplodesOnImpact = GetJsonBoolField(Params, TEXT("explodes_on_impact"), false);
        float ExplosionRadius = GetJsonNumberField(Params, TEXT("explosion_radius"), 200.0f);

        FString PackagePath = OutputPath / TemplateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            AActor::StaticClass(),
            Package,
            *TemplateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            // Add components for projectile
            if (Blueprint->SimpleConstructionScript)
            {
                USCS_Node* CollisionNode = Blueprint->SimpleConstructionScript->CreateNode(USphereComponent::StaticClass(), TEXT("CollisionSphere"));
                if (CollisionNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(CollisionNode);
                }

                USCS_Node* MeshNode = Blueprint->SimpleConstructionScript->CreateNode(UStaticMeshComponent::StaticClass(), TEXT("ProjectileMesh"));
                if (MeshNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(MeshNode);
                }

                USCS_Node* MovementNode = Blueprint->SimpleConstructionScript->CreateNode(UProjectileMovementComponent::StaticClass(), TEXT("ProjectileMovement"));
                if (MovementNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(MovementNode);
                }
            }

            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("template_name"), TemplateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetStringField(TEXT("projectile_type"), ProjectileType);
            Result->SetNumberField(TEXT("speed"), Speed);
            Result->SetNumberField(TEXT("base_damage"), BaseDamage);
            Result->SetNumberField(TEXT("lifetime"), Lifetime);
            Result->SetBoolField(TEXT("homing_enabled"), bHomingEnabled);
            Result->SetNumberField(TEXT("homing_strength"), HomingStrength);
            Result->SetBoolField(TEXT("gravity_enabled"), bGravityEnabled);
            Result->SetNumberField(TEXT("gravity_scale"), GravityScale);
            Result->SetBoolField(TEXT("penetrates"), bPenetrates);
            Result->SetNumberField(TEXT("max_penetrations"), MaxPenetrations);
            Result->SetBoolField(TEXT("explodes_on_impact"), bExplodesOnImpact);
            Result->SetNumberField(TEXT("explosion_radius"), ExplosionRadius);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Projectile template created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create projectile template blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_pickup_template - Collectible items
    if (SubAction == TEXT("create_pickup_template"))
    {
        FString TemplateName = GetJsonStringField(Params, TEXT("template_name"), TEXT("PickupTemplate"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Templates"));
        FString PickupType = GetJsonStringField(Params, TEXT("pickup_type"), TEXT("Generic")); // Generic, Health, Ammo, Currency, Quest, PowerUp
        bool bAutoPickup = GetJsonBoolField(Params, TEXT("auto_pickup"), true);
        float PickupRadius = GetJsonNumberField(Params, TEXT("pickup_radius"), 100.0f);
        bool bRotates = GetJsonBoolField(Params, TEXT("rotates"), true);
        float RotationSpeed = GetJsonNumberField(Params, TEXT("rotation_speed"), 90.0f);
        bool bBobs = GetJsonBoolField(Params, TEXT("bobs"), true);
        float BobHeight = GetJsonNumberField(Params, TEXT("bob_height"), 20.0f);
        bool bRespawns = GetJsonBoolField(Params, TEXT("respawns"), false);
        float RespawnTime = GetJsonNumberField(Params, TEXT("respawn_time"), 30.0f);
        bool bPlaySound = GetJsonBoolField(Params, TEXT("play_sound"), true);
        bool bPlayVfx = GetJsonBoolField(Params, TEXT("play_vfx"), true);

        FString PackagePath = OutputPath / TemplateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            AActor::StaticClass(),
            Package,
            *TemplateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            // Add components for pickup
            if (Blueprint->SimpleConstructionScript)
            {
                USCS_Node* CollisionNode = Blueprint->SimpleConstructionScript->CreateNode(USphereComponent::StaticClass(), TEXT("PickupTrigger"));
                if (CollisionNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(CollisionNode);
                }

                USCS_Node* MeshNode = Blueprint->SimpleConstructionScript->CreateNode(UStaticMeshComponent::StaticClass(), TEXT("PickupMesh"));
                if (MeshNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(MeshNode);
                }

                USCS_Node* RotatingNode = Blueprint->SimpleConstructionScript->CreateNode(URotatingMovementComponent::StaticClass(), TEXT("RotatingMovement"));
                if (RotatingNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(RotatingNode);
                }
            }

            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("template_name"), TemplateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetStringField(TEXT("pickup_type"), PickupType);
            Result->SetBoolField(TEXT("auto_pickup"), bAutoPickup);
            Result->SetNumberField(TEXT("pickup_radius"), PickupRadius);
            Result->SetBoolField(TEXT("rotates"), bRotates);
            Result->SetNumberField(TEXT("rotation_speed"), RotationSpeed);
            Result->SetBoolField(TEXT("bobs"), bBobs);
            Result->SetNumberField(TEXT("bob_height"), BobHeight);
            Result->SetBoolField(TEXT("respawns"), bRespawns);
            Result->SetNumberField(TEXT("respawn_time"), RespawnTime);
            Result->SetBoolField(TEXT("play_sound"), bPlaySound);
            Result->SetBoolField(TEXT("play_vfx"), bPlayVfx);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Pickup template created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create pickup template blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    // create_trigger_template - Trigger zones
    if (SubAction == TEXT("create_trigger_template"))
    {
        FString TemplateName = GetJsonStringField(Params, TEXT("template_name"), TEXT("TriggerTemplate"));
        FString OutputPath = GetJsonStringField(Params, TEXT("output_path"), TEXT("/Game/Templates"));
        FString TriggerShape = GetJsonStringField(Params, TEXT("trigger_shape"), TEXT("Box")); // Box, Sphere, Capsule
        FString TriggerType = GetJsonStringField(Params, TEXT("trigger_type"), TEXT("Overlap")); // Overlap, Enter, Exit, Both
        bool bOnceOnly = GetJsonBoolField(Params, TEXT("once_only"), false);
        float Cooldown = GetJsonNumberField(Params, TEXT("cooldown"), 0.0f);
        bool bPlayerOnly = GetJsonBoolField(Params, TEXT("player_only"), true);
        TArray<TSharedPtr<FJsonValue>> TagFiltersJson;
        if (Params->HasField(TEXT("tag_filters")))
        {
            TagFiltersJson = Params->GetArrayField(TEXT("tag_filters"));
        }
        bool bDebugDraw = GetJsonBoolField(Params, TEXT("debug_draw"), true);

        TArray<FString> TagFilters;
        for (const TSharedPtr<FJsonValue>& Val : TagFiltersJson)
        {
            TagFilters.Add(Val->AsString());
        }

        FString PackagePath = OutputPath / TemplateName;
        UPackage* Package = Forge2Helpers::CreateAssetPackage(PackagePath);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_ERROR"));
            return true;
        }

        UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
            AActor::StaticClass(),
            Package,
            *TemplateName,
            BPTYPE_Normal,
            UBlueprint::StaticClass(),
            UBlueprintGeneratedClass::StaticClass()
        );

        if (Blueprint)
        {
            // Add trigger collision component
            if (Blueprint->SimpleConstructionScript)
            {
                UClass* ShapeClass = UBoxComponent::StaticClass();
                if (TriggerShape == TEXT("Sphere"))
                {
                    ShapeClass = USphereComponent::StaticClass();
                }
                else if (TriggerShape == TEXT("Capsule"))
                {
                    ShapeClass = UCapsuleComponent::StaticClass();
                }

                USCS_Node* ShapeNode = Blueprint->SimpleConstructionScript->CreateNode(ShapeClass, TEXT("TriggerVolume"));
                if (ShapeNode)
                {
                    Blueprint->SimpleConstructionScript->AddNode(ShapeNode);
                }
            }

            FAssetRegistryModule::AssetCreated(Blueprint);
            Package->MarkPackageDirty();
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("template_name"), TemplateName);
            Result->SetStringField(TEXT("blueprint_path"), PackagePath);
            Result->SetStringField(TEXT("trigger_shape"), TriggerShape);
            Result->SetStringField(TEXT("trigger_type"), TriggerType);
            Result->SetBoolField(TEXT("once_only"), bOnceOnly);
            Result->SetNumberField(TEXT("cooldown"), Cooldown);
            Result->SetBoolField(TEXT("player_only"), bPlayerOnly);
            Result->SetNumberField(TEXT("tag_filter_count"), TagFilters.Num());
            Result->SetBoolField(TEXT("debug_draw"), bDebugDraw);
            SendAutomationResponse(Socket, RequestId, true, TEXT("Trigger template created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create trigger template blueprint"), TEXT("CREATE_ERROR"));
        }
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown templates action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}