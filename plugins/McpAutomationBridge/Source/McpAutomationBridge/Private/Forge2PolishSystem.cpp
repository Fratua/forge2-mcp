// Copyright 2025 FORGE2 Project. All Rights Reserved.

#include "Forge2PolishSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Engine/Engine.h"
#include "ImageUtils.h"
#include "HighResScreenshot.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

// ============================================================================
// UForge2SaveManager Implementation
// ============================================================================

UForge2SaveManager::UForge2SaveManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UForge2SaveManager::BeginPlay()
{
	Super::BeginPlay();

	if (AutosaveSettings.bEnabled)
	{
		PrimaryComponentTick.bStartWithTickEnabled = true;
		SetComponentTickEnabled(true);
	}
}

void UForge2SaveManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AutosaveSettings.bEnabled)
	{
		TimeSinceLastAutosave += DeltaTime;
		if (TimeSinceLastAutosave >= AutosaveSettings.IntervalSeconds)
		{
			TriggerAutosave();
			TimeSinceLastAutosave = 0.0f;
		}
	}
}

bool UForge2SaveManager::SaveGame(int32 SlotIndex, const FString& SlotName)
{
	FString SaveSlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

	// In production, would use custom USaveGame class
	USaveGame* SaveGameInstance = UGameplayStatics::CreateSaveGameObject(USaveGame::StaticClass());
	if (SaveGameInstance)
	{
		bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0);
		OnGameSaved.Broadcast(SlotIndex, bSuccess);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("FORGE2 Save: Game saved to slot %d (%s)"), SlotIndex, *SlotName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FORGE2 Save: Failed to save game to slot %d"), SlotIndex);
		}

		return bSuccess;
	}

	return false;
}

bool UForge2SaveManager::LoadGame(int32 SlotIndex)
{
	FString SaveSlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("FORGE2 Save: No save found in slot %d"), SlotIndex);
		OnGameLoaded.Broadcast(SlotIndex, false);
		return false;
	}

	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
	if (LoadedGame)
	{
		// Apply loaded data
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Save: Game loaded from slot %d"), SlotIndex);
		OnGameLoaded.Broadcast(SlotIndex, true);
		return true;
	}

	OnGameLoaded.Broadcast(SlotIndex, false);
	return false;
}

bool UForge2SaveManager::DeleteSave(int32 SlotIndex)
{
	FString SaveSlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

	if (UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Save: Deleted save slot %d"), SlotIndex);
		return true;
	}

	return false;
}

TArray<FForge2SaveSlotInfo> UForge2SaveManager::GetAllSaveSlots() const
{
	TArray<FForge2SaveSlotInfo> Slots;

	// Check slots 0-9
	for (int32 i = 0; i < 10; ++i)
	{
		FString SaveSlotName = FString::Printf(TEXT("SaveSlot_%d"), i);
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
		{
			FForge2SaveSlotInfo SlotInfo;
			SlotInfo.SlotIndex = i;
			SlotInfo.SlotName = SaveSlotName;
			// Would populate from actual save data
			Slots.Add(SlotInfo);
		}
	}

	return Slots;
}

bool UForge2SaveManager::DoesSaveExist(int32 SlotIndex) const
{
	FString SaveSlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);
}

bool UForge2SaveManager::QuickSave()
{
	return SaveGame(QuickSaveSlot, TEXT("QuickSave"));
}

bool UForge2SaveManager::QuickLoad()
{
	return LoadGame(QuickSaveSlot);
}

void UForge2SaveManager::ConfigureAutosave(const FForge2AutosaveSettings& Settings)
{
	AutosaveSettings = Settings;
	SetComponentTickEnabled(Settings.bEnabled);
	TimeSinceLastAutosave = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Save: Autosave configured - Enabled: %d, Interval: %.0fs"),
		Settings.bEnabled, Settings.IntervalSeconds);
}

void UForge2SaveManager::TriggerAutosave()
{
	if (!AutosaveSettings.bEnabled)
	{
		return;
	}

	// Rotate autosave slots
	CurrentAutosaveSlot = (CurrentAutosaveSlot + 1) % AutosaveSettings.MaxAutosaveSlots;
	int32 AutosaveSlotIndex = 100 + CurrentAutosaveSlot; // Use slots 100+ for autosaves

	FString SlotName = FString::Printf(TEXT("Autosave_%d"), CurrentAutosaveSlot + 1);
	SaveGame(AutosaveSlotIndex, SlotName);

	OnAutosaveTriggered.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Save: Autosave triggered (slot %d)"), CurrentAutosaveSlot);
}

void UForge2SaveManager::SetAutosaveEnabled(bool bEnabled)
{
	AutosaveSettings.bEnabled = bEnabled;
	SetComponentTickEnabled(bEnabled);
}

bool UForge2SaveManager::UploadToCloud(int32 SlotIndex)
{
	// Cloud save integration placeholder
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Save: Uploading slot %d to cloud"), SlotIndex);
	return true;
}

bool UForge2SaveManager::DownloadFromCloud(int32 SlotIndex)
{
	// Cloud save integration placeholder
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Save: Downloading slot %d from cloud"), SlotIndex);
	return true;
}

bool UForge2SaveManager::IsCloudSaveAvailable() const
{
	// Would check platform cloud service availability
	return false;
}

bool UForge2SaveManager::MigrateSave(int32 SlotIndex, int32 FromVersion, int32 ToVersion)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Save: Migrating slot %d from version %d to %d"),
		SlotIndex, FromVersion, ToVersion);
	// Would implement version-specific migration logic
	return true;
}

// ============================================================================
// UForge2AccessibilityManager Implementation
// ============================================================================

UForge2AccessibilityManager::UForge2AccessibilityManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2AccessibilityManager::ApplySettings(const FForge2AccessibilitySettings& Settings)
{
	CurrentSettings = Settings;

	ApplyColorblindFilter();
	UpdateSubtitleSettings();
	UpdateMotorSettings();

	UE_LOG(LogTemp, Log, TEXT("FORGE2 Accessibility: Settings applied"));
}

FForge2AccessibilitySettings UForge2AccessibilityManager::GetSettings() const
{
	return CurrentSettings;
}

void UForge2AccessibilityManager::ResetToDefaults()
{
	CurrentSettings = FForge2AccessibilitySettings();
	ApplySettings(CurrentSettings);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Accessibility: Reset to defaults"));
}

void UForge2AccessibilityManager::SetColorblindMode(EForge2ColorblindMode Mode)
{
	CurrentSettings.ColorblindMode = Mode;
	ApplyColorblindFilter();
}

void UForge2AccessibilityManager::SetColorblindIntensity(float Intensity)
{
	CurrentSettings.ColorblindIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	ApplyColorblindFilter();
}

FLinearColor UForge2AccessibilityManager::AdjustColorForColorblind(FLinearColor OriginalColor) const
{
	return UForge2PolishUtilities::SimulateColorblindness(OriginalColor, CurrentSettings.ColorblindMode);
}

void UForge2AccessibilityManager::SetSubtitlesEnabled(bool bEnabled)
{
	CurrentSettings.bSubtitlesEnabled = bEnabled;
	UpdateSubtitleSettings();
}

void UForge2AccessibilityManager::SetSubtitleSize(float Size)
{
	CurrentSettings.SubtitleSize = FMath::Clamp(Size, 0.5f, 3.0f);
	UpdateSubtitleSettings();
}

void UForge2AccessibilityManager::SetSubtitleBackground(EForge2SubtitleBackground Background)
{
	CurrentSettings.SubtitleBackground = Background;
	UpdateSubtitleSettings();
}

void UForge2AccessibilityManager::SetAutoAim(bool bEnabled, float Strength)
{
	CurrentSettings.bAutoAim = bEnabled;
	CurrentSettings.AutoAimStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
	UpdateMotorSettings();
}

void UForge2AccessibilityManager::SetHoldToToggle(bool bEnabled)
{
	CurrentSettings.bHoldToToggle = bEnabled;
	UpdateMotorSettings();
}

void UForge2AccessibilityManager::SetOneHandedMode(bool bEnabled)
{
	CurrentSettings.bOneHandedMode = bEnabled;
	UpdateMotorSettings();
}

void UForge2AccessibilityManager::SetUIScale(float Scale)
{
	CurrentSettings.UIScale = FMath::Clamp(Scale, 0.5f, 2.0f);
	// Would apply to UI system
}

void UForge2AccessibilityManager::SetHighContrast(bool bEnabled)
{
	CurrentSettings.bHighContrastUI = bEnabled;
	// Would apply high contrast theme
}

void UForge2AccessibilityManager::SetReducedMotion(bool bEnabled)
{
	CurrentSettings.bReducedMotion = bEnabled;
	// Would disable animations and camera shake
}

void UForge2AccessibilityManager::SetGodMode(bool bEnabled)
{
	CurrentSettings.bGodMode = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Accessibility: God mode %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UForge2AccessibilityManager::SetDamageMultiplier(float Multiplier)
{
	CurrentSettings.DamageMultiplier = FMath::Clamp(Multiplier, 0.0f, 2.0f);
}

void UForge2AccessibilityManager::ApplyColorblindFilter()
{
	// Would apply post-process material for colorblind simulation
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Accessibility: Colorblind mode set to %d with intensity %.2f"),
		(int32)CurrentSettings.ColorblindMode, CurrentSettings.ColorblindIntensity);
}

void UForge2AccessibilityManager::UpdateSubtitleSettings()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Accessibility: Subtitles %s, size %.1fx"),
		CurrentSettings.bSubtitlesEnabled ? TEXT("enabled") : TEXT("disabled"), CurrentSettings.SubtitleSize);
}

void UForge2AccessibilityManager::UpdateMotorSettings()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Accessibility: Auto-aim %s (%.0f%%), Hold-to-toggle %s"),
		CurrentSettings.bAutoAim ? TEXT("on") : TEXT("off"),
		CurrentSettings.AutoAimStrength * 100.0f,
		CurrentSettings.bHoldToToggle ? TEXT("on") : TEXT("off"));
}

// ============================================================================
// UForge2PhotoModeManager Implementation
// ============================================================================

UForge2PhotoModeManager::UForge2PhotoModeManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2PhotoModeManager::EnterPhotoMode()
{
	if (bPhotoModeActive)
	{
		return;
	}

	bPhotoModeActive = true;

	// Freeze the game
	if (CurrentSettings.bFreezeScene)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}

	// Store camera position
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerCameraManager)
	{
		StoredCameraLocation = PC->PlayerCameraManager->GetCameraLocation();
		StoredCameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	}

	UE_LOG(LogTemp, Log, TEXT("FORGE2 PhotoMode: Entered photo mode"));
}

void UForge2PhotoModeManager::ExitPhotoMode()
{
	if (!bPhotoModeActive)
	{
		return;
	}

	bPhotoModeActive = false;

	// Unfreeze the game
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// Restore camera
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerCameraManager)
	{
		// Would restore view
	}

	UE_LOG(LogTemp, Log, TEXT("FORGE2 PhotoMode: Exited photo mode"));
}

bool UForge2PhotoModeManager::IsInPhotoMode() const
{
	return bPhotoModeActive;
}

void UForge2PhotoModeManager::SetFOV(float FOV)
{
	CurrentSettings.FOV = FMath::Clamp(FOV, 30.0f, 120.0f);
	// Would update camera
}

void UForge2PhotoModeManager::SetRoll(float Roll)
{
	CurrentSettings.Roll = FMath::Clamp(Roll, -180.0f, 180.0f);
	// Would update camera
}

void UForge2PhotoModeManager::SetCameraOffset(FVector Offset)
{
	CurrentSettings.CameraOffset = Offset;
	// Would update camera
}

void UForge2PhotoModeManager::SetDOFEnabled(bool bEnabled)
{
	CurrentSettings.bDOFEnabled = bEnabled;
	// Would update post-process
}

void UForge2PhotoModeManager::SetFocalDistance(float Distance)
{
	CurrentSettings.FocalDistance = FMath::Max(Distance, 0.0f);
	// Would update post-process
}

void UForge2PhotoModeManager::SetBokehStrength(float Strength)
{
	CurrentSettings.BokehStrength = FMath::Clamp(Strength, 0.0f, 2.0f);
	// Would update post-process
}

void UForge2PhotoModeManager::ApplyFilter(EForge2PhotoFilter Filter, float Intensity)
{
	CurrentSettings.Filter = Filter;
	CurrentSettings.FilterIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	FForge2PhotoModeSettings FilterSettings = UForge2PolishUtilities::GetPhotoModePreset(Filter);
	if (Filter != EForge2PhotoFilter::None)
	{
		CurrentSettings.Brightness = FMath::Lerp(1.0f, FilterSettings.Brightness, Intensity);
		CurrentSettings.Contrast = FMath::Lerp(1.0f, FilterSettings.Contrast, Intensity);
		CurrentSettings.Saturation = FMath::Lerp(1.0f, FilterSettings.Saturation, Intensity);
	}
}

void UForge2PhotoModeManager::SetBrightness(float Brightness)
{
	CurrentSettings.Brightness = FMath::Clamp(Brightness, 0.0f, 2.0f);
}

void UForge2PhotoModeManager::SetContrast(float Contrast)
{
	CurrentSettings.Contrast = FMath::Clamp(Contrast, 0.0f, 2.0f);
}

void UForge2PhotoModeManager::SetSaturation(float Saturation)
{
	CurrentSettings.Saturation = FMath::Clamp(Saturation, 0.0f, 2.0f);
}

void UForge2PhotoModeManager::SetVignette(float Vignette)
{
	CurrentSettings.Vignette = FMath::Clamp(Vignette, 0.0f, 1.0f);
}

void UForge2PhotoModeManager::RegisterPose(const FForge2CharacterPose& Pose)
{
	RegisteredPoses.Add(Pose);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 PhotoMode: Registered pose %s"), *Pose.PoseId.ToString());
}

void UForge2PhotoModeManager::ApplyPose(FName PoseId)
{
	for (const FForge2CharacterPose& Pose : RegisteredPoses)
	{
		if (Pose.PoseId == PoseId && Pose.bIsUnlocked)
		{
			// Would apply animation to character
			UE_LOG(LogTemp, Log, TEXT("FORGE2 PhotoMode: Applying pose %s"), *PoseId.ToString());
			return;
		}
	}
}

TArray<FForge2CharacterPose> UForge2PhotoModeManager::GetAvailablePoses() const
{
	TArray<FForge2CharacterPose> AvailablePoses;
	for (const FForge2CharacterPose& Pose : RegisteredPoses)
	{
		if (Pose.bIsUnlocked)
		{
			AvailablePoses.Add(Pose);
		}
	}
	return AvailablePoses;
}

void UForge2PhotoModeManager::SetHidePlayer(bool bHide)
{
	CurrentSettings.bHidePlayer = bHide;
	// Would toggle player visibility
}

void UForge2PhotoModeManager::SetHideNPCs(bool bHide)
{
	CurrentSettings.bHideNPCs = bHide;
	// Would toggle NPC visibility
}

void UForge2PhotoModeManager::SetTimeOfDay(float Hour)
{
	CurrentSettings.TimeOfDay = FMath::Clamp(Hour, 0.0f, 24.0f);
	// Would update sky/lighting system
}

bool UForge2PhotoModeManager::TakePhoto(const FString& Filename)
{
	FString FullPath = FPaths::Combine(GetPhotosDirectory(), Filename + TEXT(".png"));

	// Request high-res screenshot
	FHighResScreenshotConfig& Config = GetHighResScreenshotConfig();
	Config.SetFilename(FullPath);

	UE_LOG(LogTemp, Log, TEXT("FORGE2 PhotoMode: Photo saved to %s"), *FullPath);
	return true;
}

FString UForge2PhotoModeManager::GetPhotosDirectory() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Screenshots"));
}

void UForge2PhotoModeManager::ApplySettings(const FForge2PhotoModeSettings& Settings)
{
	CurrentSettings = Settings;
}

FForge2PhotoModeSettings UForge2PhotoModeManager::GetSettings() const
{
	return CurrentSettings;
}

// ============================================================================
// UForge2AchievementManager Implementation
// ============================================================================

UForge2AchievementManager::UForge2AchievementManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2AchievementManager::RegisterAchievement(const FForge2AchievementDefinition& Achievement)
{
	Achievements.Add(Achievement.AchievementId, Achievement);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Achievements: Registered achievement %s (%d points)"),
		*Achievement.AchievementId.ToString(), Achievement.Points);
}

TArray<FForge2AchievementDefinition> UForge2AchievementManager::GetAllAchievements() const
{
	TArray<FForge2AchievementDefinition> AllAchievements;
	for (const auto& Pair : Achievements)
	{
		AllAchievements.Add(Pair.Value);
	}
	return AllAchievements;
}

void UForge2AchievementManager::SetProgress(FName AchievementId, int32 NewProgress)
{
	if (!Achievements.Contains(AchievementId))
	{
		return;
	}

	FForge2AchievementProgress& AchProgress = Progress.FindOrAdd(AchievementId);
	AchProgress.AchievementId = AchievementId;
	AchProgress.CurrentProgress = NewProgress;

	const FForge2AchievementDefinition& Definition = Achievements[AchievementId];
	float ProgressPercent = (float)NewProgress / (float)Definition.TargetProgress;
	OnAchievementProgress.Broadcast(AchievementId, ProgressPercent);

	if (NewProgress >= Definition.TargetProgress && !AchProgress.bIsUnlocked)
	{
		UnlockAchievement(AchievementId);
	}
}

void UForge2AchievementManager::AddProgress(FName AchievementId, int32 Amount)
{
	FForge2AchievementProgress CurrentProgress = GetProgress(AchievementId);
	SetProgress(AchievementId, CurrentProgress.CurrentProgress + Amount);
}

FForge2AchievementProgress UForge2AchievementManager::GetProgress(FName AchievementId) const
{
	if (const FForge2AchievementProgress* ProgressPtr = Progress.Find(AchievementId))
	{
		return *ProgressPtr;
	}
	return FForge2AchievementProgress();
}

void UForge2AchievementManager::UnlockAchievement(FName AchievementId)
{
	if (!Achievements.Contains(AchievementId))
	{
		return;
	}

	FForge2AchievementProgress& AchProgress = Progress.FindOrAdd(AchievementId);
	if (AchProgress.bIsUnlocked)
	{
		return;
	}

	AchProgress.bIsUnlocked = true;
	AchProgress.UnlockDate = FDateTime::Now();

	const FForge2AchievementDefinition& Definition = Achievements[AchievementId];
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Achievements: Unlocked %s - %s"),
		*AchievementId.ToString(), *Definition.Title.ToString());

	OnAchievementUnlocked.Broadcast(AchievementId);
}

bool UForge2AchievementManager::IsAchievementUnlocked(FName AchievementId) const
{
	if (const FForge2AchievementProgress* ProgressPtr = Progress.Find(AchievementId))
	{
		return ProgressPtr->bIsUnlocked;
	}
	return false;
}

TArray<FName> UForge2AchievementManager::GetUnlockedAchievements() const
{
	TArray<FName> Unlocked;
	for (const auto& Pair : Progress)
	{
		if (Pair.Value.bIsUnlocked)
		{
			Unlocked.Add(Pair.Key);
		}
	}
	return Unlocked;
}

void UForge2AchievementManager::SyncWithPlatform(EForge2AchievementPlatform Platform)
{
	// Would sync with platform-specific achievement system
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Achievements: Syncing with platform %d"), (int32)Platform);
}

int32 UForge2AchievementManager::GetTotalPoints() const
{
	int32 TotalPoints = 0;
	for (const FName& AchievementId : GetUnlockedAchievements())
	{
		if (const FForge2AchievementDefinition* Definition = Achievements.Find(AchievementId))
		{
			TotalPoints += Definition->Points;
		}
	}
	return TotalPoints;
}

float UForge2AchievementManager::GetCompletionPercentage() const
{
	if (Achievements.Num() == 0)
	{
		return 0.0f;
	}
	return (float)GetUnlockedAchievements().Num() / (float)Achievements.Num() * 100.0f;
}

// ============================================================================
// UForge2AnalyticsManager Implementation
// ============================================================================

UForge2AnalyticsManager::UForge2AnalyticsManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;
}

void UForge2AnalyticsManager::BeginPlay()
{
	Super::BeginPlay();
	StartSession();
}

void UForge2AnalyticsManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndSession();
	Super::EndPlay(EndPlayReason);
}

void UForge2AnalyticsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentSession.PlayTimeSeconds += DeltaTime;
}

void UForge2AnalyticsManager::TrackEvent(const FForge2AnalyticsEvent& Event)
{
	if (!bAnalyticsEnabled)
	{
		return;
	}

	FForge2AnalyticsEvent EventCopy = Event;
	EventCopy.Timestamp = FDateTime::Now();
	EventCopy.SessionId = CurrentSession.SessionId;

	EventQueue.Add(EventCopy);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Analytics: Tracked event %s"), *Event.EventName);
}

void UForge2AnalyticsManager::TrackSimpleEvent(const FString& EventName, EForge2AnalyticsCategory Category)
{
	FForge2AnalyticsEvent Event;
	Event.EventName = EventName;
	Event.Category = Category;
	TrackEvent(Event);
}

void UForge2AnalyticsManager::TrackEventWithParams(const FString& EventName, const TMap<FString, FString>& Params)
{
	FForge2AnalyticsEvent Event;
	Event.EventName = EventName;
	Event.StringParameters = Params;
	TrackEvent(Event);
}

void UForge2AnalyticsManager::StartSession()
{
	CurrentSession = FForge2SessionStats();
	CurrentSession.SessionId = FGuid::NewGuid().ToString();
	CurrentSession.SessionStart = FDateTime::Now();
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Analytics: Session started - %s"), *CurrentSession.SessionId);
}

void UForge2AnalyticsManager::EndSession()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Analytics: Session ended - Duration: %.0f seconds"), CurrentSession.PlayTimeSeconds);
	// Would send session summary to analytics backend
}

FForge2SessionStats UForge2AnalyticsManager::GetSessionStats() const
{
	return CurrentSession;
}

void UForge2AnalyticsManager::RecordHeatmapPoint(FVector Location, const FString& EventType)
{
	FForge2HeatmapPoint Point;
	Point.Location = Location;
	Point.EventType = EventType;
	Point.Timestamp = FDateTime::Now();
	HeatmapData.Add(Point);
}

TArray<FForge2HeatmapPoint> UForge2AnalyticsManager::GetHeatmapData(const FString& EventType) const
{
	TArray<FForge2HeatmapPoint> FilteredData;
	for (const FForge2HeatmapPoint& Point : HeatmapData)
	{
		if (Point.EventType == EventType || EventType.IsEmpty())
		{
			FilteredData.Add(Point);
		}
	}
	return FilteredData;
}

void UForge2AnalyticsManager::ClearHeatmapData()
{
	HeatmapData.Empty();
}

void UForge2AnalyticsManager::TrackFunnelStep(const FString& FunnelName, int32 StepIndex)
{
	TMap<int32, int32>& Funnel = FunnelData.FindOrAdd(FunnelName);
	int32& Count = Funnel.FindOrAdd(StepIndex);
	Count++;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Analytics: Funnel %s step %d (count: %d)"), *FunnelName, StepIndex, Count);
}

TMap<int32, int32> UForge2AnalyticsManager::GetFunnelData(const FString& FunnelName) const
{
	if (const TMap<int32, int32>* Funnel = FunnelData.Find(FunnelName))
	{
		return *Funnel;
	}
	return TMap<int32, int32>();
}

void UForge2AnalyticsManager::RegisterExperiment(const FString& ExperimentName, const TArray<FString>& Variants)
{
	Experiments.Add(ExperimentName, Variants);

	// Randomly assign variant
	if (Variants.Num() > 0)
	{
		int32 VariantIndex = FMath::RandRange(0, Variants.Num() - 1);
		ExperimentAssignments.Add(ExperimentName, Variants[VariantIndex]);
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Analytics: Assigned variant %s to experiment %s"),
			*Variants[VariantIndex], *ExperimentName);
	}
}

FString UForge2AnalyticsManager::GetExperimentVariant(const FString& ExperimentName) const
{
	if (const FString* Variant = ExperimentAssignments.Find(ExperimentName))
	{
		return *Variant;
	}
	return TEXT("");
}

void UForge2AnalyticsManager::SetAnalyticsEnabled(bool bEnabled)
{
	bAnalyticsEnabled = bEnabled;
}

bool UForge2AnalyticsManager::AreAnalyticsEnabled() const
{
	return bAnalyticsEnabled;
}

// ============================================================================
// UForge2LocalizationManager Implementation
// ============================================================================

UForge2LocalizationManager::UForge2LocalizationManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2LocalizationManager::SetLanguage(const FString& LanguageCode)
{
	FInternationalization::Get().SetCurrentCulture(LanguageCode);
	CurrentLanguageCode = LanguageCode;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Localization: Set language to %s"), *LanguageCode);
}

FString UForge2LocalizationManager::GetCurrentLanguage() const
{
	return CurrentLanguageCode.IsEmpty() ? FInternationalization::Get().GetCurrentCulture()->GetName() : CurrentLanguageCode;
}

TArray<FForge2LanguageInfo> UForge2LocalizationManager::GetAvailableLanguages() const
{
	return RegisteredLanguages;
}

void UForge2LocalizationManager::RegisterLanguage(const FForge2LanguageInfo& LanguageInfo)
{
	RegisteredLanguages.Add(LanguageInfo);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Localization: Registered language %s"), *LanguageInfo.LanguageCode);
}

FText UForge2LocalizationManager::GetLocalizedText(const FString& Key) const
{
	// Would look up from string tables
	return FText::FromString(Key);
}

FText UForge2LocalizationManager::FormatLocalizedText(const FString& Key, const TMap<FString, FString>& Arguments) const
{
	FText BaseText = GetLocalizedText(Key);
	// Would apply argument substitution
	return BaseText;
}

void UForge2LocalizationManager::SetAudioLanguage(const FString& LanguageCode)
{
	CurrentAudioLanguage = LanguageCode;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Localization: Set audio language to %s"), *LanguageCode);
}

FString UForge2LocalizationManager::GetAudioLanguage() const
{
	return CurrentAudioLanguage.IsEmpty() ? GetCurrentLanguage() : CurrentAudioLanguage;
}

bool UForge2LocalizationManager::IsAudioLanguageAvailable(const FString& LanguageCode) const
{
	for (const FForge2LanguageInfo& Lang : RegisteredLanguages)
	{
		if (Lang.LanguageCode == LanguageCode)
		{
			return Lang.bHasVoiceOver;
		}
	}
	return false;
}

UFont* UForge2LocalizationManager::GetFontForLanguage(const FString& LanguageCode) const
{
	// Would return appropriate font for language (CJK, Arabic, etc.)
	return nullptr;
}

bool UForge2LocalizationManager::IsCurrentLanguageRTL() const
{
	for (const FForge2LanguageInfo& Lang : RegisteredLanguages)
	{
		if (Lang.LanguageCode == CurrentLanguageCode)
		{
			return Lang.bIsRTL;
		}
	}
	return false;
}

// ============================================================================
// UForge2PolishUtilities Implementation
// ============================================================================

FLinearColor UForge2PolishUtilities::SimulateColorblindness(FLinearColor Color, EForge2ColorblindMode Mode)
{
	// Color blindness simulation matrices
	switch (Mode)
	{
	case EForge2ColorblindMode::Protanopia:
		// Red-blind simulation
		return FLinearColor(
			Color.R * 0.567f + Color.G * 0.433f,
			Color.R * 0.558f + Color.G * 0.442f,
			Color.B * 0.242f + Color.G * 0.758f,
			Color.A);

	case EForge2ColorblindMode::Deuteranopia:
		// Green-blind simulation
		return FLinearColor(
			Color.R * 0.625f + Color.G * 0.375f,
			Color.R * 0.7f + Color.G * 0.3f,
			Color.B * 0.3f + Color.G * 0.7f,
			Color.A);

	case EForge2ColorblindMode::Tritanopia:
		// Blue-blind simulation
		return FLinearColor(
			Color.R * 0.95f + Color.G * 0.05f,
			Color.G * 0.433f + Color.B * 0.567f,
			Color.G * 0.475f + Color.B * 0.525f,
			Color.A);

	case EForge2ColorblindMode::Achromatopsia:
		// Monochromacy
		{
			float Luminance = Color.R * 0.299f + Color.G * 0.587f + Color.B * 0.114f;
			return FLinearColor(Luminance, Luminance, Luminance, Color.A);
		}

	default:
		return Color;
	}
}

FForge2PhotoModeSettings UForge2PolishUtilities::GetPhotoModePreset(EForge2PhotoFilter Filter)
{
	FForge2PhotoModeSettings Settings;

	switch (Filter)
	{
	case EForge2PhotoFilter::Sepia:
		Settings.Saturation = 0.2f;
		Settings.ColorTint = FLinearColor(1.1f, 0.9f, 0.6f, 1.0f);
		Settings.Contrast = 1.1f;
		break;

	case EForge2PhotoFilter::BlackAndWhite:
		Settings.Saturation = 0.0f;
		Settings.Contrast = 1.2f;
		break;

	case EForge2PhotoFilter::Vintage:
		Settings.Saturation = 0.7f;
		Settings.Contrast = 0.9f;
		Settings.Vignette = 0.4f;
		Settings.Grain = 0.2f;
		Settings.ColorTint = FLinearColor(1.05f, 1.0f, 0.85f, 1.0f);
		break;

	case EForge2PhotoFilter::Vivid:
		Settings.Saturation = 1.5f;
		Settings.Contrast = 1.1f;
		break;

	case EForge2PhotoFilter::Cold:
		Settings.ColorTint = FLinearColor(0.9f, 0.95f, 1.1f, 1.0f);
		break;

	case EForge2PhotoFilter::Warm:
		Settings.ColorTint = FLinearColor(1.1f, 1.0f, 0.85f, 1.0f);
		break;

	case EForge2PhotoFilter::Noir:
		Settings.Saturation = 0.0f;
		Settings.Contrast = 1.4f;
		Settings.Vignette = 0.6f;
		break;

	default:
		break;
	}

	return Settings;
}

FForge2AccessibilitySettings UForge2PolishUtilities::GetVisuallyImpairedPreset()
{
	FForge2AccessibilitySettings Settings;
	Settings.UIScale = 1.5f;
	Settings.bHighContrastUI = true;
	Settings.SubtitleSize = 1.5f;
	Settings.bShowSpeakerNames = true;
	Settings.bShowSoundCaptions = true;
	Settings.bScreenReader = true;
	return Settings;
}

FForge2AccessibilitySettings UForge2PolishUtilities::GetHearingImpairedPreset()
{
	FForge2AccessibilitySettings Settings;
	Settings.bSubtitlesEnabled = true;
	Settings.SubtitleSize = 1.3f;
	Settings.SubtitleBackground = EForge2SubtitleBackground::Box;
	Settings.bShowSpeakerNames = true;
	Settings.bShowSoundCaptions = true;
	Settings.bShowDirectionalIndicators = true;
	Settings.bVisualSoundCues = true;
	return Settings;
}

FForge2AccessibilitySettings UForge2PolishUtilities::GetMotorImpairedPreset()
{
	FForge2AccessibilitySettings Settings;
	Settings.bAutoAim = true;
	Settings.AutoAimStrength = 0.8f;
	Settings.bHoldToToggle = true;
	Settings.bSimplifiedControls = true;
	Settings.bSkipQTEs = true;
	Settings.DoubleTapWindow = 0.5f;
	Settings.HoldDuration = 0.3f;
	return Settings;
}

FString UForge2PolishUtilities::FormatPlayTime(float Seconds)
{
	int32 Hours = FMath::FloorToInt(Seconds / 3600.0f);
	int32 Minutes = FMath::FloorToInt(FMath::Fmod(Seconds, 3600.0f) / 60.0f);
	int32 Secs = FMath::FloorToInt(FMath::Fmod(Seconds, 60.0f));

	if (Hours > 0)
	{
		return FString::Printf(TEXT("%d:%02d:%02d"), Hours, Minutes, Secs);
	}
	return FString::Printf(TEXT("%d:%02d"), Minutes, Secs);
}

FString UForge2PolishUtilities::FormatDateTime(const FDateTime& DateTime)
{
	return DateTime.ToString(TEXT("%Y-%m-%d %H:%M"));
}

FString UForge2PolishUtilities::FormatNumber(int32 Number, const FString& LanguageCode)
{
	// Would format according to locale conventions
	return FString::FromInt(Number);
}
