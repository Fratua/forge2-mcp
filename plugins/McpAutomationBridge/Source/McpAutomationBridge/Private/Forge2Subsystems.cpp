// Copyright Epic Games, Inc. All Rights Reserved.
// FORGE2 Subsystems Implementation

#include "Forge2Subsystems.h"
#include "Forge2Components.h"
#include "Forge2DataAssets.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "ProceduralMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"
#include "EngineUtils.h"

// ============================================================================
// QUEST SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UMcpQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMcpQuestSubsystem::Deinitialize()
{
	ActiveQuests.Empty();
	CompletedQuests.Empty();
	FailedQuests.Empty();
	Super::Deinitialize();
}

bool UMcpQuestSubsystem::StartQuest(const FString& QuestId)
{
	if (ActiveQuests.Contains(QuestId) || CompletedQuests.Contains(QuestId))
	{
		return false;
	}

	// Check prerequisites
	CheckQuestPrerequisites(QuestId);

	FMcpActiveQuest NewQuest;
	NewQuest.QuestId = QuestId;
	NewQuest.State = EMcpQuestState::Active;
	NewQuest.StartTime = GetWorld()->GetTimeSeconds();
	NewQuest.CompletionTime = 0.0f;

	ActiveQuests.Add(QuestId, NewQuest);
	OnQuestStateChanged.Broadcast(QuestId, EMcpQuestState::Active);

	return true;
}

bool UMcpQuestSubsystem::CompleteQuest(const FString& QuestId)
{
	FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
	if (!Quest || Quest->State != EMcpQuestState::Active)
	{
		return false;
	}

	Quest->State = EMcpQuestState::Completed;
	Quest->CompletionTime = GetWorld()->GetTimeSeconds();

	CompletedQuests.Add(QuestId);
	GrantQuestRewards(QuestId);

	OnQuestStateChanged.Broadcast(QuestId, EMcpQuestState::Completed);

	// Remove from active
	ActiveQuests.Remove(QuestId);

	return true;
}

bool UMcpQuestSubsystem::FailQuest(const FString& QuestId)
{
	FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
	if (!Quest || Quest->State != EMcpQuestState::Active)
	{
		return false;
	}

	Quest->State = EMcpQuestState::Failed;
	Quest->CompletionTime = GetWorld()->GetTimeSeconds();

	FailedQuests.Add(QuestId);
	OnQuestStateChanged.Broadcast(QuestId, EMcpQuestState::Failed);

	ActiveQuests.Remove(QuestId);

	return true;
}

bool UMcpQuestSubsystem::AbandonQuest(const FString& QuestId)
{
	if (!ActiveQuests.Contains(QuestId))
	{
		return false;
	}

	ActiveQuests.Remove(QuestId);
	OnQuestStateChanged.Broadcast(QuestId, EMcpQuestState::Available);

	return true;
}

void UMcpQuestSubsystem::UpdateObjectiveProgress(const FString& QuestId, const FString& ObjectiveId, int32 Progress)
{
	FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
	if (!Quest || Quest->State != EMcpQuestState::Active)
	{
		return;
	}

	Quest->ObjectiveProgress.Add(ObjectiveId, Progress);
	OnObjectiveProgress.Broadcast(QuestId, ObjectiveId, Progress);
}

bool UMcpQuestSubsystem::IsQuestAvailable(const FString& QuestId) const
{
	return !ActiveQuests.Contains(QuestId) && !CompletedQuests.Contains(QuestId) && !FailedQuests.Contains(QuestId);
}

bool UMcpQuestSubsystem::IsQuestActive(const FString& QuestId) const
{
	const FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
	return Quest && Quest->State == EMcpQuestState::Active;
}

bool UMcpQuestSubsystem::IsQuestCompleted(const FString& QuestId) const
{
	return CompletedQuests.Contains(QuestId);
}

EMcpQuestState UMcpQuestSubsystem::GetQuestState(const FString& QuestId) const
{
	if (CompletedQuests.Contains(QuestId)) return EMcpQuestState::Completed;
	if (FailedQuests.Contains(QuestId)) return EMcpQuestState::Failed;
	const FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
	if (Quest) return Quest->State;
	return EMcpQuestState::Available;
}

FMcpActiveQuest UMcpQuestSubsystem::GetActiveQuest(const FString& QuestId) const
{
	const FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
	return Quest ? *Quest : FMcpActiveQuest();
}

TArray<FMcpActiveQuest> UMcpQuestSubsystem::GetAllActiveQuests() const
{
	TArray<FMcpActiveQuest> Result;
	for (const auto& Pair : ActiveQuests)
	{
		if (Pair.Value.State == EMcpQuestState::Active)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

int32 UMcpQuestSubsystem::GetObjectiveProgress(const FString& QuestId, const FString& ObjectiveId) const
{
	const FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
	if (Quest)
	{
		const int32* Progress = Quest->ObjectiveProgress.Find(ObjectiveId);
		return Progress ? *Progress : 0;
	}
	return 0;
}

void UMcpQuestSubsystem::CheckQuestPrerequisites(const FString& QuestId)
{
	// Find the quest data asset by ID
	UMcpQuestDataAsset* QuestData = nullptr;
	
	// Search through loaded quest assets in the asset registry
	for (TObjectIterator<UMcpQuestDataAsset> It; It; ++It)
	{
		if (It->QuestId == QuestId)
		{
			QuestData = *It;
			break;
		}
	}
	
	if (!QuestData)
	{
		return;
	}
	
	// Check required completed quests
	for (const FString& RequiredQuestId : QuestData->RequiredCompletedQuests)
	{
		if (!CompletedQuests.Contains(RequiredQuestId))
		{
			// Prerequisite not met - mark quest as unavailable
			FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
			if (Quest)
			{
				Quest->State = EMcpQuestState::Unavailable;
			}
			return;
		}
	}
	
	// Check player level requirement if applicable
	if (QuestData->RequiredPlayerLevel > 0)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->GetPawn())
		{
			UMcpProgressionComponent* ProgressionComp = PC->GetPawn()->FindComponentByClass<UMcpProgressionComponent>();
			if (ProgressionComp && ProgressionComp->GetCurrentLevel() < QuestData->RequiredPlayerLevel)
			{
				FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
				if (Quest)
				{
					Quest->State = EMcpQuestState::Unavailable;
				}
				return;
			}
		}
	}
	
	// Check faction standing requirement
	if (!QuestData->RequiredFaction.IsEmpty() && QuestData->RequiredFactionStanding > 0)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->GetPawn())
		{
			UMcpFactionComponent* FactionComp = PC->GetPawn()->FindComponentByClass<UMcpFactionComponent>();
			if (FactionComp)
			{
				int32 Standing = FactionComp->GetFactionStanding(FName(*QuestData->RequiredFaction));
				if (Standing < QuestData->RequiredFactionStanding)
				{
					FMcpActiveQuest* Quest = ActiveQuests.Find(QuestId);
					if (Quest)
					{
						Quest->State = EMcpQuestState::Unavailable;
					}
					return;
				}
			}
		}
	}
}

void UMcpQuestSubsystem::GrantQuestRewards(const FString& QuestId)
{
	// Find the quest data asset by ID
	UMcpQuestDataAsset* QuestData = nullptr;
	
	for (TObjectIterator<UMcpQuestDataAsset> It; It; ++It)
	{
		if (It->QuestId == QuestId)
		{
			QuestData = *It;
			break;
		}
	}
	
	if (!QuestData)
	{
		return;
	}
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return;
	}
	
	APawn* PlayerPawn = PC->GetPawn();
	
	// Grant experience points
	if (QuestData->ExperienceReward > 0)
	{
		UMcpProgressionComponent* ProgressionComp = PlayerPawn->FindComponentByClass<UMcpProgressionComponent>();
		if (ProgressionComp)
		{
			ProgressionComp->AddExperience(QuestData->ExperienceReward);
		}
	}
	
	// Grant currency rewards
	if (QuestData->CurrencyReward > 0)
	{
		UMcpInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UMcpInventoryComponent>();
		if (InventoryComp)
		{
			FName CurrencyId = QuestData->CurrencyType.IsEmpty() ? FName(TEXT("Gold")) : FName(*QuestData->CurrencyType);
			InventoryComp->ModifyCurrency(CurrencyId, QuestData->CurrencyReward);
		}
	}
	
	// Grant item rewards
	UMcpInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UMcpInventoryComponent>();
	if (InventoryComp)
	{
		for (const FMcpItemReward& ItemReward : QuestData->ItemRewards)
		{
			InventoryComp->AddItem(FName(*ItemReward.ItemId), ItemReward.Quantity);
		}
	}
	
	// Grant faction reputation
	if (!QuestData->FactionReputationRewards.IsEmpty())
	{
		UMcpFactionComponent* FactionComp = PlayerPawn->FindComponentByClass<UMcpFactionComponent>();
		if (FactionComp)
		{
			for (const auto& FactionReward : QuestData->FactionReputationRewards)
			{
				FactionComp->ModifyFactionStanding(FName(*FactionReward.Key), FactionReward.Value);
			}
		}
	}
	
	// Unlock any follow-up quests
	for (const FString& UnlockedQuestId : QuestData->UnlocksQuests)
	{
		// Broadcast that a new quest is now available
		OnQuestStateChanged.Broadcast(UnlockedQuestId, EMcpQuestState::Available);
	}
}

// ============================================================================
// ECONOMY SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UMcpEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMcpEconomySubsystem::Deinitialize()
{
	RegisteredShops.Empty();
	AuctionListings.Empty();
	Super::Deinitialize();
}

bool UMcpEconomySubsystem::RegisterShop(const FMcpShop& Shop)
{
	if (RegisteredShops.Contains(Shop.ShopId))
	{
		return false;
	}

	RegisteredShops.Add(Shop.ShopId, Shop);
	return true;
}

bool UMcpEconomySubsystem::BuyItem(const FString& ShopId, const FString& ItemId, int32 Quantity, AActor* Buyer)
{
	FMcpShop* Shop = RegisteredShops.Find(ShopId);
	if (!Shop || !Buyer)
	{
		return false;
	}

	// Find item in shop
	FMcpShopItem* ShopItem = nullptr;
	for (auto& Item : Shop->Items)
	{
		if (Item.ItemId == ItemId)
		{
			ShopItem = &Item;
			break;
		}
	}

	if (!ShopItem || ShopItem->Stock < Quantity)
	{
		return false;
	}

	int32 TotalPrice = FMath::CeilToInt(ShopItem->Price * Quantity * Shop->BuyPriceMultiplier);

	// Apply reputation modifier
	TotalPrice = FMath::CeilToInt(TotalPrice * GetReputationPriceModifier(Buyer, Shop->RequiredFaction));

	// Get buyer's inventory component
	UMcpInventoryComponent* BuyerInventory = Buyer->FindComponentByClass<UMcpInventoryComponent>();
	if (!BuyerInventory)
	{
		return false;
	}
	
	// Check if buyer has enough currency
	int64 BuyerGold = BuyerInventory->GetCurrency(Shop->CurrencyId);
	if (BuyerGold < TotalPrice)
	{
		OnTransactionFailed.Broadcast(Buyer->GetName(), ItemId, TEXT("Insufficient funds"));
		return false;
	}
	
	// Check if buyer can hold the items
	if (!BuyerInventory->CanAddItem(FName(*ItemId), Quantity))
	{
		OnTransactionFailed.Broadcast(Buyer->GetName(), ItemId, TEXT("Inventory full"));
		return false;
	}
	
	// Deduct currency
	if (!BuyerInventory->ModifyCurrency(Shop->CurrencyId, -TotalPrice))
	{
		return false;
	}
	
	// Add items to buyer's inventory
	int32 Added = BuyerInventory->AddItem(FName(*ItemId), Quantity);
	if (Added < Quantity)
	{
		// Refund if couldn't add all items
		BuyerInventory->ModifyCurrency(Shop->CurrencyId, TotalPrice);
		return false;
	}

	ShopItem->Stock -= Quantity;
	OnTransactionComplete.Broadcast(Buyer->GetName(), ItemId, Quantity);

	return true;
}

bool UMcpEconomySubsystem::SellItem(const FString& ShopId, const FString& ItemId, int32 Quantity, AActor* Seller)
{
	FMcpShop* Shop = RegisteredShops.Find(ShopId);
	if (!Shop || !Seller)
	{
		return false;
	}

	// Find base price for item
	int32 BasePrice = 100; // Default, would be looked up from item database
	for (const auto& Item : Shop->Items)
	{
		if (Item.ItemId == ItemId)
		{
			BasePrice = Item.Price;
			break;
		}
	}

	int32 SellPrice = FMath::CeilToInt(BasePrice * Quantity * Shop->SellPriceMultiplier);

	// Get seller's inventory component
	UMcpInventoryComponent* SellerInventory = Seller->FindComponentByClass<UMcpInventoryComponent>();
	if (!SellerInventory)
	{
		return false;
	}
	
	// Check if seller has the items
	int32 SellerHas = SellerInventory->GetItemCount(FName(*ItemId));
	if (SellerHas < Quantity)
	{
		OnTransactionFailed.Broadcast(Seller->GetName(), ItemId, TEXT("Insufficient items"));
		return false;
	}
	
	// Remove items from seller's inventory
	int32 Removed = SellerInventory->RemoveItem(FName(*ItemId), Quantity);
	if (Removed < Quantity)
	{
		return false;
	}
	
	// Add currency to seller
	SellerInventory->ModifyCurrency(Shop->CurrencyId, SellPrice);

	OnTransactionComplete.Broadcast(Seller->GetName(), ItemId, -Quantity);

	return true;
}

FMcpShop UMcpEconomySubsystem::GetShop(const FString& ShopId) const
{
	const FMcpShop* Shop = RegisteredShops.Find(ShopId);
	return Shop ? *Shop : FMcpShop();
}

int32 UMcpEconomySubsystem::GetItemPrice(const FString& ShopId, const FString& ItemId, bool bBuying) const
{
	const FMcpShop* Shop = RegisteredShops.Find(ShopId);
	if (!Shop)
	{
		return 0;
	}

	for (const auto& Item : Shop->Items)
	{
		if (Item.ItemId == ItemId)
		{
			float Multiplier = bBuying ? Shop->BuyPriceMultiplier : Shop->SellPriceMultiplier;
			return FMath::CeilToInt(Item.Price * Multiplier);
		}
	}

	return 0;
}

FString UMcpEconomySubsystem::CreateAuctionListing(const FString& SellerId, const FString& ItemId, int32 Quantity, int32 StartingBid, int32 BuyoutPrice, float Duration)
{
	FMcpAuctionListing Listing;
	Listing.ListingId = FGuid::NewGuid().ToString();
	Listing.SellerId = SellerId;
	Listing.ItemId = ItemId;
	Listing.Quantity = Quantity;
	Listing.CurrentBid = StartingBid;
	Listing.BuyoutPrice = BuyoutPrice;
	Listing.HighestBidderId = TEXT("");
	Listing.ExpirationTime = GetWorld()->GetTimeSeconds() + Duration;

	AuctionListings.Add(Listing.ListingId, Listing);

	return Listing.ListingId;
}

bool UMcpEconomySubsystem::PlaceBid(const FString& ListingId, const FString& BidderId, int32 BidAmount)
{
	FMcpAuctionListing* Listing = AuctionListings.Find(ListingId);
	if (!Listing || BidAmount <= Listing->CurrentBid)
	{
		return false;
	}

	if (GetWorld()->GetTimeSeconds() > Listing->ExpirationTime)
	{
		return false;
	}

	// Get bidder actor to access their inventory
	AActor* Bidder = FindActorById(BidderId);
	if (!Bidder) return false;
	
	UMcpInventoryComponent* BidderInventory = Bidder->FindComponentByClass<UMcpInventoryComponent>();
	if (!BidderInventory) return false;
	
	// Check if bidder has enough currency
	if (BidderInventory->GetCurrency(DefaultCurrencyId) < BidAmount)
	{
		return false;
	}
	
	// Refund previous bidder if exists
	if (!Listing->HighestBidderId.IsEmpty())
	{
		AActor* PreviousBidder = FindActorById(Listing->HighestBidderId);
		if (PreviousBidder)
		{
			UMcpInventoryComponent* PrevInventory = PreviousBidder->FindComponentByClass<UMcpInventoryComponent>();
			if (PrevInventory)
			{
				// Return held currency to previous bidder
				PrevInventory->ModifyCurrency(DefaultCurrencyId, Listing->CurrentBid);
			}
		}
	}
	
	// Hold new bidder's currency
	if (!BidderInventory->ModifyCurrency(DefaultCurrencyId, -BidAmount))
	{
		return false;
	}

	Listing->CurrentBid = BidAmount;
	Listing->HighestBidderId = BidderId;
	
	OnAuctionBidPlaced.Broadcast(ListingId, BidderId, BidAmount);

	return true;
}

bool UMcpEconomySubsystem::BuyoutListing(const FString& ListingId, const FString& BuyerId)
{
	FMcpAuctionListing* Listing = AuctionListings.Find(ListingId);
	if (!Listing || Listing->BuyoutPrice <= 0)
	{
		return false;
	}

	// Get buyer actor
	AActor* Buyer = FindActorById(BuyerId);
	if (!Buyer) return false;
	
	UMcpInventoryComponent* BuyerInventory = Buyer->FindComponentByClass<UMcpInventoryComponent>();
	if (!BuyerInventory) return false;
	
	// Check if buyer has enough currency
	if (BuyerInventory->GetCurrency(DefaultCurrencyId) < Listing->BuyoutPrice)
	{
		return false;
	}
	
	// Deduct currency from buyer
	if (!BuyerInventory->ModifyCurrency(DefaultCurrencyId, -Listing->BuyoutPrice))
	{
		return false;
	}
	
	// Refund previous bidder if any
	if (!Listing->HighestBidderId.IsEmpty())
	{
		AActor* PreviousBidder = FindActorById(Listing->HighestBidderId);
		if (PreviousBidder)
		{
			UMcpInventoryComponent* PrevInventory = PreviousBidder->FindComponentByClass<UMcpInventoryComponent>();
			if (PrevInventory)
			{
				PrevInventory->ModifyCurrency(DefaultCurrencyId, Listing->CurrentBid);
			}
		}
	}
	
	// Give item to buyer
	BuyerInventory->AddItem(FName(*Listing->ItemId), Listing->Quantity);
	
	// Give currency to seller
	AActor* Seller = FindActorById(Listing->SellerId);
	if (Seller)
	{
		UMcpInventoryComponent* SellerInventory = Seller->FindComponentByClass<UMcpInventoryComponent>();
		if (SellerInventory)
		{
			SellerInventory->ModifyCurrency(DefaultCurrencyId, Listing->BuyoutPrice);
		}
	}

	OnAuctionEnded.Broadcast(ListingId, BuyerId);
	AuctionListings.Remove(ListingId);

	return true;
}

bool UMcpEconomySubsystem::CancelListing(const FString& ListingId, const FString& SellerId)
{
	FMcpAuctionListing* Listing = AuctionListings.Find(ListingId);
	if (!Listing || Listing->SellerId != SellerId)
	{
		return false;
	}

	// Refund bidder if exists
	if (!Listing->HighestBidderId.IsEmpty())
	{
		AActor* Bidder = FindActorById(Listing->HighestBidderId);
		if (Bidder)
		{
			UMcpInventoryComponent* BidderInventory = Bidder->FindComponentByClass<UMcpInventoryComponent>();
			if (BidderInventory)
			{
				BidderInventory->ModifyCurrency(DefaultCurrencyId, Listing->CurrentBid);
			}
		}
	}

	// Return item to seller
	AActor* Seller = FindActorById(SellerId);
	if (Seller)
	{
		UMcpInventoryComponent* SellerInventory = Seller->FindComponentByClass<UMcpInventoryComponent>();
		if (SellerInventory)
		{
			SellerInventory->AddItem(FName(*Listing->ItemId), Listing->Quantity);
		}
	}
	
	OnAuctionCancelled.Broadcast(ListingId);
	AuctionListings.Remove(ListingId);

	return true;
}

TArray<FMcpAuctionListing> UMcpEconomySubsystem::SearchAuctions(const FString& ItemIdFilter, int32 MaxPrice) const
{
	TArray<FMcpAuctionListing> Results;

	for (const auto& Pair : AuctionListings)
	{
		const FMcpAuctionListing& Listing = Pair.Value;

		if (!ItemIdFilter.IsEmpty() && !Listing.ItemId.Contains(ItemIdFilter))
		{
			continue;
		}

		if (MaxPrice > 0 && (Listing.BuyoutPrice > MaxPrice || (Listing.BuyoutPrice <= 0 && Listing.CurrentBid > MaxPrice)))
		{
			continue;
		}

		Results.Add(Listing);
	}

	return Results;
}

void UMcpEconomySubsystem::ProcessExpiredAuctions()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	TArray<FString> ExpiredListings;

	for (const auto& Pair : AuctionListings)
	{
		if (CurrentTime > Pair.Value.ExpirationTime)
		{
			ExpiredListings.Add(Pair.Key);
		}
	}

	for (const FString& ListingId : ExpiredListings)
	{
		FMcpAuctionListing* Listing = AuctionListings.Find(ListingId);
		if (Listing)
		{
			if (!Listing->HighestBidderId.IsEmpty())
			{
				// Auction was won - transfer item to winner and currency to seller
				AActor* Winner = FindActorById(Listing->HighestBidderId);
				if (Winner)
				{
					UMcpInventoryComponent* WinnerInventory = Winner->FindComponentByClass<UMcpInventoryComponent>();
					if (WinnerInventory)
					{
						// Winner already had currency held, so just give item
						WinnerInventory->AddItem(FName(*Listing->ItemId), Listing->Quantity);
					}
				}
				
				// Give currency to seller (minus 5% auction house fee)
				AActor* Seller = FindActorById(Listing->SellerId);
				if (Seller)
				{
					UMcpInventoryComponent* SellerInventory = Seller->FindComponentByClass<UMcpInventoryComponent>();
					if (SellerInventory)
					{
						int32 SellerPayout = FMath::FloorToInt(Listing->CurrentBid * 0.95f);
						SellerInventory->ModifyCurrency(DefaultCurrencyId, SellerPayout);
					}
				}
				
				OnAuctionEnded.Broadcast(ListingId, Listing->HighestBidderId);
			}
			else
			{
				// No bids, return item to seller
				AActor* Seller = FindActorById(Listing->SellerId);
				if (Seller)
				{
					UMcpInventoryComponent* SellerInventory = Seller->FindComponentByClass<UMcpInventoryComponent>();
					if (SellerInventory)
					{
						SellerInventory->AddItem(FName(*Listing->ItemId), Listing->Quantity);
					}
				}
				OnAuctionEnded.Broadcast(ListingId, TEXT(""));
			}

			AuctionListings.Remove(ListingId);
		}
	}
}

void UMcpEconomySubsystem::UpdateShopStock()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (auto& ShopPair : RegisteredShops)
	{
		for (auto& Item : ShopPair.Value.Items)
		{
			if (Item.Stock < Item.MaxStock && Item.RestockTime > 0)
			{
				float TimeSinceRestock = CurrentTime - Item.LastRestockTime;
				if (TimeSinceRestock >= Item.RestockTime)
				{
					Item.Stock = FMath::Min(Item.Stock + 1, Item.MaxStock);
					Item.LastRestockTime = CurrentTime;
				}
			}
		}
	}
}

float UMcpEconomySubsystem::GetReputationPriceModifier(AActor* Actor, const FString& FactionId) const
{
	if (!Actor)
	{
		return 1.0f;
	}
	
	// Check for faction component on the actor
	UMcpFactionComponent* FactionComp = Actor->FindComponentByClass<UMcpFactionComponent>();
	if (!FactionComp || FactionId.IsEmpty())
	{
		return 1.0f;
	}
	
	// Get faction standing
	int32 Standing = FactionComp->GetFactionStanding(FName(*FactionId));
	
	// Calculate price modifier based on standing
	// Standing ranges from -100 (hostile) to +100 (exalted)
	// Price modifier: 1.2x at -100, 1.0x at 0, 0.8x at +100
	float StandingNormalized = FMath::Clamp(Standing / 100.0f, -1.0f, 1.0f);
	return 1.0f - (StandingNormalized * 0.2f);
}

AActor* UMcpEconomySubsystem::FindActorById(const FString& ActorId) const
{
	UWorld* World = GetWorld();
	if (!World || ActorId.IsEmpty())
	{
		return nullptr;
	}
	
	// First try to find by player controller name
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			FString PlayerName = PC->GetPlayerState<APlayerState>() ? 
				PC->GetPlayerState<APlayerState>()->GetPlayerName() : PC->GetName();
			if (PlayerName == ActorId)
			{
				return PC->GetPawn();
			}
		}
	}
	
	// Try to find by actor name
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetName() == ActorId || It->GetFName().ToString() == ActorId)
		{
			return *It;
		}
	}
	
	// Try to find by unique network ID or GUID
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		// Check for a custom ID component if available
		UActorComponent* IdComp = It->GetComponentByClass(UMcpIdentityComponent::StaticClass());
		if (IdComp)
		{
			UMcpIdentityComponent* IdentityComp = Cast<UMcpIdentityComponent>(IdComp);
			if (IdentityComp && IdentityComp->GetUniqueId() == ActorId)
			{
				return *It;
			}
		}
	}
	
	return nullptr;
}

// ============================================================================
// SPAWN SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UMcpSpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMcpSpawnSubsystem::Deinitialize()
{
	SpawnPoints.Empty();
	WaveSpawners.Empty();

	// Clean up pooled actors
	for (auto& PoolPair : ObjectPools)
	{
		for (AActor* Actor : PoolPair.Value)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}
	}
	ObjectPools.Empty();

	Super::Deinitialize();
}

void UMcpSpawnSubsystem::Tick(float DeltaTime)
{
	ProcessSpawnPoints(DeltaTime);
	ProcessWaveSpawners(DeltaTime);
}

TStatId UMcpSpawnSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMcpSpawnSubsystem, STATGROUP_Tickables);
}

bool UMcpSpawnSubsystem::RegisterSpawnPoint(const FMcpSpawnPoint& SpawnPoint)
{
	if (SpawnPoints.Contains(SpawnPoint.SpawnPointId))
	{
		return false;
	}

	SpawnPoints.Add(SpawnPoint.SpawnPointId, SpawnPoint);
	return true;
}

bool UMcpSpawnSubsystem::UnregisterSpawnPoint(const FString& SpawnPointId)
{
	return SpawnPoints.Remove(SpawnPointId) > 0;
}

AActor* UMcpSpawnSubsystem::SpawnAtPoint(const FString& SpawnPointId)
{
	FMcpSpawnPoint* Point = SpawnPoints.Find(SpawnPointId);
	if (!Point || !Point->ActorClass)
	{
		return nullptr;
	}

	if (Point->MaxActiveSpawns > 0 && Point->CurrentActiveCount >= Point->MaxActiveSpawns)
	{
		return nullptr;
	}

	// Try to get from pool first
	AActor* SpawnedActor = GetPooledActor(Point->ActorClass);

	if (!SpawnedActor)
	{
		// Spawn new actor
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector SpawnLocation = Point->Location;
		if (Point->SpawnRadius > 0)
		{
			FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0, Point->SpawnRadius);
			RandomOffset.Z = 0;
			SpawnLocation += RandomOffset;
		}

		SpawnedActor = GetWorld()->SpawnActor<AActor>(Point->ActorClass, SpawnLocation, Point->Rotation, SpawnParams);
	}
	else
	{
		SpawnedActor->SetActorLocationAndRotation(Point->Location, Point->Rotation);
		SpawnedActor->SetActorHiddenInGame(false);
		SpawnedActor->SetActorEnableCollision(true);
	}

	if (SpawnedActor)
	{
		Point->CurrentActiveCount++;
		Point->LastSpawnTime = GetWorld()->GetTimeSeconds();
		OnActorSpawned.Broadcast(SpawnedActor, SpawnPointId);
	}

	return SpawnedActor;
}

void UMcpSpawnSubsystem::OnSpawnedActorDestroyed(const FString& SpawnPointId)
{
	FMcpSpawnPoint* Point = SpawnPoints.Find(SpawnPointId);
	if (Point && Point->CurrentActiveCount > 0)
	{
		Point->CurrentActiveCount--;
	}
}

FMcpSpawnPoint UMcpSpawnSubsystem::GetSpawnPoint(const FString& SpawnPointId) const
{
	const FMcpSpawnPoint* Point = SpawnPoints.Find(SpawnPointId);
	return Point ? *Point : FMcpSpawnPoint();
}

bool UMcpSpawnSubsystem::RegisterWaveSpawner(const FMcpWaveSpawner& WaveSpawner)
{
	if (WaveSpawners.Contains(WaveSpawner.WaveSpawnerId))
	{
		return false;
	}

	WaveSpawners.Add(WaveSpawner.WaveSpawnerId, WaveSpawner);
	return true;
}

bool UMcpSpawnSubsystem::StartWaveSpawner(const FString& WaveSpawnerId)
{
	FMcpWaveSpawner* Spawner = WaveSpawners.Find(WaveSpawnerId);
	if (!Spawner || Spawner->bWaveInProgress)
	{
		return false;
	}

	Spawner->CurrentWave = 0;
	SpawnNextWave(*Spawner);

	return true;
}

bool UMcpSpawnSubsystem::StopWaveSpawner(const FString& WaveSpawnerId)
{
	FMcpWaveSpawner* Spawner = WaveSpawners.Find(WaveSpawnerId);
	if (!Spawner)
	{
		return false;
	}

	Spawner->bWaveInProgress = false;
	return true;
}

void UMcpSpawnSubsystem::OnWaveEnemyKilled(const FString& WaveSpawnerId)
{
	FMcpWaveSpawner* Spawner = WaveSpawners.Find(WaveSpawnerId);
	if (!Spawner || !Spawner->bWaveInProgress)
	{
		return;
	}

	Spawner->RemainingEnemies--;

	if (Spawner->RemainingEnemies <= 0)
	{
		OnWaveCompleted.Broadcast(WaveSpawnerId, Spawner->CurrentWave);

		if (Spawner->CurrentWave >= Spawner->TotalWaves)
		{
			Spawner->bWaveInProgress = false;
		}
		// Next wave will be spawned in ProcessWaveSpawners after delay
	}
}

FMcpWaveSpawner UMcpSpawnSubsystem::GetWaveSpawner(const FString& WaveSpawnerId) const
{
	const FMcpWaveSpawner* Spawner = WaveSpawners.Find(WaveSpawnerId);
	return Spawner ? *Spawner : FMcpWaveSpawner();
}

void UMcpSpawnSubsystem::PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!ActorClass)
	{
		return;
	}

	TArray<AActor*>& Pool = ObjectPools.FindOrAdd(ActorClass);

	for (int32 i = 0; i < Count; i++)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* PooledActor = GetWorld()->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PooledActor)
		{
			PooledActor->SetActorHiddenInGame(true);
			PooledActor->SetActorEnableCollision(false);
			Pool.Add(PooledActor);
		}
	}
}

AActor* UMcpSpawnSubsystem::GetPooledActor(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	TArray<AActor*>* Pool = ObjectPools.Find(ActorClass);
	if (!Pool || Pool->Num() == 0)
	{
		return nullptr;
	}

	AActor* Actor = Pool->Pop();
	return Actor;
}

void UMcpSpawnSubsystem::ReturnToPool(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorLocation(FVector(0, 0, -10000));

	TArray<AActor*>& Pool = ObjectPools.FindOrAdd(Actor->GetClass());
	Pool.Add(Actor);
}

void UMcpSpawnSubsystem::ProcessSpawnPoints(float DeltaTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (auto& PointPair : SpawnPoints)
	{
		FMcpSpawnPoint& Point = PointPair.Value;

		if (Point.State == EMcpSpawnState::Cooldown)
		{
			if (CurrentTime - Point.LastSpawnTime >= Point.RespawnDelay)
			{
				Point.State = EMcpSpawnState::Idle;
			}
		}

		if (Point.State == EMcpSpawnState::Idle && Point.CurrentActiveCount < Point.MaxActiveSpawns)
		{
			SpawnAtPoint(Point.SpawnPointId);
			Point.State = EMcpSpawnState::Cooldown;
		}
	}
}

void UMcpSpawnSubsystem::ProcessWaveSpawners(float DeltaTime)
{
	// Wave spawner processing would handle wave delays and transitions
}

void UMcpSpawnSubsystem::SpawnNextWave(FMcpWaveSpawner& WaveSpawner)
{
	WaveSpawner.CurrentWave++;
	WaveSpawner.bWaveInProgress = true;

	int32 EnemiesToSpawn = FMath::CeilToInt(WaveSpawner.EnemiesPerWave * FMath::Pow(WaveSpawner.DifficultyScaling, WaveSpawner.CurrentWave - 1));
	WaveSpawner.RemainingEnemies = EnemiesToSpawn;

	OnWaveStarted.Broadcast(WaveSpawner.WaveSpawnerId, WaveSpawner.CurrentWave);

	// Spawn enemies across spawn points
	int32 PointIndex = 0;
	for (int32 i = 0; i < EnemiesToSpawn && WaveSpawner.SpawnPoints.Num() > 0; i++)
	{
		FMcpSpawnPoint& Point = WaveSpawner.SpawnPoints[PointIndex];
		SpawnAtPoint(Point.SpawnPointId);
		PointIndex = (PointIndex + 1) % WaveSpawner.SpawnPoints.Num();
	}
}

// ============================================================================
// WEATHER SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UMcpWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Default time settings
	CurrentTime.Hour = 12;
	CurrentTime.Minute = 0;
	CurrentTime.Day = 1;
	CurrentTime.Month = 1;
	CurrentTime.Year = 1;
	CurrentTime.Season = EMcpSeason::Summer;
	CurrentTime.DayLength = 1200.0f; // 20 minutes per day
	CurrentTime.bIsDaytime = true;
	CurrentTime.NormalizedTime = 0.5f;

	// Default weather
	CurrentWeather.WeatherType = EMcpWeatherType::Clear;
	CurrentWeather.Intensity = 0.0f;
	CurrentWeather.Temperature = 20.0f;
	CurrentWeather.WindSpeed = 5.0f;
	CurrentWeather.WindDirection = FVector(1, 0, 0);
	CurrentWeather.Humidity = 0.5f;
	CurrentWeather.CloudCoverage = 0.2f;
	CurrentWeather.Visibility = 1.0f;

	TargetWeather = CurrentWeather;
	TimeScale = 1.0f;
	bTimePaused = false;
	WeatherTransitionProgress = 1.0f;
	WeatherTransitionDuration = 0.0f;
}

void UMcpWeatherSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMcpWeatherSubsystem::Tick(float DeltaTime)
{
	UpdateTime(DeltaTime);
	UpdateWeatherTransition(DeltaTime);
}

TStatId UMcpWeatherSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMcpWeatherSubsystem, STATGROUP_Tickables);
}

void UMcpWeatherSubsystem::SetTime(int32 Hour, int32 Minute)
{
	int32 OldHour = CurrentTime.Hour;
	CurrentTime.Hour = FMath::Clamp(Hour, 0, 23);
	CurrentTime.Minute = FMath::Clamp(Minute, 0, 59);
	CurrentTime.NormalizedTime = (CurrentTime.Hour * 60 + CurrentTime.Minute) / 1440.0f;

	bool bWasDaytime = CurrentTime.bIsDaytime;
	CurrentTime.bIsDaytime = (CurrentTime.Hour >= 6 && CurrentTime.Hour < 20);

	if (CurrentTime.Hour != OldHour)
	{
		OnHourChanged.Broadcast(CurrentTime.Hour);
	}

	if (CurrentTime.bIsDaytime != bWasDaytime)
	{
		OnDayNightChanged.Broadcast(CurrentTime.bIsDaytime);
	}
}

void UMcpWeatherSubsystem::SetDate(int32 Day, int32 Month, int32 Year)
{
	CurrentTime.Day = FMath::Clamp(Day, 1, 30);
	CurrentTime.Month = FMath::Clamp(Month, 1, 12);
	CurrentTime.Year = FMath::Max(Year, 1);

	UpdateSeasonFromDate();
}

void UMcpWeatherSubsystem::SetTimeScale(float Scale)
{
	TimeScale = FMath::Max(Scale, 0.0f);
}

void UMcpWeatherSubsystem::PauseTime(bool bPause)
{
	bTimePaused = bPause;
}

FMcpTimeOfDay UMcpWeatherSubsystem::GetCurrentTime() const
{
	return CurrentTime;
}

bool UMcpWeatherSubsystem::IsDaytime() const
{
	return CurrentTime.bIsDaytime;
}

float UMcpWeatherSubsystem::GetSunAngle() const
{
	// 0 at sunrise (6am), 90 at noon, 180 at sunset (6pm)
	if (CurrentTime.Hour < 6 || CurrentTime.Hour >= 20)
	{
		return -1.0f; // Below horizon
	}

	float DayProgress = (CurrentTime.Hour - 6 + CurrentTime.Minute / 60.0f) / 14.0f;
	return DayProgress * 180.0f;
}

void UMcpWeatherSubsystem::SetWeather(EMcpWeatherType NewWeather, float TransitionTime)
{
	if (NewWeather == CurrentWeather.WeatherType && WeatherTransitionProgress >= 1.0f)
	{
		return;
	}

	EMcpWeatherType OldWeather = CurrentWeather.WeatherType;

	TargetWeather.WeatherType = NewWeather;

	// Set default values based on weather type
	switch (NewWeather)
	{
	case EMcpWeatherType::Clear:
		TargetWeather.CloudCoverage = 0.1f;
		TargetWeather.Intensity = 0.0f;
		TargetWeather.Visibility = 1.0f;
		break;
	case EMcpWeatherType::Cloudy:
		TargetWeather.CloudCoverage = 0.6f;
		TargetWeather.Intensity = 0.0f;
		TargetWeather.Visibility = 0.9f;
		break;
	case EMcpWeatherType::Overcast:
		TargetWeather.CloudCoverage = 0.9f;
		TargetWeather.Intensity = 0.0f;
		TargetWeather.Visibility = 0.8f;
		break;
	case EMcpWeatherType::Rain:
		TargetWeather.CloudCoverage = 0.8f;
		TargetWeather.Intensity = 0.5f;
		TargetWeather.Visibility = 0.6f;
		break;
	case EMcpWeatherType::Storm:
		TargetWeather.CloudCoverage = 1.0f;
		TargetWeather.Intensity = 1.0f;
		TargetWeather.Visibility = 0.3f;
		break;
	case EMcpWeatherType::Snow:
		TargetWeather.CloudCoverage = 0.8f;
		TargetWeather.Intensity = 0.5f;
		TargetWeather.Visibility = 0.5f;
		break;
	case EMcpWeatherType::Blizzard:
		TargetWeather.CloudCoverage = 1.0f;
		TargetWeather.Intensity = 1.0f;
		TargetWeather.Visibility = 0.1f;
		break;
	case EMcpWeatherType::Fog:
		TargetWeather.CloudCoverage = 0.5f;
		TargetWeather.Intensity = 0.7f;
		TargetWeather.Visibility = 0.2f;
		break;
	case EMcpWeatherType::Sandstorm:
		TargetWeather.CloudCoverage = 0.3f;
		TargetWeather.Intensity = 0.8f;
		TargetWeather.Visibility = 0.15f;
		break;
	}

	WeatherTransitionProgress = 0.0f;
	WeatherTransitionDuration = TransitionTime;

	OnWeatherChanged.Broadcast(OldWeather, NewWeather);
}

void UMcpWeatherSubsystem::SetWeatherIntensity(float Intensity)
{
	CurrentWeather.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	TargetWeather.Intensity = CurrentWeather.Intensity;
}

FMcpWeatherState UMcpWeatherSubsystem::GetCurrentWeather() const
{
	return CurrentWeather;
}

EMcpWeatherType UMcpWeatherSubsystem::GetWeatherType() const
{
	return CurrentWeather.WeatherType;
}

float UMcpWeatherSubsystem::GetTemperature() const
{
	return CurrentWeather.Temperature;
}

void UMcpWeatherSubsystem::SetSeason(EMcpSeason NewSeason)
{
	if (NewSeason != CurrentTime.Season)
	{
		CurrentTime.Season = NewSeason;

		// Adjust temperature based on season
		switch (NewSeason)
		{
		case EMcpSeason::Spring:
			CurrentWeather.Temperature = 15.0f;
			break;
		case EMcpSeason::Summer:
			CurrentWeather.Temperature = 25.0f;
			break;
		case EMcpSeason::Autumn:
			CurrentWeather.Temperature = 12.0f;
			break;
		case EMcpSeason::Winter:
			CurrentWeather.Temperature = -5.0f;
			break;
		}

		OnSeasonChanged.Broadcast(NewSeason);
	}
}

EMcpSeason UMcpWeatherSubsystem::GetCurrentSeason() const
{
	return CurrentTime.Season;
}

void UMcpWeatherSubsystem::UpdateTime(float DeltaTime)
{
	if (bTimePaused)
	{
		return;
	}

	// Convert real time to game time
	float GameMinutesPerSecond = 1440.0f / CurrentTime.DayLength; // Minutes in a day / seconds per day
	float MinutesToAdd = DeltaTime * GameMinutesPerSecond * TimeScale;

	int32 OldHour = CurrentTime.Hour;
	bool bWasDaytime = CurrentTime.bIsDaytime;

	CurrentTime.Minute += FMath::FloorToInt(MinutesToAdd);

	while (CurrentTime.Minute >= 60)
	{
		CurrentTime.Minute -= 60;
		CurrentTime.Hour++;

		if (CurrentTime.Hour >= 24)
		{
			CurrentTime.Hour = 0;
			CurrentTime.Day++;

			if (CurrentTime.Day > 30)
			{
				CurrentTime.Day = 1;
				CurrentTime.Month++;

				if (CurrentTime.Month > 12)
				{
					CurrentTime.Month = 1;
					CurrentTime.Year++;
				}

				UpdateSeasonFromDate();
			}
		}

		if (CurrentTime.Hour != OldHour)
		{
			OnHourChanged.Broadcast(CurrentTime.Hour);
			OldHour = CurrentTime.Hour;
		}
	}

	CurrentTime.NormalizedTime = (CurrentTime.Hour * 60 + CurrentTime.Minute) / 1440.0f;
	CurrentTime.bIsDaytime = (CurrentTime.Hour >= 6 && CurrentTime.Hour < 20);

	if (CurrentTime.bIsDaytime != bWasDaytime)
	{
		OnDayNightChanged.Broadcast(CurrentTime.bIsDaytime);
	}
}

void UMcpWeatherSubsystem::UpdateWeatherTransition(float DeltaTime)
{
	if (WeatherTransitionProgress >= 1.0f)
	{
		return;
	}

	if (WeatherTransitionDuration <= 0.0f)
	{
		CurrentWeather = TargetWeather;
		WeatherTransitionProgress = 1.0f;
		return;
	}

	WeatherTransitionProgress += DeltaTime / WeatherTransitionDuration;
	WeatherTransitionProgress = FMath::Min(WeatherTransitionProgress, 1.0f);

	// Interpolate weather values
	float Alpha = WeatherTransitionProgress;
	CurrentWeather.Intensity = FMath::Lerp(CurrentWeather.Intensity, TargetWeather.Intensity, Alpha);
	CurrentWeather.CloudCoverage = FMath::Lerp(CurrentWeather.CloudCoverage, TargetWeather.CloudCoverage, Alpha);
	CurrentWeather.Visibility = FMath::Lerp(CurrentWeather.Visibility, TargetWeather.Visibility, Alpha);
	CurrentWeather.WindSpeed = FMath::Lerp(CurrentWeather.WindSpeed, TargetWeather.WindSpeed, Alpha);

	if (WeatherTransitionProgress >= 1.0f)
	{
		CurrentWeather.WeatherType = TargetWeather.WeatherType;
	}

	ApplyWeatherEffects();
}

void UMcpWeatherSubsystem::UpdateSeasonFromDate()
{
	EMcpSeason OldSeason = CurrentTime.Season;

	if (CurrentTime.Month >= 3 && CurrentTime.Month <= 5)
	{
		CurrentTime.Season = EMcpSeason::Spring;
	}
	else if (CurrentTime.Month >= 6 && CurrentTime.Month <= 8)
	{
		CurrentTime.Season = EMcpSeason::Summer;
	}
	else if (CurrentTime.Month >= 9 && CurrentTime.Month <= 11)
	{
		CurrentTime.Season = EMcpSeason::Autumn;
	}
	else
	{
		CurrentTime.Season = EMcpSeason::Winter;
	}

	if (CurrentTime.Season != OldSeason)
	{
		OnSeasonChanged.Broadcast(CurrentTime.Season);
	}
}

void UMcpWeatherSubsystem::ApplyWeatherEffects()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	// Find and update Exponential Height Fog
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		AExponentialHeightFog* FogActor = *It;
		UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
		if (FogComponent)
		{
			// Adjust fog density based on weather visibility
			float BaseFogDensity = 0.02f;
			float TargetDensity = BaseFogDensity / FMath::Max(CurrentWeather.Visibility, 0.1f);
			
			FogComponent->SetFogDensity(FMath::Lerp(FogComponent->FogDensity, TargetDensity, 0.05f));
			
			// Adjust fog color based on weather type
			FLinearColor FogColor;
			switch (CurrentWeather.WeatherType)
			{
			case EMcpWeatherType::Storm:
			case EMcpWeatherType::Blizzard:
				FogColor = FLinearColor(0.3f, 0.3f, 0.35f);
				break;
			case EMcpWeatherType::Fog:
				FogColor = FLinearColor(0.7f, 0.7f, 0.75f);
				break;
			case EMcpWeatherType::Sandstorm:
				FogColor = FLinearColor(0.8f, 0.6f, 0.4f);
				break;
			default:
				FogColor = FLinearColor(0.5f, 0.6f, 0.7f);
				break;
			}
			FogComponent->SetFogInscatteringColor(FogColor);
		}
	}
	
	// Find and update Directional Light (Sun)
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* SunLight = *It;
		UDirectionalLightComponent* LightComponent = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent());
		if (LightComponent)
		{
			// Adjust light intensity based on cloud coverage
			float BaseIntensity = 10.0f;
			float TargetIntensity = BaseIntensity * (1.0f - CurrentWeather.CloudCoverage * 0.7f);
			
			LightComponent->SetIntensity(FMath::Lerp(LightComponent->Intensity, TargetIntensity, 0.05f));
			
			// Adjust light color based on time of day and weather
			FLinearColor LightColor;
			if (CurrentTime.bIsDaytime)
			{
				float SunAngle = GetSunAngle();
				if (SunAngle < 30.0f || SunAngle > 150.0f)
				{
					// Sunrise/sunset - warm colors
					LightColor = FLinearColor(1.0f, 0.7f, 0.5f);
				}
				else
				{
					// Midday
					LightColor = FLinearColor(1.0f, 0.95f, 0.9f);
				}
			}
			else
			{
				// Night - very dim blue
				LightColor = FLinearColor(0.2f, 0.2f, 0.4f);
			}
			
			// Desaturate light during storms
			if (CurrentWeather.WeatherType == EMcpWeatherType::Storm || 
				CurrentWeather.WeatherType == EMcpWeatherType::Blizzard)
			{
				LightColor = FMath::Lerp(LightColor, FLinearColor(0.5f, 0.5f, 0.5f), 0.5f);
			}
			
			LightComponent->SetLightColor(LightColor);
		}
	}
	
	// Update sky light for ambient lighting
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		ASkyLight* SkyLightActor = *It;
		USkyLightComponent* SkyLightComp = SkyLightActor->GetLightComponent();
		if (SkyLightComp)
		{
			// Adjust sky light intensity based on time and weather
			float BaseIntensity = CurrentTime.bIsDaytime ? 1.0f : 0.1f;
			float WeatherModifier = 1.0f - CurrentWeather.CloudCoverage * 0.5f;
			SkyLightComp->SetIntensity(BaseIntensity * WeatherModifier);
		}
	}
	
	// Manage weather particle systems
	if (WeatherParticleComponent)
	{
		bool bShouldHaveParticles = (CurrentWeather.WeatherType == EMcpWeatherType::Rain ||
									 CurrentWeather.WeatherType == EMcpWeatherType::Storm ||
									 CurrentWeather.WeatherType == EMcpWeatherType::Snow ||
									 CurrentWeather.WeatherType == EMcpWeatherType::Blizzard ||
									 CurrentWeather.WeatherType == EMcpWeatherType::Sandstorm);
		
		if (bShouldHaveParticles && CurrentWeather.Intensity > 0.0f)
		{
			// Set particle spawn rate based on intensity
			WeatherParticleComponent->SetFloatParameter(FName(TEXT("SpawnRate")), CurrentWeather.Intensity * 1000.0f);
			WeatherParticleComponent->SetFloatParameter(FName(TEXT("WindStrength")), CurrentWeather.WindSpeed);
			WeatherParticleComponent->SetVectorParameter(FName(TEXT("WindDirection")), CurrentWeather.WindDirection);
			
			if (!WeatherParticleComponent->IsActive())
			{
				WeatherParticleComponent->Activate();
			}
		}
		else
		{
			if (WeatherParticleComponent->IsActive())
			{
				WeatherParticleComponent->Deactivate();
			}
		}
	}
	
	// Update weather audio
	if (WeatherAudioComponent && WeatherAmbientSound)
	{
		bool bShouldPlayWeatherAudio = (CurrentWeather.WeatherType == EMcpWeatherType::Rain ||
										CurrentWeather.WeatherType == EMcpWeatherType::Storm ||
										CurrentWeather.WeatherType == EMcpWeatherType::Blizzard);
		
		if (bShouldPlayWeatherAudio && CurrentWeather.Intensity > 0.1f)
		{
			float VolumeMultiplier = CurrentWeather.Intensity;
			WeatherAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
			
			if (!WeatherAudioComponent->IsPlaying())
			{
				WeatherAudioComponent->SetSound(WeatherAmbientSound);
				WeatherAudioComponent->Play();
			}
		}
		else
		{
			if (WeatherAudioComponent->IsPlaying())
			{
				WeatherAudioComponent->FadeOut(2.0f, 0.0f);
			}
		}
	}
	
	// Apply wind effects to foliage
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		TArray<UHierarchicalInstancedStaticMeshComponent*> FoliageComps;
		It->GetComponents<UHierarchicalInstancedStaticMeshComponent>(FoliageComps);
		
		for (UHierarchicalInstancedStaticMeshComponent* FoliageComp : FoliageComps)
		{
			if (FoliageComp)
			{
				// Wind intensity affects foliage sway through material parameters
				UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(FoliageComp->GetMaterial(0));
				if (DynMaterial)
				{
					DynMaterial->SetScalarParameterValue(FName(TEXT("WindIntensity")), CurrentWeather.WindSpeed / 20.0f);
					DynMaterial->SetVectorParameterValue(FName(TEXT("WindDirection")), FLinearColor(CurrentWeather.WindDirection));
				}
			}
		}
	}
}

// ============================================================================
// WORLD GENERATION SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UMcpWorldGenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Default settings
	WorldSettings.WorldSeed = 12345;
	WorldSettings.ChunkSize = 10000.0f;
	WorldSettings.ViewDistance = 3;
	WorldSettings.TerrainScale = 1.0f;
	WorldSettings.TerrainOctaves = 6;
	WorldSettings.TerrainPersistence = 0.5f;
	WorldSettings.BiomeScale = 0.001f;
	WorldSettings.VegetationDensity = 0.5f;
	WorldSettings.StructureDensity = 0.1f;

	CurrentPlayerChunk = FIntVector::ZeroValue;
	CurrentPlayerBiome = EMcpBiomeType::Plains;
}

void UMcpWorldGenSubsystem::Deinitialize()
{
	// Unload all chunks
	TArray<FIntVector> ChunksToUnload;
	for (const auto& Pair : LoadedChunks)
	{
		ChunksToUnload.Add(Pair.Key);
	}

	for (const FIntVector& Coord : ChunksToUnload)
	{
		UnloadChunk(Coord);
	}

	LoadedChunks.Empty();
	Super::Deinitialize();
}

void UMcpWorldGenSubsystem::Tick(float DeltaTime)
{
	StreamChunksAroundPlayer();
}

TStatId UMcpWorldGenSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMcpWorldGenSubsystem, STATGROUP_Tickables);
}

void UMcpWorldGenSubsystem::InitializeWorld(const FMcpWorldGenSettings& Settings)
{
	WorldSettings = Settings;
}

void UMcpWorldGenSubsystem::SetWorldSeed(int32 Seed)
{
	WorldSettings.WorldSeed = Seed;
}

FMcpWorldGenSettings UMcpWorldGenSubsystem::GetWorldSettings() const
{
	return WorldSettings;
}

void UMcpWorldGenSubsystem::UpdatePlayerPosition(FVector PlayerPosition)
{
	FIntVector NewChunk = WorldToChunkCoord(PlayerPosition);

	if (NewChunk != CurrentPlayerChunk)
	{
		CurrentPlayerChunk = NewChunk;
		StreamChunksAroundPlayer();
	}

	EMcpBiomeType NewBiome = GetBiomeAtLocation(PlayerPosition);
	if (NewBiome != CurrentPlayerBiome)
	{
		EMcpBiomeType OldBiome = CurrentPlayerBiome;
		CurrentPlayerBiome = NewBiome;
		OnBiomeEntered.Broadcast(OldBiome, NewBiome);
	}
}

bool UMcpWorldGenSubsystem::LoadChunk(FIntVector ChunkCoord)
{
	if (LoadedChunks.Contains(ChunkCoord))
	{
		return true;
	}

	FMcpWorldChunk Chunk;
	Chunk.ChunkCoord = ChunkCoord;
	Chunk.Biome = CalculateBiome(ChunkCoord);
	Chunk.bIsLoaded = true;
	Chunk.bIsGenerated = false;
	Chunk.GenerationSeed = WorldSettings.WorldSeed + ChunkCoord.X * 1000 + ChunkCoord.Y;
	Chunk.LODLevel = 0;

	LoadedChunks.Add(ChunkCoord, Chunk);
	OnChunkLoaded.Broadcast(ChunkCoord);

	// Generate chunk content
	GenerateChunk(ChunkCoord);

	return true;
}

bool UMcpWorldGenSubsystem::UnloadChunk(FIntVector ChunkCoord)
{
	FMcpWorldChunk* Chunk = LoadedChunks.Find(ChunkCoord);
	if (!Chunk)
	{
		return false;
	}

	// Destroy chunk actors
	for (AActor* Actor : Chunk->ChunkActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	Chunk->ChunkActors.Empty();

	LoadedChunks.Remove(ChunkCoord);
	OnChunkUnloaded.Broadcast(ChunkCoord);

	return true;
}

bool UMcpWorldGenSubsystem::GenerateChunk(FIntVector ChunkCoord)
{
	FMcpWorldChunk* Chunk = LoadedChunks.Find(ChunkCoord);
	if (!Chunk || Chunk->bIsGenerated)
	{
		return false;
	}

	GenerateChunkTerrain(*Chunk);
	GenerateChunkVegetation(*Chunk);
	GenerateChunkStructures(*Chunk);

	Chunk->bIsGenerated = true;
	return true;
}

FMcpWorldChunk UMcpWorldGenSubsystem::GetChunk(FIntVector ChunkCoord) const
{
	const FMcpWorldChunk* Chunk = LoadedChunks.Find(ChunkCoord);
	return Chunk ? *Chunk : FMcpWorldChunk();
}

bool UMcpWorldGenSubsystem::IsChunkLoaded(FIntVector ChunkCoord) const
{
	return LoadedChunks.Contains(ChunkCoord);
}

TArray<FIntVector> UMcpWorldGenSubsystem::GetLoadedChunks() const
{
	TArray<FIntVector> Result;
	for (const auto& Pair : LoadedChunks)
	{
		Result.Add(Pair.Key);
	}
	return Result;
}

EMcpBiomeType UMcpWorldGenSubsystem::GetBiomeAtLocation(FVector WorldLocation) const
{
	FIntVector ChunkCoord = WorldToChunkCoord(WorldLocation);
	return CalculateBiome(ChunkCoord);
}

float UMcpWorldGenSubsystem::GetHeightAtLocation(FVector2D WorldLocation) const
{
	return SampleNoise(WorldLocation, WorldSettings.TerrainScale, WorldSettings.TerrainOctaves, WorldSettings.TerrainPersistence);
}

float UMcpWorldGenSubsystem::SampleNoise(FVector2D Location, float Scale, int32 Octaves, float Persistence) const
{
	float Total = 0.0f;
	float Frequency = Scale;
	float Amplitude = 1.0f;
	float MaxValue = 0.0f;

	for (int32 i = 0; i < Octaves; i++)
	{
		// Simple noise approximation using sin
		float NoiseValue = FMath::Sin(Location.X * Frequency + WorldSettings.WorldSeed)
		                 * FMath::Cos(Location.Y * Frequency + WorldSettings.WorldSeed);
		Total += NoiseValue * Amplitude;
		MaxValue += Amplitude;
		Amplitude *= Persistence;
		Frequency *= 2.0f;
	}

	return Total / MaxValue;
}

FVector2D UMcpWorldGenSubsystem::SampleNoise2D(FVector2D Location, float Scale) const
{
	float X = SampleNoise(Location, Scale, 4, 0.5f);
	float Y = SampleNoise(FVector2D(Location.Y, Location.X), Scale, 4, 0.5f);
	return FVector2D(X, Y);
}

FIntVector UMcpWorldGenSubsystem::WorldToChunkCoord(FVector WorldLocation) const
{
	return FIntVector(
		FMath::FloorToInt(WorldLocation.X / WorldSettings.ChunkSize),
		FMath::FloorToInt(WorldLocation.Y / WorldSettings.ChunkSize),
		0
	);
}

void UMcpWorldGenSubsystem::StreamChunksAroundPlayer()
{
	// Unload distant chunks
	TArray<FIntVector> ChunksToUnload;
	for (const auto& Pair : LoadedChunks)
	{
		int32 Distance = FMath::Max(
			FMath::Abs(Pair.Key.X - CurrentPlayerChunk.X),
			FMath::Abs(Pair.Key.Y - CurrentPlayerChunk.Y)
		);

		if (Distance > WorldSettings.ViewDistance)
		{
			ChunksToUnload.Add(Pair.Key);
		}
	}

	for (const FIntVector& Coord : ChunksToUnload)
	{
		UnloadChunk(Coord);
	}

	// Load nearby chunks
	for (int32 X = -WorldSettings.ViewDistance; X <= WorldSettings.ViewDistance; X++)
	{
		for (int32 Y = -WorldSettings.ViewDistance; Y <= WorldSettings.ViewDistance; Y++)
		{
			FIntVector ChunkCoord = CurrentPlayerChunk + FIntVector(X, Y, 0);
			if (!IsChunkLoaded(ChunkCoord))
			{
				LoadChunk(ChunkCoord);
			}
		}
	}
}

void UMcpWorldGenSubsystem::GenerateChunkTerrain(FMcpWorldChunk& Chunk)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	// Calculate chunk world position
	FVector ChunkOrigin(Chunk.ChunkCoord.X * WorldSettings.ChunkSize, 
						Chunk.ChunkCoord.Y * WorldSettings.ChunkSize, 0.0f);
	
	// Create procedural mesh actor for terrain
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AActor* TerrainActor = World->SpawnActor<AActor>(AActor::StaticClass(), ChunkOrigin, FRotator::ZeroRotator, SpawnParams);
	if (!TerrainActor)
	{
		return;
	}
	
	UProceduralMeshComponent* ProceduralMesh = NewObject<UProceduralMeshComponent>(TerrainActor);
	if (!ProceduralMesh)
	{
		TerrainActor->Destroy();
		return;
	}
	
	ProceduralMesh->SetupAttachment(TerrainActor->GetRootComponent());
	ProceduralMesh->RegisterComponent();
	
	// Generate terrain mesh vertices
	const int32 Resolution = 64; // Vertices per side
	const float CellSize = WorldSettings.ChunkSize / Resolution;
	
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	
	Vertices.Reserve((Resolution + 1) * (Resolution + 1));
	
	// Generate vertices with height from noise
	for (int32 Y = 0; Y <= Resolution; ++Y)
	{
		for (int32 X = 0; X <= Resolution; ++X)
		{
			FVector2D WorldPos2D(ChunkOrigin.X + X * CellSize, ChunkOrigin.Y + Y * CellSize);
			
			// Sample height from multiple octaves of noise
			float Height = GetHeightAtLocation(WorldPos2D) * WorldSettings.TerrainScale * 1000.0f;
			
			// Add biome-specific height modifications
			switch (Chunk.Biome)
			{
			case EMcpBiomeType::Mountain:
				Height *= 2.5f;
				break;
			case EMcpBiomeType::Plains:
				Height *= 0.3f;
				break;
			case EMcpBiomeType::Ocean:
				Height = FMath::Min(Height, -100.0f);
				break;
			case EMcpBiomeType::Desert:
				// Add dune-like patterns
				Height *= 0.6f;
				Height += FMath::Sin(WorldPos2D.X * 0.01f) * FMath::Sin(WorldPos2D.Y * 0.015f) * 50.0f;
				break;
			default:
				break;
			}
			
			Vertices.Add(FVector(X * CellSize, Y * CellSize, Height));
			UVs.Add(FVector2D(X / (float)Resolution, Y / (float)Resolution));
			
			// Set vertex color based on biome
			FLinearColor VertexColor = GetBiomeColor(Chunk.Biome, Height);
			VertexColors.Add(VertexColor);
		}
	}
	
	// Generate triangle indices
	Triangles.Reserve(Resolution * Resolution * 6);
	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			int32 BottomLeft = Y * (Resolution + 1) + X;
			int32 BottomRight = BottomLeft + 1;
			int32 TopLeft = BottomLeft + (Resolution + 1);
			int32 TopRight = TopLeft + 1;
			
			// First triangle
			Triangles.Add(BottomLeft);
			Triangles.Add(TopLeft);
			Triangles.Add(TopRight);
			
			// Second triangle
			Triangles.Add(BottomLeft);
			Triangles.Add(TopRight);
			Triangles.Add(BottomRight);
		}
	}
	
	// Calculate normals
	Normals.SetNum(Vertices.Num());
	for (int32 i = 0; i < Triangles.Num(); i += 3)
	{
		FVector V0 = Vertices[Triangles[i]];
		FVector V1 = Vertices[Triangles[i + 1]];
		FVector V2 = Vertices[Triangles[i + 2]];
		
		FVector Edge1 = V1 - V0;
		FVector Edge2 = V2 - V0;
		FVector Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
		
		Normals[Triangles[i]] += Normal;
		Normals[Triangles[i + 1]] += Normal;
		Normals[Triangles[i + 2]] += Normal;
	}
	
	for (FVector& Normal : Normals)
	{
		Normal.Normalize();
	}
	
	// Create the mesh section
	ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, TArray<FProcMeshTangent>(), true);
	
	// Enable collision
	ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProceduralMesh->SetCollisionProfileName(TEXT("BlockAll"));
	
	// Assign biome-appropriate material
	UMaterialInterface* TerrainMaterial = GetBiomeMaterial(Chunk.Biome);
	if (TerrainMaterial)
	{
		ProceduralMesh->SetMaterial(0, TerrainMaterial);
	}
	
	Chunk.ChunkActors.Add(TerrainActor);
}

void UMcpWorldGenSubsystem::GenerateChunkVegetation(FMcpWorldChunk& Chunk)
{
	UWorld* World = GetWorld();
	if (!World || WorldSettings.VegetationDensity <= 0.0f)
	{
		return;
	}
	
	// Calculate chunk world position
	FVector ChunkOrigin(Chunk.ChunkCoord.X * WorldSettings.ChunkSize, 
						Chunk.ChunkCoord.Y * WorldSettings.ChunkSize, 0.0f);
	
	// Get vegetation assets for this biome
	TArray<UStaticMesh*> VegetationMeshes = GetBiomeVegetation(Chunk.Biome);
	if (VegetationMeshes.Num() == 0)
	{
		return;
	}
	
	// Create HISM component for instanced vegetation
	AActor* VegetationActor = World->SpawnActor<AActor>(AActor::StaticClass(), ChunkOrigin, FRotator::ZeroRotator);
	if (!VegetationActor)
	{
		return;
	}
	
	// Create HISM for each vegetation type
	FRandomStream RandomStream(Chunk.GenerationSeed);
	
	for (UStaticMesh* VegMesh : VegetationMeshes)
	{
		if (!VegMesh)
		{
			continue;
		}
		
		UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(VegetationActor);
		HISM->SetStaticMesh(VegMesh);
		HISM->SetupAttachment(VegetationActor->GetRootComponent());
		HISM->RegisterComponent();
		
		// Calculate number of instances based on density
		int32 NumInstances = FMath::FloorToInt(WorldSettings.VegetationDensity * 100.0f);
		
		// Biome-specific density modifiers
		switch (Chunk.Biome)
		{
		case EMcpBiomeType::Forest:
		case EMcpBiomeType::Jungle:
			NumInstances = FMath::FloorToInt(NumInstances * 2.0f);
			break;
		case EMcpBiomeType::Desert:
		case EMcpBiomeType::Arctic:
		case EMcpBiomeType::Tundra:
			NumInstances = FMath::FloorToInt(NumInstances * 0.2f);
			break;
		case EMcpBiomeType::Ocean:
		case EMcpBiomeType::Volcanic:
			NumInstances = 0;
			break;
		default:
			break;
		}
		
		// Place instances using Poisson disk sampling approximation
		for (int32 i = 0; i < NumInstances; ++i)
		{
			FVector2D RandomOffset(
				RandomStream.FRandRange(0.0f, WorldSettings.ChunkSize),
				RandomStream.FRandRange(0.0f, WorldSettings.ChunkSize)
			);
			
			FVector2D WorldPos2D = FVector2D(ChunkOrigin.X, ChunkOrigin.Y) + RandomOffset;
			
			// Get terrain height at this position
			float TerrainHeight = GetHeightAtLocation(WorldPos2D) * WorldSettings.TerrainScale * 1000.0f;
			
			// Skip if underwater or on steep slopes
			if (TerrainHeight < 0.0f)
			{
				continue;
			}
			
			// Create transform for this instance
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(FVector(RandomOffset.X, RandomOffset.Y, TerrainHeight));
			
			// Random rotation (mostly Y-axis for upright vegetation)
			FRotator Rotation(
				RandomStream.FRandRange(-5.0f, 5.0f),  // Small pitch variation
				RandomStream.FRandRange(0.0f, 360.0f), // Full yaw rotation
				RandomStream.FRandRange(-5.0f, 5.0f)   // Small roll variation
			);
			InstanceTransform.SetRotation(Rotation.Quaternion());
			
			// Random scale variation
			float ScaleVariation = RandomStream.FRandRange(0.8f, 1.2f);
			InstanceTransform.SetScale3D(FVector(ScaleVariation));
			
			HISM->AddInstance(InstanceTransform);
		}
	}
	
	Chunk.ChunkActors.Add(VegetationActor);
}

void UMcpWorldGenSubsystem::GenerateChunkStructures(FMcpWorldChunk& Chunk)
{
	UWorld* World = GetWorld();
	if (!World || WorldSettings.StructureDensity <= 0.0f)
	{
		return;
	}
	
	// Calculate chunk world position
	FVector ChunkOrigin(Chunk.ChunkCoord.X * WorldSettings.ChunkSize, 
						Chunk.ChunkCoord.Y * WorldSettings.ChunkSize, 0.0f);
	
	FRandomStream RandomStream(Chunk.GenerationSeed + 1000); // Different seed offset for structures
	
	// Determine if this chunk should have structures (based on density)
	float StructureChance = WorldSettings.StructureDensity;
	
	// Adjust chance based on biome
	switch (Chunk.Biome)
	{
	case EMcpBiomeType::Plains:
		StructureChance *= 1.5f; // More structures in plains
		break;
	case EMcpBiomeType::Forest:
		StructureChance *= 0.8f; // Some structures in forests
		break;
	case EMcpBiomeType::Desert:
		StructureChance *= 0.4f; // Rare desert ruins
		break;
	case EMcpBiomeType::Mountain:
		StructureChance *= 0.3f; // Rare mountain fortresses
		break;
	case EMcpBiomeType::Ocean:
		StructureChance = 0.0f; // No underwater structures
		break;
	default:
		StructureChance *= 0.5f;
		break;
	}
	
	// Roll for structure placement
	if (RandomStream.FRand() > StructureChance)
	{
		return; // No structure in this chunk
	}
	
	// Get structure classes for this biome
	TArray<TSubclassOf<AActor>> StructureClasses = GetBiomeStructures(Chunk.Biome);
	if (StructureClasses.Num() == 0)
	{
		return;
	}
	
	// Select a random structure type
	int32 StructureIndex = RandomStream.RandRange(0, StructureClasses.Num() - 1);
	TSubclassOf<AActor> StructureClass = StructureClasses[StructureIndex];
	
	if (!StructureClass)
	{
		return;
	}
	
	// Find a suitable spawn location within the chunk
	int32 MaxAttempts = 10;
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		FVector2D RandomOffset(
			RandomStream.FRandRange(WorldSettings.ChunkSize * 0.2f, WorldSettings.ChunkSize * 0.8f),
			RandomStream.FRandRange(WorldSettings.ChunkSize * 0.2f, WorldSettings.ChunkSize * 0.8f)
		);
		
		FVector2D WorldPos2D = FVector2D(ChunkOrigin.X, ChunkOrigin.Y) + RandomOffset;
		
		// Get terrain height at this position
		float TerrainHeight = GetHeightAtLocation(WorldPos2D) * WorldSettings.TerrainScale * 1000.0f;
		
		// Skip underwater locations
		if (TerrainHeight < 10.0f)
		{
			continue;
		}
		
		// Check slope (structures need relatively flat ground)
		float HeightLeft = GetHeightAtLocation(WorldPos2D + FVector2D(-50, 0)) * WorldSettings.TerrainScale * 1000.0f;
		float HeightRight = GetHeightAtLocation(WorldPos2D + FVector2D(50, 0)) * WorldSettings.TerrainScale * 1000.0f;
		float HeightFront = GetHeightAtLocation(WorldPos2D + FVector2D(0, 50)) * WorldSettings.TerrainScale * 1000.0f;
		float HeightBack = GetHeightAtLocation(WorldPos2D + FVector2D(0, -50)) * WorldSettings.TerrainScale * 1000.0f;
		
		float MaxSlope = FMath::Max(
			FMath::Abs(HeightLeft - HeightRight),
			FMath::Abs(HeightFront - HeightBack)
		);
		
		// Skip if slope is too steep
		if (MaxSlope > 50.0f)
		{
			continue;
		}
		
		// Spawn the structure
		FVector SpawnLocation(ChunkOrigin.X + RandomOffset.X, ChunkOrigin.Y + RandomOffset.Y, TerrainHeight);
		FRotator SpawnRotation(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		
		AActor* Structure = World->SpawnActor<AActor>(StructureClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Structure)
		{
			Chunk.ChunkActors.Add(Structure);
			
			// Broadcast structure spawned event
			OnStructureSpawned.Broadcast(Chunk.ChunkCoord, Structure);
		}
		
		break; // Successfully placed structure
	}
}

EMcpBiomeType UMcpWorldGenSubsystem::CalculateBiome(FIntVector ChunkCoord) const
{
	// Sample temperature and moisture from noise
	FVector2D ChunkCenter(ChunkCoord.X * WorldSettings.ChunkSize, ChunkCoord.Y * WorldSettings.ChunkSize);

	float Temperature = SampleNoise(ChunkCenter * WorldSettings.BiomeScale, 0.5f, 3, 0.5f);
	float Moisture = SampleNoise(ChunkCenter * WorldSettings.BiomeScale * 1.5f + FVector2D(1000, 1000), 0.5f, 3, 0.5f);

	// Normalize to 0-1
	Temperature = (Temperature + 1.0f) * 0.5f;
	Moisture = (Moisture + 1.0f) * 0.5f;

	// Biome selection based on temperature and moisture
	if (Temperature < 0.2f)
	{
		return Moisture > 0.5f ? EMcpBiomeType::Arctic : EMcpBiomeType::Tundra;
	}
	else if (Temperature < 0.4f)
	{
		return Moisture > 0.6f ? EMcpBiomeType::Swamp : EMcpBiomeType::Forest;
	}
	else if (Temperature < 0.6f)
	{
		return Moisture > 0.5f ? EMcpBiomeType::Forest : EMcpBiomeType::Plains;
	}
	else if (Temperature < 0.8f)
	{
		return Moisture > 0.7f ? EMcpBiomeType::Jungle : (Moisture > 0.3f ? EMcpBiomeType::Plains : EMcpBiomeType::Desert);
	}
	else
	{
		return Moisture > 0.5f ? EMcpBiomeType::Volcanic : EMcpBiomeType::Desert;
	}
}

FLinearColor UMcpWorldGenSubsystem::GetBiomeColor(EMcpBiomeType Biome, float Height) const
{
	// Height-based coloring with biome tint
	FLinearColor BaseColor;
	
	switch (Biome)
	{
	case EMcpBiomeType::Forest:
		BaseColor = FLinearColor(0.2f, 0.5f, 0.2f); // Green
		break;
	case EMcpBiomeType::Desert:
		BaseColor = FLinearColor(0.9f, 0.8f, 0.5f); // Sandy
		break;
	case EMcpBiomeType::Tundra:
		BaseColor = FLinearColor(0.7f, 0.75f, 0.8f); // Pale blue-grey
		break;
	case EMcpBiomeType::Arctic:
		BaseColor = FLinearColor(0.95f, 0.95f, 1.0f); // Snow white
		break;
	case EMcpBiomeType::Jungle:
		BaseColor = FLinearColor(0.1f, 0.4f, 0.15f); // Dark green
		break;
	case EMcpBiomeType::Swamp:
		BaseColor = FLinearColor(0.3f, 0.35f, 0.2f); // Murky green
		break;
	case EMcpBiomeType::Mountain:
		BaseColor = FLinearColor(0.5f, 0.5f, 0.55f); // Rocky grey
		break;
	case EMcpBiomeType::Plains:
		BaseColor = FLinearColor(0.5f, 0.6f, 0.3f); // Yellow-green
		break;
	case EMcpBiomeType::Ocean:
		BaseColor = FLinearColor(0.1f, 0.3f, 0.6f); // Blue
		break;
	case EMcpBiomeType::Volcanic:
		BaseColor = FLinearColor(0.3f, 0.2f, 0.15f); // Dark brown
		break;
	default:
		BaseColor = FLinearColor(0.4f, 0.4f, 0.4f);
		break;
	}
	
	// Add height-based variation
	float HeightFactor = FMath::Clamp(Height / 1000.0f, -1.0f, 1.0f);
	
	// Higher elevations get lighter/snowier
	if (HeightFactor > 0.5f)
	{
		float SnowBlend = (HeightFactor - 0.5f) * 2.0f;
		BaseColor = FMath::Lerp(BaseColor, FLinearColor(0.9f, 0.9f, 0.95f), SnowBlend * 0.5f);
	}
	
	// Lower/underwater gets darker/bluer
	if (HeightFactor < 0.0f)
	{
		float WaterBlend = FMath::Abs(HeightFactor);
		BaseColor = FMath::Lerp(BaseColor, FLinearColor(0.1f, 0.2f, 0.4f), WaterBlend);
	}
	
	return BaseColor;
}

UMaterialInterface* UMcpWorldGenSubsystem::GetBiomeMaterial(EMcpBiomeType Biome) const
{
	// Load biome-specific material from content
	FString MaterialPath;
	
	switch (Biome)
	{
	case EMcpBiomeType::Forest:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Forest");
		break;
	case EMcpBiomeType::Desert:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Desert");
		break;
	case EMcpBiomeType::Tundra:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Tundra");
		break;
	case EMcpBiomeType::Arctic:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Arctic");
		break;
	case EMcpBiomeType::Jungle:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Jungle");
		break;
	case EMcpBiomeType::Swamp:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Swamp");
		break;
	case EMcpBiomeType::Mountain:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Mountain");
		break;
	case EMcpBiomeType::Plains:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Plains");
		break;
	case EMcpBiomeType::Ocean:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Ocean");
		break;
	case EMcpBiomeType::Volcanic:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Volcanic");
		break;
	default:
		MaterialPath = TEXT("/Game/FORGE2/Materials/Terrain/M_Terrain_Default");
		break;
	}
	
	return Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath));
}

TArray<UStaticMesh*> UMcpWorldGenSubsystem::GetBiomeVegetation(EMcpBiomeType Biome) const
{
	TArray<UStaticMesh*> VegetationMeshes;
	
	// Load biome-specific vegetation meshes
	TArray<FString> MeshPaths;
	
	switch (Biome)
	{
	case EMcpBiomeType::Forest:
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Tree_Oak"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Tree_Pine"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Bush_Forest"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Fern"));
		break;
	case EMcpBiomeType::Desert:
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Cactus_Large"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Cactus_Small"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_DesertShrub"));
		break;
	case EMcpBiomeType::Jungle:
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Tree_Palm"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Tree_Tropical"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Fern_Large"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Vine"));
		break;
	case EMcpBiomeType::Swamp:
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Tree_Willow"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Reeds"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Lily"));
		break;
	case EMcpBiomeType::Plains:
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Grass_Tall"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Flower_Wild"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Tree_Lone"));
		break;
	case EMcpBiomeType::Tundra:
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Shrub_Arctic"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Moss"));
		break;
	case EMcpBiomeType::Arctic:
		// Very sparse vegetation
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Lichen"));
		break;
	case EMcpBiomeType::Mountain:
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Tree_Spruce"));
		MeshPaths.Add(TEXT("/Game/FORGE2/Meshes/Vegetation/SM_Rock_Moss"));
		break;
	default:
		break;
	}
	
	// Load each mesh
	for (const FString& Path : MeshPaths)
	{
		UStaticMesh* Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *Path));
		if (Mesh)
		{
			VegetationMeshes.Add(Mesh);
		}
	}
	
	return VegetationMeshes;
}

TArray<TSubclassOf<AActor>> UMcpWorldGenSubsystem::GetBiomeStructures(EMcpBiomeType Biome) const
{
	TArray<TSubclassOf<AActor>> StructureClasses;
	
	// Load biome-specific structure blueprints
	TArray<FString> BlueprintPaths;
	
	switch (Biome)
	{
	case EMcpBiomeType::Forest:
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Cabin"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_WatchTower"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Camp"));
		break;
	case EMcpBiomeType::Desert:
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Pyramid"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Ruins_Desert"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Oasis"));
		break;
	case EMcpBiomeType::Plains:
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Farm"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Village"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Windmill"));
		break;
	case EMcpBiomeType::Mountain:
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Fortress"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Mine"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Shrine"));
		break;
	case EMcpBiomeType::Jungle:
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Temple"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Ruins_Jungle"));
		break;
	case EMcpBiomeType::Swamp:
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Hut_Swamp"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Dock"));
		break;
	case EMcpBiomeType::Tundra:
	case EMcpBiomeType::Arctic:
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Igloo"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Outpost"));
		break;
	case EMcpBiomeType::Volcanic:
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Forge"));
		BlueprintPaths.Add(TEXT("/Game/FORGE2/Blueprints/Structures/BP_Structure_Ruins_Volcanic"));
		break;
	default:
		break;
	}
	
	// Load each blueprint class
	for (const FString& Path : BlueprintPaths)
	{
		UClass* StructureClass = StaticLoadClass(AActor::StaticClass(), nullptr, *Path);
		if (StructureClass)
		{
			StructureClasses.Add(StructureClass);
		}
	}
	
	return StructureClasses;
}

// ============================================================================
// DIALOGUE SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UMcpDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMcpDialogueSubsystem::Deinitialize()
{
	ActiveDialogues.Empty();
	DialogueVariables.Empty();
	Super::Deinitialize();
}

bool UMcpDialogueSubsystem::StartDialogue(const FString& DialogueId, AActor* Speaker, AActor* Listener)
{
	if (ActiveDialogues.Contains(DialogueId))
	{
		return false;
	}

	FMcpActiveDialogue Dialogue;
	Dialogue.DialogueId = DialogueId;
	Dialogue.CurrentNodeId = TEXT("Start");
	Dialogue.Speaker = Speaker;
	Dialogue.Listener = Listener;
	Dialogue.bIsActive = true;

	ActiveDialogues.Add(DialogueId, Dialogue);

	// Process first node
	ProcessDialogueNode(Dialogue, TEXT("Start"));
	ActiveDialogues.Add(DialogueId, Dialogue);

	OnDialogueStarted.Broadcast(DialogueId, Speaker);

	return true;
}

bool UMcpDialogueSubsystem::AdvanceDialogue(const FString& DialogueId)
{
	FMcpActiveDialogue* Dialogue = ActiveDialogues.Find(DialogueId);
	if (!Dialogue || !Dialogue->bIsActive)
	{
		return false;
	}

	// If there are choices, player must select one
	if (Dialogue->CurrentChoices.Num() > 0)
	{
		return false;
	}

	// Move to next node (would be determined by dialogue data)
	FString NextNode = Dialogue->CurrentNodeId + TEXT("_Next");
	ProcessDialogueNode(*Dialogue, NextNode);

	return true;
}

bool UMcpDialogueSubsystem::SelectChoice(const FString& DialogueId, const FString& ChoiceId)
{
	FMcpActiveDialogue* Dialogue = ActiveDialogues.Find(DialogueId);
	if (!Dialogue || !Dialogue->bIsActive)
	{
		return false;
	}

	// Find the choice
	const FMcpDialogueChoice* SelectedChoice = nullptr;
	for (const auto& Choice : Dialogue->CurrentChoices)
	{
		if (Choice.ChoiceId == ChoiceId && Choice.bIsAvailable)
		{
			SelectedChoice = &Choice;
			break;
		}
	}

	if (!SelectedChoice)
	{
		return false;
	}

	OnDialogueChoiceMade.Broadcast(DialogueId, ChoiceId);

	// Move to the choice's target node
	ProcessDialogueNode(*Dialogue, SelectedChoice->NextNodeId);

	return true;
}

bool UMcpDialogueSubsystem::EndDialogue(const FString& DialogueId)
{
	FMcpActiveDialogue* Dialogue = ActiveDialogues.Find(DialogueId);
	if (!Dialogue)
	{
		return false;
	}

	Dialogue->bIsActive = false;
	OnDialogueEnded.Broadcast(DialogueId);
	ActiveDialogues.Remove(DialogueId);

	return true;
}

bool UMcpDialogueSubsystem::SkipToNode(const FString& DialogueId, const FString& NodeId)
{
	FMcpActiveDialogue* Dialogue = ActiveDialogues.Find(DialogueId);
	if (!Dialogue || !Dialogue->bIsActive)
	{
		return false;
	}

	ProcessDialogueNode(*Dialogue, NodeId);
	return true;
}

FMcpActiveDialogue UMcpDialogueSubsystem::GetActiveDialogue(const FString& DialogueId) const
{
	const FMcpActiveDialogue* Dialogue = ActiveDialogues.Find(DialogueId);
	return Dialogue ? *Dialogue : FMcpActiveDialogue();
}

bool UMcpDialogueSubsystem::IsDialogueActive(const FString& DialogueId) const
{
	const FMcpActiveDialogue* Dialogue = ActiveDialogues.Find(DialogueId);
	return Dialogue && Dialogue->bIsActive;
}

bool UMcpDialogueSubsystem::HasActiveDialogue() const
{
	for (const auto& Pair : ActiveDialogues)
	{
		if (Pair.Value.bIsActive)
		{
			return true;
		}
	}
	return false;
}

TArray<FMcpDialogueChoice> UMcpDialogueSubsystem::GetCurrentChoices(const FString& DialogueId) const
{
	const FMcpActiveDialogue* Dialogue = ActiveDialogues.Find(DialogueId);
	return Dialogue ? Dialogue->CurrentChoices : TArray<FMcpDialogueChoice>();
}

void UMcpDialogueSubsystem::SetDialogueVariable(const FString& VariableName, const FString& Value)
{
	DialogueVariables.Add(VariableName, Value);
}

FString UMcpDialogueSubsystem::GetDialogueVariable(const FString& VariableName) const
{
	const FString* Value = DialogueVariables.Find(VariableName);
	return Value ? *Value : TEXT("");
}

bool UMcpDialogueSubsystem::CheckDialogueCondition(const FString& ConditionType, const FString& ConditionValue) const
{
	if (ConditionType == TEXT("Variable"))
	{
		// Check if variable equals expected value
		// Format: "VariableName=ExpectedValue"
		FString VarName, ExpectedValue;
		if (ConditionValue.Split(TEXT("="), &VarName, &ExpectedValue))
		{
			return GetDialogueVariable(VarName) == ExpectedValue;
		}
	}
	// Add more condition types as needed

	return true;
}

void UMcpDialogueSubsystem::ProcessDialogueNode(FMcpActiveDialogue& Dialogue, const FString& NodeId)
{
	FString OldNode = Dialogue.CurrentNodeId;
	Dialogue.CurrentNodeId = NodeId;
	Dialogue.CurrentChoices.Empty();

	// Node processing would load from dialogue data asset
	// For now, set placeholder text
	Dialogue.CurrentText = FText::FromString(FString::Printf(TEXT("Dialogue node: %s"), *NodeId));

	OnDialogueNodeChanged.Broadcast(Dialogue.DialogueId, OldNode, NodeId);

	// Check for end node
	if (NodeId == TEXT("End") || NodeId.IsEmpty())
	{
		EndDialogue(Dialogue.DialogueId);
	}
	else
	{
		UpdateAvailableChoices(Dialogue);
	}
}

void UMcpDialogueSubsystem::ExecuteDialogueConsequence(const FString& ConsequenceType, const FString& ConsequenceValue)
{
	if (ConsequenceType == TEXT("SetVariable"))
	{
		FString VarName, VarValue;
		if (ConsequenceValue.Split(TEXT("="), &VarName, &VarValue))
		{
			SetDialogueVariable(VarName, VarValue);
		}
	}
	// Add more consequence types (give item, start quest, etc.)
}

void UMcpDialogueSubsystem::UpdateAvailableChoices(FMcpActiveDialogue& Dialogue)
{
	// Choice availability would be determined by checking conditions against dialogue data
	for (auto& Choice : Dialogue.CurrentChoices)
	{
		Choice.bIsAvailable = true;
		for (const auto& Condition : Choice.Conditions)
		{
			if (!CheckDialogueCondition(Condition.Key, Condition.Value))
			{
				Choice.bIsAvailable = false;
				break;
			}
		}
	}
}

// ============================================================================
// AUDIO SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UMcpAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentMusicState = EMcpMusicState::Exploration;
	MasterVolume = 1.0f;
	MusicVolume = 1.0f;
	SFXVolume = 1.0f;
	AmbientVolume = 1.0f;
	DialogueVolume = 1.0f;
}

void UMcpAudioSubsystem::Deinitialize()
{
	MusicLayers.Empty();
	AmbientZones.Empty();
	ActiveAmbientZones.Empty();
	Super::Deinitialize();
}

void UMcpAudioSubsystem::Tick(float DeltaTime)
{
	UpdateMusicLayers(DeltaTime);
}

TStatId UMcpAudioSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMcpAudioSubsystem, STATGROUP_Tickables);
}

void UMcpAudioSubsystem::SetMusicState(EMcpMusicState NewState, float TransitionTime)
{
	if (NewState == CurrentMusicState)
	{
		return;
	}

	EMcpMusicState OldState = CurrentMusicState;
	CurrentMusicState = NewState;

	TransitionMusicState(NewState, TransitionTime);

	OnMusicStateChanged.Broadcast(OldState, NewState);
}

EMcpMusicState UMcpAudioSubsystem::GetMusicState() const
{
	return CurrentMusicState;
}

bool UMcpAudioSubsystem::RegisterMusicLayer(const FString& LayerId, USoundBase* Sound, EMcpMusicState State)
{
	if (MusicLayers.Contains(LayerId))
	{
		return false;
	}

	FMcpMusicLayer Layer;
	Layer.LayerId = LayerId;
	Layer.Sound = Sound;
	Layer.Volume = 0.0f;
	Layer.TargetVolume = 0.0f;
	Layer.FadeSpeed = 1.0f;
	Layer.bIsPlaying = false;

	MusicLayers.Add(LayerId, Layer);
	return true;
}

void UMcpAudioSubsystem::SetLayerVolume(const FString& LayerId, float Volume, float FadeTime)
{
	FMcpMusicLayer* Layer = MusicLayers.Find(LayerId);
	if (!Layer)
	{
		return;
	}

	Layer->TargetVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	Layer->FadeSpeed = FadeTime > 0 ? 1.0f / FadeTime : 100.0f;
}

void UMcpAudioSubsystem::MuteLayer(const FString& LayerId, bool bMute)
{
	SetLayerVolume(LayerId, bMute ? 0.0f : 1.0f, 0.5f);
}

void UMcpAudioSubsystem::PlayStinger(USoundBase* StingerSound, bool bInterruptCurrent)
{
	if (!StingerSound)
	{
		return;
	}

	// Play stinger using UGameplayStatics::PlaySound2D
	UGameplayStatics::PlaySound2D(GetWorld(), StingerSound);
}

bool UMcpAudioSubsystem::RegisterAmbientZone(const FMcpAmbientZone& Zone)
{
	if (AmbientZones.Contains(Zone.ZoneId))
	{
		return false;
	}

	AmbientZones.Add(Zone.ZoneId, Zone);
	return true;
}

void UMcpAudioSubsystem::UnregisterAmbientZone(const FString& ZoneId)
{
	AmbientZones.Remove(ZoneId);
	ActiveAmbientZones.Remove(ZoneId);
}

void UMcpAudioSubsystem::UpdateListenerPosition(FVector ListenerPosition)
{
	UpdateAmbientZones(ListenerPosition);
}

void UMcpAudioSubsystem::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMcpAudioSubsystem::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMcpAudioSubsystem::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMcpAudioSubsystem::SetAmbientVolume(float Volume)
{
	AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMcpAudioSubsystem::SetDialogueVolume(float Volume)
{
	DialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMcpAudioSubsystem::UpdateMusicLayers(float DeltaTime)
{
	for (auto& Pair : MusicLayers)
	{
		FMcpMusicLayer& Layer = Pair.Value;

		if (FMath::IsNearlyEqual(Layer.Volume, Layer.TargetVolume, 0.01f))
		{
			Layer.Volume = Layer.TargetVolume;
		}
		else
		{
			float Direction = Layer.TargetVolume > Layer.Volume ? 1.0f : -1.0f;
			Layer.Volume += Direction * Layer.FadeSpeed * DeltaTime;
			Layer.Volume = FMath::Clamp(Layer.Volume, 0.0f, 1.0f);
		}
	}
}

void UMcpAudioSubsystem::UpdateAmbientZones(FVector ListenerPosition)
{
	TArray<FString> NewActiveZones;

	for (const auto& Pair : AmbientZones)
	{
		const FMcpAmbientZone& Zone = Pair.Value;
		float Distance = FVector::Dist(ListenerPosition, Zone.Center);

		if (Distance <= Zone.Radius)
		{
			NewActiveZones.Add(Zone.ZoneId);

			// Check if newly entered
			if (!ActiveAmbientZones.Contains(Zone.ZoneId))
			{
				OnAmbientZoneEntered.Broadcast(Zone.ZoneId, nullptr);
			}
		}
	}

	// Check for exited zones
	for (const FString& ActiveZone : ActiveAmbientZones)
	{
		if (!NewActiveZones.Contains(ActiveZone))
		{
			OnAmbientZoneExited.Broadcast(ActiveZone, nullptr);
		}
	}

	ActiveAmbientZones = NewActiveZones;
}

void UMcpAudioSubsystem::TransitionMusicState(EMcpMusicState NewState, float TransitionTime)
{
	// Fade out layers not associated with new state
	// Fade in layers associated with new state
	// Implementation would depend on how layers are tagged with states
}

// ============================================================================
// ANALYTICS SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UMcpAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bSessionActive = false;
}

void UMcpAnalyticsSubsystem::Deinitialize()
{
	if (bSessionActive)
	{
		EndSession();
	}
	Super::Deinitialize();
}

void UMcpAnalyticsSubsystem::StartSession()
{
	if (bSessionActive)
	{
		return;
	}

	CurrentSession.SessionId = GenerateSessionId();
	CurrentSession.StartTime = FPlatformTime::Seconds();
	CurrentSession.TotalPlayTime = 0.0f;
	CurrentSession.DeathCount = 0;
	CurrentSession.EnemiesKilled = 0;
	CurrentSession.QuestsCompleted = 0;
	CurrentSession.Events.Empty();

	bSessionActive = true;

	OnSessionStarted.Broadcast(CurrentSession.SessionId);
}

void UMcpAnalyticsSubsystem::EndSession()
{
	if (!bSessionActive)
	{
		return;
	}

	CurrentSession.TotalPlayTime = FPlatformTime::Seconds() - CurrentSession.StartTime;
	bSessionActive = false;

	OnSessionEnded.Broadcast(CurrentSession.SessionId, CurrentSession.TotalPlayTime);
}

FMcpSessionData UMcpAnalyticsSubsystem::GetCurrentSession() const
{
	return CurrentSession;
}

FString UMcpAnalyticsSubsystem::GetSessionId() const
{
	return CurrentSession.SessionId;
}

void UMcpAnalyticsSubsystem::RecordEvent(EMcpAnalyticsEventType EventType, const FString& EventName, const TMap<FString, FString>& Parameters)
{
	RecordEventAtLocation(EventType, EventName, FVector::ZeroVector, Parameters);
}

void UMcpAnalyticsSubsystem::RecordEventAtLocation(EMcpAnalyticsEventType EventType, const FString& EventName, FVector Location, const TMap<FString, FString>& Parameters)
{
	if (!bSessionActive)
	{
		return;
	}

	FMcpAnalyticsEvent Event;
	Event.EventId = FGuid::NewGuid().ToString();
	Event.EventType = EventType;
	Event.EventName = EventName;
	Event.Parameters = Parameters;
	Event.Location = Location;
	Event.Timestamp = FPlatformTime::Seconds() - CurrentSession.StartTime;
	Event.SessionId = CurrentSession.SessionId;
	Event.PlayerId = TEXT("LocalPlayer"); // Would be actual player ID in multiplayer

	CurrentSession.Events.Add(Event);

	UpdateSessionStats(Event);

	OnAnalyticsEventRecorded.Broadcast(Event);
}

void UMcpAnalyticsSubsystem::RecordPlayerDeath(FVector Location, const FString& CauseOfDeath)
{
	TMap<FString, FString> Params;
	Params.Add(TEXT("CauseOfDeath"), CauseOfDeath);

	RecordEventAtLocation(EMcpAnalyticsEventType::PlayerDeath, TEXT("PlayerDeath"), Location, Params);
	DeathHeatmapData.Add(Location);
}

void UMcpAnalyticsSubsystem::RecordEnemyKill(const FString& EnemyType, FVector Location)
{
	TMap<FString, FString> Params;
	Params.Add(TEXT("EnemyType"), EnemyType);

	RecordEventAtLocation(EMcpAnalyticsEventType::EnemyKill, TEXT("EnemyKill"), Location, Params);
}

void UMcpAnalyticsSubsystem::RecordQuestEvent(const FString& QuestId, bool bCompleted)
{
	TMap<FString, FString> Params;
	Params.Add(TEXT("QuestId"), QuestId);
	Params.Add(TEXT("Completed"), bCompleted ? TEXT("true") : TEXT("false"));

	EMcpAnalyticsEventType EventType = bCompleted ? EMcpAnalyticsEventType::QuestComplete : EMcpAnalyticsEventType::QuestStart;
	RecordEvent(EventType, bCompleted ? TEXT("QuestComplete") : TEXT("QuestStart"), Params);
}

void UMcpAnalyticsSubsystem::RecordItemPickup(const FString& ItemId, FVector Location)
{
	TMap<FString, FString> Params;
	Params.Add(TEXT("ItemId"), ItemId);

	RecordEventAtLocation(EMcpAnalyticsEventType::ItemPickup, TEXT("ItemPickup"), Location, Params);
}

void UMcpAnalyticsSubsystem::RecordPurchase(const FString& ItemId, int32 Price, const FString& Currency)
{
	TMap<FString, FString> Params;
	Params.Add(TEXT("ItemId"), ItemId);
	Params.Add(TEXT("Price"), FString::FromInt(Price));
	Params.Add(TEXT("Currency"), Currency);

	RecordEvent(EMcpAnalyticsEventType::Purchase, TEXT("Purchase"), Params);
}

TArray<FVector> UMcpAnalyticsSubsystem::GetDeathLocations() const
{
	return DeathHeatmapData;
}

TArray<FMcpAnalyticsEvent> UMcpAnalyticsSubsystem::GetEventsOfType(EMcpAnalyticsEventType EventType) const
{
	TArray<FMcpAnalyticsEvent> Result;

	for (const FMcpAnalyticsEvent& Event : CurrentSession.Events)
	{
		if (Event.EventType == EventType)
		{
			Result.Add(Event);
		}
	}

	return Result;
}

bool UMcpAnalyticsSubsystem::ExportSessionData(const FString& FilePath)
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	RootObject->SetStringField(TEXT("SessionId"), CurrentSession.SessionId);
	RootObject->SetNumberField(TEXT("StartTime"), CurrentSession.StartTime);
	RootObject->SetNumberField(TEXT("TotalPlayTime"), CurrentSession.TotalPlayTime);
	RootObject->SetNumberField(TEXT("DeathCount"), CurrentSession.DeathCount);
	RootObject->SetNumberField(TEXT("EnemiesKilled"), CurrentSession.EnemiesKilled);
	RootObject->SetNumberField(TEXT("QuestsCompleted"), CurrentSession.QuestsCompleted);

	TArray<TSharedPtr<FJsonValue>> EventsArray;
	for (const FMcpAnalyticsEvent& Event : CurrentSession.Events)
	{
		TSharedPtr<FJsonObject> EventObject = MakeShareable(new FJsonObject);
		EventObject->SetStringField(TEXT("EventId"), Event.EventId);
		EventObject->SetStringField(TEXT("EventName"), Event.EventName);
		EventObject->SetNumberField(TEXT("Timestamp"), Event.Timestamp);
		// Add more fields as needed

		EventsArray.Add(MakeShareable(new FJsonValueObject(EventObject)));
	}
	RootObject->SetArrayField(TEXT("Events"), EventsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

void UMcpAnalyticsSubsystem::ClearSessionData()
{
	CurrentSession.Events.Empty();
	DeathHeatmapData.Empty();
	CurrentSession.DeathCount = 0;
	CurrentSession.EnemiesKilled = 0;
	CurrentSession.QuestsCompleted = 0;
}

FString UMcpAnalyticsSubsystem::GenerateSessionId() const
{
	return FGuid::NewGuid().ToString();
}

void UMcpAnalyticsSubsystem::UpdateSessionStats(const FMcpAnalyticsEvent& Event)
{
	switch (Event.EventType)
	{
	case EMcpAnalyticsEventType::PlayerDeath:
		CurrentSession.DeathCount++;
		break;
	case EMcpAnalyticsEventType::EnemyKill:
		CurrentSession.EnemiesKilled++;
		break;
	case EMcpAnalyticsEventType::QuestComplete:
		CurrentSession.QuestsCompleted++;
		break;
	default:
		break;
	}
}
