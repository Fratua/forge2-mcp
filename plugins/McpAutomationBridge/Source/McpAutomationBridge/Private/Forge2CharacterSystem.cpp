// Forge2CharacterSystem.cpp
// Phase 14: Character Systems Implementation
// FORGE2 Ultimate AAA Game Development MCP Extension

#include "Forge2CharacterSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// ============================================================================
// CHARACTER CREATOR MANAGER IMPLEMENTATION
// ============================================================================

UForge2CharacterCreatorManager::UForge2CharacterCreatorManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UForge2CharacterCreatorManager::SetBodyMorphSettings(const FForge2BodyMorphSettings& Settings)
{
    CurrentAppearance.BodyMorphs = Settings;

    // Apply individual morphs
    SetBodyMorphValue(FName("Height"), Settings.Height);
    SetBodyMorphValue(FName("MuscleMass"), Settings.MuscleMass);
    SetBodyMorphValue(FName("BodyFat"), Settings.BodyFat);
    SetBodyMorphValue(FName("ShoulderWidth"), Settings.ShoulderWidth);
    SetBodyMorphValue(FName("HipWidth"), Settings.HipWidth);
    SetBodyMorphValue(FName("ChestSize"), Settings.ChestSize);
    SetBodyMorphValue(FName("WaistSize"), Settings.WaistSize);
    SetBodyMorphValue(FName("ArmLength"), Settings.ArmLength);
    SetBodyMorphValue(FName("LegLength"), Settings.LegLength);

    ApplyMorphTargets();
}

void UForge2CharacterCreatorManager::SetBodyMorphValue(FName MorphName, float Value)
{
    float ClampedValue = FMath::Clamp(Value, 0.0f, 1.0f);
    ActiveMorphTargets.Add(MorphName, ClampedValue);

    OnAppearanceChanged.Broadcast(MorphName, ClampedValue);
}

float UForge2CharacterCreatorManager::GetBodyMorphValue(FName MorphName) const
{
    if (const float* Value = ActiveMorphTargets.Find(MorphName))
    {
        return *Value;
    }
    return 0.5f;
}

void UForge2CharacterCreatorManager::ApplyBodyPreset(EForge2BodyType BodyType)
{
    FForge2BodyMorphSettings Preset;

    switch (BodyType)
    {
    case EForge2BodyType::Masculine:
        Preset = UForge2CharacterUtilities::GetMasculineBodyPreset();
        break;
    case EForge2BodyType::Feminine:
        Preset = UForge2CharacterUtilities::GetFeminineBodyPreset();
        break;
    case EForge2BodyType::Androgynous:
        Preset = UForge2CharacterUtilities::GetAndrogynousBodyPreset();
        break;
    default:
        return;
    }

    Preset.BodyType = BodyType;
    SetBodyMorphSettings(Preset);
}

void UForge2CharacterCreatorManager::RandomizeBodyMorphs()
{
    FForge2BodyMorphSettings RandomSettings;
    RandomSettings.Height = FMath::RandRange(0.2f, 0.8f);
    RandomSettings.MuscleMass = FMath::RandRange(0.1f, 0.9f);
    RandomSettings.BodyFat = FMath::RandRange(0.1f, 0.7f);
    RandomSettings.ShoulderWidth = FMath::RandRange(0.3f, 0.7f);
    RandomSettings.HipWidth = FMath::RandRange(0.3f, 0.7f);
    RandomSettings.ChestSize = FMath::RandRange(0.3f, 0.7f);
    RandomSettings.WaistSize = FMath::RandRange(0.3f, 0.7f);
    RandomSettings.ArmLength = FMath::RandRange(0.4f, 0.6f);
    RandomSettings.LegLength = FMath::RandRange(0.4f, 0.6f);
    RandomSettings.BodyType = EForge2BodyType::Custom;

    SetBodyMorphSettings(RandomSettings);
}

void UForge2CharacterCreatorManager::SetFaceSettings(const FForge2FaceSettings& Settings)
{
    CurrentAppearance.FaceSettings = Settings;
    ApplyMorphTargets();
}

void UForge2CharacterCreatorManager::SetFaceFeature(EForge2FaceFeature Feature, const FForge2FaceFeatureSetting& Setting)
{
    // Find or add feature
    bool bFound = false;
    for (FForge2FaceFeatureSetting& Existing : CurrentAppearance.FaceSettings.Features)
    {
        if (Existing.Feature == Feature)
        {
            Existing = Setting;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        CurrentAppearance.FaceSettings.Features.Add(Setting);
    }

    ApplyMorphTargets();
}

void UForge2CharacterCreatorManager::ApplyFacePreset(int32 PresetIndex)
{
    CurrentAppearance.FaceSettings.FaceShapePreset = PresetIndex;

    // Reset to default values based on preset
    CurrentAppearance.FaceSettings.Features.Empty();

    // Add default features
    TArray<EForge2FaceFeature> AllFeatures = {
        EForge2FaceFeature::Eyes,
        EForge2FaceFeature::Eyebrows,
        EForge2FaceFeature::Nose,
        EForge2FaceFeature::Mouth,
        EForge2FaceFeature::Ears,
        EForge2FaceFeature::Jaw,
        EForge2FaceFeature::Cheeks,
        EForge2FaceFeature::Chin,
        EForge2FaceFeature::Forehead
    };

    for (EForge2FaceFeature Feature : AllFeatures)
    {
        FForge2FaceFeatureSetting Setting;
        Setting.Feature = Feature;
        Setting.Size = 0.5f;
        Setting.Width = 0.5f;
        Setting.Height = 0.5f;
        Setting.VerticalPosition = 0.5f;
        Setting.HorizontalPosition = 0.5f;
        Setting.Depth = 0.5f;
        CurrentAppearance.FaceSettings.Features.Add(Setting);
    }

    ApplyMorphTargets();
}

void UForge2CharacterCreatorManager::RandomizeFace()
{
    CurrentAppearance.FaceSettings.FaceShapePreset = FMath::RandRange(0, 10);
    CurrentAppearance.FaceSettings.Features.Empty();

    TArray<EForge2FaceFeature> AllFeatures = {
        EForge2FaceFeature::Eyes,
        EForge2FaceFeature::Eyebrows,
        EForge2FaceFeature::Nose,
        EForge2FaceFeature::Mouth,
        EForge2FaceFeature::Ears,
        EForge2FaceFeature::Jaw,
        EForge2FaceFeature::Cheeks,
        EForge2FaceFeature::Chin,
        EForge2FaceFeature::Forehead
    };

    for (EForge2FaceFeature Feature : AllFeatures)
    {
        FForge2FaceFeatureSetting Setting;
        Setting.Feature = Feature;
        Setting.Size = FMath::RandRange(0.3f, 0.7f);
        Setting.Width = FMath::RandRange(0.3f, 0.7f);
        Setting.Height = FMath::RandRange(0.3f, 0.7f);
        Setting.VerticalPosition = FMath::RandRange(0.4f, 0.6f);
        Setting.HorizontalPosition = FMath::RandRange(0.4f, 0.6f);
        Setting.Depth = FMath::RandRange(0.4f, 0.6f);
        Setting.Rotation = FMath::RandRange(-0.1f, 0.1f);
        CurrentAppearance.FaceSettings.Features.Add(Setting);
    }

    CurrentAppearance.FaceSettings.AgeWrinkles = FMath::RandRange(0.0f, 0.3f);
    CurrentAppearance.FaceSettings.Freckles = FMath::RandRange(0.0f, 0.5f);

    ApplyMorphTargets();
}

void UForge2CharacterCreatorManager::SetHairSettings(const FForge2HairSettings& Settings)
{
    CurrentAppearance.HairSettings = Settings;
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::SetHairStyle(EForge2HairType Type, int32 StyleIndex)
{
    CurrentAppearance.HairSettings.HairType = Type;
    CurrentAppearance.HairSettings.HairStyleIndex = StyleIndex;
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::SetHairColor(const FLinearColor& Color)
{
    CurrentAppearance.HairSettings.HairColor = Color;
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::SetFacialHair(EForge2FacialHairType Type, int32 StyleIndex)
{
    CurrentAppearance.HairSettings.FacialHairType = Type;
    CurrentAppearance.HairSettings.FacialHairStyleIndex = StyleIndex;
    UpdateMeshComponents();
}

TArray<int32> UForge2CharacterCreatorManager::GetAvailableHairStyles(EForge2HairType Type) const
{
    TArray<int32> Styles;

    // Return available style indices based on hair type
    int32 MaxStyles = 0;
    switch (Type)
    {
    case EForge2HairType::Short: MaxStyles = 15; break;
    case EForge2HairType::Medium: MaxStyles = 12; break;
    case EForge2HairType::Long: MaxStyles = 10; break;
    case EForge2HairType::Bald: MaxStyles = 1; break;
    case EForge2HairType::Braided: MaxStyles = 8; break;
    case EForge2HairType::Mohawk: MaxStyles = 5; break;
    case EForge2HairType::Ponytail: MaxStyles = 6; break;
    case EForge2HairType::Afro: MaxStyles = 4; break;
    default: MaxStyles = 10; break;
    }

    for (int32 i = 0; i < MaxStyles; i++)
    {
        Styles.Add(i);
    }

    return Styles;
}

void UForge2CharacterCreatorManager::SetSkinSettings(const FForge2SkinSettings& Settings)
{
    CurrentAppearance.SkinSettings = Settings;
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::SetSkinTone(float Tone)
{
    CurrentAppearance.SkinSettings.SkinTone = FMath::Clamp(Tone, 0.0f, 1.0f);

    // Derive color from tone
    float R = FMath::Lerp(1.0f, 0.3f, Tone);
    float G = FMath::Lerp(0.9f, 0.2f, Tone);
    float B = FMath::Lerp(0.8f, 0.15f, Tone);

    CurrentAppearance.SkinSettings.SkinColor = FLinearColor(R, G, B);
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::AddScar(FName ScarId)
{
    CurrentAppearance.SkinSettings.ScarIds.AddUnique(ScarId);
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::RemoveScar(FName ScarId)
{
    CurrentAppearance.SkinSettings.ScarIds.Remove(ScarId);
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::AddTattoo(FName TattooId)
{
    CurrentAppearance.SkinSettings.TattooIds.AddUnique(TattooId);
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::RemoveTattoo(FName TattooId)
{
    CurrentAppearance.SkinSettings.TattooIds.Remove(TattooId);
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::EquipClothing(const FForge2ClothingItem& Item)
{
    // Remove existing item in slot
    CurrentAppearance.Clothing.RemoveAll([&Item](const FForge2ClothingItem& C)
    {
        return C.Slot == Item.Slot;
    });

    CurrentAppearance.Clothing.Add(Item);
    OnClothingChanged.Broadcast(Item.Slot, Item.ItemId);
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::UnequipClothing(EForge2ClothingSlot Slot)
{
    CurrentAppearance.Clothing.RemoveAll([Slot](const FForge2ClothingItem& C)
    {
        return C.Slot == Slot;
    });

    OnClothingChanged.Broadcast(Slot, NAME_None);
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::SetClothingColor(EForge2ClothingSlot Slot, const FLinearColor& Primary, const FLinearColor& Secondary)
{
    for (FForge2ClothingItem& Item : CurrentAppearance.Clothing)
    {
        if (Item.Slot == Slot)
        {
            Item.PrimaryColor = Primary;
            Item.SecondaryColor = Secondary;
            break;
        }
    }
    UpdateMeshComponents();
}

FForge2ClothingItem UForge2CharacterCreatorManager::GetEquippedClothing(EForge2ClothingSlot Slot) const
{
    for (const FForge2ClothingItem& Item : CurrentAppearance.Clothing)
    {
        if (Item.Slot == Slot)
        {
            return Item;
        }
    }
    return FForge2ClothingItem();
}

void UForge2CharacterCreatorManager::SetFullAppearance(const FForge2CharacterAppearance& Appearance)
{
    CurrentAppearance = Appearance;
    ApplyMorphTargets();
    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::SaveAppearancePreset(const FString& PresetName)
{
    SavedPresets.Add(PresetName, CurrentAppearance);
}

bool UForge2CharacterCreatorManager::LoadAppearancePreset(const FString& PresetName)
{
    if (const FForge2CharacterAppearance* Preset = SavedPresets.Find(PresetName))
    {
        SetFullAppearance(*Preset);
        return true;
    }
    return false;
}

void UForge2CharacterCreatorManager::RandomizeFullAppearance()
{
    RandomizeBodyMorphs();
    RandomizeFace();

    // Random hair
    CurrentAppearance.HairSettings.HairType = static_cast<EForge2HairType>(
        FMath::RandRange(0, static_cast<int32>(EForge2HairType::Custom) - 1));
    CurrentAppearance.HairSettings.HairStyleIndex = FMath::RandRange(0, 10);
    CurrentAppearance.HairSettings.HairColor = UForge2CharacterUtilities::GetRandomHairColor();

    // Random skin
    CurrentAppearance.SkinSettings.SkinTone = FMath::RandRange(0.0f, 1.0f);
    SetSkinTone(CurrentAppearance.SkinSettings.SkinTone);

    // Random eyes
    CurrentAppearance.EyeColor = UForge2CharacterUtilities::GetRandomEyeColor();
    CurrentAppearance.bHeterochromia = FMath::RandRange(0.0f, 1.0f) < 0.05f;
    if (CurrentAppearance.bHeterochromia)
    {
        CurrentAppearance.SecondEyeColor = UForge2CharacterUtilities::GetRandomEyeColor();
    }

    UpdateMeshComponents();
}

void UForge2CharacterCreatorManager::ApplyMorphTargets()
{
    // Apply morph targets to skeletal mesh component
    AActor* Owner = GetOwner();
    if (!Owner) return;

    USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!MeshComp) return;

    for (const auto& Pair : ActiveMorphTargets)
    {
        MeshComp->SetMorphTarget(Pair.Key, Pair.Value);
    }
}

void UForge2CharacterCreatorManager::UpdateMeshComponents()
{
    // Update hair, clothing, and material components
    // This would swap meshes and update material instances
}

// ============================================================================
// CLASS SYSTEM MANAGER IMPLEMENTATION
// ============================================================================

UForge2ClassSystemManager::UForge2ClassSystemManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UForge2ClassSystemManager::RegisterClass(const FForge2ClassDefinition& ClassDef)
{
    RegisteredClasses.Add(ClassDef.ClassId, ClassDef);
}

void UForge2ClassSystemManager::RegisterSubclass(const FForge2SubclassDefinition& SubclassDef)
{
    RegisteredSubclasses.Add(SubclassDef.SubclassId, SubclassDef);
}

bool UForge2ClassSystemManager::GetClassDefinition(FName ClassId, FForge2ClassDefinition& OutDef) const
{
    if (const FForge2ClassDefinition* Found = RegisteredClasses.Find(ClassId))
    {
        OutDef = *Found;
        return true;
    }
    return false;
}

bool UForge2ClassSystemManager::GetSubclassDefinition(FName SubclassId, FForge2SubclassDefinition& OutDef) const
{
    if (const FForge2SubclassDefinition* Found = RegisteredSubclasses.Find(SubclassId))
    {
        OutDef = *Found;
        return true;
    }
    return false;
}

TArray<FName> UForge2ClassSystemManager::GetAllClassIds() const
{
    TArray<FName> Ids;
    RegisteredClasses.GetKeys(Ids);
    return Ids;
}

bool UForge2ClassSystemManager::SetPlayerClass(FName ClassId)
{
    const FForge2ClassDefinition* ClassDef = RegisteredClasses.Find(ClassId);
    if (!ClassDef)
    {
        return false;
    }

    FName OldClass = CurrentClassId;
    CurrentClassId = ClassId;
    CurrentSubclassId = NAME_None;
    CurrentClassLevel = 1;
    CurrentClassXP = 0;

    RecalculateStats();
    OnClassChanged.Broadcast(OldClass, ClassId);

    return true;
}

bool UForge2ClassSystemManager::SetPlayerSubclass(FName SubclassId)
{
    const FForge2SubclassDefinition* SubclassDef = RegisteredSubclasses.Find(SubclassId);
    if (!SubclassDef)
    {
        return false;
    }

    // Check if valid for current class
    if (SubclassDef->ParentClassId != CurrentClassId)
    {
        return false;
    }

    // Check level requirement
    FForge2ClassDefinition ClassDef;
    if (GetClassDefinition(CurrentClassId, ClassDef))
    {
        if (CurrentClassLevel < ClassDef.SubclassUnlockLevel)
        {
            return false;
        }
    }

    CurrentSubclassId = SubclassId;
    RecalculateStats();
    OnSubclassUnlocked.Broadcast(SubclassId);

    return true;
}

void UForge2ClassSystemManager::AddClassExperience(int32 Amount)
{
    CurrentClassXP += Amount;

    // Check for level up
    int32 RequiredXP = UForge2CharacterUtilities::CalculateXPForLevel(CurrentClassLevel + 1);

    while (CurrentClassXP >= RequiredXP)
    {
        CurrentClassXP -= RequiredXP;
        CurrentClassLevel++;

        // Update multiclass if applicable
        if (bIsMulticlassed)
        {
            if (MulticlassConfig.ActiveClassId == MulticlassConfig.PrimaryClassId)
            {
                MulticlassConfig.PrimaryClassLevel = CurrentClassLevel;
            }
            else
            {
                MulticlassConfig.SecondaryClassLevel = CurrentClassLevel;
            }
        }

        RequiredXP = UForge2CharacterUtilities::CalculateXPForLevel(CurrentClassLevel + 1);
    }

    RecalculateStats();
}

bool UForge2ClassSystemManager::EnableMulticlass(FName SecondaryClassId)
{
    if (!CanMulticlassAs(SecondaryClassId))
    {
        return false;
    }

    const FForge2ClassDefinition* SecondaryClass = RegisteredClasses.Find(SecondaryClassId);
    if (!SecondaryClass)
    {
        return false;
    }

    bIsMulticlassed = true;
    MulticlassConfig.PrimaryClassId = CurrentClassId;
    MulticlassConfig.SecondaryClassId = SecondaryClassId;
    MulticlassConfig.PrimaryClassLevel = CurrentClassLevel;
    MulticlassConfig.SecondaryClassLevel = 1;
    MulticlassConfig.ActiveClassId = CurrentClassId;

    RecalculateStats();
    OnClassChanged.Broadcast(CurrentClassId, SecondaryClassId);

    return true;
}

void UForge2ClassSystemManager::DisableMulticlass()
{
    if (!bIsMulticlassed)
    {
        return;
    }

    bIsMulticlassed = false;
    CurrentClassId = MulticlassConfig.PrimaryClassId;
    CurrentClassLevel = MulticlassConfig.PrimaryClassLevel;
    MulticlassConfig = FForge2MulticlassConfig();

    RecalculateStats();
}

void UForge2ClassSystemManager::SwitchActiveClass()
{
    if (!bIsMulticlassed)
    {
        return;
    }

    FName OldActive = MulticlassConfig.ActiveClassId;

    if (MulticlassConfig.ActiveClassId == MulticlassConfig.PrimaryClassId)
    {
        MulticlassConfig.ActiveClassId = MulticlassConfig.SecondaryClassId;
        CurrentClassId = MulticlassConfig.SecondaryClassId;
        CurrentClassLevel = MulticlassConfig.SecondaryClassLevel;
    }
    else
    {
        MulticlassConfig.ActiveClassId = MulticlassConfig.PrimaryClassId;
        CurrentClassId = MulticlassConfig.PrimaryClassId;
        CurrentClassLevel = MulticlassConfig.PrimaryClassLevel;
    }

    RecalculateStats();
    OnClassChanged.Broadcast(OldActive, MulticlassConfig.ActiveClassId);
}

bool UForge2ClassSystemManager::CanMulticlassAs(FName ClassId) const
{
    if (ClassId == CurrentClassId)
    {
        return false;
    }

    const FForge2ClassDefinition* ClassDef = RegisteredClasses.Find(ClassId);
    if (!ClassDef || !ClassDef->bCanMulticlass)
    {
        return false;
    }

    // Check requirements
    for (const auto& Req : ClassDef->MulticlassRequirements)
    {
        // Would check player stats here
    }

    return true;
}

float UForge2ClassSystemManager::GetStatBonus(FName StatName) const
{
    if (const float* Bonus = CalculatedStatBonuses.Find(StatName))
    {
        return *Bonus;
    }
    return 0.0f;
}

TArray<FForge2StatBonus> UForge2ClassSystemManager::GetAllStatBonuses() const
{
    TArray<FForge2StatBonus> Bonuses;

    for (const auto& Pair : CalculatedStatBonuses)
    {
        FForge2StatBonus Bonus;
        Bonus.StatName = Pair.Key;
        Bonus.FlatBonus = Pair.Value;
        Bonuses.Add(Bonus);
    }

    return Bonuses;
}

EForge2ResourceType UForge2ClassSystemManager::GetResourceType() const
{
    if (CurrentSubclassId != NAME_None)
    {
        if (const FForge2SubclassDefinition* Subclass = RegisteredSubclasses.Find(CurrentSubclassId))
        {
            if (Subclass->bUseAlternateResource)
            {
                return Subclass->AlternateResource;
            }
        }
    }

    if (const FForge2ClassDefinition* ClassDef = RegisteredClasses.Find(CurrentClassId))
    {
        return ClassDef->ResourceType;
    }

    return EForge2ResourceType::Mana;
}

float UForge2ClassSystemManager::GetMaxResource() const
{
    if (const FForge2ClassDefinition* ClassDef = RegisteredClasses.Find(CurrentClassId))
    {
        return ClassDef->BaseResource + (ClassDef->ResourcePerLevel * CurrentClassLevel);
    }
    return 100.0f;
}

TArray<FName> UForge2ClassSystemManager::GetClassAbilities() const
{
    if (const FForge2ClassDefinition* ClassDef = RegisteredClasses.Find(CurrentClassId))
    {
        return ClassDef->StartingAbilities;
    }
    return TArray<FName>();
}

TArray<FName> UForge2ClassSystemManager::GetSubclassAbilities() const
{
    if (const FForge2SubclassDefinition* Subclass = RegisteredSubclasses.Find(CurrentSubclassId))
    {
        return Subclass->UniqueAbilities;
    }
    return TArray<FName>();
}

TArray<FName> UForge2ClassSystemManager::GetAllAvailableAbilities() const
{
    TArray<FName> Abilities;

    // Class abilities
    Abilities.Append(GetClassAbilities());

    // Subclass abilities
    Abilities.Append(GetSubclassAbilities());

    // Multiclass abilities
    if (bIsMulticlassed)
    {
        Abilities.Append(MulticlassConfig.SharedAbilities);

        // Secondary class abilities
        if (const FForge2ClassDefinition* SecondaryClass = RegisteredClasses.Find(MulticlassConfig.SecondaryClassId))
        {
            Abilities.Append(SecondaryClass->StartingAbilities);
        }
    }

    return Abilities;
}

void UForge2ClassSystemManager::RecalculateStats()
{
    CalculatedStatBonuses.Empty();

    // Base class bonuses
    if (const FForge2ClassDefinition* ClassDef = RegisteredClasses.Find(CurrentClassId))
    {
        for (const FForge2StatBonus& Bonus : ClassDef->BaseStats)
        {
            float Total = Bonus.FlatBonus + (Bonus.PerLevelBonus * CurrentClassLevel);
            CalculatedStatBonuses.Add(Bonus.StatName, Total);
        }
    }

    // Subclass bonuses
    if (const FForge2SubclassDefinition* Subclass = RegisteredSubclasses.Find(CurrentSubclassId))
    {
        for (const FForge2StatBonus& Bonus : Subclass->StatBonuses)
        {
            float Current = CalculatedStatBonuses.FindOrAdd(Bonus.StatName, 0.0f);
            CalculatedStatBonuses.Add(Bonus.StatName, Current + Bonus.FlatBonus);
        }
    }

    // Multiclass bonuses
    if (bIsMulticlassed)
    {
        if (const FForge2ClassDefinition* SecondaryClass = RegisteredClasses.Find(MulticlassConfig.SecondaryClassId))
        {
            for (const FForge2StatBonus& Bonus : SecondaryClass->BaseStats)
            {
                float Contribution = (Bonus.FlatBonus + (Bonus.PerLevelBonus * MulticlassConfig.SecondaryClassLevel)) * 0.5f;
                float Current = CalculatedStatBonuses.FindOrAdd(Bonus.StatName, 0.0f);
                CalculatedStatBonuses.Add(Bonus.StatName, Current + Contribution);
            }
        }
    }
}

void UForge2ClassSystemManager::ApplyClassBonuses()
{
    // Apply bonuses to player character
}

// ============================================================================
// MOUNT SYSTEM MANAGER IMPLEMENTATION
// ============================================================================

UForge2MountSystemManager::UForge2MountSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UForge2MountSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bMountSummoned)
    {
        UpdateStamina(DeltaTime);
        TrackDistance(DeltaTime);
    }
}

void UForge2MountSystemManager::RegisterMountDefinition(const FForge2MountDefinition& MountDef)
{
    RegisteredMounts.Add(MountDef.MountId, MountDef);
}

bool UForge2MountSystemManager::GetMountDefinition(FName MountId, FForge2MountDefinition& OutDef) const
{
    if (const FForge2MountDefinition* Found = RegisteredMounts.Find(MountId))
    {
        OutDef = *Found;
        return true;
    }
    return false;
}

TArray<FName> UForge2MountSystemManager::GetAllMountIds() const
{
    TArray<FName> Ids;
    RegisteredMounts.GetKeys(Ids);
    return Ids;
}

FGuid UForge2MountSystemManager::AddOwnedMount(FName MountId, const FString& CustomName)
{
    if (!RegisteredMounts.Contains(MountId))
    {
        return FGuid();
    }

    FForge2OwnedMount NewMount;
    NewMount.InstanceId = FGuid::NewGuid();
    NewMount.MountId = MountId;
    NewMount.CustomName = CustomName.IsEmpty() ? RegisteredMounts[MountId].DisplayName.ToString() : CustomName;
    NewMount.Level = 1;
    NewMount.Experience = 0;
    NewMount.BondLevel = 0.0f;

    OwnedMounts.Add(NewMount.InstanceId, NewMount);
    return NewMount.InstanceId;
}

void UForge2MountSystemManager::RemoveOwnedMount(FGuid InstanceId)
{
    if (SummonedMountId == InstanceId)
    {
        DismissMount();
    }
    OwnedMounts.Remove(InstanceId);
}

TArray<FForge2OwnedMount> UForge2MountSystemManager::GetAllOwnedMounts() const
{
    TArray<FForge2OwnedMount> Mounts;
    for (const auto& Pair : OwnedMounts)
    {
        Mounts.Add(Pair.Value);
    }
    return Mounts;
}

bool UForge2MountSystemManager::GetOwnedMount(FGuid InstanceId, FForge2OwnedMount& OutMount) const
{
    if (const FForge2OwnedMount* Found = OwnedMounts.Find(InstanceId))
    {
        OutMount = *Found;
        return true;
    }
    return false;
}

void UForge2MountSystemManager::SetMountFavorite(FGuid InstanceId, bool bFavorite)
{
    if (FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        Mount->bIsFavorite = bFavorite;
    }
}

void UForge2MountSystemManager::RenamMount(FGuid InstanceId, const FString& NewName)
{
    if (FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        Mount->CustomName = NewName;
    }
}

void UForge2MountSystemManager::SetMountCustomization(FGuid InstanceId, const FForge2MountCustomization& Customization)
{
    if (FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        Mount->Customization = Customization;

        // Update active mount if this is the summoned one
        if (bMountSummoned && SummonedMountId == InstanceId)
        {
            UpdateMountStats();
        }
    }
}

FForge2MountCustomization UForge2MountSystemManager::GetMountCustomization(FGuid InstanceId) const
{
    if (const FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        return Mount->Customization;
    }
    return FForge2MountCustomization();
}

void UForge2MountSystemManager::SetMountColors(FGuid InstanceId, const FLinearColor& Primary, const FLinearColor& Secondary, const FLinearColor& Accent)
{
    if (FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        Mount->Customization.PrimaryColor = Primary;
        Mount->Customization.SecondaryColor = Secondary;
        Mount->Customization.AccentColor = Accent;
    }
}

void UForge2MountSystemManager::EquipMountArmor(FGuid InstanceId, FName ArmorSetId)
{
    if (FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        Mount->Customization.ArmorSetId = ArmorSetId;
    }
}

bool UForge2MountSystemManager::SummonMount(FGuid InstanceId)
{
    const FForge2OwnedMount* OwnedMount = OwnedMounts.Find(InstanceId);
    if (!OwnedMount)
    {
        return false;
    }

    const FForge2MountDefinition* MountDef = RegisteredMounts.Find(OwnedMount->MountId);
    if (!MountDef)
    {
        return false;
    }

    // Dismiss current mount if any
    if (bMountSummoned)
    {
        DismissMount();
    }

    // Spawn mount actor
    if (MountDef->MountClass.IsValid())
    {
        UClass* MountClass = MountDef->MountClass.LoadSynchronous();
        if (MountClass)
        {
            AActor* Owner = GetOwner();
            if (Owner)
            {
                FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 200.0f;
                FRotator SpawnRotation = Owner->GetActorRotation();

                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                AActor* SpawnedMount = GetWorld()->SpawnActor<AActor>(MountClass, SpawnLocation, SpawnRotation, SpawnParams);
                if (SpawnedMount)
                {
                    SummonedMountActor = SpawnedMount;
                    SummonedMountId = InstanceId;
                    bMountSummoned = true;

                    // Initialize stats
                    CurrentHealth = MountDef->Stats.MaxHealth;
                    CurrentStamina = MountDef->Stats.MaxStamina;
                    LastPosition = SpawnLocation;

                    // Update summoned count
                    if (FForge2OwnedMount* MutableMount = OwnedMounts.Find(InstanceId))
                    {
                        MutableMount->TimesSummoned++;
                    }

                    OnMountSummoned.Broadcast(InstanceId, OwnedMount->MountId);
                    return true;
                }
            }
        }
    }

    return false;
}

void UForge2MountSystemManager::DismissMount()
{
    if (!bMountSummoned)
    {
        return;
    }

    if (bIsRiding)
    {
        Dismount();
    }

    FGuid OldMountId = SummonedMountId;

    if (SummonedMountActor.IsValid())
    {
        SummonedMountActor->Destroy();
    }

    bMountSummoned = false;
    SummonedMountId = FGuid();
    SummonedMountActor = nullptr;

    OnMountDismissed.Broadcast(OldMountId);
}

bool UForge2MountSystemManager::SummonRandomMount()
{
    if (OwnedMounts.Num() == 0)
    {
        return false;
    }

    TArray<FGuid> MountIds;
    OwnedMounts.GetKeys(MountIds);

    int32 RandomIndex = FMath::RandRange(0, MountIds.Num() - 1);
    return SummonMount(MountIds[RandomIndex]);
}

bool UForge2MountSystemManager::SummonFavoriteMount()
{
    for (const auto& Pair : OwnedMounts)
    {
        if (Pair.Value.bIsFavorite)
        {
            return SummonMount(Pair.Key);
        }
    }

    // No favorite, summon first available
    if (OwnedMounts.Num() > 0)
    {
        TArray<FGuid> MountIds;
        OwnedMounts.GetKeys(MountIds);
        return SummonMount(MountIds[0]);
    }

    return false;
}

void UForge2MountSystemManager::MountUp()
{
    if (!bMountSummoned || bIsRiding)
    {
        return;
    }

    bIsRiding = true;

    // Attach player to mount
    AActor* Owner = GetOwner();
    if (Owner && SummonedMountActor.IsValid())
    {
        // Would handle attachment and animation here
    }
}

void UForge2MountSystemManager::Dismount()
{
    if (!bIsRiding)
    {
        return;
    }

    bIsRiding = false;
    bIsSprinting = false;

    // Detach player from mount
}

void UForge2MountSystemManager::UseMountAbility(FName AbilityId)
{
    if (!bMountSummoned)
    {
        return;
    }

    const FForge2OwnedMount* OwnedMount = OwnedMounts.Find(SummonedMountId);
    if (!OwnedMount)
    {
        return;
    }

    const FForge2MountDefinition* MountDef = RegisteredMounts.Find(OwnedMount->MountId);
    if (!MountDef)
    {
        return;
    }

    // Find and use ability
    for (const FForge2MountAbility& Ability : MountDef->Abilities)
    {
        if (Ability.AbilityId == AbilityId)
        {
            // Execute ability
            break;
        }
    }
}

void UForge2MountSystemManager::SetMountSprinting(bool bSprinting)
{
    if (!bIsRiding)
    {
        return;
    }

    bIsSprinting = bSprinting;
}

void UForge2MountSystemManager::AddMountExperience(FGuid InstanceId, int32 Amount)
{
    FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId);
    if (!Mount)
    {
        return;
    }

    Mount->Experience += Amount;

    // Check level up
    int32 RequiredXP = CalculateXPForLevel(Mount->Level + 1);
    while (Mount->Experience >= RequiredXP)
    {
        Mount->Experience -= RequiredXP;
        Mount->Level++;

        OnMountLevelUp.Broadcast(InstanceId, Mount->Level);
        RequiredXP = CalculateXPForLevel(Mount->Level + 1);
    }
}

void UForge2MountSystemManager::IncreaseBondLevel(FGuid InstanceId, float Amount)
{
    if (FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        Mount->BondLevel = FMath::Clamp(Mount->BondLevel + Amount, 0.0f, 100.0f);
    }
}

float UForge2MountSystemManager::GetBondLevel(FGuid InstanceId) const
{
    if (const FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        return Mount->BondLevel;
    }
    return 0.0f;
}

int32 UForge2MountSystemManager::GetMountLevel(FGuid InstanceId) const
{
    if (const FForge2OwnedMount* Mount = OwnedMounts.Find(InstanceId))
    {
        return Mount->Level;
    }
    return 1;
}

void UForge2MountSystemManager::UpdateMountStats()
{
    // Update mount stats based on level, bond, and equipment
}

void UForge2MountSystemManager::UpdateStamina(float DeltaTime)
{
    const FForge2OwnedMount* OwnedMount = OwnedMounts.Find(SummonedMountId);
    if (!OwnedMount)
    {
        return;
    }

    const FForge2MountDefinition* MountDef = RegisteredMounts.Find(OwnedMount->MountId);
    if (!MountDef)
    {
        return;
    }

    if (bIsSprinting)
    {
        CurrentStamina -= MountDef->Stats.SprintStaminaCost * DeltaTime;
        CurrentStamina = FMath::Max(0.0f, CurrentStamina);

        if (CurrentStamina <= 0.0f)
        {
            bIsSprinting = false;
        }
    }
    else
    {
        CurrentStamina += MountDef->Stats.StaminaRegen * DeltaTime;
        CurrentStamina = FMath::Min(CurrentStamina, MountDef->Stats.MaxStamina);
    }
}

void UForge2MountSystemManager::TrackDistance(float DeltaTime)
{
    if (!SummonedMountActor.IsValid())
    {
        return;
    }

    FVector CurrentPosition = SummonedMountActor->GetActorLocation();
    float Distance = FVector::Dist(LastPosition, CurrentPosition);

    if (FForge2OwnedMount* Mount = OwnedMounts.Find(SummonedMountId))
    {
        Mount->DistanceTraveled += Distance;
    }

    LastPosition = CurrentPosition;

    // Award XP for distance traveled
    if (Distance > 100.0f)
    {
        AddMountExperience(SummonedMountId, FMath::FloorToInt(Distance / 1000.0f));
    }
}

int32 UForge2MountSystemManager::CalculateXPForLevel(int32 Level) const
{
    return UForge2CharacterUtilities::CalculateXPForLevel(Level, 50, 1.3f);
}

// ============================================================================
// CHARACTER UTILITIES IMPLEMENTATION
// ============================================================================

FForge2BodyMorphSettings UForge2CharacterUtilities::GetMasculineBodyPreset()
{
    FForge2BodyMorphSettings Settings;
    Settings.Height = 0.6f;
    Settings.MuscleMass = 0.6f;
    Settings.BodyFat = 0.3f;
    Settings.ShoulderWidth = 0.7f;
    Settings.HipWidth = 0.4f;
    Settings.ChestSize = 0.5f;
    Settings.WaistSize = 0.45f;
    Settings.ArmLength = 0.5f;
    Settings.LegLength = 0.5f;
    Settings.BodyType = EForge2BodyType::Masculine;
    return Settings;
}

FForge2BodyMorphSettings UForge2CharacterUtilities::GetFeminineBodyPreset()
{
    FForge2BodyMorphSettings Settings;
    Settings.Height = 0.4f;
    Settings.MuscleMass = 0.3f;
    Settings.BodyFat = 0.35f;
    Settings.ShoulderWidth = 0.4f;
    Settings.HipWidth = 0.6f;
    Settings.ChestSize = 0.6f;
    Settings.WaistSize = 0.35f;
    Settings.ArmLength = 0.5f;
    Settings.LegLength = 0.5f;
    Settings.BodyType = EForge2BodyType::Feminine;
    return Settings;
}

FForge2BodyMorphSettings UForge2CharacterUtilities::GetAndrogynousBodyPreset()
{
    FForge2BodyMorphSettings Settings;
    Settings.Height = 0.5f;
    Settings.MuscleMass = 0.4f;
    Settings.BodyFat = 0.3f;
    Settings.ShoulderWidth = 0.5f;
    Settings.HipWidth = 0.5f;
    Settings.ChestSize = 0.4f;
    Settings.WaistSize = 0.4f;
    Settings.ArmLength = 0.5f;
    Settings.LegLength = 0.5f;
    Settings.BodyType = EForge2BodyType::Androgynous;
    return Settings;
}

FLinearColor UForge2CharacterUtilities::GetRandomSkinTone()
{
    float Tone = FMath::RandRange(0.0f, 1.0f);
    float R = FMath::Lerp(1.0f, 0.3f, Tone);
    float G = FMath::Lerp(0.9f, 0.2f, Tone);
    float B = FMath::Lerp(0.8f, 0.15f, Tone);
    return FLinearColor(R, G, B);
}

FLinearColor UForge2CharacterUtilities::GetRandomHairColor()
{
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.1f, 0.08f, 0.06f),  // Black
        FLinearColor(0.3f, 0.2f, 0.1f),    // Dark brown
        FLinearColor(0.5f, 0.35f, 0.2f),   // Brown
        FLinearColor(0.7f, 0.5f, 0.3f),    // Light brown
        FLinearColor(0.9f, 0.7f, 0.4f),    // Blonde
        FLinearColor(0.95f, 0.85f, 0.6f),  // Platinum
        FLinearColor(0.6f, 0.3f, 0.1f),    // Auburn
        FLinearColor(0.8f, 0.4f, 0.2f),    // Red
        FLinearColor(0.5f, 0.5f, 0.5f),    // Grey
        FLinearColor(1.0f, 1.0f, 1.0f)     // White
    };

    return HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
}

FLinearColor UForge2CharacterUtilities::GetRandomEyeColor()
{
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.3f, 0.2f, 0.1f),    // Brown
        FLinearColor(0.5f, 0.35f, 0.2f),   // Hazel
        FLinearColor(0.2f, 0.4f, 0.6f),    // Blue
        FLinearColor(0.3f, 0.5f, 0.3f),    // Green
        FLinearColor(0.4f, 0.4f, 0.4f),    // Grey
        FLinearColor(0.5f, 0.3f, 0.2f),    // Amber
    };

    return EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
}

FForge2ClassDefinition UForge2CharacterUtilities::GetWarriorClass()
{
    FForge2ClassDefinition ClassDef;
    ClassDef.ClassId = FName("Warrior");
    ClassDef.DisplayName = FText::FromString("Warrior");
    ClassDef.Description = FText::FromString("A master of weapons and armor, excelling in melee combat.");
    ClassDef.PrimaryRole = EForge2ClassRole::Tank;
    ClassDef.SecondaryRoles = { EForge2ClassRole::DPS_Melee };
    ClassDef.ResourceType = EForge2ResourceType::Rage;
    ClassDef.BaseResource = 0.0f;
    ClassDef.ResourcePerLevel = 0.0f;
    ClassDef.ArmorProficiency = { EForge2ArmorType::Plate, EForge2ArmorType::Mail, EForge2ArmorType::Leather };
    ClassDef.WeaponProficiency = { EForge2WeaponCategory::Sword, EForge2WeaponCategory::Axe, EForge2WeaponCategory::Mace, EForge2WeaponCategory::Shield, EForge2WeaponCategory::Polearm };

    FForge2StatBonus StrBonus;
    StrBonus.StatName = FName("Strength");
    StrBonus.FlatBonus = 10.0f;
    StrBonus.PerLevelBonus = 2.0f;
    ClassDef.BaseStats.Add(StrBonus);

    FForge2StatBonus ConBonus;
    ConBonus.StatName = FName("Constitution");
    ConBonus.FlatBonus = 8.0f;
    ConBonus.PerLevelBonus = 1.5f;
    ClassDef.BaseStats.Add(ConBonus);

    ClassDef.StartingAbilities = { FName("Slash"), FName("Shield Block"), FName("Taunt") };
    ClassDef.AvailableSubclasses = { FName("Berserker"), FName("Paladin"), FName("Champion") };
    ClassDef.SubclassUnlockLevel = 10;

    return ClassDef;
}

FForge2ClassDefinition UForge2CharacterUtilities::GetMageClass()
{
    FForge2ClassDefinition ClassDef;
    ClassDef.ClassId = FName("Mage");
    ClassDef.DisplayName = FText::FromString("Mage");
    ClassDef.Description = FText::FromString("A wielder of arcane power, casting devastating spells from afar.");
    ClassDef.PrimaryRole = EForge2ClassRole::DPS_Ranged;
    ClassDef.SecondaryRoles = { EForge2ClassRole::Support };
    ClassDef.ResourceType = EForge2ResourceType::Mana;
    ClassDef.BaseResource = 100.0f;
    ClassDef.ResourcePerLevel = 10.0f;
    ClassDef.ArmorProficiency = { EForge2ArmorType::Cloth };
    ClassDef.WeaponProficiency = { EForge2WeaponCategory::Staff, EForge2WeaponCategory::Wand };

    FForge2StatBonus IntBonus;
    IntBonus.StatName = FName("Intelligence");
    IntBonus.FlatBonus = 12.0f;
    IntBonus.PerLevelBonus = 2.5f;
    ClassDef.BaseStats.Add(IntBonus);

    ClassDef.StartingAbilities = { FName("Fireball"), FName("Frost Nova"), FName("Arcane Missile") };
    ClassDef.AvailableSubclasses = { FName("Pyromancer"), FName("Cryomancer"), FName("Arcanist") };
    ClassDef.SubclassUnlockLevel = 10;

    return ClassDef;
}

FForge2ClassDefinition UForge2CharacterUtilities::GetRogueClass()
{
    FForge2ClassDefinition ClassDef;
    ClassDef.ClassId = FName("Rogue");
    ClassDef.DisplayName = FText::FromString("Rogue");
    ClassDef.Description = FText::FromString("A shadowy assassin who strikes from the darkness.");
    ClassDef.PrimaryRole = EForge2ClassRole::DPS_Melee;
    ClassDef.ResourceType = EForge2ResourceType::Energy;
    ClassDef.BaseResource = 100.0f;
    ClassDef.ResourcePerLevel = 0.0f;
    ClassDef.ArmorProficiency = { EForge2ArmorType::Leather };
    ClassDef.WeaponProficiency = { EForge2WeaponCategory::Dagger, EForge2WeaponCategory::Sword };

    FForge2StatBonus DexBonus;
    DexBonus.StatName = FName("Dexterity");
    DexBonus.FlatBonus = 12.0f;
    DexBonus.PerLevelBonus = 2.5f;
    ClassDef.BaseStats.Add(DexBonus);

    ClassDef.StartingAbilities = { FName("Backstab"), FName("Stealth"), FName("Evade") };
    ClassDef.AvailableSubclasses = { FName("Assassin"), FName("Shadowdancer"), FName("Trickster") };
    ClassDef.SubclassUnlockLevel = 10;

    return ClassDef;
}

FForge2ClassDefinition UForge2CharacterUtilities::GetClericClass()
{
    FForge2ClassDefinition ClassDef;
    ClassDef.ClassId = FName("Cleric");
    ClassDef.DisplayName = FText::FromString("Cleric");
    ClassDef.Description = FText::FromString("A holy warrior who heals allies and smites enemies.");
    ClassDef.PrimaryRole = EForge2ClassRole::Healer;
    ClassDef.SecondaryRoles = { EForge2ClassRole::Support };
    ClassDef.ResourceType = EForge2ResourceType::Mana;
    ClassDef.BaseResource = 80.0f;
    ClassDef.ResourcePerLevel = 8.0f;
    ClassDef.ArmorProficiency = { EForge2ArmorType::Mail, EForge2ArmorType::Cloth };
    ClassDef.WeaponProficiency = { EForge2WeaponCategory::Mace, EForge2WeaponCategory::Staff, EForge2WeaponCategory::Shield };

    FForge2StatBonus WisBonus;
    WisBonus.StatName = FName("Wisdom");
    WisBonus.FlatBonus = 12.0f;
    WisBonus.PerLevelBonus = 2.0f;
    ClassDef.BaseStats.Add(WisBonus);

    ClassDef.StartingAbilities = { FName("Heal"), FName("Smite"), FName("Bless") };
    ClassDef.AvailableSubclasses = { FName("Priest"), FName("Templar"), FName("Oracle") };
    ClassDef.SubclassUnlockLevel = 10;

    return ClassDef;
}

FForge2ClassDefinition UForge2CharacterUtilities::GetRangerClass()
{
    FForge2ClassDefinition ClassDef;
    ClassDef.ClassId = FName("Ranger");
    ClassDef.DisplayName = FText::FromString("Ranger");
    ClassDef.Description = FText::FromString("A master of the bow and wild companion.");
    ClassDef.PrimaryRole = EForge2ClassRole::DPS_Ranged;
    ClassDef.ResourceType = EForge2ResourceType::Focus;
    ClassDef.BaseResource = 100.0f;
    ClassDef.ResourcePerLevel = 5.0f;
    ClassDef.ArmorProficiency = { EForge2ArmorType::Leather, EForge2ArmorType::Mail };
    ClassDef.WeaponProficiency = { EForge2WeaponCategory::Bow, EForge2WeaponCategory::Crossbow, EForge2WeaponCategory::Dagger };

    FForge2StatBonus DexBonus;
    DexBonus.StatName = FName("Dexterity");
    DexBonus.FlatBonus = 10.0f;
    DexBonus.PerLevelBonus = 2.0f;
    ClassDef.BaseStats.Add(DexBonus);

    ClassDef.StartingAbilities = { FName("Aimed Shot"), FName("Trap"), FName("Call Pet") };
    ClassDef.AvailableSubclasses = { FName("Beastmaster"), FName("Sharpshooter"), FName("Warden") };
    ClassDef.SubclassUnlockLevel = 10;

    return ClassDef;
}

FForge2SubclassDefinition UForge2CharacterUtilities::GetBerserkerSubclass()
{
    FForge2SubclassDefinition SubclassDef;
    SubclassDef.SubclassId = FName("Berserker");
    SubclassDef.ParentClassId = FName("Warrior");
    SubclassDef.DisplayName = FText::FromString("Berserker");
    SubclassDef.Description = FText::FromString("A warrior consumed by battle fury, trading defense for offense.");
    SubclassDef.RoleModifier = EForge2ClassRole::DPS_Melee;

    FForge2StatBonus StrBonus;
    StrBonus.StatName = FName("Strength");
    StrBonus.FlatBonus = 5.0f;
    SubclassDef.StatBonuses.Add(StrBonus);

    SubclassDef.UniqueAbilities = { FName("Reckless Assault"), FName("Bloodlust"), FName("Frenzy") };
    SubclassDef.PassiveEffects = { FName("Enrage") };

    return SubclassDef;
}

FForge2SubclassDefinition UForge2CharacterUtilities::GetPaladinSubclass()
{
    FForge2SubclassDefinition SubclassDef;
    SubclassDef.SubclassId = FName("Paladin");
    SubclassDef.ParentClassId = FName("Warrior");
    SubclassDef.DisplayName = FText::FromString("Paladin");
    SubclassDef.Description = FText::FromString("A holy warrior who protects allies with divine power.");
    SubclassDef.RoleModifier = EForge2ClassRole::Tank;
    SubclassDef.bUseAlternateResource = true;
    SubclassDef.AlternateResource = EForge2ResourceType::Mana;

    FForge2StatBonus ConBonus;
    ConBonus.StatName = FName("Constitution");
    ConBonus.FlatBonus = 5.0f;
    SubclassDef.StatBonuses.Add(ConBonus);

    SubclassDef.UniqueAbilities = { FName("Divine Shield"), FName("Lay on Hands"), FName("Holy Strike") };
    SubclassDef.PassiveEffects = { FName("Aura of Protection") };

    return SubclassDef;
}

FForge2SubclassDefinition UForge2CharacterUtilities::GetPyromancerSubclass()
{
    FForge2SubclassDefinition SubclassDef;
    SubclassDef.SubclassId = FName("Pyromancer");
    SubclassDef.ParentClassId = FName("Mage");
    SubclassDef.DisplayName = FText::FromString("Pyromancer");
    SubclassDef.Description = FText::FromString("A mage specializing in fire magic.");
    SubclassDef.RoleModifier = EForge2ClassRole::DPS_Ranged;

    FForge2StatBonus IntBonus;
    IntBonus.StatName = FName("Intelligence");
    IntBonus.FlatBonus = 5.0f;
    SubclassDef.StatBonuses.Add(IntBonus);

    SubclassDef.UniqueAbilities = { FName("Meteor Strike"), FName("Fire Wall"), FName("Combustion") };
    SubclassDef.PassiveEffects = { FName("Burning Aura") };

    return SubclassDef;
}

FForge2SubclassDefinition UForge2CharacterUtilities::GetNecromancerSubclass()
{
    FForge2SubclassDefinition SubclassDef;
    SubclassDef.SubclassId = FName("Necromancer");
    SubclassDef.ParentClassId = FName("Mage");
    SubclassDef.DisplayName = FText::FromString("Necromancer");
    SubclassDef.Description = FText::FromString("A dark mage who commands the undead.");
    SubclassDef.RoleModifier = EForge2ClassRole::Support;
    SubclassDef.bUseAlternateResource = true;
    SubclassDef.AlternateResource = EForge2ResourceType::Essence;

    SubclassDef.UniqueAbilities = { FName("Raise Dead"), FName("Life Drain"), FName("Bone Armor") };
    SubclassDef.PassiveEffects = { FName("Undead Mastery") };

    return SubclassDef;
}

FForge2MountDefinition UForge2CharacterUtilities::GetWarhorseMount()
{
    FForge2MountDefinition MountDef;
    MountDef.MountId = FName("Warhorse");
    MountDef.DisplayName = FText::FromString("Warhorse");
    MountDef.Description = FText::FromString("A sturdy battle-trained horse.");
    MountDef.MountType = EForge2MountType::Ground;
    MountDef.Rarity = EForge2MountRarity::Common;

    MountDef.Stats.BaseSpeed = 700.0f;
    MountDef.Stats.SprintMultiplier = 1.4f;
    MountDef.Stats.JumpHeight = 300.0f;
    MountDef.Stats.MaxHealth = 500.0f;
    MountDef.Stats.Armor = 100.0f;
    MountDef.Stats.MaxStamina = 100.0f;
    MountDef.Stats.StaminaRegen = 15.0f;
    MountDef.Stats.SprintStaminaCost = 20.0f;

    MountDef.bCanAttackWhileMounted = true;
    MountDef.SummonTime = 2.0f;

    FForge2MountAbility ChargeAbility;
    ChargeAbility.AbilityId = FName("Charge");
    ChargeAbility.DisplayName = FText::FromString("Charge");
    ChargeAbility.Cooldown = 30.0f;
    MountDef.Abilities.Add(ChargeAbility);

    return MountDef;
}

FForge2MountDefinition UForge2CharacterUtilities::GetGriffonMount()
{
    FForge2MountDefinition MountDef;
    MountDef.MountId = FName("Griffon");
    MountDef.DisplayName = FText::FromString("Griffon");
    MountDef.Description = FText::FromString("A majestic creature with the body of a lion and wings of an eagle.");
    MountDef.MountType = EForge2MountType::Flying;
    MountDef.Rarity = EForge2MountRarity::Epic;

    MountDef.Stats.BaseSpeed = 600.0f;
    MountDef.Stats.SprintMultiplier = 1.5f;
    MountDef.Stats.bCanFly = true;
    MountDef.Stats.FlySpeed = 900.0f;
    MountDef.Stats.MaxAltitude = 8000.0f;
    MountDef.Stats.MaxHealth = 800.0f;
    MountDef.Stats.Armor = 150.0f;
    MountDef.Stats.MaxStamina = 150.0f;

    MountDef.RequiredLevel = 30;
    MountDef.SummonTime = 3.0f;

    FForge2MountAbility DiveAbility;
    DiveAbility.AbilityId = FName("Dive");
    DiveAbility.DisplayName = FText::FromString("Aerial Dive");
    DiveAbility.Cooldown = 45.0f;
    MountDef.Abilities.Add(DiveAbility);

    return MountDef;
}

FForge2MountDefinition UForge2CharacterUtilities::GetDragonMount()
{
    FForge2MountDefinition MountDef;
    MountDef.MountId = FName("Dragon");
    MountDef.DisplayName = FText::FromString("Dragon");
    MountDef.Description = FText::FromString("An ancient dragon of immense power.");
    MountDef.MountType = EForge2MountType::Flying;
    MountDef.Rarity = EForge2MountRarity::Legendary;

    MountDef.Stats.BaseSpeed = 800.0f;
    MountDef.Stats.SprintMultiplier = 1.6f;
    MountDef.Stats.bCanFly = true;
    MountDef.Stats.FlySpeed = 1200.0f;
    MountDef.Stats.MaxAltitude = 15000.0f;
    MountDef.Stats.MaxHealth = 2000.0f;
    MountDef.Stats.Armor = 300.0f;
    MountDef.Stats.MaxStamina = 200.0f;

    MountDef.RequiredLevel = 50;
    MountDef.SummonTime = 5.0f;
    MountDef.PassengerCount = 2;

    FForge2MountAbility FireBreath;
    FireBreath.AbilityId = FName("FireBreath");
    FireBreath.DisplayName = FText::FromString("Fire Breath");
    FireBreath.Cooldown = 60.0f;
    MountDef.Abilities.Add(FireBreath);

    MountDef.MountAttacks = { FName("Claw Swipe"), FName("Tail Sweep") };

    return MountDef;
}

FForge2MountDefinition UForge2CharacterUtilities::GetMechanicalMount()
{
    FForge2MountDefinition MountDef;
    MountDef.MountId = FName("MechaSteed");
    MountDef.DisplayName = FText::FromString("Mechanical Steed");
    MountDef.Description = FText::FromString("A clockwork marvel of engineering.");
    MountDef.MountType = EForge2MountType::Mechanical;
    MountDef.Rarity = EForge2MountRarity::Rare;

    MountDef.Stats.BaseSpeed = 750.0f;
    MountDef.Stats.SprintMultiplier = 1.7f;
    MountDef.Stats.JumpHeight = 400.0f;
    MountDef.Stats.MaxHealth = 600.0f;
    MountDef.Stats.Armor = 200.0f;
    MountDef.Stats.MaxStamina = 120.0f;
    MountDef.Stats.StaminaRegen = 20.0f;

    MountDef.bHasInventory = true;
    MountDef.InventorySlots = 20;
    MountDef.SummonTime = 1.0f;

    FForge2MountAbility TurboBoost;
    TurboBoost.AbilityId = FName("TurboBoost");
    TurboBoost.DisplayName = FText::FromString("Turbo Boost");
    TurboBoost.Cooldown = 20.0f;
    MountDef.Abilities.Add(TurboBoost);

    return MountDef;
}

float UForge2CharacterUtilities::CalculateHeightFromMorph(float HeightMorphValue, float MinHeight, float MaxHeight)
{
    return FMath::Lerp(MinHeight, MaxHeight, HeightMorphValue);
}

int32 UForge2CharacterUtilities::CalculateLevelFromXP(int32 TotalXP, float XPCurveExponent)
{
    int32 Level = 1;
    int32 AccumulatedXP = 0;

    while (AccumulatedXP <= TotalXP)
    {
        AccumulatedXP += CalculateXPForLevel(Level, 100, XPCurveExponent);
        Level++;
    }

    return Level - 1;
}

int32 UForge2CharacterUtilities::CalculateXPForLevel(int32 Level, int32 BaseXP, float Exponent)
{
    return FMath::FloorToInt(BaseXP * FMath::Pow(Level, Exponent));
}

float UForge2CharacterUtilities::CalculateBondBonus(float BondLevel)
{
    // 0-100 bond level gives 0-20% bonus
    return BondLevel * 0.002f;
}
