// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EveraGameInstance.generated.h"

/**
 *  Carries the player's choices from the main menu into the game: the name they
 *  typed and which avatar they picked. Lives for the whole session, across level
 *  loads. Also owns the avatar catalogue (mesh + idle/walk clips per character).
 */
UCLASS()
class EVERA_API UEveraGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/** Name the player typed on the menu (shown in-game). */
	UPROPERTY(BlueprintReadWrite, Category="Evera")
	FString PlayerName = TEXT("Explorer");

	/** Which avatar the player chose (index into the catalogue). */
	UPROPERTY(BlueprintReadWrite, Category="Evera")
	int32 AvatarIndex = 0;

	// ---- Avatar catalogue ----
	static int32 NumAvatars();
	static FString AvatarName(int32 Index);
	static FString AvatarMeshPath(int32 Index);
	static FString AvatarIdlePath(int32 Index);
	static FString AvatarWalkPath(int32 Index);
	static FString AvatarChopPath(int32 Index);
};
