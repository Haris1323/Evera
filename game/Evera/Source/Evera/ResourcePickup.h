// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceTypes.h"
#include "ResourcePickup.generated.h"

class UStaticMeshComponent;

/**
 *  A small loose piece of a resource (a stick/log of wood, a lump of stone) lying
 *  on the ground. The player gathers it by hand with E — no tool needed — which
 *  gives the starter materials to craft the first axe/pickaxe before they can chop
 *  trees or mine rocks. Server-authoritative; destroyed when collected.
 */
UCLASS()
class EVERA_API AResourcePickup : public AActor
{
	GENERATED_BODY()

public:
	AResourcePickup();

	/** Configure which resource and how much this piece gives (call before use). */
	void Setup(EResourceType InType, int32 InAmount);

	EResourceType GetResourceType() const { return ResourceType; }
	int32 GetAmount() const { return Amount; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UStaticMeshComponent* Mesh;

	UPROPERTY()
	EResourceType ResourceType = EResourceType::Wood;

	UPROPERTY()
	int32 Amount = 2;

private:
	void ApplyLook();
};
