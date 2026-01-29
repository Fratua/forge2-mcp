// Forge2ChaosSystem.h - Chaos Physics & Destruction System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2ChaosSystem.generated.h"

// Forward declarations
class UGeometryCollection;
class UGeometryCollectionComponent;
class UStaticMesh;
class USkeletalMeshComponent;

// Enums
UENUM(BlueprintType)
enum class EForge2FractureMode : uint8
{
    Uniform,
    Voronoi,
    Clustered,
    Radial,
    Plane,
    Brick,
    Custom
};

UENUM(BlueprintType)
enum class EForge2CollisionType : uint8
{
    None,
    Particle,
    ImplicitBox,
    ImplicitSphere,
    ImplicitCapsule,
    LevelSet
};

UENUM(BlueprintType)
enum class EForge2DamageType : uint8
{
    None,
    Impact,
    Explosion,
    Strain,
    Custom
};

UENUM(BlueprintType)
enum class EForge2DestructionLevel : uint8
{
    None,
    Minor,
    Moderate,
    Major,
    Total
};

UENUM(BlueprintType)
enum class EForge2ClothPreset : uint8
{
    Silk,
    Cotton,
    Leather,
    Rubber,
    Heavy,
    Custom
};

UENUM(BlueprintType)
enum class EForge2VehicleType : uint8
{
    Car,
    Motorcycle,
    Truck,
    Tank,
    Hovercraft,
    Custom
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FractureSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2FractureMode Mode = EForge2FractureMode::Voronoi;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 NumPieces = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RandomSeed = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinSize = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Grout = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Noise = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAddInteriorMaterial = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 InteriorMaterialIndex = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector ClusterCenter = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ClusterRadius = 100.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableDestruction = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DamageThreshold = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> LevelThresholds;  // Per-level damage thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxBreakingForce = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ClusterBreakingStrain = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bGenerateDebris = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DebrisLifespan = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxDebrisCount = 50;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSpawnEffectsOnBreak = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bPlaySoundOnBreak = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PhysicsFieldSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Direction = FVector(0, 0, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Magnitude = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Falloff = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bKillField = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSleepField = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ClothSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2ClothPreset Preset = EForge2ClothPreset::Cotton;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Mass = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float StretchStiffness = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BendStiffness = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ShearStiffness = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damping = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Friction = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WindDrag = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WindLift = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GravityScale = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableSelfCollision = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SelfCollisionRadius = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SolverIterations = 8;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VehicleWheelSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 35.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Width = 20.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Mass = 20.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LateralStiffness = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LongitudinalStiffness = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxBrakeTorque = 1500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxHandBrakeTorque = 3000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxSteerAngle = 45.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAffectedByHandbrake = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAffectedBySteering = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAffectedByEngine = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VehicleSuspensionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxCompression = 20.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxDroop = 20.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpringRate = 25000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpringPreload = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DampingRatio = 0.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CompressionDamping = 2500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReboundDamping = 2500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector WheelOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VehicleEngineSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxTorque = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxRPM = 6000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float IdleRPM = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EngineInertia = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EngineBrakeFactor = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> TorqueCurve;  // RPM -> Torque multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> GearRatios;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FinalDriveRatio = 3.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AutomaticShiftUpRPM = 5500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AutomaticShiftDownRPM = 2000.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VehicleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2VehicleType VehicleType = EForge2VehicleType::Car;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Mass = 1500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector CenterOfMass = FVector(0, 0, -20);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2VehicleEngineSettings Engine;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2VehicleWheelSettings> Wheels;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2VehicleSuspensionSettings> Suspensions;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DragCoefficient = 0.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DownforceCoefficient = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NetworkedPhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableNetworkedPhysics = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PredictionTimeMs = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CorrectionSmoothTime = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PositionErrorThreshold = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RotationErrorThreshold = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VelocityErrorThreshold = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableRewind = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 HistoryBufferSize = 60;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2ChaosManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2ChaosManager();

    // Destructible Creation
    UFUNCTION(BlueprintCallable, Category = "Chaos|Destruction")
    UGeometryCollection* CreateDestructible(UStaticMesh* SourceMesh, const FForge2FractureSettings& FractureSettings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Destruction")
    void ConfigureDestructionLevels(UGeometryCollectionComponent* Component,
                                      const FForge2DestructionSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Destruction")
    void SetDamageThresholds(UGeometryCollectionComponent* Component, TArray<float> Thresholds);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Destruction")
    void ApplyDamage(UGeometryCollectionComponent* Component, FVector Location,
                      float Damage, float Radius, EForge2DamageType DamageType);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Destruction")
    void ApplyImpulse(UGeometryCollectionComponent* Component, FVector Location,
                       FVector Impulse, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Destruction")
    EForge2DestructionLevel GetCurrentDestructionLevel(UGeometryCollectionComponent* Component);

    // Debris Management
    UFUNCTION(BlueprintCallable, Category = "Chaos|Debris")
    void ConfigureDebris(UGeometryCollectionComponent* Component, float Lifespan,
                          int32 MaxCount, bool bFadeOut);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Debris")
    void CleanupDebris(UGeometryCollectionComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Debris")
    void SetDebrisPhysicsProperties(UGeometryCollectionComponent* Component,
                                      float Mass, float Friction, float Restitution);

    // Physics Fields
    UFUNCTION(BlueprintCallable, Category = "Chaos|Fields")
    AActor* CreatePhysicsField(FVector Location, const FForge2PhysicsFieldSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Fields")
    void UpdatePhysicsField(AActor* FieldActor, const FForge2PhysicsFieldSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Fields")
    AActor* CreateExplosionField(FVector Location, float Radius, float Magnitude, float Duration);

    // Cloth Simulation
    UFUNCTION(BlueprintCallable, Category = "Chaos|Cloth")
    void ConfigureCloth(USkeletalMeshComponent* MeshComponent, const FForge2ClothSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Cloth")
    void ApplyClothPreset(USkeletalMeshComponent* MeshComponent, EForge2ClothPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Cloth")
    void SetClothWind(FVector WindVelocity);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Cloth")
    void EnableClothSimulation(USkeletalMeshComponent* MeshComponent, bool bEnable);

    // Vehicle Physics
    UFUNCTION(BlueprintCallable, Category = "Chaos|Vehicle")
    void SetupChaosVehicle(AActor* VehicleActor, const FForge2VehicleSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Vehicle")
    void ConfigureVehicleWheel(AActor* VehicleActor, int32 WheelIndex,
                                 const FForge2VehicleWheelSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Vehicle")
    void ConfigureVehicleSuspension(AActor* VehicleActor, int32 WheelIndex,
                                      const FForge2VehicleSuspensionSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Vehicle")
    void ConfigureVehicleEngine(AActor* VehicleActor, const FForge2VehicleEngineSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Vehicle")
    void SetVehicleInput(AActor* VehicleActor, float Throttle, float Steering, float Brake, bool bHandbrake);

    // Networked Physics
    UFUNCTION(BlueprintCallable, Category = "Chaos|Network")
    void ConfigureNetworkedPhysics(AActor* Actor, const FForge2NetworkedPhysicsSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Network")
    void EnablePhysicsPrediction(AActor* Actor, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Network")
    void SyncPhysicsState(AActor* Actor, FVector Position, FRotator Rotation, FVector LinearVelocity, FVector AngularVelocity);

private:
    FForge2ClothSettings GetClothPresetSettings(EForge2ClothPreset Preset);
    void SetupVehicleWheels(AActor* VehicleActor, const FForge2VehicleSettings& Settings);
    
    // Cached state
    FVector CachedWindVelocity = FVector::ZeroVector;
    FForge2NetworkedPhysicsSettings CachedNetworkSettings;
    int32 DebrisMaxCount = 50;
    float DebrisLifespan = 5.0f;
    bool bDebrisFadeOut = true;
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2ChaosUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2FractureSettings CreateVoronoiFracture(int32 NumPieces, float MinSize);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2FractureSettings CreateClusteredFracture(FVector Center, float Radius, int32 NumPieces);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2FractureSettings CreateRadialFracture(FVector Center, int32 NumRings, int32 NumSegments);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2DestructionSettings CreateDefaultDestruction();

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2DestructionSettings CreateProgressiveDestruction(int32 NumLevels);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2ClothSettings CreateClothFromPreset(EForge2ClothPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2VehicleSettings CreateDefaultCarSettings();

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2VehicleSettings CreateDefaultTruckSettings();

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2VehicleSettings CreateDefaultMotorcycleSettings();

    UFUNCTION(BlueprintCallable, Category = "Chaos|Utility")
    static FForge2NetworkedPhysicsSettings CreateDefaultNetworkSettings();
};
