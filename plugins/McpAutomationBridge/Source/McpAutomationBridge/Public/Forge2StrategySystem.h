// Forge2StrategySystem.h
// FORGE2 Strategy/RTS Genre System
// Provides RTS mechanics: units, buildings, resources, tech trees, fog of war, formations

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Forge2StrategySystem.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EForge2UnitType : uint8
{
    Infantry,
    Vehicle,
    Aircraft,
    Naval,
    Siege,
    Support,
    Hero,
    Worker,
    Scout
};

UENUM(BlueprintType)
enum class EForge2BuildingType : uint8
{
    Base,           // Main headquarters
    Barracks,       // Infantry production
    Factory,        // Vehicle production
    Airfield,       // Aircraft production
    Dock,           // Naval production
    ResourceNode,   // Resource generation
    Defense,        // Defensive structure
    Tech,           // Research building
    Support,        // Buff/utility building
    Wall            // Defensive wall
};

UENUM(BlueprintType)
enum class EForge2ResourceType : uint8
{
    Gold,
    Wood,
    Stone,
    Food,
    Oil,
    Crystal,
    Power,
    Supply,         // Population cap
    Custom
};

UENUM(BlueprintType)
enum class EForge2FormationType : uint8
{
    None,
    Line,
    Column,
    Wedge,
    Box,
    Circle,
    Scatter,
    Custom
};

UENUM(BlueprintType)
enum class EForge2UnitStance : uint8
{
    Aggressive,     // Attack on sight
    Defensive,      // Attack if attacked
    Passive,        // Never attack
    HoldPosition,   // Don't move, attack in range
    Patrol          // Patrol between points
};

UENUM(BlueprintType)
enum class EForge2CommandType : uint8
{
    Move,
    Attack,
    AttackMove,
    Patrol,
    Hold,
    Stop,
    Garrison,
    Ungarrison,
    Repair,
    Harvest,
    Build,
    Research,
    UseAbility
};

// ============================================================================
// Resource System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ResourceCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EForge2ResourceType, int32> Costs;

    bool CanAfford(const TMap<EForge2ResourceType, int32>& Available) const;
    void Apply(TMap<EForge2ResourceType, int32>& Resources) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ResourceNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2ResourceType ResourceType = EForge2ResourceType::Gold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalResources = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RemainingResources = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GatherRate = 10;              // Per second per worker

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxWorkers = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentWorkers = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInfinite = false;

    int32 Harvest(float DeltaTime);
    bool IsDepleted() const { return !bInfinite && RemainingResources <= 0; }
};

// ============================================================================
// Unit System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2UnitDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName UnitID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UnitName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2UnitType UnitType = EForge2UnitType::Infantry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2ResourceCost Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BuildTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SupplyCost = 1;

    // Combat stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Armor = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackSpeed = 1.0f;           // Attacks per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRanged = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProjectileSpeed = 1000.0f;

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoveSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TurnRate = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanFly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanSwim = false;

    // Vision
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SightRange = 500.0f;

    // Abilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> Abilities;

    // Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredTechs;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2UnitInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName UnitTypeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OwnerPlayerID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2UnitStance Stance = EForge2UnitStance::Aggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentTargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MoveTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAttacking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackCooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName GroupID;                      // Selection group

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FormationPosition = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> AbilityCooldowns;

    void TakeDamage(float Damage, float ArmorPen = 0.0f);
    bool IsDead() const { return CurrentHealth <= 0.0f; }
};

// ============================================================================
// Building System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildingDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BuildingID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BuildingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2BuildingType BuildingType = EForge2BuildingType::Barracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2ResourceCost Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BuildTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Size = FVector(200.0f, 200.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Armor = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SightRange = 400.0f;

    // Production
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ProducibleUnits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ProductionQueueSize = 5;

    // Resource generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EForge2ResourceType, int32> ResourceGeneration;

    // Supply
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SupplyProvided = 0;

    // Defense
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanAttack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackSpeed = 0.0f;

    // Garrison
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GarrisonCapacity = 0;

    // Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredTechs;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildingInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BuildingTypeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OwnerPlayerID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BuildProgress = 1.0f;         // 0-1, 1 = complete

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConstructing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ProductionQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentProductionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RallyPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> GarrisonedUnits;

    bool IsComplete() const { return BuildProgress >= 1.0f; }
    bool IsDestroyed() const { return CurrentHealth <= 0.0f; }
};

// ============================================================================
// Tech Tree
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TechDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TechID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TechName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2ResourceCost Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ResearchTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredTechs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredBuildings;

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> UnitStatBonuses;     // UnitID -> stat multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlocksUnits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlocksBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlocksAbilities;
};

// ============================================================================
// Fog of War
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FogOfWar
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridSizeX = 256;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridSizeY = 256;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CellSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<uint8> VisibilityGrid;       // 0=unseen, 1=explored, 2=visible

    void Initialize(int32 SizeX, int32 SizeY);
    void UpdateVisibility(const TArray<FVector>& VisionSources, const TArray<float>& VisionRanges);
    uint8 GetVisibility(int32 X, int32 Y) const;
    uint8 GetVisibilityAtLocation(const FVector& Location) const;
    void RevealArea(const FVector& Center, float Radius);
};

// ============================================================================
// Formation System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Formation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName FormationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2FormationType FormationType = EForge2FormationType::Line;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Spacing = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UnitsPerRow = 5;

    TArray<FVector> CalculatePositions(int32 UnitCount, const FVector& Center, const FVector& Direction) const;
};

// ============================================================================
// Command System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Command
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2CommandType CommandType = EForge2CommandType::Move;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> SelectedUnits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetID;                     // For attack/interact

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AbilityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bQueueCommand = false;         // Shift-click
};

// ============================================================================
// Player State
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StrategyPlayerState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PlayerID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TeamID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EForge2ResourceType, int32> Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentSupply = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSupply = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ResearchedTechs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentResearch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ResearchProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2FogOfWar FogOfWar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> SelectionGroups[10];  // Ctrl+1-0 groups

    bool HasTech(FName TechID) const { return ResearchedTechs.Contains(TechID); }
    bool CanAfford(const FForge2ResourceCost& Cost) const { return Cost.CanAfford(Resources); }
    void SpendResources(const FForge2ResourceCost& Cost) { Cost.Apply(Resources); }
};

// ============================================================================
// Strategy Manager
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2StrategyManager : public UObject
{
    GENERATED_BODY()

public:
    UForge2StrategyManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2UnitDefinition> UnitDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2BuildingDefinition> BuildingDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2TechDefinition> TechDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2UnitInstance> Units;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2BuildingInstance> Buildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2ResourceNode> ResourceNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2StrategyPlayerState> Players;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentTime = 0.0f;

    // Initialization
    UFUNCTION(BlueprintCallable)
    void InitializeMatch(int32 NumPlayers);

    // Update
    UFUNCTION(BlueprintCallable)
    void UpdateSimulation(float DeltaTime);

    // Commands
    UFUNCTION(BlueprintCallable)
    void IssueCommand(const FForge2Command& Command);

    UFUNCTION(BlueprintCallable)
    bool CreateUnit(int32 PlayerID, FName UnitTypeID, const FVector& Location);

    UFUNCTION(BlueprintCallable)
    bool PlaceBuilding(int32 PlayerID, FName BuildingTypeID, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable)
    bool StartResearch(int32 PlayerID, FName TechID);

    // Queries
    UFUNCTION(BlueprintCallable)
    TArray<FName> GetUnitsInArea(const FVector& Center, float Radius, int32 PlayerID = -1) const;

    UFUNCTION(BlueprintCallable)
    bool CanBuildAt(int32 PlayerID, FName BuildingTypeID, const FVector& Location) const;

    UFUNCTION(BlueprintCallable)
    bool CanTrainUnit(int32 PlayerID, FName UnitTypeID) const;

private:
    void UpdateUnits(float DeltaTime);
    void UpdateBuildings(float DeltaTime);
    void UpdateResources(float DeltaTime);
    void UpdateResearch(float DeltaTime);
    void UpdateCombat(float DeltaTime);
    void UpdateFogOfWar();
};

// ============================================================================
// Utility Functions
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2StrategyUtilities : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static FForge2UnitDefinition CreateInfantryUnit(const FString& Name, float Health, float Damage, float Speed);

    UFUNCTION(BlueprintCallable)
    static FForge2UnitDefinition CreateVehicleUnit(const FString& Name, float Health, float Damage, float Speed);

    UFUNCTION(BlueprintCallable)
    static FForge2UnitDefinition CreateWorkerUnit(const FString& Name);

    UFUNCTION(BlueprintCallable)
    static FForge2BuildingDefinition CreateProductionBuilding(const FString& Name, EForge2BuildingType Type);

    UFUNCTION(BlueprintCallable)
    static FForge2BuildingDefinition CreateDefenseBuilding(const FString& Name, float Damage, float Range);

    UFUNCTION(BlueprintCallable)
    static FForge2TechDefinition CreateUpgradeTech(const FString& Name, FName AffectedUnit, float StatBonus);
};
