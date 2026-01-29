// Forge2DialogueSystem.h
// FORGE2 Dialogue System - Complete dialogue trees with branching, conditions, and consequences
// Part of the FORGE2 Ultimate AAA Game Development MCP Extension

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Forge2DialogueSystem.generated.h"

// Forward declarations
class AActor;
class USoundWave;
class UAnimMontage;
class UTexture2D;

// ============================================================================
// DIALOGUE ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2DialogueNodeType : uint8
{
	Speech			UMETA(DisplayName = "NPC Speech"),
	PlayerChoice	UMETA(DisplayName = "Player Choice"),
	Condition		UMETA(DisplayName = "Condition Branch"),
	Event			UMETA(DisplayName = "Trigger Event"),
	Random			UMETA(DisplayName = "Random Branch"),
	Start			UMETA(DisplayName = "Start Node"),
	End				UMETA(DisplayName = "End Node"),
	Jump			UMETA(DisplayName = "Jump to Node"),
	Delay			UMETA(DisplayName = "Delay"),
	SetVariable		UMETA(DisplayName = "Set Variable"),
	Animation		UMETA(DisplayName = "Play Animation"),
	Camera			UMETA(DisplayName = "Camera Control")
};

UENUM(BlueprintType)
enum class EForge2DialogueConditionType : uint8
{
	HasItem			UMETA(DisplayName = "Has Item"),
	QuestComplete	UMETA(DisplayName = "Quest Complete"),
	QuestActive		UMETA(DisplayName = "Quest Active"),
	SkillCheck		UMETA(DisplayName = "Skill Check"),
	ReputationCheck	UMETA(DisplayName = "Reputation Check"),
	StatCheck		UMETA(DisplayName = "Stat Check"),
	VariableCheck	UMETA(DisplayName = "Variable Check"),
	FlagCheck		UMETA(DisplayName = "Flag Check"),
	TimeCheck		UMETA(DisplayName = "Time of Day"),
	RandomChance	UMETA(DisplayName = "Random Chance"),
	ClassCheck		UMETA(DisplayName = "Class Check"),
	RaceCheck		UMETA(DisplayName = "Race Check"),
	LevelCheck		UMETA(DisplayName = "Level Check"),
	GenderCheck		UMETA(DisplayName = "Gender Check"),
	Custom			UMETA(DisplayName = "Custom Condition")
};

UENUM(BlueprintType)
enum class EForge2DialogueConsequenceType : uint8
{
	SetVariable		UMETA(DisplayName = "Set Variable"),
	GiveItem		UMETA(DisplayName = "Give Item"),
	TakeItem		UMETA(DisplayName = "Take Item"),
	StartQuest		UMETA(DisplayName = "Start Quest"),
	CompleteQuest	UMETA(DisplayName = "Complete Quest"),
	FailQuest		UMETA(DisplayName = "Fail Quest"),
	CompleteObjective UMETA(DisplayName = "Complete Objective"),
	ChangeReputation UMETA(DisplayName = "Change Reputation"),
	GiveCurrency	UMETA(DisplayName = "Give Currency"),
	TakeCurrency	UMETA(DisplayName = "Take Currency"),
	GiveXP			UMETA(DisplayName = "Give Experience"),
	UnlockAbility	UMETA(DisplayName = "Unlock Ability"),
	TriggerEvent	UMETA(DisplayName = "Trigger Event"),
	SpawnActor		UMETA(DisplayName = "Spawn Actor"),
	DestroyActor	UMETA(DisplayName = "Destroy Actor"),
	PlaySound		UMETA(DisplayName = "Play Sound"),
	PlayAnimation	UMETA(DisplayName = "Play Animation"),
	SetFlag			UMETA(DisplayName = "Set Flag"),
	StartCombat		UMETA(DisplayName = "Start Combat"),
	EndDialogue		UMETA(DisplayName = "End Dialogue"),
	Custom			UMETA(DisplayName = "Custom Action")
};

UENUM(BlueprintType)
enum class EForge2DialogueComparisonType : uint8
{
	Equal			UMETA(DisplayName = "Equal"),
	NotEqual		UMETA(DisplayName = "Not Equal"),
	Greater			UMETA(DisplayName = "Greater Than"),
	GreaterOrEqual	UMETA(DisplayName = "Greater Or Equal"),
	Less			UMETA(DisplayName = "Less Than"),
	LessOrEqual		UMETA(DisplayName = "Less Or Equal"),
	Contains		UMETA(DisplayName = "Contains"),
	StartsWith		UMETA(DisplayName = "Starts With"),
	EndsWith		UMETA(DisplayName = "Ends With")
};

UENUM(BlueprintType)
enum class EForge2DialogueCameraMode : uint8
{
	Default			UMETA(DisplayName = "Default Camera"),
	CloseUp			UMETA(DisplayName = "Close-Up"),
	OverShoulder	UMETA(DisplayName = "Over Shoulder"),
	TwoShot			UMETA(DisplayName = "Two Shot"),
	WideShot		UMETA(DisplayName = "Wide Shot"),
	Custom			UMETA(DisplayName = "Custom Position")
};

UENUM(BlueprintType)
enum class EForge2DialogueEmotionType : uint8
{
	Neutral			UMETA(DisplayName = "Neutral"),
	Happy			UMETA(DisplayName = "Happy"),
	Sad				UMETA(DisplayName = "Sad"),
	Angry			UMETA(DisplayName = "Angry"),
	Surprised		UMETA(DisplayName = "Surprised"),
	Fearful			UMETA(DisplayName = "Fearful"),
	Disgusted		UMETA(DisplayName = "Disgusted"),
	Thinking		UMETA(DisplayName = "Thinking"),
	Confused		UMETA(DisplayName = "Confused"),
	Sarcastic		UMETA(DisplayName = "Sarcastic"),
	Threatening		UMETA(DisplayName = "Threatening"),
	Pleading		UMETA(DisplayName = "Pleading")
};

// ============================================================================
// SPEAKER PROFILE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SpeakerProfile
{
	GENERATED_BODY()

	// Unique speaker identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeakerId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	// Portrait texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Portrait;

	// Alternative portraits for different emotions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EForge2DialogueEmotionType, TSoftObjectPtr<UTexture2D>> EmotionPortraits;

	// Voice audio asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> VoiceClip;

	// Voice pitch modifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VoicePitch = 1.0f;

	// Speaking speed (characters per second for text display)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeakingSpeed = 30.0f;

	// Idle animation to play during dialogue
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> IdleAnimation;

	// Talking animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> TalkingAnimation;

	// Name color for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor NameColor = FLinearColor::White;

	// Text color for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TextColor = FLinearColor::White;

	// Is this the player?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlayer = false;

	// Actor reference in world
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<AActor> ActorReference;

	// Gameplay tags for this speaker
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer SpeakerTags;

	FForge2SpeakerProfile()
		: SpeakerId(TEXT(""))
		, VoicePitch(1.0f)
		, SpeakingSpeed(30.0f)
		, NameColor(FLinearColor::White)
		, TextColor(FLinearColor::White)
		, bIsPlayer(false)
	{
	}

	// Get portrait for emotion
	TSoftObjectPtr<UTexture2D> GetPortraitForEmotion(EForge2DialogueEmotionType Emotion) const;
};

// ============================================================================
// DIALOGUE CONDITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueCondition
{
	GENERATED_BODY()

	// Condition type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DialogueConditionType ConditionType = EForge2DialogueConditionType::FlagCheck;

	// Target identifier (item ID, quest ID, skill name, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	// Comparison type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DialogueComparisonType Comparison = EForge2DialogueComparisonType::Equal;

	// Required value (numeric)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredValue = 1;

	// Required string value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequiredString;

	// Random chance percentage (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RandomChance = 50.0f;

	// Invert the condition result
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInvert = false;

	// Custom function name for custom conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomFunction;

	// Display text shown to player (for skill checks, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ConditionDisplayText;

	// Should show condition requirement in choice text?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowInChoiceText = true;

	// Color when condition is met
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor MetColor = FLinearColor::Green;

	// Color when condition is not met
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor UnmetColor = FLinearColor::Red;

	FForge2DialogueCondition()
		: ConditionType(EForge2DialogueConditionType::FlagCheck)
		, Comparison(EForge2DialogueComparisonType::Equal)
		, RequiredValue(1)
		, RandomChance(50.0f)
		, bInvert(false)
		, bShowInChoiceText(true)
		, MetColor(FLinearColor::Green)
		, UnmetColor(FLinearColor::Red)
	{
	}

	// Evaluate condition against game state
	bool Evaluate(const TMap<FString, int32>& GameState) const;

	// Compare value against requirement
	bool CompareValue(int32 ActualValue) const;
};

// ============================================================================
// DIALOGUE CONSEQUENCE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueConsequence
{
	GENERATED_BODY()

	// Consequence type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DialogueConsequenceType ConsequenceType = EForge2DialogueConsequenceType::SetVariable;

	// Target identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	// Value (amount, level, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value = 1;

	// String value for string variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StringValue;

	// Actor class for spawn actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AActor> ActorClass;

	// Sound to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> Sound;

	// Animation to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> Animation;

	// Delay before executing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Delay = 0.0f;

	// Custom function name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomFunction;

	// Custom parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> CustomParameters;

	FForge2DialogueConsequence()
		: ConsequenceType(EForge2DialogueConsequenceType::SetVariable)
		, Value(1)
		, Delay(0.0f)
	{
	}

	// Execute the consequence
	void Execute(TMap<FString, int32>& GameState) const;
};

// ============================================================================
// DIALOGUE CHOICE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueChoice
{
	GENERATED_BODY()

	// Choice identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChoiceId;

	// Choice text displayed to player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChoiceText;

	// Target node ID when this choice is selected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetNodeId;

	// Conditions that must be met to show/enable this choice
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueCondition> Conditions;

	// Consequences when this choice is selected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueConsequence> Consequences;

	// Is choice visible when conditions are not met?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowWhenLocked = true;

	// Is choice selectable when conditions are not met?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSelectableWhenLocked = false;

	// Tooltip when locked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText LockedTooltip;

	// Display order
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DisplayOrder = 0;

	// Is this a "back" or exit option?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsExitOption = false;

	// Shortcut key (1-9, 0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShortcutKey = -1;

	// Icon for the choice
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ChoiceIcon;

	// Color override for choice text
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TextColor = FLinearColor::White;

	// Has this choice been selected before? (for tracking)
	UPROPERTY()
	bool bPreviouslySelected = false;

	FForge2DialogueChoice()
		: bShowWhenLocked(true)
		, bSelectableWhenLocked(false)
		, DisplayOrder(0)
		, bIsExitOption(false)
		, ShortcutKey(-1)
		, TextColor(FLinearColor::White)
		, bPreviouslySelected(false)
	{
	}

	// Check if all conditions are met
	bool AreConditionsMet(const TMap<FString, int32>& GameState) const;

	// Get formatted choice text (with condition indicators)
	FText GetFormattedText(const TMap<FString, int32>& GameState) const;
};

// ============================================================================
// DIALOGUE NODE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueNode
{
	GENERATED_BODY()

	// Unique node identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NodeId;

	// Node type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DialogueNodeType NodeType = EForge2DialogueNodeType::Speech;

	// Speaker ID for speech nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeakerId;

	// Dialogue text
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DialogueText;

	// Localized dialogue text variants
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FText> LocalizedText;

	// Audio for this line
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> Audio;

	// Animation to play during this node
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> Animation;

	// Emotion for this line
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DialogueEmotionType Emotion = EForge2DialogueEmotionType::Neutral;

	// Camera mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DialogueCameraMode CameraMode = EForge2DialogueCameraMode::Default;

	// Custom camera position/rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CustomCameraTransform;

	// Player choices (for PlayerChoice nodes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueChoice> Choices;

	// Next node ID (for linear progression)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NextNodeId;

	// Condition branches (for Condition nodes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueCondition> BranchConditions;

	// Target nodes for condition branches
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> BranchTargets;

	// Default branch if no conditions match
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DefaultBranch;

	// Random weights for random nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> RandomWeights;

	// Entry conditions for this node
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueCondition> EntryConditions;

	// Consequences triggered when entering this node
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueConsequence> EntryConsequences;

	// Consequences triggered when leaving this node
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueConsequence> ExitConsequences;

	// Event to trigger
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventName;

	// Variable name for SetVariable nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VariableName;

	// Variable value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VariableValue = 0;

	// Delay duration for delay nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayDuration = 1.0f;

	// Jump target for jump nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString JumpTargetTree;

	// Display duration override (0 = auto-calculate)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DisplayDuration = 0.0f;

	// Auto-advance to next node
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoAdvance = false;

	// Skip input allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkippable = true;

	// Node position in editor (for visual layout)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D EditorPosition = FVector2D::ZeroVector;

	// Editor comment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EditorComment;

	FForge2DialogueNode()
		: NodeType(EForge2DialogueNodeType::Speech)
		, Emotion(EForge2DialogueEmotionType::Neutral)
		, CameraMode(EForge2DialogueCameraMode::Default)
		, VariableValue(0)
		, DelayDuration(1.0f)
		, DisplayDuration(0.0f)
		, bAutoAdvance(false)
		, bSkippable(true)
	{
	}

	// Get display duration (calculated or override)
	float GetDisplayDuration(float CharactersPerSecond = 30.0f) const;

	// Get next node based on conditions and game state
	FString GetNextNode(const TMap<FString, int32>& GameState, int32 ChoiceIndex = -1) const;

	// Check if node can be entered
	bool CanEnter(const TMap<FString, int32>& GameState) const;

	// Execute entry consequences
	void OnEnter(TMap<FString, int32>& GameState) const;

	// Execute exit consequences
	void OnExit(TMap<FString, int32>& GameState) const;
};

// ============================================================================
// DIALOGUE TREE DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueTreeDefinition
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

	// All nodes in the tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueNode> Nodes;

	// Speaker profiles used in this tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2SpeakerProfile> Speakers;

	// Start node ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StartNodeId;

	// Can this dialogue be repeated?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepeatable = true;

	// Once-per-session (reset on game load)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOncePerSession = false;

	// Prerequisites for starting this dialogue
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueCondition> Prerequisites;

	// Consequences when dialogue starts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueConsequence> OnStartConsequences;

	// Consequences when dialogue ends
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DialogueConsequence> OnEndConsequences;

	// Linked quest ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LinkedQuestId;

	// Gameplay tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer DialogueTags;

	// Priority (for multiple available dialogues)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;

	// Default camera settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DialogueCameraMode DefaultCameraMode = EForge2DialogueCameraMode::TwoShot;

	// Version for save compatibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 1;

	FForge2DialogueTreeDefinition()
		: bRepeatable(true)
		, bOncePerSession(false)
		, Priority(0)
		, DefaultCameraMode(EForge2DialogueCameraMode::TwoShot)
		, Version(1)
	{
	}

	// Get node by ID
	FForge2DialogueNode* GetNode(const FString& NodeId);
	const FForge2DialogueNode* GetNode(const FString& NodeId) const;

	// Get speaker by ID
	FForge2SpeakerProfile* GetSpeaker(const FString& SpeakerId);
	const FForge2SpeakerProfile* GetSpeaker(const FString& SpeakerId) const;

	// Validate tree structure
	bool Validate(TArray<FString>& OutErrors) const;
};

// ============================================================================
// DIALOGUE RUNTIME STATE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueRuntimeState
{
	GENERATED_BODY()

	// Tree ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TreeId;

	// Current node ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentNodeId;

	// Has dialogue been completed?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	// Visited node IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FString> VisitedNodes;

	// Selected choice IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SelectedChoices;

	// Dialogue variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> DialogueVariables;

	// Time dialogue started
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	// Time spent in dialogue
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	// Number of times completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletionCount = 0;

	FForge2DialogueRuntimeState()
		: bCompleted(false)
		, StartTime(0.0f)
		, TotalTime(0.0f)
		, CompletionCount(0)
	{
	}
};

// ============================================================================
// DIALOGUE MANAGER
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2DialogueManager
{
public:
	// Dialogue tree definitions
	TMap<FString, FForge2DialogueTreeDefinition> DialogueTrees;

	// Runtime states
	TMap<FString, FForge2DialogueRuntimeState> RuntimeStates;

	// Currently active dialogue
	FString ActiveDialogueId;

	// Is dialogue currently running?
	bool bDialogueActive = false;

	// Game state reference (for condition checking)
	TMap<FString, int32>* GameState = nullptr;

	FForge2DialogueManager()
		: bDialogueActive(false)
		, GameState(nullptr)
	{
	}

	// Set game state reference
	void SetGameState(TMap<FString, int32>* InGameState) { GameState = InGameState; }

	// Register a dialogue tree
	bool RegisterDialogueTree(const FForge2DialogueTreeDefinition& Tree);

	// Check if dialogue is available
	bool IsDialogueAvailable(const FString& TreeId) const;

	// Start a dialogue
	bool StartDialogue(const FString& TreeId);

	// End current dialogue
	void EndDialogue();

	// Get current node
	const FForge2DialogueNode* GetCurrentNode() const;

	// Advance to next node
	bool AdvanceDialogue(int32 ChoiceIndex = -1);

	// Get available choices for current node
	TArray<FForge2DialogueChoice> GetAvailableChoices() const;

	// Select a choice
	bool SelectChoice(int32 ChoiceIndex);

	// Skip current node
	bool SkipCurrentNode();

	// Check if current node is skippable
	bool IsCurrentNodeSkippable() const;

	// Get dialogue state
	const FForge2DialogueRuntimeState* GetDialogueState(const FString& TreeId) const;

	// Reset dialogue state
	void ResetDialogueState(const FString& TreeId);

	// Get all dialogues for a speaker
	TArray<FString> GetDialoguesForSpeaker(const FString& SpeakerId) const;

	// Get highest priority available dialogue for a speaker
	FString GetBestDialogueForSpeaker(const FString& SpeakerId) const;

	// Mark node as visited
	void MarkNodeVisited(const FString& TreeId, const FString& NodeId);

	// Has node been visited?
	bool HasNodeBeenVisited(const FString& TreeId, const FString& NodeId) const;
};

// ============================================================================
// DIALOGUE UTILITY FUNCTIONS
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2DialogueUtils
{
public:
	// Create a simple linear dialogue
	static FForge2DialogueTreeDefinition CreateLinearDialogue(
		const FString& TreeId,
		const FString& SpeakerId,
		const TArray<FText>& Lines);

	// Create a dialogue with a single choice branch
	static FForge2DialogueTreeDefinition CreateBranchingDialogue(
		const FString& TreeId,
		const FString& SpeakerId,
		const FText& Question,
		const TArray<FForge2DialogueChoice>& Choices);

	// Create a simple yes/no dialogue
	static FForge2DialogueTreeDefinition CreateYesNoDialogue(
		const FString& TreeId,
		const FString& SpeakerId,
		const FText& Question,
		const FText& YesResponse,
		const FText& NoResponse);

	// Create a shop greeting dialogue
	static FForge2DialogueTreeDefinition CreateShopDialogue(
		const FString& TreeId,
		const FString& ShopkeeperId,
		const FText& Greeting,
		const FString& ShopId);

	// Create a quest-giving dialogue
	static FForge2DialogueTreeDefinition CreateQuestDialogue(
		const FString& TreeId,
		const FString& QuestGiverId,
		const FText& QuestIntro,
		const FString& QuestId,
		const FText& AcceptResponse,
		const FText& DeclineResponse);

	// Generate a speaker profile
	static FForge2SpeakerProfile CreateSpeakerProfile(
		const FString& SpeakerId,
		const FText& DisplayName,
		bool bIsPlayer = false);

	// Generate a simple speech node
	static FForge2DialogueNode CreateSpeechNode(
		const FString& NodeId,
		const FString& SpeakerId,
		const FText& DialogueText,
		const FString& NextNodeId);

	// Generate a choice node
	static FForge2DialogueNode CreateChoiceNode(
		const FString& NodeId,
		const TArray<FForge2DialogueChoice>& Choices);

	// Generate a condition branch node
	static FForge2DialogueNode CreateConditionNode(
		const FString& NodeId,
		const FForge2DialogueCondition& Condition,
		const FString& TrueNode,
		const FString& FalseNode);

	// Generate an event node
	static FForge2DialogueNode CreateEventNode(
		const FString& NodeId,
		const FString& EventName,
		const FString& NextNodeId);

	// Serialize dialogue state for save
	static FString SerializeDialogueState(const FForge2DialogueRuntimeState& State);

	// Deserialize dialogue state from save
	static FForge2DialogueRuntimeState DeserializeDialogueState(const FString& Data);

	// Parse dialogue script format
	static FForge2DialogueTreeDefinition ParseDialogueScript(const FString& Script);

	// Export dialogue to script format
	static FString ExportDialogueScript(const FForge2DialogueTreeDefinition& Tree);
};
