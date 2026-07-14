// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "ResourcePickup.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AResourcePickup::AResourcePickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		Mesh->SetStaticMesh(Cube.Object);
	}
}

void AResourcePickup::Setup(EResourceType InType, int32 InAmount)
{
	ResourceType = InType;
	Amount = InAmount;
	ApplyLook();
}

void AResourcePickup::BeginPlay()
{
	Super::BeginPlay();
	ApplyLook();
}

void AResourcePickup::ApplyLook()
{
	if (!Mesh)
	{
		return;
	}

	// A small, flat-ish chunk sitting on the ground.
	if (ResourceType == EResourceType::Wood)
	{
		Mesh->SetRelativeScale3D(FVector(0.5f, 0.18f, 0.18f)); // a stick/log
	}
	else
	{
		Mesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.25f)); // a lump of stone
	}

	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (UMaterialInstanceDynamic* MID = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr)
	{
		MID->SetVectorParameterValue(TEXT("Color"), ResourceType == EResourceType::Wood
			? FLinearColor(0.42f, 0.28f, 0.14f)
			: FLinearColor(0.5f, 0.5f, 0.52f));
		Mesh->SetMaterial(0, MID);
	}
}
