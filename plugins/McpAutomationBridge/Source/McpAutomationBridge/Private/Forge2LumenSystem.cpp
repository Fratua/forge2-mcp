// Forge2LumenSystem.cpp - Lumen Global Illumination System Implementation
#include "Forge2LumenSystem.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "RHI.h"
#include "RHIDefinitions.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

// FForge2LumenSceneStats Implementation
void FForge2LumenSceneStats::Reset()
{
    GIRenderTime = 0.0f;
    ReflectionRenderTime = 0.0f;
    SurfaceCacheMemoryMB = 0;
    CardCount = 0;
    MeshCardCount = 0;
    ActiveRayCount = 0;
    CoveragePercentage = 0.0f;
}

void FForge2LumenSceneStats::UpdateFromScene()
{
    // Query Lumen stats from GPU profiling data
#if STATS
    // Get render time from stat system
    double GITime = 0.0;
    double ReflectionTime = 0.0;
    
    // Query frame timing stats
    if (IConsoleVariable* GITimeVar = IConsoleManager::Get().FindConsoleVariable(TEXT("stat.Lumen.GI")))
    {
        GIRenderTime = GITimeVar->GetFloat();
    }
    
    if (IConsoleVariable* ReflectionTimeVar = IConsoleManager::Get().FindConsoleVariable(TEXT("stat.Lumen.Reflections")))
    {
        ReflectionRenderTime = ReflectionTimeVar->GetFloat();
    }
    
    // Query surface cache memory usage
    if (IConsoleVariable* SurfaceCacheMemVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.SurfaceCache.MemoryMB")))
    {
        SurfaceCacheMemoryMB = SurfaceCacheMemVar->GetInt();
    }
#endif
    
    // Estimate card counts based on scene complexity
    if (GWorld)
    {
        int32 MeshCount = 0;
        for (TActorIterator<AActor> It(GWorld); It; ++It)
        {
            if ((*It)->GetComponentByClass(UStaticMeshComponent::StaticClass()))
            {
                MeshCount++;
            }
        }
        MeshCardCount = MeshCount;
        CardCount = MeshCount * 6; // Approximate - 6 cards per mesh
    }
}

// UForge2LumenManager Implementation
UForge2LumenManager::UForge2LumenManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    GISettings = UForge2LumenUtility::CreateDefaultGISettings();
    ReflectionSettings = UForge2LumenUtility::CreateDefaultReflectionSettings();
    PerformanceSettings = UForge2LumenUtility::CreateDefaultPerformanceSettings();
}

void UForge2LumenManager::ConfigureLumenGI(const FForge2LumenGISettings& Settings)
{
    GISettings = Settings;
    ApplyGISettings();
}

void UForge2LumenManager::SetGIQuality(EForge2LumenQuality Quality)
{
    GISettings = GetGIPreset(Quality);
    ApplyGISettings();
}

void UForge2LumenManager::SetGIMethod(EForge2LumenGIMethod Method)
{
    GISettings.Method = Method;
    ApplyGISettings();
}

void UForge2LumenManager::SetSceneViewDistance(float Distance)
{
    GISettings.LumenSceneViewDistance = Distance;
    ApplyGISettings();
}

void UForge2LumenManager::ConfigureLumenReflections(const FForge2LumenReflectionSettings& Settings)
{
    ReflectionSettings = Settings;
    ApplyReflectionSettings();
}

void UForge2LumenManager::SetReflectionMethod(EForge2LumenReflectionMethod Method)
{
    ReflectionSettings.Method = Method;
    ApplyReflectionSettings();
}

void UForge2LumenManager::SetReflectionQuality(float Quality)
{
    ReflectionSettings.ReflectionQuality = FMath::Clamp(Quality, 0.0f, 4.0f);
    ApplyReflectionSettings();
}

void UForge2LumenManager::SetMaxRoughnessToTrace(float MaxRoughness)
{
    ReflectionSettings.MaxRoughnessToTrace = FMath::Clamp(MaxRoughness, 0.0f, 1.0f);
    ApplyReflectionSettings();
}

void UForge2LumenManager::ConfigureEmissiveLighting(const FForge2LumenEmissiveSettings& Settings)
{
    EmissiveSettings = Settings;
    // Apply emissive settings to relevant post process volumes
}

void UForge2LumenManager::SetEmissiveBoost(float Boost)
{
    EmissiveSettings.EmissiveBoost = FMath::Max(0.0f, Boost);
}

void UForge2LumenManager::EnableEmissiveAsLight(bool bEnable)
{
    EmissiveSettings.bTreatEmissiveAsLight = bEnable;
}

void UForge2LumenManager::ConfigurePerformance(const FForge2LumenPerformanceSettings& Settings)
{
    PerformanceSettings = Settings;
    ApplyPerformanceSettings();
}

void UForge2LumenManager::SetRayBudget(float Budget)
{
    PerformanceSettings.RayBudget = FMath::Clamp(Budget, 0.25f, 4.0f);
    ApplyPerformanceSettings();
}

void UForge2LumenManager::OptimizeForPlatform(bool bIsMobile, bool bIsConsole)
{
    if (bIsMobile)
    {
        // Lumen not supported on mobile - use fallback
        GISettings.bEnabled = false;
        ReflectionSettings.bEnabled = false;
    }
    else if (bIsConsole)
    {
        PerformanceSettings = UForge2LumenUtility::CreateConsolePerformanceSettings();
        GISettings.Method = EForge2LumenGIMethod::SoftwareRayTracing;
        ReflectionSettings.Method = EForge2LumenReflectionMethod::SoftwareRayTracing;
        GISettings.FinalGatherQuality = 0.5f;
        ReflectionSettings.ReflectionQuality = 0.5f;
    }
    else
    {
        // PC - check for hardware ray tracing support
        if (UForge2LumenUtility::IsHardwareRayTracingSupported())
        {
            GISettings.Method = EForge2LumenGIMethod::HardwareRayTracing;
            ReflectionSettings.Method = EForge2LumenReflectionMethod::HardwareRayTracing;
        }
    }

    ApplyGISettings();
    ApplyReflectionSettings();
    ApplyPerformanceSettings();
}

void UForge2LumenManager::InvalidateSurfaceCache()
{
    // Force Lumen surface cache invalidation via console command
    if (IConsoleVariable* InvalidateVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.SurfaceCache.Reset")))
    {
        InvalidateVar->Set(1);
    }
    
    // Trigger rebuild of Lumen scene
    if (GWorld)
    {
        GWorld->Scene->UpdateAllPrimitiveSceneInfos();
    }
}

void UForge2LumenManager::ForceUpdateLighting()
{
    InvalidateSurfaceCache();
    UpdateSceneStats();
}

FForge2LumenSceneStats UForge2LumenManager::GetSceneStats()
{
    UpdateSceneStats();
    return SceneStats;
}

void UForge2LumenManager::ConfigureSkyLight(USkyLightComponent* SkyLight,
                                             const FForge2SkyLightLumenSettings& Settings)
{
    if (!SkyLight) return;

    SkyLight->SetIntensity(Settings.Intensity);
    SkyLight->SetLightColor(Settings.LightColor);
    SkyLight->bRealTimeCapture = Settings.bRealTimeCapture;
    SkyLight->bCastShadows = Settings.bCastShadows;
    SkyLight->CloudAmbientOcclusionStrength = Settings.CloudAmbientOcclusionStrength;

    SkyLight->MarkRenderStateDirty();
}

void UForge2LumenManager::ConfigureDirectionalLight(UDirectionalLightComponent* DirLight,
                                                      const FForge2DirectionalLightLumenSettings& Settings)
{
    if (!DirLight) return;

    DirLight->bCastRaytracedShadow = Settings.bCastRayTracedShadows;
    DirLight->bAtmosphereSunLight = Settings.bAtmosphereSunLight;
    DirLight->CloudScatteredLuminanceScale = Settings.CloudScatteringLuminanceScale;

    // Note: Some settings require UE5 specific properties
    // LumenSourceRadius and ShadowRayTracingBias are advanced settings

    DirLight->MarkRenderStateDirty();
}

void UForge2LumenManager::ConfigurePostProcessVolume(APostProcessVolume* Volume,
                                                       const FForge2LumenVolumeSettings& Settings)
{
    if (!Volume) return;

    Volume->bEnabled = Settings.bOverrideLumenSettings;
    Volume->BlendWeight = Settings.BlendWeight;
    Volume->Priority = Settings.Priority;

    FPostProcessSettings& PPSettings = Volume->Settings;

    // GI Settings
    PPSettings.bOverride_DynamicGlobalIlluminationMethod = true;
    PPSettings.bOverride_LumenSceneViewDistance = true;
    PPSettings.bOverride_LumenSceneDetail = true;
    PPSettings.bOverride_LumenFinalGatherQuality = true;
    PPSettings.bOverride_LumenMaxTraceDistance = true;

    PPSettings.LumenSceneViewDistance = Settings.GISettings.LumenSceneViewDistance;
    PPSettings.LumenSceneDetail = Settings.GISettings.LumenSceneDetail;
    PPSettings.LumenFinalGatherQuality = Settings.GISettings.FinalGatherQuality;
    PPSettings.LumenMaxTraceDistance = Settings.GISettings.MaxTraceDistance;

    // Reflection Settings
    PPSettings.bOverride_ReflectionMethod = true;
    PPSettings.bOverride_LumenReflectionQuality = true;
    PPSettings.bOverride_LumenMaxRoughnessToTraceReflections = true;

    PPSettings.LumenReflectionQuality = Settings.ReflectionSettings.ReflectionQuality;
    PPSettings.LumenMaxRoughnessToTraceReflections = Settings.ReflectionSettings.MaxRoughnessToTrace;
}

APostProcessVolume* UForge2LumenManager::CreateLumenVolume(const FTransform& Transform,
                                                            const FForge2LumenVolumeSettings& Settings)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APostProcessVolume* Volume = World->SpawnActor<APostProcessVolume>(
        APostProcessVolume::StaticClass(),
        Transform,
        SpawnParams
    );

    if (Volume)
    {
        Volume->bUnbound = true;  // Affect entire world by default
        ConfigurePostProcessVolume(Volume, Settings);
    }

    return Volume;
}

void UForge2LumenManager::ApplyQualityPreset(EForge2LumenQuality Quality)
{
    GISettings = GetGIPreset(Quality);
    ReflectionSettings = GetReflectionPreset(Quality);

    switch (Quality)
    {
        case EForge2LumenQuality::Low:
            PerformanceSettings.RayBudget = 0.5f;
            PerformanceSettings.DetailTracing = EForge2LumenDetailTracing::None;
            break;
        case EForge2LumenQuality::Medium:
            PerformanceSettings.RayBudget = 0.75f;
            PerformanceSettings.DetailTracing = EForge2LumenDetailTracing::None;
            break;
        case EForge2LumenQuality::High:
            PerformanceSettings.RayBudget = 1.0f;
            PerformanceSettings.DetailTracing = EForge2LumenDetailTracing::DetailTracesOnly;
            break;
        case EForge2LumenQuality::Ultra:
            PerformanceSettings.RayBudget = 1.5f;
            PerformanceSettings.DetailTracing = EForge2LumenDetailTracing::Full;
            break;
        case EForge2LumenQuality::Cinematic:
            PerformanceSettings.RayBudget = 2.0f;
            PerformanceSettings.DetailTracing = EForge2LumenDetailTracing::Full;
            break;
    }

    ApplyGISettings();
    ApplyReflectionSettings();
    ApplyPerformanceSettings();
}

FForge2LumenGISettings UForge2LumenManager::GetGIPreset(EForge2LumenQuality Quality) const
{
    FForge2LumenGISettings Settings;

    switch (Quality)
    {
        case EForge2LumenQuality::Low:
            Settings.bEnabled = true;
            Settings.Method = EForge2LumenGIMethod::ScreenTraces;
            Settings.LumenSceneViewDistance = 10000.0f;
            Settings.LumenSceneDetail = 0.5f;
            Settings.FinalGatherQuality = 0.5f;
            Settings.MaxTraceDistance = 10000.0f;
            Settings.SurfaceCacheResolution = 0.5f;
            break;

        case EForge2LumenQuality::Medium:
            Settings.bEnabled = true;
            Settings.Method = EForge2LumenGIMethod::SoftwareRayTracing;
            Settings.LumenSceneViewDistance = 15000.0f;
            Settings.LumenSceneDetail = 0.75f;
            Settings.FinalGatherQuality = 0.75f;
            Settings.MaxTraceDistance = 15000.0f;
            Settings.SurfaceCacheResolution = 0.75f;
            break;

        case EForge2LumenQuality::High:
            Settings = UForge2LumenUtility::CreateDefaultGISettings();
            break;

        case EForge2LumenQuality::Ultra:
            Settings = UForge2LumenUtility::CreateHighQualityGISettings();
            break;

        case EForge2LumenQuality::Cinematic:
            Settings.bEnabled = true;
            Settings.Method = EForge2LumenGIMethod::HardwareRayTracing;
            Settings.LumenSceneViewDistance = 50000.0f;
            Settings.LumenSceneDetail = 2.0f;
            Settings.FinalGatherQuality = 2.0f;
            Settings.MaxTraceDistance = 50000.0f;
            Settings.RayLightingMode = 1;  // Hit Lighting
            Settings.bFullSkylightLeakingRemoval = true;
            Settings.SurfaceCacheResolution = 2.0f;
            break;
    }

    return Settings;
}

FForge2LumenReflectionSettings UForge2LumenManager::GetReflectionPreset(EForge2LumenQuality Quality) const
{
    FForge2LumenReflectionSettings Settings;

    switch (Quality)
    {
        case EForge2LumenQuality::Low:
            Settings.bEnabled = true;
            Settings.Method = EForge2LumenReflectionMethod::ScreenSpace;
            Settings.ReflectionQuality = 0.5f;
            Settings.MaxRoughnessToTrace = 0.3f;
            Settings.MaxReflectionBounces = 1;
            break;

        case EForge2LumenQuality::Medium:
            Settings.bEnabled = true;
            Settings.Method = EForge2LumenReflectionMethod::SoftwareRayTracing;
            Settings.ReflectionQuality = 0.75f;
            Settings.MaxRoughnessToTrace = 0.4f;
            Settings.MaxReflectionBounces = 1;
            break;

        case EForge2LumenQuality::High:
            Settings = UForge2LumenUtility::CreateDefaultReflectionSettings();
            break;

        case EForge2LumenQuality::Ultra:
            Settings = UForge2LumenUtility::CreateHighQualityReflectionSettings();
            break;

        case EForge2LumenQuality::Cinematic:
            Settings.bEnabled = true;
            Settings.Method = EForge2LumenReflectionMethod::HardwareRayTracing;
            Settings.ReflectionQuality = 2.0f;
            Settings.RayLightingQuality = 2.0f;
            Settings.MaxRoughnessToTrace = 0.6f;
            Settings.MaxReflectionBounces = 3;
            Settings.bHighQualityTranslucencyReflections = true;
            break;
    }

    return Settings;
}

void UForge2LumenManager::UpdateSceneStats()
{
    SceneStats.Reset();
    SceneStats.UpdateFromScene();
}

void UForge2LumenManager::ApplyGISettings()
{
    // Apply GI settings through console variables
    IConsoleVariable* EnableGIVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicGlobalIlluminationMethod"));
    if (EnableGIVar)
    {
        // 0=None, 1=Lumen, 2=ScreenSpace, 3=RayTracing
        int32 MethodValue = GISettings.bEnabled ? 1 : 0;
        if (GISettings.Method == EForge2LumenGIMethod::HardwareRayTracing)
            MethodValue = 3;
        else if (GISettings.Method == EForge2LumenGIMethod::ScreenTraces)
            MethodValue = 2;
        EnableGIVar->Set(MethodValue);
    }
    
    // Lumen scene settings
    if (IConsoleVariable* ViewDistVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.SceneViewDistance")))
        ViewDistVar->Set(GISettings.LumenSceneViewDistance);
    
    if (IConsoleVariable* DetailVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.SceneDetail")))
        DetailVar->Set(GISettings.LumenSceneDetail);
    
    if (IConsoleVariable* FinalGatherVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.FinalGatherQuality")))
        FinalGatherVar->Set(GISettings.FinalGatherQuality);
    
    if (IConsoleVariable* MaxTraceVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.MaxTraceDistance")))
        MaxTraceVar->Set(GISettings.MaxTraceDistance);
    
    if (IConsoleVariable* SurfaceCacheVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.SurfaceCacheResolution")))
        SurfaceCacheVar->Set(GISettings.SurfaceCacheResolution);
    
    if (IConsoleVariable* RayLightingVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing.LightingMode")))
        RayLightingVar->Set(GISettings.RayLightingMode);
    
    if (IConsoleVariable* SkylightLeakVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.FullSkylightLeakingRemoval")))
        SkylightLeakVar->Set(GISettings.bFullSkylightLeakingRemoval ? 1 : 0);
}

void UForge2LumenManager::ApplyReflectionSettings()
{
    // Apply reflection settings through console variables
    IConsoleVariable* ReflectionMethodVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ReflectionMethod"));
    if (ReflectionMethodVar)
    {
        // 0=None, 1=Lumen, 2=ScreenSpace, 3=RayTracing
        int32 MethodValue = ReflectionSettings.bEnabled ? 1 : 0;
        if (ReflectionSettings.Method == EForge2LumenReflectionMethod::HardwareRayTracing)
            MethodValue = 3;
        else if (ReflectionSettings.Method == EForge2LumenReflectionMethod::ScreenSpace)
            MethodValue = 2;
        ReflectionMethodVar->Set(MethodValue);
    }
    
    if (IConsoleVariable* QualityVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.ReflectionQuality")))
        QualityVar->Set(ReflectionSettings.ReflectionQuality);
    
    if (IConsoleVariable* MaxRoughnessVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.MaxRoughnessToTraceReflections")))
        MaxRoughnessVar->Set(ReflectionSettings.MaxRoughnessToTrace);
    
    if (IConsoleVariable* RayLightingQualityVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.RayLightingQuality")))
        RayLightingQualityVar->Set(ReflectionSettings.RayLightingQuality);
    
    if (IConsoleVariable* MaxBouncesVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.MaxBounces")))
        MaxBouncesVar->Set(ReflectionSettings.MaxReflectionBounces);
    
    if (IConsoleVariable* TranslucencyVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.HighQualityTranslucency")))
        TranslucencyVar->Set(ReflectionSettings.bHighQualityTranslucencyReflections ? 1 : 0);
}

void UForge2LumenManager::ApplyPerformanceSettings()
{
    // Apply performance settings through console variables
    if (IConsoleVariable* RayBudgetVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.ScreenProbes.RayBudget")))
        RayBudgetVar->Set(PerformanceSettings.RayBudget);
    
    if (IConsoleVariable* GatherResVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.ScreenProbes.GatherOctahedronResolution")))
        GatherResVar->Set(PerformanceSettings.ScreenProbeGatherOctahedronResolution);
    
    if (IConsoleVariable* TracingResVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.ScreenProbes.TracingOctahedronResolution")))
        TracingResVar->Set(PerformanceSettings.ScreenProbeTracingOctahedronResolution);
    
    if (IConsoleVariable* DetailTracingVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.DetailTracing")))
    {
        int32 DetailValue = (int32)PerformanceSettings.DetailTracing;
        DetailTracingVar->Set(DetailValue);
    }
    
    if (IConsoleVariable* CompressCardsVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.SurfaceCache.Compress")))
        CompressCardsVar->Set(PerformanceSettings.bCompressSceneCards ? 1 : 0);
    
    if (IConsoleVariable* UpdateDelayVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.SurfaceCache.UpdateEveryNthFrame")))
    {
        int32 FrameDelay = FMath::Max(1, FMath::RoundToInt(PerformanceSettings.SurfaceCacheUpdateDelay * 60.0f));
        UpdateDelayVar->Set(FrameDelay);
    }
    
    if (IConsoleVariable* MaxUpdatesVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.SurfaceCache.MaxUpdatesPerFrame")))
        MaxUpdatesVar->Set(PerformanceSettings.MaxSurfaceCacheUpdatesPerFrame);
}

// UForge2LumenUtility Implementation
FForge2LumenGISettings UForge2LumenUtility::CreateDefaultGISettings()
{
    FForge2LumenGISettings Settings;
    Settings.bEnabled = true;
    Settings.Method = EForge2LumenGIMethod::SoftwareRayTracing;
    Settings.LumenSceneViewDistance = 20000.0f;
    Settings.LumenSceneDetail = 1.0f;
    Settings.FinalGatherQuality = 1.0f;
    Settings.MaxTraceDistance = 20000.0f;
    Settings.RayLightingMode = 0;
    Settings.bFullSkylightLeakingRemoval = false;
    Settings.SurfaceCacheResolution = 1.0f;
    return Settings;
}

FForge2LumenGISettings UForge2LumenUtility::CreateHighQualityGISettings()
{
    FForge2LumenGISettings Settings;
    Settings.bEnabled = true;
    Settings.Method = EForge2LumenGIMethod::HardwareRayTracing;
    Settings.LumenSceneViewDistance = 30000.0f;
    Settings.LumenSceneDetail = 1.5f;
    Settings.FinalGatherQuality = 1.5f;
    Settings.MaxTraceDistance = 30000.0f;
    Settings.RayLightingMode = 1;
    Settings.bFullSkylightLeakingRemoval = true;
    Settings.SurfaceCacheResolution = 1.5f;
    return Settings;
}

FForge2LumenGISettings UForge2LumenUtility::CreatePerformanceGISettings()
{
    FForge2LumenGISettings Settings;
    Settings.bEnabled = true;
    Settings.Method = EForge2LumenGIMethod::ScreenTraces;
    Settings.LumenSceneViewDistance = 10000.0f;
    Settings.LumenSceneDetail = 0.5f;
    Settings.FinalGatherQuality = 0.5f;
    Settings.MaxTraceDistance = 10000.0f;
    Settings.RayLightingMode = 0;
    Settings.bFullSkylightLeakingRemoval = false;
    Settings.SurfaceCacheResolution = 0.5f;
    return Settings;
}

FForge2LumenReflectionSettings UForge2LumenUtility::CreateDefaultReflectionSettings()
{
    FForge2LumenReflectionSettings Settings;
    Settings.bEnabled = true;
    Settings.Method = EForge2LumenReflectionMethod::SoftwareRayTracing;
    Settings.ReflectionQuality = 1.0f;
    Settings.RayLightingQuality = 1.0f;
    Settings.MaxRoughnessToTrace = 0.4f;
    Settings.MaxReflectionBounces = 1;
    Settings.bHighQualityTranslucencyReflections = false;
    Settings.ReflectionConvolutionScale = 1.0f;
    return Settings;
}

FForge2LumenReflectionSettings UForge2LumenUtility::CreateHighQualityReflectionSettings()
{
    FForge2LumenReflectionSettings Settings;
    Settings.bEnabled = true;
    Settings.Method = EForge2LumenReflectionMethod::HardwareRayTracing;
    Settings.ReflectionQuality = 1.5f;
    Settings.RayLightingQuality = 1.5f;
    Settings.MaxRoughnessToTrace = 0.5f;
    Settings.MaxReflectionBounces = 2;
    Settings.bHighQualityTranslucencyReflections = true;
    Settings.ReflectionConvolutionScale = 1.0f;
    return Settings;
}

FForge2LumenPerformanceSettings UForge2LumenUtility::CreateDefaultPerformanceSettings()
{
    FForge2LumenPerformanceSettings Settings;
    Settings.RayBudget = 1.0f;
    Settings.ScreenProbeGatherOctahedronResolution = 8.0f;
    Settings.ScreenProbeTracingOctahedronResolution = 8.0f;
    Settings.DetailTracing = EForge2LumenDetailTracing::DetailTracesOnly;
    Settings.bCompressSceneCards = true;
    Settings.SurfaceCacheUpdateDelay = 0.1f;
    Settings.MaxSurfaceCacheUpdatesPerFrame = 16;
    return Settings;
}

FForge2LumenPerformanceSettings UForge2LumenUtility::CreateConsolePerformanceSettings()
{
    FForge2LumenPerformanceSettings Settings;
    Settings.RayBudget = 0.75f;
    Settings.ScreenProbeGatherOctahedronResolution = 6.0f;
    Settings.ScreenProbeTracingOctahedronResolution = 6.0f;
    Settings.DetailTracing = EForge2LumenDetailTracing::None;
    Settings.bCompressSceneCards = true;
    Settings.SurfaceCacheUpdateDelay = 0.2f;
    Settings.MaxSurfaceCacheUpdatesPerFrame = 8;
    return Settings;
}

bool UForge2LumenUtility::IsHardwareRayTracingSupported()
{
    // Check hardware ray tracing capability
#if RHI_RAYTRACING
    return GRHISupportsRayTracing && IsRayTracingEnabled();
#else
    return false;
#endif
}

bool UForge2LumenUtility::IsLumenSupported()
{
    // Lumen requires SM5+ (DX11 feature level 11_0+) or equivalent
    // Check the max shader platform level
    ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel;
    
    // Lumen requires at least SM5 (feature level 10+)
    if (FeatureLevel < ERHIFeatureLevel::SM5)
    {
        return false;
    }
    
    // Additional check: Lumen not supported on mobile platforms
    bool bIsMobile = (GMaxRHIShaderPlatform == SP_OPENGL_ES3_1_ANDROID || 
                      GMaxRHIShaderPlatform == SP_METAL_MRT ||
                      GMaxRHIShaderPlatform == SP_VULKAN_ES3_1_ANDROID);
    
    return !bIsMobile;
}

float UForge2LumenUtility::EstimateGPUMemoryUsageMB(const FForge2LumenGISettings& GISettings,
                                                     const FForge2LumenReflectionSettings& ReflectionSettings)
{
    // Rough estimate of Lumen VRAM usage
    float BaseMemory = 200.0f;  // Base overhead

    // Surface cache memory scales with resolution
    float SurfaceCacheMemory = 100.0f * GISettings.SurfaceCacheResolution;

    // Ray tracing memory scales with quality
    float RayTracingMemory = 50.0f * GISettings.FinalGatherQuality;

    // Reflection memory
    float ReflectionMemory = 30.0f * ReflectionSettings.ReflectionQuality;

    // Hardware ray tracing uses more memory
    if (GISettings.Method == EForge2LumenGIMethod::HardwareRayTracing)
    {
        RayTracingMemory *= 1.5f;
    }

    return BaseMemory + SurfaceCacheMemory + RayTracingMemory + ReflectionMemory;
}
