// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Game Framework Classes Implementation

#include "Forge2GameFramework.h"
#include "Forge2Components.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "AudioDevice.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

// ============================================================================
// Game Mode
// ============================================================================

AMcpGameMode::AMcpGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentDifficulty = DefaultDifficulty;
}

void AMcpGameMode::BeginPlay()
{
    Super::BeginPlay();
    CurrentDifficulty = DefaultDifficulty;
}

void AMcpGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Auto-save timer
    if (bEnableAutoSave)
    {
        AutoSaveTimer += DeltaSeconds;
        if (AutoSaveTimer >= AutoSaveInterval)
        {
            TriggerAutoSave();
            AutoSaveTimer = 0.0f;
        }
    }
}

void AMcpGameMode::SetDifficulty(int32 NewDifficulty)
{
    CurrentDifficulty = FMath::Clamp(NewDifficulty, 0, 3);

    // Notify game state
    AMcpGameState* GS = GetGameState<AMcpGameState>();
    if (GS)
    {
        GS->SetGlobalVariable(FName("Difficulty"), FString::FromInt(CurrentDifficulty));
    }
}

void AMcpGameMode::TriggerAutoSave()
{
    UMcpGameInstance* GI = Cast<UMcpGameInstance>(GetGameInstance());
    if (GI)
    {
        GI->SaveGame();
    }
}

AActor* AMcpGameMode::GetRandomSpawnPoint() const
{
    if (SpawnPoints.Num() > 0)
    {
        return SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];
    }
    return nullptr;
}

AActor* AMcpGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    AActor* RandomSpawn = GetRandomSpawnPoint();
    if (RandomSpawn)
    {
        return RandomSpawn;
    }
    return Super::ChoosePlayerStart_Implementation(Player);
}

// ============================================================================
// Game State
// ============================================================================

AMcpGameState::AMcpGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentPhase = FName("Playing");
    CurrentWeather = FName("Clear");
}

void AMcpGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMcpGameState, CurrentPhase);
    DOREPLIFETIME(AMcpGameState, GameTime);
    DOREPLIFETIME(AMcpGameState, TimeOfDay);
    DOREPLIFETIME(AMcpGameState, DayNumber);
    DOREPLIFETIME(AMcpGameState, CurrentWeather);
    DOREPLIFETIME(AMcpGameState, GlobalVariables);
    DOREPLIFETIME(AMcpGameState, ActiveWorldEvents);
}

void AMcpGameState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Advance game time
    if (HasAuthority())
    {
        GameTime += DeltaSeconds;

        // Advance time of day based on time scale
        float HoursToAdd = (DeltaSeconds / 3600.0f) * TimeScale;
        AdvanceTime(HoursToAdd);
    }
}

void AMcpGameState::SetGamePhase(FName NewPhase)
{
    if (HasAuthority() && CurrentPhase != NewPhase)
    {
        FName OldPhase = CurrentPhase;
        CurrentPhase = NewPhase;
        OnGamePhaseChanged.Broadcast(NewPhase, OldPhase);
    }
}

void AMcpGameState::SetTimeOfDay(float NewTime)
{
    if (HasAuthority())
    {
        TimeOfDay = FMath::Fmod(NewTime, 24.0f);
        if (TimeOfDay < 0)
        {
            TimeOfDay += 24.0f;
        }
    }
}

void AMcpGameState::AdvanceTime(float Hours)
{
    if (HasAuthority())
    {
        float NewTime = TimeOfDay + Hours;
        int32 DaysAdvanced = FMath::FloorToInt(NewTime / 24.0f);

        TimeOfDay = FMath::Fmod(NewTime, 24.0f);
        if (TimeOfDay < 0)
        {
            TimeOfDay += 24.0f;
            DaysAdvanced--;
        }

        DayNumber += DaysAdvanced;
    }
}

void AMcpGameState::SetWeather(FName WeatherId)
{
    if (HasAuthority())
    {
        CurrentWeather = WeatherId;
    }
}

void AMcpGameState::SetGlobalVariable(FName VarName, const FString& Value)
{
    if (HasAuthority())
    {
        GlobalVariables.Add(VarName, Value);
    }
}

FString AMcpGameState::GetGlobalVariable(FName VarName) const
{
    const FString* Value = GlobalVariables.Find(VarName);
    return Value ? *Value : FString();
}

void AMcpGameState::TriggerWorldEvent(FName EventId)
{
    if (HasAuthority() && !ActiveWorldEvents.Contains(EventId))
    {
        ActiveWorldEvents.Add(EventId);
        OnGameEventTriggered.Broadcast(EventId);
    }
}

void AMcpGameState::EndWorldEvent(FName EventId)
{
    if (HasAuthority())
    {
        ActiveWorldEvents.Remove(EventId);
    }
}

// ============================================================================
// Player State
// ============================================================================

AMcpPlayerState::AMcpPlayerState()
{
}

void AMcpPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMcpPlayerState, DisplayName);
    DOREPLIFETIME(AMcpPlayerState, PlayerLevel);
    DOREPLIFETIME(AMcpPlayerState, ClassName);
    DOREPLIFETIME(AMcpPlayerState, Kills);
    DOREPLIFETIME(AMcpPlayerState, Deaths);
    DOREPLIFETIME(AMcpPlayerState, Assists);
    DOREPLIFETIME(AMcpPlayerState, TeamId);
    DOREPLIFETIME(AMcpPlayerState, PartyId);
    DOREPLIFETIME(AMcpPlayerState, bIsPartyLeader);
    DOREPLIFETIME(AMcpPlayerState, GuildId);
    DOREPLIFETIME(AMcpPlayerState, GuildRank);
}

void AMcpPlayerState::AddKill()
{
    if (HasAuthority())
    {
        Kills++;
    }
}

void AMcpPlayerState::AddDeath()
{
    if (HasAuthority())
    {
        Deaths++;
    }
}

void AMcpPlayerState::AddAssist()
{
    if (HasAuthority())
    {
        Assists++;
    }
}

float AMcpPlayerState::GetKDRatio() const
{
    return Deaths > 0 ? static_cast<float>(Kills) / static_cast<float>(Deaths) : static_cast<float>(Kills);
}

// ============================================================================
// Player Controller
// ============================================================================

AMcpPlayerController::AMcpPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    TargetFOV = DefaultFOV;
}

void AMcpPlayerController::BeginPlay()
{
    Super::BeginPlay();
    TargetFOV = DefaultFOV;
}

void AMcpPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // FOV blending
    if (PlayerCameraManager && CurrentFOVBlendTime < FOVBlendTime)
    {
        CurrentFOVBlendTime += DeltaSeconds;
        float Alpha = FMath::Clamp(CurrentFOVBlendTime / FOVBlendTime, 0.0f, 1.0f);
        float NewFOV = FMath::Lerp(PlayerCameraManager->GetFOVAngle(), TargetFOV, Alpha);
        PlayerCameraManager->SetFOV(NewFOV);
    }
}

void AMcpPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    // Setup enhanced input here if needed
}

void AMcpPlayerController::EnterMenu()
{
    bIsInMenu = true;
    SetShowMouseCursor(true);
    SetInputMode(FInputModeGameAndUI());
}

void AMcpPlayerController::ExitMenu()
{
    bIsInMenu = false;
    SetShowMouseCursor(false);
    SetInputMode(FInputModeGameOnly());
}

void AMcpPlayerController::EnterDialogue()
{
    bIsInDialogue = true;
}

void AMcpPlayerController::ExitDialogue()
{
    bIsInDialogue = false;
}

void AMcpPlayerController::EnterCutscene()
{
    bIsInCutscene = true;
}

void AMcpPlayerController::ExitCutscene()
{
    bIsInCutscene = false;
}

bool AMcpPlayerController::CanPlayerMove() const
{
    return !bIsInMenu && !bIsInDialogue && !bIsInCutscene;
}

void AMcpPlayerController::SetFOV(float NewFOV, float BlendTime)
{
    TargetFOV = NewFOV;
    FOVBlendTime = FMath::Max(BlendTime, 0.01f);
    CurrentFOVBlendTime = 0.0f;
}

AActor* AMcpPlayerController::GetLookAtActor(float MaxDistance) const
{
    if (!PlayerCameraManager)
    {
        return nullptr;
    }

    FVector Start = PlayerCameraManager->GetCameraLocation();
    FVector End = Start + (PlayerCameraManager->GetActorForwardVector() * MaxDistance);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetPawn());

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        return Hit.GetActor();
    }

    return nullptr;
}

bool AMcpPlayerController::TryInteractWithLookedAtActor()
{
    AActor* LookAt = GetLookAtActor(500.0f);
    if (!LookAt)
    {
        return false;
    }

    UMcpInteractionComponent* Interaction = LookAt->FindComponentByClass<UMcpInteractionComponent>();
    if (Interaction)
    {
        return Interaction->TryInteract(GetPawn(), 0);
    }

    return false;
}

// ============================================================================
// Character
// ============================================================================

AMcpCharacter::AMcpCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create components
    CombatComponent = CreateDefaultSubobject<UMcpCombatComponent>(TEXT("CombatComponent"));
    InventoryComponent = CreateDefaultSubobject<UMcpInventoryComponent>(TEXT("InventoryComponent"));
    EquipmentComponent = CreateDefaultSubobject<UMcpEquipmentComponent>(TEXT("EquipmentComponent"));
    StatusEffectComponent = CreateDefaultSubobject<UMcpStatusEffectComponent>(TEXT("StatusEffectComponent"));
    InteractionComponent = CreateDefaultSubobject<UMcpInteractionComponent>(TEXT("InteractionComponent"));
}

void AMcpCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMcpCharacter, CharacterId);
    DOREPLIFETIME(AMcpCharacter, CharacterName);
    DOREPLIFETIME(AMcpCharacter, bIsSprinting);
    DOREPLIFETIME(AMcpCharacter, bIsCrouching);
    DOREPLIFETIME(AMcpCharacter, bIsAiming);
}

void AMcpCharacter::BeginPlay()
{
    Super::BeginPlay();
    UpdateMovementSpeed();
}

void AMcpCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AMcpCharacter::StartSprint()
{
    if (!bIsSprinting && !bIsCrouching)
    {
        bIsSprinting = true;
        UpdateMovementSpeed();
    }
}

void AMcpCharacter::StopSprint()
{
    if (bIsSprinting)
    {
        bIsSprinting = false;
        UpdateMovementSpeed();
    }
}

void AMcpCharacter::ToggleCrouch()
{
    bIsCrouching = !bIsCrouching;
    if (bIsCrouching)
    {
        bIsSprinting = false;
    }
    UpdateMovementSpeed();
}

void AMcpCharacter::StartAiming()
{
    bIsAiming = true;
}

void AMcpCharacter::StopAiming()
{
    bIsAiming = false;
}

bool AMcpCharacter::IsAlive() const
{
    return CombatComponent && !CombatComponent->bIsDead;
}

float AMcpCharacter::GetHealthPercent() const
{
    return CombatComponent ? CombatComponent->GetHealthPercent() : 0.0f;
}

void AMcpCharacter::UpdateMovementSpeed()
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        if (bIsCrouching)
        {
            Movement->MaxWalkSpeed = CrouchSpeed;
        }
        else if (bIsSprinting)
        {
            Movement->MaxWalkSpeed = SprintSpeed;
        }
        else
        {
            Movement->MaxWalkSpeed = WalkSpeed;
        }
    }
}

// ============================================================================
// AI Controller
// ============================================================================

AMcpAIController::AMcpAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMcpAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AMcpAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    HomeLocation = InPawn->GetActorLocation();
}

void AMcpAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Simple AI logic - can be overridden
    if (!CurrentTarget && bCanPatrol && PatrolPoints.Num() > 0)
    {
        // Check for enemies
        AActor* Enemy = FindNearestEnemy();
        if (Enemy && IsInAggroRange(Enemy))
        {
            SetTarget(Enemy);
        }
    }
    else if (CurrentTarget)
    {
        // Check if target is still valid
        if (!IsValid(CurrentTarget) || !IsInLeashRange())
        {
            ClearTarget();
            ReturnToHome();
        }
    }
}

void AMcpAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
}

void AMcpAIController::ClearTarget()
{
    CurrentTarget = nullptr;
}

void AMcpAIController::ReturnToHome()
{
    MoveToLocation(HomeLocation);
}

void AMcpAIController::MoveToNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return;
    }

    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    if (PatrolPoints[CurrentPatrolIndex])
    {
        MoveToActor(PatrolPoints[CurrentPatrolIndex]);
    }
}

bool AMcpAIController::IsInAggroRange(AActor* Actor) const
{
    if (!Actor || !GetPawn())
    {
        return false;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
    return Distance <= AggroRange;
}

bool AMcpAIController::IsInLeashRange() const
{
    if (!GetPawn())
    {
        return false;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), HomeLocation);
    return Distance <= LeashRange;
}

AActor* AMcpAIController::FindNearestEnemy() const
{
    // Get combat component to check team
    UMcpCombatComponent* MyCombat = GetPawn() ? GetPawn()->FindComponentByClass<UMcpCombatComponent>() : nullptr;
    if (!MyCombat)
    {
        return nullptr;
    }

    AActor* NearestEnemy = nullptr;
    float NearestDistance = MAX_FLT;

    // Find all characters in range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMcpCharacter::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetPawn())
        {
            continue;
        }

        UMcpCombatComponent* OtherCombat = Actor->FindComponentByClass<UMcpCombatComponent>();
        if (OtherCombat && OtherCombat->TeamId != MyCombat->TeamId)
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestEnemy = Actor;
            }
        }
    }

    return NearestEnemy;
}

// ============================================================================
// Game Instance
// ============================================================================

UMcpGameInstance::UMcpGameInstance()
{
}

void UMcpGameInstance::Init()
{
    Super::Init();
    LoadSettings();
}

void UMcpGameInstance::Shutdown()
{
    SaveSettings();
    Super::Shutdown();
}

bool UMcpGameInstance::SaveGame(int32 Slot)
{
    int32 TargetSlot = Slot >= 0 ? Slot : CurrentSaveSlot;
    FString SlotName = FString::Printf(TEXT("SaveGame_%d"), TargetSlot);

    UMcpSaveGame* SaveGameInstance = Cast<UMcpSaveGame>(UGameplayStatics::CreateSaveGameObject(UMcpSaveGame::StaticClass()));
    if (!SaveGameInstance)
    {
        return false;
    }

    // Populate save data from current game state
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC)
        {
            SaveGameInstance->PopulateFromPlayer(PC);
        }

        AGameStateBase* GS = World->GetGameState();
        if (GS)
        {
            SaveGameInstance->PopulateFromGameState(GS);
        }
    }

    SaveGameInstance->SaveName = SlotName;
    SaveGameInstance->SaveDateTime = FDateTime::Now();

    return UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}

bool UMcpGameInstance::LoadGame(int32 Slot)
{
    int32 TargetSlot = Slot >= 0 ? Slot : CurrentSaveSlot;
    FString SlotName = FString::Printf(TEXT("SaveGame_%d"), TargetSlot);

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return false;
    }

    UMcpSaveGame* SaveGameInstance = Cast<UMcpSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (!SaveGameInstance)
    {
        return false;
    }

    // Apply save data to current game state
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC)
        {
            SaveGameInstance->ApplyToPlayer(PC);
        }

        AGameStateBase* GS = World->GetGameState();
        if (GS)
        {
            SaveGameInstance->ApplyToGameState(GS);
        }
    }

    CurrentSaveSlot = TargetSlot;
    return true;
}

bool UMcpGameInstance::DeleteSave(int32 Slot)
{
    FString SlotName = FString::Printf(TEXT("SaveGame_%d"), Slot);
    return UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

bool UMcpGameInstance::DoesSaveExist(int32 Slot) const
{
    FString SlotName = FString::Printf(TEXT("SaveGame_%d"), Slot);
    return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

void UMcpGameInstance::ApplyAudioSettings()
{
    // Apply volume settings to audio mixer using UE5 Audio Modulation system
    if (FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice())
    {
        // Set master volume
        AudioDevice->SetTransientPrimaryVolume(MasterVolume);
        
        // Apply sound class volumes through sound mix
        if (USoundMix* GameSoundMix = Cast<USoundMix>(StaticLoadObject(USoundMix::StaticClass(), nullptr, TEXT("/Game/Audio/Mixes/GameSoundMix.GameSoundMix"))))
        {
            // Music volume adjustment
            FSoundClassAdjuster MusicAdjuster;
            MusicAdjuster.SoundClassObject = Cast<USoundClass>(StaticLoadObject(USoundClass::StaticClass(), nullptr, TEXT("/Game/Audio/Classes/Music.Music")));
            MusicAdjuster.VolumeAdjuster = MusicVolume;
            MusicAdjuster.bApplyToChildren = true;
            
            // SFX volume adjustment  
            FSoundClassAdjuster SFXAdjuster;
            SFXAdjuster.SoundClassObject = Cast<USoundClass>(StaticLoadObject(USoundClass::StaticClass(), nullptr, TEXT("/Game/Audio/Classes/SFX.SFX")));
            SFXAdjuster.VolumeAdjuster = SFXVolume;
            SFXAdjuster.bApplyToChildren = true;
            
            // Voice/Dialogue volume adjustment
            FSoundClassAdjuster VoiceAdjuster;
            VoiceAdjuster.SoundClassObject = Cast<USoundClass>(StaticLoadObject(USoundClass::StaticClass(), nullptr, TEXT("/Game/Audio/Classes/Voice.Voice")));
            VoiceAdjuster.VolumeAdjuster = VoiceVolume;
            VoiceAdjuster.bApplyToChildren = true;
            
            // Push the sound mix
            AudioDevice->PushSoundMixModifier(GameSoundMix);
        }
    }
}

void UMcpGameInstance::SetPersistentVariable(FName VarName, const FString& Value)
{
    PersistentVariables.Add(VarName, Value);
}

FString UMcpGameInstance::GetPersistentVariable(FName VarName) const
{
    const FString* Value = PersistentVariables.Find(VarName);
    return Value ? *Value : FString();
}

void UMcpGameInstance::LoadSettings()
{
    // Load settings from GameUserSettings and custom config
    if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
    {
        UserSettings->LoadSettings();
        
        // Apply graphics settings
        UserSettings->ApplySettings(false);
    }
    
    // Load custom game settings from config file
    const FString ConfigSection = TEXT("GameSettings");
    const FString ConfigFilePath = FPaths::GeneratedConfigDir() + TEXT("GameSettings.ini");
    
    if (FPaths::FileExists(ConfigFilePath))
    {
        GConfig->GetFloat(*ConfigSection, TEXT("MasterVolume"), MasterVolume, ConfigFilePath);
        GConfig->GetFloat(*ConfigSection, TEXT("MusicVolume"), MusicVolume, ConfigFilePath);
        GConfig->GetFloat(*ConfigSection, TEXT("SFXVolume"), SFXVolume, ConfigFilePath);
        GConfig->GetFloat(*ConfigSection, TEXT("VoiceVolume"), VoiceVolume, ConfigFilePath);
        
        // Load persistent variables
        TArray<FString> PersistentKeys;
        GConfig->GetArray(*ConfigSection, TEXT("PersistentVariableKeys"), PersistentKeys, ConfigFilePath);
        
        for (const FString& Key : PersistentKeys)
        {
            FString Value;
            if (GConfig->GetString(*ConfigSection, *FString::Printf(TEXT("PV_%s"), *Key), Value, ConfigFilePath))
            {
                PersistentVariables.Add(FName(*Key), Value);
            }
        }
    }
    
    // Apply audio settings after loading
    ApplyAudioSettings();
}

void UMcpGameInstance::SaveSettings()
{
    // Save engine/graphics settings
    if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
    {
        UserSettings->SaveSettings();
    }
    
    // Save custom game settings to config file
    const FString ConfigSection = TEXT("GameSettings");
    const FString ConfigFilePath = FPaths::GeneratedConfigDir() + TEXT("GameSettings.ini");
    
    // Ensure directory exists
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(ConfigFilePath), true);
    
    // Save audio volumes
    GConfig->SetFloat(*ConfigSection, TEXT("MasterVolume"), MasterVolume, ConfigFilePath);
    GConfig->SetFloat(*ConfigSection, TEXT("MusicVolume"), MusicVolume, ConfigFilePath);
    GConfig->SetFloat(*ConfigSection, TEXT("SFXVolume"), SFXVolume, ConfigFilePath);
    GConfig->SetFloat(*ConfigSection, TEXT("VoiceVolume"), VoiceVolume, ConfigFilePath);
    
    // Save persistent variables
    TArray<FString> PersistentKeys;
    for (const auto& Pair : PersistentVariables)
    {
        FString KeyStr = Pair.Key.ToString();
        PersistentKeys.Add(KeyStr);
        GConfig->SetString(*ConfigSection, *FString::Printf(TEXT("PV_%s"), *KeyStr), *Pair.Value, ConfigFilePath);
    }
    GConfig->SetArray(*ConfigSection, TEXT("PersistentVariableKeys"), PersistentKeys, ConfigFilePath);
    
    // Flush config to disk
    GConfig->Flush(false, ConfigFilePath);
}

// ============================================================================
// Save Game
// ============================================================================

UMcpSaveGame::UMcpSaveGame()
{
}

void UMcpSaveGame::PopulateFromPlayer(APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    APawn* Pawn = PlayerController->GetPawn();
    if (!Pawn)
    {
        return;
    }

    AMcpCharacter* Character = Cast<AMcpCharacter>(Pawn);
    if (Character)
    {
        CharacterData.CharacterId = Character->CharacterId;
        CharacterData.CharacterName = Character->CharacterName;
        CharacterData.WorldTransform = Character->GetActorTransform();

        // Get progression data
        UMcpProgressionComponent* Progression = Character->FindComponentByClass<UMcpProgressionComponent>();
        if (Progression)
        {
            CharacterData.Level = Progression->Level;
            CharacterData.CurrentXP = Progression->CurrentXP;
            CharacterData.SkillPoints = Progression->SkillPoints;
            CharacterData.UnlockedSkills = Progression->UnlockedSkills;
        }

        // Get inventory data
        if (Character->InventoryComponent)
        {
            CharacterData.Inventory = Character->InventoryComponent->Slots;
            CharacterData.Currencies = Character->InventoryComponent->Currencies;
        }

        // Get equipment data
        if (Character->EquipmentComponent)
        {
            CharacterData.Equipment = Character->EquipmentComponent->EquippedItems;
        }
    }

    // Get quest data
    UMcpQuestTrackerComponent* QuestTracker = Pawn->FindComponentByClass<UMcpQuestTrackerComponent>();
    if (QuestTracker)
    {
        QuestData.ActiveQuests = QuestTracker->ActiveQuests;
        QuestData.CompletedQuests = QuestTracker->CompletedQuests;
        QuestData.FailedQuests = QuestTracker->FailedQuests;
    }

    // Get faction data
    UMcpFactionComponent* FactionComp = Pawn->FindComponentByClass<UMcpFactionComponent>();
    if (FactionComp)
    {
        FactionStandings = FactionComp->FactionStandings;
    }

    // Get current level
    CharacterData.CurrentLevel = FName(*PlayerController->GetWorld()->GetName());
}

void UMcpSaveGame::ApplyToPlayer(APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    APawn* Pawn = PlayerController->GetPawn();
    if (!Pawn)
    {
        return;
    }

    AMcpCharacter* Character = Cast<AMcpCharacter>(Pawn);
    if (Character)
    {
        Character->CharacterId = CharacterData.CharacterId;
        Character->CharacterName = CharacterData.CharacterName;
        Character->SetActorTransform(CharacterData.WorldTransform);

        // Apply progression data
        UMcpProgressionComponent* Progression = Character->FindComponentByClass<UMcpProgressionComponent>();
        if (Progression)
        {
            Progression->Level = CharacterData.Level;
            Progression->CurrentXP = CharacterData.CurrentXP;
            Progression->SkillPoints = CharacterData.SkillPoints;
            Progression->UnlockedSkills = CharacterData.UnlockedSkills;
        }

        // Apply inventory data
        if (Character->InventoryComponent)
        {
            Character->InventoryComponent->Slots = CharacterData.Inventory;
            Character->InventoryComponent->Currencies = CharacterData.Currencies;
        }

        // Apply equipment data
        if (Character->EquipmentComponent)
        {
            Character->EquipmentComponent->EquippedItems = CharacterData.Equipment;
            Character->EquipmentComponent->RecalculateStats();
        }
    }

    // Apply quest data
    UMcpQuestTrackerComponent* QuestTracker = Pawn->FindComponentByClass<UMcpQuestTrackerComponent>();
    if (QuestTracker)
    {
        QuestTracker->ActiveQuests = QuestData.ActiveQuests;
        QuestTracker->CompletedQuests = QuestData.CompletedQuests;
        QuestTracker->FailedQuests = QuestData.FailedQuests;
    }

    // Apply faction data
    UMcpFactionComponent* FactionComp = Pawn->FindComponentByClass<UMcpFactionComponent>();
    if (FactionComp)
    {
        FactionComp->FactionStandings = FactionStandings;
    }
}

void UMcpSaveGame::PopulateFromGameState(AGameStateBase* GameState)
{
    AMcpGameState* McpGameState = Cast<AMcpGameState>(GameState);
    if (McpGameState)
    {
        WorldData.GameTime = McpGameState->GameTime;
        WorldData.TimeOfDay = McpGameState->TimeOfDay;
        WorldData.DayNumber = McpGameState->DayNumber;
        WorldData.CurrentWeather = McpGameState->CurrentWeather;
        WorldData.GlobalVariables = McpGameState->GlobalVariables;
    }
}

void UMcpSaveGame::ApplyToGameState(AGameStateBase* GameState)
{
    AMcpGameState* McpGameState = Cast<AMcpGameState>(GameState);
    if (McpGameState)
    {
        McpGameState->GameTime = WorldData.GameTime;
        McpGameState->TimeOfDay = WorldData.TimeOfDay;
        McpGameState->DayNumber = WorldData.DayNumber;
        McpGameState->CurrentWeather = WorldData.CurrentWeather;
        McpGameState->GlobalVariables = WorldData.GlobalVariables;
    }
}

// ============================================================================
// HUD
// ============================================================================

AMcpHUD::AMcpHUD()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMcpHUD::BeginPlay()
{
    Super::BeginPlay();
}

void AMcpHUD::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Fade out damage indicators
    for (int32 i = RecentDamageDirections.Num() - 1; i >= 0; --i)
    {
        // Remove old indicators
        // This is simplified - actual implementation would track time
    }
}

void AMcpHUD::DrawHUD()
{
    Super::DrawHUD();
    
    if (!Canvas) return;
    
    // Get viewport size
    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);
    FVector2D Center = ViewportSize * 0.5f;
    
    // Draw crosshair
    if (bShowCrosshair && CrosshairTexture)
    {
        float CrosshairSize = 32.0f;
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawIcon(FCanvasIcon(CrosshairTexture, 0, 0, CrosshairSize, CrosshairSize),
            Center.X - CrosshairSize * 0.5f, Center.Y - CrosshairSize * 0.5f);
    }
    
    // Draw health bar
    if (bShowHealthBar)
    {
        float BarWidth = 200.0f;
        float BarHeight = 20.0f;
        float BarX = 20.0f;
        float BarY = ViewportSize.Y - 40.0f;
        
        // Background
        Canvas->SetDrawColor(FColor(40, 40, 40, 200));
        Canvas->DrawRect(BarX, BarY, BarWidth, BarHeight);
        
        // Health fill
        float HealthPercent = FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
        Canvas->SetDrawColor(FColor::Green);
        Canvas->DrawRect(BarX + 2, BarY + 2, (BarWidth - 4) * HealthPercent, BarHeight - 4);
        
        // Health text
        FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth));
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawText(GEngine->GetSmallFont(), HealthText, BarX + BarWidth * 0.5f - 30.0f, BarY + 2.0f);
    }
    
    // Draw damage indicators
    for (int32 i = RecentDamageDirections.Num() - 1; i >= 0; i--)
    {
        FVector DamageDir = RecentDamageDirections[i];
        float Angle = FMath::Atan2(DamageDir.Y, DamageDir.X);
        
        // Draw directional damage indicator
        FVector2D IndicatorPos = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * 100.0f;
        Canvas->SetDrawColor(FColor::Red);
        Canvas->DrawRect(IndicatorPos.X - 10, IndicatorPos.Y - 5, 20, 10);
    }
    
    // Draw interaction prompt
    if (bShowingInteractionPrompt && !CurrentInteractionPrompt.IsEmpty())
    {
        FString PromptStr = CurrentInteractionPrompt.ToString();
        Canvas->SetDrawColor(FColor::White);
        Canvas->DrawText(GEngine->GetMediumFont(), PromptStr, Center.X - 100.0f, ViewportSize.Y - 100.0f);
    }
}

void AMcpHUD::ShowDamageIndicator(FVector DamageDirection, float Duration)
{
    RecentDamageDirections.Add(DamageDirection);
}

void AMcpHUD::ShowInteractionPrompt(const FText& PromptText)
{
    CurrentInteractionPrompt = PromptText;
    bShowingInteractionPrompt = true;
}

void AMcpHUD::HideInteractionPrompt()
{
    bShowingInteractionPrompt = false;
}

void AMcpHUD::ShowNotification(const FText& Title, const FText& Message, float Duration)
{
    // Create and display notification widget
    if (NotificationWidgetClass)
    {
        UUserWidget* NotificationWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), NotificationWidgetClass);
        if (NotificationWidget)
        {
            // Set notification data via interface or direct cast
            if (IMcpNotificationInterface* NotifyInterface = Cast<IMcpNotificationInterface>(NotificationWidget))
            {
                NotifyInterface->SetNotificationData(Title, Message, Duration);
            }
            
            NotificationWidget->AddToViewport(100); // High Z-order for notifications
            
            // Auto-remove after duration
            FTimerHandle RemovalTimer;
            TWeakObjectPtr<UUserWidget> WeakWidget = NotificationWidget;
            GetWorld()->GetTimerManager().SetTimer(RemovalTimer, [WeakWidget]()
            {
                if (WeakWidget.IsValid())
                {
                    WeakWidget->RemoveFromParent();
                }
            }, Duration, false);
        }
    }
    else
    {
        // Fallback: Log the notification
        UE_LOG(LogTemp, Display, TEXT("Notification: %s - %s"), *Title.ToString(), *Message.ToString());
    }
}

void AMcpHUD::SetHUDVisible(bool bVisible)
{
    bShowCrosshair = bVisible;
    bShowHealthBar = bVisible;
    bShowMinimap = bVisible;
    bShowInteractionPrompt = bVisible;
}
