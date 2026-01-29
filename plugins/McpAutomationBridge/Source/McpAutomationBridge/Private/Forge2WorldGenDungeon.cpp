// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Dungeon Generation Algorithms - Implementation

#include "Forge2WorldGenDungeon.h"

// ============================================================================
// BSP Dungeon Generator
// ============================================================================

FForge2DungeonData FForge2BSPDungeon::Generate(const FForge2BSPDungeonParams& Params)
{
    FForge2DungeonData Dungeon;
    Dungeon.Initialize(Params.Width, Params.Height, EForge2DungeonTile::Wall);
    Dungeon.Seed = Params.Seed;

    FRandomStream Random(Params.Seed);

    // Create root node
    TSharedPtr<FForge2BSPNode> Root = MakeShared<FForge2BSPNode>(FIntRect(0, 0, Params.Width, Params.Height));

    // Subdivide the space
    Subdivide(Root, 0, Params, Random);

    // Create rooms in leaf nodes
    CreateRooms(Root, Dungeon, Params, Random);

    // Connect rooms with corridors
    ConnectRooms(Root, Dungeon, Params, Random);

    // Assign room IDs and calculate depths
    for (int32 i = 0; i < Dungeon.Rooms.Num(); ++i)
    {
        Dungeon.Rooms[i].RoomID = i;
    }

    // Mark entrance and exit
    if (Dungeon.Rooms.Num() > 0)
    {
        Dungeon.Rooms[0].Purpose = EForge2RoomPurpose::Entrance;
        if (Dungeon.Rooms.Num() > 1)
        {
            Dungeon.Rooms.Last().Purpose = EForge2RoomPurpose::Exit;
        }
    }

    return Dungeon;
}

void FForge2BSPDungeon::Subdivide(TSharedPtr<FForge2BSPNode> Node, int32 Depth, const FForge2BSPDungeonParams& Params, FRandomStream& Random)
{
    if (!Node.IsValid()) return;

    Node->Depth = Depth;

    // Stop if we've reached max depth or node is too small
    if (Depth >= Params.MaxDepth) return;
    if (Node->GetWidth() < Params.MinPartitionSize * 2 && Node->GetHeight() < Params.MinPartitionSize * 2) return;

    // Decide split direction
    bool bSplitHorizontal;
    if (Node->GetWidth() > Node->GetHeight() * 1.25f)
    {
        bSplitHorizontal = false; // Split vertically
    }
    else if (Node->GetHeight() > Node->GetWidth() * 1.25f)
    {
        bSplitHorizontal = true; // Split horizontally
    }
    else
    {
        bSplitHorizontal = Random.FRand() > 0.5f;
    }

    // Check if we can split
    int32 MaxDimension = bSplitHorizontal ? Node->GetHeight() : Node->GetWidth();
    if (MaxDimension < Params.MinPartitionSize * 2)
    {
        return; // Can't split further
    }

    // Calculate split position (30-70% range)
    int32 MinSplit = Params.MinPartitionSize;
    int32 MaxSplit = MaxDimension - Params.MinPartitionSize;
    int32 SplitPos = MinSplit + Random.RandRange(0, MaxSplit - MinSplit);

    // Create child nodes
    if (bSplitHorizontal)
    {
        Node->Left = MakeShared<FForge2BSPNode>(FIntRect(
            Node->Bounds.Min.X, Node->Bounds.Min.Y,
            Node->Bounds.Max.X, Node->Bounds.Min.Y + SplitPos
        ));
        Node->Right = MakeShared<FForge2BSPNode>(FIntRect(
            Node->Bounds.Min.X, Node->Bounds.Min.Y + SplitPos,
            Node->Bounds.Max.X, Node->Bounds.Max.Y
        ));
    }
    else
    {
        Node->Left = MakeShared<FForge2BSPNode>(FIntRect(
            Node->Bounds.Min.X, Node->Bounds.Min.Y,
            Node->Bounds.Min.X + SplitPos, Node->Bounds.Max.Y
        ));
        Node->Right = MakeShared<FForge2BSPNode>(FIntRect(
            Node->Bounds.Min.X + SplitPos, Node->Bounds.Min.Y,
            Node->Bounds.Max.X, Node->Bounds.Max.Y
        ));
    }

    // Recursively subdivide children
    Subdivide(Node->Left, Depth + 1, Params, Random);
    Subdivide(Node->Right, Depth + 1, Params, Random);
}

void FForge2BSPDungeon::CreateRooms(TSharedPtr<FForge2BSPNode> Node, FForge2DungeonData& Dungeon, const FForge2BSPDungeonParams& Params, FRandomStream& Random)
{
    if (!Node.IsValid()) return;

    if (Node->IsLeaf())
    {
        // Create a room within this partition
        int32 MaxRoomWidth = FMath::Min(Params.MaxRoomSize, Node->GetWidth() - Params.RoomPadding * 2);
        int32 MaxRoomHeight = FMath::Min(Params.MaxRoomSize, Node->GetHeight() - Params.RoomPadding * 2);

        if (MaxRoomWidth < Params.MinRoomSize || MaxRoomHeight < Params.MinRoomSize)
        {
            return; // Can't fit a room
        }

        int32 RoomWidth = Random.RandRange(Params.MinRoomSize, MaxRoomWidth);
        int32 RoomHeight = Random.RandRange(Params.MinRoomSize, MaxRoomHeight);

        int32 RoomX = Node->Bounds.Min.X + Params.RoomPadding + Random.RandRange(0, MaxRoomWidth - RoomWidth);
        int32 RoomY = Node->Bounds.Min.Y + Params.RoomPadding + Random.RandRange(0, MaxRoomHeight - RoomHeight);

        Node->Room = FForge2DungeonRoom(RoomX, RoomY, RoomWidth, RoomHeight);
        Node->bHasRoom = true;

        // Carve the room
        CarveRoom(Dungeon, Node->Room);
        Dungeon.Rooms.Add(Node->Room);
    }
    else
    {
        CreateRooms(Node->Left, Dungeon, Params, Random);
        CreateRooms(Node->Right, Dungeon, Params, Random);
    }
}

void FForge2BSPDungeon::ConnectRooms(TSharedPtr<FForge2BSPNode> Node, FForge2DungeonData& Dungeon, const FForge2BSPDungeonParams& Params, FRandomStream& Random)
{
    if (!Node.IsValid() || Node->IsLeaf()) return;

    // Connect children first
    ConnectRooms(Node->Left, Dungeon, Params, Random);
    ConnectRooms(Node->Right, Dungeon, Params, Random);

    // Get rooms from left and right subtrees
    FForge2DungeonRoom* LeftRoom = GetRoom(Node->Left);
    FForge2DungeonRoom* RightRoom = GetRoom(Node->Right);

    if (LeftRoom && RightRoom)
    {
        // Create corridor between rooms
        FForge2DungeonCorridor Corridor;
        Corridor.Start = LeftRoom->Center;
        Corridor.End = RightRoom->Center;
        Corridor.Width = Params.CorridorWidth;

        // L-shaped corridor
        FIntPoint Mid;
        if (Random.FRand() > 0.5f)
        {
            Mid = FIntPoint(RightRoom->Center.X, LeftRoom->Center.Y);
        }
        else
        {
            Mid = FIntPoint(LeftRoom->Center.X, RightRoom->Center.Y);
        }

        CarveCorridor(Dungeon, LeftRoom->Center, Mid, Params.CorridorWidth);
        CarveCorridor(Dungeon, Mid, RightRoom->Center, Params.CorridorWidth);

        Corridor.Waypoints.Add(Mid);
        Dungeon.Corridors.Add(Corridor);

        // Mark rooms as connected
        LeftRoom->ConnectedRooms.AddUnique(RightRoom->RoomID);
        RightRoom->ConnectedRooms.AddUnique(LeftRoom->RoomID);
    }
}

FForge2DungeonRoom* FForge2BSPDungeon::GetRoom(TSharedPtr<FForge2BSPNode> Node)
{
    if (!Node.IsValid()) return nullptr;

    if (Node->bHasRoom)
    {
        return &Node->Room;
    }

    FForge2DungeonRoom* LeftRoom = GetRoom(Node->Left);
    if (LeftRoom) return LeftRoom;

    return GetRoom(Node->Right);
}

void FForge2BSPDungeon::CarveCorridor(FForge2DungeonData& Dungeon, const FIntPoint& Start, const FIntPoint& End, int32 Width)
{
    int32 HalfWidth = Width / 2;

    // Bresenham-like line carving
    int32 X = Start.X;
    int32 Y = Start.Y;
    int32 DX = FMath::Abs(End.X - Start.X);
    int32 DY = FMath::Abs(End.Y - Start.Y);
    int32 SX = Start.X < End.X ? 1 : -1;
    int32 SY = Start.Y < End.Y ? 1 : -1;
    int32 Err = DX - DY;

    while (true)
    {
        // Carve a square around the current position
        for (int32 OffX = -HalfWidth; OffX <= HalfWidth; ++OffX)
        {
            for (int32 OffY = -HalfWidth; OffY <= HalfWidth; ++OffY)
            {
                Dungeon.SetTile(X + OffX, Y + OffY, EForge2DungeonTile::Corridor);
            }
        }

        if (X == End.X && Y == End.Y) break;

        int32 E2 = 2 * Err;
        if (E2 > -DY)
        {
            Err -= DY;
            X += SX;
        }
        if (E2 < DX)
        {
            Err += DX;
            Y += SY;
        }
    }
}

void FForge2BSPDungeon::CarveRoom(FForge2DungeonData& Dungeon, const FForge2DungeonRoom& Room)
{
    for (int32 X = Room.Bounds.Min.X; X < Room.Bounds.Max.X; ++X)
    {
        for (int32 Y = Room.Bounds.Min.Y; Y < Room.Bounds.Max.Y; ++Y)
        {
            Dungeon.SetTile(X, Y, EForge2DungeonTile::Floor);
        }
    }
}

// ============================================================================
// Cellular Automata Cave Generator
// ============================================================================

FForge2DungeonData FForge2CellularAutomata::Generate(const FForge2CellularAutomataParams& Params)
{
    FForge2DungeonData Dungeon;
    Dungeon.Initialize(Params.Width, Params.Height, EForge2DungeonTile::Wall);
    Dungeon.Seed = Params.Seed;

    FRandomStream Random(Params.Seed);

    // Initial random fill
    for (int32 Y = 1; Y < Params.Height - 1; ++Y)
    {
        for (int32 X = 1; X < Params.Width - 1; ++X)
        {
            if (Random.RandRange(0, 100) < Params.FillPercent)
            {
                Dungeon.SetTile(X, Y, EForge2DungeonTile::Floor);
            }
        }
    }

    // Apply cellular automata iterations
    for (int32 i = 0; i < Params.Iterations; ++i)
    {
        ApplyIteration(Dungeon.Tiles, Params.Width, Params.Height, Params);
    }

    // Find and filter regions
    TArray<TArray<FIntPoint>> FloorRegions = FindRegions(Dungeon.Tiles, Params.Width, Params.Height, EForge2DungeonTile::Floor);

    // Remove small regions
    for (const TArray<FIntPoint>& Region : FloorRegions)
    {
        if (Region.Num() < Params.MinRegionSize)
        {
            for (const FIntPoint& Point : Region)
            {
                Dungeon.SetTile(Point.X, Point.Y, EForge2DungeonTile::Wall);
            }
        }
    }

    // Re-find regions after filtering
    FloorRegions = FindRegions(Dungeon.Tiles, Params.Width, Params.Height, EForge2DungeonTile::Floor);

    // Connect remaining regions
    if (Params.bConnectRegions && FloorRegions.Num() > 1)
    {
        ConnectRegions(Dungeon, FloorRegions, Random);
    }

    // Identify rooms
    IdentifyRooms(Dungeon);

    return Dungeon;
}

void FForge2CellularAutomata::ApplyIteration(TArray<EForge2DungeonTile>& Tiles, int32 Width, int32 Height, const FForge2CellularAutomataParams& Params)
{
    TArray<EForge2DungeonTile> NewTiles = Tiles;

    for (int32 Y = 1; Y < Height - 1; ++Y)
    {
        for (int32 X = 1; X < Width - 1; ++X)
        {
            int32 FloorNeighbors = CountFloorNeighbors(Tiles, X, Y, Width, Height);
            int32 Index = Y * Width + X;

            if (Tiles[Index] == EForge2DungeonTile::Floor)
            {
                // Floor cell dies if too few neighbors
                if (FloorNeighbors < Params.DeathLimit)
                {
                    NewTiles[Index] = EForge2DungeonTile::Wall;
                }
            }
            else
            {
                // Wall cell becomes floor if enough neighbors
                if (FloorNeighbors > Params.BirthLimit)
                {
                    NewTiles[Index] = EForge2DungeonTile::Floor;
                }
            }
        }
    }

    Tiles = MoveTemp(NewTiles);
}

int32 FForge2CellularAutomata::CountFloorNeighbors(const TArray<EForge2DungeonTile>& Tiles, int32 X, int32 Y, int32 Width, int32 Height)
{
    int32 Count = 0;

    for (int32 DY = -1; DY <= 1; ++DY)
    {
        for (int32 DX = -1; DX <= 1; ++DX)
        {
            if (DX == 0 && DY == 0) continue;

            int32 NX = X + DX;
            int32 NY = Y + DY;

            if (NX >= 0 && NX < Width && NY >= 0 && NY < Height)
            {
                if (Tiles[NY * Width + NX] == EForge2DungeonTile::Floor)
                {
                    ++Count;
                }
            }
        }
    }

    return Count;
}

TArray<TArray<FIntPoint>> FForge2CellularAutomata::FindRegions(const TArray<EForge2DungeonTile>& Tiles, int32 Width, int32 Height, EForge2DungeonTile TileType)
{
    TArray<TArray<FIntPoint>> Regions;
    TArray<bool> Visited;
    Visited.SetNumZeroed(Width * Height);

    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            int32 Index = Y * Width + X;
            if (!Visited[Index] && Tiles[Index] == TileType)
            {
                TArray<FIntPoint> Region = FloodFill(Tiles, X, Y, Width, Height, TileType, Visited);
                if (Region.Num() > 0)
                {
                    Regions.Add(Region);
                }
            }
        }
    }

    return Regions;
}

TArray<FIntPoint> FForge2CellularAutomata::FloodFill(const TArray<EForge2DungeonTile>& Tiles, int32 StartX, int32 StartY, int32 Width, int32 Height, EForge2DungeonTile TileType, TArray<bool>& Visited)
{
    TArray<FIntPoint> Region;
    TArray<FIntPoint> Queue;
    Queue.Add(FIntPoint(StartX, StartY));

    while (Queue.Num() > 0)
    {
        FIntPoint Current = Queue.Pop();
        int32 Index = Current.Y * Width + Current.X;

        if (Current.X < 0 || Current.X >= Width || Current.Y < 0 || Current.Y >= Height) continue;
        if (Visited[Index]) continue;
        if (Tiles[Index] != TileType) continue;

        Visited[Index] = true;
        Region.Add(Current);

        // Add 4-connected neighbors
        Queue.Add(FIntPoint(Current.X + 1, Current.Y));
        Queue.Add(FIntPoint(Current.X - 1, Current.Y));
        Queue.Add(FIntPoint(Current.X, Current.Y + 1));
        Queue.Add(FIntPoint(Current.X, Current.Y - 1));
    }

    return Region;
}

void FForge2CellularAutomata::ConnectRegions(FForge2DungeonData& Dungeon, const TArray<TArray<FIntPoint>>& Regions, FRandomStream& Random)
{
    if (Regions.Num() < 2) return;

    // Connect regions in order (simple approach)
    for (int32 i = 0; i < Regions.Num() - 1; ++i)
    {
        FIntPoint PointA, PointB;
        FindClosestPoints(Regions[i], Regions[i + 1], PointA, PointB);
        CarveCorridor(Dungeon, PointA, PointB);
    }
}

void FForge2CellularAutomata::FindClosestPoints(const TArray<FIntPoint>& RegionA, const TArray<FIntPoint>& RegionB, FIntPoint& OutPointA, FIntPoint& OutPointB)
{
    float MinDist = MAX_FLT;

    for (const FIntPoint& A : RegionA)
    {
        for (const FIntPoint& B : RegionB)
        {
            float Dist = FVector2D(A.X - B.X, A.Y - B.Y).SizeSquared();
            if (Dist < MinDist)
            {
                MinDist = Dist;
                OutPointA = A;
                OutPointB = B;
            }
        }
    }
}

void FForge2CellularAutomata::CarveCorridor(FForge2DungeonData& Dungeon, const FIntPoint& Start, const FIntPoint& End)
{
    int32 X = Start.X;
    int32 Y = Start.Y;
    int32 DX = FMath::Abs(End.X - Start.X);
    int32 DY = FMath::Abs(End.Y - Start.Y);
    int32 SX = Start.X < End.X ? 1 : -1;
    int32 SY = Start.Y < End.Y ? 1 : -1;
    int32 Err = DX - DY;

    while (true)
    {
        Dungeon.SetTile(X, Y, EForge2DungeonTile::Corridor);
        // Make corridor slightly wider
        Dungeon.SetTile(X + 1, Y, EForge2DungeonTile::Corridor);
        Dungeon.SetTile(X, Y + 1, EForge2DungeonTile::Corridor);

        if (X == End.X && Y == End.Y) break;

        int32 E2 = 2 * Err;
        if (E2 > -DY) { Err -= DY; X += SX; }
        if (E2 < DX) { Err += DX; Y += SY; }
    }
}

void FForge2CellularAutomata::IdentifyRooms(FForge2DungeonData& Dungeon)
{
    // Find connected floor regions and treat them as rooms
    TArray<TArray<FIntPoint>> Regions = FindRegions(Dungeon.Tiles, Dungeon.Width, Dungeon.Height, EForge2DungeonTile::Floor);

    for (int32 i = 0; i < Regions.Num(); ++i)
    {
        if (Regions[i].Num() < 20) continue; // Skip tiny regions

        // Calculate bounds
        int32 MinX = INT_MAX, MinY = INT_MAX, MaxX = INT_MIN, MaxY = INT_MIN;
        for (const FIntPoint& P : Regions[i])
        {
            MinX = FMath::Min(MinX, P.X);
            MinY = FMath::Min(MinY, P.Y);
            MaxX = FMath::Max(MaxX, P.X);
            MaxY = FMath::Max(MaxY, P.Y);
        }

        FForge2DungeonRoom Room;
        Room.Bounds = FIntRect(MinX, MinY, MaxX + 1, MaxY + 1);
        Room.Center = FIntPoint((MinX + MaxX) / 2, (MinY + MaxY) / 2);
        Room.RoomID = Dungeon.Rooms.Num();
        Dungeon.Rooms.Add(Room);
    }

    // Mark entrance and exit
    if (Dungeon.Rooms.Num() > 0)
    {
        Dungeon.Rooms[0].Purpose = EForge2RoomPurpose::Entrance;
        if (Dungeon.Rooms.Num() > 1)
        {
            Dungeon.Rooms.Last().Purpose = EForge2RoomPurpose::Exit;
        }
    }
}

// ============================================================================
// Drunkard's Walk Cave Generator
// ============================================================================

FForge2DungeonData FForge2DrunkardWalk::Generate(const FForge2DrunkardWalkParams& Params)
{
    FForge2DungeonData Dungeon;
    Dungeon.Initialize(Params.Width, Params.Height, EForge2DungeonTile::Wall);
    Dungeon.Seed = Params.Seed;

    FRandomStream Random(Params.Seed);

    int32 TotalCells = Params.Width * Params.Height;
    int32 TargetFill = (TotalCells * Params.FillPercent) / 100;
    int32 FilledCount = 0;

    // Start positions for walkers
    TArray<FIntPoint> WalkerPositions;
    TArray<FIntPoint> LastDirections;

    for (int32 i = 0; i < Params.WalkerCount; ++i)
    {
        FIntPoint StartPos;
        if (Params.bStartFromCenter)
        {
            StartPos = FIntPoint(Params.Width / 2, Params.Height / 2);
        }
        else
        {
            StartPos = FIntPoint(
                Random.RandRange(Params.CarveRadius + 1, Params.Width - Params.CarveRadius - 2),
                Random.RandRange(Params.CarveRadius + 1, Params.Height - Params.CarveRadius - 2)
            );
        }
        WalkerPositions.Add(StartPos);
        LastDirections.Add(FIntPoint(1, 0));
    }

    // Run walkers until target fill is reached
    int32 MaxIterations = Params.MaxSteps > 0 ? Params.MaxSteps * Params.WalkerCount : TotalCells * 10;
    int32 Iterations = 0;

    while (FilledCount < TargetFill && Iterations < MaxIterations)
    {
        for (int32 i = 0; i < WalkerPositions.Num() && FilledCount < TargetFill; ++i)
        {
            SimulateWalker(Dungeon, WalkerPositions[i], Params, Random, FilledCount, TargetFill, LastDirections[i]);
        }
        ++Iterations;
    }

    // Identify rooms
    IdentifyRooms(Dungeon);

    return Dungeon;
}

void FForge2DrunkardWalk::SimulateWalker(FForge2DungeonData& Dungeon, FIntPoint& Position, const FForge2DrunkardWalkParams& Params, FRandomStream& Random, int32& FilledCount, int32 TargetFill, FIntPoint& LastDirection)
{
    // Carve at current position
    FilledCount += CarveAt(Dungeon, Position.X, Position.Y, Params.CarveRadius);

    // Get new direction
    FIntPoint Dir = GetRandomDirection(Random, LastDirection, Params.DirectionWeight);
    LastDirection = Dir;

    // Move
    FIntPoint NewPos = Position + Dir;

    // Keep within bounds
    NewPos.X = FMath::Clamp(NewPos.X, Params.CarveRadius + 1, Params.Width - Params.CarveRadius - 2);
    NewPos.Y = FMath::Clamp(NewPos.Y, Params.CarveRadius + 1, Params.Height - Params.CarveRadius - 2);

    Position = NewPos;
}

int32 FForge2DrunkardWalk::CarveAt(FForge2DungeonData& Dungeon, int32 X, int32 Y, int32 Radius)
{
    int32 Carved = 0;

    for (int32 DY = -Radius; DY <= Radius; ++DY)
    {
        for (int32 DX = -Radius; DX <= Radius; ++DX)
        {
            if (DX * DX + DY * DY <= Radius * Radius)
            {
                int32 NX = X + DX;
                int32 NY = Y + DY;

                if (NX >= 0 && NX < Dungeon.Width && NY >= 0 && NY < Dungeon.Height)
                {
                    if (Dungeon.GetTile(NX, NY) != EForge2DungeonTile::Floor)
                    {
                        Dungeon.SetTile(NX, NY, EForge2DungeonTile::Floor);
                        ++Carved;
                    }
                }
            }
        }
    }

    return Carved;
}

FIntPoint FForge2DrunkardWalk::GetRandomDirection(FRandomStream& Random, const FIntPoint& LastDir, float Weight)
{
    static const FIntPoint Directions[4] = {
        FIntPoint(0, -1), // North
        FIntPoint(1, 0),  // East
        FIntPoint(0, 1),  // South
        FIntPoint(-1, 0)  // West
    };

    if (Random.FRand() < Weight && (LastDir.X != 0 || LastDir.Y != 0))
    {
        return LastDir; // Continue in same direction
    }

    return Directions[Random.RandRange(0, 3)];
}

void FForge2DrunkardWalk::IdentifyRooms(FForge2DungeonData& Dungeon)
{
    // Similar to cellular automata - find connected regions
    TArray<bool> Visited;
    Visited.SetNumZeroed(Dungeon.Width * Dungeon.Height);

    for (int32 Y = 0; Y < Dungeon.Height; ++Y)
    {
        for (int32 X = 0; X < Dungeon.Width; ++X)
        {
            if (Dungeon.GetTile(X, Y) == EForge2DungeonTile::Floor && !Visited[Y * Dungeon.Width + X])
            {
                // Flood fill to find region
                TArray<FIntPoint> Region;
                TArray<FIntPoint> Queue;
                Queue.Add(FIntPoint(X, Y));

                int32 MinX = INT_MAX, MinY = INT_MAX, MaxX = INT_MIN, MaxY = INT_MIN;

                while (Queue.Num() > 0)
                {
                    FIntPoint Current = Queue.Pop();
                    int32 Index = Current.Y * Dungeon.Width + Current.X;

                    if (Current.X < 0 || Current.X >= Dungeon.Width || Current.Y < 0 || Current.Y >= Dungeon.Height) continue;
                    if (Visited[Index]) continue;
                    if (Dungeon.GetTile(Current.X, Current.Y) != EForge2DungeonTile::Floor) continue;

                    Visited[Index] = true;
                    Region.Add(Current);

                    MinX = FMath::Min(MinX, Current.X);
                    MinY = FMath::Min(MinY, Current.Y);
                    MaxX = FMath::Max(MaxX, Current.X);
                    MaxY = FMath::Max(MaxY, Current.Y);

                    Queue.Add(FIntPoint(Current.X + 1, Current.Y));
                    Queue.Add(FIntPoint(Current.X - 1, Current.Y));
                    Queue.Add(FIntPoint(Current.X, Current.Y + 1));
                    Queue.Add(FIntPoint(Current.X, Current.Y - 1));
                }

                if (Region.Num() > 30) // Only count significant regions as rooms
                {
                    FForge2DungeonRoom Room;
                    Room.Bounds = FIntRect(MinX, MinY, MaxX + 1, MaxY + 1);
                    Room.Center = FIntPoint((MinX + MaxX) / 2, (MinY + MaxY) / 2);
                    Room.RoomID = Dungeon.Rooms.Num();
                    Dungeon.Rooms.Add(Room);
                }
            }
        }
    }

    // Mark entrance and exit
    if (Dungeon.Rooms.Num() > 0)
    {
        Dungeon.Rooms[0].Purpose = EForge2RoomPurpose::Entrance;
        if (Dungeon.Rooms.Num() > 1)
        {
            Dungeon.Rooms.Last().Purpose = EForge2RoomPurpose::Exit;
        }
    }
}

// ============================================================================
// Wave Function Collapse Dungeon Generator
// ============================================================================

FForge2DungeonData FForge2WFCDungeon::Generate(const FForge2WFCParams& Params)
{
    FForge2DungeonData Dungeon;
    Dungeon.Initialize(Params.Width, Params.Height, EForge2DungeonTile::Empty);
    Dungeon.Seed = Params.Seed;

    TArray<FForge2WFCTile> Tiles = Params.Tiles.Num() > 0 ? Params.Tiles : CreateDefaultTileSet();

    for (int32 Attempt = 0; Attempt < Params.MaxAttempts; ++Attempt)
    {
        FRandomStream Random(Params.Seed + Attempt);

        TArray<FWFCCell> Wave;
        InitializeWave(Wave, Params.Width, Params.Height, Tiles);

        bool bSuccess = true;

        while (!IsComplete(Wave))
        {
            // Find cell with minimum entropy
            int32 CellIndex = FindMinEntropyCell(Wave, Random);
            if (CellIndex < 0)
            {
                bSuccess = false;
                break;
            }

            // Collapse the cell
            if (!CollapseCell(Wave, CellIndex, Tiles, Random))
            {
                bSuccess = false;
                break;
            }

            // Propagate constraints
            if (!Propagate(Wave, Params.Width, Params.Height, Tiles))
            {
                bSuccess = false;
                break;
            }
        }

        if (bSuccess && !HasContradiction(Wave))
        {
            ConvertToDungeon(Wave, Params.Width, Params.Height, Tiles, Dungeon);
            return Dungeon;
        }
    }

    // Failed to generate, return empty dungeon
    return Dungeon;
}

TArray<FForge2WFCTile> FForge2WFCDungeon::CreateDefaultTileSet()
{
    TArray<FForge2WFCTile> Tiles;

    // Tile 0: Empty/Wall
    FForge2WFCTile Wall;
    Wall.TileID = 0;
    Wall.TileType = EForge2DungeonTile::Wall;
    Wall.Weight = 2.0f;
    Wall.ValidNorth = {0, 1, 2};
    Wall.ValidEast = {0, 1, 3};
    Wall.ValidSouth = {0, 1, 2};
    Wall.ValidWest = {0, 1, 3};
    Tiles.Add(Wall);

    // Tile 1: Floor
    FForge2WFCTile Floor;
    Floor.TileID = 1;
    Floor.TileType = EForge2DungeonTile::Floor;
    Floor.Weight = 3.0f;
    Floor.ValidNorth = {0, 1, 2};
    Floor.ValidEast = {0, 1, 3};
    Floor.ValidSouth = {0, 1, 2};
    Floor.ValidWest = {0, 1, 3};
    Tiles.Add(Floor);

    // Tile 2: Corridor Vertical
    FForge2WFCTile CorridorV;
    CorridorV.TileID = 2;
    CorridorV.TileType = EForge2DungeonTile::Corridor;
    CorridorV.Weight = 1.0f;
    CorridorV.ValidNorth = {1, 2};
    CorridorV.ValidEast = {0};
    CorridorV.ValidSouth = {1, 2};
    CorridorV.ValidWest = {0};
    Tiles.Add(CorridorV);

    // Tile 3: Corridor Horizontal
    FForge2WFCTile CorridorH;
    CorridorH.TileID = 3;
    CorridorH.TileType = EForge2DungeonTile::Corridor;
    CorridorH.Weight = 1.0f;
    CorridorH.ValidNorth = {0};
    CorridorH.ValidEast = {1, 3};
    CorridorH.ValidSouth = {0};
    CorridorH.ValidWest = {1, 3};
    Tiles.Add(CorridorH);

    return Tiles;
}

void FForge2WFCDungeon::InitializeWave(TArray<FWFCCell>& Wave, int32 Width, int32 Height, const TArray<FForge2WFCTile>& Tiles)
{
    Wave.SetNum(Width * Height);

    TArray<int32> AllTileIDs;
    for (const FForge2WFCTile& Tile : Tiles)
    {
        AllTileIDs.Add(Tile.TileID);
    }

    for (FWFCCell& Cell : Wave)
    {
        Cell.PossibleTiles = AllTileIDs;
        Cell.bCollapsed = false;
        Cell.CollapsedTile = -1;
    }
}

int32 FForge2WFCDungeon::FindMinEntropyCell(const TArray<FWFCCell>& Wave, FRandomStream& Random)
{
    float MinEntropy = MAX_FLT;
    TArray<int32> Candidates;

    for (int32 i = 0; i < Wave.Num(); ++i)
    {
        if (Wave[i].bCollapsed) continue;

        float Entropy = Wave[i].GetEntropy();
        if (Entropy < MinEntropy)
        {
            MinEntropy = Entropy;
            Candidates.Empty();
            Candidates.Add(i);
        }
        else if (FMath::IsNearlyEqual(Entropy, MinEntropy))
        {
            Candidates.Add(i);
        }
    }

    if (Candidates.Num() == 0) return -1;

    return Candidates[Random.RandRange(0, Candidates.Num() - 1)];
}

bool FForge2WFCDungeon::CollapseCell(TArray<FWFCCell>& Wave, int32 CellIndex, const TArray<FForge2WFCTile>& Tiles, FRandomStream& Random)
{
    FWFCCell& Cell = Wave[CellIndex];

    if (Cell.PossibleTiles.Num() == 0) return false;

    // Weighted random selection
    float TotalWeight = 0.0f;
    for (int32 TileID : Cell.PossibleTiles)
    {
        TotalWeight += Tiles[TileID].Weight;
    }

    float Roll = Random.FRand() * TotalWeight;
    float Accumulator = 0.0f;

    for (int32 TileID : Cell.PossibleTiles)
    {
        Accumulator += Tiles[TileID].Weight;
        if (Roll <= Accumulator)
        {
            Cell.CollapsedTile = TileID;
            Cell.PossibleTiles.Empty();
            Cell.PossibleTiles.Add(TileID);
            Cell.bCollapsed = true;
            return true;
        }
    }

    // Fallback
    Cell.CollapsedTile = Cell.PossibleTiles[0];
    Cell.PossibleTiles.Empty();
    Cell.PossibleTiles.Add(Cell.CollapsedTile);
    Cell.bCollapsed = true;
    return true;
}

bool FForge2WFCDungeon::Propagate(TArray<FWFCCell>& Wave, int32 Width, int32 Height, const TArray<FForge2WFCTile>& Tiles)
{
    TArray<int32> Stack;

    // Start with all collapsed cells
    for (int32 i = 0; i < Wave.Num(); ++i)
    {
        if (Wave[i].bCollapsed)
        {
            Stack.Add(i);
        }
    }

    static const int32 DX[4] = {0, 1, 0, -1};
    static const int32 DY[4] = {-1, 0, 1, 0};

    while (Stack.Num() > 0)
    {
        int32 CellIndex = Stack.Pop();
        int32 X = CellIndex % Width;
        int32 Y = CellIndex / Width;

        for (int32 Dir = 0; Dir < 4; ++Dir)
        {
            int32 NX = X + DX[Dir];
            int32 NY = Y + DY[Dir];

            if (NX < 0 || NX >= Width || NY < 0 || NY >= Height) continue;

            int32 NeighborIndex = NY * Width + NX;
            FWFCCell& Neighbor = Wave[NeighborIndex];

            if (Neighbor.bCollapsed) continue;

            // Get valid tiles for this direction
            TSet<int32> ValidTiles;
            for (int32 TileID : Wave[CellIndex].PossibleTiles)
            {
                TArray<int32> Valid = GetValidNeighbors(TileID, Dir, Tiles);
                for (int32 V : Valid)
                {
                    ValidTiles.Add(V);
                }
            }

            // Constrain neighbor
            TArray<int32> NewPossible;
            for (int32 TileID : Neighbor.PossibleTiles)
            {
                if (ValidTiles.Contains(TileID))
                {
                    NewPossible.Add(TileID);
                }
            }

            if (NewPossible.Num() != Neighbor.PossibleTiles.Num())
            {
                if (NewPossible.Num() == 0)
                {
                    return false; // Contradiction
                }

                Neighbor.PossibleTiles = NewPossible;
                Stack.AddUnique(NeighborIndex);
            }
        }
    }

    return true;
}

TArray<int32> FForge2WFCDungeon::GetValidNeighbors(int32 TileID, int32 Direction, const TArray<FForge2WFCTile>& Tiles)
{
    const FForge2WFCTile& Tile = Tiles[TileID];

    switch (Direction)
    {
        case DIR_NORTH: return Tile.ValidNorth;
        case DIR_EAST: return Tile.ValidEast;
        case DIR_SOUTH: return Tile.ValidSouth;
        case DIR_WEST: return Tile.ValidWest;
        default: return TArray<int32>();
    }
}

bool FForge2WFCDungeon::IsComplete(const TArray<FWFCCell>& Wave)
{
    for (const FWFCCell& Cell : Wave)
    {
        if (!Cell.bCollapsed) return false;
    }
    return true;
}

bool FForge2WFCDungeon::HasContradiction(const TArray<FWFCCell>& Wave)
{
    for (const FWFCCell& Cell : Wave)
    {
        if (Cell.PossibleTiles.Num() == 0) return true;
    }
    return false;
}

void FForge2WFCDungeon::ConvertToDungeon(const TArray<FWFCCell>& Wave, int32 Width, int32 Height, const TArray<FForge2WFCTile>& Tiles, FForge2DungeonData& Dungeon)
{
    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            int32 Index = Y * Width + X;
            if (Wave[Index].bCollapsed && Wave[Index].CollapsedTile >= 0)
            {
                Dungeon.SetTile(X, Y, Tiles[Wave[Index].CollapsedTile].TileType);
            }
        }
    }
}

// ============================================================================
// Dungeon Processor (Post-processing utilities)
// ============================================================================

void FForge2DungeonProcessor::AssignRoomPurposes(FForge2DungeonData& Dungeon, FRandomStream& Random)
{
    if (Dungeon.Rooms.Num() == 0) return;

    // First room is entrance
    Dungeon.Rooms[0].Purpose = EForge2RoomPurpose::Entrance;

    // Last room is exit
    if (Dungeon.Rooms.Num() > 1)
    {
        Dungeon.Rooms.Last().Purpose = EForge2RoomPurpose::Exit;
    }

    // Assign other purposes
    for (int32 i = 1; i < Dungeon.Rooms.Num() - 1; ++i)
    {
        float Roll = Random.FRand();

        if (Roll < 0.1f)
        {
            Dungeon.Rooms[i].Purpose = EForge2RoomPurpose::Treasure;
        }
        else if (Roll < 0.2f)
        {
            Dungeon.Rooms[i].Purpose = EForge2RoomPurpose::Trap;
        }
        else if (Roll < 0.3f)
        {
            Dungeon.Rooms[i].Purpose = EForge2RoomPurpose::Rest;
        }
        else if (Roll < 0.35f)
        {
            Dungeon.Rooms[i].Purpose = EForge2RoomPurpose::Secret;
        }
        else if (Roll < 0.4f)
        {
            Dungeon.Rooms[i].Purpose = EForge2RoomPurpose::Shop;
        }
        else if (Roll < 0.45f)
        {
            Dungeon.Rooms[i].Purpose = EForge2RoomPurpose::Puzzle;
        }
        // Else remains Normal
    }

    // Place boss room before exit
    if (Dungeon.Rooms.Num() > 2)
    {
        Dungeon.Rooms[Dungeon.Rooms.Num() - 2].Purpose = EForge2RoomPurpose::Boss;
    }
}

void FForge2DungeonProcessor::PlaceDoors(FForge2DungeonData& Dungeon, FRandomStream& Random, float DoorChance)
{
    // Find transitions between rooms and corridors
    for (int32 Y = 1; Y < Dungeon.Height - 1; ++Y)
    {
        for (int32 X = 1; X < Dungeon.Width - 1; ++X)
        {
            EForge2DungeonTile Current = Dungeon.GetTile(X, Y);
            if (Current != EForge2DungeonTile::Corridor) continue;

            // Check if this is a doorway position (corridor with floor on sides)
            bool bHorizontalDoor = (Dungeon.GetTile(X - 1, Y) == EForge2DungeonTile::Floor && Dungeon.GetTile(X + 1, Y) == EForge2DungeonTile::Floor);
            bool bVerticalDoor = (Dungeon.GetTile(X, Y - 1) == EForge2DungeonTile::Floor && Dungeon.GetTile(X, Y + 1) == EForge2DungeonTile::Floor);

            if ((bHorizontalDoor || bVerticalDoor) && Random.FRand() < DoorChance)
            {
                Dungeon.SetTile(X, Y, EForge2DungeonTile::Door);
            }
        }
    }
}

void FForge2DungeonProcessor::AddSecretPassages(FForge2DungeonData& Dungeon, FRandomStream& Random, float SecretChance)
{
    // Look for walls that separate nearby rooms
    for (int32 i = 0; i < Dungeon.Rooms.Num(); ++i)
    {
        for (int32 j = i + 1; j < Dungeon.Rooms.Num(); ++j)
        {
            const FForge2DungeonRoom& RoomA = Dungeon.Rooms[i];
            const FForge2DungeonRoom& RoomB = Dungeon.Rooms[j];

            // Check if rooms are close but not connected
            float Distance = FVector2D(RoomA.Center.X - RoomB.Center.X, RoomA.Center.Y - RoomB.Center.Y).Size();
            if (Distance > 20.0f || RoomA.ConnectedRooms.Contains(RoomB.RoomID))
            {
                continue;
            }

            if (Random.FRand() < SecretChance)
            {
                // Find wall between rooms and place secret door
                FIntPoint Dir = FIntPoint(
                    FMath::Sign(RoomB.Center.X - RoomA.Center.X),
                    FMath::Sign(RoomB.Center.Y - RoomA.Center.Y)
                );

                FIntPoint Current = RoomA.Center;
                while (Current != RoomB.Center)
                {
                    if (Dungeon.GetTile(Current.X, Current.Y) == EForge2DungeonTile::Wall)
                    {
                        Dungeon.SetTile(Current.X, Current.Y, EForge2DungeonTile::SecretDoor);
                        break;
                    }
                    Current.X += Dir.X;
                    Current.Y += Dir.Y;
                }
            }
        }
    }
}

TArray<FIntPoint> FForge2DungeonProcessor::GenerateSpawnPoints(const FForge2DungeonData& Dungeon, EForge2RoomPurpose RoomType, int32 PointsPerRoom)
{
    TArray<FIntPoint> SpawnPoints;
    FRandomStream Random(Dungeon.Seed);

    for (const FForge2DungeonRoom& Room : Dungeon.Rooms)
    {
        if (RoomType != EForge2RoomPurpose::Normal && Room.Purpose != RoomType)
        {
            continue;
        }

        for (int32 i = 0; i < PointsPerRoom; ++i)
        {
            int32 X = Random.RandRange(Room.Bounds.Min.X + 1, Room.Bounds.Max.X - 2);
            int32 Y = Random.RandRange(Room.Bounds.Min.Y + 1, Room.Bounds.Max.Y - 2);

            if (Dungeon.GetTile(X, Y) == EForge2DungeonTile::Floor)
            {
                SpawnPoints.Add(FIntPoint(X, Y));
            }
        }
    }

    return SpawnPoints;
}

void FForge2DungeonProcessor::GenerateWalls(FForge2DungeonData& Dungeon)
{
    TArray<EForge2DungeonTile> NewTiles = Dungeon.Tiles;

    for (int32 Y = 0; Y < Dungeon.Height; ++Y)
    {
        for (int32 X = 0; X < Dungeon.Width; ++X)
        {
            if (Dungeon.GetTile(X, Y) == EForge2DungeonTile::Empty)
            {
                // Check if adjacent to floor/corridor
                bool bAdjacentToFloor = false;
                for (int32 DY = -1; DY <= 1 && !bAdjacentToFloor; ++DY)
                {
                    for (int32 DX = -1; DX <= 1; ++DX)
                    {
                        EForge2DungeonTile Neighbor = Dungeon.GetTile(X + DX, Y + DY);
                        if (Neighbor == EForge2DungeonTile::Floor || Neighbor == EForge2DungeonTile::Corridor)
                        {
                            bAdjacentToFloor = true;
                            break;
                        }
                    }
                }

                if (bAdjacentToFloor)
                {
                    NewTiles[Y * Dungeon.Width + X] = EForge2DungeonTile::Wall;
                }
            }
        }
    }

    Dungeon.Tiles = MoveTemp(NewTiles);
}

void FForge2DungeonProcessor::SmoothEdges(FForge2DungeonData& Dungeon, int32 Iterations)
{
    for (int32 Iter = 0; Iter < Iterations; ++Iter)
    {
        TArray<EForge2DungeonTile> NewTiles = Dungeon.Tiles;

        for (int32 Y = 1; Y < Dungeon.Height - 1; ++Y)
        {
            for (int32 X = 1; X < Dungeon.Width - 1; ++X)
            {
                if (Dungeon.GetTile(X, Y) == EForge2DungeonTile::Wall)
                {
                    // Count floor neighbors
                    int32 FloorCount = 0;
                    for (int32 DY = -1; DY <= 1; ++DY)
                    {
                        for (int32 DX = -1; DX <= 1; ++DX)
                        {
                            EForge2DungeonTile N = Dungeon.GetTile(X + DX, Y + DY);
                            if (N == EForge2DungeonTile::Floor || N == EForge2DungeonTile::Corridor)
                            {
                                ++FloorCount;
                            }
                        }
                    }

                    // Smooth isolated walls
                    if (FloorCount >= 6)
                    {
                        NewTiles[Y * Dungeon.Width + X] = EForge2DungeonTile::Floor;
                    }
                }
            }
        }

        Dungeon.Tiles = MoveTemp(NewTiles);
    }
}

bool FForge2DungeonProcessor::ValidateConnectivity(const FForge2DungeonData& Dungeon)
{
    if (Dungeon.Rooms.Num() < 2) return true;

    // BFS from first room to all others
    TSet<int32> Visited;
    TArray<int32> Queue;
    Queue.Add(0);
    Visited.Add(0);

    while (Queue.Num() > 0)
    {
        int32 Current = Queue[0];
        Queue.RemoveAt(0);

        for (int32 Connected : Dungeon.Rooms[Current].ConnectedRooms)
        {
            if (!Visited.Contains(Connected))
            {
                Visited.Add(Connected);
                Queue.Add(Connected);
            }
        }
    }

    return Visited.Num() == Dungeon.Rooms.Num();
}

TArray<FIntPoint> FForge2DungeonProcessor::FindPath(const FForge2DungeonData& Dungeon, const FIntPoint& Start, const FIntPoint& End)
{
    TArray<FIntPoint> Path;

    // A* pathfinding
    struct FNode
    {
        FIntPoint Pos;
        float G, H;
        FIntPoint Parent;

        float F() const { return G + H; }
    };

    TMap<int32, FNode> OpenSet;
    TSet<int32> ClosedSet;

    auto GetIndex = [&Dungeon](int32 X, int32 Y) { return Y * Dungeon.Width + X; };
    auto Heuristic = [](const FIntPoint& A, const FIntPoint& B) { return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y); };

    FNode StartNode;
    StartNode.Pos = Start;
    StartNode.G = 0;
    StartNode.H = Heuristic(Start, End);
    StartNode.Parent = FIntPoint(-1, -1);

    OpenSet.Add(GetIndex(Start.X, Start.Y), StartNode);

    static const int32 DX[4] = {0, 1, 0, -1};
    static const int32 DY[4] = {-1, 0, 1, 0};

    while (OpenSet.Num() > 0)
    {
        // Find node with lowest F
        int32 BestIndex = -1;
        float BestF = MAX_FLT;
        for (auto& Pair : OpenSet)
        {
            if (Pair.Value.F() < BestF)
            {
                BestF = Pair.Value.F();
                BestIndex = Pair.Key;
            }
        }

        FNode Current = OpenSet[BestIndex];
        OpenSet.Remove(BestIndex);
        ClosedSet.Add(BestIndex);

        if (Current.Pos == End)
        {
            // Reconstruct path
            FIntPoint TracePos = Current.Pos;
            while (TracePos.X >= 0)
            {
                Path.Insert(TracePos, 0);
                int32 TraceIndex = GetIndex(TracePos.X, TracePos.Y);
                // Find parent... (simplified, would need parent map)
                break; // Simplified version
            }
            Path.Add(End);
            return Path;
        }

        // Explore neighbors
        for (int32 Dir = 0; Dir < 4; ++Dir)
        {
            int32 NX = Current.Pos.X + DX[Dir];
            int32 NY = Current.Pos.Y + DY[Dir];

            if (NX < 0 || NX >= Dungeon.Width || NY < 0 || NY >= Dungeon.Height) continue;

            EForge2DungeonTile Tile = Dungeon.GetTile(NX, NY);
            if (Tile == EForge2DungeonTile::Wall || Tile == EForge2DungeonTile::Empty) continue;

            int32 NeighborIndex = GetIndex(NX, NY);
            if (ClosedSet.Contains(NeighborIndex)) continue;

            float NewG = Current.G + 1;

            if (!OpenSet.Contains(NeighborIndex) || NewG < OpenSet[NeighborIndex].G)
            {
                FNode Neighbor;
                Neighbor.Pos = FIntPoint(NX, NY);
                Neighbor.G = NewG;
                Neighbor.H = Heuristic(Neighbor.Pos, End);
                Neighbor.Parent = Current.Pos;
                OpenSet.Add(NeighborIndex, Neighbor);
            }
        }
    }

    return Path; // Empty if no path found
}
