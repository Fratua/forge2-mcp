// Forge2WorldGenWater.h
// FORGE2 Ultimate AAA Game Development MCP Extension
// Water Bodies and Environment Population Generation

#pragma once

#include "CoreMinimal.h"
#include "Forge2WorldGenNoise.h"
#include "Forge2WorldGenWater.generated.h"

// Forward declarations
struct FForge2BiomeDefinition;

/**
 * Water body types
 */
UENUM(BlueprintType)
enum class EForge2WaterBodyType : uint8
{
    River       UMETA(DisplayName = "River"),
    Lake        UMETA(DisplayName = "Lake"),
    Ocean       UMETA(DisplayName = "Ocean"),
    Pond        UMETA(DisplayName = "Pond"),
    Stream      UMETA(DisplayName = "Stream"),
    Waterfall   UMETA(DisplayName = "Waterfall"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Marsh       UMETA(DisplayName = "Marsh")
};

/**
 * River flow direction type
 */
UENUM(BlueprintType)
enum class EForge2FlowDirection : uint8
{
    North       UMETA(DisplayName = "North"),
    NorthEast   UMETA(DisplayName = "North East"),
    East        UMETA(DisplayName = "East"),
    SouthEast   UMETA(DisplayName = "South East"),
    South       UMETA(DisplayName = "South"),
    SouthWest   UMETA(DisplayName = "South West"),
    West        UMETA(DisplayName = "West"),
    NorthWest   UMETA(DisplayName = "North West")
};

/**
 * Wave type for ocean/lake
 */
UENUM(BlueprintType)
enum class EForge2WaveType : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Gentle      UMETA(DisplayName = "Gentle"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Rough       UMETA(DisplayName = "Rough"),
    Stormy      UMETA(DisplayName = "Stormy")
};

/**
 * Vegetation density level
 */
UENUM(BlueprintType)
enum class EForge2VegetationDensity : uint8
{
    Sparse      UMETA(DisplayName = "Sparse"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dense       UMETA(DisplayName = "Dense"),
    Lush        UMETA(DisplayName = "Lush")
};

/**
 * Prop placement mode
 */
UENUM(BlueprintType)
enum class EForge2PropPlacementMode : uint8
{
    Random      UMETA(DisplayName = "Random"),
    Grid        UMETA(DisplayName = "Grid"),
    Clustered   UMETA(DisplayName = "Clustered"),
    Poisson     UMETA(DisplayName = "Poisson Disk"),
    AlongPath   UMETA(DisplayName = "Along Path"),
    AroundPoint UMETA(DisplayName = "Around Point")
};

/**
 * Decal type
 */
UENUM(BlueprintType)
enum class EForge2DecalType : uint8
{
    Ground      UMETA(DisplayName = "Ground"),
    Path        UMETA(DisplayName = "Path"),
    Puddle      UMETA(DisplayName = "Puddle"),
    Crack       UMETA(DisplayName = "Crack"),
    Moss        UMETA(DisplayName = "Moss"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Blood       UMETA(DisplayName = "Blood"),
    Scorch      UMETA(DisplayName = "Scorch"),
    Custom      UMETA(DisplayName = "Custom")
};

// ============================================================================
// Water Body Structures
// ============================================================================

/**
 * River spline point data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RiverPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FlowDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BankHeight;

    FForge2RiverPoint()
        : Location(FVector::ZeroVector)
        , Width(100.0f)
        , Depth(50.0f)
        , FlowSpeed(1.0f)
        , FlowDirection(FVector::ForwardVector)
        , BankHeight(20.0f)
    {}
};

/**
 * River data structure
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RiverData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2RiverPoint> SplinePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MouthLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> TributaryIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasWaterfall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WaterfallLocations;

    FForge2RiverData()
        : TotalLength(0.0f)
        , AverageWidth(100.0f)
        , AverageDepth(50.0f)
        , SourceLocation(FVector::ZeroVector)
        , MouthLocation(FVector::ZeroVector)
        , bHasWaterfall(false)
    {}
};

/**
 * Shoreline point for lakes
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ShorelinePoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BeachWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterDepthAtShore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCliff;

    FForge2ShorelinePoint()
        : Location(FVector::ZeroVector)
        , Normal(FVector::UpVector)
        , BeachWidth(50.0f)
        , WaterDepthAtShore(0.0f)
        , bIsCliff(false)
    {}
};

/**
 * Lake data structure
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SurfaceArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2ShorelinePoint> Shoreline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> ShorelinePolygon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> IslandLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InletRiverIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OutletRiverIndex;

    FForge2LakeData()
        : Center(FVector::ZeroVector)
        , SurfaceArea(0.0f)
        , MaxDepth(100.0f)
        , WaterLevel(0.0f)
        , InletRiverIndex(-1)
        , OutletRiverIndex(-1)
    {}
};

/**
 * Ocean data structure
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2OceanData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeaLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D BoundsMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D BoundsMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2WaveType WaveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaveAmplitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaveFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WaveDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2ShorelinePoint> Coastline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TideAmplitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TidePeriod;

    FForge2OceanData()
        : SeaLevel(0.0f)
        , BoundsMin(FVector2D::ZeroVector)
        , BoundsMax(FVector2D::ZeroVector)
        , WaveType(EForge2WaveType::Gentle)
        , WaveAmplitude(50.0f)
        , WaveFrequency(0.1f)
        , WaveDirection(FVector::ForwardVector)
        , TideAmplitude(100.0f)
        , TidePeriod(720.0f) // 12 minutes
    {}
};

/**
 * Water flow cell for simulation
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WaterFlowCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D FlowVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sediment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerrainHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSink;

    FForge2WaterFlowCell()
        : WaterHeight(0.0f)
        , FlowVelocity(FVector2D::ZeroVector)
        , Sediment(0.0f)
        , TerrainHeight(0.0f)
        , bIsSource(false)
        , bIsSink(false)
    {}
};

// ============================================================================
// Water Generation Parameters
// ============================================================================

/**
 * River generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RiverParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Meandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerrainCarveDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BankSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFollowTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCreateWaterfalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterfallThreshold;

    FForge2RiverParams()
        : SourceLocation(FVector::ZeroVector)
        , TargetLocation(FVector::ZeroVector)
        , StartWidth(50.0f)
        , EndWidth(200.0f)
        , StartDepth(20.0f)
        , EndDepth(80.0f)
        , Meandering(0.5f)
        , TerrainCarveDepth(30.0f)
        , BankSlope(45.0f)
        , Seed(0)
        , bFollowTerrain(true)
        , bCreateWaterfalls(true)
        , WaterfallThreshold(100.0f)
    {}
};

/**
 * Lake generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LakeParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShorelineIrregularity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BeachWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CliffChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 IslandCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IslandSizeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShorelineResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FForge2LakeParams()
        : Center(FVector::ZeroVector)
        , Radius(500.0f)
        , MaxDepth(100.0f)
        , ShorelineIrregularity(0.3f)
        , BeachWidth(50.0f)
        , CliffChance(0.1f)
        , IslandCount(0)
        , IslandSizeRange(100.0f)
        , ShorelineResolution(64)
        , Seed(0)
    {}
};

/**
 * Ocean generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2OceanParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeaLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D BoundsMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D BoundsMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2WaveType WaveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WaveDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoastlineResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateCoastline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TideAmplitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FForge2OceanParams()
        : SeaLevel(0.0f)
        , BoundsMin(FVector2D::ZeroVector)
        , BoundsMax(FVector2D(10000.0f, 10000.0f))
        , WaveType(EForge2WaveType::Gentle)
        , WaveDirection(FVector::ForwardVector)
        , CoastlineResolution(100.0f)
        , bGenerateCoastline(true)
        , TideAmplitude(100.0f)
        , Seed(0)
    {}
};

/**
 * Water flow simulation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WaterFlowParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SimulationSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Gravity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Viscosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EvaporationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RainfallRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WaterSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FForge2WaterFlowParams()
        : GridResolution(256)
        , CellSize(100.0f)
        , SimulationSteps(100)
        , TimeStep(0.1f)
        , Gravity(9.81f)
        , Viscosity(0.1f)
        , EvaporationRate(0.01f)
        , RainfallRate(0.0f)
        , Seed(0)
    {}
};

// ============================================================================
// Environment Population Structures
// ============================================================================

/**
 * Vegetation instance data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VegetationInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform Transform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MeshVariant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor TintColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCastShadow;

    FForge2VegetationInstance()
        : MeshVariant(0)
        , Scale(1.0f)
        , TintColor(FLinearColor::White)
        , bCastShadow(true)
    {}
};

/**
 * Prop instance data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PropInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform Transform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInteractable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasCollision;

    FForge2PropInstance()
        : bIsInteractable(false)
        , bHasCollision(true)
    {}
};

/**
 * Rock instance data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RockInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform Transform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmbedDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MossAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasPhysics;

    FForge2RockInstance()
        : EmbedDepth(0.0f)
        , MossAmount(0.0f)
        , bHasPhysics(false)
    {}
};

/**
 * Decal instance data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DecalInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform Transform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MaterialPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2DecalType DecalType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SortOrder;

    FForge2DecalInstance()
        : DecalType(EForge2DecalType::Ground)
        , Size(FVector2D(100.0f, 100.0f))
        , FadeDistance(1000.0f)
        , SortOrder(0)
    {}
};

/**
 * Environment population data (combined output)
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2EnvironmentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2VegetationInstance> Vegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PropInstance> Props;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2RockInstance> Rocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2DecalInstance> Decals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBox Bounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalInstances;

    FForge2EnvironmentData()
        : TotalInstances(0)
    {}

    void CalculateTotals()
    {
        TotalInstances = Vegetation.Num() + Props.Num() + Rocks.Num() + Decals.Num();
    }
};

// ============================================================================
// Environment Population Parameters
// ============================================================================

/**
 * Vegetation placement parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VegetationParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBox Bounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2VegetationDensity Density;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> MeshPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> MeshWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ScaleRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSpacing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAlignToSurface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRandomRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCluster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ClusterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FForge2VegetationParams()
        : Density(EForge2VegetationDensity::Medium)
        , ScaleRange(FVector2D(0.8f, 1.2f))
        , MinSlope(0.0f)
        , MaxSlope(45.0f)
        , MinElevation(-10000.0f)
        , MaxElevation(10000.0f)
        , MinSpacing(50.0f)
        , bAlignToSurface(true)
        , bRandomRotation(true)
        , bCluster(false)
        , ClusterRadius(200.0f)
        , ClusterCount(5)
        , Seed(0)
    {}
};

/**
 * Prop scattering parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PropScatterParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBox Bounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PropPlacementMode PlacementMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> MeshPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> MeshWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InstanceCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ScaleRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRandomRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAlignToSurface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSpacing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> ExclusionPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExclusionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FForge2PropScatterParams()
        : PlacementMode(EForge2PropPlacementMode::Random)
        , InstanceCount(100)
        , Density(0.01f)
        , ScaleRange(FVector2D(0.8f, 1.2f))
        , bRandomRotation(true)
        , bAlignToSurface(true)
        , MinSpacing(100.0f)
        , ExclusionRadius(50.0f)
        , Seed(0)
    {}
};

/**
 * Rock generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RockParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBox Bounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> MeshPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InstanceCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ScaleRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmbedDepthRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MossChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClusterChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ClusterSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPreferSlopes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FForge2RockParams()
        : InstanceCount(50)
        , ScaleRange(FVector2D(0.5f, 2.0f))
        , EmbedDepthRange(30.0f)
        , MossChance(0.3f)
        , ClusterChance(0.4f)
        , ClusterSize(5)
        , ClusterRadius(150.0f)
        , bPreferSlopes(true)
        , MinSlope(15.0f)
        , Seed(0)
    {}
};

/**
 * Decal placement parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DecalParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBox Bounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2DecalType DecalType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> MaterialPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InstanceCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D SizeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRandomRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSpacing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PathWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FForge2DecalParams()
        : DecalType(EForge2DecalType::Ground)
        , InstanceCount(20)
        , SizeRange(FVector2D(50.0f, 150.0f))
        , bRandomRotation(true)
        , MinSpacing(100.0f)
        , PathWidth(200.0f)
        , Seed(0)
    {}
};

// ============================================================================
// Water Generation Classes
// ============================================================================

/**
 * River generator using A* pathfinding and spline interpolation
 */
class MCPAUTOMATIONBRIDGE_API FForge2RiverGenerator
{
public:
    FForge2RiverGenerator();

    /** Generate a river from source to target */
    FForge2RiverData Generate(const FForge2RiverParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

    /** Generate river following terrain gradient (downhill) */
    FForge2RiverData GenerateFromGradient(const FForge2RiverParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

    /** Calculate terrain carving for river bed */
    TArray<float> CalculateTerrainCarving(const FForge2RiverData& River, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize, float CarveDepth);

private:
    /** Find path from source to target using A* on heightmap */
    TArray<FIntPoint> FindRiverPath(FIntPoint Start, FIntPoint End, const TArray<float>& HeightMap, int32 Width, int32 Height, bool bPreferDownhill);

    /** Apply meandering to path */
    TArray<FVector> ApplyMeandering(const TArray<FVector>& Path, float Amount, int32 Seed);

    /** Interpolate path using Catmull-Rom splines */
    TArray<FForge2RiverPoint> InterpolatePath(const TArray<FVector>& Path, const FForge2RiverParams& Params);

    /** Detect waterfall locations */
    TArray<FVector> DetectWaterfalls(const TArray<FForge2RiverPoint>& Points, float Threshold);

    FForge2PerlinNoise NoiseGenerator;
};

/**
 * Lake generator with irregular shorelines
 */
class MCPAUTOMATIONBRIDGE_API FForge2LakeGenerator
{
public:
    FForge2LakeGenerator();

    /** Generate a lake at specified location */
    FForge2LakeData Generate(const FForge2LakeParams& Params);

    /** Generate lake from terrain depression */
    FForge2LakeData GenerateFromDepression(FVector Center, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize, float WaterLevel);

    /** Calculate depth map for lake */
    TArray<float> CalculateDepthMap(const FForge2LakeData& Lake, int32 Resolution);

private:
    /** Generate irregular shoreline using noise */
    TArray<FForge2ShorelinePoint> GenerateShoreline(FVector Center, float Radius, float Irregularity, int32 Resolution, int32 Seed);

    /** Generate islands within lake */
    TArray<FVector> GenerateIslands(const FForge2LakeData& Lake, int32 Count, float SizeRange, int32 Seed);

    /** Determine if point is cliff or beach */
    bool IsCliffLocation(FVector Location, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize, float Threshold);

    FForge2PerlinNoise NoiseGenerator;
};

/**
 * Ocean generator with waves and coastline
 */
class MCPAUTOMATIONBRIDGE_API FForge2OceanGenerator
{
public:
    FForge2OceanGenerator();

    /** Generate ocean plane with wave settings */
    FForge2OceanData Generate(const FForge2OceanParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

    /** Calculate wave height at position and time */
    float GetWaveHeight(FVector2D Position, float Time, const FForge2OceanData& Ocean) const;

    /** Calculate tide level at time */
    float GetTideLevel(float Time, const FForge2OceanData& Ocean) const;

private:
    /** Generate coastline from heightmap */
    TArray<FForge2ShorelinePoint> GenerateCoastline(const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize, float SeaLevel, float Resolution);

    /** Configure wave parameters based on type */
    void ConfigureWaves(FForge2OceanData& Ocean, EForge2WaveType WaveType);

    FForge2PerlinNoise NoiseGenerator;
};

/**
 * Water flow simulation for river placement
 */
class MCPAUTOMATIONBRIDGE_API FForge2WaterFlowSimulator
{
public:
    FForge2WaterFlowSimulator();

    /** Simulate water flow on heightmap */
    TArray<FForge2WaterFlowCell> Simulate(const FForge2WaterFlowParams& Params, const TArray<float>& HeightMap);

    /** Get flow direction map */
    TArray<FVector2D> GetFlowDirections() const;

    /** Get accumulated water depth map */
    TArray<float> GetWaterAccumulation() const;

    /** Find optimal river paths from simulation */
    TArray<FForge2RiverData> FindRiverPaths(float MinAccumulation, int32 MaxRivers);

private:
    /** Initialize simulation grid */
    void InitializeGrid(const TArray<float>& HeightMap, const FForge2WaterFlowParams& Params);

    /** Single simulation step */
    void SimulationStep(float DeltaTime);

    /** Calculate outflow from cell */
    FVector2D CalculateOutflow(int32 X, int32 Y);

    /** Apply evaporation */
    void ApplyEvaporation(float Rate);

    TArray<FForge2WaterFlowCell> Grid;
    int32 GridWidth;
    int32 GridHeight;
    float CellSize;
    FForge2WaterFlowParams CurrentParams;
};

// ============================================================================
// Environment Population Classes
// ============================================================================

/**
 * Poisson disk sampling for even distribution
 */
class MCPAUTOMATIONBRIDGE_API FForge2PoissonDiskSampler
{
public:
    FForge2PoissonDiskSampler();

    /** Generate points with minimum spacing */
    TArray<FVector2D> Generate(FVector2D Min, FVector2D Max, float MinDistance, int32 MaxAttempts, int32 Seed);

    /** Generate points in 3D volume */
    TArray<FVector> Generate3D(FBox Bounds, float MinDistance, int32 MaxAttempts, int32 Seed);

private:
    /** Check if point is valid (meets minimum distance) */
    bool IsValidPoint(FVector2D Point, const TArray<FVector2D>& Points, float MinDistance, TMap<FIntPoint, TArray<int32>>& Grid, float CellSize);

    /** Get grid cell for point */
    FIntPoint GetGridCell(FVector2D Point, FVector2D Min, float CellSize);

    FRandomStream RandomStream;
};

/**
 * Vegetation placement system
 */
class MCPAUTOMATIONBRIDGE_API FForge2VegetationPlacer
{
public:
    FForge2VegetationPlacer();

    /** Place vegetation in area */
    TArray<FForge2VegetationInstance> Place(const FForge2VegetationParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

    /** Place biome-aware vegetation */
    TArray<FForge2VegetationInstance> PlaceWithBiome(const FForge2VegetationParams& Params, const TArray<float>& HeightMap, const TArray<int32>& BiomeMap, const TArray<struct FForge2BiomeDefinition>& Biomes, int32 MapWidth, int32 MapHeight, float CellSize);

private:
    /** Get density multiplier for vegetation level */
    float GetDensityMultiplier(EForge2VegetationDensity Density);

    /** Calculate slope at point */
    float CalculateSlope(int32 X, int32 Y, const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize);

    /** Select mesh based on weights */
    int32 SelectMesh(const TArray<float>& Weights, float Random);

    /** Generate cluster around point */
    TArray<FVector> GenerateCluster(FVector Center, float Radius, int32 Count, int32 Seed);

    FForge2PoissonDiskSampler PoissonSampler;
    FRandomStream RandomStream;
};

/**
 * Prop scattering system
 */
class MCPAUTOMATIONBRIDGE_API FForge2PropScatterer
{
public:
    FForge2PropScatterer();

    /** Scatter props in area */
    TArray<FForge2PropInstance> Scatter(const FForge2PropScatterParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

    /** Scatter props along path */
    TArray<FForge2PropInstance> ScatterAlongPath(const TArray<FVector>& PathPoints, float PathWidth, const FForge2PropScatterParams& Params);

    /** Scatter props around point */
    TArray<FForge2PropInstance> ScatterAroundPoint(FVector Center, float Radius, const FForge2PropScatterParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

private:
    /** Check if point is in exclusion zone */
    bool IsInExclusionZone(FVector Point, const TArray<FVector>& ExclusionPoints, float ExclusionRadius);

    /** Generate grid placement */
    TArray<FVector> GenerateGridPlacement(FBox Bounds, float Spacing);

    /** Generate clustered placement */
    TArray<FVector> GenerateClusteredPlacement(FBox Bounds, int32 Count, int32 ClusterCount, float ClusterRadius, int32 Seed);

    FForge2PoissonDiskSampler PoissonSampler;
    FRandomStream RandomStream;
};

/**
 * Rock and debris generator
 */
class MCPAUTOMATIONBRIDGE_API FForge2RockGenerator
{
public:
    FForge2RockGenerator();

    /** Generate rocks in area */
    TArray<FForge2RockInstance> Generate(const FForge2RockParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

    /** Generate rock formations (clusters) */
    TArray<FForge2RockInstance> GenerateFormation(FVector Center, const FForge2RockParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

private:
    /** Calculate embed depth based on slope */
    float CalculateEmbedDepth(FVector Location, const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize, float MaxDepth);

    /** Determine moss amount based on location */
    float CalculateMossAmount(FVector Location, float Chance, int32 Seed);

    FRandomStream RandomStream;
};

/**
 * Decal placement system
 */
class MCPAUTOMATIONBRIDGE_API FForge2DecalPlacer
{
public:
    FForge2DecalPlacer();

    /** Place decals in area */
    TArray<FForge2DecalInstance> Place(const FForge2DecalParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize);

    /** Place decals along path */
    TArray<FForge2DecalInstance> PlaceAlongPath(const TArray<FVector>& PathPoints, float PathWidth, const FForge2DecalParams& Params);

private:
    /** Get surface normal at point */
    FVector GetSurfaceNormal(FVector Location, const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize);

    FRandomStream RandomStream;
};

// ============================================================================
// Utility Classes
// ============================================================================

/**
 * Water and environment processor utilities
 */
class MCPAUTOMATIONBRIDGE_API FForge2WaterEnvironmentUtils
{
public:
    /** Combine multiple environment data sets */
    static FForge2EnvironmentData CombineEnvironmentData(const TArray<FForge2EnvironmentData>& DataSets);

    /** Filter instances by bounds */
    static FForge2EnvironmentData FilterByBounds(const FForge2EnvironmentData& Data, FBox Bounds);

    /** Calculate water surface area */
    static float CalculateWaterSurfaceArea(const FForge2LakeData& Lake);
    static float CalculateWaterSurfaceArea(const FForge2RiverData& River);

    /** Check if point is underwater */
    static bool IsUnderwater(FVector Point, const TArray<FForge2LakeData>& Lakes, const TArray<FForge2RiverData>& Rivers, const FForge2OceanData* Ocean);

    /** Get water depth at point */
    static float GetWaterDepth(FVector Point, const TArray<FForge2LakeData>& Lakes, const TArray<FForge2RiverData>& Rivers, const FForge2OceanData* Ocean);

    /** Convert flow direction enum to vector */
    static FVector FlowDirectionToVector(EForge2FlowDirection Direction);

    /** Get nearest water body */
    static float GetDistanceToWater(FVector Point, const TArray<FForge2LakeData>& Lakes, const TArray<FForge2RiverData>& Rivers, const FForge2OceanData* Ocean);
};

#endif // Include guard handled by #pragma once
