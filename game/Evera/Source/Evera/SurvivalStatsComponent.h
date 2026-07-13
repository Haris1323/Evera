// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalStatsComponent.generated.h"

/** Broadcast whenever any stat changes (for UI, audio, gameplay reactions). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSurvivalStatsChanged, float, Health, float, Hunger, float, Thirst, float, Energy);

/** Broadcast once when health reaches zero. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSurvivalDeath);

/**
 *  Tracks the character's basic needs: health, hunger, thirst, energy (0..MaxValue).
 *  Values decay over time. When hunger OR thirst hits zero, health starts to drop.
 *  While hunger and thirst are above a threshold, health slowly regenerates.
 *  Server-authoritative (multiplayer-ready from the start).
 */
UCLASS(ClassGroup=(Evera), meta=(BlueprintSpawnableComponent))
class EVERA_API USurvivalStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivalStatsComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ---- Actions (called by other systems: eating, drinking, resting, damage) ----

	/** Restore hunger (eating). */
	UFUNCTION(BlueprintCallable, Category="Survival")
	void ConsumeFood(float Amount);

	/** Restore thirst / hydration (drinking water). */
	UFUNCTION(BlueprintCallable, Category="Survival")
	void DrinkWater(float Amount);

	/** Restore energy (rest/sleep). */
	UFUNCTION(BlueprintCallable, Category="Survival")
	void Rest(float Amount);

	/** Restore health. */
	UFUNCTION(BlueprintCallable, Category="Survival")
	void Heal(float Amount);

	/** Reduce health (injury). */
	UFUNCTION(BlueprintCallable, Category="Survival")
	void ApplyDamage(float Amount);

	/** Whether the character is alive. */
	UFUNCTION(BlueprintPure, Category="Survival")
	bool IsAlive() const { return Health > 0.f; }

	/** Maximum value each stat can reach. */
	UFUNCTION(BlueprintPure, Category="Survival")
	float GetMaxValue() const { return MaxValue; }

	// ---- Events (for UI and gameplay) ----

	UPROPERTY(BlueprintAssignable, Category="Survival")
	FOnSurvivalStatsChanged OnStatsChanged;

	UPROPERTY(BlueprintAssignable, Category="Survival")
	FOnSurvivalDeath OnDeath;

	// ---- Current values (replicated from server to clients) ----

	UPROPERTY(ReplicatedUsing=OnRep_Stats, VisibleAnywhere, BlueprintReadOnly, Category="Survival|State")
	float Health;

	UPROPERTY(ReplicatedUsing=OnRep_Stats, VisibleAnywhere, BlueprintReadOnly, Category="Survival|State")
	float Hunger;

	UPROPERTY(ReplicatedUsing=OnRep_Stats, VisibleAnywhere, BlueprintReadOnly, Category="Survival|State")
	float Thirst;

	UPROPERTY(ReplicatedUsing=OnRep_Stats, VisibleAnywhere, BlueprintReadOnly, Category="Survival|State")
	float Energy;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called on clients when new values arrive from the server. */
	UFUNCTION()
	void OnRep_Stats();

	// ---- Configuration (tunable in the editor / Blueprint) ----

	/** Maximum value for each stat (e.g. 100). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Config", meta=(ClampMin="1.0"))
	float MaxValue = 100.f;

	/** How much hunger drops per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Config", meta=(ClampMin="0.0"))
	float HungerDecayPerSecond = 0.5f;

	/** How much thirst drops per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Config", meta=(ClampMin="0.0"))
	float ThirstDecayPerSecond = 0.75f;

	/** How much energy drops per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Config", meta=(ClampMin="0.0"))
	float EnergyDecayPerSecond = 0.35f;

	/** Health lost per second while hunger OR thirst is at zero. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Config", meta=(ClampMin="0.0"))
	float StarvingHealthLossPerSecond = 1.0f;

	/** Health regenerated per second while hunger and thirst are above the threshold. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Config", meta=(ClampMin="0.0"))
	float HealthRegenPerSecond = 0.5f;

	/** Threshold (0..1) above which hunger/thirst are considered high enough to regen health. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Config", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HealthyThreshold = 0.3f;

	/** Temporary on-screen debug readout (disable once we add a real UI). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Debug")
	bool bShowDebugOnScreen = false;

private:
	/** Decay/regeneration simulation (server only). */
	void UpdateSurvival(float DeltaTime);

	/** Broadcast OnStatsChanged and check for death. */
	void HandleStatsUpdated();

	/** Draw the stats on screen (debug). */
	void DrawDebug() const;

	bool bHasDied = false;
};
