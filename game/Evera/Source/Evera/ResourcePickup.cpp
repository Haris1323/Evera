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

	// Give each resource an unmistakable shape so kids know what they're grabbing:
	// wood is a brown log (a cylinder laid on its side), stone is a grey boulder.
	if (ResourceType == EResourceType::Wood)
	{
		if (UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
		{
			Mesh->SetStaticMesh(Cyl);
		}
		Mesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f)); // lay the log down
		Mesh->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.6f));
	}
	else if (ResourceType == EResourceType::Egg)
	{
		if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
		{
			Mesh->SetStaticMesh(Sphere);
		}
		Mesh->SetRelativeRotation(FRotator::ZeroRotator);
		Mesh->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.24f)); // a little egg
	}
	else if (ResourceType == EResourceType::Milk)
	{
		if (UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
		{
			Mesh->SetStaticMesh(Cyl);
		}
		Mesh->SetRelativeRotation(FRotator::ZeroRotator);
		Mesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.3f)); // a bottle of milk
	}
	else
	{
		if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
		{
			Mesh->SetStaticMesh(Sphere);
		}
		Mesh->SetRelativeRotation(FRotator::ZeroRotator);
		Mesh->SetRelativeScale3D(FVector(0.5f, 0.42f, 0.34f)); // a squat rock
	}

	// Our own tint material actually honours the "Color" parameter (the engine's
	// BasicShapeMaterial does not in 5.8), so wood really looks brown, stone grey.
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Evera/Materials/M_EveraTint.M_EveraTint"));
	if (!Base)
	{
		Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (UMaterialInstanceDynamic* MID = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr)
	{
		FLinearColor Tint(0.52f, 0.53f, 0.56f); // stone grey
		switch (ResourceType)
		{
		case EResourceType::Wood: Tint = FLinearColor(0.40f, 0.24f, 0.10f); break; // brown
		case EResourceType::Egg:  Tint = FLinearColor(0.96f, 0.93f, 0.82f); break; // cream
		case EResourceType::Milk: Tint = FLinearColor(0.97f, 0.97f, 1.00f); break; // white
		default: break;
		}
		MID->SetVectorParameterValue(TEXT("Color"), Tint);
		Mesh->SetMaterial(0, MID);
	}
}
