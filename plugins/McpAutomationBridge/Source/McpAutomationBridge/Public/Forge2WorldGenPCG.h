// Forge2WorldGenPCG.h
// FORGE2 Ultimate AAA Game Development MCP Extension
// PCG (Procedural Content Generation) Framework Integration

#pragma once

#include "CoreMinimal.h"
#include "PCGGraph.h"
#include "PCGComponent.h"
#include "PCGSettings.h"
#include "PCGSubsystem.h"
#include "Forge2WorldGenPCG.generated.h"

// Forward declarations
class UPCGGraph;
class UPCGNode;
class UPCGSettings;
class UPCGComponent;
class APCGVolume;

/**
 * PCG Node types supported by FORGE2
 */
UENUM(BlueprintType)
enum class EForge2PCGNodeType : uint8
{
    // Input/Output
    InputPin            UMETA(DisplayName = "Input Pin"),
    OutputPin           UMETA(DisplayName = "Output Pin"),

    // Samplers
    SurfaceSampler      UMETA(DisplayName = "Surface Sampler"),
    VolumeSampler       UMETA(DisplayName = "Volume Sampler"),
    SplineSampler       UMETA(DisplayName = "Spline Sampler"),
    LandscapeSampler    UMETA(DisplayName = "Landscape Sampler"),
    TextureSampler      UMETA(DisplayName = "Texture Sampler"),
    MeshSampler         UMETA(DisplayName = "Mesh Sampler"),

    // Spawners
    StaticMeshSpawner   UMETA(DisplayName = "Static Mesh Spawner"),
    ActorSpawner        UMETA(DisplayName = "Actor Spawner"),
    ISMSpawner          UMETA(DisplayName = "Instanced Static Mesh Spawner"),

    // Filters
    PointFilter         UMETA(DisplayName = "Point Filter"),
    DensityFilter       UMETA(DisplayName = "Density Filter"),
    BoundsFilter        UMETA(DisplayName = "Bounds Filter"),
    SteepnessFilter     UMETA(DisplayName = "Steepness Filter"),

    // Modifiers
    PointTransform      UMETA(DisplayName = "Point Transform"),
    AttributeOperation  UMETA(DisplayName = "Attribute Operation"),
    PointNoise          UMETA(DisplayName = "Point Noise"),
    ProjectOnSurface    UMETA(DisplayName = "Project On Surface"),

    // Data
    GetActorData        UMETA(DisplayName = "Get Actor Data"),
    GetSplineData       UMETA(DisplayName = "Get Spline Data"),
    GetLandscapeData    UMETA(DisplayName = "Get Landscape Data"),

    // Logic
    Branch              UMETA(DisplayName = "Branch"),
    Merge               UMETA(DisplayName = "Merge"),
    Loop                UMETA(DisplayName = "Loop"),

    // Math
    MathOperation       UMETA(DisplayName = "Math Operation"),
    VectorOperation     UMETA(DisplayName = "Vector Operation"),

    // Generation
    CopyPoints          UMETA(DisplayName = "Copy Points"),
    CreatePoints        UMETA(DisplayName = "Create Points"),
    UnionData           UMETA(DisplayName = "Union Data"),
    DifferenceData      UMETA(DisplayName = "Difference Data"),
    IntersectionData    UMETA(DisplayName = "Intersection Data"),

    // Debug
    Debug               UMETA(DisplayName = "Debug"),

    // Custom
    Subgraph            UMETA(DisplayName = "Subgraph"),
    Custom              UMETA(DisplayName = "Custom")
};

/**
 * PCG attribute types
 */
UENUM(BlueprintType)
enum class EForge2PCGAttributeType : uint8
{
    Float       UMETA(DisplayName = "Float"),
    Double      UMETA(DisplayName = "Double"),
    Int32       UMETA(DisplayName = "Int32"),
    Int64       UMETA(DisplayName = "Int64"),
    Vector      UMETA(DisplayName = "Vector"),
    Vector4     UMETA(DisplayName = "Vector4"),
    Quaternion  UMETA(DisplayName = "Quaternion"),
    Transform   UMETA(DisplayName = "Transform"),
    String      UMETA(DisplayName = "String"),
    Bool        UMETA(DisplayName = "Bool"),
    Name        UMETA(DisplayName = "Name"),
    SoftObject  UMETA(DisplayName = "Soft Object Path")
};

/**
 * PCG math operation types
 */
UENUM(BlueprintType)
enum class EForge2PCGMathOp : uint8
{
    Add         UMETA(DisplayName = "Add"),
    Subtract    UMETA(DisplayName = "Subtract"),
    Multiply    UMETA(DisplayName = "Multiply"),
    Divide      UMETA(DisplayName = "Divide"),
    Power       UMETA(DisplayName = "Power"),
    Min         UMETA(DisplayName = "Min"),
    Max         UMETA(DisplayName = "Max"),
    Clamp       UMETA(DisplayName = "Clamp"),
    Lerp        UMETA(DisplayName = "Lerp"),
    Abs         UMETA(DisplayName = "Absolute Value"),
    Sign        UMETA(DisplayName = "Sign"),
    Floor       UMETA(DisplayName = "Floor"),
    Ceil        UMETA(DisplayName = "Ceiling"),
    Round       UMETA(DisplayName = "Round"),
    Modulo      UMETA(DisplayName = "Modulo")
};

// ============================================================================
// PCG Node Configuration Structures
// ============================================================================

/**
 * Base node configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGNodeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NodeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PCGNodeType NodeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D GraphPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Comment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled;

    FForge2PCGNodeConfig()
        : NodeType(EForge2PCGNodeType::Custom)
        , GraphPosition(FVector2D::ZeroVector)
        , bEnabled(true)
    {}
};

/**
 * Surface sampler configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGSurfaceSamplerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PointsPerSquareMeter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bApplyDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUnbounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LooseBoundsFactor;

    FForge2PCGSurfaceSamplerConfig()
        : PointsPerSquareMeter(0.1f)
        , MinDistance(100.0f)
        , bApplyDensity(true)
        , Seed(0)
        , bUnbounded(false)
        , LooseBoundsFactor(1.0f)
    {}
};

/**
 * Static mesh spawner configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGMeshSpawnerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> MeshPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> MeshWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MinScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MaxScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator MinRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator MaxRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAlignToSurface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseInstancedStaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCastShadows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CollisionProfile;

    FForge2PCGMeshSpawnerConfig()
        : MinScale(FVector(0.8f))
        , MaxScale(FVector(1.2f))
        , MinRotation(FRotator::ZeroRotator)
        , MaxRotation(FRotator(0, 360, 0))
        , bAlignToSurface(true)
        , bUseInstancedStaticMesh(true)
        , bCastShadows(true)
    {}
};

/**
 * Point filter configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGPointFilterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AttributeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInvert;

    FForge2PCGPointFilterConfig()
        : MinValue(0.0f)
        , MaxValue(1.0f)
        , bInvert(false)
    {}
};

/**
 * Density filter configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGDensityFilterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LowerBound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UpperBound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInvert;

    FForge2PCGDensityFilterConfig()
        : LowerBound(0.0f)
        , UpperBound(1.0f)
        , bInvert(false)
    {}
};

/**
 * Point transform configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGPointTransformConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector OffsetMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector OffsetMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RotationMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RotationMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ScaleMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ScaleMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAbsoluteOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAbsoluteRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAbsoluteScale;

    FForge2PCGPointTransformConfig()
        : OffsetMin(FVector::ZeroVector)
        , OffsetMax(FVector::ZeroVector)
        , RotationMin(FRotator::ZeroRotator)
        , RotationMax(FRotator::ZeroRotator)
        , ScaleMin(FVector::OneVector)
        , ScaleMax(FVector::OneVector)
        , bAbsoluteOffset(false)
        , bAbsoluteRotation(false)
        , bAbsoluteScale(false)
    {}
};

/**
 * Noise configuration for point modification
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGNoiseConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetAttribute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUsePerlinNoise;

    FForge2PCGNoiseConfig()
        : TargetAttribute(TEXT("Density"))
        , NoiseScale(0.01f)
        , NoiseStrength(1.0f)
        , Seed(0)
        , bUsePerlinNoise(true)
    {}
};

/**
 * Attribute operation configuration
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGAttributeOpConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InputAttribute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OutputAttribute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EForge2PCGMathOp Operation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OperandValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OperandAttribute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseAttributeAsOperand;

    FForge2PCGAttributeOpConfig()
        : InputAttribute(TEXT("Density"))
        , OutputAttribute(TEXT("Density"))
        , Operation(EForge2PCGMathOp::Multiply)
        , OperandValue(1.0f)
        , bUseAttributeAsOperand(false)
    {}
};

/**
 * PCG connection definition
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGConnection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SourceNodeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SourcePinName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetNodeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetPinName;

    FForge2PCGConnection()
    {}
};

/**
 * Complete PCG graph definition
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGGraphDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GraphName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GraphPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PCGNodeConfig> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FForge2PCGConnection> Connections;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseHierarchicalGeneration;

    FForge2PCGGraphDefinition()
        : Seed(0)
        , bUseHierarchicalGeneration(false)
    {}
};

/**
 * PCG execution settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PCGExecutionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateAtRuntime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bForceRefresh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDirtyGenerated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPartitioned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PartitionGridSize;

    FForge2PCGExecutionSettings()
        : bGenerateAtRuntime(false)
        , bForceRefresh(false)
        , bDirtyGenerated(true)
        , bIsPartitioned(false)
        , PartitionGridSize(25600)
    {}
};

// ============================================================================
// PCG Graph Manager Class
// ============================================================================

/**
 * PCG Graph manager for creating and modifying PCG graphs
 */
class MCPAUTOMATIONBRIDGE_API FForge2PCGGraphManager
{
public:
    FForge2PCGGraphManager();

    /** Create a new PCG graph asset */
    UPCGGraph* CreateGraph(const FString& AssetPath, const FString& GraphName);

    /** Load existing PCG graph */
    UPCGGraph* LoadGraph(const FString& AssetPath);

    /** Save PCG graph */
    bool SaveGraph(UPCGGraph* Graph);

    /** Add node to graph */
    UPCGNode* AddNode(UPCGGraph* Graph, const FForge2PCGNodeConfig& NodeConfig);

    /** Add surface sampler node */
    UPCGNode* AddSurfaceSamplerNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGSurfaceSamplerConfig& Config);

    /** Add static mesh spawner node */
    UPCGNode* AddMeshSpawnerNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGMeshSpawnerConfig& Config);

    /** Add point filter node */
    UPCGNode* AddPointFilterNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGPointFilterConfig& Config);

    /** Add density filter node */
    UPCGNode* AddDensityFilterNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGDensityFilterConfig& Config);

    /** Add point transform node */
    UPCGNode* AddPointTransformNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGPointTransformConfig& Config);

    /** Add noise node */
    UPCGNode* AddNoiseNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGNoiseConfig& Config);

    /** Add attribute operation node */
    UPCGNode* AddAttributeOpNode(UPCGGraph* Graph, const FString& NodeName, const FForge2PCGAttributeOpConfig& Config);

    /** Connect two nodes */
    bool ConnectNodes(UPCGGraph* Graph, const FForge2PCGConnection& Connection);

    /** Connect nodes by name */
    bool ConnectNodes(UPCGGraph* Graph, const FString& SourceNodeName, const FString& SourcePinName, const FString& TargetNodeName, const FString& TargetPinName);

    /** Find node by name */
    UPCGNode* FindNodeByName(UPCGGraph* Graph, const FString& NodeName);

    /** Remove node from graph */
    bool RemoveNode(UPCGGraph* Graph, const FString& NodeName);

    /** Clear all nodes from graph */
    void ClearGraph(UPCGGraph* Graph);

private:
    /** Configure node settings based on type */
    void ConfigureNodeSettings(UPCGSettings* Settings, EForge2PCGNodeType NodeType);

    /** Get settings class for node type */
    TSubclassOf<UPCGSettings> GetSettingsClassForNodeType(EForge2PCGNodeType NodeType);
};

// ============================================================================
// PCG Executor Class
// ============================================================================

/**
 * PCG execution manager
 */
class MCPAUTOMATIONBRIDGE_API FForge2PCGExecutor
{
public:
    FForge2PCGExecutor();

    /** Execute PCG graph on component */
    bool ExecuteGraph(UPCGComponent* Component, const FForge2PCGExecutionSettings& Settings);

    /** Execute PCG graph on volume */
    bool ExecuteGraphOnVolume(APCGVolume* Volume, const FForge2PCGExecutionSettings& Settings);

    /** Execute PCG graph with bounds */
    bool ExecuteGraphInBounds(UWorld* World, UPCGGraph* Graph, const FBox& Bounds, const FForge2PCGExecutionSettings& Settings);

    /** Get PCG subsystem */
    UPCGSubsystem* GetPCGSubsystem(UWorld* World);

    /** Cancel running generation */
    void CancelGeneration(UPCGComponent* Component);

    /** Check if generation is in progress */
    bool IsGenerating(UPCGComponent* Component);

    /** Clean up generated resources */
    void CleanupGeneratedResources(UPCGComponent* Component);
};

// ============================================================================
// PCG Utilities
// ============================================================================

/**
 * PCG utility functions
 */
class MCPAUTOMATIONBRIDGE_API FForge2PCGUtils
{
public:
    /** Create PCG component on actor */
    static UPCGComponent* CreatePCGComponent(AActor* Actor, UPCGGraph* Graph);

    /** Create PCG volume in world */
    static APCGVolume* CreatePCGVolume(UWorld* World, UPCGGraph* Graph, const FBox& Bounds);

    /** Get PCG settings class name */
    static FString GetSettingsClassName(EForge2PCGNodeType NodeType);

    /** Convert node type to settings type */
    static FString NodeTypeToSettingsType(EForge2PCGNodeType NodeType);

    /** Create complete graph from definition */
    static UPCGGraph* CreateGraphFromDefinition(const FForge2PCGGraphDefinition& Definition);

    /** Export graph to definition */
    static FForge2PCGGraphDefinition ExportGraphToDefinition(UPCGGraph* Graph);

    /** Get default surface sampler configuration */
    static FForge2PCGSurfaceSamplerConfig GetDefaultSurfaceSamplerConfig();

    /** Get default mesh spawner configuration for vegetation */
    static FForge2PCGMeshSpawnerConfig GetDefaultVegetationSpawnerConfig();

    /** Get default mesh spawner configuration for props */
    static FForge2PCGMeshSpawnerConfig GetDefaultPropSpawnerConfig();

    /** Create foliage PCG graph preset */
    static FForge2PCGGraphDefinition CreateFoliageGraphPreset(const TArray<FString>& MeshPaths, float Density, int32 Seed);

    /** Create scatter PCG graph preset */
    static FForge2PCGGraphDefinition CreateScatterGraphPreset(const TArray<FString>& MeshPaths, float Density, float MinDistance, int32 Seed);

    /** Create path-based scatter PCG graph preset */
    static FForge2PCGGraphDefinition CreatePathScatterGraphPreset(const TArray<FString>& MeshPaths, float PathWidth, int32 Seed);
};

#endif // Include guard handled by #pragma once
