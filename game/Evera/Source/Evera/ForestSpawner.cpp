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

	// Pro trees (modeled in metres ~2.7 m tall; scatter a bit larger for a forest).
	ScatterMesh(TEXT("/Game/Evera/Nature/SM_EVERA_Oak_Pro/StaticMeshes/SM_EVERA_Oak_Pro.SM_EVERA_Oak_Pro"),         16, 700.f, 6000.f, 1.4f, 2.4f, true);
	ScatterMesh(TEXT("/Game/Evera/Nature/SM_EVERA_Pine_Pro/StaticMeshes/SM_EVERA_Pine_Pro.SM_EVERA_Pine_Pro"),       18, 700.f, 6000.f, 1.4f, 2.4f, true);
	ScatterMesh(TEXT("/Game/Evera/Nature/SM_EVERA_Birch_Pro/StaticMeshes/SM_EVERA_Birch_Pro.SM_EVERA_Birch_Pro"),     14, 700.f, 6000.f, 1.4f, 2.4f, true);
	ScatterMesh(TEXT("/Game/Evera/Nature/SM_EVERA_AppleTree_Pro/StaticMeshes/SM_EVERA_AppleTree_Pro.SM_EVERA_AppleTree_Pro"), 10, 900.f, 6000.f, 1.3f, 2.0f, true);

	// Rocks (earlier nature pack; scatter larger).
	ScatterMesh(TEXT("/Game/Evera/Nature/SM_EVERA_Rock_01/StaticMeshes/SM_EVERA_Rock_01.SM_EVERA_Rock_01"), 8, 600.f, 6000.f, 2.0f, 5.0f, true);
	ScatterMesh(TEXT("/Game/Evera/Nature/SM_EVERA_Rock_02/StaticMeshes/SM_EVERA_Rock_02.SM_EVERA_Rock_02"), 8, 600.f, 6000.f, 2.0f, 5.0f, true);
	ScatterMesh(TEXT("/Game/Evera/Nature/SM_EVERA_Rock_03/StaticMeshes/SM_EVERA_Rock_03.SM_EVERA_Rock_03"), 8, 600.f, 6000.f, 2.0f, 5.0f, true);
}

void AForestSpawner::ScatterMesh(const TCHAR* MeshPath, int32 Count, float MinRadius, float MaxRadius,
	float MinScale, float MaxScale, bool bCollision)
{
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
	if (!Mesh)
	{
		return;
	}

	UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	HISM->SetupAttachment(SceneRoot);
	HISM->SetStaticMesh(Mesh);
	HISM->SetCastShadow(true);
	if (bCollision)
	{
		HISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HISM->SetCollisionResponseToAllChannels(ECR_Block);
	}
	else
	{
		HISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	HISM->RegisterComponent();

	const FVector Center = GetActorLocation();

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(MinRadius, MaxRadius);
		FVector Position = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

		// Drop onto the ground.
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
		HISM->AddInstance(InstanceTransform, /*bWorldSpace=*/true);
	}
}
