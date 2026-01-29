// Forge2QuestSystem.h
// FORGE2 Quest System - Complete quest management with objectives, rewards, chains, and markers
// Part of the FORGE2 Ultimate AAA Game Development MCP Extension

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Forge2QuestSystem.generated.h"

// Forward declarations
class AActor;
class UWorld;

// ============================================================================
// QUEST ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2QuestCategory : uint8
{
	Main			UMETA(DisplayName = "Main Quest"),
	Side			UMETA(DisplayName = "Side Quest"),
	Daily			UMETA(DisplayName = "Daily Quest"),
	Weekly			UMETA(DisplayName = "Weekly Quest"),
	Hidden			UMETA(DisplayName = "Hidden Quest"),
	Faction			UMETA(DisplayName = "Faction Quest"),
	World			UMETA(DisplayName = "World Quest"),
	Event			UMETA(DisplayName = "Event Quest"),
	Tutorial		UMETA(DisplayName = "Tutorial Quest"),
	Bounty			UMETA(DisplayName = "Bounty Quest")
};

UENUM(BlueprintType)
enum class EForge2QuestState : uint8
{
	Unknown			UMETA(DisplayName = "Unknown"),
	Available		UMETA(DisplayName = "Available"),
	Active			UMETA(DisplayName = "Active"),
	Completed		UMETA(DisplayName = "Completed"),
	Failed			UMETA(DisplayName = "Failed"),
	Abandoned		UMETA(DisplayName = "Abandoned"),
	Locked			UMETA(DisplayName = "Locked"),
	Expired			UMETA(DisplayName = "Expired")
};

UENUM(BlueprintType)
enum class EForge2ObjectiveType : uint8
{
	Kill			UMETA(DisplayName = "Kill"),
	Collect			UMETA(DisplayName = "Collect"),
	Interact		UMETA(DisplayName = "Interact"),
	TalkTo			UMETA(DisplayName = "Talk To"),
	Escort			UMETA(DisplayName = "Escort"),
	Defend			UMETA(DisplayName = "Defend"),
	ReachLocation	UMETA(DisplayName = "Reach Location"),
	Survive			UMETA(DisplayName = "Survive"),
	Craft			UMETA(DisplayName = "Craft"),
	Discover		UMETA(DisplayName = "Discover"),
	Photograph		UMETA(DisplayName = "Photograph"),
	Deliver			UMETA(DisplayName = "Deliver"),
	Use				UMETA(DisplayName = "Use Item"),
	Equip			UMETA(DisplayName = "Equip Item"),
	LevelUp			UMETA(DisplayName = "Level Up"),
	LearnAbility	UMETA(DisplayName = "Learn Ability"),
	CompleteQuest	UMETA(DisplayName = "Complete Quest"),
	WinMatch		UMETA(DisplayName = "Win Match"),
	Custom			UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2ObjectiveState : uint8
{
	Inactive		UMETA(DisplayName = "Inactive"),
	Active			UMETA(DisplayName = "Active"),
	Completed		UMETA(DisplayName = "Completed"),
	Failed			UMETA(DisplayName = "Failed"),
	Skipped			UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EForge2PrerequisiteType : uint8
{
	QuestComplete		UMETA(DisplayName = "Quest Complete"),
	QuestActive			UMETA(DisplayName = "Quest Active"),
	Level				UMETA(DisplayName = "Level Requirement"),
	Reputation			UMETA(DisplayName = "Reputation Requirement"),
	Item				UMETA(DisplayName = "Item Required"),
	Class				UMETA(DisplayName = "Class Required"),
	Race				UMETA(DisplayName = "Race Required"),
	Skill				UMETA(DisplayName = "Skill Required"),
	Achievement			UMETA(DisplayName = "Achievement Required"),
	Time				UMETA(DisplayName = "Time Requirement"),
	Location			UMETA(DisplayName = "Location Required"),
	Custom				UMETA(DisplayName = "Custom Condition")
};

UENUM(BlueprintType)
enum class EForge2RewardType : uint8
{
	Experience		UMETA(DisplayName = "Experience"),
	Currency		UMETA(DisplayName = "Currency"),
	Item			UMETA(DisplayName = "Item"),
	Reputation		UMETA(DisplayName = "Reputation"),
	Ability			UMETA(DisplayName = "Ability Unlock"),
	Title			UMETA(DisplayName = "Title"),
	Recipe			UMETA(DisplayName = "Recipe"),
	Mount			UMETA(DisplayName = "Mount"),
	Pet				UMETA(DisplayName = "Pet"),
	Cosmetic		UMETA(DisplayName = "Cosmetic"),
	Buff			UMETA(DisplayName = "Temporary Buff"),
	SkillPoint		UMETA(DisplayName = "Skill Point"),
	StatPoint		UMETA(DisplayName = "Stat Point"),
	Unlock			UMETA(DisplayName = "Feature Unlock"),
	Custom			UMETA(DisplayName = "Custom Reward")
};

UENUM(BlueprintType)
enum class EForge2MarkerType : uint8
{
	None			UMETA(DisplayName = "No Marker"),
	Compass			UMETA(DisplayName = "Compass Only"),
	Map				UMETA(DisplayName = "Map Only"),
	Both			UMETA(DisplayName = "Compass and Map"),
	WorldSpace		UMETA(DisplayName = "World Space Marker"),
	All				UMETA(DisplayName = "All Markers")
};

UENUM(BlueprintType)
enum class EForge2FailConditionType : uint8
{
	TimeExpired			UMETA(DisplayName = "Time Expired"),
	NPCDeath			UMETA(DisplayName = "NPC Death"),
	PlayerDeath			UMETA(DisplayName = "Player Death"),
	TargetEscaped		UMETA(DisplayName = "Target Escaped"),
	ObjectDestroyed		UMETA(DisplayName = "Object Destroyed"),
	ItemLost			UMETA(DisplayName = "Item Lost"),
	AreaLeft			UMETA(DisplayName = "Left Area"),
	Abandoned			UMETA(DisplayName = "Quest Abandoned"),
	Custom				UMETA(DisplayName = "Custom Condition")
};

UENUM(BlueprintType)
enum class EForge2BranchConditionType : uint8
{
	ObjectiveComplete	UMETA(DisplayName = "Objective Complete"),
	DialogueChoice		UMETA(DisplayName = "Dialogue Choice"),
	ItemHeld			UMETA(DisplayName = "Item Held"),
	ReputationLevel		UMETA(DisplayName = "Reputation Level"),
	SkillCheck			UMETA(DisplayName = "Skill Check"),
	RandomChance		UMETA(DisplayName = "Random Chance"),
	TimeOfDay			UMETA(DisplayName = "Time of Day"),
	PlayerChoice		UMETA(DisplayName = "Player Choice"),
	Custom				UMETA(DisplayName = "Custom Condition")
};

// ============================================================================
// QUEST OBJECTIVE STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ObjectiveTarget
{
	GENERATED_BODY()

	// Target identifier (enemy type, item ID, NPC ID, location name, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	// Display name for the target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetDisplayName;

	// Target class for spawned/killed targets
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AActor> TargetClass;

	// Specific actor reference for unique targets
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<AActor> TargetActor;

	// Location for location-based objectives
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	// Radius for area-based objectives
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetRadius = 500.0f;

	// Gameplay tags to match targets
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer TargetTags;

	FForge2ObjectiveTarget()
		: TargetId(TEXT(""))
		, TargetDisplayName(TEXT(""))
		, TargetRadius(500.0f)
	{
	}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestObjective
{
	GENERATED_BODY()

	// Unique objective ID within the quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ObjectiveId;

	// Display text for the objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ObjectiveText;

	// Description with more detail
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ObjectiveDescription;

	// Type of objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2ObjectiveType ObjectiveType = EForge2ObjectiveType::Kill;

	// Current state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2ObjectiveState State = EForge2ObjectiveState::Inactive;

	// Target information
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2ObjectiveTarget Target;

	// Required count to complete
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredCount = 1;

	// Current progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCount = 0;

	// Is this objective optional?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOptional = false;

	// Is this objective hidden until certain progress?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHidden = false;

	// Progress percentage at which to reveal hidden objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RevealAtProgress = 0.0f;

	// Time limit in seconds (0 = no limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLimit = 0.0f;

	// Remaining time for timed objectives
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	// Order in which objectives should be displayed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DisplayOrder = 0;

	// Prerequisites within same quest (objective IDs that must be complete first)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PrerequisiteObjectives;

	// Marker configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2MarkerType MarkerType = EForge2MarkerType::Both;

	// Custom marker icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> MarkerIcon;

	// Marker tracking distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MarkerTrackingDistance = 10000.0f;

	// Custom event to fire on completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OnCompleteEvent;

	// Custom data for custom objective types
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> CustomData;

	FForge2QuestObjective()
		: ObjectiveId(TEXT(""))
		, ObjectiveType(EForge2ObjectiveType::Kill)
		, State(EForge2ObjectiveState::Inactive)
		, RequiredCount(1)
		, CurrentCount(0)
		, bOptional(false)
		, bHidden(false)
		, RevealAtProgress(0.0f)
		, TimeLimit(0.0f)
		, TimeRemaining(0.0f)
		, DisplayOrder(0)
		, MarkerType(EForge2MarkerType::Both)
		, MarkerTrackingDistance(10000.0f)
	{
	}

	// Get completion percentage
	float GetProgress() const
	{
		if (RequiredCount <= 0) return 1.0f;
		return FMath::Clamp((float)CurrentCount / (float)RequiredCount, 0.0f, 1.0f);
	}

	// Check if objective is complete
	bool IsComplete() const
	{
		return State == EForge2ObjectiveState::Completed || CurrentCount >= RequiredCount;
	}

	// Check if objective is available (prerequisites met)
	bool IsAvailable(const TArray<FForge2QuestObjective>& AllObjectives) const;

	// Update progress
	bool UpdateProgress(int32 Amount = 1);
};

// ============================================================================
// QUEST REWARD STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestReward
{
	GENERATED_BODY()

	// Reward type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2RewardType RewardType = EForge2RewardType::Experience;

	// Reward identifier (item ID, currency type, reputation faction, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RewardId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	// Amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 1;

	// Is this a choice reward? (player picks one from multiple)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bChoiceReward = false;

	// Choice group (rewards with same group are mutually exclusive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChoiceGroup = 0;

	// Icon for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// Item class for item rewards
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AActor> ItemClass;

	// Duration for temporary rewards (buffs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	// Level scaling factor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LevelScaling = 1.0f;

	// Bonus reward conditions (gameplay tags)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer BonusConditions;

	// Bonus multiplier when conditions met
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusMultiplier = 1.0f;

	FForge2QuestReward()
		: RewardType(EForge2RewardType::Experience)
		, RewardId(TEXT(""))
		, Amount(1)
		, bChoiceReward(false)
		, ChoiceGroup(0)
		, Duration(0.0f)
		, LevelScaling(1.0f)
		, BonusMultiplier(1.0f)
	{
	}

	// Calculate scaled reward amount
	int32 GetScaledAmount(int32 PlayerLevel) const;
};

// ============================================================================
// QUEST PREREQUISITE STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestPrerequisite
{
	GENERATED_BODY()

	// Prerequisite type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2PrerequisiteType PrerequisiteType = EForge2PrerequisiteType::Level;

	// Required value identifier (quest ID, item ID, faction ID, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequiredId;

	// Required numeric value (level, reputation amount, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredValue = 1;

	// Display text describing the requirement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RequirementText;

	// Is this a soft requirement? (shows quest as locked but visible)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSoftRequirement = false;

	// Should we hide the quest entirely if not met?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideIfNotMet = false;

	// Custom condition function name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomConditionFunction;

	FForge2QuestPrerequisite()
		: PrerequisiteType(EForge2PrerequisiteType::Level)
		, RequiredId(TEXT(""))
		, RequiredValue(1)
		, bSoftRequirement(false)
		, bHideIfNotMet(false)
	{
	}

	// Check if prerequisite is met (requires game state context)
	bool IsMet(const TMap<FString, int32>& GameState) const;
};

// ============================================================================
// QUEST CHAIN/BRANCH STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestBranchCondition
{
	GENERATED_BODY()

	// Condition type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2BranchConditionType ConditionType = EForge2BranchConditionType::ObjectiveComplete;

	// Condition identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ConditionId;

	// Required value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredValue = 1;

	// String comparison value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StringValue;

	// Random chance (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RandomChance = 100.0f;

	FForge2QuestBranchCondition()
		: ConditionType(EForge2BranchConditionType::ObjectiveComplete)
		, ConditionId(TEXT(""))
		, RequiredValue(1)
		, RandomChance(100.0f)
	{
	}

	// Evaluate condition
	bool Evaluate(const TMap<FString, int32>& GameState) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestBranch
{
	GENERATED_BODY()

	// Branch identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BranchId;

	// Next quest ID if this branch is taken
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NextQuestId;

	// Conditions that must ALL be met
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2QuestBranchCondition> Conditions;

	// Priority (higher priority branches checked first)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;

	// Display text for player-visible choices
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BranchText;

	// Is this branch visible to the player as a choice?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayerChoice = false;

	FForge2QuestBranch()
		: BranchId(TEXT(""))
		, NextQuestId(TEXT(""))
		, Priority(0)
		, bPlayerChoice(false)
	{
	}

	// Evaluate all conditions
	bool CanTakeBranch(const TMap<FString, int32>& GameState) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestChainEntry
{
	GENERATED_BODY()

	// Quest ID in the chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestId;

	// Order in chain (for linear progression)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainOrder = 0;

	// Possible branches from this quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2QuestBranch> Branches;

	// Default next quest if no branches match
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DefaultNextQuest;

	// Is this the chain start?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bChainStart = false;

	// Is this a chain endpoint?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bChainEnd = false;

	FForge2QuestChainEntry()
		: QuestId(TEXT(""))
		, ChainOrder(0)
		, bChainStart(false)
		, bChainEnd(false)
	{
	}

	// Get next quest based on game state
	FString GetNextQuest(const TMap<FString, int32>& GameState) const;
};

// ============================================================================
// QUEST FAIL CONDITIONS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestFailCondition
{
	GENERATED_BODY()

	// Fail condition type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2FailConditionType ConditionType = EForge2FailConditionType::TimeExpired;

	// Target identifier for condition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	// Target actor for specific entity conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<AActor> TargetActor;

	// Time limit for time-based conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLimit = 0.0f;

	// Location for area-based conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Radius for area conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 0.0f;

	// Can this quest be retried after failure?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanRetry = true;

	// Retry cooldown in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RetryCooldown = 0.0f;

	// Failure message
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FailureMessage;

	FForge2QuestFailCondition()
		: ConditionType(EForge2FailConditionType::TimeExpired)
		, TimeLimit(0.0f)
		, Radius(0.0f)
		, bCanRetry(true)
		, RetryCooldown(0.0f)
	{
	}
};

// ============================================================================
// QUEST MARKER STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestMarker
{
	GENERATED_BODY()

	// Marker identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MarkerId;

	// World location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Target actor (marker follows this if set)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> TargetActor;

	// Marker type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2MarkerType MarkerType = EForge2MarkerType::Both;

	// Icon texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// Icon color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor IconColor = FLinearColor::Yellow;

	// Visibility distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VisibilityDistance = 10000.0f;

	// Should marker scale with distance?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bScaleWithDistance = true;

	// Minimum scale at max distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinScale = 0.5f;

	// Maximum scale at close range
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxScale = 1.0f;

	// Pulsing animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPulse = false;

	// Pulse speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PulseSpeed = 1.0f;

	// Label text
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText LabelText;

	// Distance text visibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDistance = true;

	// Is marker currently active?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActive = true;

	FForge2QuestMarker()
		: MarkerId(TEXT(""))
		, MarkerType(EForge2MarkerType::Both)
		, IconColor(FLinearColor::Yellow)
		, VisibilityDistance(10000.0f)
		, bScaleWithDistance(true)
		, MinScale(0.5f)
		, MaxScale(1.0f)
		, bPulse(false)
		, PulseSpeed(1.0f)
		, bShowDistance(true)
		, bActive(true)
	{
	}

	// Get current world location (from actor or static location)
	FVector GetWorldLocation() const;

	// Calculate scale based on distance
	float CalculateScale(float Distance) const;
};

// ============================================================================
// QUEST DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestDefinition
{
	GENERATED_BODY()

	// Unique quest identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestName;

	// Short description for list view
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ShortDescription;

	// Full description
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FullDescription;

	// Quest category
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2QuestCategory Category = EForge2QuestCategory::Side;

	// Recommended level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecommendedLevel = 1;

	// Level requirement (hard)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LevelRequirement = 0;

	// Is quest repeatable?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepeatable = false;

	// Repeat cooldown in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RepeatCooldown = 0.0f;

	// Time since quest was last completed
	UPROPERTY()
	float TimeSinceCompletion = 0.0f;

	// Quest giver NPC ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestGiverId;

	// Quest turn-in NPC ID (empty = same as giver)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestTurnInId;

	// Zone/area where quest takes place
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestZone;

	// Estimated completion time in minutes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedDuration = 15.0f;

	// Quest icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> QuestIcon;

	// Quest objectives
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2QuestObjective> Objectives;

	// Rewards
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2QuestReward> Rewards;

	// Prerequisites
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2QuestPrerequisite> Prerequisites;

	// Fail conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2QuestFailCondition> FailConditions;

	// Quest chain ID (empty if not part of chain)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainId;

	// Gameplay tags for this quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer QuestTags;

	// Dialogue tree ID to trigger on accept
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AcceptDialogueId;

	// Dialogue tree ID to trigger on complete
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CompleteDialogueId;

	// Auto-track when accepted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoTrack = true;

	// Show in quest log
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowInLog = true;

	// Party quest (shared progress)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPartyQuest = false;

	// Shareable with party
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShareable = true;

	// Max party size for party quests
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPartySize = 5;

	FForge2QuestDefinition()
		: QuestId(TEXT(""))
		, Category(EForge2QuestCategory::Side)
		, RecommendedLevel(1)
		, LevelRequirement(0)
		, bRepeatable(false)
		, RepeatCooldown(0.0f)
		, TimeSinceCompletion(0.0f)
		, EstimatedDuration(15.0f)
		, bAutoTrack(true)
		, bShowInLog(true)
		, bPartyQuest(false)
		, bShareable(true)
		, MaxPartySize(5)
	{
	}

	// Get overall quest progress
	float GetProgress() const;

	// Check if all non-optional objectives are complete
	bool AreRequiredObjectivesComplete() const;

	// Get active objectives
	TArray<FForge2QuestObjective*> GetActiveObjectives();

	// Get next available objectives
	TArray<FForge2QuestObjective*> GetAvailableObjectives();
};

// ============================================================================
// QUEST CHAIN DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestChainDefinition
{
	GENERATED_BODY()

	// Unique chain identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainId;

	// Chain display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChainName;

	// Chain description
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChainDescription;

	// Chain category
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2QuestCategory Category = EForge2QuestCategory::Main;

	// Quests in the chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2QuestChainEntry> Entries;

	// Chain rewards (given on full completion)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2QuestReward> ChainRewards;

	// Chain icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ChainIcon;

	// Has multiple endings?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasMultipleEndings = false;

	// Number of possible endings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EndingCount = 1;

	FForge2QuestChainDefinition()
		: ChainId(TEXT(""))
		, Category(EForge2QuestCategory::Main)
		, bHasMultipleEndings(false)
		, EndingCount(1)
	{
	}

	// Get chain progress (percentage)
	float GetChainProgress(const TSet<FString>& CompletedQuests) const;

	// Get starting quest
	FString GetStartingQuest() const;

	// Get all ending quests
	TArray<FString> GetEndingQuests() const;
};

// ============================================================================
// QUEST RUNTIME STATE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2QuestRuntimeState
{
	GENERATED_BODY()

	// Quest ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestId;

	// Current state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2QuestState State = EForge2QuestState::Unknown;

	// Time accepted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AcceptedTime = 0.0f;

	// Time completed/failed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CompletedTime = 0.0f;

	// Objective progress (objective ID -> current count)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> ObjectiveProgress;

	// Objective states
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, EForge2ObjectiveState> ObjectiveStates;

	// Is currently tracked?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTracked = false;

	// Chosen rewards (for choice rewards)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ChosenRewards;

	// Custom save data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> CustomData;

	// Number of completions (for repeatable quests)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletionCount = 0;

	FForge2QuestRuntimeState()
		: State(EForge2QuestState::Unknown)
		, AcceptedTime(0.0f)
		, CompletedTime(0.0f)
		, bTracked(false)
		, CompletionCount(0)
	{
	}
};

// ============================================================================
// QUEST MANAGER
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2QuestManager
{
public:
	// Quest definitions (loaded from data assets)
	TMap<FString, FForge2QuestDefinition> QuestDefinitions;

	// Quest chains
	TMap<FString, FForge2QuestChainDefinition> QuestChains;

	// Runtime quest states
	TMap<FString, FForge2QuestRuntimeState> QuestStates;

	// Currently tracked quests
	TArray<FString> TrackedQuests;

	// Maximum tracked quests
	int32 MaxTrackedQuests = 3;

	// Active markers
	TArray<FForge2QuestMarker> ActiveMarkers;

	// Completed quest IDs
	TSet<FString> CompletedQuests;

	// Failed quest IDs
	TSet<FString> FailedQuests;

	FForge2QuestManager()
		: MaxTrackedQuests(3)
	{
	}

	// Register a quest definition
	bool RegisterQuest(const FForge2QuestDefinition& Definition);

	// Register a quest chain
	bool RegisterChain(const FForge2QuestChainDefinition& Chain);

	// Check if quest is available
	bool IsQuestAvailable(const FString& QuestId, const TMap<FString, int32>& GameState) const;

	// Accept a quest
	bool AcceptQuest(const FString& QuestId);

	// Abandon a quest
	bool AbandonQuest(const FString& QuestId);

	// Complete a quest
	bool CompleteQuest(const FString& QuestId);

	// Fail a quest
	bool FailQuest(const FString& QuestId, const FString& Reason);

	// Update objective progress
	bool UpdateObjective(const FString& QuestId, const FString& ObjectiveId, int32 Progress = 1);

	// Complete an objective
	bool CompleteObjective(const FString& QuestId, const FString& ObjectiveId);

	// Track a quest
	bool TrackQuest(const FString& QuestId);

	// Untrack a quest
	bool UntrackQuest(const FString& QuestId);

	// Get quest state
	EForge2QuestState GetQuestState(const FString& QuestId) const;

	// Get quest progress
	float GetQuestProgress(const FString& QuestId) const;

	// Get all active quests
	TArray<FString> GetActiveQuests() const;

	// Get all available quests
	TArray<FString> GetAvailableQuests(const TMap<FString, int32>& GameState) const;

	// Get quests by category
	TArray<FString> GetQuestsByCategory(EForge2QuestCategory Category) const;

	// Add marker for objective
	void AddMarkerForObjective(const FString& QuestId, const FString& ObjectiveId);

	// Remove markers for quest
	void RemoveMarkersForQuest(const FString& QuestId);

	// Update markers (call each frame for dynamic markers)
	void UpdateMarkers(const FVector& PlayerLocation);

	// Get next quest in chain
	FString GetNextQuestInChain(const FString& CurrentQuestId, const TMap<FString, int32>& GameState) const;

	// Get chain progress
	float GetChainProgress(const FString& ChainId) const;

	// Check for fail conditions
	void CheckFailConditions(const FString& QuestId, float DeltaTime, UWorld* World);

	// Tick (update timers, conditions)
	void Tick(float DeltaTime, UWorld* World);
};

// ============================================================================
// QUEST UTILITY FUNCTIONS
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2QuestUtils
{
public:
	// Create a simple kill quest
	static FForge2QuestDefinition CreateKillQuest(
		const FString& QuestId,
		const FText& QuestName,
		const FString& EnemyType,
		int32 KillCount,
		int32 XPReward);

	// Create a simple collect quest
	static FForge2QuestDefinition CreateCollectQuest(
		const FString& QuestId,
		const FText& QuestName,
		const FString& ItemId,
		int32 ItemCount,
		int32 XPReward);

	// Create a delivery quest
	static FForge2QuestDefinition CreateDeliveryQuest(
		const FString& QuestId,
		const FText& QuestName,
		const FString& ItemId,
		const FString& TargetNPCId,
		int32 XPReward);

	// Create an escort quest
	static FForge2QuestDefinition CreateEscortQuest(
		const FString& QuestId,
		const FText& QuestName,
		const FString& NPCId,
		const FVector& Destination,
		int32 XPReward);

	// Create a multi-objective quest
	static FForge2QuestDefinition CreateMultiObjectiveQuest(
		const FString& QuestId,
		const FText& QuestName,
		const TArray<FForge2QuestObjective>& Objectives,
		const TArray<FForge2QuestReward>& Rewards);

	// Create a simple quest chain
	static FForge2QuestChainDefinition CreateLinearChain(
		const FString& ChainId,
		const FText& ChainName,
		const TArray<FString>& QuestIds);

	// Create a branching quest chain
	static FForge2QuestChainDefinition CreateBranchingChain(
		const FString& ChainId,
		const FText& ChainName,
		const TArray<FForge2QuestChainEntry>& Entries);

	// Generate objective from template
	static FForge2QuestObjective GenerateObjective(
		EForge2ObjectiveType Type,
		const FString& TargetId,
		int32 Count,
		bool bOptional = false);

	// Generate reward from template
	static FForge2QuestReward GenerateReward(
		EForge2RewardType Type,
		const FString& RewardId,
		int32 Amount);

	// Calculate appropriate rewards based on level and difficulty
	static TArray<FForge2QuestReward> CalculateScaledRewards(
		int32 PlayerLevel,
		int32 QuestDifficulty,
		EForge2QuestCategory Category);

	// Serialize quest state for save
	static FString SerializeQuestState(const FForge2QuestRuntimeState& State);

	// Deserialize quest state from save
	static FForge2QuestRuntimeState DeserializeQuestState(const FString& Data);
};
