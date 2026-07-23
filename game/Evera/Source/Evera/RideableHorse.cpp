// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "RideableHorse.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
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

	SkelMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMesh"));
	SkelMesh->SetupAttachment(Root);
	SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SkelMesh->SetCollisionResponseToAllChannels(ECR_Block);
	SkelMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	// Don't start hidden — a hidden skeletal mesh freezes its pose. Always tick the
	// pose so the legs animate; the static-model branch hides this in BeginPlay.
	SkelMesh->bEnableUpdateRateOptimizations = false;
	SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	SaddlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("SaddlePoint"));
	SaddlePoint->SetupAttachment(Root);
}

void ARideableHorse::BeginPlay()
{
	Super::BeginPlay();

	// Prefer a rigged skeletal horse (animated legs); otherwise the static model.
	USkeletalMesh* Skel = LoadObject<USkeletalMesh>(nullptr, *SkelMeshPath);
	if (Skel)
	{
		bRigged = true;
		SkelMesh->SetSkeletalMeshAsset(Skel);
		HorseMesh->SetVisibility(false);
		HorseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkelMesh->SetRelativeRotation(FRotator(0.f, MeshYawOffset, 0.f));
		SkelMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);

		const FBoxSphereBounds B = Skel->GetBounds();
		const float RawHeight = B.BoxExtent.Z * 2.f;
		if (RawHeight > 1.f)
		{
			SkelMesh->SetRelativeScale3D(FVector(TargetHeight / RawHeight));
		}
		const FVector S = SkelMesh->GetRelativeScale3D();
		// Lift so the lowest point (hooves) sits on the ground, and seat the rider
		// just above the top of the body.
		FootOffset = (B.BoxExtent.Z - B.Origin.Z) * S.Z;
		SaddlePoint->SetRelativeLocation(FVector(0.f, 0.f, (B.Origin.Z + B.BoxExtent.Z) * S.Z + 12.f));

		WalkAnim = LoadObject<UAnimSequence>(nullptr, *WalkAnimPath);
		IdleAnim = LoadObject<UAnimSequence>(nullptr, *IdleAnimPath);
		PlayHorseClip(IdleAnim);
	}
	else
	{
		SkelMesh->SetVisibility(false); // static fallback: keep the empty rig hidden
		if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *HorseMeshPath))
		{
			HorseMesh->SetStaticMesh(Mesh);
		}
		else if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
		{
			HorseMesh->SetStaticMesh(Cube);
			HorseMesh->SetRelativeScale3D(FVector(2.4f, 0.9f, 1.6f));
		}

		if (HorseMesh->GetStaticMesh())
		{
			const FBox Local = HorseMesh->GetStaticMesh()->GetBoundingBox();
			const float RawHeight = Local.GetSize().Z;
			if (RawHeight > 1.f && HorseMesh->GetStaticMesh()->GetName().StartsWith(TEXT("SM_Horse")))
			{
				HorseMesh->SetRelativeScale3D(FVector(TargetHeight / RawHeight));
			}
			const FVector S = HorseMesh->GetRelativeScale3D();
			FootOffset = -Local.Min.Z * S.Z;

			const float TopZ = Local.Max.Z * S.Z;
			SaddlePoint->SetRelativeLocation(FVector(-Local.GetCenter().X * S.X, 0.f, TopZ + 12.f));
		}
	}

	LastPos = GetActorLocation();
	GroundStick();
}

void ARideableHorse::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Keep the feet planted on the terrain (whether idle or ridden).
	GroundStick();

	if (bRigged)
	{
		UpdateHorseAnim(DeltaSeconds);
	}
	LastPos = GetActorLocation();
}

void ARideableHorse::UpdateHorseAnim(float DeltaSeconds)
{
	// Decide walk vs idle from how far the horse actually moved this frame, but hold
	// the walk for a moment after it stops — otherwise frame-to-frame speed jitter
	// restarts the clip constantly and the legs never complete a stride.
	const float Speed = (DeltaSeconds > 0.f)
		? FVector::Dist2D(GetActorLocation(), LastPos) / DeltaSeconds
		: 0.f;
	MoveHoldTime = (Speed > 15.f) ? 0.3f : FMath::Max(0.f, MoveHoldTime - DeltaSeconds);

	UAnimSequence* Want = (MoveHoldTime > 0.f) ? WalkAnim : IdleAnim;
	if (Want && Want != CurrentAnim)
	{
		PlayHorseClip(Want);
	}
}

void ARideableHorse::PlayHorseClip(UAnimSequence* Clip)
{
	if (Clip && SkelMesh && SkelMesh->GetSkeletalMeshAsset())
	{
		SkelMesh->PlayAnimation(Clip, /*bLooping=*/true);
		CurrentAnim = Clip;
	}
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
