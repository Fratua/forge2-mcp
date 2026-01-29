// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Complete Data Asset Definitions for All Game Systems

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Curves/CurveFloat.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimMontage.h"
#include "NiagaraSystem.h"
#include "Forge2DataAssets.generated.h"

// ============================================================================
// Forward Declarations
// ============================================================================
class UMcpQuestDataAsset;
class UMcpDialogueTreeAsset;
class UMcpItemDefinitionAsset;
class UMcpSkillDefinitionAsset;

// ============================================================================
// Common Enumerations
// ============================================================================

UENUM(BlueprintType)
enum class EForge2Rarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Epic        UMETA(DisplayName = "Epic"),
    Legendary   UMETA(DisplayName = "Legendary"),
    Mythic      UMETA(DisplayName = "Mythic"),
    Unique      UMETA(DisplayName = "Unique")
};

UENUM(BlueprintType)
enum class EForge2DamageType : uint8
{
    Physical    UMETA(DisplayName = "Physical"),
    Fire        UMETA(DisplayName = "Fire"),
    Ice         UMETA(DisplayName = "Ice"),
    Lightning   UMETA(DisplayName = "Lightning"),
    Poison      UMETA(DisplayName = "Poison"),
    Holy        UMETA(DisplayName = "Holy"),
    Dark        UMETA(DisplayName = "Dark"),
    Arcane      UMETA(DisplayName = "Arcane"),
    Nature      UMETA(DisplayName = "Nature"),
    Psychic     UMETA(DisplayName = "Psychic"),
    True        UMETA(DisplayName = "True (Ignores Resistances)")
};

UENUM(BlueprintType)
enum class EForge2StatType : uint8
{
    Strength        UMETA(DisplayName = "Strength"),
    Dexterity       UMETA(DisplayName = "Dexterity"),
    Intelligence    UMETA(DisplayName = "Intelligence"),
    Vitality        UMETA(DisplayName = "Vitality"),
    Wisdom          UMETA(DisplayName = "Wisdom"),
    Charisma        UMETA(DisplayName = "Charisma"),
    Luck            UMETA(DisplayName = "Luck"),
    MaxHealth       UMETA(DisplayName = "Max Health"),
    MaxMana         UMETA(DisplayName = "Max Mana"),
    MaxStamina      UMETA(DisplayName = "Max Stamina"),
    HealthRegen     UMETA(DisplayName = "Health Regen"),
    ManaRegen       UMETA(DisplayName = "Mana Regen"),
    StaminaRegen    UMETA(DisplayName = "Stamina Regen"),
    Armor           UMETA(DisplayName = "Armor"),
    MagicResist     UMETA(DisplayName = "Magic Resistance"),
    CritChance      UMETA(DisplayName = "Critical Chance"),
    CritDamage      UMETA(DisplayName = "Critical Damage"),
    AttackSpeed     UMETA(DisplayName = "Attack Speed"),
    CastSpeed       UMETA(DisplayName = "Cast Speed"),
    MoveSpeed       UMETA(DisplayName = "Movement Speed"),
    CooldownReduction UMETA(DisplayName = "Cooldown Reduction"),
    LifeSteal       UMETA(DisplayName = "Life Steal"),
    ManaSteal       UMETA(DisplayName = "Mana Steal"),
    Accuracy        UMETA(DisplayName = "Accuracy"),
    Evasion         UMETA(DisplayName = "Evasion"),
    BlockChance     UMETA(DisplayName = "Block Chance"),
    BlockAmount     UMETA(DisplayName = "Block Amount")
};

UENUM(BlueprintType)
enum class EForge2ItemType : uint8
{
    Weapon          UMETA(DisplayName = "Weapon"),
    Armor           UMETA(DisplayName = "Armor"),
    Accessory       UMETA(DisplayName = "Accessory"),
    Consumable      UMETA(DisplayName = "Consumable"),
    Material        UMETA(DisplayName = "Material"),
    QuestItem       UMETA(DisplayName = "Quest Item"),
    Currency        UMETA(DisplayName = "Currency"),
    Key             UMETA(DisplayName = "Key"),
    Gem             UMETA(DisplayName = "Gem/Socket"),
    Recipe          UMETA(DisplayName = "Recipe"),
    Container       UMETA(DisplayName = "Container"),
    Misc            UMETA(DisplayName = "Miscellaneous")
};

UENUM(BlueprintType)
enum class EForge2WeaponType : uint8
{
    Sword           UMETA(DisplayName = "Sword"),
    GreatSword      UMETA(DisplayName = "Great Sword"),
    Axe             UMETA(DisplayName = "Axe"),
    GreatAxe        UMETA(DisplayName = "Great Axe"),
    Mace            UMETA(DisplayName = "Mace"),
    Hammer          UMETA(DisplayName = "Hammer"),
    Dagger          UMETA(DisplayName = "Dagger"),
    Spear           UMETA(DisplayName = "Spear"),
    Halberd         UMETA(DisplayName = "Halberd"),
    Staff           UMETA(DisplayName = "Staff"),
    Wand            UMETA(DisplayName = "Wand"),
    Bow             UMETA(DisplayName = "Bow"),
    Crossbow        UMETA(DisplayName = "Crossbow"),
    Gun             UMETA(DisplayName = "Gun"),
    Rifle           UMETA(DisplayName = "Rifle"),
    Shield          UMETA(DisplayName = "Shield"),
    Fist            UMETA(DisplayName = "Fist/Unarmed"),
    Thrown          UMETA(DisplayName = "Thrown Weapon")
};

UENUM(BlueprintType)
enum class EForge2ArmorSlot : uint8
{
    Head            UMETA(DisplayName = "Head"),
    Chest           UMETA(DisplayName = "Chest"),
    Hands           UMETA(DisplayName = "Hands"),
    Legs            UMETA(DisplayName = "Legs"),
    Feet            UMETA(DisplayName = "Feet"),
    Shoulders       UMETA(DisplayName = "Shoulders"),
    Back            UMETA(DisplayName = "Back/Cape"),
    Waist           UMETA(DisplayName = "Waist/Belt"),
    Wrist           UMETA(DisplayName = "Wrist"),
    Neck            UMETA(DisplayName = "Neck/Amulet"),
    Ring1           UMETA(DisplayName = "Ring 1"),
    Ring2           UMETA(DisplayName = "Ring 2"),
    Trinket1        UMETA(DisplayName = "Trinket 1"),
    Trinket2        UMETA(DisplayName = "Trinket 2"),
    MainHand        UMETA(DisplayName = "Main Hand"),
    OffHand         UMETA(DisplayName = "Off Hand")
};

UENUM(BlueprintType)
enum class EForge2ArmorType : uint8
{
    Cloth           UMETA(DisplayName = "Cloth"),
    Leather         UMETA(DisplayName = "Leather"),
    Mail            UMETA(DisplayName = "Mail"),
    Plate           UMETA(DisplayName = "Plate"),
    Shield          UMETA(DisplayName = "Shield"),
    None            UMETA(DisplayName = "None/Accessory")
};

UENUM(BlueprintType)
enum class EForge2BoundType : uint8
{
    None            UMETA(DisplayName = "Not Bound"),
    OnPickup        UMETA(DisplayName = "Bind on Pickup"),
    OnEquip         UMETA(DisplayName = "Bind on Equip"),
    OnUse           UMETA(DisplayName = "Bind on Use"),
    Account         UMETA(DisplayName = "Account Bound")
};

UENUM(BlueprintType)
enum class EForge2QuestCategory : uint8
{
    Main            UMETA(DisplayName = "Main Story"),
    Side            UMETA(DisplayName = "Side Quest"),
    Daily           UMETA(DisplayName = "Daily"),
    Weekly          UMETA(DisplayName = "Weekly"),
    Faction         UMETA(DisplayName = "Faction"),
    Class           UMETA(DisplayName = "Class Quest"),
    Hidden          UMETA(DisplayName = "Hidden/Secret"),
    World           UMETA(DisplayName = "World Event"),
    Dungeon         UMETA(DisplayName = "Dungeon/Raid"),
    PvP             UMETA(DisplayName = "PvP")
};

UENUM(BlueprintType)
enum class EForge2ObjectiveType : uint8
{
    Kill            UMETA(DisplayName = "Kill Enemies"),
    Collect         UMETA(DisplayName = "Collect Items"),
    Interact        UMETA(DisplayName = "Interact with Object"),
    TalkTo          UMETA(DisplayName = "Talk to NPC"),
    Escort          UMETA(DisplayName = "Escort NPC"),
    Defend          UMETA(DisplayName = "Defend Location"),
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    Survive         UMETA(DisplayName = "Survive Duration"),
    Craft           UMETA(DisplayName = "Craft Item"),
    Discover        UMETA(DisplayName = "Discover Location"),
    Photograph      UMETA(DisplayName = "Photograph Target"),
    UseAbility      UMETA(DisplayName = "Use Ability"),
    Custom          UMETA(DisplayName = "Custom Objective")
};

UENUM(BlueprintType)
enum class EForge2SkillType : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Active          UMETA(DisplayName = "Active"),
    Toggle          UMETA(DisplayName = "Toggle"),
    Aura            UMETA(DisplayName = "Aura"),
    Ultimate        UMETA(DisplayName = "Ultimate"),
    Channeled       UMETA(DisplayName = "Channeled"),
    Summon          UMETA(DisplayName = "Summon"),
    Transform       UMETA(DisplayName = "Transform")
};

UENUM(BlueprintType)
enum class EForge2TargetType : uint8
{
    Self            UMETA(DisplayName = "Self"),
    SingleEnemy     UMETA(DisplayName = "Single Enemy"),
    SingleAlly      UMETA(DisplayName = "Single Ally"),
    AllEnemies      UMETA(DisplayName = "All Enemies"),
    AllAllies       UMETA(DisplayName = "All Allies"),
    AreaEnemy       UMETA(DisplayName = "Area (Enemies)"),
    AreaAlly        UMETA(DisplayName = "Area (Allies)"),
    AreaAll         UMETA(DisplayName = "Area (All)"),
    Cone            UMETA(DisplayName = "Cone"),
    Line            UMETA(DisplayName = "Line"),
    Ground          UMETA(DisplayName = "Ground Target"),
    Projectile      UMETA(DisplayName = "Projectile")
};

UENUM(BlueprintType)
enum class EForge2BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Tundra          UMETA(DisplayName = "Tundra"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Mountain        UMETA(DisplayName = "Mountain"),
    Plains          UMETA(DisplayName = "Plains"),
    Jungle          UMETA(DisplayName = "Jungle"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Ocean           UMETA(DisplayName = "Ocean"),
    Cave            UMETA(DisplayName = "Cave/Underground"),
    Ruins           UMETA(DisplayName = "Ruins"),
    Magical         UMETA(DisplayName = "Magical/Corrupted"),
    Urban           UMETA(DisplayName = "Urban"),
    Custom          UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2StatusEffectType : uint8
{
    Buff            UMETA(DisplayName = "Buff"),
    Debuff          UMETA(DisplayName = "Debuff"),
    DamageOverTime  UMETA(DisplayName = "Damage Over Time"),
    HealOverTime    UMETA(DisplayName = "Heal Over Time"),
    CrowdControl    UMETA(DisplayName = "Crowd Control"),
    Immunity        UMETA(DisplayName = "Immunity"),
    Shield          UMETA(DisplayName = "Shield"),
    Transform       UMETA(DisplayName = "Transform")
};

UENUM(BlueprintType)
enum class EForge2CCType : uint8
{
    None            UMETA(DisplayName = "None"),
    Stun            UMETA(DisplayName = "Stun"),
    Root            UMETA(DisplayName = "Root"),
    Slow            UMETA(DisplayName = "Slow"),
    Silence         UMETA(DisplayName = "Silence"),
    Blind           UMETA(DisplayName = "Blind"),
    Fear            UMETA(DisplayName = "Fear"),
    Charm           UMETA(DisplayName = "Charm"),
    Taunt           UMETA(DisplayName = "Taunt"),
    Sleep           UMETA(DisplayName = "Sleep"),
    Knockback       UMETA(DisplayName = "Knockback"),
    Knockup         UMETA(DisplayName = "Knockup"),
    Pull            UMETA(DisplayName = "Pull"),
    Polymorph       UMETA(DisplayName = "Polymorph"),
    Freeze          UMETA(DisplayName = "Freeze"),
    Petrify         UMETA(DisplayName = "Petrify")
};

// ============================================================================
// Common Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StatModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    EForge2StatType StatType = EForge2StatType::Strength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FlatBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "-100", ClampMax = "1000"))
    float PercentBonus = 0.0f;

    FForge2StatModifier() = default;
    FForge2StatModifier(EForge2StatType InType, float InFlat, float InPercent = 0.0f)
        : StatType(InType), FlatBonus(InFlat), PercentBonus(InPercent) {}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EForge2DamageType DamageType = EForge2DamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float MinDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float MaxDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0", ClampMax = "100"))
    float CriticalChanceBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0", ClampMax = "500"))
    float CriticalDamageBonus = 0.0f;

    float GetAverageDamage() const { return (MinDamage + MaxDamage) / 2.0f; }
    float GetRandomDamage() const { return FMath::RandRange(MinDamage, MaxDamage); }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ItemDrop
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TSoftObjectPtr<UMcpItemDefinitionAsset> Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0", ClampMax = "100"))
    float DropChance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
    int32 MinQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
    int32 MaxQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    int32 MinLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    int32 MaxLevel = 0;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CurrencyAmount
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    FName CurrencyId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    int64 Amount = 0;

    FForge2CurrencyAmount() = default;
    FForge2CurrencyAmount(FName InId, int64 InAmount) : CurrencyId(InId), Amount(InAmount) {}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ReputationReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    FName FactionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    int32 Amount = 0;

    FForge2ReputationReward() = default;
    FForge2ReputationReward(FName InFaction, int32 InAmount) : FactionId(InFaction), Amount(InAmount) {}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkillRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    FName SkillId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    int32 RequiredRank = 1;
};

// ============================================================================
// Base Data Asset
// ============================================================================

UCLASS(Abstract, BlueprintType, Blueprintable)
class MCPAUTOMATIONBRIDGE_API UMcpDataAssetBase : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UMcpDataAssetBase();

    // Unique identifier for this asset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (DisplayPriority = 0))
    FName AssetId;

    // Display name (localized)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (DisplayPriority = 1))
    FText DisplayName;

    // Description (localized)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (MultiLine = true, DisplayPriority = 2))
    FText Description;

    // Gameplay tags for categorization and filtering
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
    FGameplayTagContainer Tags;

    // Icon for UI display
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UTexture2D> Icon;

    // Optional 3D preview mesh
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> PreviewMesh;

    // Version for data migration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal")
    int32 DataVersion = 1;

    // Override for primary asset ID
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const;

    // Get asset type for categorization
    UFUNCTION(BlueprintCallable, Category = "Base")
    virtual FName GetAssetType() const { return NAME_None; }

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};

// ============================================================================
// Biome Definition Asset
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VegetationEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation", meta = (ClampMin = "0", ClampMax = "1"))
    float Density = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bRandomRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bAlignToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D SlopeRange = FVector2D(0.0f, 45.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D ElevationRange = FVector2D(-10000.0f, 10000.0f);
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpBiomeDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpBiomeDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EForge2BiomeType BiomeType = EForge2BiomeType::Forest;

    // Climate parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate", meta = (ClampMin = "-50", ClampMax = "50"))
    FVector2D TemperatureRange = FVector2D(10.0f, 25.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate", meta = (ClampMin = "0", ClampMax = "100"))
    FVector2D MoistureRange = FVector2D(30.0f, 70.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate")
    FVector2D ElevationRange = FVector2D(0.0f, 2000.0f);

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> GroundDecals;

    // Vegetation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Trees;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Bushes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Grass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FForge2VegetationEntry> Rocks;

    // Props and details
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<FForge2VegetationEntry> Props;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<TSoftObjectPtr<USoundBase>> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> WindSound;

    // VFX
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> AmbientParticles;

    // Fog settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0"))
    float FogDensity = 0.02f;

    // Transition blend width
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition", meta = (ClampMin = "0"))
    float BlendWidth = 1000.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("Biome")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Item Definition Asset
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpItemDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpItemDefinitionAsset();

    // Basic properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    EForge2ItemType ItemType = EForge2ItemType::Misc;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    EForge2Rarity Rarity = EForge2Rarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0"))
    float Weight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0"))
    int64 BaseValue = 0;

    // Requirements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    int32 LevelRequirement = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TArray<FName> ClassRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TArray<FForge2StatModifier> StatRequirements;

    // Binding
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Binding")
    EForge2BoundType BoundType = EForge2BoundType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Binding")
    bool bCanBeTraded = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Binding")
    bool bCanBeDestroyed = true;

    // Visuals
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> WorldMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> EquippedMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FLinearColor RarityGlowColor = FLinearColor::White;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> PickupSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> DropSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> UseSound;

    // Set info (if part of a set)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Set")
    FName ItemSetId;

    // Sockets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (ClampMin = "0", ClampMax = "6"))
    int32 SocketCount = 0;

    // Flavor text
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lore", meta = (MultiLine = true))
    FText FlavorText;

    // Calculate sell value based on rarity and base value
    UFUNCTION(BlueprintCallable, Category = "Item")
    int64 GetSellValue() const;

    // Get rarity color
    UFUNCTION(BlueprintCallable, Category = "Item")
    FLinearColor GetRarityColor() const;

    virtual FName GetAssetType() const override { return FName(TEXT("Item")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Weapon Definition Asset
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpWeaponDefinitionAsset : public UMcpItemDefinitionAsset
{
    GENERATED_BODY()

public:
    UMcpWeaponDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    EForge2WeaponType WeaponType = EForge2WeaponType::Sword;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    bool bTwoHanded = false;

    // Damage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    FForge2DamageInfo BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    TArray<FForge2DamageInfo> BonusDamage;

    // Combat stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0"))
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0", ClampMax = "180"))
    float AttackArc = 90.0f;

    // Stat bonuses
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    TArray<FForge2StatModifier> StatBonuses;

    // Animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> HeavyAttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TArray<TSoftObjectPtr<UAnimMontage>> ComboMontages;

    // VFX
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> SwingTrail;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> HitEffect;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> SwingSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> HitSound;

    // Calculate DPS
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetDPS() const;

    virtual FName GetAssetType() const override { return FName(TEXT("Weapon")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Armor Definition Asset
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpArmorDefinitionAsset : public UMcpItemDefinitionAsset
{
    GENERATED_BODY()

public:
    UMcpArmorDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor")
    EForge2ArmorSlot Slot = EForge2ArmorSlot::Chest;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor")
    EForge2ArmorType ArmorType = EForge2ArmorType::Cloth;

    // Defense
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense", meta = (ClampMin = "0"))
    float BaseArmor = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense", meta = (ClampMin = "0"))
    float BaseMagicResist = 0.0f;

    // Resistances
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistances")
    TMap<EForge2DamageType, float> ElementalResistances;

    // Stat bonuses
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    TArray<FForge2StatModifier> StatBonuses;

    // Appearance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> OverrideMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    FName AttachSocket;

    virtual FName GetAssetType() const override { return FName(TEXT("Armor")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Consumable Definition Asset
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpConsumableDefinitionAsset : public UMcpItemDefinitionAsset
{
    GENERATED_BODY()

public:
    UMcpConsumableDefinitionAsset();

    // Effect
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    FName EffectId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    float EffectValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    float EffectDuration = 0.0f;

    // Usage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Usage", meta = (ClampMin = "0"))
    float Cooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Usage", meta = (ClampMin = "0"))
    int32 MaxCharges = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Usage")
    bool bCanUseInCombat = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Usage")
    bool bConsumedOnUse = true;

    // Cast time
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Usage", meta = (ClampMin = "0"))
    float CastTime = 0.0f;

    // Animation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> UseMontage;

    // VFX
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> UseEffect;

    virtual FName GetAssetType() const override { return FName(TEXT("Consumable")); }
};

// ============================================================================
// Quest Data Asset
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FName ObjectiveId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EForge2ObjectiveType Type = EForge2ObjectiveType::Kill;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FName TargetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (ClampMin = "1"))
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (ClampMin = "0"))
    float TargetRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TimeLimit = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bOptional = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 OrderIndex = 0;

    // Compass marker settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    bool bShowCompassMarker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    bool bShowMapMarker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    TSoftObjectPtr<UTexture2D> MarkerIcon;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ExperiencePoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FForge2CurrencyAmount> Currencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FForge2ItemDrop> Items;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FForge2ItemDrop> ChoiceItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FForge2ReputationReward> Reputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FName> AbilityUnlocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FName TitleUnlock;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpQuestDataAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpQuestDataAsset();

    // Category and sorting
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    EForge2QuestCategory Category = EForge2QuestCategory::Side;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    int32 SortOrder = 0;

    // Requirements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    int32 LevelRequirement = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TArray<FName> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TMap<FName, int32> RequiredReputation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TArray<FName> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    FName RequiredClass;

    // Behavior
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    bool bRepeatable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    float RepeatCooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    bool bAutoTrack = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    bool bShareable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    int32 MaxPartySize = 0;

    // Objectives
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
    TArray<FForge2QuestObjective> Objectives;

    // Rewards
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
    FForge2QuestReward Rewards;

    // Dialogue
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TSoftObjectPtr<UMcpDialogueTreeAsset> StartDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TSoftObjectPtr<UMcpDialogueTreeAsset> CompleteDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TSoftObjectPtr<UMcpDialogueTreeAsset> FailDialogue;

    // NPCs
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPCs")
    FName QuestGiverId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPCs")
    FName TurnInNpcId;

    // Chain
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
    FName QuestChainId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
    int32 ChainIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
    FName NextQuestId;

    // Failure
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Failure")
    bool bCanFail = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Failure")
    float TimeLimit = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Failure")
    bool bFailOnPlayerDeath = false;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetTotalObjectiveCount() const { return Objectives.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetRequiredObjectiveCount() const;

    virtual FName GetAssetType() const override { return FName(TEXT("Quest")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Dialogue Tree Asset
// ============================================================================

UENUM(BlueprintType)
enum class EForge2DialogueNodeType : uint8
{
    NPCSpeech       UMETA(DisplayName = "NPC Speech"),
    PlayerChoice    UMETA(DisplayName = "Player Choice"),
    Condition       UMETA(DisplayName = "Condition Branch"),
    Event           UMETA(DisplayName = "Trigger Event"),
    Random          UMETA(DisplayName = "Random Branch"),
    End             UMETA(DisplayName = "End Dialogue")
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FName ConditionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FName TargetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FString ComparisonOperator = TEXT(">=");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    int32 CompareValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    bool bInvert = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueConsequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequence")
    FName ConsequenceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequence")
    FName TargetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequence")
    int32 Value = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequence")
    FString StringValue;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    FName NodeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    EForge2DialogueNodeType NodeType = EForge2DialogueNodeType::NPCSpeech;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    FName SpeakerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    TSoftObjectPtr<USoundBase> VoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    TSoftObjectPtr<UAnimMontage> SpeakerAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    TArray<FForge2DialogueCondition> Conditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    TArray<FForge2DialogueConsequence> Consequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    TArray<FName> NextNodeIds;

    // Editor position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor")
    FVector2D EditorPosition = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SpeakerProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
    FName SpeakerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
    TSoftObjectPtr<UTexture2D> Portrait;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
    FLinearColor NameColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
    TSoftObjectPtr<USoundBase> VoicePitchReference;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpDialogueTreeAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpDialogueTreeAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FForge2SpeakerProfile> Speakers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FForge2DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FName StartNodeId;

    // Camera settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    bool bUseDialogueCamera = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    float CameraBlendTime = 0.5f;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> BackgroundMusic;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float MusicFadeTime = 1.0f;

    // Utility
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FForge2DialogueNode* FindNode(FName NodeId);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FForge2SpeakerProfile* FindSpeaker(FName SpeakerId);

    virtual FName GetAssetType() const override { return FName(TEXT("Dialogue")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Skill Definition Asset
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkillRank
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    int32 Rank = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    int32 PointCost = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    int32 LevelRequirement = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    float Value = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    float Cooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    float ManaCost = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    FText RankDescription;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpSkillDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpSkillDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    EForge2SkillType SkillType = EForge2SkillType::Active;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    EForge2TargetType TargetType = EForge2TargetType::Self;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    FName SkillTreeId;

    // Ranks
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranks")
    TArray<FForge2SkillRank> Ranks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranks")
    int32 MaxRank = 5;

    // Prerequisites
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisites")
    TArray<FForge2SkillRequirement> RequiredSkills;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisites")
    TArray<FForge2StatModifier> StatRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisites")
    TArray<FName> RequiredClasses;

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<FForge2DamageInfo> DamageEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<FForge2StatModifier> StatModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<FName> StatusEffectsApplied;

    // Combat
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float BaseRange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AreaRadius = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float CastTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float ChannelDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bCanCrit = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bCanBeInterrupted = true;

    // Animation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> CastAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> ChannelAnimation;

    // VFX
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> CastEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> HitEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> ProjectileEffect;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> CastSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> HitSound;

    // Tree position (for skill tree UI)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tree")
    FVector2D TreePosition = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tree")
    TArray<FName> ConnectedSkills;

    // Get rank data
    UFUNCTION(BlueprintCallable, Category = "Skill")
    FForge2SkillRank GetRankData(int32 Rank) const;

    virtual FName GetAssetType() const override { return FName(TEXT("Skill")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Status Effect Asset
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpStatusEffectAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpStatusEffectAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    EForge2StatusEffectType EffectType = EForge2StatusEffectType::Buff;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    EForge2CCType CCType = EForge2CCType::None;

    // Duration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
    float BaseDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
    bool bPermanent = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
    bool bRefreshable = true;

    // Stacking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stacking")
    bool bStackable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stacking")
    int32 MaxStacks = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stacking")
    bool bStacksIndependently = false;

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<FForge2StatModifier> StatModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float DamagePerTick = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float HealPerTick = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float TickInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    EForge2DamageType TickDamageType = EForge2DamageType::Physical;

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0", ClampMax = "100"))
    float MovementSpeedModifier = 100.0f;

    // Immunity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Immunity")
    TArray<FName> ImmuneTo;

    // Dispel
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispel")
    bool bDispellable = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispel")
    int32 DispelDifficulty = 0;

    // Visuals
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UNiagaraSystem> ActiveEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UNiagaraSystem> TickEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FLinearColor OverlayColor = FLinearColor::Transparent;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> ApplySound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> TickSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> ExpireSound;

    virtual FName GetAssetType() const override { return FName(TEXT("StatusEffect")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Loot Table Asset
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LootCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FName ConditionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    int32 MinValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    int32 MaxValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FName RequiredQuestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FName RequiredFactionId;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LootEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TSoftObjectPtr<UMcpItemDefinitionAsset> Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0"))
    float Weight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
    int32 MinQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
    int32 MaxQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TArray<FForge2LootCondition> Conditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    bool bGuaranteed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    bool bUnique = false;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpLootTableAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpLootTableAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    TArray<FForge2LootEntry> Entries;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0"))
    int32 MinDrops = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0"))
    int32 MaxDrops = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0", ClampMax = "100"))
    float NothingChance = 0.0f;

    // Currency drops
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    TArray<FForge2CurrencyAmount> GuaranteedCurrency;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    FVector2D CurrencyMultiplierRange = FVector2D(0.8f, 1.2f);

    // Level scaling
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    bool bScaleWithLevel = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    int32 BaseLevel = 1;

    // Roll loot
    UFUNCTION(BlueprintCallable, Category = "Loot")
    TArray<FForge2ItemDrop> RollLoot(int32 PlayerLevel = 1, float LuckBonus = 0.0f) const;

    virtual FName GetAssetType() const override { return FName(TEXT("LootTable")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Crafting Recipe Asset
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CraftingIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
    TSoftObjectPtr<UMcpItemDefinitionAsset> Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient", meta = (ClampMin = "1"))
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
    bool bConsumed = true;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpCraftingRecipeAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpCraftingRecipeAsset();

    // Output
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
    TSoftObjectPtr<UMcpItemDefinitionAsset> OutputItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output", meta = (ClampMin = "1"))
    int32 OutputQuantity = 1;

    // Ingredients
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredients")
    TArray<FForge2CraftingIngredient> Ingredients;

    // Requirements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    FName RequiredStation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    FName RequiredSkill;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    int32 RequiredSkillLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    int32 RequiredPlayerLevel = 0;

    // Crafting
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    float CraftingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    int32 SkillXpGain = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    bool bDiscoverable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    FName RecipeCategory;

    // Quality
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality", meta = (ClampMin = "0", ClampMax = "100"))
    float QualityVariance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality", meta = (ClampMin = "0", ClampMax = "100"))
    float CriticalCraftChance = 0.0f;

    virtual FName GetAssetType() const override { return FName(TEXT("Recipe")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Currency Definition Asset
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpCurrencyDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpCurrencyDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    FName CurrencyId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    int64 MaxAmount = 999999999;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    bool bPremium = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    bool bBound = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    bool bTradeable = true;

    // Display
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FLinearColor Color = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    int32 DisplayPriority = 0;

    // Conversion (exchange rates to base currency)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conversion")
    float ConversionRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conversion")
    FName BaseCurrencyId;

    virtual FName GetAssetType() const override { return FName(TEXT("Currency")); }
};

// ============================================================================
// Faction Definition Asset
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FactionRank
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    FName RankId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    FText RankName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    int32 RequiredReputation = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    TArray<FName> UnlockedVendorItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    TArray<FName> UnlockedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rank")
    float DiscountPercent = 0.0f;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpFactionDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpFactionDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction")
    FName FactionId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction")
    TArray<FForge2FactionRank> Ranks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction")
    int32 StartingReputation = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction")
    int32 MinReputation = -42000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction")
    int32 MaxReputation = 42000;

    // Relations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relations")
    TMap<FName, float> FactionRelations;

    // Visuals
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UTexture2D> Emblem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FLinearColor SecondaryColor = FLinearColor::Gray;

    // Get rank for reputation
    UFUNCTION(BlueprintCallable, Category = "Faction")
    FForge2FactionRank GetRankForReputation(int32 Reputation) const;

    virtual FName GetAssetType() const override { return FName(TEXT("Faction")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Class Definition Asset
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpClassDefinitionAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpClassDefinitionAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
    FName ClassId;

    // Base stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    TArray<FForge2StatModifier> BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    TArray<FForge2StatModifier> StatsPerLevel;

    // Resources
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    float BaseHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    float BaseMana = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    float HealthPerLevel = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    float ManaPerLevel = 5.0f;

    // Starting equipment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starting")
    TArray<TSoftObjectPtr<UMcpItemDefinitionAsset>> StartingItems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starting")
    TArray<FName> StartingAbilities;

    // Restrictions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restrictions")
    TArray<EForge2ArmorType> AllowedArmorTypes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restrictions")
    TArray<EForge2WeaponType> AllowedWeaponTypes;

    // Skill trees
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
    TArray<FName> SkillTreeIds;

    // Subclasses
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subclass")
    int32 SubclassUnlockLevel = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subclass")
    TArray<FName> AvailableSubclasses;

    // Visuals
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FLinearColor ClassColor = FLinearColor::White;

    virtual FName GetAssetType() const override { return FName(TEXT("Class")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};

// ============================================================================
// Dungeon Preset Asset
// ============================================================================

UENUM(BlueprintType)
enum class EForge2DungeonAlgorithm : uint8
{
    BSP                 UMETA(DisplayName = "BSP (Binary Space Partition)"),
    CellularAutomata    UMETA(DisplayName = "Cellular Automata"),
    DrunkardsWalk       UMETA(DisplayName = "Drunkard's Walk"),
    WFC                 UMETA(DisplayName = "Wave Function Collapse"),
    Maze                UMETA(DisplayName = "Maze"),
    Hybrid              UMETA(DisplayName = "Hybrid")
};

UENUM(BlueprintType)
enum class EForge2RoomPurpose : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Start       UMETA(DisplayName = "Start/Entrance"),
    Boss        UMETA(DisplayName = "Boss Room"),
    Treasure    UMETA(DisplayName = "Treasure Room"),
    Trap        UMETA(DisplayName = "Trap Room"),
    Rest        UMETA(DisplayName = "Rest/Safe Room"),
    Puzzle      UMETA(DisplayName = "Puzzle Room"),
    Shop        UMETA(DisplayName = "Shop Room"),
    Secret      UMETA(DisplayName = "Secret Room"),
    Transition  UMETA(DisplayName = "Transition/Corridor")
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RoomTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    EForge2RoomPurpose Purpose = EForge2RoomPurpose::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    FVector2D SizeRange = FVector2D(500.0f, 1500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    float SpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    int32 MinEnemies = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    int32 MaxEnemies = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    int32 MinProps = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    int32 MaxProps = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    TSoftObjectPtr<UMcpLootTableAsset> RoomLootTable;
};

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpDungeonPresetAsset : public UMcpDataAssetBase
{
    GENERATED_BODY()

public:
    UMcpDungeonPresetAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Algorithm")
    EForge2DungeonAlgorithm Algorithm = EForge2DungeonAlgorithm::BSP;

    // Size
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size")
    int32 GridWidth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size")
    int32 GridHeight = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size")
    float CellSize = 100.0f;

    // Rooms
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
    int32 MinRooms = 8;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
    int32 MaxRooms = 15;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
    FVector2D RoomSizeRange = FVector2D(400.0f, 1200.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
    TArray<FForge2RoomTemplate> RoomTemplates;

    // Corridors
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Corridors")
    float CorridorWidth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Corridors")
    bool bAllowLoops = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Corridors", meta = (ClampMin = "0", ClampMax = "100"))
    float LoopChance = 20.0f;

    // Cellular Automata specific
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CellularAutomata")
    int32 CAIterations = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CellularAutomata", meta = (ClampMin = "0", ClampMax = "100"))
    float CAFillPercent = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CellularAutomata")
    int32 CABirthLimit = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CellularAutomata")
    int32 CADeathLimit = 3;

    // Features
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Features")
    bool bIncludeSecretRooms = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Features", meta = (ClampMin = "0", ClampMax = "100"))
    float SecretRoomChance = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Features")
    bool bIncludeTraps = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Features")
    bool bIncludeDoors = true;

    // Theme
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
    TSoftObjectPtr<UMcpBiomeDefinitionAsset> DungeonBiome;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
    TArray<TSoftObjectPtr<UStaticMesh>> WallMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
    TArray<TSoftObjectPtr<UStaticMesh>> FloorMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
    TArray<TSoftObjectPtr<UStaticMesh>> PropMeshes;

    // Enemies
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemies")
    TArray<FName> EnemyPoolIds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemies")
    FName BossId;

    // Loot
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    TSoftObjectPtr<UMcpLootTableAsset> BaseLootTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    TSoftObjectPtr<UMcpLootTableAsset> BossLootTable;

    virtual FName GetAssetType() const override { return FName(TEXT("Dungeon")); }
    virtual bool ValidateAsset(TArray<FText>& OutErrors) const override;
};
