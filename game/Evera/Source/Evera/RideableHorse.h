// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RideableHorse.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/**
 *  A horse the player can walk up to and ride. The imported model is a static
 *  (non-rigged) mesh, so the horse glides rather than galloping its legs — good
 *  enough as a prototype mount; swap in a rigged horse + gallop anim later.
 *
 *  Riding is driven by AEveraCharacter: the player attaches to GetSaddlePoint()
 *  and the character moves this actor while mounted. The horse keeps its own feet
 *  on the ground every frame (GroundStick), so it follows the terrain.
 */
UCLASS()
class EVERA_API ARideableHorse : public AActor
{
	GENERATED_BODY()

public:
	ARideableHorse();

	/** Where the rider sits (above the horse's back). */
	USceneComponent* GetSaddlePoint() const { return SaddlePoint; }

	/** Forward driving speed while ridden (cm/s). */
	float GetRideSpeed() const { return RideSpeed; }

	/** How quickly the horse turns to face the rider's input (deg/s-ish factor). */
	float GetTurnSpeed() const { return TurnSpeed; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** The horse body (static mesh). */
	UPROPERTY(VisibleAnywhere, Category="Horse")
	UStaticMeshComponent* HorseMesh;

	/** Rider attach point, positioned above the back from the mesh bounds. */
	UPROPERTY(VisibleAnywhere, Category="Horse")
	USceneComponent* SaddlePoint;

	/** Content path of the horse static mesh (imported from Downloads/Horse_riding). */
	UPROPERTY(EditAnywhere, Category="Horse")
	FString HorseMeshPath = TEXT("/Game/Evera/Animals/Horse/SM_Horse.SM_Horse");

	/** The horse is auto-scaled so it stands about this tall (cm). */
	UPROPERTY(EditAnywhere, Category="Horse")
	float TargetHeight = 240.f;

	UPROPERTY(EditAnywhere, Category="Horse")
	float RideSpeed = 650.f;

	UPROPERTY(EditAnywhere, Category="Horse")
	float TurnSpeed = 4.f;

private:
	/** Keep the horse's feet on the terrain (traces down each frame). */
	void GroundStick();

	/** Distance from the actor origin down to the lowest point of the mesh (cm),
	 *  after scaling — used so the feet, not the pivot, sit on the ground. */
	float FootOffset = 0.f;
};
