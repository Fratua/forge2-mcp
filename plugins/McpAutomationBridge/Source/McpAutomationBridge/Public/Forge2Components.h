// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Runtime Actor Components for Game Systems

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Forge2DataAssets.h"
#include "Forge2Components.generated.h"

// Forward declarations
class UMcpQuestDataAsset;
class UMcpDialogueTreeAsset;
class UMcpItemDefinitionAsset;
class UMcpSkillDefinitionAsset;

// ============================================================================
// Quest Tracker Component
// Tracks quest progress for an actor (typically the player)
// ============================================================================

USTRUCT(BlueprintType)
struct FMcpActiveQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FName, int32> ObjectiveProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsTracked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStarted, FName, QuestId, UMcpQuestDataAsset*, QuestAsset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, FName, QuestId, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, FName, QuestId, FName, ObjectiveId, int32, NewProgress);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Quest Tracker")
class MCPAUTOMATIONBRIDGE_API UMcpQuestTrackerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpQuestTrackerComponent();

    // Active quests
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests", Replicated)
    TMap<FName, FMcpActiveQuest> ActiveQuests;

    // Completed quest IDs
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests", Replicated)
    TSet<FName> CompletedQuests;

    // Failed quest IDs
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests", Replicated)
    TSet<FName> FailedQuests;

    // Currently tracked quest
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests", Replicated)
    FName TrackedQuestId;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quests|Events")
    FOnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quests|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quests|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Quests")
    bool StartQuest(FName QuestId, UMcpQuestDataAsset* QuestAsset);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    bool UpdateObjective(FName QuestId, FName ObjectiveId, int32 ProgressDelta = 1);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    bool CompleteQuest(FName QuestId, bool bSuccess = true);

    UFUNCTION(BlueprintPure, Category = "Quests")
    bool HasCompletedQuest(FName QuestId) const;

    UFUNCTION(BlueprintPure, Category = "Quests")
    int32 GetObjectiveProgress(FName QuestId, FName ObjectiveId) const;

    UFUNCTION(BlueprintCallable, Category = "Quests")
    void TrackQuest(FName QuestId);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

// ============================================================================
// Dialogue Component
// Enables dialogue interaction for an actor (NPC or player)
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, AActor*, Initiator, UMcpDialogueTreeAsset*, DialogueTree);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueNodeReached, FName, NodeId, FText, NodeText);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Dialogue")
class MCPAUTOMATIONBRIDGE_API UMcpDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpDialogueComponent();

    // Default dialogue tree for this actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<UMcpDialogueTreeAsset> DefaultDialogueTree;

    // Currently active dialogue tree
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    UMcpDialogueTreeAsset* ActiveDialogueTree;

    // Current node in dialogue
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FName CurrentNodeId;

    // Whether in dialogue
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bInDialogue = false;

    // Dialogue variables (for conditions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<FName, FString> DialogueVariables;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
    FOnDialogueNodeReached OnDialogueNodeReached;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(AActor* Initiator, UMcpDialogueTreeAsset* DialogueTree = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueVariable(FName VariableName, const FString& Value);

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FString GetDialogueVariable(FName VariableName) const;

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool CanStartDialogue() const;
};

// ============================================================================
// Inventory Component
// Manages actor inventory with items, stacking, weight, etc.
// ============================================================================

USTRUCT(BlueprintType)
struct FMcpInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Durability = -1; // -1 means infinite

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FName, FString> CustomData;

    bool IsEmpty() const { return ItemId.IsNone() || Count <= 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, FName, ItemId, int32, Count, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemRemoved, FName, ItemId, int32, Count, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Inventory")
class MCPAUTOMATIONBRIDGE_API UMcpInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpInventoryComponent();

    // Inventory slots
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", Replicated)
    TArray<FMcpInventorySlot> Slots;

    // Max slots (-1 for unlimited)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxSlots = 40;

    // Max weight (-1 for unlimited)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    float MaxWeight = -1.0f;

    // Current weight
    UPROPERTY(BlueprintReadOnly, Category = "Inventory", Replicated)
    float CurrentWeight = 0.0f;

    // Currency (gold/coins)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", Replicated)
    TMap<FName, int64> Currencies;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemAdded OnItemAdded;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemRemoved OnItemRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryChanged OnInventoryChanged;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 AddItem(FName ItemId, int32 Count = 1, int32 PreferredSlot = -1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 RemoveItem(FName ItemId, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemFromSlot(int32 SlotIndex, int32 Count = 1);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemCount(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(FName ItemId, int32 MinCount = 1) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SwapSlots(int32 SlotA, int32 SlotB);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SortInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool ModifyCurrency(FName CurrencyId, int64 Delta);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int64 GetCurrency(FName CurrencyId) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool CanAddItem(FName ItemId, int32 Count = 1) const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    void RecalculateWeight();
};

// ============================================================================
// Combat Component
// Handles combat stats, damage dealing/receiving, and combat state
// ============================================================================

USTRUCT(BlueprintType)
struct FMcpCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxMana = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentMana = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Armor = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    TMap<EForge2DamageType, float> Resistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CriticalChance = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CriticalMultiplier = 2.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageTaken, AActor*, Instigator, float, Damage, EForge2DamageType, DamageType, bool, bWasCritical);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageDealt, AActor*, Target, float, Damage, EForge2DamageType, DamageType, bool, bWasCritical);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Combat")
class MCPAUTOMATIONBRIDGE_API UMcpCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpCombatComponent();

    // Combat stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", Replicated)
    FMcpCombatStats Stats;

    // Whether the actor is dead
    UPROPERTY(BlueprintReadOnly, Category = "Combat", Replicated)
    bool bIsDead = false;

    // Whether the actor is invincible
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInvincible = false;

    // Invincibility timer
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float InvincibilityRemaining = 0.0f;

    // Team ID for friendly fire
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", Replicated)
    int32 TeamId = 0;

    // Combat level (for scaling)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", Replicated)
    int32 CombatLevel = 1;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDamageDealt OnDamageDealt;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnRevived OnRevived;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnHealthChanged OnHealthChanged;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float ApplyDamage(AActor* Instigator, float BaseDamage, EForge2DamageType DamageType = EForge2DamageType::Physical, bool bCanCrit = true);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Heal(float Amount, bool bCanOverheal = false);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RestoreMana(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool SpendMana(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool SpendStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Kill(AActor* Killer = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Revive(float HealthPercent = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetInvincible(float Duration);

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlly(AActor* Other) const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    float CalculateDamageReduction(float Armor) const;
};

// ============================================================================
// Interaction Component
// Handles interaction prompts and logic
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteract, AActor*, Interactor, int32, InteractionIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionAvailable, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionUnavailable, AActor*, Interactor);

USTRUCT(BlueprintType)
struct FMcpInteractionOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText DisplayText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText ActionText; // "Press E to..."

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresHold = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta=(EditCondition="bRequiresHold"))
    float HoldDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FGameplayTagContainer RequiredTags;
};

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Interaction")
class MCPAUTOMATIONBRIDGE_API UMcpInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpInteractionComponent();

    // Available interactions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FMcpInteractionOption> Interactions;

    // Interaction distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance = 200.0f;

    // Whether to show UI prompt
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bShowPrompt = true;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnInteract OnInteract;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnInteractionAvailable OnInteractionAvailable;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnInteractionUnavailable OnInteractionUnavailable;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool TryInteract(AActor* Interactor, int32 InteractionIndex = 0);

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool CanInteract(AActor* Interactor, int32 InteractionIndex = 0) const;

    UFUNCTION(BlueprintPure, Category = "Interaction")
    TArray<int32> GetAvailableInteractions(AActor* Interactor) const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractionEnabled(int32 InteractionIndex, bool bEnabled);
};

// ============================================================================
// Status Effect Component
// Manages buffs, debuffs, and status effects
// ============================================================================

UENUM(BlueprintType)
enum class EForge2EffectType : uint8
{
    Buff        UMETA(DisplayName = "Buff"),
    Debuff      UMETA(DisplayName = "Debuff"),
    Neutral     UMETA(DisplayName = "Neutral")
};

USTRUCT(BlueprintType)
struct FMcpActiveStatusEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FName EffectId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EForge2EffectType EffectType = EForge2EffectType::Buff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float Duration = -1.0f; // -1 means permanent

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float RemainingTime = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    int32 Stacks = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    int32 MaxStacks = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    AActor* Source = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    TMap<FName, float> StatModifiers;

    bool IsPermanent() const { return Duration < 0.0f; }
    bool IsExpired() const { return !IsPermanent() && RemainingTime <= 0.0f; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectApplied, FName, EffectId, int32, Stacks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectRemoved, FName, EffectId, bool, bExpired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectStackChanged, FName, EffectId, int32, NewStacks);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Status Effects")
class MCPAUTOMATIONBRIDGE_API UMcpStatusEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpStatusEffectComponent();

    // Active effects
    UPROPERTY(BlueprintReadOnly, Category = "Effects", Replicated)
    TArray<FMcpActiveStatusEffect> ActiveEffects;

    // Immunity tags (effects with these tags won't be applied)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FGameplayTagContainer ImmunityTags;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
    FOnEffectApplied OnEffectApplied;

    UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
    FOnEffectRemoved OnEffectRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
    FOnEffectStackChanged OnEffectStackChanged;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Effects")
    bool ApplyEffect(const FMcpActiveStatusEffect& Effect);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    bool RemoveEffect(FName EffectId, bool bRemoveAllStacks = true);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void RemoveAllEffects(EForge2EffectType TypeFilter = EForge2EffectType::Neutral);

    UFUNCTION(BlueprintPure, Category = "Effects")
    bool HasEffect(FName EffectId) const;

    UFUNCTION(BlueprintPure, Category = "Effects")
    int32 GetEffectStacks(FName EffectId) const;

    UFUNCTION(BlueprintPure, Category = "Effects")
    float GetTotalStatModifier(FName StatName) const;

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void RefreshEffectDuration(FName EffectId, float NewDuration = -1.0f);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

// ============================================================================
// Equipment Component
// Manages equipped items in slots
// ============================================================================

UENUM(BlueprintType)
enum class EForge2EquipmentSlot : uint8
{
    None        UMETA(DisplayName = "None"),
    Head        UMETA(DisplayName = "Head"),
    Chest       UMETA(DisplayName = "Chest"),
    Hands       UMETA(DisplayName = "Hands"),
    Legs        UMETA(DisplayName = "Legs"),
    Feet        UMETA(DisplayName = "Feet"),
    Shoulders   UMETA(DisplayName = "Shoulders"),
    Back        UMETA(DisplayName = "Back"),
    Waist       UMETA(DisplayName = "Waist"),
    Wrist       UMETA(DisplayName = "Wrist"),
    Neck        UMETA(DisplayName = "Neck"),
    Ring1       UMETA(DisplayName = "Ring 1"),
    Ring2       UMETA(DisplayName = "Ring 2"),
    Trinket1    UMETA(DisplayName = "Trinket 1"),
    Trinket2    UMETA(DisplayName = "Trinket 2"),
    MainHand    UMETA(DisplayName = "Main Hand"),
    OffHand     UMETA(DisplayName = "Off Hand")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemEquipped, EForge2EquipmentSlot, Slot, FName, ItemId, FName, PreviousItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUnequipped, EForge2EquipmentSlot, Slot, FName, ItemId);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Equipment")
class MCPAUTOMATIONBRIDGE_API UMcpEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpEquipmentComponent();

    // Equipped items by slot
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment", Replicated)
    TMap<EForge2EquipmentSlot, FMcpInventorySlot> EquippedItems;

    // Total stats from equipment
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    TMap<EForge2StatType, float> EquipmentStats;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
    FOnItemEquipped OnItemEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
    FOnItemUnequipped OnItemUnequipped;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool EquipItem(EForge2EquipmentSlot Slot, const FMcpInventorySlot& Item);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    FMcpInventorySlot UnequipItem(EForge2EquipmentSlot Slot);

    UFUNCTION(BlueprintPure, Category = "Equipment")
    FMcpInventorySlot GetEquippedItem(EForge2EquipmentSlot Slot) const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool HasItemEquipped(EForge2EquipmentSlot Slot) const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    float GetEquipmentStat(EForge2StatType Stat) const;

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void RecalculateStats();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

// ============================================================================
// Progression Component
// Handles XP, levels, and skill points
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, SkillPointsGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPGained, int32, Amount, int32, TotalXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillUnlocked, FName, SkillId, int32, SkillRank);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Progression")
class MCPAUTOMATIONBRIDGE_API UMcpProgressionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpProgressionComponent();

    // Current level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", Replicated)
    int32 Level = 1;

    // Max level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 MaxLevel = 100;

    // Current XP
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", Replicated)
    int32 CurrentXP = 0;

    // XP required for next level
    UPROPERTY(BlueprintReadOnly, Category = "Progression")
    int32 XPToNextLevel = 100;

    // Total XP earned
    UPROPERTY(BlueprintReadOnly, Category = "Progression", Replicated)
    int32 TotalXPEarned = 0;

    // Available skill points
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", Replicated)
    int32 SkillPoints = 0;

    // Skill points per level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 SkillPointsPerLevel = 1;

    // Unlocked skills with ranks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", Replicated)
    TMap<FName, int32> UnlockedSkills;

    // XP curve type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    UCurveFloat* XPCurve;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
    FOnLevelUp OnLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
    FOnXPGained OnXPGained;

    UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
    FOnSkillUnlocked OnSkillUnlocked;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Progression")
    void AddXP(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    bool UnlockSkill(FName SkillId, int32 PointCost = 1);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    bool UpgradeSkill(FName SkillId, int32 PointCost = 1);

    UFUNCTION(BlueprintPure, Category = "Progression")
    int32 GetSkillRank(FName SkillId) const;

    UFUNCTION(BlueprintPure, Category = "Progression")
    bool HasSkill(FName SkillId, int32 MinRank = 1) const;

    UFUNCTION(BlueprintPure, Category = "Progression")
    float GetLevelProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void SetLevel(int32 NewLevel, bool bResetXP = true);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    void CalculateXPToNextLevel();
    void ProcessLevelUp();
};

// ============================================================================
// Faction Component
// Manages reputation and faction standings
// ============================================================================

USTRUCT(BlueprintType)
struct FMcpFactionStanding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    FName FactionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    int32 Reputation = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    FName CurrentRank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    bool bDiscovered = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReputationChanged, FName, FactionId, int32, NewReputation, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFactionRankChanged, FName, FactionId, FName, NewRank, FName, OldRank);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Faction")
class MCPAUTOMATIONBRIDGE_API UMcpFactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpFactionComponent();

    // Faction standings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faction", Replicated)
    TMap<FName, FMcpFactionStanding> FactionStandings;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Faction|Events")
    FOnReputationChanged OnReputationChanged;

    UPROPERTY(BlueprintAssignable, Category = "Faction|Events")
    FOnFactionRankChanged OnFactionRankChanged;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Faction")
    void ModifyReputation(FName FactionId, int32 Delta);

    UFUNCTION(BlueprintCallable, Category = "Faction")
    void SetReputation(FName FactionId, int32 NewReputation);

    UFUNCTION(BlueprintPure, Category = "Faction")
    int32 GetReputation(FName FactionId) const;

    UFUNCTION(BlueprintPure, Category = "Faction")
    FName GetFactionRank(FName FactionId) const;

    UFUNCTION(BlueprintPure, Category = "Faction")
    bool IsFactionDiscovered(FName FactionId) const;

    UFUNCTION(BlueprintCallable, Category = "Faction")
    void DiscoverFaction(FName FactionId);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

// ============================================================================
// Stealth Component
// Handles detection, visibility, and stealth state
// ============================================================================

UENUM(BlueprintType)
enum class EForge2AlertState : uint8
{
    Unaware     UMETA(DisplayName = "Unaware"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Searching   UMETA(DisplayName = "Searching"),
    Alert       UMETA(DisplayName = "Alert"),
    Combat      UMETA(DisplayName = "Combat")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAlertStateChanged, EForge2AlertState, NewState, EForge2AlertState, OldState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetected, AActor*, Detector);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHidden);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Stealth")
class MCPAUTOMATIONBRIDGE_API UMcpStealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpStealthComponent();

    // Current visibility (0 = invisible, 1 = fully visible)
    UPROPERTY(BlueprintReadOnly, Category = "Stealth", Replicated)
    float Visibility = 1.0f;

    // Current noise level
    UPROPERTY(BlueprintReadOnly, Category = "Stealth")
    float NoiseLevel = 0.0f;

    // Whether in shadow/darkness
    UPROPERTY(BlueprintReadOnly, Category = "Stealth")
    bool bInShadow = false;

    // Whether crouching (reduces visibility)
    UPROPERTY(BlueprintReadWrite, Category = "Stealth")
    bool bIsCrouching = false;

    // Current alert state (for AI)
    UPROPERTY(BlueprintReadOnly, Category = "Stealth", Replicated)
    EForge2AlertState AlertState = EForge2AlertState::Unaware;

    // Suspicion meter (0-100)
    UPROPERTY(BlueprintReadOnly, Category = "Stealth")
    float SuspicionLevel = 0.0f;

    // Base detection difficulty (higher = harder to detect)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
    float StealthRating = 1.0f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Stealth|Events")
    FOnAlertStateChanged OnAlertStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Stealth|Events")
    FOnDetected OnDetected;

    UPROPERTY(BlueprintAssignable, Category = "Stealth|Events")
    FOnHidden OnHidden;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Stealth")
    void SetAlertState(EForge2AlertState NewState);

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    void ModifySuspicion(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    void MakeNoise(float NoiseAmount, FVector Location);

    UFUNCTION(BlueprintPure, Category = "Stealth")
    float GetDetectionDifficulty() const;

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    void UpdateVisibility();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

// ============================================================================
// Needs Component
// Handles survival needs (hunger, thirst, temperature, etc.)
// ============================================================================

USTRUCT(BlueprintType)
struct FMcpSurvivalNeed
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    FName NeedId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    float CurrentValue = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    float MaxValue = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    float DecayRate = 1.0f; // Per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    float CriticalThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    bool bCausesDamage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need", meta=(EditCondition="bCausesDamage"))
    float DamagePerSecond = 1.0f;

    float GetPercent() const { return MaxValue > 0 ? CurrentValue / MaxValue : 0.0f; }
    bool IsCritical() const { return CurrentValue <= CriticalThreshold; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNeedChanged, FName, NeedId, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNeedCritical, FName, NeedId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNeedDepleted, FName, NeedId);

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent), DisplayName="MCP Needs")
class MCPAUTOMATIONBRIDGE_API UMcpNeedsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMcpNeedsComponent();

    // Survival needs
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs", Replicated)
    TMap<FName, FMcpSurvivalNeed> Needs;

    // Whether needs should tick
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    bool bNeedsEnabled = true;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Needs|Events")
    FOnNeedChanged OnNeedChanged;

    UPROPERTY(BlueprintAssignable, Category = "Needs|Events")
    FOnNeedCritical OnNeedCritical;

    UPROPERTY(BlueprintAssignable, Category = "Needs|Events")
    FOnNeedDepleted OnNeedDepleted;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void ModifyNeed(FName NeedId, float Delta);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void SetNeed(FName NeedId, float NewValue);

    UFUNCTION(BlueprintPure, Category = "Needs")
    float GetNeedValue(FName NeedId) const;

    UFUNCTION(BlueprintPure, Category = "Needs")
    float GetNeedPercent(FName NeedId) const;

    UFUNCTION(BlueprintPure, Category = "Needs")
    bool IsNeedCritical(FName NeedId) const;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void AddNeed(const FMcpSurvivalNeed& Need);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void SetDecayRate(FName NeedId, float NewRate);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
