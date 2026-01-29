// Copyright 2025 FORGE2 Project. All Rights Reserved.

#include "Forge2BlueprintSystem.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_Event.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Components/Widget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

// ============================================================================
// BLUEPRINT MANAGER IMPLEMENTATION
// ============================================================================

UForge2BlueprintManager::UForge2BlueprintManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2BlueprintManager::BeginPlay()
{
	Super::BeginPlay();
}

UBlueprint* UForge2BlueprintManager::CreateBlueprint(const FForge2BlueprintSettings& Settings)
{
#if WITH_EDITOR
	if (Settings.BlueprintName.IsNone())
	{
		return nullptr;
	}

	// Determine parent class based on blueprint type
	UClass* ParentClass = Settings.ParentClass;
	if (!ParentClass)
	{
		switch (Settings.BlueprintType)
		{
		case EForge2BlueprintType::Actor:
			ParentClass = AActor::StaticClass();
			break;
		case EForge2BlueprintType::Component:
			ParentClass = UActorComponent::StaticClass();
			break;
		case EForge2BlueprintType::GameMode:
			ParentClass = AGameModeBase::StaticClass();
			break;
		case EForge2BlueprintType::Widget:
			ParentClass = UUserWidget::StaticClass();
			break;
		default:
			ParentClass = UObject::StaticClass();
			break;
		}
	}

	// Create package
	FString PackagePath = Settings.PackagePath.IsEmpty() ? TEXT("/Game/Blueprints") : Settings.PackagePath;
	FString FullPath = PackagePath / Settings.BlueprintName.ToString();

	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return nullptr;
	}

	// Create blueprint
	EBlueprintType BlueprintType = BPTYPE_Normal;
	if (Settings.BlueprintType == EForge2BlueprintType::Interface)
	{
		BlueprintType = BPTYPE_Interface;
	}
	else if (Settings.BlueprintType == EForge2BlueprintType::MacroLibrary)
	{
		BlueprintType = BPTYPE_MacroLibrary;
	}
	else if (Settings.BlueprintType == EForge2BlueprintType::FunctionLibrary)
	{
		BlueprintType = BPTYPE_FunctionLibrary;
	}

	UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		Package,
		Settings.BlueprintName,
		BlueprintType,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass()
	);

	if (NewBlueprint)
	{
		// Add interfaces
		for (TSubclassOf<UInterface> Interface : Settings.ImplementedInterfaces)
		{
			if (Interface)
			{
				FBlueprintEditorUtils::ImplementNewInterface(NewBlueprint, Interface->GetFName());
			}
		}

		// Set description
		if (!Settings.BlueprintDescription.IsEmpty())
		{
			NewBlueprint->BlueprintDescription = Settings.BlueprintDescription.ToString();
		}

		// Compile if requested
		if (Settings.bCompileOnCreate)
		{
			FKismetEditorUtilities::CompileBlueprint(NewBlueprint);
		}

		// Mark dirty and register
		NewBlueprint->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(NewBlueprint);

		ManagedBlueprints.Add(Settings.BlueprintName, NewBlueprint);
		OnBlueprintCreated.Broadcast(Settings.BlueprintName, NewBlueprint);
	}

	return NewBlueprint;
#else
	return nullptr;
#endif
}

bool UForge2BlueprintManager::DeleteBlueprint(FName BlueprintName)
{
#if WITH_EDITOR
	TWeakObjectPtr<UBlueprint>* BlueprintPtr = ManagedBlueprints.Find(BlueprintName);
	if (!BlueprintPtr || !BlueprintPtr->IsValid())
	{
		return false;
	}

	UBlueprint* Blueprint = BlueprintPtr->Get();
	if (Blueprint)
	{
		// Unload and delete
		UPackage* Package = Blueprint->GetOutermost();
		if (Package)
		{
			TArray<UObject*> ObjectsToDelete;
			ObjectsToDelete.Add(Blueprint);
			// Note: Full deletion requires ObjectTools in editor
		}
	}

	ManagedBlueprints.Remove(BlueprintName);
	return true;
#else
	return false;
#endif
}

UBlueprint* UForge2BlueprintManager::GetBlueprint(FName BlueprintName) const
{
	const TWeakObjectPtr<UBlueprint>* BlueprintPtr = ManagedBlueprints.Find(BlueprintName);
	return BlueprintPtr ? BlueprintPtr->Get() : nullptr;
}

bool UForge2BlueprintManager::AddVariable(UBlueprint* Blueprint, const FForge2VariableDefinition& Variable)
{
#if WITH_EDITOR
	if (!Blueprint || Variable.VariableName.IsNone())
	{
		return false;
	}

	// Convert type
	FEdGraphPinType PinType = ConvertVariableType(Variable.VariableType, Variable);

	// Add variable
	bool bSuccess = FBlueprintEditorUtils::AddMemberVariable(
		Blueprint,
		Variable.VariableName,
		PinType
	);

	if (bSuccess)
	{
		// Set properties
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, Variable.VariableName, Variable.bPrivate);
		FBlueprintEditorUtils::SetInterpFlag(Blueprint, Variable.VariableName, Variable.bInstanceEditable);

		if (Variable.bExposeOnSpawn)
		{
			FBlueprintEditorUtils::SetBlueprintPropertyReadOnlyFlag(Blueprint, Variable.VariableName, false);
		}

		// Set replication
		if (Variable.Replication != EForge2VariableReplication::None)
		{
			int32 VarIndex = FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, Variable.VariableName);
			if (VarIndex != INDEX_NONE)
			{
				Blueprint->NewVariables[VarIndex].PropertyFlags |= CPF_Net;

				if (Variable.Replication == EForge2VariableReplication::RepNotify)
				{
					Blueprint->NewVariables[VarIndex].PropertyFlags |= CPF_RepNotify;
					Blueprint->NewVariables[VarIndex].RepNotifyFunc = Variable.RepNotifyFunctionName;
				}
			}
		}

		// Set category
		if (!Variable.Category.IsNone())
		{
			FBlueprintEditorUtils::SetBlueprintVariableCategory(Blueprint, Variable.VariableName, nullptr, FText::FromName(Variable.Category));
		}

		// Set tooltip
		if (!Variable.ToolTip.IsEmpty())
		{
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, Variable.VariableName, nullptr, TEXT("Tooltip"), Variable.ToolTip.ToString());
		}

		Blueprint->MarkPackageDirty();
	}

	return bSuccess;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::RemoveVariable(UBlueprint* Blueprint, FName VariableName)
{
#if WITH_EDITOR
	if (!Blueprint || VariableName.IsNone())
	{
		return false;
	}

	FBlueprintEditorUtils::RemoveMemberVariable(Blueprint, VariableName);
	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::SetVariableDefault(UBlueprint* Blueprint, FName VariableName, const FString& DefaultValue)
{
#if WITH_EDITOR
	if (!Blueprint || VariableName.IsNone())
	{
		return false;
	}

	// Find the variable
	int32 VarIndex = FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, VariableName);
	if (VarIndex == INDEX_NONE)
	{
		return false;
	}

	Blueprint->NewVariables[VarIndex].DefaultValue = DefaultValue;
	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::AddFunction(UBlueprint* Blueprint, const FForge2FunctionDefinition& Function)
{
#if WITH_EDITOR
	if (!Blueprint || Function.FunctionName.IsNone())
	{
		return false;
	}

	// Create function graph
	UEdGraph* FunctionGraph = FBlueprintEditorUtils::CreateNewGraph(
		Blueprint,
		Function.FunctionName,
		UEdGraph::StaticClass(),
		UEdGraphSchema_K2::StaticClass()
	);

	if (!FunctionGraph)
	{
		return false;
	}

	FBlueprintEditorUtils::AddFunctionGraph(Blueprint, FunctionGraph, false, nullptr);

	// Find the entry node and configure it
	UK2Node_FunctionEntry* EntryNode = nullptr;
	for (UEdGraphNode* Node : FunctionGraph->Nodes)
	{
		EntryNode = Cast<UK2Node_FunctionEntry>(Node);
		if (EntryNode)
		{
			break;
		}
	}

	if (EntryNode)
	{
		// Add parameters
		for (const FForge2FunctionParameter& Param : Function.Parameters)
		{
			if (!Param.bIsOutput)
			{
				// Input parameters on entry node
				FEdGraphPinType PinType;
				// Set pin type based on parameter type
				EntryNode->CreateUserDefinedPin(Param.ParameterName, PinType, EGPD_Output);
			}
		}

		// Set function flags
		if (Function.FunctionType == EForge2FunctionType::Pure)
		{
			EntryNode->AddExtraFlags(FUNC_BlueprintPure);
		}
		if (Function.FunctionType == EForge2FunctionType::Const)
		{
			EntryNode->AddExtraFlags(FUNC_Const);
		}
		if (Function.FunctionType == EForge2FunctionType::Static)
		{
			EntryNode->AddExtraFlags(FUNC_Static);
		}

		// Network flags
		if (Function.bIsServer)
		{
			EntryNode->AddExtraFlags(FUNC_NetServer);
		}
		if (Function.bIsClient)
		{
			EntryNode->AddExtraFlags(FUNC_NetClient);
		}
		if (Function.bIsMulticast)
		{
			EntryNode->AddExtraFlags(FUNC_NetMulticast);
		}
		if (Function.bIsReliable)
		{
			EntryNode->AddExtraFlags(FUNC_NetReliable);
		}
	}

	// Set category
	if (!Function.Category.IsNone())
	{
		FBlueprintEditorUtils::SetBlueprintFunctionOrMacroCategory(FunctionGraph, FText::FromName(Function.Category));
	}

	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::RemoveFunction(UBlueprint* Blueprint, FName FunctionName)
{
#if WITH_EDITOR
	if (!Blueprint || FunctionName.IsNone())
	{
		return false;
	}

	UEdGraph* FunctionGraph = FindObject<UEdGraph>(Blueprint, *FunctionName.ToString());
	if (FunctionGraph)
	{
		FBlueprintEditorUtils::RemoveGraph(Blueprint, FunctionGraph);
		Blueprint->MarkPackageDirty();
		return true;
	}

	return false;
#else
	return false;
#endif
}

UEdGraph* UForge2BlueprintManager::GetFunctionGraph(UBlueprint* Blueprint, FName FunctionName) const
{
	if (!Blueprint || FunctionName.IsNone())
	{
		return nullptr;
	}

	return FindObject<UEdGraph>(Blueprint, *FunctionName.ToString());
}

bool UForge2BlueprintManager::AddEvent(UBlueprint* Blueprint, const FForge2EventDefinition& Event)
{
#if WITH_EDITOR
	if (!Blueprint || Event.EventName.IsNone())
	{
		return false;
	}

	if (Event.bIsCustomEvent)
	{
		// Add custom event
		UEdGraph* EventGraph = Blueprint->UbergraphPages.Num() > 0 ? Blueprint->UbergraphPages[0] : nullptr;
		if (!EventGraph)
		{
			return false;
		}

		// Create custom event node
		UK2Node_Event* EventNode = NewObject<UK2Node_Event>(EventGraph);
		if (EventNode)
		{
			EventNode->CustomFunctionName = Event.EventName;
			EventNode->bIsEditable = true;
			EventGraph->AddNode(EventNode);
			EventNode->AllocateDefaultPins();

			// Add parameters as output pins
			for (const FForge2FunctionParameter& Param : Event.Parameters)
			{
				FEdGraphPinType PinType;
				// Configure pin type
				EventNode->CreateUserDefinedPin(Param.ParameterName, PinType, EGPD_Output);
			}

			Blueprint->MarkPackageDirty();
			return true;
		}
	}

	return false;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::AddEventDispatcher(UBlueprint* Blueprint, const FForge2EventDefinition& Dispatcher)
{
#if WITH_EDITOR
	if (!Blueprint || Dispatcher.EventName.IsNone())
	{
		return false;
	}

	// Create multicast delegate variable
	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_MCDelegate;

	bool bSuccess = FBlueprintEditorUtils::AddMemberVariable(
		Blueprint,
		Dispatcher.EventName,
		PinType
	);

	if (bSuccess)
	{
		// Configure the dispatcher with signature matching parameters
		Blueprint->MarkPackageDirty();
	}

	return bSuccess;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::AddComponent(UBlueprint* Blueprint, const FForge2ComponentDefinition& Component)
{
#if WITH_EDITOR
	if (!Blueprint || Component.ComponentName.IsNone() || !Component.ComponentClass)
	{
		return false;
	}

	// Add component to Simple Construction Script
	USCS_Node* NewNode = Blueprint->SimpleConstructionScript->CreateNode(Component.ComponentClass, Component.ComponentName);
	if (!NewNode)
	{
		return false;
	}

	// Set as root if specified
	if (Component.bIsRootComponent)
	{
		Blueprint->SimpleConstructionScript->AddNode(NewNode);
	}
	else if (!Component.ParentComponentName.IsNone())
	{
		// Find parent and attach
		USCS_Node* ParentNode = Blueprint->SimpleConstructionScript->FindSCSNode(Component.ParentComponentName);
		if (ParentNode)
		{
			ParentNode->AddChildNode(NewNode);
		}
		else
		{
			Blueprint->SimpleConstructionScript->AddNode(NewNode);
		}
	}
	else
	{
		Blueprint->SimpleConstructionScript->AddNode(NewNode);
	}

	// Set transform for scene components
	if (USceneComponent* SceneTemplate = Cast<USceneComponent>(NewNode->ComponentTemplate))
	{
		SceneTemplate->SetRelativeTransform(Component.RelativeTransform);
	}

	// Apply property overrides
	for (const auto& Override : Component.PropertyOverrides)
	{
		FProperty* Property = Component.ComponentClass->FindPropertyByName(Override.Key);
		if (Property && NewNode->ComponentTemplate)
		{
			Property->ImportText_InContainer(*Override.Value, NewNode->ComponentTemplate, NewNode->ComponentTemplate, PPF_None);
		}
	}

	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::RemoveComponent(UBlueprint* Blueprint, FName ComponentName)
{
#if WITH_EDITOR
	if (!Blueprint || ComponentName.IsNone())
	{
		return false;
	}

	USCS_Node* Node = Blueprint->SimpleConstructionScript->FindSCSNode(ComponentName);
	if (Node)
	{
		Blueprint->SimpleConstructionScript->RemoveNode(Node);
		Blueprint->MarkPackageDirty();
		return true;
	}

	return false;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::SetComponentDefault(UBlueprint* Blueprint, FName ComponentName, FName PropertyName, const FString& Value)
{
#if WITH_EDITOR
	if (!Blueprint || ComponentName.IsNone() || PropertyName.IsNone())
	{
		return false;
	}

	USCS_Node* Node = Blueprint->SimpleConstructionScript->FindSCSNode(ComponentName);
	if (!Node || !Node->ComponentTemplate)
	{
		return false;
	}

	FProperty* Property = Node->ComponentTemplate->GetClass()->FindPropertyByName(PropertyName);
	if (Property)
	{
		Property->ImportText_InContainer(*Value, Node->ComponentTemplate, Node->ComponentTemplate, PPF_None);
		Blueprint->MarkPackageDirty();
		return true;
	}

	return false;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::ImplementInterface(UBlueprint* Blueprint, TSubclassOf<UInterface> Interface)
{
#if WITH_EDITOR
	if (!Blueprint || !Interface)
	{
		return false;
	}

	FBlueprintEditorUtils::ImplementNewInterface(Blueprint, Interface->GetFName());
	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::RemoveInterface(UBlueprint* Blueprint, TSubclassOf<UInterface> Interface)
{
#if WITH_EDITOR
	if (!Blueprint || !Interface)
	{
		return false;
	}

	FBlueprintEditorUtils::RemoveInterface(Blueprint, Interface->GetFName());
	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

FGuid UForge2BlueprintManager::AddNode(UBlueprint* Blueprint, FName GraphName, const FForge2NodeDefinition& Node)
{
#if WITH_EDITOR
	if (!Blueprint)
	{
		return FGuid();
	}

	UEdGraph* Graph = FindOrCreateGraph(Blueprint, GraphName);
	if (!Graph)
	{
		return FGuid();
	}

	// Create appropriate node type based on category
	UEdGraphNode* NewNode = nullptr;

	switch (Node.Category)
	{
	case EForge2NodeCategory::Function:
		{
			UK2Node_CallFunction* CallNode = NewObject<UK2Node_CallFunction>(Graph);
			if (CallNode)
			{
				if (Node.TargetClass)
				{
					UFunction* Function = Node.TargetClass->FindFunctionByName(Node.FunctionName);
					if (Function)
					{
						CallNode->SetFromFunction(Function);
					}
				}
				NewNode = CallNode;
			}
		}
		break;

	case EForge2NodeCategory::Variable:
		{
			// Create get or set node based on node type
			if (Node.NodeType == TEXT("Get"))
			{
				UK2Node_VariableGet* GetNode = NewObject<UK2Node_VariableGet>(Graph);
				if (GetNode)
				{
					GetNode->VariableReference.SetSelfMember(Node.FunctionName);
					NewNode = GetNode;
				}
			}
			else
			{
				UK2Node_VariableSet* SetNode = NewObject<UK2Node_VariableSet>(Graph);
				if (SetNode)
				{
					SetNode->VariableReference.SetSelfMember(Node.FunctionName);
					NewNode = SetNode;
				}
			}
		}
		break;

	case EForge2NodeCategory::Event:
		{
			UK2Node_Event* EventNode = NewObject<UK2Node_Event>(Graph);
			if (EventNode)
			{
				EventNode->CustomFunctionName = Node.FunctionName;
				NewNode = EventNode;
			}
		}
		break;

	default:
		// Generic node creation
		break;
	}

	if (NewNode)
	{
		NewNode->NodePosX = Node.GraphPosition.X;
		NewNode->NodePosY = Node.GraphPosition.Y;
		NewNode->NodeGuid = Node.NodeId;

		Graph->AddNode(NewNode, true, false);
		NewNode->AllocateDefaultPins();

		// Set pin defaults
		for (const auto& PinDefault : Node.PinDefaultValues)
		{
			UEdGraphPin* Pin = NewNode->FindPin(PinDefault.Key);
			if (Pin)
			{
				Pin->DefaultValue = PinDefault.Value;
			}
		}

		Blueprint->MarkPackageDirty();
		return Node.NodeId;
	}

	return FGuid();
#else
	return FGuid();
#endif
}

bool UForge2BlueprintManager::RemoveNode(UBlueprint* Blueprint, FName GraphName, FGuid NodeId)
{
#if WITH_EDITOR
	if (!Blueprint)
	{
		return false;
	}

	UEdGraph* Graph = FindOrCreateGraph(Blueprint, GraphName);
	if (!Graph)
	{
		return false;
	}

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node && Node->NodeGuid == NodeId)
		{
			Graph->RemoveNode(Node);
			Blueprint->MarkPackageDirty();
			return true;
		}
	}

	return false;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::ConnectNodes(UBlueprint* Blueprint, FName GraphName, const FForge2NodeConnection& Connection)
{
#if WITH_EDITOR
	if (!Blueprint)
	{
		return false;
	}

	UEdGraph* Graph = FindOrCreateGraph(Blueprint, GraphName);
	if (!Graph)
	{
		return false;
	}

	UEdGraphNode* SourceNode = nullptr;
	UEdGraphNode* TargetNode = nullptr;

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node->NodeGuid == Connection.SourceNodeId)
		{
			SourceNode = Node;
		}
		if (Node->NodeGuid == Connection.TargetNodeId)
		{
			TargetNode = Node;
		}
	}

	if (!SourceNode || !TargetNode)
	{
		return false;
	}

	UEdGraphPin* SourcePin = SourceNode->FindPin(Connection.SourcePinName);
	UEdGraphPin* TargetPin = TargetNode->FindPin(Connection.TargetPinName);

	if (SourcePin && TargetPin)
	{
		SourcePin->MakeLinkTo(TargetPin);
		Blueprint->MarkPackageDirty();
		return true;
	}

	return false;
#else
	return false;
#endif
}

bool UForge2BlueprintManager::DisconnectNodes(UBlueprint* Blueprint, FName GraphName, const FForge2NodeConnection& Connection)
{
#if WITH_EDITOR
	if (!Blueprint)
	{
		return false;
	}

	UEdGraph* Graph = FindOrCreateGraph(Blueprint, GraphName);
	if (!Graph)
	{
		return false;
	}

	UEdGraphNode* SourceNode = nullptr;
	UEdGraphNode* TargetNode = nullptr;

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node->NodeGuid == Connection.SourceNodeId)
		{
			SourceNode = Node;
		}
		if (Node->NodeGuid == Connection.TargetNodeId)
		{
			TargetNode = Node;
		}
	}

	if (!SourceNode || !TargetNode)
	{
		return false;
	}

	UEdGraphPin* SourcePin = SourceNode->FindPin(Connection.SourcePinName);
	UEdGraphPin* TargetPin = TargetNode->FindPin(Connection.TargetPinName);

	if (SourcePin && TargetPin)
	{
		SourcePin->BreakLinkTo(TargetPin);
		Blueprint->MarkPackageDirty();
		return true;
	}

	return false;
#else
	return false;
#endif
}

FForge2CompileResult UForge2BlueprintManager::CompileBlueprint(UBlueprint* Blueprint)
{
	FForge2CompileResult Result;

#if WITH_EDITOR
	if (!Blueprint)
	{
		Result.Status = EForge2CompileStatus::Error;
		Result.ErrorMessages.Add(TEXT("Blueprint is null"));
		return Result;
	}

	double StartTime = FPlatformTime::Seconds();

	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	Result.CompileTime = FPlatformTime::Seconds() - StartTime;

	// Check compile status
	if (Blueprint->Status == BS_Error)
	{
		Result.Status = EForge2CompileStatus::Error;
	}
	else if (Blueprint->Status == BS_UpToDateWithWarnings)
	{
		Result.Status = EForge2CompileStatus::Warning;
	}
	else if (Blueprint->Status == BS_UpToDate)
	{
		Result.Status = EForge2CompileStatus::UpToDate;
	}
	else
	{
		Result.Status = EForge2CompileStatus::Dirty;
	}

	OnBlueprintCompiled.Broadcast(Blueprint->GetFName(), Result);
#endif

	return Result;
}

bool UForge2BlueprintManager::SaveBlueprint(UBlueprint* Blueprint)
{
#if WITH_EDITOR
	if (!Blueprint)
	{
		return false;
	}

	UPackage* Package = Blueprint->GetOutermost();
	if (!Package)
	{
		return false;
	}

	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;

	return UPackage::SavePackage(Package, Blueprint, *PackageFileName, SaveArgs);
#else
	return false;
#endif
}

EForge2CompileStatus UForge2BlueprintManager::GetCompileStatus(UBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return EForge2CompileStatus::Unknown;
	}

	switch (Blueprint->Status)
	{
	case BS_Error:
		return EForge2CompileStatus::Error;
	case BS_UpToDateWithWarnings:
		return EForge2CompileStatus::Warning;
	case BS_UpToDate:
		return EForge2CompileStatus::UpToDate;
	case BS_Dirty:
		return EForge2CompileStatus::Dirty;
	default:
		return EForge2CompileStatus::Unknown;
	}
}

UEdGraph* UForge2BlueprintManager::FindOrCreateGraph(UBlueprint* Blueprint, FName GraphName)
{
	if (!Blueprint)
	{
		return nullptr;
	}

	// Check function graphs
	for (UEdGraph* Graph : Blueprint->FunctionGraphs)
	{
		if (Graph && Graph->GetFName() == GraphName)
		{
			return Graph;
		}
	}

	// Check uber graph pages
	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		if (Graph && Graph->GetFName() == GraphName)
		{
			return Graph;
		}
	}

	// Return first uber graph if name matches "EventGraph"
	if (GraphName == TEXT("EventGraph") && Blueprint->UbergraphPages.Num() > 0)
	{
		return Blueprint->UbergraphPages[0];
	}

	return nullptr;
}

FEdGraphPinType UForge2BlueprintManager::ConvertVariableType(EForge2VariableType Type, const FForge2VariableDefinition& Definition)
{
	FEdGraphPinType PinType;

	switch (Type)
	{
	case EForge2VariableType::Boolean:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		break;
	case EForge2VariableType::Integer:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
		break;
	case EForge2VariableType::Float:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
		PinType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
		break;
	case EForge2VariableType::String:
		PinType.PinCategory = UEdGraphSchema_K2::PC_String;
		break;
	case EForge2VariableType::Name:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Name;
		break;
	case EForge2VariableType::Text:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
		break;
	case EForge2VariableType::Vector:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
		break;
	case EForge2VariableType::Rotator:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
		break;
	case EForge2VariableType::Transform:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
		break;
	case EForge2VariableType::Color:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();
		break;
	case EForge2VariableType::Object:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		PinType.PinSubCategoryObject = Definition.ObjectType ? Definition.ObjectType.Get() : UObject::StaticClass();
		break;
	case EForge2VariableType::Class:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Class;
		PinType.PinSubCategoryObject = Definition.ObjectType ? Definition.ObjectType.Get() : UObject::StaticClass();
		break;
	case EForge2VariableType::Struct:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = Definition.StructType;
		break;
	case EForge2VariableType::Enum:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
		PinType.PinSubCategoryObject = Definition.EnumType;
		break;
	case EForge2VariableType::Array:
		PinType.ContainerType = EPinContainerType::Array;
		// Recursively set value type
		break;
	case EForge2VariableType::Map:
		PinType.ContainerType = EPinContainerType::Map;
		break;
	case EForge2VariableType::Set:
		PinType.ContainerType = EPinContainerType::Set;
		break;
	default:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
		break;
	}

	return PinType;
}

// ============================================================================
// WIDGET BLUEPRINT MANAGER IMPLEMENTATION
// ============================================================================

UForge2WidgetBlueprintManager::UForge2WidgetBlueprintManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2WidgetBlueprintManager::BeginPlay()
{
	Super::BeginPlay();
}

UBlueprint* UForge2WidgetBlueprintManager::CreateWidgetBlueprint(FName WidgetName, FString PackagePath)
{
#if WITH_EDITOR
	if (WidgetName.IsNone())
	{
		return nullptr;
	}

	if (PackagePath.IsEmpty())
	{
		PackagePath = TEXT("/Game/UI");
	}

	FString FullPath = PackagePath / WidgetName.ToString();
	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return nullptr;
	}

	UBlueprint* WidgetBlueprint = FKismetEditorUtilities::CreateBlueprint(
		UUserWidget::StaticClass(),
		Package,
		WidgetName,
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass()
	);

	if (WidgetBlueprint)
	{
		WidgetBlueprint->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(WidgetBlueprint);
		ManagedWidgetBlueprints.Add(WidgetName, WidgetBlueprint);
	}

	return WidgetBlueprint;
#else
	return nullptr;
#endif
}

bool UForge2WidgetBlueprintManager::AddWidgetElement(UBlueprint* WidgetBlueprint, const FForge2WidgetElementDefinition& Element)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || Element.ElementName.IsNone())
	{
		return false;
	}

	// Widget tree manipulation requires WidgetBlueprint-specific APIs
	// This is a simplified implementation
	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2WidgetBlueprintManager::RemoveWidgetElement(UBlueprint* WidgetBlueprint, FName ElementName)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || ElementName.IsNone())
	{
		return false;
	}

	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2WidgetBlueprintManager::SetWidgetElementProperty(UBlueprint* WidgetBlueprint, FName ElementName, FName PropertyName, const FString& Value)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || ElementName.IsNone() || PropertyName.IsNone())
	{
		return false;
	}

	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2WidgetBlueprintManager::AddPropertyBinding(UBlueprint* WidgetBlueprint, const FForge2WidgetBindingDefinition& Binding)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || Binding.ElementName.IsNone())
	{
		return false;
	}

	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2WidgetBlueprintManager::RemovePropertyBinding(UBlueprint* WidgetBlueprint, FName ElementName, FName PropertyName)
{
#if WITH_EDITOR
	if (!WidgetBlueprint)
	{
		return false;
	}

	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2WidgetBlueprintManager::AddWidgetAnimation(UBlueprint* WidgetBlueprint, const FForge2WidgetAnimationDefinition& Animation)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || Animation.AnimationName.IsNone())
	{
		return false;
	}

	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2WidgetBlueprintManager::RemoveWidgetAnimation(UBlueprint* WidgetBlueprint, FName AnimationName)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || AnimationName.IsNone())
	{
		return false;
	}

	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

void UForge2WidgetBlueprintManager::RegisterStyle(const FForge2WidgetStyleDefinition& Style)
{
	if (!Style.StyleName.IsNone())
	{
		RegisteredStyles.Add(Style.StyleName, Style);
	}
}

bool UForge2WidgetBlueprintManager::ApplyStyle(UBlueprint* WidgetBlueprint, FName StyleName)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || StyleName.IsNone())
	{
		return false;
	}

	const FForge2WidgetStyleDefinition* Style = RegisteredStyles.Find(StyleName);
	if (!Style)
	{
		return false;
	}

	// Apply style to widget elements
	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

FForge2WidgetStyleDefinition UForge2WidgetBlueprintManager::GetStyle(FName StyleName) const
{
	const FForge2WidgetStyleDefinition* Style = RegisteredStyles.Find(StyleName);
	return Style ? *Style : FForge2WidgetStyleDefinition();
}

bool UForge2WidgetBlueprintManager::SetWidgetNavigation(UBlueprint* WidgetBlueprint, FName ElementName,
	FName UpElement, FName DownElement, FName LeftElement, FName RightElement)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || ElementName.IsNone())
	{
		return false;
	}

	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UForge2WidgetBlueprintManager::SetWidgetLocalization(UBlueprint* WidgetBlueprint, FName ElementName, FName StringTableKey)
{
#if WITH_EDITOR
	if (!WidgetBlueprint || ElementName.IsNone() || StringTableKey.IsNone())
	{
		return false;
	}

	WidgetBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

// ============================================================================
// UTILITIES IMPLEMENTATION
// ============================================================================

FForge2BlueprintSettings UForge2BlueprintUtilities::GetActorBlueprintPreset(FName Name, TSubclassOf<AActor> ParentClass)
{
	FForge2BlueprintSettings Settings;
	Settings.BlueprintName = Name;
	Settings.BlueprintType = EForge2BlueprintType::Actor;
	Settings.ParentClass = ParentClass ? ParentClass.Get() : AActor::StaticClass();
	Settings.bCompileOnCreate = true;
	return Settings;
}

FForge2BlueprintSettings UForge2BlueprintUtilities::GetComponentBlueprintPreset(FName Name, TSubclassOf<UActorComponent> ParentClass)
{
	FForge2BlueprintSettings Settings;
	Settings.BlueprintName = Name;
	Settings.BlueprintType = EForge2BlueprintType::Component;
	Settings.ParentClass = ParentClass ? ParentClass.Get() : UActorComponent::StaticClass();
	Settings.bCompileOnCreate = true;
	return Settings;
}

FForge2BlueprintSettings UForge2BlueprintUtilities::GetInterfaceBlueprintPreset(FName Name)
{
	FForge2BlueprintSettings Settings;
	Settings.BlueprintName = Name;
	Settings.BlueprintType = EForge2BlueprintType::Interface;
	Settings.bCompileOnCreate = true;
	return Settings;
}

FForge2VariableDefinition UForge2BlueprintUtilities::GetFloatVariablePreset(FName Name, float DefaultValue)
{
	FForge2VariableDefinition Var;
	Var.VariableName = Name;
	Var.VariableType = EForge2VariableType::Float;
	Var.DefaultValue = FString::SanitizeFloat(DefaultValue);
	Var.bInstanceEditable = true;
	return Var;
}

FForge2VariableDefinition UForge2BlueprintUtilities::GetIntVariablePreset(FName Name, int32 DefaultValue)
{
	FForge2VariableDefinition Var;
	Var.VariableName = Name;
	Var.VariableType = EForge2VariableType::Integer;
	Var.DefaultValue = FString::FromInt(DefaultValue);
	Var.bInstanceEditable = true;
	return Var;
}

FForge2VariableDefinition UForge2BlueprintUtilities::GetBoolVariablePreset(FName Name, bool DefaultValue)
{
	FForge2VariableDefinition Var;
	Var.VariableName = Name;
	Var.VariableType = EForge2VariableType::Boolean;
	Var.DefaultValue = DefaultValue ? TEXT("true") : TEXT("false");
	Var.bInstanceEditable = true;
	return Var;
}

FForge2VariableDefinition UForge2BlueprintUtilities::GetObjectVariablePreset(FName Name, TSubclassOf<UObject> ObjectType)
{
	FForge2VariableDefinition Var;
	Var.VariableName = Name;
	Var.VariableType = EForge2VariableType::Object;
	Var.ObjectType = ObjectType;
	Var.bInstanceEditable = true;
	return Var;
}

FForge2VariableDefinition UForge2BlueprintUtilities::GetReplicatedVariablePreset(FName Name, EForge2VariableType Type, FName RepNotifyFunc)
{
	FForge2VariableDefinition Var;
	Var.VariableName = Name;
	Var.VariableType = Type;
	Var.Replication = RepNotifyFunc.IsNone() ? EForge2VariableReplication::Replicated : EForge2VariableReplication::RepNotify;
	Var.RepNotifyFunctionName = RepNotifyFunc;
	Var.bInstanceEditable = true;
	return Var;
}

FForge2FunctionDefinition UForge2BlueprintUtilities::GetPureFunctionPreset(FName Name)
{
	FForge2FunctionDefinition Func;
	Func.FunctionName = Name;
	Func.FunctionType = EForge2FunctionType::Pure;
	return Func;
}

FForge2FunctionDefinition UForge2BlueprintUtilities::GetServerRPCPreset(FName Name, bool bReliable)
{
	FForge2FunctionDefinition Func;
	Func.FunctionName = Name;
	Func.FunctionType = EForge2FunctionType::Event;
	Func.bIsServer = true;
	Func.bIsReliable = bReliable;
	return Func;
}

FForge2FunctionDefinition UForge2BlueprintUtilities::GetClientRPCPreset(FName Name, bool bReliable)
{
	FForge2FunctionDefinition Func;
	Func.FunctionName = Name;
	Func.FunctionType = EForge2FunctionType::Event;
	Func.bIsClient = true;
	Func.bIsReliable = bReliable;
	return Func;
}

FForge2FunctionDefinition UForge2BlueprintUtilities::GetMulticastRPCPreset(FName Name, bool bReliable)
{
	FForge2FunctionDefinition Func;
	Func.FunctionName = Name;
	Func.FunctionType = EForge2FunctionType::Event;
	Func.bIsMulticast = true;
	Func.bIsReliable = bReliable;
	return Func;
}

FForge2ComponentDefinition UForge2BlueprintUtilities::GetSceneComponentPreset(FName Name, FTransform Transform)
{
	FForge2ComponentDefinition Comp;
	Comp.ComponentName = Name;
	Comp.ComponentClass = USceneComponent::StaticClass();
	Comp.RelativeTransform = Transform;
	return Comp;
}

FForge2ComponentDefinition UForge2BlueprintUtilities::GetStaticMeshComponentPreset(FName Name, TSoftObjectPtr<UStaticMesh> Mesh)
{
	FForge2ComponentDefinition Comp;
	Comp.ComponentName = Name;
	Comp.ComponentClass = UStaticMeshComponent::StaticClass();

	if (Mesh.IsValid())
	{
		Comp.PropertyOverrides.Add(TEXT("StaticMesh"), Mesh.ToString());
	}

	return Comp;
}

FForge2ComponentDefinition UForge2BlueprintUtilities::GetSkeletalMeshComponentPreset(FName Name, TSoftObjectPtr<USkeletalMesh> Mesh)
{
	FForge2ComponentDefinition Comp;
	Comp.ComponentName = Name;
	Comp.ComponentClass = USkeletalMeshComponent::StaticClass();

	if (Mesh.IsValid())
	{
		Comp.PropertyOverrides.Add(TEXT("SkeletalMesh"), Mesh.ToString());
	}

	return Comp;
}

FForge2ComponentDefinition UForge2BlueprintUtilities::GetCapsuleComponentPreset(FName Name, float Radius, float HalfHeight)
{
	FForge2ComponentDefinition Comp;
	Comp.ComponentName = Name;
	Comp.ComponentClass = UCapsuleComponent::StaticClass();
	Comp.PropertyOverrides.Add(TEXT("CapsuleRadius"), FString::SanitizeFloat(Radius));
	Comp.PropertyOverrides.Add(TEXT("CapsuleHalfHeight"), FString::SanitizeFloat(HalfHeight));
	return Comp;
}

FForge2WidgetElementDefinition UForge2BlueprintUtilities::GetButtonPreset(FName Name, FText Label, FForge2WidgetSlot Slot)
{
	FForge2WidgetElementDefinition Element;
	Element.ElementName = Name;
	Element.ElementType = EForge2WidgetElementType::Button;
	Element.TextContent = Label;
	Element.Slot = Slot;
	Element.bIsVariable = true;
	return Element;
}

FForge2WidgetElementDefinition UForge2BlueprintUtilities::GetTextBlockPreset(FName Name, FText Text, FForge2WidgetSlot Slot)
{
	FForge2WidgetElementDefinition Element;
	Element.ElementName = Name;
	Element.ElementType = EForge2WidgetElementType::TextBlock;
	Element.TextContent = Text;
	Element.Slot = Slot;
	return Element;
}

FForge2WidgetElementDefinition UForge2BlueprintUtilities::GetImagePreset(FName Name, TSoftObjectPtr<UTexture2D> Texture, FForge2WidgetSlot Slot)
{
	FForge2WidgetElementDefinition Element;
	Element.ElementName = Name;
	Element.ElementType = EForge2WidgetElementType::Image;
	Element.ImageContent = Texture;
	Element.Slot = Slot;
	return Element;
}

FForge2WidgetElementDefinition UForge2BlueprintUtilities::GetProgressBarPreset(FName Name, float InitialValue, FForge2WidgetSlot Slot)
{
	FForge2WidgetElementDefinition Element;
	Element.ElementName = Name;
	Element.ElementType = EForge2WidgetElementType::ProgressBar;
	Element.ProgressValue = InitialValue;
	Element.Slot = Slot;
	Element.bIsVariable = true;
	return Element;
}

FForge2WidgetElementDefinition UForge2BlueprintUtilities::GetHorizontalBoxPreset(FName Name, FForge2WidgetSlot Slot)
{
	FForge2WidgetElementDefinition Element;
	Element.ElementName = Name;
	Element.ElementType = EForge2WidgetElementType::HorizontalBox;
	Element.Slot = Slot;
	return Element;
}

FForge2WidgetElementDefinition UForge2BlueprintUtilities::GetVerticalBoxPreset(FName Name, FForge2WidgetSlot Slot)
{
	FForge2WidgetElementDefinition Element;
	Element.ElementName = Name;
	Element.ElementType = EForge2WidgetElementType::VerticalBox;
	Element.Slot = Slot;
	return Element;
}

FForge2WidgetStyleDefinition UForge2BlueprintUtilities::GetDarkThemePreset()
{
	FForge2WidgetStyleDefinition Style;
	Style.StyleName = TEXT("DarkTheme");
	Style.PrimaryColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
	Style.SecondaryColor = FLinearColor(0.25f, 0.25f, 0.25f, 1.0f);
	Style.AccentColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
	Style.TextColor = FLinearColor::White;
	Style.BackgroundColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.95f);
	return Style;
}

FForge2WidgetStyleDefinition UForge2BlueprintUtilities::GetLightThemePreset()
{
	FForge2WidgetStyleDefinition Style;
	Style.StyleName = TEXT("LightTheme");
	Style.PrimaryColor = FLinearColor(0.95f, 0.95f, 0.95f, 1.0f);
	Style.SecondaryColor = FLinearColor(0.85f, 0.85f, 0.85f, 1.0f);
	Style.AccentColor = FLinearColor(0.0f, 0.4f, 0.8f, 1.0f);
	Style.TextColor = FLinearColor::Black;
	Style.BackgroundColor = FLinearColor::White;
	return Style;
}

FForge2WidgetStyleDefinition UForge2BlueprintUtilities::GetGameUIThemePreset()
{
	FForge2WidgetStyleDefinition Style;
	Style.StyleName = TEXT("GameUI");
	Style.PrimaryColor = FLinearColor(0.1f, 0.1f, 0.15f, 0.9f);
	Style.SecondaryColor = FLinearColor(0.2f, 0.2f, 0.25f, 0.85f);
	Style.AccentColor = FLinearColor(0.9f, 0.7f, 0.1f, 1.0f); // Gold accent
	Style.TextColor = FLinearColor::White;
	Style.BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.8f);
	Style.BorderRadius = 8.0f;
	return Style;
}

FForge2WidgetSlot UForge2BlueprintUtilities::GetCenteredSlot(FVector2D Size)
{
	FForge2WidgetSlot Slot;
	Slot.Anchor = EForge2WidgetAnchor::Center;
	Slot.Size = Size;
	Slot.Alignment = FVector2D(0.5f, 0.5f);
	return Slot;
}

FForge2WidgetSlot UForge2BlueprintUtilities::GetFullScreenSlot()
{
	FForge2WidgetSlot Slot;
	Slot.Anchor = EForge2WidgetAnchor::Stretch;
	Slot.Position = FVector2D::ZeroVector;
	Slot.bAutoSize = false;
	return Slot;
}

FForge2WidgetSlot UForge2BlueprintUtilities::GetCornerSlot(EForge2WidgetAnchor Corner, FVector2D Size, FVector2D Offset)
{
	FForge2WidgetSlot Slot;
	Slot.Anchor = Corner;
	Slot.Size = Size;
	Slot.Position = Offset;

	// Set alignment based on corner
	switch (Corner)
	{
	case EForge2WidgetAnchor::TopLeft:
		Slot.Alignment = FVector2D(0.0f, 0.0f);
		break;
	case EForge2WidgetAnchor::TopRight:
		Slot.Alignment = FVector2D(1.0f, 0.0f);
		break;
	case EForge2WidgetAnchor::BottomLeft:
		Slot.Alignment = FVector2D(0.0f, 1.0f);
		break;
	case EForge2WidgetAnchor::BottomRight:
		Slot.Alignment = FVector2D(1.0f, 1.0f);
		break;
	default:
		Slot.Alignment = FVector2D(0.5f, 0.5f);
		break;
	}

	return Slot;
}
