// Forge2CharacterSystem.h
// Phase 14: Character Systems - Creation, Classes, Mounts
// FORGE2 Ultimate AAA Game Development MCP Extension

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Forge2CharacterSystem.generated.h"

// ============================================================================
// ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2BodyType : uint8
{
    Masculine       UMETA(DisplayName = "Masculine"),
    Feminine        UMETA(DisplayName = "Feminine"),
    Androgynous     UMETA(DisplayName = "Androgynous"),
    Custom          UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2MorphCategory : uint8
{
    Body            UMETA(DisplayName = "Body"),
    Face            UMETA(DisplayName = "Face"),
    Head            UMETA(DisplayName = "Head"),
    Limbs           UMETA(DisplayName = "Limbs")
};

UENUM(BlueprintType)
enum class EForge2FaceFeature : uint8
{
    Eyes            UMETA(DisplayName = "Eyes"),
    Eyebrows        UMETA(DisplayName = "Eyebrows"),
    Nose            UMETA(DisplayName = "Nose"),
    Mouth           UMETA(DisplayName = "Mouth"),
    Ears            UMETA(DisplayName = "Ears"),
    Jaw             UMETA(DisplayName = "Jaw"),
    Cheeks          UMETA(DisplayName = "Cheeks"),
    Chin            UMETA(DisplayName = "Chin"),
    Forehead        UMETA(DisplayName = "Forehead")
};

UENUM(BlueprintType)
enum class EForge2HairType : uint8
{
    Short           UMETA(DisplayName = "Short"),
    Medium          UMETA(DisplayName = "Medium"),
    Long            UMETA(DisplayName = "Long"),
    Bald            UMETA(DisplayName = "Bald"),
    Braided         UMETA(DisplayName = "Braided"),
    Mohawk          UMETA(DisplayName = "Mohawk"),
    Ponytail        UMETA(DisplayName = "Ponytail"),
    Afro            UMETA(DisplayName = "Afro"),
    Custom          UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2FacialHairType : uint8
{
    None            UMETA(DisplayName = "None"),
    Stubble         UMETA(DisplayName = "Stubble"),
    Mustache        UMETA(DisplayName = "Mustache"),
    Goatee          UMETA(DisplayName = "Goatee"),
    FullBeard       UMETA(DisplayName = "Full Beard"),
    SideWhiskers    UMETA(DisplayName = "Side Whiskers"),
    Custom          UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2ClothingSlot : uint8
{
    Head            UMETA(DisplayName = "Head"),
    Face            UMETA(DisplayName = "Face"),
    Neck            UMETA(DisplayName = "Neck"),
    Shoulders       UMETA(DisplayName = "Shoulders"),
    Chest           UMETA(DisplayName = "Chest"),
    Back            UMETA(DisplayName = "Back"),
    Arms            UMETA(DisplayName = "Arms"),
    Hands           UMETA(DisplayName = "Hands"),
    Waist           UMETA(DisplayName = "Waist"),
    Legs            UMETA(DisplayName = "Legs"),
    Feet            UMETA(DisplayName = "Feet")
};

UENUM(BlueprintType)
enum class EForge2ClassRole : uint8
{
    Tank            UMETA(DisplayName = "Tank"),
    Healer          UMETA(DisplayName = "Healer"),
    DPS_Melee       UMETA(DisplayName = "Melee DPS"),
    DPS_Ranged      UMETA(DisplayName = "Ranged DPS"),
    Support         UMETA(DisplayName = "Support"),
    Hybrid          UMETA(DisplayName = "Hybrid")
};

UENUM(BlueprintType)
enum class EForge2ResourceType : uint8
{
    Mana            UMETA(DisplayName = "Mana"),
    Rage            UMETA(DisplayName = "Rage"),
    Energy          UMETA(DisplayName = "Energy"),
    Focus           UMETA(DisplayName = "Focus"),
    Combo           UMETA(DisplayName = "Combo Points"),
    Essence         UMETA(DisplayName = "Essence"),
    Runes           UMETA(DisplayName = "Runes"),
    Heat            UMETA(DisplayName = "Heat"),
    Custom          UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2ArmorType : uint8
{
    Cloth           UMETA(DisplayName = "Cloth"),
    Leather         UMETA(DisplayName = "Leather"),
    Mail            UMETA(DisplayName = "Mail"),
    Plate           UMETA(DisplayName = "Plate"),
    Any             UMETA(DisplayName = "Any")
};

UENUM(BlueprintType)
enum class EForge2WeaponCategory : uint8
{
    Sword           UMETA(DisplayName = "Sword"),
    Axe             UMETA(DisplayName = "Axe"),
    Mace            UMETA(DisplayName = "Mace"),
    Dagger          UMETA(DisplayName = "Dagger"),
    Polearm         UMETA(DisplayName = "Polearm"),
    Staff           UMETA(DisplayName = "Staff"),
    Bow             UMETA(DisplayName = "Bow"),
    Crossbow        UMETA(DisplayName = "Crossbow"),
    Gun             UMETA(DisplayName = "Gun"),
    Wand            UMETA(DisplayName = "Wand"),
    Shield          UMETA(DisplayName = "Shield"),
    Fist            UMETA(DisplayName = "Fist Weapon")
};

UENUM(BlueprintType)
enum class EForge2MountType : uint8
{
    Ground          UMETA(DisplayName = "Ground"),
    Flying          UMETA(DisplayName = "Flying"),
    Aquatic         UMETA(DisplayName = "Aquatic"),
    Amphibious      UMETA(DisplayName = "Amphibious"),
    Mechanical      UMETA(DisplayName = "Mechanical"),
    Ethereal        UMETA(DisplayName = "Ethereal")
};

UENUM(BlueprintType)
enum class EForge2MountRarity : uint8
{
    Common          UMETA(DisplayName = "Common"),
    Uncommon        UMETA(DisplayName = "Uncommon"),
    Rare            UMETA(DisplayName = "Rare"),
    Epic            UMETA(DisplayName = "Epic"),
    Legendary       UMETA(DisplayName = "Legendary"),
    Mythic          UMETA(DisplayName = "Mythic")
};

UENUM(BlueprintType)
enum class EForge2MountState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sprinting       UMETA(DisplayName = "Sprinting"),
    Flying          UMETA(DisplayName = "Flying"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Jumping         UMETA(DisplayName = "Jumping"),
    Combat          UMETA(DisplayName = "Combat")
};

// ============================================================================
// STRUCTS - CHARACTER CREATION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MorphTarget
{
    GENERATED_BODY()

    // Morph target name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph")
    FName MorphName;

    // Category
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph")
    EForge2MorphCategory Category = EForge2MorphCategory::Body;

    // Current value (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph", meta = (ClampMin = "0", ClampMax = "1"))
    float Value = 0.5f;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph")
    FText DisplayName;

    // Tooltip
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph")
    FText Tooltip;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BodyMorphSettings
{
    GENERATED_BODY()

    // Height (0=shortest, 1=tallest)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float Height = 0.5f;

    // Muscle mass
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float MuscleMass = 0.5f;

    // Body fat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float BodyFat = 0.3f;

    // Shoulder width
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float ShoulderWidth = 0.5f;

    // Hip width
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float HipWidth = 0.5f;

    // Chest size
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float ChestSize = 0.5f;

    // Waist size
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float WaistSize = 0.5f;

    // Arm length
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float ArmLength = 0.5f;

    // Leg length
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0", ClampMax = "1"))
    float LegLength = 0.5f;

    // Body type preset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    EForge2BodyType BodyType = EForge2BodyType::Custom;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FaceFeatureSetting
{
    GENERATED_BODY()

    // Feature type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    EForge2FaceFeature Feature = EForge2FaceFeature::Eyes;

    // Size
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float Size = 0.5f;

    // Width
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float Width = 0.5f;

    // Height
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float Height = 0.5f;

    // Vertical position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float VerticalPosition = 0.5f;

    // Horizontal position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float HorizontalPosition = 0.5f;

    // Depth
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float Depth = 0.5f;

    // Rotation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "-1", ClampMax = "1"))
    float Rotation = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FaceSettings
{
    GENERATED_BODY()

    // Face shape preset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    int32 FaceShapePreset = 0;

    // Feature settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    TArray<FForge2FaceFeatureSetting> Features;

    // Skin texture index
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    int32 SkinTextureIndex = 0;

    // Age wrinkles (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float AgeWrinkles = 0.0f;

    // Freckles intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float Freckles = 0.0f;

    // Moles/beauty marks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    TArray<FVector2D> MolePositions;

    // Makeup settings (if applicable)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    FLinearColor LipColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    FLinearColor EyeShadowColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0", ClampMax = "1"))
    float MakeupIntensity = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HairSettings
{
    GENERATED_BODY()

    // Hair type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    EForge2HairType HairType = EForge2HairType::Short;

    // Hair style index
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    int32 HairStyleIndex = 0;

    // Hair color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    FLinearColor HairColor = FLinearColor(0.2f, 0.15f, 0.1f);

    // Secondary color (highlights/tips)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    FLinearColor SecondaryColor = FLinearColor(0.3f, 0.25f, 0.2f);

    // Highlight intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair", meta = (ClampMin = "0", ClampMax = "1"))
    float HighlightIntensity = 0.0f;

    // Greying amount
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair", meta = (ClampMin = "0", ClampMax = "1"))
    float GreyingAmount = 0.0f;

    // Shininess
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair", meta = (ClampMin = "0", ClampMax = "1"))
    float Shininess = 0.5f;

    // Physics enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    bool bEnablePhysics = true;

    // Facial hair type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    EForge2FacialHairType FacialHairType = EForge2FacialHairType::None;

    // Facial hair style index
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    int32 FacialHairStyleIndex = 0;

    // Facial hair color (if different)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    FLinearColor FacialHairColor = FLinearColor(0.2f, 0.15f, 0.1f);

    // Eyebrow style
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    int32 EyebrowStyleIndex = 0;

    // Eyebrow color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    FLinearColor EyebrowColor = FLinearColor(0.2f, 0.15f, 0.1f);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkinSettings
{
    GENERATED_BODY()

    // Skin tone (0-1 from light to dark)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin", meta = (ClampMin = "0", ClampMax = "1"))
    float SkinTone = 0.5f;

    // Skin color hue adjustment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    FLinearColor SkinColor = FLinearColor(0.9f, 0.75f, 0.65f);

    // Subsurface scattering intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin", meta = (ClampMin = "0", ClampMax = "1"))
    float SubsurfaceIntensity = 0.5f;

    // Roughness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin", meta = (ClampMin = "0", ClampMax = "1"))
    float Roughness = 0.5f;

    // Oiliness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin", meta = (ClampMin = "0", ClampMax = "1"))
    float Oiliness = 0.3f;

    // Scar locations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    TArray<FName> ScarIds;

    // Tattoo data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    TArray<FName> TattooIds;

    // Body paint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    FName BodyPaintId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    FLinearColor BodyPaintColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ClothingItem
{
    GENERATED_BODY()

    // Item ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FName ItemId;

    // Slot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EForge2ClothingSlot Slot = EForge2ClothingSlot::Chest;

    // Mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Mesh;

    // Primary color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor PrimaryColor = FLinearColor::White;

    // Secondary color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor SecondaryColor = FLinearColor::Gray;

    // Tertiary color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor TertiaryColor = FLinearColor::Black;

    // Material override
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInstance> MaterialOverride;

    // Hidden body parts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FName> HiddenBodyParts;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CharacterAppearance
{
    GENERATED_BODY()

    // Character name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    // Body morphs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FForge2BodyMorphSettings BodyMorphs;

    // Face settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FForge2FaceSettings FaceSettings;

    // Hair settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FForge2HairSettings HairSettings;

    // Skin settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FForge2SkinSettings SkinSettings;

    // Eye color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.2f, 0.4f, 0.6f);

    // Heterochromia (different eye colors)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHeterochromia = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SecondEyeColor = FLinearColor(0.2f, 0.4f, 0.6f);

    // Equipped clothing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FForge2ClothingItem> Clothing;

    // Accessories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FName> AccessoryIds;

    // Voice set ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FName VoiceSetId;

    // Voice pitch modifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float VoicePitch = 1.0f;
};

// ============================================================================
// STRUCTS - CLASS SYSTEM
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StatBonus
{
    GENERATED_BODY()

    // Stat name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FName StatName;

    // Flat bonus
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FlatBonus = 0.0f;

    // Percentage bonus
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PercentBonus = 0.0f;

    // Per level bonus
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PerLevelBonus = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ClassDefinition
{
    GENERATED_BODY()

    // Class ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    FName ClassId;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    FText DisplayName;

    // Description
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    FText Description;

    // Icon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TSoftObjectPtr<UTexture2D> Icon;

    // Primary role
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    EForge2ClassRole PrimaryRole = EForge2ClassRole::DPS_Melee;

    // Secondary roles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TArray<EForge2ClassRole> SecondaryRoles;

    // Resource type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    EForge2ResourceType ResourceType = EForge2ResourceType::Mana;

    // Base resource amount
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    float BaseResource = 100.0f;

    // Resource per level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    float ResourcePerLevel = 5.0f;

    // Armor proficiency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TArray<EForge2ArmorType> ArmorProficiency;

    // Weapon proficiency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TArray<EForge2WeaponCategory> WeaponProficiency;

    // Base stat bonuses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TArray<FForge2StatBonus> BaseStats;

    // Starting abilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TArray<FName> StartingAbilities;

    // Available subclasses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TArray<FName> AvailableSubclasses;

    // Unlock level for subclass
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    int32 SubclassUnlockLevel = 10;

    // Can multiclass
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    bool bCanMulticlass = true;

    // Multiclass requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TMap<FName, int32> MulticlassRequirements; // Stat name -> minimum value
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SubclassDefinition
{
    GENERATED_BODY()

    // Subclass ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    FName SubclassId;

    // Parent class ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    FName ParentClassId;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    FText DisplayName;

    // Description
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    FText Description;

    // Icon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    TSoftObjectPtr<UTexture2D> Icon;

    // Role modification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    EForge2ClassRole RoleModifier = EForge2ClassRole::Hybrid;

    // Additional stat bonuses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    TArray<FForge2StatBonus> StatBonuses;

    // Unique abilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    TArray<FName> UniqueAbilities;

    // Passive bonuses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    TArray<FName> PassiveEffects;

    // Required quests to unlock
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    TArray<FName> RequiredQuests;

    // Alternative resource type (if different)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    EForge2ResourceType AlternateResource = EForge2ResourceType::Mana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subclass")
    bool bUseAlternateResource = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MulticlassConfig
{
    GENERATED_BODY()

    // Primary class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiclass")
    FName PrimaryClassId;

    // Secondary class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiclass")
    FName SecondaryClassId;

    // Level split (primary class level)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiclass")
    int32 PrimaryClassLevel = 0;

    // Secondary class level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiclass")
    int32 SecondaryClassLevel = 0;

    // Active class (for ability bar)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiclass")
    FName ActiveClassId;

    // Shared abilities (accessible from both)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiclass")
    TArray<FName> SharedAbilities;
};

// ============================================================================
// STRUCTS - MOUNT SYSTEM
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MountAbility
{
    GENERATED_BODY()

    // Ability ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FName AbilityId;

    // Ability name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FText DisplayName;

    // Cooldown
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float Cooldown = 10.0f;

    // Resource cost
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float ResourceCost = 0.0f;

    // Is passive
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    bool bIsPassive = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MountStats
{
    GENERATED_BODY()

    // Base speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float BaseSpeed = 600.0f;

    // Sprint speed multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float SprintMultiplier = 1.5f;

    // Jump height
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float JumpHeight = 400.0f;

    // Can fly
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    bool bCanFly = false;

    // Fly speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float FlySpeed = 800.0f;

    // Max altitude
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float MaxAltitude = 5000.0f;

    // Can swim
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    bool bCanSwim = false;

    // Swim speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float SwimSpeed = 400.0f;

    // Stamina pool
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float MaxStamina = 100.0f;

    // Stamina regen per second
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float StaminaRegen = 10.0f;

    // Sprint stamina cost per second
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float SprintStaminaCost = 15.0f;

    // Health pool
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float MaxHealth = 500.0f;

    // Armor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float Armor = 100.0f;

    // Carry capacity bonus
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float CarryCapacityBonus = 100.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MountDefinition
{
    GENERATED_BODY()

    // Mount ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FName MountId;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FText DisplayName;

    // Description
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FText Description;

    // Mount type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    EForge2MountType MountType = EForge2MountType::Ground;

    // Rarity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    EForge2MountRarity Rarity = EForge2MountRarity::Common;

    // Icon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    TSoftObjectPtr<UTexture2D> Icon;

    // Mount blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    TSoftClassPtr<AActor> MountClass;

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FForge2MountStats Stats;

    // Abilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    TArray<FForge2MountAbility> Abilities;

    // Required level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    int32 RequiredLevel = 1;

    // Riding skill required
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FName RequiredSkill;

    // Summon time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float SummonTime = 2.0f;

    // Dismissable in combat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    bool bDismissableInCombat = false;

    // Passenger count
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    int32 PassengerCount = 0;

    // Has inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    bool bHasInventory = false;

    // Inventory slots
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    int32 InventorySlots = 0;

    // Can attack while mounted
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    bool bCanAttackWhileMounted = true;

    // Mount-specific attacks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    TArray<FName> MountAttacks;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MountCustomization
{
    GENERATED_BODY()

    // Armor set ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FName ArmorSetId;

    // Saddle ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FName SaddleId;

    // Barding ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FName BardingId;

    // Primary color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FLinearColor PrimaryColor = FLinearColor::White;

    // Secondary color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FLinearColor SecondaryColor = FLinearColor::Gray;

    // Accent color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FLinearColor AccentColor = FLinearColor(0.8f, 0.6f, 0.2f);

    // Emblem/decal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TSoftObjectPtr<UTexture2D> Emblem;

    // Accessories (horns, wings, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TArray<FName> AccessoryIds;

    // Trail effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TSoftObjectPtr<UNiagaraSystem> TrailEffect;

    // Aura effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TSoftObjectPtr<UNiagaraSystem> AuraEffect;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2OwnedMount
{
    GENERATED_BODY()

    // Instance ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FGuid InstanceId;

    // Mount definition ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FName MountId;

    // Custom name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FString CustomName;

    // Customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    FForge2MountCustomization Customization;

    // Experience
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    int32 Experience = 0;

    // Level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    int32 Level = 1;

    // Bond level with player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount", meta = (ClampMin = "0", ClampMax = "100"))
    float BondLevel = 0.0f;

    // Is favorite
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    bool bIsFavorite = false;

    // Times summoned
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    int32 TimesSummoned = 0;

    // Distance traveled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
    float DistanceTraveled = 0.0f;
};

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAppearanceChanged, FName, PropertyName, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClothingChanged, EForge2ClothingSlot, Slot, FName, ItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClassChanged, FName, OldClassId, FName, NewClassId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubclassUnlocked, FName, SubclassId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMountSummoned, FGuid, InstanceId, FName, MountId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMountDismissed, FGuid, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMountLevelUp, FGuid, InstanceId, int32, NewLevel);

// ============================================================================
// CHARACTER CREATOR MANAGER
// ============================================================================

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2CharacterCreatorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2CharacterCreatorManager();

    // ========== Body Morphs ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetBodyMorphSettings(const FForge2BodyMorphSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetBodyMorphValue(FName MorphName, float Value);

    UFUNCTION(BlueprintPure, Category = "FORGE2|CharacterCreator")
    float GetBodyMorphValue(FName MorphName) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void ApplyBodyPreset(EForge2BodyType BodyType);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void RandomizeBodyMorphs();

    // ========== Face Customization ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetFaceSettings(const FForge2FaceSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetFaceFeature(EForge2FaceFeature Feature, const FForge2FaceFeatureSetting& Setting);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void ApplyFacePreset(int32 PresetIndex);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void RandomizeFace();

    // ========== Hair System ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetHairSettings(const FForge2HairSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetHairStyle(EForge2HairType Type, int32 StyleIndex);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetHairColor(const FLinearColor& Color);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetFacialHair(EForge2FacialHairType Type, int32 StyleIndex);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    TArray<int32> GetAvailableHairStyles(EForge2HairType Type) const;

    // ========== Skin System ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetSkinSettings(const FForge2SkinSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetSkinTone(float Tone);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void AddScar(FName ScarId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void RemoveScar(FName ScarId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void AddTattoo(FName TattooId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void RemoveTattoo(FName TattooId);

    // ========== Clothing System ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void EquipClothing(const FForge2ClothingItem& Item);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void UnequipClothing(EForge2ClothingSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetClothingColor(EForge2ClothingSlot Slot, const FLinearColor& Primary, const FLinearColor& Secondary);

    UFUNCTION(BlueprintPure, Category = "FORGE2|CharacterCreator")
    FForge2ClothingItem GetEquippedClothing(EForge2ClothingSlot Slot) const;

    // ========== Full Appearance ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SetFullAppearance(const FForge2CharacterAppearance& Appearance);

    UFUNCTION(BlueprintPure, Category = "FORGE2|CharacterCreator")
    FForge2CharacterAppearance GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void SaveAppearancePreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    bool LoadAppearancePreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|CharacterCreator")
    void RandomizeFullAppearance();

    // ========== Events ==========

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|CharacterCreator|Events")
    FOnAppearanceChanged OnAppearanceChanged;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|CharacterCreator|Events")
    FOnClothingChanged OnClothingChanged;

protected:
    void ApplyMorphTargets();
    void UpdateMeshComponents();

private:
    FForge2CharacterAppearance CurrentAppearance;
    TMap<FName, float> ActiveMorphTargets;
    TMap<FString, FForge2CharacterAppearance> SavedPresets;
};

// ============================================================================
// CLASS SYSTEM MANAGER
// ============================================================================

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2ClassSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2ClassSystemManager();

    // ========== Class Registration ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    void RegisterClass(const FForge2ClassDefinition& ClassDef);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    void RegisterSubclass(const FForge2SubclassDefinition& SubclassDef);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    bool GetClassDefinition(FName ClassId, FForge2ClassDefinition& OutDef) const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    bool GetSubclassDefinition(FName SubclassId, FForge2SubclassDefinition& OutDef) const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    TArray<FName> GetAllClassIds() const;

    // ========== Player Class Management ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    bool SetPlayerClass(FName ClassId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    bool SetPlayerSubclass(FName SubclassId);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    FName GetPlayerClassId() const { return CurrentClassId; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    FName GetPlayerSubclassId() const { return CurrentSubclassId; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    int32 GetPlayerClassLevel() const { return CurrentClassLevel; }

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    void AddClassExperience(int32 Amount);

    // ========== Multiclass ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    bool EnableMulticlass(FName SecondaryClassId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    void DisableMulticlass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    bool IsMulticlassed() const { return bIsMulticlassed; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    FForge2MulticlassConfig GetMulticlassConfig() const { return MulticlassConfig; }

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    void SwitchActiveClass();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Class")
    bool CanMulticlassAs(FName ClassId) const;

    // ========== Stat Calculations ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    float GetStatBonus(FName StatName) const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    TArray<FForge2StatBonus> GetAllStatBonuses() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    EForge2ResourceType GetResourceType() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    float GetMaxResource() const;

    // ========== Abilities ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    TArray<FName> GetClassAbilities() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    TArray<FName> GetSubclassAbilities() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Class")
    TArray<FName> GetAllAvailableAbilities() const;

    // ========== Events ==========

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Class|Events")
    FOnClassChanged OnClassChanged;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Class|Events")
    FOnSubclassUnlocked OnSubclassUnlocked;

protected:
    void RecalculateStats();
    void ApplyClassBonuses();

private:
    TMap<FName, FForge2ClassDefinition> RegisteredClasses;
    TMap<FName, FForge2SubclassDefinition> RegisteredSubclasses;

    FName CurrentClassId;
    FName CurrentSubclassId;
    int32 CurrentClassLevel = 1;
    int32 CurrentClassXP = 0;

    bool bIsMulticlassed = false;
    FForge2MulticlassConfig MulticlassConfig;

    TMap<FName, float> CalculatedStatBonuses;
};

// ============================================================================
// MOUNT SYSTEM MANAGER
// ============================================================================

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2MountSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2MountSystemManager();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== Mount Registration ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void RegisterMountDefinition(const FForge2MountDefinition& MountDef);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    bool GetMountDefinition(FName MountId, FForge2MountDefinition& OutDef) const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    TArray<FName> GetAllMountIds() const;

    // ========== Owned Mounts ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    FGuid AddOwnedMount(FName MountId, const FString& CustomName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void RemoveOwnedMount(FGuid InstanceId);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    TArray<FForge2OwnedMount> GetAllOwnedMounts() const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    bool GetOwnedMount(FGuid InstanceId, FForge2OwnedMount& OutMount) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void SetMountFavorite(FGuid InstanceId, bool bFavorite);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void RenamMount(FGuid InstanceId, const FString& NewName);

    // ========== Mount Customization ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void SetMountCustomization(FGuid InstanceId, const FForge2MountCustomization& Customization);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    FForge2MountCustomization GetMountCustomization(FGuid InstanceId) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void SetMountColors(FGuid InstanceId, const FLinearColor& Primary, const FLinearColor& Secondary, const FLinearColor& Accent);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void EquipMountArmor(FGuid InstanceId, FName ArmorSetId);

    // ========== Summoning ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    bool SummonMount(FGuid InstanceId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void DismissMount();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    bool IsMountSummoned() const { return bMountSummoned; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    FGuid GetSummonedMountId() const { return SummonedMountId; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    AActor* GetSummonedMountActor() const { return SummonedMountActor.Get(); }

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    bool SummonRandomMount();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    bool SummonFavoriteMount();

    // ========== Mount Control ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void MountUp();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void Dismount();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    bool IsRiding() const { return bIsRiding; }

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void UseMountAbility(FName AbilityId);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    float GetMountStamina() const { return CurrentStamina; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    float GetMountHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void SetMountSprinting(bool bSprinting);

    // ========== Mount Progression ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void AddMountExperience(FGuid InstanceId, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Mount")
    void IncreaseBondLevel(FGuid InstanceId, float Amount);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    float GetBondLevel(FGuid InstanceId) const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Mount")
    int32 GetMountLevel(FGuid InstanceId) const;

    // ========== Events ==========

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Mount|Events")
    FOnMountSummoned OnMountSummoned;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Mount|Events")
    FOnMountDismissed OnMountDismissed;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Mount|Events")
    FOnMountLevelUp OnMountLevelUp;

protected:
    void UpdateMountStats();
    void UpdateStamina(float DeltaTime);
    void TrackDistance(float DeltaTime);
    int32 CalculateXPForLevel(int32 Level) const;

private:
    TMap<FName, FForge2MountDefinition> RegisteredMounts;
    TMap<FGuid, FForge2OwnedMount> OwnedMounts;

    bool bMountSummoned = false;
    FGuid SummonedMountId;
    TWeakObjectPtr<AActor> SummonedMountActor;

    bool bIsRiding = false;
    bool bIsSprinting = false;
    float CurrentStamina = 100.0f;
    float CurrentHealth = 500.0f;
    FVector LastPosition;
};

// ============================================================================
// CHARACTER UTILITIES
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2CharacterUtilities : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ========== Appearance Presets ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2BodyMorphSettings GetMasculineBodyPreset();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2BodyMorphSettings GetFeminineBodyPreset();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2BodyMorphSettings GetAndrogynousBodyPreset();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FLinearColor GetRandomSkinTone();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FLinearColor GetRandomHairColor();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FLinearColor GetRandomEyeColor();

    // ========== Class Presets ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2ClassDefinition GetWarriorClass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2ClassDefinition GetMageClass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2ClassDefinition GetRogueClass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2ClassDefinition GetClericClass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2ClassDefinition GetRangerClass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2SubclassDefinition GetBerserkerSubclass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2SubclassDefinition GetPaladinSubclass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2SubclassDefinition GetPyromancerSubclass();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2SubclassDefinition GetNecromancerSubclass();

    // ========== Mount Presets ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2MountDefinition GetWarhorseMount();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2MountDefinition GetGriffonMount();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2MountDefinition GetDragonMount();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static FForge2MountDefinition GetMechanicalMount();

    // ========== Calculations ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static float CalculateHeightFromMorph(float HeightMorphValue, float MinHeight = 150.0f, float MaxHeight = 210.0f);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static int32 CalculateLevelFromXP(int32 TotalXP, float XPCurveExponent = 1.5f);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static int32 CalculateXPForLevel(int32 Level, int32 BaseXP = 100, float Exponent = 1.5f);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Character|Utilities")
    static float CalculateBondBonus(float BondLevel);
};
