// Copyright 2025 FORGE2 Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraph/EdGraph.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Forge2BlueprintSystem.generated.h"

// ============================================================================
// BLUEPRINT ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2BlueprintType : uint8
{
	Actor UMETA(DisplayName = "Actor Blueprint"),
	Component UMETA(DisplayName = "Component Blueprint"),
	Widget UMETA(DisplayName = "Widget Blueprint"),
	GameMode UMETA(DisplayName = "Game Mode Blueprint"),
	Interface UMETA(DisplayName = "Interface Blueprint"),
	FunctionLibrary UMETA(DisplayName = "Function Library"),
	MacroLibrary UMETA(DisplayName = "Macro Library"),
	DataOnly UMETA(DisplayName = "Data Only Blueprint")
};

UENUM(BlueprintType)
enum class EForge2VariableType : uint8
{
	Boolean,
	Integer,
	Float,
	String,
	Name,
	Text,
	Vector,
	Rotator,
	Transform,
	Color,
	Object,
	Class,
	Struct,
	Enum,
	Array,
	Map,
	Set
};

UENUM(BlueprintType)
enum class EForge2VariableReplication : uint8
{
	None,
	Replicated,
	RepNotify
};

UENUM(BlueprintType)
enum class EForge2FunctionType : uint8
{
	Regular,
	Pure,
	Const,
	Static,
	Event,
	Override
};

UENUM(BlueprintType)
enum class EForge2NodeCategory : uint8
{
	FlowControl,
	Math,
	String,
	Variable,
	Function,
	Event,
	Macro,
	Comment,
	Reroute,
	Cast,
	Spawn,
	Custom
};

UENUM(BlueprintType)
enum class EForge2PinDirection : uint8
{
	Input,
	Output
};

UENUM(BlueprintType)
enum class EForge2CompileStatus : uint8
{
	Unknown,
	Dirty,
	Error,
	Warning,
	UpToDate
};

// ============================================================================
// WIDGET ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2WidgetElementType : uint8
{
	Button,
	Text,
	Image,
	ProgressBar,
	Slider,
	CheckBox,
	ComboBox,
	EditableText,
	TextBlock,
	RichTextBlock,
	Border,
	Canvas,
	HorizontalBox,
	VerticalBox,
	Grid,
	Overlay,
	SizeBox,
	Spacer,
	ScrollBox,
	ListView,
	TileView,
	Custom
};

UENUM(BlueprintType)
enum class EForge2WidgetAnchor : uint8
{
	TopLeft,
	TopCenter,
	TopRight,
	CenterLeft,
	Center,
	CenterRight,
	BottomLeft,
	BottomCenter,
	BottomRight,
	Stretch
};

UENUM(BlueprintType)
enum class EForge2WidgetVisibility : uint8
{
	Visible,
	Collapsed,
	Hidden,
	HitTestInvisible,
	SelfHitTestInvisible
};

// ============================================================================
// BLUEPRINT STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2BlueprintSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FName BlueprintName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	EForge2BlueprintType BlueprintType = EForge2BlueprintType::Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	TSubclassOf<UObject> ParentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	TArray<TSubclassOf<UInterface>> ImplementedInterfaces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FString PackagePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	bool bCompileOnCreate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FText BlueprintDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FText BlueprintCategory;

	FForge2BlueprintSettings()
		: BlueprintName(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VariableDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FName VariableName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	EForge2VariableType VariableType = EForge2VariableType::Float;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FString DefaultValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	bool bInstanceEditable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	bool bBlueprintReadOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	bool bExposeOnSpawn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	bool bPrivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	EForge2VariableReplication Replication = EForge2VariableReplication::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FName RepNotifyFunctionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FText ToolTip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FName Category;

	// For Object/Class types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	TSubclassOf<UObject> ObjectType;

	// For Struct types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	UScriptStruct* StructType = nullptr;

	// For Enum types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	UEnum* EnumType = nullptr;

	// For Array/Map/Set - inner type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	EForge2VariableType ContainerValueType = EForge2VariableType::Float;

	// For Map - key type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	EForge2VariableType MapKeyType = EForge2VariableType::String;

	FForge2VariableDefinition()
		: VariableName(NAME_None)
		, Category(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FunctionParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	FName ParameterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	EForge2VariableType ParameterType = EForge2VariableType::Float;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	bool bIsOutput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	bool bPassByReference = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	FString DefaultValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	TSubclassOf<UObject> ObjectType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	UScriptStruct* StructType = nullptr;

	FForge2FunctionParameter()
		: ParameterName(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FunctionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	FName FunctionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	EForge2FunctionType FunctionType = EForge2FunctionType::Regular;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	TArray<FForge2FunctionParameter> Parameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	FName Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	bool bCallInEditor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	TArray<FName> Keywords;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	FName AccessSpecifier; // "public", "protected", "private"

	// For Events
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	bool bIsReliable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	bool bIsServer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	bool bIsClient = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	bool bIsMulticast = false;

	FForge2FunctionDefinition()
		: FunctionName(NAME_None)
		, Category(NAME_None)
		, AccessSpecifier(TEXT("public"))
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2EventDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FForge2FunctionParameter> Parameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	bool bIsCustomEvent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	bool bIsDispatcher = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FText Description;

	FForge2EventDefinition()
		: EventName(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ComponentDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	FName ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TSubclassOf<UActorComponent> ComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	FName ParentComponentName; // Empty = attach to root

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	FTransform RelativeTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TMap<FName, FString> PropertyOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	bool bIsRootComponent = false;

	FForge2ComponentDefinition()
		: ComponentName(NAME_None)
		, ParentComponentName(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NodeConnection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FGuid SourceNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FName SourcePinName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FGuid TargetNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FName TargetPinName;

	FForge2NodeConnection()
		: SourcePinName(NAME_None)
		, TargetPinName(NAME_None)
	{
		SourceNodeId = FGuid::NewGuid();
		TargetNodeId = FGuid::NewGuid();
	}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NodeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FGuid NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	EForge2NodeCategory Category = EForge2NodeCategory::Function;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FName NodeType; // e.g., "K2Node_CallFunction", "K2Node_IfThenElse"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FName FunctionName; // For function calls

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TSubclassOf<UObject> TargetClass; // For member function calls

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FVector2D GraphPosition = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FString Comment; // For comment nodes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TMap<FName, FString> PinDefaultValues;

	FForge2NodeDefinition()
		: NodeType(NAME_None)
		, FunctionName(NAME_None)
	{
		NodeId = FGuid::NewGuid();
	}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2CompileResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compile")
	EForge2CompileStatus Status = EForge2CompileStatus::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compile")
	int32 NumErrors = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compile")
	int32 NumWarnings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compile")
	TArray<FString> ErrorMessages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compile")
	TArray<FString> WarningMessages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compile")
	double CompileTime = 0.0;
};

// ============================================================================
// WIDGET STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WidgetSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FVector2D Size = FVector2D(100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	EForge2WidgetAnchor Anchor = EForge2WidgetAnchor::TopLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FVector2D Alignment = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	bool bAutoSize = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FMargin Padding = FMargin(0.0f);
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WidgetElementDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FName ElementName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	EForge2WidgetElementType ElementType = EForge2WidgetElementType::TextBlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FForge2WidgetSlot Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	EForge2WidgetVisibility Visibility = EForge2WidgetVisibility::Visible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	bool bIsVariable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FName ParentElement; // Empty = root canvas

	// Content properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FText TextContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSoftObjectPtr<UTexture2D> ImageContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	float ProgressValue = 0.0f;

	// Style properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	float Opacity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FSlateColor TextColor = FSlateColor(FLinearColor::White);

	// Custom class for Custom type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UWidget> CustomWidgetClass;

	FForge2WidgetElementDefinition()
		: ElementName(NAME_None)
		, ParentElement(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WidgetBindingDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binding")
	FName ElementName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binding")
	FName PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binding")
	FName FunctionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binding")
	bool bIsDelegate = false;

	FForge2WidgetBindingDefinition()
		: ElementName(NAME_None)
		, PropertyName(NAME_None)
		, FunctionName(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WidgetAnimationKeyframe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EForge2InterpolationType Interpolation = EForge2InterpolationType::Cubic;

	FForge2WidgetAnimationKeyframe()
		: PropertyName(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WidgetAnimationDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName AnimationName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName TargetElement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<FForge2WidgetAnimationKeyframe> Keyframes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Duration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	int32 NumLoops = 1;

	FForge2WidgetAnimationDefinition()
		: AnimationName(NAME_None)
		, TargetElement(NAME_None)
	{}
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WidgetStyleDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FName StyleName;

	// Colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor PrimaryColor = FLinearColor(0.1f, 0.3f, 0.6f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor SecondaryColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor AccentColor = FLinearColor(0.8f, 0.6f, 0.1f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor BackgroundColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.9f);

	// Fonts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FSlateFontInfo TitleFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FSlateFontInfo BodyFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FSlateFontInfo ButtonFont;

	// Margins
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FMargin DefaultMargin = FMargin(8.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FMargin DefaultPadding = FMargin(4.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	float BorderRadius = 4.0f;

	FForge2WidgetStyleDefinition()
		: StyleName(NAME_None)
	{}
};

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FForge2OnBlueprintCompiled, FName, BlueprintName, const FForge2CompileResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FForge2OnBlueprintCreated, FName, BlueprintName, UBlueprint*, Blueprint);

// ============================================================================
// BLUEPRINT MANAGER
// ============================================================================

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2BlueprintManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2BlueprintManager();

	// Blueprint Creation
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	UBlueprint* CreateBlueprint(const FForge2BlueprintSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool DeleteBlueprint(FName BlueprintName);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint")
	UBlueprint* GetBlueprint(FName BlueprintName) const;

	// Variables
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool AddVariable(UBlueprint* Blueprint, const FForge2VariableDefinition& Variable);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool RemoveVariable(UBlueprint* Blueprint, FName VariableName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool SetVariableDefault(UBlueprint* Blueprint, FName VariableName, const FString& DefaultValue);

	// Functions
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool AddFunction(UBlueprint* Blueprint, const FForge2FunctionDefinition& Function);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool RemoveFunction(UBlueprint* Blueprint, FName FunctionName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	UEdGraph* GetFunctionGraph(UBlueprint* Blueprint, FName FunctionName) const;

	// Events
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool AddEvent(UBlueprint* Blueprint, const FForge2EventDefinition& Event);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool AddEventDispatcher(UBlueprint* Blueprint, const FForge2EventDefinition& Dispatcher);

	// Components
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool AddComponent(UBlueprint* Blueprint, const FForge2ComponentDefinition& Component);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool RemoveComponent(UBlueprint* Blueprint, FName ComponentName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool SetComponentDefault(UBlueprint* Blueprint, FName ComponentName, FName PropertyName, const FString& Value);

	// Interfaces
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool ImplementInterface(UBlueprint* Blueprint, TSubclassOf<UInterface> Interface);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool RemoveInterface(UBlueprint* Blueprint, TSubclassOf<UInterface> Interface);

	// Graph Editing
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	FGuid AddNode(UBlueprint* Blueprint, FName GraphName, const FForge2NodeDefinition& Node);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool RemoveNode(UBlueprint* Blueprint, FName GraphName, FGuid NodeId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool ConnectNodes(UBlueprint* Blueprint, FName GraphName, const FForge2NodeConnection& Connection);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool DisconnectNodes(UBlueprint* Blueprint, FName GraphName, const FForge2NodeConnection& Connection);

	// Compilation
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	FForge2CompileResult CompileBlueprint(UBlueprint* Blueprint);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Blueprint")
	bool SaveBlueprint(UBlueprint* Blueprint);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint")
	EForge2CompileStatus GetCompileStatus(UBlueprint* Blueprint) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Blueprint|Events")
	FForge2OnBlueprintCompiled OnBlueprintCompiled;

	UPROPERTY(BlueprintAssignable, Category = "FORGE2|Blueprint|Events")
	FForge2OnBlueprintCreated OnBlueprintCreated;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TMap<FName, TWeakObjectPtr<UBlueprint>> ManagedBlueprints;

	UEdGraph* FindOrCreateGraph(UBlueprint* Blueprint, FName GraphName);
	FEdGraphPinType ConvertVariableType(EForge2VariableType Type, const FForge2VariableDefinition& Definition);
};

// ============================================================================
// WIDGET BLUEPRINT MANAGER
// ============================================================================

UCLASS(ClassGroup=(FORGE2), meta=(BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2WidgetBlueprintManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2WidgetBlueprintManager();

	// Widget Blueprint Creation
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	UBlueprint* CreateWidgetBlueprint(FName WidgetName, FString PackagePath);

	// Elements
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool AddWidgetElement(UBlueprint* WidgetBlueprint, const FForge2WidgetElementDefinition& Element);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool RemoveWidgetElement(UBlueprint* WidgetBlueprint, FName ElementName);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool SetWidgetElementProperty(UBlueprint* WidgetBlueprint, FName ElementName, FName PropertyName, const FString& Value);

	// Bindings
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool AddPropertyBinding(UBlueprint* WidgetBlueprint, const FForge2WidgetBindingDefinition& Binding);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool RemovePropertyBinding(UBlueprint* WidgetBlueprint, FName ElementName, FName PropertyName);

	// Animations
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool AddWidgetAnimation(UBlueprint* WidgetBlueprint, const FForge2WidgetAnimationDefinition& Animation);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool RemoveWidgetAnimation(UBlueprint* WidgetBlueprint, FName AnimationName);

	// Styles
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	void RegisterStyle(const FForge2WidgetStyleDefinition& Style);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool ApplyStyle(UBlueprint* WidgetBlueprint, FName StyleName);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget")
	FForge2WidgetStyleDefinition GetStyle(FName StyleName) const;

	// Navigation
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool SetWidgetNavigation(UBlueprint* WidgetBlueprint, FName ElementName,
							  FName UpElement, FName DownElement, FName LeftElement, FName RightElement);

	// Localization
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Widget")
	bool SetWidgetLocalization(UBlueprint* WidgetBlueprint, FName ElementName, FName StringTableKey);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TMap<FName, TWeakObjectPtr<UBlueprint>> ManagedWidgetBlueprints;

	UPROPERTY()
	TMap<FName, FForge2WidgetStyleDefinition> RegisteredStyles;
};

// ============================================================================
// BLUEPRINT UTILITIES
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2BlueprintUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Blueprint Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2BlueprintSettings GetActorBlueprintPreset(FName Name, TSubclassOf<AActor> ParentClass = nullptr);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2BlueprintSettings GetComponentBlueprintPreset(FName Name, TSubclassOf<UActorComponent> ParentClass = nullptr);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2BlueprintSettings GetInterfaceBlueprintPreset(FName Name);

	// Variable Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2VariableDefinition GetFloatVariablePreset(FName Name, float DefaultValue = 0.0f);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2VariableDefinition GetIntVariablePreset(FName Name, int32 DefaultValue = 0);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2VariableDefinition GetBoolVariablePreset(FName Name, bool DefaultValue = false);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2VariableDefinition GetObjectVariablePreset(FName Name, TSubclassOf<UObject> ObjectType);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2VariableDefinition GetReplicatedVariablePreset(FName Name, EForge2VariableType Type, FName RepNotifyFunc = NAME_None);

	// Function Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2FunctionDefinition GetPureFunctionPreset(FName Name);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2FunctionDefinition GetServerRPCPreset(FName Name, bool bReliable = true);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2FunctionDefinition GetClientRPCPreset(FName Name, bool bReliable = true);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2FunctionDefinition GetMulticastRPCPreset(FName Name, bool bReliable = false);

	// Component Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2ComponentDefinition GetSceneComponentPreset(FName Name, FTransform Transform = FTransform::Identity);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2ComponentDefinition GetStaticMeshComponentPreset(FName Name, TSoftObjectPtr<UStaticMesh> Mesh);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2ComponentDefinition GetSkeletalMeshComponentPreset(FName Name, TSoftObjectPtr<USkeletalMesh> Mesh);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Blueprint|Presets")
	static FForge2ComponentDefinition GetCapsuleComponentPreset(FName Name, float Radius = 34.0f, float HalfHeight = 88.0f);

	// Widget Element Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetElementDefinition GetButtonPreset(FName Name, FText Label, FForge2WidgetSlot Slot);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetElementDefinition GetTextBlockPreset(FName Name, FText Text, FForge2WidgetSlot Slot);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetElementDefinition GetImagePreset(FName Name, TSoftObjectPtr<UTexture2D> Texture, FForge2WidgetSlot Slot);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetElementDefinition GetProgressBarPreset(FName Name, float InitialValue, FForge2WidgetSlot Slot);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetElementDefinition GetHorizontalBoxPreset(FName Name, FForge2WidgetSlot Slot);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetElementDefinition GetVerticalBoxPreset(FName Name, FForge2WidgetSlot Slot);

	// Widget Style Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetStyleDefinition GetDarkThemePreset();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetStyleDefinition GetLightThemePreset();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetStyleDefinition GetGameUIThemePreset();

	// Slot Presets
	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetSlot GetCenteredSlot(FVector2D Size);

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetSlot GetFullScreenSlot();

	UFUNCTION(BlueprintPure, Category = "FORGE2|Widget|Presets")
	static FForge2WidgetSlot GetCornerSlot(EForge2WidgetAnchor Corner, FVector2D Size, FVector2D Offset = FVector2D::ZeroVector);
};
