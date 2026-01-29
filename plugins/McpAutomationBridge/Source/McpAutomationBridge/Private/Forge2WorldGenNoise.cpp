// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// World Generation Noise Algorithms - Implementation

#include "Forge2WorldGenNoise.h"

// ============================================================================
// Gradient Tables
// ============================================================================

const FVector2D FForge2PerlinNoise::Gradients2D[8] = {
    FVector2D(1.0f, 0.0f), FVector2D(-1.0f, 0.0f),
    FVector2D(0.0f, 1.0f), FVector2D(0.0f, -1.0f),
    FVector2D(0.707107f, 0.707107f), FVector2D(-0.707107f, 0.707107f),
    FVector2D(0.707107f, -0.707107f), FVector2D(-0.707107f, -0.707107f)
};

const FVector FForge2PerlinNoise::Gradients3D[12] = {
    FVector(1.0f, 1.0f, 0.0f), FVector(-1.0f, 1.0f, 0.0f),
    FVector(1.0f, -1.0f, 0.0f), FVector(-1.0f, -1.0f, 0.0f),
    FVector(1.0f, 0.0f, 1.0f), FVector(-1.0f, 0.0f, 1.0f),
    FVector(1.0f, 0.0f, -1.0f), FVector(-1.0f, 0.0f, -1.0f),
    FVector(0.0f, 1.0f, 1.0f), FVector(0.0f, -1.0f, 1.0f),
    FVector(0.0f, 1.0f, -1.0f), FVector(0.0f, -1.0f, -1.0f)
};

// ============================================================================
// Base Noise Implementation
// ============================================================================

FForge2NoiseBase::FForge2NoiseBase()
    : CurrentSeed(0)
{
    Perm.SetNum(512);
    GeneratePermutationTable();
}

FForge2NoiseBase::FForge2NoiseBase(int32 Seed)
    : CurrentSeed(Seed)
{
    Perm.SetNum(512);
    GeneratePermutationTable();
}

void FForge2NoiseBase::Initialize(int32 Seed)
{
    CurrentSeed = Seed;
    GeneratePermutationTable();
}

void FForge2NoiseBase::GeneratePermutationTable()
{
    // Initialize with sequential values
    TArray<uint8> Source;
    Source.SetNum(256);
    for (int32 i = 0; i < 256; ++i)
    {
        Source[i] = static_cast<uint8>(i);
    }

    // Fisher-Yates shuffle using seed
    FRandomStream RandomStream(CurrentSeed);
    for (int32 i = 255; i > 0; --i)
    {
        int32 j = RandomStream.RandRange(0, i);
        Swap(Source[i], Source[j]);
    }

    // Duplicate to avoid modulo operations
    for (int32 i = 0; i < 256; ++i)
    {
        Perm[i] = Source[i];
        Perm[i + 256] = Source[i];
    }
}

float FForge2NoiseBase::Fade(float T)
{
    // Improved smoothstep: 6t^5 - 15t^4 + 10t^3
    return T * T * T * (T * (T * 6.0f - 15.0f) + 10.0f);
}

float FForge2NoiseBase::Lerp(float A, float B, float T)
{
    return A + T * (B - A);
}

int32 FForge2NoiseBase::FastFloor(float X)
{
    int32 Xi = static_cast<int32>(X);
    return (X < Xi) ? Xi - 1 : Xi;
}

int32 FForge2NoiseBase::Hash(int32 X) const
{
    return Perm[X & 255];
}

int32 FForge2NoiseBase::Hash(int32 X, int32 Y) const
{
    return Perm[(Perm[X & 255] + Y) & 255];
}

int32 FForge2NoiseBase::Hash(int32 X, int32 Y, int32 Z) const
{
    return Perm[(Perm[(Perm[X & 255] + Y) & 255] + Z) & 255];
}

// ============================================================================
// Perlin Noise Implementation
// ============================================================================

float FForge2PerlinNoise::Grad2D(int32 HashValue, float X, float Y) const
{
    const FVector2D& G = Gradients2D[HashValue & 7];
    return G.X * X + G.Y * Y;
}

float FForge2PerlinNoise::Grad3D(int32 HashValue, float X, float Y, float Z) const
{
    const FVector& G = Gradients3D[HashValue % 12];
    return G.X * X + G.Y * Y + G.Z * Z;
}

float FForge2PerlinNoise::GetNoise2D(float X, float Y) const
{
    // Find unit grid cell
    int32 X0 = FastFloor(X);
    int32 Y0 = FastFloor(Y);

    // Relative position within cell
    float Xf = X - X0;
    float Yf = Y - Y0;

    // Wrap coordinates
    X0 &= 255;
    Y0 &= 255;

    // Calculate fade curves
    float U = Fade(Xf);
    float V = Fade(Yf);

    // Hash coordinates of corners
    int32 AA = Hash(X0, Y0);
    int32 AB = Hash(X0, Y0 + 1);
    int32 BA = Hash(X0 + 1, Y0);
    int32 BB = Hash(X0 + 1, Y0 + 1);

    // Calculate gradient contributions
    float GradAA = Grad2D(AA, Xf, Yf);
    float GradBA = Grad2D(BA, Xf - 1.0f, Yf);
    float GradAB = Grad2D(AB, Xf, Yf - 1.0f);
    float GradBB = Grad2D(BB, Xf - 1.0f, Yf - 1.0f);

    // Interpolate
    float LerpX1 = Lerp(GradAA, GradBA, U);
    float LerpX2 = Lerp(GradAB, GradBB, U);
    float Result = Lerp(LerpX1, LerpX2, V);

    // Return value in range [-1, 1]
    return Result;
}

float FForge2PerlinNoise::GetNoise3D(float X, float Y, float Z) const
{
    int32 X0 = FastFloor(X);
    int32 Y0 = FastFloor(Y);
    int32 Z0 = FastFloor(Z);

    float Xf = X - X0;
    float Yf = Y - Y0;
    float Zf = Z - Z0;

    X0 &= 255;
    Y0 &= 255;
    Z0 &= 255;

    float U = Fade(Xf);
    float V = Fade(Yf);
    float W = Fade(Zf);

    // Hash all 8 corners
    int32 AAA = Hash(X0, Y0, Z0);
    int32 BAA = Hash(X0 + 1, Y0, Z0);
    int32 ABA = Hash(X0, Y0 + 1, Z0);
    int32 BBA = Hash(X0 + 1, Y0 + 1, Z0);
    int32 AAB = Hash(X0, Y0, Z0 + 1);
    int32 BAB = Hash(X0 + 1, Y0, Z0 + 1);
    int32 ABB = Hash(X0, Y0 + 1, Z0 + 1);
    int32 BBB = Hash(X0 + 1, Y0 + 1, Z0 + 1);

    // Gradient contributions
    float X1 = Lerp(Grad3D(AAA, Xf, Yf, Zf), Grad3D(BAA, Xf - 1, Yf, Zf), U);
    float X2 = Lerp(Grad3D(ABA, Xf, Yf - 1, Zf), Grad3D(BBA, Xf - 1, Yf - 1, Zf), U);
    float Y1 = Lerp(X1, X2, V);

    X1 = Lerp(Grad3D(AAB, Xf, Yf, Zf - 1), Grad3D(BAB, Xf - 1, Yf, Zf - 1), U);
    X2 = Lerp(Grad3D(ABB, Xf, Yf - 1, Zf - 1), Grad3D(BBB, Xf - 1, Yf - 1, Zf - 1), U);
    float Y2 = Lerp(X1, X2, V);

    return Lerp(Y1, Y2, W);
}

// ============================================================================
// Simplex Noise Implementation
// ============================================================================

float FForge2SimplexNoise::GradContrib2D(int32 HashValue, float X, float Y) const
{
    const FVector2D& G = FForge2PerlinNoise::Gradients2D[HashValue & 7];
    return G.X * X + G.Y * Y;
}

float FForge2SimplexNoise::GradContrib3D(int32 HashValue, float X, float Y, float Z) const
{
    const FVector& G = FForge2PerlinNoise::Gradients3D[HashValue % 12];
    return G.X * X + G.Y * Y + G.Z * Z;
}

float FForge2SimplexNoise::GetNoise2D(float X, float Y) const
{
    // Skew input space to simplex space
    float S = (X + Y) * F2;
    int32 I = FastFloor(X + S);
    int32 J = FastFloor(Y + S);

    // Unskew back to (x, y) space
    float T = (I + J) * G2;
    float X0 = I - T;
    float Y0 = J - T;

    // Relative coords in simplex cell
    float Xf0 = X - X0;
    float Yf0 = Y - Y0;

    // Determine which simplex we're in
    int32 I1, J1;
    if (Xf0 > Yf0)
    {
        I1 = 1; J1 = 0;
    }
    else
    {
        I1 = 0; J1 = 1;
    }

    // Offsets for middle and last corner
    float Xf1 = Xf0 - I1 + G2;
    float Yf1 = Yf0 - J1 + G2;
    float Xf2 = Xf0 - 1.0f + 2.0f * G2;
    float Yf2 = Yf0 - 1.0f + 2.0f * G2;

    // Hash the corners
    int32 II = I & 255;
    int32 JJ = J & 255;

    // Calculate contributions from corners
    float N0 = 0.0f, N1 = 0.0f, N2 = 0.0f;

    float T0 = 0.5f - Xf0 * Xf0 - Yf0 * Yf0;
    if (T0 >= 0.0f)
    {
        T0 *= T0;
        N0 = T0 * T0 * GradContrib2D(Hash(II, JJ), Xf0, Yf0);
    }

    float T1 = 0.5f - Xf1 * Xf1 - Yf1 * Yf1;
    if (T1 >= 0.0f)
    {
        T1 *= T1;
        N1 = T1 * T1 * GradContrib2D(Hash(II + I1, JJ + J1), Xf1, Yf1);
    }

    float T2 = 0.5f - Xf2 * Xf2 - Yf2 * Yf2;
    if (T2 >= 0.0f)
    {
        T2 *= T2;
        N2 = T2 * T2 * GradContrib2D(Hash(II + 1, JJ + 1), Xf2, Yf2);
    }

    // Scale to [-1, 1] range
    return 70.0f * (N0 + N1 + N2);
}

float FForge2SimplexNoise::GetNoise3D(float X, float Y, float Z) const
{
    // Skew to simplex space
    float S = (X + Y + Z) * F3;
    int32 I = FastFloor(X + S);
    int32 J = FastFloor(Y + S);
    int32 K = FastFloor(Z + S);

    // Unskew
    float T = (I + J + K) * G3;
    float X0 = I - T;
    float Y0 = J - T;
    float Z0 = K - T;

    float Xf0 = X - X0;
    float Yf0 = Y - Y0;
    float Zf0 = Z - Z0;

    // Determine simplex
    int32 I1, J1, K1, I2, J2, K2;
    if (Xf0 >= Yf0)
    {
        if (Yf0 >= Zf0)      { I1=1; J1=0; K1=0; I2=1; J2=1; K2=0; }
        else if (Xf0 >= Zf0) { I1=1; J1=0; K1=0; I2=1; J2=0; K2=1; }
        else                 { I1=0; J1=0; K1=1; I2=1; J2=0; K2=1; }
    }
    else
    {
        if (Yf0 < Zf0)       { I1=0; J1=0; K1=1; I2=0; J2=1; K2=1; }
        else if (Xf0 < Zf0)  { I1=0; J1=1; K1=0; I2=0; J2=1; K2=1; }
        else                 { I1=0; J1=1; K1=0; I2=1; J2=1; K2=0; }
    }

    float Xf1 = Xf0 - I1 + G3;
    float Yf1 = Yf0 - J1 + G3;
    float Zf1 = Zf0 - K1 + G3;
    float Xf2 = Xf0 - I2 + 2.0f * G3;
    float Yf2 = Yf0 - J2 + 2.0f * G3;
    float Zf2 = Zf0 - K2 + 2.0f * G3;
    float Xf3 = Xf0 - 1.0f + 3.0f * G3;
    float Yf3 = Yf0 - 1.0f + 3.0f * G3;
    float Zf3 = Zf0 - 1.0f + 3.0f * G3;

    int32 II = I & 255;
    int32 JJ = J & 255;
    int32 KK = K & 255;

    float N0 = 0.0f, N1 = 0.0f, N2 = 0.0f, N3 = 0.0f;

    float T0 = 0.6f - Xf0*Xf0 - Yf0*Yf0 - Zf0*Zf0;
    if (T0 >= 0.0f)
    {
        T0 *= T0;
        N0 = T0 * T0 * GradContrib3D(Hash(II, JJ, KK), Xf0, Yf0, Zf0);
    }

    float T1 = 0.6f - Xf1*Xf1 - Yf1*Yf1 - Zf1*Zf1;
    if (T1 >= 0.0f)
    {
        T1 *= T1;
        N1 = T1 * T1 * GradContrib3D(Hash(II+I1, JJ+J1, KK+K1), Xf1, Yf1, Zf1);
    }

    float T2 = 0.6f - Xf2*Xf2 - Yf2*Yf2 - Zf2*Zf2;
    if (T2 >= 0.0f)
    {
        T2 *= T2;
        N2 = T2 * T2 * GradContrib3D(Hash(II+I2, JJ+J2, KK+K2), Xf2, Yf2, Zf2);
    }

    float T3 = 0.6f - Xf3*Xf3 - Yf3*Yf3 - Zf3*Zf3;
    if (T3 >= 0.0f)
    {
        T3 *= T3;
        N3 = T3 * T3 * GradContrib3D(Hash(II+1, JJ+1, KK+1), Xf3, Yf3, Zf3);
    }

    return 32.0f * (N0 + N1 + N2 + N3);
}

// ============================================================================
// Voronoi Noise Implementation
// ============================================================================

float FForge2VoronoiNoise::CalculateDistance(float DX, float DY) const
{
    switch (DistanceFunction)
    {
    case EForge2VoronoiDistance::Euclidean:
        return FMath::Sqrt(DX * DX + DY * DY);
    case EForge2VoronoiDistance::Manhattan:
        return FMath::Abs(DX) + FMath::Abs(DY);
    case EForge2VoronoiDistance::Chebyshev:
        return FMath::Max(FMath::Abs(DX), FMath::Abs(DY));
    case EForge2VoronoiDistance::Minkowski:
        return FMath::Pow(FMath::Pow(FMath::Abs(DX), 4.0f) + FMath::Pow(FMath::Abs(DY), 4.0f), 0.25f);
    default:
        return FMath::Sqrt(DX * DX + DY * DY);
    }
}

float FForge2VoronoiNoise::CalculateDistance(float DX, float DY, float DZ) const
{
    switch (DistanceFunction)
    {
    case EForge2VoronoiDistance::Euclidean:
        return FMath::Sqrt(DX * DX + DY * DY + DZ * DZ);
    case EForge2VoronoiDistance::Manhattan:
        return FMath::Abs(DX) + FMath::Abs(DY) + FMath::Abs(DZ);
    case EForge2VoronoiDistance::Chebyshev:
        return FMath::Max3(FMath::Abs(DX), FMath::Abs(DY), FMath::Abs(DZ));
    case EForge2VoronoiDistance::Minkowski:
        return FMath::Pow(FMath::Pow(FMath::Abs(DX), 4.0f) +
                         FMath::Pow(FMath::Abs(DY), 4.0f) +
                         FMath::Pow(FMath::Abs(DZ), 4.0f), 0.25f);
    default:
        return FMath::Sqrt(DX * DX + DY * DY + DZ * DZ);
    }
}

FVector2D FForge2VoronoiNoise::GetFeaturePoint2D(int32 CellX, int32 CellY) const
{
    int32 HashValue = Hash(CellX, CellY);
    FRandomStream Random(HashValue ^ CurrentSeed);

    float OffsetX = Random.FRand() * Jitter;
    float OffsetY = Random.FRand() * Jitter;

    return FVector2D(CellX + 0.5f * (1.0f - Jitter) + OffsetX,
                     CellY + 0.5f * (1.0f - Jitter) + OffsetY);
}

FVector FForge2VoronoiNoise::GetFeaturePoint3D(int32 CellX, int32 CellY, int32 CellZ) const
{
    int32 HashValue = Hash(CellX, CellY, CellZ);
    FRandomStream Random(HashValue ^ CurrentSeed);

    float OffsetX = Random.FRand() * Jitter;
    float OffsetY = Random.FRand() * Jitter;
    float OffsetZ = Random.FRand() * Jitter;

    return FVector(CellX + 0.5f * (1.0f - Jitter) + OffsetX,
                   CellY + 0.5f * (1.0f - Jitter) + OffsetY,
                   CellZ + 0.5f * (1.0f - Jitter) + OffsetZ);
}

float FForge2VoronoiNoise::GetNoise2D(float X, float Y) const
{
    int32 CellX = FastFloor(X);
    int32 CellY = FastFloor(Y);

    float F1 = FLT_MAX;
    float F2 = FLT_MAX;
    int32 ClosestCellX = 0, ClosestCellY = 0;

    // Check 3x3 neighborhood
    for (int32 OffY = -1; OffY <= 1; ++OffY)
    {
        for (int32 OffX = -1; OffX <= 1; ++OffX)
        {
            int32 NeighborX = CellX + OffX;
            int32 NeighborY = CellY + OffY;

            FVector2D FeaturePoint = GetFeaturePoint2D(NeighborX, NeighborY);
            float Distance = CalculateDistance(X - FeaturePoint.X, Y - FeaturePoint.Y);

            if (Distance < F1)
            {
                F2 = F1;
                F1 = Distance;
                ClosestCellX = NeighborX;
                ClosestCellY = NeighborY;
            }
            else if (Distance < F2)
            {
                F2 = Distance;
            }
        }
    }

    switch (ReturnType)
    {
    case EForge2VoronoiReturn::F1:
        return F1;
    case EForge2VoronoiReturn::F2:
        return F2;
    case EForge2VoronoiReturn::F2MinusF1:
        return F2 - F1;
    case EForge2VoronoiReturn::CellValue:
        return static_cast<float>(Hash(ClosestCellX, ClosestCellY)) / 255.0f;
    default:
        return F1;
    }
}

float FForge2VoronoiNoise::GetNoise3D(float X, float Y, float Z) const
{
    int32 CellX = FastFloor(X);
    int32 CellY = FastFloor(Y);
    int32 CellZ = FastFloor(Z);

    float F1 = FLT_MAX;
    float F2 = FLT_MAX;
    int32 ClosestCellX = 0, ClosestCellY = 0, ClosestCellZ = 0;

    for (int32 OffZ = -1; OffZ <= 1; ++OffZ)
    {
        for (int32 OffY = -1; OffY <= 1; ++OffY)
        {
            for (int32 OffX = -1; OffX <= 1; ++OffX)
            {
                int32 NeighborX = CellX + OffX;
                int32 NeighborY = CellY + OffY;
                int32 NeighborZ = CellZ + OffZ;

                FVector FeaturePoint = GetFeaturePoint3D(NeighborX, NeighborY, NeighborZ);
                float Distance = CalculateDistance(X - FeaturePoint.X, Y - FeaturePoint.Y, Z - FeaturePoint.Z);

                if (Distance < F1)
                {
                    F2 = F1;
                    F1 = Distance;
                    ClosestCellX = NeighborX;
                    ClosestCellY = NeighborY;
                    ClosestCellZ = NeighborZ;
                }
                else if (Distance < F2)
                {
                    F2 = Distance;
                }
            }
        }
    }

    switch (ReturnType)
    {
    case EForge2VoronoiReturn::F1:
        return F1;
    case EForge2VoronoiReturn::F2:
        return F2;
    case EForge2VoronoiReturn::F2MinusF1:
        return F2 - F1;
    case EForge2VoronoiReturn::CellValue:
        return static_cast<float>(Hash(ClosestCellX, ClosestCellY, ClosestCellZ)) / 255.0f;
    default:
        return F1;
    }
}

int32 FForge2VoronoiNoise::GetCellID2D(float X, float Y) const
{
    int32 CellX = FastFloor(X);
    int32 CellY = FastFloor(Y);

    float MinDist = FLT_MAX;
    int32 ClosestCellX = 0, ClosestCellY = 0;

    for (int32 OffY = -1; OffY <= 1; ++OffY)
    {
        for (int32 OffX = -1; OffX <= 1; ++OffX)
        {
            int32 NeighborX = CellX + OffX;
            int32 NeighborY = CellY + OffY;

            FVector2D FeaturePoint = GetFeaturePoint2D(NeighborX, NeighborY);
            float Distance = CalculateDistance(X - FeaturePoint.X, Y - FeaturePoint.Y);

            if (Distance < MinDist)
            {
                MinDist = Distance;
                ClosestCellX = NeighborX;
                ClosestCellY = NeighborY;
            }
        }
    }

    return Hash(ClosestCellX, ClosestCellY);
}

int32 FForge2VoronoiNoise::GetCellID3D(float X, float Y, float Z) const
{
    int32 CellX = FastFloor(X);
    int32 CellY = FastFloor(Y);
    int32 CellZ = FastFloor(Z);

    float MinDist = FLT_MAX;
    int32 ClosestCellX = 0, ClosestCellY = 0, ClosestCellZ = 0;

    for (int32 OffZ = -1; OffZ <= 1; ++OffZ)
    {
        for (int32 OffY = -1; OffY <= 1; ++OffY)
        {
            for (int32 OffX = -1; OffX <= 1; ++OffX)
            {
                int32 NeighborX = CellX + OffX;
                int32 NeighborY = CellY + OffY;
                int32 NeighborZ = CellZ + OffZ;

                FVector FeaturePoint = GetFeaturePoint3D(NeighborX, NeighborY, NeighborZ);
                float Distance = CalculateDistance(X - FeaturePoint.X, Y - FeaturePoint.Y, Z - FeaturePoint.Z);

                if (Distance < MinDist)
                {
                    MinDist = Distance;
                    ClosestCellX = NeighborX;
                    ClosestCellY = NeighborY;
                    ClosestCellZ = NeighborZ;
                }
            }
        }
    }

    return Hash(ClosestCellX, ClosestCellY, ClosestCellZ);
}

// ============================================================================
// Value Noise Implementation
// ============================================================================

float FForge2ValueNoise::GetValue(int32 X, int32 Y) const
{
    return static_cast<float>(Hash(X, Y)) / 255.0f * 2.0f - 1.0f;
}

float FForge2ValueNoise::GetValue(int32 X, int32 Y, int32 Z) const
{
    return static_cast<float>(Hash(X, Y, Z)) / 255.0f * 2.0f - 1.0f;
}

float FForge2ValueNoise::GetNoise2D(float X, float Y) const
{
    int32 X0 = FastFloor(X);
    int32 Y0 = FastFloor(Y);

    float Xf = X - X0;
    float Yf = Y - Y0;

    float U = Fade(Xf);
    float V = Fade(Yf);

    float V00 = GetValue(X0, Y0);
    float V10 = GetValue(X0 + 1, Y0);
    float V01 = GetValue(X0, Y0 + 1);
    float V11 = GetValue(X0 + 1, Y0 + 1);

    float LerpX1 = Lerp(V00, V10, U);
    float LerpX2 = Lerp(V01, V11, U);

    return Lerp(LerpX1, LerpX2, V);
}

float FForge2ValueNoise::GetNoise3D(float X, float Y, float Z) const
{
    int32 X0 = FastFloor(X);
    int32 Y0 = FastFloor(Y);
    int32 Z0 = FastFloor(Z);

    float Xf = X - X0;
    float Yf = Y - Y0;
    float Zf = Z - Z0;

    float U = Fade(Xf);
    float V = Fade(Yf);
    float W = Fade(Zf);

    float V000 = GetValue(X0, Y0, Z0);
    float V100 = GetValue(X0 + 1, Y0, Z0);
    float V010 = GetValue(X0, Y0 + 1, Z0);
    float V110 = GetValue(X0 + 1, Y0 + 1, Z0);
    float V001 = GetValue(X0, Y0, Z0 + 1);
    float V101 = GetValue(X0 + 1, Y0, Z0 + 1);
    float V011 = GetValue(X0, Y0 + 1, Z0 + 1);
    float V111 = GetValue(X0 + 1, Y0 + 1, Z0 + 1);

    float LerpX00 = Lerp(V000, V100, U);
    float LerpX10 = Lerp(V010, V110, U);
    float LerpX01 = Lerp(V001, V101, U);
    float LerpX11 = Lerp(V011, V111, U);

    float LerpY0 = Lerp(LerpX00, LerpX10, V);
    float LerpY1 = Lerp(LerpX01, LerpX11, V);

    return Lerp(LerpY0, LerpY1, W);
}

// ============================================================================
// Unified Noise Generator Implementation
// ============================================================================

FForge2NoiseGenerator::FForge2NoiseGenerator()
    : CurrentAlgorithm(EForge2NoiseAlgorithm::Perlin)
    , CurrentSeed(0)
    , PerlinNoise(0)
    , SimplexNoise(0)
    , VoronoiNoise(0)
    , ValueNoise(0)
    , FBMPerlin(0)
    , FBMSimplex(0)
    , RidgedPerlin(0)
    , BillowPerlin(0)
{
}

FForge2NoiseGenerator::FForge2NoiseGenerator(int32 Seed)
    : CurrentAlgorithm(EForge2NoiseAlgorithm::Perlin)
    , CurrentSeed(Seed)
    , PerlinNoise(Seed)
    , SimplexNoise(Seed)
    , VoronoiNoise(Seed)
    , ValueNoise(Seed)
    , FBMPerlin(Seed)
    , FBMSimplex(Seed)
    , RidgedPerlin(Seed)
    , BillowPerlin(Seed)
{
}

void FForge2NoiseGenerator::Initialize(int32 Seed)
{
    CurrentSeed = Seed;
    PerlinNoise.Initialize(Seed);
    SimplexNoise.Initialize(Seed);
    VoronoiNoise.Initialize(Seed);
    ValueNoise.Initialize(Seed);
    FBMPerlin.Initialize(Seed);
    FBMSimplex.Initialize(Seed);
    RidgedPerlin.Initialize(Seed);
    BillowPerlin.Initialize(Seed);
}

void FForge2NoiseGenerator::SetAlgorithm(EForge2NoiseAlgorithm Algorithm)
{
    CurrentAlgorithm = Algorithm;
}

float FForge2NoiseGenerator::GetNoise2D(float X, float Y, const FForge2NoiseParams& Params) const
{
    switch (CurrentAlgorithm)
    {
    case EForge2NoiseAlgorithm::Perlin:
        return PerlinNoise.GetNoise2D(X * Params.Scale, Y * Params.Scale) * Params.Amplitude;

    case EForge2NoiseAlgorithm::Simplex:
        return SimplexNoise.GetNoise2D(X * Params.Scale, Y * Params.Scale) * Params.Amplitude;

    case EForge2NoiseAlgorithm::Voronoi:
    case EForge2NoiseAlgorithm::Cellular:
        return VoronoiNoise.GetNoise2D(X * Params.Scale, Y * Params.Scale) * Params.Amplitude;

    case EForge2NoiseAlgorithm::Value:
        return ValueNoise.GetNoise2D(X * Params.Scale, Y * Params.Scale) * Params.Amplitude;

    case EForge2NoiseAlgorithm::FBM:
        return FBMPerlin.GetNoise2D(X, Y, Params);

    case EForge2NoiseAlgorithm::Ridged:
        return RidgedPerlin.GetNoise2D(X, Y, Params);

    case EForge2NoiseAlgorithm::Billow:
        return BillowPerlin.GetNoise2D(X, Y, Params);

    default:
        return PerlinNoise.GetNoise2D(X * Params.Scale, Y * Params.Scale) * Params.Amplitude;
    }
}

float FForge2NoiseGenerator::GetNoise3D(float X, float Y, float Z, const FForge2NoiseParams& Params) const
{
    switch (CurrentAlgorithm)
    {
    case EForge2NoiseAlgorithm::Perlin:
        return PerlinNoise.GetNoise3D(X * Params.Scale, Y * Params.Scale, Z * Params.Scale) * Params.Amplitude;

    case EForge2NoiseAlgorithm::Simplex:
        return SimplexNoise.GetNoise3D(X * Params.Scale, Y * Params.Scale, Z * Params.Scale) * Params.Amplitude;

    case EForge2NoiseAlgorithm::Voronoi:
    case EForge2NoiseAlgorithm::Cellular:
        return VoronoiNoise.GetNoise3D(X * Params.Scale, Y * Params.Scale, Z * Params.Scale) * Params.Amplitude;

    case EForge2NoiseAlgorithm::Value:
        return ValueNoise.GetNoise3D(X * Params.Scale, Y * Params.Scale, Z * Params.Scale) * Params.Amplitude;

    case EForge2NoiseAlgorithm::FBM:
        return FBMPerlin.GetNoise3D(X, Y, Z, Params);

    case EForge2NoiseAlgorithm::Ridged:
        return RidgedPerlin.GetNoise3D(X, Y, Z, Params);

    case EForge2NoiseAlgorithm::Billow:
        return BillowPerlin.GetNoise3D(X, Y, Z, Params);

    default:
        return PerlinNoise.GetNoise3D(X * Params.Scale, Y * Params.Scale, Z * Params.Scale) * Params.Amplitude;
    }
}

TArray<float> FForge2NoiseGenerator::GenerateHeightmap(int32 Width, int32 Height, const FForge2NoiseParams& Params) const
{
    TArray<float> Heightmap;
    Heightmap.SetNum(Width * Height);

    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            Heightmap[Y * Width + X] = GetNoise2D(static_cast<float>(X), static_cast<float>(Y), Params);
        }
    }

    return Heightmap;
}

TArray<float> FForge2NoiseGenerator::GenerateVolume(int32 Width, int32 Height, int32 Depth, const FForge2NoiseParams& Params) const
{
    TArray<float> Volume;
    Volume.SetNum(Width * Height * Depth);

    for (int32 Z = 0; Z < Depth; ++Z)
    {
        for (int32 Y = 0; Y < Height; ++Y)
        {
            for (int32 X = 0; X < Width; ++X)
            {
                Volume[Z * Width * Height + Y * Width + X] = GetNoise3D(
                    static_cast<float>(X),
                    static_cast<float>(Y),
                    static_cast<float>(Z),
                    Params
                );
            }
        }
    }

    return Volume;
}

// ============================================================================
// Domain Warping Implementation
// ============================================================================

FVector2D FForge2DomainWarp::Warp2D(FVector2D Coord, const FForge2NoiseGenerator& WarpNoise,
                                     const FForge2NoiseParams& Params, float WarpStrength)
{
    FForge2NoiseParams OffsetParams = Params;
    OffsetParams.Offset.X += 1000.0f;

    float WarpX = WarpNoise.GetNoise2D(Coord.X, Coord.Y, Params) * WarpStrength;
    float WarpY = WarpNoise.GetNoise2D(Coord.X, Coord.Y, OffsetParams) * WarpStrength;

    return FVector2D(Coord.X + WarpX, Coord.Y + WarpY);
}

FVector FForge2DomainWarp::Warp3D(FVector Coord, const FForge2NoiseGenerator& WarpNoise,
                                   const FForge2NoiseParams& Params, float WarpStrength)
{
    FForge2NoiseParams OffsetParams1 = Params;
    OffsetParams1.Offset.X += 1000.0f;

    FForge2NoiseParams OffsetParams2 = Params;
    OffsetParams2.Offset.Y += 1000.0f;

    float WarpX = WarpNoise.GetNoise3D(Coord.X, Coord.Y, Coord.Z, Params) * WarpStrength;
    float WarpY = WarpNoise.GetNoise3D(Coord.X, Coord.Y, Coord.Z, OffsetParams1) * WarpStrength;
    float WarpZ = WarpNoise.GetNoise3D(Coord.X, Coord.Y, Coord.Z, OffsetParams2) * WarpStrength;

    return FVector(Coord.X + WarpX, Coord.Y + WarpY, Coord.Z + WarpZ);
}

float FForge2DomainWarp::GetWarpedNoise2D(float X, float Y, const FForge2NoiseGenerator& MainNoise,
                                           const FForge2NoiseGenerator& WarpNoise,
                                           const FForge2NoiseParams& MainParams,
                                           const FForge2NoiseParams& WarpParams,
                                           float WarpStrength, int32 WarpIterations)
{
    FVector2D WarpedCoord(X, Y);

    for (int32 i = 0; i < WarpIterations; ++i)
    {
        WarpedCoord = Warp2D(WarpedCoord, WarpNoise, WarpParams, WarpStrength);
    }

    return MainNoise.GetNoise2D(WarpedCoord.X, WarpedCoord.Y, MainParams);
}
