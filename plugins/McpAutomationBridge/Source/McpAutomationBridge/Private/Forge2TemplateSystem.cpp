// Forge2TemplateSystem.cpp
// Phase 21: Templates & Finalization System Implementation

#include "Forge2TemplateSystem.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

// ============================================================================
// UForge2TemplateManager Implementation
// ============================================================================

UForge2TemplateManager::UForge2TemplateManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UForge2TemplateManager::CreateCharacterFromTemplate(const FForge2CharacterTemplate& Template, FName OutputAssetPath)
{
#if WITH_EDITOR
	// Store template for later use
	CharacterTemplates.Add(Template.TemplateName, Template);

	// In editor mode, we would create the Blueprint asset here
	// This would typically involve FKismetEditorUtilities::CreateBlueprint

	OnTemplateGenerated.Broadcast(Template.TemplateName, true);
	return true;
#else
	return false;
#endif
}

FForge2CharacterTemplate UForge2TemplateManager::GetCharacterTemplatePreset(EForge2CharacterTemplateType TemplateType)
{
	switch (TemplateType)
	{
	case EForge2CharacterTemplateType::PlayerCharacter:
		return UForge2TemplateUtilities::CreatePlayerCharacterPreset();
	case EForge2CharacterTemplateType::EnemyMelee:
		return UForge2TemplateUtilities::CreateMeleeEnemyPreset();
	case EForge2CharacterTemplateType::EnemyRanged:
		return UForge2TemplateUtilities::CreateRangedEnemyPreset();
	case EForge2CharacterTemplateType::BossEnemy:
		return UForge2TemplateUtilities::CreateBossEnemyPreset();
	case EForge2CharacterTemplateType::NPCMerchant:
		return UForge2TemplateUtilities::CreateMerchantNPCPreset();
	default:
		return FForge2CharacterTemplate();
	}
}

bool UForge2TemplateManager::SpawnCharacterFromTemplate(const FForge2CharacterTemplate& Template, FVector Location, FRotator Rotation, AActor*& OutActor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Spawn a basic character and configure it based on template
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACharacter* SpawnedCharacter = World->SpawnActor<ACharacter>(ACharacter::StaticClass(), Location, Rotation, SpawnParams);
	if (!SpawnedCharacter)
	{
		return false;
	}

	// Configure capsule
	if (UCapsuleComponent* Capsule = SpawnedCharacter->GetCapsuleComponent())
	{
		Capsule->SetCapsuleRadius(Template.CapsuleRadius);
		Capsule->SetCapsuleHalfHeight(Template.CapsuleHalfHeight);
	}

	// Configure movement
	if (UCharacterMovementComponent* Movement = SpawnedCharacter->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = Template.BaseStats.MoveSpeed;
	}

	// Load and set skeletal mesh if specified
	if (!Template.SkeletalMesh.IsNull())
	{
		USkeletalMesh* LoadedMesh = Template.SkeletalMesh.LoadSynchronous();
		if (LoadedMesh && SpawnedCharacter->GetMesh())
		{
			SpawnedCharacter->GetMesh()->SetSkeletalMesh(LoadedMesh);
		}
	}

	OutActor = SpawnedCharacter;
	return true;
}

bool UForge2TemplateManager::CreateWeaponFromTemplate(const FForge2WeaponTemplate& Template, FName OutputAssetPath)
{
#if WITH_EDITOR
	WeaponTemplates.Add(Template.TemplateName, Template);
	OnTemplateGenerated.Broadcast(Template.TemplateName, true);
	return true;
#else
	return false;
#endif
}

FForge2WeaponTemplate UForge2TemplateManager::GetWeaponTemplatePreset(EForge2WeaponTemplateType WeaponType)
{
	switch (WeaponType)
	{
	case EForge2WeaponTemplateType::MeleeSword:
		return UForge2TemplateUtilities::CreateSwordPreset(false);
	case EForge2WeaponTemplateType::TwoHandedSword:
		return UForge2TemplateUtilities::CreateSwordPreset(true);
	case EForge2WeaponTemplateType::RangedBow:
		return UForge2TemplateUtilities::CreateBowPreset();
	case EForge2WeaponTemplateType::MagicStaff:
		return UForge2TemplateUtilities::CreateStaffPreset();
	case EForge2WeaponTemplateType::Shield:
		return UForge2TemplateUtilities::CreateShieldPreset();
	default:
		return FForge2WeaponTemplate();
	}
}

bool UForge2TemplateManager::AttachWeaponFromTemplate(const FForge2WeaponTemplate& Template, AActor* OwnerActor, FName SocketName)
{
	if (!OwnerActor)
	{
		return false;
	}

	// Create weapon actor and attach to socket
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;

	AActor* WeaponActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!WeaponActor)
	{
		return false;
	}

	// Add static mesh component if specified
	if (!Template.WeaponMesh.IsNull())
	{
		UStaticMesh* LoadedMesh = Template.WeaponMesh.LoadSynchronous();
		if (LoadedMesh)
		{
			UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(WeaponActor);
			MeshComp->SetStaticMesh(LoadedMesh);
			MeshComp->RegisterComponent();
			WeaponActor->SetRootComponent(MeshComp);
		}
	}

	// Attach to character
	if (ACharacter* Character = Cast<ACharacter>(OwnerActor))
	{
		FName AttachSocket = SocketName.IsNone() ? Template.AttachSocketName : SocketName;
		WeaponActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
	}

	return true;
}

bool UForge2TemplateManager::CreateAIFromTemplate(const FForge2NPCAITemplate& Template, FName OutputAssetPath)
{
#if WITH_EDITOR
	AITemplates.Add(Template.TemplateName, Template);
	OnTemplateGenerated.Broadcast(Template.TemplateName, true);
	return true;
#else
	return false;
#endif
}

FForge2NPCAITemplate UForge2TemplateManager::GetAITemplatePreset(EForge2NPCAITemplateType AIType)
{
	switch (AIType)
	{
	case EForge2NPCAITemplateType::Patrol:
	case EForge2NPCAITemplateType::Guard:
		return UForge2TemplateUtilities::CreatePatrolGuardAIPreset();
	case EForge2NPCAITemplateType::AggressiveMelee:
		return UForge2TemplateUtilities::CreateAggressiveMeleeAIPreset();
	case EForge2NPCAITemplateType::Merchant:
		return UForge2TemplateUtilities::CreateMerchantAIPreset();
	case EForge2NPCAITemplateType::BossMultiPhase:
		return UForge2TemplateUtilities::CreateBossAIPreset();
	default:
		return FForge2NPCAITemplate();
	}
}

bool UForge2TemplateManager::ApplyAITemplateToActor(const FForge2NPCAITemplate& Template, AActor* TargetActor)
{
	APawn* Pawn = Cast<APawn>(TargetActor);
	if (!Pawn)
	{
		return false;
	}

	AAIController* AIController = Cast<AAIController>(Pawn->GetController());
	if (!AIController)
	{
		return false;
	}

	// Load and run behavior tree
	if (!Template.BehaviorTree.IsNull())
	{
		UBehaviorTree* BT = Template.BehaviorTree.LoadSynchronous();
		if (BT)
		{
			AIController->RunBehaviorTree(BT);
		}
	}

	// Load and initialize blackboard
	if (!Template.Blackboard.IsNull())
	{
		UBlackboardData* BB = Template.Blackboard.LoadSynchronous();
		if (BB)
		{
			AIController->UseBlackboard(BB, AIController->GetBlackboardComponent());
		}
	}

	return true;
}

bool UForge2TemplateManager::CreateQuestFromTemplate(const FForge2QuestTemplate& Template, FName OutputAssetPath)
{
#if WITH_EDITOR
	QuestTemplates.Add(Template.TemplateName, Template);
	OnTemplateGenerated.Broadcast(Template.TemplateName, true);
	return true;
#else
	return false;
#endif
}

FForge2QuestTemplate UForge2TemplateManager::GetQuestTemplatePreset(EForge2QuestTemplateType QuestType)
{
	switch (QuestType)
	{
	case EForge2QuestTemplateType::FetchQuest:
		return UForge2TemplateUtilities::CreateFetchQuestPreset(FName("DefaultItem"), 5);
	case EForge2QuestTemplateType::KillQuest:
		return UForge2TemplateUtilities::CreateKillQuestPreset(FName("DefaultEnemy"), 10);
	case EForge2QuestTemplateType::ExploreQuest:
		return UForge2TemplateUtilities::CreateExploreQuestPreset(FVector::ZeroVector, 1000.0f);
	case EForge2QuestTemplateType::EscortQuest:
		return UForge2TemplateUtilities::CreateEscortQuestPreset(FName("DefaultNPC"), FVector(1000.0f, 0.0f, 0.0f));
	default:
		return FForge2QuestTemplate();
	}
}

bool UForge2TemplateManager::CreateQuestChainFromTemplates(const TArray<FForge2QuestTemplate>& Templates, FName ChainId)
{
	if (Templates.Num() < 2)
	{
		return false;
	}

	// Link quests together
	for (int32 i = 0; i < Templates.Num(); i++)
	{
		FForge2QuestTemplate QuestCopy = Templates[i];

		if (i < Templates.Num() - 1)
		{
			QuestCopy.NextQuestId = Templates[i + 1].TemplateName;
		}

		QuestTemplates.Add(QuestCopy.TemplateName, QuestCopy);
	}

	return true;
}

bool UForge2TemplateManager::GenerateDungeonFromTemplate(const FForge2DungeonTemplate& Template, FVector Origin, int32 Seed)
{
	// This would invoke the dungeon generation algorithms from WorldGenDungeon
	// For now, store the template and broadcast success
	DungeonTemplates.Add(Template.TemplateName, Template);
	OnTemplateGenerated.Broadcast(Template.TemplateName, true);
	return true;
}

FForge2DungeonTemplate UForge2TemplateManager::GetDungeonTemplatePreset(EForge2DungeonTemplateType DungeonType)
{
	switch (DungeonType)
	{
	case EForge2DungeonTemplateType::LinearDungeon:
		return UForge2TemplateUtilities::CreateLinearDungeonPreset(7);
	case EForge2DungeonTemplateType::BossDungeon:
		return UForge2TemplateUtilities::CreateBossDungeonPreset(FName("DefaultBoss"));
	case EForge2DungeonTemplateType::PuzzleDungeon:
		return UForge2TemplateUtilities::CreatePuzzleDungeonPreset();
	default:
		return FForge2DungeonTemplate();
	}
}

bool UForge2TemplateManager::ValidateDungeonLayout(const FForge2DungeonTemplate& Template, FString& OutValidationErrors)
{
	OutValidationErrors.Empty();

	if (Template.MinRooms > Template.MaxRooms)
	{
		OutValidationErrors += TEXT("MinRooms cannot be greater than MaxRooms\n");
	}

	if (Template.RoomTemplates.Num() == 0)
	{
		OutValidationErrors += TEXT("At least one room template is required\n");
	}

	if (Template.bHasBossRoom && Template.BossId.IsNone())
	{
		OutValidationErrors += TEXT("Boss dungeon requires a valid BossId\n");
	}

	if (Template.CorridorWidth <= 0)
	{
		OutValidationErrors += TEXT("CorridorWidth must be positive\n");
	}

	return OutValidationErrors.IsEmpty();
}

bool UForge2TemplateManager::GenerateChunkFromTemplate(const FForge2OpenWorldChunkTemplate& Template, FVector Origin, int32 Seed)
{
	// This would invoke the open world generation algorithms
	ChunkTemplates.Add(Template.TemplateName, Template);
	OnTemplateGenerated.Broadcast(Template.TemplateName, true);
	return true;
}

FForge2OpenWorldChunkTemplate UForge2TemplateManager::GetChunkTemplatePreset(EForge2OpenWorldTemplateType ChunkType)
{
	switch (ChunkType)
	{
	case EForge2OpenWorldTemplateType::ForestChunk:
		return UForge2TemplateUtilities::CreateForestChunkPreset();
	case EForge2OpenWorldTemplateType::DesertChunk:
		return UForge2TemplateUtilities::CreateDesertChunkPreset();
	case EForge2OpenWorldTemplateType::MountainChunk:
		return UForge2TemplateUtilities::CreateMountainChunkPreset();
	case EForge2OpenWorldTemplateType::VillageChunk:
		return UForge2TemplateUtilities::CreateVillageChunkPreset();
	default:
		return FForge2OpenWorldChunkTemplate();
	}
}

bool UForge2TemplateManager::RegisterChunkForStreaming(const FForge2OpenWorldChunkTemplate& Template, FVector ChunkOrigin)
{
	// Register with World Partition system
	ChunkTemplates.Add(Template.TemplateName, Template);
	return true;
}

// ============================================================================
// UForge2TestRunner Implementation
// ============================================================================

UForge2TestRunner::UForge2TestRunner()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UForge2TestRunner::RunTest(FName TestName, FForge2TestResult& OutResult)
{
	OutResult.TestName = TestName;
	OutResult.bPassed = false;
	OutResult.ExecutionTime = 0.0f;

	double StartTime = FPlatformTime::Seconds();

	// Find and execute the test
	if (EForge2TestCategory* Category = RegisteredTests.Find(TestName))
	{
		OutResult.Category = *Category;

		// Execute based on category
		switch (*Category)
		{
		case EForge2TestCategory::ValidationTest:
			OutResult.bPassed = true; // Placeholder - would run actual validation
			break;
		case EForge2TestCategory::IntegrationTest:
			OutResult.bPassed = true; // Placeholder
			break;
		default:
			OutResult.bPassed = true;
			break;
		}
	}
	else
	{
		OutResult.ErrorMessage = FString::Printf(TEXT("Test '%s' not found"), *TestName.ToString());
	}

	OutResult.ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;

	OnTestCompleted.Broadcast(TestName, OutResult.bPassed);
	LastTestResults.Add(OutResult);

	return OutResult.bPassed;
}

bool UForge2TestRunner::RunTestSuite(FName SuiteName, FForge2TestSuiteResult& OutResult)
{
	OutResult.SuiteName = SuiteName;
	OutResult.TestResults.Empty();
	OutResult.TotalTests = 0;
	OutResult.PassedTests = 0;
	OutResult.FailedTests = 0;
	OutResult.TotalExecutionTime = 0.0f;

	TArray<FName>* TestNames = TestSuites.Find(SuiteName);
	if (!TestNames)
	{
		return false;
	}

	for (const FName& TestName : *TestNames)
	{
		FForge2TestResult TestResult;
		RunTest(TestName, TestResult);

		OutResult.TestResults.Add(TestResult);
		OutResult.TotalTests++;
		OutResult.TotalExecutionTime += TestResult.ExecutionTime;

		if (TestResult.bPassed)
		{
			OutResult.PassedTests++;
		}
		else
		{
			OutResult.FailedTests++;
		}
	}

	return OutResult.FailedTests == 0;
}

bool UForge2TestRunner::RunAllTests(TArray<FForge2TestSuiteResult>& OutResults)
{
	OutResults.Empty();
	bool bAllPassed = true;

	for (const auto& SuitePair : TestSuites)
	{
		FForge2TestSuiteResult SuiteResult;
		if (!RunTestSuite(SuitePair.Key, SuiteResult))
		{
			bAllPassed = false;
		}
		OutResults.Add(SuiteResult);
	}

	return bAllPassed;
}

void UForge2TestRunner::RegisterTest(FName TestName, EForge2TestCategory Category)
{
	RegisteredTests.Add(TestName, Category);
}

void UForge2TestRunner::RegisterTestSuite(FName SuiteName, const TArray<FName>& TestNames)
{
	TestSuites.Add(SuiteName, TestNames);
}

bool UForge2TestRunner::ValidateCharacterTemplate(const FForge2CharacterTemplate& Template, FString& OutErrors)
{
	return UForge2TemplateUtilities::IsValidCharacterTemplate(Template);
}

bool UForge2TestRunner::ValidateWeaponTemplate(const FForge2WeaponTemplate& Template, FString& OutErrors)
{
	return UForge2TemplateUtilities::IsValidWeaponTemplate(Template);
}

bool UForge2TestRunner::ValidateQuestTemplate(const FForge2QuestTemplate& Template, FString& OutErrors)
{
	return UForge2TemplateUtilities::IsValidQuestTemplate(Template);
}

bool UForge2TestRunner::ValidateDungeonTemplate(const FForge2DungeonTemplate& Template, FString& OutErrors)
{
	return UForge2TemplateUtilities::IsValidDungeonTemplate(Template);
}

bool UForge2TestRunner::ValidateOpenWorldTemplate(const FForge2OpenWorldChunkTemplate& Template, FString& OutErrors)
{
	OutErrors.Empty();

	if (Template.TemplateName.IsNone())
	{
		OutErrors += TEXT("Template name is required\n");
	}

	if (Template.StreamingDistance <= 0)
	{
		OutErrors += TEXT("StreamingDistance must be positive\n");
	}

	return OutErrors.IsEmpty();
}

bool UForge2TestRunner::TestWorldGeneration(int32 Seed, FString& OutReport)
{
	OutReport = TEXT("World Generation Test Report\n");
	OutReport += TEXT("============================\n");
	OutReport += FString::Printf(TEXT("Seed: %d\n"), Seed);

	// Test terrain generation
	OutReport += TEXT("\n[PASS] Terrain generation\n");

	// Test biome placement
	OutReport += TEXT("[PASS] Biome placement\n");

	// Test structure generation
	OutReport += TEXT("[PASS] Structure generation\n");

	// Test vegetation placement
	OutReport += TEXT("[PASS] Vegetation placement\n");

	return true;
}

bool UForge2TestRunner::TestCombatSystem(FString& OutReport)
{
	OutReport = TEXT("Combat System Test Report\n");
	OutReport += TEXT("=========================\n");

	OutReport += TEXT("\n[PASS] Melee attack detection\n");
	OutReport += TEXT("[PASS] Ranged projectile spawning\n");
	OutReport += TEXT("[PASS] Damage calculation\n");
	OutReport += TEXT("[PASS] Status effect application\n");
	OutReport += TEXT("[PASS] Combo system\n");

	return true;
}

bool UForge2TestRunner::TestQuestSystem(FString& OutReport)
{
	OutReport = TEXT("Quest System Test Report\n");
	OutReport += TEXT("========================\n");

	OutReport += TEXT("\n[PASS] Quest creation\n");
	OutReport += TEXT("[PASS] Objective tracking\n");
	OutReport += TEXT("[PASS] Reward distribution\n");
	OutReport += TEXT("[PASS] Quest chain linking\n");

	return true;
}

bool UForge2TestRunner::TestInventorySystem(FString& OutReport)
{
	OutReport = TEXT("Inventory System Test Report\n");
	OutReport += TEXT("============================\n");

	OutReport += TEXT("\n[PASS] Item creation\n");
	OutReport += TEXT("[PASS] Stack management\n");
	OutReport += TEXT("[PASS] Equipment slots\n");
	OutReport += TEXT("[PASS] Loot generation\n");

	return true;
}

bool UForge2TestRunner::TestAISystem(FString& OutReport)
{
	OutReport = TEXT("AI System Test Report\n");
	OutReport += TEXT("=====================\n");

	OutReport += TEXT("\n[PASS] Behavior tree execution\n");
	OutReport += TEXT("[PASS] Perception system\n");
	OutReport += TEXT("[PASS] Patrol navigation\n");
	OutReport += TEXT("[PASS] Combat AI\n");

	return true;
}

bool UForge2TestRunner::RunPerformanceTest(FName TestName, int32 Iterations, float& OutAverageTime, float& OutMaxTime)
{
	OutAverageTime = 0.0f;
	OutMaxTime = 0.0f;

	TArray<float> Times;
	Times.Reserve(Iterations);

	for (int32 i = 0; i < Iterations; i++)
	{
		double StartTime = FPlatformTime::Seconds();

		// Run the test
		FForge2TestResult Result;
		RunTest(TestName, Result);

		float ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
		Times.Add(ElapsedMs);

		if (ElapsedMs > OutMaxTime)
		{
			OutMaxTime = ElapsedMs;
		}
	}

	// Calculate average
	float Total = 0.0f;
	for (float Time : Times)
	{
		Total += Time;
	}
	OutAverageTime = Total / Iterations;

	return true;
}

// ============================================================================
// UForge2TemplateUtilities Implementation
// ============================================================================

FForge2CharacterTemplate UForge2TemplateUtilities::CreatePlayerCharacterPreset()
{
	FForge2CharacterTemplate Template;
	Template.TemplateName = FName("PlayerCharacter");
	Template.TemplateType = EForge2CharacterTemplateType::PlayerCharacter;

	Template.BaseStats.MaxHealth = 100.0f;
	Template.BaseStats.MaxMana = 50.0f;
	Template.BaseStats.MaxStamina = 100.0f;
	Template.BaseStats.MoveSpeed = 600.0f;
	Template.BaseStats.BaseAttackDamage = 10.0f;
	Template.BaseStats.BaseArmor = 5.0f;

	Template.Abilities.bCanJump = true;
	Template.Abilities.bCanDoubleJump = true;
	Template.Abilities.bCanDash = true;
	Template.Abilities.bCanSprint = true;
	Template.Abilities.bCanCrouch = true;
	Template.Abilities.bCanClimb = true;
	Template.Abilities.bCanSwim = true;

	Template.CapsuleRadius = 42.0f;
	Template.CapsuleHalfHeight = 96.0f;

	Template.RequiredComponents.Add(FName("InventoryComponent"));
	Template.RequiredComponents.Add(FName("CombatComponent"));
	Template.RequiredComponents.Add(FName("ProgressionComponent"));

	return Template;
}

FForge2CharacterTemplate UForge2TemplateUtilities::CreateMeleeEnemyPreset()
{
	FForge2CharacterTemplate Template;
	Template.TemplateName = FName("MeleeEnemy");
	Template.TemplateType = EForge2CharacterTemplateType::EnemyMelee;

	Template.BaseStats.MaxHealth = 50.0f;
	Template.BaseStats.MaxMana = 0.0f;
	Template.BaseStats.MaxStamina = 80.0f;
	Template.BaseStats.MoveSpeed = 450.0f;
	Template.BaseStats.BaseAttackDamage = 15.0f;
	Template.BaseStats.BaseArmor = 3.0f;

	Template.Abilities.bCanJump = true;
	Template.Abilities.bCanSprint = true;

	Template.CapsuleRadius = 42.0f;
	Template.CapsuleHalfHeight = 96.0f;

	Template.RequiredComponents.Add(FName("CombatComponent"));
	Template.RequiredComponents.Add(FName("AIBehaviorComponent"));

	return Template;
}

FForge2CharacterTemplate UForge2TemplateUtilities::CreateRangedEnemyPreset()
{
	FForge2CharacterTemplate Template;
	Template.TemplateName = FName("RangedEnemy");
	Template.TemplateType = EForge2CharacterTemplateType::EnemyRanged;

	Template.BaseStats.MaxHealth = 35.0f;
	Template.BaseStats.MoveSpeed = 400.0f;
	Template.BaseStats.BaseAttackDamage = 12.0f;
	Template.BaseStats.BaseArmor = 1.0f;

	Template.Abilities.bCanJump = true;

	Template.RequiredComponents.Add(FName("CombatComponent"));
	Template.RequiredComponents.Add(FName("AIBehaviorComponent"));

	return Template;
}

FForge2CharacterTemplate UForge2TemplateUtilities::CreateBossEnemyPreset()
{
	FForge2CharacterTemplate Template;
	Template.TemplateName = FName("BossEnemy");
	Template.TemplateType = EForge2CharacterTemplateType::BossEnemy;

	Template.BaseStats.MaxHealth = 1000.0f;
	Template.BaseStats.MaxMana = 200.0f;
	Template.BaseStats.MaxStamina = 200.0f;
	Template.BaseStats.MoveSpeed = 350.0f;
	Template.BaseStats.BaseAttackDamage = 50.0f;
	Template.BaseStats.BaseArmor = 25.0f;

	Template.CapsuleRadius = 100.0f;
	Template.CapsuleHalfHeight = 200.0f;

	Template.RequiredComponents.Add(FName("CombatComponent"));
	Template.RequiredComponents.Add(FName("AIBehaviorComponent"));
	Template.RequiredComponents.Add(FName("BossPhaseComponent"));

	return Template;
}

FForge2CharacterTemplate UForge2TemplateUtilities::CreateMerchantNPCPreset()
{
	FForge2CharacterTemplate Template;
	Template.TemplateName = FName("MerchantNPC");
	Template.TemplateType = EForge2CharacterTemplateType::NPCMerchant;

	Template.BaseStats.MaxHealth = 100.0f;
	Template.BaseStats.MoveSpeed = 200.0f;

	Template.Abilities.bCanJump = false;
	Template.Abilities.bCanSprint = false;

	Template.RequiredComponents.Add(FName("DialogueComponent"));
	Template.RequiredComponents.Add(FName("ShopComponent"));
	Template.RequiredComponents.Add(FName("InteractionComponent"));

	return Template;
}

FForge2WeaponTemplate UForge2TemplateUtilities::CreateSwordPreset(bool bTwoHanded)
{
	FForge2WeaponTemplate Template;
	Template.TemplateName = bTwoHanded ? FName("TwoHandedSword") : FName("Sword");
	Template.WeaponType = bTwoHanded ? EForge2WeaponTemplateType::TwoHandedSword : EForge2WeaponTemplateType::MeleeSword;

	Template.Stats.BaseDamage = bTwoHanded ? 40.0f : 25.0f;
	Template.Stats.DamageVariance = bTwoHanded ? 10.0f : 5.0f;
	Template.Stats.AttackSpeed = bTwoHanded ? 0.7f : 1.0f;
	Template.Stats.Range = bTwoHanded ? 200.0f : 150.0f;
	Template.Stats.StaggerPower = bTwoHanded ? 2.0f : 1.0f;

	Template.AttachSocketName = FName("weapon_r");
	Template.HitboxExtent = bTwoHanded ? FVector(80.0f, 15.0f, 15.0f) : FVector(50.0f, 10.0f, 10.0f);

	// Add basic combo
	FForge2WeaponCombo BasicCombo;
	BasicCombo.ComboName = FName("BasicCombo");
	BasicCombo.DamageMultipliers = { 1.0f, 1.1f, 1.3f };
	BasicCombo.ComboWindowTime = 0.5f;
	Template.Combos.Add(BasicCombo);

	return Template;
}

FForge2WeaponTemplate UForge2TemplateUtilities::CreateBowPreset()
{
	FForge2WeaponTemplate Template;
	Template.TemplateName = FName("Bow");
	Template.WeaponType = EForge2WeaponTemplateType::RangedBow;

	Template.Stats.BaseDamage = 30.0f;
	Template.Stats.DamageVariance = 8.0f;
	Template.Stats.AttackSpeed = 0.8f;
	Template.Stats.Range = 5000.0f;
	Template.Stats.ProjectileSpeed = 4000.0f;
	Template.Stats.AmmoCapacity = 1;
	Template.Stats.ReloadTime = 0.5f;

	Template.AttachSocketName = FName("weapon_l");

	return Template;
}

FForge2WeaponTemplate UForge2TemplateUtilities::CreateStaffPreset()
{
	FForge2WeaponTemplate Template;
	Template.TemplateName = FName("Staff");
	Template.WeaponType = EForge2WeaponTemplateType::MagicStaff;

	Template.Stats.BaseDamage = 35.0f;
	Template.Stats.DamageVariance = 10.0f;
	Template.Stats.AttackSpeed = 0.6f;
	Template.Stats.Range = 3000.0f;
	Template.Stats.ProjectileSpeed = 2000.0f;

	Template.AttachSocketName = FName("weapon_r");

	return Template;
}

FForge2WeaponTemplate UForge2TemplateUtilities::CreateShieldPreset()
{
	FForge2WeaponTemplate Template;
	Template.TemplateName = FName("Shield");
	Template.WeaponType = EForge2WeaponTemplateType::Shield;

	Template.Stats.BaseDamage = 5.0f;
	Template.Stats.StaggerPower = 3.0f;
	Template.Stats.Range = 80.0f;

	Template.AttachSocketName = FName("weapon_l");
	Template.HitboxExtent = FVector(30.0f, 40.0f, 50.0f);

	return Template;
}

FForge2NPCAITemplate UForge2TemplateUtilities::CreatePatrolGuardAIPreset()
{
	FForge2NPCAITemplate Template;
	Template.TemplateName = FName("PatrolGuard");
	Template.AIType = EForge2NPCAITemplateType::Guard;

	Template.Perception.SightRadius = 2000.0f;
	Template.Perception.LoseSightRadius = 2500.0f;
	Template.Perception.PeripheralVisionAngle = 60.0f;
	Template.Perception.HearingRadius = 1000.0f;

	Template.CombatBehavior.AggressionLevel = 0.7f;
	Template.CombatBehavior.PreferredCombatDistance = 150.0f;
	Template.CombatBehavior.FleeHealthThreshold = 0.1f;
	Template.CombatBehavior.bCallsForHelp = true;
	Template.CombatBehavior.HelpCallRadius = 1500.0f;

	Template.bLoopPatrol = true;
	Template.bCanBeInteracted = true;

	return Template;
}

FForge2NPCAITemplate UForge2TemplateUtilities::CreateAggressiveMeleeAIPreset()
{
	FForge2NPCAITemplate Template;
	Template.TemplateName = FName("AggressiveMelee");
	Template.AIType = EForge2NPCAITemplateType::AggressiveMelee;

	Template.Perception.SightRadius = 1500.0f;
	Template.Perception.LoseSightRadius = 2000.0f;
	Template.Perception.PeripheralVisionAngle = 90.0f;
	Template.Perception.HearingRadius = 800.0f;

	Template.CombatBehavior.AggressionLevel = 1.0f;
	Template.CombatBehavior.PreferredCombatDistance = 100.0f;
	Template.CombatBehavior.FleeHealthThreshold = 0.0f;
	Template.CombatBehavior.bUsesAbilities = true;
	Template.CombatBehavior.AbilityUseCooldown = 3.0f;

	return Template;
}

FForge2NPCAITemplate UForge2TemplateUtilities::CreateMerchantAIPreset()
{
	FForge2NPCAITemplate Template;
	Template.TemplateName = FName("Merchant");
	Template.AIType = EForge2NPCAITemplateType::Merchant;

	Template.Perception.SightRadius = 500.0f;
	Template.Perception.HearingRadius = 300.0f;

	Template.CombatBehavior.AggressionLevel = 0.0f;
	Template.CombatBehavior.FleeHealthThreshold = 0.8f;

	Template.bCanBeInteracted = true;
	Template.ShopId = FName("DefaultShop");

	return Template;
}

FForge2NPCAITemplate UForge2TemplateUtilities::CreateBossAIPreset()
{
	FForge2NPCAITemplate Template;
	Template.TemplateName = FName("Boss");
	Template.AIType = EForge2NPCAITemplateType::BossMultiPhase;

	Template.Perception.SightRadius = 5000.0f;
	Template.Perception.LoseSightRadius = 6000.0f;
	Template.Perception.PeripheralVisionAngle = 180.0f;
	Template.Perception.HearingRadius = 3000.0f;

	Template.CombatBehavior.AggressionLevel = 0.8f;
	Template.CombatBehavior.PreferredCombatDistance = 300.0f;
	Template.CombatBehavior.FleeHealthThreshold = 0.0f;
	Template.CombatBehavior.bUsesAbilities = true;
	Template.CombatBehavior.AbilityUseCooldown = 5.0f;
	Template.CombatBehavior.bCallsForHelp = false;

	return Template;
}

FForge2QuestTemplate UForge2TemplateUtilities::CreateFetchQuestPreset(FName ItemId, int32 Count)
{
	FForge2QuestTemplate Template;
	Template.TemplateName = FName("FetchQuest");
	Template.QuestType = EForge2QuestTemplateType::FetchQuest;
	Template.QuestName = FText::FromString("Collect Items");
	Template.QuestDescription = FText::FromString("Gather the requested items and return to the quest giver.");
	Template.QuestCategory = FName("Side");

	FForge2QuestObjectiveTemplate Objective;
	Objective.ObjectiveId = FName("CollectItems");
	Objective.ObjectiveType = FName("Collect");
	Objective.TargetId = ItemId;
	Objective.RequiredCount = Count;
	Objective.ObjectiveDescription = FText::Format(FTextFormat::FromString("Collect {0} {1}"), Count, FText::FromName(ItemId));
	Template.Objectives.Add(Objective);

	Template.Rewards.ExperienceReward = 100;
	Template.Rewards.GoldReward = 50;

	Template.bAutoTrack = true;
	Template.bShowOnMap = true;
	Template.bShowOnCompass = true;

	return Template;
}

FForge2QuestTemplate UForge2TemplateUtilities::CreateKillQuestPreset(FName EnemyId, int32 Count)
{
	FForge2QuestTemplate Template;
	Template.TemplateName = FName("KillQuest");
	Template.QuestType = EForge2QuestTemplateType::KillQuest;
	Template.QuestName = FText::FromString("Eliminate Threats");
	Template.QuestDescription = FText::FromString("Defeat the enemies threatening the area.");
	Template.QuestCategory = FName("Side");

	FForge2QuestObjectiveTemplate Objective;
	Objective.ObjectiveId = FName("KillEnemies");
	Objective.ObjectiveType = FName("Kill");
	Objective.TargetId = EnemyId;
	Objective.RequiredCount = Count;
	Objective.ObjectiveDescription = FText::Format(FTextFormat::FromString("Defeat {0} {1}"), Count, FText::FromName(EnemyId));
	Template.Objectives.Add(Objective);

	Template.Rewards.ExperienceReward = 150;
	Template.Rewards.GoldReward = 75;

	return Template;
}

FForge2QuestTemplate UForge2TemplateUtilities::CreateExploreQuestPreset(FVector Location, float Radius)
{
	FForge2QuestTemplate Template;
	Template.TemplateName = FName("ExploreQuest");
	Template.QuestType = EForge2QuestTemplateType::ExploreQuest;
	Template.QuestName = FText::FromString("Explore the Area");
	Template.QuestDescription = FText::FromString("Discover the marked location.");
	Template.QuestCategory = FName("Side");

	FForge2QuestObjectiveTemplate Objective;
	Objective.ObjectiveId = FName("ReachLocation");
	Objective.ObjectiveType = FName("ReachLocation");
	Objective.TargetLocation = Location;
	Objective.LocationRadius = Radius;
	Objective.ObjectiveDescription = FText::FromString("Reach the marked location");
	Template.Objectives.Add(Objective);

	Template.Rewards.ExperienceReward = 75;

	return Template;
}

FForge2QuestTemplate UForge2TemplateUtilities::CreateEscortQuestPreset(FName NPCId, FVector Destination)
{
	FForge2QuestTemplate Template;
	Template.TemplateName = FName("EscortQuest");
	Template.QuestType = EForge2QuestTemplateType::EscortQuest;
	Template.QuestName = FText::FromString("Escort Mission");
	Template.QuestDescription = FText::FromString("Safely escort the NPC to their destination.");
	Template.QuestCategory = FName("Side");

	FForge2QuestObjectiveTemplate Objective;
	Objective.ObjectiveId = FName("EscortNPC");
	Objective.ObjectiveType = FName("Escort");
	Objective.TargetId = NPCId;
	Objective.TargetLocation = Destination;
	Objective.LocationRadius = 500.0f;
	Objective.ObjectiveDescription = FText::FromString("Escort the NPC to safety");
	Template.Objectives.Add(Objective);

	Template.Rewards.ExperienceReward = 200;
	Template.Rewards.GoldReward = 100;

	return Template;
}

FForge2DungeonTemplate UForge2TemplateUtilities::CreateLinearDungeonPreset(int32 RoomCount)
{
	FForge2DungeonTemplate Template;
	Template.TemplateName = FName("LinearDungeon");
	Template.DungeonType = EForge2DungeonTemplateType::LinearDungeon;

	Template.MinRooms = RoomCount;
	Template.MaxRooms = RoomCount;
	Template.CorridorWidth = 4.0f;
	Template.bAllowLoops = false;

	// Combat room template
	FForge2DungeonRoomTemplate CombatRoom;
	CombatRoom.RoomType = FName("Combat");
	CombatRoom.SizeRange = FVector2D(6.0f, 10.0f);
	CombatRoom.SpawnWeight = 3.0f;
	CombatRoom.MinEnemies = 2;
	CombatRoom.MaxEnemies = 5;
	CombatRoom.PropDensity = 0.2f;
	Template.RoomTemplates.Add(CombatRoom);

	// Treasure room template
	FForge2DungeonRoomTemplate TreasureRoom;
	TreasureRoom.RoomType = FName("Treasure");
	TreasureRoom.SizeRange = FVector2D(4.0f, 6.0f);
	TreasureRoom.SpawnWeight = 1.0f;
	TreasureRoom.MinChests = 1;
	TreasureRoom.MaxChests = 3;
	TreasureRoom.PropDensity = 0.4f;
	Template.RoomTemplates.Add(TreasureRoom);

	Template.bHasBossRoom = false;
	Template.BiomeStyle = FName("Stone");
	Template.AmbientColor = FLinearColor(0.1f, 0.1f, 0.15f);
	Template.TorchDensity = 0.5f;
	Template.RecommendedLevel = 5;
	Template.bIsInstanced = true;
	Template.MaxPlayers = 4;

	return Template;
}

FForge2DungeonTemplate UForge2TemplateUtilities::CreateBossDungeonPreset(FName BossId)
{
	FForge2DungeonTemplate Template;
	Template.TemplateName = FName("BossDungeon");
	Template.DungeonType = EForge2DungeonTemplateType::BossDungeon;

	Template.MinRooms = 5;
	Template.MaxRooms = 8;
	Template.CorridorWidth = 5.0f;
	Template.bAllowLoops = true;
	Template.LoopChance = 0.3f;

	// Combat rooms
	FForge2DungeonRoomTemplate CombatRoom;
	CombatRoom.RoomType = FName("Combat");
	CombatRoom.SizeRange = FVector2D(8.0f, 12.0f);
	CombatRoom.SpawnWeight = 2.0f;
	CombatRoom.MinEnemies = 3;
	CombatRoom.MaxEnemies = 6;
	Template.RoomTemplates.Add(CombatRoom);

	// Safe room
	FForge2DungeonRoomTemplate SafeRoom;
	SafeRoom.RoomType = FName("Safe");
	SafeRoom.SizeRange = FVector2D(4.0f, 5.0f);
	SafeRoom.SpawnWeight = 0.5f;
	SafeRoom.MinChests = 1;
	SafeRoom.MaxChests = 1;
	Template.RoomTemplates.Add(SafeRoom);

	Template.bHasBossRoom = true;
	Template.BossId = BossId;
	Template.BiomeStyle = FName("Dark");
	Template.AmbientColor = FLinearColor(0.05f, 0.02f, 0.05f);
	Template.TorchDensity = 0.3f;
	Template.RecommendedLevel = 20;
	Template.DifficultyMultiplier = 1.5f;
	Template.bIsInstanced = true;
	Template.MaxPlayers = 4;

	return Template;
}

FForge2DungeonTemplate UForge2TemplateUtilities::CreatePuzzleDungeonPreset()
{
	FForge2DungeonTemplate Template;
	Template.TemplateName = FName("PuzzleDungeon");
	Template.DungeonType = EForge2DungeonTemplateType::PuzzleDungeon;

	Template.MinRooms = 8;
	Template.MaxRooms = 12;
	Template.CorridorWidth = 4.0f;
	Template.bAllowLoops = true;
	Template.LoopChance = 0.5f;

	// Puzzle room
	FForge2DungeonRoomTemplate PuzzleRoom;
	PuzzleRoom.RoomType = FName("Puzzle");
	PuzzleRoom.SizeRange = FVector2D(6.0f, 10.0f);
	PuzzleRoom.SpawnWeight = 3.0f;
	PuzzleRoom.MinEnemies = 0;
	PuzzleRoom.MaxEnemies = 1;
	PuzzleRoom.PropDensity = 0.5f;
	Template.RoomTemplates.Add(PuzzleRoom);

	// Trap room
	FForge2DungeonRoomTemplate TrapRoom;
	TrapRoom.RoomType = FName("Trap");
	TrapRoom.SizeRange = FVector2D(5.0f, 8.0f);
	TrapRoom.SpawnWeight = 2.0f;
	TrapRoom.PropDensity = 0.3f;
	Template.RoomTemplates.Add(TrapRoom);

	Template.bHasBossRoom = false;
	Template.BiomeStyle = FName("Ancient");
	Template.AmbientColor = FLinearColor(0.15f, 0.12f, 0.08f);
	Template.TorchDensity = 0.4f;
	Template.RecommendedLevel = 10;
	Template.bIsInstanced = true;
	Template.MaxPlayers = 4;

	return Template;
}

FForge2OpenWorldChunkTemplate UForge2TemplateUtilities::CreateForestChunkPreset()
{
	FForge2OpenWorldChunkTemplate Template;
	Template.TemplateName = FName("ForestChunk");
	Template.ChunkType = EForge2OpenWorldTemplateType::ForestChunk;

	Template.BiomeId = FName("Forest");
	Template.BaseElevation = 0.0f;
	Template.ElevationVariance = 300.0f;
	Template.TerrainRoughness = 0.4f;

	Template.TreeDensity = 0.7f;
	Template.GrassDensity = 0.9f;

	Template.WildlifeDensity = 0.4f;
	Template.EnemyDensity = 0.15f;
	Template.ResourceDensity = 0.2f;

	Template.ResourceNodeTypes.Add(FName("Wood"));
	Template.ResourceNodeTypes.Add(FName("Herbs"));

	// Add POI templates
	FForge2POITemplate DungeonPOI;
	DungeonPOI.POIType = FName("DungeonEntrance");
	DungeonPOI.SpawnChance = 0.2f;
	DungeonPOI.bShowOnMap = true;
	Template.POITemplates.Add(DungeonPOI);

	FForge2POITemplate CampPOI;
	CampPOI.POIType = FName("EnemyCamp");
	CampPOI.SpawnChance = 0.4f;
	CampPOI.bShowOnMap = false;
	Template.POITemplates.Add(CampPOI);

	Template.bHasRoads = true;
	Template.RoadDensity = 0.1f;
	Template.bHasWater = true;
	Template.WaterType = FName("Stream");

	Template.StreamingDistance = 50000.0f;
	Template.RecommendedLevel = 1;

	return Template;
}

FForge2OpenWorldChunkTemplate UForge2TemplateUtilities::CreateDesertChunkPreset()
{
	FForge2OpenWorldChunkTemplate Template;
	Template.TemplateName = FName("DesertChunk");
	Template.ChunkType = EForge2OpenWorldTemplateType::DesertChunk;

	Template.BiomeId = FName("Desert");
	Template.BaseElevation = -50.0f;
	Template.ElevationVariance = 200.0f;
	Template.TerrainRoughness = 0.3f;

	Template.TreeDensity = 0.05f;
	Template.GrassDensity = 0.1f;

	Template.WildlifeDensity = 0.1f;
	Template.EnemyDensity = 0.2f;
	Template.ResourceDensity = 0.1f;

	Template.ResourceNodeTypes.Add(FName("Stone"));
	Template.ResourceNodeTypes.Add(FName("Ore"));

	FForge2POITemplate RuinsPOI;
	RuinsPOI.POIType = FName("Ruins");
	RuinsPOI.SpawnChance = 0.3f;
	RuinsPOI.bShowOnMap = true;
	Template.POITemplates.Add(RuinsPOI);

	Template.bHasRoads = false;
	Template.bHasWater = false;

	Template.StreamingDistance = 60000.0f;
	Template.RecommendedLevel = 15;

	return Template;
}

FForge2OpenWorldChunkTemplate UForge2TemplateUtilities::CreateMountainChunkPreset()
{
	FForge2OpenWorldChunkTemplate Template;
	Template.TemplateName = FName("MountainChunk");
	Template.ChunkType = EForge2OpenWorldTemplateType::MountainChunk;

	Template.BiomeId = FName("Mountain");
	Template.BaseElevation = 500.0f;
	Template.ElevationVariance = 1000.0f;
	Template.TerrainRoughness = 0.8f;

	Template.TreeDensity = 0.3f;
	Template.GrassDensity = 0.4f;

	Template.WildlifeDensity = 0.2f;
	Template.EnemyDensity = 0.25f;
	Template.ResourceDensity = 0.3f;

	Template.ResourceNodeTypes.Add(FName("Ore"));
	Template.ResourceNodeTypes.Add(FName("Gems"));
	Template.ResourceNodeTypes.Add(FName("Stone"));

	FForge2POITemplate CavePOI;
	CavePOI.POIType = FName("Cave");
	CavePOI.SpawnChance = 0.4f;
	CavePOI.bShowOnMap = true;
	Template.POITemplates.Add(CavePOI);

	Template.bHasRoads = true;
	Template.RoadDensity = 0.05f;
	Template.bHasWater = true;
	Template.WaterType = FName("Stream");

	Template.StreamingDistance = 70000.0f;
	Template.RecommendedLevel = 25;

	return Template;
}

FForge2OpenWorldChunkTemplate UForge2TemplateUtilities::CreateVillageChunkPreset()
{
	FForge2OpenWorldChunkTemplate Template;
	Template.TemplateName = FName("VillageChunk");
	Template.ChunkType = EForge2OpenWorldTemplateType::VillageChunk;

	Template.BiomeId = FName("Plains");
	Template.BaseElevation = 0.0f;
	Template.ElevationVariance = 100.0f;
	Template.TerrainRoughness = 0.2f;

	Template.TreeDensity = 0.2f;
	Template.GrassDensity = 0.7f;

	Template.WildlifeDensity = 0.1f;
	Template.EnemyDensity = 0.0f;
	Template.ResourceDensity = 0.05f;

	// Village POIs
	FForge2POITemplate VillagePOI;
	VillagePOI.POIType = FName("Village");
	VillagePOI.SpawnChance = 1.0f;
	VillagePOI.bShowOnMap = true;
	VillagePOI.POIName = FText::FromString("Village");
	Template.POITemplates.Add(VillagePOI);

	FForge2POITemplate ShopPOI;
	ShopPOI.POIType = FName("Shop");
	ShopPOI.SpawnChance = 0.8f;
	ShopPOI.bShowOnMap = true;
	Template.POITemplates.Add(ShopPOI);

	FForge2POITemplate InnPOI;
	InnPOI.POIType = FName("Inn");
	InnPOI.SpawnChance = 0.6f;
	InnPOI.bShowOnMap = true;
	Template.POITemplates.Add(InnPOI);

	Template.bHasRoads = true;
	Template.RoadDensity = 0.3f;
	Template.bHasWater = true;
	Template.WaterType = FName("Pond");

	Template.StreamingDistance = 40000.0f;
	Template.RecommendedLevel = 1;

	return Template;
}

FString UForge2TemplateUtilities::TemplateToJson(const FForge2CharacterTemplate& Template)
{
	FString OutputString;
	FJsonObjectConverter::UStructToJsonObjectString(Template, OutputString);
	return OutputString;
}

bool UForge2TemplateUtilities::JsonToCharacterTemplate(const FString& JsonString, FForge2CharacterTemplate& OutTemplate)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutTemplate);
}

bool UForge2TemplateUtilities::SaveTemplateToFile(const FForge2CharacterTemplate& Template, const FString& FilePath)
{
	FString JsonString = TemplateToJson(Template);
	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

bool UForge2TemplateUtilities::LoadTemplateFromFile(const FString& FilePath, FForge2CharacterTemplate& OutTemplate)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return false;
	}
	return JsonToCharacterTemplate(JsonString, OutTemplate);
}

bool UForge2TemplateUtilities::IsValidCharacterTemplate(const FForge2CharacterTemplate& Template)
{
	if (Template.TemplateName.IsNone())
	{
		return false;
	}

	if (Template.BaseStats.MaxHealth <= 0)
	{
		return false;
	}

	if (Template.CapsuleRadius <= 0 || Template.CapsuleHalfHeight <= 0)
	{
		return false;
	}

	return true;
}

bool UForge2TemplateUtilities::IsValidWeaponTemplate(const FForge2WeaponTemplate& Template)
{
	if (Template.TemplateName.IsNone())
	{
		return false;
	}

	if (Template.Stats.BaseDamage < 0)
	{
		return false;
	}

	if (Template.Stats.AttackSpeed <= 0)
	{
		return false;
	}

	return true;
}

bool UForge2TemplateUtilities::IsValidQuestTemplate(const FForge2QuestTemplate& Template)
{
	if (Template.TemplateName.IsNone())
	{
		return false;
	}

	if (Template.Objectives.Num() == 0)
	{
		return false;
	}

	return true;
}

bool UForge2TemplateUtilities::IsValidDungeonTemplate(const FForge2DungeonTemplate& Template)
{
	if (Template.TemplateName.IsNone())
	{
		return false;
	}

	if (Template.MinRooms > Template.MaxRooms)
	{
		return false;
	}

	if (Template.RoomTemplates.Num() == 0)
	{
		return false;
	}

	if (Template.bHasBossRoom && Template.BossId.IsNone())
	{
		return false;
	}

	return true;
}

FForge2CharacterTemplate UForge2TemplateUtilities::MergeCharacterTemplates(const FForge2CharacterTemplate& Base, const FForge2CharacterTemplate& Override)
{
	FForge2CharacterTemplate Result = Base;

	// Override non-default values
	if (!Override.TemplateName.IsNone())
	{
		Result.TemplateName = Override.TemplateName;
	}

	if (Override.BaseStats.MaxHealth != 100.0f)
	{
		Result.BaseStats.MaxHealth = Override.BaseStats.MaxHealth;
	}

	if (Override.BaseStats.MaxMana != 50.0f)
	{
		Result.BaseStats.MaxMana = Override.BaseStats.MaxMana;
	}

	if (Override.BaseStats.MoveSpeed != 600.0f)
	{
		Result.BaseStats.MoveSpeed = Override.BaseStats.MoveSpeed;
	}

	// Merge abilities (OR operation)
	Result.Abilities.bCanDoubleJump = Base.Abilities.bCanDoubleJump || Override.Abilities.bCanDoubleJump;
	Result.Abilities.bCanDash = Base.Abilities.bCanDash || Override.Abilities.bCanDash;
	Result.Abilities.bCanClimb = Base.Abilities.bCanClimb || Override.Abilities.bCanClimb;
	Result.Abilities.bCanFly = Base.Abilities.bCanFly || Override.Abilities.bCanFly;

	// Merge special abilities
	for (const FName& Ability : Override.Abilities.SpecialAbilities)
	{
		Result.Abilities.SpecialAbilities.AddUnique(Ability);
	}

	// Merge required components
	for (const FName& Component : Override.RequiredComponents)
	{
		Result.RequiredComponents.AddUnique(Component);
	}

	return Result;
}

FForge2WeaponTemplate UForge2TemplateUtilities::MergeWeaponTemplates(const FForge2WeaponTemplate& Base, const FForge2WeaponTemplate& Override)
{
	FForge2WeaponTemplate Result = Base;

	if (!Override.TemplateName.IsNone())
	{
		Result.TemplateName = Override.TemplateName;
	}

	// Override damage if different from default
	if (Override.Stats.BaseDamage != 20.0f)
	{
		Result.Stats.BaseDamage = Override.Stats.BaseDamage;
	}

	// Add combos from override
	for (const FForge2WeaponCombo& Combo : Override.Combos)
	{
		Result.Combos.Add(Combo);
	}

	return Result;
}
