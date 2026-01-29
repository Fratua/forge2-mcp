// Forge2SportsSystem.cpp
// FORGE2 Sports Genre System Implementation

#include "Forge2SportsSystem.h"

// ============================================================================
// FForge2PlayerStats
// ============================================================================

float FForge2PlayerStats::GetOverall() const
{
    // Weighted average of all stats
    float PhysicalAvg = (Speed + Acceleration + Stamina + Strength + Jumping + Agility) / 6.0f;
    float TechnicalAvg = (Shooting + Passing + Dribbling + Defense + Heading + Tackling) / 6.0f;
    float MentalAvg = (Composure + Vision + Positioning + Aggression) / 4.0f;

    return (PhysicalAvg * 0.3f + TechnicalAvg * 0.5f + MentalAvg * 0.2f);
}

float FForge2PlayerStats::GetAttributeValue(const FName& AttributeName) const
{
    FString AttrString = AttributeName.ToString().ToLower();

    if (AttrString == TEXT("speed")) return Speed;
    if (AttrString == TEXT("acceleration")) return Acceleration;
    if (AttrString == TEXT("stamina")) return Stamina;
    if (AttrString == TEXT("strength")) return Strength;
    if (AttrString == TEXT("jumping")) return Jumping;
    if (AttrString == TEXT("agility")) return Agility;
    if (AttrString == TEXT("shooting")) return Shooting;
    if (AttrString == TEXT("passing")) return Passing;
    if (AttrString == TEXT("dribbling")) return Dribbling;
    if (AttrString == TEXT("defense")) return Defense;
    if (AttrString == TEXT("heading")) return Heading;
    if (AttrString == TEXT("tackling")) return Tackling;
    if (AttrString == TEXT("composure")) return Composure;
    if (AttrString == TEXT("vision")) return Vision;
    if (AttrString == TEXT("positioning")) return Positioning;
    if (AttrString == TEXT("aggression")) return Aggression;
    if (AttrString == TEXT("diving")) return Diving;
    if (AttrString == TEXT("handling")) return Handling;
    if (AttrString == TEXT("reflexes")) return Reflexes;

    return 50.0f;
}

// ============================================================================
// FForge2SportsPlayer
// ============================================================================

void FForge2SportsPlayer::ResetMatchStats()
{
    Goals = 0;
    Assists = 0;
    Shots = 0;
    ShotsOnTarget = 0;
    Passes = 0;
    PassesCompleted = 0;
    Tackles = 0;
    Fouls = 0;
    YellowCards = 0;
    RedCards = 0;
}

float FForge2SportsPlayer::GetPassAccuracy() const
{
    if (Passes == 0) return 0.0f;
    return (static_cast<float>(PassesCompleted) / static_cast<float>(Passes)) * 100.0f;
}

float FForge2SportsPlayer::GetShotAccuracy() const
{
    if (Shots == 0) return 0.0f;
    return (static_cast<float>(ShotsOnTarget) / static_cast<float>(Shots)) * 100.0f;
}

// ============================================================================
// FForge2BallPhysics
// ============================================================================

void FForge2BallPhysics::Update(float DeltaTime)
{
    if (!bIsInPlay) return;

    // Apply gravity
    const float Gravity = 980.0f; // cm/s^2
    Velocity.Z -= Gravity * DeltaTime;

    // Apply air resistance
    FVector AirDrag = -Velocity * AirResistance * Velocity.Size() * DeltaTime;
    Velocity += AirDrag;

    // Apply Magnus effect (spin)
    FVector MagnusForce = CalculateMagnusForce();
    Velocity += MagnusForce * DeltaTime;

    // Update position
    Position += Velocity * DeltaTime;

    // Ground collision
    if (Position.Z < Radius)
    {
        Position.Z = Radius;
        ApplyBounce(FVector::UpVector);

        // Apply rolling friction
        FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
        HorizontalVelocity *= (1.0f - Friction * DeltaTime);
        Velocity.X = HorizontalVelocity.X;
        Velocity.Y = HorizontalVelocity.Y;
    }

    // Angular velocity decay
    AngularVelocity *= (1.0f - 0.1f * DeltaTime);
}

void FForge2BallPhysics::ApplyKick(const FVector& Force, const FVector& SpinAxis, float SpinAmount)
{
    Velocity += Force / Mass;
    AngularVelocity = SpinAxis * SpinAmount;
    PossessorID = NAME_None;
}

void FForge2BallPhysics::ApplyBounce(const FVector& Normal)
{
    // Reflect velocity with energy loss
    float VelocityDotNormal = FVector::DotProduct(Velocity, Normal);
    Velocity = Velocity - 2.0f * VelocityDotNormal * Normal;
    Velocity *= Bounciness;
}

FVector FForge2BallPhysics::CalculateMagnusForce() const
{
    // Magnus effect: F = coefficient * (angular_velocity x velocity)
    return MagnusCoefficient * FVector::CrossProduct(AngularVelocity, Velocity);
}

// ============================================================================
// FForge2SportsTeam
// ============================================================================

float FForge2SportsTeam::GetTeamOverall() const
{
    if (Roster.Num() == 0) return 0.0f;

    float Total = 0.0f;
    for (const FForge2SportsPlayer& Player : Roster)
    {
        Total += Player.Stats.GetOverall();
    }

    return Total / Roster.Num();
}

FForge2SportsPlayer* FForge2SportsTeam::GetPlayerByID(FName PlayerID)
{
    for (FForge2SportsPlayer& Player : Roster)
    {
        if (Player.PlayerID == PlayerID)
        {
            return &Player;
        }
    }
    return nullptr;
}

// ============================================================================
// FForge2Formation
// ============================================================================

TArray<FVector> FForge2Formation::GetWorldPositions(const FVector& FieldCenter, const FVector& FieldSize, bool bAttackingRight) const
{
    TArray<FVector> WorldPositions;
    WorldPositions.Reserve(Positions.Num());

    for (const FVector2D& NormPos : Positions)
    {
        float X = NormPos.X;
        float Y = NormPos.Y;

        // Flip if attacking left
        if (!bAttackingRight)
        {
            X = 1.0f - X;
        }

        // Convert to world coordinates
        FVector WorldPos;
        WorldPos.X = FieldCenter.X + (X - 0.5f) * FieldSize.X;
        WorldPos.Y = FieldCenter.Y + (Y - 0.5f) * FieldSize.Y;
        WorldPos.Z = 0.0f;

        WorldPositions.Add(WorldPos);
    }

    return WorldPositions;
}

// ============================================================================
// FForge2RefereeSystem
// ============================================================================

EForge2FoulType FForge2RefereeSystem::DetectFoul(const FForge2SportsPlayer& Fouler, const FForge2SportsPlayer& Victim, float ContactForce)
{
    // Base foul chance based on contact force
    float FoulChance = ContactForce / 100.0f;

    // Modify by aggression
    FoulChance += (Fouler.Stats.Aggression - 50.0f) / 200.0f;

    // Modify by referee strictness
    FoulChance *= (0.5f + Strictness);

    // Random check
    if (FMath::FRand() > FoulChance)
    {
        return EForge2FoulType::None;
    }

    // Determine severity
    if (ContactForce > 80.0f || Fouler.YellowCards > 0)
    {
        return EForge2FoulType::RedCard;
    }
    else if (ContactForce > 50.0f)
    {
        return EForge2FoulType::YellowCard;
    }
    else if (ContactForce > 30.0f)
    {
        return EForge2FoulType::MajorFoul;
    }

    return EForge2FoulType::MinorFoul;
}

bool FForge2RefereeSystem::CheckOffside(const FVector& AttackerPosition, const FVector& LastDefenderPosition)
{
    return AttackerPosition.X > LastDefenderPosition.X && AttackerPosition.X > OffsideLineX;
}

bool FForge2RefereeSystem::ShouldVARReview(EForge2FoulType FoulType) const
{
    if (!bVAREnabled) return false;

    // VAR only reviews serious incidents
    if (FoulType == EForge2FoulType::RedCard || FoulType == EForge2FoulType::MajorFoul)
    {
        return FMath::FRand() < VARReviewChance;
    }

    return false;
}

// ============================================================================
// FForge2CommentaryTrigger
// ============================================================================

bool FForge2CommentaryTrigger::CanTrigger(float CurrentTime) const
{
    return (CurrentTime - LastTriggeredTime) >= Cooldown;
}

FString FForge2CommentaryTrigger::GetRandomLine() const
{
    if (CommentaryLines.Num() == 0) return TEXT("");
    int32 Index = FMath::RandRange(0, CommentaryLines.Num() - 1);
    return CommentaryLines[Index];
}

// ============================================================================
// FForge2ReplaySystem
// ============================================================================

void FForge2ReplaySystem::RecordFrame(const FForge2ReplayFrame& Frame)
{
    if (!bIsRecording) return;

    Frames.Add(Frame);
    TrimOldFrames();
}

FForge2ReplayFrame FForge2ReplaySystem::GetFrameAtTime(float Time) const
{
    if (Frames.Num() == 0)
    {
        return FForge2ReplayFrame();
    }

    // Find the two frames surrounding the requested time
    int32 LowerIndex = 0;
    int32 UpperIndex = 0;

    for (int32 i = 0; i < Frames.Num(); ++i)
    {
        if (Frames[i].Timestamp <= Time)
        {
            LowerIndex = i;
        }
        if (Frames[i].Timestamp >= Time)
        {
            UpperIndex = i;
            break;
        }
    }

    // Interpolate between frames
    if (LowerIndex == UpperIndex)
    {
        return Frames[LowerIndex];
    }

    const FForge2ReplayFrame& Lower = Frames[LowerIndex];
    const FForge2ReplayFrame& Upper = Frames[UpperIndex];

    float Alpha = (Time - Lower.Timestamp) / (Upper.Timestamp - Lower.Timestamp);

    FForge2ReplayFrame Result;
    Result.Timestamp = Time;
    Result.BallPosition = FMath::Lerp(Lower.BallPosition, Upper.BallPosition, Alpha);
    Result.BallVelocity = FMath::Lerp(Lower.BallVelocity, Upper.BallVelocity, Alpha);

    // Interpolate player positions
    int32 NumPlayers = FMath::Min(Lower.PlayerPositions.Num(), Upper.PlayerPositions.Num());
    for (int32 i = 0; i < NumPlayers; ++i)
    {
        Result.PlayerPositions.Add(FMath::Lerp(Lower.PlayerPositions[i], Upper.PlayerPositions[i], Alpha));
        Result.PlayerRotations.Add(FMath::Lerp(Lower.PlayerRotations[i], Upper.PlayerRotations[i], Alpha));
    }

    return Result;
}

void FForge2ReplaySystem::TrimOldFrames()
{
    float MaxFrames = MaxDuration * RecordingFPS;
    while (Frames.Num() > MaxFrames)
    {
        Frames.RemoveAt(0);
    }
}

void FForge2ReplaySystem::StartPlayback(float StartTime)
{
    bIsRecording = false;
    bIsPlaying = true;
    CurrentPlaybackTime = StartTime;
}

void FForge2ReplaySystem::StopPlayback()
{
    bIsPlaying = false;
    bIsRecording = true;
}

// ============================================================================
// FForge2SeasonStanding
// ============================================================================

void FForge2SeasonStanding::UpdateFromMatch(int32 Scored, int32 Conceded)
{
    Played++;
    GoalsFor += Scored;
    GoalsAgainst += Conceded;

    if (Scored > Conceded)
    {
        Won++;
        Points += 3;
    }
    else if (Scored == Conceded)
    {
        Drawn++;
        Points += 1;
    }
    else
    {
        Lost++;
    }
}

// ============================================================================
// FForge2Season
// ============================================================================

void FForge2Season::GenerateSchedule()
{
    Schedule.Empty();
    Standings.Empty();

    // Initialize standings
    for (const FName& TeamID : TeamIDs)
    {
        FForge2SeasonStanding Standing;
        Standing.TeamID = TeamID;
        Standings.Add(Standing);
    }

    // Round-robin schedule
    int32 NumTeams = TeamIDs.Num();
    if (NumTeams < 2) return;

    // Make even number of teams
    TArray<FName> Teams = TeamIDs;
    bool bHasBye = (NumTeams % 2 != 0);
    if (bHasBye)
    {
        Teams.Add(NAME_None); // Bye
        NumTeams++;
    }

    int32 NumRounds = NumTeams - 1;
    int32 MatchesPerRound = NumTeams / 2;

    for (int32 Round = 0; Round < NumRounds * 2; ++Round) // Home and away
    {
        for (int32 Match = 0; Match < MatchesPerRound; ++Match)
        {
            int32 Home = (Round + Match) % (NumTeams - 1);
            int32 Away = (NumTeams - 1 - Match + Round) % (NumTeams - 1);

            if (Match == 0)
            {
                Away = NumTeams - 1;
            }

            // Swap home/away for second half of season
            if (Round >= NumRounds)
            {
                Swap(Home, Away);
            }

            // Skip bye matches
            if (Teams[Home].IsNone() || Teams[Away].IsNone())
            {
                continue;
            }

            FForge2SeasonMatch MatchData;
            MatchData.MatchID = *FString::Printf(TEXT("Match_%d_%d"), Round, Match);
            MatchData.HomeTeamID = Teams[Home];
            MatchData.AwayTeamID = Teams[Away];
            MatchData.Week = Round;

            Schedule.Add(MatchData);
        }
    }

    CurrentWeek = 0;
}

void FForge2Season::UpdateStandings(const FForge2SeasonMatch& Match)
{
    for (FForge2SeasonStanding& Standing : Standings)
    {
        if (Standing.TeamID == Match.HomeTeamID)
        {
            Standing.UpdateFromMatch(Match.HomeScore, Match.AwayScore);
        }
        else if (Standing.TeamID == Match.AwayTeamID)
        {
            Standing.UpdateFromMatch(Match.AwayScore, Match.HomeScore);
        }
    }

    SortStandings();
}

void FForge2Season::SortStandings()
{
    Standings.Sort([](const FForge2SeasonStanding& A, const FForge2SeasonStanding& B)
    {
        if (A.Points != B.Points) return A.Points > B.Points;
        if (A.GetGoalDifference() != B.GetGoalDifference()) return A.GetGoalDifference() > B.GetGoalDifference();
        return A.GoalsFor > B.GoalsFor;
    });
}

TArray<FForge2SeasonMatch> FForge2Season::GetMatchesForWeek(int32 Week) const
{
    TArray<FForge2SeasonMatch> Matches;
    for (const FForge2SeasonMatch& Match : Schedule)
    {
        if (Match.Week == Week)
        {
            Matches.Add(Match);
        }
    }
    return Matches;
}

// ============================================================================
// FForge2MatchState
// ============================================================================

bool FForge2MatchState::IsPeriodOver() const
{
    return MatchTime >= PeriodLength + InjuryTime;
}

bool FForge2MatchState::IsMatchOver() const
{
    // Assuming 2 periods (halves) for soccer
    return Period >= 2 && IsPeriodOver();
}

void FForge2MatchState::AdvancePeriod()
{
    Period++;
    MatchTime = 0.0f;
    InjuryTime = 0.0f;
    State = EForge2MatchState::Kickoff;
}

// ============================================================================
// UForge2SportsManager
// ============================================================================

UForge2SportsManager::UForge2SportsManager()
{
    SportType = EForge2SportType::Soccer;
}

void UForge2SportsManager::InitializeMatch(const FForge2SportsTeam& Home, const FForge2SportsTeam& Away)
{
    HomeTeam = Home;
    AwayTeam = Away;

    // Reset player stats
    for (FForge2SportsPlayer& Player : HomeTeam.Roster)
    {
        Player.ResetMatchStats();
        Player.CurrentStamina = 100.0f;
    }
    for (FForge2SportsPlayer& Player : AwayTeam.Roster)
    {
        Player.ResetMatchStats();
        Player.CurrentStamina = 100.0f;
    }

    // Reset scores
    HomeTeam.Score = 0;
    AwayTeam.Score = 0;

    // Initialize ball at center
    Ball = UForge2SportsUtilities::CreateSoccerBall();
    Ball.Position = FVector::ZeroVector;

    // Initialize match state
    MatchState.State = EForge2MatchState::PreMatch;
    MatchState.MatchTime = 0.0f;
    MatchState.RealTime = 0.0f;
    MatchState.Period = 1;

    // Clear replay
    Replay.Frames.Empty();
    Replay.bIsRecording = true;
}

void UForge2SportsManager::UpdateMatch(float DeltaTime)
{
    if (MatchState.State == EForge2MatchState::PreMatch ||
        MatchState.State == EForge2MatchState::Paused ||
        MatchState.State == EForge2MatchState::HalfTime ||
        MatchState.State == EForge2MatchState::PostMatch)
    {
        return;
    }

    UpdateMatchTime(DeltaTime);
    UpdateBallPhysics(DeltaTime);
    UpdatePlayerPositions(DeltaTime);
    CheckBallBoundaries();
    RecordReplayFrame();

    // Check for period end
    if (MatchState.IsPeriodOver())
    {
        if (MatchState.IsMatchOver())
        {
            MatchState.State = EForge2MatchState::PostMatch;
        }
        else
        {
            MatchState.State = EForge2MatchState::HalfTime;
        }
    }
}

void UForge2SportsManager::UpdateBallPhysics(float DeltaTime)
{
    Ball.Update(DeltaTime);
}

void UForge2SportsManager::UpdatePlayerPositions(float DeltaTime)
{
    // Update stamina for all players
    for (FForge2SportsPlayer& Player : HomeTeam.Roster)
    {
        // Drain stamina based on activity (simplified)
        Player.CurrentStamina -= 0.01f * DeltaTime;
        Player.CurrentStamina = FMath::Max(0.0f, Player.CurrentStamina);
    }

    for (FForge2SportsPlayer& Player : AwayTeam.Roster)
    {
        Player.CurrentStamina -= 0.01f * DeltaTime;
        Player.CurrentStamina = FMath::Max(0.0f, Player.CurrentStamina);
    }
}

void UForge2SportsManager::UpdateMatchTime(float DeltaTime)
{
    MatchState.RealTime += DeltaTime;
    MatchState.MatchTime += DeltaTime / MatchState.TimeMultiplier;
}

void UForge2SportsManager::CheckBallBoundaries()
{
    // Check if ball is out of bounds
    if (IsOutOfBounds(Ball.Position))
    {
        Ball.bIsInPlay = false;

        // Determine type of restart
        if (FMath::Abs(Ball.Position.X) > FieldSize.X / 2.0f)
        {
            // Goal or goal kick
            if (IsGoal(Ball.Position))
            {
                // Determine which team scored
                FName ScoringTeamID = Ball.Position.X > 0 ? HomeTeam.TeamID : AwayTeam.TeamID;
                OnGoalScored(ScoringTeamID, NAME_None, NAME_None);
            }
            else
            {
                MatchState.CurrentPlayType = EForge2PlayType::CornerKick;
            }
        }
        else
        {
            // Throw-in
            MatchState.CurrentPlayType = EForge2PlayType::ThrowIn;
        }

        MatchState.SetPieceLocation = Ball.Position;
    }
}

void UForge2SportsManager::RecordReplayFrame()
{
    FForge2ReplayFrame Frame;
    Frame.Timestamp = MatchState.RealTime;
    Frame.BallPosition = Ball.Position;
    Frame.BallVelocity = Ball.Velocity;

    // Record player positions (would need actual player actor positions)
    Replay.RecordFrame(Frame);
}

void UForge2SportsManager::TriggerCommentary(const FString& EventType)
{
    for (FForge2CommentaryTrigger& Trigger : CommentaryTriggers)
    {
        if (Trigger.TriggerType == EventType && Trigger.CanTrigger(MatchState.RealTime))
        {
            Trigger.LastTriggeredTime = MatchState.RealTime;
            // Would play audio clip here
            break;
        }
    }
}

void UForge2SportsManager::KickBall(FName PlayerID, const FVector& Target, float Power, float Spin)
{
    FVector Direction = (Target - Ball.Position).GetSafeNormal();
    float KickForce = Power * 5000.0f; // Convert to force

    FVector Force = Direction * KickForce;
    Force.Z += Power * 1000.0f; // Add some lift

    FVector SpinAxis = FVector::CrossProduct(Direction, FVector::UpVector);
    Ball.ApplyKick(Force, SpinAxis, Spin);

    Ball.bIsInPlay = true;
    MatchState.CurrentPlayType = EForge2PlayType::FreePlay;
}

void UForge2SportsManager::PassBall(FName FromPlayerID, FName ToPlayerID, float Power)
{
    // Find target player position (simplified - would need actual player locations)
    FForge2SportsPlayer* FromPlayer = HomeTeam.GetPlayerByID(FromPlayerID);
    if (!FromPlayer) FromPlayer = AwayTeam.GetPlayerByID(FromPlayerID);

    FForge2SportsPlayer* ToPlayer = HomeTeam.GetPlayerByID(ToPlayerID);
    if (!ToPlayer) ToPlayer = AwayTeam.GetPlayerByID(ToPlayerID);

    if (FromPlayer && ToPlayer)
    {
        FromPlayer->Passes++;

        // Pass success based on stats
        float SuccessChance = FromPlayer->Stats.Passing / 100.0f;
        if (FMath::FRand() < SuccessChance)
        {
            FromPlayer->PassesCompleted++;
        }
    }
}

void UForge2SportsManager::ShootBall(FName PlayerID, const FVector& Target, float Power, float Spin)
{
    FForge2SportsPlayer* Player = HomeTeam.GetPlayerByID(PlayerID);
    if (!Player) Player = AwayTeam.GetPlayerByID(PlayerID);

    if (Player)
    {
        Player->Shots++;

        // Calculate accuracy based on stats
        float Accuracy = Player->Stats.Shooting / 100.0f;
        FVector ActualTarget = Target;

        // Add inaccuracy
        float Inaccuracy = (1.0f - Accuracy) * 200.0f;
        ActualTarget.X += FMath::FRandRange(-Inaccuracy, Inaccuracy);
        ActualTarget.Y += FMath::FRandRange(-Inaccuracy, Inaccuracy);
        ActualTarget.Z += FMath::FRandRange(-Inaccuracy * 0.5f, Inaccuracy * 0.5f);

        // Check if on target (within goal bounds)
        if (FMath::Abs(ActualTarget.Y) < GoalSize.X / 2.0f &&
            ActualTarget.Z > 0.0f && ActualTarget.Z < GoalSize.Y)
        {
            Player->ShotsOnTarget++;
        }

        KickBall(PlayerID, ActualTarget, Power, Spin);
        TriggerCommentary(TEXT("shot"));
    }
}

void UForge2SportsManager::TackleBall(FName PlayerID, FName TargetPlayerID)
{
    FForge2SportsPlayer* Tackler = HomeTeam.GetPlayerByID(PlayerID);
    if (!Tackler) Tackler = AwayTeam.GetPlayerByID(PlayerID);

    FForge2SportsPlayer* Target = HomeTeam.GetPlayerByID(TargetPlayerID);
    if (!Target) Target = AwayTeam.GetPlayerByID(TargetPlayerID);

    if (Tackler && Target)
    {
        Tackler->Tackles++;

        // Tackle success based on stats
        float SuccessChance = Tackler->Stats.Tackling / (Tackler->Stats.Tackling + Target->Stats.Dribbling);

        // Check for foul
        float ContactForce = Tackler->Stats.Aggression * 0.5f + FMath::FRandRange(0.0f, 50.0f);
        EForge2FoulType Foul = Referee.DetectFoul(*Tackler, *Target, ContactForce);

        if (Foul != EForge2FoulType::None)
        {
            OnFoulCommitted(PlayerID, Foul);
        }
    }
}

void UForge2SportsManager::SubstitutePlayer(FName TeamID, FName OutPlayerID, FName InPlayerID)
{
    FForge2SportsTeam* Team = nullptr;
    if (TeamID == HomeTeam.TeamID)
    {
        Team = &HomeTeam;
    }
    else if (TeamID == AwayTeam.TeamID)
    {
        Team = &AwayTeam;
    }

    if (Team && Team->SubstitutionsRemaining > 0)
    {
        // Find players in roster and swap in starting lineup
        int32 OutIndex = -1;
        int32 InIndex = -1;

        for (int32 i = 0; i < Team->Roster.Num(); ++i)
        {
            if (Team->Roster[i].PlayerID == OutPlayerID) OutIndex = i;
            if (Team->Roster[i].PlayerID == InPlayerID) InIndex = i;
        }

        if (OutIndex >= 0 && InIndex >= 0)
        {
            // Swap in starting lineup
            for (int32& LineupIndex : Team->StartingLineup)
            {
                if (LineupIndex == OutIndex)
                {
                    LineupIndex = InIndex;
                    Team->SubstitutionsRemaining--;
                    break;
                }
            }
        }
    }
}

void UForge2SportsManager::CallTimeout(FName TeamID)
{
    FForge2SportsTeam* Team = nullptr;
    if (TeamID == HomeTeam.TeamID)
    {
        Team = &HomeTeam;
    }
    else if (TeamID == AwayTeam.TeamID)
    {
        Team = &AwayTeam;
    }

    if (Team && Team->TimeoutsRemaining > 0)
    {
        Team->TimeoutsRemaining--;
        MatchState.State = EForge2MatchState::Timeout;
    }
}

FForge2SportsPlayer* UForge2SportsManager::GetNearestPlayerToBall(FName TeamID)
{
    FForge2SportsTeam* Team = nullptr;
    if (TeamID == HomeTeam.TeamID)
    {
        Team = &HomeTeam;
    }
    else if (TeamID == AwayTeam.TeamID)
    {
        Team = &AwayTeam;
    }

    if (!Team) return nullptr;

    // Would need actual player positions
    // Returning first player for now
    if (Team->Roster.Num() > 0)
    {
        return &Team->Roster[0];
    }

    return nullptr;
}

bool UForge2SportsManager::IsGoal(const FVector& BallPosition) const
{
    // Check if ball crossed goal line within goal posts
    bool bCrossedLine = FMath::Abs(BallPosition.X) > FieldSize.X / 2.0f;
    bool bWithinPosts = FMath::Abs(BallPosition.Y) < GoalSize.X / 2.0f;
    bool bUnderCrossbar = BallPosition.Z < GoalSize.Y;

    return bCrossedLine && bWithinPosts && bUnderCrossbar;
}

bool UForge2SportsManager::IsOutOfBounds(const FVector& BallPosition) const
{
    return FMath::Abs(BallPosition.X) > FieldSize.X / 2.0f + Ball.Radius ||
           FMath::Abs(BallPosition.Y) > FieldSize.Y / 2.0f + Ball.Radius;
}

void UForge2SportsManager::OnGoalScored(FName TeamID, FName ScorerID, FName AssisterID)
{
    if (TeamID == HomeTeam.TeamID)
    {
        HomeTeam.Score++;
    }
    else if (TeamID == AwayTeam.TeamID)
    {
        AwayTeam.Score++;
    }

    // Update player stats
    FForge2SportsPlayer* Scorer = HomeTeam.GetPlayerByID(ScorerID);
    if (!Scorer) Scorer = AwayTeam.GetPlayerByID(ScorerID);
    if (Scorer) Scorer->Goals++;

    FForge2SportsPlayer* Assister = HomeTeam.GetPlayerByID(AssisterID);
    if (!Assister) Assister = AwayTeam.GetPlayerByID(AssisterID);
    if (Assister) Assister->Assists++;

    // Reset for kickoff
    Ball.Position = FVector::ZeroVector;
    Ball.Velocity = FVector::ZeroVector;
    Ball.bIsInPlay = true;
    MatchState.State = EForge2MatchState::Kickoff;

    TriggerCommentary(TEXT("goal"));
}

void UForge2SportsManager::OnFoulCommitted(FName FoulerID, EForge2FoulType FoulType)
{
    FForge2SportsPlayer* Fouler = HomeTeam.GetPlayerByID(FoulerID);
    FForge2SportsTeam* FoulerTeam = &HomeTeam;

    if (!Fouler)
    {
        Fouler = AwayTeam.GetPlayerByID(FoulerID);
        FoulerTeam = &AwayTeam;
    }

    if (Fouler && FoulerTeam)
    {
        Fouler->Fouls++;
        FoulerTeam->Fouls++;

        switch (FoulType)
        {
            case EForge2FoulType::YellowCard:
                Fouler->YellowCards++;
                if (Fouler->YellowCards >= 2)
                {
                    Fouler->RedCards++;
                }
                break;

            case EForge2FoulType::RedCard:
                Fouler->RedCards++;
                break;

            default:
                break;
        }

        // Set up free kick
        MatchState.CurrentPlayType = EForge2PlayType::FreeKick;
        MatchState.SetPieceLocation = Ball.Position;
        Ball.bIsInPlay = false;

        TriggerCommentary(TEXT("foul"));
    }
}

// ============================================================================
// UForge2SportsUtilities
// ============================================================================

FForge2SportsPlayer UForge2SportsUtilities::CreatePlayer(const FString& Name, int32 JerseyNumber, EForge2PlayerPosition Position, float OverallRating)
{
    FForge2SportsPlayer Player;
    Player.PlayerID = *Name.Replace(TEXT(" "), TEXT("_"));
    Player.PlayerName = Name;
    Player.JerseyNumber = JerseyNumber;
    Player.Position = Position;

    // Set stats based on overall rating with some variation
    float BaseRating = OverallRating;
    Player.Stats.Speed = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Acceleration = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Stamina = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Strength = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Jumping = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Agility = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Shooting = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Passing = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Dribbling = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Defense = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Heading = BaseRating + FMath::FRandRange(-10.0f, 10.0f);
    Player.Stats.Tackling = BaseRating + FMath::FRandRange(-10.0f, 10.0f);

    // Clamp all stats to 1-99
    Player.Stats.Speed = FMath::Clamp(Player.Stats.Speed, 1.0f, 99.0f);
    Player.Stats.Acceleration = FMath::Clamp(Player.Stats.Acceleration, 1.0f, 99.0f);
    Player.Stats.Stamina = FMath::Clamp(Player.Stats.Stamina, 1.0f, 99.0f);
    Player.Stats.Strength = FMath::Clamp(Player.Stats.Strength, 1.0f, 99.0f);
    Player.Stats.Jumping = FMath::Clamp(Player.Stats.Jumping, 1.0f, 99.0f);
    Player.Stats.Agility = FMath::Clamp(Player.Stats.Agility, 1.0f, 99.0f);
    Player.Stats.Shooting = FMath::Clamp(Player.Stats.Shooting, 1.0f, 99.0f);
    Player.Stats.Passing = FMath::Clamp(Player.Stats.Passing, 1.0f, 99.0f);
    Player.Stats.Dribbling = FMath::Clamp(Player.Stats.Dribbling, 1.0f, 99.0f);
    Player.Stats.Defense = FMath::Clamp(Player.Stats.Defense, 1.0f, 99.0f);
    Player.Stats.Heading = FMath::Clamp(Player.Stats.Heading, 1.0f, 99.0f);
    Player.Stats.Tackling = FMath::Clamp(Player.Stats.Tackling, 1.0f, 99.0f);

    return Player;
}

FForge2SportsTeam UForge2SportsUtilities::CreateTeam(const FString& Name, FLinearColor Primary, FLinearColor Secondary)
{
    FForge2SportsTeam Team;
    Team.TeamID = *Name.Replace(TEXT(" "), TEXT("_"));
    Team.TeamName = Name;
    Team.PrimaryColor = Primary;
    Team.SecondaryColor = Secondary;

    return Team;
}

FForge2Formation UForge2SportsUtilities::CreateFormation442()
{
    FForge2Formation Formation;
    Formation.FormationID = TEXT("Formation_442");
    Formation.FormationName = TEXT("4-4-2");
    Formation.SportType = EForge2SportType::Soccer;
    Formation.DefensiveStrength = 0.5f;
    Formation.OffensiveStrength = 0.5f;

    // GK
    Formation.Positions.Add(FVector2D(0.05f, 0.5f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Goalkeeper);

    // Defenders (4)
    Formation.Positions.Add(FVector2D(0.2f, 0.15f));
    Formation.Positions.Add(FVector2D(0.2f, 0.38f));
    Formation.Positions.Add(FVector2D(0.2f, 0.62f));
    Formation.Positions.Add(FVector2D(0.2f, 0.85f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);

    // Midfielders (4)
    Formation.Positions.Add(FVector2D(0.5f, 0.15f));
    Formation.Positions.Add(FVector2D(0.5f, 0.38f));
    Formation.Positions.Add(FVector2D(0.5f, 0.62f));
    Formation.Positions.Add(FVector2D(0.5f, 0.85f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);

    // Forwards (2)
    Formation.Positions.Add(FVector2D(0.8f, 0.35f));
    Formation.Positions.Add(FVector2D(0.8f, 0.65f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Forward);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Forward);

    return Formation;
}

FForge2Formation UForge2SportsUtilities::CreateFormation433()
{
    FForge2Formation Formation;
    Formation.FormationID = TEXT("Formation_433");
    Formation.FormationName = TEXT("4-3-3");
    Formation.SportType = EForge2SportType::Soccer;
    Formation.DefensiveStrength = 0.45f;
    Formation.OffensiveStrength = 0.6f;

    // GK
    Formation.Positions.Add(FVector2D(0.05f, 0.5f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Goalkeeper);

    // Defenders (4)
    Formation.Positions.Add(FVector2D(0.2f, 0.15f));
    Formation.Positions.Add(FVector2D(0.2f, 0.38f));
    Formation.Positions.Add(FVector2D(0.2f, 0.62f));
    Formation.Positions.Add(FVector2D(0.2f, 0.85f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);

    // Midfielders (3)
    Formation.Positions.Add(FVector2D(0.45f, 0.25f));
    Formation.Positions.Add(FVector2D(0.45f, 0.5f));
    Formation.Positions.Add(FVector2D(0.45f, 0.75f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);

    // Forwards (3)
    Formation.Positions.Add(FVector2D(0.8f, 0.2f));
    Formation.Positions.Add(FVector2D(0.85f, 0.5f));
    Formation.Positions.Add(FVector2D(0.8f, 0.8f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Forward);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Forward);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Forward);

    return Formation;
}

FForge2Formation UForge2SportsUtilities::CreateFormation352()
{
    FForge2Formation Formation;
    Formation.FormationID = TEXT("Formation_352");
    Formation.FormationName = TEXT("3-5-2");
    Formation.SportType = EForge2SportType::Soccer;
    Formation.DefensiveStrength = 0.4f;
    Formation.OffensiveStrength = 0.55f;

    // GK
    Formation.Positions.Add(FVector2D(0.05f, 0.5f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Goalkeeper);

    // Defenders (3)
    Formation.Positions.Add(FVector2D(0.2f, 0.25f));
    Formation.Positions.Add(FVector2D(0.2f, 0.5f));
    Formation.Positions.Add(FVector2D(0.2f, 0.75f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Defender);

    // Midfielders (5)
    Formation.Positions.Add(FVector2D(0.4f, 0.1f));
    Formation.Positions.Add(FVector2D(0.45f, 0.35f));
    Formation.Positions.Add(FVector2D(0.45f, 0.5f));
    Formation.Positions.Add(FVector2D(0.45f, 0.65f));
    Formation.Positions.Add(FVector2D(0.4f, 0.9f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Midfielder);

    // Forwards (2)
    Formation.Positions.Add(FVector2D(0.8f, 0.35f));
    Formation.Positions.Add(FVector2D(0.8f, 0.65f));
    Formation.PositionTypes.Add(EForge2PlayerPosition::Forward);
    Formation.PositionTypes.Add(EForge2PlayerPosition::Forward);

    return Formation;
}

FForge2BallPhysics UForge2SportsUtilities::CreateSoccerBall()
{
    FForge2BallPhysics Ball;
    Ball.Radius = 11.0f;      // ~22cm diameter
    Ball.Mass = 0.43f;        // 430g
    Ball.Bounciness = 0.75f;
    Ball.Friction = 0.5f;
    Ball.AirResistance = 0.01f;
    Ball.MagnusCoefficient = 0.05f;
    return Ball;
}

FForge2BallPhysics UForge2SportsUtilities::CreateBasketball()
{
    FForge2BallPhysics Ball;
    Ball.Radius = 12.1f;      // ~24cm diameter
    Ball.Mass = 0.62f;        // 620g
    Ball.Bounciness = 0.85f;
    Ball.Friction = 0.6f;
    Ball.AirResistance = 0.015f;
    Ball.MagnusCoefficient = 0.03f;
    return Ball;
}

FForge2BallPhysics UForge2SportsUtilities::CreateFootball()
{
    FForge2BallPhysics Ball;
    Ball.Radius = 8.5f;       // Prolate spheroid approximation
    Ball.Mass = 0.42f;        // 420g
    Ball.Bounciness = 0.7f;
    Ball.Friction = 0.55f;
    Ball.AirResistance = 0.02f;
    Ball.MagnusCoefficient = 0.04f;
    return Ball;
}
