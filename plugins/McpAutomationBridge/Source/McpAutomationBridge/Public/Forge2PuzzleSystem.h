// Forge2PuzzleSystem.h
// FORGE2 Puzzle Genre System
// Provides puzzle mechanics: logic gates, physics puzzles, sequences, environmental puzzles

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Forge2PuzzleSystem.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EForge2PuzzleElementType : uint8
{
    Button,             // Pressable button
    Lever,              // Toggle lever
    Switch,             // On/off switch
    PressurePlate,      // Weight-activated plate
    Door,               // Openable door
    Gate,               // Larger gate/barrier
    Platform,           // Moveable platform
    Laser,              // Laser beam
    Mirror,             // Laser reflector
    Prism,              // Light splitter
    Portal,             // Teleporter
    Cube,               // Moveable physics cube
    Orb,                // Energy orb
    Receptor,           // Energy receiver
    Emitter,            // Energy/laser source
    Bridge,             // Extendable bridge
    Elevator,           // Vertical platform
    Fan,                // Wind/propulsion
    Magnet,             // Magnetic field
    TimerSwitch,        // Timed activation
    SequenceSwitch,     // Part of sequence
    RotatingPlatform,   // Rotating surface
    Conveyor,           // Moving belt
    Water,              // Water body
    Drain,              // Water drain
    Valve,              // Water flow control
    Custom              // Custom puzzle element
};

UENUM(BlueprintType)
enum class EForge2LogicGateType : uint8
{
    AND,                // All inputs true
    OR,                 // Any input true
    NOT,                // Invert input
    XOR,                // Exclusive or
    NAND,               // NOT AND
    NOR,                // NOT OR
    XNOR,               // NOT XOR
    Buffer,             // Pass-through
    Latch,              // SR latch
    Toggle,             // Flip on trigger
    Delay,              // Delayed output
    Pulse,              // Single pulse output
    Counter,            // Count triggers
    Comparator,         // Compare values
    Memory              // Store value
};

UENUM(BlueprintType)
enum class EForge2PuzzleState : uint8
{
    Inactive,           // Not activated
    Active,             // Currently active
    Locked,             // Cannot change state
    Solving,            // Being solved
    Solved,             // Puzzle complete
    Failed,             // Failed (resettable)
    Transitioning       // State changing
};

UENUM(BlueprintType)
enum class EForge2PhysicsPuzzleType : uint8
{
    Balance,            // Weight balance
    Trajectory,         // Projectile path
    Momentum,           // Conservation of momentum
    Buoyancy,           // Floating/sinking
    Pressure,           // Fluid pressure
    Pendulum,           // Swinging
    Pulley,             // Rope and pulley
    Lever,              // Fulcrum and lever
    Ramp,               // Inclined plane
    Friction,           // Surface friction
    Magnetic,           // Magnetic attraction/repulsion
    Gravity,            // Gravity manipulation
    Wind,               // Air pressure/flow
    Elastic,            // Springs and bounce
    Chain,              // Connected objects
    Fluid               // Liquid flow
};

UENUM(BlueprintType)
enum class EForge2PuzzleResetMode : uint8
{
    Manual,             // Player must reset
    Automatic,          // Auto-reset on failure
    Timed,              // Reset after time
    OnExit,             // Reset when leaving area
    Never,              // Never resets (permanent)
    Checkpoint          // Reset to last checkpoint
};

UENUM(BlueprintType)
enum class EForge2HintLevel : uint8
{
    None,               // No hint
    Subtle,             // Visual cue only
    Moderate,           // General direction
    Obvious,            // Clear instruction
    Solution            // Full solution
};

// ============================================================================
// Logic System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LogicInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InputID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SourceElementID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInverted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCurrentValue = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LogicGate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName GateID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2LogicGateType GateType = EForge2LogicGateType::AND;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2LogicInput> Inputs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOutputValue = false;

    // For Delay gate
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DelayTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DelayTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPendingValue = false;

    // For Counter gate
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CounterTarget = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentCount = 0;

    // For Pulse gate
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PulseDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PulseTimer = 0.0f;

    // For Memory gate
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StoredValue = 0.0f;

    bool Evaluate();
    void Update(float DeltaTime);
    void Reset();
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LogicNetwork
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName NetworkID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2LogicGate> Gates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, bool> ElementStates;

    void UpdateNetwork(float DeltaTime);
    void SetElementState(FName ElementID, bool bState);
    bool GetElementState(FName ElementID) const;
    bool GetGateOutput(FName GateID) const;
    void Reset();
};

// ============================================================================
// Puzzle Elements
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PuzzleElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ElementID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PuzzleElementType ElementType = EForge2PuzzleElementType::Button;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PuzzleState CurrentState = EForge2PuzzleState::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInteractable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresItem = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InteractionTime = 0.0f;           // 0 = instant

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTransitionTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> OutputConnections;        // Elements this activates

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName LogicGateID;                      // Connected logic gate

    // Type-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RequiredWeight = 50.0f;           // For pressure plates

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimerDuration = 5.0f;             // For timed switches

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MovementRange = FVector(0, 0, 300);  // For platforms/elevators

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RotationAngle = 90.0f;            // For rotating elements

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LaserDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LaserLength = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> Material;

    void Activate();
    void Deactivate();
    void Toggle();
    bool CanInteract() const;
    FVector GetTargetLocation() const;
    FRotator GetTargetRotation() const;
};

// ============================================================================
// Physics Puzzles
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PhysicsObject
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ObjectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Mass = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density = 1.0f;                   // For buoyancy

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Friction = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Restitution = 0.3f;               // Bounciness

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGrabbable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMagnetic = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MagneticPolarity = 1.0f;          // Positive or negative

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAffectedByGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAffectedByWind = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Size = FVector(50.0f);

    void ApplyForce(const FVector& Force);
    void ApplyImpulse(const FVector& Impulse);
    float GetBuoyancyForce(float WaterDensity, float WaterLevel) const;
    float GetMagneticForce(const FForge2PhysicsObject& Other, float MagneticStrength) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PhysicsPuzzle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PuzzleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PhysicsPuzzleType PuzzleType = EForge2PhysicsPuzzleType::Balance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PhysicsObject> Objects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector GravityDirection = FVector(0, 0, -1);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GravityStrength = 980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WindDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MagneticFieldStrength = 0.0f;

    // Solution conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredObjectPositions;  // Object IDs in target zones

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> TargetZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetZoneRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BalanceThreshold = 5.0f;          // For balance puzzles

    bool CheckSolved() const;
    void UpdatePhysics(float DeltaTime);
    void Reset();
};

// ============================================================================
// Sequence Puzzles
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SequenceStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StepID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredElementID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StepOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxTimeToComplete = 0.0f;         // 0 = no limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CompletionTime = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SequencePuzzle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PuzzleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2SequenceStep> Sequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStep = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowMistakes = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxMistakes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentMistakes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowProgress = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlaybackOnSolve = false;          // Replay sequence visually

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PuzzleState State = EForge2PuzzleState::Inactive;

    bool ProcessInput(FName ElementID, float CurrentTime);
    bool CheckSolved() const;
    void Reset();
    int32 GetProgress() const { return CurrentStep; }
    int32 GetTotalSteps() const { return Sequence.Num(); }
};

// ============================================================================
// Laser/Light Puzzles
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LaserBeam
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Origin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Direction = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor Color = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxLength = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Width = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxReflections = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PathPoints;             // Calculated path including reflections

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> HitReceptors;             // Receptors this beam hits

    void CalculatePath(const TArray<FForge2PuzzleElement>& Elements);
    bool HitsReceptor(FName ReceptorID) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LaserPuzzle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PuzzleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2LaserBeam> Beams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PuzzleElement> Mirrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PuzzleElement> Prisms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredReceptors;        // All must be hit to solve

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bColorMatching = false;            // Receptors need specific colors

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FLinearColor> ReceptorColors;

    void UpdateBeams();
    bool CheckSolved() const;
    void RotateMirror(FName MirrorID, float Angle);
};

// ============================================================================
// Environmental Puzzles
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2EnvironmentalCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ConditionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ConditionType;                  // "perspective", "shadow", "reflection", "time", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RequiredPlayerPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PositionTolerance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RequiredPlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RotationTolerance = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RequiredTimeOfDay = 0.0f;         // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeTolerance = 1.0f;             // Hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMet = false;

    bool CheckCondition(const FVector& PlayerPos, const FRotator& PlayerRot, float CurrentTime, FName CurrentWeather) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2EnvironmentalPuzzle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PuzzleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2EnvironmentalCondition> Conditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllConditionsRequired = true;     // AND vs OR

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HoldDuration = 0.0f;              // Time conditions must be held

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentHoldTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PuzzleState State = EForge2PuzzleState::Inactive;

    bool CheckSolved(const FVector& PlayerPos, const FRotator& PlayerRot, float CurrentTime, FName CurrentWeather, float DeltaTime);
    void Reset();
};

// ============================================================================
// Hint System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PuzzleHint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HintID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2HintLevel HintLevel = EForge2HintLevel::Subtle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HintText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HintLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetElementID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bVisual = true;                    // Show visual indicator

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAudio = false;                    // Play audio cue

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;                  // How long hint shows

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime = 60.0f;             // Time before hint can show again

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastShownTime = -1000.0f;

    bool CanShow(float CurrentTime) const;
    void Show(float CurrentTime);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HintSystem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PuzzleHint> Hints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentHintIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bProgressiveHints = true;          // Show hints in order

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HintTriggerTime = 120.0f;         // Seconds without progress to trigger hint

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoHints = true;                 // Automatically show hints

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ManualHintsUsed = 0;

    void Update(float DeltaTime, bool bMadeProgress);
    FForge2PuzzleHint* GetNextHint(float CurrentTime);
    void RequestHint(float CurrentTime);
    void ResetProgress();
};

// ============================================================================
// Puzzle Room/Level
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PuzzleDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PuzzleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PuzzleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PuzzleElement> Elements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2LogicNetwork LogicNetwork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PhysicsPuzzle> PhysicsPuzzles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2SequencePuzzle> SequencePuzzles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2LaserPuzzle> LaserPuzzles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2EnvironmentalPuzzle> EnvironmentalPuzzles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2HintSystem HintSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PuzzleResetMode ResetMode = EForge2PuzzleResetMode::Manual;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ResetDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMultipleSolutions = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Difficulty = 1;                   // 1-5

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ParTime = 60.0f;                  // Expected solve time

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PuzzleState OverallState = EForge2PuzzleState::Inactive;
};

// ============================================================================
// Player Progress
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PuzzleProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PuzzleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSolved = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SolveTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AttemptCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HintsUsed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalTimeSpent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ResetCount = 0;

    int32 GetStarRating(float ParTime) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PuzzlePlayerState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FForge2PuzzleProgress> PuzzleProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalPuzzlesSolved = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalHintsUsed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalPuzzleTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentHeldObject;                // For physics puzzles

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> InventoryItems;           // Puzzle-specific items

    void RecordSolve(FName PuzzleID, float SolveTime, int32 HintsUsed);
    bool HasSolved(FName PuzzleID) const;
    FForge2PuzzleProgress GetProgress(FName PuzzleID) const;
};

// ============================================================================
// Puzzle Manager
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2PuzzleManager : public UObject
{
    GENERATED_BODY()

public:
    UForge2PuzzleManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2PuzzleDefinition CurrentPuzzle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2PuzzlePlayerState PlayerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PuzzleStartTime = 0.0f;

    // Initialization
    UFUNCTION(BlueprintCallable)
    void InitializePuzzle(const FForge2PuzzleDefinition& Puzzle);

    // Update
    UFUNCTION(BlueprintCallable)
    void UpdatePuzzle(float DeltaTime);

    // Element interaction
    UFUNCTION(BlueprintCallable)
    bool InteractWithElement(FName ElementID);

    UFUNCTION(BlueprintCallable)
    bool SetElementState(FName ElementID, bool bActive);

    UFUNCTION(BlueprintCallable)
    void RotateElement(FName ElementID, float Angle);

    // Physics
    UFUNCTION(BlueprintCallable)
    bool GrabObject(FName ObjectID);

    UFUNCTION(BlueprintCallable)
    void ReleaseObject();

    UFUNCTION(BlueprintCallable)
    void ThrowObject(const FVector& Direction, float Force);

    // Queries
    UFUNCTION(BlueprintCallable)
    FForge2PuzzleElement GetElement(FName ElementID) const;

    UFUNCTION(BlueprintCallable)
    bool IsElementActive(FName ElementID) const;

    UFUNCTION(BlueprintCallable)
    bool IsPuzzleSolved() const;

    // Hints
    UFUNCTION(BlueprintCallable)
    FForge2PuzzleHint RequestHint();

    UFUNCTION(BlueprintCallable)
    void SkipPuzzle();

    // Reset
    UFUNCTION(BlueprintCallable)
    void ResetPuzzle();

    UFUNCTION(BlueprintCallable)
    void ResetElement(FName ElementID);

    // Solution validation
    UFUNCTION(BlueprintCallable)
    bool ValidateSolution();

private:
    void UpdateLogicNetwork(float DeltaTime);
    void UpdatePhysicsPuzzles(float DeltaTime);
    void UpdateSequencePuzzles(float DeltaTime);
    void UpdateLaserPuzzles();
    void UpdateEnvironmentalPuzzles(float DeltaTime, const FVector& PlayerPos, const FRotator& PlayerRot);
    void CheckOverallSolved();
    void OnPuzzleSolved();
    void OnPuzzleFailed();
};

// ============================================================================
// Utility Functions
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2PuzzleUtilities : public UObject
{
    GENERATED_BODY()

public:
    // Element presets
    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreateButton(const FVector& Location, FName OutputConnection);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreateLever(const FVector& Location, FName OutputConnection);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreatePressurePlate(const FVector& Location, float RequiredWeight, FName OutputConnection);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreateDoor(const FVector& Location, const FVector& OpenOffset);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreateMovingPlatform(const FVector& Start, const FVector& End, float Speed);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreateLaserEmitter(const FVector& Location, const FVector& Direction);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreateMirror(const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreateReceptor(const FVector& Location);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleElement CreateTimedSwitch(const FVector& Location, float Duration, FName OutputConnection);

    // Logic gate presets
    UFUNCTION(BlueprintCallable)
    static FForge2LogicGate CreateANDGate(const TArray<FName>& InputElements);

    UFUNCTION(BlueprintCallable)
    static FForge2LogicGate CreateORGate(const TArray<FName>& InputElements);

    UFUNCTION(BlueprintCallable)
    static FForge2LogicGate CreateDelayGate(FName InputElement, float Delay);

    UFUNCTION(BlueprintCallable)
    static FForge2LogicGate CreateCounterGate(FName InputElement, int32 TargetCount);

    // Physics object presets
    UFUNCTION(BlueprintCallable)
    static FForge2PhysicsObject CreateCube(const FVector& Location, float Size, float Mass);

    UFUNCTION(BlueprintCallable)
    static FForge2PhysicsObject CreateSphere(const FVector& Location, float Radius, float Mass);

    UFUNCTION(BlueprintCallable)
    static FForge2PhysicsObject CreateMagneticObject(const FVector& Location, float Mass, float Polarity);

    // Puzzle presets
    UFUNCTION(BlueprintCallable)
    static FForge2SequencePuzzle CreateSimonSaysPuzzle(const TArray<FName>& ElementSequence);

    UFUNCTION(BlueprintCallable)
    static FForge2LaserPuzzle CreateBasicLaserPuzzle(const FVector& EmitterLocation, const FVector& ReceptorLocation);

    UFUNCTION(BlueprintCallable)
    static FForge2PhysicsPuzzle CreateBalancePuzzle(const FVector& FulcrumLocation, float BalanceThreshold);

    // Hint presets
    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleHint CreateSubtleHint(FName TargetElement);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleHint CreateTextHint(const FString& HintText);

    UFUNCTION(BlueprintCallable)
    static FForge2PuzzleHint CreateSolutionHint(const FString& SolutionText);
};
