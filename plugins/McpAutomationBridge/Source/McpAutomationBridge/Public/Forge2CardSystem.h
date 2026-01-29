// Forge2CardSystem.h - Card/Deck-Building Game Genre System
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2CardSystem.generated.h"

// Forward declarations
class UForge2CardManager;

// Enums
UENUM(BlueprintType)
enum class EForge2CardType : uint8
{
    Creature,
    Spell,
    Artifact,
    Enchantment,
    Land,
    Action,
    Skill,
    Equipment,
    Trap,
    Hero
};

UENUM(BlueprintType)
enum class EForge2CardRarity : uint8
{
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    Mythic,
    Token
};

UENUM(BlueprintType)
enum class EForge2CardTargetType : uint8
{
    None,
    Self,
    SingleEnemy,
    SingleAlly,
    AllEnemies,
    AllAllies,
    All,
    RandomEnemy,
    RandomAlly,
    Hand,
    Deck,
    Discard
};

UENUM(BlueprintType)
enum class EForge2CardZone : uint8
{
    Deck,
    Hand,
    Field,
    Graveyard,
    Exile,
    Stack
};

UENUM(BlueprintType)
enum class EForge2GamePhase : uint8
{
    DrawPhase,
    ManaPhase,
    MainPhase,
    CombatPhase,
    EndPhase
};

UENUM(BlueprintType)
enum class EForge2CardGameMode : uint8
{
    Standard,
    Draft,
    Arena,
    Sealed,
    Commander,
    Roguelike
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CardEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString EffectId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString EffectType;  // Damage, Heal, Draw, Buff, etc.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2CardTargetType TargetType = EForge2CardTargetType::SingleEnemy;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Value = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Duration = 0;  // 0 = instant, -1 = permanent
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Condition;  // Conditional effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FString, FString> Parameters;

    bool Evaluate(const FString& Context) const;
    FString GetDescription() const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CardDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString CardId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString CardName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString FlavorText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2CardType CardType = EForge2CardType::Creature;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2CardRarity Rarity = EForge2CardRarity::Common;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ManaCost = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> ManaColors;  // For multi-color cards
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Attack = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Health = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Defense = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> Keywords;  // Taunt, Rush, Lifesteal, etc.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2CardEffect> Effects;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2CardEffect> TriggerEffects;  // On play, on death, etc.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Tribe;  // Beast, Dragon, Mech, etc.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> CardArt;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 DustCost = 100;  // Crafting cost

    bool HasKeyword(const FString& Keyword) const;
    int32 GetCMC() const { return ManaCost; }  // Converted mana cost
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CardInstance
{
    GENERATED_BODY()

    UPROPERTY() int32 InstanceId = 0;
    UPROPERTY() FString CardId;
    UPROPERTY() FForge2CardDefinition Definition;
    UPROPERTY() EForge2CardZone Zone = EForge2CardZone::Deck;
    UPROPERTY() int32 CurrentAttack = 0;
    UPROPERTY() int32 CurrentHealth = 0;
    UPROPERTY() int32 CurrentDefense = 0;
    UPROPERTY() int32 ModifiedCost = 0;
    UPROPERTY() TArray<FString> Buffs;
    UPROPERTY() TArray<FString> Debuffs;
    UPROPERTY() bool bCanAttack = false;
    UPROPERTY() bool bHasAttacked = false;
    UPROPERTY() bool bIsSilenced = false;
    UPROPERTY() bool bIsFrozen = false;
    UPROPERTY() bool bIsStealthed = false;
    UPROPERTY() int32 AttacksRemaining = 1;
    UPROPERTY() int32 OwnerPlayerId = 0;

    void Initialize(const FForge2CardDefinition& Def, int32 PlayerId);
    void ResetForTurn();
    void TakeDamage(int32 Amount);
    void Heal(int32 Amount);
    void ApplyBuff(const FString& BuffId, int32 AttackMod, int32 HealthMod);
    void Silence();
    bool IsDead() const { return CurrentHealth <= 0; }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DeckDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DeckId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DeckName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString HeroId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FString, int32> Cards;  // CardId -> Count
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinCards = 30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxCards = 30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxCopies = 2;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxLegendaries = 1;

    int32 GetCardCount() const;
    bool IsValid() const;
    bool CanAddCard(const FString& CardId, const FForge2CardDefinition& Def) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ManaPool
{
    GENERATED_BODY()

    UPROPERTY() int32 CurrentMana = 0;
    UPROPERTY() int32 MaxMana = 0;
    UPROPERTY() int32 TemporaryMana = 0;
    UPROPERTY() int32 ManaGainPerTurn = 1;
    UPROPERTY() int32 MaxPossibleMana = 10;
    UPROPERTY() TMap<FString, int32> ColoredMana;  // For multi-color systems

    void StartTurn();
    void GainMana(int32 Amount);
    void SpendMana(int32 Amount);
    void AddTemporaryMana(int32 Amount);
    bool CanAfford(int32 Cost) const;
    void Reset();
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlayerState
{
    GENERATED_BODY()

    UPROPERTY() int32 PlayerId = 0;
    UPROPERTY() FString PlayerName;
    UPROPERTY() int32 Health = 30;
    UPROPERTY() int32 MaxHealth = 30;
    UPROPERTY() int32 Armor = 0;
    UPROPERTY() FForge2ManaPool Mana;
    UPROPERTY() TArray<FForge2CardInstance> Deck;
    UPROPERTY() TArray<FForge2CardInstance> Hand;
    UPROPERTY() TArray<FForge2CardInstance> Field;
    UPROPERTY() TArray<FForge2CardInstance> Graveyard;
    UPROPERTY() FForge2CardInstance Hero;
    UPROPERTY() int32 FatigueDamage = 0;
    UPROPERTY() int32 CardsDrawnThisTurn = 0;
    UPROPERTY() bool bHasPlayedCard = false;

    void Initialize(const FString& Name, const FForge2DeckDefinition& DeckDef,
                    const TMap<FString, FForge2CardDefinition>& CardDatabase);
    void StartTurn();
    void EndTurn();
    void DrawCard(int32 Count = 1);
    void ShuffleDeck();
    bool PlayCard(int32 HandIndex, int32 FieldPosition = -1);
    void TakeDamage(int32 Amount);
    void GainArmor(int32 Amount);
    void Heal(int32 Amount);
    bool IsDead() const { return Health <= 0; }
    int32 GetHandSize() const { return Hand.Num(); }
    int32 GetFieldSize() const { return Field.Num(); }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CombatResult
{
    GENERATED_BODY()

    UPROPERTY() int32 AttackerId = 0;
    UPROPERTY() int32 DefenderId = 0;
    UPROPERTY() int32 AttackerDamage = 0;
    UPROPERTY() int32 DefenderDamage = 0;
    UPROPERTY() bool bAttackerDied = false;
    UPROPERTY() bool bDefenderDied = false;
    UPROPERTY() TArray<FString> TriggeredEffects;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CardGameRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 StartingHandSize = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxHandSize = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxFieldSize = 7;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 StartingHealth = 30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 StartingMana = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxMana = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CardsPerDraw = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanAttackDirectly = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasTaunt = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasSummoningSickness = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TurnTimeLimit = 90.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CardGameState
{
    GENERATED_BODY()

    UPROPERTY() EForge2GamePhase CurrentPhase = EForge2GamePhase::DrawPhase;
    UPROPERTY() int32 CurrentPlayerId = 0;
    UPROPERTY() int32 TurnNumber = 0;
    UPROPERTY() float TurnTimer = 0.0f;
    UPROPERTY() bool bGameOver = false;
    UPROPERTY() int32 WinnerId = -1;
    UPROPERTY() TArray<FForge2CardInstance> Stack;  // For spell resolution
    UPROPERTY() TArray<FString> GameLog;

    void Initialize();
    void NextPhase();
    void EndTurn();
    void Log(const FString& Message);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CardGameConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2CardGameRules Rules;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2CardGameMode GameMode = EForge2CardGameMode::Standard;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableAnimations = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AnimationSpeed = 1.0f;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2CardManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2CardManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2CardGameConfig Config;
    UPROPERTY(BlueprintReadOnly) FForge2CardGameState GameState;
    UPROPERTY() TArray<FForge2PlayerState> Players;
    UPROPERTY() TMap<FString, FForge2CardDefinition> CardDatabase;
    UPROPERTY() int32 NextInstanceId = 1;

    UFUNCTION(BlueprintCallable) void Initialize(const FForge2CardGameConfig& InConfig);
    UFUNCTION(BlueprintCallable) void Update(float DeltaTime);

    // Game setup
    UFUNCTION(BlueprintCallable) void RegisterCard(const FForge2CardDefinition& Card);
    UFUNCTION(BlueprintCallable) void AddPlayer(const FString& PlayerName, const FForge2DeckDefinition& Deck);
    UFUNCTION(BlueprintCallable) void StartGame();
    UFUNCTION(BlueprintCallable) void RestartGame();

    // Turn management
    UFUNCTION(BlueprintCallable) void EndTurn();
    UFUNCTION(BlueprintCallable) void PassPriority();

    // Card actions
    UFUNCTION(BlueprintCallable) bool PlayCard(int32 PlayerId, int32 HandIndex, int32 FieldPosition = -1,
                                                int32 TargetId = -1);
    UFUNCTION(BlueprintCallable) bool UseAbility(int32 PlayerId, int32 CardInstanceId, int32 TargetId = -1);
    UFUNCTION(BlueprintCallable) bool Attack(int32 AttackerId, int32 DefenderId);

    // Queries
    UFUNCTION(BlueprintPure) FForge2PlayerState GetPlayer(int32 PlayerId) const;
    UFUNCTION(BlueprintPure) FForge2CardInstance GetCard(int32 InstanceId) const;
    UFUNCTION(BlueprintPure) TArray<FForge2CardInstance> GetValidTargets(int32 CardInstanceId,
                                                                          EForge2CardTargetType TargetType) const;
    UFUNCTION(BlueprintPure) bool CanPlayCard(int32 PlayerId, int32 HandIndex) const;
    UFUNCTION(BlueprintPure) bool IsGameOver() const { return GameState.bGameOver; }

    // Deck building
    UFUNCTION(BlueprintCallable) FForge2DeckDefinition CreateDeck(const FString& DeckName, const FString& HeroId);
    UFUNCTION(BlueprintCallable) bool AddCardToDeck(FForge2DeckDefinition& Deck, const FString& CardId, int32 Count = 1);
    UFUNCTION(BlueprintCallable) bool RemoveCardFromDeck(FForge2DeckDefinition& Deck, const FString& CardId, int32 Count = 1);
    UFUNCTION(BlueprintCallable) bool ValidateDeck(const FForge2DeckDefinition& Deck) const;

private:
    void StartTurn(int32 PlayerId);
    void DrawPhase(int32 PlayerId);
    void ManaPhase(int32 PlayerId);
    void ResolveCard(FForge2CardInstance& Card, int32 TargetId);
    void ResolveEffect(const FForge2CardEffect& Effect, FForge2CardInstance& Source, int32 TargetId);
    FForge2CombatResult ResolveCombat(FForge2CardInstance& Attacker, FForge2CardInstance& Defender);
    void ProcessDeaths();
    void CheckWinCondition();
    FForge2CardInstance* FindCard(int32 InstanceId);
    FForge2PlayerState* GetPlayerMutable(int32 PlayerId);
    int32 GenerateInstanceId() { return NextInstanceId++; }
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2CardUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Card|Creation")
    static FForge2CardDefinition CreateCreatureCard(const FString& CardId, const FString& Name,
                                                     int32 Cost, int32 Attack, int32 Health);

    UFUNCTION(BlueprintCallable, Category = "Card|Creation")
    static FForge2CardDefinition CreateSpellCard(const FString& CardId, const FString& Name,
                                                  int32 Cost, const FForge2CardEffect& Effect);

    UFUNCTION(BlueprintCallable, Category = "Card|Effects")
    static FForge2CardEffect CreateDamageEffect(int32 Damage, EForge2CardTargetType Target);

    UFUNCTION(BlueprintCallable, Category = "Card|Effects")
    static FForge2CardEffect CreateHealEffect(int32 Amount, EForge2CardTargetType Target);

    UFUNCTION(BlueprintCallable, Category = "Card|Effects")
    static FForge2CardEffect CreateDrawEffect(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Card|Effects")
    static FForge2CardEffect CreateBuffEffect(int32 AttackBuff, int32 HealthBuff, EForge2CardTargetType Target);

    UFUNCTION(BlueprintCallable, Category = "Card|Rules")
    static FForge2CardGameRules CreateStandardRules();

    UFUNCTION(BlueprintCallable, Category = "Card|Rules")
    static FForge2CardGameRules CreateRoguelikeRules();

    UFUNCTION(BlueprintCallable, Category = "Card|Deck")
    static TArray<FString> GetRandomCards(const TMap<FString, FForge2CardDefinition>& Database,
                                          int32 Count, EForge2CardRarity MinRarity);
};
