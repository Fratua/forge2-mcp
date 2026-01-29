// Forge2StrategySystem.cpp
// FORGE2 Strategy/RTS Genre System Implementation

#include "Forge2StrategySystem.h"

// ============================================================================
// FForge2ResourceCost
// ============================================================================

bool FForge2ResourceCost::CanAfford(const TMap<EForge2ResourceType, int32>& Available) const
{
    for (const auto& CostPair : Costs)
    {
        const int32* AvailableAmount = Available.Find(CostPair.Key);
        if (!AvailableAmount || *AvailableAmount < CostPair.Value)
        {
            return false;
        }
    }
    return true;
}

void FForge2ResourceCost::Apply(TMap<EForge2ResourceType, int32>& Resources) const
{
    for (const auto& CostPair : Costs)
    {
        if (int32* Amount = Resources.Find(CostPair.Key))
        {
            *Amount -= CostPair.Value;
            if (*Amount < 0) *Amount = 0;
        }
    }
}

// ============================================================================
// FForge2ResourceNode
// ============================================================================

int32 FForge2ResourceNode::Harvest(float DeltaTime)
{
    if (IsDepleted()) return 0;

    int32 AmountToHarvest = FMath::CeilToInt(GatherRate * CurrentWorkers * DeltaTime);

    if (!bInfinite)
    {
        AmountToHarvest = FMath::Min(AmountToHarvest, RemainingResources);
        RemainingResources -= AmountToHarvest;
    }

    return AmountToHarvest;
}

// ============================================================================
// FForge2UnitInstance
// ============================================================================

void FForge2UnitInstance::TakeDamage(float Damage, float ArmorPen)
{
    // Calculate armor reduction
    // Armor formula: damage_reduction = armor / (armor + 100)
    // With armor penetration
    float EffectiveArmor = FMath::Max(0.0f, 0.0f - ArmorPen); // Would get armor from definition
    float DamageReduction = EffectiveArmor / (EffectiveArmor + 100.0f);
    float FinalDamage = Damage * (1.0f - DamageReduction);

    CurrentHealth -= FinalDamage;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth);
}

// ============================================================================
// FForge2FogOfWar
// ============================================================================

void FForge2FogOfWar::Initialize(int32 SizeX, int32 SizeY)
{
    GridSizeX = SizeX;
    GridSizeY = SizeY;
    VisibilityGrid.SetNum(SizeX * SizeY);

    // Initialize all cells as unexplored
    for (int32 i = 0; i < VisibilityGrid.Num(); ++i)
    {
        VisibilityGrid[i] = 0;
    }
}

void FForge2FogOfWar::UpdateVisibility(const TArray<FVector>& VisionSources, const TArray<float>& VisionRanges)
{
    // First, demote all visible cells to explored
    for (int32 i = 0; i < VisibilityGrid.Num(); ++i)
    {
        if (VisibilityGrid[i] == 2)
        {
            VisibilityGrid[i] = 1;
        }
    }

    // Then update visibility from each source
    for (int32 SourceIdx = 0; SourceIdx < VisionSources.Num(); ++SourceIdx)
    {
        const FVector& Source = VisionSources[SourceIdx];
        float Range = (SourceIdx < VisionRanges.Num()) ? VisionRanges[SourceIdx] : 500.0f;

        RevealArea(Source, Range);
    }
}

uint8 FForge2FogOfWar::GetVisibility(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridSizeX || Y < 0 || Y >= GridSizeY)
    {
        return 0;
    }

    return VisibilityGrid[Y * GridSizeX + X];
}

uint8 FForge2FogOfWar::GetVisibilityAtLocation(const FVector& Location) const
{
    int32 X = FMath::FloorToInt(Location.X / CellSize) + GridSizeX / 2;
    int32 Y = FMath::FloorToInt(Location.Y / CellSize) + GridSizeY / 2;

    return GetVisibility(X, Y);
}

void FForge2FogOfWar::RevealArea(const FVector& Center, float Radius)
{
    int32 CenterX = FMath::FloorToInt(Center.X / CellSize) + GridSizeX / 2;
    int32 CenterY = FMath::FloorToInt(Center.Y / CellSize) + GridSizeY / 2;
    int32 CellRadius = FMath::CeilToInt(Radius / CellSize);

    for (int32 Y = CenterY - CellRadius; Y <= CenterY + CellRadius; ++Y)
    {
        for (int32 X = CenterX - CellRadius; X <= CenterX + CellRadius; ++X)
        {
            if (X < 0 || X >= GridSizeX || Y < 0 || Y >= GridSizeY)
            {
                continue;
            }

            // Check if within circular radius
            float DistSq = FMath::Square((X - CenterX) * CellSize) + FMath::Square((Y - CenterY) * CellSize);
            if (DistSq <= FMath::Square(Radius))
            {
                VisibilityGrid[Y * GridSizeX + X] = 2; // Visible
            }
        }
    }
}

// ============================================================================
// FForge2Formation
// ============================================================================

TArray<FVector> FForge2Formation::CalculatePositions(int32 UnitCount, const FVector& Center, const FVector& Direction) const
{
    TArray<FVector> Positions;
    Positions.Reserve(UnitCount);

    if (UnitCount == 0) return Positions;

    FVector Forward = Direction.GetSafeNormal();
    if (Forward.IsNearlyZero())
    {
        Forward = FVector::ForwardVector;
    }
    FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();

    switch (FormationType)
    {
        case EForge2FormationType::Line:
        {
            float TotalWidth = (UnitCount - 1) * Spacing;
            float StartOffset = -TotalWidth * 0.5f;

            for (int32 i = 0; i < UnitCount; ++i)
            {
                FVector Offset = Right * (StartOffset + i * Spacing);
                Positions.Add(Center + Offset);
            }
            break;
        }

        case EForge2FormationType::Column:
        {
            float TotalDepth = (UnitCount - 1) * Spacing;
            float StartOffset = TotalDepth * 0.5f;

            for (int32 i = 0; i < UnitCount; ++i)
            {
                FVector Offset = Forward * (StartOffset - i * Spacing);
                Positions.Add(Center + Offset);
            }
            break;
        }

        case EForge2FormationType::Wedge:
        {
            // V-formation
            Positions.Add(Center + Forward * Spacing);

            for (int32 i = 1; i < UnitCount; ++i)
            {
                int32 Row = (i + 1) / 2;
                float Side = (i % 2 == 1) ? 1.0f : -1.0f;

                FVector Offset = -Forward * Row * Spacing + Right * Side * Row * Spacing * 0.5f;
                Positions.Add(Center + Offset);
            }
            break;
        }

        case EForge2FormationType::Box:
        {
            int32 RowCount = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(UnitCount)));
            int32 ColCount = FMath::CeilToInt(static_cast<float>(UnitCount) / RowCount);

            float WidthOffset = (ColCount - 1) * Spacing * 0.5f;
            float DepthOffset = (RowCount - 1) * Spacing * 0.5f;

            int32 Index = 0;
            for (int32 Row = 0; Row < RowCount && Index < UnitCount; ++Row)
            {
                for (int32 Col = 0; Col < ColCount && Index < UnitCount; ++Col, ++Index)
                {
                    FVector Offset = Right * (Col * Spacing - WidthOffset) + Forward * (DepthOffset - Row * Spacing);
                    Positions.Add(Center + Offset);
                }
            }
            break;
        }

        case EForge2FormationType::Circle:
        {
            if (UnitCount == 1)
            {
                Positions.Add(Center);
            }
            else
            {
                float Radius = (UnitCount * Spacing) / (2.0f * PI);
                float AngleStep = (2.0f * PI) / UnitCount;

                for (int32 i = 0; i < UnitCount; ++i)
                {
                    float Angle = i * AngleStep;
                    FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Radius;
                    Positions.Add(Center + Offset);
                }
            }
            break;
        }

        case EForge2FormationType::Scatter:
        {
            float ScatterRadius = Spacing * FMath::Sqrt(static_cast<float>(UnitCount));

            for (int32 i = 0; i < UnitCount; ++i)
            {
                float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
                float Dist = FMath::FRandRange(0.0f, ScatterRadius);

                FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Dist;
                Positions.Add(Center + Offset);
            }
            break;
        }

        default:
        case EForge2FormationType::None:
        {
            // All units at center
            for (int32 i = 0; i < UnitCount; ++i)
            {
                Positions.Add(Center);
            }
            break;
        }
    }

    return Positions;
}

// ============================================================================
// UForge2StrategyManager
// ============================================================================

UForge2StrategyManager::UForge2StrategyManager()
{
    CurrentTime = 0.0f;
}

void UForge2StrategyManager::InitializeMatch(int32 NumPlayers)
{
    Players.Empty();
    Units.Empty();
    Buildings.Empty();

    for (int32 i = 0; i < NumPlayers; ++i)
    {
        FForge2StrategyPlayerState Player;
        Player.PlayerID = i;
        Player.PlayerName = FString::Printf(TEXT("Player %d"), i + 1);
        Player.TeamID = i; // Each player on own team by default

        // Starting resources
        Player.Resources.Add(EForge2ResourceType::Gold, 500);
        Player.Resources.Add(EForge2ResourceType::Wood, 200);
        Player.Resources.Add(EForge2ResourceType::Food, 100);
        Player.Resources.Add(EForge2ResourceType::Supply, 0);

        Player.CurrentSupply = 0;
        Player.MaxSupply = 10;

        // Initialize fog of war
        Player.FogOfWar.Initialize(256, 256);

        Players.Add(Player);
    }

    CurrentTime = 0.0f;
}

void UForge2StrategyManager::UpdateSimulation(float DeltaTime)
{
    CurrentTime += DeltaTime;

    UpdateUnits(DeltaTime);
    UpdateBuildings(DeltaTime);
    UpdateResources(DeltaTime);
    UpdateResearch(DeltaTime);
    UpdateCombat(DeltaTime);
    UpdateFogOfWar();
}

void UForge2StrategyManager::UpdateUnits(float DeltaTime)
{
    for (int32 i = Units.Num() - 1; i >= 0; --i)
    {
        FForge2UnitInstance& Unit = Units[i];

        // Remove dead units
        if (Unit.IsDead())
        {
            // Update supply for owner
            for (FForge2StrategyPlayerState& Player : Players)
            {
                if (Player.PlayerID == Unit.OwnerPlayerID)
                {
                    // Find unit definition to get supply cost
                    for (const FForge2UnitDefinition& Def : UnitDefinitions)
                    {
                        if (Def.UnitID == Unit.UnitTypeID)
                        {
                            Player.CurrentSupply -= Def.SupplyCost;
                            break;
                        }
                    }
                    break;
                }
            }

            Units.RemoveAt(i);
            continue;
        }

        // Update attack cooldown
        if (Unit.AttackCooldown > 0.0f)
        {
            Unit.AttackCooldown -= DeltaTime;
        }

        // Update ability cooldowns
        for (auto& CooldownPair : Unit.AbilityCooldowns)
        {
            if (CooldownPair.Value > 0.0f)
            {
                CooldownPair.Value -= DeltaTime;
            }
        }

        // Handle movement
        if (Unit.bIsMoving)
        {
            const FForge2UnitDefinition* UnitDef = nullptr;
            for (const FForge2UnitDefinition& Def : UnitDefinitions)
            {
                if (Def.UnitID == Unit.UnitTypeID)
                {
                    UnitDef = &Def;
                    break;
                }
            }

            if (UnitDef)
            {
                FVector ToTarget = Unit.MoveTargetLocation - Unit.Location;
                ToTarget.Z = 0.0f;
                float Distance = ToTarget.Size();

                if (Distance < 10.0f)
                {
                    Unit.bIsMoving = false;
                    Unit.Location = Unit.MoveTargetLocation;
                }
                else
                {
                    FVector Direction = ToTarget.GetSafeNormal();
                    FVector Movement = Direction * UnitDef->MoveSpeed * DeltaTime;

                    if (Movement.Size() > Distance)
                    {
                        Unit.Location = Unit.MoveTargetLocation;
                        Unit.bIsMoving = false;
                    }
                    else
                    {
                        Unit.Location += Movement;
                        Unit.Rotation = Direction.Rotation();
                    }
                }
            }
        }
    }
}

void UForge2StrategyManager::UpdateBuildings(float DeltaTime)
{
    for (int32 i = Buildings.Num() - 1; i >= 0; --i)
    {
        FForge2BuildingInstance& Building = Buildings[i];

        // Remove destroyed buildings
        if (Building.IsDestroyed())
        {
            // Update supply for owner
            for (FForge2StrategyPlayerState& Player : Players)
            {
                if (Player.PlayerID == Building.OwnerPlayerID)
                {
                    for (const FForge2BuildingDefinition& Def : BuildingDefinitions)
                    {
                        if (Def.BuildingID == Building.BuildingTypeID)
                        {
                            Player.MaxSupply -= Def.SupplyProvided;
                            break;
                        }
                    }
                    break;
                }
            }

            Buildings.RemoveAt(i);
            continue;
        }

        // Handle construction
        if (Building.bIsConstructing)
        {
            const FForge2BuildingDefinition* BuildingDef = nullptr;
            for (const FForge2BuildingDefinition& Def : BuildingDefinitions)
            {
                if (Def.BuildingID == Building.BuildingTypeID)
                {
                    BuildingDef = &Def;
                    break;
                }
            }

            if (BuildingDef)
            {
                Building.BuildProgress += DeltaTime / BuildingDef->BuildTime;
                if (Building.BuildProgress >= 1.0f)
                {
                    Building.BuildProgress = 1.0f;
                    Building.bIsConstructing = false;

                    // Add supply when complete
                    for (FForge2StrategyPlayerState& Player : Players)
                    {
                        if (Player.PlayerID == Building.OwnerPlayerID)
                        {
                            Player.MaxSupply += BuildingDef->SupplyProvided;
                            break;
                        }
                    }
                }
            }
        }

        // Handle production queue
        if (Building.IsComplete() && Building.ProductionQueue.Num() > 0)
        {
            FName UnitToProduce = Building.ProductionQueue[0];

            const FForge2UnitDefinition* UnitDef = nullptr;
            for (const FForge2UnitDefinition& Def : UnitDefinitions)
            {
                if (Def.UnitID == UnitToProduce)
                {
                    UnitDef = &Def;
                    break;
                }
            }

            if (UnitDef)
            {
                Building.CurrentProductionProgress += DeltaTime / UnitDef->BuildTime;

                if (Building.CurrentProductionProgress >= 1.0f)
                {
                    // Spawn unit at rally point
                    FVector SpawnLocation = Building.RallyPoint.IsNearlyZero() ?
                        Building.Location + FVector(300.0f, 0.0f, 0.0f) : Building.RallyPoint;

                    CreateUnit(Building.OwnerPlayerID, UnitToProduce, SpawnLocation);

                    // Remove from queue
                    Building.ProductionQueue.RemoveAt(0);
                    Building.CurrentProductionProgress = 0.0f;
                }
            }
        }

        // Handle resource generation
        if (Building.IsComplete())
        {
            const FForge2BuildingDefinition* BuildingDef = nullptr;
            for (const FForge2BuildingDefinition& Def : BuildingDefinitions)
            {
                if (Def.BuildingID == Building.BuildingTypeID)
                {
                    BuildingDef = &Def;
                    break;
                }
            }

            if (BuildingDef)
            {
                for (FForge2StrategyPlayerState& Player : Players)
                {
                    if (Player.PlayerID == Building.OwnerPlayerID)
                    {
                        for (const auto& GenPair : BuildingDef->ResourceGeneration)
                        {
                            int32 Amount = FMath::CeilToInt(GenPair.Value * DeltaTime);
                            if (int32* CurrentAmount = Player.Resources.Find(GenPair.Key))
                            {
                                *CurrentAmount += Amount;
                            }
                            else
                            {
                                Player.Resources.Add(GenPair.Key, Amount);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}

void UForge2StrategyManager::UpdateResources(float DeltaTime)
{
    for (FForge2ResourceNode& Node : ResourceNodes)
    {
        if (!Node.IsDepleted() && Node.CurrentWorkers > 0)
        {
            // Resources are harvested by workers assigned to this node
            // The actual resource transfer happens in unit update
        }
    }
}

void UForge2StrategyManager::UpdateResearch(float DeltaTime)
{
    for (FForge2StrategyPlayerState& Player : Players)
    {
        if (Player.CurrentResearch.IsNone())
        {
            continue;
        }

        const FForge2TechDefinition* TechDef = nullptr;
        for (const FForge2TechDefinition& Def : TechDefinitions)
        {
            if (Def.TechID == Player.CurrentResearch)
            {
                TechDef = &Def;
                break;
            }
        }

        if (TechDef)
        {
            Player.ResearchProgress += DeltaTime / TechDef->ResearchTime;

            if (Player.ResearchProgress >= 1.0f)
            {
                Player.ResearchedTechs.Add(Player.CurrentResearch);
                Player.CurrentResearch = NAME_None;
                Player.ResearchProgress = 0.0f;
            }
        }
    }
}

void UForge2StrategyManager::UpdateCombat(float DeltaTime)
{
    for (FForge2UnitInstance& Unit : Units)
    {
        if (Unit.IsDead() || !Unit.bIsAttacking)
        {
            continue;
        }

        // Find target
        FForge2UnitInstance* Target = nullptr;
        for (FForge2UnitInstance& PotentialTarget : Units)
        {
            if (PotentialTarget.InstanceID == Unit.CurrentTargetID)
            {
                Target = &PotentialTarget;
                break;
            }
        }

        if (!Target || Target->IsDead())
        {
            Unit.bIsAttacking = false;
            Unit.CurrentTargetID = NAME_None;
            continue;
        }

        // Get unit definition for attack stats
        const FForge2UnitDefinition* UnitDef = nullptr;
        for (const FForge2UnitDefinition& Def : UnitDefinitions)
        {
            if (Def.UnitID == Unit.UnitTypeID)
            {
                UnitDef = &Def;
                break;
            }
        }

        if (!UnitDef)
        {
            continue;
        }

        // Check range
        float Distance = FVector::Distance(Unit.Location, Target->Location);
        if (Distance > UnitDef->AttackRange)
        {
            // Move towards target
            Unit.bIsMoving = true;
            Unit.MoveTargetLocation = Target->Location;
            continue;
        }

        // Attack if cooldown is ready
        if (Unit.AttackCooldown <= 0.0f)
        {
            Target->TakeDamage(UnitDef->AttackDamage);
            Unit.AttackCooldown = 1.0f / UnitDef->AttackSpeed;
        }
    }
}

void UForge2StrategyManager::UpdateFogOfWar()
{
    for (FForge2StrategyPlayerState& Player : Players)
    {
        TArray<FVector> VisionSources;
        TArray<float> VisionRanges;

        // Add units
        for (const FForge2UnitInstance& Unit : Units)
        {
            if (Unit.OwnerPlayerID == Player.PlayerID)
            {
                VisionSources.Add(Unit.Location);

                // Find sight range from definition
                float SightRange = 500.0f;
                for (const FForge2UnitDefinition& Def : UnitDefinitions)
                {
                    if (Def.UnitID == Unit.UnitTypeID)
                    {
                        SightRange = Def.SightRange;
                        break;
                    }
                }
                VisionRanges.Add(SightRange);
            }
        }

        // Add buildings
        for (const FForge2BuildingInstance& Building : Buildings)
        {
            if (Building.OwnerPlayerID == Player.PlayerID && Building.IsComplete())
            {
                VisionSources.Add(Building.Location);

                float SightRange = 400.0f;
                for (const FForge2BuildingDefinition& Def : BuildingDefinitions)
                {
                    if (Def.BuildingID == Building.BuildingTypeID)
                    {
                        SightRange = Def.SightRange;
                        break;
                    }
                }
                VisionRanges.Add(SightRange);
            }
        }

        Player.FogOfWar.UpdateVisibility(VisionSources, VisionRanges);
    }
}

void UForge2StrategyManager::IssueCommand(const FForge2Command& Command)
{
    for (const FName& UnitID : Command.SelectedUnits)
    {
        for (FForge2UnitInstance& Unit : Units)
        {
            if (Unit.InstanceID != UnitID)
            {
                continue;
            }

            switch (Command.CommandType)
            {
                case EForge2CommandType::Move:
                    Unit.bIsMoving = true;
                    Unit.bIsAttacking = false;
                    Unit.MoveTargetLocation = Command.TargetLocation;
                    Unit.CurrentTargetID = NAME_None;
                    break;

                case EForge2CommandType::Attack:
                    Unit.bIsAttacking = true;
                    Unit.CurrentTargetID = Command.TargetID;
                    break;

                case EForge2CommandType::AttackMove:
                    Unit.bIsMoving = true;
                    Unit.bIsAttacking = true;
                    Unit.MoveTargetLocation = Command.TargetLocation;
                    Unit.Stance = EForge2UnitStance::Aggressive;
                    break;

                case EForge2CommandType::Stop:
                    Unit.bIsMoving = false;
                    Unit.bIsAttacking = false;
                    Unit.CurrentTargetID = NAME_None;
                    break;

                case EForge2CommandType::Hold:
                    Unit.bIsMoving = false;
                    Unit.Stance = EForge2UnitStance::HoldPosition;
                    break;

                case EForge2CommandType::Patrol:
                    Unit.bIsMoving = true;
                    Unit.Stance = EForge2UnitStance::Patrol;
                    Unit.MoveTargetLocation = Command.TargetLocation;
                    break;

                default:
                    break;
            }

            break;
        }
    }
}

bool UForge2StrategyManager::CreateUnit(int32 PlayerID, FName UnitTypeID, const FVector& Location)
{
    const FForge2UnitDefinition* UnitDef = nullptr;
    for (const FForge2UnitDefinition& Def : UnitDefinitions)
    {
        if (Def.UnitID == UnitTypeID)
        {
            UnitDef = &Def;
            break;
        }
    }

    if (!UnitDef)
    {
        return false;
    }

    // Check supply
    FForge2StrategyPlayerState* Player = nullptr;
    for (FForge2StrategyPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player)
    {
        return false;
    }

    if (Player->CurrentSupply + UnitDef->SupplyCost > Player->MaxSupply)
    {
        return false;
    }

    FForge2UnitInstance NewUnit;
    NewUnit.InstanceID = *FString::Printf(TEXT("Unit_%d_%d"), PlayerID, Units.Num());
    NewUnit.UnitTypeID = UnitTypeID;
    NewUnit.OwnerPlayerID = PlayerID;
    NewUnit.Location = Location;
    NewUnit.CurrentHealth = UnitDef->MaxHealth;

    Units.Add(NewUnit);
    Player->CurrentSupply += UnitDef->SupplyCost;

    return true;
}

bool UForge2StrategyManager::PlaceBuilding(int32 PlayerID, FName BuildingTypeID, const FVector& Location, const FRotator& Rotation)
{
    if (!CanBuildAt(PlayerID, BuildingTypeID, Location))
    {
        return false;
    }

    const FForge2BuildingDefinition* BuildingDef = nullptr;
    for (const FForge2BuildingDefinition& Def : BuildingDefinitions)
    {
        if (Def.BuildingID == BuildingTypeID)
        {
            BuildingDef = &Def;
            break;
        }
    }

    if (!BuildingDef)
    {
        return false;
    }

    FForge2StrategyPlayerState* Player = nullptr;
    for (FForge2StrategyPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player || !Player->CanAfford(BuildingDef->Cost))
    {
        return false;
    }

    Player->SpendResources(BuildingDef->Cost);

    FForge2BuildingInstance NewBuilding;
    NewBuilding.InstanceID = *FString::Printf(TEXT("Building_%d_%d"), PlayerID, Buildings.Num());
    NewBuilding.BuildingTypeID = BuildingTypeID;
    NewBuilding.OwnerPlayerID = PlayerID;
    NewBuilding.Location = Location;
    NewBuilding.Rotation = Rotation;
    NewBuilding.CurrentHealth = BuildingDef->MaxHealth;
    NewBuilding.BuildProgress = 0.0f;
    NewBuilding.bIsConstructing = true;
    NewBuilding.RallyPoint = Location + FVector(300.0f, 0.0f, 0.0f);

    Buildings.Add(NewBuilding);

    return true;
}

bool UForge2StrategyManager::StartResearch(int32 PlayerID, FName TechID)
{
    const FForge2TechDefinition* TechDef = nullptr;
    for (const FForge2TechDefinition& Def : TechDefinitions)
    {
        if (Def.TechID == TechID)
        {
            TechDef = &Def;
            break;
        }
    }

    if (!TechDef)
    {
        return false;
    }

    FForge2StrategyPlayerState* Player = nullptr;
    for (FForge2StrategyPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player)
    {
        return false;
    }

    // Check if already researched
    if (Player->HasTech(TechID))
    {
        return false;
    }

    // Check prerequisites
    for (const FName& RequiredTech : TechDef->RequiredTechs)
    {
        if (!Player->HasTech(RequiredTech))
        {
            return false;
        }
    }

    // Check if can afford
    if (!Player->CanAfford(TechDef->Cost))
    {
        return false;
    }

    Player->SpendResources(TechDef->Cost);
    Player->CurrentResearch = TechID;
    Player->ResearchProgress = 0.0f;

    return true;
}

TArray<FName> UForge2StrategyManager::GetUnitsInArea(const FVector& Center, float Radius, int32 PlayerID) const
{
    TArray<FName> Result;

    for (const FForge2UnitInstance& Unit : Units)
    {
        if (PlayerID >= 0 && Unit.OwnerPlayerID != PlayerID)
        {
            continue;
        }

        float Distance = FVector::Distance(Center, Unit.Location);
        if (Distance <= Radius)
        {
            Result.Add(Unit.InstanceID);
        }
    }

    return Result;
}

bool UForge2StrategyManager::CanBuildAt(int32 PlayerID, FName BuildingTypeID, const FVector& Location) const
{
    const FForge2BuildingDefinition* BuildingDef = nullptr;
    for (const FForge2BuildingDefinition& Def : BuildingDefinitions)
    {
        if (Def.BuildingID == BuildingTypeID)
        {
            BuildingDef = &Def;
            break;
        }
    }

    if (!BuildingDef)
    {
        return false;
    }

    // Check for overlapping buildings
    for (const FForge2BuildingInstance& Building : Buildings)
    {
        FVector Diff = Building.Location - Location;
        Diff.Z = 0.0f;

        // Simple box overlap check
        if (FMath::Abs(Diff.X) < BuildingDef->Size.X && FMath::Abs(Diff.Y) < BuildingDef->Size.Y)
        {
            return false;
        }
    }

    // Check requirements
    const FForge2StrategyPlayerState* Player = nullptr;
    for (const FForge2StrategyPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player)
    {
        return false;
    }

    for (const FName& RequiredBuilding : BuildingDef->RequiredBuildings)
    {
        bool bHasBuilding = false;
        for (const FForge2BuildingInstance& Building : Buildings)
        {
            if (Building.OwnerPlayerID == PlayerID &&
                Building.BuildingTypeID == RequiredBuilding &&
                Building.IsComplete())
            {
                bHasBuilding = true;
                break;
            }
        }

        if (!bHasBuilding)
        {
            return false;
        }
    }

    for (const FName& RequiredTech : BuildingDef->RequiredTechs)
    {
        if (!Player->HasTech(RequiredTech))
        {
            return false;
        }
    }

    return true;
}

bool UForge2StrategyManager::CanTrainUnit(int32 PlayerID, FName UnitTypeID) const
{
    const FForge2UnitDefinition* UnitDef = nullptr;
    for (const FForge2UnitDefinition& Def : UnitDefinitions)
    {
        if (Def.UnitID == UnitTypeID)
        {
            UnitDef = &Def;
            break;
        }
    }

    if (!UnitDef)
    {
        return false;
    }

    const FForge2StrategyPlayerState* Player = nullptr;
    for (const FForge2StrategyPlayerState& P : Players)
    {
        if (P.PlayerID == PlayerID)
        {
            Player = &P;
            break;
        }
    }

    if (!Player)
    {
        return false;
    }

    // Check supply
    if (Player->CurrentSupply + UnitDef->SupplyCost > Player->MaxSupply)
    {
        return false;
    }

    // Check resources
    if (!Player->CanAfford(UnitDef->Cost))
    {
        return false;
    }

    // Check requirements
    for (const FName& RequiredBuilding : UnitDef->RequiredBuildings)
    {
        bool bHasBuilding = false;
        for (const FForge2BuildingInstance& Building : Buildings)
        {
            if (Building.OwnerPlayerID == PlayerID &&
                Building.BuildingTypeID == RequiredBuilding &&
                Building.IsComplete())
            {
                bHasBuilding = true;
                break;
            }
        }

        if (!bHasBuilding)
        {
            return false;
        }
    }

    for (const FName& RequiredTech : UnitDef->RequiredTechs)
    {
        if (!Player->HasTech(RequiredTech))
        {
            return false;
        }
    }

    return true;
}

// ============================================================================
// UForge2StrategyUtilities
// ============================================================================

FForge2UnitDefinition UForge2StrategyUtilities::CreateInfantryUnit(const FString& Name, float Health, float Damage, float Speed)
{
    FForge2UnitDefinition Unit;
    Unit.UnitID = *Name.Replace(TEXT(" "), TEXT("_"));
    Unit.UnitName = Name;
    Unit.UnitType = EForge2UnitType::Infantry;
    Unit.MaxHealth = Health;
    Unit.AttackDamage = Damage;
    Unit.MoveSpeed = Speed;
    Unit.AttackSpeed = 1.0f;
    Unit.AttackRange = 100.0f;
    Unit.SightRange = 500.0f;
    Unit.SupplyCost = 1;
    Unit.BuildTime = 15.0f;

    Unit.Cost.Costs.Add(EForge2ResourceType::Gold, 50);
    Unit.Cost.Costs.Add(EForge2ResourceType::Food, 1);

    return Unit;
}

FForge2UnitDefinition UForge2StrategyUtilities::CreateVehicleUnit(const FString& Name, float Health, float Damage, float Speed)
{
    FForge2UnitDefinition Unit;
    Unit.UnitID = *Name.Replace(TEXT(" "), TEXT("_"));
    Unit.UnitName = Name;
    Unit.UnitType = EForge2UnitType::Vehicle;
    Unit.MaxHealth = Health;
    Unit.Armor = 5.0f;
    Unit.AttackDamage = Damage;
    Unit.MoveSpeed = Speed;
    Unit.AttackSpeed = 0.5f;
    Unit.AttackRange = 300.0f;
    Unit.bRanged = true;
    Unit.ProjectileSpeed = 1500.0f;
    Unit.SightRange = 600.0f;
    Unit.SupplyCost = 3;
    Unit.BuildTime = 30.0f;

    Unit.Cost.Costs.Add(EForge2ResourceType::Gold, 200);
    Unit.Cost.Costs.Add(EForge2ResourceType::Oil, 50);

    return Unit;
}

FForge2UnitDefinition UForge2StrategyUtilities::CreateWorkerUnit(const FString& Name)
{
    FForge2UnitDefinition Unit;
    Unit.UnitID = *Name.Replace(TEXT(" "), TEXT("_"));
    Unit.UnitName = Name;
    Unit.UnitType = EForge2UnitType::Worker;
    Unit.MaxHealth = 40.0f;
    Unit.AttackDamage = 5.0f;
    Unit.MoveSpeed = 150.0f;
    Unit.AttackSpeed = 1.0f;
    Unit.AttackRange = 50.0f;
    Unit.SightRange = 400.0f;
    Unit.SupplyCost = 1;
    Unit.BuildTime = 10.0f;

    Unit.Cost.Costs.Add(EForge2ResourceType::Gold, 50);

    return Unit;
}

FForge2BuildingDefinition UForge2StrategyUtilities::CreateProductionBuilding(const FString& Name, EForge2BuildingType Type)
{
    FForge2BuildingDefinition Building;
    Building.BuildingID = *Name.Replace(TEXT(" "), TEXT("_"));
    Building.BuildingName = Name;
    Building.BuildingType = Type;
    Building.MaxHealth = 1000.0f;
    Building.Armor = 5.0f;
    Building.BuildTime = 30.0f;
    Building.Size = FVector(300.0f, 300.0f, 200.0f);
    Building.SightRange = 400.0f;
    Building.ProductionQueueSize = 5;

    Building.Cost.Costs.Add(EForge2ResourceType::Gold, 150);
    Building.Cost.Costs.Add(EForge2ResourceType::Wood, 100);

    return Building;
}

FForge2BuildingDefinition UForge2StrategyUtilities::CreateDefenseBuilding(const FString& Name, float Damage, float Range)
{
    FForge2BuildingDefinition Building;
    Building.BuildingID = *Name.Replace(TEXT(" "), TEXT("_"));
    Building.BuildingName = Name;
    Building.BuildingType = EForge2BuildingType::Defense;
    Building.MaxHealth = 500.0f;
    Building.Armor = 10.0f;
    Building.BuildTime = 20.0f;
    Building.Size = FVector(100.0f, 100.0f, 300.0f);
    Building.SightRange = Range;
    Building.bCanAttack = true;
    Building.AttackDamage = Damage;
    Building.AttackRange = Range;
    Building.AttackSpeed = 1.0f;

    Building.Cost.Costs.Add(EForge2ResourceType::Gold, 100);
    Building.Cost.Costs.Add(EForge2ResourceType::Stone, 75);

    return Building;
}

FForge2TechDefinition UForge2StrategyUtilities::CreateUpgradeTech(const FString& Name, FName AffectedUnit, float StatBonus)
{
    FForge2TechDefinition Tech;
    Tech.TechID = *Name.Replace(TEXT(" "), TEXT("_"));
    Tech.TechName = Name;
    Tech.Description = FString::Printf(TEXT("Upgrades %s stats by %.0f%%"), *AffectedUnit.ToString(), (StatBonus - 1.0f) * 100.0f);
    Tech.ResearchTime = 30.0f;

    Tech.UnitStatBonuses.Add(AffectedUnit, StatBonus);

    Tech.Cost.Costs.Add(EForge2ResourceType::Gold, 100);
    Tech.Cost.Costs.Add(EForge2ResourceType::Crystal, 50);

    return Tech;
}
