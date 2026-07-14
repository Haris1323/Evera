// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EveraTimeOfDay.generated.h"

class ADirectionalLight;
class ASkyLight;

/**
 *  Drives a gentle day/night cycle: it rotates the level's sun (a directional
 *  light) through the sky over a configurable day length, dims and warms the
 *  light at dawn/dusk, and keeps a soft, never-pitch-black night so it stays
 *  friendly for young players. Exposes the current time so the HUD can show a
 *  clock. One instance is spawned by the player character on begin play.
 */
UCLASS()
class EVERA_API AEveraTimeOfDay : public AActor
{
	GENERATED_BODY()

public:
	AEveraTimeOfDay();

	virtual void Tick(float DeltaSeconds) override;

	/** Current time of day in hours [0, 24). */
	float GetTimeOfDay() const { return TimeOfDay; }

	/** "HH:MM" for the HUD clock. */
	FString GetTimeString() const;

	/** True while the sun is below the horizon (used for a moon/night look). */
	bool IsNight() const;

protected:
	virtual void BeginPlay() override;

	/** Hour the world starts at (a bright morning by default). */
	UPROPERTY(EditAnywhere, Category="Time", meta=(ClampMin="0.0", ClampMax="24.0"))
	float StartHour = 8.f;

	/** Real minutes for one full 24-hour day. */
	UPROPERTY(EditAnywhere, Category="Time", meta=(ClampMin="1.0"))
	float DayLengthMinutes = 20.f;

	/** Sun brightness at high noon and in the dead of night. */
	UPROPERTY(EditAnywhere, Category="Time")
	float DayBrightness = 6.f;

	UPROPERTY(EditAnywhere, Category="Time")
	float NightBrightness = 0.35f;

	/** Compass direction the sun tracks across (yaw, degrees). */
	UPROPERTY(EditAnywhere, Category="Time")
	float SunYaw = 45.f;

private:
	/** Point the sun + update its colour/brightness for the current time. */
	void UpdateSky();

	UPROPERTY()
	ADirectionalLight* Sun = nullptr;

	UPROPERTY()
	ASkyLight* Sky = nullptr;

	float TimeOfDay = 8.f;
	float DaySkyIntensity = 1.f; // captured from the level's sky light at start
};
