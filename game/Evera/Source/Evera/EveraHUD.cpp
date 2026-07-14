// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "EveraHUD.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "SurvivalStatsComponent.h"
#include "InventoryComponent.h"
#include "SkillsComponent.h"
#include "CraftingComponent.h"
#include "EveraCharacter.h"
#include "CompanionPet.h"
#include "BuildPiece.h"
#include "EveraGameInstance.h"
#include "EveraUITheme.h"
#include "Engine/Texture2D.h"

void AEveraHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	EnsureIcons();

	// Ornate framed panel (UI kit) in the top-left corner.
	const float PanelX = 14.f;
	const float PanelY = 14.f;
	const float PanelW = 320.f;
	const float PanelH = 366.f;
	if (PanelTex)
	{
		DrawTexture(PanelTex, PanelX, PanelY, PanelW, PanelH, 0.f, 0.f, 1.f, 1.f);
	}
	else
	{
		DrawRect(EveraUI::A(EveraUI::Ground1, 0.86f), PanelX, PanelY, PanelW, PanelH);
		DrawRect(EveraUI::A(EveraUI::Gold, 0.6f), PanelX, PanelY, PanelW, 2.f);
	}

	// Content sits centered inside the ornate frame's dark area.
	const float X = PanelX + 64.f;
	float Y = PanelY + 52.f;

	// --- Player name ---
	if (const UEveraGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UEveraGameInstance>() : nullptr)
	{
		UFont* NameFont = GEngine ? GEngine->GetMediumFont() : nullptr;
		DrawText(GI->PlayerName, EveraUI::GoldBright, X, Y, NameFont, 1.1f);
		Y += 22.f;
	}

	// --- Survival bars (kit icons + gold-framed bars) ---
	EnsureIcons();
	if (const USurvivalStatsComponent* Stats = Pawn->FindComponentByClass<USurvivalStatsComponent>())
	{
		const float Max = Stats->GetMaxValue();
		DrawStatBar(X, Y, IconHealth, Stats->Health, Max, EveraUI::Health); Y += 28.f;
		DrawStatBar(X, Y, IconHunger, Stats->Hunger, Max, EveraUI::Hunger); Y += 28.f;
		DrawStatBar(X, Y, IconThirst, Stats->Thirst, Max, EveraUI::Thirst); Y += 28.f;
		DrawStatBar(X, Y, IconEnergy, Stats->Energy, Max, EveraUI::Energy); Y += 28.f;
		DrawStatBar(X, Y, IconThirst, Stats->Hygiene, Max, EveraUI::Hygiene); Y += 28.f;
	}

	Y += 8.f;

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;

	// --- Resources (a grid of chips that grows as new resources are added) ---
	if (const UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>())
	{
		// Add new resources here as the game gains them — the grid grows on its own.
		struct FResChip { EResourceType Type; UTexture2D* Icon; };
		const FResChip Chips[] = {
			{ EResourceType::Wood,  ItemWood },
			{ EResourceType::Stone, ItemStone },
		};
		const int32 Cols = 2;
		const float CellW = 96.f;
		const float CellH = 26.f;
		const int32 Num = UE_ARRAY_COUNT(Chips);
		for (int32 i = 0; i < Num; ++i)
		{
			const float CX = X + (i % Cols) * CellW;
			const float CY = Y + (i / Cols) * CellH;
			if (Chips[i].Icon)
			{
				DrawTexture(Chips[i].Icon, CX, CY, 22.f, 22.f, 0.f, 0.f, 1.f, 1.f);
			}
			DrawText(FString::Printf(TEXT("%d"), Inventory->GetResourceCount(Chips[i].Type)),
				EveraUI::Text, CX + 28.f, CY + 4.f, Font);
		}
		Y += ((Num + Cols - 1) / Cols) * CellH + 8.f;
	}

	// --- Skills (compact single line) ---
	if (const USkillsComponent* Skills = Pawn->FindComponentByClass<USkillsComponent>())
	{
		DrawText(FString::Printf(TEXT("Woodcut Lv%d    Mining Lv%d"),
			Skills->GetLevel(ESkillType::Woodcutting), Skills->GetLevel(ESkillType::Mining)),
			FLinearColor(0.74f, 0.64f, 0.98f), X, Y, Font);
		Y += 20.f;
	}

	// --- Crafting (tools) ---
	if (const UCraftingComponent* Craft = Pawn->FindComponentByClass<UCraftingComponent>())
	{
		DrawText(FString::Printf(TEXT("Axe %d   Pickaxe %d"),
			Craft->GetCraftedCount(ECraftableItem::StoneAxe),
			Craft->GetCraftedCount(ECraftableItem::StonePickaxe)),
			EveraUI::Text, X, Y, Font);
		Y += 16.f;
		DrawText(TEXT("[C] Craft Axe   [V] Craft Pickaxe"), EveraUI::TextDim, X, Y, Font);
	}

	// --- Build mode banner (top centre) ---
	if (const AEveraCharacter* Evera = Cast<AEveraCharacter>(Pawn))
	{
		if (Evera->IsBuildMode())
		{
			const bool bFurn = Evera->IsFurnitureMode();
			FString Name;
			int32 Cost;
			if (bFurn)
			{
				const int32 Idx = Evera->GetFurnitureIndex();
				Name = ABuildPiece::FurnitureName(Idx);
				Cost = ABuildPiece::FurnitureCost(Idx);
			}
			else
			{
				const EBuildPieceType PT = Evera->GetBuildPieceType();
				Name = ABuildPiece::GetDisplayName(PT);
				Cost = ABuildPiece::GetWoodCost(PT);
			}

			UFont* BannerFont = GEngine ? GEngine->GetMediumFont() : nullptr;
			const float BW = 690.f;
			const float BH = 54.f;
			const float BX = Canvas->SizeX * 0.5f - BW * 0.5f;
			const float BY = 24.f;
			DrawRect(FLinearColor(0.05f, 0.10f, 0.06f, 0.74f), BX, BY, BW, BH);

			DrawText(FString::Printf(TEXT("%s  -  %s  (%d Wood)   Level %d"),
				bFurn ? TEXT("FURNITURE") : TEXT("BUILD"), *Name, Cost, Evera->GetBuildLevel()),
				FLinearColor(0.72f, 1.0f, 0.78f), BX + 16.f, BY + 8.f, BannerFont, 1.1f);
			DrawText(TEXT("[Tab] build/furniture  [N] next  [R] rotate  [PgUp/PgDn] level  [LMB] place  [X] remove  [B] exit"),
				FLinearColor(0.85f, 0.92f, 0.85f), BX + 16.f, BY + 32.f, Font);

			DrawBuildPalette(Evera);
		}
	}

	// --- Lea's tip bubble (bottom centre) + riding hint ---
	if (const AEveraCharacter* Evera = Cast<AEveraCharacter>(Pawn))
	{
		FString Tip;
		FString Who = TEXT("Lea");
		if (const ACompanionPet* Pet = Evera->GetCompanion())
		{
			Tip = Pet->GetActiveTip();
			Who = Pet->GetPetName();
		}
		if (!Tip.IsEmpty())
		{
			UFont* TipFont = GEngine ? GEngine->GetMediumFont() : nullptr;

			// Word-wrap the tip to a comfortable width.
			TArray<FString> Lines;
			{
				const int32 Wrap = 62;
				FString Cur;
				TArray<FString> Words;
				Tip.ParseIntoArray(Words, TEXT(" "));
				for (const FString& W : Words)
				{
					if (Cur.Len() + W.Len() + 1 > Wrap) { Lines.Add(Cur); Cur = W; }
					else { Cur = Cur.IsEmpty() ? W : Cur + TEXT(" ") + W; }
				}
				if (!Cur.IsEmpty()) { Lines.Add(Cur); }
			}

			const float BW = 720.f;
			const float LineH = 22.f;
			const float BH = 34.f + Lines.Num() * LineH;
			const float BX = Canvas->SizeX * 0.5f - BW * 0.5f;
			const float BY = Canvas->SizeY - BH - 44.f;

			DrawRect(EveraUI::A(EveraUI::Ground1, 0.92f), BX, BY, BW, BH);
			DrawRect(EveraUI::A(EveraUI::Gold, 0.75f), BX, BY, BW, 2.f);
			DrawRect(EveraUI::A(EveraUI::Gold, 0.75f), BX, BY + BH - 2.f, BW, 2.f);

			DrawText(FString::Printf(TEXT("%s"), *Who), EveraUI::GoldBright, BX + 20.f, BY + 12.f, TipFont, 1.15f);
			float TY = BY + 12.f;
			const float TX = BX + 92.f;
			for (const FString& L : Lines)
			{
				DrawText(L, EveraUI::Text, TX, TY, TipFont);
				TY += LineH;
			}
		}

		if (Evera->IsMounted())
		{
			UFont* HintFont = GEngine ? GEngine->GetMediumFont() : nullptr;
			DrawText(TEXT("Riding Lea's horse   -   [WASD] ride   [F] get off"),
				EveraUI::GoldBright, Canvas->SizeX * 0.5f - 190.f, 88.f, HintFont);
		}
	}

	// Backpack overlay (toggled with the I key).
	if (bShowInventory)
	{
		DrawBackpack(Pawn);
	}
}

void AEveraHUD::EnsureIcons()
{
	if (bIconsLoaded)
	{
		return;
	}
	bIconsLoaded = true;
	IconHealth = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Icon_Health.T_UI_Icon_Health"));
	IconHunger = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Icon_Hunger.T_UI_Icon_Hunger"));
	IconThirst = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Icon_Thirst.T_UI_Icon_Thirst"));
	IconEnergy = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Icon_Energy.T_UI_Icon_Energy"));
	PanelTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Panel_Menu.T_UI_Panel_Menu"));
	PanelWideTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Panel_Wide.T_UI_Panel_Wide"));
	SlotTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Slot.T_UI_Slot"));
	CellTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Btn_Normal.T_UI_Btn_Normal"));
	CellSelTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Btn_Selected.T_UI_Btn_Selected"));
	ItemWood = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Item_Wood.T_UI_Item_Wood"));
	ItemStone = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Item_Stone.T_UI_Item_Stone"));
	ItemAxe = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Item_Axe.T_UI_Item_Axe"));
}

void AEveraHUD::DrawStatBar(float X, float Y, UTexture2D* Icon, float Value, float Max, const FLinearColor& Color)
{
	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;

	const float IconSize = 24.f;
	const float BarX = X + IconSize + 8.f;
	const float BarWidth = 116.f;
	const float BarHeight = 12.f;
	const float BarY = Y + (IconSize - BarHeight) * 0.5f;
	const float Fraction = (Max > 0.f) ? FMath::Clamp(Value / Max, 0.f, 1.f) : 0.f;

	if (Icon)
	{
		// TintColor + BlendMode default to White + translucent.
		DrawTexture(Icon, X, Y, IconSize, IconSize, 0.f, 0.f, 1.f, 1.f);
	}
	// Gold frame + sunken dark track + coloured fill.
	DrawRect(EveraUI::A(EveraUI::Gold, 0.55f), BarX - 1.5f, BarY - 1.5f, BarWidth + 3.f, BarHeight + 3.f);
	DrawRect(EveraUI::A(EveraUI::Ground0, 0.92f), BarX, BarY, BarWidth, BarHeight);
	DrawRect(Color, BarX, BarY, BarWidth * Fraction, BarHeight);
	DrawText(FString::Printf(TEXT("%.0f"), Value), EveraUI::Text, BarX + BarWidth + 8.f, Y + 4.f, Font);
}

void AEveraHUD::DrawBuildPalette(const AEveraCharacter* Character)
{
	if (!Canvas || !Character)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;

	// Evera theme tokens (UI Bible): charcoal panel, aged gold, olive selection.
	const FLinearColor Wood = EveraUI::A(EveraUI::Ground1, 0.94f);
	const FLinearColor Slot = EveraUI::A(EveraUI::Ground2, 0.96f);
	const FLinearColor SlotSel = EveraUI::A(EveraUI::GoldDeep, 0.55f);
	const FLinearColor Gold = EveraUI::Gold;
	const FLinearColor Cream = EveraUI::Text;
	const FLinearColor Dim = EveraUI::TextDim;

	const bool bFurn = Character->IsFurnitureMode();
	const int32 Count = bFurn ? ABuildPiece::NumFurniture() : static_cast<int32>(EBuildPieceType::Pillar) + 1;
	const int32 Selected = bFurn ? Character->GetFurnitureIndex() : static_cast<int32>(Character->GetBuildPieceType());

	const float CellW = 112.f;
	const float CellH = 58.f;
	const float Gap = 6.f;
	const float TotalW = Count * CellW + (Count - 1) * Gap;
	const float StartX = Canvas->SizeX * 0.5f - TotalW * 0.5f;
	const float Y = Canvas->SizeY - 82.f;

	// Backing panel with a gold frame.
	DrawRect(Wood, StartX - 10.f, Y - 10.f, TotalW + 20.f, CellH + 20.f);
	auto Frame = [&](float bx, float by, float bw, float bh, float t, const FLinearColor& c)
	{
		DrawRect(c, bx, by, bw, t);
		DrawRect(c, bx, by + bh - t, bw, t);
		DrawRect(c, bx, by, t, bh);
		DrawRect(c, bx + bw - t, by, t, bh);
	};
	Frame(StartX - 10.f, Y - 10.f, TotalW + 20.f, CellH + 20.f, 2.f, Gold);

	// Category tabs so it's obvious you can pick BUILD PARTS or FURNITURE.
	{
		const float TabW = 190.f;
		const float TabH = 28.f;
		const float TGap = 8.f;
		const float TabsW = TabW * 2 + TGap;
		const float TX = Canvas->SizeX * 0.5f - TabsW * 0.5f;
		const float TY = Y - 46.f;
		auto DrawTab = [&](float tx, const TCHAR* Label, bool bActive)
		{
			DrawRect(bActive ? SlotSel : Slot, tx, TY, TabW, TabH);
			Frame(tx, TY, TabW, TabH, 2.f, bActive ? Gold : Dim);
			DrawText(Label, bActive ? Gold : Cream, tx + 16.f, TY + 7.f, Font);
		};
		DrawTab(TX, TEXT("BUILD PARTS"), !bFurn);
		DrawTab(TX + TabW + TGap, TEXT("FURNITURE"), bFurn);
	}

	for (int32 i = 0; i < Count; ++i)
	{
		const float CX = StartX + i * (CellW + Gap);
		const bool bSel = (i == Selected);
		const FLinearColor IconCol = bSel ? Cream : Dim;

		UTexture2D* CellArt = bSel ? CellSelTex : CellTex;
		if (CellArt)
		{
			DrawTexture(CellArt, CX, Y, CellW, CellH, 0.f, 0.f, 1.f, 1.f);
		}
		else
		{
			DrawRect(bSel ? SlotSel : Slot, CX, Y, CellW, CellH);
			if (bSel) { Frame(CX, Y, CellW, CellH, 2.f, Gold); }
		}

		FString ItemName;
		int32 ItemCost;
		if (bFurn)
		{
			ItemName = ABuildPiece::FurnitureName(i);
			ItemCost = ABuildPiece::FurnitureCost(i);
			// Generic furniture pictogram (a little stool/box).
			DrawRect(IconCol, CX + 10.f, Y + 20.f, 26.f, 8.f);   // seat/top
			DrawRect(IconCol, CX + 12.f, Y + 28.f, 5.f, 14.f);   // leg
			DrawRect(IconCol, CX + 29.f, Y + 28.f, 5.f, 14.f);   // leg
		}
		else
		{
			const EBuildPieceType Type = static_cast<EBuildPieceType>(i);
			ItemName = ABuildPiece::GetDisplayName(Type);
			ItemCost = ABuildPiece::GetWoodCost(Type);
			DrawPieceIcon(Type, CX + 8.f, Y + 14.f, 30.f, IconCol);
		}

		DrawText(FString::Printf(TEXT("%d"), i + 1), bSel ? Gold : Dim, CX + 6.f, Y + 2.f, Font);
		DrawText(ItemName, bSel ? Cream : Dim, CX + 46.f, Y + 12.f, Font);
		DrawText(FString::Printf(TEXT("%d wood"), ItemCost), bSel ? Gold : Dim, CX + 46.f, Y + 32.f, Font);
	}
}

void AEveraHUD::DrawPieceIcon(EBuildPieceType Type, float X, float Y, float S, const FLinearColor& C)
{
	// Simple pictograms so young players recognise each piece at a glance.
	switch (Type)
	{
	case EBuildPieceType::Floor:
		DrawRect(C, X, Y + S * 0.62f, S, S * 0.22f);
		break;

	case EBuildPieceType::Wall:
		DrawRect(C, X, Y + S * 0.10f, S, S * 0.80f);
		break;

	case EBuildPieceType::Doorway:
		DrawRect(C, X, Y, S * 0.22f, S);              // left post
		DrawRect(C, X + S * 0.78f, Y, S * 0.22f, S);  // right post
		DrawRect(C, X, Y, S, S * 0.20f);              // lintel
		break;

	case EBuildPieceType::Window:
	{
		const FLinearColor Hole(0.16f, 0.13f, 0.08f, 1.f);
		DrawRect(C, X, Y + S * 0.15f, S, S * 0.70f);          // frame block
		DrawRect(Hole, X + S * 0.18f, Y + S * 0.30f, S * 0.64f, S * 0.40f); // glass hole
		DrawLine(X + S * 0.5f, Y + S * 0.15f, X + S * 0.5f, Y + S * 0.85f, C, 1.5f); // mullion V
		DrawLine(X, Y + S * 0.5f, X + S, Y + S * 0.5f, C, 1.5f);                       // mullion H
		break;
	}

	case EBuildPieceType::Roof:
		DrawLine(X + S * 0.5f, Y, X, Y + S * 0.85f, C, 2.f);
		DrawLine(X + S * 0.5f, Y, X + S, Y + S * 0.85f, C, 2.f);
		DrawLine(X, Y + S * 0.85f, X + S, Y + S * 0.85f, C, 2.f);
		break;

	case EBuildPieceType::Pillar:
		DrawRect(C, X + S * 0.36f, Y, S * 0.28f, S);
		break;
	}
}

void AEveraHUD::DrawBackpack(APawn* Pawn)
{
	if (!Canvas || !Pawn)
	{
		return;
	}

	EnsureIcons();
	UFont* TitleFont = GEngine ? GEngine->GetMediumFont() : nullptr;
	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;

	// Dim the world behind the panel.
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f), 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);

	const float W = 830.f;
	const float H = 560.f;
	const float X0 = Canvas->SizeX * 0.5f - W * 0.5f;
	const float Y0 = Canvas->SizeY * 0.5f - H * 0.5f;

	if (PanelWideTex)
	{
		DrawTexture(PanelWideTex, X0, Y0, W, H, 0.f, 0.f, 1.f, 1.f);
	}
	else
	{
		DrawRect(EveraUI::A(EveraUI::Ground1, 0.96f), X0, Y0, W, H);
	}

	DrawText(TEXT("INVENTORY"), EveraUI::GoldBright, X0 + W * 0.5f - 58.f, Y0 + 40.f, TitleFont, 1.4f);

	// Collect the player's items (real kit icons).
	struct FItem { FString Label; int32 Count; UTexture2D* Icon; };
	TArray<FItem> Items;
	if (const UInventoryComponent* Inv = Pawn->FindComponentByClass<UInventoryComponent>())
	{
		Items.Add({ TEXT("Wood"),  Inv->GetResourceCount(EResourceType::Wood),  ItemWood });
		Items.Add({ TEXT("Stone"), Inv->GetResourceCount(EResourceType::Stone), ItemStone });
	}
	if (const UCraftingComponent* Craft = Pawn->FindComponentByClass<UCraftingComponent>())
	{
		Items.Add({ TEXT("Stone Axe"), Craft->GetCraftedCount(ECraftableItem::StoneAxe), ItemAxe });
	}

	// A grid of ornate slots; items fill the first cells, the rest stay empty.
	const int32 Cols = 8;
	const int32 TotalSlots = 32;
	const float SlotSize = 74.f;
	const float Gap = 9.f;
	const float GridW = Cols * SlotSize + (Cols - 1) * Gap;
	const float GX = X0 + (W - GridW) * 0.5f;
	const float GY = Y0 + 100.f;

	for (int32 s = 0; s < TotalSlots; ++s)
	{
		const float SX = GX + (s % Cols) * (SlotSize + Gap);
		const float SY = GY + (s / Cols) * (SlotSize + Gap);
		if (SlotTex)
		{
			DrawTexture(SlotTex, SX, SY, SlotSize, SlotSize, 0.f, 0.f, 1.f, 1.f);
		}
		else
		{
			DrawRect(EveraUI::A(EveraUI::Ground2, 0.9f), SX, SY, SlotSize, SlotSize);
		}

		if (s < Items.Num())
		{
			// Real item icon centered in the slot + count.
			const float Pad = 10.f;
			if (Items[s].Icon)
			{
				DrawTexture(Items[s].Icon, SX + Pad, SY + Pad, SlotSize - 2.f * Pad, SlotSize - 2.f * Pad, 0.f, 0.f, 1.f, 1.f);
			}
			DrawText(FString::Printf(TEXT("%d"), Items[s].Count), EveraUI::Text, SX + SlotSize - 22.f, SY + SlotSize - 18.f, Font);
		}
	}

	DrawText(TEXT("[I] Close"), EveraUI::TextDim, X0 + 42.f, Y0 + H - 42.f, Font);
}
