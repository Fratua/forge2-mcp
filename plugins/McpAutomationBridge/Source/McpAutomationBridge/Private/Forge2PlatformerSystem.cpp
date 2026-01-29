// Forge2PlatformerSystem.cpp
// FORGE2 Platformer Genre System Implementation
// Provides platformer mechanics: movement, jumping, platforms, hazards, collectibles

#include "Forge2PlatformerSystem.h"

// ============================================================================
// Jump Configuration
// ============================================================================

float FForge2JumpConfig::CalculateGravity() const
{
    // Gravity = 2 * JumpHeight / TimeToApex^2
    if (TimeToApex <= 0.0f) return 980.0f;
    return (2.0f * JumpHeight) / (TimeToApex * TimeToApex);
}

float FForge2JumpConfig::CalculateJumpVelocity() const
{
    // JumpVelocity = 2 * JumpHeight / TimeToApex
    if (TimeToApex <= 0.0f) return 600.0f;
    return (2.0f * JumpHeight) / TimeToApex;
}

// ============================================================================
// Movement Configuration
// ============================================================================

float FForge2MovementConfig::GetEffectiveSpeed(bool bSprinting, bool bCrouching) const
{
    if (bCrouching) return MaxSpeed * CrouchSpeedMultiplier;
    if (bSprinting) return MaxSpeed * SprintSpeedMultiplier;
    return MaxSpeed;
}

float FForge2MovementConfig::GetAccelerationTime() const
{
    if (Acceleration <= 0.0f) return 0.0f;
    return MaxSpeed / Acceleration;
}

float FForge2MovementConfig::GetStoppingDistance() const
{
    if (Deceleration <= 0.0f) return FLT_MAX;
    return (MaxSpeed * MaxSpeed) / (2.0f * Deceleration);
}

// ============================================================================
// Platform Definition
// ============================================================================

FVector FForge2PlatformDefinition::GetPositionAtTime(float Time) const
{
    if (PlatformType == EForge2PlatformType::Static || MovementPath.Num() < 2)
    {
        return Location;
    }

    // Calculate total path length
    float TotalLength = 0.0f;
    for (int32 i = 0; i < MovementPath.Num() - 1; i++)
    {
        TotalLength += FVector::Distance(MovementPath[i], MovementPath[i + 1]);
    }

    // Time for one cycle
    float CycleTime = TotalLength / MoveSpeed;
    if (CycleTime <= 0.0f) return Location;

    // Normalize time within cycle
    float NormalizedTime = FMath::Fmod(Time, CycleTime * 2.0f); // *2 for round trip
    bool bReverse = NormalizedTime > CycleTime;
    if (bReverse) NormalizedTime = CycleTime * 2.0f - NormalizedTime;

    // Find position along path
    float TargetDistance = NormalizedTime * MoveSpeed;
    float AccumulatedDistance = 0.0f;

    for (int32 i = 0; i < MovementPath.Num() - 1; i++)
    {
        float SegmentLength = FVector::Distance(MovementPath[i], MovementPath[i + 1]);
        if (AccumulatedDistance + SegmentLength >= TargetDistance)
        {
            float SegmentProgress = (TargetDistance - AccumulatedDistance) / SegmentLength;
            return FMath::Lerp(MovementPath[i], MovementPath[i + 1], SegmentProgress);
        }
        AccumulatedDistance += SegmentLength;
    }

    return MovementPath.Last();
}

bool FForge2PlatformDefinition::IsSafeToStandOn(float Time) const
{
    switch (PlatformType)
    {
        case EForge2PlatformType::Falling:
            return !bTriggered; // Safe until triggered

        case EForge2PlatformType::Crumbling:
            return CurrentHealth > 0.0f;

        case EForge2PlatformType::Phasing:
        {
            float CycleTime = PhaseOnTime + PhaseOffTime;
            if (CycleTime <= 0.0f) return true;
            float Phase = FMath::Fmod(Time, CycleTime);
            return Phase < PhaseOnTime;
        }

        case EForge2PlatformType::Spike:
        {
            float CycleTime = SpikeActiveTime + SpikeInactiveTime;
            if (CycleTime <= 0.0f) return false;
            float Phase = FMath::Fmod(Time, CycleTime);
            return Phase >= SpikeActiveTime; // Safe during inactive time
        }

        default:
            return true;
    }
}

float FForge2PlatformDefinition::GetSurfaceModifier() const
{
    switch (PlatformType)
    {
        case EForge2PlatformType::Ice:
            return IceFrictionMultiplier;

        case EForge2PlatformType::Sticky:
            return 0.5f; // Reduced speed on sticky

        default:
            return 1.0f;
    }
}

// ============================================================================
// Hazard Definition
// ============================================================================

bool FForge2HazardDefinition::IsActiveAtTime(float Time) const
{
    if (!bTimed) return true;

    float CycleTime = ActiveDuration + InactiveDuration;
    if (CycleTime <= 0.0f) return true;

    float Phase = FMath::Fmod(Time + PhaseOffset, CycleTime);
    return Phase < ActiveDuration;
}

FVector FForge2HazardDefinition::GetPositionAtTime(float Time) const
{
    if (MovementPath.Num() < 2 || MovementSpeed <= 0.0f)
    {
        return Location;
    }

    // Calculate position along movement path
    float TotalLength = 0.0f;
    for (int32 i = 0; i < MovementPath.Num() - 1; i++)
    {
        TotalLength += FVector::Distance(MovementPath[i], MovementPath[i + 1]);
    }

    float CycleTime = TotalLength / MovementSpeed;
    if (CycleTime <= 0.0f) return Location;

    float NormalizedTime = FMath::Fmod(Time, CycleTime * 2.0f);
    bool bReverse = NormalizedTime > CycleTime;
    if (bReverse) NormalizedTime = CycleTime * 2.0f - NormalizedTime;

    float TargetDistance = NormalizedTime * MovementSpeed;
    float AccumulatedDistance = 0.0f;

    for (int32 i = 0; i < MovementPath.Num() - 1; i++)
    {
        float SegmentLength = FVector::Distance(MovementPath[i], MovementPath[i + 1]);
        if (AccumulatedDistance + SegmentLength >= TargetDistance)
        {
            float SegmentProgress = (TargetDistance - AccumulatedDistance) / SegmentLength;
            return FMath::Lerp(MovementPath[i], MovementPath[i + 1], SegmentProgress);
        }
        AccumulatedDistance += SegmentLength;
    }

    return MovementPath.Last();
}

float FForge2HazardDefinition::GetCurrentRotation(float Time) const
{
    if (RotationSpeed == 0.0f) return 0.0f;
    return FMath::Fmod(Time * RotationSpeed, 360.0f);
}

// ============================================================================
// Collectible Definition
// ============================================================================

bool FForge2CollectibleDefinition::CanCollect(const TArray<FName>& CollectedCollectibles, int32 CurrentCoins) const
{
    // Check if already collected (non-respawning)
    if (!bRespawns && CollectedCollectibles.Contains(CollectibleID))
    {
        return false;
    }

    // Check requirement
    if (RequiredCollectible != NAME_None && !CollectedCollectibles.Contains(RequiredCollectible))
    {
        return false;
    }

    // Check coin requirement
    if (RequiredCoins > CurrentCoins)
    {
        return false;
    }

    return true;
}

float FForge2CollectibleDefinition::GetBobOffset(float Time) const
{
    if (!bBobbing) return 0.0f;
    return FMath::Sin(Time * BobSpeed) * BobHeight;
}

float FForge2CollectibleDefinition::GetRotation(float Time) const
{
    if (!bRotating) return 0.0f;
    return FMath::Fmod(Time * RotationSpeed, 360.0f);
}

// ============================================================================
// Power-Up Definition
// ============================================================================

bool FForge2PowerUpDefinition::IsExpired(float ActivationTime, float CurrentTime) const
{
    if (Duration <= 0.0f) return false; // Permanent
    return (CurrentTime - ActivationTime) >= Duration;
}

float FForge2PowerUpDefinition::GetRemainingTime(float ActivationTime, float CurrentTime) const
{
    if (Duration <= 0.0f) return FLT_MAX;
    return FMath::Max(0.0f, Duration - (CurrentTime - ActivationTime));
}

float FForge2PowerUpDefinition::GetEffectMultiplier() const
{
    switch (PowerUpType)
    {
        case EForge2PowerUpType::SpeedBoost:
            return SpeedMultiplier;
        case EForge2PowerUpType::JumpBoost:
            return JumpMultiplier;
        case EForge2PowerUpType::SizeChange:
            return SizeMultiplier;
        case EForge2PowerUpType::SlowMotion:
            return TimeScaleMultiplier;
        default:
            return 1.0f;
    }
}

// ============================================================================
// Checkpoint Definition
// ============================================================================

void FForge2CheckpointDefinition::Activate(FForge2PlayerState& PlayerState) const
{
    PlayerState.LastCheckpointID = CheckpointID;
    PlayerState.RespawnLocation = SpawnLocation;
    PlayerState.RespawnRotation = SpawnRotation;

    if (bRestoreHealth)
    {
        PlayerState.CurrentHealth = PlayerState.MaxHealth;
    }
}

// ============================================================================
// Level Definition
// ============================================================================

int32 FForge2LevelDefinition::GetTotalCollectibles() const
{
    return TotalCoins + TotalGems + TotalStars + TotalSecrets;
}

float FForge2LevelDefinition::GetCompletionPercentage(const FForge2PlayerState& PlayerState) const
{
    int32 Total = GetTotalCollectibles();
    if (Total == 0) return 100.0f;

    int32 Collected = PlayerState.CoinsCollected + PlayerState.GemsCollected +
                      PlayerState.StarsCollected + PlayerState.SecretsFound;
    return (float)Collected / (float)Total * 100.0f;
}

int32 FForge2LevelDefinition::GetStarRating(float CompletionTime, int32 CollectiblesCollected) const
{
    int32 Stars = 0;

    // Time-based stars
    if (CompletionTime <= GoldTime) Stars = 3;
    else if (CompletionTime <= SilverTime) Stars = 2;
    else if (CompletionTime <= BronzeTime) Stars = 1;

    // Bonus for collectibles
    int32 Total = GetTotalCollectibles();
    if (Total > 0)
    {
        float CollectionPercent = (float)CollectiblesCollected / (float)Total;
        if (CollectionPercent >= 1.0f) Stars = FMath::Max(Stars, 3);
        else if (CollectionPercent >= 0.75f) Stars = FMath::Max(Stars, 2);
        else if (CollectionPercent >= 0.5f) Stars = FMath::Max(Stars, 1);
    }

    return Stars;
}

// ============================================================================
// Player State
// ============================================================================

void FForge2PlayerState::Reset()
{
    CurrentHealth = MaxHealth;
    CurrentLives = MaxLives;
    CoinsCollected = 0;
    GemsCollected = 0;
    StarsCollected = 0;
    SecretsFound = 0;
    CollectedIDs.Empty();
    ActivePowerUps.Empty();

    bIsGrounded = false;
    bIsJumping = false;
    bIsFalling = false;
    bIsWallSliding = false;
    bIsDashing = false;
    bIsCrouching = false;

    CurrentJumpCount = 0;
    CurrentDashCount = 0;
    CoyoteTimeRemaining = 0.0f;
    JumpBufferTimeRemaining = 0.0f;

    CurrentVelocity = FVector::ZeroVector;

    DeathCount = 0;
    TotalPlayTime = 0.0f;
}

void FForge2PlayerState::TakeDamage(int32 Damage)
{
    // Check for invincibility power-up
    for (const auto& PowerUp : ActivePowerUps)
    {
        if (PowerUp.Value.PowerUpType == EForge2PowerUpType::Invincibility)
        {
            return; // No damage while invincible
        }
    }

    CurrentHealth = FMath::Max(0, CurrentHealth - Damage);

    if (CurrentHealth <= 0)
    {
        Die();
    }
}

void FForge2PlayerState::Die()
{
    CurrentLives--;
    DeathCount++;

    if (CurrentLives > 0)
    {
        // Respawn at checkpoint
        CurrentHealth = MaxHealth;
        // Position will be set by respawn system
    }
    // If no lives left, game over handled elsewhere
}

void FForge2PlayerState::CollectItem(const FForge2CollectibleDefinition& Collectible)
{
    switch (Collectible.CollectibleType)
    {
        case EForge2CollectibleType::Coin:
            CoinsCollected += Collectible.Value;
            break;
        case EForge2CollectibleType::Gem:
            GemsCollected++;
            break;
        case EForge2CollectibleType::Star:
            StarsCollected++;
            break;
        case EForge2CollectibleType::SecretItem:
            SecretsFound++;
            break;
        case EForge2CollectibleType::HealthPickup:
            CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Collectible.Value);
            break;
        case EForge2CollectibleType::ExtraLife:
            CurrentLives = FMath::Min(MaxLives + 5, CurrentLives + 1); // Cap at MaxLives + 5
            break;
        default:
            break;
    }

    if (!Collectible.bRespawns)
    {
        CollectedIDs.Add(Collectible.CollectibleID);
    }
}

void FForge2PlayerState::ActivatePowerUp(const FForge2PowerUpDefinition& PowerUp, float CurrentTime)
{
    FForge2ActivePowerUp Active;
    Active.PowerUpType = PowerUp.PowerUpType;
    Active.ActivationTime = CurrentTime;
    Active.Duration = PowerUp.Duration;
    Active.EffectMultiplier = PowerUp.GetEffectMultiplier();

    ActivePowerUps.Add(PowerUp.PowerUpID, Active);
}

void FForge2PlayerState::UpdatePowerUps(float CurrentTime)
{
    TArray<FName> ExpiredPowerUps;

    for (auto& Pair : ActivePowerUps)
    {
        if (Pair.Value.Duration > 0.0f)
        {
            float Elapsed = CurrentTime - Pair.Value.ActivationTime;
            if (Elapsed >= Pair.Value.Duration)
            {
                ExpiredPowerUps.Add(Pair.Key);
            }
        }
    }

    for (const FName& ID : ExpiredPowerUps)
    {
        ActivePowerUps.Remove(ID);
    }
}

bool FForge2PlayerState::HasPowerUp(EForge2PowerUpType Type) const
{
    for (const auto& Pair : ActivePowerUps)
    {
        if (Pair.Value.PowerUpType == Type)
        {
            return true;
        }
    }
    return false;
}

float FForge2PlayerState::GetPowerUpMultiplier(EForge2PowerUpType Type) const
{
    for (const auto& Pair : ActivePowerUps)
    {
        if (Pair.Value.PowerUpType == Type)
        {
            return Pair.Value.EffectMultiplier;
        }
    }
    return 1.0f;
}

void FForge2PlayerState::UpdateCoyoteTime(float DeltaTime, bool bWasGrounded)
{
    if (bWasGrounded && !bIsGrounded && !bIsJumping)
    {
        // Just left ground without jumping - start coyote time
        // CoyoteTimeRemaining should be set from JumpConfig
    }
    else if (bIsGrounded)
    {
        CoyoteTimeRemaining = 0.0f;
    }
    else
    {
        CoyoteTimeRemaining = FMath::Max(0.0f, CoyoteTimeRemaining - DeltaTime);
    }
}

void FForge2PlayerState::UpdateJumpBuffer(float DeltaTime, bool bJumpPressed)
{
    if (bJumpPressed)
    {
        // JumpBufferTimeRemaining should be set from JumpConfig
    }
    else
    {
        JumpBufferTimeRemaining = FMath::Max(0.0f, JumpBufferTimeRemaining - DeltaTime);
    }
}

bool FForge2PlayerState::CanJump(const FForge2JumpConfig& JumpConfig, const FForge2MovementConfig& MoveConfig) const
{
    // Grounded jump
    if (bIsGrounded) return true;

    // Coyote time jump
    if (CoyoteTimeRemaining > 0.0f) return true;

    // Double/triple jump
    if (MoveConfig.Abilities.Contains(EForge2MovementAbility::DoubleJump) && CurrentJumpCount < 2)
    {
        return true;
    }

    // Wall jump
    if (bIsWallSliding && MoveConfig.Abilities.Contains(EForge2MovementAbility::WallJump))
    {
        return true;
    }

    return false;
}

bool FForge2PlayerState::CanDash(const FForge2MovementConfig& MoveConfig) const
{
    if (!MoveConfig.Abilities.Contains(EForge2MovementAbility::Dash))
    {
        return false;
    }

    if (bIsDashing) return false;

    if (MoveConfig.bDashRefreshesOnGround)
    {
        return CurrentDashCount < MoveConfig.DashCount || bIsGrounded;
    }

    return CurrentDashCount < MoveConfig.DashCount;
}

// ============================================================================
// Platformer Manager
// ============================================================================

UForge2PlatformerManager::UForge2PlatformerManager()
{
    CurrentTime = 0.0f;
}

void UForge2PlatformerManager::InitializeLevel(const FForge2LevelDefinition& Level)
{
    CurrentLevel = Level;
    PlayerState.Reset();
    PlayerState.MaxHealth = 3; // Default for platformers
    PlayerState.MaxLives = 3;
    CurrentTime = 0.0f;

    // Set initial spawn
    if (Level.Checkpoints.Num() > 0)
    {
        PlayerState.RespawnLocation = Level.Checkpoints[0].SpawnLocation;
        PlayerState.RespawnRotation = Level.Checkpoints[0].SpawnRotation;
    }
}

void UForge2PlatformerManager::UpdatePlatforms(float DeltaTime)
{
    CurrentTime += DeltaTime;

    for (FForge2PlatformDefinition& Platform : CurrentLevel.Platforms)
    {
        // Update platform states
        switch (Platform.PlatformType)
        {
            case EForge2PlatformType::Falling:
                if (Platform.bTriggered)
                {
                    Platform.FallTimer += DeltaTime;
                    if (Platform.FallTimer >= Platform.FallDelay)
                    {
                        // Platform is falling - handled by physics
                    }
                }
                break;

            case EForge2PlatformType::Crumbling:
                if (Platform.bTriggered)
                {
                    Platform.CurrentHealth -= DeltaTime * Platform.CrumbleRate;
                    if (Platform.CurrentHealth <= 0.0f && Platform.bRespawns)
                    {
                        // Start respawn timer
                        Platform.RespawnTimer += DeltaTime;
                        if (Platform.RespawnTimer >= Platform.RespawnTime)
                        {
                            Platform.CurrentHealth = 1.0f;
                            Platform.bTriggered = false;
                            Platform.RespawnTimer = 0.0f;
                        }
                    }
                }
                break;

            default:
                break;
        }
    }
}

void UForge2PlatformerManager::UpdateHazards(float DeltaTime)
{
    // Hazard updates handled by GetHazardState queries
}

void UForge2PlatformerManager::UpdateCollectibles(float DeltaTime)
{
    // Collectible respawn timers
    for (FForge2CollectibleDefinition& Collectible : CurrentLevel.Collectibles)
    {
        if (Collectible.bRespawns && Collectible.bCollected)
        {
            Collectible.RespawnTimer += DeltaTime;
            if (Collectible.RespawnTimer >= Collectible.RespawnTime)
            {
                Collectible.bCollected = false;
                Collectible.RespawnTimer = 0.0f;
            }
        }
    }
}

void UForge2PlatformerManager::UpdatePowerUps(float DeltaTime)
{
    PlayerState.UpdatePowerUps(CurrentTime);
}

void UForge2PlatformerManager::CheckCollisions(const FVector& PlayerPosition, const FVector& PlayerExtent)
{
    FBox PlayerBox(PlayerPosition - PlayerExtent, PlayerPosition + PlayerExtent);

    // Check hazard collisions
    for (const FForge2HazardDefinition& Hazard : CurrentLevel.Hazards)
    {
        if (!Hazard.IsActiveAtTime(CurrentTime)) continue;

        FVector HazardPos = Hazard.GetPositionAtTime(CurrentTime);
        FBox HazardBox(HazardPos - Hazard.Size * 0.5f, HazardPos + Hazard.Size * 0.5f);

        if (PlayerBox.Intersect(HazardBox))
        {
            if (Hazard.bInstantKill)
            {
                PlayerState.CurrentHealth = 0;
                PlayerState.Die();
            }
            else
            {
                PlayerState.TakeDamage(Hazard.Damage);
            }
        }
    }

    // Check collectible collisions
    for (FForge2CollectibleDefinition& Collectible : CurrentLevel.Collectibles)
    {
        if (Collectible.bCollected) continue;
        if (!Collectible.CanCollect(PlayerState.CollectedIDs, PlayerState.CoinsCollected)) continue;

        FVector CollectiblePos = Collectible.Location;
        CollectiblePos.Z += Collectible.GetBobOffset(CurrentTime);

        float Distance = FVector::Distance(PlayerPosition, CollectiblePos);
        if (Distance <= Collectible.CollectionRadius)
        {
            PlayerState.CollectItem(Collectible);
            Collectible.bCollected = true;
            Collectible.RespawnTimer = 0.0f;
        }
    }

    // Check power-up collisions
    for (FForge2PowerUpDefinition& PowerUp : CurrentLevel.PowerUps)
    {
        if (PowerUp.bCollected) continue;

        float Distance = FVector::Distance(PlayerPosition, PowerUp.Location);
        if (Distance <= PowerUp.CollectionRadius)
        {
            PlayerState.ActivatePowerUp(PowerUp, CurrentTime);
            PowerUp.bCollected = true;

            if (PowerUp.bRespawns)
            {
                PowerUp.RespawnTimer = 0.0f;
            }
        }
    }

    // Check checkpoint collisions
    for (FForge2CheckpointDefinition& Checkpoint : CurrentLevel.Checkpoints)
    {
        if (Checkpoint.bActivated) continue;

        float Distance = FVector::Distance(PlayerPosition, Checkpoint.Location);
        if (Distance <= Checkpoint.ActivationRadius)
        {
            Checkpoint.bActivated = true;
            Checkpoint.Activate(PlayerState);
        }
    }
}

FVector UForge2PlatformerManager::GetPlatformPosition(int32 PlatformIndex) const
{
    if (!CurrentLevel.Platforms.IsValidIndex(PlatformIndex))
    {
        return FVector::ZeroVector;
    }

    return CurrentLevel.Platforms[PlatformIndex].GetPositionAtTime(CurrentTime);
}

bool UForge2PlatformerManager::IsPlatformSafe(int32 PlatformIndex) const
{
    if (!CurrentLevel.Platforms.IsValidIndex(PlatformIndex))
    {
        return false;
    }

    return CurrentLevel.Platforms[PlatformIndex].IsSafeToStandOn(CurrentTime);
}

void UForge2PlatformerManager::TriggerPlatform(int32 PlatformIndex)
{
    if (!CurrentLevel.Platforms.IsValidIndex(PlatformIndex))
    {
        return;
    }

    FForge2PlatformDefinition& Platform = CurrentLevel.Platforms[PlatformIndex];
    Platform.bTriggered = true;
    Platform.FallTimer = 0.0f;
}

void UForge2PlatformerManager::RespawnPlayer()
{
    PlayerState.CurrentHealth = PlayerState.MaxHealth;
    PlayerState.CurrentJumpCount = 0;
    PlayerState.CurrentDashCount = 0;
    PlayerState.bIsGrounded = true;
    PlayerState.bIsJumping = false;
    PlayerState.bIsFalling = false;
    PlayerState.bIsWallSliding = false;
    PlayerState.bIsDashing = false;
    PlayerState.CurrentVelocity = FVector::ZeroVector;

    // Clear temporary power-ups on death (optional)
    // PlayerState.ActivePowerUps.Empty();
}

bool UForge2PlatformerManager::IsLevelComplete() const
{
    // Level is complete when reaching the exit/goal
    // This should be checked by game logic
    return false;
}

float UForge2PlatformerManager::GetLevelCompletionPercentage() const
{
    return CurrentLevel.GetCompletionPercentage(PlayerState);
}

int32 UForge2PlatformerManager::GetStarRating() const
{
    return CurrentLevel.GetStarRating(CurrentTime,
        PlayerState.CoinsCollected + PlayerState.GemsCollected +
        PlayerState.StarsCollected + PlayerState.SecretsFound);
}

// ============================================================================
// Utility Functions
// ============================================================================

FForge2JumpConfig UForge2PlatformerUtilities::CreateStandardJump()
{
    FForge2JumpConfig Config;
    Config.JumpType = EForge2JumpType::Normal;
    Config.JumpHeight = 300.0f;
    Config.TimeToApex = 0.35f;
    Config.FallGravityMultiplier = 2.0f;
    Config.bVariableHeight = true;
    Config.MinJumpHeightPercent = 0.4f;
    Config.CoyoteTime = 0.1f;
    Config.JumpBufferTime = 0.1f;
    return Config;
}

FForge2JumpConfig UForge2PlatformerUtilities::CreateFloatyJump()
{
    FForge2JumpConfig Config;
    Config.JumpType = EForge2JumpType::Normal;
    Config.JumpHeight = 350.0f;
    Config.TimeToApex = 0.5f;
    Config.FallGravityMultiplier = 1.5f;
    Config.bVariableHeight = true;
    Config.MinJumpHeightPercent = 0.3f;
    Config.CoyoteTime = 0.15f;
    Config.JumpBufferTime = 0.15f;
    return Config;
}

FForge2JumpConfig UForge2PlatformerUtilities::CreateSnappyJump()
{
    FForge2JumpConfig Config;
    Config.JumpType = EForge2JumpType::Normal;
    Config.JumpHeight = 250.0f;
    Config.TimeToApex = 0.25f;
    Config.FallGravityMultiplier = 2.5f;
    Config.bVariableHeight = true;
    Config.MinJumpHeightPercent = 0.5f;
    Config.CoyoteTime = 0.08f;
    Config.JumpBufferTime = 0.12f;
    return Config;
}

FForge2JumpConfig UForge2PlatformerUtilities::CreateDoubleJump()
{
    FForge2JumpConfig Config = CreateStandardJump();
    Config.JumpType = EForge2JumpType::Double;
    Config.JumpHeight = 280.0f; // Slightly lower second jump
    return Config;
}

FForge2JumpConfig UForge2PlatformerUtilities::CreateWallJump()
{
    FForge2JumpConfig Config;
    Config.JumpType = EForge2JumpType::Wall;
    Config.JumpHeight = 280.0f;
    Config.TimeToApex = 0.35f;
    Config.FallGravityMultiplier = 2.0f;
    Config.WallJumpHorizontalForce = 400.0f;
    Config.WallJumpAngle = 45.0f;
    return Config;
}

FForge2MovementConfig UForge2PlatformerUtilities::CreateStandardMovement()
{
    FForge2MovementConfig Config;
    Config.MaxSpeed = 600.0f;
    Config.Acceleration = 2000.0f;
    Config.Deceleration = 2500.0f;
    Config.AirAcceleration = 1500.0f;
    Config.AirDeceleration = 1000.0f;
    Config.SprintSpeedMultiplier = 1.5f;
    Config.CrouchSpeedMultiplier = 0.5f;
    return Config;
}

FForge2MovementConfig UForge2PlatformerUtilities::CreatePrecisionMovement()
{
    FForge2MovementConfig Config;
    Config.MaxSpeed = 500.0f;
    Config.Acceleration = 3000.0f;
    Config.Deceleration = 4000.0f;
    Config.AirAcceleration = 2500.0f;
    Config.AirDeceleration = 2000.0f;
    Config.TurnSpeed = 15.0f;

    // Precision platformer abilities
    Config.Abilities.Add(EForge2MovementAbility::Dash);
    Config.Abilities.Add(EForge2MovementAbility::WallSlide);
    Config.Abilities.Add(EForge2MovementAbility::WallJump);
    Config.Abilities.Add(EForge2MovementAbility::WallClimb);

    Config.DashSpeed = 1200.0f;
    Config.DashDuration = 0.15f;
    Config.DashCooldown = 0.0f; // No cooldown, count-based
    Config.DashCount = 1;
    Config.bDashRefreshesOnGround = true;

    Config.WallSlideSpeed = 100.0f;
    Config.WallClimbSpeed = 200.0f;
    Config.WallStaminaDrain = 25.0f;

    return Config;
}

FForge2MovementConfig UForge2PlatformerUtilities::CreateMomentumMovement()
{
    FForge2MovementConfig Config;
    Config.MaxSpeed = 800.0f;
    Config.Acceleration = 1200.0f;
    Config.Deceleration = 800.0f;
    Config.AirAcceleration = 600.0f;
    Config.AirDeceleration = 400.0f;
    Config.TurnSpeed = 6.0f;

    Config.Abilities.Add(EForge2MovementAbility::Slide);
    Config.Abilities.Add(EForge2MovementAbility::Roll);

    Config.SlideSpeed = 900.0f;
    Config.SlideDuration = 0.5f;
    Config.SlideCooldown = 0.3f;

    return Config;
}

FForge2PlatformDefinition UForge2PlatformerUtilities::CreateStaticPlatform(const FVector& Location, const FVector& Size)
{
    FForge2PlatformDefinition Platform;
    Platform.PlatformID = FName(*FString::Printf(TEXT("Platform_%d"), FMath::Rand()));
    Platform.PlatformType = EForge2PlatformType::Static;
    Platform.Location = Location;
    Platform.Size = Size;
    return Platform;
}

FForge2PlatformDefinition UForge2PlatformerUtilities::CreateMovingPlatform(const TArray<FVector>& Path, float Speed)
{
    FForge2PlatformDefinition Platform;
    Platform.PlatformID = FName(*FString::Printf(TEXT("MovingPlatform_%d"), FMath::Rand()));
    Platform.PlatformType = EForge2PlatformType::Moving;
    Platform.MovementPath = Path;
    Platform.MoveSpeed = Speed;
    Platform.Size = FVector(200.0f, 200.0f, 30.0f);

    if (Path.Num() > 0)
    {
        Platform.Location = Path[0];
    }

    return Platform;
}

FForge2PlatformDefinition UForge2PlatformerUtilities::CreateFallingPlatform(const FVector& Location, float Delay)
{
    FForge2PlatformDefinition Platform;
    Platform.PlatformID = FName(*FString::Printf(TEXT("FallingPlatform_%d"), FMath::Rand()));
    Platform.PlatformType = EForge2PlatformType::Falling;
    Platform.Location = Location;
    Platform.Size = FVector(150.0f, 150.0f, 30.0f);
    Platform.FallDelay = Delay;
    Platform.bRespawns = true;
    Platform.RespawnTime = 3.0f;
    return Platform;
}

FForge2PlatformDefinition UForge2PlatformerUtilities::CreateBouncyPlatform(const FVector& Location, float BounceForce)
{
    FForge2PlatformDefinition Platform;
    Platform.PlatformID = FName(*FString::Printf(TEXT("BouncyPlatform_%d"), FMath::Rand()));
    Platform.PlatformType = EForge2PlatformType::Bouncy;
    Platform.Location = Location;
    Platform.Size = FVector(150.0f, 150.0f, 30.0f);
    Platform.BounceForce = BounceForce;
    return Platform;
}

FForge2HazardDefinition UForge2PlatformerUtilities::CreateSpikeHazard(const FVector& Location, const FVector& Size)
{
    FForge2HazardDefinition Hazard;
    Hazard.HazardID = FName(*FString::Printf(TEXT("Spike_%d"), FMath::Rand()));
    Hazard.HazardType = EForge2HazardType::Spike;
    Hazard.Location = Location;
    Hazard.Size = Size;
    Hazard.Damage = 1;
    Hazard.bInstantKill = false;
    return Hazard;
}

FForge2HazardDefinition UForge2PlatformerUtilities::CreateSawBladeHazard(const TArray<FVector>& Path, float Speed, float BladeRadius)
{
    FForge2HazardDefinition Hazard;
    Hazard.HazardID = FName(*FString::Printf(TEXT("SawBlade_%d"), FMath::Rand()));
    Hazard.HazardType = EForge2HazardType::SawBlade;
    Hazard.MovementPath = Path;
    Hazard.MovementSpeed = Speed;
    Hazard.Size = FVector(BladeRadius * 2.0f);
    Hazard.RotationSpeed = 720.0f;
    Hazard.Damage = 1;

    if (Path.Num() > 0)
    {
        Hazard.Location = Path[0];
    }

    return Hazard;
}

FForge2HazardDefinition UForge2PlatformerUtilities::CreateLaserHazard(const FVector& Start, const FVector& End, float OnTime, float OffTime)
{
    FForge2HazardDefinition Hazard;
    Hazard.HazardID = FName(*FString::Printf(TEXT("Laser_%d"), FMath::Rand()));
    Hazard.HazardType = EForge2HazardType::Laser;
    Hazard.Location = Start;
    Hazard.LaserEndPoint = End;
    Hazard.LaserWidth = 20.0f;
    Hazard.bTimed = true;
    Hazard.ActiveDuration = OnTime;
    Hazard.InactiveDuration = OffTime;
    Hazard.Damage = 1;

    // Calculate size from start to end
    FVector Direction = End - Start;
    Hazard.Size = FVector(Direction.Size(), Hazard.LaserWidth, Hazard.LaserWidth);

    return Hazard;
}

FForge2CollectibleDefinition UForge2PlatformerUtilities::CreateCoin(const FVector& Location, int32 Value)
{
    FForge2CollectibleDefinition Collectible;
    Collectible.CollectibleID = FName(*FString::Printf(TEXT("Coin_%d"), FMath::Rand()));
    Collectible.CollectibleType = EForge2CollectibleType::Coin;
    Collectible.Location = Location;
    Collectible.Value = Value;
    Collectible.CollectionRadius = 50.0f;
    Collectible.bBobbing = true;
    Collectible.BobHeight = 10.0f;
    Collectible.BobSpeed = 3.0f;
    Collectible.bRotating = true;
    Collectible.RotationSpeed = 180.0f;
    return Collectible;
}

FForge2CollectibleDefinition UForge2PlatformerUtilities::CreateGem(const FVector& Location)
{
    FForge2CollectibleDefinition Collectible;
    Collectible.CollectibleID = FName(*FString::Printf(TEXT("Gem_%d"), FMath::Rand()));
    Collectible.CollectibleType = EForge2CollectibleType::Gem;
    Collectible.Location = Location;
    Collectible.Value = 1;
    Collectible.CollectionRadius = 60.0f;
    Collectible.bBobbing = true;
    Collectible.BobHeight = 15.0f;
    Collectible.BobSpeed = 2.0f;
    Collectible.bRotating = true;
    Collectible.RotationSpeed = 90.0f;
    Collectible.bRespawns = false;
    return Collectible;
}

FForge2CollectibleDefinition UForge2PlatformerUtilities::CreateStar(const FVector& Location)
{
    FForge2CollectibleDefinition Collectible;
    Collectible.CollectibleID = FName(*FString::Printf(TEXT("Star_%d"), FMath::Rand()));
    Collectible.CollectibleType = EForge2CollectibleType::Star;
    Collectible.Location = Location;
    Collectible.Value = 1;
    Collectible.CollectionRadius = 75.0f;
    Collectible.bBobbing = true;
    Collectible.BobHeight = 20.0f;
    Collectible.BobSpeed = 1.5f;
    Collectible.bRotating = true;
    Collectible.RotationSpeed = 60.0f;
    Collectible.bRespawns = false;
    return Collectible;
}

FForge2PowerUpDefinition UForge2PlatformerUtilities::CreateInvincibilityPowerUp(const FVector& Location, float Duration)
{
    FForge2PowerUpDefinition PowerUp;
    PowerUp.PowerUpID = FName(*FString::Printf(TEXT("Invincibility_%d"), FMath::Rand()));
    PowerUp.PowerUpType = EForge2PowerUpType::Invincibility;
    PowerUp.Location = Location;
    PowerUp.Duration = Duration;
    PowerUp.CollectionRadius = 50.0f;
    return PowerUp;
}

FForge2PowerUpDefinition UForge2PlatformerUtilities::CreateSpeedBoostPowerUp(const FVector& Location, float Duration, float Multiplier)
{
    FForge2PowerUpDefinition PowerUp;
    PowerUp.PowerUpID = FName(*FString::Printf(TEXT("SpeedBoost_%d"), FMath::Rand()));
    PowerUp.PowerUpType = EForge2PowerUpType::SpeedBoost;
    PowerUp.Location = Location;
    PowerUp.Duration = Duration;
    PowerUp.SpeedMultiplier = Multiplier;
    PowerUp.CollectionRadius = 50.0f;
    return PowerUp;
}

FForge2PowerUpDefinition UForge2PlatformerUtilities::CreateDoubleJumpPowerUp(const FVector& Location, float Duration)
{
    FForge2PowerUpDefinition PowerUp;
    PowerUp.PowerUpID = FName(*FString::Printf(TEXT("DoubleJump_%d"), FMath::Rand()));
    PowerUp.PowerUpType = EForge2PowerUpType::DoubleJump;
    PowerUp.Location = Location;
    PowerUp.Duration = Duration;
    PowerUp.CollectionRadius = 50.0f;
    return PowerUp;
}

FForge2CheckpointDefinition UForge2PlatformerUtilities::CreateCheckpoint(const FVector& Location, const FRotator& SpawnRotation)
{
    FForge2CheckpointDefinition Checkpoint;
    Checkpoint.CheckpointID = FName(*FString::Printf(TEXT("Checkpoint_%d"), FMath::Rand()));
    Checkpoint.Location = Location;
    Checkpoint.SpawnLocation = Location + FVector(0.0f, 0.0f, 100.0f); // Spawn slightly above
    Checkpoint.SpawnRotation = SpawnRotation;
    Checkpoint.ActivationRadius = 100.0f;
    Checkpoint.bRestoreHealth = true;
    return Checkpoint;
}
