// Forge2SportsSystem.h
// FORGE2 Sports Genre System
// Provides sports game mechanics: player stats, ball physics, teams, seasons, referees, replays

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Forge2SportsSystem.generated.h"

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EForge2SportType : uint8
{
    Soccer,
    Basketball,
    Football,
    Baseball,
    Hockey,
    Tennis,
    Golf,
    Racing,
    Custom
};

UENUM(BlueprintType)
enum class EForge2PlayerPosition : uint8
{
    Goalkeeper,
    Defender,
    Midfielder,
    Forward,
    PointGuard,
    ShootingGuard,
    SmallForward,
    PowerForward,
    Center,
    Quarterback,
    RunningBack,
    WideReceiver,
    Pitcher,
    Batter,
    Goalie,
    Custom
};

UENUM(BlueprintType)
enum class EForge2MatchState : uint8
{
    PreMatch,
    Kickoff,
    Playing,
    Paused,
    HalfTime,
    Timeout,
    InjuryTime,
    Overtime,
    PenaltyShootout,
    PostMatch
};

UENUM(BlueprintType)
enum class EForge2FoulType : uint8
{
    None,
    MinorFoul,
    MajorFoul,
    TechnicalFoul,
    FlagrantFoul,
    YellowCard,
    RedCard,
    Offside,
    OutOfBounds,
    Traveling,
    DoubleDribble
};

UENUM(BlueprintType)
enum class EForge2PlayType : uint8
{
    FreePlay,
    SetPiece,
    FreeKick,
    PenaltyKick,
    CornerKick,
    ThrowIn,
    FreeThrow,
    Inbound,
    Serve
};

// ============================================================================
// Player Stats
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlayerStats
{
    GENERATED_BODY()

    // Physical attributes (0-100)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Acceleration = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Stamina = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Strength = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Jumping = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Agility = 70.0f;

    // Technical attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Shooting = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Passing = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Dribbling = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Defense = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Heading = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Tackling = 70.0f;

    // Mental attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Composure = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Vision = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Positioning = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggression = 50.0f;

    // Goalkeeper specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Diving = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Handling = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Reflexes = 50.0f;

    float GetOverall() const;
    float GetAttributeValue(const FName& AttributeName) const;
};

// ============================================================================
// Player Definition
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SportsPlayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 JerseyNumber = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PlayerPosition Position = EForge2PlayerPosition::Midfielder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PlayerPosition SecondaryPosition = EForge2PlayerPosition::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2PlayerStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentMorale = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InjuryRisk = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInjured = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InjuryDaysRemaining = 0;

    // Match stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Goals = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Assists = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Shots = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShotsOnTarget = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Passes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PassesCompleted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Tackles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Fouls = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 YellowCards = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RedCards = 0;

    void ResetMatchStats();
    float GetPassAccuracy() const;
    float GetShotAccuracy() const;
};

// ============================================================================
// Ball Physics
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BallPhysics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AngularVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 11.0f;             // Ball radius in cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Mass = 0.43f;               // Mass in kg

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Bounciness = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Friction = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AirResistance = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MagnusCoefficient = 0.05f;  // For spin effects

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PossessorID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInPlay = true;

    void Update(float DeltaTime);
    void ApplyKick(const FVector& Force, const FVector& SpinAxis, float SpinAmount);
    void ApplyBounce(const FVector& Normal);
    FVector CalculateMagnusForce() const;
};

// ============================================================================
// Team Definition
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SportsTeam
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TeamName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2SportsPlayer> Roster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> StartingLineup;     // Indices into Roster

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName FormationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Score = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Fouls = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimeoutsRemaining = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SubstitutionsRemaining = 3;

    float GetTeamOverall() const;
    FForge2SportsPlayer* GetPlayerByID(FName PlayerID);
};

// ============================================================================
// Formation System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Formation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName FormationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FormationName;              // e.g., "4-4-2", "3-5-2"

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2SportType SportType = EForge2SportType::Soccer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> Positions;        // Normalized positions (0-1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EForge2PlayerPosition> PositionTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefensiveStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OffensiveStrength = 0.5f;

    TArray<FVector> GetWorldPositions(const FVector& FieldCenter, const FVector& FieldSize, bool bAttackingRight) const;
};

// ============================================================================
// Play/Tactics System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlayDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PlayID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PlayType PlayType = EForge2PlayType::FreePlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> PlayerRoutes;     // Movement paths for each player

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> RouteTiming;          // When each player starts route

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PrimaryReceiver = 0;          // Index of primary target

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuccessRate = 0.5f;           // AI difficulty modifier
};

// ============================================================================
// Referee System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RefereeSystem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Strictness = 0.5f;            // 0 = lenient, 1 = strict

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FoulDetectionRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OffsideLineX = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bVAREnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VARReviewChance = 0.5f;

    EForge2FoulType DetectFoul(const FForge2SportsPlayer& Fouler, const FForge2SportsPlayer& Victim, float ContactForce);
    bool CheckOffside(const FVector& AttackerPosition, const FVector& LastDefenderPosition);
    bool ShouldVARReview(EForge2FoulType FoulType) const;
};

// ============================================================================
// Commentary System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CommentaryTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TriggerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TriggerType;                // "goal", "foul", "save", "miss", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> CommentaryLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AudioClips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cooldown = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastTriggeredTime = -1000.0f;

    bool CanTrigger(float CurrentTime) const;
    FString GetRandomLine() const;
};

// ============================================================================
// Replay System
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ReplayFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector BallPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector BallVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PlayerPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRotator> PlayerRotations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> PlayerAnimStates;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ReplaySystem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2ReplayFrame> Frames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecordingFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRecording = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlaying = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlaybackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentPlaybackTime = 0.0f;

    void RecordFrame(const FForge2ReplayFrame& Frame);
    FForge2ReplayFrame GetFrameAtTime(float Time) const;
    void TrimOldFrames();
    void StartPlayback(float StartTime);
    void StopPlayback();
};

// ============================================================================
// Season/Career Mode
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SeasonMatch
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MatchID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HomeTeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AwayTeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HomeScore = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AwayScore = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Week = 0;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SeasonStanding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Played = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Won = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Drawn = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Lost = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoalsFor = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoalsAgainst = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Points = 0;

    int32 GetGoalDifference() const { return GoalsFor - GoalsAgainst; }
    void UpdateFromMatch(int32 Scored, int32 Conceded);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Season
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SeasonID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SeasonName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> TeamIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2SeasonMatch> Schedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2SeasonStanding> Standings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentWeek = 0;

    void GenerateSchedule();
    void UpdateStandings(const FForge2SeasonMatch& Match);
    void SortStandings();
    TArray<FForge2SeasonMatch> GetMatchesForWeek(int32 Week) const;
};

// ============================================================================
// Match State
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MatchState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2MatchState State = EForge2MatchState::PreMatch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MatchTime = 0.0f;             // In-game time in minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RealTime = 0.0f;              // Actual elapsed time

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Period = 1;                   // Half, quarter, inning, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PeriodLength = 45.0f;         // Minutes per period

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InjuryTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeMultiplier = 3.0f;        // Real seconds per game minute

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PossessingTeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PlayType CurrentPlayType = EForge2PlayType::FreePlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SetPieceLocation = FVector::ZeroVector;

    bool IsPeriodOver() const;
    bool IsMatchOver() const;
    void AdvancePeriod();
};

// ============================================================================
// Sports Manager
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2SportsManager : public UObject
{
    GENERATED_BODY()

public:
    UForge2SportsManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2SportType SportType = EForge2SportType::Soccer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2SportsTeam HomeTeam;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2SportsTeam AwayTeam;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2BallPhysics Ball;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2MatchState MatchState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2RefereeSystem Referee;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FForge2ReplaySystem Replay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2Formation> Formations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PlayDefinition> Plays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2CommentaryTrigger> CommentaryTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FieldSize = FVector(10500.0f, 6800.0f, 0.0f);   // FIFA standard pitch

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector GoalSize = FVector(732.0f, 244.0f, 0.0f);       // FIFA standard goal

    // Initialization
    UFUNCTION(BlueprintCallable)
    void InitializeMatch(const FForge2SportsTeam& Home, const FForge2SportsTeam& Away);

    // Update
    UFUNCTION(BlueprintCallable)
    void UpdateMatch(float DeltaTime);

    // Actions
    UFUNCTION(BlueprintCallable)
    void KickBall(FName PlayerID, const FVector& Target, float Power, float Spin);

    UFUNCTION(BlueprintCallable)
    void PassBall(FName FromPlayerID, FName ToPlayerID, float Power);

    UFUNCTION(BlueprintCallable)
    void ShootBall(FName PlayerID, const FVector& Target, float Power, float Spin);

    UFUNCTION(BlueprintCallable)
    void TackleBall(FName PlayerID, FName TargetPlayerID);

    UFUNCTION(BlueprintCallable)
    void SubstitutePlayer(FName TeamID, FName OutPlayerID, FName InPlayerID);

    UFUNCTION(BlueprintCallable)
    void CallTimeout(FName TeamID);

    // Queries
    UFUNCTION(BlueprintCallable)
    FForge2SportsPlayer* GetNearestPlayerToBall(FName TeamID);

    UFUNCTION(BlueprintCallable)
    bool IsGoal(const FVector& BallPosition) const;

    UFUNCTION(BlueprintCallable)
    bool IsOutOfBounds(const FVector& BallPosition) const;

    // Events
    UFUNCTION(BlueprintCallable)
    void OnGoalScored(FName TeamID, FName ScorerID, FName AssisterID);

    UFUNCTION(BlueprintCallable)
    void OnFoulCommitted(FName FoulerID, EForge2FoulType FoulType);

private:
    void UpdateBallPhysics(float DeltaTime);
    void UpdatePlayerPositions(float DeltaTime);
    void UpdateMatchTime(float DeltaTime);
    void CheckBallBoundaries();
    void RecordReplayFrame();
    void TriggerCommentary(const FString& EventType);
};

// ============================================================================
// Utility Functions
// ============================================================================

UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2SportsUtilities : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static FForge2SportsPlayer CreatePlayer(const FString& Name, int32 JerseyNumber, EForge2PlayerPosition Position, float OverallRating);

    UFUNCTION(BlueprintCallable)
    static FForge2SportsTeam CreateTeam(const FString& Name, FLinearColor Primary, FLinearColor Secondary);

    UFUNCTION(BlueprintCallable)
    static FForge2Formation CreateFormation442();

    UFUNCTION(BlueprintCallable)
    static FForge2Formation CreateFormation433();

    UFUNCTION(BlueprintCallable)
    static FForge2Formation CreateFormation352();

    UFUNCTION(BlueprintCallable)
    static FForge2BallPhysics CreateSoccerBall();

    UFUNCTION(BlueprintCallable)
    static FForge2BallPhysics CreateBasketball();

    UFUNCTION(BlueprintCallable)
    static FForge2BallPhysics CreateFootball();
};
