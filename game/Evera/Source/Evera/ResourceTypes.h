// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ResourceTypes.generated.h"

/** Kinds of raw resources that can be gathered in the world. */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Wood  UMETA(DisplayName = "Wood"),
	Stone UMETA(DisplayName = "Stone")
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
