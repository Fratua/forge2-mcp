// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// World Generation Noise Algorithms

#pragma once

#include "CoreMinimal.h"
#include "Forge2WorldGenNoise.generated.h"

/**
 * Noise algorithm types supported by FORGE2
 */
UENUM(BlueprintType)
enum class EForge2NoiseAlgorithm : uint8
{
    Perlin        UMETA(DisplayName = "Perlin Noise"),
    Simplex       UMETA(DisplayName = "Simplex Noise"),
    Voronoi       UMETA(DisplayName = "Voronoi/Worley Noise"),
    FBM           UMETA(DisplayName = "Fractal Brownian Motion"),
    Ridged        UMETA(DisplayName = "Ridged Noise"),
    Billow        UMETA(DisplayName = "Billow Noise"),
    Cellular      UMETA(DisplayName = "Cellular Noise"),
    Value         UMETA(DisplayName = "Value Noise")
};

/**
 * Voronoi distance function types
 */
UENUM(BlueprintType)
enum class EForge2VoronoiDistance : uint8
{
    Euclidean     UMETA(DisplayName = "Euclidean"),
    Manhattan     UMETA(DisplayName = "Manhattan"),
    Chebyshev     UMETA(DisplayName = "Chebyshev"),
    Minkowski     UMETA(DisplayName = "Minkowski")
};

/**
 * Voronoi return type
 */
UENUM(BlueprintType)
enum class EForge2VoronoiReturn : uint8
{
    F1            UMETA(DisplayName = "Nearest Point (F1)"),
    F2            UMETA(DisplayName = "Second Nearest (F2)"),
    F2MinusF1     UMETA(DisplayName = "F2 - F1 (Cell Edges)"),
    CellValue     UMETA(DisplayName = "Cell Value")
};

/**
 * Noise generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NoiseParams
{
    GENERATED_BODY()

    /** Random seed for deterministic generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 Seed = 0;

    /** Scale of the noise (smaller = larger features) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.0001"))
    float Scale = 0.01f;

    /** Number of octaves for fractal noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "1", ClampMax = "16"))
    int32 Octaves = 4;

    /** Persistence (amplitude multiplier per octave) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Persistence = 0.5f;

    /** Lacunarity (frequency multiplier per octave) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "1.0", ClampMax = "4.0"))
    float Lacunarity = 2.0f;

    /** Output amplitude multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float Amplitude = 1.0f;

    /** Offset applied to coordinates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    FVector Offset = FVector::ZeroVector;
};

/**
 * Base class for noise generation
 * Provides common functionality for all noise algorithms
 */
class MCPAUTOMATIONBRIDGE_API FForge2NoiseBase
{
public:
    FForge2NoiseBase();
    explicit FForge2NoiseBase(int32 Seed);
    virtual ~FForge2NoiseBase() = default;

    /** Initialize with seed */
    void Initialize(int32 Seed);

    /** Get 2D noise value at coordinates */
    virtual float GetNoise2D(float X, float Y) const { return 0.0f; }

    /** Get 3D noise value at coordinates */
    virtual float GetNoise3D(float X, float Y, float Z) const { return 0.0f; }

    /** Get current seed */
    int32 GetSeed() const { return CurrentSeed; }

protected:
    /** Smoothstep interpolation (fade) function */
    static float Fade(float T);

    /** Linear interpolation */
    static float Lerp(float A, float B, float T);

    /** Fast floor function */
    static int32 FastFloor(float X);

    /** Hash function for coordinate */
    int32 Hash(int32 X) const;
    int32 Hash(int32 X, int32 Y) const;
    int32 Hash(int32 X, int32 Y, int32 Z) const;

    /** Permutation table (256 values, doubled for overflow) */
    TArray<uint8> Perm;

    /** Current seed */
    int32 CurrentSeed;

private:
    /** Generate permutation table from seed */
    void GeneratePermutationTable();
};

/**
 * Perlin Noise Generator
 * Classic gradient noise with smooth interpolation
 */
class MCPAUTOMATIONBRIDGE_API FForge2PerlinNoise : public FForge2NoiseBase
{
public:
    using FForge2NoiseBase::FForge2NoiseBase;

    virtual float GetNoise2D(float X, float Y) const override;
    virtual float GetNoise3D(float X, float Y, float Z) const override;

private:
    /** Get gradient value for 2D coordinates */
    float Grad2D(int32 Hash, float X, float Y) const;

    /** Get gradient value for 3D coordinates */
    float Grad3D(int32 Hash, float X, float Y, float Z) const;

    /** Gradient vectors for 2D */
    static const FVector2D Gradients2D[8];

    /** Gradient vectors for 3D */
    static const FVector Gradients3D[12];
};

/**
 * Simplex Noise Generator
 * Faster and less artifacts than Perlin, especially in higher dimensions
 */
class MCPAUTOMATIONBRIDGE_API FForge2SimplexNoise : public FForge2NoiseBase
{
public:
    using FForge2NoiseBase::FForge2NoiseBase;

    virtual float GetNoise2D(float X, float Y) const override;
    virtual float GetNoise3D(float X, float Y, float Z) const override;

private:
    /** Skewing factors */
    static constexpr float F2 = 0.3660254037844386f;  // (sqrt(3) - 1) / 2
    static constexpr float G2 = 0.21132486540518713f; // (3 - sqrt(3)) / 6
    static constexpr float F3 = 1.0f / 3.0f;
    static constexpr float G3 = 1.0f / 6.0f;

    /** Gradient contribution calculation */
    float GradContrib2D(int32 Hash, float X, float Y) const;
    float GradContrib3D(int32 Hash, float X, float Y, float Z) const;
};

/**
 * Voronoi (Worley/Cellular) Noise Generator
 * Creates cell-like patterns based on distance to feature points
 */
class MCPAUTOMATIONBRIDGE_API FForge2VoronoiNoise : public FForge2NoiseBase
{
public:
    using FForge2NoiseBase::FForge2NoiseBase;

    /** Distance function to use */
    EForge2VoronoiDistance DistanceFunction = EForge2VoronoiDistance::Euclidean;

    /** Return value type */
    EForge2VoronoiReturn ReturnType = EForge2VoronoiReturn::F1;

    /** Jitter amount for feature points (0-1) */
    float Jitter = 1.0f;

    virtual float GetNoise2D(float X, float Y) const override;
    virtual float GetNoise3D(float X, float Y, float Z) const override;

    /** Get cell ID at position (for coloring cells) */
    int32 GetCellID2D(float X, float Y) const;
    int32 GetCellID3D(float X, float Y, float Z) const;

private:
    /** Calculate distance based on current function */
    float CalculateDistance(float DX, float DY) const;
    float CalculateDistance(float DX, float DY, float DZ) const;

    /** Get feature point position in cell */
    FVector2D GetFeaturePoint2D(int32 CellX, int32 CellY) const;
    FVector GetFeaturePoint3D(int32 CellX, int32 CellY, int32 CellZ) const;
};

/**
 * Value Noise Generator
 * Simple noise using interpolated random values at grid points
 */
class MCPAUTOMATIONBRIDGE_API FForge2ValueNoise : public FForge2NoiseBase
{
public:
    using FForge2NoiseBase::FForge2NoiseBase;

    virtual float GetNoise2D(float X, float Y) const override;
    virtual float GetNoise3D(float X, float Y, float Z) const override;

private:
    /** Get random value at integer coordinate */
    float GetValue(int32 X, int32 Y) const;
    float GetValue(int32 X, int32 Y, int32 Z) const;
};

/**
 * Fractal Brownian Motion (FBM)
 * Combines multiple octaves of noise for natural-looking terrain
 */
template<typename NoiseType>
class MCPAUTOMATIONBRIDGE_API TForge2FBMNoise
{
public:
    TForge2FBMNoise() : BaseNoise(0) {}
    explicit TForge2FBMNoise(int32 Seed) : BaseNoise(Seed) {}

    void Initialize(int32 Seed) { BaseNoise.Initialize(Seed); }

    float GetNoise2D(float X, float Y, const FForge2NoiseParams& Params) const
    {
        float Total = 0.0f;
        float Frequency = Params.Scale;
        float Amplitude = 1.0f;
        float MaxValue = 0.0f;

        for (int32 i = 0; i < Params.Octaves; ++i)
        {
            Total += BaseNoise.GetNoise2D(
                (X + Params.Offset.X) * Frequency,
                (Y + Params.Offset.Y) * Frequency
            ) * Amplitude;

            MaxValue += Amplitude;
            Amplitude *= Params.Persistence;
            Frequency *= Params.Lacunarity;
        }

        return (Total / MaxValue) * Params.Amplitude;
    }

    float GetNoise3D(float X, float Y, float Z, const FForge2NoiseParams& Params) const
    {
        float Total = 0.0f;
        float Frequency = Params.Scale;
        float Amplitude = 1.0f;
        float MaxValue = 0.0f;

        for (int32 i = 0; i < Params.Octaves; ++i)
        {
            Total += BaseNoise.GetNoise3D(
                (X + Params.Offset.X) * Frequency,
                (Y + Params.Offset.Y) * Frequency,
                (Z + Params.Offset.Z) * Frequency
            ) * Amplitude;

            MaxValue += Amplitude;
            Amplitude *= Params.Persistence;
            Frequency *= Params.Lacunarity;
        }

        return (Total / MaxValue) * Params.Amplitude;
    }

    NoiseType& GetBaseNoise() { return BaseNoise; }
    const NoiseType& GetBaseNoise() const { return BaseNoise; }

private:
    NoiseType BaseNoise;
};

/**
 * Ridged Multifractal Noise
 * Creates sharp ridges, good for mountains
 */
template<typename NoiseType>
class MCPAUTOMATIONBRIDGE_API TForge2RidgedNoise
{
public:
    TForge2RidgedNoise() : BaseNoise(0) {}
    explicit TForge2RidgedNoise(int32 Seed) : BaseNoise(Seed) {}

    void Initialize(int32 Seed) { BaseNoise.Initialize(Seed); }

    /** Ridge sharpness (higher = sharper ridges) */
    float Sharpness = 2.0f;

    float GetNoise2D(float X, float Y, const FForge2NoiseParams& Params) const
    {
        float Total = 0.0f;
        float Frequency = Params.Scale;
        float Amplitude = 1.0f;
        float Weight = 1.0f;

        for (int32 i = 0; i < Params.Octaves; ++i)
        {
            float Signal = BaseNoise.GetNoise2D(
                (X + Params.Offset.X) * Frequency,
                (Y + Params.Offset.Y) * Frequency
            );

            // Make ridges by taking absolute value and inverting
            Signal = FMath::Abs(Signal);
            Signal = 1.0f - Signal;
            Signal = FMath::Pow(Signal, Sharpness);

            // Apply weight from previous octave
            Signal *= Weight;
            Weight = FMath::Clamp(Signal * 2.0f, 0.0f, 1.0f);

            Total += Signal * Amplitude;
            Amplitude *= Params.Persistence;
            Frequency *= Params.Lacunarity;
        }

        return Total * Params.Amplitude;
    }

    float GetNoise3D(float X, float Y, float Z, const FForge2NoiseParams& Params) const
    {
        float Total = 0.0f;
        float Frequency = Params.Scale;
        float Amplitude = 1.0f;
        float Weight = 1.0f;

        for (int32 i = 0; i < Params.Octaves; ++i)
        {
            float Signal = BaseNoise.GetNoise3D(
                (X + Params.Offset.X) * Frequency,
                (Y + Params.Offset.Y) * Frequency,
                (Z + Params.Offset.Z) * Frequency
            );

            Signal = FMath::Abs(Signal);
            Signal = 1.0f - Signal;
            Signal = FMath::Pow(Signal, Sharpness);

            Signal *= Weight;
            Weight = FMath::Clamp(Signal * 2.0f, 0.0f, 1.0f);

            Total += Signal * Amplitude;
            Amplitude *= Params.Persistence;
            Frequency *= Params.Lacunarity;
        }

        return Total * Params.Amplitude;
    }

    NoiseType& GetBaseNoise() { return BaseNoise; }
    const NoiseType& GetBaseNoise() const { return BaseNoise; }

private:
    NoiseType BaseNoise;
};

/**
 * Billow Noise
 * Creates rounded, cloud-like forms (inverted ridged noise)
 */
template<typename NoiseType>
class MCPAUTOMATIONBRIDGE_API TForge2BillowNoise
{
public:
    TForge2BillowNoise() : BaseNoise(0) {}
    explicit TForge2BillowNoise(int32 Seed) : BaseNoise(Seed) {}

    void Initialize(int32 Seed) { BaseNoise.Initialize(Seed); }

    float GetNoise2D(float X, float Y, const FForge2NoiseParams& Params) const
    {
        float Total = 0.0f;
        float Frequency = Params.Scale;
        float Amplitude = 1.0f;
        float MaxValue = 0.0f;

        for (int32 i = 0; i < Params.Octaves; ++i)
        {
            float Signal = BaseNoise.GetNoise2D(
                (X + Params.Offset.X) * Frequency,
                (Y + Params.Offset.Y) * Frequency
            );

            // Billow = absolute value (creates billowy clouds)
            Signal = FMath::Abs(Signal) * 2.0f - 1.0f;

            Total += Signal * Amplitude;
            MaxValue += Amplitude;
            Amplitude *= Params.Persistence;
            Frequency *= Params.Lacunarity;
        }

        return (Total / MaxValue) * Params.Amplitude;
    }

    float GetNoise3D(float X, float Y, float Z, const FForge2NoiseParams& Params) const
    {
        float Total = 0.0f;
        float Frequency = Params.Scale;
        float Amplitude = 1.0f;
        float MaxValue = 0.0f;

        for (int32 i = 0; i < Params.Octaves; ++i)
        {
            float Signal = BaseNoise.GetNoise3D(
                (X + Params.Offset.X) * Frequency,
                (Y + Params.Offset.Y) * Frequency,
                (Z + Params.Offset.Z) * Frequency
            );

            Signal = FMath::Abs(Signal) * 2.0f - 1.0f;

            Total += Signal * Amplitude;
            MaxValue += Amplitude;
            Amplitude *= Params.Persistence;
            Frequency *= Params.Lacunarity;
        }

        return (Total / MaxValue) * Params.Amplitude;
    }

    NoiseType& GetBaseNoise() { return BaseNoise; }
    const NoiseType& GetBaseNoise() const { return BaseNoise; }

private:
    NoiseType BaseNoise;
};

/**
 * Unified noise generator that can produce any type of noise
 */
class MCPAUTOMATIONBRIDGE_API FForge2NoiseGenerator
{
public:
    FForge2NoiseGenerator();
    explicit FForge2NoiseGenerator(int32 Seed);

    /** Initialize with seed */
    void Initialize(int32 Seed);

    /** Set the noise algorithm to use */
    void SetAlgorithm(EForge2NoiseAlgorithm Algorithm);

    /** Get 2D noise value */
    float GetNoise2D(float X, float Y, const FForge2NoiseParams& Params) const;

    /** Get 3D noise value */
    float GetNoise3D(float X, float Y, float Z, const FForge2NoiseParams& Params) const;

    /** Generate a 2D heightmap */
    TArray<float> GenerateHeightmap(int32 Width, int32 Height, const FForge2NoiseParams& Params) const;

    /** Generate a 3D volume */
    TArray<float> GenerateVolume(int32 Width, int32 Height, int32 Depth, const FForge2NoiseParams& Params) const;

    /** Access specific noise types for advanced configuration */
    FForge2PerlinNoise& GetPerlin() { return PerlinNoise; }
    FForge2SimplexNoise& GetSimplex() { return SimplexNoise; }
    FForge2VoronoiNoise& GetVoronoi() { return VoronoiNoise; }
    FForge2ValueNoise& GetValue() { return ValueNoise; }

private:
    EForge2NoiseAlgorithm CurrentAlgorithm;
    int32 CurrentSeed;

    FForge2PerlinNoise PerlinNoise;
    FForge2SimplexNoise SimplexNoise;
    FForge2VoronoiNoise VoronoiNoise;
    FForge2ValueNoise ValueNoise;

    TForge2FBMNoise<FForge2PerlinNoise> FBMPerlin;
    TForge2FBMNoise<FForge2SimplexNoise> FBMSimplex;
    TForge2RidgedNoise<FForge2PerlinNoise> RidgedPerlin;
    TForge2BillowNoise<FForge2PerlinNoise> BillowPerlin;
};

/**
 * Domain warping utilities for more organic noise
 */
class MCPAUTOMATIONBRIDGE_API FForge2DomainWarp
{
public:
    /** Apply domain warping to coordinates */
    static FVector2D Warp2D(FVector2D Coord, const FForge2NoiseGenerator& WarpNoise,
                            const FForge2NoiseParams& Params, float WarpStrength);

    static FVector Warp3D(FVector Coord, const FForge2NoiseGenerator& WarpNoise,
                          const FForge2NoiseParams& Params, float WarpStrength);

    /** Multi-level domain warping for complex terrain */
    static float GetWarpedNoise2D(float X, float Y, const FForge2NoiseGenerator& MainNoise,
                                   const FForge2NoiseGenerator& WarpNoise,
                                   const FForge2NoiseParams& MainParams,
                                   const FForge2NoiseParams& WarpParams,
                                   float WarpStrength, int32 WarpIterations = 1);
};

// Type aliases for common noise combinations
using FForge2FBMPerlin = TForge2FBMNoise<FForge2PerlinNoise>;
using FForge2FBMSimplex = TForge2FBMNoise<FForge2SimplexNoise>;
using FForge2RidgedPerlin = TForge2RidgedNoise<FForge2PerlinNoise>;
using FForge2BillowPerlin = TForge2BillowNoise<FForge2PerlinNoise>;
