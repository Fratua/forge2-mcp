// Forge2WorldGenBiome.h
// FORGE2 Ultimate AAA Game Development MCP Extension
// Biome System with Climate Modeling and Blending

#pragma once

#include "CoreMinimal.h"
#include "Forge2WorldGenNoise.h"
#include "Forge2WorldGenBiome.generated.h"

/**
 * Biome type enumeration
 */
UENUM(BlueprintType)
enum class EForge2BiomeType : uint8
{
    // Cold biomes
    IceCap              UMETA(DisplayName = "Ice Cap"),
    Tundra              UMETA(DisplayName = "Tundra"),
    BorealForest        UMETA(DisplayName = "Boreal Forest/Taiga"),

    // Temperate biomes
    TemperateRainforest UMETA(DisplayName = "Temperate Rainforest"),
    TemperateForest     UMETA(DisplayName = "Temperate Forest"),
    TemperateGrassland  UMETA(DisplayName = "Temperate Grassland"),

    // Warm biomes
    TropicalRainforest  UMETA(DisplayName = "Tropical Rainforest"),
    TropicalForest      UMETA(DisplayName = "Tropical Forest"),
    Savanna             UMETA(DisplayName = "Savanna"),

    // Dry biomes
    Desert              UMETA(DisplayName = "Desert"),
    SemiAridDesert      UMETA(DisplayName = "Semi-Arid Desert"),
    Shrubland           UMETA(DisplayName = "Shrubland"),

    // Special biomes
    Mountain            UMETA(DisplayName = "Mountain"),
    SnowMountain        UMETA(DisplayName = "Snow Mountain"),
    Swamp               UMETA(DisplayName = "Swamp"),
    Marsh               UMETA(DisplayName = "Marsh"),
    Beach               UMETA(DisplayName = "Beach"),
    Ocean               UMETA(DisplayName = "Ocean"),
    River               UMETA(DisplayName = "River"),
    Lake                UMETA(DisplayName = "Lake"),

    // Fantasy biomes
    MagicForest         UMETA(DisplayName = "Magic Forest"),
    CrystalCaverns      UMETA(DisplayName = "Crystal Caverns"),
    VolcanicWasteland   UMETA(DisplayName = "Volcanic Wasteland"),
    FrozenWasteland     UMETA(DisplayName = "Frozen Wasteland"),
    CorruptedLand       UMETA(DisplayName = "Corrupted Land"),

    Custom              UMETA(DisplayName = "Custom")
};

/**
 * Vegetation layer type
 */
UENUM(BlueprintType)
enum class EForge2VegetationLayer : uint8
{
    Canopy      UMETA(DisplayName = "Canopy (Trees)"),
    Understory  UMETA(DisplayName = "Understory (Shrubs)"),
    GroundCover UMETA(DisplayName = "Ground Cover"),
    Emergent    UMETA(DisplayName = "Emergent (Tall Trees)"),
    Aquatic     UMETA(DisplayName = "Aquatic Plants")
};

/**
 * Climate zone
 */
UENUM(BlueprintType)
enum class EForge2ClimateZone : uint8
{
    Arctic      UMETA(DisplayName = "Arctic"),
    Subarctic   UMETA(DisplayName = "Subarctic"),
    Temperate   UMETA(DisplayName = "Temperate"),
    Subtropical UMETA(DisplayName = "Subtropical"),
    Tropical    UMETA(DisplayName = "Tropical")
};

// ============================================================================
// Biome Definition Structures
// ============================================================================

/**
 * Vegetation type definition for biome
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BiomeVegetation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2VegetationLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density;

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
    float ClusterChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ClusterSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FLinearColor> TintVariations;

    FForge2BiomeVegetation()
        : Layer(EForge2VegetationLayer::GroundCover)
        , Density(0.1f)
        , ScaleRange(FVector2D(0.8f, 1.2f))
        , MinSlope(0.0f)
        , MaxSlope(45.0f)
        , MinElevation(-10000.0f)
        , MaxElevation(10000.0f)
        , ClusterChance(0.0f)
        , ClusterSize(3)
    {}
};

/**
 * Ambient sound definition for biome
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BiomeAmbientSound
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SoundPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDaytime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNighttime;

    FForge2BiomeAmbientSound()
        : Volume(1.0f)
        , PlayChance(1.0f)
        , bIsLooping(true)
        , bIsDaytime(true)
        , bIsNighttime(true)
    {}
};

/**
 * Wildlife definition for biome
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BiomeWildlife
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CreatureBlueprintPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinGroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxGroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPredator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNocturnal;

    FForge2BiomeWildlife()
        : SpawnDensity(0.01f)
        , MinGroupSize(1)
        , MaxGroupSize(5)
        , bIsPredator(false)
        , bIsNocturnal(false)
    {}
};

/**
 * Complete biome definition
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BiomeDefinition
{
    GENERATED_BODY()

    // Identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BiomeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2BiomeType BiomeType;

    // Climate ranges (what climate produces this biome)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinMoisture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxMoisture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxElevation;

    // Visual properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GroundMaterialPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CliffMaterialPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyTint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity;

    // Vegetation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2BiomeVegetation> Vegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VegetationDensityMultiplier;

    // Props and debris
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PropMeshPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> PropWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PropDensity;

    // Rocks
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RockMeshPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RockDensity;

    // Ambient sounds
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2BiomeAmbientSound> AmbientSounds;

    // Wildlife
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2BiomeWildlife> Wildlife;

    // Weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RainChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SnowChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StormChance;

    // Blend weight when adjacent to other biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendWeight;

    // Priority for overlap resolution (higher = wins)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    FForge2BiomeDefinition()
        : BiomeType(EForge2BiomeType::TemperateForest)
        , MinTemperature(-50.0f)
        , MaxTemperature(50.0f)
        , MinMoisture(0.0f)
        , MaxMoisture(1.0f)
        , MinElevation(-1000.0f)
        , MaxElevation(10000.0f)
        , FogColor(FLinearColor(0.5f, 0.5f, 0.6f))
        , FogDensity(0.001f)
        , SkyTint(FLinearColor::White)
        , SunIntensity(1.0f)
        , VegetationDensityMultiplier(1.0f)
        , PropDensity(0.01f)
        , RockDensity(0.005f)
        , RainChance(0.2f)
        , SnowChance(0.0f)
        , FogChance(0.1f)
        , StormChance(0.05f)
        , BlendWeight(1.0f)
        , Priority(0)
    {}
};

/**
 * Climate data at a point
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ClimateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Moisture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToWater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Latitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Slope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2ClimateZone ClimateZone;

    FForge2ClimateData()
        : Temperature(20.0f)
        , Moisture(0.5f)
        , Elevation(100.0f)
        , DistanceToWater(1000.0f)
        , Latitude(0.0f)
        , Slope(0.0f)
        , ClimateZone(EForge2ClimateZone::Temperate)
    {}
};

/**
 * Biome blend data (when at transition between biomes)
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BiomeBlend
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PrimaryBiomeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SecondaryBiomeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> NearbyBiomeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> NearbyBiomeWeights;

    FForge2BiomeBlend()
        : PrimaryBiomeIndex(-1)
        , SecondaryBiomeIndex(-1)
        , BlendFactor(0.0f)
    {}
};

/**
 * Biome map data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BiomeMapData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> BiomeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2BiomeBlend> BlendData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2ClimateData> ClimateData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Origin;

    FForge2BiomeMapData()
        : Width(0)
        , Height(0)
        , CellSize(100.0f)
        , Origin(FVector2D::ZeroVector)
    {}

    int32 GetIndex(int32 X, int32 Y) const
    {
        if (X < 0 || X >= Width || Y < 0 || Y >= Height)
            return -1;
        return Y * Width + X;
    }

    FIntPoint GetCell(FVector2D WorldPos) const
    {
        return FIntPoint(
            FMath::FloorToInt((WorldPos.X - Origin.X) / CellSize),
            FMath::FloorToInt((WorldPos.Y - Origin.Y) / CellSize)
        );
    }
};

// ============================================================================
// Climate Modeling Parameters
// ============================================================================

/**
 * Climate modeling parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ClimateParams
{
    GENERATED_BODY()

    // World parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WorldSizeY;  // For latitude calculation

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EquatorY;  // Y position of equator

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeaLevel;

    // Temperature parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseTemperatureAtEquator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TemperatureDropPerLatitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TemperatureDropPerMeterElevation;

    // Moisture parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseMoistureNearWater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoistureDecayDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoistureFromRain;

    // Wind/weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D PrevailingWindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindMoistureCarry;

    // Noise for variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TemperatureNoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TemperatureNoiseStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoistureNoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoistureNoiseStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    FForge2ClimateParams()
        : WorldSizeY(10000.0f)
        , EquatorY(5000.0f)
        , SeaLevel(0.0f)
        , BaseTemperatureAtEquator(30.0f)
        , TemperatureDropPerLatitude(0.6f)
        , TemperatureDropPerMeterElevation(0.0065f)
        , BaseMoistureNearWater(1.0f)
        , MoistureDecayDistance(2000.0f)
        , MoistureFromRain(0.5f)
        , PrevailingWindDirection(FVector2D(1.0f, 0.0f))
        , WindMoistureCarry(0.3f)
        , TemperatureNoiseScale(0.001f)
        , TemperatureNoiseStrength(5.0f)
        , MoistureNoiseScale(0.002f)
        , MoistureNoiseStrength(0.2f)
        , Seed(0)
    {}
};

/**
 * Biome map generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BiomeMapParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Origin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2ClimateParams ClimateParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2BiomeDefinition> Biomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateTransitionZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionZoneWidth;

    FForge2BiomeMapParams()
        : Origin(FVector2D::ZeroVector)
        , Size(FVector2D(10000.0f, 10000.0f))
        , CellSize(100.0f)
        , BlendDistance(500.0f)
        , bGenerateTransitionZones(true)
        , TransitionZoneWidth(200.0f)
    {}
};

// ============================================================================
// Climate Modeling Class
// ============================================================================

/**
 * Climate modeling system
 */
class MCPAUTOMATIONBRIDGE_API FForge2ClimateModeler
{
public:
    FForge2ClimateModeler();

    /** Calculate climate data for entire heightmap */
    TArray<FForge2ClimateData> CalculateClimate(
        const TArray<float>& HeightMap,
        int32 Width,
        int32 Height,
        float CellSize,
        const FForge2ClimateParams& Params,
        const TArray<FVector>& WaterBodies  // Centers of water bodies for moisture calculation
    );

    /** Calculate climate at specific point */
    FForge2ClimateData CalculateClimateAtPoint(
        FVector2D Position,
        float Elevation,
        float Slope,
        const FForge2ClimateParams& Params,
        const TArray<FVector>& WaterBodies
    );

    /** Get climate zone from temperature */
    static EForge2ClimateZone GetClimateZone(float Temperature);

private:
    /** Calculate temperature based on latitude and elevation */
    float CalculateTemperature(FVector2D Position, float Elevation, const FForge2ClimateParams& Params);

    /** Calculate moisture based on distance to water and wind */
    float CalculateMoisture(FVector2D Position, float Elevation, const FForge2ClimateParams& Params, const TArray<FVector>& WaterBodies);

    /** Calculate distance to nearest water body */
    float CalculateDistanceToWater(FVector2D Position, const TArray<FVector>& WaterBodies);

    /** Calculate slope at point */
    float CalculateSlope(int32 X, int32 Y, const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize);

    FForge2PerlinNoise NoiseGenerator;
};

// ============================================================================
// Biome Distribution Class
// ============================================================================

/**
 * Biome distribution and lookup system
 */
class MCPAUTOMATIONBRIDGE_API FForge2BiomeDistributor
{
public:
    FForge2BiomeDistributor();

    /** Generate biome map from climate data */
    FForge2BiomeMapData GenerateBiomeMap(
        const TArray<FForge2ClimateData>& ClimateData,
        int32 Width,
        int32 Height,
        float CellSize,
        FVector2D Origin,
        const TArray<FForge2BiomeDefinition>& Biomes
    );

    /** Find best matching biome for climate */
    int32 FindBiomeForClimate(const FForge2ClimateData& Climate, const TArray<FForge2BiomeDefinition>& Biomes);

    /** Calculate biome blend data for cell */
    FForge2BiomeBlend CalculateBlend(
        int32 X,
        int32 Y,
        const TArray<int32>& BiomeIndices,
        int32 Width,
        int32 Height,
        const TArray<FForge2BiomeDefinition>& Biomes,
        float BlendDistance,
        float CellSize
    );

    /** Get biome at world position with blending */
    FForge2BiomeBlend GetBiomeAtPosition(
        FVector2D WorldPos,
        const FForge2BiomeMapData& BiomeMap,
        const TArray<FForge2BiomeDefinition>& Biomes
    );

private:
    /** Calculate match score between climate and biome */
    float CalculateBiomeMatchScore(const FForge2ClimateData& Climate, const FForge2BiomeDefinition& Biome);

    /** Find nearby cells with different biomes */
    TArray<FIntPoint> FindNearbyDifferentBiomes(
        int32 X,
        int32 Y,
        const TArray<int32>& BiomeIndices,
        int32 Width,
        int32 Height,
        int32 SearchRadius
    );
};

// ============================================================================
// Biome Blending Class
// ============================================================================

/**
 * Biome edge blending system
 */
class MCPAUTOMATIONBRIDGE_API FForge2BiomeBlender
{
public:
    FForge2BiomeBlender();

    /** Smooth biome transitions in map */
    void SmoothBiomeEdges(
        FForge2BiomeMapData& BiomeMap,
        const TArray<FForge2BiomeDefinition>& Biomes,
        float BlendDistance
    );

    /** Get blended vegetation parameters for position */
    TArray<FForge2BiomeVegetation> GetBlendedVegetation(
        const FForge2BiomeBlend& Blend,
        const TArray<FForge2BiomeDefinition>& Biomes
    );

    /** Get blended material for position */
    FString GetBlendedMaterialPath(
        const FForge2BiomeBlend& Blend,
        const TArray<FForge2BiomeDefinition>& Biomes
    );

    /** Get blended fog settings */
    void GetBlendedFogSettings(
        const FForge2BiomeBlend& Blend,
        const TArray<FForge2BiomeDefinition>& Biomes,
        FLinearColor& OutFogColor,
        float& OutFogDensity
    );

private:
    /** Interpolate between two biome values */
    template<typename T>
    T InterpolateBiomeValue(T ValueA, T ValueB, float BlendFactor);

    FForge2PerlinNoise NoiseGenerator;
};

// ============================================================================
// Biome Processor (Main Interface)
// ============================================================================

/**
 * Main biome processor combining all systems
 */
class MCPAUTOMATIONBRIDGE_API FForge2BiomeProcessor
{
public:
    FForge2BiomeProcessor();

    /** Generate complete biome map from heightmap */
    FForge2BiomeMapData GenerateFromHeightmap(
        const TArray<float>& HeightMap,
        int32 Width,
        int32 Height,
        float CellSize,
        FVector2D Origin,
        const FForge2BiomeMapParams& Params,
        const TArray<FVector>& WaterBodies
    );

    /** Apply biome to region (modify vegetation, props, etc.) */
    void ApplyBiomeToRegion(
        FForge2BiomeMapData& BiomeMap,
        int32 BiomeIndex,
        FIntRect Region,
        float Strength
    );

    /** Get biome info at world position */
    FForge2BiomeBlend QueryBiomeAtPosition(
        FVector2D WorldPos,
        const FForge2BiomeMapData& BiomeMap,
        const TArray<FForge2BiomeDefinition>& Biomes
    );

    /** Export biome map as color image data (for visualization) */
    TArray<FColor> ExportBiomeMapAsImage(
        const FForge2BiomeMapData& BiomeMap,
        const TArray<FForge2BiomeDefinition>& Biomes
    );

    /** Get default biome definitions */
    static TArray<FForge2BiomeDefinition> GetDefaultBiomes();

    /** Get biome color for visualization */
    static FColor GetBiomeColor(EForge2BiomeType BiomeType);

private:
    FForge2ClimateModeler ClimateModeler;
    FForge2BiomeDistributor BiomeDistributor;
    FForge2BiomeBlender BiomeBlender;
};

// ============================================================================
// Biome Utilities
// ============================================================================

/**
 * Biome utility functions
 */
class MCPAUTOMATIONBRIDGE_API FForge2BiomeUtils
{
public:
    /** Whittaker diagram biome lookup (temperature/precipitation) */
    static EForge2BiomeType WhittakerLookup(float Temperature, float Precipitation);

    /** Get biome display name */
    static FString GetBiomeName(EForge2BiomeType BiomeType);

    /** Check if biome is water-based */
    static bool IsWaterBiome(EForge2BiomeType BiomeType);

    /** Check if biome is cold */
    static bool IsColdBiome(EForge2BiomeType BiomeType);

    /** Check if biome supports trees */
    static bool SupportsTreesbiome(EForge2BiomeType BiomeType);

    /** Get typical vegetation density for biome type */
    static float GetTypicalVegetationDensity(EForge2BiomeType BiomeType);

    /** Get typical rock density for biome type */
    static float GetTypicalRockDensity(EForge2BiomeType BiomeType);

    /** Create biome definition from type with defaults */
    static FForge2BiomeDefinition CreateDefaultBiome(EForge2BiomeType BiomeType);
};

#endif // Include guard handled by #pragma once
