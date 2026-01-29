// Forge2EnvironmentSystem.cpp
// Phase 13: Environment Systems Implementation
// FORGE2 Ultimate AAA Game Development MCP Extension

#include "Forge2EnvironmentSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

// ============================================================================
// TIME MANAGER IMPLEMENTATION
// ============================================================================

UForge2TimeManager::UForge2TimeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UForge2TimeManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize with defaults if not set
    if (TimeSettings.SecondsPerHour <= 0.0f)
    {
        TimeSettings = UForge2EnvironmentUtilities::GetDefaultTimeSettings();
    }

    CurrentHour = TimeSettings.StartingHour;
    CurrentMinute = TimeSettings.StartingMinute;
    CurrentDay = TimeSettings.StartingDay;
    CurrentMonth = TimeSettings.StartingMonth;
    CurrentYear = TimeSettings.StartingYear;

    UpdateTimeOfDay();
    UpdateSeason();
}

void UForge2TimeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bTimePaused)
    {
        return;
    }

    // Accumulate time
    float EffectiveDelta = DeltaTime * TimeSpeedMultiplier;
    TimeAccumulator += EffectiveDelta;

    // Convert to game time
    float SecondsPerMinute = TimeSettings.SecondsPerHour / 60.0f;

    while (TimeAccumulator >= SecondsPerMinute)
    {
        TimeAccumulator -= SecondsPerMinute;
        CurrentMinute++;
        TotalGameHours += 1.0f / 60.0f;

        if (CurrentMinute >= 60)
        {
            CurrentMinute = 0;
            int32 OldHour = CurrentHour;
            CurrentHour++;

            if (CurrentHour >= 24)
            {
                CurrentHour = 0;
                CurrentDay++;
                OnDayChanged.Broadcast(CurrentDay);

                if (CurrentDay > TimeSettings.DaysPerMonth)
                {
                    CurrentDay = 1;
                    CurrentMonth++;

                    if (CurrentMonth > TimeSettings.MonthsPerYear)
                    {
                        CurrentMonth = 1;
                        CurrentYear++;
                    }

                    UpdateSeason();
                }
            }

            OnHourChanged.Broadcast(CurrentHour);
            UpdateTimeOfDay();
        }
    }

    UpdateSunPosition();
}

void UForge2TimeManager::InitializeTime(const FForge2TimeSettings& Settings)
{
    TimeSettings = Settings;
    CurrentHour = Settings.StartingHour;
    CurrentMinute = Settings.StartingMinute;
    CurrentDay = Settings.StartingDay;
    CurrentMonth = Settings.StartingMonth;
    CurrentYear = Settings.StartingYear;
    TimeAccumulator = 0.0f;

    UpdateTimeOfDay();
    UpdateSeason();
}

void UForge2TimeManager::SetTimeSpeed(float SpeedMultiplier)
{
    TimeSpeedMultiplier = FMath::Max(0.0f, SpeedMultiplier);
}

void UForge2TimeManager::PauseTime(bool bPause)
{
    bTimePaused = bPause;
}

void UForge2TimeManager::SetTime(int32 Hour, int32 Minute)
{
    CurrentHour = FMath::Clamp(Hour, 0, 23);
    CurrentMinute = FMath::Clamp(Minute, 0, 59);
    TimeAccumulator = 0.0f;

    UpdateTimeOfDay();
    UpdateSunPosition();
    OnHourChanged.Broadcast(CurrentHour);
}

void UForge2TimeManager::SetDate(int32 Day, int32 Month, int32 Year)
{
    CurrentDay = FMath::Max(1, Day);
    CurrentMonth = FMath::Clamp(Month, 1, TimeSettings.MonthsPerYear);
    CurrentYear = Year;

    if (CurrentDay > TimeSettings.DaysPerMonth)
    {
        CurrentDay = TimeSettings.DaysPerMonth;
    }

    UpdateSeason();
    OnDayChanged.Broadcast(CurrentDay);
}

void UForge2TimeManager::AdvanceTime(float Hours)
{
    float TotalMinutes = Hours * 60.0f;
    int32 MinutesToAdd = FMath::FloorToInt(TotalMinutes);

    for (int32 i = 0; i < MinutesToAdd; i++)
    {
        CurrentMinute++;
        TotalGameHours += 1.0f / 60.0f;

        if (CurrentMinute >= 60)
        {
            CurrentMinute = 0;
            CurrentHour++;

            if (CurrentHour >= 24)
            {
                CurrentHour = 0;
                CurrentDay++;

                if (CurrentDay > TimeSettings.DaysPerMonth)
                {
                    CurrentDay = 1;
                    CurrentMonth++;

                    if (CurrentMonth > TimeSettings.MonthsPerYear)
                    {
                        CurrentMonth = 1;
                        CurrentYear++;
                    }
                }
            }
        }
    }

    UpdateTimeOfDay();
    UpdateSeason();
    UpdateSunPosition();
}

float UForge2TimeManager::GetTimeNormalized() const
{
    return (CurrentHour * 60.0f + CurrentMinute) / (24.0f * 60.0f);
}

bool UForge2TimeManager::IsNightTime() const
{
    float CurrentTime = CurrentHour + CurrentMinute / 60.0f;
    return CurrentTime < SunSettings.SunriseHour || CurrentTime > SunSettings.SunsetHour;
}

void UForge2TimeManager::SetupSun(const FForge2SunSettings& Settings)
{
    SunSettings = Settings;
    UpdateSunPosition();
}

void UForge2TimeManager::SetupSky(const FForge2SkySettings& Settings)
{
    SkySettings = Settings;
}

FRotator UForge2TimeManager::GetSunRotation() const
{
    float TimeNormalized = GetTimeNormalized();

    // Calculate sun angle based on time
    // Sun rises in east, sets in west
    float SunAngle = (TimeNormalized - 0.25f) * 360.0f; // 0.25 = 6:00 AM

    // Sun altitude (pitch)
    float AltitudeAngle;
    float CurrentTime = CurrentHour + CurrentMinute / 60.0f;

    if (CurrentTime < SunSettings.SunriseHour || CurrentTime > SunSettings.SunsetHour)
    {
        // Night time - sun below horizon
        AltitudeAngle = -15.0f;
    }
    else
    {
        // Day time - sun above horizon
        float DayProgress = (CurrentTime - SunSettings.SunriseHour) / (SunSettings.SunsetHour - SunSettings.SunriseHour);
        AltitudeAngle = FMath::Sin(DayProgress * PI) * 80.0f; // Max 80 degrees at noon
    }

    return FRotator(AltitudeAngle, SunAngle + SunSettings.NorthOffset, 0.0f);
}

FLinearColor UForge2TimeManager::GetSunColor() const
{
    float CurrentTime = CurrentHour + CurrentMinute / 60.0f;

    if (CurrentTime < SunSettings.SunriseHour || CurrentTime > SunSettings.SunsetHour)
    {
        // Night - very dim blue
        return FLinearColor(0.1f, 0.1f, 0.2f);
    }

    // Calculate color based on time of day
    float DayProgress = (CurrentTime - SunSettings.SunriseHour) / (SunSettings.SunsetHour - SunSettings.SunriseHour);

    if (DayProgress < 0.15f)
    {
        // Dawn
        float Alpha = DayProgress / 0.15f;
        return FMath::Lerp(SunSettings.DawnColor, SunSettings.NoonColor, Alpha);
    }
    else if (DayProgress > 0.85f)
    {
        // Dusk
        float Alpha = (DayProgress - 0.85f) / 0.15f;
        return FMath::Lerp(SunSettings.NoonColor, SunSettings.DuskColor, Alpha);
    }
    else
    {
        // Mid-day
        return SunSettings.NoonColor;
    }
}

float UForge2TimeManager::GetSunIntensity() const
{
    float CurrentTime = CurrentHour + CurrentMinute / 60.0f;

    if (CurrentTime < SunSettings.SunriseHour || CurrentTime > SunSettings.SunsetHour)
    {
        return SunSettings.NightIntensity;
    }

    float DayProgress = (CurrentTime - SunSettings.SunriseHour) / (SunSettings.SunsetHour - SunSettings.SunriseHour);
    float IntensityCurve = FMath::Sin(DayProgress * PI);

    return FMath::Lerp(SunSettings.NightIntensity, SunSettings.DayIntensity, IntensityCurve);
}

void UForge2TimeManager::SetupSeasons(const TArray<FForge2SeasonSettings>& Settings)
{
    SeasonConfigs = Settings;
    UpdateSeason();
}

int32 UForge2TimeManager::GetDayOfSeason() const
{
    if (SeasonConfigs.Num() == 0)
    {
        return CurrentDay;
    }

    int32 DaysPerSeason = TimeSettings.DaysPerMonth * (TimeSettings.MonthsPerYear / 4);
    int32 TotalDayOfYear = ((CurrentMonth - 1) * TimeSettings.DaysPerMonth) + CurrentDay;

    return ((TotalDayOfYear - 1) % DaysPerSeason) + 1;
}

void UForge2TimeManager::UpdateTimeOfDay()
{
    EForge2TimeOfDay OldTimeOfDay = CurrentTimeOfDay;
    CurrentTimeOfDay = UForge2EnvironmentUtilities::GetTimeOfDayFromHour(CurrentHour);

    if (OldTimeOfDay != CurrentTimeOfDay)
    {
        OnTimeOfDayChanged.Broadcast(OldTimeOfDay, CurrentTimeOfDay);
    }
}

void UForge2TimeManager::UpdateSunPosition()
{
    if (SunSettings.SunLightActor.IsValid())
    {
        AActor* SunActor = SunSettings.SunLightActor.Get();
        if (SunActor)
        {
            SunActor->SetActorRotation(GetSunRotation());

            // Try to set light intensity and color
            TArray<ULightComponent*> LightComponents;
            SunActor->GetComponents<ULightComponent>(LightComponents);

            for (ULightComponent* LightComp : LightComponents)
            {
                LightComp->SetIntensity(GetSunIntensity());
                LightComp->SetLightColor(GetSunColor());
            }
        }
    }
}

void UForge2TimeManager::UpdateSeason()
{
    if (TimeSettings.MonthsPerYear < 4)
    {
        return;
    }

    EForge2Season OldSeason = CurrentSeason;
    CurrentSeason = UForge2EnvironmentUtilities::GetSeasonFromMonth(CurrentMonth, TimeSettings.MonthsPerYear);

    if (OldSeason != CurrentSeason)
    {
        OnSeasonChanged.Broadcast(OldSeason, CurrentSeason);
        ApplySeasonEffects();
    }
}

void UForge2TimeManager::ApplySeasonEffects()
{
    // Find matching season settings
    for (const FForge2SeasonSettings& SeasonConfig : SeasonConfigs)
    {
        if (SeasonConfig.Season == CurrentSeason)
        {
            // Apply season-specific effects
            // This would typically involve updating materials, foliage colors, etc.
            break;
        }
    }
}

// ============================================================================
// WEATHER MANAGER IMPLEMENTATION
// ============================================================================

UForge2WeatherManager::UForge2WeatherManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UForge2WeatherManager::BeginPlay()
{
    Super::BeginPlay();

    // Start with clear weather
    CurrentWeatherState = UForge2EnvironmentUtilities::GetClearWeather();
    TargetWeatherState = CurrentWeatherState;
}

void UForge2WeatherManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }

    UpdateLightning(DeltaTime);
    UpdateWeatherEffects();

    if (bDynamicWeatherEnabled)
    {
        float CurrentGameTime = GetWorld()->GetTimeSeconds();
        if (CurrentGameTime >= NextWeatherChangeTime)
        {
            SelectNextWeather();
        }
    }
}

void UForge2WeatherManager::SetWeather(const FForge2WeatherState& NewWeather, float Duration)
{
    FForge2WeatherState OldWeather = CurrentWeatherState;

    if (Duration <= 0.0f)
    {
        CurrentWeatherState = NewWeather;
        bTransitioning = false;
    }
    else
    {
        StartWeatherState = CurrentWeatherState;
        TargetWeatherState = NewWeather;
        TransitionDuration = Duration;
        TransitionProgress = 0.0f;
        bTransitioning = true;
    }

    OnWeatherChanged.Broadcast(OldWeather, NewWeather);
}

void UForge2WeatherManager::SetWeatherType(EForge2WeatherType Type, EForge2WeatherIntensity Intensity, float Duration)
{
    FForge2WeatherState NewState = CurrentWeatherState;
    NewState.WeatherType = Type;
    NewState.Intensity = Intensity;

    // Adjust related parameters based on weather type
    switch (Type)
    {
    case EForge2WeatherType::Clear:
        NewState.CloudCoverage = 0.1f;
        NewState.Precipitation = 0.0f;
        NewState.FogDensity = 0.0f;
        NewState.VisibilityRange = 10000.0f;
        break;

    case EForge2WeatherType::Cloudy:
        NewState.CloudCoverage = 0.5f;
        NewState.Precipitation = 0.0f;
        break;

    case EForge2WeatherType::Overcast:
        NewState.CloudCoverage = 0.9f;
        NewState.Precipitation = 0.0f;
        break;

    case EForge2WeatherType::Foggy:
        NewState.FogDensity = static_cast<float>(Intensity) * 0.25f;
        NewState.VisibilityRange = 1000.0f / (static_cast<float>(Intensity) + 1.0f);
        break;

    case EForge2WeatherType::Rainy:
        NewState.CloudCoverage = 0.8f;
        NewState.Precipitation = static_cast<float>(Intensity) * 0.25f;
        NewState.Humidity = 0.8f;
        break;

    case EForge2WeatherType::Stormy:
        NewState.CloudCoverage = 1.0f;
        NewState.Precipitation = 0.8f;
        NewState.WindSpeed = 30.0f;
        NewState.LightningFrequency = 2.0f;
        break;

    case EForge2WeatherType::Snowy:
        NewState.CloudCoverage = 0.7f;
        NewState.Precipitation = static_cast<float>(Intensity) * 0.25f;
        NewState.Temperature = -5.0f;
        break;

    case EForge2WeatherType::Blizzard:
        NewState.CloudCoverage = 1.0f;
        NewState.Precipitation = 1.0f;
        NewState.Temperature = -15.0f;
        NewState.WindSpeed = 40.0f;
        NewState.VisibilityRange = 100.0f;
        break;

    case EForge2WeatherType::Sandstorm:
        NewState.FogDensity = 0.8f;
        NewState.WindSpeed = 35.0f;
        NewState.VisibilityRange = 200.0f;
        break;

    case EForge2WeatherType::Heatwave:
        NewState.CloudCoverage = 0.0f;
        NewState.Temperature = 40.0f;
        break;

    default:
        break;
    }

    SetWeather(NewState, Duration);
}

void UForge2WeatherManager::ClearWeather(float Duration)
{
    SetWeather(UForge2EnvironmentUtilities::GetClearWeather(), Duration);
}

void UForge2WeatherManager::SetupWeatherEffects(const FForge2WeatherEffects& Effects)
{
    WeatherEffects = Effects;
}

FVector UForge2WeatherManager::GetWindDirection() const
{
    float Radians = FMath::DegreesToRadians(CurrentWeatherState.WindDirection);
    return FVector(FMath::Cos(Radians), FMath::Sin(Radians), 0.0f);
}

bool UForge2WeatherManager::IsRaining() const
{
    return CurrentWeatherState.WeatherType == EForge2WeatherType::Rainy ||
           CurrentWeatherState.WeatherType == EForge2WeatherType::Stormy;
}

bool UForge2WeatherManager::IsSnowing() const
{
    return CurrentWeatherState.WeatherType == EForge2WeatherType::Snowy ||
           CurrentWeatherState.WeatherType == EForge2WeatherType::Blizzard;
}

bool UForge2WeatherManager::IsStormy() const
{
    return CurrentWeatherState.WeatherType == EForge2WeatherType::Stormy ||
           CurrentWeatherState.WeatherType == EForge2WeatherType::Blizzard;
}

void UForge2WeatherManager::EnableDynamicWeather(bool bEnable)
{
    bDynamicWeatherEnabled = bEnable;
    if (bEnable)
    {
        NextWeatherChangeTime = GetWorld()->GetTimeSeconds() + FMath::RandRange(MinWeatherInterval, MaxWeatherInterval);
    }
}

void UForge2WeatherManager::SetWeatherProbabilities(const TMap<EForge2WeatherType, float>& Probabilities)
{
    WeatherProbabilities = Probabilities;
}

void UForge2WeatherManager::SetWeatherChangeInterval(float Min, float Max)
{
    MinWeatherInterval = FMath::Max(1.0f, Min);
    MaxWeatherInterval = FMath::Max(MinWeatherInterval, Max);
}

void UForge2WeatherManager::TriggerLightning(const FVector& Location)
{
    OnLightningStrike.Broadcast(Location);

    // Spawn lightning effect
    if (WeatherEffects.LightningEffect.IsValid())
    {
        UNiagaraSystem* LightningSystem = WeatherEffects.LightningEffect.Get();
        if (LightningSystem)
        {
            // Spawn at location
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                LightningSystem,
                Location + FVector(0, 0, 5000), // Start from sky
                FRotator::ZeroRotator
            );
        }
    }

    // Play thunder sound with delay based on distance
    if (WeatherEffects.ThunderSound.IsValid())
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
            float Delay = Distance / 34300.0f; // Speed of sound in cm/s

            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                [this, Location]()
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        GetWorld(),
                        WeatherEffects.ThunderSound.Get(),
                        Location
                    );
                },
                Delay,
                false
            );
        }
    }
}

void UForge2WeatherManager::SetLightningFrequency(float StrikesPerMinute)
{
    CurrentWeatherState.LightningFrequency = FMath::Max(0.0f, StrikesPerMinute);
}

void UForge2WeatherManager::SetPlayerIndoors(bool bIndoors)
{
    bPlayerIndoors = bIndoors;
}

void UForge2WeatherManager::UpdateWeatherTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;

    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bTransitioning = false;
        CurrentWeatherState = TargetWeatherState;
    }
    else
    {
        CurrentWeatherState = InterpolateWeather(StartWeatherState, TargetWeatherState, TransitionProgress);
    }
}

void UForge2WeatherManager::UpdateWeatherEffects()
{
    // Update weather particle systems based on current state
    // This would activate/deactivate rain, snow, etc. based on weather type and intensity
}

void UForge2WeatherManager::UpdateLightning(float DeltaTime)
{
    if (CurrentWeatherState.LightningFrequency <= 0.0f || bPlayerIndoors)
    {
        return;
    }

    LightningTimer += DeltaTime;
    float StrikeInterval = 60.0f / CurrentWeatherState.LightningFrequency;

    if (LightningTimer >= StrikeInterval)
    {
        LightningTimer = 0.0f;

        // Random location around player
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            float RandomAngle = FMath::RandRange(0.0f, 360.0f);
            float RandomDistance = FMath::RandRange(1000.0f, 10000.0f);

            FVector StrikeLocation = PlayerLocation + FVector(
                FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
                FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
                0.0f
            );

            TriggerLightning(StrikeLocation);
        }
    }
}

void UForge2WeatherManager::SelectNextWeather()
{
    if (WeatherProbabilities.Num() == 0)
    {
        return;
    }

    // Calculate total weight
    float TotalWeight = 0.0f;
    for (const auto& Pair : WeatherProbabilities)
    {
        TotalWeight += Pair.Value;
    }

    // Random selection
    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float AccumulatedWeight = 0.0f;

    EForge2WeatherType SelectedType = EForge2WeatherType::Clear;
    for (const auto& Pair : WeatherProbabilities)
    {
        AccumulatedWeight += Pair.Value;
        if (RandomValue <= AccumulatedWeight)
        {
            SelectedType = Pair.Key;
            break;
        }
    }

    // Random intensity
    EForge2WeatherIntensity RandomIntensity = static_cast<EForge2WeatherIntensity>(
        FMath::RandRange(1, static_cast<int32>(EForge2WeatherIntensity::Heavy))
    );

    SetWeatherType(SelectedType, RandomIntensity, 30.0f);

    // Schedule next change
    NextWeatherChangeTime = GetWorld()->GetTimeSeconds() + FMath::RandRange(MinWeatherInterval, MaxWeatherInterval);
}

FForge2WeatherState UForge2WeatherManager::InterpolateWeather(const FForge2WeatherState& A, const FForge2WeatherState& B, float Alpha)
{
    FForge2WeatherState Result;

    Result.WeatherType = Alpha < 0.5f ? A.WeatherType : B.WeatherType;
    Result.Intensity = Alpha < 0.5f ? A.Intensity : B.Intensity;
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.WindDirection = FMath::Lerp(A.WindDirection, B.WindDirection, Alpha);
    Result.WindSpeed = FMath::Lerp(A.WindSpeed, B.WindSpeed, Alpha);
    Result.Precipitation = FMath::Lerp(A.Precipitation, B.Precipitation, Alpha);
    Result.Temperature = FMath::Lerp(A.Temperature, B.Temperature, Alpha);
    Result.Humidity = FMath::Lerp(A.Humidity, B.Humidity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.LightningFrequency = FMath::Lerp(A.LightningFrequency, B.LightningFrequency, Alpha);
    Result.VisibilityRange = FMath::Lerp(A.VisibilityRange, B.VisibilityRange, Alpha);

    return Result;
}

// ============================================================================
// ECOSYSTEM MANAGER IMPLEMENTATION
// ============================================================================

UForge2EcosystemManager::UForge2EcosystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UForge2EcosystemManager::BeginPlay()
{
    Super::BeginPlay();
}

void UForge2EcosystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Spawn check
    SpawnCheckTimer += DeltaTime;
    if (SpawnCheckTimer >= SpawningSettings.SpawnCheckInterval)
    {
        SpawnCheckTimer = 0.0f;
        UpdateSpawning();
    }

    // Behavior update
    BehaviorUpdateTimer += DeltaTime;
    if (BehaviorUpdateTimer >= 0.5f)
    {
        BehaviorUpdateTimer = 0.0f;
        UpdateAnimalBehaviors(0.5f);
        ProcessPredatorPrey(0.5f);
    }

    // Herd update
    UpdateHerds(DeltaTime);

    // Cleanup distant animals
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 5.0f)
    {
        CleanupTimer = 0.0f;
        CleanupDistantAnimals();
    }
}

void UForge2EcosystemManager::RegisterSpecies(const FForge2AnimalSpecies& Species)
{
    RegisteredSpecies.Add(Species.SpeciesId, Species);
}

bool UForge2EcosystemManager::GetSpecies(FName SpeciesId, FForge2AnimalSpecies& OutSpecies) const
{
    if (const FForge2AnimalSpecies* Found = RegisteredSpecies.Find(SpeciesId))
    {
        OutSpecies = *Found;
        return true;
    }
    return false;
}

TArray<FName> UForge2EcosystemManager::GetAllSpeciesIds() const
{
    TArray<FName> Ids;
    RegisteredSpecies.GetKeys(Ids);
    return Ids;
}

void UForge2EcosystemManager::SetSpawningSettings(const FForge2SpawningSettings& Settings)
{
    SpawningSettings = Settings;
}

FGuid UForge2EcosystemManager::SpawnAnimal(FName SpeciesId, const FVector& Location, const FRotator& Rotation)
{
    const FForge2AnimalSpecies* Species = RegisteredSpecies.Find(SpeciesId);
    if (!Species)
    {
        return FGuid();
    }

    // Create instance
    FForge2AnimalInstance Instance;
    Instance.InstanceId = FGuid::NewGuid();
    Instance.SpeciesId = SpeciesId;
    Instance.CurrentBehavior = EForge2AnimalBehavior::Idle;

    // Spawn actor if class is valid
    if (Species->AnimalClass.IsValid())
    {
        UClass* AnimalClassPtr = Species->AnimalClass.LoadSynchronous();
        if (AnimalClassPtr)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(AnimalClassPtr, Location, Rotation, SpawnParams);
            if (SpawnedActor)
            {
                Instance.ActorRef = SpawnedActor;
            }
        }
    }

    Animals.Add(Instance.InstanceId, Instance);
    OnAnimalSpawned.Broadcast(Instance.InstanceId, SpeciesId);

    return Instance.InstanceId;
}

FGuid UForge2EcosystemManager::SpawnHerd(FName SpeciesId, const FVector& CenterLocation, int32 Count)
{
    const FForge2AnimalSpecies* Species = RegisteredSpecies.Find(SpeciesId);
    if (!Species)
    {
        return FGuid();
    }

    // Determine count
    int32 ActualCount = Count;
    if (ActualCount <= 0)
    {
        ActualCount = FMath::RandRange(Species->HerdSizeRange.X, Species->HerdSizeRange.Y);
    }

    // Create herd
    FForge2HerdInfo Herd;
    Herd.HerdId = FGuid::NewGuid();
    Herd.SpeciesId = SpeciesId;
    Herd.CenterLocation = CenterLocation;
    Herd.TerritoryCenter = CenterLocation;

    // Spawn members
    for (int32 i = 0; i < ActualCount; i++)
    {
        FVector Offset = FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );

        FVector SpawnLocation = CenterLocation + Offset;
        FGuid MemberId = SpawnAnimal(SpeciesId, SpawnLocation);

        if (MemberId.IsValid())
        {
            Herd.MemberIds.Add(MemberId);

            // Set herd ID on animal
            if (FForge2AnimalInstance* Animal = Animals.Find(MemberId))
            {
                Animal->HerdId = Herd.HerdId;
            }

            // First member is leader
            if (i == 0)
            {
                Herd.LeaderId = MemberId;
            }
        }
    }

    Herds.Add(Herd.HerdId, Herd);

    return Herd.HerdId;
}

void UForge2EcosystemManager::DespawnAnimal(FGuid InstanceId)
{
    if (FForge2AnimalInstance* Animal = Animals.Find(InstanceId))
    {
        // Remove from herd
        if (Animal->HerdId.IsValid())
        {
            if (FForge2HerdInfo* Herd = Herds.Find(Animal->HerdId))
            {
                Herd->MemberIds.Remove(InstanceId);
                if (Herd->LeaderId == InstanceId && Herd->MemberIds.Num() > 0)
                {
                    Herd->LeaderId = Herd->MemberIds[0];
                }
            }
        }

        // Destroy actor
        if (Animal->ActorRef.IsValid())
        {
            Animal->ActorRef->Destroy();
        }

        Animals.Remove(InstanceId);
        OnAnimalDespawned.Broadcast(InstanceId);
    }
}

void UForge2EcosystemManager::DespawnHerd(FGuid HerdId)
{
    if (FForge2HerdInfo* Herd = Herds.Find(HerdId))
    {
        TArray<FGuid> MembersToRemove = Herd->MemberIds;
        for (const FGuid& MemberId : MembersToRemove)
        {
            DespawnAnimal(MemberId);
        }

        Herds.Remove(HerdId);
    }
}

int32 UForge2EcosystemManager::GetAnimalCountBySpecies(FName SpeciesId) const
{
    int32 Count = 0;
    for (const auto& Pair : Animals)
    {
        if (Pair.Value.SpeciesId == SpeciesId)
        {
            Count++;
        }
    }
    return Count;
}

bool UForge2EcosystemManager::GetAnimalInfo(FGuid InstanceId, FForge2AnimalInstance& OutInfo) const
{
    if (const FForge2AnimalInstance* Found = Animals.Find(InstanceId))
    {
        OutInfo = *Found;
        return true;
    }
    return false;
}

TArray<FForge2AnimalInstance> UForge2EcosystemManager::GetAnimalsInRadius(const FVector& Center, float Radius) const
{
    TArray<FForge2AnimalInstance> Result;
    float RadiusSq = Radius * Radius;

    for (const auto& Pair : Animals)
    {
        if (Pair.Value.ActorRef.IsValid())
        {
            FVector AnimalLocation = Pair.Value.ActorRef->GetActorLocation();
            if (FVector::DistSquared(Center, AnimalLocation) <= RadiusSq)
            {
                Result.Add(Pair.Value);
            }
        }
    }

    return Result;
}

TArray<FForge2AnimalInstance> UForge2EcosystemManager::GetAnimalsBySpecies(FName SpeciesId) const
{
    TArray<FForge2AnimalInstance> Result;
    for (const auto& Pair : Animals)
    {
        if (Pair.Value.SpeciesId == SpeciesId)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

bool UForge2EcosystemManager::GetHerdInfo(FGuid HerdId, FForge2HerdInfo& OutInfo) const
{
    if (const FForge2HerdInfo* Found = Herds.Find(HerdId))
    {
        OutInfo = *Found;
        return true;
    }
    return false;
}

void UForge2EcosystemManager::SetHerdMigrationTarget(FGuid HerdId, const FVector& Target)
{
    if (FForge2HerdInfo* Herd = Herds.Find(HerdId))
    {
        Herd->MigrationTarget = Target;
    }
}

void UForge2EcosystemManager::StartHerdMigration(FGuid HerdId, const FVector& Target)
{
    if (FForge2HerdInfo* Herd = Herds.Find(HerdId))
    {
        Herd->MigrationTarget = Target;
        Herd->bIsMigrating = true;

        // Set all members to migrating behavior
        for (const FGuid& MemberId : Herd->MemberIds)
        {
            SetAnimalBehavior(MemberId, EForge2AnimalBehavior::Migrating);
        }
    }
}

void UForge2EcosystemManager::SetAnimalBehavior(FGuid InstanceId, EForge2AnimalBehavior Behavior)
{
    if (FForge2AnimalInstance* Animal = Animals.Find(InstanceId))
    {
        EForge2AnimalBehavior OldBehavior = Animal->CurrentBehavior;
        Animal->CurrentBehavior = Behavior;

        if (OldBehavior != Behavior)
        {
            OnAnimalBehaviorChanged.Broadcast(InstanceId, Behavior);
        }
    }
}

void UForge2EcosystemManager::FleeFrom(FGuid InstanceId, const FVector& ThreatLocation)
{
    if (FForge2AnimalInstance* Animal = Animals.Find(InstanceId))
    {
        Animal->CurrentBehavior = EForge2AnimalBehavior::Fleeing;
        OnAnimalBehaviorChanged.Broadcast(InstanceId, EForge2AnimalBehavior::Fleeing);

        // Set flee direction away from threat
        if (Animal->ActorRef.IsValid())
        {
            FVector FleeDirection = Animal->ActorRef->GetActorLocation() - ThreatLocation;
            FleeDirection.Normalize();

            // AI would use this direction to move
        }
    }
}

void UForge2EcosystemManager::AttackTarget(FGuid PredatorId, FGuid PreyId)
{
    if (FForge2AnimalInstance* Predator = Animals.Find(PredatorId))
    {
        Predator->CurrentBehavior = EForge2AnimalBehavior::Hunting;
        OnAnimalBehaviorChanged.Broadcast(PredatorId, EForge2AnimalBehavior::Hunting);
    }

    if (FForge2AnimalInstance* Prey = Animals.Find(PreyId))
    {
        Prey->CurrentBehavior = EForge2AnimalBehavior::Fleeing;
        OnAnimalBehaviorChanged.Broadcast(PreyId, EForge2AnimalBehavior::Fleeing);
    }
}

bool UForge2EcosystemManager::TryTameAnimal(FGuid InstanceId, FGuid PlayerId, float SuccessChance)
{
    FForge2AnimalInstance* Animal = Animals.Find(InstanceId);
    if (!Animal)
    {
        return false;
    }

    const FForge2AnimalSpecies* Species = RegisteredSpecies.Find(Animal->SpeciesId);
    if (!Species || !Species->bCanBeTamed)
    {
        return false;
    }

    float FinalChance = SuccessChance * (1.0f - Species->TamingDifficulty);
    if (FMath::RandRange(0.0f, 1.0f) <= FinalChance)
    {
        Animal->bIsTamed = true;
        Animal->OwnerId = PlayerId;
        return true;
    }

    return false;
}

void UForge2EcosystemManager::ReleaseAnimal(FGuid InstanceId)
{
    if (FForge2AnimalInstance* Animal = Animals.Find(InstanceId))
    {
        Animal->bIsTamed = false;
        Animal->OwnerId = FGuid();
    }
}

TArray<FForge2AnimalInstance> UForge2EcosystemManager::GetPlayerPets(FGuid PlayerId) const
{
    TArray<FForge2AnimalInstance> Result;
    for (const auto& Pair : Animals)
    {
        if (Pair.Value.bIsTamed && Pair.Value.OwnerId == PlayerId)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

void UForge2EcosystemManager::UpdateSpawning()
{
    if (Animals.Num() >= SpawningSettings.MaxAnimals)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    // Try to spawn new animals
    for (const auto& SpeciesPair : RegisteredSpecies)
    {
        float SpawnWeight = 1.0f;
        if (const float* Weight = SpawningSettings.SpeciesWeights.Find(SpeciesPair.Key))
        {
            SpawnWeight = *Weight;
        }

        if (FMath::RandRange(0.0f, 1.0f) <= SpawnWeight * 0.1f)
        {
            FVector SpawnLocation = FindSpawnLocation(SpeciesPair.Key);
            if (!SpawnLocation.IsZero())
            {
                SpawnHerd(SpeciesPair.Key, SpawnLocation);
            }
        }
    }
}

void UForge2EcosystemManager::UpdateAnimalBehaviors(float DeltaTime)
{
    for (auto& Pair : Animals)
    {
        FForge2AnimalInstance& Animal = Pair.Value;

        // Update hunger
        Animal.Hunger = FMath::Min(1.0f, Animal.Hunger + DeltaTime * 0.001f);

        // Update stamina
        if (Animal.CurrentBehavior == EForge2AnimalBehavior::Fleeing ||
            Animal.CurrentBehavior == EForge2AnimalBehavior::Hunting)
        {
            Animal.Stamina = FMath::Max(0.0f, Animal.Stamina - DeltaTime * 0.05f);
        }
        else
        {
            Animal.Stamina = FMath::Min(1.0f, Animal.Stamina + DeltaTime * 0.02f);
        }

        // Behavior state machine
        if (Animal.Stamina <= 0.1f && Animal.CurrentBehavior == EForge2AnimalBehavior::Fleeing)
        {
            SetAnimalBehavior(Animal.InstanceId, EForge2AnimalBehavior::Resting);
        }
    }
}

void UForge2EcosystemManager::UpdateHerds(float DeltaTime)
{
    for (auto& Pair : Herds)
    {
        FForge2HerdInfo& Herd = Pair.Value;

        // Update center location
        if (Herd.MemberIds.Num() > 0)
        {
            FVector Sum = FVector::ZeroVector;
            int32 ValidCount = 0;

            for (const FGuid& MemberId : Herd.MemberIds)
            {
                if (const FForge2AnimalInstance* Animal = Animals.Find(MemberId))
                {
                    if (Animal->ActorRef.IsValid())
                    {
                        Sum += Animal->ActorRef->GetActorLocation();
                        ValidCount++;
                    }
                }
            }

            if (ValidCount > 0)
            {
                Herd.CenterLocation = Sum / ValidCount;
            }
        }

        // Handle migration
        if (Herd.bIsMigrating)
        {
            float DistanceToTarget = FVector::Dist(Herd.CenterLocation, Herd.MigrationTarget);
            if (DistanceToTarget < 500.0f)
            {
                Herd.bIsMigrating = false;
                Herd.TerritoryCenter = Herd.MigrationTarget;

                // Return to normal behavior
                for (const FGuid& MemberId : Herd.MemberIds)
                {
                    SetAnimalBehavior(MemberId, EForge2AnimalBehavior::Grazing);
                }
            }
        }
    }
}

void UForge2EcosystemManager::ProcessPredatorPrey(float DeltaTime)
{
    // Check for predator-prey interactions
    for (auto& PredatorPair : Animals)
    {
        const FForge2AnimalSpecies* PredatorSpecies = RegisteredSpecies.Find(PredatorPair.Value.SpeciesId);
        if (!PredatorSpecies || !PredatorSpecies->bIsPredator)
        {
            continue;
        }

        if (!PredatorPair.Value.ActorRef.IsValid())
        {
            continue;
        }

        FVector PredatorLocation = PredatorPair.Value.ActorRef->GetActorLocation();

        // Look for prey
        for (auto& PreyPair : Animals)
        {
            if (PreyPair.Key == PredatorPair.Key)
            {
                continue;
            }

            if (!PredatorSpecies->PreySpecies.Contains(PreyPair.Value.SpeciesId))
            {
                continue;
            }

            if (!PreyPair.Value.ActorRef.IsValid())
            {
                continue;
            }

            FVector PreyLocation = PreyPair.Value.ActorRef->GetActorLocation();
            float Distance = FVector::Dist(PredatorLocation, PreyLocation);

            // Detection range
            if (Distance < 2000.0f && PredatorPair.Value.CurrentBehavior != EForge2AnimalBehavior::Hunting)
            {
                if (PredatorPair.Value.Hunger > 0.3f) // Only hunt if hungry
                {
                    AttackTarget(PredatorPair.Key, PreyPair.Key);
                }
            }
        }
    }
}

FVector UForge2EcosystemManager::FindSpawnLocation(FName SpeciesId)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return FVector::ZeroVector;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Try multiple random locations
    for (int32 i = 0; i < 10; i++)
    {
        float RandomAngle = FMath::RandRange(0.0f, 360.0f);
        float RandomDistance = FMath::RandRange(SpawningSettings.MinSpawnDistance, SpawningSettings.SpawnRadius);

        FVector TestLocation = PlayerLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
            FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
            0.0f
        );

        if (IsValidSpawnLocation(TestLocation, SpeciesId))
        {
            return TestLocation;
        }
    }

    return FVector::ZeroVector;
}

bool UForge2EcosystemManager::IsValidSpawnLocation(const FVector& Location, FName SpeciesId)
{
    // Check if on nav mesh
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (!NavSystem->ProjectPointToNavigation(Location, NavLocation))
        {
            return false;
        }
    }

    // Check biome compatibility
    // This would involve checking the biome at the location

    return true;
}

void UForge2EcosystemManager::CleanupDistantAnimals()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    TArray<FGuid> ToRemove;

    for (const auto& Pair : Animals)
    {
        // Don't despawn tamed animals
        if (Pair.Value.bIsTamed)
        {
            continue;
        }

        if (Pair.Value.ActorRef.IsValid())
        {
            float Distance = FVector::Dist(PlayerLocation, Pair.Value.ActorRef->GetActorLocation());
            if (Distance > SpawningSettings.DespawnRadius)
            {
                ToRemove.Add(Pair.Key);
            }
        }
    }

    for (const FGuid& Id : ToRemove)
    {
        DespawnAnimal(Id);
    }
}

// ============================================================================
// NPC SCHEDULE MANAGER IMPLEMENTATION
// ============================================================================

UForge2NPCScheduleManager::UForge2NPCScheduleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UForge2NPCScheduleManager::BeginPlay()
{
    Super::BeginPlay();

    // Find time manager in world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        UForge2TimeManager* TimeMgr = Actor->FindComponentByClass<UForge2TimeManager>();
        if (TimeMgr)
        {
            TimeManager = TimeMgr;
            break;
        }
    }
}

void UForge2NPCScheduleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Get current time
    int32 CurrentHour = 12;
    int32 CurrentMinute = 0;
    int32 CurrentDay = 1;

    if (TimeManager.IsValid())
    {
        CurrentHour = TimeManager->GetCurrentHour();
        CurrentMinute = TimeManager->GetCurrentMinute();
        CurrentDay = TimeManager->GetCurrentDay();
    }

    // Update schedules if hour changed
    if (CurrentHour != LastProcessedHour)
    {
        LastProcessedHour = CurrentHour;
        int32 DayOfWeek = CurrentDay % 7;
        UpdateNPCSchedules(CurrentHour, CurrentMinute, DayOfWeek);
    }

    // Update memory decay
    MemoryDecayTimer += DeltaTime;
    if (MemoryDecayTimer >= 60.0f) // Every minute
    {
        MemoryDecayTimer = 0.0f;
        UpdateNPCMemoryDecay(60.0f);
    }

    // Handle interrupted NPCs
    float CurrentGameTime = GetWorld()->GetTimeSeconds();
    TArray<FGuid> EndedInterrupts;

    for (const auto& Pair : InterruptedNPCs)
    {
        if (Pair.Value > 0.0f && CurrentGameTime >= Pair.Value)
        {
            EndedInterrupts.Add(Pair.Key);
        }
    }

    for (const FGuid& NPCId : EndedInterrupts)
    {
        ResumeNormalSchedule(NPCId);
    }
}

FGuid UForge2NPCScheduleManager::RegisterNPC(const FForge2NPCInfo& NPCInfo)
{
    FForge2NPCInfo NewInfo = NPCInfo;
    if (!NewInfo.NPCId.IsValid())
    {
        NewInfo.NPCId = FGuid::NewGuid();
    }

    RegisteredNPCs.Add(NewInfo.NPCId, NewInfo);
    return NewInfo.NPCId;
}

void UForge2NPCScheduleManager::UnregisterNPC(FGuid NPCId)
{
    RegisteredNPCs.Remove(NPCId);
    InterruptedNPCs.Remove(NPCId);
    InterruptedActivities.Remove(NPCId);
}

bool UForge2NPCScheduleManager::GetNPCInfo(FGuid NPCId, FForge2NPCInfo& OutInfo) const
{
    if (const FForge2NPCInfo* Found = RegisteredNPCs.Find(NPCId))
    {
        OutInfo = *Found;
        return true;
    }
    return false;
}

void UForge2NPCScheduleManager::UpdateNPCInfo(FGuid NPCId, const FForge2NPCInfo& NewInfo)
{
    if (FForge2NPCInfo* Existing = RegisteredNPCs.Find(NPCId))
    {
        *Existing = NewInfo;
        Existing->NPCId = NPCId; // Preserve ID
    }
}

void UForge2NPCScheduleManager::SetNPCSchedule(FGuid NPCId, const FForge2NPCSchedule& Schedule)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        NPC->Schedule = Schedule;
    }
}

void UForge2NPCScheduleManager::AddScheduleEntry(FGuid NPCId, const FForge2NPCScheduleEntry& Entry)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        NPC->Schedule.Entries.Add(Entry);
    }
}

void UForge2NPCScheduleManager::ClearSchedule(FGuid NPCId)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        NPC->Schedule.Entries.Empty();
    }
}

EForge2NPCActivity UForge2NPCScheduleManager::GetCurrentActivity(FGuid NPCId) const
{
    // Check if interrupted
    if (const EForge2NPCActivity* Override = InterruptedActivities.Find(NPCId))
    {
        return *Override;
    }

    if (const FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        return NPC->CurrentActivity;
    }

    return EForge2NPCActivity::Working;
}

FVector UForge2NPCScheduleManager::GetCurrentActivityLocation(FGuid NPCId) const
{
    if (const FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        int32 CurrentHour = TimeManager.IsValid() ? TimeManager->GetCurrentHour() : 12;
        int32 DayOfWeek = TimeManager.IsValid() ? TimeManager->GetCurrentDay() % 7 : 0;

        for (const FForge2NPCScheduleEntry& Entry : NPC->Schedule.Entries)
        {
            if (CurrentHour >= Entry.StartHour && CurrentHour < Entry.EndHour)
            {
                if (Entry.RequiredDays & (1 << DayOfWeek))
                {
                    if (Entry.LocationActor.IsValid())
                    {
                        return Entry.LocationActor->GetActorLocation();
                    }
                    return Entry.ActivityLocation;
                }
            }
        }

        // Default to home location
        return NPC->Schedule.HomeLocation;
    }

    return FVector::ZeroVector;
}

void UForge2NPCScheduleManager::SetNPCMood(FGuid NPCId, EForge2NPCMood Mood)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        EForge2NPCMood OldMood = NPC->Mood;
        NPC->Mood = Mood;

        if (OldMood != Mood)
        {
            OnNPCMoodChanged.Broadcast(NPCId, Mood);
        }
    }
}

void UForge2NPCScheduleManager::ModifyPlayerRelationship(FGuid NPCId, float Delta)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        NPC->PlayerRelationship = FMath::Clamp(NPC->PlayerRelationship + Delta, -100.0f, 100.0f);
    }
}

void UForge2NPCScheduleManager::SetNPCRelationship(FGuid NPCId, FGuid TargetNPCId, float Value)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        for (FForge2NPCRelationship& Relationship : NPC->Relationships)
        {
            if (Relationship.TargetNPCId == TargetNPCId)
            {
                Relationship.RelationshipValue = FMath::Clamp(Value, -100.0f, 100.0f);
                return;
            }
        }

        // Add new relationship
        FForge2NPCRelationship NewRelation;
        NewRelation.TargetNPCId = TargetNPCId;
        NewRelation.RelationshipValue = FMath::Clamp(Value, -100.0f, 100.0f);
        NPC->Relationships.Add(NewRelation);
    }
}

float UForge2NPCScheduleManager::GetPlayerRelationship(FGuid NPCId) const
{
    if (const FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        return NPC->PlayerRelationship;
    }
    return 0.0f;
}

void UForge2NPCScheduleManager::AddNPCMemory(FGuid NPCId, const FForge2NPCMemory& Memory)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        // Check if memory with same key exists
        for (FForge2NPCMemory& Existing : NPC->Memories)
        {
            if (Existing.MemoryKey == Memory.MemoryKey)
            {
                Existing = Memory;
                return;
            }
        }

        NPC->Memories.Add(Memory);
    }
}

bool UForge2NPCScheduleManager::GetNPCMemory(FGuid NPCId, FName MemoryKey, FForge2NPCMemory& OutMemory) const
{
    if (const FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        for (const FForge2NPCMemory& Memory : NPC->Memories)
        {
            if (Memory.MemoryKey == MemoryKey)
            {
                OutMemory = Memory;
                return true;
            }
        }
    }
    return false;
}

void UForge2NPCScheduleManager::ClearNPCMemory(FGuid NPCId, FName MemoryKey)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        NPC->Memories.RemoveAll([MemoryKey](const FForge2NPCMemory& M) { return M.MemoryKey == MemoryKey; });
    }
}

TArray<FForge2NPCMemory> UForge2NPCScheduleManager::GetNPCMemories(FGuid NPCId) const
{
    if (const FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        return NPC->Memories;
    }
    return TArray<FForge2NPCMemory>();
}

void UForge2NPCScheduleManager::InterruptNPC(FGuid NPCId, EForge2NPCActivity OverrideActivity, float Duration)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        float EndTime = Duration > 0.0f ? GetWorld()->GetTimeSeconds() + Duration : -1.0f;
        InterruptedNPCs.Add(NPCId, EndTime);
        InterruptedActivities.Add(NPCId, OverrideActivity);

        EForge2NPCActivity OldActivity = NPC->CurrentActivity;
        NPC->CurrentActivity = OverrideActivity;

        if (OldActivity != OverrideActivity)
        {
            OnNPCActivityChanged.Broadcast(NPCId, OverrideActivity);
        }
    }
}

void UForge2NPCScheduleManager::ResumeNormalSchedule(FGuid NPCId)
{
    InterruptedNPCs.Remove(NPCId);
    InterruptedActivities.Remove(NPCId);

    // Trigger schedule update
    if (TimeManager.IsValid())
    {
        int32 Hour = TimeManager->GetCurrentHour();
        int32 Minute = TimeManager->GetCurrentMinute();
        int32 DayOfWeek = TimeManager->GetCurrentDay() % 7;

        if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
        {
            FForge2NPCScheduleEntry* Entry = FindCurrentScheduleEntry(NPC->Schedule, Hour, DayOfWeek);
            if (Entry)
            {
                MoveNPCToActivity(NPCId, *Entry);
            }
        }
    }
}

bool UForge2NPCScheduleManager::IsNPCInterrupted(FGuid NPCId) const
{
    return InterruptedNPCs.Contains(NPCId);
}

void UForge2NPCScheduleManager::SpawnCrowd(const FVector& CenterLocation, int32 Count, float Radius)
{
    // Crowd spawning would create temporary NPC actors for ambient population
    // This is a placeholder for full implementation
}

void UForge2NPCScheduleManager::DespawnCrowd(const FVector& CenterLocation, float Radius)
{
    // Despawn crowd NPCs in radius
}

TArray<FGuid> UForge2NPCScheduleManager::GetAllNPCIds() const
{
    TArray<FGuid> Ids;
    RegisteredNPCs.GetKeys(Ids);
    return Ids;
}

TArray<FForge2NPCInfo> UForge2NPCScheduleManager::GetNPCsInRadius(const FVector& Center, float Radius) const
{
    TArray<FForge2NPCInfo> Result;
    float RadiusSq = Radius * Radius;

    for (const auto& Pair : RegisteredNPCs)
    {
        if (Pair.Value.ActorRef.IsValid())
        {
            if (FVector::DistSquared(Center, Pair.Value.ActorRef->GetActorLocation()) <= RadiusSq)
            {
                Result.Add(Pair.Value);
            }
        }
    }

    return Result;
}

TArray<FForge2NPCInfo> UForge2NPCScheduleManager::GetNPCsByActivity(EForge2NPCActivity Activity) const
{
    TArray<FForge2NPCInfo> Result;
    for (const auto& Pair : RegisteredNPCs)
    {
        if (Pair.Value.CurrentActivity == Activity)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

TArray<FForge2NPCInfo> UForge2NPCScheduleManager::GetNPCsByMood(EForge2NPCMood Mood) const
{
    TArray<FForge2NPCInfo> Result;
    for (const auto& Pair : RegisteredNPCs)
    {
        if (Pair.Value.Mood == Mood)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

void UForge2NPCScheduleManager::UpdateNPCSchedules(int32 CurrentHour, int32 CurrentMinute, int32 CurrentDayOfWeek)
{
    for (auto& Pair : RegisteredNPCs)
    {
        // Skip interrupted NPCs
        if (InterruptedNPCs.Contains(Pair.Key))
        {
            continue;
        }

        FForge2NPCScheduleEntry* Entry = FindCurrentScheduleEntry(Pair.Value.Schedule, CurrentHour, CurrentDayOfWeek);
        if (Entry)
        {
            if (Pair.Value.CurrentActivity != Entry->Activity)
            {
                MoveNPCToActivity(Pair.Key, *Entry);
            }
        }
        else
        {
            // Check if should be sleeping
            bool bShouldSleep = false;
            if (CurrentHour >= Pair.Value.Schedule.SleepStartHour || CurrentHour < Pair.Value.Schedule.WakeHour)
            {
                bShouldSleep = true;
            }

            if (bShouldSleep && Pair.Value.CurrentActivity != EForge2NPCActivity::Sleeping)
            {
                EForge2NPCActivity OldActivity = Pair.Value.CurrentActivity;
                Pair.Value.CurrentActivity = EForge2NPCActivity::Sleeping;
                OnNPCActivityChanged.Broadcast(Pair.Key, EForge2NPCActivity::Sleeping);
            }
        }
    }
}

void UForge2NPCScheduleManager::UpdateNPCMemoryDecay(float DeltaTime)
{
    for (auto& Pair : RegisteredNPCs)
    {
        TArray<int32> ToRemove;

        for (int32 i = 0; i < Pair.Value.Memories.Num(); i++)
        {
            FForge2NPCMemory& Memory = Pair.Value.Memories[i];

            if (!Memory.bIsPermanent)
            {
                Memory.Strength -= DeltaTime * 0.0001f; // Very slow decay
                if (Memory.Strength <= 0.0f)
                {
                    ToRemove.Add(i);
                }
            }
        }

        // Remove decayed memories (in reverse order)
        for (int32 i = ToRemove.Num() - 1; i >= 0; i--)
        {
            Pair.Value.Memories.RemoveAt(ToRemove[i]);
        }
    }
}

FForge2NPCScheduleEntry* UForge2NPCScheduleManager::FindCurrentScheduleEntry(const FForge2NPCSchedule& Schedule, int32 Hour, int32 DayOfWeek)
{
    FForge2NPCScheduleEntry* BestEntry = nullptr;
    int32 BestPriority = -1;

    for (FForge2NPCScheduleEntry& Entry : const_cast<FForge2NPCSchedule&>(Schedule).Entries)
    {
        if (Hour >= Entry.StartHour && Hour < Entry.EndHour)
        {
            if (Entry.RequiredDays & (1 << DayOfWeek))
            {
                if (Entry.Priority > BestPriority)
                {
                    BestEntry = &Entry;
                    BestPriority = Entry.Priority;
                }
            }
        }
    }

    return BestEntry;
}

void UForge2NPCScheduleManager::MoveNPCToActivity(FGuid NPCId, const FForge2NPCScheduleEntry& Entry)
{
    if (FForge2NPCInfo* NPC = RegisteredNPCs.Find(NPCId))
    {
        EForge2NPCActivity OldActivity = NPC->CurrentActivity;
        NPC->CurrentActivity = Entry.Activity;

        if (OldActivity != Entry.Activity)
        {
            OnNPCActivityChanged.Broadcast(NPCId, Entry.Activity);
        }

        // Move actor to activity location
        if (NPC->ActorRef.IsValid())
        {
            FVector TargetLocation = Entry.LocationActor.IsValid() ?
                Entry.LocationActor->GetActorLocation() : Entry.ActivityLocation;

            // This would trigger AI movement to location
        }
    }
}

// ============================================================================
// ENVIRONMENT UTILITIES IMPLEMENTATION
// ============================================================================

FString UForge2EnvironmentUtilities::FormatGameTime(int32 Hour, int32 Minute, bool bUse24Hour)
{
    if (bUse24Hour)
    {
        return FString::Printf(TEXT("%02d:%02d"), Hour, Minute);
    }
    else
    {
        int32 DisplayHour = Hour % 12;
        if (DisplayHour == 0) DisplayHour = 12;
        FString Suffix = Hour >= 12 ? TEXT("PM") : TEXT("AM");
        return FString::Printf(TEXT("%d:%02d %s"), DisplayHour, Minute, *Suffix);
    }
}

FString UForge2EnvironmentUtilities::FormatGameDate(int32 Day, int32 Month, int32 Year)
{
    return FString::Printf(TEXT("%d/%d/%d"), Month, Day, Year);
}

int32 UForge2EnvironmentUtilities::GetDayOfWeek(int32 Day, int32 Month, int32 Year, int32 DaysPerMonth, int32 MonthsPerYear)
{
    int32 TotalDays = (Year - 1) * MonthsPerYear * DaysPerMonth + (Month - 1) * DaysPerMonth + Day;
    return TotalDays % 7;
}

EForge2TimeOfDay UForge2EnvironmentUtilities::GetTimeOfDayFromHour(int32 Hour)
{
    if (Hour >= 5 && Hour < 7) return EForge2TimeOfDay::Dawn;
    if (Hour >= 7 && Hour < 12) return EForge2TimeOfDay::Morning;
    if (Hour >= 12 && Hour < 14) return EForge2TimeOfDay::Noon;
    if (Hour >= 14 && Hour < 17) return EForge2TimeOfDay::Afternoon;
    if (Hour >= 17 && Hour < 20) return EForge2TimeOfDay::Dusk;
    if (Hour >= 20 && Hour < 22) return EForge2TimeOfDay::Evening;
    if (Hour >= 22 || Hour < 2) return EForge2TimeOfDay::Night;
    return EForge2TimeOfDay::Midnight;
}

EForge2Season UForge2EnvironmentUtilities::GetSeasonFromMonth(int32 Month, int32 MonthsPerYear)
{
    float MonthFraction = (float)(Month - 1) / (float)MonthsPerYear;

    if (MonthFraction < 0.25f) return EForge2Season::Spring;
    if (MonthFraction < 0.5f) return EForge2Season::Summer;
    if (MonthFraction < 0.75f) return EForge2Season::Autumn;
    return EForge2Season::Winter;
}

float UForge2EnvironmentUtilities::GetWindChill(float Temperature, float WindSpeed)
{
    if (Temperature > 10.0f || WindSpeed < 4.8f)
    {
        return Temperature;
    }

    // Wind chill formula (Celsius)
    return 13.12f + 0.6215f * Temperature - 11.37f * FMath::Pow(WindSpeed, 0.16f) +
           0.3965f * Temperature * FMath::Pow(WindSpeed, 0.16f);
}

float UForge2EnvironmentUtilities::GetHeatIndex(float Temperature, float Humidity)
{
    if (Temperature < 27.0f)
    {
        return Temperature;
    }

    // Simplified heat index (Celsius)
    float T = Temperature;
    float R = Humidity * 100.0f;

    float HI = -8.784695f + 1.61139411f * T + 2.338549f * R
               - 0.14611605f * T * R - 0.012308094f * T * T
               - 0.016424828f * R * R + 0.002211732f * T * T * R
               + 0.00072546f * T * R * R - 0.000003582f * T * T * R * R;

    return HI;
}

bool UForge2EnvironmentUtilities::CanSnow(float Temperature)
{
    return Temperature <= 2.0f;
}

FLinearColor UForge2EnvironmentUtilities::GetWeatherLightingTint(EForge2WeatherType WeatherType, EForge2WeatherIntensity Intensity)
{
    float IntensityMod = 1.0f - (static_cast<float>(Intensity) * 0.1f);

    switch (WeatherType)
    {
    case EForge2WeatherType::Clear:
        return FLinearColor(1.0f, 1.0f, 1.0f) * IntensityMod;
    case EForge2WeatherType::Cloudy:
        return FLinearColor(0.9f, 0.9f, 0.95f) * IntensityMod;
    case EForge2WeatherType::Overcast:
        return FLinearColor(0.7f, 0.7f, 0.75f) * IntensityMod;
    case EForge2WeatherType::Foggy:
        return FLinearColor(0.8f, 0.8f, 0.85f) * IntensityMod;
    case EForge2WeatherType::Rainy:
        return FLinearColor(0.6f, 0.65f, 0.7f) * IntensityMod;
    case EForge2WeatherType::Stormy:
        return FLinearColor(0.4f, 0.45f, 0.5f) * IntensityMod;
    case EForge2WeatherType::Snowy:
        return FLinearColor(0.9f, 0.95f, 1.0f) * IntensityMod;
    case EForge2WeatherType::Blizzard:
        return FLinearColor(0.95f, 0.95f, 1.0f) * 0.5f;
    case EForge2WeatherType::Sandstorm:
        return FLinearColor(0.9f, 0.7f, 0.5f) * IntensityMod;
    case EForge2WeatherType::Heatwave:
        return FLinearColor(1.0f, 0.95f, 0.8f);
    default:
        return FLinearColor::White;
    }
}

float UForge2EnvironmentUtilities::GetWeatherVisibilityMultiplier(EForge2WeatherType WeatherType, EForge2WeatherIntensity Intensity)
{
    float IntensityMod = 1.0f - (static_cast<float>(Intensity) * 0.2f);

    switch (WeatherType)
    {
    case EForge2WeatherType::Clear: return 1.0f;
    case EForge2WeatherType::Cloudy: return 0.95f;
    case EForge2WeatherType::Overcast: return 0.9f;
    case EForge2WeatherType::Foggy: return 0.3f * IntensityMod;
    case EForge2WeatherType::Rainy: return 0.7f * IntensityMod;
    case EForge2WeatherType::Stormy: return 0.4f;
    case EForge2WeatherType::Snowy: return 0.6f * IntensityMod;
    case EForge2WeatherType::Blizzard: return 0.1f;
    case EForge2WeatherType::Sandstorm: return 0.2f;
    case EForge2WeatherType::Heatwave: return 0.85f;
    default: return 1.0f;
    }
}

bool UForge2EnvironmentUtilities::IsPreyFor(const FForge2AnimalSpecies& Prey, const FForge2AnimalSpecies& Predator)
{
    return Predator.PreySpecies.Contains(Prey.SpeciesId);
}

bool UForge2EnvironmentUtilities::CanSpawnInBiome(const FForge2AnimalSpecies& Species, EForge2BiomeType Biome)
{
    return Species.PreferredBiomes.Contains(Biome);
}

bool UForge2EnvironmentUtilities::IsActiveAtTime(const FForge2AnimalSpecies& Species, bool bIsNight)
{
    if (bIsNight)
    {
        return Species.bNocturnal;
    }
    return Species.bDiurnal;
}

float UForge2EnvironmentUtilities::CalculateTamingChance(const FForge2AnimalSpecies& Species, float PlayerSkill)
{
    if (!Species.bCanBeTamed)
    {
        return 0.0f;
    }

    float BaseChance = 1.0f - Species.TamingDifficulty;
    return FMath::Clamp(BaseChance * PlayerSkill, 0.0f, 1.0f);
}

FForge2TimeSettings UForge2EnvironmentUtilities::GetDefaultTimeSettings()
{
    FForge2TimeSettings Settings;
    Settings.SecondsPerHour = 60.0f;
    Settings.StartingHour = 8;
    Settings.StartingMinute = 0;
    Settings.StartingDay = 1;
    Settings.StartingMonth = 6;
    Settings.StartingYear = 1;
    Settings.DaysPerMonth = 30;
    Settings.MonthsPerYear = 12;
    return Settings;
}

FForge2TimeSettings UForge2EnvironmentUtilities::GetRealtimeSettings()
{
    FForge2TimeSettings Settings;
    Settings.SecondsPerHour = 3600.0f; // Real time

    FDateTime Now = FDateTime::Now();
    Settings.StartingHour = Now.GetHour();
    Settings.StartingMinute = Now.GetMinute();
    Settings.StartingDay = Now.GetDay();
    Settings.StartingMonth = Now.GetMonth();
    Settings.StartingYear = Now.GetYear();
    Settings.DaysPerMonth = 30;
    Settings.MonthsPerYear = 12;
    return Settings;
}

FForge2SunSettings UForge2EnvironmentUtilities::GetEarthlikeSunSettings()
{
    FForge2SunSettings Settings;
    Settings.SunriseHour = 6.0f;
    Settings.SunsetHour = 18.0f;
    Settings.DawnColor = FLinearColor(1.0f, 0.6f, 0.4f);
    Settings.NoonColor = FLinearColor(1.0f, 0.98f, 0.95f);
    Settings.DuskColor = FLinearColor(1.0f, 0.5f, 0.3f);
    Settings.DayIntensity = 10.0f;
    Settings.NightIntensity = 0.1f;
    return Settings;
}

TArray<FForge2SeasonSettings> UForge2EnvironmentUtilities::GetTemperateSeasons()
{
    TArray<FForge2SeasonSettings> Seasons;

    FForge2SeasonSettings Spring;
    Spring.Season = EForge2Season::Spring;
    Spring.DaysPerSeason = 90;
    Spring.TemperatureModifier = -5.0f;
    Spring.DayLengthModifier = 1.0f;
    Spring.FoliageColorTint = FLinearColor(0.8f, 1.0f, 0.8f);
    Spring.GrassColorTint = FLinearColor(0.9f, 1.0f, 0.8f);
    Seasons.Add(Spring);

    FForge2SeasonSettings Summer;
    Summer.Season = EForge2Season::Summer;
    Summer.DaysPerSeason = 90;
    Summer.TemperatureModifier = 10.0f;
    Summer.DayLengthModifier = 2.0f;
    Summer.FoliageColorTint = FLinearColor(0.2f, 0.8f, 0.2f);
    Summer.GrassColorTint = FLinearColor(0.3f, 0.9f, 0.2f);
    Seasons.Add(Summer);

    FForge2SeasonSettings Autumn;
    Autumn.Season = EForge2Season::Autumn;
    Autumn.DaysPerSeason = 90;
    Autumn.TemperatureModifier = -5.0f;
    Autumn.DayLengthModifier = -1.0f;
    Autumn.FoliageColorTint = FLinearColor(1.0f, 0.6f, 0.2f);
    Autumn.GrassColorTint = FLinearColor(0.8f, 0.7f, 0.3f);
    Seasons.Add(Autumn);

    FForge2SeasonSettings Winter;
    Winter.Season = EForge2Season::Winter;
    Winter.DaysPerSeason = 90;
    Winter.TemperatureModifier = -20.0f;
    Winter.DayLengthModifier = -2.0f;
    Winter.FoliageColorTint = FLinearColor(0.5f, 0.4f, 0.3f);
    Winter.GrassColorTint = FLinearColor(0.9f, 0.9f, 1.0f);
    Seasons.Add(Winter);

    return Seasons;
}

TArray<FForge2SeasonSettings> UForge2EnvironmentUtilities::GetTropicalSeasons()
{
    TArray<FForge2SeasonSettings> Seasons;

    FForge2SeasonSettings Dry;
    Dry.Season = EForge2Season::Summer;
    Dry.DaysPerSeason = 180;
    Dry.TemperatureModifier = 5.0f;
    Dry.FoliageColorTint = FLinearColor(0.6f, 0.8f, 0.3f);
    Seasons.Add(Dry);

    FForge2SeasonSettings Wet;
    Wet.Season = EForge2Season::Winter;
    Wet.DaysPerSeason = 180;
    Wet.TemperatureModifier = 0.0f;
    Wet.FoliageColorTint = FLinearColor(0.2f, 0.9f, 0.3f);
    Seasons.Add(Wet);

    return Seasons;
}

TArray<FForge2SeasonSettings> UForge2EnvironmentUtilities::GetArcticSeasons()
{
    TArray<FForge2SeasonSettings> Seasons;

    FForge2SeasonSettings ArcticSummer;
    ArcticSummer.Season = EForge2Season::Summer;
    ArcticSummer.DaysPerSeason = 90;
    ArcticSummer.TemperatureModifier = -10.0f;
    ArcticSummer.DayLengthModifier = 6.0f; // Midnight sun
    Seasons.Add(ArcticSummer);

    FForge2SeasonSettings ArcticWinter;
    ArcticWinter.Season = EForge2Season::Winter;
    ArcticWinter.DaysPerSeason = 270;
    ArcticWinter.TemperatureModifier = -40.0f;
    ArcticWinter.DayLengthModifier = -6.0f; // Polar night
    Seasons.Add(ArcticWinter);

    return Seasons;
}

FForge2WeatherState UForge2EnvironmentUtilities::GetClearWeather()
{
    FForge2WeatherState State;
    State.WeatherType = EForge2WeatherType::Clear;
    State.Intensity = EForge2WeatherIntensity::None;
    State.CloudCoverage = 0.1f;
    State.WindSpeed = 5.0f;
    State.Temperature = 20.0f;
    State.Humidity = 0.4f;
    State.VisibilityRange = 10000.0f;
    return State;
}

FForge2WeatherState UForge2EnvironmentUtilities::GetRainyWeather(EForge2WeatherIntensity Intensity)
{
    FForge2WeatherState State;
    State.WeatherType = EForge2WeatherType::Rainy;
    State.Intensity = Intensity;
    State.CloudCoverage = 0.7f + static_cast<float>(Intensity) * 0.1f;
    State.WindSpeed = 10.0f + static_cast<float>(Intensity) * 5.0f;
    State.Precipitation = static_cast<float>(Intensity) * 0.25f;
    State.Temperature = 15.0f;
    State.Humidity = 0.8f;
    State.VisibilityRange = 5000.0f - static_cast<float>(Intensity) * 1000.0f;
    return State;
}

FForge2WeatherState UForge2EnvironmentUtilities::GetStormyWeather()
{
    FForge2WeatherState State;
    State.WeatherType = EForge2WeatherType::Stormy;
    State.Intensity = EForge2WeatherIntensity::Heavy;
    State.CloudCoverage = 1.0f;
    State.WindSpeed = 35.0f;
    State.Precipitation = 0.8f;
    State.Temperature = 12.0f;
    State.Humidity = 0.95f;
    State.LightningFrequency = 3.0f;
    State.VisibilityRange = 1000.0f;
    return State;
}

FForge2WeatherState UForge2EnvironmentUtilities::GetSnowyWeather(EForge2WeatherIntensity Intensity)
{
    FForge2WeatherState State;
    State.WeatherType = EForge2WeatherType::Snowy;
    State.Intensity = Intensity;
    State.CloudCoverage = 0.8f;
    State.WindSpeed = 8.0f + static_cast<float>(Intensity) * 3.0f;
    State.Precipitation = static_cast<float>(Intensity) * 0.25f;
    State.Temperature = -5.0f - static_cast<float>(Intensity) * 2.0f;
    State.Humidity = 0.7f;
    State.VisibilityRange = 3000.0f - static_cast<float>(Intensity) * 500.0f;
    return State;
}

FForge2WeatherState UForge2EnvironmentUtilities::GetFoggyWeather(EForge2WeatherIntensity Intensity)
{
    FForge2WeatherState State;
    State.WeatherType = EForge2WeatherType::Foggy;
    State.Intensity = Intensity;
    State.CloudCoverage = 0.5f;
    State.WindSpeed = 2.0f;
    State.FogDensity = static_cast<float>(Intensity) * 0.25f;
    State.Temperature = 10.0f;
    State.Humidity = 0.9f;
    State.VisibilityRange = 500.0f / (static_cast<float>(Intensity) + 1.0f);
    return State;
}

FForge2AnimalSpecies UForge2EnvironmentUtilities::GetDeerSpecies()
{
    FForge2AnimalSpecies Species;
    Species.SpeciesId = FName("Deer");
    Species.DisplayName = FText::FromString("Deer");
    Species.Diet = EForge2AnimalDiet::Herbivore;
    Species.Size = EForge2AnimalSize::Medium;
    Species.PreferredBiomes = { EForge2BiomeType::Forest, EForge2BiomeType::Grassland };
    Species.bDiurnal = true;
    Species.bNocturnal = false;
    Species.HerdSizeRange = FIntPoint(3, 8);
    Species.AggressionLevel = 0.1f;
    Species.FearLevel = 0.8f;
    Species.CuriosityLevel = 0.4f;
    Species.MovementSpeed = 400.0f;
    Species.RunSpeedMultiplier = 2.5f;
    Species.bCanBeTamed = true;
    Species.TamingDifficulty = 0.6f;
    return Species;
}

FForge2AnimalSpecies UForge2EnvironmentUtilities::GetWolfSpecies()
{
    FForge2AnimalSpecies Species;
    Species.SpeciesId = FName("Wolf");
    Species.DisplayName = FText::FromString("Wolf");
    Species.Diet = EForge2AnimalDiet::Carnivore;
    Species.Size = EForge2AnimalSize::Medium;
    Species.PreferredBiomes = { EForge2BiomeType::Forest, EForge2BiomeType::Tundra, EForge2BiomeType::Mountain };
    Species.bDiurnal = true;
    Species.bNocturnal = true;
    Species.bIsPredator = true;
    Species.PreySpecies = { FName("Deer"), FName("Rabbit") };
    Species.HerdSizeRange = FIntPoint(3, 7);
    Species.AggressionLevel = 0.6f;
    Species.FearLevel = 0.3f;
    Species.CuriosityLevel = 0.5f;
    Species.MovementSpeed = 500.0f;
    Species.RunSpeedMultiplier = 2.0f;
    Species.bCanBeTamed = true;
    Species.TamingDifficulty = 0.8f;
    return Species;
}

FForge2AnimalSpecies UForge2EnvironmentUtilities::GetRabbitSpecies()
{
    FForge2AnimalSpecies Species;
    Species.SpeciesId = FName("Rabbit");
    Species.DisplayName = FText::FromString("Rabbit");
    Species.Diet = EForge2AnimalDiet::Herbivore;
    Species.Size = EForge2AnimalSize::Small;
    Species.PreferredBiomes = { EForge2BiomeType::Forest, EForge2BiomeType::Grassland };
    Species.bDiurnal = true;
    Species.bNocturnal = false;
    Species.PredatorSpecies = { FName("Wolf"), FName("Bear") };
    Species.HerdSizeRange = FIntPoint(2, 5);
    Species.AggressionLevel = 0.0f;
    Species.FearLevel = 0.95f;
    Species.CuriosityLevel = 0.3f;
    Species.MovementSpeed = 350.0f;
    Species.RunSpeedMultiplier = 3.0f;
    Species.bCanBeTamed = true;
    Species.TamingDifficulty = 0.3f;
    return Species;
}

FForge2AnimalSpecies UForge2EnvironmentUtilities::GetBearSpecies()
{
    FForge2AnimalSpecies Species;
    Species.SpeciesId = FName("Bear");
    Species.DisplayName = FText::FromString("Bear");
    Species.Diet = EForge2AnimalDiet::Omnivore;
    Species.Size = EForge2AnimalSize::Large;
    Species.PreferredBiomes = { EForge2BiomeType::Forest, EForge2BiomeType::Mountain };
    Species.bDiurnal = true;
    Species.bNocturnal = false;
    Species.bIsPredator = true;
    Species.PreySpecies = { FName("Deer"), FName("Rabbit") };
    Species.HerdSizeRange = FIntPoint(1, 2);
    Species.AggressionLevel = 0.5f;
    Species.FearLevel = 0.1f;
    Species.CuriosityLevel = 0.6f;
    Species.MovementSpeed = 300.0f;
    Species.RunSpeedMultiplier = 1.8f;
    Species.bCanBeTamed = false;
    Species.TamingDifficulty = 0.95f;
    return Species;
}

FForge2NPCSchedule UForge2EnvironmentUtilities::GetShopkeeperSchedule()
{
    FForge2NPCSchedule Schedule;
    Schedule.SleepStartHour = 22;
    Schedule.WakeHour = 7;
    Schedule.bRandomizeSchedule = true;
    Schedule.RandomizationVariance = 0.25f;

    // Morning setup
    FForge2NPCScheduleEntry MorningEntry;
    MorningEntry.Activity = EForge2NPCActivity::Working;
    MorningEntry.StartHour = 7;
    MorningEntry.EndHour = 12;
    MorningEntry.Priority = 10;
    Schedule.Entries.Add(MorningEntry);

    // Lunch break
    FForge2NPCScheduleEntry LunchEntry;
    LunchEntry.Activity = EForge2NPCActivity::Eating;
    LunchEntry.StartHour = 12;
    LunchEntry.EndHour = 13;
    LunchEntry.Priority = 5;
    Schedule.Entries.Add(LunchEntry);

    // Afternoon work
    FForge2NPCScheduleEntry AfternoonEntry;
    AfternoonEntry.Activity = EForge2NPCActivity::Working;
    AfternoonEntry.StartHour = 13;
    AfternoonEntry.EndHour = 19;
    AfternoonEntry.Priority = 10;
    Schedule.Entries.Add(AfternoonEntry);

    // Evening at tavern
    FForge2NPCScheduleEntry EveningEntry;
    EveningEntry.Activity = EForge2NPCActivity::Socializing;
    EveningEntry.StartHour = 19;
    EveningEntry.EndHour = 22;
    EveningEntry.Priority = 5;
    Schedule.Entries.Add(EveningEntry);

    return Schedule;
}

FForge2NPCSchedule UForge2EnvironmentUtilities::GetGuardSchedule()
{
    FForge2NPCSchedule Schedule;
    Schedule.SleepStartHour = 23;
    Schedule.WakeHour = 6;
    Schedule.bRandomizeSchedule = false;

    // Day patrol
    FForge2NPCScheduleEntry DayPatrol;
    DayPatrol.Activity = EForge2NPCActivity::Patrolling;
    DayPatrol.StartHour = 6;
    DayPatrol.EndHour = 14;
    DayPatrol.Priority = 10;
    Schedule.Entries.Add(DayPatrol);

    // Break
    FForge2NPCScheduleEntry BreakEntry;
    BreakEntry.Activity = EForge2NPCActivity::Eating;
    BreakEntry.StartHour = 14;
    BreakEntry.EndHour = 15;
    BreakEntry.Priority = 5;
    Schedule.Entries.Add(BreakEntry);

    // Evening patrol
    FForge2NPCScheduleEntry EveningPatrol;
    EveningPatrol.Activity = EForge2NPCActivity::Patrolling;
    EveningPatrol.StartHour = 15;
    EveningPatrol.EndHour = 23;
    EveningPatrol.Priority = 10;
    Schedule.Entries.Add(EveningPatrol);

    return Schedule;
}

FForge2NPCSchedule UForge2EnvironmentUtilities::GetFarmerSchedule()
{
    FForge2NPCSchedule Schedule;
    Schedule.SleepStartHour = 21;
    Schedule.WakeHour = 5;
    Schedule.bRandomizeSchedule = true;
    Schedule.RandomizationVariance = 0.5f;

    // Early morning work
    FForge2NPCScheduleEntry MorningWork;
    MorningWork.Activity = EForge2NPCActivity::Working;
    MorningWork.StartHour = 5;
    MorningWork.EndHour = 12;
    MorningWork.Priority = 10;
    Schedule.Entries.Add(MorningWork);

    // Lunch
    FForge2NPCScheduleEntry Lunch;
    Lunch.Activity = EForge2NPCActivity::Eating;
    Lunch.StartHour = 12;
    Lunch.EndHour = 13;
    Lunch.Priority = 5;
    Schedule.Entries.Add(Lunch);

    // Afternoon work
    FForge2NPCScheduleEntry AfternoonWork;
    AfternoonWork.Activity = EForge2NPCActivity::Working;
    AfternoonWork.StartHour = 13;
    AfternoonWork.EndHour = 18;
    AfternoonWork.Priority = 10;
    Schedule.Entries.Add(AfternoonWork);

    // Evening relaxation
    FForge2NPCScheduleEntry Evening;
    Evening.Activity = EForge2NPCActivity::Relaxing;
    Evening.StartHour = 18;
    Evening.EndHour = 21;
    Evening.Priority = 5;
    Schedule.Entries.Add(Evening);

    return Schedule;
}

FForge2NPCSchedule UForge2EnvironmentUtilities::GetTavernSchedule()
{
    FForge2NPCSchedule Schedule;
    Schedule.SleepStartHour = 2;
    Schedule.WakeHour = 10;
    Schedule.bRandomizeSchedule = true;

    // Late morning prep
    FForge2NPCScheduleEntry MorningPrep;
    MorningPrep.Activity = EForge2NPCActivity::Working;
    MorningPrep.StartHour = 10;
    MorningPrep.EndHour = 12;
    MorningPrep.Priority = 10;
    Schedule.Entries.Add(MorningPrep);

    // Service hours
    FForge2NPCScheduleEntry Service;
    Service.Activity = EForge2NPCActivity::Working;
    Service.StartHour = 12;
    Service.EndHour = 2;
    Service.Priority = 10;
    Schedule.Entries.Add(Service);

    return Schedule;
}
