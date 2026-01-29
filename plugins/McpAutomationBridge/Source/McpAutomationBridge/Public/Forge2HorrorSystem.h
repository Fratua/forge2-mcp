// Forge2HorrorSystem.h
// FORGE2 Horror Genre System
// Provides horror game mechanics: fear, sanity, atmosphere, monsters, jumpscares, safe zones

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Forge2HorrorSystem.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EForge2FearLevel : uint8
{
    Calm,
    Uneasy,
    Nervous,
    Scared,
    Terrified,
    Panic
};

UENUM(BlueprintType)
enum class EForge2SanityState : uint8
{
    Stable,
    Stressed,
    Disturbed,
    Unstable,
    Broken
};

UENUM(BlueprintType)
enum class EForge2MonsterBehavior : uint8
{
    Patrol,
    Idle,
    Stalking,
    Chasing,
    Searching,
    Attacking,
    Retreating,
    Dormant
};

UENUM(BlueprintType)
enum class EForge2HorrorEventType : uint8
{
    Jumpscare,
    Ambience,
    VisualDistortion,
    AudioDistortion,
    Hallucination,
    EntityAppearance,
    ObjectMovement,
    LightFlicker,
    DoorSlam,
    Whisper,
    Footsteps,
    Breathing
};

UENUM(BlueprintType)
enum class EForge2AtmosphereType : uint8
{
    Safe,
    Eerie,
    Tense,
    Dread,
    Nightmare
};

// ============================================================================
// Fear System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FearConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFear = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearDecayRate = 5.0f;             // Per second in safe areas

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearBuildupRate = 2.0f;           // Per second in danger

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpscareIntensity = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MonsterSightFear = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DarknessFearRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseFearMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> FearThresholds;           // Thresholds for each EForge2FearLevel

    EForge2FearLevel GetFearLevel(float CurrentFear) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FearState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentFear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2FearLevel Level = EForge2FearLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeInCurrentLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInSafeZone = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMonsterVisible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LightLevel = 1.0f;                // 0 = pitch black, 1 = well lit

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastJumpscareTime = -100.0f;

    void Update(float DeltaTime, const FForge2FearConfig& Config);
    void AddFear(float Amount);
    void TriggerJumpscare(float Intensity);
};

// ============================================================================
// Sanity System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SanityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSanity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SanityRecoveryRate = 1.0f;        // Per second in safe areas

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SanityDrainRate = 0.5f;           // Per second when stressed

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MonsterDrainMultiplier = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DarknessDrainMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CorpseDrainAmount = 5.0f;         // Per corpse seen

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> SanityThresholds;         // Thresholds for each EForge2SanityState

    EForge2SanityState GetSanityState(float CurrentSanity) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SanityState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentSanity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2SanityState State = EForge2SanityState::Stable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HallucinationChance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VisualDistortionIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AudioDistortionIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ActiveHallucinations;

    void Update(float DeltaTime, const FForge2SanityConfig& Config, const FForge2FearState& FearState);
    void DrainSanity(float Amount);
    void RestoreSanity(float Amount);
    void UpdateEffects();
};

// ============================================================================
// Atmosphere System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AtmosphereConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2AtmosphereType Type = EForge2AtmosphereType::Eerie;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.1f, 0.1f, 0.15f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientLightIntensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor = FLinearColor(0.05f, 0.05f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShadowIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientSoundVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeartbeatVolume = 0.0f;           // Increases with fear

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreathingVolume = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VignetteIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChromaticAberration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FilmGrain = 0.1f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AtmosphereZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Extent = FVector(500.0f, 500.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2AtmosphereConfig Atmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSafeZone = false;

    bool IsLocationInZone(const FVector& Location) const;
    float GetBlendWeight(const FVector& Location) const;
};

// ============================================================================
// Monster/Enemy System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MonsterDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MonsterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MonsterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChaseSpeed = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeLookedAt = true;             // False = Weeping Angel style

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAvoidLight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LightDamage = 0.0f;               // Damage per second in light

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInstantKill = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearAura = 10.0f;                 // Fear caused per second when visible

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SanityDrain = 5.0f;               // Sanity drained per second when visible

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> WeakSpots;                // Vulnerable points

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ImmuneToWeapons;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MonsterInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MonsterTypeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2MonsterBehavior Behavior = EForge2MonsterBehavior::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetPlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenPlayerTime = -100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SearchTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackCooldownRemaining = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPatrolIndex = 0;

    void UpdateBehavior(float DeltaTime, const FForge2MonsterDefinition& Definition);
    bool CanSeePlayer(const FVector& PlayerLocation, const FForge2MonsterDefinition& Definition) const;
    bool CanHearPlayer(const FVector& PlayerLocation, float NoiseLevel, const FForge2MonsterDefinition& Definition) const;
};

// ============================================================================
// Horror Event System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HorrorEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2HorrorEventType EventType = EForge2HorrorEventType::Ambience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearAmount = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SanityDrain = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TriggerSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TriggerVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TriggerChance = 0.1f;             // Per check

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresDarkness = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLowSanity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSanityThreshold = 50.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2JumpscareDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName JumpscareID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EntityMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ScreamSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CameraShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearAmount = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SanityDrain = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFreezePlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bForceCamera = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SpawnOffset = FVector(150.0f, 0.0f, 0.0f);
};

// ============================================================================
// Resource Scarcity
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ResourceScarcity
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHealthItems = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAmmo = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxBatteries = 2;                 // For flashlight

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSanityItems = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ItemSpawnChance = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlashlightDrainRate = 0.05f;      // Battery per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentBatteryLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentHealthItems = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentAmmo = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentBatteries = 1;

    void UseBattery();
    void UseHealthItem(float& Health);
    bool HasFlashlightPower() const { return CurrentBatteryLevel > 0.0f; }
};

// ============================================================================
// Save System (Horror-specific)
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HorrorSaveConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLimitedSaves = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSaves = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentSaves = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequireSaveRoom = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequireSaveItem = false;          // Ink ribbon style

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SaveItemID;

    bool CanSave() const;
    void UseSave();
};

// ============================================================================
// Safe Zone
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SafeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMonstersCanEnter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanSaveHere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SafeMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearRecoveryMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SanityRecoveryMultiplier = 2.0f;

    bool IsLocationInZone(const FVector& Location) const;
};

// ============================================================================
// Player State
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HorrorPlayerState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2FearState Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2SanityState Sanity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2ResourceScarcity Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFlashlightOn = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHiding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRunning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseLevel = 0.0f;                // 0 = silent, 1 = loud

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaRemaining = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentSafeZoneID;

    void UpdateNoiseLevel();
    bool IsInDanger() const;
};

// ============================================================================
// Horror Manager
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2HorrorManager : public UObject
{
    GENERATED_BODY()

public:
    UForge2HorrorManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2FearConfig FearConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2SanityConfig SanityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2MonsterDefinition> MonsterDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2MonsterInstance> ActiveMonsters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2AtmosphereZone> AtmosphereZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2SafeZone> SafeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2HorrorEvent> PossibleEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2JumpscareDefinition> Jumpscares;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2HorrorPlayerState PlayerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2HorrorSaveConfig SaveConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GlobalTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EventCheckInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastEventCheckTime = 0.0f;

    // Initialization
    UFUNCTION(BlueprintCallable)
    void Initialize();

    // Update
    UFUNCTION(BlueprintCallable)
    void Update(float DeltaTime);

    // Fear/Sanity
    UFUNCTION(BlueprintCallable)
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable)
    void DrainSanity(float Amount);

    UFUNCTION(BlueprintCallable)
    void RestoreSanity(float Amount);

    // Events
    UFUNCTION(BlueprintCallable)
    void TriggerJumpscare(FName JumpscareID);

    UFUNCTION(BlueprintCallable)
    void TriggerEvent(FName EventID);

    UFUNCTION(BlueprintCallable)
    void CheckForRandomEvents();

    // Monsters
    UFUNCTION(BlueprintCallable)
    void SpawnMonster(FName MonsterTypeID, const FVector& Location);

    UFUNCTION(BlueprintCallable)
    void DespawnMonster(FName InstanceID);

    UFUNCTION(BlueprintCallable)
    void AlertMonstersToLocation(const FVector& Location, float NoiseRadius);

    // Queries
    UFUNCTION(BlueprintCallable)
    bool IsPlayerInSafeZone() const;

    UFUNCTION(BlueprintCallable)
    FForge2AtmosphereConfig GetAtmosphereAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable)
    bool CanPlayerSave() const;

private:
    void UpdatePlayerState(float DeltaTime);
    void UpdateMonsters(float DeltaTime);
    void UpdateAtmosphere();
    void CheckMonsterDetection();
};

// ============================================================================
// Utility Functions
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2HorrorUtilities : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static FForge2MonsterDefinition CreateSlasherMonster(const FString& Name);

    UFUNCTION(BlueprintCallable)
    static FForge2MonsterDefinition CreateGhostMonster(const FString& Name);

    UFUNCTION(BlueprintCallable)
    static FForge2MonsterDefinition CreateCreatureMonster(const FString& Name);

    UFUNCTION(BlueprintCallable)
    static FForge2AtmosphereConfig CreateEerieAtmosphere();

    UFUNCTION(BlueprintCallable)
    static FForge2AtmosphereConfig CreateNightmareAtmosphere();

    UFUNCTION(BlueprintCallable)
    static FForge2JumpscareDefinition CreateBasicJumpscare(const FString& Name, float Intensity);

    UFUNCTION(BlueprintCallable)
    static FForge2HorrorEvent CreateAmbienceEvent(const FString& Name, EForge2HorrorEventType Type);
};
