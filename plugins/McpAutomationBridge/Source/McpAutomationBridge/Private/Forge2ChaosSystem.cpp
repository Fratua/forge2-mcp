// Forge2ChaosSystem.cpp - Chaos Physics & Destruction System Implementation
#include "Forge2ChaosSystem.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "GeometryCollection/GeometryCollectionAlgo.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemActor.h"
#include "PhysicsField/PhysicsFieldComponent.h"
#include "ChaosCloth/ChaosClothingSimulationFactory.h"
#include "ClothingSimulationInteractor.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/IConsoleManager.h"

#if WITH_EDITOR
#include "GeometryCollection/GeometryCollectionConversion.h"
#include "FractureToolSettings.h"
#endif

// UForge2ChaosManager Implementation
UForge2ChaosManager::UForge2ChaosManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UGeometryCollection* UForge2ChaosManager::CreateDestructible(UStaticMesh* SourceMesh,
                                                               const FForge2FractureSettings& FractureSettings)
{
    if (!SourceMesh) return nullptr;

#if WITH_EDITOR
    // Create new geometry collection asset
    UGeometryCollection* GeometryCollection = NewObject<UGeometryCollection>(GetTransientPackage(), NAME_None, RF_Transactional);
    
    // Initialize the geometry collection
    GeometryCollection->GeometrySource.Reset();
    
    // Convert static mesh to geometry collection geometry
    FGeometryCollectionConversion::AppendStaticMesh(
        SourceMesh, 
        nullptr, // Material overrides
        FTransform::Identity, 
        GeometryCollection, 
        false // ReindexMaterials
    );
    
    // Get the geometry group for fracturing
    TSharedPtr<FGeometryCollection, ESPMode::ThreadSafe> GeomCollection = GeometryCollection->GetGeometryCollection();
    if (!GeomCollection.IsValid()) return nullptr;
    
    // Apply fracturing based on mode
    FRandomStream RandomStream(FractureSettings.RandomSeed);
    
    switch (FractureSettings.Mode)
    {
        case EForge2FractureMode::Voronoi:
        {
            // Generate Voronoi sites
            TArray<FVector> Sites;
            FBox Bounds = SourceMesh->GetBoundingBox();
            FVector Extent = Bounds.GetExtent();
            FVector Center = Bounds.GetCenter();
            
            for (int32 i = 0; i < FractureSettings.NumPieces; i++)
            {
                FVector Site = Center + FVector(
                    RandomStream.FRandRange(-Extent.X, Extent.X),
                    RandomStream.FRandRange(-Extent.Y, Extent.Y),
                    RandomStream.FRandRange(-Extent.Z, Extent.Z)
                );
                Sites.Add(Site);
            }
            
            // Apply Voronoi fracture
            FVoronoiDiagram VoronoiDiagram(Sites, Bounds, FractureSettings.Noise);
            GeometryCollectionAlgo::VoronoiFracture(*GeomCollection, 0, Sites, FractureSettings.Grout, RandomStream.GetCurrentSeed(), FTransform::Identity);
            break;
        }
        
        case EForge2FractureMode::Clustered:
        {
            TArray<FVector> Sites;
            for (int32 i = 0; i < FractureSettings.NumPieces; i++)
            {
                FVector Offset = RandomStream.VRand() * FractureSettings.ClusterRadius;
                Sites.Add(FractureSettings.ClusterCenter + Offset);
            }
            GeometryCollectionAlgo::VoronoiFracture(*GeomCollection, 0, Sites, FractureSettings.Grout, RandomStream.GetCurrentSeed(), FTransform::Identity);
            break;
        }
        
        case EForge2FractureMode::Radial:
        {
            TArray<FVector> Sites;
            int32 NumRings = FMath::Max(1, FMath::FloorToInt(FMath::Sqrt((float)FractureSettings.NumPieces)));
            int32 NumSegments = FractureSettings.NumPieces / NumRings;
            
            for (int32 Ring = 1; Ring <= NumRings; Ring++)
            {
                float Radius = (FractureSettings.ClusterRadius / NumRings) * Ring;
                for (int32 Seg = 0; Seg < NumSegments; Seg++)
                {
                    float Angle = (2.0f * PI * Seg) / NumSegments;
                    FVector Site = FractureSettings.ClusterCenter + FVector(
                        FMath::Cos(Angle) * Radius,
                        FMath::Sin(Angle) * Radius,
                        0.0f
                    );
                    Sites.Add(Site);
                }
            }
            GeometryCollectionAlgo::VoronoiFracture(*GeomCollection, 0, Sites, FractureSettings.Grout, RandomStream.GetCurrentSeed(), FTransform::Identity);
            break;
        }
        
        case EForge2FractureMode::Plane:
        {
            // Single plane slice
            FPlane SlicePlane(FractureSettings.ClusterCenter, FVector::UpVector);
            GeometryCollectionAlgo::SplitWithPlane(*GeomCollection, 0, SlicePlane, FractureSettings.Grout);
            break;
        }
        
        case EForge2FractureMode::Uniform:
        default:
        {
            // Uniform grid-based fracture
            FBox Bounds = SourceMesh->GetBoundingBox();
            int32 GridSize = FMath::CeilToInt(FMath::Pow((float)FractureSettings.NumPieces, 1.0f/3.0f));
            TArray<FVector> Sites;
            FVector Step = Bounds.GetSize() / (float)GridSize;
            
            for (int32 X = 0; X < GridSize; X++)
            {
                for (int32 Y = 0; Y < GridSize; Y++)
                {
                    for (int32 Z = 0; Z < GridSize; Z++)
                    {
                        FVector Site = Bounds.Min + FVector(
                            Step.X * (X + 0.5f + RandomStream.FRandRange(-0.3f, 0.3f)),
                            Step.Y * (Y + 0.5f + RandomStream.FRandRange(-0.3f, 0.3f)),
                            Step.Z * (Z + 0.5f + RandomStream.FRandRange(-0.3f, 0.3f))
                        );
                        Sites.Add(Site);
                    }
                }
            }
            GeometryCollectionAlgo::VoronoiFracture(*GeomCollection, 0, Sites, FractureSettings.Grout, RandomStream.GetCurrentSeed(), FTransform::Identity);
            break;
        }
    }
    
    // Add interior material if requested
    if (FractureSettings.bAddInteriorMaterial)
    {
        // Set interior material index on all new faces
        TManagedArray<int32>& MaterialIDs = GeomCollection->MaterialID;
        for (int32 i = 0; i < MaterialIDs.Num(); i++)
        {
            if (MaterialIDs[i] == INDEX_NONE)
            {
                MaterialIDs[i] = FractureSettings.InteriorMaterialIndex;
            }
        }
    }
    
    // Rebuild the geometry collection
    GeometryCollection->InitializeMaterials();
    
    return GeometryCollection;
#else
    return nullptr;
#endif
}

void UForge2ChaosManager::ConfigureDestructionLevels(UGeometryCollectionComponent* Component,
                                                       const FForge2DestructionSettings& Settings)
{
    if (!Component) return;
    
    // Enable damage on the component
    Component->SetNotifyBreaks(Settings.bEnableDestruction);
    Component->SetNotifyRemovals(Settings.bGenerateDebris);
    
    // Configure damage threshold
    Component->SetDamageThreshold(Settings.DamageThreshold);
    
    // Set cluster breaking strain
    if (UGeometryCollection* GC = const_cast<UGeometryCollection*>(Component->GetRestCollection()))
    {
        TSharedPtr<FGeometryCollection, ESPMode::ThreadSafe> GeomCollection = GC->GetGeometryCollection();
        if (GeomCollection.IsValid())
        {
            // Configure per-cluster damage thresholds based on levels
            TManagedArray<float>& DamageThresholds = GeomCollection->GetAttribute<float>("DamageThreshold", FGeometryCollection::TransformGroup);
            int32 NumTransforms = GeomCollection->NumElements(FGeometryCollection::TransformGroup);
            
            for (int32 i = 0; i < NumTransforms && i < Settings.LevelThresholds.Num(); i++)
            {
                DamageThresholds[i] = Settings.LevelThresholds[i];
            }
        }
    }
}

void UForge2ChaosManager::SetDamageThresholds(UGeometryCollectionComponent* Component,
                                                TArray<float> Thresholds)
{
    if (!Component) return;
    
    if (UGeometryCollection* GC = const_cast<UGeometryCollection*>(Component->GetRestCollection()))
    {
        TSharedPtr<FGeometryCollection, ESPMode::ThreadSafe> GeomCollection = GC->GetGeometryCollection();
        if (GeomCollection.IsValid())
        {
            TManagedArray<float>& DamageThresholdArray = GeomCollection->GetAttribute<float>("DamageThreshold", FGeometryCollection::TransformGroup);
            int32 NumTransforms = GeomCollection->NumElements(FGeometryCollection::TransformGroup);
            
            for (int32 i = 0; i < NumTransforms && i < Thresholds.Num(); i++)
            {
                DamageThresholdArray[i] = Thresholds[i];
            }
        }
    }
}

void UForge2ChaosManager::ApplyDamage(UGeometryCollectionComponent* Component, FVector Location,
                                        float Damage, float Radius, EForge2DamageType DamageType)
{
    if (!Component) return;
    
    // Calculate damage multiplier based on type
    float DamageMultiplier = 1.0f;
    switch (DamageType)
    {
        case EForge2DamageType::Explosion: DamageMultiplier = 1.5f; break;
        case EForge2DamageType::Impact: DamageMultiplier = 1.0f; break;
        case EForge2DamageType::Strain: DamageMultiplier = 0.8f; break;
        default: break;
    }
    
    // Apply radial damage using physics field
    Component->ApplyPhysicsField(
        true, // Enable field
        EFieldPhysicsType::Field_ExternalClusterStrain,
        nullptr, // No specific field actor
        [Location, Radius, Damage, DamageMultiplier](const FVector& SamplePosition) -> float
        {
            float Distance = FVector::Dist(SamplePosition, Location);
            if (Distance > Radius) return 0.0f;
            float Falloff = 1.0f - (Distance / Radius);
            return Damage * DamageMultiplier * Falloff;
        }
    );
    
    // Also apply internal strain to break clusters
    Component->ApplyInternalStrain(0, Location, Radius, Damage * DamageMultiplier);
}

void UForge2ChaosManager::ApplyImpulse(UGeometryCollectionComponent* Component, FVector Location,
                                         FVector Impulse, float Radius)
{
    if (!Component) return;
    
    // Apply radial impulse to broken pieces
    Component->ApplyPhysicsField(
        true,
        EFieldPhysicsType::Field_LinearForce,
        nullptr,
        [Location, Impulse, Radius](const FVector& SamplePosition) -> FVector
        {
            float Distance = FVector::Dist(SamplePosition, Location);
            if (Distance > Radius) return FVector::ZeroVector;
            float Falloff = 1.0f - (Distance / Radius);
            FVector Direction = (SamplePosition - Location).GetSafeNormal();
            return Impulse * Falloff + Direction * Impulse.Size() * Falloff * 0.5f;
        }
    );
}

EForge2DestructionLevel UForge2ChaosManager::GetCurrentDestructionLevel(UGeometryCollectionComponent* Component)
{
    if (!Component) return EForge2DestructionLevel::None;
    
    // Get the rest collection to count total pieces
    const UGeometryCollection* RestCollection = Component->GetRestCollection();
    if (!RestCollection) return EForge2DestructionLevel::None;
    
    TSharedPtr<const FGeometryCollection, ESPMode::ThreadSafe> GeomCollection = RestCollection->GetGeometryCollection();
    if (!GeomCollection.IsValid()) return EForge2DestructionLevel::None;
    
    int32 TotalClusters = GeomCollection->NumElements(FGeometryCollection::TransformGroup);
    if (TotalClusters == 0) return EForge2DestructionLevel::None;
    
    // Count broken/active pieces
    int32 BrokenCount = 0;
    const TArray<bool>& DisabledFlags = Component->GetDisabledFlags();
    for (bool bDisabled : DisabledFlags)
    {
        if (bDisabled) BrokenCount++;
    }
    
    // Calculate destruction percentage
    float DestructionPercent = (float)BrokenCount / (float)TotalClusters;
    
    if (DestructionPercent >= 0.9f) return EForge2DestructionLevel::Total;
    if (DestructionPercent >= 0.6f) return EForge2DestructionLevel::Major;
    if (DestructionPercent >= 0.3f) return EForge2DestructionLevel::Moderate;
    if (DestructionPercent >= 0.1f) return EForge2DestructionLevel::Minor;
    return EForge2DestructionLevel::None;
}

void UForge2ChaosManager::ConfigureDebris(UGeometryCollectionComponent* Component, float Lifespan,
                                            int32 MaxCount, bool bFadeOut)
{
    if (!Component) return;
    
    // Configure removal on break behavior
    Component->SetNotifyRemovals(true);
    
    // Configure decay/removal settings through the geometry collection
    if (UGeometryCollection* GC = const_cast<UGeometryCollection*>(Component->GetRestCollection()))
    {
        TSharedPtr<FGeometryCollection, ESPMode::ThreadSafe> GeomCollection = GC->GetGeometryCollection();
        if (GeomCollection.IsValid())
        {
            // Set removal duration (lifespan) for pieces
            if (!GeomCollection->HasAttribute("RemovalDuration", FGeometryCollection::TransformGroup))
            {
                GeomCollection->AddAttribute<float>("RemovalDuration", FGeometryCollection::TransformGroup);
            }
            TManagedArray<float>& RemovalDurations = GeomCollection->GetAttribute<float>("RemovalDuration", FGeometryCollection::TransformGroup);
            for (int32 i = 0; i < RemovalDurations.Num(); i++)
            {
                RemovalDurations[i] = Lifespan;
            }
        }
    }
    
    // Store max count for runtime limiting
    DebrisMaxCount = MaxCount;
    bDebrisFadeOut = bFadeOut;
    DebrisLifespan = Lifespan;
}

void UForge2ChaosManager::CleanupDebris(UGeometryCollectionComponent* Component)
{
    if (!Component) return;
    
    // Reset the geometry collection to its rest state
    Component->ResetDynamicCollection();
    
    // Clear any pending removal timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
}

void UForge2ChaosManager::SetDebrisPhysicsProperties(UGeometryCollectionComponent* Component,
                                                       float Mass, float Friction, float Restitution)
{
    if (!Component) return;
    
    // Configure physics material properties
    if (UPhysicalMaterial* PhysMat = NewObject<UPhysicalMaterial>(GetTransientPackage()))
    {
        PhysMat->Friction = Friction;
        PhysMat->Restitution = Restitution;
        PhysMat->Density = Mass / 1000.0f; // Convert to density approximation
        
        Component->SetPhysMaterialOverride(PhysMat);
    }
    
    // Set mass scale on the component
    Component->SetMassScale(NAME_None, Mass / 100.0f);

    // Set physics properties for debris pieces
}

AActor* UForge2ChaosManager::CreatePhysicsField(FVector Location, const FForge2PhysicsFieldSettings& Settings)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // Spawn a Field System Actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AFieldSystemActor* FieldActor = World->SpawnActor<AFieldSystemActor>(AFieldSystemActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (FieldActor)
    {
        UFieldSystemComponent* FieldComp = FieldActor->GetFieldSystemComponent();
        if (FieldComp)
        {
            // Create and configure the field
            UpdatePhysicsField(FieldActor, Settings);
        }
    }

    return FieldActor;
}

void UForge2ChaosManager::UpdatePhysicsField(AActor* FieldActor, const FForge2PhysicsFieldSettings& Settings)
{
    if (!FieldActor) return;
    
    AFieldSystemActor* FSActor = Cast<AFieldSystemActor>(FieldActor);
    if (!FSActor) return;
    
    UFieldSystemComponent* FieldComp = FSActor->GetFieldSystemComponent();
    if (!FieldComp) return;
    
    // Clear existing commands
    FieldComp->ResetFieldSystem();
    
    if (!Settings.bEnabled) return;
    
    // Create radial falloff field
    URadialFalloff* RadialFalloff = NewObject<URadialFalloff>(FieldComp);
    RadialFalloff->SetRadialFalloff(Settings.Magnitude, 0.0f, 1.0f, 0.0f, Settings.Radius, FieldActor->GetActorLocation(), EFieldFalloffType::Field_FallOff_Linear);
    
    // Create appropriate field type based on settings
    if (Settings.bKillField)
    {
        UKillField* KillField = NewObject<UKillField>(FieldComp);
        FieldComp->ApplyPhysicsField(true, EFieldPhysicsType::Field_Kill, nullptr, KillField);
    }
    else if (Settings.bSleepField)
    {
        USleepField* SleepField = NewObject<USleepField>(FieldComp);
        FieldComp->ApplyPhysicsField(true, EFieldPhysicsType::Field_SleepingThreshold, nullptr, SleepField);
    }
    else
    {
        // Directional force field
        UUniformVector* DirectionField = NewObject<UUniformVector>(FieldComp);
        DirectionField->SetUniformVector(Settings.Magnitude, Settings.Direction);
        
        // Apply with radial falloff
        FieldComp->ApplyPhysicsField(true, EFieldPhysicsType::Field_LinearForce, RadialFalloff, DirectionField);
    }
}

AActor* UForge2ChaosManager::CreateExplosionField(FVector Location, float Radius, float Magnitude, float Duration)
{
    FForge2PhysicsFieldSettings Settings;
    Settings.Direction = FVector(0, 0, 1);  // Radial outward
    Settings.Magnitude = Magnitude;
    Settings.Radius = Radius;
    Settings.Falloff = 1.0f;
    Settings.bEnabled = true;

    AActor* ExplosionField = CreatePhysicsField(Location, Settings);

    // Set up timer to disable/destroy after duration
    UWorld* World = GetWorld();
    if (ExplosionField && Duration > 0.0f && World)
    {
        FTimerHandle TimerHandle;
        TWeakObjectPtr<AActor> WeakField = ExplosionField;
        World->GetTimerManager().SetTimer(TimerHandle, [WeakField]()
        {
            if (WeakField.IsValid())
            {
                WeakField->Destroy();
            }
        }, Duration, false);
    }

    return ExplosionField;
}

void UForge2ChaosManager::ConfigureCloth(USkeletalMeshComponent* MeshComponent,
                                           const FForge2ClothSettings& Settings)
{
    if (!MeshComponent) return;
    
    // Get the clothing simulation interactor
    IClothingSimulationInteractor* ClothInteractor = MeshComponent->GetClothingSimulationInteractor();
    if (!ClothInteractor) return;
    
    // Configure cloth simulation context
    ClothingSimulationContext Context;
    Context.DeltaSeconds = 1.0f / 60.0f;
    Context.MaxPhysicsDeltaTime = 1.0f / 30.0f;
    Context.WindVelocity = CachedWindVelocity;
    Context.WindAdaption = Settings.WindDrag;
    Context.GravityScale = Settings.GravityScale;
    
    // Apply per-asset configuration through cloth config
    UClothingAssetBase* ClothAsset = MeshComponent->GetClothingAsset();
    if (ClothAsset)
    {
        // Configure physics properties
        ClothInteractor->SetNumIterations(Settings.SolverIterations);
        ClothInteractor->SetNumSubsteps(FMath::Max(1, Settings.SolverIterations / 4));
        
        // Set cloth physical properties via console variables for Chaos Cloth
        IConsoleVariable* MassVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Cloth.MassScale"));
        if (MassVar) MassVar->Set(Settings.Mass);
        
        IConsoleVariable* FrictionVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Cloth.Friction"));
        if (FrictionVar) FrictionVar->Set(Settings.Friction);
        
        IConsoleVariable* DampingVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Cloth.Damping"));
        if (DampingVar) DampingVar->Set(Settings.Damping);
    }
    
    // Enable self collision if requested
    if (Settings.bEnableSelfCollision)
    {
        ClothInteractor->EnableCollision(true);
    }
}

void UForge2ChaosManager::ApplyClothPreset(USkeletalMeshComponent* MeshComponent, EForge2ClothPreset Preset)
{
    FForge2ClothSettings Settings = GetClothPresetSettings(Preset);
    ConfigureCloth(MeshComponent, Settings);
}

void UForge2ChaosManager::SetClothWind(FVector WindVelocity)
{
    CachedWindVelocity = WindVelocity;
    
    // Update wind through the cloth manager
    IConsoleVariable* WindXVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Cloth.WindVelocityX"));
    IConsoleVariable* WindYVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Cloth.WindVelocityY"));
    IConsoleVariable* WindZVar = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Cloth.WindVelocityZ"));
    
    if (WindXVar) WindXVar->Set(WindVelocity.X);
    if (WindYVar) WindYVar->Set(WindVelocity.Y);
    if (WindZVar) WindZVar->Set(WindVelocity.Z);
}

void UForge2ChaosManager::EnableClothSimulation(USkeletalMeshComponent* MeshComponent, bool bEnable)
{
    if (!MeshComponent) return;
    
    // Control cloth simulation state
    if (bEnable)
    {
        MeshComponent->ResumeClothingSimulation();
    }
    else
    {
        MeshComponent->SuspendClothingSimulation();
    }
}

FForge2ClothSettings UForge2ChaosManager::GetClothPresetSettings(EForge2ClothPreset Preset)
{
    FForge2ClothSettings Settings;

    switch (Preset)
    {
        case EForge2ClothPreset::Silk:
            Settings.Mass = 0.5f;
            Settings.StretchStiffness = 0.8f;
            Settings.BendStiffness = 0.2f;
            Settings.ShearStiffness = 0.3f;
            Settings.Damping = 0.05f;
            Settings.WindDrag = 0.2f;
            Settings.WindLift = 0.15f;
            break;

        case EForge2ClothPreset::Cotton:
            Settings.Mass = 1.0f;
            Settings.StretchStiffness = 1.0f;
            Settings.BendStiffness = 0.5f;
            Settings.ShearStiffness = 0.5f;
            Settings.Damping = 0.1f;
            Settings.WindDrag = 0.1f;
            Settings.WindLift = 0.1f;
            break;

        case EForge2ClothPreset::Leather:
            Settings.Mass = 2.0f;
            Settings.StretchStiffness = 1.5f;
            Settings.BendStiffness = 0.8f;
            Settings.ShearStiffness = 0.8f;
            Settings.Damping = 0.2f;
            Settings.WindDrag = 0.05f;
            Settings.WindLift = 0.02f;
            break;

        case EForge2ClothPreset::Rubber:
            Settings.Mass = 1.5f;
            Settings.StretchStiffness = 0.5f;
            Settings.BendStiffness = 0.3f;
            Settings.ShearStiffness = 0.4f;
            Settings.Damping = 0.15f;
            Settings.WindDrag = 0.02f;
            Settings.WindLift = 0.01f;
            break;

        case EForge2ClothPreset::Heavy:
            Settings.Mass = 5.0f;
            Settings.StretchStiffness = 2.0f;
            Settings.BendStiffness = 1.0f;
            Settings.ShearStiffness = 1.0f;
            Settings.Damping = 0.3f;
            Settings.WindDrag = 0.01f;
            Settings.WindLift = 0.005f;
            break;

        default:
            break;
    }

    return Settings;
}

void UForge2ChaosManager::SetupChaosVehicle(AActor* VehicleActor, const FForge2VehicleSettings& Settings)
{
    if (!VehicleActor) return;
    
    // Get or create the vehicle movement component
    UChaosWheeledVehicleMovementComponent* VehicleMovement = VehicleActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    if (!VehicleMovement)
    {
        VehicleMovement = NewObject<UChaosWheeledVehicleMovementComponent>(VehicleActor, TEXT("VehicleMovement"));
        VehicleMovement->RegisterComponent();
    }
    
    // Configure mass properties
    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(VehicleActor->GetRootComponent()))
    {
        RootPrimitive->SetMassOverrideInKg(NAME_None, Settings.Mass, true);
        
        // Set center of mass offset
        FBodyInstance* BodyInstance = RootPrimitive->GetBodyInstance();
        if (BodyInstance)
        {
            BodyInstance->COMNudge = Settings.CenterOfMass;
            BodyInstance->UpdateMassProperties();
        }
    }
    
    // Configure drag
    VehicleMovement->DragCoefficient = Settings.DragCoefficient;
    VehicleMovement->DownforceCoefficient = Settings.DownforceCoefficient;

    // Setup wheels and suspension
    SetupVehicleWheels(VehicleActor, Settings);
    ConfigureVehicleEngine(VehicleActor, Settings.Engine);
}

void UForge2ChaosManager::SetupVehicleWheels(AActor* VehicleActor, const FForge2VehicleSettings& Settings)
{
    UChaosWheeledVehicleMovementComponent* VehicleMovement = VehicleActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    if (!VehicleMovement) return;
    
    // Configure wheel setups
    TArray<FChaosWheelSetup> WheelSetups;
    for (int32 i = 0; i < Settings.Wheels.Num(); i++)
    {
        FChaosWheelSetup WheelSetup;
        WheelSetup.WheelClass = UChaosVehicleWheel::StaticClass();
        WheelSetup.BoneName = FName(*FString::Printf(TEXT("Wheel_%d"), i));
        WheelSetups.Add(WheelSetup);
        
        ConfigureVehicleWheel(VehicleActor, i, Settings.Wheels[i]);
        
        if (i < Settings.Suspensions.Num())
        {
            ConfigureVehicleSuspension(VehicleActor, i, Settings.Suspensions[i]);
        }
    }
    
    VehicleMovement->WheelSetups = WheelSetups;
}

void UForge2ChaosManager::ConfigureVehicleWheel(AActor* VehicleActor, int32 WheelIndex,
                                                  const FForge2VehicleWheelSettings& Settings)
{
    if (!VehicleActor) return;
    
    UChaosWheeledVehicleMovementComponent* VehicleMovement = VehicleActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    if (!VehicleMovement) return;
    
    if (WheelIndex >= 0 && WheelIndex < VehicleMovement->Wheels.Num())
    {
        UChaosVehicleWheel* Wheel = Cast<UChaosVehicleWheel>(VehicleMovement->Wheels[WheelIndex]);
        if (Wheel)
        {
            Wheel->WheelRadius = Settings.Radius;
            Wheel->WheelWidth = Settings.Width;
            Wheel->WheelMass = Settings.Mass;
            Wheel->MaxBrakeTorque = Settings.MaxBrakeTorque;
            Wheel->MaxHandBrakeTorque = Settings.MaxHandBrakeTorque;
            Wheel->MaxSteerAngle = Settings.MaxSteerAngle;
            Wheel->bAffectedByHandbrake = Settings.bAffectedByHandbrake;
            Wheel->bAffectedBySteering = Settings.bAffectedBySteering;
            Wheel->bAffectedByEngine = Settings.bAffectedByEngine;
            Wheel->LateralFrictionForceMultiplier = Settings.LateralStiffness;
            Wheel->LongitudinalFrictionForceMultiplier = Settings.LongitudinalStiffness;
        }
    }
}

void UForge2ChaosManager::ConfigureVehicleSuspension(AActor* VehicleActor, int32 WheelIndex,
                                                       const FForge2VehicleSuspensionSettings& Settings)
{
    if (!VehicleActor) return;
    
    UChaosWheeledVehicleMovementComponent* VehicleMovement = VehicleActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    if (!VehicleMovement) return;
    
    if (WheelIndex >= 0 && WheelIndex < VehicleMovement->Wheels.Num())
    {
        UChaosVehicleWheel* Wheel = Cast<UChaosVehicleWheel>(VehicleMovement->Wheels[WheelIndex]);
        if (Wheel)
        {
            Wheel->SuspensionMaxRaise = Settings.MaxCompression;
            Wheel->SuspensionMaxDrop = Settings.MaxDroop;
            Wheel->SpringRate = Settings.SpringRate;
            Wheel->SpringPreload = Settings.SpringPreload;
            Wheel->SuspensionDampingRatio = Settings.DampingRatio;
            Wheel->WheelLoadRatio = 1.0f;
            Wheel->SuspensionForceOffset = Settings.WheelOffset;
        }
    }
}

void UForge2ChaosManager::ConfigureVehicleEngine(AActor* VehicleActor,
                                                   const FForge2VehicleEngineSettings& Settings)
{
    if (!VehicleActor) return;
    
    UChaosWheeledVehicleMovementComponent* VehicleMovement = VehicleActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    if (!VehicleMovement) return;
    
    // Configure engine settings
    VehicleMovement->EngineSetup.MaxTorque = Settings.MaxTorque;
    VehicleMovement->EngineSetup.MaxRPM = Settings.MaxRPM;
    VehicleMovement->EngineSetup.EngineIdleRPM = Settings.IdleRPM;
    VehicleMovement->EngineSetup.EngineBrakeEffect = Settings.EngineBrakeFactor;
    VehicleMovement->EngineSetup.EngineRevUpMOI = Settings.EngineInertia;
    VehicleMovement->EngineSetup.EngineRevDownRate = Settings.EngineInertia * 0.5f;
    
    // Configure transmission
    VehicleMovement->TransmissionSetup.bUseAutomaticGears = true;
    VehicleMovement->TransmissionSetup.FinalRatio = Settings.FinalDriveRatio;
    VehicleMovement->TransmissionSetup.ChangeUpRPM = Settings.AutomaticShiftUpRPM;
    VehicleMovement->TransmissionSetup.ChangeDownRPM = Settings.AutomaticShiftDownRPM;
    
    // Configure gear ratios
    VehicleMovement->TransmissionSetup.ForwardGearRatios.Empty();
    for (float GearRatio : Settings.GearRatios)
    {
        VehicleMovement->TransmissionSetup.ForwardGearRatios.Add(GearRatio);
    }
    
    // Configure torque curve if provided
    if (Settings.TorqueCurve.Num() > 0)
    {
        VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
        float RPMStep = Settings.MaxRPM / Settings.TorqueCurve.Num();
        for (int32 i = 0; i < Settings.TorqueCurve.Num(); i++)
        {
            VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(RPMStep * i, Settings.TorqueCurve[i]);
        }
    }
}

void UForge2ChaosManager::SetVehicleInput(AActor* VehicleActor, float Throttle, float Steering,
                                            float Brake, bool bHandbrake)
{
    if (!VehicleActor) return;
    
    UChaosWheeledVehicleMovementComponent* VehicleMovement = VehicleActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    if (!VehicleMovement) return;
    
    // Apply normalized input values
    VehicleMovement->SetThrottleInput(FMath::Clamp(Throttle, 0.0f, 1.0f));
    VehicleMovement->SetSteeringInput(FMath::Clamp(Steering, -1.0f, 1.0f));
    VehicleMovement->SetBrakeInput(FMath::Clamp(Brake, 0.0f, 1.0f));
    VehicleMovement->SetHandbrakeInput(bHandbrake);
}

void UForge2ChaosManager::ConfigureNetworkedPhysics(AActor* Actor,
                                                      const FForge2NetworkedPhysicsSettings& Settings)
{
    if (!Actor) return;
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!RootPrimitive) return;
    
    // Store settings for use during synchronization
    CachedNetworkSettings = Settings;
    
    // Configure replication settings
    Actor->SetReplicates(Settings.bEnableNetworkedPhysics);
    Actor->SetReplicateMovement(Settings.bEnableNetworkedPhysics);
    
    // Configure physics body for network prediction
    FBodyInstance* BodyInstance = RootPrimitive->GetBodyInstance();
    if (BodyInstance)
    {
        // Enable physics simulation on all clients
        BodyInstance->bSimulatePhysics = true;
        
        // Configure smoothing for network corrections
        if (Settings.bEnableRewind)
        {
            Actor->NetUpdateFrequency = 1000.0f / Settings.PredictionTimeMs;
        }
    }
}

void UForge2ChaosManager::EnablePhysicsPrediction(AActor* Actor, bool bEnable)
{
    if (!Actor) return;
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!RootPrimitive) return;
    
    if (bEnable)
    {
        // Enable prediction mode - simulate locally while waiting for server updates
        RootPrimitive->SetSimulatePhysics(true);
        Actor->SetReplicateMovement(true);
    }
    else
    {
        // Disable prediction - follow server state exactly
        Actor->SetReplicateMovement(true);
    }
}

void UForge2ChaosManager::SyncPhysicsState(AActor* Actor, FVector Position, FRotator Rotation,
                                            FVector LinearVelocity, FVector AngularVelocity)
{
    if (!Actor) return;

    // Sync physics state from server
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (RootPrimitive)
    {
        RootPrimitive->SetWorldLocationAndRotation(Position, Rotation);
        RootPrimitive->SetPhysicsLinearVelocity(LinearVelocity);
        RootPrimitive->SetPhysicsAngularVelocityInRadians(AngularVelocity);
    }
}

// UForge2ChaosUtility Implementation
FForge2FractureSettings UForge2ChaosUtility::CreateVoronoiFracture(int32 NumPieces, float MinSize)
{
    FForge2FractureSettings Settings;
    Settings.Mode = EForge2FractureMode::Voronoi;
    Settings.NumPieces = NumPieces;
    Settings.MinSize = MinSize;
    Settings.Grout = 0.0f;
    Settings.Noise = 0.1f;
    Settings.bAddInteriorMaterial = true;
    return Settings;
}

FForge2FractureSettings UForge2ChaosUtility::CreateClusteredFracture(FVector Center, float Radius,
                                                                       int32 NumPieces)
{
    FForge2FractureSettings Settings;
    Settings.Mode = EForge2FractureMode::Clustered;
    Settings.NumPieces = NumPieces;
    Settings.ClusterCenter = Center;
    Settings.ClusterRadius = Radius;
    Settings.MinSize = 5.0f;
    Settings.bAddInteriorMaterial = true;
    return Settings;
}

FForge2FractureSettings UForge2ChaosUtility::CreateRadialFracture(FVector Center, int32 NumRings,
                                                                    int32 NumSegments)
{
    FForge2FractureSettings Settings;
    Settings.Mode = EForge2FractureMode::Radial;
    Settings.NumPieces = NumRings * NumSegments;
    Settings.ClusterCenter = Center;
    Settings.bAddInteriorMaterial = true;
    return Settings;
}

FForge2DestructionSettings UForge2ChaosUtility::CreateDefaultDestruction()
{
    FForge2DestructionSettings Settings;
    Settings.bEnableDestruction = true;
    Settings.DamageThreshold = 100.0f;
    Settings.MaxBreakingForce = 1000.0f;
    Settings.ClusterBreakingStrain = 0.5f;
    Settings.bGenerateDebris = true;
    Settings.DebrisLifespan = 5.0f;
    Settings.MaxDebrisCount = 50;
    Settings.bSpawnEffectsOnBreak = true;
    Settings.bPlaySoundOnBreak = true;
    return Settings;
}

FForge2DestructionSettings UForge2ChaosUtility::CreateProgressiveDestruction(int32 NumLevels)
{
    FForge2DestructionSettings Settings;
    Settings.bEnableDestruction = true;
    Settings.DamageThreshold = 100.0f;
    Settings.bGenerateDebris = true;
    Settings.DebrisLifespan = 5.0f;
    Settings.MaxDebrisCount = 50;

    // Create progressive thresholds
    float BaseThreshold = Settings.DamageThreshold;
    for (int32 i = 0; i < NumLevels; i++)
    {
        Settings.LevelThresholds.Add(BaseThreshold * (i + 1));
    }

    return Settings;
}

FForge2ClothSettings UForge2ChaosUtility::CreateClothFromPreset(EForge2ClothPreset Preset)
{
    FForge2ClothSettings Settings;
    Settings.Preset = Preset;

    switch (Preset)
    {
        case EForge2ClothPreset::Silk:
            Settings.Mass = 0.5f;
            Settings.StretchStiffness = 0.8f;
            Settings.BendStiffness = 0.2f;
            break;
        case EForge2ClothPreset::Cotton:
            Settings.Mass = 1.0f;
            Settings.StretchStiffness = 1.0f;
            Settings.BendStiffness = 0.5f;
            break;
        case EForge2ClothPreset::Leather:
            Settings.Mass = 2.0f;
            Settings.StretchStiffness = 1.5f;
            Settings.BendStiffness = 0.8f;
            break;
        default:
            break;
    }

    return Settings;
}

FForge2VehicleSettings UForge2ChaosUtility::CreateDefaultCarSettings()
{
    FForge2VehicleSettings Settings;
    Settings.VehicleType = EForge2VehicleType::Car;
    Settings.Mass = 1500.0f;
    Settings.CenterOfMass = FVector(0, 0, -20);
    Settings.DragCoefficient = 0.3f;

    // Engine
    Settings.Engine.MaxTorque = 300.0f;
    Settings.Engine.MaxRPM = 6000.0f;
    Settings.Engine.IdleRPM = 1000.0f;
    Settings.Engine.GearRatios = {3.5f, 2.5f, 1.8f, 1.4f, 1.1f, 0.9f};  // 6-speed
    Settings.Engine.FinalDriveRatio = 3.5f;

    // 4 wheels
    for (int32 i = 0; i < 4; i++)
    {
        FForge2VehicleWheelSettings Wheel;
        Wheel.Radius = 35.0f;
        Wheel.Width = 20.0f;
        Wheel.bAffectedBySteering = (i < 2);  // Front wheels steer
        Wheel.bAffectedByEngine = true;
        Wheel.bAffectedByHandbrake = (i >= 2);  // Rear wheels have handbrake
        Settings.Wheels.Add(Wheel);

        FForge2VehicleSuspensionSettings Suspension;
        Settings.Suspensions.Add(Suspension);
    }

    return Settings;
}

FForge2VehicleSettings UForge2ChaosUtility::CreateDefaultTruckSettings()
{
    FForge2VehicleSettings Settings = CreateDefaultCarSettings();
    Settings.VehicleType = EForge2VehicleType::Truck;
    Settings.Mass = 5000.0f;
    Settings.Engine.MaxTorque = 800.0f;
    Settings.Engine.MaxRPM = 4000.0f;
    Settings.DragCoefficient = 0.6f;

    for (auto& Wheel : Settings.Wheels)
    {
        Wheel.Radius = 50.0f;
        Wheel.Width = 30.0f;
    }

    return Settings;
}

FForge2VehicleSettings UForge2ChaosUtility::CreateDefaultMotorcycleSettings()
{
    FForge2VehicleSettings Settings;
    Settings.VehicleType = EForge2VehicleType::Motorcycle;
    Settings.Mass = 200.0f;
    Settings.CenterOfMass = FVector(0, 0, 0);
    Settings.DragCoefficient = 0.5f;

    Settings.Engine.MaxTorque = 100.0f;
    Settings.Engine.MaxRPM = 12000.0f;
    Settings.Engine.GearRatios = {2.8f, 2.0f, 1.6f, 1.3f, 1.1f, 0.95f};

    // 2 wheels
    for (int32 i = 0; i < 2; i++)
    {
        FForge2VehicleWheelSettings Wheel;
        Wheel.Radius = 30.0f;
        Wheel.Width = 15.0f;
        Wheel.bAffectedBySteering = (i == 0);
        Wheel.bAffectedByEngine = (i == 1);
        Wheel.bAffectedByHandbrake = false;
        Settings.Wheels.Add(Wheel);

        FForge2VehicleSuspensionSettings Suspension;
        Settings.Suspensions.Add(Suspension);
    }

    return Settings;
}

FForge2NetworkedPhysicsSettings UForge2ChaosUtility::CreateDefaultNetworkSettings()
{
    FForge2NetworkedPhysicsSettings Settings;
    Settings.bEnableNetworkedPhysics = true;
    Settings.PredictionTimeMs = 100.0f;
    Settings.CorrectionSmoothTime = 0.1f;
    Settings.PositionErrorThreshold = 10.0f;
    Settings.RotationErrorThreshold = 5.0f;
    Settings.VelocityErrorThreshold = 50.0f;
    Settings.bEnableRewind = true;
    Settings.HistoryBufferSize = 60;
    return Settings;
}
