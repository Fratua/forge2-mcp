// Forge2InputSystem.cpp
// FORGE2 Enhanced Input System Implementation

#include "Forge2InputSystem.h"
#include "Curves/CurveFloat.h"

//=============================================================================
// FForge2InputBuffer Implementation
//=============================================================================

void FForge2InputBuffer::Configure(const FForge2InputBufferConfig& Config)
{
	BufferConfig = Config;
}

void FForge2InputBuffer::AddInput(const FForge2BufferedInput& Input)
{
	// Check if action is bufferable
	if (BufferConfig.BufferableActions.Num() > 0 &&
		!BufferConfig.BufferableActions.Contains(Input.ActionId))
	{
		return;
	}

	// Add to buffer
	Buffer.Add(Input);

	// Trim if over max size
	while (Buffer.Num() > BufferConfig.MaxBufferSize)
	{
		Buffer.RemoveAt(0);
	}
}

void FForge2InputBuffer::Clear()
{
	Buffer.Empty();
}

void FForge2InputBuffer::Update(float CurrentTime)
{
	// Remove expired inputs
	Buffer.RemoveAll([&](const FForge2BufferedInput& Input)
	{
		return (CurrentTime - Input.Timestamp) > BufferConfig.BufferDuration;
	});
}

bool FForge2InputBuffer::HasInput(const FString& ActionId) const
{
	for (const FForge2BufferedInput& Input : Buffer)
	{
		if (Input.ActionId == ActionId)
		{
			return true;
		}
	}
	return false;
}

TOptional<FForge2BufferedInput> FForge2InputBuffer::GetInput(const FString& ActionId) const
{
	// Return most recent matching input
	for (int32 i = Buffer.Num() - 1; i >= 0; i--)
	{
		if (Buffer[i].ActionId == ActionId)
		{
			return Buffer[i];
		}
	}
	return TOptional<FForge2BufferedInput>();
}

TOptional<FForge2BufferedInput> FForge2InputBuffer::ConsumeInput(const FString& ActionId)
{
	for (int32 i = Buffer.Num() - 1; i >= 0; i--)
	{
		if (Buffer[i].ActionId == ActionId)
		{
			FForge2BufferedInput Result = Buffer[i];
			if (BufferConfig.bClearOnConsume)
			{
				Buffer.RemoveAt(i);
			}
			return Result;
		}
	}
	return TOptional<FForge2BufferedInput>();
}

TArray<FForge2BufferedInput> FForge2InputBuffer::GetAllInputs() const
{
	return Buffer;
}

TOptional<FForge2BufferedInput> FForge2InputBuffer::GetMostRecent(float CurrentTime) const
{
	float MostRecentTime = -FLT_MAX;
	TOptional<FForge2BufferedInput> MostRecent;

	for (const FForge2BufferedInput& Input : Buffer)
	{
		if (Input.Timestamp > MostRecentTime &&
			(CurrentTime - Input.Timestamp) <= BufferConfig.BufferDuration)
		{
			MostRecentTime = Input.Timestamp;
			MostRecent = Input;
		}
	}

	return MostRecent;
}

//=============================================================================
// FForge2ComboDetector Implementation
//=============================================================================

void FForge2ComboDetector::AddCombo(const FForge2ComboSequence& Combo)
{
	// Insert sorted by priority (highest first)
	int32 InsertIndex = 0;
	for (int32 i = 0; i < RegisteredCombos.Num(); i++)
	{
		if (Combo.Priority > RegisteredCombos[i].Priority)
		{
			InsertIndex = i;
			break;
		}
		InsertIndex = i + 1;
	}
	RegisteredCombos.Insert(Combo, InsertIndex);
}

void FForge2ComboDetector::RemoveCombo(const FString& ComboId)
{
	RegisteredCombos.RemoveAll([&](const FForge2ComboSequence& Combo)
	{
		return Combo.ComboId == ComboId;
	});
}

void FForge2ComboDetector::ClearCombos()
{
	RegisteredCombos.Empty();
	Reset();
}

void FForge2ComboDetector::ProcessInput(const FForge2BufferedInput& Input, float CurrentTime, const FGameplayTagContainer& CurrentTags)
{
	CompletedCombo.Reset();

	// Update existing combo progress
	for (int32 i = ActiveCombos.Num() - 1; i >= 0; i--)
	{
		FComboProgress& Progress = ActiveCombos[i];

		// Find the combo definition
		const FForge2ComboSequence* Combo = nullptr;
		for (const FForge2ComboSequence& C : RegisteredCombos)
		{
			if (C.ComboId == Progress.ComboId)
			{
				Combo = &C;
				break;
			}
		}

		if (!Combo)
		{
			ActiveCombos.RemoveAt(i);
			continue;
		}

		// Check time limits
		if (Combo->TotalTimeLimit > 0.0f && (CurrentTime - Progress.StartTime) > Combo->TotalTimeLimit)
		{
			ActiveCombos.RemoveAt(i);
			continue;
		}

		// Check if current step matches
		if (Progress.CurrentStep < Combo->Steps.Num())
		{
			const FForge2ComboInputStep& Step = Combo->Steps[Progress.CurrentStep];

			// Check time window from last input
			if ((CurrentTime - Progress.LastInputTime) > Step.MaxTimeWindow)
			{
				ActiveCombos.RemoveAt(i);
				continue;
			}

			// Check if input matches step
			if (CheckStepMatch(Step, Input))
			{
				Progress.CurrentStep++;
				Progress.LastInputTime = CurrentTime;

				// Check if combo is complete
				if (Progress.CurrentStep >= Combo->Steps.Num())
				{
					CompletedCombo = Combo->OutputAction;
					ActiveCombos.RemoveAt(i);
					return;  // Return first completed combo (highest priority)
				}
			}
		}
	}

	// Check for new combo starts
	for (const FForge2ComboSequence& Combo : RegisteredCombos)
	{
		// Check tag requirements
		if (!Combo.RequiredTags.IsEmpty() && !CurrentTags.HasAll(Combo.RequiredTags))
		{
			continue;
		}

		// Check if already tracking this combo
		bool bAlreadyTracking = false;
		for (const FComboProgress& Progress : ActiveCombos)
		{
			if (Progress.ComboId == Combo.ComboId)
			{
				bAlreadyTracking = true;
				break;
			}
		}

		if (bAlreadyTracking)
		{
			continue;
		}

		// Check first step
		if (Combo.Steps.Num() > 0 && CheckStepMatch(Combo.Steps[0], Input))
		{
			FComboProgress NewProgress;
			NewProgress.ComboId = Combo.ComboId;
			NewProgress.CurrentStep = 1;
			NewProgress.StartTime = CurrentTime;
			NewProgress.LastInputTime = CurrentTime;

			// Single step combos complete immediately
			if (Combo.Steps.Num() == 1)
			{
				CompletedCombo = Combo.OutputAction;
				return;
			}

			ActiveCombos.Add(NewProgress);
		}
	}
}

TOptional<FString> FForge2ComboDetector::CheckForCompletedCombo()
{
	TOptional<FString> Result = CompletedCombo;
	CompletedCombo.Reset();
	return Result;
}

void FForge2ComboDetector::Reset()
{
	ActiveCombos.Empty();
	CompletedCombo.Reset();
}

float FForge2ComboDetector::GetComboProgress(const FString& ComboId) const
{
	for (const FComboProgress& Progress : ActiveCombos)
	{
		if (Progress.ComboId == ComboId)
		{
			for (const FForge2ComboSequence& Combo : RegisteredCombos)
			{
				if (Combo.ComboId == ComboId && Combo.Steps.Num() > 0)
				{
					return static_cast<float>(Progress.CurrentStep) / static_cast<float>(Combo.Steps.Num());
				}
			}
		}
	}
	return 0.0f;
}

TArray<FString> FForge2ComboDetector::GetActivePartialCombos() const
{
	TArray<FString> Result;
	for (const FComboProgress& Progress : ActiveCombos)
	{
		Result.Add(Progress.ComboId);
	}
	return Result;
}

bool FForge2ComboDetector::CheckStepMatch(const FForge2ComboInputStep& Step, const FForge2BufferedInput& Input) const
{
	// Check required actions (any match)
	if (Step.RequiredActions.Num() > 0)
	{
		bool bFoundMatch = false;
		for (const FString& RequiredAction : Step.RequiredActions)
		{
			if (Input.ActionId == RequiredAction)
			{
				bFoundMatch = true;
				break;
			}
		}
		if (!bFoundMatch)
		{
			return false;
		}
	}

	// Check hold requirement
	if (Step.bRequireHold && (!Input.bWasHeld || Input.HoldDuration < Step.MinHoldDuration))
	{
		return false;
	}

	// Check direction requirement
	if (!Step.DirectionRequirement.IsNearlyZero())
	{
		FVector2D NormalizedInput = Input.Direction.GetSafeNormal();
		FVector2D NormalizedRequired = Step.DirectionRequirement.GetSafeNormal();

		float Dot = FVector2D::DotProduct(NormalizedInput, NormalizedRequired);
		float Angle = FMath::Acos(Dot) * (180.0f / PI);

		if (Angle > Step.DirectionTolerance)
		{
			return false;
		}
	}

	return true;
}

//=============================================================================
// FForge2FightingInputProcessor Implementation
//=============================================================================

void FForge2FightingInputProcessor::AddMotion(const FForge2FightingMotion& Motion)
{
	Motions.Add(Motion);
}

void FForge2FightingInputProcessor::AddChargeInput(const FForge2ChargeInput& ChargeInput)
{
	ChargeInputs.Add(ChargeInput);
}

void FForge2FightingInputProcessor::ProcessInput(EForge2FightingInputType Input, float CurrentTime, int32 FrameNumber)
{
	// Add to history
	FForge2FightingInput NewInput;
	NewInput.InputType = Input;
	NewInput.Timestamp = CurrentTime;
	NewInput.FrameNumber = FrameNumber;
	InputHistory.Add(NewInput);

	// Trim history
	while (InputHistory.Num() > MaxInputHistorySize)
	{
		InputHistory.RemoveAt(0);
	}

	// Update charge states
	for (const FForge2ChargeInput& ChargeInput : ChargeInputs)
	{
		bool bFoundCharge = false;
		for (FChargeState& State : ActiveCharges)
		{
			if (State.ChargeId == ChargeInput.ChargeId)
			{
				bFoundCharge = true;

				// Check if still charging
				if (Input == ChargeInput.ChargeDirection)
				{
					// Continue charging
					if ((CurrentTime - State.StartTime) >= ChargeInput.MinChargeTime)
					{
						State.bFullyCharged = true;
					}
				}
				else if (Input != ChargeInput.ChargeDirection)
				{
					// Charge broken
					ActiveCharges.RemoveAll([&](const FChargeState& S) { return S.ChargeId == ChargeInput.ChargeId; });
				}
				break;
			}
		}

		// Start new charge
		if (!bFoundCharge && Input == ChargeInput.ChargeDirection)
		{
			FChargeState NewCharge;
			NewCharge.ChargeId = ChargeInput.ChargeId;
			NewCharge.StartTime = CurrentTime;
			NewCharge.bFullyCharged = false;
			ActiveCharges.Add(NewCharge);
		}
	}
}

TOptional<FString> FForge2FightingInputProcessor::CheckForMotion()
{
	if (InputHistory.Num() < 2)
	{
		return TOptional<FString>();
	}

	float CurrentTime = InputHistory.Last().Timestamp;

	for (const FForge2FightingMotion& Motion : Motions)
	{
		if (CheckMotionMatch(Motion, CurrentTime))
		{
			return Motion.OutputAction;
		}
	}

	return TOptional<FString>();
}

TOptional<FString> FForge2FightingInputProcessor::CheckForCharge()
{
	if (InputHistory.Num() < 2)
	{
		return TOptional<FString>();
	}

	EForge2FightingInputType LastInput = InputHistory.Last().InputType;
	float CurrentTime = InputHistory.Last().Timestamp;

	for (const FForge2ChargeInput& ChargeInput : ChargeInputs)
	{
		// Check if button pressed
		bool bButtonPressed = false;
		for (EForge2FightingInputType Finisher : ChargeInput.FinisherButtons)
		{
			if (LastInput == Finisher)
			{
				bButtonPressed = true;
				break;
			}
		}

		if (!bButtonPressed)
		{
			continue;
		}

		// Check for release direction in recent history
		for (int32 i = InputHistory.Num() - 2; i >= 0; i--)
		{
			float TimeDiff = CurrentTime - InputHistory[i].Timestamp;
			if (TimeDiff > ChargeInput.MaxReleaseWindow)
			{
				break;
			}

			if (InputHistory[i].InputType == ChargeInput.ReleaseDirection)
			{
				// Found release, check for charged state
				for (const FChargeState& State : ActiveCharges)
				{
					if (State.ChargeId == ChargeInput.ChargeId && State.bFullyCharged)
					{
						return ChargeInput.OutputAction;
					}
				}
			}
		}
	}

	return TOptional<FString>();
}

void FForge2FightingInputProcessor::Reset()
{
	InputHistory.Empty();
	ActiveCharges.Empty();
}

void FForge2FightingInputProcessor::ClearInputHistory()
{
	InputHistory.Empty();
}

TArray<FForge2FightingInput> FForge2FightingInputProcessor::GetInputHistory(float TimeWindow) const
{
	if (InputHistory.Num() == 0)
	{
		return TArray<FForge2FightingInput>();
	}

	TArray<FForge2FightingInput> Result;
	float CurrentTime = InputHistory.Last().Timestamp;

	for (const FForge2FightingInput& Input : InputHistory)
	{
		if ((CurrentTime - Input.Timestamp) <= TimeWindow)
		{
			Result.Add(Input);
		}
	}

	return Result;
}

bool FForge2FightingInputProcessor::IsCharging(const FString& ChargeId) const
{
	for (const FChargeState& State : ActiveCharges)
	{
		if (State.ChargeId == ChargeId)
		{
			return true;
		}
	}
	return false;
}

float FForge2FightingInputProcessor::GetChargeTime(const FString& ChargeId) const
{
	if (InputHistory.Num() == 0)
	{
		return 0.0f;
	}

	float CurrentTime = InputHistory.Last().Timestamp;
	for (const FChargeState& State : ActiveCharges)
	{
		if (State.ChargeId == ChargeId)
		{
			return CurrentTime - State.StartTime;
		}
	}
	return 0.0f;
}

bool FForge2FightingInputProcessor::CheckMotionMatch(const FForge2FightingMotion& Motion, float CurrentTime) const
{
	if (Motion.DirectionSequence.Num() == 0)
	{
		return false;
	}

	// Get inputs within motion time window
	TArray<FForge2FightingInput> RecentInputs = GetInputHistory(Motion.MaxDuration);
	if (RecentInputs.Num() < Motion.DirectionSequence.Num())
	{
		return false;
	}

	// Check if last input is a finisher button
	EForge2FightingInputType LastInput = RecentInputs.Last().InputType;
	bool bFinisherPressed = false;
	for (EForge2FightingInputType Finisher : Motion.FinisherButtons)
	{
		if (LastInput == Finisher)
		{
			bFinisherPressed = true;
			break;
		}
	}

	if (!bFinisherPressed)
	{
		return false;
	}

	// Match direction sequence (allowing shortcuts if enabled)
	int32 SequenceIndex = 0;
	for (int32 i = 0; i < RecentInputs.Num() - 1; i++)  // -1 to exclude button press
	{
		EForge2FightingInputType CurrentInput = RecentInputs[i].InputType;

		// Check if matches current sequence step
		if (CurrentInput == Motion.DirectionSequence[SequenceIndex])
		{
			SequenceIndex++;
			if (SequenceIndex >= Motion.DirectionSequence.Num())
			{
				return true;  // Full sequence matched
			}
		}
		else if (Motion.bAllowShortcuts)
		{
			// Check for shortcut match (e.g., DownForward counts as Down then Forward)
			if (SequenceIndex > 0)
			{
				EForge2FightingInputType Expected = Motion.DirectionSequence[SequenceIndex];
				EForge2FightingInputType Prev = Motion.DirectionSequence[SequenceIndex - 1];

				// DownForward shortcuts
				if ((Prev == EForge2FightingInputType::Down && Expected == EForge2FightingInputType::Forward &&
					 CurrentInput == EForge2FightingInputType::DownForward) ||
					(Prev == EForge2FightingInputType::Down && Expected == EForge2FightingInputType::Back &&
					 CurrentInput == EForge2FightingInputType::DownBack))
				{
					SequenceIndex++;
					if (SequenceIndex >= Motion.DirectionSequence.Num())
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

EForge2FightingInputType FForge2FightingInputProcessor::DirectionFromStick(FVector2D StickInput) const
{
	return FForge2InputUtils::GetNumpadDirection(StickInput);
}

//=============================================================================
// FForge2ContextManager Implementation
//=============================================================================

void FForge2ContextManager::AddContextualAction(const FForge2ContextualAction& Action)
{
	ContextualActions.Add(Action);
	// Sort by priority
	ContextualActions.Sort([](const FForge2ContextualAction& A, const FForge2ContextualAction& B)
	{
		return A.Priority > B.Priority;
	});
}

void FForge2ContextManager::RemoveContextualAction(const FString& ActionId)
{
	ContextualActions.RemoveAll([&](const FForge2ContextualAction& Action)
	{
		return Action.ActionId == ActionId;
	});
}

void FForge2ContextManager::ClearContextualActions()
{
	ContextualActions.Empty();
}

FString FForge2ContextManager::EvaluateContext(
	const FString& InputAction,
	const FGameplayTagContainer& Tags,
	bool bGrounded,
	bool bMoving,
	bool bTargetLocked,
	bool bNearInteractable,
	float HealthPercent,
	const TArray<FString>& Equipment)
{
	for (const FForge2ContextualAction& Action : ContextualActions)
	{
		if (Action.SourceInputAction != InputAction)
		{
			continue;
		}

		if (EvaluateCondition(Action.Condition, Tags, bGrounded, bMoving, bTargetLocked, bNearInteractable, HealthPercent, Equipment))
		{
			return Action.OutputAction;
		}
	}

	// Return original action if no contextual override
	return InputAction;
}

TOptional<FForge2ContextualAction> FForge2ContextManager::GetActivePrompt(const FString& InputAction) const
{
	for (const FForge2ContextualAction& Action : ContextualActions)
	{
		if (Action.SourceInputAction == InputAction && !Action.PromptText.IsEmpty())
		{
			return Action;
		}
	}
	return TOptional<FForge2ContextualAction>();
}

bool FForge2ContextManager::EvaluateCondition(
	const FForge2ContextCondition& Condition,
	const FGameplayTagContainer& Tags,
	bool bGrounded,
	bool bMoving,
	bool bTargetLocked,
	bool bNearInteractable,
	float HealthPercent,
	const TArray<FString>& Equipment) const
{
	// Check tag query
	if (!Condition.TagQuery.IsEmpty() && !Condition.TagQuery.Matches(Tags))
	{
		return false;
	}

	// Check state requirements
	if (Condition.bRequireGrounded && !bGrounded)
	{
		return false;
	}
	if (Condition.bRequireAirborne && bGrounded)
	{
		return false;
	}
	if (Condition.bRequireMoving && !bMoving)
	{
		return false;
	}
	if (Condition.bRequireIdle && bMoving)
	{
		return false;
	}
	if (Condition.bRequireTargetLocked && !bTargetLocked)
	{
		return false;
	}
	if (Condition.bRequireNearInteractable && !bNearInteractable)
	{
		return false;
	}

	// Check health
	if (HealthPercent < Condition.MinHealth || HealthPercent > Condition.MaxHealth)
	{
		return false;
	}

	// Check equipment
	for (const FString& RequiredItem : Condition.RequiredEquipment)
	{
		if (!Equipment.Contains(RequiredItem))
		{
			return false;
		}
	}

	return true;
}

//=============================================================================
// FForge2GestureRecognizer Implementation
//=============================================================================

void FForge2GestureRecognizer::AddGesture(const FForge2GestureDefinition& Gesture)
{
	GestureDefinitions.Add(Gesture);
}

void FForge2GestureRecognizer::RemoveGesture(const FString& GestureId)
{
	GestureDefinitions.RemoveAll([&](const FForge2GestureDefinition& Gesture)
	{
		return Gesture.GestureId == GestureId;
	});
}

void FForge2GestureRecognizer::BeginTouch(const FVector2D& Position, float Timestamp)
{
	TouchPath.Empty();
	FForge2GesturePoint Point;
	Point.Position = Position;
	Point.Timestamp = Timestamp;
	TouchPath.Add(Point);
}

void FForge2GestureRecognizer::UpdateTouch(const FVector2D& Position, float Timestamp)
{
	FForge2GesturePoint Point;
	Point.Position = Position;
	Point.Timestamp = Timestamp;
	TouchPath.Add(Point);
}

FForge2GestureResult FForge2GestureRecognizer::EndTouch(const FVector2D& Position, float Timestamp)
{
	FForge2GestureResult Result;

	if (TouchPath.Num() < 2)
	{
		// Check for tap
		if (TouchPath.Num() == 1)
		{
			float Duration = Timestamp - TouchPath[0].Timestamp;
			if (Duration < 0.3f)  // Quick tap
			{
				// Check for double tap
				if ((Timestamp - LastTapTime) < 0.4f &&
					FVector2D::Distance(Position, LastTapPosition) < 50.0f)
				{
					Result.bRecognized = true;
					Result.GestureType = EForge2GestureType::DoubleTap;
				}
				else
				{
					Result.bRecognized = true;
					Result.GestureType = EForge2GestureType::Tap;
				}
				LastTapTime = Timestamp;
				LastTapPosition = Position;
			}
			else
			{
				Result.bRecognized = true;
				Result.GestureType = EForge2GestureType::LongPress;
			}
		}
		return Result;
	}

	// Add final point
	FForge2GesturePoint FinalPoint;
	FinalPoint.Position = Position;
	FinalPoint.Timestamp = Timestamp;
	TouchPath.Add(FinalPoint);

	// Analyze gesture
	Result.GestureType = RecognizeGesture(TouchPath);
	Result.bRecognized = (Result.GestureType != EForge2GestureType::None);

	if (Result.bRecognized)
	{
		Result.StartPosition = TouchPath[0].Position;
		Result.EndPosition = Position;
		Result.Duration = Timestamp - TouchPath[0].Timestamp;
		Result.Direction = CalculateSwipeDirection(TouchPath);
		Result.Magnitude = FVector2D::Distance(TouchPath[0].Position, Position);

		// Find matching gesture definition
		for (const FForge2GestureDefinition& GestureDef : GestureDefinitions)
		{
			if (GestureDef.GestureType == Result.GestureType)
			{
				Result.MatchedGestureId = GestureDef.GestureId;
				break;
			}
		}
	}

	TouchPath.Empty();
	return Result;
}

void FForge2GestureRecognizer::BeginSecondTouch(const FVector2D& Position, float Timestamp)
{
	SecondTouchPath.Empty();
	FForge2GesturePoint Point;
	Point.Position = Position;
	Point.Timestamp = Timestamp;
	SecondTouchPath.Add(Point);
}

void FForge2GestureRecognizer::UpdateSecondTouch(const FVector2D& Position, float Timestamp)
{
	FForge2GesturePoint Point;
	Point.Position = Position;
	Point.Timestamp = Timestamp;
	SecondTouchPath.Add(Point);
}

FForge2GestureResult FForge2GestureRecognizer::EndSecondTouch(const FVector2D& Position, float Timestamp)
{
	FForge2GestureResult Result;

	if (TouchPath.Num() < 2 || SecondTouchPath.Num() < 2)
	{
		return Result;
	}

	// Calculate pinch/spread
	float StartDistance = FVector2D::Distance(TouchPath[0].Position, SecondTouchPath[0].Position);
	float EndDistance = FVector2D::Distance(TouchPath.Last().Position, Position);

	float Ratio = EndDistance / StartDistance;

	if (Ratio < 0.7f)
	{
		Result.bRecognized = true;
		Result.GestureType = EForge2GestureType::Pinch;
		Result.Magnitude = 1.0f - Ratio;
	}
	else if (Ratio > 1.3f)
	{
		Result.bRecognized = true;
		Result.GestureType = EForge2GestureType::Spread;
		Result.Magnitude = Ratio - 1.0f;
	}

	SecondTouchPath.Empty();
	return Result;
}

void FForge2GestureRecognizer::Reset()
{
	TouchPath.Empty();
	SecondTouchPath.Empty();
	LastTapTime = 0.0f;
	LastTapPosition = FVector2D::ZeroVector;
}

EForge2GestureType FForge2GestureRecognizer::RecognizeGesture(const TArray<FForge2GesturePoint>& Path)
{
	if (Path.Num() < 2)
	{
		return EForge2GestureType::None;
	}

	// Check for circle
	if (IsCircle(Path))
	{
		return EForge2GestureType::Circle;
	}

	// Calculate swipe direction
	FVector2D Direction = CalculateSwipeDirection(Path);
	float Distance = FVector2D::Distance(Path[0].Position, Path.Last().Position);
	float Duration = Path.Last().Timestamp - Path[0].Timestamp;

	// Must have minimum distance
	if (Distance < 50.0f)
	{
		return EForge2GestureType::None;
	}

	// Must be quick enough
	if (Duration > 0.5f)
	{
		return EForge2GestureType::None;
	}

	// Determine direction
	float Angle = FMath::Atan2(Direction.Y, Direction.X) * (180.0f / PI);

	if (Angle >= -45.0f && Angle < 45.0f)
	{
		return EForge2GestureType::SwipeRight;
	}
	else if (Angle >= 45.0f && Angle < 135.0f)
	{
		return EForge2GestureType::SwipeDown;  // Y is inverted in screen space
	}
	else if (Angle >= -135.0f && Angle < -45.0f)
	{
		return EForge2GestureType::SwipeUp;
	}
	else
	{
		return EForge2GestureType::SwipeLeft;
	}
}

FVector2D FForge2GestureRecognizer::CalculateSwipeDirection(const TArray<FForge2GesturePoint>& Path)
{
	if (Path.Num() < 2)
	{
		return FVector2D::ZeroVector;
	}

	FVector2D Direction = Path.Last().Position - Path[0].Position;
	Direction.Normalize();
	return Direction;
}

bool FForge2GestureRecognizer::IsCircle(const TArray<FForge2GesturePoint>& Path)
{
	if (Path.Num() < 8)  // Need enough points
	{
		return false;
	}

	// Check if end is near start
	float EndToStartDistance = FVector2D::Distance(Path[0].Position, Path.Last().Position);
	if (EndToStartDistance > 100.0f)  // Not closed
	{
		return false;
	}

	// Calculate centroid
	FVector2D Centroid = FVector2D::ZeroVector;
	for (const FForge2GesturePoint& Point : Path)
	{
		Centroid += Point.Position;
	}
	Centroid /= Path.Num();

	// Check if points are roughly equidistant from centroid
	float AvgDistance = 0.0f;
	for (const FForge2GesturePoint& Point : Path)
	{
		AvgDistance += FVector2D::Distance(Point.Position, Centroid);
	}
	AvgDistance /= Path.Num();

	// Check variance
	float Variance = 0.0f;
	for (const FForge2GesturePoint& Point : Path)
	{
		float Dist = FVector2D::Distance(Point.Position, Centroid);
		Variance += FMath::Square(Dist - AvgDistance);
	}
	Variance /= Path.Num();

	// If variance is low relative to average distance, it's a circle
	return FMath::Sqrt(Variance) < AvgDistance * 0.3f;
}

//=============================================================================
// FForge2InputUtils Implementation
//=============================================================================

FString FForge2InputUtils::GetKeyDisplayName(const FKey& Key)
{
	return Key.GetDisplayName().ToString();
}

FString FForge2InputUtils::GetGamepadButtonIcon(const FKey& Key)
{
	// Return icon paths for common gamepad buttons
	if (Key == EKeys::Gamepad_FaceButton_Bottom) return TEXT("XboxA");
	if (Key == EKeys::Gamepad_FaceButton_Right) return TEXT("XboxB");
	if (Key == EKeys::Gamepad_FaceButton_Left) return TEXT("XboxX");
	if (Key == EKeys::Gamepad_FaceButton_Top) return TEXT("XboxY");
	if (Key == EKeys::Gamepad_LeftShoulder) return TEXT("LB");
	if (Key == EKeys::Gamepad_RightShoulder) return TEXT("RB");
	if (Key == EKeys::Gamepad_LeftTrigger) return TEXT("LT");
	if (Key == EKeys::Gamepad_RightTrigger) return TEXT("RT");
	return Key.GetDisplayName().ToString();
}

FVector2D FForge2InputUtils::GetDirectionFromInput(float AxisX, float AxisY, float DeadZone)
{
	FVector2D Input(AxisX, AxisY);
	float Magnitude = Input.Size();

	if (Magnitude < DeadZone)
	{
		return FVector2D::ZeroVector;
	}

	// Normalize and apply dead zone remapping
	Input.Normalize();
	float RemappedMagnitude = (Magnitude - DeadZone) / (1.0f - DeadZone);
	return Input * RemappedMagnitude;
}

EForge2FightingInputType FForge2InputUtils::GetNumpadDirection(FVector2D StickInput, float DeadZone)
{
	float Magnitude = StickInput.Size();
	if (Magnitude < DeadZone)
	{
		return EForge2FightingInputType::Neutral;
	}

	float Angle = FMath::Atan2(StickInput.Y, StickInput.X) * (180.0f / PI);

	// Map angle to numpad direction
	// Right = 0, Up = 90, Left = 180/-180, Down = -90
	if (Angle >= -22.5f && Angle < 22.5f) return EForge2FightingInputType::Forward;
	if (Angle >= 22.5f && Angle < 67.5f) return EForge2FightingInputType::UpForward;
	if (Angle >= 67.5f && Angle < 112.5f) return EForge2FightingInputType::Up;
	if (Angle >= 112.5f && Angle < 157.5f) return EForge2FightingInputType::UpBack;
	if (Angle >= 157.5f || Angle < -157.5f) return EForge2FightingInputType::Back;
	if (Angle >= -157.5f && Angle < -112.5f) return EForge2FightingInputType::DownBack;
	if (Angle >= -112.5f && Angle < -67.5f) return EForge2FightingInputType::Down;
	if (Angle >= -67.5f && Angle < -22.5f) return EForge2FightingInputType::DownForward;

	return EForge2FightingInputType::Neutral;
}

bool FForge2InputUtils::AreKeysEquivalent(const FKey& Key1, const FKey& Key2)
{
	return Key1 == Key2;
}

bool FForge2InputUtils::IsAxisInput(const FKey& Key)
{
	return Key.IsAxis1D() || Key.IsAxis2D() || Key.IsAxis3D();
}

bool FForge2InputUtils::IsGamepadKey(const FKey& Key)
{
	return Key.IsGamepadKey();
}

TArray<EForge2FightingInputType> FForge2InputUtils::ParseFightingNotation(const FString& Notation)
{
	TArray<EForge2FightingInputType> Result;

	for (int32 i = 0; i < Notation.Len(); i++)
	{
		TCHAR Char = Notation[i];

		switch (Char)
		{
		case '1': Result.Add(EForge2FightingInputType::DownBack); break;
		case '2': Result.Add(EForge2FightingInputType::Down); break;
		case '3': Result.Add(EForge2FightingInputType::DownForward); break;
		case '4': Result.Add(EForge2FightingInputType::Back); break;
		case '5': Result.Add(EForge2FightingInputType::Neutral); break;
		case '6': Result.Add(EForge2FightingInputType::Forward); break;
		case '7': Result.Add(EForge2FightingInputType::UpBack); break;
		case '8': Result.Add(EForge2FightingInputType::Up); break;
		case '9': Result.Add(EForge2FightingInputType::UpForward); break;
		case 'P': case 'p': Result.Add(EForge2FightingInputType::LightPunch); break;
		case 'K': case 'k': Result.Add(EForge2FightingInputType::LightKick); break;
		case 'S': case 's': Result.Add(EForge2FightingInputType::Special); break;
		default: break;
		}
	}

	return Result;
}

FString FForge2InputUtils::ToFightingNotation(const TArray<EForge2FightingInputType>& Inputs)
{
	FString Result;

	for (EForge2FightingInputType Input : Inputs)
	{
		switch (Input)
		{
		case EForge2FightingInputType::DownBack: Result += TEXT("1"); break;
		case EForge2FightingInputType::Down: Result += TEXT("2"); break;
		case EForge2FightingInputType::DownForward: Result += TEXT("3"); break;
		case EForge2FightingInputType::Back: Result += TEXT("4"); break;
		case EForge2FightingInputType::Neutral: Result += TEXT("5"); break;
		case EForge2FightingInputType::Forward: Result += TEXT("6"); break;
		case EForge2FightingInputType::UpBack: Result += TEXT("7"); break;
		case EForge2FightingInputType::Up: Result += TEXT("8"); break;
		case EForge2FightingInputType::UpForward: Result += TEXT("9"); break;
		case EForge2FightingInputType::LightPunch: Result += TEXT("P"); break;
		case EForge2FightingInputType::LightKick: Result += TEXT("K"); break;
		case EForge2FightingInputType::Special: Result += TEXT("S"); break;
		default: break;
		}
	}

	return Result;
}

FForge2FightingMotion FForge2InputUtils::CreateQCF(const FString& Name, const TArray<EForge2FightingInputType>& Buttons)
{
	FForge2FightingMotion Motion;
	Motion.MotionId = TEXT("qcf_") + Name;
	Motion.MotionName = TEXT("Quarter Circle Forward - ") + Name;
	Motion.DirectionSequence = {
		EForge2FightingInputType::Down,
		EForge2FightingInputType::DownForward,
		EForge2FightingInputType::Forward
	};
	Motion.FinisherButtons = Buttons;
	Motion.MaxDuration = 0.5f;
	Motion.bAllowShortcuts = true;
	Motion.OutputAction = TEXT("special_qcf_") + Name;
	return Motion;
}

FForge2FightingMotion FForge2InputUtils::CreateQCB(const FString& Name, const TArray<EForge2FightingInputType>& Buttons)
{
	FForge2FightingMotion Motion;
	Motion.MotionId = TEXT("qcb_") + Name;
	Motion.MotionName = TEXT("Quarter Circle Back - ") + Name;
	Motion.DirectionSequence = {
		EForge2FightingInputType::Down,
		EForge2FightingInputType::DownBack,
		EForge2FightingInputType::Back
	};
	Motion.FinisherButtons = Buttons;
	Motion.MaxDuration = 0.5f;
	Motion.bAllowShortcuts = true;
	Motion.OutputAction = TEXT("special_qcb_") + Name;
	return Motion;
}

FForge2FightingMotion FForge2InputUtils::CreateDP(const FString& Name, const TArray<EForge2FightingInputType>& Buttons)
{
	FForge2FightingMotion Motion;
	Motion.MotionId = TEXT("dp_") + Name;
	Motion.MotionName = TEXT("Dragon Punch - ") + Name;
	Motion.DirectionSequence = {
		EForge2FightingInputType::Forward,
		EForge2FightingInputType::Down,
		EForge2FightingInputType::DownForward
	};
	Motion.FinisherButtons = Buttons;
	Motion.MaxDuration = 0.4f;
	Motion.bAllowShortcuts = true;
	Motion.OutputAction = TEXT("special_dp_") + Name;
	return Motion;
}

FForge2FightingMotion FForge2InputUtils::CreateHCF(const FString& Name, const TArray<EForge2FightingInputType>& Buttons)
{
	FForge2FightingMotion Motion;
	Motion.MotionId = TEXT("hcf_") + Name;
	Motion.MotionName = TEXT("Half Circle Forward - ") + Name;
	Motion.DirectionSequence = {
		EForge2FightingInputType::Back,
		EForge2FightingInputType::DownBack,
		EForge2FightingInputType::Down,
		EForge2FightingInputType::DownForward,
		EForge2FightingInputType::Forward
	};
	Motion.FinisherButtons = Buttons;
	Motion.MaxDuration = 0.6f;
	Motion.bAllowShortcuts = true;
	Motion.OutputAction = TEXT("special_hcf_") + Name;
	return Motion;
}

FForge2FightingMotion FForge2InputUtils::CreateHCB(const FString& Name, const TArray<EForge2FightingInputType>& Buttons)
{
	FForge2FightingMotion Motion;
	Motion.MotionId = TEXT("hcb_") + Name;
	Motion.MotionName = TEXT("Half Circle Back - ") + Name;
	Motion.DirectionSequence = {
		EForge2FightingInputType::Forward,
		EForge2FightingInputType::DownForward,
		EForge2FightingInputType::Down,
		EForge2FightingInputType::DownBack,
		EForge2FightingInputType::Back
	};
	Motion.FinisherButtons = Buttons;
	Motion.MaxDuration = 0.6f;
	Motion.bAllowShortcuts = true;
	Motion.OutputAction = TEXT("special_hcb_") + Name;
	return Motion;
}

FForge2FightingMotion FForge2InputUtils::Create360(const FString& Name, const TArray<EForge2FightingInputType>& Buttons)
{
	FForge2FightingMotion Motion;
	Motion.MotionId = TEXT("360_") + Name;
	Motion.MotionName = TEXT("360 - ") + Name;
	Motion.DirectionSequence = {
		EForge2FightingInputType::Forward,
		EForge2FightingInputType::DownForward,
		EForge2FightingInputType::Down,
		EForge2FightingInputType::DownBack,
		EForge2FightingInputType::Back,
		EForge2FightingInputType::UpBack,
		EForge2FightingInputType::Up
	};
	Motion.FinisherButtons = Buttons;
	Motion.MaxDuration = 0.5f;
	Motion.bAllowShortcuts = true;
	Motion.OutputAction = TEXT("special_360_") + Name;
	return Motion;
}

FForge2ChargeInput FForge2InputUtils::CreateChargeBack(const FString& Name, const TArray<EForge2FightingInputType>& Buttons, float ChargeTime)
{
	FForge2ChargeInput Charge;
	Charge.ChargeId = TEXT("charge_back_") + Name;
	Charge.ChargeDirection = EForge2FightingInputType::Back;
	Charge.ReleaseDirection = EForge2FightingInputType::Forward;
	Charge.FinisherButtons = Buttons;
	Charge.MinChargeTime = ChargeTime;
	Charge.MaxReleaseWindow = 0.2f;
	Charge.OutputAction = TEXT("special_charge_") + Name;
	return Charge;
}

FForge2ChargeInput FForge2InputUtils::CreateChargeDown(const FString& Name, const TArray<EForge2FightingInputType>& Buttons, float ChargeTime)
{
	FForge2ChargeInput Charge;
	Charge.ChargeId = TEXT("charge_down_") + Name;
	Charge.ChargeDirection = EForge2FightingInputType::Down;
	Charge.ReleaseDirection = EForge2FightingInputType::Up;
	Charge.FinisherButtons = Buttons;
	Charge.MinChargeTime = ChargeTime;
	Charge.MaxReleaseWindow = 0.2f;
	Charge.OutputAction = TEXT("special_flash_") + Name;
	return Charge;
}

float FForge2InputUtils::ApplyDeadZone(float Value, float InnerDeadZone, float OuterDeadZone)
{
	float AbsValue = FMath::Abs(Value);
	if (AbsValue < InnerDeadZone)
	{
		return 0.0f;
	}
	if (AbsValue > OuterDeadZone)
	{
		return FMath::Sign(Value);
	}

	float RemappedValue = (AbsValue - InnerDeadZone) / (OuterDeadZone - InnerDeadZone);
	return RemappedValue * FMath::Sign(Value);
}

FVector2D FForge2InputUtils::ApplyDeadZone2D(FVector2D Value, float InnerDeadZone, float OuterDeadZone)
{
	float Magnitude = Value.Size();
	if (Magnitude < InnerDeadZone)
	{
		return FVector2D::ZeroVector;
	}
	if (Magnitude > OuterDeadZone)
	{
		return Value.GetSafeNormal();
	}

	float RemappedMagnitude = (Magnitude - InnerDeadZone) / (OuterDeadZone - InnerDeadZone);
	return Value.GetSafeNormal() * RemappedMagnitude;
}

float FForge2InputUtils::ApplyResponseCurve(float Value, UCurveFloat* Curve)
{
	if (!Curve)
	{
		return Value;
	}

	float Sign = FMath::Sign(Value);
	float AbsValue = FMath::Abs(Value);
	return Curve->GetFloatValue(AbsValue) * Sign;
}
