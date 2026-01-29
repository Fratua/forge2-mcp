// Forge2LumenSystem.h - Lumen Global Illumination System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2LumenSystem.generated.h"

// Forward declarations
class UPostProcessComponent;
class APostProcessVolume;
class UDirectionalLightComponent;
class USkyLightComponent;

// Enums
UENUM(BlueprintType)
enum class EForge2LumenQuality : uint8
{
    Low,
    Medium,
    High,
    Ultra,
    Cinematic
};

UENUM(BlueprintType)
enum class EForge2LumenReflectionMethod : uint8
{
    ScreenSpace,
    HardwareRayTracing,
    SoftwareRayTracing,
    Hybrid
};

UENUM(BlueprintType)
enum class EForge2LumenGIMethod : uint8
{
    ScreenTraces,
    HardwareRayTracing,
    SoftwareRayTracing,
    Hybrid
};

UENUM(BlueprintType)
enum class EForge2LumenDetailTracing : uint8
{
    None,
    DetailTracesOnly,
    Full
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LumenGISettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2LumenGIMethod Method = EForge2LumenGIMethod::SoftwareRayTracing;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LumenSceneViewDistance = 20000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LumenSceneDetail = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FinalGatherQuality = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxTraceDistance = 20000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RayLightingMode = 0;  // 0 = Surface Cache, 1 = Hit Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFullSkylightLeakingRemoval = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SurfaceCacheResolution = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LumenReflectionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2LumenReflectionMethod Method = EForge2LumenReflectionMethod::SoftwareRayTracing;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReflectionQuality = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RayLightingQuality = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxRoughnessToTrace = 0.4f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxReflectionBounces = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHighQualityTranslucencyReflections = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReflectionConvolutionScale = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LumenEmissiveSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEmissiveLightingEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EmissiveBoost = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EmissiveIntensityThreshold = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bTreatEmissiveAsLight = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EmissiveLightRadius = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor EmissiveTint = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LumenPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RayBudget = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ScreenProbeGatherOctahedronResolution = 8.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ScreenProbeTracingOctahedronResolution = 8.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2LumenDetailTracing DetailTracing = EForge2LumenDetailTracing::DetailTracesOnly;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCompressSceneCards = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SurfaceCacheUpdateDelay = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxSurfaceCacheUpdatesPerFrame = 16;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LumenSceneStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) float GIRenderTime = 0.0f;
    UPROPERTY(BlueprintReadOnly) float ReflectionRenderTime = 0.0f;
    UPROPERTY(BlueprintReadOnly) int32 SurfaceCacheMemoryMB = 0;
    UPROPERTY(BlueprintReadOnly) int32 CardCount = 0;
    UPROPERTY(BlueprintReadOnly) int32 MeshCardCount = 0;
    UPROPERTY(BlueprintReadOnly) int32 ActiveRayCount = 0;
    UPROPERTY(BlueprintReadOnly) float CoveragePercentage = 0.0f;

    void Reset();
    void UpdateFromScene();
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LumenVolumeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bOverrideLumenSettings = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendWeight = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Priority = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2LumenGISettings GISettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2LumenReflectionSettings ReflectionSettings;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkyLightLumenSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRealTimeCapture = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCastShadows = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CloudAmbientOcclusionStrength = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Intensity = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor LightColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DirectionalLightLumenSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCastRayTracedShadows = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ShadowRayTracingBias = 0.01f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LumenSourceRadius = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAtmosphereSunLight = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CloudScatteringLuminanceScale = 1.0f;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2LumenManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2LumenManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2LumenGISettings GISettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2LumenReflectionSettings ReflectionSettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2LumenEmissiveSettings EmissiveSettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2LumenPerformanceSettings PerformanceSettings;
    UPROPERTY(BlueprintReadOnly) FForge2LumenSceneStats SceneStats;

    // GI Configuration
    UFUNCTION(BlueprintCallable, Category = "Lumen|GI")
    void ConfigureLumenGI(const FForge2LumenGISettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lumen|GI")
    void SetGIQuality(EForge2LumenQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Lumen|GI")
    void SetGIMethod(EForge2LumenGIMethod Method);

    UFUNCTION(BlueprintCallable, Category = "Lumen|GI")
    void SetSceneViewDistance(float Distance);

    // Reflection Configuration
    UFUNCTION(BlueprintCallable, Category = "Lumen|Reflections")
    void ConfigureLumenReflections(const FForge2LumenReflectionSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Reflections")
    void SetReflectionMethod(EForge2LumenReflectionMethod Method);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Reflections")
    void SetReflectionQuality(float Quality);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Reflections")
    void SetMaxRoughnessToTrace(float MaxRoughness);

    // Emissive Lighting
    UFUNCTION(BlueprintCallable, Category = "Lumen|Emissive")
    void ConfigureEmissiveLighting(const FForge2LumenEmissiveSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Emissive")
    void SetEmissiveBoost(float Boost);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Emissive")
    void EnableEmissiveAsLight(bool bEnable);

    // Performance
    UFUNCTION(BlueprintCallable, Category = "Lumen|Performance")
    void ConfigurePerformance(const FForge2LumenPerformanceSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Performance")
    void SetRayBudget(float Budget);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Performance")
    void OptimizeForPlatform(bool bIsMobile, bool bIsConsole);

    // Scene Management
    UFUNCTION(BlueprintCallable, Category = "Lumen|Scene")
    void InvalidateSurfaceCache();

    UFUNCTION(BlueprintCallable, Category = "Lumen|Scene")
    void ForceUpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Lumen|Scene")
    FForge2LumenSceneStats GetSceneStats();

    // Light Configuration
    UFUNCTION(BlueprintCallable, Category = "Lumen|Lights")
    void ConfigureSkyLight(USkyLightComponent* SkyLight, const FForge2SkyLightLumenSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Lights")
    void ConfigureDirectionalLight(UDirectionalLightComponent* DirLight, const FForge2DirectionalLightLumenSettings& Settings);

    // Post Process Volumes
    UFUNCTION(BlueprintCallable, Category = "Lumen|Volumes")
    void ConfigurePostProcessVolume(APostProcessVolume* Volume, const FForge2LumenVolumeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lumen|Volumes")
    APostProcessVolume* CreateLumenVolume(const FTransform& Transform, const FForge2LumenVolumeSettings& Settings);

    // Quality Presets
    UFUNCTION(BlueprintCallable, Category = "Lumen|Presets")
    void ApplyQualityPreset(EForge2LumenQuality Quality);

    UFUNCTION(BlueprintPure, Category = "Lumen|Presets")
    FForge2LumenGISettings GetGIPreset(EForge2LumenQuality Quality) const;

    UFUNCTION(BlueprintPure, Category = "Lumen|Presets")
    FForge2LumenReflectionSettings GetReflectionPreset(EForge2LumenQuality Quality) const;

private:
    void UpdateSceneStats();
    void ApplyGISettings();
    void ApplyReflectionSettings();
    void ApplyPerformanceSettings();
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2LumenUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Lumen|Utility")
    static FForge2LumenGISettings CreateDefaultGISettings();

    UFUNCTION(BlueprintCallable, Category = "Lumen|Utility")
    static FForge2LumenGISettings CreateHighQualityGISettings();

    UFUNCTION(BlueprintCallable, Category = "Lumen|Utility")
    static FForge2LumenGISettings CreatePerformanceGISettings();

    UFUNCTION(BlueprintCallable, Category = "Lumen|Utility")
    static FForge2LumenReflectionSettings CreateDefaultReflectionSettings();

    UFUNCTION(BlueprintCallable, Category = "Lumen|Utility")
    static FForge2LumenReflectionSettings CreateHighQualityReflectionSettings();

    UFUNCTION(BlueprintCallable, Category = "Lumen|Utility")
    static FForge2LumenPerformanceSettings CreateDefaultPerformanceSettings();

    UFUNCTION(BlueprintCallable, Category = "Lumen|Utility")
    static FForge2LumenPerformanceSettings CreateConsolePerformanceSettings();

    UFUNCTION(BlueprintPure, Category = "Lumen|Utility")
    static bool IsHardwareRayTracingSupported();

    UFUNCTION(BlueprintPure, Category = "Lumen|Utility")
    static bool IsLumenSupported();

    UFUNCTION(BlueprintPure, Category = "Lumen|Utility")
    static float EstimateGPUMemoryUsageMB(const FForge2LumenGISettings& GISettings, const FForge2LumenReflectionSettings& ReflectionSettings);
};
