// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForestSpawner.generated.h"

/**
 *  Procedurally scatters textured forest props (trees, rocks, ferns, bushes)
 *  around itself using Hierarchical Instanced Static Meshes, turning the empty
 *  level into a forest without hand-placing anything. Spawned from code.
 *
 *  glTF trees import as several parts (bark + leaves); each prop is described by
 *  a list of its part meshes so they are scattered together at the same spots.
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
	/** Scatter Count copies of a prop (all its part-meshes) in a ring around the actor. */
	void ScatterProp(const TArray<FString>& MeshPaths, int32 Count, float MinRadius, float MaxRadius,
		float MinScale, float MaxScale, bool bCollision);

	UPROPERTY()
	USceneComponent* SceneRoot;
};
