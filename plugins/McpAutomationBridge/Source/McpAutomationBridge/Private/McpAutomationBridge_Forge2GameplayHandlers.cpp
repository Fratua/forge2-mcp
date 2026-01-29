// McpAutomationBridge_Forge2GameplayHandlers.cpp
// FORGE2: Complete Gameplay System Handlers (Combat, Inventory, Skills, Dialogue)

#include "McpAutomationBridgeSubsystem.h"
#include "McpAutomationBridgeHelpers.h"
#include "McpAutomationBridgeGlobals.h"
#include "Forge2DataAssets.h"
#include "ScopedTransaction.h"

#if WITH_EDITOR
#include "Async/Async.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/Material.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/DataAssetFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EditorAssetLibrary.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimMontage.h"
#endif

// ============================================================================
// Forge2 Gameplay Helpers
// ============================================================================

#if WITH_EDITOR
namespace Forge2GameplayHelpers
{
    static FString GetActionFromPayload(const TSharedPtr<FJsonObject>& Payload)
    {
        if (!Payload.IsValid()) return TEXT("");
        const TSharedPtr<FJsonObject>* ParamsObj = nullptr;
        if (Payload->TryGetObjectField(TEXT("params"), ParamsObj) && ParamsObj && (*ParamsObj).IsValid())
        {
            FString ActionStr;
            if ((*ParamsObj)->TryGetStringField(TEXT("action"), ActionStr)) return ActionStr;
        }
        FString ActionStr;
        if (Payload->TryGetStringField(TEXT("action"), ActionStr)) return ActionStr;
        return TEXT("");
    }

    static TSharedPtr<FJsonObject> GetParams(const TSharedPtr<FJsonObject>& Payload)
    {
        if (!Payload.IsValid()) return nullptr;
        const TSharedPtr<FJsonObject>* ParamsObj = nullptr;
        if (Payload->TryGetObjectField(TEXT("params"), ParamsObj) && ParamsObj) return *ParamsObj;
        return Payload;
    }

    static UPackage* CreateAssetPackage(const FString& Path, const FString& Name)
    {
        FString PackagePath = Path.IsEmpty() ? TEXT("/Game/FORGE2") : Path;
        if (!PackagePath.StartsWith(TEXT("/"))) PackagePath = TEXT("/Game/") + PackagePath;
        return CreatePackage(*(PackagePath / Name));
    }

    static FVector GetVectorFromJson(const TSharedPtr<FJsonObject>& Obj)
    {
        if (!Obj.IsValid()) return FVector::ZeroVector;
        double X = 0, Y = 0, Z = 0;
        Obj->TryGetNumberField(TEXT("x"), X);
        Obj->TryGetNumberField(TEXT("y"), Y);
        Obj->TryGetNumberField(TEXT("z"), Z);
        return FVector(X, Y, Z);
    }

    static FLinearColor GetColorFromJson(const TSharedPtr<FJsonObject>& Obj)
    {
        if (!Obj.IsValid()) return FLinearColor::White;
        double R = 1, G = 1, B = 1, A = 1;
        Obj->TryGetNumberField(TEXT("r"), R);
        Obj->TryGetNumberField(TEXT("g"), G);
        Obj->TryGetNumberField(TEXT("b"), B);
        Obj->TryGetNumberField(TEXT("a"), A);
        return FLinearColor(R, G, B, A);
    }
}
#endif

// ============================================================================
// FORGE2 DIALOGUE HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2DialogueAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 dialogue requires editor"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2GameplayHelpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2GameplayHelpers::GetParams(Payload);

    // ========================================================================
    // create_dialogue_tree - Create new dialogue tree asset
    // ========================================================================
    if (SubAction == TEXT("create_dialogue_tree"))
    {
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("NewDialogue"));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Dialogues"));

        UPackage* Package = Forge2GameplayHelpers::CreateAssetPackage(Path, Name);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpDialogueTreeAsset* DialogueAsset = NewObject<UMcpDialogueTreeAsset>(
            Package, FName(*Name), RF_Public | RF_Standalone);

        if (DialogueAsset)
        {
            DialogueAsset->AssetId = FName(*Name);
            DialogueAsset->DisplayName = FText::FromString(Name);
            DialogueAsset->Description = FText::FromString(GetJsonStringField(Params, TEXT("description"), TEXT("")));

            // Create initial start node
            FForge2DialogueNode StartNode;
            StartNode.NodeId = FName(TEXT("Start"));
            StartNode.NodeType = EForge2DialogueNodeType::NPCSpeech;
            StartNode.DialogueText = FText::FromString(GetJsonStringField(Params, TEXT("start_text"), TEXT("Hello!")));
            DialogueAsset->Nodes.Add(StartNode);
            DialogueAsset->StartNodeId = StartNode.NodeId;

            DialogueAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(DialogueAsset);
            McpSafeAssetSave(DialogueAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("name"), Name);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetStringField(TEXT("start_node"), TEXT("Start"));

            SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue tree created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create dialogue asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // ========================================================================
    // add_dialogue_node - Add node to dialogue tree
    // ========================================================================
    if (SubAction == TEXT("add_dialogue_node"))
    {
        FString DialoguePath = GetJsonStringField(Params, TEXT("dialogue_path"), TEXT(""));
        FString NodeId = GetJsonStringField(Params, TEXT("node_id"), TEXT(""));
        FString NodeType = GetJsonStringField(Params, TEXT("type"), TEXT("npc_speech"));
        FString Text = GetJsonStringField(Params, TEXT("text"), TEXT(""));
        FString SpeakerId = GetJsonStringField(Params, TEXT("speaker_id"), TEXT(""));

        if (DialoguePath.IsEmpty() || NodeId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing dialogue_path or node_id"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpDialogueTreeAsset* DialogueAsset = LoadObject<UMcpDialogueTreeAsset>(nullptr, *DialoguePath);
        if (!DialogueAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Dialogue asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        FForge2DialogueNode NewNode;
        NewNode.NodeId = FName(*NodeId);
        NewNode.DialogueText = FText::FromString(Text);
        NewNode.SpeakerId = FName(*SpeakerId);

        if (NodeType == TEXT("player_choice"))
            NewNode.NodeType = EForge2DialogueNodeType::PlayerChoice;
        else if (NodeType == TEXT("condition"))
            NewNode.NodeType = EForge2DialogueNodeType::Condition;
        else if (NodeType == TEXT("event"))
            NewNode.NodeType = EForge2DialogueNodeType::Event;
        else if (NodeType == TEXT("random"))
            NewNode.NodeType = EForge2DialogueNodeType::Random;
        else if (NodeType == TEXT("end"))
            NewNode.NodeType = EForge2DialogueNodeType::End;
        else
            NewNode.NodeType = EForge2DialogueNodeType::NPCSpeech;

        // Get next node IDs if provided
        const TArray<TSharedPtr<FJsonValue>>* NextNodesArray;
        if (Params->TryGetArrayField(TEXT("next_nodes"), NextNodesArray))
        {
            for (const auto& NextVal : *NextNodesArray)
            {
                NewNode.NextNodeIds.Add(FName(*NextVal->AsString()));
            }
        }

        // Editor position
        const TSharedPtr<FJsonObject>* PosObj;
        if (Params->TryGetObjectField(TEXT("position"), PosObj))
        {
            double X = 0, Y = 0;
            (*PosObj)->TryGetNumberField(TEXT("x"), X);
            (*PosObj)->TryGetNumberField(TEXT("y"), Y);
            NewNode.EditorPosition = FVector2D(X, Y);
        }

        DialogueAsset->Nodes.Add(NewNode);
        DialogueAsset->MarkPackageDirty();
        McpSafeAssetSave(DialogueAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("dialogue_path"), DialoguePath);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("type"), NodeType);
        Result->SetNumberField(TEXT("total_nodes"), DialogueAsset->Nodes.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Dialogue node added"), Result);
        return true;
    }

    // ========================================================================
    // connect_dialogue_nodes - Connect two nodes
    // ========================================================================
    if (SubAction == TEXT("connect_dialogue_nodes"))
    {
        FString DialoguePath = GetJsonStringField(Params, TEXT("dialogue_path"), TEXT(""));
        FString FromNodeId = GetJsonStringField(Params, TEXT("from_node"), TEXT(""));
        FString ToNodeId = GetJsonStringField(Params, TEXT("to_node"), TEXT(""));

        if (DialoguePath.IsEmpty() || FromNodeId.IsEmpty() || ToNodeId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpDialogueTreeAsset* DialogueAsset = LoadObject<UMcpDialogueTreeAsset>(nullptr, *DialoguePath);
        if (!DialogueAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Dialogue asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        FForge2DialogueNode* FromNode = DialogueAsset->FindNode(FName(*FromNodeId));
        if (!FromNode)
        {
            SendAutomationError(Socket, RequestId, TEXT("From node not found"), TEXT("NODE_NOT_FOUND"));
            return true;
        }

        FromNode->NextNodeIds.AddUnique(FName(*ToNodeId));
        DialogueAsset->MarkPackageDirty();
        McpSafeAssetSave(DialogueAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("from_node"), FromNodeId);
        Result->SetStringField(TEXT("to_node"), ToNodeId);
        Result->SetNumberField(TEXT("connections"), FromNode->NextNodeIds.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Nodes connected"), Result);
        return true;
    }

    // ========================================================================
    // create_speaker_profile - Create speaker profile
    // ========================================================================
    if (SubAction == TEXT("create_speaker_profile"))
    {
        FString DialoguePath = GetJsonStringField(Params, TEXT("dialogue_path"), TEXT(""));
        FString SpeakerId = GetJsonStringField(Params, TEXT("speaker_id"), TEXT(""));
        FString SpeakerName = GetJsonStringField(Params, TEXT("name"), TEXT(""));
        FString PortraitPath = GetJsonStringField(Params, TEXT("portrait"), TEXT(""));

        if (DialoguePath.IsEmpty() || SpeakerId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpDialogueTreeAsset* DialogueAsset = LoadObject<UMcpDialogueTreeAsset>(nullptr, *DialoguePath);
        if (!DialogueAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Dialogue asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        FForge2SpeakerProfile NewSpeaker;
        NewSpeaker.SpeakerId = FName(*SpeakerId);
        NewSpeaker.DisplayName = FText::FromString(SpeakerName.IsEmpty() ? SpeakerId : SpeakerName);

        if (!PortraitPath.IsEmpty())
        {
            NewSpeaker.Portrait = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(PortraitPath));
        }

        const TSharedPtr<FJsonObject>* ColorObj;
        if (Params->TryGetObjectField(TEXT("color"), ColorObj))
        {
            NewSpeaker.NameColor = Forge2GameplayHelpers::GetColorFromJson(*ColorObj);
        }

        DialogueAsset->Speakers.Add(NewSpeaker);
        DialogueAsset->MarkPackageDirty();
        McpSafeAssetSave(DialogueAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("speaker_id"), SpeakerId);
        Result->SetStringField(TEXT("name"), NewSpeaker.DisplayName.ToString());
        Result->SetNumberField(TEXT("total_speakers"), DialogueAsset->Speakers.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Speaker profile created"), Result);
        return true;
    }

    // ========================================================================
    // add_choice_condition - Add condition to dialogue choice
    // ========================================================================
    if (SubAction == TEXT("add_choice_condition"))
    {
        FString DialoguePath = GetJsonStringField(Params, TEXT("dialogue_path"), TEXT(""));
        FString NodeId = GetJsonStringField(Params, TEXT("node_id"), TEXT(""));
        FString ConditionType = GetJsonStringField(Params, TEXT("condition_type"), TEXT(""));
        FString TargetId = GetJsonStringField(Params, TEXT("target_id"), TEXT(""));
        FString Operator = GetJsonStringField(Params, TEXT("operator"), TEXT(">="));
        int32 Value = GetJsonNumberField(Params, TEXT("value"), 0);

        if (DialoguePath.IsEmpty() || NodeId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpDialogueTreeAsset* DialogueAsset = LoadObject<UMcpDialogueTreeAsset>(nullptr, *DialoguePath);
        if (!DialogueAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Dialogue asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        FForge2DialogueNode* Node = DialogueAsset->FindNode(FName(*NodeId));
        if (!Node)
        {
            SendAutomationError(Socket, RequestId, TEXT("Node not found"), TEXT("NODE_NOT_FOUND"));
            return true;
        }

        FForge2DialogueCondition NewCondition;
        NewCondition.ConditionType = FName(*ConditionType);
        NewCondition.TargetId = FName(*TargetId);
        NewCondition.ComparisonOperator = Operator;
        NewCondition.CompareValue = Value;
        NewCondition.bInvert = GetJsonBoolField(Params, TEXT("invert"), false);

        Node->Conditions.Add(NewCondition);
        DialogueAsset->MarkPackageDirty();
        McpSafeAssetSave(DialogueAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("condition_type"), ConditionType);
        Result->SetNumberField(TEXT("total_conditions"), Node->Conditions.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Condition added"), Result);
        return true;
    }

    // ========================================================================
    // add_consequence - Add consequence to dialogue node
    // ========================================================================
    if (SubAction == TEXT("add_consequence"))
    {
        FString DialoguePath = GetJsonStringField(Params, TEXT("dialogue_path"), TEXT(""));
        FString NodeId = GetJsonStringField(Params, TEXT("node_id"), TEXT(""));
        FString ConsequenceType = GetJsonStringField(Params, TEXT("consequence_type"), TEXT(""));
        FString TargetId = GetJsonStringField(Params, TEXT("target_id"), TEXT(""));
        int32 Value = GetJsonNumberField(Params, TEXT("value"), 0);
        FString StringValue = GetJsonStringField(Params, TEXT("string_value"), TEXT(""));

        if (DialoguePath.IsEmpty() || NodeId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpDialogueTreeAsset* DialogueAsset = LoadObject<UMcpDialogueTreeAsset>(nullptr, *DialoguePath);
        if (!DialogueAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Dialogue asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        FForge2DialogueNode* Node = DialogueAsset->FindNode(FName(*NodeId));
        if (!Node)
        {
            SendAutomationError(Socket, RequestId, TEXT("Node not found"), TEXT("NODE_NOT_FOUND"));
            return true;
        }

        FForge2DialogueConsequence NewConsequence;
        NewConsequence.ConsequenceType = FName(*ConsequenceType);
        NewConsequence.TargetId = FName(*TargetId);
        NewConsequence.Value = Value;
        NewConsequence.StringValue = StringValue;

        Node->Consequences.Add(NewConsequence);
        DialogueAsset->MarkPackageDirty();
        McpSafeAssetSave(DialogueAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("consequence_type"), ConsequenceType);
        Result->SetNumberField(TEXT("total_consequences"), Node->Consequences.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Consequence added"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown dialogue action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 SKILL TREE HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2SkillTreeAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 skill_tree requires editor"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2GameplayHelpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2GameplayHelpers::GetParams(Payload);

    // ========================================================================
    // create_skill - Create new skill definition
    // ========================================================================
    if (SubAction == TEXT("create_skill"))
    {
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("NewSkill"));
        FString SkillId = GetJsonStringField(Params, TEXT("skill_id"), Name);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Skills"));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString SkillType = GetJsonStringField(Params, TEXT("type"), TEXT("active"));
        FString TargetType = GetJsonStringField(Params, TEXT("target_type"), TEXT("self"));
        int32 MaxRank = GetJsonNumberField(Params, TEXT("max_rank"), 5);

        UPackage* Package = Forge2GameplayHelpers::CreateAssetPackage(Path, Name);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpSkillDefinitionAsset* SkillAsset = NewObject<UMcpSkillDefinitionAsset>(
            Package, FName(*Name), RF_Public | RF_Standalone);

        if (SkillAsset)
        {
            SkillAsset->AssetId = FName(*SkillId);
            SkillAsset->DisplayName = FText::FromString(Name);
            SkillAsset->Description = FText::FromString(Description);
            SkillAsset->MaxRank = MaxRank;

            // Set skill type
            if (SkillType == TEXT("passive"))
                SkillAsset->SkillType = EForge2SkillType::Passive;
            else if (SkillType == TEXT("toggle"))
                SkillAsset->SkillType = EForge2SkillType::Toggle;
            else if (SkillType == TEXT("aura"))
                SkillAsset->SkillType = EForge2SkillType::Aura;
            else if (SkillType == TEXT("ultimate"))
                SkillAsset->SkillType = EForge2SkillType::Ultimate;
            else if (SkillType == TEXT("channeled"))
                SkillAsset->SkillType = EForge2SkillType::Channeled;
            else
                SkillAsset->SkillType = EForge2SkillType::Active;

            // Set target type
            if (TargetType == TEXT("single_enemy"))
                SkillAsset->TargetType = EForge2TargetType::SingleEnemy;
            else if (TargetType == TEXT("single_ally"))
                SkillAsset->TargetType = EForge2TargetType::SingleAlly;
            else if (TargetType == TEXT("area_enemy"))
                SkillAsset->TargetType = EForge2TargetType::AreaEnemy;
            else if (TargetType == TEXT("area_ally"))
                SkillAsset->TargetType = EForge2TargetType::AreaAlly;
            else if (TargetType == TEXT("area_all"))
                SkillAsset->TargetType = EForge2TargetType::AreaAll;
            else if (TargetType == TEXT("projectile"))
                SkillAsset->TargetType = EForge2TargetType::Projectile;
            else
                SkillAsset->TargetType = EForge2TargetType::Self;

            // Create default ranks
            for (int32 i = 1; i <= MaxRank; i++)
            {
                FForge2SkillRank Rank;
                Rank.Rank = i;
                Rank.PointCost = i;
                Rank.LevelRequirement = i * 2;
                Rank.Value = 10.0f * i;
                Rank.Cooldown = FMath::Max(0.0f, 10.0f - (i - 1) * 0.5f);
                Rank.ManaCost = 10.0f + (i - 1) * 5.0f;
                SkillAsset->Ranks.Add(Rank);
            }

            // Combat settings
            SkillAsset->BaseRange = GetJsonNumberField(Params, TEXT("range"), 500.0f);
            SkillAsset->AreaRadius = GetJsonNumberField(Params, TEXT("area_radius"), 0.0f);
            SkillAsset->CastTime = GetJsonNumberField(Params, TEXT("cast_time"), 0.0f);
            SkillAsset->bCanCrit = GetJsonBoolField(Params, TEXT("can_crit"), true);

            // Tree position
            const TSharedPtr<FJsonObject>* PosObj;
            if (Params->TryGetObjectField(TEXT("tree_position"), PosObj))
            {
                double X = 0, Y = 0;
                (*PosObj)->TryGetNumberField(TEXT("x"), X);
                (*PosObj)->TryGetNumberField(TEXT("y"), Y);
                SkillAsset->TreePosition = FVector2D(X, Y);
            }

            SkillAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(SkillAsset);
            McpSafeAssetSave(SkillAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("skill_id"), SkillId);
            Result->SetStringField(TEXT("name"), Name);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetStringField(TEXT("type"), SkillType);
            Result->SetNumberField(TEXT("max_rank"), MaxRank);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Skill created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create skill asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // ========================================================================
    // configure_skill_rank - Configure specific rank data
    // ========================================================================
    if (SubAction == TEXT("configure_skill_rank"))
    {
        FString SkillPath = GetJsonStringField(Params, TEXT("skill_path"), TEXT(""));
        int32 Rank = GetJsonNumberField(Params, TEXT("rank"), 1);

        if (SkillPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing skill_path"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpSkillDefinitionAsset* SkillAsset = LoadObject<UMcpSkillDefinitionAsset>(nullptr, *SkillPath);
        if (!SkillAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Skill asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        // Find or create rank
        FForge2SkillRank* RankData = nullptr;
        for (FForge2SkillRank& R : SkillAsset->Ranks)
        {
            if (R.Rank == Rank)
            {
                RankData = &R;
                break;
            }
        }

        if (!RankData)
        {
            FForge2SkillRank NewRank;
            NewRank.Rank = Rank;
            SkillAsset->Ranks.Add(NewRank);
            RankData = &SkillAsset->Ranks.Last();
        }

        // Update rank data
        RankData->PointCost = GetJsonNumberField(Params, TEXT("point_cost"), RankData->PointCost);
        RankData->LevelRequirement = GetJsonNumberField(Params, TEXT("level_requirement"), RankData->LevelRequirement);
        RankData->Value = GetJsonNumberField(Params, TEXT("value"), RankData->Value);
        RankData->Cooldown = GetJsonNumberField(Params, TEXT("cooldown"), RankData->Cooldown);
        RankData->ManaCost = GetJsonNumberField(Params, TEXT("mana_cost"), RankData->ManaCost);

        FString RankDesc = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        if (!RankDesc.IsEmpty())
        {
            RankData->RankDescription = FText::FromString(RankDesc);
        }

        SkillAsset->MarkPackageDirty();
        McpSafeAssetSave(SkillAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skill_path"), SkillPath);
        Result->SetNumberField(TEXT("rank"), Rank);
        Result->SetNumberField(TEXT("value"), RankData->Value);
        Result->SetNumberField(TEXT("cooldown"), RankData->Cooldown);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Skill rank configured"), Result);
        return true;
    }

    // ========================================================================
    // add_skill_prerequisite - Add prerequisite to skill
    // ========================================================================
    if (SubAction == TEXT("add_skill_prerequisite"))
    {
        FString SkillPath = GetJsonStringField(Params, TEXT("skill_path"), TEXT(""));
        FString RequiredSkillId = GetJsonStringField(Params, TEXT("required_skill"), TEXT(""));
        int32 RequiredRank = GetJsonNumberField(Params, TEXT("required_rank"), 1);

        if (SkillPath.IsEmpty() || RequiredSkillId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpSkillDefinitionAsset* SkillAsset = LoadObject<UMcpSkillDefinitionAsset>(nullptr, *SkillPath);
        if (!SkillAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Skill asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        FForge2SkillRequirement Req;
        Req.SkillId = FName(*RequiredSkillId);
        Req.RequiredRank = RequiredRank;
        SkillAsset->RequiredSkills.Add(Req);

        SkillAsset->MarkPackageDirty();
        McpSafeAssetSave(SkillAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skill_path"), SkillPath);
        Result->SetStringField(TEXT("required_skill"), RequiredSkillId);
        Result->SetNumberField(TEXT("required_rank"), RequiredRank);
        Result->SetNumberField(TEXT("total_prerequisites"), SkillAsset->RequiredSkills.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Prerequisite added"), Result);
        return true;
    }

    // ========================================================================
    // connect_skills - Connect skills in tree
    // ========================================================================
    if (SubAction == TEXT("connect_skills"))
    {
        FString SkillPath = GetJsonStringField(Params, TEXT("skill_path"), TEXT(""));
        FString ConnectedSkillId = GetJsonStringField(Params, TEXT("connected_skill"), TEXT(""));

        if (SkillPath.IsEmpty() || ConnectedSkillId.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpSkillDefinitionAsset* SkillAsset = LoadObject<UMcpSkillDefinitionAsset>(nullptr, *SkillPath);
        if (!SkillAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Skill asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        SkillAsset->ConnectedSkills.AddUnique(FName(*ConnectedSkillId));
        SkillAsset->MarkPackageDirty();
        McpSafeAssetSave(SkillAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skill_path"), SkillPath);
        Result->SetStringField(TEXT("connected_skill"), ConnectedSkillId);
        Result->SetNumberField(TEXT("total_connections"), SkillAsset->ConnectedSkills.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Skills connected"), Result);
        return true;
    }

    // ========================================================================
    // add_damage_effect - Add damage effect to skill
    // ========================================================================
    if (SubAction == TEXT("add_damage_effect"))
    {
        FString SkillPath = GetJsonStringField(Params, TEXT("skill_path"), TEXT(""));
        FString DamageType = GetJsonStringField(Params, TEXT("damage_type"), TEXT("physical"));
        float MinDamage = GetJsonNumberField(Params, TEXT("min_damage"), 10.0f);
        float MaxDamage = GetJsonNumberField(Params, TEXT("max_damage"), 20.0f);

        if (SkillPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing skill_path"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpSkillDefinitionAsset* SkillAsset = LoadObject<UMcpSkillDefinitionAsset>(nullptr, *SkillPath);
        if (!SkillAsset)
        {
            SendAutomationError(Socket, RequestId, TEXT("Skill asset not found"), TEXT("NOT_FOUND"));
            return true;
        }

        FForge2DamageInfo DmgInfo;
        DmgInfo.MinDamage = MinDamage;
        DmgInfo.MaxDamage = MaxDamage;
        DmgInfo.CriticalChanceBonus = GetJsonNumberField(Params, TEXT("crit_chance"), 0.0f);
        DmgInfo.CriticalDamageBonus = GetJsonNumberField(Params, TEXT("crit_damage"), 50.0f);

        // Parse damage type
        if (DamageType == TEXT("fire"))
            DmgInfo.DamageType = EForge2DamageType::Fire;
        else if (DamageType == TEXT("ice"))
            DmgInfo.DamageType = EForge2DamageType::Ice;
        else if (DamageType == TEXT("lightning"))
            DmgInfo.DamageType = EForge2DamageType::Lightning;
        else if (DamageType == TEXT("poison"))
            DmgInfo.DamageType = EForge2DamageType::Poison;
        else if (DamageType == TEXT("holy"))
            DmgInfo.DamageType = EForge2DamageType::Holy;
        else if (DamageType == TEXT("dark"))
            DmgInfo.DamageType = EForge2DamageType::Dark;
        else if (DamageType == TEXT("arcane"))
            DmgInfo.DamageType = EForge2DamageType::Arcane;
        else
            DmgInfo.DamageType = EForge2DamageType::Physical;

        SkillAsset->DamageEffects.Add(DmgInfo);
        SkillAsset->MarkPackageDirty();
        McpSafeAssetSave(SkillAsset);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("skill_path"), SkillPath);
        Result->SetStringField(TEXT("damage_type"), DamageType);
        Result->SetNumberField(TEXT("min_damage"), MinDamage);
        Result->SetNumberField(TEXT("max_damage"), MaxDamage);
        Result->SetNumberField(TEXT("total_damage_effects"), SkillAsset->DamageEffects.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Damage effect added"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown skill_tree action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}

// ============================================================================
// FORGE2 INVENTORY HANDLER - Full Implementation
// ============================================================================

bool UMcpAutomationBridgeSubsystem::HandleForge2InventoryAction(
    const FString& RequestId, const FString& Action,
    const TSharedPtr<FJsonObject>& Payload,
    TSharedPtr<FMcpBridgeWebSocket> Socket)
{
#if !WITH_EDITOR
    SendAutomationError(Socket, RequestId, TEXT("FORGE2 inventory requires editor"), TEXT("EDITOR_ONLY"));
    return true;
#else
    if (!Payload.IsValid())
    {
        SendAutomationError(Socket, RequestId, TEXT("Missing payload"), TEXT("INVALID_PAYLOAD"));
        return true;
    }

    FString SubAction = Forge2GameplayHelpers::GetActionFromPayload(Payload);
    TSharedPtr<FJsonObject> Params = Forge2GameplayHelpers::GetParams(Payload);

    // ========================================================================
    // create_item - Create generic item
    // ========================================================================
    if (SubAction == TEXT("create_item"))
    {
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("NewItem"));
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), Name);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Items"));
        FString Description = GetJsonStringField(Params, TEXT("description"), TEXT(""));
        FString ItemType = GetJsonStringField(Params, TEXT("type"), TEXT("misc"));
        FString Rarity = GetJsonStringField(Params, TEXT("rarity"), TEXT("common"));
        int32 MaxStack = GetJsonNumberField(Params, TEXT("max_stack"), 1);
        float Weight = GetJsonNumberField(Params, TEXT("weight"), 0.0f);
        int64 Value = static_cast<int64>(GetJsonNumberField(Params, TEXT("value"), 0));

        UPackage* Package = Forge2GameplayHelpers::CreateAssetPackage(Path, Name);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpItemDefinitionAsset* ItemAsset = NewObject<UMcpItemDefinitionAsset>(
            Package, FName(*Name), RF_Public | RF_Standalone);

        if (ItemAsset)
        {
            ItemAsset->AssetId = FName(*ItemId);
            ItemAsset->DisplayName = FText::FromString(Name);
            ItemAsset->Description = FText::FromString(Description);
            ItemAsset->MaxStackSize = MaxStack;
            ItemAsset->Weight = Weight;
            ItemAsset->BaseValue = Value;

            // Parse item type
            if (ItemType == TEXT("weapon"))
                ItemAsset->ItemType = EForge2ItemType::Weapon;
            else if (ItemType == TEXT("armor"))
                ItemAsset->ItemType = EForge2ItemType::Armor;
            else if (ItemType == TEXT("accessory"))
                ItemAsset->ItemType = EForge2ItemType::Accessory;
            else if (ItemType == TEXT("consumable"))
                ItemAsset->ItemType = EForge2ItemType::Consumable;
            else if (ItemType == TEXT("material"))
                ItemAsset->ItemType = EForge2ItemType::Material;
            else if (ItemType == TEXT("quest"))
                ItemAsset->ItemType = EForge2ItemType::QuestItem;
            else
                ItemAsset->ItemType = EForge2ItemType::Misc;

            // Parse rarity
            if (Rarity == TEXT("uncommon"))
                ItemAsset->Rarity = EForge2Rarity::Uncommon;
            else if (Rarity == TEXT("rare"))
                ItemAsset->Rarity = EForge2Rarity::Rare;
            else if (Rarity == TEXT("epic"))
                ItemAsset->Rarity = EForge2Rarity::Epic;
            else if (Rarity == TEXT("legendary"))
                ItemAsset->Rarity = EForge2Rarity::Legendary;
            else if (Rarity == TEXT("mythic"))
                ItemAsset->Rarity = EForge2Rarity::Mythic;
            else if (Rarity == TEXT("unique"))
                ItemAsset->Rarity = EForge2Rarity::Unique;
            else
                ItemAsset->Rarity = EForge2Rarity::Common;

            // Level requirement
            ItemAsset->LevelRequirement = GetJsonNumberField(Params, TEXT("level_requirement"), 0);

            // Binding
            FString BoundType = GetJsonStringField(Params, TEXT("bound_type"), TEXT("none"));
            if (BoundType == TEXT("pickup"))
                ItemAsset->BoundType = EForge2BoundType::OnPickup;
            else if (BoundType == TEXT("equip"))
                ItemAsset->BoundType = EForge2BoundType::OnEquip;
            else if (BoundType == TEXT("use"))
                ItemAsset->BoundType = EForge2BoundType::OnUse;
            else if (BoundType == TEXT("account"))
                ItemAsset->BoundType = EForge2BoundType::Account;
            else
                ItemAsset->BoundType = EForge2BoundType::None;

            // Visuals
            FString IconPath = GetJsonStringField(Params, TEXT("icon"), TEXT(""));
            if (!IconPath.IsEmpty())
            {
                ItemAsset->Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconPath));
            }

            FString MeshPath = GetJsonStringField(Params, TEXT("mesh"), TEXT(""));
            if (!MeshPath.IsEmpty())
            {
                ItemAsset->WorldMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(MeshPath));
            }

            // Sockets
            ItemAsset->SocketCount = GetJsonNumberField(Params, TEXT("socket_count"), 0);

            // Flavor text
            FString FlavorText = GetJsonStringField(Params, TEXT("flavor_text"), TEXT(""));
            if (!FlavorText.IsEmpty())
            {
                ItemAsset->FlavorText = FText::FromString(FlavorText);
            }

            ItemAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(ItemAsset);
            McpSafeAssetSave(ItemAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("item_id"), ItemId);
            Result->SetStringField(TEXT("name"), Name);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetStringField(TEXT("type"), ItemType);
            Result->SetStringField(TEXT("rarity"), Rarity);
            Result->SetNumberField(TEXT("value"), Value);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Item created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create item asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // ========================================================================
    // create_weapon - Create weapon item
    // ========================================================================
    if (SubAction == TEXT("create_weapon"))
    {
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("NewWeapon"));
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), Name);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Items/Weapons"));
        FString WeaponType = GetJsonStringField(Params, TEXT("weapon_type"), TEXT("sword"));
        FString DamageType = GetJsonStringField(Params, TEXT("damage_type"), TEXT("physical"));
        float MinDamage = GetJsonNumberField(Params, TEXT("min_damage"), 10.0f);
        float MaxDamage = GetJsonNumberField(Params, TEXT("max_damage"), 20.0f);
        float AttackSpeed = GetJsonNumberField(Params, TEXT("attack_speed"), 1.0f);
        float Range = GetJsonNumberField(Params, TEXT("range"), 200.0f);
        bool bTwoHanded = GetJsonBoolField(Params, TEXT("two_handed"), false);

        UPackage* Package = Forge2GameplayHelpers::CreateAssetPackage(Path, Name);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpWeaponDefinitionAsset* WeaponAsset = NewObject<UMcpWeaponDefinitionAsset>(
            Package, FName(*Name), RF_Public | RF_Standalone);

        if (WeaponAsset)
        {
            WeaponAsset->AssetId = FName(*ItemId);
            WeaponAsset->DisplayName = FText::FromString(Name);
            WeaponAsset->ItemType = EForge2ItemType::Weapon;
            WeaponAsset->bTwoHanded = bTwoHanded;
            WeaponAsset->AttackSpeed = AttackSpeed;
            WeaponAsset->Range = Range;
            WeaponAsset->AttackArc = GetJsonNumberField(Params, TEXT("attack_arc"), 90.0f);

            // Parse weapon type
            if (WeaponType == TEXT("greatsword"))
                WeaponAsset->WeaponType = EForge2WeaponType::GreatSword;
            else if (WeaponType == TEXT("axe"))
                WeaponAsset->WeaponType = EForge2WeaponType::Axe;
            else if (WeaponType == TEXT("greataxe"))
                WeaponAsset->WeaponType = EForge2WeaponType::GreatAxe;
            else if (WeaponType == TEXT("mace"))
                WeaponAsset->WeaponType = EForge2WeaponType::Mace;
            else if (WeaponType == TEXT("hammer"))
                WeaponAsset->WeaponType = EForge2WeaponType::Hammer;
            else if (WeaponType == TEXT("dagger"))
                WeaponAsset->WeaponType = EForge2WeaponType::Dagger;
            else if (WeaponType == TEXT("spear"))
                WeaponAsset->WeaponType = EForge2WeaponType::Spear;
            else if (WeaponType == TEXT("staff"))
                WeaponAsset->WeaponType = EForge2WeaponType::Staff;
            else if (WeaponType == TEXT("wand"))
                WeaponAsset->WeaponType = EForge2WeaponType::Wand;
            else if (WeaponType == TEXT("bow"))
                WeaponAsset->WeaponType = EForge2WeaponType::Bow;
            else if (WeaponType == TEXT("crossbow"))
                WeaponAsset->WeaponType = EForge2WeaponType::Crossbow;
            else if (WeaponType == TEXT("gun"))
                WeaponAsset->WeaponType = EForge2WeaponType::Gun;
            else if (WeaponType == TEXT("rifle"))
                WeaponAsset->WeaponType = EForge2WeaponType::Rifle;
            else if (WeaponType == TEXT("shield"))
                WeaponAsset->WeaponType = EForge2WeaponType::Shield;
            else if (WeaponType == TEXT("fist"))
                WeaponAsset->WeaponType = EForge2WeaponType::Fist;
            else
                WeaponAsset->WeaponType = EForge2WeaponType::Sword;

            // Base damage
            WeaponAsset->BaseDamage.MinDamage = MinDamage;
            WeaponAsset->BaseDamage.MaxDamage = MaxDamage;
            WeaponAsset->BaseDamage.CriticalChanceBonus = GetJsonNumberField(Params, TEXT("crit_chance"), 0.0f);
            WeaponAsset->BaseDamage.CriticalDamageBonus = GetJsonNumberField(Params, TEXT("crit_damage"), 50.0f);

            // Parse damage type
            if (DamageType == TEXT("fire"))
                WeaponAsset->BaseDamage.DamageType = EForge2DamageType::Fire;
            else if (DamageType == TEXT("ice"))
                WeaponAsset->BaseDamage.DamageType = EForge2DamageType::Ice;
            else if (DamageType == TEXT("lightning"))
                WeaponAsset->BaseDamage.DamageType = EForge2DamageType::Lightning;
            else
                WeaponAsset->BaseDamage.DamageType = EForge2DamageType::Physical;

            // Rarity
            FString Rarity = GetJsonStringField(Params, TEXT("rarity"), TEXT("common"));
            if (Rarity == TEXT("rare"))
                WeaponAsset->Rarity = EForge2Rarity::Rare;
            else if (Rarity == TEXT("epic"))
                WeaponAsset->Rarity = EForge2Rarity::Epic;
            else if (Rarity == TEXT("legendary"))
                WeaponAsset->Rarity = EForge2Rarity::Legendary;
            else
                WeaponAsset->Rarity = EForge2Rarity::Common;

            WeaponAsset->BaseValue = static_cast<int64>(GetJsonNumberField(Params, TEXT("value"), 100));
            WeaponAsset->LevelRequirement = GetJsonNumberField(Params, TEXT("level_requirement"), 1);

            WeaponAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(WeaponAsset);
            McpSafeAssetSave(WeaponAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("item_id"), ItemId);
            Result->SetStringField(TEXT("name"), Name);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetStringField(TEXT("weapon_type"), WeaponType);
            Result->SetNumberField(TEXT("dps"), WeaponAsset->GetDPS());

            SendAutomationResponse(Socket, RequestId, true, TEXT("Weapon created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create weapon asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // ========================================================================
    // create_armor - Create armor item
    // ========================================================================
    if (SubAction == TEXT("create_armor"))
    {
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("NewArmor"));
        FString ItemId = GetJsonStringField(Params, TEXT("item_id"), Name);
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Items/Armor"));
        FString SlotType = GetJsonStringField(Params, TEXT("slot"), TEXT("chest"));
        FString ArmorType = GetJsonStringField(Params, TEXT("armor_type"), TEXT("plate"));
        float Armor = GetJsonNumberField(Params, TEXT("armor"), 50.0f);
        float MagicResist = GetJsonNumberField(Params, TEXT("magic_resist"), 0.0f);

        UPackage* Package = Forge2GameplayHelpers::CreateAssetPackage(Path, Name);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpArmorDefinitionAsset* ArmorAsset = NewObject<UMcpArmorDefinitionAsset>(
            Package, FName(*Name), RF_Public | RF_Standalone);

        if (ArmorAsset)
        {
            ArmorAsset->AssetId = FName(*ItemId);
            ArmorAsset->DisplayName = FText::FromString(Name);
            ArmorAsset->ItemType = EForge2ItemType::Armor;
            ArmorAsset->BaseArmor = Armor;
            ArmorAsset->BaseMagicResist = MagicResist;

            // Parse slot
            if (SlotType == TEXT("head"))
                ArmorAsset->Slot = EForge2ArmorSlot::Head;
            else if (SlotType == TEXT("hands"))
                ArmorAsset->Slot = EForge2ArmorSlot::Hands;
            else if (SlotType == TEXT("legs"))
                ArmorAsset->Slot = EForge2ArmorSlot::Legs;
            else if (SlotType == TEXT("feet"))
                ArmorAsset->Slot = EForge2ArmorSlot::Feet;
            else if (SlotType == TEXT("shoulders"))
                ArmorAsset->Slot = EForge2ArmorSlot::Shoulders;
            else if (SlotType == TEXT("back"))
                ArmorAsset->Slot = EForge2ArmorSlot::Back;
            else
                ArmorAsset->Slot = EForge2ArmorSlot::Chest;

            // Parse armor type
            if (ArmorType == TEXT("cloth"))
                ArmorAsset->ArmorType = EForge2ArmorType::Cloth;
            else if (ArmorType == TEXT("leather"))
                ArmorAsset->ArmorType = EForge2ArmorType::Leather;
            else if (ArmorType == TEXT("mail"))
                ArmorAsset->ArmorType = EForge2ArmorType::Mail;
            else
                ArmorAsset->ArmorType = EForge2ArmorType::Plate;

            // Rarity and value
            FString Rarity = GetJsonStringField(Params, TEXT("rarity"), TEXT("common"));
            if (Rarity == TEXT("rare"))
                ArmorAsset->Rarity = EForge2Rarity::Rare;
            else if (Rarity == TEXT("epic"))
                ArmorAsset->Rarity = EForge2Rarity::Epic;
            else
                ArmorAsset->Rarity = EForge2Rarity::Common;

            ArmorAsset->BaseValue = static_cast<int64>(GetJsonNumberField(Params, TEXT("value"), 100));
            ArmorAsset->LevelRequirement = GetJsonNumberField(Params, TEXT("level_requirement"), 1);

            ArmorAsset->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(ArmorAsset);
            McpSafeAssetSave(ArmorAsset);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("item_id"), ItemId);
            Result->SetStringField(TEXT("name"), Name);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetStringField(TEXT("slot"), SlotType);
            Result->SetNumberField(TEXT("armor"), Armor);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Armor created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create armor asset"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // ========================================================================
    // create_loot_table - Create loot table
    // ========================================================================
    if (SubAction == TEXT("create_loot_table"))
    {
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("NewLootTable"));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/LootTables"));
        int32 MinDrops = GetJsonNumberField(Params, TEXT("min_drops"), 1);
        int32 MaxDrops = GetJsonNumberField(Params, TEXT("max_drops"), 3);
        float NothingChance = GetJsonNumberField(Params, TEXT("nothing_chance"), 0.0f);

        UPackage* Package = Forge2GameplayHelpers::CreateAssetPackage(Path, Name);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpLootTableAsset* LootTable = NewObject<UMcpLootTableAsset>(
            Package, FName(*Name), RF_Public | RF_Standalone);

        if (LootTable)
        {
            LootTable->AssetId = FName(*Name);
            LootTable->DisplayName = FText::FromString(Name);
            LootTable->MinDrops = MinDrops;
            LootTable->MaxDrops = MaxDrops;
            LootTable->NothingChance = NothingChance;
            LootTable->bScaleWithLevel = GetJsonBoolField(Params, TEXT("scale_with_level"), false);
            LootTable->BaseLevel = GetJsonNumberField(Params, TEXT("base_level"), 1);

            LootTable->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(LootTable);
            McpSafeAssetSave(LootTable);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("name"), Name);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetNumberField(TEXT("min_drops"), MinDrops);
            Result->SetNumberField(TEXT("max_drops"), MaxDrops);

            SendAutomationResponse(Socket, RequestId, true, TEXT("Loot table created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create loot table"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // ========================================================================
    // add_loot_entry - Add entry to loot table
    // ========================================================================
    if (SubAction == TEXT("add_loot_entry"))
    {
        FString LootTablePath = GetJsonStringField(Params, TEXT("loot_table_path"), TEXT(""));
        FString ItemPath = GetJsonStringField(Params, TEXT("item_path"), TEXT(""));
        float Weight = GetJsonNumberField(Params, TEXT("weight"), 100.0f);
        int32 MinQty = GetJsonNumberField(Params, TEXT("min_quantity"), 1);
        int32 MaxQty = GetJsonNumberField(Params, TEXT("max_quantity"), 1);
        bool bGuaranteed = GetJsonBoolField(Params, TEXT("guaranteed"), false);

        if (LootTablePath.IsEmpty() || ItemPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing required parameters"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpLootTableAsset* LootTable = LoadObject<UMcpLootTableAsset>(nullptr, *LootTablePath);
        if (!LootTable)
        {
            SendAutomationError(Socket, RequestId, TEXT("Loot table not found"), TEXT("NOT_FOUND"));
            return true;
        }

        FForge2LootEntry Entry;
        Entry.Item = TSoftObjectPtr<UMcpItemDefinitionAsset>(FSoftObjectPath(ItemPath));
        Entry.Weight = Weight;
        Entry.MinQuantity = MinQty;
        Entry.MaxQuantity = MaxQty;
        Entry.bGuaranteed = bGuaranteed;
        Entry.bUnique = GetJsonBoolField(Params, TEXT("unique"), false);

        LootTable->Entries.Add(Entry);
        LootTable->MarkPackageDirty();
        McpSafeAssetSave(LootTable);

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("loot_table"), LootTablePath);
        Result->SetStringField(TEXT("item"), ItemPath);
        Result->SetNumberField(TEXT("weight"), Weight);
        Result->SetNumberField(TEXT("total_entries"), LootTable->Entries.Num());

        SendAutomationResponse(Socket, RequestId, true, TEXT("Loot entry added"), Result);
        return true;
    }

    // ========================================================================
    // create_crafting_recipe - Create crafting recipe
    // ========================================================================
    if (SubAction == TEXT("create_crafting_recipe"))
    {
        FString Name = GetJsonStringField(Params, TEXT("name"), TEXT("NewRecipe"));
        FString Path = GetJsonStringField(Params, TEXT("path"), TEXT("/Game/FORGE2/Recipes"));
        FString OutputItemPath = GetJsonStringField(Params, TEXT("output_item"), TEXT(""));
        int32 OutputQty = GetJsonNumberField(Params, TEXT("output_quantity"), 1);
        float CraftingTime = GetJsonNumberField(Params, TEXT("crafting_time"), 0.0f);

        if (OutputItemPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing output_item"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UPackage* Package = Forge2GameplayHelpers::CreateAssetPackage(Path, Name);
        if (!Package)
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create package"), TEXT("PACKAGE_FAILED"));
            return true;
        }

        UMcpCraftingRecipeAsset* Recipe = NewObject<UMcpCraftingRecipeAsset>(
            Package, FName(*Name), RF_Public | RF_Standalone);

        if (Recipe)
        {
            Recipe->AssetId = FName(*Name);
            Recipe->DisplayName = FText::FromString(Name);
            Recipe->OutputItem = TSoftObjectPtr<UMcpItemDefinitionAsset>(FSoftObjectPath(OutputItemPath));
            Recipe->OutputQuantity = OutputQty;
            Recipe->CraftingTime = CraftingTime;
            Recipe->RequiredStation = FName(*GetJsonStringField(Params, TEXT("required_station"), TEXT("")));
            Recipe->RequiredSkill = FName(*GetJsonStringField(Params, TEXT("required_skill"), TEXT("")));
            Recipe->RequiredSkillLevel = GetJsonNumberField(Params, TEXT("required_skill_level"), 0);
            Recipe->RequiredPlayerLevel = GetJsonNumberField(Params, TEXT("required_level"), 0);
            Recipe->SkillXpGain = GetJsonNumberField(Params, TEXT("skill_xp"), 0);
            Recipe->bDiscoverable = GetJsonBoolField(Params, TEXT("discoverable"), false);
            Recipe->RecipeCategory = FName(*GetJsonStringField(Params, TEXT("category"), TEXT("")));

            // Parse ingredients
            const TArray<TSharedPtr<FJsonValue>>* IngredientsArray;
            if (Params->TryGetArrayField(TEXT("ingredients"), IngredientsArray))
            {
                for (const auto& IngVal : *IngredientsArray)
                {
                    const TSharedPtr<FJsonObject>* IngObj;
                    if (IngVal->TryGetObject(IngObj))
                    {
                        FForge2CraftingIngredient Ingredient;
                        FString IngItemPath = GetJsonStringField(*IngObj, TEXT("item"), TEXT(""));
                        if (!IngItemPath.IsEmpty())
                        {
                            Ingredient.Item = TSoftObjectPtr<UMcpItemDefinitionAsset>(FSoftObjectPath(IngItemPath));
                            Ingredient.Quantity = GetJsonNumberField(*IngObj, TEXT("quantity"), 1);
                            Ingredient.bConsumed = GetJsonBoolField(*IngObj, TEXT("consumed"), true);
                            Recipe->Ingredients.Add(Ingredient);
                        }
                    }
                }
            }

            Recipe->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(Recipe);
            McpSafeAssetSave(Recipe);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("name"), Name);
            Result->SetStringField(TEXT("path"), Package->GetName());
            Result->SetStringField(TEXT("output"), OutputItemPath);
            Result->SetNumberField(TEXT("ingredient_count"), Recipe->Ingredients.Num());

            SendAutomationResponse(Socket, RequestId, true, TEXT("Recipe created"), Result);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Failed to create recipe"), TEXT("CREATION_FAILED"));
        }
        return true;
    }

    // ========================================================================
    // add_stat_bonus - Add stat bonus to item
    // ========================================================================
    if (SubAction == TEXT("add_stat_bonus"))
    {
        FString ItemPath = GetJsonStringField(Params, TEXT("item_path"), TEXT(""));
        FString StatType = GetJsonStringField(Params, TEXT("stat"), TEXT("strength"));
        float FlatBonus = GetJsonNumberField(Params, TEXT("flat_bonus"), 0.0f);
        float PercentBonus = GetJsonNumberField(Params, TEXT("percent_bonus"), 0.0f);

        if (ItemPath.IsEmpty())
        {
            SendAutomationError(Socket, RequestId, TEXT("Missing item_path"), TEXT("MISSING_PARAMETER"));
            return true;
        }

        UMcpWeaponDefinitionAsset* Weapon = LoadObject<UMcpWeaponDefinitionAsset>(nullptr, *ItemPath);
        UMcpArmorDefinitionAsset* Armor = LoadObject<UMcpArmorDefinitionAsset>(nullptr, *ItemPath);

        FForge2StatModifier Modifier;
        Modifier.FlatBonus = FlatBonus;
        Modifier.PercentBonus = PercentBonus;

        // Parse stat type
        if (StatType == TEXT("dexterity"))
            Modifier.StatType = EForge2StatType::Dexterity;
        else if (StatType == TEXT("intelligence"))
            Modifier.StatType = EForge2StatType::Intelligence;
        else if (StatType == TEXT("vitality"))
            Modifier.StatType = EForge2StatType::Vitality;
        else if (StatType == TEXT("max_health"))
            Modifier.StatType = EForge2StatType::MaxHealth;
        else if (StatType == TEXT("crit_chance"))
            Modifier.StatType = EForge2StatType::CritChance;
        else if (StatType == TEXT("crit_damage"))
            Modifier.StatType = EForge2StatType::CritDamage;
        else if (StatType == TEXT("attack_speed"))
            Modifier.StatType = EForge2StatType::AttackSpeed;
        else
            Modifier.StatType = EForge2StatType::Strength;

        if (Weapon)
        {
            Weapon->StatBonuses.Add(Modifier);
            Weapon->MarkPackageDirty();
            McpSafeAssetSave(Weapon);
        }
        else if (Armor)
        {
            Armor->StatBonuses.Add(Modifier);
            Armor->MarkPackageDirty();
            McpSafeAssetSave(Armor);
        }
        else
        {
            SendAutomationError(Socket, RequestId, TEXT("Item not found or wrong type"), TEXT("NOT_FOUND"));
            return true;
        }

        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        Result->SetStringField(TEXT("item"), ItemPath);
        Result->SetStringField(TEXT("stat"), StatType);
        Result->SetNumberField(TEXT("flat_bonus"), FlatBonus);
        Result->SetNumberField(TEXT("percent_bonus"), PercentBonus);

        SendAutomationResponse(Socket, RequestId, true, TEXT("Stat bonus added"), Result);
        return true;
    }

    SendAutomationError(Socket, RequestId,
        FString::Printf(TEXT("Unknown inventory action: %s"), *SubAction), TEXT("UNKNOWN_ACTION"));
    return true;
#endif
}
