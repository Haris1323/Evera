// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "ForestSpawner.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

AForestSpawner::AForestSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AForestSpawner::BeginPlay()
{
	Super::BeginPlay();

	// CozyNature pack (professional, ready-made working materials).
	const FString CN = TEXT("/Game/CozyNature/Meshes/");

	// Trees (collide).
	ScatterProp({ CN + TEXT("Trees/SM_Tree1.SM_Tree1") }, 16, 800.f, 6000.f, 0.8f, 1.6f, true);
	ScatterProp({ CN + TEXT("Trees/SM_Tree2.SM_Tree2") }, 16, 800.f, 6000.f, 0.8f, 1.6f, true);
	ScatterProp({ CN + TEXT("Trees/SM_Tree3.SM_Tree3") }, 14, 800.f, 6000.f, 0.8f, 1.6f, true);

	// Rocks (collide).
	ScatterProp({ CN + TEXT("Rocks/SM_Rock1.SM_Rock1") }, 8, 500.f, 6000.f, 0.8f, 2.0f, true);
	ScatterProp({ CN + TEXT("Rocks/SM_Rock2.SM_Rock2") }, 8, 500.f, 6000.f, 0.8f, 2.0f, true);
	ScatterProp({ CN + TEXT("Rocks/SM_Rock3.SM_Rock3") }, 8, 500.f, 6000.f, 0.8f, 2.0f, true);

	// Ground cover (no collision).
	ScatterProp({ CN + TEXT("Foliage/SM_Bush.SM_Bush") },       30, 300.f, 6000.f, 0.8f, 1.5f, false);
	ScatterProp({ CN + TEXT("Foliage/SM_Grass1.SM_Grass1") }, 120, 250.f, 6000.f, 0.8f, 1.6f, false);
	ScatterProp({ CN + TEXT("Foliage/SM_Grass2.SM_Grass2") }, 120, 250.f, 6000.f, 0.8f, 1.6f, false);
	ScatterProp({ CN + TEXT("Foliage/SM_Flower1.SM_Flower1") }, 40, 300.f, 6000.f, 0.8f, 1.5f, false);
	ScatterProp({ CN + TEXT("Foliage/SM_Flower2.SM_Flower2") }, 40, 300.f, 6000.f, 0.8f, 1.5f, false);
}

void AForestSpawner::ScatterProp(const TArray<FString>& MeshPaths, int32 Count, float MinRadius, float MaxRadius,
	float MinScale, float MaxScale, bool bCollision)
{
	TArray<UHierarchicalInstancedStaticMeshComponent*> Hisms;
	for (const FString& Path : MeshPaths)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path);
		if (!Mesh)
		{
			continue;
		}

		UHierarchicalInstancedStaticMeshComponent* Hism = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		Hism->SetupAttachment(SceneRoot);
		Hism->SetStaticMesh(Mesh);
		Hism->SetCastShadow(true);
		if (bCollision)
		{
			Hism->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Hism->SetCollisionResponseToAllChannels(ECR_Block);
		}
		else
		{
			Hism->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		Hism->RegisterComponent();
		Hisms.Add(Hism);
	}

	if (Hisms.Num() == 0)
	{
		return;
	}

	const FVector Center = GetActorLocation();

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(MinRadius, MaxRadius);
		FVector Position = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

		FHitResult Ground;
		const FVector Start = Position + FVector(0.f, 0.f, 2000.f);
		const FVector End = Position - FVector(0.f, 0.f, 4000.f);
		FCollisionQueryParams Params(FName(TEXT("ForestGround")), false, this);
		if (GetWorld()->LineTraceSingleByChannel(Ground, Start, End, ECC_Visibility, Params))
		{
			Position.Z = Ground.ImpactPoint.Z;
		}
		else
		{
			Position.Z = Center.Z;
		}

		const float Scale = FMath::FRandRange(MinScale, MaxScale);
		const FRotator Rotation(0.f, FMath::FRandRange(0.f, 360.f), 0.f);
		const FTransform InstanceTransform(Rotation, Position, FVector(Scale));

		// Add the same transform to every part so a whole tree/rock appears together.
		for (UHierarchicalInstancedStaticMeshComponent* Hism : Hisms)
		{
			Hism->AddInstance(InstanceTransform, /*bWorldSpace=*/true);
		}
	}
}
