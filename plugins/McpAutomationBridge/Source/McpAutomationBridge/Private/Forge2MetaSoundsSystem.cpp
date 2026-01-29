// Forge2MetaSoundsSystem.cpp - MetaSounds Procedural Audio System Implementation
#include "Forge2MetaSoundsSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "MetasoundSource.h"
#include "MetasoundAssetManager.h"
#include "AudioDevice.h"
#include "Audio/AudioMixerBlueprintLibrary.h"

// UForge2MetaSoundsManager Implementation
UForge2MetaSoundsManager::UForge2MetaSoundsManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    IdCounter = 0;
}

FString UForge2MetaSoundsManager::GenerateUniqueId(const FString& Prefix)
{
    return FString::Printf(TEXT("%s_%d"), *Prefix, IdCounter++);
}

FString UForge2MetaSoundsManager::AddOscillator(const FForge2OscillatorSettings& Settings)
{
    FString Id = GenerateUniqueId(TEXT("Osc"));
    Oscillators.Add(Id, Settings);
    return Id;
}

void UForge2MetaSoundsManager::ConfigureOscillator(const FString& OscillatorId,
                                                     const FForge2OscillatorSettings& Settings)
{
    if (Oscillators.Contains(OscillatorId))
    {
        Oscillators[OscillatorId] = Settings;
    }
}

void UForge2MetaSoundsManager::SetOscillatorFrequency(const FString& OscillatorId, float Frequency)
{
    if (Oscillators.Contains(OscillatorId))
    {
        Oscillators[OscillatorId].Frequency = FMath::Clamp(Frequency, 20.0f, 20000.0f);
    }
}

void UForge2MetaSoundsManager::SetOscillatorType(const FString& OscillatorId, EForge2OscillatorType Type)
{
    if (Oscillators.Contains(OscillatorId))
    {
        Oscillators[OscillatorId].Type = Type;
    }
}

FString UForge2MetaSoundsManager::AddFilter(const FForge2FilterSettings& Settings)
{
    FString Id = GenerateUniqueId(TEXT("Filter"));
    Filters.Add(Id, Settings);
    return Id;
}

void UForge2MetaSoundsManager::ConfigureFilter(const FString& FilterId,
                                                 const FForge2FilterSettings& Settings)
{
    if (Filters.Contains(FilterId))
    {
        Filters[FilterId] = Settings;
    }
}

void UForge2MetaSoundsManager::SetFilterCutoff(const FString& FilterId, float Cutoff)
{
    if (Filters.Contains(FilterId))
    {
        Filters[FilterId].CutoffFrequency = FMath::Clamp(Cutoff, 20.0f, 20000.0f);
    }
}

void UForge2MetaSoundsManager::SetFilterResonance(const FString& FilterId, float Resonance)
{
    if (Filters.Contains(FilterId))
    {
        Filters[FilterId].Resonance = FMath::Clamp(Resonance, 0.0f, 1.0f);
    }
}

FString UForge2MetaSoundsManager::AddEnvelope(const FForge2EnvelopeSettings& Settings)
{
    FString Id = GenerateUniqueId(TEXT("Env"));
    Envelopes.Add(Id, Settings);
    return Id;
}

void UForge2MetaSoundsManager::ConfigureEnvelope(const FString& EnvelopeId,
                                                   const FForge2EnvelopeSettings& Settings)
{
    if (Envelopes.Contains(EnvelopeId))
    {
        Envelopes[EnvelopeId] = Settings;
    }
}

void UForge2MetaSoundsManager::TriggerEnvelope(const FString& EnvelopeId)
{
    if (!Envelopes.Contains(EnvelopeId))
    {
        return;
    }
    
    const FForge2EnvelopeSettings& Settings = Envelopes[EnvelopeId];
    
    // Find all audio components using this envelope and trigger them
    for (const auto& AudioPair : ActiveAudioComponents)
    {
        if (UAudioComponent* AudioComp = AudioPair.Value.Get())
        {
            // Set envelope trigger parameter on MetaSound
            AudioComp->SetFloatParameter(FName(*FString::Printf(TEXT("%s_Gate"), *EnvelopeId)), 1.0f);
            AudioComp->SetFloatParameter(FName(*FString::Printf(TEXT("%s_Attack"), *EnvelopeId)), Settings.AttackTime);
            AudioComp->SetFloatParameter(FName(*FString::Printf(TEXT("%s_Decay"), *EnvelopeId)), Settings.DecayTime);
            AudioComp->SetFloatParameter(FName(*FString::Printf(TEXT("%s_Sustain"), *EnvelopeId)), Settings.SustainLevel);
            AudioComp->SetFloatParameter(FName(*FString::Printf(TEXT("%s_Release"), *EnvelopeId)), Settings.ReleaseTime);
        }
    }
    
    // Mark envelope as triggered
    EnvelopeStates.Add(EnvelopeId, true);
}

void UForge2MetaSoundsManager::ReleaseEnvelope(const FString& EnvelopeId)
{
    if (!Envelopes.Contains(EnvelopeId))
    {
        return;
    }
    
    // Find all audio components using this envelope and release them
    for (const auto& AudioPair : ActiveAudioComponents)
    {
        if (UAudioComponent* AudioComp = AudioPair.Value.Get())
        {
            // Set envelope gate to 0 to trigger release phase
            AudioComp->SetFloatParameter(FName(*FString::Printf(TEXT("%s_Gate"), *EnvelopeId)), 0.0f);
        }
    }
    
    // Mark envelope as released
    EnvelopeStates.Add(EnvelopeId, false);
}

FString UForge2MetaSoundsManager::AddLFO(const FForge2LFOSettings& Settings)
{
    FString Id = GenerateUniqueId(TEXT("LFO"));
    LFOs.Add(Id, Settings);
    return Id;
}

void UForge2MetaSoundsManager::ConfigureLFO(const FString& LFOId, const FForge2LFOSettings& Settings)
{
    if (LFOs.Contains(LFOId))
    {
        LFOs[LFOId] = Settings;
    }
}

void UForge2MetaSoundsManager::AddModulationRouting(const FForge2ModulationRouting& Routing)
{
    ModulationRoutings.Add(Routing);
}

void UForge2MetaSoundsManager::RemoveModulationRouting(const FString& SourceName,
                                                         const FString& DestinationParameter)
{
    ModulationRoutings.RemoveAll([&](const FForge2ModulationRouting& Routing)
    {
        return Routing.SourceName == SourceName && Routing.DestinationParameter == DestinationParameter;
    });
}

void UForge2MetaSoundsManager::CreateProceduralSFX(const FString& SFXName,
                                                     const FForge2ProceduralSFXSettings& Settings)
{
    ProceduralSFXs.Add(SFXName, Settings);
}

void UForge2MetaSoundsManager::PlayProceduralSFX(const FString& SFXName, FVector Location)
{
    if (!ProceduralSFXs.Contains(SFXName)) return;

    const FForge2ProceduralSFXSettings& Settings = ProceduralSFXs[SFXName];

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Load the MetaSound source asset for this SFX category
    FString MetaSoundPath = GetMetaSoundPathForCategory(Settings.Category);
    UMetaSoundSource* MetaSoundAsset = Cast<UMetaSoundSource>(
        StaticLoadObject(UMetaSoundSource::StaticClass(), nullptr, *MetaSoundPath));
    
    if (!MetaSoundAsset)
    {
        // Fallback to basic sound playback
        return;
    }
    
    // Create audio component at location
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        MetaSoundAsset,
        Location,
        FRotator::ZeroRotator,
        1.0f, // Volume
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation
        nullptr, // Concurrency
        true // Auto destroy
    );
    
    if (!AudioComp)
    {
        return;
    }
    
    // Apply randomization to oscillator parameters
    FRandomStream Random(FMath::Rand());
    
    for (int32 i = 0; i < Settings.Oscillators.Num(); ++i)
    {
        const FForge2OscillatorSettings& Osc = Settings.Oscillators[i];
        
        // Apply pitch randomization
        float RandomPitch = FMath::Lerp(
            1.0f - Settings.PitchVariation,
            1.0f + Settings.PitchVariation,
            Random.FRand()
        );
        AudioComp->SetFloatParameter(FName(*FString::Printf(TEXT("Osc%d_Frequency"), i)), Osc.Frequency * RandomPitch);
        AudioComp->SetFloatParameter(FName(*FString::Printf(TEXT("Osc%d_Amplitude"), i)), Osc.Amplitude);
        AudioComp->SetIntParameter(FName(*FString::Printf(TEXT("Osc%d_Type"), i)), static_cast<int32>(Osc.Type));
    }
    
    // Apply envelope settings
    if (Settings.Envelope.AttackTime > 0.0f || Settings.Envelope.ReleaseTime > 0.0f)
    {
        AudioComp->SetFloatParameter(FName(TEXT("Env_Attack")), Settings.Envelope.AttackTime);
        AudioComp->SetFloatParameter(FName(TEXT("Env_Decay")), Settings.Envelope.DecayTime);
        AudioComp->SetFloatParameter(FName(TEXT("Env_Sustain")), Settings.Envelope.SustainLevel);
        AudioComp->SetFloatParameter(FName(TEXT("Env_Release")), Settings.Envelope.ReleaseTime);
    }
    
    // Apply filter settings
    if (Settings.Filter.CutoffFrequency > 0.0f)
    {
        AudioComp->SetFloatParameter(FName(TEXT("Filter_Cutoff")), Settings.Filter.CutoffFrequency);
        AudioComp->SetFloatParameter(FName(TEXT("Filter_Resonance")), Settings.Filter.Resonance);
        AudioComp->SetIntParameter(FName(TEXT("Filter_Type")), static_cast<int32>(Settings.Filter.Type));
    }
    
    // Trigger envelope and play
    AudioComp->SetFloatParameter(FName(TEXT("Env_Gate")), 1.0f);
    
    // Store reference for tracking
    FString InstanceId = GenerateUniqueId(TEXT("SFX"));
    ActiveAudioComponents.Add(InstanceId, AudioComp);
}

FForge2ProceduralSFXSettings UForge2MetaSoundsManager::CreateFootstepSFX(const FString& SurfaceType)
{
    FForge2ProceduralSFXSettings Settings;
    Settings.Category = EForge2SFXCategory::Footsteps;

    // Base noise oscillator for texture
    FForge2OscillatorSettings NoiseOsc;
    NoiseOsc.Type = EForge2OscillatorType::Noise;
    NoiseOsc.Amplitude = 0.3f;
    Settings.Oscillators.Add(NoiseOsc);

    // Add characteristic based on surface
    FForge2OscillatorSettings ToneOsc;
    ToneOsc.Type = EForge2OscillatorType::Sine;
    ToneOsc.Amplitude = 0.1f;

    if (SurfaceType == TEXT("Wood"))
    {
        ToneOsc.Frequency = 200.0f;
        Settings.AmpEnvelope = UForge2MetaSoundsUtility::CreatePercussiveEnvelope(0.001f, 0.1f);
    }
    else if (SurfaceType == TEXT("Metal"))
    {
        ToneOsc.Frequency = 800.0f;
        ToneOsc.Amplitude = 0.2f;
        Settings.AmpEnvelope = UForge2MetaSoundsUtility::CreatePercussiveEnvelope(0.001f, 0.15f);
    }
    else if (SurfaceType == TEXT("Gravel"))
    {
        ToneOsc.Frequency = 100.0f;
        NoiseOsc.Amplitude = 0.5f;
        Settings.AmpEnvelope = UForge2MetaSoundsUtility::CreatePercussiveEnvelope(0.005f, 0.08f);
    }
    else if (SurfaceType == TEXT("Grass"))
    {
        ToneOsc.Frequency = 150.0f;
        NoiseOsc.Amplitude = 0.4f;
        Settings.AmpEnvelope = UForge2MetaSoundsUtility::CreatePercussiveEnvelope(0.01f, 0.05f);
    }
    else  // Default (concrete)
    {
        ToneOsc.Frequency = 300.0f;
        Settings.AmpEnvelope = UForge2MetaSoundsUtility::CreatePercussiveEnvelope(0.001f, 0.08f);
    }

    Settings.Oscillators.Add(ToneOsc);

    // Low pass filter for natural sound
    FForge2FilterSettings LPFilter;
    LPFilter.Type = EForge2FilterType::LowPass;
    LPFilter.CutoffFrequency = 2000.0f;
    LPFilter.Resonance = 0.2f;
    Settings.Filters.Add(LPFilter);

    // Add randomization
    Settings.PitchRandomRange = 0.1f;
    Settings.VolumeRandomRange = 0.15f;

    return Settings;
}

FForge2ProceduralSFXSettings UForge2MetaSoundsManager::CreateImpactSFX(float Intensity)
{
    FForge2ProceduralSFXSettings Settings;
    Settings.Category = EForge2SFXCategory::Impacts;

    // Noise burst for initial transient
    FForge2OscillatorSettings NoiseOsc;
    NoiseOsc.Type = EForge2OscillatorType::Noise;
    NoiseOsc.Amplitude = Intensity;
    Settings.Oscillators.Add(NoiseOsc);

    // Low sine for body/rumble
    FForge2OscillatorSettings BodyOsc;
    BodyOsc.Type = EForge2OscillatorType::Sine;
    BodyOsc.Frequency = 60.0f + (Intensity * 40.0f);
    BodyOsc.Amplitude = Intensity * 0.5f;
    Settings.Oscillators.Add(BodyOsc);

    // Envelope scales with intensity
    float DecayTime = 0.05f + (Intensity * 0.3f);
    Settings.AmpEnvelope = UForge2MetaSoundsUtility::CreatePercussiveEnvelope(0.001f, DecayTime);

    // Filter envelope for punch
    Settings.FilterEnvelope.Type = EForge2EnvelopeType::AR;
    Settings.FilterEnvelope.Attack = 0.001f;
    Settings.FilterEnvelope.Release = DecayTime * 0.8f;

    FForge2FilterSettings HPFilter;
    HPFilter.Type = EForge2FilterType::HighPass;
    HPFilter.CutoffFrequency = 50.0f;
    HPFilter.bEnvelopeModulation = true;
    HPFilter.EnvelopeAmount = 500.0f;
    Settings.Filters.Add(HPFilter);

    Settings.PitchRandomRange = 0.05f;
    Settings.VolumeRandomRange = 0.1f;

    return Settings;
}

FForge2ProceduralSFXSettings UForge2MetaSoundsManager::CreateAmbienceSFX(const FString& AmbienceType)
{
    FForge2ProceduralSFXSettings Settings;
    Settings.Category = EForge2SFXCategory::Ambience;

    if (AmbienceType == TEXT("Wind"))
    {
        FForge2OscillatorSettings NoiseOsc;
        NoiseOsc.Type = EForge2OscillatorType::Noise;
        NoiseOsc.Amplitude = 0.3f;
        Settings.Oscillators.Add(NoiseOsc);

        FForge2FilterSettings BPFilter;
        BPFilter.Type = EForge2FilterType::BandPass;
        BPFilter.CutoffFrequency = 500.0f;
        BPFilter.Resonance = 0.3f;
        Settings.Filters.Add(BPFilter);

        FForge2LFOSettings VolumeModLFO;
        VolumeModLFO.Shape = EForge2OscillatorType::Sine;
        VolumeModLFO.Rate = 0.1f;
        VolumeModLFO.Depth = 0.3f;
        Settings.LFOs.Add(VolumeModLFO);

        FForge2LFOSettings FilterModLFO;
        FilterModLFO.Shape = EForge2OscillatorType::Sine;
        FilterModLFO.Rate = 0.05f;
        FilterModLFO.Depth = 0.5f;
        Settings.LFOs.Add(FilterModLFO);
    }
    else if (AmbienceType == TEXT("Rain"))
    {
        FForge2OscillatorSettings NoiseOsc;
        NoiseOsc.Type = EForge2OscillatorType::Noise;
        NoiseOsc.Amplitude = 0.4f;
        Settings.Oscillators.Add(NoiseOsc);

        FForge2FilterSettings HPFilter;
        HPFilter.Type = EForge2FilterType::HighPass;
        HPFilter.CutoffFrequency = 1000.0f;
        Settings.Filters.Add(HPFilter);

        FForge2FilterSettings LPFilter;
        LPFilter.Type = EForge2FilterType::LowPass;
        LPFilter.CutoffFrequency = 8000.0f;
        Settings.Filters.Add(LPFilter);
    }
    else if (AmbienceType == TEXT("Fire"))
    {
        FForge2OscillatorSettings NoiseOsc;
        NoiseOsc.Type = EForge2OscillatorType::Noise;
        NoiseOsc.Amplitude = 0.5f;
        Settings.Oscillators.Add(NoiseOsc);

        FForge2FilterSettings BPFilter;
        BPFilter.Type = EForge2FilterType::BandPass;
        BPFilter.CutoffFrequency = 800.0f;
        BPFilter.Resonance = 0.4f;
        Settings.Filters.Add(BPFilter);

        FForge2LFOSettings CracklingLFO;
        CracklingLFO.Shape = EForge2OscillatorType::Noise;
        CracklingLFO.Rate = 3.0f;
        CracklingLFO.Depth = 0.4f;
        Settings.LFOs.Add(CracklingLFO);
    }

    return Settings;
}

void UForge2MetaSoundsManager::CreateDynamicMusic(const FString& TrackName,
                                                    const FForge2DynamicMusicSettings& Settings)
{
    DynamicMusicTracks.Add(TrackName, Settings);
}

void UForge2MetaSoundsManager::PlayDynamicMusic(const FString& TrackName)
{
    if (!DynamicMusicTracks.Contains(TrackName)) return;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    const FForge2DynamicMusicSettings& Settings = DynamicMusicTracks[TrackName];
    
    // Load the MetaSound music source
    FString MetaSoundPath = FString::Printf(TEXT("/Game/FORGE2/Audio/Music/MS_Music_%s"), *TrackName);
    UMetaSoundSource* MusicAsset = Cast<UMetaSoundSource>(
        StaticLoadObject(UMetaSoundSource::StaticClass(), nullptr, *MetaSoundPath));
    
    if (!MusicAsset)
    {
        // Try loading as regular sound wave
        USoundBase* SoundAsset = Cast<USoundBase>(
            StaticLoadObject(USoundBase::StaticClass(), nullptr, *MetaSoundPath));
        if (!SoundAsset)
        {
            return;
        }
    }
    
    // Create 2D audio component for music (non-spatialized)
    UAudioComponent* MusicComp = NewObject<UAudioComponent>(GetOwner());
    if (!MusicComp)
    {
        return;
    }
    
    MusicComp->SetSound(MusicAsset);
    MusicComp->bAutoDestroy = false;
    MusicComp->bIsUISound = true; // 2D playback
    MusicComp->RegisterComponent();
    
    // Set initial BPM and time signature parameters
    MusicComp->SetFloatParameter(FName(TEXT("BPM")), Settings.BPM);
    MusicComp->SetIntParameter(FName(TEXT("BeatsPerBar")), Settings.BeatsPerBar);
    
    // Initialize all layers to their starting state
    for (const FForge2MusicLayer& Layer : Settings.Layers)
    {
        FString LayerVolumeParam = FString::Printf(TEXT("Layer_%s_Volume"), *Layer.LayerName);
        float InitialVolume = Layer.bIsActive ? 1.0f : 0.0f;
        MusicComp->SetFloatParameter(FName(*LayerVolumeParam), InitialVolume);
    }
    
    // Store the music component
    ActiveMusicTracks.Add(TrackName, MusicComp);
    
    // Start playback
    MusicComp->Play();
}

void UForge2MetaSoundsManager::StopDynamicMusic(const FString& TrackName, float FadeOutTime)
{
    // Fade out and stop the track
}

void UForge2MetaSoundsManager::SetMusicLayer(const FString& TrackName, const FString& LayerName, bool bEnabled)
{
    // Enable/disable individual music layers
}

void UForge2MetaSoundsManager::SetMusicIntensity(const FString& TrackName, float Intensity)
{
    // Adjust music intensity (0-1) which controls which layers are active
}

void UForge2MetaSoundsManager::TransitionToTrack(const FString& FromTrack, const FString& ToTrack,
                                                   float TransitionTime)
{
    // Cross-fade between tracks with optional quantization
}

void UForge2MetaSoundsManager::ConfigureSpatialization(UAudioComponent* AudioComponent,
                                                         const FForge2SpatializationSettings& Settings)
{
    if (!AudioComponent) return;

    // Configure spatialization settings
    AudioComponent->bAllowSpatialization = Settings.bEnableSpatialAudio;

    // In UE5, would configure attenuation settings here
}

void UForge2MetaSoundsManager::SetListenerPosition(FVector Position, FRotator Rotation)
{
    // Update listener position for spatialization
}

// UForge2MetaSoundsUtility Implementation
FForge2OscillatorSettings UForge2MetaSoundsUtility::CreateBasicOscillator(EForge2OscillatorType Type,
                                                                           float Frequency)
{
    FForge2OscillatorSettings Settings;
    Settings.Type = Type;
    Settings.Frequency = Frequency;
    Settings.Amplitude = 1.0f;
    Settings.Phase = 0.0f;
    Settings.PulseWidth = 0.5f;
    Settings.Detune = 0.0f;
    Settings.UnisonVoices = 1;
    return Settings;
}

FForge2FilterSettings UForge2MetaSoundsUtility::CreateBasicFilter(EForge2FilterType Type,
                                                                   float Cutoff, float Resonance)
{
    FForge2FilterSettings Settings;
    Settings.Type = Type;
    Settings.CutoffFrequency = Cutoff;
    Settings.Resonance = Resonance;
    Settings.Gain = 0.0f;
    Settings.KeyTracking = 0.0f;
    return Settings;
}

FForge2EnvelopeSettings UForge2MetaSoundsUtility::CreateADSREnvelope(float Attack, float Decay,
                                                                      float Sustain, float Release)
{
    FForge2EnvelopeSettings Settings;
    Settings.Type = EForge2EnvelopeType::ADSR;
    Settings.Attack = Attack;
    Settings.Decay = Decay;
    Settings.Sustain = Sustain;
    Settings.Release = Release;
    return Settings;
}

FForge2EnvelopeSettings UForge2MetaSoundsUtility::CreatePercussiveEnvelope(float Attack, float Decay)
{
    FForge2EnvelopeSettings Settings;
    Settings.Type = EForge2EnvelopeType::AR;
    Settings.Attack = Attack;
    Settings.Decay = 0.0f;
    Settings.Sustain = 0.0f;
    Settings.Release = Decay;
    return Settings;
}

FForge2LFOSettings UForge2MetaSoundsUtility::CreateBasicLFO(EForge2OscillatorType Shape,
                                                             float Rate, float Depth)
{
    FForge2LFOSettings Settings;
    Settings.Shape = Shape;
    Settings.Rate = Rate;
    Settings.Depth = Depth;
    Settings.Phase = 0.0f;
    Settings.bSyncToTempo = false;
    Settings.bRetrigger = false;
    return Settings;
}

FForge2SpatializationSettings UForge2MetaSoundsUtility::CreateDefaultSpatialization()
{
    FForge2SpatializationSettings Settings;
    Settings.bEnableSpatialAudio = true;
    Settings.InnerRadius = 100.0f;
    Settings.OuterRadius = 2000.0f;
    Settings.FalloffExponent = 2.0f;
    Settings.bEnableOcclusion = true;
    Settings.OcclusionLowPassFrequency = 1000.0f;
    Settings.OcclusionVolumeAttenuation = 0.5f;
    Settings.bEnableReverb = true;
    Settings.ReverbSendLevel = 0.5f;
    return Settings;
}

FForge2SpatializationSettings UForge2MetaSoundsUtility::CreateIndoorSpatialization()
{
    FForge2SpatializationSettings Settings = CreateDefaultSpatialization();
    Settings.InnerRadius = 50.0f;
    Settings.OuterRadius = 1000.0f;
    Settings.FalloffExponent = 1.5f;
    Settings.OcclusionLowPassFrequency = 500.0f;
    Settings.ReverbSendLevel = 0.8f;
    return Settings;
}

FForge2SpatializationSettings UForge2MetaSoundsUtility::CreateOutdoorSpatialization()
{
    FForge2SpatializationSettings Settings = CreateDefaultSpatialization();
    Settings.InnerRadius = 200.0f;
    Settings.OuterRadius = 5000.0f;
    Settings.FalloffExponent = 2.5f;
    Settings.OcclusionLowPassFrequency = 2000.0f;
    Settings.ReverbSendLevel = 0.3f;
    return Settings;
}

float UForge2MetaSoundsUtility::MidiNoteToFrequency(int32 MidiNote)
{
    // A4 (MIDI note 69) = 440 Hz
    return 440.0f * FMath::Pow(2.0f, (MidiNote - 69) / 12.0f);
}

int32 UForge2MetaSoundsUtility::FrequencyToMidiNote(float Frequency)
{
    if (Frequency <= 0.0f) return 0;
    return FMath::RoundToInt(69.0f + 12.0f * FMath::Loge(Frequency / 440.0f) / FMath::Loge(2.0f));
}
