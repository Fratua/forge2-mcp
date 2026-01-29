// Forge2StealthSystem.h
// FORGE2 Stealth Genre System
// Provides stealth mechanics: detection, AI awareness, patrols, distractions, cover, disguises

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Forge2StealthSystem.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EForge2AwarenessState : uint8
{
    Unaware,        // Normal behavior, no knowledge of player
    Suspicious,     // Heard something, investigating
    Alerted,        // Knows player is nearby, actively searching
    Combat,         // In direct combat with player
    Searching,      // Lost sight, searching last known location
    Returning,      // Returning to normal patrol after search
    Distracted      // Currently investigating a distraction
};

UENUM(BlueprintType)
enum class EForge2DetectionType : uint8
{
    Vision,         // Line of sight detection
    Hearing,        // Sound-based detection
    Touch,          // Physical contact
    Smell,          // Scent detection (dogs, etc.)
    Radar,          // Electronic detection
    Thermal,        // Heat signature detection
    Motion,         // Movement detection
    Proximity       // Distance-based detection
};

UENUM(BlueprintType)
enum class EForge2CoverType : uint8
{
    None,
    Low,            // Crouch cover
    High,           // Standing cover
    Corner,         // Corner peek
    Doorway,        // Door frame cover
    Vehicle,        // Behind vehicle
    Foliage,        // Bush/grass concealment
    Darkness        // Shadow concealment
};

UENUM(BlueprintType)
enum class EForge2DisguiseType : uint8
{
    None,
    Civilian,       // Blend with civilians
    Guard,          // Enemy guard uniform
    Worker,         // Maintenance/worker outfit
    Officer,        // High-ranking disguise
    Specialist,     // Special access (scientist, etc.)
    Custom          // Custom disguise
};

UENUM(BlueprintType)
enum class EForge2DistractionType : uint8
{
    Noise,          // Thrown object, whistle
    Visual,         // Flare, light
    Decoy,          // Fake target
    Environmental,  // Triggered environment (alarm, explosion)
    Social,         // Conversation, fake call
    Electronic      // Hacked device, radio
};

UENUM(BlueprintType)
enum class EForge2SecurityType : uint8
{
    Camera,         // Security camera
    Laser,          // Laser tripwire
    PressurePlate,  // Floor sensor
    MotionSensor,   // Motion detector
    Alarm,          // Manual/triggered alarm
    Guard,          // Human guard
    Dog,            // Guard dog
    Drone,          // Patrol drone
    Turret          // Automated turret
};

UENUM(BlueprintType)
enum class EForge2TakedownType : uint8
{
    NonLethal,      // Knockout
    Lethal,         // Kill
    Grab,           // Hold as shield
    Interrogate,    // Question for info
    Distract        // Use as distraction
};

UENUM(BlueprintType)
enum class EForge2NoiseLevel : uint8
{
    Silent,         // No noise
    Quiet,          // Footsteps on soft surface
    Normal,         // Normal walking
    Loud,           // Running, dropping items
    VeryLoud        // Gunshot, explosion
};

// ============================================================================
// Detection System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VisionCone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ViewAngle = 90.0f;                    // Horizontal field of view

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ViewDistance = 1500.0f;               // Maximum view distance

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PeripheralAngle = 160.0f;             // Wider peripheral (reduced detection)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PeripheralMultiplier = 0.3f;          // Detection rate in peripheral

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloseRangeDistance = 200.0f;          // Instant detection range

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeightTolerance = 300.0f;             // Vertical detection range

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRate = 1.0f;                 // Base detection speed

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LightSensitivity = 1.0f;              // How much light affects detection

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSensitivity = 1.5f;           // Bonus detection for moving targets

    // Check if a point is within the vision cone
    bool IsInVisionCone(const FVector& ObserverLocation, const FVector& ObserverForward, const FVector& TargetLocation) const;
    float GetDetectionMultiplier(const FVector& ObserverLocation, const FVector& ObserverForward, const FVector& TargetLocation) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HearingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HearingRange = 1000.0f;               // Maximum hearing distance

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SilentThreshold = 0.1f;               // Below this, sound is ignored

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float QuietThreshold = 0.3f;                // Quiet sounds

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NormalThreshold = 0.6f;               // Normal sounds

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LoudThreshold = 0.9f;                 // Loud sounds

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HearingAccuracy = 0.8f;               // How accurately direction is determined

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WallAttenuation = 0.5f;               // Sound reduction through walls

    // Calculate effective volume at distance
    float CalculatePerceivedVolume(float SourceVolume, float Distance, int32 WallCount) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DetectionMeter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentDetection = 0.0f;              // Current detection level (0-1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuspicionThreshold = 0.3f;            // Level to become suspicious

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertThreshold = 0.7f;                // Level to become alerted

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CombatThreshold = 1.0f;               // Level to enter combat

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayRate = 0.2f;                     // Detection decay per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayDelay = 2.0f;                    // Seconds before decay starts

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastDetectionTime = 0.0f;

    void AddDetection(float Amount);
    void UpdateDecay(float DeltaTime, float CurrentTime);
    EForge2AwarenessState GetAwarenessState() const;
};

// ============================================================================
// AI Awareness
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AIAwareness
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2AwarenessState CurrentState = EForge2AwarenessState::Unaware;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2DetectionMeter DetectionMeter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastKnownPlayerTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastHeardSoundLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastHeardSoundTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SearchDuration = 30.0f;               // How long to search

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuspicionDuration = 10.0f;            // How long to stay suspicious

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeInCurrentState = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanCallReinforcements = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasCalledReinforcements = false;

    void TransitionToState(EForge2AwarenessState NewState, float CurrentTime);
    void UpdateState(float DeltaTime, float CurrentTime);
    bool ShouldReturnToPatrol(float CurrentTime) const;
};

// ============================================================================
// Patrol System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator LookDirection = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaitTime = 2.0f;                      // Time to wait at this point

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLookAround = false;                   // Scan surroundings

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LookAroundAngle = 90.0f;              // Angle to look around

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AnimationToPlay;                    // Optional animation at point

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueToPlay;                     // Optional dialogue at point
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PatrolRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RouteID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = true;                       // Loop or ping-pong

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRandomize = false;                    // Randomize point order

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalkSpeed = 200.0f;                   // Speed while patrolling

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RunSpeed = 400.0f;                    // Speed when alerted

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bReversing = false;                    // For ping-pong

    FForge2PatrolPoint GetCurrentPoint() const;
    FForge2PatrolPoint GetNextPoint();
    void ResetRoute();
};

// ============================================================================
// Distraction System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Distraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DistractionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2DistractionType DistractionType = EForge2DistractionType::Noise;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectRadius = 500.0f;                // How far it attracts AI

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;                      // How long distraction lasts

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseLevel = 0.7f;                    // For audio distractions

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAffectedAI = 2;                    // Max AI that will investigate

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InvestigationTime = 10.0f;            // How long AI investigates

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOneShot = true;                       // Single use or repeatable

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTriggered = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TriggerTime = 0.0f;

    bool IsActive(float CurrentTime) const;
    float GetAttractionStrength(float Distance) const;
};

// ============================================================================
// Cover System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CoverPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CoverID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2CoverType CoverType = EForge2CoverType::Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator CoverDirection = FRotator::ZeroRotator;  // Direction cover faces

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoverHeight = 100.0f;                 // Height of cover

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoverWidth = 150.0f;                  // Width of cover

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanPeekLeft = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanPeekRight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanPeekOver = false;                  // For low cover

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBlindFire = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDestructible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOccupied = false;

    bool ProvidesCoverFrom(const FVector& ThreatLocation) const;
    float GetCoverEffectiveness(const FVector& ThreatLocation) const;
};

// ============================================================================
// Disguise System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Disguise
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DisguiseID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisguiseName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2DisguiseType DisguiseType = EForge2DisguiseType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AllowedAreas;                 // Areas this disguise can access

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> SuspiciousActions;            // Actions that raise suspicion

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendInRadius = 500.0f;               // Distance to blend with similar NPCs

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionMultiplier = 0.5f;           // Reduced detection rate

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuspicionBuildRate = 0.1f;            // How fast suspicion builds

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresID = false;                   // Needs ID check to pass

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> DisguiseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> DisguiseMaterial;

    bool CanAccessArea(FName AreaID) const;
    bool IsActionSuspicious(FName ActionID) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DisguiseState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2Disguise CurrentDisguise;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuspicionLevel = 0.0f;                // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlownThreshold = 1.0f;                // When disguise is blown

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> WitnessedBy;                  // NPCs who saw through disguise

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDisguiseBlown = false;

    void AddSuspicion(float Amount);
    void ClearSuspicion();
    bool IsDisguiseBlown() const { return bDisguiseBlown; }
};

// ============================================================================
// Security System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SecurityDevice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DeviceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2SecurityType SecurityType = EForge2SecurityType::Camera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHacked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDestroyed = false;

    // Camera specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CameraFOV = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CameraRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CameraSweepAngle = 90.0f;             // Rotation sweep

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CameraSweepSpeed = 30.0f;             // Degrees per second

    // Laser/sensor specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LaserEndPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SensorRadius = 100.0f;

    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTimed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActiveDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InactiveDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PhaseOffset = 0.0f;

    // Response
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTriggerAlarm = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlarmDelay = 2.0f;                    // Time before alarm triggers

    bool IsActiveAtTime(float Time) const;
    FRotator GetCameraRotationAtTime(float Time) const;
    bool CheckDetection(const FVector& TargetLocation, float Time) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AlarmState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAlarmActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlarmStartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlarmDuration = 60.0f;                // How long alarm lasts

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AlarmLevel = 0;                       // Escalation level

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAlarmLevel = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AlarmTriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLockdownActive = false;

    void TriggerAlarm(const FVector& Location, float CurrentTime);
    void EscalateAlarm();
    void DisableAlarm();
    bool IsAlarmExpired(float CurrentTime) const;
};

// ============================================================================
// Shadow/Light System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LightLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientLight = 0.3f;                  // Base ambient light

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DirectLight = 0.0f;                   // Light from light sources

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalLight = 0.3f;                    // Combined light level

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShadowThreshold = 0.3f;               // Below this is "in shadow"

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DarkThreshold = 0.15f;                // Below this is "in darkness"

    bool IsInShadow() const { return TotalLight < ShadowThreshold; }
    bool IsInDarkness() const { return TotalLight < DarkThreshold; }
    float GetVisibilityMultiplier() const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LightSource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName LightID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDestructible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFlickering = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlickerRate = 0.0f;

    float GetLightContribution(const FVector& TargetLocation) const;
};

// ============================================================================
// Takedown System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TakedownDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TakedownID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2TakedownType TakedownType = EForge2TakedownType::NonLethal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range = 150.0f;                       // Max distance to initiate

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Angle = 90.0f;                        // Cone behind target

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2NoiseLevel NoiseLevel = EForge2NoiseLevel::Quiet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 1.5f;                      // Animation duration

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresBehind = true;                // Must be behind target

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeInterrupted = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AnimationMontage;

    bool CanPerform(const FVector& PlayerLocation, const FVector& TargetLocation, const FVector& TargetForward) const;
};

// ============================================================================
// Body Handling
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BodyState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BodyID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAlive = false;                      // Unconscious or dead

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsBeingCarried = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasBeenDiscovered = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HidingSpotID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeUntilWakeUp = 0.0f;               // For unconscious (0 = permanent)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2Disguise Disguise;                   // Disguise that can be taken
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HidingSpot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SpotID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Capacity = 2;                         // How many bodies fit

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCloseable = true;                     // Can be closed (container)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsClosed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InteractionTime = 2.0f;               // Time to hide body

    bool CanHideBody() const { return CurrentCount < Capacity; }
};

// ============================================================================
// NPC Definition
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StealthNPCDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2VisionCone VisionCone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2HearingConfig HearingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2PatrolRoute PatrolRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2AIAwareness Awareness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2Disguise Disguise;                   // NPC's current disguise/uniform

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> CanDetectDisguises;           // Disguises this NPC can see through

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsEssential = false;                  // Cannot be killed

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDistracted = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeTakenDown = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReactionTime = 0.5f;                  // Delay before reacting

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CombatSkill = 0.5f;                   // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> NPCActor;            // Reference to the actual NPC actor in the world

    bool CanSeeThrough(const FForge2Disguise& TargetDisguise) const;
    FVector GetLocation() const { return NPCActor.IsValid() ? NPCActor->GetActorLocation() : FVector::ZeroVector; }
    FVector GetForward() const { return NPCActor.IsValid() ? NPCActor->GetActorForwardVector() : FVector::ForwardVector; }
};

// ============================================================================
// Level Definition
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StealthLevelDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName LevelID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2StealthNPCDefinition> NPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2SecurityDevice> SecurityDevices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2CoverPoint> CoverPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2HidingSpot> HidingSpots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2LightSource> LightSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2Disguise> AvailableDisguises;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2AlarmState AlarmState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientLightLevel = 0.3f;

    // Objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGhostObjective = true;                // No detections

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNoKillsObjective = true;              // Non-lethal only

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNoAlarmsObjective = true;             // No alarms triggered

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNoBodiesFoundObjective = true;        // Hide all bodies

    // Stats tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesDetected = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 KillCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 KnockoutCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BodiesFound = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AlarmsTriggered = 0;
};

// ============================================================================
// Player State
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StealthPlayerState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2DisguiseState DisguiseState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2LightLevel CurrentLightLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInCover = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentCoverID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRunning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2NoiseLevel CurrentNoiseLevel = EForge2NoiseLevel::Silent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCarryingBody = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CarriedBodyID;

    float GetVisibilityScore() const;
    float GetNoiseMultiplier() const;
};

// ============================================================================
// Stealth Manager
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2StealthManager : public UObject
{
    GENERATED_BODY()

public:
    UForge2StealthManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2StealthLevelDefinition CurrentLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2StealthPlayerState PlayerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentTime = 0.0f;

    // Initialization
    UFUNCTION(BlueprintCallable)
    void InitializeLevel(const FForge2StealthLevelDefinition& Level);

    // Update functions
    UFUNCTION(BlueprintCallable)
    void UpdateNPCAwareness(float DeltaTime);

    UFUNCTION(BlueprintCallable)
    void UpdateSecurityDevices(float DeltaTime);

    UFUNCTION(BlueprintCallable)
    void UpdateLightLevels();

    UFUNCTION(BlueprintCallable)
    void UpdateDistractions(float DeltaTime);

    // Detection
    UFUNCTION(BlueprintCallable)
    bool CheckPlayerDetection(int32 NPCIndex);

    UFUNCTION(BlueprintCallable)
    void ProcessSoundAtLocation(const FVector& Location, float Volume, EForge2NoiseLevel NoiseLevel);

    // Actions
    UFUNCTION(BlueprintCallable)
    bool TriggerDistraction(const FForge2Distraction& Distraction);

    UFUNCTION(BlueprintCallable)
    bool PerformTakedown(int32 NPCIndex, EForge2TakedownType TakedownType);

    UFUNCTION(BlueprintCallable)
    bool HideBody(FName BodyID, FName HidingSpotID);

    UFUNCTION(BlueprintCallable)
    bool ChangeDisguise(FName DisguiseID);

    UFUNCTION(BlueprintCallable)
    bool HackSecurityDevice(FName DeviceID);

    UFUNCTION(BlueprintCallable)
    bool DisableSecurityDevice(FName DeviceID);

    // Cover
    UFUNCTION(BlueprintCallable)
    bool EnterCover(FName CoverID);

    UFUNCTION(BlueprintCallable)
    void ExitCover();

    // Queries
    UFUNCTION(BlueprintCallable)
    FForge2LightLevel GetLightLevelAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable)
    TArray<FForge2CoverPoint> GetNearbyCover(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable)
    bool IsInRestrictedArea(const FVector& Location, const FForge2Disguise& Disguise) const;

    // Score/Rating
    UFUNCTION(BlueprintCallable)
    int32 CalculateStealthRating() const;

    UFUNCTION(BlueprintCallable)
    bool IsGhostRun() const;

    UFUNCTION(BlueprintCallable)
    bool IsSilentAssassinRun() const;
};

// ============================================================================
// Utility Functions
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2StealthUtilities : public UObject
{
    GENERATED_BODY()

public:
    // Vision cone presets
    UFUNCTION(BlueprintCallable)
    static FForge2VisionCone CreateStandardVision();

    UFUNCTION(BlueprintCallable)
    static FForge2VisionCone CreateAlertedVision();

    UFUNCTION(BlueprintCallable)
    static FForge2VisionCone CreateSniper();

    UFUNCTION(BlueprintCallable)
    static FForge2HearingConfig CreateStandardHearing();

    UFUNCTION(BlueprintCallable)
    static FForge2HearingConfig CreateAlertedHearing();

    // NPC presets
    UFUNCTION(BlueprintCallable)
    static FForge2StealthNPCDefinition CreateBasicGuard();

    UFUNCTION(BlueprintCallable)
    static FForge2StealthNPCDefinition CreateEliteGuard();

    UFUNCTION(BlueprintCallable)
    static FForge2StealthNPCDefinition CreateCivilian();

    // Security presets
    UFUNCTION(BlueprintCallable)
    static FForge2SecurityDevice CreateSecurityCamera(const FVector& Location, const FRotator& Rotation, float SweepAngle = 90.0f);

    UFUNCTION(BlueprintCallable)
    static FForge2SecurityDevice CreateLaserTripwire(const FVector& Start, const FVector& End);

    UFUNCTION(BlueprintCallable)
    static FForge2SecurityDevice CreateMotionSensor(const FVector& Location, float Radius);

    // Distraction presets
    UFUNCTION(BlueprintCallable)
    static FForge2Distraction CreateNoiseDistraction(const FVector& Location, float Volume);

    UFUNCTION(BlueprintCallable)
    static FForge2Distraction CreateDecoyDistraction(const FVector& Location);

    // Cover presets
    UFUNCTION(BlueprintCallable)
    static FForge2CoverPoint CreateLowCover(const FVector& Location, const FRotator& Direction);

    UFUNCTION(BlueprintCallable)
    static FForge2CoverPoint CreateHighCover(const FVector& Location, const FRotator& Direction);

    UFUNCTION(BlueprintCallable)
    static FForge2CoverPoint CreateCornerCover(const FVector& Location, const FRotator& Direction);

    // Disguise presets
    UFUNCTION(BlueprintCallable)
    static FForge2Disguise CreateGuardDisguise();

    UFUNCTION(BlueprintCallable)
    static FForge2Disguise CreateCivilianDisguise();

    UFUNCTION(BlueprintCallable)
    static FForge2Disguise CreateWorkerDisguise();
};
