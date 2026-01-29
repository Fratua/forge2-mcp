// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Game Framework Classes (GameMode, GameState, PlayerState, etc.)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h"
#include "AIController.h"
#include "GameFramework/HUD.h"
#include "Forge2Components.h"
#include "Forge2GameFramework.generated.h"

// Forward declarations
class UMcpQuestTrackerComponent;
class UMcpInventoryComponent;
class UMcpProgressionComponent;

// ============================================================================
// Game Mode
// Base game mode with FORGE2 integration
// ============================================================================

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API AMcpGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMcpGameMode();

    // Game settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Settings")
    bool bEnableAutoSave = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Settings")
    float AutoSaveInterval = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Settings")
    bool bEnableDifficultyScaling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Settings")
    int32 DefaultDifficulty = 1; // 0=Easy, 1=Normal, 2=Hard, 3=Nightmare

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Settings")
    bool bFriendlyFireEnabled = false;

    // Spawn settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Spawning")
    TSubclassOf<APawn> DefaultCharacterClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Spawning")
    TArray<AActor*> SpawnPoints;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    virtual void SetDifficulty(int32 NewDifficulty);

    UFUNCTION(BlueprintPure, Category = "FORGE2")
    int32 GetDifficulty() const { return CurrentDifficulty; }

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    virtual void TriggerAutoSave();

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    virtual AActor* GetRandomSpawnPoint() const;

    // Overrides
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

protected:
    int32 CurrentDifficulty = 1;
    float AutoSaveTimer = 0.0f;
};

// ============================================================================
// Game State
// Replicated game state for all players
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePhaseChanged, FName, NewPhase, FName, OldPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEventTriggered, FName, EventId);

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API AMcpGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AMcpGameState();

    // Current game phase (lobby, playing, ended, etc.)
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|State")
    FName CurrentPhase;

    // Game time (in-game seconds)
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|State")
    float GameTime = 0.0f;

    // Time of day (0-24)
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|State")
    float TimeOfDay = 12.0f;

    // Day number
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|State")
    int32 DayNumber = 1;

    // Weather state
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|State")
    FName CurrentWeather;

    // Global game variables
    UPROPERTY(BlueprintReadWrite, Replicated, Category = "FORGE2|State")
    TMap<FName, FString> GlobalVariables;

    // Active world events
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|State")
    TSet<FName> ActiveWorldEvents;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Events")
    FOnGamePhaseChanged OnGamePhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Events")
    FOnGameEventTriggered OnGameEventTriggered;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void SetGamePhase(FName NewPhase);

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void AdvanceTime(float Hours);

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void SetWeather(FName WeatherId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void SetGlobalVariable(FName VarName, const FString& Value);

    UFUNCTION(BlueprintPure, Category = "FORGE2")
    FString GetGlobalVariable(FName VarName) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void TriggerWorldEvent(FName EventId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void EndWorldEvent(FName EventId);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Tick(float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Settings")
    float TimeScale = 1.0f; // 1 = real-time, 60 = 1 minute = 1 hour
};

// ============================================================================
// Player State
// Per-player replicated data
// ============================================================================

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API AMcpPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AMcpPlayerState();

    // Player display info
    UPROPERTY(BlueprintReadWrite, Replicated, Category = "FORGE2|Player")
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite, Replicated, Category = "FORGE2|Player")
    int32 PlayerLevel = 1;

    UPROPERTY(BlueprintReadWrite, Replicated, Category = "FORGE2|Player")
    FName ClassName;

    // Combat stats (replicated summary)
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Combat")
    int32 Kills = 0;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Combat")
    int32 Deaths = 0;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Combat")
    int32 Assists = 0;

    // Team
    UPROPERTY(BlueprintReadWrite, Replicated, Category = "FORGE2|Team")
    int32 TeamId = 0;

    // Party/Group
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Party")
    FName PartyId;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Party")
    bool bIsPartyLeader = false;

    // Guild
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Guild")
    FName GuildId;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Guild")
    FName GuildRank;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void AddKill();

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void AddDeath();

    UFUNCTION(BlueprintCallable, Category = "FORGE2")
    void AddAssist();

    UFUNCTION(BlueprintPure, Category = "FORGE2")
    float GetKDRatio() const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

// ============================================================================
// Player Controller
// Enhanced player controller with FORGE2 components
// ============================================================================

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API AMcpPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMcpPlayerController();

    // UI State
    UPROPERTY(BlueprintReadOnly, Category = "FORGE2|UI")
    bool bIsInMenu = false;

    UPROPERTY(BlueprintReadOnly, Category = "FORGE2|UI")
    bool bIsInDialogue = false;

    UPROPERTY(BlueprintReadOnly, Category = "FORGE2|UI")
    bool bIsInCutscene = false;

    // Input State
    UPROPERTY(BlueprintReadOnly, Category = "FORGE2|Input")
    bool bGamepadActive = false;

    // Camera
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Camera")
    float DefaultFOV = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Camera")
    float AimingFOV = 60.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "FORGE2|UI")
    void EnterMenu();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|UI")
    void ExitMenu();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|UI")
    void EnterDialogue();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|UI")
    void ExitDialogue();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|UI")
    void EnterCutscene();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|UI")
    void ExitCutscene();

    UFUNCTION(BlueprintPure, Category = "FORGE2")
    bool CanPlayerMove() const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Camera")
    void SetFOV(float NewFOV, float BlendTime = 0.2f);

    // Interaction
    UFUNCTION(BlueprintCallable, Category = "FORGE2|Interaction")
    AActor* GetLookAtActor(float MaxDistance = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Interaction")
    bool TryInteractWithLookedAtActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupInputComponent() override;

private:
    float TargetFOV;
    float FOVBlendTime;
    float CurrentFOVBlendTime;
};

// ============================================================================
// Character
// Base character with FORGE2 components
// ============================================================================

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API AMcpCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMcpCharacter();

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FORGE2|Components")
    UMcpCombatComponent* CombatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FORGE2|Components")
    UMcpInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FORGE2|Components")
    UMcpEquipmentComponent* EquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FORGE2|Components")
    UMcpStatusEffectComponent* StatusEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FORGE2|Components")
    UMcpInteractionComponent* InteractionComponent;

    // Character Info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "FORGE2|Character")
    FName CharacterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "FORGE2|Character")
    FText CharacterName;

    // Movement modes
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Movement")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "FORGE2|Movement")
    bool bIsAiming = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Movement")
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|Movement")
    float CrouchSpeed = 150.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "FORGE2|Movement")
    void StartSprint();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Movement")
    void StopSprint();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Movement")
    void ToggleCrouch();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Combat")
    void StartAiming();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Combat")
    void StopAiming();

    UFUNCTION(BlueprintPure, Category = "FORGE2")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2")
    float GetHealthPercent() const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void UpdateMovementSpeed();
};

// ============================================================================
// AI Controller
// Base AI controller with FORGE2 integration
// ============================================================================

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API AMcpAIController : public AAIController
{
    GENERATED_BODY()

public:
    AMcpAIController();

    // AI Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|AI")
    float AggroRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|AI")
    float LeashRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|AI")
    bool bCanPatrol = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FORGE2|AI")
    TArray<AActor*> PatrolPoints;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "FORGE2|AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "FORGE2|AI")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "FORGE2|AI")
    int32 CurrentPatrolIndex = 0;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "FORGE2|AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|AI")
    void ReturnToHome();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|AI")
    void MoveToNextPatrolPoint();

    UFUNCTION(BlueprintPure, Category = "FORGE2|AI")
    bool IsInAggroRange(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|AI")
    bool IsInLeashRange() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|AI")
    AActor* FindNearestEnemy() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;
};

// ============================================================================
// Game Instance
// Persistent data across level loads
// ============================================================================

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UMcpGameInstance();

    // Current save slot
    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Save")
    int32 CurrentSaveSlot = 0;

    // Player account info
    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Account")
    FString PlayerName;

    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Account")
    FString PlayerId;

    // Settings
    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Settings")
    float MusicVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Settings")
    float MouseSensitivity = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Settings")
    bool bInvertY = false;

    // Persistent data between levels
    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|Persistent")
    TMap<FName, FString> PersistentVariables;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
    bool SaveGame(int32 Slot = -1);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
    bool LoadGame(int32 Slot = -1);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Save")
    bool DeleteSave(int32 Slot);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Save")
    bool DoesSaveExist(int32 Slot) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Settings")
    void ApplyAudioSettings();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Persistent")
    void SetPersistentVariable(FName VarName, const FString& Value);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Persistent")
    FString GetPersistentVariable(FName VarName) const;

    virtual void Init() override;
    virtual void Shutdown() override;

protected:
    void LoadSettings();
    void SaveSettings();
};

// ============================================================================
// Save Game
// Save data structure
// ============================================================================

USTRUCT(BlueprintType)
struct FMcpCharacterSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FName CharacterId;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FText CharacterName;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    int32 Level = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    int32 CurrentXP = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    int32 SkillPoints = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TMap<FName, int32> UnlockedSkills;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TArray<FMcpInventorySlot> Inventory;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TMap<EForge2EquipmentSlot, FMcpInventorySlot> Equipment;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TMap<FName, int64> Currencies;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FTransform WorldTransform;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FName CurrentLevel;
};

USTRUCT(BlueprintType)
struct FMcpQuestSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TMap<FName, FMcpActiveQuest> ActiveQuests;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TSet<FName> CompletedQuests;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TSet<FName> FailedQuests;
};

USTRUCT(BlueprintType)
struct FMcpWorldSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    float GameTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    float TimeOfDay = 12.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    int32 DayNumber = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FName CurrentWeather;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TMap<FName, FString> GlobalVariables;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TSet<FName> DiscoveredLocations;
};

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API UMcpSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UMcpSaveGame();

    // Save metadata
    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FString SaveName;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FDateTime SaveDateTime;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    float PlayTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Save")
    int32 SaveVersion = 1;

    // Character data
    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FMcpCharacterSaveData CharacterData;

    // Quest data
    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FMcpQuestSaveData QuestData;

    // World data
    UPROPERTY(BlueprintReadWrite, Category = "Save")
    FMcpWorldSaveData WorldData;

    // Faction standings
    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TMap<FName, FMcpFactionStanding> FactionStandings;

    // Custom data
    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TMap<FName, FString> CustomData;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Save")
    void PopulateFromPlayer(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Save")
    void ApplyToPlayer(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Save")
    void PopulateFromGameState(AGameStateBase* GameState);

    UFUNCTION(BlueprintCallable, Category = "Save")
    void ApplyToGameState(AGameStateBase* GameState);
};

// ============================================================================
// HUD
// Base HUD class
// ============================================================================

UCLASS(Blueprintable, BlueprintType)
class MCPAUTOMATIONBRIDGE_API AMcpHUD : public AHUD
{
    GENERATED_BODY()

public:
    AMcpHUD();

    // HUD visibility
    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|HUD")
    bool bShowCrosshair = true;

    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|HUD")
    bool bShowHealthBar = true;

    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|HUD")
    bool bShowMinimap = true;

    UPROPERTY(BlueprintReadWrite, Category = "FORGE2|HUD")
    bool bShowInteractionPrompt = true;

    // Damage indicators
    UPROPERTY(BlueprintReadOnly, Category = "FORGE2|HUD")
    TArray<FVector> RecentDamageDirections;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "FORGE2|HUD")
    void ShowDamageIndicator(FVector DamageDirection, float Duration = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|HUD")
    void ShowInteractionPrompt(const FText& PromptText);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|HUD")
    void HideInteractionPrompt();

    UFUNCTION(BlueprintCallable, Category = "FORGE2|HUD")
    void ShowNotification(const FText& Title, const FText& Message, float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|HUD")
    void SetHUDVisible(bool bVisible);

protected:
    virtual void DrawHUD() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    FText CurrentInteractionPrompt;
    bool bShowingInteractionPrompt = false;
};
