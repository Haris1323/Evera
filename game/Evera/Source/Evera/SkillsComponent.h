// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillsComponent.generated.h"

/** Skills the player improves purely by doing the related activity. No classes. */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Woodcutting UMETA(DisplayName = "Woodcutting"),
	Mining      UMETA(DisplayName = "Mining")
};

/** Progress in a single skill. */
USTRUCT(BlueprintType)
struct FSkillProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Skills")
	ESkillType Type = ESkillType::Woodcutting;

	UPROPERTY(BlueprintReadOnly, Category = "Skills")
	int32 Level = 1;

	/** XP accumulated toward the next level. */
	UPROPERTY(BlueprintReadOnly, Category = "Skills")
	float XP = 0.f;
};

/** Broadcast when a skill reaches a new level. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillLevelUp, ESkillType, Skill, int32, NewLevel);

/**
 *  Tracks per-activity skills that grow through use (the "become what you do"
 *  pillar). Higher levels grant small bonuses. Server-authoritative and replicated.
 */
UCLASS(ClassGroup=(Evera), meta=(BlueprintSpawnableComponent))
class EVERA_API USkillsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillsComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Grant XP to a skill. Should be called on the server. */
	UFUNCTION(BlueprintCallable, Category="Skills")
	void AddXP(ESkillType Skill, float Amount);

	/** Current level of a skill (1 if never trained). */
	UFUNCTION(BlueprintPure, Category="Skills")
	int32 GetLevel(ESkillType Skill) const;

	/** Extra resources granted per gather thanks to this skill (0 at level 1). */
	UFUNCTION(BlueprintPure, Category="Skills")
	int32 GetGatherBonus(ESkillType Skill) const;

	UPROPERTY(BlueprintAssignable, Category="Skills")
	FOnSkillLevelUp OnSkillLevelUp;

	/** Temporary on-screen debug readout (disable once we add a real UI). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skills|Debug")
	bool bShowDebugOnScreen = true;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_Skills, VisibleAnywhere, BlueprintReadOnly, Category="Skills")
	TArray<FSkillProgress> Skills;

	UFUNCTION()
	void OnRep_Skills();

	/** XP needed to advance from level L to L+1 equals XPPerLevel * L. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skills|Config", meta=(ClampMin="1.0"))
	float XPPerLevel = 50.f;

private:
	FSkillProgress& FindOrAddSkill(ESkillType Skill);
	const FSkillProgress* FindSkill(ESkillType Skill) const;

	void DrawDebug() const;
	static FString SkillName(ESkillType Skill);
};
