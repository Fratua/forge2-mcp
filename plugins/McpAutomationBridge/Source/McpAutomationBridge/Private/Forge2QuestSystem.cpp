// Forge2QuestSystem.cpp
// FORGE2 Quest System Implementation
// Part of the FORGE2 Ultimate AAA Game Development MCP Extension

#include "Forge2QuestSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

// ============================================================================
// FForge2QuestObjective Implementation
// ============================================================================

bool FForge2QuestObjective::IsAvailable(const TArray<FForge2QuestObjective>& AllObjectives) const
{
	// Check if all prerequisite objectives are complete
	for (const FString& PrereqId : PrerequisiteObjectives)
	{
		for (const FForge2QuestObjective& Objective : AllObjectives)
		{
			if (Objective.ObjectiveId == PrereqId)
			{
				if (Objective.State != EForge2ObjectiveState::Completed)
				{
					return false;
				}
				break;
			}
		}
	}
	return true;
}

bool FForge2QuestObjective::UpdateProgress(int32 Amount)
{
	if (State == EForge2ObjectiveState::Completed || State == EForge2ObjectiveState::Failed)
	{
		return false;
	}

	CurrentCount = FMath::Min(CurrentCount + Amount, RequiredCount);

	if (CurrentCount >= RequiredCount)
	{
		State = EForge2ObjectiveState::Completed;
		return true;
	}

	return false;
}

// ============================================================================
// FForge2QuestReward Implementation
// ============================================================================

int32 FForge2QuestReward::GetScaledAmount(int32 PlayerLevel) const
{
	if (LevelScaling <= 0.0f || PlayerLevel <= 1)
	{
		return Amount;
	}

	// Simple level scaling formula
	float ScaleFactor = 1.0f + (LevelScaling * (PlayerLevel - 1) * 0.05f);
	return FMath::RoundToInt(Amount * ScaleFactor);
}

// ============================================================================
// FForge2QuestPrerequisite Implementation
// ============================================================================

bool FForge2QuestPrerequisite::IsMet(const TMap<FString, int32>& GameState) const
{
	switch (PrerequisiteType)
	{
		case EForge2PrerequisiteType::QuestComplete:
		{
			FString Key = FString::Printf(TEXT("Quest_%s_Complete"), *RequiredId);
			const int32* Value = GameState.Find(Key);
			return Value && *Value > 0;
		}

		case EForge2PrerequisiteType::QuestActive:
		{
			FString Key = FString::Printf(TEXT("Quest_%s_Active"), *RequiredId);
			const int32* Value = GameState.Find(Key);
			return Value && *Value > 0;
		}

		case EForge2PrerequisiteType::Level:
		{
			const int32* PlayerLevel = GameState.Find(TEXT("PlayerLevel"));
			return PlayerLevel && *PlayerLevel >= RequiredValue;
		}

		case EForge2PrerequisiteType::Reputation:
		{
			FString Key = FString::Printf(TEXT("Reputation_%s"), *RequiredId);
			const int32* Rep = GameState.Find(Key);
			return Rep && *Rep >= RequiredValue;
		}

		case EForge2PrerequisiteType::Item:
		{
			FString Key = FString::Printf(TEXT("Item_%s_Count"), *RequiredId);
			const int32* Count = GameState.Find(Key);
			return Count && *Count >= RequiredValue;
		}

		case EForge2PrerequisiteType::Class:
		{
			FString Key = FString::Printf(TEXT("PlayerClass_%s"), *RequiredId);
			const int32* HasClass = GameState.Find(Key);
			return HasClass && *HasClass > 0;
		}

		case EForge2PrerequisiteType::Race:
		{
			FString Key = FString::Printf(TEXT("PlayerRace_%s"), *RequiredId);
			const int32* HasRace = GameState.Find(Key);
			return HasRace && *HasRace > 0;
		}

		case EForge2PrerequisiteType::Skill:
		{
			FString Key = FString::Printf(TEXT("Skill_%s_Level"), *RequiredId);
			const int32* SkillLevel = GameState.Find(Key);
			return SkillLevel && *SkillLevel >= RequiredValue;
		}

		case EForge2PrerequisiteType::Achievement:
		{
			FString Key = FString::Printf(TEXT("Achievement_%s"), *RequiredId);
			const int32* HasAchievement = GameState.Find(Key);
			return HasAchievement && *HasAchievement > 0;
		}

		case EForge2PrerequisiteType::Time:
		{
			const int32* GameTime = GameState.Find(TEXT("GameTimeHours"));
			return GameTime && *GameTime >= RequiredValue;
		}

		case EForge2PrerequisiteType::Location:
		{
			FString Key = FString::Printf(TEXT("InLocation_%s"), *RequiredId);
			const int32* InLocation = GameState.Find(Key);
			return InLocation && *InLocation > 0;
		}

		case EForge2PrerequisiteType::Custom:
		{
			// Custom conditions should be evaluated externally
			FString Key = FString::Printf(TEXT("Custom_%s"), *CustomConditionFunction);
			const int32* Result = GameState.Find(Key);
			return Result && *Result > 0;
		}

		default:
			return true;
	}
}

// ============================================================================
// FForge2QuestBranchCondition Implementation
// ============================================================================

bool FForge2QuestBranchCondition::Evaluate(const TMap<FString, int32>& GameState) const
{
	switch (ConditionType)
	{
		case EForge2BranchConditionType::ObjectiveComplete:
		{
			FString Key = FString::Printf(TEXT("Objective_%s_Complete"), *ConditionId);
			const int32* Value = GameState.Find(Key);
			return Value && *Value > 0;
		}

		case EForge2BranchConditionType::DialogueChoice:
		{
			FString Key = FString::Printf(TEXT("DialogueChoice_%s"), *ConditionId);
			const int32* Choice = GameState.Find(Key);
			return Choice && *Choice == RequiredValue;
		}

		case EForge2BranchConditionType::ItemHeld:
		{
			FString Key = FString::Printf(TEXT("Item_%s_Count"), *ConditionId);
			const int32* Count = GameState.Find(Key);
			return Count && *Count >= RequiredValue;
		}

		case EForge2BranchConditionType::ReputationLevel:
		{
			FString Key = FString::Printf(TEXT("Reputation_%s"), *ConditionId);
			const int32* Rep = GameState.Find(Key);
			return Rep && *Rep >= RequiredValue;
		}

		case EForge2BranchConditionType::SkillCheck:
		{
			FString Key = FString::Printf(TEXT("Skill_%s_Level"), *ConditionId);
			const int32* SkillLevel = GameState.Find(Key);
			return SkillLevel && *SkillLevel >= RequiredValue;
		}

		case EForge2BranchConditionType::RandomChance:
		{
			return FMath::FRand() * 100.0f <= RandomChance;
		}

		case EForge2BranchConditionType::TimeOfDay:
		{
			const int32* Hour = GameState.Find(TEXT("GameTimeHour"));
			if (Hour)
			{
				// RequiredValue = min hour, StringValue contains max hour
				int32 MaxHour = FCString::Atoi(*StringValue);
				return *Hour >= RequiredValue && *Hour <= MaxHour;
			}
			return false;
		}

		case EForge2BranchConditionType::PlayerChoice:
		{
			FString Key = FString::Printf(TEXT("PlayerChoice_%s"), *ConditionId);
			const int32* Choice = GameState.Find(Key);
			return Choice && *Choice == RequiredValue;
		}

		case EForge2BranchConditionType::Custom:
		{
			FString Key = FString::Printf(TEXT("CustomBranch_%s"), *ConditionId);
			const int32* Result = GameState.Find(Key);
			return Result && *Result > 0;
		}

		default:
			return true;
	}
}

// ============================================================================
// FForge2QuestBranch Implementation
// ============================================================================

bool FForge2QuestBranch::CanTakeBranch(const TMap<FString, int32>& GameState) const
{
	for (const FForge2QuestBranchCondition& Condition : Conditions)
	{
		if (!Condition.Evaluate(GameState))
		{
			return false;
		}
	}
	return true;
}

// ============================================================================
// FForge2QuestChainEntry Implementation
// ============================================================================

FString FForge2QuestChainEntry::GetNextQuest(const TMap<FString, int32>& GameState) const
{
	// Sort branches by priority
	TArray<FForge2QuestBranch> SortedBranches = Branches;
	SortedBranches.Sort([](const FForge2QuestBranch& A, const FForge2QuestBranch& B)
	{
		return A.Priority > B.Priority;
	});

	// Check branches in priority order
	for (const FForge2QuestBranch& Branch : SortedBranches)
	{
		if (Branch.CanTakeBranch(GameState))
		{
			return Branch.NextQuestId;
		}
	}

	// Return default if no branches match
	return DefaultNextQuest;
}

// ============================================================================
// FForge2QuestMarker Implementation
// ============================================================================

FVector FForge2QuestMarker::GetWorldLocation() const
{
	if (TargetActor.IsValid())
	{
		return TargetActor->GetActorLocation();
	}
	return Location;
}

float FForge2QuestMarker::CalculateScale(float Distance) const
{
	if (!bScaleWithDistance || VisibilityDistance <= 0.0f)
	{
		return MaxScale;
	}

	float T = FMath::Clamp(Distance / VisibilityDistance, 0.0f, 1.0f);
	return FMath::Lerp(MaxScale, MinScale, T);
}

// ============================================================================
// FForge2QuestDefinition Implementation
// ============================================================================

float FForge2QuestDefinition::GetProgress() const
{
	if (Objectives.Num() == 0)
	{
		return 0.0f;
	}

	int32 RequiredCount = 0;
	int32 CompletedCount = 0;

	for (const FForge2QuestObjective& Objective : Objectives)
	{
		if (!Objective.bOptional)
		{
			RequiredCount++;
			if (Objective.State == EForge2ObjectiveState::Completed)
			{
				CompletedCount++;
			}
		}
	}

	if (RequiredCount == 0)
	{
		return 1.0f;
	}

	return (float)CompletedCount / (float)RequiredCount;
}

bool FForge2QuestDefinition::AreRequiredObjectivesComplete() const
{
	for (const FForge2QuestObjective& Objective : Objectives)
	{
		if (!Objective.bOptional && Objective.State != EForge2ObjectiveState::Completed)
		{
			return false;
		}
	}
	return true;
}

TArray<FForge2QuestObjective*> FForge2QuestDefinition::GetActiveObjectives()
{
	TArray<FForge2QuestObjective*> Result;
	for (FForge2QuestObjective& Objective : Objectives)
	{
		if (Objective.State == EForge2ObjectiveState::Active)
		{
			Result.Add(&Objective);
		}
	}
	return Result;
}

TArray<FForge2QuestObjective*> FForge2QuestDefinition::GetAvailableObjectives()
{
	TArray<FForge2QuestObjective*> Result;
	for (FForge2QuestObjective& Objective : Objectives)
	{
		if (Objective.State == EForge2ObjectiveState::Inactive && Objective.IsAvailable(Objectives))
		{
			// Check hidden status
			if (Objective.bHidden)
			{
				float CurrentProgress = GetProgress();
				if (CurrentProgress < Objective.RevealAtProgress)
				{
					continue;
				}
			}
			Result.Add(&Objective);
		}
	}
	return Result;
}

// ============================================================================
// FForge2QuestChainDefinition Implementation
// ============================================================================

float FForge2QuestChainDefinition::GetChainProgress(const TSet<FString>& CompletedQuests) const
{
	if (Entries.Num() == 0)
	{
		return 0.0f;
	}

	int32 CompletedCount = 0;
	for (const FForge2QuestChainEntry& Entry : Entries)
	{
		if (CompletedQuests.Contains(Entry.QuestId))
		{
			CompletedCount++;
		}
	}

	return (float)CompletedCount / (float)Entries.Num();
}

FString FForge2QuestChainDefinition::GetStartingQuest() const
{
	for (const FForge2QuestChainEntry& Entry : Entries)
	{
		if (Entry.bChainStart)
		{
			return Entry.QuestId;
		}
	}

	// If no explicit start, return first entry
	if (Entries.Num() > 0)
	{
		return Entries[0].QuestId;
	}

	return TEXT("");
}

TArray<FString> FForge2QuestChainDefinition::GetEndingQuests() const
{
	TArray<FString> Result;
	for (const FForge2QuestChainEntry& Entry : Entries)
	{
		if (Entry.bChainEnd)
		{
			Result.Add(Entry.QuestId);
		}
	}
	return Result;
}

// ============================================================================
// FForge2QuestManager Implementation
// ============================================================================

bool FForge2QuestManager::RegisterQuest(const FForge2QuestDefinition& Definition)
{
	if (Definition.QuestId.IsEmpty())
	{
		return false;
	}

	QuestDefinitions.Add(Definition.QuestId, Definition);
	return true;
}

bool FForge2QuestManager::RegisterChain(const FForge2QuestChainDefinition& Chain)
{
	if (Chain.ChainId.IsEmpty())
	{
		return false;
	}

	QuestChains.Add(Chain.ChainId, Chain);
	return true;
}

bool FForge2QuestManager::IsQuestAvailable(const FString& QuestId, const TMap<FString, int32>& GameState) const
{
	const FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	if (!Definition)
	{
		return false;
	}

	// Check if already completed (and not repeatable)
	if (CompletedQuests.Contains(QuestId))
	{
		if (!Definition->bRepeatable)
		{
			return false;
		}

		// Check cooldown for repeatable quests
		const FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);
		if (State && Definition->RepeatCooldown > 0.0f)
		{
			if (State->CompletedTime + Definition->RepeatCooldown > State->AcceptedTime)
			{
				return false;
			}
		}
	}

	// Check if currently active
	const FForge2QuestRuntimeState* CurrentState = QuestStates.Find(QuestId);
	if (CurrentState && CurrentState->State == EForge2QuestState::Active)
	{
		return false;
	}

	// Check prerequisites
	for (const FForge2QuestPrerequisite& Prereq : Definition->Prerequisites)
	{
		if (!Prereq.IsMet(GameState))
		{
			return false;
		}
	}

	return true;
}

bool FForge2QuestManager::AcceptQuest(const FString& QuestId)
{
	FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	if (!Definition)
	{
		return false;
	}

	// Create or update runtime state
	FForge2QuestRuntimeState& State = QuestStates.FindOrAdd(QuestId);
	State.QuestId = QuestId;
	State.State = EForge2QuestState::Active;
	State.AcceptedTime = FPlatformTime::Seconds();
	State.bTracked = Definition->bAutoTrack;

	// Initialize objective states
	for (FForge2QuestObjective& Objective : Definition->Objectives)
	{
		// Activate objectives with no prerequisites
		if (Objective.PrerequisiteObjectives.Num() == 0)
		{
			Objective.State = EForge2ObjectiveState::Active;
			State.ObjectiveStates.Add(Objective.ObjectiveId, EForge2ObjectiveState::Active);
		}
		else
		{
			State.ObjectiveStates.Add(Objective.ObjectiveId, EForge2ObjectiveState::Inactive);
		}
		State.ObjectiveProgress.Add(Objective.ObjectiveId, 0);

		// Set up timed objectives
		if (Objective.TimeLimit > 0.0f)
		{
			Objective.TimeRemaining = Objective.TimeLimit;
		}

		// Add markers for active objectives
		if (Objective.State == EForge2ObjectiveState::Active && Objective.MarkerType != EForge2MarkerType::None)
		{
			AddMarkerForObjective(QuestId, Objective.ObjectiveId);
		}
	}

	// Add to tracked quests if auto-track
	if (Definition->bAutoTrack && TrackedQuests.Num() < MaxTrackedQuests)
	{
		TrackQuest(QuestId);
	}

	return true;
}

bool FForge2QuestManager::AbandonQuest(const FString& QuestId)
{
	FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);
	if (!State || State->State != EForge2QuestState::Active)
	{
		return false;
	}

	State->State = EForge2QuestState::Abandoned;

	// Remove from tracked
	UntrackQuest(QuestId);

	// Remove markers
	RemoveMarkersForQuest(QuestId);

	return true;
}

bool FForge2QuestManager::CompleteQuest(const FString& QuestId)
{
	FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);

	if (!Definition || !State || State->State != EForge2QuestState::Active)
	{
		return false;
	}

	// Check if all required objectives are complete
	if (!Definition->AreRequiredObjectivesComplete())
	{
		return false;
	}

	State->State = EForge2QuestState::Completed;
	State->CompletedTime = FPlatformTime::Seconds();
	State->CompletionCount++;

	// Add to completed set
	CompletedQuests.Add(QuestId);

	// Remove from tracked
	UntrackQuest(QuestId);

	// Remove markers
	RemoveMarkersForQuest(QuestId);

	return true;
}

bool FForge2QuestManager::FailQuest(const FString& QuestId, const FString& Reason)
{
	FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);
	if (!State || State->State != EForge2QuestState::Active)
	{
		return false;
	}

	State->State = EForge2QuestState::Failed;
	State->CompletedTime = FPlatformTime::Seconds();
	State->CustomData.Add(TEXT("FailReason"), Reason);

	// Add to failed set
	FailedQuests.Add(QuestId);

	// Remove from tracked
	UntrackQuest(QuestId);

	// Remove markers
	RemoveMarkersForQuest(QuestId);

	return true;
}

bool FForge2QuestManager::UpdateObjective(const FString& QuestId, const FString& ObjectiveId, int32 Progress)
{
	FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);

	if (!Definition || !State || State->State != EForge2QuestState::Active)
	{
		return false;
	}

	// Find the objective
	for (FForge2QuestObjective& Objective : Definition->Objectives)
	{
		if (Objective.ObjectiveId == ObjectiveId)
		{
			if (Objective.State != EForge2ObjectiveState::Active)
			{
				return false;
			}

			bool WasCompleted = Objective.UpdateProgress(Progress);
			State->ObjectiveProgress.Add(ObjectiveId, Objective.CurrentCount);

			if (WasCompleted)
			{
				State->ObjectiveStates.Add(ObjectiveId, EForge2ObjectiveState::Completed);

				// Remove marker for this objective
				ActiveMarkers.RemoveAll([&](const FForge2QuestMarker& Marker)
				{
					return Marker.MarkerId == FString::Printf(TEXT("%s_%s"), *QuestId, *ObjectiveId);
				});

				// Activate any objectives that depend on this one
				for (FForge2QuestObjective& NextObjective : Definition->Objectives)
				{
					if (NextObjective.State == EForge2ObjectiveState::Inactive)
					{
						if (NextObjective.PrerequisiteObjectives.Contains(ObjectiveId) &&
							NextObjective.IsAvailable(Definition->Objectives))
						{
							NextObjective.State = EForge2ObjectiveState::Active;
							State->ObjectiveStates.Add(NextObjective.ObjectiveId, EForge2ObjectiveState::Active);

							// Add marker for newly active objective
							if (NextObjective.MarkerType != EForge2MarkerType::None)
							{
								AddMarkerForObjective(QuestId, NextObjective.ObjectiveId);
							}
						}
					}
				}

				// Check if quest is complete
				if (Definition->AreRequiredObjectivesComplete())
				{
					// Quest can be completed - fire completion available event
					// The quest isn't auto-completed to allow for turn-in mechanics
					State->bReadyForTurnIn = true;
					
					// Broadcast objective completed event
					OnObjectiveCompleted.Broadcast(QuestId, ObjectiveId);
					
					// Broadcast quest ready for completion event
					OnQuestReadyForTurnIn.Broadcast(QuestId);
					
					// Auto-complete if the quest has no turn-in NPC
					if (Definition->QuestTurnInId.IsEmpty() || Definition->bAutoComplete)
					{
						CompleteQuest(QuestId);
						OnQuestCompleted.Broadcast(QuestId);
					}
				}
				else
				{
					// Broadcast objective completed event
					OnObjectiveCompleted.Broadcast(QuestId, ObjectiveId);
				}
			}

			return true;
		}
	}

	return false;
}

bool FForge2QuestManager::CompleteObjective(const FString& QuestId, const FString& ObjectiveId)
{
	FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	if (!Definition)
	{
		return false;
	}

	for (FForge2QuestObjective& Objective : Definition->Objectives)
	{
		if (Objective.ObjectiveId == ObjectiveId)
		{
			return UpdateObjective(QuestId, ObjectiveId, Objective.RequiredCount - Objective.CurrentCount);
		}
	}

	return false;
}

bool FForge2QuestManager::TrackQuest(const FString& QuestId)
{
	if (TrackedQuests.Contains(QuestId))
	{
		return true;
	}

	if (TrackedQuests.Num() >= MaxTrackedQuests)
	{
		return false;
	}

	FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);
	if (!State || State->State != EForge2QuestState::Active)
	{
		return false;
	}

	TrackedQuests.Add(QuestId);
	State->bTracked = true;

	// Add markers for active objectives
	FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	if (Definition)
	{
		for (const FForge2QuestObjective& Objective : Definition->Objectives)
		{
			if (Objective.State == EForge2ObjectiveState::Active && Objective.MarkerType != EForge2MarkerType::None)
			{
				AddMarkerForObjective(QuestId, Objective.ObjectiveId);
			}
		}
	}

	return true;
}

bool FForge2QuestManager::UntrackQuest(const FString& QuestId)
{
	if (!TrackedQuests.Contains(QuestId))
	{
		return false;
	}

	TrackedQuests.Remove(QuestId);

	FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);
	if (State)
	{
		State->bTracked = false;
	}

	// Remove markers
	RemoveMarkersForQuest(QuestId);

	return true;
}

EForge2QuestState FForge2QuestManager::GetQuestState(const FString& QuestId) const
{
	const FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);
	if (State)
	{
		return State->State;
	}
	return EForge2QuestState::Unknown;
}

float FForge2QuestManager::GetQuestProgress(const FString& QuestId) const
{
	const FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	if (Definition)
	{
		return Definition->GetProgress();
	}
	return 0.0f;
}

TArray<FString> FForge2QuestManager::GetActiveQuests() const
{
	TArray<FString> Result;
	for (const auto& Pair : QuestStates)
	{
		if (Pair.Value.State == EForge2QuestState::Active)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

TArray<FString> FForge2QuestManager::GetAvailableQuests(const TMap<FString, int32>& GameState) const
{
	TArray<FString> Result;
	for (const auto& Pair : QuestDefinitions)
	{
		if (IsQuestAvailable(Pair.Key, GameState))
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

TArray<FString> FForge2QuestManager::GetQuestsByCategory(EForge2QuestCategory Category) const
{
	TArray<FString> Result;
	for (const auto& Pair : QuestDefinitions)
	{
		if (Pair.Value.Category == Category)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

void FForge2QuestManager::AddMarkerForObjective(const FString& QuestId, const FString& ObjectiveId)
{
	const FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	if (!Definition)
	{
		return;
	}

	for (const FForge2QuestObjective& Objective : Definition->Objectives)
	{
		if (Objective.ObjectiveId == ObjectiveId)
		{
			FForge2QuestMarker Marker;
			Marker.MarkerId = FString::Printf(TEXT("%s_%s"), *QuestId, *ObjectiveId);
			Marker.Location = Objective.Target.TargetLocation;
			Marker.MarkerType = Objective.MarkerType;
			Marker.VisibilityDistance = Objective.MarkerTrackingDistance;
			Marker.LabelText = Objective.ObjectiveText;

			if (Objective.MarkerIcon.IsValid())
			{
				Marker.Icon = Objective.MarkerIcon;
			}

			// Set target actor if available
			if (Objective.Target.TargetActor.IsValid())
			{
				Marker.TargetActor = Objective.Target.TargetActor.Get();
			}

			ActiveMarkers.Add(Marker);
			break;
		}
	}
}

void FForge2QuestManager::RemoveMarkersForQuest(const FString& QuestId)
{
	ActiveMarkers.RemoveAll([&](const FForge2QuestMarker& Marker)
	{
		return Marker.MarkerId.StartsWith(QuestId);
	});
}

void FForge2QuestManager::UpdateMarkers(const FVector& PlayerLocation)
{
	for (FForge2QuestMarker& Marker : ActiveMarkers)
	{
		if (!Marker.bActive)
		{
			continue;
		}

		FVector MarkerLocation = Marker.GetWorldLocation();
		float Distance = FVector::Dist(PlayerLocation, MarkerLocation);

		// Update visibility based on distance
		Marker.bActive = Distance <= Marker.VisibilityDistance;
	}
}

FString FForge2QuestManager::GetNextQuestInChain(const FString& CurrentQuestId, const TMap<FString, int32>& GameState) const
{
	const FForge2QuestDefinition* Definition = QuestDefinitions.Find(CurrentQuestId);
	if (!Definition || Definition->ChainId.IsEmpty())
	{
		return TEXT("");
	}

	const FForge2QuestChainDefinition* Chain = QuestChains.Find(Definition->ChainId);
	if (!Chain)
	{
		return TEXT("");
	}

	for (const FForge2QuestChainEntry& Entry : Chain->Entries)
	{
		if (Entry.QuestId == CurrentQuestId)
		{
			return Entry.GetNextQuest(GameState);
		}
	}

	return TEXT("");
}

float FForge2QuestManager::GetChainProgress(const FString& ChainId) const
{
	const FForge2QuestChainDefinition* Chain = QuestChains.Find(ChainId);
	if (!Chain)
	{
		return 0.0f;
	}

	return Chain->GetChainProgress(CompletedQuests);
}

void FForge2QuestManager::CheckFailConditions(const FString& QuestId, float DeltaTime, UWorld* World)
{
	FForge2QuestDefinition* Definition = QuestDefinitions.Find(QuestId);
	FForge2QuestRuntimeState* State = QuestStates.Find(QuestId);

	if (!Definition || !State || State->State != EForge2QuestState::Active)
	{
		return;
	}

	for (const FForge2QuestFailCondition& Condition : Definition->FailConditions)
	{
		switch (Condition.ConditionType)
		{
			case EForge2FailConditionType::TimeExpired:
			{
				// Check timed objectives
				for (FForge2QuestObjective& Objective : Definition->Objectives)
				{
					if (Objective.TimeLimit > 0.0f && Objective.State == EForge2ObjectiveState::Active)
					{
						Objective.TimeRemaining -= DeltaTime;
						if (Objective.TimeRemaining <= 0.0f)
						{
							Objective.State = EForge2ObjectiveState::Failed;
							State->ObjectiveStates.Add(Objective.ObjectiveId, EForge2ObjectiveState::Failed);

							if (!Objective.bOptional)
							{
								FailQuest(QuestId, FString::Printf(TEXT("Objective time expired: %s"), *Objective.ObjectiveId));
								return;
							}
						}
					}
				}
				break;
			}

			case EForge2FailConditionType::NPCDeath:
			{
				if (Condition.TargetActor.IsValid())
				{
					AActor* Target = Condition.TargetActor.Get();
					if (!IsValid(Target) || Target->IsPendingKill())
					{
						FailQuest(QuestId, FString::Printf(TEXT("NPC died: %s"), *Condition.TargetId));
						return;
					}
				}
				break;
			}

			case EForge2FailConditionType::AreaLeft:
			{
				// Check if player has left the required area
				if (World)
				{
					APlayerController* PC = World->GetFirstPlayerController();
					if (PC && PC->GetPawn())
					{
						FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
						
						// Check if player is outside the required area bounds
						if (Condition.AreaBounds.IsValid)
						{
							if (!Condition.AreaBounds.IsInside(PlayerLocation))
							{
								// Player left the required area
								FailQuest(QuestId, FString::Printf(TEXT("Left required area: %s"), *Condition.TargetId));
								return;
							}
						}
						else if (Condition.TargetActor.IsValid())
						{
							// Use target actor as area center with radius check
							AActor* AreaCenter = Condition.TargetActor.Get();
							if (AreaCenter)
							{
								float Distance = FVector::Dist(PlayerLocation, AreaCenter->GetActorLocation());
								if (Distance > Condition.AreaRadius)
								{
									FailQuest(QuestId, FString::Printf(TEXT("Left required area: %s"), *Condition.TargetId));
									return;
								}
							}
						}
						else if (!Condition.TargetId.IsEmpty())
						{
							// Find area trigger by name/tag
							for (TActorIterator<AActor> It(World); It; ++It)
							{
								AActor* Actor = *It;
								if (Actor && Actor->ActorHasTag(FName(*Condition.TargetId)))
								{
									// Check if player is still overlapping with the trigger
									TArray<AActor*> OverlappingActors;
									Actor->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
									
									bool bPlayerInArea = false;
									for (AActor* Overlapping : OverlappingActors)
									{
										if (Overlapping == PC->GetPawn())
										{
											bPlayerInArea = true;
											break;
										}
									}
									
									if (!bPlayerInArea)
									{
										FailQuest(QuestId, FString::Printf(TEXT("Left required area: %s"), *Condition.TargetId));
										return;
									}
									break;
								}
							}
						}
					}
				}
				break;
			}

			case EForge2FailConditionType::ObjectDestroyed:
			{
				if (Condition.TargetActor.IsValid())
				{
					AActor* Target = Condition.TargetActor.Get();
					if (!IsValid(Target) || Target->IsPendingKill())
					{
						FailQuest(QuestId, FString::Printf(TEXT("Object destroyed: %s"), *Condition.TargetId));
						return;
					}
				}
				break;
			}

			default:
				break;
		}
	}
}

void FForge2QuestManager::Tick(float DeltaTime, UWorld* World)
{
	// Check fail conditions for all active quests
	TArray<FString> ActiveQuestIds = GetActiveQuests();
	for (const FString& QuestId : ActiveQuestIds)
	{
		CheckFailConditions(QuestId, DeltaTime, World);
	}

	// Update cooldowns for repeatable quests
	for (auto& Pair : QuestDefinitions)
	{
		if (Pair.Value.bRepeatable && CompletedQuests.Contains(Pair.Key))
		{
			Pair.Value.TimeSinceCompletion += DeltaTime;
		}
	}
}

// ============================================================================
// FForge2QuestUtils Implementation
// ============================================================================

FForge2QuestDefinition FForge2QuestUtils::CreateKillQuest(
	const FString& QuestId,
	const FText& QuestName,
	const FString& EnemyType,
	int32 KillCount,
	int32 XPReward)
{
	FForge2QuestDefinition Quest;
	Quest.QuestId = QuestId;
	Quest.QuestName = QuestName;
	Quest.Category = EForge2QuestCategory::Side;

	// Create kill objective
	FForge2QuestObjective Objective;
	Objective.ObjectiveId = FString::Printf(TEXT("%s_kill"), *QuestId);
	Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Kill %d %s"), KillCount, *EnemyType));
	Objective.ObjectiveType = EForge2ObjectiveType::Kill;
	Objective.Target.TargetId = EnemyType;
	Objective.Target.TargetDisplayName = EnemyType;
	Objective.RequiredCount = KillCount;
	Objective.State = EForge2ObjectiveState::Active;

	Quest.Objectives.Add(Objective);

	// Add XP reward
	FForge2QuestReward Reward;
	Reward.RewardType = EForge2RewardType::Experience;
	Reward.RewardId = TEXT("XP");
	Reward.Amount = XPReward;
	Reward.DisplayName = FText::FromString(FString::Printf(TEXT("%d XP"), XPReward));

	Quest.Rewards.Add(Reward);

	return Quest;
}

FForge2QuestDefinition FForge2QuestUtils::CreateCollectQuest(
	const FString& QuestId,
	const FText& QuestName,
	const FString& ItemId,
	int32 ItemCount,
	int32 XPReward)
{
	FForge2QuestDefinition Quest;
	Quest.QuestId = QuestId;
	Quest.QuestName = QuestName;
	Quest.Category = EForge2QuestCategory::Side;

	// Create collect objective
	FForge2QuestObjective Objective;
	Objective.ObjectiveId = FString::Printf(TEXT("%s_collect"), *QuestId);
	Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Collect %d %s"), ItemCount, *ItemId));
	Objective.ObjectiveType = EForge2ObjectiveType::Collect;
	Objective.Target.TargetId = ItemId;
	Objective.Target.TargetDisplayName = ItemId;
	Objective.RequiredCount = ItemCount;
	Objective.State = EForge2ObjectiveState::Active;

	Quest.Objectives.Add(Objective);

	// Add XP reward
	FForge2QuestReward Reward;
	Reward.RewardType = EForge2RewardType::Experience;
	Reward.RewardId = TEXT("XP");
	Reward.Amount = XPReward;
	Reward.DisplayName = FText::FromString(FString::Printf(TEXT("%d XP"), XPReward));

	Quest.Rewards.Add(Reward);

	return Quest;
}

FForge2QuestDefinition FForge2QuestUtils::CreateDeliveryQuest(
	const FString& QuestId,
	const FText& QuestName,
	const FString& ItemId,
	const FString& TargetNPCId,
	int32 XPReward)
{
	FForge2QuestDefinition Quest;
	Quest.QuestId = QuestId;
	Quest.QuestName = QuestName;
	Quest.Category = EForge2QuestCategory::Side;

	// Create delivery objective
	FForge2QuestObjective Objective;
	Objective.ObjectiveId = FString::Printf(TEXT("%s_deliver"), *QuestId);
	Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Deliver %s to %s"), *ItemId, *TargetNPCId));
	Objective.ObjectiveType = EForge2ObjectiveType::Deliver;
	Objective.Target.TargetId = TargetNPCId;
	Objective.Target.TargetDisplayName = TargetNPCId;
	Objective.RequiredCount = 1;
	Objective.State = EForge2ObjectiveState::Active;
	Objective.CustomData.Add(TEXT("ItemId"), ItemId);

	Quest.Objectives.Add(Objective);
	Quest.QuestTurnInId = TargetNPCId;

	// Add XP reward
	FForge2QuestReward Reward;
	Reward.RewardType = EForge2RewardType::Experience;
	Reward.RewardId = TEXT("XP");
	Reward.Amount = XPReward;
	Reward.DisplayName = FText::FromString(FString::Printf(TEXT("%d XP"), XPReward));

	Quest.Rewards.Add(Reward);

	return Quest;
}

FForge2QuestDefinition FForge2QuestUtils::CreateEscortQuest(
	const FString& QuestId,
	const FText& QuestName,
	const FString& NPCId,
	const FVector& Destination,
	int32 XPReward)
{
	FForge2QuestDefinition Quest;
	Quest.QuestId = QuestId;
	Quest.QuestName = QuestName;
	Quest.Category = EForge2QuestCategory::Side;

	// Create escort objective
	FForge2QuestObjective Objective;
	Objective.ObjectiveId = FString::Printf(TEXT("%s_escort"), *QuestId);
	Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Escort %s to safety"), *NPCId));
	Objective.ObjectiveType = EForge2ObjectiveType::Escort;
	Objective.Target.TargetId = NPCId;
	Objective.Target.TargetDisplayName = NPCId;
	Objective.Target.TargetLocation = Destination;
	Objective.Target.TargetRadius = 300.0f;
	Objective.RequiredCount = 1;
	Objective.State = EForge2ObjectiveState::Active;

	Quest.Objectives.Add(Objective);

	// Add fail condition if NPC dies
	FForge2QuestFailCondition FailCondition;
	FailCondition.ConditionType = EForge2FailConditionType::NPCDeath;
	FailCondition.TargetId = NPCId;
	FailCondition.FailureMessage = FText::FromString(FString::Printf(TEXT("%s has died"), *NPCId));

	Quest.FailConditions.Add(FailCondition);

	// Add XP reward
	FForge2QuestReward Reward;
	Reward.RewardType = EForge2RewardType::Experience;
	Reward.RewardId = TEXT("XP");
	Reward.Amount = XPReward;
	Reward.DisplayName = FText::FromString(FString::Printf(TEXT("%d XP"), XPReward));

	Quest.Rewards.Add(Reward);

	return Quest;
}

FForge2QuestDefinition FForge2QuestUtils::CreateMultiObjectiveQuest(
	const FString& QuestId,
	const FText& QuestName,
	const TArray<FForge2QuestObjective>& Objectives,
	const TArray<FForge2QuestReward>& Rewards)
{
	FForge2QuestDefinition Quest;
	Quest.QuestId = QuestId;
	Quest.QuestName = QuestName;
	Quest.Category = EForge2QuestCategory::Side;
	Quest.Objectives = Objectives;
	Quest.Rewards = Rewards;

	return Quest;
}

FForge2QuestChainDefinition FForge2QuestUtils::CreateLinearChain(
	const FString& ChainId,
	const FText& ChainName,
	const TArray<FString>& QuestIds)
{
	FForge2QuestChainDefinition Chain;
	Chain.ChainId = ChainId;
	Chain.ChainName = ChainName;

	for (int32 i = 0; i < QuestIds.Num(); i++)
	{
		FForge2QuestChainEntry Entry;
		Entry.QuestId = QuestIds[i];
		Entry.ChainOrder = i;
		Entry.bChainStart = (i == 0);
		Entry.bChainEnd = (i == QuestIds.Num() - 1);

		if (i < QuestIds.Num() - 1)
		{
			Entry.DefaultNextQuest = QuestIds[i + 1];
		}

		Chain.Entries.Add(Entry);
	}

	return Chain;
}

FForge2QuestChainDefinition FForge2QuestUtils::CreateBranchingChain(
	const FString& ChainId,
	const FText& ChainName,
	const TArray<FForge2QuestChainEntry>& Entries)
{
	FForge2QuestChainDefinition Chain;
	Chain.ChainId = ChainId;
	Chain.ChainName = ChainName;
	Chain.Entries = Entries;
	Chain.bHasMultipleEndings = true;

	// Count endings
	int32 EndCount = 0;
	for (const FForge2QuestChainEntry& Entry : Entries)
	{
		if (Entry.bChainEnd)
		{
			EndCount++;
		}
	}
	Chain.EndingCount = FMath::Max(1, EndCount);

	return Chain;
}

FForge2QuestObjective FForge2QuestUtils::GenerateObjective(
	EForge2ObjectiveType Type,
	const FString& TargetId,
	int32 Count,
	bool bOptional)
{
	FForge2QuestObjective Objective;
	Objective.ObjectiveId = FString::Printf(TEXT("obj_%s_%d"), *TargetId, FMath::Rand());
	Objective.ObjectiveType = Type;
	Objective.Target.TargetId = TargetId;
	Objective.Target.TargetDisplayName = TargetId;
	Objective.RequiredCount = Count;
	Objective.bOptional = bOptional;

	// Generate objective text based on type
	switch (Type)
	{
		case EForge2ObjectiveType::Kill:
			Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Kill %d %s"), Count, *TargetId));
			break;
		case EForge2ObjectiveType::Collect:
			Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Collect %d %s"), Count, *TargetId));
			break;
		case EForge2ObjectiveType::Interact:
			Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Interact with %s"), *TargetId));
			break;
		case EForge2ObjectiveType::TalkTo:
			Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Talk to %s"), *TargetId));
			break;
		case EForge2ObjectiveType::ReachLocation:
			Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Go to %s"), *TargetId));
			break;
		default:
			Objective.ObjectiveText = FText::FromString(FString::Printf(TEXT("Complete: %s"), *TargetId));
			break;
	}

	return Objective;
}

FForge2QuestReward FForge2QuestUtils::GenerateReward(
	EForge2RewardType Type,
	const FString& RewardId,
	int32 Amount)
{
	FForge2QuestReward Reward;
	Reward.RewardType = Type;
	Reward.RewardId = RewardId;
	Reward.Amount = Amount;

	// Generate display name based on type
	switch (Type)
	{
		case EForge2RewardType::Experience:
			Reward.DisplayName = FText::FromString(FString::Printf(TEXT("%d XP"), Amount));
			break;
		case EForge2RewardType::Currency:
			Reward.DisplayName = FText::FromString(FString::Printf(TEXT("%d %s"), Amount, *RewardId));
			break;
		case EForge2RewardType::Item:
			Reward.DisplayName = FText::FromString(FString::Printf(TEXT("%s x%d"), *RewardId, Amount));
			break;
		case EForge2RewardType::Reputation:
			Reward.DisplayName = FText::FromString(FString::Printf(TEXT("+%d %s Reputation"), Amount, *RewardId));
			break;
		default:
			Reward.DisplayName = FText::FromString(RewardId);
			break;
	}

	return Reward;
}

TArray<FForge2QuestReward> FForge2QuestUtils::CalculateScaledRewards(
	int32 PlayerLevel,
	int32 QuestDifficulty,
	EForge2QuestCategory Category)
{
	TArray<FForge2QuestReward> Rewards;

	// Base values
	float BaseXP = 100.0f;
	float BaseGold = 50.0f;

	// Level scaling
	float LevelMultiplier = 1.0f + (PlayerLevel - 1) * 0.1f;

	// Difficulty multiplier
	float DifficultyMultiplier = 1.0f + QuestDifficulty * 0.25f;

	// Category multiplier
	float CategoryMultiplier = 1.0f;
	switch (Category)
	{
		case EForge2QuestCategory::Main:
			CategoryMultiplier = 2.0f;
			break;
		case EForge2QuestCategory::Daily:
			CategoryMultiplier = 0.5f;
			break;
		case EForge2QuestCategory::Weekly:
			CategoryMultiplier = 1.5f;
			break;
		case EForge2QuestCategory::Hidden:
			CategoryMultiplier = 2.5f;
			break;
		default:
			CategoryMultiplier = 1.0f;
			break;
	}

	// Calculate final values
	int32 FinalXP = FMath::RoundToInt(BaseXP * LevelMultiplier * DifficultyMultiplier * CategoryMultiplier);
	int32 FinalGold = FMath::RoundToInt(BaseGold * LevelMultiplier * DifficultyMultiplier * CategoryMultiplier);

	// Add XP reward
	Rewards.Add(GenerateReward(EForge2RewardType::Experience, TEXT("XP"), FinalXP));

	// Add gold reward
	Rewards.Add(GenerateReward(EForge2RewardType::Currency, TEXT("Gold"), FinalGold));

	return Rewards;
}

FString FForge2QuestUtils::SerializeQuestState(const FForge2QuestRuntimeState& State)
{
	// Simple serialization format: QuestId|State|AcceptedTime|CompletedTime|bTracked|CompletionCount|ObjectiveProgress
	FString Result;
	Result += State.QuestId + TEXT("|");
	Result += FString::FromInt((int32)State.State) + TEXT("|");
	Result += FString::SanitizeFloat(State.AcceptedTime) + TEXT("|");
	Result += FString::SanitizeFloat(State.CompletedTime) + TEXT("|");
	Result += State.bTracked ? TEXT("1") : TEXT("0");
	Result += TEXT("|");
	Result += FString::FromInt(State.CompletionCount);
	Result += TEXT("|");

	// Serialize objective progress
	for (const auto& Pair : State.ObjectiveProgress)
	{
		Result += Pair.Key + TEXT(":") + FString::FromInt(Pair.Value) + TEXT(",");
	}

	return Result;
}

FForge2QuestRuntimeState FForge2QuestUtils::DeserializeQuestState(const FString& Data)
{
	FForge2QuestRuntimeState State;

	TArray<FString> Parts;
	Data.ParseIntoArray(Parts, TEXT("|"));

	if (Parts.Num() >= 6)
	{
		State.QuestId = Parts[0];
		State.State = (EForge2QuestState)FCString::Atoi(*Parts[1]);
		State.AcceptedTime = FCString::Atof(*Parts[2]);
		State.CompletedTime = FCString::Atof(*Parts[3]);
		State.bTracked = Parts[4] == TEXT("1");
		State.CompletionCount = FCString::Atoi(*Parts[5]);

		// Deserialize objective progress
		if (Parts.Num() > 6)
		{
			TArray<FString> ProgressParts;
			Parts[6].ParseIntoArray(ProgressParts, TEXT(","));

			for (const FString& ProgressPart : ProgressParts)
			{
				TArray<FString> KeyValue;
				ProgressPart.ParseIntoArray(KeyValue, TEXT(":"));
				if (KeyValue.Num() == 2)
				{
					State.ObjectiveProgress.Add(KeyValue[0], FCString::Atoi(*KeyValue[1]));
				}
			}
		}
	}

	return State;
}
