// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "EveraCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USurvivalStatsComponent;
class UInventoryComponent;
class USkillsComponent;
class UCraftingComponent;
class UStaticMeshComponent;
class AResourceNode;
class UAnimMontage;
class UAnimSequence;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AEveraCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Tracks the character's needs: health, hunger, thirst, energy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival", meta = (AllowPrivateAccess = "true"))
	USurvivalStatsComponent* SurvivalStats;

	/** Holds resources the character has gathered */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* Inventory;

	/** Tracks skills that grow through use (woodcutting, mining, ...) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival", meta = (AllowPrivateAccess = "true"))
	USkillsComponent* Skills;

	/** Turns gathered resources into crafted items (stone axe, ...) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival", meta = (AllowPrivateAccess = "true"))
	UCraftingComponent* Crafting;

	/** The stone axe held in the right hand; shown once the player has crafted one. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* HeldAxe;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

	/** Constructor */
	AEveraCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

protected:

	/** How far in front of the camera the interact look-trace reaches (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	float InteractDistance = 800.f;

	/** Max distance from the character to a node for a gather to be valid (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	float MaxGatherDistance = 500.f;

	/** Gentle "bend down and pick up" animation, played when gathering by hand. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UAnimSequence> PickupAnim;

	/** Gentle "chop wood" animation, played when gathering with the axe. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UAnimSequence> ChopAnim;

	/** Play the appropriate gather animation (pickup by hand, or chop with axe). */
	void PlayGatherAnim();

	virtual void BeginPlay() override;

	/** Prototype helper: spawn a ring of gatherable nodes around the player on start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bSpawnTestResourceNodes = true;

	/** Interact with whatever the player is looking at (bound to the E key). */
	void Interact();

	/** Server RPC that performs the authoritative gather. */
	UFUNCTION(Server, Reliable)
	void ServerGather(AResourceNode* Node);

	/** Try to craft a stone axe (bound to the C key). */
	void CraftStoneAxe();

	/** Open/close the backpack inventory screen (bound to the I key). */
	void ToggleInventory();

	/** Hand bone/socket the axe attaches to. */
	UPROPERTY(EditAnywhere, Category="Equipment")
	FName AxeHandSocket = TEXT("hand_r");

	/** Grip transform of the axe relative to the hand socket (tune to fit). */
	UPROPERTY(EditAnywhere, Category="Equipment")
	FVector AxeGripLocation = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category="Equipment")
	FRotator AxeGripRotation = FRotator(0.f, 0.f, 180.f);

	UPROPERTY(EditAnywhere, Category="Equipment")
	float AxeGripScale = 0.01f;

	/** Show/hide the held axe based on whether the player owns one. */
	UFUNCTION()
	void UpdateHeldAxe();

	/** Server RPC that performs the authoritative craft. */
	UFUNCTION(Server, Reliable)
	void ServerCraftStoneAxe();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

