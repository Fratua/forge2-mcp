// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// World Generation Erosion Algorithms - Implementation

#include "Forge2WorldGenErosion.h"

// ============================================================================
// Thermal Erosion Implementation
// ============================================================================

constexpr int32 FForge2ThermalErosion::DirX[8];
constexpr int32 FForge2ThermalErosion::DirY[8];
constexpr float FForge2ThermalErosion::DirDist[8];

bool FForge2ThermalErosion::IsValidCoord(int32 X, int32 Y, int32 Width, int32 Height) const
{
    return X >= 0 && X < Width && Y >= 0 && Y < Height;
}

float FForge2ThermalErosion::GetSlope(const TArray<float>& Heightmap, int32 Width, int32 Height,
                                       int32 X, int32 Y, int32 Dx, int32 Dy, float CellSize) const
{
    int32 Nx = X + Dx;
    int32 Ny = Y + Dy;

    if (!IsValidCoord(Nx, Ny, Width, Height))
    {
        return 0.0f;
    }

    float HeightDiff = Heightmap[Y * Width + X] - Heightmap[Ny * Width + Nx];
    float Distance = CellSize * FMath::Sqrt(static_cast<float>(Dx * Dx + Dy * Dy));

    return HeightDiff / Distance;
}

void FForge2ThermalErosion::Apply(TArray<float>& Heightmap, int32 Width, int32 Height,
                                   const FForge2ThermalErosionParams& Params)
{
    for (int32 Iter = 0; Iter < Params.Iterations; ++Iter)
    {
        float MaterialMoved = ApplyIteration(Heightmap, Width, Height, Params);

        // Early exit if minimal change (converged)
        if (MaterialMoved < 0.0001f)
        {
            break;
        }
    }
}

float FForge2ThermalErosion::ApplyIteration(TArray<float>& Heightmap, int32 Width, int32 Height,
                                             const FForge2ThermalErosionParams& Params)
{
    // Convert talus angle to slope threshold
    float TalusSlope = FMath::Tan(FMath::DegreesToRadians(Params.TalusAngle));

    float TotalMaterialMoved = 0.0f;

    // Create delta buffer to accumulate changes
    TArray<float> Delta;
    Delta.SetNumZeroed(Width * Height);

    // Process each cell
    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            int32 Idx = Y * Width + X;
            float CurrentHeight = Heightmap[Idx];

            // Find neighbors below talus angle and accumulate slope differences
            float TotalSlopeDiff = 0.0f;
            float MaxSlope = 0.0f;
            int32 NeighborCount = 0;

            struct NeighborInfo
            {
                int32 Index;
                float SlopeDiff;
            };
            TArray<NeighborInfo> ValidNeighbors;

            for (int32 Dir = 0; Dir < 8; ++Dir)
            {
                int32 Nx = X + DirX[Dir];
                int32 Ny = Y + DirY[Dir];

                if (!IsValidCoord(Nx, Ny, Width, Height))
                {
                    continue;
                }

                int32 NIdx = Ny * Width + Nx;
                float NeighborHeight = Heightmap[NIdx];
                float HeightDiff = CurrentHeight - NeighborHeight;
                float Distance = Params.CellSize * DirDist[Dir];
                float Slope = HeightDiff / Distance;

                if (Slope > TalusSlope)
                {
                    float SlopeDiff = Slope - TalusSlope;
                    ValidNeighbors.Add({NIdx, SlopeDiff});
                    TotalSlopeDiff += SlopeDiff;
                    MaxSlope = FMath::Max(MaxSlope, Slope);
                }
            }

            // Distribute material to neighbors proportionally
            if (TotalSlopeDiff > 0.0f && ValidNeighbors.Num() > 0)
            {
                // Calculate amount to move based on max excess slope
                float ExcessHeight = (MaxSlope - TalusSlope) * Params.CellSize;
                float AmountToMove = ExcessHeight * Params.ErosionStrength;

                // Distribute to each valid neighbor based on slope difference
                for (const NeighborInfo& Neighbor : ValidNeighbors)
                {
                    float Proportion = Neighbor.SlopeDiff / TotalSlopeDiff;
                    float Transfer = AmountToMove * Proportion;

                    Delta[Idx] -= Transfer;
                    Delta[Neighbor.Index] += Transfer;
                    TotalMaterialMoved += Transfer;
                }
            }
        }
    }

    // Apply accumulated changes
    for (int32 i = 0; i < Width * Height; ++i)
    {
        Heightmap[i] += Delta[i];
    }

    return TotalMaterialMoved;
}

// ============================================================================
// Hydraulic Erosion Implementation
// ============================================================================

bool FForge2HydraulicErosion::IsValidCoord(int32 X, int32 Y, int32 Width, int32 Height) const
{
    return X >= 0 && X < Width && Y >= 0 && Y < Height;
}

float FForge2HydraulicErosion::SampleHeight(const TArray<float>& Heightmap, int32 Width, int32 Height,
                                             float X, float Y) const
{
    // Bilinear interpolation
    int32 X0 = FMath::FloorToInt(X);
    int32 Y0 = FMath::FloorToInt(Y);
    int32 X1 = X0 + 1;
    int32 Y1 = Y0 + 1;

    // Clamp to valid range
    X0 = FMath::Clamp(X0, 0, Width - 1);
    X1 = FMath::Clamp(X1, 0, Width - 1);
    Y0 = FMath::Clamp(Y0, 0, Height - 1);
    Y1 = FMath::Clamp(Y1, 0, Height - 1);

    float Fx = X - FMath::FloorToFloat(X);
    float Fy = Y - FMath::FloorToFloat(Y);

    float H00 = Heightmap[Y0 * Width + X0];
    float H10 = Heightmap[Y0 * Width + X1];
    float H01 = Heightmap[Y1 * Width + X0];
    float H11 = Heightmap[Y1 * Width + X1];

    float H0 = FMath::Lerp(H00, H10, Fx);
    float H1 = FMath::Lerp(H01, H11, Fx);

    return FMath::Lerp(H0, H1, Fy);
}

FVector FForge2HydraulicErosion::CalculateHeightAndGradient(const TArray<float>& Heightmap,
                                                             int32 Width, int32 Height,
                                                             float PosX, float PosY) const
{
    int32 X0 = FMath::FloorToInt(PosX);
    int32 Y0 = FMath::FloorToInt(PosY);

    // Clamp coordinates
    X0 = FMath::Clamp(X0, 0, Width - 2);
    Y0 = FMath::Clamp(Y0, 0, Height - 2);

    float X = PosX - X0;
    float Y = PosY - Y0;

    // Heights of the four corners
    float H00 = Heightmap[Y0 * Width + X0];
    float H10 = Heightmap[Y0 * Width + X0 + 1];
    float H01 = Heightmap[(Y0 + 1) * Width + X0];
    float H11 = Heightmap[(Y0 + 1) * Width + X0 + 1];

    // Calculate gradient using bilinear interpolation
    float GradX = (H10 - H00) * (1 - Y) + (H11 - H01) * Y;
    float GradY = (H01 - H00) * (1 - X) + (H11 - H10) * X;

    // Calculate interpolated height
    float Height = H00 * (1 - X) * (1 - Y) +
                   H10 * X * (1 - Y) +
                   H01 * (1 - X) * Y +
                   H11 * X * Y;

    return FVector(Height, GradX, GradY);
}

void FForge2HydraulicErosion::InitializeErosionBrush(int32 Radius)
{
    if (ErosionBrushWeights.Contains(Radius))
    {
        return; // Already initialized
    }

    TArray<float> Weights;
    TArray<FIntPoint> Indices;
    float WeightSum = 0.0f;

    for (int32 Y = -Radius; Y <= Radius; ++Y)
    {
        for (int32 X = -Radius; X <= Radius; ++X)
        {
            float Dist = FMath::Sqrt(static_cast<float>(X * X + Y * Y));
            if (Dist <= Radius)
            {
                float Weight = FMath::Max(0.0f, Radius - Dist);
                Weights.Add(Weight);
                Indices.Add(FIntPoint(X, Y));
                WeightSum += Weight;
            }
        }
    }

    // Normalize weights
    if (WeightSum > 0.0f)
    {
        for (float& W : Weights)
        {
            W /= WeightSum;
        }
    }

    ErosionBrushWeights.Add(Radius, Weights);
    ErosionBrushIndices.Add(Radius, Indices);
}

const TArray<float>& FForge2HydraulicErosion::GetErosionBrush(int32 Radius)
{
    InitializeErosionBrush(Radius);
    return ErosionBrushWeights[Radius];
}

const TArray<FIntPoint>& FForge2HydraulicErosion::GetErosionBrushIndices(int32 Radius)
{
    InitializeErosionBrush(Radius);
    return ErosionBrushIndices[Radius];
}

void FForge2HydraulicErosion::Apply(TArray<float>& Heightmap, int32 Width, int32 Height,
                                     const FForge2HydraulicErosionParams& Params)
{
    FRandomStream Random(Params.Seed);

    // Pre-initialize erosion brush
    InitializeErosionBrush(Params.ErosionRadius);

    for (int32 i = 0; i < Params.Iterations; ++i)
    {
        SimulateDroplet(Heightmap, Width, Height, Params, Random);
    }
}

void FForge2HydraulicErosion::SimulateDroplet(TArray<float>& Heightmap, int32 Width, int32 Height,
                                               const FForge2HydraulicErosionParams& Params,
                                               FRandomStream& Random)
{
    // Initialize droplet at random position
    float PosX = Random.FRand() * (Width - 1);
    float PosY = Random.FRand() * (Height - 1);
    float DirX = 0.0f;
    float DirY = 0.0f;
    float Speed = Params.InitialSpeed;
    float Water = Params.InitialWaterVolume;
    float Sediment = 0.0f;

    const TArray<float>& BrushWeights = GetErosionBrush(Params.ErosionRadius);
    const TArray<FIntPoint>& BrushIndices = GetErosionBrushIndices(Params.ErosionRadius);

    for (int32 Lifetime = 0; Lifetime < Params.MaxDropletLifetime; ++Lifetime)
    {
        int32 NodeX = FMath::FloorToInt(PosX);
        int32 NodeY = FMath::FloorToInt(PosY);
        int32 DropletIndex = NodeY * Width + NodeX;

        // Calculate droplet's offset inside the cell
        float CellOffsetX = PosX - NodeX;
        float CellOffsetY = PosY - NodeY;

        // Get height and gradient at current position
        FVector HeightAndGradient = CalculateHeightAndGradient(Heightmap, Width, Height, PosX, PosY);
        float CurrentHeight = HeightAndGradient.X;
        float GradX = HeightAndGradient.Y;
        float GradY = HeightAndGradient.Z;

        // Update droplet direction (with inertia)
        DirX = DirX * Params.Inertia - GradX * (1.0f - Params.Inertia);
        DirY = DirY * Params.Inertia - GradY * (1.0f - Params.Inertia);

        // Normalize direction
        float DirLen = FMath::Sqrt(DirX * DirX + DirY * DirY);
        if (DirLen > 0.0001f)
        {
            DirX /= DirLen;
            DirY /= DirLen;
        }
        else
        {
            // Random direction if on flat terrain
            float Angle = Random.FRand() * 2.0f * PI;
            DirX = FMath::Cos(Angle);
            DirY = FMath::Sin(Angle);
        }

        // Calculate new position
        float NewPosX = PosX + DirX;
        float NewPosY = PosY + DirY;

        // Check bounds
        if (NewPosX < 0 || NewPosX >= Width - 1 || NewPosY < 0 || NewPosY >= Height - 1)
        {
            break;
        }

        // Find height at new position
        float NewHeight = SampleHeight(Heightmap, Width, Height, NewPosX, NewPosY);
        float DeltaHeight = NewHeight - CurrentHeight;

        // Calculate sediment capacity
        float SedimentCapacity = FMath::Max(-DeltaHeight * Speed * Water * Params.SedimentCapacityFactor,
                                             Params.MinSedimentCapacity);

        // Erode or deposit depending on whether carrying more than capacity
        if (Sediment > SedimentCapacity || DeltaHeight > 0)
        {
            // Deposit sediment
            float AmountToDeposit = (DeltaHeight > 0) ?
                FMath::Min(DeltaHeight, Sediment) :
                (Sediment - SedimentCapacity) * Params.DepositSpeed;

            Sediment -= AmountToDeposit;

            // Deposit to the four corners of the cell
            int32 X0 = FMath::Clamp(NodeX, 0, Width - 1);
            int32 Y0 = FMath::Clamp(NodeY, 0, Height - 1);
            int32 X1 = FMath::Clamp(NodeX + 1, 0, Width - 1);
            int32 Y1 = FMath::Clamp(NodeY + 1, 0, Height - 1);

            Heightmap[Y0 * Width + X0] += AmountToDeposit * (1 - CellOffsetX) * (1 - CellOffsetY);
            Heightmap[Y0 * Width + X1] += AmountToDeposit * CellOffsetX * (1 - CellOffsetY);
            Heightmap[Y1 * Width + X0] += AmountToDeposit * (1 - CellOffsetX) * CellOffsetY;
            Heightmap[Y1 * Width + X1] += AmountToDeposit * CellOffsetX * CellOffsetY;
        }
        else
        {
            // Erode terrain
            float AmountToErode = FMath::Min((SedimentCapacity - Sediment) * Params.ErodeSpeed, -DeltaHeight);

            // Erode using brush
            for (int32 BrushIdx = 0; BrushIdx < BrushIndices.Num(); ++BrushIdx)
            {
                int32 ErodeX = NodeX + BrushIndices[BrushIdx].X;
                int32 ErodeY = NodeY + BrushIndices[BrushIdx].Y;

                if (ErodeX >= 0 && ErodeX < Width && ErodeY >= 0 && ErodeY < Height)
                {
                    float WeightedAmount = AmountToErode * BrushWeights[BrushIdx];
                    float CurrentHeightAtBrush = Heightmap[ErodeY * Width + ErodeX];
                    float DeltaSediment = FMath::Min(CurrentHeightAtBrush, WeightedAmount);

                    Heightmap[ErodeY * Width + ErodeX] -= DeltaSediment;
                    Sediment += DeltaSediment;
                }
            }
        }

        // Update speed
        Speed = FMath::Sqrt(FMath::Max(0.0f, Speed * Speed + DeltaHeight * Params.Gravity));

        // Evaporate water
        Water *= (1.0f - Params.EvaporateSpeed);

        // Update position
        PosX = NewPosX;
        PosY = NewPosY;

        // Stop if water has evaporated
        if (Water < 0.001f)
        {
            break;
        }
    }
}

// ============================================================================
// Wind Erosion Implementation
// ============================================================================

void FForge2WindErosion::Apply(TArray<float>& Heightmap, int32 Width, int32 Height, const FParams& Params)
{
    FRandomStream Random(Params.Seed);

    for (int32 i = 0; i < Params.Iterations; ++i)
    {
        SimulateParticle(Heightmap, Width, Height, Params, Random);
    }
}

void FForge2WindErosion::SimulateParticle(TArray<float>& Heightmap, int32 Width, int32 Height,
                                           const FParams& Params, FRandomStream& Random)
{
    // Start particle at random position on upwind edge
    float PosX, PosY;

    if (FMath::Abs(Params.WindDirection.X) > FMath::Abs(Params.WindDirection.Y))
    {
        // Wind primarily horizontal
        PosX = (Params.WindDirection.X > 0) ? 0.0f : Width - 1.0f;
        PosY = Random.FRand() * (Height - 1);
    }
    else
    {
        // Wind primarily vertical
        PosX = Random.FRand() * (Width - 1);
        PosY = (Params.WindDirection.Y > 0) ? 0.0f : Height - 1.0f;
    }

    float HeightCarried = 0.0f;
    float ParticleHeight = 0.0f;
    const int32 MaxSteps = FMath::Max(Width, Height) * 2;

    FVector2D Velocity = Params.WindDirection.GetSafeNormal() * Params.WindStrength;

    for (int32 Step = 0; Step < MaxSteps; ++Step)
    {
        int32 X = FMath::FloorToInt(PosX);
        int32 Y = FMath::FloorToInt(PosY);

        if (X < 0 || X >= Width || Y < 0 || Y >= Height)
        {
            break;
        }

        int32 Idx = Y * Width + X;
        float TerrainHeight = Heightmap[Idx];

        // Particle behavior based on height relative to terrain
        if (ParticleHeight < TerrainHeight)
        {
            // On or below terrain - pick up material
            float PickupAmount = Params.SuspensionRate * Params.WindStrength;
            PickupAmount = FMath::Min(PickupAmount, Heightmap[Idx]);

            Heightmap[Idx] -= PickupAmount;
            HeightCarried += PickupAmount;
            ParticleHeight = TerrainHeight;
        }
        else
        {
            // Above terrain - settle material
            float SettleAmount = HeightCarried * Params.SettlingRate;
            HeightCarried -= SettleAmount;
            Heightmap[Idx] += SettleAmount;

            // Gravity pulls particle down
            ParticleHeight -= Params.Gravity;
        }

        // Move particle
        PosX += Velocity.X;
        PosY += Velocity.Y;

        // Add some randomness
        PosX += (Random.FRand() - 0.5f) * 0.5f;
        PosY += (Random.FRand() - 0.5f) * 0.5f;
    }
}

// ============================================================================
// Combined Erosion Processor Implementation
// ============================================================================

void FForge2ErosionProcessor::ApplyErosion(TArray<float>& Heightmap, int32 Width, int32 Height,
                                            const FForge2ThermalErosionParams* ThermalParams,
                                            const FForge2HydraulicErosionParams* HydraulicParams,
                                            const FForge2WindErosion::FParams* WindParams)
{
    if (ThermalParams)
    {
        ThermalErosion.Apply(Heightmap, Width, Height, *ThermalParams);
    }

    if (HydraulicParams)
    {
        HydraulicErosion.Apply(Heightmap, Width, Height, *HydraulicParams);
    }

    if (WindParams)
    {
        WindErosion.Apply(Heightmap, Width, Height, *WindParams);
    }
}

void FForge2ErosionProcessor::ApplyRealisticErosion(TArray<float>& Heightmap, int32 Width, int32 Height,
                                                     int32 Seed, float Intensity)
{
    // Thermal erosion first (smooths sharp features)
    FForge2ThermalErosionParams ThermalParams;
    ThermalParams.Iterations = FMath::RoundToInt(30 * Intensity);
    ThermalParams.TalusAngle = 40.0f;
    ThermalParams.ErosionStrength = 0.5f * Intensity;
    ThermalErosion.Apply(Heightmap, Width, Height, ThermalParams);

    // Hydraulic erosion (carves river valleys)
    FForge2HydraulicErosionParams HydraulicParams;
    HydraulicParams.Seed = Seed;
    HydraulicParams.Iterations = FMath::RoundToInt(Width * Height * 0.5f * Intensity);
    HydraulicParams.MaxDropletLifetime = 64;
    HydraulicParams.Inertia = 0.05f;
    HydraulicParams.SedimentCapacityFactor = 4.0f;
    HydraulicParams.ErodeSpeed = 0.3f * Intensity;
    HydraulicParams.DepositSpeed = 0.3f;
    HydraulicParams.EvaporateSpeed = 0.01f;
    HydraulicParams.ErosionRadius = 3;
    HydraulicErosion.Apply(Heightmap, Width, Height, HydraulicParams);

    // Light thermal pass to smooth erosion artifacts
    ThermalParams.Iterations = FMath::RoundToInt(10 * Intensity);
    ThermalParams.ErosionStrength = 0.3f * Intensity;
    ThermalErosion.Apply(Heightmap, Width, Height, ThermalParams);
}

void FForge2ErosionProcessor::ApplyQuickErosion(TArray<float>& Heightmap, int32 Width, int32 Height,
                                                 int32 Seed, float Intensity)
{
    // Quick thermal erosion only
    FForge2ThermalErosionParams ThermalParams;
    ThermalParams.Iterations = FMath::RoundToInt(20 * Intensity);
    ThermalParams.TalusAngle = 35.0f;
    ThermalParams.ErosionStrength = 0.6f * Intensity;
    ThermalErosion.Apply(Heightmap, Width, Height, ThermalParams);

    // Fewer hydraulic iterations
    FForge2HydraulicErosionParams HydraulicParams;
    HydraulicParams.Seed = Seed;
    HydraulicParams.Iterations = FMath::RoundToInt(Width * Height * 0.1f * Intensity);
    HydraulicParams.MaxDropletLifetime = 32;
    HydraulicParams.ErosionRadius = 2;
    HydraulicErosion.Apply(Heightmap, Width, Height, HydraulicParams);
}

// ============================================================================
// Flow Map Generator Implementation
// ============================================================================

FVector2D FForge2FlowMapGenerator::GetFlowDirection(const TArray<float>& Heightmap,
                                                     int32 Width, int32 Height, int32 X, int32 Y)
{
    float CurrentHeight = Heightmap[Y * Width + X];
    float MaxSlope = 0.0f;
    FVector2D FlowDir = FVector2D::ZeroVector;

    // Check 8 neighbors
    static const int32 DX[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static const int32 DY[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static const float Dist[] = {1.414f, 1.0f, 1.414f, 1.0f, 1.0f, 1.414f, 1.0f, 1.414f};

    for (int32 i = 0; i < 8; ++i)
    {
        int32 Nx = X + DX[i];
        int32 Ny = Y + DY[i];

        if (Nx >= 0 && Nx < Width && Ny >= 0 && Ny < Height)
        {
            float NeighborHeight = Heightmap[Ny * Width + Nx];
            float Slope = (CurrentHeight - NeighborHeight) / Dist[i];

            if (Slope > MaxSlope)
            {
                MaxSlope = Slope;
                FlowDir = FVector2D(DX[i], DY[i]).GetSafeNormal();
            }
        }
    }

    return FlowDir;
}

TArray<FVector2D> FForge2FlowMapGenerator::GenerateFlowMap(const TArray<float>& Heightmap,
                                                            int32 Width, int32 Height)
{
    TArray<FVector2D> FlowMap;
    FlowMap.SetNum(Width * Height);

    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            FlowMap[Y * Width + X] = GetFlowDirection(Heightmap, Width, Height, X, Y);
        }
    }

    return FlowMap;
}

void FForge2FlowMapGenerator::TraceFlow(const TArray<FVector2D>& FlowMap, TArray<float>& Accumulation,
                                         int32 Width, int32 Height, int32 StartX, int32 StartY)
{
    int32 X = StartX;
    int32 Y = StartY;
    TSet<int32> Visited;

    while (X >= 0 && X < Width && Y >= 0 && Y < Height)
    {
        int32 Idx = Y * Width + X;

        if (Visited.Contains(Idx))
        {
            break; // Avoid infinite loops
        }

        Visited.Add(Idx);
        Accumulation[Idx] += 1.0f;

        FVector2D Flow = FlowMap[Idx];
        if (Flow.IsNearlyZero())
        {
            break; // No flow direction (flat or pit)
        }

        X += FMath::RoundToInt(Flow.X);
        Y += FMath::RoundToInt(Flow.Y);
    }
}

TArray<float> FForge2FlowMapGenerator::GenerateFlowAccumulation(const TArray<float>& Heightmap,
                                                                  int32 Width, int32 Height)
{
    TArray<FVector2D> FlowMap = GenerateFlowMap(Heightmap, Width, Height);

    TArray<float> Accumulation;
    Accumulation.SetNumZeroed(Width * Height);

    // Trace flow from each cell
    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            TraceFlow(FlowMap, Accumulation, Width, Height, X, Y);
        }
    }

    return Accumulation;
}

TArray<TArray<FVector2D>> FForge2FlowMapGenerator::FindRiverPaths(const TArray<float>& FlowAccumulation,
                                                                    int32 Width, int32 Height, float Threshold)
{
    TArray<TArray<FVector2D>> Rivers;
    TSet<int32> ProcessedCells;

    // Find river source points (high accumulation cells not already processed)
    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            int32 Idx = Y * Width + X;

            if (FlowAccumulation[Idx] >= Threshold && !ProcessedCells.Contains(Idx))
            {
                // Start tracing a river
                TArray<FVector2D> RiverPath;
                int32 CurX = X;
                int32 CurY = Y;

                while (CurX >= 0 && CurX < Width && CurY >= 0 && CurY < Height)
                {
                    int32 CurIdx = CurY * Width + CurX;

                    if (ProcessedCells.Contains(CurIdx))
                    {
                        break;
                    }

                    if (FlowAccumulation[CurIdx] < Threshold)
                    {
                        break;
                    }

                    ProcessedCells.Add(CurIdx);
                    RiverPath.Add(FVector2D(CurX, CurY));

                    // Find next cell with highest flow that flows here
                    // (simplified - just go to highest accumulation neighbor that's lower)
                    float MaxAccum = 0.0f;
                    int32 NextX = -1, NextY = -1;

                    static const int32 DX[] = {-1, 0, 1, -1, 1, -1, 0, 1};
                    static const int32 DY[] = {-1, -1, -1, 0, 0, 1, 1, 1};

                    for (int32 i = 0; i < 8; ++i)
                    {
                        int32 Nx = CurX + DX[i];
                        int32 Ny = CurY + DY[i];

                        if (Nx >= 0 && Nx < Width && Ny >= 0 && Ny < Height)
                        {
                            int32 NIdx = Ny * Width + Nx;
                            if (FlowAccumulation[NIdx] > MaxAccum && !ProcessedCells.Contains(NIdx))
                            {
                                MaxAccum = FlowAccumulation[NIdx];
                                NextX = Nx;
                                NextY = Ny;
                            }
                        }
                    }

                    if (NextX < 0)
                    {
                        break;
                    }

                    CurX = NextX;
                    CurY = NextY;
                }

                if (RiverPath.Num() > 2)
                {
                    Rivers.Add(RiverPath);
                }
            }
        }
    }

    return Rivers;
}
