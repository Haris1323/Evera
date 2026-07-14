// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "EveraMenuGameMode.h"
#include "EveraMenuPlayerController.h"

AEveraMenuGameMode::AEveraMenuGameMode()
{
	PlayerControllerClass = AEveraMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr; // the menu has no player pawn
}
