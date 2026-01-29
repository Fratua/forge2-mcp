// Forge2PuzzleSystem.cpp
// FORGE2 Puzzle Genre System Implementation
// Provides puzzle mechanics: logic gates, physics puzzles, sequences, environmental puzzles

#include "Forge2PuzzleSystem.h"

// ============================================================================
// Logic Gate
// ============================================================================

bool FForge2LogicGate::Evaluate()
{
    if (Inputs.Num() == 0)
    {
        bOutputValue = false;
        return bOutputValue;
    }

    switch (GateType)
    {
        case EForge2LogicGateType::AND:
        {
            bOutputValue = true;
            for (const FForge2LogicInput& Input : Inputs)
            {
                bool Value = Input.bInverted ? !Input.bCurrentValue : Input.bCurrentValue;
                if (!Value)
                {
                    bOutputValue = false;
                    break;
                }
            }
            break;
        }

        case EForge2LogicGateType::OR:
        {
            bOutputValue = false;
            for (const FForge2LogicInput& Input : Inputs)
            {
                bool Value = Input.bInverted ? !Input.bCurrentValue : Input.bCurrentValue;
                if (Value)
                {
                    bOutputValue = true;
                    break;
                }
            }
            break;
        }

        case EForge2LogicGateType::NOT:
        {
            if (Inputs.Num() > 0)
            {
                bool Value = Inputs[0].bInverted ? !Inputs[0].bCurrentValue : Inputs[0].bCurrentValue;
                bOutputValue = !Value;
            }
            break;
        }

        case EForge2LogicGateType::XOR:
        {
            int32 TrueCount = 0;
            for (const FForge2LogicInput& Input : Inputs)
            {
                bool Value = Input.bInverted ? !Input.bCurrentValue : Input.bCurrentValue;
                if (Value) TrueCount++;
            }
            bOutputValue = (TrueCount % 2) == 1;
            break;
        }

        case EForge2LogicGateType::NAND:
        {
            bool AllTrue = true;
            for (const FForge2LogicInput& Input : Inputs)
            {
                bool Value = Input.bInverted ? !Input.bCurrentValue : Input.bCurrentValue;
                if (!Value)
                {
                    AllTrue = false;
                    break;
                }
            }
            bOutputValue = !AllTrue;
            break;
        }

        case EForge2LogicGateType::NOR:
        {
            bool AnyTrue = false;
            for (const FForge2LogicInput& Input : Inputs)
            {
                bool Value = Input.bInverted ? !Input.bCurrentValue : Input.bCurrentValue;
                if (Value)
                {
                    AnyTrue = true;
                    break;
                }
            }
            bOutputValue = !AnyTrue;
            break;
        }

        case EForge2LogicGateType::XNOR:
        {
            int32 TrueCount = 0;
            for (const FForge2LogicInput& Input : Inputs)
            {
                bool Value = Input.bInverted ? !Input.bCurrentValue : Input.bCurrentValue;
                if (Value) TrueCount++;
            }
            bOutputValue = (TrueCount % 2) == 0;
            break;
        }

        case EForge2LogicGateType::Buffer:
        {
            if (Inputs.Num() > 0)
            {
                bOutputValue = Inputs[0].bInverted ? !Inputs[0].bCurrentValue : Inputs[0].bCurrentValue;
            }
            break;
        }

        case EForge2LogicGateType::Toggle:
        {
            // Toggle on rising edge of first input
            if (Inputs.Num() > 0)
            {
                bool Value = Inputs[0].bInverted ? !Inputs[0].bCurrentValue : Inputs[0].bCurrentValue;
                static bool LastValue = false;
                if (Value && !LastValue)
                {
                    bOutputValue = !bOutputValue;
                }
                LastValue = Value;
            }
            break;
        }

        case EForge2LogicGateType::Counter:
        {
            // Handled in Update()
            break;
        }

        case EForge2LogicGateType::Delay:
        {
            // Handled in Update()
            break;
        }

        case EForge2LogicGateType::Pulse:
        {
            // Handled in Update()
            break;
        }

        case EForge2LogicGateType::Latch:
        {
            // SR Latch: First input = Set, Second input = Reset
            if (Inputs.Num() >= 2)
            {
                bool Set = Inputs[0].bInverted ? !Inputs[0].bCurrentValue : Inputs[0].bCurrentValue;
                bool Reset = Inputs[1].bInverted ? !Inputs[1].bCurrentValue : Inputs[1].bCurrentValue;

                if (Set && !Reset) bOutputValue = true;
                else if (!Set && Reset) bOutputValue = false;
                // Both high or both low = maintain state
            }
            break;
        }

        case EForge2LogicGateType::Comparator:
        {
            // Compare stored value with input count
            int32 TrueCount = 0;
            for (const FForge2LogicInput& Input : Inputs)
            {
                bool Value = Input.bInverted ? !Input.bCurrentValue : Input.bCurrentValue;
                if (Value) TrueCount++;
            }
            bOutputValue = (float)TrueCount >= StoredValue;
            break;
        }

        case EForge2LogicGateType::Memory:
        {
            // Store input value when trigger (second input) is high
            if (Inputs.Num() >= 2)
            {
                bool Trigger = Inputs[1].bInverted ? !Inputs[1].bCurrentValue : Inputs[1].bCurrentValue;
                if (Trigger)
                {
                    StoredValue = Inputs[0].bCurrentValue ? 1.0f : 0.0f;
                }
                bOutputValue = StoredValue > 0.5f;
            }
            break;
        }
    }

    return bOutputValue;
}

void FForge2LogicGate::Update(float DeltaTime)
{
    if (GateType == EForge2LogicGateType::Delay)
    {
        bool InputValue = false;
        if (Inputs.Num() > 0)
        {
            InputValue = Inputs[0].bInverted ? !Inputs[0].bCurrentValue : Inputs[0].bCurrentValue;
        }

        if (InputValue != bPendingValue)
        {
            bPendingValue = InputValue;
            DelayTimer = 0.0f;
        }
        else
        {
            DelayTimer += DeltaTime;
            if (DelayTimer >= DelayTime)
            {
                bOutputValue = bPendingValue;
            }
        }
    }
    else if (GateType == EForge2LogicGateType::Counter)
    {
        bool InputValue = false;
        if (Inputs.Num() > 0)
        {
            InputValue = Inputs[0].bInverted ? !Inputs[0].bCurrentValue : Inputs[0].bCurrentValue;
        }

        static bool LastInputValue = false;
        if (InputValue && !LastInputValue) // Rising edge
        {
            CurrentCount++;
            if (CurrentCount >= CounterTarget)
            {
                bOutputValue = true;
            }
        }
        LastInputValue = InputValue;
    }
    else if (GateType == EForge2LogicGateType::Pulse)
    {
        bool InputValue = false;
        if (Inputs.Num() > 0)
        {
            InputValue = Inputs[0].bInverted ? !Inputs[0].bCurrentValue : Inputs[0].bCurrentValue;
        }

        static bool LastInputValue = false;
        if (InputValue && !LastInputValue) // Rising edge
        {
            bOutputValue = true;
            PulseTimer = PulseDuration;
        }
        LastInputValue = InputValue;

        if (PulseTimer > 0.0f)
        {
            PulseTimer -= DeltaTime;
            if (PulseTimer <= 0.0f)
            {
                bOutputValue = false;
            }
        }
    }
    else
    {
        Evaluate();
    }
}

void FForge2LogicGate::Reset()
{
    bOutputValue = false;
    DelayTimer = 0.0f;
    bPendingValue = false;
    CurrentCount = 0;
    PulseTimer = 0.0f;
    StoredValue = 0.0f;

    for (FForge2LogicInput& Input : Inputs)
    {
        Input.bCurrentValue = false;
    }
}

// ============================================================================
// Logic Network
// ============================================================================

void FForge2LogicNetwork::UpdateNetwork(float DeltaTime)
{
    // Update inputs from element states
    for (FForge2LogicGate& Gate : Gates)
    {
        for (FForge2LogicInput& Input : Gate.Inputs)
        {
            if (const bool* State = ElementStates.Find(Input.SourceElementID))
            {
                Input.bCurrentValue = *State;
            }
        }

        Gate.Update(DeltaTime);
    }
}

void FForge2LogicNetwork::SetElementState(FName ElementID, bool bState)
{
    ElementStates.Add(ElementID, bState);
}

bool FForge2LogicNetwork::GetElementState(FName ElementID) const
{
    if (const bool* State = ElementStates.Find(ElementID))
    {
        return *State;
    }
    return false;
}

bool FForge2LogicNetwork::GetGateOutput(FName GateID) const
{
    for (const FForge2LogicGate& Gate : Gates)
    {
        if (Gate.GateID == GateID)
        {
            return Gate.bOutputValue;
        }
    }
    return false;
}

void FForge2LogicNetwork::Reset()
{
    ElementStates.Empty();
    for (FForge2LogicGate& Gate : Gates)
    {
        Gate.Reset();
    }
}

// ============================================================================
// Puzzle Element
// ============================================================================

void FForge2PuzzleElement::Activate()
{
    if (CurrentState == EForge2PuzzleState::Locked) return;
    CurrentState = EForge2PuzzleState::Active;
}

void FForge2PuzzleElement::Deactivate()
{
    if (CurrentState == EForge2PuzzleState::Locked) return;
    CurrentState = EForge2PuzzleState::Inactive;
}

void FForge2PuzzleElement::Toggle()
{
    if (CurrentState == EForge2PuzzleState::Locked) return;

    if (CurrentState == EForge2PuzzleState::Active)
    {
        CurrentState = EForge2PuzzleState::Inactive;
    }
    else
    {
        CurrentState = EForge2PuzzleState::Active;
    }
}

bool FForge2PuzzleElement::CanInteract() const
{
    if (!bIsInteractable) return false;
    if (CurrentState == EForge2PuzzleState::Locked) return false;
    if (CurrentState == EForge2PuzzleState::Transitioning) return false;
    return true;
}

FVector FForge2PuzzleElement::GetTargetLocation() const
{
    if (CurrentState == EForge2PuzzleState::Active)
    {
        return Location + MovementRange;
    }
    return Location;
}

FRotator FForge2PuzzleElement::GetTargetRotation() const
{
    if (CurrentState == EForge2PuzzleState::Active)
    {
        FRotator Target = Rotation;
        Target.Yaw += RotationAngle;
        return Target;
    }
    return Rotation;
}

// ============================================================================
// Physics Object
// ============================================================================

void FForge2PhysicsObject::ApplyForce(const FVector& Force)
{
    // F = ma, a = F/m
    FVector Acceleration = Force / Mass;
    Velocity += Acceleration * 0.016f; // Assuming 60fps
}

void FForge2PhysicsObject::ApplyImpulse(const FVector& Impulse)
{
    // Impulse directly changes velocity
    Velocity += Impulse / Mass;
}

float FForge2PhysicsObject::GetBuoyancyForce(float WaterDensity, float WaterLevel) const
{
    // Check if submerged
    float BottomZ = Location.Z - Size.Z * 0.5f;
    float TopZ = Location.Z + Size.Z * 0.5f;

    if (BottomZ >= WaterLevel) return 0.0f; // Above water

    float SubmergedHeight = FMath::Clamp(WaterLevel - BottomZ, 0.0f, Size.Z);
    float SubmergedRatio = SubmergedHeight / Size.Z;

    // Buoyancy = density_water * gravity * volume_displaced
    float Volume = Size.X * Size.Y * Size.Z * SubmergedRatio;
    float BuoyancyForce = WaterDensity * 980.0f * Volume;

    // Subtract weight
    float Weight = Mass * 980.0f;

    return BuoyancyForce - Weight * SubmergedRatio;
}

float FForge2PhysicsObject::GetMagneticForce(const FForge2PhysicsObject& Other, float MagneticStrength) const
{
    if (!bIsMagnetic || !Other.bIsMagnetic) return 0.0f;

    float Distance = FVector::Distance(Location, Other.Location);
    if (Distance < 1.0f) Distance = 1.0f;

    // Magnetic force falls off with square of distance
    // Same polarity = repel, opposite = attract
    float PolarityProduct = MagneticPolarity * Other.MagneticPolarity;
    float Force = -PolarityProduct * MagneticStrength / (Distance * Distance);

    return Force;
}

// ============================================================================
// Physics Puzzle
// ============================================================================

bool FForge2PhysicsPuzzle::CheckSolved() const
{
    if (PuzzleType == EForge2PhysicsPuzzleType::Balance)
    {
        // Check if balance is within threshold using torque calculation
        float LeftTorque = 0.0f;
        float RightTorque = 0.0f;
        
        // Calculate the balance point (fulcrum) - use center of bounds or specified point
        FVector FulcrumPoint = BalanceFulcrumLocation;
        
        for (const FForge2PhysicsObject& Obj : Objects)
        {
            // Calculate horizontal distance from fulcrum (lever arm)
            float LeverArm = Obj.Location.X - FulcrumPoint.X;
            
            // Torque = Force * Distance = Mass * Gravity * LeverArm
            float Torque = Obj.Mass * 980.0f * FMath::Abs(LeverArm);
            
            if (LeverArm < 0.0f)
            {
                // Object is on the left side of fulcrum
                LeftTorque += Torque;
            }
            else if (LeverArm > 0.0f)
            {
                // Object is on the right side of fulcrum
                RightTorque += Torque;
            }
            // Objects directly on fulcrum contribute no torque
        }
        
        // Calculate the difference in torques
        float TorqueDifference = FMath::Abs(LeftTorque - RightTorque);
        float TotalTorque = LeftTorque + RightTorque;
        
        // Check if balance is within threshold
        // Use relative difference to handle different mass scales
        if (TotalTorque > 0.0f)
        {
            float BalanceRatio = TorqueDifference / TotalTorque;
            if (BalanceRatio > BalanceThreshold)
            {
                return false; // Not balanced
            }
        }
        
        // Balance is achieved
        return true;
    }

    // Check all required objects are in target zones
    if (RequiredObjectPositions.Num() != TargetZones.Num())
    {
        return RequiredObjectPositions.Num() == 0;
    }

    for (int32 i = 0; i < RequiredObjectPositions.Num(); i++)
    {
        FName RequiredID = RequiredObjectPositions[i];
        FVector TargetZone = TargetZones[i];

        bool Found = false;
        for (const FForge2PhysicsObject& Obj : Objects)
        {
            if (Obj.ObjectID == RequiredID)
            {
                float Distance = FVector::Distance(Obj.Location, TargetZone);
                if (Distance <= TargetZoneRadius)
                {
                    Found = true;
                    break;
                }
            }
        }

        if (!Found) return false;
    }

    return true;
}

void FForge2PhysicsPuzzle::UpdatePhysics(float DeltaTime)
{
    for (FForge2PhysicsObject& Obj : Objects)
    {
        // Apply gravity
        if (Obj.bAffectedByGravity)
        {
            FVector GravityForce = GravityDirection * GravityStrength * Obj.Mass;
            Obj.ApplyForce(GravityForce);
        }

        // Apply wind
        if (Obj.bAffectedByWind && WindStrength > 0.0f)
        {
            FVector WindForce = WindDirection * WindStrength;
            Obj.ApplyForce(WindForce);
        }

        // Apply buoyancy
        if (WaterLevel > 0.0f)
        {
            float BuoyancyForce = Obj.GetBuoyancyForce(WaterDensity, WaterLevel);
            if (BuoyancyForce != 0.0f)
            {
                Obj.ApplyForce(FVector(0, 0, BuoyancyForce));
            }
        }

        // Apply magnetic forces
        if (Obj.bIsMagnetic && MagneticFieldStrength > 0.0f)
        {
            for (const FForge2PhysicsObject& Other : Objects)
            {
                if (Other.ObjectID != Obj.ObjectID)
                {
                    float MagForce = Obj.GetMagneticForce(Other, MagneticFieldStrength);
                    if (MagForce != 0.0f)
                    {
                        FVector Direction = (Other.Location - Obj.Location).GetSafeNormal();
                        Obj.ApplyForce(Direction * MagForce);
                    }
                }
            }
        }

        // Update position
        Obj.Location += Obj.Velocity * DeltaTime;

        // Apply friction/damping
        Obj.Velocity *= (1.0f - Obj.Friction * DeltaTime);
    }
}

void FForge2PhysicsPuzzle::Reset()
{
    // Reset would restore original positions
    for (FForge2PhysicsObject& Obj : Objects)
    {
        Obj.Velocity = FVector::ZeroVector;
    }
}

// ============================================================================
// Sequence Puzzle
// ============================================================================

bool FForge2SequencePuzzle::ProcessInput(FName ElementID, float CurrentTime)
{
    if (State == EForge2PuzzleState::Solved || State == EForge2PuzzleState::Failed)
    {
        return false;
    }

    if (!Sequence.IsValidIndex(CurrentStep))
    {
        return false;
    }

    FForge2SequenceStep& Step = Sequence[CurrentStep];

    // Check time limit
    if (Step.MaxTimeToComplete > 0.0f && CurrentStep > 0)
    {
        float TimeSinceLastStep = CurrentTime - Sequence[CurrentStep - 1].CompletionTime;
        if (TimeSinceLastStep > Step.MaxTimeToComplete)
        {
            if (!bAllowMistakes)
            {
                State = EForge2PuzzleState::Failed;
                return false;
            }
            CurrentMistakes++;
            if (MaxMistakes > 0 && CurrentMistakes >= MaxMistakes)
            {
                State = EForge2PuzzleState::Failed;
                return false;
            }
        }
    }

    // Check if correct element
    if (Step.RequiredElementID == ElementID)
    {
        Step.bCompleted = true;
        Step.CompletionTime = CurrentTime;
        CurrentStep++;

        if (CheckSolved())
        {
            State = EForge2PuzzleState::Solved;
        }
        return true;
    }
    else
    {
        // Wrong element
        if (!bAllowMistakes)
        {
            State = EForge2PuzzleState::Failed;
            return false;
        }

        CurrentMistakes++;
        if (MaxMistakes > 0 && CurrentMistakes >= MaxMistakes)
        {
            State = EForge2PuzzleState::Failed;
        }
        return false;
    }
}

bool FForge2SequencePuzzle::CheckSolved() const
{
    return CurrentStep >= Sequence.Num();
}

void FForge2SequencePuzzle::Reset()
{
    CurrentStep = 0;
    CurrentMistakes = 0;
    State = EForge2PuzzleState::Inactive;

    for (FForge2SequenceStep& Step : Sequence)
    {
        Step.bCompleted = false;
        Step.CompletionTime = 0.0f;
    }
}

// ============================================================================
// Laser Beam
// ============================================================================

void FForge2LaserBeam::CalculatePath(const TArray<FForge2PuzzleElement>& Elements)
{
    PathPoints.Empty();
    HitReceptors.Empty();

    if (!bEnabled) return;

    PathPoints.Add(Origin);
    FVector CurrentPos = Origin;
    FVector CurrentDir = Direction.GetSafeNormal();
    int32 Reflections = 0;

    while (Reflections < MaxReflections)
    {
        // Find nearest hit
        float NearestDist = MaxLength;
        FForge2PuzzleElement* HitElement = nullptr;
        FVector HitPoint = CurrentPos + CurrentDir * MaxLength;

        for (const FForge2PuzzleElement& Element : Elements)
        {
            if (Element.ElementType == EForge2PuzzleElementType::Mirror ||
                Element.ElementType == EForge2PuzzleElementType::Prism ||
                Element.ElementType == EForge2PuzzleElementType::Receptor)
            {
                // Simple sphere collision for demonstration
                float Dist = FVector::Distance(CurrentPos, Element.Location);
                if (Dist < NearestDist && Dist > 10.0f)
                {
                    // Check if element is in direction of beam
                    FVector ToElement = (Element.Location - CurrentPos).GetSafeNormal();
                    if (FVector::DotProduct(CurrentDir, ToElement) > 0.9f)
                    {
                        NearestDist = Dist;
                        HitElement = const_cast<FForge2PuzzleElement*>(&Element);
                        HitPoint = Element.Location;
                    }
                }
            }
        }

        PathPoints.Add(HitPoint);

        if (!HitElement)
        {
            break; // Beam ends
        }

        if (HitElement->ElementType == EForge2PuzzleElementType::Receptor)
        {
            HitReceptors.Add(HitElement->ElementID);
            break; // Beam absorbed
        }
        else if (HitElement->ElementType == EForge2PuzzleElementType::Mirror)
        {
            // Reflect beam
            FVector Normal = HitElement->Rotation.Vector();
            CurrentDir = CurrentDir - 2.0f * FVector::DotProduct(CurrentDir, Normal) * Normal;
            CurrentPos = HitPoint;
            Reflections++;
        }
        else if (HitElement->ElementType == EForge2PuzzleElementType::Prism)
        {
            // Split beam (simplified - just change color and continue)
            CurrentPos = HitPoint;
            Reflections++;
        }
    }
}

bool FForge2LaserBeam::HitsReceptor(FName ReceptorID) const
{
    return HitReceptors.Contains(ReceptorID);
}

// ============================================================================
// Laser Puzzle
// ============================================================================

void FForge2LaserPuzzle::UpdateBeams()
{
    TArray<FForge2PuzzleElement> AllElements;
    AllElements.Append(Mirrors);
    AllElements.Append(Prisms);

    // Add receptors as elements for hit detection
    // (In full implementation, receptors would be separate)

    for (FForge2LaserBeam& Beam : Beams)
    {
        Beam.CalculatePath(AllElements);
    }
}

bool FForge2LaserPuzzle::CheckSolved() const
{
    for (FName ReceptorID : RequiredReceptors)
    {
        bool Hit = false;
        for (const FForge2LaserBeam& Beam : Beams)
        {
            if (Beam.HitsReceptor(ReceptorID))
            {
                if (bColorMatching)
                {
                    // Check color match
                    if (const FLinearColor* RequiredColor = ReceptorColors.Find(ReceptorID))
                    {
                        if (Beam.Color.Equals(*RequiredColor, 0.1f))
                        {
                            Hit = true;
                            break;
                        }
                    }
                }
                else
                {
                    Hit = true;
                    break;
                }
            }
        }

        if (!Hit) return false;
    }

    return true;
}

void FForge2LaserPuzzle::RotateMirror(FName MirrorID, float Angle)
{
    for (FForge2PuzzleElement& Mirror : Mirrors)
    {
        if (Mirror.ElementID == MirrorID)
        {
            Mirror.Rotation.Yaw += Angle;
            break;
        }
    }

    UpdateBeams();
}

// ============================================================================
// Environmental Condition
// ============================================================================

bool FForge2EnvironmentalCondition::CheckCondition(const FVector& PlayerPos, const FRotator& PlayerRot, float CurrentTime, FName CurrentWeather) const
{
    if (ConditionType == TEXT("perspective") || ConditionType == TEXT("position"))
    {
        float Distance = FVector::Distance(PlayerPos, RequiredPlayerPosition);
        if (Distance > PositionTolerance) return false;

        float AngleDiff = FMath::Abs(PlayerRot.Yaw - RequiredPlayerRotation.Yaw);
        if (AngleDiff > 180.0f) AngleDiff = 360.0f - AngleDiff;
        if (AngleDiff > RotationTolerance) return false;

        return true;
    }
    else if (ConditionType == TEXT("time"))
    {
        float TimeDiff = FMath::Abs(CurrentTime - RequiredTimeOfDay);
        if (TimeDiff > 12.0f) TimeDiff = 24.0f - TimeDiff;
        return TimeDiff <= TimeTolerance;
    }
    else if (ConditionType == TEXT("weather"))
    {
        return CurrentWeather == RequiredWeather;
    }

    return false;
}

// ============================================================================
// Environmental Puzzle
// ============================================================================

bool FForge2EnvironmentalPuzzle::CheckSolved(const FVector& PlayerPos, const FRotator& PlayerRot, float CurrentTime, FName CurrentWeather, float DeltaTime)
{
    bool ConditionsMet = false;

    if (bAllConditionsRequired)
    {
        ConditionsMet = true;
        for (const FForge2EnvironmentalCondition& Condition : Conditions)
        {
            if (!Condition.CheckCondition(PlayerPos, PlayerRot, CurrentTime, CurrentWeather))
            {
                ConditionsMet = false;
                break;
            }
        }
    }
    else
    {
        for (const FForge2EnvironmentalCondition& Condition : Conditions)
        {
            if (Condition.CheckCondition(PlayerPos, PlayerRot, CurrentTime, CurrentWeather))
            {
                ConditionsMet = true;
                break;
            }
        }
    }

    if (ConditionsMet)
    {
        if (HoldDuration > 0.0f)
        {
            CurrentHoldTime += DeltaTime;
            if (CurrentHoldTime >= HoldDuration)
            {
                State = EForge2PuzzleState::Solved;
                return true;
            }
        }
        else
        {
            State = EForge2PuzzleState::Solved;
            return true;
        }
    }
    else
    {
        CurrentHoldTime = 0.0f;
    }

    return false;
}

void FForge2EnvironmentalPuzzle::Reset()
{
    State = EForge2PuzzleState::Inactive;
    CurrentHoldTime = 0.0f;
}

// ============================================================================
// Hint
// ============================================================================

bool FForge2PuzzleHint::CanShow(float CurrentTime) const
{
    return (CurrentTime - LastShownTime) >= CooldownTime;
}

void FForge2PuzzleHint::Show(float CurrentTime)
{
    LastShownTime = CurrentTime;
}

// ============================================================================
// Hint System
// ============================================================================

void FForge2HintSystem::Update(float DeltaTime, bool bMadeProgress)
{
    if (bMadeProgress)
    {
        TimeSinceLastProgress = 0.0f;
    }
    else
    {
        TimeSinceLastProgress += DeltaTime;
    }
}

FForge2PuzzleHint* FForge2HintSystem::GetNextHint(float CurrentTime)
{
    if (!bAutoHints) return nullptr;
    if (TimeSinceLastProgress < HintTriggerTime) return nullptr;

    if (bProgressiveHints)
    {
        if (Hints.IsValidIndex(CurrentHintIndex))
        {
            FForge2PuzzleHint& Hint = Hints[CurrentHintIndex];
            if (Hint.CanShow(CurrentTime))
            {
                Hint.Show(CurrentTime);
                CurrentHintIndex++;
                return &Hint;
            }
        }
    }
    else
    {
        // Show any available hint
        for (FForge2PuzzleHint& Hint : Hints)
        {
            if (Hint.CanShow(CurrentTime))
            {
                Hint.Show(CurrentTime);
                return &Hint;
            }
        }
    }

    return nullptr;
}

void FForge2HintSystem::RequestHint(float CurrentTime)
{
    ManualHintsUsed++;
    TimeSinceLastProgress = HintTriggerTime; // Force hint availability
}

void FForge2HintSystem::ResetProgress()
{
    CurrentHintIndex = 0;
    TimeSinceLastProgress = 0.0f;
}

// ============================================================================
// Puzzle Progress
// ============================================================================

int32 FForge2PuzzleProgress::GetStarRating(float ParTime) const
{
    if (!bSolved) return 0;

    // 3 stars: under par time, no hints
    // 2 stars: under 2x par time or some hints
    // 1 star: solved

    if (SolveTime <= ParTime && HintsUsed == 0) return 3;
    if (SolveTime <= ParTime * 2.0f && HintsUsed <= 1) return 2;
    return 1;
}

// ============================================================================
// Player State
// ============================================================================

void FForge2PuzzlePlayerState::RecordSolve(FName PuzzleID, float SolveTime, int32 HintsUsed)
{
    FForge2PuzzleProgress Progress;
    if (FForge2PuzzleProgress* Existing = PuzzleProgress.Find(PuzzleID))
    {
        Progress = *Existing;
    }

    Progress.PuzzleID = PuzzleID;
    Progress.bSolved = true;
    Progress.AttemptCount++;

    // Record best time
    if (Progress.SolveTime == 0.0f || SolveTime < Progress.SolveTime)
    {
        Progress.SolveTime = SolveTime;
    }

    Progress.HintsUsed = FMath::Min(Progress.HintsUsed, HintsUsed);

    PuzzleProgress.Add(PuzzleID, Progress);
    TotalPuzzlesSolved++;
    TotalHintsUsed += HintsUsed;
}

bool FForge2PuzzlePlayerState::HasSolved(FName PuzzleID) const
{
    if (const FForge2PuzzleProgress* Progress = PuzzleProgress.Find(PuzzleID))
    {
        return Progress->bSolved;
    }
    return false;
}

FForge2PuzzleProgress FForge2PuzzlePlayerState::GetProgress(FName PuzzleID) const
{
    if (const FForge2PuzzleProgress* Progress = PuzzleProgress.Find(PuzzleID))
    {
        return *Progress;
    }
    return FForge2PuzzleProgress();
}

// ============================================================================
// Puzzle Manager
// ============================================================================

UForge2PuzzleManager::UForge2PuzzleManager()
{
    CurrentTime = 0.0f;
    PuzzleStartTime = 0.0f;
}

void UForge2PuzzleManager::InitializePuzzle(const FForge2PuzzleDefinition& Puzzle)
{
    CurrentPuzzle = Puzzle;
    CurrentPuzzle.OverallState = EForge2PuzzleState::Inactive;
    PuzzleStartTime = CurrentTime;

    // Initialize logic network
    CurrentPuzzle.LogicNetwork.Reset();

    // Reset all sub-puzzles
    for (FForge2PhysicsPuzzle& PP : CurrentPuzzle.PhysicsPuzzles)
    {
        PP.Reset();
    }

    for (FForge2SequencePuzzle& SP : CurrentPuzzle.SequencePuzzles)
    {
        SP.Reset();
    }

    for (FForge2EnvironmentalPuzzle& EP : CurrentPuzzle.EnvironmentalPuzzles)
    {
        EP.Reset();
    }

    CurrentPuzzle.HintSystem.ResetProgress();
}

void UForge2PuzzleManager::UpdatePuzzle(float DeltaTime)
{
    CurrentTime += DeltaTime;

    if (CurrentPuzzle.OverallState == EForge2PuzzleState::Solved ||
        CurrentPuzzle.OverallState == EForge2PuzzleState::Locked)
    {
        return;
    }

    UpdateLogicNetwork(DeltaTime);
    UpdatePhysicsPuzzles(DeltaTime);
    UpdateLaserPuzzles();

    // Update hint system
    bool bMadeProgress = false; // Would track actual progress
    CurrentPuzzle.HintSystem.Update(DeltaTime, bMadeProgress);

    CheckOverallSolved();
}

void UForge2PuzzleManager::UpdateLogicNetwork(float DeltaTime)
{
    // Update element states in network
    for (const FForge2PuzzleElement& Element : CurrentPuzzle.Elements)
    {
        bool bActive = Element.CurrentState == EForge2PuzzleState::Active;
        CurrentPuzzle.LogicNetwork.SetElementState(Element.ElementID, bActive);
    }

    CurrentPuzzle.LogicNetwork.UpdateNetwork(DeltaTime);

    // Apply gate outputs to connected elements
    for (FForge2PuzzleElement& Element : CurrentPuzzle.Elements)
    {
        if (Element.LogicGateID != NAME_None)
        {
            bool bGateOutput = CurrentPuzzle.LogicNetwork.GetGateOutput(Element.LogicGateID);
            if (bGateOutput && Element.CurrentState != EForge2PuzzleState::Active)
            {
                Element.Activate();
            }
            else if (!bGateOutput && Element.CurrentState != EForge2PuzzleState::Inactive)
            {
                Element.Deactivate();
            }
        }
    }
}

void UForge2PuzzleManager::UpdatePhysicsPuzzles(float DeltaTime)
{
    for (FForge2PhysicsPuzzle& Puzzle : CurrentPuzzle.PhysicsPuzzles)
    {
        Puzzle.UpdatePhysics(DeltaTime);
    }
}

void UForge2PuzzleManager::UpdateSequencePuzzles(float DeltaTime)
{
    // Update timed sequence puzzles and check for timeout failures
    for (FForge2SequencePuzzle& Puzzle : CurrentPuzzle.SequencePuzzles)
    {
        // Skip puzzles that are already solved or failed
        if (Puzzle.State == EForge2PuzzleState::Solved || Puzzle.State == EForge2PuzzleState::Failed)
        {
            continue;
        }
        
        // Check for overall puzzle timeout
        if (Puzzle.TotalTimeLimit > 0.0f)
        {
            Puzzle.TotalTimeElapsed += DeltaTime;
            if (Puzzle.TotalTimeElapsed >= Puzzle.TotalTimeLimit)
            {
                Puzzle.State = EForge2PuzzleState::Failed;
                OnPuzzleFailed();
                continue;
            }
        }
        
        // Update current step timeout
        if (Puzzle.Sequence.IsValidIndex(Puzzle.CurrentStep))
        {
            FForge2SequenceStep& CurrentStep = Puzzle.Sequence[Puzzle.CurrentStep];
            
            if (CurrentStep.MaxTimeToComplete > 0.0f && !CurrentStep.bCompleted)
            {
                CurrentStep.TimeElapsed += DeltaTime;
                
                // Check if step timed out
                if (CurrentStep.TimeElapsed >= CurrentStep.MaxTimeToComplete)
                {
                    if (!Puzzle.bAllowMistakes)
                    {
                        Puzzle.State = EForge2PuzzleState::Failed;
                        OnPuzzleFailed();
                    }
                    else
                    {
                        // Count timeout as a mistake
                        Puzzle.CurrentMistakes++;
                        
                        if (Puzzle.MaxMistakes > 0 && Puzzle.CurrentMistakes >= Puzzle.MaxMistakes)
                        {
                            Puzzle.State = EForge2PuzzleState::Failed;
                            OnPuzzleFailed();
                        }
                        else
                        {
                            // Reset current step timer for retry
                            CurrentStep.TimeElapsed = 0.0f;
                        }
                    }
                }
            }
        }
        
        // Handle pattern playback for Simon-says style puzzles
        if (Puzzle.bShowPattern && !Puzzle.bPatternShown)
        {
            Puzzle.PatternPlaybackTimer += DeltaTime;
            
            // Calculate which step in pattern to highlight
            float StepDuration = Puzzle.PatternStepDuration > 0.0f ? Puzzle.PatternStepDuration : 0.5f;
            int32 PatternStep = FMath::FloorToInt(Puzzle.PatternPlaybackTimer / StepDuration);
            
            if (PatternStep < Puzzle.Sequence.Num())
            {
                // Current step being shown
                Puzzle.CurrentPatternStep = PatternStep;
            }
            else
            {
                // Pattern playback complete
                Puzzle.bPatternShown = true;
                Puzzle.CurrentPatternStep = -1;
                Puzzle.PatternPlaybackTimer = 0.0f;
            }
        }
        
        // Auto-progress for timed sequence puzzles with auto-advance
        if (Puzzle.bAutoAdvance && Puzzle.Sequence.IsValidIndex(Puzzle.CurrentStep))
        {
            FForge2SequenceStep& CurrentStep = Puzzle.Sequence[Puzzle.CurrentStep];
            
            if (CurrentStep.bCompleted && CurrentStep.AutoAdvanceDelay > 0.0f)
            {
                CurrentStep.PostCompletionTimer += DeltaTime;
                
                if (CurrentStep.PostCompletionTimer >= CurrentStep.AutoAdvanceDelay)
                {
                    // Auto-advance to next step (visual feedback)
                    // The actual step progression happens in ProcessInput
                }
            }
        }
    }
}

void UForge2PuzzleManager::UpdateLaserPuzzles()
{
    for (FForge2LaserPuzzle& Puzzle : CurrentPuzzle.LaserPuzzles)
    {
        Puzzle.UpdateBeams();
    }
}

void UForge2PuzzleManager::UpdateEnvironmentalPuzzles(float DeltaTime, const FVector& PlayerPos, const FRotator& PlayerRot)
{
    for (FForge2EnvironmentalPuzzle& Puzzle : CurrentPuzzle.EnvironmentalPuzzles)
    {
        Puzzle.CheckSolved(PlayerPos, PlayerRot, CurrentTime, NAME_None, DeltaTime);
    }
}

bool UForge2PuzzleManager::InteractWithElement(FName ElementID)
{
    for (FForge2PuzzleElement& Element : CurrentPuzzle.Elements)
    {
        if (Element.ElementID == ElementID && Element.CanInteract())
        {
            Element.Toggle();

            // Process sequence puzzles
            for (FForge2SequencePuzzle& Puzzle : CurrentPuzzle.SequencePuzzles)
            {
                Puzzle.ProcessInput(ElementID, CurrentTime);
            }

            return true;
        }
    }
    return false;
}

bool UForge2PuzzleManager::SetElementState(FName ElementID, bool bActive)
{
    for (FForge2PuzzleElement& Element : CurrentPuzzle.Elements)
    {
        if (Element.ElementID == ElementID)
        {
            if (bActive)
            {
                Element.Activate();
            }
            else
            {
                Element.Deactivate();
            }
            return true;
        }
    }
    return false;
}

void UForge2PuzzleManager::RotateElement(FName ElementID, float Angle)
{
    for (FForge2PuzzleElement& Element : CurrentPuzzle.Elements)
    {
        if (Element.ElementID == ElementID)
        {
            Element.Rotation.Yaw += Angle;
            break;
        }
    }

    // Update laser puzzles if mirrors changed
    for (FForge2LaserPuzzle& Puzzle : CurrentPuzzle.LaserPuzzles)
    {
        Puzzle.RotateMirror(ElementID, Angle);
    }
}

bool UForge2PuzzleManager::GrabObject(FName ObjectID)
{
    PlayerState.CurrentHeldObject = ObjectID;
    return true;
}

void UForge2PuzzleManager::ReleaseObject()
{
    PlayerState.CurrentHeldObject = NAME_None;
}

void UForge2PuzzleManager::ThrowObject(const FVector& Direction, float Force)
{
    if (PlayerState.CurrentHeldObject == NAME_None) return;

    for (FForge2PhysicsPuzzle& Puzzle : CurrentPuzzle.PhysicsPuzzles)
    {
        for (FForge2PhysicsObject& Obj : Puzzle.Objects)
        {
            if (Obj.ObjectID == PlayerState.CurrentHeldObject)
            {
                Obj.ApplyImpulse(Direction * Force);
                break;
            }
        }
    }

    PlayerState.CurrentHeldObject = NAME_None;
}

FForge2PuzzleElement UForge2PuzzleManager::GetElement(FName ElementID) const
{
    for (const FForge2PuzzleElement& Element : CurrentPuzzle.Elements)
    {
        if (Element.ElementID == ElementID)
        {
            return Element;
        }
    }
    return FForge2PuzzleElement();
}

bool UForge2PuzzleManager::IsElementActive(FName ElementID) const
{
    for (const FForge2PuzzleElement& Element : CurrentPuzzle.Elements)
    {
        if (Element.ElementID == ElementID)
        {
            return Element.CurrentState == EForge2PuzzleState::Active;
        }
    }
    return false;
}

bool UForge2PuzzleManager::IsPuzzleSolved() const
{
    return CurrentPuzzle.OverallState == EForge2PuzzleState::Solved;
}

FForge2PuzzleHint UForge2PuzzleManager::RequestHint()
{
    CurrentPuzzle.HintSystem.RequestHint(CurrentTime);
    FForge2PuzzleHint* Hint = CurrentPuzzle.HintSystem.GetNextHint(CurrentTime);
    if (Hint)
    {
        return *Hint;
    }
    return FForge2PuzzleHint();
}

void UForge2PuzzleManager::SkipPuzzle()
{
    CurrentPuzzle.OverallState = EForge2PuzzleState::Solved;
    // Would mark as skipped rather than solved properly
}

void UForge2PuzzleManager::ResetPuzzle()
{
    InitializePuzzle(CurrentPuzzle);
}

void UForge2PuzzleManager::ResetElement(FName ElementID)
{
    for (FForge2PuzzleElement& Element : CurrentPuzzle.Elements)
    {
        if (Element.ElementID == ElementID)
        {
            Element.Deactivate();
            break;
        }
    }
}

bool UForge2PuzzleManager::ValidateSolution()
{
    return CheckOverallSolved(), IsPuzzleSolved();
}

void UForge2PuzzleManager::CheckOverallSolved()
{
    bool AllSolved = true;

    // Check physics puzzles
    for (const FForge2PhysicsPuzzle& Puzzle : CurrentPuzzle.PhysicsPuzzles)
    {
        if (!Puzzle.CheckSolved())
        {
            AllSolved = false;
            break;
        }
    }

    // Check sequence puzzles
    if (AllSolved)
    {
        for (const FForge2SequencePuzzle& Puzzle : CurrentPuzzle.SequencePuzzles)
        {
            if (!Puzzle.CheckSolved())
            {
                AllSolved = false;
                break;
            }
        }
    }

    // Check laser puzzles
    if (AllSolved)
    {
        for (const FForge2LaserPuzzle& Puzzle : CurrentPuzzle.LaserPuzzles)
        {
            if (!Puzzle.CheckSolved())
            {
                AllSolved = false;
                break;
            }
        }
    }

    // Check environmental puzzles
    if (AllSolved)
    {
        for (const FForge2EnvironmentalPuzzle& Puzzle : CurrentPuzzle.EnvironmentalPuzzles)
        {
            if (Puzzle.State != EForge2PuzzleState::Solved)
            {
                AllSolved = false;
                break;
            }
        }
    }

    if (AllSolved && CurrentPuzzle.OverallState != EForge2PuzzleState::Solved)
    {
        OnPuzzleSolved();
    }
}

void UForge2PuzzleManager::OnPuzzleSolved()
{
    CurrentPuzzle.OverallState = EForge2PuzzleState::Solved;

    float SolveTime = CurrentTime - PuzzleStartTime;
    PlayerState.RecordSolve(CurrentPuzzle.PuzzleID, SolveTime, CurrentPuzzle.HintSystem.ManualHintsUsed);
}

void UForge2PuzzleManager::OnPuzzleFailed()
{
    CurrentPuzzle.OverallState = EForge2PuzzleState::Failed;

    switch (CurrentPuzzle.ResetMode)
    {
        case EForge2PuzzleResetMode::Automatic:
            ResetPuzzle();
            break;
        case EForge2PuzzleResetMode::Timed:
            // Would start timer
            break;
        default:
            break;
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

FForge2PuzzleElement UForge2PuzzleUtilities::CreateButton(const FVector& Location, FName OutputConnection)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("Button_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Button");
    Element.ElementType = EForge2PuzzleElementType::Button;
    Element.Location = Location;
    Element.OutputConnections.Add(OutputConnection);
    return Element;
}

FForge2PuzzleElement UForge2PuzzleUtilities::CreateLever(const FVector& Location, FName OutputConnection)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("Lever_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Lever");
    Element.ElementType = EForge2PuzzleElementType::Lever;
    Element.Location = Location;
    Element.OutputConnections.Add(OutputConnection);
    return Element;
}

FForge2PuzzleElement UForge2PuzzleUtilities::CreatePressurePlate(const FVector& Location, float RequiredWeight, FName OutputConnection)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("PressurePlate_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Pressure Plate");
    Element.ElementType = EForge2PuzzleElementType::PressurePlate;
    Element.Location = Location;
    Element.RequiredWeight = RequiredWeight;
    Element.OutputConnections.Add(OutputConnection);
    return Element;
}

FForge2PuzzleElement UForge2PuzzleUtilities::CreateDoor(const FVector& Location, const FVector& OpenOffset)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("Door_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Door");
    Element.ElementType = EForge2PuzzleElementType::Door;
    Element.Location = Location;
    Element.MovementRange = OpenOffset;
    Element.bIsInteractable = false; // Doors are activated by other elements
    return Element;
}

FForge2PuzzleElement UForge2PuzzleUtilities::CreateMovingPlatform(const FVector& Start, const FVector& End, float Speed)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("Platform_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Moving Platform");
    Element.ElementType = EForge2PuzzleElementType::Platform;
    Element.Location = Start;
    Element.MovementRange = End - Start;
    Element.MovementSpeed = Speed;
    return Element;
}

FForge2PuzzleElement UForge2PuzzleUtilities::CreateLaserEmitter(const FVector& Location, const FVector& Direction)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("Emitter_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Laser Emitter");
    Element.ElementType = EForge2PuzzleElementType::Emitter;
    Element.Location = Location;
    Element.LaserDirection = Direction;
    Element.CurrentState = EForge2PuzzleState::Active;
    return Element;
}

FForge2PuzzleElement UForge2PuzzleUtilities::CreateMirror(const FVector& Location, const FRotator& Rotation)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("Mirror_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Mirror");
    Element.ElementType = EForge2PuzzleElementType::Mirror;
    Element.Location = Location;
    Element.Rotation = Rotation;
    return Element;
}

FForge2PuzzleElement UForge2PuzzleUtilities::CreateReceptor(const FVector& Location)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("Receptor_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Laser Receptor");
    Element.ElementType = EForge2PuzzleElementType::Receptor;
    Element.Location = Location;
    Element.bIsInteractable = false;
    return Element;
}

FForge2PuzzleElement UForge2PuzzleUtilities::CreateTimedSwitch(const FVector& Location, float Duration, FName OutputConnection)
{
    FForge2PuzzleElement Element;
    Element.ElementID = FName(*FString::Printf(TEXT("TimedSwitch_%d"), FMath::Rand()));
    Element.DisplayName = TEXT("Timed Switch");
    Element.ElementType = EForge2PuzzleElementType::TimerSwitch;
    Element.Location = Location;
    Element.TimerDuration = Duration;
    Element.OutputConnections.Add(OutputConnection);
    return Element;
}

FForge2LogicGate UForge2PuzzleUtilities::CreateANDGate(const TArray<FName>& InputElements)
{
    FForge2LogicGate Gate;
    Gate.GateID = FName(*FString::Printf(TEXT("AND_%d"), FMath::Rand()));
    Gate.GateType = EForge2LogicGateType::AND;

    for (FName ElementID : InputElements)
    {
        FForge2LogicInput Input;
        Input.InputID = FName(*FString::Printf(TEXT("Input_%d"), FMath::Rand()));
        Input.SourceElementID = ElementID;
        Gate.Inputs.Add(Input);
    }

    return Gate;
}

FForge2LogicGate UForge2PuzzleUtilities::CreateORGate(const TArray<FName>& InputElements)
{
    FForge2LogicGate Gate;
    Gate.GateID = FName(*FString::Printf(TEXT("OR_%d"), FMath::Rand()));
    Gate.GateType = EForge2LogicGateType::OR;

    for (FName ElementID : InputElements)
    {
        FForge2LogicInput Input;
        Input.InputID = FName(*FString::Printf(TEXT("Input_%d"), FMath::Rand()));
        Input.SourceElementID = ElementID;
        Gate.Inputs.Add(Input);
    }

    return Gate;
}

FForge2LogicGate UForge2PuzzleUtilities::CreateDelayGate(FName InputElement, float Delay)
{
    FForge2LogicGate Gate;
    Gate.GateID = FName(*FString::Printf(TEXT("Delay_%d"), FMath::Rand()));
    Gate.GateType = EForge2LogicGateType::Delay;
    Gate.DelayTime = Delay;

    FForge2LogicInput Input;
    Input.InputID = FName(*FString::Printf(TEXT("Input_%d"), FMath::Rand()));
    Input.SourceElementID = InputElement;
    Gate.Inputs.Add(Input);

    return Gate;
}

FForge2LogicGate UForge2PuzzleUtilities::CreateCounterGate(FName InputElement, int32 TargetCount)
{
    FForge2LogicGate Gate;
    Gate.GateID = FName(*FString::Printf(TEXT("Counter_%d"), FMath::Rand()));
    Gate.GateType = EForge2LogicGateType::Counter;
    Gate.CounterTarget = TargetCount;

    FForge2LogicInput Input;
    Input.InputID = FName(*FString::Printf(TEXT("Input_%d"), FMath::Rand()));
    Input.SourceElementID = InputElement;
    Gate.Inputs.Add(Input);

    return Gate;
}

FForge2PhysicsObject UForge2PuzzleUtilities::CreateCube(const FVector& Location, float Size, float Mass)
{
    FForge2PhysicsObject Obj;
    Obj.ObjectID = FName(*FString::Printf(TEXT("Cube_%d"), FMath::Rand()));
    Obj.Location = Location;
    Obj.Size = FVector(Size);
    Obj.Mass = Mass;
    return Obj;
}

FForge2PhysicsObject UForge2PuzzleUtilities::CreateSphere(const FVector& Location, float Radius, float Mass)
{
    FForge2PhysicsObject Obj;
    Obj.ObjectID = FName(*FString::Printf(TEXT("Sphere_%d"), FMath::Rand()));
    Obj.Location = Location;
    Obj.Size = FVector(Radius * 2.0f);
    Obj.Mass = Mass;
    Obj.Restitution = 0.6f; // Bouncier
    return Obj;
}

FForge2PhysicsObject UForge2PuzzleUtilities::CreateMagneticObject(const FVector& Location, float Mass, float Polarity)
{
    FForge2PhysicsObject Obj;
    Obj.ObjectID = FName(*FString::Printf(TEXT("Magnetic_%d"), FMath::Rand()));
    Obj.Location = Location;
    Obj.Size = FVector(50.0f);
    Obj.Mass = Mass;
    Obj.bIsMagnetic = true;
    Obj.MagneticPolarity = Polarity;
    return Obj;
}

FForge2SequencePuzzle UForge2PuzzleUtilities::CreateSimonSaysPuzzle(const TArray<FName>& ElementSequence)
{
    FForge2SequencePuzzle Puzzle;
    Puzzle.PuzzleID = FName(*FString::Printf(TEXT("SimonSays_%d"), FMath::Rand()));
    Puzzle.bAllowMistakes = false;
    Puzzle.bShowProgress = true;
    Puzzle.bPlaybackOnSolve = true;

    for (int32 i = 0; i < ElementSequence.Num(); i++)
    {
        FForge2SequenceStep Step;
        Step.StepID = FName(*FString::Printf(TEXT("Step_%d"), i));
        Step.RequiredElementID = ElementSequence[i];
        Step.StepOrder = i;
        Puzzle.Sequence.Add(Step);
    }

    return Puzzle;
}

FForge2LaserPuzzle UForge2PuzzleUtilities::CreateBasicLaserPuzzle(const FVector& EmitterLocation, const FVector& ReceptorLocation)
{
    FForge2LaserPuzzle Puzzle;
    Puzzle.PuzzleID = FName(*FString::Printf(TEXT("Laser_%d"), FMath::Rand()));

    FForge2LaserBeam Beam;
    Beam.BeamID = FName(TEXT("MainBeam"));
    Beam.Origin = EmitterLocation;
    Beam.Direction = (ReceptorLocation - EmitterLocation).GetSafeNormal();
    Beam.Color = FLinearColor::Red;
    Puzzle.Beams.Add(Beam);

    FName ReceptorID = FName(*FString::Printf(TEXT("Receptor_%d"), FMath::Rand()));
    Puzzle.RequiredReceptors.Add(ReceptorID);

    return Puzzle;
}

FForge2PhysicsPuzzle UForge2PuzzleUtilities::CreateBalancePuzzle(const FVector& FulcrumLocation, float BalanceThreshold)
{
    FForge2PhysicsPuzzle Puzzle;
    Puzzle.PuzzleID = FName(*FString::Printf(TEXT("Balance_%d"), FMath::Rand()));
    Puzzle.PuzzleType = EForge2PhysicsPuzzleType::Balance;
    Puzzle.BalanceThreshold = BalanceThreshold;

    // Add default balance beam objects
    FForge2PhysicsObject LeftWeight = CreateCube(FulcrumLocation + FVector(-200, 0, 100), 50.0f, 20.0f);
    LeftWeight.ObjectID = FName(TEXT("LeftWeight"));

    FForge2PhysicsObject RightWeight = CreateCube(FulcrumLocation + FVector(200, 0, 100), 50.0f, 10.0f);
    RightWeight.ObjectID = FName(TEXT("RightWeight"));

    Puzzle.Objects.Add(LeftWeight);
    Puzzle.Objects.Add(RightWeight);

    return Puzzle;
}

FForge2PuzzleHint UForge2PuzzleUtilities::CreateSubtleHint(FName TargetElement)
{
    FForge2PuzzleHint Hint;
    Hint.HintID = FName(*FString::Printf(TEXT("SubtleHint_%d"), FMath::Rand()));
    Hint.HintLevel = EForge2HintLevel::Subtle;
    Hint.TargetElementID = TargetElement;
    Hint.bVisual = true;
    Hint.bAudio = false;
    Hint.Duration = 3.0f;
    return Hint;
}

FForge2PuzzleHint UForge2PuzzleUtilities::CreateTextHint(const FString& HintText)
{
    FForge2PuzzleHint Hint;
    Hint.HintID = FName(*FString::Printf(TEXT("TextHint_%d"), FMath::Rand()));
    Hint.HintLevel = EForge2HintLevel::Moderate;
    Hint.HintText = HintText;
    Hint.bVisual = false;
    Hint.Duration = 5.0f;
    return Hint;
}

FForge2PuzzleHint UForge2PuzzleUtilities::CreateSolutionHint(const FString& SolutionText)
{
    FForge2PuzzleHint Hint;
    Hint.HintID = FName(*FString::Printf(TEXT("SolutionHint_%d"), FMath::Rand()));
    Hint.HintLevel = EForge2HintLevel::Solution;
    Hint.HintText = SolutionText;
    Hint.bVisual = false;
    Hint.Duration = 10.0f;
    Hint.CooldownTime = 300.0f; // 5 minute cooldown for solution hints
    return Hint;
}
