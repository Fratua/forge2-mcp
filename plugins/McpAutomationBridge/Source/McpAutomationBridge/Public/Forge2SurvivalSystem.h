// Forge2SurvivalSystem.h
// FORGE2 Survival Genre System
// Provides survival game mechanics: needs, crafting, building, harvesting, weather, wildlife

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Forge2SurvivalSystem.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EForge2NeedType : uint8
{
    Hunger,
    Thirst,
    Sleep,
    Temperature,
    Health,
    Stamina,
    Oxygen,
    Sanity
};

UENUM(BlueprintType)
enum class EForge2ResourceCategory : uint8
{
    Wood,
    Stone,
    Metal,
    Fiber,
    Food,
    Water,
    Fuel,
    Component
};

UENUM(BlueprintType)
enum class EForge2BuildingPieceType : uint8
{
    Foundation,
    Wall,
    Doorway,
    Window,
    Floor,
    Ceiling,
    Roof,
    Ramp,
    Stairs,
    Pillar,
    Fence,
    Gate
};

UENUM(BlueprintType)
enum class EForge2WeatherType : uint8
{
    Clear,
    Cloudy,
    Rain,
    Storm,
    Snow,
    Blizzard,
    Fog,
    Heatwave,
    Sandstorm
};

UENUM(BlueprintType)
enum class EForge2AnimalBehavior : uint8
{
    Passive,
    Neutral,
    Aggressive,
    Predator,
    Prey,
    Tameable
};

// ============================================================================
// Needs System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NeedConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2NeedType NeedType = EForge2NeedType::Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxValue = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayRate = 1.0f;             // Per minute

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CriticalThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DangerThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamagePerSecond = 1.0f;       // At danger level

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> StatusEffectsAtCritical;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NeedState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentValue = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCritical = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDanger = false;

    void Update(float DeltaTime, const FForge2NeedConfig& Config);
    void Satisfy(float Amount, float MaxValue);
    float GetPercentage(float MaxValue) const { return CurrentValue / MaxValue * 100.0f; }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NeedsSystem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EForge2NeedType, FForge2NeedConfig> NeedConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EForge2NeedType, FForge2NeedState> NeedStates;

    void Initialize();
    void Update(float DeltaTime);
    void SatisfyNeed(EForge2NeedType NeedType, float Amount);
    float GetNeedValue(EForge2NeedType NeedType) const;
    bool IsNeedCritical(EForge2NeedType NeedType) const;
};

// ============================================================================
// Harvesting System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HarvestableResource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ResourceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResourceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2ResourceCategory Category = EForge2ResourceCategory::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxYield = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentYield = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HarvestTime = 1.0f;           // Seconds per hit

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 YieldPerHit = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredTool;                 // Empty = hands OK

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ToolEfficiencyMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RespawnTime = 300.0f;         // Seconds, 0 = no respawn

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RespawnTimer = 0.0f;

    int32 Harvest(bool bHasTool);
    bool CanHarvest() const { return CurrentYield > 0; }
    void UpdateRespawn(float DeltaTime);
};

// ============================================================================
// Crafting System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CraftingIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount = 1;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RecipeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName OutputItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OutputAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CraftingTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredStation;              // Empty = hand crafting

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUnlocked = false;

    bool CanCraft(const TMap<FName, int32>& Inventory, FName CurrentStation) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CraftingQueue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> QueuedRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentRecipeTime = 0.0f;

    void AddToQueue(FName RecipeID);
    void Update(float DeltaTime);
    bool IsComplete() const;
    FName GetCurrentRecipe() const;
};

// ============================================================================
// Building System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildingPiece
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PieceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PieceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2BuildingPieceType PieceType = EForge2BuildingPieceType::Wall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2CraftingIngredient> Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Size = FVector(200.0f, 200.0f, 200.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EForge2BuildingPieceType> SnapsTo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Tier = 1;                     // Wood = 1, Stone = 2, Metal = 3
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlacedStructure
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PieceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName OwnerPlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ConnectedPieces;

    void TakeDamage(float Damage);
    bool IsDestroyed() const { return CurrentHealth <= 0.0f; }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StructureDecay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDecayEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayStartTime = 86400.0f;    // Seconds before decay starts (24h)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayRate = 0.1f;             // Health per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RepairCostMultiplier = 0.5f;  // % of original cost
};

// ============================================================================
// Weather System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2WeatherType WeatherType = EForge2WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 20.0f;          // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TemperatureModifier = 0.0f;   // Applied to base temp

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Visibility = 1.0f;            // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanStartFire = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDuration = 300.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2WeatherConfig CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2WeatherConfig NextWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeRemaining = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration = 30.0f;

    void Update(float DeltaTime);
    void TransitionTo(const FForge2WeatherConfig& NewWeather);
    float GetCurrentTemperature() const;
};

// ============================================================================
// Wildlife System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AnimalDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AnimalID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AnimalName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2AnimalBehavior Behavior = EForge2AnimalBehavior::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggroRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> LootTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> FoodPreferences;      // For taming

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TameTime = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanRide = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RideSpeed = 500.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AnimalInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AnimalTypeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsTamed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName OwnerPlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TameProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAggro = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AggroTargetID;

    void TakeDamage(float Damage);
    void Feed(FName FoodItemID, const FForge2AnimalDefinition& Definition);
    bool IsDead() const { return CurrentHealth <= 0.0f; }
};

// ============================================================================
// Player State
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SurvivalPlayerState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2NeedsSystem Needs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInventorySlots = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CarryWeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxCarryWeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CraftingLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> TamedAnimals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentRidingAnimalID;

    bool AddToInventory(FName ItemID, int32 Amount);
    bool RemoveFromInventory(FName ItemID, int32 Amount);
    int32 GetItemCount(FName ItemID) const;
    bool HasItem(FName ItemID, int32 Amount = 1) const;
};

// ============================================================================
// Survival Manager
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2SurvivalManager : public UObject
{
    GENERATED_BODY()

public:
    UForge2SurvivalManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2HarvestableResource> Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2CraftingRecipe> Recipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2BuildingPiece> BuildingPieces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PlacedStructure> PlacedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2AnimalDefinition> AnimalDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2AnimalInstance> Animals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2WeatherState Weather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2StructureDecay DecayConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2SurvivalPlayerState> Players;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DayLength = 1200.0f;          // Seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentTime = 0.0f;

    // Initialization
    UFUNCTION(BlueprintCallable)
    void Initialize();

    // Update
    UFUNCTION(BlueprintCallable)
    void Update(float DeltaTime);

    // Harvesting
    UFUNCTION(BlueprintCallable)
    int32 HarvestResource(FName ResourceID, FName PlayerID, bool bHasTool);

    // Crafting
    UFUNCTION(BlueprintCallable)
    bool StartCrafting(FName RecipeID, FName PlayerID);

    UFUNCTION(BlueprintCallable)
    bool CanCraftRecipe(FName RecipeID, FName PlayerID) const;

    // Building
    UFUNCTION(BlueprintCallable)
    bool PlaceStructure(FName PieceID, FName PlayerID, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable)
    bool CanPlaceStructure(FName PieceID, const FVector& Location) const;

    UFUNCTION(BlueprintCallable)
    void RepairStructure(FName InstanceID, FName PlayerID);

    // Wildlife
    UFUNCTION(BlueprintCallable)
    void SpawnAnimal(FName AnimalTypeID, const FVector& Location);

    UFUNCTION(BlueprintCallable)
    void TameAnimal(FName AnimalInstanceID, FName PlayerID, FName FoodItemID);

    // Weather
    UFUNCTION(BlueprintCallable)
    void SetWeather(EForge2WeatherType WeatherType);

private:
    void UpdateNeeds(float DeltaTime);
    void UpdateResources(float DeltaTime);
    void UpdateStructures(float DeltaTime);
    void UpdateAnimals(float DeltaTime);
    void UpdateWeather(float DeltaTime);
};

// ============================================================================
// Utility Functions
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2SurvivalUtilities : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static FForge2NeedConfig CreateHungerNeed();

    UFUNCTION(BlueprintCallable)
    static FForge2NeedConfig CreateThirstNeed();

    UFUNCTION(BlueprintCallable)
    static FForge2NeedConfig CreateTemperatureNeed();

    UFUNCTION(BlueprintCallable)
    static FForge2HarvestableResource CreateTreeResource(const FString& Name);

    UFUNCTION(BlueprintCallable)
    static FForge2HarvestableResource CreateRockResource(const FString& Name);

    UFUNCTION(BlueprintCallable)
    static FForge2BuildingPiece CreateWallPiece(const FString& Name, int32 Tier);

    UFUNCTION(BlueprintCallable)
    static FForge2AnimalDefinition CreatePassiveAnimal(const FString& Name);

    UFUNCTION(BlueprintCallable)
    static FForge2AnimalDefinition CreatePredatorAnimal(const FString& Name);

    UFUNCTION(BlueprintCallable)
    static FForge2WeatherConfig CreateWeatherPreset(EForge2WeatherType Type);
};
