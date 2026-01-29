// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Procedural Building Generation - Implementation

#include "Forge2WorldGenBuilding.h"

// ============================================================================
// Footprint Generator
// ============================================================================

TArray<FVector2D> FForge2FootprintGenerator::GenerateRectangle(float Width, float Depth)
{
    TArray<FVector2D> Footprint;
    Footprint.Add(FVector2D(0, 0));
    Footprint.Add(FVector2D(Width, 0));
    Footprint.Add(FVector2D(Width, Depth));
    Footprint.Add(FVector2D(0, Depth));
    return Footprint;
}

TArray<FVector2D> FForge2FootprintGenerator::GenerateLShape(float Width, float Depth, float CutWidth, float CutDepth, bool bFlipX, bool bFlipY)
{
    TArray<FVector2D> Footprint;

    float CutX = bFlipX ? 0 : Width - CutWidth;
    float CutY = bFlipY ? 0 : Depth - CutDepth;

    // Generate L-shape counter-clockwise
    Footprint.Add(FVector2D(0, 0));
    Footprint.Add(FVector2D(Width, 0));

    if (!bFlipY)
    {
        Footprint.Add(FVector2D(Width, CutY));
        Footprint.Add(FVector2D(CutX, CutY));
        Footprint.Add(FVector2D(CutX, Depth));
    }
    else
    {
        Footprint.Add(FVector2D(Width, CutDepth));
        Footprint.Add(FVector2D(CutWidth, CutDepth));
        Footprint.Add(FVector2D(CutWidth, Depth));
    }

    Footprint.Add(FVector2D(0, Depth));

    return Footprint;
}

TArray<FVector2D> FForge2FootprintGenerator::GenerateTShape(float Width, float Depth, float WingWidth, float WingDepth)
{
    TArray<FVector2D> Footprint;

    float CenterOffset = (Width - WingWidth) * 0.5f;

    Footprint.Add(FVector2D(CenterOffset, 0));
    Footprint.Add(FVector2D(CenterOffset + WingWidth, 0));
    Footprint.Add(FVector2D(CenterOffset + WingWidth, WingDepth));
    Footprint.Add(FVector2D(Width, WingDepth));
    Footprint.Add(FVector2D(Width, Depth));
    Footprint.Add(FVector2D(0, Depth));
    Footprint.Add(FVector2D(0, WingDepth));
    Footprint.Add(FVector2D(CenterOffset, WingDepth));

    return Footprint;
}

TArray<FVector2D> FForge2FootprintGenerator::GenerateUShape(float Width, float Depth, float CourtWidth, float CourtDepth)
{
    TArray<FVector2D> Footprint;

    float WingWidth = (Width - CourtWidth) * 0.5f;

    Footprint.Add(FVector2D(0, 0));
    Footprint.Add(FVector2D(Width, 0));
    Footprint.Add(FVector2D(Width, Depth));
    Footprint.Add(FVector2D(Width - WingWidth, Depth));
    Footprint.Add(FVector2D(Width - WingWidth, CourtDepth));
    Footprint.Add(FVector2D(WingWidth, CourtDepth));
    Footprint.Add(FVector2D(WingWidth, Depth));
    Footprint.Add(FVector2D(0, Depth));

    return Footprint;
}

TArray<FVector2D> FForge2FootprintGenerator::GenerateCross(float Size, float ArmWidth)
{
    TArray<FVector2D> Footprint;

    float Offset = (Size - ArmWidth) * 0.5f;

    // Top of vertical arm
    Footprint.Add(FVector2D(Offset, 0));
    Footprint.Add(FVector2D(Offset + ArmWidth, 0));
    // Right side
    Footprint.Add(FVector2D(Offset + ArmWidth, Offset));
    Footprint.Add(FVector2D(Size, Offset));
    Footprint.Add(FVector2D(Size, Offset + ArmWidth));
    Footprint.Add(FVector2D(Offset + ArmWidth, Offset + ArmWidth));
    // Bottom
    Footprint.Add(FVector2D(Offset + ArmWidth, Size));
    Footprint.Add(FVector2D(Offset, Size));
    // Left side
    Footprint.Add(FVector2D(Offset, Offset + ArmWidth));
    Footprint.Add(FVector2D(0, Offset + ArmWidth));
    Footprint.Add(FVector2D(0, Offset));
    Footprint.Add(FVector2D(Offset, Offset));

    return Footprint;
}

TArray<FVector2D> FForge2FootprintGenerator::GenerateCircle(float Radius, int32 Segments)
{
    TArray<FVector2D> Footprint;

    for (int32 i = 0; i < Segments; ++i)
    {
        float Angle = (static_cast<float>(i) / Segments) * 2.0f * PI;
        Footprint.Add(FVector2D(
            Radius + FMath::Cos(Angle) * Radius,
            Radius + FMath::Sin(Angle) * Radius
        ));
    }

    return Footprint;
}

TArray<FVector2D> FForge2FootprintGenerator::GenerateOctagon(float Size)
{
    TArray<FVector2D> Footprint;

    float Cut = Size * 0.293f; // 1 - 1/sqrt(2)

    Footprint.Add(FVector2D(Cut, 0));
    Footprint.Add(FVector2D(Size - Cut, 0));
    Footprint.Add(FVector2D(Size, Cut));
    Footprint.Add(FVector2D(Size, Size - Cut));
    Footprint.Add(FVector2D(Size - Cut, Size));
    Footprint.Add(FVector2D(Cut, Size));
    Footprint.Add(FVector2D(0, Size - Cut));
    Footprint.Add(FVector2D(0, Cut));

    return Footprint;
}

TArray<FVector2D> FForge2FootprintGenerator::GenerateRandom(FRandomStream& Random, float MinSize, float MaxSize, int32 MinVertices, int32 MaxVertices)
{
    int32 VertexCount = Random.RandRange(MinVertices, MaxVertices);
    float Width = Random.FRandRange(MinSize, MaxSize);
    float Depth = Random.FRandRange(MinSize, MaxSize);

    // Start with a rectangle and add variations
    TArray<FVector2D> Footprint;

    for (int32 i = 0; i < VertexCount; ++i)
    {
        float Angle = (static_cast<float>(i) / VertexCount) * 2.0f * PI;
        float Radius = (Random.FRandRange(0.7f, 1.0f)) * FMath::Min(Width, Depth) * 0.5f;

        Footprint.Add(FVector2D(
            Width * 0.5f + FMath::Cos(Angle) * Radius * (Width / FMath::Min(Width, Depth)),
            Depth * 0.5f + FMath::Sin(Angle) * Radius * (Depth / FMath::Min(Width, Depth))
        ));
    }

    return Footprint;
}

float FForge2FootprintGenerator::CalculateArea(const TArray<FVector2D>& Footprint)
{
    if (Footprint.Num() < 3) return 0.0f;

    float Area = 0.0f;
    int32 N = Footprint.Num();

    for (int32 i = 0; i < N; ++i)
    {
        int32 j = (i + 1) % N;
        Area += Footprint[i].X * Footprint[j].Y;
        Area -= Footprint[j].X * Footprint[i].Y;
    }

    return FMath::Abs(Area) * 0.5f;
}

FVector2D FForge2FootprintGenerator::CalculateCentroid(const TArray<FVector2D>& Footprint)
{
    if (Footprint.Num() == 0) return FVector2D::ZeroVector;

    FVector2D Centroid(0, 0);
    for (const FVector2D& V : Footprint)
    {
        Centroid += V;
    }
    return Centroid / Footprint.Num();
}

bool FForge2FootprintGenerator::IsConvex(const TArray<FVector2D>& Footprint)
{
    if (Footprint.Num() < 3) return false;

    int32 Sign = 0;
    int32 N = Footprint.Num();

    for (int32 i = 0; i < N; ++i)
    {
        FVector2D D1 = Footprint[(i + 1) % N] - Footprint[i];
        FVector2D D2 = Footprint[(i + 2) % N] - Footprint[(i + 1) % N];

        float Cross = D1.X * D2.Y - D1.Y * D2.X;
        int32 CurrentSign = (Cross > 0) ? 1 : ((Cross < 0) ? -1 : 0);

        if (CurrentSign != 0)
        {
            if (Sign == 0)
            {
                Sign = CurrentSign;
            }
            else if (Sign != CurrentSign)
            {
                return false;
            }
        }
    }

    return true;
}

TArray<FVector2D> FForge2FootprintGenerator::Simplify(const TArray<FVector2D>& Footprint, float Tolerance)
{
    if (Footprint.Num() <= 3) return Footprint;

    TArray<FVector2D> Simplified;
    float ToleranceSq = Tolerance * Tolerance;

    for (int32 i = 0; i < Footprint.Num(); ++i)
    {
        FVector2D Prev = Footprint[(i - 1 + Footprint.Num()) % Footprint.Num()];
        FVector2D Curr = Footprint[i];
        FVector2D Next = Footprint[(i + 1) % Footprint.Num()];

        // Check if point is on the line between prev and next
        FVector2D V1 = Curr - Prev;
        FVector2D V2 = Next - Prev;

        float T = FVector2D::DotProduct(V1, V2) / V2.SizeSquared();
        FVector2D Projected = Prev + V2 * FMath::Clamp(T, 0.0f, 1.0f);

        if ((Curr - Projected).SizeSquared() > ToleranceSq)
        {
            Simplified.Add(Curr);
        }
    }

    return Simplified.Num() >= 3 ? Simplified : Footprint;
}

// ============================================================================
// Building Generator
// ============================================================================

FForge2BuildingData FForge2BuildingGenerator::Generate(const FForge2BuildingParams& Params)
{
    return GenerateAt(Params, FVector::ZeroVector, FRotator::ZeroRotator);
}

FForge2BuildingData FForge2BuildingGenerator::GenerateAt(const FForge2BuildingParams& Params, const FVector& Location, const FRotator& Rotation)
{
    FForge2BuildingData Building;
    Building.Origin = Location;
    Building.Rotation = Rotation;
    Building.Style = Params.Style;
    Building.RoofType = Params.RoofType;
    Building.Seed = Params.Seed;

    FRandomStream Random(Params.Seed);

    // Apply style-specific modifiers
    FForge2BuildingParams ModifiedParams = Params;
    ApplyStyleModifiers(ModifiedParams, Params.Style);

    // Generate footprint
    Building.Footprint = GenerateFootprint(ModifiedParams, Random);

    // Generate structure (floors, walls)
    GenerateStructure(Building, ModifiedParams, Random);

    // Generate roof
    GenerateRoof(Building, ModifiedParams, Random);

    // Generate windows
    GenerateWindows(Building, ModifiedParams, Random);

    // Generate doors
    GenerateDoors(Building, ModifiedParams, Random);

    // Generate interior if requested
    if (ModifiedParams.bGenerateInterior)
    {
        GenerateInterior(Building, ModifiedParams, Random);
    }

    // Generate furniture if requested
    if (ModifiedParams.bGenerateFurniture)
    {
        GenerateFurniture(Building, ModifiedParams, Random);
    }

    // Calculate final bounds
    CalculateBounds(Building);

    return Building;
}

TArray<FVector2D> FForge2BuildingGenerator::GenerateFootprint(const FForge2BuildingParams& Params, FRandomStream& Random)
{
    if (Params.CustomFootprint.Num() >= 3)
    {
        return Params.CustomFootprint;
    }

    float Width = Random.FRandRange(Params.FootprintMin.X, Params.FootprintMax.X);
    float Depth = Random.FRandRange(Params.FootprintMin.Y, Params.FootprintMax.Y);

    // Choose footprint type based on style and randomness
    float Roll = Random.FRand();

    switch (Params.Style)
    {
        case EForge2BuildingStyle::Medieval:
        case EForge2BuildingStyle::Fantasy:
            if (Roll < 0.6f)
                return FForge2FootprintGenerator::GenerateRectangle(Width, Depth);
            else if (Roll < 0.8f)
                return FForge2FootprintGenerator::GenerateLShape(Width, Depth, Width * 0.4f, Depth * 0.4f, Random.GetFraction() > 0.5f, Random.GetFraction() > 0.5f);
            else
                return FForge2FootprintGenerator::GenerateTShape(Width, Depth, Width * 0.4f, Depth * 0.3f);

        case EForge2BuildingStyle::Modern:
        case EForge2BuildingStyle::Industrial:
            if (Roll < 0.7f)
                return FForge2FootprintGenerator::GenerateRectangle(Width, Depth);
            else
                return FForge2FootprintGenerator::GenerateLShape(Width, Depth, Width * 0.3f, Depth * 0.5f, false, false);

        case EForge2BuildingStyle::Gothic:
            return FForge2FootprintGenerator::GenerateCross(FMath::Max(Width, Depth), FMath::Min(Width, Depth) * 0.4f);

        case EForge2BuildingStyle::Arabic:
        case EForge2BuildingStyle::Ancient:
            if (Roll < 0.5f)
                return FForge2FootprintGenerator::GenerateRectangle(Width, Depth);
            else
                return FForge2FootprintGenerator::GenerateUShape(Width, Depth, Width * 0.4f, Depth * 0.5f);

        default:
            return FForge2FootprintGenerator::GenerateRectangle(Width, Depth);
    }
}

void FForge2BuildingGenerator::GenerateStructure(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    int32 FloorCount = Random.RandRange(Params.MinFloors, Params.MaxFloors);

    float CurrentHeight = 0.0f;

    for (int32 Floor = 0; Floor < FloorCount; ++Floor)
    {
        FForge2FloorData FloorData;
        FloorData.Height = CurrentHeight;
        FloorData.CeilingHeight = Params.FloorHeight;
        FloorData.Footprint = Building.Footprint;

        // Generate exterior walls for this floor
        GenerateExteriorWalls(Building, Floor, Building.Footprint, CurrentHeight, Params.FloorHeight, Params.WallThickness);

        Building.Floors.Add(FloorData);
        CurrentHeight += Params.FloorHeight;
    }
}

void FForge2BuildingGenerator::GenerateExteriorWalls(FForge2BuildingData& Building, int32 FloorIndex, const TArray<FVector2D>& Footprint, float FloorHeight, float CeilingHeight, float WallThickness)
{
    if (Footprint.Num() < 3) return;

    for (int32 i = 0; i < Footprint.Num(); ++i)
    {
        int32 Next = (i + 1) % Footprint.Num();

        FForge2WallSegment Wall;
        Wall.Start = FVector(Footprint[i].X, Footprint[i].Y, FloorHeight);
        Wall.End = FVector(Footprint[Next].X, Footprint[Next].Y, FloorHeight);
        Wall.Height = CeilingHeight;
        Wall.Thickness = WallThickness;
        Wall.bExterior = true;
        Wall.FloorIndex = FloorIndex;

        Building.Walls.Add(Wall);
    }
}

void FForge2BuildingGenerator::GenerateRoof(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    if (Building.Floors.Num() == 0) return;

    float RoofHeight = Building.Floors.Last().Height + Building.Floors.Last().CeilingHeight;

    switch (Params.RoofType)
    {
        case EForge2RoofType::Flat:
            FForge2RoofGenerator::GenerateFlatRoof(Building, Building.Footprint, RoofHeight);
            break;

        case EForge2RoofType::Gabled:
            FForge2RoofGenerator::GenerateGabledRoof(Building, Building.Footprint, RoofHeight);
            break;

        case EForge2RoofType::Hipped:
            FForge2RoofGenerator::GenerateHippedRoof(Building, Building.Footprint, RoofHeight);
            break;

        case EForge2RoofType::Dome:
            FForge2RoofGenerator::GenerateDomeRoof(Building, Building.Footprint, RoofHeight);
            break;

        case EForge2RoofType::Pyramid:
            FForge2RoofGenerator::GeneratePyramidRoof(Building, Building.Footprint, RoofHeight);
            break;

        case EForge2RoofType::Mansard:
            FForge2RoofGenerator::GenerateMansardRoof(Building, Building.Footprint, RoofHeight);
            break;

        case EForge2RoofType::Gambrel:
            FForge2RoofGenerator::GenerateGambrelRoof(Building, Building.Footprint, RoofHeight);
            break;

        default:
            FForge2RoofGenerator::GenerateFlatRoof(Building, Building.Footprint, RoofHeight);
            break;
    }
}

void FForge2BuildingGenerator::GenerateWindows(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    if (Params.WindowDensity <= 0.0f) return;

    float WindowWidth = 100.0f;
    float WindowHeight = 150.0f;
    float WindowSpacing = 150.0f;

    // Adjust based on style
    switch (Params.Style)
    {
        case EForge2BuildingStyle::Gothic:
            WindowWidth = 80.0f;
            WindowHeight = 200.0f;
            break;
        case EForge2BuildingStyle::Modern:
            WindowWidth = 150.0f;
            WindowHeight = 180.0f;
            break;
        case EForge2BuildingStyle::Japanese:
            WindowWidth = 120.0f;
            WindowHeight = 100.0f;
            break;
        default:
            break;
    }

    // Place windows on each wall segment
    for (const FForge2WallSegment& Wall : Building.Walls)
    {
        if (!Wall.bExterior) continue;

        FVector WallDir = (Wall.End - Wall.Start);
        float WallLength = WallDir.Size2D();
        WallDir.Normalize();

        FVector WallNormal(-WallDir.Y, WallDir.X, 0);

        // Calculate number of windows
        int32 MaxWindows = FMath::FloorToInt((WallLength - 100.0f) / (WindowWidth + WindowSpacing));
        int32 WindowCount = FMath::RoundToInt(MaxWindows * Params.WindowDensity);

        if (WindowCount <= 0) continue;

        float ActualSpacing = WallLength / (WindowCount + 1);
        float WindowZ = Wall.Start.Z + Wall.Height * 0.6f;

        for (int32 i = 0; i < WindowCount; ++i)
        {
            if (Random.FRand() > Params.WindowDensity) continue;

            float Offset = ActualSpacing * (i + 1);
            FVector WindowPos = Wall.Start + WallDir * Offset + FVector(0, 0, WindowZ);

            FForge2WindowData Window;
            Window.Position = WindowPos;
            Window.Rotation = FRotator(0, FMath::Atan2(WallNormal.Y, WallNormal.X) * (180.0f / PI), 0);
            Window.Size = FVector2D(WindowWidth, WindowHeight);
            Window.FloorIndex = Wall.FloorIndex;
            Window.WindowStyle = Random.RandRange(0, 3);

            Building.Windows.Add(Window);
        }
    }
}

void FForge2BuildingGenerator::GenerateDoors(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    if (Building.Walls.Num() == 0) return;

    int32 DoorCount = FMath::Max(1, FMath::RoundToInt(Params.DoorCount));

    // Place main entrance on longest ground floor wall
    float LongestWall = 0.0f;
    int32 LongestWallIndex = -1;

    for (int32 i = 0; i < Building.Walls.Num(); ++i)
    {
        const FForge2WallSegment& Wall = Building.Walls[i];
        if (Wall.FloorIndex != 0 || !Wall.bExterior) continue;

        float Length = (Wall.End - Wall.Start).Size2D();
        if (Length > LongestWall)
        {
            LongestWall = Length;
            LongestWallIndex = i;
        }
    }

    if (LongestWallIndex >= 0)
    {
        const FForge2WallSegment& Wall = Building.Walls[LongestWallIndex];
        FVector WallCenter = (Wall.Start + Wall.End) * 0.5f;
        FVector WallDir = (Wall.End - Wall.Start).GetSafeNormal();
        FVector WallNormal(-WallDir.Y, WallDir.X, 0);

        FForge2DoorData MainDoor;
        MainDoor.Position = FVector(WallCenter.X, WallCenter.Y, Wall.Start.Z);
        MainDoor.Rotation = FRotator(0, FMath::Atan2(WallNormal.Y, WallNormal.X) * (180.0f / PI), 0);
        MainDoor.Size = FVector2D(120.0f, 240.0f);
        MainDoor.bExterior = true;
        MainDoor.bDoubleDoor = (Params.Style == EForge2BuildingStyle::Gothic || Params.Style == EForge2BuildingStyle::Victorian);
        MainDoor.FloorIndex = 0;

        Building.Doors.Add(MainDoor);
    }

    // Add additional doors if needed
    for (int32 d = 1; d < DoorCount; ++d)
    {
        int32 WallIndex = Random.RandRange(0, Building.Walls.Num() - 1);
        const FForge2WallSegment& Wall = Building.Walls[WallIndex];

        if (Wall.FloorIndex != 0 || !Wall.bExterior) continue;

        FVector WallCenter = Wall.Start + (Wall.End - Wall.Start) * Random.FRandRange(0.3f, 0.7f);
        FVector WallDir = (Wall.End - Wall.Start).GetSafeNormal();
        FVector WallNormal(-WallDir.Y, WallDir.X, 0);

        FForge2DoorData Door;
        Door.Position = FVector(WallCenter.X, WallCenter.Y, Wall.Start.Z);
        Door.Rotation = FRotator(0, FMath::Atan2(WallNormal.Y, WallNormal.X) * (180.0f / PI), 0);
        Door.Size = FVector2D(100.0f, 220.0f);
        Door.bExterior = true;
        Door.FloorIndex = 0;

        Building.Doors.Add(Door);
    }
}

void FForge2BuildingGenerator::GenerateInterior(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    for (int32 FloorIndex = 0; FloorIndex < Building.Floors.Num(); ++FloorIndex)
    {
        SubdivideFloor(Building, FloorIndex, Building.Footprint, Building.Floors[FloorIndex].Height, Params, Random);
    }

    AssignRoomTypes(Building, Random);
    GenerateInteriorWalls(Building, Params);

    if (Building.Floors.Num() > 1)
    {
        GenerateStairs(Building, Params, Random);
    }
}

void FForge2BuildingGenerator::SubdivideFloor(FForge2BuildingData& Building, int32 FloorIndex, const TArray<FVector2D>& Footprint, float FloorHeight, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    // Calculate floor bounds from footprint
    FVector2D Min(MAX_FLT, MAX_FLT);
    FVector2D Max(-MAX_FLT, -MAX_FLT);

    for (const FVector2D& V : Footprint)
    {
        Min.X = FMath::Min(Min.X, V.X);
        Min.Y = FMath::Min(Min.Y, V.Y);
        Max.X = FMath::Max(Max.X, V.X);
        Max.Y = FMath::Max(Max.Y, V.Y);
    }

    FBox FloorBounds(
        FVector(Min.X, Min.Y, FloorHeight),
        FVector(Max.X, Max.Y, FloorHeight + Params.FloorHeight)
    );

    int32 TargetRooms = Random.RandRange(Params.MinRoomsPerFloor, Params.MaxRoomsPerFloor);

    TArray<FBox> RoomBounds = FForge2InteriorGenerator::GenerateRoomLayout(FloorBounds, Params.MinRoomsPerFloor, Params.MaxRoomsPerFloor, Random);

    for (const FBox& Bounds : RoomBounds)
    {
        FForge2RoomData Room;
        Room.Bounds = Bounds;
        Room.FloorIndex = FloorIndex;
        Room.RoomType = EForge2RoomType::LivingRoom; // Will be assigned later

        Building.Rooms.Add(Room);
        Building.Floors[FloorIndex].RoomIndices.Add(Building.Rooms.Num() - 1);
    }
}

void FForge2BuildingGenerator::AssignRoomTypes(FForge2BuildingData& Building, FRandomStream& Random)
{
    FForge2InteriorGenerator::AssignRoomTypes(Building.Rooms, FForge2BuildingParams(), Random);
}

void FForge2BuildingGenerator::GenerateInteriorWalls(FForge2BuildingData& Building, const FForge2BuildingParams& Params)
{
    // Add interior walls between adjacent rooms
    for (int32 i = 0; i < Building.Rooms.Num(); ++i)
    {
        const FForge2RoomData& Room = Building.Rooms[i];

        // Create walls along room boundaries
        FVector RoomMin = Room.Bounds.Min;
        FVector RoomMax = Room.Bounds.Max;

        // North wall
        FForge2WallSegment NorthWall;
        NorthWall.Start = FVector(RoomMin.X, RoomMax.Y, RoomMin.Z);
        NorthWall.End = FVector(RoomMax.X, RoomMax.Y, RoomMin.Z);
        NorthWall.Height = RoomMax.Z - RoomMin.Z;
        NorthWall.Thickness = Params.WallThickness * 0.5f;
        NorthWall.bExterior = false;
        NorthWall.FloorIndex = Room.FloorIndex;
        Building.Walls.Add(NorthWall);

        // South wall
        FForge2WallSegment SouthWall;
        SouthWall.Start = FVector(RoomMin.X, RoomMin.Y, RoomMin.Z);
        SouthWall.End = FVector(RoomMax.X, RoomMin.Y, RoomMin.Z);
        SouthWall.Height = RoomMax.Z - RoomMin.Z;
        SouthWall.Thickness = Params.WallThickness * 0.5f;
        SouthWall.bExterior = false;
        SouthWall.FloorIndex = Room.FloorIndex;
        Building.Walls.Add(SouthWall);
    }
}

void FForge2BuildingGenerator::GenerateStairs(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    // Find suitable rooms for stairs (preferably hallways)
    for (int32 FloorIndex = 0; FloorIndex < Building.Floors.Num() - 1; ++FloorIndex)
    {
        const TArray<int32>& RoomIndices = Building.Floors[FloorIndex].RoomIndices;
        if (RoomIndices.Num() == 0) continue;

        // Find a hallway or large room
        int32 StairRoomIndex = -1;
        for (int32 RoomIdx : RoomIndices)
        {
            if (Building.Rooms[RoomIdx].RoomType == EForge2RoomType::Hallway)
            {
                StairRoomIndex = RoomIdx;
                break;
            }
        }

        if (StairRoomIndex < 0)
        {
            StairRoomIndex = RoomIndices[0];
        }

        // Mark room as containing stairs
        Building.Rooms[StairRoomIndex].RoomType = EForge2RoomType::Hallway;
    }
}

void FForge2BuildingGenerator::GenerateFurniture(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    for (int32 i = 0; i < Building.Rooms.Num(); ++i)
    {
        const FForge2RoomData& Room = Building.Rooms[i];
        TArray<FName> FurnitureTypes = FForge2InteriorGenerator::GetFurnitureForRoomType(Room.RoomType);
        TArray<FForge2FurnitureData> RoomFurniture = FForge2InteriorGenerator::PlaceFurniture(Room, FurnitureTypes, Params.FurnitureDensity, Random);

        for (FForge2FurnitureData& Furniture : RoomFurniture)
        {
            Furniture.RoomIndex = i;
            Building.Furniture.Add(Furniture);
        }
    }
}

void FForge2BuildingGenerator::ApplyStyleModifiers(FForge2BuildingParams& Params, EForge2BuildingStyle Style)
{
    switch (Style)
    {
        case EForge2BuildingStyle::Medieval:
            Params.RoofType = (Params.RoofType == EForge2RoofType::Flat) ? EForge2RoofType::Gabled : Params.RoofType;
            Params.bIncludeChimneys = true;
            break;

        case EForge2BuildingStyle::Gothic:
            Params.WindowDensity = FMath::Min(Params.WindowDensity, 0.3f);
            Params.FloorHeight *= 1.5f;
            break;

        case EForge2BuildingStyle::Modern:
            Params.RoofType = EForge2RoofType::Flat;
            Params.WindowDensity = FMath::Max(Params.WindowDensity, 0.7f);
            Params.bIncludeChimneys = false;
            break;

        case EForge2BuildingStyle::Japanese:
            Params.RoofType = EForge2RoofType::Hipped;
            Params.FloorHeight *= 0.9f;
            break;

        case EForge2BuildingStyle::Victorian:
            Params.bIncludeBalconies = true;
            Params.RoofType = EForge2RoofType::Mansard;
            break;

        default:
            break;
    }
}

void FForge2BuildingGenerator::CalculateBounds(FForge2BuildingData& Building)
{
    FVector2D Min(MAX_FLT, MAX_FLT);
    FVector2D Max(-MAX_FLT, -MAX_FLT);

    for (const FVector2D& V : Building.Footprint)
    {
        Min.X = FMath::Min(Min.X, V.X);
        Min.Y = FMath::Min(Min.Y, V.Y);
        Max.X = FMath::Max(Max.X, V.X);
        Max.Y = FMath::Max(Max.Y, V.Y);
    }

    Building.Width = Max.X - Min.X;
    Building.Depth = Max.Y - Min.Y;

    // Calculate total height including roof
    float WallHeight = 0.0f;
    for (const FForge2FloorData& Floor : Building.Floors)
    {
        WallHeight = Floor.Height + Floor.CeilingHeight;
    }

    float RoofHeight = 0.0f;
    for (const FVector& V : Building.RoofVertices)
    {
        RoofHeight = FMath::Max(RoofHeight, V.Z);
    }

    Building.TotalHeight = FMath::Max(WallHeight, RoofHeight);
}

// ============================================================================
// Roof Generator
// ============================================================================

void FForge2RoofGenerator::GetFootprintBounds(const TArray<FVector2D>& Footprint, FVector2D& OutCenter, FVector2D& OutMin, FVector2D& OutMax)
{
    OutMin = FVector2D(MAX_FLT, MAX_FLT);
    OutMax = FVector2D(-MAX_FLT, -MAX_FLT);

    for (const FVector2D& V : Footprint)
    {
        OutMin.X = FMath::Min(OutMin.X, V.X);
        OutMin.Y = FMath::Min(OutMin.Y, V.Y);
        OutMax.X = FMath::Max(OutMax.X, V.X);
        OutMax.Y = FMath::Max(OutMax.Y, V.Y);
    }

    OutCenter = (OutMin + OutMax) * 0.5f;
}

void FForge2RoofGenerator::AddTriangle(FForge2BuildingData& Building, const FVector& V0, const FVector& V1, const FVector& V2)
{
    int32 BaseIndex = Building.RoofVertices.Num();
    Building.RoofVertices.Add(V0);
    Building.RoofVertices.Add(V1);
    Building.RoofVertices.Add(V2);
    Building.RoofTriangles.Add(BaseIndex);
    Building.RoofTriangles.Add(BaseIndex + 1);
    Building.RoofTriangles.Add(BaseIndex + 2);
}

void FForge2RoofGenerator::AddQuad(FForge2BuildingData& Building, const FVector& V0, const FVector& V1, const FVector& V2, const FVector& V3)
{
    AddTriangle(Building, V0, V1, V2);
    AddTriangle(Building, V0, V2, V3);
}

void FForge2RoofGenerator::GenerateFlatRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float Overhang)
{
    FVector2D Center, Min, Max;
    GetFootprintBounds(Footprint, Center, Min, Max);

    // Simple flat roof with slight overhang
    FVector V0(Min.X - Overhang, Min.Y - Overhang, Height);
    FVector V1(Max.X + Overhang, Min.Y - Overhang, Height);
    FVector V2(Max.X + Overhang, Max.Y + Overhang, Height);
    FVector V3(Min.X - Overhang, Max.Y + Overhang, Height);

    AddQuad(Building, V0, V1, V2, V3);
}

void FForge2RoofGenerator::GenerateGabledRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float Pitch, float Overhang)
{
    FVector2D Center, Min, Max;
    GetFootprintBounds(Footprint, Center, Min, Max);

    float Width = Max.X - Min.X;
    float Depth = Max.Y - Min.Y;
    float RidgeHeight = (Width * 0.5f) * FMath::Tan(FMath::DegreesToRadians(Pitch));

    // Ridge line runs along the longer axis
    bool bRidgeAlongX = Width < Depth;

    if (bRidgeAlongX)
    {
        FVector Ridge0(Min.X - Overhang, Center.Y, Height + RidgeHeight);
        FVector Ridge1(Max.X + Overhang, Center.Y, Height + RidgeHeight);

        FVector Eave0(Min.X - Overhang, Min.Y - Overhang, Height);
        FVector Eave1(Max.X + Overhang, Min.Y - Overhang, Height);
        FVector Eave2(Max.X + Overhang, Max.Y + Overhang, Height);
        FVector Eave3(Min.X - Overhang, Max.Y + Overhang, Height);

        // Front slope
        AddQuad(Building, Eave0, Eave1, Ridge1, Ridge0);
        // Back slope
        AddQuad(Building, Eave2, Eave3, Ridge0, Ridge1);
        // Gable ends
        AddTriangle(Building, Eave0, Ridge0, Eave3);
        AddTriangle(Building, Eave1, Eave2, Ridge1);
    }
    else
    {
        FVector Ridge0(Center.X, Min.Y - Overhang, Height + RidgeHeight);
        FVector Ridge1(Center.X, Max.Y + Overhang, Height + RidgeHeight);

        FVector Eave0(Min.X - Overhang, Min.Y - Overhang, Height);
        FVector Eave1(Max.X + Overhang, Min.Y - Overhang, Height);
        FVector Eave2(Max.X + Overhang, Max.Y + Overhang, Height);
        FVector Eave3(Min.X - Overhang, Max.Y + Overhang, Height);

        // Left slope
        AddQuad(Building, Eave0, Ridge0, Ridge1, Eave3);
        // Right slope
        AddQuad(Building, Eave1, Eave2, Ridge1, Ridge0);
        // Gable ends
        AddTriangle(Building, Eave0, Eave1, Ridge0);
        AddTriangle(Building, Eave2, Eave3, Ridge1);
    }
}

void FForge2RoofGenerator::GenerateHippedRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float Pitch, float Overhang)
{
    FVector2D Center, Min, Max;
    GetFootprintBounds(Footprint, Center, Min, Max);

    float Width = Max.X - Min.X;
    float Depth = Max.Y - Min.Y;
    float SmallDim = FMath::Min(Width, Depth);
    float RidgeHeight = (SmallDim * 0.5f) * FMath::Tan(FMath::DegreesToRadians(Pitch));

    // Ridge offset from edges
    float Inset = SmallDim * 0.5f;

    FVector Eave0(Min.X - Overhang, Min.Y - Overhang, Height);
    FVector Eave1(Max.X + Overhang, Min.Y - Overhang, Height);
    FVector Eave2(Max.X + Overhang, Max.Y + Overhang, Height);
    FVector Eave3(Min.X - Overhang, Max.Y + Overhang, Height);

    if (Width > Depth)
    {
        FVector Ridge0(Min.X + Inset, Center.Y, Height + RidgeHeight);
        FVector Ridge1(Max.X - Inset, Center.Y, Height + RidgeHeight);

        // Front face
        AddQuad(Building, Eave0, Eave1, Ridge1, Ridge0);
        // Back face
        AddQuad(Building, Eave2, Eave3, Ridge0, Ridge1);
        // Left hip
        AddTriangle(Building, Eave0, Ridge0, Eave3);
        // Right hip
        AddTriangle(Building, Eave1, Eave2, Ridge1);
    }
    else
    {
        FVector Ridge0(Center.X, Min.Y + Inset, Height + RidgeHeight);
        FVector Ridge1(Center.X, Max.Y - Inset, Height + RidgeHeight);

        // Front hip
        AddTriangle(Building, Eave0, Eave1, Ridge0);
        // Back hip
        AddTriangle(Building, Eave2, Eave3, Ridge1);
        // Left face
        AddQuad(Building, Eave0, Ridge0, Ridge1, Eave3);
        // Right face
        AddQuad(Building, Eave1, Eave2, Ridge1, Ridge0);
    }
}

void FForge2RoofGenerator::GenerateDomeRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, int32 Segments)
{
    FVector2D Center, Min, Max;
    GetFootprintBounds(Footprint, Center, Min, Max);

    float RadiusX = (Max.X - Min.X) * 0.5f;
    float RadiusY = (Max.Y - Min.Y) * 0.5f;
    float DomeHeight = FMath::Min(RadiusX, RadiusY);

    int32 RingCount = Segments / 2;

    for (int32 Ring = 0; Ring < RingCount; ++Ring)
    {
        float Phi0 = (static_cast<float>(Ring) / RingCount) * (PI * 0.5f);
        float Phi1 = (static_cast<float>(Ring + 1) / RingCount) * (PI * 0.5f);

        float Z0 = Height + FMath::Sin(Phi0) * DomeHeight;
        float Z1 = Height + FMath::Sin(Phi1) * DomeHeight;
        float R0 = FMath::Cos(Phi0);
        float R1 = FMath::Cos(Phi1);

        for (int32 Seg = 0; Seg < Segments; ++Seg)
        {
            float Theta0 = (static_cast<float>(Seg) / Segments) * 2.0f * PI;
            float Theta1 = (static_cast<float>(Seg + 1) / Segments) * 2.0f * PI;

            FVector V00(Center.X + FMath::Cos(Theta0) * RadiusX * R0, Center.Y + FMath::Sin(Theta0) * RadiusY * R0, Z0);
            FVector V10(Center.X + FMath::Cos(Theta1) * RadiusX * R0, Center.Y + FMath::Sin(Theta1) * RadiusY * R0, Z0);
            FVector V01(Center.X + FMath::Cos(Theta0) * RadiusX * R1, Center.Y + FMath::Sin(Theta0) * RadiusY * R1, Z1);
            FVector V11(Center.X + FMath::Cos(Theta1) * RadiusX * R1, Center.Y + FMath::Sin(Theta1) * RadiusY * R1, Z1);

            AddQuad(Building, V00, V10, V11, V01);
        }
    }
}

void FForge2RoofGenerator::GeneratePyramidRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float Overhang)
{
    FVector2D Center, Min, Max;
    GetFootprintBounds(Footprint, Center, Min, Max);

    float Width = Max.X - Min.X;
    float Depth = Max.Y - Min.Y;
    float PyramidHeight = FMath::Min(Width, Depth) * 0.5f;

    FVector Apex(Center.X, Center.Y, Height + PyramidHeight);

    FVector Eave0(Min.X - Overhang, Min.Y - Overhang, Height);
    FVector Eave1(Max.X + Overhang, Min.Y - Overhang, Height);
    FVector Eave2(Max.X + Overhang, Max.Y + Overhang, Height);
    FVector Eave3(Min.X - Overhang, Max.Y + Overhang, Height);

    AddTriangle(Building, Eave0, Eave1, Apex);
    AddTriangle(Building, Eave1, Eave2, Apex);
    AddTriangle(Building, Eave2, Eave3, Apex);
    AddTriangle(Building, Eave3, Eave0, Apex);
}

void FForge2RoofGenerator::GenerateMansardRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float LowerPitch, float UpperPitch)
{
    FVector2D Center, Min, Max;
    GetFootprintBounds(Footprint, Center, Min, Max);

    float Width = Max.X - Min.X;
    float Depth = Max.Y - Min.Y;

    float Inset = FMath::Min(Width, Depth) * 0.2f;
    float LowerHeight = Inset * FMath::Tan(FMath::DegreesToRadians(LowerPitch));
    float UpperHeight = Inset * FMath::Tan(FMath::DegreesToRadians(UpperPitch));

    // Eave level
    FVector E0(Min.X, Min.Y, Height);
    FVector E1(Max.X, Min.Y, Height);
    FVector E2(Max.X, Max.Y, Height);
    FVector E3(Min.X, Max.Y, Height);

    // Mid level (after lower mansard slope)
    FVector M0(Min.X + Inset, Min.Y + Inset, Height + LowerHeight);
    FVector M1(Max.X - Inset, Min.Y + Inset, Height + LowerHeight);
    FVector M2(Max.X - Inset, Max.Y - Inset, Height + LowerHeight);
    FVector M3(Min.X + Inset, Max.Y - Inset, Height + LowerHeight);

    // Lower steep slopes
    AddQuad(Building, E0, E1, M1, M0);
    AddQuad(Building, E1, E2, M2, M1);
    AddQuad(Building, E2, E3, M3, M2);
    AddQuad(Building, E3, E0, M0, M3);

    // Upper flat or gentle slope - just use gabled for simplicity
    GenerateGabledRoof(Building, TArray<FVector2D>({FVector2D(M0.X, M0.Y), FVector2D(M1.X, M1.Y), FVector2D(M2.X, M2.Y), FVector2D(M3.X, M3.Y)}), Height + LowerHeight, UpperPitch, 0);
}

void FForge2RoofGenerator::GenerateGambrelRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float LowerPitch, float UpperPitch)
{
    // Similar to mansard but with gables
    GenerateMansardRoof(Building, Footprint, Height, LowerPitch, UpperPitch);
}

// ============================================================================
// Interior Generator
// ============================================================================

TArray<FBox> FForge2InteriorGenerator::GenerateRoomLayout(const FBox& Bounds, int32 MinRooms, int32 MaxRooms, FRandomStream& Random)
{
    TArray<FBox> Rooms;

    TSharedPtr<FBSPNode> Root = MakeShared<FBSPNode>();
    Root->Bounds = Bounds;

    // Calculate depth based on target room count
    int32 TargetRooms = Random.RandRange(MinRooms, MaxRooms);
    int32 MaxDepth = FMath::CeilToInt(FMath::Log2(static_cast<float>(TargetRooms)));

    float MinRoomSize = 150.0f; // Minimum room dimension

    Subdivide(Root, 0, MaxDepth, MinRoomSize, Random);
    CollectRooms(Root, Rooms);

    return Rooms;
}

void FForge2InteriorGenerator::Subdivide(TSharedPtr<FBSPNode> Node, int32 Depth, int32 MaxDepth, float MinRoomSize, FRandomStream& Random)
{
    if (!Node.IsValid() || Depth >= MaxDepth) return;

    FVector Size = Node->Bounds.GetSize();

    // Check if we can split
    bool bCanSplitX = Size.X > MinRoomSize * 2.5f;
    bool bCanSplitY = Size.Y > MinRoomSize * 2.5f;

    if (!bCanSplitX && !bCanSplitY) return;

    // Choose split direction
    bool bSplitX = bCanSplitX && (!bCanSplitY || Random.FRand() > 0.5f);

    // Calculate split position (30-70% range)
    float SplitPos;
    if (bSplitX)
    {
        float SplitMin = Node->Bounds.Min.X + MinRoomSize;
        float SplitMax = Node->Bounds.Max.X - MinRoomSize;
        SplitPos = Random.FRandRange(SplitMin, SplitMax);

        Node->Left = MakeShared<FBSPNode>();
        Node->Left->Bounds = FBox(Node->Bounds.Min, FVector(SplitPos, Node->Bounds.Max.Y, Node->Bounds.Max.Z));

        Node->Right = MakeShared<FBSPNode>();
        Node->Right->Bounds = FBox(FVector(SplitPos, Node->Bounds.Min.Y, Node->Bounds.Min.Z), Node->Bounds.Max);
    }
    else
    {
        float SplitMin = Node->Bounds.Min.Y + MinRoomSize;
        float SplitMax = Node->Bounds.Max.Y - MinRoomSize;
        SplitPos = Random.FRandRange(SplitMin, SplitMax);

        Node->Left = MakeShared<FBSPNode>();
        Node->Left->Bounds = FBox(Node->Bounds.Min, FVector(Node->Bounds.Max.X, SplitPos, Node->Bounds.Max.Z));

        Node->Right = MakeShared<FBSPNode>();
        Node->Right->Bounds = FBox(FVector(Node->Bounds.Min.X, SplitPos, Node->Bounds.Min.Z), Node->Bounds.Max);
    }

    Subdivide(Node->Left, Depth + 1, MaxDepth, MinRoomSize, Random);
    Subdivide(Node->Right, Depth + 1, MaxDepth, MinRoomSize, Random);
}

void FForge2InteriorGenerator::CollectRooms(TSharedPtr<FBSPNode> Node, TArray<FBox>& OutRooms)
{
    if (!Node.IsValid()) return;

    if (Node->IsLeaf())
    {
        OutRooms.Add(Node->Bounds);
    }
    else
    {
        CollectRooms(Node->Left, OutRooms);
        CollectRooms(Node->Right, OutRooms);
    }
}

void FForge2InteriorGenerator::AssignRoomTypes(TArray<FForge2RoomData>& Rooms, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    TArray<EForge2RoomType> AvailableTypes = {
        EForge2RoomType::LivingRoom,
        EForge2RoomType::Bedroom,
        EForge2RoomType::Kitchen,
        EForge2RoomType::DiningRoom,
        EForge2RoomType::Bathroom,
        EForge2RoomType::Storage,
        EForge2RoomType::Office
    };

    for (FForge2RoomData& Room : Rooms)
    {
        // Assign based on room size
        FVector Size = Room.Bounds.GetSize();
        float Area = Size.X * Size.Y;

        if (Area < 10000.0f) // Small room
        {
            TArray<EForge2RoomType> SmallRooms = {EForge2RoomType::Bathroom, EForge2RoomType::Storage};
            Room.RoomType = SmallRooms[Random.RandRange(0, SmallRooms.Num() - 1)];
        }
        else if (Area > 40000.0f) // Large room
        {
            TArray<EForge2RoomType> LargeRooms = {EForge2RoomType::LivingRoom, EForge2RoomType::ThroneRoom, EForge2RoomType::Library};
            Room.RoomType = LargeRooms[Random.RandRange(0, LargeRooms.Num() - 1)];
        }
        else
        {
            Room.RoomType = AvailableTypes[Random.RandRange(0, AvailableTypes.Num() - 1)];
        }
    }
}

TArray<FName> FForge2InteriorGenerator::GetFurnitureForRoomType(EForge2RoomType RoomType)
{
    TArray<FName> Furniture;

    switch (RoomType)
    {
        case EForge2RoomType::Bedroom:
            Furniture.Add(FName("Bed"));
            Furniture.Add(FName("Wardrobe"));
            Furniture.Add(FName("Nightstand"));
            Furniture.Add(FName("Dresser"));
            break;

        case EForge2RoomType::Kitchen:
            Furniture.Add(FName("Stove"));
            Furniture.Add(FName("Table"));
            Furniture.Add(FName("Cabinet"));
            Furniture.Add(FName("Sink"));
            break;

        case EForge2RoomType::LivingRoom:
            Furniture.Add(FName("Sofa"));
            Furniture.Add(FName("Armchair"));
            Furniture.Add(FName("Table"));
            Furniture.Add(FName("Bookshelf"));
            Furniture.Add(FName("Fireplace"));
            break;

        case EForge2RoomType::DiningRoom:
            Furniture.Add(FName("DiningTable"));
            Furniture.Add(FName("Chair"));
            Furniture.Add(FName("Cabinet"));
            Furniture.Add(FName("Chandelier"));
            break;

        case EForge2RoomType::Bathroom:
            Furniture.Add(FName("Toilet"));
            Furniture.Add(FName("Sink"));
            Furniture.Add(FName("Bathtub"));
            Furniture.Add(FName("Mirror"));
            break;

        case EForge2RoomType::Office:
            Furniture.Add(FName("Desk"));
            Furniture.Add(FName("Chair"));
            Furniture.Add(FName("Bookshelf"));
            Furniture.Add(FName("FileCabinet"));
            break;

        case EForge2RoomType::Library:
            Furniture.Add(FName("Bookshelf"));
            Furniture.Add(FName("ReadingChair"));
            Furniture.Add(FName("Desk"));
            Furniture.Add(FName("Ladder"));
            break;

        case EForge2RoomType::ThroneRoom:
            Furniture.Add(FName("Throne"));
            Furniture.Add(FName("Carpet"));
            Furniture.Add(FName("Banner"));
            Furniture.Add(FName("Brazier"));
            break;

        default:
            Furniture.Add(FName("Chair"));
            Furniture.Add(FName("Table"));
            break;
    }

    return Furniture;
}

TArray<FForge2FurnitureData> FForge2InteriorGenerator::PlaceFurniture(const FForge2RoomData& Room, const TArray<FName>& FurnitureTypes, float Density, FRandomStream& Random)
{
    TArray<FForge2FurnitureData> Furniture;

    FVector RoomSize = Room.Bounds.GetSize();
    FVector RoomCenter = Room.Bounds.GetCenter();
    float Area = RoomSize.X * RoomSize.Y;

    int32 FurnitureCount = FMath::RoundToInt((Area / 10000.0f) * Density * FurnitureTypes.Num());

    for (int32 i = 0; i < FurnitureCount && i < FurnitureTypes.Num(); ++i)
    {
        FForge2FurnitureData Item;
        Item.FurnitureType = FurnitureTypes[i % FurnitureTypes.Num()];

        // Random position within room bounds (with margin)
        float Margin = 50.0f;
        Item.Position = FVector(
            Random.FRandRange(Room.Bounds.Min.X + Margin, Room.Bounds.Max.X - Margin),
            Random.FRandRange(Room.Bounds.Min.Y + Margin, Room.Bounds.Max.Y - Margin),
            Room.Bounds.Min.Z
        );

        Item.Rotation = FRotator(0, Random.FRandRange(0.0f, 360.0f), 0);
        Item.Scale = FVector(Random.FRandRange(0.9f, 1.1f));

        Furniture.Add(Item);
    }

    return Furniture;
}

// ============================================================================
// Building Decorator
// ============================================================================

void FForge2BuildingDecorator::AddChimneys(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    if (!Params.bIncludeChimneys || Building.Floors.Num() == 0) return;

    // Add furniture entry for chimney
    FForge2FurnitureData Chimney;
    Chimney.FurnitureType = FName("Chimney");

    // Place on roof near a random position
    FVector2D Center, Min, Max;
    FForge2RoofGenerator::GetFootprintBounds(Building.Footprint, Center, Min, Max);

    Chimney.Position = FVector(
        Random.FRandRange(Min.X + 100, Max.X - 100),
        Random.FRandRange(Min.Y + 100, Max.Y - 100),
        Building.TotalHeight
    );

    Chimney.Rotation = FRotator::ZeroRotator;
    Chimney.Scale = FVector(1.0f);
    Chimney.RoomIndex = -1;

    Building.Furniture.Add(Chimney);
}

void FForge2BuildingDecorator::AddBalconies(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random)
{
    if (!Params.bIncludeBalconies || Building.Floors.Num() < 2) return;

    // Add balcony to random upper floor window
    for (int32 i = 0; i < Building.Windows.Num(); ++i)
    {
        if (Building.Windows[i].FloorIndex > 0 && Random.FRand() < 0.3f)
        {
            FForge2FurnitureData Balcony;
            Balcony.FurnitureType = FName("Balcony");
            Balcony.Position = Building.Windows[i].Position - FVector(0, 0, Building.Windows[i].Size.Y * 0.5f);
            Balcony.Rotation = Building.Windows[i].Rotation;
            Balcony.Scale = FVector(1.0f);
            Balcony.RoomIndex = -1;

            Building.Furniture.Add(Balcony);
        }
    }
}

void FForge2BuildingDecorator::AddExteriorLighting(FForge2BuildingData& Building, EForge2BuildingStyle Style, FRandomStream& Random)
{
    // Add lights near doors
    for (const FForge2DoorData& Door : Building.Doors)
    {
        FForge2FurnitureData Light;

        switch (Style)
        {
            case EForge2BuildingStyle::Medieval:
            case EForge2BuildingStyle::Fantasy:
                Light.FurnitureType = FName("Torch");
                break;
            case EForge2BuildingStyle::Modern:
            case EForge2BuildingStyle::SciFi:
                Light.FurnitureType = FName("WallLight");
                break;
            case EForge2BuildingStyle::Victorian:
                Light.FurnitureType = FName("Lantern");
                break;
            default:
                Light.FurnitureType = FName("WallLight");
                break;
        }

        // Place beside door
        FVector Offset = Door.Rotation.RotateVector(FVector(Door.Size.X * 0.7f, 0, Door.Size.Y * 0.6f));
        Light.Position = Door.Position + Offset;
        Light.Rotation = Door.Rotation;
        Light.Scale = FVector(1.0f);
        Light.RoomIndex = -1;

        Building.Furniture.Add(Light);
    }
}

void FForge2BuildingDecorator::AddTrim(FForge2BuildingData& Building, EForge2BuildingStyle Style)
{
    // Trim would be added as additional geometry data
    // For now, mark as a furniture type for the mesh generator to handle
    FForge2FurnitureData Trim;
    Trim.FurnitureType = FName("WallTrim");
    Trim.Position = Building.Origin;
    Trim.Rotation = Building.Rotation;
    Trim.Scale = FVector(1.0f);
    Trim.RoomIndex = -1;

    Building.Furniture.Add(Trim);
}

void FForge2BuildingDecorator::AddShutters(FForge2BuildingData& Building, EForge2BuildingStyle Style)
{
    if (Style != EForge2BuildingStyle::Colonial &&
        Style != EForge2BuildingStyle::Victorian &&
        Style != EForge2BuildingStyle::Medieval)
    {
        return;
    }

    for (const FForge2WindowData& Window : Building.Windows)
    {
        FForge2FurnitureData Shutter;
        Shutter.FurnitureType = FName("WindowShutter");
        Shutter.Position = Window.Position;
        Shutter.Rotation = Window.Rotation;
        Shutter.Scale = FVector(1.0f);
        Shutter.RoomIndex = -1;

        Building.Furniture.Add(Shutter);
    }
}

void FForge2BuildingDecorator::AddFlowerBoxes(FForge2BuildingData& Building, float Chance, FRandomStream& Random)
{
    for (const FForge2WindowData& Window : Building.Windows)
    {
        if (Random.FRand() < Chance)
        {
            FForge2FurnitureData FlowerBox;
            FlowerBox.FurnitureType = FName("FlowerBox");
            FlowerBox.Position = Window.Position - FVector(0, 0, Window.Size.Y * 0.5f + 20.0f);
            FlowerBox.Rotation = Window.Rotation;
            FlowerBox.Scale = FVector(1.0f);
            FlowerBox.RoomIndex = -1;

            Building.Furniture.Add(FlowerBox);
        }
    }
}

void FForge2BuildingDecorator::AddAwnings(FForge2BuildingData& Building, EForge2BuildingStyle Style, float Chance, FRandomStream& Random)
{
    if (Style != EForge2BuildingStyle::Modern &&
        Style != EForge2BuildingStyle::Victorian &&
        Style != EForge2BuildingStyle::Colonial)
    {
        return;
    }

    for (const FForge2DoorData& Door : Building.Doors)
    {
        if (Door.bExterior && Random.FRand() < Chance)
        {
            FForge2FurnitureData Awning;
            Awning.FurnitureType = FName("Awning");
            Awning.Position = Door.Position + FVector(0, 0, Door.Size.Y + 30.0f);
            Awning.Rotation = Door.Rotation;
            Awning.Scale = FVector(1.0f);
            Awning.RoomIndex = -1;

            Building.Furniture.Add(Awning);
        }
    }
}

void FForge2BuildingDecorator::AddSignage(FForge2BuildingData& Building, const FString& BuildingName, FRandomStream& Random)
{
    if (Building.Doors.Num() == 0) return;

    const FForge2DoorData& MainDoor = Building.Doors[0];

    FForge2FurnitureData Sign;
    Sign.FurnitureType = FName("Sign");
    Sign.Position = MainDoor.Position + FVector(0, 0, MainDoor.Size.Y + 50.0f);
    Sign.Rotation = MainDoor.Rotation;
    Sign.Scale = FVector(1.0f);
    Sign.RoomIndex = -1;

    Building.Furniture.Add(Sign);
}
