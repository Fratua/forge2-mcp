// Forge2CameraSystem.cpp
// FORGE2 Camera System Implementation

#include "Forge2CameraSystem.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"

//=============================================================================
// FForge2CameraSystem Implementation
//=============================================================================

FForge2CameraModeDefinition FForge2CameraSystem::CreateDefaultThirdPerson()
{
	FForge2CameraModeDefinition Mode;
	Mode.ModeId = TEXT("default_third_person");
	Mode.ModeType = EForge2CameraMode::ThirdPerson;
	Mode.FieldOfView = 90.0f;

	Mode.ThirdPersonConfig.TargetArmLength = 300.0f;
	Mode.ThirdPersonConfig.TargetOffset = FVector(0.0f, 0.0f, 60.0f);
	Mode.ThirdPersonConfig.bUsePawnControlRotation = true;
	Mode.ThirdPersonConfig.PitchMin = -80.0f;
	Mode.ThirdPersonConfig.PitchMax = 80.0f;

	Mode.CollisionConfig.bEnableCollision = true;
	Mode.CollisionConfig.CollisionMode = EForge2CameraCollisionMode::Push;
	Mode.CollisionConfig.ProbeSize = 12.0f;

	Mode.LagConfig.bEnablePositionLag = true;
	Mode.LagConfig.PositionLagSpeed = 10.0f;
	Mode.LagConfig.bEnableRotationLag = true;
	Mode.LagConfig.RotationLagSpeed = 10.0f;

	return Mode;
}

FForge2CameraModeDefinition FForge2CameraSystem::CreateDefaultFirstPerson()
{
	FForge2CameraModeDefinition Mode;
	Mode.ModeId = TEXT("default_first_person");
	Mode.ModeType = EForge2CameraMode::FirstPerson;
	Mode.FieldOfView = 90.0f;

	Mode.FirstPersonConfig.AttachSocketName = "head";
	Mode.FirstPersonConfig.PitchMin = -89.0f;
	Mode.FirstPersonConfig.PitchMax = 89.0f;
	Mode.FirstPersonConfig.bHideOwnerMesh = true;

	Mode.CollisionConfig.bEnableCollision = false;

	return Mode;
}

FForge2CameraModeDefinition FForge2CameraSystem::CreateDefaultIsometric()
{
	FForge2CameraModeDefinition Mode;
	Mode.ModeId = TEXT("default_isometric");
	Mode.ModeType = EForge2CameraMode::Isometric;
	Mode.FieldOfView = 60.0f;

	Mode.IsometricConfig.Height = 1000.0f;
	Mode.IsometricConfig.Angle = 45.0f;
	Mode.IsometricConfig.Rotation = FRotator(-45.0f, 45.0f, 0.0f);
	Mode.IsometricConfig.bOrthographic = true;
	Mode.IsometricConfig.OrthoWidth = 2048.0f;
	Mode.IsometricConfig.bFixedRotation = true;

	Mode.LagConfig.bEnablePositionLag = true;
	Mode.LagConfig.PositionLagSpeed = 5.0f;

	return Mode;
}

FForge2CameraModeDefinition FForge2CameraSystem::CreateDefaultSideScroller()
{
	FForge2CameraModeDefinition Mode;
	Mode.ModeId = TEXT("default_side_scroller");
	Mode.ModeType = EForge2CameraMode::SideScroller;
	Mode.FieldOfView = 90.0f;

	Mode.SideScrollerConfig.Distance = 500.0f;
	Mode.SideScrollerConfig.DeadZoneX = 100.0f;
	Mode.SideScrollerConfig.DeadZoneY = 50.0f;
	Mode.SideScrollerConfig.LookAheadDistance = 150.0f;

	Mode.LagConfig.bEnablePositionLag = true;
	Mode.LagConfig.PositionLagSpeed = 3.0f;

	return Mode;
}

FForge2CameraModeDefinition FForge2CameraSystem::CreateDefaultTopDown()
{
	FForge2CameraModeDefinition Mode;
	Mode.ModeId = TEXT("default_top_down");
	Mode.ModeType = EForge2CameraMode::TopDown;
	Mode.FieldOfView = 60.0f;

	Mode.IsometricConfig.Height = 800.0f;
	Mode.IsometricConfig.Angle = 90.0f;
	Mode.IsometricConfig.Rotation = FRotator(-90.0f, 0.0f, 0.0f);
	Mode.IsometricConfig.bOrthographic = false;

	return Mode;
}

FForge2CameraModeDefinition FForge2CameraSystem::CreateDefaultOrbit()
{
	FForge2CameraModeDefinition Mode;
	Mode.ModeId = TEXT("default_orbit");
	Mode.ModeType = EForge2CameraMode::Orbit;
	Mode.FieldOfView = 60.0f;

	Mode.OrbitConfig.Distance = 500.0f;
	Mode.OrbitConfig.MinDistance = 100.0f;
	Mode.OrbitConfig.MaxDistance = 2000.0f;
	Mode.OrbitConfig.RotationSpeed = 1.0f;
	Mode.OrbitConfig.PitchMin = -89.0f;
	Mode.OrbitConfig.PitchMax = 89.0f;

	Mode.CollisionConfig.bEnableCollision = true;
	Mode.CollisionConfig.CollisionMode = EForge2CameraCollisionMode::Push;

	return Mode;
}

void FForge2CameraSystem::CalculateCameraTransform(
	const FForge2CameraModeDefinition& Mode,
	const FTransform& TargetTransform,
	const FVector2D& Input,
	float DeltaTime,
	FVector& OutLocation,
	FRotator& OutRotation,
	float& OutFOV)
{
	OutFOV = Mode.FieldOfView;

	switch (Mode.ModeType)
	{
	case EForge2CameraMode::ThirdPerson:
	{
		const FForge2ThirdPersonCameraConfig& Config = Mode.ThirdPersonConfig;

		// Calculate camera position relative to target
		FVector TargetLocation = TargetTransform.GetLocation() + Config.TargetOffset;
		FRotator TargetRotation = TargetTransform.GetRotation().Rotator();

		// Apply arm rotation
		FVector ArmOffset = TargetRotation.Vector() * -Config.TargetArmLength;
		OutLocation = TargetLocation + ArmOffset + Config.SocketOffset;
		OutRotation = (TargetLocation - OutLocation).Rotation();
		break;
	}

	case EForge2CameraMode::FirstPerson:
	{
		const FForge2FirstPersonCameraConfig& Config = Mode.FirstPersonConfig;
		OutLocation = TargetTransform.GetLocation() + Config.LocationOffset;
		OutRotation = TargetTransform.GetRotation().Rotator() + Config.RotationOffset;
		OutFOV = Config.FieldOfView;
		break;
	}

	case EForge2CameraMode::Isometric:
	case EForge2CameraMode::TopDown:
	{
		const FForge2IsometricCameraConfig& Config = Mode.IsometricConfig;
		FVector TargetLocation = TargetTransform.GetLocation();

		// Calculate camera position above target
		FVector Offset = Config.Rotation.Vector() * -Config.Height;
		OutLocation = TargetLocation + FVector(0.0f, 0.0f, Config.Height);
		OutRotation = Config.Rotation;
		break;
	}

	case EForge2CameraMode::SideScroller:
	{
		const FForge2SideScrollerCameraConfig& Config = Mode.SideScrollerConfig;
		FVector TargetLocation = TargetTransform.GetLocation();

		// Side camera (typically looking from -Y or +Y)
		OutLocation = FVector(TargetLocation.X, TargetLocation.Y + Config.Distance, TargetLocation.Z);
		OutRotation = FRotator(0.0f, -90.0f, 0.0f);
		break;
	}

	case EForge2CameraMode::Orbit:
	{
		const FForge2OrbitCameraConfig& Config = Mode.OrbitConfig;
		FVector TargetLocation = TargetTransform.GetLocation() + Config.FocusOffset;

		// Calculate orbit position using input as angles
		float Yaw = Input.X * 360.0f;
		float Pitch = FMath::Clamp(Input.Y * 90.0f, Config.PitchMin, Config.PitchMax);

		FRotator OrbitRotation(Pitch, Yaw, 0.0f);
		FVector OrbitOffset = OrbitRotation.Vector() * Config.Distance;

		OutLocation = TargetLocation - OrbitOffset;
		OutRotation = (TargetLocation - OutLocation).Rotation();
		break;
	}

	default:
		OutLocation = TargetTransform.GetLocation();
		OutRotation = TargetTransform.GetRotation().Rotator();
		break;
	}
}

//=============================================================================
// FForge2CameraCollision Implementation
//=============================================================================

FForge2CameraCollisionResult FForge2CameraCollision::CheckCollision(
	UWorld* World,
	const FVector& TargetLocation,
	const FVector& CameraLocation,
	const FForge2CameraCollisionConfig& Config)
{
	FForge2CameraCollisionResult Result;

	if (!World || !Config.bEnableCollision)
	{
		return Result;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	for (AActor* IgnoredActor : Config.IgnoredActors)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}

	FHitResult HitResult;
	bool bHit = World->SweepSingleByChannel(
		HitResult,
		TargetLocation,
		CameraLocation,
		FQuat::Identity,
		Config.ProbeChannel,
		FCollisionShape::MakeSphere(Config.ProbeSize),
		QueryParams
	);

	if (bHit)
	{
		Result.bIsColliding = true;
		Result.CollisionDistance = HitResult.Distance;
		Result.CollisionPoint = HitResult.ImpactPoint;
		Result.BlockingActor = HitResult.GetActor();
	}

	return Result;
}

FVector FForge2CameraCollision::ApplyCollisionResponse(
	const FVector& DesiredLocation,
	const FVector& TargetLocation,
	const FForge2CameraCollisionResult& CollisionResult,
	const FForge2CameraCollisionConfig& Config,
	float CurrentDistance,
	float DeltaTime)
{
	if (!CollisionResult.bIsColliding)
	{
		// Smoothly return to desired position
		float DesiredDist = FVector::Dist(DesiredLocation, TargetLocation);
		float NewDist = FMath::FInterpTo(CurrentDistance, DesiredDist, DeltaTime, Config.ReturnSpeed);
		FVector Direction = (DesiredLocation - TargetLocation).GetSafeNormal();
		return TargetLocation + Direction * NewDist;
	}

	switch (Config.CollisionMode)
	{
	case EForge2CameraCollisionMode::Push:
	{
		float SafeDistance = FMath::Max(Config.MinimumDistance, CollisionResult.CollisionDistance - Config.ProbeSize);
		float NewDist = FMath::FInterpTo(CurrentDistance, SafeDistance, DeltaTime, Config.PushSpeed);
		FVector Direction = (DesiredLocation - TargetLocation).GetSafeNormal();
		return TargetLocation + Direction * NewDist;
	}

	case EForge2CameraCollisionMode::Cut:
	{
		float SafeDistance = FMath::Max(Config.MinimumDistance, CollisionResult.CollisionDistance - Config.ProbeSize);
		FVector Direction = (DesiredLocation - TargetLocation).GetSafeNormal();
		return TargetLocation + Direction * SafeDistance;
	}

	default:
		return DesiredLocation;
	}
}

TArray<AActor*> FForge2CameraCollision::FindBlockingActors(
	UWorld* World,
	const FVector& CameraLocation,
	const FVector& TargetLocation,
	const FForge2CameraCollisionConfig& Config)
{
	TArray<AActor*> BlockingActors;

	if (!World)
	{
		return BlockingActors;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	for (AActor* IgnoredActor : Config.IgnoredActors)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}

	TArray<FHitResult> HitResults;
	World->SweepMultiByChannel(
		HitResults,
		CameraLocation,
		TargetLocation,
		FQuat::Identity,
		Config.ProbeChannel,
		FCollisionShape::MakeSphere(Config.ProbeSize * 2.0f),  // Slightly larger for transparency
		QueryParams
	);

	for (const FHitResult& Hit : HitResults)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (!BlockingActors.Contains(HitActor))
			{
				BlockingActors.Add(HitActor);
			}
		}
	}

	return BlockingActors;
}

//=============================================================================
// FForge2CameraShake Implementation
//=============================================================================

FForge2CameraShakeInstance FForge2CameraShake::StartShake(
	const FForge2CameraShakeConfig& Config,
	float Scale,
	const FVector& Epicenter)
{
	FForge2CameraShakeInstance Instance;
	Instance.ShakeId = Config.ShakeId;
	Instance.ElapsedTime = 0.0f;
	Instance.Scale = Scale;
	Instance.Epicenter = Epicenter;
	Instance.bInfinite = (Config.Duration <= 0.0f);
	return Instance;
}

bool FForge2CameraShake::UpdateShake(
	FForge2CameraShakeInstance& Instance,
	const FForge2CameraShakeConfig& Config,
	float DeltaTime,
	const FVector& CameraLocation)
{
	Instance.ElapsedTime += DeltaTime;

	// Check if shake should end
	if (!Instance.bInfinite && Instance.ElapsedTime >= Config.Duration)
	{
		return false;
	}

	// Calculate intensity based on time
	float Intensity = Instance.Scale;

	// Blend in/out
	if (Instance.ElapsedTime < Config.BlendInTime)
	{
		Intensity *= Instance.ElapsedTime / Config.BlendInTime;
	}
	else if (!Instance.bInfinite && Instance.ElapsedTime > (Config.Duration - Config.BlendOutTime))
	{
		float RemainingTime = Config.Duration - Instance.ElapsedTime;
		Intensity *= RemainingTime / Config.BlendOutTime;
	}

	// Distance attenuation
	if (Config.bAttenuateWithDistance && !Instance.Epicenter.IsZero())
	{
		float Distance = FVector::Dist(CameraLocation, Instance.Epicenter);
		if (Distance > Config.InnerRadius)
		{
			float Falloff = 1.0f - FMath::Clamp(
				(Distance - Config.InnerRadius) / (Config.OuterRadius - Config.InnerRadius),
				0.0f, 1.0f
			);
			Intensity *= FMath::Pow(Falloff, Config.Falloff);
		}
	}

	// Calculate shake offsets
	GetShakeOffset(Instance, Config, Instance.CurrentLocationOffset, Instance.CurrentRotationOffset, Instance.CurrentFOVOffset);

	// Apply intensity
	Instance.CurrentLocationOffset *= Intensity;
	Instance.CurrentRotationOffset *= Intensity;
	Instance.CurrentFOVOffset *= Intensity;

	return true;
}

void FForge2CameraShake::GetShakeOffset(
	const FForge2CameraShakeInstance& Instance,
	const FForge2CameraShakeConfig& Config,
	FVector& OutLocationOffset,
	FRotator& OutRotationOffset,
	float& OutFOVOffset)
{
	float Time = Instance.ElapsedTime;

	switch (Config.ShakeType)
	{
	case EForge2CameraShakeType::Perlin:
	{
		OutLocationOffset = FVector(
			SamplePerlinNoise(Time + 0.0f, Config.LocationFrequency.X) * Config.LocationAmplitude.X,
			SamplePerlinNoise(Time + 100.0f, Config.LocationFrequency.Y) * Config.LocationAmplitude.Y,
			SamplePerlinNoise(Time + 200.0f, Config.LocationFrequency.Z) * Config.LocationAmplitude.Z
		);
		OutRotationOffset = FRotator(
			SamplePerlinNoise(Time + 300.0f, Config.RotationFrequency.Pitch) * Config.RotationAmplitude.Pitch,
			SamplePerlinNoise(Time + 400.0f, Config.RotationFrequency.Yaw) * Config.RotationAmplitude.Yaw,
			SamplePerlinNoise(Time + 500.0f, Config.RotationFrequency.Roll) * Config.RotationAmplitude.Roll
		);
		OutFOVOffset = SamplePerlinNoise(Time + 600.0f, 10.0f) * Config.FOVAmplitude;
		break;
	}

	case EForge2CameraShakeType::Oscillation:
	{
		OutLocationOffset = FVector(
			SampleOscillation(Time, Config.LocationFrequency.X) * Config.LocationAmplitude.X,
			SampleOscillation(Time, Config.LocationFrequency.Y) * Config.LocationAmplitude.Y,
			SampleOscillation(Time, Config.LocationFrequency.Z) * Config.LocationAmplitude.Z
		);
		OutRotationOffset = FRotator(
			SampleOscillation(Time, Config.RotationFrequency.Pitch) * Config.RotationAmplitude.Pitch,
			SampleOscillation(Time, Config.RotationFrequency.Yaw) * Config.RotationAmplitude.Yaw,
			SampleOscillation(Time, Config.RotationFrequency.Roll) * Config.RotationAmplitude.Roll
		);
		OutFOVOffset = SampleOscillation(Time, 10.0f) * Config.FOVAmplitude;
		break;
	}

	case EForge2CameraShakeType::Impact:
	{
		// Exponential decay
		float Decay = FMath::Exp(-Time * 5.0f);
		float OscTime = Time * 20.0f;

		OutLocationOffset = FVector(
			FMath::Sin(OscTime * 1.0f) * Decay * Config.LocationAmplitude.X,
			FMath::Sin(OscTime * 1.3f) * Decay * Config.LocationAmplitude.Y,
			FMath::Sin(OscTime * 0.7f) * Decay * Config.LocationAmplitude.Z
		);
		OutRotationOffset = FRotator(
			FMath::Sin(OscTime * 1.1f) * Decay * Config.RotationAmplitude.Pitch,
			FMath::Sin(OscTime * 0.9f) * Decay * Config.RotationAmplitude.Yaw,
			FMath::Sin(OscTime * 1.2f) * Decay * Config.RotationAmplitude.Roll
		);
		OutFOVOffset = FMath::Sin(OscTime) * Decay * Config.FOVAmplitude;
		break;
	}

	default:
		OutLocationOffset = FVector::ZeroVector;
		OutRotationOffset = FRotator::ZeroRotator;
		OutFOVOffset = 0.0f;
		break;
	}
}

FForge2CameraShakeConfig FForge2CameraShake::CreateHitShake(float Intensity)
{
	FForge2CameraShakeConfig Config;
	Config.ShakeId = TEXT("hit_shake");
	Config.ShakeType = EForge2CameraShakeType::Impact;
	Config.Duration = 0.3f;
	Config.BlendInTime = 0.0f;
	Config.BlendOutTime = 0.1f;
	Config.LocationAmplitude = FVector(5.0f, 5.0f, 3.0f) * Intensity;
	Config.RotationAmplitude = FRotator(1.0f, 1.0f, 0.5f) * Intensity;
	return Config;
}

FForge2CameraShakeConfig FForge2CameraShake::CreateExplosionShake(float Intensity, float Distance)
{
	FForge2CameraShakeConfig Config;
	Config.ShakeId = TEXT("explosion_shake");
	Config.ShakeType = EForge2CameraShakeType::Impact;
	Config.Duration = 0.8f;
	Config.BlendInTime = 0.0f;
	Config.BlendOutTime = 0.3f;
	Config.LocationAmplitude = FVector(15.0f, 15.0f, 10.0f) * Intensity;
	Config.RotationAmplitude = FRotator(3.0f, 3.0f, 1.0f) * Intensity;
	Config.FOVAmplitude = 2.0f * Intensity;
	Config.bAttenuateWithDistance = true;
	Config.InnerRadius = 0.0f;
	Config.OuterRadius = Distance;
	Config.Falloff = 1.5f;
	return Config;
}

FForge2CameraShakeConfig FForge2CameraShake::CreateFootstepShake(float Intensity)
{
	FForge2CameraShakeConfig Config;
	Config.ShakeId = TEXT("footstep_shake");
	Config.ShakeType = EForge2CameraShakeType::Oscillation;
	Config.Duration = 0.15f;
	Config.LocationAmplitude = FVector(0.0f, 0.0f, 1.0f) * Intensity;
	Config.LocationFrequency = FVector(0.0f, 0.0f, 15.0f);
	return Config;
}

FForge2CameraShakeConfig FForge2CameraShake::CreateAmbientShake(float Intensity)
{
	FForge2CameraShakeConfig Config;
	Config.ShakeId = TEXT("ambient_shake");
	Config.ShakeType = EForge2CameraShakeType::Perlin;
	Config.Duration = 0.0f;  // Infinite
	Config.LocationAmplitude = FVector(0.5f, 0.5f, 0.3f) * Intensity;
	Config.LocationFrequency = FVector(2.0f, 2.0f, 1.5f);
	Config.RotationAmplitude = FRotator(0.1f, 0.1f, 0.05f) * Intensity;
	Config.RotationFrequency = FRotator(1.0f, 1.0f, 0.5f);
	return Config;
}

float FForge2CameraShake::SamplePerlinNoise(float Time, float Frequency)
{
	// Simplified Perlin-like noise using multiple sine waves
	float Value = FMath::Sin(Time * Frequency);
	Value += FMath::Sin(Time * Frequency * 2.17f + 1.0f) * 0.5f;
	Value += FMath::Sin(Time * Frequency * 4.31f + 2.0f) * 0.25f;
	return Value / 1.75f;  // Normalize
}

float FForge2CameraShake::SampleOscillation(float Time, float Frequency)
{
	return FMath::Sin(Time * Frequency * PI * 2.0f);
}

//=============================================================================
// FForge2CameraLag Implementation
//=============================================================================

FVector FForge2CameraLag::ApplyPositionLag(
	const FVector& CurrentLocation,
	const FVector& TargetLocation,
	const FForge2CameraLagConfig& Config,
	float DeltaTime)
{
	if (!Config.bEnablePositionLag)
	{
		return TargetLocation;
	}

	FVector NewLocation;

	if (Config.bSeparateAxes)
	{
		NewLocation = FVector(
			FMath::FInterpTo(CurrentLocation.X, TargetLocation.X, DeltaTime, Config.PositionLagSpeedPerAxis.X),
			FMath::FInterpTo(CurrentLocation.Y, TargetLocation.Y, DeltaTime, Config.PositionLagSpeedPerAxis.Y),
			FMath::FInterpTo(CurrentLocation.Z, TargetLocation.Z, DeltaTime, Config.PositionLagSpeedPerAxis.Z)
		);
	}
	else
	{
		NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, Config.PositionLagSpeed);
	}

	// Clamp max distance
	FVector Offset = NewLocation - TargetLocation;
	if (Offset.Size() > Config.MaxPositionLagDistance)
	{
		Offset = Offset.GetSafeNormal() * Config.MaxPositionLagDistance;
		NewLocation = TargetLocation + Offset;
	}

	return NewLocation;
}

FRotator FForge2CameraLag::ApplyRotationLag(
	const FRotator& CurrentRotation,
	const FRotator& TargetRotation,
	const FForge2CameraLagConfig& Config,
	float DeltaTime)
{
	if (!Config.bEnableRotationLag)
	{
		return TargetRotation;
	}

	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, Config.RotationLagSpeed);

	// Clamp max angle difference
	FRotator Delta = (NewRotation - TargetRotation).GetNormalized();
	float MaxAngle = Config.MaxRotationLagAngle;

	Delta.Pitch = FMath::Clamp(Delta.Pitch, -MaxAngle, MaxAngle);
	Delta.Yaw = FMath::Clamp(Delta.Yaw, -MaxAngle, MaxAngle);
	Delta.Roll = FMath::Clamp(Delta.Roll, -MaxAngle, MaxAngle);

	return TargetRotation + Delta;
}

float FForge2CameraLag::ApplyZoomLag(
	float CurrentZoom,
	float TargetZoom,
	const FForge2CameraLagConfig& Config,
	float DeltaTime)
{
	if (!Config.bEnableZoomLag)
	{
		return TargetZoom;
	}

	return FMath::FInterpTo(CurrentZoom, TargetZoom, DeltaTime, Config.ZoomLagSpeed);
}

//=============================================================================
// FForge2CameraTransitions Implementation
//=============================================================================

FForge2CameraTransition FForge2CameraTransitions::BeginTransition(
	const FVector& FromLocation,
	const FRotator& FromRotation,
	float FromFOV,
	const FVector& ToLocation,
	const FRotator& ToRotation,
	float ToFOV,
	const FForge2CameraTransition& TransitionSettings)
{
	FForge2CameraTransition Transition = TransitionSettings;
	Transition.CurrentBlend = 0.0f;
	Transition.StartLocation = FromLocation;
	Transition.StartRotation = FromRotation;
	Transition.StartFOV = FromFOV;
	Transition.TargetLocation = ToLocation;
	Transition.TargetRotation = ToRotation;
	Transition.TargetFOV = ToFOV;
	return Transition;
}

bool FForge2CameraTransitions::UpdateTransition(
	FForge2CameraTransition& Transition,
	float DeltaTime,
	FVector& OutLocation,
	FRotator& OutRotation,
	float& OutFOV)
{
	if (Transition.TransitionType == EForge2CameraTransitionType::Cut)
	{
		OutLocation = Transition.TargetLocation;
		OutRotation = Transition.TargetRotation;
		OutFOV = Transition.TargetFOV;
		return false;
	}

	// Update blend
	if (Transition.Duration > 0.0f)
	{
		Transition.CurrentBlend += DeltaTime / Transition.Duration;
		Transition.CurrentBlend = FMath::Clamp(Transition.CurrentBlend, 0.0f, 1.0f);
	}
	else
	{
		Transition.CurrentBlend = 1.0f;
	}

	// Get eased alpha
	float Alpha = GetEasedAlpha(Transition.CurrentBlend, Transition.TransitionType,
		Transition.CustomCurve.LoadSynchronous());

	// Interpolate
	OutLocation = FMath::Lerp(Transition.StartLocation, Transition.TargetLocation, Alpha);
	OutRotation = FMath::Lerp(Transition.StartRotation, Transition.TargetRotation, Alpha);
	OutFOV = FMath::Lerp(Transition.StartFOV, Transition.TargetFOV, Alpha);

	return Transition.CurrentBlend < 1.0f;
}

float FForge2CameraTransitions::GetEasedAlpha(float Alpha, EForge2CameraTransitionType Type, UCurveFloat* CustomCurve)
{
	switch (Type)
	{
	case EForge2CameraTransitionType::Cut:
		return 1.0f;

	case EForge2CameraTransitionType::Blend:
		return Alpha;

	case EForge2CameraTransitionType::EaseIn:
		return Alpha * Alpha;

	case EForge2CameraTransitionType::EaseOut:
		return 1.0f - (1.0f - Alpha) * (1.0f - Alpha);

	case EForge2CameraTransitionType::EaseInOut:
		return Alpha < 0.5f
			? 2.0f * Alpha * Alpha
			: 1.0f - 2.0f * (1.0f - Alpha) * (1.0f - Alpha);

	case EForge2CameraTransitionType::Custom:
		if (CustomCurve)
		{
			return CustomCurve->GetFloatValue(Alpha);
		}
		return Alpha;

	default:
		return Alpha;
	}
}

//=============================================================================
// FForge2SplitScreen Implementation
//=============================================================================

TArray<FForge2SplitScreenViewport> FForge2SplitScreen::CalculateViewports(
	int32 PlayerCount,
	const FForge2SplitScreenConfig& Config,
	const TArray<FVector>& PlayerLocations)
{
	TArray<FForge2SplitScreenViewport> Viewports;

	if (PlayerCount <= 0)
	{
		return Viewports;
	}

	// Single player - full screen
	if (PlayerCount == 1 || Config.Mode == EForge2SplitScreenMode::None)
	{
		FForge2SplitScreenViewport Viewport;
		Viewport.PlayerIndex = 0;
		Viewport.Origin = FVector2D::ZeroVector;
		Viewport.Size = FVector2D(1.0f, 1.0f);
		Viewports.Add(Viewport);
		return Viewports;
	}

	// Check for dynamic merge
	if (Config.Mode == EForge2SplitScreenMode::Dynamic && ShouldMerge(PlayerLocations, Config))
	{
		FForge2SplitScreenViewport Viewport;
		Viewport.PlayerIndex = 0;  // All players share
		Viewport.Origin = FVector2D::ZeroVector;
		Viewport.Size = FVector2D(1.0f, 1.0f);
		Viewport.bIsMerged = true;
		Viewports.Add(Viewport);
		return Viewports;
	}

	// Create split viewports
	switch (Config.Mode)
	{
	case EForge2SplitScreenMode::Horizontal:
		for (int32 i = 0; i < PlayerCount && i < 2; i++)
		{
			FForge2SplitScreenViewport Viewport;
			Viewport.PlayerIndex = i;
			Viewport.Origin = FVector2D(0.0f, i * 0.5f);
			Viewport.Size = FVector2D(1.0f, 0.5f);
			Viewports.Add(Viewport);
		}
		break;

	case EForge2SplitScreenMode::Vertical:
		for (int32 i = 0; i < PlayerCount && i < 2; i++)
		{
			FForge2SplitScreenViewport Viewport;
			Viewport.PlayerIndex = i;
			Viewport.Origin = FVector2D(i * 0.5f, 0.0f);
			Viewport.Size = FVector2D(0.5f, 1.0f);
			Viewports.Add(Viewport);
		}
		break;

	case EForge2SplitScreenMode::Quadrant:
		for (int32 i = 0; i < PlayerCount && i < 4; i++)
		{
			FForge2SplitScreenViewport Viewport;
			Viewport.PlayerIndex = i;
			Viewport.Origin = FVector2D((i % 2) * 0.5f, (i / 2) * 0.5f);
			Viewport.Size = FVector2D(0.5f, 0.5f);
			Viewports.Add(Viewport);
		}
		break;

	case EForge2SplitScreenMode::Dynamic:
		// Fall back to vertical for 2 players
		for (int32 i = 0; i < PlayerCount && i < 2; i++)
		{
			FForge2SplitScreenViewport Viewport;
			Viewport.PlayerIndex = i;
			Viewport.Origin = FVector2D(i * 0.5f, 0.0f);
			Viewport.Size = FVector2D(0.5f, 1.0f);
			Viewports.Add(Viewport);
		}
		break;

	default:
		break;
	}

	return Viewports;
}

bool FForge2SplitScreen::ShouldMerge(
	const TArray<FVector>& PlayerLocations,
	const FForge2SplitScreenConfig& Config)
{
	if (PlayerLocations.Num() < 2)
	{
		return true;
	}

	// Check if all players are within merge distance
	float MaxDistance = 0.0f;
	for (int32 i = 0; i < PlayerLocations.Num(); i++)
	{
		for (int32 j = i + 1; j < PlayerLocations.Num(); j++)
		{
			float Distance = FVector::Dist(PlayerLocations[i], PlayerLocations[j]);
			MaxDistance = FMath::Max(MaxDistance, Distance);
		}
	}

	return MaxDistance <= Config.DynamicMergeDistance;
}

void FForge2SplitScreen::CalculateMergedCamera(
	const TArray<FVector>& PlayerLocations,
	FVector& OutCameraLocation,
	float& OutZoom)
{
	if (PlayerLocations.Num() == 0)
	{
		OutCameraLocation = FVector::ZeroVector;
		OutZoom = 1.0f;
		return;
	}

	// Calculate center point
	FVector Center = FVector::ZeroVector;
	for (const FVector& Location : PlayerLocations)
	{
		Center += Location;
	}
	Center /= PlayerLocations.Num();

	// Calculate needed zoom to fit all players
	float MaxDistance = 0.0f;
	for (const FVector& Location : PlayerLocations)
	{
		float Distance = FVector::Dist(Location, Center);
		MaxDistance = FMath::Max(MaxDistance, Distance);
	}

	OutCameraLocation = Center;
	OutZoom = 1.0f + (MaxDistance / 500.0f);  // Adjust based on desired base distance
}

//=============================================================================
// FForge2LockOnCamera Implementation
//=============================================================================

void FForge2LockOnCamera::CalculateLockOnCamera(
	const FVector& PlayerLocation,
	const FVector& TargetLocation,
	const FVector& PlayerForward,
	const FForge2LockOnCameraConfig& Config,
	const FVector2D& PlayerInput,
	FVector& OutCameraLocation,
	FRotator& OutCameraRotation)
{
	// Calculate midpoint between player and target
	FVector MidPoint = (PlayerLocation + TargetLocation) * 0.5f;
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// Calculate camera position relative to the player-target line
	FVector Right = FVector::CrossProduct(FVector::UpVector, ToTarget).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Right = FVector::CrossProduct(FVector::RightVector, ToTarget).GetSafeNormal();
	}

	// Apply yaw offset
	FRotator YawRotation(0.0f, Config.YawOffset, 0.0f);
	FVector OffsetDirection = YawRotation.RotateVector(-ToTarget);

	// Calculate camera location
	OutCameraLocation = PlayerLocation + Config.CameraOffset.X * OffsetDirection +
						Config.CameraOffset.Y * Right +
						FVector(0.0f, 0.0f, Config.CameraOffset.Z);

	// Apply player input influence
	if (Config.bAllowPlayerInput)
	{
		FVector InputOffset = Right * PlayerInput.X * 100.0f * Config.InputInfluence;
		InputOffset.Z += PlayerInput.Y * 50.0f * Config.InputInfluence;
		OutCameraLocation += InputOffset;
	}

	// Look at target (with offset)
	FVector LookAtPoint = TargetLocation + Config.TargetOffset;
	OutCameraRotation = (LookAtPoint - OutCameraLocation).Rotation();
}

void FForge2LockOnCamera::BlendToLockOn(
	const FVector& CurrentLocation,
	const FRotator& CurrentRotation,
	const FVector& LockOnLocation,
	const FRotator& LockOnRotation,
	float BlendAlpha,
	FVector& OutLocation,
	FRotator& OutRotation)
{
	OutLocation = FMath::Lerp(CurrentLocation, LockOnLocation, BlendAlpha);
	OutRotation = FMath::Lerp(CurrentRotation, LockOnRotation, BlendAlpha);
}

//=============================================================================
// FForge2CameraUtils Implementation
//=============================================================================

float FForge2CameraUtils::ClampPitch(float Pitch, float Min, float Max)
{
	return FMath::Clamp(Pitch, Min, Max);
}

float FForge2CameraUtils::ClampYaw(float Yaw, float Min, float Max)
{
	// Normalize yaw first
	Yaw = FMath::Fmod(Yaw + 180.0f, 360.0f) - 180.0f;
	return FMath::Clamp(Yaw, Min, Max);
}

float FForge2CameraUtils::ClampDistance(float Distance, float Min, float Max)
{
	return FMath::Clamp(Distance, Min, Max);
}

FVector FForge2CameraUtils::ClampToVolume(const FVector& Location, const FBox& Volume)
{
	return FVector(
		FMath::Clamp(Location.X, Volume.Min.X, Volume.Max.X),
		FMath::Clamp(Location.Y, Volume.Min.Y, Volume.Max.Y),
		FMath::Clamp(Location.Z, Volume.Min.Z, Volume.Max.Z)
	);
}

float FForge2CameraUtils::DampedSpring(float Current, float Target, float& Velocity, float DampingRatio, float Frequency, float DeltaTime)
{
	float Omega = Frequency * 2.0f * PI;
	float Damping = 2.0f * DampingRatio * Omega;
	float SpringForce = Omega * Omega;

	float Error = Target - Current;
	float Acceleration = SpringForce * Error - Damping * Velocity;

	Velocity += Acceleration * DeltaTime;
	return Current + Velocity * DeltaTime;
}

FVector FForge2CameraUtils::SmoothDamp(const FVector& Current, const FVector& Target, FVector& Velocity, float SmoothTime, float DeltaTime, float MaxSpeed)
{
	SmoothTime = FMath::Max(0.0001f, SmoothTime);
	float Omega = 2.0f / SmoothTime;

	float x = Omega * DeltaTime;
	float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

	FVector Change = Current - Target;
	FVector OriginalTo = Target;

	// Clamp max speed
	float MaxChange = MaxSpeed * SmoothTime;
	float MaxChangeSq = MaxChange * MaxChange;
	float ChangeSq = Change.SizeSquared();

	if (ChangeSq > MaxChangeSq)
	{
		Change = Change.GetSafeNormal() * MaxChange;
	}

	FVector NewTarget = Current - Change;
	FVector Temp = (Velocity + Omega * Change) * DeltaTime;
	Velocity = (Velocity - Omega * Temp) * exp;
	FVector Output = NewTarget + (Change + Temp) * exp;

	// Prevent overshoot
	if (FVector::DotProduct(OriginalTo - Current, Output - OriginalTo) > 0)
	{
		Output = OriginalTo;
		Velocity = DeltaTime > KINDA_SMALL_NUMBER ? (Output - OriginalTo) / DeltaTime : FVector::ZeroVector;
	}

	return Output;
}

FRotator FForge2CameraUtils::SmoothDampRotation(const FRotator& Current, const FRotator& Target, FRotator& Velocity, float SmoothTime, float DeltaTime)
{
	FVector CurrentVec(Current.Pitch, Current.Yaw, Current.Roll);
	FVector TargetVec(Target.Pitch, Target.Yaw, Target.Roll);
	FVector VelocityVec(Velocity.Pitch, Velocity.Yaw, Velocity.Roll);

	// Handle angle wrapping
	FVector Delta = TargetVec - CurrentVec;
	if (Delta.Y > 180.0f) Delta.Y -= 360.0f;
	if (Delta.Y < -180.0f) Delta.Y += 360.0f;

	FVector AdjustedTarget = CurrentVec + Delta;
	FVector Result = SmoothDamp(CurrentVec, AdjustedTarget, VelocityVec, SmoothTime, DeltaTime);

	Velocity = FRotator(VelocityVec.X, VelocityVec.Y, VelocityVec.Z);
	return FRotator(Result.X, Result.Y, Result.Z);
}

FVector2D FForge2CameraUtils::ProjectWorldToScreen(const FVector& WorldLocation, const FMatrix& ViewProjectionMatrix, const FIntRect& ViewRect)
{
	FVector4 ProjectedPoint = ViewProjectionMatrix.TransformFVector4(FVector4(WorldLocation, 1.0f));

	if (ProjectedPoint.W > 0.0f)
	{
		float ScreenX = (ProjectedPoint.X / ProjectedPoint.W + 1.0f) * 0.5f * ViewRect.Width() + ViewRect.Min.X;
		float ScreenY = (-ProjectedPoint.Y / ProjectedPoint.W + 1.0f) * 0.5f * ViewRect.Height() + ViewRect.Min.Y;
		return FVector2D(ScreenX, ScreenY);
	}

	return FVector2D(-1.0f, -1.0f);
}

bool FForge2CameraUtils::IsOnScreen(const FVector2D& ScreenPosition, const FIntRect& ViewRect, float Margin)
{
	return ScreenPosition.X >= ViewRect.Min.X - Margin &&
		   ScreenPosition.X <= ViewRect.Max.X + Margin &&
		   ScreenPosition.Y >= ViewRect.Min.Y - Margin &&
		   ScreenPosition.Y <= ViewRect.Max.Y + Margin;
}

float FForge2CameraUtils::CalculateDistanceForFrustumWidth(float Width, float FOV)
{
	float HalfFOVRad = FMath::DegreesToRadians(FOV * 0.5f);
	return (Width * 0.5f) / FMath::Tan(HalfFOVRad);
}

float FForge2CameraUtils::CalculateFOVForDistance(float Distance, float Width)
{
	if (Distance <= 0.0f) return 90.0f;  // Default FOV if distance is invalid
	float HalfFOVRad = FMath::Atan((Width * 0.5f) / Distance);
	return FMath::RadiansToDegrees(HalfFOVRad) * 2.0f;
}
