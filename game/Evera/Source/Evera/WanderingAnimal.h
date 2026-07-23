// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WanderingAnimal.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UAnimSequence;
class ACharacter;

/**
 *  A gentle ambient animal that wanders its home area and stops to graze. It
 *  walks to a random nearby point, plays the grazing animation for a while, then
 *  picks a new spot. It sticks to the ground with a downward trace each frame, so
 *  it follows the terrain without needing a navmesh. Defaults to the cow assets;
 *  point the paths at another creature to reuse it. Family-friendly (no combat).
 */
UCLASS()
class EVERA_API AWanderingAnimal : public AActor
{
	GENERATED_BODY()

public:
	AWanderingAnimal();

	virtual void Tick(float DeltaSeconds) override;

	/** Point this animal at a different species (mesh + walk/graze clips + name).
	 *  Call before FinishSpawning so BeginPlay loads the right assets. */
	void ConfigureSpecies(const FString& InMesh, const FString& InWalk, const FString& InGraze,
		const FString& InName, float InYaw, float InCapsuleHalf, float InCapsuleRadius);

	/** Tame this animal so it joins the player's farm and follows them around. */
	void Tame(ACharacter* NewOwner);

	bool IsTamed() const { return bTamed; }

	/** A friendly display name for the HUD ("Cow", "Deer", ...). */
	UPROPERTY(EditAnywhere, Category="Animal")
	FString SpeciesName = TEXT("Cow");

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Animal")
	UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, Category="Animal")
	USkeletalMeshComponent* Mesh;

	// ---- Which creature (defaults to the cow) ----
	UPROPERTY(EditAnywhere, Category="Animal|Assets")
	FString MeshPath = TEXT("/Game/Realistic_Cow/Models/Cow/SK_Cow_01.SK_Cow_01");

	UPROPERTY(EditAnywhere, Category="Animal|Assets")
	FString WalkAnimPath = TEXT("/Game/Realistic_Cow/Animations/Cow/A_Cow_Walk_01.A_Cow_Walk_01");

	UPROPERTY(EditAnywhere, Category="Animal|Assets")
	FString GrazeAnimPath = TEXT("/Game/Realistic_Cow/Animations/Cow/A_Cow_Eating_01.A_Cow_Eating_01");

	// ---- Behaviour tuning ----
	UPROPERTY(EditAnywhere, Category="Animal|Behaviour")
	float WalkSpeed = 150.f;

	UPROPERTY(EditAnywhere, Category="Animal|Behaviour")
	float WanderRadius = 1800.f;

	UPROPERTY(EditAnywhere, Category="Animal|Behaviour")
	float GrazeSeconds = 5.f;

	/** Yaw added to the facing so the mesh points the right way (flip if it walks sideways). */
	UPROPERTY(EditAnywhere, Category="Animal|Behaviour")
	float MeshYawOffset = -90.f;

	UPROPERTY(EditAnywhere, Category="Animal|Behaviour")
	float CapsuleHalfHeight = 95.f;

	UPROPERTY(EditAnywhere, Category="Animal|Behaviour")
	float CapsuleRadius = 80.f;

private:
	void PickNewTarget();
	void StickToGround();
	void PlayClip(UAnimSequence* Clip);

	UPROPERTY() UAnimSequence* WalkAnim = nullptr;
	UPROPERTY() UAnimSequence* GrazeAnim = nullptr;

	FVector Home = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	bool bGrazing = false;
	float GrazeTimer = 0.f;

	/** Once tamed, the animal follows this player instead of wandering its home. */
	UPROPERTY()
	ACharacter* TamedOwner = nullptr;

	bool bTamed = false;

	/** How close a tamed animal keeps to its owner before catching up (cm). */
	UPROPERTY(EditAnywhere, Category="Animal|Behaviour")
	float FollowDistance = 450.f;

	// ---- Farm produce (tamed animals give food over time) -------------------

	/** Seconds between a tamed animal dropping produce (eggs / milk). */
	UPROPERTY(EditAnywhere, Category="Animal|Farm")
	float ProduceInterval = 40.f;

	float ProduceTimer = 0.f;

	/** Drop a piece of produce on the ground for the owner to collect. */
	void DropProduce();
};
