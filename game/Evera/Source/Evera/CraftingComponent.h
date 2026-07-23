// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingComponent.generated.h"

/** Items the player can craft. */
UENUM(BlueprintType)
enum class ECraftableItem : uint8
{
	StoneAxe     UMETA(DisplayName = "Stone Axe"),
	StonePickaxe UMETA(DisplayName = "Stone Pickaxe"),
	/** Digs holes in the ground — sometimes there's treasure buried down there. */
	StoneShovel  UMETA(DisplayName = "Stone Shovel"),
	/** Carried light so the world stays playable after dark. */
	Torch        UMETA(DisplayName = "Torch"),
	/** Catches fish at the water's edge. */
	FishingRod   UMETA(DisplayName = "Fishing Rod")
};

/** How many of a crafted item the player owns. */
USTRUCT(BlueprintType)
struct FCraftedItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	ECraftableItem Type = ECraftableItem::StoneAxe;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	int32 Count = 0;
};

/** Broadcast when the set of crafted items changes (for UI). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCraftingChanged);

/**
 *  Turns gathered resources into crafted items using simple recipes.
 *  Server-authoritative and replicated. Starts with a single recipe (stone axe).
 */
UCLASS(ClassGroup=(Evera), meta=(BlueprintSpawnableComponent))
class EVERA_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingComponent();

	/** Craft any item from its recipe, if the owner can afford it. Server-side. */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	bool TryCraft(ECraftableItem Item);

	/** Wood/stone a recipe costs (also used by the HUD to show the price). */
	UFUNCTION(BlueprintPure, Category="Crafting")
	static int32 GetRecipeWoodCost(ECraftableItem Item);

	UFUNCTION(BlueprintPure, Category="Crafting")
	static int32 GetRecipeStoneCost(ECraftableItem Item);

	/** Short label for the HUD ("Axe", "Shovel", ...). */
	static FString GetItemName(ECraftableItem Item);

	/** Try to craft a stone axe from the owner's inventory. Server-side. Returns success. */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	bool TryCraftStoneAxe();

	/** Try to craft a stone pickaxe (for mining stone). Server-side. Returns success. */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	bool TryCraftStonePickaxe();

	UFUNCTION(BlueprintPure, Category="Crafting")
	int32 GetStonePickaxeWoodCost() const { return GetRecipeWoodCost(ECraftableItem::StonePickaxe); }

	UFUNCTION(BlueprintPure, Category="Crafting")
	int32 GetStonePickaxeStoneCost() const { return GetRecipeStoneCost(ECraftableItem::StonePickaxe); }

	/** How many of a crafted item the owner has. */
	UFUNCTION(BlueprintPure, Category="Crafting")
	int32 GetCraftedCount(ECraftableItem Item) const;

	UFUNCTION(BlueprintPure, Category="Crafting")
	int32 GetStoneAxeWoodCost() const { return GetRecipeWoodCost(ECraftableItem::StoneAxe); }

	UFUNCTION(BlueprintPure, Category="Crafting")
	int32 GetStoneAxeStoneCost() const { return GetRecipeStoneCost(ECraftableItem::StoneAxe); }

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnCraftingChanged OnCraftingChanged;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_Crafted, VisibleAnywhere, BlueprintReadOnly, Category="Crafting")
	TArray<FCraftedItem> Crafted;

	UFUNCTION()
	void OnRep_Crafted();

	// Recipe costs live in one place: GetRecipeWoodCost / GetRecipeStoneCost.

private:
	void AddCrafted(ECraftableItem Item, int32 Amount);
};
