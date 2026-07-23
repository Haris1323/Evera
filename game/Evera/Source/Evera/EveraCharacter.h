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
class UInstancedStaticMeshComponent;
class AResourceNode;
class ABuildPiece;
class AResourcePickup;
class ARideableHorse;
class ACompanionPet;
class ACampfire;
class AWanderingAnimal;
class AEveraTimeOfDay;
enum class EBuildPieceType : uint8;
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

	/** Adventurer backpack worn on the back. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Backpack;

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
	virtual void Tick(float DeltaSeconds) override;
	virtual void PawnClientRestart() override;

	/** Force game input mode + hide the cursor (the menu leaves input in UI mode). */
	void EnsureGameInput();

	/** Prototype helper: spawn a ring of gatherable nodes around the player on start.
	 *  Off by default now that the world (EveraForest) ships real, level-placed
	 *  harvestable nodes. Turn on only when testing in an empty level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bSpawnTestResourceNodes = false;

	/** Interact with whatever the player is looking at (bound to the E key). */
	void Interact();

	/** Server RPC that performs the authoritative gather. */
	UFUNCTION(Server, Reliable)
	void ServerGather(AResourceNode* Node);

	/** Server RPC that harvests a single foliage tree instance (fells it, grants
	 *  wood). Lets the whole painted forest be chopped without a per-tree actor. */
	UFUNCTION(Server, Reliable)
	void ServerGatherFoliage(UInstancedStaticMeshComponent* FoliageComp, int32 InstanceIndex);

	/** Try to craft a stone axe (bound to the C key). */
	void CraftStoneAxe();

	/** Try to craft a stone pickaxe (bound to the V key). */
	void CraftStonePickaxe();

	UFUNCTION(Server, Reliable)
	void ServerCraftStonePickaxe();

	/** Server RPC: collect a loose ground pickup (no tool needed). */
	UFUNCTION(Server, Reliable)
	void ServerGatherPickup(AResourcePickup* Pickup);

	/** Per-tree remaining health for foliage (keyed by world position, so it is
	 *  stable even when instance indices shift on removal). */
	TMap<FIntVector, int32> FoliageHealth;

	/** How much "health" a foliage tree has before it is felled. */
	UPROPERTY(EditAnywhere, Category="Gathering")
	int32 TreeHealthMax = 20;

	// ---- Riding the horse ---------------------------------------------------

	/** Mount the nearest horse, or dismount if already riding (bound to the F key). */
	void MountToggle();

	/** Sit the player on a horse and hand movement over to it. */
	void Mount(ARideableHorse* Horse);

	/** Get off the horse and stand beside it. */
	void Dismount();

	/** While mounted, steer + move the horse from the player's movement input. */
	void DriveMountedHorse(float DeltaSeconds);

	UPROPERTY()
	ARideableHorse* CurrentHorse = nullptr;

	bool bMounted = false;

	/** Movement input captured this frame while mounted (consumed in Tick). */
	FVector2D MountMoveInput = FVector2D::ZeroVector;

	/** How close the player must be to a horse to climb on (cm). */
	UPROPERTY(EditAnywhere, Category="Mount")
	float MountRange = 450.f;

	// ---- Companion dog (Lea) ------------------------------------------------

	/** The player's own companion dog, spawned on begin play. */
	UPROPERTY()
	ACompanionPet* Companion = nullptr;

	// ---- Farm (tame animals) + campfire -------------------------------------

	/** Server RPC: light/extinguish a campfire the player is looking at. */
	UFUNCTION(Server, Reliable)
	void ServerToggleCampfire(ACampfire* Fire);

	/** Server RPC: tame an animal so it joins the player's farm. */
	UFUNCTION(Server, Reliable)
	void ServerTameAnimal(AWanderingAnimal* Animal);

	/** Eat some farm food to top up hunger (bound to the G key). */
	void EatFood();

	UFUNCTION(Server, Reliable)
	void ServerEatFood();

	// ---- Crafting the rest of the toolkit -----------------------------------

	void CraftShovel();
	void CraftTorch();
	void CraftRod();

	/** Server RPC: craft any recipe by index (see ECraftableItem). */
	UFUNCTION(Server, Reliable)
	void ServerCraftItem(uint8 ItemIndex);

	// ---- Digging for buried treasure ----------------------------------------

	/** Server RPC: dig the ground at a spot, maybe turning up a gem. */
	UFUNCTION(Server, Reliable)
	void ServerDig(FVector Location);

	/** Spots already dug (quantised), so the same patch can't be farmed forever. */
	TSet<FIntVector> DugSpots;

	/** Chance (0-1) that a dig turns up a gem rather than plain earth or stone. */
	UPROPERTY(EditAnywhere, Category="Digging")
	float GemChance = 0.15f;

	/** Server RPC: sleep in a bed — pass the night and wake up rested. */
	UFUNCTION(Server, Reliable)
	void ServerSleep();

	// ---- Fishing -------------------------------------------------------------

	/** Server RPC: start fishing at the water's edge. */
	UFUNCTION(Server, Reliable)
	void ServerStartFishing();

	/** Called when the line finally catches something. */
	void FinishFishing();

	UPROPERTY(EditAnywhere, Category="Fishing")
	float FishingSeconds = 4.f;

	bool bFishing = false;
	FTimerHandle FishingTimer;

	// ---- Torch (carry a light after dark) ------------------------------------

	/** Turn the crafted torch on/off (bound to the L key). */
	void ToggleTorch();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment", meta = (AllowPrivateAccess = "true"))
	class UPointLightComponent* TorchLight;

	bool bTorchOn = false;

	/** How many animals the player has tamed onto their farm. */
	int32 FarmAnimalCount = 0;

	// ---- Day / night --------------------------------------------------------

	/** The world's day/night driver, spawned once on begin play. */
	UPROPERTY()
	AEveraTimeOfDay* TimeOfDay = nullptr;

	/** Open/close the backpack inventory screen (bound to the I key). */
	void ToggleInventory();

	// ---- Building (place modular house pieces on a grid) --------------------

	/** Enter/leave build mode (bound to the B key). Spawns/destroys the preview. */
	void ToggleBuildMode();

	/** Switch to the next piece type: wall -> doorway -> window -> ... (N key). */
	void CycleBuildPiece();

	/** Toggle the palette between structure and furniture (Tab key). */
	void ToggleBuildCategory();

	/** Re-apply the current selection (structure piece or furniture) to the ghost. */
	void RefreshBuildGhost();

	/** Select a piece by palette index (number keys 1-6). */
	void SelectBuildPiece(int32 Index);
	void SelectBuild1() { SelectBuildPiece(0); }
	void SelectBuild2() { SelectBuildPiece(1); }
	void SelectBuild3() { SelectBuildPiece(2); }
	void SelectBuild4() { SelectBuildPiece(3); }
	void SelectBuild5() { SelectBuildPiece(4); }
	void SelectBuild6() { SelectBuildPiece(5); }
	void SelectBuild7() { SelectBuildPiece(6); }
	void SelectBuild8() { SelectBuildPiece(7); }
	void SelectBuild9() { SelectBuildPiece(8); }

	/** Rotate the piece to place by 90 degrees (R key). */
	void RotateBuildPiece();

	/** Raise/lower the floor level pieces are placed at (build up, storey by storey). */
	void RaiseBuildLevel();
	void LowerBuildLevel();

	/** Place the currently previewed piece, spending wood (left mouse button). */
	void PlaceBuildPiece();

	/** Remove the built piece the player is looking at, refunding some wood (X key). */
	void RemoveBuildPiece();

	/** Move/colour the placement preview each frame while in build mode. */
	void UpdateBuildGhost();

	/** Work out where the previewed piece would be placed (traced + grid-snapped). */
	bool ComputeBuildPlacement(FTransform& OutXform) const;

	/** Server RPC: authoritatively spend wood and spawn a piece. */
	UFUNCTION(Server, Reliable)
	void ServerPlaceBuildPiece(uint8 TypeIndex, FTransform Xform);

	/** Server RPC: authoritatively remove a piece and refund some wood. */
	UFUNCTION(Server, Reliable)
	void ServerRemoveBuildPiece(ABuildPiece* Piece);

	/** Server RPC: authoritatively spend wood and place a furniture prop. */
	UFUNCTION(Server, Reliable)
	void ServerPlaceFurniture(uint8 FurnitureIdx, FTransform Xform);

	/** How far in front of the camera a piece can be placed (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Build")
	float BuildReach = 900.f;

	bool bBuildMode = false;
	bool bFurnitureMode = false;
	EBuildPieceType BuildPieceType;
	int32 FurnitureIndex = 0;
	float BuildYaw = 0.f;
	int32 BuildLevel = 0;

	UPROPERTY()
	ABuildPiece* BuildGhost = nullptr;

public:
	/** Whether the player is currently in build mode (for the HUD). */
	bool IsBuildMode() const { return bBuildMode; }

	/** Whether the palette is showing furniture rather than structure (for the HUD). */
	bool IsFurnitureMode() const { return bFurnitureMode; }

	/** Which piece the player is about to place (for the HUD). */
	EBuildPieceType GetBuildPieceType() const { return BuildPieceType; }

	/** Selected furniture index (for the HUD). */
	int32 GetFurnitureIndex() const { return FurnitureIndex; }

	/** Current floor level pieces are placed at (for the HUD). */
	int32 GetBuildLevel() const { return BuildLevel; }

	/** Whether the player is currently riding a horse (for the HUD). */
	bool IsMounted() const { return bMounted; }

	/** The player's companion dog, so the HUD can show her tips. */
	ACompanionPet* GetCompanion() const { return Companion; }

	/** Number of animals tamed onto the farm (for the HUD). */
	int32 GetFarmAnimalCount() const { return FarmAnimalCount; }

	/** The day/night driver, so the HUD can show a clock. */
	AEveraTimeOfDay* GetTimeOfDay() const { return TimeOfDay; }

	/** Whether the player is waiting on a bite (for the HUD). */
	bool IsFishing() const { return bFishing; }

	/** Whether the carried torch is currently lit (for the HUD). */
	bool IsTorchOn() const { return bTorchOn; }

protected:

	/** Hand bone/socket the axe attaches to. */
	UPROPERTY(EditAnywhere, Category="Equipment")
	FName AxeHandSocket = TEXT("hand_r");

	/** Grip transform of the axe relative to the hand socket (tune to fit). */
	UPROPERTY(EditAnywhere, Category="Equipment")
	FVector AxeGripLocation = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category="Equipment")
	FRotator AxeGripRotation = FRotator(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category="Equipment")
	float AxeGripScale = 1.0f;

	/** Backpack attachment (bone/socket + transform on the back; tune to fit).
	 *  The hero skeleton has a dedicated "BackpackBone" for exactly this. */
	UPROPERTY(EditAnywhere, Category="Equipment")
	FName BackpackSocket = TEXT("BackpackBone");

	UPROPERTY(EditAnywhere, Category="Equipment")
	FVector BackpackLocation = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category="Equipment")
	FRotator BackpackRotation = FRotator(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category="Equipment")
	float BackpackScale = 1.0f;

	// ---- Hero avatar (chibi mesh + code-driven idle/walk, no AnimBP) ----

	UPROPERTY(EditAnywhere, Category="Hero")
	FString HeroMeshPath = TEXT("/Game/RPGHeroSquad/Mesh/Character/SK_TinyHeroPolyart.SK_TinyHeroPolyart");

	UPROPERTY(EditAnywhere, Category="Hero")
	FString HeroIdlePath = TEXT("/Game/RPGHeroSquad/Animation/TinyHero/Anim_Idle_Normal_TinyHero.Anim_Idle_Normal_TinyHero");

	UPROPERTY(EditAnywhere, Category="Hero")
	FString HeroWalkPath = TEXT("/Game/RPGHeroSquad/Animation/TinyHero/InPlace/Anim_MoveFWD_Normal_InPlace_TinyHero.Anim_MoveFWD_Normal_InPlace_TinyHero");

	/** Mesh transform on the capsule (tune to fit the chibi). Nudged ~2 cm below
	 *  the capsule bottom so the feet plant on the ground through the movement
	 *  component's small floor-hover, instead of floating just above it. */
	UPROPERTY(EditAnywhere, Category="Hero")
	FVector HeroMeshOffset = FVector(0.f, 0.f, -82.f);

	UPROPERTY(EditAnywhere, Category="Hero")
	FRotator HeroMeshRotation = FRotator(0.f, -90.f, 0.f);

	UPROPERTY(EditAnywhere, Category="Hero")
	float HeroMeshScale = 1.0f;

	UPROPERTY()
	UAnimSequence* HeroIdleAnim = nullptr;

	UPROPERTY()
	UAnimSequence* HeroWalkAnim = nullptr;

	UPROPERTY()
	UAnimSequence* HeroChopAnim = nullptr;

	UAnimSequence* CurrentHeroAnim = nullptr;

	/** World time until which the one-shot gather/chop animation is playing. */
	float GatherAnimEndTime = 0.f;

	/** Swap in the chibi hero mesh + load its idle/walk clips. */
	void SetupHeroAvatar();

	/** Play walk while moving, idle while still (no AnimBP needed). */
	void UpdateLocomotionAnim();

	/** Little flies that buzz around the player when hygiene gets low. */
	UPROPERTY()
	TArray<UStaticMeshComponent*> Flies;

	void UpdateFlies(float TimeSeconds);

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

