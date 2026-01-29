// Forge2AudioSystem.h - Audio System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2AudioSystem.generated.h"

// Forward declarations
class USoundCue;
class USoundWave;
class USoundAttenuation;
class USoundConcurrency;
class USoundClass;
class USoundMix;
class UAudioComponent;

// Enums
UENUM(BlueprintType)
enum class EForge2AttenuationShape : uint8
{
    Sphere,
    Capsule,
    Box,
    Cone
};

UENUM(BlueprintType)
enum class EForge2AttenuationFalloff : uint8
{
    Linear,
    Logarithmic,
    NaturalSound,
    LogReverse,
    Custom
};

UENUM(BlueprintType)
enum class EForge2SpatializationMode : uint8
{
    Default,
    HRTF,
    Binaural,
    Stereo
};

UENUM(BlueprintType)
enum class EForge2SoundType : uint8
{
    SFX,
    Music,
    Dialogue,
    Ambient,
    UI,
    Footstep,
    Weapon,
    Voice
};

UENUM(BlueprintType)
enum class EForge2MusicState : uint8
{
    Explore,
    Combat,
    Boss,
    Stealth,
    Tension,
    Victory,
    Defeat,
    Menu,
    Cutscene,
    Custom
};

UENUM(BlueprintType)
enum class EForge2MusicTransition : uint8
{
    Crossfade,
    FadeOutIn,
    Immediate,
    BeatSync,
    BarSync,
    Stinger
};

UENUM(BlueprintType)
enum class EForge2OcclusionMode : uint8
{
    None,
    Simple,
    Complex,
    Raycast
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AttenuationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2AttenuationShape Shape = EForge2AttenuationShape::Sphere;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2AttenuationFalloff Falloff = EForge2AttenuationFalloff::NaturalSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float InnerRadius = 400.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FalloffDistance = 3600.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float dBAttenuationAtMax = -60.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAttenuate = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSpatialize = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2SpatializationMode SpatializationMode = EForge2SpatializationMode::Default;

    // Focus settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableFocusInterpolation = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FocusAzimuth = 30.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NonFocusAzimuth = 60.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FocusDistanceScale = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NonFocusDistanceScale = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FocusVolumeAttenuation = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float NonFocusVolumeAttenuation = -3.0f;

    // Priority
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PriorityAttenuationMin = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PriorityAttenuationMax = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ManualPriorityAttenuation = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2OcclusionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2OcclusionMode Mode = EForge2OcclusionMode::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LowPassFilterFrequency = 20000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VolumeAttenuation = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float InterpolationTime = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseComplexCollision = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TEnumAsByte<ECollisionChannel>> OcclusionTraceChannels;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ReverbSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Density = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Diffusion = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Gain = 0.32f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GainHF = 0.89f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DecayTime = 1.49f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DecayHFRatio = 0.83f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReflectionsGain = 0.05f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReflectionsDelay = 0.007f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LateReverbGain = 1.26f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LateReverbDelay = 0.011f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AirAbsorptionGainHF = 0.994f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RoomRolloffFactor = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SoundCueSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TSoftObjectPtr<USoundWave>> SoundWaves;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRandomize = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLoop = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VolumeMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PitchMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D VolumeRange = FVector2D(1.0f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D PitchRange = FVector2D(1.0f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeInTime = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeOutTime = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxConcurrentPlayCount = 8;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2SoundType SoundType = EForge2SoundType::SFX;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MusicLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString LayerName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USoundWave> SoundWave;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseVolume = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CurrentVolume = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<EForge2MusicState> ActiveStates;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeInTime = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeOutTime = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MusicTrack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TrackName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2MusicLayer> Layers;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BPM = 120.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BeatsPerBar = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2MusicState DefaultState = EForge2MusicState::Explore;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USoundWave> IntroStinger;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USoundWave> OutroStinger;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MusicTransitionRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2MusicState FromState;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2MusicState ToState;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2MusicTransition TransitionType = EForge2MusicTransition::Crossfade;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TransitionDuration = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USoundWave> TransitionStinger;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bWaitForBeat = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bWaitForBar = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SoundMixSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString MixName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EForge2SoundType, float> VolumeMultipliers;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MasterVolume = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeInTime = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeOutTime = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bApplyEQ = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EQLowFrequencyGain = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EQMidFrequencyGain = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EQHighFrequencyGain = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DialogueSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USoundWave> DialogueWave;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SubtitleText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SpeakerName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration = 0.0f;  // 0 = auto from wave
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowSubtitles = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDuckMusic = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MusicDuckAmount = 0.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDuckSFX = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SFXDuckAmount = 0.5f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AmbientZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ZoneName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TSoftObjectPtr<USoundWave>> AmbientSounds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D VolumeRange = FVector2D(0.8f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D PitchRange = FVector2D(0.95f, 1.05f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D DelayRange = FVector2D(5.0f, 15.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CrossfadeTime = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRandomizePosition = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RandomPositionRadius = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2ReverbSettings ReverbOverride;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2AudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2AudioManager();

    // Sound Cue Creation
    UFUNCTION(BlueprintCallable, Category = "Audio|SoundCue")
    USoundCue* CreateSoundCue(const FString& CueName, const FForge2SoundCueSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio|SoundCue")
    UAudioComponent* PlaySoundAtLocation(USoundCue* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Audio|SoundCue")
    UAudioComponent* PlaySoundAttached(USoundCue* Sound, USceneComponent* AttachTo, FName SocketName);

    // Attenuation
    UFUNCTION(BlueprintCallable, Category = "Audio|Attenuation")
    USoundAttenuation* CreateAttenuation(const FString& AttenuationName, const FForge2AttenuationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio|Attenuation")
    void ConfigureAttenuation(USoundAttenuation* Attenuation, const FForge2AttenuationSettings& Settings);

    // Occlusion
    UFUNCTION(BlueprintCallable, Category = "Audio|Occlusion")
    void ConfigureOcclusion(USoundAttenuation* Attenuation, const FForge2OcclusionSettings& Settings);

    // Sound Classes & Mixing
    UFUNCTION(BlueprintCallable, Category = "Audio|Mixing")
    USoundClass* CreateSoundClass(const FString& ClassName, EForge2SoundType SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Mixing")
    USoundMix* CreateSoundMix(const FString& MixName, const FForge2SoundMixSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio|Mixing")
    void PushSoundMix(USoundMix* SoundMix);

    UFUNCTION(BlueprintCallable, Category = "Audio|Mixing")
    void PopSoundMix(USoundMix* SoundMix);

    UFUNCTION(BlueprintCallable, Category = "Audio|Mixing")
    void SetSoundClassVolume(EForge2SoundType SoundType, float Volume);

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetupMusicTrack(const FForge2MusicTrack& Track);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void PlayMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StopMusic(float FadeOutTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicState(EForge2MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetLayerVolume(const FString& LayerName, float Volume, float FadeTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void EnableLayer(const FString& LayerName, bool bEnable, float FadeTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void AddMusicTransitionRule(const FForge2MusicTransitionRule& Rule);

    // Dialogue
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayDialogue(const FForge2DialogueSettings& Dialogue);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void QueueDialogue(const FForge2DialogueSettings& Dialogue);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void ClearDialogueQueue();

    // Ambient
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void EnterAmbientZone(const FForge2AmbientZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void ExitAmbientZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetReverbSettings(const FForge2ReverbSettings& Settings);

    // Presets
    UFUNCTION(BlueprintCallable, Category = "Audio|Presets")
    FForge2AttenuationSettings GetDefaultAttenuation(EForge2SoundType SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Presets")
    FForge2ReverbSettings GetReverbPreset(const FString& PresetName);

private:
    UPROPERTY() FForge2MusicTrack CurrentMusicTrack;
    UPROPERTY() EForge2MusicState CurrentMusicState;
    UPROPERTY() TArray<UAudioComponent*> MusicLayerComponents;
    UPROPERTY() TArray<FForge2MusicTransitionRule> TransitionRules;
    UPROPERTY() TArray<FForge2DialogueSettings> DialogueQueue;
    UPROPERTY() UAudioComponent* CurrentDialogueComponent;
    UPROPERTY() TMap<FString, TArray<UAudioComponent*>> AmbientComponents;

    void ApplyMusicState(EForge2MusicState State, float TransitionTime);
    FForge2MusicTransitionRule* FindTransitionRule(EForge2MusicState From, EForge2MusicState To);
    void ProcessDialogueQueue();
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2AudioUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2AttenuationSettings CreateSmallAttenuation();  // Indoor/close range

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2AttenuationSettings CreateMediumAttenuation();  // Standard outdoor

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2AttenuationSettings CreateLargeAttenuation();  // Explosions, distant

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2ReverbSettings CreateIndoorReverb();

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2ReverbSettings CreateOutdoorReverb();

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2ReverbSettings CreateCaveReverb();

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2ReverbSettings CreateCathedralReverb();

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2SoundMixSettings CreateCombatMix();

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2SoundMixSettings CreateCutsceneMix();

    UFUNCTION(BlueprintCallable, Category = "Audio|Utility")
    static FForge2SoundMixSettings CreatePausedMix();
};
