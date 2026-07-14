// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "EveraMenuPlayerController.h"
#include "EveraMainMenu.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AEveraMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Look through the scenic backdrop camera (an actor tagged "MenuCam"), if any.
	TArray<AActor*> Cams;
	UGameplayStatics::GetAllActorsWithTag(this, FName(TEXT("MenuCam")), Cams);
	if (Cams.Num() > 0)
	{
		SetViewTargetWithBlend(Cams[0], 0.f);
	}

	UEveraMainMenu* Menu = CreateWidget<UEveraMainMenu>(this, UEveraMainMenu::StaticClass());
	if (Menu)
	{
		Menu->AddToViewport();

		FInputModeUIOnly Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
		bShowMouseCursor = true;
	}
}
