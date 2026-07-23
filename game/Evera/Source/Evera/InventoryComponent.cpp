// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "InventoryComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShowDebugOnScreen)
	{
		DrawDebug();
	}
}

void UInventoryComponent::AddResource(EResourceType Type, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	// Find an existing stack of this type, or add a new one.
	for (FInventoryItem& Item : Items)
	{
		if (Item.Type == Type)
		{
			Item.Count += Amount;
			OnInventoryChanged.Broadcast();
			return;
		}
	}

	FInventoryItem NewItem;
	NewItem.Type = Type;
	NewItem.Count = Amount;
	Items.Add(NewItem);

	OnInventoryChanged.Broadcast();
}

int32 UInventoryComponent::GetResourceCount(EResourceType Type) const
{
	for (const FInventoryItem& Item : Items)
	{
		if (Item.Type == Type)
		{
			return Item.Count;
		}
	}
	return 0;
}

bool UInventoryComponent::HasResource(EResourceType Type, int32 Amount) const
{
	return GetResourceCount(Type) >= Amount;
}

bool UInventoryComponent::RemoveResource(EResourceType Type, int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	for (FInventoryItem& Item : Items)
	{
		if (Item.Type == Type)
		{
			if (Item.Count < Amount)
			{
				return false;
			}
			Item.Count -= Amount;
			OnInventoryChanged.Broadcast();
			return true;
		}
	}
	return false;
}

void UInventoryComponent::OnRep_Items()
{
	// On the client: inventory changed on the server -> refresh UI.
	OnInventoryChanged.Broadcast();
}

FString UInventoryComponent::ResourceName(EResourceType Type)
{
	switch (Type)
	{
	case EResourceType::Wood:  return TEXT("Wood");
	case EResourceType::Stone: return TEXT("Stone");
	case EResourceType::Egg:   return TEXT("Egg");
	case EResourceType::Milk:  return TEXT("Milk");
	case EResourceType::Gem:   return TEXT("Gem");
	case EResourceType::Fish:  return TEXT("Fish");
	default:                   return TEXT("Resource");
	}
}

void UInventoryComponent::DrawDebug() const
{
	if (!GEngine)
	{
		return;
	}

	// Keys 1010+ so these don't collide with the survival stats (1001-1004).
	int32 Key = 1010;
	for (const FInventoryItem& Item : Items)
	{
		const FString Msg = FString::Printf(TEXT("%-7s x %d"), *ResourceName(Item.Type), Item.Count);
		GEngine->AddOnScreenDebugMessage(Key++, 0.f, FColor::Yellow, Msg);
	}
}
