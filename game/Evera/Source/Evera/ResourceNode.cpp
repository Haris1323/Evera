// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "ResourceNode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "InventoryComponent.h"
#include "SkillsComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

// Art asset paths. Loaded lazily at runtime (loading skeletal meshes in the
// constructor/CDO phase crashes the engine). Missing assets -> cube fallback.
namespace
{
	const TCHAR* StoneMeshPath = TEXT("/Game/Fab/Stone_-_photogrammetry/stone_photogrammetry/StaticMeshes/stone_photogrammetry.stone_photogrammetry");
	const TCHAR* TreeMeshPath  = TEXT("/Game/Megaplant_Library/Tree_Norway_Spruce/Tree_Norway_Spruce_01/Tree_Norway_Spruce_01_A.Tree_Norway_Spruce_01_A");
}

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Root static mesh: rock (stone) or a fallback cube.
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	// The cube is the only asset safe to load in the constructor.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	// Skeletal visual for tree nodes (no collision; the box handles interaction).
	TreeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TreeMesh"));
	TreeMesh->SetupAttachment(Mesh);
	TreeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TreeMesh->SetVisibility(false);

	// Invisible trunk collider for reliable look-to-interact on trees.
	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	Collider->SetupAttachment(Mesh);
	Collider->SetBoxExtent(FVector(60.f, 60.f, 300.f));
	Collider->SetRelativeLocation(FVector(0.f, 0.f, 300.f));
	Collider->SetCollisionResponseToAllChannels(ECR_Block);
	Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision); // enabled per-type in BeginPlay
}

void AResourceNode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		RemainingAmount = TotalAmount;
	}

	ApplyVisualsForType();
}

void AResourceNode::ApplyVisualsForType()
{
	if (ResourceType == EResourceType::Wood)
	{
		// Load the tree at runtime (safe, unlike in the constructor).
		if (USkeletalMesh* Tree = LoadObject<USkeletalMesh>(nullptr, TreeMeshPath))
		{
			const float Scale = FMath::FRandRange(TreeScaleMin, TreeScaleMax);
			TreeMesh->SetSkeletalMeshAsset(Tree);
			TreeMesh->SetVisibility(true);
			TreeMesh->SetRelativeScale3D(FVector(Scale));

			Mesh->SetVisibility(false);
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Size the invisible collider to roughly match the tree's height.
			const float ColliderHeight = FMath::Max(150.f, 1500.f * Scale);
			Collider->SetBoxExtent(FVector(50.f, 50.f, ColliderHeight * 0.5f));
			Collider->SetRelativeLocation(FVector(0.f, 0.f, ColliderHeight * 0.5f));
			Collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Collider->SetCollisionResponseToAllChannels(ECR_Block);
			return;
		}
	}
	else // Stone
	{
		if (UStaticMesh* Rock = LoadObject<UStaticMesh>(nullptr, StoneMeshPath))
		{
			const float Scale = FMath::FRandRange(StoneScaleMin, StoneScaleMax);
			Mesh->SetStaticMesh(Rock);
			Mesh->SetVisibility(true);
			Mesh->SetRelativeScale3D(FVector(Scale));
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Mesh->SetCollisionResponseToAllChannels(ECR_Block);

			TreeMesh->SetVisibility(false);
			Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			return;
		}
	}

	// Fallback: coloured cube distinguishing wood (tall) from stone (squat).
	const FVector Scale = (ResourceType == EResourceType::Wood)
		? FVector(0.6f, 0.6f, 2.0f)
		: FVector(1.4f, 1.4f, 0.8f);

	Mesh->SetVisibility(true);
	Mesh->SetRelativeScale3D(Scale);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	TreeMesh->SetVisibility(false);
	Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AResourceNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceNode, RemainingAmount);
}

int32 AResourceNode::Gather(AActor* Gatherer)
{
	if (!HasAuthority() || RemainingAmount <= 0 || !Gatherer)
	{
		return 0;
	}

	// The matching skill makes you gather more, and gathering trains that skill.
	const ESkillType Skill = (ResourceType == EResourceType::Wood) ? ESkillType::Woodcutting : ESkillType::Mining;
	USkillsComponent* Skills = Gatherer->FindComponentByClass<USkillsComponent>();
	const int32 Bonus = Skills ? Skills->GetGatherBonus(Skill) : 0;

	const int32 Granted = FMath::Min(AmountPerGather + Bonus, RemainingAmount);

	if (UInventoryComponent* Inventory = Gatherer->FindComponentByClass<UInventoryComponent>())
	{
		Inventory->AddResource(ResourceType, Granted);
	}

	if (Skills)
	{
		Skills->AddXP(Skill, static_cast<float>(Granted));
	}

	RemainingAmount -= Granted;

	if (RemainingAmount <= 0)
	{
		SetDepleted(true);
	}

	return Granted;
}

void AResourceNode::SetDepleted(bool bDepleted)
{
	// bHidden and collision-enabled are replicated by AActor, so clients follow.
	SetActorHiddenInGame(bDepleted);
	SetActorEnableCollision(!bDepleted);

	if (bDepleted && RespawnSeconds > 0.f)
	{
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &AResourceNode::Respawn, RespawnSeconds, false);
	}
}

void AResourceNode::Respawn()
{
	if (!HasAuthority())
	{
		return;
	}

	RemainingAmount = TotalAmount;
	SetDepleted(false);
}
