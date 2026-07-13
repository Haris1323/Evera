// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "ResourceNode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "InventoryComponent.h"
#include "SkillsComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// Default to a basic engine cube so the node is visible without any setup.
	// Designers can swap this for a tree/rock mesh later.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	// Block the visibility channel so the player's look-trace can hit it.
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AResourceNode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		RemainingAmount = TotalAmount;
	}

	// Give wood and stone slightly different shapes so they're distinguishable
	// until real meshes are assigned: wood = tall (log), stone = squat (boulder).
	const FVector Scale = (ResourceType == EResourceType::Wood)
		? FVector(0.6f, 0.6f, 2.0f)
		: FVector(1.4f, 1.4f, 0.8f);
	Mesh->SetWorldScale3D(Scale);
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
