// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ResourceTypes.generated.h"

/** Kinds of raw resources that can be gathered in the world. */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Wood  UMETA(DisplayName = "Wood"),
	Stone UMETA(DisplayName = "Stone"),
	/** Food produced by the player's tamed farm animals. */
	Egg   UMETA(DisplayName = "Egg"),
	Milk  UMETA(DisplayName = "Milk"),
	/** Treasure dug up with a shovel — the rare, exciting find. */
	Gem   UMETA(DisplayName = "Gem"),
	/** Caught with a fishing rod at the water's edge. */
	Fish  UMETA(DisplayName = "Fish"),
	/** Picked by hand from bushes — the easiest snack in the forest. */
	Berry UMETA(DisplayName = "Berry")
};

/** A single stack of a resource held in an inventory. */
USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	EResourceType Type = EResourceType::Wood;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Count = 0;
};
