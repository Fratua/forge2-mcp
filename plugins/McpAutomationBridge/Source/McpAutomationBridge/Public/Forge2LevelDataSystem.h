// Copyright 2025 FORGE2 Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "Engine/LevelStreaming.h"
#include "GameplayTagContainer.h"
#include "Forge2LevelDataSystem.generated.h"

// ============================================================================
// LEVEL MANAGEMENT ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2LevelStreamingMethod : uint8
{
	Blueprint UMETA(DisplayName = "Blueprint Controlled"),
	Distance UMETA(DisplayName = "Distance Based"),
	AlwaysLoaded UMETA(DisplayName = "Always Loaded"),
	Manual UMETA(DisplayName = "Manual Control")
};

UENUM(BlueprintType)
enum class EForge2LevelState : uint8
{
	Unloaded,
	Loading,
	Loaded,
	Visible,
	Unloading
};

UENUM(BlueprintType)
enum class EForge2WorldPartitionState : uint8
{
	Disabled,
	Enabled,
	Streaming
};

// ============================================================================
// TRIGGER SYSTEM ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2TriggerShape : uint8
{
	Box,
	Sphere,
	Capsule,
	Custom
};

UENUM(BlueprintType)
enum class EForge2TriggerEventType : uint8
{
	OnEnter,
	OnExit,
	OnStay,
	OnOverlapBegin,
	OnOverlapEnd,
	OnHit,
	OnInteract,
	Custom
};

UENUM(BlueprintType)
enum class EForge2EventDispatchType : uint8
{
	Local,
	Multicast,
	Server,
	Client
};

// ============================================================================
// DATA TABLE ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2CurveType : uint8
{
	Float,
	Vector,
	Color,
	LinearColor
};

UENUM(BlueprintType)
enum class EForge2InterpolationType : uint8
{
	Linear,
	Cubic,
	Constant,
	CubicClamped
};

// ============================================================================
// ASSET MANAGEMENT ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2AssetLoadPriority : uint8
{
	Low,
	Normal,
	High,
	Critical
};

UENUM(BlueprintType)
enum class EForge2AssetBundleType : uint8
{
	None,
	Gameplay,
	UI,
	Audio,
	Cinematic,
	Streaming
};

// ============================================================================
// LEVEL MANAGEMENT STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SublevelInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	FName LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	TSoftObjectPtr<UWorld> LevelAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	EForge2LevelStreamingMethod StreamingMethod = EForge2LevelStreamingMethod::Distance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	float StreamingDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	FVector LevelOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	FRotator LevelRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	bool bShouldBlockOnLoad = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	bool bInitiallyLoaded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	bool bInitiallyVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	int32 LODIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sublevel")
	FGameplayTagContainer LevelTags;

	FForge2SublevelInfo()
		: LevelName(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WorldPartitionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
	FIntVector GridSize = FIntVector(128, 128, 128);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
	float LoadingRange = 25600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
	int32 RuntimeHashGridSize = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
	bool bEnableStreaming = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
	TArray<FName> StreamingSources;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LevelTransitionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FName TargetLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FName SpawnPointTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bSeamlessTravel = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bShowLoadingScreen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float FadeOutDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float FadeInDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FLinearColor FadeColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	TSoftObjectPtr<UTexture2D> LoadingScreenImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FText LoadingText;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LevelBounds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
	FBox WorldBounds = FBox(FVector(-100000.0f), FVector(100000.0f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
	float KillZ = -10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
	bool bEnableWorldBoundsChecks = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
	bool bKillOnBoundsExit = true;
};

// ============================================================================
// TRIGGER SYSTEM STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TriggerVolumeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FName TriggerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	EForge2TriggerShape Shape = EForge2TriggerShape::Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FVector Extent = FVector(100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	float Radius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	float HalfHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	TArray<EForge2TriggerEventType> EnabledEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	bool bOnlyTriggerOnce = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	bool bEnabled = true;

	FForge2TriggerVolumeSettings()
		: TriggerName(NAME_None)
	{
		EnabledEvents.Add(EForge2TriggerEventType::OnEnter);
		EnabledEvents.Add(EForge2TriggerEventType::OnExit);
	}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TriggerEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName TriggerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EForge2TriggerEventType EventType = EForge2TriggerEventType::OnEnter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	AActor* InstigatorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FVector EventLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FGameplayTagContainer EventTags;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GameplayEventData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	AActor* Instigator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	AActor* Target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FVector EventLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float Magnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FName, FString> CustomData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	UObject* OptionalObject = nullptr;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GameplayCueSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	FGameplayTag CueTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	TSoftObjectPtr<UParticleSystem> ParticleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	TSoftObjectPtr<USoundBase> SoundEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	bool bAttachToTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	FRotator RotationOffset = FRotator::ZeroRotator;
};

// ============================================================================
// DATA TABLE STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DataTableRowBase : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Row")
	FName RowId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Row")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Row")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Row")
	FGameplayTagContainer Tags;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CurvePointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	float ArriveTangent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	float LeaveTangent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	EForge2InterpolationType InterpMode = EForge2InterpolationType::Cubic;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CurveTableSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	FName CurveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	EForge2CurveType CurveType = EForge2CurveType::Float;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	TArray<FForge2CurvePointData> Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	float PreInfinityExtrap = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	float PostInfinityExtrap = 0.0f;

	FForge2CurveTableSettings()
		: CurveName(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DataTableSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	FName TableName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	UScriptStruct* RowStruct = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	FString CSVImportPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	bool bIgnoreExtraFields = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	bool bIgnoreMissingFields = false;

	FForge2DataTableSettings()
		: TableName(NAME_None)
	{}
};

// ============================================================================
// ASSET MANAGEMENT STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AssetReference
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	FName AssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	FSoftObjectPath AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	EForge2AssetLoadPriority LoadPriority = EForge2AssetLoadPriority::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	EForge2AssetBundleType BundleType = EForge2AssetBundleType::Gameplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	FGameplayTagContainer AssetTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	bool bPreload = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	bool bKeepLoaded = false;

	FForge2AssetReference()
		: AssetId(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AssetLoadRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	FGuid RequestId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	TArray<FForge2AssetReference> Assets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	bool bAsync = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	bool bBlockOnLoad = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	float TimeoutSeconds = 30.0f;

	FForge2AssetLoadRequest()
	{
		RequestId = FGuid::NewGuid();
	}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DataAssetDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataAsset")
	FName AssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataAsset")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataAsset")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataAsset")
	TSubclassOf<UPrimaryDataAsset> AssetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataAsset")
	FPrimaryAssetType AssetType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataAsset")
	FGameplayTagContainer Categories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataAsset")
	TMap<FName, FString> Metadata;

	FForge2DataAssetDefinition()
		: AssetId(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AssetRegistryQuery
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FPrimaryAssetType AssetType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FGameplayTagContainer ExcludedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FString NameFilter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FString PathFilter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	int32 MaxResults = 100;
};

// ============================================================================
// SPAWN POINT STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SpawnPointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName SpawnPointId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FGameplayTagContainer SpawnTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bPlayerSpawn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float CooldownTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float LastUsedTime = 0.0f;

	FForge2SpawnPointData()
		: SpawnPointId(NAME_None)
	{}
};

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FForge2OnLevelStateChanged, FName, LevelName, EForge2LevelState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FForge2OnLevelLoaded, FName, LevelName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FForge2OnLevelUnloaded, FName, LevelName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FForge2OnTriggerEvent, const FForge2TriggerEvent&, TriggerEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FForge2OnGameplayEvent, const FForge2GameplayEventData&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FForge2OnAssetLoaded, FGuid, RequestId, bool, bSuccess);

// ============================================================================
// LEVEL MANAGER
// ============================================================================

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2LevelManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2LevelManager();

	// Sublevel Management
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	bool RegisterSublevel(const FForge2SublevelInfo& SublevelInfo);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	bool LoadSublevel(FName LevelName, bool bMakeVisible = true, bool bBlockOnLoad = false);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	bool UnloadSublevel(FName LevelName, bool bBlockOnUnload = false);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	bool SetSublevelVisibility(FName LevelName, bool bVisible);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level")
	EForge2LevelState GetSublevelState(FName LevelName) const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level")
	TArray<FName> GetLoadedSublevels() const;

	// World Partition
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	void ConfigureWorldPartition(const FForge2WorldPartitionSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	void AddStreamingSource(AActor* SourceActor);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	void RemoveStreamingSource(AActor* SourceActor);

	// Level Transitions
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	void InitiateLevelTransition(const FForge2LevelTransitionSettings& TransitionSettings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	void SetLevelBounds(const FForge2LevelBounds& Bounds);

	// Spawn Points
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	void RegisterSpawnPoint(const FForge2SpawnPointData& SpawnPoint);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level")
	FForge2SpawnPointData GetSpawnPointByTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level")
	TArray<FForge2SpawnPointData> GetAllSpawnPoints() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Level")
	FForge2SpawnPointData GetBestSpawnPoint(const FGameplayTagContainer& PreferredTags) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Level|Events")
	FForge2OnLevelStateChanged OnLevelStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Level|Events")
	FForge2OnLevelLoaded OnLevelLoaded;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Level|Events")
	FForge2OnLevelUnloaded OnLevelUnloaded;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TMap<FName, FForge2SublevelInfo> RegisteredSublevels;

	UPROPERTY()
	TMap<FName, EForge2LevelState> SublevelStates;

	UPROPERTY()
	FForge2WorldPartitionSettings WorldPartitionSettings;

	UPROPERTY()
	FForge2LevelBounds CurrentLevelBounds;

	UPROPERTY()
	TMap<FName, FForge2SpawnPointData> SpawnPoints;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> StreamingSources;

	void UpdateDistanceBasedStreaming();
	void HandleLevelStreamingComplete(ULevelStreaming* LevelStreaming);
};

// ============================================================================
// TRIGGER MANAGER
// ============================================================================

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2TriggerManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2TriggerManager();

	// Trigger Management
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	bool RegisterTrigger(const FForge2TriggerVolumeSettings& TriggerSettings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	bool UnregisterTrigger(FName TriggerName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	bool SetTriggerEnabled(FName TriggerName, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	bool ResetTrigger(FName TriggerName);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Trigger")
	bool IsTriggerActive(FName TriggerName) const;

	// Event Dispatching
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	void DispatchTriggerEvent(const FForge2TriggerEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	void DispatchGameplayEvent(const FForge2GameplayEventData& EventData);

	// Gameplay Cues
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	void RegisterGameplayCue(const FForge2GameplayCueSettings& CueSettings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	void ExecuteGameplayCue(FGameplayTag CueTag, AActor* Target, FVector Location);

	// Gameplay Tags
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	bool ActorHasGameplayTag(AActor* Actor, FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	void AddGameplayTagToActor(AActor* Actor, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Trigger")
	void RemoveGameplayTagFromActor(AActor* Actor, FGameplayTag Tag);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Trigger|Events")
	FForge2OnTriggerEvent OnTriggerEvent;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Trigger|Events")
	FForge2OnGameplayEvent OnGameplayEvent;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TMap<FName, FForge2TriggerVolumeSettings> RegisteredTriggers;

	UPROPERTY()
	TMap<FName, float> TriggerCooldowns;

	UPROPERTY()
	TSet<FName> TriggeredOnce;

	UPROPERTY()
	TMap<FGameplayTag, FForge2GameplayCueSettings> GameplayCues;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FGameplayTagContainer> ActorTags;

	bool CheckTriggerConditions(const FForge2TriggerVolumeSettings& Settings, AActor* Instigator) const;
	void ProcessCooldowns(float DeltaTime);
};

// ============================================================================
// DATA TABLE MANAGER
// ============================================================================

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2DataTableManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2DataTableManager();

	// Data Table Operations
	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	UDataTable* CreateDataTable(const FForge2DataTableSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	bool AddDataTableRow(UDataTable* DataTable, FName RowName, const FTableRowBase& RowData);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	bool RemoveDataTableRow(UDataTable* DataTable, FName RowName);

	UFUNCTION(BlueprintPure, Category = "FORGE2|DataTable")
	FTableRowBase* GetDataTableRow(UDataTable* DataTable, FName RowName) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	bool ImportFromCSV(UDataTable* DataTable, const FString& CSVPath);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	FString ExportToCSV(UDataTable* DataTable) const;

	// Curve Tables
	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	UCurveTable* CreateCurveTable(FName TableName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	bool AddCurve(UCurveTable* CurveTable, const FForge2CurveTableSettings& CurveSettings);

	UFUNCTION(BlueprintPure, Category = "FORGE2|DataTable")
	float EvaluateCurve(UCurveTable* CurveTable, FName CurveName, float Time) const;

	// Curve Assets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	UCurveFloat* CreateFloatCurve(const TArray<FForge2CurvePointData>& Points);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	UCurveVector* CreateVectorCurve(const TArray<FForge2CurvePointData>& XPoints,
									 const TArray<FForge2CurvePointData>& YPoints,
									 const TArray<FForge2CurvePointData>& ZPoints);

	// Composite Tables
	UFUNCTION(BlueprintCallable, Category = "FORGE2|DataTable")
	void RegisterCompositeTable(FName CompositeName, const TArray<UDataTable*>& Tables);

	UFUNCTION(BlueprintPure, Category = "FORGE2|DataTable")
	FTableRowBase* GetCompositeTableRow(FName CompositeName, FName RowName) const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TMap<FName, TArray<TWeakObjectPtr<UDataTable>>> CompositeTables;

	UPROPERTY()
	TMap<FName, TWeakObjectPtr<UDataTable>> ManagedTables;

	UPROPERTY()
	TMap<FName, TWeakObjectPtr<UCurveTable>> ManagedCurveTables;
};

// ============================================================================
// ASSET MANAGER COMPONENT
// ============================================================================

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2AssetManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2AssetManagerComponent();

	// Asset Registration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	void RegisterAsset(const FForge2AssetReference& AssetRef);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	void UnregisterAsset(FName AssetId);

	// Async Loading
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	FGuid RequestAsyncLoad(const TArray<FForge2AssetReference>& Assets);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	void CancelLoadRequest(FGuid RequestId);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Asset")
	bool IsLoadComplete(FGuid RequestId) const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Asset")
	float GetLoadProgress(FGuid RequestId) const;

	// Synchronous Loading
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	UObject* LoadAssetSync(const FForge2AssetReference& AssetRef);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	TArray<UObject*> LoadAssetsSync(const TArray<FForge2AssetReference>& Assets);

	// Asset Unloading
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	void UnloadAsset(FName AssetId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	void UnloadAssetsByBundle(EForge2AssetBundleType BundleType);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	void UnloadUnusedAssets();

	// Asset Queries
	UFUNCTION(BlueprintPure, Category = "FORGE2|Asset")
	bool IsAssetLoaded(FName AssetId) const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Asset")
	UObject* GetLoadedAsset(FName AssetId) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	TArray<FForge2AssetReference> QueryAssets(const FForge2AssetRegistryQuery& Query) const;

	// Data Assets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Asset")
	UPrimaryDataAsset* CreateDataAsset(const FForge2DataAssetDefinition& Definition);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Asset")
	TArray<UPrimaryDataAsset*> GetDataAssetsByType(FPrimaryAssetType AssetType) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Asset|Events")
	FForge2OnAssetLoaded OnAssetLoaded;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TMap<FName, FForge2AssetReference> RegisteredAssets;

	UPROPERTY()
	TMap<FName, TWeakObjectPtr<UObject>> LoadedAssets;

	UPROPERTY()
	TMap<FGuid, FForge2AssetLoadRequest> ActiveLoadRequests;

	UPROPERTY()
	TMap<FGuid, float> LoadProgress;

	UPROPERTY()
	TSet<FGuid> CompletedRequests;

	void HandleAsyncLoadComplete(FGuid RequestId, const TArray<UObject*>& LoadedObjects);
	void ProcessPreloadQueue();
};

// ============================================================================
// LEVEL & DATA UTILITIES
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2LevelDataUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Level Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Level|Presets")
	static FForge2SublevelInfo GetStreamingSublevelPreset(FName LevelName, TSoftObjectPtr<UWorld> LevelAsset, float StreamDistance = 10000.0f);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level|Presets")
	static FForge2SublevelInfo GetAlwaysLoadedSublevelPreset(FName LevelName, TSoftObjectPtr<UWorld> LevelAsset);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level|Presets")
	static FForge2WorldPartitionSettings GetDefaultWorldPartitionSettings();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level|Presets")
	static FForge2WorldPartitionSettings GetOpenWorldPartitionSettings(float WorldSize = 100000.0f);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level|Presets")
	static FForge2LevelTransitionSettings GetSeamlessTransitionPreset(FName TargetLevel, FName SpawnTag);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Level|Presets")
	static FForge2LevelTransitionSettings GetLoadingScreenTransitionPreset(FName TargetLevel, TSoftObjectPtr<UTexture2D> LoadingImage);

	// Trigger Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Trigger|Presets")
	static FForge2TriggerVolumeSettings GetBoxTriggerPreset(FName Name, FVector Location, FVector Extent);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Trigger|Presets")
	static FForge2TriggerVolumeSettings GetSphereTriggerPreset(FName Name, FVector Location, float Radius);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Trigger|Presets")
	static FForge2TriggerVolumeSettings GetOneShotTriggerPreset(FName Name, FVector Location, FVector Extent);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Trigger|Presets")
	static FForge2TriggerVolumeSettings GetCooldownTriggerPreset(FName Name, FVector Location, FVector Extent, float Cooldown);

	// Data Table Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Data|Presets")
	static FForge2CurveTableSettings GetLinearCurvePreset(FName Name, float StartValue, float EndValue, float Duration);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Data|Presets")
	static FForge2CurveTableSettings GetEaseInOutCurvePreset(FName Name, float StartValue, float EndValue, float Duration);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Data|Presets")
	static FForge2CurveTableSettings GetBounceCurvePreset(FName Name, float Amplitude, int32 Bounces, float Duration);

	// Asset Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Asset|Presets")
	static FForge2AssetReference GetPreloadAssetReference(FName AssetId, FSoftObjectPath Path);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Asset|Presets")
	static FForge2AssetReference GetStreamingAssetReference(FName AssetId, FSoftObjectPath Path);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Asset|Presets")
	static FForge2AssetRegistryQuery GetTagBasedQuery(FPrimaryAssetType Type, const FGameplayTagContainer& Tags);

	// Spawn Point Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Spawn|Presets")
	static FForge2SpawnPointData GetPlayerSpawnPreset(FName Id, FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Spawn|Presets")
	static FForge2SpawnPointData GetEnemySpawnPreset(FName Id, FVector Location, float Cooldown = 30.0f);

	// Conversion Helpers
	UFUNCTION(BlueprintPure, Category = "FORGE2|Level|Utility")
	static FBox CalculateLevelBounds(const TArray<AActor*>& Actors);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Trigger|Utility")
	static bool IsPointInTrigger(const FForge2TriggerVolumeSettings& Trigger, FVector Point);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Data|Utility")
	static float EvaluateCurvePoints(const TArray<FForge2CurvePointData>& Points, float Time);
};
