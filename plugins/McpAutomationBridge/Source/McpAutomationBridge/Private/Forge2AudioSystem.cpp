// Forge2AudioSystem.cpp - Audio System Implementation
#include "Forge2AudioSystem.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/ReverbEffect.h"
#include "Sound/AudioVolume.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "Sound/SoundNodeAttenuation.h"
#include "Sound/SoundNodeModulator.h"
#include "Sound/SoundNodeRandom.h"
#include "Sound/SoundNodeMixer.h"
#include "Sound/SoundNodeConcatenator.h"
#include "Sound/SoundNodeLooping.h"
#include "Sound/SoundNodeDelay.h"
#include "Sound/SoundNodeEnveloper.h"

// UForge2AudioManager Implementation
UForge2AudioManager::UForge2AudioManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentMusicState = EForge2MusicState::Explore;
    CurrentDialogueComponent = nullptr;
}

USoundCue* UForge2AudioManager::CreateSoundCue(const FString& CueName, const FForge2SoundCueSettings& Settings)
{
#if WITH_EDITOR
    // Create the SoundCue asset
    USoundCue* SoundCue = NewObject<USoundCue>(GetTransientPackage(), *CueName, RF_Public | RF_Standalone);
    if (!SoundCue) return nullptr;

    // Track current Y position for node placement
    int32 NodePosY = 0;
    const int32 NodeSpacingY = 150;

    // Create the first graph node based on settings
    USoundNode* OutputNode = nullptr;

    // If we have multiple sound waves, create appropriate structure
    if (Settings.SoundWaves.Num() > 1)
    {
        if (Settings.bRandomize)
        {
            // Create Random node for multiple sounds
            USoundNodeRandom* RandomNode = SoundCue->ConstructSoundNode<USoundNodeRandom>();
            RandomNode->GraphNode->NodePosX = -300;
            RandomNode->GraphNode->NodePosY = NodePosY;
            
            // Add wave players as children
            RandomNode->SetChildNodes(Settings.SoundWaves.Num());
            
            for (int32 i = 0; i < Settings.SoundWaves.Num(); ++i)
            {
                if (Settings.SoundWaves[i].IsValid())
                {
                    USoundNodeWavePlayer* WavePlayer = SoundCue->ConstructSoundNode<USoundNodeWavePlayer>();
                    WavePlayer->SetSoundWave(Settings.SoundWaves[i].LoadSynchronous());
                    WavePlayer->GraphNode->NodePosX = -600;
                    WavePlayer->GraphNode->NodePosY = NodePosY + (i * NodeSpacingY);
                    
                    RandomNode->InsertChildNode(i);
                    RandomNode->ChildNodes[i] = WavePlayer;
                }
            }
            
            OutputNode = RandomNode;
        }
        else if (Settings.bConcatenate)
        {
            // Create Concatenator for sequential playback
            USoundNodeConcatenator* ConcatNode = SoundCue->ConstructSoundNode<USoundNodeConcatenator>();
            ConcatNode->GraphNode->NodePosX = -300;
            ConcatNode->GraphNode->NodePosY = NodePosY;
            
            ConcatNode->SetChildNodes(Settings.SoundWaves.Num());
            
            for (int32 i = 0; i < Settings.SoundWaves.Num(); ++i)
            {
                if (Settings.SoundWaves[i].IsValid())
                {
                    USoundNodeWavePlayer* WavePlayer = SoundCue->ConstructSoundNode<USoundNodeWavePlayer>();
                    WavePlayer->SetSoundWave(Settings.SoundWaves[i].LoadSynchronous());
                    WavePlayer->GraphNode->NodePosX = -600;
                    WavePlayer->GraphNode->NodePosY = NodePosY + (i * NodeSpacingY);
                    
                    ConcatNode->InsertChildNode(i);
                    ConcatNode->ChildNodes[i] = WavePlayer;
                }
            }
            
            OutputNode = ConcatNode;
        }
        else
        {
            // Create Mixer for layered sounds
            USoundNodeMixer* MixerNode = SoundCue->ConstructSoundNode<USoundNodeMixer>();
            MixerNode->GraphNode->NodePosX = -300;
            MixerNode->GraphNode->NodePosY = NodePosY;
            
            MixerNode->SetChildNodes(Settings.SoundWaves.Num());
            
            for (int32 i = 0; i < Settings.SoundWaves.Num(); ++i)
            {
                if (Settings.SoundWaves[i].IsValid())
                {
                    USoundNodeWavePlayer* WavePlayer = SoundCue->ConstructSoundNode<USoundNodeWavePlayer>();
                    WavePlayer->SetSoundWave(Settings.SoundWaves[i].LoadSynchronous());
                    WavePlayer->GraphNode->NodePosX = -600;
                    WavePlayer->GraphNode->NodePosY = NodePosY + (i * NodeSpacingY);
                    
                    MixerNode->InsertChildNode(i);
                    MixerNode->ChildNodes[i] = WavePlayer;
                    
                    // Set input volume for mixer
                    if (i < MixerNode->InputVolume.Num())
                    {
                        MixerNode->InputVolume[i] = 1.0f / Settings.SoundWaves.Num();
                    }
                }
            }
            
            OutputNode = MixerNode;
        }
    }
    else if (Settings.SoundWaves.Num() == 1 && Settings.SoundWaves[0].IsValid())
    {
        // Single sound wave
        USoundNodeWavePlayer* WavePlayer = SoundCue->ConstructSoundNode<USoundNodeWavePlayer>();
        WavePlayer->SetSoundWave(Settings.SoundWaves[0].LoadSynchronous());
        WavePlayer->GraphNode->NodePosX = -300;
        WavePlayer->GraphNode->NodePosY = NodePosY;
        
        OutputNode = WavePlayer;
    }

    if (!OutputNode) return SoundCue;

    // Add modulator for pitch/volume variation
    if (Settings.PitchVariation > 0.0f || Settings.VolumeVariation > 0.0f)
    {
        USoundNodeModulator* ModulatorNode = SoundCue->ConstructSoundNode<USoundNodeModulator>();
        ModulatorNode->PitchMin = 1.0f - Settings.PitchVariation;
        ModulatorNode->PitchMax = 1.0f + Settings.PitchVariation;
        ModulatorNode->VolumeMin = 1.0f - Settings.VolumeVariation;
        ModulatorNode->VolumeMax = 1.0f + Settings.VolumeVariation;
        ModulatorNode->GraphNode->NodePosX = -150;
        ModulatorNode->GraphNode->NodePosY = NodePosY;
        
        ModulatorNode->SetChildNodes(1);
        ModulatorNode->ChildNodes[0] = OutputNode;
        OutputNode = ModulatorNode;
    }

    // Add looping if needed
    if (Settings.bLoop)
    {
        USoundNodeLooping* LoopNode = SoundCue->ConstructSoundNode<USoundNodeLooping>();
        LoopNode->bLoopIndefinitely = true;
        LoopNode->GraphNode->NodePosX = 0;
        LoopNode->GraphNode->NodePosY = NodePosY;
        
        LoopNode->SetChildNodes(1);
        LoopNode->ChildNodes[0] = OutputNode;
        OutputNode = LoopNode;
    }

    // Add attenuation
    if (Settings.Attenuation.IsValid())
    {
        USoundNodeAttenuation* AttNode = SoundCue->ConstructSoundNode<USoundNodeAttenuation>();
        AttNode->AttenuationSettings = Settings.Attenuation.LoadSynchronous();
        AttNode->GraphNode->NodePosX = 150;
        AttNode->GraphNode->NodePosY = NodePosY;
        
        AttNode->SetChildNodes(1);
        AttNode->ChildNodes[0] = OutputNode;
        OutputNode = AttNode;
    }

    // Set first node as output
    SoundCue->FirstNode = OutputNode;

    // Set cue properties
    SoundCue->VolumeMultiplier = Settings.VolumeMultiplier;
    SoundCue->PitchMultiplier = Settings.PitchMultiplier;

    // Link nodes
    SoundCue->LinkGraphNodesFromSoundNodes();

    return SoundCue;
#else
    return nullptr;
#endif
}

UAudioComponent* UForge2AudioManager::PlaySoundAtLocation(USoundCue* Sound, FVector Location,
                                                            float VolumeMultiplier, float PitchMultiplier)
{
    if (!Sound || !GetWorld()) return nullptr;

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        VolumeMultiplier,
        PitchMultiplier,
        0.0f,  // Start time
        nullptr,  // Attenuation
        nullptr,  // Concurrency
        true  // Auto destroy
    );

    return AudioComponent;
}

UAudioComponent* UForge2AudioManager::PlaySoundAttached(USoundCue* Sound, USceneComponent* AttachTo, FName SocketName)
{
    if (!Sound || !AttachTo) return nullptr;

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(
        Sound,
        AttachTo,
        SocketName,
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        true,  // Stop when attached to destroyed
        1.0f,  // Volume
        1.0f,  // Pitch
        0.0f,  // Start time
        nullptr,  // Attenuation
        nullptr,  // Concurrency
        true  // Auto destroy
    );

    return AudioComponent;
}

USoundAttenuation* UForge2AudioManager::CreateAttenuation(const FString& AttenuationName,
                                                            const FForge2AttenuationSettings& Settings)
{
#if WITH_EDITOR
    USoundAttenuation* Attenuation = NewObject<USoundAttenuation>(GetTransientPackage(), *AttenuationName, RF_Public | RF_Standalone);
    if (Attenuation)
    {
        ConfigureAttenuation(Attenuation, Settings);
    }
    return Attenuation;
#else
    return nullptr;
#endif
}

void UForge2AudioManager::ConfigureAttenuation(USoundAttenuation* Attenuation,
                                                  const FForge2AttenuationSettings& Settings)
{
    if (!Attenuation) return;

    FSoundAttenuationSettings& AttSettings = Attenuation->Attenuation;

    // Shape
    switch (Settings.Shape)
    {
        case EForge2AttenuationShape::Sphere:
            AttSettings.AttenuationShape = EAttenuationShape::Sphere;
            break;
        case EForge2AttenuationShape::Capsule:
            AttSettings.AttenuationShape = EAttenuationShape::Capsule;
            break;
        case EForge2AttenuationShape::Box:
            AttSettings.AttenuationShape = EAttenuationShape::Box;
            break;
        case EForge2AttenuationShape::Cone:
            AttSettings.AttenuationShape = EAttenuationShape::Cone;
            break;
    }

    // Falloff
    switch (Settings.Falloff)
    {
        case EForge2AttenuationFalloff::Linear:
            AttSettings.FalloffMode = EAttenuationDistanceModel::Linear;
            break;
        case EForge2AttenuationFalloff::Logarithmic:
            AttSettings.FalloffMode = EAttenuationDistanceModel::Logarithmic;
            break;
        case EForge2AttenuationFalloff::NaturalSound:
            AttSettings.FalloffMode = EAttenuationDistanceModel::NaturalSound;
            break;
        case EForge2AttenuationFalloff::LogReverse:
            AttSettings.FalloffMode = EAttenuationDistanceModel::LogReverse;
            break;
        case EForge2AttenuationFalloff::Custom:
            AttSettings.FalloffMode = EAttenuationDistanceModel::Custom;
            break;
    }

    // Distance settings
    AttSettings.AttenuationShapeExtents = FVector(Settings.InnerRadius);
    AttSettings.FalloffDistance = Settings.FalloffDistance;
    AttSettings.dBAttenuationAtMax = Settings.dBAttenuationAtMax;

    // Attenuation and spatialization
    AttSettings.bAttenuate = Settings.bAttenuate;
    AttSettings.bSpatialize = Settings.bSpatialize;

    // Spatialization mode
    switch (Settings.SpatializationMode)
    {
        case EForge2SpatializationMode::Default:
            AttSettings.SpatializationAlgorithm = ESoundSpatializationAlgorithm::SPATIALIZATION_Default;
            break;
        case EForge2SpatializationMode::HRTF:
            AttSettings.SpatializationAlgorithm = ESoundSpatializationAlgorithm::SPATIALIZATION_HRTF;
            break;
        default:
            AttSettings.SpatializationAlgorithm = ESoundSpatializationAlgorithm::SPATIALIZATION_Default;
            break;
    }

    // Focus settings
    AttSettings.bEnableListenerFocus = Settings.bEnableFocusInterpolation;
    AttSettings.FocusAzimuth = Settings.FocusAzimuth;
    AttSettings.NonFocusAzimuth = Settings.NonFocusAzimuth;
    AttSettings.FocusDistanceScale = Settings.FocusDistanceScale;
    AttSettings.NonFocusDistanceScale = Settings.NonFocusDistanceScale;
    AttSettings.FocusVolumeAttenuation = Settings.FocusVolumeAttenuation;
    AttSettings.NonFocusVolumeAttenuation = Settings.NonFocusVolumeAttenuation;

    // Priority
    AttSettings.PriorityAttenuationMin = Settings.PriorityAttenuationMin;
    AttSettings.PriorityAttenuationMax = Settings.PriorityAttenuationMax;
    AttSettings.ManualPriorityAttenuation = Settings.ManualPriorityAttenuation;
}

void UForge2AudioManager::ConfigureOcclusion(USoundAttenuation* Attenuation,
                                               const FForge2OcclusionSettings& Settings)
{
    if (!Attenuation) return;

    FSoundAttenuationSettings& AttSettings = Attenuation->Attenuation;

    AttSettings.bEnableOcclusion = Settings.Mode != EForge2OcclusionMode::None;
    AttSettings.OcclusionLowPassFilterFrequency = Settings.LowPassFilterFrequency;
    AttSettings.OcclusionVolumeAttenuation = Settings.VolumeAttenuation;
    AttSettings.OcclusionInterpolationTime = Settings.InterpolationTime;
    AttSettings.bUseComplexCollisionForOcclusion = Settings.bUseComplexCollision;

    if (Settings.Mode == EForge2OcclusionMode::Raycast)
    {
        AttSettings.OcclusionTraceChannel = ECC_Visibility;
    }
}

USoundClass* UForge2AudioManager::CreateSoundClass(const FString& ClassName, EForge2SoundType SoundType)
{
#if WITH_EDITOR
    USoundClass* SoundClass = NewObject<USoundClass>(GetTransientPackage(), *ClassName, RF_Public | RF_Standalone);
    if (SoundClass)
    {
        // Configure properties based on sound type
        switch (SoundType)
        {
            case EForge2SoundType::Music:
                SoundClass->Properties.Volume = 1.0f;
                SoundClass->Properties.Pitch = 1.0f;
                SoundClass->Properties.StereoBleed = 0.0f;
                SoundClass->Properties.LFEBleed = 0.25f;
                SoundClass->Properties.VoiceCenterChannelVolume = 0.0f;
                SoundClass->Properties.bApplyAmbientVolumes = false;
                SoundClass->Properties.bCenterChannelOnly = false;
                SoundClass->Properties.bIsUISound = true;
                SoundClass->Properties.bIsMusic = true;
                break;

            case EForge2SoundType::Dialogue:
                SoundClass->Properties.Volume = 1.0f;
                SoundClass->Properties.Pitch = 1.0f;
                SoundClass->Properties.StereoBleed = 0.0f;
                SoundClass->Properties.LFEBleed = 0.0f;
                SoundClass->Properties.VoiceCenterChannelVolume = 1.0f;
                SoundClass->Properties.bApplyAmbientVolumes = false;
                SoundClass->Properties.bCenterChannelOnly = true;
                SoundClass->Properties.bIsUISound = false;
                break;

            case EForge2SoundType::UI:
                SoundClass->Properties.Volume = 1.0f;
                SoundClass->Properties.Pitch = 1.0f;
                SoundClass->Properties.StereoBleed = 0.0f;
                SoundClass->Properties.LFEBleed = 0.0f;
                SoundClass->Properties.bApplyAmbientVolumes = false;
                SoundClass->Properties.bIsUISound = true;
                SoundClass->Properties.bAlwaysPlay = true;
                break;

            case EForge2SoundType::Weapon:
                SoundClass->Properties.Volume = 1.0f;
                SoundClass->Properties.Pitch = 1.0f;
                SoundClass->Properties.LFEBleed = 0.5f;
                SoundClass->Properties.bApplyAmbientVolumes = true;
                break;

            case EForge2SoundType::Footstep:
                SoundClass->Properties.Volume = 0.8f;
                SoundClass->Properties.Pitch = 1.0f;
                SoundClass->Properties.LFEBleed = 0.1f;
                SoundClass->Properties.bApplyAmbientVolumes = true;
                break;

            case EForge2SoundType::Ambient:
                SoundClass->Properties.Volume = 0.7f;
                SoundClass->Properties.Pitch = 1.0f;
                SoundClass->Properties.LFEBleed = 0.2f;
                SoundClass->Properties.bApplyAmbientVolumes = true;
                break;

            default:
                SoundClass->Properties.Volume = 1.0f;
                SoundClass->Properties.Pitch = 1.0f;
                SoundClass->Properties.bApplyAmbientVolumes = true;
                break;
        }
        
        // Store in our registry
        RegisteredSoundClasses.Add(SoundType, SoundClass);
    }
    return SoundClass;
#else
    return nullptr;
#endif
}

USoundMix* UForge2AudioManager::CreateSoundMix(const FString& MixName, const FForge2SoundMixSettings& Settings)
{
#if WITH_EDITOR
    USoundMix* SoundMix = NewObject<USoundMix>(GetTransientPackage(), *MixName, RF_Public | RF_Standalone);
    if (SoundMix)
    {
        SoundMix->FadeInTime = Settings.FadeInTime;
        SoundMix->FadeOutTime = Settings.FadeOutTime;
        SoundMix->Duration = -1.0f; // Infinite duration until popped
        SoundMix->bApplyEQ = Settings.bApplyEQ;
        
        // Configure EQ settings if enabled
        if (Settings.bApplyEQ)
        {
            SoundMix->EQSettings.FrequencyCenter0 = 250.0f;  // Low band center
            SoundMix->EQSettings.Gain0 = Settings.EQLowFrequencyGain;
            SoundMix->EQSettings.Bandwidth0 = 1.0f;
            
            SoundMix->EQSettings.FrequencyCenter1 = 1000.0f;  // Mid band center
            SoundMix->EQSettings.Gain1 = Settings.EQMidFrequencyGain;
            SoundMix->EQSettings.Bandwidth1 = 1.0f;
            
            SoundMix->EQSettings.FrequencyCenter2 = 4000.0f;  // High band center  
            SoundMix->EQSettings.Gain2 = Settings.EQHighFrequencyGain;
            SoundMix->EQSettings.Bandwidth2 = 1.0f;
            
            SoundMix->EQSettings.FrequencyCenter3 = 8000.0f;  // Very high band
            SoundMix->EQSettings.Gain3 = Settings.EQHighFrequencyGain;
            SoundMix->EQSettings.Bandwidth3 = 1.0f;
        }

        // Configure sound class adjustments based on VolumeMultipliers map
        for (const auto& Pair : Settings.VolumeMultipliers)
        {
            USoundClass** FoundClass = RegisteredSoundClasses.Find(Pair.Key);
            if (FoundClass && *FoundClass)
            {
                FSoundClassAdjuster Adjuster;
                Adjuster.SoundClassObject = *FoundClass;
                Adjuster.VolumeAdjuster = Pair.Value;
                Adjuster.PitchAdjuster = 1.0f;
                Adjuster.bApplyToChildren = true;
                
                SoundMix->SoundClassEffects.Add(Adjuster);
            }
        }
        
        // Apply master volume adjustment
        if (Settings.MasterVolume != 1.0f)
        {
            // Create a master adjustment that affects all sounds
            FSoundClassAdjuster MasterAdjuster;
            MasterAdjuster.VolumeAdjuster = Settings.MasterVolume;
            MasterAdjuster.PitchAdjuster = 1.0f;
            MasterAdjuster.bApplyToChildren = true;
            // Note: SoundClassObject left null applies to all
            SoundMix->SoundClassEffects.Add(MasterAdjuster);
        }
    }
    return SoundMix;
#else
    return nullptr;
#endif
}

void UForge2AudioManager::PushSoundMix(USoundMix* SoundMix)
{
    if (!SoundMix || !GetWorld()) return;

    UGameplayStatics::PushSoundMixModifier(GetWorld(), SoundMix);
}

void UForge2AudioManager::PopSoundMix(USoundMix* SoundMix)
{
    if (!SoundMix || !GetWorld()) return;

    UGameplayStatics::PopSoundMixModifier(GetWorld(), SoundMix);
}

void UForge2AudioManager::SetSoundClassVolume(EForge2SoundType SoundType, float Volume)
{
    if (!GetWorld()) return;
    
    // Find the sound class for this type
    USoundClass** FoundClass = RegisteredSoundClasses.Find(SoundType);
    if (FoundClass && *FoundClass)
    {
        // Get audio device and set the sound class volume
        FAudioDeviceHandle AudioDevice = GetWorld()->GetAudioDevice();
        if (AudioDevice.IsValid())
        {
            AudioDevice->SetSoundClassVolume(*FoundClass, Volume);
        }
    }
    else
    {
        // Fallback: Create a temporary sound mix to adjust volume
        USoundMix* TempMix = NewObject<USoundMix>();
        if (TempMix)
        {
            TempMix->FadeInTime = 0.1f;
            TempMix->FadeOutTime = 0.1f;
            
            // Try to find a matching global sound class by name
            FString ClassName;
            switch (SoundType)
            {
                case EForge2SoundType::Music: ClassName = TEXT("Music"); break;
                case EForge2SoundType::SFX: ClassName = TEXT("SFX"); break;
                case EForge2SoundType::Dialogue: ClassName = TEXT("Voice"); break;
                case EForge2SoundType::Ambient: ClassName = TEXT("Ambient"); break;
                case EForge2SoundType::UI: ClassName = TEXT("UI"); break;
                case EForge2SoundType::Weapon: ClassName = TEXT("Weapon"); break;
                case EForge2SoundType::Footstep: ClassName = TEXT("Foley"); break;
                default: ClassName = TEXT("Master"); break;
            }
            
            // Store volume setting for later use
            SoundTypeVolumes.Add(SoundType, Volume);
            
            // Apply via gameplay statics as fallback
            UGameplayStatics::SetSoundMixClassOverride(GetWorld(), TempMix, nullptr, Volume, 1.0f, 0.1f, true);
            UGameplayStatics::PushSoundMixModifier(GetWorld(), TempMix);
        }
    }
}

void UForge2AudioManager::SetupMusicTrack(const FForge2MusicTrack& Track)
{
    // Stop current music
    StopMusic(0.5f);

    CurrentMusicTrack = Track;
    MusicLayerComponents.Empty();

    // Create audio components for each layer
    for (const FForge2MusicLayer& Layer : Track.Layers)
    {
        if (Layer.SoundWave.IsValid())
        {
            UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetOwner());
            if (AudioComp)
            {
                AudioComp->SetSound(Layer.SoundWave.LoadSynchronous());
                AudioComp->bAutoActivate = false;
                AudioComp->bIsUISound = true;  // Not affected by distance
                AudioComp->VolumeMultiplier = Layer.bEnabled ? Layer.BaseVolume : 0.0f;
                AudioComp->RegisterComponent();
                MusicLayerComponents.Add(AudioComp);
            }
        }
    }
}

void UForge2AudioManager::PlayMusic()
{
    for (UAudioComponent* Comp : MusicLayerComponents)
    {
        if (Comp && !Comp->IsPlaying())
        {
            Comp->Play();
        }
    }

    ApplyMusicState(CurrentMusicTrack.DefaultState, 0.0f);
}

void UForge2AudioManager::StopMusic(float FadeOutTime)
{
    for (UAudioComponent* Comp : MusicLayerComponents)
    {
        if (Comp && Comp->IsPlaying())
        {
            if (FadeOutTime > 0.0f)
            {
                Comp->FadeOut(FadeOutTime, 0.0f);
            }
            else
            {
                Comp->Stop();
            }
        }
    }
}

void UForge2AudioManager::SetMusicState(EForge2MusicState NewState)
{
    if (NewState == CurrentMusicState) return;

    FForge2MusicTransitionRule* Rule = FindTransitionRule(CurrentMusicState, NewState);
    float TransitionTime = Rule ? Rule->TransitionDuration : 1.0f;

    // Play transition stinger if defined
    if (Rule && Rule->TransitionStinger.IsValid())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Rule->TransitionStinger.LoadSynchronous());
    }

    ApplyMusicState(NewState, TransitionTime);
    CurrentMusicState = NewState;
}

void UForge2AudioManager::ApplyMusicState(EForge2MusicState State, float TransitionTime)
{
    for (int32 i = 0; i < CurrentMusicTrack.Layers.Num() && i < MusicLayerComponents.Num(); ++i)
    {
        const FForge2MusicLayer& Layer = CurrentMusicTrack.Layers[i];
        UAudioComponent* Comp = MusicLayerComponents[i];

        if (!Comp) continue;

        bool bShouldBeActive = Layer.ActiveStates.Contains(State);
        float TargetVolume = bShouldBeActive ? Layer.BaseVolume : 0.0f;

        if (TransitionTime > 0.0f)
        {
            Comp->AdjustVolume(TransitionTime, TargetVolume);
        }
        else
        {
            Comp->SetVolumeMultiplier(TargetVolume);
        }
    }
}

void UForge2AudioManager::SetLayerVolume(const FString& LayerName, float Volume, float FadeTime)
{
    for (int32 i = 0; i < CurrentMusicTrack.Layers.Num() && i < MusicLayerComponents.Num(); ++i)
    {
        if (CurrentMusicTrack.Layers[i].LayerName == LayerName)
        {
            UAudioComponent* Comp = MusicLayerComponents[i];
            if (Comp)
            {
                if (FadeTime > 0.0f)
                {
                    Comp->AdjustVolume(FadeTime, Volume);
                }
                else
                {
                    Comp->SetVolumeMultiplier(Volume);
                }
            }
            break;
        }
    }
}

void UForge2AudioManager::EnableLayer(const FString& LayerName, bool bEnable, float FadeTime)
{
    for (int32 i = 0; i < CurrentMusicTrack.Layers.Num(); ++i)
    {
        if (CurrentMusicTrack.Layers[i].LayerName == LayerName)
        {
            CurrentMusicTrack.Layers[i].bEnabled = bEnable;

            if (i < MusicLayerComponents.Num())
            {
                float TargetVolume = bEnable ? CurrentMusicTrack.Layers[i].BaseVolume : 0.0f;
                SetLayerVolume(LayerName, TargetVolume, FadeTime);
            }
            break;
        }
    }
}

void UForge2AudioManager::AddMusicTransitionRule(const FForge2MusicTransitionRule& Rule)
{
    TransitionRules.Add(Rule);
}

FForge2MusicTransitionRule* UForge2AudioManager::FindTransitionRule(EForge2MusicState From, EForge2MusicState To)
{
    for (FForge2MusicTransitionRule& Rule : TransitionRules)
    {
        if (Rule.FromState == From && Rule.ToState == To)
        {
            return &Rule;
        }
    }
    return nullptr;
}

void UForge2AudioManager::PlayDialogue(const FForge2DialogueSettings& Dialogue)
{
    // Stop current dialogue
    StopDialogue();

    if (!Dialogue.DialogueWave.IsValid()) return;

    // Apply ducking
    if (Dialogue.bDuckMusic)
    {
        for (UAudioComponent* Comp : MusicLayerComponents)
        {
            if (Comp)
            {
                Comp->AdjustVolume(0.5f, Dialogue.MusicDuckAmount);
            }
        }
    }

    // Play dialogue
    CurrentDialogueComponent = UGameplayStatics::SpawnSound2D(
        GetWorld(),
        Dialogue.DialogueWave.LoadSynchronous(),
        1.0f,
        1.0f
    );

    // Set up completion delegate to restore ducking and process queue
    if (CurrentDialogueComponent)
    {
        CurrentDialogueComponent->OnAudioFinished.AddDynamic(this, &UForge2AudioManager::ProcessDialogueQueue);
    }
}

void UForge2AudioManager::StopDialogue()
{
    if (CurrentDialogueComponent && CurrentDialogueComponent->IsPlaying())
    {
        CurrentDialogueComponent->Stop();
    }

    // Restore music volume
    for (int32 i = 0; i < CurrentMusicTrack.Layers.Num() && i < MusicLayerComponents.Num(); ++i)
    {
        if (CurrentMusicTrack.Layers[i].bEnabled && MusicLayerComponents[i])
        {
            MusicLayerComponents[i]->AdjustVolume(0.5f, CurrentMusicTrack.Layers[i].BaseVolume);
        }
    }
}

void UForge2AudioManager::QueueDialogue(const FForge2DialogueSettings& Dialogue)
{
    DialogueQueue.Add(Dialogue);

    // If nothing playing, start the queue
    if (!CurrentDialogueComponent || !CurrentDialogueComponent->IsPlaying())
    {
        ProcessDialogueQueue();
    }
}

void UForge2AudioManager::ClearDialogueQueue()
{
    DialogueQueue.Empty();
}

void UForge2AudioManager::ProcessDialogueQueue()
{
    if (DialogueQueue.Num() > 0)
    {
        FForge2DialogueSettings NextDialogue = DialogueQueue[0];
        DialogueQueue.RemoveAt(0);
        PlayDialogue(NextDialogue);
    }
    else
    {
        // Queue empty, restore ducking
        StopDialogue();
    }
}

void UForge2AudioManager::EnterAmbientZone(const FForge2AmbientZone& Zone)
{
    TArray<UAudioComponent*>& ZoneComponents = AmbientComponents.FindOrAdd(Zone.ZoneName);

    for (const TSoftObjectPtr<USoundWave>& SoundPtr : Zone.AmbientSounds)
    {
        if (SoundPtr.IsValid())
        {
            UAudioComponent* Comp = UGameplayStatics::SpawnSound2D(
                GetWorld(),
                SoundPtr.LoadSynchronous(),
                FMath::RandRange(Zone.VolumeRange.X, Zone.VolumeRange.Y),
                FMath::RandRange(Zone.PitchRange.X, Zone.PitchRange.Y)
            );

            if (Comp)
            {
                Comp->FadeIn(Zone.CrossfadeTime);
                ZoneComponents.Add(Comp);
            }
        }
    }

    // Apply reverb
    if (Zone.ReverbOverride.bEnabled)
    {
        SetReverbSettings(Zone.ReverbOverride);
    }
}

void UForge2AudioManager::ExitAmbientZone(const FString& ZoneName)
{
    TArray<UAudioComponent*>* ZoneComponents = AmbientComponents.Find(ZoneName);
    if (ZoneComponents)
    {
        for (UAudioComponent* Comp : *ZoneComponents)
        {
            if (Comp)
            {
                Comp->FadeOut(2.0f, 0.0f);
            }
        }
        AmbientComponents.Remove(ZoneName);
    }
}

void UForge2AudioManager::SetReverbSettings(const FForge2ReverbSettings& Settings)
{
    if (!GetWorld()) return;
    
    // Get the audio device for this world
    FAudioDeviceHandle AudioDevice = GetWorld()->GetAudioDevice();
    if (!AudioDevice.IsValid()) return;
    
    // Create or get reverb effect
    UReverbEffect* ReverbEffect = NewObject<UReverbEffect>(GetTransientPackage());
    if (ReverbEffect)
    {
        // Configure the reverb effect with our settings
        ReverbEffect->Density = Settings.Density;
        ReverbEffect->Diffusion = Settings.Diffusion;
        ReverbEffect->Gain = Settings.Gain;
        ReverbEffect->GainHF = Settings.GainHF;
        ReverbEffect->DecayTime = Settings.DecayTime;
        ReverbEffect->DecayHFRatio = Settings.DecayHFRatio;
        ReverbEffect->ReflectionsGain = Settings.ReflectionsGain;
        ReverbEffect->ReflectionsDelay = Settings.ReflectionsDelay;
        ReverbEffect->LateGain = Settings.LateReverbGain;
        ReverbEffect->LateDelay = Settings.LateReverbDelay;
        ReverbEffect->AirAbsorptionGainHF = Settings.AirAbsorptionGainHF;
        ReverbEffect->RoomRolloffFactor = Settings.RoomRolloffFactor;
        
        // Apply to audio device via world settings
        AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();
        if (WorldSettings)
        {
            WorldSettings->DefaultReverbSettings.ReverbEffect = ReverbEffect;
            WorldSettings->DefaultReverbSettings.Volume = Settings.bEnabled ? 1.0f : 0.0f;
            WorldSettings->DefaultReverbSettings.FadeTime = Settings.FadeTime;
        }
        
        // Also apply directly via activation
        FReverbSettings NewReverbSettings;
        NewReverbSettings.ReverbEffect = ReverbEffect;
        NewReverbSettings.Volume = Settings.bEnabled ? 1.0f : 0.0f;
        NewReverbSettings.FadeTime = Settings.FadeTime;
        
        AudioDevice->ActivateReverbEffect(ReverbEffect, FName(TEXT("Forge2Reverb")), Settings.Priority, 
            Settings.bEnabled ? 1.0f : 0.0f, Settings.FadeTime);
    }
    
    // Store current settings
    CurrentReverbSettings = Settings;
}

FForge2AttenuationSettings UForge2AudioManager::GetDefaultAttenuation(EForge2SoundType SoundType)
{
    switch (SoundType)
    {
        case EForge2SoundType::Footstep:
            return UForge2AudioUtility::CreateSmallAttenuation();

        case EForge2SoundType::Weapon:
            return UForge2AudioUtility::CreateLargeAttenuation();

        case EForge2SoundType::Ambient:
            return UForge2AudioUtility::CreateMediumAttenuation();

        default:
            return UForge2AudioUtility::CreateMediumAttenuation();
    }
}

FForge2ReverbSettings UForge2AudioManager::GetReverbPreset(const FString& PresetName)
{
    if (PresetName == TEXT("Indoor"))
    {
        return UForge2AudioUtility::CreateIndoorReverb();
    }
    else if (PresetName == TEXT("Outdoor"))
    {
        return UForge2AudioUtility::CreateOutdoorReverb();
    }
    else if (PresetName == TEXT("Cave"))
    {
        return UForge2AudioUtility::CreateCaveReverb();
    }
    else if (PresetName == TEXT("Cathedral"))
    {
        return UForge2AudioUtility::CreateCathedralReverb();
    }

    return FForge2ReverbSettings();
}

// UForge2AudioUtility Implementation
FForge2AttenuationSettings UForge2AudioUtility::CreateSmallAttenuation()
{
    FForge2AttenuationSettings Settings;
    Settings.Shape = EForge2AttenuationShape::Sphere;
    Settings.Falloff = EForge2AttenuationFalloff::NaturalSound;
    Settings.InnerRadius = 100.0f;
    Settings.FalloffDistance = 800.0f;
    Settings.dBAttenuationAtMax = -60.0f;
    return Settings;
}

FForge2AttenuationSettings UForge2AudioUtility::CreateMediumAttenuation()
{
    FForge2AttenuationSettings Settings;
    Settings.Shape = EForge2AttenuationShape::Sphere;
    Settings.Falloff = EForge2AttenuationFalloff::NaturalSound;
    Settings.InnerRadius = 400.0f;
    Settings.FalloffDistance = 3600.0f;
    Settings.dBAttenuationAtMax = -60.0f;
    return Settings;
}

FForge2AttenuationSettings UForge2AudioUtility::CreateLargeAttenuation()
{
    FForge2AttenuationSettings Settings;
    Settings.Shape = EForge2AttenuationShape::Sphere;
    Settings.Falloff = EForge2AttenuationFalloff::NaturalSound;
    Settings.InnerRadius = 1000.0f;
    Settings.FalloffDistance = 10000.0f;
    Settings.dBAttenuationAtMax = -60.0f;
    return Settings;
}

FForge2ReverbSettings UForge2AudioUtility::CreateIndoorReverb()
{
    FForge2ReverbSettings Settings;
    Settings.bEnabled = true;
    Settings.Density = 1.0f;
    Settings.Diffusion = 1.0f;
    Settings.Gain = 0.32f;
    Settings.GainHF = 0.89f;
    Settings.DecayTime = 1.49f;
    Settings.DecayHFRatio = 0.83f;
    Settings.ReflectionsGain = 0.05f;
    Settings.ReflectionsDelay = 0.007f;
    Settings.LateReverbGain = 1.26f;
    Settings.LateReverbDelay = 0.011f;
    return Settings;
}

FForge2ReverbSettings UForge2AudioUtility::CreateOutdoorReverb()
{
    FForge2ReverbSettings Settings;
    Settings.bEnabled = true;
    Settings.Density = 0.21f;
    Settings.Diffusion = 1.0f;
    Settings.Gain = 0.32f;
    Settings.GainHF = 0.77f;
    Settings.DecayTime = 0.54f;
    Settings.DecayHFRatio = 0.66f;
    Settings.ReflectionsGain = 0.79f;
    Settings.ReflectionsDelay = 0.002f;
    Settings.LateReverbGain = 0.56f;
    Settings.LateReverbDelay = 0.006f;
    return Settings;
}

FForge2ReverbSettings UForge2AudioUtility::CreateCaveReverb()
{
    FForge2ReverbSettings Settings;
    Settings.bEnabled = true;
    Settings.Density = 1.0f;
    Settings.Diffusion = 1.0f;
    Settings.Gain = 0.32f;
    Settings.GainHF = 1.0f;
    Settings.DecayTime = 2.91f;
    Settings.DecayHFRatio = 1.3f;
    Settings.ReflectionsGain = 0.5f;
    Settings.ReflectionsDelay = 0.015f;
    Settings.LateReverbGain = 0.71f;
    Settings.LateReverbDelay = 0.022f;
    return Settings;
}

FForge2ReverbSettings UForge2AudioUtility::CreateCathedralReverb()
{
    FForge2ReverbSettings Settings;
    Settings.bEnabled = true;
    Settings.Density = 1.0f;
    Settings.Diffusion = 1.0f;
    Settings.Gain = 0.32f;
    Settings.GainHF = 0.89f;
    Settings.DecayTime = 3.92f;
    Settings.DecayHFRatio = 0.7f;
    Settings.ReflectionsGain = 0.25f;
    Settings.ReflectionsDelay = 0.02f;
    Settings.LateReverbGain = 1.26f;
    Settings.LateReverbDelay = 0.04f;
    return Settings;
}

FForge2SoundMixSettings UForge2AudioUtility::CreateCombatMix()
{
    FForge2SoundMixSettings Settings;
    Settings.MixName = TEXT("Combat");
    Settings.MasterVolume = 1.0f;
    Settings.VolumeMultipliers.Add(EForge2SoundType::Music, 0.7f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::Ambient, 0.5f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::Weapon, 1.2f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::SFX, 1.0f);
    Settings.FadeInTime = 0.5f;
    Settings.FadeOutTime = 1.0f;
    return Settings;
}

FForge2SoundMixSettings UForge2AudioUtility::CreateCutsceneMix()
{
    FForge2SoundMixSettings Settings;
    Settings.MixName = TEXT("Cutscene");
    Settings.MasterVolume = 1.0f;
    Settings.VolumeMultipliers.Add(EForge2SoundType::Dialogue, 1.2f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::Music, 0.8f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::Ambient, 0.3f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::SFX, 0.5f);
    Settings.FadeInTime = 0.3f;
    Settings.FadeOutTime = 0.5f;
    return Settings;
}

FForge2SoundMixSettings UForge2AudioUtility::CreatePausedMix()
{
    FForge2SoundMixSettings Settings;
    Settings.MixName = TEXT("Paused");
    Settings.MasterVolume = 0.3f;
    Settings.VolumeMultipliers.Add(EForge2SoundType::Music, 0.5f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::Ambient, 0.1f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::SFX, 0.0f);
    Settings.VolumeMultipliers.Add(EForge2SoundType::UI, 1.0f);
    Settings.FadeInTime = 0.2f;
    Settings.FadeOutTime = 0.2f;
    Settings.bApplyEQ = true;
    Settings.EQLowFrequencyGain = 0.8f;
    Settings.EQMidFrequencyGain = 0.9f;
    Settings.EQHighFrequencyGain = 0.7f;
    return Settings;
}
