// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EveraMainMenu.generated.h"

class UEditableTextBox;
class UTextBlock;
class UButton;
class UPanelWidget;
class UTexture2D;

/**
 *  The start screen: title, a name field, four avatar choices and a Play button.
 *  Built entirely in C++ (no UMG asset). On Play it stores the name + avatar in
 *  the game instance and opens the world.
 */
UCLASS()
class EVERA_API UEveraMainMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION() void OnAvatar0();
	UFUNCTION() void OnAvatar1();
	UFUNCTION() void OnAvatar2();
	UFUNCTION() void OnAvatar3();
	UFUNCTION() void OnPlayClicked();

private:
	void BuildMenu();
	void SelectAvatar(int32 Index);
	UButton* MakeButton(UPanelWidget* Parent, const FString& Label, float FontSize, FName Name, float Width, float Height);

	UPROPERTY() UEditableTextBox* NameBox = nullptr;
	UPROPERTY() UTextBlock* AvatarLabel = nullptr;
	UPROPERTY() TArray<UButton*> AvatarButtons;

	UPROPERTY() UTexture2D* BtnNormalTex = nullptr;
	UPROPERTY() UTexture2D* BtnHoverTex = nullptr;
	UPROPERTY() UTexture2D* BtnSelectedTex = nullptr;

	int32 SelectedAvatar = 0;
};
