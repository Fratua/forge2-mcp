// Forge2BattleRoyaleSystem.cpp - Battle Royale Genre Implementation
#include "Forge2BattleRoyaleSystem.h"

// FForge2BRZoneState Implementation
void FForge2BRZoneState::Initialize(const FForge2BRZoneConfig& Config, const FVector& MapCenter)
{
    CurrentCenter = MapCenter;
    NextCenter = MapCenter;
    CurrentRadius = Config.InitialRadius;
    TargetRadius = Config.InitialRadius;
    CurrentPhase = 0;
    PhaseTimer = Config.WaitTimeBeforeFirstShrink;
    bIsShrinking = false;
    CurrentDamage = Config.DamagePerSecond.Num() > 0 ? Config.DamagePerSecond[0] : 1.0f;
}

void FForge2BRZoneState::Update(float DeltaTime, const FForge2BRZoneConfig& Config)
{
    PhaseTimer -= DeltaTime;

    if (PhaseTimer <= 0.0f)
    {
        if (bIsShrinking)
        {
            // Finished shrinking, start wait phase
            bIsShrinking = false;
            CurrentRadius = TargetRadius;
            CurrentCenter = NextCenter;
            CurrentPhase++;

            if (CurrentPhase < Config.NumberOfPhases)
            {
                PhaseTimer = Config.PhaseDurations.IsValidIndex(CurrentPhase) ?
                    Config.PhaseDurations[CurrentPhase] : 60.0f;

                if (Config.DamagePerSecond.IsValidIndex(CurrentPhase))
                {
                    CurrentDamage = Config.DamagePerSecond[CurrentPhase];
                }
            }
        }
        else
        {
            // Start shrinking
            AdvancePhase(Config);
        }
    }

    if (bIsShrinking)
    {
        float ShrinkDuration = Config.ShrinkDurations.IsValidIndex(CurrentPhase) ?
            Config.ShrinkDurations[CurrentPhase] : 60.0f;
        float Alpha = 1.0f - (PhaseTimer / ShrinkDuration);
        Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

        // Interpolate radius
        float StartRadius = (CurrentPhase == 0) ? Config.InitialRadius :
            (Config.InitialRadius * FMath::Pow(0.5f, CurrentPhase));
        CurrentRadius = FMath::Lerp(StartRadius, TargetRadius, Alpha);

        // Interpolate center
        FVector StartCenter = CurrentCenter;
        CurrentCenter = FMath::Lerp(StartCenter, NextCenter, Alpha);
    }
}

bool FForge2BRZoneState::IsPositionSafe(const FVector& Position) const
{
    float Distance = FVector::Dist2D(Position, CurrentCenter);
    return Distance <= CurrentRadius;
}

float FForge2BRZoneState::GetDamageAtPosition(const FVector& Position) const
{
    if (IsPositionSafe(Position))
    {
        return 0.0f;
    }
    return CurrentDamage;
}

void FForge2BRZoneState::AdvancePhase(const FForge2BRZoneConfig& Config)
{
    bIsShrinking = true;

    // Calculate next radius
    float Ratio = FMath::Pow(Config.FinalRadius / Config.InitialRadius, 1.0f / Config.NumberOfPhases);
    TargetRadius = CurrentRadius * Ratio;
    TargetRadius = FMath::Max(TargetRadius, Config.FinalRadius);

    // Calculate next center
    NextCenter = CalculateNextCenter(TargetRadius);

    PhaseTimer = Config.ShrinkDurations.IsValidIndex(CurrentPhase) ?
        Config.ShrinkDurations[CurrentPhase] : 60.0f;
}

FVector FForge2BRZoneState::CalculateNextCenter(float NewRadius) const
{
    // New center must be within current circle
    float MaxOffset = CurrentRadius - NewRadius;
    if (MaxOffset <= 0.0f)
    {
        return CurrentCenter;
    }

    float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
    float RandomDist = FMath::FRandRange(0.0f, MaxOffset * 0.7f);

    return CurrentCenter + FVector(
        FMath::Cos(RandomAngle) * RandomDist,
        FMath::Sin(RandomAngle) * RandomDist,
        0.0f
    );
}

// FForge2BRPlayerInventory Implementation
bool FForge2BRPlayerInventory::AddWeapon(const FForge2BRWeapon& Weapon)
{
    if (Weapons.Num() >= MaxWeapons) return false;
    Weapons.Add(Weapon);
    return true;
}

bool FForge2BRPlayerInventory::AddItem(const FForge2BRLootItem& Item)
{
    // Check for stackable item
    for (FForge2BRLootItem& ExistingItem : Items)
    {
        if (ExistingItem.ItemId == Item.ItemId && ExistingItem.StackSize < ExistingItem.MaxStack)
        {
            int32 CanAdd = ExistingItem.MaxStack - ExistingItem.StackSize;
            int32 ToAdd = FMath::Min(CanAdd, Item.StackSize);
            ExistingItem.StackSize += ToAdd;
            return true;
        }
    }

    if (Items.Num() >= MaxItems) return false;
    Items.Add(Item);
    return true;
}

bool FForge2BRPlayerInventory::AddAmmo(const FString& AmmoType, int32 Amount)
{
    int32* Current = Ammo.Find(AmmoType);
    if (Current)
    {
        *Current = FMath::Min(*Current + Amount, 999);
    }
    else
    {
        Ammo.Add(AmmoType, FMath::Min(Amount, 999));
    }
    return true;
}

void FForge2BRPlayerInventory::RemoveWeapon(int32 Index)
{
    if (Weapons.IsValidIndex(Index))
    {
        Weapons.RemoveAt(Index);
    }
}

void FForge2BRPlayerInventory::RemoveItem(int32 Index)
{
    if (Items.IsValidIndex(Index))
    {
        Items.RemoveAt(Index);
    }
}

int32 FForge2BRPlayerInventory::GetAmmoCount(const FString& AmmoType) const
{
    const int32* Count = Ammo.Find(AmmoType);
    return Count ? *Count : 0;
}

// FForge2BRPlayer Implementation
void FForge2BRPlayer::Initialize(const FString& Id, const FString& Name, int32 Team)
{
    PlayerId = Id;
    PlayerName = Name;
    TeamId = Team;
    State = EForge2BRPlayerState::Alive;
    Health = 100.0f;
    MaxHealth = 100.0f;
    Shield = 0.0f;
    MaxShield = 100.0f;
    KnockdownTimer = 0.0f;
    Stats = FForge2BRPlayerStats();
}

void FForge2BRPlayer::Update(float DeltaTime)
{
    Stats.SurvivalTime += DeltaTime;

    if (State == EForge2BRPlayerState::Knocked)
    {
        KnockdownTimer -= DeltaTime;
        if (KnockdownTimer <= 0.0f)
        {
            Eliminate();
        }
    }
}

void FForge2BRPlayer::TakeDamage(float Damage, const FString& AttackerId)
{
    if (State != EForge2BRPlayerState::Alive) return;

    Stats.DamageTaken += Damage;

    // Shield absorbs first
    if (Shield > 0.0f)
    {
        float ShieldDamage = FMath::Min(Shield, Damage);
        Shield -= ShieldDamage;
        Damage -= ShieldDamage;
    }

    Health -= Damage;

    if (Health <= 0.0f)
    {
        Health = 0.0f;
        Knock(AttackerId);
    }
}

void FForge2BRPlayer::Heal(float Amount)
{
    if (State != EForge2BRPlayerState::Alive) return;
    Health = FMath::Min(Health + Amount, MaxHealth);
}

void FForge2BRPlayer::AddShield(float Amount)
{
    if (State != EForge2BRPlayerState::Alive) return;
    Shield = FMath::Min(Shield + Amount, MaxShield);
}

void FForge2BRPlayer::Knock(const FString& AttackerId)
{
    State = EForge2BRPlayerState::Knocked;
    KnockedBy = AttackerId;
    KnockdownTimer = KnockdownDuration;
}

void FForge2BRPlayer::Eliminate()
{
    State = EForge2BRPlayerState::Eliminated;
    Health = 0.0f;
    Shield = 0.0f;
}

void FForge2BRPlayer::Revive(float HealthPercent)
{
    if (State != EForge2BRPlayerState::Knocked) return;
    State = EForge2BRPlayerState::Alive;
    Health = MaxHealth * HealthPercent;
    KnockedBy.Empty();
}

// FForge2BRTeam Implementation
int32 FForge2BRTeam::GetAliveCount(const TMap<FString, FForge2BRPlayer>& Players) const
{
    int32 Count = 0;
    for (const FString& PlayerId : PlayerIds)
    {
        const FForge2BRPlayer* Player = Players.Find(PlayerId);
        if (Player && Player->IsAlive())
        {
            Count++;
        }
    }
    return Count;
}

bool FForge2BRTeam::IsEliminated(const TMap<FString, FForge2BRPlayer>& Players) const
{
    for (const FString& PlayerId : PlayerIds)
    {
        const FForge2BRPlayer* Player = Players.Find(PlayerId);
        if (Player && (Player->State == EForge2BRPlayerState::Alive ||
                       Player->State == EForge2BRPlayerState::Knocked))
        {
            return false;
        }
    }
    return true;
}

// FForge2BRSupplyDrop Implementation
void FForge2BRSupplyDrop::Update(float DeltaTime)
{
    if (bLanded) return;

    TimeToLand -= DeltaTime;
    CurrentPosition.Z -= DropSpeed * DeltaTime;

    if (CurrentPosition.Z <= TargetLocation.Z || TimeToLand <= 0.0f)
    {
        CurrentPosition.Z = TargetLocation.Z;
        bLanded = true;
    }
}

void FForge2BRSupplyDrop::GenerateContents(EForge2BRLootTier GuaranteedTier)
{
    Contents.Empty();

    // Add guaranteed high-tier weapon
    FForge2BRLootItem Weapon;
    Weapon.ItemId = TEXT("SupplyWeapon");
    Weapon.ItemName = TEXT("Supply Drop Weapon");
    Weapon.ItemType = EForge2BRItemType::Weapon;
    Weapon.Tier = GuaranteedTier;
    Contents.Add(Weapon);

    // Add healing items
    FForge2BRLootItem Healing;
    Healing.ItemId = TEXT("MedKit");
    Healing.ItemName = TEXT("Med Kit");
    Healing.ItemType = EForge2BRItemType::Healing;
    Healing.Tier = EForge2BRLootTier::Rare;
    Healing.StackSize = 2;
    Contents.Add(Healing);

    // Add ammo
    FForge2BRLootItem Ammo;
    Ammo.ItemId = TEXT("HeavyAmmo");
    Ammo.ItemName = TEXT("Heavy Ammo");
    Ammo.ItemType = EForge2BRItemType::Ammo;
    Ammo.StackSize = 60;
    Contents.Add(Ammo);
}

// FForge2BRVehicle Implementation
void FForge2BRVehicle::Update(float DeltaTime, float Throttle)
{
    if (bIsDestroyed) return;

    if (Throttle != 0.0f && Fuel > 0.0f)
    {
        Fuel -= FuelConsumption * FMath::Abs(Throttle) * DeltaTime;
        if (Fuel < 0.0f) Fuel = 0.0f;
    }
}

void FForge2BRVehicle::TakeDamage(float Damage)
{
    if (bIsDestroyed) return;

    Health -= Damage;
    if (Health <= 0.0f)
    {
        Health = 0.0f;
        bIsDestroyed = true;
        OccupantIds.Empty();
    }
}

bool FForge2BRVehicle::EnterVehicle(const FString& PlayerId, int32 SeatIndex)
{
    if (bIsDestroyed) return false;
    if (OccupantIds.Num() >= SeatCount) return false;
    if (OccupantIds.Contains(PlayerId)) return false;

    OccupantIds.Add(PlayerId);
    return true;
}

void FForge2BRVehicle::ExitVehicle(const FString& PlayerId)
{
    OccupantIds.Remove(PlayerId);
}

// FForge2BRMatchState Implementation
void FForge2BRMatchState::Initialize(const FForge2BRMatchConfig& Config)
{
    Phase = EForge2BRMatchPhase::Lobby;
    MatchTime = 0.0f;
    PhaseTimer = Config.LobbyCountdown;
    PlayersAlive = 0;
    TeamsAlive = 0;
    TotalPlayers = 0;
    PlaneProgress = 0.0f;
    NextPlacement = 1;

    ZoneState.Initialize(Config.ZoneConfig, FVector::ZeroVector);
}

void FForge2BRMatchState::Update(float DeltaTime, const FForge2BRMatchConfig& Config)
{
    MatchTime += DeltaTime;

    switch (Phase)
    {
        case EForge2BRMatchPhase::Lobby:
            PhaseTimer -= DeltaTime;
            if (PhaseTimer <= 0.0f && TotalPlayers >= Config.MinPlayersToStart)
            {
                TransitionToPhase(EForge2BRMatchPhase::Deploying);
            }
            break;

        case EForge2BRMatchPhase::Deploying:
            UpdatePlanePosition(DeltaTime, Config.DeployConfig);
            if (PlaneProgress >= 1.0f)
            {
                TransitionToPhase(EForge2BRMatchPhase::EarlyGame);
            }
            break;

        case EForge2BRMatchPhase::EarlyGame:
        case EForge2BRMatchPhase::MidGame:
        case EForge2BRMatchPhase::EndGame:
            ZoneState.Update(DeltaTime, Config.ZoneConfig);

            if (ZoneState.CurrentPhase >= Config.ZoneConfig.NumberOfPhases / 2 &&
                Phase == EForge2BRMatchPhase::EarlyGame)
            {
                TransitionToPhase(EForge2BRMatchPhase::MidGame);
            }
            else if (ZoneState.CurrentPhase >= Config.ZoneConfig.NumberOfPhases - 2 &&
                     Phase == EForge2BRMatchPhase::MidGame)
            {
                TransitionToPhase(EForge2BRMatchPhase::EndGame);
            }
            break;

        case EForge2BRMatchPhase::GameOver:
            break;
    }
}

void FForge2BRMatchState::UpdatePlanePosition(float DeltaTime, const FForge2BRDeployConfig& Config)
{
    float FlightDistance = FVector::Dist(Config.PlaneStartPos, Config.PlaneEndPos);
    float FlightTime = FlightDistance / Config.PlaneSpeed;

    PlaneProgress += DeltaTime / FlightTime;
    PlaneProgress = FMath::Clamp(PlaneProgress, 0.0f, 1.0f);

    PlanePosition = FMath::Lerp(Config.PlaneStartPos, Config.PlaneEndPos, PlaneProgress);
    PlanePosition.Z = Config.PlaneAltitude;
}

void FForge2BRMatchState::TransitionToPhase(EForge2BRMatchPhase NewPhase)
{
    Phase = NewPhase;

    switch (NewPhase)
    {
        case EForge2BRMatchPhase::Deploying:
            PlaneProgress = 0.0f;
            break;
        case EForge2BRMatchPhase::EarlyGame:
            break;
        default:
            break;
    }
}

// UForge2BattleRoyaleManager Implementation
UForge2BattleRoyaleManager::UForge2BattleRoyaleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UForge2BattleRoyaleManager::Initialize(const FForge2BRMatchConfig& InConfig)
{
    Config = InConfig;
    MatchState.Initialize(Config);

    SetupWeaponDatabase();
    SetupItemDatabase();
}

void UForge2BattleRoyaleManager::Update(float DeltaTime)
{
    MatchState.Update(DeltaTime, Config);

    UpdatePlayers(DeltaTime);
    UpdateZone(DeltaTime);
    UpdateSupplyDrops(DeltaTime);
    UpdateVehicles(DeltaTime);
    UpdatePings(DeltaTime);
    CheckMatchEnd();
}

void UForge2BattleRoyaleManager::SetupWeaponDatabase()
{
    WeaponDatabase.Empty();

    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateAssaultRifle(TEXT("AR_Common"), EForge2BRLootTier::Common));
    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateAssaultRifle(TEXT("AR_Rare"), EForge2BRLootTier::Rare));
    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateAssaultRifle(TEXT("AR_Epic"), EForge2BRLootTier::Epic));
    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateShotgun(TEXT("SG_Common"), EForge2BRLootTier::Common));
    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateShotgun(TEXT("SG_Rare"), EForge2BRLootTier::Rare));
    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateSMG(TEXT("SMG_Common"), EForge2BRLootTier::Common));
    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateSMG(TEXT("SMG_Rare"), EForge2BRLootTier::Rare));
    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateSniper(TEXT("Sniper_Rare"), EForge2BRLootTier::Rare));
    WeaponDatabase.Add(UForge2BattleRoyaleUtility::CreateSniper(TEXT("Sniper_Legendary"), EForge2BRLootTier::Legendary));
}

void UForge2BattleRoyaleManager::SetupItemDatabase()
{
    ItemDatabase.Empty();

    ItemDatabase.Add(UForge2BattleRoyaleUtility::CreateHealingItem(TEXT("Bandage"), 15.0f, 3.0f));
    ItemDatabase.Add(UForge2BattleRoyaleUtility::CreateHealingItem(TEXT("MedKit"), 100.0f, 8.0f));
    ItemDatabase.Add(UForge2BattleRoyaleUtility::CreateShieldItem(TEXT("MiniShield"), 25.0f, 2.0f));
    ItemDatabase.Add(UForge2BattleRoyaleUtility::CreateShieldItem(TEXT("ShieldPotion"), 50.0f, 5.0f));

    // Ammo types
    FForge2BRLootItem LightAmmo;
    LightAmmo.ItemId = TEXT("LightAmmo");
    LightAmmo.ItemName = TEXT("Light Ammo");
    LightAmmo.ItemType = EForge2BRItemType::Ammo;
    LightAmmo.StackSize = 30;
    LightAmmo.MaxStack = 999;
    ItemDatabase.Add(LightAmmo);

    FForge2BRLootItem HeavyAmmo;
    HeavyAmmo.ItemId = TEXT("HeavyAmmo");
    HeavyAmmo.ItemName = TEXT("Heavy Ammo");
    HeavyAmmo.ItemType = EForge2BRItemType::Ammo;
    HeavyAmmo.StackSize = 20;
    HeavyAmmo.MaxStack = 999;
    ItemDatabase.Add(HeavyAmmo);

    FForge2BRLootItem Shells;
    Shells.ItemId = TEXT("Shells");
    Shells.ItemName = TEXT("Shells");
    Shells.ItemType = EForge2BRItemType::Ammo;
    Shells.StackSize = 10;
    Shells.MaxStack = 99;
    ItemDatabase.Add(Shells);
}

FString UForge2BattleRoyaleManager::AddPlayer(const FString& PlayerName, int32 TeamId)
{
    FString PlayerId = FString::Printf(TEXT("Player_%d"), Players.Num());

    FForge2BRPlayer Player;
    Player.Initialize(PlayerId, PlayerName, TeamId);
    Players.Add(PlayerId, Player);

    // Create or join team
    if (Config.TeamSize > 1)
    {
        if (TeamId < 0)
        {
            TeamId = CreateTeam();
        }

        FForge2BRTeam* Team = Teams.Find(TeamId);
        if (Team)
        {
            Team->PlayerIds.Add(PlayerId);
        }

        FForge2BRPlayer* AddedPlayer = Players.Find(PlayerId);
        if (AddedPlayer)
        {
            AddedPlayer->TeamId = TeamId;
        }
    }

    MatchState.TotalPlayers++;
    MatchState.PlayersAlive++;

    return PlayerId;
}

void UForge2BattleRoyaleManager::RemovePlayer(const FString& PlayerId)
{
    FForge2BRPlayer* Player = Players.Find(PlayerId);
    if (!Player) return;

    if (Player->IsAlive())
    {
        MatchState.PlayersAlive--;
    }

    if (Player->TeamId >= 0)
    {
        FForge2BRTeam* Team = Teams.Find(Player->TeamId);
        if (Team)
        {
            Team->PlayerIds.Remove(PlayerId);
        }
    }

    Players.Remove(PlayerId);
    MatchState.TotalPlayers--;
}

void UForge2BattleRoyaleManager::PlayerJump(const FString& PlayerId)
{
    if (MatchState.Phase != EForge2BRMatchPhase::Deploying) return;

    float JumpProgress = MatchState.PlaneProgress;
    if (JumpProgress < Config.DeployConfig.JumpWindowStart ||
        JumpProgress > Config.DeployConfig.JumpWindowEnd)
    {
        return;
    }

    FForge2BRPlayer* Player = Players.Find(PlayerId);
    if (!Player) return;

    Player->Position = MatchState.PlanePosition;
}

void UForge2BattleRoyaleManager::PlayerDeploy(const FString& PlayerId, const FVector& LandingPos)
{
    FForge2BRPlayer* Player = Players.Find(PlayerId);
    if (!Player) return;

    Player->Position = LandingPos;
}

void UForge2BattleRoyaleManager::ProcessDamage(const FString& AttackerId, const FString& VictimId,
                                                float Damage, bool bHeadshot)
{
    FForge2BRPlayer* Victim = Players.Find(VictimId);
    if (!Victim || !Victim->IsAlive()) return;

    FForge2BRPlayer* Attacker = Players.Find(AttackerId);
    if (Attacker)
    {
        Attacker->Stats.DamageDealt += Damage;
    }

    float ActualDamage = bHeadshot ? Damage * 2.0f : Damage;
    Victim->TakeDamage(ActualDamage, AttackerId);

    if (Victim->State == EForge2BRPlayerState::Knocked)
    {
        // Check if team can revive
        if (Config.TeamSize == 1 || (Victim->TeamId >= 0))
        {
            FForge2BRTeam* Team = Teams.Find(Victim->TeamId);
            if (!Team || Team->GetAliveCount(Players) == 0)
            {
                EliminatePlayer(VictimId, AttackerId);
            }
        }
    }
}

void UForge2BattleRoyaleManager::RevivePlayer(const FString& ReviverId, const FString& TargetId)
{
    FForge2BRPlayer* Target = Players.Find(TargetId);
    if (!Target || Target->State != EForge2BRPlayerState::Knocked) return;

    FForge2BRPlayer* Reviver = Players.Find(ReviverId);
    if (!Reviver || !Reviver->IsAlive()) return;

    // Must be same team
    if (Target->TeamId != Reviver->TeamId) return;

    Target->Revive(0.3f);
    Reviver->Stats.Revives++;
}

void UForge2BattleRoyaleManager::SpawnLoot()
{
    for (FForge2BRLootSpawn& Spawn : LootSpawns)
    {
        if (Spawn.bHasSpawned) continue;

        if (FMath::FRand() > Spawn.SpawnChance) continue;

        int32 ItemCount = FMath::RandRange(1, 3);
        if (Spawn.bIsHotDrop) ItemCount += 2;

        for (int32 i = 0; i < ItemCount; i++)
        {
            FForge2BRLootItem Item = GenerateLootItem(Spawn.MinTier, Spawn.MaxTier);
            Spawn.SpawnedItems.Add(Item);
        }

        Spawn.bHasSpawned = true;
    }
}

FForge2BRLootItem UForge2BattleRoyaleManager::GenerateLootItem(EForge2BRLootTier MinTier,
                                                                EForge2BRLootTier MaxTier)
{
    // Random tier within range
    int32 TierRange = (int32)MaxTier - (int32)MinTier + 1;
    EForge2BRLootTier SelectedTier = (EForge2BRLootTier)((int32)MinTier + FMath::RandRange(0, TierRange - 1));

    // 40% chance weapon, 60% chance item
    if (FMath::FRand() < 0.4f && WeaponDatabase.Num() > 0)
    {
        TArray<FForge2BRWeapon> ValidWeapons;
        for (const FForge2BRWeapon& Weapon : WeaponDatabase)
        {
            if ((int32)Weapon.Tier >= (int32)MinTier && (int32)Weapon.Tier <= (int32)MaxTier)
            {
                ValidWeapons.Add(Weapon);
            }
        }

        if (ValidWeapons.Num() > 0)
        {
            int32 Index = FMath::RandRange(0, ValidWeapons.Num() - 1);
            FForge2BRLootItem WeaponItem;
            WeaponItem.ItemId = ValidWeapons[Index].WeaponId;
            WeaponItem.ItemName = ValidWeapons[Index].WeaponName;
            WeaponItem.ItemType = EForge2BRItemType::Weapon;
            WeaponItem.Tier = ValidWeapons[Index].Tier;
            return WeaponItem;
        }
    }

    // Return random item
    if (ItemDatabase.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, ItemDatabase.Num() - 1);
        return ItemDatabase[Index];
    }

    return FForge2BRLootItem();
}

void UForge2BattleRoyaleManager::PickupItem(const FString& PlayerId, const FString& SpawnId, int32 ItemIndex)
{
    FForge2BRPlayer* Player = Players.Find(PlayerId);
    if (!Player || !Player->IsAlive()) return;

    FForge2BRLootSpawn* Spawn = nullptr;
    for (FForge2BRLootSpawn& S : LootSpawns)
    {
        if (S.SpawnId == SpawnId)
        {
            Spawn = &S;
            break;
        }
    }

    if (!Spawn || !Spawn->SpawnedItems.IsValidIndex(ItemIndex)) return;

    FForge2BRLootItem& Item = Spawn->SpawnedItems[ItemIndex];

    if (Item.ItemType == EForge2BRItemType::Weapon)
    {
        // Find weapon in database
        for (const FForge2BRWeapon& Weapon : WeaponDatabase)
        {
            if (Weapon.WeaponId == Item.ItemId)
            {
                if (Player->Inventory.AddWeapon(Weapon))
                {
                    Spawn->SpawnedItems.RemoveAt(ItemIndex);
                    Player->Stats.ItemsLooted++;
                }
                return;
            }
        }
    }
    else if (Item.ItemType == EForge2BRItemType::Ammo)
    {
        Player->Inventory.AddAmmo(Item.ItemId, Item.StackSize);
        Spawn->SpawnedItems.RemoveAt(ItemIndex);
        Player->Stats.ItemsLooted++;
    }
    else
    {
        if (Player->Inventory.AddItem(Item))
        {
            Spawn->SpawnedItems.RemoveAt(ItemIndex);
            Player->Stats.ItemsLooted++;
        }
    }
}

void UForge2BattleRoyaleManager::DropItem(const FString& PlayerId, int32 InventoryIndex,
                                          const FVector& Location)
{
    FForge2BRPlayer* Player = Players.Find(PlayerId);
    if (!Player) return;

    if (Player->Inventory.Items.IsValidIndex(InventoryIndex))
    {
        FForge2BRLootItem DroppedItem = Player->Inventory.Items[InventoryIndex];
        Player->Inventory.RemoveItem(InventoryIndex);

        // Create new loot spawn
        FForge2BRLootSpawn NewSpawn;
        NewSpawn.SpawnId = FString::Printf(TEXT("Drop_%f"), FMath::FRand());
        NewSpawn.Location = Location;
        NewSpawn.bHasSpawned = true;
        NewSpawn.SpawnedItems.Add(DroppedItem);
        LootSpawns.Add(NewSpawn);
    }
}

void UForge2BattleRoyaleManager::SpawnSupplyDrop(const FVector& Location)
{
    FForge2BRSupplyDrop Drop;
    Drop.DropId = FString::Printf(TEXT("Supply_%d"), SupplyDrops.Num());
    Drop.TargetLocation = Location;
    Drop.CurrentPosition = FVector(Location.X, Location.Y, Location.Z + 2000.0f);
    Drop.TimeToLand = 2000.0f / Drop.DropSpeed;
    Drop.GenerateContents(EForge2BRLootTier::Epic);

    SupplyDrops.Add(Drop);
}

void UForge2BattleRoyaleManager::EnterVehicle(const FString& PlayerId, const FString& VehicleId, int32 Seat)
{
    FForge2BRPlayer* Player = Players.Find(PlayerId);
    if (!Player || !Player->IsAlive()) return;

    for (FForge2BRVehicle& Vehicle : Vehicles)
    {
        if (Vehicle.VehicleId == VehicleId)
        {
            Vehicle.EnterVehicle(PlayerId, Seat);
            return;
        }
    }
}

void UForge2BattleRoyaleManager::ExitVehicle(const FString& PlayerId)
{
    for (FForge2BRVehicle& Vehicle : Vehicles)
    {
        if (Vehicle.OccupantIds.Contains(PlayerId))
        {
            Vehicle.ExitVehicle(PlayerId);
            return;
        }
    }
}

void UForge2BattleRoyaleManager::CreatePing(const FString& PlayerId, const FVector& Location,
                                            const FString& PingType)
{
    FForge2BRPlayer* Player = Players.Find(PlayerId);
    if (!Player) return;

    FForge2BRPingMarker Ping;
    Ping.PingId = FString::Printf(TEXT("Ping_%d"), Pings.Num());
    Ping.SenderId = PlayerId;
    Ping.TeamId = Player->TeamId;
    Ping.Location = Location;
    Ping.PingType = PingType;
    Ping.Duration = 5.0f;
    Ping.TimeRemaining = 5.0f;

    Pings.Add(Ping);
}

FForge2BRPlayer UForge2BattleRoyaleManager::GetPlayer(const FString& PlayerId) const
{
    const FForge2BRPlayer* Player = Players.Find(PlayerId);
    return Player ? *Player : FForge2BRPlayer();
}

void UForge2BattleRoyaleManager::UpdatePlayers(float DeltaTime)
{
    for (auto& Pair : Players)
    {
        FForge2BRPlayer& Player = Pair.Value;
        Player.Update(DeltaTime);

        // Zone damage
        if (Player.IsAlive())
        {
            float Damage = MatchState.ZoneState.GetDamageAtPosition(Player.Position);
            if (Damage > 0.0f)
            {
                Player.TakeDamage(Damage * DeltaTime, TEXT("Zone"));
            }
        }
    }
}

void UForge2BattleRoyaleManager::UpdateZone(float DeltaTime)
{
    // Zone update happens in MatchState.Update
}

void UForge2BattleRoyaleManager::UpdateSupplyDrops(float DeltaTime)
{
    for (FForge2BRSupplyDrop& Drop : SupplyDrops)
    {
        Drop.Update(DeltaTime);
    }
}

void UForge2BattleRoyaleManager::UpdateVehicles(float DeltaTime)
{
    for (FForge2BRVehicle& Vehicle : Vehicles)
    {
        Vehicle.Update(DeltaTime, 0.0f);

        // Zone damage to vehicles
        if (!Vehicle.bIsDestroyed)
        {
            float Damage = MatchState.ZoneState.GetDamageAtPosition(Vehicle.Position);
            if (Damage > 0.0f)
            {
                Vehicle.TakeDamage(Damage * DeltaTime * 2.0f);
            }
        }
    }
}

void UForge2BattleRoyaleManager::UpdatePings(float DeltaTime)
{
    for (int32 i = Pings.Num() - 1; i >= 0; i--)
    {
        Pings[i].Update(DeltaTime);
        if (Pings[i].IsExpired())
        {
            Pings.RemoveAt(i);
        }
    }
}

void UForge2BattleRoyaleManager::CheckMatchEnd()
{
    if (MatchState.Phase == EForge2BRMatchPhase::GameOver) return;

    // Count alive players/teams
    int32 AliveCount = 0;
    int32 TeamsWithAlive = 0;

    for (const auto& Pair : Players)
    {
        if (Pair.Value.IsAlive())
        {
            AliveCount++;
        }
    }

    if (Config.TeamSize > 1)
    {
        for (const auto& Pair : Teams)
        {
            if (!Pair.Value.IsEliminated(Players))
            {
                TeamsWithAlive++;
            }
        }
    }
    else
    {
        TeamsWithAlive = AliveCount;
    }

    MatchState.PlayersAlive = AliveCount;
    MatchState.TeamsAlive = TeamsWithAlive;

    // Win condition
    if (TeamsWithAlive <= 1 && MatchState.Phase >= EForge2BRMatchPhase::EarlyGame)
    {
        MatchState.TransitionToPhase(EForge2BRMatchPhase::GameOver);
    }
}

void UForge2BattleRoyaleManager::EliminatePlayer(const FString& PlayerId, const FString& KillerId)
{
    FForge2BRPlayer* Player = Players.Find(PlayerId);
    if (!Player) return;

    Player->Eliminate();
    Player->Stats.Placement = MatchState.NextPlacement++;

    MatchState.PlayersAlive--;

    // Award kill
    FForge2BRPlayer* Killer = Players.Find(KillerId);
    if (Killer && Killer != Player)
    {
        Killer->Stats.Kills++;

        if (Killer->TeamId >= 0)
        {
            FForge2BRTeam* Team = Teams.Find(Killer->TeamId);
            if (Team) Team->TotalKills++;
        }
    }

    // Check if team eliminated
    if (Player->TeamId >= 0)
    {
        FForge2BRTeam* Team = Teams.Find(Player->TeamId);
        if (Team && Team->IsEliminated(Players))
        {
            Team->bEliminated = true;
            Team->Placement = MatchState.NextPlacement++;
            MatchState.TeamsAlive--;
        }
    }
}

void UForge2BattleRoyaleManager::UpdatePlacements()
{
    // Called when match ends
}

int32 UForge2BattleRoyaleManager::CreateTeam()
{
    int32 NewTeamId = Teams.Num();
    FForge2BRTeam NewTeam;
    NewTeam.TeamId = NewTeamId;
    Teams.Add(NewTeamId, NewTeam);
    MatchState.TeamsAlive++;
    return NewTeamId;
}

// UForge2BattleRoyaleUtility Implementation
FForge2BRWeapon UForge2BattleRoyaleUtility::CreateAssaultRifle(const FString& WeaponId, EForge2BRLootTier Tier)
{
    FForge2BRWeapon Weapon;
    Weapon.WeaponId = WeaponId;
    Weapon.WeaponName = FString::Printf(TEXT("Assault Rifle (%s)"),
        *UEnum::GetValueAsString(Tier));
    Weapon.WeaponType = EForge2BRWeaponType::AssaultRifle;
    Weapon.Tier = Tier;

    float TierMult = 1.0f + (int32)Tier * 0.1f;
    Weapon.Damage = 30.0f * TierMult;
    Weapon.FireRate = 5.5f;
    Weapon.MagazineSize = 30;
    Weapon.ReloadTime = 2.2f / TierMult;
    Weapon.Range = 5000.0f;
    Weapon.Spread = 2.5f / TierMult;
    Weapon.AimSpread = 0.5f / TierMult;
    Weapon.AmmoType = TEXT("LightAmmo");
    Weapon.bAutomatic = true;

    return Weapon;
}

FForge2BRWeapon UForge2BattleRoyaleUtility::CreateShotgun(const FString& WeaponId, EForge2BRLootTier Tier)
{
    FForge2BRWeapon Weapon;
    Weapon.WeaponId = WeaponId;
    Weapon.WeaponName = FString::Printf(TEXT("Shotgun (%s)"),
        *UEnum::GetValueAsString(Tier));
    Weapon.WeaponType = EForge2BRWeaponType::Shotgun;
    Weapon.Tier = Tier;

    float TierMult = 1.0f + (int32)Tier * 0.1f;
    Weapon.Damage = 100.0f * TierMult;
    Weapon.FireRate = 1.0f;
    Weapon.MagazineSize = 8;
    Weapon.ReloadTime = 4.0f / TierMult;
    Weapon.Range = 500.0f;
    Weapon.Spread = 8.0f / TierMult;
    Weapon.AimSpread = 6.0f / TierMult;
    Weapon.AmmoType = TEXT("Shells");
    Weapon.bAutomatic = false;

    return Weapon;
}

FForge2BRWeapon UForge2BattleRoyaleUtility::CreateSMG(const FString& WeaponId, EForge2BRLootTier Tier)
{
    FForge2BRWeapon Weapon;
    Weapon.WeaponId = WeaponId;
    Weapon.WeaponName = FString::Printf(TEXT("SMG (%s)"),
        *UEnum::GetValueAsString(Tier));
    Weapon.WeaponType = EForge2BRWeaponType::SMG;
    Weapon.Tier = Tier;

    float TierMult = 1.0f + (int32)Tier * 0.1f;
    Weapon.Damage = 18.0f * TierMult;
    Weapon.FireRate = 10.0f;
    Weapon.MagazineSize = 35;
    Weapon.ReloadTime = 1.8f / TierMult;
    Weapon.Range = 2000.0f;
    Weapon.Spread = 4.0f / TierMult;
    Weapon.AimSpread = 2.0f / TierMult;
    Weapon.AmmoType = TEXT("LightAmmo");
    Weapon.bAutomatic = true;

    return Weapon;
}

FForge2BRWeapon UForge2BattleRoyaleUtility::CreateSniper(const FString& WeaponId, EForge2BRLootTier Tier)
{
    FForge2BRWeapon Weapon;
    Weapon.WeaponId = WeaponId;
    Weapon.WeaponName = FString::Printf(TEXT("Sniper Rifle (%s)"),
        *UEnum::GetValueAsString(Tier));
    Weapon.WeaponType = EForge2BRWeaponType::Sniper;
    Weapon.Tier = Tier;

    float TierMult = 1.0f + (int32)Tier * 0.15f;
    Weapon.Damage = 105.0f * TierMult;
    Weapon.HeadshotMultiplier = 2.5f;
    Weapon.FireRate = 0.75f;
    Weapon.MagazineSize = 5;
    Weapon.ReloadTime = 3.0f / TierMult;
    Weapon.Range = 15000.0f;
    Weapon.Spread = 0.1f;
    Weapon.AimSpread = 0.0f;
    Weapon.AmmoType = TEXT("HeavyAmmo");
    Weapon.bAutomatic = false;

    return Weapon;
}

FForge2BRLootItem UForge2BattleRoyaleUtility::CreateHealingItem(const FString& ItemId,
                                                                 float HealAmount, float UseTime)
{
    FForge2BRLootItem Item;
    Item.ItemId = ItemId;
    Item.ItemName = ItemId;
    Item.ItemType = EForge2BRItemType::Healing;

    if (HealAmount >= 100.0f)
    {
        Item.Tier = EForge2BRLootTier::Rare;
        Item.MaxStack = 3;
    }
    else
    {
        Item.Tier = EForge2BRLootTier::Common;
        Item.MaxStack = 15;
    }

    Item.StackSize = 1;
    Item.SpawnWeight = 1.0f;

    return Item;
}

FForge2BRLootItem UForge2BattleRoyaleUtility::CreateShieldItem(const FString& ItemId,
                                                                float ShieldAmount, float UseTime)
{
    FForge2BRLootItem Item;
    Item.ItemId = ItemId;
    Item.ItemName = ItemId;
    Item.ItemType = EForge2BRItemType::Shield;

    if (ShieldAmount >= 50.0f)
    {
        Item.Tier = EForge2BRLootTier::Rare;
        Item.MaxStack = 2;
    }
    else
    {
        Item.Tier = EForge2BRLootTier::Common;
        Item.MaxStack = 10;
    }

    Item.StackSize = 1;
    Item.SpawnWeight = 1.0f;

    return Item;
}

FForge2BRZoneConfig UForge2BattleRoyaleUtility::CreateStandardZoneConfig()
{
    FForge2BRZoneConfig Config;
    Config.ZoneType = EForge2BRZoneType::Circle;
    Config.InitialRadius = 5000.0f;
    Config.FinalRadius = 50.0f;
    Config.NumberOfPhases = 8;
    Config.WaitTimeBeforeFirstShrink = 120.0f;
    Config.bRandomCenter = true;

    // Phase durations (wait time)
    Config.PhaseDurations = { 120.0f, 90.0f, 75.0f, 60.0f, 45.0f, 30.0f, 20.0f, 10.0f };
    // Shrink durations
    Config.ShrinkDurations = { 120.0f, 90.0f, 75.0f, 60.0f, 45.0f, 30.0f, 20.0f, 10.0f };
    // Damage per second
    Config.DamagePerSecond = { 1.0f, 2.0f, 3.0f, 5.0f, 7.0f, 10.0f, 15.0f, 20.0f };

    return Config;
}

FForge2BRZoneConfig UForge2BattleRoyaleUtility::CreateFastZoneConfig()
{
    FForge2BRZoneConfig Config;
    Config.ZoneType = EForge2BRZoneType::Circle;
    Config.InitialRadius = 3000.0f;
    Config.FinalRadius = 30.0f;
    Config.NumberOfPhases = 5;
    Config.WaitTimeBeforeFirstShrink = 60.0f;
    Config.bRandomCenter = true;

    Config.PhaseDurations = { 60.0f, 45.0f, 30.0f, 20.0f, 10.0f };
    Config.ShrinkDurations = { 45.0f, 30.0f, 20.0f, 15.0f, 10.0f };
    Config.DamagePerSecond = { 2.0f, 5.0f, 10.0f, 15.0f, 25.0f };

    return Config;
}

FForge2BRVehicle UForge2BattleRoyaleUtility::CreateCar(const FString& VehicleId, const FVector& Position)
{
    FForge2BRVehicle Vehicle;
    Vehicle.VehicleId = VehicleId;
    Vehicle.VehicleType = EForge2BRVehicleType::Car;
    Vehicle.Position = Position;
    Vehicle.Health = 800.0f;
    Vehicle.MaxHealth = 800.0f;
    Vehicle.Fuel = 100.0f;
    Vehicle.MaxFuel = 100.0f;
    Vehicle.FuelConsumption = 8.0f;
    Vehicle.SeatCount = 4;
    Vehicle.MaxSpeed = 100.0f;

    return Vehicle;
}

FForge2BRVehicle UForge2BattleRoyaleUtility::CreateHelicopter(const FString& VehicleId, const FVector& Position)
{
    FForge2BRVehicle Vehicle;
    Vehicle.VehicleId = VehicleId;
    Vehicle.VehicleType = EForge2BRVehicleType::Helicopter;
    Vehicle.Position = Position;
    Vehicle.Health = 1500.0f;
    Vehicle.MaxHealth = 1500.0f;
    Vehicle.Fuel = 100.0f;
    Vehicle.MaxFuel = 100.0f;
    Vehicle.FuelConsumption = 15.0f;
    Vehicle.SeatCount = 5;
    Vehicle.MaxSpeed = 150.0f;

    return Vehicle;
}
