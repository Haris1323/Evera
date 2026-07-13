// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForestSpawner.generated.h"

/**
 *  Procedurally scatters nature meshes (trees, rocks, grass, plants) around
 *  itself using Hierarchical Instanced Static Meshes, turning the empty level
 *  into a forest without hand-placing anything. Spawned from code at runtime.
 */
UCLASS()
class EVERA_API AForestSpawner : public AActor
{
	GENERATED_BODY()

public:
	AForestSpawner();

protected:
	virtual void BeginPlay() override;

private:
	/** Load a mesh and scatter Count instances in a ring around the actor. */
	void ScatterMesh(const TCHAR* MeshPath, int32 Count, float MinRadius, float MaxRadius,
		float MinScale, float MaxScale, bool bCollision);

	UPROPERTY()
	USceneComponent* SceneRoot;
};
