// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceTypes.h"
#include "ResourceNode.generated.h"

class UStaticMeshComponent;

/**
 *  A harvestable object in the world (a tree, a rock, ...).
 *  The player looks at it and interacts to gather resources into their inventory.
 *  Depletes as it is harvested and respawns after a delay. Server-authoritative.
 */
UCLASS()
class EVERA_API AResourceNode : public AActor
{
	GENERATED_BODY()

public:
	AResourceNode();

	/**
	 *  Harvest one step from this node and grant resources to the gatherer's
	 *  inventory. Runs on the server only. Returns the amount actually granted.
	 */
	int32 Gather(AActor* Gatherer);

	/** Set which resource this node yields (used when spawning nodes from code). */
	UFUNCTION(BlueprintCallable, Category="Resource")
	void SetResourceType(EResourceType NewType) { ResourceType = NewType; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Visual mesh (defaults to a simple cube so it is visible without setup). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Resource")
	UStaticMeshComponent* Mesh;

	/** Which resource this node yields. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource")
	EResourceType ResourceType = EResourceType::Wood;

	/** Total amount available before the node is depleted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource", meta=(ClampMin="1"))
	int32 TotalAmount = 20;

	/** Amount granted per single gather interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource", meta=(ClampMin="1"))
	int32 AmountPerGather = 5;

	/** Seconds until a depleted node comes back. Set to 0 to never respawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource", meta=(ClampMin="0.0"))
	float RespawnSeconds = 15.f;

	/** Amount still remaining (replicated for UI/feedback). */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Resource")
	int32 RemainingAmount = 0;

private:
	/** Toggle the depleted state (hides mesh + disables collision). */
	void SetDepleted(bool bDepleted);

	/** Refill and reactivate the node. */
	void Respawn();

	FTimerHandle RespawnTimer;
};
