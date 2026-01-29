// Forge2MOBASystem.h
// FORGE2 MOBA Genre System
// Provides MOBA mechanics: heroes, abilities, lanes, minions, towers, jungle, items

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Forge2MOBASystem.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EForge2Lane : uint8 { Top, Mid, Bot, Jungle };

UENUM(BlueprintType)
enum class EForge2HeroRole : uint8 { Carry, Support, Tank, Mage, Assassin, Fighter, Marksman };

UENUM(BlueprintType)
enum class EForge2AbilityType : uint8 { Passive, Active, Toggle, Ultimate, Summon };

UENUM(BlueprintType)
enum class EForge2TargetType : uint8 { Self, SingleTarget, PointTarget, Cone, Circle, Line, Global };

UENUM(BlueprintType)
enum class EForge2ObjectiveType : uint8 { Tower, Inhibitor, Nexus, Dragon, Baron, Buff, Herald };

// ============================================================================
// Hero Abilities
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HeroAbility
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AbilityID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString AbilityName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2AbilityType Type = EForge2AbilityType::Active;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2TargetType TargetType = EForge2TargetType::SingleTarget;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> ManaCost;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> Cooldown;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> BaseDamage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ADScaling = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float APScaling = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxRank = 5;

    float GetDamage(int32 Rank, float AD, float AP) const;
    float GetCooldown(int32 Rank) const;
    float GetManaCost(int32 Rank) const;
};

// ============================================================================
// Hero Definition
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HeroDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName HeroID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString HeroName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2HeroRole PrimaryRole = EForge2HeroRole::Fighter;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2HeroRole SecondaryRole = EForge2HeroRole::Fighter;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseHealth = 600.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HealthPerLevel = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseMana = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ManaPerLevel = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseAD = 60.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ADPerLevel = 3.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseArmor = 30.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseMR = 30.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseMS = 335.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackSpeed = 0.65f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackRange = 125.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2HeroAbility> Abilities;

    float GetMaxHealth(int32 Level) const { return BaseHealth + HealthPerLevel * (Level - 1); }
    float GetAD(int32 Level) const { return BaseAD + ADPerLevel * (Level - 1); }
};

// ============================================================================
// Hero Instance
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HeroInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName InstanceID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName HeroID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TeamID = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Level = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 XP = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CurrentHealth = 600.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CurrentMana = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> AbilityRanks;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> AbilityCooldowns;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> Items;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Gold = 500;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Kills = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Deaths = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Assists = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CS = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsDead = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RespawnTimer = 0.0f;

    void GainXP(int32 Amount);
    void LevelUp();
    int32 GetXPForNextLevel() const;
    FString GetKDA() const;
};

// ============================================================================
// Lane and Minions
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Minion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName InstanceID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TeamID = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2Lane Lane = EForge2Lane::Mid;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Type = TEXT("Melee");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Health = 450.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxHealth = 450.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damage = 12.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GoldValue = 21.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float XPValue = 59.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location = FVector::ZeroVector;

    void TakeDamage(float Amount);
    bool IsDead() const { return Health <= 0.0f; }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MinionWave
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 WaveNumber = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2Minion> Minions;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2Lane Lane = EForge2Lane::Mid;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpawnTime = 0.0f;

    void SpawnWave(int32 TeamID, EForge2Lane TargetLane, float GameTime);
};

// ============================================================================
// Structures
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Tower
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TowerID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TeamID = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2Lane Lane = EForge2Lane::Mid;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TierIndex = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Health = 3500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxHealth = 3500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damage = 150.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 750.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsDestroyed = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName CurrentTargetID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackCooldown = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GoldReward = 250;

    void TakeDamage(float Amount);
    void UpdateTargeting(const TArray<FForge2HeroInstance>& Heroes, const TArray<FForge2Minion>& Minions);
};

// ============================================================================
// Jungle
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2JungleCamp
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName CampID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString CampName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2ObjectiveType Type = EForge2ObjectiveType::Buff;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Health = 1800.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxHealth = 1800.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damage = 70.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GoldValue = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float XPValue = 200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RespawnTime = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RespawnTimer = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsAlive = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName BuffGranted;

    void TakeDamage(float Amount);
    void UpdateRespawn(float DeltaTime);
};

// ============================================================================
// Items
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MOBAItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ItemName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Cost = 400;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> BuildsFrom;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusAD = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusAP = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusHealth = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusMana = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusArmor = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusMR = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusMS = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusAS = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusCDR = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BonusLifesteal = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName PassiveID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ActiveID;

    int32 GetTotalCost(const TMap<FName, FForge2MOBAItem>& AllItems) const;
};

// ============================================================================
// Match State
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MOBAMatchState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GameTime = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 WaveCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NextWaveTime = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WaveInterval = 30.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> TeamKills;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> TeamGold;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> DragonCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> BaronCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bGameOver = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 WinningTeam = -1;
};

// ============================================================================
// MOBA Manager
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2MOBAManager : public UObject
{
    GENERATED_BODY()

public:
    UForge2MOBAManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2HeroDefinition> HeroPool;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2HeroInstance> Heroes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2MinionWave> ActiveWaves;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2Tower> Towers;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2JungleCamp> JungleCamps;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FName, FForge2MOBAItem> ItemShop;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2MOBAMatchState MatchState;

    UFUNCTION(BlueprintCallable) void InitializeMatch();
    UFUNCTION(BlueprintCallable) void Update(float DeltaTime);
    UFUNCTION(BlueprintCallable) void SpawnHero(FName HeroID, int32 TeamID);
    UFUNCTION(BlueprintCallable) void UseAbility(FName HeroInstanceID, int32 AbilityIndex, FVector TargetLocation, FName TargetID);
    UFUNCTION(BlueprintCallable) bool BuyItem(FName HeroInstanceID, FName ItemID);
    UFUNCTION(BlueprintCallable) void AttackTarget(FName AttackerID, FName TargetID);
    UFUNCTION(BlueprintCallable) void OnHeroKill(FName KillerID, FName VictimID);

private:
    void UpdateMinions(float DeltaTime);
    void UpdateTowers(float DeltaTime);
    void UpdateJungle(float DeltaTime);
    void UpdateHeroes(float DeltaTime);
    void SpawnMinionWave();
    void CheckVictoryConditions();
};

// ============================================================================
// Utility
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2MOBAUtilities : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable) static FForge2HeroDefinition CreateFighterHero(const FString& Name);
    UFUNCTION(BlueprintCallable) static FForge2HeroDefinition CreateMageHero(const FString& Name);
    UFUNCTION(BlueprintCallable) static FForge2HeroDefinition CreateMarksmanHero(const FString& Name);
    UFUNCTION(BlueprintCallable) static FForge2HeroAbility CreateDamageAbility(const FString& Name, float BaseDmg, float ADRatio, float APRatio);
    UFUNCTION(BlueprintCallable) static FForge2Tower CreateTower(int32 TeamID, EForge2Lane Lane, int32 Tier, const FVector& Location);
    UFUNCTION(BlueprintCallable) static FForge2JungleCamp CreateBuffCamp(const FString& Name, const FVector& Location, FName BuffID);
    UFUNCTION(BlueprintCallable) static FForge2MOBAItem CreateADItem(const FString& Name, int32 Cost, float AD);
    UFUNCTION(BlueprintCallable) static FForge2MOBAItem CreateAPItem(const FString& Name, int32 Cost, float AP);
};
