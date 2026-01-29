// Copyright 2025 FORGE2 Project. All Rights Reserved.
// Platformer Genre System - Movement, Platforms, Collectibles, Hazards

#pragma once

#include "CoreMinimal.h"
#include "Forge2PlatformerSystem.generated.h"

// ============================================================================
// PLATFORMER SYSTEM ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2PlatformerStyle : uint8
{
	Classic_2D			UMETA(DisplayName = "Classic 2D"),
	Modern_2D			UMETA(DisplayName = "Modern 2D (Celeste-style)"),
	Metroidvania		UMETA(DisplayName = "Metroidvania"),
	Platformer_3D		UMETA(DisplayName = "3D Platformer"),
	Collectathon		UMETA(DisplayName = "Collectathon"),
	Precision			UMETA(DisplayName = "Precision Platformer"),
	Puzzle_Platformer	UMETA(DisplayName = "Puzzle Platformer"),
	Auto_Runner			UMETA(DisplayName = "Auto Runner")
};

UENUM(BlueprintType)
enum class EForge2JumpType : uint8
{
	Normal			UMETA(DisplayName = "Normal Jump"),
	Double			UMETA(DisplayName = "Double Jump"),
	Wall			UMETA(DisplayName = "Wall Jump"),
	Triple			UMETA(DisplayName = "Triple Jump"),
	Long			UMETA(DisplayName = "Long Jump"),
	High			UMETA(DisplayName = "High Jump"),
	Ground_Pound	UMETA(DisplayName = "Ground Pound"),
	Spin			UMETA(DisplayName = "Spin Jump"),
	Flutter			UMETA(DisplayName = "Flutter Jump"),
	Glide			UMETA(DisplayName = "Glide")
};

UENUM(BlueprintType)
enum class EForge2MovementAbility : uint8
{
	None			UMETA(DisplayName = "None"),
	Dash			UMETA(DisplayName = "Dash"),
	Slide			UMETA(DisplayName = "Slide"),
	Wall_Slide		UMETA(DisplayName = "Wall Slide"),
	Wall_Climb		UMETA(DisplayName = "Wall Climb"),
	Ledge_Grab		UMETA(DisplayName = "Ledge Grab"),
	Roll			UMETA(DisplayName = "Roll"),
	Crouch			UMETA(DisplayName = "Crouch"),
	Dive			UMETA(DisplayName = "Dive"),
	Grapple			UMETA(DisplayName = "Grapple"),
	Teleport		UMETA(DisplayName = "Teleport"),
	Phase			UMETA(DisplayName = "Phase Through")
};

UENUM(BlueprintType)
enum class EForge2PlatformType : uint8
{
	Static			UMETA(DisplayName = "Static"),
	Moving			UMETA(DisplayName = "Moving"),
	Falling			UMETA(DisplayName = "Falling"),
	Bouncy			UMETA(DisplayName = "Bouncy"),
	Ice				UMETA(DisplayName = "Ice (Slippery)"),
	Conveyor		UMETA(DisplayName = "Conveyor"),
	One_Way			UMETA(DisplayName = "One-Way"),
	Crumbling		UMETA(DisplayName = "Crumbling"),
	Toggle			UMETA(DisplayName = "Toggle (On/Off)"),
	Rotating		UMETA(DisplayName = "Rotating"),
	Sinking			UMETA(DisplayName = "Sinking"),
	Cloud			UMETA(DisplayName = "Cloud (Drop-through)")
};

UENUM(BlueprintType)
enum class EForge2HazardType : uint8
{
	Spike			UMETA(DisplayName = "Spike"),
	Pit				UMETA(DisplayName = "Bottomless Pit"),
	Lava			UMETA(DisplayName = "Lava/Fire"),
	Water			UMETA(DisplayName = "Water (Drowning)"),
	Crusher			UMETA(DisplayName = "Crusher"),
	Saw_Blade		UMETA(DisplayName = "Saw Blade"),
	Laser			UMETA(DisplayName = "Laser"),
	Electric		UMETA(DisplayName = "Electric"),
	Enemy			UMETA(DisplayName = "Enemy Contact"),
	Projectile		UMETA(DisplayName = "Projectile"),
	Wind			UMETA(DisplayName = "Wind/Push"),
	Poison			UMETA(DisplayName = "Poison Gas")
};

UENUM(BlueprintType)
enum class EForge2CollectibleType : uint8
{
	Coin			UMETA(DisplayName = "Coin"),
	Gem				UMETA(DisplayName = "Gem"),
	Star			UMETA(DisplayName = "Star"),
	Key				UMETA(DisplayName = "Key"),
	Heart			UMETA(DisplayName = "Heart/Health"),
	Power_Up		UMETA(DisplayName = "Power-Up"),
	Extra_Life		UMETA(DisplayName = "Extra Life"),
	Secret			UMETA(DisplayName = "Secret Item"),
	Letter			UMETA(DisplayName = "Letter/Rune"),
	Ability			UMETA(DisplayName = "Ability Unlock")
};

UENUM(BlueprintType)
enum class EForge2PowerUpType : uint8
{
	Invincibility	UMETA(DisplayName = "Invincibility"),
	Speed_Boost		UMETA(DisplayName = "Speed Boost"),
	Double_Jump		UMETA(DisplayName = "Double Jump"),
	Glide			UMETA(DisplayName = "Glide"),
	Size_Big		UMETA(DisplayName = "Size Up"),
	Size_Small		UMETA(DisplayName = "Size Down"),
	Fireball		UMETA(DisplayName = "Fireball"),
	Flight			UMETA(DisplayName = "Flight"),
	Magnet			UMETA(DisplayName = "Coin Magnet"),
	Shield			UMETA(DisplayName = "Shield")
};

UENUM(BlueprintType)
enum class EForge2CharacterState : uint8
{
	Grounded		UMETA(DisplayName = "Grounded"),
	Jumping			UMETA(DisplayName = "Jumping"),
	Falling			UMETA(DisplayName = "Falling"),
	Wall_Sliding	UMETA(DisplayName = "Wall Sliding"),
	Wall_Climbing	UMETA(DisplayName = "Wall Climbing"),
	Dashing			UMETA(DisplayName = "Dashing"),
	Sliding			UMETA(DisplayName = "Sliding"),
	Swimming		UMETA(DisplayName = "Swimming"),
	Climbing		UMETA(DisplayName = "Climbing Ladder"),
	Dead			UMETA(DisplayName = "Dead"),
	Respawning		UMETA(DisplayName = "Respawning")
};

// ============================================================================
// MOVEMENT CONFIG STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2JumpConfig
{
	GENERATED_BODY()

	// Jump type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	EForge2JumpType JumpType = EForge2JumpType::Normal;

	// Jump height (units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float JumpHeight = 400.0f;

	// Time to apex (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float TimeToApex = 0.4f;

	// Fall gravity multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float FallGravityMultiplier = 2.0f;

	// Variable jump height (release early = lower jump)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	bool bVariableHeight = true;

	// Minimum jump height ratio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float MinJumpHeightRatio = 0.4f;

	// Air control (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float AirControl = 0.8f;

	// Coyote time (seconds after leaving ground when jump still allowed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assists")
	float CoyoteTime = 0.1f;

	// Jump buffering (seconds before landing when jump input is stored)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assists")
	float JumpBufferTime = 0.1f;

	// Calculate gravity from height and time
	float CalculateGravity() const;

	// Calculate initial velocity from height
	float CalculateJumpVelocity() const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MovementConfig
{
	GENERATED_BODY()

	// Platformer style
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	EForge2PlatformerStyle Style = EForge2PlatformerStyle::Modern_2D;

	// Max run speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxRunSpeed = 600.0f;

	// Ground acceleration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundAcceleration = 2000.0f;

	// Ground deceleration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundDeceleration = 2500.0f;

	// Turn speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TurnSpeed = 0.15f;

	// Slope limit (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SlopeLimit = 45.0f;

	// Jump configurations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	TArray<FForge2JumpConfig> JumpConfigs;

	// Max jumps (for multi-jump)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	int32 MaxJumps = 1;

	// Wall slide speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	float WallSlideSpeed = 100.0f;

	// Wall jump horizontal velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	float WallJumpHorizontalVelocity = 400.0f;

	// Can wall slide
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	bool bCanWallSlide = false;

	// Can wall jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	bool bCanWallJump = false;

	// Can wall climb
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	bool bCanWallClimb = false;

	// Dash distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDistance = 300.0f;

	// Dash duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDuration = 0.15f;

	// Dash cooldown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashCooldown = 0.3f;

	// Can dash
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bCanDash = false;

	// Can dash in air
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bCanAirDash = false;

	// Max air dashes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	int32 MaxAirDashes = 1;

	// Available movement abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<EForge2MovementAbility> Abilities;
};

// ============================================================================
// PLATFORM STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlatformMovement
{
	GENERATED_BODY()

	// Path points (local space)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	TArray<FVector> PathPoints;

	// Movement speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Speed = 200.0f;

	// Wait time at each point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WaitTimeAtPoints = 0.5f;

	// Loop the path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bLoop = true;

	// Ping-pong (reverse at end instead of jump to start)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bPingPong = true;

	// Start moving only when player stands on
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	bool bActivateOnTouch = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlatformDefinition
{
	GENERATED_BODY()

	// Platform ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString PlatformId;

	// Platform type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2PlatformType PlatformType = EForge2PlatformType::Static;

	// Platform size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
	FVector Size = FVector(200.0f, 50.0f, 20.0f);

	// Movement config (for moving platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FForge2PlatformMovement Movement;

	// Bounce force (for bouncy platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce")
	float BounceForce = 1000.0f;

	// Conveyor speed (for conveyor platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	float ConveyorSpeed = 200.0f;

	// Conveyor direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor")
	FVector ConveyorDirection = FVector(1.0f, 0.0f, 0.0f);

	// Ice friction multiplier (0-1, lower = more slippery)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice")
	float IceFriction = 0.1f;

	// Fall delay (for falling platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling")
	float FallDelay = 0.5f;

	// Respawn time (for falling/crumbling platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	float RespawnTime = 3.0f;

	// Toggle state (for toggle platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toggle")
	bool bInitiallyActive = true;

	// Toggle interval (0 = manual toggle only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toggle")
	float ToggleInterval = 2.0f;

	// Rotation speed (for rotating platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	float RotationSpeed = 30.0f;

	// Crumble time (for crumbling platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crumble")
	float CrumbleTime = 1.0f;

	// Sink depth (for sinking platforms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinking")
	float SinkDepth = 50.0f;

	// Sink speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinking")
	float SinkSpeed = 100.0f;

	// Visual mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> PlatformMesh;

	// Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UMaterialInterface> PlatformMaterial;
};

// ============================================================================
// HAZARD STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HazardDefinition
{
	GENERATED_BODY()

	// Hazard ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString HazardId;

	// Hazard type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2HazardType HazardType = EForge2HazardType::Spike;

	// Damage dealt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	int32 Damage = 1;

	// Instant kill
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bInstantKill = false;

	// Knockback direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback")
	FVector KnockbackDirection = FVector::ZeroVector;

	// Knockback force
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback")
	float KnockbackForce = 500.0f;

	// Damage interval (for continuous damage hazards)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Continuous")
	float DamageInterval = 0.5f;

	// Active duration (0 = always active)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float ActiveDuration = 0.0f;

	// Inactive duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float InactiveDuration = 0.0f;

	// Start active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	bool bStartActive = true;

	// Movement path (for moving hazards)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TArray<FVector> MovementPath;

	// Movement speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MovementSpeed = 200.0f;

	// Wind/push direction (for wind hazards)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

	// Wind force
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	float WindForce = 400.0f;
};

// ============================================================================
// COLLECTIBLE STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CollectibleDefinition
{
	GENERATED_BODY()

	// Collectible ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CollectibleId;

	// Collectible type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2CollectibleType CollectibleType = EForge2CollectibleType::Coin;

	// Value (points, score, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	int32 Value = 1;

	// Respawns after collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	bool bRespawns = false;

	// Respawn time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	float RespawnTime = 5.0f;

	// Requires ability to collect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	EForge2MovementAbility RequiredAbility = EForge2MovementAbility::None;

	// Collection radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	float CollectionRadius = 30.0f;

	// Magnetic attraction radius (0 = no attraction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	float AttractionRadius = 0.0f;

	// Visual mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> CollectibleMesh;

	// Collect sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> CollectSound;

	// Collect effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> CollectEffect;
};

// ============================================================================
// POWER-UP STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PowerUpDefinition
{
	GENERATED_BODY()

	// Power-up ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString PowerUpId;

	// Power-up type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2PowerUpType PowerUpType = EForge2PowerUpType::Invincibility;

	// Duration (0 = permanent until hit/lost)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duration")
	float Duration = 10.0f;

	// Effect magnitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float Magnitude = 1.5f;

	// Grants ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	EForge2MovementAbility GrantedAbility = EForge2MovementAbility::None;

	// Lost on damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loss")
	bool bLostOnDamage = true;

	// Number of hits to lose (if lost on damage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loss")
	int32 HitsToLose = 1;

	// Visual mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> PowerUpMesh;

	// Active effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> ActiveEffect;
};

// ============================================================================
// CHECKPOINT STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CheckpointDefinition
{
	GENERATED_BODY()

	// Checkpoint ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CheckpointId;

	// Checkpoint index (order)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	int32 CheckpointIndex = 0;

	// Spawn location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector SpawnLocation = FVector::ZeroVector;

	// Spawn rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FRotator SpawnRotation = FRotator::ZeroRotator;

	// Activation radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
	float ActivationRadius = 100.0f;

	// Requires manual activation (touch vs button press)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
	bool bManualActivation = false;

	// Restore health on activation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	bool bRestoreHealth = true;

	// Restore power-ups on activation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	bool bRestorePowerUps = false;

	// Only activatable once per life
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	bool bSingleUse = false;

	// Inactive mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> InactiveMesh;

	// Active mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> ActiveMesh;
};

// ============================================================================
// LEVEL STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlatformerLevelDefinition
{
	GENERATED_BODY()

	// Level ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString LevelId;

	// Level name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText LevelName;

	// World/Zone number
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	int32 WorldNumber = 1;

	// Level number within world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	int32 LevelNumber = 1;

	// Time limit (0 = no limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float TimeLimit = 0.0f;

	// Par time (for time attack)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float ParTime = 60.0f;

	// Target collectible count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	int32 TotalCoins = 100;

	// Total secret collectibles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	int32 TotalSecrets = 3;

	// Start position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector StartPosition = FVector::ZeroVector;

	// Goal position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal")
	FVector GoalPosition = FVector::ZeroVector;

	// Checkpoints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
	TArray<FForge2CheckpointDefinition> Checkpoints;

	// Background music
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> BackgroundMusic;

	// Level asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSoftObjectPtr<UWorld> LevelAsset;
};

// ============================================================================
// PLAYER STATE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlatformerPlayerState
{
	GENERATED_BODY()

	// Current health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	int32 CurrentHealth = 3;

	// Max health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	int32 MaxHealth = 3;

	// Lives remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lives")
	int32 Lives = 3;

	// Current score
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Score = 0;

	// Coins collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	int32 CoinsCollected = 0;

	// Secrets found
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	int32 SecretsFound = 0;

	// Current state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EForge2CharacterState CurrentState = EForge2CharacterState::Grounded;

	// Jumps remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	int32 JumpsRemaining = 1;

	// Air dashes remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	int32 AirDashesRemaining = 0;

	// Coyote time remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CoyoteTimeRemaining = 0.0f;

	// Jump buffer time remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpBufferRemaining = 0.0f;

	// Dash cooldown remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashCooldownRemaining = 0.0f;

	// Current power-up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
	FString CurrentPowerUpId;

	// Power-up time remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
	float PowerUpTimeRemaining = 0.0f;

	// Power-up hits remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
	int32 PowerUpHitsRemaining = 0;

	// Last checkpoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FString LastCheckpointId;

	// Invincibility frames remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float InvincibilityRemaining = 0.0f;

	// Total time played
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float TotalTime = 0.0f;

	// Level time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float LevelTime = 0.0f;

	// Unlocked abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<EForge2MovementAbility> UnlockedAbilities;

	// Check if has ability
	bool HasAbility(EForge2MovementAbility Ability) const;

	// Reset for new level
	void ResetForLevel();

	// Reset for respawn
	void ResetForRespawn();
};

// ============================================================================
// PLATFORMER MANAGER
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2PlatformerManager
{
public:
	// Platform management
	bool RegisterPlatform(const FForge2PlatformDefinition& Platform);
	FForge2PlatformDefinition* GetPlatform(const FString& PlatformId);
	TArray<FString> GetAllPlatformIds() const;

	// Hazard management
	bool RegisterHazard(const FForge2HazardDefinition& Hazard);
	FForge2HazardDefinition* GetHazard(const FString& HazardId);

	// Collectible management
	bool RegisterCollectible(const FForge2CollectibleDefinition& Collectible);
	FForge2CollectibleDefinition* GetCollectible(const FString& CollectibleId);

	// Power-up management
	bool RegisterPowerUp(const FForge2PowerUpDefinition& PowerUp);
	FForge2PowerUpDefinition* GetPowerUp(const FString& PowerUpId);

	// Level management
	bool RegisterLevel(const FForge2PlatformerLevelDefinition& Level);
	FForge2PlatformerLevelDefinition* GetLevel(const FString& LevelId);
	TArray<FString> GetAllLevelIds() const;

	// Gameplay
	bool StartLevel(const FString& LevelId);
	void EndLevel(bool bCompleted);
	void RespawnPlayer();
	void CollectItem(const FString& CollectibleId);
	void ApplyPowerUp(const FString& PowerUpId);
	void TakeDamage(int32 Damage);
	void ActivateCheckpoint(const FString& CheckpointId);

	// Movement config
	void SetMovementConfig(const FForge2MovementConfig& Config);
	FForge2MovementConfig GetMovementConfig() const { return MovementConfig; }

	// Player state
	FForge2PlatformerPlayerState GetPlayerState() const { return PlayerState; }
	void SetPlayerState(const FForge2PlatformerPlayerState& State) { PlayerState = State; }

	// Update
	void Update(float DeltaTime);

	// Reset
	void Reset();

private:
	TMap<FString, FForge2PlatformDefinition> Platforms;
	TMap<FString, FForge2HazardDefinition> Hazards;
	TMap<FString, FForge2CollectibleDefinition> Collectibles;
	TMap<FString, FForge2PowerUpDefinition> PowerUps;
	TMap<FString, FForge2PlatformerLevelDefinition> Levels;

	FForge2MovementConfig MovementConfig;
	FForge2PlatformerPlayerState PlayerState;

	FString ActiveLevelId;
	bool bLevelActive = false;
};

// ============================================================================
// PLATFORMER UTILITIES
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2PlatformerUtils
{
public:
	// Create movement presets
	static FForge2MovementConfig CreateClassicMovement();
	static FForge2MovementConfig CreateModernMovement();
	static FForge2MovementConfig CreatePrecisionMovement();
	static FForge2MovementConfig Create3DMovement();

	// Create common platforms
	static FForge2PlatformDefinition CreateMovingPlatform(const FString& Id, const TArray<FVector>& Path, float Speed);
	static FForge2PlatformDefinition CreateFallingPlatform(const FString& Id, float FallDelay, float RespawnTime);
	static FForge2PlatformDefinition CreateBouncyPlatform(const FString& Id, float BounceForce);
	static FForge2PlatformDefinition CreateConveyorPlatform(const FString& Id, float Speed, const FVector& Direction);

	// Create common hazards
	static FForge2HazardDefinition CreateSpike(const FString& Id);
	static FForge2HazardDefinition CreateMovingSawBlade(const FString& Id, const TArray<FVector>& Path, float Speed);
	static FForge2HazardDefinition CreateLaser(const FString& Id, float ActiveTime, float InactiveTime);

	// Create common collectibles
	static FForge2CollectibleDefinition CreateCoin(const FString& Id, int32 Value = 1);
	static FForge2CollectibleDefinition CreateGem(const FString& Id, int32 Value = 10);
	static FForge2CollectibleDefinition CreateSecretItem(const FString& Id);

	// Physics calculations
	static float CalculateJumpGravity(float JumpHeight, float TimeToApex);
	static float CalculateJumpVelocity(float JumpHeight, float Gravity);
	static float CalculateTimeInAir(float JumpVelocity, float Gravity);
};
