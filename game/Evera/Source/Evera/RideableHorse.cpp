// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "RideableHorse.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

ARideableHorse::ARideableHorse()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	HorseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HorseMesh"));
	HorseMesh->SetupAttachment(Root);
	// Block the look-trace so the player can target it, but let pawns walk right up.
	HorseMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HorseMesh->SetCollisionResponseToAllChannels(ECR_Block);
	HorseMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SaddlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("SaddlePoint"));
	SaddlePoint->SetupAttachment(Root);
}

void ARideableHorse::BeginPlay()
{
	Super::BeginPlay();

	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *HorseMeshPath))
	{
		HorseMesh->SetStaticMesh(Mesh);
	}
	else
	{
		// Fallback so the horse is still visible/rideable before the model is imported.
		if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
		{
			HorseMesh->SetStaticMesh(Cube);
			HorseMesh->SetRelativeScale3D(FVector(2.4f, 0.9f, 1.6f));
		}
	}

	// Auto-fit: scale the raw imported model so it stands ~TargetHeight tall, then
	// remember how far its feet are below the pivot so GroundStick can seat it.
	if (HorseMesh->GetStaticMesh())
	{
		const FBox Local = HorseMesh->GetStaticMesh()->GetBoundingBox();
		const float RawHeight = Local.GetSize().Z;
		if (RawHeight > 1.f && HorseMesh->GetStaticMesh()->GetName().StartsWith(TEXT("SM_Horse")))
		{
			const float Scale = TargetHeight / RawHeight;
			HorseMesh->SetRelativeScale3D(FVector(Scale));
		}
		const FVector S = HorseMesh->GetRelativeScale3D();
		FootOffset = -Local.Min.Z * S.Z;

		// Seat the rider a bit above and slightly back from the top of the body.
		const float TopZ = Local.Max.Z * S.Z;
		SaddlePoint->SetRelativeLocation(FVector(-Local.GetCenter().X * S.X, 0.f, TopZ + 12.f));
	}

	GroundStick();
}

void ARideableHorse::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Every frame, keep the feet planted on the terrain (whether idle or ridden).
	GroundStick();
}

void ARideableHorse::GroundStick()
{
	if (!GetWorld())
	{
		return;
	}

	const FVector Loc = GetActorLocation();
	FHitResult Hit;
	FCollisionQueryParams Params(FName(TEXT("HorseGround")), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Loc + FVector(0, 0, 400.f), Loc - FVector(0, 0, 4000.f), ECC_Visibility, Params))
	{
		FVector NewLoc = Loc;
		NewLoc.Z = Hit.ImpactPoint.Z + FootOffset;
		SetActorLocation(NewLoc);
	}
}
