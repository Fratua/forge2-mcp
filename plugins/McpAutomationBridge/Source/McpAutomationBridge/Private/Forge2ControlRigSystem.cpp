// Forge2ControlRigSystem.cpp - Control Rig & Advanced Animation System Implementation
#include "Forge2ControlRigSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

// UForge2ControlRigManager Implementation
UForge2ControlRigManager::UForge2ControlRigManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    bFootPlacementEnabled = false;
    bLookAtEnabled = false;
}

void UForge2ControlRigManager::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bFootPlacementEnabled)
    {
        UpdateFootPlacement(DeltaTime);
    }

    if (bLookAtEnabled)
    {
        UpdateLookAt(DeltaTime);
    }

    UpdateProceduralAnimations(DeltaTime);
}

void UForge2ControlRigManager::CreateIKChain(const FForge2IKChainSettings& Settings)
{
    IKChains.Add(Settings.ChainName, Settings);
}

void UForge2ControlRigManager::SetIKTarget(const FName& ChainName, FVector TargetLocation,
                                            FRotator TargetRotation)
{
    if (IKChains.Contains(ChainName))
    {
        SolveIKChain(IKChains[ChainName], TargetLocation);
    }
}

void UForge2ControlRigManager::SetIKBlendWeight(const FName& ChainName, float Weight)
{
    if (IKChains.Contains(ChainName))
    {
        IKChains[ChainName].BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
    }
}

void UForge2ControlRigManager::SetIKPoleVector(const FName& ChainName, FVector PoleVectorLocation)
{
    // Set pole vector for IK chain (used by two-bone IK)
}

void UForge2ControlRigManager::CreateFKControl(const FForge2FKControlSettings& Settings)
{
    FKControls.Add(Settings.ControlName, Settings);
}

void UForge2ControlRigManager::SetFKControlTransform(const FName& ControlName, const FTransform& Transform)
{
    if (FKControls.Contains(ControlName))
    {
        // Apply transform to the target bone
    }
}

void UForge2ControlRigManager::SetFKControlRotation(const FName& ControlName, const FRotator& Rotation)
{
    if (FKControls.Contains(ControlName))
    {
        // Apply rotation to the target bone
    }
}

void UForge2ControlRigManager::SetupFullBodyIK(const FForge2FullBodyIKSettings& Settings)
{
    FullBodyIKSettings = Settings;
}

void UForge2ControlRigManager::SetFullBodyIKEffector(const FName& EffectorBone, FVector TargetLocation,
                                                       float Weight)
{
    // Set FBIK effector target
}

void UForge2ControlRigManager::ConfigureFootPlacement(const FForge2FootPlacementSettings& Settings)
{
    FootPlacementSettings = Settings;
}

void UForge2ControlRigManager::EnableFootPlacement(bool bEnable)
{
    bFootPlacementEnabled = bEnable;
}

void UForge2ControlRigManager::SetFootPlacementBlend(float BlendWeight)
{
    // Set blend weight for foot placement
}

void UForge2ControlRigManager::ConfigureHandIK(const FForge2HandIKSettings& Settings)
{
    HandIKSettings = Settings;
}

void UForge2ControlRigManager::SetLeftHandTarget(AActor* TargetActor, FName SocketName)
{
    if (!TargetActor) return;

    FVector TargetLocation;
    if (SocketName != NAME_None)
    {
        USceneComponent* RootComp = TargetActor->GetRootComponent();
        if (RootComp)
        {
            TargetLocation = RootComp->GetSocketLocation(SocketName);
        }
    }
    else
    {
        TargetLocation = TargetActor->GetActorLocation();
    }

    // Apply IK to left hand
}

void UForge2ControlRigManager::SetRightHandTarget(AActor* TargetActor, FName SocketName)
{
    if (!TargetActor) return;

    FVector TargetLocation;
    if (SocketName != NAME_None)
    {
        USceneComponent* RootComp = TargetActor->GetRootComponent();
        if (RootComp)
        {
            TargetLocation = RootComp->GetSocketLocation(SocketName);
        }
    }
    else
    {
        TargetLocation = TargetActor->GetActorLocation();
    }

    // Apply IK to right hand
}

void UForge2ControlRigManager::ConfigureLookAt(const FForge2LookAtSettings& Settings)
{
    LookAtSettings = Settings;
}

void UForge2ControlRigManager::SetLookAtTarget(FVector TargetLocation)
{
    CurrentLookAtTarget = TargetLocation;
}

void UForge2ControlRigManager::SetLookAtTargetActor(AActor* TargetActor)
{
    if (TargetActor)
    {
        CurrentLookAtTarget = TargetActor->GetActorLocation() + LookAtSettings.LookAtOffset;
    }
}

void UForge2ControlRigManager::EnableLookAt(bool bEnable)
{
    bLookAtEnabled = bEnable;
}

void UForge2ControlRigManager::AddProceduralAnimation(const FString& AnimName,
                                                        const FForge2ProceduralAnimSettings& Settings)
{
    ProceduralAnimations.Add(AnimName, Settings);
}

void UForge2ControlRigManager::RemoveProceduralAnimation(const FString& AnimName)
{
    ProceduralAnimations.Remove(AnimName);
}

void UForge2ControlRigManager::SetProceduralAnimBlend(const FString& AnimName, float BlendWeight)
{
    if (ProceduralAnimations.Contains(AnimName))
    {
        ProceduralAnimations[AnimName].BlendWeight = FMath::Clamp(BlendWeight, 0.0f, 1.0f);
    }
}

FForge2ProceduralAnimSettings UForge2ControlRigManager::CreateBreathingAnimation(TArray<FName> SpineBones,
                                                                                   float BreathRate)
{
    FForge2ProceduralAnimSettings Settings;
    Settings.Type = EForge2ProceduralAnimType::Breathing;
    Settings.AffectedBones = SpineBones;
    Settings.Frequency = BreathRate;
    Settings.Amplitude = 2.0f;
    Settings.Direction = FVector(1, 0, 0.2f);  // Chest expansion + slight lift
    return Settings;
}

FForge2ProceduralAnimSettings UForge2ControlRigManager::CreateLeanAnimation(FName PelvisBone,
                                                                              float MaxLeanAngle)
{
    FForge2ProceduralAnimSettings Settings;
    Settings.Type = EForge2ProceduralAnimType::Lean;
    Settings.AffectedBones.Add(PelvisBone);
    Settings.Amplitude = MaxLeanAngle;
    Settings.Frequency = 0.0f;  // Controlled by velocity, not time
    return Settings;
}

void UForge2ControlRigManager::ConfigureRagdoll(const FForge2RagdollSettings& Settings)
{
    RagdollSettings = Settings;
}

void UForge2ControlRigManager::EnableRagdoll(EForge2RagdollBlendType BlendType)
{
    RagdollSettings.BlendType = BlendType;

    if (!TargetMesh)
    {
        ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
        if (OwnerChar)
        {
            TargetMesh = OwnerChar->GetMesh();
        }
    }

    if (TargetMesh)
    {
        switch (BlendType)
        {
            case EForge2RagdollBlendType::Full:
                TargetMesh->SetSimulatePhysics(true);
                TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                break;

            case EForge2RagdollBlendType::Partial:
                // Set specific bodies to simulate
                for (const FName& BoneName : RagdollSettings.SimulatedBones)
                {
                    TargetMesh->SetAllBodiesBelowSimulatePhysics(BoneName, true);
                }
                break;

            case EForge2RagdollBlendType::PhysicsBlend:
                TargetMesh->SetAllBodiesPhysicsBlendWeight(RagdollSettings.PhysicsBlendWeight);
                break;

            default:
                break;
        }
    }
}

void UForge2ControlRigManager::DisableRagdoll()
{
    if (TargetMesh)
    {
        TargetMesh->SetSimulatePhysics(false);
        TargetMesh->SetAllBodiesPhysicsBlendWeight(0.0f);

        if (RagdollSettings.bResetOnBlendOut)
        {
            // Reset pose would happen here
        }
    }

    RagdollSettings.BlendType = EForge2RagdollBlendType::None;
}

void UForge2ControlRigManager::SetRagdollBlendWeight(float Weight)
{
    RagdollSettings.BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

    if (TargetMesh)
    {
        TargetMesh->SetAllBodiesPhysicsBlendWeight(Weight);
    }
}

void UForge2ControlRigManager::ConfigureHitReaction(const FForge2HitReactionSettings& Settings)
{
    HitReactionSettings = Settings;
}

void UForge2ControlRigManager::TriggerHitReaction(FVector ImpactPoint, FVector ImpactDirection,
                                                    float Magnitude)
{
    if (!HitReactionSettings.bEnabled) return;

    if (!TargetMesh)
    {
        ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
        if (OwnerChar)
        {
            TargetMesh = OwnerChar->GetMesh();
        }
    }

    if (TargetMesh)
    {
        // Find closest bone to impact point
        FName ClosestBone = TargetMesh->FindClosestBone(ImpactPoint);

        // Apply impulse
        FVector Impulse = ImpactDirection.GetSafeNormal() * Magnitude * HitReactionSettings.ImpulseMultiplier;

        if (HitReactionSettings.bUsePartialRagdoll)
        {
            // Enable partial ragdoll on hit area
            TargetMesh->SetAllBodiesBelowPhysicsBlendWeight(ClosestBone,
                                                             HitReactionSettings.PartialRagdollBlend);
            TargetMesh->AddImpulseAtLocation(Impulse, ImpactPoint, ClosestBone);
        }
        else
        {
            // Just apply impulse
            TargetMesh->AddImpulseAtLocation(Impulse, ImpactPoint, ClosestBone);
        }
    }
}

void UForge2ControlRigManager::ConfigureMotionMatching(const FForge2MotionMatchingSettings& Settings)
{
    MotionMatchingSettings = Settings;
}

void UForge2ControlRigManager::SetDesiredTrajectory(const TArray<FVector>& TrajectoryPoints,
                                                      const TArray<FVector>& Velocities)
{
    // Store trajectory for motion matching evaluation
    if (TrajectoryPoints.Num() == 0 || TrajectoryPoints.Num() != Velocities.Num())
    {
        return;
    }
    
    // Get the owning character's animation instance
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerChar->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }
    
    // Store trajectory data for motion matching query
    DesiredTrajectoryPoints = TrajectoryPoints;
    DesiredTrajectoryVelocities = Velocities;
    
    // Calculate trajectory features for motion matching
    FVector CurrentVelocity = OwnerChar->GetVelocity();
    FVector CurrentFacing = OwnerChar->GetActorForwardVector();
    
    // Build trajectory query for motion matching
    // This feeds into UE5's motion matching system via animation blueprint
    if (TrajectoryPoints.Num() >= 2)
    {
        // Calculate desired facing from trajectory
        FVector TrajectoryDirection = (TrajectoryPoints[1] - TrajectoryPoints[0]).GetSafeNormal();
        float DesiredYaw = FMath::Atan2(TrajectoryDirection.Y, TrajectoryDirection.X) * (180.0f / PI);
        
        // Store motion matching query parameters as custom properties
        // These can be read by Animation Blueprint via property access
        MotionMatchingQuery.DesiredVelocity = Velocities.Num() > 0 ? Velocities[0] : FVector::ZeroVector;
        MotionMatchingQuery.DesiredFacing = TrajectoryDirection;
        MotionMatchingQuery.DesiredSpeed = MotionMatchingQuery.DesiredVelocity.Size();
        MotionMatchingQuery.TurningAngle = FMath::FindDeltaAngleDegrees(CurrentFacing.Rotation().Yaw, DesiredYaw);
        
        // Store future trajectory samples for pose matching
        MotionMatchingQuery.FuturePositions.Empty();
        MotionMatchingQuery.FutureVelocities.Empty();
        
        for (int32 i = 0; i < FMath::Min(TrajectoryPoints.Num(), MotionMatchingSettings.TrajectoryHistoryCount); i++)
        {
            MotionMatchingQuery.FuturePositions.Add(TrajectoryPoints[i]);
            MotionMatchingQuery.FutureVelocities.Add(Velocities[i]);
        }
    }
}

void UForge2ControlRigManager::UpdateFootPlacement(float DeltaTime)
{
    if (!FootPlacementSettings.bEnabled) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector ActorLocation = Owner->GetActorLocation();
    FVector TraceStart = ActorLocation + FVector(0, 0, FootPlacementSettings.TraceDistance);
    FVector TraceEnd = ActorLocation - FVector(0, 0, FootPlacementSettings.TraceDistance * 2);

    // Perform ground trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        FootPlacementSettings.TraceChannel,
        QueryParams
    );

    if (bHit)
    {
        // Calculate pelvis adjustment
        float GroundDistance = HitResult.Location.Z - ActorLocation.Z;
        // Apply to skeleton...
    }
}

void UForge2ControlRigManager::UpdateLookAt(float DeltaTime)
{
    if (!LookAtSettings.bEnabled) return;

    // Calculate look direction and apply to head/spine bones
}

void UForge2ControlRigManager::UpdateProceduralAnimations(float DeltaTime)
{
    float Time = GetWorld()->GetTimeSeconds();

    for (auto& Pair : ProceduralAnimations)
    {
        const FForge2ProceduralAnimSettings& Settings = Pair.Value;

        if (Settings.BlendWeight <= 0.0f) continue;

        float Phase = Time * Settings.Frequency * 2.0f * PI + Settings.Phase;

        switch (Settings.Type)
        {
            case EForge2ProceduralAnimType::Breathing:
            {
                float BreathValue = FMath::Sin(Phase) * Settings.Amplitude * Settings.BlendWeight;
                // Apply breathing offset to affected bones
                break;
            }

            case EForge2ProceduralAnimType::HeadBob:
            {
                float BobValue = FMath::Abs(FMath::Sin(Phase)) * Settings.Amplitude * Settings.BlendWeight;
                // Apply head bob
                break;
            }

            case EForge2ProceduralAnimType::Sway:
            {
                float SwayValue = FMath::Sin(Phase) * Settings.Amplitude * Settings.BlendWeight;
                // Apply sway rotation
                break;
            }

            default:
                break;
        }
    }
}

void UForge2ControlRigManager::SolveIKChain(const FForge2IKChainSettings& Chain, FVector TargetLocation)
{
    switch (Chain.ChainType)
    {
        case EForge2IKChainType::TwoBone:
            // Two-bone IK solver
            break;

        case EForge2IKChainType::FABRIK:
            // Forward and Backward Reaching IK
            break;

        case EForge2IKChainType::CCDIK:
            // Cyclic Coordinate Descent IK
            break;

        case EForge2IKChainType::Spline:
            // Spline-based IK
            break;

        default:
            break;
    }
}

// UForge2ControlRigUtility Implementation
FForge2IKChainSettings UForge2ControlRigUtility::CreateTwoBoneIKChain(FName ChainName,
                                                                        FName RootBone,
                                                                        FName MiddleBone,
                                                                        FName TipBone)
{
    FForge2IKChainSettings Settings;
    Settings.ChainName = ChainName;
    Settings.ChainType = EForge2IKChainType::TwoBone;
    Settings.RootBone = RootBone;
    Settings.TipBone = TipBone;
    Settings.IntermediateBones.Add(MiddleBone);
    Settings.BlendWeight = 1.0f;
    return Settings;
}

FForge2IKChainSettings UForge2ControlRigUtility::CreateFABRIKChain(FName ChainName, TArray<FName> Bones)
{
    FForge2IKChainSettings Settings;
    Settings.ChainName = ChainName;
    Settings.ChainType = EForge2IKChainType::FABRIK;

    if (Bones.Num() >= 2)
    {
        Settings.RootBone = Bones[0];
        Settings.TipBone = Bones.Last();

        for (int32 i = 1; i < Bones.Num() - 1; i++)
        {
            Settings.IntermediateBones.Add(Bones[i]);
        }
    }

    Settings.MaxIterations = 10;
    Settings.Precision = 0.01f;
    Settings.BlendWeight = 1.0f;
    return Settings;
}

FForge2FootPlacementSettings UForge2ControlRigUtility::CreateDefaultFootPlacement()
{
    FForge2FootPlacementSettings Settings;
    Settings.bEnabled = true;
    Settings.LeftFootBone = TEXT("foot_l");
    Settings.RightFootBone = TEXT("foot_r");
    Settings.LeftToeBone = TEXT("ball_l");
    Settings.RightToeBone = TEXT("ball_r");
    Settings.PelvisBone = TEXT("pelvis");
    Settings.TraceDistance = 50.0f;
    Settings.FootOffset = 5.0f;
    Settings.PelvisAdjustmentSpeed = 10.0f;
    Settings.FootAdjustmentSpeed = 15.0f;
    Settings.MaxSlopeAngle = 45.0f;
    Settings.bRotateFeetToSlope = true;
    return Settings;
}

FForge2LookAtSettings UForge2ControlRigUtility::CreateDefaultLookAt()
{
    FForge2LookAtSettings Settings;
    Settings.bEnabled = true;
    Settings.HeadBone = TEXT("head");
    Settings.SpineBones.Add(TEXT("spine_03"));
    Settings.SpineBones.Add(TEXT("spine_02"));
    Settings.HeadWeight = 1.0f;
    Settings.SpineWeight = 0.3f;
    Settings.InterpSpeed = 8.0f;
    Settings.MaxPitchAngle = 60.0f;
    Settings.MaxYawAngle = 90.0f;
    Settings.bClampAngles = true;
    return Settings;
}

FForge2RagdollSettings UForge2ControlRigUtility::CreateFullRagdoll()
{
    FForge2RagdollSettings Settings;
    Settings.BlendType = EForge2RagdollBlendType::Full;
    Settings.BlendWeight = 1.0f;
    Settings.BlendInTime = 0.1f;
    Settings.BlendOutTime = 0.5f;
    Settings.bResetOnBlendOut = true;
    return Settings;
}

FForge2RagdollSettings UForge2ControlRigUtility::CreateUpperBodyRagdoll()
{
    FForge2RagdollSettings Settings;
    Settings.BlendType = EForge2RagdollBlendType::Partial;
    Settings.BlendWeight = 1.0f;
    Settings.BlendInTime = 0.1f;
    Settings.BlendOutTime = 0.3f;
    Settings.bResetOnBlendOut = true;

    Settings.SimulatedBones.Add(TEXT("spine_03"));
    Settings.SimulatedBones.Add(TEXT("clavicle_l"));
    Settings.SimulatedBones.Add(TEXT("clavicle_r"));
    Settings.SimulatedBones.Add(TEXT("head"));

    Settings.KinematicBones.Add(TEXT("pelvis"));
    Settings.KinematicBones.Add(TEXT("thigh_l"));
    Settings.KinematicBones.Add(TEXT("thigh_r"));

    return Settings;
}

FForge2HitReactionSettings UForge2ControlRigUtility::CreateDefaultHitReaction()
{
    FForge2HitReactionSettings Settings;
    Settings.bEnabled = true;
    Settings.ImpulseMultiplier = 1.0f;
    Settings.BlendDuration = 0.3f;
    Settings.RecoveryTime = 0.5f;
    Settings.bUsePartialRagdoll = true;
    Settings.PartialRagdollBlend = 0.5f;
    return Settings;
}
