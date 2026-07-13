// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceTypes.h"
#include "ResourceNode.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UBoxComponent;
class UStaticMesh;
class USkeletalMesh;

/**
 *  A harvestable object in the world (a tree, a rock, ...).
 *  The player looks at it and interacts to gather resources into their inventory.
 *  Depletes as it is harvested and respawns after a delay. Server-authoritative.
 *
 *  Visuals: stone nodes use a static rock mesh; wood nodes use a skeletal tree mesh
 *  with an invisible box collider so the look-to-interact trace is reliable. Falls
 *  back to a coloured cube if the art assets are missing.
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

	/** Static visual: the rock mesh (stone) or a fallback cube. Also the root. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Resource")
	UStaticMeshComponent* Mesh;

	/** Skeletal visual: the tree mesh (wood). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Resource")
	USkeletalMeshComponent* TreeMesh;

	/** Invisible collider used for the look-to-interact trace on tree nodes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Resource")
	UBoxComponent* Collider;

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

	/** Random tree scale range (real Megascans trees are ~15 m tall at 1.0). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource|Visual", meta=(ClampMin="0.01"))
	float TreeScaleMin = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource|Visual", meta=(ClampMin="0.01"))
	float TreeScaleMax = 0.35f;

	/** Random stone scale range (variety: small rocks to big boulders). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource|Visual", meta=(ClampMin="0.01"))
	float StoneScaleMin = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resource|Visual", meta=(ClampMin="0.01"))
	float StoneScaleMax = 0.8f;

	/** Amount still remaining (replicated for UI/feedback). */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Resource")
	int32 RemainingAmount = 0;

private:
	/** Configure which visual and collider are active based on the resource type. */
	void ApplyVisualsForType();

	/** Toggle the depleted state (hides everything + disables collision). */
	void SetDepleted(bool bDepleted);

	/** Refill and reactivate the node. */
	void Respawn();

	FTimerHandle RespawnTimer;
};
