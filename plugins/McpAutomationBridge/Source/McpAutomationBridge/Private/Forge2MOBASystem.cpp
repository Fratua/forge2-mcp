// Forge2MOBASystem.cpp - MOBA Genre Implementation
#include "Forge2MOBASystem.h"

// FForge2HeroAbility Implementation
float FForge2HeroAbility::GetDamage(int32 Rank, float AD, float AP) const
{
    int32 Index = FMath::Clamp(Rank - 1, 0, BaseDamage.Num() - 1);
    float Base = BaseDamage.IsValidIndex(Index) ? BaseDamage[Index] : 0.0f;
    return Base + (AD * ADScaling) + (AP * APScaling);
}

float FForge2HeroAbility::GetManaCost(int32 Rank) const
{
    int32 Index = FMath::Clamp(Rank - 1, 0, ManaCost.Num() - 1);
    return ManaCost.IsValidIndex(Index) ? ManaCost[Index] : 0.0f;
}

float FForge2HeroAbility::GetCooldown(int32 Rank) const
{
    int32 Index = FMath::Clamp(Rank - 1, 0, Cooldown.Num() - 1);
    return Cooldown.IsValidIndex(Index) ? Cooldown[Index] : 0.0f;
}

// FForge2HeroState Implementation
void FForge2HeroState::Initialize(const FForge2HeroDefinition& Definition)
{
    HeroId = Definition.HeroId;
    CurrentHealth = Definition.BaseHealth;
    MaxHealth = Definition.BaseHealth;
    CurrentMana = Definition.BaseMana;
    MaxMana = Definition.BaseMana;
    AttackDamage = Definition.BaseAttackDamage;
    AbilityPower = 0.0f;
    Armor = Definition.BaseArmor;
    MagicResist = Definition.BaseMagicResist;
    MoveSpeed = Definition.BaseMoveSpeed;
    AttackSpeed = Definition.BaseAttackSpeed;
    Level = 1;
    Experience = 0.0f;
    Gold = 500;
    AbilityRanks.SetNum(4);
    AbilityCooldowns.SetNum(4);
    for (int32 i = 0; i < 4; i++)
    {
        AbilityRanks[i] = 0;
        AbilityCooldowns[i] = 0.0f;
    }
    bIsDead = false;
    DeathTimer = 0.0f;
}

void FForge2HeroState::Update(float DeltaTime, const FForge2HeroDefinition& Definition)
{
    if (bIsDead)
    {
        DeathTimer -= DeltaTime;
        if (DeathTimer <= 0.0f)
        {
            Respawn(Definition);
        }
        return;
    }

    // Update cooldowns
    for (int32 i = 0; i < AbilityCooldowns.Num(); i++)
    {
        if (AbilityCooldowns[i] > 0.0f)
        {
            AbilityCooldowns[i] -= DeltaTime;
            if (AbilityCooldowns[i] < 0.0f)
            {
                AbilityCooldowns[i] = 0.0f;
            }
        }
    }

    // Health and mana regeneration
    float HealthRegen = 5.0f + (Level * 0.5f);
    float ManaRegen = 3.0f + (Level * 0.3f);

    CurrentHealth = FMath::Min(CurrentHealth + HealthRegen * DeltaTime, MaxHealth);
    CurrentMana = FMath::Min(CurrentMana + ManaRegen * DeltaTime, MaxMana);
}

void FForge2HeroState::LevelUp(const FForge2HeroDefinition& Definition)
{
    if (Level >= 18) return;

    Level++;
    MaxHealth += Definition.HealthPerLevel;
    CurrentHealth += Definition.HealthPerLevel;
    MaxMana += Definition.ManaPerLevel;
    CurrentMana += Definition.ManaPerLevel;
    AttackDamage += Definition.AttackDamagePerLevel;
    Armor += Definition.ArmorPerLevel;
    MagicResist += Definition.MagicResistPerLevel;
    SkillPoints++;
}

void FForge2HeroState::GainExperience(float Amount, const FForge2HeroDefinition& Definition)
{
    Experience += Amount;

    float XPRequired = GetExperienceForLevel(Level + 1);
    while (Experience >= XPRequired && Level < 18)
    {
        Experience -= XPRequired;
        LevelUp(Definition);
        XPRequired = GetExperienceForLevel(Level + 1);
    }
}

float FForge2HeroState::GetExperienceForLevel(int32 TargetLevel) const
{
    if (TargetLevel <= 1) return 0.0f;
    return 100.0f + (TargetLevel - 1) * 80.0f;
}

bool FForge2HeroState::CanUseAbility(int32 AbilityIndex) const
{
    if (bIsDead) return false;
    if (AbilityIndex < 0 || AbilityIndex >= AbilityRanks.Num()) return false;
    if (AbilityRanks[AbilityIndex] <= 0) return false;
    if (AbilityCooldowns[AbilityIndex] > 0.0f) return false;
    return true;
}

void FForge2HeroState::UseAbility(int32 AbilityIndex, const FForge2HeroAbility& Ability)
{
    if (!CanUseAbility(AbilityIndex)) return;

    int32 Rank = AbilityRanks[AbilityIndex];
    float ManaCost = Ability.GetManaCost(Rank);

    if (CurrentMana < ManaCost) return;

    CurrentMana -= ManaCost;
    AbilityCooldowns[AbilityIndex] = Ability.GetCooldown(Rank);
}

void FForge2HeroState::TakeDamage(float PhysicalDamage, float MagicDamage)
{
    float PhysicalReduction = Armor / (100.0f + Armor);
    float MagicReduction = MagicResist / (100.0f + MagicResist);

    float ActualPhysical = PhysicalDamage * (1.0f - PhysicalReduction);
    float ActualMagic = MagicDamage * (1.0f - MagicReduction);

    CurrentHealth -= (ActualPhysical + ActualMagic);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

void FForge2HeroState::Die()
{
    bIsDead = true;
    DeathTimer = 10.0f + (Level * 2.0f);
    Deaths++;
}

void FForge2HeroState::Respawn(const FForge2HeroDefinition& Definition)
{
    bIsDead = false;
    CurrentHealth = MaxHealth;
    CurrentMana = MaxMana;
    Position = SpawnLocation;
}

// FForge2MinionWave Implementation
void FForge2MinionWave::Initialize(int32 InTeam, const FForge2LaneConfig& Lane, float GameTime)
{
    Team = InTeam;
    LaneIndex = Lane.LaneIndex;
    SpawnTime = GameTime;
    bWaveComplete = false;

    // Add minions based on wave type
    for (int32 i = 0; i < 3; i++)
    {
        FForge2MinionState Melee;
        Melee.MinionId = FString::Printf(TEXT("Melee_%d_%d_%d"), Team, LaneIndex, i);
        Melee.Type = EForge2MinionType::Melee;
        Melee.Team = Team;
        Melee.CurrentHealth = 450.0f;
        Melee.MaxHealth = 450.0f;
        Melee.AttackDamage = 12.0f;
        Melee.Armor = 0.0f;
        Melee.MoveSpeed = 325.0f;
        Melee.GoldValue = 21;
        Melee.ExperienceValue = 30.0f;
        Melee.bIsDead = false;
        Minions.Add(Melee);
    }

    for (int32 i = 0; i < 3; i++)
    {
        FForge2MinionState Caster;
        Caster.MinionId = FString::Printf(TEXT("Caster_%d_%d_%d"), Team, LaneIndex, i);
        Caster.Type = EForge2MinionType::Caster;
        Caster.Team = Team;
        Caster.CurrentHealth = 280.0f;
        Caster.MaxHealth = 280.0f;
        Caster.AttackDamage = 23.0f;
        Caster.Armor = 0.0f;
        Caster.MoveSpeed = 325.0f;
        Caster.GoldValue = 14;
        Caster.ExperienceValue = 20.0f;
        Caster.bIsDead = false;
        Minions.Add(Caster);
    }
}

void FForge2MinionWave::Update(float DeltaTime, const TArray<FVector>& Waypoints)
{
    int32 AliveCount = 0;

    for (FForge2MinionState& Minion : Minions)
    {
        if (!Minion.bIsDead)
        {
            AliveCount++;

            // Move along waypoints
            if (Minion.CurrentWaypointIndex < Waypoints.Num())
            {
                FVector Target = Waypoints[Minion.CurrentWaypointIndex];
                FVector Direction = (Target - Minion.Position).GetSafeNormal();
                Minion.Position += Direction * Minion.MoveSpeed * DeltaTime;

                if (FVector::Dist(Minion.Position, Target) < 50.0f)
                {
                    Minion.CurrentWaypointIndex++;
                }
            }
        }
    }

    bWaveComplete = (AliveCount == 0);
}

int32 FForge2MinionWave::GetAliveCount() const
{
    int32 Count = 0;
    for (const FForge2MinionState& Minion : Minions)
    {
        if (!Minion.bIsDead) Count++;
    }
    return Count;
}

// FForge2TowerState Implementation
void FForge2TowerState::Initialize(const FForge2TowerConfig& Config)
{
    TowerId = Config.TowerId;
    Position = Config.Position;
    CurrentHealth = Config.MaxHealth;
    MaxHealth = Config.MaxHealth;
    AttackDamage = Config.AttackDamage;
    AttackSpeed = Config.AttackSpeed;
    AttackRange = Config.AttackRange;
    Team = Config.Team;
    Type = Config.Type;
    bIsDestroyed = false;
    bIsInvulnerable = Config.bInvulnerableUntilPreviousDestroyed;
    GoldReward = Config.GoldReward;
    AttackCooldown = 0.0f;
}

void FForge2TowerState::Update(float DeltaTime, const TArray<FForge2MinionState>& EnemyMinions,
                                const TArray<FForge2HeroState>& EnemyHeroes)
{
    if (bIsDestroyed || bIsInvulnerable) return;

    AttackCooldown -= DeltaTime;

    if (AttackCooldown <= 0.0f)
    {
        // Find target - prioritize minions over heroes
        CurrentTargetId.Empty();
        float ClosestDist = AttackRange;

        for (const FForge2MinionState& Minion : EnemyMinions)
        {
            if (!Minion.bIsDead)
            {
                float Dist = FVector::Dist(Position, Minion.Position);
                if (Dist < ClosestDist)
                {
                    ClosestDist = Dist;
                    CurrentTargetId = Minion.MinionId;
                }
            }
        }

        if (CurrentTargetId.IsEmpty())
        {
            for (const FForge2HeroState& Hero : EnemyHeroes)
            {
                if (!Hero.bIsDead)
                {
                    float Dist = FVector::Dist(Position, Hero.Position);
                    if (Dist < ClosestDist)
                    {
                        ClosestDist = Dist;
                        CurrentTargetId = Hero.HeroId;
                    }
                }
            }
        }

        if (!CurrentTargetId.IsEmpty())
        {
            AttackCooldown = 1.0f / AttackSpeed;
        }
    }
}

void FForge2TowerState::TakeDamage(float Damage)
{
    if (bIsDestroyed || bIsInvulnerable) return;

    CurrentHealth -= Damage;
    if (CurrentHealth <= 0.0f)
    {
        CurrentHealth = 0.0f;
        bIsDestroyed = true;
    }
}

// FForge2JungleCamp Implementation
void FForge2JungleCamp::Initialize(const FForge2JungleCampConfig& Config)
{
    CampId = Config.CampId;
    Position = Config.Position;
    Type = Config.Type;
    Team = Config.Team;
    GoldReward = Config.GoldReward;
    ExperienceReward = Config.ExperienceReward;
    RespawnTime = Config.RespawnTime;
    BuffDuration = Config.BuffDuration;
    bIsAlive = true;
    bSpawnedThisGame = false;
    RespawnTimer = 0.0f;

    SpawnMonsters(Config);
}

void FForge2JungleCamp::SpawnMonsters(const FForge2JungleCampConfig& Config)
{
    Monsters.Empty();

    switch (Type)
    {
        case EForge2JungleCampType::SmallCamp:
            AddMonster(TEXT("Raptor"), 400.0f, 20.0f);
            AddMonster(TEXT("Raptor_Small"), 200.0f, 10.0f);
            AddMonster(TEXT("Raptor_Small"), 200.0f, 10.0f);
            break;
        case EForge2JungleCampType::MediumCamp:
            AddMonster(TEXT("Wolf_Large"), 600.0f, 30.0f);
            AddMonster(TEXT("Wolf"), 300.0f, 15.0f);
            AddMonster(TEXT("Wolf"), 300.0f, 15.0f);
            break;
        case EForge2JungleCampType::BuffCamp:
            AddMonster(TEXT("Buff_Monster"), 1800.0f, 80.0f);
            break;
        case EForge2JungleCampType::Dragon:
            AddMonster(TEXT("Dragon"), 4500.0f, 200.0f);
            break;
        case EForge2JungleCampType::Baron:
            AddMonster(TEXT("Baron"), 9000.0f, 400.0f);
            break;
        case EForge2JungleCampType::Rift:
            AddMonster(TEXT("Rift_Herald"), 6000.0f, 150.0f);
            break;
    }

    bIsAlive = true;
}

void FForge2JungleCamp::AddMonster(const FString& Name, float Health, float Damage)
{
    FForge2JungleMonster Monster;
    Monster.MonsterId = FString::Printf(TEXT("%s_%s"), *CampId, *Name);
    Monster.Name = Name;
    Monster.CurrentHealth = Health;
    Monster.MaxHealth = Health;
    Monster.AttackDamage = Damage;
    Monster.Position = Position + FMath::VRand() * 100.0f;
    Monster.bIsDead = false;
    Monsters.Add(Monster);
}

void FForge2JungleCamp::Update(float DeltaTime)
{
    if (!bIsAlive)
    {
        RespawnTimer -= DeltaTime;
        if (RespawnTimer <= 0.0f)
        {
            FForge2JungleCampConfig Config;
            Config.CampId = CampId;
            Config.Position = Position;
            Config.Type = Type;
            SpawnMonsters(Config);
        }
    }
    else
    {
        bool bAnyAlive = false;
        for (const FForge2JungleMonster& Monster : Monsters)
        {
            if (!Monster.bIsDead)
            {
                bAnyAlive = true;
                break;
            }
        }

        if (!bAnyAlive)
        {
            bIsAlive = false;
            RespawnTimer = RespawnTime;
        }
    }
}

// FForge2MOBAItemShop Implementation
void FForge2MOBAItemShop::Initialize()
{
    AvailableItems.Empty();

    // Starting items
    AddItem(TEXT("Dorans_Blade"), TEXT("Doran's Blade"), 450, EForge2MOBAItemCategory::Starting);
    AddItem(TEXT("Dorans_Ring"), TEXT("Doran's Ring"), 400, EForge2MOBAItemCategory::Starting);
    AddItem(TEXT("Dorans_Shield"), TEXT("Doran's Shield"), 450, EForge2MOBAItemCategory::Starting);

    // Boots
    AddItem(TEXT("Boots"), TEXT("Boots"), 300, EForge2MOBAItemCategory::Boots);
    AddItem(TEXT("Berserker_Greaves"), TEXT("Berserker's Greaves"), 1100, EForge2MOBAItemCategory::Boots);
    AddItem(TEXT("Sorcerer_Shoes"), TEXT("Sorcerer's Shoes"), 1100, EForge2MOBAItemCategory::Boots);

    // AD items
    AddItem(TEXT("BF_Sword"), TEXT("B.F. Sword"), 1300, EForge2MOBAItemCategory::Offensive);
    AddItem(TEXT("Infinity_Edge"), TEXT("Infinity Edge"), 3400, EForge2MOBAItemCategory::Offensive);

    // AP items
    AddItem(TEXT("Needlessly_Large_Rod"), TEXT("Needlessly Large Rod"), 1250, EForge2MOBAItemCategory::Offensive);
    AddItem(TEXT("Rabadons"), TEXT("Rabadon's Deathcap"), 3600, EForge2MOBAItemCategory::Offensive);

    // Defensive
    AddItem(TEXT("Chain_Vest"), TEXT("Chain Vest"), 800, EForge2MOBAItemCategory::Defensive);
    AddItem(TEXT("Thornmail"), TEXT("Thornmail"), 2700, EForge2MOBAItemCategory::Defensive);

    // Consumables
    AddItem(TEXT("Health_Potion"), TEXT("Health Potion"), 50, EForge2MOBAItemCategory::Consumable);
    AddItem(TEXT("Mana_Potion"), TEXT("Mana Potion"), 50, EForge2MOBAItemCategory::Consumable);
    AddItem(TEXT("Control_Ward"), TEXT("Control Ward"), 75, EForge2MOBAItemCategory::Consumable);
}

void FForge2MOBAItemShop::AddItem(const FString& Id, const FString& Name, int32 Cost, EForge2MOBAItemCategory Category)
{
    FForge2MOBAItem Item;
    Item.ItemId = Id;
    Item.ItemName = Name;
    Item.Cost = Cost;
    Item.SellValue = Cost * 0.7f;
    Item.Category = Category;

    // Set stats based on item
    if (Id == TEXT("Dorans_Blade"))
    {
        Item.AttackDamage = 8.0f;
        Item.Health = 80.0f;
    }
    else if (Id == TEXT("Dorans_Ring"))
    {
        Item.AbilityPower = 15.0f;
        Item.Mana = 70.0f;
    }
    else if (Id == TEXT("BF_Sword"))
    {
        Item.AttackDamage = 40.0f;
    }
    else if (Id == TEXT("Infinity_Edge"))
    {
        Item.AttackDamage = 70.0f;
        Item.CriticalChance = 0.25f;
    }
    else if (Id == TEXT("Needlessly_Large_Rod"))
    {
        Item.AbilityPower = 60.0f;
    }
    else if (Id == TEXT("Rabadons"))
    {
        Item.AbilityPower = 120.0f;
    }
    else if (Id == TEXT("Chain_Vest"))
    {
        Item.Armor = 40.0f;
    }
    else if (Id == TEXT("Thornmail"))
    {
        Item.Armor = 70.0f;
        Item.Health = 350.0f;
    }
    else if (Id == TEXT("Boots"))
    {
        Item.MoveSpeed = 25.0f;
    }
    else if (Id == TEXT("Berserker_Greaves"))
    {
        Item.MoveSpeed = 45.0f;
        Item.AttackSpeed = 0.35f;
    }
    else if (Id == TEXT("Sorcerer_Shoes"))
    {
        Item.MoveSpeed = 45.0f;
        Item.MagicPenetration = 18.0f;
    }

    AvailableItems.Add(Item);
}

bool FForge2MOBAItemShop::CanPurchase(const FString& ItemId, int32 Gold, const TArray<FString>& CurrentItems) const
{
    if (CurrentItems.Num() >= 6) return false;

    const FForge2MOBAItem* Item = GetItem(ItemId);
    if (!Item) return false;

    return Gold >= Item->Cost;
}

const FForge2MOBAItem* FForge2MOBAItemShop::GetItem(const FString& ItemId) const
{
    for (const FForge2MOBAItem& Item : AvailableItems)
    {
        if (Item.ItemId == ItemId)
        {
            return &Item;
        }
    }
    return nullptr;
}

TArray<FForge2MOBAItem> FForge2MOBAItemShop::GetItemsByCategory(EForge2MOBAItemCategory Category) const
{
    TArray<FForge2MOBAItem> Result;
    for (const FForge2MOBAItem& Item : AvailableItems)
    {
        if (Item.Category == Category)
        {
            Result.Add(Item);
        }
    }
    return Result;
}

// FForge2MOBAMatchState Implementation
void FForge2MOBAMatchState::Initialize()
{
    MatchTime = 0.0f;
    Phase = EForge2MOBAMatchPhase::Pregame;

    TeamKills.SetNum(2);
    TeamDeaths.SetNum(2);
    TeamGold.SetNum(2);
    TeamTowersDestroyed.SetNum(2);
    TeamDragonsKilled.SetNum(2);
    TeamBaronsKilled.SetNum(2);

    for (int32 i = 0; i < 2; i++)
    {
        TeamKills[i] = 0;
        TeamDeaths[i] = 0;
        TeamGold[i] = 0;
        TeamTowersDestroyed[i] = 0;
        TeamDragonsKilled[i] = 0;
        TeamBaronsKilled[i] = 0;
    }

    WinningTeam = -1;
    bMatchEnded = false;

    NextMinionWaveTime = 90.0f;
    MinionWaveInterval = 30.0f;

    bBaronSpawned = false;
    BaronSpawnTime = 20.0f * 60.0f;
    DragonRespawnTime = 5.0f * 60.0f;

    ItemShop.Initialize();
}

void FForge2MOBAMatchState::Update(float DeltaTime)
{
    if (bMatchEnded) return;

    MatchTime += DeltaTime;

    // Update phase based on time
    if (MatchTime < 90.0f)
    {
        Phase = EForge2MOBAMatchPhase::Pregame;
    }
    else if (MatchTime < 15.0f * 60.0f)
    {
        Phase = EForge2MOBAMatchPhase::EarlyGame;
    }
    else if (MatchTime < 25.0f * 60.0f)
    {
        Phase = EForge2MOBAMatchPhase::MidGame;
    }
    else
    {
        Phase = EForge2MOBAMatchPhase::LateGame;
    }

    // Check for Baron spawn
    if (!bBaronSpawned && MatchTime >= BaronSpawnTime)
    {
        bBaronSpawned = true;
    }
}

void FForge2MOBAMatchState::RegisterKill(int32 KillerTeam, int32 VictimTeam, int32 GoldReward)
{
    if (KillerTeam >= 0 && KillerTeam < TeamKills.Num())
    {
        TeamKills[KillerTeam]++;
        TeamGold[KillerTeam] += GoldReward;
    }
    if (VictimTeam >= 0 && VictimTeam < TeamDeaths.Num())
    {
        TeamDeaths[VictimTeam]++;
    }
}

void FForge2MOBAMatchState::RegisterTowerDestroyed(int32 Team)
{
    if (Team >= 0 && Team < TeamTowersDestroyed.Num())
    {
        TeamTowersDestroyed[Team]++;
    }
}

void FForge2MOBAMatchState::RegisterObjectiveKill(int32 Team, bool bIsBaron)
{
    if (Team >= 0 && Team < 2)
    {
        if (bIsBaron)
        {
            TeamBaronsKilled[Team]++;
        }
        else
        {
            TeamDragonsKilled[Team]++;
        }
    }
}

void FForge2MOBAMatchState::EndMatch(int32 Winner)
{
    WinningTeam = Winner;
    bMatchEnded = true;
}

// UForge2MOBAManager Implementation
UForge2MOBAManager::UForge2MOBAManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UForge2MOBAManager::Initialize(const FForge2MOBAConfig& InConfig)
{
    Config = InConfig;
    MatchState.Initialize();

    SetupLanes();
    SetupTowers();
    SetupJungle();
}

void UForge2MOBAManager::Update(float DeltaTime)
{
    MatchState.Update(DeltaTime);

    if (MatchState.bMatchEnded) return;

    UpdateHeroes(DeltaTime);
    UpdateMinions(DeltaTime);
    UpdateTowers(DeltaTime);
    UpdateJungle(DeltaTime);
    CheckWinCondition();
}

void UForge2MOBAManager::SetupLanes()
{
    Lanes.Empty();

    // Top lane
    FForge2LaneConfig Top;
    Top.LaneIndex = 0;
    Top.LaneName = TEXT("Top");
    Top.Team1Waypoints.Add(FVector(-10000, 10000, 0));
    Top.Team1Waypoints.Add(FVector(-10000, 0, 0));
    Top.Team1Waypoints.Add(FVector(0, 0, 0));
    Top.Team1Waypoints.Add(FVector(10000, 0, 0));
    Top.Team1Waypoints.Add(FVector(10000, -10000, 0));
    Top.Team2Waypoints = Top.Team1Waypoints;
    Algo::Reverse(Top.Team2Waypoints);
    Lanes.Add(Top);

    // Mid lane
    FForge2LaneConfig Mid;
    Mid.LaneIndex = 1;
    Mid.LaneName = TEXT("Mid");
    Mid.Team1Waypoints.Add(FVector(-10000, 10000, 0));
    Mid.Team1Waypoints.Add(FVector(0, 0, 0));
    Mid.Team1Waypoints.Add(FVector(10000, -10000, 0));
    Mid.Team2Waypoints = Mid.Team1Waypoints;
    Algo::Reverse(Mid.Team2Waypoints);
    Lanes.Add(Mid);

    // Bot lane
    FForge2LaneConfig Bot;
    Bot.LaneIndex = 2;
    Bot.LaneName = TEXT("Bot");
    Bot.Team1Waypoints.Add(FVector(-10000, 10000, 0));
    Bot.Team1Waypoints.Add(FVector(0, 10000, 0));
    Bot.Team1Waypoints.Add(FVector(0, 0, 0));
    Bot.Team1Waypoints.Add(FVector(0, -10000, 0));
    Bot.Team1Waypoints.Add(FVector(10000, -10000, 0));
    Bot.Team2Waypoints = Bot.Team1Waypoints;
    Algo::Reverse(Bot.Team2Waypoints);
    Lanes.Add(Bot);
}

void UForge2MOBAManager::SetupTowers()
{
    Towers.Empty();

    // Each team has 11 towers (3 per lane + 2 nexus)
    for (int32 Team = 0; Team < 2; Team++)
    {
        float TeamDir = (Team == 0) ? -1.0f : 1.0f;

        for (int32 Lane = 0; Lane < 3; Lane++)
        {
            for (int32 Tier = 0; Tier < 3; Tier++)
            {
                FForge2TowerConfig TowerConfig;
                TowerConfig.TowerId = FString::Printf(TEXT("Tower_T%d_L%d_Tier%d"), Team, Lane, Tier);
                TowerConfig.Team = Team;
                TowerConfig.Type = (EForge2TowerType)Tier;
                TowerConfig.MaxHealth = 3000.0f + (Tier * 500.0f);
                TowerConfig.AttackDamage = 150.0f + (Tier * 30.0f);
                TowerConfig.AttackSpeed = 0.833f;
                TowerConfig.AttackRange = 750.0f;
                TowerConfig.GoldReward = 250;
                TowerConfig.bInvulnerableUntilPreviousDestroyed = (Tier > 0);

                // Position based on lane and tier
                float LaneOffset = (Lane - 1) * 3000.0f;
                float TierDist = (Tier + 1) * 3000.0f;
                TowerConfig.Position = FVector(TeamDir * TierDist, LaneOffset, 0);

                FForge2TowerState TowerState;
                TowerState.Initialize(TowerConfig);
                Towers.Add(TowerState);
            }
        }

        // Nexus towers
        for (int32 i = 0; i < 2; i++)
        {
            FForge2TowerConfig NexusTower;
            NexusTower.TowerId = FString::Printf(TEXT("NexusTower_T%d_%d"), Team, i);
            NexusTower.Team = Team;
            NexusTower.Type = EForge2TowerType::Nexus;
            NexusTower.MaxHealth = 2500.0f;
            NexusTower.AttackDamage = 180.0f;
            NexusTower.AttackSpeed = 0.833f;
            NexusTower.AttackRange = 750.0f;
            NexusTower.GoldReward = 50;
            NexusTower.Position = FVector(TeamDir * 10000.0f, (i == 0 ? -500.0f : 500.0f), 0);

            FForge2TowerState TowerState;
            TowerState.Initialize(NexusTower);
            Towers.Add(TowerState);
        }
    }
}

void UForge2MOBAManager::SetupJungle()
{
    JungleCamps.Empty();

    // Add jungle camps for each side
    for (int32 Team = 0; Team < 2; Team++)
    {
        float TeamDir = (Team == 0) ? -1.0f : 1.0f;

        // Buff camps
        AddJungleCamp(FString::Printf(TEXT("Red_%d"), Team),
                      FVector(TeamDir * 5000, 2000, 0),
                      EForge2JungleCampType::BuffCamp, Team);
        AddJungleCamp(FString::Printf(TEXT("Blue_%d"), Team),
                      FVector(TeamDir * 5000, -2000, 0),
                      EForge2JungleCampType::BuffCamp, Team);

        // Small camps
        AddJungleCamp(FString::Printf(TEXT("Raptors_%d"), Team),
                      FVector(TeamDir * 4000, 0, 0),
                      EForge2JungleCampType::SmallCamp, Team);
        AddJungleCamp(FString::Printf(TEXT("Wolves_%d"), Team),
                      FVector(TeamDir * 6000, 0, 0),
                      EForge2JungleCampType::MediumCamp, Team);
    }

    // Neutral objectives
    AddJungleCamp(TEXT("Dragon"), FVector(0, -4000, 0), EForge2JungleCampType::Dragon, -1);
    AddJungleCamp(TEXT("Baron"), FVector(0, 4000, 0), EForge2JungleCampType::Baron, -1);
    AddJungleCamp(TEXT("Rift"), FVector(0, 4000, 0), EForge2JungleCampType::Rift, -1);
}

void UForge2MOBAManager::AddJungleCamp(const FString& CampId, const FVector& Position,
                                        EForge2JungleCampType Type, int32 Team)
{
    FForge2JungleCampConfig Config;
    Config.CampId = CampId;
    Config.Position = Position;
    Config.Type = Type;
    Config.Team = Team;

    switch (Type)
    {
        case EForge2JungleCampType::SmallCamp:
            Config.GoldReward = 80;
            Config.ExperienceReward = 115.0f;
            Config.RespawnTime = 120.0f;
            break;
        case EForge2JungleCampType::MediumCamp:
            Config.GoldReward = 100;
            Config.ExperienceReward = 140.0f;
            Config.RespawnTime = 120.0f;
            break;
        case EForge2JungleCampType::BuffCamp:
            Config.GoldReward = 120;
            Config.ExperienceReward = 200.0f;
            Config.RespawnTime = 300.0f;
            Config.BuffDuration = 120.0f;
            break;
        case EForge2JungleCampType::Dragon:
            Config.GoldReward = 25;
            Config.ExperienceReward = 75.0f;
            Config.RespawnTime = 300.0f;
            break;
        case EForge2JungleCampType::Baron:
            Config.GoldReward = 300;
            Config.ExperienceReward = 600.0f;
            Config.RespawnTime = 360.0f;
            Config.BuffDuration = 180.0f;
            break;
        case EForge2JungleCampType::Rift:
            Config.GoldReward = 100;
            Config.ExperienceReward = 200.0f;
            Config.RespawnTime = 0.0f; // Doesn't respawn
            break;
    }

    FForge2JungleCamp Camp;
    Camp.Initialize(Config);
    JungleCamps.Add(Camp);
}

void UForge2MOBAManager::UpdateHeroes(float DeltaTime)
{
    for (auto& Pair : HeroDefinitions)
    {
        FForge2HeroState* State = HeroStates.Find(Pair.Key);
        if (State)
        {
            State->Update(DeltaTime, Pair.Value);
        }
    }
}

void UForge2MOBAManager::UpdateMinions(float DeltaTime)
{
    // Check for new wave spawn
    if (MatchState.MatchTime >= MatchState.NextMinionWaveTime)
    {
        SpawnMinionWave();
        MatchState.NextMinionWaveTime += MatchState.MinionWaveInterval;
    }

    // Update existing waves
    for (FForge2MinionWave& Wave : MinionWaves)
    {
        if (!Wave.bWaveComplete)
        {
            TArray<FVector> Waypoints;
            if (Wave.LaneIndex < Lanes.Num())
            {
                Waypoints = (Wave.Team == 0) ?
                    Lanes[Wave.LaneIndex].Team1Waypoints :
                    Lanes[Wave.LaneIndex].Team2Waypoints;
            }
            Wave.Update(DeltaTime, Waypoints);
        }
    }

    // Remove completed waves
    MinionWaves.RemoveAll([](const FForge2MinionWave& Wave) { return Wave.bWaveComplete; });
}

void UForge2MOBAManager::SpawnMinionWave()
{
    for (int32 Team = 0; Team < 2; Team++)
    {
        for (int32 Lane = 0; Lane < Lanes.Num(); Lane++)
        {
            FForge2MinionWave Wave;
            Wave.Initialize(Team, Lanes[Lane], MatchState.MatchTime);
            MinionWaves.Add(Wave);
        }
    }
}

void UForge2MOBAManager::UpdateTowers(float DeltaTime)
{
    // Collect enemies for each team
    TArray<FForge2MinionState> Team0Minions, Team1Minions;
    TArray<FForge2HeroState> Team0Heroes, Team1Heroes;

    for (const FForge2MinionWave& Wave : MinionWaves)
    {
        for (const FForge2MinionState& Minion : Wave.Minions)
        {
            if (Wave.Team == 0)
                Team0Minions.Add(Minion);
            else
                Team1Minions.Add(Minion);
        }
    }

    for (const auto& Pair : HeroStates)
    {
        if (Pair.Value.Team == 0)
            Team0Heroes.Add(Pair.Value);
        else
            Team1Heroes.Add(Pair.Value);
    }

    for (FForge2TowerState& Tower : Towers)
    {
        if (!Tower.bIsDestroyed)
        {
            // Tower attacks enemies
            if (Tower.Team == 0)
                Tower.Update(DeltaTime, Team1Minions, Team1Heroes);
            else
                Tower.Update(DeltaTime, Team0Minions, Team0Heroes);
        }
    }
}

void UForge2MOBAManager::UpdateJungle(float DeltaTime)
{
    for (FForge2JungleCamp& Camp : JungleCamps)
    {
        Camp.Update(DeltaTime);
    }
}

void UForge2MOBAManager::CheckWinCondition()
{
    // Check if a nexus is destroyed
    for (const FForge2TowerState& Tower : Towers)
    {
        if (Tower.Type == EForge2TowerType::Nexus && Tower.bIsDestroyed)
        {
            // The team whose nexus was destroyed loses
            int32 Winner = (Tower.Team == 0) ? 1 : 0;
            MatchState.EndMatch(Winner);
            return;
        }
    }
}

FString UForge2MOBAManager::AddHero(const FForge2HeroDefinition& Definition)
{
    HeroDefinitions.Add(Definition.HeroId, Definition);

    FForge2HeroState State;
    State.Initialize(Definition);
    HeroStates.Add(Definition.HeroId, State);

    return Definition.HeroId;
}

bool UForge2MOBAManager::PurchaseItem(const FString& HeroId, const FString& ItemId)
{
    FForge2HeroState* State = HeroStates.Find(HeroId);
    if (!State) return false;

    if (!MatchState.ItemShop.CanPurchase(ItemId, State->Gold, State->Items))
        return false;

    const FForge2MOBAItem* Item = MatchState.ItemShop.GetItem(ItemId);
    if (!Item) return false;

    State->Gold -= Item->Cost;
    State->Items.Add(ItemId);

    // Apply stats
    State->AttackDamage += Item->AttackDamage;
    State->AbilityPower += Item->AbilityPower;
    State->Armor += Item->Armor;
    State->MagicResist += Item->MagicResist;
    State->MaxHealth += Item->Health;
    State->CurrentHealth += Item->Health;
    State->MaxMana += Item->Mana;
    State->CurrentMana += Item->Mana;
    State->MoveSpeed += Item->MoveSpeed;
    State->AttackSpeed += Item->AttackSpeed;

    return true;
}

// Forge2MOBAUtility Implementation
FForge2HeroDefinition UForge2MOBAUtility::CreateFighter(const FString& HeroId, const FString& HeroName)
{
    FForge2HeroDefinition Hero;
    Hero.HeroId = HeroId;
    Hero.HeroName = HeroName;
    Hero.Role = EForge2HeroRole::Fighter;

    Hero.BaseHealth = 580.0f;
    Hero.HealthPerLevel = 95.0f;
    Hero.BaseMana = 300.0f;
    Hero.ManaPerLevel = 40.0f;
    Hero.BaseAttackDamage = 60.0f;
    Hero.AttackDamagePerLevel = 3.5f;
    Hero.BaseArmor = 36.0f;
    Hero.ArmorPerLevel = 3.5f;
    Hero.BaseMagicResist = 32.0f;
    Hero.MagicResistPerLevel = 1.25f;
    Hero.BaseMoveSpeed = 345.0f;
    Hero.BaseAttackSpeed = 0.658f;
    Hero.AttackRange = 175.0f;

    return Hero;
}

FForge2HeroDefinition UForge2MOBAUtility::CreateMage(const FString& HeroId, const FString& HeroName)
{
    FForge2HeroDefinition Hero;
    Hero.HeroId = HeroId;
    Hero.HeroName = HeroName;
    Hero.Role = EForge2HeroRole::Mage;

    Hero.BaseHealth = 480.0f;
    Hero.HealthPerLevel = 80.0f;
    Hero.BaseMana = 450.0f;
    Hero.ManaPerLevel = 60.0f;
    Hero.BaseAttackDamage = 52.0f;
    Hero.AttackDamagePerLevel = 2.0f;
    Hero.BaseArmor = 22.0f;
    Hero.ArmorPerLevel = 3.0f;
    Hero.BaseMagicResist = 30.0f;
    Hero.MagicResistPerLevel = 0.5f;
    Hero.BaseMoveSpeed = 335.0f;
    Hero.BaseAttackSpeed = 0.625f;
    Hero.AttackRange = 550.0f;

    return Hero;
}

FForge2HeroDefinition UForge2MOBAUtility::CreateMarksman(const FString& HeroId, const FString& HeroName)
{
    FForge2HeroDefinition Hero;
    Hero.HeroId = HeroId;
    Hero.HeroName = HeroName;
    Hero.Role = EForge2HeroRole::Marksman;

    Hero.BaseHealth = 500.0f;
    Hero.HealthPerLevel = 85.0f;
    Hero.BaseMana = 300.0f;
    Hero.ManaPerLevel = 35.0f;
    Hero.BaseAttackDamage = 57.0f;
    Hero.AttackDamagePerLevel = 3.0f;
    Hero.BaseArmor = 26.0f;
    Hero.ArmorPerLevel = 3.0f;
    Hero.BaseMagicResist = 30.0f;
    Hero.MagicResistPerLevel = 0.5f;
    Hero.BaseMoveSpeed = 330.0f;
    Hero.BaseAttackSpeed = 0.680f;
    Hero.AttackRange = 550.0f;

    return Hero;
}

FForge2HeroDefinition UForge2MOBAUtility::CreateTank(const FString& HeroId, const FString& HeroName)
{
    FForge2HeroDefinition Hero;
    Hero.HeroId = HeroId;
    Hero.HeroName = HeroName;
    Hero.Role = EForge2HeroRole::Tank;

    Hero.BaseHealth = 640.0f;
    Hero.HealthPerLevel = 110.0f;
    Hero.BaseMana = 350.0f;
    Hero.ManaPerLevel = 45.0f;
    Hero.BaseAttackDamage = 55.0f;
    Hero.AttackDamagePerLevel = 2.5f;
    Hero.BaseArmor = 40.0f;
    Hero.ArmorPerLevel = 4.0f;
    Hero.BaseMagicResist = 36.0f;
    Hero.MagicResistPerLevel = 1.5f;
    Hero.BaseMoveSpeed = 335.0f;
    Hero.BaseAttackSpeed = 0.625f;
    Hero.AttackRange = 175.0f;

    return Hero;
}

FForge2HeroDefinition UForge2MOBAUtility::CreateSupport(const FString& HeroId, const FString& HeroName)
{
    FForge2HeroDefinition Hero;
    Hero.HeroId = HeroId;
    Hero.HeroName = HeroName;
    Hero.Role = EForge2HeroRole::Support;

    Hero.BaseHealth = 500.0f;
    Hero.HealthPerLevel = 75.0f;
    Hero.BaseMana = 400.0f;
    Hero.ManaPerLevel = 50.0f;
    Hero.BaseAttackDamage = 45.0f;
    Hero.AttackDamagePerLevel = 2.0f;
    Hero.BaseArmor = 28.0f;
    Hero.ArmorPerLevel = 3.0f;
    Hero.BaseMagicResist = 32.0f;
    Hero.MagicResistPerLevel = 0.75f;
    Hero.BaseMoveSpeed = 340.0f;
    Hero.BaseAttackSpeed = 0.625f;
    Hero.AttackRange = 500.0f;

    return Hero;
}

FForge2HeroAbility UForge2MOBAUtility::CreateDamageAbility(const FString& AbilityId,
                                                           float BaseDamage, float Scaling,
                                                           float ManaCost, float Cooldown)
{
    FForge2HeroAbility Ability;
    Ability.AbilityId = AbilityId;
    Ability.MaxRank = 5;

    for (int32 i = 0; i < 5; i++)
    {
        Ability.BaseDamage.Add(BaseDamage + (i * BaseDamage * 0.2f));
        Ability.ManaCost.Add(ManaCost + (i * 10.0f));
        Ability.Cooldown.Add(FMath::Max(Cooldown - (i * 1.0f), Cooldown * 0.5f));
    }

    Ability.APScaling = Scaling;
    Ability.Range = 700.0f;
    Ability.TargetType = EForge2AbilityTargetType::Enemy;

    return Ability;
}
