// Copyright 2025 FORGE2 Project. All Rights Reserved.

#include "Forge2BuildPlatformSystem.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformMisc.h"
#include "RHI.h"
#include "GenericPlatform/GenericApplication.h"
#include "GameFramework/GameUserSettings.h"

// ============================================================================
// BUILD MANAGER IMPLEMENTATION
// ============================================================================

UForge2BuildManager::UForge2BuildManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // Update progress every second
}

void UForge2BuildManager::BeginPlay()
{
	Super::BeginPlay();
}

void UForge2BuildManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bBuildInProgress)
	{
		UpdateBuildProgress();
	}
}

void UForge2BuildManager::ConfigureBuildSettings(const FForge2BuildSettings& Settings)
{
	CurrentBuildSettings = Settings;
}

void UForge2BuildManager::ConfigurePakSettings(const TArray<FForge2PakFileSettings>& PakSettings)
{
	CurrentPakSettings = PakSettings;
}

void UForge2BuildManager::ConfigureShaderSettings(const FForge2ShaderCompilationSettings& ShaderSettings)
{
	CurrentShaderSettings = ShaderSettings;
}

void UForge2BuildManager::SaveBuildProfile(const FForge2BuildProfile& Profile)
{
	if (!Profile.ProfileName.IsNone())
	{
		SavedProfiles.Add(Profile.ProfileName, Profile);
	}
}

FForge2BuildProfile UForge2BuildManager::LoadBuildProfile(FName ProfileName) const
{
	const FForge2BuildProfile* Profile = SavedProfiles.Find(ProfileName);
	return Profile ? *Profile : FForge2BuildProfile();
}

bool UForge2BuildManager::DeleteBuildProfile(FName ProfileName)
{
	return SavedProfiles.Remove(ProfileName) > 0;
}

TArray<FName> UForge2BuildManager::GetAvailableProfiles() const
{
	TArray<FName> ProfileNames;
	SavedProfiles.GetKeys(ProfileNames);
	return ProfileNames;
}

bool UForge2BuildManager::StartBuild(FName ProfileName)
{
	const FForge2BuildProfile* Profile = SavedProfiles.Find(ProfileName);
	if (!Profile)
	{
		return false;
	}

	return StartBuildWithSettings(Profile->BuildSettings);
}

bool UForge2BuildManager::StartBuildWithSettings(const FForge2BuildSettings& Settings)
{
	if (bBuildInProgress)
	{
		return false;
	}

	CurrentBuildSettings = Settings;
	bBuildInProgress = true;
	BuildStartTime = FPlatformTime::Seconds();

	CurrentProgress = FForge2BuildProgress();
	CurrentProgress.Status = EForge2BuildStatus::Preparing;
	CurrentProgress.CurrentStep = TEXT("Initializing build...");

	OnBuildProgressUpdate.Broadcast(CurrentProgress);

	// Note: Actual build execution requires UAT (Unreal Automation Tool)
	// This is a framework for triggering and monitoring builds
	// Real implementation would launch UAT process and monitor output

	return true;
}

void UForge2BuildManager::CancelBuild()
{
	if (bBuildInProgress)
	{
		bBuildInProgress = false;
		CurrentProgress.Status = EForge2BuildStatus::Cancelled;
		OnBuildProgressUpdate.Broadcast(CurrentProgress);
		OnBuildComplete.Broadcast(CurrentBuildSettings.BuildName, false);
	}
}

FForge2BuildProgress UForge2BuildManager::GetBuildProgress() const
{
	return CurrentProgress;
}

bool UForge2BuildManager::IsBuildInProgress() const
{
	return bBuildInProgress;
}

bool UForge2BuildManager::CreateDLCPackage(const FForge2DLCSettings& DLCSettings)
{
	if (DLCSettings.DLCName.IsNone())
	{
		return false;
	}

	// DLC package creation requires UAT with specific parameters
	// This is a placeholder for the framework
	return true;
}

bool UForge2BuildManager::MountDLC(FName DLCName)
{
	if (DLCName.IsNone() || MountedDLCList.Contains(DLCName))
	{
		return false;
	}

	// Mount the DLC pak file
	// FCoreDelegates::MountPak would be used here
	MountedDLCList.Add(DLCName);
	return true;
}

bool UForge2BuildManager::UnmountDLC(FName DLCName)
{
	if (!MountedDLCList.Contains(DLCName))
	{
		return false;
	}

	// Unmount the DLC pak file
	MountedDLCList.Remove(DLCName);
	return true;
}

TArray<FName> UForge2BuildManager::GetMountedDLC() const
{
	return MountedDLCList;
}

TArray<FString> UForge2BuildManager::ValidateAssets()
{
	TArray<FString> Issues;

	// Check for missing references, invalid assets, etc.
	// This would use the Asset Registry to scan for problems

	return Issues;
}

TArray<FString> UForge2BuildManager::ValidateBlueprintErrors()
{
	TArray<FString> Errors;

	// Scan all blueprints for compile errors
	// This would iterate through BP assets and check their status

	return Errors;
}

TArray<FString> UForge2BuildManager::CheckNamingConventions()
{
	TArray<FString> Violations;

	// Check asset naming against project conventions
	// Common patterns: BP_, M_, T_, SM_, SK_, etc.

	return Violations;
}

void UForge2BuildManager::UpdateBuildProgress()
{
	double CurrentTime = FPlatformTime::Seconds();
	CurrentProgress.ElapsedSeconds = CurrentTime - BuildStartTime;

	// Simulate progress updates (real implementation would monitor UAT output)
	// This is a placeholder that demonstrates the framework
}

// ============================================================================
// PLATFORM MANAGER IMPLEMENTATION
// ============================================================================

UForge2PlatformManager::UForge2PlatformManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2PlatformManager::BeginPlay()
{
	Super::BeginPlay();

	DetectPlatformCapabilities();
	LoadSettingsFromConfig();
}

void UForge2PlatformManager::ConfigurePlatformSettings(const FForge2PlatformSettings& Settings)
{
	CurrentPlatformSettings = Settings;
	ApplyPlatformOverrides();
	OnSettingsChanged.Broadcast(TEXT("Platform"), TEXT("Updated"));
}

FForge2PlatformSettings UForge2PlatformManager::GetCurrentPlatformSettings() const
{
	return CurrentPlatformSettings;
}

EForge2TargetPlatform UForge2PlatformManager::GetCurrentPlatform() const
{
#if PLATFORM_WINDOWS
	return EForge2TargetPlatform::Windows;
#elif PLATFORM_LINUX
	return EForge2TargetPlatform::Linux;
#elif PLATFORM_MAC
	return EForge2TargetPlatform::Mac;
#elif PLATFORM_ANDROID
	return EForge2TargetPlatform::Android;
#elif PLATFORM_IOS
	return EForge2TargetPlatform::IOS;
#else
	return EForge2TargetPlatform::Windows;
#endif
}

void UForge2PlatformManager::ApplyScalabilitySettings(const FForge2ScalabilitySettings& Settings)
{
	CurrentScalability = Settings;

	// Apply to Scalability system
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->SetViewDistanceQuality(Settings.ViewDistanceQuality);
		UserSettings->SetShadowQuality(Settings.ShadowQuality);
		UserSettings->SetPostProcessingQuality(Settings.PostProcessQuality);
		UserSettings->SetTextureQuality(Settings.TextureQuality);
		UserSettings->SetVisualEffectQuality(Settings.EffectsQuality);
		UserSettings->SetFoliageQuality(Settings.FoliageQuality);
		UserSettings->SetShadingQuality(Settings.ShadingQuality);

		// Apply anti-aliasing
		int32 AAMethod = 0;
		switch (Settings.AntiAliasing)
		{
		case EForge2AntiAliasingMethod::None: AAMethod = 0; break;
		case EForge2AntiAliasingMethod::FXAA: AAMethod = 1; break;
		case EForge2AntiAliasingMethod::TAA: AAMethod = 2; break;
		case EForge2AntiAliasingMethod::MSAA: AAMethod = 4; break;
		case EForge2AntiAliasingMethod::TSR: AAMethod = 5; break;
		}
		UserSettings->SetAntiAliasingQuality(AAMethod);

		UserSettings->ApplySettings(true);
	}

	OnSettingsChanged.Broadcast(TEXT("Scalability"), TEXT("Updated"));
}

FForge2ScalabilitySettings UForge2PlatformManager::GetScalabilitySettings() const
{
	return CurrentScalability;
}

void UForge2PlatformManager::SetQualityPreset(EForge2QualityLevel Quality)
{
	switch (Quality)
	{
	case EForge2QualityLevel::Low:
		ApplyScalabilitySettings(UForge2BuildPlatformUtilities::GetLowQualityPreset());
		break;
	case EForge2QualityLevel::Medium:
		ApplyScalabilitySettings(UForge2BuildPlatformUtilities::GetMediumQualityPreset());
		break;
	case EForge2QualityLevel::High:
		ApplyScalabilitySettings(UForge2BuildPlatformUtilities::GetHighQualityPreset());
		break;
	case EForge2QualityLevel::Epic:
		ApplyScalabilitySettings(UForge2BuildPlatformUtilities::GetEpicQualityPreset());
		break;
	case EForge2QualityLevel::Cinematic:
		ApplyScalabilitySettings(UForge2BuildPlatformUtilities::GetCinematicQualityPreset());
		break;
	default:
		break;
	}

	CurrentScalability.OverallQuality = Quality;
}

void UForge2PlatformManager::AutoDetectQualitySettings()
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->RunHardwareBenchmark();
		UserSettings->ApplyHardwareBenchmarkResults();
		UserSettings->ApplySettings(true);

		// Update our tracking
		CurrentScalability.ViewDistanceQuality = UserSettings->GetViewDistanceQuality();
		CurrentScalability.ShadowQuality = UserSettings->GetShadowQuality();
		CurrentScalability.PostProcessQuality = UserSettings->GetPostProcessingQuality();
		CurrentScalability.TextureQuality = UserSettings->GetTextureQuality();
		CurrentScalability.EffectsQuality = UserSettings->GetVisualEffectQuality();
		CurrentScalability.FoliageQuality = UserSettings->GetFoliageQuality();
		CurrentScalability.ShadingQuality = UserSettings->GetShadingQuality();
	}
}

void UForge2PlatformManager::ApplyInputSettings(const FForge2InputSettings& Settings)
{
	CurrentInputSettings = Settings;

	// Apply to input system
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		// Mouse settings would be applied through player controller
		// Gamepad settings through input settings
	}

	OnSettingsChanged.Broadcast(TEXT("Input"), TEXT("Updated"));
}

FForge2InputSettings UForge2PlatformManager::GetInputSettings() const
{
	return CurrentInputSettings;
}

void UForge2PlatformManager::SetInputMode(EForge2InputMode Mode)
{
	CurrentPlatformSettings.DefaultInputMode = Mode;

	switch (Mode)
	{
	case EForge2InputMode::KeyboardMouse:
		ApplyInputSettings(UForge2BuildPlatformUtilities::GetDefaultKeyboardMouseSettings());
		break;
	case EForge2InputMode::Gamepad:
		ApplyInputSettings(UForge2BuildPlatformUtilities::GetDefaultGamepadSettings());
		break;
	case EForge2InputMode::Touch:
		ApplyInputSettings(UForge2BuildPlatformUtilities::GetDefaultTouchSettings());
		break;
	default:
		break;
	}
}

void UForge2PlatformManager::ApplyAudioSettings(const FForge2AudioSettings& Settings)
{
	CurrentAudioSettings = Settings;

	// Apply to audio system through sound mix/sound class system
	// This would update UGameUserSettings audio volumes

	OnSettingsChanged.Broadcast(TEXT("Audio"), TEXT("Updated"));
}

FForge2AudioSettings UForge2PlatformManager::GetAudioSettings() const
{
	return CurrentAudioSettings;
}

void UForge2PlatformManager::SetDisplayMode(EForge2DisplayMode Mode)
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		EWindowMode::Type WindowMode;
		switch (Mode)
		{
		case EForge2DisplayMode::Windowed:
			WindowMode = EWindowMode::Windowed;
			break;
		case EForge2DisplayMode::WindowedFullscreen:
			WindowMode = EWindowMode::WindowedFullscreen;
			break;
		case EForge2DisplayMode::Fullscreen:
			WindowMode = EWindowMode::Fullscreen;
			break;
		default:
			WindowMode = EWindowMode::Windowed;
			break;
		}

		UserSettings->SetFullscreenMode(WindowMode);
		UserSettings->ApplySettings(true);
	}
}

void UForge2PlatformManager::SetResolution(FIntPoint Resolution)
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->SetScreenResolution(Resolution);
		UserSettings->ApplySettings(true);
	}

	CurrentPlatformSettings.DefaultResolution = Resolution;
}

void UForge2PlatformManager::SetFrameRateLimit(int32 FrameRate)
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->SetFrameRateLimit(static_cast<float>(FrameRate));
		UserSettings->ApplySettings(true);
	}

	CurrentPlatformSettings.TargetFrameRate = FrameRate;
}

void UForge2PlatformManager::SetVSyncEnabled(bool bEnabled)
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->SetVSyncEnabled(bEnabled);
		UserSettings->ApplySettings(true);
	}

	CurrentPlatformSettings.bVSyncEnabled = bEnabled;
}

TArray<FIntPoint> UForge2PlatformManager::GetSupportedResolutions() const
{
	TArray<FIntPoint> Resolutions;

	FScreenResolutionArray ResolutionArray;
	if (RHIGetAvailableResolutions(ResolutionArray, true))
	{
		for (const FScreenResolutionRHI& Resolution : ResolutionArray)
		{
			Resolutions.Add(FIntPoint(Resolution.Width, Resolution.Height));
		}
	}

	return Resolutions;
}

void UForge2PlatformManager::ConfigureCrossPlatform(const FForge2CrossPlatformSettings& Settings)
{
	CrossPlatformSettings = Settings;
	OnSettingsChanged.Broadcast(TEXT("CrossPlatform"), TEXT("Updated"));
}

FForge2CrossPlatformSettings UForge2PlatformManager::GetCrossPlatformSettings() const
{
	return CrossPlatformSettings;
}

bool UForge2PlatformManager::IsCrossPlayEnabled() const
{
	return CrossPlatformSettings.bEnableCrossPlay;
}

bool UForge2PlatformManager::SupportsRayTracing() const
{
	return GRHISupportsRayTracing;
}

bool UForge2PlatformManager::SupportsHDR() const
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		return UserSettings->IsHDREnabled();
	}
	return false;
}

bool UForge2PlatformManager::SupportsNanite() const
{
	// Nanite requires SM6 capable hardware
	return GRHISupportsRayTracing; // Simplified check
}

bool UForge2PlatformManager::SupportsLumen() const
{
	// Lumen has software and hardware paths
	return true; // Software Lumen is widely supported
}

int32 UForge2PlatformManager::GetAvailableVRAM() const
{
	// Get dedicated video memory in MB
	FTextureMemoryStats TextureMemStats;
	RHIGetTextureMemoryStats(TextureMemStats);
	return TextureMemStats.DedicatedVideoMemory / (1024 * 1024);
}

void UForge2PlatformManager::SaveSettingsToConfig()
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->SaveSettings();
	}
}

void UForge2PlatformManager::LoadSettingsFromConfig()
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->LoadSettings();

		// Sync our tracking with loaded settings
		CurrentScalability.ViewDistanceQuality = UserSettings->GetViewDistanceQuality();
		CurrentScalability.ShadowQuality = UserSettings->GetShadowQuality();
		CurrentScalability.PostProcessQuality = UserSettings->GetPostProcessingQuality();
		CurrentScalability.TextureQuality = UserSettings->GetTextureQuality();
		CurrentScalability.EffectsQuality = UserSettings->GetVisualEffectQuality();
	}
}

void UForge2PlatformManager::ResetToDefaults()
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->SetToDefaults();
		UserSettings->ApplySettings(true);
	}

	CurrentScalability = FForge2ScalabilitySettings();
	CurrentInputSettings = FForge2InputSettings();
	CurrentAudioSettings = FForge2AudioSettings();
}

void UForge2PlatformManager::DetectPlatformCapabilities()
{
	CurrentPlatformSettings.Platform = GetCurrentPlatform();
	CurrentPlatformSettings.bRayTracingSupported = SupportsRayTracing();
	CurrentPlatformSettings.bHDRSupported = SupportsHDR();
	CurrentPlatformSettings.TexturePoolSizeMB = GetAvailableVRAM();

	// Detect default input mode based on platform
#if PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC
	CurrentPlatformSettings.DefaultInputMode = EForge2InputMode::KeyboardMouse;
#elif PLATFORM_ANDROID || PLATFORM_IOS
	CurrentPlatformSettings.DefaultInputMode = EForge2InputMode::Touch;
#else
	CurrentPlatformSettings.DefaultInputMode = EForge2InputMode::Gamepad;
#endif
}

void UForge2PlatformManager::ApplyPlatformOverrides()
{
	// Apply platform-specific overrides
	for (const auto& Override : CurrentPlatformSettings.PlatformSpecificSettings)
	{
		// Would apply console commands or config settings
	}
}

// ============================================================================
// UTILITIES IMPLEMENTATION
// ============================================================================

FForge2BuildSettings UForge2BuildPlatformUtilities::GetShippingBuildPreset(FName BuildName, EForge2TargetPlatform Platform)
{
	FForge2BuildSettings Settings;
	Settings.BuildName = BuildName;
	Settings.Configuration = EForge2BuildConfiguration::Shipping;
	Settings.TargetPlatform = Platform;
	Settings.bCompressAssets = true;
	Settings.CompressionMethod = EForge2CompressionMethod::Oodle;
	Settings.bBuildEditorContent = false;
	return Settings;
}

FForge2BuildSettings UForge2BuildPlatformUtilities::GetDevelopmentBuildPreset(FName BuildName, EForge2TargetPlatform Platform)
{
	FForge2BuildSettings Settings;
	Settings.BuildName = BuildName;
	Settings.Configuration = EForge2BuildConfiguration::Development;
	Settings.TargetPlatform = Platform;
	Settings.bCompressAssets = false;
	Settings.bIterativeCooking = true;
	Settings.bBuildEditorContent = true;
	return Settings;
}

FForge2PakFileSettings UForge2BuildPlatformUtilities::GetDefaultPakSettings(FName ChunkName, int32 ChunkId)
{
	FForge2PakFileSettings Settings;
	Settings.ChunkName = ChunkName;
	Settings.ChunkId = ChunkId;
	Settings.bCompress = true;
	Settings.CompressionMethod = EForge2CompressionMethod::Oodle;
	Settings.bEncrypt = false;
	Settings.bSign = false;
	return Settings;
}

FForge2PakFileSettings UForge2BuildPlatformUtilities::GetEncryptedPakSettings(FName ChunkName, int32 ChunkId)
{
	FForge2PakFileSettings Settings = GetDefaultPakSettings(ChunkName, ChunkId);
	Settings.bEncrypt = true;
	Settings.bSign = true;
	return Settings;
}

FForge2DLCSettings UForge2BuildPlatformUtilities::GetDLCPreset(FName DLCName, const TArray<FString>& ContentPaths)
{
	FForge2DLCSettings Settings;
	Settings.DLCName = DLCName;
	Settings.ContentPaths = ContentPaths;
	Settings.bMountAtStartup = false;
	Settings.MountOrder = 100;
	return Settings;
}

FForge2PlatformSettings UForge2BuildPlatformUtilities::GetWindowsPlatformPreset()
{
	FForge2PlatformSettings Settings;
	Settings.Platform = EForge2TargetPlatform::Windows;
	Settings.DefaultInputMode = EForge2InputMode::KeyboardMouse;
	Settings.DefaultQuality = EForge2QualityLevel::High;
	Settings.DefaultResolution = FIntPoint(1920, 1080);
	Settings.TargetFrameRate = 60;
	Settings.bVSyncEnabled = true;
	Settings.bDynamicResolution = false;
	Settings.bHDRSupported = true;
	Settings.bRayTracingSupported = true;
	Settings.TexturePoolSizeMB = 2048;
	return Settings;
}

FForge2PlatformSettings UForge2BuildPlatformUtilities::GetConsolePlatformPreset(EForge2TargetPlatform Console)
{
	FForge2PlatformSettings Settings;
	Settings.Platform = Console;
	Settings.DefaultInputMode = EForge2InputMode::Gamepad;
	Settings.DefaultQuality = EForge2QualityLevel::High;
	Settings.bVSyncEnabled = true;

	switch (Console)
	{
	case EForge2TargetPlatform::PS5:
	case EForge2TargetPlatform::XboxSeriesX:
		Settings.DefaultResolution = FIntPoint(3840, 2160);
		Settings.TargetFrameRate = 60;
		Settings.bHDRSupported = true;
		Settings.bRayTracingSupported = true;
		Settings.TexturePoolSizeMB = 12288;
		break;
	case EForge2TargetPlatform::Switch:
		Settings.DefaultResolution = FIntPoint(1920, 1080);
		Settings.TargetFrameRate = 30;
		Settings.DefaultQuality = EForge2QualityLevel::Medium;
		Settings.bHDRSupported = false;
		Settings.bRayTracingSupported = false;
		Settings.TexturePoolSizeMB = 2048;
		break;
	default:
		break;
	}

	return Settings;
}

FForge2PlatformSettings UForge2BuildPlatformUtilities::GetMobilePlatformPreset(EForge2TargetPlatform MobilePlatform)
{
	FForge2PlatformSettings Settings;
	Settings.Platform = MobilePlatform;
	Settings.DefaultInputMode = EForge2InputMode::Touch;
	Settings.DefaultQuality = EForge2QualityLevel::Medium;
	Settings.DefaultResolution = FIntPoint(1920, 1080);
	Settings.TargetFrameRate = 30;
	Settings.bVSyncEnabled = true;
	Settings.bDynamicResolution = true;
	Settings.bHDRSupported = false;
	Settings.bRayTracingSupported = false;
	Settings.TexturePoolSizeMB = 512;
	return Settings;
}

FForge2ScalabilitySettings UForge2BuildPlatformUtilities::GetLowQualityPreset()
{
	FForge2ScalabilitySettings Settings;
	Settings.OverallQuality = EForge2QualityLevel::Low;
	Settings.ViewDistanceQuality = 0;
	Settings.ShadowQuality = 0;
	Settings.PostProcessQuality = 0;
	Settings.TextureQuality = 0;
	Settings.EffectsQuality = 0;
	Settings.FoliageQuality = 0;
	Settings.ShadingQuality = 0;
	Settings.AntiAliasing = EForge2AntiAliasingMethod::FXAA;
	Settings.GlobalIllumination = EForge2GlobalIlluminationMethod::None;
	Settings.ScreenPercentage = 75.0f;
	return Settings;
}

FForge2ScalabilitySettings UForge2BuildPlatformUtilities::GetMediumQualityPreset()
{
	FForge2ScalabilitySettings Settings;
	Settings.OverallQuality = EForge2QualityLevel::Medium;
	Settings.ViewDistanceQuality = 1;
	Settings.ShadowQuality = 1;
	Settings.PostProcessQuality = 1;
	Settings.TextureQuality = 2;
	Settings.EffectsQuality = 1;
	Settings.FoliageQuality = 1;
	Settings.ShadingQuality = 1;
	Settings.AntiAliasing = EForge2AntiAliasingMethod::TAA;
	Settings.GlobalIllumination = EForge2GlobalIlluminationMethod::ScreenSpace;
	Settings.ScreenPercentage = 100.0f;
	return Settings;
}

FForge2ScalabilitySettings UForge2BuildPlatformUtilities::GetHighQualityPreset()
{
	FForge2ScalabilitySettings Settings;
	Settings.OverallQuality = EForge2QualityLevel::High;
	Settings.ViewDistanceQuality = 2;
	Settings.ShadowQuality = 2;
	Settings.PostProcessQuality = 2;
	Settings.TextureQuality = 3;
	Settings.EffectsQuality = 2;
	Settings.FoliageQuality = 2;
	Settings.ShadingQuality = 2;
	Settings.AntiAliasing = EForge2AntiAliasingMethod::TAA;
	Settings.GlobalIllumination = EForge2GlobalIlluminationMethod::Lumen;
	Settings.ScreenPercentage = 100.0f;
	return Settings;
}

FForge2ScalabilitySettings UForge2BuildPlatformUtilities::GetEpicQualityPreset()
{
	FForge2ScalabilitySettings Settings;
	Settings.OverallQuality = EForge2QualityLevel::Epic;
	Settings.ViewDistanceQuality = 3;
	Settings.ShadowQuality = 3;
	Settings.PostProcessQuality = 3;
	Settings.TextureQuality = 3;
	Settings.EffectsQuality = 3;
	Settings.FoliageQuality = 3;
	Settings.ShadingQuality = 3;
	Settings.AntiAliasing = EForge2AntiAliasingMethod::TSR;
	Settings.GlobalIllumination = EForge2GlobalIlluminationMethod::Lumen;
	Settings.ScreenPercentage = 100.0f;
	return Settings;
}

FForge2ScalabilitySettings UForge2BuildPlatformUtilities::GetCinematicQualityPreset()
{
	FForge2ScalabilitySettings Settings;
	Settings.OverallQuality = EForge2QualityLevel::Cinematic;
	Settings.ViewDistanceQuality = 4;
	Settings.ShadowQuality = 4;
	Settings.PostProcessQuality = 4;
	Settings.TextureQuality = 4;
	Settings.EffectsQuality = 4;
	Settings.FoliageQuality = 4;
	Settings.ShadingQuality = 4;
	Settings.AntiAliasing = EForge2AntiAliasingMethod::TSR;
	Settings.GlobalIllumination = EForge2GlobalIlluminationMethod::HardwareRayTracing;
	Settings.ScreenPercentage = 100.0f;
	return Settings;
}

FForge2InputSettings UForge2BuildPlatformUtilities::GetDefaultKeyboardMouseSettings()
{
	FForge2InputSettings Settings;
	Settings.MouseSensitivity = 1.0f;
	Settings.bInvertLookY = false;
	Settings.bInvertLookX = false;
	return Settings;
}

FForge2InputSettings UForge2BuildPlatformUtilities::GetDefaultGamepadSettings()
{
	FForge2InputSettings Settings;
	Settings.GamepadSensitivity = 1.0f;
	Settings.GamepadDeadzone = 0.25f;
	Settings.bEnableVibration = true;
	Settings.bEnableTriggerFeedback = true;
	Settings.bInvertLookY = false;
	return Settings;
}

FForge2InputSettings UForge2BuildPlatformUtilities::GetDefaultTouchSettings()
{
	FForge2InputSettings Settings;
	Settings.TouchSensitivity = 1.0f;
	Settings.bInvertLookY = false;
	return Settings;
}

FForge2AudioSettings UForge2BuildPlatformUtilities::GetDefaultAudioSettings()
{
	FForge2AudioSettings Settings;
	Settings.MasterVolume = 1.0f;
	Settings.MusicVolume = 0.8f;
	Settings.SFXVolume = 1.0f;
	Settings.DialogueVolume = 1.0f;
	Settings.AmbientVolume = 0.7f;
	Settings.bEnableSubtitles = true;
	Settings.bEnable3DAudio = true;
	Settings.MaxAudioChannels = 64;
	return Settings;
}

FForge2AudioSettings UForge2BuildPlatformUtilities::GetHeadphoneAudioSettings()
{
	FForge2AudioSettings Settings = GetDefaultAudioSettings();
	Settings.bEnable3DAudio = true;
	Settings.MaxAudioChannels = 128;
	return Settings;
}

FForge2AudioSettings UForge2BuildPlatformUtilities::GetSpeakerAudioSettings()
{
	FForge2AudioSettings Settings = GetDefaultAudioSettings();
	Settings.bEnable3DAudio = true;
	Settings.MaxAudioChannels = 32;
	return Settings;
}

FString UForge2BuildPlatformUtilities::GetPlatformName(EForge2TargetPlatform Platform)
{
	switch (Platform)
	{
	case EForge2TargetPlatform::Windows: return TEXT("Windows");
	case EForge2TargetPlatform::Linux: return TEXT("Linux");
	case EForge2TargetPlatform::Mac: return TEXT("macOS");
	case EForge2TargetPlatform::Android: return TEXT("Android");
	case EForge2TargetPlatform::IOS: return TEXT("iOS");
	case EForge2TargetPlatform::PS5: return TEXT("PlayStation 5");
	case EForge2TargetPlatform::XboxSeriesX: return TEXT("Xbox Series X");
	case EForge2TargetPlatform::Switch: return TEXT("Nintendo Switch");
	default: return TEXT("Unknown");
	}
}

FString UForge2BuildPlatformUtilities::GetConfigurationName(EForge2BuildConfiguration Config)
{
	switch (Config)
	{
	case EForge2BuildConfiguration::Debug: return TEXT("Debug");
	case EForge2BuildConfiguration::DebugGame: return TEXT("DebugGame");
	case EForge2BuildConfiguration::Development: return TEXT("Development");
	case EForge2BuildConfiguration::Shipping: return TEXT("Shipping");
	case EForge2BuildConfiguration::Test: return TEXT("Test");
	default: return TEXT("Unknown");
	}
}

FString UForge2BuildPlatformUtilities::GetQualityLevelName(EForge2QualityLevel Quality)
{
	switch (Quality)
	{
	case EForge2QualityLevel::Low: return TEXT("Low");
	case EForge2QualityLevel::Medium: return TEXT("Medium");
	case EForge2QualityLevel::High: return TEXT("High");
	case EForge2QualityLevel::Epic: return TEXT("Epic");
	case EForge2QualityLevel::Cinematic: return TEXT("Cinematic");
	case EForge2QualityLevel::Custom: return TEXT("Custom");
	default: return TEXT("Unknown");
	}
}

bool UForge2BuildPlatformUtilities::IsPlatformSupported(EForge2TargetPlatform Platform)
{
	// Check if platform is supported for building
	switch (Platform)
	{
	case EForge2TargetPlatform::Windows:
	case EForge2TargetPlatform::Linux:
	case EForge2TargetPlatform::Mac:
	case EForge2TargetPlatform::Android:
	case EForge2TargetPlatform::IOS:
		return true;
	case EForge2TargetPlatform::PS5:
	case EForge2TargetPlatform::XboxSeriesX:
	case EForge2TargetPlatform::Switch:
		// Console platforms require additional SDK setup
		return false; // Would check for SDK presence
	default:
		return false;
	}
}

FIntPoint UForge2BuildPlatformUtilities::GetRecommendedResolution(EForge2TargetPlatform Platform)
{
	switch (Platform)
	{
	case EForge2TargetPlatform::Windows:
	case EForge2TargetPlatform::Linux:
	case EForge2TargetPlatform::Mac:
		return FIntPoint(1920, 1080);
	case EForge2TargetPlatform::PS5:
	case EForge2TargetPlatform::XboxSeriesX:
		return FIntPoint(3840, 2160);
	case EForge2TargetPlatform::Switch:
		return FIntPoint(1280, 720);
	case EForge2TargetPlatform::Android:
	case EForge2TargetPlatform::IOS:
		return FIntPoint(1920, 1080);
	default:
		return FIntPoint(1920, 1080);
	}
}
