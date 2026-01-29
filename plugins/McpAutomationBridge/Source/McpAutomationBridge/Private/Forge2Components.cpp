// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Runtime Actor Components Implementation

#include "Forge2Components.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/World.h"

// ============================================================================
// Quest Tracker Component
// ============================================================================

UMcpQuestTrackerComponent::UMcpQuestTrackerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UMcpQuestTrackerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpQuestTrackerComponent, ActiveQuests);
    DOREPLIFETIME(UMcpQuestTrackerComponent, CompletedQuests);
    DOREPLIFETIME(UMcpQuestTrackerComponent, FailedQuests);
    DOREPLIFETIME(UMcpQuestTrackerComponent, TrackedQuestId);
}

bool UMcpQuestTrackerComponent::StartQuest(FName QuestId, UMcpQuestDataAsset* QuestAsset)
{
    if (QuestId.IsNone() || ActiveQuests.Contains(QuestId) || CompletedQuests.Contains(QuestId))
    {
        return false;
    }

    FMcpActiveQuest NewQuest;
    NewQuest.QuestId = QuestId;
    NewQuest.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewQuest.bIsTracked = ActiveQuests.Num() == 0; // Auto-track if first quest

    ActiveQuests.Add(QuestId, NewQuest);

    if (NewQuest.bIsTracked)
    {
        TrackedQuestId = QuestId;
    }

    OnQuestStarted.Broadcast(QuestId, QuestAsset);
    return true;
}

bool UMcpQuestTrackerComponent::UpdateObjective(FName QuestId, FName ObjectiveId, int32 ProgressDelta)
{
    FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
    if (!Quest)
    {
        return false;
    }

    int32& Progress = Quest->ObjectiveProgress.FindOrAdd(ObjectiveId);
    Progress += ProgressDelta;

    OnObjectiveUpdated.Broadcast(QuestId, ObjectiveId, Progress);
    return true;
}

bool UMcpQuestTrackerComponent::CompleteQuest(FName QuestId, bool bSuccess)
{
    if (!ActiveQuests.Contains(QuestId))
    {
        return false;
    }

    ActiveQuests.Remove(QuestId);

    if (bSuccess)
    {
        CompletedQuests.Add(QuestId);
    }
    else
    {
        FailedQuests.Add(QuestId);
    }

    if (TrackedQuestId == QuestId)
    {
        TrackedQuestId = NAME_None;
        // Auto-track next quest
        for (const auto& Pair : ActiveQuests)
        {
            TrackedQuestId = Pair.Key;
            break;
        }
    }

    OnQuestCompleted.Broadcast(QuestId, bSuccess);
    return true;
}

bool UMcpQuestTrackerComponent::HasCompletedQuest(FName QuestId) const
{
    return CompletedQuests.Contains(QuestId);
}

int32 UMcpQuestTrackerComponent::GetObjectiveProgress(FName QuestId, FName ObjectiveId) const
{
    const FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
    if (!Quest)
    {
        return 0;
    }

    const int32* Progress = Quest->ObjectiveProgress.Find(ObjectiveId);
    return Progress ? *Progress : 0;
}

void UMcpQuestTrackerComponent::TrackQuest(FName QuestId)
{
    if (ActiveQuests.Contains(QuestId))
    {
        TrackedQuestId = QuestId;
    }
}

// ============================================================================
// Dialogue Component
// ============================================================================

UMcpDialogueComponent::UMcpDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UMcpDialogueComponent::StartDialogue(AActor* Initiator, UMcpDialogueTreeAsset* DialogueTree)
{
    if (bInDialogue)
    {
        return false;
    }

    // Use provided dialogue tree or default
    UMcpDialogueTreeAsset* TreeToUse = DialogueTree;
    if (!TreeToUse && DefaultDialogueTree.IsValid())
    {
        TreeToUse = DefaultDialogueTree.LoadSynchronous();
    }

    if (!TreeToUse)
    {
        return false;
    }

    ActiveDialogueTree = TreeToUse;
    bInDialogue = true;
    CurrentNodeId = NAME_None; // Will be set to root node

    OnDialogueStarted.Broadcast(Initiator, ActiveDialogueTree);
    return true;
}

void UMcpDialogueComponent::EndDialogue()
{
    if (!bInDialogue)
    {
        return;
    }

    bInDialogue = false;
    ActiveDialogueTree = nullptr;
    CurrentNodeId = NAME_None;

    OnDialogueEnded.Broadcast();
}

bool UMcpDialogueComponent::SelectChoice(int32 ChoiceIndex)
{
    if (!bInDialogue)
    {
        return false;
    }

    // Choice selection would be handled by dialogue system
    // This is a placeholder for the actual implementation
    return true;
}

void UMcpDialogueComponent::SetDialogueVariable(FName VariableName, const FString& Value)
{
    DialogueVariables.Add(VariableName, Value);
}

FString UMcpDialogueComponent::GetDialogueVariable(FName VariableName) const
{
    const FString* Value = DialogueVariables.Find(VariableName);
    return Value ? *Value : FString();
}

bool UMcpDialogueComponent::CanStartDialogue() const
{
    return !bInDialogue && (DefaultDialogueTree.IsValid() || DefaultDialogueTree.IsNull() == false);
}

// ============================================================================
// Inventory Component
// ============================================================================

UMcpInventoryComponent::UMcpInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UMcpInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpInventoryComponent, Slots);
    DOREPLIFETIME(UMcpInventoryComponent, CurrentWeight);
    DOREPLIFETIME(UMcpInventoryComponent, Currencies);
}

int32 UMcpInventoryComponent::AddItem(FName ItemId, int32 Count, int32 PreferredSlot)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return 0;
    }

    int32 RemainingCount = Count;
    int32 TotalAdded = 0;
    
    // Get max stack size from item definition registry
    int32 MaxStackSize = 99; // Default max stack
    if (ItemDefinitionRegistry.Contains(ItemId))
    {
        UMcpItemDefinitionAsset* ItemDef = ItemDefinitionRegistry[ItemId].LoadSynchronous();
        if (ItemDef)
        {
            MaxStackSize = ItemDef->MaxStackSize;
        }
    }

    // Ensure we have enough slots
    if (MaxSlots > 0)
    {
        while (Slots.Num() < MaxSlots && Slots.Num() < PreferredSlot + 1)
        {
            Slots.Add(FMcpInventorySlot());
        }
    }

    // First try to stack with existing items (respecting max stack size)
    for (int32 i = 0; i < Slots.Num() && RemainingCount > 0; ++i)
    {
        if (Slots[i].ItemId == ItemId && Slots[i].Count < MaxStackSize)
        {
            int32 SpaceInStack = MaxStackSize - Slots[i].Count;
            int32 ToAdd = FMath::Min(SpaceInStack, RemainingCount);
            Slots[i].Count += ToAdd;
            TotalAdded += ToAdd;
            RemainingCount -= ToAdd;
            OnItemAdded.Broadcast(ItemId, ToAdd, i);
        }
    }

    // Then try to add to empty slots
    for (int32 i = 0; i < Slots.Num() && RemainingCount > 0; ++i)
    {
        if (Slots[i].IsEmpty())
        {
            int32 ToAdd = FMath::Min(MaxStackSize, RemainingCount);
            Slots[i].ItemId = ItemId;
            Slots[i].Count = ToAdd;
            TotalAdded += ToAdd;
            RemainingCount -= ToAdd;
            OnItemAdded.Broadcast(ItemId, ToAdd, i);
        }
    }

    // Add new slots if possible
    while (RemainingCount > 0 && (MaxSlots < 0 || Slots.Num() < MaxSlots))
    {
        FMcpInventorySlot NewSlot;
        NewSlot.ItemId = ItemId;
        NewSlot.Count = FMath::Min(MaxStackSize, RemainingCount);
        int32 NewIndex = Slots.Add(NewSlot);
        TotalAdded += NewSlot.Count;
        RemainingCount -= NewSlot.Count;
        OnItemAdded.Broadcast(ItemId, NewSlot.Count, NewIndex);
    }

    if (TotalAdded > 0)
    {
        RecalculateWeight();
        OnInventoryChanged.Broadcast();
    }

    return TotalAdded;
}

int32 UMcpInventoryComponent::RemoveItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return 0;
    }

    int32 RemainingToRemove = Count;
    int32 TotalRemoved = 0;

    for (int32 i = Slots.Num() - 1; i >= 0 && RemainingToRemove > 0; --i)
    {
        if (Slots[i].ItemId == ItemId)
        {
            int32 ToRemove = FMath::Min(Slots[i].Count, RemainingToRemove);
            Slots[i].Count -= ToRemove;
            RemainingToRemove -= ToRemove;
            TotalRemoved += ToRemove;

            if (Slots[i].Count <= 0)
            {
                Slots[i] = FMcpInventorySlot();
            }

            OnItemRemoved.Broadcast(ItemId, ToRemove, i);
        }
    }

    if (TotalRemoved > 0)
    {
        RecalculateWeight();
        OnInventoryChanged.Broadcast();
    }

    return TotalRemoved;
}

bool UMcpInventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 Count)
{
    if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
    {
        return false;
    }

    FName ItemId = Slots[SlotIndex].ItemId;
    int32 ToRemove = FMath::Min(Slots[SlotIndex].Count, Count);
    Slots[SlotIndex].Count -= ToRemove;

    if (Slots[SlotIndex].Count <= 0)
    {
        Slots[SlotIndex] = FMcpInventorySlot();
    }

    OnItemRemoved.Broadcast(ItemId, ToRemove, SlotIndex);
    RecalculateWeight();
    OnInventoryChanged.Broadcast();
    return true;
}

int32 UMcpInventoryComponent::GetItemCount(FName ItemId) const
{
    int32 Total = 0;
    for (const FMcpInventorySlot& Slot : Slots)
    {
        if (Slot.ItemId == ItemId)
        {
            Total += Slot.Count;
        }
    }
    return Total;
}

bool UMcpInventoryComponent::HasItem(FName ItemId, int32 MinCount) const
{
    return GetItemCount(ItemId) >= MinCount;
}

bool UMcpInventoryComponent::SwapSlots(int32 SlotA, int32 SlotB)
{
    if (!Slots.IsValidIndex(SlotA) || !Slots.IsValidIndex(SlotB))
    {
        return false;
    }

    Slots.Swap(SlotA, SlotB);
    OnInventoryChanged.Broadcast();
    return true;
}

void UMcpInventoryComponent::SortInventory()
{
    Slots.Sort([](const FMcpInventorySlot& A, const FMcpInventorySlot& B)
    {
        if (A.IsEmpty() && !B.IsEmpty()) return false;
        if (!A.IsEmpty() && B.IsEmpty()) return true;
        return A.ItemId.ToString() < B.ItemId.ToString();
    });
    OnInventoryChanged.Broadcast();
}

bool UMcpInventoryComponent::ModifyCurrency(FName CurrencyId, int64 Delta)
{
    int64& Amount = Currencies.FindOrAdd(CurrencyId);
    int64 NewAmount = Amount + Delta;

    if (NewAmount < 0)
    {
        return false;
    }

    Amount = NewAmount;
    return true;
}

int64 UMcpInventoryComponent::GetCurrency(FName CurrencyId) const
{
    const int64* Amount = Currencies.Find(CurrencyId);
    return Amount ? *Amount : 0;
}

bool UMcpInventoryComponent::CanAddItem(FName ItemId, int32 Count) const
{
    if (ItemId.IsNone() || Count <= 0) return false;
    
    // Get item weight from definition
    float ItemWeight = 0.0f;
    int32 MaxStackSize = 99;
    if (ItemDefinitionRegistry.Contains(ItemId))
    {
        UMcpItemDefinitionAsset* ItemDef = ItemDefinitionRegistry[ItemId].LoadSynchronous();
        if (ItemDef)
        {
            ItemWeight = ItemDef->Weight;
            MaxStackSize = ItemDef->MaxStackSize;
        }
    }
    
    // Check weight capacity
    if (MaxWeight > 0.0f)
    {
        float TotalWeight = CurrentWeight + (ItemWeight * Count);
        if (TotalWeight > MaxWeight)
        {
            return false;
        }
    }
    
    // Calculate how much space we need
    int32 RemainingToAdd = Count;
    
    // Check existing stacks
    for (const FMcpInventorySlot& Slot : Slots)
    {
        if (Slot.ItemId == ItemId && Slot.Count < MaxStackSize)
        {
            int32 SpaceInStack = MaxStackSize - Slot.Count;
            RemainingToAdd -= FMath::Min(SpaceInStack, RemainingToAdd);
            if (RemainingToAdd <= 0) return true;
        }
    }
    
    // Check empty slots
    for (const FMcpInventorySlot& Slot : Slots)
    {
        if (Slot.IsEmpty())
        {
            RemainingToAdd -= FMath::Min(MaxStackSize, RemainingToAdd);
            if (RemainingToAdd <= 0) return true;
        }
    }
    
    // Check if we can create new slots
    if (MaxSlots < 0) return true; // Unlimited slots
    
    int32 AvailableNewSlots = MaxSlots - Slots.Num();
    int32 SlotsNeeded = FMath::CeilToInt((float)RemainingToAdd / (float)MaxStackSize);
    
    return SlotsNeeded <= AvailableNewSlots;
}

void UMcpInventoryComponent::RecalculateWeight()
{
    CurrentWeight = 0.0f;
    
    for (const FMcpInventorySlot& Slot : Slots)
    {
        if (!Slot.IsEmpty() && ItemDefinitionRegistry.Contains(Slot.ItemId))
        {
            UMcpItemDefinitionAsset* ItemDef = ItemDefinitionRegistry[Slot.ItemId].LoadSynchronous();
            if (ItemDef)
            {
                CurrentWeight += ItemDef->Weight * Slot.Count;
            }
        }
    }
}

// ============================================================================
// Combat Component
// ============================================================================

UMcpCombatComponent::UMcpCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UMcpCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpCombatComponent, Stats);
    DOREPLIFETIME(UMcpCombatComponent, bIsDead);
    DOREPLIFETIME(UMcpCombatComponent, TeamId);
    DOREPLIFETIME(UMcpCombatComponent, CombatLevel);
}

void UMcpCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update invincibility
    if (InvincibilityRemaining > 0)
    {
        InvincibilityRemaining -= DeltaTime;
        if (InvincibilityRemaining <= 0)
        {
            bIsInvincible = false;
        }
    }

    // Regeneration (simplified)
    if (!bIsDead)
    {
        // Health regen would be stat-based
        // Stats.CurrentHealth = FMath::Min(Stats.CurrentHealth + HealthRegen * DeltaTime, Stats.MaxHealth);
    }
}

float UMcpCombatComponent::ApplyDamage(AActor* Instigator, float BaseDamage, EForge2DamageType DamageType, bool bCanCrit)
{
    if (bIsDead || bIsInvincible || BaseDamage <= 0)
    {
        return 0.0f;
    }

    float FinalDamage = BaseDamage;
    bool bWasCritical = false;

    // Apply critical hit
    if (bCanCrit)
    {
        // Critical would be based on attacker's crit chance
        // This is simplified - normally you'd get crit stats from the attacker
        if (FMath::FRand() < 0.1f) // 10% base crit chance
        {
            FinalDamage *= 2.0f;
            bWasCritical = true;
        }
    }

    // Apply resistance
    const float* Resistance = Stats.Resistances.Find(DamageType);
    if (Resistance)
    {
        FinalDamage *= (1.0f - FMath::Clamp(*Resistance, 0.0f, 0.9f));
    }

    // Apply armor (for physical)
    if (DamageType == EForge2DamageType::Physical)
    {
        FinalDamage *= CalculateDamageReduction(Stats.Armor);
    }

    // Apply damage
    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - FinalDamage);

    OnDamageTaken.Broadcast(Instigator, FinalDamage, DamageType, bWasCritical);
    OnHealthChanged.Broadcast(Stats.CurrentHealth, Stats.MaxHealth);

    // Check for death
    if (Stats.CurrentHealth <= 0)
    {
        Kill(Instigator);
    }

    return FinalDamage;
}

void UMcpCombatComponent::Heal(float Amount, bool bCanOverheal)
{
    if (bIsDead || Amount <= 0)
    {
        return;
    }

    float MaxHeal = bCanOverheal ? Stats.MaxHealth * 1.5f : Stats.MaxHealth;
    Stats.CurrentHealth = FMath::Min(Stats.CurrentHealth + Amount, MaxHeal);
    OnHealthChanged.Broadcast(Stats.CurrentHealth, Stats.MaxHealth);
}

void UMcpCombatComponent::RestoreMana(float Amount)
{
    Stats.CurrentMana = FMath::Min(Stats.CurrentMana + Amount, Stats.MaxMana);
}

void UMcpCombatComponent::RestoreStamina(float Amount)
{
    Stats.CurrentStamina = FMath::Min(Stats.CurrentStamina + Amount, Stats.MaxStamina);
}

bool UMcpCombatComponent::SpendMana(float Amount)
{
    if (Stats.CurrentMana < Amount)
    {
        return false;
    }
    Stats.CurrentMana -= Amount;
    return true;
}

bool UMcpCombatComponent::SpendStamina(float Amount)
{
    if (Stats.CurrentStamina < Amount)
    {
        return false;
    }
    Stats.CurrentStamina -= Amount;
    return true;
}

void UMcpCombatComponent::Kill(AActor* Killer)
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    Stats.CurrentHealth = 0.0f;
    OnDeath.Broadcast(Killer);
}

void UMcpCombatComponent::Revive(float HealthPercent)
{
    if (!bIsDead)
    {
        return;
    }

    bIsDead = false;
    Stats.CurrentHealth = Stats.MaxHealth * FMath::Clamp(HealthPercent, 0.01f, 1.0f);
    OnRevived.Broadcast();
    OnHealthChanged.Broadcast(Stats.CurrentHealth, Stats.MaxHealth);
}

void UMcpCombatComponent::SetInvincible(float Duration)
{
    bIsInvincible = true;
    InvincibilityRemaining = Duration;
}

float UMcpCombatComponent::GetHealthPercent() const
{
    return Stats.MaxHealth > 0 ? Stats.CurrentHealth / Stats.MaxHealth : 0.0f;
}

bool UMcpCombatComponent::IsAlly(AActor* Other) const
{
    if (!Other)
    {
        return false;
    }

    UMcpCombatComponent* OtherCombat = Other->FindComponentByClass<UMcpCombatComponent>();
    return OtherCombat && OtherCombat->TeamId == TeamId;
}

float UMcpCombatComponent::CalculateDamageReduction(float Armor) const
{
    // Diminishing returns formula
    // 100 armor = 50% reduction, 200 = 66%, 400 = 80%
    return 100.0f / (100.0f + Armor);
}

// ============================================================================
// Interaction Component
// ============================================================================

UMcpInteractionComponent::UMcpInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UMcpInteractionComponent::TryInteract(AActor* Interactor, int32 InteractionIndex)
{
    if (!CanInteract(Interactor, InteractionIndex))
    {
        return false;
    }

    OnInteract.Broadcast(Interactor, InteractionIndex);
    return true;
}

bool UMcpInteractionComponent::CanInteract(AActor* Interactor, int32 InteractionIndex) const
{
    if (!Interactor || !Interactions.IsValidIndex(InteractionIndex))
    {
        return false;
    }

    const FMcpInteractionOption& Option = Interactions[InteractionIndex];
    if (!Option.bEnabled)
    {
        return false;
    }

    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Interactor->GetActorLocation());
    if (Distance > InteractionDistance)
    {
        return false;
    }

    // Check required tags on the interactor
    if (InteractionIndex >= 0 && Interactions.IsValidIndex(InteractionIndex))
    {
        const FMcpInteractionData& InteractionData = Interactions[InteractionIndex];
        
        // Get gameplay tags from interactor (if it has a tag component)
        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Interactor);
        if (TagInterface)
        {
            FGameplayTagContainer InteractorTags;
            TagInterface->GetOwnedGameplayTags(InteractorTags);
            
            // Check required tags
            if (InteractionData.RequiredTags.Num() > 0)
            {
                if (!InteractorTags.HasAll(InteractionData.RequiredTags))
                {
                    return false;
                }
            }
            
            // Check blocked tags
            if (InteractionData.BlockedTags.Num() > 0)
            {
                if (InteractorTags.HasAny(InteractionData.BlockedTags))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

TArray<int32> UMcpInteractionComponent::GetAvailableInteractions(AActor* Interactor) const
{
    TArray<int32> Available;
    for (int32 i = 0; i < Interactions.Num(); ++i)
    {
        if (CanInteract(Interactor, i))
        {
            Available.Add(i);
        }
    }
    return Available;
}

void UMcpInteractionComponent::SetInteractionEnabled(int32 InteractionIndex, bool bEnabled)
{
    if (Interactions.IsValidIndex(InteractionIndex))
    {
        Interactions[InteractionIndex].bEnabled = bEnabled;
    }
}

// ============================================================================
// Status Effect Component
// ============================================================================

UMcpStatusEffectComponent::UMcpStatusEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UMcpStatusEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpStatusEffectComponent, ActiveEffects);
}

void UMcpStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update effect durations
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        FMcpActiveStatusEffect& Effect = ActiveEffects[i];
        if (!Effect.IsPermanent())
        {
            Effect.RemainingTime -= DeltaTime;
            if (Effect.IsExpired())
            {
                FName EffectId = Effect.EffectId;
                ActiveEffects.RemoveAt(i);
                OnEffectRemoved.Broadcast(EffectId, true);
            }
        }
    }
}

bool UMcpStatusEffectComponent::ApplyEffect(const FMcpActiveStatusEffect& Effect)
{
    // Check for immunity - if effect has tags, check against immunity tags
    if (Effect.EffectTags.Num() > 0 && ImmunityTags.Num() > 0)
    {
        if (Effect.EffectTags.HasAny(ImmunityTags))
        {
            // Immune to this effect
            OnEffectBlocked.Broadcast(Effect.EffectId, TEXT("Immunity"));
            return false;
        }
    }
    
    // Check against active immunity effects (temporary immunities)
    for (const FMcpActiveStatusEffect& ActiveEffect : ActiveEffects)
    {
        if (ActiveEffect.GrantedImmunityTags.Num() > 0)
        {
            if (Effect.EffectTags.HasAny(ActiveEffect.GrantedImmunityTags))
            {
                OnEffectBlocked.Broadcast(Effect.EffectId, TEXT("Temporary Immunity"));
                return false;
            }
        }
    }

    // Check for existing effect
    for (FMcpActiveStatusEffect& Existing : ActiveEffects)
    {
        if (Existing.EffectId == Effect.EffectId)
        {
            // Stack or refresh
            if (Existing.Stacks < Existing.MaxStacks)
            {
                Existing.Stacks++;
                OnEffectStackChanged.Broadcast(Effect.EffectId, Existing.Stacks);
            }
            Existing.RemainingTime = Effect.Duration;
            return true;
        }
    }

    // Add new effect
    ActiveEffects.Add(Effect);
    OnEffectApplied.Broadcast(Effect.EffectId, Effect.Stacks);
    return true;
}

bool UMcpStatusEffectComponent::RemoveEffect(FName EffectId, bool bRemoveAllStacks)
{
    for (int32 i = 0; i < ActiveEffects.Num(); ++i)
    {
        if (ActiveEffects[i].EffectId == EffectId)
        {
            if (bRemoveAllStacks || ActiveEffects[i].Stacks <= 1)
            {
                ActiveEffects.RemoveAt(i);
                OnEffectRemoved.Broadcast(EffectId, false);
            }
            else
            {
                ActiveEffects[i].Stacks--;
                OnEffectStackChanged.Broadcast(EffectId, ActiveEffects[i].Stacks);
            }
            return true;
        }
    }
    return false;
}

void UMcpStatusEffectComponent::RemoveAllEffects(EForge2EffectType TypeFilter)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (TypeFilter == EForge2EffectType::Neutral || ActiveEffects[i].EffectType == TypeFilter)
        {
            FName EffectId = ActiveEffects[i].EffectId;
            ActiveEffects.RemoveAt(i);
            OnEffectRemoved.Broadcast(EffectId, false);
        }
    }
}

bool UMcpStatusEffectComponent::HasEffect(FName EffectId) const
{
    for (const FMcpActiveStatusEffect& Effect : ActiveEffects)
    {
        if (Effect.EffectId == EffectId)
        {
            return true;
        }
    }
    return false;
}

int32 UMcpStatusEffectComponent::GetEffectStacks(FName EffectId) const
{
    for (const FMcpActiveStatusEffect& Effect : ActiveEffects)
    {
        if (Effect.EffectId == EffectId)
        {
            return Effect.Stacks;
        }
    }
    return 0;
}

float UMcpStatusEffectComponent::GetTotalStatModifier(FName StatName) const
{
    float Total = 0.0f;
    for (const FMcpActiveStatusEffect& Effect : ActiveEffects)
    {
        const float* Modifier = Effect.StatModifiers.Find(StatName);
        if (Modifier)
        {
            Total += *Modifier * Effect.Stacks;
        }
    }
    return Total;
}

void UMcpStatusEffectComponent::RefreshEffectDuration(FName EffectId, float NewDuration)
{
    for (FMcpActiveStatusEffect& Effect : ActiveEffects)
    {
        if (Effect.EffectId == EffectId)
        {
            Effect.RemainingTime = NewDuration < 0 ? Effect.Duration : NewDuration;
            break;
        }
    }
}

// ============================================================================
// Equipment Component
// ============================================================================

UMcpEquipmentComponent::UMcpEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UMcpEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpEquipmentComponent, EquippedItems);
}

bool UMcpEquipmentComponent::EquipItem(EForge2EquipmentSlot Slot, const FMcpInventorySlot& Item)
{
    if (Slot == EForge2EquipmentSlot::None)
    {
        return false;
    }

    FName PreviousItemId = NAME_None;
    if (FMcpInventorySlot* Existing = EquippedItems.Find(Slot))
    {
        PreviousItemId = Existing->ItemId;
    }

    EquippedItems.Add(Slot, Item);
    RecalculateStats();

    OnItemEquipped.Broadcast(Slot, Item.ItemId, PreviousItemId);
    return true;
}

FMcpInventorySlot UMcpEquipmentComponent::UnequipItem(EForge2EquipmentSlot Slot)
{
    FMcpInventorySlot RemovedItem;

    if (FMcpInventorySlot* Existing = EquippedItems.Find(Slot))
    {
        RemovedItem = *Existing;
        EquippedItems.Remove(Slot);
        RecalculateStats();
        OnItemUnequipped.Broadcast(Slot, RemovedItem.ItemId);
    }

    return RemovedItem;
}

FMcpInventorySlot UMcpEquipmentComponent::GetEquippedItem(EForge2EquipmentSlot Slot) const
{
    const FMcpInventorySlot* Item = EquippedItems.Find(Slot);
    return Item ? *Item : FMcpInventorySlot();
}

bool UMcpEquipmentComponent::HasItemEquipped(EForge2EquipmentSlot Slot) const
{
    const FMcpInventorySlot* Item = EquippedItems.Find(Slot);
    return Item && !Item->IsEmpty();
}

float UMcpEquipmentComponent::GetEquipmentStat(EForge2StatType Stat) const
{
    const float* Value = EquipmentStats.Find(Stat);
    return Value ? *Value : 0.0f;
}

void UMcpEquipmentComponent::RecalculateStats()
{
    EquipmentStats.Empty();
    
    // Iterate through all equipped items and sum their stats
    for (const auto& EquipPair : EquippedItems)
    {
        const FMcpInventorySlot& Slot = EquipPair.Value;
        if (Slot.IsEmpty()) continue;
        
        // Look up item definition
        if (ItemDefinitionRegistry.Contains(Slot.ItemId))
        {
            UMcpItemDefinitionAsset* ItemDef = ItemDefinitionRegistry[Slot.ItemId].LoadSynchronous();
            if (ItemDef)
            {
                // Check if it's armor (has defense stats)
                if (UMcpArmorDefinitionAsset* ArmorDef = Cast<UMcpArmorDefinitionAsset>(ItemDef))
                {
                    // Add armor stats
                    EquipmentStats.FindOrAdd(EForge2StatType::Defense) += ArmorDef->Defense;
                    EquipmentStats.FindOrAdd(EForge2StatType::MagicResist) += ArmorDef->MagicResist;
                    
                    // Add any bonus stats from the armor
                    for (const auto& BonusStat : ArmorDef->BonusStats)
                    {
                        EquipmentStats.FindOrAdd(BonusStat.Key) += BonusStat.Value;
                    }
                }
                // Check if it's a weapon (has damage stats)
                else if (UMcpWeaponDefinitionAsset* WeaponDef = Cast<UMcpWeaponDefinitionAsset>(ItemDef))
                {
                    EquipmentStats.FindOrAdd(EForge2StatType::Attack) += WeaponDef->BaseDamage;
                    EquipmentStats.FindOrAdd(EForge2StatType::AttackSpeed) += WeaponDef->AttackSpeed;
                    
                    // Add any bonus stats from the weapon
                    for (const auto& BonusStat : WeaponDef->BonusStats)
                    {
                        EquipmentStats.FindOrAdd(BonusStat.Key) += BonusStat.Value;
                    }
                }
                
                // Add base item stats (all equipment can have these)
                for (const auto& BaseStat : ItemDef->BaseStats)
                {
                    EquipmentStats.FindOrAdd(BaseStat.Key) += BaseStat.Value;
                }
            }
        }
    }
    
    OnStatsChanged.Broadcast();
}

// ============================================================================
// Progression Component
// ============================================================================

UMcpProgressionComponent::UMcpProgressionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    CalculateXPToNextLevel();
}

void UMcpProgressionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpProgressionComponent, Level);
    DOREPLIFETIME(UMcpProgressionComponent, CurrentXP);
    DOREPLIFETIME(UMcpProgressionComponent, TotalXPEarned);
    DOREPLIFETIME(UMcpProgressionComponent, SkillPoints);
    DOREPLIFETIME(UMcpProgressionComponent, UnlockedSkills);
}

void UMcpProgressionComponent::AddXP(int32 Amount)
{
    if (Amount <= 0 || Level >= MaxLevel)
    {
        return;
    }

    CurrentXP += Amount;
    TotalXPEarned += Amount;
    OnXPGained.Broadcast(Amount, TotalXPEarned);

    // Check for level up
    while (CurrentXP >= XPToNextLevel && Level < MaxLevel)
    {
        CurrentXP -= XPToNextLevel;
        ProcessLevelUp();
    }

    // Cap XP at max level
    if (Level >= MaxLevel)
    {
        CurrentXP = 0;
    }
}

bool UMcpProgressionComponent::UnlockSkill(FName SkillId, int32 PointCost)
{
    if (SkillId.IsNone() || SkillPoints < PointCost || UnlockedSkills.Contains(SkillId))
    {
        return false;
    }

    SkillPoints -= PointCost;
    UnlockedSkills.Add(SkillId, 1);
    OnSkillUnlocked.Broadcast(SkillId, 1);
    return true;
}

bool UMcpProgressionComponent::UpgradeSkill(FName SkillId, int32 PointCost)
{
    if (SkillId.IsNone() || SkillPoints < PointCost)
    {
        return false;
    }

    int32* CurrentRank = UnlockedSkills.Find(SkillId);
    if (!CurrentRank)
    {
        return false;
    }

    SkillPoints -= PointCost;
    (*CurrentRank)++;
    OnSkillUnlocked.Broadcast(SkillId, *CurrentRank);
    return true;
}

int32 UMcpProgressionComponent::GetSkillRank(FName SkillId) const
{
    const int32* Rank = UnlockedSkills.Find(SkillId);
    return Rank ? *Rank : 0;
}

bool UMcpProgressionComponent::HasSkill(FName SkillId, int32 MinRank) const
{
    return GetSkillRank(SkillId) >= MinRank;
}

float UMcpProgressionComponent::GetLevelProgress() const
{
    return XPToNextLevel > 0 ? static_cast<float>(CurrentXP) / static_cast<float>(XPToNextLevel) : 0.0f;
}

void UMcpProgressionComponent::SetLevel(int32 NewLevel, bool bResetXP)
{
    Level = FMath::Clamp(NewLevel, 1, MaxLevel);
    if (bResetXP)
    {
        CurrentXP = 0;
    }
    CalculateXPToNextLevel();
}

void UMcpProgressionComponent::CalculateXPToNextLevel()
{
    if (XPCurve)
    {
        XPToNextLevel = FMath::RoundToInt(XPCurve->GetFloatValue(static_cast<float>(Level)));
    }
    else
    {
        // Default exponential curve
        XPToNextLevel = FMath::RoundToInt(100.0f * FMath::Pow(1.5f, Level - 1));
    }
}

void UMcpProgressionComponent::ProcessLevelUp()
{
    Level++;
    SkillPoints += SkillPointsPerLevel;
    CalculateXPToNextLevel();
    OnLevelUp.Broadcast(Level, SkillPointsPerLevel);
}

// ============================================================================
// Faction Component
// ============================================================================

UMcpFactionComponent::UMcpFactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UMcpFactionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpFactionComponent, FactionStandings);
}

void UMcpFactionComponent::ModifyReputation(FName FactionId, int32 Delta)
{
    if (FactionId.IsNone() || Delta == 0)
    {
        return;
    }

    FMcpFactionStanding& Standing = FactionStandings.FindOrAdd(FactionId);
    Standing.FactionId = FactionId;
    Standing.bDiscovered = true;

    int32 OldRep = Standing.Reputation;
    Standing.Reputation += Delta;

    OnReputationChanged.Broadcast(FactionId, Standing.Reputation, Delta);

    // Check for rank changes based on reputation thresholds
    EForge2FactionRank OldRank = GetReputationRank(OldRep);
    EForge2FactionRank NewRank = GetReputationRank(Standing.Reputation);
    
    if (NewRank != OldRank)
    {
        Standing.CurrentRank = NewRank;
        OnRankChanged.Broadcast(FactionId, OldRank, NewRank);
        
        // Grant rewards for rank up
        if ((int32)NewRank > (int32)OldRank)
        {
            OnFactionRankUp.Broadcast(FactionId, NewRank);
        }
    }
}

EForge2FactionRank UMcpFactionComponent::GetReputationRank(int32 Reputation) const
{
    // Standard reputation thresholds
    if (Reputation <= -6000) return EForge2FactionRank::Hated;
    if (Reputation <= -3000) return EForge2FactionRank::Hostile;
    if (Reputation <= -1000) return EForge2FactionRank::Unfriendly;
    if (Reputation < 1000) return EForge2FactionRank::Neutral;
    if (Reputation < 3000) return EForge2FactionRank::Friendly;
    if (Reputation < 6000) return EForge2FactionRank::Honored;
    if (Reputation < 12000) return EForge2FactionRank::Revered;
    return EForge2FactionRank::Exalted;
}

void UMcpFactionComponent::SetReputation(FName FactionId, int32 NewReputation)
{
    if (FactionId.IsNone())
    {
        return;
    }

    FMcpFactionStanding& Standing = FactionStandings.FindOrAdd(FactionId);
    int32 Delta = NewReputation - Standing.Reputation;
    Standing.FactionId = FactionId;
    Standing.Reputation = NewReputation;
    Standing.bDiscovered = true;

    OnReputationChanged.Broadcast(FactionId, Standing.Reputation, Delta);
}

int32 UMcpFactionComponent::GetReputation(FName FactionId) const
{
    const FMcpFactionStanding* Standing = FactionStandings.Find(FactionId);
    return Standing ? Standing->Reputation : 0;
}

FName UMcpFactionComponent::GetFactionRank(FName FactionId) const
{
    const FMcpFactionStanding* Standing = FactionStandings.Find(FactionId);
    return Standing ? Standing->CurrentRank : NAME_None;
}

bool UMcpFactionComponent::IsFactionDiscovered(FName FactionId) const
{
    const FMcpFactionStanding* Standing = FactionStandings.Find(FactionId);
    return Standing && Standing->bDiscovered;
}

void UMcpFactionComponent::DiscoverFaction(FName FactionId)
{
    if (FactionId.IsNone())
    {
        return;
    }

    FMcpFactionStanding& Standing = FactionStandings.FindOrAdd(FactionId);
    Standing.FactionId = FactionId;
    Standing.bDiscovered = true;
}

// ============================================================================
// Stealth Component
// ============================================================================

UMcpStealthComponent::UMcpStealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UMcpStealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpStealthComponent, Visibility);
    DOREPLIFETIME(UMcpStealthComponent, AlertState);
}

void UMcpStealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update visibility based on conditions
    UpdateVisibility();

    // Decay noise level
    if (NoiseLevel > 0)
    {
        NoiseLevel = FMath::Max(0.0f, NoiseLevel - 50.0f * DeltaTime);
    }

    // Decay suspicion when not in combat
    if (AlertState != EForge2AlertState::Combat && AlertState != EForge2AlertState::Alert)
    {
        if (SuspicionLevel > 0)
        {
            SuspicionLevel = FMath::Max(0.0f, SuspicionLevel - 10.0f * DeltaTime);
        }
    }
}

void UMcpStealthComponent::SetAlertState(EForge2AlertState NewState)
{
    if (AlertState != NewState)
    {
        EForge2AlertState OldState = AlertState;
        AlertState = NewState;
        OnAlertStateChanged.Broadcast(NewState, OldState);
    }
}

void UMcpStealthComponent::ModifySuspicion(float Delta)
{
    float OldSuspicion = SuspicionLevel;
    SuspicionLevel = FMath::Clamp(SuspicionLevel + Delta, 0.0f, 100.0f);

    // Update alert state based on suspicion
    if (SuspicionLevel >= 100.0f && AlertState < EForge2AlertState::Alert)
    {
        SetAlertState(EForge2AlertState::Alert);
        OnDetected.Broadcast(nullptr);
    }
    else if (SuspicionLevel >= 50.0f && AlertState < EForge2AlertState::Searching)
    {
        SetAlertState(EForge2AlertState::Searching);
    }
    else if (SuspicionLevel >= 25.0f && AlertState < EForge2AlertState::Suspicious)
    {
        SetAlertState(EForge2AlertState::Suspicious);
    }
    else if (SuspicionLevel <= 0.0f && AlertState != EForge2AlertState::Unaware)
    {
        SetAlertState(EForge2AlertState::Unaware);
        OnHidden.Broadcast();
    }
}

void UMcpStealthComponent::MakeNoise(float NoiseAmount, FVector Location)
{
    NoiseLevel = FMath::Max(NoiseLevel, NoiseAmount);
    
    // Notify nearby AI via perception system
    if (UWorld* World = GetWorld())
    {
        // Use UAIPerceptionSystem to report noise event
        UAIPerceptionSystem* PerceptionSys = UAIPerceptionSystem::GetCurrent(World);
        if (PerceptionSys)
        {
            // Create hearing stimulus
            FAIStimulus Stimulus(
                FAISenseAffiliationFilter::DetectAll,
                NoiseAmount,
                Location,
                GetOwner()->GetActorLocation()
            );
            Stimulus.Tag = TEXT("Noise");
            Stimulus.Strength = NoiseAmount / 100.0f;
            Stimulus.StimulusLocation = Location.IsZero() ? GetOwner()->GetActorLocation() : Location;
            
            // Report to perception system
            PerceptionSys->OnEvent(Stimulus);
        }
        
        // Also use gameplay ability system noise event if available
        if (AActor* Owner = GetOwner())
        {
            UAISense_Hearing::ReportNoiseEvent(
                World,
                Location.IsZero() ? Owner->GetActorLocation() : Location,
                NoiseAmount / 100.0f,
                Owner,
                NoiseAmount * 100.0f, // Max range
                NAME_None
            );
        }
    }
}

float UMcpStealthComponent::GetDetectionDifficulty() const
{
    float Difficulty = StealthRating;

    // Modifiers
    if (bIsCrouching)
    {
        Difficulty *= 1.5f;
    }
    if (bInShadow)
    {
        Difficulty *= 2.0f;
    }

    // Reduce by visibility
    Difficulty /= FMath::Max(Visibility, 0.1f);

    return Difficulty;
}

void UMcpStealthComponent::UpdateVisibility()
{
    // Base visibility
    float NewVisibility = 1.0f;

    // Reduce in shadow
    if (bInShadow)
    {
        NewVisibility *= 0.3f;
    }

    // Reduce when crouching
    if (bIsCrouching)
    {
        NewVisibility *= 0.7f;
    }

    Visibility = NewVisibility;
}

// ============================================================================
// Needs Component
// ============================================================================

UMcpNeedsComponent::UMcpNeedsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UMcpNeedsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMcpNeedsComponent, Needs);
}

void UMcpNeedsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bNeedsEnabled)
    {
        return;
    }

    for (auto& Pair : Needs)
    {
        FMcpSurvivalNeed& Need = Pair.Value;
        float OldValue = Need.CurrentValue;

        // Decay
        if (Need.DecayRate > 0)
        {
            Need.CurrentValue = FMath::Max(0.0f, Need.CurrentValue - Need.DecayRate * DeltaTime);
        }

        // Check for critical/depleted
        if (Need.CurrentValue <= 0.0f && OldValue > 0.0f)
        {
            OnNeedDepleted.Broadcast(Need.NeedId);
        }
        else if (Need.IsCritical() && OldValue > Need.CriticalThreshold)
        {
            OnNeedCritical.Broadcast(Need.NeedId);
        }

        // Notify if changed significantly
        if (FMath::Abs(Need.CurrentValue - OldValue) > 0.1f)
        {
            OnNeedChanged.Broadcast(Need.NeedId, Need.CurrentValue);
        }

        // Apply damage if depleted
        if (Need.bCausesDamage && Need.CurrentValue <= 0.0f)
        {
            // Apply damage to combat component
            if (AActor* Owner = GetOwner())
            {
                UMcpCombatComponent* CombatComp = Owner->FindComponentByClass<UMcpCombatComponent>();
                if (CombatComp)
                {
                    // Apply periodic damage based on need type
                    float DamageAmount = Need.DamagePerSecond * GetWorld()->GetDeltaSeconds();
                    
                    FMcpDamageInfo DamageInfo;
                    DamageInfo.BaseDamage = DamageAmount;
                    DamageInfo.DamageType = EForge2DamageType::Environmental;
                    DamageInfo.bBypassArmor = true;
                    DamageInfo.SourceNeedId = Need.NeedId;
                    
                    CombatComp->ApplyDamage(DamageInfo, nullptr);
                }
            }
        }
    }
}

void UMcpNeedsComponent::ModifyNeed(FName NeedId, float Delta)
{
    FMcpSurvivalNeed* Need = Needs.Find(NeedId);
    if (!Need)
    {
        return;
    }

    float OldValue = Need->CurrentValue;
    Need->CurrentValue = FMath::Clamp(Need->CurrentValue + Delta, 0.0f, Need->MaxValue);

    if (Need->CurrentValue != OldValue)
    {
        OnNeedChanged.Broadcast(NeedId, Need->CurrentValue);
    }
}

void UMcpNeedsComponent::SetNeed(FName NeedId, float NewValue)
{
    FMcpSurvivalNeed* Need = Needs.Find(NeedId);
    if (!Need)
    {
        return;
    }

    Need->CurrentValue = FMath::Clamp(NewValue, 0.0f, Need->MaxValue);
    OnNeedChanged.Broadcast(NeedId, Need->CurrentValue);
}

float UMcpNeedsComponent::GetNeedValue(FName NeedId) const
{
    const FMcpSurvivalNeed* Need = Needs.Find(NeedId);
    return Need ? Need->CurrentValue : 0.0f;
}

float UMcpNeedsComponent::GetNeedPercent(FName NeedId) const
{
    const FMcpSurvivalNeed* Need = Needs.Find(NeedId);
    return Need ? Need->GetPercent() : 0.0f;
}

bool UMcpNeedsComponent::IsNeedCritical(FName NeedId) const
{
    const FMcpSurvivalNeed* Need = Needs.Find(NeedId);
    return Need && Need->IsCritical();
}

void UMcpNeedsComponent::AddNeed(const FMcpSurvivalNeed& Need)
{
    Needs.Add(Need.NeedId, Need);
}

void UMcpNeedsComponent::SetDecayRate(FName NeedId, float NewRate)
{
    FMcpSurvivalNeed* Need = Needs.Find(NeedId);
    if (Need)
    {
        Need->DecayRate = NewRate;
    }
}
