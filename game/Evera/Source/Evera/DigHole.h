// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DigHole.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/**
 *  The little hole left behind after the player digs with the shovel, plus the
 *  mound of earth beside it. Purely decorative — it marks spots that have already
 *  been dug so the world visibly records where the player has been treasure hunting.
 */
UCLASS()
class EVERA_API ADigHole : public AActor
{
	GENERATED_BODY()

public:
	ADigHole();

protected:
	virtual void BeginPlay() override;

	/** Dark, flattened disc that reads as an opening in the ground. */
	UPROPERTY(VisibleAnywhere, Category="Dig")
	UStaticMeshComponent* Hole;

	/** Loose earth piled next to the hole. */
	UPROPERTY(VisibleAnywhere, Category="Dig")
	UStaticMeshComponent* Mound;
};
