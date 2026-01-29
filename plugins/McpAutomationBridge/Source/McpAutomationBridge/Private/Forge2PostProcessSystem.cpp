// Forge2PostProcessSystem.cpp - Post-Processing System Implementation
#include "Forge2PostProcessSystem.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"

// UForge2PostProcessManager Implementation
UForge2PostProcessManager::UForge2PostProcessManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

APostProcessVolume* UForge2PostProcessManager::CreatePostProcessVolume(FVector Location, FVector Extent, bool bUnbound)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APostProcessVolume* Volume = World->SpawnActor<APostProcessVolume>(
        APostProcessVolume::StaticClass(),
        Location,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (Volume)
    {
        Volume->bUnbound = bUnbound;

        if (!bUnbound)
        {
            // Set volume extent via brush
            Volume->SetActorScale3D(Extent / 100.0f);  // Convert to relative scale
        }

        Volume->bEnabled = true;
    }

    return Volume;
}

void UForge2PostProcessManager::ConfigureVolumePriority(APostProcessVolume* Volume, float Priority, float BlendRadius)
{
    if (!Volume) return;

    Volume->Priority = Priority;
    Volume->BlendRadius = BlendRadius;
}

void UForge2PostProcessManager::ConfigureBloom(APostProcessVolume* Volume, const FForge2BloomSettings& Settings)
{
    if (!Volume) return;

    FPostProcessSettings& PPSettings = Volume->Settings;

    PPSettings.bOverride_BloomIntensity = true;
    PPSettings.BloomIntensity = Settings.Intensity;

    PPSettings.bOverride_BloomThreshold = true;
    PPSettings.BloomThreshold = Settings.Threshold;

    PPSettings.bOverride_BloomSizeScale = true;
    PPSettings.BloomSizeScale = Settings.SizeScale;

    // Bloom method is set via project settings in UE5
    // Method-specific settings
    if (Settings.Method == EForge2BloomMethod::Convolution)
    {
        PPSettings.bOverride_BloomConvolutionScatterDispersion = true;
        PPSettings.BloomConvolutionScatterDispersion = Settings.ConvolutionScatterDispersion;
    }
}

void UForge2PostProcessManager::ConfigureColorGrading(APostProcessVolume* Volume, const FForge2ColorGradingSettings& Settings)
{
    if (!Volume) return;

    FPostProcessSettings& PPSettings = Volume->Settings;

    // White Balance
    PPSettings.bOverride_WhiteTemp = true;
    PPSettings.WhiteTemp = Settings.Temperature;

    PPSettings.bOverride_WhiteTint = true;
    PPSettings.WhiteTint = Settings.Tint;

    // Global
    PPSettings.bOverride_ColorSaturation = true;
    PPSettings.ColorSaturation = FVector4(Settings.ColorSaturation);

    PPSettings.bOverride_ColorContrast = true;
    PPSettings.ColorContrast = FVector4(Settings.ColorContrast);

    PPSettings.bOverride_ColorGamma = true;
    PPSettings.ColorGamma = FVector4(Settings.ColorGamma);

    PPSettings.bOverride_ColorGain = true;
    PPSettings.ColorGain = FVector4(Settings.ColorGain);

    PPSettings.bOverride_ColorOffset = true;
    PPSettings.ColorOffset = FVector4(Settings.ColorOffset);

    // Shadows
    PPSettings.bOverride_ColorSaturationShadows = true;
    PPSettings.ColorSaturationShadows = FVector4(Settings.ShadowsSaturation);

    PPSettings.bOverride_ColorContrastShadows = true;
    PPSettings.ColorContrastShadows = FVector4(Settings.ShadowsContrast);

    PPSettings.bOverride_ColorGammaShadows = true;
    PPSettings.ColorGammaShadows = FVector4(Settings.ShadowsGamma);

    PPSettings.bOverride_ColorGainShadows = true;
    PPSettings.ColorGainShadows = FVector4(Settings.ShadowsGain);

    PPSettings.bOverride_ColorOffsetShadows = true;
    PPSettings.ColorOffsetShadows = FVector4(Settings.ShadowsOffset);

    PPSettings.bOverride_ColorCorrectionShadowsMax = true;
    PPSettings.ColorCorrectionShadowsMax = Settings.ShadowsMax;

    // Midtones
    PPSettings.bOverride_ColorSaturationMidtones = true;
    PPSettings.ColorSaturationMidtones = FVector4(Settings.MidtonesSaturation);

    PPSettings.bOverride_ColorContrastMidtones = true;
    PPSettings.ColorContrastMidtones = FVector4(Settings.MidtonesContrast);

    PPSettings.bOverride_ColorGammaMidtones = true;
    PPSettings.ColorGammaMidtones = FVector4(Settings.MidtonesGamma);

    PPSettings.bOverride_ColorGainMidtones = true;
    PPSettings.ColorGainMidtones = FVector4(Settings.MidtonesGain);

    PPSettings.bOverride_ColorOffsetMidtones = true;
    PPSettings.ColorOffsetMidtones = FVector4(Settings.MidtonesOffset);

    // Highlights
    PPSettings.bOverride_ColorSaturationHighlights = true;
    PPSettings.ColorSaturationHighlights = FVector4(Settings.HighlightsSaturation);

    PPSettings.bOverride_ColorContrastHighlights = true;
    PPSettings.ColorContrastHighlights = FVector4(Settings.HighlightsContrast);

    PPSettings.bOverride_ColorGammaHighlights = true;
    PPSettings.ColorGammaHighlights = FVector4(Settings.HighlightsGamma);

    PPSettings.bOverride_ColorGainHighlights = true;
    PPSettings.ColorGainHighlights = FVector4(Settings.HighlightsGain);

    PPSettings.bOverride_ColorOffsetHighlights = true;
    PPSettings.ColorOffsetHighlights = FVector4(Settings.HighlightsOffset);

    PPSettings.bOverride_ColorCorrectionHighlightsMin = true;
    PPSettings.ColorCorrectionHighlightsMin = Settings.HighlightsMin;

    // LUT
    if (Settings.LookupTable.IsValid())
    {
        PPSettings.bOverride_ColorGradingLUT = true;
        PPSettings.ColorGradingLUT = Settings.LookupTable.LoadSynchronous();

        PPSettings.bOverride_ColorGradingIntensity = true;
        PPSettings.ColorGradingIntensity = Settings.LUTIntensity;
    }
}

void UForge2PostProcessManager::ApplyLUT(APostProcessVolume* Volume, UTexture* LUT, float Intensity)
{
    if (!Volume || !LUT) return;

    FPostProcessSettings& PPSettings = Volume->Settings;

    PPSettings.bOverride_ColorGradingLUT = true;
    PPSettings.ColorGradingLUT = LUT;

    PPSettings.bOverride_ColorGradingIntensity = true;
    PPSettings.ColorGradingIntensity = Intensity;
}

void UForge2PostProcessManager::ConfigureDepthOfField(APostProcessVolume* Volume, const FForge2DOFSettings& Settings)
{
    if (!Volume) return;

    FPostProcessSettings& PPSettings = Volume->Settings;

    PPSettings.bOverride_DepthOfFieldFocalDistance = true;
    PPSettings.DepthOfFieldFocalDistance = Settings.FocalDistance;

    PPSettings.bOverride_DepthOfFieldFocalRegion = true;
    PPSettings.DepthOfFieldFocalRegion = Settings.FocalRegion;

    PPSettings.bOverride_DepthOfFieldNearTransitionRegion = true;
    PPSettings.DepthOfFieldNearTransitionRegion = Settings.NearTransitionRegion;

    PPSettings.bOverride_DepthOfFieldFarTransitionRegion = true;
    PPSettings.DepthOfFieldFarTransitionRegion = Settings.FarTransitionRegion;

    PPSettings.bOverride_DepthOfFieldDepthBlurAmount = true;
    PPSettings.DepthOfFieldDepthBlurAmount = Settings.DepthBlurAmount;

    PPSettings.bOverride_DepthOfFieldDepthBlurRadius = true;
    PPSettings.DepthOfFieldDepthBlurRadius = Settings.DepthBlurRadius;

    PPSettings.bOverride_DepthOfFieldFstop = true;
    PPSettings.DepthOfFieldFstop = Settings.Aperture;

    PPSettings.bOverride_DepthOfFieldBladeCount = true;
    PPSettings.DepthOfFieldBladeCount = Settings.BladeCount;

    PPSettings.bOverride_DepthOfFieldMinFstop = true;
    PPSettings.DepthOfFieldMinFstop = Settings.BokehMinSize;
}

void UForge2PostProcessManager::SetFocalDistance(APostProcessVolume* Volume, float Distance)
{
    if (!Volume) return;

    Volume->Settings.bOverride_DepthOfFieldFocalDistance = true;
    Volume->Settings.DepthOfFieldFocalDistance = Distance;
}

void UForge2PostProcessManager::ConfigureMotionBlur(APostProcessVolume* Volume, const FForge2MotionBlurSettings& Settings)
{
    if (!Volume) return;

    FPostProcessSettings& PPSettings = Volume->Settings;

    PPSettings.bOverride_MotionBlurAmount = true;
    PPSettings.MotionBlurAmount = Settings.Amount;

    PPSettings.bOverride_MotionBlurMax = true;
    PPSettings.MotionBlurMax = Settings.Max;

    PPSettings.bOverride_MotionBlurTargetFPS = true;
    PPSettings.MotionBlurTargetFPS = Settings.TargetFPS;

    PPSettings.bOverride_MotionBlurPerObjectSize = true;
    PPSettings.MotionBlurPerObjectSize = Settings.PerObjectSize;
}

void UForge2PostProcessManager::ConfigureAmbientOcclusion(APostProcessVolume* Volume, const FForge2AmbientOcclusionSettings& Settings)
{
    if (!Volume) return;

    FPostProcessSettings& PPSettings = Volume->Settings;

    PPSettings.bOverride_AmbientOcclusionIntensity = true;
    PPSettings.AmbientOcclusionIntensity = Settings.Intensity;

    PPSettings.bOverride_AmbientOcclusionRadius = true;
    PPSettings.AmbientOcclusionRadius = Settings.Radius;

    PPSettings.bOverride_AmbientOcclusionRadiusInWS = true;
    PPSettings.AmbientOcclusionRadiusInWS = Settings.bRadiusInWorldSpace;

    PPSettings.bOverride_AmbientOcclusionMipBlend = true;
    PPSettings.AmbientOcclusionMipBlend = Settings.MipBlend;

    PPSettings.bOverride_AmbientOcclusionMipScale = true;
    PPSettings.AmbientOcclusionMipScale = Settings.MipScale;

    PPSettings.bOverride_AmbientOcclusionMipThreshold = true;
    PPSettings.AmbientOcclusionMipThreshold = Settings.MipThreshold;

    PPSettings.bOverride_AmbientOcclusionPower = true;
    PPSettings.AmbientOcclusionPower = Settings.Power;

    PPSettings.bOverride_AmbientOcclusionBias = true;
    PPSettings.AmbientOcclusionBias = Settings.Bias;

    PPSettings.bOverride_AmbientOcclusionQuality = true;
    PPSettings.AmbientOcclusionQuality = Settings.Quality;

    PPSettings.bOverride_AmbientOcclusionStaticFraction = true;
    PPSettings.AmbientOcclusionStaticFraction = Settings.StaticFraction.R;
}

void UForge2PostProcessManager::ConfigureScreenEffects(APostProcessVolume* Volume, const FForge2ScreenEffectsSettings& Settings)
{
    if (!Volume) return;

    SetVignette(Volume, Settings.VignetteIntensity, Settings.VignetteColor);
    SetFilmGrain(Volume, Settings.GrainIntensity, Settings.GrainJitter);
    SetChromaticAberration(Volume, Settings.ChromaticAberrationIntensity, Settings.ChromaticAberrationStartOffset);

    FPostProcessSettings& PPSettings = Volume->Settings;

    // Lens Flare
    PPSettings.bOverride_LensFlareIntensity = true;
    PPSettings.LensFlareIntensity = Settings.LensFlareIntensity;

    PPSettings.bOverride_LensFlareTint = true;
    PPSettings.LensFlareTint = Settings.LensFlareTint;

    PPSettings.bOverride_LensFlareBokehSize = true;
    PPSettings.LensFlareBokehSize = Settings.LensFlareBokehSize;

    PPSettings.bOverride_LensFlareThreshold = true;
    PPSettings.LensFlareThreshold = Settings.LensFlareThreshold;
}

void UForge2PostProcessManager::SetVignette(APostProcessVolume* Volume, float Intensity, FLinearColor Color)
{
    if (!Volume) return;

    Volume->Settings.bOverride_VignetteIntensity = true;
    Volume->Settings.VignetteIntensity = Intensity;
}

void UForge2PostProcessManager::SetFilmGrain(APostProcessVolume* Volume, float Intensity, float Jitter)
{
    if (!Volume) return;

    Volume->Settings.bOverride_FilmGrainIntensity = true;
    Volume->Settings.FilmGrainIntensity = Intensity;

    Volume->Settings.bOverride_FilmGrainIntensityShadows = true;
    Volume->Settings.FilmGrainIntensityShadows = Intensity;

    Volume->Settings.bOverride_FilmGrainIntensityMidtones = true;
    Volume->Settings.FilmGrainIntensityMidtones = Intensity;

    Volume->Settings.bOverride_FilmGrainIntensityHighlights = true;
    Volume->Settings.FilmGrainIntensityHighlights = Intensity * 0.5f;
}

void UForge2PostProcessManager::SetChromaticAberration(APostProcessVolume* Volume, float Intensity, float StartOffset)
{
    if (!Volume) return;

    Volume->Settings.bOverride_SceneFringeIntensity = true;
    Volume->Settings.SceneFringeIntensity = Intensity;

    Volume->Settings.bOverride_ChromaticAberrationStartOffset = true;
    Volume->Settings.ChromaticAberrationStartOffset = StartOffset;
}

void UForge2PostProcessManager::ConfigureExposure(APostProcessVolume* Volume, const FForge2ExposureSettings& Settings)
{
    if (!Volume) return;

    FPostProcessSettings& PPSettings = Volume->Settings;

    PPSettings.bOverride_AutoExposureMinBrightness = true;
    PPSettings.AutoExposureMinBrightness = Settings.MinBrightness;

    PPSettings.bOverride_AutoExposureMaxBrightness = true;
    PPSettings.AutoExposureMaxBrightness = Settings.MaxBrightness;

    PPSettings.bOverride_AutoExposureSpeedUp = true;
    PPSettings.AutoExposureSpeedUp = Settings.SpeedUp;

    PPSettings.bOverride_AutoExposureSpeedDown = true;
    PPSettings.AutoExposureSpeedDown = Settings.SpeedDown;

    PPSettings.bOverride_AutoExposureLowPercent = true;
    PPSettings.AutoExposureLowPercent = Settings.LowPercent;

    PPSettings.bOverride_AutoExposureHighPercent = true;
    PPSettings.AutoExposureHighPercent = Settings.HighPercent;

    PPSettings.bOverride_HistogramLogMin = true;
    PPSettings.HistogramLogMin = Settings.HistogramLogMin;

    PPSettings.bOverride_HistogramLogMax = true;
    PPSettings.HistogramLogMax = Settings.HistogramLogMax;

    PPSettings.bOverride_AutoExposureBias = true;
    PPSettings.AutoExposureBias = Settings.Compensation;
}

void UForge2PostProcessManager::ConfigureFog(APostProcessVolume* Volume, const FForge2FogSettings& Settings)
{
    if (!Volume) return;

    // Height fog settings are typically on the fog actor, not post process volume
    // This handles post-process related fog settings

    FPostProcessSettings& PPSettings = Volume->Settings;

    // Volumetric fog settings that can be overridden in post process
    if (Settings.bVolumetricFog)
    {
        // These would typically be on ExponentialHeightFog actor
        // But some settings can be controlled per-volume
    }
}

void UForge2PostProcessManager::ApplyPreset(APostProcessVolume* Volume, EForge2PostProcessPreset Preset)
{
    if (!Volume) return;

    FForge2PostProcessPresetData PresetData = GetPresetData(Preset);
    ApplySettings(Volume, PresetData);
}

FForge2PostProcessPresetData UForge2PostProcessManager::GetPresetData(EForge2PostProcessPreset Preset)
{
    switch (Preset)
    {
        case EForge2PostProcessPreset::Cinematic:
            return GetCinematicPreset();

        case EForge2PostProcessPreset::Horror:
            return GetHorrorPreset();

        case EForge2PostProcessPreset::Noir:
            return GetNoirPreset();

        case EForge2PostProcessPreset::Realistic:
        default:
            {
                FForge2PostProcessPresetData Data;
                Data.PresetName = TEXT("Realistic");
                Data.PresetType = EForge2PostProcessPreset::Realistic;
                Data.Bloom = UForge2PostProcessUtility::CreateDefaultBloom();
                Data.ColorGrading = UForge2PostProcessUtility::CreateDefaultColorGrading();
                return Data;
            }
    }
}

FForge2PostProcessPresetData UForge2PostProcessManager::GetCinematicPreset()
{
    FForge2PostProcessPresetData Data;
    Data.PresetName = TEXT("Cinematic");
    Data.PresetType = EForge2PostProcessPreset::Cinematic;

    // Subtle bloom
    Data.Bloom.Intensity = 0.5f;
    Data.Bloom.Threshold = 1.0f;
    Data.Bloom.SizeScale = 6.0f;

    // Rich color grading
    Data.ColorGrading.Temperature = 6200.0f;
    Data.ColorGrading.ColorContrast = FLinearColor(1.1f, 1.1f, 1.1f, 1.0f);
    Data.ColorGrading.ColorSaturation = FLinearColor(1.05f, 1.05f, 1.05f, 1.0f);
    Data.ColorGrading.ShadowsGain = FLinearColor(0.95f, 0.95f, 1.0f, 1.0f);  // Slight blue in shadows
    Data.ColorGrading.HighlightsGain = FLinearColor(1.05f, 1.02f, 0.98f, 1.0f);  // Warm highlights

    // Cinematic DOF
    Data.DepthOfField = UForge2PostProcessUtility::CreateCinematicDOF();

    // Subtle motion blur
    Data.MotionBlur.Amount = 0.3f;
    Data.MotionBlur.Max = 3.0f;

    // Screen effects
    Data.ScreenEffects.VignetteIntensity = 0.3f;
    Data.ScreenEffects.GrainIntensity = 0.05f;

    // Exposure
    Data.Exposure.MinBrightness = 0.05f;
    Data.Exposure.MaxBrightness = 1.5f;
    Data.Exposure.SpeedUp = 2.0f;
    Data.Exposure.SpeedDown = 0.5f;

    return Data;
}

FForge2PostProcessPresetData UForge2PostProcessManager::GetHorrorPreset()
{
    FForge2PostProcessPresetData Data;
    Data.PresetName = TEXT("Horror");
    Data.PresetType = EForge2PostProcessPreset::Horror;

    // Dark, moody bloom
    Data.Bloom.Intensity = 0.3f;
    Data.Bloom.Threshold = 2.0f;
    Data.Bloom.Tint = FLinearColor(0.9f, 0.85f, 1.0f);

    // Desaturated, cold color grading
    Data.ColorGrading.Temperature = 5500.0f;
    Data.ColorGrading.Tint = 5.0f;
    Data.ColorGrading.ColorSaturation = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
    Data.ColorGrading.ColorContrast = FLinearColor(1.3f, 1.3f, 1.3f, 1.0f);
    Data.ColorGrading.ColorGamma = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
    Data.ColorGrading.ShadowsGain = FLinearColor(0.0f, 0.0f, 0.05f, 1.0f);  // Deep blacks with blue

    // Heavy vignette and grain
    Data.ScreenEffects.VignetteIntensity = 0.6f;
    Data.ScreenEffects.VignetteColor = FLinearColor::Black;
    Data.ScreenEffects.GrainIntensity = 0.15f;
    Data.ScreenEffects.GrainJitter = 0.5f;
    Data.ScreenEffects.ChromaticAberrationIntensity = 0.3f;

    // Dark exposure
    Data.Exposure.MinBrightness = 0.01f;
    Data.Exposure.MaxBrightness = 0.8f;
    Data.Exposure.Compensation = -0.5f;

    return Data;
}

FForge2PostProcessPresetData UForge2PostProcessManager::GetNoirPreset()
{
    FForge2PostProcessPresetData Data;
    Data.PresetName = TEXT("Noir");
    Data.PresetType = EForge2PostProcessPreset::Noir;

    // High contrast bloom
    Data.Bloom.Intensity = 0.8f;
    Data.Bloom.Threshold = 0.5f;

    // Black and white with high contrast
    Data.ColorGrading.ColorSaturation = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Full desaturation
    Data.ColorGrading.ColorContrast = FLinearColor(1.5f, 1.5f, 1.5f, 1.0f);
    Data.ColorGrading.ColorGamma = FLinearColor(0.85f, 0.85f, 0.85f, 1.0f);

    // Strong vignette
    Data.ScreenEffects.VignetteIntensity = 0.5f;
    Data.ScreenEffects.GrainIntensity = 0.1f;

    return Data;
}

void UForge2PostProcessManager::BlendToPreset(APostProcessVolume* Volume, const FForge2PostProcessPresetData& TargetPreset, float Duration)
{
    if (!Volume || !GetWorld()) return;
    
    // Store current settings as start point
    FForge2PostProcessPresetData StartPreset = CaptureCurrentSettings(Volume);
    
    // Store blend data
    FPostProcessBlendData BlendData;
    BlendData.Volume = Volume;
    BlendData.StartPreset = StartPreset;
    BlendData.TargetPreset = TargetPreset;
    BlendData.Duration = Duration;
    BlendData.ElapsedTime = 0.0f;
    
    ActiveBlends.Add(BlendData);
    
    // Enable tick for interpolation updates
    if (!PrimaryComponentTick.IsTickFunctionEnabled())
    {
        PrimaryComponentTick.SetTickFunctionEnable(true);
    }
}

void UForge2PostProcessManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Process active blends
    for (int32 i = ActiveBlends.Num() - 1; i >= 0; i--)
    {
        FPostProcessBlendData& Blend = ActiveBlends[i];
        Blend.ElapsedTime += DeltaTime;
        
        float Alpha = FMath::Clamp(Blend.ElapsedTime / Blend.Duration, 0.0f, 1.0f);
        
        // Lerp all settings
        FForge2PostProcessPresetData LerpedSettings = LerpPresets(Blend.StartPreset, Blend.TargetPreset, Alpha);
        ApplySettings(Blend.Volume.Get(), LerpedSettings);
        
        // Remove completed blends
        if (Alpha >= 1.0f)
        {
            ActiveBlends.RemoveAt(i);
        }
    }
    
    // Disable tick if no active blends
    if (ActiveBlends.Num() == 0)
    {
        PrimaryComponentTick.SetTickFunctionEnable(false);
    }
}

void UForge2PostProcessManager::ApplySettings(APostProcessVolume* Volume, const FForge2PostProcessPresetData& Settings)
{
    if (!Volume) return;

    ConfigureBloom(Volume, Settings.Bloom);
    ConfigureColorGrading(Volume, Settings.ColorGrading);
    ConfigureDepthOfField(Volume, Settings.DepthOfField);
    ConfigureMotionBlur(Volume, Settings.MotionBlur);
    ConfigureAmbientOcclusion(Volume, Settings.AmbientOcclusion);
    ConfigureScreenEffects(Volume, Settings.ScreenEffects);
    ConfigureExposure(Volume, Settings.Exposure);
    ConfigureFog(Volume, Settings.Fog);

    Volume->BlendWeight = Settings.BlendWeight;
}

// UForge2PostProcessUtility Implementation
FForge2BloomSettings UForge2PostProcessUtility::CreateDefaultBloom()
{
    FForge2BloomSettings Settings;
    Settings.Method = EForge2BloomMethod::Standard;
    Settings.Intensity = 0.675f;
    Settings.Threshold = -1.0f;
    Settings.SizeScale = 4.0f;
    Settings.Tint = FLinearColor::White;
    return Settings;
}

FForge2ColorGradingSettings UForge2PostProcessUtility::CreateDefaultColorGrading()
{
    FForge2ColorGradingSettings Settings;
    Settings.Temperature = 6500.0f;
    Settings.Tint = 0.0f;
    Settings.ColorSaturation = FLinearColor(1, 1, 1, 1);
    Settings.ColorContrast = FLinearColor(1, 1, 1, 1);
    Settings.ColorGamma = FLinearColor(1, 1, 1, 1);
    Settings.ColorGain = FLinearColor(1, 1, 1, 1);
    Settings.ColorOffset = FLinearColor(0, 0, 0, 0);
    Settings.ToneCurve = EForge2ToneCurveType::ACES;
    return Settings;
}

FForge2ColorGradingSettings UForge2PostProcessUtility::CreateWarmColorGrading()
{
    FForge2ColorGradingSettings Settings = CreateDefaultColorGrading();
    Settings.Temperature = 7500.0f;
    Settings.ColorGain = FLinearColor(1.05f, 1.0f, 0.95f, 1.0f);
    Settings.HighlightsGain = FLinearColor(1.1f, 1.02f, 0.95f, 1.0f);
    return Settings;
}

FForge2ColorGradingSettings UForge2PostProcessUtility::CreateCoolColorGrading()
{
    FForge2ColorGradingSettings Settings = CreateDefaultColorGrading();
    Settings.Temperature = 5500.0f;
    Settings.ColorGain = FLinearColor(0.95f, 0.98f, 1.05f, 1.0f);
    Settings.ShadowsGain = FLinearColor(0.9f, 0.95f, 1.1f, 1.0f);
    return Settings;
}

FForge2ColorGradingSettings UForge2PostProcessUtility::CreateDesaturatedColorGrading()
{
    FForge2ColorGradingSettings Settings = CreateDefaultColorGrading();
    Settings.ColorSaturation = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
    Settings.ColorContrast = FLinearColor(1.1f, 1.1f, 1.1f, 1.0f);
    return Settings;
}

FForge2DOFSettings UForge2PostProcessUtility::CreateCinematicDOF()
{
    FForge2DOFSettings Settings;
    Settings.Method = EForge2DOFMethod::CircleDOF;
    Settings.FocalDistance = 1000.0f;
    Settings.FocalRegion = 100.0f;
    Settings.NearTransitionRegion = 300.0f;
    Settings.FarTransitionRegion = 500.0f;
    Settings.DepthBlurAmount = 1.0f;
    Settings.Aperture = 2.8f;
    Settings.BladeCount = 7;
    Settings.BokehMaxSize = 32.0f;
    return Settings;
}

FForge2DOFSettings UForge2PostProcessUtility::CreateTiltShiftDOF()
{
    FForge2DOFSettings Settings;
    Settings.Method = EForge2DOFMethod::Gaussian;
    Settings.FocalDistance = 500.0f;
    Settings.FocalRegion = 50.0f;
    Settings.NearTransitionRegion = 100.0f;
    Settings.FarTransitionRegion = 100.0f;
    Settings.DepthBlurAmount = 2.0f;
    return Settings;
}

FForge2ScreenEffectsSettings UForge2PostProcessUtility::CreateDramaticScreenEffects()
{
    FForge2ScreenEffectsSettings Settings;
    Settings.VignetteIntensity = 0.5f;
    Settings.VignetteColor = FLinearColor::Black;
    Settings.GrainIntensity = 0.08f;
    Settings.GrainJitter = 0.3f;
    Settings.ChromaticAberrationIntensity = 0.2f;
    Settings.LensFlareIntensity = 1.5f;
    return Settings;
}

FForge2PostProcessPresetData UForge2PostProcessManager::CaptureCurrentSettings(APostProcessVolume* Volume)
{
    FForge2PostProcessPresetData CapturedData;
    
    if (!Volume)
    {
        return CapturedData;
    }
    
    const FPostProcessSettings& PPSettings = Volume->Settings;
    
    CapturedData.PresetName = TEXT("Captured");
    CapturedData.BlendWeight = Volume->BlendWeight;
    
    // Capture Bloom settings
    CapturedData.Bloom.Intensity = PPSettings.BloomIntensity;
    CapturedData.Bloom.Threshold = PPSettings.BloomThreshold;
    CapturedData.Bloom.SizeScale = PPSettings.BloomSizeScale;
    CapturedData.Bloom.ConvolutionScatterDispersion = PPSettings.BloomConvolutionScatterDispersion;
    
    // Capture Color Grading settings
    CapturedData.ColorGrading.Temperature = PPSettings.WhiteTemp;
    CapturedData.ColorGrading.Tint = PPSettings.WhiteTint;
    CapturedData.ColorGrading.ColorSaturation = FLinearColor(PPSettings.ColorSaturation);
    CapturedData.ColorGrading.ColorContrast = FLinearColor(PPSettings.ColorContrast);
    CapturedData.ColorGrading.ColorGamma = FLinearColor(PPSettings.ColorGamma);
    CapturedData.ColorGrading.ColorGain = FLinearColor(PPSettings.ColorGain);
    CapturedData.ColorGrading.ColorOffset = FLinearColor(PPSettings.ColorOffset);
    
    // Shadows
    CapturedData.ColorGrading.ShadowsSaturation = FLinearColor(PPSettings.ColorSaturationShadows);
    CapturedData.ColorGrading.ShadowsContrast = FLinearColor(PPSettings.ColorContrastShadows);
    CapturedData.ColorGrading.ShadowsGamma = FLinearColor(PPSettings.ColorGammaShadows);
    CapturedData.ColorGrading.ShadowsGain = FLinearColor(PPSettings.ColorGainShadows);
    CapturedData.ColorGrading.ShadowsOffset = FLinearColor(PPSettings.ColorOffsetShadows);
    CapturedData.ColorGrading.ShadowsMax = PPSettings.ColorCorrectionShadowsMax;
    
    // Midtones
    CapturedData.ColorGrading.MidtonesSaturation = FLinearColor(PPSettings.ColorSaturationMidtones);
    CapturedData.ColorGrading.MidtonesContrast = FLinearColor(PPSettings.ColorContrastMidtones);
    CapturedData.ColorGrading.MidtonesGamma = FLinearColor(PPSettings.ColorGammaMidtones);
    CapturedData.ColorGrading.MidtonesGain = FLinearColor(PPSettings.ColorGainMidtones);
    CapturedData.ColorGrading.MidtonesOffset = FLinearColor(PPSettings.ColorOffsetMidtones);
    
    // Highlights
    CapturedData.ColorGrading.HighlightsSaturation = FLinearColor(PPSettings.ColorSaturationHighlights);
    CapturedData.ColorGrading.HighlightsContrast = FLinearColor(PPSettings.ColorContrastHighlights);
    CapturedData.ColorGrading.HighlightsGamma = FLinearColor(PPSettings.ColorGammaHighlights);
    CapturedData.ColorGrading.HighlightsGain = FLinearColor(PPSettings.ColorGainHighlights);
    CapturedData.ColorGrading.HighlightsOffset = FLinearColor(PPSettings.ColorOffsetHighlights);
    CapturedData.ColorGrading.HighlightsMin = PPSettings.ColorCorrectionHighlightsMin;
    
    // Capture DOF settings
    CapturedData.DepthOfField.FocalDistance = PPSettings.DepthOfFieldFocalDistance;
    CapturedData.DepthOfField.FocalRegion = PPSettings.DepthOfFieldFocalRegion;
    CapturedData.DepthOfField.NearTransitionRegion = PPSettings.DepthOfFieldNearTransitionRegion;
    CapturedData.DepthOfField.FarTransitionRegion = PPSettings.DepthOfFieldFarTransitionRegion;
    CapturedData.DepthOfField.DepthBlurAmount = PPSettings.DepthOfFieldDepthBlurAmount;
    CapturedData.DepthOfField.DepthBlurRadius = PPSettings.DepthOfFieldDepthBlurRadius;
    CapturedData.DepthOfField.Aperture = PPSettings.DepthOfFieldFstop;
    CapturedData.DepthOfField.BladeCount = PPSettings.DepthOfFieldBladeCount;
    CapturedData.DepthOfField.BokehMinSize = PPSettings.DepthOfFieldMinFstop;
    
    // Capture Motion Blur settings
    CapturedData.MotionBlur.Amount = PPSettings.MotionBlurAmount;
    CapturedData.MotionBlur.Max = PPSettings.MotionBlurMax;
    CapturedData.MotionBlur.TargetFPS = PPSettings.MotionBlurTargetFPS;
    CapturedData.MotionBlur.PerObjectSize = PPSettings.MotionBlurPerObjectSize;
    
    // Capture AO settings
    CapturedData.AmbientOcclusion.Intensity = PPSettings.AmbientOcclusionIntensity;
    CapturedData.AmbientOcclusion.Radius = PPSettings.AmbientOcclusionRadius;
    CapturedData.AmbientOcclusion.bRadiusInWorldSpace = PPSettings.AmbientOcclusionRadiusInWS;
    CapturedData.AmbientOcclusion.MipBlend = PPSettings.AmbientOcclusionMipBlend;
    CapturedData.AmbientOcclusion.MipScale = PPSettings.AmbientOcclusionMipScale;
    CapturedData.AmbientOcclusion.MipThreshold = PPSettings.AmbientOcclusionMipThreshold;
    CapturedData.AmbientOcclusion.Power = PPSettings.AmbientOcclusionPower;
    CapturedData.AmbientOcclusion.Bias = PPSettings.AmbientOcclusionBias;
    CapturedData.AmbientOcclusion.Quality = PPSettings.AmbientOcclusionQuality;
    
    // Capture Screen Effects
    CapturedData.ScreenEffects.VignetteIntensity = PPSettings.VignetteIntensity;
    CapturedData.ScreenEffects.GrainIntensity = PPSettings.FilmGrainIntensity;
    CapturedData.ScreenEffects.ChromaticAberrationIntensity = PPSettings.SceneFringeIntensity;
    CapturedData.ScreenEffects.ChromaticAberrationStartOffset = PPSettings.ChromaticAberrationStartOffset;
    CapturedData.ScreenEffects.LensFlareIntensity = PPSettings.LensFlareIntensity;
    CapturedData.ScreenEffects.LensFlareTint = PPSettings.LensFlareTint;
    CapturedData.ScreenEffects.LensFlareBokehSize = PPSettings.LensFlareBokehSize;
    CapturedData.ScreenEffects.LensFlareThreshold = PPSettings.LensFlareThreshold;
    
    // Capture Exposure settings
    CapturedData.Exposure.MinBrightness = PPSettings.AutoExposureMinBrightness;
    CapturedData.Exposure.MaxBrightness = PPSettings.AutoExposureMaxBrightness;
    CapturedData.Exposure.SpeedUp = PPSettings.AutoExposureSpeedUp;
    CapturedData.Exposure.SpeedDown = PPSettings.AutoExposureSpeedDown;
    CapturedData.Exposure.LowPercent = PPSettings.AutoExposureLowPercent;
    CapturedData.Exposure.HighPercent = PPSettings.AutoExposureHighPercent;
    CapturedData.Exposure.HistogramLogMin = PPSettings.HistogramLogMin;
    CapturedData.Exposure.HistogramLogMax = PPSettings.HistogramLogMax;
    CapturedData.Exposure.Compensation = PPSettings.AutoExposureBias;
    
    return CapturedData;
}

FForge2PostProcessPresetData UForge2PostProcessManager::LerpPresets(
    const FForge2PostProcessPresetData& A,
    const FForge2PostProcessPresetData& B,
    float Alpha)
{
    FForge2PostProcessPresetData Result;
    Result.PresetName = TEXT("Lerped");
    
    // Helper lambda for FLinearColor lerp
    auto LerpColor = [Alpha](const FLinearColor& ColorA, const FLinearColor& ColorB) -> FLinearColor
    {
        return FLinearColor(
            FMath::Lerp(ColorA.R, ColorB.R, Alpha),
            FMath::Lerp(ColorA.G, ColorB.G, Alpha),
            FMath::Lerp(ColorA.B, ColorB.B, Alpha),
            FMath::Lerp(ColorA.A, ColorB.A, Alpha)
        );
    };
    
    // Lerp Bloom
    Result.Bloom.Intensity = FMath::Lerp(A.Bloom.Intensity, B.Bloom.Intensity, Alpha);
    Result.Bloom.Threshold = FMath::Lerp(A.Bloom.Threshold, B.Bloom.Threshold, Alpha);
    Result.Bloom.SizeScale = FMath::Lerp(A.Bloom.SizeScale, B.Bloom.SizeScale, Alpha);
    Result.Bloom.ConvolutionScatterDispersion = FMath::Lerp(A.Bloom.ConvolutionScatterDispersion, B.Bloom.ConvolutionScatterDispersion, Alpha);
    Result.Bloom.Tint = LerpColor(A.Bloom.Tint, B.Bloom.Tint);
    
    // Lerp Color Grading - Global
    Result.ColorGrading.Temperature = FMath::Lerp(A.ColorGrading.Temperature, B.ColorGrading.Temperature, Alpha);
    Result.ColorGrading.Tint = FMath::Lerp(A.ColorGrading.Tint, B.ColorGrading.Tint, Alpha);
    Result.ColorGrading.ColorSaturation = LerpColor(A.ColorGrading.ColorSaturation, B.ColorGrading.ColorSaturation);
    Result.ColorGrading.ColorContrast = LerpColor(A.ColorGrading.ColorContrast, B.ColorGrading.ColorContrast);
    Result.ColorGrading.ColorGamma = LerpColor(A.ColorGrading.ColorGamma, B.ColorGrading.ColorGamma);
    Result.ColorGrading.ColorGain = LerpColor(A.ColorGrading.ColorGain, B.ColorGrading.ColorGain);
    Result.ColorGrading.ColorOffset = LerpColor(A.ColorGrading.ColorOffset, B.ColorGrading.ColorOffset);
    
    // Lerp Color Grading - Shadows
    Result.ColorGrading.ShadowsSaturation = LerpColor(A.ColorGrading.ShadowsSaturation, B.ColorGrading.ShadowsSaturation);
    Result.ColorGrading.ShadowsContrast = LerpColor(A.ColorGrading.ShadowsContrast, B.ColorGrading.ShadowsContrast);
    Result.ColorGrading.ShadowsGamma = LerpColor(A.ColorGrading.ShadowsGamma, B.ColorGrading.ShadowsGamma);
    Result.ColorGrading.ShadowsGain = LerpColor(A.ColorGrading.ShadowsGain, B.ColorGrading.ShadowsGain);
    Result.ColorGrading.ShadowsOffset = LerpColor(A.ColorGrading.ShadowsOffset, B.ColorGrading.ShadowsOffset);
    Result.ColorGrading.ShadowsMax = FMath::Lerp(A.ColorGrading.ShadowsMax, B.ColorGrading.ShadowsMax, Alpha);
    
    // Lerp Color Grading - Midtones
    Result.ColorGrading.MidtonesSaturation = LerpColor(A.ColorGrading.MidtonesSaturation, B.ColorGrading.MidtonesSaturation);
    Result.ColorGrading.MidtonesContrast = LerpColor(A.ColorGrading.MidtonesContrast, B.ColorGrading.MidtonesContrast);
    Result.ColorGrading.MidtonesGamma = LerpColor(A.ColorGrading.MidtonesGamma, B.ColorGrading.MidtonesGamma);
    Result.ColorGrading.MidtonesGain = LerpColor(A.ColorGrading.MidtonesGain, B.ColorGrading.MidtonesGain);
    Result.ColorGrading.MidtonesOffset = LerpColor(A.ColorGrading.MidtonesOffset, B.ColorGrading.MidtonesOffset);
    
    // Lerp Color Grading - Highlights
    Result.ColorGrading.HighlightsSaturation = LerpColor(A.ColorGrading.HighlightsSaturation, B.ColorGrading.HighlightsSaturation);
    Result.ColorGrading.HighlightsContrast = LerpColor(A.ColorGrading.HighlightsContrast, B.ColorGrading.HighlightsContrast);
    Result.ColorGrading.HighlightsGamma = LerpColor(A.ColorGrading.HighlightsGamma, B.ColorGrading.HighlightsGamma);
    Result.ColorGrading.HighlightsGain = LerpColor(A.ColorGrading.HighlightsGain, B.ColorGrading.HighlightsGain);
    Result.ColorGrading.HighlightsOffset = LerpColor(A.ColorGrading.HighlightsOffset, B.ColorGrading.HighlightsOffset);
    Result.ColorGrading.HighlightsMin = FMath::Lerp(A.ColorGrading.HighlightsMin, B.ColorGrading.HighlightsMin, Alpha);
    
    // LUT - use target if alpha > 0.5 (can't blend LUT textures)
    Result.ColorGrading.LookupTable = Alpha > 0.5f ? B.ColorGrading.LookupTable : A.ColorGrading.LookupTable;
    Result.ColorGrading.LUTIntensity = FMath::Lerp(A.ColorGrading.LUTIntensity, B.ColorGrading.LUTIntensity, Alpha);
    
    // Lerp DOF
    Result.DepthOfField.FocalDistance = FMath::Lerp(A.DepthOfField.FocalDistance, B.DepthOfField.FocalDistance, Alpha);
    Result.DepthOfField.FocalRegion = FMath::Lerp(A.DepthOfField.FocalRegion, B.DepthOfField.FocalRegion, Alpha);
    Result.DepthOfField.NearTransitionRegion = FMath::Lerp(A.DepthOfField.NearTransitionRegion, B.DepthOfField.NearTransitionRegion, Alpha);
    Result.DepthOfField.FarTransitionRegion = FMath::Lerp(A.DepthOfField.FarTransitionRegion, B.DepthOfField.FarTransitionRegion, Alpha);
    Result.DepthOfField.DepthBlurAmount = FMath::Lerp(A.DepthOfField.DepthBlurAmount, B.DepthOfField.DepthBlurAmount, Alpha);
    Result.DepthOfField.DepthBlurRadius = FMath::Lerp(A.DepthOfField.DepthBlurRadius, B.DepthOfField.DepthBlurRadius, Alpha);
    Result.DepthOfField.Aperture = FMath::Lerp(A.DepthOfField.Aperture, B.DepthOfField.Aperture, Alpha);
    Result.DepthOfField.BladeCount = FMath::RoundToInt(FMath::Lerp((float)A.DepthOfField.BladeCount, (float)B.DepthOfField.BladeCount, Alpha));
    Result.DepthOfField.BokehMinSize = FMath::Lerp(A.DepthOfField.BokehMinSize, B.DepthOfField.BokehMinSize, Alpha);
    Result.DepthOfField.BokehMaxSize = FMath::Lerp(A.DepthOfField.BokehMaxSize, B.DepthOfField.BokehMaxSize, Alpha);
    
    // Lerp Motion Blur
    Result.MotionBlur.Amount = FMath::Lerp(A.MotionBlur.Amount, B.MotionBlur.Amount, Alpha);
    Result.MotionBlur.Max = FMath::Lerp(A.MotionBlur.Max, B.MotionBlur.Max, Alpha);
    Result.MotionBlur.TargetFPS = FMath::RoundToInt(FMath::Lerp((float)A.MotionBlur.TargetFPS, (float)B.MotionBlur.TargetFPS, Alpha));
    Result.MotionBlur.PerObjectSize = FMath::Lerp(A.MotionBlur.PerObjectSize, B.MotionBlur.PerObjectSize, Alpha);
    
    // Lerp Ambient Occlusion
    Result.AmbientOcclusion.Intensity = FMath::Lerp(A.AmbientOcclusion.Intensity, B.AmbientOcclusion.Intensity, Alpha);
    Result.AmbientOcclusion.Radius = FMath::Lerp(A.AmbientOcclusion.Radius, B.AmbientOcclusion.Radius, Alpha);
    Result.AmbientOcclusion.bRadiusInWorldSpace = Alpha > 0.5f ? B.AmbientOcclusion.bRadiusInWorldSpace : A.AmbientOcclusion.bRadiusInWorldSpace;
    Result.AmbientOcclusion.MipBlend = FMath::Lerp(A.AmbientOcclusion.MipBlend, B.AmbientOcclusion.MipBlend, Alpha);
    Result.AmbientOcclusion.MipScale = FMath::Lerp(A.AmbientOcclusion.MipScale, B.AmbientOcclusion.MipScale, Alpha);
    Result.AmbientOcclusion.MipThreshold = FMath::Lerp(A.AmbientOcclusion.MipThreshold, B.AmbientOcclusion.MipThreshold, Alpha);
    Result.AmbientOcclusion.Power = FMath::Lerp(A.AmbientOcclusion.Power, B.AmbientOcclusion.Power, Alpha);
    Result.AmbientOcclusion.Bias = FMath::Lerp(A.AmbientOcclusion.Bias, B.AmbientOcclusion.Bias, Alpha);
    Result.AmbientOcclusion.Quality = FMath::Lerp(A.AmbientOcclusion.Quality, B.AmbientOcclusion.Quality, Alpha);
    Result.AmbientOcclusion.StaticFraction = LerpColor(A.AmbientOcclusion.StaticFraction, B.AmbientOcclusion.StaticFraction);
    
    // Lerp Screen Effects
    Result.ScreenEffects.VignetteIntensity = FMath::Lerp(A.ScreenEffects.VignetteIntensity, B.ScreenEffects.VignetteIntensity, Alpha);
    Result.ScreenEffects.VignetteColor = LerpColor(A.ScreenEffects.VignetteColor, B.ScreenEffects.VignetteColor);
    Result.ScreenEffects.GrainIntensity = FMath::Lerp(A.ScreenEffects.GrainIntensity, B.ScreenEffects.GrainIntensity, Alpha);
    Result.ScreenEffects.GrainJitter = FMath::Lerp(A.ScreenEffects.GrainJitter, B.ScreenEffects.GrainJitter, Alpha);
    Result.ScreenEffects.ChromaticAberrationIntensity = FMath::Lerp(A.ScreenEffects.ChromaticAberrationIntensity, B.ScreenEffects.ChromaticAberrationIntensity, Alpha);
    Result.ScreenEffects.ChromaticAberrationStartOffset = FMath::Lerp(A.ScreenEffects.ChromaticAberrationStartOffset, B.ScreenEffects.ChromaticAberrationStartOffset, Alpha);
    Result.ScreenEffects.LensFlareIntensity = FMath::Lerp(A.ScreenEffects.LensFlareIntensity, B.ScreenEffects.LensFlareIntensity, Alpha);
    Result.ScreenEffects.LensFlareTint = LerpColor(A.ScreenEffects.LensFlareTint, B.ScreenEffects.LensFlareTint);
    Result.ScreenEffects.LensFlareBokehSize = FMath::Lerp(A.ScreenEffects.LensFlareBokehSize, B.ScreenEffects.LensFlareBokehSize, Alpha);
    Result.ScreenEffects.LensFlareThreshold = FMath::Lerp(A.ScreenEffects.LensFlareThreshold, B.ScreenEffects.LensFlareThreshold, Alpha);
    
    // Lerp Exposure
    Result.Exposure.MinBrightness = FMath::Lerp(A.Exposure.MinBrightness, B.Exposure.MinBrightness, Alpha);
    Result.Exposure.MaxBrightness = FMath::Lerp(A.Exposure.MaxBrightness, B.Exposure.MaxBrightness, Alpha);
    Result.Exposure.SpeedUp = FMath::Lerp(A.Exposure.SpeedUp, B.Exposure.SpeedUp, Alpha);
    Result.Exposure.SpeedDown = FMath::Lerp(A.Exposure.SpeedDown, B.Exposure.SpeedDown, Alpha);
    Result.Exposure.LowPercent = FMath::Lerp(A.Exposure.LowPercent, B.Exposure.LowPercent, Alpha);
    Result.Exposure.HighPercent = FMath::Lerp(A.Exposure.HighPercent, B.Exposure.HighPercent, Alpha);
    Result.Exposure.HistogramLogMin = FMath::Lerp(A.Exposure.HistogramLogMin, B.Exposure.HistogramLogMin, Alpha);
    Result.Exposure.HistogramLogMax = FMath::Lerp(A.Exposure.HistogramLogMax, B.Exposure.HistogramLogMax, Alpha);
    Result.Exposure.Compensation = FMath::Lerp(A.Exposure.Compensation, B.Exposure.Compensation, Alpha);
    
    // Blend weight
    Result.BlendWeight = FMath::Lerp(A.BlendWeight, B.BlendWeight, Alpha);
    
    return Result;
}
