// Forge2BattleRoyaleSystem.h - Battle Royale Genre System
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2BattleRoyaleSystem.generated.h"

// Forward declarations
class UForge2BattleRoyaleManager;

// Enums
UENUM(BlueprintType)
enum class EForge2BRMatchPhase : uint8
{
    Lobby,
    Deploying,
    EarlyGame,
    MidGame,
    EndGame,
    GameOver
};

UENUM(BlueprintType)
enum class EForge2BRPlayerState : uint8
{
    Alive,
    Knocked,
    Eliminated,
    Spectating
};

UENUM(BlueprintType)
enum class EForge2BRLootTier : uint8
{
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    Mythic
};

UENUM(BlueprintType)
enum class EForge2BRItemType : uint8
{
    Weapon,
    Ammo,
    Healing,
    Shield,
    Throwable,
    Utility,
    Material
};

UENUM(BlueprintType)
enum class EForge2BRWeaponType : uint8
{
    AssaultRifle,
    Shotgun,
    SMG,
    Sniper,
    Pistol,
    Launcher,
    Melee
};

UENUM(BlueprintType)
enum class EForge2BRZoneType : uint8
{
    Circle,
    Gas,
    Storm,
    Fire,
    Custom
};

UENUM(BlueprintType)
enum class EForge2BRVehicleType : uint8
{
    Car,
    Truck,
    Boat,
    Helicopter,
    ATV,
    Motorcycle
};

UENUM(BlueprintType)
enum class EForge2BRDropType : uint8
{
    Plane,
    DropPod,
    Skydive,
    Teleport
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BRZoneType ZoneType = EForge2BRZoneType::Circle;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float InitialRadius = 5000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FinalRadius = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 NumberOfPhases = 8;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> PhaseDurations;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> ShrinkDurations;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> DamagePerSecond;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WaitTimeBeforeFirstShrink = 120.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRandomCenter = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRZoneState
{
    GENERATED_BODY()

    UPROPERTY() FVector CurrentCenter = FVector::ZeroVector;
    UPROPERTY() FVector NextCenter = FVector::ZeroVector;
    UPROPERTY() float CurrentRadius = 5000.0f;
    UPROPERTY() float TargetRadius = 5000.0f;
    UPROPERTY() int32 CurrentPhase = 0;
    UPROPERTY() float PhaseTimer = 0.0f;
    UPROPERTY() bool bIsShrinking = false;
    UPROPERTY() float CurrentDamage = 1.0f;

    void Initialize(const FForge2BRZoneConfig& Config, const FVector& MapCenter);
    void Update(float DeltaTime, const FForge2BRZoneConfig& Config);
    bool IsPositionSafe(const FVector& Position) const;
    float GetDamageAtPosition(const FVector& Position) const;
    void AdvancePhase(const FForge2BRZoneConfig& Config);
    FVector CalculateNextCenter(float NewRadius) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRLootItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ItemId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ItemName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BRItemType ItemType = EForge2BRItemType::Weapon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BRLootTier Tier = EForge2BRLootTier::Common;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 StackSize = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxStack = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpawnWeight = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UStaticMesh> WorldMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRWeapon
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString WeaponId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString WeaponName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BRWeaponType WeaponType = EForge2BRWeaponType::AssaultRifle;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BRLootTier Tier = EForge2BRLootTier::Common;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damage = 30.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeadshotMultiplier = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FireRate = 5.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MagazineSize = 30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReloadTime = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 5000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Spread = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AimSpread = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString AmmoType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutomatic = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BurstCount = 1;

    float GetDPS() const { return Damage * FireRate; }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRPlayerInventory
{
    GENERATED_BODY()

    UPROPERTY() TArray<FForge2BRWeapon> Weapons;
    UPROPERTY() TMap<FString, int32> Ammo;
    UPROPERTY() TArray<FForge2BRLootItem> Items;
    UPROPERTY() int32 MaxWeapons = 5;
    UPROPERTY() int32 MaxItems = 6;
    UPROPERTY() int32 Materials = 0;
    UPROPERTY() int32 MaxMaterials = 999;

    bool AddWeapon(const FForge2BRWeapon& Weapon);
    bool AddItem(const FForge2BRLootItem& Item);
    bool AddAmmo(const FString& AmmoType, int32 Amount);
    void RemoveWeapon(int32 Index);
    void RemoveItem(int32 Index);
    int32 GetAmmoCount(const FString& AmmoType) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRPlayerStats
{
    GENERATED_BODY()

    UPROPERTY() int32 Kills = 0;
    UPROPERTY() int32 Assists = 0;
    UPROPERTY() int32 DamageDealt = 0;
    UPROPERTY() int32 DamageTaken = 0;
    UPROPERTY() int32 Revives = 0;
    UPROPERTY() float DistanceTraveled = 0.0f;
    UPROPERTY() int32 ItemsLooted = 0;
    UPROPERTY() float SurvivalTime = 0.0f;
    UPROPERTY() int32 Placement = 0;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRPlayer
{
    GENERATED_BODY()

    UPROPERTY() FString PlayerId;
    UPROPERTY() FString PlayerName;
    UPROPERTY() int32 TeamId = -1;
    UPROPERTY() EForge2BRPlayerState State = EForge2BRPlayerState::Alive;
    UPROPERTY() FVector Position = FVector::ZeroVector;
    UPROPERTY() float Health = 100.0f;
    UPROPERTY() float MaxHealth = 100.0f;
    UPROPERTY() float Shield = 0.0f;
    UPROPERTY() float MaxShield = 100.0f;
    UPROPERTY() float KnockdownTimer = 0.0f;
    UPROPERTY() float KnockdownDuration = 90.0f;
    UPROPERTY() FString KnockedBy;
    UPROPERTY() FForge2BRPlayerInventory Inventory;
    UPROPERTY() FForge2BRPlayerStats Stats;

    void Initialize(const FString& Id, const FString& Name, int32 Team);
    void Update(float DeltaTime);
    void TakeDamage(float Damage, const FString& AttackerId);
    void Heal(float Amount);
    void AddShield(float Amount);
    void Knock(const FString& AttackerId);
    void Eliminate();
    void Revive(float HealthPercent = 0.3f);
    float GetTotalHealth() const { return Health + Shield; }
    bool IsAlive() const { return State == EForge2BRPlayerState::Alive; }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRTeam
{
    GENERATED_BODY()

    UPROPERTY() int32 TeamId = -1;
    UPROPERTY() TArray<FString> PlayerIds;
    UPROPERTY() int32 Placement = 0;
    UPROPERTY() bool bEliminated = false;
    UPROPERTY() int32 TotalKills = 0;

    int32 GetAliveCount(const TMap<FString, FForge2BRPlayer>& Players) const;
    bool IsEliminated(const TMap<FString, FForge2BRPlayer>& Players) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRLootSpawn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SpawnId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BRLootTier MinTier = EForge2BRLootTier::Common;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BRLootTier MaxTier = EForge2BRLootTier::Rare;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpawnChance = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsHotDrop = false;
    UPROPERTY() bool bHasSpawned = false;
    UPROPERTY() TArray<FForge2BRLootItem> SpawnedItems;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRSupplyDrop
{
    GENERATED_BODY()

    UPROPERTY() FString DropId;
    UPROPERTY() FVector TargetLocation = FVector::ZeroVector;
    UPROPERTY() FVector CurrentPosition = FVector::ZeroVector;
    UPROPERTY() float DropSpeed = 500.0f;
    UPROPERTY() bool bLanded = false;
    UPROPERTY() bool bLooted = false;
    UPROPERTY() TArray<FForge2BRLootItem> Contents;
    UPROPERTY() float OpenTime = 5.0f;
    UPROPERTY() float TimeToLand = 0.0f;

    void Update(float DeltaTime);
    void GenerateContents(EForge2BRLootTier GuaranteedTier);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRVehicle
{
    GENERATED_BODY()

    UPROPERTY() FString VehicleId;
    UPROPERTY() EForge2BRVehicleType VehicleType = EForge2BRVehicleType::Car;
    UPROPERTY() FVector Position = FVector::ZeroVector;
    UPROPERTY() float Health = 1000.0f;
    UPROPERTY() float MaxHealth = 1000.0f;
    UPROPERTY() float Fuel = 100.0f;
    UPROPERTY() float MaxFuel = 100.0f;
    UPROPERTY() float FuelConsumption = 5.0f;
    UPROPERTY() int32 SeatCount = 4;
    UPROPERTY() float MaxSpeed = 100.0f;
    UPROPERTY() TArray<FString> OccupantIds;
    UPROPERTY() bool bIsDestroyed = false;

    void Update(float DeltaTime, float Throttle);
    void TakeDamage(float Damage);
    bool EnterVehicle(const FString& PlayerId, int32 SeatIndex);
    void ExitVehicle(const FString& PlayerId);
    int32 GetOccupantCount() const { return OccupantIds.Num(); }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRPingMarker
{
    GENERATED_BODY()

    UPROPERTY() FString PingId;
    UPROPERTY() FString SenderId;
    UPROPERTY() int32 TeamId = -1;
    UPROPERTY() FVector Location = FVector::ZeroVector;
    UPROPERTY() FString PingType; // Enemy, Loot, Location, Danger, etc.
    UPROPERTY() float Duration = 5.0f;
    UPROPERTY() float TimeRemaining = 5.0f;

    void Update(float DeltaTime) { TimeRemaining -= DeltaTime; }
    bool IsExpired() const { return TimeRemaining <= 0.0f; }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRDeployConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BRDropType DropType = EForge2BRDropType::Plane;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector PlaneStartPos = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector PlaneEndPos = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PlaneSpeed = 2000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PlaneAltitude = 3000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float JumpWindowStart = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float JumpWindowEnd = 0.8f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SkydiveSpeed = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GlideSpeed = 250.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ParachuteAltitude = 200.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRMatchConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxPlayers = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TeamSize = 1; // 1=Solo, 2=Duo, 3=Trio, 4=Squad
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2BRZoneConfig ZoneConfig;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2BRDeployConfig DeployConfig;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LobbyCountdown = 60.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinPlayersToStart = 20;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableVehicles = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableRespawn = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableBuilding = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReviveTime = 10.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BRMatchState
{
    GENERATED_BODY()

    UPROPERTY() EForge2BRMatchPhase Phase = EForge2BRMatchPhase::Lobby;
    UPROPERTY() float MatchTime = 0.0f;
    UPROPERTY() float PhaseTimer = 0.0f;
    UPROPERTY() int32 PlayersAlive = 0;
    UPROPERTY() int32 TeamsAlive = 0;
    UPROPERTY() int32 TotalPlayers = 0;
    UPROPERTY() FForge2BRZoneState ZoneState;
    UPROPERTY() float PlaneProgress = 0.0f;
    UPROPERTY() FVector PlanePosition = FVector::ZeroVector;
    UPROPERTY() int32 NextPlacement = 1;

    void Initialize(const FForge2BRMatchConfig& Config);
    void Update(float DeltaTime, const FForge2BRMatchConfig& Config);
    void UpdatePlanePosition(float DeltaTime, const FForge2BRDeployConfig& Config);
    void TransitionToPhase(EForge2BRMatchPhase NewPhase);
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2BattleRoyaleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2BattleRoyaleManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2BRMatchConfig Config;
    UPROPERTY(BlueprintReadOnly) FForge2BRMatchState MatchState;
    UPROPERTY() TMap<FString, FForge2BRPlayer> Players;
    UPROPERTY() TMap<int32, FForge2BRTeam> Teams;
    UPROPERTY() TArray<FForge2BRLootSpawn> LootSpawns;
    UPROPERTY() TArray<FForge2BRSupplyDrop> SupplyDrops;
    UPROPERTY() TArray<FForge2BRVehicle> Vehicles;
    UPROPERTY() TArray<FForge2BRPingMarker> Pings;
    UPROPERTY() TArray<FForge2BRWeapon> WeaponDatabase;
    UPROPERTY() TArray<FForge2BRLootItem> ItemDatabase;

    UFUNCTION(BlueprintCallable) void Initialize(const FForge2BRMatchConfig& InConfig);
    UFUNCTION(BlueprintCallable) void Update(float DeltaTime);

    // Player management
    UFUNCTION(BlueprintCallable) FString AddPlayer(const FString& PlayerName, int32 TeamId = -1);
    UFUNCTION(BlueprintCallable) void RemovePlayer(const FString& PlayerId);
    UFUNCTION(BlueprintCallable) void PlayerJump(const FString& PlayerId);
    UFUNCTION(BlueprintCallable) void PlayerDeploy(const FString& PlayerId, const FVector& LandingPos);

    // Combat
    UFUNCTION(BlueprintCallable) void ProcessDamage(const FString& AttackerId, const FString& VictimId, float Damage, bool bHeadshot);
    UFUNCTION(BlueprintCallable) void RevivePlayer(const FString& ReviverId, const FString& TargetId);

    // Loot
    UFUNCTION(BlueprintCallable) void SpawnLoot();
    UFUNCTION(BlueprintCallable) void PickupItem(const FString& PlayerId, const FString& SpawnId, int32 ItemIndex);
    UFUNCTION(BlueprintCallable) void DropItem(const FString& PlayerId, int32 InventoryIndex, const FVector& Location);
    UFUNCTION(BlueprintCallable) void SpawnSupplyDrop(const FVector& Location);

    // Vehicles
    UFUNCTION(BlueprintCallable) void EnterVehicle(const FString& PlayerId, const FString& VehicleId, int32 Seat);
    UFUNCTION(BlueprintCallable) void ExitVehicle(const FString& PlayerId);

    // Communication
    UFUNCTION(BlueprintCallable) void CreatePing(const FString& PlayerId, const FVector& Location, const FString& PingType);

    // Queries
    UFUNCTION(BlueprintPure) int32 GetPlayersAlive() const { return MatchState.PlayersAlive; }
    UFUNCTION(BlueprintPure) int32 GetTeamsAlive() const { return MatchState.TeamsAlive; }
    UFUNCTION(BlueprintPure) FForge2BRPlayer GetPlayer(const FString& PlayerId) const;
    UFUNCTION(BlueprintPure) bool IsMatchOver() const { return MatchState.Phase == EForge2BRMatchPhase::GameOver; }

private:
    void UpdatePlayers(float DeltaTime);
    void UpdateZone(float DeltaTime);
    void UpdateSupplyDrops(float DeltaTime);
    void UpdateVehicles(float DeltaTime);
    void UpdatePings(float DeltaTime);
    void CheckMatchEnd();
    void EliminatePlayer(const FString& PlayerId, const FString& KillerId);
    void UpdatePlacements();
    void SetupWeaponDatabase();
    void SetupItemDatabase();
    FForge2BRLootItem GenerateLootItem(EForge2BRLootTier MinTier, EForge2BRLootTier MaxTier);
    int32 CreateTeam();
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2BattleRoyaleUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "BR|Weapons")
    static FForge2BRWeapon CreateAssaultRifle(const FString& WeaponId, EForge2BRLootTier Tier);

    UFUNCTION(BlueprintCallable, Category = "BR|Weapons")
    static FForge2BRWeapon CreateShotgun(const FString& WeaponId, EForge2BRLootTier Tier);

    UFUNCTION(BlueprintCallable, Category = "BR|Weapons")
    static FForge2BRWeapon CreateSMG(const FString& WeaponId, EForge2BRLootTier Tier);

    UFUNCTION(BlueprintCallable, Category = "BR|Weapons")
    static FForge2BRWeapon CreateSniper(const FString& WeaponId, EForge2BRLootTier Tier);

    UFUNCTION(BlueprintCallable, Category = "BR|Items")
    static FForge2BRLootItem CreateHealingItem(const FString& ItemId, float HealAmount, float UseTime);

    UFUNCTION(BlueprintCallable, Category = "BR|Items")
    static FForge2BRLootItem CreateShieldItem(const FString& ItemId, float ShieldAmount, float UseTime);

    UFUNCTION(BlueprintCallable, Category = "BR|Config")
    static FForge2BRZoneConfig CreateStandardZoneConfig();

    UFUNCTION(BlueprintCallable, Category = "BR|Config")
    static FForge2BRZoneConfig CreateFastZoneConfig();

    UFUNCTION(BlueprintCallable, Category = "BR|Vehicles")
    static FForge2BRVehicle CreateCar(const FString& VehicleId, const FVector& Position);

    UFUNCTION(BlueprintCallable, Category = "BR|Vehicles")
    static FForge2BRVehicle CreateHelicopter(const FString& VehicleId, const FVector& Position);
};
