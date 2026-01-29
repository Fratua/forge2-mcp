// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Procedural Building Generation Algorithms

#pragma once

#include "CoreMinimal.h"
#include "Forge2WorldGenBuilding.generated.h"

/**
 * Architectural styles for building generation
 */
UENUM(BlueprintType)
enum class EForge2BuildingStyle : uint8
{
    Medieval UMETA(DisplayName = "Medieval"),
    Fantasy UMETA(DisplayName = "Fantasy"),
    Modern UMETA(DisplayName = "Modern"),
    SciFi UMETA(DisplayName = "Sci-Fi"),
    Ancient UMETA(DisplayName = "Ancient"),
    Victorian UMETA(DisplayName = "Victorian"),
    Japanese UMETA(DisplayName = "Japanese"),
    Arabic UMETA(DisplayName = "Arabic/Middle Eastern"),
    Industrial UMETA(DisplayName = "Industrial"),
    Gothic UMETA(DisplayName = "Gothic"),
    Colonial UMETA(DisplayName = "Colonial"),
    Art_Deco UMETA(DisplayName = "Art Deco")
};

/**
 * Roof types for buildings
 */
UENUM(BlueprintType)
enum class EForge2RoofType : uint8
{
    Flat UMETA(DisplayName = "Flat"),
    Gabled UMETA(DisplayName = "Gabled"),
    Hipped UMETA(DisplayName = "Hipped"),
    Gambrel UMETA(DisplayName = "Gambrel"),
    Mansard UMETA(DisplayName = "Mansard"),
    Dome UMETA(DisplayName = "Dome"),
    Pyramid UMETA(DisplayName = "Pyramid"),
    Shed UMETA(DisplayName = "Shed"),
    Butterfly UMETA(DisplayName = "Butterfly"),
    Sawtooth UMETA(DisplayName = "Sawtooth")
};

/**
 * Room types for interior generation
 */
UENUM(BlueprintType)
enum class EForge2RoomType : uint8
{
    Hallway UMETA(DisplayName = "Hallway"),
    Bedroom UMETA(DisplayName = "Bedroom"),
    Kitchen UMETA(DisplayName = "Kitchen"),
    LivingRoom UMETA(DisplayName = "Living Room"),
    DiningRoom UMETA(DisplayName = "Dining Room"),
    Bathroom UMETA(DisplayName = "Bathroom"),
    Storage UMETA(DisplayName = "Storage"),
    Office UMETA(DisplayName = "Office"),
    Library UMETA(DisplayName = "Library"),
    ThroneRoom UMETA(DisplayName = "Throne Room"),
    Armory UMETA(DisplayName = "Armory"),
    Treasury UMETA(DisplayName = "Treasury"),
    Chapel UMETA(DisplayName = "Chapel"),
    Tavern UMETA(DisplayName = "Tavern"),
    Workshop UMETA(DisplayName = "Workshop"),
    Cellar UMETA(DisplayName = "Cellar"),
    Tower UMETA(DisplayName = "Tower Room"),
    Balcony UMETA(DisplayName = "Balcony")
};

/**
 * Wall segment for building construction
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WallSegment
{
    GENERATED_BODY()

    /** Start point of wall */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Start;

    /** End point of wall */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector End;

    /** Wall height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float Height = 300.0f;

    /** Wall thickness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float Thickness = 20.0f;

    /** Is this an exterior wall? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bExterior = true;

    /** Floor index this wall belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 FloorIndex = 0;
};

/**
 * Window placement data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WindowData
{
    GENERATED_BODY()

    /** Center position of window */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Position;

    /** Window rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FRotator Rotation;

    /** Window size */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector2D Size = FVector2D(100.0f, 150.0f);

    /** Floor index */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 FloorIndex = 0;

    /** Style-specific window type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 WindowStyle = 0;
};

/**
 * Door placement data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DoorData
{
    GENERATED_BODY()

    /** Door position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Position;

    /** Door rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FRotator Rotation;

    /** Door size */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector2D Size = FVector2D(100.0f, 220.0f);

    /** Is this an exterior door? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bExterior = true;

    /** Is this a double door? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bDoubleDoor = false;

    /** Floor index */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 FloorIndex = 0;
};

/**
 * Interior room data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RoomData
{
    GENERATED_BODY()

    /** Room bounds (relative to building origin) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FBox Bounds;

    /** Room type/purpose */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EForge2RoomType RoomType = EForge2RoomType::LivingRoom;

    /** Floor this room is on */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 FloorIndex = 0;

    /** Connected room indices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<int32> ConnectedRooms;

    /** Walls belonging to this room */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<int32> WallIndices;
};

/**
 * Floor data for multi-story buildings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FloorData
{
    GENERATED_BODY()

    /** Floor height above ground */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float Height = 0.0f;

    /** Ceiling height for this floor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float CeilingHeight = 300.0f;

    /** Floor footprint (may differ per floor) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FVector2D> Footprint;

    /** Rooms on this floor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<int32> RoomIndices;
};

/**
 * Furniture/prop placement data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FurnitureData
{
    GENERATED_BODY()

    /** Position in world space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Position;

    /** Rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FRotator Rotation;

    /** Scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Scale = FVector(1.0f);

    /** Furniture type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FName FurnitureType;

    /** Room index this belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 RoomIndex = -1;
};

/**
 * Complete building data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildingData
{
    GENERATED_BODY()

    /** Building origin in world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Origin;

    /** Building rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FRotator Rotation;

    /** Architectural style */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EForge2BuildingStyle Style = EForge2BuildingStyle::Medieval;

    /** Roof type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EForge2RoofType RoofType = EForge2RoofType::Gabled;

    /** Base footprint vertices (2D, counter-clockwise) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FVector2D> Footprint;

    /** Floors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FForge2FloorData> Floors;

    /** All wall segments */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FForge2WallSegment> Walls;

    /** All windows */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FForge2WindowData> Windows;

    /** All doors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FForge2DoorData> Doors;

    /** All rooms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FForge2RoomData> Rooms;

    /** All furniture */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FForge2FurnitureData> Furniture;

    /** Roof vertices (for mesh generation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FVector> RoofVertices;

    /** Roof triangles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<int32> RoofTriangles;

    /** Total width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float Width = 0.0f;

    /** Total depth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float Depth = 0.0f;

    /** Total height (including roof) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float TotalHeight = 0.0f;

    /** Random seed used */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 Seed = 0;
};

/**
 * Building generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildingParams
{
    GENERATED_BODY()

    /** Architectural style */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EForge2BuildingStyle Style = EForge2BuildingStyle::Medieval;

    /** Roof type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EForge2RoofType RoofType = EForge2RoofType::Gabled;

    /** Building footprint size range (width, depth) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector2D FootprintMin = FVector2D(500.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector2D FootprintMax = FVector2D(1500.0f, 1000.0f);

    /** Number of floors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MinFloors = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MaxFloors = 3;

    /** Height per floor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "200", ClampMax = "1000"))
    float FloorHeight = 300.0f;

    /** Wall thickness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "10", ClampMax = "100"))
    float WallThickness = 20.0f;

    /** Window density (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindowDensity = 0.5f;

    /** Door count per floor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DoorCount = 1.0f;

    /** Generate interior rooms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bGenerateInterior = true;

    /** Minimum rooms per floor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MinRoomsPerFloor = 2;

    /** Maximum rooms per floor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MaxRoomsPerFloor = 6;

    /** Generate furniture */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bGenerateFurniture = true;

    /** Furniture density (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FurnitureDensity = 0.6f;

    /** Include balconies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bIncludeBalconies = false;

    /** Include chimneys */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bIncludeChimneys = true;

    /** Random seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 Seed = 0;

    /** Custom footprint (overrides size range if not empty) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FVector2D> CustomFootprint;
};

/**
 * Footprint generator for various building shapes
 */
class MCPAUTOMATIONBRIDGE_API FForge2FootprintGenerator
{
public:
    /** Generate a rectangular footprint */
    static TArray<FVector2D> GenerateRectangle(float Width, float Depth);

    /** Generate an L-shaped footprint */
    static TArray<FVector2D> GenerateLShape(float Width, float Depth, float CutWidth, float CutDepth, bool bFlipX = false, bool bFlipY = false);

    /** Generate a T-shaped footprint */
    static TArray<FVector2D> GenerateTShape(float Width, float Depth, float WingWidth, float WingDepth);

    /** Generate a U-shaped footprint */
    static TArray<FVector2D> GenerateUShape(float Width, float Depth, float CourtWidth, float CourtDepth);

    /** Generate a plus/cross footprint */
    static TArray<FVector2D> GenerateCross(float Size, float ArmWidth);

    /** Generate a circular approximation footprint */
    static TArray<FVector2D> GenerateCircle(float Radius, int32 Segments = 16);

    /** Generate an octagonal footprint */
    static TArray<FVector2D> GenerateOctagon(float Size);

    /** Generate a random complex footprint */
    static TArray<FVector2D> GenerateRandom(FRandomStream& Random, float MinSize, float MaxSize, int32 MinVertices = 4, int32 MaxVertices = 8);

    /** Calculate area of a footprint */
    static float CalculateArea(const TArray<FVector2D>& Footprint);

    /** Calculate centroid of a footprint */
    static FVector2D CalculateCentroid(const TArray<FVector2D>& Footprint);

    /** Check if footprint is convex */
    static bool IsConvex(const TArray<FVector2D>& Footprint);

    /** Simplify/clean up a footprint */
    static TArray<FVector2D> Simplify(const TArray<FVector2D>& Footprint, float Tolerance = 1.0f);
};

/**
 * Main building generator class
 */
class MCPAUTOMATIONBRIDGE_API FForge2BuildingGenerator
{
public:
    FForge2BuildingGenerator() = default;

    /**
     * Generate a complete building
     * @param Params - Generation parameters
     * @return Complete building data
     */
    FForge2BuildingData Generate(const FForge2BuildingParams& Params);

    /**
     * Generate building at specific location
     */
    FForge2BuildingData GenerateAt(const FForge2BuildingParams& Params, const FVector& Location, const FRotator& Rotation);

private:
    /** Generate footprint based on style */
    TArray<FVector2D> GenerateFootprint(const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Generate floors and walls */
    void GenerateStructure(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Generate exterior walls for a floor */
    void GenerateExteriorWalls(FForge2BuildingData& Building, int32 FloorIndex, const TArray<FVector2D>& Footprint, float FloorHeight, float CeilingHeight, float WallThickness);

    /** Generate roof */
    void GenerateRoof(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Generate windows */
    void GenerateWindows(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Generate doors */
    void GenerateDoors(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Generate interior rooms */
    void GenerateInterior(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Subdivide floor into rooms using BSP */
    void SubdivideFloor(FForge2BuildingData& Building, int32 FloorIndex, const TArray<FVector2D>& Footprint, float FloorHeight, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Assign room types based on position and adjacency */
    void AssignRoomTypes(FForge2BuildingData& Building, FRandomStream& Random);

    /** Generate interior walls */
    void GenerateInteriorWalls(FForge2BuildingData& Building, const FForge2BuildingParams& Params);

    /** Generate stairs between floors */
    void GenerateStairs(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Generate furniture for rooms */
    void GenerateFurniture(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Get style-specific parameters */
    void ApplyStyleModifiers(FForge2BuildingParams& Params, EForge2BuildingStyle Style);

    /** Calculate building bounds */
    void CalculateBounds(FForge2BuildingData& Building);
};

/**
 * Roof generator specialized for different roof types
 */
class MCPAUTOMATIONBRIDGE_API FForge2RoofGenerator
{
public:
    /** Generate flat roof */
    static void GenerateFlatRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float Overhang = 20.0f);

    /** Generate gabled roof */
    static void GenerateGabledRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float Pitch = 45.0f, float Overhang = 30.0f);

    /** Generate hipped roof */
    static void GenerateHippedRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float Pitch = 30.0f, float Overhang = 30.0f);

    /** Generate dome roof */
    static void GenerateDomeRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, int32 Segments = 12);

    /** Generate pyramid roof */
    static void GeneratePyramidRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float Overhang = 20.0f);

    /** Generate mansard roof */
    static void GenerateMansardRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float LowerPitch = 70.0f, float UpperPitch = 30.0f);

    /** Generate gambrel roof */
    static void GenerateGambrelRoof(FForge2BuildingData& Building, const TArray<FVector2D>& Footprint, float Height, float LowerPitch = 60.0f, float UpperPitch = 30.0f);

private:
    /** Add roof triangle */
    static void AddTriangle(FForge2BuildingData& Building, const FVector& V0, const FVector& V1, const FVector& V2);

    /** Add roof quad (as two triangles) */
    static void AddQuad(FForge2BuildingData& Building, const FVector& V0, const FVector& V1, const FVector& V2, const FVector& V3);

    /** Calculate footprint center and bounds */
    static void GetFootprintBounds(const TArray<FVector2D>& Footprint, FVector2D& OutCenter, FVector2D& OutMin, FVector2D& OutMax);
};

/**
 * Interior room generator
 */
class MCPAUTOMATIONBRIDGE_API FForge2InteriorGenerator
{
public:
    /**
     * Generate interior layout using BSP
     * @param Bounds - Room bounds
     * @param MinRooms - Minimum number of rooms
     * @param MaxRooms - Maximum number of rooms
     * @param Random - Random stream
     * @return Array of room bounds
     */
    static TArray<FBox> GenerateRoomLayout(const FBox& Bounds, int32 MinRooms, int32 MaxRooms, FRandomStream& Random);

    /**
     * Assign room types based on layout
     */
    static void AssignRoomTypes(TArray<FForge2RoomData>& Rooms, const FForge2BuildingParams& Params, FRandomStream& Random);

    /**
     * Get appropriate furniture for a room type
     */
    static TArray<FName> GetFurnitureForRoomType(EForge2RoomType RoomType);

    /**
     * Place furniture in a room
     */
    static TArray<FForge2FurnitureData> PlaceFurniture(const FForge2RoomData& Room, const TArray<FName>& FurnitureTypes, float Density, FRandomStream& Random);

private:
    /** BSP Node for room subdivision */
    struct FBSPNode
    {
        FBox Bounds;
        TSharedPtr<FBSPNode> Left;
        TSharedPtr<FBSPNode> Right;

        bool IsLeaf() const { return !Left.IsValid() && !Right.IsValid(); }
    };

    /** Recursively subdivide space */
    static void Subdivide(TSharedPtr<FBSPNode> Node, int32 Depth, int32 MaxDepth, float MinRoomSize, FRandomStream& Random);

    /** Collect leaf nodes as rooms */
    static void CollectRooms(TSharedPtr<FBSPNode> Node, TArray<FBox>& OutRooms);
};

/**
 * Decorator class for adding details to buildings
 */
class MCPAUTOMATIONBRIDGE_API FForge2BuildingDecorator
{
public:
    /** Add trim/molding to building */
    static void AddTrim(FForge2BuildingData& Building, EForge2BuildingStyle Style);

    /** Add shutters to windows */
    static void AddShutters(FForge2BuildingData& Building, EForge2BuildingStyle Style);

    /** Add flower boxes to windows */
    static void AddFlowerBoxes(FForge2BuildingData& Building, float Chance, FRandomStream& Random);

    /** Add balconies */
    static void AddBalconies(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Add chimneys */
    static void AddChimneys(FForge2BuildingData& Building, const FForge2BuildingParams& Params, FRandomStream& Random);

    /** Add awnings to windows/doors */
    static void AddAwnings(FForge2BuildingData& Building, EForge2BuildingStyle Style, float Chance, FRandomStream& Random);

    /** Add exterior lighting */
    static void AddExteriorLighting(FForge2BuildingData& Building, EForge2BuildingStyle Style, FRandomStream& Random);

    /** Add signage (for commercial buildings) */
    static void AddSignage(FForge2BuildingData& Building, const FString& BuildingName, FRandomStream& Random);
};
