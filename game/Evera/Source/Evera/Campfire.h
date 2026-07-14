// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Campfire.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 *  A campfire the player builds and can light. Look at it and press E to set it
 *  alight (or put it out). While lit it glows with a warm, gentle flame + light
 *  and slowly restores the energy of players standing nearby — a cosy family camp,
 *  no harmful fire. Server-authoritative; the lit state replicates to everyone.
 */
UCLASS()
class EVERA_API ACampfire : public AActor
{
	GENERATED_BODY()

public:
	ACampfire();

	virtual void Tick(float DeltaSeconds) override;

	/** Light it if it's out, put it out if it's burning (server-side). */
	void ToggleLit();

	bool IsLit() const { return bLit; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, Category="Campfire")
	USceneComponent* SceneRoot;

	/** The logs / fire pit body. */
	UPROPERTY(VisibleAnywhere, Category="Campfire")
	UStaticMeshComponent* LogsMesh;

	/** A soft glowing flame shape, shown only when lit. */
	UPROPERTY(VisibleAnywhere, Category="Campfire")
	UStaticMeshComponent* FlameMesh;

	/** Warm light cast while burning. */
	UPROPERTY(VisibleAnywhere, Category="Campfire")
	UPointLightComponent* FireLight;

	/** Content path of the campfire mesh. */
	UPROPERTY(EditAnywhere, Category="Campfire")
	FString LogsMeshPath = TEXT("/Game/Medieval_Campfire/Campfire.Campfire");

	/** How far the fire's warmth reaches players (cm). */
	UPROPERTY(EditAnywhere, Category="Campfire")
	float WarmthRadius = 450.f;

	/** Energy restored per second to nearby players while lit. */
	UPROPERTY(EditAnywhere, Category="Campfire")
	float WarmthPerSecond = 4.f;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Lit)
	bool bLit = false;

	UFUNCTION()
	void OnRep_Lit();

	/** Show/hide the flame + light to match bLit. */
	void ApplyLitVisuals();

	void WarmNearbyPlayers(float DeltaSeconds);
};
