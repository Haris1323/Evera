// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CompanionPet.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class USceneComponent;
class ACharacter;

/**
 *  Lea — the player's companion dog. Every player gets their own Lea: she trots
 *  after the avatar and pops up friendly, context-aware tips telling the child
 *  what to try next (pick up wood, craft an axe, build a house, ride the horse).
 *
 *  Self-contained AI (no navmesh): follows the owner each Tick and ground-sticks
 *  to the terrain, exactly like AWanderingAnimal. The dog model can be a static or
 *  skeletal mesh; until one is imported she shows a small stand-in shape so the
 *  follow + tips can be tested. Swap DogMeshPath to the real dog when it lands.
 */
UCLASS()
class EVERA_API ACompanionPet : public AActor
{
	GENERATED_BODY()

public:
	ACompanionPet();

	/** Bind the pet to the player it should follow and advise. */
	void SetOwnerPlayer(ACharacter* InOwner) { OwnerPlayer = InOwner; }

	/** The tip Lea is currently saying, or empty if she's quiet right now. */
	FString GetActiveTip() const;

	/** Lea's name (shown in the tip bubble). */
	const FString& GetPetName() const { return PetName; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category="Pet")
	USceneComponent* PetRoot;

	/** Dog body when the model is a static mesh (the imported low-poly dog). */
	UPROPERTY(VisibleAnywhere, Category="Pet")
	UStaticMeshComponent* BodyMesh;

	/** Dog body when the model is a rigged skeletal mesh (optional). */
	UPROPERTY(VisibleAnywhere, Category="Pet")
	USkeletalMeshComponent* SkelBody;

	/** Content path of the dog mesh. Static mesh first; if that fails a skeletal
	 *  mesh is tried; if both fail a small stand-in shape is shown. */
	UPROPERTY(EditAnywhere, Category="Pet")
	FString DogMeshPath = TEXT("/Game/Evera/Animals/Lea/SM_Lea.SM_Lea");

	UPROPERTY(EditAnywhere, Category="Pet")
	FString DogSkelMeshPath = TEXT("/Game/Evera/Animals/Lea/SK_Lea.SK_Lea");

	/** The dog is auto-scaled to about this tall (cm) — a small, kid-friendly pup. */
	UPROPERTY(EditAnywhere, Category="Pet")
	float TargetHeight = 55.f;

	UPROPERTY(EditAnywhere, Category="Pet")
	FString PetName = TEXT("Lea");

	/** Stay at least this close, but start catching up past FollowFar. */
	UPROPERTY(EditAnywhere, Category="Pet")
	float FollowNear = 220.f;

	UPROPERTY(EditAnywhere, Category="Pet")
	float FollowFar = 900.f;

	UPROPERTY(EditAnywhere, Category="Pet")
	float TrotSpeed = 520.f;

	/** Seconds between Lea's tips. */
	UPROPERTY(EditAnywhere, Category="Pet")
	float TipInterval = 15.f;

	/** How long each tip stays on screen. */
	UPROPERTY(EditAnywhere, Category="Pet")
	float TipDuration = 8.f;

private:
	UPROPERTY()
	ACharacter* OwnerPlayer = nullptr;

	/** Pick the most useful tip for the player's current progress. */
	FString ChooseTip();

	void FollowOwner(float DeltaSeconds);
	void GroundStick();

	float FootOffset = 0.f;
	float NextTipTime = 0.f;
	float TipHideTime = 0.f;
	FString CurrentTip;
	int32 GeneralTipCursor = 0;
	bool bSaidHello = false;
};
