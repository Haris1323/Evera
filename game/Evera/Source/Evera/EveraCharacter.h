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
class AResourceNode;
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

	virtual void BeginPlay() override;

	/** Prototype helper: spawn a ring of gatherable nodes around the player on start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bSpawnTestResourceNodes = true;

	/** Interact with whatever the player is looking at (bound to the E key). */
	void Interact();

	/** Server RPC that performs the authoritative gather. */
	UFUNCTION(Server, Reliable)
	void ServerGather(AResourceNode* Node);

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

