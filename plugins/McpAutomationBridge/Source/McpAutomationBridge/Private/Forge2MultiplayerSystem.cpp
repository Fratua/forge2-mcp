// Copyright 2025 FORGE2 Project. All Rights Reserved.

#include "Forge2MultiplayerSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

// ============================================================================
// UForge2MultiplayerManager Implementation
// ============================================================================

UForge2MultiplayerManager::UForge2MultiplayerManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2MultiplayerManager::InitializeMultiplayer()
{
	// Initialize online subsystem and session interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Online subsystem initialized - %s"), *OnlineSub->GetSubsystemName().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FORGE2 Multiplayer: No online subsystem available"));
	}
}

bool UForge2MultiplayerManager::CreateSession(const FForge2SessionSettings& Settings)
{
	if (CurrentSessionState != EForge2SessionState::NoSession)
	{
		UE_LOG(LogTemp, Warning, TEXT("FORGE2 Multiplayer: Cannot create session while in state %d"), (int32)CurrentSessionState);
		return false;
	}

	CurrentSessionState = EForge2SessionState::Creating;

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		// Fallback to basic networking
		UWorld* World = GetWorld();
		if (World)
		{
			FString URL = FString::Printf(TEXT("%s?listen?MaxPlayers=%d"), *Settings.MapName, Settings.MaxPlayers);
			if (!Settings.GameMode.IsEmpty())
			{
				URL += FString::Printf(TEXT("?game=%s"), *Settings.GameMode);
			}

			World->ServerTravel(URL);
			CurrentSessionState = EForge2SessionState::InProgress;
			CurrentNetworkRole = EForge2NetworkRole::ListenServer;
			return true;
		}

		CurrentSessionState = EForge2SessionState::NoSession;
		return false;
	}

	IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		CurrentSessionState = EForge2SessionState::NoSession;
		return false;
	}

	// Create session settings
	FOnlineSessionSettings OnlineSettings;
	OnlineSettings.NumPublicConnections = Settings.MaxPlayers;
	OnlineSettings.NumPrivateConnections = 0;
	OnlineSettings.bIsLANMatch = Settings.bIsLANMatch;
	OnlineSettings.bIsDedicated = Settings.bIsDedicated;
	OnlineSettings.bUsesPresence = Settings.bUsesPresence;
	OnlineSettings.bAllowJoinInProgress = Settings.bAllowJoinInProgress;
	OnlineSettings.bAllowInvites = Settings.bAllowInvites;
	OnlineSettings.bUsesStats = Settings.bUsesStats;
	OnlineSettings.bAntiCheatProtected = Settings.bAntiCheatProtected;
	OnlineSettings.bShouldAdvertise = true;

	// Add custom settings
	OnlineSettings.Set(TEXT("SESSIONNAME"), Settings.SessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSettings.Set(TEXT("GAMEMODE"), Settings.GameMode, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSettings.Set(TEXT("MAPNAME"), Settings.MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	for (const TPair<FString, FString>& CustomSetting : Settings.CustomSettings)
	{
		OnlineSettings.Set(*CustomSetting.Key, CustomSetting.Value, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	}

	// Create session (async)
	CurrentNetworkRole = Settings.bIsDedicated ? EForge2NetworkRole::DedicatedServer : EForge2NetworkRole::ListenServer;
	CurrentSessionState = EForge2SessionState::Pending;

	return true;
}

bool UForge2MultiplayerManager::JoinSession(const FString& SessionId)
{
	if (CurrentSessionState != EForge2SessionState::NoSession)
	{
		UE_LOG(LogTemp, Warning, TEXT("FORGE2 Multiplayer: Cannot join session while in state %d"), (int32)CurrentSessionState);
		return false;
	}

	CurrentSessionState = EForge2SessionState::Starting;
	CurrentNetworkRole = EForge2NetworkRole::Client;

	// For basic networking, try direct connection
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC)
		{
			PC->ClientTravel(SessionId, ETravelType::TRAVEL_Absolute);
			CurrentSessionState = EForge2SessionState::InProgress;
			return true;
		}
	}

	CurrentSessionState = EForge2SessionState::NoSession;
	return false;
}

bool UForge2MultiplayerManager::LeaveSession()
{
	if (CurrentSessionState == EForge2SessionState::NoSession)
	{
		return false;
	}

	CurrentSessionState = EForge2SessionState::Ending;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC)
		{
			PC->ClientTravel(TEXT("/Game/Maps/MainMenu"), ETravelType::TRAVEL_Absolute);
		}
	}

	CurrentSessionState = EForge2SessionState::NoSession;
	CurrentNetworkRole = EForge2NetworkRole::None;
	return true;
}

void UForge2MultiplayerManager::SearchSessions(int32 MaxResults, TArray<FForge2SessionSearchResult>& OutResults)
{
	OutResults.Empty();

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("FORGE2 Multiplayer: No online subsystem for session search"));
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	// Session search would be async, this is a placeholder for the search setup
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Searching for sessions, max results: %d"), MaxResults);
}

EForge2SessionState UForge2MultiplayerManager::GetSessionState() const
{
	return CurrentSessionState;
}

bool UForge2MultiplayerManager::StartMatchmaking(const FForge2MatchmakingSettings& Settings)
{
	if (CurrentMatchmakingState != EForge2MatchmakingState::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("FORGE2 Multiplayer: Cannot start matchmaking while in state %d"), (int32)CurrentMatchmakingState);
		return false;
	}

	CurrentMatchmakingState = EForge2MatchmakingState::Searching;

	// Matchmaking implementation would go here
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Starting matchmaking for game mode: %s, skill: %d +/- %d"),
		*Settings.GameMode, Settings.SkillRating, Settings.SkillRange);

	return true;
}

void UForge2MultiplayerManager::CancelMatchmaking()
{
	if (CurrentMatchmakingState == EForge2MatchmakingState::Searching ||
		CurrentMatchmakingState == EForge2MatchmakingState::Found)
	{
		CurrentMatchmakingState = EForge2MatchmakingState::Cancelled;
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Matchmaking cancelled"));
	}

	CurrentMatchmakingState = EForge2MatchmakingState::Idle;
}

EForge2MatchmakingState UForge2MultiplayerManager::GetMatchmakingState() const
{
	return CurrentMatchmakingState;
}

bool UForge2MultiplayerManager::StartDedicatedServer(const FForge2DedicatedServerSettings& Settings)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Configure server settings
	if (GEngine)
	{
		GEngine->SetMaxTickRate(CurrentQoSSettings.TickRate, true);
	}

	// Build URL with settings
	FString URL = FString::Printf(TEXT("%s?listen?MaxPlayers=%d?Port=%d"),
		*Settings.DefaultMap, Settings.MaxPlayers, Settings.Port);

	if (!Settings.Password.IsEmpty())
	{
		URL += FString::Printf(TEXT("?password=%s"), *Settings.Password);
	}

	World->ServerTravel(URL);
	CurrentNetworkRole = EForge2NetworkRole::DedicatedServer;
	CurrentSessionState = EForge2SessionState::InProgress;

	UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Dedicated server started - %s on port %d"), *Settings.ServerName, Settings.Port);
	return true;
}

bool UForge2MultiplayerManager::StartListenServer(const FForge2SessionSettings& Settings)
{
	return CreateSession(Settings);
}

void UForge2MultiplayerManager::ShutdownServer()
{
	if (CurrentNetworkRole == EForge2NetworkRole::None)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		// Disconnect all clients
		AGameModeBase* GameMode = World->GetAuthGameMode();
		if (GameMode)
		{
			// Notify all players
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();
				if (PC && !PC->IsLocalController())
				{
					PC->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("Server shutting down")));
				}
			}
		}
	}

	CurrentSessionState = EForge2SessionState::NoSession;
	CurrentNetworkRole = EForge2NetworkRole::None;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Server shutdown complete"));
}

EForge2NetworkRole UForge2MultiplayerManager::GetNetworkRole() const
{
	return CurrentNetworkRole;
}

void UForge2MultiplayerManager::KickPlayer(const FString& PlayerId, const FString& Reason)
{
	UWorld* World = GetWorld();
	if (!World || CurrentNetworkRole == EForge2NetworkRole::Client)
	{
		return;
	}

	AGameModeBase* GameMode = World->GetAuthGameMode();
	if (GameMode)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC && PC->PlayerState)
			{
				FString CurrentPlayerId = PC->PlayerState->GetPlayerName(); // Would use UniqueId in production
				if (CurrentPlayerId == PlayerId)
				{
					GameMode->GameSession->KickPlayer(PC, FText::FromString(Reason));
					UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Kicked player %s - %s"), *PlayerId, *Reason);
					return;
				}
			}
		}
	}
}

void UForge2MultiplayerManager::BanPlayer(const FString& PlayerId, float DurationSeconds, const FString& Reason)
{
	// In production, this would add to a ban list (stored in database)
	KickPlayer(PlayerId, FString::Printf(TEXT("Banned: %s (Duration: %.0f seconds)"), *Reason, DurationSeconds));
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Banned player %s for %.0f seconds - %s"), *PlayerId, DurationSeconds, *Reason);
}

void UForge2MultiplayerManager::ConfigureActorReplication(AActor* Actor, bool bReplicates, bool bAlwaysRelevant)
{
	if (!Actor)
	{
		return;
	}

	Actor->SetReplicates(bReplicates);
	Actor->bAlwaysRelevant = bAlwaysRelevant;

	if (bReplicates)
	{
		Actor->SetReplicateMovement(true);
	}
}

void UForge2MultiplayerManager::SetNetUpdateFrequency(AActor* Actor, float Frequency)
{
	if (Actor)
	{
		Actor->NetUpdateFrequency = Frequency;
		Actor->MinNetUpdateFrequency = Frequency * 0.1f;
	}
}

void UForge2MultiplayerManager::SetNetPriority(AActor* Actor, float Priority)
{
	if (Actor)
	{
		Actor->NetPriority = Priority;
	}
}

TArray<FForge2PlayerNetworkInfo> UForge2MultiplayerManager::GetConnectedPlayers() const
{
	TArray<FForge2PlayerNetworkInfo> Players;

	UWorld* World = GetWorld();
	if (!World)
	{
		return Players;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->PlayerState)
		{
			FForge2PlayerNetworkInfo Info;
			Info.PlayerId = PC->PlayerState->GetPlayerName();
			Info.PlayerName = PC->PlayerState->GetPlayerName();
			Info.Ping = PC->PlayerState->GetPingInMilliseconds();
			Info.bIsHost = PC->HasAuthority();
			Info.bIsLocalPlayer = PC->IsLocalController();
			Info.ConnectionQuality = FMath::Clamp(100 - (Info.Ping / 2), 0, 100);
			Players.Add(Info);
		}
	}

	return Players;
}

FForge2PlayerNetworkInfo UForge2MultiplayerManager::GetPlayerNetworkInfo(const FString& PlayerId) const
{
	TArray<FForge2PlayerNetworkInfo> AllPlayers = GetConnectedPlayers();
	for (const FForge2PlayerNetworkInfo& Info : AllPlayers)
	{
		if (Info.PlayerId == PlayerId)
		{
			return Info;
		}
	}
	return FForge2PlayerNetworkInfo();
}

int32 UForge2MultiplayerManager::GetLocalPing() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC && PC->PlayerState)
		{
			return PC->PlayerState->GetPingInMilliseconds();
		}
	}
	return 0;
}

void UForge2MultiplayerManager::ApplyQoSSettings(const FForge2NetworkQoSSettings& Settings)
{
	CurrentQoSSettings = Settings;

	if (GEngine)
	{
		GEngine->SetMaxTickRate(Settings.TickRate, true);
	}

	UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Applied QoS settings - TickRate: %d, NetUpdate: %d"),
		Settings.TickRate, Settings.NetUpdateFrequency);
}

void UForge2MultiplayerManager::EnableNetworkEmulation(int32 LatencyMs, float PacketLossPercent)
{
#if !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	if (World)
	{
		UNetDriver* NetDriver = World->GetNetDriver();
		if (NetDriver)
		{
			// Network emulation settings
			// In production, would use console commands or packet simulator
			UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Network emulation enabled - Latency: %dms, PacketLoss: %.1f%%"),
				LatencyMs, PacketLossPercent);
		}
	}
#endif
}

void UForge2MultiplayerManager::DisableNetworkEmulation()
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Multiplayer: Network emulation disabled"));
#endif
}

// ============================================================================
// UForge2MMOManager Implementation
// ============================================================================

UForge2MMOManager::UForge2MMOManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<FForge2WorldShardInfo> UForge2MMOManager::GetAvailableShards() const
{
	TArray<FForge2WorldShardInfo> Shards;

	// In production, this would query a master server
	// For now, return example shards
	FForge2WorldShardInfo Shard1;
	Shard1.ShardId = TEXT("US-East-1");
	Shard1.ShardName = TEXT("Azeroth");
	Shard1.Region = TEXT("US-East");
	Shard1.CurrentPlayers = 1250;
	Shard1.MaxPlayers = 5000;
	Shard1.Load = 0.25f;
	Shard1.Status = TEXT("Online");
	Shards.Add(Shard1);

	FForge2WorldShardInfo Shard2;
	Shard2.ShardId = TEXT("EU-West-1");
	Shard2.ShardName = TEXT("Kalimdor");
	Shard2.Region = TEXT("EU-West");
	Shard2.CurrentPlayers = 3200;
	Shard2.MaxPlayers = 5000;
	Shard2.Load = 0.64f;
	Shard2.Status = TEXT("Online");
	Shards.Add(Shard2);

	return Shards;
}

bool UForge2MMOManager::JoinShard(const FString& ShardId)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Joining shard %s"), *ShardId);
	CurrentShard.ShardId = ShardId;
	return true;
}

FForge2WorldShardInfo UForge2MMOManager::GetCurrentShard() const
{
	return CurrentShard;
}

bool UForge2MMOManager::CreateInstance(const FString& InstanceType, int32 MaxPlayers, bool bIsPrivate)
{
	CurrentInstance.InstanceId = FGuid::NewGuid().ToString();
	CurrentInstance.InstanceType = InstanceType;
	CurrentInstance.MaxPlayers = MaxPlayers;
	CurrentInstance.bIsPrivate = bIsPrivate;
	CurrentInstance.CurrentPlayers = 1;

	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Created instance %s of type %s"), *CurrentInstance.InstanceId, *InstanceType);
	return true;
}

bool UForge2MMOManager::JoinInstance(const FString& InstanceId)
{
	CurrentInstance.InstanceId = InstanceId;
	CurrentInstance.CurrentPlayers++;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Joined instance %s"), *InstanceId);
	return true;
}

void UForge2MMOManager::LeaveInstance()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Left instance %s"), *CurrentInstance.InstanceId);
	CurrentInstance = FForge2InstanceInfo();
}

FForge2InstanceInfo UForge2MMOManager::GetCurrentInstance() const
{
	return CurrentInstance;
}

void UForge2MMOManager::SendChatMessage(const FString& Message, EForge2ChatChannel Channel, const FString& TargetId)
{
	FForge2ChatMessage ChatMsg;
	ChatMsg.SenderId = TEXT("LocalPlayer");
	ChatMsg.SenderName = TEXT("Player");
	ChatMsg.Message = Message;
	ChatMsg.Channel = Channel;
	ChatMsg.TargetId = TargetId;
	ChatMsg.Timestamp = FDateTime::Now();

	ChatHistory.Add(ChatMsg);

	// In production, this would be sent via RPC
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO Chat [%d]: %s"), (int32)Channel, *Message);
}

TArray<FForge2ChatMessage> UForge2MMOManager::GetRecentMessages(EForge2ChatChannel Channel, int32 Count) const
{
	TArray<FForge2ChatMessage> FilteredMessages;
	for (int32 i = ChatHistory.Num() - 1; i >= 0 && FilteredMessages.Num() < Count; --i)
	{
		if (ChatHistory[i].Channel == Channel)
		{
			FilteredMessages.Add(ChatHistory[i]);
		}
	}
	return FilteredMessages;
}

void UForge2MMOManager::JoinChatChannel(EForge2ChatChannel Channel)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Joined chat channel %d"), (int32)Channel);
}

void UForge2MMOManager::LeaveChatChannel(EForge2ChatChannel Channel)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Left chat channel %d"), (int32)Channel);
}

bool UForge2MMOManager::CreateGuild(const FString& GuildName, const FString& GuildTag)
{
	CurrentGuild.GuildId = FGuid::NewGuid().ToString();
	CurrentGuild.GuildName = GuildName;
	CurrentGuild.GuildTag = GuildTag;
	CurrentGuild.MemberCount = 1;
	CurrentGuild.Level = 1;
	CurrentGuild.CreatedDate = FDateTime::Now();

	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Created guild %s [%s]"), *GuildName, *GuildTag);
	return true;
}

bool UForge2MMOManager::JoinGuild(const FString& GuildId)
{
	CurrentGuild.GuildId = GuildId;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Joined guild %s"), *GuildId);
	return true;
}

void UForge2MMOManager::LeaveGuild()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Left guild %s"), *CurrentGuild.GuildName);
	CurrentGuild = FForge2GuildInfo();
}

FForge2GuildInfo UForge2MMOManager::GetCurrentGuild() const
{
	return CurrentGuild;
}

TArray<FForge2GuildMember> UForge2MMOManager::GetGuildMembers() const
{
	// Would query from server in production
	return TArray<FForge2GuildMember>();
}

void UForge2MMOManager::SetMemberRank(const FString& MemberId, EForge2GuildRank NewRank)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Set member %s rank to %d"), *MemberId, (int32)NewRank);
}

void UForge2MMOManager::KickMember(const FString& MemberId)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Kicked member %s from guild"), *MemberId);
}

bool UForge2MMOManager::CreateParty()
{
	CurrentParty.PartyId = FGuid::NewGuid().ToString();
	CurrentParty.LeaderId = TEXT("LocalPlayer");
	CurrentParty.MemberCount = 1;
	CurrentParty.MaxMembers = 5;

	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Created party %s"), *CurrentParty.PartyId);
	return true;
}

bool UForge2MMOManager::JoinParty(const FString& PartyId)
{
	CurrentParty.PartyId = PartyId;
	CurrentParty.MemberCount++;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Joined party %s"), *PartyId);
	return true;
}

void UForge2MMOManager::LeaveParty()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Left party %s"), *CurrentParty.PartyId);
	CurrentParty = FForge2PartyInfo();
}

void UForge2MMOManager::InviteToParty(const FString& PlayerId)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Invited %s to party"), *PlayerId);
}

FForge2PartyInfo UForge2MMOManager::GetCurrentParty() const
{
	return CurrentParty;
}

TArray<FForge2PartyMember> UForge2MMOManager::GetPartyMembers() const
{
	// Would query from server in production
	return TArray<FForge2PartyMember>();
}

void UForge2MMOManager::PromoteToLeader(const FString& MemberId)
{
	CurrentParty.LeaderId = MemberId;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Promoted %s to party leader"), *MemberId);
}

void UForge2MMOManager::SendMail(const FString& RecipientId, const FString& Subject, const FString& Body, const TArray<FString>& AttachmentIds, int32 CurrencyAmount)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Sent mail to %s - Subject: %s"), *RecipientId, *Subject);
}

TArray<FForge2MailMessage> UForge2MMOManager::GetInbox(bool bIncludeRead) const
{
	// Would query from server in production
	return TArray<FForge2MailMessage>();
}

void UForge2MMOManager::ReadMail(const FString& MailId)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Read mail %s"), *MailId);
}

void UForge2MMOManager::ClaimMailAttachments(const FString& MailId)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Claimed attachments from mail %s"), *MailId);
}

void UForge2MMOManager::DeleteMail(const FString& MailId)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 MMO: Deleted mail %s"), *MailId);
}

// ============================================================================
// UForge2SocialManager Implementation
// ============================================================================

UForge2SocialManager::UForge2SocialManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UForge2SocialManager::SendFriendRequest(const FString& PlayerId)
{
	FForge2FriendInfo Request;
	Request.PlayerId = PlayerId;
	Request.Relationship = EForge2RelationshipType::Requested;
	FriendsList.Add(Request);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Sent friend request to %s"), *PlayerId);
}

void UForge2SocialManager::AcceptFriendRequest(const FString& PlayerId)
{
	for (FForge2FriendInfo& Friend : FriendsList)
	{
		if (Friend.PlayerId == PlayerId && Friend.Relationship == EForge2RelationshipType::Pending)
		{
			Friend.Relationship = EForge2RelationshipType::Friend;
			UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Accepted friend request from %s"), *PlayerId);
			return;
		}
	}
}

void UForge2SocialManager::DeclineFriendRequest(const FString& PlayerId)
{
	FriendsList.RemoveAll([&PlayerId](const FForge2FriendInfo& Friend)
	{
		return Friend.PlayerId == PlayerId && Friend.Relationship == EForge2RelationshipType::Pending;
	});
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Declined friend request from %s"), *PlayerId);
}

void UForge2SocialManager::RemoveFriend(const FString& PlayerId)
{
	FriendsList.RemoveAll([&PlayerId](const FForge2FriendInfo& Friend)
	{
		return Friend.PlayerId == PlayerId;
	});
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Removed friend %s"), *PlayerId);
}

TArray<FForge2FriendInfo> UForge2SocialManager::GetFriendsList() const
{
	TArray<FForge2FriendInfo> Friends;
	for (const FForge2FriendInfo& Friend : FriendsList)
	{
		if (Friend.Relationship == EForge2RelationshipType::Friend)
		{
			Friends.Add(Friend);
		}
	}
	return Friends;
}

TArray<FForge2FriendInfo> UForge2SocialManager::GetPendingRequests() const
{
	TArray<FForge2FriendInfo> Pending;
	for (const FForge2FriendInfo& Friend : FriendsList)
	{
		if (Friend.Relationship == EForge2RelationshipType::Pending)
		{
			Pending.Add(Friend);
		}
	}
	return Pending;
}

TArray<FForge2FriendInfo> UForge2SocialManager::GetOnlineFriends() const
{
	TArray<FForge2FriendInfo> Online;
	for (const FForge2FriendInfo& Friend : FriendsList)
	{
		if (Friend.Relationship == EForge2RelationshipType::Friend &&
			Friend.Presence != EForge2PresenceState::Offline)
		{
			Online.Add(Friend);
		}
	}
	return Online;
}

void UForge2SocialManager::BlockPlayer(const FString& PlayerId)
{
	if (!BlockedPlayerIds.Contains(PlayerId))
	{
		BlockedPlayerIds.Add(PlayerId);
		RemoveFriend(PlayerId);
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Blocked player %s"), *PlayerId);
	}
}

void UForge2SocialManager::UnblockPlayer(const FString& PlayerId)
{
	BlockedPlayerIds.Remove(PlayerId);
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Unblocked player %s"), *PlayerId);
}

TArray<FForge2FriendInfo> UForge2SocialManager::GetBlockedPlayers() const
{
	TArray<FForge2FriendInfo> Blocked;
	for (const FString& PlayerId : BlockedPlayerIds)
	{
		FForge2FriendInfo Info;
		Info.PlayerId = PlayerId;
		Info.Relationship = EForge2RelationshipType::Blocked;
		Blocked.Add(Info);
	}
	return Blocked;
}

bool UForge2SocialManager::IsPlayerBlocked(const FString& PlayerId) const
{
	return BlockedPlayerIds.Contains(PlayerId);
}

void UForge2SocialManager::SetPresence(EForge2PresenceState State, const FString& Activity)
{
	CurrentPresence = State;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Set presence to %d - %s"), (int32)State, *Activity);
}

EForge2PresenceState UForge2SocialManager::GetPresence() const
{
	return CurrentPresence;
}

bool UForge2SocialManager::InitiateTrade(const FString& TargetPlayerId)
{
	CurrentTrade = FForge2TradeOffer();
	CurrentTrade.TradeId = FGuid::NewGuid().ToString();
	CurrentTrade.InitiatorId = TEXT("LocalPlayer");
	CurrentTrade.TargetId = TargetPlayerId;

	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Initiated trade with %s"), *TargetPlayerId);
	return true;
}

void UForge2SocialManager::AddItemToTrade(const FString& ItemId)
{
	CurrentTrade.InitiatorItems.Add(ItemId);
	CurrentTrade.bInitiatorConfirmed = false;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Added item %s to trade"), *ItemId);
}

void UForge2SocialManager::RemoveItemFromTrade(const FString& ItemId)
{
	CurrentTrade.InitiatorItems.Remove(ItemId);
	CurrentTrade.bInitiatorConfirmed = false;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Removed item %s from trade"), *ItemId);
}

void UForge2SocialManager::SetTradeCurrency(int32 Amount)
{
	CurrentTrade.InitiatorCurrency = Amount;
	CurrentTrade.bInitiatorConfirmed = false;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Set trade currency to %d"), Amount);
}

void UForge2SocialManager::ConfirmTrade()
{
	CurrentTrade.bInitiatorConfirmed = true;

	if (CurrentTrade.bInitiatorConfirmed && CurrentTrade.bTargetConfirmed)
	{
		// Trade complete - would exchange items
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Trade completed!"));
		CurrentTrade = FForge2TradeOffer();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Trade confirmed, waiting for other player"));
	}
}

void UForge2SocialManager::CancelTrade()
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Trade cancelled"));
	CurrentTrade = FForge2TradeOffer();
}

FForge2TradeOffer UForge2SocialManager::GetCurrentTradeOffer() const
{
	return CurrentTrade;
}

void UForge2SocialManager::ConfigureVoiceChat(const FForge2VoiceChatSettings& Settings)
{
	VoiceSettings = Settings;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Voice chat configured - Enabled: %d, PTT: %d"),
		Settings.bEnabled, Settings.bPushToTalk);
}

void UForge2SocialManager::SetVoiceChatEnabled(bool bEnabled)
{
	VoiceSettings.bEnabled = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Voice chat %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UForge2SocialManager::MutePlayer(const FString& PlayerId, bool bMuted)
{
	if (bMuted)
	{
		MutedPlayerIds.AddUnique(PlayerId);
	}
	else
	{
		MutedPlayerIds.Remove(PlayerId);
	}
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Player %s %s"), *PlayerId, bMuted ? TEXT("muted") : TEXT("unmuted"));
}

bool UForge2SocialManager::IsPlayerMuted(const FString& PlayerId) const
{
	return MutedPlayerIds.Contains(PlayerId);
}

void UForge2SocialManager::SetPlayerVolume(const FString& PlayerId, float Volume)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Set player %s volume to %.2f"), *PlayerId, Volume);
}

void UForge2SocialManager::PlayEmote(const FString& EmoteId)
{
	UE_LOG(LogTemp, Log, TEXT("FORGE2 Social: Playing emote %s"), *EmoteId);
}

TArray<FString> UForge2SocialManager::GetUnlockedEmotes() const
{
	// Would load from player data
	TArray<FString> Emotes;
	Emotes.Add(TEXT("wave"));
	Emotes.Add(TEXT("bow"));
	Emotes.Add(TEXT("dance"));
	Emotes.Add(TEXT("sit"));
	return Emotes;
}

// ============================================================================
// UForge2MultiplayerUtilities Implementation
// ============================================================================

bool UForge2MultiplayerUtilities::IsServer(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World && World->GetNetMode() != NM_Client;
}

bool UForge2MultiplayerUtilities::IsClient(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World && World->GetNetMode() == NM_Client;
}

bool UForge2MultiplayerUtilities::IsDedicatedServer(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World && World->GetNetMode() == NM_DedicatedServer;
}

bool UForge2MultiplayerUtilities::IsListenServer(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World && World->GetNetMode() == NM_ListenServer;
}

bool UForge2MultiplayerUtilities::HasAuthority(const AActor* Actor)
{
	return Actor && Actor->HasAuthority();
}

bool UForge2MultiplayerUtilities::IsLocallyControlled(const APawn* Pawn)
{
	return Pawn && Pawn->IsLocallyControlled();
}

FString UForge2MultiplayerUtilities::GetLocalPlayerId(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return TEXT("");
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC && PC->PlayerState)
		{
			return PC->PlayerState->GetPlayerName();
		}
	}
	return TEXT("");
}

FString UForge2MultiplayerUtilities::GetLocalPlayerName(const UObject* WorldContextObject)
{
	return GetLocalPlayerId(WorldContextObject);
}

int32 UForge2MultiplayerUtilities::GetPlayerCount(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return 0;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		int32 Count = 0;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			Count++;
		}
		return Count;
	}
	return 0;
}

FString UForge2MultiplayerUtilities::GenerateSessionId()
{
	return FGuid::NewGuid().ToString();
}

FString UForge2MultiplayerUtilities::GenerateMatchmakingTicket()
{
	return FString::Printf(TEXT("MM-%s-%lld"), *FGuid::NewGuid().ToString().Left(8), FDateTime::Now().ToUnixTimestamp());
}

bool UForge2MultiplayerUtilities::ValidatePlayerName(const FString& Name, FString& OutError)
{
	if (Name.Len() < 3)
	{
		OutError = TEXT("Name must be at least 3 characters");
		return false;
	}
	if (Name.Len() > 20)
	{
		OutError = TEXT("Name must be 20 characters or less");
		return false;
	}
	// Add profanity filter check in production
	return true;
}

bool UForge2MultiplayerUtilities::ValidateChatMessage(const FString& Message, FString& OutError)
{
	if (Message.IsEmpty())
	{
		OutError = TEXT("Message cannot be empty");
		return false;
	}
	if (Message.Len() > 500)
	{
		OutError = TEXT("Message exceeds maximum length");
		return false;
	}
	// Add profanity filter check in production
	return true;
}

FForge2SessionSettings UForge2MultiplayerUtilities::GetCoopPreset()
{
	FForge2SessionSettings Settings;
	Settings.SessionName = TEXT("Coop Game");
	Settings.MaxPlayers = 4;
	Settings.bIsLANMatch = false;
	Settings.bIsDedicated = false;
	Settings.bUsesPresence = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowInvites = true;
	Settings.GameMode = TEXT("Coop");
	return Settings;
}

FForge2SessionSettings UForge2MultiplayerUtilities::GetPvPPreset()
{
	FForge2SessionSettings Settings;
	Settings.SessionName = TEXT("PvP Match");
	Settings.MaxPlayers = 16;
	Settings.bIsLANMatch = false;
	Settings.bIsDedicated = true;
	Settings.bUsesPresence = true;
	Settings.bAllowJoinInProgress = false;
	Settings.bAllowInvites = false;
	Settings.bAntiCheatProtected = true;
	Settings.GameMode = TEXT("PvP");
	return Settings;
}

FForge2SessionSettings UForge2MultiplayerUtilities::GetMMOPreset()
{
	FForge2SessionSettings Settings;
	Settings.SessionName = TEXT("MMO World");
	Settings.MaxPlayers = 1000;
	Settings.bIsLANMatch = false;
	Settings.bIsDedicated = true;
	Settings.bUsesPresence = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowInvites = true;
	Settings.bUsesStats = true;
	Settings.GameMode = TEXT("MMO");
	return Settings;
}

FForge2MatchmakingSettings UForge2MultiplayerUtilities::GetCasualMatchmakingPreset()
{
	FForge2MatchmakingSettings Settings;
	Settings.GameMode = TEXT("Casual");
	Settings.MinPlayers = 2;
	Settings.MaxPlayers = 8;
	Settings.SearchTimeoutSeconds = 120.0f;
	Settings.SkillRange = 500;
	Settings.bExpandSkillOverTime = true;
	Settings.SkillExpansionRate = 20.0f;
	Settings.MaxPing = 200;
	return Settings;
}

FForge2MatchmakingSettings UForge2MultiplayerUtilities::GetRankedMatchmakingPreset()
{
	FForge2MatchmakingSettings Settings;
	Settings.GameMode = TEXT("Ranked");
	Settings.MinPlayers = 2;
	Settings.MaxPlayers = 10;
	Settings.SearchTimeoutSeconds = 180.0f;
	Settings.SkillRange = 100;
	Settings.bExpandSkillOverTime = true;
	Settings.SkillExpansionRate = 5.0f;
	Settings.MaxPing = 100;
	return Settings;
}

FForge2NetworkQoSSettings UForge2MultiplayerUtilities::GetLowLatencyQoSPreset()
{
	FForge2NetworkQoSSettings Settings;
	Settings.TickRate = 128;
	Settings.NetUpdateFrequency = 100;
	Settings.MinNetUpdateFrequency = 30;
	Settings.NetPriority = 3.0f;
	Settings.InterpolationDelay = 0.05f;
	Settings.bUseAdaptiveNetFrequency = false;
	Settings.MaxClientRate = 50000;
	Settings.MaxInternetClientRate = 30000;
	return Settings;
}

FForge2NetworkQoSSettings UForge2MultiplayerUtilities::GetBandwidthSaverQoSPreset()
{
	FForge2NetworkQoSSettings Settings;
	Settings.TickRate = 30;
	Settings.NetUpdateFrequency = 30;
	Settings.MinNetUpdateFrequency = 2;
	Settings.NetPriority = 1.0f;
	Settings.InterpolationDelay = 0.15f;
	Settings.bUseAdaptiveNetFrequency = true;
	Settings.MaxClientRate = 10000;
	Settings.MaxInternetClientRate = 5000;
	return Settings;
}
