// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "Campfire.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "SurvivalStatsComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

ACampfire::ACampfire()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	LogsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LogsMesh"));
	LogsMesh->SetupAttachment(SceneRoot);
	LogsMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LogsMesh->SetCollisionResponseToAllChannels(ECR_Block);

	FlameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlameMesh"));
	FlameMesh->SetupAttachment(SceneRoot);
	FlameMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlameMesh->SetVisibility(false);

	FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
	FireLight->SetupAttachment(SceneRoot);
	FireLight->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	FireLight->SetLightColor(FLinearColor(1.0f, 0.55f, 0.18f));
	FireLight->SetAttenuationRadius(700.f);
	FireLight->SetIntensity(0.f);
	FireLight->SetCastShadows(false);
	FireLight->SetVisibility(false);
}

void ACampfire::BeginPlay()
{
	Super::BeginPlay();

	if (UStaticMesh* Logs = LoadObject<UStaticMesh>(nullptr, *LogsMeshPath))
	{
		LogsMesh->SetStaticMesh(Logs);
	}
	else if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		LogsMesh->SetStaticMesh(Cube);
		LogsMesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.25f));
	}

	// A little glowing flame shape above the logs (warm orange).
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		FlameMesh->SetStaticMesh(Sphere);
		FlameMesh->SetRelativeLocation(FVector(0.f, 0.f, 55.f));
		FlameMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.95f));
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Evera/Materials/M_EveraTint.M_EveraTint"));
		if (!Base)
		{
			Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		}
		if (Base)
		{
			if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this))
			{
				MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.45f, 0.10f));
				FlameMesh->SetMaterial(0, MID);
			}
		}
	}

	ApplyLitVisuals();
}

void ACampfire::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACampfire, bLit);
}

void ACampfire::ToggleLit()
{
	if (!HasAuthority())
	{
		return;
	}
	bLit = !bLit;
	ApplyLitVisuals(); // server updates itself; clients follow via OnRep_Lit
}

void ACampfire::OnRep_Lit()
{
	ApplyLitVisuals();
}

void ACampfire::ApplyLitVisuals()
{
	FlameMesh->SetVisibility(bLit);
	FireLight->SetVisibility(bLit);
	FireLight->SetIntensity(bLit ? 5000.f : 0.f);
}

void ACampfire::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bLit)
	{
		return;
	}

	// Gentle flicker so the fire feels alive (visual only, runs everywhere).
	if (GetWorld())
	{
		const float T = GetWorld()->GetTimeSeconds();
		const float Flicker = 1.f + 0.12f * FMath::Sin(T * 11.f) + 0.06f * FMath::Sin(T * 23.f);
		FireLight->SetIntensity(5000.f * Flicker);
		const float S = 0.95f + 0.06f * FMath::Sin(T * 13.f);
		FlameMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, S));
	}

	// Warmth only granted authoritatively.
	if (HasAuthority())
	{
		WarmNearbyPlayers(DeltaSeconds);
	}
}

void ACampfire::WarmNearbyPlayers(float DeltaSeconds)
{
	if (!GetWorld())
	{
		return;
	}
	const FVector FireLoc = GetActorLocation();
	for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
	{
		ACharacter* Char = *It;
		if (!Char)
		{
			continue;
		}
		if (FVector::Dist(FireLoc, Char->GetActorLocation()) > WarmthRadius)
		{
			continue;
		}
		if (USurvivalStatsComponent* Stats = Char->FindComponentByClass<USurvivalStatsComponent>())
		{
			const float Max = Stats->GetMaxValue();
			Stats->Energy = FMath::Min(Max, Stats->Energy + WarmthPerSecond * DeltaSeconds);
		}
	}
}
