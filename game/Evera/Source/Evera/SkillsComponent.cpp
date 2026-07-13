// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "SkillsComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

USkillsComponent::USkillsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USkillsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USkillsComponent, Skills);
}

void USkillsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShowDebugOnScreen)
	{
		DrawDebug();
	}
}

FSkillProgress& USkillsComponent::FindOrAddSkill(ESkillType Skill)
{
	for (FSkillProgress& Progress : Skills)
	{
		if (Progress.Type == Skill)
		{
			return Progress;
		}
	}

	FSkillProgress NewProgress;
	NewProgress.Type = Skill;
	const int32 Index = Skills.Add(NewProgress);
	return Skills[Index];
}

const FSkillProgress* USkillsComponent::FindSkill(ESkillType Skill) const
{
	for (const FSkillProgress& Progress : Skills)
	{
		if (Progress.Type == Skill)
		{
			return &Progress;
		}
	}
	return nullptr;
}

void USkillsComponent::AddXP(ESkillType Skill, float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	FSkillProgress& Progress = FindOrAddSkill(Skill);
	Progress.XP += Amount;

	// Spend accumulated XP on as many levels as it covers.
	while (Progress.XP >= XPPerLevel * Progress.Level)
	{
		Progress.XP -= XPPerLevel * Progress.Level;
		Progress.Level++;
		OnSkillLevelUp.Broadcast(Skill, Progress.Level);
	}
}

int32 USkillsComponent::GetLevel(ESkillType Skill) const
{
	const FSkillProgress* Progress = FindSkill(Skill);
	return Progress ? Progress->Level : 1;
}

int32 USkillsComponent::GetGatherBonus(ESkillType Skill) const
{
	// +1 resource per level above the first.
	return FMath::Max(0, GetLevel(Skill) - 1);
}

void USkillsComponent::OnRep_Skills()
{
	// Clients could refresh a skills UI here once we have one.
}

FString USkillsComponent::SkillName(ESkillType Skill)
{
	switch (Skill)
	{
	case ESkillType::Woodcutting: return TEXT("Woodcutting");
	case ESkillType::Mining:      return TEXT("Mining");
	default:                      return TEXT("Skill");
	}
}

void USkillsComponent::DrawDebug() const
{
	if (!GEngine)
	{
		return;
	}

	// Keys 1020+ so these don't collide with survival (1001-1004) or inventory (1010+).
	int32 Key = 1020;
	for (const FSkillProgress& Progress : Skills)
	{
		const int32 Needed = FMath::RoundToInt(XPPerLevel * Progress.Level);
		const FString Msg = FString::Printf(TEXT("%-11s Lv %d  (%d/%d)"),
			*SkillName(Progress.Type), Progress.Level, FMath::RoundToInt(Progress.XP), Needed);
		GEngine->AddOnScreenDebugMessage(Key++, 0.f, FColor(180, 140, 255), Msg);
	}
}
