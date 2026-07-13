// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "SurvivalStatsComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

USurvivalStatsComponent::USurvivalStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// Start full (MaxValue is already initialized to 100 in the header).
	Health = MaxValue;
	Hunger = MaxValue;
	Thirst = MaxValue;
	Energy = MaxValue;
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

	const float HealthyLevel = MaxValue * HealthyThreshold;
	const bool bStarving = (Hunger <= 0.f) || (Thirst <= 0.f);

	if (bStarving)
	{
		Health = FMath::Clamp(Health - StarvingHealthLossPerSecond * DeltaTime, 0.f, MaxValue);
	}
	else if (Hunger > HealthyLevel && Thirst > HealthyLevel)
	{
		Health = FMath::Clamp(Health + HealthRegenPerSecond * DeltaTime, 0.f, MaxValue);
	}

	HandleStatsUpdated();
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
	ShowLine(1004, TEXT("Energy"), Energy, MaxValue, FColor::Green);
	ShowLine(1003, TEXT("Thirst"), Thirst, MaxValue, FColor::Cyan);
	ShowLine(1002, TEXT("Hunger"), Hunger, MaxValue, FColor::Orange);
	ShowLine(1001, TEXT("Health"), Health, MaxValue, FColor::Red);
}
