// Copyright FORGE2 Project. All Rights Reserved.
// FORGE2: Ultimate AAA Game Development MCP Extension
// Additional Data Asset Implementations - Part 2

#include "Forge2DataAssets2.h"

// ============================================================================
// World Generation Data Asset Constructors
// ============================================================================

UMcpStructureTemplateAsset::UMcpStructureTemplateAsset()
{
    BuildingStyle = EForge2BuildingStyle::Medieval;
    RoofType = EForge2RoofType::Gabled;
    FootprintRange = FVector2D(500.0f, 2000.0f);
    MinFloors = 1;
    MaxFloors = 3;
    FloorHeight = 300.0f;
    WindowDensity = 0.5f;
    DoorCount = 1.0f;
    bIncludeBalconies = false;
    bIncludeChimneys = true;
    bGenerateInterior = true;
    MinRoomsPerFloor = 2;
    MaxRoomsPerFloor = 6;
    FurnitureDensity = 0.6f;
}

UMcpCityLayoutAsset::UMcpCityLayoutAsset()
{
    CityBounds = FVector2D(10000.0f, 10000.0f);
    PopulationTarget = 1000;
    MinDistricts = 3;
    MaxDistricts = 8;
    MainRoadWidth = 800.0f;
    SideRoadWidth = 400.0f;
    bGridLayout = false;
    RoadDensity = 0.5f;
    bIncludeWalls = true;
    GateCount = 4;
    bIncludePlazas = true;
    LandmarkCount = 3;
    BuildingDensity = 0.7f;

    // Default district weights
    DistrictWeights.Add(FName(TEXT("Residential")), 0.4f);
    DistrictWeights.Add(FName(TEXT("Commercial")), 0.25f);
    DistrictWeights.Add(FName(TEXT("Industrial")), 0.15f);
    DistrictWeights.Add(FName(TEXT("Civic")), 0.1f);
    DistrictWeights.Add(FName(TEXT("Religious")), 0.1f);
}

UMcpTerrainPresetAsset::UMcpTerrainPresetAsset()
{
    NoiseType = EForge2NoiseType::FBM;
    Seed = 0;
    Scale = 0.001f;
    Octaves = 6;
    Persistence = 0.5f;
    Lacunarity = 2.0f;
    Amplitude = 5000.0f;
    bApplyThermalErosion = true;
    ThermalErosionIterations = 50;
    bApplyHydraulicErosion = true;
    HydraulicErosionIterations = 100;
    HeightRange = FVector2D(0.0f, 5000.0f);
}

UMcpVegetationPresetAsset::UMcpVegetationPresetAsset()
{
    GlobalDensityMultiplier = 1.0f;
    SlopeRange = FVector2D(0.0f, 45.0f);
    ElevationRange = FVector2D(-1000.0f, 5000.0f);
    bUseNoiseMask = true;
}

// ============================================================================
// Extended Game Systems Constructors
// ============================================================================

UMcpQuestChainAsset::UMcpQuestChainAsset()
{
    bLinear = true;
}

UMcpSpeakerProfileAsset::UMcpSpeakerProfileAsset()
{
    NameColor = FLinearColor::White;
    VoicePitch = 1.0f;
}

UMcpSkillTreeAsset::UMcpSkillTreeAsset()
{
    TreeBounds = FVector2D(1000.0f, 1500.0f);
    UnlockLevel = 1;
}

UMcpSubclassDefinitionAsset::UMcpSubclassDefinitionAsset()
{
    UnlockLevel = 10;
}

UMcpAffixPoolAsset::UMcpAffixPoolAsset()
{
    MaxPrefixes = 3;
    MaxSuffixes = 3;
}

UMcpShopDefinitionAsset::UMcpShopDefinitionAsset()
{
    AcceptedCurrency = FName(TEXT("Gold"));
    BuyPriceMultiplier = 1.0f;
    SellPriceMultiplier = 0.5f;
    bDynamicPricing = false;
    RefreshInterval = 0.0f;
    bRestockOnRefresh = true;
}

UMcpAchievementDefinitionAsset::UMcpAchievementDefinitionAsset()
{
    Type = EForge2AchievementType::OneTime;
    Points = 10;
    bSecret = false;
    TargetCount = 1;
}

UMcpStatDefinitionAsset::UMcpStatDefinitionAsset()
{
    StatType = EForge2StatType::Strength;
    BaseValue = 10.0f;
    MinValue = 0.0f;
    MaxValue = 9999.0f;
    bIsDerived = false;
}

UMcpDamageTypeDefinitionAsset::UMcpDamageTypeDefinitionAsset()
{
    DamageType = EForge2DamageType::Physical;
    DamageColor = FLinearColor::Red;
    StatusEffectChance = 0.0f;
    ResistanceStat = EForge2StatType::Armor;
    ResistanceMultiplier = 1.0f;
}

UMcpCombatProfileAsset::UMcpCombatProfileAsset()
{
    Aggression = EForge2AIAggression::Neutral;
    DetectionRange = 2000.0f;
    AttackRange = 200.0f;
    bPrioritizeLowHealth = false;
    bPrioritizeHighThreat = true;
    FleeHealthPercent = 0.0f;
    bCallForHelp = true;
    HelpCallRadius = 1000.0f;
    AbilityCooldownMultiplier = 1.0f;
}

// ============================================================================
// Genre-Specific Data Asset Constructors
// ============================================================================

UMcpVehicleDefinitionAsset::UMcpVehicleDefinitionAsset()
{
    VehicleType = EForge2VehicleType::Car;
    DriveTrain = EForge2DriveTrain::RWD;
    MaxSpeed = 200.0f;
    Acceleration = 8.0f;
    BrakingForce = 1.0f;
    Handling = 0.7f;
    Mass = 1500.0f;
    CenterOfMass = FVector::ZeroVector;
    Downforce = 0.5f;
    TireGrip = 1.0f;
    bSimulateTireWear = false;
    bSimulateTireTemp = false;
    bVisualDamage = true;
    bMechanicalDamage = false;
}

UMcpTrackDefinitionAsset::UMcpTrackDefinitionAsset()
{
    TrackLength = 5000.0f;
    LapCount = 3;
    CheckpointCount = 10;
    bNightRace = false;
    bWeatherEffects = false;
    DifficultyRating = 3;
    MaxRacers = 12;
}

UMcpFighterDefinitionAsset::UMcpFighterDefinitionAsset()
{
    Health = 1000.0f;
    WalkSpeed = 1.0f;
    DashSpeed = 1.5f;
    JumpHeight = 1.0f;
    StandingHurtboxSize = FVector(50, 30, 180);
    CrouchingHurtboxSize = FVector(50, 30, 100);
}

UMcpPlatformDefinitionAsset::UMcpPlatformDefinitionAsset()
{
    Behavior = EForge2PlatformBehavior::Static;
    MoveDistance = FVector(500, 0, 0);
    MoveSpeed = 200.0f;
    WaitTime = 1.0f;
    FallDelay = 0.5f;
    RespawnTime = 3.0f;
    BounceForce = 1000.0f;
    ConveyorDirection = FVector(1, 0, 0);
    ConveyorSpeed = 200.0f;
    FrictionMultiplier = 0.1f;
}

UMcpCardDefinitionAsset::UMcpCardDefinitionAsset()
{
    CardType = EForge2CardType::Creature;
    Rarity = EForge2Rarity::Common;
    ManaCost = 1;
    Attack = 0;
    Health = 0;
}

UMcpDeckDefinitionAsset::UMcpDeckDefinitionAsset()
{
    MinCards = 30;
    MaxCards = 60;
    MaxCopiesPerCard = 2;
}

UMcpHeroDefinitionAsset::UMcpHeroDefinitionAsset()
{
    BaseHealth = 500.0f;
    BaseMana = 300.0f;
    BaseAttackDamage = 50.0f;
    AttackRange = 150.0f;
    MoveSpeed = 340.0f;
    HealthPerLevel = 80.0f;
    ManaPerLevel = 40.0f;
    AttackDamagePerLevel = 3.0f;
}

UMcpUnitDefinitionAsset::UMcpUnitDefinitionAsset()
{
    Category = EForge2UnitCategory::Infantry;
    SupplyCost = 1;
    BuildTime = 10.0f;
    Health = 100.0f;
    Armor = 0.0f;
    MoveSpeed = 200.0f;
    SightRange = 1000.0f;
    AttackDamage = 10.0f;
    AttackRange = 100.0f;
    AttackCooldown = 1.0f;
}

UMcpRTSBuildingDefinitionAsset::UMcpRTSBuildingDefinitionAsset()
{
    BuildTime = 30.0f;
    Health = 1000.0f;
    Armor = 2.0f;
    ProductionQueueSize = 5;
    SupplyProvided = 0;
}

UMcpNeedsDefinitionAsset::UMcpNeedsDefinitionAsset()
{
    MaxValue = 100.0f;
    StartValue = 100.0f;
    DecayRate = 1.0f;
    LowThreshold = 30.0f;
    CriticalThreshold = 10.0f;
    DamageThreshold = 0.0f;
    DamagePerSecond = 0.0f;
}

// ============================================================================
// Environment Data Asset Constructors
// ============================================================================

UMcpWeatherPresetAsset::UMcpWeatherPresetAsset()
{
    WeatherType = EForge2WeatherType::Clear;
    FogDensity = 0.0f;
    FogColor = FLinearColor::White;
    CloudCoverage = 0.0f;
    SunIntensityMultiplier = 1.0f;
    SkyLightIntensityMultiplier = 1.0f;
    AmbientVolume = 1.0f;
    VisibilityMultiplier = 1.0f;
}

UMcpTimeOfDayPresetAsset::UMcpTimeOfDayPresetAsset()
{
    TimeHour = 12.0f;
    SunAngle = 60.0f;
    SunColor = FLinearColor::White;
    SunIntensity = 5.0f;
    SkyColor = FLinearColor(0.2f, 0.4f, 0.8f);
    HorizonColor = FLinearColor(0.8f, 0.6f, 0.4f);
    StarVisibility = 0.0f;
    FogInscatteringColor = FLinearColor::White;
    ExposureCompensation = 0.0f;
    BloomIntensity = 0.5f;
}

UMcpSeasonPresetAsset::UMcpSeasonPresetAsset()
{
    Season = EForge2Season::Summer;
    TemperatureRange = FVector2D(15.0f, 25.0f);
    FoliageDensityMultiplier = 1.0f;
    FoliageColorTint = FLinearColor::White;
    bShowFlowers = true;
    bShowSnow = false;
    DaytimeHours = 12.0f;

    // Default weather probabilities for summer
    WeatherProbabilities.Add(EForge2WeatherType::Clear, 0.5f);
    WeatherProbabilities.Add(EForge2WeatherType::Cloudy, 0.25f);
    WeatherProbabilities.Add(EForge2WeatherType::Rain, 0.2f);
    WeatherProbabilities.Add(EForge2WeatherType::Storm, 0.05f);
}

UMcpAnimalSpeciesAsset::UMcpAnimalSpeciesAsset()
{
    DefaultBehavior = EForge2AnimalBehavior::Neutral;
    bHerdAnimal = false;
    HerdSize = 1;
    Health = 100.0f;
    MoveSpeed = 400.0f;
    DetectionRange = 1000.0f;
    FleeRange = 1500.0f;
    bNocturnal = false;
    bTameable = false;
}

UMcpNPCScheduleAsset::UMcpNPCScheduleAsset()
{
    bInterruptible = true;
    WanderRadius = 200.0f;
}

UMcpNPCDialogueAsset::UMcpNPCDialogueAsset()
{
    IdleChatterChance = 0.1f;
}

// ============================================================================
// UI/Polish Data Asset Constructors
// ============================================================================

UMcpUIThemeAsset::UMcpUIThemeAsset()
{
    PrimaryColor = FLinearColor(0.2f, 0.6f, 0.9f);
    SecondaryColor = FLinearColor(0.1f, 0.1f, 0.15f);
    AccentColor = FLinearColor(0.9f, 0.7f, 0.2f);
    TextColor = FLinearColor::White;
    ErrorColor = FLinearColor::Red;
    SuccessColor = FLinearColor::Green;
    HeaderFontSize = 24;
    BodyFontSize = 14;
    DefaultPadding = FMargin(10.0f);
    ElementSpacing = 5.0f;
}

UMcpInputPresetAsset::UMcpInputPresetAsset()
{
    MouseSensitivity = 1.0f;
    ControllerSensitivity = 1.0f;
    AimSensitivityMultiplier = 0.5f;
    bInvertMouseY = false;
    bInvertControllerY = false;
    LeftStickDeadzone = 0.2f;
    RightStickDeadzone = 0.2f;
    TriggerDeadzone = 0.1f;
    bEnableVibration = true;
    VibrationIntensity = 1.0f;
}

UMcpPhotoModePresetAsset::UMcpPhotoModePresetAsset()
{
    Saturation = 1.0f;
    Contrast = 1.0f;
    Brightness = 1.0f;
    ColorTint = FLinearColor::White;
    FocalDistance = 1000.0f;
    Aperture = 2.8f;
    VignetteIntensity = 0.0f;
    FilmGrain = 0.0f;
    ChromaticAberration = 0.0f;
}

UMcpAccessibilityPresetAsset::UMcpAccessibilityPresetAsset()
{
    bEnableColorblindMode = false;
    ColorblindType = 0;
    ColorblindStrength = 1.0f;
    bEnableSubtitles = true;
    SubtitleScale = 1.0f;
    bSubtitleBackground = true;
    bShowSpeakerNames = true;
    bEnableAudioCues = false;
    bEnableScreenReader = false;
    bHoldToToggle = false;
    bAutoAim = false;
    AutoAimStrength = 0.5f;
    UIScale = 1.0f;
}
