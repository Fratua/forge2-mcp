// Forge2CardSystem.cpp - Card/Deck-Building Game Genre Implementation
#include "Forge2CardSystem.h"

// FForge2CardEffect Implementation
bool FForge2CardEffect::Evaluate(const FString& Context) const
{
    if (Condition.IsEmpty()) return true;
    
    // Parse and evaluate condition string
    // Format: "VARIABLE OPERATOR VALUE" e.g., "HEALTH < 50", "CARDS_IN_HAND >= 5", "HAS_KEYWORD Taunt"
    TArray<FString> Tokens;
    Condition.ParseIntoArrayWS(Tokens);
    
    if (Tokens.Num() < 2)
    {
        return true; // Invalid condition format, default to true
    }
    
    FString Variable = Tokens[0].ToUpper();
    
    // Handle special conditions
    if (Variable == TEXT("HAS_KEYWORD"))
    {
        if (Tokens.Num() >= 2)
        {
            // Check if context contains the keyword
            return Context.Contains(Tokens[1]);
        }
        return false;
    }
    
    if (Variable == TEXT("ALWAYS"))
    {
        return true;
    }
    
    if (Variable == TEXT("NEVER"))
    {
        return false;
    }
    
    // Parse comparison conditions
    if (Tokens.Num() >= 3)
    {
        FString Operator = Tokens[1];
        int32 CompareValue = FCString::Atoi(*Tokens[2]);
        
        // Extract variable value from context (format: "VAR=VALUE,VAR2=VALUE2,...")
        int32 VariableValue = 0;
        TArray<FString> ContextPairs;
        Context.ParseIntoArray(ContextPairs, TEXT(","));
        
        for (const FString& Pair : ContextPairs)
        {
            FString Key, Value;
            if (Pair.Split(TEXT("="), &Key, &Value))
            {
                if (Key.TrimStartAndEnd().ToUpper() == Variable)
                {
                    VariableValue = FCString::Atoi(*Value.TrimStartAndEnd());
                    break;
                }
            }
        }
        
        // Evaluate comparison
        if (Operator == TEXT("==") || Operator == TEXT("="))
        {
            return VariableValue == CompareValue;
        }
        else if (Operator == TEXT("!=") || Operator == TEXT("<>"))
        {
            return VariableValue != CompareValue;
        }
        else if (Operator == TEXT("<"))
        {
            return VariableValue < CompareValue;
        }
        else if (Operator == TEXT("<="))
        {
            return VariableValue <= CompareValue;
        }
        else if (Operator == TEXT(">"))
        {
            return VariableValue > CompareValue;
        }
        else if (Operator == TEXT(">="))
        {
            return VariableValue >= CompareValue;
        }
    }
    
    return true; // Default to true for unrecognized conditions
}

FString FForge2CardEffect::GetDescription() const
{
    FString Desc;

    if (EffectType == TEXT("Damage"))
    {
        Desc = FString::Printf(TEXT("Deal %d damage"), Value);
    }
    else if (EffectType == TEXT("Heal"))
    {
        Desc = FString::Printf(TEXT("Restore %d health"), Value);
    }
    else if (EffectType == TEXT("Draw"))
    {
        Desc = FString::Printf(TEXT("Draw %d card(s)"), Value);
    }
    else if (EffectType == TEXT("Buff"))
    {
        Desc = FString::Printf(TEXT("+%d/+%d"), Value, Duration);
    }

    return Desc;
}

// FForge2CardDefinition Implementation
bool FForge2CardDefinition::HasKeyword(const FString& Keyword) const
{
    return Keywords.Contains(Keyword);
}

// FForge2CardInstance Implementation
void FForge2CardInstance::Initialize(const FForge2CardDefinition& Def, int32 PlayerId)
{
    CardId = Def.CardId;
    Definition = Def;
    CurrentAttack = Def.Attack;
    CurrentHealth = Def.Health;
    CurrentDefense = Def.Defense;
    ModifiedCost = Def.ManaCost;
    OwnerPlayerId = PlayerId;
    bCanAttack = !Definition.HasKeyword(TEXT("Rush")) ? false : true;
    AttacksRemaining = Definition.HasKeyword(TEXT("Windfury")) ? 2 : 1;
}

void FForge2CardInstance::ResetForTurn()
{
    bCanAttack = true;
    bHasAttacked = false;
    AttacksRemaining = Definition.HasKeyword(TEXT("Windfury")) ? 2 : 1;

    if (bIsFrozen)
    {
        bIsFrozen = false;
        bCanAttack = false;
    }
}

void FForge2CardInstance::TakeDamage(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }
    
    // Check for Divine Shield keyword
    if (Definition.HasKeyword(TEXT("Divine Shield")) && bHasDivineShield)
    {
        // Remove divine shield instead of taking damage
        bHasDivineShield = false;
        
        // Remove the Divine Shield keyword from active buffs
        Buffs.Remove(TEXT("DivineShield"));
        
        // Shield absorbed all damage
        return;
    }
    
    // Check for damage reduction buffs
    int32 ActualDamage = Amount;
    
    // Apply armor/defense reduction if present
    if (CurrentDefense > 0)
    {
        int32 Blocked = FMath::Min(CurrentDefense, ActualDamage);
        ActualDamage -= Blocked;
        CurrentDefense -= Blocked;
    }
    
    // Apply damage
    CurrentHealth -= ActualDamage;
    
    // Track total damage taken for statistics
    TotalDamageTaken += ActualDamage;
}

void FForge2CardInstance::Heal(int32 Amount)
{
    CurrentHealth = FMath::Min(CurrentHealth + Amount, Definition.Health);
}

void FForge2CardInstance::ApplyBuff(const FString& BuffId, int32 AttackMod, int32 HealthMod)
{
    if (bIsSilenced) return;

    CurrentAttack += AttackMod;
    CurrentHealth += HealthMod;
    Buffs.Add(BuffId);
}

void FForge2CardInstance::Silence()
{
    bIsSilenced = true;
    Buffs.Empty();
    Debuffs.Empty();

    // Reset to base stats (keep damage taken)
    int32 DamageTaken = Definition.Health - CurrentHealth;
    CurrentAttack = Definition.Attack;
    CurrentHealth = Definition.Health - DamageTaken;
}

// FForge2DeckDefinition Implementation
int32 FForge2DeckDefinition::GetCardCount() const
{
    int32 Count = 0;
    for (const auto& Pair : Cards)
    {
        Count += Pair.Value;
    }
    return Count;
}

bool FForge2DeckDefinition::IsValid() const
{
    int32 Count = GetCardCount();
    return Count >= MinCards && Count <= MaxCards;
}

bool FForge2DeckDefinition::CanAddCard(const FString& CardId, const FForge2CardDefinition& Def) const
{
    if (GetCardCount() >= MaxCards) return false;

    const int32* CurrentCount = Cards.Find(CardId);
    int32 Count = CurrentCount ? *CurrentCount : 0;

    if (Def.Rarity == EForge2CardRarity::Legendary)
    {
        return Count < MaxLegendaries;
    }

    return Count < MaxCopies;
}

// FForge2ManaPool Implementation
void FForge2ManaPool::StartTurn()
{
    if (MaxMana < MaxPossibleMana)
    {
        MaxMana += ManaGainPerTurn;
        MaxMana = FMath::Min(MaxMana, MaxPossibleMana);
    }

    CurrentMana = MaxMana + TemporaryMana;
    TemporaryMana = 0;
}

void FForge2ManaPool::GainMana(int32 Amount)
{
    MaxMana = FMath::Min(MaxMana + Amount, MaxPossibleMana);
}

void FForge2ManaPool::SpendMana(int32 Amount)
{
    CurrentMana = FMath::Max(0, CurrentMana - Amount);
}

void FForge2ManaPool::AddTemporaryMana(int32 Amount)
{
    CurrentMana += Amount;
    TemporaryMana += Amount;
}

bool FForge2ManaPool::CanAfford(int32 Cost) const
{
    return CurrentMana >= Cost;
}

void FForge2ManaPool::Reset()
{
    CurrentMana = 0;
    MaxMana = 0;
    TemporaryMana = 0;
}

// FForge2PlayerState Implementation
void FForge2PlayerState::Initialize(const FString& Name, const FForge2DeckDefinition& DeckDef,
                                     const TMap<FString, FForge2CardDefinition>& CardDatabase)
{
    PlayerName = Name;
    Health = 30;
    MaxHealth = 30;
    Armor = 0;
    Mana.Reset();
    Deck.Empty();
    Hand.Empty();
    Field.Empty();
    Graveyard.Empty();
    FatigueDamage = 0;

    // Build deck from definition
    static int32 InstanceCounter = 1;
    for (const auto& Pair : DeckDef.Cards)
    {
        const FForge2CardDefinition* CardDef = CardDatabase.Find(Pair.Key);
        if (!CardDef) continue;

        for (int32 i = 0; i < Pair.Value; i++)
        {
            FForge2CardInstance Instance;
            Instance.InstanceId = InstanceCounter++;
            Instance.Initialize(*CardDef, PlayerId);
            Instance.Zone = EForge2CardZone::Deck;
            Deck.Add(Instance);
        }
    }

    ShuffleDeck();
}

void FForge2PlayerState::StartTurn()
{
    Mana.StartTurn();
    CardsDrawnThisTurn = 0;
    bHasPlayedCard = false;

    // Reset field creatures
    for (FForge2CardInstance& Card : Field)
    {
        Card.ResetForTurn();
    }

    DrawCard(1);
}

void FForge2PlayerState::EndTurn()
{
    // End of turn effects would trigger here
}

void FForge2PlayerState::DrawCard(int32 Count)
{
    for (int32 i = 0; i < Count; i++)
    {
        if (Deck.Num() == 0)
        {
            // Fatigue damage
            FatigueDamage++;
            TakeDamage(FatigueDamage);
            continue;
        }

        if (Hand.Num() >= 10)
        {
            // Hand is full, card is burned
            FForge2CardInstance Burned = Deck.Pop();
            Graveyard.Add(Burned);
            continue;
        }

        FForge2CardInstance DrawnCard = Deck.Pop();
        DrawnCard.Zone = EForge2CardZone::Hand;
        Hand.Add(DrawnCard);
        CardsDrawnThisTurn++;
    }
}

void FForge2PlayerState::ShuffleDeck()
{
    for (int32 i = Deck.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        Deck.Swap(i, j);
    }
}

bool FForge2PlayerState::PlayCard(int32 HandIndex, int32 FieldPosition)
{
    if (!Hand.IsValidIndex(HandIndex)) return false;

    FForge2CardInstance& Card = Hand[HandIndex];

    if (!Mana.CanAfford(Card.ModifiedCost)) return false;

    Mana.SpendMana(Card.ModifiedCost);

    Card.Zone = EForge2CardZone::Field;

    if (Card.Definition.CardType == EForge2CardType::Creature)
    {
        if (FieldPosition >= 0 && FieldPosition <= Field.Num())
        {
            Field.Insert(Card, FieldPosition);
        }
        else
        {
            Field.Add(Card);
        }
    }
    else
    {
        // Spell - goes to graveyard after resolution
        Graveyard.Add(Card);
    }

    Hand.RemoveAt(HandIndex);
    bHasPlayedCard = true;

    return true;
}

void FForge2PlayerState::TakeDamage(int32 Amount)
{
    // Armor absorbs first
    if (Armor > 0)
    {
        int32 Absorbed = FMath::Min(Armor, Amount);
        Armor -= Absorbed;
        Amount -= Absorbed;
    }

    Health -= Amount;
}

void FForge2PlayerState::GainArmor(int32 Amount)
{
    Armor += Amount;
}

void FForge2PlayerState::Heal(int32 Amount)
{
    Health = FMath::Min(Health + Amount, MaxHealth);
}

// FForge2CardGameState Implementation
void FForge2CardGameState::Initialize()
{
    CurrentPhase = EForge2GamePhase::DrawPhase;
    CurrentPlayerId = 0;
    TurnNumber = 1;
    TurnTimer = 0.0f;
    bGameOver = false;
    WinnerId = -1;
    Stack.Empty();
    GameLog.Empty();
}

void FForge2CardGameState::NextPhase()
{
    switch (CurrentPhase)
    {
        case EForge2GamePhase::DrawPhase:
            CurrentPhase = EForge2GamePhase::ManaPhase;
            break;
        case EForge2GamePhase::ManaPhase:
            CurrentPhase = EForge2GamePhase::MainPhase;
            break;
        case EForge2GamePhase::MainPhase:
            CurrentPhase = EForge2GamePhase::CombatPhase;
            break;
        case EForge2GamePhase::CombatPhase:
            CurrentPhase = EForge2GamePhase::EndPhase;
            break;
        case EForge2GamePhase::EndPhase:
            EndTurn();
            break;
    }
}

void FForge2CardGameState::EndTurn()
{
    CurrentPlayerId = (CurrentPlayerId + 1) % 2;
    if (CurrentPlayerId == 0)
    {
        TurnNumber++;
    }
    CurrentPhase = EForge2GamePhase::DrawPhase;
    TurnTimer = 0.0f;
}

void FForge2CardGameState::Log(const FString& Message)
{
    FString TimestampedMsg = FString::Printf(TEXT("[Turn %d] %s"), TurnNumber, *Message);
    GameLog.Add(TimestampedMsg);
}

// UForge2CardManager Implementation
UForge2CardManager::UForge2CardManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UForge2CardManager::Initialize(const FForge2CardGameConfig& InConfig)
{
    Config = InConfig;
    GameState.Initialize();
    Players.Empty();
    NextInstanceId = 1;
}

void UForge2CardManager::Update(float DeltaTime)
{
    if (GameState.bGameOver) return;

    GameState.TurnTimer += DeltaTime;

    // Auto-end turn if time limit exceeded
    if (Config.Rules.TurnTimeLimit > 0 && GameState.TurnTimer >= Config.Rules.TurnTimeLimit)
    {
        EndTurn();
    }
}

void UForge2CardManager::RegisterCard(const FForge2CardDefinition& Card)
{
    CardDatabase.Add(Card.CardId, Card);
}

void UForge2CardManager::AddPlayer(const FString& PlayerName, const FForge2DeckDefinition& Deck)
{
    if (Players.Num() >= 2) return;

    FForge2PlayerState Player;
    Player.PlayerId = Players.Num();
    Player.Initialize(PlayerName, Deck, CardDatabase);
    Players.Add(Player);
}

void UForge2CardManager::StartGame()
{
    if (Players.Num() < 2) return;

    GameState.Initialize();

    // Initial draw
    for (FForge2PlayerState& Player : Players)
    {
        Player.ShuffleDeck();
        Player.DrawCard(Config.Rules.StartingHandSize);
    }

    // Second player gets extra card (coin equivalent)
    if (Players.Num() > 1)
    {
        Players[1].DrawCard(1);
    }

    // Start first player's turn
    StartTurn(0);

    GameState.Log(TEXT("Game started!"));
}

void UForge2CardManager::RestartGame()
{
    // Reset players but keep deck configurations
    TArray<FForge2DeckDefinition> Decks;
    TArray<FString> Names;

    for (const FForge2PlayerState& Player : Players)
    {
        // Would need to store original deck def
        Names.Add(Player.PlayerName);
    }

    GameState.Initialize();
    Players.Empty();
    NextInstanceId = 1;
}

void UForge2CardManager::EndTurn()
{
    if (GameState.bGameOver) return;

    FForge2PlayerState* CurrentPlayer = GetPlayerMutable(GameState.CurrentPlayerId);
    if (CurrentPlayer)
    {
        CurrentPlayer->EndTurn();
    }

    GameState.EndTurn();

    // Start next player's turn
    StartTurn(GameState.CurrentPlayerId);
}

void UForge2CardManager::PassPriority()
{
    GameState.NextPhase();
}

void UForge2CardManager::StartTurn(int32 PlayerId)
{
    FForge2PlayerState* Player = GetPlayerMutable(PlayerId);
    if (!Player) return;

    Player->StartTurn();
    GameState.CurrentPhase = EForge2GamePhase::MainPhase;  // Simplified - skip draw/mana phases
    GameState.TurnTimer = 0.0f;

    GameState.Log(FString::Printf(TEXT("%s's turn (Turn %d)"), *Player->PlayerName, GameState.TurnNumber));
}

void UForge2CardManager::DrawPhase(int32 PlayerId)
{
    FForge2PlayerState* Player = GetPlayerMutable(PlayerId);
    if (Player)
    {
        Player->DrawCard(Config.Rules.CardsPerDraw);
    }
}

void UForge2CardManager::ManaPhase(int32 PlayerId)
{
    FForge2PlayerState* Player = GetPlayerMutable(PlayerId);
    if (Player)
    {
        Player->Mana.StartTurn();
    }
}

bool UForge2CardManager::PlayCard(int32 PlayerId, int32 HandIndex, int32 FieldPosition, int32 TargetId)
{
    if (GameState.CurrentPlayerId != PlayerId) return false;
    if (GameState.CurrentPhase != EForge2GamePhase::MainPhase) return false;

    FForge2PlayerState* Player = GetPlayerMutable(PlayerId);
    if (!Player) return false;
    if (!Player->Hand.IsValidIndex(HandIndex)) return false;

    FForge2CardInstance CardCopy = Player->Hand[HandIndex];

    if (!Player->Mana.CanAfford(CardCopy.ModifiedCost)) return false;

    // Check field size for creatures
    if (CardCopy.Definition.CardType == EForge2CardType::Creature)
    {
        if (Player->Field.Num() >= Config.Rules.MaxFieldSize) return false;
    }

    if (Player->PlayCard(HandIndex, FieldPosition))
    {
        // Resolve battlecry/on-play effects
        ResolveCard(CardCopy, TargetId);

        GameState.Log(FString::Printf(TEXT("%s played %s"),
            *Player->PlayerName, *CardCopy.Definition.CardName));

        ProcessDeaths();
        CheckWinCondition();

        return true;
    }

    return false;
}

bool UForge2CardManager::UseAbility(int32 PlayerId, int32 CardInstanceId, int32 TargetId)
{
    if (GameState.CurrentPlayerId != PlayerId) return false;

    FForge2CardInstance* Card = FindCard(CardInstanceId);
    if (!Card) return false;
    if (Card->OwnerPlayerId != PlayerId) return false;

    // Process ability effects
    for (const FForge2CardEffect& Effect : Card->Definition.Effects)
    {
        ResolveEffect(Effect, *Card, TargetId);
    }

    ProcessDeaths();
    CheckWinCondition();

    return true;
}

bool UForge2CardManager::Attack(int32 AttackerId, int32 DefenderId)
{
    FForge2CardInstance* Attacker = FindCard(AttackerId);
    if (!Attacker) return false;
    if (!Attacker->bCanAttack || Attacker->AttacksRemaining <= 0) return false;
    if (Attacker->bIsFrozen) return false;

    FForge2PlayerState* AttackerOwner = GetPlayerMutable(Attacker->OwnerPlayerId);
    if (!AttackerOwner) return false;

    if (GameState.CurrentPlayerId != Attacker->OwnerPlayerId) return false;

    // Check for taunt
    int32 OpponentId = (Attacker->OwnerPlayerId + 1) % 2;
    FForge2PlayerState* Opponent = GetPlayerMutable(OpponentId);
    if (!Opponent) return false;

    bool bHasTaunt = false;
    for (const FForge2CardInstance& Card : Opponent->Field)
    {
        if (Card.Definition.HasKeyword(TEXT("Taunt")) && !Card.bIsStealthed)
        {
            bHasTaunt = true;
            break;
        }
    }

    FForge2CardInstance* Defender = FindCard(DefenderId);

    if (Defender)
    {
        // Attacking a minion
        if (Defender->OwnerPlayerId == Attacker->OwnerPlayerId) return false;

        // Must attack taunt if one exists
        if (bHasTaunt && !Defender->Definition.HasKeyword(TEXT("Taunt"))) return false;

        // Can't attack stealthed minions
        if (Defender->bIsStealthed) return false;

        FForge2CombatResult Result = ResolveCombat(*Attacker, *Defender);

        GameState.Log(FString::Printf(TEXT("%s attacks %s"),
            *Attacker->Definition.CardName, *Defender->Definition.CardName));
    }
    else
    {
        // Attacking hero
        if (bHasTaunt) return false;  // Must attack taunt first

        Opponent->TakeDamage(Attacker->CurrentAttack);

        GameState.Log(FString::Printf(TEXT("%s attacks %s for %d damage"),
            *Attacker->Definition.CardName, *Opponent->PlayerName, Attacker->CurrentAttack));

        // Lifesteal
        if (Attacker->Definition.HasKeyword(TEXT("Lifesteal")))
        {
            AttackerOwner->Heal(Attacker->CurrentAttack);
        }
    }

    Attacker->AttacksRemaining--;
    Attacker->bHasAttacked = true;
    if (Attacker->AttacksRemaining <= 0)
    {
        Attacker->bCanAttack = false;
    }

    // Stealth breaks on attack
    Attacker->bIsStealthed = false;

    ProcessDeaths();
    CheckWinCondition();

    return true;
}

void UForge2CardManager::ResolveCard(FForge2CardInstance& Card, int32 TargetId)
{
    // Resolve battlecry/on-play effects
    for (const FForge2CardEffect& Effect : Card.Definition.TriggerEffects)
    {
        // Check if this is an on-play effect
        ResolveEffect(Effect, Card, TargetId);
    }
}

void UForge2CardManager::ResolveEffect(const FForge2CardEffect& Effect, FForge2CardInstance& Source, int32 TargetId)
{
    if (Effect.EffectType == TEXT("Damage"))
    {
        if (TargetId >= 0)
        {
            FForge2CardInstance* Target = FindCard(TargetId);
            if (Target)
            {
                Target->TakeDamage(Effect.Value);
            }
        }
    }
    else if (Effect.EffectType == TEXT("Heal"))
    {
        if (TargetId >= 0)
        {
            FForge2CardInstance* Target = FindCard(TargetId);
            if (Target)
            {
                Target->Heal(Effect.Value);
            }
        }
    }
    else if (Effect.EffectType == TEXT("Draw"))
    {
        FForge2PlayerState* Owner = GetPlayerMutable(Source.OwnerPlayerId);
        if (Owner)
        {
            Owner->DrawCard(Effect.Value);
        }
    }
    else if (Effect.EffectType == TEXT("Buff"))
    {
        if (TargetId >= 0)
        {
            FForge2CardInstance* Target = FindCard(TargetId);
            if (Target)
            {
                Target->ApplyBuff(Effect.EffectId, Effect.Value, Effect.Duration);
            }
        }
    }
}

FForge2CombatResult UForge2CardManager::ResolveCombat(FForge2CardInstance& Attacker, FForge2CardInstance& Defender)
{
    FForge2CombatResult Result;
    Result.AttackerId = Attacker.InstanceId;
    Result.DefenderId = Defender.InstanceId;
    Result.AttackerDamage = Defender.CurrentAttack;
    Result.DefenderDamage = Attacker.CurrentAttack;

    // Apply damage
    Attacker.TakeDamage(Result.AttackerDamage);
    Defender.TakeDamage(Result.DefenderDamage);

    Result.bAttackerDied = Attacker.IsDead();
    Result.bDefenderDied = Defender.IsDead();

    // Lifesteal
    if (Attacker.Definition.HasKeyword(TEXT("Lifesteal")))
    {
        FForge2PlayerState* Owner = GetPlayerMutable(Attacker.OwnerPlayerId);
        if (Owner)
        {
            Owner->Heal(Attacker.CurrentAttack);
        }
    }

    if (Defender.Definition.HasKeyword(TEXT("Lifesteal")))
    {
        FForge2PlayerState* Owner = GetPlayerMutable(Defender.OwnerPlayerId);
        if (Owner)
        {
            Owner->Heal(Defender.CurrentAttack);
        }
    }

    // Poisonous
    if (Attacker.Definition.HasKeyword(TEXT("Poisonous")))
    {
        Defender.CurrentHealth = 0;
        Result.bDefenderDied = true;
    }

    if (Defender.Definition.HasKeyword(TEXT("Poisonous")))
    {
        Attacker.CurrentHealth = 0;
        Result.bAttackerDied = true;
    }

    return Result;
}

void UForge2CardManager::ProcessDeaths()
{
    for (FForge2PlayerState& Player : Players)
    {
        for (int32 i = Player.Field.Num() - 1; i >= 0; i--)
        {
            if (Player.Field[i].IsDead())
            {
                FForge2CardInstance DeadCard = Player.Field[i];
                DeadCard.Zone = EForge2CardZone::Graveyard;

                // Deathrattle effects would trigger here

                Player.Graveyard.Add(DeadCard);
                Player.Field.RemoveAt(i);

                GameState.Log(FString::Printf(TEXT("%s died"), *DeadCard.Definition.CardName));
            }
        }
    }
}

void UForge2CardManager::CheckWinCondition()
{
    for (int32 i = 0; i < Players.Num(); i++)
    {
        if (Players[i].IsDead())
        {
            GameState.bGameOver = true;
            GameState.WinnerId = (i + 1) % Players.Num();
            GameState.Log(FString::Printf(TEXT("%s wins!"), *Players[GameState.WinnerId].PlayerName));
            return;
        }
    }
}

FForge2PlayerState UForge2CardManager::GetPlayer(int32 PlayerId) const
{
    if (Players.IsValidIndex(PlayerId))
    {
        return Players[PlayerId];
    }
    return FForge2PlayerState();
}

FForge2CardInstance UForge2CardManager::GetCard(int32 InstanceId) const
{
    for (const FForge2PlayerState& Player : Players)
    {
        for (const FForge2CardInstance& Card : Player.Hand)
        {
            if (Card.InstanceId == InstanceId) return Card;
        }
        for (const FForge2CardInstance& Card : Player.Field)
        {
            if (Card.InstanceId == InstanceId) return Card;
        }
    }
    return FForge2CardInstance();
}

TArray<FForge2CardInstance> UForge2CardManager::GetValidTargets(int32 CardInstanceId,
                                                                 EForge2CardTargetType TargetType) const
{
    TArray<FForge2CardInstance> Targets;

    FForge2CardInstance SourceCard = GetCard(CardInstanceId);
    if (SourceCard.InstanceId == 0) return Targets;

    int32 OwnerId = SourceCard.OwnerPlayerId;
    int32 OpponentId = (OwnerId + 1) % 2;

    switch (TargetType)
    {
        case EForge2CardTargetType::SingleEnemy:
            if (Players.IsValidIndex(OpponentId))
            {
                for (const FForge2CardInstance& Card : Players[OpponentId].Field)
                {
                    if (!Card.bIsStealthed)
                    {
                        Targets.Add(Card);
                    }
                }
            }
            break;

        case EForge2CardTargetType::SingleAlly:
            if (Players.IsValidIndex(OwnerId))
            {
                for (const FForge2CardInstance& Card : Players[OwnerId].Field)
                {
                    Targets.Add(Card);
                }
            }
            break;

        case EForge2CardTargetType::All:
            for (const FForge2PlayerState& Player : Players)
            {
                for (const FForge2CardInstance& Card : Player.Field)
                {
                    Targets.Add(Card);
                }
            }
            break;

        default:
            break;
    }

    return Targets;
}

bool UForge2CardManager::CanPlayCard(int32 PlayerId, int32 HandIndex) const
{
    if (!Players.IsValidIndex(PlayerId)) return false;

    const FForge2PlayerState& Player = Players[PlayerId];
    if (!Player.Hand.IsValidIndex(HandIndex)) return false;

    const FForge2CardInstance& Card = Player.Hand[HandIndex];
    if (!Player.Mana.CanAfford(Card.ModifiedCost)) return false;

    if (Card.Definition.CardType == EForge2CardType::Creature)
    {
        if (Player.Field.Num() >= Config.Rules.MaxFieldSize) return false;
    }

    return true;
}

FForge2DeckDefinition UForge2CardManager::CreateDeck(const FString& DeckName, const FString& HeroId)
{
    FForge2DeckDefinition Deck;
    Deck.DeckId = FString::Printf(TEXT("%s_%d"), *DeckName, FMath::RandRange(1000, 9999));
    Deck.DeckName = DeckName;
    Deck.HeroId = HeroId;
    return Deck;
}

bool UForge2CardManager::AddCardToDeck(FForge2DeckDefinition& Deck, const FString& CardId, int32 Count)
{
    const FForge2CardDefinition* CardDef = CardDatabase.Find(CardId);
    if (!CardDef) return false;

    for (int32 i = 0; i < Count; i++)
    {
        if (!Deck.CanAddCard(CardId, *CardDef)) return false;

        int32* CurrentCount = Deck.Cards.Find(CardId);
        if (CurrentCount)
        {
            (*CurrentCount)++;
        }
        else
        {
            Deck.Cards.Add(CardId, 1);
        }
    }

    return true;
}

bool UForge2CardManager::RemoveCardFromDeck(FForge2DeckDefinition& Deck, const FString& CardId, int32 Count)
{
    int32* CurrentCount = Deck.Cards.Find(CardId);
    if (!CurrentCount) return false;

    *CurrentCount -= Count;
    if (*CurrentCount <= 0)
    {
        Deck.Cards.Remove(CardId);
    }

    return true;
}

bool UForge2CardManager::ValidateDeck(const FForge2DeckDefinition& Deck) const
{
    return Deck.IsValid();
}

FForge2CardInstance* UForge2CardManager::FindCard(int32 InstanceId)
{
    for (FForge2PlayerState& Player : Players)
    {
        for (FForge2CardInstance& Card : Player.Hand)
        {
            if (Card.InstanceId == InstanceId) return &Card;
        }
        for (FForge2CardInstance& Card : Player.Field)
        {
            if (Card.InstanceId == InstanceId) return &Card;
        }
    }
    return nullptr;
}

FForge2PlayerState* UForge2CardManager::GetPlayerMutable(int32 PlayerId)
{
    if (Players.IsValidIndex(PlayerId))
    {
        return &Players[PlayerId];
    }
    return nullptr;
}

// UForge2CardUtility Implementation
FForge2CardDefinition UForge2CardUtility::CreateCreatureCard(const FString& CardId, const FString& Name,
                                                              int32 Cost, int32 Attack, int32 Health)
{
    FForge2CardDefinition Card;
    Card.CardId = CardId;
    Card.CardName = Name;
    Card.CardType = EForge2CardType::Creature;
    Card.ManaCost = Cost;
    Card.Attack = Attack;
    Card.Health = Health;
    Card.Rarity = EForge2CardRarity::Common;
    Card.DustCost = 40;
    return Card;
}

FForge2CardDefinition UForge2CardUtility::CreateSpellCard(const FString& CardId, const FString& Name,
                                                           int32 Cost, const FForge2CardEffect& Effect)
{
    FForge2CardDefinition Card;
    Card.CardId = CardId;
    Card.CardName = Name;
    Card.CardType = EForge2CardType::Spell;
    Card.ManaCost = Cost;
    Card.Rarity = EForge2CardRarity::Common;
    Card.DustCost = 40;
    Card.Effects.Add(Effect);
    return Card;
}

FForge2CardEffect UForge2CardUtility::CreateDamageEffect(int32 Damage, EForge2CardTargetType Target)
{
    FForge2CardEffect Effect;
    Effect.EffectId = TEXT("Damage");
    Effect.EffectType = TEXT("Damage");
    Effect.Value = Damage;
    Effect.TargetType = Target;
    return Effect;
}

FForge2CardEffect UForge2CardUtility::CreateHealEffect(int32 Amount, EForge2CardTargetType Target)
{
    FForge2CardEffect Effect;
    Effect.EffectId = TEXT("Heal");
    Effect.EffectType = TEXT("Heal");
    Effect.Value = Amount;
    Effect.TargetType = Target;
    return Effect;
}

FForge2CardEffect UForge2CardUtility::CreateDrawEffect(int32 Count)
{
    FForge2CardEffect Effect;
    Effect.EffectId = TEXT("Draw");
    Effect.EffectType = TEXT("Draw");
    Effect.Value = Count;
    Effect.TargetType = EForge2CardTargetType::Self;
    return Effect;
}

FForge2CardEffect UForge2CardUtility::CreateBuffEffect(int32 AttackBuff, int32 HealthBuff, EForge2CardTargetType Target)
{
    FForge2CardEffect Effect;
    Effect.EffectId = TEXT("Buff");
    Effect.EffectType = TEXT("Buff");
    Effect.Value = AttackBuff;
    Effect.Duration = HealthBuff;
    Effect.TargetType = Target;
    return Effect;
}

FForge2CardGameRules UForge2CardUtility::CreateStandardRules()
{
    FForge2CardGameRules Rules;
    Rules.StartingHandSize = 3;
    Rules.MaxHandSize = 10;
    Rules.MaxFieldSize = 7;
    Rules.StartingHealth = 30;
    Rules.StartingMana = 0;
    Rules.MaxMana = 10;
    Rules.CardsPerDraw = 1;
    Rules.bCanAttackDirectly = true;
    Rules.bHasTaunt = true;
    Rules.bHasSummoningSickness = true;
    Rules.TurnTimeLimit = 75.0f;
    return Rules;
}

FForge2CardGameRules UForge2CardUtility::CreateRoguelikeRules()
{
    FForge2CardGameRules Rules;
    Rules.StartingHandSize = 5;
    Rules.MaxHandSize = 10;
    Rules.MaxFieldSize = 5;
    Rules.StartingHealth = 80;
    Rules.StartingMana = 3;
    Rules.MaxMana = 10;
    Rules.CardsPerDraw = 1;
    Rules.bCanAttackDirectly = true;
    Rules.bHasTaunt = true;
    Rules.bHasSummoningSickness = false;
    Rules.TurnTimeLimit = 0.0f;  // No time limit
    return Rules;
}

TArray<FString> UForge2CardUtility::GetRandomCards(const TMap<FString, FForge2CardDefinition>& Database,
                                                    int32 Count, EForge2CardRarity MinRarity)
{
    TArray<FString> Result;
    TArray<FString> ValidCards;

    for (const auto& Pair : Database)
    {
        if ((int32)Pair.Value.Rarity >= (int32)MinRarity)
        {
            ValidCards.Add(Pair.Key);
        }
    }

    if (ValidCards.Num() == 0) return Result;

    for (int32 i = 0; i < Count && ValidCards.Num() > 0; i++)
    {
        int32 Index = FMath::RandRange(0, ValidCards.Num() - 1);
        Result.Add(ValidCards[Index]);
    }

    return Result;
}
