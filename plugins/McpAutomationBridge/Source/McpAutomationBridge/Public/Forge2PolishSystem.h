// Copyright 2025 FORGE2 Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SaveGame.h"
#include "Forge2PolishSystem.generated.h"

// Forward declarations
class APlayerController;
class UTexture2D;
class UMaterialInterface;

/**
 * Colorblind modes
 */
UENUM(BlueprintType)
enum class EForge2ColorblindMode : uint8
{
	None UMETA(DisplayName = "None"),
	Protanopia UMETA(DisplayName = "Protanopia (Red-Blind)"),
	Deuteranopia UMETA(DisplayName = "Deuteranopia (Green-Blind)"),
	Tritanopia UMETA(DisplayName = "Tritanopia (Blue-Blind)"),
	Achromatopsia UMETA(DisplayName = "Achromatopsia (Monochromacy)")
};

/**
 * Subtitle background style
 */
UENUM(BlueprintType)
enum class EForge2SubtitleBackground : uint8
{
	None UMETA(DisplayName = "None"),
	Box UMETA(DisplayName = "Box"),
	Gradient UMETA(DisplayName = "Gradient"),
	Shadow UMETA(DisplayName = "Shadow")
};

/**
 * Photo mode filter type
 */
UENUM(BlueprintType)
enum class EForge2PhotoFilter : uint8
{
	None UMETA(DisplayName = "None"),
	Sepia UMETA(DisplayName = "Sepia"),
	BlackAndWhite UMETA(DisplayName = "Black and White"),
	Vintage UMETA(DisplayName = "Vintage"),
	Vivid UMETA(DisplayName = "Vivid"),
	Cold UMETA(DisplayName = "Cold"),
	Warm UMETA(DisplayName = "Warm"),
	Noir UMETA(DisplayName = "Noir"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Photo mode border style
 */
UENUM(BlueprintType)
enum class EForge2PhotoBorder : uint8
{
	None UMETA(DisplayName = "None"),
	Simple UMETA(DisplayName = "Simple"),
	Polaroid UMETA(DisplayName = "Polaroid"),
	Film UMETA(DisplayName = "Film"),
	Ornate UMETA(DisplayName = "Ornate"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Analytics event category
 */
UENUM(BlueprintType)
enum class EForge2AnalyticsCategory : uint8
{
	Gameplay UMETA(DisplayName = "Gameplay"),
	Progression UMETA(DisplayName = "Progression"),
	Economy UMETA(DisplayName = "Economy"),
	Social UMETA(DisplayName = "Social"),
	Technical UMETA(DisplayName = "Technical"),
	Combat UMETA(DisplayName = "Combat"),
	Navigation UMETA(DisplayName = "Navigation"),
	UI UMETA(DisplayName = "UI"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Achievement platform
 */
UENUM(BlueprintType)
enum class EForge2AchievementPlatform : uint8
{
	Steam UMETA(DisplayName = "Steam"),
	PlayStation UMETA(DisplayName = "PlayStation"),
	Xbox UMETA(DisplayName = "Xbox"),
	Nintendo UMETA(DisplayName = "Nintendo"),
	Epic UMETA(DisplayName = "Epic Games"),
	GOG UMETA(DisplayName = "GOG"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Save slot data
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 CharacterLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString CurrentLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float PlayTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FDateTime SaveDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	UTexture2D* Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 SaveVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bIsAutoSave = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bIsQuickSave = false;
};

/**
 * Autosave settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AutosaveSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Autosave")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Autosave")
	float IntervalSeconds = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Autosave")
	int32 MaxAutosaveSlots = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Autosave")
	bool bSaveOnLevelChange = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Autosave")
	bool bSaveOnQuestComplete = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Autosave")
	bool bSaveOnBossDeath = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Autosave")
	bool bNotifyOnAutosave = true;
};

/**
 * Accessibility settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AccessibilitySettings
{
	GENERATED_BODY()

	// Visual
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Visual")
	EForge2ColorblindMode ColorblindMode = EForge2ColorblindMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Visual")
	float ColorblindIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Visual")
	float UIScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Visual")
	bool bHighContrastUI = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Visual")
	bool bReducedMotion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Visual")
	bool bDisableScreenShake = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Visual")
	bool bDisableFlashing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Visual")
	float ScreenFlashIntensity = 1.0f;

	// Subtitles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Subtitles")
	bool bSubtitlesEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Subtitles")
	float SubtitleSize = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Subtitles")
	EForge2SubtitleBackground SubtitleBackground = EForge2SubtitleBackground::Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Subtitles")
	FLinearColor SubtitleBackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.75f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Subtitles")
	bool bShowSpeakerNames = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Subtitles")
	bool bShowSoundCaptions = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Subtitles")
	bool bShowDirectionalIndicators = false;

	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Audio")
	bool bMonoAudio = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Audio")
	bool bVisualSoundCues = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Audio")
	bool bScreenReader = false;

	// Motor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motor")
	bool bAutoAim = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motor")
	float AutoAimStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motor")
	bool bHoldToToggle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motor")
	bool bOneHandedMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motor")
	bool bSimplifiedControls = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motor")
	float DoubleTapWindow = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motor")
	float HoldDuration = 0.5f;

	// Difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Difficulty")
	bool bGodMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Difficulty")
	bool bInfiniteAmmo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Difficulty")
	bool bNoTimers = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Difficulty")
	bool bSkipQTEs = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Difficulty")
	float DamageMultiplier = 1.0f;
};

/**
 * Photo mode settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PhotoModeSettings
{
	GENERATED_BODY()

	// Camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Camera")
	float FOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Camera")
	float Roll = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Camera")
	FVector CameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Camera")
	float MaxCameraDistance = 1000.0f;

	// Depth of Field
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|DOF")
	bool bDOFEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|DOF")
	float FocalDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|DOF")
	float FocalRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|DOF")
	float BokehStrength = 1.0f;

	// Filters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Filter")
	EForge2PhotoFilter Filter = EForge2PhotoFilter::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Filter")
	float FilterIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Filter")
	float Brightness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Filter")
	float Contrast = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Filter")
	float Saturation = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Filter")
	float Vignette = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Filter")
	float Grain = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Filter")
	FLinearColor ColorTint = FLinearColor::White;

	// Frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Frame")
	EForge2PhotoBorder Border = EForge2PhotoBorder::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Frame")
	FLinearColor BorderColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Frame")
	bool bShowWatermark = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Frame")
	bool bShowTimestamp = false;

	// Scene
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Scene")
	bool bHidePlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Scene")
	bool bHideNPCs = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Scene")
	bool bFreezeScene = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Scene")
	float TimeOfDay = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhotoMode|Scene")
	bool bControlWeather = false;
};

/**
 * Character pose
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CharacterPose
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
	FName PoseId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
	UAnimSequence* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
	FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
	UTexture2D* Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
	bool bIsUnlocked = true;
};

/**
 * Achievement definition
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AchievementDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName AchievementId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText LockedDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	UTexture2D* LockedIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	int32 Points = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsSecret = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	int32 TargetProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	TMap<FString, FString> PlatformIds;
};

/**
 * Achievement progress
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AchievementProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName AchievementId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FDateTime UnlockDate;
};

/**
 * Analytics event
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AnalyticsEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	FString EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	EForge2AnalyticsCategory Category = EForge2AnalyticsCategory::Gameplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	TMap<FString, FString> StringParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	TMap<FString, float> NumericParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	FString SessionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	FVector Location = FVector::ZeroVector;
};

/**
 * Session statistics
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SessionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	FString SessionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	FDateTime SessionStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	float PlayTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 DeathCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 KillCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 QuestsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	float DistanceTraveled = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 ItemsCollected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 GoldEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 GoldSpent = 0;
};

/**
 * Heatmap data point
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HeatmapPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heatmap")
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heatmap")
	FString EventType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heatmap")
	float Value = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heatmap")
	FDateTime Timestamp;
};

/**
 * Localization language
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LanguageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FString LanguageCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	FText NativeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	bool bIsRTL = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	bool bHasVoiceOver = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	UTexture2D* FlagIcon;
};

/**
 * Save Game Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2SaveManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2SaveManager();

	// Core Save/Load
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool SaveGame(int32 SlotIndex, const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool LoadGame(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool DeleteSave(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	TArray<FForge2SaveSlotInfo> GetAllSaveSlots() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool DoesSaveExist(int32 SlotIndex) const;

	// Quick Save/Load
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool QuickSave();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool QuickLoad();

	// Autosave
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	void ConfigureAutosave(const FForge2AutosaveSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	void TriggerAutosave();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	void SetAutosaveEnabled(bool bEnabled);

	// Cloud Save
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool UploadToCloud(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool DownloadFromCloud(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool IsCloudSaveAvailable() const;

	// Save Migration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
	bool MigrateSave(int32 SlotIndex, int32 FromVersion, int32 ToVersion);

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameSaved, int32, SlotIndex, bool, bSuccess);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameLoaded, int32, SlotIndex, bool, bSuccess);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAutosaveTriggered);

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Save")
	FOnGameSaved OnGameSaved;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Save")
	FOnGameLoaded OnGameLoaded;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Save")
	FOnAutosaveTriggered OnAutosaveTriggered;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	FForge2AutosaveSettings AutosaveSettings;

	UPROPERTY()
	float TimeSinceLastAutosave = 0.0f;

	UPROPERTY()
	int32 CurrentAutosaveSlot = 0;

	UPROPERTY()
	int32 QuickSaveSlot = -1;
};

/**
 * Accessibility Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2AccessibilityManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2AccessibilityManager();

	// Settings
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility")
	void ApplySettings(const FForge2AccessibilitySettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility")
	FForge2AccessibilitySettings GetSettings() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility")
	void ResetToDefaults();

	// Colorblind
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Colorblind")
	void SetColorblindMode(EForge2ColorblindMode Mode);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Colorblind")
	void SetColorblindIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Colorblind")
	FLinearColor AdjustColorForColorblind(FLinearColor OriginalColor) const;

	// Subtitles
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Subtitles")
	void SetSubtitlesEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Subtitles")
	void SetSubtitleSize(float Size);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Subtitles")
	void SetSubtitleBackground(EForge2SubtitleBackground Background);

	// Motor
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Motor")
	void SetAutoAim(bool bEnabled, float Strength);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Motor")
	void SetHoldToToggle(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Motor")
	void SetOneHandedMode(bool bEnabled);

	// Visual
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Visual")
	void SetUIScale(float Scale);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Visual")
	void SetHighContrast(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Visual")
	void SetReducedMotion(bool bEnabled);

	// Difficulty Assists
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Difficulty")
	void SetGodMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Accessibility|Difficulty")
	void SetDamageMultiplier(float Multiplier);

protected:
	UPROPERTY()
	FForge2AccessibilitySettings CurrentSettings;

	void ApplyColorblindFilter();
	void UpdateSubtitleSettings();
	void UpdateMotorSettings();
};

/**
 * Photo Mode Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2PhotoModeManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2PhotoModeManager();

	// Mode Control
	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode")
	void EnterPhotoMode();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode")
	void ExitPhotoMode();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode")
	bool IsInPhotoMode() const;

	// Camera
	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Camera")
	void SetFOV(float FOV);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Camera")
	void SetRoll(float Roll);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Camera")
	void SetCameraOffset(FVector Offset);

	// DOF
	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|DOF")
	void SetDOFEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|DOF")
	void SetFocalDistance(float Distance);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|DOF")
	void SetBokehStrength(float Strength);

	// Filters
	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Filter")
	void ApplyFilter(EForge2PhotoFilter Filter, float Intensity);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Filter")
	void SetBrightness(float Brightness);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Filter")
	void SetContrast(float Contrast);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Filter")
	void SetSaturation(float Saturation);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Filter")
	void SetVignette(float Vignette);

	// Poses
	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Pose")
	void RegisterPose(const FForge2CharacterPose& Pose);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Pose")
	void ApplyPose(FName PoseId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Pose")
	TArray<FForge2CharacterPose> GetAvailablePoses() const;

	// Scene
	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Scene")
	void SetHidePlayer(bool bHide);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Scene")
	void SetHideNPCs(bool bHide);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Scene")
	void SetTimeOfDay(float Hour);

	// Capture
	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Capture")
	bool TakePhoto(const FString& Filename);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode|Capture")
	FString GetPhotosDirectory() const;

	// Settings
	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode")
	void ApplySettings(const FForge2PhotoModeSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|PhotoMode")
	FForge2PhotoModeSettings GetSettings() const;

protected:
	UPROPERTY()
	FForge2PhotoModeSettings CurrentSettings;

	UPROPERTY()
	TArray<FForge2CharacterPose> RegisteredPoses;

	UPROPERTY()
	bool bPhotoModeActive = false;

	UPROPERTY()
	FVector StoredCameraLocation;

	UPROPERTY()
	FRotator StoredCameraRotation;
};

/**
 * Achievement Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2AchievementManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2AchievementManager();

	// Registration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	void RegisterAchievement(const FForge2AchievementDefinition& Achievement);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	TArray<FForge2AchievementDefinition> GetAllAchievements() const;

	// Progress
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	void SetProgress(FName AchievementId, int32 Progress);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	void AddProgress(FName AchievementId, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	FForge2AchievementProgress GetProgress(FName AchievementId) const;

	// Unlock
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	void UnlockAchievement(FName AchievementId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	bool IsAchievementUnlocked(FName AchievementId) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	TArray<FName> GetUnlockedAchievements() const;

	// Platform Integration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	void SyncWithPlatform(EForge2AchievementPlatform Platform);

	// Stats
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	int32 GetTotalPoints() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Achievements")
	float GetCompletionPercentage() const;

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, FName, AchievementId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementProgress, FName, AchievementId, float, ProgressPercent);

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Achievements")
	FOnAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Achievements")
	FOnAchievementProgress OnAchievementProgress;

protected:
	UPROPERTY()
	TMap<FName, FForge2AchievementDefinition> Achievements;

	UPROPERTY()
	TMap<FName, FForge2AchievementProgress> Progress;
};

/**
 * Analytics Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2AnalyticsManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2AnalyticsManager();

	// Events
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics")
	void TrackEvent(const FForge2AnalyticsEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics")
	void TrackSimpleEvent(const FString& EventName, EForge2AnalyticsCategory Category);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics")
	void TrackEventWithParams(const FString& EventName, const TMap<FString, FString>& Params);

	// Session
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics")
	void EndSession();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics")
	FForge2SessionStats GetSessionStats() const;

	// Heatmaps
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics|Heatmap")
	void RecordHeatmapPoint(FVector Location, const FString& EventType);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics|Heatmap")
	TArray<FForge2HeatmapPoint> GetHeatmapData(const FString& EventType) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics|Heatmap")
	void ClearHeatmapData();

	// Funnels
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics|Funnel")
	void TrackFunnelStep(const FString& FunnelName, int32 StepIndex);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics|Funnel")
	TMap<int32, int32> GetFunnelData(const FString& FunnelName) const;

	// A/B Testing
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics|ABTest")
	void RegisterExperiment(const FString& ExperimentName, const TArray<FString>& Variants);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics|ABTest")
	FString GetExperimentVariant(const FString& ExperimentName) const;

	// Settings
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics")
	void SetAnalyticsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Analytics")
	bool AreAnalyticsEnabled() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	TArray<FForge2AnalyticsEvent> EventQueue;

	UPROPERTY()
	FForge2SessionStats CurrentSession;

	UPROPERTY()
	TArray<FForge2HeatmapPoint> HeatmapData;

	UPROPERTY()
	TMap<FString, TArray<FString>> Experiments;

	UPROPERTY()
	TMap<FString, FString> ExperimentAssignments;

	UPROPERTY()
	TMap<FString, TMap<int32, int32>> FunnelData;

	UPROPERTY()
	bool bAnalyticsEnabled = true;
};

/**
 * Localization Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2LocalizationManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2LocalizationManager();

	// Language
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	void SetLanguage(const FString& LanguageCode);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	FString GetCurrentLanguage() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	TArray<FForge2LanguageInfo> GetAvailableLanguages() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	void RegisterLanguage(const FForge2LanguageInfo& LanguageInfo);

	// Localized Text
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	FText GetLocalizedText(const FString& Key) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	FText FormatLocalizedText(const FString& Key, const TMap<FString, FString>& Arguments) const;

	// Audio Localization
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	void SetAudioLanguage(const FString& LanguageCode);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	FString GetAudioLanguage() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	bool IsAudioLanguageAvailable(const FString& LanguageCode) const;

	// Font
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	UFont* GetFontForLanguage(const FString& LanguageCode) const;

	// RTL Support
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Localization")
	bool IsCurrentLanguageRTL() const;

protected:
	UPROPERTY()
	TArray<FForge2LanguageInfo> RegisteredLanguages;

	UPROPERTY()
	FString CurrentLanguageCode;

	UPROPERTY()
	FString CurrentAudioLanguage;
};

/**
 * Polish Utilities
 */
UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2PolishUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Colorblind simulation
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Polish|Utilities")
	static FLinearColor SimulateColorblindness(FLinearColor Color, EForge2ColorblindMode Mode);

	// Photo mode presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Polish|Utilities")
	static FForge2PhotoModeSettings GetPhotoModePreset(EForge2PhotoFilter Filter);

	// Accessibility presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Polish|Utilities")
	static FForge2AccessibilitySettings GetVisuallyImpairedPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Polish|Utilities")
	static FForge2AccessibilitySettings GetHearingImpairedPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Polish|Utilities")
	static FForge2AccessibilitySettings GetMotorImpairedPreset();

	// Format helpers
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Polish|Utilities")
	static FString FormatPlayTime(float Seconds);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Polish|Utilities")
	static FString FormatDateTime(const FDateTime& DateTime);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Polish|Utilities")
	static FString FormatNumber(int32 Number, const FString& LanguageCode);
};
