// Forge2StealthSystem.cpp
// FORGE2 Stealth Genre System Implementation
// Provides stealth mechanics: detection, AI awareness, patrols, distractions, cover, disguises

#include "Forge2StealthSystem.h"

// ============================================================================
// Vision Cone
// ============================================================================

bool FForge2VisionCone::IsInVisionCone(const FVector& ObserverLocation, const FVector& ObserverForward, const FVector& TargetLocation) const
{
    FVector ToTarget = TargetLocation - ObserverLocation;
    float Distance = ToTarget.Size();

    // Check distance
    if (Distance > ViewDistance) return false;

    // Check height
    float HeightDiff = FMath::Abs(ToTarget.Z);
    if (HeightDiff > HeightTolerance) return false;

    // Check angle
    ToTarget.Normalize();
    float DotProduct = FVector::DotProduct(ObserverForward, ToTarget);
    float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    return Angle <= PeripheralAngle * 0.5f;
}

float FForge2VisionCone::GetDetectionMultiplier(const FVector& ObserverLocation, const FVector& ObserverForward, const FVector& TargetLocation) const
{
    FVector ToTarget = TargetLocation - ObserverLocation;
    float Distance = ToTarget.Size();

    // Close range = instant detection
    if (Distance <= CloseRangeDistance) return 10.0f;

    // Out of range
    if (Distance > ViewDistance) return 0.0f;

    // Calculate angle
    ToTarget.Normalize();
    float DotProduct = FVector::DotProduct(ObserverForward, ToTarget);
    float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    // Out of peripheral
    if (Angle > PeripheralAngle * 0.5f) return 0.0f;

    float Multiplier = DetectionRate;

    // Distance falloff
    float DistanceRatio = 1.0f - (Distance / ViewDistance);
    Multiplier *= DistanceRatio;

    // Angle modifier
    float HalfViewAngle = ViewAngle * 0.5f;
    if (Angle > HalfViewAngle)
    {
        // In peripheral zone
        Multiplier *= PeripheralMultiplier;
    }

    return Multiplier;
}

// ============================================================================
// Hearing Config
// ============================================================================

float FForge2HearingConfig::CalculatePerceivedVolume(float SourceVolume, float Distance, int32 WallCount) const
{
    if (Distance > HearingRange) return 0.0f;

    // Distance falloff (inverse square)
    float DistanceFactor = 1.0f - (Distance / HearingRange);
    DistanceFactor = DistanceFactor * DistanceFactor;

    // Wall attenuation
    float WallFactor = FMath::Pow(WallAttenuation, (float)WallCount);

    return SourceVolume * DistanceFactor * WallFactor;
}

// ============================================================================
// Detection Meter
// ============================================================================

void FForge2DetectionMeter::AddDetection(float Amount)
{
    CurrentDetection = FMath::Clamp(CurrentDetection + Amount, 0.0f, 1.0f);
}

void FForge2DetectionMeter::UpdateDecay(float DeltaTime, float CurrentTime)
{
    if (CurrentTime - LastDetectionTime >= DecayDelay)
    {
        CurrentDetection = FMath::Max(0.0f, CurrentDetection - DecayRate * DeltaTime);
    }
}

EForge2AwarenessState FForge2DetectionMeter::GetAwarenessState() const
{
    if (CurrentDetection >= CombatThreshold) return EForge2AwarenessState::Combat;
    if (CurrentDetection >= AlertThreshold) return EForge2AwarenessState::Alerted;
    if (CurrentDetection >= SuspicionThreshold) return EForge2AwarenessState::Suspicious;
    return EForge2AwarenessState::Unaware;
}

// ============================================================================
// AI Awareness
// ============================================================================

void FForge2AIAwareness::TransitionToState(EForge2AwarenessState NewState, float CurrentTime)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        TimeInCurrentState = 0.0f;

        if (NewState == EForge2AwarenessState::Combat || NewState == EForge2AwarenessState::Alerted)
        {
            LastKnownPlayerTime = CurrentTime;
        }
    }
}

void FForge2AIAwareness::UpdateState(float DeltaTime, float CurrentTime)
{
    TimeInCurrentState += DeltaTime;

    // Update detection meter decay
    DetectionMeter.UpdateDecay(DeltaTime, CurrentTime);

    // Check for state transitions based on time
    EForge2AwarenessState MeterState = DetectionMeter.GetAwarenessState();

    switch (CurrentState)
    {
        case EForge2AwarenessState::Suspicious:
            if (TimeInCurrentState >= SuspicionDuration && MeterState == EForge2AwarenessState::Unaware)
            {
                TransitionToState(EForge2AwarenessState::Returning, CurrentTime);
            }
            break;

        case EForge2AwarenessState::Searching:
            if (TimeInCurrentState >= SearchDuration && MeterState < EForge2AwarenessState::Alerted)
            {
                TransitionToState(EForge2AwarenessState::Returning, CurrentTime);
            }
            break;

        case EForge2AwarenessState::Returning:
            // Stay in returning until patrol is resumed (handled externally)
            break;

        case EForge2AwarenessState::Distracted:
            // Handled by distraction system
            break;

        default:
            // Check if detection meter pushes to higher state
            if (MeterState > CurrentState)
            {
                TransitionToState(MeterState, CurrentTime);
            }
            break;
    }
}

bool FForge2AIAwareness::ShouldReturnToPatrol(float CurrentTime) const
{
    if (CurrentState == EForge2AwarenessState::Returning)
    {
        return true;
    }

    if (CurrentState == EForge2AwarenessState::Searching)
    {
        return TimeInCurrentState >= SearchDuration;
    }

    if (CurrentState == EForge2AwarenessState::Suspicious)
    {
        return TimeInCurrentState >= SuspicionDuration && DetectionMeter.CurrentDetection < DetectionMeter.SuspicionThreshold;
    }

    return CurrentState == EForge2AwarenessState::Unaware;
}

// ============================================================================
// Patrol Route
// ============================================================================

FForge2PatrolPoint FForge2PatrolRoute::GetCurrentPoint() const
{
    if (PatrolPoints.IsValidIndex(CurrentPointIndex))
    {
        return PatrolPoints[CurrentPointIndex];
    }
    return FForge2PatrolPoint();
}

FForge2PatrolPoint FForge2PatrolRoute::GetNextPoint()
{
    if (PatrolPoints.Num() == 0) return FForge2PatrolPoint();

    if (bLooping)
    {
        CurrentPointIndex = (CurrentPointIndex + 1) % PatrolPoints.Num();
    }
    else
    {
        // Ping-pong
        if (bReversing)
        {
            CurrentPointIndex--;
            if (CurrentPointIndex < 0)
            {
                CurrentPointIndex = 1;
                bReversing = false;
            }
        }
        else
        {
            CurrentPointIndex++;
            if (CurrentPointIndex >= PatrolPoints.Num())
            {
                CurrentPointIndex = PatrolPoints.Num() - 2;
                bReversing = true;
            }
        }
    }

    return GetCurrentPoint();
}

void FForge2PatrolRoute::ResetRoute()
{
    CurrentPointIndex = 0;
    bReversing = false;
}

// ============================================================================
// Distraction
// ============================================================================

bool FForge2Distraction::IsActive(float CurrentTime) const
{
    if (!bTriggered) return false;
    return (CurrentTime - TriggerTime) < Duration;
}

float FForge2Distraction::GetAttractionStrength(float Distance) const
{
    if (Distance > EffectRadius) return 0.0f;
    return 1.0f - (Distance / EffectRadius);
}

// ============================================================================
// Cover Point
// ============================================================================

bool FForge2CoverPoint::ProvidesCoverFrom(const FVector& ThreatLocation) const
{
    // Check if threat is on the opposite side of cover
    FVector ToThreat = ThreatLocation - Location;
    ToThreat.Z = 0.0f;
    ToThreat.Normalize();

    FVector CoverForward = CoverDirection.Vector();
    CoverForward.Z = 0.0f;
    CoverForward.Normalize();

    float DotProduct = FVector::DotProduct(CoverForward, ToThreat);

    // Cover provides protection if threat is in front of cover direction
    return DotProduct > 0.0f;
}

float FForge2CoverPoint::GetCoverEffectiveness(const FVector& ThreatLocation) const
{
    if (!ProvidesCoverFrom(ThreatLocation)) return 0.0f;

    FVector ToThreat = ThreatLocation - Location;
    float Distance = ToThreat.Size();

    // Effectiveness decreases with distance (bullets have arc)
    float DistanceEffectiveness = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.3f, 1.0f);

    // Cover type effectiveness
    float TypeEffectiveness = 1.0f;
    switch (CoverType)
    {
        case EForge2CoverType::Low:
            TypeEffectiveness = 0.7f;
            break;
        case EForge2CoverType::High:
            TypeEffectiveness = 0.95f;
            break;
        case EForge2CoverType::Corner:
            TypeEffectiveness = 0.85f;
            break;
        case EForge2CoverType::Foliage:
            TypeEffectiveness = 0.4f; // Concealment, not cover
            break;
        case EForge2CoverType::Darkness:
            TypeEffectiveness = 0.2f; // Concealment only
            break;
        default:
            break;
    }

    return DistanceEffectiveness * TypeEffectiveness;
}

// ============================================================================
// Disguise
// ============================================================================

bool FForge2Disguise::CanAccessArea(FName AreaID) const
{
    return AllowedAreas.Contains(AreaID);
}

bool FForge2Disguise::IsActionSuspicious(FName ActionID) const
{
    return SuspiciousActions.Contains(ActionID);
}

// ============================================================================
// Disguise State
// ============================================================================

void FForge2DisguiseState::AddSuspicion(float Amount)
{
    SuspicionLevel = FMath::Clamp(SuspicionLevel + Amount, 0.0f, BlownThreshold);
    if (SuspicionLevel >= BlownThreshold)
    {
        bDisguiseBlown = true;
    }
}

void FForge2DisguiseState::ClearSuspicion()
{
    SuspicionLevel = 0.0f;
    // Note: Once blown, disguise stays blown unless changed
}

// ============================================================================
// Security Device
// ============================================================================

bool FForge2SecurityDevice::IsActiveAtTime(float Time) const
{
    if (!bEnabled || bHacked || bDestroyed) return false;
    if (!bTimed) return true;

    float CycleTime = ActiveDuration + InactiveDuration;
    if (CycleTime <= 0.0f) return true;

    float Phase = FMath::Fmod(Time + PhaseOffset, CycleTime);
    return Phase < ActiveDuration;
}

FRotator FForge2SecurityDevice::GetCameraRotationAtTime(float Time) const
{
    if (SecurityType != EForge2SecurityType::Camera || CameraSweepAngle <= 0.0f)
    {
        return Rotation;
    }

    // Calculate sweep position
    float SweepTime = CameraSweepAngle / CameraSweepSpeed;
    float CycleTime = SweepTime * 2.0f; // Back and forth

    float Phase = FMath::Fmod(Time, CycleTime);
    float SweepProgress;

    if (Phase < SweepTime)
    {
        SweepProgress = Phase / SweepTime;
    }
    else
    {
        SweepProgress = 1.0f - (Phase - SweepTime) / SweepTime;
    }

    float YawOffset = FMath::Lerp(-CameraSweepAngle * 0.5f, CameraSweepAngle * 0.5f, SweepProgress);

    FRotator CurrentRotation = Rotation;
    CurrentRotation.Yaw += YawOffset;

    return CurrentRotation;
}

bool FForge2SecurityDevice::CheckDetection(const FVector& TargetLocation, float Time) const
{
    if (!IsActiveAtTime(Time)) return false;

    switch (SecurityType)
    {
        case EForge2SecurityType::Camera:
        {
            // Check if target is within camera FOV
            FVector ToTarget = TargetLocation - Location;
            float Distance = ToTarget.Size();
            if (Distance > CameraRange) return false;

            ToTarget.Normalize();
            FRotator CurrentRotation = GetCameraRotationAtTime(Time);
            FVector CameraForward = CurrentRotation.Vector();

            float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
            float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

            return Angle <= CameraFOV * 0.5f;
        }

        case EForge2SecurityType::Laser:
        {
            // Check if target intersects laser beam
            FVector LaserDir = (LaserEndPoint - Location).GetSafeNormal();
            float LaserLength = FVector::Distance(Location, LaserEndPoint);

            FVector ToTarget = TargetLocation - Location;
            float ProjectedDistance = FVector::DotProduct(ToTarget, LaserDir);

            if (ProjectedDistance < 0.0f || ProjectedDistance > LaserLength) return false;

            FVector ClosestPoint = Location + LaserDir * ProjectedDistance;
            float DistanceToLaser = FVector::Distance(TargetLocation, ClosestPoint);

            return DistanceToLaser < 50.0f; // Laser width tolerance
        }

        case EForge2SecurityType::PressurePlate:
        case EForge2SecurityType::MotionSensor:
        {
            float Distance = FVector::Distance(Location, TargetLocation);
            return Distance <= SensorRadius;
        }

        default:
            return false;
    }
}

// ============================================================================
// Alarm State
// ============================================================================

void FForge2AlarmState::TriggerAlarm(const FVector& Location, float CurrentTime)
{
    if (!bAlarmActive)
    {
        bAlarmActive = true;
        AlarmStartTime = CurrentTime;
        AlarmTriggerLocation = Location;
        AlarmLevel = 1;
    }
}

void FForge2AlarmState::EscalateAlarm()
{
    if (AlarmLevel < MaxAlarmLevel)
    {
        AlarmLevel++;
    }

    if (AlarmLevel >= MaxAlarmLevel)
    {
        bLockdownActive = true;
    }
}

void FForge2AlarmState::DisableAlarm()
{
    bAlarmActive = false;
    AlarmLevel = 0;
    bLockdownActive = false;
}

bool FForge2AlarmState::IsAlarmExpired(float CurrentTime) const
{
    if (!bAlarmActive) return true;
    return (CurrentTime - AlarmStartTime) >= AlarmDuration;
}

// ============================================================================
// Light Level
// ============================================================================

float FForge2LightLevel::GetVisibilityMultiplier() const
{
    if (IsInDarkness()) return 0.2f;
    if (IsInShadow()) return 0.5f;
    return 1.0f + (TotalLight - ShadowThreshold) * 0.5f;
}

// ============================================================================
// Light Source
// ============================================================================

float FForge2LightSource::GetLightContribution(const FVector& TargetLocation) const
{
    if (!bEnabled) return 0.0f;

    float Distance = FVector::Distance(Location, TargetLocation);
    if (Distance > Radius) return 0.0f;

    // Inverse square falloff
    float Falloff = 1.0f - (Distance / Radius);
    Falloff = Falloff * Falloff;

    return Intensity * Falloff;
}

// ============================================================================
// Takedown Definition
// ============================================================================

bool FForge2TakedownDefinition::CanPerform(const FVector& PlayerLocation, const FVector& TargetLocation, const FVector& TargetForward) const
{
    // Check distance
    float Distance = FVector::Distance(PlayerLocation, TargetLocation);
    if (Distance > Range) return false;

    if (bRequiresBehind)
    {
        // Check if player is behind target
        FVector ToPlayer = PlayerLocation - TargetLocation;
        ToPlayer.Normalize();

        float DotProduct = FVector::DotProduct(TargetForward, ToPlayer);
        float AngleToPlayer = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

        // Player must be behind (angle > 90 degrees from forward)
        if (AngleToPlayer < 90.0f) return false;

        // Check angle tolerance
        float BehindAngle = 180.0f - AngleToPlayer;
        if (BehindAngle > Angle * 0.5f) return false;
    }

    return true;
}

// ============================================================================
// NPC Definition
// ============================================================================

bool FForge2StealthNPCDefinition::CanSeeThrough(const FForge2Disguise& TargetDisguise) const
{
    return CanDetectDisguises.Contains(TargetDisguise.DisguiseID);
}

// ============================================================================
// Player State
// ============================================================================

float FForge2StealthPlayerState::GetVisibilityScore() const
{
    float Score = 1.0f;

    // Light level
    Score *= CurrentLightLevel.GetVisibilityMultiplier();

    // Cover
    if (bInCover) Score *= 0.3f;

    // Crouching
    if (bCrouching) Score *= 0.7f;

    // Movement
    if (bRunning) Score *= 1.5f;
    else if (bMoving) Score *= 1.2f;

    // Carrying body
    if (bCarryingBody) Score *= 1.3f;

    return Score;
}

float FForge2StealthPlayerState::GetNoiseMultiplier() const
{
    float Multiplier = 1.0f;

    if (bCrouching) Multiplier *= 0.5f;
    if (bRunning) Multiplier *= 2.0f;
    if (bCarryingBody) Multiplier *= 1.2f;

    return Multiplier;
}

// ============================================================================
// Stealth Manager
// ============================================================================

UForge2StealthManager::UForge2StealthManager()
{
    CurrentTime = 0.0f;
}

void UForge2StealthManager::InitializeLevel(const FForge2StealthLevelDefinition& Level)
{
    CurrentLevel = Level;
    CurrentTime = 0.0f;

    // Reset all NPC states
    for (FForge2StealthNPCDefinition& NPC : CurrentLevel.NPCs)
    {
        NPC.Awareness.CurrentState = EForge2AwarenessState::Unaware;
        NPC.Awareness.DetectionMeter.CurrentDetection = 0.0f;
        NPC.PatrolRoute.ResetRoute();
    }

    // Reset security devices
    for (FForge2SecurityDevice& Device : CurrentLevel.SecurityDevices)
    {
        Device.bHacked = false;
        Device.bDestroyed = false;
    }

    // Reset alarm
    CurrentLevel.AlarmState.DisableAlarm();

    // Reset stats
    CurrentLevel.TimesDetected = 0;
    CurrentLevel.KillCount = 0;
    CurrentLevel.KnockoutCount = 0;
    CurrentLevel.BodiesFound = 0;
    CurrentLevel.AlarmsTriggered = 0;
}

void UForge2StealthManager::UpdateNPCAwareness(float DeltaTime)
{
    CurrentTime += DeltaTime;

    for (FForge2StealthNPCDefinition& NPC : CurrentLevel.NPCs)
    {
        // Update awareness state
        NPC.Awareness.UpdateState(DeltaTime, CurrentTime);

        // Check if NPC should return to patrol
        if (NPC.Awareness.ShouldReturnToPatrol(CurrentTime))
        {
            NPC.Awareness.TransitionToState(EForge2AwarenessState::Unaware, CurrentTime);
        }
    }
}

void UForge2StealthManager::UpdateSecurityDevices(float DeltaTime)
{
    // Check for player detection by security devices
    for (FForge2SecurityDevice& Device : CurrentLevel.SecurityDevices)
    {
        if (Device.CheckDetection(PlayerState.Location, CurrentTime))
        {
            if (Device.bTriggerAlarm && !CurrentLevel.AlarmState.bAlarmActive)
            {
                // Delay before alarm
                // In a full implementation, this would track per-device countdown
                CurrentLevel.AlarmState.TriggerAlarm(PlayerState.Location, CurrentTime);
                CurrentLevel.AlarmsTriggered++;
            }
        }
    }

    // Update alarm state
    if (CurrentLevel.AlarmState.bAlarmActive)
    {
        if (CurrentLevel.AlarmState.IsAlarmExpired(CurrentTime))
        {
            CurrentLevel.AlarmState.DisableAlarm();
        }
    }
}

void UForge2StealthManager::UpdateLightLevels()
{
    PlayerState.CurrentLightLevel = GetLightLevelAtLocation(PlayerState.Location);
}

void UForge2StealthManager::UpdateDistractions(float DeltaTime)
{
    // Distractions would be tracked externally and processed here
}

bool UForge2StealthManager::CheckPlayerDetection(int32 NPCIndex)
{
    if (!CurrentLevel.NPCs.IsValidIndex(NPCIndex)) return false;

    FForge2StealthNPCDefinition& NPC = CurrentLevel.NPCs[NPCIndex];

    // Skip if NPC actor is invalid or incapacitated
    if (!NPC.NPCActor.IsValid())
    {
        return false;
    }
    
    // Check if NPC is incapacitated through their state (via a component or interface)
    if (IForge2StealthSubject* StealthSubject = Cast<IForge2StealthSubject>(NPC.NPCActor.Get()))
    {
        if (StealthSubject->IsIncapacitated())
        {
            return false;
        }
    }

    // Get NPC location and forward from the actual actor
    FVector NPCLocation = NPC.GetLocation();
    FVector NPCForward = NPC.GetForward();

    // Check vision detection
    if (NPC.VisionCone.IsInVisionCone(NPCLocation, NPCForward, PlayerState.Location))
    {
        // Line of sight check would happen here

        float DetectionRate = NPC.VisionCone.GetDetectionMultiplier(NPCLocation, NPCForward, PlayerState.Location);

        // Modify by player visibility
        DetectionRate *= PlayerState.GetVisibilityScore();

        // Modify by disguise
        if (PlayerState.DisguiseState.CurrentDisguise.DisguiseType != EForge2DisguiseType::None)
        {
            if (!NPC.CanSeeThrough(PlayerState.DisguiseState.CurrentDisguise))
            {
                DetectionRate *= PlayerState.DisguiseState.CurrentDisguise.DetectionMultiplier;
            }
        }

        // Add detection
        if (DetectionRate > 0.0f)
        {
            NPC.Awareness.DetectionMeter.AddDetection(DetectionRate * 0.016f); // Assuming 60fps
            NPC.Awareness.DetectionMeter.LastDetectionTime = CurrentTime;

            if (NPC.Awareness.DetectionMeter.CurrentDetection >= NPC.Awareness.DetectionMeter.AlertThreshold)
            {
                NPC.Awareness.LastKnownPlayerLocation = PlayerState.Location;
                NPC.Awareness.LastKnownPlayerTime = CurrentTime;
                CurrentLevel.TimesDetected++;
                return true;
            }
        }
    }

    return false;
}

void UForge2StealthManager::ProcessSoundAtLocation(const FVector& Location, float Volume, EForge2NoiseLevel NoiseLevel)
{
    for (FForge2StealthNPCDefinition& NPC : CurrentLevel.NPCs)
    {
        // Get NPC location (placeholder)
        FVector NPCLocation = FVector::ZeroVector;

        float PerceivedVolume = NPC.HearingConfig.CalculatePerceivedVolume(Volume, FVector::Distance(NPCLocation, Location), 0);

        if (PerceivedVolume > NPC.HearingConfig.SilentThreshold)
        {
            NPC.Awareness.LastHeardSoundLocation = Location;
            NPC.Awareness.LastHeardSoundTime = CurrentTime;

            // Determine detection amount based on noise level
            float DetectionAmount = 0.0f;
            switch (NoiseLevel)
            {
                case EForge2NoiseLevel::Quiet:
                    DetectionAmount = 0.1f;
                    break;
                case EForge2NoiseLevel::Normal:
                    DetectionAmount = 0.2f;
                    break;
                case EForge2NoiseLevel::Loud:
                    DetectionAmount = 0.4f;
                    break;
                case EForge2NoiseLevel::VeryLoud:
                    DetectionAmount = 0.7f;
                    break;
                default:
                    break;
            }

            NPC.Awareness.DetectionMeter.AddDetection(DetectionAmount * PerceivedVolume);

            // Transition to suspicious if heard something
            if (NPC.Awareness.CurrentState == EForge2AwarenessState::Unaware &&
                NPC.Awareness.DetectionMeter.CurrentDetection > NPC.Awareness.DetectionMeter.SuspicionThreshold)
            {
                NPC.Awareness.TransitionToState(EForge2AwarenessState::Suspicious, CurrentTime);
            }
        }
    }
}

bool UForge2StealthManager::TriggerDistraction(const FForge2Distraction& Distraction)
{
    // Process distraction for all NPCs in range
    int32 AffectedCount = 0;

    for (FForge2StealthNPCDefinition& NPC : CurrentLevel.NPCs)
    {
        if (!NPC.bCanBeDistracted) continue;
        if (AffectedCount >= Distraction.MaxAffectedAI) break;

        // Get NPC location (placeholder)
        FVector NPCLocation = FVector::ZeroVector;

        float Attraction = Distraction.GetAttractionStrength(FVector::Distance(NPCLocation, Distraction.Location));
        if (Attraction > 0.3f)
        {
            NPC.Awareness.TransitionToState(EForge2AwarenessState::Distracted, CurrentTime);
            NPC.Awareness.LastHeardSoundLocation = Distraction.Location;
            AffectedCount++;
        }
    }

    return AffectedCount > 0;
}

bool UForge2StealthManager::PerformTakedown(int32 NPCIndex, EForge2TakedownType TakedownType)
{
    if (!CurrentLevel.NPCs.IsValidIndex(NPCIndex)) return false;

    FForge2StealthNPCDefinition& NPC = CurrentLevel.NPCs[NPCIndex];

    if (!NPC.bCanBeTakenDown) return false;

    // Would check takedown conditions here

    switch (TakedownType)
    {
        case EForge2TakedownType::NonLethal:
            CurrentLevel.KnockoutCount++;
            break;
        case EForge2TakedownType::Lethal:
            if (NPC.bIsEssential) return false;
            CurrentLevel.KillCount++;
            break;
        default:
            break;
    }

    return true;
}

bool UForge2StealthManager::HideBody(FName BodyID, FName HidingSpotID)
{
    FForge2HidingSpot* Spot = nullptr;
    for (FForge2HidingSpot& S : CurrentLevel.HidingSpots)
    {
        if (S.SpotID == HidingSpotID)
        {
            Spot = &S;
            break;
        }
    }

    if (!Spot || !Spot->CanHideBody()) return false;

    Spot->CurrentCount++;
    return true;
}

bool UForge2StealthManager::ChangeDisguise(FName DisguiseID)
{
    for (const FForge2Disguise& Disguise : CurrentLevel.AvailableDisguises)
    {
        if (Disguise.DisguiseID == DisguiseID)
        {
            PlayerState.DisguiseState.CurrentDisguise = Disguise;
            PlayerState.DisguiseState.SuspicionLevel = 0.0f;
            PlayerState.DisguiseState.bDisguiseBlown = false;
            PlayerState.DisguiseState.WitnessedBy.Empty();
            return true;
        }
    }
    return false;
}

bool UForge2StealthManager::HackSecurityDevice(FName DeviceID)
{
    for (FForge2SecurityDevice& Device : CurrentLevel.SecurityDevices)
    {
        if (Device.DeviceID == DeviceID)
        {
            Device.bHacked = true;
            return true;
        }
    }
    return false;
}

bool UForge2StealthManager::DisableSecurityDevice(FName DeviceID)
{
    for (FForge2SecurityDevice& Device : CurrentLevel.SecurityDevices)
    {
        if (Device.DeviceID == DeviceID)
        {
            Device.bEnabled = false;
            return true;
        }
    }
    return false;
}

bool UForge2StealthManager::EnterCover(FName CoverID)
{
    for (FForge2CoverPoint& Cover : CurrentLevel.CoverPoints)
    {
        if (Cover.CoverID == CoverID && !Cover.bOccupied)
        {
            Cover.bOccupied = true;
            PlayerState.bInCover = true;
            PlayerState.CurrentCoverID = CoverID;
            return true;
        }
    }
    return false;
}

void UForge2StealthManager::ExitCover()
{
    for (FForge2CoverPoint& Cover : CurrentLevel.CoverPoints)
    {
        if (Cover.CoverID == PlayerState.CurrentCoverID)
        {
            Cover.bOccupied = false;
            break;
        }
    }

    PlayerState.bInCover = false;
    PlayerState.CurrentCoverID = NAME_None;
}

FForge2LightLevel UForge2StealthManager::GetLightLevelAtLocation(const FVector& Location) const
{
    FForge2LightLevel LightLevel;
    LightLevel.AmbientLight = CurrentLevel.AmbientLightLevel;
    LightLevel.DirectLight = 0.0f;

    for (const FForge2LightSource& Light : CurrentLevel.LightSources)
    {
        LightLevel.DirectLight += Light.GetLightContribution(Location);
    }

    LightLevel.TotalLight = FMath::Clamp(LightLevel.AmbientLight + LightLevel.DirectLight, 0.0f, 1.0f);

    return LightLevel;
}

TArray<FForge2CoverPoint> UForge2StealthManager::GetNearbyCover(const FVector& Location, float Radius) const
{
    TArray<FForge2CoverPoint> NearbyCover;

    for (const FForge2CoverPoint& Cover : CurrentLevel.CoverPoints)
    {
        if (FVector::Distance(Location, Cover.Location) <= Radius && !Cover.bOccupied)
        {
            NearbyCover.Add(Cover);
        }
    }

    return NearbyCover;
}

bool UForge2StealthManager::IsInRestrictedArea(const FVector& Location, const FForge2Disguise& Disguise) const
{
    // Would check area boundaries and disguise permissions
    return false;
}

int32 UForge2StealthManager::CalculateStealthRating() const
{
    int32 Rating = 100;

    // Deductions
    Rating -= CurrentLevel.TimesDetected * 10;
    Rating -= CurrentLevel.AlarmsTriggered * 15;
    Rating -= CurrentLevel.BodiesFound * 5;

    // Bonuses
    if (IsGhostRun()) Rating += 20;
    if (IsSilentAssassinRun()) Rating += 10;

    return FMath::Clamp(Rating, 0, 100);
}

bool UForge2StealthManager::IsGhostRun() const
{
    return CurrentLevel.TimesDetected == 0 &&
           CurrentLevel.AlarmsTriggered == 0 &&
           CurrentLevel.BodiesFound == 0;
}

bool UForge2StealthManager::IsSilentAssassinRun() const
{
    return CurrentLevel.TimesDetected == 0 &&
           CurrentLevel.AlarmsTriggered == 0 &&
           CurrentLevel.KillCount == 0;
}

// ============================================================================
// Utility Functions
// ============================================================================

FForge2VisionCone UForge2StealthUtilities::CreateStandardVision()
{
    FForge2VisionCone Vision;
    Vision.ViewAngle = 90.0f;
    Vision.ViewDistance = 1500.0f;
    Vision.PeripheralAngle = 160.0f;
    Vision.PeripheralMultiplier = 0.3f;
    Vision.CloseRangeDistance = 200.0f;
    Vision.DetectionRate = 1.0f;
    return Vision;
}

FForge2VisionCone UForge2StealthUtilities::CreateAlertedVision()
{
    FForge2VisionCone Vision = CreateStandardVision();
    Vision.ViewAngle = 120.0f;
    Vision.ViewDistance = 2000.0f;
    Vision.PeripheralAngle = 180.0f;
    Vision.PeripheralMultiplier = 0.5f;
    Vision.DetectionRate = 2.0f;
    return Vision;
}

FForge2VisionCone UForge2StealthUtilities::CreateSniper()
{
    FForge2VisionCone Vision;
    Vision.ViewAngle = 30.0f;
    Vision.ViewDistance = 5000.0f;
    Vision.PeripheralAngle = 60.0f;
    Vision.PeripheralMultiplier = 0.1f;
    Vision.CloseRangeDistance = 100.0f;
    Vision.DetectionRate = 2.0f;
    return Vision;
}

FForge2HearingConfig UForge2StealthUtilities::CreateStandardHearing()
{
    FForge2HearingConfig Hearing;
    Hearing.HearingRange = 1000.0f;
    Hearing.HearingAccuracy = 0.8f;
    return Hearing;
}

FForge2HearingConfig UForge2StealthUtilities::CreateAlertedHearing()
{
    FForge2HearingConfig Hearing;
    Hearing.HearingRange = 1500.0f;
    Hearing.HearingAccuracy = 0.9f;
    Hearing.SilentThreshold = 0.05f;
    return Hearing;
}

FForge2StealthNPCDefinition UForge2StealthUtilities::CreateBasicGuard()
{
    FForge2StealthNPCDefinition NPC;
    NPC.NPCID = FName(*FString::Printf(TEXT("Guard_%d"), FMath::Rand()));
    NPC.NPCName = TEXT("Guard");
    NPC.VisionCone = CreateStandardVision();
    NPC.HearingConfig = CreateStandardHearing();
    NPC.bCanBeDistracted = true;
    NPC.bCanBeTakenDown = true;
    NPC.ReactionTime = 0.5f;
    NPC.CombatSkill = 0.5f;
    return NPC;
}

FForge2StealthNPCDefinition UForge2StealthUtilities::CreateEliteGuard()
{
    FForge2StealthNPCDefinition NPC = CreateBasicGuard();
    NPC.NPCName = TEXT("Elite Guard");
    NPC.VisionCone = CreateAlertedVision();
    NPC.HearingConfig = CreateAlertedHearing();
    NPC.ReactionTime = 0.3f;
    NPC.CombatSkill = 0.8f;
    NPC.Awareness.bCanCallReinforcements = true;
    return NPC;
}

FForge2StealthNPCDefinition UForge2StealthUtilities::CreateCivilian()
{
    FForge2StealthNPCDefinition NPC;
    NPC.NPCID = FName(*FString::Printf(TEXT("Civilian_%d"), FMath::Rand()));
    NPC.NPCName = TEXT("Civilian");
    NPC.VisionCone.ViewDistance = 1000.0f;
    NPC.VisionCone.DetectionRate = 0.5f;
    NPC.HearingConfig.HearingRange = 500.0f;
    NPC.bCanBeDistracted = true;
    NPC.bCanBeTakenDown = true;
    NPC.ReactionTime = 1.0f;
    NPC.CombatSkill = 0.1f;
    return NPC;
}

FForge2SecurityDevice UForge2StealthUtilities::CreateSecurityCamera(const FVector& Location, const FRotator& Rotation, float SweepAngle)
{
    FForge2SecurityDevice Device;
    Device.DeviceID = FName(*FString::Printf(TEXT("Camera_%d"), FMath::Rand()));
    Device.SecurityType = EForge2SecurityType::Camera;
    Device.Location = Location;
    Device.Rotation = Rotation;
    Device.CameraFOV = 60.0f;
    Device.CameraRange = 1000.0f;
    Device.CameraSweepAngle = SweepAngle;
    Device.CameraSweepSpeed = 30.0f;
    Device.bTriggerAlarm = true;
    Device.AlarmDelay = 2.0f;
    return Device;
}

FForge2SecurityDevice UForge2StealthUtilities::CreateLaserTripwire(const FVector& Start, const FVector& End)
{
    FForge2SecurityDevice Device;
    Device.DeviceID = FName(*FString::Printf(TEXT("Laser_%d"), FMath::Rand()));
    Device.SecurityType = EForge2SecurityType::Laser;
    Device.Location = Start;
    Device.LaserEndPoint = End;
    Device.bTriggerAlarm = true;
    Device.AlarmDelay = 0.0f; // Instant
    return Device;
}

FForge2SecurityDevice UForge2StealthUtilities::CreateMotionSensor(const FVector& Location, float Radius)
{
    FForge2SecurityDevice Device;
    Device.DeviceID = FName(*FString::Printf(TEXT("Motion_%d"), FMath::Rand()));
    Device.SecurityType = EForge2SecurityType::MotionSensor;
    Device.Location = Location;
    Device.SensorRadius = Radius;
    Device.bTriggerAlarm = true;
    Device.AlarmDelay = 1.0f;
    return Device;
}

FForge2Distraction UForge2StealthUtilities::CreateNoiseDistraction(const FVector& Location, float Volume)
{
    FForge2Distraction Distraction;
    Distraction.DistractionID = FName(*FString::Printf(TEXT("Noise_%d"), FMath::Rand()));
    Distraction.DistractionType = EForge2DistractionType::Noise;
    Distraction.Location = Location;
    Distraction.NoiseLevel = Volume;
    Distraction.EffectRadius = 500.0f * Volume;
    Distraction.Duration = 3.0f;
    Distraction.MaxAffectedAI = 2;
    return Distraction;
}

FForge2Distraction UForge2StealthUtilities::CreateDecoyDistraction(const FVector& Location)
{
    FForge2Distraction Distraction;
    Distraction.DistractionID = FName(*FString::Printf(TEXT("Decoy_%d"), FMath::Rand()));
    Distraction.DistractionType = EForge2DistractionType::Decoy;
    Distraction.Location = Location;
    Distraction.EffectRadius = 300.0f;
    Distraction.Duration = 10.0f;
    Distraction.MaxAffectedAI = 1;
    return Distraction;
}

FForge2CoverPoint UForge2StealthUtilities::CreateLowCover(const FVector& Location, const FRotator& Direction)
{
    FForge2CoverPoint Cover;
    Cover.CoverID = FName(*FString::Printf(TEXT("LowCover_%d"), FMath::Rand()));
    Cover.CoverType = EForge2CoverType::Low;
    Cover.Location = Location;
    Cover.CoverDirection = Direction;
    Cover.CoverHeight = 100.0f;
    Cover.CoverWidth = 150.0f;
    Cover.bCanPeekOver = true;
    return Cover;
}

FForge2CoverPoint UForge2StealthUtilities::CreateHighCover(const FVector& Location, const FRotator& Direction)
{
    FForge2CoverPoint Cover;
    Cover.CoverID = FName(*FString::Printf(TEXT("HighCover_%d"), FMath::Rand()));
    Cover.CoverType = EForge2CoverType::High;
    Cover.Location = Location;
    Cover.CoverDirection = Direction;
    Cover.CoverHeight = 200.0f;
    Cover.CoverWidth = 200.0f;
    Cover.bCanPeekLeft = true;
    Cover.bCanPeekRight = true;
    Cover.bCanBlindFire = true;
    return Cover;
}

FForge2CoverPoint UForge2StealthUtilities::CreateCornerCover(const FVector& Location, const FRotator& Direction)
{
    FForge2CoverPoint Cover;
    Cover.CoverID = FName(*FString::Printf(TEXT("Corner_%d"), FMath::Rand()));
    Cover.CoverType = EForge2CoverType::Corner;
    Cover.Location = Location;
    Cover.CoverDirection = Direction;
    Cover.CoverHeight = 200.0f;
    Cover.CoverWidth = 100.0f;
    Cover.bCanPeekLeft = false;
    Cover.bCanPeekRight = true; // Assuming right-side corner
    return Cover;
}

FForge2Disguise UForge2StealthUtilities::CreateGuardDisguise()
{
    FForge2Disguise Disguise;
    Disguise.DisguiseID = FName(TEXT("GuardDisguise"));
    Disguise.DisguiseName = TEXT("Security Guard");
    Disguise.DisguiseType = EForge2DisguiseType::Guard;
    Disguise.AllowedAreas.Add(FName(TEXT("PublicArea")));
    Disguise.AllowedAreas.Add(FName(TEXT("GuardArea")));
    Disguise.SuspiciousActions.Add(FName(TEXT("Running")));
    Disguise.SuspiciousActions.Add(FName(TEXT("Trespassing")));
    Disguise.DetectionMultiplier = 0.3f;
    return Disguise;
}

FForge2Disguise UForge2StealthUtilities::CreateCivilianDisguise()
{
    FForge2Disguise Disguise;
    Disguise.DisguiseID = FName(TEXT("CivilianDisguise"));
    Disguise.DisguiseName = TEXT("Civilian");
    Disguise.DisguiseType = EForge2DisguiseType::Civilian;
    Disguise.AllowedAreas.Add(FName(TEXT("PublicArea")));
    Disguise.SuspiciousActions.Add(FName(TEXT("Running")));
    Disguise.SuspiciousActions.Add(FName(TEXT("Trespassing")));
    Disguise.SuspiciousActions.Add(FName(TEXT("GuardArea")));
    Disguise.DetectionMultiplier = 0.5f;
    return Disguise;
}

FForge2Disguise UForge2StealthUtilities::CreateWorkerDisguise()
{
    FForge2Disguise Disguise;
    Disguise.DisguiseID = FName(TEXT("WorkerDisguise"));
    Disguise.DisguiseName = TEXT("Maintenance Worker");
    Disguise.DisguiseType = EForge2DisguiseType::Worker;
    Disguise.AllowedAreas.Add(FName(TEXT("PublicArea")));
    Disguise.AllowedAreas.Add(FName(TEXT("MaintenanceArea")));
    Disguise.AllowedAreas.Add(FName(TEXT("BackstageArea")));
    Disguise.SuspiciousActions.Add(FName(TEXT("Running")));
    Disguise.SuspiciousActions.Add(FName(TEXT("GuardArea")));
    Disguise.DetectionMultiplier = 0.4f;
    return Disguise;
}
