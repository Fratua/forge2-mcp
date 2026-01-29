// Copyright 2025 FORGE2 Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetDriver.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Forge2MultiplayerSystem.generated.h"

// Forward declarations
class APlayerController;
class APlayerState;
class AGameModeBase;
class AGameStateBase;

/**
 * Replication condition types
 */
UENUM(BlueprintType)
enum class EForge2ReplicationCondition : uint8
{
	None UMETA(DisplayName = "None"),
	InitialOnly UMETA(DisplayName = "Initial Only"),
	OwnerOnly UMETA(DisplayName = "Owner Only"),
	SkipOwner UMETA(DisplayName = "Skip Owner"),
	SimulatedOnly UMETA(DisplayName = "Simulated Only"),
	AutonomousOnly UMETA(DisplayName = "Autonomous Only"),
	SimulatedOrPhysics UMETA(DisplayName = "Simulated Or Physics"),
	InitialOrOwner UMETA(DisplayName = "Initial Or Owner"),
	Custom UMETA(DisplayName = "Custom"),
	ReplayOrOwner UMETA(DisplayName = "Replay Or Owner"),
	ReplayOnly UMETA(DisplayName = "Replay Only"),
	SimulatedOnlyNoReplay UMETA(DisplayName = "Simulated Only No Replay"),
	SimulatedOrPhysicsNoReplay UMETA(DisplayName = "Simulated Or Physics No Replay"),
	SkipReplay UMETA(DisplayName = "Skip Replay"),
	Never UMETA(DisplayName = "Never")
};

/**
 * RPC execution types
 */
UENUM(BlueprintType)
enum class EForge2RPCType : uint8
{
	Server UMETA(DisplayName = "Server"),
	Client UMETA(DisplayName = "Client"),
	NetMulticast UMETA(DisplayName = "NetMulticast")
};

/**
 * RPC reliability
 */
UENUM(BlueprintType)
enum class EForge2RPCReliability : uint8
{
	Reliable UMETA(DisplayName = "Reliable"),
	Unreliable UMETA(DisplayName = "Unreliable")
};

/**
 * Session state
 */
UENUM(BlueprintType)
enum class EForge2SessionState : uint8
{
	NoSession UMETA(DisplayName = "No Session"),
	Creating UMETA(DisplayName = "Creating"),
	Pending UMETA(DisplayName = "Pending"),
	Starting UMETA(DisplayName = "Starting"),
	InProgress UMETA(DisplayName = "In Progress"),
	Ending UMETA(DisplayName = "Ending"),
	Ended UMETA(DisplayName = "Ended"),
	Destroying UMETA(DisplayName = "Destroying")
};

/**
 * Matchmaking state
 */
UENUM(BlueprintType)
enum class EForge2MatchmakingState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Searching UMETA(DisplayName = "Searching"),
	Found UMETA(DisplayName = "Found"),
	Joining UMETA(DisplayName = "Joining"),
	Failed UMETA(DisplayName = "Failed"),
	Cancelled UMETA(DisplayName = "Cancelled")
};

/**
 * Network role
 */
UENUM(BlueprintType)
enum class EForge2NetworkRole : uint8
{
	None UMETA(DisplayName = "None"),
	DedicatedServer UMETA(DisplayName = "Dedicated Server"),
	ListenServer UMETA(DisplayName = "Listen Server"),
	Client UMETA(DisplayName = "Client")
};

/**
 * Chat channel type
 */
UENUM(BlueprintType)
enum class EForge2ChatChannel : uint8
{
	Global UMETA(DisplayName = "Global"),
	Zone UMETA(DisplayName = "Zone"),
	Party UMETA(DisplayName = "Party"),
	Guild UMETA(DisplayName = "Guild"),
	Whisper UMETA(DisplayName = "Whisper"),
	Trade UMETA(DisplayName = "Trade"),
	System UMETA(DisplayName = "System")
};

/**
 * Guild rank
 */
UENUM(BlueprintType)
enum class EForge2GuildRank : uint8
{
	Member UMETA(DisplayName = "Member"),
	Veteran UMETA(DisplayName = "Veteran"),
	Officer UMETA(DisplayName = "Officer"),
	CoLeader UMETA(DisplayName = "Co-Leader"),
	Leader UMETA(DisplayName = "Leader")
};

/**
 * Party role
 */
UENUM(BlueprintType)
enum class EForge2PartyRole : uint8
{
	Member UMETA(DisplayName = "Member"),
	Assistant UMETA(DisplayName = "Assistant"),
	Leader UMETA(DisplayName = "Leader")
};

/**
 * Social relationship type
 */
UENUM(BlueprintType)
enum class EForge2RelationshipType : uint8
{
	None UMETA(DisplayName = "None"),
	Friend UMETA(DisplayName = "Friend"),
	Blocked UMETA(DisplayName = "Blocked"),
	Pending UMETA(DisplayName = "Pending"),
	Requested UMETA(DisplayName = "Requested")
};

/**
 * Player presence state
 */
UENUM(BlueprintType)
enum class EForge2PresenceState : uint8
{
	Offline UMETA(DisplayName = "Offline"),
	Online UMETA(DisplayName = "Online"),
	Away UMETA(DisplayName = "Away"),
	Busy UMETA(DisplayName = "Busy"),
	InGame UMETA(DisplayName = "In Game"),
	InMenu UMETA(DisplayName = "In Menu")
};

/**
 * Replicated property settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ReplicatedProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	FName PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	EForge2ReplicationCondition Condition = EForge2ReplicationCondition::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	bool bNotifyOnChanged = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	FName NotifyFunctionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	bool bUseCustomDeltaSerialization = false;
};

/**
 * RPC definition
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2RPCDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPC")
	FName FunctionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPC")
	EForge2RPCType RPCType = EForge2RPCType::Server;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPC")
	EForge2RPCReliability Reliability = EForge2RPCReliability::Reliable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPC")
	bool bWithValidation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPC")
	FName ValidationFunctionName;
};

/**
 * Session settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SessionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString SessionName = TEXT("DefaultSession");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bIsLANMatch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bIsDedicated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bUsesPresence = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bAllowJoinInProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bAllowInvites = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bUsesStats = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bAntiCheatProtected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString GameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	TMap<FString, FString> CustomSettings;
};

/**
 * Matchmaking settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MatchmakingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	FString GameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	int32 MinPlayers = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	float SearchTimeoutSeconds = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	int32 SkillRating = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	int32 SkillRange = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	bool bExpandSkillOverTime = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	float SkillExpansionRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	TArray<FString> PreferredRegions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matchmaking")
	int32 MaxPing = 150;
};

/**
 * Search result
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SessionSearchResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	FString SessionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	FString SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	FString HostName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	int32 CurrentPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	int32 MaxPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	int32 Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	FString GameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	bool bIsLAN = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	TMap<FString, FString> CustomSettings;
};

/**
 * Player network info
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PlayerNetworkInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float PacketLoss = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bIsHost = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bIsLocalPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 ConnectionQuality = 100;
};

/**
 * Chat message
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2ChatMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString SenderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	EForge2ChatChannel Channel = EForge2ChatChannel::Global;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	bool bIsSystem = false;
};

/**
 * Guild definition
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GuildInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString GuildId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString GuildName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString GuildTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString LeaderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 MemberCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 MaxMembers = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FDateTime CreatedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString IconPath;
};

/**
 * Guild member
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2GuildMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	EForge2GuildRank Rank = EForge2GuildRank::Member;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FString Note;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FDateTime JoinDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	FDateTime LastOnline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guild")
	int32 ContributionPoints = 0;
};

/**
 * Party info
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PartyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString PartyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString LeaderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 MemberCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 MaxMembers = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	bool bIsPublic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString LootMode;
};

/**
 * Party member
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2PartyMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	EForge2PartyRole Role = EForge2PartyRole::Member;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString CharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	float HealthPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	bool bIsOnline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FVector Location;
};

/**
 * Friend info
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2FriendInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
	EForge2RelationshipType Relationship = EForge2RelationshipType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
	EForge2PresenceState Presence = EForge2PresenceState::Offline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
	FString CurrentActivity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
	FString CurrentLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
	FDateTime LastOnline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
	FString Note;
};

/**
 * Mail message
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2MailMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	FString MailId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	FString SenderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	FString SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	FString Subject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	FString Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	FDateTime SentDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	FDateTime ExpiryDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	bool bIsRead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	TArray<FString> AttachmentIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	int32 CurrencyAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	FString CurrencyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mail")
	bool bIsSystem = false;
};

/**
 * World shard info
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WorldShardInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString ShardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString ShardName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString Region;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	int32 CurrentPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	int32 MaxPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	float Load = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString Status;
};

/**
 * Instance info
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2InstanceInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString InstanceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString DungeonId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	int32 CurrentPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	int32 MaxPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString Difficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	bool bIsPrivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO")
	FString LeaderId;
};

/**
 * Trade offer
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TradeOffer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	FString TradeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	FString InitiatorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	FString TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	TArray<FString> InitiatorItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	TArray<FString> TargetItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 InitiatorCurrency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 TargetCurrency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bInitiatorConfirmed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bTargetConfirmed = false;
};

/**
 * Voice chat settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2VoiceChatSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	bool bPushToTalk = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	float InputVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	float OutputVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	float VoiceActivationThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	bool bPositionalAudio = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	float MaxVoiceDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	bool bEchoCancellation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	bool bNoiseSuppression = true;
};

/**
 * Network quality of service settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NetworkQoSSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	int32 TickRate = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	int32 NetUpdateFrequency = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	int32 MinNetUpdateFrequency = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	float NetPriority = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	float InterpolationDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	bool bUseAdaptiveNetFrequency = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	int32 MaxClientRate = 15000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	int32 MaxInternetClientRate = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	bool bEnableNetworkEmulation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	int32 EmulatedLatency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QoS")
	float EmulatedPacketLoss = 0.0f;
};

/**
 * Dedicated server settings
 */
USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2DedicatedServerSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	FString ServerName = TEXT("FORGE2 Server");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	int32 Port = 7777;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	int32 QueryPort = 27015;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	int32 MaxPlayers = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	FString AdminPassword;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	FString MOTD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	bool bLAN = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	FString DefaultMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	TArray<FString> MapRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	float IdleTimeout = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	bool bAutoRestart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server")
	float RestartInterval = 86400.0f;
};

/**
 * Multiplayer Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2MultiplayerManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2MultiplayerManager();

	// Initialization
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer")
	void InitializeMultiplayer();

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Session")
	bool CreateSession(const FForge2SessionSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Session")
	bool JoinSession(const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Session")
	bool LeaveSession();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Session")
	void SearchSessions(int32 MaxResults, TArray<FForge2SessionSearchResult>& OutResults);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Session")
	EForge2SessionState GetSessionState() const;

	// Matchmaking
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Matchmaking")
	bool StartMatchmaking(const FForge2MatchmakingSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Matchmaking")
	void CancelMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Matchmaking")
	EForge2MatchmakingState GetMatchmakingState() const;

	// Server Functions
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Server")
	bool StartDedicatedServer(const FForge2DedicatedServerSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Server")
	bool StartListenServer(const FForge2SessionSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Server")
	void ShutdownServer();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Server")
	EForge2NetworkRole GetNetworkRole() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Server")
	void KickPlayer(const FString& PlayerId, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Server")
	void BanPlayer(const FString& PlayerId, float DurationSeconds, const FString& Reason);

	// Replication
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Replication")
	void ConfigureActorReplication(AActor* Actor, bool bReplicates, bool bAlwaysRelevant = false);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Replication")
	void SetNetUpdateFrequency(AActor* Actor, float Frequency);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Replication")
	void SetNetPriority(AActor* Actor, float Priority);

	// Network Info
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Network")
	TArray<FForge2PlayerNetworkInfo> GetConnectedPlayers() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Network")
	FForge2PlayerNetworkInfo GetPlayerNetworkInfo(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Network")
	int32 GetLocalPing() const;

	// QoS
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|QoS")
	void ApplyQoSSettings(const FForge2NetworkQoSSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|QoS")
	void EnableNetworkEmulation(int32 LatencyMs, float PacketLossPercent);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|QoS")
	void DisableNetworkEmulation();

protected:
	UPROPERTY()
	EForge2SessionState CurrentSessionState = EForge2SessionState::NoSession;

	UPROPERTY()
	EForge2MatchmakingState CurrentMatchmakingState = EForge2MatchmakingState::Idle;

	UPROPERTY()
	EForge2NetworkRole CurrentNetworkRole = EForge2NetworkRole::None;

	UPROPERTY()
	FForge2NetworkQoSSettings CurrentQoSSettings;
};

/**
 * MMO Systems Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2MMOManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2MMOManager();

	// World Shards
	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Shards")
	TArray<FForge2WorldShardInfo> GetAvailableShards() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Shards")
	bool JoinShard(const FString& ShardId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Shards")
	FForge2WorldShardInfo GetCurrentShard() const;

	// Instances
	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Instances")
	bool CreateInstance(const FString& InstanceType, int32 MaxPlayers, bool bIsPrivate);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Instances")
	bool JoinInstance(const FString& InstanceId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Instances")
	void LeaveInstance();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Instances")
	FForge2InstanceInfo GetCurrentInstance() const;

	// Chat
	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Chat")
	void SendChatMessage(const FString& Message, EForge2ChatChannel Channel, const FString& TargetId = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Chat")
	TArray<FForge2ChatMessage> GetRecentMessages(EForge2ChatChannel Channel, int32 Count) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Chat")
	void JoinChatChannel(EForge2ChatChannel Channel);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Chat")
	void LeaveChatChannel(EForge2ChatChannel Channel);

	// Guilds
	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Guild")
	bool CreateGuild(const FString& GuildName, const FString& GuildTag);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Guild")
	bool JoinGuild(const FString& GuildId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Guild")
	void LeaveGuild();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Guild")
	FForge2GuildInfo GetCurrentGuild() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Guild")
	TArray<FForge2GuildMember> GetGuildMembers() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Guild")
	void SetMemberRank(const FString& MemberId, EForge2GuildRank NewRank);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Guild")
	void KickMember(const FString& MemberId);

	// Parties
	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Party")
	bool CreateParty();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Party")
	bool JoinParty(const FString& PartyId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Party")
	void LeaveParty();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Party")
	void InviteToParty(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Party")
	FForge2PartyInfo GetCurrentParty() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Party")
	TArray<FForge2PartyMember> GetPartyMembers() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Party")
	void PromoteToLeader(const FString& MemberId);

	// Mail
	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Mail")
	void SendMail(const FString& RecipientId, const FString& Subject, const FString& Body, const TArray<FString>& AttachmentIds, int32 CurrencyAmount);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Mail")
	TArray<FForge2MailMessage> GetInbox(bool bIncludeRead) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Mail")
	void ReadMail(const FString& MailId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Mail")
	void ClaimMailAttachments(const FString& MailId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|MMO|Mail")
	void DeleteMail(const FString& MailId);

protected:
	UPROPERTY()
	FForge2WorldShardInfo CurrentShard;

	UPROPERTY()
	FForge2InstanceInfo CurrentInstance;

	UPROPERTY()
	FForge2GuildInfo CurrentGuild;

	UPROPERTY()
	FForge2PartyInfo CurrentParty;

	UPROPERTY()
	TArray<FForge2ChatMessage> ChatHistory;
};

/**
 * Social Systems Manager Component
 */
UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2SocialManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UForge2SocialManager();

	// Friends
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Friends")
	void SendFriendRequest(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Friends")
	void AcceptFriendRequest(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Friends")
	void DeclineFriendRequest(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Friends")
	void RemoveFriend(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Friends")
	TArray<FForge2FriendInfo> GetFriendsList() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Friends")
	TArray<FForge2FriendInfo> GetPendingRequests() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Friends")
	TArray<FForge2FriendInfo> GetOnlineFriends() const;

	// Blocking
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Blocking")
	void BlockPlayer(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Blocking")
	void UnblockPlayer(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Blocking")
	TArray<FForge2FriendInfo> GetBlockedPlayers() const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Blocking")
	bool IsPlayerBlocked(const FString& PlayerId) const;

	// Presence
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Presence")
	void SetPresence(EForge2PresenceState State, const FString& Activity);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Presence")
	EForge2PresenceState GetPresence() const;

	// Trading
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Trade")
	bool InitiateTrade(const FString& TargetPlayerId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Trade")
	void AddItemToTrade(const FString& ItemId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Trade")
	void RemoveItemFromTrade(const FString& ItemId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Trade")
	void SetTradeCurrency(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Trade")
	void ConfirmTrade();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Trade")
	void CancelTrade();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Trade")
	FForge2TradeOffer GetCurrentTradeOffer() const;

	// Voice Chat
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Voice")
	void ConfigureVoiceChat(const FForge2VoiceChatSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Voice")
	void SetVoiceChatEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Voice")
	void MutePlayer(const FString& PlayerId, bool bMuted);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Voice")
	bool IsPlayerMuted(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Voice")
	void SetPlayerVolume(const FString& PlayerId, float Volume);

	// Emotes
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Emotes")
	void PlayEmote(const FString& EmoteId);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Social|Emotes")
	TArray<FString> GetUnlockedEmotes() const;

protected:
	UPROPERTY()
	TArray<FForge2FriendInfo> FriendsList;

	UPROPERTY()
	TArray<FString> BlockedPlayerIds;

	UPROPERTY()
	TArray<FString> MutedPlayerIds;

	UPROPERTY()
	EForge2PresenceState CurrentPresence = EForge2PresenceState::Online;

	UPROPERTY()
	FForge2TradeOffer CurrentTrade;

	UPROPERTY()
	FForge2VoiceChatSettings VoiceSettings;
};

/**
 * Utilities for multiplayer operations
 */
UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2MultiplayerUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Network helpers
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static bool IsServer(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static bool IsClient(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static bool IsDedicatedServer(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static bool IsListenServer(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static bool HasAuthority(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static bool IsLocallyControlled(const APawn* Pawn);

	// Connection helpers
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FString GetLocalPlayerId(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FString GetLocalPlayerName(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static int32 GetPlayerCount(const UObject* WorldContextObject);

	// Session helpers
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FString GenerateSessionId();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FString GenerateMatchmakingTicket();

	// Data validation
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static bool ValidatePlayerName(const FString& Name, FString& OutError);

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static bool ValidateChatMessage(const FString& Message, FString& OutError);

	// Presets
	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FForge2SessionSettings GetCoopPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FForge2SessionSettings GetPvPPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FForge2SessionSettings GetMMOPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FForge2MatchmakingSettings GetCasualMatchmakingPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FForge2MatchmakingSettings GetRankedMatchmakingPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FForge2NetworkQoSSettings GetLowLatencyQoSPreset();

	UFUNCTION(BlueprintCallable, Category = "FORGE2|Multiplayer|Utilities")
	static FForge2NetworkQoSSettings GetBandwidthSaverQoSPreset();
};
