// Forge2RhythmSystem.cpp - Rhythm Game Genre Implementation
#include "Forge2RhythmSystem.h"

// FForge2RhythmTimingWindow Implementation
EForge2RhythmJudgment FForge2RhythmTimingWindow::GetJudgment(float TimingError) const
{
    float AbsError = FMath::Abs(TimingError);

    if (AbsError <= PerfectWindow)
        return EForge2RhythmJudgment::Perfect;
    if (AbsError <= GreatWindow)
        return EForge2RhythmJudgment::Great;
    if (AbsError <= GoodWindow)
        return EForge2RhythmJudgment::Good;
    if (AbsError <= BadWindow)
        return EForge2RhythmJudgment::Bad;

    return EForge2RhythmJudgment::Miss;
}

// FForge2RhythmBeatMap Implementation
float FForge2RhythmBeatMap::BeatsToSeconds(float Beats) const
{
    return (Beats * 60.0f / BPM) + Offset;
}

float FForge2RhythmBeatMap::SecondsToBeats(float Seconds) const
{
    return (Seconds - Offset) * BPM / 60.0f;
}

void FForge2RhythmBeatMap::SortNotes()
{
    Notes.Sort([](const FForge2RhythmNote& A, const FForge2RhythmNote& B)
    {
        return A.HitTime < B.HitTime;
    });

    // Assign IDs
    for (int32 i = 0; i < Notes.Num(); i++)
    {
        Notes[i].NoteId = i;
    }
}

// FForge2RhythmComboState Implementation
void FForge2RhythmComboState::RegisterHit(EForge2RhythmJudgment Judgment)
{
    NotesHit++;

    switch (Judgment)
    {
        case EForge2RhythmJudgment::Perfect:
            PerfectCount++;
            CurrentCombo++;
            break;
        case EForge2RhythmJudgment::Great:
            GreatCount++;
            CurrentCombo++;
            break;
        case EForge2RhythmJudgment::Good:
            GoodCount++;
            CurrentCombo++;
            break;
        case EForge2RhythmJudgment::Bad:
            BadCount++;
            CurrentCombo = 0;
            break;
        default:
            break;
    }

    if (CurrentCombo > MaxCombo)
    {
        MaxCombo = CurrentCombo;
    }

    UpdateAccuracy();
}

void FForge2RhythmComboState::RegisterMiss()
{
    MissCount++;
    CurrentCombo = 0;
    UpdateAccuracy();
}

void FForge2RhythmComboState::Reset()
{
    CurrentCombo = 0;
    MaxCombo = 0;
    PerfectCount = 0;
    GreatCount = 0;
    GoodCount = 0;
    BadCount = 0;
    MissCount = 0;
    Accuracy = 0.0f;
    TotalNotes = 0;
    NotesHit = 0;
}

void FForge2RhythmComboState::UpdateAccuracy()
{
    if (TotalNotes == 0)
    {
        Accuracy = 0.0f;
        return;
    }

    // Weighted accuracy: Perfect=100, Great=80, Good=50, Bad=25, Miss=0
    float WeightedScore = (PerfectCount * 100.0f) + (GreatCount * 80.0f) +
                          (GoodCount * 50.0f) + (BadCount * 25.0f);
    float MaxScore = TotalNotes * 100.0f;

    Accuracy = (WeightedScore / MaxScore) * 100.0f;
}

FString FForge2RhythmComboState::GetGrade() const
{
    if (MissCount == 0 && BadCount == 0 && GoodCount == 0 && GreatCount == 0)
        return TEXT("SSS");  // All Perfect
    if (MissCount == 0 && BadCount == 0 && GoodCount == 0)
        return TEXT("SS");   // Full Combo, no Good
    if (MissCount == 0 && BadCount == 0)
        return TEXT("S");    // Full Combo
    if (Accuracy >= 95.0f)
        return TEXT("A");
    if (Accuracy >= 90.0f)
        return TEXT("B");
    if (Accuracy >= 80.0f)
        return TEXT("C");
    if (Accuracy >= 70.0f)
        return TEXT("D");
    return TEXT("F");
}

// FForge2RhythmScore Implementation
void FForge2RhythmScore::AddNoteScore(EForge2RhythmJudgment Judgment, int32 Combo)
{
    CalculateComboMultiplier(Combo);

    float JudgmentMult = 0.0f;
    switch (Judgment)
    {
        case EForge2RhythmJudgment::Perfect:
            JudgmentMult = PerfectMultiplier;
            break;
        case EForge2RhythmJudgment::Great:
            JudgmentMult = GreatMultiplier;
            break;
        case EForge2RhythmJudgment::Good:
            JudgmentMult = GoodMultiplier;
            break;
        case EForge2RhythmJudgment::Bad:
            JudgmentMult = BadMultiplier;
            break;
        default:
            return;
    }

    int32 NoteScore = FMath::RoundToInt(BaseNoteScore * JudgmentMult * ComboMultiplier);
    Score += NoteScore;
    ComboBonus += FMath::RoundToInt(Combo * 10 * JudgmentMult);
}

void FForge2RhythmScore::CalculateComboMultiplier(int32 Combo)
{
    if (Combo < 10)
        ComboMultiplier = 1.0f;
    else if (Combo < 30)
        ComboMultiplier = 1.1f;
    else if (Combo < 50)
        ComboMultiplier = 1.25f;
    else if (Combo < 100)
        ComboMultiplier = 1.5f;
    else if (Combo < 200)
        ComboMultiplier = 1.75f;
    else
        ComboMultiplier = 2.0f;
}

void FForge2RhythmScore::Reset()
{
    Score = 0;
    ComboBonus = 0;
    ComboMultiplier = 1.0f;
}

// FForge2RhythmModifier Implementation
void FForge2RhythmModifier::ApplyToNotes(TArray<FForge2RhythmNote>& Notes, int32 LaneCount)
{
    if (bMirror)
    {
        for (FForge2RhythmNote& Note : Notes)
        {
            int32 Lane = (int32)Note.Lane;
            Note.Lane = (EForge2RhythmLane)(LaneCount - 1 - Lane);

            int32 EndLane = (int32)Note.EndLane;
            Note.EndLane = (EForge2RhythmLane)(LaneCount - 1 - EndLane);
        }
    }

    if (bRandom)
    {
        TArray<int32> LaneMapping;
        for (int32 i = 0; i < LaneCount; i++)
        {
            LaneMapping.Add(i);
        }

        // Shuffle lanes
        for (int32 i = LaneMapping.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            LaneMapping.Swap(i, j);
        }

        for (FForge2RhythmNote& Note : Notes)
        {
            int32 Lane = (int32)Note.Lane;
            Note.Lane = (EForge2RhythmLane)LaneMapping[Lane];

            int32 EndLane = (int32)Note.EndLane;
            Note.EndLane = (EForge2RhythmLane)LaneMapping[EndLane];
        }
    }

    // Adjust speed
    if (SpeedMultiplier != 1.0f)
    {
        // Speed affects visual approach, not actual timing
        // This is handled in the visual system
    }

    // Calculate score multiplier
    ScoreMultiplier = 1.0f;
    if (bNoFail) ScoreMultiplier *= 0.5f;
    if (bSuddenDeath) ScoreMultiplier *= 1.2f;
    if (bHidden) ScoreMultiplier *= 1.1f;
    if (bFlashlight) ScoreMultiplier *= 1.15f;
    if (SpeedMultiplier > 1.5f) ScoreMultiplier *= 1.1f;
    if (SpeedMultiplier < 0.75f) ScoreMultiplier *= 0.8f;
}

// FForge2RhythmLaneState Implementation
void FForge2RhythmLaneState::Press(float Time)
{
    bPressed = true;
    PressTime = Time;
}

void FForge2RhythmLaneState::Release(float Time)
{
    bPressed = false;
    CurrentHoldNote = -1;
    HoldProgress = 0.0f;
}

// FForge2RhythmPlayState Implementation
void FForge2RhythmPlayState::Initialize(int32 LaneCount)
{
    CurrentTime = 0.0f;
    CurrentBeat = 0.0f;
    bPlaying = false;
    bPaused = false;
    bFailed = false;
    NextNoteIndex = 0;
    Health = MaxHealth;

    LaneStates.SetNum(LaneCount);
    for (int32 i = 0; i < LaneCount; i++)
    {
        LaneStates[i].Lane = (EForge2RhythmLane)i;
        LaneStates[i].bPressed = false;
    }

    ComboState.Reset();
    Score.Reset();
}

void FForge2RhythmPlayState::Update(float DeltaTime, const FForge2RhythmBeatMap& Map)
{
    if (!bPlaying || bPaused || bFailed) return;

    CurrentTime += DeltaTime;
    CurrentBeat = Map.SecondsToBeats(CurrentTime);
}

void FForge2RhythmPlayState::TakeDamage(float Amount)
{
    Health = FMath::Max(0.0f, Health - Amount);
    if (Health <= 0.0f)
    {
        bFailed = true;
    }
}

void FForge2RhythmPlayState::Heal(float Amount)
{
    Health = FMath::Min(MaxHealth, Health + Amount);
}

// UForge2RhythmManager Implementation
UForge2RhythmManager::UForge2RhythmManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UForge2RhythmManager::Initialize(const FForge2RhythmConfig& InConfig)
{
    Config = InConfig;
    PlayState.Initialize(Config.DefaultLaneCount);
}

void UForge2RhythmManager::Update(float DeltaTime)
{
    if (!PlayState.bPlaying || PlayState.bPaused) return;

    PlayState.Update(DeltaTime, CurrentMap);

    CheckMissedNotes();
    UpdateHoldNotes();

    // Check song completion
    if (PlayState.CurrentTime >= CurrentMap.Duration)
    {
        FinishSong();
    }
}

bool UForge2RhythmManager::LoadBeatMap(const FForge2RhythmBeatMap& Map)
{
    FForge2RhythmBeatMap ProcessedMap = Map;

    // Calculate hit times from beat times
    for (FForge2RhythmNote& Note : ProcessedMap.Notes)
    {
        Note.HitTime = ProcessedMap.BeatsToSeconds(Note.BeatTime);
        if (Note.Duration > 0.0f)
        {
            Note.EndTime = ProcessedMap.BeatsToSeconds(Note.BeatTime + Note.Duration);
        }
    }

    ProcessedMap.SortNotes();
    LoadedMaps.Add(ProcessedMap.MapId, ProcessedMap);

    return true;
}

void UForge2RhythmManager::StartSong(const FString& MapId, const FForge2RhythmModifier& Modifiers)
{
    FForge2RhythmBeatMap* Map = LoadedMaps.Find(MapId);
    if (!Map)
    {
        return;
    }

    CurrentMap = *Map;
    CurrentModifiers = Modifiers;

    // Apply modifiers
    CurrentModifiers.ApplyToNotes(CurrentMap.Notes, CurrentMap.LaneCount);

    // Initialize play state
    PlayState.Initialize(CurrentMap.LaneCount);
    PlayState.ComboState.TotalNotes = CurrentMap.Notes.Num();
    PlayState.bPlaying = true;
}

void UForge2RhythmManager::PauseSong()
{
    if (PlayState.bPlaying)
    {
        PlayState.bPaused = true;
    }
}

void UForge2RhythmManager::ResumeSong()
{
    PlayState.bPaused = false;
}

void UForge2RhythmManager::StopSong()
{
    PlayState.bPlaying = false;
    PlayState.bPaused = false;
}

void UForge2RhythmManager::RestartSong()
{
    FString MapId = CurrentMap.MapId;
    FForge2RhythmModifier Mods = CurrentModifiers;
    StopSong();
    StartSong(MapId, Mods);
}

void UForge2RhythmManager::PressLane(int32 LaneIndex)
{
    ProcessInput(LaneIndex, true);
}

void UForge2RhythmManager::ReleaseLane(int32 LaneIndex)
{
    ProcessInput(LaneIndex, false);
}

void UForge2RhythmManager::ProcessInput(int32 LaneIndex, bool bPressed)
{
    if (!PlayState.bPlaying || PlayState.bPaused) return;
    if (!PlayState.LaneStates.IsValidIndex(LaneIndex)) return;

    float AdjustedTime = PlayState.CurrentTime - Config.InputLatencyOffset;

    FForge2RhythmLaneState& LaneState = PlayState.LaneStates[LaneIndex];

    if (bPressed)
    {
        LaneState.Press(AdjustedTime);

        // Find closest note in this lane
        FForge2RhythmNote* ClosestNote = nullptr;
        float ClosestError = Config.TimingWindows.MissWindow;

        for (int32 i = PlayState.NextNoteIndex; i < CurrentMap.Notes.Num(); i++)
        {
            FForge2RhythmNote& Note = CurrentMap.Notes[i];

            // Skip if wrong lane or already processed
            if ((int32)Note.Lane != LaneIndex) continue;
            if (Note.bHit || Note.bMissed) continue;

            float TimingError = AdjustedTime - Note.HitTime;

            // Too early, check if within window
            if (TimingError < -Config.TimingWindows.MissWindow)
            {
                break;  // Notes are sorted, no point checking further ahead
            }

            // Within window
            if (FMath::Abs(TimingError) < ClosestError)
            {
                ClosestError = FMath::Abs(TimingError);
                ClosestNote = &Note;
            }
        }

        if (ClosestNote)
        {
            ProcessNoteHit(*ClosestNote, AdjustedTime - ClosestNote->HitTime);

            // If hold note, start tracking
            if (ClosestNote->Type == EForge2RhythmNoteType::Hold)
            {
                LaneState.CurrentHoldNote = ClosestNote->NoteId;
            }
        }
    }
    else
    {
        // Release
        if (LaneState.CurrentHoldNote >= 0)
        {
            // Check if hold was completed
            for (FForge2RhythmNote& Note : CurrentMap.Notes)
            {
                if (Note.NoteId == LaneState.CurrentHoldNote)
                {
                    float ReleaseError = FMath::Abs(AdjustedTime - Note.EndTime);
                    if (ReleaseError > Config.TimingWindows.GoodWindow)
                    {
                        // Released too early, counts as miss
                        ProcessNoteMiss(Note);
                    }
                    break;
                }
            }
        }

        LaneState.Release(AdjustedTime);
    }
}

void UForge2RhythmManager::CheckMissedNotes()
{
    float MissThreshold = PlayState.CurrentTime - Config.TimingWindows.MissWindow;

    while (PlayState.NextNoteIndex < CurrentMap.Notes.Num())
    {
        FForge2RhythmNote& Note = CurrentMap.Notes[PlayState.NextNoteIndex];

        if (Note.HitTime > MissThreshold)
        {
            break;  // Haven't reached miss window yet
        }

        if (!Note.bHit && !Note.bMissed)
        {
            ProcessNoteMiss(Note);
        }

        PlayState.NextNoteIndex++;
    }
}

void UForge2RhythmManager::ProcessNoteHit(FForge2RhythmNote& Note, float TimingError)
{
    Note.bHit = true;
    Note.ActualHitTime = PlayState.CurrentTime;
    Note.Judgment = Config.TimingWindows.GetJudgment(TimingError);

    PlayState.ComboState.RegisterHit(Note.Judgment);
    PlayState.Score.AddNoteScore(Note.Judgment, PlayState.ComboState.CurrentCombo);

    // Health recovery
    if (Note.Judgment != EForge2RhythmJudgment::Bad)
    {
        PlayState.Heal(Config.HealthRecoveryRate);
    }

    // Sudden death check
    if (CurrentModifiers.bSuddenDeath &&
        (Note.Judgment == EForge2RhythmJudgment::Bad || Note.Judgment == EForge2RhythmJudgment::Miss))
    {
        PlayState.bFailed = true;
    }
}

void UForge2RhythmManager::ProcessNoteMiss(FForge2RhythmNote& Note)
{
    Note.bMissed = true;
    Note.Judgment = EForge2RhythmJudgment::Miss;

    PlayState.ComboState.RegisterMiss();

    // Health damage
    if (!CurrentModifiers.bNoFail)
    {
        PlayState.TakeDamage(Config.HealthDrainRate);
    }

    // Sudden death
    if (CurrentModifiers.bSuddenDeath)
    {
        PlayState.bFailed = true;
    }
}

void UForge2RhythmManager::UpdateHoldNotes()
{
    float CurrentTime = PlayState.CurrentTime;

    for (FForge2RhythmLaneState& LaneState : PlayState.LaneStates)
    {
        if (LaneState.CurrentHoldNote >= 0)
        {
            for (FForge2RhythmNote& Note : CurrentMap.Notes)
            {
                if (Note.NoteId == LaneState.CurrentHoldNote)
                {
                    float TotalDuration = Note.EndTime - Note.HitTime;
                    float Elapsed = CurrentTime - Note.HitTime;
                    LaneState.HoldProgress = FMath::Clamp(Elapsed / TotalDuration, 0.0f, 1.0f);

                    // Auto-complete when hold ends
                    if (CurrentTime >= Note.EndTime)
                    {
                        LaneState.CurrentHoldNote = -1;
                        LaneState.HoldProgress = 0.0f;
                    }
                    break;
                }
            }
        }
    }
}

void UForge2RhythmManager::FinishSong()
{
    PlayState.bPlaying = false;

    // Calculate final score
    PlayState.Score.Score += PlayState.Score.ComboBonus;
    int32 FinalScore = FMath::RoundToInt(PlayState.Score.Score * CurrentModifiers.ScoreMultiplier);
    PlayState.Score.Score = FinalScore;

    // Save high score
    SaveHighScore(CurrentMap.MapId, FinalScore);
}

float UForge2RhythmManager::GetProgress() const
{
    if (CurrentMap.Duration <= 0.0f) return 0.0f;
    return FMath::Clamp(PlayState.CurrentTime / CurrentMap.Duration, 0.0f, 1.0f);
}

TArray<FForge2RhythmNote> UForge2RhythmManager::GetVisibleNotes(float LookAhead) const
{
    TArray<FForge2RhythmNote> VisibleNotes;

    float StartTime = PlayState.CurrentTime - Config.TimingWindows.MissWindow;
    float EndTime = PlayState.CurrentTime + LookAhead;

    for (const FForge2RhythmNote& Note : CurrentMap.Notes)
    {
        if (Note.HitTime >= StartTime && Note.HitTime <= EndTime)
        {
            if (!Note.bHit && !Note.bMissed)
            {
                VisibleNotes.Add(Note);
            }
        }
        else if (Note.HitTime > EndTime)
        {
            break;
        }
    }

    return VisibleNotes;
}

FForge2RhythmSongResult UForge2RhythmManager::GetCurrentResult() const
{
    FForge2RhythmSongResult Result;
    Result.MapId = CurrentMap.MapId;
    Result.FinalScore = PlayState.Score;
    Result.ComboStats = PlayState.ComboState;
    Result.Grade = PlayState.ComboState.GetGrade();
    Result.bFullCombo = PlayState.ComboState.MissCount == 0 && PlayState.ComboState.BadCount == 0;
    Result.bAllPerfect = PlayState.ComboState.GreatCount == 0 && PlayState.ComboState.GoodCount == 0 &&
                         PlayState.ComboState.BadCount == 0 && PlayState.ComboState.MissCount == 0;
    Result.CompletionTime = PlayState.CurrentTime;

    const int32* PrevScore = HighScores.Find(CurrentMap.MapId);
    Result.PreviousHighScore = PrevScore ? *PrevScore : 0;
    Result.bNewHighScore = PlayState.Score.Score > Result.PreviousHighScore;

    return Result;
}

int32 UForge2RhythmManager::GetHighScore(const FString& MapId) const
{
    const int32* Score = HighScores.Find(MapId);
    return Score ? *Score : 0;
}

void UForge2RhythmManager::SaveHighScore(const FString& MapId, int32 Score)
{
    int32* Existing = HighScores.Find(MapId);
    if (!Existing || Score > *Existing)
    {
        HighScores.Add(MapId, Score);
    }
}

void UForge2RhythmManager::AddNote(FForge2RhythmNote Note)
{
    Note.HitTime = CurrentMap.BeatsToSeconds(Note.BeatTime);
    if (Note.Duration > 0.0f)
    {
        Note.EndTime = CurrentMap.BeatsToSeconds(Note.BeatTime + Note.Duration);
    }
    CurrentMap.Notes.Add(Note);
    CurrentMap.SortNotes();
}

void UForge2RhythmManager::RemoveNote(int32 NoteId)
{
    CurrentMap.Notes.RemoveAll([NoteId](const FForge2RhythmNote& Note)
    {
        return Note.NoteId == NoteId;
    });
    CurrentMap.SortNotes();
}

void UForge2RhythmManager::ClearNotes()
{
    CurrentMap.Notes.Empty();
}

FForge2RhythmBeatMap UForge2RhythmManager::CreateEmptyBeatMap(const FString& SongName, float BPM)
{
    FForge2RhythmBeatMap Map;
    Map.MapId = FString::Printf(TEXT("%s_%d"), *SongName, FMath::RandRange(1000, 9999));
    Map.SongName = SongName;
    Map.BPM = BPM;
    Map.LaneCount = Config.DefaultLaneCount;
    Map.Difficulty = EForge2RhythmDifficulty::Normal;
    return Map;
}

// UForge2RhythmUtility Implementation
FForge2RhythmNote UForge2RhythmUtility::CreateTapNote(int32 Lane, float BeatTime)
{
    FForge2RhythmNote Note;
    Note.Type = EForge2RhythmNoteType::Tap;
    Note.Lane = (EForge2RhythmLane)FMath::Clamp(Lane, 0, 7);
    Note.BeatTime = BeatTime;
    return Note;
}

FForge2RhythmNote UForge2RhythmUtility::CreateHoldNote(int32 Lane, float StartBeat, float EndBeat)
{
    FForge2RhythmNote Note;
    Note.Type = EForge2RhythmNoteType::Hold;
    Note.Lane = (EForge2RhythmLane)FMath::Clamp(Lane, 0, 7);
    Note.BeatTime = StartBeat;
    Note.Duration = EndBeat - StartBeat;
    return Note;
}

FForge2RhythmNote UForge2RhythmUtility::CreateSlideNote(int32 StartLane, int32 EndLane,
                                                         float StartBeat, float EndBeat)
{
    FForge2RhythmNote Note;
    Note.Type = EForge2RhythmNoteType::Slide;
    Note.Lane = (EForge2RhythmLane)FMath::Clamp(StartLane, 0, 7);
    Note.EndLane = (EForge2RhythmLane)FMath::Clamp(EndLane, 0, 7);
    Note.BeatTime = StartBeat;
    Note.Duration = EndBeat - StartBeat;
    return Note;
}

FForge2RhythmBeatMap UForge2RhythmUtility::CreateSimpleBeatMap(const FString& SongName,
                                                                float BPM, float Duration)
{
    FForge2RhythmBeatMap Map;
    Map.MapId = FString::Printf(TEXT("%s_%d"), *SongName, FMath::RandRange(1000, 9999));
    Map.SongName = SongName;
    Map.BPM = BPM;
    Map.Duration = Duration;
    Map.LaneCount = 4;
    Map.Difficulty = EForge2RhythmDifficulty::Normal;
    Map.DifficultyRating = 5;
    return Map;
}

void UForge2RhythmUtility::GenerateBasicPattern(FForge2RhythmBeatMap& Map,
                                                 float StartBeat, float EndBeat, int32 PatternType)
{
    float CurrentBeat = StartBeat;
    float Interval = 1.0f;  // One note per beat by default

    switch (PatternType)
    {
        case 0:  // Quarter notes, alternating lanes
            {
                int32 Lane = 0;
                while (CurrentBeat < EndBeat)
                {
                    Map.Notes.Add(CreateTapNote(Lane % Map.LaneCount, CurrentBeat));
                    Lane++;
                    CurrentBeat += 1.0f;
                }
            }
            break;

        case 1:  // Eighth notes, stairs pattern
            {
                int32 Lane = 0;
                int32 Direction = 1;
                while (CurrentBeat < EndBeat)
                {
                    Map.Notes.Add(CreateTapNote(Lane, CurrentBeat));
                    Lane += Direction;
                    if (Lane >= Map.LaneCount - 1) Direction = -1;
                    if (Lane <= 0) Direction = 1;
                    CurrentBeat += 0.5f;
                }
            }
            break;

        case 2:  // Hold notes
            {
                int32 Lane = 0;
                while (CurrentBeat < EndBeat)
                {
                    Map.Notes.Add(CreateHoldNote(Lane, CurrentBeat, CurrentBeat + 2.0f));
                    Lane = (Lane + 1) % Map.LaneCount;
                    CurrentBeat += 4.0f;
                }
            }
            break;

        case 3:  // Random pattern
            {
                while (CurrentBeat < EndBeat)
                {
                    int32 Lane = FMath::RandRange(0, Map.LaneCount - 1);
                    Map.Notes.Add(CreateTapNote(Lane, CurrentBeat));
                    CurrentBeat += FMath::FRandRange(0.25f, 1.0f);
                }
            }
            break;

        default:
            break;
    }

    Map.SortNotes();
}

FForge2RhythmTimingWindow UForge2RhythmUtility::CreateStrictTiming()
{
    FForge2RhythmTimingWindow Timing;
    Timing.PerfectWindow = 0.020f;  // 20ms
    Timing.GreatWindow = 0.045f;    // 45ms
    Timing.GoodWindow = 0.070f;     // 70ms
    Timing.BadWindow = 0.100f;      // 100ms
    Timing.MissWindow = 0.150f;     // 150ms
    return Timing;
}

FForge2RhythmTimingWindow UForge2RhythmUtility::CreateRelaxedTiming()
{
    FForge2RhythmTimingWindow Timing;
    Timing.PerfectWindow = 0.050f;  // 50ms
    Timing.GreatWindow = 0.100f;    // 100ms
    Timing.GoodWindow = 0.150f;     // 150ms
    Timing.BadWindow = 0.200f;      // 200ms
    Timing.MissWindow = 0.300f;     // 300ms
    return Timing;
}

float UForge2RhythmUtility::CalculateDifficulty(const FForge2RhythmBeatMap& Map)
{
    if (Map.Notes.Num() == 0 || Map.Duration <= 0.0f)
    {
        return 1.0f;
    }

    // Notes per second
    float NPS = Map.Notes.Num() / Map.Duration;

    // Count complex note types
    int32 HoldCount = 0;
    int32 SlideCount = 0;
    int32 SimultaneousCount = 0;

    for (const FForge2RhythmNote& Note : Map.Notes)
    {
        if (Note.Type == EForge2RhythmNoteType::Hold) HoldCount++;
        if (Note.Type == EForge2RhythmNoteType::Slide) SlideCount++;
        if (Note.SimultaneousGroup >= 0) SimultaneousCount++;
    }

    float ComplexityBonus = (HoldCount * 0.5f + SlideCount * 0.75f + SimultaneousCount * 0.25f) /
                            Map.Notes.Num();

    // Base difficulty from NPS (2 NPS = 1, 10 NPS = 15)
    float Difficulty = (NPS - 2.0f) * 1.75f + 1.0f;
    Difficulty += ComplexityBonus * 3.0f;

    // Add BPM factor
    Difficulty += (Map.BPM - 100.0f) / 50.0f;

    return FMath::Clamp(Difficulty, 1.0f, 15.0f);
}

float UForge2RhythmUtility::BPMToBeatsPerSecond(float BPM)
{
    return BPM / 60.0f;
}

float UForge2RhythmUtility::BeatsToMilliseconds(float Beats, float BPM)
{
    return (Beats / BPM) * 60000.0f;
}
