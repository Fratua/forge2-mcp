// Forge2PostProcessSystem.h - Post-Processing System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2PostProcessSystem.generated.h"

// Forward declarations
class UPostProcessComponent;
class APostProcessVolume;

// Enums
UENUM(BlueprintType)
enum class EForge2BloomMethod : uint8
{
    Standard,
    Convolution
};

UENUM(BlueprintType)
enum class EForge2DOFMethod : uint8
{
    Gaussian,
    Bokeh,
    CircleDOF
};

UENUM(BlueprintType)
enum class EForge2MotionBlurQuality : uint8
{
    Low,
    Medium,
    High,
    VeryHigh
};

UENUM(BlueprintType)
enum class EForge2AmbientOcclusionMethod : uint8
{
    SSAO,
    GTAO
};

UENUM(BlueprintType)
enum class EForge2ToneCurveType : uint8
{
    ACES,
    Filmic,
    Reinhard,
    Linear
};

UENUM(BlueprintType)
enum class EForge2PostProcessPreset : uint8
{
    Realistic,
    Cinematic,
    Stylized,
    Horror,
    Vibrant,
    Noir,
    Retro,
    Custom
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BloomSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BloomMethod Method = EForge2BloomMethod::Standard;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Intensity = 0.675f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Threshold = -1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SizeScale = 4.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor Tint = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConvolutionScatterDispersion = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bConvolutionAllowFastPath = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ColorGradingSettings
{
    GENERATED_BODY()

    // White Balance
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Temperature = 6500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Tint = 0.0f;

    // Global
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ColorSaturation = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ColorContrast = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ColorGamma = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ColorGain = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ColorOffset = FLinearColor(0, 0, 0, 0);

    // Shadows
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ShadowsSaturation = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ShadowsContrast = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ShadowsGamma = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ShadowsGain = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ShadowsOffset = FLinearColor(0, 0, 0, 0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ShadowsMax = 0.09f;

    // Midtones
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MidtonesSaturation = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MidtonesContrast = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MidtonesGamma = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MidtonesGain = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MidtonesOffset = FLinearColor(0, 0, 0, 0);

    // Highlights
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor HighlightsSaturation = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor HighlightsContrast = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor HighlightsGamma = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor HighlightsGain = FLinearColor(1, 1, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor HighlightsOffset = FLinearColor(0, 0, 0, 0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HighlightsMin = 0.5f;

    // Tone Curve
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2ToneCurveType ToneCurve = EForge2ToneCurveType::ACES;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ToeStrength = 0.55f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ShoulderStrength = 0.26f;

    // LUT
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture> LookupTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LUTIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DOFSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2DOFMethod Method = EForge2DOFMethod::CircleDOF;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FocalDistance = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FocalRegion = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NearTransitionRegion = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FarTransitionRegion = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DepthBlurAmount = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DepthBlurRadius = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Aperture = 4.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BladeCount = 5;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BokehMinSize = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BokehMaxSize = 64.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MotionBlurSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Amount = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Max = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TargetFPS = 30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PerObjectSize = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2MotionBlurQuality Quality = EForge2MotionBlurQuality::Medium;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AmbientOcclusionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2AmbientOcclusionMethod Method = EForge2AmbientOcclusionMethod::GTAO;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Intensity = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRadiusInWorldSpace = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MipBlend = 0.6f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MipScale = 1.7f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MipThreshold = 0.01f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Power = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Bias = 3.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Quality = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor StaticFraction = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ScreenEffectsSettings
{
    GENERATED_BODY()

    // Vignette
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VignetteIntensity = 0.4f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor VignetteColor = FLinearColor::Black;

    // Film Grain
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GrainIntensity = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GrainJitter = 0.0f;

    // Chromatic Aberration
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ChromaticAberrationIntensity = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ChromaticAberrationStartOffset = 0.0f;

    // Lens Flare
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LensFlareIntensity = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor LensFlareTint = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LensFlareBokehSize = 3.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LensFlareThreshold = 8.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ExposureSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoExposure = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinBrightness = 0.03f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxBrightness = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpeedUp = 3.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpeedDown = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LowPercent = 80.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HighPercent = 98.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HistogramLogMin = -8.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HistogramLogMax = 4.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Compensation = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Density = 0.02f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeightFalloff = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float StartDistance = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor InscatteringColor = FLinearColor(0.447f, 0.638f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxOpacity = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CutoffDistance = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FogHeight = 0.0f;

    // Volumetric Fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bVolumetricFog = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VolumetricFogScatteringDistribution = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VolumetricFogExtinctionScale = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VolumetricFogDistance = 6000.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PostProcessPresetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString PresetName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2PostProcessPreset PresetType = EForge2PostProcessPreset::Custom;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2BloomSettings Bloom;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2ColorGradingSettings ColorGrading;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2DOFSettings DepthOfField;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2MotionBlurSettings MotionBlur;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2AmbientOcclusionSettings AmbientOcclusion;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2ScreenEffectsSettings ScreenEffects;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2ExposureSettings Exposure;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2FogSettings Fog;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendWeight = 1.0f;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2PostProcessManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2PostProcessManager();

    // Volume Creation
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Volume")
    APostProcessVolume* CreatePostProcessVolume(FVector Location, FVector Extent, bool bUnbound);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Volume")
    void ConfigureVolumePriority(APostProcessVolume* Volume, float Priority, float BlendRadius);

    // Bloom
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Bloom")
    void ConfigureBloom(APostProcessVolume* Volume, const FForge2BloomSettings& Settings);

    // Color Grading
    UFUNCTION(BlueprintCallable, Category = "PostProcess|ColorGrading")
    void ConfigureColorGrading(APostProcessVolume* Volume, const FForge2ColorGradingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|ColorGrading")
    void ApplyLUT(APostProcessVolume* Volume, UTexture* LUT, float Intensity);

    // Depth of Field
    UFUNCTION(BlueprintCallable, Category = "PostProcess|DOF")
    void ConfigureDepthOfField(APostProcessVolume* Volume, const FForge2DOFSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|DOF")
    void SetFocalDistance(APostProcessVolume* Volume, float Distance);

    // Motion Blur
    UFUNCTION(BlueprintCallable, Category = "PostProcess|MotionBlur")
    void ConfigureMotionBlur(APostProcessVolume* Volume, const FForge2MotionBlurSettings& Settings);

    // Ambient Occlusion
    UFUNCTION(BlueprintCallable, Category = "PostProcess|AO")
    void ConfigureAmbientOcclusion(APostProcessVolume* Volume, const FForge2AmbientOcclusionSettings& Settings);

    // Screen Effects
    UFUNCTION(BlueprintCallable, Category = "PostProcess|ScreenEffects")
    void ConfigureScreenEffects(APostProcessVolume* Volume, const FForge2ScreenEffectsSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|ScreenEffects")
    void SetVignette(APostProcessVolume* Volume, float Intensity, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|ScreenEffects")
    void SetFilmGrain(APostProcessVolume* Volume, float Intensity, float Jitter);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|ScreenEffects")
    void SetChromaticAberration(APostProcessVolume* Volume, float Intensity, float StartOffset);

    // Exposure
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Exposure")
    void ConfigureExposure(APostProcessVolume* Volume, const FForge2ExposureSettings& Settings);

    // Fog
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Fog")
    void ConfigureFog(APostProcessVolume* Volume, const FForge2FogSettings& Settings);

    // Presets
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Presets")
    void ApplyPreset(APostProcessVolume* Volume, EForge2PostProcessPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Presets")
    FForge2PostProcessPresetData GetPresetData(EForge2PostProcessPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Presets")
    FForge2PostProcessPresetData GetCinematicPreset();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Presets")
    FForge2PostProcessPresetData GetHorrorPreset();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Presets")
    FForge2PostProcessPresetData GetNoirPreset();

    // Blending
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Blend")
    void BlendToPreset(APostProcessVolume* Volume, const FForge2PostProcessPresetData& TargetPreset, float Duration);

private:
    void ApplySettings(APostProcessVolume* Volume, const FForge2PostProcessPresetData& Settings);
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2PostProcessUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    static FForge2BloomSettings CreateDefaultBloom();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    static FForge2ColorGradingSettings CreateDefaultColorGrading();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    static FForge2ColorGradingSettings CreateWarmColorGrading();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    static FForge2ColorGradingSettings CreateCoolColorGrading();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    static FForge2ColorGradingSettings CreateDesaturatedColorGrading();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    static FForge2DOFSettings CreateCinematicDOF();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    static FForge2DOFSettings CreateTiltShiftDOF();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    static FForge2ScreenEffectsSettings CreateDramaticScreenEffects();
};
