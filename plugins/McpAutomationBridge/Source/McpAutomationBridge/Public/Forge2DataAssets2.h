// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Additional Data Asset Definitions - Part 2
// Genre-Specific, Environment, UI/Polish, and Extended Game Systems

#pragma once

#include "CoreMinimal.h"
#include "Forge2DataAssets.h"
#include "Forge2DataAssets2.generated.h"

// ============================================================================
// World Generation Data Assets
// ============================================================================

UENUM(BlueprintType)
enum class EForge2BuildingStyle : uint8
{
    Medieval    UMETA(DisplayName = "Medieval"),
    Fantasy     UMETA(DisplayName = "Fantasy"),
    Modern      UMETA(DisplayName = "Modern"),
    SciFi       UMETA(DisplayName = "Sci-Fi"),
    Ancient     UMETA(DisplayName = "Ancient"),
    Victorian   UMETA(DisplayName = "Victorian"),
    Japanese    UMETA(DisplayName = "Japanese"),
    Arabic      UMETA(DisplayName = "Arabic"),
    Industrial  UMETA(DisplayName = "Industrial"),
    Rustic      UMETA(DisplayName = "Rustic")
};

UENUM(BlueprintType)
enum class EForge2RoofType : uint8
{
    Flat        UMETA(DisplayName = "Flat"),
    Gabled      UMETA(DisplayName = "Gabled"),
    Hipped      UMETA(DisplayName = "Hipped"),
    Dome        UMETA(DisplayName = "Dome"),
    Pagoda      UMETA(DisplayName = "Pagoda"),
    Custom      UMETA(DisplayName = "Custom")
};

// Structure/Building Template Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpStructureTemplateAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpStructureTemplateAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    EForge2BuildingStyle BuildingStyle = EForge2BuildingStyle::Medieval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    EForge2RoofType RoofType = EForge2RoofType::Gabled;

    // Dimensions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimensions")
    FVector2D FootprintRange = FVector2D(500.0f, 2000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimensions")
    int32 MinFloors = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimensions")
    int32 MaxFloors = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimensions")
    float FloorHeight = 300.0f;

    // Exterior features
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior")
    float WindowDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior")
    float DoorCount = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior")
    bool bIncludeBalconies = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior")
    bool bIncludeChimneys = true;

    // Interior generation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    bool bGenerateInterior = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    int32 MinRoomsPerFloor = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    int32 MaxRoomsPerFloor = 6;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    float FurnitureDensity = 0.6f;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> WallMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> RoofMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> FloorMaterials;

    // Props
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<TSoftObjectPtr<UStaticMesh>> ExteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<TSoftObjectPtr<UStaticMesh>> InteriorFurniture;

    virtual FName GetAssetType() const override { return FName(TEXT("Structure")); }
};

// City Layout Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpCityLayoutAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpCityLayoutAsset();

    // Size
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size")
    FVector2D CityBounds = FVector2D(10000.0f, 10000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size")
    int32 PopulationTarget = 1000;

    // Districts
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Districts")
    TMap<FName, float> DistrictWeights;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Districts")
    int32 MinDistricts = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Districts")
    int32 MaxDistricts = 8;

    // Roads
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roads")
    float MainRoadWidth = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roads")
    float SideRoadWidth = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roads")
    bool bGridLayout = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roads")
    float RoadDensity = 0.5f;

    // Features
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Features")
    bool bIncludeWalls = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Features")
    int32 GateCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Features")
    bool bIncludePlazas = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Features")
    int32 LandmarkCount = 3;

    // Building templates per district
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings")
    TMap<FName, TSoftObjectPtr<UMcpStructureTemplateAsset>> DistrictBuildingTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings")
    float BuildingDensity = 0.7f;

    virtual FName GetAssetType() const override { return FName(TEXT("CityLayout")); }
};

// Terrain Preset Asset
UENUM(BlueprintType)
enum class EForge2NoiseType : uint8
{
    Perlin      UMETA(DisplayName = "Perlin"),
    Simplex     UMETA(DisplayName = "Simplex"),
    Voronoi     UMETA(DisplayName = "Voronoi"),
    FBM         UMETA(DisplayName = "Fractal Brownian Motion"),
    Ridged      UMETA(DisplayName = "Ridged"),
    Worley      UMETA(DisplayName = "Worley")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpTerrainPresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpTerrainPresetAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
    EForge2NoiseType NoiseType = EForge2NoiseType::FBM;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
    int32 Seed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
    float Scale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
    int32 Octaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
    float Persistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
    float Lacunarity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
    float Amplitude = 5000.0f;

    // Erosion
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Erosion")
    bool bApplyThermalErosion = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Erosion")
    int32 ThermalErosionIterations = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Erosion")
    bool bApplyHydraulicErosion = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Erosion")
    int32 HydraulicErosionIterations = 100;

    // Output
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
    FVector2D HeightRange = FVector2D(0.0f, 5000.0f);

    virtual FName GetAssetType() const override { return FName(TEXT("Terrain")); }
};

// Vegetation Preset Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpVegetationPresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpVegetationPresetAsset();

    // Biome association
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    TSoftObjectPtr<UMcpBiomeDefinitionAsset> AssociatedBiome;

    // Vegetation entries
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Trees;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Bushes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Grass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Flowers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Rocks;

    // Placement rules
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    float GlobalDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D SlopeRange = FVector2D(0.0f, 45.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ElevationRange = FVector2D(-1000.0f, 5000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    bool bUseNoiseMask = true;

    virtual FName GetAssetType() const override { return FName(TEXT("Vegetation")); }
};

// ============================================================================
// Extended Game Systems
// ============================================================================

// Quest Chain Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpQuestChainAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpQuestChainAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
    TArray<TSoftObjectPtr<UMcpQuestDataAsset>> Quests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
    bool bLinear = true;

    // Branching
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Branching")
    TMap<int32, TArray<int32>> BranchPaths;

    // Rewards for completing entire chain
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion")
    FForge2QuestReward ChainCompletionReward;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Completion")
    FName UnlockAchievement;

    virtual FName GetAssetType() const override { return FName(TEXT("QuestChain")); }
};

// Speaker Profile Asset (for dialogue)
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpSpeakerProfileAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpSpeakerProfileAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speaker")
    TSoftObjectPtr<UTexture2D> Portrait;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speaker")
    TArray<TSoftObjectPtr<UTexture2D>> EmotionPortraits;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speaker")
    FLinearColor NameColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> VoicePitchReference;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float VoicePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> TalkingAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> IdleAnimation;

    virtual FName GetAssetType() const override { return FName(TEXT("SpeakerProfile")); }
};

// Skill Tree Layout Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpSkillTreeAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpSkillTreeAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tree")
    TArray<TSoftObjectPtr<UMcpSkillDefinitionAsset>> Skills;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tree")
    TMap<FName, TArray<FName>> SkillConnections;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tree")
    FVector2D TreeBounds = FVector2D(1000.0f, 1500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
    FName RequiredClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
    int32 UnlockLevel = 1;

    virtual FName GetAssetType() const override { return FName(TEXT("SkillTree")); }
};

// Subclass Definition Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpSubclassDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpSubclassDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subclass")
    FName ParentClassId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subclass")
    int32 UnlockLevel = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subclass")
    FName UnlockQuestId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    TArray<FForge2StatModifier> BonusStats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
    TArray<FName> ExclusiveSkillTreeIds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
    TArray<FName> ExclusiveAbilityIds;

    virtual FName GetAssetType() const override { return FName(TEXT("Subclass")); }
};

// Affix Pool Asset (for procedural loot)
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ItemAffix
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
    FName AffixId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
    FText AffixName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
    bool bIsPrefix = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
    TArray<FForge2StatModifier> StatModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
    float Weight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
    int32 MinItemLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
    TArray<EForge2ItemType> ValidItemTypes;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpAffixPoolAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpAffixPoolAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affixes")
    TArray<FForge2ItemAffix> Prefixes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affixes")
    TArray<FForge2ItemAffix> Suffixes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
    int32 MaxPrefixes = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
    int32 MaxSuffixes = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
    TArray<FName> ExclusiveGroups;

    virtual FName GetAssetType() const override { return FName(TEXT("AffixPool")); }
};

// Shop Definition Asset
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ShopItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftObjectPtr<UMcpItemDefinitionAsset> Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Stock = -1; // -1 = unlimited

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float PriceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName RequiredReputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 RequiredReputationLevel = 0;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpShopDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpShopDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
    TArray<FForge2ShopItem> Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
    FName AcceptedCurrency = FName(TEXT("Gold"));

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
    float BuyPriceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
    float SellPriceMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
    bool bDynamicPricing = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Refresh")
    float RefreshInterval = 0.0f; // 0 = never refresh

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Refresh")
    bool bRestockOnRefresh = true;

    virtual FName GetAssetType() const override { return FName(TEXT("Shop")); }
};

// Achievement Definition Asset
UENUM(BlueprintType)
enum class EForge2AchievementType : uint8
{
    Progress    UMETA(DisplayName = "Progress Based"),
    OneTime     UMETA(DisplayName = "One Time"),
    Hidden      UMETA(DisplayName = "Hidden"),
    Timed       UMETA(DisplayName = "Timed")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpAchievementDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpAchievementDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
    EForge2AchievementType Type = EForge2AchievementType::OneTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
    int32 Points = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
    bool bSecret = false;

    // Criteria
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Criteria")
    FName CriteriaType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Criteria")
    FName TargetId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Criteria")
    int32 TargetCount = 1;

    // Rewards
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
    TArray<FForge2CurrencyAmount> CurrencyRewards;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
    TArray<TSoftObjectPtr<UMcpItemDefinitionAsset>> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
    FName TitleUnlock;

    virtual FName GetAssetType() const override { return FName(TEXT("Achievement")); }
};

// Stat Definition Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpStatDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpStatDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
    EForge2StatType StatType = EForge2StatType::Strength;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
    float BaseValue = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
    float MinValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
    float MaxValue = 9999.0f;

    // Derived stat formula
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formula")
    bool bIsDerived = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formula")
    FString Formula;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formula")
    TArray<EForge2StatType> DependsOn;

    virtual FName GetAssetType() const override { return FName(TEXT("Stat")); }
};

// Damage Type Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpDamageTypeDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpDamageTypeDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    EForge2DamageType DamageType = EForge2DamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    FLinearColor DamageColor = FLinearColor::Red;

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<FName> AppliedStatusEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float StatusEffectChance = 0.0f;

    // Resistance stat
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistance")
    EForge2StatType ResistanceStat = EForge2StatType::Armor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistance")
    float ResistanceMultiplier = 1.0f;

    // VFX
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> HitEffect;

    virtual FName GetAssetType() const override { return FName(TEXT("DamageType")); }
};

// Combat AI Profile Asset
UENUM(BlueprintType)
enum class EForge2AIAggression : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Berserk     UMETA(DisplayName = "Berserk")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpCombatProfileAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpCombatProfileAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    EForge2AIAggression Aggression = EForge2AIAggression::Neutral;

    // Targeting
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
    bool bPrioritizeLowHealth = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
    bool bPrioritizeHighThreat = true;

    // Behavior
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    float FleeHealthPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    bool bCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    float HelpCallRadius = 1000.0f;

    // Abilities
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TArray<FName> AvailableAbilities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    float AbilityCooldownMultiplier = 1.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("CombatProfile")); }
};

// ============================================================================
// Genre-Specific Data Assets
// ============================================================================

// Vehicle Definition Asset (Racing)
UENUM(BlueprintType)
enum class EForge2VehicleType : uint8
{
    Car         UMETA(DisplayName = "Car"),
    Motorcycle  UMETA(DisplayName = "Motorcycle"),
    Truck       UMETA(DisplayName = "Truck"),
    Boat        UMETA(DisplayName = "Boat"),
    Aircraft    UMETA(DisplayName = "Aircraft"),
    Hovercraft  UMETA(DisplayName = "Hovercraft")
};

UENUM(BlueprintType)
enum class EForge2DriveTrain : uint8
{
    FWD     UMETA(DisplayName = "Front Wheel Drive"),
    RWD     UMETA(DisplayName = "Rear Wheel Drive"),
    AWD     UMETA(DisplayName = "All Wheel Drive")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpVehicleDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpVehicleDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle")
    EForge2VehicleType VehicleType = EForge2VehicleType::Car;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle")
    EForge2DriveTrain DriveTrain = EForge2DriveTrain::RWD;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxSpeed = 200.0f; // km/h

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float Acceleration = 8.0f; // 0-100 km/h in seconds

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float BrakingForce = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float Handling = 0.7f;

    // Physics
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float Mass = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    FVector CenterOfMass = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float Downforce = 0.5f;

    // Tires
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tires")
    float TireGrip = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tires")
    bool bSimulateTireWear = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tires")
    bool bSimulateTireTemp = false;

    // Damage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    bool bVisualDamage = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    bool bMechanicalDamage = false;

    // Mesh
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> VehicleMesh;

    virtual FName GetAssetType() const override { return FName(TEXT("Vehicle")); }
};

// Track Definition Asset (Racing)
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpTrackDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpTrackDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
    float TrackLength = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
    int32 LapCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
    int32 CheckpointCount = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
    bool bNightRace = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
    bool bWeatherEffects = false;

    // Difficulty
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Difficulty")
    int32 DifficultyRating = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Difficulty")
    int32 MaxRacers = 12;

    // Environment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    TSoftObjectPtr<UMcpBiomeDefinitionAsset> TrackBiome;

    virtual FName GetAssetType() const override { return FName(TEXT("Track")); }
};

// Fighter Definition Asset (Fighting games)
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FighterMove
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    FName MoveId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    FText MoveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
    FString InputCommand;

    // Frame data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
    int32 StartupFrames = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
    int32 ActiveFrames = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
    int32 RecoveryFrames = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
    int32 BlockAdvantage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
    int32 HitAdvantage = 5;

    // Damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Damage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float ChipDamage = 5.0f;

    // Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    bool bOverhead = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    bool bLow = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    bool bLauncher = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    bool bProjectile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> Animation;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpFighterDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpFighterDefinitionAsset();

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Health = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float WalkSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float DashSpeed = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float JumpHeight = 1.0f;

    // Moves
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moves")
    TArray<FForge2FighterMove> NormalMoves;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moves")
    TArray<FForge2FighterMove> SpecialMoves;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moves")
    TArray<FForge2FighterMove> SuperMoves;

    // Hitbox/Hurtbox
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    FVector StandingHurtboxSize = FVector(50, 30, 180);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    FVector CrouchingHurtboxSize = FVector(50, 30, 100);

    virtual FName GetAssetType() const override { return FName(TEXT("Fighter")); }
};

// Platform Definition Asset (Platformer games)
UENUM(BlueprintType)
enum class EForge2PlatformBehavior : uint8
{
    Static      UMETA(DisplayName = "Static"),
    Moving      UMETA(DisplayName = "Moving"),
    Falling     UMETA(DisplayName = "Falling"),
    Bouncy      UMETA(DisplayName = "Bouncy"),
    Conveyor    UMETA(DisplayName = "Conveyor"),
    OneWay      UMETA(DisplayName = "One Way"),
    Crumbling   UMETA(DisplayName = "Crumbling"),
    Ice         UMETA(DisplayName = "Ice/Slippery")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpPlatformDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpPlatformDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
    EForge2PlatformBehavior Behavior = EForge2PlatformBehavior::Static;

    // Movement (for moving platforms)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    FVector MoveDistance = FVector(500, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float MoveSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float WaitTime = 1.0f;

    // Falling
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling")
    float FallDelay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling")
    float RespawnTime = 3.0f;

    // Bouncy
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bouncy")
    float BounceForce = 1000.0f;

    // Conveyor
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conveyor")
    FVector ConveyorDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conveyor")
    float ConveyorSpeed = 200.0f;

    // Ice
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ice")
    float FrictionMultiplier = 0.1f;

    // Visuals
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> PlatformMesh;

    virtual FName GetAssetType() const override { return FName(TEXT("Platform")); }
};

// Card Definition Asset (Card games)
UENUM(BlueprintType)
enum class EForge2CardType : uint8
{
    Creature    UMETA(DisplayName = "Creature"),
    Spell       UMETA(DisplayName = "Spell"),
    Artifact    UMETA(DisplayName = "Artifact"),
    Enchantment UMETA(DisplayName = "Enchantment"),
    Land        UMETA(DisplayName = "Land"),
    Hero        UMETA(DisplayName = "Hero")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpCardDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpCardDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
    EForge2CardType CardType = EForge2CardType::Creature;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
    EForge2Rarity Rarity = EForge2Rarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
    int32 ManaCost = 1;

    // Stats (for creatures)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 Attack = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 Health = 0;

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    FText EffectText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<FName> Keywords;

    // Visuals
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UTexture2D> CardArt;

    virtual FName GetAssetType() const override { return FName(TEXT("Card")); }
};

// Deck Definition Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpDeckDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpDeckDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
    int32 MinCards = 30;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
    int32 MaxCards = 60;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
    int32 MaxCopiesPerCard = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
    TArray<FName> BannedCards;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
    TArray<FName> RestrictedCards;

    virtual FName GetAssetType() const override { return FName(TEXT("DeckRules")); }
};

// Hero Definition Asset (MOBA)
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpHeroDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpHeroDefinitionAsset();

    // Base stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BaseHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BaseMana = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BaseAttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MoveSpeed = 340.0f;

    // Scaling
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float HealthPerLevel = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float ManaPerLevel = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float AttackDamagePerLevel = 3.0f;

    // Abilities
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TArray<TSoftObjectPtr<UMcpSkillDefinitionAsset>> Abilities;

    // Role
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Role")
    FName PrimaryRole;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Role")
    TArray<FName> SecondaryRoles;

    virtual FName GetAssetType() const override { return FName(TEXT("Hero")); }
};

// Unit Definition Asset (RTS)
UENUM(BlueprintType)
enum class EForge2UnitCategory : uint8
{
    Infantry    UMETA(DisplayName = "Infantry"),
    Vehicle     UMETA(DisplayName = "Vehicle"),
    Air         UMETA(DisplayName = "Air"),
    Naval       UMETA(DisplayName = "Naval"),
    Structure   UMETA(DisplayName = "Structure"),
    Hero        UMETA(DisplayName = "Hero")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpUnitDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpUnitDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit")
    EForge2UnitCategory Category = EForge2UnitCategory::Infantry;

    // Cost
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
    TMap<FName, int32> ResourceCost;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
    int32 SupplyCost = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
    float BuildTime = 10.0f;

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Armor = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MoveSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float SightRange = 1000.0f;

    // Combat
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TArray<EForge2UnitCategory> TargetableCategories;

    // Abilities
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TArray<FName> AbilityIds;

    virtual FName GetAssetType() const override { return FName(TEXT("Unit")); }
};

// Building Definition Asset (RTS)
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpRTSBuildingDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpRTSBuildingDefinitionAsset();

    // Cost
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
    TMap<FName, int32> ResourceCost;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
    float BuildTime = 30.0f;

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Health = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Armor = 2.0f;

    // Production
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<TSoftObjectPtr<UMcpUnitDefinitionAsset>> ProducibleUnits;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    int32 ProductionQueueSize = 5;

    // Research
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Research")
    TArray<FName> AvailableResearch;

    // Resources
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    TMap<FName, float> ResourceGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    int32 SupplyProvided = 0;

    // Requirements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TArray<FName> RequiredBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TArray<FName> RequiredResearch;

    virtual FName GetAssetType() const override { return FName(TEXT("RTSBuilding")); }
};

// Needs Definition Asset (Survival)
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpNeedsDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpNeedsDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need")
    float MaxValue = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need")
    float StartValue = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need")
    float DecayRate = 1.0f; // per second

    // Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float LowThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float CriticalThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float DamageThreshold = 0.0f;

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<FForge2StatModifier> LowEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<FForge2StatModifier> CriticalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float DamagePerSecond = 0.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("Need")); }
};

// ============================================================================
// Environment Data Assets
// ============================================================================

// Weather Preset Asset
UENUM(BlueprintType)
enum class EForge2WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Snow        UMETA(DisplayName = "Snow"),
    Blizzard    UMETA(DisplayName = "Blizzard"),
    Fog         UMETA(DisplayName = "Fog"),
    Sandstorm   UMETA(DisplayName = "Sandstorm")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpWeatherPresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpWeatherPresetAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    EForge2WeatherType WeatherType = EForge2WeatherType::Clear;

    // Visuals
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UNiagaraSystem> WeatherParticles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    float FogDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    float CloudCoverage = 0.0f;

    // Lighting
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting")
    float SunIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting")
    float SkyLightIntensityMultiplier = 1.0f;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float AmbientVolume = 1.0f;

    // Gameplay effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    TArray<FForge2StatModifier> PlayerModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    float VisibilityMultiplier = 1.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("Weather")); }
};

// Time of Day Preset Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpTimeOfDayPresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpTimeOfDayPresetAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
    float TimeHour = 12.0f; // 0-24

    // Sun
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sun")
    float SunAngle = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sun")
    float SunIntensity = 5.0f;

    // Sky
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.2f, 0.4f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sky")
    FLinearColor HorizonColor = FLinearColor(0.8f, 0.6f, 0.4f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sky")
    float StarVisibility = 0.0f;

    // Fog
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor::White;

    // Post process
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcess")
    float ExposureCompensation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcess")
    float BloomIntensity = 0.5f;

    virtual FName GetAssetType() const override { return FName(TEXT("TimeOfDay")); }
};

// Season Preset Asset
UENUM(BlueprintType)
enum class EForge2Season : uint8
{
    Spring  UMETA(DisplayName = "Spring"),
    Summer  UMETA(DisplayName = "Summer"),
    Autumn  UMETA(DisplayName = "Autumn"),
    Winter  UMETA(DisplayName = "Winter")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpSeasonPresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpSeasonPresetAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Season")
    EForge2Season Season = EForge2Season::Summer;

    // Temperature
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperature")
    FVector2D TemperatureRange = FVector2D(15.0f, 25.0f);

    // Weather weights
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    TMap<EForge2WeatherType, float> WeatherProbabilities;

    // Vegetation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    float FoliageDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    FLinearColor FoliageColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    bool bShowFlowers = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    bool bShowSnow = false;

    // Day length
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DayLength")
    float DaytimeHours = 12.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("Season")); }
};

// Animal Species Asset
UENUM(BlueprintType)
enum class EForge2AnimalBehavior : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Skittish    UMETA(DisplayName = "Skittish"),
    Curious     UMETA(DisplayName = "Curious")
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpAnimalSpeciesAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpAnimalSpeciesAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    EForge2AnimalBehavior DefaultBehavior = EForge2AnimalBehavior::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    bool bHerdAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    int32 HerdSize = 1;

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float FleeRange = 1500.0f;

    // Habitat
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Habitat")
    TArray<EForge2BiomeType> PreferredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Habitat")
    bool bNocturnal = false;

    // Drops
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drops")
    TSoftObjectPtr<UMcpLootTableAsset> DropTable;

    // Taming
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Taming")
    bool bTameable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Taming")
    TArray<FName> TamingFoods;

    virtual FName GetAssetType() const override { return FName(TEXT("AnimalSpecies")); }
};

// NPC Schedule Asset
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FName LocationId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FName ActivityId;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpNPCScheduleAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpNPCScheduleAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
    TArray<FForge2ScheduleEntry> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
    TMap<FName, TArray<FForge2ScheduleEntry>> SpecialSchedules;

    // Behavior
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    bool bInterruptible = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    float WanderRadius = 200.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("NPCSchedule")); }
};

// NPC Dialogue Reactions Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpNPCDialogueAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpNPCDialogueAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greetings")
    TArray<FText> Greetings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greetings")
    TArray<FText> Farewells;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
    TMap<FName, TArray<FText>> ReactionDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
    TArray<FText> IdleChatter;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
    float IdleChatterChance = 0.1f;

    virtual FName GetAssetType() const override { return FName(TEXT("NPCDialogue")); }
};

// ============================================================================
// UI/Polish Data Assets
// ============================================================================

// UI Theme Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpUIThemeAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpUIThemeAsset();

    // Colors
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colors")
    FLinearColor PrimaryColor = FLinearColor(0.2f, 0.6f, 0.9f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colors")
    FLinearColor SecondaryColor = FLinearColor(0.1f, 0.1f, 0.15f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colors")
    FLinearColor AccentColor = FLinearColor(0.9f, 0.7f, 0.2f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colors")
    FLinearColor TextColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colors")
    FLinearColor ErrorColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colors")
    FLinearColor SuccessColor = FLinearColor::Green;

    // Fonts
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fonts")
    TSoftObjectPtr<UFont> HeaderFont;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fonts")
    TSoftObjectPtr<UFont> BodyFont;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fonts")
    int32 HeaderFontSize = 24;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fonts")
    int32 BodyFontSize = 14;

    // Spacing
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spacing")
    FMargin DefaultPadding = FMargin(10.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spacing")
    float ElementSpacing = 5.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("UITheme")); }
};

// Input Preset Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpInputPresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpInputPresetAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sensitivity")
    float MouseSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sensitivity")
    float ControllerSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sensitivity")
    float AimSensitivityMultiplier = 0.5f;

    // Inversion
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inversion")
    bool bInvertMouseY = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inversion")
    bool bInvertControllerY = false;

    // Deadzones
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deadzone")
    float LeftStickDeadzone = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deadzone")
    float RightStickDeadzone = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deadzone")
    float TriggerDeadzone = 0.1f;

    // Vibration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vibration")
    bool bEnableVibration = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vibration")
    float VibrationIntensity = 1.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("InputPreset")); }
};

// Photo Mode Preset Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpPhotoModePresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpPhotoModePresetAsset();

    // Color grading
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ColorGrading")
    float Saturation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ColorGrading")
    float Contrast = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ColorGrading")
    float Brightness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ColorGrading")
    FLinearColor ColorTint = FLinearColor::White;

    // DOF
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DepthOfField")
    float FocalDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DepthOfField")
    float Aperture = 2.8f;

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float VignetteIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float FilmGrain = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float ChromaticAberration = 0.0f;

    // Frame
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frame")
    TSoftObjectPtr<UTexture2D> FrameTexture;

    virtual FName GetAssetType() const override { return FName(TEXT("PhotoModePreset")); }
};

// Accessibility Preset Asset
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpAccessibilityPresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpAccessibilityPresetAsset();

    // Colorblind modes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colorblind")
    bool bEnableColorblindMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colorblind")
    int32 ColorblindType = 0; // 0=None, 1=Protanopia, 2=Deuteranopia, 3=Tritanopia

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colorblind")
    float ColorblindStrength = 1.0f;

    // Subtitles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subtitles")
    bool bEnableSubtitles = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subtitles")
    float SubtitleScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subtitles")
    bool bSubtitleBackground = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subtitles")
    bool bShowSpeakerNames = true;

    // Audio cues
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    bool bEnableAudioCues = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    bool bEnableScreenReader = false;

    // Motor accessibility
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motor")
    bool bHoldToToggle = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motor")
    bool bAutoAim = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motor")
    float AutoAimStrength = 0.5f;

    // UI scale
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    float UIScale = 1.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("Accessibility")); }
};
