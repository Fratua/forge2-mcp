// Forge2AISystem.cpp - AI System Implementation
#include "Forge2AISystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Name.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_String.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/Composites/BTComposite_Selector.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTree/Composites/BTComposite_SimpleParallel.h"
#include "BehaviorTree/Tasks/BTTask_Wait.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BehaviorTree/Tasks/BTTask_RunBehavior.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BehaviorTree/Decorators/BTDecorator_Cooldown.h"
#include "BehaviorTree/Services/BTService_DefaultFocus.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// UForge2AIManager Implementation
UForge2AIManager::UForge2AIManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UBehaviorTree* UForge2AIManager::CreateBehaviorTree(const FString& TreeName, const FForge2BTNodeDef& RootNode)
{
#if WITH_EDITOR
    UBehaviorTree* BT = NewObject<UBehaviorTree>(GetTransientPackage(), *TreeName, RF_Public | RF_Standalone);
    if (BT)
    {
        // Create BlackboardData if not provided
        if (!BT->BlackboardAsset)
        {
            BT->BlackboardAsset = NewObject<UBlackboardData>(BT, TEXT("DefaultBlackboard"));
        }

        // Create root composite node based on RootNode definition
        UBTCompositeNode* RootComposite = nullptr;
        
        if (RootNode.NodeType == TEXT("Selector"))
        {
            RootComposite = NewObject<UBTComposite_Selector>(BT);
        }
        else if (RootNode.NodeType == TEXT("Sequence"))
        {
            RootComposite = NewObject<UBTComposite_Sequence>(BT);
        }
        else if (RootNode.NodeType == TEXT("SimpleParallel"))
        {
            RootComposite = NewObject<UBTComposite_SimpleParallel>(BT);
        }
        else
        {
            // Default to Selector
            RootComposite = NewObject<UBTComposite_Selector>(BT);
        }

        if (RootComposite)
        {
            RootComposite->InitializeNode(nullptr, 0, 0, 0);
            BT->RootNode = RootComposite;
            
            // Process children recursively
            int32 ExecutionIndex = 1;
            int32 MemoryOffset = 0;
            for (const FForge2BTNodeDef& ChildDef : RootNode.Children)
            {
                AddBTNodeRecursive(BT, RootComposite, ChildDef, ExecutionIndex, MemoryOffset);
            }
        }
    }
    return BT;
#else
    return nullptr;
#endif
}

void UForge2AIManager::AddBTNodeRecursive(UBehaviorTree* Tree, UBTCompositeNode* ParentNode, 
    const FForge2BTNodeDef& NodeDef, int32& ExecutionIndex, int32& MemoryOffset)
{
#if WITH_EDITOR
    if (!Tree || !ParentNode) return;

    UBTNode* NewNode = nullptr;

    // Create composite or task based on type
    if (NodeDef.NodeType == TEXT("Selector"))
    {
        UBTComposite_Selector* Selector = NewObject<UBTComposite_Selector>(Tree);
        NewNode = Selector;
    }
    else if (NodeDef.NodeType == TEXT("Sequence"))
    {
        UBTComposite_Sequence* Sequence = NewObject<UBTComposite_Sequence>(Tree);
        NewNode = Sequence;
    }
    else if (NodeDef.NodeType == TEXT("SimpleParallel"))
    {
        UBTComposite_SimpleParallel* Parallel = NewObject<UBTComposite_SimpleParallel>(Tree);
        NewNode = Parallel;
    }
    else if (NodeDef.NodeType == TEXT("Wait"))
    {
        UBTTask_Wait* WaitTask = NewObject<UBTTask_Wait>(Tree);
        if (NodeDef.Parameters.Contains(TEXT("WaitTime")))
        {
            WaitTask->WaitTime = FCString::Atof(*NodeDef.Parameters[TEXT("WaitTime")]);
        }
        NewNode = WaitTask;
    }
    else if (NodeDef.NodeType == TEXT("MoveTo"))
    {
        UBTTask_MoveTo* MoveTask = NewObject<UBTTask_MoveTo>(Tree);
        if (NodeDef.Parameters.Contains(TEXT("AcceptableRadius")))
        {
            MoveTask->AcceptableRadius = FCString::Atof(*NodeDef.Parameters[TEXT("AcceptableRadius")]);
        }
        NewNode = MoveTask;
    }
    else if (NodeDef.NodeType == TEXT("RunBehavior"))
    {
        UBTTask_RunBehavior* RunTask = NewObject<UBTTask_RunBehavior>(Tree);
        NewNode = RunTask;
    }

    if (NewNode)
    {
        // Initialize the node
        NewNode->InitializeNode(ParentNode, ExecutionIndex++, 0, MemoryOffset);
        MemoryOffset += NewNode->GetInstanceMemorySize();

        // Add to parent's children
        FBTCompositeChild& NewChild = ParentNode->Children.AddDefaulted_GetRef();
        
        if (UBTCompositeNode* CompositeNode = Cast<UBTCompositeNode>(NewNode))
        {
            NewChild.ChildComposite = CompositeNode;
            
            // Recursively add children
            for (const FForge2BTNodeDef& ChildDef : NodeDef.Children)
            {
                AddBTNodeRecursive(Tree, CompositeNode, ChildDef, ExecutionIndex, MemoryOffset);
            }
        }
        else if (UBTTaskNode* TaskNode = Cast<UBTTaskNode>(NewNode))
        {
            NewChild.ChildTask = TaskNode;
        }
    }
#endif
}

void UForge2AIManager::AddBTNode(UBehaviorTree* Tree, const FForge2BTNodeDef& NodeDef, int32 ParentIndex)
{
    if (!Tree || !Tree->RootNode) return;

#if WITH_EDITOR
    UBTCompositeNode* ParentNode = Tree->RootNode;
    
    // Find parent node by index if specified
    if (ParentIndex > 0)
    {
        TArray<UBTCompositeNode*> CompositeNodes;
        CollectCompositeNodes(Tree->RootNode, CompositeNodes);
        
        if (ParentIndex < CompositeNodes.Num())
        {
            ParentNode = CompositeNodes[ParentIndex];
        }
    }

    int32 ExecutionIndex = CountTreeNodes(Tree->RootNode) + 1;
    int32 MemoryOffset = 0;
    AddBTNodeRecursive(Tree, ParentNode, NodeDef, ExecutionIndex, MemoryOffset);
#endif
}

void UForge2AIManager::CollectCompositeNodes(UBTCompositeNode* Node, TArray<UBTCompositeNode*>& OutNodes)
{
    if (!Node) return;
    
    OutNodes.Add(Node);
    
    for (const FBTCompositeChild& Child : Node->Children)
    {
        if (Child.ChildComposite)
        {
            CollectCompositeNodes(Child.ChildComposite, OutNodes);
        }
    }
}

int32 UForge2AIManager::CountTreeNodes(UBTCompositeNode* Node)
{
    if (!Node) return 0;
    
    int32 Count = 1;
    for (const FBTCompositeChild& Child : Node->Children)
    {
        if (Child.ChildComposite)
        {
            Count += CountTreeNodes(Child.ChildComposite);
        }
        else if (Child.ChildTask)
        {
            Count++;
        }
    }
    return Count;
}

void UForge2AIManager::AddBTDecorator(UBehaviorTree* Tree, int32 NodeIndex, const FForge2BTNodeDef& DecoratorDef)
{
    if (!Tree || !Tree->RootNode) return;

#if WITH_EDITOR
    TArray<UBTCompositeNode*> CompositeNodes;
    CollectCompositeNodes(Tree->RootNode, CompositeNodes);
    
    if (NodeIndex >= CompositeNodes.Num()) return;
    
    UBTCompositeNode* TargetNode = CompositeNodes[NodeIndex];
    UBTDecorator* NewDecorator = nullptr;
    
    if (DecoratorDef.NodeType == TEXT("Blackboard"))
    {
        UBTDecorator_Blackboard* BBDecorator = NewObject<UBTDecorator_Blackboard>(Tree);
        if (DecoratorDef.Parameters.Contains(TEXT("KeyName")))
        {
            // Set up blackboard key check
            BBDecorator->BlackboardKey.SelectedKeyName = FName(*DecoratorDef.Parameters[TEXT("KeyName")]);
        }
        NewDecorator = BBDecorator;
    }
    else if (DecoratorDef.NodeType == TEXT("Cooldown"))
    {
        UBTDecorator_Cooldown* CDDecorator = NewObject<UBTDecorator_Cooldown>(Tree);
        if (DecoratorDef.Parameters.Contains(TEXT("CooldownTime")))
        {
            CDDecorator->CoolDownTime = FCString::Atof(*DecoratorDef.Parameters[TEXT("CooldownTime")]);
        }
        NewDecorator = CDDecorator;
    }
    
    if (NewDecorator && TargetNode->Children.Num() > 0)
    {
        int32 ExecutionIndex = CountTreeNodes(Tree->RootNode) + 1;
        NewDecorator->InitializeNode(TargetNode, ExecutionIndex, 0, 0);
        TargetNode->Children.Last().Decorators.Add(NewDecorator);
    }
#endif
}

void UForge2AIManager::AddBTService(UBehaviorTree* Tree, int32 NodeIndex, const FForge2BTNodeDef& ServiceDef)
{
    if (!Tree || !Tree->RootNode) return;

#if WITH_EDITOR
    TArray<UBTCompositeNode*> CompositeNodes;
    CollectCompositeNodes(Tree->RootNode, CompositeNodes);
    
    if (NodeIndex >= CompositeNodes.Num()) return;
    
    UBTCompositeNode* TargetNode = CompositeNodes[NodeIndex];
    UBTService* NewService = nullptr;
    
    if (ServiceDef.NodeType == TEXT("DefaultFocus"))
    {
        UBTService_DefaultFocus* FocusService = NewObject<UBTService_DefaultFocus>(Tree);
        NewService = FocusService;
    }
    
    if (NewService)
    {
        int32 ExecutionIndex = CountTreeNodes(Tree->RootNode) + 1;
        NewService->InitializeNode(TargetNode, ExecutionIndex, 0, 0);
        TargetNode->Services.Add(NewService);
    }
#endif
}

UBlackboardData* UForge2AIManager::CreateBlackboard(const FString& BlackboardName,
                                                       const TArray<FForge2BlackboardKeyDef>& Keys)
{
#if WITH_EDITOR
    UBlackboardData* Blackboard = NewObject<UBlackboardData>(GetTransientPackage(), *BlackboardName, RF_Public | RF_Standalone);
    if (Blackboard)
    {
        for (const FForge2BlackboardKeyDef& KeyDef : Keys)
        {
            AddBlackboardKey(Blackboard, KeyDef);
        }
        
        // Post-edit to validate keys
        Blackboard->PostEditChange();
    }
    return Blackboard;
#else
    return nullptr;
#endif
}

void UForge2AIManager::AddBlackboardKey(UBlackboardData* Blackboard, const FForge2BlackboardKeyDef& KeyDef)
{
    if (!Blackboard) return;

#if WITH_EDITOR
    FBlackboardEntry Entry;
    Entry.EntryName = KeyDef.KeyName;
    Entry.bInstanceSynced = KeyDef.bInstanceSynced;

    switch (KeyDef.KeyType)
    {
        case EForge2BlackboardKeyType::Bool:
            Entry.KeyType = NewObject<UBlackboardKeyType_Bool>(Blackboard);
            break;
        case EForge2BlackboardKeyType::Int:
            Entry.KeyType = NewObject<UBlackboardKeyType_Int>(Blackboard);
            break;
        case EForge2BlackboardKeyType::Float:
            Entry.KeyType = NewObject<UBlackboardKeyType_Float>(Blackboard);
            break;
        case EForge2BlackboardKeyType::String:
            Entry.KeyType = NewObject<UBlackboardKeyType_String>(Blackboard);
            break;
        case EForge2BlackboardKeyType::Name:
            Entry.KeyType = NewObject<UBlackboardKeyType_Name>(Blackboard);
            break;
        case EForge2BlackboardKeyType::Vector:
            Entry.KeyType = NewObject<UBlackboardKeyType_Vector>(Blackboard);
            break;
        case EForge2BlackboardKeyType::Rotator:
            Entry.KeyType = NewObject<UBlackboardKeyType_Rotator>(Blackboard);
            break;
        case EForge2BlackboardKeyType::Object:
            {
                UBlackboardKeyType_Object* ObjKey = NewObject<UBlackboardKeyType_Object>(Blackboard);
                if (KeyDef.BaseClass)
                {
                    ObjKey->BaseClass = KeyDef.BaseClass;
                }
                Entry.KeyType = ObjKey;
            }
            break;
        default:
            Entry.KeyType = NewObject<UBlackboardKeyType_Object>(Blackboard);
            break;
    }

    Blackboard->Keys.Add(Entry);
#endif
}

void UForge2AIManager::SetBlackboardValue(UBlackboardComponent* BlackboardComp, FName KeyName, UObject* Value)
{
    if (!BlackboardComp) return;
    BlackboardComp->SetValueAsObject(KeyName, Value);
}

void UForge2AIManager::SetBlackboardValueAsVector(UBlackboardComponent* BlackboardComp, FName KeyName, FVector Value)
{
    if (!BlackboardComp) return;
    BlackboardComp->SetValueAsVector(KeyName, Value);
}

void UForge2AIManager::SetBlackboardValueAsFloat(UBlackboardComponent* BlackboardComp, FName KeyName, float Value)
{
    if (!BlackboardComp) return;
    BlackboardComp->SetValueAsFloat(KeyName, Value);
}

UAIPerceptionComponent* UForge2AIManager::CreatePerceptionComponent(AActor* Owner,
                                                                       const FForge2PerceptionConfig& Config)
{
    if (!Owner) return nullptr;

    UAIPerceptionComponent* PerceptionComp = NewObject<UAIPerceptionComponent>(Owner);
    if (PerceptionComp)
    {
        if (Config.bEnableSight)
        {
            ConfigureSight(PerceptionComp, Config.SightConfig);
        }
        if (Config.bEnableHearing)
        {
            ConfigureHearing(PerceptionComp, Config.HearingConfig);
        }
        if (Config.bEnableDamage)
        {
            ConfigureDamage(PerceptionComp, Config.DamageConfig);
        }

        PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
        PerceptionComp->RegisterComponent();
    }

    return PerceptionComp;
}

void UForge2AIManager::ConfigureSight(UAIPerceptionComponent* PerceptionComp, const FForge2SightConfig& Config)
{
    if (!PerceptionComp) return;

    UAISenseConfig_Sight* SightConfig = NewObject<UAISenseConfig_Sight>(PerceptionComp);
    if (SightConfig)
    {
        SightConfig->SightRadius = Config.SightRadius;
        SightConfig->LoseSightRadius = Config.LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = Config.PeripheralVisionAngle;
        SightConfig->AutoSuccessRangeFromLastSeenLocation = Config.AutoSuccessRangeFromLastSeenLocation;
        SightConfig->SetMaxAge(Config.MaxAge);

        SightConfig->DetectionByAffiliation.bDetectEnemies = Config.bDetectEnemies;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = Config.bDetectNeutrals;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = Config.bDetectFriendlies;

        PerceptionComp->ConfigureSense(*SightConfig);
    }
}

void UForge2AIManager::ConfigureHearing(UAIPerceptionComponent* PerceptionComp, const FForge2HearingConfig& Config)
{
    if (!PerceptionComp) return;

    UAISenseConfig_Hearing* HearingConfig = NewObject<UAISenseConfig_Hearing>(PerceptionComp);
    if (HearingConfig)
    {
        HearingConfig->HearingRange = Config.HearingRange;
        HearingConfig->SetMaxAge(Config.MaxAge);

        HearingConfig->DetectionByAffiliation.bDetectEnemies = Config.bDetectEnemies;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = Config.bDetectNeutrals;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = Config.bDetectFriendlies;

        PerceptionComp->ConfigureSense(*HearingConfig);
    }
}

void UForge2AIManager::ConfigureDamage(UAIPerceptionComponent* PerceptionComp, const FForge2DamageConfig& Config)
{
    if (!PerceptionComp) return;

    UAISenseConfig_Damage* DamageConfig = NewObject<UAISenseConfig_Damage>(PerceptionComp);
    if (DamageConfig)
    {
        DamageConfig->SetMaxAge(Config.MaxAge);
        PerceptionComp->ConfigureSense(*DamageConfig);
    }
}

void UForge2AIManager::SetPatrolRoute(AActor* AIActor, const FForge2PatrolRoute& Route)
{
    if (!AIActor) return;

    FString ActorKey = AIActor->GetName();
    PatrolRoutes.Add(ActorKey, Route);
    PatrolIndices.Add(ActorKey, 0);
}

FVector UForge2AIManager::GetNextPatrolPoint(AActor* AIActor)
{
    if (!AIActor) return FVector::ZeroVector;

    FString ActorKey = AIActor->GetName();
    FForge2PatrolRoute* Route = PatrolRoutes.Find(ActorKey);
    int32* CurrentIndex = PatrolIndices.Find(ActorKey);

    if (!Route || !CurrentIndex || Route->Points.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector NextPoint = Route->Points[*CurrentIndex].Location;

    // Advance index
    if (Route->bRandomize)
    {
        *CurrentIndex = FMath::RandRange(0, Route->Points.Num() - 1);
    }
    else
    {
        (*CurrentIndex)++;
        if (*CurrentIndex >= Route->Points.Num())
        {
            if (Route->bLoop)
            {
                *CurrentIndex = 0;
            }
            else if (Route->bReverseOnEnd)
            {
                // Reverse the array for back-and-forth patrol
                Algo::Reverse(Route->Points);
                *CurrentIndex = 1;  // Skip the current point
            }
            else
            {
                *CurrentIndex = Route->Points.Num() - 1;  // Stay at end
            }
        }
    }

    return NextPoint;
}

void UForge2AIManager::RegisterGOAPAction(AActor* AIActor, const FForge2GOAPAction& Action)
{
    if (!AIActor) return;
    
    FString ActorKey = AIActor->GetName();
    TArray<FForge2GOAPAction>& Actions = GOAPActions.FindOrAdd(ActorKey);
    
    // Check if action already exists, update if so
    bool bFound = false;
    for (FForge2GOAPAction& ExistingAction : Actions)
    {
        if (ExistingAction.ActionName == Action.ActionName)
        {
            ExistingAction = Action;
            bFound = true;
            break;
        }
    }
    
    if (!bFound)
    {
        Actions.Add(Action);
    }
}

void UForge2AIManager::SetGOAPGoal(AActor* AIActor, const FForge2GOAPGoal& Goal)
{
    if (!AIActor) return;
    
    FString ActorKey = AIActor->GetName();
    CurrentGOAPGoals.Add(ActorKey, Goal);
}

TArray<FString> UForge2AIManager::PlanGOAPActions(AActor* AIActor)
{
    TArray<FString> Plan;
    if (!AIActor) return Plan;

    FString ActorKey = AIActor->GetName();
    
    // Get current world state
    TMap<FName, bool>* WorldState = GOAPWorldStates.Find(ActorKey);
    FForge2GOAPGoal* Goal = CurrentGOAPGoals.Find(ActorKey);
    TArray<FForge2GOAPAction>* Actions = GOAPActions.Find(ActorKey);
    
    if (!WorldState || !Goal || !Actions) return Plan;

    // A* planning implementation
    struct FGOAPNode
    {
        TMap<FName, bool> State;
        TArray<FString> ActionsTaken;
        float GCost;
        float HCost;
        float FCost() const { return GCost + HCost; }
    };

    // Calculate heuristic - number of unsatisfied goal conditions
    auto CalculateHeuristic = [&Goal](const TMap<FName, bool>& State) -> float
    {
        float H = 0.0f;
        for (const auto& Condition : Goal->GoalState)
        {
            const bool* StateValue = State.Find(Condition.Key);
            if (!StateValue || *StateValue != Condition.Value)
            {
                H += 1.0f;
            }
        }
        return H;
    };

    // Check if goal is satisfied
    auto IsGoalSatisfied = [&Goal](const TMap<FName, bool>& State) -> bool
    {
        for (const auto& Condition : Goal->GoalState)
        {
            const bool* StateValue = State.Find(Condition.Key);
            if (!StateValue || *StateValue != Condition.Value)
            {
                return false;
            }
        }
        return true;
    };

    // Check if action preconditions are met
    auto CanExecuteAction = [](const FForge2GOAPAction& Action, const TMap<FName, bool>& State) -> bool
    {
        for (const auto& Precondition : Action.Preconditions)
        {
            const bool* StateValue = State.Find(Precondition.Key);
            if (!StateValue || *StateValue != Precondition.Value)
            {
                return false;
            }
        }
        return true;
    };

    // Apply action effects to state
    auto ApplyAction = [](const FForge2GOAPAction& Action, TMap<FName, bool> State) -> TMap<FName, bool>
    {
        for (const auto& Effect : Action.Effects)
        {
            State.Add(Effect.Key, Effect.Value);
        }
        return State;
    };

    // Initialize open list with start state
    TArray<FGOAPNode> OpenList;
    TSet<uint32> ClosedSet;

    FGOAPNode StartNode;
    StartNode.State = *WorldState;
    StartNode.GCost = 0.0f;
    StartNode.HCost = CalculateHeuristic(StartNode.State);
    OpenList.Add(StartNode);

    int32 MaxIterations = 1000;
    int32 Iterations = 0;

    while (OpenList.Num() > 0 && Iterations < MaxIterations)
    {
        Iterations++;

        // Find node with lowest F cost
        int32 BestIndex = 0;
        float BestF = OpenList[0].FCost();
        for (int32 i = 1; i < OpenList.Num(); ++i)
        {
            if (OpenList[i].FCost() < BestF)
            {
                BestF = OpenList[i].FCost();
                BestIndex = i;
            }
        }

        FGOAPNode CurrentNode = OpenList[BestIndex];
        OpenList.RemoveAt(BestIndex);

        // Check if goal reached
        if (IsGoalSatisfied(CurrentNode.State))
        {
            Plan = CurrentNode.ActionsTaken;
            break;
        }

        // Generate hash for closed set
        uint32 StateHash = GetTypeHash(CurrentNode.ActionsTaken.Num());
        for (const auto& Pair : CurrentNode.State)
        {
            StateHash = HashCombine(StateHash, GetTypeHash(Pair.Key));
            StateHash = HashCombine(StateHash, GetTypeHash(Pair.Value));
        }

        if (ClosedSet.Contains(StateHash))
        {
            continue;
        }
        ClosedSet.Add(StateHash);

        // Expand neighbors (try all valid actions)
        for (const FForge2GOAPAction& Action : *Actions)
        {
            if (CanExecuteAction(Action, CurrentNode.State))
            {
                FGOAPNode NewNode;
                NewNode.State = ApplyAction(Action, CurrentNode.State);
                NewNode.ActionsTaken = CurrentNode.ActionsTaken;
                NewNode.ActionsTaken.Add(Action.ActionName);
                NewNode.GCost = CurrentNode.GCost + Action.Cost;
                NewNode.HCost = CalculateHeuristic(NewNode.State);

                OpenList.Add(NewNode);
            }
        }
    }

    return Plan;
}

void UForge2AIManager::SetGOAPWorldState(AActor* AIActor, FName Key, bool Value)
{
    if (!AIActor) return;
    
    FString ActorKey = AIActor->GetName();
    TMap<FName, bool>& WorldState = GOAPWorldStates.FindOrAdd(ActorKey);
    WorldState.Add(Key, Value);
}

void UForge2AIManager::RegisterUtilityAction(AActor* AIActor, const FForge2UtilityAction& Action)
{
    if (!AIActor) return;
    
    FString ActorKey = AIActor->GetName();
    TArray<FForge2UtilityAction>& Actions = UtilityActions.FindOrAdd(ActorKey);
    
    // Check if action already exists, update if so
    bool bFound = false;
    for (FForge2UtilityAction& ExistingAction : Actions)
    {
        if (ExistingAction.ActionName == Action.ActionName)
        {
            ExistingAction = Action;
            bFound = true;
            break;
        }
    }
    
    if (!bFound)
    {
        Actions.Add(Action);
    }
}

void UForge2AIManager::SetUtilityConsideration(AActor* AIActor, FName Key, float Value)
{
    if (!AIActor) return;
    
    FString ActorKey = AIActor->GetName();
    TMap<FName, float>& Considerations = UtilityConsiderations.FindOrAdd(ActorKey);
    Considerations.Add(Key, FMath::Clamp(Value, 0.0f, 1.0f));
}

FString UForge2AIManager::EvaluateUtilityActions(AActor* AIActor)
{
    if (!AIActor) return TEXT("");

    FString ActorKey = AIActor->GetName();
    TArray<FForge2UtilityAction>* Actions = UtilityActions.Find(ActorKey);
    TMap<FName, float>* Considerations = UtilityConsiderations.Find(ActorKey);
    FString* CurrentAction = CurrentUtilityActions.Find(ActorKey);
    
    if (!Actions || Actions->Num() == 0) return TEXT("");
    if (!Considerations) return TEXT("");

    FString BestAction = TEXT("");
    float BestScore = -1.0f;

    for (const FForge2UtilityAction& Action : *Actions)
    {
        // Calculate score by multiplying all consideration values
        float Score = Action.Weight;
        bool bValidAction = true;

        for (const FName& ConsiderationName : Action.ConsiderationNames)
        {
            const float* ConsiderationValue = Considerations->Find(ConsiderationName);
            if (ConsiderationValue)
            {
                Score *= *ConsiderationValue;
            }
            else
            {
                // Missing consideration, mark invalid
                bValidAction = false;
                break;
            }
        }

        if (!bValidAction) continue;

        // Apply momentum bonus if this is the current action
        if (CurrentAction && *CurrentAction == Action.ActionName)
        {
            Score *= (1.0f + Action.MomentumBonus);
        }

        // Apply response curve (simple power curve)
        Score = FMath::Pow(Score, Action.ResponseCurveExponent);

        if (Score > BestScore)
        {
            BestScore = Score;
            BestAction = Action.ActionName;
        }
    }

    // Update current action for momentum tracking
    if (!BestAction.IsEmpty())
    {
        CurrentUtilityActions.Add(ActorKey, BestAction);
    }

    return BestAction;
}

void UForge2AIManager::AddInfluenceSource(const FForge2InfluenceSource& Source)
{
    InfluenceSources.Add(Source);
}

void UForge2AIManager::RemoveInfluenceSource(AActor* SourceActor, FName InfluenceType)
{
    InfluenceSources.RemoveAll([SourceActor, InfluenceType](const FForge2InfluenceSource& Source)
    {
        return Source.SourceActor == SourceActor && Source.InfluenceType == InfluenceType;
    });
}

float UForge2AIManager::GetInfluenceAtLocation(FVector Location, FName InfluenceType)
{
    float TotalInfluence = 0.0f;

    for (const FForge2InfluenceSource& Source : InfluenceSources)
    {
        if (Source.InfluenceType != InfluenceType) continue;

        float Distance = FVector::Dist(Location, Source.Location);
        if (Distance < Source.Radius)
        {
            // Linear falloff
            float Contribution = Source.Strength * (1.0f - (Distance / Source.Radius));
            TotalInfluence += Contribution;
        }
    }

    return TotalInfluence;
}

FVector UForge2AIManager::FindBestInfluenceLocation(FVector Origin, float SearchRadius, FName InfluenceType, bool bMaximize)
{
    FVector BestLocation = Origin;
    float BestInfluence = bMaximize ? -FLT_MAX : FLT_MAX;

    // Sample grid
    const int32 GridSize = 10;
    float Step = (SearchRadius * 2) / GridSize;

    for (int32 X = 0; X < GridSize; ++X)
    {
        for (int32 Y = 0; Y < GridSize; ++Y)
        {
            FVector SampleLocation = Origin + FVector(
                (X - GridSize / 2) * Step,
                (Y - GridSize / 2) * Step,
                0.0f
            );

            float Influence = GetInfluenceAtLocation(SampleLocation, InfluenceType);

            if ((bMaximize && Influence > BestInfluence) || (!bMaximize && Influence < BestInfluence))
            {
                BestInfluence = Influence;
                BestLocation = SampleLocation;
            }
        }
    }

    return BestLocation;
}

void UForge2AIManager::CreateSquad(const FForge2Squad& Squad)
{
    Squads.Add(Squad.SquadName, Squad);
}

void UForge2AIManager::AddToSquad(const FString& SquadName, AActor* Member, EForge2SquadRole Role)
{
    FForge2Squad* Squad = Squads.Find(SquadName);
    if (!Squad || !Member) return;

    FForge2SquadMember NewMember;
    NewMember.Actor = Member;
    NewMember.Role = Role;
    NewMember.bIsAlive = true;
    NewMember.FormationOffset = CalculateFormationOffset(
        Squad->Formation,
        Squad->Members.Num(),
        Squad->Members.Num() + 1,
        Squad->FormationSpacing
    );

    Squad->Members.Add(NewMember);
}

void UForge2AIManager::RemoveFromSquad(const FString& SquadName, AActor* Member)
{
    FForge2Squad* Squad = Squads.Find(SquadName);
    if (!Squad) return;

    Squad->Members.RemoveAll([Member](const FForge2SquadMember& M) { return M.Actor == Member; });
}

void UForge2AIManager::SetSquadFormation(const FString& SquadName, EForge2FormationType Formation)
{
    FForge2Squad* Squad = Squads.Find(SquadName);
    if (!Squad) return;

    Squad->Formation = Formation;

    // Recalculate all formation offsets
    for (int32 i = 0; i < Squad->Members.Num(); ++i)
    {
        Squad->Members[i].FormationOffset = CalculateFormationOffset(
            Formation,
            i,
            Squad->Members.Num(),
            Squad->FormationSpacing
        );
    }
}

void UForge2AIManager::OrderSquadMoveTo(const FString& SquadName, FVector Location)
{
    FForge2Squad* Squad = Squads.Find(SquadName);
    if (!Squad) return;

    Squad->CurrentTarget = Location;
    Squad->SquadState = EForge2AIState::Patrol;

    // Notify all squad members of new destination via their AI controllers
    for (FForge2SquadMember& Member : Squad->Members)
    {
        if (!Member.Actor || !Member.bIsAlive) continue;
        
        APawn* MemberPawn = Cast<APawn>(Member.Actor);
        if (MemberPawn)
        {
            AAIController* AIController = Cast<AAIController>(MemberPawn->GetController());
            if (AIController)
            {
                // Calculate this member's formation position
                FVector FormationPosition = Location + Member.FormationOffset;
                
                // Set blackboard value for destination
                UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
                if (BlackboardComp)
                {
                    BlackboardComp->SetValueAsVector(TEXT("MoveToLocation"), FormationPosition);
                    BlackboardComp->SetValueAsVector(TEXT("SquadTarget"), Location);
                    BlackboardComp->SetValueAsBool(TEXT("HasSquadOrder"), true);
                }
                
                // Direct move command as fallback
                AIController->MoveToLocation(FormationPosition, 50.0f, true, true, false, true);
            }
        }
    }
}

void UForge2AIManager::OrderSquadAttack(const FString& SquadName, AActor* Target)
{
    FForge2Squad* Squad = Squads.Find(SquadName);
    if (!Squad || !Target) return;

    Squad->CurrentTarget = Target->GetActorLocation();
    Squad->SquadState = EForge2AIState::Combat;

    // Notify all squad members to attack target via AI controllers
    for (FForge2SquadMember& Member : Squad->Members)
    {
        if (!Member.Actor || !Member.bIsAlive) continue;
        
        APawn* MemberPawn = Cast<APawn>(Member.Actor);
        if (MemberPawn)
        {
            AAIController* AIController = Cast<AAIController>(MemberPawn->GetController());
            if (AIController)
            {
                // Set blackboard values for combat
                UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
                if (BlackboardComp)
                {
                    BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Target);
                    BlackboardComp->SetValueAsVector(TEXT("TargetLocation"), Target->GetActorLocation());
                    BlackboardComp->SetValueAsBool(TEXT("CanSeeTarget"), true);
                    BlackboardComp->SetValueAsBool(TEXT("IsInCombat"), true);
                    BlackboardComp->SetValueAsFloat(TEXT("DistanceToTarget"), 
                        FVector::Dist(MemberPawn->GetActorLocation(), Target->GetActorLocation()));
                    
                    // Set role-specific behavior
                    switch (Member.Role)
                    {
                        case EForge2SquadRole::Leader:
                            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), 0.7f);
                            break;
                        case EForge2SquadRole::Assault:
                            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), 1.0f);
                            break;
                        case EForge2SquadRole::Support:
                            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), 0.3f);
                            break;
                        case EForge2SquadRole::Flanker:
                            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), 0.8f);
                            BlackboardComp->SetValueAsBool(TEXT("ShouldFlank"), true);
                            break;
                        default:
                            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), 0.5f);
                            break;
                    }
                }
                
                // Set focus on target
                AIController->SetFocus(Target);
            }
        }
    }
}

FVector UForge2AIManager::GetFormationPosition(const FString& SquadName, AActor* Member)
{
    FForge2Squad* Squad = Squads.Find(SquadName);
    if (!Squad) return FVector::ZeroVector;

    // Find leader
    AActor* Leader = nullptr;
    for (const FForge2SquadMember& M : Squad->Members)
    {
        if (M.Role == EForge2SquadRole::Leader && M.Actor && M.bIsAlive)
        {
            Leader = M.Actor;
            break;
        }
    }

    if (!Leader)
    {
        // No leader, use first alive member
        for (const FForge2SquadMember& M : Squad->Members)
        {
            if (M.Actor && M.bIsAlive)
            {
                Leader = M.Actor;
                break;
            }
        }
    }

    if (!Leader) return FVector::ZeroVector;

    // Find member offset
    for (const FForge2SquadMember& M : Squad->Members)
    {
        if (M.Actor == Member)
        {
            FVector LeaderLocation = Leader->GetActorLocation();
            FRotator LeaderRotation = Leader->GetActorRotation();

            // Rotate offset by leader's facing direction
            FVector WorldOffset = LeaderRotation.RotateVector(M.FormationOffset);
            return LeaderLocation + WorldOffset;
        }
    }

    return FVector::ZeroVector;
}

FVector UForge2AIManager::CalculateFormationOffset(EForge2FormationType Formation, int32 MemberIndex, int32 TotalMembers, float Spacing)
{
    switch (Formation)
    {
        case EForge2FormationType::Line:
            return FVector(0, (MemberIndex - TotalMembers / 2) * Spacing, 0);

        case EForge2FormationType::Column:
            return FVector(-MemberIndex * Spacing, 0, 0);

        case EForge2FormationType::Wedge:
            {
                int32 Row = MemberIndex;
                float Offset = (Row % 2 == 0) ? 1.0f : -1.0f;
                return FVector(-Row * Spacing * 0.7f, Offset * (Row / 2 + 1) * Spacing * 0.5f, 0);
            }

        case EForge2FormationType::Circle:
            {
                float Angle = (2 * PI * MemberIndex) / TotalMembers;
                return FVector(FMath::Cos(Angle) * Spacing, FMath::Sin(Angle) * Spacing, 0);
            }

        case EForge2FormationType::Staggered:
            {
                int32 Row = MemberIndex / 2;
                bool bLeftSide = (MemberIndex % 2 == 0);
                return FVector(-Row * Spacing, (bLeftSide ? -1 : 1) * Spacing * 0.5f, 0);
            }

        default:
            return FVector::ZeroVector;
    }
}

void UForge2AIManager::SetupBossPhases(AActor* BossActor, const TArray<FForge2BossPhase>& Phases)
{
    if (!BossActor) return;

    BossPhases.Add(BossActor, Phases);
    CurrentBossPhaseIndex.Add(BossActor, 0);
}

void UForge2AIManager::TransitionBossPhase(AActor* BossActor, const FString& PhaseName)
{
    TArray<FForge2BossPhase>* Phases = BossPhases.Find(BossActor);
    if (!Phases) return;

    for (int32 i = 0; i < Phases->Num(); ++i)
    {
        if ((*Phases)[i].PhaseName == PhaseName)
        {
            CurrentBossPhaseIndex.Add(BossActor, i);
            FForge2BossPhase& Phase = (*Phases)[i];

            // Get the AI controller for this boss
            APawn* BossPawn = Cast<APawn>(BossActor);
            if (BossPawn)
            {
                AAIController* AIController = Cast<AAIController>(BossPawn->GetController());
                if (AIController)
                {
                    // 1. Play transition animation via blackboard trigger
                    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
                    if (BlackboardComp)
                    {
                        BlackboardComp->SetValueAsName(TEXT("CurrentPhase"), FName(*PhaseName));
                        BlackboardComp->SetValueAsBool(TEXT("IsTransitioning"), true);
                        BlackboardComp->SetValueAsFloat(TEXT("DamageMultiplier"), Phase.DamageMultiplier);
                        BlackboardComp->SetValueAsFloat(TEXT("AggressionMultiplier"), Phase.AggressionMultiplier);
                        BlackboardComp->SetValueAsFloat(TEXT("SpeedMultiplier"), Phase.SpeedMultiplier);
                        
                        // Set enabled abilities as a bitmask or individual bools
                        for (const FString& AbilityName : Phase.EnabledAbilities)
                        {
                            BlackboardComp->SetValueAsBool(FName(*FString::Printf(TEXT("Ability_%s"), *AbilityName)), true);
                        }
                        
                        // Set disabled abilities
                        for (const FString& AbilityName : Phase.DisabledAbilities)
                        {
                            BlackboardComp->SetValueAsBool(FName(*FString::Printf(TEXT("Ability_%s"), *AbilityName)), false);
                        }
                    }

                    // 2. Switch behavior tree if specified
                    if (Phase.PhaseBehaviorTree.IsValid())
                    {
                        UBehaviorTree* NewBT = Phase.PhaseBehaviorTree.LoadSynchronous();
                        if (NewBT)
                        {
                            UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIController->GetBrainComponent());
                            if (BTComp)
                            {
                                BTComp->StopTree(EBTStopMode::Safe);
                                BTComp->StartTree(*NewBT, EBTExecutionMode::Looped);
                            }
                            else
                            {
                                AIController->RunBehaviorTree(NewBT);
                            }
                        }
                    }

                    // 3. Apply speed multiplier to movement
                    ACharacter* BossCharacter = Cast<ACharacter>(BossPawn);
                    if (BossCharacter)
                    {
                        UCharacterMovementComponent* MovementComp = BossCharacter->GetCharacterMovement();
                        if (MovementComp)
                        {
                            MovementComp->MaxWalkSpeed *= Phase.SpeedMultiplier;
                        }
                    }
                }
            }

            // Broadcast phase change event
            OnBossPhaseChanged.Broadcast(BossActor, PhaseName, i);
            
            break;
        }
    }
}

void UForge2AIManager::CheckBossPhaseTransition(AActor* BossActor, float CurrentHealthPercent)
{
    TArray<FForge2BossPhase>* Phases = BossPhases.Find(BossActor);
    int32* CurrentPhase = CurrentBossPhaseIndex.Find(BossActor);

    if (!Phases || !CurrentPhase) return;

    // Check if we should transition to next phase
    for (int32 i = *CurrentPhase + 1; i < Phases->Num(); ++i)
    {
        if (CurrentHealthPercent <= (*Phases)[i].HealthThreshold)
        {
            TransitionBossPhase(BossActor, (*Phases)[i].PhaseName);
            break;
        }
    }
}

void UForge2AIManager::SetAIDifficulty(AActor* AIActor, float DifficultyScale)
{
    if (!AIActor) return;
    
    APawn* AIPawn = Cast<APawn>(AIActor);
    if (!AIPawn) return;
    
    AAIController* AIController = Cast<AAIController>(AIPawn->GetController());
    if (AIController)
    {
        UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
        if (BlackboardComp)
        {
            // Set difficulty-related blackboard values
            BlackboardComp->SetValueAsFloat(TEXT("DifficultyScale"), DifficultyScale);
            
            // Reaction time: lower difficulty = slower reactions (higher value)
            float ReactionTime = FMath::Lerp(1.0f, 0.1f, DifficultyScale);
            BlackboardComp->SetValueAsFloat(TEXT("ReactionTime"), ReactionTime);
            
            // Accuracy: higher difficulty = better accuracy
            float Accuracy = FMath::Lerp(0.3f, 0.95f, DifficultyScale);
            BlackboardComp->SetValueAsFloat(TEXT("Accuracy"), Accuracy);
            
            // Aggression: higher difficulty = more aggressive
            float Aggression = FMath::Lerp(0.3f, 0.9f, DifficultyScale);
            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), Aggression);
        }
    }
    
    // Store difficulty for this actor
    AIDifficultyLevels.Add(AIActor->GetName(), DifficultyScale);
}

void UForge2AIManager::ConfigureAIAggression(AActor* AIActor, float AggressionLevel)
{
    if (!AIActor) return;
    
    float ClampedAggression = FMath::Clamp(AggressionLevel, 0.0f, 1.0f);
    
    APawn* AIPawn = Cast<APawn>(AIActor);
    if (!AIPawn) return;
    
    AAIController* AIController = Cast<AAIController>(AIPawn->GetController());
    if (AIController)
    {
        UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), ClampedAggression);
            
            // Aggression affects:
            // Attack frequency: higher aggression = attack more often
            float AttackCooldown = FMath::Lerp(3.0f, 0.5f, ClampedAggression);
            BlackboardComp->SetValueAsFloat(TEXT("AttackCooldown"), AttackCooldown);
            
            // Pursuit distance: higher aggression = chase further
            float PursuitDistance = FMath::Lerp(500.0f, 3000.0f, ClampedAggression);
            BlackboardComp->SetValueAsFloat(TEXT("PursuitDistance"), PursuitDistance);
            
            // Retreat threshold: higher aggression = retreat at lower health
            float RetreatThreshold = FMath::Lerp(0.5f, 0.1f, ClampedAggression);
            BlackboardComp->SetValueAsFloat(TEXT("RetreatHealthThreshold"), RetreatThreshold);
            
            // Engagement range: higher aggression = engage from further
            float EngagementRange = FMath::Lerp(500.0f, 1500.0f, ClampedAggression);
            BlackboardComp->SetValueAsFloat(TEXT("EngagementRange"), EngagementRange);
        }
    }
    
    // Store aggression for this actor
    AIAggressionLevels.Add(AIActor->GetName(), ClampedAggression);
}

// UForge2AIUtility Implementation
FForge2PerceptionConfig UForge2AIUtility::CreateDefaultPerception()
{
    FForge2PerceptionConfig Config;
    Config.bEnableSight = true;
    Config.SightConfig.SightRadius = 2000.0f;
    Config.SightConfig.LoseSightRadius = 2500.0f;
    Config.SightConfig.PeripheralVisionAngle = 90.0f;
    Config.bEnableHearing = true;
    Config.HearingConfig.HearingRange = 1500.0f;
    Config.bEnableDamage = true;
    return Config;
}

FForge2PerceptionConfig UForge2AIUtility::CreateStealthPerception()
{
    FForge2PerceptionConfig Config;
    Config.bEnableSight = true;
    Config.SightConfig.SightRadius = 3000.0f;
    Config.SightConfig.LoseSightRadius = 3500.0f;
    Config.SightConfig.PeripheralVisionAngle = 60.0f;  // Narrower cone
    Config.SightConfig.MaxAge = 8.0f;  // Remember longer
    Config.bEnableHearing = true;
    Config.HearingConfig.HearingRange = 1000.0f;  // Reduced hearing
    Config.bEnableDamage = true;
    return Config;
}

FForge2PerceptionConfig UForge2AIUtility::CreateGuardPerception()
{
    FForge2PerceptionConfig Config;
    Config.bEnableSight = true;
    Config.SightConfig.SightRadius = 2500.0f;
    Config.SightConfig.LoseSightRadius = 3000.0f;
    Config.SightConfig.PeripheralVisionAngle = 120.0f;  // Wide cone
    Config.bEnableHearing = true;
    Config.HearingConfig.HearingRange = 2000.0f;
    Config.HearingConfig.bDetectNeutrals = true;
    Config.bEnableDamage = true;
    return Config;
}

FForge2PatrolRoute UForge2AIUtility::CreateSimplePatrolRoute(const TArray<FVector>& Points, float WaitTime)
{
    FForge2PatrolRoute Route;
    Route.bLoop = true;
    Route.MovementSpeed = 300.0f;

    for (const FVector& Point : Points)
    {
        FForge2PatrolPoint PatrolPoint;
        PatrolPoint.Location = Point;
        PatrolPoint.WaitTime = WaitTime;
        PatrolPoint.bLookAround = true;
        Route.Points.Add(PatrolPoint);
    }

    return Route;
}

TArray<FForge2BlackboardKeyDef> UForge2AIUtility::CreateCombatBlackboardKeys()
{
    TArray<FForge2BlackboardKeyDef> Keys;

    FForge2BlackboardKeyDef TargetKey;
    TargetKey.KeyName = TEXT("TargetActor");
    TargetKey.KeyType = EForge2BlackboardKeyType::Object;
    TargetKey.BaseClass = AActor::StaticClass();
    Keys.Add(TargetKey);

    FForge2BlackboardKeyDef TargetLocation;
    TargetLocation.KeyName = TEXT("TargetLocation");
    TargetLocation.KeyType = EForge2BlackboardKeyType::Vector;
    Keys.Add(TargetLocation);

    FForge2BlackboardKeyDef LastKnownLocation;
    LastKnownLocation.KeyName = TEXT("LastKnownLocation");
    LastKnownLocation.KeyType = EForge2BlackboardKeyType::Vector;
    Keys.Add(LastKnownLocation);

    FForge2BlackboardKeyDef CanSeeTarget;
    CanSeeTarget.KeyName = TEXT("CanSeeTarget");
    CanSeeTarget.KeyType = EForge2BlackboardKeyType::Bool;
    Keys.Add(CanSeeTarget);

    FForge2BlackboardKeyDef DistanceToTarget;
    DistanceToTarget.KeyName = TEXT("DistanceToTarget");
    DistanceToTarget.KeyType = EForge2BlackboardKeyType::Float;
    Keys.Add(DistanceToTarget);

    FForge2BlackboardKeyDef CoverLocation;
    CoverLocation.KeyName = TEXT("CoverLocation");
    CoverLocation.KeyType = EForge2BlackboardKeyType::Vector;
    Keys.Add(CoverLocation);

    return Keys;
}

TArray<FForge2BlackboardKeyDef> UForge2AIUtility::CreatePatrolBlackboardKeys()
{
    TArray<FForge2BlackboardKeyDef> Keys;

    FForge2BlackboardKeyDef PatrolLocation;
    PatrolLocation.KeyName = TEXT("PatrolLocation");
    PatrolLocation.KeyType = EForge2BlackboardKeyType::Vector;
    Keys.Add(PatrolLocation);

    FForge2BlackboardKeyDef HomeLocation;
    HomeLocation.KeyName = TEXT("HomeLocation");
    HomeLocation.KeyType = EForge2BlackboardKeyType::Vector;
    Keys.Add(HomeLocation);

    FForge2BlackboardKeyDef InvestigateLocation;
    InvestigateLocation.KeyName = TEXT("InvestigateLocation");
    InvestigateLocation.KeyType = EForge2BlackboardKeyType::Vector;
    Keys.Add(InvestigateLocation);

    FForge2BlackboardKeyDef IsAlerted;
    IsAlerted.KeyName = TEXT("IsAlerted");
    IsAlerted.KeyType = EForge2BlackboardKeyType::Bool;
    Keys.Add(IsAlerted);

    FForge2BlackboardKeyDef WaitTime;
    WaitTime.KeyName = TEXT("WaitTime");
    WaitTime.KeyType = EForge2BlackboardKeyType::Float;
    Keys.Add(WaitTime);

    return Keys;
}
