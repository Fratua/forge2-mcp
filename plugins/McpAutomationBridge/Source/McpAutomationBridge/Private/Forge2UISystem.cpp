// Copyright 2025 FORGE2 Project. All Rights Reserved.

#include "Forge2UISystem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

// ============================================================================
// UForge2UIManager Implementation
// ============================================================================

UForge2UIManager::UForge2UIManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2UIManager::InitializeUI(APlayerController* OwnerController)
{
	OwningController = OwnerController;
	CurrentTheme = UForge2UIUtilities::GetDefaultTheme();
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Manager initialized"));
}

UUserWidget* UForge2UIManager::CreateHUDElement(const FForge2HUDElementConfig& Config)
{
	if (!OwningController || !Config.WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("FORGE2 UI: Cannot create HUD element - missing controller or widget class"));
		return nullptr;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(OwningController, Config.WidgetClass);
	if (Widget)
	{
		Widget->AddToViewport(Config.ZOrder);
		Widget->SetVisibility(Config.bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Widget->SetRenderOpacity(Config.Opacity);

		HUDElements.Add(Config.ElementId, Widget);
		UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Created HUD element %s"), *Config.ElementId.ToString());
	}

	return Widget;
}

void UForge2UIManager::ShowHUDElement(FName ElementId)
{
	if (UUserWidget** WidgetPtr = HUDElements.Find(ElementId))
	{
		(*WidgetPtr)->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Showing HUD element %s"), *ElementId.ToString());
	}
}

void UForge2UIManager::HideHUDElement(FName ElementId)
{
	if (UUserWidget** WidgetPtr = HUDElements.Find(ElementId))
	{
		(*WidgetPtr)->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Hiding HUD element %s"), *ElementId.ToString());
	}
}

void UForge2UIManager::ToggleHUDElement(FName ElementId)
{
	if (UUserWidget** WidgetPtr = HUDElements.Find(ElementId))
	{
		ESlateVisibility Current = (*WidgetPtr)->GetVisibility();
		(*WidgetPtr)->SetVisibility(Current == ESlateVisibility::Visible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UForge2UIManager::SetHUDElementOpacity(FName ElementId, float Opacity)
{
	if (UUserWidget** WidgetPtr = HUDElements.Find(ElementId))
	{
		(*WidgetPtr)->SetRenderOpacity(FMath::Clamp(Opacity, 0.0f, 1.0f));
	}
}

void UForge2UIManager::HideAllHUD()
{
	for (auto& Pair : HUDElements)
	{
		if (Pair.Value)
		{
			Pair.Value->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: All HUD elements hidden"));
}

void UForge2UIManager::ShowAllHUD()
{
	for (auto& Pair : HUDElements)
	{
		if (Pair.Value)
		{
			Pair.Value->SetVisibility(ESlateVisibility::Visible);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: All HUD elements shown"));
}

UUserWidget* UForge2UIManager::OpenMenu(const FForge2MenuConfig& Config)
{
	if (!OwningController || !Config.WidgetClass)
	{
		return nullptr;
	}

	// Close existing menu of same ID if open
	if (OpenMenus.Contains(Config.MenuId))
	{
		CloseMenu(Config.MenuId);
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(OwningController, Config.WidgetClass);
	if (Widget)
	{
		Widget->AddToViewport(100); // Menus above HUD
		OpenMenus.Add(Config.MenuId, Widget);

		// Configure input mode
		if (Config.bCaptureInput)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Widget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			OwningController->SetInputMode(InputMode);
		}

		if (Config.bShowCursor)
		{
			OwningController->SetShowMouseCursor(true);
		}

		if (Config.bPauseGame)
		{
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}

		// Play animation
		if (Config.AnimationDuration > 0.0f)
		{
			PlayWidgetAnimation(Widget, Config.OpenAnimation, Config.AnimationDuration);
		}

		// Play sound
		if (Config.OpenSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), Config.OpenSound);
		}

		UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Opened menu %s"), *Config.MenuId.ToString());
	}

	return Widget;
}

void UForge2UIManager::CloseMenu(FName MenuId)
{
	if (UUserWidget** WidgetPtr = OpenMenus.Find(MenuId))
	{
		UUserWidget* Widget = *WidgetPtr;
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
		OpenMenus.Remove(MenuId);

		// Restore input mode if no menus open
		if (OpenMenus.Num() == 0 && OwningController)
		{
			FInputModeGameOnly InputMode;
			OwningController->SetInputMode(InputMode);
			OwningController->SetShowMouseCursor(false);
			UGameplayStatics::SetGamePaused(GetWorld(), false);
		}

		UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Closed menu %s"), *MenuId.ToString());
	}
}

void UForge2UIManager::CloseAllMenus()
{
	TArray<FName> MenuIds;
	OpenMenus.GetKeys(MenuIds);

	for (const FName& MenuId : MenuIds)
	{
		CloseMenu(MenuId);
	}
}

void UForge2UIManager::ToggleMenu(FName MenuId)
{
	if (IsMenuOpen(MenuId))
	{
		CloseMenu(MenuId);
	}
	else
	{
		// Would need config to open - this is a simplified toggle
		UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Toggle requires menu config to open"));
	}
}

bool UForge2UIManager::IsMenuOpen(FName MenuId) const
{
	return OpenMenus.Contains(MenuId);
}

UUserWidget* UForge2UIManager::GetOpenMenu(FName MenuId) const
{
	const UUserWidget* const* WidgetPtr = OpenMenus.Find(MenuId);
	return WidgetPtr ? const_cast<UUserWidget*>(*WidgetPtr) : nullptr;
}

void UForge2UIManager::ShowNotification(const FForge2NotificationConfig& Config)
{
	// In production, this would create and queue notification widgets
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Notification [%d] %s: %s"),
		(int32)Config.Type, *Config.Title.ToString(), *Config.Message.ToString());

	// Play sound
	if (Config.bPlaySound && Config.CustomSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Config.CustomSound);
	}
}

void UForge2UIManager::ClearNotifications()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Notifications cleared"));
}

void UForge2UIManager::SetMaxNotifications(int32 MaxCount)
{
	MaxNotifications = FMath::Max(1, MaxCount);
}

void UForge2UIManager::ShowTooltip(UWidget* Target, const FForge2TooltipConfig& Config)
{
	if (!OwningController || !Target)
	{
		return;
	}

	HideTooltip();

	// Create tooltip widget
	if (Config.CustomWidgetClass)
	{
		CurrentTooltip = CreateWidget<UUserWidget>(OwningController, Config.CustomWidgetClass);
	}

	if (CurrentTooltip)
	{
		CurrentTooltip->AddToViewport(200);

		// Position tooltip
		FVector2D MousePos;
		OwningController->GetMousePosition(MousePos.X, MousePos.Y);
		// Would position based on Config.Position

		UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Showing tooltip - %s"), *Config.Title.ToString());
	}
}

void UForge2UIManager::HideTooltip()
{
	if (CurrentTooltip)
	{
		CurrentTooltip->RemoveFromParent();
		CurrentTooltip = nullptr;
	}
}

void UForge2UIManager::SetTooltipDelay(float Delay)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Tooltip delay set to %.2f"), Delay);
}

void UForge2UIManager::RegisterInputPrompt(const FForge2InputPromptMapping& Mapping)
{
	InputPrompts.Add(Mapping);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Registered input prompt for %s"), *Mapping.ActionName.ToString());
}

UTexture2D* UForge2UIManager::GetInputIcon(FName ActionName) const
{
	for (const FForge2InputPromptMapping& Mapping : InputPrompts)
	{
		if (Mapping.ActionName == ActionName)
		{
			switch (CurrentInputType)
			{
			case EForge2InputPromptType::Keyboard:
				return Mapping.KeyboardIcon;
			case EForge2InputPromptType::Xbox:
				return Mapping.XboxIcon;
			case EForge2InputPromptType::PlayStation:
				return Mapping.PlayStationIcon;
			case EForge2InputPromptType::Switch:
				return Mapping.SwitchIcon;
			case EForge2InputPromptType::Auto:
			default:
				// Would detect controller type
				return Mapping.KeyboardIcon;
			}
		}
	}
	return nullptr;
}

FText UForge2UIManager::GetInputDisplayText(FName ActionName) const
{
	for (const FForge2InputPromptMapping& Mapping : InputPrompts)
	{
		if (Mapping.ActionName == ActionName)
		{
			return Mapping.DisplayText;
		}
	}
	return FText::GetEmpty();
}

void UForge2UIManager::SetInputPromptType(EForge2InputPromptType Type)
{
	CurrentInputType = Type;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Input prompt type set to %d"), (int32)Type);
}

void UForge2UIManager::SpawnDamageNumber(const FVector& WorldLocation, const FForge2DamageNumberConfig& Config)
{
	if (!OwningController)
	{
		return;
	}

	// Convert world location to screen
	FVector2D ScreenPos;
	if (OwningController->ProjectWorldLocationToScreen(WorldLocation, ScreenPos))
	{
		// Would create damage number widget at screen position
		UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Damage number %d at screen (%.0f, %.0f)"),
			Config.DamageAmount, ScreenPos.X, ScreenPos.Y);
	}
}

void UForge2UIManager::ConfigureDamageNumbers(TSubclassOf<UUserWidget> WidgetClass)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Damage number widget class configured"));
}

void UForge2UIManager::ShowSubtitle(const FForge2SubtitleConfig& Config)
{
	if (!bSubtitlesEnabled)
	{
		return;
	}

	// Would display subtitle widget
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Subtitle [%s]: %s"),
		*Config.SpeakerName.ToString(), *Config.Text.ToString());
}

void UForge2UIManager::HideSubtitles()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Subtitles hidden"));
}

void UForge2UIManager::SetSubtitlesEnabled(bool bEnabled)
{
	bSubtitlesEnabled = bEnabled;
	if (!bEnabled)
	{
		HideSubtitles();
	}
}

void UForge2UIManager::ShowLoadingScreen(const FForge2LoadingScreenConfig& Config)
{
	if (!OwningController)
	{
		return;
	}

	// Would create loading screen widget
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Loading screen shown"));
}

void UForge2UIManager::UpdateLoadingProgress(float Progress)
{
	// Would update loading bar widget
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Loading progress: %.0f%%"), Progress * 100.0f);
}

void UForge2UIManager::HideLoadingScreen()
{
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;
	}
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Loading screen hidden"));
}

void UForge2UIManager::ApplyTheme(const FForge2UITheme& Theme)
{
	CurrentTheme = Theme;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 UI: Applied theme %s"), *Theme.ThemeName.ToString());

	// Would apply theme to all existing widgets
}

FForge2UITheme UForge2UIManager::GetCurrentTheme() const
{
	return CurrentTheme;
}

void UForge2UIManager::PlayWidgetAnimation(UUserWidget* Widget, EForge2WidgetAnimationType AnimationType, float Duration)
{
	if (!Widget)
	{
		return;
	}

	// In production, would trigger widget blueprint animations
	switch (AnimationType)
	{
	case EForge2WidgetAnimationType::FadeIn:
		Widget->SetRenderOpacity(0.0f);
		// Would animate to 1.0
		break;
	case EForge2WidgetAnimationType::FadeOut:
		// Would animate opacity to 0.0
		break;
	case EForge2WidgetAnimationType::ScaleIn:
		Widget->SetRenderTransform(FWidgetTransform());
		break;
	default:
		break;
	}
}

// ============================================================================
// UForge2TutorialManager Implementation
// ============================================================================

UForge2TutorialManager::UForge2TutorialManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2TutorialManager::RegisterTutorialSequence(const FForge2TutorialSequence& Sequence)
{
	RegisteredSequences.Add(Sequence.SequenceId, Sequence);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Registered sequence %s with %d steps"),
		*Sequence.SequenceId.ToString(), Sequence.Steps.Num());
}

bool UForge2TutorialManager::StartTutorial(FName SequenceId)
{
	if (!bTutorialsEnabled)
	{
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Tutorials are disabled"));
		return false;
	}

	if (!RegisteredSequences.Contains(SequenceId))
	{
		UE_LOG(LogTemp, Warning, TEXT("FORGE2 Tutorial: Sequence %s not found"), *SequenceId.ToString());
		return false;
	}

	if (IsSequenceCompleted(SequenceId))
	{
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Sequence %s already completed"), *SequenceId.ToString());
		return false;
	}

	CurrentSequenceId = SequenceId;
	CurrentStepIndex = 0;
	bTutorialActive = true;

	OnTutorialStarted.Broadcast(SequenceId);

	const FForge2TutorialSequence& Sequence = RegisteredSequences[SequenceId];
	if (Sequence.Steps.Num() > 0)
	{
		OnTutorialStepChanged.Broadcast(0, Sequence.Steps[0]);
	}

	UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Started sequence %s"), *SequenceId.ToString());
	return true;
}

void UForge2TutorialManager::StopTutorial()
{
	if (bTutorialActive)
	{
		bTutorialActive = false;
		CurrentSequenceId = NAME_None;
		CurrentStepIndex = -1;
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Tutorial stopped"));
	}
}

void UForge2TutorialManager::AdvanceTutorial()
{
	if (!bTutorialActive || !RegisteredSequences.Contains(CurrentSequenceId))
	{
		return;
	}

	const FForge2TutorialSequence& Sequence = RegisteredSequences[CurrentSequenceId];
	CurrentStepIndex++;

	if (CurrentStepIndex >= Sequence.Steps.Num())
	{
		// Tutorial complete
		MarkSequenceCompleted(CurrentSequenceId);
		OnTutorialCompleted.Broadcast(CurrentSequenceId);
		StopTutorial();
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Sequence completed"));
	}
	else
	{
		OnTutorialStepChanged.Broadcast(CurrentStepIndex, Sequence.Steps[CurrentStepIndex]);
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Advanced to step %d"), CurrentStepIndex);
	}
}

void UForge2TutorialManager::SkipTutorial()
{
	if (bTutorialActive)
	{
		OnTutorialSkipped.Broadcast();
		StopTutorial();
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Tutorial skipped"));
	}
}

void UForge2TutorialManager::CompleteTutorialStep(FName StepId)
{
	if (!bTutorialActive || !RegisteredSequences.Contains(CurrentSequenceId))
	{
		return;
	}

	const FForge2TutorialSequence& Sequence = RegisteredSequences[CurrentSequenceId];
	if (CurrentStepIndex < Sequence.Steps.Num() &&
		Sequence.Steps[CurrentStepIndex].StepId == StepId)
	{
		AdvanceTutorial();
	}
}

bool UForge2TutorialManager::IsTutorialActive() const
{
	return bTutorialActive;
}

FForge2TutorialStep UForge2TutorialManager::GetCurrentStep() const
{
	if (bTutorialActive && RegisteredSequences.Contains(CurrentSequenceId))
	{
		const FForge2TutorialSequence& Sequence = RegisteredSequences[CurrentSequenceId];
		if (CurrentStepIndex >= 0 && CurrentStepIndex < Sequence.Steps.Num())
		{
			return Sequence.Steps[CurrentStepIndex];
		}
	}
	return FForge2TutorialStep();
}

int32 UForge2TutorialManager::GetCurrentStepIndex() const
{
	return CurrentStepIndex;
}

int32 UForge2TutorialManager::GetTotalSteps() const
{
	if (RegisteredSequences.Contains(CurrentSequenceId))
	{
		return RegisteredSequences[CurrentSequenceId].Steps.Num();
	}
	return 0;
}

bool UForge2TutorialManager::IsSequenceCompleted(FName SequenceId) const
{
	return CompletedSequences.Contains(SequenceId);
}

void UForge2TutorialManager::MarkSequenceCompleted(FName SequenceId)
{
	CompletedSequences.Add(SequenceId);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Marked sequence %s as completed"), *SequenceId.ToString());
}

void UForge2TutorialManager::ResetTutorialProgress()
{
	CompletedSequences.Empty();
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Tutorial: Progress reset"));
}

void UForge2TutorialManager::SetTutorialsEnabled(bool bEnabled)
{
	bTutorialsEnabled = bEnabled;
	if (!bEnabled && bTutorialActive)
	{
		StopTutorial();
	}
}

bool UForge2TutorialManager::AreTutorialsEnabled() const
{
	return bTutorialsEnabled;
}

// ============================================================================
// UForge2UIUtilities Implementation
// ============================================================================

FAnchors UForge2UIUtilities::GetAnchorsForPreset(EForge2WidgetAnchor Anchor)
{
	switch (Anchor)
	{
	case EForge2WidgetAnchor::TopLeft:
		return FAnchors(0.0f, 0.0f, 0.0f, 0.0f);
	case EForge2WidgetAnchor::TopCenter:
		return FAnchors(0.5f, 0.0f, 0.5f, 0.0f);
	case EForge2WidgetAnchor::TopRight:
		return FAnchors(1.0f, 0.0f, 1.0f, 0.0f);
	case EForge2WidgetAnchor::CenterLeft:
		return FAnchors(0.0f, 0.5f, 0.0f, 0.5f);
	case EForge2WidgetAnchor::Center:
		return FAnchors(0.5f, 0.5f, 0.5f, 0.5f);
	case EForge2WidgetAnchor::CenterRight:
		return FAnchors(1.0f, 0.5f, 1.0f, 0.5f);
	case EForge2WidgetAnchor::BottomLeft:
		return FAnchors(0.0f, 1.0f, 0.0f, 1.0f);
	case EForge2WidgetAnchor::BottomCenter:
		return FAnchors(0.5f, 1.0f, 0.5f, 1.0f);
	case EForge2WidgetAnchor::BottomRight:
		return FAnchors(1.0f, 1.0f, 1.0f, 1.0f);
	case EForge2WidgetAnchor::StretchHorizontal:
		return FAnchors(0.0f, 0.5f, 1.0f, 0.5f);
	case EForge2WidgetAnchor::StretchVertical:
		return FAnchors(0.5f, 0.0f, 0.5f, 1.0f);
	case EForge2WidgetAnchor::StretchFull:
		return FAnchors(0.0f, 0.0f, 1.0f, 1.0f);
	default:
		return FAnchors(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

FVector2D UForge2UIUtilities::GetAlignmentForPreset(EForge2WidgetAnchor Anchor)
{
	switch (Anchor)
	{
	case EForge2WidgetAnchor::TopLeft:
		return FVector2D(0.0f, 0.0f);
	case EForge2WidgetAnchor::TopCenter:
		return FVector2D(0.5f, 0.0f);
	case EForge2WidgetAnchor::TopRight:
		return FVector2D(1.0f, 0.0f);
	case EForge2WidgetAnchor::CenterLeft:
		return FVector2D(0.0f, 0.5f);
	case EForge2WidgetAnchor::Center:
		return FVector2D(0.5f, 0.5f);
	case EForge2WidgetAnchor::CenterRight:
		return FVector2D(1.0f, 0.5f);
	case EForge2WidgetAnchor::BottomLeft:
		return FVector2D(0.0f, 1.0f);
	case EForge2WidgetAnchor::BottomCenter:
		return FVector2D(0.5f, 1.0f);
	case EForge2WidgetAnchor::BottomRight:
		return FVector2D(1.0f, 1.0f);
	default:
		return FVector2D(0.0f, 0.0f);
	}
}

FLinearColor UForge2UIUtilities::GetNotificationColor(EForge2NotificationType Type)
{
	switch (Type)
	{
	case EForge2NotificationType::Info:
		return FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
	case EForge2NotificationType::Success:
		return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
	case EForge2NotificationType::Warning:
		return FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
	case EForge2NotificationType::Error:
		return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
	case EForge2NotificationType::Achievement:
		return FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);
	case EForge2NotificationType::Quest:
		return FLinearColor(0.8f, 0.6f, 0.0f, 1.0f);
	case EForge2NotificationType::LevelUp:
		return FLinearColor(0.6f, 0.4f, 1.0f, 1.0f);
	case EForge2NotificationType::ItemPickup:
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
	case EForge2NotificationType::System:
		return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
	default:
		return FLinearColor::White;
	}
}

FLinearColor UForge2UIUtilities::GetDamageNumberColor(FName DamageType, bool bIsCritical, bool bIsHealing)
{
	if (bIsHealing)
	{
		return bIsCritical ? FLinearColor(0.0f, 1.0f, 0.5f, 1.0f) : FLinearColor(0.0f, 0.8f, 0.0f, 1.0f);
	}

	if (bIsCritical)
	{
		return FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
	}

	// Color by damage type
	if (DamageType == FName("Fire"))
	{
		return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
	}
	else if (DamageType == FName("Ice"))
	{
		return FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
	}
	else if (DamageType == FName("Lightning"))
	{
		return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (DamageType == FName("Poison"))
	{
		return FLinearColor(0.0f, 0.8f, 0.2f, 1.0f);
	}
	else if (DamageType == FName("Holy"))
	{
		return FLinearColor(1.0f, 1.0f, 0.8f, 1.0f);
	}
	else if (DamageType == FName("Dark"))
	{
		return FLinearColor(0.5f, 0.0f, 0.5f, 1.0f);
	}

	return FLinearColor::White;
}

UUserWidget* UForge2UIUtilities::CreateWidgetFromClass(APlayerController* Owner, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!Owner || !WidgetClass)
	{
		return nullptr;
	}

	return CreateWidget<UUserWidget>(Owner, WidgetClass);
}

void UForge2UIUtilities::SetWidgetOpacity(UUserWidget* Widget, float Opacity)
{
	if (Widget)
	{
		Widget->SetRenderOpacity(FMath::Clamp(Opacity, 0.0f, 1.0f));
	}
}

FVector2D UForge2UIUtilities::GetScreenCenter(APlayerController* Controller)
{
	if (!Controller)
	{
		return FVector2D::ZeroVector;
	}

	int32 SizeX, SizeY;
	Controller->GetViewportSize(SizeX, SizeY);
	return FVector2D(SizeX * 0.5f, SizeY * 0.5f);
}

FVector2D UForge2UIUtilities::WorldToScreen(APlayerController* Controller, FVector WorldLocation)
{
	if (!Controller)
	{
		return FVector2D::ZeroVector;
	}

	FVector2D ScreenPos;
	Controller->ProjectWorldLocationToScreen(WorldLocation, ScreenPos);
	return ScreenPos;
}

FForge2UITheme UForge2UIUtilities::GetDefaultTheme()
{
	FForge2UITheme Theme;
	Theme.ThemeName = FName("Default");
	Theme.PrimaryColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);
	Theme.SecondaryColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
	Theme.AccentColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
	Theme.BackgroundColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.9f);
	Theme.TextColor = FLinearColor::White;
	Theme.ErrorColor = FLinearColor::Red;
	Theme.SuccessColor = FLinearColor::Green;
	Theme.WarningColor = FLinearColor::Yellow;
	Theme.ElementPadding = 10.0f;
	Theme.ElementSpacing = 5.0f;
	Theme.BorderRadius = 4.0f;
	Theme.BorderWidth = 1.0f;
	return Theme;
}

FForge2UITheme UForge2UIUtilities::GetDarkTheme()
{
	FForge2UITheme Theme = GetDefaultTheme();
	Theme.ThemeName = FName("Dark");
	Theme.BackgroundColor = FLinearColor(0.02f, 0.02f, 0.02f, 0.95f);
	Theme.PrimaryColor = FLinearColor(0.3f, 0.3f, 0.9f, 1.0f);
	return Theme;
}

FForge2UITheme UForge2UIUtilities::GetLightTheme()
{
	FForge2UITheme Theme = GetDefaultTheme();
	Theme.ThemeName = FName("Light");
	Theme.BackgroundColor = FLinearColor(0.95f, 0.95f, 0.95f, 0.9f);
	Theme.TextColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
	Theme.PrimaryColor = FLinearColor(0.0f, 0.4f, 0.8f, 1.0f);
	return Theme;
}

FForge2UITheme UForge2UIUtilities::GetFantasyTheme()
{
	FForge2UITheme Theme = GetDefaultTheme();
	Theme.ThemeName = FName("Fantasy");
	Theme.PrimaryColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
	Theme.AccentColor = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f);
	Theme.BackgroundColor = FLinearColor(0.1f, 0.08f, 0.05f, 0.9f);
	Theme.BorderRadius = 2.0f;
	Theme.BorderWidth = 2.0f;
	return Theme;
}

FForge2UITheme UForge2UIUtilities::GetSciFiTheme()
{
	FForge2UITheme Theme = GetDefaultTheme();
	Theme.ThemeName = FName("SciFi");
	Theme.PrimaryColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
	Theme.AccentColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);
	Theme.BackgroundColor = FLinearColor(0.0f, 0.02f, 0.05f, 0.9f);
	Theme.BorderRadius = 0.0f;
	Theme.BorderWidth = 1.0f;
	return Theme;
}

TArray<FForge2HUDElementConfig> UForge2UIUtilities::GetRPGHUDPreset()
{
	TArray<FForge2HUDElementConfig> Elements;

	// Health bar
	FForge2HUDElementConfig HealthBar;
	HealthBar.ElementId = FName("HealthBar");
	HealthBar.ElementType = EForge2HUDElementType::HealthBar;
	HealthBar.Anchor = EForge2WidgetAnchor::TopLeft;
	HealthBar.Position = FVector2D(20.0f, 20.0f);
	HealthBar.Size = FVector2D(300.0f, 30.0f);
	Elements.Add(HealthBar);

	// Mana bar
	FForge2HUDElementConfig ManaBar;
	ManaBar.ElementId = FName("ManaBar");
	ManaBar.ElementType = EForge2HUDElementType::ManaBar;
	ManaBar.Anchor = EForge2WidgetAnchor::TopLeft;
	ManaBar.Position = FVector2D(20.0f, 55.0f);
	ManaBar.Size = FVector2D(250.0f, 20.0f);
	Elements.Add(ManaBar);

	// Minimap
	FForge2HUDElementConfig Minimap;
	Minimap.ElementId = FName("Minimap");
	Minimap.ElementType = EForge2HUDElementType::Minimap;
	Minimap.Anchor = EForge2WidgetAnchor::TopRight;
	Minimap.Position = FVector2D(-170.0f, 20.0f);
	Minimap.Size = FVector2D(150.0f, 150.0f);
	Elements.Add(Minimap);

	// Hotbar
	FForge2HUDElementConfig Hotbar;
	Hotbar.ElementId = FName("Hotbar");
	Hotbar.ElementType = EForge2HUDElementType::Hotbar;
	Hotbar.Anchor = EForge2WidgetAnchor::BottomCenter;
	Hotbar.Position = FVector2D(0.0f, -80.0f);
	Hotbar.Size = FVector2D(500.0f, 60.0f);
	Elements.Add(Hotbar);

	// Quest tracker
	FForge2HUDElementConfig QuestTracker;
	QuestTracker.ElementId = FName("QuestTracker");
	QuestTracker.ElementType = EForge2HUDElementType::QuestTracker;
	QuestTracker.Anchor = EForge2WidgetAnchor::TopRight;
	QuestTracker.Position = FVector2D(-20.0f, 200.0f);
	QuestTracker.Size = FVector2D(300.0f, 200.0f);
	Elements.Add(QuestTracker);

	return Elements;
}

TArray<FForge2HUDElementConfig> UForge2UIUtilities::GetFPSHUDPreset()
{
	TArray<FForge2HUDElementConfig> Elements;

	// Crosshair
	FForge2HUDElementConfig Crosshair;
	Crosshair.ElementId = FName("Crosshair");
	Crosshair.ElementType = EForge2HUDElementType::Crosshair;
	Crosshair.Anchor = EForge2WidgetAnchor::Center;
	Crosshair.Size = FVector2D(64.0f, 64.0f);
	Elements.Add(Crosshair);

	// Health
	FForge2HUDElementConfig Health;
	Health.ElementId = FName("Health");
	Health.ElementType = EForge2HUDElementType::HealthBar;
	Health.Anchor = EForge2WidgetAnchor::BottomLeft;
	Health.Position = FVector2D(20.0f, -80.0f);
	Health.Size = FVector2D(200.0f, 40.0f);
	Elements.Add(Health);

	// Ammo
	FForge2HUDElementConfig Ammo;
	Ammo.ElementId = FName("Ammo");
	Ammo.ElementType = EForge2HUDElementType::AmmoCounter;
	Ammo.Anchor = EForge2WidgetAnchor::BottomRight;
	Ammo.Position = FVector2D(-20.0f, -80.0f);
	Ammo.Size = FVector2D(150.0f, 60.0f);
	Elements.Add(Ammo);

	// Compass
	FForge2HUDElementConfig Compass;
	Compass.ElementId = FName("Compass");
	Compass.ElementType = EForge2HUDElementType::Compass;
	Compass.Anchor = EForge2WidgetAnchor::TopCenter;
	Compass.Position = FVector2D(0.0f, 20.0f);
	Compass.Size = FVector2D(400.0f, 40.0f);
	Elements.Add(Compass);

	return Elements;
}

TArray<FForge2HUDElementConfig> UForge2UIUtilities::GetMinimalHUDPreset()
{
	TArray<FForge2HUDElementConfig> Elements;

	// Health only
	FForge2HUDElementConfig Health;
	Health.ElementId = FName("Health");
	Health.ElementType = EForge2HUDElementType::HealthBar;
	Health.Anchor = EForge2WidgetAnchor::BottomLeft;
	Health.Position = FVector2D(20.0f, -40.0f);
	Health.Size = FVector2D(100.0f, 10.0f);
	Health.Opacity = 0.6f;
	Elements.Add(Health);

	// Interaction prompt
	FForge2HUDElementConfig Interact;
	Interact.ElementId = FName("InteractionPrompt");
	Interact.ElementType = EForge2HUDElementType::InteractionPrompt;
	Interact.Anchor = EForge2WidgetAnchor::BottomCenter;
	Interact.Position = FVector2D(0.0f, -100.0f);
	Interact.Size = FVector2D(200.0f, 40.0f);
	Elements.Add(Interact);

	return Elements;
}

FForge2LoadingScreenConfig UForge2UIUtilities::GetDefaultLoadingScreen()
{
	FForge2LoadingScreenConfig Config;
	Config.bShowProgressBar = true;
	Config.bShowSpinner = true;
	Config.LoadingText = FText::FromString(TEXT("Loading..."));
	Config.MinimumDisplayTime = 2.0f;
	Config.TipRotationInterval = 5.0f;

	Config.Tips.Add(FText::FromString(TEXT("Tip: Press Tab to open your inventory.")));
	Config.Tips.Add(FText::FromString(TEXT("Tip: Remember to save your progress regularly.")));
	Config.Tips.Add(FText::FromString(TEXT("Tip: Explore the world to find hidden treasures.")));
	Config.Tips.Add(FText::FromString(TEXT("Tip: Talk to NPCs for quests and information.")));

	return Config;
}
