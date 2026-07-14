// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EveraMenuPlayerController.generated.h"

/** Shows the main-menu widget and puts input into UI mode. */
UCLASS()
class EVERA_API AEveraMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
