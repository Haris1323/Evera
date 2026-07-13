// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EveraHUD.generated.h"

/**
 *  On-screen HUD drawn entirely in C++ via the Canvas: survival bars
 *  (health, hunger, thirst, energy), inventory counts and skill levels.
 *  Replaces the temporary debug text on the components.
 */
UCLASS()
class EVERA_API AEveraHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	/** Draw one labelled stat bar and return the next Y position. */
	void DrawStatBar(float X, float Y, const FString& Label, float Value, float Max, const FLinearColor& Color);
};
