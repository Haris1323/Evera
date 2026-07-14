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

bool UCraftingComponent::TryCraftStoneAxe()
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

	// Need all inputs before consuming any.
	if (!Inventory->HasResource(EResourceType::Wood, StoneAxeWoodCost) ||
		!Inventory->HasResource(EResourceType::Stone, StoneAxeStoneCost))
	{
		return false;
	}

	Inventory->RemoveResource(EResourceType::Wood, StoneAxeWoodCost);
	Inventory->RemoveResource(EResourceType::Stone, StoneAxeStoneCost);
	AddCrafted(ECraftableItem::StoneAxe, 1);
	return true;
}

bool UCraftingComponent::TryCraftStonePickaxe()
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

	if (!Inventory->HasResource(EResourceType::Wood, StonePickaxeWoodCost) ||
		!Inventory->HasResource(EResourceType::Stone, StonePickaxeStoneCost))
	{
		return false;
	}

	Inventory->RemoveResource(EResourceType::Wood, StonePickaxeWoodCost);
	Inventory->RemoveResource(EResourceType::Stone, StonePickaxeStoneCost);
	AddCrafted(ECraftableItem::StonePickaxe, 1);
	return true;
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
