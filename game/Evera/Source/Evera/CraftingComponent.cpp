// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "CraftingComponent.h"
#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCraftingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCraftingComponent, Crafted);
}

int32 UCraftingComponent::GetRecipeWoodCost(ECraftableItem Item)
{
	switch (Item)
	{
	case ECraftableItem::StoneAxe:     return 5;
	case ECraftableItem::StonePickaxe: return 3;
	case ECraftableItem::StoneShovel:  return 3;
	case ECraftableItem::Torch:        return 2;
	case ECraftableItem::FishingRod:   return 4;
	default:                           return 0;
	}
}

int32 UCraftingComponent::GetRecipeStoneCost(ECraftableItem Item)
{
	switch (Item)
	{
	case ECraftableItem::StoneAxe:     return 3;
	case ECraftableItem::StonePickaxe: return 5;
	case ECraftableItem::StoneShovel:  return 2;
	case ECraftableItem::Torch:        return 0;
	case ECraftableItem::FishingRod:   return 0;
	default:                           return 0;
	}
}

FString UCraftingComponent::GetItemName(ECraftableItem Item)
{
	switch (Item)
	{
	case ECraftableItem::StoneAxe:     return TEXT("Axe");
	case ECraftableItem::StonePickaxe: return TEXT("Pickaxe");
	case ECraftableItem::StoneShovel:  return TEXT("Shovel");
	case ECraftableItem::Torch:        return TEXT("Torch");
	case ECraftableItem::FishingRod:   return TEXT("Rod");
	default:                           return TEXT("Item");
	}
}

bool UCraftingComponent::TryCraft(ECraftableItem Item)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	UInventoryComponent* Inventory = Owner->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	const int32 WoodCost = GetRecipeWoodCost(Item);
	const int32 StoneCost = GetRecipeStoneCost(Item);

	// Need all inputs before consuming any.
	if (!Inventory->HasResource(EResourceType::Wood, WoodCost) ||
		!Inventory->HasResource(EResourceType::Stone, StoneCost))
	{
		return false;
	}

	Inventory->RemoveResource(EResourceType::Wood, WoodCost);
	Inventory->RemoveResource(EResourceType::Stone, StoneCost);
	AddCrafted(Item, 1);
	return true;
}

bool UCraftingComponent::TryCraftStoneAxe()
{
	return TryCraft(ECraftableItem::StoneAxe);
}

bool UCraftingComponent::TryCraftStonePickaxe()
{
	return TryCraft(ECraftableItem::StonePickaxe);
}

void UCraftingComponent::AddCrafted(ECraftableItem Item, int32 Amount)
{
	for (FCraftedItem& Entry : Crafted)
	{
		if (Entry.Type == Item)
		{
			Entry.Count += Amount;
			OnCraftingChanged.Broadcast();
			return;
		}
	}

	FCraftedItem NewEntry;
	NewEntry.Type = Item;
	NewEntry.Count = Amount;
	Crafted.Add(NewEntry);
	OnCraftingChanged.Broadcast();
}

int32 UCraftingComponent::GetCraftedCount(ECraftableItem Item) const
{
	for (const FCraftedItem& Entry : Crafted)
	{
		if (Entry.Type == Item)
		{
			return Entry.Count;
		}
	}
	return 0;
}

void UCraftingComponent::OnRep_Crafted()
{
	OnCraftingChanged.Broadcast();
}
