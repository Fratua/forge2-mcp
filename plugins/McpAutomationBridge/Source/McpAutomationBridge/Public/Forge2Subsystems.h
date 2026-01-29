// Copyright Epic Games, Inc. All Rights Reserved.
// FORGE2 Subsystems - World and Game Instance Subsystems for MCP Integration

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Forge2Subsystems.generated.h"

// Forward declarations
class UMcpQuestDataAsset;
class UMcpDialogueTreeAsset;
class UNiagaraSystem;
class USoundBase;

// ============================================================================
// Quest Subsystem Structures
// ============================================================================

UENUM(BlueprintType)
enum class EMcpQuestState : uint8
{
	Unavailable,
	Available,
	Active,
	Completed,
	Failed
};

USTRUCT(BlueprintType)
struct FMcpActiveQuest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FString QuestId;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	EMcpQuestState State;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TMap<FString, int32> ObjectiveProgress;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	float StartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	float CompletionTime;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, const FString&, QuestId, EMcpQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveProgress, const FString&, QuestId, const FString&, ObjectiveId, int32, Progress);

// ============================================================================
// Economy Subsystem Structures
// ============================================================================

USTRUCT(BlueprintType)
struct FMcpShopItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FString ItemId;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	int32 Price;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	int32 Stock;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	int32 MaxStock;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	float RestockTime;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	float LastRestockTime;
};

USTRUCT(BlueprintType)
struct FMcpShop
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FString ShopId;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FString ShopName;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	TArray<FMcpShopItem> Items;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	float BuyPriceMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	float SellPriceMultiplier = 0.5f;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FString RequiredFaction;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	int32 RequiredFactionRank = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FName CurrencyId = FName(TEXT("Gold"));
};

USTRUCT(BlueprintType)
struct FMcpAuctionListing
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FString ListingId;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FString SellerId;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FString ItemId;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	int32 Quantity;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	int32 CurrentBid;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	int32 BuyoutPrice;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	FString HighestBidderId;

	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	float ExpirationTime;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTransactionComplete, const FString&, BuyerId, const FString&, ItemId, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTransactionFailed, const FString&, BuyerId, const FString&, ItemId, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuctionEnded, const FString&, ListingId, const FString&, WinnerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAuctionBidPlaced, const FString&, ListingId, const FString&, BidderId, int32, BidAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuctionCancelled, const FString&, ListingId);

// ============================================================================
// Spawn Subsystem Structures
// ============================================================================

UENUM(BlueprintType)
enum class EMcpSpawnState : uint8
{
	Idle,
	Spawning,
	Active,
	Cooldown
};

USTRUCT(BlueprintType)
struct FMcpSpawnPoint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	FString SpawnPointId;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	FVector Location;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	FRotator Rotation;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	float RespawnDelay;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	int32 MaxActiveSpawns;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	float SpawnRadius;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	TArray<FString> SpawnTags;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	EMcpSpawnState State;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	float LastSpawnTime;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	int32 CurrentActiveCount;
};

USTRUCT(BlueprintType)
struct FMcpWaveSpawner
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	FString WaveSpawnerId;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	TArray<FMcpSpawnPoint> SpawnPoints;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	int32 CurrentWave;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	int32 TotalWaves;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	float WaveDelay;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	int32 EnemiesPerWave;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	float DifficultyScaling;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	bool bWaveInProgress;

	UPROPERTY(BlueprintReadWrite, Category = "Spawn")
	int32 RemainingEnemies;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorSpawned, AActor*, SpawnedActor, const FString&, SpawnPointId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStarted, const FString&, WaveSpawnerId, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveCompleted, const FString&, WaveSpawnerId, int32, WaveNumber);

// ============================================================================
// Weather Subsystem Structures
// ============================================================================

UENUM(BlueprintType)
enum class EMcpWeatherType : uint8
{
	Clear,
	Cloudy,
	Overcast,
	Rain,
	Storm,
	Snow,
	Blizzard,
	Fog,
	Sandstorm
};

UENUM(BlueprintType)
enum class EMcpSeason : uint8
{
	Spring,
	Summer,
	Autumn,
	Winter
};

USTRUCT(BlueprintType)
struct FMcpWeatherState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Weather")
	EMcpWeatherType WeatherType;

	UPROPERTY(BlueprintReadWrite, Category = "Weather")
	float Intensity;

	UPROPERTY(BlueprintReadWrite, Category = "Weather")
	float Temperature;

	UPROPERTY(BlueprintReadWrite, Category = "Weather")
	float WindSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "Weather")
	FVector WindDirection;

	UPROPERTY(BlueprintReadWrite, Category = "Weather")
	float Humidity;

	UPROPERTY(BlueprintReadWrite, Category = "Weather")
	float CloudCoverage;

	UPROPERTY(BlueprintReadWrite, Category = "Weather")
	float Visibility;
};

USTRUCT(BlueprintType)
struct FMcpTimeOfDay
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	int32 Hour;

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	int32 Minute;

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	int32 Day;

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	int32 Month;

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	int32 Year;

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	EMcpSeason Season;

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	float NormalizedTime; // 0-1, 0 = midnight, 0.5 = noon

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	float DayLength; // Real seconds per game day

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	bool bIsDaytime;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EMcpWeatherType, OldWeather, EMcpWeatherType, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeasonChanged, EMcpSeason, NewSeason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayNightChanged, bool, bIsDaytime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHourChanged, int32, NewHour);

// ============================================================================
// World Generation Subsystem Structures
// ============================================================================

UENUM(BlueprintType)
enum class EMcpBiomeType : uint8
{
	Forest,
	Desert,
	Tundra,
	Jungle,
	Swamp,
	Mountain,
	Plains,
	Ocean,
	Volcanic,
	Arctic
};

USTRUCT(BlueprintType)
struct FMcpWorldChunk
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	FIntVector ChunkCoord;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	EMcpBiomeType Biome;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	bool bIsLoaded;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	bool bIsGenerated;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	TArray<AActor*> ChunkActors;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	float GenerationSeed;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	int32 LODLevel;
};

USTRUCT(BlueprintType)
struct FMcpWorldGenSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	int32 WorldSeed;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	float ChunkSize;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	int32 ViewDistance;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	float TerrainScale;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	int32 TerrainOctaves;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	float TerrainPersistence;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	float BiomeScale;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	float VegetationDensity;

	UPROPERTY(BlueprintReadWrite, Category = "WorldGen")
	float StructureDensity;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChunkLoaded, FIntVector, ChunkCoord);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChunkUnloaded, FIntVector, ChunkCoord);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeEntered, EMcpBiomeType, OldBiome, EMcpBiomeType, NewBiome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStructureSpawned, FIntVector, ChunkCoord, AActor*, Structure);

// ============================================================================
// Dialogue Subsystem Structures
// ============================================================================

UENUM(BlueprintType)
enum class EMcpDialogueNodeType : uint8
{
	NpcSpeech,
	PlayerChoice,
	Condition,
	Event,
	Random,
	End
};

USTRUCT(BlueprintType)
struct FMcpDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString ChoiceId;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FText ChoiceText;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString NextNodeId;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	TMap<FString, FString> Conditions;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	bool bIsAvailable;
};

USTRUCT(BlueprintType)
struct FMcpActiveDialogue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString DialogueId;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString CurrentNodeId;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	AActor* Speaker;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	AActor* Listener;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FText CurrentText;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	TArray<FMcpDialogueChoice> CurrentChoices;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	bool bIsActive;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, DialogueId, AActor*, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, DialogueId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueChoiceMade, const FString&, DialogueId, const FString&, ChoiceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueNodeChanged, const FString&, DialogueId, const FString&, OldNodeId, const FString&, NewNodeId);

// ============================================================================
// Audio Subsystem Structures
// ============================================================================

UENUM(BlueprintType)
enum class EMcpMusicState : uint8
{
	Exploration,
	Combat,
	Boss,
	Tension,
	Victory,
	Defeat,
	Menu,
	Cinematic,
	Ambient
};

USTRUCT(BlueprintType)
struct FMcpMusicLayer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	FString LayerId;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	USoundBase* Sound;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float Volume;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float TargetVolume;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float FadeSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	bool bIsPlaying;
};

USTRUCT(BlueprintType)
struct FMcpAmbientZone
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	FString ZoneId;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	FVector Center;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float Radius;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	TArray<USoundBase*> AmbientSounds;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float VolumeMultiplier;

	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float Priority;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMusicStateChanged, EMcpMusicState, OldState, EMcpMusicState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmbientZoneEntered, const FString&, ZoneId, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmbientZoneExited, const FString&, ZoneId, AActor*, Actor);

// ============================================================================
// Analytics Subsystem Structures
// ============================================================================

UENUM(BlueprintType)
enum class EMcpAnalyticsEventType : uint8
{
	GameStart,
	GameEnd,
	LevelStart,
	LevelComplete,
	PlayerDeath,
	EnemyKill,
	ItemPickup,
	QuestStart,
	QuestComplete,
	Achievement,
	Purchase,
	Custom
};

USTRUCT(BlueprintType)
struct FMcpAnalyticsEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString EventId;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	EMcpAnalyticsEventType EventType;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString EventName;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	TMap<FString, FString> Parameters;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FVector Location;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString PlayerId;
};

USTRUCT(BlueprintType)
struct FMcpSessionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float StartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float TotalPlayTime;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 DeathCount;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 EnemiesKilled;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 QuestsCompleted;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	TArray<FMcpAnalyticsEvent> Events;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalyticsEventRecorded, const FMcpAnalyticsEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStarted, const FString&, SessionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionEnded, const FString&, SessionId, float, Duration);

// ============================================================================
// QUEST SUBSYSTEM
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UMcpQuestSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// Quest Management
	UFUNCTION(BlueprintCallable, Category = "MCP|Quest")
	bool StartQuest(const FString& QuestId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Quest")
	bool CompleteQuest(const FString& QuestId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Quest")
	bool FailQuest(const FString& QuestId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Quest")
	bool AbandonQuest(const FString& QuestId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Quest")
	void UpdateObjectiveProgress(const FString& QuestId, const FString& ObjectiveId, int32 Progress);

	UFUNCTION(BlueprintCallable, Category = "MCP|Quest")
	bool IsQuestAvailable(const FString& QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "MCP|Quest")
	bool IsQuestActive(const FString& QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "MCP|Quest")
	bool IsQuestCompleted(const FString& QuestId) const;

	UFUNCTION(BlueprintPure, Category = "MCP|Quest")
	EMcpQuestState GetQuestState(const FString& QuestId) const;

	UFUNCTION(BlueprintPure, Category = "MCP|Quest")
	FMcpActiveQuest GetActiveQuest(const FString& QuestId) const;

	UFUNCTION(BlueprintPure, Category = "MCP|Quest")
	TArray<FMcpActiveQuest> GetAllActiveQuests() const;

	UFUNCTION(BlueprintPure, Category = "MCP|Quest")
	int32 GetObjectiveProgress(const FString& QuestId, const FString& ObjectiveId) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "MCP|Quest")
	FOnQuestStateChanged OnQuestStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Quest")
	FOnObjectiveProgress OnObjectiveProgress;

protected:
	UPROPERTY()
	TMap<FString, FMcpActiveQuest> ActiveQuests;

	UPROPERTY()
	TSet<FString> CompletedQuests;

	UPROPERTY()
	TSet<FString> FailedQuests;

	void CheckQuestPrerequisites(const FString& QuestId);
	void GrantQuestRewards(const FString& QuestId);
};

// ============================================================================
// ECONOMY SUBSYSTEM
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UMcpEconomySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// Shop Management
	UFUNCTION(BlueprintCallable, Category = "MCP|Economy")
	bool RegisterShop(const FMcpShop& Shop);

	UFUNCTION(BlueprintCallable, Category = "MCP|Economy")
	bool BuyItem(const FString& ShopId, const FString& ItemId, int32 Quantity, AActor* Buyer);

	UFUNCTION(BlueprintCallable, Category = "MCP|Economy")
	bool SellItem(const FString& ShopId, const FString& ItemId, int32 Quantity, AActor* Seller);

	UFUNCTION(BlueprintPure, Category = "MCP|Economy")
	FMcpShop GetShop(const FString& ShopId) const;

	UFUNCTION(BlueprintPure, Category = "MCP|Economy")
	int32 GetItemPrice(const FString& ShopId, const FString& ItemId, bool bBuying) const;

	// Auction House
	UFUNCTION(BlueprintCallable, Category = "MCP|Economy")
	FString CreateAuctionListing(const FString& SellerId, const FString& ItemId, int32 Quantity, int32 StartingBid, int32 BuyoutPrice, float Duration);

	UFUNCTION(BlueprintCallable, Category = "MCP|Economy")
	bool PlaceBid(const FString& ListingId, const FString& BidderId, int32 BidAmount);

	UFUNCTION(BlueprintCallable, Category = "MCP|Economy")
	bool BuyoutListing(const FString& ListingId, const FString& BuyerId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Economy")
	bool CancelListing(const FString& ListingId, const FString& SellerId);

	UFUNCTION(BlueprintPure, Category = "MCP|Economy")
	TArray<FMcpAuctionListing> SearchAuctions(const FString& ItemIdFilter, int32 MaxPrice) const;

	UFUNCTION(BlueprintCallable, Category = "MCP|Economy")
	void ProcessExpiredAuctions();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "MCP|Economy")
	FOnTransactionComplete OnTransactionComplete;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Economy")
	FOnTransactionFailed OnTransactionFailed;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Economy")
	FOnAuctionEnded OnAuctionEnded;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Economy")
	FOnAuctionBidPlaced OnAuctionBidPlaced;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Economy")
	FOnAuctionCancelled OnAuctionCancelled;

protected:
	UPROPERTY()
	TMap<FString, FMcpShop> RegisteredShops;

	UPROPERTY()
	TMap<FString, FMcpAuctionListing> AuctionListings;

	UPROPERTY()
	FName DefaultCurrencyId = FName(TEXT("Gold"));

	void UpdateShopStock();
	float GetReputationPriceModifier(AActor* Actor, const FString& FactionId) const;
	
	// Find actor by string ID (player name, unique ID, etc.)
	AActor* FindActorById(const FString& ActorId) const;
};

// ============================================================================
// SPAWN SUBSYSTEM
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UMcpSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual TStatId GetStatId() const override;

	// Spawn Point Management
	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	bool RegisterSpawnPoint(const FMcpSpawnPoint& SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	bool UnregisterSpawnPoint(const FString& SpawnPointId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	AActor* SpawnAtPoint(const FString& SpawnPointId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	void OnSpawnedActorDestroyed(const FString& SpawnPointId);

	UFUNCTION(BlueprintPure, Category = "MCP|Spawn")
	FMcpSpawnPoint GetSpawnPoint(const FString& SpawnPointId) const;

	// Wave Spawner
	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	bool RegisterWaveSpawner(const FMcpWaveSpawner& WaveSpawner);

	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	bool StartWaveSpawner(const FString& WaveSpawnerId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	bool StopWaveSpawner(const FString& WaveSpawnerId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	void OnWaveEnemyKilled(const FString& WaveSpawnerId);

	UFUNCTION(BlueprintPure, Category = "MCP|Spawn")
	FMcpWaveSpawner GetWaveSpawner(const FString& WaveSpawnerId) const;

	// Object Pooling
	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	void PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	AActor* GetPooledActor(TSubclassOf<AActor> ActorClass);

	UFUNCTION(BlueprintCallable, Category = "MCP|Spawn")
	void ReturnToPool(AActor* Actor);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "MCP|Spawn")
	FOnActorSpawned OnActorSpawned;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Spawn")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Spawn")
	FOnWaveCompleted OnWaveCompleted;

protected:
	UPROPERTY()
	TMap<FString, FMcpSpawnPoint> SpawnPoints;

	UPROPERTY()
	TMap<FString, FMcpWaveSpawner> WaveSpawners;

	UPROPERTY()
	TMap<UClass*, TArray<AActor*>> ObjectPools;

	void ProcessSpawnPoints(float DeltaTime);
	void ProcessWaveSpawners(float DeltaTime);
	void SpawnNextWave(FMcpWaveSpawner& WaveSpawner);
};

// ============================================================================
// WEATHER SUBSYSTEM
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UMcpWeatherSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual TStatId GetStatId() const override;

	// Time Management
	UFUNCTION(BlueprintCallable, Category = "MCP|Weather")
	void SetTime(int32 Hour, int32 Minute);

	UFUNCTION(BlueprintCallable, Category = "MCP|Weather")
	void SetDate(int32 Day, int32 Month, int32 Year);

	UFUNCTION(BlueprintCallable, Category = "MCP|Weather")
	void SetTimeScale(float Scale);

	UFUNCTION(BlueprintCallable, Category = "MCP|Weather")
	void PauseTime(bool bPause);

	UFUNCTION(BlueprintPure, Category = "MCP|Weather")
	FMcpTimeOfDay GetCurrentTime() const;

	UFUNCTION(BlueprintPure, Category = "MCP|Weather")
	bool IsDaytime() const;

	UFUNCTION(BlueprintPure, Category = "MCP|Weather")
	float GetSunAngle() const;

	// Weather Management
	UFUNCTION(BlueprintCallable, Category = "MCP|Weather")
	void SetWeather(EMcpWeatherType NewWeather, float TransitionTime = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "MCP|Weather")
	void SetWeatherIntensity(float Intensity);

	UFUNCTION(BlueprintPure, Category = "MCP|Weather")
	FMcpWeatherState GetCurrentWeather() const;

	UFUNCTION(BlueprintPure, Category = "MCP|Weather")
	EMcpWeatherType GetWeatherType() const;

	UFUNCTION(BlueprintPure, Category = "MCP|Weather")
	float GetTemperature() const;

	// Season
	UFUNCTION(BlueprintCallable, Category = "MCP|Weather")
	void SetSeason(EMcpSeason NewSeason);

	UFUNCTION(BlueprintPure, Category = "MCP|Weather")
	EMcpSeason GetCurrentSeason() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "MCP|Weather")
	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Weather")
	FOnSeasonChanged OnSeasonChanged;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Weather")
	FOnDayNightChanged OnDayNightChanged;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Weather")
	FOnHourChanged OnHourChanged;

protected:
	UPROPERTY()
	FMcpTimeOfDay CurrentTime;

	UPROPERTY()
	FMcpWeatherState CurrentWeather;

	UPROPERTY()
	FMcpWeatherState TargetWeather;

	UPROPERTY()
	float TimeScale;

	UPROPERTY()
	bool bTimePaused;

	UPROPERTY()
	float WeatherTransitionProgress;

	UPROPERTY()
	float WeatherTransitionDuration;

	// Weather effect components
	UPROPERTY()
	class UNiagaraComponent* WeatherParticleComponent;

	UPROPERTY()
	class UAudioComponent* WeatherAudioComponent;

	UPROPERTY()
	class USoundBase* WeatherAmbientSound;

	void UpdateTime(float DeltaTime);
	void UpdateWeatherTransition(float DeltaTime);
	void UpdateSeasonFromDate();
	void ApplyWeatherEffects();
};

// Forward declaration
class UNiagaraComponent;
class UAudioComponent;

// ============================================================================
// WORLD GENERATION SUBSYSTEM
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UMcpWorldGenSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual TStatId GetStatId() const override;

	// World Settings
	UFUNCTION(BlueprintCallable, Category = "MCP|WorldGen")
	void InitializeWorld(const FMcpWorldGenSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "MCP|WorldGen")
	void SetWorldSeed(int32 Seed);

	UFUNCTION(BlueprintPure, Category = "MCP|WorldGen")
	FMcpWorldGenSettings GetWorldSettings() const;

	// Chunk Management
	UFUNCTION(BlueprintCallable, Category = "MCP|WorldGen")
	void UpdatePlayerPosition(FVector PlayerPosition);

	UFUNCTION(BlueprintCallable, Category = "MCP|WorldGen")
	bool LoadChunk(FIntVector ChunkCoord);

	UFUNCTION(BlueprintCallable, Category = "MCP|WorldGen")
	bool UnloadChunk(FIntVector ChunkCoord);

	UFUNCTION(BlueprintCallable, Category = "MCP|WorldGen")
	bool GenerateChunk(FIntVector ChunkCoord);

	UFUNCTION(BlueprintPure, Category = "MCP|WorldGen")
	FMcpWorldChunk GetChunk(FIntVector ChunkCoord) const;

	UFUNCTION(BlueprintPure, Category = "MCP|WorldGen")
	bool IsChunkLoaded(FIntVector ChunkCoord) const;

	UFUNCTION(BlueprintPure, Category = "MCP|WorldGen")
	TArray<FIntVector> GetLoadedChunks() const;

	// Biome Queries
	UFUNCTION(BlueprintPure, Category = "MCP|WorldGen")
	EMcpBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "MCP|WorldGen")
	float GetHeightAtLocation(FVector2D WorldLocation) const;

	// Procedural Generation
	UFUNCTION(BlueprintCallable, Category = "MCP|WorldGen")
	float SampleNoise(FVector2D Location, float Scale, int32 Octaves, float Persistence) const;

	UFUNCTION(BlueprintCallable, Category = "MCP|WorldGen")
	FVector2D SampleNoise2D(FVector2D Location, float Scale) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "MCP|WorldGen")
	FOnChunkLoaded OnChunkLoaded;

	UPROPERTY(BlueprintAssignable, Category = "MCP|WorldGen")
	FOnChunkUnloaded OnChunkUnloaded;

	UPROPERTY(BlueprintAssignable, Category = "MCP|WorldGen")
	FOnBiomeEntered OnBiomeEntered;

	UPROPERTY(BlueprintAssignable, Category = "MCP|WorldGen")
	FOnStructureSpawned OnStructureSpawned;

protected:
	UPROPERTY()
	FMcpWorldGenSettings WorldSettings;

	UPROPERTY()
	TMap<FIntVector, FMcpWorldChunk> LoadedChunks;

	UPROPERTY()
	FIntVector CurrentPlayerChunk;

	UPROPERTY()
	EMcpBiomeType CurrentPlayerBiome;

	FIntVector WorldToChunkCoord(FVector WorldLocation) const;
	void StreamChunksAroundPlayer();
	void GenerateChunkTerrain(FMcpWorldChunk& Chunk);
	void GenerateChunkVegetation(FMcpWorldChunk& Chunk);
	void GenerateChunkStructures(FMcpWorldChunk& Chunk);
	EMcpBiomeType CalculateBiome(FIntVector ChunkCoord) const;
	
	// Helper functions for biome-specific content
	FLinearColor GetBiomeColor(EMcpBiomeType Biome, float Height) const;
	class UMaterialInterface* GetBiomeMaterial(EMcpBiomeType Biome) const;
	TArray<class UStaticMesh*> GetBiomeVegetation(EMcpBiomeType Biome) const;
	TArray<TSubclassOf<AActor>> GetBiomeStructures(EMcpBiomeType Biome) const;
};

// ============================================================================
// DIALOGUE SUBSYSTEM
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UMcpDialogueSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// Dialogue Control
	UFUNCTION(BlueprintCallable, Category = "MCP|Dialogue")
	bool StartDialogue(const FString& DialogueId, AActor* Speaker, AActor* Listener);

	UFUNCTION(BlueprintCallable, Category = "MCP|Dialogue")
	bool AdvanceDialogue(const FString& DialogueId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Dialogue")
	bool SelectChoice(const FString& DialogueId, const FString& ChoiceId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Dialogue")
	bool EndDialogue(const FString& DialogueId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Dialogue")
	bool SkipToNode(const FString& DialogueId, const FString& NodeId);

	// Query
	UFUNCTION(BlueprintPure, Category = "MCP|Dialogue")
	FMcpActiveDialogue GetActiveDialogue(const FString& DialogueId) const;

	UFUNCTION(BlueprintPure, Category = "MCP|Dialogue")
	bool IsDialogueActive(const FString& DialogueId) const;

	UFUNCTION(BlueprintPure, Category = "MCP|Dialogue")
	bool HasActiveDialogue() const;

	UFUNCTION(BlueprintPure, Category = "MCP|Dialogue")
	TArray<FMcpDialogueChoice> GetCurrentChoices(const FString& DialogueId) const;

	// Variables
	UFUNCTION(BlueprintCallable, Category = "MCP|Dialogue")
	void SetDialogueVariable(const FString& VariableName, const FString& Value);

	UFUNCTION(BlueprintPure, Category = "MCP|Dialogue")
	FString GetDialogueVariable(const FString& VariableName) const;

	UFUNCTION(BlueprintCallable, Category = "MCP|Dialogue")
	bool CheckDialogueCondition(const FString& ConditionType, const FString& ConditionValue) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "MCP|Dialogue")
	FOnDialogueStarted OnDialogueStarted;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Dialogue")
	FOnDialogueEnded OnDialogueEnded;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Dialogue")
	FOnDialogueChoiceMade OnDialogueChoiceMade;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Dialogue")
	FOnDialogueNodeChanged OnDialogueNodeChanged;

protected:
	UPROPERTY()
	TMap<FString, FMcpActiveDialogue> ActiveDialogues;

	UPROPERTY()
	TMap<FString, FString> DialogueVariables;

	void ProcessDialogueNode(FMcpActiveDialogue& Dialogue, const FString& NodeId);
	void ExecuteDialogueConsequence(const FString& ConsequenceType, const FString& ConsequenceValue);
	void UpdateAvailableChoices(FMcpActiveDialogue& Dialogue);
};

// ============================================================================
// AUDIO SUBSYSTEM
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UMcpAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual TStatId GetStatId() const override;

	// Music State
	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void SetMusicState(EMcpMusicState NewState, float TransitionTime = 2.0f);

	UFUNCTION(BlueprintPure, Category = "MCP|Audio")
	EMcpMusicState GetMusicState() const;

	// Music Layers
	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	bool RegisterMusicLayer(const FString& LayerId, USoundBase* Sound, EMcpMusicState State);

	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void SetLayerVolume(const FString& LayerId, float Volume, float FadeTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void MuteLayer(const FString& LayerId, bool bMute);

	// Stingers
	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void PlayStinger(USoundBase* StingerSound, bool bInterruptCurrent = false);

	// Ambient Zones
	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	bool RegisterAmbientZone(const FMcpAmbientZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void UnregisterAmbientZone(const FString& ZoneId);

	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void UpdateListenerPosition(FVector ListenerPosition);

	// Volume Control
	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void SetAmbientVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "MCP|Audio")
	void SetDialogueVolume(float Volume);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "MCP|Audio")
	FOnMusicStateChanged OnMusicStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Audio")
	FOnAmbientZoneEntered OnAmbientZoneEntered;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Audio")
	FOnAmbientZoneExited OnAmbientZoneExited;

protected:
	UPROPERTY()
	EMcpMusicState CurrentMusicState;

	UPROPERTY()
	TMap<FString, FMcpMusicLayer> MusicLayers;

	UPROPERTY()
	TMap<FString, FMcpAmbientZone> AmbientZones;

	UPROPERTY()
	TArray<FString> ActiveAmbientZones;

	UPROPERTY()
	float MasterVolume;

	UPROPERTY()
	float MusicVolume;

	UPROPERTY()
	float SFXVolume;

	UPROPERTY()
	float AmbientVolume;

	UPROPERTY()
	float DialogueVolume;

	void UpdateMusicLayers(float DeltaTime);
	void UpdateAmbientZones(FVector ListenerPosition);
	void TransitionMusicState(EMcpMusicState NewState, float TransitionTime);
};

// ============================================================================
// ANALYTICS SUBSYSTEM (Game Instance Subsystem)
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UMcpAnalyticsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void EndSession();

	UFUNCTION(BlueprintPure, Category = "MCP|Analytics")
	FMcpSessionData GetCurrentSession() const;

	UFUNCTION(BlueprintPure, Category = "MCP|Analytics")
	FString GetSessionId() const;

	// Event Recording
	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void RecordEvent(EMcpAnalyticsEventType EventType, const FString& EventName, const TMap<FString, FString>& Parameters);

	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void RecordEventAtLocation(EMcpAnalyticsEventType EventType, const FString& EventName, FVector Location, const TMap<FString, FString>& Parameters);

	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void RecordPlayerDeath(FVector Location, const FString& CauseOfDeath);

	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void RecordEnemyKill(const FString& EnemyType, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void RecordQuestEvent(const FString& QuestId, bool bCompleted);

	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void RecordItemPickup(const FString& ItemId, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void RecordPurchase(const FString& ItemId, int32 Price, const FString& Currency);

	// Heatmap Data
	UFUNCTION(BlueprintPure, Category = "MCP|Analytics")
	TArray<FVector> GetDeathLocations() const;

	UFUNCTION(BlueprintPure, Category = "MCP|Analytics")
	TArray<FMcpAnalyticsEvent> GetEventsOfType(EMcpAnalyticsEventType EventType) const;

	// Export
	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	bool ExportSessionData(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "MCP|Analytics")
	void ClearSessionData();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "MCP|Analytics")
	FOnAnalyticsEventRecorded OnAnalyticsEventRecorded;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Analytics")
	FOnSessionStarted OnSessionStarted;

	UPROPERTY(BlueprintAssignable, Category = "MCP|Analytics")
	FOnSessionEnded OnSessionEnded;

protected:
	UPROPERTY()
	FMcpSessionData CurrentSession;

	UPROPERTY()
	TArray<FVector> DeathHeatmapData;

	UPROPERTY()
	bool bSessionActive;

	FString GenerateSessionId() const;
	void UpdateSessionStats(const FMcpAnalyticsEvent& Event);
};
