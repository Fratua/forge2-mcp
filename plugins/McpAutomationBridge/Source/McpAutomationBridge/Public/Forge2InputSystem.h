// Forge2InputSystem.h
// FORGE2 Enhanced Input System
// Comprehensive input handling with combo detection, input buffering, and contextual actions

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "GameplayTagContainer.h"
#include "Forge2InputSystem.generated.h"

//=============================================================================
// ENUMS
//=============================================================================

UENUM(BlueprintType)
enum class EForge2InputTriggerType : uint8
{
	Pressed UMETA(DisplayName = "Pressed"),
	Released UMETA(DisplayName = "Released"),
	Held UMETA(DisplayName = "Held"),
	Tap UMETA(DisplayName = "Tap"),
	DoubleTap UMETA(DisplayName = "Double Tap"),
	Hold UMETA(DisplayName = "Hold (Duration)"),
	Chord UMETA(DisplayName = "Chord (Multiple Keys)")
};

UENUM(BlueprintType)
enum class EForge2InputModifierType : uint8
{
	None UMETA(DisplayName = "None"),
	DeadZone UMETA(DisplayName = "Dead Zone"),
	Scale UMETA(DisplayName = "Scale"),
	Negate UMETA(DisplayName = "Negate"),
	Smooth UMETA(DisplayName = "Smooth"),
	ResponseCurve UMETA(DisplayName = "Response Curve"),
	Swizzle UMETA(DisplayName = "Swizzle Axis"),
	Clamp UMETA(DisplayName = "Clamp")
};

UENUM(BlueprintType)
enum class EForge2InputPriority : uint8
{
	Low UMETA(DisplayName = "Low"),
	Normal UMETA(DisplayName = "Normal"),
	High UMETA(DisplayName = "High"),
	Critical UMETA(DisplayName = "Critical"),
	Override UMETA(DisplayName = "Override")
};

UENUM(BlueprintType)
enum class EForge2GestureType : uint8
{
	None UMETA(DisplayName = "None"),
	SwipeUp UMETA(DisplayName = "Swipe Up"),
	SwipeDown UMETA(DisplayName = "Swipe Down"),
	SwipeLeft UMETA(DisplayName = "Swipe Left"),
	SwipeRight UMETA(DisplayName = "Swipe Right"),
	Circle UMETA(DisplayName = "Circle"),
	Tap UMETA(DisplayName = "Tap"),
	DoubleTap UMETA(DisplayName = "Double Tap"),
	LongPress UMETA(DisplayName = "Long Press"),
	Pinch UMETA(DisplayName = "Pinch"),
	Spread UMETA(DisplayName = "Spread")
};

UENUM(BlueprintType)
enum class EForge2FightingInputType : uint8
{
	// Standard directions
	Neutral UMETA(DisplayName = "Neutral (5)"),
	Up UMETA(DisplayName = "Up (8)"),
	Down UMETA(DisplayName = "Down (2)"),
	Back UMETA(DisplayName = "Back (4)"),
	Forward UMETA(DisplayName = "Forward (6)"),
	UpBack UMETA(DisplayName = "Up-Back (7)"),
	UpForward UMETA(DisplayName = "Up-Forward (9)"),
	DownBack UMETA(DisplayName = "Down-Back (1)"),
	DownForward UMETA(DisplayName = "Down-Forward (3)"),

	// Buttons
	LightPunch UMETA(DisplayName = "Light Punch"),
	MediumPunch UMETA(DisplayName = "Medium Punch"),
	HeavyPunch UMETA(DisplayName = "Heavy Punch"),
	LightKick UMETA(DisplayName = "Light Kick"),
	MediumKick UMETA(DisplayName = "Medium Kick"),
	HeavyKick UMETA(DisplayName = "Heavy Kick"),
	Special UMETA(DisplayName = "Special"),
	Grab UMETA(DisplayName = "Grab")
};

//=============================================================================
// DATA STRUCTURES - Input Actions
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InputBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey Key = EKeys::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey GamepadKey = EKeys::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FKey> ChordKeys;  // Keys that must be held with main key

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNegate = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InputModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2InputModifierType Type = EForge2InputModifierType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value1 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value2 = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UCurveFloat> ResponseCurve;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InputTrigger
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2InputTriggerType Type = EForge2InputTriggerType::Pressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HoldThreshold = 0.2f;  // For held triggers

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TapWindow = 0.15f;  // Max time for tap

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DoubleTapWindow = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HoldDuration = 0.5f;  // For hold trigger
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InputActionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2InputBinding> Bindings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2InputModifier> Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2InputTrigger> Triggers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2InputPriority Priority = EForge2InputPriority::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConsumeInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer BlockedByTags;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InputMappingContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ContextId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ContextName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2InputActionDefinition> Actions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBlocksLowerPriority = false;
};

//=============================================================================
// DATA STRUCTURES - Input Buffer
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BufferedInput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ActionId;

	UPROPERTY(BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float Value = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector2D Direction = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	bool bWasHeld = false;

	UPROPERTY(BlueprintReadWrite)
	float HoldDuration = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InputBufferConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BufferDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxBufferSize = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrioritizeRecent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClearOnConsume = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> BufferableActions;  // Empty = all actions bufferable
};

//=============================================================================
// DATA STRUCTURES - Combo/Sequence Detection
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComboInputStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RequiredActions;  // Any of these

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RequiredSimultaneous;  // All of these at same time

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTimeWindow = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireRelease = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireHold = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinHoldDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D DirectionRequirement = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DirectionTolerance = 45.0f;  // Degrees
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComboSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComboId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComboName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2ComboInputStep> Steps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OutputAction;  // Action to trigger on completion

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeLimit = 0.0f;  // 0 = no limit

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;  // Higher priority combos checked first

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPartialMatch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer RequiredTags;
};

//=============================================================================
// DATA STRUCTURES - Fighting Game Inputs
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FightingInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2FightingInputType InputType = EForge2FightingInputType::Neutral;

	UPROPERTY(BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	int32 FrameNumber = 0;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FightingMotion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MotionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MotionName;

	// Predefined motions (QCF = Down, DownForward, Forward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EForge2FightingInputType> DirectionSequence;

	// Button to finish motion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EForge2FightingInputType> FinisherButtons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 0.5f;  // Time to complete motion

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowShortcuts = true;  // Allow DF for QCF motion

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireNeutralStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OutputAction;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ChargeInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChargeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2FightingInputType ChargeDirection = EForge2FightingInputType::Back;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2FightingInputType ReleaseDirection = EForge2FightingInputType::Forward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EForge2FightingInputType> FinisherButtons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinChargeTime = 0.5f;  // 30 frames at 60fps

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxReleaseWindow = 0.2f;  // Time after release to press button

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OutputAction;
};

//=============================================================================
// DATA STRUCTURES - Contextual Actions
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ContextCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery TagQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireGrounded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireIdle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireTargetLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireNearInteractable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RequiredEquipment;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ContextualAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourceInputAction;  // The base input that triggers check

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OutputAction;  // Action to trigger if conditions met

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2ContextCondition Condition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PromptText;  // For UI display

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> PromptIcon;
};

//=============================================================================
// DATA STRUCTURES - Gesture Detection
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GesturePoint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GestureDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GestureId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2GestureType GestureType = EForge2GestureType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 50.0f;  // For swipes

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DirectionTolerance = 30.0f;  // Degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OutputAction;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GestureResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bRecognized = false;

	UPROPERTY(BlueprintReadWrite)
	EForge2GestureType GestureType = EForge2GestureType::None;

	UPROPERTY(BlueprintReadWrite)
	FString MatchedGestureId;

	UPROPERTY(BlueprintReadWrite)
	FVector2D Direction = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float Magnitude = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector2D StartPosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector2D EndPosition = FVector2D::ZeroVector;
};

//=============================================================================
// INPUT SYSTEM CLASSES
//=============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2InputBuffer
{
public:
	// Configuration
	void Configure(const FForge2InputBufferConfig& Config);

	// Buffer management
	void AddInput(const FForge2BufferedInput& Input);
	void Clear();
	void Update(float CurrentTime);

	// Query
	bool HasInput(const FString& ActionId) const;
	TOptional<FForge2BufferedInput> GetInput(const FString& ActionId) const;
	TOptional<FForge2BufferedInput> ConsumeInput(const FString& ActionId);
	TArray<FForge2BufferedInput> GetAllInputs() const;

	// Get most recent input within buffer window
	TOptional<FForge2BufferedInput> GetMostRecent(float CurrentTime) const;

private:
	FForge2InputBufferConfig BufferConfig;
	TArray<FForge2BufferedInput> Buffer;
};

class MCPAUTOMATIONBRIDGE_API FForge2ComboDetector
{
public:
	// Configuration
	void AddCombo(const FForge2ComboSequence& Combo);
	void RemoveCombo(const FString& ComboId);
	void ClearCombos();

	// Detection
	void ProcessInput(const FForge2BufferedInput& Input, float CurrentTime, const FGameplayTagContainer& CurrentTags);
	TOptional<FString> CheckForCompletedCombo();

	// State
	void Reset();
	float GetComboProgress(const FString& ComboId) const;  // 0-1 progress
	TArray<FString> GetActivePartialCombos() const;

private:
	struct FComboProgress
	{
		FString ComboId;
		int32 CurrentStep;
		float StartTime;
		float LastInputTime;
	};

	TArray<FForge2ComboSequence> RegisteredCombos;
	TArray<FComboProgress> ActiveCombos;
	TOptional<FString> CompletedCombo;

	bool CheckStepMatch(const FForge2ComboInputStep& Step, const FForge2BufferedInput& Input) const;
};

class MCPAUTOMATIONBRIDGE_API FForge2FightingInputProcessor
{
public:
	// Configuration
	void AddMotion(const FForge2FightingMotion& Motion);
	void AddChargeInput(const FForge2ChargeInput& ChargeInput);

	// Processing
	void ProcessInput(EForge2FightingInputType Input, float CurrentTime, int32 FrameNumber);
	TOptional<FString> CheckForMotion();
	TOptional<FString> CheckForCharge();

	// State
	void Reset();
	void ClearInputHistory();
	TArray<FForge2FightingInput> GetInputHistory(float TimeWindow) const;

	// Charge state
	bool IsCharging(const FString& ChargeId) const;
	float GetChargeTime(const FString& ChargeId) const;

private:
	struct FChargeState
	{
		FString ChargeId;
		float StartTime;
		bool bFullyCharged;
	};

	TArray<FForge2FightingMotion> Motions;
	TArray<FForge2ChargeInput> ChargeInputs;
	TArray<FForge2FightingInput> InputHistory;
	TArray<FChargeState> ActiveCharges;

	static constexpr int32 MaxInputHistorySize = 60;  // 1 second at 60fps

	bool CheckMotionMatch(const FForge2FightingMotion& Motion, float CurrentTime) const;
	EForge2FightingInputType DirectionFromStick(FVector2D StickInput) const;
};

class MCPAUTOMATIONBRIDGE_API FForge2ContextManager
{
public:
	// Configuration
	void AddContextualAction(const FForge2ContextualAction& Action);
	void RemoveContextualAction(const FString& ActionId);
	void ClearContextualActions();

	// Evaluation
	FString EvaluateContext(
		const FString& InputAction,
		const FGameplayTagContainer& Tags,
		bool bGrounded,
		bool bMoving,
		bool bTargetLocked,
		bool bNearInteractable,
		float HealthPercent,
		const TArray<FString>& Equipment
	);

	// Get prompt for current context
	TOptional<FForge2ContextualAction> GetActivePrompt(const FString& InputAction) const;

private:
	TArray<FForge2ContextualAction> ContextualActions;

	bool EvaluateCondition(
		const FForge2ContextCondition& Condition,
		const FGameplayTagContainer& Tags,
		bool bGrounded,
		bool bMoving,
		bool bTargetLocked,
		bool bNearInteractable,
		float HealthPercent,
		const TArray<FString>& Equipment
	) const;
};

class MCPAUTOMATIONBRIDGE_API FForge2GestureRecognizer
{
public:
	// Configuration
	void AddGesture(const FForge2GestureDefinition& Gesture);
	void RemoveGesture(const FString& GestureId);

	// Processing
	void BeginTouch(const FVector2D& Position, float Timestamp);
	void UpdateTouch(const FVector2D& Position, float Timestamp);
	FForge2GestureResult EndTouch(const FVector2D& Position, float Timestamp);

	// Multi-touch
	void BeginSecondTouch(const FVector2D& Position, float Timestamp);
	void UpdateSecondTouch(const FVector2D& Position, float Timestamp);
	FForge2GestureResult EndSecondTouch(const FVector2D& Position, float Timestamp);

	// State
	void Reset();

private:
	TArray<FForge2GestureDefinition> GestureDefinitions;
	TArray<FForge2GesturePoint> TouchPath;
	TArray<FForge2GesturePoint> SecondTouchPath;
	float LastTapTime = 0.0f;
	FVector2D LastTapPosition = FVector2D::ZeroVector;

	EForge2GestureType RecognizeGesture(const TArray<FForge2GesturePoint>& Path);
	FVector2D CalculateSwipeDirection(const TArray<FForge2GesturePoint>& Path);
	bool IsCircle(const TArray<FForge2GesturePoint>& Path);
};

//=============================================================================
// UTILITY CLASS
//=============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2InputUtils
{
public:
	// Key display names
	static FString GetKeyDisplayName(const FKey& Key);
	static FString GetGamepadButtonIcon(const FKey& Key);

	// Direction calculation
	static FVector2D GetDirectionFromInput(float AxisX, float AxisY, float DeadZone = 0.2f);
	static EForge2FightingInputType GetNumpadDirection(FVector2D StickInput, float DeadZone = 0.4f);

	// Input comparison
	static bool AreKeysEquivalent(const FKey& Key1, const FKey& Key2);
	static bool IsAxisInput(const FKey& Key);
	static bool IsGamepadKey(const FKey& Key);

	// Combo notation parsing (e.g., "236P" for QCF+Punch)
	static TArray<EForge2FightingInputType> ParseFightingNotation(const FString& Notation);
	static FString ToFightingNotation(const TArray<EForge2FightingInputType>& Inputs);

	// Create standard fighting game motions
	static FForge2FightingMotion CreateQCF(const FString& Name, const TArray<EForge2FightingInputType>& Buttons);
	static FForge2FightingMotion CreateQCB(const FString& Name, const TArray<EForge2FightingInputType>& Buttons);
	static FForge2FightingMotion CreateDP(const FString& Name, const TArray<EForge2FightingInputType>& Buttons);
	static FForge2FightingMotion CreateHCF(const FString& Name, const TArray<EForge2FightingInputType>& Buttons);
	static FForge2FightingMotion CreateHCB(const FString& Name, const TArray<EForge2FightingInputType>& Buttons);
	static FForge2FightingMotion Create360(const FString& Name, const TArray<EForge2FightingInputType>& Buttons);
	static FForge2ChargeInput CreateChargeBack(const FString& Name, const TArray<EForge2FightingInputType>& Buttons, float ChargeTime = 0.5f);
	static FForge2ChargeInput CreateChargeDown(const FString& Name, const TArray<EForge2FightingInputType>& Buttons, float ChargeTime = 0.5f);

	// Input modifier application
	static float ApplyDeadZone(float Value, float InnerDeadZone, float OuterDeadZone = 1.0f);
	static FVector2D ApplyDeadZone2D(FVector2D Value, float InnerDeadZone, float OuterDeadZone = 1.0f);
	static float ApplyResponseCurve(float Value, UCurveFloat* Curve);
};

#endif // Include guard handled by pragma once
