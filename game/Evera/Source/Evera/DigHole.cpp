// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "DigHole.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

ADigHole::ADigHole()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Hole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hole"));
	Hole->SetupAttachment(Root);
	Hole->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Mound = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mound"));
	Mound->SetupAttachment(Root);
	Mound->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADigHole::BeginPlay()
{
	Super::BeginPlay();

	UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Evera/Materials/M_EveraTint.M_EveraTint"));
	if (!Base)
	{
		Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}

	if (Cyl)
	{
		Hole->SetStaticMesh(Cyl);
		Hole->SetRelativeLocation(FVector(0.f, 0.f, 2.f));
		Hole->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.04f)); // a shallow dark pit
		if (UMaterialInstanceDynamic* MID = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr)
		{
			MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.06f, 0.04f, 0.02f));
			Hole->SetMaterial(0, MID);
		}
	}

	if (Sphere)
	{
		Mound->SetStaticMesh(Sphere);
		Mound->SetRelativeLocation(FVector(55.f, 20.f, 6.f));
		Mound->SetRelativeScale3D(FVector(0.42f, 0.38f, 0.16f)); // earth piled beside it
		if (UMaterialInstanceDynamic* MID = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr)
		{
			MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.24f, 0.16f, 0.09f));
			Mound->SetMaterial(0, MID);
		}
	}
}
