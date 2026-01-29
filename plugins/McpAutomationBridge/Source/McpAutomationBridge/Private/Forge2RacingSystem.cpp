// Copyright 2025 FORGE2 Project. All Rights Reserved.
// Racing Genre System Implementation

#include "Forge2RacingSystem.h"

// ============================================================================
// ENGINE CONFIG
// ============================================================================

float FForge2EngineConfig::GetPowerAtRPM(float CurrentRPM) const
{
	// Clamp to valid range
	CurrentRPM = FMath::Clamp(CurrentRPM, IdleRPM, RedlineRPM);

	// Simple power curve approximation
	// Power increases until MaxHorsepowerRPM, then drops off
	float RPMRatio = CurrentRPM / MaxHorsepowerRPM;
	float PowerMultiplier;

	if (RPMRatio <= 1.0f)
	{
		// Linear increase to peak
		PowerMultiplier = RPMRatio;
	}
	else
	{
		// Gradual falloff after peak
		float OverRatio = (CurrentRPM - MaxHorsepowerRPM) / (RedlineRPM - MaxHorsepowerRPM);
		PowerMultiplier = 1.0f - (OverRatio * 0.2f);
	}

	float BasePower = MaxHorsepower * PowerMultiplier;

	// Apply turbo boost if present
	if (TurboBoost > 0.0f && CurrentRPM > MaxTorqueRPM)
	{
		float BoostRPMRatio = (CurrentRPM - MaxTorqueRPM) / (RedlineRPM - MaxTorqueRPM);
		BasePower *= (1.0f + TurboBoost * FMath::Min(BoostRPMRatio, 1.0f));
	}

	return BasePower;
}

// ============================================================================
// TRANSMISSION CONFIG
// ============================================================================

void FForge2TransmissionConfig::SetupDefaultGears(int32 NumGears)
{
	GearRatios.Empty();

	// Reverse
	GearRatios.Add(-3.5f);

	// Neutral
	GearRatios.Add(0.0f);

	// Forward gears (typical ratios for a sports car)
	if (NumGears >= 1) GearRatios.Add(3.5f);  // 1st
	if (NumGears >= 2) GearRatios.Add(2.2f);  // 2nd
	if (NumGears >= 3) GearRatios.Add(1.5f);  // 3rd
	if (NumGears >= 4) GearRatios.Add(1.1f);  // 4th
	if (NumGears >= 5) GearRatios.Add(0.85f); // 5th
	if (NumGears >= 6) GearRatios.Add(0.7f);  // 6th
	if (NumGears >= 7) GearRatios.Add(0.6f);  // 7th
}

float FForge2TransmissionConfig::GetTotalRatio(int32 CurrentGear) const
{
	if (CurrentGear < 0 || CurrentGear >= GearRatios.Num())
	{
		return 0.0f;
	}

	return GearRatios[CurrentGear] * FinalDriveRatio;
}

// ============================================================================
// TIRE CONFIG
// ============================================================================

float FForge2TireConfig::CalculateGrip(float Temperature, float WearPercentage) const
{
	float BaseGrip = GripCoefficient;

	// Temperature effect
	float TempDiff = FMath::Abs(Temperature - OptimalTemperature);
	if (TempDiff > OptimalTemperatureRange)
	{
		float TempPenalty = (TempDiff - OptimalTemperatureRange) / 50.0f;
		BaseGrip *= FMath::Max(0.5f, 1.0f - TempPenalty);
	}

	// Wear effect
	if (WearPercentage > 50.0f)
	{
		float WearPenalty = (WearPercentage - 50.0f) / 100.0f;
		BaseGrip *= FMath::Max(0.3f, 1.0f - WearPenalty);
	}

	return BaseGrip;
}

// ============================================================================
// TIRE RUNTIME STATE
// ============================================================================

EForge2TireCondition FForge2TireRuntimeState::GetCondition() const
{
	if (bPunctured || WearPercentage >= 100.0f)
	{
		return EForge2TireCondition::Destroyed;
	}
	if (WearPercentage >= 80.0f)
	{
		return EForge2TireCondition::Critical;
	}
	if (WearPercentage >= 50.0f)
	{
		return EForge2TireCondition::Worn;
	}
	if (WearPercentage >= 20.0f)
	{
		return EForge2TireCondition::Good;
	}
	return EForge2TireCondition::Fresh;
}

void FForge2TireRuntimeState::Update(float DeltaTime, float SlipRatio, float SlipAngle, const FForge2TireConfig& Config)
{
	if (bPunctured)
	{
		return;
	}

	// Temperature change based on slip
	float SlipMagnitude = FMath::Sqrt(SlipRatio * SlipRatio + SlipAngle * SlipAngle);
	float HeatGeneration = SlipMagnitude * 50.0f;
	float Cooling = (Temperature - 25.0f) * 0.1f;

	Temperature += (HeatGeneration - Cooling) * DeltaTime;
	Temperature = FMath::Clamp(Temperature, 10.0f, 150.0f);

	// Wear based on slip and temperature
	float WearRate = SlipMagnitude * Config.WearRateMultiplier;
	if (Temperature > Config.OptimalTemperature + Config.OptimalTemperatureRange)
	{
		WearRate *= 1.5f; // Higher wear when overheating
	}

	WearPercentage += WearRate * DeltaTime * 0.1f;
	WearPercentage = FMath::Min(WearPercentage, 100.0f);
}

// ============================================================================
// AERODYNAMICS CONFIG
// ============================================================================

float FForge2AerodynamicsConfig::CalculateDrag(float SpeedKph) const
{
	float SpeedMs = SpeedKph / 3.6f;
	float AirDensity = 1.225f; // kg/m^3

	return 0.5f * AirDensity * DragCoefficient * FrontalArea * SpeedMs * SpeedMs;
}

float FForge2AerodynamicsConfig::CalculateDownforce(float SpeedKph) const
{
	float SpeedMs = SpeedKph / 3.6f;
	float AirDensity = 1.225f;

	return 0.5f * AirDensity * DownforceCoefficient * FrontalArea * SpeedMs * SpeedMs;
}

// ============================================================================
// VEHICLE DEFINITION
// ============================================================================

void FForge2VehicleDefinition::SetupDefaultWheels()
{
	Suspensions.Empty();
	Tires.Empty();

	for (int32 i = 0; i < NumWheels; ++i)
	{
		FForge2SuspensionConfig Suspension;
		FForge2TireConfig Tire;

		// Front wheels typically have slightly different settings
		if (i < NumWheels / 2)
		{
			Suspension.SpringStiffness = 45000.0f;
			Tire.Width = 23.0f;
		}
		else
		{
			Suspension.SpringStiffness = 55000.0f;
			Tire.Width = 27.0f;
		}

		Suspensions.Add(Suspension);
		Tires.Add(Tire);
	}
}

float FForge2VehicleDefinition::CalculateTheoreticalTopSpeed() const
{
	if (Transmission.GearRatios.Num() < 3 || Tires.Num() == 0)
	{
		return TopSpeed;
	}

	// Get highest gear ratio
	float HighestGearRatio = Transmission.GearRatios.Last();
	float TotalRatio = HighestGearRatio * Transmission.FinalDriveRatio;

	if (TotalRatio <= 0.0f)
	{
		return TopSpeed;
	}

	// Calculate wheel circumference
	float WheelCircumference = 2.0f * PI * (Tires[0].Radius / 100.0f); // Convert cm to m

	// At redline RPM
	float WheelRPM = Engine.RedlineRPM / TotalRatio;
	float WheelSpeedMs = (WheelRPM * WheelCircumference) / 60.0f;
	float TopSpeedKph = WheelSpeedMs * 3.6f;

	return TopSpeedKph;
}

// ============================================================================
// COMPONENT DAMAGE
// ============================================================================

float FForge2ComponentDamage::GetPerformanceMultiplier() const
{
	// Different components affect performance differently
	switch (Component)
	{
	case EForge2VehicleComponent::Engine:
		return FMath::Max(0.3f, 1.0f - DamagePercentage * 0.007f);

	case EForge2VehicleComponent::Transmission:
		return FMath::Max(0.5f, 1.0f - DamagePercentage * 0.005f);

	case EForge2VehicleComponent::Suspension:
		return FMath::Max(0.6f, 1.0f - DamagePercentage * 0.004f);

	case EForge2VehicleComponent::Brakes:
		return FMath::Max(0.4f, 1.0f - DamagePercentage * 0.006f);

	case EForge2VehicleComponent::Steering:
		return FMath::Max(0.3f, 1.0f - DamagePercentage * 0.007f);

	case EForge2VehicleComponent::Aerodynamics:
		return FMath::Max(0.7f, 1.0f - DamagePercentage * 0.003f);

	default:
		return 1.0f;
	}
}

// ============================================================================
// VEHICLE RUNTIME STATE
// ============================================================================

bool FForge2VehicleRuntimeState::IsOperational() const
{
	if (CurrentHealth <= 0.0f)
	{
		return false;
	}

	// Check for critical component damage
	for (const FForge2ComponentDamage& Damage : ComponentDamage)
	{
		if (Damage.DamagePercentage >= 100.0f)
		{
			if (Damage.Component == EForge2VehicleComponent::Engine ||
				Damage.Component == EForge2VehicleComponent::Transmission)
			{
				return false;
			}
		}
	}

	// Check for all tires destroyed
	int32 DestroyedTires = 0;
	for (const FForge2TireRuntimeState& Tire : TireStates)
	{
		if (Tire.GetCondition() == EForge2TireCondition::Destroyed)
		{
			DestroyedTires++;
		}
	}

	// Can't drive with more than half the tires destroyed
	if (TireStates.Num() > 0 && DestroyedTires > TireStates.Num() / 2)
	{
		return false;
	}

	return true;
}

void FForge2VehicleRuntimeState::ApplyDamage(float DamageAmount, EForge2VehicleComponent TargetComponent)
{
	CurrentHealth -= DamageAmount;
	CurrentHealth = FMath::Max(0.0f, CurrentHealth);

	// Apply component damage
	FForge2ComponentDamage* ExistingDamage = ComponentDamage.FindByPredicate(
		[TargetComponent](const FForge2ComponentDamage& D) { return D.Component == TargetComponent; });

	if (ExistingDamage)
	{
		ExistingDamage->DamagePercentage += DamageAmount;
		ExistingDamage->DamagePercentage = FMath::Min(100.0f, ExistingDamage->DamagePercentage);
	}
	else
	{
		FForge2ComponentDamage NewDamage;
		NewDamage.Component = TargetComponent;
		NewDamage.DamagePercentage = FMath::Min(100.0f, DamageAmount);
		ComponentDamage.Add(NewDamage);
	}
}

// ============================================================================
// TRACK DEFINITION
// ============================================================================

int32 FForge2TrackDefinition::GetSegmentAtDistance(float Distance) const
{
	if (Segments.Num() == 0)
	{
		return -1;
	}

	float AccumulatedDistance = 0.0f;

	for (int32 i = 0; i < Segments.Num(); ++i)
	{
		// Approximate segment length from spline points
		float SegmentLength = 0.0f;
		for (int32 j = 1; j < Segments[i].SplinePoints.Num(); ++j)
		{
			SegmentLength += FVector::Distance(
				Segments[i].SplinePoints[j - 1],
				Segments[i].SplinePoints[j]);
		}

		if (AccumulatedDistance + SegmentLength >= Distance)
		{
			return i;
		}

		AccumulatedDistance += SegmentLength;
	}

	return Segments.Num() - 1;
}

// ============================================================================
// RACE CONFIGURATION
// ============================================================================

void FForge2RaceConfiguration::SetupDefaultScoring()
{
	PositionPoints.Empty();

	// Standard F1-style points
	PositionPoints.Add(25); // 1st
	PositionPoints.Add(18); // 2nd
	PositionPoints.Add(15); // 3rd
	PositionPoints.Add(12); // 4th
	PositionPoints.Add(10); // 5th
	PositionPoints.Add(8);  // 6th
	PositionPoints.Add(6);  // 7th
	PositionPoints.Add(4);  // 8th
	PositionPoints.Add(2);  // 9th
	PositionPoints.Add(1);  // 10th
}

// ============================================================================
// AI RACER CONFIG
// ============================================================================

float FForge2AIRacerConfig::GetRubberBandBoost(int32 CurrentPosition, int32 TotalRacers, float DistanceBehindLeader) const
{
	if (RubberBanding == EForge2RubberBandType::None || TotalRacers <= 1)
	{
		return 1.0f;
	}

	// Calculate position percentage (0 = leader, 1 = last)
	float PositionPercent = (float)(CurrentPosition - 1) / (float)(TotalRacers - 1);

	// Base boost for being behind
	float Boost = 1.0f;

	switch (RubberBanding)
	{
	case EForge2RubberBandType::Subtle:
		Boost = 1.0f + (PositionPercent * 0.05f);
		break;

	case EForge2RubberBandType::Moderate:
		Boost = 1.0f + (PositionPercent * 0.1f);
		break;

	case EForge2RubberBandType::Aggressive:
		Boost = 1.0f + (PositionPercent * 0.2f);
		break;

	default:
		break;
	}

	// Additional boost based on distance behind
	if (DistanceBehindLeader > 100.0f)
	{
		float DistanceBoost = FMath::Min(DistanceBehindLeader / 1000.0f, 0.1f);
		Boost += DistanceBoost;
	}

	return Boost;
}

// ============================================================================
// LAP TIME
// ============================================================================

FString FForge2LapTime::GetFormattedTime() const
{
	int32 Minutes = (int32)(LapTimeSeconds / 60.0f);
	float Seconds = FMath::Fmod(LapTimeSeconds, 60.0f);

	return FString::Printf(TEXT("%d:%06.3f"), Minutes, Seconds);
}

// ============================================================================
// GHOST DATA
// ============================================================================

FTransform FForge2GhostData::GetTransformAtTime(float Time) const
{
	if (Frames.Num() == 0)
	{
		return FTransform::Identity;
	}

	if (Time <= Frames[0].Timestamp)
	{
		return Frames[0].VehicleTransform;
	}

	if (Time >= Frames.Last().Timestamp)
	{
		return Frames.Last().VehicleTransform;
	}

	// Find surrounding frames
	for (int32 i = 1; i < Frames.Num(); ++i)
	{
		if (Frames[i].Timestamp >= Time)
		{
			// Interpolate between frames
			float T = (Time - Frames[i - 1].Timestamp) / (Frames[i].Timestamp - Frames[i - 1].Timestamp);

			FTransform Result;
			Result.Blend(Frames[i - 1].VehicleTransform, Frames[i].VehicleTransform, T);
			return Result;
		}
	}

	return Frames.Last().VehicleTransform;
}

// ============================================================================
// RACING MANAGER
// ============================================================================

bool FForge2RacingManager::RegisterVehicle(const FForge2VehicleDefinition& Vehicle)
{
	if (Vehicle.VehicleId.IsEmpty())
	{
		return false;
	}

	Vehicles.Add(Vehicle.VehicleId, Vehicle);
	return true;
}

FForge2VehicleDefinition* FForge2RacingManager::GetVehicle(const FString& VehicleId)
{
	return Vehicles.Find(VehicleId);
}

TArray<FString> FForge2RacingManager::GetAllVehicleIds() const
{
	TArray<FString> Ids;
	Vehicles.GetKeys(Ids);
	return Ids;
}

bool FForge2RacingManager::RegisterTrack(const FForge2TrackDefinition& Track)
{
	if (Track.TrackId.IsEmpty())
	{
		return false;
	}

	Tracks.Add(Track.TrackId, Track);
	return true;
}

FForge2TrackDefinition* FForge2RacingManager::GetTrack(const FString& TrackId)
{
	return Tracks.Find(TrackId);
}

TArray<FString> FForge2RacingManager::GetAllTrackIds() const
{
	TArray<FString> Ids;
	Tracks.GetKeys(Ids);
	return Ids;
}

void FForge2RacingManager::RegisterPowerup(const FForge2Powerup& Powerup)
{
	Powerups.Add(Powerup.PowerupType, Powerup);
}

FForge2Powerup* FForge2RacingManager::GetPowerup(EForge2PowerupType Type)
{
	return Powerups.Find(Type);
}

EForge2PowerupType FForge2RacingManager::RollPowerup(int32 CurrentPosition, int32 TotalRacers) const
{
	if (Powerups.Num() == 0)
	{
		return EForge2PowerupType::SpeedBoost;
	}

	// Calculate position ratio (0 = first, 1 = last)
	float PositionRatio = (TotalRacers > 1) ?
		(float)(CurrentPosition - 1) / (float)(TotalRacers - 1) : 0.0f;

	// Build weighted list
	TArray<TPair<EForge2PowerupType, float>> WeightedList;
	float TotalWeight = 0.0f;

	for (const auto& Pair : Powerups)
	{
		float Weight = Pair.Value.SpawnWeight;

		// Apply position bias (higher bias = more likely when behind)
		Weight *= (1.0f + Pair.Value.PositionBias * PositionRatio);

		WeightedList.Add(TPair<EForge2PowerupType, float>(Pair.Key, Weight));
		TotalWeight += Weight;
	}

	// Random selection
	float RandomValue = FMath::FRand() * TotalWeight;
	float AccumulatedWeight = 0.0f;

	for (const auto& Entry : WeightedList)
	{
		AccumulatedWeight += Entry.Value;
		if (RandomValue <= AccumulatedWeight)
		{
			return Entry.Key;
		}
	}

	return WeightedList.Last().Key;
}

bool FForge2RacingManager::StartRace(const FString& TrackId, const FForge2RaceConfiguration& Config)
{
	if (!Tracks.Contains(TrackId))
	{
		return false;
	}

	bRaceActive = true;
	ActiveTrackId = TrackId;
	ActiveConfig = Config;
	Racers.Empty();

	return true;
}

void FForge2RacingManager::EndRace()
{
	bRaceActive = false;
}

bool FForge2RacingManager::AddRacer(const FString& RacerId, const FString& VehicleId, bool bIsAI, const FForge2AIRacerConfig& AIConfig)
{
	if (!bRaceActive || !Vehicles.Contains(VehicleId))
	{
		return false;
	}

	FRacerData Data;
	Data.VehicleId = VehicleId;
	Data.bIsAI = bIsAI;
	Data.AIConfig = AIConfig;
	Data.CurrentCheckpoint = 0;
	Data.CurrentLap = 0;
	Data.DistanceAlongTrack = 0.0f;
	Data.CurrentPosition = Racers.Num() + 1;
	Data.bEliminated = false;
	Data.bRecordingGhost = false;

	Racers.Add(RacerId, Data);
	return true;
}

void FForge2RacingManager::UpdateRacerPosition(const FString& RacerId, int32 CurrentCheckpoint, int32 CurrentLap, float DistanceAlongTrack)
{
	if (FRacerData* Data = Racers.Find(RacerId))
	{
		Data->CurrentCheckpoint = CurrentCheckpoint;
		Data->CurrentLap = CurrentLap;
		Data->DistanceAlongTrack = DistanceAlongTrack;

		UpdatePositions();
	}
}

int32 FForge2RacingManager::GetRacerPosition(const FString& RacerId) const
{
	if (const FRacerData* Data = Racers.Find(RacerId))
	{
		return Data->CurrentPosition;
	}
	return -1;
}

void FForge2RacingManager::RecordLapTime(const FString& RacerId, const FForge2LapTime& LapTime)
{
	if (FRacerData* Data = Racers.Find(RacerId))
	{
		Data->LapTimes.Add(LapTime);
	}
}

void FForge2RacingManager::EliminateRacer(const FString& RacerId, const FString& Reason)
{
	if (FRacerData* Data = Racers.Find(RacerId))
	{
		Data->bEliminated = true;
		Data->EliminationReason = Reason;
	}
}

void FForge2RacingManager::StartGhostRecording(const FString& RacerId)
{
	if (FRacerData* Data = Racers.Find(RacerId))
	{
		Data->bRecordingGhost = true;
		Data->GhostFrames.Empty();
	}
}

void FForge2RacingManager::RecordGhostFrame(const FString& RacerId, const FForge2GhostFrame& Frame)
{
	if (FRacerData* Data = Racers.Find(RacerId))
	{
		if (Data->bRecordingGhost)
		{
			Data->GhostFrames.Add(Frame);
		}
	}
}

FForge2GhostData FForge2RacingManager::StopGhostRecording(const FString& RacerId)
{
	FForge2GhostData Result;

	if (FRacerData* Data = Racers.Find(RacerId))
	{
		Data->bRecordingGhost = false;

		Result.TrackId = ActiveTrackId;
		Result.VehicleId = Data->VehicleId;
		Result.Frames = Data->GhostFrames;

		if (Data->LapTimes.Num() > 0)
		{
			Result.LapTime = Data->LapTimes.Last().LapTimeSeconds;
		}

		Data->GhostFrames.Empty();
	}

	return Result;
}

bool FForge2RacingManager::SaveGhostData(const FForge2GhostData& GhostData)
{
	// In a real implementation, this would save to disk
	// For now, just return success
	return true;
}

TArray<FForge2GhostData> FForge2RacingManager::LoadGhostData(const FString& TrackId, int32 MaxCount)
{
	// In a real implementation, this would load from disk
	TArray<FForge2GhostData> Result;
	return Result;
}

TArray<FForge2RacerResult> FForge2RacingManager::GetRaceResults() const
{
	TArray<FForge2RacerResult> Results;

	for (const auto& Pair : Racers)
	{
		FForge2RacerResult Result;
		Result.RacerId = Pair.Key;
		Result.FinishPosition = Pair.Value.CurrentPosition;
		Result.LapTimes = Pair.Value.LapTimes;
		Result.bDidNotFinish = Pair.Value.bEliminated;
		Result.DNFReason = Pair.Value.EliminationReason;

		// Calculate total time
		for (const FForge2LapTime& Lap : Pair.Value.LapTimes)
		{
			Result.TotalTime += Lap.LapTimeSeconds;

			// Track best lap
			if (Result.BestLap.LapTimeSeconds == 0.0f ||
				Lap.LapTimeSeconds < Result.BestLap.LapTimeSeconds)
			{
				Result.BestLap = Lap;
			}
		}

		// Award points
		if (Pair.Value.CurrentPosition > 0 &&
			Pair.Value.CurrentPosition <= ActiveConfig.PositionPoints.Num())
		{
			Result.PointsEarned = ActiveConfig.PositionPoints[Pair.Value.CurrentPosition - 1];
		}

		Results.Add(Result);
	}

	// Sort by position
	Results.Sort([](const FForge2RacerResult& A, const FForge2RacerResult& B)
	{
		return A.FinishPosition < B.FinishPosition;
	});

	return Results;
}

void FForge2RacingManager::Reset()
{
	Vehicles.Empty();
	Tracks.Empty();
	Powerups.Empty();
	Racers.Empty();
	bRaceActive = false;
	ActiveTrackId.Empty();
}

void FForge2RacingManager::UpdatePositions()
{
	// Calculate total distance for each racer
	TArray<TPair<FString, float>> RacerDistances;

	for (const auto& Pair : Racers)
	{
		if (!Pair.Value.bEliminated)
		{
			float TotalDistance = CalculateTotalDistance(Pair.Value);
			RacerDistances.Add(TPair<FString, float>(Pair.Key, TotalDistance));
		}
	}

	// Sort by total distance (highest = first place)
	RacerDistances.Sort([](const TPair<FString, float>& A, const TPair<FString, float>& B)
	{
		return A.Value > B.Value;
	});

	// Assign positions
	for (int32 i = 0; i < RacerDistances.Num(); ++i)
	{
		if (FRacerData* Data = Racers.Find(RacerDistances[i].Key))
		{
			Data->CurrentPosition = i + 1;
		}
	}

	// Eliminated racers get last positions
	int32 LastPosition = RacerDistances.Num() + 1;
	for (auto& Pair : Racers)
	{
		if (Pair.Value.bEliminated)
		{
			Pair.Value.CurrentPosition = LastPosition++;
		}
	}
}

float FForge2RacingManager::CalculateTotalDistance(const FRacerData& Racer) const
{
	const FForge2TrackDefinition* Track = Tracks.Find(ActiveTrackId);
	if (!Track)
	{
		return 0.0f;
	}

	// Distance = (Laps completed * track length) + (checkpoints * avg checkpoint distance) + distance to next checkpoint
	float LapDistance = Racer.CurrentLap * Track->TrackLength;
	float CheckpointDistance = 0.0f;

	if (Track->Checkpoints.Num() > 0)
	{
		float AvgCheckpointDistance = Track->TrackLength / Track->Checkpoints.Num();
		CheckpointDistance = Racer.CurrentCheckpoint * AvgCheckpointDistance;
	}

	return LapDistance + CheckpointDistance + Racer.DistanceAlongTrack;
}

// ============================================================================
// RACING UTILITIES
// ============================================================================

FForge2VehicleDefinition FForge2RacingUtils::CreateSportsCar(const FString& VehicleId, const FText& Name)
{
	FForge2VehicleDefinition Vehicle;
	Vehicle.VehicleId = VehicleId;
	Vehicle.VehicleName = Name;
	Vehicle.VehicleType = EForge2VehicleType::Car;
	Vehicle.DriveType = EForge2DriveType::RearWheelDrive;
	Vehicle.HandlingType = EForge2HandlingType::Simcade;
	Vehicle.Mass = 1400.0f;

	Vehicle.Engine.MaxHorsepower = 450.0f;
	Vehicle.Engine.MaxTorque = 520.0f;
	Vehicle.Engine.MaxHorsepowerRPM = 7000.0f;
	Vehicle.Engine.RedlineRPM = 8500.0f;

	Vehicle.Transmission.SetupDefaultGears(7);
	Vehicle.Transmission.FinalDriveRatio = 3.2f;

	Vehicle.NumWheels = 4;
	Vehicle.SetupDefaultWheels();

	Vehicle.Aerodynamics.DragCoefficient = 0.32f;
	Vehicle.Aerodynamics.DownforceCoefficient = 0.4f;

	Vehicle.TopSpeed = 320.0f;
	Vehicle.AccelerationRating = 8.0f;
	Vehicle.HandlingRating = 7.5f;

	return Vehicle;
}

FForge2VehicleDefinition FForge2RacingUtils::CreateMuscleCar(const FString& VehicleId, const FText& Name)
{
	FForge2VehicleDefinition Vehicle;
	Vehicle.VehicleId = VehicleId;
	Vehicle.VehicleName = Name;
	Vehicle.VehicleType = EForge2VehicleType::Car;
	Vehicle.DriveType = EForge2DriveType::RearWheelDrive;
	Vehicle.HandlingType = EForge2HandlingType::Arcade;
	Vehicle.Mass = 1800.0f;

	Vehicle.Engine.MaxHorsepower = 550.0f;
	Vehicle.Engine.MaxTorque = 700.0f;
	Vehicle.Engine.MaxHorsepowerRPM = 5500.0f;
	Vehicle.Engine.RedlineRPM = 6500.0f;

	Vehicle.Transmission.SetupDefaultGears(6);
	Vehicle.Transmission.FinalDriveRatio = 3.9f;

	Vehicle.NumWheels = 4;
	Vehicle.SetupDefaultWheels();

	Vehicle.Aerodynamics.DragCoefficient = 0.45f;
	Vehicle.Aerodynamics.DownforceCoefficient = 0.2f;

	Vehicle.TopSpeed = 280.0f;
	Vehicle.AccelerationRating = 9.0f;
	Vehicle.HandlingRating = 5.0f;

	return Vehicle;
}

FForge2VehicleDefinition FForge2RacingUtils::CreateKart(const FString& VehicleId, const FText& Name)
{
	FForge2VehicleDefinition Vehicle;
	Vehicle.VehicleId = VehicleId;
	Vehicle.VehicleName = Name;
	Vehicle.VehicleType = EForge2VehicleType::Kart;
	Vehicle.DriveType = EForge2DriveType::RearWheelDrive;
	Vehicle.HandlingType = EForge2HandlingType::Arcade;
	Vehicle.Mass = 200.0f;

	Vehicle.Engine.MaxHorsepower = 15.0f;
	Vehicle.Engine.MaxTorque = 25.0f;
	Vehicle.Engine.MaxHorsepowerRPM = 8000.0f;
	Vehicle.Engine.RedlineRPM = 10000.0f;

	Vehicle.Transmission.SetupDefaultGears(1);
	Vehicle.Transmission.FinalDriveRatio = 6.0f;

	Vehicle.NumWheels = 4;
	Vehicle.SetupDefaultWheels();

	Vehicle.MaxSteeringAngle = 45.0f;
	Vehicle.SteeringSpeed = 200.0f;

	Vehicle.TopSpeed = 100.0f;
	Vehicle.AccelerationRating = 7.0f;
	Vehicle.HandlingRating = 9.0f;

	Vehicle.NitroCapacity = 3.0f;
	Vehicle.BoostStrength = 2.0f;

	return Vehicle;
}

FForge2VehicleDefinition FForge2RacingUtils::CreateMotorcycle(const FString& VehicleId, const FText& Name)
{
	FForge2VehicleDefinition Vehicle;
	Vehicle.VehicleId = VehicleId;
	Vehicle.VehicleName = Name;
	Vehicle.VehicleType = EForge2VehicleType::Motorcycle;
	Vehicle.DriveType = EForge2DriveType::RearWheelDrive;
	Vehicle.HandlingType = EForge2HandlingType::Simcade;
	Vehicle.Mass = 200.0f;

	Vehicle.Engine.MaxHorsepower = 180.0f;
	Vehicle.Engine.MaxTorque = 110.0f;
	Vehicle.Engine.MaxHorsepowerRPM = 12000.0f;
	Vehicle.Engine.RedlineRPM = 14000.0f;

	Vehicle.Transmission.SetupDefaultGears(6);
	Vehicle.Transmission.FinalDriveRatio = 2.8f;

	Vehicle.NumWheels = 2;
	Vehicle.SetupDefaultWheels();

	Vehicle.MaxSteeringAngle = 25.0f;

	Vehicle.TopSpeed = 300.0f;
	Vehicle.AccelerationRating = 9.5f;
	Vehicle.HandlingRating = 8.0f;

	return Vehicle;
}

TArray<FForge2Powerup> FForge2RacingUtils::CreateArcadePowerups()
{
	TArray<FForge2Powerup> Powerups;

	{
		FForge2Powerup Boost;
		Boost.PowerupType = EForge2PowerupType::SpeedBoost;
		Boost.Duration = 3.0f;
		Boost.Magnitude = 1.5f;
		Boost.Uses = 1;
		Boost.SpawnWeight = 3.0f;
		Boost.PositionBias = 0.5f;
		Powerups.Add(Boost);
	}

	{
		FForge2Powerup Shield;
		Shield.PowerupType = EForge2PowerupType::Shield;
		Shield.Duration = 5.0f;
		Shield.Magnitude = 1.0f;
		Shield.Uses = 1;
		Shield.SpawnWeight = 2.0f;
		Shield.PositionBias = 0.3f;
		Powerups.Add(Shield);
	}

	{
		FForge2Powerup Missile;
		Missile.PowerupType = EForge2PowerupType::Missile;
		Missile.Duration = 0.0f;
		Missile.Magnitude = 25.0f;
		Missile.Uses = 1;
		Missile.SpawnWeight = 2.0f;
		Missile.PositionBias = 0.7f;
		Powerups.Add(Missile);
	}

	return Powerups;
}

TArray<FForge2Powerup> FForge2RacingUtils::CreateCombatRacingPowerups()
{
	TArray<FForge2Powerup> Powerups = CreateArcadePowerups();

	{
		FForge2Powerup Mines;
		Mines.PowerupType = EForge2PowerupType::Mines;
		Mines.Duration = 30.0f;
		Mines.Magnitude = 20.0f;
		Mines.Uses = 3;
		Mines.SpawnWeight = 1.5f;
		Mines.PositionBias = -0.3f; // More likely for leaders
		Powerups.Add(Mines);
	}

	{
		FForge2Powerup OilSlick;
		OilSlick.PowerupType = EForge2PowerupType::OilSlick;
		OilSlick.Duration = 15.0f;
		OilSlick.Magnitude = 1.0f;
		OilSlick.Uses = 1;
		OilSlick.SpawnWeight = 1.5f;
		OilSlick.PositionBias = -0.2f;
		Powerups.Add(OilSlick);
	}

	{
		FForge2Powerup Lightning;
		Lightning.PowerupType = EForge2PowerupType::Lightning;
		Lightning.Duration = 0.0f;
		Lightning.Magnitude = 15.0f;
		Lightning.Uses = 1;
		Lightning.SpawnWeight = 0.5f;
		Lightning.PositionBias = 1.0f; // Very rare for leaders
		Powerups.Add(Lightning);
	}

	return Powerups;
}

FForge2TrackDefinition FForge2RacingUtils::GenerateOvalTrack(const FString& TrackId, const FText& Name, float Length, float Width)
{
	FForge2TrackDefinition Track;
	Track.TrackId = TrackId;
	Track.TrackName = Name;
	Track.TrackLength = Length;
	Track.bIsCircuit = true;
	Track.DefaultLaps = 10;

	// Calculate oval dimensions
	float StraightLength = Length * 0.4f / 2.0f;
	float CurveLength = Length * 0.6f / 2.0f;
	float CurveRadius = CurveLength / PI;

	// Create spline points for oval
	TArray<FVector> SplinePoints;
	int32 PointsPerCurve = 16;

	// Bottom straight
	SplinePoints.Add(FVector(0, -StraightLength / 2, 0));
	SplinePoints.Add(FVector(0, StraightLength / 2, 0));

	// Right curve
	for (int32 i = 0; i <= PointsPerCurve; ++i)
	{
		float Angle = (float)i / (float)PointsPerCurve * PI;
		float X = CurveRadius * FMath::Sin(Angle);
		float Y = StraightLength / 2 + CurveRadius * (1.0f - FMath::Cos(Angle));
		SplinePoints.Add(FVector(X, Y, 0));
	}

	// Top straight
	SplinePoints.Add(FVector(0, StraightLength / 2 + 2 * CurveRadius, 0));

	// Left curve
	for (int32 i = 0; i <= PointsPerCurve; ++i)
	{
		float Angle = PI + (float)i / (float)PointsPerCurve * PI;
		float X = CurveRadius * FMath::Sin(Angle);
		float Y = StraightLength / 2 + CurveRadius * (1.0f - FMath::Cos(Angle));
		SplinePoints.Add(FVector(X, Y, 0));
	}

	// Create single segment
	FForge2TrackSegment Segment;
	Segment.SegmentId = 0;
	Segment.SplinePoints = SplinePoints;
	Segment.Surface = EForge2TrackSurface::Asphalt;

	for (int32 i = 0; i < SplinePoints.Num(); ++i)
	{
		Segment.WidthAtPoints.Add(Width);
		Segment.RacingLineOffset.Add(0.0f);
		Segment.RecommendedSpeed.Add(i < 2 || i > SplinePoints.Num() - 3 ? 200.0f : 150.0f);
	}

	Track.Segments.Add(Segment);

	// Create checkpoints
	for (int32 i = 0; i < 4; ++i)
	{
		FForge2Checkpoint Checkpoint;
		Checkpoint.CheckpointIndex = i;
		Checkpoint.Location = SplinePoints[i * (SplinePoints.Num() / 4)];
		Checkpoint.Width = Width;
		Checkpoint.bIsStartFinish = (i == 0);
		Track.Checkpoints.Add(Checkpoint);
	}

	// Create starting grid
	for (int32 i = 0; i < 12; ++i)
	{
		int32 Row = i / 2;
		int32 Side = i % 2;
		FTransform GridPos;
		GridPos.SetLocation(FVector(Side == 0 ? -Width / 4 : Width / 4, -100 * Row, 0));
		GridPos.SetRotation(FQuat::Identity);
		Track.StartingGridPositions.Add(GridPos);
	}

	return Track;
}

FForge2TrackDefinition FForge2RacingUtils::GenerateCircuitFromSpline(const FString& TrackId, const FText& Name, const TArray<FVector>& SplinePoints, float TrackWidth)
{
	FForge2TrackDefinition Track;
	Track.TrackId = TrackId;
	Track.TrackName = Name;
	Track.bIsCircuit = true;

	// Calculate track length
	float TotalLength = 0.0f;
	for (int32 i = 1; i < SplinePoints.Num(); ++i)
	{
		TotalLength += FVector::Distance(SplinePoints[i - 1], SplinePoints[i]);
	}
	// Add closing segment if circuit
	TotalLength += FVector::Distance(SplinePoints.Last(), SplinePoints[0]);
	Track.TrackLength = TotalLength;

	// Create segment
	FForge2TrackSegment Segment;
	Segment.SegmentId = 0;
	Segment.SplinePoints = SplinePoints;
	Segment.Surface = EForge2TrackSurface::Asphalt;

	for (int32 i = 0; i < SplinePoints.Num(); ++i)
	{
		Segment.WidthAtPoints.Add(TrackWidth);
		Segment.RacingLineOffset.Add(0.0f);
		Segment.RecommendedSpeed.Add(150.0f);
	}

	Track.Segments.Add(Segment);

	// Create checkpoints every ~500m
	int32 NumCheckpoints = FMath::Max(4, (int32)(TotalLength / 500.0f));
	float CheckpointInterval = TotalLength / NumCheckpoints;
	float AccumulatedDistance = 0.0f;
	int32 CheckpointIndex = 0;

	for (int32 i = 1; i < SplinePoints.Num(); ++i)
	{
		float SegmentLength = FVector::Distance(SplinePoints[i - 1], SplinePoints[i]);

		while (AccumulatedDistance + SegmentLength >= CheckpointInterval * CheckpointIndex &&
			   CheckpointIndex < NumCheckpoints)
		{
			FForge2Checkpoint Checkpoint;
			Checkpoint.CheckpointIndex = CheckpointIndex;
			Checkpoint.Location = SplinePoints[i];
			Checkpoint.Width = TrackWidth;
			Checkpoint.bIsStartFinish = (CheckpointIndex == 0);
			Track.Checkpoints.Add(Checkpoint);
			CheckpointIndex++;
		}

		AccumulatedDistance += SegmentLength;
	}

	// Create starting grid at first spline point
	FVector StartDir = (SplinePoints[1] - SplinePoints[0]).GetSafeNormal();
	FVector RightDir = FVector::CrossProduct(StartDir, FVector::UpVector);

	for (int32 i = 0; i < 12; ++i)
	{
		int32 Row = i / 2;
		int32 Side = i % 2;
		FTransform GridPos;
		FVector Pos = SplinePoints[0] - StartDir * 100.0f * Row;
		Pos += RightDir * TrackWidth / 4.0f * (Side == 0 ? -1.0f : 1.0f);
		GridPos.SetLocation(Pos);
		GridPos.SetRotation(FRotationMatrix::MakeFromX(StartDir).ToQuat());
		Track.StartingGridPositions.Add(GridPos);
	}

	return Track;
}

float FForge2RacingUtils::CalculateSlipAngle(const FVector& Velocity, const FVector& ForwardVector)
{
	if (Velocity.SizeSquared() < 1.0f)
	{
		return 0.0f;
	}

	FVector NormalizedVelocity = Velocity.GetSafeNormal();
	float Dot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
	Dot = FMath::Clamp(Dot, -1.0f, 1.0f);

	return FMath::Acos(Dot);
}

float FForge2RacingUtils::CalculateSlipRatio(float WheelSpeed, float VehicleSpeed)
{
	if (FMath::Abs(VehicleSpeed) < 1.0f && FMath::Abs(WheelSpeed) < 1.0f)
	{
		return 0.0f;
	}

	float MaxSpeed = FMath::Max(FMath::Abs(WheelSpeed), FMath::Abs(VehicleSpeed));
	return (WheelSpeed - VehicleSpeed) / MaxSpeed;
}

FVector FForge2RacingUtils::CalculateTireForce(float SlipAngle, float SlipRatio, float NormalForce, float GripCoefficient)
{
	// Simplified Pacejka-like tire model
	float LateralSlip = FMath::Sin(SlipAngle);
	float LongitudinalSlip = FMath::Clamp(SlipRatio, -1.0f, 1.0f);

	// Combined slip magnitude
	float CombinedSlip = FMath::Sqrt(LateralSlip * LateralSlip + LongitudinalSlip * LongitudinalSlip);
	CombinedSlip = FMath::Max(0.001f, CombinedSlip); // Prevent division by zero

	// Force magnitude (simplified)
	float ForceMagnitude = NormalForce * GripCoefficient * (1.0f - FMath::Exp(-CombinedSlip * 10.0f));

	// Distribute force
	float LateralForce = ForceMagnitude * LateralSlip / CombinedSlip;
	float LongitudinalForce = ForceMagnitude * LongitudinalSlip / CombinedSlip;

	return FVector(LongitudinalForce, LateralForce, 0.0f);
}

FString FForge2RacingUtils::FormatLapTime(float TimeSeconds)
{
	if (TimeSeconds <= 0.0f)
	{
		return TEXT("--:--.---");
	}

	int32 Minutes = (int32)(TimeSeconds / 60.0f);
	float Seconds = FMath::Fmod(TimeSeconds, 60.0f);

	return FString::Printf(TEXT("%d:%06.3f"), Minutes, Seconds);
}

FString FForge2RacingUtils::FormatDelta(float DeltaSeconds)
{
	if (FMath::Abs(DeltaSeconds) < 0.001f)
	{
		return TEXT("+0.000");
	}

	FString Sign = DeltaSeconds > 0.0f ? TEXT("+") : TEXT("-");
	float AbsDelta = FMath::Abs(DeltaSeconds);

	if (AbsDelta >= 60.0f)
	{
		int32 Minutes = (int32)(AbsDelta / 60.0f);
		float Seconds = FMath::Fmod(AbsDelta, 60.0f);
		return FString::Printf(TEXT("%s%d:%06.3f"), *Sign, Minutes, Seconds);
	}

	return FString::Printf(TEXT("%s%.3f"), *Sign, AbsDelta);
}
