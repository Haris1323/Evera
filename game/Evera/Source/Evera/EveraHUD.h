// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EveraHUD.generated.h"

enum class EBuildPieceType : uint8;
class UTexture2D;

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

	/** Show/hide the backpack (inventory) panel. */
	void ToggleInventory() { bShowInventory = !bShowInventory; }

	bool IsInventoryOpen() const { return bShowInventory; }

private:
	/** Draw one stat bar with a kit icon + gold frame. */
	void DrawStatBar(float X, float Y, UTexture2D* Icon, float Value, float Max, const FLinearColor& Color);

	/** Lazily load the HUD kit icon textures. */
	void EnsureIcons();

	UPROPERTY() UTexture2D* IconHealth = nullptr;
	UPROPERTY() UTexture2D* IconHunger = nullptr;
	UPROPERTY() UTexture2D* IconThirst = nullptr;
	UPROPERTY() UTexture2D* IconEnergy = nullptr;
	UPROPERTY() UTexture2D* PanelTex = nullptr;
	UPROPERTY() UTexture2D* PanelWideTex = nullptr;
	UPROPERTY() UTexture2D* SlotTex = nullptr;
	UPROPERTY() UTexture2D* CellTex = nullptr;
	UPROPERTY() UTexture2D* CellSelTex = nullptr;
	UPROPERTY() UTexture2D* ItemWood = nullptr;
	UPROPERTY() UTexture2D* ItemStone = nullptr;
	UPROPERTY() UTexture2D* ItemAxe = nullptr;
	bool bIconsLoaded = false;

	/** Draw the centered backpack panel listing everything the player carries. */
	void DrawBackpack(class APawn* Pawn);

	/** Draw the themed build-piece palette along the bottom while in build mode. */
	void DrawBuildPalette(const class AEveraCharacter* Character);

	/** Draw a small pictogram for a build piece (so kids recognise each one). */
	void DrawPieceIcon(EBuildPieceType Type, float X, float Y, float Size, const FLinearColor& Color);

	bool bShowInventory = false;
};
