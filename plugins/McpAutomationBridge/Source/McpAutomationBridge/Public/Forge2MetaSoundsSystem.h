// Forge2MetaSoundsSystem.h - MetaSounds Procedural Audio System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2MetaSoundsSystem.generated.h"

// Forward declarations
class USoundWave;
class UAudioComponent;
class UMetaSoundSource;

// Enums
UENUM(BlueprintType)
enum class EForge2OscillatorType : uint8
{
    Sine,
    Saw,
    Square,
    Triangle,
    Noise,
    Pulse,
    Custom
};

UENUM(BlueprintType)
enum class EForge2FilterType : uint8
{
    LowPass,
    HighPass,
    BandPass,
    Notch,
    LowShelf,
    HighShelf,
    Peaking
};

UENUM(BlueprintType)
enum class EForge2EnvelopeType : uint8
{
    ADSR,
    AR,
    ASR,
    Custom
};

UENUM(BlueprintType)
enum class EForge2ModulationType : uint8
{
    None,
    FM,
    AM,
    RingMod,
    PWM
};

UENUM(BlueprintType)
enum class EForge2SFXCategory : uint8
{
    Footsteps,
    Impacts,
    Weapons,
    Ambience,
    UI,
    Creatures,
    Environment,
    Magic,
    Vehicles,
    Custom
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2OscillatorSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2OscillatorType Type = EForge2OscillatorType::Sine;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Frequency = 440.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Amplitude = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Phase = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PulseWidth = 0.5f;  // For pulse wave
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Detune = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 UnisonVoices = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float UnisonSpread = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FilterSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2FilterType Type = EForge2FilterType::LowPass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CutoffFrequency = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Resonance = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Gain = 0.0f;  // For shelf/peaking filters
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float KeyTracking = 0.0f;  // Filter follows pitch
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnvelopeModulation = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EnvelopeAmount = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2EnvelopeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2EnvelopeType Type = EForge2EnvelopeType::ADSR;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Attack = 0.01f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Decay = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Sustain = 0.7f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Release = 0.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackCurve = 0.0f;  // -1 to 1, 0 = linear
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DecayCurve = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReleaseCurve = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2LFOSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2OscillatorType Shape = EForge2OscillatorType::Sine;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Rate = 1.0f;  // Hz
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Depth = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Phase = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSyncToTempo = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TempoSyncDivision = 1.0f;  // 1 = quarter note
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRetrigger = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ModulationRouting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SourceName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DestinationParameter;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Amount = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bBipolar = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ProceduralSFXSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2SFXCategory Category = EForge2SFXCategory::Custom;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2OscillatorSettings> Oscillators;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2FilterSettings> Filters;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2EnvelopeSettings AmpEnvelope;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2EnvelopeSettings FilterEnvelope;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2LFOSettings> LFOs;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2ModulationRouting> Modulations;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PitchRandomRange = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VolumeRandomRange = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MusicLayerSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString LayerName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USoundWave> AudioAsset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Volume = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeInTime = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeOutTime = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLooping = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float StartOffset = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TriggerCondition;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DynamicMusicSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TrackName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BPM = 120.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BeatsPerBar = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2MusicLayerSettings> Layers;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CrossfadeTime = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bQuantizeTransitions = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TransitionQuantization = 4;  // Beats
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SpatializationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableSpatialAudio = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float InnerRadius = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float OuterRadius = 2000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FalloffExponent = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableOcclusion = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float OcclusionLowPassFrequency = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float OcclusionVolumeAttenuation = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableReverb = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReverbSendLevel = 0.5f;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2MetaSoundsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2MetaSoundsManager();

    // Oscillator Management
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Oscillator")
    FString AddOscillator(const FForge2OscillatorSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Oscillator")
    void ConfigureOscillator(const FString& OscillatorId, const FForge2OscillatorSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Oscillator")
    void SetOscillatorFrequency(const FString& OscillatorId, float Frequency);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Oscillator")
    void SetOscillatorType(const FString& OscillatorId, EForge2OscillatorType Type);

    // Filter Management
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Filter")
    FString AddFilter(const FForge2FilterSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Filter")
    void ConfigureFilter(const FString& FilterId, const FForge2FilterSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Filter")
    void SetFilterCutoff(const FString& FilterId, float Cutoff);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Filter")
    void SetFilterResonance(const FString& FilterId, float Resonance);

    // Envelope Management
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Envelope")
    FString AddEnvelope(const FForge2EnvelopeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Envelope")
    void ConfigureEnvelope(const FString& EnvelopeId, const FForge2EnvelopeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Envelope")
    void TriggerEnvelope(const FString& EnvelopeId);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Envelope")
    void ReleaseEnvelope(const FString& EnvelopeId);

    // LFO Management
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|LFO")
    FString AddLFO(const FForge2LFOSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|LFO")
    void ConfigureLFO(const FString& LFOId, const FForge2LFOSettings& Settings);

    // Modulation Routing
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Modulation")
    void AddModulationRouting(const FForge2ModulationRouting& Routing);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Modulation")
    void RemoveModulationRouting(const FString& SourceName, const FString& DestinationParameter);

    // Procedural SFX
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|SFX")
    void CreateProceduralSFX(const FString& SFXName, const FForge2ProceduralSFXSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|SFX")
    void PlayProceduralSFX(const FString& SFXName, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|SFX")
    FForge2ProceduralSFXSettings CreateFootstepSFX(const FString& SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|SFX")
    FForge2ProceduralSFXSettings CreateImpactSFX(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|SFX")
    FForge2ProceduralSFXSettings CreateAmbienceSFX(const FString& AmbienceType);

    // Dynamic Music
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Music")
    void CreateDynamicMusic(const FString& TrackName, const FForge2DynamicMusicSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Music")
    void PlayDynamicMusic(const FString& TrackName);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Music")
    void StopDynamicMusic(const FString& TrackName, float FadeOutTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Music")
    void SetMusicLayer(const FString& TrackName, const FString& LayerName, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Music")
    void SetMusicIntensity(const FString& TrackName, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Music")
    void TransitionToTrack(const FString& FromTrack, const FString& ToTrack, float TransitionTime);

    // Spatialization
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Spatial")
    void ConfigureSpatialization(UAudioComponent* AudioComponent, const FForge2SpatializationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Spatial")
    void SetListenerPosition(FVector Position, FRotator Rotation);

private:
    UPROPERTY() TMap<FString, FForge2OscillatorSettings> Oscillators;
    UPROPERTY() TMap<FString, FForge2FilterSettings> Filters;
    UPROPERTY() TMap<FString, FForge2EnvelopeSettings> Envelopes;
    UPROPERTY() TMap<FString, FForge2LFOSettings> LFOs;
    UPROPERTY() TArray<FForge2ModulationRouting> ModulationRoutings;
    UPROPERTY() TMap<FString, FForge2ProceduralSFXSettings> ProceduralSFXs;
    UPROPERTY() TMap<FString, FForge2DynamicMusicSettings> DynamicMusicTracks;

    FString GenerateUniqueId(const FString& Prefix);
    int32 IdCounter = 0;
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2MetaSoundsUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Utility")
    static FForge2OscillatorSettings CreateBasicOscillator(EForge2OscillatorType Type, float Frequency);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Utility")
    static FForge2FilterSettings CreateBasicFilter(EForge2FilterType Type, float Cutoff, float Resonance);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Utility")
    static FForge2EnvelopeSettings CreateADSREnvelope(float Attack, float Decay, float Sustain, float Release);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Utility")
    static FForge2EnvelopeSettings CreatePercussiveEnvelope(float Attack, float Decay);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Utility")
    static FForge2LFOSettings CreateBasicLFO(EForge2OscillatorType Shape, float Rate, float Depth);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Utility")
    static FForge2SpatializationSettings CreateDefaultSpatialization();

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Utility")
    static FForge2SpatializationSettings CreateIndoorSpatialization();

    UFUNCTION(BlueprintCallable, Category = "MetaSounds|Utility")
    static FForge2SpatializationSettings CreateOutdoorSpatialization();

    UFUNCTION(BlueprintPure, Category = "MetaSounds|Utility")
    static float MidiNoteToFrequency(int32 MidiNote);

    UFUNCTION(BlueprintPure, Category = "MetaSounds|Utility")
    static int32 FrequencyToMidiNote(float Frequency);
};
