// Forge2MaterialSystem.h - Material System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2MaterialSystem.generated.h"

// Forward declarations
class UMaterial;
class UMaterialInstance;
class UMaterialInstanceDynamic;
class UMaterialFunction;
class UTexture;
class UTexture2D;

// Enums
UENUM(BlueprintType)
enum class EForge2MaterialDomain : uint8
{
    Surface,
    DeferredDecal,
    LightFunction,
    Volume,
    PostProcess,
    UI
};

UENUM(BlueprintType)
enum class EForge2BlendMode : uint8
{
    Opaque,
    Masked,
    Translucent,
    Additive,
    Modulate,
    AlphaComposite,
    AlphaHoldout
};

UENUM(BlueprintType)
enum class EForge2ShadingModel : uint8
{
    DefaultLit,
    Unlit,
    Subsurface,
    SubsurfaceProfile,
    PreintegratedSkin,
    ClearCoat,
    TwoSidedFoliage,
    Hair,
    Cloth,
    Eye,
    ThinTranslucent,
    SingleLayerWater
};

UENUM(BlueprintType)
enum class EForge2MaterialParameterType : uint8
{
    Scalar,
    Vector,
    Texture,
    StaticSwitch,
    StaticComponentMask
};

UENUM(BlueprintType)
enum class EForge2LandscapeLayerBlend : uint8
{
    WeightBlend,
    AlphaBlend,
    HeightBlend
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MaterialSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2MaterialDomain Domain = EForge2MaterialDomain::Surface;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BlendMode BlendMode = EForge2BlendMode::Opaque;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2ShadingModel ShadingModel = EForge2ShadingModel::DefaultLit;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bTwoSided = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseMaterialAttributes = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float OpacityMaskClipValue = 0.333f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCastShadowsAsMasked = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableResponsiveAA = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bScreenSpaceReflections = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bContactShadows = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MaterialParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ParameterName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2MaterialParameterType Type = EForge2MaterialParameterType::Scalar;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ScalarValue = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor VectorValue = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture> TextureValue;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool StaticSwitchValue = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Group;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SortPriority = 0;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PBRTextureSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> BaseColor;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Normal;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Roughness;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Metallic;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> AmbientOcclusion;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Height;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Emissive;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> ORMPacked;  // Occlusion/Roughness/Metallic
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SubsurfaceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor SubsurfaceColor = FLinearColor(1.0f, 0.3f, 0.1f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Opacity = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SubsurfaceRadius = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SubsurfaceProfileAsset;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LandscapeLayerSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName LayerName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2PBRTextureSet Textures;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Tiling = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2LandscapeLayerBlend BlendType = EForge2LandscapeLayerBlend::WeightBlend;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeightOffset = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeightBlendSharpness = 0.5f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DecalSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeScreenSize = 0.01f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeStartDelay = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeDuration = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bProjectOnSkeletal = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SortOrder = 0;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ProceduralTextureSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString PatternType = TEXT("Noise");  // Noise, Gradient, Checker, Voronoi
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Scale = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Octaves = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Persistence = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Lacunarity = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor Color1 = FLinearColor::Black;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor Color2 = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Seed = 0;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2MaterialManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2MaterialManager();

    // Material Creation
    UFUNCTION(BlueprintCallable, Category = "Material|Creation")
    UMaterial* CreateMaterial(const FString& MaterialName, const FForge2MaterialSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Material|Creation")
    UMaterialInstanceDynamic* CreateMaterialInstanceDynamic(UMaterialInterface* ParentMaterial,
                                                              const FString& InstanceName);

    UFUNCTION(BlueprintCallable, Category = "Material|Creation")
    UMaterial* CreatePBRMaterial(const FString& MaterialName, const FForge2PBRTextureSet& Textures);

    // Parameter Management
    UFUNCTION(BlueprintCallable, Category = "Material|Parameters")
    void SetScalarParameter(UMaterialInstanceDynamic* Material, FName ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Material|Parameters")
    void SetVectorParameter(UMaterialInstanceDynamic* Material, FName ParameterName, FLinearColor Value);

    UFUNCTION(BlueprintCallable, Category = "Material|Parameters")
    void SetTextureParameter(UMaterialInstanceDynamic* Material, FName ParameterName, UTexture* Texture);

    UFUNCTION(BlueprintCallable, Category = "Material|Parameters")
    void SetMultipleParameters(UMaterialInstanceDynamic* Material, const TArray<FForge2MaterialParameter>& Parameters);

    UFUNCTION(BlueprintCallable, Category = "Material|Parameters")
    float GetScalarParameter(UMaterialInstanceDynamic* Material, FName ParameterName);

    UFUNCTION(BlueprintCallable, Category = "Material|Parameters")
    FLinearColor GetVectorParameter(UMaterialInstanceDynamic* Material, FName ParameterName);

    // Material Interpolation
    UFUNCTION(BlueprintCallable, Category = "Material|Animation")
    void InterpolateScalarParameter(UMaterialInstanceDynamic* Material, FName ParameterName,
                                      float TargetValue, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Material|Animation")
    void InterpolateVectorParameter(UMaterialInstanceDynamic* Material, FName ParameterName,
                                      FLinearColor TargetValue, float Duration);

    // Landscape Materials
    UFUNCTION(BlueprintCallable, Category = "Material|Landscape")
    UMaterial* CreateLandscapeMaterial(const FString& MaterialName,
                                         const TArray<FForge2LandscapeLayerSettings>& Layers);

    UFUNCTION(BlueprintCallable, Category = "Material|Landscape")
    void AddLandscapeLayer(UMaterial* LandscapeMaterial, const FForge2LandscapeLayerSettings& Layer);

    UFUNCTION(BlueprintCallable, Category = "Material|Landscape")
    void ConfigureAutoMaterial(UMaterial* LandscapeMaterial, float SlopeThreshold, float HeightThreshold);

    // Decal Materials
    UFUNCTION(BlueprintCallable, Category = "Material|Decal")
    UMaterial* CreateDecalMaterial(const FString& MaterialName, const FForge2DecalSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Material|Decal")
    void ConfigureDecal(UMaterial* DecalMaterial, const FForge2DecalSettings& Settings);

    // Subsurface Materials
    UFUNCTION(BlueprintCallable, Category = "Material|Subsurface")
    void ConfigureSubsurface(UMaterial* Material, const FForge2SubsurfaceSettings& Settings);

    // Procedural Textures
    UFUNCTION(BlueprintCallable, Category = "Material|Procedural")
    UTexture2D* CreateProceduralTexture(const FString& TextureName,
                                          const FForge2ProceduralTextureSettings& Settings,
                                          int32 Width, int32 Height);

    UFUNCTION(BlueprintCallable, Category = "Material|Procedural")
    UMaterialFunction* CreateNoiseFunction(const FString& FunctionName, int32 Octaves, float Persistence);

    // Material Presets
    UFUNCTION(BlueprintCallable, Category = "Material|Presets")
    UMaterial* CreateMetalMaterial(const FString& MaterialName, FLinearColor BaseColor, float Roughness);

    UFUNCTION(BlueprintCallable, Category = "Material|Presets")
    UMaterial* CreateGlassMaterial(const FString& MaterialName, float Opacity, float Roughness,
                                     FLinearColor Tint);

    UFUNCTION(BlueprintCallable, Category = "Material|Presets")
    UMaterial* CreateEmissiveMaterial(const FString& MaterialName, FLinearColor EmissiveColor,
                                        float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Material|Presets")
    UMaterial* CreateClothMaterial(const FString& MaterialName, const FForge2PBRTextureSet& Textures);

    UFUNCTION(BlueprintCallable, Category = "Material|Presets")
    UMaterial* CreateSkinMaterial(const FString& MaterialName, const FForge2SubsurfaceSettings& SSS);

private:
    void SetupMaterialNode(UMaterial* Material, const FForge2MaterialSettings& Settings);
    void ConnectPBRTextures(UMaterial* Material, const FForge2PBRTextureSet& Textures);
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2MaterialUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2MaterialSettings CreateDefaultSettings();

    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2MaterialSettings CreateTranslucentSettings(float Opacity);

    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2MaterialSettings CreateMaskedSettings(float ClipValue);

    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2SubsurfaceSettings CreateSkinSSS();

    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2SubsurfaceSettings CreateFoliageSSS();

    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2SubsurfaceSettings CreateWaxSSS();

    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2DecalSettings CreateDefaultDecalSettings();

    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2DecalSettings CreateBloodDecalSettings();

    UFUNCTION(BlueprintCallable, Category = "Material|Utility")
    static FForge2DecalSettings CreateBulletHoleDecalSettings();
};
