// Forge2WorldGenPCG.cpp
// FORGE2 Ultimate AAA Game Development MCP Extension
// PCG Framework Integration Implementation

#include "Forge2WorldGenPCG.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// PCG includes
#include "PCGGraph.h"
#include "PCGNode.h"
#include "PCGPin.h"
#include "PCGComponent.h"
#include "PCGVolume.h"
#include "PCGSubsystem.h"
#include "PCGSettings.h"

// PCG Settings includes (specific node types)
#include "Elements/PCGSurfaceSampler.h"
#include "Elements/PCGStaticMeshSpawner.h"
#include "Elements/PCGPointFilter.h"
#include "Elements/PCGDensityFilter.h"
#include "Elements/PCGTransformPoints.h"
#include "Elements/PCGAttributeNoise.h"
#include "Elements/PCGAttributeOperation.h"
#include "Elements/Metadata/PCGMetadataBreakVector.h"
#include "Elements/PCGProjectionParams.h"

// ============================================================================
// FForge2PCGGraphManager Implementation
// ============================================================================

FForge2PCGGraphManager::FForge2PCGGraphManager()
{
}

UPCGGraph* FForge2PCGGraphManager::CreateGraph(const FString& AssetPath, const FString& GraphName)
{
#if WITH_EDITOR
    // Create package
    FString PackagePath = AssetPath;
    if (!PackagePath.EndsWith(TEXT("/")))
    {
        PackagePath += TEXT("/");
    }
    PackagePath += GraphName;

    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create package for PCG graph: %s"), *PackagePath);
        return nullptr;
    }

    // Create graph
    UPCGGraph* Graph = NewObject<UPCGGraph>(Package, *GraphName, RF_Public | RF_Standalone);
    if (!Graph)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create PCG graph: %s"), *GraphName);
        return nullptr;
    }

    // Mark package dirty
    Package->MarkPackageDirty();

    // Register with asset registry
    FAssetRegistryModule::AssetCreated(Graph);

    return Graph;
#else
    return nullptr;
#endif
}

UPCGGraph* FForge2PCGGraphManager::LoadGraph(const FString& AssetPath)
{
    return LoadObject<UPCGGraph>(nullptr, *AssetPath);
}

bool FForge2PCGGraphManager::SaveGraph(UPCGGraph* Graph)
{
#if WITH_EDITOR
    if (!Graph)
    {
        return false;
    }

    UPackage* Package = Graph->GetOutermost();
    if (!Package)
    {
        return false;
    }

    FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Standalone;
    SaveArgs.Error = GError;

    return UPackage::SavePackage(Package, Graph, *PackageFileName, SaveArgs);
#else
    return false;
#endif
}

UPCGNode* FForge2PCGGraphManager::AddNode(UPCGGraph* Graph, const FForge2PCGNodeConfig& NodeConfig)
{
    if (!Graph)
    {
        return nullptr;
    }

    TSubclassOf<UPCGSettings> SettingsClass = GetSettingsClassForNodeType(NodeConfig.NodeType);
    if (!SettingsClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unknown PCG node type, using default settings"));
        return nullptr;
    }

    // Create settings
    UPCGSettings* Settings = NewObject<UPCGSettings>(Graph, SettingsClass);
    if (!Settings)
    {
        return nullptr;
    }

    // Configure base settings
    ConfigureNodeSettings(Settings, NodeConfig.NodeType);

    // Add node to graph
    UPCGNode* Node = Graph->AddNode(Settings);
    if (Node)
    {
        Node->NodeTitle = FName(*NodeConfig.NodeName);
        Node->PositionX = NodeConfig.GraphPosition.X;
        Node->PositionY = NodeConfig.GraphPosition.Y;
        Node->NodeComment = NodeConfig.Comment;

        if (!NodeConfig.bEnabled)
        {
            // Disable node (implementation depends on PCG version)
            // Settings->bEnabled = false;
        }
    }

    return Node;
}

UPCGNode* FForge2PCGGraphManager::AddSurfaceSamplerNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGSurfaceSamplerConfig& Config)
{
    if (!Graph)
    {
        return nullptr;
    }

    UPCGSurfaceSamplerSettings* Settings = NewObject<UPCGSurfaceSamplerSettings>(Graph);
    if (!Settings)
    {
        return nullptr;
    }

    // Configure surface sampler
    Settings->PointsPerSquaredMeter = Config.PointsPerSquareMeter;
    Settings->PointExtents = FVector(Config.MinDistance * 0.5f);
    Settings->bApplyDensityToPoints = Config.bApplyDensity;
    Settings->Seed = Config.Seed;
    Settings->bUnbounded = Config.bUnbounded;
    Settings->LooseBounds = Config.LooseBoundsFactor;

    UPCGNode* Node = Graph->AddNode(Settings);
    if (Node)
    {
        Node->NodeTitle = FName(*NodeName);
    }

    return Node;
}

UPCGNode* FForge2PCGGraphManager::AddMeshSpawnerNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGMeshSpawnerConfig& Config)
{
    if (!Graph)
    {
        return nullptr;
    }

    UPCGStaticMeshSpawnerSettings* Settings = NewObject<UPCGStaticMeshSpawnerSettings>(Graph);
    if (!Settings)
    {
        return nullptr;
    }

    // Configure mesh spawner
    // Note: The actual mesh setup would require loading static mesh assets
    // This is a simplified version

    // For actual implementation, you would:
    // 1. Load meshes from MeshPaths
    // 2. Create FPCGStaticMeshSpawnerEntry for each
    // 3. Set weights from Config.MeshWeights

    UPCGNode* Node = Graph->AddNode(Settings);
    if (Node)
    {
        Node->NodeTitle = FName(*NodeName);
    }

    return Node;
}

UPCGNode* FForge2PCGGraphManager::AddPointFilterNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGPointFilterConfig& Config)
{
    if (!Graph)
    {
        return nullptr;
    }

    UPCGPointFilterSettings* Settings = NewObject<UPCGPointFilterSettings>(Graph);
    if (!Settings)
    {
        return nullptr;
    }

    // Configure point filter
    // Note: PCG Point Filter configuration depends on the specific UE version

    UPCGNode* Node = Graph->AddNode(Settings);
    if (Node)
    {
        Node->NodeTitle = FName(*NodeName);
    }

    return Node;
}

UPCGNode* FForge2PCGGraphManager::AddDensityFilterNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGDensityFilterConfig& Config)
{
    if (!Graph)
    {
        return nullptr;
    }

    UPCGDensityFilterSettings* Settings = NewObject<UPCGDensityFilterSettings>(Graph);
    if (!Settings)
    {
        return nullptr;
    }

    // Configure density filter
    Settings->LowerBound = Config.LowerBound;
    Settings->UpperBound = Config.UpperBound;
    Settings->bInvertFilter = Config.bInvert;

    UPCGNode* Node = Graph->AddNode(Settings);
    if (Node)
    {
        Node->NodeTitle = FName(*NodeName);
    }

    return Node;
}

UPCGNode* FForge2PCGGraphManager::AddPointTransformNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGPointTransformConfig& Config)
{
    if (!Graph)
    {
        return nullptr;
    }

    UPCGTransformPointsSettings* Settings = NewObject<UPCGTransformPointsSettings>(Graph);
    if (!Settings)
    {
        return nullptr;
    }

    // Configure transform
    Settings->bApplyToAttribute = false;
    Settings->OffsetMin = Config.OffsetMin;
    Settings->OffsetMax = Config.OffsetMax;
    Settings->RotationMin = Config.RotationMin;
    Settings->RotationMax = Config.RotationMax;
    Settings->ScaleMin = Config.ScaleMin;
    Settings->ScaleMax = Config.ScaleMax;
    Settings->bAbsoluteOffset = Config.bAbsoluteOffset;
    Settings->bAbsoluteRotation = Config.bAbsoluteRotation;
    Settings->bAbsoluteScale = Config.bAbsoluteScale;

    UPCGNode* Node = Graph->AddNode(Settings);
    if (Node)
    {
        Node->NodeTitle = FName(*NodeName);
    }

    return Node;
}

UPCGNode* FForge2PCGGraphManager::AddNoiseNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGNoiseConfig& Config)
{
    if (!Graph)
    {
        return nullptr;
    }

    // Create noise settings (attribute noise)
    UPCGAttributeNoiseSettings* Settings = NewObject<UPCGAttributeNoiseSettings>(Graph);
    if (!Settings)
    {
        return nullptr;
    }

    // Configure noise
    // Note: Configuration depends on specific PCG version

    UPCGNode* Node = Graph->AddNode(Settings);
    if (Node)
    {
        Node->NodeTitle = FName(*NodeName);
    }

    return Node;
}

UPCGNode* FForge2PCGGraphManager::AddAttributeOpNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGAttributeOpConfig& Config)
{
    if (!Graph)
    {
        return nullptr;
    }

    // Note: UPCGAttributeOperationSettings may vary by engine version
    // This is a placeholder implementation

    UPCGNode* Node = nullptr;
    // Implementation would create appropriate attribute operation node

    return Node;
}

bool FForge2PCGGraphManager::ConnectNodes(UPCGGraph* Graph, const FForge2PCGConnection& Connection)
{
    return ConnectNodes(Graph, Connection.SourceNodeName, Connection.SourcePinName, Connection.TargetNodeName, Connection.TargetPinName);
}

bool FForge2PCGGraphManager::ConnectNodes(UPCGGraph* Graph, const FString& SourceNodeName, const FString& SourcePinName, const FString& TargetNodeName, const FString& TargetPinName)
{
    if (!Graph)
    {
        return false;
    }

    UPCGNode* SourceNode = FindNodeByName(Graph, SourceNodeName);
    UPCGNode* TargetNode = FindNodeByName(Graph, TargetNodeName);

    if (!SourceNode || !TargetNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not find source or target node for connection"));
        return false;
    }

    // Find output pin on source
    UPCGPin* OutputPin = nullptr;
    for (UPCGPin* Pin : SourceNode->GetOutputPins())
    {
        if (Pin->Properties.Label.ToString() == SourcePinName || SourcePinName.IsEmpty())
        {
            OutputPin = Pin;
            break;
        }
    }

    // Find input pin on target
    UPCGPin* InputPin = nullptr;
    for (UPCGPin* Pin : TargetNode->GetInputPins())
    {
        if (Pin->Properties.Label.ToString() == TargetPinName || TargetPinName.IsEmpty())
        {
            InputPin = Pin;
            break;
        }
    }

    if (!OutputPin || !InputPin)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not find pins for connection"));
        return false;
    }

    // Create connection
    return Graph->AddLabeledEdge(SourceNode, OutputPin->Properties.Label, TargetNode, InputPin->Properties.Label);
}

UPCGNode* FForge2PCGGraphManager::FindNodeByName(UPCGGraph* Graph, const FString& NodeName)
{
    if (!Graph)
    {
        return nullptr;
    }

    const TArray<TObjectPtr<UPCGNode>>& Nodes = Graph->GetNodes();
    for (UPCGNode* Node : Nodes)
    {
        if (Node && Node->NodeTitle.ToString() == NodeName)
        {
            return Node;
        }
    }

    return nullptr;
}

bool FForge2PCGGraphManager::RemoveNode(UPCGGraph* Graph, const FString& NodeName)
{
    if (!Graph)
    {
        return false;
    }

    UPCGNode* Node = FindNodeByName(Graph, NodeName);
    if (!Node)
    {
        return false;
    }

    Graph->RemoveNode(Node);
    return true;
}

void FForge2PCGGraphManager::ClearGraph(UPCGGraph* Graph)
{
    if (!Graph)
    {
        return;
    }

    // Remove all nodes
    TArray<TObjectPtr<UPCGNode>> NodesToRemove = Graph->GetNodes();
    for (UPCGNode* Node : NodesToRemove)
    {
        if (Node)
        {
            Graph->RemoveNode(Node);
        }
    }
}

void FForge2PCGGraphManager::ConfigureNodeSettings(UPCGSettings* Settings, EForge2PCGNodeType NodeType)
{
    if (!Settings)
    {
        return;
    }

    // Base configuration common to all nodes
    // Specific configuration is done in the Add*Node methods
}

TSubclassOf<UPCGSettings> FForge2PCGGraphManager::GetSettingsClassForNodeType(EForge2PCGNodeType NodeType)
{
    switch (NodeType)
    {
    case EForge2PCGNodeType::SurfaceSampler:
        return UPCGSurfaceSamplerSettings::StaticClass();
    case EForge2PCGNodeType::StaticMeshSpawner:
        return UPCGStaticMeshSpawnerSettings::StaticClass();
    case EForge2PCGNodeType::PointFilter:
        return UPCGPointFilterSettings::StaticClass();
    case EForge2PCGNodeType::DensityFilter:
        return UPCGDensityFilterSettings::StaticClass();
    case EForge2PCGNodeType::PointTransform:
        return UPCGTransformPointsSettings::StaticClass();
    case EForge2PCGNodeType::PointNoise:
        return UPCGAttributeNoiseSettings::StaticClass();
    default:
        return nullptr;
    }
}

// ============================================================================
// FForge2PCGExecutor Implementation
// ============================================================================

FForge2PCGExecutor::FForge2PCGExecutor()
{
}

bool FForge2PCGExecutor::ExecuteGraph(UPCGComponent* Component, const FForge2PCGExecutionSettings& Settings)
{
    if (!Component)
    {
        return false;
    }

    // Configure component
    Component->bGenerated = false;

    if (Settings.bForceRefresh)
    {
        Component->Cleanup(true);
    }

    // Generate
    Component->Generate(Settings.bForceRefresh);

    return true;
}

bool FForge2PCGExecutor::ExecuteGraphOnVolume(APCGVolume* Volume, const FForge2PCGExecutionSettings& Settings)
{
    if (!Volume)
    {
        return false;
    }

    UPCGComponent* Component = Volume->FindComponentByClass<UPCGComponent>();
    if (!Component)
    {
        return false;
    }

    return ExecuteGraph(Component, Settings);
}

bool FForge2PCGExecutor::ExecuteGraphInBounds(UWorld* World, UPCGGraph* Graph, const FBox& Bounds, const FForge2PCGExecutionSettings& Settings)
{
    if (!World || !Graph)
    {
        return false;
    }

    UPCGSubsystem* Subsystem = GetPCGSubsystem(World);
    if (!Subsystem)
    {
        return false;
    }

    // Create temporary actor with PCG component
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* TempActor = World->SpawnActor<AActor>(AActor::StaticClass(), Bounds.GetCenter(), FRotator::ZeroRotator, SpawnParams);
    if (!TempActor)
    {
        return false;
    }

    UPCGComponent* Component = FForge2PCGUtils::CreatePCGComponent(TempActor, Graph);
    if (!Component)
    {
        TempActor->Destroy();
        return false;
    }

    // Set bounds
    // Component->SetBounds(Bounds); // Depends on PCG version

    return ExecuteGraph(Component, Settings);
}

UPCGSubsystem* FForge2PCGExecutor::GetPCGSubsystem(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }

    return World->GetSubsystem<UPCGSubsystem>();
}

void FForge2PCGExecutor::CancelGeneration(UPCGComponent* Component)
{
    if (Component)
    {
        Component->CancelGeneration();
    }
}

bool FForge2PCGExecutor::IsGenerating(UPCGComponent* Component)
{
    if (!Component)
    {
        return false;
    }

    return Component->IsGenerating();
}

void FForge2PCGExecutor::CleanupGeneratedResources(UPCGComponent* Component)
{
    if (Component)
    {
        Component->Cleanup(true);
    }
}

// ============================================================================
// FForge2PCGUtils Implementation
// ============================================================================

UPCGComponent* FForge2PCGUtils::CreatePCGComponent(AActor* Actor, UPCGGraph* Graph)
{
    if (!Actor)
    {
        return nullptr;
    }

    UPCGComponent* Component = NewObject<UPCGComponent>(Actor);
    if (!Component)
    {
        return nullptr;
    }

    Actor->AddInstanceComponent(Component);
    Component->RegisterComponent();

    if (Graph)
    {
        Component->SetGraph(Graph);
    }

    return Component;
}

APCGVolume* FForge2PCGUtils::CreatePCGVolume(UWorld* World, UPCGGraph* Graph, const FBox& Bounds)
{
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APCGVolume* Volume = World->SpawnActor<APCGVolume>(APCGVolume::StaticClass(), Bounds.GetCenter(), FRotator::ZeroRotator, SpawnParams);
    if (!Volume)
    {
        return nullptr;
    }

    // Set volume size
    FVector Extents = Bounds.GetExtent();
    Volume->SetActorScale3D(Extents / 50.0f); // Adjust based on default volume size

    // Set graph
    UPCGComponent* Component = Volume->FindComponentByClass<UPCGComponent>();
    if (Component && Graph)
    {
        Component->SetGraph(Graph);
    }

    return Volume;
}

FString FForge2PCGUtils::GetSettingsClassName(EForge2PCGNodeType NodeType)
{
    switch (NodeType)
    {
    case EForge2PCGNodeType::SurfaceSampler: return TEXT("PCGSurfaceSamplerSettings");
    case EForge2PCGNodeType::StaticMeshSpawner: return TEXT("PCGStaticMeshSpawnerSettings");
    case EForge2PCGNodeType::PointFilter: return TEXT("PCGPointFilterSettings");
    case EForge2PCGNodeType::DensityFilter: return TEXT("PCGDensityFilterSettings");
    case EForge2PCGNodeType::PointTransform: return TEXT("PCGTransformPointsSettings");
    case EForge2PCGNodeType::PointNoise: return TEXT("PCGAttributeNoiseSettings");
    default: return TEXT("PCGSettings");
    }
}

FString FForge2PCGUtils::NodeTypeToSettingsType(EForge2PCGNodeType NodeType)
{
    return GetSettingsClassName(NodeType);
}

UPCGGraph* FForge2PCGUtils::CreateGraphFromDefinition(const FForge2PCGGraphDefinition& Definition)
{
    FForge2PCGGraphManager Manager;

    UPCGGraph* Graph = Manager.CreateGraph(Definition.GraphPath, Definition.GraphName);
    if (!Graph)
    {
        return nullptr;
    }

    // Add nodes
    for (const FForge2PCGNodeConfig& NodeConfig : Definition.Nodes)
    {
        Manager.AddNode(Graph, NodeConfig);
    }

    // Create connections
    for (const FForge2PCGConnection& Connection : Definition.Connections)
    {
        Manager.ConnectNodes(Graph, Connection);
    }

    // Save graph
    Manager.SaveGraph(Graph);

    return Graph;
}

FForge2PCGGraphDefinition FForge2PCGUtils::ExportGraphToDefinition(UPCGGraph* Graph)
{
    FForge2PCGGraphDefinition Definition;

    if (!Graph)
    {
        return Definition;
    }

    Definition.GraphName = Graph->GetName();
    Definition.GraphPath = Graph->GetPathName();

    // Export nodes
    const TArray<TObjectPtr<UPCGNode>>& Nodes = Graph->GetNodes();
    for (UPCGNode* Node : Nodes)
    {
        if (!Node)
            continue;

        FForge2PCGNodeConfig NodeConfig;
        NodeConfig.NodeName = Node->NodeTitle.ToString();
        NodeConfig.GraphPosition = FVector2D(Node->PositionX, Node->PositionY);
        NodeConfig.Comment = Node->NodeComment;

        // Determine node type from settings
        // This is simplified - actual implementation would check settings class

        Definition.Nodes.Add(NodeConfig);
    }

    // Export connections
    for (UPCGNode* Node : Nodes)
    {
        if (!Node)
            continue;

        for (UPCGPin* OutputPin : Node->GetOutputPins())
        {
            for (UPCGEdge* Edge : OutputPin->Edges)
            {
                if (!Edge)
                    continue;

                FForge2PCGConnection Connection;
                Connection.SourceNodeName = Node->NodeTitle.ToString();
                Connection.SourcePinName = OutputPin->Properties.Label.ToString();

                if (Edge->InboundNode)
                {
                    Connection.TargetNodeName = Edge->InboundNode->NodeTitle.ToString();
                }
                if (Edge->InboundPin)
                {
                    Connection.TargetPinName = Edge->InboundPin->Properties.Label.ToString();
                }

                Definition.Connections.Add(Connection);
            }
        }
    }

    return Definition;
}

FForge2PCGSurfaceSamplerConfig FForge2PCGUtils::GetDefaultSurfaceSamplerConfig()
{
    FForge2PCGSurfaceSamplerConfig Config;
    Config.PointsPerSquareMeter = 0.1f;
    Config.MinDistance = 100.0f;
    Config.bApplyDensity = true;
    Config.Seed = FMath::RandRange(0, 10000);
    return Config;
}

FForge2PCGMeshSpawnerConfig FForge2PCGUtils::GetDefaultVegetationSpawnerConfig()
{
    FForge2PCGMeshSpawnerConfig Config;
    Config.MinScale = FVector(0.8f);
    Config.MaxScale = FVector(1.2f);
    Config.MinRotation = FRotator(0, 0, 0);
    Config.MaxRotation = FRotator(0, 360, 0);
    Config.bAlignToSurface = true;
    Config.bUseInstancedStaticMesh = true;
    Config.bCastShadows = true;
    return Config;
}

FForge2PCGMeshSpawnerConfig FForge2PCGUtils::GetDefaultPropSpawnerConfig()
{
    FForge2PCGMeshSpawnerConfig Config;
    Config.MinScale = FVector(0.9f);
    Config.MaxScale = FVector(1.1f);
    Config.MinRotation = FRotator(-10, 0, -10);
    Config.MaxRotation = FRotator(10, 360, 10);
    Config.bAlignToSurface = true;
    Config.bUseInstancedStaticMesh = true;
    Config.bCastShadows = true;
    return Config;
}

FForge2PCGGraphDefinition FForge2PCGUtils::CreateFoliageGraphPreset(const TArray<FString>& MeshPaths, float Density, int32 Seed)
{
    FForge2PCGGraphDefinition Definition;
    Definition.GraphName = TEXT("FoliageGraph");
    Definition.Seed = Seed;

    // Add surface sampler
    FForge2PCGNodeConfig SamplerNode;
    SamplerNode.NodeName = TEXT("SurfaceSampler");
    SamplerNode.NodeType = EForge2PCGNodeType::SurfaceSampler;
    SamplerNode.GraphPosition = FVector2D(0, 0);
    Definition.Nodes.Add(SamplerNode);

    // Add density filter
    FForge2PCGNodeConfig DensityNode;
    DensityNode.NodeName = TEXT("DensityFilter");
    DensityNode.NodeType = EForge2PCGNodeType::DensityFilter;
    DensityNode.GraphPosition = FVector2D(300, 0);
    Definition.Nodes.Add(DensityNode);

    // Add mesh spawner
    FForge2PCGNodeConfig SpawnerNode;
    SpawnerNode.NodeName = TEXT("MeshSpawner");
    SpawnerNode.NodeType = EForge2PCGNodeType::StaticMeshSpawner;
    SpawnerNode.GraphPosition = FVector2D(600, 0);
    Definition.Nodes.Add(SpawnerNode);

    // Connect nodes
    FForge2PCGConnection Conn1;
    Conn1.SourceNodeName = TEXT("SurfaceSampler");
    Conn1.TargetNodeName = TEXT("DensityFilter");
    Definition.Connections.Add(Conn1);

    FForge2PCGConnection Conn2;
    Conn2.SourceNodeName = TEXT("DensityFilter");
    Conn2.TargetNodeName = TEXT("MeshSpawner");
    Definition.Connections.Add(Conn2);

    return Definition;
}

FForge2PCGGraphDefinition FForge2PCGUtils::CreateScatterGraphPreset(const TArray<FString>& MeshPaths, float Density, float MinDistance, int32 Seed)
{
    FForge2PCGGraphDefinition Definition;
    Definition.GraphName = TEXT("ScatterGraph");
    Definition.Seed = Seed;

    // Add surface sampler
    FForge2PCGNodeConfig SamplerNode;
    SamplerNode.NodeName = TEXT("SurfaceSampler");
    SamplerNode.NodeType = EForge2PCGNodeType::SurfaceSampler;
    SamplerNode.GraphPosition = FVector2D(0, 0);
    Definition.Nodes.Add(SamplerNode);

    // Add point filter
    FForge2PCGNodeConfig FilterNode;
    FilterNode.NodeName = TEXT("PointFilter");
    FilterNode.NodeType = EForge2PCGNodeType::PointFilter;
    FilterNode.GraphPosition = FVector2D(300, 0);
    Definition.Nodes.Add(FilterNode);

    // Add transform
    FForge2PCGNodeConfig TransformNode;
    TransformNode.NodeName = TEXT("PointTransform");
    TransformNode.NodeType = EForge2PCGNodeType::PointTransform;
    TransformNode.GraphPosition = FVector2D(600, 0);
    Definition.Nodes.Add(TransformNode);

    // Add mesh spawner
    FForge2PCGNodeConfig SpawnerNode;
    SpawnerNode.NodeName = TEXT("MeshSpawner");
    SpawnerNode.NodeType = EForge2PCGNodeType::StaticMeshSpawner;
    SpawnerNode.GraphPosition = FVector2D(900, 0);
    Definition.Nodes.Add(SpawnerNode);

    // Connect nodes
    FForge2PCGConnection Conn1;
    Conn1.SourceNodeName = TEXT("SurfaceSampler");
    Conn1.TargetNodeName = TEXT("PointFilter");
    Definition.Connections.Add(Conn1);

    FForge2PCGConnection Conn2;
    Conn2.SourceNodeName = TEXT("PointFilter");
    Conn2.TargetNodeName = TEXT("PointTransform");
    Definition.Connections.Add(Conn2);

    FForge2PCGConnection Conn3;
    Conn3.SourceNodeName = TEXT("PointTransform");
    Conn3.TargetNodeName = TEXT("MeshSpawner");
    Definition.Connections.Add(Conn3);

    return Definition;
}

FForge2PCGGraphDefinition FForge2PCGUtils::CreatePathScatterGraphPreset(const TArray<FString>& MeshPaths, float PathWidth, int32 Seed)
{
    FForge2PCGGraphDefinition Definition;
    Definition.GraphName = TEXT("PathScatterGraph");
    Definition.Seed = Seed;

    // Add spline sampler
    FForge2PCGNodeConfig SplineSamplerNode;
    SplineSamplerNode.NodeName = TEXT("SplineSampler");
    SplineSamplerNode.NodeType = EForge2PCGNodeType::SplineSampler;
    SplineSamplerNode.GraphPosition = FVector2D(0, 0);
    Definition.Nodes.Add(SplineSamplerNode);

    // Add noise for variation
    FForge2PCGNodeConfig NoiseNode;
    NoiseNode.NodeName = TEXT("PointNoise");
    NoiseNode.NodeType = EForge2PCGNodeType::PointNoise;
    NoiseNode.GraphPosition = FVector2D(300, 0);
    Definition.Nodes.Add(NoiseNode);

    // Add mesh spawner
    FForge2PCGNodeConfig SpawnerNode;
    SpawnerNode.NodeName = TEXT("MeshSpawner");
    SpawnerNode.NodeType = EForge2PCGNodeType::StaticMeshSpawner;
    SpawnerNode.GraphPosition = FVector2D(600, 0);
    Definition.Nodes.Add(SpawnerNode);

    // Connect nodes
    FForge2PCGConnection Conn1;
    Conn1.SourceNodeName = TEXT("SplineSampler");
    Conn1.TargetNodeName = TEXT("PointNoise");
    Definition.Connections.Add(Conn1);

    FForge2PCGConnection Conn2;
    Conn2.SourceNodeName = TEXT("PointNoise");
    Conn2.TargetNodeName = TEXT("MeshSpawner");
    Definition.Connections.Add(Conn2);

    return Definition;
}
