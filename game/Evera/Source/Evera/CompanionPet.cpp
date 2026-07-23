// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "CompanionPet.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "InventoryComponent.h"
#include "CraftingComponent.h"
#include "SkillsComponent.h"
#include "ResourceTypes.h"

ACompanionPet::ACompanionPet()
{
	PrimaryActorTick.bCanEverTick = true;

	PetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PetRoot"));
	SetRootComponent(PetRoot);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(PetRoot);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SkelBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelBody"));
	SkelBody->SetupAttachment(PetRoot);
	SkelBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Do NOT start hidden: a skeletal mesh that begins hidden gets its pose frozen
	// (this is exactly why Lea's legs weren't moving). An empty SkelBody with no
	// mesh renders nothing anyway; the box-dog branch hides it explicitly.
	SkelBody->bEnableUpdateRateOptimizations = false; // never throttle the anim rate
	SkelBody->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void ACompanionPet::BeginPlay()
{
	Super::BeginPlay();

	// Prefer a static dog mesh; fall back to a skeletal one; finally a box stand-in.
	UStaticMesh* Dog = LoadObject<UStaticMesh>(nullptr, *DogMeshPath);
	USkeletalMesh* DogSkel = Dog ? nullptr : LoadObject<USkeletalMesh>(nullptr, *DogSkelMeshPath);

	if (Dog)
	{
		BodyMesh->SetStaticMesh(Dog);
		const FBox Local = Dog->GetBoundingBox();
		const float RawHeight = Local.GetSize().Z;
		if (RawHeight > 1.f)
		{
			BodyMesh->SetRelativeScale3D(FVector(TargetHeight / RawHeight));
		}
		FootOffset = -Local.Min.Z * BodyMesh->GetRelativeScale3D().Z;
	}
	else if (DogSkel)
	{
		bRigged = true;
		SkelBody->SetSkeletalMeshAsset(DogSkel);
		SkelBody->SetRelativeRotation(FRotator(0.f, MeshYawOffset, 0.f));
		BodyMesh->SetVisibility(false);
		SkelBody->SetAnimationMode(EAnimationMode::AnimationSingleNode);

		// Auto-fit the dog to a small, kid-friendly size and seat her on the ground.
		const FBoxSphereBounds B = DogSkel->GetBounds();
		const float RawHeight = B.BoxExtent.Z * 2.f;
		if (RawHeight > 1.f)
		{
			SkelBody->SetRelativeScale3D(FVector(TargetHeight / RawHeight));
		}
		const float SZ = SkelBody->GetRelativeScale3D().Z;
		FootOffset = (B.BoxExtent.Z - B.Origin.Z) * SZ;

		WalkAnim = LoadObject<UAnimSequence>(nullptr, *DogWalkAnimPath);
		IdleAnim = LoadObject<UAnimSequence>(nullptr, *DogIdleAnimPath);
		PlayDogClip(IdleAnim);
	}
	else
	{
		// No dog model yet: build a recognisable little box-dog so follow + tips can
		// be tested. Its legs reach the ground at the actor origin, so FootOffset = 0.
		SkelBody->SetVisibility(false); // empty anyway, but keep it out of the way
		BuildStandInDog();
		FootOffset = 0.f;
	}

	if (GetWorld())
	{
		NextTipTime = GetWorld()->GetTimeSeconds() + 2.5f; // greet shortly after spawn
	}
	GroundStick();
}

void ACompanionPet::BuildStandInDog()
{
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (!Sphere)
	{
		return;
	}

	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Evera/Materials/M_EveraTint.M_EveraTint"));
	if (!Base)
	{
		Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	UMaterialInstanceDynamic* Brown = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr;
	if (Brown) { Brown->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.44f, 0.26f, 0.12f)); }
	UMaterialInstanceDynamic* Dark = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr;
	if (Dark) { Dark->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.09f, 0.06f, 0.03f)); }

	auto Part = [&](UStaticMesh* SM, FVector Loc, FRotator Rot, FVector Scale, UMaterialInterface* M)
	{
		if (!SM) { return; }
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(PetRoot);
		C->SetStaticMesh(SM);
		C->SetRelativeLocation(Loc);
		C->SetRelativeRotation(Rot);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (M) { C->SetMaterial(0, M); }
		C->RegisterComponent();
	};

	// Rounded puppy: soft sphere body + head, little cylinder legs. Dog faces +X.
	BodyMesh->SetStaticMesh(Sphere);
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, 32.f));
	BodyMesh->SetRelativeScale3D(FVector(0.60f, 0.34f, 0.36f));
	if (Brown) { BodyMesh->SetMaterial(0, Brown); }

	Part(Sphere, FVector(36.f, 0.f, 44.f), FRotator::ZeroRotator, FVector(0.34f, 0.32f, 0.32f), Brown); // head
	Part(Sphere, FVector(50.f, 0.f, 40.f), FRotator::ZeroRotator, FVector(0.17f, 0.15f, 0.14f), Dark);  // snout
	Part(Sphere, FVector(48.f, 8.f, 44.f), FRotator::ZeroRotator, FVector(0.05f, 0.05f, 0.05f), Dark);  // eye L
	Part(Sphere, FVector(48.f, -8.f, 44.f), FRotator::ZeroRotator, FVector(0.05f, 0.05f, 0.05f), Dark); // eye R
	Part(Sphere, FVector(30.f, 11.f, 58.f), FRotator::ZeroRotator, FVector(0.08f, 0.05f, 0.14f), Brown);// ear L
	Part(Sphere, FVector(30.f, -11.f, 58.f), FRotator::ZeroRotator, FVector(0.08f, 0.05f, 0.14f), Brown);// ear R
	Part(Cyl, FVector(20.f, 12.f, 12.f), FRotator::ZeroRotator, FVector(0.10f, 0.10f, 0.24f), Brown);   // leg front-L
	Part(Cyl, FVector(20.f, -12.f, 12.f), FRotator::ZeroRotator, FVector(0.10f, 0.10f, 0.24f), Brown);  // leg front-R
	Part(Cyl, FVector(-18.f, 12.f, 12.f), FRotator::ZeroRotator, FVector(0.10f, 0.10f, 0.24f), Brown);  // leg back-L
	Part(Cyl, FVector(-18.f, -12.f, 12.f), FRotator::ZeroRotator, FVector(0.10f, 0.10f, 0.24f), Brown); // leg back-R
	Part(Cyl, FVector(-34.f, 0.f, 46.f), FRotator(50.f, 0.f, 0.f), FVector(0.06f, 0.06f, 0.26f), Brown);// tail
}

void ACompanionPet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FollowOwner(DeltaSeconds);
	GroundStick();

	if (bRigged)
	{
		UpdateDogAnim(DeltaSeconds);
	}
	LastPos = GetActorLocation();

	// Time for a new tip?
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now >= NextTipTime)
	{
		CurrentTip = ChooseTip();
		TipHideTime = Now + TipDuration;
		NextTipTime = Now + TipInterval;
	}
}

void ACompanionPet::UpdateDogAnim(float DeltaSeconds)
{
	// Drive the clip from the movement STATE, not the instantaneous speed: speed
	// flickers frame to frame, and every flicker restarted the walk clip at frame 0.
	UAnimSequence* Want = bMoving ? WalkAnim : IdleAnim;
	if (Want && Want != CurrentAnim)
	{
		PlayDogClip(Want);
	}
}

void ACompanionPet::PlayDogClip(UAnimSequence* Clip)
{
	if (Clip && SkelBody && SkelBody->GetSkeletalMeshAsset())
	{
		SkelBody->PlayAnimation(Clip, /*bLooping=*/true);
		CurrentAnim = Clip;
	}
}

void ACompanionPet::FollowOwner(float DeltaSeconds)
{
	if (!OwnerPlayer)
	{
		return;
	}

	const FVector MyLoc = GetActorLocation();
	const FVector TargetLoc = OwnerPlayer->GetActorLocation();
	FVector ToOwner = TargetLoc - MyLoc;
	ToOwner.Z = 0.f;
	const float Dist = ToOwner.Size();

	// Hysteresis: she starts trotting once she falls behind FollowNear and keeps
	// going until she's back within StopDistance. Without that gap she'd start and
	// stop every few frames, restarting the walk clip so her legs never stride.
	if (!bMoving && Dist > FollowNear)
	{
		bMoving = true;
	}
	else if (bMoving && Dist < StopDistance)
	{
		bMoving = false;
	}

	if (bMoving)
	{
		const FVector Dir = ToOwner.GetSafeNormal();
		// Speed up the further behind she is, so she never gets left behind.
		const float SpeedScale = (Dist > FollowFar) ? 1.8f : 1.f;
		FVector NewLoc = MyLoc + Dir * TrotSpeed * SpeedScale * DeltaSeconds;
		SetActorLocation(NewLoc, true);

		// Face the way she's trotting.
		const FRotator Want(0.f, Dir.Rotation().Yaw, 0.f);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), Want, DeltaSeconds, 8.f));
	}
	else
	{
		// Idle: turn to look up at the player.
		FVector Look = ToOwner;
		if (!Look.IsNearlyZero())
		{
			const FRotator Want(0.f, Look.Rotation().Yaw, 0.f);
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), Want, DeltaSeconds, 4.f));
		}
	}
}

void ACompanionPet::GroundStick()
{
	if (!GetWorld())
	{
		return;
	}
	const FVector Loc = GetActorLocation();
	FHitResult Hit;
	FCollisionQueryParams Params(FName(TEXT("PetGround")), false, this);
	if (OwnerPlayer)
	{
		Params.AddIgnoredActor(OwnerPlayer);
	}
	if (GetWorld()->LineTraceSingleByChannel(Hit, Loc + FVector(0, 0, 200.f), Loc - FVector(0, 0, 3000.f), ECC_Visibility, Params))
	{
		FVector NewLoc = Loc;
		NewLoc.Z = Hit.ImpactPoint.Z + FootOffset;
		SetActorLocation(NewLoc);
	}
}

FString ACompanionPet::GetActiveTip() const
{
	if (!CurrentTip.IsEmpty() && GetWorld() && GetWorld()->GetTimeSeconds() < TipHideTime)
	{
		return CurrentTip;
	}
	return FString();
}

FString ACompanionPet::ChooseTip()
{
	if (!bSaidHello)
	{
		bSaidHello = true;
		return FString::Printf(TEXT("Hi! I'm %s, your best friend. Let's explore together!"), *PetName);
	}

	const UInventoryComponent* Inv = OwnerPlayer ? OwnerPlayer->FindComponentByClass<UInventoryComponent>() : nullptr;
	const UCraftingComponent* Craft = OwnerPlayer ? OwnerPlayer->FindComponentByClass<UCraftingComponent>() : nullptr;

	const int32 Wood = Inv ? Inv->GetResourceCount(EResourceType::Wood) : 0;
	const int32 Stone = Inv ? Inv->GetResourceCount(EResourceType::Stone) : 0;
	const int32 Axes = Craft ? Craft->GetCraftedCount(ECraftableItem::StoneAxe) : 0;
	const int32 Picks = Craft ? Craft->GetCraftedCount(ECraftableItem::StonePickaxe) : 0;

	// Guide the child through the early loop in order.
	if (Wood <= 0 && Stone <= 0)
	{
		return TEXT("Look on the ground for wood and stone — walk up and press E to pick them up!");
	}
	if (Axes <= 0)
	{
		return TEXT("You have some wood and stone. Press C to craft an axe so we can chop trees!");
	}
	if (Picks <= 0)
	{
		return TEXT("Great! Now press V to craft a pickaxe so we can mine rocks!");
	}

	// Everything unlocked: rotate gentle reminders of what else there is to do.
	static const TCHAR* General[] = {
		TEXT("Press B to start building your very own house, piece by piece!"),
		TEXT("Feeling grubby? Hop into the water to wash up and stay clean!"),
		TEXT("The more you chop and mine, the faster you get. Keep practising!"),
		TEXT("Press I to open your backpack and see everything you've collected."),
		TEXT("See the horse? Walk up to it and press F to ride together!"),
		TEXT("Look at a cow or deer and press E to tame it for your farm!"),
		TEXT("Tamed chickens lay eggs and cows give milk — pick them up off the ground!"),
		TEXT("Hungry? Press G to eat an egg. Milk helps with thirst too!"),
		TEXT("Craft a shovel with H, then press E on open ground to dig for treasure!"),
		TEXT("Sparkly gems are buried all over. Keep digging — you'll find them!"),
		TEXT("Make a rod with Y, stand by the water and press E to catch a fish!"),
		TEXT("Dark outside? Craft a torch with T and light it with L."),
		TEXT("Build a campfire, then look at it and press E to light it and warm up!"),
		TEXT("Fences (in the build menu) make a cosy pen for your farm animals."),
		TEXT("It gets dark at night — a campfire keeps things bright and warm!"),
	};
	const int32 Count = UE_ARRAY_COUNT(General);
	const FString Tip = General[GeneralTipCursor % Count];
	GeneralTipCursor++;
	return Tip;
}
