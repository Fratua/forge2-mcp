// Copyright 2025 FORGE2 Project. All Rights Reserved.
// Racing Genre System - Vehicles, Tracks, Race Modes, Physics

#pragma once

#include "CoreMinimal.h"
#include "Forge2RacingSystem.generated.h"

// ============================================================================
// RACING SYSTEM ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2VehicleType : uint8
{
	Car				UMETA(DisplayName = "Car"),
	Motorcycle		UMETA(DisplayName = "Motorcycle"),
	Truck			UMETA(DisplayName = "Truck"),
	Boat			UMETA(DisplayName = "Boat"),
	Jet				UMETA(DisplayName = "Jet"),
	Hovercraft		UMETA(DisplayName = "Hovercraft"),
	Kart			UMETA(DisplayName = "Kart"),
	ATV				UMETA(DisplayName = "ATV"),
	Snowmobile		UMETA(DisplayName = "Snowmobile"),
	Custom			UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2DriveType : uint8
{
	FrontWheelDrive		UMETA(DisplayName = "Front Wheel Drive"),
	RearWheelDrive		UMETA(DisplayName = "Rear Wheel Drive"),
	AllWheelDrive		UMETA(DisplayName = "All Wheel Drive"),
	MidEngine			UMETA(DisplayName = "Mid Engine"),
	Custom				UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2HandlingType : uint8
{
	Arcade			UMETA(DisplayName = "Arcade"),
	Simcade			UMETA(DisplayName = "Simcade"),
	Simulation		UMETA(DisplayName = "Simulation")
};

UENUM(BlueprintType)
enum class EForge2TireType : uint8
{
	Soft			UMETA(DisplayName = "Soft"),
	Medium			UMETA(DisplayName = "Medium"),
	Hard			UMETA(DisplayName = "Hard"),
	Wet				UMETA(DisplayName = "Wet"),
	Intermediate	UMETA(DisplayName = "Intermediate"),
	Slick			UMETA(DisplayName = "Slick"),
	OffRoad			UMETA(DisplayName = "Off-Road"),
	Drift			UMETA(DisplayName = "Drift")
};

UENUM(BlueprintType)
enum class EForge2TireCondition : uint8
{
	Fresh			UMETA(DisplayName = "Fresh"),
	Good			UMETA(DisplayName = "Good"),
	Worn			UMETA(DisplayName = "Worn"),
	Critical		UMETA(DisplayName = "Critical"),
	Destroyed		UMETA(DisplayName = "Destroyed")
};

UENUM(BlueprintType)
enum class EForge2DamageType : uint8
{
	None			UMETA(DisplayName = "None"),
	Cosmetic		UMETA(DisplayName = "Cosmetic Only"),
	Mechanical		UMETA(DisplayName = "Mechanical"),
	Full			UMETA(DisplayName = "Full Simulation")
};

UENUM(BlueprintType)
enum class EForge2VehicleComponent : uint8
{
	Engine			UMETA(DisplayName = "Engine"),
	Transmission	UMETA(DisplayName = "Transmission"),
	Suspension		UMETA(DisplayName = "Suspension"),
	Brakes			UMETA(DisplayName = "Brakes"),
	Steering		UMETA(DisplayName = "Steering"),
	Tires			UMETA(DisplayName = "Tires"),
	Body			UMETA(DisplayName = "Body"),
	Aerodynamics	UMETA(DisplayName = "Aerodynamics")
};

UENUM(BlueprintType)
enum class EForge2RaceMode : uint8
{
	Circuit			UMETA(DisplayName = "Circuit Race"),
	TimeTrial		UMETA(DisplayName = "Time Trial"),
	Sprint			UMETA(DisplayName = "Sprint"),
	Elimination		UMETA(DisplayName = "Elimination"),
	Drift			UMETA(DisplayName = "Drift Challenge"),
	Drag			UMETA(DisplayName = "Drag Race"),
	Checkpoint		UMETA(DisplayName = "Checkpoint"),
	BattleRoyale	UMETA(DisplayName = "Battle Royale"),
	Pursuit			UMETA(DisplayName = "Pursuit"),
	FreeRoam		UMETA(DisplayName = "Free Roam")
};

UENUM(BlueprintType)
enum class EForge2TrackSurface : uint8
{
	Asphalt			UMETA(DisplayName = "Asphalt"),
	Concrete		UMETA(DisplayName = "Concrete"),
	Dirt			UMETA(DisplayName = "Dirt"),
	Gravel			UMETA(DisplayName = "Gravel"),
	Sand			UMETA(DisplayName = "Sand"),
	Grass			UMETA(DisplayName = "Grass"),
	Snow			UMETA(DisplayName = "Snow"),
	Ice				UMETA(DisplayName = "Ice"),
	Water			UMETA(DisplayName = "Water"),
	Mud				UMETA(DisplayName = "Mud")
};

UENUM(BlueprintType)
enum class EForge2PowerupType : uint8
{
	SpeedBoost		UMETA(DisplayName = "Speed Boost"),
	Shield			UMETA(DisplayName = "Shield"),
	Missile			UMETA(DisplayName = "Missile"),
	OilSlick		UMETA(DisplayName = "Oil Slick"),
	Mines			UMETA(DisplayName = "Mines"),
	Lightning		UMETA(DisplayName = "Lightning"),
	Ghost			UMETA(DisplayName = "Ghost"),
	Teleport		UMETA(DisplayName = "Teleport"),
	SlowField		UMETA(DisplayName = "Slow Field"),
	Repair			UMETA(DisplayName = "Repair")
};

UENUM(BlueprintType)
enum class EForge2AIRacerDifficulty : uint8
{
	Beginner		UMETA(DisplayName = "Beginner"),
	Easy			UMETA(DisplayName = "Easy"),
	Normal			UMETA(DisplayName = "Normal"),
	Hard			UMETA(DisplayName = "Hard"),
	Expert			UMETA(DisplayName = "Expert"),
	Legendary		UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EForge2RubberBandType : uint8
{
	None			UMETA(DisplayName = "None"),
	Subtle			UMETA(DisplayName = "Subtle"),
	Moderate		UMETA(DisplayName = "Moderate"),
	Aggressive		UMETA(DisplayName = "Aggressive")
};

// ============================================================================
// VEHICLE PHYSICS STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2EngineConfig
{
	GENERATED_BODY()

	// Maximum engine power in horsepower
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxHorsepower = 400.0f;

	// Maximum torque in Nm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxTorque = 500.0f;

	// RPM at which max horsepower occurs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxHorsepowerRPM = 6500.0f;

	// RPM at which max torque occurs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float MaxTorqueRPM = 4500.0f;

	// Engine redline RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float RedlineRPM = 8000.0f;

	// Idle RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float IdleRPM = 800.0f;

	// Engine braking force
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float EngineBraking = 0.3f;

	// Turbo/supercharger boost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	float TurboBoost = 0.0f;

	// Calculate power at given RPM
	float GetPowerAtRPM(float CurrentRPM) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TransmissionConfig
{
	GENERATED_BODY()

	// Gear ratios (index 0 = reverse, 1 = neutral, 2+ = forward gears)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	TArray<float> GearRatios;

	// Final drive ratio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	float FinalDriveRatio = 3.5f;

	// Time to shift gears (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	float ShiftTime = 0.2f;

	// Automatic transmission
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	bool bAutomatic = true;

	// Optimal shift up RPM percentage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	float ShiftUpRPMPercent = 0.9f;

	// Optimal shift down RPM percentage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	float ShiftDownRPMPercent = 0.4f;

	// Initialize default gear ratios
	void SetupDefaultGears(int32 NumGears = 6);

	// Get total ratio for current gear
	float GetTotalRatio(int32 CurrentGear) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SuspensionConfig
{
	GENERATED_BODY()

	// Spring stiffness (N/m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float SpringStiffness = 50000.0f;

	// Damper compression force (N*s/m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float DamperCompression = 5000.0f;

	// Damper rebound force (N*s/m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float DamperRebound = 5500.0f;

	// Maximum suspension travel (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float MaxTravel = 15.0f;

	// Natural rest position (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float NaturalRestPosition = 0.5f;

	// Anti-roll bar stiffness
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float AntiRollStiffness = 1000.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TireConfig
{
	GENERATED_BODY()

	// Tire type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire")
	EForge2TireType TireType = EForge2TireType::Medium;

	// Tire radius (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire")
	float Radius = 35.0f;

	// Tire width (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire")
	float Width = 25.0f;

	// Base grip coefficient
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire")
	float GripCoefficient = 1.0f;

	// Lateral grip multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire")
	float LateralGripMultiplier = 1.0f;

	// Optimal temperature (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire")
	float OptimalTemperature = 90.0f;

	// Temperature range for optimal grip (+/- degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire")
	float OptimalTemperatureRange = 15.0f;

	// Wear rate multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire")
	float WearRateMultiplier = 1.0f;

	// Calculate grip based on temperature and wear
	float CalculateGrip(float Temperature, float WearPercentage) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TireRuntimeState
{
	GENERATED_BODY()

	// Current temperature
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float Temperature = 25.0f;

	// Current wear percentage (0 = new, 100 = destroyed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float WearPercentage = 0.0f;

	// Current pressure (PSI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float Pressure = 32.0f;

	// Punctured
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bPunctured = false;

	// Get current condition
	EForge2TireCondition GetCondition() const;

	// Update tire state based on driving
	void Update(float DeltaTime, float SlipRatio, float SlipAngle, const FForge2TireConfig& Config);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AerodynamicsConfig
{
	GENERATED_BODY()

	// Drag coefficient
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float DragCoefficient = 0.35f;

	// Frontal area (m^2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float FrontalArea = 2.2f;

	// Downforce coefficient
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float DownforceCoefficient = 0.5f;

	// Front downforce balance (0-1, 0.5 = equal front/rear)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float DownforceBalance = 0.45f;

	// Calculate drag force at speed
	float CalculateDrag(float SpeedKph) const;

	// Calculate downforce at speed
	float CalculateDownforce(float SpeedKph) const;
};

// ============================================================================
// VEHICLE DEFINITION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VehicleDefinition
{
	GENERATED_BODY()

	// Unique vehicle ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString VehicleId;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText VehicleName;

	// Manufacturer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText Manufacturer;

	// Vehicle type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2VehicleType VehicleType = EForge2VehicleType::Car;

	// Drive type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2DriveType DriveType = EForge2DriveType::RearWheelDrive;

	// Handling type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EForge2HandlingType HandlingType = EForge2HandlingType::Simcade;

	// Vehicle mass (kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Mass = 1500.0f;

	// Center of mass offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	FVector CenterOfMassOffset = FVector::ZeroVector;

	// Number of wheels
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
	int32 NumWheels = 4;

	// Engine configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	FForge2EngineConfig Engine;

	// Transmission configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	FForge2TransmissionConfig Transmission;

	// Suspension configuration (per wheel)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	TArray<FForge2SuspensionConfig> Suspensions;

	// Tire configuration (per wheel)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tires")
	TArray<FForge2TireConfig> Tires;

	// Aerodynamics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	FForge2AerodynamicsConfig Aerodynamics;

	// Maximum steering angle (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float MaxSteeringAngle = 35.0f;

	// Steering speed (degrees per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float SteeringSpeed = 100.0f;

	// Brake force (N)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float MaxBrakeForce = 10000.0f;

	// Handbrake force (N)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float HandbrakeForce = 8000.0f;

	// Front brake bias (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float FrontBrakeBias = 0.6f;

	// Top speed (kph) - used for arcade handling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float TopSpeed = 300.0f;

	// Acceleration rating (0-10) - arcade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float AccelerationRating = 7.0f;

	// Handling rating (0-10) - arcade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float HandlingRating = 7.0f;

	// Boost strength multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float BoostStrength = 1.5f;

	// Nitro capacity (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float NitroCapacity = 5.0f;

	// Damage type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	EForge2DamageType DamageType = EForge2DamageType::Cosmetic;

	// Maximum health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float MaxHealth = 100.0f;

	// Mesh path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<USkeletalMesh> VehicleMesh;

	// Engine sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> EngineSound;

	// Initialize default suspensions and tires
	void SetupDefaultWheels();

	// Calculate theoretical top speed
	float CalculateTheoreticalTopSpeed() const;
};

// ============================================================================
// VEHICLE RUNTIME STATE
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComponentDamage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	EForge2VehicleComponent Component = EForge2VehicleComponent::Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamagePercentage = 0.0f;

	// Performance reduction based on damage
	float GetPerformanceMultiplier() const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VehicleRuntimeState
{
	GENERATED_BODY()

	// Current speed (kph)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentSpeedKph = 0.0f;

	// Current RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentRPM = 0.0f;

	// Current gear (0 = reverse, 1 = neutral, 2+ = forward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 CurrentGear = 1;

	// Throttle input (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float ThrottleInput = 0.0f;

	// Brake input (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float BrakeInput = 0.0f;

	// Steering input (-1 to 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float SteeringInput = 0.0f;

	// Handbrake active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bHandbrakeActive = false;

	// Nitro active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bNitroActive = false;

	// Remaining nitro (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float RemainingNitro = 0.0f;

	// Current health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentHealth = 100.0f;

	// Tire states
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TArray<FForge2TireRuntimeState> TireStates;

	// Component damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TArray<FForge2ComponentDamage> ComponentDamage;

	// Is vehicle operational
	bool IsOperational() const;

	// Apply damage to vehicle
	void ApplyDamage(float DamageAmount, EForge2VehicleComponent TargetComponent);
};

// ============================================================================
// TRACK SYSTEM
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Checkpoint
{
	GENERATED_BODY()

	// Checkpoint index
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 CheckpointIndex = 0;

	// World location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FVector Location = FVector::ZeroVector;

	// Checkpoint width
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float Width = 1000.0f;

	// Is start/finish line
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsStartFinish = false;

	// Optional time bonus (for checkpoint modes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float TimeBonus = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TrackSegment
{
	GENERATED_BODY()

	// Segment ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	int32 SegmentId = 0;

	// Spline points for this segment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	TArray<FVector> SplinePoints;

	// Track width at each point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	TArray<float> WidthAtPoints;

	// Surface type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	EForge2TrackSurface Surface = EForge2TrackSurface::Asphalt;

	// Optimal racing line (offset from center, -1 to 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<float> RacingLineOffset;

	// Recommended speed at each point (kph)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<float> RecommendedSpeed;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TrackDefinition
{
	GENERATED_BODY()

	// Track ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString TrackId;

	// Track name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText TrackName;

	// Track length (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float TrackLength = 5000.0f;

	// Number of laps for circuit mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	int32 DefaultLaps = 3;

	// Is circuit (closed loop)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIsCircuit = true;

	// Track segments
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FForge2TrackSegment> Segments;

	// Checkpoints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FForge2Checkpoint> Checkpoints;

	// Starting grid positions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FTransform> StartingGridPositions;

	// Pit lane entry location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector PitLaneEntry = FVector::ZeroVector;

	// Pit lane exit location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector PitLaneExit = FVector::ZeroVector;

	// Has pit lane
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	bool bHasPitLane = false;

	// Powerup spawn locations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	TArray<FVector> PowerupSpawnLocations;

	// Get segment at distance along track
	int32 GetSegmentAtDistance(float Distance) const;
};

// ============================================================================
// RACE MODE & CONFIGURATION
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RaceConfiguration
{
	GENERATED_BODY()

	// Race mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mode")
	EForge2RaceMode RaceMode = EForge2RaceMode::Circuit;

	// Number of laps (circuit mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mode")
	int32 NumLaps = 3;

	// Time limit (seconds, for time trial/checkpoint modes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mode")
	float TimeLimit = 300.0f;

	// Number of racers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racers")
	int32 NumRacers = 8;

	// Allow collisions between racers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	bool bAllowCollisions = true;

	// Enable damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	bool bEnableDamage = true;

	// Enable powerups
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	bool bEnablePowerups = true;

	// Elimination interval (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elimination")
	float EliminationInterval = 30.0f;

	// Points for positions (array index = position, value = points)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	TArray<int32> PositionPoints;

	// Drift scoring multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float DriftScoreMultiplier = 1.0f;

	// Initialize default position points
	void SetupDefaultScoring();
};

// ============================================================================
// POWERUP SYSTEM
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Powerup
{
	GENERATED_BODY()

	// Powerup type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerup")
	EForge2PowerupType PowerupType = EForge2PowerupType::SpeedBoost;

	// Duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerup")
	float Duration = 3.0f;

	// Effect magnitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerup")
	float Magnitude = 1.5f;

	// Number of uses
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerup")
	int32 Uses = 1;

	// Spawn probability weight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnWeight = 1.0f;

	// Position-based spawn bias (higher = more likely when behind)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float PositionBias = 0.0f;
};

// ============================================================================
// AI RACER
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AIRacerConfig
{
	GENERATED_BODY()

	// Difficulty level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	EForge2AIRacerDifficulty Difficulty = EForge2AIRacerDifficulty::Normal;

	// Rubber banding type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rubber Band")
	EForge2RubberBandType RubberBanding = EForge2RubberBandType::Subtle;

	// Skill level (0-1, affects racing line accuracy)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float SkillLevel = 0.7f;

	// Aggression level (0-1, affects overtaking/blocking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Aggression = 0.5f;

	// Consistency (0-1, affects lap time variance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Consistency = 0.8f;

	// Maximum speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MaxSpeedMultiplier = 1.0f;

	// Acceleration multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AccelerationMultiplier = 1.0f;

	// Will use powerups
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bUsePowerups = true;

	// Will attempt drafting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bUseDrafting = true;

	// Calculate rubber band boost based on position
	float GetRubberBandBoost(int32 CurrentPosition, int32 TotalRacers, float DistanceBehindLeader) const;
};

// ============================================================================
// RACE RESULTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LapTime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	int32 LapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float LapTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	TArray<float> SectorTimes;

	// Format as string (mm:ss.mmm)
	FString GetFormattedTime() const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RacerResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	FString RacerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	FText RacerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 FinishPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float TotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FForge2LapTime> LapTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FForge2LapTime BestLap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 PointsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 OvertakesMade = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TotalDriftScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bDidNotFinish = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FString DNFReason;
};

// ============================================================================
// GHOST DATA (For Time Trials)
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GhostFrame
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame")
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame")
	FTransform VehicleTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame")
	float WheelRotation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame")
	float SteeringAngle = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GhostData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	float LapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TArray<FForge2GhostFrame> Frames;

	// Recording interval (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float RecordingInterval = 0.033f;

	// Get interpolated transform at time
	FTransform GetTransformAtTime(float Time) const;
};

// ============================================================================
// RACING MANAGER
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2RacingManager
{
public:
	// Vehicle management
	bool RegisterVehicle(const FForge2VehicleDefinition& Vehicle);
	FForge2VehicleDefinition* GetVehicle(const FString& VehicleId);
	TArray<FString> GetAllVehicleIds() const;

	// Track management
	bool RegisterTrack(const FForge2TrackDefinition& Track);
	FForge2TrackDefinition* GetTrack(const FString& TrackId);
	TArray<FString> GetAllTrackIds() const;

	// Powerup management
	void RegisterPowerup(const FForge2Powerup& Powerup);
	FForge2Powerup* GetPowerup(EForge2PowerupType Type);
	EForge2PowerupType RollPowerup(int32 CurrentPosition, int32 TotalRacers) const;

	// Race management
	bool StartRace(const FString& TrackId, const FForge2RaceConfiguration& Config);
	void EndRace();
	bool IsRaceActive() const { return bRaceActive; }

	// Racer management
	bool AddRacer(const FString& RacerId, const FString& VehicleId, bool bIsAI, const FForge2AIRacerConfig& AIConfig = FForge2AIRacerConfig());
	void UpdateRacerPosition(const FString& RacerId, int32 CurrentCheckpoint, int32 CurrentLap, float DistanceAlongTrack);
	int32 GetRacerPosition(const FString& RacerId) const;
	void RecordLapTime(const FString& RacerId, const FForge2LapTime& LapTime);
	void EliminateRacer(const FString& RacerId, const FString& Reason);

	// Ghost data
	void StartGhostRecording(const FString& RacerId);
	void RecordGhostFrame(const FString& RacerId, const FForge2GhostFrame& Frame);
	FForge2GhostData StopGhostRecording(const FString& RacerId);
	bool SaveGhostData(const FForge2GhostData& GhostData);
	TArray<FForge2GhostData> LoadGhostData(const FString& TrackId, int32 MaxCount = 10);

	// Results
	TArray<FForge2RacerResult> GetRaceResults() const;

	// Clear all data
	void Reset();

private:
	TMap<FString, FForge2VehicleDefinition> Vehicles;
	TMap<FString, FForge2TrackDefinition> Tracks;
	TMap<EForge2PowerupType, FForge2Powerup> Powerups;

	bool bRaceActive = false;
	FString ActiveTrackId;
	FForge2RaceConfiguration ActiveConfig;

	struct FRacerData
	{
		FString VehicleId;
		bool bIsAI;
		FForge2AIRacerConfig AIConfig;
		int32 CurrentCheckpoint;
		int32 CurrentLap;
		float DistanceAlongTrack;
		int32 CurrentPosition;
		TArray<FForge2LapTime> LapTimes;
		bool bEliminated;
		FString EliminationReason;
		TArray<FForge2GhostFrame> GhostFrames;
		bool bRecordingGhost;
	};
	TMap<FString, FRacerData> Racers;

	void UpdatePositions();
	float CalculateTotalDistance(const FRacerData& Racer) const;
};

// ============================================================================
// RACING UTILITIES
// ============================================================================

class MCPAUTOMATIONBRIDGE_API FForge2RacingUtils
{
public:
	// Create common vehicle presets
	static FForge2VehicleDefinition CreateSportsCar(const FString& VehicleId, const FText& Name);
	static FForge2VehicleDefinition CreateMuscleCar(const FString& VehicleId, const FText& Name);
	static FForge2VehicleDefinition CreateKart(const FString& VehicleId, const FText& Name);
	static FForge2VehicleDefinition CreateMotorcycle(const FString& VehicleId, const FText& Name);

	// Create common powerup sets
	static TArray<FForge2Powerup> CreateArcadePowerups();
	static TArray<FForge2Powerup> CreateCombatRacingPowerups();

	// Track generation helpers
	static FForge2TrackDefinition GenerateOvalTrack(const FString& TrackId, const FText& Name, float Length, float Width);
	static FForge2TrackDefinition GenerateCircuitFromSpline(const FString& TrackId, const FText& Name, const TArray<FVector>& SplinePoints, float TrackWidth);

	// Physics helpers
	static float CalculateSlipAngle(const FVector& Velocity, const FVector& ForwardVector);
	static float CalculateSlipRatio(float WheelSpeed, float VehicleSpeed);
	static FVector CalculateTireForce(float SlipAngle, float SlipRatio, float NormalForce, float GripCoefficient);

	// Time formatting
	static FString FormatLapTime(float TimeSeconds);
	static FString FormatDelta(float DeltaSeconds);
};
