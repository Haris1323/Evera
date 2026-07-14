// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "SurvivalStatsComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

USurvivalStatsComponent::USurvivalStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// Start full (MaxValue is already initialized to 100 in the header).
	Health = MaxValue;
	Hunger = MaxValue;
	Thirst = MaxValue;
	Energy = MaxValue;
	Hygiene = MaxValue;
}

void USurvivalStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// The server sets the initial state; clients receive it via replication.
	if (GetOwnerRole() == ROLE_Authority)
	{
		Health = MaxValue;
		Hunger = MaxValue;
		Thirst = MaxValue;
		Energy = MaxValue;
		Hygiene = MaxValue;
		HandleStatsUpdated();
	}
}

void USurvivalStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USurvivalStatsComponent, Health);
	DOREPLIFETIME(USurvivalStatsComponent, Hunger);
	DOREPLIFETIME(USurvivalStatsComponent, Thirst);
	DOREPLIFETIME(USurvivalStatsComponent, Energy);
	DOREPLIFETIME(USurvivalStatsComponent, Hygiene);
}

void USurvivalStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// The server (authority) drives the needs simulation.
	if (GetOwnerRole() == ROLE_Authority)
	{
		UpdateSurvival(DeltaTime);
	}

	if (bShowDebugOnScreen)
	{
		DrawDebug();
	}
}

void USurvivalStatsComponent::UpdateSurvival(float DeltaTime)
{
	if (bHasDied)
	{
		return;
	}

	Hunger = FMath::Clamp(Hunger - HungerDecayPerSecond * DeltaTime, 0.f, MaxValue);
	Thirst = FMath::Clamp(Thirst - ThirstDecayPerSecond * DeltaTime, 0.f, MaxValue);
	Energy = FMath::Clamp(Energy - EnergyDecayPerSecond * DeltaTime, 0.f, MaxValue);
	Hygiene = FMath::Clamp(Hygiene - HygieneDecayPerSecond * DeltaTime, 0.f, MaxValue);

	// Wading into the river washes you clean and lets you drink (refills faster
	// than it drains, so the water is a reliable place to freshen up).
	if (IsOwnerInWater())
	{
		Hygiene = FMath::Clamp(Hygiene + WaterWashPerSecond * DeltaTime, 0.f, MaxValue);
		Thirst = FMath::Clamp(Thirst + WaterWashPerSecond * DeltaTime, 0.f, MaxValue);
	}

	const float HealthyLevel = MaxValue * HealthyThreshold;
	const bool bStarving = (Hunger <= 0.f) || (Thirst <= 0.f);

	if (bStarving)
	{
		Health = FMath::Clamp(Health - StarvingHealthLossPerSecond * DeltaTime, 0.f, MaxValue);
	}
	// Recover health only when well-fed, hydrated AND reasonably clean — hygiene
	// matters, but gently (a family game: being dirty just slows recovery).
	else if (Hunger > HealthyLevel && Thirst > HealthyLevel && Hygiene > HealthyLevel)
	{
		Health = FMath::Clamp(Health + HealthRegenPerSecond * DeltaTime, 0.f, MaxValue);
	}

	HandleStatsUpdated();
}

bool USurvivalStatsComponent::IsOwnerInWater()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Find the level's water body once (an actor tagged "EveraWater").
	if (!WaterActor.IsValid())
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsWithTag(Owner, FName(TEXT("EveraWater")), Found);
		if (Found.Num() == 0)
		{
			return false;
		}
		WaterActor = Found[0];
	}

	FVector WaterOrigin, WaterExtent;
	WaterActor->GetActorBounds(/*bOnlyCollidingComponents=*/false, WaterOrigin, WaterExtent);

	const FVector Loc = Owner->GetActorLocation();

	// Inside the water footprint horizontally?
	if (FMath::Abs(Loc.X - WaterOrigin.X) > WaterExtent.X ||
		FMath::Abs(Loc.Y - WaterOrigin.Y) > WaterExtent.Y)
	{
		return false;
	}

	// And low enough that the character's feet are at/under the surface.
	float FeetZ = Loc.Z;
	if (const ACharacter* OwnerChar = Cast<ACharacter>(Owner))
	{
		FeetZ = Loc.Z - OwnerChar->GetSimpleCollisionHalfHeight();
	}
	return FeetZ <= WaterOrigin.Z + 20.f && FeetZ >= WaterOrigin.Z - 400.f;
}

void USurvivalStatsComponent::HandleStatsUpdated()
{
	OnStatsChanged.Broadcast(Health, Hunger, Thirst, Energy);

	if (Health <= 0.f && !bHasDied)
	{
		bHasDied = true;
		OnDeath.Broadcast();
	}
}

void USurvivalStatsComponent::OnRep_Stats()
{
	// On the client: new values arrived from the server -> refresh UI.
	HandleStatsUpdated();
}

void USurvivalStatsComponent::ConsumeFood(float Amount)
{
	Hunger = FMath::Clamp(Hunger + Amount, 0.f, MaxValue);
	HandleStatsUpdated();
}

void USurvivalStatsComponent::DrinkWater(float Amount)
{
	Thirst = FMath::Clamp(Thirst + Amount, 0.f, MaxValue);
	HandleStatsUpdated();
}

void USurvivalStatsComponent::Rest(float Amount)
{
	Energy = FMath::Clamp(Energy + Amount, 0.f, MaxValue);
	HandleStatsUpdated();
}

void USurvivalStatsComponent::Wash(float Amount)
{
	Hygiene = FMath::Clamp(Hygiene + Amount, 0.f, MaxValue);
	HandleStatsUpdated();
}

void USurvivalStatsComponent::Heal(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.f, MaxValue);
	HandleStatsUpdated();
}

void USurvivalStatsComponent::ApplyDamage(float Amount)
{
	Health = FMath::Clamp(Health - Amount, 0.f, MaxValue);
	HandleStatsUpdated();
}

void USurvivalStatsComponent::DrawDebug() const
{
	if (!GEngine)
	{
		return;
	}

	auto ShowLine = [](int32 Key, const TCHAR* Label, float Value, float Max, const FColor& Color)
	{
		const FString Msg = FString::Printf(TEXT("%-7s %3.0f / %3.0f"), Label, Value, Max);
		GEngine->AddOnScreenDebugMessage(Key, 0.f, Color, Msg);
	};

	// Stable keys -> messages refresh in place instead of stacking up.
	ShowLine(1005, TEXT("Hygiene"), Hygiene, MaxValue, FColor(120, 200, 220));
	ShowLine(1004, TEXT("Energy"), Energy, MaxValue, FColor::Green);
	ShowLine(1003, TEXT("Thirst"), Thirst, MaxValue, FColor::Cyan);
	ShowLine(1002, TEXT("Hunger"), Hunger, MaxValue, FColor::Orange);
	ShowLine(1001, TEXT("Health"), Health, MaxValue, FColor::Red);
}
