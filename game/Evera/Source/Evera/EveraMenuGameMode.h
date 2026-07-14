// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EveraMenuGameMode.generated.h"

/** Game mode for the main-menu level: menu controller, no pawn. */
UCLASS()
class EVERA_API AEveraMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEveraMenuGameMode();
};
