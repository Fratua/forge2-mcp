// Forge2CameraSystem.h
// FORGE2 Camera System
// Comprehensive camera management with modes, collision, shake, lag, and transitions

#pragma once

#include "CoreMinimal.h"
#include "Forge2CameraSystem.generated.h"

// Forward declarations
class UCameraComponent;
class USpringArmComponent;
class UCameraShakeBase;
class UCurveFloat;
class UCurveVector;

//=============================================================================
// ENUMS
//=============================================================================

UENUM(BlueprintType)
enum class EForge2CameraMode : uint8
{
	ThirdPerson UMETA(DisplayName = "Third Person"),
	FirstPerson UMETA(DisplayName = "First Person"),
	Isometric UMETA(DisplayName = "Isometric"),
	TopDown UMETA(DisplayName = "Top Down"),
	SideScroller UMETA(DisplayName = "Side Scroller"),
	Orbit UMETA(DisplayName = "Orbit"),
	OverShoulder UMETA(DisplayName = "Over Shoulder"),
	TwinStick UMETA(DisplayName = "Twin Stick"),
	Cinematic UMETA(DisplayName = "Cinematic"),
	FreeCam UMETA(DisplayName = "Free Camera"),
	DeathCam UMETA(DisplayName = "Death Camera"),
	SpectatorFirstPerson UMETA(DisplayName = "Spectator First Person"),
	SpectatorFreeCam UMETA(DisplayName = "Spectator Free Camera")
};

UENUM(BlueprintType)
enum class EForge2CameraCollisionMode : uint8
{
	None UMETA(DisplayName = "None"),
	Push UMETA(DisplayName = "Push (Move Closer)"),
	Cut UMETA(DisplayName = "Cut (Instant)"),
	Fade UMETA(DisplayName = "Fade Objects"),
	Transparent UMETA(DisplayName = "Make Transparent"),
	Clip UMETA(DisplayName = "Clip Through")
};

UENUM(BlueprintType)
enum class EForge2CameraShakeType : uint8
{
	Perlin UMETA(DisplayName = "Perlin Noise"),
	Oscillation UMETA(DisplayName = "Oscillation"),
	Wave UMETA(DisplayName = "Wave"),
	Sequence UMETA(DisplayName = "Sequence/Anim"),
	Random UMETA(DisplayName = "Random"),
	Impact UMETA(DisplayName = "Impact")
};

UENUM(BlueprintType)
enum class EForge2CameraTransitionType : uint8
{
	Cut UMETA(DisplayName = "Cut"),
	Blend UMETA(DisplayName = "Blend"),
	EaseIn UMETA(DisplayName = "Ease In"),
	EaseOut UMETA(DisplayName = "Ease Out"),
	EaseInOut UMETA(DisplayName = "Ease In/Out"),
	Custom UMETA(DisplayName = "Custom Curve")
};

UENUM(BlueprintType)
enum class EForge2SplitScreenMode : uint8
{
	None UMETA(DisplayName = "None"),
	Horizontal UMETA(DisplayName = "Horizontal (Top/Bottom)"),
	Vertical UMETA(DisplayName = "Vertical (Left/Right)"),
	Quadrant UMETA(DisplayName = "Quadrant (4 Players)"),
	Dynamic UMETA(DisplayName = "Dynamic (Merge When Close)")
};

//=============================================================================
// DATA STRUCTURES - Camera Configuration
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ThirdPersonCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetArmLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetOffset = FVector(0.0f, 0.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsePawnControlRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMin = -80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMax = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInheritPitch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInheritYaw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInheritRoll = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FirstPersonCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachSocketName = "head";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocationOffset = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMin = -89.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMax = 89.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideOwnerMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowArms = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2IsometricCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Angle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator(-45.0f, 45.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOrthographic = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OrthoWidth = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFixedRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowZoom = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinZoom = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxZoom = 2.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SideScrollerCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D ViewportOffset = FVector2D(0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLockXAxis = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLockYAxis = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZoneX = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZoneY = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookAheadDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSnapToPixelGrid = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2OrbitCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoomSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMin = -89.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMax = 89.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoRotate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoRotateSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector FocusOffset = FVector::ZeroVector;
};

//=============================================================================
// DATA STRUCTURES - Camera Collision
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CameraCollisionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2CameraCollisionMode CollisionMode = EForge2CameraCollisionMode::Push;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProbeSize = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> ProbeChannel = ECC_Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PushSpeed = 10.0f;  // How fast camera moves when colliding

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReturnSpeed = 5.0f;  // How fast camera returns after collision

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinimumDistance = 50.0f;  // Don't push closer than this

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransparentFadeDistance = 100.0f;  // Distance to start fading objects

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransparentMinOpacity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> IgnoredActors;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CameraCollisionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bIsColliding = false;

	UPROPERTY(BlueprintReadWrite)
	float CollisionDistance = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector CollisionPoint = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	AActor* BlockingActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> TransparentActors;
};

//=============================================================================
// DATA STRUCTURES - Camera Shake
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CameraShakeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShakeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2CameraShakeType ShakeType = EForge2CameraShakeType::Perlin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendInTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendOutTime = 0.2f;

	// Amplitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocationAmplitude = FVector(5.0f, 5.0f, 5.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationAmplitude = FRotator(2.0f, 2.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOVAmplitude = 0.0f;

	// Frequency
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocationFrequency = FVector(10.0f, 10.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationFrequency = FRotator(8.0f, 8.0f, 8.0f);

	// Falloff
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAttenuateWithDistance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InnerRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OuterRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Falloff = 1.0f;

	// Custom curve
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UCurveFloat> IntensityCurve;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CameraShakeInstance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ShakeId;

	UPROPERTY(BlueprintReadWrite)
	float ElapsedTime = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float Scale = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector Epicenter = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	bool bInfinite = false;

	// Current state
	UPROPERTY(BlueprintReadWrite)
	FVector CurrentLocationOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator CurrentRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite)
	float CurrentFOVOffset = 0.0f;
};

//=============================================================================
// DATA STRUCTURES - Camera Lag
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CameraLagConfig
{
	GENERATED_BODY()

	// Position lag
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnablePositionLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositionLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPositionLagDistance = 200.0f;

	// Rotation lag
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableRotationLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRotationLagAngle = 30.0f;

	// Zoom lag
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableZoomLag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoomLagSpeed = 5.0f;

	// Use separate horizontal/vertical lag
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSeparateAxes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PositionLagSpeedPerAxis = FVector(10.0f, 10.0f, 10.0f);

	// Lag in local vs world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLagInLocalSpace = false;
};

//=============================================================================
// DATA STRUCTURES - Camera Constraints
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CameraConstraints
{
	GENERATED_BODY()

	// Pitch constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConstrainPitch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMin = -89.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMax = 89.0f;

	// Yaw constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConstrainYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float YawMin = -180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float YawMax = 180.0f;

	// Distance constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConstrainDistance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 1000.0f;

	// World bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConstrainToVolume = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBox WorldBounds = FBox(FVector(-10000.0f), FVector(10000.0f));
};

//=============================================================================
// DATA STRUCTURES - Camera Transition
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CameraTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2CameraTransitionType TransitionType = EForge2CameraTransitionType::Blend;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UCurveFloat> CustomCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLockOutgoing = false;  // Don't update source during transition

	UPROPERTY(BlueprintReadWrite)
	float CurrentBlend = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator StartRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite)
	float StartFOV = 90.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite)
	float TargetFOV = 90.0f;
};

//=============================================================================
// DATA STRUCTURES - Split Screen
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SplitScreenConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2SplitScreenMode Mode = EForge2SplitScreenMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DynamicMergeDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DynamicSplitDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BorderWidth = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BorderColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowPlayerIndicators = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SplitScreenViewport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 PlayerIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	FVector2D Origin = FVector2D::ZeroVector;  // Top-left corner (0-1)

	UPROPERTY(BlueprintReadWrite)
	FVector2D Size = FVector2D(1.0f, 1.0f);  // Width and height (0-1)

	UPROPERTY(BlueprintReadWrite)
	bool bIsMerged = false;
};

//=============================================================================
// DATA STRUCTURES - Lock-On Camera
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LockOnCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CameraOffset = FVector(-300.0f, 0.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetOffset = FVector(0.0f, 0.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRotateTowardsTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float YawOffset = 0.0f;  // Offset from directly behind player

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPlayerInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InputInfluence = 0.3f;  // How much player input affects camera during lock-on
};

//=============================================================================
// DATA STRUCTURES - Camera Mode Definition
//=============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CameraModeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ModeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EForge2CameraMode ModeType = EForge2CameraMode::ThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2ThirdPersonCameraConfig ThirdPersonConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2FirstPersonCameraConfig FirstPersonConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2IsometricCameraConfig IsometricConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2SideScrollerCameraConfig SideScrollerConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2OrbitCameraConfig OrbitConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2CameraCollisionConfig CollisionConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2CameraLagConfig LagConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2CameraConstraints Constraints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2LockOnCameraConfig LockOnConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FForge2CameraTransition DefaultTransition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

//=============================================================================
// CAMERA SYSTEM CLASSES
//=============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2CameraSystem
{
public:
	// Mode management
	static FForge2CameraModeDefinition CreateDefaultThirdPerson();
	static FForge2CameraModeDefinition CreateDefaultFirstPerson();
	static FForge2CameraModeDefinition CreateDefaultIsometric();
	static FForge2CameraModeDefinition CreateDefaultSideScroller();
	static FForge2CameraModeDefinition CreateDefaultTopDown();
	static FForge2CameraModeDefinition CreateDefaultOrbit();

	// Calculate camera transform for mode
	static void CalculateCameraTransform(
		const FForge2CameraModeDefinition& Mode,
		const FTransform& TargetTransform,
		const FVector2D& Input,
		float DeltaTime,
		FVector& OutLocation,
		FRotator& OutRotation,
		float& OutFOV
	);
};

class MCPAUTOMATIONBRIDGE_API FForge2CameraCollision
{
public:
	// Collision detection
	static FForge2CameraCollisionResult CheckCollision(
		UWorld* World,
		const FVector& TargetLocation,
		const FVector& CameraLocation,
		const FForge2CameraCollisionConfig& Config
	);

	// Apply collision response
	static FVector ApplyCollisionResponse(
		const FVector& DesiredLocation,
		const FVector& TargetLocation,
		const FForge2CameraCollisionResult& CollisionResult,
		const FForge2CameraCollisionConfig& Config,
		float CurrentDistance,
		float DeltaTime
	);

	// Find objects to make transparent
	static TArray<AActor*> FindBlockingActors(
		UWorld* World,
		const FVector& CameraLocation,
		const FVector& TargetLocation,
		const FForge2CameraCollisionConfig& Config
	);
};

class MCPAUTOMATIONBRIDGE_API FForge2CameraShake
{
public:
	// Shake management
	static FForge2CameraShakeInstance StartShake(
		const FForge2CameraShakeConfig& Config,
		float Scale = 1.0f,
		const FVector& Epicenter = FVector::ZeroVector
	);

	// Update shake
	static bool UpdateShake(
		FForge2CameraShakeInstance& Instance,
		const FForge2CameraShakeConfig& Config,
		float DeltaTime,
		const FVector& CameraLocation
	);

	// Calculate shake offset
	static void GetShakeOffset(
		const FForge2CameraShakeInstance& Instance,
		const FForge2CameraShakeConfig& Config,
		FVector& OutLocationOffset,
		FRotator& OutRotationOffset,
		float& OutFOVOffset
	);

	// Create preset shakes
	static FForge2CameraShakeConfig CreateHitShake(float Intensity = 1.0f);
	static FForge2CameraShakeConfig CreateExplosionShake(float Intensity = 1.0f, float Distance = 500.0f);
	static FForge2CameraShakeConfig CreateFootstepShake(float Intensity = 0.3f);
	static FForge2CameraShakeConfig CreateAmbientShake(float Intensity = 0.1f);

private:
	static float SamplePerlinNoise(float Time, float Frequency);
	static float SampleOscillation(float Time, float Frequency);
};

class MCPAUTOMATIONBRIDGE_API FForge2CameraLag
{
public:
	// Apply position lag
	static FVector ApplyPositionLag(
		const FVector& CurrentLocation,
		const FVector& TargetLocation,
		const FForge2CameraLagConfig& Config,
		float DeltaTime
	);

	// Apply rotation lag
	static FRotator ApplyRotationLag(
		const FRotator& CurrentRotation,
		const FRotator& TargetRotation,
		const FForge2CameraLagConfig& Config,
		float DeltaTime
	);

	// Apply zoom lag
	static float ApplyZoomLag(
		float CurrentZoom,
		float TargetZoom,
		const FForge2CameraLagConfig& Config,
		float DeltaTime
	);
};

class MCPAUTOMATIONBRIDGE_API FForge2CameraTransitions
{
public:
	// Start transition
	static FForge2CameraTransition BeginTransition(
		const FVector& FromLocation,
		const FRotator& FromRotation,
		float FromFOV,
		const FVector& ToLocation,
		const FRotator& ToRotation,
		float ToFOV,
		const FForge2CameraTransition& TransitionSettings
	);

	// Update transition
	static bool UpdateTransition(
		FForge2CameraTransition& Transition,
		float DeltaTime,
		FVector& OutLocation,
		FRotator& OutRotation,
		float& OutFOV
	);

	// Get easing value
	static float GetEasedAlpha(float Alpha, EForge2CameraTransitionType Type, UCurveFloat* CustomCurve = nullptr);
};

class MCPAUTOMATIONBRIDGE_API FForge2SplitScreen
{
public:
	// Calculate viewports
	static TArray<FForge2SplitScreenViewport> CalculateViewports(
		int32 PlayerCount,
		const FForge2SplitScreenConfig& Config,
		const TArray<FVector>& PlayerLocations
	);

	// Check if should merge (dynamic mode)
	static bool ShouldMerge(
		const TArray<FVector>& PlayerLocations,
		const FForge2SplitScreenConfig& Config
	);

	// Calculate merged camera position
	static void CalculateMergedCamera(
		const TArray<FVector>& PlayerLocations,
		FVector& OutCameraLocation,
		float& OutZoom
	);
};

class MCPAUTOMATIONBRIDGE_API FForge2LockOnCamera
{
public:
	// Calculate lock-on camera position
	static void CalculateLockOnCamera(
		const FVector& PlayerLocation,
		const FVector& TargetLocation,
		const FVector& PlayerForward,
		const FForge2LockOnCameraConfig& Config,
		const FVector2D& PlayerInput,
		FVector& OutCameraLocation,
		FRotator& OutCameraRotation
	);

	// Blend from free camera to lock-on
	static void BlendToLockOn(
		const FVector& CurrentLocation,
		const FRotator& CurrentRotation,
		const FVector& LockOnLocation,
		const FRotator& LockOnRotation,
		float BlendAlpha,
		FVector& OutLocation,
		FRotator& OutRotation
	);
};

//=============================================================================
// UTILITY CLASS
//=============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2CameraUtils
{
public:
	// Constraint helpers
	static float ClampPitch(float Pitch, float Min, float Max);
	static float ClampYaw(float Yaw, float Min, float Max);
	static float ClampDistance(float Distance, float Min, float Max);
	static FVector ClampToVolume(const FVector& Location, const FBox& Volume);

	// Smoothing helpers
	static float DampedSpring(float Current, float Target, float& Velocity, float DampingRatio, float Frequency, float DeltaTime);
	static FVector SmoothDamp(const FVector& Current, const FVector& Target, FVector& Velocity, float SmoothTime, float DeltaTime, float MaxSpeed = FLT_MAX);
	static FRotator SmoothDampRotation(const FRotator& Current, const FRotator& Target, FRotator& Velocity, float SmoothTime, float DeltaTime);

	// World-to-screen helpers
	static FVector2D ProjectWorldToScreen(const FVector& WorldLocation, const FMatrix& ViewProjectionMatrix, const FIntRect& ViewRect);
	static bool IsOnScreen(const FVector2D& ScreenPosition, const FIntRect& ViewRect, float Margin = 0.0f);

	// FOV helpers
	static float CalculateDistanceForFrustumWidth(float Width, float FOV);
	static float CalculateFOVForDistance(float Distance, float Width);
};

#endif // Include guard handled by pragma once
