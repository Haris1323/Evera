// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "CompanionPet.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
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
	SkelBody->SetVisibility(false);
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
		SkelBody->SetSkeletalMeshAsset(DogSkel);
		SkelBody->SetVisibility(true);
		BodyMesh->SetVisibility(false);
		FootOffset = 0.f;
	}
	else
	{
		// No dog model yet: build a recognisable little box-dog so follow + tips can
		// be tested. Its legs reach the ground at the actor origin, so FootOffset = 0.
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
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!Cube)
	{
		return;
	}

	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Evera/Materials/M_EveraTint.M_EveraTint"));
	if (!Base)
	{
		Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	UMaterialInstanceDynamic* Brown = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr;
	if (Brown) { Brown->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.40f, 0.24f, 0.11f)); }
	UMaterialInstanceDynamic* Dark = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr;
	if (Dark) { Dark->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.10f, 0.07f, 0.04f)); }

	auto Part = [&](FVector Loc, FVector Scale, UMaterialInterface* M)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(PetRoot);
		C->SetStaticMesh(Cube);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (M) { C->SetMaterial(0, M); }
		C->RegisterComponent();
	};

	// Torso reuses BodyMesh; everything else is an extra box. Dog faces +X.
	BodyMesh->SetStaticMesh(Cube);
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
	BodyMesh->SetRelativeScale3D(FVector(0.52f, 0.28f, 0.28f));
	if (Brown) { BodyMesh->SetMaterial(0, Brown); }

	Part(FVector(34.f, 0.f, 40.f), FVector(0.26f, 0.24f, 0.24f), Brown);   // head
	Part(FVector(48.f, 0.f, 36.f), FVector(0.16f, 0.13f, 0.11f), Dark);    // snout
	Part(FVector(30.f, 9.f, 54.f), FVector(0.06f, 0.05f, 0.12f), Brown);   // ear L
	Part(FVector(30.f, -9.f, 54.f), FVector(0.06f, 0.05f, 0.12f), Brown);  // ear R
	Part(FVector(18.f, 10.f, 13.f), FVector(0.09f, 0.09f, 0.28f), Brown);  // leg front-L
	Part(FVector(18.f, -10.f, 13.f), FVector(0.09f, 0.09f, 0.28f), Brown); // leg front-R
	Part(FVector(-18.f, 10.f, 13.f), FVector(0.09f, 0.09f, 0.28f), Brown); // leg back-L
	Part(FVector(-18.f, -10.f, 13.f), FVector(0.09f, 0.09f, 0.28f), Brown);// leg back-R
	Part(FVector(-32.f, 0.f, 44.f), FVector(0.24f, 0.06f, 0.06f), Brown);  // tail
}

void ACompanionPet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FollowOwner(DeltaSeconds);
	GroundStick();

	// Time for a new tip?
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now >= NextTipTime)
	{
		CurrentTip = ChooseTip();
		TipHideTime = Now + TipDuration;
		NextTipTime = Now + TipInterval;
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

	// Catch up when the player wanders off; hold position when nicely close.
	if (Dist > FollowNear)
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
		TEXT("Build a campfire, then look at it and press E to light it and warm up!"),
		TEXT("Fences (in the build menu) make a cosy pen for your farm animals."),
		TEXT("It gets dark at night — a campfire keeps things bright and warm!"),
	};
	const int32 Count = UE_ARRAY_COUNT(General);
	const FString Tip = General[GeneralTipCursor % Count];
	GeneralTipCursor++;
	return Tip;
}
