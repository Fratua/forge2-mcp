// Forge2MaterialSystem.cpp - Material System Implementation
#include "Forge2MaterialSystem.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionNoise.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionSmoothStep.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionDepthFade.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionFunctionInput.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "Materials/MaterialExpressionLandscapeLayerCoords.h"
#include "Engine/Texture2D.h"
#include "Engine/SubsurfaceProfile.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

// UForge2MaterialManager Implementation
UForge2MaterialManager::UForge2MaterialManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UMaterial* UForge2MaterialManager::CreateMaterial(const FString& MaterialName,
                                                    const FForge2MaterialSettings& Settings)
{
#if WITH_EDITOR
    // Create material package and asset
    FString PackagePath = TEXT("/Game/Materials/Generated/") + MaterialName;
    UPackage* Package = CreatePackage(*PackagePath);
    
    UMaterial* NewMaterial = NewObject<UMaterial>(Package, *MaterialName, RF_Public | RF_Standalone);
    if (NewMaterial)
    {
        // Set material domain and blend mode
        NewMaterial->MaterialDomain = Settings.MaterialDomain;
        NewMaterial->BlendMode = Settings.BlendMode;
        NewMaterial->SetShadingModel(Settings.ShadingModel);
        NewMaterial->TwoSided = Settings.bTwoSided;
        
        // Setup material expression nodes
        SetupMaterialNode(NewMaterial, Settings);
        
        // Compile the material
        NewMaterial->PreEditChange(nullptr);
        NewMaterial->PostEditChange();
        
        // Mark package dirty
        Package->MarkPackageDirty();
        
        // Save the asset
        FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
        UPackage::SavePackage(Package, NewMaterial, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);
    }
    return NewMaterial;
#else
    return nullptr;
#endif
}

UMaterialInstanceDynamic* UForge2MaterialManager::CreateMaterialInstanceDynamic(
    UMaterialInterface* ParentMaterial, const FString& InstanceName)
{
    if (!ParentMaterial) return nullptr;

    UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(ParentMaterial, this);
    return MID;
}

UMaterial* UForge2MaterialManager::CreatePBRMaterial(const FString& MaterialName,
                                                       const FForge2PBRTextureSet& Textures)
{
    FForge2MaterialSettings Settings = UForge2MaterialUtility::CreateDefaultSettings();
    UMaterial* Material = CreateMaterial(MaterialName, Settings);

    if (Material)
    {
        ConnectPBRTextures(Material, Textures);
    }

    return Material;
}

void UForge2MaterialManager::SetScalarParameter(UMaterialInstanceDynamic* Material,
                                                  FName ParameterName, float Value)
{
    if (Material)
    {
        Material->SetScalarParameterValue(ParameterName, Value);
    }
}

void UForge2MaterialManager::SetVectorParameter(UMaterialInstanceDynamic* Material,
                                                  FName ParameterName, FLinearColor Value)
{
    if (Material)
    {
        Material->SetVectorParameterValue(ParameterName, Value);
    }
}

void UForge2MaterialManager::SetTextureParameter(UMaterialInstanceDynamic* Material,
                                                   FName ParameterName, UTexture* Texture)
{
    if (Material && Texture)
    {
        Material->SetTextureParameterValue(ParameterName, Texture);
    }
}

void UForge2MaterialManager::SetMultipleParameters(UMaterialInstanceDynamic* Material,
                                                     const TArray<FForge2MaterialParameter>& Parameters)
{
    if (!Material) return;

    for (const FForge2MaterialParameter& Param : Parameters)
    {
        switch (Param.Type)
        {
            case EForge2MaterialParameterType::Scalar:
                Material->SetScalarParameterValue(Param.ParameterName, Param.ScalarValue);
                break;

            case EForge2MaterialParameterType::Vector:
                Material->SetVectorParameterValue(Param.ParameterName, Param.VectorValue);
                break;

            case EForge2MaterialParameterType::Texture:
                if (Param.TextureValue.IsValid())
                {
                    UTexture* Texture = Param.TextureValue.LoadSynchronous();
                    if (Texture)
                    {
                        Material->SetTextureParameterValue(Param.ParameterName, Texture);
                    }
                }
                break;

            default:
                break;
        }
    }
}

float UForge2MaterialManager::GetScalarParameter(UMaterialInstanceDynamic* Material, FName ParameterName)
{
    if (!Material) return 0.0f;

    float Value = 0.0f;
    Material->GetScalarParameterValue(ParameterName, Value);
    return Value;
}

FLinearColor UForge2MaterialManager::GetVectorParameter(UMaterialInstanceDynamic* Material,
                                                          FName ParameterName)
{
    if (!Material) return FLinearColor::Black;

    FLinearColor Value = FLinearColor::Black;
    Material->GetVectorParameterValue(ParameterName, Value);
    return Value;
}

void UForge2MaterialManager::InterpolateScalarParameter(UMaterialInstanceDynamic* Material,
                                                          FName ParameterName,
                                                          float TargetValue, float Duration)
{
    if (!Material || !GetWorld()) return;

    // Get current value
    float StartValue = GetScalarParameter(Material, ParameterName);
    
    // Store interpolation data
    FMaterialParameterInterpolation Interp;
    Interp.Material = Material;
    Interp.ParameterName = ParameterName;
    Interp.StartScalar = StartValue;
    Interp.TargetScalar = TargetValue;
    Interp.Duration = Duration;
    Interp.ElapsedTime = 0.0f;
    Interp.bIsVector = false;
    
    ActiveInterpolations.Add(Interp);
    
    // Start tick if not already ticking
    if (!PrimaryComponentTick.IsTickFunctionEnabled())
    {
        PrimaryComponentTick.SetTickFunctionEnable(true);
    }
}

void UForge2MaterialManager::InterpolateVectorParameter(UMaterialInstanceDynamic* Material,
                                                          FName ParameterName,
                                                          FLinearColor TargetValue, float Duration)
{
    if (!Material || !GetWorld()) return;

    // Get current value
    FLinearColor StartValue = GetVectorParameter(Material, ParameterName);
    
    // Store interpolation data
    FMaterialParameterInterpolation Interp;
    Interp.Material = Material;
    Interp.ParameterName = ParameterName;
    Interp.StartVector = StartValue;
    Interp.TargetVector = TargetValue;
    Interp.Duration = Duration;
    Interp.ElapsedTime = 0.0f;
    Interp.bIsVector = true;
    
    ActiveInterpolations.Add(Interp);
    
    // Start tick if not already ticking
    if (!PrimaryComponentTick.IsTickFunctionEnabled())
    {
        PrimaryComponentTick.SetTickFunctionEnable(true);
    }
}

UMaterial* UForge2MaterialManager::CreateLandscapeMaterial(const FString& MaterialName,
                                                             const TArray<FForge2LandscapeLayerSettings>& Layers)
{
    FForge2MaterialSettings Settings;
    Settings.Domain = EForge2MaterialDomain::Surface;
    Settings.BlendMode = EForge2BlendMode::Opaque;

    UMaterial* Material = CreateMaterial(MaterialName, Settings);

    if (Material)
    {
        for (const FForge2LandscapeLayerSettings& Layer : Layers)
        {
            AddLandscapeLayer(Material, Layer);
        }
    }

    return Material;
}

void UForge2MaterialManager::AddLandscapeLayer(UMaterial* LandscapeMaterial,
                                                 const FForge2LandscapeLayerSettings& Layer)
{
    if (!LandscapeMaterial) return;

#if WITH_EDITOR
    // Create landscape layer blend node
    UMaterialExpressionLandscapeLayerBlend* LayerBlend = nullptr;
    
    // Find existing layer blend or create new one
    for (UMaterialExpression* Expr : LandscapeMaterial->GetExpressionCollection().Expressions)
    {
        if (UMaterialExpressionLandscapeLayerBlend* ExistingBlend = Cast<UMaterialExpressionLandscapeLayerBlend>(Expr))
        {
            LayerBlend = ExistingBlend;
            break;
        }
    }
    
    if (!LayerBlend)
    {
        LayerBlend = NewObject<UMaterialExpressionLandscapeLayerBlend>(LandscapeMaterial);
        LandscapeMaterial->GetExpressionCollection().AddExpression(LayerBlend);
    }
    
    // Add the layer to the blend node
    FLayerBlendInput NewLayer;
    NewLayer.LayerName = Layer.LayerName;
    NewLayer.BlendType = Layer.bUseHeightBlend ? LB_HeightBlend : LB_WeightBlend;
    NewLayer.PreviewWeight = 1.0f;
    LayerBlend->Layers.Add(NewLayer);
    
    // Create texture sampler for this layer's base color
    if (Layer.BaseColorTexture)
    {
        UMaterialExpressionTextureSample* BaseColorSampler = NewObject<UMaterialExpressionTextureSample>(LandscapeMaterial);
        BaseColorSampler->Texture = Layer.BaseColorTexture;
        BaseColorSampler->SamplerType = SAMPLERTYPE_Color;
        LandscapeMaterial->GetExpressionCollection().AddExpression(BaseColorSampler);
        
        // Create landscape coord node for proper UV tiling
        UMaterialExpressionLandscapeLayerCoords* LayerCoords = NewObject<UMaterialExpressionLandscapeLayerCoords>(LandscapeMaterial);
        LayerCoords->MappingScale = Layer.TextureTiling;
        LandscapeMaterial->GetExpressionCollection().AddExpression(LayerCoords);
        
        BaseColorSampler->Coordinates.Expression = LayerCoords;
    }
    
    // Create texture sampler for normal map
    if (Layer.NormalTexture)
    {
        UMaterialExpressionTextureSample* NormalSampler = NewObject<UMaterialExpressionTextureSample>(LandscapeMaterial);
        NormalSampler->Texture = Layer.NormalTexture;
        NormalSampler->SamplerType = SAMPLERTYPE_Normal;
        LandscapeMaterial->GetExpressionCollection().AddExpression(NormalSampler);
    }
    
    // Connect layer blend to material output
    LandscapeMaterial->GetEditorOnlyData()->BaseColor.Expression = LayerBlend;
    
    // Recompile material
    LandscapeMaterial->PreEditChange(nullptr);
    LandscapeMaterial->PostEditChange();
#endif
}

void UForge2MaterialManager::ConfigureAutoMaterial(UMaterial* LandscapeMaterial,
                                                     float SlopeThreshold, float HeightThreshold)
{
    if (!LandscapeMaterial) return;

#if WITH_EDITOR
    // Create world position node for height-based blending
    UMaterialExpressionWorldPosition* WorldPos = NewObject<UMaterialExpressionWorldPosition>(LandscapeMaterial);
    WorldPos->WorldPositionShaderOffset = WPT_Default;
    LandscapeMaterial->GetExpressionCollection().AddExpression(WorldPos);
    
    // Extract Z component (height) using component mask
    UMaterialExpressionComponentMask* HeightMask = NewObject<UMaterialExpressionComponentMask>(LandscapeMaterial);
    HeightMask->Input.Expression = WorldPos;
    HeightMask->R = false;
    HeightMask->G = false;
    HeightMask->B = true;  // Z is height
    HeightMask->A = false;
    LandscapeMaterial->GetExpressionCollection().AddExpression(HeightMask);
    
    // Create height threshold constant
    UMaterialExpressionConstant* HeightThresholdNode = NewObject<UMaterialExpressionConstant>(LandscapeMaterial);
    HeightThresholdNode->R = HeightThreshold;
    LandscapeMaterial->GetExpressionCollection().AddExpression(HeightThresholdNode);
    
    // Create height blend (smoothstep for gradual transition)
    UMaterialExpressionSmoothStep* HeightBlend = NewObject<UMaterialExpressionSmoothStep>(LandscapeMaterial);
    HeightBlend->Min.Expression = HeightThresholdNode;
    
    UMaterialExpressionConstant* HeightTransition = NewObject<UMaterialExpressionConstant>(LandscapeMaterial);
    HeightTransition->R = HeightThreshold + 500.0f;  // 500 unit transition zone
    LandscapeMaterial->GetExpressionCollection().AddExpression(HeightTransition);
    HeightBlend->Max.Expression = HeightTransition;
    HeightBlend->Value.Expression = HeightMask;
    LandscapeMaterial->GetExpressionCollection().AddExpression(HeightBlend);
    
    // Create vertex normal for slope detection
    UMaterialExpressionVertexNormalWS* VertexNormal = NewObject<UMaterialExpressionVertexNormalWS>(LandscapeMaterial);
    LandscapeMaterial->GetExpressionCollection().AddExpression(VertexNormal);
    
    // Get Z component of normal (flatness indicator: 1 = flat, 0 = vertical)
    UMaterialExpressionComponentMask* NormalZ = NewObject<UMaterialExpressionComponentMask>(LandscapeMaterial);
    NormalZ->Input.Expression = VertexNormal;
    NormalZ->R = false;
    NormalZ->G = false;
    NormalZ->B = true;
    NormalZ->A = false;
    LandscapeMaterial->GetExpressionCollection().AddExpression(NormalZ);
    
    // Create slope threshold (convert angle to normal Z value: cos(angle))
    float SlopeNormalZ = FMath::Cos(FMath::DegreesToRadians(SlopeThreshold));
    UMaterialExpressionConstant* SlopeThresholdNode = NewObject<UMaterialExpressionConstant>(LandscapeMaterial);
    SlopeThresholdNode->R = SlopeNormalZ;
    LandscapeMaterial->GetExpressionCollection().AddExpression(SlopeThresholdNode);
    
    // Slope blend using smoothstep
    UMaterialExpressionSmoothStep* SlopeBlend = NewObject<UMaterialExpressionSmoothStep>(LandscapeMaterial);
    
    UMaterialExpressionConstant* SlopeTransition = NewObject<UMaterialExpressionConstant>(LandscapeMaterial);
    SlopeTransition->R = SlopeNormalZ - 0.1f;  // Transition zone
    LandscapeMaterial->GetExpressionCollection().AddExpression(SlopeTransition);
    
    SlopeBlend->Min.Expression = SlopeTransition;
    SlopeBlend->Max.Expression = SlopeThresholdNode;
    SlopeBlend->Value.Expression = NormalZ;
    LandscapeMaterial->GetExpressionCollection().AddExpression(SlopeBlend);
    
    // Store auto material nodes for later use with layer blending
    // These can be used as alpha inputs for lerp between materials
    
    LandscapeMaterial->PreEditChange(nullptr);
    LandscapeMaterial->PostEditChange();
#endif
}

UMaterial* UForge2MaterialManager::CreateDecalMaterial(const FString& MaterialName,
                                                         const FForge2DecalSettings& Settings)
{
    FForge2MaterialSettings MaterialSettings;
    MaterialSettings.Domain = EForge2MaterialDomain::DeferredDecal;
    MaterialSettings.BlendMode = EForge2BlendMode::Translucent;

    UMaterial* Material = CreateMaterial(MaterialName, MaterialSettings);

    if (Material)
    {
        ConfigureDecal(Material, Settings);
    }

    return Material;
}

void UForge2MaterialManager::ConfigureDecal(UMaterial* DecalMaterial, const FForge2DecalSettings& Settings)
{
    if (!DecalMaterial) return;

#if WITH_EDITOR
    // Ensure material domain is set to deferred decal
    DecalMaterial->MaterialDomain = MD_DeferredDecal;
    DecalMaterial->BlendMode = BLEND_Translucent;
    
    // Configure decal blend mode based on what channels to affect
    DecalMaterial->SetDecalBlendMode(DBM_Translucent);
    
    // Set sort order for decal layering (higher = rendered on top)
    // Note: Sort order is typically set on the decal component, not material
    
    // Create fade parameters for distance-based fading
    UMaterialExpressionDepthFade* DepthFade = NewObject<UMaterialExpressionDepthFade>(DecalMaterial);
    DepthFade->FadeDistanceDefault = Settings.FadeScreenSize * 10000.0f;  // Convert to units
    DecalMaterial->GetExpressionCollection().AddExpression(DepthFade);
    
    // Create time-based fade for decal lifetime
    if (Settings.FadeDuration > 0.0f)
    {
        // Get time since spawn (requires custom parameter set by blueprint/code)
        UMaterialExpressionScalarParameter* SpawnTimeParam = NewObject<UMaterialExpressionScalarParameter>(DecalMaterial);
        SpawnTimeParam->ParameterName = TEXT("SpawnTime");
        SpawnTimeParam->DefaultValue = 0.0f;
        DecalMaterial->GetExpressionCollection().AddExpression(SpawnTimeParam);
        
        // Get current game time
        UMaterialExpressionTime* TimeNode = NewObject<UMaterialExpressionTime>(DecalMaterial);
        DecalMaterial->GetExpressionCollection().AddExpression(TimeNode);
        
        // Calculate age = CurrentTime - SpawnTime
        UMaterialExpressionSubtract* AgeCalc = NewObject<UMaterialExpressionSubtract>(DecalMaterial);
        AgeCalc->A.Expression = TimeNode;
        AgeCalc->B.Expression = SpawnTimeParam;
        DecalMaterial->GetExpressionCollection().AddExpression(AgeCalc);
        
        // Calculate fade start time and fade alpha
        UMaterialExpressionConstant* FadeStartDelay = NewObject<UMaterialExpressionConstant>(DecalMaterial);
        FadeStartDelay->R = Settings.FadeStartDelay;
        DecalMaterial->GetExpressionCollection().AddExpression(FadeStartDelay);
        
        UMaterialExpressionConstant* FadeDuration = NewObject<UMaterialExpressionConstant>(DecalMaterial);
        FadeDuration->R = Settings.FadeDuration;
        DecalMaterial->GetExpressionCollection().AddExpression(FadeDuration);
        
        // FadeAlpha = 1 - saturate((Age - FadeStartDelay) / FadeDuration)
        UMaterialExpressionSubtract* FadeAge = NewObject<UMaterialExpressionSubtract>(DecalMaterial);
        FadeAge->A.Expression = AgeCalc;
        FadeAge->B.Expression = FadeStartDelay;
        DecalMaterial->GetExpressionCollection().AddExpression(FadeAge);
        
        UMaterialExpressionDivide* FadeProgress = NewObject<UMaterialExpressionDivide>(DecalMaterial);
        FadeProgress->A.Expression = FadeAge;
        FadeProgress->B.Expression = FadeDuration;
        DecalMaterial->GetExpressionCollection().AddExpression(FadeProgress);
        
        UMaterialExpressionSaturate* ClampedFade = NewObject<UMaterialExpressionSaturate>(DecalMaterial);
        ClampedFade->Input.Expression = FadeProgress;
        DecalMaterial->GetExpressionCollection().AddExpression(ClampedFade);
        
        UMaterialExpressionOneMinus* InvertedFade = NewObject<UMaterialExpressionOneMinus>(DecalMaterial);
        InvertedFade->Input.Expression = ClampedFade;
        DecalMaterial->GetExpressionCollection().AddExpression(InvertedFade);
        
        // Multiply with depth fade for final opacity
        UMaterialExpressionMultiply* FinalOpacity = NewObject<UMaterialExpressionMultiply>(DecalMaterial);
        FinalOpacity->A.Expression = InvertedFade;
        FinalOpacity->B.Expression = DepthFade;
        DecalMaterial->GetExpressionCollection().AddExpression(FinalOpacity);
        
        // Connect to opacity output
        DecalMaterial->GetEditorOnlyData()->Opacity.Expression = FinalOpacity;
    }
    else
    {
        // No time-based fade, just use depth fade
        DecalMaterial->GetEditorOnlyData()->Opacity.Expression = DepthFade;
    }
    
    // Recompile material
    DecalMaterial->PreEditChange(nullptr);
    DecalMaterial->PostEditChange();
#endif
}

void UForge2MaterialManager::ConfigureSubsurface(UMaterial* Material,
                                                   const FForge2SubsurfaceSettings& Settings)
{
    if (!Material) return;

#if WITH_EDITOR
    // Set shading model to subsurface or subsurface profile
    if (Settings.SubsurfaceProfile)
    {
        Material->SetShadingModel(MSM_SubsurfaceProfile);
        Material->SubsurfaceProfile = Settings.SubsurfaceProfile;
    }
    else
    {
        Material->SetShadingModel(MSM_Subsurface);
    }
    
    // Create subsurface color constant
    UMaterialExpressionConstant4Vector* SubsurfaceColorNode = NewObject<UMaterialExpressionConstant4Vector>(Material);
    SubsurfaceColorNode->Constant = Settings.SubsurfaceColor;
    Material->GetExpressionCollection().AddExpression(SubsurfaceColorNode);
    
    // Connect to subsurface color output
    Material->GetEditorOnlyData()->SubsurfaceColor.Expression = SubsurfaceColorNode;
    
    // Create opacity constant for subsurface amount
    UMaterialExpressionConstant* OpacityNode = NewObject<UMaterialExpressionConstant>(Material);
    OpacityNode->R = Settings.Opacity;
    Material->GetExpressionCollection().AddExpression(OpacityNode);
    
    // Connect to opacity (controls subsurface scattering amount for non-profile SSS)
    Material->GetEditorOnlyData()->Opacity.Expression = OpacityNode;
    
    // If using subsurface profile, also set up the profile parameters
    if (Settings.SubsurfaceProfile)
    {
        // The subsurface profile asset contains:
        // - Scatter radius
        // - Subsurface color (can override material's)
        // - Mean free path
        // - World unit scale
        // These are configured in the profile asset itself
    }
    else
    {
        // For basic subsurface (non-profile), we can control scatter via subsurface color alpha
        // and the general look via the subsurface color's hue
        
        // Create a custom scatter radius parameter
        UMaterialExpressionScalarParameter* ScatterRadiusParam = NewObject<UMaterialExpressionScalarParameter>(Material);
        ScatterRadiusParam->ParameterName = TEXT("SubsurfaceRadius");
        ScatterRadiusParam->DefaultValue = Settings.SubsurfaceRadius;
        Material->GetExpressionCollection().AddExpression(ScatterRadiusParam);
        
        // Multiply subsurface color by radius for controlling scatter distance
        UMaterialExpressionMultiply* ScaledSubsurface = NewObject<UMaterialExpressionMultiply>(Material);
        ScaledSubsurface->A.Expression = SubsurfaceColorNode;
        ScaledSubsurface->B.Expression = ScatterRadiusParam;
        Material->GetExpressionCollection().AddExpression(ScaledSubsurface);
        
        // Override connection with scaled version
        Material->GetEditorOnlyData()->SubsurfaceColor.Expression = ScaledSubsurface;
    }
    
    // Recompile material
    Material->PreEditChange(nullptr);
    Material->PostEditChange();
#endif
}

UTexture2D* UForge2MaterialManager::CreateProceduralTexture(const FString& TextureName,
                                                              const FForge2ProceduralTextureSettings& Settings,
                                                              int32 Width, int32 Height)
{
    UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);

    if (Texture)
    {
        // Generate procedural texture data
        FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
        void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

        uint8* Pixels = static_cast<uint8*>(Data);

        for (int32 Y = 0; Y < Height; Y++)
        {
            for (int32 X = 0; X < Width; X++)
            {
                int32 Index = (Y * Width + X) * 4;

                float U = static_cast<float>(X) / Width;
                float V = static_cast<float>(Y) / Height;

                FLinearColor Color = FLinearColor::White;

                if (Settings.PatternType == TEXT("Checker"))
                {
                    bool bChecker = (FMath::FloorToInt(U * Settings.Scale) + FMath::FloorToInt(V * Settings.Scale)) % 2 == 0;
                    Color = bChecker ? Settings.Color1 : Settings.Color2;
                }
                else if (Settings.PatternType == TEXT("Gradient"))
                {
                    Color = FLinearColor::LerpUsingHSV(Settings.Color1, Settings.Color2, U);
                }
                else  // Default to noise-like pattern
                {
                    float NoiseValue = FMath::PerlinNoise2D(FVector2D(U * Settings.Scale, V * Settings.Scale));
                    NoiseValue = (NoiseValue + 1.0f) * 0.5f;  // Normalize to 0-1
                    Color = FLinearColor::LerpUsingHSV(Settings.Color1, Settings.Color2, NoiseValue);
                }

                Pixels[Index + 0] = static_cast<uint8>(Color.B * 255);
                Pixels[Index + 1] = static_cast<uint8>(Color.G * 255);
                Pixels[Index + 2] = static_cast<uint8>(Color.R * 255);
                Pixels[Index + 3] = 255;
            }
        }

        Mip.BulkData.Unlock();
        Texture->UpdateResource();
    }

    return Texture;
}

UMaterialFunction* UForge2MaterialManager::CreateNoiseFunction(const FString& FunctionName,
                                                                 int32 Octaves, float Persistence)
{
#if WITH_EDITOR
    // Create material function package and asset
    FString PackagePath = TEXT("/Game/Materials/Functions/") + FunctionName;
    UPackage* Package = CreatePackage(*PackagePath);
    
    UMaterialFunction* NoiseFunction = NewObject<UMaterialFunction>(Package, *FunctionName, RF_Public | RF_Standalone);
    if (!NoiseFunction) return nullptr;
    
    NoiseFunction->Description = FString::Printf(TEXT("Fractal Noise with %d octaves, %.2f persistence"), Octaves, Persistence);
    
    // Create function input for UV/Position
    UMaterialExpressionFunctionInput* InputPosition = NewObject<UMaterialExpressionFunctionInput>(NoiseFunction);
    InputPosition->InputName = TEXT("Position");
    InputPosition->InputType = FunctionInput_Vector3;
    InputPosition->Preview.Expression = nullptr;
    NoiseFunction->FunctionExpressions.Add(InputPosition);
    
    // Create function input for base scale
    UMaterialExpressionFunctionInput* InputScale = NewObject<UMaterialExpressionFunctionInput>(NoiseFunction);
    InputScale->InputName = TEXT("Scale");
    InputScale->InputType = FunctionInput_Scalar;
    InputScale->PreviewValue = FVector4(1.0f, 0, 0, 0);
    NoiseFunction->FunctionExpressions.Add(InputScale);
    
    // Scale the input position
    UMaterialExpressionMultiply* ScaledPosition = NewObject<UMaterialExpressionMultiply>(NoiseFunction);
    ScaledPosition->A.Expression = InputPosition;
    ScaledPosition->B.Expression = InputScale;
    NoiseFunction->FunctionExpressions.Add(ScaledPosition);
    
    // Build fractal noise by layering octaves
    UMaterialExpression* AccumulatedNoise = nullptr;
    UMaterialExpression* PreviousNoise = nullptr;
    float CurrentAmplitude = 1.0f;
    float TotalAmplitude = 0.0f;
    
    for (int32 i = 0; i < Octaves; ++i)
    {
        // Calculate frequency multiplier for this octave (2^i)
        float FrequencyMult = FMath::Pow(2.0f, static_cast<float>(i));
        
        // Create frequency multiplier constant
        UMaterialExpressionConstant* FreqMultNode = NewObject<UMaterialExpressionConstant>(NoiseFunction);
        FreqMultNode->R = FrequencyMult;
        NoiseFunction->FunctionExpressions.Add(FreqMultNode);
        
        // Scale position by frequency
        UMaterialExpressionMultiply* OctavePosition = NewObject<UMaterialExpressionMultiply>(NoiseFunction);
        OctavePosition->A.Expression = ScaledPosition;
        OctavePosition->B.Expression = FreqMultNode;
        NoiseFunction->FunctionExpressions.Add(OctavePosition);
        
        // Create noise node for this octave
        UMaterialExpressionNoise* NoiseNode = NewObject<UMaterialExpressionNoise>(NoiseFunction);
        NoiseNode->NoiseFunction = NOISEFUNCTION_GradientTex;
        NoiseNode->bTurbulence = false;
        NoiseNode->Levels = 1;
        NoiseNode->OutputMin = -1.0f;
        NoiseNode->OutputMax = 1.0f;
        NoiseNode->Position.Expression = OctavePosition;
        NoiseFunction->FunctionExpressions.Add(NoiseNode);
        
        // Create amplitude constant
        UMaterialExpressionConstant* AmpNode = NewObject<UMaterialExpressionConstant>(NoiseFunction);
        AmpNode->R = CurrentAmplitude;
        NoiseFunction->FunctionExpressions.Add(AmpNode);
        
        // Scale noise by amplitude
        UMaterialExpressionMultiply* ScaledNoise = NewObject<UMaterialExpressionMultiply>(NoiseFunction);
        ScaledNoise->A.Expression = NoiseNode;
        ScaledNoise->B.Expression = AmpNode;
        NoiseFunction->FunctionExpressions.Add(ScaledNoise);
        
        // Accumulate noise
        if (AccumulatedNoise == nullptr)
        {
            AccumulatedNoise = ScaledNoise;
        }
        else
        {
            UMaterialExpressionAdd* AddNode = NewObject<UMaterialExpressionAdd>(NoiseFunction);
            AddNode->A.Expression = AccumulatedNoise;
            AddNode->B.Expression = ScaledNoise;
            NoiseFunction->FunctionExpressions.Add(AddNode);
            AccumulatedNoise = AddNode;
        }
        
        TotalAmplitude += CurrentAmplitude;
        CurrentAmplitude *= Persistence;
    }
    
    // Normalize by total amplitude
    UMaterialExpressionConstant* NormFactor = NewObject<UMaterialExpressionConstant>(NoiseFunction);
    NormFactor->R = 1.0f / TotalAmplitude;
    NoiseFunction->FunctionExpressions.Add(NormFactor);
    
    UMaterialExpressionMultiply* NormalizedNoise = NewObject<UMaterialExpressionMultiply>(NoiseFunction);
    NormalizedNoise->A.Expression = AccumulatedNoise;
    NormalizedNoise->B.Expression = NormFactor;
    NoiseFunction->FunctionExpressions.Add(NormalizedNoise);
    
    // Remap from -1,1 to 0,1
    UMaterialExpressionConstant* HalfNode = NewObject<UMaterialExpressionConstant>(NoiseFunction);
    HalfNode->R = 0.5f;
    NoiseFunction->FunctionExpressions.Add(HalfNode);
    
    UMaterialExpressionMultiply* ScaledResult = NewObject<UMaterialExpressionMultiply>(NoiseFunction);
    ScaledResult->A.Expression = NormalizedNoise;
    ScaledResult->B.Expression = HalfNode;
    NoiseFunction->FunctionExpressions.Add(ScaledResult);
    
    UMaterialExpressionAdd* RemappedResult = NewObject<UMaterialExpressionAdd>(NoiseFunction);
    RemappedResult->A.Expression = ScaledResult;
    RemappedResult->B.Expression = HalfNode;
    NoiseFunction->FunctionExpressions.Add(RemappedResult);
    
    // Create output
    UMaterialExpressionFunctionOutput* Output = NewObject<UMaterialExpressionFunctionOutput>(NoiseFunction);
    Output->OutputName = TEXT("Noise");
    Output->A.Expression = RemappedResult;
    NoiseFunction->FunctionExpressions.Add(Output);
    
    // Mark for compilation
    NoiseFunction->PreEditChange(nullptr);
    NoiseFunction->PostEditChange();
    
    // Save the package
    Package->MarkPackageDirty();
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    UPackage::SavePackage(Package, NoiseFunction, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);
    
    return NoiseFunction;
#else
    return nullptr;
#endif
}

UMaterial* UForge2MaterialManager::CreateMetalMaterial(const FString& MaterialName,
                                                         FLinearColor BaseColor, float Roughness)
{
    FForge2MaterialSettings Settings = UForge2MaterialUtility::CreateDefaultSettings();
    UMaterial* Material = CreateMaterial(MaterialName, Settings);

#if WITH_EDITOR
    if (Material)
    {
        // Create BaseColor constant node
        UMaterialExpressionConstant4Vector* BaseColorNode = NewObject<UMaterialExpressionConstant4Vector>(Material);
        BaseColorNode->Constant = BaseColor;
        Material->GetExpressionCollection().AddExpression(BaseColorNode);
        Material->GetEditorOnlyData()->BaseColor.Expression = BaseColorNode;
        
        // Create Metallic constant node (1.0 for metal)
        UMaterialExpressionConstant* MetallicNode = NewObject<UMaterialExpressionConstant>(Material);
        MetallicNode->R = 1.0f;
        Material->GetExpressionCollection().AddExpression(MetallicNode);
        Material->GetEditorOnlyData()->Metallic.Expression = MetallicNode;
        
        // Create Roughness constant node
        UMaterialExpressionConstant* RoughnessNode = NewObject<UMaterialExpressionConstant>(Material);
        RoughnessNode->R = Roughness;
        Material->GetExpressionCollection().AddExpression(RoughnessNode);
        Material->GetEditorOnlyData()->Roughness.Expression = RoughnessNode;
        
        // Compile changes
        Material->PreEditChange(nullptr);
        Material->PostEditChange();
    }
#endif

    return Material;
}

UMaterial* UForge2MaterialManager::CreateGlassMaterial(const FString& MaterialName,
                                                         float Opacity, float Roughness, FLinearColor Tint)
{
    FForge2MaterialSettings Settings = UForge2MaterialUtility::CreateTranslucentSettings(Opacity);
    Settings.bScreenSpaceReflections = true;

    UMaterial* Material = CreateMaterial(MaterialName, Settings);

    return Material;
}

UMaterial* UForge2MaterialManager::CreateEmissiveMaterial(const FString& MaterialName,
                                                            FLinearColor EmissiveColor, float Intensity)
{
    FForge2MaterialSettings Settings = UForge2MaterialUtility::CreateDefaultSettings();
    UMaterial* Material = CreateMaterial(MaterialName, Settings);

#if WITH_EDITOR
    if (Material)
    {
        // Create emissive color constant
        UMaterialExpressionConstant4Vector* EmissiveColorNode = NewObject<UMaterialExpressionConstant4Vector>(Material);
        EmissiveColorNode->Constant = EmissiveColor;
        Material->GetExpressionCollection().AddExpression(EmissiveColorNode);
        
        // Create intensity scalar
        UMaterialExpressionConstant* IntensityNode = NewObject<UMaterialExpressionConstant>(Material);
        IntensityNode->R = Intensity;
        Material->GetExpressionCollection().AddExpression(IntensityNode);
        
        // Create multiply node to combine color * intensity
        UMaterialExpressionMultiply* MultiplyNode = NewObject<UMaterialExpressionMultiply>(Material);
        MultiplyNode->A.Expression = EmissiveColorNode;
        MultiplyNode->B.Expression = IntensityNode;
        Material->GetExpressionCollection().AddExpression(MultiplyNode);
        
        // Connect to emissive output
        Material->GetEditorOnlyData()->EmissiveColor.Expression = MultiplyNode;
        
        // Compile changes
        Material->PreEditChange(nullptr);
        Material->PostEditChange();
    }
#endif

    return Material;
}

UMaterial* UForge2MaterialManager::CreateClothMaterial(const FString& MaterialName,
                                                         const FForge2PBRTextureSet& Textures)
{
    FForge2MaterialSettings Settings = UForge2MaterialUtility::CreateDefaultSettings();
    Settings.ShadingModel = EForge2ShadingModel::Cloth;

    UMaterial* Material = CreateMaterial(MaterialName, Settings);

    if (Material)
    {
        ConnectPBRTextures(Material, Textures);
    }

    return Material;
}

UMaterial* UForge2MaterialManager::CreateSkinMaterial(const FString& MaterialName,
                                                        const FForge2SubsurfaceSettings& SSS)
{
    FForge2MaterialSettings Settings = UForge2MaterialUtility::CreateDefaultSettings();
    Settings.ShadingModel = EForge2ShadingModel::SubsurfaceProfile;

    UMaterial* Material = CreateMaterial(MaterialName, Settings);

    if (Material)
    {
        ConfigureSubsurface(Material, SSS);
    }

    return Material;
}

void UForge2MaterialManager::SetupMaterialNode(UMaterial* Material, const FForge2MaterialSettings& Settings)
{
    if (!Material) return;

#if WITH_EDITOR
    // Map domain
    switch (Settings.Domain)
    {
        case EForge2MaterialDomain::Surface: Material->MaterialDomain = EMaterialDomain::MD_Surface; break;
        case EForge2MaterialDomain::DeferredDecal: Material->MaterialDomain = EMaterialDomain::MD_DeferredDecal; break;
        case EForge2MaterialDomain::LightFunction: Material->MaterialDomain = EMaterialDomain::MD_LightFunction; break;
        case EForge2MaterialDomain::PostProcess: Material->MaterialDomain = EMaterialDomain::MD_PostProcess; break;
        case EForge2MaterialDomain::UI: Material->MaterialDomain = EMaterialDomain::MD_UI; break;
        default: Material->MaterialDomain = EMaterialDomain::MD_Surface; break;
    }
    
    // Map blend mode
    switch (Settings.BlendMode)
    {
        case EForge2BlendMode::Opaque: Material->BlendMode = BLEND_Opaque; break;
        case EForge2BlendMode::Masked: Material->BlendMode = BLEND_Masked; break;
        case EForge2BlendMode::Translucent: Material->BlendMode = BLEND_Translucent; break;
        case EForge2BlendMode::Additive: Material->BlendMode = BLEND_Additive; break;
        case EForge2BlendMode::Modulate: Material->BlendMode = BLEND_Modulate; break;
        default: Material->BlendMode = BLEND_Opaque; break;
    }
    
    // Map shading model
    switch (Settings.ShadingModel)
    {
        case EForge2ShadingModel::Unlit: Material->SetShadingModel(MSM_Unlit); break;
        case EForge2ShadingModel::DefaultLit: Material->SetShadingModel(MSM_DefaultLit); break;
        case EForge2ShadingModel::Subsurface: Material->SetShadingModel(MSM_Subsurface); break;
        case EForge2ShadingModel::ClearCoat: Material->SetShadingModel(MSM_ClearCoat); break;
        case EForge2ShadingModel::Cloth: Material->SetShadingModel(MSM_Cloth); break;
        case EForge2ShadingModel::Eye: Material->SetShadingModel(MSM_Eye); break;
        case EForge2ShadingModel::Hair: Material->SetShadingModel(MSM_Hair); break;
        default: Material->SetShadingModel(MSM_DefaultLit); break;
    }
    
    // Additional settings
    Material->TwoSided = Settings.bTwoSided;
    Material->bContactShadows = Settings.bContactShadows;
    Material->bEnableResponsiveAA = Settings.bResponsiveAA;
    Material->bScreenSpaceReflections = Settings.bScreenSpaceReflections;
#endif
}

void UForge2MaterialManager::ConnectPBRTextures(UMaterial* Material, const FForge2PBRTextureSet& Textures)
{
    if (!Material) return;

#if WITH_EDITOR
    // Helper lambda to create texture sampler and connect
    auto CreateTextureSampler = [&Material](UTexture* Texture, int32 CoordinateIndex) -> UMaterialExpressionTextureSample*
    {
        if (!Texture) return nullptr;
        
        UMaterialExpressionTextureSample* Sampler = NewObject<UMaterialExpressionTextureSample>(Material);
        Sampler->Texture = Texture;
        Sampler->SamplerType = SAMPLERTYPE_Color;
        Material->GetExpressionCollection().AddExpression(Sampler);
        return Sampler;
    };
    
    // Base Color
    if (UMaterialExpressionTextureSample* BaseColorSampler = CreateTextureSampler(Textures.BaseColor, 0))
    {
        Material->GetEditorOnlyData()->BaseColor.Expression = BaseColorSampler;
    }
    
    // Normal Map
    if (UMaterialExpressionTextureSample* NormalSampler = CreateTextureSampler(Textures.Normal, 0))
    {
        NormalSampler->SamplerType = SAMPLERTYPE_Normal;
        Material->GetEditorOnlyData()->Normal.Expression = NormalSampler;
    }
    
    // Roughness
    if (UMaterialExpressionTextureSample* RoughnessSampler = CreateTextureSampler(Textures.Roughness, 0))
    {
        Material->GetEditorOnlyData()->Roughness.Expression = RoughnessSampler;
        Material->GetEditorOnlyData()->Roughness.OutputIndex = 1; // Use R channel
    }
    
    // Metallic
    if (UMaterialExpressionTextureSample* MetallicSampler = CreateTextureSampler(Textures.Metallic, 0))
    {
        Material->GetEditorOnlyData()->Metallic.Expression = MetallicSampler;
        Material->GetEditorOnlyData()->Metallic.OutputIndex = 1; // Use R channel
    }
    
    // Ambient Occlusion
    if (UMaterialExpressionTextureSample* AOSampler = CreateTextureSampler(Textures.AmbientOcclusion, 0))
    {
        Material->GetEditorOnlyData()->AmbientOcclusion.Expression = AOSampler;
        Material->GetEditorOnlyData()->AmbientOcclusion.OutputIndex = 1; // Use R channel
    }
    
    // Emissive
    if (UMaterialExpressionTextureSample* EmissiveSampler = CreateTextureSampler(Textures.Emissive, 0))
    {
        Material->GetEditorOnlyData()->EmissiveColor.Expression = EmissiveSampler;
    }
    
    // Compile the material
    Material->PreEditChange(nullptr);
    Material->PostEditChange();
#endif
}

// UForge2MaterialUtility Implementation
FForge2MaterialSettings UForge2MaterialUtility::CreateDefaultSettings()
{
    FForge2MaterialSettings Settings;
    Settings.Domain = EForge2MaterialDomain::Surface;
    Settings.BlendMode = EForge2BlendMode::Opaque;
    Settings.ShadingModel = EForge2ShadingModel::DefaultLit;
    Settings.bTwoSided = false;
    Settings.bContactShadows = true;
    return Settings;
}

FForge2MaterialSettings UForge2MaterialUtility::CreateTranslucentSettings(float Opacity)
{
    FForge2MaterialSettings Settings = CreateDefaultSettings();
    Settings.BlendMode = EForge2BlendMode::Translucent;
    return Settings;
}

FForge2MaterialSettings UForge2MaterialUtility::CreateMaskedSettings(float ClipValue)
{
    FForge2MaterialSettings Settings = CreateDefaultSettings();
    Settings.BlendMode = EForge2BlendMode::Masked;
    Settings.OpacityMaskClipValue = ClipValue;
    return Settings;
}

FForge2SubsurfaceSettings UForge2MaterialUtility::CreateSkinSSS()
{
    FForge2SubsurfaceSettings Settings;
    Settings.SubsurfaceColor = FLinearColor(1.0f, 0.4f, 0.25f);
    Settings.Opacity = 1.0f;
    Settings.SubsurfaceRadius = 1.2f;
    return Settings;
}

FForge2SubsurfaceSettings UForge2MaterialUtility::CreateFoliageSSS()
{
    FForge2SubsurfaceSettings Settings;
    Settings.SubsurfaceColor = FLinearColor(0.3f, 0.6f, 0.1f);
    Settings.Opacity = 0.8f;
    Settings.SubsurfaceRadius = 0.5f;
    return Settings;
}

FForge2SubsurfaceSettings UForge2MaterialUtility::CreateWaxSSS()
{
    FForge2SubsurfaceSettings Settings;
    Settings.SubsurfaceColor = FLinearColor(1.0f, 0.9f, 0.7f);
    Settings.Opacity = 0.9f;
    Settings.SubsurfaceRadius = 2.0f;
    return Settings;
}

FForge2DecalSettings UForge2MaterialUtility::CreateDefaultDecalSettings()
{
    FForge2DecalSettings Settings;
    Settings.FadeScreenSize = 0.01f;
    Settings.FadeStartDelay = 0.0f;
    Settings.FadeDuration = 1.0f;
    Settings.bProjectOnSkeletal = false;
    Settings.SortOrder = 0;
    return Settings;
}

FForge2DecalSettings UForge2MaterialUtility::CreateBloodDecalSettings()
{
    FForge2DecalSettings Settings = CreateDefaultDecalSettings();
    Settings.FadeStartDelay = 30.0f;
    Settings.FadeDuration = 10.0f;
    Settings.bProjectOnSkeletal = true;
    return Settings;
}

FForge2DecalSettings UForge2MaterialUtility::CreateBulletHoleDecalSettings()
{
    FForge2DecalSettings Settings = CreateDefaultDecalSettings();
    Settings.FadeStartDelay = 60.0f;
    Settings.FadeDuration = 5.0f;
    Settings.bProjectOnSkeletal = false;
    Settings.SortOrder = 1;
    return Settings;
}
