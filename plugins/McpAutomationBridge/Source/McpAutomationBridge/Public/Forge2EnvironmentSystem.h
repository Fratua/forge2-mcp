// Forge2EnvironmentSystem.h
// Phase 13: Environment Systems - Day/Night, Weather, Ecosystem, NPC Behaviors
// FORGE2 Ultimate AAA Game Development MCP Extension

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Forge2EnvironmentSystem.generated.h"

// ============================================================================
// ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EForge2TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Evening     UMETA(DisplayName = "Evening"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class EForge2Season : uint8
{
    Spring      UMETA(DisplayName = "Spring"),
    Summer      UMETA(DisplayName = "Summer"),
    Autumn      UMETA(DisplayName = "Autumn"),
    Winter      UMETA(DisplayName = "Winter")
};

UENUM(BlueprintType)
enum class EForge2WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Snowy       UMETA(DisplayName = "Snowy"),
    Blizzard    UMETA(DisplayName = "Blizzard"),
    Sandstorm   UMETA(DisplayName = "Sandstorm"),
    Heatwave    UMETA(DisplayName = "Heatwave")
};

UENUM(BlueprintType)
enum class EForge2WeatherIntensity : uint8
{
    None        UMETA(DisplayName = "None"),
    Light       UMETA(DisplayName = "Light"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

UENUM(BlueprintType)
enum class EForge2AnimalBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Resting     UMETA(DisplayName = "Resting"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Mating      UMETA(DisplayName = "Mating"),
    Defending   UMETA(DisplayName = "Defending")
};

UENUM(BlueprintType)
enum class EForge2AnimalDiet : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
    Scavenger   UMETA(DisplayName = "Scavenger")
};

UENUM(BlueprintType)
enum class EForge2AnimalSize : uint8
{
    Tiny        UMETA(DisplayName = "Tiny"),
    Small       UMETA(DisplayName = "Small"),
    Medium      UMETA(DisplayName = "Medium"),
    Large       UMETA(DisplayName = "Large"),
    Huge        UMETA(DisplayName = "Huge"),
    Colossal    UMETA(DisplayName = "Colossal")
};

UENUM(BlueprintType)
enum class EForge2NPCActivity : uint8
{
    Working     UMETA(DisplayName = "Working"),
    Shopping    UMETA(DisplayName = "Shopping"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Socializing UMETA(DisplayName = "Socializing"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Traveling   UMETA(DisplayName = "Traveling"),
    Relaxing    UMETA(DisplayName = "Relaxing"),
    Praying     UMETA(DisplayName = "Praying"),
    Training    UMETA(DisplayName = "Training"),
    Custom      UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EForge2NPCMood : uint8
{
    Happy       UMETA(DisplayName = "Happy"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Sad         UMETA(DisplayName = "Sad"),
    Angry       UMETA(DisplayName = "Angry"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Excited     UMETA(DisplayName = "Excited"),
    Tired       UMETA(DisplayName = "Tired")
};

UENUM(BlueprintType)
enum class EForge2BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Tundra      UMETA(DisplayName = "Tundra"),
    Grassland   UMETA(DisplayName = "Grassland"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Jungle      UMETA(DisplayName = "Jungle"),
    Ocean       UMETA(DisplayName = "Ocean"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Urban       UMETA(DisplayName = "Urban")
};

// ============================================================================
// STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2TimeSettings
{
    GENERATED_BODY()

    // Real seconds per in-game hour
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SecondsPerHour = 60.0f;

    // Starting hour (0-23)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0", ClampMax = "23"))
    int32 StartingHour = 8;

    // Starting minute (0-59)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0", ClampMax = "59"))
    int32 StartingMinute = 0;

    // Starting day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "1"))
    int32 StartingDay = 1;

    // Starting month
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "1", ClampMax = "12"))
    int32 StartingMonth = 6;

    // Starting year
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    int32 StartingYear = 1;

    // Days per month
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "1"))
    int32 DaysPerMonth = 30;

    // Months per year
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "1"))
    int32 MonthsPerYear = 12;

    // Pause time when paused
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bPauseTimeWhenPaused = true;

    // Allow time manipulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAllowTimeManipulation = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SunSettings
{
    GENERATED_BODY()

    // Sun light actor reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    TSoftObjectPtr<AActor> SunLightActor;

    // Sunrise hour
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun", meta = (ClampMin = "0", ClampMax = "12"))
    float SunriseHour = 6.0f;

    // Sunset hour
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun", meta = (ClampMin = "12", ClampMax = "24"))
    float SunsetHour = 18.0f;

    // Sun color at dawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor DawnColor = FLinearColor(1.0f, 0.6f, 0.4f);

    // Sun color at noon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor NoonColor = FLinearColor(1.0f, 0.98f, 0.95f);

    // Sun color at dusk
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor DuskColor = FLinearColor(1.0f, 0.5f, 0.3f);

    // Sun intensity during day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float DayIntensity = 10.0f;

    // Sun intensity at night (moonlight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float NightIntensity = 0.1f;

    // North direction offset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float NorthOffset = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SkySettings
{
    GENERATED_BODY()

    // Sky atmosphere actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    TSoftObjectPtr<AActor> SkyAtmosphereActor;

    // Sky light actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    TSoftObjectPtr<AActor> SkyLightActor;

    // Enable stars at night
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    bool bEnableStars = true;

    // Star brightness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float StarBrightness = 1.0f;

    // Moon texture
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    TSoftObjectPtr<UTexture2D> MoonTexture;

    // Moon phase duration (days)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky", meta = (ClampMin = "1"))
    int32 MoonPhaseDays = 8;

    // Enable aurora at night (polar regions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    bool bEnableAurora = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WeatherState
{
    GENERATED_BODY()

    // Current weather type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EForge2WeatherType WeatherType = EForge2WeatherType::Clear;

    // Weather intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EForge2WeatherIntensity Intensity = EForge2WeatherIntensity::None;

    // Cloud coverage (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0", ClampMax = "1"))
    float CloudCoverage = 0.2f;

    // Wind direction (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0", ClampMax = "360"))
    float WindDirection = 0.0f;

    // Wind speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0"))
    float WindSpeed = 5.0f;

    // Precipitation amount (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0", ClampMax = "1"))
    float Precipitation = 0.0f;

    // Temperature in Celsius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 20.0f;

    // Humidity (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0", ClampMax = "1"))
    float Humidity = 0.5f;

    // Fog density (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0", ClampMax = "1"))
    float FogDensity = 0.0f;

    // Lightning frequency (strikes per minute)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0"))
    float LightningFrequency = 0.0f;

    // Visibility range in meters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0"))
    float VisibilityRange = 10000.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WeatherTransition
{
    GENERATED_BODY()

    // Target weather state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FForge2WeatherState TargetState;

    // Transition duration in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float Duration = 30.0f;

    // Use ease in/out
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bUseEasing = true;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2WeatherEffects
{
    GENERATED_BODY()

    // Rain particle system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UNiagaraSystem> RainEffect;

    // Snow particle system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UNiagaraSystem> SnowEffect;

    // Fog volume
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<AActor> FogVolume;

    // Lightning effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UNiagaraSystem> LightningEffect;

    // Sandstorm effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UNiagaraSystem> SandstormEffect;

    // Wind audio cue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<USoundBase> WindSound;

    // Rain audio cue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<USoundBase> RainSound;

    // Thunder audio cue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<USoundBase> ThunderSound;

    // Wet surface material function
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UMaterialFunction> WetSurfaceFunction;

    // Snow accumulation material
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UMaterialInstance> SnowAccumulationMaterial;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SeasonSettings
{
    GENERATED_BODY()

    // Season type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season")
    EForge2Season Season = EForge2Season::Summer;

    // Days per season
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season", meta = (ClampMin = "1"))
    int32 DaysPerSeason = 30;

    // Temperature modifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season")
    float TemperatureModifier = 0.0f;

    // Day length modifier (hours)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season")
    float DayLengthModifier = 0.0f;

    // Foliage color tint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season")
    FLinearColor FoliageColorTint = FLinearColor::White;

    // Grass color tint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season")
    FLinearColor GrassColorTint = FLinearColor::White;

    // Weather probability weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season")
    TMap<EForge2WeatherType, float> WeatherProbabilities;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AnimalSpecies
{
    GENERATED_BODY()

    // Species ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FName SpeciesId;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FText DisplayName;

    // Diet type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EForge2AnimalDiet Diet = EForge2AnimalDiet::Herbivore;

    // Size category
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EForge2AnimalSize Size = EForge2AnimalSize::Medium;

    // Preferred biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<EForge2BiomeType> PreferredBiomes;

    // Active during day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bDiurnal = true;

    // Active during night
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bNocturnal = false;

    // Is predator
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPredator = false;

    // Prey species IDs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FName> PreySpecies;

    // Predator species IDs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FName> PredatorSpecies;

    // Herd size range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FIntPoint HerdSizeRange = FIntPoint(3, 8);

    // Aggression level (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species", meta = (ClampMin = "0", ClampMax = "1"))
    float AggressionLevel = 0.2f;

    // Fear level (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species", meta = (ClampMin = "0", ClampMax = "1"))
    float FearLevel = 0.5f;

    // Curiosity level (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species", meta = (ClampMin = "0", ClampMax = "1"))
    float CuriosityLevel = 0.3f;

    // Movement speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MovementSpeed = 300.0f;

    // Run speed multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float RunSpeedMultiplier = 2.0f;

    // Can be tamed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bCanBeTamed = false;

    // Tameness difficulty (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species", meta = (ClampMin = "0", ClampMax = "1"))
    float TamingDifficulty = 0.5f;

    // Animal blueprint class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TSoftClassPtr<AActor> AnimalClass;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2AnimalInstance
{
    GENERATED_BODY()

    // Unique ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FGuid InstanceId;

    // Species ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FName SpeciesId;

    // Current behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    EForge2AnimalBehavior CurrentBehavior = EForge2AnimalBehavior::Idle;

    // Actor reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    TWeakObjectPtr<AActor> ActorRef;

    // Herd ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FGuid HerdId;

    // Is tamed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    bool bIsTamed = false;

    // Owner player (if tamed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FGuid OwnerId;

    // Current health (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (ClampMin = "0", ClampMax = "1"))
    float Health = 1.0f;

    // Current hunger (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (ClampMin = "0", ClampMax = "1"))
    float Hunger = 0.0f;

    // Current stamina (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (ClampMin = "0", ClampMax = "1"))
    float Stamina = 1.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2HerdInfo
{
    GENERATED_BODY()

    // Herd ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FGuid HerdId;

    // Species ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FName SpeciesId;

    // Member IDs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    TArray<FGuid> MemberIds;

    // Leader ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FGuid LeaderId;

    // Herd center location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector CenterLocation;

    // Territory center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector TerritoryCenter;

    // Territory radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float TerritoryRadius = 5000.0f;

    // Migration target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector MigrationTarget;

    // Is migrating
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    bool bIsMigrating = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NPCScheduleEntry
{
    GENERATED_BODY()

    // Activity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    EForge2NPCActivity Activity = EForge2NPCActivity::Working;

    // Start hour
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule", meta = (ClampMin = "0", ClampMax = "23"))
    int32 StartHour = 8;

    // End hour
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule", meta = (ClampMin = "0", ClampMax = "23"))
    int32 EndHour = 17;

    // Location for activity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector ActivityLocation;

    // Location actor (alternative)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TSoftObjectPtr<AActor> LocationActor;

    // Priority (higher = more important)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    int32 Priority = 0;

    // Required days (bitmask: 1=Mon, 2=Tue, 4=Wed, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    int32 RequiredDays = 127; // All days

    // Can be interrupted
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bCanBeInterrupted = true;

    // Animation to play
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TSoftObjectPtr<UAnimMontage> ActivityAnimation;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NPCSchedule
{
    GENERATED_BODY()

    // Schedule entries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FForge2NPCScheduleEntry> Entries;

    // Home location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector HomeLocation;

    // Work location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector WorkLocation;

    // Sleep start hour
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule", meta = (ClampMin = "0", ClampMax = "23"))
    int32 SleepStartHour = 22;

    // Wake hour
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule", meta = (ClampMin = "0", ClampMax = "23"))
    int32 WakeHour = 6;

    // Randomize schedule
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bRandomizeSchedule = true;

    // Randomization variance (hours)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float RandomizationVariance = 0.5f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NPCRelationship
{
    GENERATED_BODY()

    // Target NPC ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FGuid TargetNPCId;

    // Relationship value (-100 to 100)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "-100", ClampMax = "100"))
    float RelationshipValue = 0.0f;

    // Is family member
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    bool bIsFamily = false;

    // Is friend
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    bool bIsFriend = false;

    // Is rival
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    bool bIsRival = false;

    // Last interaction time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteractionTime = 0.0f;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NPCMemory
{
    GENERATED_BODY()

    // Memory key
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FName MemoryKey;

    // Memory value
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString MemoryValue;

    // Associated actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TWeakObjectPtr<AActor> AssociatedActor;

    // Associated location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector AssociatedLocation;

    // Creation game time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float CreationTime = 0.0f;

    // Memory strength (0-1, decays over time)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (ClampMin = "0", ClampMax = "1"))
    float Strength = 1.0f;

    // Is permanent
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsPermanent = false;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2NPCInfo
{
    GENERATED_BODY()

    // NPC ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FGuid NPCId;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FText DisplayName;

    // Current mood
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    EForge2NPCMood Mood = EForge2NPCMood::Neutral;

    // Current activity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    EForge2NPCActivity CurrentActivity = EForge2NPCActivity::Working;

    // Schedule
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FForge2NPCSchedule Schedule;

    // Relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FForge2NPCRelationship> Relationships;

    // Memories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FForge2NPCMemory> Memories;

    // Relationship with player (-100 to 100)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC", meta = (ClampMin = "-100", ClampMax = "100"))
    float PlayerRelationship = 0.0f;

    // Home biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    EForge2BiomeType HomeBiome = EForge2BiomeType::Urban;

    // Actor reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TWeakObjectPtr<AActor> ActorRef;
};

USTRUCT(BlueprintType)
struct MCPAUTOMATIONBRIDGE_API FForge2SpawningSettings
{
    GENERATED_BODY()

    // Maximum animals in world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "0"))
    int32 MaxAnimals = 100;

    // Spawn radius around player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius = 10000.0f;

    // Despawn radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float DespawnRadius = 15000.0f;

    // Minimum spawn distance from player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MinSpawnDistance = 2000.0f;

    // Spawn check interval
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnCheckInterval = 5.0f;

    // Species spawn weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TMap<FName, float> SpeciesWeights;

    // Biome spawn modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TMap<EForge2BiomeType, float> BiomeModifiers;
};

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, EForge2TimeOfDay, OldTimeOfDay, EForge2TimeOfDay, NewTimeOfDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHourChanged, int32, NewHour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32, NewDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSeasonChanged, EForge2Season, OldSeason, EForge2Season, NewSeason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, FForge2WeatherState, OldWeather, FForge2WeatherState, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLightningStrike, FVector, StrikeLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimalSpawned, FGuid, InstanceId, FName, SpeciesId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimalDespawned, FGuid, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimalBehaviorChanged, FGuid, InstanceId, EForge2AnimalBehavior, NewBehavior);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCActivityChanged, FGuid, NPCId, EForge2NPCActivity, NewActivity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCMoodChanged, FGuid, NPCId, EForge2NPCMood, NewMood);

// ============================================================================
// TIME MANAGER
// ============================================================================

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2TimeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2TimeManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== Time Control ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void InitializeTime(const FForge2TimeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void SetTimeSpeed(float SpeedMultiplier);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void PauseTime(bool bPause);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void SetTime(int32 Hour, int32 Minute);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void SetDate(int32 Day, int32 Month, int32 Year);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void AdvanceTime(float Hours);

    // ========== Time Queries ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    int32 GetCurrentHour() const { return CurrentHour; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    int32 GetCurrentMinute() const { return CurrentMinute; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    int32 GetCurrentDay() const { return CurrentDay; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    int32 GetCurrentMonth() const { return CurrentMonth; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    int32 GetCurrentYear() const { return CurrentYear; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    float GetTimeNormalized() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    EForge2TimeOfDay GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    bool IsNightTime() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    float GetTotalGameHours() const { return TotalGameHours; }

    // ========== Sun/Sky ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void SetupSun(const FForge2SunSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void SetupSky(const FForge2SkySettings& Settings);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    FRotator GetSunRotation() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    FLinearColor GetSunColor() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    float GetSunIntensity() const;

    // ========== Seasons ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Time")
    void SetupSeasons(const TArray<FForge2SeasonSettings>& SeasonSettings);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    EForge2Season GetCurrentSeason() const { return CurrentSeason; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Time")
    int32 GetDayOfSeason() const;

    // ========== Events ==========

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Time|Events")
    FOnTimeOfDayChanged OnTimeOfDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Time|Events")
    FOnHourChanged OnHourChanged;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Time|Events")
    FOnDayChanged OnDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Time|Events")
    FOnSeasonChanged OnSeasonChanged;

protected:
    void UpdateTimeOfDay();
    void UpdateSunPosition();
    void UpdateSeason();
    void ApplySeasonEffects();

private:
    FForge2TimeSettings TimeSettings;
    FForge2SunSettings SunSettings;
    FForge2SkySettings SkySettings;
    TArray<FForge2SeasonSettings> SeasonConfigs;

    int32 CurrentHour = 8;
    int32 CurrentMinute = 0;
    int32 CurrentDay = 1;
    int32 CurrentMonth = 6;
    int32 CurrentYear = 1;
    float TimeAccumulator = 0.0f;
    float TotalGameHours = 0.0f;
    float TimeSpeedMultiplier = 1.0f;
    bool bTimePaused = false;

    EForge2TimeOfDay CurrentTimeOfDay = EForge2TimeOfDay::Morning;
    EForge2Season CurrentSeason = EForge2Season::Summer;
};

// ============================================================================
// WEATHER MANAGER
// ============================================================================

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2WeatherManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2WeatherManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== Weather Control ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void SetWeather(const FForge2WeatherState& NewWeather, float TransitionDuration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void SetWeatherType(EForge2WeatherType Type, EForge2WeatherIntensity Intensity, float TransitionDuration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void ClearWeather(float TransitionDuration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void SetupWeatherEffects(const FForge2WeatherEffects& Effects);

    // ========== Weather Queries ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    FForge2WeatherState GetCurrentWeather() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    EForge2WeatherType GetWeatherType() const { return CurrentWeatherState.WeatherType; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    EForge2WeatherIntensity GetWeatherIntensity() const { return CurrentWeatherState.Intensity; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    float GetTemperature() const { return CurrentWeatherState.Temperature; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    float GetWindSpeed() const { return CurrentWeatherState.WindSpeed; }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    FVector GetWindDirection() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    bool IsRaining() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    bool IsSnowing() const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    bool IsStormy() const;

    // ========== Dynamic Weather ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void EnableDynamicWeather(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void SetWeatherProbabilities(const TMap<EForge2WeatherType, float>& Probabilities);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void SetWeatherChangeInterval(float MinInterval, float MaxInterval);

    // ========== Lightning ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void TriggerLightning(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void SetLightningFrequency(float StrikesPerMinute);

    // ========== Indoor/Outdoor ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Weather")
    void SetPlayerIndoors(bool bIndoors);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Weather")
    bool IsPlayerIndoors() const { return bPlayerIndoors; }

    // ========== Events ==========

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Weather|Events")
    FOnWeatherChanged OnWeatherChanged;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Weather|Events")
    FOnLightningStrike OnLightningStrike;

protected:
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateWeatherEffects();
    void UpdateLightning(float DeltaTime);
    void SelectNextWeather();
    FForge2WeatherState InterpolateWeather(const FForge2WeatherState& A, const FForge2WeatherState& B, float Alpha);

private:
    FForge2WeatherState CurrentWeatherState;
    FForge2WeatherState TargetWeatherState;
    FForge2WeatherState StartWeatherState;
    FForge2WeatherEffects WeatherEffects;

    bool bTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 0.0f;

    bool bDynamicWeatherEnabled = false;
    TMap<EForge2WeatherType, float> WeatherProbabilities;
    float MinWeatherInterval = 60.0f;
    float MaxWeatherInterval = 300.0f;
    float NextWeatherChangeTime = 0.0f;

    float LightningTimer = 0.0f;
    bool bPlayerIndoors = false;
};

// ============================================================================
// ECOSYSTEM MANAGER
// ============================================================================

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2EcosystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2EcosystemManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== Species Management ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void RegisterSpecies(const FForge2AnimalSpecies& Species);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    bool GetSpecies(FName SpeciesId, FForge2AnimalSpecies& OutSpecies) const;

    UFUNCTION(BlueprintPure, Category = "FORGE2|Ecosystem")
    TArray<FName> GetAllSpeciesIds() const;

    // ========== Animal Spawning ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void SetSpawningSettings(const FForge2SpawningSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    FGuid SpawnAnimal(FName SpeciesId, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    FGuid SpawnHerd(FName SpeciesId, const FVector& CenterLocation, int32 Count = -1);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void DespawnAnimal(FGuid InstanceId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void DespawnHerd(FGuid HerdId);

    // ========== Animal Queries ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Ecosystem")
    int32 GetAnimalCount() const { return Animals.Num(); }

    UFUNCTION(BlueprintPure, Category = "FORGE2|Ecosystem")
    int32 GetAnimalCountBySpecies(FName SpeciesId) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    bool GetAnimalInfo(FGuid InstanceId, FForge2AnimalInstance& OutInfo) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    TArray<FForge2AnimalInstance> GetAnimalsInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    TArray<FForge2AnimalInstance> GetAnimalsBySpecies(FName SpeciesId) const;

    // ========== Herd Management ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    bool GetHerdInfo(FGuid HerdId, FForge2HerdInfo& OutInfo) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void SetHerdMigrationTarget(FGuid HerdId, const FVector& Target);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void StartHerdMigration(FGuid HerdId, const FVector& Target);

    // ========== Behavior Control ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void SetAnimalBehavior(FGuid InstanceId, EForge2AnimalBehavior Behavior);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void FleeFrom(FGuid InstanceId, const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void AttackTarget(FGuid PredatorId, FGuid PreyId);

    // ========== Taming ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    bool TryTameAnimal(FGuid InstanceId, FGuid PlayerId, float SuccessChance = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|Ecosystem")
    void ReleaseAnimal(FGuid InstanceId);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Ecosystem")
    TArray<FForge2AnimalInstance> GetPlayerPets(FGuid PlayerId) const;

    // ========== Events ==========

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Ecosystem|Events")
    FOnAnimalSpawned OnAnimalSpawned;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Ecosystem|Events")
    FOnAnimalDespawned OnAnimalDespawned;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|Ecosystem|Events")
    FOnAnimalBehaviorChanged OnAnimalBehaviorChanged;

protected:
    void UpdateSpawning();
    void UpdateAnimalBehaviors(float DeltaTime);
    void UpdateHerds(float DeltaTime);
    void ProcessPredatorPrey(float DeltaTime);
    FVector FindSpawnLocation(FName SpeciesId);
    bool IsValidSpawnLocation(const FVector& Location, FName SpeciesId);
    void CleanupDistantAnimals();

private:
    TMap<FName, FForge2AnimalSpecies> RegisteredSpecies;
    TMap<FGuid, FForge2AnimalInstance> Animals;
    TMap<FGuid, FForge2HerdInfo> Herds;
    FForge2SpawningSettings SpawningSettings;

    float SpawnCheckTimer = 0.0f;
    float BehaviorUpdateTimer = 0.0f;
    float CleanupTimer = 0.0f;
};

// ============================================================================
// NPC SCHEDULE MANAGER
// ============================================================================

UCLASS(ClassGroup = (FORGE2), meta = (BlueprintSpawnableComponent))
class MCPAUTOMATIONBRIDGE_API UForge2NPCScheduleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UForge2NPCScheduleManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== NPC Registration ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    FGuid RegisterNPC(const FForge2NPCInfo& NPCInfo);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void UnregisterNPC(FGuid NPCId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    bool GetNPCInfo(FGuid NPCId, FForge2NPCInfo& OutInfo) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void UpdateNPCInfo(FGuid NPCId, const FForge2NPCInfo& NewInfo);

    // ========== Schedule Management ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void SetNPCSchedule(FGuid NPCId, const FForge2NPCSchedule& Schedule);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void AddScheduleEntry(FGuid NPCId, const FForge2NPCScheduleEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void ClearSchedule(FGuid NPCId);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    EForge2NPCActivity GetCurrentActivity(FGuid NPCId) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    FVector GetCurrentActivityLocation(FGuid NPCId) const;

    // ========== Mood & Relationships ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void SetNPCMood(FGuid NPCId, EForge2NPCMood Mood);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void ModifyPlayerRelationship(FGuid NPCId, float Delta);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void SetNPCRelationship(FGuid NPCId, FGuid TargetNPCId, float Value);

    UFUNCTION(BlueprintPure, Category = "FORGE2|NPC")
    float GetPlayerRelationship(FGuid NPCId) const;

    // ========== Memory System ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void AddNPCMemory(FGuid NPCId, const FForge2NPCMemory& Memory);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    bool GetNPCMemory(FGuid NPCId, FName MemoryKey, FForge2NPCMemory& OutMemory) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void ClearNPCMemory(FGuid NPCId, FName MemoryKey);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    TArray<FForge2NPCMemory> GetNPCMemories(FGuid NPCId) const;

    // ========== Behavior Override ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void InterruptNPC(FGuid NPCId, EForge2NPCActivity OverrideActivity, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void ResumeNormalSchedule(FGuid NPCId);

    UFUNCTION(BlueprintPure, Category = "FORGE2|NPC")
    bool IsNPCInterrupted(FGuid NPCId) const;

    // ========== Crowd Simulation ==========

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void SpawnCrowd(const FVector& CenterLocation, int32 Count, float Radius);

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    void DespawnCrowd(const FVector& CenterLocation, float Radius);

    // ========== Queries ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|NPC")
    TArray<FGuid> GetAllNPCIds() const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    TArray<FForge2NPCInfo> GetNPCsInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    TArray<FForge2NPCInfo> GetNPCsByActivity(EForge2NPCActivity Activity) const;

    UFUNCTION(BlueprintCallable, Category = "FORGE2|NPC")
    TArray<FForge2NPCInfo> GetNPCsByMood(EForge2NPCMood Mood) const;

    // ========== Events ==========

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|NPC|Events")
    FOnNPCActivityChanged OnNPCActivityChanged;

    UPROPERTY(BlueprintAssignable, Category = "FORGE2|NPC|Events")
    FOnNPCMoodChanged OnNPCMoodChanged;

protected:
    void UpdateNPCSchedules(int32 CurrentHour, int32 CurrentMinute, int32 CurrentDayOfWeek);
    void UpdateNPCMemoryDecay(float DeltaTime);
    FForge2NPCScheduleEntry* FindCurrentScheduleEntry(const FForge2NPCSchedule& Schedule, int32 Hour, int32 DayOfWeek);
    void MoveNPCToActivity(FGuid NPCId, const FForge2NPCScheduleEntry& Entry);

private:
    TMap<FGuid, FForge2NPCInfo> RegisteredNPCs;
    TMap<FGuid, float> InterruptedNPCs; // NPC ID -> End time
    TMap<FGuid, EForge2NPCActivity> InterruptedActivities;

    int32 LastProcessedHour = -1;
    float MemoryDecayTimer = 0.0f;

    // Reference to time manager
    TWeakObjectPtr<UForge2TimeManager> TimeManager;
};

// ============================================================================
// ENVIRONMENT UTILITIES
// ============================================================================

UCLASS()
class MCPAUTOMATIONBRIDGE_API UForge2EnvironmentUtilities : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ========== Time Utilities ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FString FormatGameTime(int32 Hour, int32 Minute, bool bUse24Hour = true);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FString FormatGameDate(int32 Day, int32 Month, int32 Year);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static int32 GetDayOfWeek(int32 Day, int32 Month, int32 Year, int32 DaysPerMonth, int32 MonthsPerYear);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static EForge2TimeOfDay GetTimeOfDayFromHour(int32 Hour);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static EForge2Season GetSeasonFromMonth(int32 Month, int32 MonthsPerYear);

    // ========== Weather Utilities ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static float GetWindChill(float Temperature, float WindSpeed);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static float GetHeatIndex(float Temperature, float Humidity);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static bool CanSnow(float Temperature);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FLinearColor GetWeatherLightingTint(EForge2WeatherType WeatherType, EForge2WeatherIntensity Intensity);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static float GetWeatherVisibilityMultiplier(EForge2WeatherType WeatherType, EForge2WeatherIntensity Intensity);

    // ========== Ecosystem Utilities ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static bool IsPreyFor(const FForge2AnimalSpecies& Prey, const FForge2AnimalSpecies& Predator);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static bool CanSpawnInBiome(const FForge2AnimalSpecies& Species, EForge2BiomeType Biome);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static bool IsActiveAtTime(const FForge2AnimalSpecies& Species, bool bIsNight);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static float CalculateTamingChance(const FForge2AnimalSpecies& Species, float PlayerSkill);

    // ========== Preset Generators ==========

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2TimeSettings GetDefaultTimeSettings();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2TimeSettings GetRealtimeSettings();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2SunSettings GetEarthlikeSunSettings();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static TArray<FForge2SeasonSettings> GetTemperateSeasons();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static TArray<FForge2SeasonSettings> GetTropicalSeasons();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static TArray<FForge2SeasonSettings> GetArcticSeasons();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2WeatherState GetClearWeather();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2WeatherState GetRainyWeather(EForge2WeatherIntensity Intensity);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2WeatherState GetStormyWeather();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2WeatherState GetSnowyWeather(EForge2WeatherIntensity Intensity);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2WeatherState GetFoggyWeather(EForge2WeatherIntensity Intensity);

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2AnimalSpecies GetDeerSpecies();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2AnimalSpecies GetWolfSpecies();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2AnimalSpecies GetRabbitSpecies();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2AnimalSpecies GetBearSpecies();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2NPCSchedule GetShopkeeperSchedule();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2NPCSchedule GetGuardSchedule();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2NPCSchedule GetFarmerSchedule();

    UFUNCTION(BlueprintPure, Category = "FORGE2|Environment|Utilities")
    static FForge2NPCSchedule GetTavernSchedule();
};
