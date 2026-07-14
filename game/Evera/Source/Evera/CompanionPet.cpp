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

	// Prefer a static dog mesh; fall back to a skeletal one; finally a stand-in.
	UStaticMesh* Dog = LoadObject<UStaticMesh>(nullptr, *DogMeshPath);
	USkeletalMesh* DogSkel = Dog ? nullptr : LoadObject<USkeletalMesh>(nullptr, *DogSkelMeshPath);

	if (Dog)
	{
		BodyMesh->SetStaticMesh(Dog);
	}
	else if (DogSkel)
	{
		SkelBody->SetSkeletalMeshAsset(DogSkel);
		SkelBody->SetVisibility(true);
		BodyMesh->SetVisibility(false);
	}
	else
	{
		// Visible stand-in so follow + tips can be tested before the dog is imported:
		// a small warm-brown capsule-ish shape (sphere body).
		if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
		{
			BodyMesh->SetStaticMesh(Sphere);
			BodyMesh->SetRelativeScale3D(FVector(0.7f, 0.45f, 0.5f));
			if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
			{
				if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this))
				{
					MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.42f, 0.26f, 0.13f)); // brown dog
					BodyMesh->SetMaterial(0, MID);
				}
			}
		}
	}

	// Auto-fit the dog to a small, kid-friendly size and seat her on the ground.
	if (BodyMesh->GetStaticMesh() && BodyMesh->GetStaticMesh()->GetName().StartsWith(TEXT("SM_Lea")))
	{
		const FBox Local = BodyMesh->GetStaticMesh()->GetBoundingBox();
		const float RawHeight = Local.GetSize().Z;
		if (RawHeight > 1.f)
		{
			BodyMesh->SetRelativeScale3D(FVector(TargetHeight / RawHeight));
		}
	}
	if (BodyMesh->GetStaticMesh())
	{
		const FBox Local = BodyMesh->GetStaticMesh()->GetBoundingBox();
		FootOffset = -Local.Min.Z * BodyMesh->GetRelativeScale3D().Z;
	}

	if (GetWorld())
	{
		NextTipTime = GetWorld()->GetTimeSeconds() + 2.5f; // greet shortly after spawn
	}
	GroundStick();
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
