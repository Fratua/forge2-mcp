// Forge2SurvivalSystem.cpp
// FORGE2 Survival Genre System Implementation

#include "Forge2SurvivalSystem.h"

// ============================================================================
// FForge2NeedState
// ============================================================================

void FForge2NeedState::Update(float DeltaTime, const FForge2NeedConfig& Config)
{
    // Decay over time
    CurrentValue -= Config.DecayRate * (DeltaTime / 60.0f); // Per minute
    CurrentValue = FMath::Clamp(CurrentValue, 0.0f, Config.MaxValue);

    bIsCritical = CurrentValue <= Config.CriticalThreshold;
    bIsDanger = CurrentValue <= Config.DangerThreshold;
}

void FForge2NeedState::Satisfy(float Amount, float MaxValue)
{
    CurrentValue += Amount;
    CurrentValue = FMath::Clamp(CurrentValue, 0.0f, MaxValue);
}

// ============================================================================
// FForge2NeedsSystem
// ============================================================================

void FForge2NeedsSystem::Initialize()
{
    // Initialize all need states to max
    for (const auto& ConfigPair : NeedConfigs)
    {
        FForge2NeedState State;
        State.CurrentValue = ConfigPair.Value.MaxValue;
        NeedStates.Add(ConfigPair.Key, State);
    }
}

void FForge2NeedsSystem::Update(float DeltaTime)
{
    for (auto& StatePair : NeedStates)
    {
        if (const FForge2NeedConfig* Config = NeedConfigs.Find(StatePair.Key))
        {
            StatePair.Value.Update(DeltaTime, *Config);
        }
    }
}

void FForge2NeedsSystem::SatisfyNeed(EForge2NeedType NeedType, float Amount)
{
    if (FForge2NeedState* State = NeedStates.Find(NeedType))
    {
        if (const FForge2NeedConfig* Config = NeedConfigs.Find(NeedType))
        {
            State->Satisfy(Amount, Config->MaxValue);
        }
    }
}

float FForge2NeedsSystem::GetNeedValue(EForge2NeedType NeedType) const
{
    if (const FForge2NeedState* State = NeedStates.Find(NeedType))
    {
        return State->CurrentValue;
    }
    return 0.0f;
}

bool FForge2NeedsSystem::IsNeedCritical(EForge2NeedType NeedType) const
{
    if (const FForge2NeedState* State = NeedStates.Find(NeedType))
    {
        return State->bIsCritical;
    }
    return false;
}

// ============================================================================
// FForge2HarvestableResource
// ============================================================================

int32 FForge2HarvestableResource::Harvest(bool bHasTool)
{
    if (!CanHarvest()) return 0;

    int32 Yield = YieldPerHit;
    if (bHasTool)
    {
        Yield = FMath::CeilToInt(Yield * ToolEfficiencyMultiplier);
    }

    Yield = FMath::Min(Yield, CurrentYield);
    CurrentYield -= Yield;

    if (CurrentYield <= 0 && RespawnTime > 0.0f)
    {
        RespawnTimer = RespawnTime;
    }

    return Yield;
}

void FForge2HarvestableResource::UpdateRespawn(float DeltaTime)
{
    if (RespawnTimer > 0.0f)
    {
        RespawnTimer -= DeltaTime;
        if (RespawnTimer <= 0.0f)
        {
            CurrentYield = MaxYield;
            RespawnTimer = 0.0f;
        }
    }
}

// ============================================================================
// FForge2CraftingRecipe
// ============================================================================

bool FForge2CraftingRecipe::CanCraft(const TMap<FName, int32>& Inventory, FName CurrentStation) const
{
    if (!bUnlocked) return false;

    // Check station requirement
    if (!RequiredStation.IsNone() && RequiredStation != CurrentStation)
    {
        return false;
    }

    // Check ingredients
    for (const FForge2CraftingIngredient& Ingredient : Ingredients)
    {
        const int32* Count = Inventory.Find(Ingredient.ItemID);
        if (!Count || *Count < Ingredient.Amount)
        {
            return false;
        }
    }

    return true;
}

// ============================================================================
// FForge2CraftingQueue
// ============================================================================

void FForge2CraftingQueue::AddToQueue(FName RecipeID)
{
    QueuedRecipes.Add(RecipeID);
}

void FForge2CraftingQueue::Update(float DeltaTime)
{
    if (QueuedRecipes.Num() == 0) return;

    CurrentProgress += DeltaTime;
}

bool FForge2CraftingQueue::IsComplete() const
{
    return CurrentProgress >= CurrentRecipeTime;
}

FName FForge2CraftingQueue::GetCurrentRecipe() const
{
    return QueuedRecipes.Num() > 0 ? QueuedRecipes[0] : NAME_None;
}

// ============================================================================
// FForge2PlacedStructure
// ============================================================================

void FForge2PlacedStructure::TakeDamage(float Damage)
{
    CurrentHealth -= Damage;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth);
}

// ============================================================================
// FForge2WeatherState
// ============================================================================

void FForge2WeatherState::Update(float DeltaTime)
{
    if (TransitionProgress < 1.0f)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        TransitionProgress = FMath::Clamp(TransitionProgress, 0.0f, 1.0f);

        if (TransitionProgress >= 1.0f)
        {
            CurrentWeather = NextWeather;
        }
    }

    TimeRemaining -= DeltaTime;
}

void FForge2WeatherState::TransitionTo(const FForge2WeatherConfig& NewWeather)
{
    NextWeather = NewWeather;
    TransitionProgress = 0.0f;
    TimeRemaining = FMath::FRandRange(NewWeather.MinDuration, NewWeather.MaxDuration);
}

float FForge2WeatherState::GetCurrentTemperature() const
{
    if (TransitionProgress < 1.0f)
    {
        float CurrentTemp = CurrentWeather.Temperature + CurrentWeather.TemperatureModifier;
        float NextTemp = NextWeather.Temperature + NextWeather.TemperatureModifier;
        return FMath::Lerp(CurrentTemp, NextTemp, TransitionProgress);
    }

    return CurrentWeather.Temperature + CurrentWeather.TemperatureModifier;
}

// ============================================================================
// FForge2AnimalInstance
// ============================================================================

void FForge2AnimalInstance::TakeDamage(float Damage)
{
    CurrentHealth -= Damage;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth);
}

void FForge2AnimalInstance::Feed(FName FoodItemID, const FForge2AnimalDefinition& Definition)
{
    if (bIsTamed) return;

    // Check if this is preferred food
    if (Definition.FoodPreferences.Contains(FoodItemID))
    {
        TameProgress += 100.0f / (Definition.TameTime / 10.0f); // Assumes 10 feedings
        if (TameProgress >= 100.0f)
        {
            bIsTamed = true;
            bIsAggro = false;
        }
    }
}

// ============================================================================
// FForge2SurvivalPlayerState
// ============================================================================

bool FForge2SurvivalPlayerState::AddToInventory(FName ItemID, int32 Amount)
{
    if (int32* Count = Inventory.Find(ItemID))
    {
        *Count += Amount;
    }
    else
    {
        if (Inventory.Num() >= MaxInventorySlots)
        {
            return false; // Inventory full
        }
        Inventory.Add(ItemID, Amount);
    }
    return true;
}

bool FForge2SurvivalPlayerState::RemoveFromInventory(FName ItemID, int32 Amount)
{
    if (int32* Count = Inventory.Find(ItemID))
    {
        if (*Count >= Amount)
        {
            *Count -= Amount;
            if (*Count <= 0)
            {
                Inventory.Remove(ItemID);
            }
            return true;
        }
    }
    return false;
}

int32 FForge2SurvivalPlayerState::GetItemCount(FName ItemID) const
{
    if (const int32* Count = Inventory.Find(ItemID))
    {
        return *Count;
    }
    return 0;
}

bool FForge2SurvivalPlayerState::HasItem(FName ItemID, int32 Amount) const
{
    return GetItemCount(ItemID) >= Amount;
}

// ============================================================================
// UForge2SurvivalManager
// ============================================================================

UForge2SurvivalManager::UForge2SurvivalManager()
{
    DayLength = 1200.0f;
    CurrentTime = 0.0f;
}

void UForge2SurvivalManager::Initialize()
{
    for (FForge2SurvivalPlayerState& Player : Players)
    {
        Player.Needs.Initialize();
    }

    // Set initial weather
    Weather.CurrentWeather = UForge2SurvivalUtilities::CreateWeatherPreset(EForge2WeatherType::Clear);
    Weather.TimeRemaining = FMath::FRandRange(Weather.CurrentWeather.MinDuration, Weather.CurrentWeather.MaxDuration);
}

void UForge2SurvivalManager::Update(float DeltaTime)
{
    CurrentTime += DeltaTime;

    UpdateNeeds(DeltaTime);
    UpdateResources(DeltaTime);
    UpdateStructures(DeltaTime);
    UpdateAnimals(DeltaTime);
    UpdateWeather(DeltaTime);
}

void UForge2SurvivalManager::UpdateNeeds(float DeltaTime)
{
    for (FForge2SurvivalPlayerState& Player : Players)
    {
        Player.Needs.Update(DeltaTime);

        // Apply temperature effects
        float Temp = Weather.GetCurrentTemperature();
        if (Temp < 0.0f || Temp > 40.0f)
        {
            // Temperature stress
            Player.Needs.SatisfyNeed(EForge2NeedType::Temperature, -1.0f * DeltaTime / 60.0f);
        }
    }
}

void UForge2SurvivalManager::UpdateResources(float DeltaTime)
{
    for (FForge2HarvestableResource& Resource : Resources)
    {
        Resource.UpdateRespawn(DeltaTime);
    }
}

void UForge2SurvivalManager::UpdateStructures(float DeltaTime)
{
    if (!DecayConfig.bDecayEnabled) return;

    for (int32 i = PlacedStructures.Num() - 1; i >= 0; --i)
    {
        FForge2PlacedStructure& Structure = PlacedStructures[i];

        Structure.DecayTimer += DeltaTime;
        if (Structure.DecayTimer >= DecayConfig.DecayStartTime)
        {
            Structure.TakeDamage(DecayConfig.DecayRate * DeltaTime);

            if (Structure.IsDestroyed())
            {
                PlacedStructures.RemoveAt(i);
            }
        }
    }
}

void UForge2SurvivalManager::UpdateAnimals(float DeltaTime)
{
    for (int32 i = Animals.Num() - 1; i >= 0; --i)
    {
        FForge2AnimalInstance& Animal = Animals[i];

        if (Animal.IsDead())
        {
            // Could drop loot here
            Animals.RemoveAt(i);
            continue;
        }

        // Simple AI behavior
        const FForge2AnimalDefinition* Definition = nullptr;
        for (const FForge2AnimalDefinition& Def : AnimalDefinitions)
        {
            if (Def.AnimalID == Animal.AnimalTypeID)
            {
                Definition = &Def;
                break;
            }
        }

        if (!Definition) continue;

        // Handle aggro for predators
        if (Definition->Behavior == EForge2AnimalBehavior::Predator ||
            Definition->Behavior == EForge2AnimalBehavior::Aggressive)
        {
            // Check for players in range
            for (const FForge2SurvivalPlayerState& Player : Players)
            {
                // Would need player location
            }
        }
    }
}

void UForge2SurvivalManager::UpdateWeather(float DeltaTime)
{
    Weather.Update(DeltaTime);

    if (Weather.TimeRemaining <= 0.0f)
    {
        // Random new weather
        TArray<EForge2WeatherType> WeatherTypes = {
            EForge2WeatherType::Clear,
            EForge2WeatherType::Cloudy,
            EForge2WeatherType::Rain,
            EForge2WeatherType::Fog
        };

        int32 Index = FMath::RandRange(0, WeatherTypes.Num() - 1);
        FForge2WeatherConfig NewWeather = UForge2SurvivalUtilities::CreateWeatherPreset(WeatherTypes[Index]);
        Weather.TransitionTo(NewWeather);
    }
}

int32 UForge2SurvivalManager::HarvestResource(FName ResourceID, FName PlayerID, bool bHasTool)
{
    FForge2SurvivalPlayerState* Player = nullptr;
    for (FForge2SurvivalPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player) return 0;

    for (FForge2HarvestableResource& Resource : Resources)
    {
        if (Resource.ResourceID == ResourceID)
        {
            int32 Yield = Resource.Harvest(bHasTool);
            if (Yield > 0)
            {
                Player->AddToInventory(ResourceID, Yield);
            }
            return Yield;
        }
    }

    return 0;
}

bool UForge2SurvivalManager::StartCrafting(FName RecipeID, FName PlayerID)
{
    FForge2SurvivalPlayerState* Player = nullptr;
    for (FForge2SurvivalPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player) return false;

    for (const FForge2CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeID == RecipeID)
        {
            if (!Recipe.CanCraft(Player->Inventory, NAME_None))
            {
                return false;
            }

            // Remove ingredients
            for (const FForge2CraftingIngredient& Ingredient : Recipe.Ingredients)
            {
                Player->RemoveFromInventory(Ingredient.ItemID, Ingredient.Amount);
            }

            // Add output (simplified - would normally use crafting queue)
            Player->AddToInventory(Recipe.OutputItemID, Recipe.OutputAmount);
            return true;
        }
    }

    return false;
}

bool UForge2SurvivalManager::CanCraftRecipe(FName RecipeID, FName PlayerID) const
{
    const FForge2SurvivalPlayerState* Player = nullptr;
    for (const FForge2SurvivalPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player) return false;

    for (const FForge2CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeID == RecipeID)
        {
            return Recipe.CanCraft(Player->Inventory, NAME_None);
        }
    }

    return false;
}

bool UForge2SurvivalManager::PlaceStructure(FName PieceID, FName PlayerID, const FVector& Location, const FRotator& Rotation)
{
    if (!CanPlaceStructure(PieceID, Location)) return false;

    const FForge2BuildingPiece* Piece = nullptr;
    for (const FForge2BuildingPiece& P : BuildingPieces)
    {
        if (P.PieceID == PieceID)
        {
            Piece = &P;
            break;
        }
    }

    if (!Piece) return false;

    // Check if player has materials
    FForge2SurvivalPlayerState* Player = nullptr;
    for (FForge2SurvivalPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player) return false;

    for (const FForge2CraftingIngredient& Ingredient : Piece->Cost)
    {
        if (!Player->HasItem(Ingredient.ItemID, Ingredient.Amount))
        {
            return false;
        }
    }

    // Remove materials
    for (const FForge2CraftingIngredient& Ingredient : Piece->Cost)
    {
        Player->RemoveFromInventory(Ingredient.ItemID, Ingredient.Amount);
    }

    // Place structure
    FForge2PlacedStructure Structure;
    Structure.InstanceID = *FString::Printf(TEXT("Structure_%d"), PlacedStructures.Num());
    Structure.PieceID = PieceID;
    Structure.Location = Location;
    Structure.Rotation = Rotation;
    Structure.CurrentHealth = Piece->MaxHealth;
    Structure.OwnerPlayerID = PlayerID;

    PlacedStructures.Add(Structure);
    return true;
}

bool UForge2SurvivalManager::CanPlaceStructure(FName PieceID, const FVector& Location) const
{
    const FForge2BuildingPiece* Piece = nullptr;
    for (const FForge2BuildingPiece& P : BuildingPieces)
    {
        if (P.PieceID == PieceID)
        {
            Piece = &P;
            break;
        }
    }

    if (!Piece) return false;

    // Check for overlapping structures
    for (const FForge2PlacedStructure& Structure : PlacedStructures)
    {
        float Distance = FVector::Distance(Structure.Location, Location);
        if (Distance < 100.0f) // Simple overlap check
        {
            return false;
        }
    }

    return true;
}

void UForge2SurvivalManager::RepairStructure(FName InstanceID, FName PlayerID)
{
    FForge2PlacedStructure* Structure = nullptr;
    for (FForge2PlacedStructure& S : PlacedStructures)
    {
        if (S.InstanceID == InstanceID)
        {
            Structure = &S;
            break;
        }
    }

    if (!Structure) return;

    const FForge2BuildingPiece* Piece = nullptr;
    for (const FForge2BuildingPiece& P : BuildingPieces)
    {
        if (P.PieceID == Structure->PieceID)
        {
            Piece = &P;
            break;
        }
    }

    if (!Piece) return;

    Structure->CurrentHealth = Piece->MaxHealth;
    Structure->DecayTimer = 0.0f;
}

void UForge2SurvivalManager::SpawnAnimal(FName AnimalTypeID, const FVector& Location)
{
    FForge2AnimalInstance Animal;
    Animal.InstanceID = *FString::Printf(TEXT("Animal_%d"), Animals.Num());
    Animal.AnimalTypeID = AnimalTypeID;
    Animal.Location = Location;

    for (const FForge2AnimalDefinition& Def : AnimalDefinitions)
    {
        if (Def.AnimalID == AnimalTypeID)
        {
            Animal.CurrentHealth = Def.Health;
            break;
        }
    }

    Animals.Add(Animal);
}

void UForge2SurvivalManager::TameAnimal(FName AnimalInstanceID, FName PlayerID, FName FoodItemID)
{
    for (FForge2AnimalInstance& Animal : Animals)
    {
        if (Animal.InstanceID == AnimalInstanceID)
        {
            for (const FForge2AnimalDefinition& Def : AnimalDefinitions)
            {
                if (Def.AnimalID == Animal.AnimalTypeID)
                {
                    Animal.Feed(FoodItemID, Def);

                    if (Animal.bIsTamed)
                    {
                        Animal.OwnerPlayerID = PlayerID;

                        for (FForge2SurvivalPlayerState& Player : Players)
                        {
                            if (Player.PlayerID == PlayerID)
                            {
                                Player.TamedAnimals.Add(AnimalInstanceID);
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}

void UForge2SurvivalManager::SetWeather(EForge2WeatherType WeatherType)
{
    FForge2WeatherConfig NewWeather = UForge2SurvivalUtilities::CreateWeatherPreset(WeatherType);
    Weather.TransitionTo(NewWeather);
}

// ============================================================================
// UForge2SurvivalUtilities
// ============================================================================

FForge2NeedConfig UForge2SurvivalUtilities::CreateHungerNeed()
{
    FForge2NeedConfig Config;
    Config.NeedType = EForge2NeedType::Hunger;
    Config.MaxValue = 100.0f;
    Config.DecayRate = 2.0f;
    Config.CriticalThreshold = 20.0f;
    Config.DangerThreshold = 10.0f;
    Config.DamagePerSecond = 1.0f;
    return Config;
}

FForge2NeedConfig UForge2SurvivalUtilities::CreateThirstNeed()
{
    FForge2NeedConfig Config;
    Config.NeedType = EForge2NeedType::Thirst;
    Config.MaxValue = 100.0f;
    Config.DecayRate = 3.0f;
    Config.CriticalThreshold = 25.0f;
    Config.DangerThreshold = 15.0f;
    Config.DamagePerSecond = 2.0f;
    return Config;
}

FForge2NeedConfig UForge2SurvivalUtilities::CreateTemperatureNeed()
{
    FForge2NeedConfig Config;
    Config.NeedType = EForge2NeedType::Temperature;
    Config.MaxValue = 100.0f;
    Config.DecayRate = 0.0f;
    Config.CriticalThreshold = 30.0f;
    Config.DangerThreshold = 15.0f;
    Config.DamagePerSecond = 1.5f;
    return Config;
}

FForge2HarvestableResource UForge2SurvivalUtilities::CreateTreeResource(const FString& Name)
{
    FForge2HarvestableResource Resource;
    Resource.ResourceID = *Name.Replace(TEXT(" "), TEXT("_"));
    Resource.ResourceName = Name;
    Resource.Category = EForge2ResourceCategory::Wood;
    Resource.MaxYield = 100;
    Resource.CurrentYield = 100;
    Resource.HarvestTime = 1.5f;
    Resource.YieldPerHit = 5;
    Resource.RequiredTool = TEXT("Axe");
    Resource.ToolEfficiencyMultiplier = 3.0f;
    Resource.RespawnTime = 600.0f;
    return Resource;
}

FForge2HarvestableResource UForge2SurvivalUtilities::CreateRockResource(const FString& Name)
{
    FForge2HarvestableResource Resource;
    Resource.ResourceID = *Name.Replace(TEXT(" "), TEXT("_"));
    Resource.ResourceName = Name;
    Resource.Category = EForge2ResourceCategory::Stone;
    Resource.MaxYield = 50;
    Resource.CurrentYield = 50;
    Resource.HarvestTime = 2.0f;
    Resource.YieldPerHit = 3;
    Resource.RequiredTool = TEXT("Pickaxe");
    Resource.ToolEfficiencyMultiplier = 2.5f;
    Resource.RespawnTime = 900.0f;
    return Resource;
}

FForge2BuildingPiece UForge2SurvivalUtilities::CreateWallPiece(const FString& Name, int32 Tier)
{
    FForge2BuildingPiece Piece;
    Piece.PieceID = *Name.Replace(TEXT(" "), TEXT("_"));
    Piece.PieceName = Name;
    Piece.PieceType = EForge2BuildingPieceType::Wall;
    Piece.Size = FVector(200.0f, 20.0f, 200.0f);
    Piece.Tier = Tier;

    FForge2CraftingIngredient Ingredient;
    switch (Tier)
    {
        case 1:
            Ingredient.ItemID = TEXT("Wood");
            Ingredient.Amount = 10;
            Piece.MaxHealth = 500.0f;
            break;
        case 2:
            Ingredient.ItemID = TEXT("Stone");
            Ingredient.Amount = 15;
            Piece.MaxHealth = 1500.0f;
            break;
        case 3:
            Ingredient.ItemID = TEXT("Metal");
            Ingredient.Amount = 10;
            Piece.MaxHealth = 3000.0f;
            break;
        default:
            Ingredient.ItemID = TEXT("Wood");
            Ingredient.Amount = 10;
            Piece.MaxHealth = 500.0f;
    }

    Piece.Cost.Add(Ingredient);

    Piece.SnapsTo.Add(EForge2BuildingPieceType::Foundation);
    Piece.SnapsTo.Add(EForge2BuildingPieceType::Wall);
    Piece.SnapsTo.Add(EForge2BuildingPieceType::Floor);

    return Piece;
}

FForge2AnimalDefinition UForge2SurvivalUtilities::CreatePassiveAnimal(const FString& Name)
{
    FForge2AnimalDefinition Animal;
    Animal.AnimalID = *Name.Replace(TEXT(" "), TEXT("_"));
    Animal.AnimalName = Name;
    Animal.Behavior = EForge2AnimalBehavior::Passive;
    Animal.Health = 50.0f;
    Animal.Damage = 0.0f;
    Animal.Speed = 200.0f;
    Animal.AggroRange = 0.0f;
    Animal.FleeRange = 500.0f;
    Animal.TameTime = 30.0f;
    Animal.bCanRide = false;

    Animal.FoodPreferences.Add(TEXT("Berry"));
    Animal.FoodPreferences.Add(TEXT("Vegetable"));

    return Animal;
}

FForge2AnimalDefinition UForge2SurvivalUtilities::CreatePredatorAnimal(const FString& Name)
{
    FForge2AnimalDefinition Animal;
    Animal.AnimalID = *Name.Replace(TEXT(" "), TEXT("_"));
    Animal.AnimalName = Name;
    Animal.Behavior = EForge2AnimalBehavior::Predator;
    Animal.Health = 150.0f;
    Animal.Damage = 25.0f;
    Animal.Speed = 400.0f;
    Animal.AggroRange = 800.0f;
    Animal.FleeRange = 0.0f;
    Animal.TameTime = 120.0f;
    Animal.bCanRide = true;
    Animal.RideSpeed = 600.0f;

    Animal.FoodPreferences.Add(TEXT("Raw_Meat"));

    return Animal;
}

FForge2WeatherConfig UForge2SurvivalUtilities::CreateWeatherPreset(EForge2WeatherType Type)
{
    FForge2WeatherConfig Config;
    Config.WeatherType = Type;

    switch (Type)
    {
        case EForge2WeatherType::Clear:
            Config.Temperature = 25.0f;
            Config.RainIntensity = 0.0f;
            Config.WindSpeed = 5.0f;
            Config.Visibility = 1.0f;
            Config.MovementSpeedMultiplier = 1.0f;
            Config.bCanStartFire = true;
            Config.MinDuration = 300.0f;
            Config.MaxDuration = 600.0f;
            break;

        case EForge2WeatherType::Rain:
            Config.Temperature = 18.0f;
            Config.RainIntensity = 0.7f;
            Config.WindSpeed = 15.0f;
            Config.Visibility = 0.7f;
            Config.MovementSpeedMultiplier = 0.9f;
            Config.bCanStartFire = false;
            Config.MinDuration = 120.0f;
            Config.MaxDuration = 300.0f;
            break;

        case EForge2WeatherType::Storm:
            Config.Temperature = 15.0f;
            Config.RainIntensity = 1.0f;
            Config.WindSpeed = 40.0f;
            Config.Visibility = 0.4f;
            Config.MovementSpeedMultiplier = 0.7f;
            Config.bCanStartFire = false;
            Config.MinDuration = 60.0f;
            Config.MaxDuration = 180.0f;
            break;

        case EForge2WeatherType::Snow:
            Config.Temperature = -5.0f;
            Config.RainIntensity = 0.5f;
            Config.WindSpeed = 10.0f;
            Config.Visibility = 0.6f;
            Config.MovementSpeedMultiplier = 0.8f;
            Config.bCanStartFire = true;
            Config.MinDuration = 180.0f;
            Config.MaxDuration = 400.0f;
            break;

        case EForge2WeatherType::Blizzard:
            Config.Temperature = -20.0f;
            Config.RainIntensity = 1.0f;
            Config.WindSpeed = 50.0f;
            Config.Visibility = 0.2f;
            Config.MovementSpeedMultiplier = 0.5f;
            Config.bCanStartFire = false;
            Config.MinDuration = 60.0f;
            Config.MaxDuration = 120.0f;
            break;

        case EForge2WeatherType::Heatwave:
            Config.Temperature = 45.0f;
            Config.RainIntensity = 0.0f;
            Config.WindSpeed = 5.0f;
            Config.Visibility = 0.9f;
            Config.MovementSpeedMultiplier = 0.85f;
            Config.bCanStartFire = true;
            Config.MinDuration = 180.0f;
            Config.MaxDuration = 360.0f;
            break;

        default:
            Config.Temperature = 20.0f;
            Config.Visibility = 1.0f;
            break;
    }

    return Config;
}
