// Forge2SkillSystem.cpp
// FORGE2 Skill System Implementation
// Part of the FORGE2 Ultimate AAA Game Development MCP Extension

#include "Forge2SkillSystem.h"

// ============================================================================
// FForge2SkillEffect Implementation
// ============================================================================

float FForge2SkillEffect::CalculateValue(int32 Rank, float StatValue) const
{
	float Value = BaseValue + (ValuePerRank * (Rank - 1));

	switch (ScalingType)
	{
		case EForge2SkillScalingType::Linear:
			Value += StatValue * ScalingCoefficient;
			break;

		case EForge2SkillScalingType::Percentage:
			Value *= (1.0f + (StatValue * ScalingCoefficient / 100.0f));
			break;

		case EForge2SkillScalingType::Exponential:
			Value *= FMath::Pow(ScalingCoefficient, StatValue / 100.0f);
			break;

		case EForge2SkillScalingType::Diminishing:
		{
			// Diminishing returns formula
			float Bonus = StatValue * ScalingCoefficient;
			Value += Bonus / (1.0f + Bonus / 100.0f);
			break;
		}

		case EForge2SkillScalingType::None:
		case EForge2SkillScalingType::Custom:
		default:
			break;
	}

	return Value;
}

// ============================================================================
// FForge2SkillCost Implementation
// ============================================================================

float FForge2SkillCost::CalculateCost(int32 Rank) const
{
	return FMath::Max(0.0f, BaseCost + (CostPerRank * (Rank - 1)));
}

// ============================================================================
// FForge2SkillDefinition Implementation
// ============================================================================

FText FForge2SkillDefinition::GetFormattedDescription(int32 Rank) const
{
	FString FormattedStr = Description.ToString();

	// Replace placeholders with calculated values
	for (int32 i = 0; i < Effects.Num(); i++)
	{
		const FForge2SkillEffect& Effect = Effects[i];
		float Value = Effect.CalculateValue(Rank);

		FString Placeholder = FString::Printf(TEXT("{%d}"), i);
		FString ValueStr = Effect.bPercentage
			? FString::Printf(TEXT("%.1f%%"), Value)
			: FString::Printf(TEXT("%.0f"), Value);

		FormattedStr = FormattedStr.Replace(*Placeholder, *ValueStr);
	}

	// Replace cooldown
	float CurrentCooldown = FMath::Max(0.0f, Cooldown - (CooldownPerRank * (Rank - 1)));
	FormattedStr = FormattedStr.Replace(TEXT("{cooldown}"), *FString::Printf(TEXT("%.1fs"), CurrentCooldown));

	// Replace range
	FormattedStr = FormattedStr.Replace(TEXT("{range}"), *FString::Printf(TEXT("%.0f"), Range));

	// Replace radius
	float CurrentRadius = Radius + (RadiusPerRank * (Rank - 1));
	FormattedStr = FormattedStr.Replace(TEXT("{radius}"), *FString::Printf(TEXT("%.0f"), CurrentRadius));

	return FText::FromString(FormattedStr);
}

int32 FForge2SkillDefinition::GetTotalPointCost() const
{
	return PointCostPerRank * MaxRanks;
}

// ============================================================================
// FForge2SkillTreeNode Implementation
// ============================================================================

bool FForge2SkillTreeNode::CanUnlock(const TMap<FString, int32>& AllocatedPoints, int32 TotalPointsSpent, const TMap<FString, int32>& GameState) const
{
	// Check required points in tree
	if (TotalPointsSpent < RequiredPointsInTree)
	{
		return false;
	}

	// Check prerequisites
	for (const FString& PrereqId : PrerequisiteNodes)
	{
		const int32* Points = AllocatedPoints.Find(PrereqId);
		const EForge2NodeConnectionType* ConnectionType = ConnectionTypes.Find(PrereqId);

		EForge2NodeConnectionType Type = ConnectionType ? *ConnectionType : EForge2NodeConnectionType::Required;

		switch (Type)
		{
			case EForge2NodeConnectionType::Required:
				if (!Points || *Points <= 0)
				{
					return false;
				}
				break;

			case EForge2NodeConnectionType::Optional:
				// Optional prerequisites don't block
				break;

			case EForge2NodeConnectionType::Exclusive:
				// Exclusive means we can't have this if the other is allocated
				if (Points && *Points > 0)
				{
					return false;
				}
				break;

			case EForge2NodeConnectionType::Linked:
				// Linked nodes must be unlocked together
				// This requires special handling elsewhere
				break;
		}
	}

	// Check custom condition
	if (!CustomUnlockCondition.IsEmpty())
	{
		FString Key = FString::Printf(TEXT("SkillUnlock_%s"), *CustomUnlockCondition);
		const int32* ConditionMet = GameState.Find(Key);
		if (!ConditionMet || *ConditionMet <= 0)
		{
			return false;
		}
	}

	return true;
}

// ============================================================================
// FForge2SkillTreeDefinition Implementation
// ============================================================================

FForge2SkillTreeNode* FForge2SkillTreeDefinition::GetNode(const FString& NodeId)
{
	for (FForge2SkillTreeNode& Node : Nodes)
	{
		if (Node.NodeId == NodeId)
		{
			return &Node;
		}
	}
	return nullptr;
}

const FForge2SkillTreeNode* FForge2SkillTreeDefinition::GetNode(const FString& NodeId) const
{
	for (const FForge2SkillTreeNode& Node : Nodes)
	{
		if (Node.NodeId == NodeId)
		{
			return &Node;
		}
	}
	return nullptr;
}

FForge2SkillDefinition* FForge2SkillTreeDefinition::GetSkill(const FString& SkillId)
{
	for (FForge2SkillDefinition& Skill : Skills)
	{
		if (Skill.SkillId == SkillId)
		{
			return &Skill;
		}
	}
	return nullptr;
}

const FForge2SkillDefinition* FForge2SkillTreeDefinition::GetSkill(const FString& SkillId) const
{
	for (const FForge2SkillDefinition& Skill : Skills)
	{
		if (Skill.SkillId == SkillId)
		{
			return &Skill;
		}
	}
	return nullptr;
}

bool FForge2SkillTreeDefinition::Validate(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();

	// Check each node
	for (const FForge2SkillTreeNode& Node : Nodes)
	{
		// Check skill reference
		if (!GetSkill(Node.SkillId))
		{
			OutErrors.Add(FString::Printf(TEXT("Node '%s' references non-existent skill '%s'"),
				*Node.NodeId, *Node.SkillId));
		}

		// Check prerequisite references
		for (const FString& PrereqId : Node.PrerequisiteNodes)
		{
			if (!GetNode(PrereqId))
			{
				OutErrors.Add(FString::Printf(TEXT("Node '%s' references non-existent prerequisite '%s'"),
					*Node.NodeId, *PrereqId));
			}
		}

		// Check tier
		if (Node.Tier < 0 || Node.Tier >= TierCount)
		{
			OutErrors.Add(FString::Printf(TEXT("Node '%s' has invalid tier %d (max: %d)"),
				*Node.NodeId, Node.Tier, TierCount - 1));
		}
	}

	return OutErrors.Num() == 0;
}

// ============================================================================
// FForge2ClassDefinition Implementation
// ============================================================================

float FForge2ClassDefinition::GetStatAtLevel(EForge2StatType Stat, int32 Level) const
{
	const float* BaseStat = BaseStats.Find(Stat);
	const float* Growth = StatGrowth.Find(Stat);

	float Base = BaseStat ? *BaseStat : 10.0f;
	float GrowthRate = Growth ? *Growth : 1.0f;

	return Base + (GrowthRate * (Level - 1));
}

// ============================================================================
// FForge2CharacterSkillProfile Implementation
// ============================================================================

int32 FForge2CharacterSkillProfile::GetTotalPointsSpent() const
{
	int32 Total = 0;
	for (const auto& Pair : SkillRanks)
	{
		Total += Pair.Value;
	}
	return Total;
}

int32 FForge2CharacterSkillProfile::GetPointsInTree(const FString& TreeId) const
{
	const int32* Points = PointsPerTree.Find(TreeId);
	return Points ? *Points : 0;
}

// ============================================================================
// FForge2SkillManager Implementation
// ============================================================================

bool FForge2SkillManager::RegisterSkill(const FForge2SkillDefinition& Skill)
{
	if (Skill.SkillId.IsEmpty())
	{
		return false;
	}

	SkillDefinitions.Add(Skill.SkillId, Skill);
	return true;
}

bool FForge2SkillManager::RegisterSkillTree(const FForge2SkillTreeDefinition& Tree)
{
	if (Tree.TreeId.IsEmpty())
	{
		return false;
	}

	TArray<FString> Errors;
	if (!Tree.Validate(Errors))
	{
		for (const FString& Error : Errors)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill tree '%s' validation: %s"), *Tree.TreeId, *Error);
		}
	}

	SkillTrees.Add(Tree.TreeId, Tree);

	// Also register all skills in the tree
	for (const FForge2SkillDefinition& Skill : Tree.Skills)
	{
		RegisterSkill(Skill);
	}

	return true;
}

bool FForge2SkillManager::RegisterClass(const FForge2ClassDefinition& Class)
{
	if (Class.ClassId.IsEmpty())
	{
		return false;
	}

	ClassDefinitions.Add(Class.ClassId, Class);
	return true;
}

bool FForge2SkillManager::RegisterSpecialization(const FForge2SpecializationDefinition& Spec)
{
	if (Spec.SpecId.IsEmpty())
	{
		return false;
	}

	Specializations.Add(Spec.SpecId, Spec);
	return true;
}

bool FForge2SkillManager::CanLearnSkill(const FForge2CharacterSkillProfile& Profile, const FString& SkillId, const TMap<FString, int32>& GameState) const
{
	// Check if we have points available
	if (Profile.AvailablePoints <= 0)
	{
		return false;
	}

	// Find skill definition
	const FForge2SkillDefinition* SkillDef = SkillDefinitions.Find(SkillId);
	if (!SkillDef)
	{
		return false;
	}

	// Check if already at max rank
	const int32* CurrentRank = Profile.SkillRanks.Find(SkillId);
	if (CurrentRank && *CurrentRank >= SkillDef->MaxRanks)
	{
		return false;
	}

	// Find which tree this skill is in
	for (const auto& TreePair : SkillTrees)
	{
		const FForge2SkillTreeNode* Node = nullptr;
		for (const FForge2SkillTreeNode& N : TreePair.Value.Nodes)
		{
			if (N.SkillId == SkillId)
			{
				Node = &N;
				break;
			}
		}

		if (Node)
		{
			// Check tree class restriction
			if (!TreePair.Value.RequiredClass.IsEmpty() &&
				TreePair.Value.RequiredClass != Profile.ClassId)
			{
				return false;
			}

			// Check node can be unlocked
			int32 PointsInTree = Profile.GetPointsInTree(TreePair.Key);
			if (!Node->CanUnlock(Profile.SkillRanks, PointsInTree, GameState))
			{
				return false;
			}

			break;
		}
	}

	return true;
}

bool FForge2SkillManager::LearnSkill(FForge2CharacterSkillProfile& Profile, const FString& SkillId)
{
	TMap<FString, int32> EmptyGameState;
	if (!CanLearnSkill(Profile, SkillId, EmptyGameState))
	{
		return false;
	}

	// Find skill definition
	const FForge2SkillDefinition* SkillDef = SkillDefinitions.Find(SkillId);
	if (!SkillDef)
	{
		return false;
	}

	// Allocate point
	int32& CurrentRank = Profile.SkillRanks.FindOrAdd(SkillId);
	CurrentRank++;

	// Deduct point cost
	Profile.AvailablePoints -= SkillDef->PointCostPerRank;

	// Find tree and update points in tree
	for (const auto& TreePair : SkillTrees)
	{
		for (const FForge2SkillTreeNode& Node : TreePair.Value.Nodes)
		{
			if (Node.SkillId == SkillId)
			{
				int32& TreePoints = Profile.PointsPerTree.FindOrAdd(TreePair.Key);
				TreePoints += SkillDef->PointCostPerRank;
				break;
			}
		}
	}

	// Create/update runtime state
	FForge2SkillRuntimeState& State = Profile.SkillStates.FindOrAdd(SkillId);
	State.SkillId = SkillId;
	State.CurrentRank = CurrentRank;
	State.bUnlocked = true;

	return true;
}

bool FForge2SkillManager::UnlearnSkill(FForge2CharacterSkillProfile& Profile, const FString& SkillId)
{
	int32* CurrentRank = Profile.SkillRanks.Find(SkillId);
	if (!CurrentRank || *CurrentRank <= 0)
	{
		return false;
	}

	const FForge2SkillDefinition* SkillDef = SkillDefinitions.Find(SkillId);
	if (!SkillDef)
	{
		return false;
	}

	// Refund point
	(*CurrentRank)--;
	Profile.AvailablePoints += SkillDef->PointCostPerRank;

	// Update tree points
	for (const auto& TreePair : SkillTrees)
	{
		for (const FForge2SkillTreeNode& Node : TreePair.Value.Nodes)
		{
			if (Node.SkillId == SkillId)
			{
				int32* TreePoints = Profile.PointsPerTree.Find(TreePair.Key);
				if (TreePoints)
				{
					*TreePoints = FMath::Max(0, *TreePoints - SkillDef->PointCostPerRank);
				}
				break;
			}
		}
	}

	// Update runtime state
	if (*CurrentRank <= 0)
	{
		FForge2SkillRuntimeState* State = Profile.SkillStates.Find(SkillId);
		if (State)
		{
			State->bUnlocked = false;
			State->CurrentRank = 0;
		}
	}
	else
	{
		FForge2SkillRuntimeState* State = Profile.SkillStates.Find(SkillId);
		if (State)
		{
			State->CurrentRank = *CurrentRank;
		}
	}

	return true;
}

bool FForge2SkillManager::RespecTree(FForge2CharacterSkillProfile& Profile, const FString& TreeId)
{
	const FForge2SkillTreeDefinition* Tree = SkillTrees.Find(TreeId);
	if (!Tree)
	{
		return false;
	}

	// Refund all points from this tree
	for (const FForge2SkillTreeNode& Node : Tree->Nodes)
	{
		const int32* Rank = Profile.SkillRanks.Find(Node.SkillId);
		if (Rank && *Rank > 0)
		{
			const FForge2SkillDefinition* SkillDef = Tree->GetSkill(Node.SkillId);
			if (SkillDef)
			{
				Profile.AvailablePoints += SkillDef->PointCostPerRank * (*Rank);
			}

			Profile.SkillRanks.Remove(Node.SkillId);

			FForge2SkillRuntimeState* State = Profile.SkillStates.Find(Node.SkillId);
			if (State)
			{
				State->bUnlocked = false;
				State->CurrentRank = 0;
			}
		}
	}

	Profile.PointsPerTree.Remove(TreeId);
	Profile.RespecsUsed++;

	return true;
}

bool FForge2SkillManager::RespecAll(FForge2CharacterSkillProfile& Profile)
{
	// Refund all points
	for (const auto& Pair : Profile.SkillRanks)
	{
		const FForge2SkillDefinition* SkillDef = SkillDefinitions.Find(Pair.Key);
		if (SkillDef)
		{
			Profile.AvailablePoints += SkillDef->PointCostPerRank * Pair.Value;
		}
	}

	Profile.SkillRanks.Empty();
	Profile.PointsPerTree.Empty();

	// Reset all skill states
	for (auto& Pair : Profile.SkillStates)
	{
		Pair.Value.bUnlocked = false;
		Pair.Value.CurrentRank = 0;
	}

	Profile.RespecsUsed++;

	return true;
}

int32 FForge2SkillManager::CalculateRespecCost(const FForge2CharacterSkillProfile& Profile) const
{
	if (Profile.RespecsUsed < FreeRespecsPerCharacter)
	{
		return 0;
	}

	int32 BaseCost = 100;
	int32 PointsSpent = Profile.GetTotalPointsSpent();

	return FMath::RoundToInt(BaseCost + (PointsSpent * 10 * RespecCostMultiplier));
}

bool FForge2SkillManager::CanUseSkill(const FForge2CharacterSkillProfile& Profile, const FString& SkillId, float CurrentResource, const TMap<FString, int32>& GameState) const
{
	// Check if skill is learned
	const FForge2SkillRuntimeState* State = Profile.SkillStates.Find(SkillId);
	if (!State || !State->bUnlocked)
	{
		return false;
	}

	// Check cooldown
	if (State->CooldownRemaining > 0.0f)
	{
		return false;
	}

	// Check resource cost
	const FForge2SkillDefinition* SkillDef = SkillDefinitions.Find(SkillId);
	if (!SkillDef)
	{
		return false;
	}

	for (const FForge2SkillCost& Cost : SkillDef->Costs)
	{
		float RequiredCost = Cost.CalculateCost(State->CurrentRank);
		if (CurrentResource < RequiredCost)
		{
			return false;
		}
	}

	return true;
}

bool FForge2SkillManager::UseSkill(FForge2CharacterSkillProfile& Profile, const FString& SkillId, float& CurrentResource)
{
	TMap<FString, int32> EmptyGameState;
	if (!CanUseSkill(Profile, SkillId, CurrentResource, EmptyGameState))
	{
		return false;
	}

	FForge2SkillRuntimeState* State = Profile.SkillStates.Find(SkillId);
	const FForge2SkillDefinition* SkillDef = SkillDefinitions.Find(SkillId);

	if (!State || !SkillDef)
	{
		return false;
	}

	// Deduct resource cost
	for (const FForge2SkillCost& Cost : SkillDef->Costs)
	{
		float RequiredCost = Cost.CalculateCost(State->CurrentRank);
		CurrentResource -= RequiredCost;
	}

	// Apply cooldown
	float Cooldown = FMath::Max(0.0f, SkillDef->Cooldown - (SkillDef->CooldownPerRank * (State->CurrentRank - 1)));
	State->CooldownRemaining = Cooldown;

	// Increment usage count
	State->TimesUsed++;

	// Handle toggles
	if (SkillDef->SkillType == EForge2SkillType::Toggle || SkillDef->SkillType == EForge2SkillType::Stance)
	{
		State->bActive = !State->bActive;
	}

	return true;
}

void FForge2SkillManager::TickCooldowns(FForge2CharacterSkillProfile& Profile, float DeltaTime)
{
	for (auto& Pair : Profile.SkillStates)
	{
		if (Pair.Value.CooldownRemaining > 0.0f)
		{
			Pair.Value.CooldownRemaining = FMath::Max(0.0f, Pair.Value.CooldownRemaining - DeltaTime);
		}
	}
}

float FForge2SkillManager::GetSkillValue(const FForge2CharacterSkillProfile& Profile, const FString& SkillId, int32 EffectIndex, float StatValue) const
{
	const FForge2SkillRuntimeState* State = Profile.SkillStates.Find(SkillId);
	const FForge2SkillDefinition* SkillDef = SkillDefinitions.Find(SkillId);

	if (!State || !SkillDef || EffectIndex < 0 || EffectIndex >= SkillDef->Effects.Num())
	{
		return 0.0f;
	}

	return SkillDef->Effects[EffectIndex].CalculateValue(State->CurrentRank, StatValue);
}

TArray<FString> FForge2SkillManager::GetAvailableSkills(const FForge2CharacterSkillProfile& Profile) const
{
	TArray<FString> Result;
	TMap<FString, int32> EmptyGameState;

	for (const auto& Pair : SkillDefinitions)
	{
		if (CanLearnSkill(Profile, Pair.Key, EmptyGameState))
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

TArray<FString> FForge2SkillManager::GetLearnedSkills(const FForge2CharacterSkillProfile& Profile) const
{
	TArray<FString> Result;

	for (const auto& Pair : Profile.SkillStates)
	{
		if (Pair.Value.bUnlocked)
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

bool FForge2SkillManager::AssignToHotbar(FForge2CharacterSkillProfile& Profile, int32 Slot, const FString& SkillId)
{
	if (Slot < 0)
	{
		return false;
	}

	// Check if skill is active type (can't hotbar passives)
	const FForge2SkillDefinition* SkillDef = SkillDefinitions.Find(SkillId);
	if (SkillDef && SkillDef->SkillType == EForge2SkillType::Passive)
	{
		return false;
	}

	// Check if skill is learned
	const FForge2SkillRuntimeState* State = Profile.SkillStates.Find(SkillId);
	if (!State || !State->bUnlocked)
	{
		return false;
	}

	Profile.HotbarSlots.Add(Slot, SkillId);
	return true;
}

bool FForge2SkillManager::ChooseSpecialization(FForge2CharacterSkillProfile& Profile, const FString& SpecId, const TMap<FString, int32>& GameState)
{
	const FForge2SpecializationDefinition* Spec = Specializations.Find(SpecId);
	if (!Spec)
	{
		return false;
	}

	// Check parent class matches
	if (Spec->ParentClassId != Profile.ClassId)
	{
		return false;
	}

	// Check if already specialized
	if (!Profile.SpecializationId.IsEmpty())
	{
		return false;
	}

	// Check class has this specialization available
	const FForge2ClassDefinition* ClassDef = ClassDefinitions.Find(Profile.ClassId);
	if (!ClassDef || !ClassDef->SpecializationIds.Contains(SpecId))
	{
		return false;
	}

	// Check level requirement
	const int32* PlayerLevel = GameState.Find(TEXT("PlayerLevel"));
	if (!PlayerLevel || *PlayerLevel < ClassDef->SpecializationLevel)
	{
		return false;
	}

	Profile.SpecializationId = SpecId;
	return true;
}

// ============================================================================
// FForge2SkillUtils Implementation
// ============================================================================

FForge2SkillDefinition FForge2SkillUtils::CreatePassiveSkill(
	const FString& SkillId,
	const FText& SkillName,
	EForge2StatType AffectedStat,
	float BaseBonus,
	float BonusPerRank,
	bool bPercentage)
{
	FForge2SkillDefinition Skill;
	Skill.SkillId = SkillId;
	Skill.SkillName = SkillName;
	Skill.SkillType = EForge2SkillType::Passive;
	Skill.TargetType = EForge2SkillTargetType::Self;
	Skill.MaxRanks = 5;

	FForge2SkillEffect Effect;
	Effect.AffectedStat = AffectedStat;
	Effect.BaseValue = BaseBonus;
	Effect.ValuePerRank = BonusPerRank;
	Effect.bPercentage = bPercentage;

	Skill.Effects.Add(Effect);

	Skill.Description = FText::FromString(FString::Printf(TEXT("Increases %s by {0}"),
		*UEnum::GetValueAsString(AffectedStat)));

	return Skill;
}

FForge2SkillDefinition FForge2SkillUtils::CreateDamageSkill(
	const FString& SkillId,
	const FText& SkillName,
	float BaseDamage,
	float DamagePerRank,
	EForge2SkillResourceType ResourceType,
	float ResourceCost,
	float Cooldown)
{
	FForge2SkillDefinition Skill;
	Skill.SkillId = SkillId;
	Skill.SkillName = SkillName;
	Skill.SkillType = EForge2SkillType::Active;
	Skill.TargetType = EForge2SkillTargetType::SingleEnemy;
	Skill.MaxRanks = 5;
	Skill.Cooldown = Cooldown;

	FForge2SkillCost Cost;
	Cost.ResourceType = ResourceType;
	Cost.BaseCost = ResourceCost;
	Skill.Costs.Add(Cost);

	FForge2SkillEffect Effect;
	Effect.EffectId = TEXT("damage");
	Effect.BaseValue = BaseDamage;
	Effect.ValuePerRank = DamagePerRank;
	Skill.Effects.Add(Effect);

	Skill.Description = FText::FromString(TEXT("Deals {0} damage to the target."));

	return Skill;
}

FForge2SkillDefinition FForge2SkillUtils::CreateHealSkill(
	const FString& SkillId,
	const FText& SkillName,
	float BaseHeal,
	float HealPerRank,
	EForge2SkillResourceType ResourceType,
	float ResourceCost,
	float Cooldown)
{
	FForge2SkillDefinition Skill;
	Skill.SkillId = SkillId;
	Skill.SkillName = SkillName;
	Skill.SkillType = EForge2SkillType::Active;
	Skill.TargetType = EForge2SkillTargetType::SingleAlly;
	Skill.MaxRanks = 5;
	Skill.Cooldown = Cooldown;

	FForge2SkillCost Cost;
	Cost.ResourceType = ResourceType;
	Cost.BaseCost = ResourceCost;
	Skill.Costs.Add(Cost);

	FForge2SkillEffect Effect;
	Effect.EffectId = TEXT("heal");
	Effect.BaseValue = BaseHeal;
	Effect.ValuePerRank = HealPerRank;
	Skill.Effects.Add(Effect);

	Skill.Description = FText::FromString(TEXT("Heals the target for {0} health."));

	return Skill;
}

FForge2SkillDefinition FForge2SkillUtils::CreateBuffSkill(
	const FString& SkillId,
	const FText& SkillName,
	EForge2StatType BuffedStat,
	float BaseBonus,
	float BonusPerRank,
	float Duration,
	EForge2SkillResourceType ResourceType,
	float ResourceCost)
{
	FForge2SkillDefinition Skill;
	Skill.SkillId = SkillId;
	Skill.SkillName = SkillName;
	Skill.SkillType = EForge2SkillType::Active;
	Skill.TargetType = EForge2SkillTargetType::Self;
	Skill.MaxRanks = 5;

	FForge2SkillCost Cost;
	Cost.ResourceType = ResourceType;
	Cost.BaseCost = ResourceCost;
	Skill.Costs.Add(Cost);

	FForge2SkillEffect Effect;
	Effect.AffectedStat = BuffedStat;
	Effect.BaseValue = BaseBonus;
	Effect.ValuePerRank = BonusPerRank;
	Effect.Duration = Duration;
	Skill.Effects.Add(Effect);

	Skill.Description = FText::FromString(FString::Printf(TEXT("Increases %s by {0} for %.1f seconds."),
		*UEnum::GetValueAsString(BuffedStat), Duration));

	return Skill;
}

FForge2SkillTreeDefinition FForge2SkillUtils::CreateLinearTree(
	const FString& TreeId,
	const FText& TreeName,
	const TArray<FForge2SkillDefinition>& Skills)
{
	FForge2SkillTreeDefinition Tree;
	Tree.TreeId = TreeId;
	Tree.TreeName = TreeName;
	Tree.Skills = Skills;
	Tree.TierCount = Skills.Num();

	for (int32 i = 0; i < Skills.Num(); i++)
	{
		FForge2SkillTreeNode Node;
		Node.NodeId = FString::Printf(TEXT("node_%d"), i);
		Node.SkillId = Skills[i].SkillId;
		Node.Position = FVector2D(100.0f, i * 100.0f);
		Node.Tier = i;

		if (i > 0)
		{
			Node.PrerequisiteNodes.Add(FString::Printf(TEXT("node_%d"), i - 1));
		}

		Tree.Nodes.Add(Node);
	}

	return Tree;
}

FForge2ClassDefinition FForge2SkillUtils::CreateClass(
	const FString& ClassId,
	const FText& ClassName,
	EForge2ClassRole Role,
	const TMap<EForge2StatType, float>& BaseStats,
	const TArray<FString>& SkillTreeIds)
{
	FForge2ClassDefinition Class;
	Class.ClassId = ClassId;
	Class.ClassName = ClassName;
	Class.Role = Role;
	Class.BaseStats = BaseStats;
	Class.SkillTreeIds = SkillTreeIds;

	// Default growth rates
	Class.StatGrowth.Add(EForge2StatType::Strength, 2.0f);
	Class.StatGrowth.Add(EForge2StatType::Dexterity, 2.0f);
	Class.StatGrowth.Add(EForge2StatType::Intelligence, 2.0f);
	Class.StatGrowth.Add(EForge2StatType::Constitution, 3.0f);
	Class.StatGrowth.Add(EForge2StatType::Wisdom, 1.5f);
	Class.StatGrowth.Add(EForge2StatType::Charisma, 1.0f);

	return Class;
}

FString FForge2SkillUtils::SerializeSkillProfile(const FForge2CharacterSkillProfile& Profile)
{
	// Simple serialization
	FString Result;
	Result += Profile.ClassId + TEXT("|");
	Result += Profile.SpecializationId + TEXT("|");
	Result += FString::FromInt(Profile.AvailablePoints) + TEXT("|");
	Result += FString::FromInt(Profile.RespecsUsed) + TEXT("|");

	// Skill ranks
	for (const auto& Pair : Profile.SkillRanks)
	{
		Result += Pair.Key + TEXT(":") + FString::FromInt(Pair.Value) + TEXT(",");
	}
	Result += TEXT("|");

	// Hotbar
	for (const auto& Pair : Profile.HotbarSlots)
	{
		Result += FString::FromInt(Pair.Key) + TEXT(":") + Pair.Value + TEXT(",");
	}

	return Result;
}

FForge2CharacterSkillProfile FForge2SkillUtils::DeserializeSkillProfile(const FString& Data)
{
	FForge2CharacterSkillProfile Profile;

	TArray<FString> Parts;
	Data.ParseIntoArray(Parts, TEXT("|"));

	if (Parts.Num() >= 4)
	{
		Profile.ClassId = Parts[0];
		Profile.SpecializationId = Parts[1];
		Profile.AvailablePoints = FCString::Atoi(*Parts[2]);
		Profile.RespecsUsed = FCString::Atoi(*Parts[3]);

		// Skill ranks
		if (Parts.Num() > 4)
		{
			TArray<FString> RankParts;
			Parts[4].ParseIntoArray(RankParts, TEXT(","));
			for (const FString& RankPart : RankParts)
			{
				TArray<FString> KeyValue;
				RankPart.ParseIntoArray(KeyValue, TEXT(":"));
				if (KeyValue.Num() == 2)
				{
					Profile.SkillRanks.Add(KeyValue[0], FCString::Atoi(*KeyValue[1]));
				}
			}
		}

		// Hotbar
		if (Parts.Num() > 5)
		{
			TArray<FString> HotbarParts;
			Parts[5].ParseIntoArray(HotbarParts, TEXT(","));
			for (const FString& HotbarPart : HotbarParts)
			{
				TArray<FString> KeyValue;
				HotbarPart.ParseIntoArray(KeyValue, TEXT(":"));
				if (KeyValue.Num() == 2)
				{
					Profile.HotbarSlots.Add(FCString::Atoi(*KeyValue[0]), KeyValue[1]);
				}
			}
		}
	}

	return Profile;
}
