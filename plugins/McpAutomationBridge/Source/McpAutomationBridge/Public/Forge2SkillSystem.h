// Forge2SkillSystem.h
// FORGE2 Skill System - Complete skill trees with abilities, classes, and progression
// Part of the FORGE2 Ultimate AAA Game Development MCP Extension

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Forge2SkillSystem.generated.h"

// Forward declarations
class AActor;
class UTexture2D;

// ============================================================================
// SKILL ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2SkillType : uint8
{
	Passive			UMETA(DisplayName = "Passive"),
	Active			UMETA(DisplayName = "Active"),
	Toggle			UMETA(DisplayName = "Toggle"),
	Aura			UMETA(DisplayName = "Aura"),
	Ultimate		UMETA(DisplayName = "Ultimate"),
	Channeled		UMETA(DisplayName = "Channeled"),
	Combo			UMETA(DisplayName = "Combo Skill"),
	Transform		UMETA(DisplayName = "Transformation"),
	Summon			UMETA(DisplayName = "Summon"),
	Stance			UMETA(DisplayName = "Stance")
};

UENUM(BlueprintType)
enum class EForge2SkillTargetType : uint8
{
	Self			UMETA(DisplayName = "Self"),
	SingleEnemy		UMETA(DisplayName = "Single Enemy"),
	SingleAlly		UMETA(DisplayName = "Single Ally"),
	AreaEnemy		UMETA(DisplayName = "Area (Enemies)"),
	AreaAlly		UMETA(DisplayName = "Area (Allies)"),
	AreaAll			UMETA(DisplayName = "Area (All)"),
	Ground			UMETA(DisplayName = "Ground Target"),
	Direction		UMETA(DisplayName = "Direction"),
	Projectile		UMETA(DisplayName = "Projectile"),
	Chain			UMETA(DisplayName = "Chain"),
	Cone			UMETA(DisplayName = "Cone"),
	Line			UMETA(DisplayName = "Line"),
	Global			UMETA(DisplayName = "Global")
};

UENUM(BlueprintType)
enum class EForge2SkillResourceType : uint8
{
	None			UMETA(DisplayName = "None"),
	Mana			UMETA(DisplayName = "Mana"),
	Stamina			UMETA(DisplayName = "Stamina"),
	Rage			UMETA(DisplayName = "Rage"),
	Energy			UMETA(DisplayName = "Energy"),
	Focus			UMETA(DisplayName = "Focus"),
	Combo			UMETA(DisplayName = "Combo Points"),
	Rune			UMETA(DisplayName = "Runes"),
	Charge			UMETA(DisplayName = "Charges"),
	Health			UMETA(DisplayName = "Health"),
	Soul			UMETA(DisplayName = "Soul"),
	Custom			UMETA(DisplayName = "Custom Resource")
};

UENUM(BlueprintType)
enum class EForge2SkillScalingType : uint8
{
	None			UMETA(DisplayName = "No Scaling"),
	Linear			UMETA(DisplayName = "Linear"),
	Percentage		UMETA(DisplayName = "Percentage"),
	Exponential		UMETA(DisplayName = "Exponential"),
	Diminishing		UMETA(DisplayName = "Diminishing Returns"),
	Custom			UMETA(DisplayName = "Custom Formula")
};

UENUM(BlueprintType)
enum class EForge2StatType : uint8
{
	Strength		UMETA(DisplayName = "Strength"),
	Dexterity		UMETA(DisplayName = "Dexterity"),
	Intelligence	UMETA(DisplayName = "Intelligence"),
	Constitution	UMETA(DisplayName = "Constitution"),
	Wisdom			UMETA(DisplayName = "Wisdom"),
	Charisma		UMETA(DisplayName = "Charisma"),
	Luck			UMETA(DisplayName = "Luck"),
	Spirit			UMETA(DisplayName = "Spirit"),
	Vitality		UMETA(DisplayName = "Vitality"),
	Agility			UMETA(DisplayName = "Agility"),
	AttackPower		UMETA(DisplayName = "Attack Power"),
	SpellPower		UMETA(DisplayName = "Spell Power"),
	CritChance		UMETA(DisplayName = "Critical Chance"),
	CritDamage		UMETA(DisplayName = "Critical Damage"),
	Armor			UMETA(DisplayName = "Armor"),
	MagicResist		UMETA(DisplayName = "Magic Resistance"),
	Haste			UMETA(DisplayName = "Haste"),
	Custom			UMETA(DisplayName = "Custom Stat")
};

UENUM(BlueprintType)
enum class EForge2ClassRole : uint8
{
	Tank			UMETA(DisplayName = "Tank"),
	Healer			UMETA(DisplayName = "Healer"),
	MeleeDPS		UMETA(DisplayName = "Melee DPS"),
	RangedDPS		UMETA(DisplayName = "Ranged DPS"),
	Support			UMETA(DisplayName = "Support"),
	Controller		UMETA(DisplayName = "Controller"),
	Hybrid			UMETA(DisplayName = "Hybrid")
};

UENUM(BlueprintType)
enum class EForge2NodeConnectionType : uint8
{
	Required		UMETA(DisplayName = "Required"),
	Optional		UMETA(DisplayName = "Optional"),
	Exclusive		UMETA(DisplayName = "Mutually Exclusive"),
	Linked			UMETA(DisplayName = "Linked (Both or Neither)")
};

// ============================================================================
// SKILL EFFECT STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkillEffect
{
	GENERATED_BODY()

	// Effect identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EffectId;

	// Effect description
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Stat affected (for stat modifications)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2StatType AffectedStat = EForge2StatType::Strength;

	// Base value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseValue = 10.0f;

	// Value per rank
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ValuePerRank = 5.0f;

	// Scaling type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2SkillScalingType ScalingType = EForge2SkillScalingType::Linear;

	// Scaling stat (for stat-based scaling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2StatType ScalingStat = EForge2StatType::Strength;

	// Scaling coefficient
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScalingCoefficient = 1.0f;

	// Duration (0 = permanent/instant)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	// Duration per rank
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationPerRank = 0.0f;

	// Is this a percentage modifier?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPercentage = false;

	// Stacks?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanStack = false;

	// Max stacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStacks = 1;

	// Tick interval for DOTs/HOTs (0 = instant)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TickInterval = 0.0f;

	// Visual effect to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AActor> VisualEffectClass;

	// Gameplay tags granted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer GrantedTags;

	// Custom effect function
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomFunction;

	FForge2SkillEffect()
		: AffectedStat(EForge2StatType::Strength)
		, BaseValue(10.0f)
		, ValuePerRank(5.0f)
		, ScalingType(EForge2SkillScalingType::Linear)
		, ScalingStat(EForge2StatType::Strength)
		, ScalingCoefficient(1.0f)
		, Duration(0.0f)
		, DurationPerRank(0.0f)
		, bPercentage(false)
		, bCanStack(false)
		, MaxStacks(1)
		, TickInterval(0.0f)
	{
	}

	// Calculate effect value at rank
	float CalculateValue(int32 Rank, float StatValue = 0.0f) const;
};

// ============================================================================
// SKILL COST STRUCTURE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkillCost
{
	GENERATED_BODY()

	// Resource type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2SkillResourceType ResourceType = EForge2SkillResourceType::Mana;

	// Custom resource name (if using Custom)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomResourceName;

	// Base cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseCost = 10.0f;

	// Cost per rank (can be negative for reduction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CostPerRank = 0.0f;

	// Percentage of max resource
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPercentageOfMax = false;

	// Cost per second (for channeled/toggle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CostPerSecond = 0.0f;

	FForge2SkillCost()
		: ResourceType(EForge2SkillResourceType::Mana)
		, BaseCost(10.0f)
		, CostPerRank(0.0f)
		, bPercentageOfMax(false)
		, CostPerSecond(0.0f)
	{
	}

	// Calculate cost at rank
	float CalculateCost(int32 Rank) const;
};

// ============================================================================
// SKILL DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkillDefinition
{
	GENERATED_BODY()

	// Unique skill identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkillId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SkillName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Description with values formatted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FormattedDescription;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// Skill type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2SkillType SkillType = EForge2SkillType::Active;

	// Target type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2SkillTargetType TargetType = EForge2SkillTargetType::SingleEnemy;

	// Maximum ranks
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRanks = 5;

	// Point cost per rank
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointCostPerRank = 1;

	// Resource costs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2SkillCost> Costs;

	// Base cooldown
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 0.0f;

	// Cooldown reduction per rank
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownPerRank = 0.0f;

	// Cast time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CastTime = 0.0f;

	// Channel duration (for channeled skills)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChannelDuration = 0.0f;

	// Range
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 500.0f;

	// Radius (for AoE)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 0.0f;

	// Radius per rank
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RadiusPerRank = 0.0f;

	// Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2SkillEffect> Effects;

	// Level requirement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LevelRequirement = 1;

	// Stat requirements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EForge2StatType, int32> StatRequirements;

	// Gameplay tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer SkillTags;

	// Required tags to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer RequiredTags;

	// Blocking tags (can't use if any present)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer BlockingTags;

	// Animation to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> Animation;

	// VFX class
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AActor> VFXClass;

	// Sound effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> Sound;

	// Is this a signature/class-defining skill?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSignatureSkill = false;

	// Can be interrupted?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInterruptible = true;

	// Can be used while moving?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsableWhileMoving = false;

	// Shared cooldown group
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CooldownGroup;

	// Combo next skills
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ComboNextSkills;

	// Combo window duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboWindow = 0.0f;

	FForge2SkillDefinition()
		: SkillType(EForge2SkillType::Active)
		, TargetType(EForge2SkillTargetType::SingleEnemy)
		, MaxRanks(5)
		, PointCostPerRank(1)
		, Cooldown(0.0f)
		, CooldownPerRank(0.0f)
		, CastTime(0.0f)
		, ChannelDuration(0.0f)
		, Range(500.0f)
		, Radius(0.0f)
		, RadiusPerRank(0.0f)
		, LevelRequirement(1)
		, bSignatureSkill(false)
		, bInterruptible(true)
		, bUsableWhileMoving(false)
		, ComboWindow(0.0f)
	{
	}

	// Get formatted description with values for rank
	FText GetFormattedDescription(int32 Rank) const;

	// Calculate total point cost to max rank
	int32 GetTotalPointCost() const;
};

// ============================================================================
// SKILL TREE NODE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkillTreeNode
{
	GENERATED_BODY()

	// Node identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NodeId;

	// Skill ID (references skill definition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkillId;

	// Position in tree UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position = FVector2D::ZeroVector;

	// Node size multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SizeMultiplier = 1.0f;

	// Connected nodes (prerequisites)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PrerequisiteNodes;

	// Connection types for prerequisites
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, EForge2NodeConnectionType> ConnectionTypes;

	// Required total points spent in tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredPointsInTree = 0;

	// Required points in specific tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredPointsInTier = 0;

	// Tier (row in tree)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Tier = 0;

	// Is this a keystone/capstone node?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bKeystone = false;

	// Custom unlock condition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomUnlockCondition;

	// Custom icon override
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> IconOverride;

	// Background color override
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BackgroundColor = FLinearColor::Transparent;

	// Frame color override
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FrameColor = FLinearColor::Transparent;

	FForge2SkillTreeNode()
		: SizeMultiplier(1.0f)
		, RequiredPointsInTree(0)
		, RequiredPointsInTier(0)
		, Tier(0)
		, bKeystone(false)
	{
	}

	// Check if node is unlockable
	bool CanUnlock(const TMap<FString, int32>& AllocatedPoints, int32 TotalPointsSpent, const TMap<FString, int32>& GameState) const;
};

// ============================================================================
// SKILL TREE DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkillTreeDefinition
{
	GENERATED_BODY()

	// Unique tree identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TreeId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TreeName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// Background image
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Background;

	// Nodes in tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2SkillTreeNode> Nodes;

	// Skill definitions for this tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2SkillDefinition> Skills;

	// Number of tiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TierCount = 6;

	// Points required per tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> PointsPerTier;

	// Class restriction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequiredClass;

	// Level requirement to access
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LevelRequirement = 1;

	// Primary color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	// Secondary color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::Gray;

	// Gameplay tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer TreeTags;

	FForge2SkillTreeDefinition()
		: TierCount(6)
		, LevelRequirement(1)
		, PrimaryColor(FLinearColor::White)
		, SecondaryColor(FLinearColor::Gray)
	{
	}

	// Get node by ID
	FForge2SkillTreeNode* GetNode(const FString& NodeId);
	const FForge2SkillTreeNode* GetNode(const FString& NodeId) const;

	// Get skill by ID
	FForge2SkillDefinition* GetSkill(const FString& SkillId);
	const FForge2SkillDefinition* GetSkill(const FString& SkillId) const;

	// Validate tree structure
	bool Validate(TArray<FString>& OutErrors) const;
};

// ============================================================================
// CLASS DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ClassDefinition
{
	GENERATED_BODY()

	// Unique class identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClassId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ClassName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Lore/backstory
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Lore;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// Role
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2ClassRole Role = EForge2ClassRole::MeleeDPS;

	// Secondary role
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2ClassRole SecondaryRole = EForge2ClassRole::MeleeDPS;

	// Base stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EForge2StatType, float> BaseStats;

	// Stat growth per level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EForge2StatType, float> StatGrowth;

	// Primary resource
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2SkillResourceType PrimaryResource = EForge2SkillResourceType::Mana;

	// Secondary resource
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2SkillResourceType SecondaryResource = EForge2SkillResourceType::None;

	// Skill trees for this class
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SkillTreeIds;

	// Starting abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> StartingAbilities;

	// Allowed weapon types
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedWeaponTypes;

	// Allowed armor types
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedArmorTypes;

	// Specializations available
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SpecializationIds;

	// Level at which specializations unlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpecializationLevel = 10;

	// Gameplay tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ClassTags;

	// Class difficulty rating (1-5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DifficultyRating = 3;

	// Recommended new player class
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRecommendedForNewPlayers = false;

	FForge2ClassDefinition()
		: Role(EForge2ClassRole::MeleeDPS)
		, SecondaryRole(EForge2ClassRole::MeleeDPS)
		, PrimaryResource(EForge2SkillResourceType::Mana)
		, SecondaryResource(EForge2SkillResourceType::None)
		, SpecializationLevel(10)
		, DifficultyRating(3)
		, bRecommendedForNewPlayers(false)
	{
	}

	// Get stat at level
	float GetStatAtLevel(EForge2StatType Stat, int32 Level) const;
};

// ============================================================================
// SPECIALIZATION DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SpecializationDefinition
{
	GENERATED_BODY()

	// Unique specialization identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpecId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpecName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// Parent class
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ParentClassId;

	// Role override
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2ClassRole RoleOverride = EForge2ClassRole::MeleeDPS;

	// Stat modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EForge2StatType, float> StatModifiers;

	// Additional skill tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AdditionalSkillTreeId;

	// Exclusive abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ExclusiveAbilities;

	// New allowed weapon types
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AdditionalWeaponTypes;

	// New allowed armor types
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AdditionalArmorTypes;

	// Passive bonuses
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2SkillEffect> PassiveBonuses;

	// Gameplay tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer SpecTags;

	FForge2SpecializationDefinition()
		: RoleOverride(EForge2ClassRole::MeleeDPS)
	{
	}
};

// ============================================================================
// SKILL RUNTIME STATE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkillRuntimeState
{
	GENERATED_BODY()

	// Skill ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkillId;

	// Current rank
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentRank = 0;

	// Is unlocked?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;

	// Current cooldown remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	// Is currently active (for toggles/stances)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActive = false;

	// Current stacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStacks = 0;

	// Times used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesUsed = 0;

	FForge2SkillRuntimeState()
		: CurrentRank(0)
		, bUnlocked(false)
		, CooldownRemaining(0.0f)
		, bActive(false)
		, CurrentStacks(0)
		, TimesUsed(0)
	{
	}
};

// ============================================================================
// CHARACTER SKILL PROFILE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CharacterSkillProfile
{
	GENERATED_BODY()

	// Character class
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClassId;

	// Specialization (if chosen)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpecializationId;

	// Available skill points
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AvailablePoints = 0;

	// Points spent per tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> PointsPerTree;

	// Skill ranks allocated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> SkillRanks;

	// Skill runtime states
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FForge2SkillRuntimeState> SkillStates;

	// Hotbar assignments (slot -> skill ID)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, FString> HotbarSlots;

	// Total respecs used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RespecsUsed = 0;

	FForge2CharacterSkillProfile()
		: AvailablePoints(0)
		, RespecsUsed(0)
	{
	}

	// Get total points spent
	int32 GetTotalPointsSpent() const;

	// Get points spent in tree
	int32 GetPointsInTree(const FString& TreeId) const;
};

// ============================================================================
// SKILL MANAGER
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2SkillManager
{
public:
	// Skill definitions
	TMap<FString, FForge2SkillDefinition> SkillDefinitions;

	// Skill trees
	TMap<FString, FForge2SkillTreeDefinition> SkillTrees;

	// Class definitions
	TMap<FString, FForge2ClassDefinition> ClassDefinitions;

	// Specializations
	TMap<FString, FForge2SpecializationDefinition> Specializations;

	// Respec settings
	int32 FreeRespecsPerCharacter = 1;
	float RespecCostMultiplier = 1.0f;
	bool bAllowPartialRespec = true;

	FForge2SkillManager()
		: FreeRespecsPerCharacter(1)
		, RespecCostMultiplier(1.0f)
		, bAllowPartialRespec(true)
	{
	}

	// Register skill definition
	bool RegisterSkill(const FForge2SkillDefinition& Skill);

	// Register skill tree
	bool RegisterSkillTree(const FForge2SkillTreeDefinition& Tree);

	// Register class
	bool RegisterClass(const FForge2ClassDefinition& Class);

	// Register specialization
	bool RegisterSpecialization(const FForge2SpecializationDefinition& Spec);

	// Check if skill can be learned
	bool CanLearnSkill(const FForge2CharacterSkillProfile& Profile, const FString& SkillId, const TMap<FString, int32>& GameState) const;

	// Learn skill (allocate point)
	bool LearnSkill(FForge2CharacterSkillProfile& Profile, const FString& SkillId);

	// Unlearn skill (for respec)
	bool UnlearnSkill(FForge2CharacterSkillProfile& Profile, const FString& SkillId);

	// Full respec tree
	bool RespecTree(FForge2CharacterSkillProfile& Profile, const FString& TreeId);

	// Full respec all
	bool RespecAll(FForge2CharacterSkillProfile& Profile);

	// Calculate respec cost
	int32 CalculateRespecCost(const FForge2CharacterSkillProfile& Profile) const;

	// Can use skill?
	bool CanUseSkill(const FForge2CharacterSkillProfile& Profile, const FString& SkillId, float CurrentResource, const TMap<FString, int32>& GameState) const;

	// Use skill
	bool UseSkill(FForge2CharacterSkillProfile& Profile, const FString& SkillId, float& CurrentResource);

	// Tick cooldowns
	void TickCooldowns(FForge2CharacterSkillProfile& Profile, float DeltaTime);

	// Get skill value at current rank
	float GetSkillValue(const FForge2CharacterSkillProfile& Profile, const FString& SkillId, int32 EffectIndex = 0, float StatValue = 0.0f) const;

	// Get available skills for character
	TArray<FString> GetAvailableSkills(const FForge2CharacterSkillProfile& Profile) const;

	// Get learned skills
	TArray<FString> GetLearnedSkills(const FForge2CharacterSkillProfile& Profile) const;

	// Assign to hotbar
	bool AssignToHotbar(FForge2CharacterSkillProfile& Profile, int32 Slot, const FString& SkillId);

	// Choose specialization
	bool ChooseSpecialization(FForge2CharacterSkillProfile& Profile, const FString& SpecId, const TMap<FString, int32>& GameState);
};

// ============================================================================
// SKILL UTILITY FUNCTIONS
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2SkillUtils
{
public:
	// Create a passive skill
	static FForge2SkillDefinition CreatePassiveSkill(
		const FString& SkillId,
		const FText& SkillName,
		EForge2StatType AffectedStat,
		float BaseBonus,
		float BonusPerRank,
		bool bPercentage = false);

	// Create an active damage skill
	static FForge2SkillDefinition CreateDamageSkill(
		const FString& SkillId,
		const FText& SkillName,
		float BaseDamage,
		float DamagePerRank,
		EForge2SkillResourceType ResourceType,
		float ResourceCost,
		float Cooldown);

	// Create a heal skill
	static FForge2SkillDefinition CreateHealSkill(
		const FString& SkillId,
		const FText& SkillName,
		float BaseHeal,
		float HealPerRank,
		EForge2SkillResourceType ResourceType,
		float ResourceCost,
		float Cooldown);

	// Create a buff skill
	static FForge2SkillDefinition CreateBuffSkill(
		const FString& SkillId,
		const FText& SkillName,
		EForge2StatType BuffedStat,
		float BaseBonus,
		float BonusPerRank,
		float Duration,
		EForge2SkillResourceType ResourceType,
		float ResourceCost);

	// Create a simple skill tree
	static FForge2SkillTreeDefinition CreateLinearTree(
		const FString& TreeId,
		const FText& TreeName,
		const TArray<FForge2SkillDefinition>& Skills);

	// Create a standard class
	static FForge2ClassDefinition CreateClass(
		const FString& ClassId,
		const FText& ClassName,
		EForge2ClassRole Role,
		const TMap<EForge2StatType, float>& BaseStats,
		const TArray<FString>& SkillTreeIds);

	// Serialize skill profile for save
	static FString SerializeSkillProfile(const FForge2CharacterSkillProfile& Profile);

	// Deserialize skill profile from save
	static FForge2CharacterSkillProfile DeserializeSkillProfile(const FString& Data);
};
