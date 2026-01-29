// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// World Generation Erosion Algorithms

#pragma once

#include "CoreMinimal.h"
#include "Forge2WorldGenErosion.generated.h"

/**
 * Thermal erosion parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ThermalErosionParams
{
    GENERATED_BODY()

    /** Number of erosion iterations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "1000"))
    int32 Iterations = 50;

    /** Talus angle in degrees (material moves when slope exceeds this) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float TalusAngle = 40.0f;

    /** Amount of material transferred per iteration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ErosionStrength = 0.5f;

    /** Cell size for angle calculation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.1"))
    float CellSize = 1.0f;
};

/**
 * Hydraulic erosion parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HydraulicErosionParams
{
    GENERATED_BODY()

    /** Number of water droplet simulations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "1000000"))
    int32 Iterations = 10000;

    /** Random seed for droplet placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    int32 Seed = 0;

    /** Maximum steps a droplet can take before evaporating */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "256"))
    int32 MaxDropletLifetime = 64;

    /** Inertia - higher means droplet resists changing direction (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Inertia = 0.05f;

    /** How much sediment a droplet can carry relative to its speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float SedimentCapacityFactor = 4.0f;

    /** Minimum sediment capacity (prevents excessive deposition on flat terrain) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0"))
    float MinSedimentCapacity = 0.01f;

    /** Rate at which sediment is picked up */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ErodeSpeed = 0.3f;

    /** Rate at which sediment is deposited */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DepositSpeed = 0.3f;

    /** Rate at which water evaporates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EvaporateSpeed = 0.01f;

    /** Gravity affecting droplet speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Gravity = 4.0f;

    /** Initial water volume of each droplet */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float InitialWaterVolume = 1.0f;

    /** Initial speed of each droplet */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float InitialSpeed = 1.0f;

    /** Radius of erosion effect around droplet */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "8"))
    int32 ErosionRadius = 3;
};

/**
 * Thermal Erosion Simulator
 * Simulates slope-based erosion where material falls down steep slopes
 */
class MCPAUTOMATIONBRIDGE_API FForge2ThermalErosion
{
public:
    FForge2ThermalErosion() = default;

    /**
     * Apply thermal erosion to a heightmap
     * @param Heightmap - The heightmap data (modified in place)
     * @param Width - Width of the heightmap
     * @param Height - Height of the heightmap
     * @param Params - Erosion parameters
     */
    void Apply(TArray<float>& Heightmap, int32 Width, int32 Height, const FForge2ThermalErosionParams& Params);

    /**
     * Perform a single iteration of thermal erosion
     * @return Amount of material moved (for convergence detection)
     */
    float ApplyIteration(TArray<float>& Heightmap, int32 Width, int32 Height, const FForge2ThermalErosionParams& Params);

private:
    /** Calculate slope to neighbor */
    float GetSlope(const TArray<float>& Heightmap, int32 Width, int32 Height,
                   int32 X, int32 Y, int32 Dx, int32 Dy, float CellSize) const;

    /** Check if coordinates are valid */
    bool IsValidCoord(int32 X, int32 Y, int32 Width, int32 Height) const;

    /** Direction offsets for 8 neighbors */
    static constexpr int32 DirX[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static constexpr int32 DirY[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static constexpr float DirDist[8] = {1.414f, 1.0f, 1.414f, 1.0f, 1.0f, 1.414f, 1.0f, 1.414f};
};

/**
 * Hydraulic Erosion Simulator
 * Simulates water droplets carving terrain as they flow downhill
 */
class MCPAUTOMATIONBRIDGE_API FForge2HydraulicErosion
{
public:
    FForge2HydraulicErosion() = default;

    /**
     * Apply hydraulic erosion to a heightmap
     * @param Heightmap - The heightmap data (modified in place)
     * @param Width - Width of the heightmap
     * @param Height - Height of the heightmap
     * @param Params - Erosion parameters
     */
    void Apply(TArray<float>& Heightmap, int32 Width, int32 Height, const FForge2HydraulicErosionParams& Params);

    /**
     * Simulate a single droplet
     */
    void SimulateDroplet(TArray<float>& Heightmap, int32 Width, int32 Height,
                         const FForge2HydraulicErosionParams& Params, FRandomStream& Random);

    /** Get the erosion brush weights for a given radius (cached) */
    const TArray<float>& GetErosionBrush(int32 Radius);

    /** Get the erosion brush indices for a given radius (cached) */
    const TArray<FIntPoint>& GetErosionBrushIndices(int32 Radius);

private:
    /**
     * Calculate gradient and height at a position using bilinear interpolation
     * @param HeightAndGradient - Output: X = height, Y = gradX, Z = gradY
     */
    FVector CalculateHeightAndGradient(const TArray<float>& Heightmap, int32 Width, int32 Height,
                                        float PosX, float PosY) const;

    /** Check if coordinates are valid */
    bool IsValidCoord(int32 X, int32 Y, int32 Width, int32 Height) const;

    /** Bilinear interpolation helper */
    float SampleHeight(const TArray<float>& Heightmap, int32 Width, int32 Height, float X, float Y) const;

    /** Cached erosion brushes by radius */
    TMap<int32, TArray<float>> ErosionBrushWeights;
    TMap<int32, TArray<FIntPoint>> ErosionBrushIndices;

    /** Initialize erosion brush for given radius */
    void InitializeErosionBrush(int32 Radius);
};

/**
 * Wind Erosion Simulator
 * Simulates wind carrying particles and depositing them elsewhere
 */
class MCPAUTOMATIONBRIDGE_API FForge2WindErosion
{
public:
    /**
     * Wind erosion parameters
     */
    struct FParams
    {
        int32 Iterations = 1000;
        FVector2D WindDirection = FVector2D(1.0f, 0.0f);
        float WindStrength = 1.0f;
        float SuspensionRate = 0.1f;
        float AbrasionRate = 0.05f;
        float SettlingRate = 0.3f;
        float Gravity = 0.5f;
        int32 Seed = 0;
    };

    void Apply(TArray<float>& Heightmap, int32 Width, int32 Height, const FParams& Params);

private:
    void SimulateParticle(TArray<float>& Heightmap, int32 Width, int32 Height,
                          const FParams& Params, FRandomStream& Random);
};

/**
 * Combined erosion processor
 * Applies multiple erosion types in sequence for realistic terrain
 */
class MCPAUTOMATIONBRIDGE_API FForge2ErosionProcessor
{
public:
    /**
     * Apply a complete erosion pass
     * @param Heightmap - Heightmap to erode
     * @param Width - Width
     * @param Height - Height
     * @param ThermalParams - Thermal erosion parameters (optional, nullptr to skip)
     * @param HydraulicParams - Hydraulic erosion parameters (optional, nullptr to skip)
     * @param WindParams - Wind erosion parameters (optional, nullptr to skip)
     */
    void ApplyErosion(TArray<float>& Heightmap, int32 Width, int32 Height,
                      const FForge2ThermalErosionParams* ThermalParams = nullptr,
                      const FForge2HydraulicErosionParams* HydraulicParams = nullptr,
                      const FForge2WindErosion::FParams* WindParams = nullptr);

    /**
     * Apply realistic erosion preset
     * Uses balanced parameters for natural-looking terrain
     */
    void ApplyRealisticErosion(TArray<float>& Heightmap, int32 Width, int32 Height,
                                int32 Seed = 0, float Intensity = 1.0f);

    /**
     * Apply quick erosion for preview
     * Faster but lower quality
     */
    void ApplyQuickErosion(TArray<float>& Heightmap, int32 Width, int32 Height,
                           int32 Seed = 0, float Intensity = 1.0f);

private:
    FForge2ThermalErosion ThermalErosion;
    FForge2HydraulicErosion HydraulicErosion;
    FForge2WindErosion WindErosion;
};

/**
 * Flow map generator
 * Creates flow direction maps for rivers and water effects
 */
class MCPAUTOMATIONBRIDGE_API FForge2FlowMapGenerator
{
public:
    /**
     * Generate a flow direction map from a heightmap
     * @param Heightmap - Input heightmap
     * @param Width - Width
     * @param Height - Height
     * @return Array of flow directions (FVector2D per cell)
     */
    TArray<FVector2D> GenerateFlowMap(const TArray<float>& Heightmap, int32 Width, int32 Height);

    /**
     * Generate flow accumulation map (how much water flows through each cell)
     * Higher values indicate river channels
     */
    TArray<float> GenerateFlowAccumulation(const TArray<float>& Heightmap, int32 Width, int32 Height);

    /**
     * Find river paths based on flow accumulation
     * @param FlowAccumulation - Flow accumulation map
     * @param Width - Width
     * @param Height - Height
     * @param Threshold - Minimum flow for river classification
     * @return List of river polylines (each river is a list of points)
     */
    TArray<TArray<FVector2D>> FindRiverPaths(const TArray<float>& FlowAccumulation,
                                              int32 Width, int32 Height, float Threshold);

private:
    /** Get steepest descent direction */
    FVector2D GetFlowDirection(const TArray<float>& Heightmap, int32 Width, int32 Height, int32 X, int32 Y);

    /** Follow flow to count accumulation */
    void TraceFlow(const TArray<FVector2D>& FlowMap, TArray<float>& Accumulation,
                   int32 Width, int32 Height, int32 StartX, int32 StartY);
};
