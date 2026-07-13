// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingComponent.generated.h"

/** Items the player can craft. */
UENUM(BlueprintType)
enum class ECraftableItem : uint8
{
	StoneAxe UMETA(DisplayName = "Stone Axe")
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

	/** Try to craft a stone axe from the owner's inventory. Server-side. Returns success. */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	bool TryCraftStoneAxe();

	/** How many of a crafted item the owner has. */
	UFUNCTION(BlueprintPure, Category="Crafting")
	int32 GetCraftedCount(ECraftableItem Item) const;

	UFUNCTION(BlueprintPure, Category="Crafting")
	int32 GetStoneAxeWoodCost() const { return StoneAxeWoodCost; }

	UFUNCTION(BlueprintPure, Category="Crafting")
	int32 GetStoneAxeStoneCost() const { return StoneAxeStoneCost; }

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnCraftingChanged OnCraftingChanged;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_Crafted, VisibleAnywhere, BlueprintReadOnly, Category="Crafting")
	TArray<FCraftedItem> Crafted;

	UFUNCTION()
	void OnRep_Crafted();

	/** Stone axe recipe cost. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting|Recipes", meta=(ClampMin="0"))
	int32 StoneAxeWoodCost = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting|Recipes", meta=(ClampMin="0"))
	int32 StoneAxeStoneCost = 3;

private:
	void AddCrafted(ECraftableItem Item, int32 Amount);
};
