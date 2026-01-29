// Copyright 2025 FORGE2 Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2BuildPlatformSystem.generated.h"

// ============================================================================
// BUILD SYSTEM ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2BuildConfiguration : uint8
{
	Debug UMETA(DisplayName = "Debug"),
	DebugGame UMETA(DisplayName = "Debug Game"),
	Development UMETA(DisplayName = "Development"),
	Shipping UMETA(DisplayName = "Shipping"),
	Test UMETA(DisplayName = "Test")
};

UENUM(BlueprintType)
enum class EForge2TargetPlatform : uint8
{
	Windows UMETA(DisplayName = "Windows"),
	Linux UMETA(DisplayName = "Linux"),
	Mac UMETA(DisplayName = "macOS"),
	Android UMETA(DisplayName = "Android"),
	IOS UMETA(DisplayName = "iOS"),
	PS5 UMETA(DisplayName = "PlayStation 5"),
	XboxSeriesX UMETA(DisplayName = "Xbox Series X"),
	Switch UMETA(DisplayName = "Nintendo Switch")
};

UENUM(BlueprintType)
enum class EForge2CompressionMethod : uint8
{
	None,
	Zlib,
	Gzip,
	LZ4,
	Oodle
};

UENUM(BlueprintType)
enum class EForge2BuildStatus : uint8
{
	Idle,
	Preparing,
	Cooking,
	Packaging,
	Staging,
	Complete,
	Failed,
	Cancelled
};

UENUM(BlueprintType)
enum class EForge2CookingProfile : uint8
{
	Default,
	Fast,
	Full,
	Iterative,
	SharedDataBuild
};

UENUM(BlueprintType)
enum class EForge2ShaderPlatform : uint8
{
	PCD3D_SM5,
	PCD3D_SM6,
	VULKAN_SM5,
	VULKAN_SM6,
	METAL_SM5,
	OPENGL_ES3_1,
	VULKAN_ES3_1
};

// ============================================================================
// PLATFORM ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2InputMode : uint8
{
	KeyboardMouse,
	Gamepad,
	Touch,
	Mixed
};

UENUM(BlueprintType)
enum class EForge2QualityLevel : uint8
{
	Low,
	Medium,
	High,
	Epic,
	Cinematic,
	Custom
};

UENUM(BlueprintType)
enum class EForge2DisplayMode : uint8
{
	Windowed,
	WindowedFullscreen,
	Fullscreen
};

UENUM(BlueprintType)
enum class EForge2AntiAliasingMethod : uint8
{
	None,
	FXAA,
	TAA,
	MSAA,
	TSR
};

UENUM(BlueprintType)
enum class EForge2GlobalIlluminationMethod : uint8
{
	None,
	ScreenSpace,
	Lumen,
	HardwareRayTracing
};

// ============================================================================
// BUILD STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	FName BuildName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	EForge2BuildConfiguration Configuration = EForge2BuildConfiguration::Shipping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	EForge2TargetPlatform TargetPlatform = EForge2TargetPlatform::Windows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	FString OutputDirectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	FString ProjectPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	bool bBuildEditorContent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	bool bIterativeCooking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	bool bCompressAssets = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	EForge2CompressionMethod CompressionMethod = EForge2CompressionMethod::Oodle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	TArray<FName> MapsToInclude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	TArray<FName> MapsToExclude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	TArray<FString> AdditionalCookArgs;

	FForge2BuildSettings()
		: BuildName(TEXT("DefaultBuild"))
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PakFileSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	FName ChunkName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	int32 ChunkId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	TArray<FString> IncludedPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	TArray<FString> ExcludedPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	bool bEncrypt = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	bool bSign = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	bool bCompress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	EForge2CompressionMethod CompressionMethod = EForge2CompressionMethod::Oodle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak")
	int32 PriorityOrder = 0;

	FForge2PakFileSettings()
		: ChunkName(TEXT("Chunk0"))
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DLCSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	FName DLCName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	FString Version;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	TArray<FString> ContentPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	TArray<FName> RequiredDLC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	bool bMountAtStartup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLC")
	int32 MountOrder = 100;

	FForge2DLCSettings()
		: DLCName(TEXT("DLC_01"))
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ShaderCompilationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	TArray<EForge2ShaderPlatform> TargetShaderPlatforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	bool bShaderDebugInfo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	bool bCompileShaderOffline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	int32 ShaderCompilerWorkers = 0; // 0 = auto

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	bool bShareMaterialShaderCode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	bool bSharedMaterialNativeLibraries = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FName ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FForge2BuildSettings BuildSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	TArray<FForge2PakFileSettings> PakSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FForge2ShaderCompilationSettings ShaderSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	TMap<FName, FString> CustomSettings;

	FForge2BuildProfile()
		: ProfileName(TEXT("DefaultProfile"))
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BuildProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	EForge2BuildStatus Status = EForge2BuildStatus::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float OverallProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FString CurrentStep;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 AssetsCookedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 TotalAssetsToCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	double ElapsedSeconds = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FString> Warnings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FString> Errors;
};

// ============================================================================
// PLATFORM STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlatformSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	EForge2TargetPlatform Platform = EForge2TargetPlatform::Windows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	EForge2InputMode DefaultInputMode = EForge2InputMode::KeyboardMouse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	EForge2QualityLevel DefaultQuality = EForge2QualityLevel::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	FIntPoint DefaultResolution = FIntPoint(1920, 1080);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	int32 TargetFrameRate = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	bool bVSyncEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	bool bDynamicResolution = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	float ResolutionScale = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	bool bHDRSupported = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	bool bRayTracingSupported = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	int32 TexturePoolSizeMB = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	TMap<FName, FString> PlatformSpecificSettings;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ScalabilitySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	EForge2QualityLevel OverallQuality = EForge2QualityLevel::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	int32 ViewDistanceQuality = 3; // 0-4, Cinematic = 4

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	int32 ShadowQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	int32 PostProcessQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	int32 TextureQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	int32 EffectsQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	int32 FoliageQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	int32 ShadingQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	EForge2AntiAliasingMethod AntiAliasing = EForge2AntiAliasingMethod::TAA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	EForge2GlobalIlluminationMethod GlobalIllumination = EForge2GlobalIlluminationMethod::Lumen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scalability")
	float ScreenPercentage = 100.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InputSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float GamepadSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bInvertLookY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bInvertLookX = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float GamepadDeadzone = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bEnableVibration = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bEnableTriggerFeedback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bEnableGyroAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float TouchSensitivity = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AudioSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MusicVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float SFXVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float DialogueVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float AmbientVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bEnableSubtitles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	FName AudioLanguage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bEnable3DAudio = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	int32 MaxAudioChannels = 64;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CrossPlatformSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrossPlatform")
	bool bEnableCrossPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrossPlatform")
	bool bEnableCrossSave = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrossPlatform")
	bool bEnableCrossProgression = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrossPlatform")
	TArray<EForge2TargetPlatform> SupportedPlatforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrossPlatform")
	FString CloudSaveProvider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrossPlatform")
	int32 SaveDataVersion = 1;
};

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FForge2OnBuildProgressUpdate, const FForge2BuildProgress&, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FForge2OnBuildComplete, FName, BuildName, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FForge2OnSettingsChanged, FName, SettingName, FString, NewValue);

// ============================================================================
// BUILD MANAGER
// ============================================================================

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2BuildManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2BuildManager();

	// Build Configuration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	void ConfigureBuildSettings(const FForge2BuildSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	void ConfigurePakSettings(const TArray<FForge2PakFileSettings>& PakSettings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	void ConfigureShaderSettings(const FForge2ShaderCompilationSettings& ShaderSettings);

	// Build Profiles
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	void SaveBuildProfile(const FForge2BuildProfile& Profile);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	FForge2BuildProfile LoadBuildProfile(FName ProfileName) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	bool DeleteBuildProfile(FName ProfileName);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build")
	TArray<FName> GetAvailableProfiles() const;

	// Build Execution
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	bool StartBuild(FName ProfileName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	bool StartBuildWithSettings(const FForge2BuildSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	void CancelBuild();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build")
	FForge2BuildProgress GetBuildProgress() const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build")
	bool IsBuildInProgress() const;

	// DLC
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	bool CreateDLCPackage(const FForge2DLCSettings& DLCSettings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	bool MountDLC(FName DLCName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	bool UnmountDLC(FName DLCName);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build")
	TArray<FName> GetMountedDLC() const;

	// Validation
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	TArray<FString> ValidateAssets();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	TArray<FString> ValidateBlueprintErrors();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Build")
	TArray<FString> CheckNamingConventions();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Build|Events")
	FForge2OnBuildProgressUpdate OnBuildProgressUpdate;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Build|Events")
	FForge2OnBuildComplete OnBuildComplete;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	FForge2BuildSettings CurrentBuildSettings;

	UPROPERTY()
	TArray<FForge2PakFileSettings> CurrentPakSettings;

	UPROPERTY()
	FForge2ShaderCompilationSettings CurrentShaderSettings;

	UPROPERTY()
	TMap<FName, FForge2BuildProfile> SavedProfiles;

	UPROPERTY()
	FForge2BuildProgress CurrentProgress;

	UPROPERTY()
	TArray<FName> MountedDLCList;

	bool bBuildInProgress = false;
	double BuildStartTime = 0.0;

	void UpdateBuildProgress();
};

// ============================================================================
// PLATFORM MANAGER
// ============================================================================

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2PlatformManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2PlatformManager();

	// Platform Configuration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void ConfigurePlatformSettings(const FForge2PlatformSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	FForge2PlatformSettings GetCurrentPlatformSettings() const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	EForge2TargetPlatform GetCurrentPlatform() const;

	// Scalability
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void ApplyScalabilitySettings(const FForge2ScalabilitySettings& Settings);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	FForge2ScalabilitySettings GetScalabilitySettings() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void SetQualityPreset(EForge2QualityLevel Quality);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void AutoDetectQualitySettings();

	// Input Configuration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void ApplyInputSettings(const FForge2InputSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	FForge2InputSettings GetInputSettings() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void SetInputMode(EForge2InputMode Mode);

	// Audio Configuration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void ApplyAudioSettings(const FForge2AudioSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	FForge2AudioSettings GetAudioSettings() const;

	// Display Configuration
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void SetDisplayMode(EForge2DisplayMode Mode);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void SetResolution(FIntPoint Resolution);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void SetFrameRateLimit(int32 FrameRate);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void SetVSyncEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	TArray<FIntPoint> GetSupportedResolutions() const;

	// Cross-Platform
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void ConfigureCrossPlatform(const FForge2CrossPlatformSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	FForge2CrossPlatformSettings GetCrossPlatformSettings() const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	bool IsCrossPlayEnabled() const;

	// Platform Features
	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	bool SupportsRayTracing() const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	bool SupportsHDR() const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	bool SupportsNanite() const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	bool SupportsLumen() const;

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform")
	int32 GetAvailableVRAM() const;

	// Settings Persistence
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void SaveSettingsToConfig();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void LoadSettingsFromConfig();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Platform")
	void ResetToDefaults();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Platform|Events")
	FForge2OnSettingsChanged OnSettingsChanged;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	FForge2PlatformSettings CurrentPlatformSettings;

	UPROPERTY()
	FForge2ScalabilitySettings CurrentScalability;

	UPROPERTY()
	FForge2InputSettings CurrentInputSettings;

	UPROPERTY()
	FForge2AudioSettings CurrentAudioSettings;

	UPROPERTY()
	FForge2CrossPlatformSettings CrossPlatformSettings;

	void DetectPlatformCapabilities();
	void ApplyPlatformOverrides();
};

// ============================================================================
// BUILD & PLATFORM UTILITIES
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2BuildPlatformUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Build Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Build|Presets")
	static FForge2BuildSettings GetShippingBuildPreset(FName BuildName, EForge2TargetPlatform Platform);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build|Presets")
	static FForge2BuildSettings GetDevelopmentBuildPreset(FName BuildName, EForge2TargetPlatform Platform);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build|Presets")
	static FForge2PakFileSettings GetDefaultPakSettings(FName ChunkName, int32 ChunkId);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build|Presets")
	static FForge2PakFileSettings GetEncryptedPakSettings(FName ChunkName, int32 ChunkId);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build|Presets")
	static FForge2DLCSettings GetDLCPreset(FName DLCName, const TArray<FString>& ContentPaths);

	// Platform Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2PlatformSettings GetWindowsPlatformPreset();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2PlatformSettings GetConsolePlatformPreset(EForge2TargetPlatform Console);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2PlatformSettings GetMobilePlatformPreset(EForge2TargetPlatform MobilePlatform);

	// Scalability Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2ScalabilitySettings GetLowQualityPreset();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2ScalabilitySettings GetMediumQualityPreset();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2ScalabilitySettings GetHighQualityPreset();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2ScalabilitySettings GetEpicQualityPreset();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2ScalabilitySettings GetCinematicQualityPreset();

	// Input Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2InputSettings GetDefaultKeyboardMouseSettings();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2InputSettings GetDefaultGamepadSettings();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2InputSettings GetDefaultTouchSettings();

	// Audio Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2AudioSettings GetDefaultAudioSettings();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2AudioSettings GetHeadphoneAudioSettings();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Presets")
	static FForge2AudioSettings GetSpeakerAudioSettings();

	// Utility Functions
	UFUNCTION(BlueprintPure, Category = "FORGE2|Build|Utility")
	static FString GetPlatformName(EForge2TargetPlatform Platform);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build|Utility")
	static FString GetConfigurationName(EForge2BuildConfiguration Config);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Build|Utility")
	static FString GetQualityLevelName(EForge2QualityLevel Quality);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Utility")
	static bool IsPlatformSupported(EForge2TargetPlatform Platform);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Platform|Utility")
	static FIntPoint GetRecommendedResolution(EForge2TargetPlatform Platform);
};
