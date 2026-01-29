// Forge2CombatSystem.h
// FORGE2 Combat & Gameplay Core System
// Comprehensive combat system with melee, ranged, hit detection, targeting, dodge, stamina, and combo systems

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Forge2CombatSystem.generated.h"

// Forward declarations
class UAnimMontage;
class UNiagaraSystem;
class USoundBase;
class UCurveFloat;

//=============================================================================
// ENUMS
//=============================================================================

UENUM(BlueprintType)
enum class EForge2WeaponType : uint8
{
	// Melee
	Sword UMETA(DisplayName = "Sword"),
	Axe UMETA(DisplayName = "Axe"),
	Mace UMETA(DisplayName = "Mace"),
	Dagger UMETA(DisplayName = "Dagger"),
	Fist UMETA(DisplayName = "Fist/Unarmed"),
	Spear UMETA(DisplayName = "Spear"),
	Hammer UMETA(DisplayName = "Hammer"),
	Scythe UMETA(DisplayName = "Scythe"),
	Katana UMETA(DisplayName = "Katana"),
	Greatsword UMETA(DisplayName = "Greatsword"),
	DualBlades UMETA(DisplayName = "Dual Blades"),

	// Ranged
	Bow UMETA(DisplayName = "Bow"),
	Crossbow UMETA(DisplayName = "Crossbow"),
	Gun UMETA(DisplayName = "Gun/Pistol"),
	Rifle UMETA(DisplayName = "Rifle"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	Staff UMETA(DisplayName = "Staff"),
	Wand UMETA(DisplayName = "Wand"),
	ThrowingWeapon UMETA(DisplayName = "Throwing Weapon"),

	// Special
	Shield UMETA(DisplayName = "Shield"),
	None UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EForge2DamageType : uint8
{
	Physical UMETA(DisplayName = "Physical"),
	Slashing UMETA(DisplayName = "Slashing"),
	Piercing UMETA(DisplayName = "Piercing"),
	Blunt UMETA(DisplayName = "Blunt"),
	Fire UMETA(DisplayName = "Fire"),
	Ice UMETA(DisplayName = "Ice"),
	Lightning UMETA(DisplayName = "Lightning"),
	Poison UMETA(DisplayName = "Poison"),
	Holy UMETA(DisplayName = "Holy"),
	Dark UMETA(DisplayName = "Dark"),
	Arcane UMETA(DisplayName = "Arcane"),
	Nature UMETA(DisplayName = "Nature"),
	Psychic UMETA(DisplayName = "Psychic"),
	True UMETA(DisplayName = "True Damage")
};

UENUM(BlueprintType)
enum class EForge2AttackType : uint8
{
	Light UMETA(DisplayName = "Light Attack"),
	Medium UMETA(DisplayName = "Medium Attack"),
	Heavy UMETA(DisplayName = "Heavy Attack"),
	Special UMETA(DisplayName = "Special Attack"),
	Finisher UMETA(DisplayName = "Finisher"),
	Launcher UMETA(DisplayName = "Launcher"),
	Slam UMETA(DisplayName = "Slam"),
	Sweep UMETA(DisplayName = "Sweep"),
	Thrust UMETA(DisplayName = "Thrust"),
	Charged UMETA(DisplayName = "Charged Attack"),
	Aerial UMETA(DisplayName = "Aerial Attack"),
	Dash UMETA(DisplayName = "Dash Attack")
};

UENUM(BlueprintType)
enum class EForge2HitReactionType : uint8
{
	None UMETA(DisplayName = "None"),
	Flinch UMETA(DisplayName = "Flinch"),
	Stagger UMETA(DisplayName = "Stagger"),
	Knockback UMETA(DisplayName = "Knockback"),
	Knockdown UMETA(DisplayName = "Knockdown"),
	Launch UMETA(DisplayName = "Launch"),
	Crumple UMETA(DisplayName = "Crumple"),
	Stun UMETA(DisplayName = "Stun"),
	Freeze UMETA(DisplayName = "Freeze")
};

UENUM(BlueprintType)
enum class EForge2BlockType : uint8
{
	None UMETA(DisplayName = "None"),
	Block UMETA(DisplayName = "Block"),
	Parry UMETA(DisplayName = "Parry"),
	PerfectBlock UMETA(DisplayName = "Perfect Block"),
	GuardBreak UMETA(DisplayName = "Guard Break")
};

UENUM(BlueprintType)
enum class EForge2DodgeType : uint8
{
	Roll UMETA(DisplayName = "Roll"),
	Dash UMETA(DisplayName = "Dash"),
	Sidestep UMETA(DisplayName = "Sidestep"),
	Backstep UMETA(DisplayName = "Backstep"),
	Blink UMETA(DisplayName = "Blink/Teleport"),
	Phase UMETA(DisplayName = "Phase Through")
};

UENUM(BlueprintType)
enum class EForge2TargetPriority : uint8
{
	Closest UMETA(DisplayName = "Closest"),
	LowestHealth UMETA(DisplayName = "Lowest Health"),
	HighestThreat UMETA(DisplayName = "Highest Threat"),
	InCrosshair UMETA(DisplayName = "In Crosshair"),
	LastAttacker UMETA(DisplayName = "Last Attacker"),
	HighestDamage UMETA(DisplayName = "Highest Damage Dealer")
};

UENUM(BlueprintType)
enum class EForge2ComboInputType : uint8
{
	LightAttack UMETA(DisplayName = "Light Attack"),
	HeavyAttack UMETA(DisplayName = "Heavy Attack"),
	Special UMETA(DisplayName = "Special"),
	Dodge UMETA(DisplayName = "Dodge"),
	Jump UMETA(DisplayName = "Jump"),
	Block UMETA(DisplayName = "Block"),
	DirectionUp UMETA(DisplayName = "Direction Up"),
	DirectionDown UMETA(DisplayName = "Direction Down"),
	DirectionLeft UMETA(DisplayName = "Direction Left"),
	DirectionRight UMETA(DisplayName = "Direction Right"),
	DirectionForward UMETA(DisplayName = "Direction Forward"),
	DirectionBack UMETA(DisplayName = "Direction Back")
};

UENUM(BlueprintType)
enum class EForge2ProjectileType : uint8
{
	Physical UMETA(DisplayName = "Physical Projectile"),
	Hitscan UMETA(DisplayName = "Hitscan"),
	Beam UMETA(DisplayName = "Beam"),
	Arc UMETA(DisplayName = "Arc/Lobbed"),
	Homing UMETA(DisplayName = "Homing"),
	Explosive UMETA(DisplayName = "Explosive"),
	Piercing UMETA(DisplayName = "Piercing"),
	Bouncing UMETA(DisplayName = "Bouncing")
};

UENUM(BlueprintType)
enum class EForge2HitboxShape : uint8
{
	Box UMETA(DisplayName = "Box"),
	Sphere UMETA(DisplayName = "Sphere"),
	Capsule UMETA(DisplayName = "Capsule"),
	Mesh UMETA(DisplayName = "Mesh"),
	Trace UMETA(DisplayName = "Line Trace")
};

//=============================================================================
// DATA STRUCTURES - Attack/Damage
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DamageType DamageType = EForge2DamageType::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2HitReactionType HitReaction = EForge2HitReactionType::Flinch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KnockbackForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector KnockbackDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCrit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CritMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreArmor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmorPenetration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTag> StatusEffectsToApply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StatusEffectChance = 1.0f;

	// Calculate final damage
	float CalculateFinalDamage(float TargetArmor, float TargetResistance, bool bIsCrit) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HitResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bHit = false;

	UPROPERTY(BlueprintReadWrite)
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadWrite)
	AActor* HitActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FName HitBone = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	float FinalDamage = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	bool bWasCritical = false;

	UPROPERTY(BlueprintReadWrite)
	bool bWasBlocked = false;

	UPROPERTY(BlueprintReadWrite)
	bool bWasParried = false;

	UPROPERTY(BlueprintReadWrite)
	EForge2HitReactionType AppliedReaction = EForge2HitReactionType::None;

	UPROPERTY(BlueprintReadWrite)
	float HitStunDuration = 0.0f;
};

//=============================================================================
// DATA STRUCTURES - Melee Attack
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HitboxData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HitboxId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2HitboxShape Shape = EForge2HitboxShape::Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Offset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Extent = FVector(50.0f);  // Box half-extents or sphere radius

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CapsuleRadius = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CapsuleHalfHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachBone = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWeakPoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeakPointMultiplier = 1.5f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MeleeAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2AttackType AttackType = EForge2AttackType::Light;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2WeaponType RequiredWeapon = EForge2WeaponType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2DamageInfo DamageInfo;

	// Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate = 1.0f;

	// Frame data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartupFrames = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActiveFrames = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecoveryFrames = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStopDuration = 0.05f;

	// Hitbox
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2HitboxData> Hitboxes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitboxActiveStart = 0.2f;  // Normalized time

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitboxActiveEnd = 0.5f;

	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardMovement = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRootMotion = true;

	// Costs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ManaCost = 0.0f;

	// Cancel windows
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CancelWindowStart = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CancelWindowEnd = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCancelIntoBlock = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCancelIntoDodge = true;

	// Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> TrailEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> HitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> HitSound;

	// Combo links
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> NextAttackIds;  // Attacks that can chain from this

	// Calculate total duration in frames
	float GetTotalFrames() const { return StartupFrames + ActiveFrames + RecoveryFrames; }
	float GetTotalDuration(float FrameRate = 60.0f) const { return GetTotalFrames() / FrameRate; }
};

//=============================================================================
// DATA STRUCTURES - Ranged Combat
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ProjectileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProjectileId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2ProjectileType ProjectileType = EForge2ProjectileType::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2DamageInfo DamageInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRange = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Lifetime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Gravity = 0.0f;  // For arc projectiles

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HomingStrength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPenetrations = 0;  // 0 = stops on first hit

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxBounces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionFalloff = 1.0f;  // Damage falloff from center

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> ProjectileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> TrailEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> ImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> FlightSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ImpactSound;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RangedAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2WeaponType RequiredWeapon = EForge2WeaponType::Bow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2ProjectileData ProjectileData;

	// Accuracy
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseSpread = 0.0f;  // Degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpreadMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConsecutiveShotSpreadIncrease = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpreadRecoveryRate = 5.0f;

	// Fire rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 1.0f;  // Shots per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BurstCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BurstDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectilesPerShot = 1;  // Shotgun-style

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileSpread = 5.0f;  // For multiple projectiles

	// Ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineSize = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmmoPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReloadByRound = false;  // Like a shotgun

	// Charging
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCharge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChargeTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeMultiplier = 2.0f;

	// Costs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaCost = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ManaCost = 0.0f;

	// Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MuzzleSocketName = "Muzzle";

	// Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> EmptySound;

	// Aim down sights
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanADS = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ADSSpreadMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ADSMoveSpeedMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ADSZoom = 1.5f;
};

//=============================================================================
// DATA STRUCTURES - Blocking/Parrying
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BlockData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBlock = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanParry = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlockDamageReduction = 0.7f;  // 70% damage blocked

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ParryDamageReduction = 1.0f;  // 100% damage blocked

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ParryWindow = 0.2f;  // Seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectBlockWindow = 0.1f;  // Tighter window for perfect block

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlockStaminaCostMultiplier = 0.5f;  // Of attack's damage

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ParryStunDuration = 1.0f;  // Stun attacker

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlockRecovery = 0.3f;  // Recovery time after blocking

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxBlockAngle = 120.0f;  // Frontal arc

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GuardBreakThreshold = 50.0f;  // Damage to break guard

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EForge2DamageType> BlockableDamageTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EForge2DamageType> UnblockableDamageTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> BlockMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> ParryMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> BlockEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> ParryEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> BlockSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ParrySound;
};

//=============================================================================
// DATA STRUCTURES - Dodge System
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DodgeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DodgeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2DodgeType DodgeType = EForge2DodgeType::Roll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCharges = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeRegenTime = 2.0f;

	// I-frames
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasIFrames = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IFrameStart = 0.0f;  // Normalized time

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IFrameEnd = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EForge2DamageType> IFrameImmunity;  // Empty = immune to all

	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDirectional = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanDodgeInAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRootMotion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UCurveFloat> MovementCurve;

	// Cancel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackCancelWindow = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanChainDodge = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainDodgeCostMultiplier = 1.5f;

	// Animation/Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> DodgeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> DodgeSound;
};

//=============================================================================
// DATA STRUCTURES - Stamina System
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2StaminaConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RegenRate = 20.0f;  // Per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RegenDelay = 1.0f;  // After action

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExhaustedThreshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExhaustedRecoveryThreshold = 30.0f;  // Must reach this to recover

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExhaustedRegenMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintCostPerSecond = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClimbCostPerSecond = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBlockRegenWhileSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBlockRegenWhileBlocking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EquipmentWeightPenalty = 0.0f;  // Reduce regen per weight unit
};

//=============================================================================
// DATA STRUCTURES - Combo System
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComboInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2ComboInputType InputType = EForge2ComboInputType::LightAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHoldRequired = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HoldDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTimeSincePrevious = 1.0f;  // Combo drops if exceeded
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComboNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackId;  // Reference to attack data

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboHitNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageScaling = 1.0f;  // Combo damage scaling

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStunScaling = 1.0f;  // Reduced hitstun as combo progresses

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2ComboInput> RequiredInputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> NextNodeIds;  // Possible next nodes

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFinisher = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLauncher = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresTarget = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComboDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComboId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComboName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2WeaponType RequiredWeapon = EForge2WeaponType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2ComboNode> Nodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RootNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InputBufferTime = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboDropTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxComboLength = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDamageScaling = 0.3f;  // Can't go below this

	// Get combo node by ID
	const FForge2ComboNode* FindNode(const FString& NodeId) const;
};

//=============================================================================
// DATA STRUCTURES - Targeting System
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TargetingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableLockOn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSoftLock = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableAutoAim = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SoftLockRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoAimRange = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnAngle = 45.0f;  // Degrees from forward

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SoftLockAngle = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoAimAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoAimStrength = 0.5f;  // Blend factor

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2TargetPriority TargetPriority = EForge2TargetPriority::Closest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetSwitchCooldown = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBreakLockOnObstruction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstructionBreakDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLockOnCameraFacing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnCameraOffset = FVector::ZeroVector.Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag TargetableTag;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TargetInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	AActor* TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector TargetBoneLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FName TargetBone = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float Angle = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	bool bIsLocked = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsVisible = true;

	UPROPERTY(BlueprintReadWrite)
	float ThreatLevel = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float HealthPercent = 1.0f;
};

//=============================================================================
// DATA STRUCTURES - Hit Feedback
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HitFeedbackConfig
{
	GENERATED_BODY()

	// Camera shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableCameraShake = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraShakeIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraShakeDuration = 0.2f;

	// Hit stop/freeze frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableHitStop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStopDuration = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStopTimeScale = 0.1f;

	// Controller vibration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableControllerVibration = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VibrationIntensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VibrationDuration = 0.2f;

	// Screen effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableScreenFlash = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ScreenFlashColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScreenFlashDuration = 0.1f;

	// Damage numbers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDamageNumbers = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCriticalIndicator = true;
};

//=============================================================================
// DATA STRUCTURES - Combat Parameters
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MeleeSystemParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2MeleeAttackData> Attacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2ComboDefinition> Combos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2BlockData BlockConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2StaminaConfig StaminaConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2HitFeedbackConfig HitFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableHitboxVisualization = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RangedSystemParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2RangedAttackData> Attacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2TargetingConfig TargetingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2HitFeedbackConfig HitFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalAccuracyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableProjectileVisualization = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DodgeSystemParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FForge2DodgeData> DodgeTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DefaultDodgeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDirectionalDodge = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAirDodge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AirDodgeCount = 1;
};

//=============================================================================
// COMBAT SYSTEM CLASSES
//=============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2MeleeSystem
{
public:
	// Attack execution
	static FForge2HitResult ExecuteAttack(
		AActor* Attacker,
		const FForge2MeleeAttackData& AttackData,
		const TArray<AActor*>& PotentialTargets
	);

	// Hitbox processing
	static TArray<FForge2HitResult> ProcessHitboxes(
		AActor* Attacker,
		const TArray<FForge2HitboxData>& Hitboxes,
		const TArray<AActor*>& PotentialTargets,
		const FForge2DamageInfo& DamageInfo
	);

	// Block/Parry check
	static EForge2BlockType CheckBlock(
		AActor* Defender,
		AActor* Attacker,
		const FForge2BlockData& BlockConfig,
		float AttackTime
	);

	// Apply block
	static FForge2HitResult ApplyBlockedDamage(
		const FForge2HitResult& OriginalHit,
		EForge2BlockType BlockType,
		const FForge2BlockData& BlockConfig
	);

	// Combo validation
	static bool ValidateComboInput(
		const FForge2ComboDefinition& Combo,
		const FString& CurrentNodeId,
		const TArray<EForge2ComboInputType>& InputBuffer,
		float TimeSinceLastInput,
		FString& OutNextNodeId
	);

	// Create default melee attacks for weapon type
	static TArray<FForge2MeleeAttackData> CreateDefaultAttacks(EForge2WeaponType WeaponType);
};

class MCPAUTOMATIONBRIDGE_API FForge2RangedSystem
{
public:
	// Fire weapon
	static FForge2HitResult FireWeapon(
		AActor* Attacker,
		const FForge2RangedAttackData& AttackData,
		const FVector& AimDirection,
		float CurrentSpread
	);

	// Spawn projectile
	static AActor* SpawnProjectile(
		UWorld* World,
		const FForge2ProjectileData& ProjectileData,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation,
		AActor* Instigator
	);

	// Hitscan
	static FForge2HitResult PerformHitscan(
		UWorld* World,
		AActor* Attacker,
		const FForge2RangedAttackData& AttackData,
		const FVector& Start,
		const FVector& End
	);

	// Calculate spread
	static FVector ApplySpread(const FVector& AimDirection, float SpreadAngle, int32 Seed);

	// Create default ranged attacks for weapon type
	static TArray<FForge2RangedAttackData> CreateDefaultAttacks(EForge2WeaponType WeaponType);
};

class MCPAUTOMATIONBRIDGE_API FForge2DodgeSystem
{
public:
	// Execute dodge
	static bool ExecuteDodge(
		AActor* Character,
		const FForge2DodgeData& DodgeData,
		const FVector& Direction,
		float& OutIFrameStart,
		float& OutIFrameEnd
	);

	// Check i-frame immunity
	static bool IsInIFrames(
		float CurrentTime,
		float DodgeStartTime,
		float Duration,
		float IFrameStart,
		float IFrameEnd
	);

	// Create default dodge data
	static FForge2DodgeData CreateDefaultDodge(EForge2DodgeType DodgeType);
};

class MCPAUTOMATIONBRIDGE_API FForge2StaminaSystem
{
public:
	// Consume stamina
	static bool ConsumeStamina(
		float& CurrentStamina,
		float Cost,
		const FForge2StaminaConfig& Config,
		bool& bIsExhausted
	);

	// Regenerate stamina
	static void RegenerateStamina(
		float& CurrentStamina,
		float DeltaTime,
		float& RegenDelayRemaining,
		const FForge2StaminaConfig& Config,
		bool bIsExhausted,
		bool bIsSprinting,
		bool bIsBlocking
	);

	// Check exhaustion recovery
	static bool CheckExhaustionRecovery(
		float CurrentStamina,
		const FForge2StaminaConfig& Config,
		bool& bIsExhausted
	);
};

class MCPAUTOMATIONBRIDGE_API FForge2TargetingSystem
{
public:
	// Find targets in range
	static TArray<FForge2TargetInfo> FindTargetsInRange(
		AActor* Seeker,
		const FForge2TargetingConfig& Config,
		const TArray<AActor*>& PotentialTargets
	);

	// Select best target
	static FForge2TargetInfo SelectBestTarget(
		AActor* Seeker,
		const TArray<FForge2TargetInfo>& Targets,
		const FForge2TargetingConfig& Config
	);

	// Update lock-on
	static bool UpdateLockOn(
		AActor* Seeker,
		AActor* CurrentTarget,
		const FForge2TargetingConfig& Config,
		float DeltaTime,
		float& ObstructionTime
	);

	// Switch target
	static FForge2TargetInfo SwitchTarget(
		AActor* Seeker,
		AActor* CurrentTarget,
		const TArray<FForge2TargetInfo>& Targets,
		const FVector& SwitchDirection
	);

	// Apply auto-aim
	static FVector ApplyAutoAim(
		const FVector& AimDirection,
		const FForge2TargetInfo& Target,
		const FForge2TargetingConfig& Config
	);
};

class MCPAUTOMATIONBRIDGE_API FForge2HitFeedback
{
public:
	// Apply all hit feedback
	static void ApplyHitFeedback(
		AActor* Attacker,
		AActor* Victim,
		const FForge2HitResult& HitResult,
		const FForge2HitFeedbackConfig& Config
	);

	// Camera shake
	static void ApplyCameraShake(
		APlayerController* Controller,
		const FForge2HitFeedbackConfig& Config,
		bool bWasCritical
	);

	// Hit stop
	static void ApplyHitStop(
		UWorld* World,
		const FForge2HitFeedbackConfig& Config,
		bool bWasCritical
	);

	// Controller vibration
	static void ApplyControllerVibration(
		APlayerController* Controller,
		const FForge2HitFeedbackConfig& Config,
		bool bWasCritical
	);
};

//=============================================================================
// UTILITY CLASS
//=============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2CombatUtils
{
public:
	// Calculate damage with armor/resistance
	static float CalculateDamage(
		float BaseDamage,
		float ArmorValue,
		float Resistance,
		float ArmorPenetration,
		bool bIgnoreArmor
	);

	// Calculate critical hit
	static bool RollCritical(float CritChance, int32 Seed = -1);

	// Apply hit reaction
	static EForge2HitReactionType DetermineHitReaction(
		float Damage,
		float Poise,
		EForge2HitReactionType BaseReaction
	);

	// Calculate knockback
	static FVector CalculateKnockback(
		const FVector& AttackerLocation,
		const FVector& VictimLocation,
		float KnockbackForce,
		const FVector& CustomDirection
	);

	// Get attack from ID
	static const FForge2MeleeAttackData* FindMeleeAttack(
		const TArray<FForge2MeleeAttackData>& Attacks,
		const FString& AttackId
	);

	static const FForge2RangedAttackData* FindRangedAttack(
		const TArray<FForge2RangedAttackData>& Attacks,
		const FString& AttackId
	);

	// Hitbox visualization (editor only)
	static void DrawHitboxDebug(
		UWorld* World,
		const FForge2HitboxData& Hitbox,
		const FTransform& Transform,
		float Duration = 0.5f
	);

	// Frame to time conversion
	static float FramesToSeconds(float Frames, float FrameRate = 60.0f);
	static float SecondsToFrames(float Seconds, float FrameRate = 60.0f);
};

#endif // Include guard is handled by #pragma once
