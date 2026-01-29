// Forge2RhythmSystem.h - Rhythm Game Genre System
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2RhythmSystem.generated.h"

// Forward declarations
class UForge2RhythmManager;

// Enums
UENUM(BlueprintType)
enum class EForge2RhythmNoteType : uint8
{
    Tap,
    Hold,
    Slide,
    Flick,
    Simultaneous,
    Trail
};

UENUM(BlueprintType)
enum class EForge2RhythmJudgment : uint8
{
    Perfect,
    Great,
    Good,
    Bad,
    Miss
};

UENUM(BlueprintType)
enum class EForge2RhythmDifficulty : uint8
{
    Easy,
    Normal,
    Hard,
    Expert,
    Master
};

UENUM(BlueprintType)
enum class EForge2RhythmLane : uint8
{
    Lane1,
    Lane2,
    Lane3,
    Lane4,
    Lane5,
    Lane6,
    Lane7,
    Lane8
};

UENUM(BlueprintType)
enum class EForge2RhythmGameMode : uint8
{
    Standard,
    Endless,
    Practice,
    Versus,
    Cooperative
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmTimingWindow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PerfectWindow = 0.033f;  // 33ms
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GreatWindow = 0.066f;    // 66ms
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GoodWindow = 0.100f;     // 100ms
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BadWindow = 0.150f;      // 150ms
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MissWindow = 0.200f;     // 200ms

    EForge2RhythmJudgment GetJudgment(float TimingError) const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmNote
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 NoteId = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2RhythmNoteType Type = EForge2RhythmNoteType::Tap;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2RhythmLane Lane = EForge2RhythmLane::Lane1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BeatTime = 0.0f;  // Time in beats
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HitTime = 0.0f;   // Time in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration = 0.0f;  // For hold notes
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EndTime = 0.0f;   // For hold/slide notes
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2RhythmLane EndLane = EForge2RhythmLane::Lane1;  // For slides
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SimultaneousGroup = -1;  // Notes hit together
    UPROPERTY() bool bHit = false;
    UPROPERTY() bool bMissed = false;
    UPROPERTY() EForge2RhythmJudgment Judgment = EForge2RhythmJudgment::Miss;
    UPROPERTY() float ActualHitTime = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmBeatMap
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString MapId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SongName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Artist;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2RhythmDifficulty Difficulty = EForge2RhythmDifficulty::Normal;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 DifficultyRating = 5;  // 1-15 scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BPM = 120.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Offset = 0.0f;  // Audio offset in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration = 0.0f;  // Song length
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 LaneCount = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2RhythmNote> Notes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USoundWave> AudioTrack;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> CoverArt;

    float BeatsToSeconds(float Beats) const;
    float SecondsToBeats(float Seconds) const;
    void SortNotes();
    int32 GetNoteCount() const { return Notes.Num(); }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmComboState
{
    GENERATED_BODY()

    UPROPERTY() int32 CurrentCombo = 0;
    UPROPERTY() int32 MaxCombo = 0;
    UPROPERTY() int32 PerfectCount = 0;
    UPROPERTY() int32 GreatCount = 0;
    UPROPERTY() int32 GoodCount = 0;
    UPROPERTY() int32 BadCount = 0;
    UPROPERTY() int32 MissCount = 0;
    UPROPERTY() float Accuracy = 0.0f;
    UPROPERTY() int32 TotalNotes = 0;
    UPROPERTY() int32 NotesHit = 0;

    void RegisterHit(EForge2RhythmJudgment Judgment);
    void RegisterMiss();
    void Reset();
    void UpdateAccuracy();
    FString GetGrade() const;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmScore
{
    GENERATED_BODY()

    UPROPERTY() int32 Score = 0;
    UPROPERTY() int32 BaseNoteScore = 1000;
    UPROPERTY() float PerfectMultiplier = 1.0f;
    UPROPERTY() float GreatMultiplier = 0.8f;
    UPROPERTY() float GoodMultiplier = 0.5f;
    UPROPERTY() float BadMultiplier = 0.25f;
    UPROPERTY() int32 ComboBonus = 0;
    UPROPERTY() float ComboMultiplier = 1.0f;

    void AddNoteScore(EForge2RhythmJudgment Judgment, int32 Combo);
    void CalculateComboMultiplier(int32 Combo);
    void Reset();
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmSongResult
{
    GENERATED_BODY()

    UPROPERTY() FString MapId;
    UPROPERTY() FForge2RhythmScore FinalScore;
    UPROPERTY() FForge2RhythmComboState ComboStats;
    UPROPERTY() FString Grade;
    UPROPERTY() bool bFullCombo = false;
    UPROPERTY() bool bAllPerfect = false;
    UPROPERTY() bool bNewHighScore = false;
    UPROPERTY() int32 PreviousHighScore = 0;
    UPROPERTY() float CompletionTime = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMirror = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRandom = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpeedMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bNoFail = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSuddenDeath = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHidden = false;  // Notes fade
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFlashlight = false;  // Limited view
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ScoreMultiplier = 1.0f;

    void ApplyToNotes(TArray<FForge2RhythmNote>& Notes, int32 LaneCount);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmLaneState
{
    GENERATED_BODY()

    UPROPERTY() EForge2RhythmLane Lane = EForge2RhythmLane::Lane1;
    UPROPERTY() bool bPressed = false;
    UPROPERTY() float PressTime = 0.0f;
    UPROPERTY() int32 CurrentHoldNote = -1;  // ID of note being held
    UPROPERTY() float HoldProgress = 0.0f;  // 0-1 for held note

    void Press(float Time);
    void Release(float Time);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmPlayState
{
    GENERATED_BODY()

    UPROPERTY() float CurrentTime = 0.0f;
    UPROPERTY() float CurrentBeat = 0.0f;
    UPROPERTY() bool bPlaying = false;
    UPROPERTY() bool bPaused = false;
    UPROPERTY() bool bFailed = false;
    UPROPERTY() int32 NextNoteIndex = 0;
    UPROPERTY() float Health = 100.0f;
    UPROPERTY() float MaxHealth = 100.0f;
    UPROPERTY() TArray<FForge2RhythmLaneState> LaneStates;
    UPROPERTY() FForge2RhythmComboState ComboState;
    UPROPERTY() FForge2RhythmScore Score;

    void Initialize(int32 LaneCount);
    void Update(float DeltaTime, const FForge2RhythmBeatMap& Map);
    void TakeDamage(float Amount);
    void Heal(float Amount);
    bool IsFailed() const { return bFailed || Health <= 0.0f; }
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmVisualSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NoteSpeed = 1.0f;  // Note fall speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HitPosition = 0.1f;  // Where notes should be hit (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LaneWidth = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LaneSpacing = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor PerfectColor = FLinearColor(1, 1, 0, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor GreatColor = FLinearColor(0, 1, 0, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor GoodColor = FLinearColor(0, 0.5f, 1, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor BadColor = FLinearColor(0.5f, 0.5f, 0.5f, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MissColor = FLinearColor(1, 0, 0, 1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowComboText = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowJudgmentText = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowProgressBar = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RhythmConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 DefaultLaneCount = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2RhythmTimingWindow TimingWindows;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2RhythmVisualSettings VisualSettings;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HealthDrainRate = 1.0f;  // Per miss
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HealthRecoveryRate = 0.5f;  // Per hit
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AudioLatencyOffset = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float InputLatencyOffset = 0.0f;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2RhythmManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2RhythmManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2RhythmConfig Config;
    UPROPERTY(BlueprintReadOnly) FForge2RhythmPlayState PlayState;
    UPROPERTY() FForge2RhythmBeatMap CurrentMap;
    UPROPERTY() FForge2RhythmModifier CurrentModifiers;
    UPROPERTY() TMap<FString, FForge2RhythmBeatMap> LoadedMaps;
    UPROPERTY() TMap<FString, int32> HighScores;

    UFUNCTION(BlueprintCallable) void Initialize(const FForge2RhythmConfig& InConfig);
    UFUNCTION(BlueprintCallable) void Update(float DeltaTime);

    // Song management
    UFUNCTION(BlueprintCallable) bool LoadBeatMap(const FForge2RhythmBeatMap& Map);
    UFUNCTION(BlueprintCallable) void StartSong(const FString& MapId, const FForge2RhythmModifier& Modifiers);
    UFUNCTION(BlueprintCallable) void PauseSong();
    UFUNCTION(BlueprintCallable) void ResumeSong();
    UFUNCTION(BlueprintCallable) void StopSong();
    UFUNCTION(BlueprintCallable) void RestartSong();

    // Input
    UFUNCTION(BlueprintCallable) void PressLane(int32 LaneIndex);
    UFUNCTION(BlueprintCallable) void ReleaseLane(int32 LaneIndex);

    // Queries
    UFUNCTION(BlueprintPure) bool IsPlaying() const { return PlayState.bPlaying && !PlayState.bPaused; }
    UFUNCTION(BlueprintPure) float GetProgress() const;
    UFUNCTION(BlueprintPure) TArray<FForge2RhythmNote> GetVisibleNotes(float LookAhead) const;
    UFUNCTION(BlueprintPure) FForge2RhythmSongResult GetCurrentResult() const;
    UFUNCTION(BlueprintPure) int32 GetHighScore(const FString& MapId) const;

    // Beat map creation
    UFUNCTION(BlueprintCallable) void AddNote(FForge2RhythmNote Note);
    UFUNCTION(BlueprintCallable) void RemoveNote(int32 NoteId);
    UFUNCTION(BlueprintCallable) void ClearNotes();
    UFUNCTION(BlueprintCallable) FForge2RhythmBeatMap CreateEmptyBeatMap(const FString& SongName, float BPM);

private:
    void ProcessInput(int32 LaneIndex, bool bPressed);
    void CheckMissedNotes();
    void ProcessNoteHit(FForge2RhythmNote& Note, float TimingError);
    void ProcessNoteMiss(FForge2RhythmNote& Note);
    void UpdateHoldNotes();
    void FinishSong();
    void SaveHighScore(const FString& MapId, int32 Score);
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2RhythmUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Rhythm|Notes")
    static FForge2RhythmNote CreateTapNote(int32 Lane, float BeatTime);

    UFUNCTION(BlueprintCallable, Category = "Rhythm|Notes")
    static FForge2RhythmNote CreateHoldNote(int32 Lane, float StartBeat, float EndBeat);

    UFUNCTION(BlueprintCallable, Category = "Rhythm|Notes")
    static FForge2RhythmNote CreateSlideNote(int32 StartLane, int32 EndLane, float StartBeat, float EndBeat);

    UFUNCTION(BlueprintCallable, Category = "Rhythm|BeatMap")
    static FForge2RhythmBeatMap CreateSimpleBeatMap(const FString& SongName, float BPM, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Rhythm|BeatMap")
    static void GenerateBasicPattern(FForge2RhythmBeatMap& Map, float StartBeat, float EndBeat, int32 PatternType);

    UFUNCTION(BlueprintCallable, Category = "Rhythm|Config")
    static FForge2RhythmTimingWindow CreateStrictTiming();

    UFUNCTION(BlueprintCallable, Category = "Rhythm|Config")
    static FForge2RhythmTimingWindow CreateRelaxedTiming();

    UFUNCTION(BlueprintCallable, Category = "Rhythm|Analysis")
    static float CalculateDifficulty(const FForge2RhythmBeatMap& Map);

    UFUNCTION(BlueprintPure, Category = "Rhythm|Utility")
    static float BPMToBeatsPerSecond(float BPM);

    UFUNCTION(BlueprintPure, Category = "Rhythm|Utility")
    static float BeatsToMilliseconds(float Beats, float BPM);
};
