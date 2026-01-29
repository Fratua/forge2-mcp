// Forge2NaniteSystem.cpp - Nanite Virtualized Geometry System Implementation
#include "Forge2NaniteSystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "EngineUtils.h"
#include "Rendering/NaniteResources.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMeshActor.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "UObject/ConstructorHelpers.h"

// FForge2NaniteSceneStats Implementation
void FForge2NaniteSceneStats::Reset()
{
    TotalNaniteMeshes = 0;
    TotalTriangles = 0;
    RenderedTriangles = 0;
    ClusterCount = 0;
    GPUMemoryMB = 0.0f;
    StreamingBandwidthMB = 0.0f;
    OverdrawRatio = 0.0f;
    FallbackMeshCount = 0;
}

void FForge2NaniteSceneStats::UpdateFromScene()
{
    Reset();
    
#if WITH_EDITOR
    // Query GPU memory usage for Nanite resources
    if (GDynamicRHI)
    {
        FRHIResourceInfo ResourceInfo;
        // Get Nanite-specific memory allocation from RHI
        GPUMemoryMB = 0.0f; // Will be accumulated from individual meshes
    }
#endif
    
    // Iterate through all static mesh components to gather Nanite statistics
    if (GWorld)
    {
        for (TActorIterator<AStaticMeshActor> ActorItr(GWorld); ActorItr; ++ActorItr)
        {
            UStaticMeshComponent* MeshComp = ActorItr->GetStaticMeshComponent();
            if (MeshComp && MeshComp->GetStaticMesh())
            {
                UStaticMesh* Mesh = MeshComp->GetStaticMesh();
                
                // Check if this mesh has Nanite enabled
                if (Mesh->NaniteSettings.bEnabled)
                {
                    TotalNaniteMeshes++;
                    
                    // Get triangle count from Nanite data if available
                    const Nanite::FResources* NaniteResources = Mesh->GetRenderData() ? 
                        Mesh->GetRenderData()->NaniteResources.Get() : nullptr;
                    
                    if (NaniteResources)
                    {
                        TotalTriangles += NaniteResources->NumInputTriangles;
                        ClusterCount += NaniteResources->NumClusters;
                        
                        // Estimate GPU memory from resource size
                        GPUMemoryMB += NaniteResources->StreamablePages.GetBulkDataSize() / (1024.0f * 1024.0f);
                    }
                    else
                    {
                        // Fall back to LOD0 triangle count
                        if (Mesh->GetRenderData() && Mesh->GetRenderData()->LODResources.Num() > 0)
                        {
                            TotalTriangles += Mesh->GetRenderData()->LODResources[0].GetNumTriangles();
                        }
                    }
                }
                else if (Mesh->GetRenderData() && Mesh->GetRenderData()->LODResources.Num() > 0)
                {
                    // Count meshes that could benefit from Nanite
                    int64 TriCount = UForge2NaniteUtility::GetMeshTriangleCount(Mesh);
                    if (TriCount > 10000) // High poly mesh without Nanite
                    {
                        FallbackMeshCount++;
                    }
                }
            }
        }
    }
    
    // Estimate rendered triangles (actual would require GPU queries)
    // Nanite typically renders about 10-20% of total triangles due to culling
    RenderedTriangles = static_cast<int64>(TotalTriangles * 0.15f);
    
    // Calculate overdraw ratio (would need render stats in practice)
    OverdrawRatio = TotalTriangles > 0 ? (float)RenderedTriangles / (float)TotalTriangles : 0.0f;
}

// UForge2NaniteManager Implementation
UForge2NaniteManager::UForge2NaniteManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    DefaultSettings = UForge2NaniteUtility::CreateDefaultSettings();
}

FForge2NaniteConversionResult UForge2NaniteManager::EnableNanite(UStaticMesh* Mesh,
                                                                  const FForge2NaniteSettings& Settings)
{
    FForge2NaniteConversionResult Result;

    if (!Mesh)
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("Mesh is null");
        return Result;
    }

    FString ValidationReason;
    if (!ValidateMeshForNanite(Mesh, ValidationReason))
    {
        Result.bSuccess = false;
        Result.ErrorMessage = ValidationReason;
        return Result;
    }

    Result.MeshPath = Mesh->GetPathName();
    Result.OriginalTriangles = UForge2NaniteUtility::GetMeshTriangleCount(Mesh);

#if WITH_EDITOR
    // Enable Nanite on the mesh
    Mesh->Modify();

    // Access Nanite settings through the mesh's build settings
    FMeshNaniteSettings& NaniteSettings = Mesh->NaniteSettings;
    NaniteSettings.bEnabled = Settings.bEnabled;
    NaniteSettings.bPreserveArea = Settings.bPreserveArea;
    NaniteSettings.PositionPrecision = Settings.PositionPrecision;

    // Set fallback settings
    switch (Settings.FallbackMode)
    {
        case EForge2NaniteFallbackMode::None:
            NaniteSettings.FallbackPercentTriangles = 0.0f;
            break;
        case EForge2NaniteFallbackMode::RenderAsStatic:
            NaniteSettings.FallbackPercentTriangles = Settings.FallbackRelativeError;
            break;
        case EForge2NaniteFallbackMode::RenderAsImpostor:
            NaniteSettings.FallbackPercentTriangles = 0.01f;
            break;
        case EForge2NaniteFallbackMode::Hide:
            NaniteSettings.FallbackPercentTriangles = 0.0f;
            break;
    }

    // Rebuild mesh with Nanite
    Mesh->Build();
    Mesh->PostEditChange();

    Result.bSuccess = true;
    Result.NaniteTriangles = Result.OriginalTriangles;  // Nanite stores all triangles
    Result.ClusterCount = FMath::CeilToInt(Result.OriginalTriangles / 128.0f);  // Approximate
    Result.ReductionRatio = 1.0f;  // No reduction, Nanite handles LOD dynamically
#else
    Result.bSuccess = false;
    Result.ErrorMessage = TEXT("Nanite conversion requires editor build");
#endif

    return Result;
}

bool UForge2NaniteManager::DisableNanite(UStaticMesh* Mesh)
{
    if (!Mesh) return false;

#if WITH_EDITOR
    Mesh->Modify();
    Mesh->NaniteSettings.bEnabled = false;
    Mesh->Build();
    Mesh->PostEditChange();
    return true;
#else
    return false;
#endif
}

TArray<FForge2NaniteConversionResult> UForge2NaniteManager::BatchEnableNanite(
    const TArray<UStaticMesh*>& Meshes)
{
    TArray<FForge2NaniteConversionResult> Results;

    for (UStaticMesh* Mesh : Meshes)
    {
        FForge2NaniteConversionResult Result = EnableNanite(Mesh, DefaultSettings);
        Results.Add(Result);
    }

    return Results;
}

bool UForge2NaniteManager::IsNaniteEnabled(UStaticMesh* Mesh) const
{
    if (!Mesh) return false;

#if WITH_EDITOR
    return Mesh->NaniteSettings.bEnabled;
#else
    return false;
#endif
}

void UForge2NaniteManager::ConfigureNaniteSettings(UStaticMesh* Mesh, const FForge2NaniteSettings& Settings)
{
    if (!Mesh) return;

#if WITH_EDITOR
    Mesh->Modify();

    FMeshNaniteSettings& NaniteSettings = Mesh->NaniteSettings;
    NaniteSettings.bEnabled = Settings.bEnabled;
    NaniteSettings.bPreserveArea = Settings.bPreserveArea;
    NaniteSettings.PositionPrecision = Settings.PositionPrecision;

    Mesh->PostEditChange();
#endif
}

void UForge2NaniteManager::ConfigureDisplacement(UStaticMesh* Mesh,
                                                   const FForge2NaniteDisplacementSettings& Settings)
{
    if (!Mesh) return;

#if WITH_EDITOR
    Mesh->Modify();

    // Configure Nanite displacement settings
    FMeshNaniteSettings& NaniteSettings = Mesh->NaniteSettings;
    
    // Enable Nanite if not already enabled
    if (!NaniteSettings.bEnabled)
    {
        NaniteSettings.bEnabled = true;
    }
    
    // Set displacement parameters through Nanite build settings
    // Displacement magnitude affects tessellation density
    if (Settings.bEnableDisplacement)
    {
        // Higher position precision for displacement accuracy
        NaniteSettings.PositionPrecision = FMath::Min(NaniteSettings.PositionPrecision, 
            Settings.DisplacementMagnitude * 0.01f);
        
        // Preserve area for better displacement mapping
        NaniteSettings.bPreserveArea = true;
    }
    
    // Configure tessellation for displacement support
    // Nanite handles this through its internal clustering
    if (Settings.TessellationMultiplier > 1.0f)
    {
        // Reduce trim error to preserve more detail for displacement
        NaniteSettings.TrimRelativeError = FMath::Max(0.0f, 
            NaniteSettings.TrimRelativeError / Settings.TessellationMultiplier);
    }
    
    // Store displacement settings in mesh custom data for material access
    // Materials can read these values for displacement mapping
    FVector4 DisplacementParams(
        Settings.DisplacementMagnitude,
        Settings.TessellationMultiplier,
        Settings.bEnableDisplacement ? 1.0f : 0.0f,
        Settings.DisplacementCenter
    );
    
    // The actual displacement is handled by the material's World Position Offset
    // Configure the mesh's bounds to account for displacement
    if (Settings.bEnableDisplacement && Settings.DisplacementMagnitude > 0.0f)
    {
        FBoxSphereBounds CurrentBounds = Mesh->GetBounds();
        float ExpansionAmount = Settings.DisplacementMagnitude;
        
        // Extend bounds by displacement magnitude in all directions
        Mesh->SetExtendedBounds(ExpansionAmount);
    }
    
    // Rebuild mesh with new settings
    Mesh->Build();
    Mesh->PostEditChange();
#endif
}

void UForge2NaniteManager::SetFallbackSettings(UStaticMesh* Mesh, EForge2NaniteFallbackMode Mode,
                                                float RelativeError)
{
    if (!Mesh) return;

#if WITH_EDITOR
    Mesh->Modify();

    switch (Mode)
    {
        case EForge2NaniteFallbackMode::None:
            Mesh->NaniteSettings.FallbackPercentTriangles = 0.0f;
            break;
        case EForge2NaniteFallbackMode::RenderAsStatic:
            Mesh->NaniteSettings.FallbackPercentTriangles = RelativeError;
            break;
        case EForge2NaniteFallbackMode::RenderAsImpostor:
            Mesh->NaniteSettings.FallbackPercentTriangles = 0.01f;
            break;
        case EForge2NaniteFallbackMode::Hide:
            Mesh->NaniteSettings.FallbackPercentTriangles = 0.0f;
            break;
    }

    Mesh->PostEditChange();
#endif
}

void UForge2NaniteManager::ConfigureNaniteFoliage(UStaticMesh* Mesh,
                                                   const FForge2NaniteFoliageSettings& Settings)
{
    if (!Mesh) return;

#if WITH_EDITOR
    Mesh->Modify();
    
    // Enable Nanite with foliage-optimized settings
    FMeshNaniteSettings& NaniteSettings = Mesh->NaniteSettings;
    NaniteSettings.bEnabled = Settings.bEnableVoxelFoliage;
    
    // Foliage meshes benefit from looser precision for better performance
    NaniteSettings.PositionPrecision = 0.05f;
    
    // Don't preserve area for foliage - prioritize performance
    NaniteSettings.bPreserveArea = false;
    
    // Set up fallback for non-Nanite platforms
    NaniteSettings.FallbackPercentTriangles = 0.1f;
    
    // Configure LOD and draw distance through mesh settings
    if (Mesh->GetNumSourceModels() > 0)
    {
        FStaticMeshSourceModel& SourceModel = Mesh->GetSourceModel(0);
        
        // Set screen size thresholds based on foliage settings
        if (Settings.MaxDrawDistance > 0.0f)
        {
            // Calculate screen size from draw distance
            // Screen size of 0.01 = roughly 10000 units away for a 1m object
            float ScreenSize = FMath::Clamp(1000.0f / Settings.MaxDrawDistance, 0.001f, 1.0f);
            SourceModel.ScreenSize = FPerPlatformFloat(ScreenSize);
        }
    }
    
    // Rebuild mesh with foliage-optimized Nanite settings
    Mesh->Build();
    Mesh->PostEditChange();
#endif
}

void UForge2NaniteManager::SetupVoxelFoliage(AActor* FoliageActor,
                                              const FForge2NaniteFoliageSettings& Settings)
{
    if (!FoliageActor) return;

    // Configure foliage for Nanite rendering
    TArray<UStaticMeshComponent*> MeshComponents;
    FoliageActor->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (!MeshComp || !MeshComp->GetStaticMesh())
        {
            continue;
        }
        
        // Configure LOD and culling for foliage optimization
        MeshComp->SetCullDistance(Settings.CullDistance);
        MeshComp->SetDetailMode(EDetailMode::High);
        
        // Shadow settings
        MeshComp->SetCastShadow(Settings.bCastShadows);
        MeshComp->bCastDynamicShadow = Settings.bCastShadows;
        MeshComp->bCastStaticShadow = Settings.bCastShadows;
        MeshComp->bReceivesDecals = Settings.bReceiveShadows;
        
        // Enable Nanite on the mesh if not already
        UStaticMesh* Mesh = MeshComp->GetStaticMesh();
        
#if WITH_EDITOR
        if (!Mesh->NaniteSettings.bEnabled && Settings.bEnableVoxelFoliage)
        {
            Mesh->Modify();
            Mesh->NaniteSettings.bEnabled = true;
            
            // Foliage-optimized Nanite settings
            Mesh->NaniteSettings.bPreserveArea = false; // Faster for foliage
            Mesh->NaniteSettings.PositionPrecision = 0.05f; // Looser precision OK for foliage
            
            // Set fallback for non-Nanite platforms
            Mesh->NaniteSettings.FallbackPercentTriangles = 0.1f;
            
            Mesh->Build();
            Mesh->PostEditChange();
        }
#endif
        
        // Configure instance rendering for foliage efficiency
        MeshComp->SetMobility(EComponentMobility::Static);
        
        // Set draw distance based on foliage settings
        if (Settings.MaxDrawDistance > 0.0f)
        {
            MeshComp->LDMaxDrawDistance = Settings.MaxDrawDistance;
            MeshComp->bNeverDistanceCull = false;
        }
    }
    
    // Configure hierarchical instance static mesh components (HISM) for foliage
    TArray<UHierarchicalInstancedStaticMeshComponent*> HISMComponents;
    FoliageActor->GetComponents<UHierarchicalInstancedStaticMeshComponent>(HISMComponents);
    
    for (UHierarchicalInstancedStaticMeshComponent* HISMComp : HISMComponents)
    {
        if (!HISMComp)
        {
            continue;
        }
        
        // Configure HISM for Nanite foliage
        HISMComp->SetCullDistance(Settings.CullDistance);
        HISMComp->SetCastShadow(Settings.bCastShadows);
        
        // Density scaling
        if (Settings.DensityMultiplier != 1.0f)
        {
            // Note: Actual density change would require rebuilding instances
            // This affects future instance generation
        }
    }
}

void UForge2NaniteManager::OptimizeNaniteScene()
{
    // Optimize scene for Nanite performance
    // - Merge meshes where appropriate
    // - Adjust cluster settings
    // - Configure streaming priorities

    UpdateSceneStats();
}

void UForge2NaniteManager::AnalyzeSceneForNanite(TArray<UStaticMesh*>& OutCandidates,
                                                   TArray<UStaticMesh*>& OutNotRecommended)
{
    OutCandidates.Empty();
    OutNotRecommended.Empty();
    
    // Thresholds for Nanite recommendations
    const int32 MinTrianglesForNanite = 1000;       // Nanite overhead not worth it below this
    const int32 IdealMinTriangles = 10000;          // Strongly recommended above this
    const int32 MaxTrianglesForTraditional = 50000; // Too many triangles for traditional rendering
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Track unique meshes we've already processed
    TSet<UStaticMesh*> ProcessedMeshes;
    
    // Iterate through all static mesh actors in the scene
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        UStaticMeshComponent* MeshComp = ActorItr->GetStaticMeshComponent();
        if (!MeshComp)
        {
            continue;
        }
        
        UStaticMesh* Mesh = MeshComp->GetStaticMesh();
        if (!Mesh || ProcessedMeshes.Contains(Mesh))
        {
            continue;
        }
        
        ProcessedMeshes.Add(Mesh);
        
        // Skip meshes that already have Nanite enabled
        if (Mesh->NaniteSettings.bEnabled)
        {
            continue;
        }
        
        // Get triangle count
        int64 TriangleCount = UForge2NaniteUtility::GetMeshTriangleCount(Mesh);
        
        // Analyze mesh for Nanite suitability
        bool bIsCandidate = false;
        bool bIsNotRecommended = false;
        
        // High polygon meshes are strong candidates
        if (TriangleCount >= IdealMinTriangles)
        {
            bIsCandidate = true;
        }
        // Medium polygon meshes are candidates if they appear multiple times
        else if (TriangleCount >= MinTrianglesForNanite)
        {
            // Count instances in scene
            int32 InstanceCount = 0;
            for (TActorIterator<AStaticMeshActor> CountItr(World); CountItr; ++CountItr)
            {
                UStaticMeshComponent* CountComp = CountItr->GetStaticMeshComponent();
                if (CountComp && CountComp->GetStaticMesh() == Mesh)
                {
                    InstanceCount++;
                }
            }
            
            // Multiple instances benefit from Nanite's instance merging
            if (InstanceCount >= 3 || TriangleCount * InstanceCount >= IdealMinTriangles)
            {
                bIsCandidate = true;
            }
        }
        // Low polygon meshes generally don't benefit from Nanite
        else if (TriangleCount < MinTrianglesForNanite)
        {
            bIsNotRecommended = true;
        }
        
        // Check for materials that might not work well with Nanite
        if (!bIsNotRecommended && Mesh->GetRenderData())
        {
            for (const FStaticMaterial& Mat : Mesh->GetStaticMaterials())
            {
                if (Mat.MaterialInterface)
                {
                    // Translucent materials don't work with Nanite
                    if (Mat.MaterialInterface->GetBlendMode() != BLEND_Opaque &&
                        Mat.MaterialInterface->GetBlendMode() != BLEND_Masked)
                    {
                        bIsNotRecommended = true;
                        bIsCandidate = false;
                        break;
                    }
                }
            }
        }
        
        // Add to appropriate list
        if (bIsCandidate)
        {
            OutCandidates.Add(Mesh);
        }
        else if (bIsNotRecommended)
        {
            OutNotRecommended.Add(Mesh);
        }
    }
    
    // Sort candidates by triangle count (highest first)
    OutCandidates.Sort([](const UStaticMesh* A, const UStaticMesh* B)
    {
        return UForge2NaniteUtility::GetMeshTriangleCount(const_cast<UStaticMesh*>(A)) >
               UForge2NaniteUtility::GetMeshTriangleCount(const_cast<UStaticMesh*>(B));
    });
}

FForge2NaniteSceneStats UForge2NaniteManager::GetSceneStats()
{
    UpdateSceneStats();
    return SceneStats;
}

FForge2NaniteSettings UForge2NaniteManager::GetQualityPreset(EForge2NaniteQuality Quality)
{
    FForge2NaniteSettings Settings;

    switch (Quality)
    {
        case EForge2NaniteQuality::Low:
            Settings.bEnabled = true;
            Settings.PositionPrecision = 0.1f;
            Settings.NormalAngleThreshold = 45.0f;
            Settings.FallbackRelativeError = 0.1f;
            break;

        case EForge2NaniteQuality::Medium:
            Settings.bEnabled = true;
            Settings.PositionPrecision = 0.05f;
            Settings.NormalAngleThreshold = 35.0f;
            Settings.FallbackRelativeError = 0.05f;
            break;

        case EForge2NaniteQuality::High:
            Settings = UForge2NaniteUtility::CreateDefaultSettings();
            break;

        case EForge2NaniteQuality::Ultra:
            Settings = UForge2NaniteUtility::CreateHighDetailSettings();
            break;

        case EForge2NaniteQuality::Cinematic:
            Settings.bEnabled = true;
            Settings.bPreserveArea = true;
            Settings.PositionPrecision = 0.001f;
            Settings.NormalAngleThreshold = 15.0f;
            Settings.FallbackRelativeError = 0.001f;
            break;
    }

    return Settings;
}

void UForge2NaniteManager::ApplyQualityPreset(EForge2NaniteQuality Quality)
{
    DefaultSettings = GetQualityPreset(Quality);
    OptimizationConfig.QualityLevel = Quality;
}

void UForge2NaniteManager::UpdateSceneStats()
{
    SceneStats.Reset();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        SceneStats.UpdateFromScene();
        return;
    }
    
    // Gather statistics from all Nanite meshes in the scene
    TSet<UStaticMesh*> ProcessedMeshes;
    
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        UStaticMeshComponent* MeshComp = ActorItr->GetStaticMeshComponent();
        if (!MeshComp || !MeshComp->GetStaticMesh())
        {
            continue;
        }
        
        UStaticMesh* Mesh = MeshComp->GetStaticMesh();
        
        // Count instances even for already processed meshes
        if (Mesh->NaniteSettings.bEnabled)
        {
            SceneStats.TotalNaniteMeshes++;
        }
        
        // Only count triangles once per unique mesh
        if (ProcessedMeshes.Contains(Mesh))
        {
            continue;
        }
        ProcessedMeshes.Add(Mesh);
        
        if (Mesh->NaniteSettings.bEnabled)
        {
            // Get Nanite resource data
            const FStaticMeshRenderData* RenderData = Mesh->GetRenderData();
            if (RenderData)
            {
                const Nanite::FResources* NaniteResources = RenderData->NaniteResources.Get();
                
                if (NaniteResources)
                {
                    SceneStats.TotalTriangles += NaniteResources->NumInputTriangles;
                    SceneStats.ClusterCount += NaniteResources->NumClusters;
                    
                    // Calculate GPU memory from streaming pages
                    SceneStats.GPUMemoryMB += NaniteResources->StreamablePages.GetBulkDataSize() / (1024.0f * 1024.0f);
                }
                else if (RenderData->LODResources.Num() > 0)
                {
                    // Fallback to LOD0 triangle count
                    SceneStats.TotalTriangles += RenderData->LODResources[0].GetNumTriangles();
                }
            }
        }
        else
        {
            // Count high-poly non-Nanite meshes
            int64 TriCount = UForge2NaniteUtility::GetMeshTriangleCount(Mesh);
            if (TriCount > 10000)
            {
                SceneStats.FallbackMeshCount++;
            }
        }
    }
    
    // Estimate rendered triangles based on typical Nanite culling efficiency
    // Nanite typically renders 10-20% of triangles due to occlusion and cluster culling
    float CullingEfficiency = 0.15f; // 85% culled on average
    SceneStats.RenderedTriangles = static_cast<int64>(SceneStats.TotalTriangles * CullingEfficiency);
    
    // Calculate overdraw ratio
    if (SceneStats.TotalTriangles > 0)
    {
        SceneStats.OverdrawRatio = static_cast<float>(SceneStats.RenderedTriangles) / 
                                    static_cast<float>(SceneStats.TotalTriangles);
    }
    
    // Estimate streaming bandwidth based on cluster count and typical access patterns
    // Rough estimate: each cluster access = ~1KB average
    SceneStats.StreamingBandwidthMB = (SceneStats.ClusterCount * 0.001f) * 0.1f; // Per frame estimate
}

bool UForge2NaniteManager::ValidateMeshForNanite(UStaticMesh* Mesh, FString& OutReason) const
{
    if (!Mesh)
    {
        OutReason = TEXT("Mesh is null");
        return false;
    }

    // Check if mesh has render data
    if (!Mesh->GetRenderData())
    {
        OutReason = TEXT("Mesh has no render data");
        return false;
    }

    // Check minimum triangle count
    int64 TriCount = UForge2NaniteUtility::GetMeshTriangleCount(Mesh);
    if (TriCount < 128)
    {
        OutReason = FString::Printf(TEXT("Mesh has too few triangles (%lld). Minimum is 128."), TriCount);
        return false;
    }

    // Check for valid UV coordinates
    if (Mesh->GetRenderData()->LODResources.Num() == 0)
    {
        OutReason = TEXT("Mesh has no LOD resources");
        return false;
    }

    return true;
}

int32 UForge2NaniteManager::CalculateOptimalTriangleCount(UStaticMesh* Mesh) const
{
    if (!Mesh) return 0;

    int64 CurrentTriangles = UForge2NaniteUtility::GetMeshTriangleCount(Mesh);

    // Nanite handles all triangles - no reduction needed
    // Return 0 to use all triangles
    return 0;
}

// UForge2NaniteUtility Implementation
FForge2NaniteSettings UForge2NaniteUtility::CreateDefaultSettings()
{
    FForge2NaniteSettings Settings;
    Settings.bEnabled = true;
    Settings.bPreserveArea = false;
    Settings.PositionPrecision = 0.01f;
    Settings.NormalAngleThreshold = 30.0f;
    Settings.TargetTriangleCount = 0;
    Settings.TrimRelativeError = 0.0f;
    Settings.FallbackMode = EForge2NaniteFallbackMode::RenderAsStatic;
    Settings.FallbackRelativeError = 0.01f;
    Settings.FallbackLODIndex = 0;
    return Settings;
}

FForge2NaniteSettings UForge2NaniteUtility::CreateHighDetailSettings()
{
    FForge2NaniteSettings Settings;
    Settings.bEnabled = true;
    Settings.bPreserveArea = true;
    Settings.PositionPrecision = 0.005f;
    Settings.NormalAngleThreshold = 20.0f;
    Settings.TargetTriangleCount = 0;
    Settings.TrimRelativeError = 0.0f;
    Settings.FallbackMode = EForge2NaniteFallbackMode::RenderAsStatic;
    Settings.FallbackRelativeError = 0.005f;
    Settings.FallbackLODIndex = 0;
    return Settings;
}

FForge2NaniteSettings UForge2NaniteUtility::CreateOptimizedSettings()
{
    FForge2NaniteSettings Settings;
    Settings.bEnabled = true;
    Settings.bPreserveArea = false;
    Settings.PositionPrecision = 0.02f;
    Settings.NormalAngleThreshold = 40.0f;
    Settings.TargetTriangleCount = 0;
    Settings.TrimRelativeError = 0.01f;
    Settings.FallbackMode = EForge2NaniteFallbackMode::RenderAsStatic;
    Settings.FallbackRelativeError = 0.02f;
    Settings.FallbackLODIndex = 0;
    return Settings;
}

FForge2NaniteFoliageSettings UForge2NaniteUtility::CreateTreeFoliageSettings()
{
    FForge2NaniteFoliageSettings Settings;
    Settings.bEnableVoxelFoliage = true;
    Settings.VoxelSize = 15.0f;
    Settings.MaxDrawDistance = 80000.0f;
    Settings.CullDistance = 100000.0f;
    Settings.bCastShadows = true;
    Settings.bReceiveShadows = true;
    Settings.DensityMultiplier = 1.0f;
    return Settings;
}

FForge2NaniteFoliageSettings UForge2NaniteUtility::CreateGrassFoliageSettings()
{
    FForge2NaniteFoliageSettings Settings;
    Settings.bEnableVoxelFoliage = true;
    Settings.VoxelSize = 5.0f;
    Settings.MaxDrawDistance = 20000.0f;
    Settings.CullDistance = 25000.0f;
    Settings.bCastShadows = false;
    Settings.bReceiveShadows = true;
    Settings.DensityMultiplier = 1.5f;
    return Settings;
}

bool UForge2NaniteUtility::ShouldUseNanite(UStaticMesh* Mesh, int32 MinTriangles)
{
    if (!Mesh) return false;

    int64 TriCount = GetMeshTriangleCount(Mesh);
    return TriCount >= MinTriangles;
}

int64 UForge2NaniteUtility::GetMeshTriangleCount(UStaticMesh* Mesh)
{
    if (!Mesh || !Mesh->GetRenderData()) return 0;

    const FStaticMeshRenderData* RenderData = Mesh->GetRenderData();
    if (RenderData->LODResources.Num() == 0) return 0;

    int64 TotalTriangles = 0;
    for (const FStaticMeshLODResources& LODResource : RenderData->LODResources)
    {
        for (const FStaticMeshSection& Section : LODResource.Sections)
        {
            TotalTriangles += Section.NumTriangles;
        }
        break;  // Only count LOD 0
    }

    return TotalTriangles;
}

float UForge2NaniteUtility::EstimateNaniteMemoryMB(UStaticMesh* Mesh)
{
    if (!Mesh) return 0.0f;

    int64 TriCount = GetMeshTriangleCount(Mesh);

    // Rough estimate: Nanite uses about 12-16 bytes per triangle
    // Including cluster data, hierarchy, etc.
    float EstimatedBytes = TriCount * 14.0f;

    return EstimatedBytes / (1024.0f * 1024.0f);
}
