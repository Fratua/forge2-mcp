// Forge2AISystem.h - AI System Integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Forge2AISystem.generated.h"

// Forward declarations
class UBehaviorTree;
class UBlackboardData;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UNavigationQueryFilter;

// Enums
UENUM(BlueprintType)
enum class EForge2BTNodeType : uint8
{
    Selector,
    Sequence,
    Parallel,
    SimpleParallel,
    Task,
    Decorator,
    Service
};

UENUM(BlueprintType)
enum class EForge2BTTaskType : uint8
{
    MoveTo,
    Wait,
    PlayAnimation,
    PlaySound,
    SetBlackboardValue,
    ExecuteAbility,
    RotateToFace,
    LookAt,
    Custom
};

UENUM(BlueprintType)
enum class EForge2BTDecoratorType : uint8
{
    Blackboard,
    Cooldown,
    LoopCount,
    TimeLimit,
    ConditionalLoop,
    ForceSuccess,
    IsAtLocation,
    TagCooldown,
    KeepInCone,
    Custom
};

UENUM(BlueprintType)
enum class EForge2BTServiceType : uint8
{
    SetDefaultFocus,
    UpdateBlackboard,
    RunEQS,
    Custom
};

UENUM(BlueprintType)
enum class EForge2BlackboardKeyType : uint8
{
    Bool,
    Int,
    Float,
    String,
    Name,
    Vector,
    Rotator,
    Object,
    Class,
    Enum
};

UENUM(BlueprintType)
enum class EForge2PerceptionSense : uint8
{
    Sight,
    Hearing,
    Damage,
    Touch,
    Team,
    Prediction,
    Custom
};

UENUM(BlueprintType)
enum class EForge2AIState : uint8
{
    Idle,
    Patrol,
    Alert,
    Investigate,
    Combat,
    Flee,
    Search,
    Dead,
    Custom
};

UENUM(BlueprintType)
enum class EForge2SquadRole : uint8
{
    Leader,
    Flanker,
    Support,
    Sniper,
    Tank,
    Medic,
    Scout,
    None
};

UENUM(BlueprintType)
enum class EForge2FormationType : uint8
{
    Line,
    Column,
    Wedge,
    Circle,
    Staggered,
    Custom
};

// Structs
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BlackboardKeyDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName KeyName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BlackboardKeyType KeyType = EForge2BlackboardKeyType::Object;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bInstanceSynced = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UClass* BaseClass = nullptr;  // For Object/Class types
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BTNodeDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString NodeName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BTNodeType NodeType = EForge2BTNodeType::Sequence;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2BTNodeDef> Children;

    // For Task nodes
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BTTaskType TaskType = EForge2BTTaskType::MoveTo;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TargetBlackboardKey;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AcceptableRadius = 50.0f;

    // For Decorator nodes
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BTDecoratorType DecoratorType = EForge2BTDecoratorType::Blackboard;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName DecoratorKey;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DecoratorValue = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAbortSelf = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAbortLowerPriority = false;

    // For Service nodes
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2BTServiceType ServiceType = EForge2BTServiceType::UpdateBlackboard;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ServiceInterval = 0.5f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SightRadius = 3000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LoseSightRadius = 3500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PeripheralVisionAngle = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AutoSuccessRangeFromLastSeenLocation = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxAge = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDetectEnemies = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDetectNeutrals = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDetectFriendlies = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HearingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HearingRange = 3000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LoSHearingRange = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxAge = 3.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDetectEnemies = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDetectNeutrals = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDetectFriendlies = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DamageConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxAge = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDetectEnemies = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDetectFriendlies = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PerceptionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableSight = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2SightConfig SightConfig;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableHearing = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2HearingConfig HearingConfig;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableDamage = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FForge2DamageConfig DamageConfig;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DominantSenseInterval = 0.5f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WaitTime = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator LookDirection = FRotator::ZeroRotator;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLookAround = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LookAroundTime = 3.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PatrolRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString RouteName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2PatrolPoint> Points;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLoop = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bReverseOnEnd = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRandomize = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MovementSpeed = 300.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GOAPAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ActionName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Cost = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FName, bool> Preconditions;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FName, bool> Effects;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRequiresTarget = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GOAPGoal
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString GoalName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FName, bool> DesiredState;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Priority = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2UtilityAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ActionName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> ConsiderationKeys;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> ConsiderationWeights;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Momentum = 0.1f;  // Bonus for current action
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Cooldown = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InfluenceSource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Strength = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DecayRate = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName InfluenceType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) AActor* SourceActor = nullptr;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SquadMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) AActor* Actor = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2SquadRole Role = EForge2SquadRole::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector FormationOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsAlive = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2Squad
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SquadName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FForge2SquadMember> Members;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2FormationType Formation = EForge2FormationType::Wedge;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FormationSpacing = 200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector CurrentTarget = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EForge2AIState SquadState = EForge2AIState::Idle;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BossPhase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString PhaseName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HealthThreshold = 1.0f;  // Trigger when health below this %
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> EnabledAbilities;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> DisabledAbilities;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AggressionMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DamageMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UBehaviorTree> PhaseBehaviorTree;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TransitionAnimation;
};

// Manager Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2AIManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2AIManager();

    // Behavior Tree
    UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
    UBehaviorTree* CreateBehaviorTree(const FString& TreeName, const FForge2BTNodeDef& RootNode);

    UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
    void AddBTNode(UBehaviorTree* Tree, const FForge2BTNodeDef& NodeDef, int32 ParentIndex);

    UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
    void AddBTDecorator(UBehaviorTree* Tree, int32 NodeIndex, const FForge2BTNodeDef& DecoratorDef);

    UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
    void AddBTService(UBehaviorTree* Tree, int32 NodeIndex, const FForge2BTNodeDef& ServiceDef);

    // Blackboard
    UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
    UBlackboardData* CreateBlackboard(const FString& BlackboardName, const TArray<FForge2BlackboardKeyDef>& Keys);

    UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
    void AddBlackboardKey(UBlackboardData* Blackboard, const FForge2BlackboardKeyDef& KeyDef);

    UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
    void SetBlackboardValue(UBlackboardComponent* BlackboardComp, FName KeyName, UObject* Value);

    UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
    void SetBlackboardValueAsVector(UBlackboardComponent* BlackboardComp, FName KeyName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
    void SetBlackboardValueAsFloat(UBlackboardComponent* BlackboardComp, FName KeyName, float Value);

    // Perception
    UFUNCTION(BlueprintCallable, Category = "AI|Perception")
    UAIPerceptionComponent* CreatePerceptionComponent(AActor* Owner, const FForge2PerceptionConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "AI|Perception")
    void ConfigureSight(UAIPerceptionComponent* PerceptionComp, const FForge2SightConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "AI|Perception")
    void ConfigureHearing(UAIPerceptionComponent* PerceptionComp, const FForge2HearingConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "AI|Perception")
    void ConfigureDamage(UAIPerceptionComponent* PerceptionComp, const FForge2DamageConfig& Config);

    // Patrol
    UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
    void SetPatrolRoute(AActor* AIActor, const FForge2PatrolRoute& Route);

    UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
    FVector GetNextPatrolPoint(AActor* AIActor);

    // GOAP
    UFUNCTION(BlueprintCallable, Category = "AI|GOAP")
    void RegisterGOAPAction(AActor* AIActor, const FForge2GOAPAction& Action);

    UFUNCTION(BlueprintCallable, Category = "AI|GOAP")
    void SetGOAPGoal(AActor* AIActor, const FForge2GOAPGoal& Goal);

    UFUNCTION(BlueprintCallable, Category = "AI|GOAP")
    TArray<FString> PlanGOAPActions(AActor* AIActor);

    UFUNCTION(BlueprintCallable, Category = "AI|GOAP")
    void SetGOAPWorldState(AActor* AIActor, FName Key, bool Value);

    // Utility AI
    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    void RegisterUtilityAction(AActor* AIActor, const FForge2UtilityAction& Action);

    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    void SetUtilityConsideration(AActor* AIActor, FName Key, float Value);

    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    FString EvaluateUtilityActions(AActor* AIActor);

    // Influence Maps
    UFUNCTION(BlueprintCallable, Category = "AI|InfluenceMap")
    void AddInfluenceSource(const FForge2InfluenceSource& Source);

    UFUNCTION(BlueprintCallable, Category = "AI|InfluenceMap")
    void RemoveInfluenceSource(AActor* SourceActor, FName InfluenceType);

    UFUNCTION(BlueprintCallable, Category = "AI|InfluenceMap")
    float GetInfluenceAtLocation(FVector Location, FName InfluenceType);

    UFUNCTION(BlueprintCallable, Category = "AI|InfluenceMap")
    FVector FindBestInfluenceLocation(FVector Origin, float SearchRadius, FName InfluenceType, bool bMaximize);

    // Squad AI
    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void CreateSquad(const FForge2Squad& Squad);

    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void AddToSquad(const FString& SquadName, AActor* Member, EForge2SquadRole Role);

    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void RemoveFromSquad(const FString& SquadName, AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void SetSquadFormation(const FString& SquadName, EForge2FormationType Formation);

    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void OrderSquadMoveTo(const FString& SquadName, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    void OrderSquadAttack(const FString& SquadName, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI|Squad")
    FVector GetFormationPosition(const FString& SquadName, AActor* Member);

    // Boss AI
    UFUNCTION(BlueprintCallable, Category = "AI|Boss")
    void SetupBossPhases(AActor* BossActor, const TArray<FForge2BossPhase>& Phases);

    UFUNCTION(BlueprintCallable, Category = "AI|Boss")
    void TransitionBossPhase(AActor* BossActor, const FString& PhaseName);

    UFUNCTION(BlueprintCallable, Category = "AI|Boss")
    void CheckBossPhaseTransition(AActor* BossActor, float CurrentHealthPercent);

    // Difficulty
    UFUNCTION(BlueprintCallable, Category = "AI|Difficulty")
    void SetAIDifficulty(AActor* AIActor, float DifficultyScale);

    UFUNCTION(BlueprintCallable, Category = "AI|Difficulty")
    void ConfigureAIAggression(AActor* AIActor, float AggressionLevel);

private:
    UPROPERTY() TMap<FString, FForge2PatrolRoute> PatrolRoutes;
    UPROPERTY() TMap<FString, int32> PatrolIndices;
    UPROPERTY() TArray<FForge2InfluenceSource> InfluenceSources;
    UPROPERTY() TMap<FString, FForge2Squad> Squads;
    UPROPERTY() TMap<AActor*, TArray<FForge2BossPhase>> BossPhases;
    UPROPERTY() TMap<AActor*, int32> CurrentBossPhaseIndex;

    TArray<FForge2GOAPAction>* GetGOAPActions(AActor* AIActor);
    TMap<FName, bool>* GetGOAPWorldState(AActor* AIActor);
    FVector CalculateFormationOffset(EForge2FormationType Formation, int32 MemberIndex, int32 TotalMembers, float Spacing);
};

// Utility Class
UCLASS(BlueprintType)
class MCPAUTOMATIONBRIDGE_API UForge2AIUtility : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    static FForge2PerceptionConfig CreateDefaultPerception();

    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    static FForge2PerceptionConfig CreateStealthPerception();  // High sight, low hearing

    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    static FForge2PerceptionConfig CreateGuardPerception();  // Balanced

    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    static FForge2PatrolRoute CreateSimplePatrolRoute(const TArray<FVector>& Points, float WaitTime);

    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    static TArray<FForge2BlackboardKeyDef> CreateCombatBlackboardKeys();

    UFUNCTION(BlueprintCallable, Category = "AI|Utility")
    static TArray<FForge2BlackboardKeyDef> CreatePatrolBlackboardKeys();
};
