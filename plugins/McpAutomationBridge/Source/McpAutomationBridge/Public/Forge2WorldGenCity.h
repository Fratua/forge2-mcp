// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// City and Settlement Generation Algorithms

#pragma once

#include "CoreMinimal.h"
#include "Forge2WorldGenBuilding.h"
#include "Forge2WorldGenCity.generated.h"

/**
 * District types for city generation
 */
UENUM(BlueprintType)
enum class EForge2DistrictType : uint8
{
    Residential UMETA(DisplayName = "Residential"),
    Commercial UMETA(DisplayName = "Commercial"),
    Industrial UMETA(DisplayName = "Industrial"),
    Civic UMETA(DisplayName = "Civic/Government"),
    Religious UMETA(DisplayName = "Religious"),
    Military UMETA(DisplayName = "Military"),
    Academic UMETA(DisplayName = "Academic"),
    Entertainment UMETA(DisplayName = "Entertainment"),
    Harbor UMETA(DisplayName = "Harbor/Docks"),
    Agricultural UMETA(DisplayName = "Agricultural"),
    Slums UMETA(DisplayName = "Slums"),
    Noble UMETA(DisplayName = "Noble/Upper Class"),
    Market UMETA(DisplayName = "Market/Bazaar"),
    Artisan UMETA(DisplayName = "Artisan/Craftsmen")
};

/**
 * Road types
 */
UENUM(BlueprintType)
enum class EForge2RoadType : uint8
{
    MainRoad UMETA(DisplayName = "Main Road"),
    SecondaryRoad UMETA(DisplayName = "Secondary Road"),
    Alley UMETA(DisplayName = "Alley"),
    Path UMETA(DisplayName = "Path/Trail"),
    Bridge UMETA(DisplayName = "Bridge")
};

/**
 * Settlement types
 */
UENUM(BlueprintType)
enum class EForge2SettlementType : uint8
{
    Hamlet UMETA(DisplayName = "Hamlet (tiny)"),
    Village UMETA(DisplayName = "Village"),
    Town UMETA(DisplayName = "Town"),
    City UMETA(DisplayName = "City"),
    Metropolis UMETA(DisplayName = "Metropolis"),
    Fortress UMETA(DisplayName = "Fortress"),
    Castle UMETA(DisplayName = "Castle"),
    Outpost UMETA(DisplayName = "Outpost"),
    Camp UMETA(DisplayName = "Camp")
};

/**
 * Fortification types
 */
UENUM(BlueprintType)
enum class EForge2FortificationType : uint8
{
    None UMETA(DisplayName = "None"),
    Palisade UMETA(DisplayName = "Wooden Palisade"),
    StoneWall UMETA(DisplayName = "Stone Wall"),
    CastleWall UMETA(DisplayName = "Castle Wall"),
    Moat UMETA(DisplayName = "Moat"),
    Rampart UMETA(DisplayName = "Rampart")
};

/**
 * Road segment data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RoadSegment
{
    GENERATED_BODY()

    /** Start point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Start;

    /** End point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D End;

    /** Road width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Width = 400.0f;

    /** Road type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    EForge2RoadType RoadType = EForge2RoadType::SecondaryRoad;

    /** Connected road indices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<int32> ConnectedRoads;

    FVector2D GetDirection() const { return (End - Start).GetSafeNormal(); }
    float GetLength() const { return (End - Start).Size(); }
    FVector2D GetMidpoint() const { return (Start + End) * 0.5f; }
};

/**
 * Intersection data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Intersection
{
    GENERATED_BODY()

    /** Intersection position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Position;

    /** Connected road segment indices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<int32> RoadIndices;

    /** Intersection type (3-way, 4-way, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 Type = 4;
};

/**
 * District data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DistrictData
{
    GENERATED_BODY()

    /** District type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    EForge2DistrictType DistrictType = EForge2DistrictType::Residential;

    /** District bounds (polygon vertices) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FVector2D> Bounds;

    /** Center point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Center;

    /** Building lot indices in this district */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<int32> BuildingLots;

    /** Road segment indices bordering this district */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<int32> BorderRoads;

    /** District wealth level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float WealthLevel = 0.5f;

    /** District density (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Density = 0.5f;

    float GetArea() const;
};

/**
 * Building lot (plot) data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildingLot
{
    GENERATED_BODY()

    /** Lot bounds (polygon vertices) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FVector2D> Bounds;

    /** Lot center */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Center;

    /** Lot rotation (facing street) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Rotation = 0.0f;

    /** District this lot belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 DistrictIndex = -1;

    /** Road this lot faces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 FacingRoadIndex = -1;

    /** Building generated on this lot */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 BuildingIndex = -1;

    /** Is this a corner lot? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bCornerLot = false;

    float GetArea() const;
    FVector2D GetSize() const;
};

/**
 * Plaza/Open space data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlazaData
{
    GENERATED_BODY()

    /** Plaza bounds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FVector2D> Bounds;

    /** Center point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Center;

    /** Plaza name/type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FName PlazaType;

    /** Has fountain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bHasFountain = false;

    /** Has monument/statue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bHasMonument = false;

    /** Connected road indices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<int32> ConnectedRoads;
};

/**
 * Wall segment data for fortifications
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WallSection
{
    GENERATED_BODY()

    /** Start point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Start;

    /** End point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D End;

    /** Wall height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Height = 500.0f;

    /** Wall thickness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Thickness = 100.0f;

    /** Fortification type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    EForge2FortificationType FortType = EForge2FortificationType::StoneWall;

    /** Has walkway on top */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bHasWalkway = true;

    /** Has crenellations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bHasCrenellations = true;
};

/**
 * Gate data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GateData
{
    GENERATED_BODY()

    /** Gate position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Position;

    /** Gate rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Rotation = 0.0f;

    /** Gate width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Width = 400.0f;

    /** Gate name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FName GateName;

    /** Is main gate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bMainGate = false;

    /** Connected road index */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 RoadIndex = -1;
};

/**
 * Tower data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TowerData
{
    GENERATED_BODY()

    /** Tower position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Position;

    /** Tower radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Radius = 150.0f;

    /** Tower height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float Height = 800.0f;

    /** Tower type (corner, gate, watch) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FName TowerType;

    /** Wall section indices this tower connects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<int32> WallIndices;
};

/**
 * Complete city/settlement data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CityData
{
    GENERATED_BODY()

    /** City name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FString CityName;

    /** Settlement type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    EForge2SettlementType SettlementType = EForge2SettlementType::Town;

    /** City center point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D Center;

    /** City bounds (convex hull of all content) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FVector2D> Bounds;

    /** All road segments */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2RoadSegment> Roads;

    /** All intersections */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2Intersection> Intersections;

    /** All districts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2DistrictData> Districts;

    /** All building lots */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2BuildingLot> BuildingLots;

    /** All buildings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2BuildingData> Buildings;

    /** All plazas/open spaces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2PlazaData> Plazas;

    /** Wall sections (if fortified) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2WallSection> Walls;

    /** Gates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2GateData> Gates;

    /** Towers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TArray<FForge2TowerData> Towers;

    /** Population estimate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 Population = 0;

    /** Random seed used */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 Seed = 0;
};

/**
 * City generation parameters
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CityParams
{
    GENERATED_BODY()

    /** Settlement type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    EForge2SettlementType SettlementType = EForge2SettlementType::Town;

    /** City bounds size */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FVector2D CitySize = FVector2D(10000.0f, 10000.0f);

    /** Target population (affects building count) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 PopulationTarget = 1000;

    /** Number of districts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MinDistricts = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MaxDistricts = 8;

    /** District type weights */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    TMap<EForge2DistrictType, float> DistrictWeights;

    /** Main road width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float MainRoadWidth = 800.0f;

    /** Secondary road width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float SecondaryRoadWidth = 400.0f;

    /** Alley width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float AlleyWidth = 200.0f;

    /** Use grid layout (vs organic) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bGridLayout = false;

    /** Grid angle (if grid layout) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    float GridAngle = 0.0f;

    /** Road density (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RoadDensity = 0.5f;

    /** Include fortifications */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bIncludeWalls = true;

    /** Fortification type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    EForge2FortificationType FortificationType = EForge2FortificationType::StoneWall;

    /** Number of gates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City", meta = (ClampMin = "1", ClampMax = "8"))
    int32 GateCount = 4;

    /** Include plazas/markets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bIncludePlazas = true;

    /** Number of major landmarks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City", meta = (ClampMin = "0", ClampMax = "10"))
    int32 LandmarkCount = 3;

    /** Building style */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    EForge2BuildingStyle BuildingStyle = EForge2BuildingStyle::Medieval;

    /** Building density (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BuildingDensity = 0.7f;

    /** Random seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    int32 Seed = 0;

    FForge2CityParams()
    {
        // Default district weights
        DistrictWeights.Add(EForge2DistrictType::Residential, 0.4f);
        DistrictWeights.Add(EForge2DistrictType::Commercial, 0.2f);
        DistrictWeights.Add(EForge2DistrictType::Market, 0.1f);
        DistrictWeights.Add(EForge2DistrictType::Civic, 0.1f);
        DistrictWeights.Add(EForge2DistrictType::Religious, 0.1f);
        DistrictWeights.Add(EForge2DistrictType::Artisan, 0.1f);
    }
};

/**
 * L-System rule for road generation
 */
struct MCPAUTOMATIONBRIDGE_API FForge2LSystemRule
{
    FString Input;
    TArray<FString> Outputs;
    TArray<float> Weights;

    FString Apply(FRandomStream& Random) const;
};

/**
 * L-System for procedural road networks
 */
class MCPAUTOMATIONBRIDGE_API FForge2LSystemRoads
{
public:
    FForge2LSystemRoads();

    /** Set axiom (starting string) */
    void SetAxiom(const FString& Axiom);

    /** Add production rule */
    void AddRule(const FString& Input, const FString& Output, float Weight = 1.0f);

    /** Iterate L-system */
    FString Iterate(int32 Iterations, FRandomStream& Random);

    /** Interpret string as road network */
    void InterpretRoads(const FString& LString, FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

private:
    FString CurrentAxiom;
    TArray<FForge2LSystemRule> Rules;

    /** Turtle graphics state */
    struct FTurtleState
    {
        FVector2D Position;
        float Angle;
        float StepLength;
        float Width;
        EForge2RoadType RoadType;
    };
};

/**
 * Main city generator class
 */
class MCPAUTOMATIONBRIDGE_API FForge2CityGenerator
{
public:
    FForge2CityGenerator() = default;

    /**
     * Generate a complete city
     * @param Params - Generation parameters
     * @return Complete city data
     */
    FForge2CityData Generate(const FForge2CityParams& Params);

    /**
     * Generate city at specific location
     */
    FForge2CityData GenerateAt(const FForge2CityParams& Params, const FVector2D& Location);

private:
    /** Generate road network */
    void GenerateRoads(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate grid-based roads */
    void GenerateGridRoads(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate organic roads using L-system */
    void GenerateOrganicRoads(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate radial roads (for circular cities) */
    void GenerateRadialRoads(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Find and create intersections */
    void CreateIntersections(FForge2CityData& City);

    /** Generate districts using Voronoi */
    void GenerateDistricts(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Assign district types based on position */
    void AssignDistrictTypes(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate building lots within districts */
    void GenerateBuildingLots(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate buildings on lots */
    void GenerateBuildings(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate plazas and open spaces */
    void GeneratePlazas(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate fortifications */
    void GenerateFortifications(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Place gates in walls */
    void PlaceGates(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Place towers at corners and gates */
    void PlaceTowers(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate landmarks */
    void GenerateLandmarks(FForge2CityData& City, const FForge2CityParams& Params, FRandomStream& Random);

    /** Calculate city bounds */
    void CalculateBounds(FForge2CityData& City);

    /** Calculate population estimate */
    void CalculatePopulation(FForge2CityData& City);

    /** Helper: Get building params for district type */
    FForge2BuildingParams GetBuildingParamsForDistrict(EForge2DistrictType DistrictType, const FForge2CityParams& CityParams, FRandomStream& Random);
};

/**
 * Village generator (simplified settlement)
 */
class MCPAUTOMATIONBRIDGE_API FForge2VillageGenerator
{
public:
    FForge2VillageGenerator() = default;

    /**
     * Generate a village
     */
    FForge2CityData Generate(const FForge2CityParams& Params);

private:
    /** Generate village center (well, square, etc.) */
    void GenerateVillageCenter(FForge2CityData& Village, const FForge2CityParams& Params, FRandomStream& Random);

    /** Generate paths connecting buildings */
    void GeneratePaths(FForge2CityData& Village, FRandomStream& Random);

    /** Generate farm fields around village */
    void GenerateFarmFields(FForge2CityData& Village, const FForge2CityParams& Params, FRandomStream& Random);
};

/**
 * Fortification generator
 */
class MCPAUTOMATIONBRIDGE_API FForge2FortificationGenerator
{
public:
    /**
     * Generate walls around a settlement
     * @param Bounds - Settlement boundary points
     * @param FortType - Type of fortification
     * @param GatePositions - Preferred gate positions
     * @return Wall sections
     */
    static TArray<FForge2WallSection> GenerateWalls(const TArray<FVector2D>& Bounds, EForge2FortificationType FortType, const TArray<FVector2D>& GatePositions);

    /**
     * Generate towers at wall corners
     */
    static TArray<FForge2TowerData> GenerateTowers(const TArray<FForge2WallSection>& Walls, float Spacing = 500.0f);

    /**
     * Generate gates in walls
     */
    static TArray<FForge2GateData> GenerateGates(TArray<FForge2WallSection>& Walls, const TArray<FVector2D>& GatePositions, float GateWidth = 400.0f);

    /**
     * Generate a moat around walls
     */
    static TArray<FVector2D> GenerateMoat(const TArray<FForge2WallSection>& Walls, float MoatWidth = 300.0f);
};

/**
 * Road network utilities
 */
class MCPAUTOMATIONBRIDGE_API FForge2RoadUtils
{
public:
    /** Find intersection point of two road segments */
    static bool FindIntersection(const FForge2RoadSegment& A, const FForge2RoadSegment& B, FVector2D& OutPoint);

    /** Connect roads at intersection */
    static void ConnectAtIntersection(FForge2CityData& City, int32 RoadA, int32 RoadB, const FVector2D& IntersectionPoint);

    /** Split road at point */
    static int32 SplitRoad(FForge2CityData& City, int32 RoadIndex, const FVector2D& SplitPoint);

    /** Check if point is on road */
    static bool IsPointOnRoad(const FForge2RoadSegment& Road, const FVector2D& Point, float Tolerance = 10.0f);

    /** Get road polygon (for mesh generation) */
    static TArray<FVector2D> GetRoadPolygon(const FForge2RoadSegment& Road);

    /** Smooth road network (remove sharp angles) */
    static void SmoothNetwork(FForge2CityData& City, float MaxAngle = 120.0f);
};
