// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "WanderingAnimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

AWanderingAnimal::AWanderingAnimal()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(Capsule);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionResponseToAllChannels(ECR_Block);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWanderingAnimal::BeginPlay()
{
	Super::BeginPlay();

	Capsule->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	// Drop the mesh so its feet sit at the bottom of the capsule.
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, -CapsuleHalfHeight));

	if (USkeletalMesh* SkelMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
	{
		Mesh->SetSkeletalMeshAsset(SkelMesh);
	}
	WalkAnim = LoadObject<UAnimSequence>(nullptr, *WalkAnimPath);
	GrazeAnim = LoadObject<UAnimSequence>(nullptr, *GrazeAnimPath);

	Home = GetActorLocation();
	StickToGround();
	Home = GetActorLocation();

	PickNewTarget();
	PlayClip(WalkAnim);
}

void AWanderingAnimal::PickNewTarget()
{
	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(WanderRadius * 0.25f, WanderRadius);
	Target = Home + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
}

void AWanderingAnimal::StickToGround()
{
	const FVector Loc = GetActorLocation();
	FHitResult Hit;
	const FVector Start(Loc.X, Loc.Y, Loc.Z + 400.f);
	const FVector End(Loc.X, Loc.Y, Loc.Z - 2000.f);
	FCollisionQueryParams Params(FName(TEXT("AnimalGround")), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		SetActorLocation(FVector(Loc.X, Loc.Y, Hit.ImpactPoint.Z + CapsuleHalfHeight), false);
	}
}

void AWanderingAnimal::PlayClip(UAnimSequence* Clip)
{
	if (Clip && Mesh && Mesh->GetSkeletalMeshAsset())
	{
		Mesh->PlayAnimation(Clip, /*bLooping=*/true);
	}
}

void AWanderingAnimal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	if (bGrazing)
	{
		GrazeTimer -= DeltaSeconds;
		if (GrazeTimer <= 0.f)
		{
			bGrazing = false;
			PickNewTarget();
			PlayClip(WalkAnim);
		}
		StickToGround();
		return;
	}

	const FVector Loc = GetActorLocation();
	FVector ToTarget = Target - Loc;
	ToTarget.Z = 0.f;
	const float Dist = ToTarget.Size();

	if (Dist < 150.f)
	{
		// Arrived: stop and graze for a while.
		bGrazing = true;
		GrazeTimer = GrazeSeconds;
		PlayClip(GrazeAnim);
		return;
	}

	const FVector Dir = ToTarget / Dist;
	SetActorLocation(Loc + Dir * WalkSpeed * DeltaSeconds, false);

	FRotator Facing = Dir.Rotation();
	Facing.Pitch = 0.f;
	Facing.Roll = 0.f;
	Facing.Yaw += MeshYawOffset;
	SetActorRotation(Facing);

	StickToGround();
}
