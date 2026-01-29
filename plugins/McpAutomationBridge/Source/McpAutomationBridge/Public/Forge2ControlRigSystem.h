// Forge2ControlRigSystem.h - Control Rig & Advanced Animation System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2ControlRigSystem.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class UAnimInstance;
class UControlRig;

// Enums
UENUM(BlueprintType)
enum class EForge2IKChainType : uint8
{
    TwoBone,
    FABRIK,
    CCDIK,
    Spline,
    LimbIK,
    FullBodyIK
};

UENUM(BlueprintType)
enum class EForge2ControlType : uint8
{
    Transform,
    Rotation,
    Translation,
    Scale,
    Float,
    Bool,
    Vector
};

UENUM(BlueprintType)
enum class EForge2BlendMode : uint8
{
    Override,
    Additive,
    Multiply,
    Interpolate
};

UENUM(BlueprintType)
enum class EForge2ProceduralAnimType : uint8
{
    Breathing,
    LookAt,
    Lean,
    HeadBob,
    FootPlacement,
    Sway,
    Recoil,
    Custom
};

UENUM(BlueprintType)
enum class EForge2RagdollBlendType : uint8
{
    None,
    Partial,
    Full,
    PhysicsBlend
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2IKChainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ChainName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2IKChainType ChainType = EForge2IKChainType::TwoBone;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RootBone;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TipBone;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> IntermediateBones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName PoleVectorBone;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PoleVectorOffset = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ChainLength = 0.0f;  // 0 = auto-calculate
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxIterations = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Precision = 0.01f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FKControlSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ControlName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TargetBone;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2ControlType ControlType = EForge2ControlType::Transform;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform InitialTransform;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform MinConstraints;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform MaxConstraints;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bApplyConstraints = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BlendMode BlendMode = EForge2BlendMode::Override;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FullBodyIKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RootBone = TEXT("pelvis");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> EffectorBones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SolverIterations = 20;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RootMotionWeight = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float StiffnessMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableCollision = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CollisionRadius = 10.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FootPlacementSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName LeftFootBone = TEXT("foot_l");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RightFootBone = TEXT("foot_r");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName LeftToeBone = TEXT("ball_l");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RightToeBone = TEXT("ball_r");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName PelvisBone = TEXT("pelvis");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TraceDistance = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FootOffset = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PelvisAdjustmentSpeed = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FootAdjustmentSpeed = 15.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxSlopeAngle = 45.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRotateFeetToSlope = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HandIKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName LeftHandBone = TEXT("hand_l");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RightHandBone = TEXT("hand_r");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector LeftHandTargetOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector RightHandTargetOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendSpeed = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxReachDistance = 100.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LookAtSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName HeadBone = TEXT("head");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> SpineBones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeadWeight = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpineWeight = 0.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float InterpSpeed = 8.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxPitchAngle = 60.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxYawAngle = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bClampAngles = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector LookAtOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ProceduralAnimSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2ProceduralAnimType Type = EForge2ProceduralAnimType::Breathing;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> AffectedBones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Frequency = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Amplitude = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Direction = FVector(0, 0, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Phase = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2RagdollBlendType BlendType = EForge2RagdollBlendType::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> SimulatedBones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> KinematicBones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendWeight = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendInTime = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendOutTime = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PhysicsBlendWeight = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bResetOnBlendOut = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HitReactionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ImpulseMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendDuration = 0.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RecoveryTime = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUsePartialRagdoll = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PartialRagdollBlend = 0.5f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MotionMatchingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TrajectoryPredictionTime = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TrajectoryPoints = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PoseMatchWeight = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TrajectoryMatchWeight = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VelocityMatchWeight = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SearchThreshold = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendTime = 0.2f;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2ControlRigManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2ControlRigManager();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction) override;

    // IK Chain Management
    UFUNCTION(BlueprintCallable, Category = "ControlRig|IK")
    void CreateIKChain(const FForge2IKChainSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|IK")
    void SetIKTarget(const FName& ChainName, FVector TargetLocation, FRotator TargetRotation);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|IK")
    void SetIKBlendWeight(const FName& ChainName, float Weight);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|IK")
    void SetIKPoleVector(const FName& ChainName, FVector PoleVectorLocation);

    // FK Control Management
    UFUNCTION(BlueprintCallable, Category = "ControlRig|FK")
    void CreateFKControl(const FForge2FKControlSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|FK")
    void SetFKControlTransform(const FName& ControlName, const FTransform& Transform);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|FK")
    void SetFKControlRotation(const FName& ControlName, const FRotator& Rotation);

    // Full Body IK
    UFUNCTION(BlueprintCallable, Category = "ControlRig|FBIK")
    void SetupFullBodyIK(const FForge2FullBodyIKSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|FBIK")
    void SetFullBodyIKEffector(const FName& EffectorBone, FVector TargetLocation, float Weight);

    // Foot Placement
    UFUNCTION(BlueprintCallable, Category = "ControlRig|Feet")
    void ConfigureFootPlacement(const FForge2FootPlacementSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Feet")
    void EnableFootPlacement(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Feet")
    void SetFootPlacementBlend(float BlendWeight);

    // Hand IK
    UFUNCTION(BlueprintCallable, Category = "ControlRig|Hands")
    void ConfigureHandIK(const FForge2HandIKSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Hands")
    void SetLeftHandTarget(AActor* TargetActor, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Hands")
    void SetRightHandTarget(AActor* TargetActor, FName SocketName = NAME_None);

    // Look At
    UFUNCTION(BlueprintCallable, Category = "ControlRig|LookAt")
    void ConfigureLookAt(const FForge2LookAtSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|LookAt")
    void SetLookAtTarget(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|LookAt")
    void SetLookAtTargetActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|LookAt")
    void EnableLookAt(bool bEnable);

    // Procedural Animation
    UFUNCTION(BlueprintCallable, Category = "ControlRig|Procedural")
    void AddProceduralAnimation(const FString& AnimName, const FForge2ProceduralAnimSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Procedural")
    void RemoveProceduralAnimation(const FString& AnimName);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Procedural")
    void SetProceduralAnimBlend(const FString& AnimName, float BlendWeight);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Procedural")
    FForge2ProceduralAnimSettings CreateBreathingAnimation(TArray<FName> SpineBones, float BreathRate);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Procedural")
    FForge2ProceduralAnimSettings CreateLeanAnimation(FName PelvisBone, float MaxLeanAngle);

    // Ragdoll
    UFUNCTION(BlueprintCallable, Category = "ControlRig|Ragdoll")
    void ConfigureRagdoll(const FForge2RagdollSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Ragdoll")
    void EnableRagdoll(EForge2RagdollBlendType BlendType);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Ragdoll")
    void DisableRagdoll();

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Ragdoll")
    void SetRagdollBlendWeight(float Weight);

    // Hit Reactions
    UFUNCTION(BlueprintCallable, Category = "ControlRig|HitReaction")
    void ConfigureHitReaction(const FForge2HitReactionSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|HitReaction")
    void TriggerHitReaction(FVector ImpactPoint, FVector ImpactDirection, float Magnitude);

    // Motion Matching
    UFUNCTION(BlueprintCallable, Category = "ControlRig|MotionMatching")
    void ConfigureMotionMatching(const FForge2MotionMatchingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|MotionMatching")
    void SetDesiredTrajectory(const TArray<FVector>& TrajectoryPoints, const TArray<FVector>& Velocities);

private:
    UPROPERTY() USkeletalMeshComponent* TargetMesh;
    UPROPERTY() TMap<FName, FForge2IKChainSettings> IKChains;
    UPROPERTY() TMap<FName, FForge2FKControlSettings> FKControls;
    UPROPERTY() FForge2FullBodyIKSettings FullBodyIKSettings;
    UPROPERTY() FForge2FootPlacementSettings FootPlacementSettings;
    UPROPERTY() FForge2HandIKSettings HandIKSettings;
    UPROPERTY() FForge2LookAtSettings LookAtSettings;
    UPROPERTY() TMap<FString, FForge2ProceduralAnimSettings> ProceduralAnimations;
    UPROPERTY() FForge2RagdollSettings RagdollSettings;
    UPROPERTY() FForge2HitReactionSettings HitReactionSettings;
    UPROPERTY() FForge2MotionMatchingSettings MotionMatchingSettings;

    FVector CurrentLookAtTarget;
    bool bFootPlacementEnabled = false;
    bool bLookAtEnabled = false;

    void UpdateFootPlacement(float DeltaTime);
    void UpdateLookAt(float DeltaTime);
    void UpdateProceduralAnimations(float DeltaTime);
    void SolveIKChain(const FForge2IKChainSettings& Chain, FVector TargetLocation);
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2ControlRigUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "ControlRig|Utility")
    static FForge2IKChainSettings CreateTwoBoneIKChain(FName ChainName, FName RootBone,
                                                         FName MiddleBone, FName TipBone);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Utility")
    static FForge2IKChainSettings CreateFABRIKChain(FName ChainName, TArray<FName> Bones);

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Utility")
    static FForge2FootPlacementSettings CreateDefaultFootPlacement();

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Utility")
    static FForge2LookAtSettings CreateDefaultLookAt();

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Utility")
    static FForge2RagdollSettings CreateFullRagdoll();

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Utility")
    static FForge2RagdollSettings CreateUpperBodyRagdoll();

    UFUNCTION(BlueprintCallable, Category = "ControlRig|Utility")
    static FForge2HitReactionSettings CreateDefaultHitReaction();
};
