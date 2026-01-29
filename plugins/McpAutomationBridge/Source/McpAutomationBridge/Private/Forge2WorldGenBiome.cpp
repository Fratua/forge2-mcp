// Forge2WorldGenBiome.cpp
// FORGE2 Ultimate AAA Game Development MCP Extension
// Biome System Implementation

#include "Forge2WorldGenBiome.h"

// ============================================================================
// FForge2ClimateModeler Implementation
// ============================================================================

FForge2ClimateModeler::FForge2ClimateModeler()
{
}

TArray<FForge2ClimateData> FForge2ClimateModeler::CalculateClimate(
    const TArray<float>& HeightMap,
    int32 Width,
    int32 Height,
    float CellSize,
    const FForge2ClimateParams& Params,
    const TArray<FVector>& WaterBodies)
{
    TArray<FForge2ClimateData> ClimateData;
    ClimateData.SetNum(Width * Height);

    NoiseGenerator.SetSeed(Params.Seed);

    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            FVector2D Position(X * CellSize, Y * CellSize);
            float Elevation = HeightMap[Index];
            float Slope = CalculateSlope(X, Y, HeightMap, Width, Height, CellSize);

            ClimateData[Index] = CalculateClimateAtPoint(Position, Elevation, Slope, Params, WaterBodies);
        }
    }

    return ClimateData;
}

FForge2ClimateData FForge2ClimateModeler::CalculateClimateAtPoint(
    FVector2D Position,
    float Elevation,
    float Slope,
    const FForge2ClimateParams& Params,
    const TArray<FVector>& WaterBodies)
{
    FForge2ClimateData Climate;

    Climate.Elevation = Elevation;
    Climate.Slope = Slope;
    Climate.DistanceToWater = CalculateDistanceToWater(Position, WaterBodies);
    Climate.Temperature = CalculateTemperature(Position, Elevation, Params);
    Climate.Moisture = CalculateMoisture(Position, Elevation, Params, WaterBodies);

    // Calculate latitude (normalized 0-1 from equator)
    Climate.Latitude = FMath::Abs(Position.Y - Params.EquatorY) / (Params.WorldSizeY * 0.5f);
    Climate.Latitude = FMath::Clamp(Climate.Latitude, 0.0f, 1.0f);

    // Determine climate zone
    Climate.ClimateZone = GetClimateZone(Climate.Temperature);

    return Climate;
}

EForge2ClimateZone FForge2ClimateModeler::GetClimateZone(float Temperature)
{
    if (Temperature < -10.0f)
        return EForge2ClimateZone::Arctic;
    else if (Temperature < 5.0f)
        return EForge2ClimateZone::Subarctic;
    else if (Temperature < 18.0f)
        return EForge2ClimateZone::Temperate;
    else if (Temperature < 25.0f)
        return EForge2ClimateZone::Subtropical;
    else
        return EForge2ClimateZone::Tropical;
}

float FForge2ClimateModeler::CalculateTemperature(FVector2D Position, float Elevation, const FForge2ClimateParams& Params)
{
    // Base temperature at equator
    float Temperature = Params.BaseTemperatureAtEquator;

    // Reduce temperature based on latitude (distance from equator)
    float Latitude = FMath::Abs(Position.Y - Params.EquatorY) / (Params.WorldSizeY * 0.5f);
    Latitude = FMath::Clamp(Latitude, 0.0f, 1.0f);
    Temperature -= Latitude * 90.0f * Params.TemperatureDropPerLatitude;

    // Reduce temperature based on elevation
    float ElevationAboveSeaLevel = FMath::Max(0.0f, Elevation - Params.SeaLevel);
    Temperature -= ElevationAboveSeaLevel * Params.TemperatureDropPerMeterElevation;

    // Add noise for variation
    float NoiseValue = NoiseGenerator.Noise2D(Position.X * Params.TemperatureNoiseScale, Position.Y * Params.TemperatureNoiseScale);
    Temperature += NoiseValue * Params.TemperatureNoiseStrength;

    return Temperature;
}

float FForge2ClimateModeler::CalculateMoisture(FVector2D Position, float Elevation, const FForge2ClimateParams& Params, const TArray<FVector>& WaterBodies)
{
    float Moisture = 0.0f;

    // Base moisture from nearby water
    float DistanceToWater = CalculateDistanceToWater(Position, WaterBodies);
    if (DistanceToWater < Params.MoistureDecayDistance)
    {
        float DistanceFactor = 1.0f - (DistanceToWater / Params.MoistureDecayDistance);
        Moisture += Params.BaseMoistureNearWater * DistanceFactor;
    }

    // Wind-carried moisture (rain shadow effect)
    // Cells downwind of water get more moisture, cells behind mountains get less
    for (const FVector& WaterBody : WaterBodies)
    {
        FVector2D ToPosition = Position - FVector2D(WaterBody.X, WaterBody.Y);
        float Distance = ToPosition.Size();

        if (Distance > 0.0f && Distance < Params.MoistureDecayDistance * 2.0f)
        {
            // Check if position is downwind from water
            FVector2D ToPositionNorm = ToPosition.GetSafeNormal();
            float DotProduct = FVector2D::DotProduct(ToPositionNorm, Params.PrevailingWindDirection);

            if (DotProduct > 0.0f) // Downwind
            {
                float WindFactor = DotProduct * (1.0f - Distance / (Params.MoistureDecayDistance * 2.0f));
                Moisture += Params.WindMoistureCarry * WindFactor;
            }
        }
    }

    // Higher elevations tend to be drier (rain shadow)
    float ElevationAboveSeaLevel = FMath::Max(0.0f, Elevation - Params.SeaLevel);
    if (ElevationAboveSeaLevel > 1000.0f)
    {
        Moisture -= (ElevationAboveSeaLevel - 1000.0f) * 0.0001f;
    }

    // Add moisture from rain based on climate zone
    // More rain in temperate and tropical zones
    float Latitude = FMath::Abs(Position.Y - Params.EquatorY) / (Params.WorldSizeY * 0.5f);
    if (Latitude < 0.3f) // Tropical
    {
        Moisture += Params.MoistureFromRain * 0.8f;
    }
    else if (Latitude < 0.6f) // Temperate
    {
        Moisture += Params.MoistureFromRain * 0.5f;
    }

    // Add noise
    float NoiseValue = NoiseGenerator.Noise2D(Position.X * Params.MoistureNoiseScale, Position.Y * Params.MoistureNoiseScale);
    Moisture += NoiseValue * Params.MoistureNoiseStrength;

    return FMath::Clamp(Moisture, 0.0f, 1.0f);
}

float FForge2ClimateModeler::CalculateDistanceToWater(FVector2D Position, const TArray<FVector>& WaterBodies)
{
    float MinDistance = MAX_FLT;

    for (const FVector& WaterBody : WaterBodies)
    {
        float Distance = FVector2D::Distance(Position, FVector2D(WaterBody.X, WaterBody.Y));
        MinDistance = FMath::Min(MinDistance, Distance);
    }

    return MinDistance;
}

float FForge2ClimateModeler::CalculateSlope(int32 X, int32 Y, const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize)
{
    if (X <= 0 || X >= Width - 1 || Y <= 0 || Y >= Height - 1)
        return 0.0f;

    int32 Index = Y * Width + X;
    float Center = HeightMap[Index];
    float Left = HeightMap[Index - 1];
    float Right = HeightMap[Index + 1];
    float Down = HeightMap[Index - Width];
    float Up = HeightMap[Index + Width];

    float DX = (Right - Left) / (2.0f * CellSize);
    float DY = (Up - Down) / (2.0f * CellSize);

    float Slope = FMath::Atan(FMath::Sqrt(DX * DX + DY * DY));
    return FMath::RadiansToDegrees(Slope);
}

// ============================================================================
// FForge2BiomeDistributor Implementation
// ============================================================================

FForge2BiomeDistributor::FForge2BiomeDistributor()
{
}

FForge2BiomeMapData FForge2BiomeDistributor::GenerateBiomeMap(
    const TArray<FForge2ClimateData>& ClimateData,
    int32 Width,
    int32 Height,
    float CellSize,
    FVector2D Origin,
    const TArray<FForge2BiomeDefinition>& Biomes)
{
    FForge2BiomeMapData BiomeMap;
    BiomeMap.Width = Width;
    BiomeMap.Height = Height;
    BiomeMap.CellSize = CellSize;
    BiomeMap.Origin = Origin;
    BiomeMap.ClimateData = ClimateData;
    BiomeMap.BiomeIndices.SetNum(Width * Height);
    BiomeMap.BlendData.SetNum(Width * Height);

    // First pass: assign biomes based on climate
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            BiomeMap.BiomeIndices[Index] = FindBiomeForClimate(ClimateData[Index], Biomes);
        }
    }

    // Second pass: calculate blend data for transitions
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            int32 Index = Y * Width + X;
            BiomeMap.BlendData[Index] = CalculateBlend(X, Y, BiomeMap.BiomeIndices, Width, Height, Biomes, 500.0f, CellSize);
        }
    }

    return BiomeMap;
}

int32 FForge2BiomeDistributor::FindBiomeForClimate(const FForge2ClimateData& Climate, const TArray<FForge2BiomeDefinition>& Biomes)
{
    int32 BestBiome = 0;
    float BestScore = -MAX_FLT;

    for (int32 i = 0; i < Biomes.Num(); i++)
    {
        float Score = CalculateBiomeMatchScore(Climate, Biomes[i]);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestBiome = i;
        }
    }

    return BestBiome;
}

FForge2BiomeBlend FForge2BiomeDistributor::CalculateBlend(
    int32 X,
    int32 Y,
    const TArray<int32>& BiomeIndices,
    int32 Width,
    int32 Height,
    const TArray<FForge2BiomeDefinition>& Biomes,
    float BlendDistance,
    float CellSize)
{
    FForge2BiomeBlend Blend;

    int32 Index = Y * Width + X;
    int32 CenterBiome = BiomeIndices[Index];
    Blend.PrimaryBiomeIndex = CenterBiome;

    // Search for nearby different biomes
    int32 SearchRadius = FMath::CeilToInt(BlendDistance / CellSize);
    TMap<int32, float> BiomeWeights;
    BiomeWeights.Add(CenterBiome, 1.0f);

    for (int32 DY = -SearchRadius; DY <= SearchRadius; DY++)
    {
        for (int32 DX = -SearchRadius; DX <= SearchRadius; DX++)
        {
            if (DX == 0 && DY == 0)
                continue;

            int32 NX = X + DX;
            int32 NY = Y + DY;

            if (NX < 0 || NX >= Width || NY < 0 || NY >= Height)
                continue;

            int32 NeighborIndex = NY * Width + NX;
            int32 NeighborBiome = BiomeIndices[NeighborIndex];

            if (NeighborBiome != CenterBiome)
            {
                float Distance = FMath::Sqrt((float)(DX * DX + DY * DY)) * CellSize;
                if (Distance < BlendDistance)
                {
                    float Weight = 1.0f - (Distance / BlendDistance);
                    Weight *= Weight; // Smooth falloff

                    float* ExistingWeight = BiomeWeights.Find(NeighborBiome);
                    if (ExistingWeight)
                    {
                        *ExistingWeight = FMath::Max(*ExistingWeight, Weight);
                    }
                    else
                    {
                        BiomeWeights.Add(NeighborBiome, Weight);
                    }
                }
            }
        }
    }

    // Normalize weights and find secondary biome
    float TotalWeight = 0.0f;
    for (const auto& Pair : BiomeWeights)
    {
        TotalWeight += Pair.Value;
    }

    Blend.SecondaryBiomeIndex = -1;
    float MaxSecondaryWeight = 0.0f;

    for (const auto& Pair : BiomeWeights)
    {
        float NormalizedWeight = Pair.Value / TotalWeight;

        if (Pair.Key != CenterBiome && NormalizedWeight > MaxSecondaryWeight)
        {
            MaxSecondaryWeight = NormalizedWeight;
            Blend.SecondaryBiomeIndex = Pair.Key;
            Blend.BlendFactor = NormalizedWeight;
        }

        Blend.NearbyBiomeIndices.Add(Pair.Key);
        Blend.NearbyBiomeWeights.Add(NormalizedWeight);
    }

    return Blend;
}

FForge2BiomeBlend FForge2BiomeDistributor::GetBiomeAtPosition(
    FVector2D WorldPos,
    const FForge2BiomeMapData& BiomeMap,
    const TArray<FForge2BiomeDefinition>& Biomes)
{
    FIntPoint Cell = BiomeMap.GetCell(WorldPos);

    if (Cell.X < 0 || Cell.X >= BiomeMap.Width || Cell.Y < 0 || Cell.Y >= BiomeMap.Height)
    {
        FForge2BiomeBlend Empty;
        Empty.PrimaryBiomeIndex = 0;
        return Empty;
    }

    int32 Index = Cell.Y * BiomeMap.Width + Cell.X;
    return BiomeMap.BlendData[Index];
}

float FForge2BiomeDistributor::CalculateBiomeMatchScore(const FForge2ClimateData& Climate, const FForge2BiomeDefinition& Biome)
{
    float Score = 0.0f;

    // Temperature match
    if (Climate.Temperature >= Biome.MinTemperature && Climate.Temperature <= Biome.MaxTemperature)
    {
        float TempRange = Biome.MaxTemperature - Biome.MinTemperature;
        if (TempRange > 0.0f)
        {
            float TempCenter = (Biome.MinTemperature + Biome.MaxTemperature) * 0.5f;
            float TempDistance = FMath::Abs(Climate.Temperature - TempCenter) / (TempRange * 0.5f);
            Score += (1.0f - TempDistance) * 10.0f;
        }
        else
        {
            Score += 10.0f;
        }
    }
    else
    {
        // Penalty for being outside temperature range
        float TempDistance = 0.0f;
        if (Climate.Temperature < Biome.MinTemperature)
            TempDistance = Biome.MinTemperature - Climate.Temperature;
        else
            TempDistance = Climate.Temperature - Biome.MaxTemperature;
        Score -= TempDistance;
    }

    // Moisture match
    if (Climate.Moisture >= Biome.MinMoisture && Climate.Moisture <= Biome.MaxMoisture)
    {
        float MoistureRange = Biome.MaxMoisture - Biome.MinMoisture;
        if (MoistureRange > 0.0f)
        {
            float MoistureCenter = (Biome.MinMoisture + Biome.MaxMoisture) * 0.5f;
            float MoistureDistance = FMath::Abs(Climate.Moisture - MoistureCenter) / (MoistureRange * 0.5f);
            Score += (1.0f - MoistureDistance) * 10.0f;
        }
        else
        {
            Score += 10.0f;
        }
    }
    else
    {
        float MoistureDistance = 0.0f;
        if (Climate.Moisture < Biome.MinMoisture)
            MoistureDistance = (Biome.MinMoisture - Climate.Moisture) * 20.0f;
        else
            MoistureDistance = (Climate.Moisture - Biome.MaxMoisture) * 20.0f;
        Score -= MoistureDistance;
    }

    // Elevation match
    if (Climate.Elevation >= Biome.MinElevation && Climate.Elevation <= Biome.MaxElevation)
    {
        Score += 5.0f;
    }
    else
    {
        Score -= 10.0f;
    }

    // Priority bonus
    Score += Biome.Priority * 0.5f;

    return Score;
}

TArray<FIntPoint> FForge2BiomeDistributor::FindNearbyDifferentBiomes(
    int32 X,
    int32 Y,
    const TArray<int32>& BiomeIndices,
    int32 Width,
    int32 Height,
    int32 SearchRadius)
{
    TArray<FIntPoint> DifferentCells;

    int32 Index = Y * Width + X;
    int32 CenterBiome = BiomeIndices[Index];

    for (int32 DY = -SearchRadius; DY <= SearchRadius; DY++)
    {
        for (int32 DX = -SearchRadius; DX <= SearchRadius; DX++)
        {
            if (DX == 0 && DY == 0)
                continue;

            int32 NX = X + DX;
            int32 NY = Y + DY;

            if (NX < 0 || NX >= Width || NY < 0 || NY >= Height)
                continue;

            int32 NeighborIndex = NY * Width + NX;
            if (BiomeIndices[NeighborIndex] != CenterBiome)
            {
                DifferentCells.Add(FIntPoint(NX, NY));
            }
        }
    }

    return DifferentCells;
}

// ============================================================================
// FForge2BiomeBlender Implementation
// ============================================================================

FForge2BiomeBlender::FForge2BiomeBlender()
{
}

void FForge2BiomeBlender::SmoothBiomeEdges(
    FForge2BiomeMapData& BiomeMap,
    const TArray<FForge2BiomeDefinition>& Biomes,
    float BlendDistance)
{
    // Already handled in CalculateBlend during generation
    // This function could be used for additional smoothing passes
}

TArray<FForge2BiomeVegetation> FForge2BiomeBlender::GetBlendedVegetation(
    const FForge2BiomeBlend& Blend,
    const TArray<FForge2BiomeDefinition>& Biomes)
{
    TArray<FForge2BiomeVegetation> BlendedVegetation;

    if (Blend.PrimaryBiomeIndex < 0 || Blend.PrimaryBiomeIndex >= Biomes.Num())
        return BlendedVegetation;

    const FForge2BiomeDefinition& PrimaryBiome = Biomes[Blend.PrimaryBiomeIndex];

    // Add primary biome vegetation with full density
    for (const FForge2BiomeVegetation& Veg : PrimaryBiome.Vegetation)
    {
        FForge2BiomeVegetation BlendedVeg = Veg;
        BlendedVeg.Density *= (1.0f - Blend.BlendFactor);
        BlendedVegetation.Add(BlendedVeg);
    }

    // Add secondary biome vegetation with blend factor
    if (Blend.SecondaryBiomeIndex >= 0 && Blend.SecondaryBiomeIndex < Biomes.Num())
    {
        const FForge2BiomeDefinition& SecondaryBiome = Biomes[Blend.SecondaryBiomeIndex];
        for (const FForge2BiomeVegetation& Veg : SecondaryBiome.Vegetation)
        {
            FForge2BiomeVegetation BlendedVeg = Veg;
            BlendedVeg.Density *= Blend.BlendFactor;
            BlendedVegetation.Add(BlendedVeg);
        }
    }

    return BlendedVegetation;
}

FString FForge2BiomeBlender::GetBlendedMaterialPath(
    const FForge2BiomeBlend& Blend,
    const TArray<FForge2BiomeDefinition>& Biomes)
{
    if (Blend.PrimaryBiomeIndex < 0 || Blend.PrimaryBiomeIndex >= Biomes.Num())
        return FString();

    // For now, return primary biome material
    // In practice, you'd use a material with blend parameters
    return Biomes[Blend.PrimaryBiomeIndex].GroundMaterialPath;
}

void FForge2BiomeBlender::GetBlendedFogSettings(
    const FForge2BiomeBlend& Blend,
    const TArray<FForge2BiomeDefinition>& Biomes,
    FLinearColor& OutFogColor,
    float& OutFogDensity)
{
    if (Blend.PrimaryBiomeIndex < 0 || Blend.PrimaryBiomeIndex >= Biomes.Num())
    {
        OutFogColor = FLinearColor(0.5f, 0.5f, 0.6f);
        OutFogDensity = 0.001f;
        return;
    }

    const FForge2BiomeDefinition& PrimaryBiome = Biomes[Blend.PrimaryBiomeIndex];
    OutFogColor = PrimaryBiome.FogColor;
    OutFogDensity = PrimaryBiome.FogDensity;

    if (Blend.SecondaryBiomeIndex >= 0 && Blend.SecondaryBiomeIndex < Biomes.Num() && Blend.BlendFactor > 0.0f)
    {
        const FForge2BiomeDefinition& SecondaryBiome = Biomes[Blend.SecondaryBiomeIndex];
        OutFogColor = FMath::Lerp(PrimaryBiome.FogColor, SecondaryBiome.FogColor, Blend.BlendFactor);
        OutFogDensity = FMath::Lerp(PrimaryBiome.FogDensity, SecondaryBiome.FogDensity, Blend.BlendFactor);
    }
}

// ============================================================================
// FForge2BiomeProcessor Implementation
// ============================================================================

FForge2BiomeProcessor::FForge2BiomeProcessor()
{
}

FForge2BiomeMapData FForge2BiomeProcessor::GenerateFromHeightmap(
    const TArray<float>& HeightMap,
    int32 Width,
    int32 Height,
    float CellSize,
    FVector2D Origin,
    const FForge2BiomeMapParams& Params,
    const TArray<FVector>& WaterBodies)
{
    // Calculate climate data
    TArray<FForge2ClimateData> ClimateData = ClimateModeler.CalculateClimate(
        HeightMap,
        Width,
        Height,
        CellSize,
        Params.ClimateParams,
        WaterBodies
    );

    // Generate biome map from climate
    FForge2BiomeMapData BiomeMap = BiomeDistributor.GenerateBiomeMap(
        ClimateData,
        Width,
        Height,
        CellSize,
        Origin,
        Params.Biomes
    );

    // Smooth edges if requested
    if (Params.bGenerateTransitionZones)
    {
        BiomeBlender.SmoothBiomeEdges(BiomeMap, Params.Biomes, Params.BlendDistance);
    }

    return BiomeMap;
}

void FForge2BiomeProcessor::ApplyBiomeToRegion(
    FForge2BiomeMapData& BiomeMap,
    int32 BiomeIndex,
    FIntRect Region,
    float Strength)
{
    for (int32 Y = Region.Min.Y; Y < Region.Max.Y; Y++)
    {
        for (int32 X = Region.Min.X; X < Region.Max.X; X++)
        {
            int32 Index = BiomeMap.GetIndex(X, Y);
            if (Index < 0)
                continue;

            // Blend with existing biome based on strength
            if (Strength >= 1.0f)
            {
                BiomeMap.BiomeIndices[Index] = BiomeIndex;
            }
            else
            {
                // Keep as secondary blend
                BiomeMap.BlendData[Index].SecondaryBiomeIndex = BiomeIndex;
                BiomeMap.BlendData[Index].BlendFactor = Strength;
            }
        }
    }
}

FForge2BiomeBlend FForge2BiomeProcessor::QueryBiomeAtPosition(
    FVector2D WorldPos,
    const FForge2BiomeMapData& BiomeMap,
    const TArray<FForge2BiomeDefinition>& Biomes)
{
    return BiomeDistributor.GetBiomeAtPosition(WorldPos, BiomeMap, Biomes);
}

TArray<FColor> FForge2BiomeProcessor::ExportBiomeMapAsImage(
    const FForge2BiomeMapData& BiomeMap,
    const TArray<FForge2BiomeDefinition>& Biomes)
{
    TArray<FColor> ImageData;
    ImageData.SetNum(BiomeMap.Width * BiomeMap.Height);

    for (int32 i = 0; i < BiomeMap.BiomeIndices.Num(); i++)
    {
        int32 BiomeIndex = BiomeMap.BiomeIndices[i];
        if (BiomeIndex >= 0 && BiomeIndex < Biomes.Num())
        {
            ImageData[i] = GetBiomeColor(Biomes[BiomeIndex].BiomeType);
        }
        else
        {
            ImageData[i] = FColor::Black;
        }
    }

    return ImageData;
}

TArray<FForge2BiomeDefinition> FForge2BiomeProcessor::GetDefaultBiomes()
{
    TArray<FForge2BiomeDefinition> Biomes;

    // Ice Cap
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::IceCap));

    // Tundra
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::Tundra));

    // Boreal Forest
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::BorealForest));

    // Temperate Forest
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::TemperateForest));

    // Temperate Grassland
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::TemperateGrassland));

    // Desert
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::Desert));

    // Savanna
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::Savanna));

    // Tropical Rainforest
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::TropicalRainforest));

    // Mountain
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::Mountain));

    // Swamp
    Biomes.Add(FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType::Swamp));

    return Biomes;
}

FColor FForge2BiomeProcessor::GetBiomeColor(EForge2BiomeType BiomeType)
{
    switch (BiomeType)
    {
    case EForge2BiomeType::IceCap: return FColor(240, 250, 255);
    case EForge2BiomeType::Tundra: return FColor(200, 220, 230);
    case EForge2BiomeType::BorealForest: return FColor(40, 80, 60);
    case EForge2BiomeType::TemperateRainforest: return FColor(30, 100, 50);
    case EForge2BiomeType::TemperateForest: return FColor(50, 120, 50);
    case EForge2BiomeType::TemperateGrassland: return FColor(120, 180, 80);
    case EForge2BiomeType::TropicalRainforest: return FColor(20, 80, 30);
    case EForge2BiomeType::TropicalForest: return FColor(40, 100, 40);
    case EForge2BiomeType::Savanna: return FColor(180, 160, 80);
    case EForge2BiomeType::Desert: return FColor(220, 200, 150);
    case EForge2BiomeType::SemiAridDesert: return FColor(200, 180, 120);
    case EForge2BiomeType::Shrubland: return FColor(140, 150, 100);
    case EForge2BiomeType::Mountain: return FColor(120, 100, 90);
    case EForge2BiomeType::SnowMountain: return FColor(230, 240, 250);
    case EForge2BiomeType::Swamp: return FColor(60, 80, 50);
    case EForge2BiomeType::Marsh: return FColor(80, 100, 70);
    case EForge2BiomeType::Beach: return FColor(230, 210, 170);
    case EForge2BiomeType::Ocean: return FColor(30, 80, 150);
    case EForge2BiomeType::River: return FColor(40, 100, 180);
    case EForge2BiomeType::Lake: return FColor(50, 90, 160);
    case EForge2BiomeType::MagicForest: return FColor(80, 50, 120);
    case EForge2BiomeType::CrystalCaverns: return FColor(100, 200, 220);
    case EForge2BiomeType::VolcanicWasteland: return FColor(80, 30, 20);
    case EForge2BiomeType::FrozenWasteland: return FColor(180, 200, 220);
    case EForge2BiomeType::CorruptedLand: return FColor(60, 30, 60);
    default: return FColor::Magenta;
    }
}

// ============================================================================
// FForge2BiomeUtils Implementation
// ============================================================================

EForge2BiomeType FForge2BiomeUtils::WhittakerLookup(float Temperature, float Precipitation)
{
    // Simplified Whittaker diagram implementation
    // Temperature in Celsius, Precipitation in mm/year normalized to 0-1

    if (Temperature < -10.0f)
    {
        return EForge2BiomeType::IceCap;
    }
    else if (Temperature < 0.0f)
    {
        if (Precipitation > 0.3f)
            return EForge2BiomeType::BorealForest;
        else
            return EForge2BiomeType::Tundra;
    }
    else if (Temperature < 15.0f)
    {
        if (Precipitation > 0.7f)
            return EForge2BiomeType::TemperateRainforest;
        else if (Precipitation > 0.4f)
            return EForge2BiomeType::TemperateForest;
        else if (Precipitation > 0.2f)
            return EForge2BiomeType::TemperateGrassland;
        else
            return EForge2BiomeType::Shrubland;
    }
    else if (Temperature < 25.0f)
    {
        if (Precipitation > 0.6f)
            return EForge2BiomeType::TropicalForest;
        else if (Precipitation > 0.3f)
            return EForge2BiomeType::Savanna;
        else if (Precipitation > 0.1f)
            return EForge2BiomeType::SemiAridDesert;
        else
            return EForge2BiomeType::Desert;
    }
    else
    {
        if (Precipitation > 0.7f)
            return EForge2BiomeType::TropicalRainforest;
        else if (Precipitation > 0.4f)
            return EForge2BiomeType::TropicalForest;
        else if (Precipitation > 0.2f)
            return EForge2BiomeType::Savanna;
        else
            return EForge2BiomeType::Desert;
    }
}

FString FForge2BiomeUtils::GetBiomeName(EForge2BiomeType BiomeType)
{
    switch (BiomeType)
    {
    case EForge2BiomeType::IceCap: return TEXT("Ice Cap");
    case EForge2BiomeType::Tundra: return TEXT("Tundra");
    case EForge2BiomeType::BorealForest: return TEXT("Boreal Forest");
    case EForge2BiomeType::TemperateRainforest: return TEXT("Temperate Rainforest");
    case EForge2BiomeType::TemperateForest: return TEXT("Temperate Forest");
    case EForge2BiomeType::TemperateGrassland: return TEXT("Temperate Grassland");
    case EForge2BiomeType::TropicalRainforest: return TEXT("Tropical Rainforest");
    case EForge2BiomeType::TropicalForest: return TEXT("Tropical Forest");
    case EForge2BiomeType::Savanna: return TEXT("Savanna");
    case EForge2BiomeType::Desert: return TEXT("Desert");
    case EForge2BiomeType::SemiAridDesert: return TEXT("Semi-Arid Desert");
    case EForge2BiomeType::Shrubland: return TEXT("Shrubland");
    case EForge2BiomeType::Mountain: return TEXT("Mountain");
    case EForge2BiomeType::SnowMountain: return TEXT("Snow Mountain");
    case EForge2BiomeType::Swamp: return TEXT("Swamp");
    case EForge2BiomeType::Marsh: return TEXT("Marsh");
    case EForge2BiomeType::Beach: return TEXT("Beach");
    case EForge2BiomeType::Ocean: return TEXT("Ocean");
    case EForge2BiomeType::River: return TEXT("River");
    case EForge2BiomeType::Lake: return TEXT("Lake");
    case EForge2BiomeType::MagicForest: return TEXT("Magic Forest");
    case EForge2BiomeType::CrystalCaverns: return TEXT("Crystal Caverns");
    case EForge2BiomeType::VolcanicWasteland: return TEXT("Volcanic Wasteland");
    case EForge2BiomeType::FrozenWasteland: return TEXT("Frozen Wasteland");
    case EForge2BiomeType::CorruptedLand: return TEXT("Corrupted Land");
    case EForge2BiomeType::Custom: return TEXT("Custom");
    default: return TEXT("Unknown");
    }
}

bool FForge2BiomeUtils::IsWaterBiome(EForge2BiomeType BiomeType)
{
    return BiomeType == EForge2BiomeType::Ocean ||
           BiomeType == EForge2BiomeType::River ||
           BiomeType == EForge2BiomeType::Lake ||
           BiomeType == EForge2BiomeType::Swamp ||
           BiomeType == EForge2BiomeType::Marsh;
}

bool FForge2BiomeUtils::IsColdBiome(EForge2BiomeType BiomeType)
{
    return BiomeType == EForge2BiomeType::IceCap ||
           BiomeType == EForge2BiomeType::Tundra ||
           BiomeType == EForge2BiomeType::SnowMountain ||
           BiomeType == EForge2BiomeType::FrozenWasteland;
}

bool FForge2BiomeUtils::SupportsTreesbiome(EForge2BiomeType BiomeType)
{
    switch (BiomeType)
    {
    case EForge2BiomeType::BorealForest:
    case EForge2BiomeType::TemperateRainforest:
    case EForge2BiomeType::TemperateForest:
    case EForge2BiomeType::TropicalRainforest:
    case EForge2BiomeType::TropicalForest:
    case EForge2BiomeType::Swamp:
    case EForge2BiomeType::MagicForest:
        return true;
    default:
        return false;
    }
}

float FForge2BiomeUtils::GetTypicalVegetationDensity(EForge2BiomeType BiomeType)
{
    switch (BiomeType)
    {
    case EForge2BiomeType::TropicalRainforest: return 1.0f;
    case EForge2BiomeType::TemperateRainforest: return 0.9f;
    case EForge2BiomeType::TropicalForest: return 0.8f;
    case EForge2BiomeType::BorealForest: return 0.7f;
    case EForge2BiomeType::TemperateForest: return 0.7f;
    case EForge2BiomeType::Swamp: return 0.6f;
    case EForge2BiomeType::Savanna: return 0.3f;
    case EForge2BiomeType::TemperateGrassland: return 0.4f;
    case EForge2BiomeType::Shrubland: return 0.3f;
    case EForge2BiomeType::Marsh: return 0.4f;
    case EForge2BiomeType::Tundra: return 0.2f;
    case EForge2BiomeType::SemiAridDesert: return 0.1f;
    case EForge2BiomeType::Desert: return 0.05f;
    case EForge2BiomeType::IceCap: return 0.0f;
    case EForge2BiomeType::Mountain: return 0.2f;
    case EForge2BiomeType::SnowMountain: return 0.05f;
    case EForge2BiomeType::MagicForest: return 0.8f;
    default: return 0.3f;
    }
}

float FForge2BiomeUtils::GetTypicalRockDensity(EForge2BiomeType BiomeType)
{
    switch (BiomeType)
    {
    case EForge2BiomeType::Mountain: return 0.5f;
    case EForge2BiomeType::SnowMountain: return 0.4f;
    case EForge2BiomeType::Desert: return 0.3f;
    case EForge2BiomeType::SemiAridDesert: return 0.25f;
    case EForge2BiomeType::Tundra: return 0.2f;
    case EForge2BiomeType::VolcanicWasteland: return 0.4f;
    case EForge2BiomeType::CrystalCaverns: return 0.3f;
    default: return 0.1f;
    }
}

FForge2BiomeDefinition FForge2BiomeUtils::CreateDefaultBiome(EForge2BiomeType BiomeType)
{
    FForge2BiomeDefinition Biome;
    Biome.BiomeType = BiomeType;
    Biome.DisplayName = GetBiomeName(BiomeType);
    Biome.BiomeId = Biome.DisplayName.Replace(TEXT(" "), TEXT("_"));
    Biome.VegetationDensityMultiplier = GetTypicalVegetationDensity(BiomeType);
    Biome.RockDensity = GetTypicalRockDensity(BiomeType);

    switch (BiomeType)
    {
    case EForge2BiomeType::IceCap:
        Biome.MinTemperature = -100.0f;
        Biome.MaxTemperature = -10.0f;
        Biome.MinMoisture = 0.0f;
        Biome.MaxMoisture = 0.3f;
        Biome.FogColor = FLinearColor(0.8f, 0.85f, 0.9f);
        Biome.FogDensity = 0.003f;
        Biome.SnowChance = 0.8f;
        Biome.RainChance = 0.0f;
        break;

    case EForge2BiomeType::Tundra:
        Biome.MinTemperature = -20.0f;
        Biome.MaxTemperature = 5.0f;
        Biome.MinMoisture = 0.1f;
        Biome.MaxMoisture = 0.4f;
        Biome.FogColor = FLinearColor(0.7f, 0.75f, 0.8f);
        Biome.FogDensity = 0.002f;
        Biome.SnowChance = 0.5f;
        Biome.RainChance = 0.1f;
        break;

    case EForge2BiomeType::BorealForest:
        Biome.MinTemperature = -10.0f;
        Biome.MaxTemperature = 10.0f;
        Biome.MinMoisture = 0.3f;
        Biome.MaxMoisture = 0.7f;
        Biome.FogColor = FLinearColor(0.5f, 0.6f, 0.65f);
        Biome.FogDensity = 0.002f;
        Biome.SnowChance = 0.3f;
        Biome.RainChance = 0.3f;
        break;

    case EForge2BiomeType::TemperateForest:
        Biome.MinTemperature = 5.0f;
        Biome.MaxTemperature = 20.0f;
        Biome.MinMoisture = 0.4f;
        Biome.MaxMoisture = 0.8f;
        Biome.FogColor = FLinearColor(0.5f, 0.55f, 0.6f);
        Biome.FogDensity = 0.001f;
        Biome.SnowChance = 0.1f;
        Biome.RainChance = 0.3f;
        break;

    case EForge2BiomeType::TemperateGrassland:
        Biome.MinTemperature = 0.0f;
        Biome.MaxTemperature = 25.0f;
        Biome.MinMoisture = 0.2f;
        Biome.MaxMoisture = 0.5f;
        Biome.FogColor = FLinearColor(0.6f, 0.65f, 0.7f);
        Biome.FogDensity = 0.0005f;
        Biome.RainChance = 0.2f;
        break;

    case EForge2BiomeType::Desert:
        Biome.MinTemperature = 15.0f;
        Biome.MaxTemperature = 50.0f;
        Biome.MinMoisture = 0.0f;
        Biome.MaxMoisture = 0.1f;
        Biome.FogColor = FLinearColor(0.8f, 0.75f, 0.6f);
        Biome.FogDensity = 0.0002f;
        Biome.RainChance = 0.01f;
        Biome.StormChance = 0.05f;
        break;

    case EForge2BiomeType::Savanna:
        Biome.MinTemperature = 20.0f;
        Biome.MaxTemperature = 35.0f;
        Biome.MinMoisture = 0.2f;
        Biome.MaxMoisture = 0.5f;
        Biome.FogColor = FLinearColor(0.7f, 0.65f, 0.5f);
        Biome.FogDensity = 0.0003f;
        Biome.RainChance = 0.2f;
        break;

    case EForge2BiomeType::TropicalRainforest:
        Biome.MinTemperature = 20.0f;
        Biome.MaxTemperature = 35.0f;
        Biome.MinMoisture = 0.7f;
        Biome.MaxMoisture = 1.0f;
        Biome.FogColor = FLinearColor(0.4f, 0.5f, 0.45f);
        Biome.FogDensity = 0.003f;
        Biome.RainChance = 0.7f;
        Biome.FogChance = 0.4f;
        break;

    case EForge2BiomeType::Mountain:
        Biome.MinTemperature = -20.0f;
        Biome.MaxTemperature = 15.0f;
        Biome.MinMoisture = 0.0f;
        Biome.MaxMoisture = 0.6f;
        Biome.MinElevation = 2000.0f;
        Biome.MaxElevation = 10000.0f;
        Biome.FogColor = FLinearColor(0.6f, 0.65f, 0.7f);
        Biome.FogDensity = 0.002f;
        Biome.Priority = 10; // Higher priority for elevation-based biomes
        break;

    case EForge2BiomeType::Swamp:
        Biome.MinTemperature = 10.0f;
        Biome.MaxTemperature = 30.0f;
        Biome.MinMoisture = 0.8f;
        Biome.MaxMoisture = 1.0f;
        Biome.FogColor = FLinearColor(0.4f, 0.45f, 0.35f);
        Biome.FogDensity = 0.005f;
        Biome.FogChance = 0.6f;
        Biome.RainChance = 0.4f;
        break;

    default:
        // Use constructor defaults
        break;
    }

    return Biome;
}
