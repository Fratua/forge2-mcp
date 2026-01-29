// Copyright 2025 FORGE2 Project. All Rights Reserved.

#include "Forge2LevelDataSystem.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/PlayerStart.h"

// ============================================================================
// LEVEL MANAGER IMPLEMENTATION
// ============================================================================

UForge2LevelManager::UForge2LevelManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Update every 500ms for streaming
}

void UForge2LevelManager::BeginPlay()
{
	Super::BeginPlay();

	// Load initially loaded sublevels
	for (const auto& Pair : RegisteredSublevels)
	{
		if (Pair.Value.bInitiallyLoaded)
		{
			LoadSublevel(Pair.Key, Pair.Value.bInitiallyVisible, false);
		}
	}
}

void UForge2LevelManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateDistanceBasedStreaming();
}

bool UForge2LevelManager::RegisterSublevel(const FForge2SublevelInfo& SublevelInfo)
{
	if (SublevelInfo.LevelName.IsNone())
	{
		return false;
	}

	RegisteredSublevels.Add(SublevelInfo.LevelName, SublevelInfo);
	SublevelStates.Add(SublevelInfo.LevelName, EForge2LevelState::Unloaded);

	return true;
}

bool UForge2LevelManager::LoadSublevel(FName LevelName, bool bMakeVisible, bool bBlockOnLoad)
{
	FForge2SublevelInfo* SublevelInfo = RegisteredSublevels.Find(LevelName);
	if (!SublevelInfo || !SublevelInfo->LevelAsset.IsValid())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Update state
	SublevelStates.Add(LevelName, EForge2LevelState::Loading);
	OnLevelStateChanged.Broadcast(LevelName, EForge2LevelState::Loading);

	// Create streaming level
	bool bSuccess = false;
	ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstance(
		World,
		SublevelInfo->LevelAsset.GetLongPackageName(),
		SublevelInfo->LevelOffset,
		SublevelInfo->LevelRotation,
		bSuccess
	);

	if (StreamingLevel && bSuccess)
	{
		StreamingLevel->SetShouldBeLoaded(true);
		StreamingLevel->SetShouldBeVisible(bMakeVisible);

		if (bBlockOnLoad)
		{
			// Flush streaming
			World->FlushLevelStreaming();
		}

		// Update state on completion
		SublevelStates.Add(LevelName, bMakeVisible ? EForge2LevelState::Visible : EForge2LevelState::Loaded);
		OnLevelStateChanged.Broadcast(LevelName, SublevelStates[LevelName]);
		OnLevelLoaded.Broadcast(LevelName);

		return true;
	}

	SublevelStates.Add(LevelName, EForge2LevelState::Unloaded);
	return false;
}

bool UForge2LevelManager::UnloadSublevel(FName LevelName, bool bBlockOnUnload)
{
	EForge2LevelState* CurrentState = SublevelStates.Find(LevelName);
	if (!CurrentState || *CurrentState == EForge2LevelState::Unloaded)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Find the streaming level
	for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
	{
		if (StreamingLevel && StreamingLevel->GetWorldAssetPackageFName() == LevelName)
		{
			SublevelStates.Add(LevelName, EForge2LevelState::Unloading);
			OnLevelStateChanged.Broadcast(LevelName, EForge2LevelState::Unloading);

			StreamingLevel->SetShouldBeLoaded(false);
			StreamingLevel->SetShouldBeVisible(false);

			if (bBlockOnUnload)
			{
				World->FlushLevelStreaming();
			}

			SublevelStates.Add(LevelName, EForge2LevelState::Unloaded);
			OnLevelStateChanged.Broadcast(LevelName, EForge2LevelState::Unloaded);
			OnLevelUnloaded.Broadcast(LevelName);

			return true;
		}
	}

	return false;
}

bool UForge2LevelManager::SetSublevelVisibility(FName LevelName, bool bVisible)
{
	EForge2LevelState* CurrentState = SublevelStates.Find(LevelName);
	if (!CurrentState)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
	{
		if (StreamingLevel && StreamingLevel->GetWorldAssetPackageFName() == LevelName)
		{
			StreamingLevel->SetShouldBeVisible(bVisible);
			SublevelStates.Add(LevelName, bVisible ? EForge2LevelState::Visible : EForge2LevelState::Loaded);
			OnLevelStateChanged.Broadcast(LevelName, SublevelStates[LevelName]);
			return true;
		}
	}

	return false;
}

EForge2LevelState UForge2LevelManager::GetSublevelState(FName LevelName) const
{
	const EForge2LevelState* State = SublevelStates.Find(LevelName);
	return State ? *State : EForge2LevelState::Unloaded;
}

TArray<FName> UForge2LevelManager::GetLoadedSublevels() const
{
	TArray<FName> LoadedLevels;
	for (const auto& Pair : SublevelStates)
	{
		if (Pair.Value == EForge2LevelState::Loaded || Pair.Value == EForge2LevelState::Visible)
		{
			LoadedLevels.Add(Pair.Key);
		}
	}
	return LoadedLevels;
}

void UForge2LevelManager::ConfigureWorldPartition(const FForge2WorldPartitionSettings& Settings)
{
	WorldPartitionSettings = Settings;
	// World Partition configuration typically requires editor-time setup
	// Runtime adjustments are limited to streaming sources and loading range
}

void UForge2LevelManager::AddStreamingSource(AActor* SourceActor)
{
	if (SourceActor)
	{
		StreamingSources.AddUnique(SourceActor);
	}
}

void UForge2LevelManager::RemoveStreamingSource(AActor* SourceActor)
{
	StreamingSources.Remove(SourceActor);
}

void UForge2LevelManager::InitiateLevelTransition(const FForge2LevelTransitionSettings& TransitionSettings)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Build options string with spawn point tag
	FString Options;
	if (!TransitionSettings.SpawnPointTag.IsNone())
	{
		Options = FString::Printf(TEXT("?SpawnPoint=%s"), *TransitionSettings.SpawnPointTag.ToString());
	}

	if (TransitionSettings.bSeamlessTravel)
	{
		World->ServerTravel(TransitionSettings.TargetLevel.ToString() + Options, true);
	}
	else
	{
		UGameplayStatics::OpenLevel(World, TransitionSettings.TargetLevel, true, Options);
	}
}

void UForge2LevelManager::SetLevelBounds(const FForge2LevelBounds& Bounds)
{
	CurrentLevelBounds = Bounds;
	// Kill Z is typically set via World Settings in editor
	// Runtime modification requires accessing WorldSettings
}

void UForge2LevelManager::RegisterSpawnPoint(const FForge2SpawnPointData& SpawnPoint)
{
	if (!SpawnPoint.SpawnPointId.IsNone())
	{
		SpawnPoints.Add(SpawnPoint.SpawnPointId, SpawnPoint);
	}
}

FForge2SpawnPointData UForge2LevelManager::GetSpawnPointByTag(FGameplayTag Tag) const
{
	for (const auto& Pair : SpawnPoints)
	{
		if (Pair.Value.bEnabled && Pair.Value.SpawnTags.HasTag(Tag))
		{
			return Pair.Value;
		}
	}
	return FForge2SpawnPointData();
}

TArray<FForge2SpawnPointData> UForge2LevelManager::GetAllSpawnPoints() const
{
	TArray<FForge2SpawnPointData> AllPoints;
	for (const auto& Pair : SpawnPoints)
	{
		AllPoints.Add(Pair.Value);
	}
	return AllPoints;
}

FForge2SpawnPointData UForge2LevelManager::GetBestSpawnPoint(const FGameplayTagContainer& PreferredTags) const
{
	FForge2SpawnPointData BestSpawn;
	int32 BestScore = -1;
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	for (const auto& Pair : SpawnPoints)
	{
		const FForge2SpawnPointData& Spawn = Pair.Value;

		if (!Spawn.bEnabled)
		{
			continue;
		}

		// Check cooldown
		if (Spawn.CooldownTime > 0.0f && (CurrentTime - Spawn.LastUsedTime) < Spawn.CooldownTime)
		{
			continue;
		}

		// Calculate score based on tag matching and priority
		int32 Score = Spawn.Priority;

		for (const FGameplayTag& Tag : PreferredTags)
		{
			if (Spawn.SpawnTags.HasTag(Tag))
			{
				Score += 10;
			}
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestSpawn = Spawn;
		}
	}

	return BestSpawn;
}

void UForge2LevelManager::UpdateDistanceBasedStreaming()
{
	if (!WorldPartitionSettings.bEnableStreaming)
	{
		return;
	}

	// Get streaming source locations
	TArray<FVector> SourceLocations;
	for (const TWeakObjectPtr<AActor>& Source : StreamingSources)
	{
		if (Source.IsValid())
		{
			SourceLocations.Add(Source->GetActorLocation());
		}
	}

	// Add player locations as default sources
	UWorld* World = GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					SourceLocations.Add(Pawn->GetActorLocation());
				}
			}
		}
	}

	// Check each registered sublevel for distance-based streaming
	for (auto& Pair : RegisteredSublevels)
	{
		if (Pair.Value.StreamingMethod != EForge2LevelStreamingMethod::Distance)
		{
			continue;
		}

		FVector LevelCenter = Pair.Value.LevelOffset;
		bool bShouldBeLoaded = false;

		for (const FVector& SourceLoc : SourceLocations)
		{
			float Distance = FVector::Dist(SourceLoc, LevelCenter);
			if (Distance < Pair.Value.StreamingDistance)
			{
				bShouldBeLoaded = true;
				break;
			}
		}

		EForge2LevelState CurrentState = GetSublevelState(Pair.Key);

		if (bShouldBeLoaded && CurrentState == EForge2LevelState::Unloaded)
		{
			LoadSublevel(Pair.Key, true, false);
		}
		else if (!bShouldBeLoaded && (CurrentState == EForge2LevelState::Loaded || CurrentState == EForge2LevelState::Visible))
		{
			UnloadSublevel(Pair.Key, false);
		}
	}
}

void UForge2LevelManager::HandleLevelStreamingComplete(ULevelStreaming* LevelStreaming)
{
	// Callback for async level streaming completion
	if (LevelStreaming)
	{
		FName LevelName = LevelStreaming->GetWorldAssetPackageFName();
		if (LevelStreaming->IsLevelLoaded())
		{
			SublevelStates.Add(LevelName, LevelStreaming->IsLevelVisible() ? EForge2LevelState::Visible : EForge2LevelState::Loaded);
			OnLevelLoaded.Broadcast(LevelName);
		}
		else
		{
			SublevelStates.Add(LevelName, EForge2LevelState::Unloaded);
			OnLevelUnloaded.Broadcast(LevelName);
		}
		OnLevelStateChanged.Broadcast(LevelName, SublevelStates[LevelName]);
	}
}

// ============================================================================
// TRIGGER MANAGER IMPLEMENTATION
// ============================================================================

UForge2TriggerManager::UForge2TriggerManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UForge2TriggerManager::BeginPlay()
{
	Super::BeginPlay();
}

bool UForge2TriggerManager::RegisterTrigger(const FForge2TriggerVolumeSettings& TriggerSettings)
{
	if (TriggerSettings.TriggerName.IsNone())
	{
		return false;
	}

	RegisteredTriggers.Add(TriggerSettings.TriggerName, TriggerSettings);
	TriggerCooldowns.Add(TriggerSettings.TriggerName, 0.0f);

	return true;
}

bool UForge2TriggerManager::UnregisterTrigger(FName TriggerName)
{
	if (!RegisteredTriggers.Contains(TriggerName))
	{
		return false;
	}

	RegisteredTriggers.Remove(TriggerName);
	TriggerCooldowns.Remove(TriggerName);
	TriggeredOnce.Remove(TriggerName);

	return true;
}

bool UForge2TriggerManager::SetTriggerEnabled(FName TriggerName, bool bEnabled)
{
	FForge2TriggerVolumeSettings* Settings = RegisteredTriggers.Find(TriggerName);
	if (!Settings)
	{
		return false;
	}

	Settings->bEnabled = bEnabled;
	return true;
}

bool UForge2TriggerManager::ResetTrigger(FName TriggerName)
{
	if (!RegisteredTriggers.Contains(TriggerName))
	{
		return false;
	}

	TriggerCooldowns.Add(TriggerName, 0.0f);
	TriggeredOnce.Remove(TriggerName);

	return true;
}

bool UForge2TriggerManager::IsTriggerActive(FName TriggerName) const
{
	const FForge2TriggerVolumeSettings* Settings = RegisteredTriggers.Find(TriggerName);
	if (!Settings)
	{
		return false;
	}

	if (!Settings->bEnabled)
	{
		return false;
	}

	if (Settings->bOnlyTriggerOnce && TriggeredOnce.Contains(TriggerName))
	{
		return false;
	}

	const float* Cooldown = TriggerCooldowns.Find(TriggerName);
	if (Cooldown && *Cooldown > 0.0f)
	{
		return false;
	}

	return true;
}

void UForge2TriggerManager::DispatchTriggerEvent(const FForge2TriggerEvent& Event)
{
	FForge2TriggerVolumeSettings* Settings = RegisteredTriggers.Find(Event.TriggerName);
	if (!Settings || !IsTriggerActive(Event.TriggerName))
	{
		return;
	}

	// Check if event type is enabled
	if (!Settings->EnabledEvents.Contains(Event.EventType))
	{
		return;
	}

	// Check conditions
	if (!CheckTriggerConditions(*Settings, Event.InstigatorActor))
	{
		return;
	}

	// Handle once-only triggers
	if (Settings->bOnlyTriggerOnce)
	{
		TriggeredOnce.Add(Event.TriggerName);
	}

	// Set cooldown
	if (Settings->Cooldown > 0.0f)
	{
		TriggerCooldowns.Add(Event.TriggerName, Settings->Cooldown);
	}

	// Broadcast event
	OnTriggerEvent.Broadcast(Event);
}

void UForge2TriggerManager::DispatchGameplayEvent(const FForge2GameplayEventData& EventData)
{
	OnGameplayEvent.Broadcast(EventData);

	// Execute associated gameplay cue if registered
	if (EventData.EventTag.IsValid())
	{
		FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(
			FName(*FString::Printf(TEXT("GameplayCue.%s"), *EventData.EventTag.ToString())), false);

		if (CueTag.IsValid())
		{
			ExecuteGameplayCue(CueTag, EventData.Target, EventData.EventLocation);
		}
	}
}

void UForge2TriggerManager::RegisterGameplayCue(const FForge2GameplayCueSettings& CueSettings)
{
	if (CueSettings.CueTag.IsValid())
	{
		GameplayCues.Add(CueSettings.CueTag, CueSettings);
	}
}

void UForge2TriggerManager::ExecuteGameplayCue(FGameplayTag CueTag, AActor* Target, FVector Location)
{
	const FForge2GameplayCueSettings* CueSettings = GameplayCues.Find(CueTag);
	if (!CueSettings)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector SpawnLocation = Location + CueSettings->LocationOffset;
	FRotator SpawnRotation = CueSettings->RotationOffset;

	if (Target && CueSettings->bAttachToTarget)
	{
		SpawnLocation = Target->GetActorLocation() + CueSettings->LocationOffset;
	}

	// Spawn particle effect
	if (CueSettings->ParticleEffect.IsValid())
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			World,
			CueSettings->ParticleEffect.LoadSynchronous(),
			SpawnLocation,
			SpawnRotation,
			FVector(1.0f),
			true
		);
	}

	// Play sound
	if (CueSettings->SoundEffect.IsValid())
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			CueSettings->SoundEffect.LoadSynchronous(),
			SpawnLocation
		);
	}

	// Camera shake
	if (CueSettings->CameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			World,
			CueSettings->CameraShake,
			SpawnLocation,
			500.0f,
			1000.0f
		);
	}
}

bool UForge2TriggerManager::ActorHasGameplayTag(AActor* Actor, FGameplayTag Tag) const
{
	if (!Actor)
	{
		return false;
	}

	const FGameplayTagContainer* Tags = ActorTags.Find(Actor);
	return Tags && Tags->HasTag(Tag);
}

void UForge2TriggerManager::AddGameplayTagToActor(AActor* Actor, FGameplayTag Tag)
{
	if (!Actor || !Tag.IsValid())
	{
		return;
	}

	FGameplayTagContainer& Tags = ActorTags.FindOrAdd(Actor);
	Tags.AddTag(Tag);
}

void UForge2TriggerManager::RemoveGameplayTagFromActor(AActor* Actor, FGameplayTag Tag)
{
	if (!Actor)
	{
		return;
	}

	FGameplayTagContainer* Tags = ActorTags.Find(Actor);
	if (Tags)
	{
		Tags->RemoveTag(Tag);
	}
}

bool UForge2TriggerManager::CheckTriggerConditions(const FForge2TriggerVolumeSettings& Settings, AActor* Instigator) const
{
	if (!Instigator)
	{
		return Settings.RequiredTags.IsEmpty();
	}

	const FGameplayTagContainer* InstigatorTags = ActorTags.Find(Instigator);
	FGameplayTagContainer EmptyTags;
	const FGameplayTagContainer& Tags = InstigatorTags ? *InstigatorTags : EmptyTags;

	// Check required tags
	if (!Settings.RequiredTags.IsEmpty() && !Tags.HasAll(Settings.RequiredTags))
	{
		return false;
	}

	// Check blocked tags
	if (!Settings.BlockedTags.IsEmpty() && Tags.HasAny(Settings.BlockedTags))
	{
		return false;
	}

	return true;
}

void UForge2TriggerManager::ProcessCooldowns(float DeltaTime)
{
	for (auto& Pair : TriggerCooldowns)
	{
		if (Pair.Value > 0.0f)
		{
			Pair.Value = FMath::Max(0.0f, Pair.Value - DeltaTime);
		}
	}
}

// ============================================================================
// DATA TABLE MANAGER IMPLEMENTATION
// ============================================================================

UForge2DataTableManager::UForge2DataTableManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2DataTableManager::BeginPlay()
{
	Super::BeginPlay();
}

UDataTable* UForge2DataTableManager::CreateDataTable(const FForge2DataTableSettings& Settings)
{
	if (Settings.TableName.IsNone() || !Settings.RowStruct)
	{
		return nullptr;
	}

	UDataTable* NewTable = NewObject<UDataTable>(GetTransientPackage(), Settings.TableName);
	if (NewTable)
	{
		NewTable->RowStruct = Settings.RowStruct;
		ManagedTables.Add(Settings.TableName, NewTable);

		// Import from CSV if path provided
		if (!Settings.CSVImportPath.IsEmpty())
		{
			ImportFromCSV(NewTable, Settings.CSVImportPath);
		}
	}

	return NewTable;
}

bool UForge2DataTableManager::AddDataTableRow(UDataTable* DataTable, FName RowName, const FTableRowBase& RowData)
{
	if (!DataTable || RowName.IsNone())
	{
		return false;
	}

	// DataTable row manipulation at runtime requires copying the struct data
	// This is a simplified implementation
	DataTable->AddRow(RowName, RowData);
	return true;
}

bool UForge2DataTableManager::RemoveDataTableRow(UDataTable* DataTable, FName RowName)
{
	if (!DataTable || RowName.IsNone())
	{
		return false;
	}

	DataTable->RemoveRow(RowName);
	return true;
}

FTableRowBase* UForge2DataTableManager::GetDataTableRow(UDataTable* DataTable, FName RowName) const
{
	if (!DataTable || RowName.IsNone())
	{
		return nullptr;
	}

	// Note: This returns the base pointer; caller needs to cast to actual row type
	return DataTable->FindRow<FTableRowBase>(RowName, TEXT(""));
}

bool UForge2DataTableManager::ImportFromCSV(UDataTable* DataTable, const FString& CSVPath)
{
	if (!DataTable)
	{
		return false;
	}

	FString CSVData;
	if (!FFileHelper::LoadFileToString(CSVData, *CSVPath))
	{
		return false;
	}

	TArray<FString> Errors = DataTable->CreateTableFromCSVString(CSVData);
	return Errors.Num() == 0;
}

FString UForge2DataTableManager::ExportToCSV(UDataTable* DataTable) const
{
	if (!DataTable)
	{
		return FString();
	}

	return DataTable->GetTableAsCSV();
}

UCurveTable* UForge2DataTableManager::CreateCurveTable(FName TableName)
{
	if (TableName.IsNone())
	{
		return nullptr;
	}

	UCurveTable* NewTable = NewObject<UCurveTable>(GetTransientPackage(), TableName);
	if (NewTable)
	{
		ManagedCurveTables.Add(TableName, NewTable);
	}

	return NewTable;
}

bool UForge2DataTableManager::AddCurve(UCurveTable* CurveTable, const FForge2CurveTableSettings& CurveSettings)
{
	if (!CurveTable || CurveSettings.CurveName.IsNone())
	{
		return false;
	}

	// Create a simple curve from the points
	FSimpleCurve NewCurve;
	for (const FForge2CurvePointData& Point : CurveSettings.Points)
	{
		FKeyHandle Handle = NewCurve.AddKey(Point.Time, Point.Value);

		ERichCurveInterpMode InterpMode = RCIM_Cubic;
		switch (Point.InterpMode)
		{
		case EForge2InterpolationType::Linear:
			InterpMode = RCIM_Linear;
			break;
		case EForge2InterpolationType::Constant:
			InterpMode = RCIM_Constant;
			break;
		default:
			InterpMode = RCIM_Cubic;
			break;
		}
		NewCurve.SetKeyInterpMode(Handle, InterpMode);
	}

	// Note: UCurveTable row manipulation requires more complex handling
	// This is a simplified conceptual implementation
	return true;
}

float UForge2DataTableManager::EvaluateCurve(UCurveTable* CurveTable, FName CurveName, float Time) const
{
	if (!CurveTable || CurveName.IsNone())
	{
		return 0.0f;
	}

	static const FString Context(TEXT("UForge2DataTableManager::EvaluateCurve"));
	FRealCurve* Curve = CurveTable->FindCurve(CurveName, Context, false);

	if (Curve)
	{
		return Curve->Eval(Time);
	}

	return 0.0f;
}

UCurveFloat* UForge2DataTableManager::CreateFloatCurve(const TArray<FForge2CurvePointData>& Points)
{
	UCurveFloat* NewCurve = NewObject<UCurveFloat>(GetTransientPackage());
	if (!NewCurve)
	{
		return nullptr;
	}

	for (const FForge2CurvePointData& Point : Points)
	{
		FKeyHandle Handle = NewCurve->FloatCurve.AddKey(Point.Time, Point.Value);
		NewCurve->FloatCurve.SetKeyTangentMode(Handle, RCTM_Auto);
	}

	return NewCurve;
}

UCurveVector* UForge2DataTableManager::CreateVectorCurve(
	const TArray<FForge2CurvePointData>& XPoints,
	const TArray<FForge2CurvePointData>& YPoints,
	const TArray<FForge2CurvePointData>& ZPoints)
{
	UCurveVector* NewCurve = NewObject<UCurveVector>(GetTransientPackage());
	if (!NewCurve)
	{
		return nullptr;
	}

	for (const FForge2CurvePointData& Point : XPoints)
	{
		NewCurve->FloatCurves[0].AddKey(Point.Time, Point.Value);
	}

	for (const FForge2CurvePointData& Point : YPoints)
	{
		NewCurve->FloatCurves[1].AddKey(Point.Time, Point.Value);
	}

	for (const FForge2CurvePointData& Point : ZPoints)
	{
		NewCurve->FloatCurves[2].AddKey(Point.Time, Point.Value);
	}

	return NewCurve;
}

void UForge2DataTableManager::RegisterCompositeTable(FName CompositeName, const TArray<UDataTable*>& Tables)
{
	if (CompositeName.IsNone())
	{
		return;
	}

	TArray<TWeakObjectPtr<UDataTable>> WeakTables;
	for (UDataTable* Table : Tables)
	{
		if (Table)
		{
			WeakTables.Add(Table);
		}
	}

	CompositeTables.Add(CompositeName, WeakTables);
}

FTableRowBase* UForge2DataTableManager::GetCompositeTableRow(FName CompositeName, FName RowName) const
{
	const TArray<TWeakObjectPtr<UDataTable>>* Tables = CompositeTables.Find(CompositeName);
	if (!Tables)
	{
		return nullptr;
	}

	// Search through all tables in the composite
	for (const TWeakObjectPtr<UDataTable>& WeakTable : *Tables)
	{
		if (UDataTable* Table = WeakTable.Get())
		{
			FTableRowBase* Row = Table->FindRow<FTableRowBase>(RowName, TEXT(""));
			if (Row)
			{
				return Row;
			}
		}
	}

	return nullptr;
}

// ============================================================================
// ASSET MANAGER COMPONENT IMPLEMENTATION
// ============================================================================

UForge2AssetManagerComponent::UForge2AssetManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2AssetManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	ProcessPreloadQueue();
}

void UForge2AssetManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cancel all pending load requests
	for (auto& Pair : ActiveLoadRequests)
	{
		CancelLoadRequest(Pair.Key);
	}

	Super::EndPlay(EndPlayReason);
}

void UForge2AssetManagerComponent::RegisterAsset(const FForge2AssetReference& AssetRef)
{
	if (!AssetRef.AssetId.IsNone())
	{
		RegisteredAssets.Add(AssetRef.AssetId, AssetRef);
	}
}

void UForge2AssetManagerComponent::UnregisterAsset(FName AssetId)
{
	RegisteredAssets.Remove(AssetId);
	UnloadAsset(AssetId);
}

FGuid UForge2AssetManagerComponent::RequestAsyncLoad(const TArray<FForge2AssetReference>& Assets)
{
	FForge2AssetLoadRequest Request;
	Request.Assets = Assets;

	ActiveLoadRequests.Add(Request.RequestId, Request);
	LoadProgress.Add(Request.RequestId, 0.0f);

	// Collect all paths to load
	TArray<FSoftObjectPath> PathsToLoad;
	for (const FForge2AssetReference& Asset : Assets)
	{
		PathsToLoad.Add(Asset.AssetPath);
	}

	// Start async loading
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

	StreamableManager.RequestAsyncLoad(
		PathsToLoad,
		FStreamableDelegate::CreateLambda([this, RequestId = Request.RequestId, PathsToLoad]()
		{
			TArray<UObject*> LoadedObjects;
			for (const FSoftObjectPath& Path : PathsToLoad)
			{
				if (UObject* Obj = Path.ResolveObject())
				{
					LoadedObjects.Add(Obj);
				}
			}
			HandleAsyncLoadComplete(RequestId, LoadedObjects);
		})
	);

	return Request.RequestId;
}

void UForge2AssetManagerComponent::CancelLoadRequest(FGuid RequestId)
{
	ActiveLoadRequests.Remove(RequestId);
	LoadProgress.Remove(RequestId);
}

bool UForge2AssetManagerComponent::IsLoadComplete(FGuid RequestId) const
{
	return CompletedRequests.Contains(RequestId);
}

float UForge2AssetManagerComponent::GetLoadProgress(FGuid RequestId) const
{
	const float* Progress = LoadProgress.Find(RequestId);
	return Progress ? *Progress : 0.0f;
}

UObject* UForge2AssetManagerComponent::LoadAssetSync(const FForge2AssetReference& AssetRef)
{
	if (!AssetRef.AssetPath.IsValid())
	{
		return nullptr;
	}

	UObject* LoadedObject = AssetRef.AssetPath.TryLoad();
	if (LoadedObject && !AssetRef.AssetId.IsNone())
	{
		LoadedAssets.Add(AssetRef.AssetId, LoadedObject);
	}

	return LoadedObject;
}

TArray<UObject*> UForge2AssetManagerComponent::LoadAssetsSync(const TArray<FForge2AssetReference>& Assets)
{
	TArray<UObject*> LoadedObjects;
	for (const FForge2AssetReference& Asset : Assets)
	{
		if (UObject* Obj = LoadAssetSync(Asset))
		{
			LoadedObjects.Add(Obj);
		}
	}
	return LoadedObjects;
}

void UForge2AssetManagerComponent::UnloadAsset(FName AssetId)
{
	LoadedAssets.Remove(AssetId);
	// Note: Actual asset unloading happens when no references remain
}

void UForge2AssetManagerComponent::UnloadAssetsByBundle(EForge2AssetBundleType BundleType)
{
	TArray<FName> AssetsToUnload;
	for (const auto& Pair : RegisteredAssets)
	{
		if (Pair.Value.BundleType == BundleType)
		{
			AssetsToUnload.Add(Pair.Key);
		}
	}

	for (FName AssetId : AssetsToUnload)
	{
		UnloadAsset(AssetId);
	}
}

void UForge2AssetManagerComponent::UnloadUnusedAssets()
{
	TArray<FName> AssetsToRemove;
	for (auto& Pair : LoadedAssets)
	{
		if (!Pair.Value.IsValid())
		{
			AssetsToRemove.Add(Pair.Key);
		}
	}

	for (FName AssetId : AssetsToRemove)
	{
		LoadedAssets.Remove(AssetId);
	}

	// Request garbage collection
	GEngine->ForceGarbageCollection(true);
}

bool UForge2AssetManagerComponent::IsAssetLoaded(FName AssetId) const
{
	const TWeakObjectPtr<UObject>* Asset = LoadedAssets.Find(AssetId);
	return Asset && Asset->IsValid();
}

UObject* UForge2AssetManagerComponent::GetLoadedAsset(FName AssetId) const
{
	const TWeakObjectPtr<UObject>* Asset = LoadedAssets.Find(AssetId);
	return Asset ? Asset->Get() : nullptr;
}

TArray<FForge2AssetReference> UForge2AssetManagerComponent::QueryAssets(const FForge2AssetRegistryQuery& Query) const
{
	TArray<FForge2AssetReference> Results;
	int32 ResultCount = 0;

	for (const auto& Pair : RegisteredAssets)
	{
		if (ResultCount >= Query.MaxResults)
		{
			break;
		}

		const FForge2AssetReference& Asset = Pair.Value;

		// Filter by tags
		if (!Query.RequiredTags.IsEmpty() && !Asset.AssetTags.HasAll(Query.RequiredTags))
		{
			continue;
		}

		if (!Query.ExcludedTags.IsEmpty() && Asset.AssetTags.HasAny(Query.ExcludedTags))
		{
			continue;
		}

		// Filter by name
		if (!Query.NameFilter.IsEmpty())
		{
			FString AssetName = Asset.AssetPath.GetAssetName();
			if (!AssetName.Contains(Query.NameFilter))
			{
				continue;
			}
		}

		// Filter by path
		if (!Query.PathFilter.IsEmpty())
		{
			FString AssetPath = Asset.AssetPath.ToString();
			if (!AssetPath.Contains(Query.PathFilter))
			{
				continue;
			}
		}

		Results.Add(Asset);
		ResultCount++;
	}

	return Results;
}

UPrimaryDataAsset* UForge2AssetManagerComponent::CreateDataAsset(const FForge2DataAssetDefinition& Definition)
{
	if (Definition.AssetId.IsNone() || !Definition.AssetClass)
	{
		return nullptr;
	}

	UPrimaryDataAsset* NewAsset = NewObject<UPrimaryDataAsset>(GetTransientPackage(), Definition.AssetClass, Definition.AssetId);
	return NewAsset;
}

TArray<UPrimaryDataAsset*> UForge2AssetManagerComponent::GetDataAssetsByType(FPrimaryAssetType AssetType) const
{
	TArray<UPrimaryDataAsset*> Results;

	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(AssetType, AssetIds);

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		if (UObject* Asset = AssetManager.GetPrimaryAssetObject(AssetId))
		{
			if (UPrimaryDataAsset* DataAsset = Cast<UPrimaryDataAsset>(Asset))
			{
				Results.Add(DataAsset);
			}
		}
	}

	return Results;
}

void UForge2AssetManagerComponent::HandleAsyncLoadComplete(FGuid RequestId, const TArray<UObject*>& LoadedObjects)
{
	FForge2AssetLoadRequest* Request = ActiveLoadRequests.Find(RequestId);
	if (!Request)
	{
		return;
	}

	// Store loaded assets
	for (int32 i = 0; i < LoadedObjects.Num() && i < Request->Assets.Num(); i++)
	{
		if (LoadedObjects[i] && !Request->Assets[i].AssetId.IsNone())
		{
			LoadedAssets.Add(Request->Assets[i].AssetId, LoadedObjects[i]);
		}
	}

	LoadProgress.Add(RequestId, 1.0f);
	CompletedRequests.Add(RequestId);
	ActiveLoadRequests.Remove(RequestId);

	bool bSuccess = LoadedObjects.Num() == Request->Assets.Num();
	OnAssetLoaded.Broadcast(RequestId, bSuccess);
}

void UForge2AssetManagerComponent::ProcessPreloadQueue()
{
	TArray<FForge2AssetReference> PreloadAssets;
	for (const auto& Pair : RegisteredAssets)
	{
		if (Pair.Value.bPreload)
		{
			PreloadAssets.Add(Pair.Value);
		}
	}

	if (PreloadAssets.Num() > 0)
	{
		RequestAsyncLoad(PreloadAssets);
	}
}

// ============================================================================
// UTILITIES IMPLEMENTATION
// ============================================================================

FForge2SublevelInfo UForge2LevelDataUtilities::GetStreamingSublevelPreset(FName LevelName, TSoftObjectPtr<UWorld> LevelAsset, float StreamDistance)
{
	FForge2SublevelInfo Info;
	Info.LevelName = LevelName;
	Info.LevelAsset = LevelAsset;
	Info.StreamingMethod = EForge2LevelStreamingMethod::Distance;
	Info.StreamingDistance = StreamDistance;
	Info.bInitiallyLoaded = false;
	Info.bInitiallyVisible = false;
	return Info;
}

FForge2SublevelInfo UForge2LevelDataUtilities::GetAlwaysLoadedSublevelPreset(FName LevelName, TSoftObjectPtr<UWorld> LevelAsset)
{
	FForge2SublevelInfo Info;
	Info.LevelName = LevelName;
	Info.LevelAsset = LevelAsset;
	Info.StreamingMethod = EForge2LevelStreamingMethod::AlwaysLoaded;
	Info.bInitiallyLoaded = true;
	Info.bInitiallyVisible = true;
	return Info;
}

FForge2WorldPartitionSettings UForge2LevelDataUtilities::GetDefaultWorldPartitionSettings()
{
	FForge2WorldPartitionSettings Settings;
	Settings.bEnabled = true;
	Settings.GridSize = FIntVector(128, 128, 128);
	Settings.LoadingRange = 25600.0f;
	Settings.RuntimeHashGridSize = 1024;
	Settings.bEnableStreaming = true;
	return Settings;
}

FForge2WorldPartitionSettings UForge2LevelDataUtilities::GetOpenWorldPartitionSettings(float WorldSize)
{
	FForge2WorldPartitionSettings Settings;
	Settings.bEnabled = true;

	// Calculate appropriate grid size based on world size
	int32 GridDimension = FMath::CeilToInt(WorldSize / 12800.0f);
	Settings.GridSize = FIntVector(GridDimension, GridDimension, 1);

	Settings.LoadingRange = WorldSize * 0.1f; // 10% of world size
	Settings.RuntimeHashGridSize = FMath::Max(1024, FMath::CeilToInt(WorldSize / 100.0f));
	Settings.bEnableStreaming = true;
	return Settings;
}

FForge2LevelTransitionSettings UForge2LevelDataUtilities::GetSeamlessTransitionPreset(FName TargetLevel, FName SpawnTag)
{
	FForge2LevelTransitionSettings Settings;
	Settings.TargetLevel = TargetLevel;
	Settings.SpawnPointTag = SpawnTag;
	Settings.bSeamlessTravel = true;
	Settings.bShowLoadingScreen = false;
	Settings.FadeOutDuration = 0.0f;
	Settings.FadeInDuration = 0.0f;
	return Settings;
}

FForge2LevelTransitionSettings UForge2LevelDataUtilities::GetLoadingScreenTransitionPreset(FName TargetLevel, TSoftObjectPtr<UTexture2D> LoadingImage)
{
	FForge2LevelTransitionSettings Settings;
	Settings.TargetLevel = TargetLevel;
	Settings.bSeamlessTravel = false;
	Settings.bShowLoadingScreen = true;
	Settings.LoadingScreenImage = LoadingImage;
	Settings.FadeOutDuration = 0.5f;
	Settings.FadeInDuration = 0.5f;
	Settings.FadeColor = FLinearColor::Black;
	return Settings;
}

FForge2TriggerVolumeSettings UForge2LevelDataUtilities::GetBoxTriggerPreset(FName Name, FVector Location, FVector Extent)
{
	FForge2TriggerVolumeSettings Settings;
	Settings.TriggerName = Name;
	Settings.Shape = EForge2TriggerShape::Box;
	Settings.Location = Location;
	Settings.Extent = Extent;
	Settings.bEnabled = true;
	return Settings;
}

FForge2TriggerVolumeSettings UForge2LevelDataUtilities::GetSphereTriggerPreset(FName Name, FVector Location, float Radius)
{
	FForge2TriggerVolumeSettings Settings;
	Settings.TriggerName = Name;
	Settings.Shape = EForge2TriggerShape::Sphere;
	Settings.Location = Location;
	Settings.Radius = Radius;
	Settings.bEnabled = true;
	return Settings;
}

FForge2TriggerVolumeSettings UForge2LevelDataUtilities::GetOneShotTriggerPreset(FName Name, FVector Location, FVector Extent)
{
	FForge2TriggerVolumeSettings Settings = GetBoxTriggerPreset(Name, Location, Extent);
	Settings.bOnlyTriggerOnce = true;
	return Settings;
}

FForge2TriggerVolumeSettings UForge2LevelDataUtilities::GetCooldownTriggerPreset(FName Name, FVector Location, FVector Extent, float Cooldown)
{
	FForge2TriggerVolumeSettings Settings = GetBoxTriggerPreset(Name, Location, Extent);
	Settings.Cooldown = Cooldown;
	return Settings;
}

FForge2CurveTableSettings UForge2LevelDataUtilities::GetLinearCurvePreset(FName Name, float StartValue, float EndValue, float Duration)
{
	FForge2CurveTableSettings Settings;
	Settings.CurveName = Name;
	Settings.CurveType = EForge2CurveType::Float;

	FForge2CurvePointData StartPoint;
	StartPoint.Time = 0.0f;
	StartPoint.Value = StartValue;
	StartPoint.InterpMode = EForge2InterpolationType::Linear;

	FForge2CurvePointData EndPoint;
	EndPoint.Time = Duration;
	EndPoint.Value = EndValue;
	EndPoint.InterpMode = EForge2InterpolationType::Linear;

	Settings.Points.Add(StartPoint);
	Settings.Points.Add(EndPoint);

	return Settings;
}

FForge2CurveTableSettings UForge2LevelDataUtilities::GetEaseInOutCurvePreset(FName Name, float StartValue, float EndValue, float Duration)
{
	FForge2CurveTableSettings Settings;
	Settings.CurveName = Name;
	Settings.CurveType = EForge2CurveType::Float;

	FForge2CurvePointData StartPoint;
	StartPoint.Time = 0.0f;
	StartPoint.Value = StartValue;
	StartPoint.InterpMode = EForge2InterpolationType::Cubic;
	StartPoint.LeaveTangent = 0.0f;

	FForge2CurvePointData EndPoint;
	EndPoint.Time = Duration;
	EndPoint.Value = EndValue;
	EndPoint.InterpMode = EForge2InterpolationType::Cubic;
	EndPoint.ArriveTangent = 0.0f;

	Settings.Points.Add(StartPoint);
	Settings.Points.Add(EndPoint);

	return Settings;
}

FForge2CurveTableSettings UForge2LevelDataUtilities::GetBounceCurvePreset(FName Name, float Amplitude, int32 Bounces, float Duration)
{
	FForge2CurveTableSettings Settings;
	Settings.CurveName = Name;
	Settings.CurveType = EForge2CurveType::Float;

	float TimePerBounce = Duration / static_cast<float>(Bounces);
	float CurrentAmplitude = Amplitude;

	FForge2CurvePointData StartPoint;
	StartPoint.Time = 0.0f;
	StartPoint.Value = 0.0f;
	Settings.Points.Add(StartPoint);

	for (int32 i = 0; i < Bounces; i++)
	{
		float PeakTime = (i + 0.5f) * TimePerBounce;
		float EndTime = (i + 1) * TimePerBounce;

		FForge2CurvePointData PeakPoint;
		PeakPoint.Time = PeakTime;
		PeakPoint.Value = CurrentAmplitude;
		PeakPoint.InterpMode = EForge2InterpolationType::Cubic;
		Settings.Points.Add(PeakPoint);

		FForge2CurvePointData LandPoint;
		LandPoint.Time = EndTime;
		LandPoint.Value = 0.0f;
		LandPoint.InterpMode = EForge2InterpolationType::Cubic;
		Settings.Points.Add(LandPoint);

		CurrentAmplitude *= 0.6f; // Decay per bounce
	}

	return Settings;
}

FForge2AssetReference UForge2LevelDataUtilities::GetPreloadAssetReference(FName AssetId, FSoftObjectPath Path)
{
	FForge2AssetReference Ref;
	Ref.AssetId = AssetId;
	Ref.AssetPath = Path;
	Ref.LoadPriority = EForge2AssetLoadPriority::High;
	Ref.bPreload = true;
	Ref.bKeepLoaded = true;
	return Ref;
}

FForge2AssetReference UForge2LevelDataUtilities::GetStreamingAssetReference(FName AssetId, FSoftObjectPath Path)
{
	FForge2AssetReference Ref;
	Ref.AssetId = AssetId;
	Ref.AssetPath = Path;
	Ref.LoadPriority = EForge2AssetLoadPriority::Normal;
	Ref.BundleType = EForge2AssetBundleType::Streaming;
	Ref.bPreload = false;
	Ref.bKeepLoaded = false;
	return Ref;
}

FForge2AssetRegistryQuery UForge2LevelDataUtilities::GetTagBasedQuery(FPrimaryAssetType Type, const FGameplayTagContainer& Tags)
{
	FForge2AssetRegistryQuery Query;
	Query.AssetType = Type;
	Query.RequiredTags = Tags;
	Query.MaxResults = 100;
	return Query;
}

FForge2SpawnPointData UForge2LevelDataUtilities::GetPlayerSpawnPreset(FName Id, FVector Location, FRotator Rotation)
{
	FForge2SpawnPointData Spawn;
	Spawn.SpawnPointId = Id;
	Spawn.Location = Location;
	Spawn.Rotation = Rotation;
	Spawn.bPlayerSpawn = true;
	Spawn.bEnabled = true;
	Spawn.Priority = 100;
	return Spawn;
}

FForge2SpawnPointData UForge2LevelDataUtilities::GetEnemySpawnPreset(FName Id, FVector Location, float Cooldown)
{
	FForge2SpawnPointData Spawn;
	Spawn.SpawnPointId = Id;
	Spawn.Location = Location;
	Spawn.bPlayerSpawn = false;
	Spawn.bEnabled = true;
	Spawn.Priority = 50;
	Spawn.CooldownTime = Cooldown;
	return Spawn;
}

FBox UForge2LevelDataUtilities::CalculateLevelBounds(const TArray<AActor*>& Actors)
{
	if (Actors.Num() == 0)
	{
		return FBox(FVector::ZeroVector, FVector::ZeroVector);
	}

	FBox Bounds(ForceInit);
	for (AActor* Actor : Actors)
	{
		if (Actor)
		{
			FVector Origin, Extent;
			Actor->GetActorBounds(false, Origin, Extent);
			Bounds += FBox(Origin - Extent, Origin + Extent);
		}
	}

	return Bounds;
}

bool UForge2LevelDataUtilities::IsPointInTrigger(const FForge2TriggerVolumeSettings& Trigger, FVector Point)
{
	FVector LocalPoint = Point - Trigger.Location;

	switch (Trigger.Shape)
	{
	case EForge2TriggerShape::Box:
		return FMath::Abs(LocalPoint.X) <= Trigger.Extent.X &&
			   FMath::Abs(LocalPoint.Y) <= Trigger.Extent.Y &&
			   FMath::Abs(LocalPoint.Z) <= Trigger.Extent.Z;

	case EForge2TriggerShape::Sphere:
		return LocalPoint.Size() <= Trigger.Radius;

	case EForge2TriggerShape::Capsule:
		{
			float HorizontalDist = FVector2D(LocalPoint.X, LocalPoint.Y).Size();
			float VerticalDist = FMath::Abs(LocalPoint.Z);

			if (VerticalDist <= Trigger.HalfHeight - Trigger.Radius)
			{
				return HorizontalDist <= Trigger.Radius;
			}
			else if (VerticalDist <= Trigger.HalfHeight)
			{
				float SphereCenter = Trigger.HalfHeight - Trigger.Radius;
				FVector SpherePoint = FVector(LocalPoint.X, LocalPoint.Y, FMath::Sign(LocalPoint.Z) * SphereCenter);
				return (LocalPoint - SpherePoint).Size() <= Trigger.Radius;
			}
			return false;
		}

	default:
		return false;
	}
}

float UForge2LevelDataUtilities::EvaluateCurvePoints(const TArray<FForge2CurvePointData>& Points, float Time)
{
	if (Points.Num() == 0)
	{
		return 0.0f;
	}

	if (Points.Num() == 1)
	{
		return Points[0].Value;
	}

	// Clamp to curve range
	if (Time <= Points[0].Time)
	{
		return Points[0].Value;
	}

	if (Time >= Points.Last().Time)
	{
		return Points.Last().Value;
	}

	// Find segment
	for (int32 i = 0; i < Points.Num() - 1; i++)
	{
		if (Time >= Points[i].Time && Time <= Points[i + 1].Time)
		{
			float T = (Time - Points[i].Time) / (Points[i + 1].Time - Points[i].Time);

			switch (Points[i].InterpMode)
			{
			case EForge2InterpolationType::Constant:
				return Points[i].Value;

			case EForge2InterpolationType::Linear:
				return FMath::Lerp(Points[i].Value, Points[i + 1].Value, T);

			case EForge2InterpolationType::Cubic:
			case EForge2InterpolationType::CubicClamped:
				{
					// Simple smoothstep
					float SmoothedT = T * T * (3.0f - 2.0f * T);
					return FMath::Lerp(Points[i].Value, Points[i + 1].Value, SmoothedT);
				}

			default:
				return FMath::Lerp(Points[i].Value, Points[i + 1].Value, T);
			}
		}
	}

	return Points.Last().Value;
}
