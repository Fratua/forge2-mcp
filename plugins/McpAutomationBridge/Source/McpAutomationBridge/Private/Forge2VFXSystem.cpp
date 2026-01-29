// Forge2VFXSystem.cpp - Niagara Visual Effects System Implementation
#include "Forge2VFXSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraEmitter.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"

// Surface color mapping for impact effects
FLinearColor UForge2VFXManager::GetSurfaceColor(const FString& SurfaceType)
{
    static TMap<FString, FLinearColor> SurfaceColorMap;
    if (SurfaceColorMap.Num() == 0)
    {
        SurfaceColorMap.Add(TEXT("Concrete"), FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
        SurfaceColorMap.Add(TEXT("Metal"), FLinearColor(0.7f, 0.7f, 0.8f, 1.0f));
        SurfaceColorMap.Add(TEXT("Wood"), FLinearColor(0.6f, 0.4f, 0.2f, 1.0f));
        SurfaceColorMap.Add(TEXT("Dirt"), FLinearColor(0.4f, 0.3f, 0.2f, 1.0f));
        SurfaceColorMap.Add(TEXT("Sand"), FLinearColor(0.8f, 0.7f, 0.5f, 1.0f));
        SurfaceColorMap.Add(TEXT("Rock"), FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
        SurfaceColorMap.Add(TEXT("Water"), FLinearColor(0.2f, 0.4f, 0.8f, 0.6f));
        SurfaceColorMap.Add(TEXT("Glass"), FLinearColor(0.9f, 0.9f, 1.0f, 0.3f));
        SurfaceColorMap.Add(TEXT("Flesh"), FLinearColor(0.8f, 0.2f, 0.2f, 1.0f));
        SurfaceColorMap.Add(TEXT("Foliage"), FLinearColor(0.2f, 0.5f, 0.2f, 1.0f));
        SurfaceColorMap.Add(TEXT("Snow"), FLinearColor(0.95f, 0.95f, 1.0f, 1.0f));
        SurfaceColorMap.Add(TEXT("Ice"), FLinearColor(0.8f, 0.9f, 1.0f, 0.7f));
        SurfaceColorMap.Add(TEXT("Default"), FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
    }
    
    const FLinearColor* FoundColor = SurfaceColorMap.Find(SurfaceType);
    return FoundColor ? *FoundColor : SurfaceColorMap[TEXT("Default")];
}

// UForge2VFXManager Implementation
UForge2VFXManager::UForge2VFXManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UNiagaraSystem* UForge2VFXManager::CreateNiagaraSystem(const FString& SystemName)
{
#if WITH_EDITOR
    // Create Niagara system package and asset
    FString PackagePath = TEXT("/Game/VFX/Generated/") + SystemName;
    UPackage* Package = CreatePackage(*PackagePath);
    
    UNiagaraSystem* NiagaraSystem = NewObject<UNiagaraSystem>(Package, *SystemName, RF_Public | RF_Standalone);
    if (NiagaraSystem)
    {
        // Initialize the system
        NiagaraSystem->SetAutoImportChangedEmitters(false);
        
        // Create a default emitter
        UNiagaraEmitter* DefaultEmitter = NewObject<UNiagaraEmitter>(NiagaraSystem, TEXT("DefaultEmitter"), RF_Transactional);
        if (DefaultEmitter)
        {
            // Set default emitter properties
            DefaultEmitter->bLocalSpace = false;
            DefaultEmitter->bDeterminism = false;
            
            // Add the emitter to the system
            FNiagaraSystemScalabilityOverride ScalabilityOverride;
            NiagaraSystem->AddEmitterHandle(*DefaultEmitter, FName(*SystemName));
        }
        
        // Compile the system
        NiagaraSystem->RequestCompile(false);
        
        // Save the package
        Package->MarkPackageDirty();
        FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
        UPackage::SavePackage(Package, NiagaraSystem, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);
    }
    
    return NiagaraSystem;
#else
    return nullptr;
#endif
}

UNiagaraComponent* UForge2VFXManager::SpawnSystemAtLocation(UNiagaraSystem* System,
                                                              FVector Location, FRotator Rotation)
{
    if (!System || !GetWorld()) return nullptr;

    UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector(1.0f),
        true,
        true,
        ENCPoolMethod::AutoRelease
    );

    return Component;
}

UNiagaraComponent* UForge2VFXManager::SpawnSystemAttached(UNiagaraSystem* System,
                                                            USceneComponent* AttachTo,
                                                            FName SocketName)
{
    if (!System || !AttachTo) return nullptr;

    UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAttached(
        System,
        AttachTo,
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true,
        true,
        ENCPoolMethod::AutoRelease
    );

    return Component;
}

void UForge2VFXManager::AddEmitter(UNiagaraSystem* System, const FString& EmitterName)
{
    if (!System) return;

#if WITH_EDITOR
    // Adding emitters requires editor functionality
    // In runtime, emitters are pre-configured in the system asset
#endif
}

void UForge2VFXManager::ConfigureSpawn(UNiagaraSystem* System, const FString& EmitterName,
                                         const FForge2ParticleSpawnSettings& Settings)
{
    if (!System) return;

#if WITH_EDITOR
    // Configure spawn module settings
    // This maps our settings to Niagara spawn module parameters
#endif
}

void UForge2VFXManager::ConfigureAttributes(UNiagaraSystem* System, const FString& EmitterName,
                                              const FForge2ParticleAttributes& Attributes)
{
    if (!System) return;

#if WITH_EDITOR
    // Configure particle attributes
    // Maps to Initialize Particle, Update Particle modules
#endif
}

void UForge2VFXManager::AddForce(UNiagaraSystem* System, const FString& EmitterName,
                                   const FForge2ParticleForce& Force)
{
    if (!System) return;

#if WITH_EDITOR
    // Add force module based on type
    // Drag, Noise, Curl, Vortex, Point Attractor, Vector, Gravity
#endif
}

void UForge2VFXManager::ConfigureCollision(UNiagaraSystem* System, const FString& EmitterName,
                                             const FForge2ParticleCollision& Collision)
{
    if (!System) return;

#if WITH_EDITOR
    // Configure collision module
    // Kill on collision, spawn on collision, restitution, friction
#endif
}

void UForge2VFXManager::ConfigureSpriteRenderer(UNiagaraSystem* System, const FString& EmitterName,
                                                   const FForge2SpriteSettings& Settings)
{
    if (!System) return;

#if WITH_EDITOR
    // Configure sprite renderer
    // Texture, sub-images, facing mode, sort offset
#endif
}

void UForge2VFXManager::ConfigureRibbonRenderer(UNiagaraSystem* System, const FString& EmitterName,
                                                   const FForge2RibbonSettings& Settings)
{
    if (!System) return;

#if WITH_EDITOR
    // Configure ribbon renderer
    // Width, segments, facing, twist, UV settings
#endif
}

void UForge2VFXManager::ConfigureMeshRenderer(UNiagaraSystem* System, const FString& EmitterName,
                                                 const FForge2MeshParticleSettings& Settings)
{
    if (!System) return;

#if WITH_EDITOR
    // Configure mesh renderer
    // Mesh, material override, LOD, shadows, motion blur
#endif
}

void UForge2VFXManager::CreateImpactVFX(FVector Location, FVector Normal, const FString& SurfaceType)
{
    // Get impact system from registered effects or load from path based on surface type
    UNiagaraSystem* ImpactSystem = nullptr;
    
    // Check registered surface effects first
    if (UNiagaraSystem** FoundSystem = SurfaceImpactEffects.Find(FName(*SurfaceType)))
    {
        ImpactSystem = *FoundSystem;
    }
    else
    {
        // Load from standard path based on surface type
        FString EffectPath = FString::Printf(TEXT("/Game/VFX/Impacts/NS_Impact_%s.NS_Impact_%s"), *SurfaceType, *SurfaceType);
        ImpactSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
        
        // Fallback to default if not found
        if (!ImpactSystem)
        {
            EffectPath = TEXT("/Game/VFX/Impacts/NS_Impact_Default.NS_Impact_Default");
            ImpactSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
        }
        
        // Cache for future use
        if (ImpactSystem)
        {
            SurfaceImpactEffects.Add(FName(*SurfaceType), ImpactSystem);
        }
    }

    if (ImpactSystem)
    {
        UNiagaraComponent* Component = SpawnSystemAtLocation(
            ImpactSystem,
            Location,
            Normal.Rotation()
        );

        if (Component)
        {
            // Set impact parameters
            Component->SetVariableVec3(TEXT("ImpactNormal"), Normal);
            Component->SetVariableFloat(TEXT("ImpactIntensity"), 1.0f);
            Component->SetVariableLinearColor(TEXT("ImpactColor"), GetSurfaceColor(SurfaceType));
        }
    }
}

void UForge2VFXManager::CreateMuzzleFlashVFX(FVector Location, FRotator Direction, const FString& WeaponType)
{
    UNiagaraSystem* MuzzleSystem = nullptr;
    
    // Define weapon-specific parameters
    float FlashScale = 1.0f;
    float FlashIntensity = 1.0f;
    FLinearColor FlashColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f);
    int32 SmokeAmount = 10;

    // Load muzzle flash based on weapon type and configure parameters
    if (WeaponType.Equals(TEXT("Pistol"), ESearchCase::IgnoreCase))
    {
        FString EffectPath = TEXT("/Game/VFX/Weapons/NS_MuzzleFlash_Pistol.NS_MuzzleFlash_Pistol");
        MuzzleSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
        FlashScale = 0.8f;
        FlashIntensity = 0.9f;
        SmokeAmount = 5;
    }
    else if (WeaponType.Equals(TEXT("Rifle"), ESearchCase::IgnoreCase) || 
             WeaponType.Equals(TEXT("AssaultRifle"), ESearchCase::IgnoreCase))
    {
        FString EffectPath = TEXT("/Game/VFX/Weapons/NS_MuzzleFlash_Rifle.NS_MuzzleFlash_Rifle");
        MuzzleSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
        FlashScale = 1.0f;
        FlashIntensity = 1.2f;
        SmokeAmount = 8;
    }
    else if (WeaponType.Equals(TEXT("Shotgun"), ESearchCase::IgnoreCase))
    {
        FString EffectPath = TEXT("/Game/VFX/Weapons/NS_MuzzleFlash_Shotgun.NS_MuzzleFlash_Shotgun");
        MuzzleSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
        FlashScale = 1.5f;
        FlashIntensity = 1.8f;
        SmokeAmount = 20;
    }
    else if (WeaponType.Equals(TEXT("Sniper"), ESearchCase::IgnoreCase))
    {
        FString EffectPath = TEXT("/Game/VFX/Weapons/NS_MuzzleFlash_Sniper.NS_MuzzleFlash_Sniper");
        MuzzleSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
        FlashScale = 1.3f;
        FlashIntensity = 1.5f;
        SmokeAmount = 15;
    }
    else if (WeaponType.Equals(TEXT("Energy"), ESearchCase::IgnoreCase) ||
             WeaponType.Equals(TEXT("Plasma"), ESearchCase::IgnoreCase))
    {
        FString EffectPath = TEXT("/Game/VFX/Weapons/NS_MuzzleFlash_Energy.NS_MuzzleFlash_Energy");
        MuzzleSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
        FlashScale = 1.0f;
        FlashIntensity = 2.0f;
        FlashColor = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f);  // Blue energy color
        SmokeAmount = 0;  // Energy weapons don't produce smoke
    }
    else if (WeaponType.Equals(TEXT("SMG"), ESearchCase::IgnoreCase))
    {
        FString EffectPath = TEXT("/Game/VFX/Weapons/NS_MuzzleFlash_SMG.NS_MuzzleFlash_SMG");
        MuzzleSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
        FlashScale = 0.7f;
        FlashIntensity = 0.8f;
        SmokeAmount = 4;
    }
    
    // Fallback to default muzzle flash if specific type not found
    if (!MuzzleSystem)
    {
        FString DefaultPath = TEXT("/Game/VFX/Weapons/NS_MuzzleFlash_Default.NS_MuzzleFlash_Default");
        MuzzleSystem = LoadObject<UNiagaraSystem>(nullptr, *DefaultPath);
    }

    if (MuzzleSystem)
    {
        UNiagaraComponent* Component = SpawnSystemAtLocation(
            MuzzleSystem,
            Location,
            Direction
        );

        if (Component)
        {
            Component->SetVariableFloat(TEXT("FlashIntensity"), FlashIntensity);
            Component->SetVariableFloat(TEXT("FlashScale"), FlashScale);
            Component->SetVariableLinearColor(TEXT("FlashColor"), FlashColor);
            Component->SetVariableInt(TEXT("SmokeParticleCount"), SmokeAmount);
            Component->SetVariableVec3(TEXT("FlashDirection"), Direction.Vector());
        }
    }
}

void UForge2VFXManager::CreateBloodSplatterVFX(FVector Location, FVector Direction, float Intensity)
{
    UNiagaraSystem* BloodSystem = nullptr;
    
    // Determine which blood effect to use based on intensity
    FString EffectPath;
    if (Intensity > 1.5f)
    {
        // High intensity - large splatter (headshots, critical hits)
        EffectPath = TEXT("/Game/VFX/Gore/NS_BloodSplatter_Large.NS_BloodSplatter_Large");
    }
    else if (Intensity > 0.8f)
    {
        // Medium intensity - standard splatter
        EffectPath = TEXT("/Game/VFX/Gore/NS_BloodSplatter_Medium.NS_BloodSplatter_Medium");
    }
    else
    {
        // Low intensity - small splatter (grazes, minor hits)
        EffectPath = TEXT("/Game/VFX/Gore/NS_BloodSplatter_Small.NS_BloodSplatter_Small");
    }
    
    BloodSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
    
    // Fallback to default if specific variant not found
    if (!BloodSystem)
    {
        EffectPath = TEXT("/Game/VFX/Gore/NS_BloodSplatter_Default.NS_BloodSplatter_Default");
        BloodSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
    }

    if (BloodSystem)
    {
        // Normalize direction for consistent behavior
        FVector NormalizedDirection = Direction.GetSafeNormal();
        if (NormalizedDirection.IsNearlyZero())
        {
            NormalizedDirection = FVector::UpVector;  // Default upward if no direction
        }
        
        UNiagaraComponent* Component = SpawnSystemAtLocation(
            BloodSystem,
            Location,
            NormalizedDirection.Rotation()
        );

        if (Component)
        {
            // Core parameters
            Component->SetVariableFloat(TEXT("SplatterIntensity"), Intensity);
            Component->SetVariableVec3(TEXT("SplatterDirection"), NormalizedDirection);
            
            // Blood coloring (allow variation)
            FLinearColor BloodColor = FLinearColor(0.5f, 0.0f, 0.0f, 1.0f);  // Dark red
            BloodColor.R += FMath::FRandRange(-0.1f, 0.1f);  // Slight variation
            Component->SetVariableLinearColor(TEXT("BloodColor"), BloodColor);
            
            // Particle counts based on intensity
            int32 DropletCount = FMath::RoundToInt(30.0f * Intensity);
            int32 MistCount = FMath::RoundToInt(50.0f * Intensity);
            Component->SetVariableInt(TEXT("DropletCount"), DropletCount);
            Component->SetVariableInt(TEXT("MistCount"), MistCount);
            
            // Velocity scaling based on intensity
            float VelocityScale = FMath::Lerp(0.5f, 2.0f, FMath::Clamp(Intensity, 0.0f, 2.0f) / 2.0f);
            Component->SetVariableFloat(TEXT("VelocityScale"), VelocityScale);
            
            // Spread angle (higher intensity = wider spread)
            float SpreadAngle = FMath::Lerp(15.0f, 60.0f, FMath::Clamp(Intensity, 0.0f, 2.0f) / 2.0f);
            Component->SetVariableFloat(TEXT("SpreadAngle"), SpreadAngle);
            
            // Gravity influence
            Component->SetVariableFloat(TEXT("GravityScale"), 1.0f);
            
            // Decal spawning parameters
            Component->SetVariableBool(TEXT("SpawnDecals"), true);
            Component->SetVariableFloat(TEXT("DecalSize"), 20.0f * Intensity);
        }
    }
}

void UForge2VFXManager::CreateExplosionVFX(FVector Location, float Radius, const FString& ExplosionType)
{
    UNiagaraSystem* ExplosionSystem = nullptr;
    
    // Type-specific parameters
    FLinearColor ExplosionColor = FLinearColor(1.0f, 0.5f, 0.1f, 1.0f);  // Default fiery orange
    float FireIntensity = 1.0f;
    float SmokeIntensity = 1.0f;
    float DebrisAmount = 1.0f;
    float ShockwaveStrength = 1.0f;
    bool bHasSecondaryFires = false;
    
    // Load explosion system and configure based on type
    FString EffectPath;
    
    if (ExplosionType.Equals(TEXT("Fire"), ESearchCase::IgnoreCase) ||
        ExplosionType.Equals(TEXT("Incendiary"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Explosions/NS_Explosion_Fire.NS_Explosion_Fire");
        FireIntensity = 2.0f;
        SmokeIntensity = 0.5f;
        bHasSecondaryFires = true;
        ExplosionColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f);
    }
    else if (ExplosionType.Equals(TEXT("Smoke"), ESearchCase::IgnoreCase) ||
             ExplosionType.Equals(TEXT("Flashbang"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Explosions/NS_Explosion_Smoke.NS_Explosion_Smoke");
        FireIntensity = 0.2f;
        SmokeIntensity = 3.0f;
        DebrisAmount = 0.1f;
        ShockwaveStrength = 1.5f;
        ExplosionColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    }
    else if (ExplosionType.Equals(TEXT("Electric"), ESearchCase::IgnoreCase) ||
             ExplosionType.Equals(TEXT("EMP"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Explosions/NS_Explosion_Electric.NS_Explosion_Electric");
        FireIntensity = 0.0f;
        SmokeIntensity = 0.3f;
        DebrisAmount = 0.2f;
        ShockwaveStrength = 2.0f;
        ExplosionColor = FLinearColor(0.3f, 0.5f, 1.0f, 1.0f);
    }
    else if (ExplosionType.Equals(TEXT("Plasma"), ESearchCase::IgnoreCase) ||
             ExplosionType.Equals(TEXT("Energy"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Explosions/NS_Explosion_Plasma.NS_Explosion_Plasma");
        FireIntensity = 0.5f;
        SmokeIntensity = 0.2f;
        DebrisAmount = 0.0f;
        ShockwaveStrength = 1.8f;
        ExplosionColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
    }
    else if (ExplosionType.Equals(TEXT("Debris"), ESearchCase::IgnoreCase) ||
             ExplosionType.Equals(TEXT("Structural"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Explosions/NS_Explosion_Debris.NS_Explosion_Debris");
        FireIntensity = 0.3f;
        SmokeIntensity = 1.5f;
        DebrisAmount = 3.0f;
        ShockwaveStrength = 0.8f;
        ExplosionColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
    }
    else if (ExplosionType.Equals(TEXT("Nuclear"), ESearchCase::IgnoreCase) ||
             ExplosionType.Equals(TEXT("Massive"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Explosions/NS_Explosion_Nuclear.NS_Explosion_Nuclear");
        FireIntensity = 3.0f;
        SmokeIntensity = 4.0f;
        DebrisAmount = 2.0f;
        ShockwaveStrength = 5.0f;
        bHasSecondaryFires = true;
        ExplosionColor = FLinearColor(1.0f, 1.0f, 0.8f, 1.0f);
    }
    else
    {
        // Default explosion
        EffectPath = TEXT("/Game/VFX/Explosions/NS_Explosion_Default.NS_Explosion_Default");
    }
    
    ExplosionSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
    
    // Fallback to default if specific type not found
    if (!ExplosionSystem)
    {
        FString DefaultPath = TEXT("/Game/VFX/Explosions/NS_Explosion_Default.NS_Explosion_Default");
        ExplosionSystem = LoadObject<UNiagaraSystem>(nullptr, *DefaultPath);
    }

    if (ExplosionSystem)
    {
        UNiagaraComponent* Component = SpawnSystemAtLocation(
            ExplosionSystem,
            Location,
            FRotator::ZeroRotator
        );

        if (Component)
        {
            // Scale all parameters by radius
            float RadiusScale = Radius / 100.0f;  // Normalize to 100 unit base
            
            // Core explosion parameters
            Component->SetVariableFloat(TEXT("ExplosionRadius"), Radius);
            Component->SetVariableFloat(TEXT("ExplosionIntensity"), RadiusScale);
            Component->SetVariableLinearColor(TEXT("ExplosionColor"), ExplosionColor);
            
            // Fire parameters
            Component->SetVariableFloat(TEXT("FireIntensity"), FireIntensity * RadiusScale);
            Component->SetVariableFloat(TEXT("FireRadius"), Radius * 0.8f);
            Component->SetVariableFloat(TEXT("FireDuration"), 0.5f + (RadiusScale * 0.5f));
            Component->SetVariableBool(TEXT("SpawnSecondaryFires"), bHasSecondaryFires);
            
            // Smoke parameters
            Component->SetVariableFloat(TEXT("SmokeIntensity"), SmokeIntensity * RadiusScale);
            Component->SetVariableFloat(TEXT("SmokeRadius"), Radius * 1.5f);
            Component->SetVariableFloat(TEXT("SmokeDuration"), 2.0f + RadiusScale);
            Component->SetVariableInt(TEXT("SmokeParticleCount"), FMath::RoundToInt(100 * SmokeIntensity * RadiusScale));
            
            // Debris parameters
            Component->SetVariableFloat(TEXT("DebrisAmount"), DebrisAmount * RadiusScale);
            Component->SetVariableInt(TEXT("DebrisCount"), FMath::RoundToInt(50 * DebrisAmount * RadiusScale));
            Component->SetVariableFloat(TEXT("DebrisSpread"), Radius * 2.0f);
            Component->SetVariableFloat(TEXT("DebrisVelocity"), 500.0f * RadiusScale);
            
            // Shockwave parameters
            Component->SetVariableFloat(TEXT("ShockwaveStrength"), ShockwaveStrength * RadiusScale);
            Component->SetVariableFloat(TEXT("ShockwaveRadius"), Radius * 1.2f);
            Component->SetVariableFloat(TEXT("ShockwaveSpeed"), 2000.0f);
            
            // Light flash parameters
            Component->SetVariableFloat(TEXT("LightIntensity"), 10000.0f * RadiusScale);
            Component->SetVariableFloat(TEXT("LightRadius"), Radius * 3.0f);
            Component->SetVariableLinearColor(TEXT("LightColor"), ExplosionColor);
            
            // Camera shake trigger radius
            Component->SetVariableFloat(TEXT("CameraShakeRadius"), Radius * 5.0f);
            Component->SetVariableFloat(TEXT("CameraShakeIntensity"), RadiusScale);
        }
    }
}

void UForge2VFXManager::CreateFootstepVFX(FVector Location, const FString& SurfaceType)
{
    UNiagaraSystem* FootstepSystem = nullptr;
    
    // Surface-specific parameters
    FLinearColor ParticleColor = FLinearColor::White;
    float ParticleScale = 1.0f;
    int32 ParticleCount = 10;
    float ParticleLifetime = 1.0f;
    float ParticleVelocity = 50.0f;
    bool bSpawnRipples = false;
    bool bSpawnSplash = false;
    
    // Determine effect path and parameters based on surface type
    FString EffectPath;
    
    if (SurfaceType.Equals(TEXT("Dirt"), ESearchCase::IgnoreCase) ||
        SurfaceType.Equals(TEXT("Mud"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Dirt.NS_Footstep_Dirt");
        ParticleColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        ParticleCount = 15;
        ParticleLifetime = 1.5f;
        ParticleVelocity = 30.0f;
    }
    else if (SurfaceType.Equals(TEXT("Sand"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Sand.NS_Footstep_Sand");
        ParticleColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
        ParticleCount = 20;
        ParticleLifetime = 2.0f;
        ParticleVelocity = 20.0f;
        ParticleScale = 0.8f;
    }
    else if (SurfaceType.Equals(TEXT("Water"), ESearchCase::IgnoreCase) ||
             SurfaceType.Equals(TEXT("Shallow"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Water.NS_Footstep_Water");
        ParticleColor = FLinearColor(0.6f, 0.8f, 1.0f, 0.8f);
        ParticleCount = 30;
        ParticleLifetime = 0.8f;
        ParticleVelocity = 100.0f;
        bSpawnRipples = true;
        bSpawnSplash = true;
    }
    else if (SurfaceType.Equals(TEXT("Snow"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Snow.NS_Footstep_Snow");
        ParticleColor = FLinearColor(0.95f, 0.95f, 1.0f, 1.0f);
        ParticleCount = 25;
        ParticleLifetime = 2.5f;
        ParticleVelocity = 15.0f;
        ParticleScale = 1.2f;
    }
    else if (SurfaceType.Equals(TEXT("Grass"), ESearchCase::IgnoreCase) ||
             SurfaceType.Equals(TEXT("Foliage"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Grass.NS_Footstep_Grass");
        ParticleColor = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);
        ParticleCount = 8;
        ParticleLifetime = 1.0f;
        ParticleVelocity = 40.0f;
    }
    else if (SurfaceType.Equals(TEXT("Concrete"), ESearchCase::IgnoreCase) ||
             SurfaceType.Equals(TEXT("Stone"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Concrete.NS_Footstep_Concrete");
        ParticleColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
        ParticleCount = 5;
        ParticleLifetime = 0.5f;
        ParticleVelocity = 60.0f;
        ParticleScale = 0.5f;
    }
    else if (SurfaceType.Equals(TEXT("Metal"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Metal.NS_Footstep_Metal");
        ParticleColor = FLinearColor(0.7f, 0.7f, 0.75f, 1.0f);
        ParticleCount = 3;
        ParticleLifetime = 0.3f;
        ParticleVelocity = 80.0f;
        ParticleScale = 0.3f;
    }
    else if (SurfaceType.Equals(TEXT("Wood"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Wood.NS_Footstep_Wood");
        ParticleColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
        ParticleCount = 6;
        ParticleLifetime = 0.8f;
        ParticleVelocity = 45.0f;
        ParticleScale = 0.6f;
    }
    else if (SurfaceType.Equals(TEXT("Gravel"), ESearchCase::IgnoreCase))
    {
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Gravel.NS_Footstep_Gravel");
        ParticleColor = FLinearColor(0.55f, 0.5f, 0.45f, 1.0f);
        ParticleCount = 12;
        ParticleLifetime = 0.6f;
        ParticleVelocity = 70.0f;
    }
    else
    {
        // Default footstep
        EffectPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Default.NS_Footstep_Default");
    }
    
    FootstepSystem = LoadObject<UNiagaraSystem>(nullptr, *EffectPath);
    
    // Fallback to default if specific type not found
    if (!FootstepSystem)
    {
        FString DefaultPath = TEXT("/Game/VFX/Footsteps/NS_Footstep_Default.NS_Footstep_Default");
        FootstepSystem = LoadObject<UNiagaraSystem>(nullptr, *DefaultPath);
    }

    if (FootstepSystem)
    {
        UNiagaraComponent* Component = SpawnSystemAtLocation(
            FootstepSystem,
            Location,
            FRotator::ZeroRotator
        );
        
        if (Component)
        {
            // Core particle parameters
            Component->SetVariableLinearColor(TEXT("ParticleColor"), ParticleColor);
            Component->SetVariableFloat(TEXT("ParticleScale"), ParticleScale);
            Component->SetVariableInt(TEXT("ParticleCount"), ParticleCount);
            Component->SetVariableFloat(TEXT("ParticleLifetime"), ParticleLifetime);
            Component->SetVariableFloat(TEXT("ParticleVelocity"), ParticleVelocity);
            
            // Surface-specific features
            Component->SetVariableBool(TEXT("SpawnRipples"), bSpawnRipples);
            Component->SetVariableBool(TEXT("SpawnSplash"), bSpawnSplash);
            
            // General parameters
            Component->SetVariableFloat(TEXT("SpreadRadius"), 15.0f);
            Component->SetVariableVec3(TEXT("SpawnLocation"), Location);
            Component->SetVariableFloat(TEXT("GravityScale"), 1.0f);
        }
    }
}

void UForge2VFXManager::InitializePool(UNiagaraSystem* System, int32 PoolSize)
{
    if (!System || PoolSize <= 0) return;

    TArray<UNiagaraComponent*>& Pool = VFXPool.FindOrAdd(System);

    // Pre-spawn deactivated components
    for (int32 i = Pool.Num(); i < PoolSize; ++i)
    {
        UNiagaraComponent* Component = NewObject<UNiagaraComponent>(this);
        if (Component)
        {
            Component->SetAsset(System);
            Component->SetAutoActivate(false);
            Component->RegisterComponent();
            Pool.Add(Component);
        }
    }
}

UNiagaraComponent* UForge2VFXManager::GetFromPool(UNiagaraSystem* System)
{
    if (!System) return nullptr;

    TArray<UNiagaraComponent*>* Pool = VFXPool.Find(System);
    if (!Pool || Pool->Num() == 0)
    {
        // Pool empty or doesn't exist, spawn new
        return SpawnSystemAtLocation(System, FVector::ZeroVector, FRotator::ZeroRotator);
    }

    // Find inactive component
    for (UNiagaraComponent* Component : *Pool)
    {
        if (Component && !Component->IsActive())
        {
            Component->Activate(true);
            return Component;
        }
    }

    // All in use, spawn new
    return SpawnSystemAtLocation(System, FVector::ZeroVector, FRotator::ZeroRotator);
}

void UForge2VFXManager::ReturnToPool(UNiagaraComponent* Component)
{
    if (!Component) return;

    Component->Deactivate();
    Component->SetWorldLocation(FVector::ZeroVector);
}

UNiagaraSystem* UForge2VFXManager::CreateFromPreset(const FForge2VFXPreset& Preset)
{
#if WITH_EDITOR
    UNiagaraSystem* System = CreateNiagaraSystem(Preset.PresetName);

    if (System)
    {
        // Add emitter
        AddEmitter(System, TEXT("MainEmitter"));

        // Configure based on preset
        ConfigureSpawn(System, TEXT("MainEmitter"), Preset.SpawnSettings);
        ConfigureAttributes(System, TEXT("MainEmitter"), Preset.Attributes);

        // Add forces
        for (const FForge2ParticleForce& Force : Preset.Forces)
        {
            AddForce(System, TEXT("MainEmitter"), Force);
        }

        // Configure collision
        ConfigureCollision(System, TEXT("MainEmitter"), Preset.Collision);

        // Configure renderer
        switch (Preset.RenderType)
        {
            case EForge2ParticleRenderType::Sprite:
                ConfigureSpriteRenderer(System, TEXT("MainEmitter"), Preset.SpriteSettings);
                break;

            case EForge2ParticleRenderType::Ribbon:
                // Configure ribbon
                break;

            case EForge2ParticleRenderType::Mesh:
                // Configure mesh
                break;

            default:
                break;
        }
    }

    return System;
#else
    return nullptr;
#endif
}

FForge2VFXPreset UForge2VFXManager::GetFirePreset()
{
    FForge2VFXPreset Preset;
    Preset.PresetName = TEXT("Fire");
    Preset.Category = EForge2VFXCategory::Fire;

    // Spawn settings - continuous upward flames
    Preset.SpawnSettings.Mode = EForge2SpawnMode::Continuous;
    Preset.SpawnSettings.SpawnRate = 50.0f;
    Preset.SpawnSettings.MaxParticles = 500;
    Preset.SpawnSettings.SpawnRadius = 20.0f;

    // Particle attributes
    Preset.Attributes.LifetimeRange = FVector2D(0.5f, 1.5f);
    Preset.Attributes.SizeRange = FVector2D(20.0f, 50.0f);
    Preset.Attributes.ColorStart = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);
    Preset.Attributes.ColorEnd = FLinearColor(1.0f, 0.2f, 0.0f, 0.0f);
    Preset.Attributes.VelocityRange = FVector2D(50.0f, 150.0f);
    Preset.Attributes.VelocityDirection = FVector(0, 0, 1);
    Preset.Attributes.VelocityConeAngle = 15.0f;

    // Forces - upward with noise
    FForge2ParticleForce NoiseForce;
    NoiseForce.Type = EForge2ForceType::Noise;
    NoiseForce.NoiseAmplitude = 50.0f;
    NoiseForce.NoiseFrequency = 2.0f;
    Preset.Forces.Add(NoiseForce);

    FForge2ParticleForce DragForce;
    DragForce.Type = EForge2ForceType::Drag;
    DragForce.Strength = 0.5f;
    Preset.Forces.Add(DragForce);

    Preset.RenderType = EForge2ParticleRenderType::Sprite;

    return Preset;
}

FForge2VFXPreset UForge2VFXManager::GetSmokePreset()
{
    FForge2VFXPreset Preset;
    Preset.PresetName = TEXT("Smoke");
    Preset.Category = EForge2VFXCategory::Smoke;

    // Spawn settings
    Preset.SpawnSettings.Mode = EForge2SpawnMode::Continuous;
    Preset.SpawnSettings.SpawnRate = 20.0f;
    Preset.SpawnSettings.MaxParticles = 200;
    Preset.SpawnSettings.SpawnRadius = 30.0f;

    // Particle attributes - slow rise, expand, fade
    Preset.Attributes.LifetimeRange = FVector2D(3.0f, 5.0f);
    Preset.Attributes.SizeRange = FVector2D(50.0f, 150.0f);
    Preset.Attributes.ColorStart = FLinearColor(0.3f, 0.3f, 0.3f, 0.8f);
    Preset.Attributes.ColorEnd = FLinearColor(0.5f, 0.5f, 0.5f, 0.0f);
    Preset.Attributes.VelocityRange = FVector2D(20.0f, 50.0f);
    Preset.Attributes.VelocityDirection = FVector(0, 0, 1);
    Preset.Attributes.VelocityConeAngle = 30.0f;

    // Forces
    FForge2ParticleForce CurlForce;
    CurlForce.Type = EForge2ForceType::Curl;
    CurlForce.Strength = 30.0f;
    CurlForce.NoiseFrequency = 0.5f;
    Preset.Forces.Add(CurlForce);

    FForge2ParticleForce DragForce;
    DragForce.Type = EForge2ForceType::Drag;
    DragForce.Strength = 0.8f;
    Preset.Forces.Add(DragForce);

    Preset.RenderType = EForge2ParticleRenderType::Sprite;

    return Preset;
}

FForge2VFXPreset UForge2VFXManager::GetSparkPreset()
{
    FForge2VFXPreset Preset;
    Preset.PresetName = TEXT("Sparks");
    Preset.Category = EForge2VFXCategory::Electricity;

    // Spawn settings - burst of sparks
    Preset.SpawnSettings.Mode = EForge2SpawnMode::Burst;
    Preset.SpawnSettings.BurstCount = 30;
    Preset.SpawnSettings.MaxParticles = 100;

    // Particle attributes - fast, bright, short-lived
    Preset.Attributes.LifetimeRange = FVector2D(0.2f, 0.5f);
    Preset.Attributes.SizeRange = FVector2D(2.0f, 5.0f);
    Preset.Attributes.ColorStart = FLinearColor(1.0f, 0.9f, 0.5f, 1.0f);
    Preset.Attributes.ColorEnd = FLinearColor(1.0f, 0.5f, 0.0f, 0.0f);
    Preset.Attributes.VelocityRange = FVector2D(200.0f, 500.0f);
    Preset.Attributes.VelocityDirection = FVector(0, 0, 1);
    Preset.Attributes.VelocityConeAngle = 60.0f;

    // Forces - gravity, drag
    FForge2ParticleForce GravityForce;
    GravityForce.Type = EForge2ForceType::Gravity;
    GravityForce.Strength = 980.0f;
    Preset.Forces.Add(GravityForce);

    FForge2ParticleForce DragForce;
    DragForce.Type = EForge2ForceType::Drag;
    DragForce.Strength = 0.2f;
    Preset.Forces.Add(DragForce);

    // Collision
    Preset.Collision.bEnabled = true;
    Preset.Collision.Restitution = 0.3f;
    Preset.Collision.bKillOnCollision = false;

    Preset.RenderType = EForge2ParticleRenderType::Sprite;

    return Preset;
}

FForge2VFXPreset UForge2VFXManager::GetDustPreset()
{
    FForge2VFXPreset Preset;
    Preset.PresetName = TEXT("Dust");
    Preset.Category = EForge2VFXCategory::Debris;

    // Spawn settings
    Preset.SpawnSettings.Mode = EForge2SpawnMode::Burst;
    Preset.SpawnSettings.BurstCount = 15;
    Preset.SpawnSettings.MaxParticles = 50;
    Preset.SpawnSettings.SpawnRadius = 20.0f;

    // Particle attributes - slow settling
    Preset.Attributes.LifetimeRange = FVector2D(2.0f, 4.0f);
    Preset.Attributes.SizeRange = FVector2D(30.0f, 80.0f);
    Preset.Attributes.ColorStart = FLinearColor(0.6f, 0.5f, 0.4f, 0.5f);
    Preset.Attributes.ColorEnd = FLinearColor(0.6f, 0.5f, 0.4f, 0.0f);
    Preset.Attributes.VelocityRange = FVector2D(10.0f, 40.0f);
    Preset.Attributes.VelocityDirection = FVector(0, 0, 1);
    Preset.Attributes.VelocityConeAngle = 45.0f;

    // Forces
    FForge2ParticleForce GravityForce;
    GravityForce.Type = EForge2ForceType::Gravity;
    GravityForce.Strength = 50.0f;
    Preset.Forces.Add(GravityForce);

    FForge2ParticleForce DragForce;
    DragForce.Type = EForge2ForceType::Drag;
    DragForce.Strength = 2.0f;
    Preset.Forces.Add(DragForce);

    Preset.RenderType = EForge2ParticleRenderType::Sprite;

    return Preset;
}

FForge2VFXPreset UForge2VFXManager::GetMagicPreset(FLinearColor Color)
{
    FForge2VFXPreset Preset;
    Preset.PresetName = TEXT("Magic");
    Preset.Category = EForge2VFXCategory::Magic;

    // Spawn settings
    Preset.SpawnSettings.Mode = EForge2SpawnMode::Continuous;
    Preset.SpawnSettings.SpawnRate = 40.0f;
    Preset.SpawnSettings.MaxParticles = 300;
    Preset.SpawnSettings.SpawnRadius = 50.0f;

    // Particle attributes - magical swirl
    Preset.Attributes.LifetimeRange = FVector2D(1.0f, 2.0f);
    Preset.Attributes.SizeRange = FVector2D(5.0f, 15.0f);
    Preset.Attributes.ColorStart = Color;
    Preset.Attributes.ColorEnd = FLinearColor(Color.R, Color.G, Color.B, 0.0f);
    Preset.Attributes.VelocityRange = FVector2D(30.0f, 80.0f);
    Preset.Attributes.VelocityDirection = FVector(0, 0, 1);
    Preset.Attributes.VelocityConeAngle = 180.0f;

    // Forces - vortex for swirling effect
    FForge2ParticleForce VortexForce;
    VortexForce.Type = EForge2ForceType::Vortex;
    VortexForce.Strength = 100.0f;
    VortexForce.AttractorLocation = FVector::ZeroVector;
    Preset.Forces.Add(VortexForce);

    FForge2ParticleForce NoiseForce;
    NoiseForce.Type = EForge2ForceType::Noise;
    NoiseForce.NoiseAmplitude = 30.0f;
    NoiseForce.NoiseFrequency = 1.0f;
    Preset.Forces.Add(NoiseForce);

    Preset.RenderType = EForge2ParticleRenderType::Sprite;

    return Preset;
}

// UForge2VFXUtility Implementation
FForge2ParticleSpawnSettings UForge2VFXUtility::CreateContinuousSpawn(float Rate, int32 MaxParticles)
{
    FForge2ParticleSpawnSettings Settings;
    Settings.Mode = EForge2SpawnMode::Continuous;
    Settings.SpawnRate = Rate;
    Settings.MaxParticles = MaxParticles;
    return Settings;
}

FForge2ParticleSpawnSettings UForge2VFXUtility::CreateBurstSpawn(int32 Count, int32 BurstCount)
{
    FForge2ParticleSpawnSettings Settings;
    Settings.Mode = EForge2SpawnMode::Burst;
    Settings.BurstCount = Count;
    Settings.MaxParticles = Count * BurstCount;
    return Settings;
}

FForge2ParticleAttributes UForge2VFXUtility::CreateDefaultAttributes()
{
    FForge2ParticleAttributes Attributes;
    Attributes.LifetimeRange = FVector2D(1.0f, 2.0f);
    Attributes.SizeRange = FVector2D(10.0f, 20.0f);
    Attributes.ColorStart = FLinearColor::White;
    Attributes.ColorEnd = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);
    Attributes.VelocityRange = FVector2D(50.0f, 100.0f);
    Attributes.VelocityDirection = FVector(0, 0, 1);
    Attributes.VelocityConeAngle = 30.0f;
    return Attributes;
}

FForge2ParticleForce UForge2VFXUtility::CreateGravityForce(float Strength)
{
    FForge2ParticleForce Force;
    Force.Type = EForge2ForceType::Gravity;
    Force.Strength = Strength;
    Force.Direction = FVector(0, 0, -1);
    return Force;
}

FForge2ParticleForce UForge2VFXUtility::CreateDragForce(float Strength)
{
    FForge2ParticleForce Force;
    Force.Type = EForge2ForceType::Drag;
    Force.Strength = Strength;
    return Force;
}

FForge2ParticleForce UForge2VFXUtility::CreateNoiseForce(float Amplitude, float Frequency)
{
    FForge2ParticleForce Force;
    Force.Type = EForge2ForceType::Noise;
    Force.NoiseAmplitude = Amplitude;
    Force.NoiseFrequency = Frequency;
    return Force;
}

FForge2ParticleForce UForge2VFXUtility::CreateVortexForce(FVector Center, float Strength)
{
    FForge2ParticleForce Force;
    Force.Type = EForge2ForceType::Vortex;
    Force.Strength = Strength;
    Force.AttractorLocation = Center;
    return Force;
}
