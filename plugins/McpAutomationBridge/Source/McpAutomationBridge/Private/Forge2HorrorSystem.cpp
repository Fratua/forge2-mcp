// Forge2HorrorSystem.cpp
// FORGE2 Horror Genre System Implementation

#include "Forge2HorrorSystem.h"

// ============================================================================
// FForge2FearConfig
// ============================================================================

EForge2FearLevel FForge2FearConfig::GetFearLevel(float CurrentFear) const
{
    if (FearThresholds.Num() >= 5)
    {
        if (CurrentFear >= FearThresholds[4]) return EForge2FearLevel::Panic;
        if (CurrentFear >= FearThresholds[3]) return EForge2FearLevel::Terrified;
        if (CurrentFear >= FearThresholds[2]) return EForge2FearLevel::Scared;
        if (CurrentFear >= FearThresholds[1]) return EForge2FearLevel::Nervous;
        if (CurrentFear >= FearThresholds[0]) return EForge2FearLevel::Uneasy;
    }
    else
    {
        // Default thresholds
        if (CurrentFear >= 80.0f) return EForge2FearLevel::Panic;
        if (CurrentFear >= 60.0f) return EForge2FearLevel::Terrified;
        if (CurrentFear >= 40.0f) return EForge2FearLevel::Scared;
        if (CurrentFear >= 20.0f) return EForge2FearLevel::Nervous;
        if (CurrentFear >= 10.0f) return EForge2FearLevel::Uneasy;
    }

    return EForge2FearLevel::Calm;
}

// ============================================================================
// FForge2FearState
// ============================================================================

void FForge2FearState::Update(float DeltaTime, const FForge2FearConfig& Config)
{
    // Decay fear in safe zones
    if (bInSafeZone)
    {
        CurrentFear -= Config.FearDecayRate * DeltaTime;
    }
    else
    {
        // Build fear based on conditions
        if (bMonsterVisible)
        {
            CurrentFear += Config.MonsterSightFear * DeltaTime;
        }

        if (LightLevel < 0.3f)
        {
            CurrentFear += Config.DarknessFearRate * (1.0f - LightLevel) * DeltaTime;
        }
    }

    // Clamp fear
    CurrentFear = FMath::Clamp(CurrentFear, 0.0f, Config.MaxFear);

    // Update level
    EForge2FearLevel NewLevel = Config.GetFearLevel(CurrentFear);
    if (NewLevel != Level)
    {
        Level = NewLevel;
        TimeInCurrentLevel = 0.0f;
    }
    else
    {
        TimeInCurrentLevel += DeltaTime;
    }
}

void FForge2FearState::AddFear(float Amount)
{
    CurrentFear += Amount;
}

void FForge2FearState::TriggerJumpscare(float Intensity)
{
    CurrentFear += Intensity;
    LastJumpscareTime = 0.0f; // Would be set to current game time
}

// ============================================================================
// FForge2SanityConfig
// ============================================================================

EForge2SanityState FForge2SanityConfig::GetSanityState(float CurrentSanity) const
{
    if (SanityThresholds.Num() >= 4)
    {
        if (CurrentSanity <= SanityThresholds[3]) return EForge2SanityState::Broken;
        if (CurrentSanity <= SanityThresholds[2]) return EForge2SanityState::Unstable;
        if (CurrentSanity <= SanityThresholds[1]) return EForge2SanityState::Disturbed;
        if (CurrentSanity <= SanityThresholds[0]) return EForge2SanityState::Stressed;
    }
    else
    {
        // Default thresholds
        if (CurrentSanity <= 10.0f) return EForge2SanityState::Broken;
        if (CurrentSanity <= 25.0f) return EForge2SanityState::Unstable;
        if (CurrentSanity <= 50.0f) return EForge2SanityState::Disturbed;
        if (CurrentSanity <= 75.0f) return EForge2SanityState::Stressed;
    }

    return EForge2SanityState::Stable;
}

// ============================================================================
// FForge2SanityState
// ============================================================================

void FForge2SanityState::Update(float DeltaTime, const FForge2SanityConfig& Config, const FForge2FearState& FearState)
{
    // Drain sanity based on fear level
    float DrainMultiplier = 1.0f;
    switch (FearState.Level)
    {
        case EForge2FearLevel::Panic: DrainMultiplier = 3.0f; break;
        case EForge2FearLevel::Terrified: DrainMultiplier = 2.0f; break;
        case EForge2FearLevel::Scared: DrainMultiplier = 1.5f; break;
        case EForge2FearLevel::Nervous: DrainMultiplier = 1.0f; break;
        default: DrainMultiplier = 0.0f; break;
    }

    if (FearState.bInSafeZone)
    {
        // Recover in safe zones
        CurrentSanity += Config.SanityRecoveryRate * DeltaTime;
    }
    else if (DrainMultiplier > 0.0f)
    {
        CurrentSanity -= Config.SanityDrainRate * DrainMultiplier * DeltaTime;
    }

    // Monster visibility effect
    if (FearState.bMonsterVisible)
    {
        CurrentSanity -= Config.SanityDrainRate * Config.MonsterDrainMultiplier * DeltaTime;
    }

    // Darkness effect
    if (FearState.LightLevel < 0.3f)
    {
        CurrentSanity -= Config.SanityDrainRate * Config.DarknessDrainMultiplier * (1.0f - FearState.LightLevel) * DeltaTime;
    }

    // Clamp sanity
    CurrentSanity = FMath::Clamp(CurrentSanity, 0.0f, Config.MaxSanity);

    // Update state
    State = Config.GetSanityState(CurrentSanity);

    // Update effects
    UpdateEffects();
}

void FForge2SanityState::DrainSanity(float Amount)
{
    CurrentSanity -= Amount;
    CurrentSanity = FMath::Max(0.0f, CurrentSanity);
}

void FForge2SanityState::RestoreSanity(float Amount)
{
    CurrentSanity += Amount;
    CurrentSanity = FMath::Min(100.0f, CurrentSanity);
}

void FForge2SanityState::UpdateEffects()
{
    // Calculate effect intensities based on sanity level
    float SanityPercent = CurrentSanity / 100.0f;

    HallucinationChance = (1.0f - SanityPercent) * 0.3f;
    VisualDistortionIntensity = (1.0f - SanityPercent) * 0.5f;
    AudioDistortionIntensity = (1.0f - SanityPercent) * 0.4f;

    // Special effects at low sanity
    if (State == EForge2SanityState::Broken)
    {
        HallucinationChance = 0.5f;
        VisualDistortionIntensity = 1.0f;
        AudioDistortionIntensity = 0.8f;
    }
    else if (State == EForge2SanityState::Unstable)
    {
        HallucinationChance = 0.3f;
        VisualDistortionIntensity = 0.6f;
        AudioDistortionIntensity = 0.5f;
    }
}

// ============================================================================
// FForge2AtmosphereZone
// ============================================================================

bool FForge2AtmosphereZone::IsLocationInZone(const FVector& Location) const
{
    FVector Diff = Location - Center;
    return FMath::Abs(Diff.X) <= Extent.X &&
           FMath::Abs(Diff.Y) <= Extent.Y &&
           FMath::Abs(Diff.Z) <= Extent.Z;
}

float FForge2AtmosphereZone::GetBlendWeight(const FVector& Location) const
{
    if (!IsLocationInZone(Location))
    {
        // Calculate distance to zone for blending
        FVector Diff = Location - Center;
        FVector ClampedDiff;
        ClampedDiff.X = FMath::Clamp(Diff.X, -Extent.X, Extent.X);
        ClampedDiff.Y = FMath::Clamp(Diff.Y, -Extent.Y, Extent.Y);
        ClampedDiff.Z = FMath::Clamp(Diff.Z, -Extent.Z, Extent.Z);

        float Distance = (Diff - ClampedDiff).Size();
        if (Distance > BlendRadius)
        {
            return 0.0f;
        }

        return 1.0f - (Distance / BlendRadius);
    }

    return 1.0f;
}

// ============================================================================
// FForge2MonsterInstance
// ============================================================================

void FForge2MonsterInstance::UpdateBehavior(float DeltaTime, const FForge2MonsterDefinition& Definition)
{
    // Update attack cooldown
    if (AttackCooldownRemaining > 0.0f)
    {
        AttackCooldownRemaining -= DeltaTime;
    }

    // Update search timer
    if (Behavior == EForge2MonsterBehavior::Searching)
    {
        SearchTimer -= DeltaTime;
        if (SearchTimer <= 0.0f)
        {
            Behavior = EForge2MonsterBehavior::Patrol;
        }
    }

    // Move towards target based on behavior
    float Speed = 0.0f;
    switch (Behavior)
    {
        case EForge2MonsterBehavior::Patrol:
        {
            if (PatrolPoints.Num() > 0)
            {
                TargetLocation = PatrolPoints[CurrentPatrolIndex];
                Speed = Definition.MoveSpeed;

                // Check if reached patrol point
                if (FVector::Distance(Location, TargetLocation) < 50.0f)
                {
                    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
                }
            }
            break;
        }

        case EForge2MonsterBehavior::Chasing:
        {
            Speed = Definition.ChaseSpeed;
            break;
        }

        case EForge2MonsterBehavior::Stalking:
        {
            Speed = Definition.MoveSpeed * 0.5f;
            break;
        }

        case EForge2MonsterBehavior::Searching:
        {
            TargetLocation = LastKnownPlayerLocation;
            Speed = Definition.MoveSpeed * 0.75f;
            break;
        }

        default:
            break;
    }

    // Apply movement
    if (Speed > 0.0f && Behavior != EForge2MonsterBehavior::Idle && Behavior != EForge2MonsterBehavior::Dormant)
    {
        FVector Direction = (TargetLocation - Location).GetSafeNormal();
        Location += Direction * Speed * DeltaTime;

        // Face movement direction
        if (!Direction.IsNearlyZero())
        {
            Rotation = Direction.Rotation();
        }
    }
}

bool FForge2MonsterInstance::CanSeePlayer(const FVector& PlayerLocation, const FForge2MonsterDefinition& Definition) const
{
    float Distance = FVector::Distance(Location, PlayerLocation);
    if (Distance > Definition.DetectionRadius)
    {
        return false;
    }

    // Check angle
    FVector ToPlayer = (PlayerLocation - Location).GetSafeNormal();
    FVector Forward = Rotation.Vector();
    float Dot = FVector::DotProduct(Forward, ToPlayer);

    // ~120 degree field of view
    return Dot > 0.5f;
}

bool FForge2MonsterInstance::CanHearPlayer(const FVector& PlayerLocation, float NoiseLevel, const FForge2MonsterDefinition& Definition) const
{
    if (NoiseLevel <= 0.0f) return false;

    float Distance = FVector::Distance(Location, PlayerLocation);
    float HearingRange = Definition.HearingRadius * NoiseLevel;

    return Distance <= HearingRange;
}

// ============================================================================
// FForge2ResourceScarcity
// ============================================================================

void FForge2ResourceScarcity::UseBattery()
{
    if (CurrentBatteries > 0)
    {
        CurrentBatteries--;
        CurrentBatteryLevel = 1.0f;
    }
}

void FForge2ResourceScarcity::UseHealthItem(float& Health)
{
    if (CurrentHealthItems > 0)
    {
        CurrentHealthItems--;
        Health = FMath::Min(Health + 50.0f, 100.0f);
    }
}

// ============================================================================
// FForge2HorrorSaveConfig
// ============================================================================

bool FForge2HorrorSaveConfig::CanSave() const
{
    if (!bLimitedSaves) return true;
    return CurrentSaves > 0;
}

void FForge2HorrorSaveConfig::UseSave()
{
    if (bLimitedSaves && CurrentSaves > 0)
    {
        CurrentSaves--;
    }
}

// ============================================================================
// FForge2SafeZone
// ============================================================================

bool FForge2SafeZone::IsLocationInZone(const FVector& Location) const
{
    return FVector::Distance(Location, Center) <= Radius;
}

// ============================================================================
// FForge2HorrorPlayerState
// ============================================================================

void FForge2HorrorPlayerState::UpdateNoiseLevel()
{
    NoiseLevel = 0.0f;

    if (bIsRunning)
    {
        NoiseLevel += 0.8f;
    }
    else if (!bIsHiding)
    {
        NoiseLevel += 0.2f; // Walking noise
    }

    if (bFlashlightOn)
    {
        NoiseLevel += 0.1f; // Slight noise from flashlight
    }

    NoiseLevel = FMath::Clamp(NoiseLevel, 0.0f, 1.0f);
}

bool FForge2HorrorPlayerState::IsInDanger() const
{
    return Fear.Level >= EForge2FearLevel::Scared || Fear.bMonsterVisible;
}

// ============================================================================
// UForge2HorrorManager
// ============================================================================

UForge2HorrorManager::UForge2HorrorManager()
{
    GlobalTime = 0.0f;
    EventCheckInterval = 5.0f;
    LastEventCheckTime = 0.0f;

    // Initialize default fear thresholds
    FearConfig.FearThresholds.Add(10.0f);   // Uneasy
    FearConfig.FearThresholds.Add(25.0f);   // Nervous
    FearConfig.FearThresholds.Add(45.0f);   // Scared
    FearConfig.FearThresholds.Add(65.0f);   // Terrified
    FearConfig.FearThresholds.Add(85.0f);   // Panic

    // Initialize default sanity thresholds
    SanityConfig.SanityThresholds.Add(75.0f);   // Stressed
    SanityConfig.SanityThresholds.Add(50.0f);   // Disturbed
    SanityConfig.SanityThresholds.Add(25.0f);   // Unstable
    SanityConfig.SanityThresholds.Add(10.0f);   // Broken
}

void UForge2HorrorManager::Initialize()
{
    PlayerState.Fear.CurrentFear = 0.0f;
    PlayerState.Sanity.CurrentSanity = 100.0f;
    PlayerState.Health = 100.0f;
    PlayerState.Resources.CurrentBatteryLevel = 1.0f;

    ActiveMonsters.Empty();
    GlobalTime = 0.0f;
}

void UForge2HorrorManager::Update(float DeltaTime)
{
    GlobalTime += DeltaTime;

    UpdatePlayerState(DeltaTime);
    UpdateMonsters(DeltaTime);
    UpdateAtmosphere();
    CheckMonsterDetection();

    // Check for random events
    if (GlobalTime - LastEventCheckTime >= EventCheckInterval)
    {
        CheckForRandomEvents();
        LastEventCheckTime = GlobalTime;
    }
}

void UForge2HorrorManager::UpdatePlayerState(float DeltaTime)
{
    // Check if in safe zone
    PlayerState.Fear.bInSafeZone = IsPlayerInSafeZone();
    PlayerState.CurrentSafeZoneID = NAME_None;

    for (const FForge2SafeZone& Zone : SafeZones)
    {
        if (Zone.IsLocationInZone(PlayerState.Location))
        {
            PlayerState.CurrentSafeZoneID = Zone.ZoneID;
            break;
        }
    }

    // Update flashlight battery
    if (PlayerState.bFlashlightOn && PlayerState.Resources.HasFlashlightPower())
    {
        PlayerState.Resources.CurrentBatteryLevel -= PlayerState.Resources.FlashlightDrainRate * DeltaTime;
        if (PlayerState.Resources.CurrentBatteryLevel <= 0.0f)
        {
            PlayerState.Resources.CurrentBatteryLevel = 0.0f;
            PlayerState.bFlashlightOn = false;
        }
    }

    // Update noise level
    PlayerState.UpdateNoiseLevel();

    // Update stamina
    if (PlayerState.bIsRunning)
    {
        PlayerState.StaminaRemaining -= 10.0f * DeltaTime;
        if (PlayerState.StaminaRemaining <= 0.0f)
        {
            PlayerState.StaminaRemaining = 0.0f;
            PlayerState.bIsRunning = false;
        }
    }
    else
    {
        PlayerState.StaminaRemaining += 5.0f * DeltaTime;
        PlayerState.StaminaRemaining = FMath::Min(PlayerState.StaminaRemaining, 100.0f);
    }

    // Calculate light level at player location
    PlayerState.Fear.LightLevel = PlayerState.bFlashlightOn ? 0.8f : 0.2f;

    // Update fear and sanity
    PlayerState.Fear.Update(DeltaTime, FearConfig);
    PlayerState.Sanity.Update(DeltaTime, SanityConfig, PlayerState.Fear);
}

void UForge2HorrorManager::UpdateMonsters(float DeltaTime)
{
    for (FForge2MonsterInstance& Monster : ActiveMonsters)
    {
        if (!Monster.bIsActive) continue;

        // Find monster definition
        const FForge2MonsterDefinition* Definition = nullptr;
        for (const FForge2MonsterDefinition& Def : MonsterDefinitions)
        {
            if (Def.MonsterID == Monster.MonsterTypeID)
            {
                Definition = &Def;
                break;
            }
        }

        if (!Definition) continue;

        // Check if monster should avoid light
        if (Definition->bAvoidLight && PlayerState.bFlashlightOn)
        {
            FVector ToPlayer = PlayerState.Location - Monster.Location;
            float Distance = ToPlayer.Size();

            // Check if monster is in flashlight beam
            if (Distance < 500.0f)
            {
                Monster.Behavior = EForge2MonsterBehavior::Retreating;
            }
        }

        // Check if player can be detected
        bool bCanSee = Monster.CanSeePlayer(PlayerState.Location, *Definition);
        bool bCanHear = Monster.CanHearPlayer(PlayerState.Location, PlayerState.NoiseLevel, *Definition);

        if (bCanSee || bCanHear)
        {
            Monster.LastSeenPlayerTime = GlobalTime;
            Monster.LastKnownPlayerLocation = PlayerState.Location;
            Monster.TargetPlayerID = PlayerState.PlayerID;

            if (bCanSee)
            {
                Monster.Behavior = EForge2MonsterBehavior::Chasing;
                PlayerState.Fear.bMonsterVisible = true;
            }
            else
            {
                Monster.Behavior = EForge2MonsterBehavior::Searching;
                Monster.SearchTimer = 10.0f;
            }
        }
        else
        {
            // Lose sight of player
            if (Monster.Behavior == EForge2MonsterBehavior::Chasing)
            {
                Monster.Behavior = EForge2MonsterBehavior::Searching;
                Monster.SearchTimer = 15.0f;
            }
        }

        // Check for attack
        if (Monster.Behavior == EForge2MonsterBehavior::Chasing)
        {
            float Distance = FVector::Distance(Monster.Location, PlayerState.Location);
            if (Distance <= Definition->AttackRange && Monster.AttackCooldownRemaining <= 0.0f)
            {
                Monster.Behavior = EForge2MonsterBehavior::Attacking;

                if (Definition->bInstantKill)
                {
                    PlayerState.Health = 0.0f;
                }
                else
                {
                    PlayerState.Health -= Definition->AttackDamage;
                    AddFear(30.0f);
                }

                Monster.AttackCooldownRemaining = Definition->AttackCooldown;
            }
        }

        // Update monster behavior
        Monster.UpdateBehavior(DeltaTime, *Definition);
    }

    // Reset monster visibility flag if no monster is currently visible
    bool bAnyMonsterVisible = false;
    for (const FForge2MonsterInstance& Monster : ActiveMonsters)
    {
        if (Monster.Behavior == EForge2MonsterBehavior::Chasing)
        {
            bAnyMonsterVisible = true;
            break;
        }
    }
    PlayerState.Fear.bMonsterVisible = bAnyMonsterVisible;
}

void UForge2HorrorManager::UpdateAtmosphere()
{
    // Atmosphere updates would be applied to post-process and audio systems
}

void UForge2HorrorManager::CheckMonsterDetection()
{
    // Already handled in UpdateMonsters
}

void UForge2HorrorManager::AddFear(float Amount)
{
    PlayerState.Fear.AddFear(Amount);
}

void UForge2HorrorManager::DrainSanity(float Amount)
{
    PlayerState.Sanity.DrainSanity(Amount);
}

void UForge2HorrorManager::RestoreSanity(float Amount)
{
    PlayerState.Sanity.RestoreSanity(Amount);
}

void UForge2HorrorManager::TriggerJumpscare(FName JumpscareID)
{
    for (const FForge2JumpscareDefinition& Jumpscare : Jumpscares)
    {
        if (Jumpscare.JumpscareID == JumpscareID)
        {
            PlayerState.Fear.TriggerJumpscare(Jumpscare.FearAmount);
            PlayerState.Sanity.DrainSanity(Jumpscare.SanityDrain);

            // Would trigger visual/audio effects here
            break;
        }
    }
}

void UForge2HorrorManager::TriggerEvent(FName EventID)
{
    for (const FForge2HorrorEvent& Event : PossibleEvents)
    {
        if (Event.EventID == EventID)
        {
            PlayerState.Fear.AddFear(Event.FearAmount);
            PlayerState.Sanity.DrainSanity(Event.SanityDrain);

            // Would trigger visual/audio effects here
            break;
        }
    }
}

void UForge2HorrorManager::CheckForRandomEvents()
{
    if (PlayerState.Fear.bInSafeZone) return;

    for (const FForge2HorrorEvent& Event : PossibleEvents)
    {
        // Check conditions
        if (Event.bRequiresDarkness && PlayerState.Fear.LightLevel > 0.3f)
        {
            continue;
        }

        if (Event.bRequiresLowSanity && PlayerState.Sanity.CurrentSanity > Event.MinSanityThreshold)
        {
            continue;
        }

        // Random chance
        if (FMath::FRand() < Event.TriggerChance)
        {
            TriggerEvent(Event.EventID);
            break; // Only one event per check
        }
    }
}

void UForge2HorrorManager::SpawnMonster(FName MonsterTypeID, const FVector& Location)
{
    FForge2MonsterInstance Monster;
    Monster.InstanceID = *FString::Printf(TEXT("Monster_%d"), ActiveMonsters.Num());
    Monster.MonsterTypeID = MonsterTypeID;
    Monster.Location = Location;
    Monster.bIsActive = true;
    Monster.Behavior = EForge2MonsterBehavior::Patrol;

    ActiveMonsters.Add(Monster);
}

void UForge2HorrorManager::DespawnMonster(FName InstanceID)
{
    for (int32 i = ActiveMonsters.Num() - 1; i >= 0; --i)
    {
        if (ActiveMonsters[i].InstanceID == InstanceID)
        {
            ActiveMonsters.RemoveAt(i);
            break;
        }
    }
}

void UForge2HorrorManager::AlertMonstersToLocation(const FVector& Location, float NoiseRadius)
{
    for (FForge2MonsterInstance& Monster : ActiveMonsters)
    {
        if (!Monster.bIsActive) continue;

        float Distance = FVector::Distance(Monster.Location, Location);
        if (Distance <= NoiseRadius)
        {
            Monster.LastKnownPlayerLocation = Location;
            Monster.Behavior = EForge2MonsterBehavior::Searching;
            Monster.SearchTimer = 15.0f;
        }
    }
}

bool UForge2HorrorManager::IsPlayerInSafeZone() const
{
    for (const FForge2SafeZone& Zone : SafeZones)
    {
        if (Zone.IsLocationInZone(PlayerState.Location))
        {
            return true;
        }
    }
    return false;
}

FForge2AtmosphereConfig UForge2HorrorManager::GetAtmosphereAtLocation(const FVector& Location) const
{
    FForge2AtmosphereConfig BlendedAtmosphere;

    float TotalWeight = 0.0f;
    for (const FForge2AtmosphereZone& Zone : AtmosphereZones)
    {
        float Weight = Zone.GetBlendWeight(Location);
        if (Weight > 0.0f)
        {
            // Blend atmosphere properties
            BlendedAtmosphere.FogDensity += Zone.Atmosphere.FogDensity * Weight;
            BlendedAtmosphere.AmbientLightIntensity += Zone.Atmosphere.AmbientLightIntensity * Weight;
            BlendedAtmosphere.VignetteIntensity += Zone.Atmosphere.VignetteIntensity * Weight;

            TotalWeight += Weight;
        }
    }

    // Normalize blended values
    if (TotalWeight > 0.0f)
    {
        BlendedAtmosphere.FogDensity /= TotalWeight;
        BlendedAtmosphere.AmbientLightIntensity /= TotalWeight;
        BlendedAtmosphere.VignetteIntensity /= TotalWeight;
    }

    return BlendedAtmosphere;
}

bool UForge2HorrorManager::CanPlayerSave() const
{
    if (!SaveConfig.CanSave()) return false;

    if (SaveConfig.bRequireSaveRoom)
    {
        for (const FForge2SafeZone& Zone : SafeZones)
        {
            if (Zone.IsLocationInZone(PlayerState.Location) && Zone.bCanSaveHere)
            {
                return true;
            }
        }
        return false;
    }

    return true;
}

// ============================================================================
// UForge2HorrorUtilities
// ============================================================================

FForge2MonsterDefinition UForge2HorrorUtilities::CreateSlasherMonster(const FString& Name)
{
    FForge2MonsterDefinition Monster;
    Monster.MonsterID = *Name.Replace(TEXT(" "), TEXT("_"));
    Monster.MonsterName = Name;
    Monster.MoveSpeed = 250.0f;
    Monster.ChaseSpeed = 400.0f;
    Monster.DetectionRadius = 800.0f;
    Monster.HearingRadius = 1500.0f;
    Monster.AttackRange = 150.0f;
    Monster.AttackDamage = 50.0f;
    Monster.AttackCooldown = 2.5f;
    Monster.bCanBeLookedAt = true;
    Monster.bAvoidLight = false;
    Monster.bInstantKill = false;
    Monster.FearAura = 15.0f;
    Monster.SanityDrain = 8.0f;

    return Monster;
}

FForge2MonsterDefinition UForge2HorrorUtilities::CreateGhostMonster(const FString& Name)
{
    FForge2MonsterDefinition Monster;
    Monster.MonsterID = *Name.Replace(TEXT(" "), TEXT("_"));
    Monster.MonsterName = Name;
    Monster.MoveSpeed = 150.0f;
    Monster.ChaseSpeed = 300.0f;
    Monster.DetectionRadius = 600.0f;
    Monster.HearingRadius = 2000.0f;
    Monster.AttackRange = 100.0f;
    Monster.AttackDamage = 30.0f;
    Monster.AttackCooldown = 1.5f;
    Monster.bCanBeLookedAt = true;
    Monster.bAvoidLight = true;
    Monster.LightDamage = 10.0f;
    Monster.bInstantKill = false;
    Monster.FearAura = 25.0f;
    Monster.SanityDrain = 15.0f;

    return Monster;
}

FForge2MonsterDefinition UForge2HorrorUtilities::CreateCreatureMonster(const FString& Name)
{
    FForge2MonsterDefinition Monster;
    Monster.MonsterID = *Name.Replace(TEXT(" "), TEXT("_"));
    Monster.MonsterName = Name;
    Monster.MoveSpeed = 350.0f;
    Monster.ChaseSpeed = 600.0f;
    Monster.DetectionRadius = 500.0f;
    Monster.HearingRadius = 2500.0f;
    Monster.AttackRange = 200.0f;
    Monster.AttackDamage = 100.0f;
    Monster.AttackCooldown = 3.0f;
    Monster.bCanBeLookedAt = true;
    Monster.bAvoidLight = false;
    Monster.bInstantKill = true;
    Monster.FearAura = 30.0f;
    Monster.SanityDrain = 20.0f;

    return Monster;
}

FForge2AtmosphereConfig UForge2HorrorUtilities::CreateEerieAtmosphere()
{
    FForge2AtmosphereConfig Atmosphere;
    Atmosphere.Type = EForge2AtmosphereType::Eerie;
    Atmosphere.FogColor = FLinearColor(0.1f, 0.1f, 0.15f, 1.0f);
    Atmosphere.FogDensity = 0.02f;
    Atmosphere.AmbientLightIntensity = 0.15f;
    Atmosphere.AmbientColor = FLinearColor(0.05f, 0.05f, 0.1f, 1.0f);
    Atmosphere.ShadowIntensity = 1.3f;
    Atmosphere.VignetteIntensity = 0.3f;
    Atmosphere.ChromaticAberration = 0.0f;
    Atmosphere.FilmGrain = 0.1f;

    return Atmosphere;
}

FForge2AtmosphereConfig UForge2HorrorUtilities::CreateNightmareAtmosphere()
{
    FForge2AtmosphereConfig Atmosphere;
    Atmosphere.Type = EForge2AtmosphereType::Nightmare;
    Atmosphere.FogColor = FLinearColor(0.15f, 0.0f, 0.05f, 1.0f);
    Atmosphere.FogDensity = 0.05f;
    Atmosphere.AmbientLightIntensity = 0.05f;
    Atmosphere.AmbientColor = FLinearColor(0.1f, 0.0f, 0.05f, 1.0f);
    Atmosphere.ShadowIntensity = 2.0f;
    Atmosphere.VignetteIntensity = 0.6f;
    Atmosphere.ChromaticAberration = 0.3f;
    Atmosphere.FilmGrain = 0.25f;

    return Atmosphere;
}

FForge2JumpscareDefinition UForge2HorrorUtilities::CreateBasicJumpscare(const FString& Name, float Intensity)
{
    FForge2JumpscareDefinition Jumpscare;
    Jumpscare.JumpscareID = *Name.Replace(TEXT(" "), TEXT("_"));
    Jumpscare.Duration = 0.5f;
    Jumpscare.CameraShakeIntensity = Intensity;
    Jumpscare.FearAmount = 20.0f + Intensity * 30.0f;
    Jumpscare.SanityDrain = 5.0f + Intensity * 15.0f;
    Jumpscare.bFreezePlayer = true;
    Jumpscare.bForceCamera = true;
    Jumpscare.SpawnOffset = FVector(150.0f, 0.0f, 0.0f);

    return Jumpscare;
}

FForge2HorrorEvent UForge2HorrorUtilities::CreateAmbienceEvent(const FString& Name, EForge2HorrorEventType Type)
{
    FForge2HorrorEvent Event;
    Event.EventID = *Name.Replace(TEXT(" "), TEXT("_"));
    Event.EventType = Type;
    Event.Intensity = 0.5f;
    Event.Duration = 2.0f;
    Event.FearAmount = 5.0f;
    Event.SanityDrain = 2.0f;
    Event.CooldownTime = 30.0f;
    Event.TriggerChance = 0.1f;
    Event.bRequiresDarkness = (Type == EForge2HorrorEventType::Whisper || Type == EForge2HorrorEventType::Footsteps);
    Event.bRequiresLowSanity = (Type == EForge2HorrorEventType::Hallucination || Type == EForge2HorrorEventType::VisualDistortion);
    Event.MinSanityThreshold = 50.0f;

    return Event;
}
