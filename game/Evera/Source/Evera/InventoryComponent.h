// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceTypes.h"
#include "InventoryComponent.generated.h"

/** Broadcast whenever the inventory contents change (for UI). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 *  Simple resource inventory: stores how many of each resource the owner holds.
 *  Server-authoritative and replicated. For now it only tracks raw resources
 *  (wood, stone); it will grow into the full item system later.
 */
UCLASS(ClassGroup=(Evera), meta=(BlueprintSpawnableComponent))
class EVERA_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Add an amount of a resource. Should be called on the server. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void AddResource(EResourceType Type, int32 Amount);

	/** How many of a resource the owner currently holds. */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetResourceCount(EResourceType Type) const;

	/** Whether the inventory holds at least Amount of a resource. */
	UFUNCTION(BlueprintPure, Category="Inventory")
	bool HasResource(EResourceType Type, int32 Amount) const;

	/** Remove an amount of a resource. Returns false if there isn't enough. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveResource(EResourceType Type, int32 Amount);

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryChanged OnInventoryChanged;

	/** Temporary on-screen debug readout of the inventory (disable once we add a real UI). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory|Debug")
	bool bShowDebugOnScreen = false;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_Items, VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<FInventoryItem> Items;

	UFUNCTION()
	void OnRep_Items();

private:
	void DrawDebug() const;

	/** Human-readable name for a resource type. */
	static FString ResourceName(EResourceType Type);
};
