// Forge2TemplateSystem.h
// Phase 21: Templates & Finalization System
// Provides pre-built templates for common game development patterns

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Character.h"
#include "Forge2TemplateSystem.generated.h"

// Forward declarations
class UBehaviorTree;
class UBlackboardData;
class UAnimMontage;
class USkeletalMesh;
class UStaticMesh;
class UMaterialInterface;
class USoundBase;
class UNiagaraSystem;

// ============================================================================
// Template Type Enumerations
// ============================================================================

UENUM(BlueprintType)
enum class EForge2CharacterTemplateType : uint8
{
	PlayerCharacter			UMETA(DisplayName = "Player Character"),
	NPCVillager				UMETA(DisplayName = "NPC Villager"),
	NPCMerchant				UMETA(DisplayName = "NPC Merchant"),
	NPCGuard				UMETA(DisplayName = "NPC Guard"),
	EnemyMelee				UMETA(DisplayName = "Enemy Melee"),
	EnemyRanged				UMETA(DisplayName = "Enemy Ranged"),
	EnemyCaster				UMETA(DisplayName = "Enemy Caster"),
	BossEnemy				UMETA(DisplayName = "Boss Enemy"),
	CompanionNPC			UMETA(DisplayName = "Companion NPC"),
	MountableCreature		UMETA(DisplayName = "Mountable Creature")
};

UENUM(BlueprintType)
enum class EForge2WeaponTemplateType : uint8
{
	MeleeSword				UMETA(DisplayName = "Melee Sword"),
	MeleeAxe				UMETA(DisplayName = "Melee Axe"),
	MeleeMace				UMETA(DisplayName = "Melee Mace"),
	MeleeDagger				UMETA(DisplayName = "Melee Dagger"),
	MeleeSpear				UMETA(DisplayName = "Melee Spear"),
	RangedBow				UMETA(DisplayName = "Ranged Bow"),
	RangedCrossbow			UMETA(DisplayName = "Ranged Crossbow"),
	RangedGun				UMETA(DisplayName = "Ranged Gun"),
	RangedRifle				UMETA(DisplayName = "Ranged Rifle"),
	MagicStaff				UMETA(DisplayName = "Magic Staff"),
	MagicWand				UMETA(DisplayName = "Magic Wand"),
	Shield					UMETA(DisplayName = "Shield"),
	TwoHandedSword			UMETA(DisplayName = "Two-Handed Sword"),
	DualWield				UMETA(DisplayName = "Dual Wield")
};

UENUM(BlueprintType)
enum class EForge2NPCAITemplateType : uint8
{
	Idle					UMETA(DisplayName = "Idle"),
	Patrol					UMETA(DisplayName = "Patrol"),
	Guard					UMETA(DisplayName = "Guard"),
	Merchant				UMETA(DisplayName = "Merchant"),
	Follower				UMETA(DisplayName = "Follower"),
	AggressiveMelee			UMETA(DisplayName = "Aggressive Melee"),
	AggressiveRanged		UMETA(DisplayName = "Aggressive Ranged"),
	DefensiveTank			UMETA(DisplayName = "Defensive Tank"),
	SupportHealer			UMETA(DisplayName = "Support Healer"),
	StealthAssassin			UMETA(DisplayName = "Stealth Assassin"),
	BossMultiPhase			UMETA(DisplayName = "Boss Multi-Phase"),
	FleeingCoward			UMETA(DisplayName = "Fleeing Coward"),
	Berserker				UMETA(DisplayName = "Berserker")
};

UENUM(BlueprintType)
enum class EForge2QuestTemplateType : uint8
{
	FetchQuest				UMETA(DisplayName = "Fetch Quest"),
	KillQuest				UMETA(DisplayName = "Kill Quest"),
	EscortQuest				UMETA(DisplayName = "Escort Quest"),
	DefendQuest				UMETA(DisplayName = "Defend Quest"),
	ExploreQuest			UMETA(DisplayName = "Explore Quest"),
	TalkToQuest				UMETA(DisplayName = "Talk To Quest"),
	CraftingQuest			UMETA(DisplayName = "Crafting Quest"),
	BountyQuest				UMETA(DisplayName = "Bounty Quest"),
	InvestigationQuest		UMETA(DisplayName = "Investigation Quest"),
	MultiPartChain			UMETA(DisplayName = "Multi-Part Chain"),
	BranchingStory			UMETA(DisplayName = "Branching Story"),
	DailyRepeatable			UMETA(DisplayName = "Daily Repeatable"),
	WorldEvent				UMETA(DisplayName = "World Event")
};

UENUM(BlueprintType)
enum class EForge2DungeonTemplateType : uint8
{
	LinearDungeon			UMETA(DisplayName = "Linear Dungeon"),
	BranchingDungeon		UMETA(DisplayName = "Branching Dungeon"),
	LoopingDungeon			UMETA(DisplayName = "Looping Dungeon"),
	BossDungeon				UMETA(DisplayName = "Boss Dungeon"),
	PuzzleDungeon			UMETA(DisplayName = "Puzzle Dungeon"),
	HordeDungeon			UMETA(DisplayName = "Horde Dungeon"),
	TreasureVault			UMETA(DisplayName = "Treasure Vault"),
	CaveDungeon				UMETA(DisplayName = "Cave Dungeon"),
	CryptDungeon			UMETA(DisplayName = "Crypt Dungeon"),
	SewerDungeon			UMETA(DisplayName = "Sewer Dungeon"),
	TowerDungeon			UMETA(DisplayName = "Tower Dungeon"),
	RaidDungeon				UMETA(DisplayName = "Raid Dungeon")
};

UENUM(BlueprintType)
enum class EForge2OpenWorldTemplateType : uint8
{
	ForestChunk				UMETA(DisplayName = "Forest Chunk"),
	DesertChunk				UMETA(DisplayName = "Desert Chunk"),
	MountainChunk			UMETA(DisplayName = "Mountain Chunk"),
	SwampChunk				UMETA(DisplayName = "Swamp Chunk"),
	TundraChunk				UMETA(DisplayName = "Tundra Chunk"),
	PlainsChunk				UMETA(DisplayName = "Plains Chunk"),
	CoastalChunk			UMETA(DisplayName = "Coastal Chunk"),
	VillageChunk			UMETA(DisplayName = "Village Chunk"),
	CityChunk				UMETA(DisplayName = "City Chunk"),
	RuinsChunk				UMETA(DisplayName = "Ruins Chunk"),
	DungeonEntranceChunk	UMETA(DisplayName = "Dungeon Entrance Chunk"),
	BossArenaChunk			UMETA(DisplayName = "Boss Arena Chunk")
};

UENUM(BlueprintType)
enum class EForge2TestCategory : uint8
{
	UnitTest				UMETA(DisplayName = "Unit Test"),
	IntegrationTest			UMETA(DisplayName = "Integration Test"),
	FunctionalTest			UMETA(DisplayName = "Functional Test"),
	PerformanceTest			UMETA(DisplayName = "Performance Test"),
	StressTest				UMETA(DisplayName = "Stress Test"),
	ValidationTest			UMETA(DisplayName = "Validation Test")
};

// ============================================================================
// Character Template Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CharacterStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxMana = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseAttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseArmor = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CriticalChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CriticalMultiplier = 1.5f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CharacterAbilities
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	bool bCanJump = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	bool bCanDoubleJump = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	bool bCanDash = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	bool bCanSprint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	bool bCanCrouch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	bool bCanClimb = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	bool bCanSwim = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	bool bCanFly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<FName> SpecialAbilities;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CharacterTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FName TemplateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	EForge2CharacterTemplateType TemplateType = EForge2CharacterTemplateType::PlayerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FForge2CharacterStats BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FForge2CharacterAbilities Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TSoftClassPtr<UAnimInstance> AnimBlueprintClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TSoftObjectPtr<UBlackboardData> BlackboardAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CapsuleRadius = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CapsuleHalfHeight = 96.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TArray<FName> RequiredComponents;
};

// ============================================================================
// Weapon Template Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WeaponStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageVariance = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Range = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float StaggerPower = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged")
	float ProjectileSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged")
	int32 AmmoCapacity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged")
	float ReloadTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged")
	float Spread = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WeaponCombo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FName ComboName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<TSoftObjectPtr<UAnimMontage>> ComboMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<float> DamageMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float ComboWindowTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float CancelWindowStart = 0.3f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WeaponTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FName TemplateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	EForge2WeaponTemplateType WeaponType = EForge2WeaponTemplateType::MeleeSword;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FForge2WeaponStats Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FForge2WeaponCombo> Combos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TSoftObjectPtr<UStaticMesh> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TSoftObjectPtr<USkeletalMesh> WeaponSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
	TSoftObjectPtr<UMaterialInterface> DefaultMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<UNiagaraSystem> TrailEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<UNiagaraSystem> ImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FName AttachSocketName = FName("weapon_socket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FVector HitboxExtent = FVector(50.0f, 10.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> RequiredClasses;
};

// ============================================================================
// NPC AI Template Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AIPatrolPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	float WaitTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	FRotator FacingDirection = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	FName AnimationToPlay;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AICombatBehavior
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AggressionLevel = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float PreferredCombatDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FleeHealthThreshold = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bUsesRangedAttacks = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bUsesAbilities = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AbilityUseCooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bCallsForHelp = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HelpCallRadius = 1000.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AIPerception
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float SightRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float LoseSightRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float PeripheralVisionAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hearing")
	float HearingRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bSensesDamage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	bool bDetectsNeutrals = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NPCAITemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FName TemplateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	EForge2NPCAITemplateType AIType = EForge2NPCAITemplateType::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	FForge2AIPerception Perception;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FForge2AICombatBehavior CombatBehavior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	TArray<FForge2AIPatrolPoint> PatrolPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	bool bLoopPatrol = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	TSoftObjectPtr<UBlackboardData> Blackboard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanBeInteracted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName DialogueTreeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName ShopId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	TMap<FName, FVector> ScheduleLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	TMap<FName, float> ScheduleTimes;
};

// ============================================================================
// Quest Template Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestObjectiveTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName ObjectiveId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FText ObjectiveDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName ObjectiveType; // Kill, Collect, Interact, TalkTo, ReachLocation, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 RequiredCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	float LocationRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bHiddenUntilProgress = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	float TimeLimit = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestRewardTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 ExperienceReward = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 GoldReward = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TMap<FName, int32> CurrencyRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TMap<FName, int32> ItemRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TArray<FName> ChoiceItemRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TMap<FName, int32> ReputationRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName AbilityUnlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName TitleUnlock;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FName TemplateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	EForge2QuestTemplateType QuestType = EForge2QuestTemplateType::FetchQuest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FText QuestDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName QuestCategory; // Main, Side, Daily, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> RequiredQuests;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TMap<FName, int32> RequiredReputation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
	TArray<FForge2QuestObjectiveTemplate> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FForge2QuestRewardTemplate Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName QuestGiverId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName QuestTurnInId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName DialogueOnAccept;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName DialogueOnComplete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	FName NextQuestId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	TMap<FName, FName> BranchQuests;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float RepeatCooldown = 86400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoTrack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markers")
	bool bShowOnMap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markers")
	bool bShowOnCompass = true;
};

// ============================================================================
// Dungeon Template Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DungeonRoomTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	FName RoomType; // Combat, Puzzle, Treasure, Boss, Safe, Trap

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	FVector2D SizeRange = FVector2D(5.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawns")
	TArray<FName> EnemySpawnTables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawns")
	int32 MinEnemies = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawns")
	int32 MaxEnemies = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FName> LootTables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 MinChests = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 MaxChests = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	TArray<TSoftObjectPtr<UStaticMesh>> PropMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	float PropDensity = 0.3f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DungeonTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FName TemplateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	EForge2DungeonTemplateType DungeonType = EForge2DungeonTemplateType::LinearDungeon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	int32 MinRooms = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	int32 MaxRooms = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float CorridorWidth = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	bool bAllowLoops = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float LoopChance = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	TArray<FForge2DungeonRoomTemplate> RoomTemplates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	bool bHasBossRoom = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	FName BossId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FName BiomeStyle; // Stone, Ice, Fire, Nature, Corrupted

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	TArray<TSoftObjectPtr<UMaterialInterface>> WallMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	TArray<TSoftObjectPtr<UMaterialInterface>> FloorMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor AmbientColor = FLinearColor(0.1f, 0.1f, 0.15f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float TorchDensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	int32 RecommendedLevel = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float DifficultyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
	bool bIsInstanced = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
	int32 MaxPlayers = 4;
};

// ============================================================================
// Open World Chunk Template Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2POITemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FName POIType; // Dungeon, Village, Camp, Shrine, Landmark

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	float SpawnChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FName LinkedTemplateId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bShowOnMap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	FText POIName;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2OpenWorldChunkTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	FName TemplateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	EForge2OpenWorldTemplateType ChunkType = EForge2OpenWorldTemplateType::ForestChunk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	FName BiomeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float BaseElevation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float ElevationVariance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float TerrainRoughness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float TreeDensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float GrassDensity = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	TArray<TSoftObjectPtr<UStaticMesh>> TreeMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	TArray<TSoftObjectPtr<UStaticMesh>> RockMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
	TArray<FName> WildlifeSpawnTables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
	float WildlifeDensity = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemies")
	TArray<FName> EnemySpawnTables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemies")
	float EnemyDensity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TArray<FName> ResourceNodeTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	float ResourceDensity = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	TArray<FForge2POITemplate> POITemplates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roads")
	bool bHasRoads = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roads")
	float RoadDensity = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	bool bHasWater = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	FName WaterType; // River, Lake, Pond, Stream

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	float StreamingDistance = 50000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	int32 RecommendedLevel = 1;
};

// ============================================================================
// Test Result Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TestResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FName TestName;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bPassed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float ExecutionTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	EForge2TestCategory Category = EForge2TestCategory::UnitTest;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TestSuiteResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Suite")
	FName SuiteName;

	UPROPERTY(BlueprintReadOnly, Category = "Suite")
	TArray<FForge2TestResult> TestResults;

	UPROPERTY(BlueprintReadOnly, Category = "Suite")
	int32 TotalTests = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Suite")
	int32 PassedTests = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Suite")
	int32 FailedTests = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Suite")
	float TotalExecutionTime = 0.0f;
};

// ============================================================================
// Template Manager Component
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTemplateGenerated, FName, TemplateId, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTestCompleted, FName, TestName, bool, bPassed);

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2TemplateManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2TemplateManager();

	// Character Templates
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Character")
	bool CreateCharacterFromTemplate(const FForge2CharacterTemplate& Template, FName OutputAssetPath);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Character")
	FForge2CharacterTemplate GetCharacterTemplatePreset(EForge2CharacterTemplateType TemplateType);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Character")
	bool SpawnCharacterFromTemplate(const FForge2CharacterTemplate& Template, FVector Location, FRotator Rotation, AActor*& OutActor);

	// Weapon Templates
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Weapon")
	bool CreateWeaponFromTemplate(const FForge2WeaponTemplate& Template, FName OutputAssetPath);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Weapon")
	FForge2WeaponTemplate GetWeaponTemplatePreset(EForge2WeaponTemplateType WeaponType);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Weapon")
	bool AttachWeaponFromTemplate(const FForge2WeaponTemplate& Template, AActor* OwnerActor, FName SocketName);

	// NPC AI Templates
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|AI")
	bool CreateAIFromTemplate(const FForge2NPCAITemplate& Template, FName OutputAssetPath);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|AI")
	FForge2NPCAITemplate GetAITemplatePreset(EForge2NPCAITemplateType AIType);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|AI")
	bool ApplyAITemplateToActor(const FForge2NPCAITemplate& Template, AActor* TargetActor);

	// Quest Templates
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Quest")
	bool CreateQuestFromTemplate(const FForge2QuestTemplate& Template, FName OutputAssetPath);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Quest")
	FForge2QuestTemplate GetQuestTemplatePreset(EForge2QuestTemplateType QuestType);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Quest")
	bool CreateQuestChainFromTemplates(const TArray<FForge2QuestTemplate>& Templates, FName ChainId);

	// Dungeon Templates
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Dungeon")
	bool GenerateDungeonFromTemplate(const FForge2DungeonTemplate& Template, FVector Origin, int32 Seed);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Dungeon")
	FForge2DungeonTemplate GetDungeonTemplatePreset(EForge2DungeonTemplateType DungeonType);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Dungeon")
	bool ValidateDungeonLayout(const FForge2DungeonTemplate& Template, FString& OutValidationErrors);

	// Open World Chunk Templates
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|OpenWorld")
	bool GenerateChunkFromTemplate(const FForge2OpenWorldChunkTemplate& Template, FVector Origin, int32 Seed);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|OpenWorld")
	FForge2OpenWorldChunkTemplate GetChunkTemplatePreset(EForge2OpenWorldTemplateType ChunkType);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|OpenWorld")
	bool RegisterChunkForStreaming(const FForge2OpenWorldChunkTemplate& Template, FVector ChunkOrigin);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Templates|Events")
	FOnTemplateGenerated OnTemplateGenerated;

protected:
	UPROPERTY()
	TMap<FName, FForge2CharacterTemplate> CharacterTemplates;

	UPROPERTY()
	TMap<FName, FForge2WeaponTemplate> WeaponTemplates;

	UPROPERTY()
	TMap<FName, FForge2NPCAITemplate> AITemplates;

	UPROPERTY()
	TMap<FName, FForge2QuestTemplate> QuestTemplates;

	UPROPERTY()
	TMap<FName, FForge2DungeonTemplate> DungeonTemplates;

	UPROPERTY()
	TMap<FName, FForge2OpenWorldChunkTemplate> ChunkTemplates;
};

// ============================================================================
// Test Runner Component
// ============================================================================

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2TestRunner : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2TestRunner();

	// Test Execution
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing")
	bool RunTest(FName TestName, FForge2TestResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing")
	bool RunTestSuite(FName SuiteName, FForge2TestSuiteResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing")
	bool RunAllTests(TArray<FForge2TestSuiteResult>& OutResults);

	// Test Registration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing")
	void RegisterTest(FName TestName, EForge2TestCategory Category);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing")
	void RegisterTestSuite(FName SuiteName, const TArray<FName>& TestNames);

	// Validation Tests
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Validation")
	bool ValidateCharacterTemplate(const FForge2CharacterTemplate& Template, FString& OutErrors);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Validation")
	bool ValidateWeaponTemplate(const FForge2WeaponTemplate& Template, FString& OutErrors);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Validation")
	bool ValidateQuestTemplate(const FForge2QuestTemplate& Template, FString& OutErrors);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Validation")
	bool ValidateDungeonTemplate(const FForge2DungeonTemplate& Template, FString& OutErrors);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Validation")
	bool ValidateOpenWorldTemplate(const FForge2OpenWorldChunkTemplate& Template, FString& OutErrors);

	// Integration Tests
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Integration")
	bool TestWorldGeneration(int32 Seed, FString& OutReport);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Integration")
	bool TestCombatSystem(FString& OutReport);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Integration")
	bool TestQuestSystem(FString& OutReport);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Integration")
	bool TestInventorySystem(FString& OutReport);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Integration")
	bool TestAISystem(FString& OutReport);

	// Performance Tests
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Testing|Performance")
	bool RunPerformanceTest(FName TestName, int32 Iterations, float& OutAverageTime, float& OutMaxTime);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Testing|Events")
	FOnTestCompleted OnTestCompleted;

protected:
	UPROPERTY()
	TMap<FName, EForge2TestCategory> RegisteredTests;

	UPROPERTY()
	TMap<FName, TArray<FName>> TestSuites;

	UPROPERTY()
	TArray<FForge2TestResult> LastTestResults;
};

// ============================================================================
// Template Utilities
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2TemplateUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Character Template Presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2CharacterTemplate CreatePlayerCharacterPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2CharacterTemplate CreateMeleeEnemyPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2CharacterTemplate CreateRangedEnemyPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2CharacterTemplate CreateBossEnemyPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2CharacterTemplate CreateMerchantNPCPreset();

	// Weapon Template Presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2WeaponTemplate CreateSwordPreset(bool bTwoHanded = false);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2WeaponTemplate CreateBowPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2WeaponTemplate CreateStaffPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2WeaponTemplate CreateShieldPreset();

	// AI Template Presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2NPCAITemplate CreatePatrolGuardAIPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2NPCAITemplate CreateAggressiveMeleeAIPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2NPCAITemplate CreateMerchantAIPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2NPCAITemplate CreateBossAIPreset();

	// Quest Template Presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2QuestTemplate CreateFetchQuestPreset(FName ItemId, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2QuestTemplate CreateKillQuestPreset(FName EnemyId, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2QuestTemplate CreateExploreQuestPreset(FVector Location, float Radius);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2QuestTemplate CreateEscortQuestPreset(FName NPCId, FVector Destination);

	// Dungeon Template Presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2DungeonTemplate CreateLinearDungeonPreset(int32 RoomCount);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2DungeonTemplate CreateBossDungeonPreset(FName BossId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2DungeonTemplate CreatePuzzleDungeonPreset();

	// Open World Template Presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2OpenWorldChunkTemplate CreateForestChunkPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2OpenWorldChunkTemplate CreateDesertChunkPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2OpenWorldChunkTemplate CreateMountainChunkPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Presets")
	static FForge2OpenWorldChunkTemplate CreateVillageChunkPreset();

	// Template Conversion
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Conversion")
	static FString TemplateToJson(const FForge2CharacterTemplate& Template);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Conversion")
	static bool JsonToCharacterTemplate(const FString& JsonString, FForge2CharacterTemplate& OutTemplate);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Conversion")
	static bool SaveTemplateToFile(const FForge2CharacterTemplate& Template, const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Conversion")
	static bool LoadTemplateFromFile(const FString& FilePath, FForge2CharacterTemplate& OutTemplate);

	// Validation Helpers
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Validation")
	static bool IsValidCharacterTemplate(const FForge2CharacterTemplate& Template);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Validation")
	static bool IsValidWeaponTemplate(const FForge2WeaponTemplate& Template);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Validation")
	static bool IsValidQuestTemplate(const FForge2QuestTemplate& Template);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Validation")
	static bool IsValidDungeonTemplate(const FForge2DungeonTemplate& Template);

	// Template Merging
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Merge")
	static FForge2CharacterTemplate MergeCharacterTemplates(const FForge2CharacterTemplate& Base, const FForge2CharacterTemplate& Override);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Templates|Merge")
	static FForge2WeaponTemplate MergeWeaponTemplates(const FForge2WeaponTemplate& Base, const FForge2WeaponTemplate& Override);
};
