// Copyright 2025 FORGE2 Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blueprint/UserWidget.h"
#include "Forge2UISystem.generated.h"

// Forward declarations
class UWidgetComponent;
class UWidgetAnimation;
class UImage;
class UTextBlock;
class UProgressBar;
class UButton;

/**
 * HUD element types
 */
UENUM(BlueprintType)
enum class EForge2HUDElementType : uint8
{
	HealthBar UMETA(DisplayName = "Health Bar"),
	ManaBar UMETA(DisplayName = "Mana Bar"),
	StaminaBar UMETA(DisplayName = "Stamina Bar"),
	ExperienceBar UMETA(DisplayName = "Experience Bar"),
	Crosshair UMETA(DisplayName = "Crosshair"),
	AmmoCounter UMETA(DisplayName = "Ammo Counter"),
	Minimap UMETA(DisplayName = "Minimap"),
	Compass UMETA(DisplayName = "Compass"),
	QuestTracker UMETA(DisplayName = "Quest Tracker"),
	Hotbar UMETA(DisplayName = "Hotbar"),
	BuffBar UMETA(DisplayName = "Buff Bar"),
	InteractionPrompt UMETA(DisplayName = "Interaction Prompt"),
	DamageIndicator UMETA(DisplayName = "Damage Indicator"),
	NotificationArea UMETA(DisplayName = "Notification Area"),
	SubtitleArea UMETA(DisplayName = "Subtitle Area"),
	BossHealthBar UMETA(DisplayName = "Boss Health Bar"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Menu types
 */
UENUM(BlueprintType)
enum class EForge2MenuType : uint8
{
	MainMenu UMETA(DisplayName = "Main Menu"),
	PauseMenu UMETA(DisplayName = "Pause Menu"),
	OptionsMenu UMETA(DisplayName = "Options Menu"),
	InventoryMenu UMETA(DisplayName = "Inventory Menu"),
	CharacterMenu UMETA(DisplayName = "Character Menu"),
	MapMenu UMETA(DisplayName = "Map Menu"),
	QuestMenu UMETA(DisplayName = "Quest Menu"),
	SkillMenu UMETA(DisplayName = "Skill Menu"),
	CraftingMenu UMETA(DisplayName = "Crafting Menu"),
	ShopMenu UMETA(DisplayName = "Shop Menu"),
	DialogueMenu UMETA(DisplayName = "Dialogue Menu"),
	LoadingScreen UMETA(DisplayName = "Loading Screen"),
	DeathScreen UMETA(DisplayName = "Death Screen"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Widget anchor presets
 */
UENUM(BlueprintType)
enum class EForge2WidgetAnchor : uint8
{
	TopLeft UMETA(DisplayName = "Top Left"),
	TopCenter UMETA(DisplayName = "Top Center"),
	TopRight UMETA(DisplayName = "Top Right"),
	CenterLeft UMETA(DisplayName = "Center Left"),
	Center UMETA(DisplayName = "Center"),
	CenterRight UMETA(DisplayName = "Center Right"),
	BottomLeft UMETA(DisplayName = "Bottom Left"),
	BottomCenter UMETA(DisplayName = "Bottom Center"),
	BottomRight UMETA(DisplayName = "Bottom Right"),
	StretchHorizontal UMETA(DisplayName = "Stretch Horizontal"),
	StretchVertical UMETA(DisplayName = "Stretch Vertical"),
	StretchFull UMETA(DisplayName = "Stretch Full")
};

/**
 * Notification type
 */
UENUM(BlueprintType)
enum class EForge2NotificationType : uint8
{
	Info UMETA(DisplayName = "Info"),
	Success UMETA(DisplayName = "Success"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error"),
	Achievement UMETA(DisplayName = "Achievement"),
	Quest UMETA(DisplayName = "Quest"),
	LevelUp UMETA(DisplayName = "Level Up"),
	ItemPickup UMETA(DisplayName = "Item Pickup"),
	System UMETA(DisplayName = "System")
};

/**
 * Tooltip position
 */
UENUM(BlueprintType)
enum class EForge2TooltipPosition : uint8
{
	Auto UMETA(DisplayName = "Auto"),
	Above UMETA(DisplayName = "Above"),
	Below UMETA(DisplayName = "Below"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	Cursor UMETA(DisplayName = "Cursor")
};

/**
 * Input prompt type
 */
UENUM(BlueprintType)
enum class EForge2InputPromptType : uint8
{
	Keyboard UMETA(DisplayName = "Keyboard"),
	Xbox UMETA(DisplayName = "Xbox"),
	PlayStation UMETA(DisplayName = "PlayStation"),
	Switch UMETA(DisplayName = "Switch"),
	Generic UMETA(DisplayName = "Generic"),
	Auto UMETA(DisplayName = "Auto")
};

/**
 * Tutorial highlight style
 */
UENUM(BlueprintType)
enum class EForge2TutorialHighlight : uint8
{
	None UMETA(DisplayName = "None"),
	Glow UMETA(DisplayName = "Glow"),
	Pulse UMETA(DisplayName = "Pulse"),
	Arrow UMETA(DisplayName = "Arrow"),
	Circle UMETA(DisplayName = "Circle"),
	Spotlight UMETA(DisplayName = "Spotlight")
};

/**
 * Widget animation type
 */
UENUM(BlueprintType)
enum class EForge2WidgetAnimationType : uint8
{
	None UMETA(DisplayName = "None"),
	FadeIn UMETA(DisplayName = "Fade In"),
	FadeOut UMETA(DisplayName = "Fade Out"),
	SlideIn UMETA(DisplayName = "Slide In"),
	SlideOut UMETA(DisplayName = "Slide Out"),
	ScaleIn UMETA(DisplayName = "Scale In"),
	ScaleOut UMETA(DisplayName = "Scale Out"),
	Bounce UMETA(DisplayName = "Bounce"),
	Shake UMETA(DisplayName = "Shake"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * HUD element configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HUDElementConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FName ElementId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	EForge2HUDElementType ElementType = EForge2HUDElementType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	EForge2WidgetAnchor Anchor = EForge2WidgetAnchor::TopLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FVector2D Size = FVector2D(100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	float Opacity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	int32 ZOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TMap<FName, FString> CustomParameters;
};

/**
 * Menu configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MenuConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FName MenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	EForge2MenuType MenuType = EForge2MenuType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool bPauseGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool bShowCursor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool bCaptureInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	EForge2WidgetAnimationType OpenAnimation = EForge2WidgetAnimationType::FadeIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	EForge2WidgetAnimationType CloseAnimation = EForge2WidgetAnimationType::FadeOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	float AnimationDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	USoundBase* OpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	USoundBase* CloseSound;
};

/**
 * Notification configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NotificationConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	EForge2NotificationType Type = EForge2NotificationType::Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	bool bPlaySound = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	USoundBase* CustomSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	FLinearColor BackgroundColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	FLinearColor BorderColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	int32 Priority = 0;
};

/**
 * Tooltip configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TooltipConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	EForge2TooltipPosition Position = EForge2TooltipPosition::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	float Delay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	FVector2D Offset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	TSubclassOf<UUserWidget> CustomWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	TMap<FName, FString> ExtraData;
};

/**
 * Input prompt mapping
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InputPromptMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FName ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UTexture2D* KeyboardIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UTexture2D* XboxIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UTexture2D* PlayStationIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UTexture2D* SwitchIcon;
};

/**
 * Tutorial step
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TutorialStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName StepId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName HighlightWidgetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	EForge2TutorialHighlight HighlightStyle = EForge2TutorialHighlight::Pulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FVector WorldHighlightLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bUseWorldLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName RequiredAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bBlockInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bSkippable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	float AutoAdvanceTime = 0.0f;
};

/**
 * Tutorial sequence
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TutorialSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName SequenceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText SequenceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TArray<FForge2TutorialStep> Steps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bRequired = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	int32 Priority = 0;
};

/**
 * Damage number configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DamageNumberConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	int32 DamageAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	FName DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	bool bIsCritical = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	bool bIsHealing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	float Scale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	float Duration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	FVector2D FloatDirection = FVector2D(0.0f, -1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumbers")
	float FloatSpeed = 100.0f;
};

/**
 * Subtitle configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SubtitleConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	FLinearColor SpeakerColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	bool bShowBackground = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	int32 FontSize = 24;
};

/**
 * Loading screen configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LoadingScreenConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	UTexture2D* BackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	TArray<FText> Tips;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	float TipRotationInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	bool bShowProgressBar = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	bool bShowSpinner = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	FText LoadingText = FText::FromString(TEXT("Loading..."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	float MinimumDisplayTime = 2.0f;
};

/**
 * UI Theme
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2UITheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FName ThemeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor PrimaryColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor SecondaryColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor AccentColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor BackgroundColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor ErrorColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor SuccessColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor WarningColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Fonts")
	FSlateFontInfo TitleFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Fonts")
	FSlateFontInfo BodyFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Fonts")
	FSlateFontInfo ButtonFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Spacing")
	float ElementPadding = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Spacing")
	float ElementSpacing = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Spacing")
	float BorderRadius = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Spacing")
	float BorderWidth = 1.0f;
};

/**
 * UI Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2UIManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2UIManager();

	// Initialization
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI")
	void InitializeUI(APlayerController* OwnerController);

	// HUD Management
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|HUD")
	UUserWidget* CreateHUDElement(const FForge2HUDElementConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|HUD")
	void ShowHUDElement(FName ElementId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|HUD")
	void HideHUDElement(FName ElementId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|HUD")
	void ToggleHUDElement(FName ElementId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|HUD")
	void SetHUDElementOpacity(FName ElementId, float Opacity);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|HUD")
	void HideAllHUD();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|HUD")
	void ShowAllHUD();

	// Menu Management
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Menu")
	UUserWidget* OpenMenu(const FForge2MenuConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Menu")
	void CloseMenu(FName MenuId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Menu")
	void CloseAllMenus();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Menu")
	void ToggleMenu(FName MenuId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Menu")
	bool IsMenuOpen(FName MenuId) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Menu")
	UUserWidget* GetOpenMenu(FName MenuId) const;

	// Notifications
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Notifications")
	void ShowNotification(const FForge2NotificationConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Notifications")
	void ClearNotifications();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Notifications")
	void SetMaxNotifications(int32 MaxCount);

	// Tooltips
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Tooltips")
	void ShowTooltip(UWidget* Target, const FForge2TooltipConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Tooltips")
	void HideTooltip();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Tooltips")
	void SetTooltipDelay(float Delay);

	// Input Prompts
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Input")
	void RegisterInputPrompt(const FForge2InputPromptMapping& Mapping);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Input")
	UTexture2D* GetInputIcon(FName ActionName) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Input")
	FText GetInputDisplayText(FName ActionName) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Input")
	void SetInputPromptType(EForge2InputPromptType Type);

	// Damage Numbers
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|DamageNumbers")
	void SpawnDamageNumber(const FVector& WorldLocation, const FForge2DamageNumberConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|DamageNumbers")
	void ConfigureDamageNumbers(TSubclassOf<UUserWidget> WidgetClass);

	// Subtitles
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Subtitles")
	void ShowSubtitle(const FForge2SubtitleConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Subtitles")
	void HideSubtitles();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Subtitles")
	void SetSubtitlesEnabled(bool bEnabled);

	// Loading Screen
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Loading")
	void ShowLoadingScreen(const FForge2LoadingScreenConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Loading")
	void UpdateLoadingProgress(float Progress);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Loading")
	void HideLoadingScreen();

	// Theme
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Theme")
	void ApplyTheme(const FForge2UITheme& Theme);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Theme")
	FForge2UITheme GetCurrentTheme() const;

	// Widget Animations
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Animation")
	void PlayWidgetAnimation(UUserWidget* Widget, EForge2WidgetAnimationType AnimationType, float Duration);

protected:
	UPROPERTY()
	APlayerController* OwningController;

	UPROPERTY()
	TMap<FName, UUserWidget*> HUDElements;

	UPROPERTY()
	TMap<FName, UUserWidget*> OpenMenus;

	UPROPERTY()
	TArray<FForge2InputPromptMapping> InputPrompts;

	UPROPERTY()
	EForge2InputPromptType CurrentInputType = EForge2InputPromptType::Auto;

	UPROPERTY()
	FForge2UITheme CurrentTheme;

	UPROPERTY()
	int32 MaxNotifications = 5;

	UPROPERTY()
	bool bSubtitlesEnabled = true;

	UPROPERTY()
	UUserWidget* CurrentTooltip;

	UPROPERTY()
	UUserWidget* LoadingScreenWidget;
};

/**
 * Tutorial Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2TutorialManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2TutorialManager();

	// Tutorial Sequences
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	void RegisterTutorialSequence(const FForge2TutorialSequence& Sequence);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	bool StartTutorial(FName SequenceId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	void StopTutorial();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	void AdvanceTutorial();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	void SkipTutorial();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	void CompleteTutorialStep(FName StepId);

	// Status
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	bool IsTutorialActive() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	FForge2TutorialStep GetCurrentStep() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	int32 GetCurrentStepIndex() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	int32 GetTotalSteps() const;

	// Completion Tracking
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	bool IsSequenceCompleted(FName SequenceId) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	void MarkSequenceCompleted(FName SequenceId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	void ResetTutorialProgress();

	// Settings
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	void SetTutorialsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Tutorial")
	bool AreTutorialsEnabled() const;

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStarted, FName, SequenceId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialCompleted, FName, SequenceId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepChanged, int32, StepIndex, FForge2TutorialStep, Step);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialSkipped);

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Tutorial")
	FOnTutorialStarted OnTutorialStarted;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Tutorial")
	FOnTutorialCompleted OnTutorialCompleted;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Tutorial")
	FOnTutorialStepChanged OnTutorialStepChanged;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Tutorial")
	FOnTutorialSkipped OnTutorialSkipped;

protected:
	UPROPERTY()
	TMap<FName, FForge2TutorialSequence> RegisteredSequences;

	UPROPERTY()
	TSet<FName> CompletedSequences;

	UPROPERTY()
	FName CurrentSequenceId;

	UPROPERTY()
	int32 CurrentStepIndex = -1;

	UPROPERTY()
	bool bTutorialActive = false;

	UPROPERTY()
	bool bTutorialsEnabled = true;
};

/**
 * UI Utilities
 */
UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2UIUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Anchor calculations
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FAnchors GetAnchorsForPreset(EForge2WidgetAnchor Anchor);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FVector2D GetAlignmentForPreset(EForge2WidgetAnchor Anchor);

	// Color utilities
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FLinearColor GetNotificationColor(EForge2NotificationType Type);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FLinearColor GetDamageNumberColor(FName DamageType, bool bIsCritical, bool bIsHealing);

	// Widget helpers
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static UUserWidget* CreateWidgetFromClass(APlayerController* Owner, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static void SetWidgetOpacity(UUserWidget* Widget, float Opacity);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FVector2D GetScreenCenter(APlayerController* Controller);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FVector2D WorldToScreen(APlayerController* Controller, FVector WorldLocation);

	// Theme presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FForge2UITheme GetDefaultTheme();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FForge2UITheme GetDarkTheme();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FForge2UITheme GetLightTheme();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FForge2UITheme GetFantasyTheme();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FForge2UITheme GetSciFiTheme();

	// HUD presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static TArray<FForge2HUDElementConfig> GetRPGHUDPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static TArray<FForge2HUDElementConfig> GetFPSHUDPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static TArray<FForge2HUDElementConfig> GetMinimalHUDPreset();

	// Loading screen presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|UI|Utilities")
	static FForge2LoadingScreenConfig GetDefaultLoadingScreen();
};
