// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "EveraTimeOfDay.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

AEveraTimeOfDay::AEveraTimeOfDay()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEveraTimeOfDay::BeginPlay()
{
	Super::BeginPlay();

	TimeOfDay = StartHour;

	// Adopt the level's existing sun; if there isn't one, make our own.
	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		Sun = *It;
		break;
	}
	if (!Sun)
	{
		Sun = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FTransform(FRotator(-45.f, SunYaw, 0.f)));
	}
	if (Sun)
	{
		// Make it drivable at runtime (the level's sun may be baked as Static). The
		// level's directional light is already the atmosphere sun light, so rotating
		// it makes the sky colour follow along by itself.
		if (USceneComponent* Root = Sun->GetRootComponent())
		{
			Root->SetMobility(EComponentMobility::Movable);
		}
	}

	// Grab the sky light so we can dim ambient bounce at night (if it's movable).
	for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
	{
		Sky = *It;
		break;
	}
	if (Sky && Sky->GetLightComponent())
	{
		DaySkyIntensity = Sky->GetLightComponent()->Intensity;
	}

	UpdateSky();
}

void AEveraTimeOfDay::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float HoursPerSecond = 24.f / FMath::Max(1.f, DayLengthMinutes * 60.f);
	TimeOfDay += HoursPerSecond * DeltaSeconds;
	while (TimeOfDay >= 24.f)
	{
		TimeOfDay -= 24.f;
	}

	UpdateSky();
}

void AEveraTimeOfDay::UpdateSky()
{
	// Sun angle around the day: 6h = rising at the horizon, 12h = overhead,
	// 18h = setting, 0h = below the world (night).
	const float SunAngleDeg = (TimeOfDay - 6.f) / 24.f * 360.f;
	const float SunAngleRad = FMath::DegreesToRadians(SunAngleDeg);

	if (Sun)
	{
		// Pitch -90 points the light straight down (noon sun overhead).
		Sun->SetActorRotation(FRotator(-SunAngleDeg, SunYaw, 0.f));

		// How high the sun is: 1 at noon, 0 at the horizon, negative at night.
		const float SunHeight = FMath::Sin(SunAngleRad);
		const float DayFactor = FMath::Clamp(SunHeight, 0.f, 1.f);

		if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
		{
			LC->SetIntensity(FMath::Lerp(NightBrightness, DayBrightness, DayFactor));
			// Warm orange low on the horizon, clean white high in the sky.
			const FLinearColor Warm(1.0f, 0.62f, 0.32f);
			const FLinearColor Noon(1.0f, 0.98f, 0.95f);
			LC->SetLightColor(FMath::Lerp(Warm, Noon, DayFactor));
		}
	}

	if (Sky && Sky->GetLightComponent())
	{
		// Keep a soft floor so night is cosy-dim, never pitch black for kids.
		const float DayFactor = FMath::Clamp(FMath::Sin(SunAngleRad), 0.f, 1.f);
		Sky->GetLightComponent()->SetIntensity(FMath::Lerp(DaySkyIntensity * 0.28f, DaySkyIntensity, DayFactor));
	}
}

FString AEveraTimeOfDay::GetTimeString() const
{
	const int32 H = FMath::FloorToInt(TimeOfDay);
	const int32 M = FMath::FloorToInt((TimeOfDay - H) * 60.f);
	return FString::Printf(TEXT("%02d:%02d"), H, M);
}

bool AEveraTimeOfDay::IsNight() const
{
	return TimeOfDay < 6.f || TimeOfDay > 20.f;
}

void AEveraTimeOfDay::SkipToMorning()
{
	TimeOfDay = 7.f;
	UpdateSky();
}
