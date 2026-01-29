// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// City and Settlement Generation - Implementation

#include "Forge2WorldGenCity.h"

// ============================================================================
// District Data Helpers
// ============================================================================

float FForge2DistrictData::GetArea() const
{
    if (Bounds.Num() < 3) return 0.0f;

    float Area = 0.0f;
    int32 N = Bounds.Num();

    for (int32 i = 0; i < N; ++i)
    {
        int32 j = (i + 1) % N;
        Area += Bounds[i].X * Bounds[j].Y;
        Area -= Bounds[j].X * Bounds[i].Y;
    }

    return FMath::Abs(Area) * 0.5f;
}

float FForge2BuildingLot::GetArea() const
{
    if (Bounds.Num() < 3) return 0.0f;

    float Area = 0.0f;
    int32 N = Bounds.Num();

    for (int32 i = 0; i < N; ++i)
    {
        int32 j = (i + 1) % N;
        Area += Bounds[i].X * Bounds[j].Y;
        Area -= Bounds[j].X * Bounds[i].Y;
    }

    return FMath::Abs(Area) * 0.5f;
}

FVector2D FForge2BuildingLot::GetSize() const
{
    if (Bounds.Num() < 2) return FVector2D::ZeroVector;

    FVector2D Min(MAX_FLT, MAX_FLT);
    FVector2D Max(-MAX_FLT, -MAX_FLT);

    for (const FVector2D& V : Bounds)
    {
        Min.X = FMath::Min(Min.X, V.X);
        Min.Y = FMath::Min(Min.Y, V.Y);
        Max.X = FMath::Max(Max.X, V.X);
        Max.Y = FMath::Max(Max.Y, V.Y);
    }

    return Max - Min;
}

// ============================================================================
// L-System Roads
// ============================================================================

FString FForge2LSystemRule::Apply(FRandomStream& Random) const
{
    if (Outputs.Num() == 0) return Input;
    if (Outputs.Num() == 1) return Outputs[0];

    float TotalWeight = 0.0f;
    for (float W : Weights) TotalWeight += W;

    float Roll = Random.FRand() * TotalWeight;
    float Accumulator = 0.0f;

    for (int32 i = 0; i < Outputs.Num(); ++i)
    {
        Accumulator += (i < Weights.Num()) ? Weights[i] : 1.0f;
        if (Roll <= Accumulator)
        {
            return Outputs[i];
        }
    }

    return Outputs.Last();
}

FForge2LSystemRoads::FForge2LSystemRoads()
{
    CurrentAxiom = TEXT("F");
}

void FForge2LSystemRoads::SetAxiom(const FString& Axiom)
{
    CurrentAxiom = Axiom;
}

void FForge2LSystemRoads::AddRule(const FString& Input, const FString& Output, float Weight)
{
    for (FForge2LSystemRule& Rule : Rules)
    {
        if (Rule.Input == Input)
        {
            Rule.Outputs.Add(Output);
            Rule.Weights.Add(Weight);
            return;
        }
    }

    FForge2LSystemRule NewRule;
    NewRule.Input = Input;
    NewRule.Outputs.Add(Output);
    NewRule.Weights.Add(Weight);
    Rules.Add(NewRule);
}

FString FForge2LSystemRoads::Iterate(int32 Iterations, FRandomStream& Random)
{
    FString Result = CurrentAxiom;

    for (int32 Iter = 0; Iter < Iterations; ++Iter)
    {
        FString NewResult;

        for (int32 i = 0; i < Result.Len(); ++i)
        {
            FString Char = Result.Mid(i, 1);
            bool bReplaced = false;

            for (const FForge2LSystemRule& Rule : Rules)
            {
                if (Rule.Input == Char)
                {
                    NewResult += Rule.Apply(Random);
                    bReplaced = true;
                    break;
                }
            }

            if (!bReplaced)
            {
                NewResult += Char;
            }
        }

        Result = NewResult;
    }

    return Result;
}

void FForge2LSystemRoads::InterpretRoads(const FString& LString, FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    TArray<FTurtleState> StateStack;

    FTurtleState State;
    State.Position = City.Center;
    State.Angle = 0.0f;
    State.StepLength = 200.0f;
    State.Width = Params.MainRoadWidth;
    State.RoadType = EForge2RoadType::MainRoad;

    for (int32 i = 0; i < LString.Len(); ++i)
    {
        TCHAR Char = LString[i];

        switch (Char)
        {
            case 'F': // Move forward, draw road
            case 'G':
            {
                FVector2D Direction(FMath::Cos(FMath::DegreesToRadians(State.Angle)),
                                    FMath::Sin(FMath::DegreesToRadians(State.Angle)));
                FVector2D NewPos = State.Position + Direction * State.StepLength;

                // Clamp to city bounds
                NewPos.X = FMath::Clamp(NewPos.X, City.Center.X - Params.CitySize.X * 0.5f, City.Center.X + Params.CitySize.X * 0.5f);
                NewPos.Y = FMath::Clamp(NewPos.Y, City.Center.Y - Params.CitySize.Y * 0.5f, City.Center.Y + Params.CitySize.Y * 0.5f);

                FForge2RoadSegment Road;
                Road.Start = State.Position;
                Road.End = NewPos;
                Road.Width = State.Width;
                Road.RoadType = State.RoadType;
                City.Roads.Add(Road);

                State.Position = NewPos;
                break;
            }

            case 'f': // Move forward without drawing
            {
                FVector2D Direction(FMath::Cos(FMath::DegreesToRadians(State.Angle)),
                                    FMath::Sin(FMath::DegreesToRadians(State.Angle)));
                State.Position += Direction * State.StepLength;
                break;
            }

            case '+': // Turn right
                State.Angle += Random.FRandRange(20.0f, 40.0f);
                break;

            case '-': // Turn left
                State.Angle -= Random.FRandRange(20.0f, 40.0f);
                break;

            case '|': // Turn around
                State.Angle += 180.0f;
                break;

            case '[': // Push state
                StateStack.Add(State);
                break;

            case ']': // Pop state
                if (StateStack.Num() > 0)
                {
                    State = StateStack.Pop();
                }
                break;

            case '>': // Decrease step length
                State.StepLength *= 0.8f;
                State.Width *= 0.7f;
                State.RoadType = (State.Width < Params.SecondaryRoadWidth * 0.5f) ?
                    EForge2RoadType::Alley : EForge2RoadType::SecondaryRoad;
                break;

            case '<': // Increase step length
                State.StepLength *= 1.2f;
                State.Width *= 1.3f;
                State.RoadType = EForge2RoadType::MainRoad;
                break;

            default:
                break;
        }
    }
}

// ============================================================================
// City Generator
// ============================================================================

FForge2CityData FForge2CityGenerator::Generate(const FForge2CityParams& Params)
{
    return GenerateAt(Params, FVector2D::ZeroVector);
}

FForge2CityData FForge2CityGenerator::GenerateAt(const FForge2CityParams& Params, const FVector2D& Location)
{
    FForge2CityData City;
    City.Center = Location;
    City.SettlementType = Params.SettlementType;
    City.Seed = Params.Seed;

    FRandomStream Random(Params.Seed);

    // Generate road network
    GenerateRoads(City, Params, Random);

    // Create intersections
    CreateIntersections(City);

    // Generate districts
    GenerateDistricts(City, Params, Random);

    // Assign district types
    AssignDistrictTypes(City, Params, Random);

    // Generate plazas
    if (Params.bIncludePlazas)
    {
        GeneratePlazas(City, Params, Random);
    }

    // Generate building lots
    GenerateBuildingLots(City, Params, Random);

    // Generate buildings
    GenerateBuildings(City, Params, Random);

    // Generate landmarks
    GenerateLandmarks(City, Params, Random);

    // Generate fortifications
    if (Params.bIncludeWalls && Params.FortificationType != EForge2FortificationType::None)
    {
        GenerateFortifications(City, Params, Random);
        PlaceGates(City, Params, Random);
        PlaceTowers(City, Params, Random);
    }

    // Calculate bounds and population
    CalculateBounds(City);
    CalculatePopulation(City);

    return City;
}

void FForge2CityGenerator::GenerateRoads(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    if (Params.bGridLayout)
    {
        GenerateGridRoads(City, Params, Random);
    }
    else
    {
        // Mix of radial and organic for larger settlements
        if (Params.SettlementType == EForge2SettlementType::City ||
            Params.SettlementType == EForge2SettlementType::Metropolis)
        {
            GenerateRadialRoads(City, Params, Random);
        }
        GenerateOrganicRoads(City, Params, Random);
    }
}

void FForge2CityGenerator::GenerateGridRoads(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    float HalfWidth = Params.CitySize.X * 0.5f;
    float HalfDepth = Params.CitySize.Y * 0.5f;

    // Calculate grid spacing based on density
    float GridSpacing = FMath::Lerp(500.0f, 200.0f, Params.RoadDensity);

    // Apply grid rotation
    float CosAngle = FMath::Cos(FMath::DegreesToRadians(Params.GridAngle));
    float SinAngle = FMath::Sin(FMath::DegreesToRadians(Params.GridAngle));

    auto RotatePoint = [&](float X, float Y) -> FVector2D
    {
        return FVector2D(
            City.Center.X + X * CosAngle - Y * SinAngle,
            City.Center.Y + X * SinAngle + Y * CosAngle
        );
    };

    // Generate horizontal roads
    for (float Y = -HalfDepth; Y <= HalfDepth; Y += GridSpacing)
    {
        FForge2RoadSegment Road;
        Road.Start = RotatePoint(-HalfWidth, Y);
        Road.End = RotatePoint(HalfWidth, Y);
        Road.Width = (FMath::Abs(Y) < GridSpacing) ? Params.MainRoadWidth : Params.SecondaryRoadWidth;
        Road.RoadType = (Road.Width == Params.MainRoadWidth) ? EForge2RoadType::MainRoad : EForge2RoadType::SecondaryRoad;
        City.Roads.Add(Road);
    }

    // Generate vertical roads
    for (float X = -HalfWidth; X <= HalfWidth; X += GridSpacing)
    {
        FForge2RoadSegment Road;
        Road.Start = RotatePoint(X, -HalfDepth);
        Road.End = RotatePoint(X, HalfDepth);
        Road.Width = (FMath::Abs(X) < GridSpacing) ? Params.MainRoadWidth : Params.SecondaryRoadWidth;
        Road.RoadType = (Road.Width == Params.MainRoadWidth) ? EForge2RoadType::MainRoad : EForge2RoadType::SecondaryRoad;
        City.Roads.Add(Road);
    }
}

void FForge2CityGenerator::GenerateOrganicRoads(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    FForge2LSystemRoads LSystem;

    // Set up L-system rules for organic roads
    LSystem.SetAxiom(TEXT("F"));
    LSystem.AddRule(TEXT("F"), TEXT("F[+F][-F]"), 0.4f);
    LSystem.AddRule(TEXT("F"), TEXT("FF"), 0.3f);
    LSystem.AddRule(TEXT("F"), TEXT("F[+F]F"), 0.3f);

    int32 Iterations = 3 + static_cast<int32>(Params.RoadDensity * 2);
    FString LString = LSystem.Iterate(Iterations, Random);

    LSystem.InterpretRoads(LString, City, Params, Random);
}

void FForge2CityGenerator::GenerateRadialRoads(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    float CityRadius = FMath::Min(Params.CitySize.X, Params.CitySize.Y) * 0.5f;

    // Main radial roads from center
    int32 RadialCount = 4 + Random.RandRange(0, 4);

    for (int32 i = 0; i < RadialCount; ++i)
    {
        float Angle = (static_cast<float>(i) / RadialCount) * 2.0f * PI;
        Angle += Random.FRandRange(-0.1f, 0.1f); // Slight randomization

        FForge2RoadSegment Road;
        Road.Start = City.Center;
        Road.End = City.Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * CityRadius;
        Road.Width = Params.MainRoadWidth;
        Road.RoadType = EForge2RoadType::MainRoad;
        City.Roads.Add(Road);
    }

    // Ring roads at various radii
    int32 RingCount = 2 + static_cast<int32>(Params.RoadDensity * 3);
    int32 SegmentsPerRing = 16;

    for (int32 Ring = 1; Ring <= RingCount; ++Ring)
    {
        float Radius = (static_cast<float>(Ring) / (RingCount + 1)) * CityRadius;

        for (int32 Seg = 0; Seg < SegmentsPerRing; ++Seg)
        {
            float Angle0 = (static_cast<float>(Seg) / SegmentsPerRing) * 2.0f * PI;
            float Angle1 = (static_cast<float>(Seg + 1) / SegmentsPerRing) * 2.0f * PI;

            FForge2RoadSegment Road;
            Road.Start = City.Center + FVector2D(FMath::Cos(Angle0), FMath::Sin(Angle0)) * Radius;
            Road.End = City.Center + FVector2D(FMath::Cos(Angle1), FMath::Sin(Angle1)) * Radius;
            Road.Width = (Ring == 1) ? Params.MainRoadWidth : Params.SecondaryRoadWidth;
            Road.RoadType = (Ring == 1) ? EForge2RoadType::MainRoad : EForge2RoadType::SecondaryRoad;
            City.Roads.Add(Road);
        }
    }
}

void FForge2CityGenerator::CreateIntersections(FForge2CityData& City)
{
    TMap<int64, FForge2Intersection> IntersectionMap;

    auto HashPosition = [](const FVector2D& Pos) -> int64
    {
        int32 X = FMath::RoundToInt(Pos.X / 10.0f);
        int32 Y = FMath::RoundToInt(Pos.Y / 10.0f);
        return (static_cast<int64>(X) << 32) | static_cast<int64>(Y);
    };

    // Find all road endpoints and intersections
    for (int32 i = 0; i < City.Roads.Num(); ++i)
    {
        const FForge2RoadSegment& Road = City.Roads[i];

        // Add start point
        int64 StartHash = HashPosition(Road.Start);
        if (!IntersectionMap.Contains(StartHash))
        {
            FForge2Intersection Intersection;
            Intersection.Position = Road.Start;
            IntersectionMap.Add(StartHash, Intersection);
        }
        IntersectionMap[StartHash].RoadIndices.AddUnique(i);

        // Add end point
        int64 EndHash = HashPosition(Road.End);
        if (!IntersectionMap.Contains(EndHash))
        {
            FForge2Intersection Intersection;
            Intersection.Position = Road.End;
            IntersectionMap.Add(EndHash, Intersection);
        }
        IntersectionMap[EndHash].RoadIndices.AddUnique(i);
    }

    // Convert map to array
    for (auto& Pair : IntersectionMap)
    {
        Pair.Value.Type = Pair.Value.RoadIndices.Num();
        City.Intersections.Add(Pair.Value);
    }

    // Update road connectivity
    for (const FForge2Intersection& Intersection : City.Intersections)
    {
        for (int32 i = 0; i < Intersection.RoadIndices.Num(); ++i)
        {
            for (int32 j = i + 1; j < Intersection.RoadIndices.Num(); ++j)
            {
                int32 RoadA = Intersection.RoadIndices[i];
                int32 RoadB = Intersection.RoadIndices[j];

                City.Roads[RoadA].ConnectedRoads.AddUnique(RoadB);
                City.Roads[RoadB].ConnectedRoads.AddUnique(RoadA);
            }
        }
    }
}

void FForge2CityGenerator::GenerateDistricts(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    int32 DistrictCount = Random.RandRange(Params.MinDistricts, Params.MaxDistricts);

    // Use Voronoi-like distribution for districts
    TArray<FVector2D> DistrictCenters;

    // Generate district center points
    for (int32 i = 0; i < DistrictCount; ++i)
    {
        FVector2D Point;
        Point.X = City.Center.X + Random.FRandRange(-Params.CitySize.X * 0.4f, Params.CitySize.X * 0.4f);
        Point.Y = City.Center.Y + Random.FRandRange(-Params.CitySize.Y * 0.4f, Params.CitySize.Y * 0.4f);
        DistrictCenters.Add(Point);
    }

    // Create district data
    for (int32 i = 0; i < DistrictCount; ++i)
    {
        FForge2DistrictData District;
        District.Center = DistrictCenters[i];

        // Generate approximate bounds (simplified Voronoi cell)
        int32 BoundaryPoints = 8;
        float BaseRadius = FMath::Min(Params.CitySize.X, Params.CitySize.Y) / DistrictCount * 0.5f;

        for (int32 j = 0; j < BoundaryPoints; ++j)
        {
            float Angle = (static_cast<float>(j) / BoundaryPoints) * 2.0f * PI;
            float Radius = BaseRadius * Random.FRandRange(0.7f, 1.3f);

            // Check distance to nearest neighbor and adjust
            float MinNeighborDist = MAX_FLT;
            for (int32 k = 0; k < DistrictCenters.Num(); ++k)
            {
                if (k == i) continue;
                float Dist = (DistrictCenters[k] - District.Center).Size();
                MinNeighborDist = FMath::Min(MinNeighborDist, Dist);
            }
            Radius = FMath::Min(Radius, MinNeighborDist * 0.5f);

            District.Bounds.Add(District.Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
        }

        City.Districts.Add(District);
    }
}

void FForge2CityGenerator::AssignDistrictTypes(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    // Calculate total weights
    float TotalWeight = 0.0f;
    for (const auto& Pair : Params.DistrictWeights)
    {
        TotalWeight += Pair.Value;
    }

    // Assign center district as civic/market
    if (City.Districts.Num() > 0)
    {
        float MinDistToCenter = MAX_FLT;
        int32 CenterDistrictIndex = 0;

        for (int32 i = 0; i < City.Districts.Num(); ++i)
        {
            float Dist = (City.Districts[i].Center - City.Center).Size();
            if (Dist < MinDistToCenter)
            {
                MinDistToCenter = Dist;
                CenterDistrictIndex = i;
            }
        }

        City.Districts[CenterDistrictIndex].DistrictType = EForge2DistrictType::Civic;
        City.Districts[CenterDistrictIndex].WealthLevel = 0.8f;
    }

    // Assign other districts based on weights
    for (FForge2DistrictData& District : City.Districts)
    {
        if (District.DistrictType != EForge2DistrictType::Civic)
        {
            float Roll = Random.FRand() * TotalWeight;
            float Accumulator = 0.0f;

            for (const auto& Pair : Params.DistrictWeights)
            {
                Accumulator += Pair.Value;
                if (Roll <= Accumulator)
                {
                    District.DistrictType = Pair.Key;
                    break;
                }
            }

            // Set wealth and density based on type
            switch (District.DistrictType)
            {
                case EForge2DistrictType::Noble:
                    District.WealthLevel = Random.FRandRange(0.8f, 1.0f);
                    District.Density = Random.FRandRange(0.3f, 0.5f);
                    break;
                case EForge2DistrictType::Slums:
                    District.WealthLevel = Random.FRandRange(0.0f, 0.2f);
                    District.Density = Random.FRandRange(0.8f, 1.0f);
                    break;
                case EForge2DistrictType::Industrial:
                    District.WealthLevel = Random.FRandRange(0.3f, 0.5f);
                    District.Density = Random.FRandRange(0.6f, 0.8f);
                    break;
                case EForge2DistrictType::Commercial:
                case EForge2DistrictType::Market:
                    District.WealthLevel = Random.FRandRange(0.5f, 0.7f);
                    District.Density = Random.FRandRange(0.7f, 0.9f);
                    break;
                default:
                    District.WealthLevel = Random.FRandRange(0.3f, 0.7f);
                    District.Density = Random.FRandRange(0.5f, 0.7f);
                    break;
            }
        }
    }
}

void FForge2CityGenerator::GenerateBuildingLots(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    // Generate lots along roads
    for (int32 RoadIndex = 0; RoadIndex < City.Roads.Num(); ++RoadIndex)
    {
        const FForge2RoadSegment& Road = City.Roads[RoadIndex];

        if (Road.RoadType == EForge2RoadType::Bridge) continue;

        FVector2D RoadDir = Road.GetDirection();
        FVector2D RoadNormal(-RoadDir.Y, RoadDir.X);
        float RoadLength = Road.GetLength();

        // Lot dimensions based on road type
        float LotWidth = (Road.RoadType == EForge2RoadType::MainRoad) ? 150.0f : 100.0f;
        float LotDepth = (Road.RoadType == EForge2RoadType::MainRoad) ? 200.0f : 150.0f;

        // Place lots on both sides of the road
        for (int32 Side = -1; Side <= 1; Side += 2)
        {
            float CurrentPos = LotWidth * 0.5f;

            while (CurrentPos < RoadLength - LotWidth * 0.5f)
            {
                // Skip some lots based on density
                if (Random.FRand() > Params.BuildingDensity)
                {
                    CurrentPos += LotWidth;
                    continue;
                }

                FVector2D LotCenter = Road.Start + RoadDir * CurrentPos + RoadNormal * (Road.Width * 0.5f + LotDepth * 0.5f) * Side;

                FForge2BuildingLot Lot;
                Lot.Center = LotCenter;
                Lot.Rotation = FMath::Atan2(RoadNormal.Y * Side, RoadNormal.X * Side) * (180.0f / PI);
                Lot.FacingRoadIndex = RoadIndex;

                // Generate lot bounds
                float HalfWidth = LotWidth * 0.5f;
                float HalfDepth = LotDepth * 0.5f;
                Lot.Bounds.Add(LotCenter + RoadDir * -HalfWidth + RoadNormal * -HalfDepth * Side);
                Lot.Bounds.Add(LotCenter + RoadDir * HalfWidth + RoadNormal * -HalfDepth * Side);
                Lot.Bounds.Add(LotCenter + RoadDir * HalfWidth + RoadNormal * HalfDepth * Side);
                Lot.Bounds.Add(LotCenter + RoadDir * -HalfWidth + RoadNormal * HalfDepth * Side);

                // Find which district this lot belongs to
                for (int32 DistIndex = 0; DistIndex < City.Districts.Num(); ++DistIndex)
                {
                    float Dist = (City.Districts[DistIndex].Center - LotCenter).Size();
                    if (Lot.DistrictIndex < 0 || Dist < (City.Districts[Lot.DistrictIndex].Center - LotCenter).Size())
                    {
                        Lot.DistrictIndex = DistIndex;
                    }
                }

                City.BuildingLots.Add(Lot);
                if (Lot.DistrictIndex >= 0)
                {
                    City.Districts[Lot.DistrictIndex].BuildingLots.Add(City.BuildingLots.Num() - 1);
                }

                CurrentPos += LotWidth + Random.FRandRange(10.0f, 30.0f);
            }
        }
    }
}

void FForge2CityGenerator::GenerateBuildings(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    FForge2BuildingGenerator BuildingGen;

    for (int32 LotIndex = 0; LotIndex < City.BuildingLots.Num(); ++LotIndex)
    {
        FForge2BuildingLot& Lot = City.BuildingLots[LotIndex];

        if (Lot.DistrictIndex < 0) continue;

        FForge2BuildingParams BuildingParams = GetBuildingParamsForDistrict(
            City.Districts[Lot.DistrictIndex].DistrictType,
            Params,
            Random
        );

        FVector2D LotSize = Lot.GetSize();
        BuildingParams.FootprintMin = FVector2D(LotSize.X * 0.6f, LotSize.Y * 0.6f);
        BuildingParams.FootprintMax = FVector2D(LotSize.X * 0.9f, LotSize.Y * 0.9f);
        BuildingParams.Seed = Params.Seed + LotIndex;

        FForge2BuildingData Building = BuildingGen.GenerateAt(
            BuildingParams,
            FVector(Lot.Center.X, Lot.Center.Y, 0.0f),
            FRotator(0.0f, Lot.Rotation, 0.0f)
        );

        Lot.BuildingIndex = City.Buildings.Num();
        City.Buildings.Add(Building);
    }
}

void FForge2CityGenerator::GeneratePlazas(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    // Place main plaza at city center
    FForge2PlazaData MainPlaza;
    MainPlaza.Center = City.Center;
    MainPlaza.PlazaType = FName("MainSquare");
    MainPlaza.bHasFountain = true;
    MainPlaza.bHasMonument = true;

    float PlazaSize = FMath::Min(Params.CitySize.X, Params.CitySize.Y) * 0.05f;
    MainPlaza.Bounds.Add(City.Center + FVector2D(-PlazaSize, -PlazaSize));
    MainPlaza.Bounds.Add(City.Center + FVector2D(PlazaSize, -PlazaSize));
    MainPlaza.Bounds.Add(City.Center + FVector2D(PlazaSize, PlazaSize));
    MainPlaza.Bounds.Add(City.Center + FVector2D(-PlazaSize, PlazaSize));

    City.Plazas.Add(MainPlaza);

    // Add market plaza in commercial district
    for (const FForge2DistrictData& District : City.Districts)
    {
        if (District.DistrictType == EForge2DistrictType::Market ||
            District.DistrictType == EForge2DistrictType::Commercial)
        {
            FForge2PlazaData MarketPlaza;
            MarketPlaza.Center = District.Center;
            MarketPlaza.PlazaType = FName("Market");
            MarketPlaza.bHasFountain = Random.FRand() > 0.5f;

            float Size = PlazaSize * 0.7f;
            MarketPlaza.Bounds.Add(District.Center + FVector2D(-Size, -Size));
            MarketPlaza.Bounds.Add(District.Center + FVector2D(Size, -Size));
            MarketPlaza.Bounds.Add(District.Center + FVector2D(Size, Size));
            MarketPlaza.Bounds.Add(District.Center + FVector2D(-Size, Size));

            City.Plazas.Add(MarketPlaza);
            break;
        }
    }
}

void FForge2CityGenerator::GenerateFortifications(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    // Calculate city wall bounds
    float WallRadius = FMath::Max(Params.CitySize.X, Params.CitySize.Y) * 0.5f + 200.0f;
    int32 WallSegments = 16;

    TArray<FVector2D> WallPoints;
    for (int32 i = 0; i < WallSegments; ++i)
    {
        float Angle = (static_cast<float>(i) / WallSegments) * 2.0f * PI;
        float Radius = WallRadius * Random.FRandRange(0.95f, 1.05f);
        WallPoints.Add(City.Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
    }

    // Create wall sections
    for (int32 i = 0; i < WallPoints.Num(); ++i)
    {
        int32 Next = (i + 1) % WallPoints.Num();

        FForge2WallSection Section;
        Section.Start = WallPoints[i];
        Section.End = WallPoints[Next];
        Section.Height = 500.0f;
        Section.Thickness = 100.0f;
        Section.FortType = Params.FortificationType;
        Section.bHasWalkway = true;
        Section.bHasCrenellations = true;

        City.Walls.Add(Section);
    }
}

void FForge2CityGenerator::PlaceGates(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    if (City.Walls.Num() == 0) return;

    int32 GateCount = FMath::Min(Params.GateCount, City.Walls.Num());
    int32 Spacing = City.Walls.Num() / GateCount;

    for (int32 i = 0; i < GateCount; ++i)
    {
        int32 WallIndex = (i * Spacing) % City.Walls.Num();
        FForge2WallSection& Wall = City.Walls[WallIndex];

        FForge2GateData Gate;
        Gate.Position = (Wall.Start + Wall.End) * 0.5f;
        Gate.Rotation = FMath::Atan2(Wall.End.Y - Wall.Start.Y, Wall.End.X - Wall.Start.X) * (180.0f / PI);
        Gate.Width = 400.0f;
        Gate.bMainGate = (i == 0);
        Gate.GateName = (i == 0) ? FName("MainGate") : FName(*FString::Printf(TEXT("Gate_%d"), i));

        City.Gates.Add(Gate);
    }
}

void FForge2CityGenerator::PlaceTowers(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    // Place towers at wall corners
    for (int32 i = 0; i < City.Walls.Num(); ++i)
    {
        const FForge2WallSection& Wall = City.Walls[i];

        FForge2TowerData Tower;
        Tower.Position = Wall.Start;
        Tower.Radius = 150.0f;
        Tower.Height = Wall.Height + 300.0f;
        Tower.TowerType = FName("CornerTower");
        Tower.WallIndices.Add(i);
        Tower.WallIndices.Add((i - 1 + City.Walls.Num()) % City.Walls.Num());

        City.Towers.Add(Tower);
    }

    // Place gate towers
    for (const FForge2GateData& Gate : City.Gates)
    {
        FVector2D GateDir(FMath::Cos(FMath::DegreesToRadians(Gate.Rotation)), FMath::Sin(FMath::DegreesToRadians(Gate.Rotation)));
        FVector2D GateNormal(-GateDir.Y, GateDir.X);

        for (int32 Side = -1; Side <= 1; Side += 2)
        {
            FForge2TowerData Tower;
            Tower.Position = Gate.Position + GateNormal * (Gate.Width * 0.5f + 100.0f) * Side;
            Tower.Radius = 120.0f;
            Tower.Height = 700.0f;
            Tower.TowerType = FName("GateTower");

            City.Towers.Add(Tower);
        }
    }
}

void FForge2CityGenerator::GenerateLandmarks(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random)
{
    // Place landmarks at district centers or plazas
    TArray<FName> LandmarkTypes = {
        FName("Cathedral"),
        FName("Castle"),
        FName("TownHall"),
        FName("Temple"),
        FName("Fortress"),
        FName("Monument")
    };

    int32 LandmarksPlaced = 0;

    for (FForge2PlazaData& Plaza : City.Plazas)
    {
        if (LandmarksPlaced >= Params.LandmarkCount) break;

        // Create a landmark building
        FForge2BuildingParams LandmarkParams;
        LandmarkParams.Style = Params.BuildingStyle;
        LandmarkParams.MinFloors = 3;
        LandmarkParams.MaxFloors = 5;
        LandmarkParams.FootprintMin = FVector2D(300.0f, 300.0f);
        LandmarkParams.FootprintMax = FVector2D(500.0f, 500.0f);
        LandmarkParams.Seed = Params.Seed + LandmarksPlaced + 10000;

        if (LandmarkTypes[LandmarksPlaced % LandmarkTypes.Num()] == FName("Cathedral") ||
            LandmarkTypes[LandmarksPlaced % LandmarkTypes.Num()] == FName("Temple"))
        {
            LandmarkParams.RoofType = EForge2RoofType::Dome;
        }
        else if (LandmarkTypes[LandmarksPlaced % LandmarkTypes.Num()] == FName("Castle"))
        {
            LandmarkParams.MinFloors = 4;
            LandmarkParams.MaxFloors = 6;
        }

        FForge2BuildingGenerator BuildingGen;
        FForge2BuildingData Landmark = BuildingGen.GenerateAt(
            LandmarkParams,
            FVector(Plaza.Center.X, Plaza.Center.Y, 0.0f),
            FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f)
        );

        City.Buildings.Add(Landmark);
        ++LandmarksPlaced;
    }
}

void FForge2CityGenerator::CalculateBounds(FForge2CityData& City)
{
    if (City.Roads.Num() == 0 && City.Buildings.Num() == 0) return;

    FVector2D Min(MAX_FLT, MAX_FLT);
    FVector2D Max(-MAX_FLT, -MAX_FLT);

    for (const FForge2RoadSegment& Road : City.Roads)
    {
        Min.X = FMath::Min3(Min.X, Road.Start.X, Road.End.X);
        Min.Y = FMath::Min3(Min.Y, Road.Start.Y, Road.End.Y);
        Max.X = FMath::Max3(Max.X, Road.Start.X, Road.End.X);
        Max.Y = FMath::Max3(Max.Y, Road.Start.Y, Road.End.Y);
    }

    for (const FForge2WallSection& Wall : City.Walls)
    {
        Min.X = FMath::Min3(Min.X, Wall.Start.X, Wall.End.X);
        Min.Y = FMath::Min3(Min.Y, Wall.Start.Y, Wall.End.Y);
        Max.X = FMath::Max3(Max.X, Wall.Start.X, Wall.End.X);
        Max.Y = FMath::Max3(Max.Y, Wall.Start.Y, Wall.End.Y);
    }

    // Create bounds polygon
    City.Bounds.Empty();
    City.Bounds.Add(Min);
    City.Bounds.Add(FVector2D(Max.X, Min.Y));
    City.Bounds.Add(Max);
    City.Bounds.Add(FVector2D(Min.X, Max.Y));
}

void FForge2CityGenerator::CalculatePopulation(FForge2CityData& City)
{
    // Estimate population based on building count and district types
    int32 Population = 0;

    for (const FForge2DistrictData& District : City.Districts)
    {
        int32 BuildingCount = District.BuildingLots.Num();

        switch (District.DistrictType)
        {
            case EForge2DistrictType::Residential:
                Population += BuildingCount * 4; // 4 people per house
                break;
            case EForge2DistrictType::Slums:
                Population += BuildingCount * 8; // Overcrowded
                break;
            case EForge2DistrictType::Noble:
                Population += BuildingCount * 6; // Larger families + servants
                break;
            default:
                Population += BuildingCount * 2; // Fewer residents in commercial/industrial
                break;
        }
    }

    City.Population = Population;
}

FForge2BuildingParams FForge2CityGenerator::GetBuildingParamsForDistrict(EForge2DistrictType DistrictType, const FForge2CityParams& CityParams, FRandomStream& Random)
{
    FForge2BuildingParams Params;
    Params.Style = CityParams.BuildingStyle;

    switch (DistrictType)
    {
        case EForge2DistrictType::Residential:
            Params.MinFloors = 1;
            Params.MaxFloors = 2;
            Params.bGenerateInterior = true;
            Params.RoofType = EForge2RoofType::Gabled;
            break;

        case EForge2DistrictType::Commercial:
        case EForge2DistrictType::Market:
            Params.MinFloors = 1;
            Params.MaxFloors = 3;
            Params.WindowDensity = 0.7f;
            Params.RoofType = EForge2RoofType::Flat;
            break;

        case EForge2DistrictType::Industrial:
            Params.MinFloors = 1;
            Params.MaxFloors = 2;
            Params.WindowDensity = 0.3f;
            Params.RoofType = EForge2RoofType::Shed;
            break;

        case EForge2DistrictType::Noble:
            Params.MinFloors = 2;
            Params.MaxFloors = 4;
            Params.bIncludeBalconies = true;
            Params.RoofType = EForge2RoofType::Mansard;
            Params.FurnitureDensity = 0.8f;
            break;

        case EForge2DistrictType::Slums:
            Params.MinFloors = 1;
            Params.MaxFloors = 2;
            Params.WindowDensity = 0.2f;
            Params.FurnitureDensity = 0.3f;
            Params.RoofType = EForge2RoofType::Shed;
            break;

        case EForge2DistrictType::Religious:
            Params.MinFloors = 1;
            Params.MaxFloors = 2;
            Params.RoofType = EForge2RoofType::Dome;
            break;

        case EForge2DistrictType::Civic:
            Params.MinFloors = 2;
            Params.MaxFloors = 3;
            Params.RoofType = EForge2RoofType::Hipped;
            break;

        default:
            Params.MinFloors = 1;
            Params.MaxFloors = 2;
            break;
    }

    return Params;
}

// ============================================================================
// Village Generator
// ============================================================================

FForge2CityData FForge2VillageGenerator::Generate(const FForge2CityParams& Params)
{
    FForge2CityData Village;
    Village.SettlementType = EForge2SettlementType::Village;
    Village.Center = FVector2D::ZeroVector;
    Village.Seed = Params.Seed;

    FRandomStream Random(Params.Seed);

    // Generate village center
    GenerateVillageCenter(Village, Params, Random);

    // Place buildings in a cluster
    int32 BuildingCount = FMath::Min(Params.PopulationTarget / 4, 30);
    FForge2BuildingGenerator BuildingGen;

    for (int32 i = 0; i < BuildingCount; ++i)
    {
        float Angle = Random.FRand() * 2.0f * PI;
        float Distance = Random.FRandRange(200.0f, Params.CitySize.X * 0.3f);

        FVector2D BuildingPos = Village.Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Distance;

        FForge2BuildingParams BuildingParams;
        BuildingParams.Style = Params.BuildingStyle;
        BuildingParams.MinFloors = 1;
        BuildingParams.MaxFloors = 2;
        BuildingParams.FootprintMin = FVector2D(80.0f, 80.0f);
        BuildingParams.FootprintMax = FVector2D(150.0f, 120.0f);
        BuildingParams.Seed = Params.Seed + i;

        FForge2BuildingData Building = BuildingGen.GenerateAt(
            BuildingParams,
            FVector(BuildingPos.X, BuildingPos.Y, 0.0f),
            FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f)
        );

        Village.Buildings.Add(Building);
    }

    // Generate paths
    GeneratePaths(Village, Random);

    // Generate farm fields
    GenerateFarmFields(Village, Params, Random);

    Village.Population = BuildingCount * 4;

    return Village;
}

void FForge2VillageGenerator::GenerateVillageCenter(FForge2CityData& Village, const FForge2CityParams& Params, FRandomStream& Random)
{
    // Create a central plaza/well
    FForge2PlazaData Center;
    Center.Center = Village.Center;
    Center.PlazaType = FName("VillageSquare");
    Center.bHasFountain = true; // Well/fountain

    float Size = 50.0f;
    Center.Bounds.Add(Village.Center + FVector2D(-Size, -Size));
    Center.Bounds.Add(Village.Center + FVector2D(Size, -Size));
    Center.Bounds.Add(Village.Center + FVector2D(Size, Size));
    Center.Bounds.Add(Village.Center + FVector2D(-Size, Size));

    Village.Plazas.Add(Center);
}

void FForge2VillageGenerator::GeneratePaths(FForge2CityData& Village, FRandomStream& Random)
{
    // Connect buildings to center with paths
    for (const FForge2BuildingData& Building : Village.Buildings)
    {
        FForge2RoadSegment Path;
        Path.Start = Village.Center;
        Path.End = FVector2D(Building.Origin.X, Building.Origin.Y);
        Path.Width = 100.0f;
        Path.RoadType = EForge2RoadType::Path;

        Village.Roads.Add(Path);
    }
}

void FForge2VillageGenerator::GenerateFarmFields(FForge2CityData& Village, const FForge2CityParams& Params, FRandomStream& Random)
{
    // Create farm field districts around village
    int32 FieldCount = Random.RandRange(3, 6);

    for (int32 i = 0; i < FieldCount; ++i)
    {
        float Angle = (static_cast<float>(i) / FieldCount) * 2.0f * PI;
        float Distance = Params.CitySize.X * 0.4f;

        FForge2DistrictData Field;
        Field.DistrictType = EForge2DistrictType::Agricultural;
        Field.Center = Village.Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Distance;
        Field.WealthLevel = 0.3f;
        Field.Density = 0.0f; // No buildings in fields

        float FieldSize = Random.FRandRange(100.0f, 200.0f);
        Field.Bounds.Add(Field.Center + FVector2D(-FieldSize, -FieldSize));
        Field.Bounds.Add(Field.Center + FVector2D(FieldSize, -FieldSize));
        Field.Bounds.Add(Field.Center + FVector2D(FieldSize, FieldSize));
        Field.Bounds.Add(Field.Center + FVector2D(-FieldSize, FieldSize));

        Village.Districts.Add(Field);
    }
}

// ============================================================================
// Fortification Generator
// ============================================================================

TArray<FForge2WallSection> FForge2FortificationGenerator::GenerateWalls(const TArray<FVector2D>& Bounds, EForge2FortificationType FortType, const TArray<FVector2D>& GatePositions)
{
    TArray<FForge2WallSection> Walls;

    for (int32 i = 0; i < Bounds.Num(); ++i)
    {
        int32 Next = (i + 1) % Bounds.Num();

        FForge2WallSection Section;
        Section.Start = Bounds[i];
        Section.End = Bounds[Next];
        Section.FortType = FortType;

        switch (FortType)
        {
            case EForge2FortificationType::Palisade:
                Section.Height = 300.0f;
                Section.Thickness = 30.0f;
                Section.bHasCrenellations = false;
                break;
            case EForge2FortificationType::StoneWall:
                Section.Height = 500.0f;
                Section.Thickness = 100.0f;
                break;
            case EForge2FortificationType::CastleWall:
                Section.Height = 800.0f;
                Section.Thickness = 200.0f;
                break;
            default:
                Section.Height = 500.0f;
                Section.Thickness = 100.0f;
                break;
        }

        Walls.Add(Section);
    }

    return Walls;
}

TArray<FForge2TowerData> FForge2FortificationGenerator::GenerateTowers(const TArray<FForge2WallSection>& Walls, float Spacing)
{
    TArray<FForge2TowerData> Towers;

    for (int32 i = 0; i < Walls.Num(); ++i)
    {
        const FForge2WallSection& Wall = Walls[i];

        // Tower at start of wall (corner)
        FForge2TowerData CornerTower;
        CornerTower.Position = Wall.Start;
        CornerTower.Radius = Wall.Thickness * 1.5f;
        CornerTower.Height = Wall.Height + 200.0f;
        CornerTower.TowerType = FName("CornerTower");
        CornerTower.WallIndices.Add(i);
        Towers.Add(CornerTower);

        // Additional towers along long walls
        float WallLength = (Wall.End - Wall.Start).Size();
        int32 IntermediateTowers = FMath::FloorToInt(WallLength / Spacing) - 1;

        for (int32 j = 1; j <= IntermediateTowers; ++j)
        {
            float T = static_cast<float>(j) / (IntermediateTowers + 1);

            FForge2TowerData IntermediateTower;
            IntermediateTower.Position = Wall.Start + (Wall.End - Wall.Start) * T;
            IntermediateTower.Radius = Wall.Thickness;
            IntermediateTower.Height = Wall.Height + 100.0f;
            IntermediateTower.TowerType = FName("WallTower");
            IntermediateTower.WallIndices.Add(i);
            Towers.Add(IntermediateTower);
        }
    }

    return Towers;
}

TArray<FForge2GateData> FForge2FortificationGenerator::GenerateGates(TArray<FForge2WallSection>& Walls, const TArray<FVector2D>& GatePositions, float GateWidth)
{
    TArray<FForge2GateData> Gates;

    for (const FVector2D& GatePos : GatePositions)
    {
        // Find closest wall
        int32 ClosestWall = -1;
        float ClosestDist = MAX_FLT;

        for (int32 i = 0; i < Walls.Num(); ++i)
        {
            FVector2D MidPoint = (Walls[i].Start + Walls[i].End) * 0.5f;
            float Dist = (MidPoint - GatePos).Size();

            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                ClosestWall = i;
            }
        }

        if (ClosestWall >= 0)
        {
            const FForge2WallSection& Wall = Walls[ClosestWall];
            FVector2D WallDir = (Wall.End - Wall.Start).GetSafeNormal();

            FForge2GateData Gate;
            Gate.Position = (Wall.Start + Wall.End) * 0.5f;
            Gate.Rotation = FMath::Atan2(WallDir.Y, WallDir.X) * (180.0f / PI);
            Gate.Width = GateWidth;
            Gate.RoadIndex = -1;
            Gate.bMainGate = (Gates.Num() == 0);
            Gate.GateName = FName(*FString::Printf(TEXT("Gate_%d"), Gates.Num()));

            Gates.Add(Gate);
        }
    }

    return Gates;
}

TArray<FVector2D> FForge2FortificationGenerator::GenerateMoat(const TArray<FForge2WallSection>& Walls, float MoatWidth)
{
    TArray<FVector2D> MoatPath;

    for (const FForge2WallSection& Wall : Walls)
    {
        FVector2D WallDir = (Wall.End - Wall.Start).GetSafeNormal();
        FVector2D WallNormal(-WallDir.Y, WallDir.X);

        // Moat runs outside the wall
        MoatPath.Add(Wall.Start + WallNormal * (Wall.Thickness * 0.5f + MoatWidth));
    }

    return MoatPath;
}

// ============================================================================
// Road Utilities
// ============================================================================

bool FForge2RoadUtils::FindIntersection(const FForge2RoadSegment& A, const FForge2RoadSegment& B, FVector2D& OutPoint)
{
    FVector2D P1 = A.Start;
    FVector2D P2 = A.End;
    FVector2D P3 = B.Start;
    FVector2D P4 = B.End;

    float D = (P1.X - P2.X) * (P3.Y - P4.Y) - (P1.Y - P2.Y) * (P3.X - P4.X);

    if (FMath::Abs(D) < KINDA_SMALL_NUMBER) return false;

    float T = ((P1.X - P3.X) * (P3.Y - P4.Y) - (P1.Y - P3.Y) * (P3.X - P4.X)) / D;
    float U = -((P1.X - P2.X) * (P1.Y - P3.Y) - (P1.Y - P2.Y) * (P1.X - P3.X)) / D;

    if (T >= 0.0f && T <= 1.0f && U >= 0.0f && U <= 1.0f)
    {
        OutPoint = P1 + (P2 - P1) * T;
        return true;
    }

    return false;
}

void FForge2RoadUtils::ConnectAtIntersection(FForge2CityData& City, int32 RoadA, int32 RoadB, const FVector2D& IntersectionPoint)
{
    City.Roads[RoadA].ConnectedRoads.AddUnique(RoadB);
    City.Roads[RoadB].ConnectedRoads.AddUnique(RoadA);

    FForge2Intersection Intersection;
    Intersection.Position = IntersectionPoint;
    Intersection.RoadIndices.Add(RoadA);
    Intersection.RoadIndices.Add(RoadB);
    Intersection.Type = 2;

    City.Intersections.Add(Intersection);
}

int32 FForge2RoadUtils::SplitRoad(FForge2CityData& City, int32 RoadIndex, const FVector2D& SplitPoint)
{
    FForge2RoadSegment& OriginalRoad = City.Roads[RoadIndex];

    // Create new road segment
    FForge2RoadSegment NewRoad;
    NewRoad.Start = SplitPoint;
    NewRoad.End = OriginalRoad.End;
    NewRoad.Width = OriginalRoad.Width;
    NewRoad.RoadType = OriginalRoad.RoadType;
    NewRoad.ConnectedRoads = OriginalRoad.ConnectedRoads;

    // Modify original road
    OriginalRoad.End = SplitPoint;
    OriginalRoad.ConnectedRoads.Empty();
    OriginalRoad.ConnectedRoads.Add(City.Roads.Num()); // Connect to new road

    int32 NewRoadIndex = City.Roads.Num();
    NewRoad.ConnectedRoads.Add(RoadIndex);
    City.Roads.Add(NewRoad);

    return NewRoadIndex;
}

bool FForge2RoadUtils::IsPointOnRoad(const FForge2RoadSegment& Road, const FVector2D& Point, float Tolerance)
{
    FVector2D RoadVec = Road.End - Road.Start;
    FVector2D PointVec = Point - Road.Start;

    float T = FVector2D::DotProduct(PointVec, RoadVec) / RoadVec.SizeSquared();

    if (T < 0.0f || T > 1.0f) return false;

    FVector2D ClosestPoint = Road.Start + RoadVec * T;
    return (Point - ClosestPoint).Size() <= Tolerance + Road.Width * 0.5f;
}

TArray<FVector2D> FForge2RoadUtils::GetRoadPolygon(const FForge2RoadSegment& Road)
{
    TArray<FVector2D> Polygon;

    FVector2D RoadDir = Road.GetDirection();
    FVector2D RoadNormal(-RoadDir.Y, RoadDir.X);
    float HalfWidth = Road.Width * 0.5f;

    Polygon.Add(Road.Start + RoadNormal * HalfWidth);
    Polygon.Add(Road.End + RoadNormal * HalfWidth);
    Polygon.Add(Road.End - RoadNormal * HalfWidth);
    Polygon.Add(Road.Start - RoadNormal * HalfWidth);

    return Polygon;
}

void FForge2RoadUtils::SmoothNetwork(FForge2CityData& City, float MaxAngle)
{
    // Implementation would iterate through intersections and smooth sharp angles
    // This is a placeholder for the full implementation
}
