// Forge2DialogueSystem.cpp
// FORGE2 Dialogue System Implementation
// Part of the FORGE2 Ultimate AAA Game Development MCP Extension

#include "Forge2DialogueSystem.h"

// ============================================================================
// FForge2SpeakerProfile Implementation
// ============================================================================

TSoftObjectPtr<UTexture2D> FForge2SpeakerProfile::GetPortraitForEmotion(EForge2DialogueEmotionType Emotion) const
{
	const TSoftObjectPtr<UTexture2D>* EmotionPortrait = EmotionPortraits.Find(Emotion);
	if (EmotionPortrait && EmotionPortrait->IsValid())
	{
		return *EmotionPortrait;
	}
	return Portrait;
}

// ============================================================================
// FForge2DialogueCondition Implementation
// ============================================================================

bool FForge2DialogueCondition::Evaluate(const TMap<FString, int32>& GameState) const
{
	bool Result = false;

	switch (ConditionType)
	{
		case EForge2DialogueConditionType::HasItem:
		{
			FString Key = FString::Printf(TEXT("Item_%s_Count"), *TargetId);
			const int32* Count = GameState.Find(Key);
			Result = Count && CompareValue(*Count);
			break;
		}

		case EForge2DialogueConditionType::QuestComplete:
		{
			FString Key = FString::Printf(TEXT("Quest_%s_Complete"), *TargetId);
			const int32* Complete = GameState.Find(Key);
			Result = Complete && *Complete > 0;
			break;
		}

		case EForge2DialogueConditionType::QuestActive:
		{
			FString Key = FString::Printf(TEXT("Quest_%s_Active"), *TargetId);
			const int32* Active = GameState.Find(Key);
			Result = Active && *Active > 0;
			break;
		}

		case EForge2DialogueConditionType::SkillCheck:
		{
			FString Key = FString::Printf(TEXT("Skill_%s_Level"), *TargetId);
			const int32* SkillLevel = GameState.Find(Key);
			Result = SkillLevel && CompareValue(*SkillLevel);
			break;
		}

		case EForge2DialogueConditionType::ReputationCheck:
		{
			FString Key = FString::Printf(TEXT("Reputation_%s"), *TargetId);
			const int32* Rep = GameState.Find(Key);
			Result = Rep && CompareValue(*Rep);
			break;
		}

		case EForge2DialogueConditionType::StatCheck:
		{
			FString Key = FString::Printf(TEXT("Stat_%s"), *TargetId);
			const int32* Stat = GameState.Find(Key);
			Result = Stat && CompareValue(*Stat);
			break;
		}

		case EForge2DialogueConditionType::VariableCheck:
		{
			const int32* Var = GameState.Find(TargetId);
			Result = Var && CompareValue(*Var);
			break;
		}

		case EForge2DialogueConditionType::FlagCheck:
		{
			FString Key = FString::Printf(TEXT("Flag_%s"), *TargetId);
			const int32* Flag = GameState.Find(Key);
			Result = Flag && *Flag > 0;
			break;
		}

		case EForge2DialogueConditionType::TimeCheck:
		{
			const int32* Hour = GameState.Find(TEXT("GameTimeHour"));
			Result = Hour && CompareValue(*Hour);
			break;
		}

		case EForge2DialogueConditionType::RandomChance:
		{
			Result = FMath::FRand() * 100.0f <= RandomChance;
			break;
		}

		case EForge2DialogueConditionType::ClassCheck:
		{
			FString Key = FString::Printf(TEXT("PlayerClass_%s"), *TargetId);
			const int32* HasClass = GameState.Find(Key);
			Result = HasClass && *HasClass > 0;
			break;
		}

		case EForge2DialogueConditionType::RaceCheck:
		{
			FString Key = FString::Printf(TEXT("PlayerRace_%s"), *TargetId);
			const int32* HasRace = GameState.Find(Key);
			Result = HasRace && *HasRace > 0;
			break;
		}

		case EForge2DialogueConditionType::LevelCheck:
		{
			const int32* Level = GameState.Find(TEXT("PlayerLevel"));
			Result = Level && CompareValue(*Level);
			break;
		}

		case EForge2DialogueConditionType::GenderCheck:
		{
			FString Key = FString::Printf(TEXT("PlayerGender_%s"), *TargetId);
			const int32* HasGender = GameState.Find(Key);
			Result = HasGender && *HasGender > 0;
			break;
		}

		case EForge2DialogueConditionType::Custom:
		{
			FString Key = FString::Printf(TEXT("CustomCondition_%s"), *CustomFunction);
			const int32* CustomResult = GameState.Find(Key);
			Result = CustomResult && *CustomResult > 0;
			break;
		}
	}

	return bInvert ? !Result : Result;
}

bool FForge2DialogueCondition::CompareValue(int32 ActualValue) const
{
	switch (Comparison)
	{
		case EForge2DialogueComparisonType::Equal:
			return ActualValue == RequiredValue;
		case EForge2DialogueComparisonType::NotEqual:
			return ActualValue != RequiredValue;
		case EForge2DialogueComparisonType::Greater:
			return ActualValue > RequiredValue;
		case EForge2DialogueComparisonType::GreaterOrEqual:
			return ActualValue >= RequiredValue;
		case EForge2DialogueComparisonType::Less:
			return ActualValue < RequiredValue;
		case EForge2DialogueComparisonType::LessOrEqual:
			return ActualValue <= RequiredValue;
		default:
			return false;
	}
}

// ============================================================================
// FForge2DialogueConsequence Implementation
// ============================================================================

void FForge2DialogueConsequence::Execute(TMap<FString, int32>& GameState) const
{
	switch (ConsequenceType)
	{
		case EForge2DialogueConsequenceType::SetVariable:
		{
			GameState.Add(TargetId, Value);
			break;
		}

		case EForge2DialogueConsequenceType::GiveItem:
		{
			FString Key = FString::Printf(TEXT("Item_%s_Count"), *TargetId);
			int32 CurrentCount = GameState.FindOrAdd(Key);
			GameState.Add(Key, CurrentCount + Value);
			break;
		}

		case EForge2DialogueConsequenceType::TakeItem:
		{
			FString Key = FString::Printf(TEXT("Item_%s_Count"), *TargetId);
			int32* CurrentCount = GameState.Find(Key);
			if (CurrentCount)
			{
				*CurrentCount = FMath::Max(0, *CurrentCount - Value);
			}
			break;
		}

		case EForge2DialogueConsequenceType::StartQuest:
		{
			FString Key = FString::Printf(TEXT("Quest_%s_Active"), *TargetId);
			GameState.Add(Key, 1);
			break;
		}

		case EForge2DialogueConsequenceType::CompleteQuest:
		{
			FString ActiveKey = FString::Printf(TEXT("Quest_%s_Active"), *TargetId);
			FString CompleteKey = FString::Printf(TEXT("Quest_%s_Complete"), *TargetId);
			GameState.Add(ActiveKey, 0);
			GameState.Add(CompleteKey, 1);
			break;
		}

		case EForge2DialogueConsequenceType::FailQuest:
		{
			FString ActiveKey = FString::Printf(TEXT("Quest_%s_Active"), *TargetId);
			FString FailedKey = FString::Printf(TEXT("Quest_%s_Failed"), *TargetId);
			GameState.Add(ActiveKey, 0);
			GameState.Add(FailedKey, 1);
			break;
		}

		case EForge2DialogueConsequenceType::CompleteObjective:
		{
			FString Key = FString::Printf(TEXT("Objective_%s_Complete"), *TargetId);
			GameState.Add(Key, 1);
			break;
		}

		case EForge2DialogueConsequenceType::ChangeReputation:
		{
			FString Key = FString::Printf(TEXT("Reputation_%s"), *TargetId);
			int32 CurrentRep = GameState.FindOrAdd(Key);
			GameState.Add(Key, CurrentRep + Value);
			break;
		}

		case EForge2DialogueConsequenceType::GiveCurrency:
		{
			FString Key = FString::Printf(TEXT("Currency_%s"), *TargetId);
			int32 Current = GameState.FindOrAdd(Key);
			GameState.Add(Key, Current + Value);
			break;
		}

		case EForge2DialogueConsequenceType::TakeCurrency:
		{
			FString Key = FString::Printf(TEXT("Currency_%s"), *TargetId);
			int32* Current = GameState.Find(Key);
			if (Current)
			{
				*Current = FMath::Max(0, *Current - Value);
			}
			break;
		}

		case EForge2DialogueConsequenceType::GiveXP:
		{
			int32 CurrentXP = GameState.FindOrAdd(TEXT("PlayerXP"));
			GameState.Add(TEXT("PlayerXP"), CurrentXP + Value);
			break;
		}

		case EForge2DialogueConsequenceType::UnlockAbility:
		{
			FString Key = FString::Printf(TEXT("Ability_%s_Unlocked"), *TargetId);
			GameState.Add(Key, 1);
			break;
		}

		case EForge2DialogueConsequenceType::SetFlag:
		{
			FString Key = FString::Printf(TEXT("Flag_%s"), *TargetId);
			GameState.Add(Key, Value);
			break;
		}

		case EForge2DialogueConsequenceType::TriggerEvent:
		case EForge2DialogueConsequenceType::SpawnActor:
		case EForge2DialogueConsequenceType::DestroyActor:
		case EForge2DialogueConsequenceType::PlaySound:
		case EForge2DialogueConsequenceType::PlayAnimation:
		case EForge2DialogueConsequenceType::StartCombat:
		case EForge2DialogueConsequenceType::Custom:
		{
			// These require external handling
			FString Key = FString::Printf(TEXT("PendingAction_%s"), *TargetId);
			GameState.Add(Key, (int32)ConsequenceType);
			break;
		}

		case EForge2DialogueConsequenceType::EndDialogue:
		{
			GameState.Add(TEXT("DialogueEnded"), 1);
			break;
		}
	}
}

// ============================================================================
// FForge2DialogueChoice Implementation
// ============================================================================

bool FForge2DialogueChoice::AreConditionsMet(const TMap<FString, int32>& GameState) const
{
	for (const FForge2DialogueCondition& Condition : Conditions)
	{
		if (!Condition.Evaluate(GameState))
		{
			return false;
		}
	}
	return true;
}

FText FForge2DialogueChoice::GetFormattedText(const TMap<FString, int32>& GameState) const
{
	FString FormattedText = ChoiceText.ToString();

	// Add condition indicators
	for (const FForge2DialogueCondition& Condition : Conditions)
	{
		if (Condition.bShowInChoiceText && !Condition.ConditionDisplayText.IsEmpty())
		{
			bool bConditionMet = Condition.Evaluate(GameState);
			FLinearColor Color = bConditionMet ? Condition.MetColor : Condition.UnmetColor;

			// In a real implementation, you'd use rich text formatting
			FormattedText = FString::Printf(TEXT("[%s] %s"),
				*Condition.ConditionDisplayText.ToString(),
				*FormattedText);
		}
	}

	return FText::FromString(FormattedText);
}

// ============================================================================
// FForge2DialogueNode Implementation
// ============================================================================

float FForge2DialogueNode::GetDisplayDuration(float CharactersPerSecond) const
{
	if (DisplayDuration > 0.0f)
	{
		return DisplayDuration;
	}

	// Calculate based on text length
	int32 TextLength = DialogueText.ToString().Len();
	float CalculatedDuration = TextLength / CharactersPerSecond;

	// Minimum duration
	return FMath::Max(1.0f, CalculatedDuration);
}

FString FForge2DialogueNode::GetNextNode(const TMap<FString, int32>& GameState, int32 ChoiceIndex) const
{
	switch (NodeType)
	{
		case EForge2DialogueNodeType::Speech:
		case EForge2DialogueNodeType::Animation:
		case EForge2DialogueNodeType::Delay:
		case EForge2DialogueNodeType::SetVariable:
		case EForge2DialogueNodeType::Event:
		case EForge2DialogueNodeType::Camera:
		{
			return NextNodeId;
		}

		case EForge2DialogueNodeType::PlayerChoice:
		{
			if (ChoiceIndex >= 0 && ChoiceIndex < Choices.Num())
			{
				return Choices[ChoiceIndex].TargetNodeId;
			}
			return TEXT("");
		}

		case EForge2DialogueNodeType::Condition:
		{
			for (int32 i = 0; i < BranchConditions.Num() && i < BranchTargets.Num(); i++)
			{
				if (BranchConditions[i].Evaluate(GameState))
				{
					return BranchTargets[i];
				}
			}
			return DefaultBranch;
		}

		case EForge2DialogueNodeType::Random:
		{
			if (BranchTargets.Num() == 0)
			{
				return TEXT("");
			}

			// Calculate total weight
			float TotalWeight = 0.0f;
			for (int32 i = 0; i < BranchTargets.Num(); i++)
			{
				float Weight = (i < RandomWeights.Num()) ? RandomWeights[i] : 1.0f;
				TotalWeight += Weight;
			}

			// Select random target
			float RandomValue = FMath::FRand() * TotalWeight;
			float AccumulatedWeight = 0.0f;

			for (int32 i = 0; i < BranchTargets.Num(); i++)
			{
				float Weight = (i < RandomWeights.Num()) ? RandomWeights[i] : 1.0f;
				AccumulatedWeight += Weight;

				if (RandomValue <= AccumulatedWeight)
				{
					return BranchTargets[i];
				}
			}

			return BranchTargets.Last();
		}

		case EForge2DialogueNodeType::Jump:
		{
			return JumpTargetTree;
		}

		case EForge2DialogueNodeType::End:
		case EForge2DialogueNodeType::Start:
		default:
		{
			return NextNodeId;
		}
	}
}

bool FForge2DialogueNode::CanEnter(const TMap<FString, int32>& GameState) const
{
	for (const FForge2DialogueCondition& Condition : EntryConditions)
	{
		if (!Condition.Evaluate(GameState))
		{
			return false;
		}
	}
	return true;
}

void FForge2DialogueNode::OnEnter(TMap<FString, int32>& GameState) const
{
	for (const FForge2DialogueConsequence& Consequence : EntryConsequences)
	{
		Consequence.Execute(GameState);
	}

	// Handle SetVariable nodes
	if (NodeType == EForge2DialogueNodeType::SetVariable && !VariableName.IsEmpty())
	{
		GameState.Add(VariableName, VariableValue);
	}
}

void FForge2DialogueNode::OnExit(TMap<FString, int32>& GameState) const
{
	for (const FForge2DialogueConsequence& Consequence : ExitConsequences)
	{
		Consequence.Execute(GameState);
	}
}

// ============================================================================
// FForge2DialogueTreeDefinition Implementation
// ============================================================================

FForge2DialogueNode* FForge2DialogueTreeDefinition::GetNode(const FString& NodeId)
{
	for (FForge2DialogueNode& Node : Nodes)
	{
		if (Node.NodeId == NodeId)
		{
			return &Node;
		}
	}
	return nullptr;
}

const FForge2DialogueNode* FForge2DialogueTreeDefinition::GetNode(const FString& NodeId) const
{
	for (const FForge2DialogueNode& Node : Nodes)
	{
		if (Node.NodeId == NodeId)
		{
			return &Node;
		}
	}
	return nullptr;
}

FForge2SpeakerProfile* FForge2DialogueTreeDefinition::GetSpeaker(const FString& SpeakerId)
{
	for (FForge2SpeakerProfile& Speaker : Speakers)
	{
		if (Speaker.SpeakerId == SpeakerId)
		{
			return &Speaker;
		}
	}
	return nullptr;
}

const FForge2SpeakerProfile* FForge2DialogueTreeDefinition::GetSpeaker(const FString& SpeakerId) const
{
	for (const FForge2SpeakerProfile& Speaker : Speakers)
	{
		if (Speaker.SpeakerId == SpeakerId)
		{
			return &Speaker;
		}
	}
	return nullptr;
}

bool FForge2DialogueTreeDefinition::Validate(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();

	// Check for start node
	if (StartNodeId.IsEmpty())
	{
		OutErrors.Add(TEXT("No start node specified"));
	}
	else if (!GetNode(StartNodeId))
	{
		OutErrors.Add(FString::Printf(TEXT("Start node '%s' not found"), *StartNodeId));
	}

	// Check all node references
	for (const FForge2DialogueNode& Node : Nodes)
	{
		// Check next node reference
		if (!Node.NextNodeId.IsEmpty() && !GetNode(Node.NextNodeId))
		{
			OutErrors.Add(FString::Printf(TEXT("Node '%s' references non-existent node '%s'"),
				*Node.NodeId, *Node.NextNodeId));
		}

		// Check choice references
		for (const FForge2DialogueChoice& Choice : Node.Choices)
		{
			if (!Choice.TargetNodeId.IsEmpty() && !GetNode(Choice.TargetNodeId))
			{
				OutErrors.Add(FString::Printf(TEXT("Choice '%s' in node '%s' references non-existent node '%s'"),
					*Choice.ChoiceId, *Node.NodeId, *Choice.TargetNodeId));
			}
		}

		// Check branch targets
		for (const FString& Target : Node.BranchTargets)
		{
			if (!Target.IsEmpty() && !GetNode(Target))
			{
				OutErrors.Add(FString::Printf(TEXT("Branch in node '%s' references non-existent node '%s'"),
					*Node.NodeId, *Target));
			}
		}

		// Check default branch
		if (!Node.DefaultBranch.IsEmpty() && !GetNode(Node.DefaultBranch))
		{
			OutErrors.Add(FString::Printf(TEXT("Default branch in node '%s' references non-existent node '%s'"),
				*Node.NodeId, *Node.DefaultBranch));
		}

		// Check speaker reference
		if (Node.NodeType == EForge2DialogueNodeType::Speech && !Node.SpeakerId.IsEmpty())
		{
			if (!GetSpeaker(Node.SpeakerId))
			{
				OutErrors.Add(FString::Printf(TEXT("Node '%s' references non-existent speaker '%s'"),
					*Node.NodeId, *Node.SpeakerId));
			}
		}
	}

	return OutErrors.Num() == 0;
}

// ============================================================================
// FForge2DialogueManager Implementation
// ============================================================================

bool FForge2DialogueManager::RegisterDialogueTree(const FForge2DialogueTreeDefinition& Tree)
{
	if (Tree.TreeId.IsEmpty())
	{
		return false;
	}

	TArray<FString> Errors;
	if (!Tree.Validate(Errors))
	{
		// Log errors but still register
		for (const FString& Error : Errors)
		{
			UE_LOG(LogTemp, Warning, TEXT("Dialogue '%s' validation: %s"), *Tree.TreeId, *Error);
		}
	}

	DialogueTrees.Add(Tree.TreeId, Tree);
	return true;
}

bool FForge2DialogueManager::IsDialogueAvailable(const FString& TreeId) const
{
	const FForge2DialogueTreeDefinition* Tree = DialogueTrees.Find(TreeId);
	if (!Tree)
	{
		return false;
	}

	// Check if already completed and not repeatable
	const FForge2DialogueRuntimeState* State = RuntimeStates.Find(TreeId);
	if (State && State->bCompleted)
	{
		if (!Tree->bRepeatable)
		{
			return false;
		}
		if (Tree->bOncePerSession)
		{
			return false;
		}
	}

	// Check prerequisites
	if (GameState)
	{
		for (const FForge2DialogueCondition& Prereq : Tree->Prerequisites)
		{
			if (!Prereq.Evaluate(*GameState))
			{
				return false;
			}
		}
	}

	return true;
}

bool FForge2DialogueManager::StartDialogue(const FString& TreeId)
{
	if (!IsDialogueAvailable(TreeId))
	{
		return false;
	}

	const FForge2DialogueTreeDefinition* Tree = DialogueTrees.Find(TreeId);
	if (!Tree || Tree->StartNodeId.IsEmpty())
	{
		return false;
	}

	// Create or update runtime state
	FForge2DialogueRuntimeState& State = RuntimeStates.FindOrAdd(TreeId);
	State.TreeId = TreeId;
	State.CurrentNodeId = Tree->StartNodeId;
	State.bCompleted = false;
	State.StartTime = FPlatformTime::Seconds();

	ActiveDialogueId = TreeId;
	bDialogueActive = true;

	// Execute start consequences
	if (GameState)
	{
		for (const FForge2DialogueConsequence& Consequence : Tree->OnStartConsequences)
		{
			Consequence.Execute(*GameState);
		}

		// Enter first node
		const FForge2DialogueNode* StartNode = Tree->GetNode(Tree->StartNodeId);
		if (StartNode)
		{
			StartNode->OnEnter(*GameState);
			MarkNodeVisited(TreeId, Tree->StartNodeId);
		}
	}

	return true;
}

void FForge2DialogueManager::EndDialogue()
{
	if (!bDialogueActive || ActiveDialogueId.IsEmpty())
	{
		return;
	}

	FForge2DialogueRuntimeState* State = RuntimeStates.Find(ActiveDialogueId);
	const FForge2DialogueTreeDefinition* Tree = DialogueTrees.Find(ActiveDialogueId);

	if (State)
	{
		State->bCompleted = true;
		State->TotalTime = FPlatformTime::Seconds() - State->StartTime;
		State->CompletionCount++;
	}

	// Execute end consequences
	if (Tree && GameState)
	{
		for (const FForge2DialogueConsequence& Consequence : Tree->OnEndConsequences)
		{
			Consequence.Execute(*GameState);
		}
	}

	ActiveDialogueId = TEXT("");
	bDialogueActive = false;
}

const FForge2DialogueNode* FForge2DialogueManager::GetCurrentNode() const
{
	if (!bDialogueActive || ActiveDialogueId.IsEmpty())
	{
		return nullptr;
	}

	const FForge2DialogueTreeDefinition* Tree = DialogueTrees.Find(ActiveDialogueId);
	const FForge2DialogueRuntimeState* State = RuntimeStates.Find(ActiveDialogueId);

	if (Tree && State)
	{
		return Tree->GetNode(State->CurrentNodeId);
	}

	return nullptr;
}

bool FForge2DialogueManager::AdvanceDialogue(int32 ChoiceIndex)
{
	if (!bDialogueActive || !GameState)
	{
		return false;
	}

	FForge2DialogueTreeDefinition* Tree = DialogueTrees.Find(ActiveDialogueId);
	FForge2DialogueRuntimeState* State = RuntimeStates.Find(ActiveDialogueId);

	if (!Tree || !State)
	{
		return false;
	}

	FForge2DialogueNode* CurrentNode = Tree->GetNode(State->CurrentNodeId);
	if (!CurrentNode)
	{
		EndDialogue();
		return false;
	}

	// Exit current node
	CurrentNode->OnExit(*GameState);

	// Get next node
	FString NextNodeId = CurrentNode->GetNextNode(*GameState, ChoiceIndex);

	// Handle choice selection
	if (CurrentNode->NodeType == EForge2DialogueNodeType::PlayerChoice &&
		ChoiceIndex >= 0 && ChoiceIndex < CurrentNode->Choices.Num())
	{
		const FForge2DialogueChoice& Choice = CurrentNode->Choices[ChoiceIndex];
		State->SelectedChoices.Add(Choice.ChoiceId);

		// Execute choice consequences
		for (const FForge2DialogueConsequence& Consequence : Choice.Consequences)
		{
			Consequence.Execute(*GameState);
		}
	}

	// Check for end
	if (NextNodeId.IsEmpty())
	{
		EndDialogue();
		return true;
	}

	// Move to next node
	FForge2DialogueNode* NextNode = Tree->GetNode(NextNodeId);
	if (!NextNode)
	{
		EndDialogue();
		return false;
	}

	// Check if we can enter next node
	if (!NextNode->CanEnter(*GameState))
	{
		// Skip to next available node
		FString SkipNodeId = NextNode->GetNextNode(*GameState);
		if (!SkipNodeId.IsEmpty())
		{
			NextNode = Tree->GetNode(SkipNodeId);
			NextNodeId = SkipNodeId;
		}
	}

	if (!NextNode)
	{
		EndDialogue();
		return false;
	}

	// Check for end node
	if (NextNode->NodeType == EForge2DialogueNodeType::End)
	{
		EndDialogue();
		return true;
	}

	// Enter next node
	State->CurrentNodeId = NextNodeId;
	NextNode->OnEnter(*GameState);
	MarkNodeVisited(ActiveDialogueId, NextNodeId);

	// Handle special nodes that auto-advance
	if (NextNode->NodeType == EForge2DialogueNodeType::Event ||
		NextNode->NodeType == EForge2DialogueNodeType::SetVariable ||
		NextNode->NodeType == EForge2DialogueNodeType::Condition ||
		NextNode->NodeType == EForge2DialogueNodeType::Random)
	{
		return AdvanceDialogue(-1);
	}

	return true;
}

TArray<FForge2DialogueChoice> FForge2DialogueManager::GetAvailableChoices() const
{
	TArray<FForge2DialogueChoice> Result;

	const FForge2DialogueNode* CurrentNode = GetCurrentNode();
	if (!CurrentNode || CurrentNode->NodeType != EForge2DialogueNodeType::PlayerChoice)
	{
		return Result;
	}

	for (const FForge2DialogueChoice& Choice : CurrentNode->Choices)
	{
		bool bConditionsMet = true;
		if (GameState)
		{
			bConditionsMet = Choice.AreConditionsMet(*GameState);
		}

		if (bConditionsMet || Choice.bShowWhenLocked)
		{
			Result.Add(Choice);
		}
	}

	// Sort by display order
	Result.Sort([](const FForge2DialogueChoice& A, const FForge2DialogueChoice& B)
	{
		return A.DisplayOrder < B.DisplayOrder;
	});

	return Result;
}

bool FForge2DialogueManager::SelectChoice(int32 ChoiceIndex)
{
	const FForge2DialogueNode* CurrentNode = GetCurrentNode();
	if (!CurrentNode || CurrentNode->NodeType != EForge2DialogueNodeType::PlayerChoice)
	{
		return false;
	}

	if (ChoiceIndex < 0 || ChoiceIndex >= CurrentNode->Choices.Num())
	{
		return false;
	}

	const FForge2DialogueChoice& Choice = CurrentNode->Choices[ChoiceIndex];

	// Check if choice is selectable
	if (GameState && !Choice.AreConditionsMet(*GameState) && !Choice.bSelectableWhenLocked)
	{
		return false;
	}

	return AdvanceDialogue(ChoiceIndex);
}

bool FForge2DialogueManager::SkipCurrentNode()
{
	const FForge2DialogueNode* CurrentNode = GetCurrentNode();
	if (!CurrentNode || !CurrentNode->bSkippable)
	{
		return false;
	}

	if (CurrentNode->NodeType == EForge2DialogueNodeType::PlayerChoice)
	{
		return false;
	}

	return AdvanceDialogue(-1);
}

bool FForge2DialogueManager::IsCurrentNodeSkippable() const
{
	const FForge2DialogueNode* CurrentNode = GetCurrentNode();
	if (!CurrentNode)
	{
		return false;
	}

	return CurrentNode->bSkippable && CurrentNode->NodeType != EForge2DialogueNodeType::PlayerChoice;
}

const FForge2DialogueRuntimeState* FForge2DialogueManager::GetDialogueState(const FString& TreeId) const
{
	return RuntimeStates.Find(TreeId);
}

void FForge2DialogueManager::ResetDialogueState(const FString& TreeId)
{
	RuntimeStates.Remove(TreeId);
}

TArray<FString> FForge2DialogueManager::GetDialoguesForSpeaker(const FString& SpeakerId) const
{
	TArray<FString> Result;

	for (const auto& Pair : DialogueTrees)
	{
		for (const FForge2SpeakerProfile& Speaker : Pair.Value.Speakers)
		{
			if (Speaker.SpeakerId == SpeakerId)
			{
				Result.Add(Pair.Key);
				break;
			}
		}
	}

	return Result;
}

FString FForge2DialogueManager::GetBestDialogueForSpeaker(const FString& SpeakerId) const
{
	TArray<FString> Dialogues = GetDialoguesForSpeaker(SpeakerId);

	FString BestDialogue;
	int32 HighestPriority = INT_MIN;

	for (const FString& TreeId : Dialogues)
	{
		if (IsDialogueAvailable(TreeId))
		{
			const FForge2DialogueTreeDefinition* Tree = DialogueTrees.Find(TreeId);
			if (Tree && Tree->Priority > HighestPriority)
			{
				HighestPriority = Tree->Priority;
				BestDialogue = TreeId;
			}
		}
	}

	return BestDialogue;
}

void FForge2DialogueManager::MarkNodeVisited(const FString& TreeId, const FString& NodeId)
{
	FForge2DialogueRuntimeState* State = RuntimeStates.Find(TreeId);
	if (State)
	{
		State->VisitedNodes.Add(NodeId);
	}
}

bool FForge2DialogueManager::HasNodeBeenVisited(const FString& TreeId, const FString& NodeId) const
{
	const FForge2DialogueRuntimeState* State = RuntimeStates.Find(TreeId);
	if (State)
	{
		return State->VisitedNodes.Contains(NodeId);
	}
	return false;
}

// ============================================================================
// FForge2DialogueUtils Implementation
// ============================================================================

FForge2DialogueTreeDefinition FForge2DialogueUtils::CreateLinearDialogue(
	const FString& TreeId,
	const FString& SpeakerId,
	const TArray<FText>& Lines)
{
	FForge2DialogueTreeDefinition Tree;
	Tree.TreeId = TreeId;

	// Add speaker
	Tree.Speakers.Add(CreateSpeakerProfile(SpeakerId, FText::FromString(SpeakerId)));

	// Create nodes for each line
	for (int32 i = 0; i < Lines.Num(); i++)
	{
		FString NodeId = FString::Printf(TEXT("node_%d"), i);
		FString NextNodeId = (i < Lines.Num() - 1)
			? FString::Printf(TEXT("node_%d"), i + 1)
			: TEXT("");

		FForge2DialogueNode Node = CreateSpeechNode(NodeId, SpeakerId, Lines[i], NextNodeId);
		Tree.Nodes.Add(Node);
	}

	// Add end node
	FForge2DialogueNode EndNode;
	EndNode.NodeId = TEXT("end");
	EndNode.NodeType = EForge2DialogueNodeType::End;
	Tree.Nodes.Add(EndNode);

	if (Lines.Num() > 0)
	{
		Tree.Nodes.Last().NextNodeId = TEXT("end");
	}

	Tree.StartNodeId = TEXT("node_0");

	return Tree;
}

FForge2DialogueTreeDefinition FForge2DialogueUtils::CreateBranchingDialogue(
	const FString& TreeId,
	const FString& SpeakerId,
	const FText& Question,
	const TArray<FForge2DialogueChoice>& Choices)
{
	FForge2DialogueTreeDefinition Tree;
	Tree.TreeId = TreeId;

	// Add speaker
	Tree.Speakers.Add(CreateSpeakerProfile(SpeakerId, FText::FromString(SpeakerId)));

	// Create question node
	FForge2DialogueNode QuestionNode = CreateSpeechNode(TEXT("question"), SpeakerId, Question, TEXT("choices"));
	Tree.Nodes.Add(QuestionNode);

	// Create choice node
	FForge2DialogueNode ChoiceNode = CreateChoiceNode(TEXT("choices"), Choices);
	Tree.Nodes.Add(ChoiceNode);

	// Create end node
	FForge2DialogueNode EndNode;
	EndNode.NodeId = TEXT("end");
	EndNode.NodeType = EForge2DialogueNodeType::End;
	Tree.Nodes.Add(EndNode);

	Tree.StartNodeId = TEXT("question");

	return Tree;
}

FForge2DialogueTreeDefinition FForge2DialogueUtils::CreateYesNoDialogue(
	const FString& TreeId,
	const FString& SpeakerId,
	const FText& Question,
	const FText& YesResponse,
	const FText& NoResponse)
{
	TArray<FForge2DialogueChoice> Choices;

	FForge2DialogueChoice YesChoice;
	YesChoice.ChoiceId = TEXT("yes");
	YesChoice.ChoiceText = FText::FromString(TEXT("Yes"));
	YesChoice.TargetNodeId = TEXT("response_yes");
	YesChoice.DisplayOrder = 0;
	Choices.Add(YesChoice);

	FForge2DialogueChoice NoChoice;
	NoChoice.ChoiceId = TEXT("no");
	NoChoice.ChoiceText = FText::FromString(TEXT("No"));
	NoChoice.TargetNodeId = TEXT("response_no");
	NoChoice.DisplayOrder = 1;
	Choices.Add(NoChoice);

	FForge2DialogueTreeDefinition Tree = CreateBranchingDialogue(TreeId, SpeakerId, Question, Choices);

	// Add response nodes
	FForge2DialogueNode YesResponseNode = CreateSpeechNode(TEXT("response_yes"), SpeakerId, YesResponse, TEXT("end"));
	Tree.Nodes.Add(YesResponseNode);

	FForge2DialogueNode NoResponseNode = CreateSpeechNode(TEXT("response_no"), SpeakerId, NoResponse, TEXT("end"));
	Tree.Nodes.Add(NoResponseNode);

	return Tree;
}

FForge2DialogueTreeDefinition FForge2DialogueUtils::CreateShopDialogue(
	const FString& TreeId,
	const FString& ShopkeeperId,
	const FText& Greeting,
	const FString& ShopId)
{
	FForge2DialogueTreeDefinition Tree;
	Tree.TreeId = TreeId;

	Tree.Speakers.Add(CreateSpeakerProfile(ShopkeeperId, FText::FromString(ShopkeeperId)));

	// Greeting
	FForge2DialogueNode GreetingNode = CreateSpeechNode(TEXT("greeting"), ShopkeeperId, Greeting, TEXT("options"));
	Tree.Nodes.Add(GreetingNode);

	// Options
	TArray<FForge2DialogueChoice> Choices;

	FForge2DialogueChoice BuyChoice;
	BuyChoice.ChoiceId = TEXT("buy");
	BuyChoice.ChoiceText = FText::FromString(TEXT("Let me see what you have for sale."));
	BuyChoice.TargetNodeId = TEXT("open_shop");
	BuyChoice.DisplayOrder = 0;
	Choices.Add(BuyChoice);

	FForge2DialogueChoice LeaveChoice;
	LeaveChoice.ChoiceId = TEXT("leave");
	LeaveChoice.ChoiceText = FText::FromString(TEXT("Goodbye."));
	LeaveChoice.TargetNodeId = TEXT("farewell");
	LeaveChoice.DisplayOrder = 1;
	LeaveChoice.bIsExitOption = true;
	Choices.Add(LeaveChoice);

	FForge2DialogueNode OptionsNode = CreateChoiceNode(TEXT("options"), Choices);
	Tree.Nodes.Add(OptionsNode);

	// Open shop event
	FForge2DialogueNode ShopNode = CreateEventNode(TEXT("open_shop"), FString::Printf(TEXT("OpenShop_%s"), *ShopId), TEXT("end"));
	Tree.Nodes.Add(ShopNode);

	// Farewell
	FForge2DialogueNode FarewellNode = CreateSpeechNode(TEXT("farewell"), ShopkeeperId,
		FText::FromString(TEXT("Come back soon!")), TEXT("end"));
	Tree.Nodes.Add(FarewellNode);

	// End
	FForge2DialogueNode EndNode;
	EndNode.NodeId = TEXT("end");
	EndNode.NodeType = EForge2DialogueNodeType::End;
	Tree.Nodes.Add(EndNode);

	Tree.StartNodeId = TEXT("greeting");

	return Tree;
}

FForge2DialogueTreeDefinition FForge2DialogueUtils::CreateQuestDialogue(
	const FString& TreeId,
	const FString& QuestGiverId,
	const FText& QuestIntro,
	const FString& QuestId,
	const FText& AcceptResponse,
	const FText& DeclineResponse)
{
	FForge2DialogueTreeDefinition Tree;
	Tree.TreeId = TreeId;
	Tree.LinkedQuestId = QuestId;

	Tree.Speakers.Add(CreateSpeakerProfile(QuestGiverId, FText::FromString(QuestGiverId)));

	// Introduction
	FForge2DialogueNode IntroNode = CreateSpeechNode(TEXT("intro"), QuestGiverId, QuestIntro, TEXT("decision"));
	Tree.Nodes.Add(IntroNode);

	// Decision
	TArray<FForge2DialogueChoice> Choices;

	FForge2DialogueChoice AcceptChoice;
	AcceptChoice.ChoiceId = TEXT("accept");
	AcceptChoice.ChoiceText = FText::FromString(TEXT("I'll help you."));
	AcceptChoice.TargetNodeId = TEXT("accept_response");
	AcceptChoice.DisplayOrder = 0;

	FForge2DialogueConsequence StartQuestConsequence;
	StartQuestConsequence.ConsequenceType = EForge2DialogueConsequenceType::StartQuest;
	StartQuestConsequence.TargetId = QuestId;
	AcceptChoice.Consequences.Add(StartQuestConsequence);

	Choices.Add(AcceptChoice);

	FForge2DialogueChoice DeclineChoice;
	DeclineChoice.ChoiceId = TEXT("decline");
	DeclineChoice.ChoiceText = FText::FromString(TEXT("I can't help right now."));
	DeclineChoice.TargetNodeId = TEXT("decline_response");
	DeclineChoice.DisplayOrder = 1;
	Choices.Add(DeclineChoice);

	FForge2DialogueNode DecisionNode = CreateChoiceNode(TEXT("decision"), Choices);
	Tree.Nodes.Add(DecisionNode);

	// Accept response
	FForge2DialogueNode AcceptResponseNode = CreateSpeechNode(TEXT("accept_response"), QuestGiverId, AcceptResponse, TEXT("end"));
	Tree.Nodes.Add(AcceptResponseNode);

	// Decline response
	FForge2DialogueNode DeclineResponseNode = CreateSpeechNode(TEXT("decline_response"), QuestGiverId, DeclineResponse, TEXT("end"));
	Tree.Nodes.Add(DeclineResponseNode);

	// End
	FForge2DialogueNode EndNode;
	EndNode.NodeId = TEXT("end");
	EndNode.NodeType = EForge2DialogueNodeType::End;
	Tree.Nodes.Add(EndNode);

	Tree.StartNodeId = TEXT("intro");

	return Tree;
}

FForge2SpeakerProfile FForge2DialogueUtils::CreateSpeakerProfile(
	const FString& SpeakerId,
	const FText& DisplayName,
	bool bIsPlayer)
{
	FForge2SpeakerProfile Profile;
	Profile.SpeakerId = SpeakerId;
	Profile.DisplayName = DisplayName;
	Profile.bIsPlayer = bIsPlayer;
	return Profile;
}

FForge2DialogueNode FForge2DialogueUtils::CreateSpeechNode(
	const FString& NodeId,
	const FString& SpeakerId,
	const FText& DialogueText,
	const FString& NextNodeId)
{
	FForge2DialogueNode Node;
	Node.NodeId = NodeId;
	Node.NodeType = EForge2DialogueNodeType::Speech;
	Node.SpeakerId = SpeakerId;
	Node.DialogueText = DialogueText;
	Node.NextNodeId = NextNodeId;
	return Node;
}

FForge2DialogueNode FForge2DialogueUtils::CreateChoiceNode(
	const FString& NodeId,
	const TArray<FForge2DialogueChoice>& Choices)
{
	FForge2DialogueNode Node;
	Node.NodeId = NodeId;
	Node.NodeType = EForge2DialogueNodeType::PlayerChoice;
	Node.Choices = Choices;
	return Node;
}

FForge2DialogueNode FForge2DialogueUtils::CreateConditionNode(
	const FString& NodeId,
	const FForge2DialogueCondition& Condition,
	const FString& TrueNode,
	const FString& FalseNode)
{
	FForge2DialogueNode Node;
	Node.NodeId = NodeId;
	Node.NodeType = EForge2DialogueNodeType::Condition;
	Node.BranchConditions.Add(Condition);
	Node.BranchTargets.Add(TrueNode);
	Node.DefaultBranch = FalseNode;
	return Node;
}

FForge2DialogueNode FForge2DialogueUtils::CreateEventNode(
	const FString& NodeId,
	const FString& EventName,
	const FString& NextNodeId)
{
	FForge2DialogueNode Node;
	Node.NodeId = NodeId;
	Node.NodeType = EForge2DialogueNodeType::Event;
	Node.EventName = EventName;
	Node.NextNodeId = NextNodeId;
	return Node;
}

FString FForge2DialogueUtils::SerializeDialogueState(const FForge2DialogueRuntimeState& State)
{
	// Simple serialization
	FString Result;
	Result += State.TreeId + TEXT("|");
	Result += State.CurrentNodeId + TEXT("|");
	Result += State.bCompleted ? TEXT("1") : TEXT("0");
	Result += TEXT("|");
	Result += FString::SanitizeFloat(State.StartTime) + TEXT("|");
	Result += FString::SanitizeFloat(State.TotalTime) + TEXT("|");
	Result += FString::FromInt(State.CompletionCount) + TEXT("|");

	// Visited nodes
	for (const FString& NodeId : State.VisitedNodes)
	{
		Result += NodeId + TEXT(",");
	}
	Result += TEXT("|");

	// Selected choices
	for (const FString& ChoiceId : State.SelectedChoices)
	{
		Result += ChoiceId + TEXT(",");
	}

	return Result;
}

FForge2DialogueRuntimeState FForge2DialogueUtils::DeserializeDialogueState(const FString& Data)
{
	FForge2DialogueRuntimeState State;

	TArray<FString> Parts;
	Data.ParseIntoArray(Parts, TEXT("|"));

	if (Parts.Num() >= 6)
	{
		State.TreeId = Parts[0];
		State.CurrentNodeId = Parts[1];
		State.bCompleted = Parts[2] == TEXT("1");
		State.StartTime = FCString::Atof(*Parts[3]);
		State.TotalTime = FCString::Atof(*Parts[4]);
		State.CompletionCount = FCString::Atoi(*Parts[5]);

		if (Parts.Num() > 6)
		{
			TArray<FString> VisitedParts;
			Parts[6].ParseIntoArray(VisitedParts, TEXT(","));
			for (const FString& NodeId : VisitedParts)
			{
				if (!NodeId.IsEmpty())
				{
					State.VisitedNodes.Add(NodeId);
				}
			}
		}

		if (Parts.Num() > 7)
		{
			TArray<FString> ChoiceParts;
			Parts[7].ParseIntoArray(ChoiceParts, TEXT(","));
			for (const FString& ChoiceId : ChoiceParts)
			{
				if (!ChoiceId.IsEmpty())
				{
					State.SelectedChoices.Add(ChoiceId);
				}
			}
		}
	}

	return State;
}

FForge2DialogueTreeDefinition FForge2DialogueUtils::ParseDialogueScript(const FString& Script)
{
	// Simple script parser for dialogue format:
	// TREE: TreeId
	// SPEAKER: SpeakerId "Display Name"
	// NODE: NodeId
	//   TYPE: Speech
	//   SPEAKER: SpeakerId
	//   TEXT: "Dialogue text"
	//   NEXT: NextNodeId
	// ...

	FForge2DialogueTreeDefinition Tree;

	TArray<FString> Lines;
	Script.ParseIntoArrayLines(Lines);

	FForge2DialogueNode* CurrentNode = nullptr;

	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();
		if (TrimmedLine.IsEmpty() || TrimmedLine.StartsWith(TEXT("//")))
		{
			continue;
		}

		if (TrimmedLine.StartsWith(TEXT("TREE:")))
		{
			Tree.TreeId = TrimmedLine.Mid(5).TrimStartAndEnd();
		}
		else if (TrimmedLine.StartsWith(TEXT("SPEAKER:")))
		{
			FString SpeakerData = TrimmedLine.Mid(8).TrimStartAndEnd();
			// Parse "SpeakerId \"Display Name\""
			int32 QuoteStart = SpeakerData.Find(TEXT("\""));
			if (QuoteStart != INDEX_NONE)
			{
				FString SpeakerId = SpeakerData.Left(QuoteStart).TrimStartAndEnd();
				FString DisplayName = SpeakerData.Mid(QuoteStart + 1);
				DisplayName = DisplayName.LeftChop(1); // Remove trailing quote
				Tree.Speakers.Add(CreateSpeakerProfile(SpeakerId, FText::FromString(DisplayName)));
			}
		}
		else if (TrimmedLine.StartsWith(TEXT("NODE:")))
		{
			FForge2DialogueNode NewNode;
			NewNode.NodeId = TrimmedLine.Mid(5).TrimStartAndEnd();
			Tree.Nodes.Add(NewNode);
			CurrentNode = &Tree.Nodes.Last();
		}
		else if (CurrentNode)
		{
			if (TrimmedLine.StartsWith(TEXT("TYPE:")))
			{
				FString TypeStr = TrimmedLine.Mid(5).TrimStartAndEnd();
				if (TypeStr == TEXT("Speech")) CurrentNode->NodeType = EForge2DialogueNodeType::Speech;
				else if (TypeStr == TEXT("Choice")) CurrentNode->NodeType = EForge2DialogueNodeType::PlayerChoice;
				else if (TypeStr == TEXT("Condition")) CurrentNode->NodeType = EForge2DialogueNodeType::Condition;
				else if (TypeStr == TEXT("Event")) CurrentNode->NodeType = EForge2DialogueNodeType::Event;
				else if (TypeStr == TEXT("End")) CurrentNode->NodeType = EForge2DialogueNodeType::End;
			}
			else if (TrimmedLine.StartsWith(TEXT("SPEAKER:")))
			{
				CurrentNode->SpeakerId = TrimmedLine.Mid(8).TrimStartAndEnd();
			}
			else if (TrimmedLine.StartsWith(TEXT("TEXT:")))
			{
				FString Text = TrimmedLine.Mid(5).TrimStartAndEnd();
				if (Text.StartsWith(TEXT("\""))) Text = Text.Mid(1);
				if (Text.EndsWith(TEXT("\""))) Text = Text.LeftChop(1);
				CurrentNode->DialogueText = FText::FromString(Text);
			}
			else if (TrimmedLine.StartsWith(TEXT("NEXT:")))
			{
				CurrentNode->NextNodeId = TrimmedLine.Mid(5).TrimStartAndEnd();
			}
			else if (TrimmedLine.StartsWith(TEXT("START")))
			{
				Tree.StartNodeId = CurrentNode->NodeId;
			}
		}
	}

	if (Tree.StartNodeId.IsEmpty() && Tree.Nodes.Num() > 0)
	{
		Tree.StartNodeId = Tree.Nodes[0].NodeId;
	}

	return Tree;
}

FString FForge2DialogueUtils::ExportDialogueScript(const FForge2DialogueTreeDefinition& Tree)
{
	FString Script;

	Script += FString::Printf(TEXT("TREE: %s\n\n"), *Tree.TreeId);

	for (const FForge2SpeakerProfile& Speaker : Tree.Speakers)
	{
		Script += FString::Printf(TEXT("SPEAKER: %s \"%s\"\n"),
			*Speaker.SpeakerId, *Speaker.DisplayName.ToString());
	}
	Script += TEXT("\n");

	for (const FForge2DialogueNode& Node : Tree.Nodes)
	{
		Script += FString::Printf(TEXT("NODE: %s\n"), *Node.NodeId);

		if (Node.NodeId == Tree.StartNodeId)
		{
			Script += TEXT("  START\n");
		}

		FString TypeStr;
		switch (Node.NodeType)
		{
			case EForge2DialogueNodeType::Speech: TypeStr = TEXT("Speech"); break;
			case EForge2DialogueNodeType::PlayerChoice: TypeStr = TEXT("Choice"); break;
			case EForge2DialogueNodeType::Condition: TypeStr = TEXT("Condition"); break;
			case EForge2DialogueNodeType::Event: TypeStr = TEXT("Event"); break;
			case EForge2DialogueNodeType::End: TypeStr = TEXT("End"); break;
			default: TypeStr = TEXT("Unknown"); break;
		}
		Script += FString::Printf(TEXT("  TYPE: %s\n"), *TypeStr);

		if (!Node.SpeakerId.IsEmpty())
		{
			Script += FString::Printf(TEXT("  SPEAKER: %s\n"), *Node.SpeakerId);
		}

		if (!Node.DialogueText.IsEmpty())
		{
			Script += FString::Printf(TEXT("  TEXT: \"%s\"\n"), *Node.DialogueText.ToString());
		}

		if (!Node.NextNodeId.IsEmpty())
		{
			Script += FString::Printf(TEXT("  NEXT: %s\n"), *Node.NextNodeId);
		}

		Script += TEXT("\n");
	}

	return Script;
}
