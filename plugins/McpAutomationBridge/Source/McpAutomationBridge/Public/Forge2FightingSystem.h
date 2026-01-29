// Copyright 2025 FORGE2 Project. All Rights Reserved.
// Fighting Genre System - Fighters, Movesets, Frame Data, Combos

#pragma once

#include "CoreMinimal.h"
#include "Forge2FightingSystem.generated.h"

// ============================================================================
// FIGHTING SYSTEM ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2FighterArchetype : uint8
{
	Balanced		UMETA(DisplayName = "Balanced"),
	Rushdown		UMETA(DisplayName = "Rushdown"),
	Zoner			UMETA(DisplayName = "Zoner"),
	Grappler		UMETA(DisplayName = "Grappler"),
	Mixup			UMETA(DisplayName = "Mixup"),
	Setplay			UMETA(DisplayName = "Setplay"),
	Footsies		UMETA(DisplayName = "Footsies"),
	Puppet			UMETA(DisplayName = "Puppet"),
	Glass_Cannon	UMETA(DisplayName = "Glass Cannon"),
	Tank			UMETA(DisplayName = "Tank")
};

UENUM(BlueprintType)
enum class EForge2MoveType : uint8
{
	// Normals
	Standing_Light		UMETA(DisplayName = "Standing Light"),
	Standing_Medium		UMETA(DisplayName = "Standing Medium"),
	Standing_Heavy		UMETA(DisplayName = "Standing Heavy"),
	Crouching_Light		UMETA(DisplayName = "Crouching Light"),
	Crouching_Medium	UMETA(DisplayName = "Crouching Medium"),
	Crouching_Heavy		UMETA(DisplayName = "Crouching Heavy"),
	Jumping_Light		UMETA(DisplayName = "Jumping Light"),
	Jumping_Medium		UMETA(DisplayName = "Jumping Medium"),
	Jumping_Heavy		UMETA(DisplayName = "Jumping Heavy"),

	// Command Normals
	Command_Normal		UMETA(DisplayName = "Command Normal"),

	// Specials
	Special				UMETA(DisplayName = "Special Move"),
	EX_Special			UMETA(DisplayName = "EX Special"),

	// Supers
	Super				UMETA(DisplayName = "Super"),
	Ultra				UMETA(DisplayName = "Ultra"),
	Instant_Kill		UMETA(DisplayName = "Instant Kill"),

	// Universal
	Throw				UMETA(DisplayName = "Throw"),
	Air_Throw			UMETA(DisplayName = "Air Throw"),
	Dash				UMETA(DisplayName = "Dash"),
	Backdash			UMETA(DisplayName = "Backdash"),

	// System
	Burst				UMETA(DisplayName = "Burst"),
	Roman_Cancel		UMETA(DisplayName = "Roman Cancel"),
	Parry				UMETA(DisplayName = "Parry"),
	Focus_Attack		UMETA(DisplayName = "Focus Attack")
};

UENUM(BlueprintType)
enum class EForge2AttackProperty : uint8
{
	None				UMETA(DisplayName = "None"),
	Overhead			UMETA(DisplayName = "Overhead"),
	Low					UMETA(DisplayName = "Low"),
	Mid					UMETA(DisplayName = "Mid"),
	Unblockable			UMETA(DisplayName = "Unblockable"),
	Projectile			UMETA(DisplayName = "Projectile"),
	AntiAir				UMETA(DisplayName = "Anti-Air"),
	Armor				UMETA(DisplayName = "Armor"),
	Invincible			UMETA(DisplayName = "Invincible"),
	Grab				UMETA(DisplayName = "Grab"),
	Counter				UMETA(DisplayName = "Counter")
};

UENUM(BlueprintType)
enum class EForge2HitEffect : uint8
{
	None				UMETA(DisplayName = "None"),
	Knockdown			UMETA(DisplayName = "Knockdown"),
	Hard_Knockdown		UMETA(DisplayName = "Hard Knockdown"),
	Launch				UMETA(DisplayName = "Launch"),
	Wall_Bounce			UMETA(DisplayName = "Wall Bounce"),
	Wall_Splat			UMETA(DisplayName = "Wall Splat"),
	Ground_Bounce		UMETA(DisplayName = "Ground Bounce"),
	Crumple				UMETA(DisplayName = "Crumple"),
	Stagger				UMETA(DisplayName = "Stagger"),
	Spin				UMETA(DisplayName = "Spin"),
	Float				UMETA(DisplayName = "Float")
};

UENUM(BlueprintType)
enum class EForge2BlockType : uint8
{
	Standing			UMETA(DisplayName = "Standing"),
	Crouching			UMETA(DisplayName = "Crouching"),
	Air					UMETA(DisplayName = "Air"),
	Auto				UMETA(DisplayName = "Auto-Block"),
	Just_Defend			UMETA(DisplayName = "Just Defend"),
	Parry				UMETA(DisplayName = "Parry"),
	Faultless			UMETA(DisplayName = "Faultless Defense")
};

UENUM(BlueprintType)
enum class EForge2MotionInput : uint8
{
	None				UMETA(DisplayName = "None"),
	QCF					UMETA(DisplayName = "Quarter Circle Forward (236)"),
	QCB					UMETA(DisplayName = "Quarter Circle Back (214)"),
	HCF					UMETA(DisplayName = "Half Circle Forward (41236)"),
	HCB					UMETA(DisplayName = "Half Circle Back (63214)"),
	DP					UMETA(DisplayName = "Dragon Punch (623)"),
	RDP					UMETA(DisplayName = "Reverse Dragon Punch (421)"),
	Charge_Back_Forward	UMETA(DisplayName = "Charge Back-Forward ([4]6)"),
	Charge_Down_Up		UMETA(DisplayName = "Charge Down-Up ([2]8)"),
	Full_Circle			UMETA(DisplayName = "Full Circle (632147896)"),
	Double_QCF			UMETA(DisplayName = "Double QCF (236236)"),
	Double_QCB			UMETA(DisplayName = "Double QCB (214214)"),
	SPD					UMETA(DisplayName = "SPD (6321478)"),
	Pretzel				UMETA(DisplayName = "Pretzel (1632143)"),
	Direction_Button	UMETA(DisplayName = "Direction + Button")
};

UENUM(BlueprintType)
enum class EForge2InputButton : uint8
{
	Light_Punch			UMETA(DisplayName = "Light Punch"),
	Medium_Punch		UMETA(DisplayName = "Medium Punch"),
	Heavy_Punch			UMETA(DisplayName = "Heavy Punch"),
	Light_Kick			UMETA(DisplayName = "Light Kick"),
	Medium_Kick			UMETA(DisplayName = "Medium Kick"),
	Heavy_Kick			UMETA(DisplayName = "Heavy Kick"),
	Punch				UMETA(DisplayName = "Punch (Any)"),
	Kick				UMETA(DisplayName = "Kick (Any)"),
	Slash				UMETA(DisplayName = "Slash"),
	Heavy_Slash			UMETA(DisplayName = "Heavy Slash"),
	Dust				UMETA(DisplayName = "Dust"),
	Special_Button		UMETA(DisplayName = "Special Button"),
	Taunt				UMETA(DisplayName = "Taunt"),
	Throw_Button		UMETA(DisplayName = "Throw"),
	Any					UMETA(DisplayName = "Any")
};

UENUM(BlueprintType)
enum class EForge2MeterType : uint8
{
	Super				UMETA(DisplayName = "Super Meter"),
	Rage				UMETA(DisplayName = "Rage"),
	Tension				UMETA(DisplayName = "Tension"),
	Drive				UMETA(DisplayName = "Drive Gauge"),
	V_Trigger			UMETA(DisplayName = "V-Trigger"),
	Burst				UMETA(DisplayName = "Burst"),
	Guard				UMETA(DisplayName = "Guard Gauge"),
	Health				UMETA(DisplayName = "Health"),
	Stamina				UMETA(DisplayName = "Stamina"),
	Unique				UMETA(DisplayName = "Character Unique")
};

UENUM(BlueprintType)
enum class EForge2CancelType : uint8
{
	None				UMETA(DisplayName = "Not Cancelable"),
	Chain				UMETA(DisplayName = "Chain Cancel"),
	Special				UMETA(DisplayName = "Special Cancel"),
	Super				UMETA(DisplayName = "Super Cancel"),
	Jump				UMETA(DisplayName = "Jump Cancel"),
	Dash				UMETA(DisplayName = "Dash Cancel"),
	Roman				UMETA(DisplayName = "Roman Cancel"),
	Focus				UMETA(DisplayName = "Focus Cancel"),
	Any					UMETA(DisplayName = "Any Cancel")
};

UENUM(BlueprintType)
enum class EForge2FighterState : uint8
{
	Standing			UMETA(DisplayName = "Standing"),
	Crouching			UMETA(DisplayName = "Crouching"),
	Jumping				UMETA(DisplayName = "Jumping"),
	Air_Rising			UMETA(DisplayName = "Air Rising"),
	Air_Falling			UMETA(DisplayName = "Air Falling"),
	Attacking			UMETA(DisplayName = "Attacking"),
	Blocking			UMETA(DisplayName = "Blocking"),
	Hitstun				UMETA(DisplayName = "Hitstun"),
	Blockstun			UMETA(DisplayName = "Blockstun"),
	Knockdown			UMETA(DisplayName = "Knockdown"),
	Wakeup				UMETA(DisplayName = "Wakeup"),
	Dashing				UMETA(DisplayName = "Dashing"),
	Special				UMETA(DisplayName = "Special Move"),
	Super				UMETA(DisplayName = "Super")
};

// ============================================================================
// FRAME DATA STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FrameData
{
	GENERATED_BODY()

	// Startup frames (before hitbox becomes active)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 StartupFrames = 5;

	// Active frames (hitbox is active)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 ActiveFrames = 3;

	// Recovery frames (after active, before actionable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 RecoveryFrames = 10;

	// Total frames
	int32 GetTotalFrames() const { return StartupFrames + ActiveFrames + RecoveryFrames; }

	// Frame advantage on block
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advantage")
	int32 BlockAdvantage = -5;

	// Frame advantage on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advantage")
	int32 HitAdvantage = 3;

	// Frame advantage on counter hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advantage")
	int32 CounterHitAdvantage = 6;

	// Invincibility start frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invincibility")
	int32 InvincibilityStartFrame = 0;

	// Invincibility end frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invincibility")
	int32 InvincibilityEndFrame = 0;

	// Armor start frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
	int32 ArmorStartFrame = 0;

	// Armor end frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
	int32 ArmorEndFrame = 0;

	// Number of armor hits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
	int32 ArmorHits = 0;

	// Is airborne during these frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bAirborne = false;

	// Can be cancelled during these frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cancel")
	TArray<EForge2CancelType> CancelOptions;

	// First frame where cancel is available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cancel")
	int32 CancelWindowStart = 0;

	// Last frame where cancel is available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cancel")
	int32 CancelWindowEnd = 0;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HitboxData
{
	GENERATED_BODY()

	// Hitbox offset from character origin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FVector Offset = FVector::ZeroVector;

	// Hitbox size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FVector Extent = FVector(50.0f, 20.0f, 50.0f);

	// Hit effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	EForge2HitEffect HitEffect = EForge2HitEffect::None;

	// Knockback direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FVector KnockbackDirection = FVector(1.0f, 0.0f, 0.0f);

	// Knockback strength
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	float KnockbackStrength = 100.0f;

	// Hitstun frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	int32 HitstunFrames = 15;

	// Blockstun frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	int32 BlockstunFrames = 10;

	// Can hit standing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	bool bHitsStanding = true;

	// Can hit crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	bool bHitsCrouching = true;

	// Can hit airborne
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	bool bHitsAirborne = true;

	// Can hit grounded
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	bool bHitsGrounded = false;
};

// ============================================================================
// MOVE DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MoveInput
{
	GENERATED_BODY()

	// Motion input required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EForge2MotionInput MotionInput = EForge2MotionInput::None;

	// Button(s) required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<EForge2InputButton> Buttons;

	// Direction modifier (for command normals)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector2D DirectionRequired = FVector2D::ZeroVector;

	// Input leniency frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	int32 InputLeniency = 8;

	// Charge time required (frames)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	int32 ChargeTime = 0;

	// Can be performed in air
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bCanDoInAir = false;

	// Convert input to display string
	FString GetInputDisplay() const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MoveDefinition
{
	GENERATED_BODY()

	// Unique move ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString MoveId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText MoveName;

	// Move type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2MoveType MoveType = EForge2MoveType::Standing_Light;

	// Attack property
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2AttackProperty AttackProperty = EForge2AttackProperty::Mid;

	// Input required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FForge2MoveInput Input;

	// Base damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	int32 BaseDamage = 30;

	// Chip damage (on block)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	int32 ChipDamage = 0;

	// Stun damage (for stun gauge)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	int32 StunDamage = 50;

	// Meter gain on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meter")
	int32 MeterGainHit = 10;

	// Meter gain on whiff
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meter")
	int32 MeterGainWhiff = 2;

	// Meter cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meter")
	int32 MeterCost = 0;

	// Frame data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	FForge2FrameData FrameData;

	// Hitboxes (can have multiple per move, for multi-hits)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitboxes")
	TArray<FForge2HitboxData> Hitboxes;

	// Number of hits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	int32 NumberOfHits = 1;

	// Animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> Animation;

	// Sound on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> HitSound;

	// Sound on whiff
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> WhiffSound;

	// Visual effect on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> HitEffect;

	// Projectile class (if projectile)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;

	// Projectile speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileSpeed = 1000.0f;

	// Is the move enabled (for unlockable moves)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	bool bEnabled = true;

	// Calculate scaled damage based on combo
	int32 CalculateComboScaledDamage(int32 HitCount, float DamageScaling) const;
};

// ============================================================================
// COMBO SYSTEM
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComboRoute
{
	GENERATED_BODY()

	// Sequence of move IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TArray<FString> MoveSequence;

	// Requires corner
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bRequiresCorner = false;

	// Requires counter hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bRequiresCounterHit = false;

	// Meter cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	int32 TotalMeterCost = 0;

	// Estimated damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	int32 EstimatedDamage = 0;

	// Difficulty rating (1-5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	int32 DifficultyRating = 1;

	// Notes for the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FText ComboNotes;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComboState
{
	GENERATED_BODY()

	// Current hit count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 HitCount = 0;

	// Total damage dealt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 TotalDamage = 0;

	// Current damage scaling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float DamageScaling = 1.0f;

	// Current hitstun decay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float HitstunDecay = 1.0f;

	// Gravity scaling (for juggles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float GravityScaling = 1.0f;

	// Juggle count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 JuggleCount = 0;

	// Moves used in combo (for proration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TArray<FString> MovesUsed;

	// Is valid combo (not dropped)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bValidCombo = true;

	// Counter hit starter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bCounterHitStarter = false;

	// Reset combo state
	void Reset();

	// Add hit to combo
	void AddHit(const FForge2MoveDefinition& Move, bool bCounterHit = false);

	// Check if move can be used (not already used, juggle limit)
	bool CanUseMove(const FString& MoveId, int32 MaxJuggles) const;
};

// ============================================================================
// METER SYSTEM
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MeterDefinition
{
	GENERATED_BODY()

	// Meter type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	EForge2MeterType MeterType = EForge2MeterType::Super;

	// Maximum value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Values")
	int32 MaxValue = 100;

	// Number of stocks (for multi-stock meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Values")
	int32 MaxStocks = 3;

	// Value per stock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Values")
	int32 ValuePerStock = 100;

	// Starting value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Values")
	int32 StartingValue = 0;

	// Gain rate multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rates")
	float GainRateMultiplier = 1.0f;

	// Passive gain per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rates")
	float PassiveGainPerSecond = 0.0f;

	// Drain rate when not in use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rates")
	float DrainRate = 0.0f;

	// Can overflow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bCanOverflow = false;

	// Carries between rounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bCarriesBetweenRounds = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MeterState
{
	GENERATED_BODY()

	// Current value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 CurrentValue = 0;

	// Current stocks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 CurrentStocks = 0;

	// Add meter
	void AddMeter(int32 Amount, const FForge2MeterDefinition& Definition);

	// Spend meter
	bool SpendMeter(int32 Amount, const FForge2MeterDefinition& Definition);

	// Check if can afford
	bool CanAfford(int32 Amount, const FForge2MeterDefinition& Definition) const;

	// Get total value (stocks * valuePerStock + current)
	int32 GetTotalValue(const FForge2MeterDefinition& Definition) const;
};

// ============================================================================
// FIGHTER DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FighterStats
{
	GENERATED_BODY()

	// Base health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 MaxHealth = 10000;

	// Stun threshold
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 StunThreshold = 1000;

	// Walk speed (forward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeedForward = 400.0f;

	// Walk speed (backward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeedBackward = 350.0f;

	// Dash distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashDistance = 300.0f;

	// Dash duration (frames)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	int32 DashDurationFrames = 20;

	// Backdash distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BackdashDistance = 250.0f;

	// Backdash invincibility frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	int32 BackdashInvincibilityFrames = 5;

	// Jump height
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpHeight = 400.0f;

	// Jump arc (horizontal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpArc = 300.0f;

	// Can double jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bCanDoubleJump = false;

	// Can air dash
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bCanAirDash = false;

	// Number of air dashes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	int32 NumAirDashes = 1;

	// Defense modifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float DefenseModifier = 1.0f;

	// Guard gauge max
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	int32 MaxGuardGauge = 100;

	// Wakeup frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	int32 WakeupFrames = 30;

	// Quick rise frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	int32 QuickRiseFrames = 20;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HurtboxData
{
	GENERATED_BODY()

	// Offset from origin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hurtbox")
	FVector Offset = FVector::ZeroVector;

	// Hurtbox extent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hurtbox")
	FVector Extent = FVector(30.0f, 15.0f, 90.0f);

	// Is for standing state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hurtbox")
	bool bStanding = true;

	// Is for crouching state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hurtbox")
	bool bCrouching = false;

	// Is for air state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hurtbox")
	bool bAirborne = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FighterDefinition
{
	GENERATED_BODY()

	// Unique fighter ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString FighterId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText FighterName;

	// Fighter archetype
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	EForge2FighterArchetype Archetype = EForge2FighterArchetype::Balanced;

	// Fighter stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FForge2FighterStats Stats;

	// Meter definitions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meters")
	TArray<FForge2MeterDefinition> Meters;

	// All moves
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moves")
	TArray<FForge2MoveDefinition> Moveset;

	// Preset combo routes (for training mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combos")
	TArray<FForge2ComboRoute> PresetCombos;

	// Hurtboxes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hurtboxes")
	TArray<FForge2HurtboxData> Hurtboxes;

	// Character mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<USkeletalMesh> CharacterMesh;

	// Portrait texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> Portrait;

	// Character select animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UAnimMontage> CharacterSelectAnimation;

	// Victory animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UAnimMontage> VictoryAnimation;

	// Defeat animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UAnimMontage> DefeatAnimation;

	// Get move by ID
	const FForge2MoveDefinition* GetMove(const FString& MoveId) const;

	// Get all moves of type
	TArray<const FForge2MoveDefinition*> GetMovesOfType(EForge2MoveType Type) const;
};

// ============================================================================
// STAGE DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StageWall
{
	GENERATED_BODY()

	// Wall location (X position)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	float WallPosition = 1000.0f;

	// Is breakable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	bool bBreakable = false;

	// Hits to break
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	int32 HitsToBreak = 3;

	// Bonus damage on wallbreak
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	int32 WallbreakBonusDamage = 500;

	// Stage transition on break
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	bool bStageTransition = false;

	// Next stage ID (for transition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	FString NextStageId;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StageHazard
{
	GENERATED_BODY()

	// Hazard location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	FVector Location = FVector::ZeroVector;

	// Hazard radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float Radius = 100.0f;

	// Damage on contact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	int32 Damage = 100;

	// Hit effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	EForge2HitEffect HitEffect = EForge2HitEffect::Knockdown;

	// Active interval (seconds, 0 = always active)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float ActiveInterval = 0.0f;

	// Warning time before activation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float WarningTime = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StageDefinition
{
	GENERATED_BODY()

	// Stage ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString StageId;

	// Stage name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText StageName;

	// Stage width (playable area)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
	float StageWidth = 2000.0f;

	// Starting distance between fighters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
	float StartingDistance = 500.0f;

	// Floor Y position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
	float FloorHeight = 0.0f;

	// Walls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walls")
	TArray<FForge2StageWall> Walls;

	// Hazards
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazards")
	TArray<FForge2StageHazard> Hazards;

	// Has ring out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bHasRingOut = false;

	// Ring out damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	int32 RingOutDamage = 9999;

	// Background music
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> BackgroundMusic;

	// Round win music
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> RoundWinMusic;

	// Stage level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSoftObjectPtr<UWorld> StageLevel;

	// Get wall at position
	const FForge2StageWall* GetWallAtPosition(float XPosition) const;
};

// ============================================================================
// MATCH STATE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FighterRuntimeState
{
	GENERATED_BODY()

	// Current health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	int32 CurrentHealth = 10000;

	// Current stun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
	int32 CurrentStun = 0;

	// Is stunned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
	bool bIsStunned = false;

	// Current state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EForge2FighterState CurrentState = EForge2FighterState::Standing;

	// Position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	FVector Position = FVector::ZeroVector;

	// Is facing right
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	bool bFacingRight = true;

	// Current move (if attacking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	FString CurrentMoveId;

	// Current frame of move
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	int32 CurrentMoveFrame = 0;

	// Remaining air actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
	int32 RemainingAirDashes = 0;

	// Has double jumped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
	bool bHasDoubleJumped = false;

	// Meter states
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meters")
	TArray<FForge2MeterState> MeterStates;

	// Combo state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FForge2ComboState ComboState;

	// Guard gauge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
	int32 GuardGauge = 100;

	// Input buffer (last N frames of inputs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<int32> InputBuffer;

	// Reset for new round
	void ResetForRound(const FForge2FighterDefinition& Definition);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MatchConfiguration
{
	GENERATED_BODY()

	// Rounds to win
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rounds")
	int32 RoundsToWin = 2;

	// Round time limit (seconds, 0 = infinite)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	int32 RoundTimeLimit = 99;

	// Damage scaling enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bDamageScaling = true;

	// Minimum damage scaling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float MinDamageScaling = 0.1f;

	// Damage scaling per hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageScalingPerHit = 0.1f;

	// Hitstun decay enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitstun")
	bool bHitstunDecay = true;

	// Minimum hitstun scaling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitstun")
	float MinHitstunScaling = 0.3f;

	// Maximum juggle count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Juggle")
	int32 MaxJuggleCount = 15;

	// Gravity scaling per juggle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Juggle")
	float GravityScalingPerJuggle = 0.05f;

	// Enable chip kills
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chip")
	bool bChipKill = false;

	// Stun recovery rate per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
	float StunRecoveryRate = 50.0f;

	// Guard break enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard")
	bool bGuardBreak = true;

	// Stage hazards enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	bool bStageHazards = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MatchState
{
	GENERATED_BODY()

	// Player 1 wins
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Player1Wins = 0;

	// Player 2 wins
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Player2Wins = 0;

	// Current round
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	int32 CurrentRound = 1;

	// Round time remaining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	float RoundTimeRemaining = 99.0f;

	// Round is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	bool bRoundActive = false;

	// Player states
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players")
	FForge2FighterRuntimeState Player1State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players")
	FForge2FighterRuntimeState Player2State;

	// Is match over
	bool IsMatchOver(int32 RoundsToWin) const;

	// Get winner (0 = none, 1 = P1, 2 = P2)
	int32 GetMatchWinner(int32 RoundsToWin) const;
};

// ============================================================================
// TRAINING MODE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TrainingModeSettings
{
	GENERATED_BODY()

	// Dummy action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
	FString DummyAction = TEXT("Stand");

	// Dummy guard mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
	FString DummyGuard = TEXT("None");

	// Dummy recovery mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
	FString DummyRecovery = TEXT("None");

	// Show hitboxes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bShowHitboxes = false;

	// Show hurtboxes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bShowHurtboxes = false;

	// Show frame data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bShowFrameData = true;

	// Show input display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bShowInputDisplay = true;

	// Show damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bShowDamage = true;

	// Show combo counter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bShowComboCounter = true;

	// Infinite health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bInfiniteHealth = true;

	// Infinite meter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bInfiniteMeter = true;

	// Reset position on drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bResetOnDrop = false;

	// Slow motion speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float SlowMotionSpeed = 0.25f;

	// Frame advance enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bFrameAdvanceMode = false;
};

// ============================================================================
// FIGHTING MANAGER
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2FightingManager
{
public:
	// Fighter management
	bool RegisterFighter(const FForge2FighterDefinition& Fighter);
	FForge2FighterDefinition* GetFighter(const FString& FighterId);
	TArray<FString> GetAllFighterIds() const;

	// Stage management
	bool RegisterStage(const FForge2StageDefinition& Stage);
	FForge2StageDefinition* GetStage(const FString& StageId);
	TArray<FString> GetAllStageIds() const;

	// Match management
	bool StartMatch(const FString& Fighter1Id, const FString& Fighter2Id, const FString& StageId, const FForge2MatchConfiguration& Config);
	void EndMatch();
	bool IsMatchActive() const { return bMatchActive; }

	// Round management
	void StartRound();
	void EndRound(int32 Winner); // 1 = P1, 2 = P2, 0 = Draw
	bool IsRoundActive() const;

	// Gameplay
	void ProcessInput(int32 PlayerIndex, int32 InputFlags);
	void Update(float DeltaTime);
	FForge2MatchState GetMatchState() const { return MatchState; }

	// Input processing
	FString CheckInputForMove(const TArray<int32>& InputBuffer, const FForge2FighterDefinition& Fighter, EForge2FighterState CurrentState) const;

	// Damage calculation
	int32 CalculateDamage(const FForge2MoveDefinition& Move, const FForge2ComboState& ComboState, bool bCounterHit, float DefenseModifier) const;

	// Collision detection
	bool CheckHitboxCollision(const FForge2HitboxData& Hitbox, const FForge2HurtboxData& Hurtbox, const FVector& AttackerPos, const FVector& DefenderPos, bool bAttackerFacingRight) const;

	// Training mode
	void EnableTrainingMode(const FForge2TrainingModeSettings& Settings);
	void DisableTrainingMode();
	bool IsTrainingMode() const { return bTrainingMode; }
	FForge2TrainingModeSettings GetTrainingSettings() const { return TrainingSettings; }

	// Clear all data
	void Reset();

private:
	TMap<FString, FForge2FighterDefinition> Fighters;
	TMap<FString, FForge2StageDefinition> Stages;

	bool bMatchActive = false;
	FString ActiveFighter1Id;
	FString ActiveFighter2Id;
	FString ActiveStageId;
	FForge2MatchConfiguration MatchConfig;
	FForge2MatchState MatchState;

	bool bTrainingMode = false;
	FForge2TrainingModeSettings TrainingSettings;

	// Internal helpers
	void UpdateFighterState(FForge2FighterRuntimeState& State, const FForge2FighterDefinition& Definition, float DeltaTime);
	void ProcessMove(int32 PlayerIndex, const FString& MoveId);
	void ApplyHit(int32 AttackerIndex, const FForge2MoveDefinition& Move, bool bCounterHit);
	void CheckRoundEnd();
};

// ============================================================================
// FIGHTING UTILITIES
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2FightingUtils
{
public:
	// Create common fighter presets
	static FForge2FighterDefinition CreateShotoFighter(const FString& FighterId, const FText& Name);
	static FForge2FighterDefinition CreateGrapplerFighter(const FString& FighterId, const FText& Name);
	static FForge2FighterDefinition CreateZonerFighter(const FString& FighterId, const FText& Name);

	// Create basic moveset
	static TArray<FForge2MoveDefinition> CreateBasicNormals();
	static FForge2MoveDefinition CreateFireball(const FString& MoveId, const FText& Name, int32 Damage, int32 MeterCost = 0);
	static FForge2MoveDefinition CreateDragonPunch(const FString& MoveId, const FText& Name, int32 Damage, int32 InvincibilityFrames);
	static FForge2MoveDefinition CreateCommandGrab(const FString& MoveId, const FText& Name, int32 Damage);

	// Create stages
	static FForge2StageDefinition CreateBasicStage(const FString& StageId, const FText& Name, float Width);
	static FForge2StageDefinition CreateCornerStage(const FString& StageId, const FText& Name, float Width, bool bBreakableWalls);

	// Frame data helpers
	static FForge2FrameData CreateFrameData(int32 Startup, int32 Active, int32 Recovery, int32 BlockAdv, int32 HitAdv);

	// Input parsing
	static EForge2MotionInput ParseMotionInput(const TArray<int32>& InputBuffer);
	static FString MotionInputToString(EForge2MotionInput Motion);
	static FString InputButtonToString(EForge2InputButton Button);

	// Combo calculation
	static int32 CalculateComboTotalDamage(const TArray<FForge2MoveDefinition>& Moves, const FForge2MatchConfiguration& Config);
};
