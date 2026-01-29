// Forge2CombatSystem.cpp
// FORGE2 Combat & Gameplay Core System Implementation

#include "Forge2CombatSystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

//=============================================================================
// FForge2DamageInfo Implementation
//=============================================================================

float FForge2DamageInfo::CalculateFinalDamage(float TargetArmor, float TargetResistance, bool bIsCrit) const
{
	float FinalDamage = BaseDamage * DamageMultiplier;

	// Apply critical hit
	if (bIsCrit && bCanCrit)
	{
		FinalDamage *= CritMultiplier;
	}

	// Apply armor
	if (!bIgnoreArmor && TargetArmor > 0.0f)
	{
		float EffectiveArmor = FMath::Max(0.0f, TargetArmor - ArmorPenetration);
		// Armor formula: damage reduction = armor / (armor + 100)
		float DamageReduction = EffectiveArmor / (EffectiveArmor + 100.0f);
		FinalDamage *= (1.0f - DamageReduction);
	}

	// Apply resistance (percentage based)
	if (TargetResistance != 0.0f)
	{
		FinalDamage *= (1.0f - FMath::Clamp(TargetResistance, -1.0f, 0.9f));
	}

	return FMath::Max(0.0f, FinalDamage);
}

//=============================================================================
// FForge2ComboDefinition Implementation
//=============================================================================

const FForge2ComboNode* FForge2ComboDefinition::FindNode(const FString& NodeId) const
{
	for (const FForge2ComboNode& Node : Nodes)
	{
		if (Node.NodeId == NodeId)
		{
			return &Node;
		}
	}
	return nullptr;
}

//=============================================================================
// FForge2MeleeSystem Implementation
//=============================================================================

FForge2HitResult FForge2MeleeSystem::ExecuteAttack(
	AActor* Attacker,
	const FForge2MeleeAttackData& AttackData,
	const TArray<AActor*>& PotentialTargets)
{
	FForge2HitResult Result;
	if (!Attacker)
	{
		return Result;
	}

	// Process all hitboxes
	TArray<FForge2HitResult> Hits = ProcessHitboxes(
		Attacker,
		AttackData.Hitboxes,
		PotentialTargets,
		AttackData.DamageInfo
	);

	// Return the most significant hit (highest damage)
	float MaxDamage = 0.0f;
	for (const FForge2HitResult& Hit : Hits)
	{
		if (Hit.bHit && Hit.FinalDamage > MaxDamage)
		{
			MaxDamage = Hit.FinalDamage;
			Result = Hit;
		}
	}

	return Result;
}

TArray<FForge2HitResult> FForge2MeleeSystem::ProcessHitboxes(
	AActor* Attacker,
	const TArray<FForge2HitboxData>& Hitboxes,
	const TArray<AActor*>& PotentialTargets,
	const FForge2DamageInfo& DamageInfo)
{
	TArray<FForge2HitResult> Results;
	if (!Attacker)
	{
		return Results;
	}

	UWorld* World = Attacker->GetWorld();
	if (!World)
	{
		return Results;
	}

	TSet<AActor*> AlreadyHit;  // Prevent double hits

	for (const FForge2HitboxData& Hitbox : Hitboxes)
	{
		FTransform HitboxTransform;

		// Get hitbox world transform
		if (Hitbox.AttachBone != NAME_None)
		{
			// Try to get bone transform from skeletal mesh
			if (USkeletalMeshComponent* SkelMesh = Attacker->FindComponentByClass<USkeletalMeshComponent>())
			{
				FTransform BoneTransform = SkelMesh->GetBoneTransform(Hitbox.AttachBone);
				HitboxTransform = FTransform(Hitbox.Rotation, Hitbox.Offset) * BoneTransform;
			}
			else
			{
				HitboxTransform = FTransform(Hitbox.Rotation, Hitbox.Offset) * Attacker->GetActorTransform();
			}
		}
		else
		{
			HitboxTransform = FTransform(Hitbox.Rotation, Hitbox.Offset) * Attacker->GetActorTransform();
		}

		FVector HitboxCenter = HitboxTransform.GetLocation();

		// Check against potential targets
		for (AActor* Target : PotentialTargets)
		{
			if (!Target || Target == Attacker || AlreadyHit.Contains(Target))
			{
				continue;
			}

			bool bHit = false;
			FVector ClosestPoint = Target->GetActorLocation();

			// Get target bounds
			FVector TargetOrigin, TargetExtent;
			Target->GetActorBounds(true, TargetOrigin, TargetExtent);

			// Check collision based on hitbox shape
			switch (Hitbox.Shape)
			{
			case EForge2HitboxShape::Sphere:
			{
				float Distance = FVector::Dist(HitboxCenter, TargetOrigin);
				float CombinedRadius = Hitbox.Extent.X + TargetExtent.GetMax();
				bHit = Distance <= CombinedRadius;
				break;
			}
			case EForge2HitboxShape::Box:
			{
				FBox HitboxBounds(HitboxCenter - Hitbox.Extent, HitboxCenter + Hitbox.Extent);
				FBox TargetBounds(TargetOrigin - TargetExtent, TargetOrigin + TargetExtent);
				bHit = HitboxBounds.Intersect(TargetBounds);
				break;
			}
			case EForge2HitboxShape::Capsule:
			{
				// Simplified capsule check - use distance to line segment
				FVector CapsuleStart = HitboxCenter - FVector(0, 0, Hitbox.CapsuleHalfHeight);
				FVector CapsuleEnd = HitboxCenter + FVector(0, 0, Hitbox.CapsuleHalfHeight);
				float Distance = FMath::PointDistToSegment(TargetOrigin, CapsuleStart, CapsuleEnd);
				bHit = Distance <= (Hitbox.CapsuleRadius + TargetExtent.GetMax());
				break;
			}
			case EForge2HitboxShape::Trace:
			{
				FHitResult HitResult;
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(Attacker);

				FVector TraceEnd = HitboxCenter + HitboxTransform.GetRotation().GetForwardVector() * Hitbox.Extent.X;
				bHit = World->LineTraceSingleByChannel(
					HitResult,
					HitboxCenter,
					TraceEnd,
					ECC_Pawn,
					QueryParams
				);

				if (bHit && HitResult.GetActor() == Target)
				{
					ClosestPoint = HitResult.ImpactPoint;
				}
				else
				{
					bHit = false;
				}
				break;
			}
			default:
				break;
			}

			if (bHit)
			{
				AlreadyHit.Add(Target);

				FForge2HitResult HitResult;
				HitResult.bHit = true;
				HitResult.HitActor = Target;
				HitResult.HitLocation = ClosestPoint;
				HitResult.HitNormal = (Attacker->GetActorLocation() - ClosestPoint).GetSafeNormal();

				// Calculate damage
				float DamageMultiplier = Hitbox.DamageMultiplier;
				if (Hitbox.bIsWeakPoint)
				{
					DamageMultiplier *= Hitbox.WeakPointMultiplier;
				}

				// Roll for critical
				HitResult.bWasCritical = FForge2CombatUtils::RollCritical(0.1f);  // Default 10% crit

				// Calculate final damage (assuming 0 armor/resistance for now - should come from target)
				float FinalDamage = DamageInfo.BaseDamage * DamageInfo.DamageMultiplier * DamageMultiplier;
				if (HitResult.bWasCritical && DamageInfo.bCanCrit)
				{
					FinalDamage *= DamageInfo.CritMultiplier;
				}

				HitResult.FinalDamage = FinalDamage;
				HitResult.AppliedReaction = DamageInfo.HitReaction;

				Results.Add(HitResult);
			}
		}
	}

	return Results;
}

EForge2BlockType FForge2MeleeSystem::CheckBlock(
	AActor* Defender,
	AActor* Attacker,
	const FForge2BlockData& BlockConfig,
	float AttackTime)
{
	if (!Defender || !Attacker)
	{
		return EForge2BlockType::None;
	}

	// Check if defender is facing attacker
	FVector ToAttacker = (Attacker->GetActorLocation() - Defender->GetActorLocation()).GetSafeNormal();
	FVector DefenderForward = Defender->GetActorForwardVector();
	float Angle = FMath::Acos(FVector::DotProduct(ToAttacker, DefenderForward)) * (180.0f / PI);

	if (Angle > BlockConfig.MaxBlockAngle * 0.5f)
	{
		return EForge2BlockType::None;
	}

	// Check for parry (tighter timing)
	if (BlockConfig.bCanParry && AttackTime <= BlockConfig.PerfectBlockWindow)
	{
		return EForge2BlockType::PerfectBlock;
	}
	else if (BlockConfig.bCanParry && AttackTime <= BlockConfig.ParryWindow)
	{
		return EForge2BlockType::Parry;
	}
	else if (BlockConfig.bCanBlock)
	{
		return EForge2BlockType::Block;
	}

	return EForge2BlockType::None;
}

FForge2HitResult FForge2MeleeSystem::ApplyBlockedDamage(
	const FForge2HitResult& OriginalHit,
	EForge2BlockType BlockType,
	const FForge2BlockData& BlockConfig)
{
	FForge2HitResult Result = OriginalHit;

	switch (BlockType)
	{
	case EForge2BlockType::PerfectBlock:
	case EForge2BlockType::Parry:
		Result.bWasParried = true;
		Result.FinalDamage *= (1.0f - BlockConfig.ParryDamageReduction);
		break;

	case EForge2BlockType::Block:
		Result.bWasBlocked = true;
		Result.FinalDamage *= (1.0f - BlockConfig.BlockDamageReduction);
		break;

	default:
		break;
	}

	return Result;
}

bool FForge2MeleeSystem::ValidateComboInput(
	const FForge2ComboDefinition& Combo,
	const FString& CurrentNodeId,
	const TArray<EForge2ComboInputType>& InputBuffer,
	float TimeSinceLastInput,
	FString& OutNextNodeId)
{
	const FForge2ComboNode* CurrentNode = Combo.FindNode(CurrentNodeId);
	if (!CurrentNode)
	{
		// Start from root
		CurrentNode = Combo.FindNode(Combo.RootNodeId);
		if (!CurrentNode)
		{
			return false;
		}
	}

	// Check timing
	if (TimeSinceLastInput > Combo.InputBufferTime)
	{
		// Input too late, check if combo dropped
		if (TimeSinceLastInput > Combo.ComboDropTime)
		{
			OutNextNodeId = Combo.RootNodeId;
			return false;
		}
	}

	// Find matching next node
	for (const FString& NextNodeId : CurrentNode->NextNodeIds)
	{
		const FForge2ComboNode* NextNode = Combo.FindNode(NextNodeId);
		if (!NextNode)
		{
			continue;
		}

		// Check if input matches required inputs
		bool bInputsMatch = true;
		if (NextNode->RequiredInputs.Num() > 0 && InputBuffer.Num() > 0)
		{
			for (const FForge2ComboInput& RequiredInput : NextNode->RequiredInputs)
			{
				bool bFoundInput = false;
				for (EForge2ComboInputType BufferedInput : InputBuffer)
				{
					if (BufferedInput == RequiredInput.InputType)
					{
						bFoundInput = true;
						break;
					}
				}

				if (!bFoundInput)
				{
					bInputsMatch = false;
					break;
				}
			}
		}

		if (bInputsMatch)
		{
			OutNextNodeId = NextNodeId;
			return true;
		}
	}

	return false;
}

TArray<FForge2MeleeAttackData> FForge2MeleeSystem::CreateDefaultAttacks(EForge2WeaponType WeaponType)
{
	TArray<FForge2MeleeAttackData> Attacks;

	FForge2MeleeAttackData LightAttack;
	LightAttack.AttackId = TEXT("light_1");
	LightAttack.AttackName = TEXT("Light Attack 1");
	LightAttack.AttackType = EForge2AttackType::Light;
	LightAttack.RequiredWeapon = WeaponType;
	LightAttack.DamageInfo.BaseDamage = 20.0f;
	LightAttack.StartupFrames = 4.0f;
	LightAttack.ActiveFrames = 6.0f;
	LightAttack.RecoveryFrames = 10.0f;
	LightAttack.StaminaCost = 10.0f;

	FForge2HitboxData LightHitbox;
	LightHitbox.HitboxId = TEXT("light_hitbox");
	LightHitbox.Shape = EForge2HitboxShape::Sphere;
	LightHitbox.Offset = FVector(100.0f, 0.0f, 50.0f);
	LightHitbox.Extent = FVector(60.0f);
	LightAttack.Hitboxes.Add(LightHitbox);
	LightAttack.NextAttackIds.Add(TEXT("light_2"));

	Attacks.Add(LightAttack);

	FForge2MeleeAttackData LightAttack2;
	LightAttack2.AttackId = TEXT("light_2");
	LightAttack2.AttackName = TEXT("Light Attack 2");
	LightAttack2.AttackType = EForge2AttackType::Light;
	LightAttack2.RequiredWeapon = WeaponType;
	LightAttack2.DamageInfo.BaseDamage = 25.0f;
	LightAttack2.StartupFrames = 5.0f;
	LightAttack2.ActiveFrames = 8.0f;
	LightAttack2.RecoveryFrames = 12.0f;
	LightAttack2.StaminaCost = 12.0f;
	LightAttack2.Hitboxes.Add(LightHitbox);
	LightAttack2.NextAttackIds.Add(TEXT("light_3"));
	LightAttack2.NextAttackIds.Add(TEXT("heavy_finisher"));

	Attacks.Add(LightAttack2);

	FForge2MeleeAttackData LightAttack3;
	LightAttack3.AttackId = TEXT("light_3");
	LightAttack3.AttackName = TEXT("Light Attack 3");
	LightAttack3.AttackType = EForge2AttackType::Light;
	LightAttack3.RequiredWeapon = WeaponType;
	LightAttack3.DamageInfo.BaseDamage = 30.0f;
	LightAttack3.StartupFrames = 6.0f;
	LightAttack3.ActiveFrames = 10.0f;
	LightAttack3.RecoveryFrames = 15.0f;
	LightAttack3.StaminaCost = 15.0f;
	LightAttack3.Hitboxes.Add(LightHitbox);

	Attacks.Add(LightAttack3);

	FForge2MeleeAttackData HeavyAttack;
	HeavyAttack.AttackId = TEXT("heavy_1");
	HeavyAttack.AttackName = TEXT("Heavy Attack");
	HeavyAttack.AttackType = EForge2AttackType::Heavy;
	HeavyAttack.RequiredWeapon = WeaponType;
	HeavyAttack.DamageInfo.BaseDamage = 50.0f;
	HeavyAttack.DamageInfo.HitReaction = EForge2HitReactionType::Stagger;
	HeavyAttack.StartupFrames = 15.0f;
	HeavyAttack.ActiveFrames = 10.0f;
	HeavyAttack.RecoveryFrames = 25.0f;
	HeavyAttack.StaminaCost = 30.0f;

	FForge2HitboxData HeavyHitbox;
	HeavyHitbox.HitboxId = TEXT("heavy_hitbox");
	HeavyHitbox.Shape = EForge2HitboxShape::Sphere;
	HeavyHitbox.Offset = FVector(120.0f, 0.0f, 50.0f);
	HeavyHitbox.Extent = FVector(80.0f);
	HeavyAttack.Hitboxes.Add(HeavyHitbox);
	HeavyAttack.NextAttackIds.Add(TEXT("heavy_finisher"));

	Attacks.Add(HeavyAttack);

	FForge2MeleeAttackData HeavyFinisher;
	HeavyFinisher.AttackId = TEXT("heavy_finisher");
	HeavyFinisher.AttackName = TEXT("Heavy Finisher");
	HeavyFinisher.AttackType = EForge2AttackType::Finisher;
	HeavyFinisher.RequiredWeapon = WeaponType;
	HeavyFinisher.DamageInfo.BaseDamage = 80.0f;
	HeavyFinisher.DamageInfo.HitReaction = EForge2HitReactionType::Knockdown;
	HeavyFinisher.DamageInfo.KnockbackForce = 500.0f;
	HeavyFinisher.StartupFrames = 20.0f;
	HeavyFinisher.ActiveFrames = 15.0f;
	HeavyFinisher.RecoveryFrames = 30.0f;
	HeavyFinisher.StaminaCost = 40.0f;

	FForge2HitboxData FinisherHitbox;
	FinisherHitbox.HitboxId = TEXT("finisher_hitbox");
	FinisherHitbox.Shape = EForge2HitboxShape::Sphere;
	FinisherHitbox.Offset = FVector(150.0f, 0.0f, 50.0f);
	FinisherHitbox.Extent = FVector(100.0f);
	HeavyFinisher.Hitboxes.Add(FinisherHitbox);

	Attacks.Add(HeavyFinisher);

	return Attacks;
}

//=============================================================================
// FForge2RangedSystem Implementation
//=============================================================================

FForge2HitResult FForge2RangedSystem::FireWeapon(
	AActor* Attacker,
	const FForge2RangedAttackData& AttackData,
	const FVector& AimDirection,
	float CurrentSpread)
{
	FForge2HitResult Result;
	if (!Attacker)
	{
		return Result;
	}

	UWorld* World = Attacker->GetWorld();
	if (!World)
	{
		return Result;
	}

	// Get muzzle location
	FVector MuzzleLocation = Attacker->GetActorLocation();
	if (USkeletalMeshComponent* SkelMesh = Attacker->FindComponentByClass<USkeletalMeshComponent>())
	{
		FTransform SocketTransform = SkelMesh->GetSocketTransform(AttackData.MuzzleSocketName);
		MuzzleLocation = SocketTransform.GetLocation();
	}

	// Apply spread
	float TotalSpread = AttackData.BaseSpread + CurrentSpread;
	FVector FinalDirection = ApplySpread(AimDirection, TotalSpread, FMath::Rand());

	// Fire based on projectile type
	if (AttackData.ProjectileData.ProjectileType == EForge2ProjectileType::Hitscan)
	{
		FVector TraceEnd = MuzzleLocation + FinalDirection * AttackData.ProjectileData.MaxRange;
		Result = PerformHitscan(World, Attacker, AttackData, MuzzleLocation, TraceEnd);
	}
	else
	{
		// Spawn projectile
		FRotator SpawnRotation = FinalDirection.Rotation();
		AActor* Projectile = SpawnProjectile(
			World,
			AttackData.ProjectileData,
			MuzzleLocation,
			SpawnRotation,
			Attacker
		);

		if (Projectile)
		{
			Result.bHit = true;  // Projectile spawned successfully
			Result.HitLocation = MuzzleLocation;
		}
	}

	return Result;
}

AActor* FForge2RangedSystem::SpawnProjectile(
	UWorld* World,
	const FForge2ProjectileData& ProjectileData,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation,
	AActor* Instigator)
{
	if (!World)
	{
		return nullptr;
	}

	// In a real implementation, this would spawn a projectile actor class
	// For now, we return nullptr as we'd need the actual projectile BP class
	// The TypeScript handler should handle spawning via Unreal's actor system

	return nullptr;
}

FForge2HitResult FForge2RangedSystem::PerformHitscan(
	UWorld* World,
	AActor* Attacker,
	const FForge2RangedAttackData& AttackData,
	const FVector& Start,
	const FVector& End)
{
	FForge2HitResult Result;
	if (!World || !Attacker)
	{
		return Result;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Attacker);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		Result.bHit = true;
		Result.HitActor = HitResult.GetActor();
		Result.HitLocation = HitResult.ImpactPoint;
		Result.HitNormal = HitResult.ImpactNormal;
		Result.HitBone = HitResult.BoneName;

		// Calculate damage with falloff
		float Distance = FVector::Dist(Start, HitResult.ImpactPoint);
		float DistanceRatio = Distance / AttackData.ProjectileData.MaxRange;
		float DamageFalloff = 1.0f;  // Could use a curve here

		Result.bWasCritical = FForge2CombatUtils::RollCritical(0.1f);
		Result.FinalDamage = AttackData.ProjectileData.DamageInfo.CalculateFinalDamage(0.0f, 0.0f, Result.bWasCritical);
		Result.FinalDamage *= DamageFalloff;
	}

	return Result;
}

FVector FForge2RangedSystem::ApplySpread(const FVector& AimDirection, float SpreadAngle, int32 Seed)
{
	if (SpreadAngle <= 0.0f)
	{
		return AimDirection;
	}

	FRandomStream RandomStream(Seed);

	// Random angle within cone
	float HalfAngleRad = FMath::DegreesToRadians(SpreadAngle * 0.5f);
	float RandomAngle = RandomStream.FRand() * 2.0f * PI;
	float RandomRadius = FMath::Sqrt(RandomStream.FRand()) * FMath::Sin(HalfAngleRad);

	// Calculate perpendicular vectors
	FVector Right = FVector::CrossProduct(AimDirection, FVector::UpVector);
	if (Right.IsNearlyZero())
	{
		Right = FVector::CrossProduct(AimDirection, FVector::RightVector);
	}
	Right.Normalize();

	FVector Up = FVector::CrossProduct(Right, AimDirection);
	Up.Normalize();

	// Apply offset
	FVector SpreadOffset = Right * (FMath::Cos(RandomAngle) * RandomRadius) +
						   Up * (FMath::Sin(RandomAngle) * RandomRadius);

	FVector Result = AimDirection * FMath::Cos(HalfAngleRad) + SpreadOffset;
	Result.Normalize();

	return Result;
}

TArray<FForge2RangedAttackData> FForge2RangedSystem::CreateDefaultAttacks(EForge2WeaponType WeaponType)
{
	TArray<FForge2RangedAttackData> Attacks;

	FForge2RangedAttackData DefaultAttack;
	DefaultAttack.AttackId = TEXT("primary_fire");
	DefaultAttack.AttackName = TEXT("Primary Fire");
	DefaultAttack.RequiredWeapon = WeaponType;

	switch (WeaponType)
	{
	case EForge2WeaponType::Bow:
		DefaultAttack.ProjectileData.ProjectileType = EForge2ProjectileType::Physical;
		DefaultAttack.ProjectileData.Speed = 5000.0f;
		DefaultAttack.ProjectileData.DamageInfo.BaseDamage = 40.0f;
		DefaultAttack.ProjectileData.Gravity = 980.0f;
		DefaultAttack.FireRate = 1.0f;
		DefaultAttack.bCanCharge = true;
		DefaultAttack.ChargeMultiplier = 2.0f;
		DefaultAttack.BaseSpread = 1.0f;
		break;

	case EForge2WeaponType::Gun:
		DefaultAttack.ProjectileData.ProjectileType = EForge2ProjectileType::Hitscan;
		DefaultAttack.ProjectileData.DamageInfo.BaseDamage = 25.0f;
		DefaultAttack.FireRate = 3.0f;
		DefaultAttack.MagazineSize = 12;
		DefaultAttack.ReloadTime = 1.5f;
		DefaultAttack.BaseSpread = 2.0f;
		break;

	case EForge2WeaponType::Rifle:
		DefaultAttack.ProjectileData.ProjectileType = EForge2ProjectileType::Hitscan;
		DefaultAttack.ProjectileData.DamageInfo.BaseDamage = 35.0f;
		DefaultAttack.FireRate = 8.0f;
		DefaultAttack.bAutomatic = true;
		DefaultAttack.MagazineSize = 30;
		DefaultAttack.ReloadTime = 2.5f;
		DefaultAttack.BaseSpread = 1.5f;
		DefaultAttack.ConsecutiveShotSpreadIncrease = 0.3f;
		break;

	case EForge2WeaponType::Shotgun:
		DefaultAttack.ProjectileData.ProjectileType = EForge2ProjectileType::Hitscan;
		DefaultAttack.ProjectileData.DamageInfo.BaseDamage = 15.0f;
		DefaultAttack.ProjectilesPerShot = 8;
		DefaultAttack.ProjectileSpread = 10.0f;
		DefaultAttack.FireRate = 1.0f;
		DefaultAttack.MagazineSize = 8;
		DefaultAttack.bReloadByRound = true;
		DefaultAttack.ReloadTime = 0.5f;  // Per shell
		break;

	case EForge2WeaponType::Staff:
		DefaultAttack.ProjectileData.ProjectileType = EForge2ProjectileType::Physical;
		DefaultAttack.ProjectileData.Speed = 3000.0f;
		DefaultAttack.ProjectileData.DamageInfo.BaseDamage = 30.0f;
		DefaultAttack.ProjectileData.DamageInfo.DamageType = EForge2DamageType::Arcane;
		DefaultAttack.FireRate = 2.0f;
		DefaultAttack.ManaCost = 20.0f;
		DefaultAttack.StaminaCost = 0.0f;
		DefaultAttack.BaseSpread = 0.5f;
		break;

	case EForge2WeaponType::Wand:
		DefaultAttack.ProjectileData.ProjectileType = EForge2ProjectileType::Physical;
		DefaultAttack.ProjectileData.Speed = 4000.0f;
		DefaultAttack.ProjectileData.DamageInfo.BaseDamage = 15.0f;
		DefaultAttack.ProjectileData.DamageInfo.DamageType = EForge2DamageType::Arcane;
		DefaultAttack.FireRate = 4.0f;
		DefaultAttack.ManaCost = 10.0f;
		DefaultAttack.StaminaCost = 0.0f;
		DefaultAttack.BaseSpread = 1.0f;
		break;

	default:
		DefaultAttack.ProjectileData.ProjectileType = EForge2ProjectileType::Physical;
		DefaultAttack.ProjectileData.Speed = 3000.0f;
		DefaultAttack.ProjectileData.DamageInfo.BaseDamage = 20.0f;
		DefaultAttack.FireRate = 2.0f;
		break;
	}

	Attacks.Add(DefaultAttack);

	return Attacks;
}

//=============================================================================
// FForge2DodgeSystem Implementation
//=============================================================================

bool FForge2DodgeSystem::ExecuteDodge(
	AActor* Character,
	const FForge2DodgeData& DodgeData,
	const FVector& Direction,
	float& OutIFrameStart,
	float& OutIFrameEnd)
{
	if (!Character)
	{
		return false;
	}

	// Calculate i-frame times
	OutIFrameStart = DodgeData.IFrameStart * DodgeData.Duration;
	OutIFrameEnd = DodgeData.IFrameEnd * DodgeData.Duration;

	// The actual movement would be handled by the character movement component
	// This function just calculates the timing
	return true;
}

bool FForge2DodgeSystem::IsInIFrames(
	float CurrentTime,
	float DodgeStartTime,
	float Duration,
	float IFrameStart,
	float IFrameEnd)
{
	if (Duration <= 0.0f) return false;
	
	float ElapsedTime = CurrentTime - DodgeStartTime;
	float NormalizedTime = ElapsedTime / Duration;

	return NormalizedTime >= IFrameStart && NormalizedTime <= IFrameEnd;
}

FForge2DodgeData FForge2DodgeSystem::CreateDefaultDodge(EForge2DodgeType DodgeType)
{
	FForge2DodgeData DodgeData;
	DodgeData.DodgeType = DodgeType;

	switch (DodgeType)
	{
	case EForge2DodgeType::Roll:
		DodgeData.DodgeId = TEXT("roll");
		DodgeData.Distance = 400.0f;
		DodgeData.Duration = 0.6f;
		DodgeData.StaminaCost = 20.0f;
		DodgeData.IFrameStart = 0.1f;
		DodgeData.IFrameEnd = 0.5f;
		DodgeData.AttackCancelWindow = 0.4f;
		break;

	case EForge2DodgeType::Dash:
		DodgeData.DodgeId = TEXT("dash");
		DodgeData.Distance = 300.0f;
		DodgeData.Duration = 0.3f;
		DodgeData.StaminaCost = 15.0f;
		DodgeData.IFrameStart = 0.0f;
		DodgeData.IFrameEnd = 0.4f;
		DodgeData.AttackCancelWindow = 0.2f;
		DodgeData.MaxCharges = 2;
		DodgeData.ChargeRegenTime = 3.0f;
		break;

	case EForge2DodgeType::Sidestep:
		DodgeData.DodgeId = TEXT("sidestep");
		DodgeData.Distance = 200.0f;
		DodgeData.Duration = 0.25f;
		DodgeData.StaminaCost = 10.0f;
		DodgeData.IFrameStart = 0.0f;
		DodgeData.IFrameEnd = 0.3f;
		DodgeData.AttackCancelWindow = 0.15f;
		break;

	case EForge2DodgeType::Backstep:
		DodgeData.DodgeId = TEXT("backstep");
		DodgeData.Distance = 250.0f;
		DodgeData.Duration = 0.35f;
		DodgeData.StaminaCost = 12.0f;
		DodgeData.IFrameStart = 0.05f;
		DodgeData.IFrameEnd = 0.25f;
		DodgeData.bDirectional = false;
		break;

	case EForge2DodgeType::Blink:
		DodgeData.DodgeId = TEXT("blink");
		DodgeData.Distance = 500.0f;
		DodgeData.Duration = 0.1f;
		DodgeData.StaminaCost = 30.0f;
		DodgeData.IFrameStart = 0.0f;
		DodgeData.IFrameEnd = 1.0f;
		DodgeData.Cooldown = 5.0f;
		break;

	default:
		DodgeData.DodgeId = TEXT("default_dodge");
		DodgeData.Distance = 300.0f;
		DodgeData.Duration = 0.5f;
		DodgeData.StaminaCost = 20.0f;
		break;
	}

	return DodgeData;
}

//=============================================================================
// FForge2StaminaSystem Implementation
//=============================================================================

bool FForge2StaminaSystem::ConsumeStamina(
	float& CurrentStamina,
	float Cost,
	const FForge2StaminaConfig& Config,
	bool& bIsExhausted)
{
	// Can't perform action if exhausted
	if (bIsExhausted)
	{
		return false;
	}

	// Check if we have enough stamina
	if (CurrentStamina < Cost)
	{
		return false;
	}

	// Consume stamina
	CurrentStamina = FMath::Max(0.0f, CurrentStamina - Cost);

	// Check exhaustion
	if (CurrentStamina <= Config.ExhaustedThreshold)
	{
		bIsExhausted = true;
	}

	return true;
}

void FForge2StaminaSystem::RegenerateStamina(
	float& CurrentStamina,
	float DeltaTime,
	float& RegenDelayRemaining,
	const FForge2StaminaConfig& Config,
	bool bIsExhausted,
	bool bIsSprinting,
	bool bIsBlocking)
{
	// Update regen delay
	if (RegenDelayRemaining > 0.0f)
	{
		RegenDelayRemaining -= DeltaTime;
		return;
	}

	// Check regen blockers
	if (bIsSprinting && Config.bBlockRegenWhileSprinting)
	{
		return;
	}
	if (bIsBlocking && Config.bBlockRegenWhileBlocking)
	{
		return;
	}

	// Calculate regen rate
	float RegenRate = Config.RegenRate;
	if (bIsExhausted)
	{
		RegenRate *= Config.ExhaustedRegenMultiplier;
	}

	// Regenerate
	CurrentStamina = FMath::Min(Config.MaxStamina, CurrentStamina + RegenRate * DeltaTime);
}

bool FForge2StaminaSystem::CheckExhaustionRecovery(
	float CurrentStamina,
	const FForge2StaminaConfig& Config,
	bool& bIsExhausted)
{
	if (!bIsExhausted)
	{
		return true;
	}

	if (CurrentStamina >= Config.ExhaustedRecoveryThreshold)
	{
		bIsExhausted = false;
		return true;
	}

	return false;
}

//=============================================================================
// FForge2TargetingSystem Implementation
//=============================================================================

TArray<FForge2TargetInfo> FForge2TargetingSystem::FindTargetsInRange(
	AActor* Seeker,
	const FForge2TargetingConfig& Config,
	const TArray<AActor*>& PotentialTargets)
{
	TArray<FForge2TargetInfo> Results;
	if (!Seeker)
	{
		return Results;
	}

	FVector SeekerLocation = Seeker->GetActorLocation();
	FVector SeekerForward = Seeker->GetActorForwardVector();

	for (AActor* Target : PotentialTargets)
	{
		if (!Target || Target == Seeker)
		{
			continue;
		}

		FVector ToTarget = Target->GetActorLocation() - SeekerLocation;
		float Distance = ToTarget.Size();
		ToTarget.Normalize();

		float Angle = FMath::Acos(FVector::DotProduct(ToTarget, SeekerForward)) * (180.0f / PI);

		// Check if within lock-on range and angle
		if (Distance <= Config.LockOnRange && Angle <= Config.LockOnAngle)
		{
			FForge2TargetInfo TargetInfo;
			TargetInfo.TargetActor = Target;
			TargetInfo.TargetLocation = Target->GetActorLocation();
			TargetInfo.Distance = Distance;
			TargetInfo.Angle = Angle;
			
			// Perform line of sight check
			TargetInfo.bIsVisible = false;
			if (UWorld* World = Seeker->GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(Seeker);
				QueryParams.AddIgnoredActor(Target);
				
				FVector SeekerEyeLocation = Seeker->GetActorLocation();
				if (APawn* SeekerPawn = Cast<APawn>(Seeker))
				{
					SeekerEyeLocation = SeekerPawn->GetPawnViewLocation();
				}
				
				bool bHit = World->LineTraceSingleByChannel(
					HitResult,
					SeekerEyeLocation,
					Target->GetActorLocation(),
					ECC_Visibility,
					QueryParams
				);
				
				// Visible if no blocking hit, or if we hit the target itself
				TargetInfo.bIsVisible = !bHit || HitResult.GetActor() == Target;
			}

			Results.Add(TargetInfo);
		}
	}

	return Results;
}

FForge2TargetInfo FForge2TargetingSystem::SelectBestTarget(
	AActor* Seeker,
	const TArray<FForge2TargetInfo>& Targets,
	const FForge2TargetingConfig& Config)
{
	FForge2TargetInfo BestTarget;
	if (Targets.Num() == 0)
	{
		return BestTarget;
	}

	float BestScore = -FLT_MAX;

	for (const FForge2TargetInfo& Target : Targets)
	{
		float Score = 0.0f;

		switch (Config.TargetPriority)
		{
		case EForge2TargetPriority::Closest:
			Score = -Target.Distance;
			break;

		case EForge2TargetPriority::LowestHealth:
			Score = -Target.HealthPercent;
			break;

		case EForge2TargetPriority::HighestThreat:
			Score = Target.ThreatLevel;
			break;

		case EForge2TargetPriority::InCrosshair:
			Score = -Target.Angle;
			break;

		default:
			Score = -Target.Distance;
			break;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

bool FForge2TargetingSystem::UpdateLockOn(
	AActor* Seeker,
	AActor* CurrentTarget,
	const FForge2TargetingConfig& Config,
	float DeltaTime,
	float& ObstructionTime)
{
	if (!Seeker || !CurrentTarget)
	{
		return false;
	}

	// Check distance
	float Distance = FVector::Dist(Seeker->GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > Config.LockOnRange * 1.2f)  // Allow some margin
	{
		return false;
	}

	// Check line of sight
	UWorld* World = Seeker->GetWorld();
	if (World && Config.bBreakLockOnObstruction)
	{
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Seeker);
		QueryParams.AddIgnoredActor(CurrentTarget);

		bool bObstructed = World->LineTraceSingleByChannel(
			HitResult,
			Seeker->GetActorLocation(),
			CurrentTarget->GetActorLocation(),
			ECC_Visibility,
			QueryParams
		);

		if (bObstructed)
		{
			ObstructionTime += DeltaTime;
			if (ObstructionTime >= Config.ObstructionBreakDelay)
			{
				return false;
			}
		}
		else
		{
			ObstructionTime = 0.0f;
		}
	}

	return true;
}

FForge2TargetInfo FForge2TargetingSystem::SwitchTarget(
	AActor* Seeker,
	AActor* CurrentTarget,
	const TArray<FForge2TargetInfo>& Targets,
	const FVector& SwitchDirection)
{
	if (!Seeker || Targets.Num() == 0)
	{
		return FForge2TargetInfo();
	}

	FVector SeekerLocation = Seeker->GetActorLocation();
	FVector CurrentTargetDir = FVector::ZeroVector;
	if (CurrentTarget)
	{
		CurrentTargetDir = (CurrentTarget->GetActorLocation() - SeekerLocation).GetSafeNormal();
	}

	FForge2TargetInfo BestTarget;
	float BestScore = -FLT_MAX;

	for (const FForge2TargetInfo& Target : Targets)
	{
		if (Target.TargetActor == CurrentTarget)
		{
			continue;
		}

		FVector ToTarget = (Target.TargetLocation - SeekerLocation).GetSafeNormal();
		float DirectionScore = FVector::DotProduct(ToTarget, SwitchDirection);

		// Prefer targets in the switch direction
		if (DirectionScore > 0.0f && DirectionScore > BestScore)
		{
			BestScore = DirectionScore;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

FVector FForge2TargetingSystem::ApplyAutoAim(
	const FVector& AimDirection,
	const FForge2TargetInfo& Target,
	const FForge2TargetingConfig& Config)
{
	if (!Target.TargetActor || !Config.bEnableAutoAim)
	{
		return AimDirection;
	}

	if (Target.Distance > Config.AutoAimRange || Target.Angle > Config.AutoAimAngle)
	{
		return AimDirection;
	}

	FVector ToTarget = Target.TargetLocation - Target.TargetActor->GetActorLocation();
	ToTarget.Normalize();

	// Blend aim direction towards target
	FVector BlendedDirection = FMath::Lerp(AimDirection, ToTarget, Config.AutoAimStrength);
	BlendedDirection.Normalize();

	return BlendedDirection;
}

//=============================================================================
// FForge2HitFeedback Implementation
//=============================================================================

void FForge2HitFeedback::ApplyHitFeedback(
	AActor* Attacker,
	AActor* Victim,
	const FForge2HitResult& HitResult,
	const FForge2HitFeedbackConfig& Config)
{
	if (!Attacker)
	{
		return;
	}

	// Get player controller
	APlayerController* Controller = nullptr;
	if (APawn* Pawn = Cast<APawn>(Attacker))
	{
		Controller = Cast<APlayerController>(Pawn->GetController());
	}

	if (Controller)
	{
		ApplyCameraShake(Controller, Config, HitResult.bWasCritical);
		ApplyControllerVibration(Controller, Config, HitResult.bWasCritical);
	}

	if (Attacker->GetWorld())
	{
		ApplyHitStop(Attacker->GetWorld(), Config, HitResult.bWasCritical);
	}
}

void FForge2HitFeedback::ApplyCameraShake(
	APlayerController* Controller,
	const FForge2HitFeedbackConfig& Config,
	bool bWasCritical)
{
	if (!Controller || !Config.bEnableCameraShake)
	{
		return;
	}

	float Intensity = Config.CameraShakeIntensity;
	if (bWasCritical)
	{
		Intensity *= 1.5f;
	}

	// Would call ClientStartCameraShake with a camera shake class
	// Controller->ClientStartCameraShake(CameraShakeClass, Intensity);
}

void FForge2HitFeedback::ApplyHitStop(
	UWorld* World,
	const FForge2HitFeedbackConfig& Config,
	bool bWasCritical)
{
	if (!World || !Config.bEnableHitStop)
	{
		return;
	}

	float Duration = Config.HitStopDuration;
	if (bWasCritical)
	{
		Duration *= 1.5f;
	}

	// Apply time dilation to the game
	UGameplayStatics::SetGlobalTimeDilation(World, Config.HitStopTimeDilation);
	
	// Set up timer to restore normal time
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([World]()
	{
		if (World && World->IsValidLowLevel())
		{
			UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
		}
	});
	
	World->GetTimerManager().SetTimer(
		TimerHandle,
		TimerDelegate,
		Duration * Config.HitStopTimeDilation, // Adjusted for time dilation
		false
	);
}

void FForge2HitFeedback::ApplyControllerVibration(
	APlayerController* Controller,
	const FForge2HitFeedbackConfig& Config,
	bool bWasCritical)
{
	if (!Controller || !Config.bEnableControllerVibration)
	{
		return;
	}

	float Intensity = Config.VibrationIntensity;
	float Duration = Config.VibrationDuration;
	if (bWasCritical)
	{
		Intensity *= 1.5f;
		Duration *= 1.25f;
	}

	Controller->PlayDynamicForceFeedback(
		Intensity,
		Duration,
		true, true,  // Left and right large motors
		true, true,  // Left and right small motors
		EDynamicForceFeedbackAction::Start
	);
}

//=============================================================================
// FForge2CombatUtils Implementation
//=============================================================================

float FForge2CombatUtils::CalculateDamage(
	float BaseDamage,
	float ArmorValue,
	float Resistance,
	float ArmorPenetration,
	bool bIgnoreArmor)
{
	float FinalDamage = BaseDamage;

	if (!bIgnoreArmor && ArmorValue > 0.0f)
	{
		float EffectiveArmor = FMath::Max(0.0f, ArmorValue - ArmorPenetration);
		float DamageReduction = EffectiveArmor / (EffectiveArmor + 100.0f);
		FinalDamage *= (1.0f - DamageReduction);
	}

	if (Resistance != 0.0f)
	{
		FinalDamage *= (1.0f - FMath::Clamp(Resistance, -1.0f, 0.9f));
	}

	return FMath::Max(0.0f, FinalDamage);
}

bool FForge2CombatUtils::RollCritical(float CritChance, int32 Seed)
{
	if (CritChance <= 0.0f)
	{
		return false;
	}

	FRandomStream RandomStream(Seed >= 0 ? Seed : FMath::Rand());
	return RandomStream.FRand() < CritChance;
}

EForge2HitReactionType FForge2CombatUtils::DetermineHitReaction(
	float Damage,
	float Poise,
	EForge2HitReactionType BaseReaction)
{
	// High poise can reduce reaction severity
	if (Poise > 0.0f && Damage < Poise)
	{
		// Reduce reaction severity
		switch (BaseReaction)
		{
		case EForge2HitReactionType::Knockdown:
			return EForge2HitReactionType::Stagger;
		case EForge2HitReactionType::Knockback:
			return EForge2HitReactionType::Stagger;
		case EForge2HitReactionType::Stagger:
			return EForge2HitReactionType::Flinch;
		case EForge2HitReactionType::Flinch:
			return EForge2HitReactionType::None;
		default:
			return BaseReaction;
		}
	}

	return BaseReaction;
}

FVector FForge2CombatUtils::CalculateKnockback(
	const FVector& AttackerLocation,
	const FVector& VictimLocation,
	float KnockbackForce,
	const FVector& CustomDirection)
{
	if (KnockbackForce <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	FVector Direction = CustomDirection;
	if (Direction.IsNearlyZero())
	{
		Direction = (VictimLocation - AttackerLocation).GetSafeNormal();
		// Add some upward component
		Direction.Z = FMath::Max(0.2f, Direction.Z);
		Direction.Normalize();
	}

	return Direction * KnockbackForce;
}

const FForge2MeleeAttackData* FForge2CombatUtils::FindMeleeAttack(
	const TArray<FForge2MeleeAttackData>& Attacks,
	const FString& AttackId)
{
	for (const FForge2MeleeAttackData& Attack : Attacks)
	{
		if (Attack.AttackId == AttackId)
		{
			return &Attack;
		}
	}
	return nullptr;
}

const FForge2RangedAttackData* FForge2CombatUtils::FindRangedAttack(
	const TArray<FForge2RangedAttackData>& Attacks,
	const FString& AttackId)
{
	for (const FForge2RangedAttackData& Attack : Attacks)
	{
		if (Attack.AttackId == AttackId)
		{
			return &Attack;
		}
	}
	return nullptr;
}

void FForge2CombatUtils::DrawHitboxDebug(
	UWorld* World,
	const FForge2HitboxData& Hitbox,
	const FTransform& Transform,
	float Duration)
{
#if WITH_EDITOR
	if (!World)
	{
		return;
	}

	FVector Location = Transform.TransformPosition(Hitbox.Offset);
	FQuat Rotation = Transform.GetRotation() * FQuat(Hitbox.Rotation);

	FColor Color = Hitbox.bIsWeakPoint ? FColor::Yellow : FColor::Red;

	switch (Hitbox.Shape)
	{
	case EForge2HitboxShape::Sphere:
		DrawDebugSphere(World, Location, Hitbox.Extent.X, 16, Color, false, Duration);
		break;

	case EForge2HitboxShape::Box:
		DrawDebugBox(World, Location, Hitbox.Extent, Rotation, Color, false, Duration);
		break;

	case EForge2HitboxShape::Capsule:
		DrawDebugCapsule(World, Location, Hitbox.CapsuleHalfHeight, Hitbox.CapsuleRadius,
			Rotation, Color, false, Duration);
		break;

	case EForge2HitboxShape::Trace:
		{
			FVector End = Location + Rotation.GetForwardVector() * Hitbox.Extent.X;
			DrawDebugLine(World, Location, End, Color, false, Duration, 0, 2.0f);
		}
		break;

	default:
		break;
	}
#endif
}

float FForge2CombatUtils::FramesToSeconds(float Frames, float FrameRate)
{
	if (FrameRate <= 0.0f) return 0.0f;
	return Frames / FrameRate;
}

float FForge2CombatUtils::SecondsToFrames(float Seconds, float FrameRate)
{
	return Seconds * FrameRate;
}
