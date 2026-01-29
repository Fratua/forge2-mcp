// Forge2NaniteSystem.h - Nanite Virtualized Geometry System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2NaniteSystem.generated.h"

// Forward declarations
class UStaticMesh;
class UStaticMeshComponent;

// Enums
UENUM(BlueprintType)
enum class EForge2NaniteFallbackMode : uint8
{
    None,
    RenderAsStatic,
    RenderAsImpostor,
    Hide
};

UENUM(BlueprintType)
enum class EForge2NaniteQuality : uint8
{
    Low,
    Medium,
    High,
    Ultra,
    Cinematic
};

UENUM(BlueprintType)
enum class EForge2NaniteDisplacementMode : uint8
{
    None,
    Tessellation,
    VirtualDisplacement,
    ParallaxOcclusion
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NaniteSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bPreserveArea = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PositionPrecision = 0.01f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NormalAngleThreshold = 30.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TargetTriangleCount = 0;  // 0 = auto
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TrimRelativeError = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2NaniteFallbackMode FallbackMode = EForge2NaniteFallbackMode::RenderAsStatic;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FallbackRelativeError = 0.01f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FallbackLODIndex = 0;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NaniteFoliageSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableVoxelFoliage = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VoxelSize = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxDrawDistance = 50000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CullDistance = 60000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCastShadows = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bReceiveShadows = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DensityMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NaniteDisplacementSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2NaniteDisplacementMode Mode = EForge2NaniteDisplacementMode::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DisplacementMagnitude = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DisplacementCenter = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> DisplacementMap;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TessellationLevel = 4;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NaniteSceneStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) int32 TotalNaniteMeshes = 0;
    UPROPERTY(BlueprintReadOnly) int64 TotalTriangles = 0;
    UPROPERTY(BlueprintReadOnly) int64 RenderedTriangles = 0;
    UPROPERTY(BlueprintReadOnly) int32 ClusterCount = 0;
    UPROPERTY(BlueprintReadOnly) float GPUMemoryMB = 0.0f;
    UPROPERTY(BlueprintReadOnly) float StreamingBandwidthMB = 0.0f;
    UPROPERTY(BlueprintReadOnly) float OverdrawRatio = 0.0f;
    UPROPERTY(BlueprintReadOnly) int32 FallbackMeshCount = 0;

    void Reset();
    void UpdateFromScene();
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NaniteConversionResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) bool bSuccess = false;
    UPROPERTY(BlueprintReadOnly) FString MeshPath;
    UPROPERTY(BlueprintReadOnly) int32 OriginalTriangles = 0;
    UPROPERTY(BlueprintReadOnly) int32 NaniteTriangles = 0;
    UPROPERTY(BlueprintReadOnly) int32 ClusterCount = 0;
    UPROPERTY(BlueprintReadOnly) float ReductionRatio = 0.0f;
    UPROPERTY(BlueprintReadOnly) FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NaniteOptimizationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2NaniteQuality QualityLevel = EForge2NaniteQuality::High;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bOptimizeForStreaming = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxClustersPerMesh = 0;  // 0 = unlimited
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ScreenSizeThreshold = 0.001f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAllowCompression = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bGenerateFallback = true;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2NaniteManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2NaniteManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2NaniteSettings DefaultSettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2NaniteOptimizationConfig OptimizationConfig;
    UPROPERTY(BlueprintReadOnly) FForge2NaniteSceneStats SceneStats;

    // Nanite conversion
    UFUNCTION(BlueprintCallable, Category = "Nanite|Conversion")
    FForge2NaniteConversionResult EnableNanite(UStaticMesh* Mesh, const FForge2NaniteSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Nanite|Conversion")
    bool DisableNanite(UStaticMesh* Mesh);

    UFUNCTION(BlueprintCallable, Category = "Nanite|Conversion")
    TArray<FForge2NaniteConversionResult> BatchEnableNanite(const TArray<UStaticMesh*>& Meshes);

    UFUNCTION(BlueprintPure, Category = "Nanite|Query")
    bool IsNaniteEnabled(UStaticMesh* Mesh) const;

    // Settings management
    UFUNCTION(BlueprintCallable, Category = "Nanite|Settings")
    void ConfigureNaniteSettings(UStaticMesh* Mesh, const FForge2NaniteSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Nanite|Settings")
    void ConfigureDisplacement(UStaticMesh* Mesh, const FForge2NaniteDisplacementSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Nanite|Settings")
    void SetFallbackSettings(UStaticMesh* Mesh, EForge2NaniteFallbackMode Mode, float RelativeError);

    // Foliage integration
    UFUNCTION(BlueprintCallable, Category = "Nanite|Foliage")
    void ConfigureNaniteFoliage(UStaticMesh* Mesh, const FForge2NaniteFoliageSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Nanite|Foliage")
    void SetupVoxelFoliage(AActor* FoliageActor, const FForge2NaniteFoliageSettings& Settings);

    // Scene optimization
    UFUNCTION(BlueprintCallable, Category = "Nanite|Optimization")
    void OptimizeNaniteScene();

    UFUNCTION(BlueprintCallable, Category = "Nanite|Optimization")
    void AnalyzeSceneForNanite(TArray<UStaticMesh*>& OutCandidates, TArray<UStaticMesh*>& OutNotRecommended);

    UFUNCTION(BlueprintCallable, Category = "Nanite|Optimization")
    FForge2NaniteSceneStats GetSceneStats();

    // Quality presets
    UFUNCTION(BlueprintCallable, Category = "Nanite|Presets")
    FForge2NaniteSettings GetQualityPreset(EForge2NaniteQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Nanite|Presets")
    void ApplyQualityPreset(EForge2NaniteQuality Quality);

private:
    void UpdateSceneStats();
    bool ValidateMeshForNanite(UStaticMesh* Mesh, FString& OutReason) const;
    int32 CalculateOptimalTriangleCount(UStaticMesh* Mesh) const;
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2NaniteUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Nanite|Utility")
    static FForge2NaniteSettings CreateDefaultSettings();

    UFUNCTION(BlueprintCallable, Category = "Nanite|Utility")
    static FForge2NaniteSettings CreateHighDetailSettings();

    UFUNCTION(BlueprintCallable, Category = "Nanite|Utility")
    static FForge2NaniteSettings CreateOptimizedSettings();

    UFUNCTION(BlueprintCallable, Category = "Nanite|Utility")
    static FForge2NaniteFoliageSettings CreateTreeFoliageSettings();

    UFUNCTION(BlueprintCallable, Category = "Nanite|Utility")
    static FForge2NaniteFoliageSettings CreateGrassFoliageSettings();

    UFUNCTION(BlueprintPure, Category = "Nanite|Utility")
    static bool ShouldUseNanite(UStaticMesh* Mesh, int32 MinTriangles = 1000);

    UFUNCTION(BlueprintPure, Category = "Nanite|Utility")
    static int64 GetMeshTriangleCount(UStaticMesh* Mesh);

    UFUNCTION(BlueprintPure, Category = "Nanite|Utility")
    static float EstimateNaniteMemoryMB(UStaticMesh* Mesh);
};
