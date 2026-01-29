// Copyright 2025 FORGE2 Project. All Rights Reserved.
// Fighting Genre System Implementation

#include "Forge2FightingSystem.h"

// ============================================================================
// MOVE INPUT
// ============================================================================

FString FForge2MoveInput::GetInputDisplay() const
{
	FString Result;

	// Add motion input
	switch (MotionInput)
	{
	case EForge2MotionInput::QCF: Result = TEXT("236"); break;
	case EForge2MotionInput::QCB: Result = TEXT("214"); break;
	case EForge2MotionInput::HCF: Result = TEXT("41236"); break;
	case EForge2MotionInput::HCB: Result = TEXT("63214"); break;
	case EForge2MotionInput::DP: Result = TEXT("623"); break;
	case EForge2MotionInput::RDP: Result = TEXT("421"); break;
	case EForge2MotionInput::Charge_Back_Forward: Result = TEXT("[4]6"); break;
	case EForge2MotionInput::Charge_Down_Up: Result = TEXT("[2]8"); break;
	case EForge2MotionInput::Full_Circle: Result = TEXT("360"); break;
	case EForge2MotionInput::Double_QCF: Result = TEXT("236236"); break;
	case EForge2MotionInput::Double_QCB: Result = TEXT("214214"); break;
	case EForge2MotionInput::SPD: Result = TEXT("720"); break;
	case EForge2MotionInput::Pretzel: Result = TEXT("1632143"); break;
	default: break;
	}

	// Add direction modifier
	if (!DirectionRequired.IsNearlyZero())
	{
		if (DirectionRequired.X > 0) Result += TEXT("6");
		else if (DirectionRequired.X < 0) Result += TEXT("4");
		if (DirectionRequired.Y > 0) Result += TEXT("8");
		else if (DirectionRequired.Y < 0) Result += TEXT("2");
	}

	// Add buttons
	for (EForge2InputButton Button : Buttons)
	{
		switch (Button)
		{
		case EForge2InputButton::Light_Punch: Result += TEXT("LP"); break;
		case EForge2InputButton::Medium_Punch: Result += TEXT("MP"); break;
		case EForge2InputButton::Heavy_Punch: Result += TEXT("HP"); break;
		case EForge2InputButton::Light_Kick: Result += TEXT("LK"); break;
		case EForge2InputButton::Medium_Kick: Result += TEXT("MK"); break;
		case EForge2InputButton::Heavy_Kick: Result += TEXT("HK"); break;
		case EForge2InputButton::Punch: Result += TEXT("P"); break;
		case EForge2InputButton::Kick: Result += TEXT("K"); break;
		case EForge2InputButton::Slash: Result += TEXT("S"); break;
		case EForge2InputButton::Heavy_Slash: Result += TEXT("HS"); break;
		case EForge2InputButton::Dust: Result += TEXT("D"); break;
		default: break;
		}
	}

	return Result;
}

// ============================================================================
// MOVE DEFINITION
// ============================================================================

int32 FForge2MoveDefinition::CalculateComboScaledDamage(int32 HitCount, float DamageScaling) const
{
	return FMath::Max(1, FMath::RoundToInt(BaseDamage * DamageScaling));
}

// ============================================================================
// COMBO STATE
// ============================================================================

void FForge2ComboState::Reset()
{
	HitCount = 0;
	TotalDamage = 0;
	DamageScaling = 1.0f;
	HitstunDecay = 1.0f;
	GravityScaling = 1.0f;
	JuggleCount = 0;
	MovesUsed.Empty();
	bValidCombo = true;
	bCounterHitStarter = false;
}

void FForge2ComboState::AddHit(const FForge2MoveDefinition& Move, bool bCounterHit)
{
	if (HitCount == 0 && bCounterHit)
	{
		bCounterHitStarter = true;
	}

	HitCount++;
	TotalDamage += Move.CalculateComboScaledDamage(HitCount, DamageScaling);
	MovesUsed.Add(Move.MoveId);

	// Apply scaling
	DamageScaling = FMath::Max(0.1f, DamageScaling - 0.1f);
	HitstunDecay = FMath::Max(0.3f, HitstunDecay - 0.05f);

	// Track juggles
	if (Move.FrameData.bAirborne)
	{
		JuggleCount++;
		GravityScaling = FMath::Min(2.0f, GravityScaling + 0.05f);
	}
}

bool FForge2ComboState::CanUseMove(const FString& MoveId, int32 MaxJuggles) const
{
	// Check juggle limit
	if (JuggleCount >= MaxJuggles)
	{
		return false;
	}

	// Some games restrict move repetition
	// Uncomment to enable:
	// if (MovesUsed.Contains(MoveId)) return false;

	return true;
}

// ============================================================================
// METER STATE
// ============================================================================

void FForge2MeterState::AddMeter(int32 Amount, const FForge2MeterDefinition& Definition)
{
	int32 TotalAmount = CurrentValue + Amount;

	// Convert to stocks if applicable
	while (TotalAmount >= Definition.ValuePerStock && CurrentStocks < Definition.MaxStocks)
	{
		CurrentStocks++;
		TotalAmount -= Definition.ValuePerStock;
	}

	// Handle overflow
	if (CurrentStocks >= Definition.MaxStocks)
	{
		CurrentValue = Definition.bCanOverflow ?
			FMath::Min(TotalAmount, Definition.MaxValue) : 0;
	}
	else
	{
		CurrentValue = TotalAmount;
	}
}

bool FForge2MeterState::SpendMeter(int32 Amount, const FForge2MeterDefinition& Definition)
{
	if (!CanAfford(Amount, Definition))
	{
		return false;
	}

	int32 TotalValue = GetTotalValue(Definition);
	TotalValue -= Amount;

	// Recalculate stocks and current
	CurrentStocks = TotalValue / Definition.ValuePerStock;
	CurrentValue = TotalValue % Definition.ValuePerStock;

	return true;
}

bool FForge2MeterState::CanAfford(int32 Amount, const FForge2MeterDefinition& Definition) const
{
	return GetTotalValue(Definition) >= Amount;
}

int32 FForge2MeterState::GetTotalValue(const FForge2MeterDefinition& Definition) const
{
	return (CurrentStocks * Definition.ValuePerStock) + CurrentValue;
}

// ============================================================================
// FIGHTER DEFINITION
// ============================================================================

const FForge2MoveDefinition* FForge2FighterDefinition::GetMove(const FString& MoveId) const
{
	for (const FForge2MoveDefinition& Move : Moveset)
	{
		if (Move.MoveId == MoveId)
		{
			return &Move;
		}
	}
	return nullptr;
}

TArray<const FForge2MoveDefinition*> FForge2FighterDefinition::GetMovesOfType(EForge2MoveType Type) const
{
	TArray<const FForge2MoveDefinition*> Result;

	for (const FForge2MoveDefinition& Move : Moveset)
	{
		if (Move.MoveType == Type)
		{
			Result.Add(&Move);
		}
	}

	return Result;
}

// ============================================================================
// STAGE DEFINITION
// ============================================================================

const FForge2StageWall* FForge2StageDefinition::GetWallAtPosition(float XPosition) const
{
	float HalfWidth = StageWidth / 2.0f;

	for (const FForge2StageWall& Wall : Walls)
	{
		if (FMath::Abs(Wall.WallPosition - XPosition) < 50.0f ||
			FMath::Abs(Wall.WallPosition + XPosition) < 50.0f)
		{
			return &Wall;
		}
	}

	return nullptr;
}

// ============================================================================
// FIGHTER RUNTIME STATE
// ============================================================================

void FForge2FighterRuntimeState::ResetForRound(const FForge2FighterDefinition& Definition)
{
	CurrentHealth = Definition.Stats.MaxHealth;
	CurrentStun = 0;
	bIsStunned = false;
	CurrentState = EForge2FighterState::Standing;
	CurrentMoveId.Empty();
	CurrentMoveFrame = 0;
	RemainingAirDashes = Definition.Stats.NumAirDashes;
	bHasDoubleJumped = false;
	GuardGauge = Definition.Stats.MaxGuardGauge;
	ComboState.Reset();
	InputBuffer.Empty();

	// Reset meters based on definition
	MeterStates.Empty();
	for (const FForge2MeterDefinition& MeterDef : Definition.Meters)
	{
		FForge2MeterState State;
		State.CurrentValue = MeterDef.StartingValue % MeterDef.ValuePerStock;
		State.CurrentStocks = MeterDef.StartingValue / MeterDef.ValuePerStock;
		MeterStates.Add(State);
	}
}

// ============================================================================
// MATCH STATE
// ============================================================================

bool FForge2MatchState::IsMatchOver(int32 RoundsToWin) const
{
	return Player1Wins >= RoundsToWin || Player2Wins >= RoundsToWin;
}

int32 FForge2MatchState::GetMatchWinner(int32 RoundsToWin) const
{
	if (Player1Wins >= RoundsToWin) return 1;
	if (Player2Wins >= RoundsToWin) return 2;
	return 0;
}

// ============================================================================
// FIGHTING MANAGER
// ============================================================================

bool FForge2FightingManager::RegisterFighter(const FForge2FighterDefinition& Fighter)
{
	if (Fighter.FighterId.IsEmpty())
	{
		return false;
	}

	Fighters.Add(Fighter.FighterId, Fighter);
	return true;
}

FForge2FighterDefinition* FForge2FightingManager::GetFighter(const FString& FighterId)
{
	return Fighters.Find(FighterId);
}

TArray<FString> FForge2FightingManager::GetAllFighterIds() const
{
	TArray<FString> Ids;
	Fighters.GetKeys(Ids);
	return Ids;
}

bool FForge2FightingManager::RegisterStage(const FForge2StageDefinition& Stage)
{
	if (Stage.StageId.IsEmpty())
	{
		return false;
	}

	Stages.Add(Stage.StageId, Stage);
	return true;
}

FForge2StageDefinition* FForge2FightingManager::GetStage(const FString& StageId)
{
	return Stages.Find(StageId);
}

TArray<FString> FForge2FightingManager::GetAllStageIds() const
{
	TArray<FString> Ids;
	Stages.GetKeys(Ids);
	return Ids;
}

bool FForge2FightingManager::StartMatch(const FString& Fighter1Id, const FString& Fighter2Id, const FString& StageId, const FForge2MatchConfiguration& Config)
{
	if (!Fighters.Contains(Fighter1Id) || !Fighters.Contains(Fighter2Id) || !Stages.Contains(StageId))
	{
		return false;
	}

	bMatchActive = true;
	ActiveFighter1Id = Fighter1Id;
	ActiveFighter2Id = Fighter2Id;
	ActiveStageId = StageId;
	MatchConfig = Config;

	// Initialize match state
	MatchState.Player1Wins = 0;
	MatchState.Player2Wins = 0;
	MatchState.CurrentRound = 1;
	MatchState.bRoundActive = false;

	// Initialize fighter states
	const FForge2FighterDefinition* Fighter1 = Fighters.Find(Fighter1Id);
	const FForge2FighterDefinition* Fighter2 = Fighters.Find(Fighter2Id);
	const FForge2StageDefinition* Stage = Stages.Find(StageId);

	if (Fighter1 && Fighter2 && Stage)
	{
		MatchState.Player1State.ResetForRound(*Fighter1);
		MatchState.Player2State.ResetForRound(*Fighter2);

		// Set starting positions
		MatchState.Player1State.Position = FVector(-Stage->StartingDistance / 2, 0, Stage->FloorHeight);
		MatchState.Player1State.bFacingRight = true;

		MatchState.Player2State.Position = FVector(Stage->StartingDistance / 2, 0, Stage->FloorHeight);
		MatchState.Player2State.bFacingRight = false;
	}

	return true;
}

void FForge2FightingManager::EndMatch()
{
	bMatchActive = false;
	MatchState.bRoundActive = false;
}

void FForge2FightingManager::StartRound()
{
	if (!bMatchActive)
	{
		return;
	}

	const FForge2FighterDefinition* Fighter1 = Fighters.Find(ActiveFighter1Id);
	const FForge2FighterDefinition* Fighter2 = Fighters.Find(ActiveFighter2Id);
	const FForge2StageDefinition* Stage = Stages.Find(ActiveStageId);

	if (!Fighter1 || !Fighter2 || !Stage)
	{
		return;
	}

	// Reset fighter states for new round
	MatchState.Player1State.ResetForRound(*Fighter1);
	MatchState.Player2State.ResetForRound(*Fighter2);

	// Set starting positions
	MatchState.Player1State.Position = FVector(-Stage->StartingDistance / 2, 0, Stage->FloorHeight);
	MatchState.Player1State.bFacingRight = true;

	MatchState.Player2State.Position = FVector(Stage->StartingDistance / 2, 0, Stage->FloorHeight);
	MatchState.Player2State.bFacingRight = false;

	// Reset timer
	MatchState.RoundTimeRemaining = (float)MatchConfig.RoundTimeLimit;
	MatchState.bRoundActive = true;
}

void FForge2FightingManager::EndRound(int32 Winner)
{
	MatchState.bRoundActive = false;

	if (Winner == 1)
	{
		MatchState.Player1Wins++;
	}
	else if (Winner == 2)
	{
		MatchState.Player2Wins++;
	}
	// Winner == 0 is a draw, no wins awarded

	MatchState.CurrentRound++;
}

bool FForge2FightingManager::IsRoundActive() const
{
	return bMatchActive && MatchState.bRoundActive;
}

void FForge2FightingManager::ProcessInput(int32 PlayerIndex, int32 InputFlags)
{
	if (!IsRoundActive())
	{
		return;
	}

	FForge2FighterRuntimeState* State = (PlayerIndex == 0) ?
		&MatchState.Player1State : &MatchState.Player2State;

	// Add to input buffer
	State->InputBuffer.Add(InputFlags);

	// Keep buffer to last 60 frames (1 second at 60fps)
	if (State->InputBuffer.Num() > 60)
	{
		State->InputBuffer.RemoveAt(0);
	}

	// Check for move execution
	const FForge2FighterDefinition* Fighter = (PlayerIndex == 0) ?
		Fighters.Find(ActiveFighter1Id) : Fighters.Find(ActiveFighter2Id);

	if (Fighter && State->CurrentMoveId.IsEmpty())
	{
		FString DetectedMove = CheckInputForMove(State->InputBuffer, *Fighter, State->CurrentState);
		if (!DetectedMove.IsEmpty())
		{
			ProcessMove(PlayerIndex, DetectedMove);
		}
	}
}

void FForge2FightingManager::Update(float DeltaTime)
{
	if (!IsRoundActive())
	{
		return;
	}

	// Update timer
	if (MatchConfig.RoundTimeLimit > 0)
	{
		MatchState.RoundTimeRemaining -= DeltaTime;
	}

	// Get fighters
	const FForge2FighterDefinition* Fighter1 = Fighters.Find(ActiveFighter1Id);
	const FForge2FighterDefinition* Fighter2 = Fighters.Find(ActiveFighter2Id);

	if (Fighter1)
	{
		UpdateFighterState(MatchState.Player1State, *Fighter1, DeltaTime);
	}

	if (Fighter2)
	{
		UpdateFighterState(MatchState.Player2State, *Fighter2, DeltaTime);
	}

	// Training mode overrides
	if (bTrainingMode)
	{
		if (TrainingSettings.bInfiniteHealth)
		{
			if (Fighter1) MatchState.Player1State.CurrentHealth = Fighter1->Stats.MaxHealth;
			if (Fighter2) MatchState.Player2State.CurrentHealth = Fighter2->Stats.MaxHealth;
		}

		if (TrainingSettings.bInfiniteMeter && Fighter1 && Fighter2)
		{
			for (int32 i = 0; i < MatchState.Player1State.MeterStates.Num(); ++i)
			{
				if (i < Fighter1->Meters.Num())
				{
					MatchState.Player1State.MeterStates[i].CurrentStocks = Fighter1->Meters[i].MaxStocks;
					MatchState.Player1State.MeterStates[i].CurrentValue = 0;
				}
			}
		}
	}

	CheckRoundEnd();
}

FString FForge2FightingManager::CheckInputForMove(const TArray<int32>& InputBuffer, const FForge2FighterDefinition& Fighter, EForge2FighterState CurrentState) const
{
	// Check moves in priority order (supers > specials > normals)
	// This is a simplified version - real fighting games have more complex priority

	// Check from most recent inputs
	for (const FForge2MoveDefinition& Move : Fighter.Moveset)
	{
		if (!Move.bEnabled)
		{
			continue;
		}

		// Check if move can be done in current state
		bool bCanDoMove = true;
		bool bInAir = (CurrentState == EForge2FighterState::Jumping ||
					   CurrentState == EForge2FighterState::Air_Rising ||
					   CurrentState == EForge2FighterState::Air_Falling);

		if (bInAir && !Move.Input.bCanDoInAir)
		{
			bCanDoMove = false;
		}

		if (!bCanDoMove)
		{
			continue;
		}

		// Check motion input
		EForge2MotionInput DetectedMotion = FForge2FightingUtils::ParseMotionInput(InputBuffer);

		if (Move.Input.MotionInput != EForge2MotionInput::None)
		{
			if (DetectedMotion != Move.Input.MotionInput)
			{
				continue;
			}
		}

		// Check button (simplified - just check if last input has required buttons)
		if (Move.Input.Buttons.Num() > 0 && InputBuffer.Num() > 0)
		{
			int32 LastInput = InputBuffer.Last();
			bool bHasButtons = true;

			for (EForge2InputButton RequiredButton : Move.Input.Buttons)
			{
				// Simplified button check - in real implementation, map button enum to input flags
				int32 ButtonFlag = 1 << (int32)RequiredButton;
				if ((LastInput & ButtonFlag) == 0)
				{
					bHasButtons = false;
					break;
				}
			}

			if (!bHasButtons)
			{
				continue;
			}
		}

		// Move detected
		return Move.MoveId;
	}

	return FString();
}

int32 FForge2FightingManager::CalculateDamage(const FForge2MoveDefinition& Move, const FForge2ComboState& ComboState, bool bCounterHit, float DefenseModifier) const
{
	float Damage = (float)Move.BaseDamage;

	// Apply combo scaling
	if (MatchConfig.bDamageScaling && ComboState.HitCount > 0)
	{
		Damage *= ComboState.DamageScaling;
	}

	// Counter hit bonus
	if (bCounterHit)
	{
		Damage *= 1.2f;
	}

	// Defense modifier
	Damage *= DefenseModifier;

	return FMath::Max(1, FMath::RoundToInt(Damage));
}

bool FForge2FightingManager::CheckHitboxCollision(const FForge2HitboxData& Hitbox, const FForge2HurtboxData& Hurtbox, const FVector& AttackerPos, const FVector& DefenderPos, bool bAttackerFacingRight) const
{
	// Transform hitbox offset based on facing direction
	FVector HitboxOffset = Hitbox.Offset;
	if (!bAttackerFacingRight)
	{
		HitboxOffset.X *= -1;
	}

	FVector HitboxCenter = AttackerPos + HitboxOffset;
	FVector HurtboxCenter = DefenderPos + Hurtbox.Offset;

	// Simple AABB collision check
	FVector HitMin = HitboxCenter - Hitbox.Extent;
	FVector HitMax = HitboxCenter + Hitbox.Extent;
	FVector HurtMin = HurtboxCenter - Hurtbox.Extent;
	FVector HurtMax = HurtboxCenter + Hurtbox.Extent;

	return (HitMin.X <= HurtMax.X && HitMax.X >= HurtMin.X) &&
		   (HitMin.Y <= HurtMax.Y && HitMax.Y >= HurtMin.Y) &&
		   (HitMin.Z <= HurtMax.Z && HitMax.Z >= HurtMin.Z);
}

void FForge2FightingManager::EnableTrainingMode(const FForge2TrainingModeSettings& Settings)
{
	bTrainingMode = true;
	TrainingSettings = Settings;
}

void FForge2FightingManager::DisableTrainingMode()
{
	bTrainingMode = false;
}

void FForge2FightingManager::Reset()
{
	Fighters.Empty();
	Stages.Empty();
	bMatchActive = false;
	bTrainingMode = false;
}

void FForge2FightingManager::UpdateFighterState(FForge2FighterRuntimeState& State, const FForge2FighterDefinition& Definition, float DeltaTime)
{
	// Update current move frame
	if (!State.CurrentMoveId.IsEmpty())
	{
		State.CurrentMoveFrame++;

		const FForge2MoveDefinition* CurrentMove = Definition.GetMove(State.CurrentMoveId);
		if (CurrentMove && State.CurrentMoveFrame >= CurrentMove->FrameData.GetTotalFrames())
		{
			// Move finished
			State.CurrentMoveId.Empty();
			State.CurrentMoveFrame = 0;
			State.CurrentState = EForge2FighterState::Standing;
		}
	}

	// Update stun recovery
	if (State.CurrentStun > 0 && !State.bIsStunned)
	{
		State.CurrentStun -= FMath::RoundToInt(MatchConfig.StunRecoveryRate * DeltaTime);
		State.CurrentStun = FMath::Max(0, State.CurrentStun);
	}

	// Check for stun
	if (State.CurrentStun >= Definition.Stats.StunThreshold && !State.bIsStunned)
	{
		State.bIsStunned = true;
	}
}

void FForge2FightingManager::ProcessMove(int32 PlayerIndex, const FString& MoveId)
{
	FForge2FighterRuntimeState* State = (PlayerIndex == 0) ?
		&MatchState.Player1State : &MatchState.Player2State;

	const FForge2FighterDefinition* Fighter = (PlayerIndex == 0) ?
		Fighters.Find(ActiveFighter1Id) : Fighters.Find(ActiveFighter2Id);

	if (!Fighter)
	{
		return;
	}

	const FForge2MoveDefinition* Move = Fighter->GetMove(MoveId);
	if (!Move || !Move->bEnabled)
	{
		return;
	}

	// Check meter cost
	if (Move->MeterCost > 0 && Fighter->Meters.Num() > 0)
	{
		if (!State->MeterStates[0].CanAfford(Move->MeterCost, Fighter->Meters[0]))
		{
			return;
		}
		State->MeterStates[0].SpendMeter(Move->MeterCost, Fighter->Meters[0]);
	}

	// Start the move
	State->CurrentMoveId = MoveId;
	State->CurrentMoveFrame = 0;
	State->CurrentState = EForge2FighterState::Attacking;
}

void FForge2FightingManager::ApplyHit(int32 AttackerIndex, const FForge2MoveDefinition& Move, bool bCounterHit)
{
	FForge2FighterRuntimeState* DefenderState = (AttackerIndex == 0) ?
		&MatchState.Player2State : &MatchState.Player1State;

	const FForge2FighterDefinition* Defender = (AttackerIndex == 0) ?
		Fighters.Find(ActiveFighter2Id) : Fighters.Find(ActiveFighter1Id);

	if (!Defender)
	{
		return;
	}

	// Calculate and apply damage
	int32 Damage = CalculateDamage(Move, DefenderState->ComboState, bCounterHit, Defender->Stats.DefenseModifier);
	DefenderState->CurrentHealth -= Damage;
	DefenderState->CurrentHealth = FMath::Max(0, DefenderState->CurrentHealth);

	// Apply stun damage
	DefenderState->CurrentStun += Move.StunDamage;

	// Update combo state
	DefenderState->ComboState.AddHit(Move, bCounterHit);

	// Set defender state
	DefenderState->CurrentState = EForge2FighterState::Hitstun;

	// Award meter to attacker
	FForge2FighterRuntimeState* AttackerState = (AttackerIndex == 0) ?
		&MatchState.Player1State : &MatchState.Player2State;

	const FForge2FighterDefinition* Attacker = (AttackerIndex == 0) ?
		Fighters.Find(ActiveFighter1Id) : Fighters.Find(ActiveFighter2Id);

	if (Attacker && Attacker->Meters.Num() > 0 && AttackerState->MeterStates.Num() > 0)
	{
		AttackerState->MeterStates[0].AddMeter(Move.MeterGainHit, Attacker->Meters[0]);
	}
}

void FForge2FightingManager::CheckRoundEnd()
{
	// Check KO
	if (MatchState.Player1State.CurrentHealth <= 0)
	{
		EndRound(2); // Player 2 wins
		return;
	}

	if (MatchState.Player2State.CurrentHealth <= 0)
	{
		EndRound(1); // Player 1 wins
		return;
	}

	// Check time out
	if (MatchConfig.RoundTimeLimit > 0 && MatchState.RoundTimeRemaining <= 0)
	{
		if (MatchState.Player1State.CurrentHealth > MatchState.Player2State.CurrentHealth)
		{
			EndRound(1);
		}
		else if (MatchState.Player2State.CurrentHealth > MatchState.Player1State.CurrentHealth)
		{
			EndRound(2);
		}
		else
		{
			EndRound(0); // Draw
		}
	}
}

// ============================================================================
// FIGHTING UTILITIES
// ============================================================================

FForge2FighterDefinition FForge2FightingUtils::CreateShotoFighter(const FString& FighterId, const FText& Name)
{
	FForge2FighterDefinition Fighter;
	Fighter.FighterId = FighterId;
	Fighter.FighterName = Name;
	Fighter.Archetype = EForge2FighterArchetype::Balanced;

	// Default stats
	Fighter.Stats.MaxHealth = 10000;
	Fighter.Stats.WalkSpeedForward = 400.0f;
	Fighter.Stats.WalkSpeedBackward = 350.0f;
	Fighter.Stats.DashDistance = 300.0f;
	Fighter.Stats.JumpHeight = 400.0f;

	// Add basic meter
	FForge2MeterDefinition SuperMeter;
	SuperMeter.MeterType = EForge2MeterType::Super;
	SuperMeter.MaxStocks = 3;
	SuperMeter.ValuePerStock = 100;
	Fighter.Meters.Add(SuperMeter);

	// Add basic normals
	Fighter.Moveset = CreateBasicNormals();

	// Add fireball
	Fighter.Moveset.Add(CreateFireball(TEXT("FIREBALL"), FText::FromString(TEXT("Hadoken")), 60, 0));

	// Add dragon punch
	Fighter.Moveset.Add(CreateDragonPunch(TEXT("DP"), FText::FromString(TEXT("Shoryuken")), 100, 5));

	// Add hurtboxes
	FForge2HurtboxData StandingHurtbox;
	StandingHurtbox.Extent = FVector(30.0f, 15.0f, 90.0f);
	StandingHurtbox.bStanding = true;
	Fighter.Hurtboxes.Add(StandingHurtbox);

	FForge2HurtboxData CrouchingHurtbox;
	CrouchingHurtbox.Extent = FVector(35.0f, 15.0f, 50.0f);
	CrouchingHurtbox.Offset = FVector(0.0f, 0.0f, 25.0f);
	CrouchingHurtbox.bCrouching = true;
	Fighter.Hurtboxes.Add(CrouchingHurtbox);

	return Fighter;
}

FForge2FighterDefinition FForge2FightingUtils::CreateGrapplerFighter(const FString& FighterId, const FText& Name)
{
	FForge2FighterDefinition Fighter;
	Fighter.FighterId = FighterId;
	Fighter.FighterName = Name;
	Fighter.Archetype = EForge2FighterArchetype::Grappler;

	// Grappler stats
	Fighter.Stats.MaxHealth = 11000;
	Fighter.Stats.WalkSpeedForward = 300.0f;
	Fighter.Stats.WalkSpeedBackward = 250.0f;
	Fighter.Stats.DashDistance = 400.0f;
	Fighter.Stats.JumpHeight = 350.0f;
	Fighter.Stats.DefenseModifier = 0.9f;

	// Add basic meter
	FForge2MeterDefinition SuperMeter;
	SuperMeter.MeterType = EForge2MeterType::Super;
	SuperMeter.MaxStocks = 3;
	SuperMeter.ValuePerStock = 100;
	Fighter.Meters.Add(SuperMeter);

	// Add basic normals
	Fighter.Moveset = CreateBasicNormals();

	// Add command grab
	Fighter.Moveset.Add(CreateCommandGrab(TEXT("SPD"), FText::FromString(TEXT("Spinning Piledriver")), 200));

	// Add hurtboxes (bigger)
	FForge2HurtboxData StandingHurtbox;
	StandingHurtbox.Extent = FVector(40.0f, 20.0f, 100.0f);
	StandingHurtbox.bStanding = true;
	Fighter.Hurtboxes.Add(StandingHurtbox);

	return Fighter;
}

FForge2FighterDefinition FForge2FightingUtils::CreateZonerFighter(const FString& FighterId, const FText& Name)
{
	FForge2FighterDefinition Fighter;
	Fighter.FighterId = FighterId;
	Fighter.FighterName = Name;
	Fighter.Archetype = EForge2FighterArchetype::Zoner;

	// Zoner stats
	Fighter.Stats.MaxHealth = 9000;
	Fighter.Stats.WalkSpeedForward = 350.0f;
	Fighter.Stats.WalkSpeedBackward = 400.0f;
	Fighter.Stats.DashDistance = 250.0f;
	Fighter.Stats.JumpHeight = 450.0f;

	// Add basic meter
	FForge2MeterDefinition SuperMeter;
	SuperMeter.MeterType = EForge2MeterType::Super;
	SuperMeter.MaxStocks = 3;
	SuperMeter.ValuePerStock = 100;
	Fighter.Meters.Add(SuperMeter);

	// Add basic normals
	Fighter.Moveset = CreateBasicNormals();

	// Add multiple projectiles
	Fighter.Moveset.Add(CreateFireball(TEXT("FIREBALL_L"), FText::FromString(TEXT("Light Projectile")), 40, 0));
	Fighter.Moveset.Add(CreateFireball(TEXT("FIREBALL_M"), FText::FromString(TEXT("Medium Projectile")), 60, 0));
	Fighter.Moveset.Add(CreateFireball(TEXT("FIREBALL_H"), FText::FromString(TEXT("Heavy Projectile")), 80, 0));

	// Add hurtboxes (smaller)
	FForge2HurtboxData StandingHurtbox;
	StandingHurtbox.Extent = FVector(25.0f, 12.0f, 85.0f);
	StandingHurtbox.bStanding = true;
	Fighter.Hurtboxes.Add(StandingHurtbox);

	return Fighter;
}

TArray<FForge2MoveDefinition> FForge2FightingUtils::CreateBasicNormals()
{
	TArray<FForge2MoveDefinition> Moves;

	// Standing Light
	{
		FForge2MoveDefinition Move;
		Move.MoveId = TEXT("5LP");
		Move.MoveName = FText::FromString(TEXT("Standing Light Punch"));
		Move.MoveType = EForge2MoveType::Standing_Light;
		Move.AttackProperty = EForge2AttackProperty::Mid;
		Move.BaseDamage = 25;
		Move.FrameData = CreateFrameData(4, 2, 6, -2, 2);
		Move.Input.Buttons.Add(EForge2InputButton::Light_Punch);

		FForge2HitboxData Hitbox;
		Hitbox.Offset = FVector(40.0f, 0.0f, 60.0f);
		Hitbox.Extent = FVector(30.0f, 15.0f, 20.0f);
		Hitbox.HitstunFrames = 10;
		Move.Hitboxes.Add(Hitbox);

		Moves.Add(Move);
	}

	// Standing Medium
	{
		FForge2MoveDefinition Move;
		Move.MoveId = TEXT("5MP");
		Move.MoveName = FText::FromString(TEXT("Standing Medium Punch"));
		Move.MoveType = EForge2MoveType::Standing_Medium;
		Move.AttackProperty = EForge2AttackProperty::Mid;
		Move.BaseDamage = 50;
		Move.FrameData = CreateFrameData(6, 3, 10, -4, 4);
		Move.Input.Buttons.Add(EForge2InputButton::Medium_Punch);

		FForge2HitboxData Hitbox;
		Hitbox.Offset = FVector(50.0f, 0.0f, 65.0f);
		Hitbox.Extent = FVector(40.0f, 15.0f, 25.0f);
		Hitbox.HitstunFrames = 15;
		Move.Hitboxes.Add(Hitbox);

		Moves.Add(Move);
	}

	// Standing Heavy
	{
		FForge2MoveDefinition Move;
		Move.MoveId = TEXT("5HP");
		Move.MoveName = FText::FromString(TEXT("Standing Heavy Punch"));
		Move.MoveType = EForge2MoveType::Standing_Heavy;
		Move.AttackProperty = EForge2AttackProperty::Mid;
		Move.BaseDamage = 80;
		Move.FrameData = CreateFrameData(10, 4, 18, -8, 3);
		Move.Input.Buttons.Add(EForge2InputButton::Heavy_Punch);

		FForge2HitboxData Hitbox;
		Hitbox.Offset = FVector(60.0f, 0.0f, 70.0f);
		Hitbox.Extent = FVector(50.0f, 20.0f, 30.0f);
		Hitbox.HitstunFrames = 20;
		Hitbox.HitEffect = EForge2HitEffect::Knockdown;
		Move.Hitboxes.Add(Hitbox);

		Moves.Add(Move);
	}

	// Crouching Light
	{
		FForge2MoveDefinition Move;
		Move.MoveId = TEXT("2LK");
		Move.MoveName = FText::FromString(TEXT("Crouching Light Kick"));
		Move.MoveType = EForge2MoveType::Crouching_Light;
		Move.AttackProperty = EForge2AttackProperty::Low;
		Move.BaseDamage = 20;
		Move.FrameData = CreateFrameData(4, 2, 7, -1, 3);
		Move.Input.Buttons.Add(EForge2InputButton::Light_Kick);
		Move.Input.DirectionRequired = FVector2D(0.0f, -1.0f);

		FForge2HitboxData Hitbox;
		Hitbox.Offset = FVector(35.0f, 0.0f, 10.0f);
		Hitbox.Extent = FVector(35.0f, 15.0f, 15.0f);
		Hitbox.HitstunFrames = 10;
		Hitbox.bHitsStanding = false;
		Move.Hitboxes.Add(Hitbox);

		Moves.Add(Move);
	}

	// Crouching Heavy (Sweep)
	{
		FForge2MoveDefinition Move;
		Move.MoveId = TEXT("2HK");
		Move.MoveName = FText::FromString(TEXT("Sweep"));
		Move.MoveType = EForge2MoveType::Crouching_Heavy;
		Move.AttackProperty = EForge2AttackProperty::Low;
		Move.BaseDamage = 70;
		Move.FrameData = CreateFrameData(8, 3, 22, -12, 0);
		Move.Input.Buttons.Add(EForge2InputButton::Heavy_Kick);
		Move.Input.DirectionRequired = FVector2D(0.0f, -1.0f);

		FForge2HitboxData Hitbox;
		Hitbox.Offset = FVector(50.0f, 0.0f, 5.0f);
		Hitbox.Extent = FVector(60.0f, 15.0f, 15.0f);
		Hitbox.HitstunFrames = 0;
		Hitbox.HitEffect = EForge2HitEffect::Hard_Knockdown;
		Hitbox.bHitsStanding = false;
		Move.Hitboxes.Add(Hitbox);

		Moves.Add(Move);
	}

	// Jumping Heavy
	{
		FForge2MoveDefinition Move;
		Move.MoveId = TEXT("jHP");
		Move.MoveName = FText::FromString(TEXT("Jumping Heavy Punch"));
		Move.MoveType = EForge2MoveType::Jumping_Heavy;
		Move.AttackProperty = EForge2AttackProperty::Overhead;
		Move.BaseDamage = 70;
		Move.FrameData = CreateFrameData(7, 5, 0, 0, 0);
		Move.FrameData.bAirborne = true;
		Move.Input.Buttons.Add(EForge2InputButton::Heavy_Punch);
		Move.Input.bCanDoInAir = true;

		FForge2HitboxData Hitbox;
		Hitbox.Offset = FVector(40.0f, 0.0f, -20.0f);
		Hitbox.Extent = FVector(45.0f, 20.0f, 40.0f);
		Hitbox.HitstunFrames = 18;
		Hitbox.bHitsCrouching = true;
		Move.Hitboxes.Add(Hitbox);

		Moves.Add(Move);
	}

	return Moves;
}

FForge2MoveDefinition FForge2FightingUtils::CreateFireball(const FString& MoveId, const FText& Name, int32 Damage, int32 MeterCost)
{
	FForge2MoveDefinition Move;
	Move.MoveId = MoveId;
	Move.MoveName = Name;
	Move.MoveType = EForge2MoveType::Special;
	Move.AttackProperty = EForge2AttackProperty::Projectile;
	Move.BaseDamage = Damage;
	Move.MeterCost = MeterCost;
	Move.FrameData = CreateFrameData(12, 2, 30, -10, -5);

	Move.Input.MotionInput = EForge2MotionInput::QCF;
	Move.Input.Buttons.Add(EForge2InputButton::Punch);

	Move.ProjectileSpeed = 800.0f;

	FForge2HitboxData Hitbox;
	Hitbox.Extent = FVector(25.0f, 25.0f, 25.0f);
	Hitbox.HitstunFrames = 15;
	Move.Hitboxes.Add(Hitbox);

	return Move;
}

FForge2MoveDefinition FForge2FightingUtils::CreateDragonPunch(const FString& MoveId, const FText& Name, int32 Damage, int32 InvincibilityFrames)
{
	FForge2MoveDefinition Move;
	Move.MoveId = MoveId;
	Move.MoveName = Name;
	Move.MoveType = EForge2MoveType::Special;
	Move.AttackProperty = EForge2AttackProperty::AntiAir;
	Move.BaseDamage = Damage;
	Move.FrameData = CreateFrameData(3, 10, 25, -30, 0);
	Move.FrameData.InvincibilityStartFrame = 1;
	Move.FrameData.InvincibilityEndFrame = InvincibilityFrames;

	Move.Input.MotionInput = EForge2MotionInput::DP;
	Move.Input.Buttons.Add(EForge2InputButton::Punch);

	FForge2HitboxData Hitbox;
	Hitbox.Offset = FVector(30.0f, 0.0f, 80.0f);
	Hitbox.Extent = FVector(40.0f, 30.0f, 60.0f);
	Hitbox.HitstunFrames = 0;
	Hitbox.HitEffect = EForge2HitEffect::Knockdown;
	Hitbox.KnockbackDirection = FVector(0.5f, 0.0f, 1.0f);
	Hitbox.KnockbackStrength = 500.0f;
	Move.Hitboxes.Add(Hitbox);

	return Move;
}

FForge2MoveDefinition FForge2FightingUtils::CreateCommandGrab(const FString& MoveId, const FText& Name, int32 Damage)
{
	FForge2MoveDefinition Move;
	Move.MoveId = MoveId;
	Move.MoveName = Name;
	Move.MoveType = EForge2MoveType::Special;
	Move.AttackProperty = EForge2AttackProperty::Grab;
	Move.BaseDamage = Damage;
	Move.FrameData = CreateFrameData(2, 3, 40, 0, 0);

	Move.Input.MotionInput = EForge2MotionInput::Full_Circle;
	Move.Input.Buttons.Add(EForge2InputButton::Punch);

	FForge2HitboxData Hitbox;
	Hitbox.Offset = FVector(50.0f, 0.0f, 50.0f);
	Hitbox.Extent = FVector(60.0f, 30.0f, 50.0f);
	Hitbox.HitstunFrames = 0;
	Hitbox.HitEffect = EForge2HitEffect::Hard_Knockdown;
	Hitbox.bHitsAirborne = false; // Grabs don't hit airborne
	Move.Hitboxes.Add(Hitbox);

	return Move;
}

FForge2StageDefinition FForge2FightingUtils::CreateBasicStage(const FString& StageId, const FText& Name, float Width)
{
	FForge2StageDefinition Stage;
	Stage.StageId = StageId;
	Stage.StageName = Name;
	Stage.StageWidth = Width;
	Stage.StartingDistance = 500.0f;

	// Add walls at edges
	FForge2StageWall LeftWall;
	LeftWall.WallPosition = -Width / 2.0f;
	LeftWall.bBreakable = false;
	Stage.Walls.Add(LeftWall);

	FForge2StageWall RightWall;
	RightWall.WallPosition = Width / 2.0f;
	RightWall.bBreakable = false;
	Stage.Walls.Add(RightWall);

	return Stage;
}

FForge2StageDefinition FForge2FightingUtils::CreateCornerStage(const FString& StageId, const FText& Name, float Width, bool bBreakableWalls)
{
	FForge2StageDefinition Stage;
	Stage.StageId = StageId;
	Stage.StageName = Name;
	Stage.StageWidth = Width;
	Stage.StartingDistance = 500.0f;

	FForge2StageWall LeftWall;
	LeftWall.WallPosition = -Width / 2.0f;
	LeftWall.bBreakable = bBreakableWalls;
	LeftWall.HitsToBreak = 3;
	LeftWall.WallbreakBonusDamage = 500;
	Stage.Walls.Add(LeftWall);

	FForge2StageWall RightWall;
	RightWall.WallPosition = Width / 2.0f;
	RightWall.bBreakable = bBreakableWalls;
	RightWall.HitsToBreak = 3;
	RightWall.WallbreakBonusDamage = 500;
	Stage.Walls.Add(RightWall);

	return Stage;
}

FForge2FrameData FForge2FightingUtils::CreateFrameData(int32 Startup, int32 Active, int32 Recovery, int32 BlockAdv, int32 HitAdv)
{
	FForge2FrameData FrameData;
	FrameData.StartupFrames = Startup;
	FrameData.ActiveFrames = Active;
	FrameData.RecoveryFrames = Recovery;
	FrameData.BlockAdvantage = BlockAdv;
	FrameData.HitAdvantage = HitAdv;
	FrameData.CounterHitAdvantage = HitAdv + 4;

	// Set default cancel window
	FrameData.CancelWindowStart = Startup;
	FrameData.CancelWindowEnd = Startup + Active + 5;

	return FrameData;
}

EForge2MotionInput FForge2FightingUtils::ParseMotionInput(const TArray<int32>& InputBuffer)
{
	// This is a simplified motion input parser
	// A real implementation would be more sophisticated

	if (InputBuffer.Num() < 3)
	{
		return EForge2MotionInput::None;
	}

	// Extract recent directional inputs (simplified)
	// In a real game, each input would have directional and button data encoded

	// For now, just return None - actual implementation would parse
	// the numpad notation from the input buffer
	return EForge2MotionInput::None;
}

FString FForge2FightingUtils::MotionInputToString(EForge2MotionInput Motion)
{
	switch (Motion)
	{
	case EForge2MotionInput::QCF: return TEXT("236");
	case EForge2MotionInput::QCB: return TEXT("214");
	case EForge2MotionInput::HCF: return TEXT("41236");
	case EForge2MotionInput::HCB: return TEXT("63214");
	case EForge2MotionInput::DP: return TEXT("623");
	case EForge2MotionInput::RDP: return TEXT("421");
	case EForge2MotionInput::Charge_Back_Forward: return TEXT("[4]6");
	case EForge2MotionInput::Charge_Down_Up: return TEXT("[2]8");
	case EForge2MotionInput::Full_Circle: return TEXT("360");
	case EForge2MotionInput::Double_QCF: return TEXT("236236");
	case EForge2MotionInput::Double_QCB: return TEXT("214214");
	case EForge2MotionInput::SPD: return TEXT("720");
	case EForge2MotionInput::Pretzel: return TEXT("1632143");
	default: return TEXT("");
	}
}

FString FForge2FightingUtils::InputButtonToString(EForge2InputButton Button)
{
	switch (Button)
	{
	case EForge2InputButton::Light_Punch: return TEXT("LP");
	case EForge2InputButton::Medium_Punch: return TEXT("MP");
	case EForge2InputButton::Heavy_Punch: return TEXT("HP");
	case EForge2InputButton::Light_Kick: return TEXT("LK");
	case EForge2InputButton::Medium_Kick: return TEXT("MK");
	case EForge2InputButton::Heavy_Kick: return TEXT("HK");
	case EForge2InputButton::Punch: return TEXT("P");
	case EForge2InputButton::Kick: return TEXT("K");
	case EForge2InputButton::Slash: return TEXT("S");
	case EForge2InputButton::Heavy_Slash: return TEXT("HS");
	case EForge2InputButton::Dust: return TEXT("D");
	default: return TEXT("");
	}
}

int32 FForge2FightingUtils::CalculateComboTotalDamage(const TArray<FForge2MoveDefinition>& Moves, const FForge2MatchConfiguration& Config)
{
	FForge2ComboState ComboState;
	int32 TotalDamage = 0;

	for (const FForge2MoveDefinition& Move : Moves)
	{
		int32 MoveDamage = Move.CalculateComboScaledDamage(ComboState.HitCount, ComboState.DamageScaling);
		TotalDamage += MoveDamage;
		ComboState.AddHit(Move, false);
	}

	return TotalDamage;
}
