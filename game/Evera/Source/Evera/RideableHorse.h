// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RideableHorse.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UAnimSequence;

/**
 *  A horse the player can walk up to and ride.
 *
 *  Two visual modes, chosen automatically at begin play:
 *   - If a rigged SKELETAL horse is available (SkelMeshPath), it is used and its
 *     walk/idle animations play by movement speed — so the legs actually move.
 *   - Otherwise it falls back to the STATIC imported model (glides, no leg motion).
 *
 *  Either way the horse auto-scales to a sensible size and keeps its feet on the
 *  terrain each frame. Riding is driven by AEveraCharacter (attach to the saddle).
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

	/** How quickly the horse turns to face the rider's input. */
	float GetTurnSpeed() const { return TurnSpeed; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** The horse body when using the static (non-rigged) model. */
	UPROPERTY(VisibleAnywhere, Category="Horse")
	UStaticMeshComponent* HorseMesh;

	/** The horse body when using a rigged skeletal model (preferred). */
	UPROPERTY(VisibleAnywhere, Category="Horse")
	USkeletalMeshComponent* SkelMesh;

	/** Rider attach point, positioned above the back from the mesh bounds. */
	UPROPERTY(VisibleAnywhere, Category="Horse")
	USceneComponent* SaddlePoint;

	/** Static model path (fallback: the imported Downloads/Horse_riding model). */
	UPROPERTY(EditAnywhere, Category="Horse")
	FString HorseMeshPath = TEXT("/Game/Evera/Animals/Horse/SM_Horse.SM_Horse");

	/** Rigged skeletal model path (preferred; set when a stylized horse is added). */
	UPROPERTY(EditAnywhere, Category="Horse")
	FString SkelMeshPath = TEXT("/Game/Evera/Animals/Horse/SK_Horse.SK_Horse");

	/** Walk + idle clips for the rigged horse (played by movement, no AnimBP). */
	UPROPERTY(EditAnywhere, Category="Horse")
	FString WalkAnimPath = TEXT("/Game/Evera/Animals/Horse/A_Horse_Walk.A_Horse_Walk");

	UPROPERTY(EditAnywhere, Category="Horse")
	FString IdleAnimPath = TEXT("/Game/Evera/Animals/Horse/A_Horse_Idle.A_Horse_Idle");

	/** Yaw added to the mesh facing (flip if a rigged horse faces sideways). */
	UPROPERTY(EditAnywhere, Category="Horse")
	float MeshYawOffset = 0.f;

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

	/** Swap between walk and idle clips based on how fast the horse is moving. */
	void UpdateHorseAnim(float DeltaSeconds);

	void PlayHorseClip(UAnimSequence* Clip);

	/** True once a rigged skeletal horse is in use (enables leg animation). */
	bool bRigged = false;

	UPROPERTY() UAnimSequence* WalkAnim = nullptr;
	UPROPERTY() UAnimSequence* IdleAnim = nullptr;
	UAnimSequence* CurrentAnim = nullptr;

	FVector LastPos = FVector::ZeroVector;

	/** Distance from the actor origin down to the lowest point of the mesh (cm). */
	float FootOffset = 0.f;
};
