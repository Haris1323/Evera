// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "EveraHUD.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "SurvivalStatsComponent.h"
#include "InventoryComponent.h"
#include "SkillsComponent.h"

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

	// Compact, unobtrusive panel in the top-left corner.
	const float PanelX = 16.f;
	const float PanelY = 16.f;
	const float PanelW = 210.f;
	const float PanelH = 156.f;
	DrawRect(FLinearColor(0.03f, 0.05f, 0.07f, 0.42f), PanelX, PanelY, PanelW, PanelH);

	const float X = PanelX + 12.f;
	float Y = PanelY + 10.f;

	// --- Survival bars ---
	if (const USurvivalStatsComponent* Stats = Pawn->FindComponentByClass<USurvivalStatsComponent>())
	{
		const float Max = Stats->GetMaxValue();
		DrawStatBar(X, Y, TEXT("Health"), Stats->Health, Max, FLinearColor(0.90f, 0.28f, 0.28f)); Y += 16.f;
		DrawStatBar(X, Y, TEXT("Hunger"), Stats->Hunger, Max, FLinearColor(0.94f, 0.62f, 0.26f)); Y += 16.f;
		DrawStatBar(X, Y, TEXT("Thirst"), Stats->Thirst, Max, FLinearColor(0.34f, 0.68f, 0.95f)); Y += 16.f;
		DrawStatBar(X, Y, TEXT("Energy"), Stats->Energy, Max, FLinearColor(0.45f, 0.82f, 0.42f)); Y += 16.f;
	}

	Y += 8.f;

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;

	// --- Inventory (compact single line) ---
	if (const UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>())
	{
		const int32 Wood = Inventory->GetResourceCount(EResourceType::Wood);
		const int32 Stone = Inventory->GetResourceCount(EResourceType::Stone);
		DrawText(FString::Printf(TEXT("Wood %d    Stone %d"), Wood, Stone), FLinearColor(0.92f, 0.88f, 0.70f), X, Y, Font);
		Y += 16.f;
	}

	// --- Skills (compact single line) ---
	if (const USkillsComponent* Skills = Pawn->FindComponentByClass<USkillsComponent>())
	{
		DrawText(FString::Printf(TEXT("Woodcut Lv%d    Mining Lv%d"),
			Skills->GetLevel(ESkillType::Woodcutting), Skills->GetLevel(ESkillType::Mining)),
			FLinearColor(0.74f, 0.64f, 0.98f), X, Y, Font);
	}
}

void AEveraHUD::DrawStatBar(float X, float Y, const FString& Label, float Value, float Max, const FLinearColor& Color)
{
	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;

	const float LabelWidth = 46.f;
	const float BarWidth = 108.f;
	const float BarHeight = 9.f;
	const float BarY = Y + 2.f;
	const float Fraction = (Max > 0.f) ? FMath::Clamp(Value / Max, 0.f, 1.f) : 0.f;

	DrawText(Label, FLinearColor(0.88f, 0.90f, 0.92f), X, Y, Font);
	// Track and fill.
	DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.14f), X + LabelWidth, BarY, BarWidth, BarHeight);
	DrawRect(Color, X + LabelWidth, BarY, BarWidth * Fraction, BarHeight);
	DrawText(FString::Printf(TEXT("%.0f"), Value), FLinearColor(0.80f, 0.82f, 0.85f), X + LabelWidth + BarWidth + 6.f, Y, Font);
}
