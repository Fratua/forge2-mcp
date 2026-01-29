// Forge2VFXSystem.h - Niagara Visual Effects System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2VFXSystem.generated.h"

// Forward declarations
class UNiagaraSystem;
class UNiagaraComponent;
class UNiagaraEmitter;

// Enums
UENUM(BlueprintType)
enum class EForge2ParticleRenderType : uint8
{
    Sprite,
    Ribbon,
    Mesh,
    Light,
    Component
};

UENUM(BlueprintType)
enum class EForge2SpawnMode : uint8
{
    Continuous,
    Burst,
    BurstInstant,
    OnEvent
};

UENUM(BlueprintType)
enum class EForge2VFXCategory : uint8
{
    Combat,
    Environment,
    Magic,
    Fire,
    Water,
    Electricity,
    Smoke,
    Debris,
    Blood,
    UI,
    Custom
};

UENUM(BlueprintType)
enum class EForge2ForceType : uint8
{
    Drag,
    Noise,
    Curl,
    Vortex,
    PointAttractor,
    Vector,
    Gravity
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ParticleSpawnSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2SpawnMode Mode = EForge2SpawnMode::Continuous;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpawnRate = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BurstCount = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BurstInterval = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxParticles = 1000;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector SpawnLocation = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpawnRadius = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRandomizeSpawn = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ParticleAttributes
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D LifetimeRange = FVector2D(1.0f, 2.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D SizeRange = FVector2D(10.0f, 20.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ColorStart = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ColorEnd = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AlphaStart = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AlphaEnd = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D VelocityRange = FVector2D(100.0f, 200.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector VelocityDirection = FVector(0, 0, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VelocityConeAngle = 30.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D RotationRange = FVector2D(0.0f, 360.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RotationRate = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Mass = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ParticleForce
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2ForceType Type = EForge2ForceType::Drag;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Strength = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Direction = FVector(0, 0, -1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NoiseFrequency = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NoiseAmplitude = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector AttractorLocation = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttractorRadius = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ParticleCollision
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bKillOnCollision = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Restitution = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Friction = 0.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSpawnOnCollision = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SpawnSystemOnCollision;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CollisionSpawnCount = 3;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SpriteSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Texture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SubImageRows = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SubImageColumns = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SubImageAnimationRate = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRandomSubImage = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFacingCamera = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SortOffset = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RibbonSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Width = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxSegments = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFaceCamera = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bTwist = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TwistAmount = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float UVTiling = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float UVScrollSpeed = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MeshParticleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UStaticMesh> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UMaterialInterface> OverrideMaterial;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableLOD = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LODDistanceScale = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCastShadows = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableMotionBlur = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VFXPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2VFXCategory Category = EForge2VFXCategory::Custom;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString PresetName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2ParticleSpawnSettings SpawnSettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2ParticleAttributes Attributes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2ParticleForce> Forces;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2ParticleCollision Collision;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2ParticleRenderType RenderType = EForge2ParticleRenderType::Sprite;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2SpriteSettings SpriteSettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration = 0.0f;  // 0 = infinite
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2VFXManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2VFXManager();

    // System Creation
    UFUNCTION(BlueprintCallable, Category = "VFX|Creation")
    UNiagaraSystem* CreateNiagaraSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "VFX|Creation")
    UNiagaraComponent* SpawnSystemAtLocation(UNiagaraSystem* System, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "VFX|Creation")
    UNiagaraComponent* SpawnSystemAttached(UNiagaraSystem* System, USceneComponent* AttachTo,
                                            FName SocketName = NAME_None);

    // Emitter Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX|Emitter")
    void AddEmitter(UNiagaraSystem* System, const FString& EmitterName);

    UFUNCTION(BlueprintCallable, Category = "VFX|Emitter")
    void ConfigureSpawn(UNiagaraSystem* System, const FString& EmitterName,
                         const FForge2ParticleSpawnSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX|Emitter")
    void ConfigureAttributes(UNiagaraSystem* System, const FString& EmitterName,
                              const FForge2ParticleAttributes& Attributes);

    UFUNCTION(BlueprintCallable, Category = "VFX|Emitter")
    void AddForce(UNiagaraSystem* System, const FString& EmitterName,
                   const FForge2ParticleForce& Force);

    UFUNCTION(BlueprintCallable, Category = "VFX|Emitter")
    void ConfigureCollision(UNiagaraSystem* System, const FString& EmitterName,
                             const FForge2ParticleCollision& Collision);

    // Renderer Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX|Renderer")
    void ConfigureSpriteRenderer(UNiagaraSystem* System, const FString& EmitterName,
                                   const FForge2SpriteSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX|Renderer")
    void ConfigureRibbonRenderer(UNiagaraSystem* System, const FString& EmitterName,
                                   const FForge2RibbonSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX|Renderer")
    void ConfigureMeshRenderer(UNiagaraSystem* System, const FString& EmitterName,
                                 const FForge2MeshParticleSettings& Settings);

    // VFX Events
    UFUNCTION(BlueprintCallable, Category = "VFX|Events")
    void CreateImpactVFX(FVector Location, FVector Normal, const FString& SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Events")
    void CreateMuzzleFlashVFX(FVector Location, FRotator Direction, const FString& WeaponType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Events")
    void CreateBloodSplatterVFX(FVector Location, FVector Direction, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Events")
    void CreateExplosionVFX(FVector Location, float Radius, const FString& ExplosionType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Events")
    void CreateFootstepVFX(FVector Location, const FString& SurfaceType);

    // Pooling
    UFUNCTION(BlueprintCallable, Category = "VFX|Pooling")
    void InitializePool(UNiagaraSystem* System, int32 PoolSize);

    UFUNCTION(BlueprintCallable, Category = "VFX|Pooling")
    UNiagaraComponent* GetFromPool(UNiagaraSystem* System);

    UFUNCTION(BlueprintCallable, Category = "VFX|Pooling")
    void ReturnToPool(UNiagaraComponent* Component);

    // Presets
    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    UNiagaraSystem* CreateFromPreset(const FForge2VFXPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    FForge2VFXPreset GetFirePreset();

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    FForge2VFXPreset GetSmokePreset();

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    FForge2VFXPreset GetSparkPreset();

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    FForge2VFXPreset GetDustPreset();

    UFUNCTION(BlueprintCallable, Category = "VFX|Presets")
    FForge2VFXPreset GetMagicPreset(FLinearColor Color);

private:
    UPROPERTY() TMap<UNiagaraSystem*, TArray<UNiagaraComponent*>> VFXPool;
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2VFXUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static FForge2ParticleSpawnSettings CreateContinuousSpawn(float Rate, int32 MaxParticles);

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static FForge2ParticleSpawnSettings CreateBurstSpawn(int32 Count, int32 BurstCount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static FForge2ParticleAttributes CreateDefaultAttributes();

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static FForge2ParticleForce CreateGravityForce(float Strength);

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static FForge2ParticleForce CreateDragForce(float Strength);

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static FForge2ParticleForce CreateNoiseForce(float Amplitude, float Frequency);

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static FForge2ParticleForce CreateVortexForce(FVector Center, float Strength);
};
