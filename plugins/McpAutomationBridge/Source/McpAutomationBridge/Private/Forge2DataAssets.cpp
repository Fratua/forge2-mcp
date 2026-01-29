// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Complete Data Asset Implementations

#include "Forge2DataAssets.h"
#include "Misc/DataValidation.h"

// ============================================================================
// Color Definitions for Rarity
// ============================================================================

namespace Forge2RarityColors
{
    const FLinearColor Common = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);      // Gray
    const FLinearColor Uncommon = FLinearColor(0.12f, 0.8f, 0.12f, 1.0f);  // Green
    const FLinearColor Rare = FLinearColor(0.0f, 0.44f, 0.87f, 1.0f);      // Blue
    const FLinearColor Epic = FLinearColor(0.64f, 0.21f, 0.93f, 1.0f);     // Purple
    const FLinearColor Legendary = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);   // Orange
    const FLinearColor Mythic = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);      // Red
    const FLinearColor Unique = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);      // Cyan
}

// ============================================================================
// Base Data Asset Implementation
// ============================================================================

UMcpDataAssetBase::UMcpDataAssetBase()
{
    AssetId = NAME_None;
    DisplayName = FText::GetEmpty();
    Description = FText::GetEmpty();
    DataVersion = 1;
}

FPrimaryAssetId UMcpDataAssetBase::GetPrimaryAssetId() const
{
    // Use the asset type and asset ID to create a unique primary asset ID
    FName AssetType = GetAssetType();
    if (AssetType.IsNone())
    {
        AssetType = FName(TEXT("McpDataAsset"));
    }

    FName IdToUse = AssetId.IsNone() ? GetFName() : AssetId;
    return FPrimaryAssetId(AssetType, IdToUse);
}

bool UMcpDataAssetBase::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = true;

    if (AssetId.IsNone())
    {
        OutErrors.Add(FText::FromString(TEXT("Asset ID is not set")));
        bValid = false;
    }

    if (DisplayName.IsEmpty())
    {
        OutErrors.Add(FText::FromString(TEXT("Display name is not set")));
        bValid = false;
    }

    return bValid;
}

#if WITH_EDITOR
void UMcpDataAssetBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Auto-generate asset ID from name if not set
    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMcpDataAssetBase, DisplayName))
    {
        if (AssetId.IsNone() && !DisplayName.IsEmpty())
        {
            FString IdString = DisplayName.ToString();
            IdString = IdString.Replace(TEXT(" "), TEXT("_"));
            IdString = IdString.Replace(TEXT("'"), TEXT(""));
            AssetId = FName(*IdString);
        }
    }
}

EDataValidationResult UMcpDataAssetBase::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

    TArray<FText> Errors;
    if (!ValidateAsset(Errors))
    {
        for (const FText& Error : Errors)
        {
            Context.AddError(Error);
        }
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}
#endif

// ============================================================================
// Biome Definition Asset Implementation
// ============================================================================

UMcpBiomeDefinitionAsset::UMcpBiomeDefinitionAsset()
{
    BiomeType = EForge2BiomeType::Forest;
    TemperatureRange = FVector2D(10.0f, 25.0f);
    MoistureRange = FVector2D(30.0f, 70.0f);
    ElevationRange = FVector2D(0.0f, 2000.0f);
    FogColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
    FogDensity = 0.02f;
    BlendWidth = 1000.0f;
}

bool UMcpBiomeDefinitionAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (TemperatureRange.X > TemperatureRange.Y)
    {
        OutErrors.Add(FText::FromString(TEXT("Temperature range min is greater than max")));
        bValid = false;
    }

    if (MoistureRange.X > MoistureRange.Y)
    {
        OutErrors.Add(FText::FromString(TEXT("Moisture range min is greater than max")));
        bValid = false;
    }

    if (ElevationRange.X > ElevationRange.Y)
    {
        OutErrors.Add(FText::FromString(TEXT("Elevation range min is greater than max")));
        bValid = false;
    }

    if (!TerrainMaterial.IsNull())
    {
        // Valid terrain material reference
    }
    else
    {
        OutErrors.Add(FText::FromString(TEXT("Terrain material is not set")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Item Definition Asset Implementation
// ============================================================================

UMcpItemDefinitionAsset::UMcpItemDefinitionAsset()
{
    ItemType = EForge2ItemType::Misc;
    Rarity = EForge2Rarity::Common;
    MaxStackSize = 1;
    Weight = 0.0f;
    BaseValue = 0;
    LevelRequirement = 0;
    BoundType = EForge2BoundType::None;
    bCanBeTraded = true;
    bCanBeDestroyed = true;
    RarityGlowColor = FLinearColor::White;
    SocketCount = 0;
}

int64 UMcpItemDefinitionAsset::GetSellValue() const
{
    // Calculate sell value based on rarity multiplier
    float RarityMultiplier = 1.0f;
    switch (Rarity)
    {
        case EForge2Rarity::Common:    RarityMultiplier = 1.0f; break;
        case EForge2Rarity::Uncommon:  RarityMultiplier = 1.5f; break;
        case EForge2Rarity::Rare:      RarityMultiplier = 2.5f; break;
        case EForge2Rarity::Epic:      RarityMultiplier = 4.0f; break;
        case EForge2Rarity::Legendary: RarityMultiplier = 8.0f; break;
        case EForge2Rarity::Mythic:    RarityMultiplier = 16.0f; break;
        case EForge2Rarity::Unique:    RarityMultiplier = 10.0f; break;
    }

    // Sell value is typically 25% of base value
    return FMath::Max<int64>(1, static_cast<int64>(BaseValue * RarityMultiplier * 0.25f));
}

FLinearColor UMcpItemDefinitionAsset::GetRarityColor() const
{
    switch (Rarity)
    {
        case EForge2Rarity::Common:    return Forge2RarityColors::Common;
        case EForge2Rarity::Uncommon:  return Forge2RarityColors::Uncommon;
        case EForge2Rarity::Rare:      return Forge2RarityColors::Rare;
        case EForge2Rarity::Epic:      return Forge2RarityColors::Epic;
        case EForge2Rarity::Legendary: return Forge2RarityColors::Legendary;
        case EForge2Rarity::Mythic:    return Forge2RarityColors::Mythic;
        case EForge2Rarity::Unique:    return Forge2RarityColors::Unique;
        default:                       return FLinearColor::White;
    }
}

bool UMcpItemDefinitionAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (MaxStackSize < 1)
    {
        OutErrors.Add(FText::FromString(TEXT("Max stack size must be at least 1")));
        bValid = false;
    }

    if (Weight < 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Weight cannot be negative")));
        bValid = false;
    }

    if (BaseValue < 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Base value cannot be negative")));
        bValid = false;
    }

    if (SocketCount < 0 || SocketCount > 6)
    {
        OutErrors.Add(FText::FromString(TEXT("Socket count must be between 0 and 6")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Weapon Definition Asset Implementation
// ============================================================================

UMcpWeaponDefinitionAsset::UMcpWeaponDefinitionAsset()
{
    ItemType = EForge2ItemType::Weapon;
    WeaponType = EForge2WeaponType::Sword;
    bTwoHanded = false;
    AttackSpeed = 1.0f;
    Range = 200.0f;
    AttackArc = 90.0f;
}

float UMcpWeaponDefinitionAsset::GetDPS() const
{
    float AverageDamage = BaseDamage.GetAverageDamage();

    // Add bonus damage from additional damage sources
    for (const FForge2DamageInfo& Bonus : BonusDamage)
    {
        AverageDamage += Bonus.GetAverageDamage();
    }

    // DPS = Damage * AttackSpeed
    float DPS = AverageDamage * AttackSpeed;

    // Factor in critical hits (expected value)
    float CritChance = FMath::Clamp(BaseDamage.CriticalChanceBonus / 100.0f, 0.0f, 1.0f);
    float CritMultiplier = 1.0f + (BaseDamage.CriticalDamageBonus / 100.0f);
    float CritFactor = 1.0f + (CritChance * (CritMultiplier - 1.0f));

    return DPS * CritFactor;
}

bool UMcpWeaponDefinitionAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (BaseDamage.MinDamage < 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Minimum damage cannot be negative")));
        bValid = false;
    }

    if (BaseDamage.MaxDamage < BaseDamage.MinDamage)
    {
        OutErrors.Add(FText::FromString(TEXT("Maximum damage cannot be less than minimum damage")));
        bValid = false;
    }

    if (AttackSpeed <= 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Attack speed must be greater than 0")));
        bValid = false;
    }

    if (Range <= 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Range must be greater than 0")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Armor Definition Asset Implementation
// ============================================================================

UMcpArmorDefinitionAsset::UMcpArmorDefinitionAsset()
{
    ItemType = EForge2ItemType::Armor;
    Slot = EForge2ArmorSlot::Chest;
    ArmorType = EForge2ArmorType::Cloth;
    BaseArmor = 0.0f;
    BaseMagicResist = 0.0f;
}

bool UMcpArmorDefinitionAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (BaseArmor < 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Base armor cannot be negative")));
        bValid = false;
    }

    if (BaseMagicResist < 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Base magic resist cannot be negative")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Consumable Definition Asset Implementation
// ============================================================================

UMcpConsumableDefinitionAsset::UMcpConsumableDefinitionAsset()
{
    ItemType = EForge2ItemType::Consumable;
    EffectValue = 0.0f;
    EffectDuration = 0.0f;
    Cooldown = 0.0f;
    MaxCharges = 0;
    bCanUseInCombat = true;
    bConsumedOnUse = true;
    CastTime = 0.0f;
}

// ============================================================================
// Quest Data Asset Implementation
// ============================================================================

UMcpQuestDataAsset::UMcpQuestDataAsset()
{
    Category = EForge2QuestCategory::Side;
    SortOrder = 0;
    LevelRequirement = 0;
    bRepeatable = false;
    RepeatCooldown = 0.0f;
    bAutoTrack = true;
    bShareable = false;
    MaxPartySize = 0;
    ChainIndex = 0;
    bCanFail = false;
    TimeLimit = 0.0f;
    bFailOnPlayerDeath = false;
}

int32 UMcpQuestDataAsset::GetRequiredObjectiveCount() const
{
    int32 Count = 0;
    for (const FForge2QuestObjective& Objective : Objectives)
    {
        if (!Objective.bOptional)
        {
            Count++;
        }
    }
    return Count;
}

bool UMcpQuestDataAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (Objectives.Num() == 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Quest must have at least one objective")));
        bValid = false;
    }

    // Check for duplicate objective IDs
    TSet<FName> ObjectiveIds;
    for (const FForge2QuestObjective& Objective : Objectives)
    {
        if (Objective.ObjectiveId.IsNone())
        {
            OutErrors.Add(FText::FromString(TEXT("Objective has no ID")));
            bValid = false;
        }
        else if (ObjectiveIds.Contains(Objective.ObjectiveId))
        {
            OutErrors.Add(FText::Format(
                NSLOCTEXT("Forge2", "DuplicateObjectiveId", "Duplicate objective ID: {0}"),
                FText::FromName(Objective.ObjectiveId)
            ));
            bValid = false;
        }
        else
        {
            ObjectiveIds.Add(Objective.ObjectiveId);
        }

        if (Objective.RequiredCount < 1)
        {
            OutErrors.Add(FText::FromString(TEXT("Objective required count must be at least 1")));
            bValid = false;
        }
    }

    // Validate required objectives exist
    if (GetRequiredObjectiveCount() == 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Quest must have at least one required (non-optional) objective")));
        bValid = false;
    }

    // Validate quest chain
    if (!QuestChainId.IsNone() && ChainIndex < 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Chain index cannot be negative")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Dialogue Tree Asset Implementation
// ============================================================================

UMcpDialogueTreeAsset::UMcpDialogueTreeAsset()
{
    bUseDialogueCamera = true;
    CameraBlendTime = 0.5f;
    MusicFadeTime = 1.0f;
}

FForge2DialogueNode* UMcpDialogueTreeAsset::FindNode(FName NodeId)
{
    for (FForge2DialogueNode& Node : Nodes)
    {
        if (Node.NodeId == NodeId)
        {
            return &Node;
        }
    }
    return nullptr;
}

FForge2SpeakerProfile* UMcpDialogueTreeAsset::FindSpeaker(FName SpeakerId)
{
    for (FForge2SpeakerProfile& Speaker : Speakers)
    {
        if (Speaker.SpeakerId == SpeakerId)
        {
            return &Speaker;
        }
    }
    return nullptr;
}

bool UMcpDialogueTreeAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (Nodes.Num() == 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Dialogue tree must have at least one node")));
        bValid = false;
    }

    if (StartNodeId.IsNone())
    {
        OutErrors.Add(FText::FromString(TEXT("Start node ID is not set")));
        bValid = false;
    }

    // Validate start node exists
    bool bStartNodeFound = false;
    TSet<FName> NodeIds;
    TSet<FName> SpeakerIds;

    for (const FForge2SpeakerProfile& Speaker : Speakers)
    {
        if (Speaker.SpeakerId.IsNone())
        {
            OutErrors.Add(FText::FromString(TEXT("Speaker has no ID")));
            bValid = false;
        }
        else if (SpeakerIds.Contains(Speaker.SpeakerId))
        {
            OutErrors.Add(FText::Format(
                NSLOCTEXT("Forge2", "DuplicateSpeakerId", "Duplicate speaker ID: {0}"),
                FText::FromName(Speaker.SpeakerId)
            ));
            bValid = false;
        }
        else
        {
            SpeakerIds.Add(Speaker.SpeakerId);
        }
    }

    for (const FForge2DialogueNode& Node : Nodes)
    {
        if (Node.NodeId.IsNone())
        {
            OutErrors.Add(FText::FromString(TEXT("Node has no ID")));
            bValid = false;
        }
        else
        {
            if (NodeIds.Contains(Node.NodeId))
            {
                OutErrors.Add(FText::Format(
                    NSLOCTEXT("Forge2", "DuplicateNodeId", "Duplicate node ID: {0}"),
                    FText::FromName(Node.NodeId)
                ));
                bValid = false;
            }
            else
            {
                NodeIds.Add(Node.NodeId);
            }

            if (Node.NodeId == StartNodeId)
            {
                bStartNodeFound = true;
            }
        }

        // Validate speaker exists for speech nodes
        if (Node.NodeType == EForge2DialogueNodeType::NPCSpeech ||
            Node.NodeType == EForge2DialogueNodeType::PlayerChoice)
        {
            if (!Node.SpeakerId.IsNone() && !SpeakerIds.Contains(Node.SpeakerId))
            {
                OutErrors.Add(FText::Format(
                    NSLOCTEXT("Forge2", "UnknownSpeaker", "Node {0} references unknown speaker: {1}"),
                    FText::FromName(Node.NodeId),
                    FText::FromName(Node.SpeakerId)
                ));
                bValid = false;
            }
        }
    }

    if (!bStartNodeFound && !StartNodeId.IsNone())
    {
        OutErrors.Add(FText::Format(
            NSLOCTEXT("Forge2", "StartNodeNotFound", "Start node not found: {0}"),
            FText::FromName(StartNodeId)
        ));
        bValid = false;
    }

    // Validate node connections
    for (const FForge2DialogueNode& Node : Nodes)
    {
        for (const FName& NextId : Node.NextNodeIds)
        {
            if (!NextId.IsNone() && !NodeIds.Contains(NextId))
            {
                OutErrors.Add(FText::Format(
                    NSLOCTEXT("Forge2", "InvalidConnection", "Node {0} references non-existent node: {1}"),
                    FText::FromName(Node.NodeId),
                    FText::FromName(NextId)
                ));
                bValid = false;
            }
        }
    }

    return bValid;
}

// ============================================================================
// Skill Definition Asset Implementation
// ============================================================================

UMcpSkillDefinitionAsset::UMcpSkillDefinitionAsset()
{
    SkillType = EForge2SkillType::Active;
    TargetType = EForge2TargetType::Self;
    MaxRank = 5;
    BaseRange = 0.0f;
    AreaRadius = 0.0f;
    CastTime = 0.0f;
    ChannelDuration = 0.0f;
    bCanCrit = true;
    bCanBeInterrupted = true;
    TreePosition = FVector2D::ZeroVector;
}

FForge2SkillRank UMcpSkillDefinitionAsset::GetRankData(int32 Rank) const
{
    // Find the rank data for the specified rank
    for (const FForge2SkillRank& RankData : Ranks)
    {
        if (RankData.Rank == Rank)
        {
            return RankData;
        }
    }

    // Return default if not found
    FForge2SkillRank Default;
    Default.Rank = Rank;
    return Default;
}

bool UMcpSkillDefinitionAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (MaxRank < 1)
    {
        OutErrors.Add(FText::FromString(TEXT("Max rank must be at least 1")));
        bValid = false;
    }

    if (Ranks.Num() == 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Skill must have at least one rank defined")));
        bValid = false;
    }

    // Validate ranks
    TSet<int32> RankNumbers;
    for (const FForge2SkillRank& RankData : Ranks)
    {
        if (RankData.Rank < 1 || RankData.Rank > MaxRank)
        {
            OutErrors.Add(FText::Format(
                NSLOCTEXT("Forge2", "InvalidRank", "Rank {0} is outside valid range (1-{1})"),
                FText::AsNumber(RankData.Rank),
                FText::AsNumber(MaxRank)
            ));
            bValid = false;
        }

        if (RankNumbers.Contains(RankData.Rank))
        {
            OutErrors.Add(FText::Format(
                NSLOCTEXT("Forge2", "DuplicateRank", "Duplicate rank definition: {0}"),
                FText::AsNumber(RankData.Rank)
            ));
            bValid = false;
        }
        else
        {
            RankNumbers.Add(RankData.Rank);
        }

        if (RankData.PointCost < 0)
        {
            OutErrors.Add(FText::FromString(TEXT("Point cost cannot be negative")));
            bValid = false;
        }
    }

    // Channeled skills should have duration
    if (SkillType == EForge2SkillType::Channeled && ChannelDuration <= 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Channeled skills must have a channel duration > 0")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Status Effect Asset Implementation
// ============================================================================

UMcpStatusEffectAsset::UMcpStatusEffectAsset()
{
    EffectType = EForge2StatusEffectType::Buff;
    CCType = EForge2CCType::None;
    BaseDuration = 5.0f;
    bPermanent = false;
    bRefreshable = true;
    bStackable = false;
    MaxStacks = 1;
    bStacksIndependently = false;
    DamagePerTick = 0.0f;
    HealPerTick = 0.0f;
    TickInterval = 1.0f;
    TickDamageType = EForge2DamageType::Physical;
    MovementSpeedModifier = 100.0f;
    bDispellable = true;
    DispelDifficulty = 0;
    OverlayColor = FLinearColor::Transparent;
}

bool UMcpStatusEffectAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (!bPermanent && BaseDuration <= 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Non-permanent effects must have a duration > 0")));
        bValid = false;
    }

    if (MaxStacks < 1)
    {
        OutErrors.Add(FText::FromString(TEXT("Max stacks must be at least 1")));
        bValid = false;
    }

    if (TickInterval <= 0.0f && (DamagePerTick > 0.0f || HealPerTick > 0.0f))
    {
        OutErrors.Add(FText::FromString(TEXT("Tick interval must be > 0 when using tick damage/heal")));
        bValid = false;
    }

    if (MovementSpeedModifier < 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Movement speed modifier cannot be negative")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Loot Table Asset Implementation
// ============================================================================

UMcpLootTableAsset::UMcpLootTableAsset()
{
    MinDrops = 1;
    MaxDrops = 3;
    NothingChance = 0.0f;
    CurrencyMultiplierRange = FVector2D(0.8f, 1.2f);
    bScaleWithLevel = false;
    BaseLevel = 1;
}

TArray<FForge2ItemDrop> UMcpLootTableAsset::RollLoot(int32 PlayerLevel, float LuckBonus) const
{
    TArray<FForge2ItemDrop> Results;

    // Check for nothing drop
    if (FMath::RandRange(0.0f, 100.0f) < NothingChance)
    {
        return Results;
    }

    // Determine number of drops
    int32 NumDrops = FMath::RandRange(MinDrops, MaxDrops);

    // First, add guaranteed drops
    for (const FForge2LootEntry& Entry : Entries)
    {
        if (Entry.bGuaranteed)
        {
            FForge2ItemDrop Drop;
            Drop.Item = Entry.Item;
            Drop.DropChance = 100.0f;
            Drop.MinQuantity = Entry.MinQuantity;
            Drop.MaxQuantity = Entry.MaxQuantity;
            Results.Add(Drop);
        }
    }

    // Calculate total weight for non-guaranteed items
    float TotalWeight = 0.0f;
    TArray<int32> ValidIndices;

    for (int32 i = 0; i < Entries.Num(); i++)
    {
        const FForge2LootEntry& Entry = Entries[i];

        if (Entry.bGuaranteed)
        {
            continue;
        }

        // Check conditions
        bool bMeetsConditions = true;
        for (const FForge2LootCondition& Condition : Entry.Conditions)
        {
            if (Condition.ConditionType == FName(TEXT("Level")))
            {
                if (PlayerLevel < Condition.MinValue ||
                    (Condition.MaxValue > 0 && PlayerLevel > Condition.MaxValue))
                {
                    bMeetsConditions = false;
                    break;
                }
            }
        }

        if (bMeetsConditions)
        {
            ValidIndices.Add(i);
            TotalWeight += Entry.Weight * (1.0f + LuckBonus);
        }
    }

    // Roll for additional drops
    TSet<int32> UsedUniqueIndices;

    for (int32 DropNum = 0; DropNum < NumDrops && ValidIndices.Num() > 0; DropNum++)
    {
        float Roll = FMath::RandRange(0.0f, TotalWeight);
        float CurrentWeight = 0.0f;

        for (int32 i : ValidIndices)
        {
            const FForge2LootEntry& Entry = Entries[i];
            CurrentWeight += Entry.Weight * (1.0f + LuckBonus);

            if (Roll <= CurrentWeight)
            {
                // Skip if unique and already dropped
                if (Entry.bUnique && UsedUniqueIndices.Contains(i))
                {
                    continue;
                }

                FForge2ItemDrop Drop;
                Drop.Item = Entry.Item;
                Drop.DropChance = (Entry.Weight / TotalWeight) * 100.0f;
                Drop.MinQuantity = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
                Drop.MaxQuantity = Drop.MinQuantity;
                Results.Add(Drop);

                if (Entry.bUnique)
                {
                    UsedUniqueIndices.Add(i);
                    TotalWeight -= Entry.Weight * (1.0f + LuckBonus);
                }

                break;
            }
        }
    }

    return Results;
}

bool UMcpLootTableAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (MinDrops < 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Min drops cannot be negative")));
        bValid = false;
    }

    if (MaxDrops < MinDrops)
    {
        OutErrors.Add(FText::FromString(TEXT("Max drops cannot be less than min drops")));
        bValid = false;
    }

    if (NothingChance < 0.0f || NothingChance > 100.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Nothing chance must be between 0 and 100")));
        bValid = false;
    }

    for (const FForge2LootEntry& Entry : Entries)
    {
        if (Entry.Item.IsNull())
        {
            OutErrors.Add(FText::FromString(TEXT("Loot entry has no item")));
            bValid = false;
        }

        if (Entry.Weight < 0.0f)
        {
            OutErrors.Add(FText::FromString(TEXT("Loot entry weight cannot be negative")));
            bValid = false;
        }

        if (Entry.MinQuantity < 1)
        {
            OutErrors.Add(FText::FromString(TEXT("Loot entry min quantity must be at least 1")));
            bValid = false;
        }

        if (Entry.MaxQuantity < Entry.MinQuantity)
        {
            OutErrors.Add(FText::FromString(TEXT("Loot entry max quantity cannot be less than min")));
            bValid = false;
        }
    }

    return bValid;
}

// ============================================================================
// Crafting Recipe Asset Implementation
// ============================================================================

UMcpCraftingRecipeAsset::UMcpCraftingRecipeAsset()
{
    OutputQuantity = 1;
    RequiredSkillLevel = 0;
    RequiredPlayerLevel = 0;
    CraftingTime = 0.0f;
    SkillXpGain = 0;
    bDiscoverable = false;
    QualityVariance = 0.0f;
    CriticalCraftChance = 0.0f;
}

bool UMcpCraftingRecipeAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (OutputItem.IsNull())
    {
        OutErrors.Add(FText::FromString(TEXT("Recipe must have an output item")));
        bValid = false;
    }

    if (OutputQuantity < 1)
    {
        OutErrors.Add(FText::FromString(TEXT("Output quantity must be at least 1")));
        bValid = false;
    }

    if (Ingredients.Num() == 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Recipe must have at least one ingredient")));
        bValid = false;
    }

    for (const FForge2CraftingIngredient& Ingredient : Ingredients)
    {
        if (Ingredient.Item.IsNull())
        {
            OutErrors.Add(FText::FromString(TEXT("Ingredient has no item")));
            bValid = false;
        }

        if (Ingredient.Quantity < 1)
        {
            OutErrors.Add(FText::FromString(TEXT("Ingredient quantity must be at least 1")));
            bValid = false;
        }
    }

    if (CraftingTime < 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Crafting time cannot be negative")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Currency Definition Asset Implementation
// ============================================================================

UMcpCurrencyDefinitionAsset::UMcpCurrencyDefinitionAsset()
{
    MaxAmount = 999999999;
    bPremium = false;
    bBound = false;
    bTradeable = true;
    Color = FLinearColor::White;
    DisplayPriority = 0;
    ConversionRate = 1.0f;
}

// ============================================================================
// Faction Definition Asset Implementation
// ============================================================================

UMcpFactionDefinitionAsset::UMcpFactionDefinitionAsset()
{
    StartingReputation = 0;
    MinReputation = -42000;
    MaxReputation = 42000;
    PrimaryColor = FLinearColor::White;
    SecondaryColor = FLinearColor::Gray;
}

FForge2FactionRank UMcpFactionDefinitionAsset::GetRankForReputation(int32 Reputation) const
{
    FForge2FactionRank BestRank;
    int32 HighestThreshold = INT_MIN;

    for (const FForge2FactionRank& Rank : Ranks)
    {
        if (Reputation >= Rank.RequiredReputation && Rank.RequiredReputation > HighestThreshold)
        {
            BestRank = Rank;
            HighestThreshold = Rank.RequiredReputation;
        }
    }

    return BestRank;
}

bool UMcpFactionDefinitionAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (FactionId.IsNone())
    {
        OutErrors.Add(FText::FromString(TEXT("Faction ID is not set")));
        bValid = false;
    }

    if (Ranks.Num() == 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Faction must have at least one rank")));
        bValid = false;
    }

    if (MinReputation > MaxReputation)
    {
        OutErrors.Add(FText::FromString(TEXT("Min reputation cannot be greater than max reputation")));
        bValid = false;
    }

    if (StartingReputation < MinReputation || StartingReputation > MaxReputation)
    {
        OutErrors.Add(FText::FromString(TEXT("Starting reputation must be within min/max range")));
        bValid = false;
    }

    // Validate rank thresholds are ordered
    TArray<int32> Thresholds;
    for (const FForge2FactionRank& Rank : Ranks)
    {
        if (Rank.RankId.IsNone())
        {
            OutErrors.Add(FText::FromString(TEXT("Rank has no ID")));
            bValid = false;
        }
        Thresholds.Add(Rank.RequiredReputation);
    }
    Thresholds.Sort();

    for (int32 i = 0; i < Thresholds.Num() - 1; i++)
    {
        if (Thresholds[i] == Thresholds[i + 1])
        {
            OutErrors.Add(FText::Format(
                NSLOCTEXT("Forge2", "DuplicateRankThreshold", "Duplicate rank threshold: {0}"),
                FText::AsNumber(Thresholds[i])
            ));
            bValid = false;
        }
    }

    return bValid;
}

// ============================================================================
// Class Definition Asset Implementation
// ============================================================================

UMcpClassDefinitionAsset::UMcpClassDefinitionAsset()
{
    BaseHealth = 100.0f;
    BaseMana = 100.0f;
    HealthPerLevel = 10.0f;
    ManaPerLevel = 5.0f;
    SubclassUnlockLevel = 10;
    ClassColor = FLinearColor::White;
}

bool UMcpClassDefinitionAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (ClassId.IsNone())
    {
        OutErrors.Add(FText::FromString(TEXT("Class ID is not set")));
        bValid = false;
    }

    if (BaseHealth <= 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Base health must be greater than 0")));
        bValid = false;
    }

    if (BaseMana < 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Base mana cannot be negative")));
        bValid = false;
    }

    if (AllowedArmorTypes.Num() == 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Class must have at least one allowed armor type")));
        bValid = false;
    }

    if (AllowedWeaponTypes.Num() == 0)
    {
        OutErrors.Add(FText::FromString(TEXT("Class must have at least one allowed weapon type")));
        bValid = false;
    }

    if (SubclassUnlockLevel < 1)
    {
        OutErrors.Add(FText::FromString(TEXT("Subclass unlock level must be at least 1")));
        bValid = false;
    }

    return bValid;
}

// ============================================================================
// Dungeon Preset Asset Implementation
// ============================================================================

UMcpDungeonPresetAsset::UMcpDungeonPresetAsset()
{
    Algorithm = EForge2DungeonAlgorithm::BSP;
    GridWidth = 100;
    GridHeight = 100;
    CellSize = 100.0f;
    MinRooms = 8;
    MaxRooms = 15;
    RoomSizeRange = FVector2D(400.0f, 1200.0f);
    CorridorWidth = 200.0f;
    bAllowLoops = true;
    LoopChance = 20.0f;
    CAIterations = 5;
    CAFillPercent = 45.0f;
    CABirthLimit = 4;
    CADeathLimit = 3;
    bIncludeSecretRooms = true;
    SecretRoomChance = 15.0f;
    bIncludeTraps = true;
    bIncludeDoors = true;
}

bool UMcpDungeonPresetAsset::ValidateAsset(TArray<FText>& OutErrors) const
{
    bool bValid = Super::ValidateAsset(OutErrors);

    if (GridWidth < 10)
    {
        OutErrors.Add(FText::FromString(TEXT("Grid width must be at least 10")));
        bValid = false;
    }

    if (GridHeight < 10)
    {
        OutErrors.Add(FText::FromString(TEXT("Grid height must be at least 10")));
        bValid = false;
    }

    if (CellSize <= 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Cell size must be greater than 0")));
        bValid = false;
    }

    if (MinRooms < 1)
    {
        OutErrors.Add(FText::FromString(TEXT("Min rooms must be at least 1")));
        bValid = false;
    }

    if (MaxRooms < MinRooms)
    {
        OutErrors.Add(FText::FromString(TEXT("Max rooms cannot be less than min rooms")));
        bValid = false;
    }

    if (RoomSizeRange.X > RoomSizeRange.Y)
    {
        OutErrors.Add(FText::FromString(TEXT("Room size range min is greater than max")));
        bValid = false;
    }

    if (CorridorWidth <= 0.0f)
    {
        OutErrors.Add(FText::FromString(TEXT("Corridor width must be greater than 0")));
        bValid = false;
    }

    // Cellular automata specific validation
    if (Algorithm == EForge2DungeonAlgorithm::CellularAutomata)
    {
        if (CAIterations < 1)
        {
            OutErrors.Add(FText::FromString(TEXT("CA iterations must be at least 1")));
            bValid = false;
        }

        if (CAFillPercent < 0.0f || CAFillPercent > 100.0f)
        {
            OutErrors.Add(FText::FromString(TEXT("CA fill percent must be between 0 and 100")));
            bValid = false;
        }
    }

    return bValid;
}
