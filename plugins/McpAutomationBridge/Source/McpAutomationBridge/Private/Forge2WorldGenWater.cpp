// Forge2WorldGenWater.cpp
// FORGE2 Ultimate AAA Game Development MCP Extension
// Water Bodies and Environment Population Generation Implementation

#include "Forge2WorldGenWater.h"

// ============================================================================
// FForge2RiverGenerator Implementation
// ============================================================================

FForge2RiverGenerator::FForge2RiverGenerator()
{
}

FForge2RiverData FForge2RiverGenerator::Generate(const FForge2RiverParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    FForge2RiverData Result;
    Result.SourceLocation = Params.SourceLocation;
    Result.MouthLocation = Params.TargetLocation;

    // Convert world positions to grid coordinates
    FIntPoint StartGrid(
        FMath::Clamp(FMath::FloorToInt(Params.SourceLocation.X / CellSize), 0, MapWidth - 1),
        FMath::Clamp(FMath::FloorToInt(Params.SourceLocation.Y / CellSize), 0, MapHeight - 1)
    );
    FIntPoint EndGrid(
        FMath::Clamp(FMath::FloorToInt(Params.TargetLocation.X / CellSize), 0, MapWidth - 1),
        FMath::Clamp(FMath::FloorToInt(Params.TargetLocation.Y / CellSize), 0, MapHeight - 1)
    );

    // Find path using A*
    TArray<FIntPoint> GridPath = FindRiverPath(StartGrid, EndGrid, HeightMap, MapWidth, MapHeight, Params.bFollowTerrain);

    if (GridPath.Num() < 2)
    {
        // Fallback to direct path
        GridPath.Add(StartGrid);
        GridPath.Add(EndGrid);
    }

    // Convert to world coordinates
    TArray<FVector> WorldPath;
    for (const FIntPoint& GridPoint : GridPath)
    {
        int32 Index = GridPoint.Y * MapWidth + GridPoint.X;
        float Height = (Index >= 0 && Index < HeightMap.Num()) ? HeightMap[Index] : 0.0f;
        WorldPath.Add(FVector(GridPoint.X * CellSize, GridPoint.Y * CellSize, Height));
    }

    // Apply meandering
    if (Params.Meandering > 0.0f)
    {
        WorldPath = ApplyMeandering(WorldPath, Params.Meandering, Params.Seed);
    }

    // Interpolate path with splines
    Result.SplinePoints = InterpolatePath(WorldPath, Params);

    // Calculate total length
    Result.TotalLength = 0.0f;
    for (int32 i = 1; i < Result.SplinePoints.Num(); i++)
    {
        Result.TotalLength += FVector::Dist(Result.SplinePoints[i - 1].Location, Result.SplinePoints[i].Location);
    }

    // Calculate averages
    float TotalWidth = 0.0f;
    float TotalDepth = 0.0f;
    for (const FForge2RiverPoint& Point : Result.SplinePoints)
    {
        TotalWidth += Point.Width;
        TotalDepth += Point.Depth;
    }
    Result.AverageWidth = Result.SplinePoints.Num() > 0 ? TotalWidth / Result.SplinePoints.Num() : 0.0f;
    Result.AverageDepth = Result.SplinePoints.Num() > 0 ? TotalDepth / Result.SplinePoints.Num() : 0.0f;

    // Detect waterfalls
    if (Params.bCreateWaterfalls)
    {
        Result.WaterfallLocations = DetectWaterfalls(Result.SplinePoints, Params.WaterfallThreshold);
        Result.bHasWaterfall = Result.WaterfallLocations.Num() > 0;
    }

    return Result;
}

FForge2RiverData FForge2RiverGenerator::GenerateFromGradient(const FForge2RiverParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    FForge2RiverData Result;
    Result.SourceLocation = Params.SourceLocation;

    FRandomStream Random(Params.Seed);

    // Start from source and follow downhill
    FIntPoint CurrentGrid(
        FMath::Clamp(FMath::FloorToInt(Params.SourceLocation.X / CellSize), 0, MapWidth - 1),
        FMath::Clamp(FMath::FloorToInt(Params.SourceLocation.Y / CellSize), 0, MapHeight - 1)
    );

    TArray<FVector> WorldPath;
    TSet<FIntPoint> Visited;

    int32 MaxIterations = MapWidth * MapHeight;
    int32 Iterations = 0;

    while (Iterations < MaxIterations)
    {
        Iterations++;

        int32 CurrentIndex = CurrentGrid.Y * MapWidth + CurrentGrid.X;
        if (CurrentIndex < 0 || CurrentIndex >= HeightMap.Num())
            break;

        float CurrentHeight = HeightMap[CurrentIndex];
        WorldPath.Add(FVector(CurrentGrid.X * CellSize, CurrentGrid.Y * CellSize, CurrentHeight));
        Visited.Add(CurrentGrid);

        // Find lowest neighbor
        FIntPoint BestNeighbor = CurrentGrid;
        float LowestHeight = CurrentHeight;

        static const FIntPoint Neighbors[] = {
            FIntPoint(-1, 0), FIntPoint(1, 0), FIntPoint(0, -1), FIntPoint(0, 1),
            FIntPoint(-1, -1), FIntPoint(1, -1), FIntPoint(-1, 1), FIntPoint(1, 1)
        };

        for (const FIntPoint& Offset : Neighbors)
        {
            FIntPoint NeighborGrid = CurrentGrid + Offset;
            if (NeighborGrid.X < 0 || NeighborGrid.X >= MapWidth ||
                NeighborGrid.Y < 0 || NeighborGrid.Y >= MapHeight)
                continue;

            if (Visited.Contains(NeighborGrid))
                continue;

            int32 NeighborIndex = NeighborGrid.Y * MapWidth + NeighborGrid.X;
            float NeighborHeight = HeightMap[NeighborIndex];

            // Add some randomness for meandering
            float AdjustedHeight = NeighborHeight + Random.FRandRange(-Params.Meandering * 10.0f, Params.Meandering * 10.0f);

            if (AdjustedHeight < LowestHeight)
            {
                LowestHeight = AdjustedHeight;
                BestNeighbor = NeighborGrid;
            }
        }

        // Check if we're stuck (no lower neighbor)
        if (BestNeighbor == CurrentGrid)
        {
            // Reached a local minimum (lake or ocean)
            break;
        }

        CurrentGrid = BestNeighbor;

        // Check if reached edge (ocean)
        if (CurrentGrid.X == 0 || CurrentGrid.X == MapWidth - 1 ||
            CurrentGrid.Y == 0 || CurrentGrid.Y == MapHeight - 1)
        {
            break;
        }
    }

    if (WorldPath.Num() < 2)
    {
        return Result;
    }

    Result.MouthLocation = WorldPath.Last();

    // Apply meandering
    if (Params.Meandering > 0.0f)
    {
        WorldPath = ApplyMeandering(WorldPath, Params.Meandering, Params.Seed);
    }

    // Interpolate path with splines
    Result.SplinePoints = InterpolatePath(WorldPath, Params);

    // Calculate total length
    Result.TotalLength = 0.0f;
    for (int32 i = 1; i < Result.SplinePoints.Num(); i++)
    {
        Result.TotalLength += FVector::Dist(Result.SplinePoints[i - 1].Location, Result.SplinePoints[i].Location);
    }

    // Calculate averages
    float TotalWidth = 0.0f;
    float TotalDepth = 0.0f;
    for (const FForge2RiverPoint& Point : Result.SplinePoints)
    {
        TotalWidth += Point.Width;
        TotalDepth += Point.Depth;
    }
    Result.AverageWidth = Result.SplinePoints.Num() > 0 ? TotalWidth / Result.SplinePoints.Num() : 0.0f;
    Result.AverageDepth = Result.SplinePoints.Num() > 0 ? TotalDepth / Result.SplinePoints.Num() : 0.0f;

    // Detect waterfalls
    if (Params.bCreateWaterfalls)
    {
        Result.WaterfallLocations = DetectWaterfalls(Result.SplinePoints, Params.WaterfallThreshold);
        Result.bHasWaterfall = Result.WaterfallLocations.Num() > 0;
    }

    return Result;
}

TArray<float> FForge2RiverGenerator::CalculateTerrainCarving(const FForge2RiverData& River, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize, float CarveDepth)
{
    TArray<float> CarvedHeightMap = HeightMap;

    for (const FForge2RiverPoint& Point : River.SplinePoints)
    {
        int32 CenterX = FMath::FloorToInt(Point.Location.X / CellSize);
        int32 CenterY = FMath::FloorToInt(Point.Location.Y / CellSize);
        int32 RadiusCells = FMath::CeilToInt(Point.Width / (2.0f * CellSize)) + 2;

        for (int32 DY = -RadiusCells; DY <= RadiusCells; DY++)
        {
            for (int32 DX = -RadiusCells; DX <= RadiusCells; DX++)
            {
                int32 X = CenterX + DX;
                int32 Y = CenterY + DY;

                if (X < 0 || X >= MapWidth || Y < 0 || Y >= MapHeight)
                    continue;

                float WorldX = X * CellSize;
                float WorldY = Y * CellSize;
                float DistFromCenter = FVector2D::Distance(FVector2D(WorldX, WorldY), FVector2D(Point.Location.X, Point.Location.Y));
                float HalfWidth = Point.Width * 0.5f;

                if (DistFromCenter <= HalfWidth + Point.BankHeight)
                {
                    int32 Index = Y * MapWidth + X;
                    float NormalizedDist = DistFromCenter / (HalfWidth + Point.BankHeight);
                    float CarveAmount = 0.0f;

                    if (DistFromCenter <= HalfWidth)
                    {
                        // River bed - full carve
                        float BedProfile = 1.0f - FMath::Pow(DistFromCenter / HalfWidth, 2.0f);
                        CarveAmount = CarveDepth + Point.Depth * BedProfile;
                    }
                    else
                    {
                        // Bank - gradual slope
                        float BankDist = (DistFromCenter - HalfWidth) / Point.BankHeight;
                        CarveAmount = CarveDepth * (1.0f - BankDist);
                    }

                    CarvedHeightMap[Index] = FMath::Min(CarvedHeightMap[Index], HeightMap[Index] - CarveAmount);
                }
            }
        }
    }

    return CarvedHeightMap;
}

TArray<FIntPoint> FForge2RiverGenerator::FindRiverPath(FIntPoint Start, FIntPoint End, const TArray<float>& HeightMap, int32 Width, int32 Height, bool bPreferDownhill)
{
    TArray<FIntPoint> Path;

    // Simple A* implementation
    struct FPathNode
    {
        FIntPoint Position;
        float GCost;
        float HCost;
        int32 ParentIndex;

        float FCost() const { return GCost + HCost; }
    };

    TArray<FPathNode> OpenList;
    TSet<FIntPoint> ClosedSet;
    TMap<FIntPoint, int32> NodeIndices;

    FPathNode StartNode;
    StartNode.Position = Start;
    StartNode.GCost = 0.0f;
    StartNode.HCost = FVector2D::Distance(FVector2D(Start.X, Start.Y), FVector2D(End.X, End.Y));
    StartNode.ParentIndex = -1;

    OpenList.Add(StartNode);
    NodeIndices.Add(Start, 0);

    static const FIntPoint Neighbors[] = {
        FIntPoint(-1, 0), FIntPoint(1, 0), FIntPoint(0, -1), FIntPoint(0, 1),
        FIntPoint(-1, -1), FIntPoint(1, -1), FIntPoint(-1, 1), FIntPoint(1, 1)
    };

    while (OpenList.Num() > 0)
    {
        // Find lowest F cost
        int32 BestIndex = 0;
        float BestFCost = OpenList[0].FCost();
        for (int32 i = 1; i < OpenList.Num(); i++)
        {
            if (OpenList[i].FCost() < BestFCost)
            {
                BestFCost = OpenList[i].FCost();
                BestIndex = i;
            }
        }

        FPathNode CurrentNode = OpenList[BestIndex];
        OpenList.RemoveAt(BestIndex);
        ClosedSet.Add(CurrentNode.Position);

        // Check if reached goal
        if (CurrentNode.Position == End)
        {
            // Reconstruct path
            TArray<FIntPoint> ReversePath;
            int32 CurrentIndex = ClosedSet.Num() - 1;

            // Find the node in our tracking
            FIntPoint Pos = End;
            while (true)
            {
                ReversePath.Add(Pos);
                if (Pos == Start)
                    break;

                // Find parent - this is simplified, in practice you'd store parent pointers
                bool bFoundParent = false;
                for (const FIntPoint& Neighbor : Neighbors)
                {
                    FIntPoint NeighborPos = Pos + Neighbor;
                    if (ClosedSet.Contains(NeighborPos) && !ReversePath.Contains(NeighborPos))
                    {
                        int32 CurrentIdx = Pos.Y * Width + Pos.X;
                        int32 NeighborIdx = NeighborPos.Y * Width + NeighborPos.X;

                        if (CurrentIdx >= 0 && CurrentIdx < HeightMap.Num() &&
                            NeighborIdx >= 0 && NeighborIdx < HeightMap.Num())
                        {
                            if (HeightMap[NeighborIdx] >= HeightMap[CurrentIdx])
                            {
                                Pos = NeighborPos;
                                bFoundParent = true;
                                break;
                            }
                        }
                    }
                }

                if (!bFoundParent)
                {
                    // Fallback: find any closed neighbor
                    for (const FIntPoint& Neighbor : Neighbors)
                    {
                        FIntPoint NeighborPos = Pos + Neighbor;
                        if (ClosedSet.Contains(NeighborPos) && !ReversePath.Contains(NeighborPos))
                        {
                            Pos = NeighborPos;
                            break;
                        }
                    }
                }

                if (ReversePath.Num() > Width * Height)
                    break; // Safety
            }

            // Reverse to get correct order
            for (int32 i = ReversePath.Num() - 1; i >= 0; i--)
            {
                Path.Add(ReversePath[i]);
            }

            return Path;
        }

        // Explore neighbors
        for (const FIntPoint& Offset : Neighbors)
        {
            FIntPoint NeighborPos = CurrentNode.Position + Offset;

            if (NeighborPos.X < 0 || NeighborPos.X >= Width ||
                NeighborPos.Y < 0 || NeighborPos.Y >= Height)
                continue;

            if (ClosedSet.Contains(NeighborPos))
                continue;

            int32 CurrentIndex = CurrentNode.Position.Y * Width + CurrentNode.Position.X;
            int32 NeighborIndex = NeighborPos.Y * Width + NeighborPos.X;

            float MovementCost = (Offset.X != 0 && Offset.Y != 0) ? 1.414f : 1.0f;

            // Add height cost if preferring downhill
            if (bPreferDownhill)
            {
                float HeightDiff = HeightMap[NeighborIndex] - HeightMap[CurrentIndex];
                if (HeightDiff > 0)
                {
                    MovementCost += HeightDiff * 0.1f; // Penalty for going uphill
                }
                else
                {
                    MovementCost += HeightDiff * -0.05f; // Small bonus for going downhill
                }
            }

            float NewGCost = CurrentNode.GCost + MovementCost;

            // Check if this path is better
            int32* ExistingIndex = NodeIndices.Find(NeighborPos);
            if (ExistingIndex)
            {
                if (OpenList.IsValidIndex(*ExistingIndex) && NewGCost < OpenList[*ExistingIndex].GCost)
                {
                    OpenList[*ExistingIndex].GCost = NewGCost;
                    OpenList[*ExistingIndex].ParentIndex = ClosedSet.Num() - 1;
                }
            }
            else
            {
                FPathNode NewNode;
                NewNode.Position = NeighborPos;
                NewNode.GCost = NewGCost;
                NewNode.HCost = FVector2D::Distance(FVector2D(NeighborPos.X, NeighborPos.Y), FVector2D(End.X, End.Y));
                NewNode.ParentIndex = ClosedSet.Num() - 1;

                NodeIndices.Add(NeighborPos, OpenList.Num());
                OpenList.Add(NewNode);
            }
        }
    }

    // No path found - return direct line
    Path.Add(Start);
    Path.Add(End);
    return Path;
}

TArray<FVector> FForge2RiverGenerator::ApplyMeandering(const TArray<FVector>& Path, float Amount, int32 Seed)
{
    if (Path.Num() < 3 || Amount <= 0.0f)
    {
        return Path;
    }

    TArray<FVector> MeanderedPath;
    MeanderedPath.Add(Path[0]); // Keep start point

    NoiseGenerator.SetSeed(Seed);

    for (int32 i = 1; i < Path.Num() - 1; i++)
    {
        FVector Point = Path[i];
        FVector Direction = (Path[i + 1] - Path[i - 1]).GetSafeNormal();
        FVector Perpendicular(-Direction.Y, Direction.X, 0.0f);

        float NoiseValue = NoiseGenerator.Noise2D(Point.X * 0.01f, Point.Y * 0.01f);
        float Offset = NoiseValue * Amount * 100.0f;

        Point += Perpendicular * Offset;
        MeanderedPath.Add(Point);
    }

    MeanderedPath.Add(Path.Last()); // Keep end point

    return MeanderedPath;
}

TArray<FForge2RiverPoint> FForge2RiverGenerator::InterpolatePath(const TArray<FVector>& Path, const FForge2RiverParams& Params)
{
    TArray<FForge2RiverPoint> InterpolatedPoints;

    if (Path.Num() < 2)
    {
        return InterpolatedPoints;
    }

    float TotalDistance = 0.0f;
    for (int32 i = 1; i < Path.Num(); i++)
    {
        TotalDistance += FVector::Dist(Path[i - 1], Path[i]);
    }

    // Create points along the path
    float SegmentLength = 50.0f; // Distance between spline points
    int32 NumPoints = FMath::Max(2, FMath::CeilToInt(TotalDistance / SegmentLength));

    for (int32 i = 0; i < NumPoints; i++)
    {
        float T = (float)i / (float)(NumPoints - 1);

        // Find position along path using Catmull-Rom
        float TargetDist = T * TotalDistance;
        float AccumulatedDist = 0.0f;
        int32 SegmentIndex = 0;

        for (int32 j = 1; j < Path.Num(); j++)
        {
            float SegmentDist = FVector::Dist(Path[j - 1], Path[j]);
            if (AccumulatedDist + SegmentDist >= TargetDist)
            {
                SegmentIndex = j - 1;
                break;
            }
            AccumulatedDist += SegmentDist;
        }

        float SegmentDist = FVector::Dist(Path[SegmentIndex], Path[FMath::Min(SegmentIndex + 1, Path.Num() - 1)]);
        float SegmentT = (SegmentDist > 0.0f) ? (TargetDist - AccumulatedDist) / SegmentDist : 0.0f;

        // Get control points for Catmull-Rom
        FVector P0 = Path[FMath::Max(0, SegmentIndex - 1)];
        FVector P1 = Path[SegmentIndex];
        FVector P2 = Path[FMath::Min(SegmentIndex + 1, Path.Num() - 1)];
        FVector P3 = Path[FMath::Min(SegmentIndex + 2, Path.Num() - 1)];

        // Catmull-Rom interpolation
        float T2 = SegmentT * SegmentT;
        float T3 = T2 * SegmentT;

        FVector Position = 0.5f * (
            (2.0f * P1) +
            (-P0 + P2) * SegmentT +
            (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * T2 +
            (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * T3
        );

        FForge2RiverPoint Point;
        Point.Location = Position;
        Point.Width = FMath::Lerp(Params.StartWidth, Params.EndWidth, T);
        Point.Depth = FMath::Lerp(Params.StartDepth, Params.EndDepth, T);
        Point.FlowSpeed = 1.0f; // Could vary based on slope
        Point.BankHeight = Params.BankSlope;

        // Calculate flow direction from path tangent
        FVector Tangent = (P2 - P0).GetSafeNormal();
        Point.FlowDirection = Tangent;

        InterpolatedPoints.Add(Point);
    }

    return InterpolatedPoints;
}

TArray<FVector> FForge2RiverGenerator::DetectWaterfalls(const TArray<FForge2RiverPoint>& Points, float Threshold)
{
    TArray<FVector> Waterfalls;

    for (int32 i = 1; i < Points.Num(); i++)
    {
        float HeightDiff = Points[i - 1].Location.Z - Points[i].Location.Z;
        float HorizontalDist = FVector2D::Distance(
            FVector2D(Points[i - 1].Location.X, Points[i - 1].Location.Y),
            FVector2D(Points[i].Location.X, Points[i].Location.Y)
        );

        if (HorizontalDist > 0.0f && HeightDiff / HorizontalDist > Threshold / 100.0f)
        {
            Waterfalls.Add((Points[i - 1].Location + Points[i].Location) * 0.5f);
        }
    }

    return Waterfalls;
}

// ============================================================================
// FForge2LakeGenerator Implementation
// ============================================================================

FForge2LakeGenerator::FForge2LakeGenerator()
{
}

FForge2LakeData FForge2LakeGenerator::Generate(const FForge2LakeParams& Params)
{
    FForge2LakeData Result;
    Result.Center = Params.Center;
    Result.MaxDepth = Params.MaxDepth;
    Result.WaterLevel = Params.Center.Z;

    // Generate shoreline
    Result.Shoreline = GenerateShoreline(Params.Center, Params.Radius, Params.ShorelineIrregularity, Params.ShorelineResolution, Params.Seed);

    // Create 2D polygon for bounds checking
    for (const FForge2ShorelinePoint& Point : Result.Shoreline)
    {
        Result.ShorelinePolygon.Add(FVector2D(Point.Location.X, Point.Location.Y));
    }

    // Calculate surface area (approximate using polygon area formula)
    float Area = 0.0f;
    int32 N = Result.ShorelinePolygon.Num();
    for (int32 i = 0; i < N; i++)
    {
        int32 j = (i + 1) % N;
        Area += Result.ShorelinePolygon[i].X * Result.ShorelinePolygon[j].Y;
        Area -= Result.ShorelinePolygon[j].X * Result.ShorelinePolygon[i].Y;
    }
    Result.SurfaceArea = FMath::Abs(Area) * 0.5f;

    // Generate islands
    if (Params.IslandCount > 0)
    {
        Result.IslandLocations = GenerateIslands(Result, Params.IslandCount, Params.IslandSizeRange, Params.Seed);
    }

    return Result;
}

FForge2LakeData FForge2LakeGenerator::GenerateFromDepression(FVector Center, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize, float WaterLevel)
{
    FForge2LakeData Result;
    Result.Center = Center;
    Result.WaterLevel = WaterLevel;

    // Flood fill to find lake extent
    int32 CenterX = FMath::FloorToInt(Center.X / CellSize);
    int32 CenterY = FMath::FloorToInt(Center.Y / CellSize);

    TSet<FIntPoint> LakeCells;
    TArray<FIntPoint> Queue;
    Queue.Add(FIntPoint(CenterX, CenterY));

    while (Queue.Num() > 0)
    {
        FIntPoint Current = Queue.Pop();

        if (Current.X < 0 || Current.X >= MapWidth ||
            Current.Y < 0 || Current.Y >= MapHeight)
            continue;

        if (LakeCells.Contains(Current))
            continue;

        int32 Index = Current.Y * MapWidth + Current.X;
        if (HeightMap[Index] > WaterLevel)
            continue;

        LakeCells.Add(Current);

        Queue.Add(FIntPoint(Current.X - 1, Current.Y));
        Queue.Add(FIntPoint(Current.X + 1, Current.Y));
        Queue.Add(FIntPoint(Current.X, Current.Y - 1));
        Queue.Add(FIntPoint(Current.X, Current.Y + 1));
    }

    // Find shoreline (cells adjacent to non-lake cells)
    TArray<FForge2ShorelinePoint> ShorelinePoints;
    for (const FIntPoint& Cell : LakeCells)
    {
        bool bIsEdge = false;
        FIntPoint Neighbors[] = {
            FIntPoint(Cell.X - 1, Cell.Y),
            FIntPoint(Cell.X + 1, Cell.Y),
            FIntPoint(Cell.X, Cell.Y - 1),
            FIntPoint(Cell.X, Cell.Y + 1)
        };

        for (const FIntPoint& Neighbor : Neighbors)
        {
            if (!LakeCells.Contains(Neighbor))
            {
                bIsEdge = true;
                break;
            }
        }

        if (bIsEdge)
        {
            FForge2ShorelinePoint Point;
            int32 Index = Cell.Y * MapWidth + Cell.X;
            Point.Location = FVector(Cell.X * CellSize, Cell.Y * CellSize, WaterLevel);
            Point.WaterDepthAtShore = WaterLevel - HeightMap[Index];

            // Calculate normal (pointing outward)
            FVector Normal = FVector::ZeroVector;
            for (const FIntPoint& Neighbor : Neighbors)
            {
                if (!LakeCells.Contains(Neighbor))
                {
                    Normal += FVector(Neighbor.X - Cell.X, Neighbor.Y - Cell.Y, 0.0f);
                }
            }
            Point.Normal = Normal.GetSafeNormal();
            Point.BeachWidth = 50.0f;

            ShorelinePoints.Add(Point);
        }
    }

    Result.Shoreline = ShorelinePoints;

    // Calculate max depth
    Result.MaxDepth = 0.0f;
    for (const FIntPoint& Cell : LakeCells)
    {
        int32 Index = Cell.Y * MapWidth + Cell.X;
        float Depth = WaterLevel - HeightMap[Index];
        Result.MaxDepth = FMath::Max(Result.MaxDepth, Depth);
    }

    // Calculate surface area
    Result.SurfaceArea = LakeCells.Num() * CellSize * CellSize;

    // Create polygon
    for (const FForge2ShorelinePoint& Point : Result.Shoreline)
    {
        Result.ShorelinePolygon.Add(FVector2D(Point.Location.X, Point.Location.Y));
    }

    return Result;
}

TArray<float> FForge2LakeGenerator::CalculateDepthMap(const FForge2LakeData& Lake, int32 Resolution)
{
    TArray<float> DepthMap;
    DepthMap.SetNum(Resolution * Resolution);

    // Find bounds
    FVector2D Min(MAX_FLT, MAX_FLT);
    FVector2D Max(-MAX_FLT, -MAX_FLT);
    for (const FVector2D& Point : Lake.ShorelinePolygon)
    {
        Min.X = FMath::Min(Min.X, Point.X);
        Min.Y = FMath::Min(Min.Y, Point.Y);
        Max.X = FMath::Max(Max.X, Point.X);
        Max.Y = FMath::Max(Max.Y, Point.Y);
    }

    FVector2D Size = Max - Min;
    float CellSize = FMath::Max(Size.X, Size.Y) / (float)Resolution;

    FVector2D Center2D(Lake.Center.X, Lake.Center.Y);

    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            FVector2D Point = Min + FVector2D(X * CellSize, Y * CellSize);

            // Check if inside polygon (ray casting)
            bool bInside = false;
            int32 N = Lake.ShorelinePolygon.Num();
            for (int32 i = 0, j = N - 1; i < N; j = i++)
            {
                if (((Lake.ShorelinePolygon[i].Y > Point.Y) != (Lake.ShorelinePolygon[j].Y > Point.Y)) &&
                    (Point.X < (Lake.ShorelinePolygon[j].X - Lake.ShorelinePolygon[i].X) *
                    (Point.Y - Lake.ShorelinePolygon[i].Y) / (Lake.ShorelinePolygon[j].Y - Lake.ShorelinePolygon[i].Y) +
                    Lake.ShorelinePolygon[i].X))
                {
                    bInside = !bInside;
                }
            }

            if (bInside)
            {
                // Calculate depth based on distance from center
                float DistFromCenter = FVector2D::Distance(Point, Center2D);
                float MaxDist = FMath::Max(Size.X, Size.Y) * 0.5f;
                float NormalizedDist = DistFromCenter / MaxDist;
                float Depth = Lake.MaxDepth * (1.0f - NormalizedDist * NormalizedDist);
                DepthMap[Y * Resolution + X] = Depth;
            }
            else
            {
                DepthMap[Y * Resolution + X] = 0.0f;
            }
        }
    }

    return DepthMap;
}

TArray<FForge2ShorelinePoint> FForge2LakeGenerator::GenerateShoreline(FVector Center, float Radius, float Irregularity, int32 Resolution, int32 Seed)
{
    TArray<FForge2ShorelinePoint> Points;
    NoiseGenerator.SetSeed(Seed);

    for (int32 i = 0; i < Resolution; i++)
    {
        float Angle = (float)i / (float)Resolution * 2.0f * PI;

        // Apply noise to radius
        float NoiseValue = NoiseGenerator.Noise2D(FMath::Cos(Angle) * 2.0f, FMath::Sin(Angle) * 2.0f);
        float ModifiedRadius = Radius * (1.0f + NoiseValue * Irregularity);

        FForge2ShorelinePoint Point;
        Point.Location = Center + FVector(FMath::Cos(Angle) * ModifiedRadius, FMath::Sin(Angle) * ModifiedRadius, 0.0f);
        Point.Normal = (Point.Location - Center).GetSafeNormal();
        Point.Normal.Z = 0.0f;
        Point.BeachWidth = 50.0f;
        Point.WaterDepthAtShore = 0.0f;
        Point.bIsCliff = FMath::FRand() < 0.1f; // 10% chance of cliff

        Points.Add(Point);
    }

    return Points;
}

TArray<FVector> FForge2LakeGenerator::GenerateIslands(const FForge2LakeData& Lake, int32 Count, float SizeRange, int32 Seed)
{
    TArray<FVector> Islands;
    FRandomStream Random(Seed);

    for (int32 i = 0; i < Count; i++)
    {
        // Random position within lake
        float Angle = Random.FRandRange(0.0f, 2.0f * PI);
        float DistFromCenter = Random.FRandRange(0.2f, 0.6f); // 20-60% of distance to shore

        // Find approximate radius at this angle
        float MinDist = MAX_FLT;
        for (const FForge2ShorelinePoint& ShorePoint : Lake.Shoreline)
        {
            float ShoreAngle = FMath::Atan2(ShorePoint.Location.Y - Lake.Center.Y, ShorePoint.Location.X - Lake.Center.X);
            if (FMath::Abs(ShoreAngle - Angle) < 0.1f)
            {
                float Dist = FVector2D::Distance(FVector2D(ShorePoint.Location.X, ShorePoint.Location.Y), FVector2D(Lake.Center.X, Lake.Center.Y));
                MinDist = FMath::Min(MinDist, Dist);
            }
        }

        if (MinDist == MAX_FLT)
        {
            MinDist = 500.0f; // Default
        }

        FVector IslandPos = Lake.Center + FVector(FMath::Cos(Angle) * MinDist * DistFromCenter, FMath::Sin(Angle) * MinDist * DistFromCenter, Lake.WaterLevel + 5.0f);
        Islands.Add(IslandPos);
    }

    return Islands;
}

bool FForge2LakeGenerator::IsCliffLocation(FVector Location, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize, float Threshold)
{
    int32 X = FMath::FloorToInt(Location.X / CellSize);
    int32 Y = FMath::FloorToInt(Location.Y / CellSize);

    if (X < 1 || X >= MapWidth - 1 || Y < 1 || Y >= MapHeight - 1)
        return false;

    // Check slope
    int32 Index = Y * MapWidth + X;
    float CurrentHeight = HeightMap[Index];

    float MaxSlope = 0.0f;
    static const FIntPoint Neighbors[] = {
        FIntPoint(-1, 0), FIntPoint(1, 0), FIntPoint(0, -1), FIntPoint(0, 1)
    };

    for (const FIntPoint& Offset : Neighbors)
    {
        int32 NeighborIndex = (Y + Offset.Y) * MapWidth + (X + Offset.X);
        float HeightDiff = FMath::Abs(HeightMap[NeighborIndex] - CurrentHeight);
        MaxSlope = FMath::Max(MaxSlope, HeightDiff / CellSize);
    }

    return MaxSlope > Threshold;
}

// ============================================================================
// FForge2OceanGenerator Implementation
// ============================================================================

FForge2OceanGenerator::FForge2OceanGenerator()
{
}

FForge2OceanData FForge2OceanGenerator::Generate(const FForge2OceanParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    FForge2OceanData Result;
    Result.SeaLevel = Params.SeaLevel;
    Result.BoundsMin = Params.BoundsMin;
    Result.BoundsMax = Params.BoundsMax;
    Result.WaveType = Params.WaveType;
    Result.WaveDirection = Params.WaveDirection;
    Result.TideAmplitude = Params.TideAmplitude;
    Result.TidePeriod = 720.0f; // 12 minutes

    ConfigureWaves(Result, Params.WaveType);

    if (Params.bGenerateCoastline)
    {
        Result.Coastline = GenerateCoastline(HeightMap, MapWidth, MapHeight, CellSize, Params.SeaLevel, Params.CoastlineResolution);
    }

    return Result;
}

float FForge2OceanGenerator::GetWaveHeight(FVector2D Position, float Time, const FForge2OceanData& Ocean) const
{
    // Gerstner wave approximation
    float WaveX = FVector2D::DotProduct(Position, FVector2D(Ocean.WaveDirection.X, Ocean.WaveDirection.Y));
    float Phase = WaveX * Ocean.WaveFrequency + Time;

    float Height = FMath::Sin(Phase) * Ocean.WaveAmplitude;

    // Add secondary wave for more realistic look
    float Phase2 = WaveX * Ocean.WaveFrequency * 0.5f + Time * 0.7f;
    Height += FMath::Sin(Phase2) * Ocean.WaveAmplitude * 0.3f;

    return Height;
}

float FForge2OceanGenerator::GetTideLevel(float Time, const FForge2OceanData& Ocean) const
{
    return Ocean.SeaLevel + FMath::Sin(Time / Ocean.TidePeriod * 2.0f * PI) * Ocean.TideAmplitude;
}

TArray<FForge2ShorelinePoint> FForge2OceanGenerator::GenerateCoastline(const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize, float SeaLevel, float Resolution)
{
    TArray<FForge2ShorelinePoint> Coastline;

    // Find cells at sea level (marching squares approach)
    for (int32 Y = 0; Y < Height - 1; Y++)
    {
        for (int32 X = 0; X < Width - 1; X++)
        {
            int32 Index00 = Y * Width + X;
            int32 Index10 = Y * Width + X + 1;
            int32 Index01 = (Y + 1) * Width + X;
            int32 Index11 = (Y + 1) * Width + X + 1;

            bool b00 = HeightMap[Index00] < SeaLevel;
            bool b10 = HeightMap[Index10] < SeaLevel;
            bool b01 = HeightMap[Index01] < SeaLevel;
            bool b11 = HeightMap[Index11] < SeaLevel;

            // If any corner is different, this is a coastline cell
            if (b00 != b10 || b00 != b01 || b00 != b11)
            {
                // Skip based on resolution
                if ((X % (int32)FMath::Max(1.0f, Resolution / CellSize) != 0) ||
                    (Y % (int32)FMath::Max(1.0f, Resolution / CellSize) != 0))
                    continue;

                FForge2ShorelinePoint Point;
                Point.Location = FVector(X * CellSize + CellSize * 0.5f, Y * CellSize + CellSize * 0.5f, SeaLevel);

                // Calculate normal
                FVector Normal = FVector::ZeroVector;
                if (!b00) Normal += FVector(-1, -1, 0);
                if (!b10) Normal += FVector(1, -1, 0);
                if (!b01) Normal += FVector(-1, 1, 0);
                if (!b11) Normal += FVector(1, 1, 0);
                Point.Normal = Normal.GetSafeNormal();

                Point.BeachWidth = 100.0f;
                Point.WaterDepthAtShore = 0.0f;

                // Check if cliff
                float MaxSlope = 0.0f;
                if (X > 0 && X < Width - 1 && Y > 0 && Y < Height - 1)
                {
                    float CenterHeight = HeightMap[Y * Width + X];
                    for (int32 DY = -1; DY <= 1; DY++)
                    {
                        for (int32 DX = -1; DX <= 1; DX++)
                        {
                            if (DX == 0 && DY == 0) continue;
                            float NeighborHeight = HeightMap[(Y + DY) * Width + (X + DX)];
                            float Slope = FMath::Abs(NeighborHeight - CenterHeight) / CellSize;
                            MaxSlope = FMath::Max(MaxSlope, Slope);
                        }
                    }
                }
                Point.bIsCliff = MaxSlope > 0.5f;

                Coastline.Add(Point);
            }
        }
    }

    return Coastline;
}

void FForge2OceanGenerator::ConfigureWaves(FForge2OceanData& Ocean, EForge2WaveType WaveType)
{
    switch (WaveType)
    {
    case EForge2WaveType::Calm:
        Ocean.WaveAmplitude = 10.0f;
        Ocean.WaveFrequency = 0.02f;
        break;
    case EForge2WaveType::Gentle:
        Ocean.WaveAmplitude = 30.0f;
        Ocean.WaveFrequency = 0.03f;
        break;
    case EForge2WaveType::Moderate:
        Ocean.WaveAmplitude = 60.0f;
        Ocean.WaveFrequency = 0.04f;
        break;
    case EForge2WaveType::Rough:
        Ocean.WaveAmplitude = 100.0f;
        Ocean.WaveFrequency = 0.05f;
        break;
    case EForge2WaveType::Stormy:
        Ocean.WaveAmplitude = 200.0f;
        Ocean.WaveFrequency = 0.06f;
        break;
    }
}

// ============================================================================
// FForge2WaterFlowSimulator Implementation
// ============================================================================

FForge2WaterFlowSimulator::FForge2WaterFlowSimulator()
    : GridWidth(0)
    , GridHeight(0)
    , CellSize(100.0f)
{
}

TArray<FForge2WaterFlowCell> FForge2WaterFlowSimulator::Simulate(const FForge2WaterFlowParams& Params, const TArray<float>& HeightMap)
{
    InitializeGrid(HeightMap, Params);

    for (int32 Step = 0; Step < Params.SimulationSteps; Step++)
    {
        SimulationStep(Params.TimeStep);

        if (Params.RainfallRate > 0.0f)
        {
            // Add rainfall
            for (FForge2WaterFlowCell& Cell : Grid)
            {
                Cell.WaterHeight += Params.RainfallRate * Params.TimeStep;
            }
        }

        ApplyEvaporation(Params.EvaporationRate * Params.TimeStep);
    }

    return Grid;
}

TArray<FVector2D> FForge2WaterFlowSimulator::GetFlowDirections() const
{
    TArray<FVector2D> Directions;
    Directions.Reserve(Grid.Num());

    for (const FForge2WaterFlowCell& Cell : Grid)
    {
        Directions.Add(Cell.FlowVelocity.GetSafeNormal());
    }

    return Directions;
}

TArray<float> FForge2WaterFlowSimulator::GetWaterAccumulation() const
{
    TArray<float> Accumulation;
    Accumulation.Reserve(Grid.Num());

    for (const FForge2WaterFlowCell& Cell : Grid)
    {
        Accumulation.Add(Cell.WaterHeight);
    }

    return Accumulation;
}

TArray<FForge2RiverData> FForge2WaterFlowSimulator::FindRiverPaths(float MinAccumulation, int32 MaxRivers)
{
    TArray<FForge2RiverData> Rivers;

    // Find cells with high water accumulation
    TArray<TPair<int32, float>> HighAccumulation;
    for (int32 i = 0; i < Grid.Num(); i++)
    {
        if (Grid[i].WaterHeight >= MinAccumulation)
        {
            HighAccumulation.Add(TPair<int32, float>(i, Grid[i].WaterHeight));
        }
    }

    // Sort by accumulation (highest first)
    HighAccumulation.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
    {
        return A.Value > B.Value;
    });

    // Create rivers from top accumulation points
    TSet<int32> UsedCells;
    for (int32 i = 0; i < FMath::Min(MaxRivers, HighAccumulation.Num()); i++)
    {
        int32 StartIndex = HighAccumulation[i].Key;
        if (UsedCells.Contains(StartIndex))
            continue;

        FForge2RiverData River;
        River.SourceLocation = FVector(
            (StartIndex % GridWidth) * CellSize,
            (StartIndex / GridWidth) * CellSize,
            Grid[StartIndex].TerrainHeight
        );

        // Follow flow direction
        TArray<FForge2RiverPoint> Points;
        int32 CurrentIndex = StartIndex;
        TSet<int32> PathCells;

        while (CurrentIndex >= 0 && CurrentIndex < Grid.Num() && PathCells.Num() < GridWidth * GridHeight)
        {
            PathCells.Add(CurrentIndex);
            UsedCells.Add(CurrentIndex);

            FForge2RiverPoint Point;
            Point.Location = FVector(
                (CurrentIndex % GridWidth) * CellSize,
                (CurrentIndex / GridWidth) * CellSize,
                Grid[CurrentIndex].TerrainHeight
            );
            Point.Width = FMath::Sqrt(Grid[CurrentIndex].WaterHeight) * 10.0f;
            Point.Depth = Grid[CurrentIndex].WaterHeight * 0.5f;
            Point.FlowSpeed = Grid[CurrentIndex].FlowVelocity.Size();
            Point.FlowDirection = FVector(Grid[CurrentIndex].FlowVelocity.X, Grid[CurrentIndex].FlowVelocity.Y, 0.0f);

            Points.Add(Point);

            // Find next cell in flow direction
            int32 X = CurrentIndex % GridWidth;
            int32 Y = CurrentIndex / GridWidth;

            FVector2D Flow = Grid[CurrentIndex].FlowVelocity;
            if (Flow.IsNearlyZero())
                break;

            int32 NextX = X + FMath::RoundToInt(FMath::Sign(Flow.X));
            int32 NextY = Y + FMath::RoundToInt(FMath::Sign(Flow.Y));

            NextX = FMath::Clamp(NextX, 0, GridWidth - 1);
            NextY = FMath::Clamp(NextY, 0, GridHeight - 1);

            int32 NextIndex = NextY * GridWidth + NextX;
            if (PathCells.Contains(NextIndex))
                break;

            CurrentIndex = NextIndex;
        }

        if (Points.Num() > 2)
        {
            River.SplinePoints = Points;
            River.MouthLocation = Points.Last().Location;
            Rivers.Add(River);
        }
    }

    return Rivers;
}

void FForge2WaterFlowSimulator::InitializeGrid(const TArray<float>& HeightMap, const FForge2WaterFlowParams& Params)
{
    GridWidth = Params.GridResolution;
    GridHeight = Params.GridResolution;
    CellSize = Params.CellSize;
    CurrentParams = Params;

    Grid.SetNum(GridWidth * GridHeight);

    // Initialize cells
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            int32 Index = Y * GridWidth + X;
            FForge2WaterFlowCell& Cell = Grid[Index];

            // Map heightmap to grid
            int32 HeightMapWidth = FMath::Sqrt((float)HeightMap.Num());
            int32 HX = (X * HeightMapWidth) / GridWidth;
            int32 HY = (Y * HeightMapWidth) / GridHeight;
            int32 HIndex = FMath::Clamp(HY * HeightMapWidth + HX, 0, HeightMap.Num() - 1);

            Cell.TerrainHeight = HeightMap[HIndex];
            Cell.WaterHeight = 0.0f;
            Cell.FlowVelocity = FVector2D::ZeroVector;
            Cell.Sediment = 0.0f;
            Cell.bIsSource = false;
            Cell.bIsSink = false;
        }
    }

    // Add water sources
    for (const FVector& Source : Params.WaterSources)
    {
        int32 X = FMath::Clamp(FMath::FloorToInt(Source.X / CellSize), 0, GridWidth - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(Source.Y / CellSize), 0, GridHeight - 1);
        Grid[Y * GridWidth + X].bIsSource = true;
        Grid[Y * GridWidth + X].WaterHeight = 100.0f;
    }

    // Mark edges as sinks
    for (int32 X = 0; X < GridWidth; X++)
    {
        Grid[X].bIsSink = true;
        Grid[(GridHeight - 1) * GridWidth + X].bIsSink = true;
    }
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        Grid[Y * GridWidth].bIsSink = true;
        Grid[Y * GridWidth + GridWidth - 1].bIsSink = true;
    }
}

void FForge2WaterFlowSimulator::SimulationStep(float DeltaTime)
{
    TArray<FForge2WaterFlowCell> NewGrid = Grid;

    for (int32 Y = 1; Y < GridHeight - 1; Y++)
    {
        for (int32 X = 1; X < GridWidth - 1; X++)
        {
            int32 Index = Y * GridWidth + X;
            FForge2WaterFlowCell& Cell = Grid[Index];

            if (Cell.bIsSource)
            {
                Cell.WaterHeight = 100.0f; // Constant source
            }

            if (Cell.bIsSink)
            {
                NewGrid[Index].WaterHeight = 0.0f;
                continue;
            }

            // Calculate outflow
            FVector2D Outflow = CalculateOutflow(X, Y);

            // Update water height
            float OutflowAmount = (FMath::Abs(Outflow.X) + FMath::Abs(Outflow.Y)) * DeltaTime;
            NewGrid[Index].WaterHeight = FMath::Max(0.0f, Cell.WaterHeight - OutflowAmount);
            NewGrid[Index].FlowVelocity = Outflow;

            // Transfer water to neighbors
            if (Outflow.X > 0)
            {
                NewGrid[Index + 1].WaterHeight += Outflow.X * DeltaTime;
            }
            else if (Outflow.X < 0)
            {
                NewGrid[Index - 1].WaterHeight -= Outflow.X * DeltaTime;
            }

            if (Outflow.Y > 0)
            {
                NewGrid[Index + GridWidth].WaterHeight += Outflow.Y * DeltaTime;
            }
            else if (Outflow.Y < 0)
            {
                NewGrid[Index - GridWidth].WaterHeight -= Outflow.Y * DeltaTime;
            }
        }
    }

    Grid = NewGrid;
}

FVector2D FForge2WaterFlowSimulator::CalculateOutflow(int32 X, int32 Y)
{
    int32 Index = Y * GridWidth + X;
    FForge2WaterFlowCell& Cell = Grid[Index];

    float SurfaceHeight = Cell.TerrainHeight + Cell.WaterHeight;

    FVector2D Outflow = FVector2D::ZeroVector;

    // Check all 4 neighbors
    struct FNeighbor { int32 DX, DY; };
    FNeighbor Neighbors[] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

    for (const FNeighbor& N : Neighbors)
    {
        int32 NX = X + N.DX;
        int32 NY = Y + N.DY;

        if (NX < 0 || NX >= GridWidth || NY < 0 || NY >= GridHeight)
            continue;

        int32 NIndex = NY * GridWidth + NX;
        FForge2WaterFlowCell& NeighborCell = Grid[NIndex];

        float NeighborSurface = NeighborCell.TerrainHeight + NeighborCell.WaterHeight;
        float HeightDiff = SurfaceHeight - NeighborSurface;

        if (HeightDiff > 0)
        {
            // Water flows downhill
            float FlowRate = HeightDiff * CurrentParams.Gravity * (1.0f - CurrentParams.Viscosity);
            FlowRate = FMath::Min(FlowRate, Cell.WaterHeight * 0.25f); // Don't transfer more than 25% per step

            if (N.DX != 0)
            {
                Outflow.X += N.DX * FlowRate;
            }
            else
            {
                Outflow.Y += N.DY * FlowRate;
            }
        }
    }

    return Outflow;
}

void FForge2WaterFlowSimulator::ApplyEvaporation(float Rate)
{
    for (FForge2WaterFlowCell& Cell : Grid)
    {
        if (!Cell.bIsSource)
        {
            Cell.WaterHeight = FMath::Max(0.0f, Cell.WaterHeight - Rate);
        }
    }
}

// ============================================================================
// FForge2PoissonDiskSampler Implementation
// ============================================================================

FForge2PoissonDiskSampler::FForge2PoissonDiskSampler()
{
}

TArray<FVector2D> FForge2PoissonDiskSampler::Generate(FVector2D Min, FVector2D Max, float MinDistance, int32 MaxAttempts, int32 Seed)
{
    TArray<FVector2D> Points;
    TArray<FVector2D> ActiveList;

    RandomStream.Initialize(Seed);

    float CellSize = MinDistance / FMath::Sqrt(2.0f);
    int32 GridWidth = FMath::CeilToInt((Max.X - Min.X) / CellSize);
    int32 GridHeight = FMath::CeilToInt((Max.Y - Min.Y) / CellSize);

    TMap<FIntPoint, TArray<int32>> Grid;

    // Add first point
    FVector2D FirstPoint(
        RandomStream.FRandRange(Min.X, Max.X),
        RandomStream.FRandRange(Min.Y, Max.Y)
    );
    Points.Add(FirstPoint);
    ActiveList.Add(FirstPoint);

    FIntPoint GridCell = GetGridCell(FirstPoint, Min, CellSize);
    Grid.FindOrAdd(GridCell).Add(0);

    while (ActiveList.Num() > 0)
    {
        int32 ActiveIndex = RandomStream.RandRange(0, ActiveList.Num() - 1);
        FVector2D ActivePoint = ActiveList[ActiveIndex];

        bool bFoundValid = false;

        for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
        {
            float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
            float Distance = RandomStream.FRandRange(MinDistance, MinDistance * 2.0f);

            FVector2D NewPoint = ActivePoint + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Distance;

            if (NewPoint.X < Min.X || NewPoint.X > Max.X ||
                NewPoint.Y < Min.Y || NewPoint.Y > Max.Y)
                continue;

            if (IsValidPoint(NewPoint, Points, MinDistance, Grid, CellSize))
            {
                Points.Add(NewPoint);
                ActiveList.Add(NewPoint);

                FIntPoint NewCell = GetGridCell(NewPoint, Min, CellSize);
                Grid.FindOrAdd(NewCell).Add(Points.Num() - 1);

                bFoundValid = true;
                break;
            }
        }

        if (!bFoundValid)
        {
            ActiveList.RemoveAt(ActiveIndex);
        }
    }

    return Points;
}

TArray<FVector> FForge2PoissonDiskSampler::Generate3D(FBox Bounds, float MinDistance, int32 MaxAttempts, int32 Seed)
{
    // Generate 2D points on XY plane
    TArray<FVector2D> Points2D = Generate(
        FVector2D(Bounds.Min.X, Bounds.Min.Y),
        FVector2D(Bounds.Max.X, Bounds.Max.Y),
        MinDistance,
        MaxAttempts,
        Seed
    );

    // Convert to 3D (Z will be set by terrain later)
    TArray<FVector> Points3D;
    for (const FVector2D& Point2D : Points2D)
    {
        Points3D.Add(FVector(Point2D.X, Point2D.Y, Bounds.Min.Z));
    }

    return Points3D;
}

bool FForge2PoissonDiskSampler::IsValidPoint(FVector2D Point, const TArray<FVector2D>& Points, float MinDistance, TMap<FIntPoint, TArray<int32>>& Grid, float CellSize)
{
    FIntPoint Cell = GetGridCell(Point, FVector2D::ZeroVector, CellSize);

    // Check surrounding cells
    for (int32 DY = -2; DY <= 2; DY++)
    {
        for (int32 DX = -2; DX <= 2; DX++)
        {
            FIntPoint NeighborCell = Cell + FIntPoint(DX, DY);
            TArray<int32>* CellPoints = Grid.Find(NeighborCell);
            if (CellPoints)
            {
                for (int32 PointIndex : *CellPoints)
                {
                    if (FVector2D::Distance(Point, Points[PointIndex]) < MinDistance)
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

FIntPoint FForge2PoissonDiskSampler::GetGridCell(FVector2D Point, FVector2D Min, float CellSize)
{
    return FIntPoint(
        FMath::FloorToInt((Point.X - Min.X) / CellSize),
        FMath::FloorToInt((Point.Y - Min.Y) / CellSize)
    );
}

// ============================================================================
// FForge2VegetationPlacer Implementation
// ============================================================================

FForge2VegetationPlacer::FForge2VegetationPlacer()
{
}

TArray<FForge2VegetationInstance> FForge2VegetationPlacer::Place(const FForge2VegetationParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    TArray<FForge2VegetationInstance> Instances;

    if (Params.MeshPaths.Num() == 0)
        return Instances;

    RandomStream.Initialize(Params.Seed);

    float DensityMultiplier = GetDensityMultiplier(Params.Density);
    int32 TargetCount = FMath::RoundToInt(
        (Params.Bounds.GetSize().X * Params.Bounds.GetSize().Y) /
        (Params.MinSpacing * Params.MinSpacing) * DensityMultiplier
    );

    // Generate placement points
    TArray<FVector2D> PlacementPoints;

    if (Params.bCluster)
    {
        // Generate cluster centers
        TArray<FVector2D> ClusterCenters = PoissonSampler.Generate(
            FVector2D(Params.Bounds.Min.X, Params.Bounds.Min.Y),
            FVector2D(Params.Bounds.Max.X, Params.Bounds.Max.Y),
            Params.ClusterRadius * 2.0f,
            30,
            Params.Seed
        );

        // Generate points around clusters
        for (const FVector2D& Center : ClusterCenters)
        {
            TArray<FVector> ClusterPoints = GenerateCluster(FVector(Center.X, Center.Y, 0.0f), Params.ClusterRadius, Params.ClusterCount, Params.Seed + PlacementPoints.Num());
            for (const FVector& Point : ClusterPoints)
            {
                PlacementPoints.Add(FVector2D(Point.X, Point.Y));
            }
        }
    }
    else
    {
        PlacementPoints = PoissonSampler.Generate(
            FVector2D(Params.Bounds.Min.X, Params.Bounds.Min.Y),
            FVector2D(Params.Bounds.Max.X, Params.Bounds.Max.Y),
            Params.MinSpacing,
            30,
            Params.Seed
        );
    }

    // Create instances
    for (const FVector2D& Point : PlacementPoints)
    {
        // Sample height from heightmap
        int32 X = FMath::Clamp(FMath::FloorToInt(Point.X / CellSize), 0, MapWidth - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(Point.Y / CellSize), 0, MapHeight - 1);
        int32 Index = Y * MapWidth + X;

        float Height = (Index >= 0 && Index < HeightMap.Num()) ? HeightMap[Index] : 0.0f;

        // Check elevation constraints
        if (Height < Params.MinElevation || Height > Params.MaxElevation)
            continue;

        // Check slope constraints
        float Slope = CalculateSlope(X, Y, HeightMap, MapWidth, MapHeight, CellSize);
        if (Slope < Params.MinSlope || Slope > Params.MaxSlope)
            continue;

        FForge2VegetationInstance Instance;

        FVector Location(Point.X, Point.Y, Height);
        FRotator Rotation = FRotator::ZeroRotator;

        if (Params.bRandomRotation)
        {
            Rotation.Yaw = RandomStream.FRandRange(0.0f, 360.0f);
        }

        if (Params.bAlignToSurface && X > 0 && X < MapWidth - 1 && Y > 0 && Y < MapHeight - 1)
        {
            // Calculate surface normal
            float HLeft = HeightMap[Y * MapWidth + X - 1];
            float HRight = HeightMap[Y * MapWidth + X + 1];
            float HDown = HeightMap[(Y - 1) * MapWidth + X];
            float HUp = HeightMap[(Y + 1) * MapWidth + X];

            FVector Normal = FVector(HLeft - HRight, HDown - HUp, 2.0f * CellSize).GetSafeNormal();
            Rotation = Normal.Rotation();
        }

        float Scale = RandomStream.FRandRange(Params.ScaleRange.X, Params.ScaleRange.Y);

        Instance.Transform = FTransform(Rotation, Location, FVector(Scale));
        Instance.MeshVariant = SelectMesh(Params.MeshWeights, RandomStream.FRand());
        Instance.MeshPath = Params.MeshPaths[Instance.MeshVariant % Params.MeshPaths.Num()];
        Instance.Scale = Scale;
        Instance.TintColor = FLinearColor::White;
        Instance.bCastShadow = true;

        Instances.Add(Instance);
    }

    return Instances;
}

TArray<FForge2VegetationInstance> FForge2VegetationPlacer::PlaceWithBiome(const FForge2VegetationParams& Params, const TArray<float>& HeightMap, const TArray<int32>& BiomeMap, const TArray<struct FForge2BiomeDefinition>& Biomes, int32 MapWidth, int32 MapHeight, float CellSize)
{
    // For now, just use basic placement
    // Biome-aware placement would check BiomeMap and use biome-specific vegetation
    return Place(Params, HeightMap, MapWidth, MapHeight, CellSize);
}

float FForge2VegetationPlacer::GetDensityMultiplier(EForge2VegetationDensity Density)
{
    switch (Density)
    {
    case EForge2VegetationDensity::Sparse: return 0.1f;
    case EForge2VegetationDensity::Light: return 0.3f;
    case EForge2VegetationDensity::Medium: return 0.5f;
    case EForge2VegetationDensity::Dense: return 0.8f;
    case EForge2VegetationDensity::Lush: return 1.0f;
    default: return 0.5f;
    }
}

float FForge2VegetationPlacer::CalculateSlope(int32 X, int32 Y, const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize)
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

int32 FForge2VegetationPlacer::SelectMesh(const TArray<float>& Weights, float Random)
{
    if (Weights.Num() == 0)
        return 0;

    float TotalWeight = 0.0f;
    for (float W : Weights)
    {
        TotalWeight += W;
    }

    float Target = Random * TotalWeight;
    float Accumulated = 0.0f;

    for (int32 i = 0; i < Weights.Num(); i++)
    {
        Accumulated += Weights[i];
        if (Accumulated >= Target)
            return i;
    }

    return Weights.Num() - 1;
}

TArray<FVector> FForge2VegetationPlacer::GenerateCluster(FVector Center, float Radius, int32 Count, int32 Seed)
{
    TArray<FVector> Points;
    FRandomStream ClusterRandom(Seed);

    for (int32 i = 0; i < Count; i++)
    {
        float Angle = ClusterRandom.FRandRange(0.0f, 2.0f * PI);
        float Distance = ClusterRandom.FRandRange(0.0f, Radius);

        FVector Point = Center + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
        Points.Add(Point);
    }

    return Points;
}

// ============================================================================
// FForge2PropScatterer Implementation
// ============================================================================

FForge2PropScatterer::FForge2PropScatterer()
{
}

TArray<FForge2PropInstance> FForge2PropScatterer::Scatter(const FForge2PropScatterParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    TArray<FForge2PropInstance> Instances;

    if (Params.MeshPaths.Num() == 0)
        return Instances;

    RandomStream.Initialize(Params.Seed);

    TArray<FVector> PlacementPoints;

    switch (Params.PlacementMode)
    {
    case EForge2PropPlacementMode::Random:
        for (int32 i = 0; i < Params.InstanceCount; i++)
        {
            FVector Point(
                RandomStream.FRandRange(Params.Bounds.Min.X, Params.Bounds.Max.X),
                RandomStream.FRandRange(Params.Bounds.Min.Y, Params.Bounds.Max.Y),
                0.0f
            );
            PlacementPoints.Add(Point);
        }
        break;

    case EForge2PropPlacementMode::Grid:
        PlacementPoints = GenerateGridPlacement(Params.Bounds, Params.MinSpacing);
        break;

    case EForge2PropPlacementMode::Clustered:
        PlacementPoints = GenerateClusteredPlacement(Params.Bounds, Params.InstanceCount, 10, Params.MinSpacing * 5.0f, Params.Seed);
        break;

    case EForge2PropPlacementMode::Poisson:
        {
            TArray<FVector2D> Points2D = PoissonSampler.Generate(
                FVector2D(Params.Bounds.Min.X, Params.Bounds.Min.Y),
                FVector2D(Params.Bounds.Max.X, Params.Bounds.Max.Y),
                Params.MinSpacing,
                30,
                Params.Seed
            );
            for (const FVector2D& P : Points2D)
            {
                PlacementPoints.Add(FVector(P.X, P.Y, 0.0f));
            }
        }
        break;

    default:
        break;
    }

    // Create instances
    for (const FVector& Point : PlacementPoints)
    {
        if (Instances.Num() >= Params.InstanceCount)
            break;

        // Check exclusion zones
        if (IsInExclusionZone(Point, Params.ExclusionPoints, Params.ExclusionRadius))
            continue;

        // Sample height
        int32 X = FMath::Clamp(FMath::FloorToInt(Point.X / CellSize), 0, MapWidth - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(Point.Y / CellSize), 0, MapHeight - 1);
        int32 Index = Y * MapWidth + X;
        float Height = (Index >= 0 && Index < HeightMap.Num()) ? HeightMap[Index] : 0.0f;

        FForge2PropInstance Instance;

        FVector Location(Point.X, Point.Y, Height);
        FRotator Rotation = FRotator::ZeroRotator;

        if (Params.bRandomRotation)
        {
            Rotation.Yaw = RandomStream.FRandRange(0.0f, 360.0f);
        }

        float Scale = RandomStream.FRandRange(Params.ScaleRange.X, Params.ScaleRange.Y);

        Instance.Transform = FTransform(Rotation, Location, FVector(Scale));

        int32 MeshIndex = 0;
        if (Params.MeshWeights.Num() > 0)
        {
            float TotalWeight = 0.0f;
            for (float W : Params.MeshWeights) TotalWeight += W;
            float Target = RandomStream.FRand() * TotalWeight;
            float Accumulated = 0.0f;
            for (int32 i = 0; i < Params.MeshWeights.Num(); i++)
            {
                Accumulated += Params.MeshWeights[i];
                if (Accumulated >= Target)
                {
                    MeshIndex = i;
                    break;
                }
            }
        }
        else
        {
            MeshIndex = RandomStream.RandRange(0, Params.MeshPaths.Num() - 1);
        }

        Instance.MeshPath = Params.MeshPaths[MeshIndex % Params.MeshPaths.Num()];
        Instance.bIsInteractable = false;
        Instance.bHasCollision = true;

        Instances.Add(Instance);
    }

    return Instances;
}

TArray<FForge2PropInstance> FForge2PropScatterer::ScatterAlongPath(const TArray<FVector>& PathPoints, float PathWidth, const FForge2PropScatterParams& Params)
{
    TArray<FForge2PropInstance> Instances;

    if (PathPoints.Num() < 2 || Params.MeshPaths.Num() == 0)
        return Instances;

    RandomStream.Initialize(Params.Seed);

    // Place props along path
    float PathLength = 0.0f;
    for (int32 i = 1; i < PathPoints.Num(); i++)
    {
        PathLength += FVector::Dist(PathPoints[i - 1], PathPoints[i]);
    }

    int32 NumProps = FMath::Max(1, FMath::FloorToInt(PathLength / Params.MinSpacing));

    for (int32 i = 0; i < NumProps && Instances.Num() < Params.InstanceCount; i++)
    {
        float T = (float)i / (float)NumProps;

        // Find position along path
        float TargetDist = T * PathLength;
        float AccumulatedDist = 0.0f;
        FVector Position = PathPoints[0];
        FVector Direction = FVector::ForwardVector;

        for (int32 j = 1; j < PathPoints.Num(); j++)
        {
            float SegmentLength = FVector::Dist(PathPoints[j - 1], PathPoints[j]);
            if (AccumulatedDist + SegmentLength >= TargetDist)
            {
                float SegmentT = (TargetDist - AccumulatedDist) / SegmentLength;
                Position = FMath::Lerp(PathPoints[j - 1], PathPoints[j], SegmentT);
                Direction = (PathPoints[j] - PathPoints[j - 1]).GetSafeNormal();
                break;
            }
            AccumulatedDist += SegmentLength;
        }

        // Offset perpendicular to path
        FVector Perpendicular(-Direction.Y, Direction.X, 0.0f);
        float Offset = RandomStream.FRandRange(-PathWidth * 0.5f, PathWidth * 0.5f);
        Position += Perpendicular * Offset;

        FForge2PropInstance Instance;
        FRotator Rotation = Params.bRandomRotation ? FRotator(0, RandomStream.FRandRange(0.0f, 360.0f), 0) : Direction.Rotation();
        float Scale = RandomStream.FRandRange(Params.ScaleRange.X, Params.ScaleRange.Y);

        Instance.Transform = FTransform(Rotation, Position, FVector(Scale));
        Instance.MeshPath = Params.MeshPaths[RandomStream.RandRange(0, Params.MeshPaths.Num() - 1)];
        Instance.bIsInteractable = false;
        Instance.bHasCollision = true;

        Instances.Add(Instance);
    }

    return Instances;
}

TArray<FForge2PropInstance> FForge2PropScatterer::ScatterAroundPoint(FVector Center, float Radius, const FForge2PropScatterParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    FForge2PropScatterParams ModifiedParams = Params;
    ModifiedParams.Bounds = FBox(
        Center - FVector(Radius, Radius, 0),
        Center + FVector(Radius, Radius, 0)
    );

    TArray<FForge2PropInstance> AllInstances = Scatter(ModifiedParams, HeightMap, MapWidth, MapHeight, CellSize);

    // Filter to only include props within radius
    TArray<FForge2PropInstance> FilteredInstances;
    for (const FForge2PropInstance& Instance : AllInstances)
    {
        if (FVector::Dist2D(Instance.Transform.GetLocation(), Center) <= Radius)
        {
            FilteredInstances.Add(Instance);
        }
    }

    return FilteredInstances;
}

bool FForge2PropScatterer::IsInExclusionZone(FVector Point, const TArray<FVector>& ExclusionPoints, float ExclusionRadius)
{
    for (const FVector& ExPoint : ExclusionPoints)
    {
        if (FVector::Dist2D(Point, ExPoint) < ExclusionRadius)
        {
            return true;
        }
    }
    return false;
}

TArray<FVector> FForge2PropScatterer::GenerateGridPlacement(FBox Bounds, float Spacing)
{
    TArray<FVector> Points;

    for (float Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y += Spacing)
    {
        for (float X = Bounds.Min.X; X <= Bounds.Max.X; X += Spacing)
        {
            Points.Add(FVector(X, Y, 0.0f));
        }
    }

    return Points;
}

TArray<FVector> FForge2PropScatterer::GenerateClusteredPlacement(FBox Bounds, int32 Count, int32 ClusterCount, float ClusterRadius, int32 Seed)
{
    TArray<FVector> Points;
    FRandomStream Random(Seed);

    // Generate cluster centers
    TArray<FVector> Centers;
    for (int32 i = 0; i < ClusterCount; i++)
    {
        FVector Center(
            Random.FRandRange(Bounds.Min.X + ClusterRadius, Bounds.Max.X - ClusterRadius),
            Random.FRandRange(Bounds.Min.Y + ClusterRadius, Bounds.Max.Y - ClusterRadius),
            0.0f
        );
        Centers.Add(Center);
    }

    // Generate points around clusters
    int32 PointsPerCluster = Count / ClusterCount;
    for (const FVector& Center : Centers)
    {
        for (int32 i = 0; i < PointsPerCluster; i++)
        {
            float Angle = Random.FRandRange(0.0f, 2.0f * PI);
            float Distance = Random.FRandRange(0.0f, ClusterRadius);

            FVector Point = Center + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
            Points.Add(Point);
        }
    }

    return Points;
}

// ============================================================================
// FForge2RockGenerator Implementation
// ============================================================================

FForge2RockGenerator::FForge2RockGenerator()
{
}

TArray<FForge2RockInstance> FForge2RockGenerator::Generate(const FForge2RockParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    TArray<FForge2RockInstance> Instances;

    if (Params.MeshPaths.Num() == 0)
        return Instances;

    RandomStream.Initialize(Params.Seed);

    for (int32 i = 0; i < Params.InstanceCount; i++)
    {
        FVector Point(
            RandomStream.FRandRange(Params.Bounds.Min.X, Params.Bounds.Max.X),
            RandomStream.FRandRange(Params.Bounds.Min.Y, Params.Bounds.Max.Y),
            0.0f
        );

        // Sample height
        int32 X = FMath::Clamp(FMath::FloorToInt(Point.X / CellSize), 0, MapWidth - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(Point.Y / CellSize), 0, MapHeight - 1);
        int32 Index = Y * MapWidth + X;
        float Height = (Index >= 0 && Index < HeightMap.Num()) ? HeightMap[Index] : 0.0f;

        // Check slope preference
        if (Params.bPreferSlopes && X > 0 && X < MapWidth - 1 && Y > 0 && Y < MapHeight - 1)
        {
            float Center = HeightMap[Index];
            float MaxDiff = 0.0f;
            static const FIntPoint Neighbors[] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
            for (const FIntPoint& N : Neighbors)
            {
                int32 NIndex = (Y + N.Y) * MapWidth + (X + N.X);
                MaxDiff = FMath::Max(MaxDiff, FMath::Abs(HeightMap[NIndex] - Center));
            }
            float Slope = FMath::RadiansToDegrees(FMath::Atan(MaxDiff / CellSize));
            if (Slope < Params.MinSlope)
            {
                continue; // Skip flat areas
            }
        }

        // Check if this should be a cluster
        if (RandomStream.FRand() < Params.ClusterChance)
        {
            TArray<FForge2RockInstance> ClusterRocks = GenerateFormation(Point, Params, HeightMap, MapWidth, MapHeight, CellSize);
            Instances.Append(ClusterRocks);
            i += ClusterRocks.Num() - 1; // Account for extra rocks
        }
        else
        {
            FForge2RockInstance Instance;

            float Scale = RandomStream.FRandRange(Params.ScaleRange.X, Params.ScaleRange.Y);
            float EmbedDepth = CalculateEmbedDepth(Point, HeightMap, MapWidth, MapHeight, CellSize, Params.EmbedDepthRange);

            Point.Z = Height - EmbedDepth;

            FRotator Rotation(
                RandomStream.FRandRange(-15.0f, 15.0f),
                RandomStream.FRandRange(0.0f, 360.0f),
                RandomStream.FRandRange(-15.0f, 15.0f)
            );

            Instance.Transform = FTransform(Rotation, Point, FVector(Scale));
            Instance.MeshPath = Params.MeshPaths[RandomStream.RandRange(0, Params.MeshPaths.Num() - 1)];
            Instance.EmbedDepth = EmbedDepth;
            Instance.MossAmount = CalculateMossAmount(Point, Params.MossChance, Params.Seed + i);
            Instance.bHasPhysics = false;

            Instances.Add(Instance);
        }
    }

    return Instances;
}

TArray<FForge2RockInstance> FForge2RockGenerator::GenerateFormation(FVector Center, const FForge2RockParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    TArray<FForge2RockInstance> Instances;

    int32 Count = RandomStream.RandRange(3, Params.ClusterSize);

    // Main central rock
    {
        FForge2RockInstance MainRock;

        int32 X = FMath::Clamp(FMath::FloorToInt(Center.X / CellSize), 0, MapWidth - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(Center.Y / CellSize), 0, MapHeight - 1);
        int32 Index = Y * MapWidth + X;
        float Height = (Index >= 0 && Index < HeightMap.Num()) ? HeightMap[Index] : 0.0f;

        float Scale = RandomStream.FRandRange(Params.ScaleRange.Y * 0.8f, Params.ScaleRange.Y);
        float EmbedDepth = Params.EmbedDepthRange * 0.5f;

        FVector Position = Center;
        Position.Z = Height - EmbedDepth;

        FRotator Rotation(0, RandomStream.FRandRange(0.0f, 360.0f), 0);

        MainRock.Transform = FTransform(Rotation, Position, FVector(Scale));
        MainRock.MeshPath = Params.MeshPaths[RandomStream.RandRange(0, Params.MeshPaths.Num() - 1)];
        MainRock.EmbedDepth = EmbedDepth;
        MainRock.MossAmount = CalculateMossAmount(Position, Params.MossChance, Params.Seed);
        MainRock.bHasPhysics = false;

        Instances.Add(MainRock);
    }

    // Surrounding rocks
    for (int32 i = 1; i < Count; i++)
    {
        float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
        float Distance = RandomStream.FRandRange(50.0f, Params.ClusterRadius);

        FVector Offset(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
        FVector Position = Center + Offset;

        int32 X = FMath::Clamp(FMath::FloorToInt(Position.X / CellSize), 0, MapWidth - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(Position.Y / CellSize), 0, MapHeight - 1);
        int32 Index = Y * MapWidth + X;
        float Height = (Index >= 0 && Index < HeightMap.Num()) ? HeightMap[Index] : 0.0f;

        FForge2RockInstance Instance;

        float Scale = RandomStream.FRandRange(Params.ScaleRange.X, Params.ScaleRange.Y * 0.7f);
        float EmbedDepth = RandomStream.FRandRange(0.0f, Params.EmbedDepthRange);

        Position.Z = Height - EmbedDepth;

        FRotator Rotation(
            RandomStream.FRandRange(-20.0f, 20.0f),
            RandomStream.FRandRange(0.0f, 360.0f),
            RandomStream.FRandRange(-20.0f, 20.0f)
        );

        Instance.Transform = FTransform(Rotation, Position, FVector(Scale));
        Instance.MeshPath = Params.MeshPaths[RandomStream.RandRange(0, Params.MeshPaths.Num() - 1)];
        Instance.EmbedDepth = EmbedDepth;
        Instance.MossAmount = CalculateMossAmount(Position, Params.MossChance, Params.Seed + i);
        Instance.bHasPhysics = false;

        Instances.Add(Instance);
    }

    return Instances;
}

float FForge2RockGenerator::CalculateEmbedDepth(FVector Location, const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize, float MaxDepth)
{
    // Rocks on slopes embed more
    int32 X = FMath::Clamp(FMath::FloorToInt(Location.X / CellSize), 1, Width - 2);
    int32 Y = FMath::Clamp(FMath::FloorToInt(Location.Y / CellSize), 1, Height - 2);
    int32 Index = Y * Width + X;

    float Center = HeightMap[Index];
    float MaxDiff = 0.0f;

    static const FIntPoint Neighbors[] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
    for (const FIntPoint& N : Neighbors)
    {
        int32 NIndex = (Y + N.Y) * Width + (X + N.X);
        MaxDiff = FMath::Max(MaxDiff, FMath::Abs(HeightMap[NIndex] - Center));
    }

    float Slope = MaxDiff / CellSize;
    return MaxDepth * FMath::Clamp(Slope, 0.0f, 1.0f);
}

float FForge2RockGenerator::CalculateMossAmount(FVector Location, float Chance, int32 Seed)
{
    FRandomStream Random(Seed);
    if (Random.FRand() > Chance)
        return 0.0f;

    return Random.FRandRange(0.1f, 1.0f);
}

// ============================================================================
// FForge2DecalPlacer Implementation
// ============================================================================

FForge2DecalPlacer::FForge2DecalPlacer()
{
}

TArray<FForge2DecalInstance> FForge2DecalPlacer::Place(const FForge2DecalParams& Params, const TArray<float>& HeightMap, int32 MapWidth, int32 MapHeight, float CellSize)
{
    TArray<FForge2DecalInstance> Instances;

    if (Params.MaterialPaths.Num() == 0)
        return Instances;

    RandomStream.Initialize(Params.Seed);

    for (int32 i = 0; i < Params.InstanceCount; i++)
    {
        FVector Point(
            RandomStream.FRandRange(Params.Bounds.Min.X, Params.Bounds.Max.X),
            RandomStream.FRandRange(Params.Bounds.Min.Y, Params.Bounds.Max.Y),
            0.0f
        );

        // Sample height
        int32 X = FMath::Clamp(FMath::FloorToInt(Point.X / CellSize), 0, MapWidth - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(Point.Y / CellSize), 0, MapHeight - 1);
        int32 Index = Y * MapWidth + X;
        float Height = (Index >= 0 && Index < HeightMap.Num()) ? HeightMap[Index] : 0.0f;

        Point.Z = Height + 1.0f; // Slightly above surface

        FForge2DecalInstance Instance;

        FRotator Rotation = FRotator(-90, 0, 0); // Face down
        if (Params.bRandomRotation)
        {
            Rotation.Roll = RandomStream.FRandRange(0.0f, 360.0f);
        }

        // Align to surface normal
        FVector Normal = GetSurfaceNormal(Point, HeightMap, MapWidth, MapHeight, CellSize);
        Rotation = Normal.Rotation();
        Rotation.Pitch -= 90.0f;

        float Size = RandomStream.FRandRange(Params.SizeRange.X, Params.SizeRange.Y);

        Instance.Transform = FTransform(Rotation, Point, FVector(1.0f));
        Instance.MaterialPath = Params.MaterialPaths[RandomStream.RandRange(0, Params.MaterialPaths.Num() - 1)];
        Instance.DecalType = Params.DecalType;
        Instance.Size = FVector2D(Size, Size);
        Instance.FadeDistance = 1000.0f;
        Instance.SortOrder = 0;

        Instances.Add(Instance);
    }

    return Instances;
}

TArray<FForge2DecalInstance> FForge2DecalPlacer::PlaceAlongPath(const TArray<FVector>& PathPoints, float PathWidth, const FForge2DecalParams& Params)
{
    TArray<FForge2DecalInstance> Instances;

    if (PathPoints.Num() < 2 || Params.MaterialPaths.Num() == 0)
        return Instances;

    RandomStream.Initialize(Params.Seed);

    float PathLength = 0.0f;
    for (int32 i = 1; i < PathPoints.Num(); i++)
    {
        PathLength += FVector::Dist(PathPoints[i - 1], PathPoints[i]);
    }

    int32 NumDecals = FMath::Max(1, FMath::FloorToInt(PathLength / Params.MinSpacing));

    for (int32 i = 0; i < NumDecals && Instances.Num() < Params.InstanceCount; i++)
    {
        float T = (float)i / (float)NumDecals + RandomStream.FRandRange(-0.1f, 0.1f);
        T = FMath::Clamp(T, 0.0f, 1.0f);

        float TargetDist = T * PathLength;
        float AccumulatedDist = 0.0f;
        FVector Position = PathPoints[0];
        FVector Direction = FVector::ForwardVector;

        for (int32 j = 1; j < PathPoints.Num(); j++)
        {
            float SegmentLength = FVector::Dist(PathPoints[j - 1], PathPoints[j]);
            if (AccumulatedDist + SegmentLength >= TargetDist)
            {
                float SegmentT = (TargetDist - AccumulatedDist) / SegmentLength;
                Position = FMath::Lerp(PathPoints[j - 1], PathPoints[j], SegmentT);
                Direction = (PathPoints[j] - PathPoints[j - 1]).GetSafeNormal();
                break;
            }
            AccumulatedDist += SegmentLength;
        }

        // Offset perpendicular to path
        FVector Perpendicular(-Direction.Y, Direction.X, 0.0f);
        float Offset = RandomStream.FRandRange(-PathWidth * 0.4f, PathWidth * 0.4f);
        Position += Perpendicular * Offset;

        FForge2DecalInstance Instance;

        FRotator Rotation = Direction.Rotation();
        Rotation.Pitch = -90.0f;
        if (Params.bRandomRotation)
        {
            Rotation.Roll = RandomStream.FRandRange(0.0f, 360.0f);
        }

        float Size = RandomStream.FRandRange(Params.SizeRange.X, Params.SizeRange.Y);

        Instance.Transform = FTransform(Rotation, Position, FVector(1.0f));
        Instance.MaterialPath = Params.MaterialPaths[RandomStream.RandRange(0, Params.MaterialPaths.Num() - 1)];
        Instance.DecalType = Params.DecalType;
        Instance.Size = FVector2D(Size, Size);
        Instance.FadeDistance = 1000.0f;
        Instance.SortOrder = 0;

        Instances.Add(Instance);
    }

    return Instances;
}

FVector FForge2DecalPlacer::GetSurfaceNormal(FVector Location, const TArray<float>& HeightMap, int32 Width, int32 Height, float CellSize)
{
    int32 X = FMath::Clamp(FMath::FloorToInt(Location.X / CellSize), 1, Width - 2);
    int32 Y = FMath::Clamp(FMath::FloorToInt(Location.Y / CellSize), 1, Height - 2);

    float HLeft = HeightMap[Y * Width + X - 1];
    float HRight = HeightMap[Y * Width + X + 1];
    float HDown = HeightMap[(Y - 1) * Width + X];
    float HUp = HeightMap[(Y + 1) * Width + X];

    FVector Normal(HLeft - HRight, HDown - HUp, 2.0f * CellSize);
    return Normal.GetSafeNormal();
}

// ============================================================================
// FForge2WaterEnvironmentUtils Implementation
// ============================================================================

FForge2EnvironmentData FForge2WaterEnvironmentUtils::CombineEnvironmentData(const TArray<FForge2EnvironmentData>& DataSets)
{
    FForge2EnvironmentData Combined;

    for (const FForge2EnvironmentData& Data : DataSets)
    {
        Combined.Vegetation.Append(Data.Vegetation);
        Combined.Props.Append(Data.Props);
        Combined.Rocks.Append(Data.Rocks);
        Combined.Decals.Append(Data.Decals);

        Combined.Bounds += Data.Bounds;
    }

    Combined.CalculateTotals();
    return Combined;
}

FForge2EnvironmentData FForge2WaterEnvironmentUtils::FilterByBounds(const FForge2EnvironmentData& Data, FBox Bounds)
{
    FForge2EnvironmentData Filtered;
    Filtered.Bounds = Bounds;

    for (const FForge2VegetationInstance& Instance : Data.Vegetation)
    {
        if (Bounds.IsInside(Instance.Transform.GetLocation()))
        {
            Filtered.Vegetation.Add(Instance);
        }
    }

    for (const FForge2PropInstance& Instance : Data.Props)
    {
        if (Bounds.IsInside(Instance.Transform.GetLocation()))
        {
            Filtered.Props.Add(Instance);
        }
    }

    for (const FForge2RockInstance& Instance : Data.Rocks)
    {
        if (Bounds.IsInside(Instance.Transform.GetLocation()))
        {
            Filtered.Rocks.Add(Instance);
        }
    }

    for (const FForge2DecalInstance& Instance : Data.Decals)
    {
        if (Bounds.IsInside(Instance.Transform.GetLocation()))
        {
            Filtered.Decals.Add(Instance);
        }
    }

    Filtered.CalculateTotals();
    return Filtered;
}

float FForge2WaterEnvironmentUtils::CalculateWaterSurfaceArea(const FForge2LakeData& Lake)
{
    return Lake.SurfaceArea;
}

float FForge2WaterEnvironmentUtils::CalculateWaterSurfaceArea(const FForge2RiverData& River)
{
    return River.TotalLength * River.AverageWidth;
}

bool FForge2WaterEnvironmentUtils::IsUnderwater(FVector Point, const TArray<FForge2LakeData>& Lakes, const TArray<FForge2RiverData>& Rivers, const FForge2OceanData* Ocean)
{
    // Check ocean
    if (Ocean)
    {
        if (Point.X >= Ocean->BoundsMin.X && Point.X <= Ocean->BoundsMax.X &&
            Point.Y >= Ocean->BoundsMin.Y && Point.Y <= Ocean->BoundsMax.Y &&
            Point.Z < Ocean->SeaLevel)
        {
            return true;
        }
    }

    // Check lakes
    for (const FForge2LakeData& Lake : Lakes)
    {
        if (Point.Z < Lake.WaterLevel)
        {
            // Check if inside lake polygon
            FVector2D Point2D(Point.X, Point.Y);
            bool bInside = false;
            int32 N = Lake.ShorelinePolygon.Num();
            for (int32 i = 0, j = N - 1; i < N; j = i++)
            {
                if (((Lake.ShorelinePolygon[i].Y > Point2D.Y) != (Lake.ShorelinePolygon[j].Y > Point2D.Y)) &&
                    (Point2D.X < (Lake.ShorelinePolygon[j].X - Lake.ShorelinePolygon[i].X) *
                    (Point2D.Y - Lake.ShorelinePolygon[i].Y) / (Lake.ShorelinePolygon[j].Y - Lake.ShorelinePolygon[i].Y) +
                    Lake.ShorelinePolygon[i].X))
                {
                    bInside = !bInside;
                }
            }
            if (bInside)
            {
                return true;
            }
        }
    }

    // Check rivers
    for (const FForge2RiverData& River : Rivers)
    {
        for (const FForge2RiverPoint& RiverPoint : River.SplinePoints)
        {
            float Dist2D = FVector2D::Distance(FVector2D(Point.X, Point.Y), FVector2D(RiverPoint.Location.X, RiverPoint.Location.Y));
            if (Dist2D < RiverPoint.Width * 0.5f && Point.Z < RiverPoint.Location.Z)
            {
                return true;
            }
        }
    }

    return false;
}

float FForge2WaterEnvironmentUtils::GetWaterDepth(FVector Point, const TArray<FForge2LakeData>& Lakes, const TArray<FForge2RiverData>& Rivers, const FForge2OceanData* Ocean)
{
    float MaxDepth = 0.0f;

    // Check ocean
    if (Ocean)
    {
        if (Point.X >= Ocean->BoundsMin.X && Point.X <= Ocean->BoundsMax.X &&
            Point.Y >= Ocean->BoundsMin.Y && Point.Y <= Ocean->BoundsMax.Y &&
            Point.Z < Ocean->SeaLevel)
        {
            MaxDepth = FMath::Max(MaxDepth, Ocean->SeaLevel - Point.Z);
        }
    }

    // Check lakes
    for (const FForge2LakeData& Lake : Lakes)
    {
        if (Point.Z < Lake.WaterLevel)
        {
            FVector2D Point2D(Point.X, Point.Y);
            bool bInside = false;
            int32 N = Lake.ShorelinePolygon.Num();
            for (int32 i = 0, j = N - 1; i < N; j = i++)
            {
                if (((Lake.ShorelinePolygon[i].Y > Point2D.Y) != (Lake.ShorelinePolygon[j].Y > Point2D.Y)) &&
                    (Point2D.X < (Lake.ShorelinePolygon[j].X - Lake.ShorelinePolygon[i].X) *
                    (Point2D.Y - Lake.ShorelinePolygon[i].Y) / (Lake.ShorelinePolygon[j].Y - Lake.ShorelinePolygon[i].Y) +
                    Lake.ShorelinePolygon[i].X))
                {
                    bInside = !bInside;
                }
            }
            if (bInside)
            {
                MaxDepth = FMath::Max(MaxDepth, Lake.WaterLevel - Point.Z);
            }
        }
    }

    // Check rivers
    for (const FForge2RiverData& River : Rivers)
    {
        for (const FForge2RiverPoint& RiverPoint : River.SplinePoints)
        {
            float Dist2D = FVector2D::Distance(FVector2D(Point.X, Point.Y), FVector2D(RiverPoint.Location.X, RiverPoint.Location.Y));
            if (Dist2D < RiverPoint.Width * 0.5f && Point.Z < RiverPoint.Location.Z)
            {
                MaxDepth = FMath::Max(MaxDepth, RiverPoint.Location.Z - Point.Z);
            }
        }
    }

    return MaxDepth;
}

FVector FForge2WaterEnvironmentUtils::FlowDirectionToVector(EForge2FlowDirection Direction)
{
    switch (Direction)
    {
    case EForge2FlowDirection::North: return FVector(0, 1, 0);
    case EForge2FlowDirection::NorthEast: return FVector(0.707f, 0.707f, 0);
    case EForge2FlowDirection::East: return FVector(1, 0, 0);
    case EForge2FlowDirection::SouthEast: return FVector(0.707f, -0.707f, 0);
    case EForge2FlowDirection::South: return FVector(0, -1, 0);
    case EForge2FlowDirection::SouthWest: return FVector(-0.707f, -0.707f, 0);
    case EForge2FlowDirection::West: return FVector(-1, 0, 0);
    case EForge2FlowDirection::NorthWest: return FVector(-0.707f, 0.707f, 0);
    default: return FVector::ForwardVector;
    }
}

float FForge2WaterEnvironmentUtils::GetDistanceToWater(FVector Point, const TArray<FForge2LakeData>& Lakes, const TArray<FForge2RiverData>& Rivers, const FForge2OceanData* Ocean)
{
    float MinDistance = MAX_FLT;

    // Check ocean coastline
    if (Ocean)
    {
        for (const FForge2ShorelinePoint& ShorePoint : Ocean->Coastline)
        {
            float Dist = FVector::Dist(Point, ShorePoint.Location);
            MinDistance = FMath::Min(MinDistance, Dist);
        }
    }

    // Check lake shorelines
    for (const FForge2LakeData& Lake : Lakes)
    {
        for (const FForge2ShorelinePoint& ShorePoint : Lake.Shoreline)
        {
            float Dist = FVector::Dist(Point, ShorePoint.Location);
            MinDistance = FMath::Min(MinDistance, Dist);
        }
    }

    // Check rivers
    for (const FForge2RiverData& River : Rivers)
    {
        for (const FForge2RiverPoint& RiverPoint : River.SplinePoints)
        {
            float Dist = FVector::Dist(Point, RiverPoint.Location) - RiverPoint.Width * 0.5f;
            MinDistance = FMath::Min(MinDistance, FMath::Max(0.0f, Dist));
        }
    }

    return MinDistance;
}
