// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "EveraMainMenu.h"
#include "EveraGameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateTypes.h"
#include "Kismet/GameplayStatics.h"
#include "EveraUITheme.h"

namespace
{
	const FLinearColor Gold = EveraUI::Gold;
	const FLinearColor Cream = EveraUI::Text;
	const FLinearColor SlotDark = EveraUI::A(EveraUI::Ground2, 1.f);
	const FLinearColor SlotHover = EveraUI::A(EveraUI::Ground3, 1.f);
	const FLinearColor GoldSoft = EveraUI::GoldDeep;

	FSlateBrush RoundedBrush(const FLinearColor& Fill, const FLinearColor& Outline, float Radius, float OutlineWidth)
	{
		FSlateBrush B;
		B.DrawAs = ESlateBrushDrawType::RoundedBox;
		B.TintColor = FSlateColor(Fill);
		B.OutlineSettings.Color = FSlateColor(Outline);
		B.OutlineSettings.CornerRadii = FVector4(Radius, Radius, Radius, Radius);
		B.OutlineSettings.Width = OutlineWidth;
		B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		return B;
	}

	FButtonStyle ButtonStyle(const FLinearColor& Normal, const FLinearColor& Hover)
	{
		FButtonStyle S;
		S.SetNormal(RoundedBrush(Normal, GoldSoft, 8.f, 1.5f));
		S.SetHovered(RoundedBrush(Hover, Gold, 8.f, 2.f));
		S.SetPressed(RoundedBrush(Hover, Gold, 8.f, 2.f));
		S.SetNormalPadding(FMargin(14.f, 7.f));
		S.SetPressedPadding(FMargin(14.f, 7.f));
		return S;
	}

	// Ornate button art (from the UI kit) as the button's state brushes.
	FSlateBrush TexBrush(UTexture2D* T)
	{
		FSlateBrush B;
		B.SetResourceObject(T);
		B.DrawAs = ESlateBrushDrawType::Image;
		B.ImageSize = FVector2D(217.f, 116.f);
		B.TintColor = FSlateColor(FLinearColor::White);
		return B;
	}
	FButtonStyle OrnateStyle(UTexture2D* N, UTexture2D* H)
	{
		FButtonStyle S;
		S.SetNormal(TexBrush(N));
		S.SetHovered(TexBrush(H));
		S.SetPressed(TexBrush(H));
		S.SetDisabled(TexBrush(N));
		S.SetNormalPadding(FMargin(0.f));
		S.SetPressedPadding(FMargin(0.f));
		return S;
	}
}

TSharedRef<SWidget> UEveraMainMenu::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildMenu();
	}
	return Super::RebuildWidget();
}

UButton* UEveraMainMenu::MakeButton(UPanelWidget* Parent, const FString& Label, float FontSize, FName Name, float Width, float Height)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetStyle((BtnNormalTex && BtnHoverTex) ? OrnateStyle(BtnNormalTex, BtnHoverTex)
	                                               : ButtonStyle(SlotDark, SlotHover));

	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(FText::FromString(Label));
	FSlateFontInfo Font = Text->GetFont();
	Font.Size = FontSize;
	Text->SetFont(Font);
	Text->SetColorAndOpacity(FSlateColor(Cream));
	Text->SetJustification(ETextJustify::Center);
	Button->AddChild(Text);

	// Fixed size so the ornate art keeps its aspect and doesn't distort.
	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Box->SetWidthOverride(Width);
	Box->SetHeightOverride(Height);
	Box->SetContent(Button);

	Parent->AddChild(Box);
	if (UHorizontalBoxSlot* HB = Cast<UHorizontalBoxSlot>(Box->Slot))
	{
		HB->SetPadding(FMargin(5.f, 0.f));
		HB->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* VBs = Cast<UVerticalBoxSlot>(Box->Slot))
	{
		VBs->SetHorizontalAlignment(HAlign_Center);
	}
	return Button;
}

void UEveraMainMenu::BuildMenu()
{
	// Ornate button art from the UI kit.
	BtnNormalTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Btn_Normal.T_UI_Btn_Normal"));
	BtnHoverTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Btn_Hover.T_UI_Btn_Hover"));
	BtnSelectedTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Btn_Selected.T_UI_Btn_Selected"));

	// Root: a warm dark veil so the 3D backdrop still reads through.
	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Root"));
	Root->SetBrushColor(EveraUI::A(EveraUI::Ground0, 0.34f));
	Root->SetHorizontalAlignment(HAlign_Center);
	Root->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Root;

	// Fixed-size ornate panel frame (art), with the form overlaid inside it.
	USizeBox* Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Frame->SetWidthOverride(720.f);
	Frame->SetHeightOverride(720.f);
	Root->SetContent(Frame);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Frame->SetContent(Overlay);

	if (UTexture2D* PanelTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Panel_Menu.T_UI_Panel_Menu")))
	{
		UImage* PanelImg = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		PanelImg->SetBrushFromTexture(PanelTex, true);
		if (UOverlaySlot* PSlot = Overlay->AddChildToOverlay(PanelImg))
		{
			PSlot->SetHorizontalAlignment(HAlign_Fill);
			PSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	UVerticalBox* VB = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VB"));
	if (UOverlaySlot* CSlot = Overlay->AddChildToOverlay(VB))
	{
		CSlot->SetHorizontalAlignment(HAlign_Fill);
		CSlot->SetVerticalAlignment(VAlign_Center);
		CSlot->SetPadding(FMargin(116.f, 82.f, 116.f, 122.f)); // sit inside the ornate border, lifted up a touch
	}

	auto AddRow = [&](UWidget* W, float PadTop)
	{
		if (UVerticalBoxSlot* VBSlot = VB->AddChildToVerticalBox(W))
		{
			VBSlot->SetHorizontalAlignment(HAlign_Center);
			VBSlot->SetPadding(FMargin(0.f, PadTop, 0.f, 0.f));
		}
	};

	// Logo image, held at a fixed size by a SizeBox (the brush fills it, keeping
	// the 1672x941 aspect). Falls back to a gold wordmark if the texture is missing.
	UTexture2D* LogoTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Evera/UI/T_UI_Logo.T_UI_Logo"));
	if (LogoTex)
	{
		USizeBox* LogoBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		LogoBox->SetWidthOverride(300.f);
		LogoBox->SetHeightOverride(200.f); // 1536x1024 aspect

		UImage* Logo = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Logo->SetBrushFromTexture(LogoTex, true);
		LogoBox->AddChild(Logo);

		AddRow(LogoBox, 0.f);
	}
	else
	{
		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Title->SetText(FText::FromString(TEXT("EVERA")));
		FSlateFontInfo Font = Title->GetFont();
		Font.Size = 96;
		Font.LetterSpacing = 300;
		Title->SetFont(Font);
		Title->SetColorAndOpacity(FSlateColor(Gold));
		AddRow(Title, 0.f);
	}

	// Name entry.
	UTextBlock* NameLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	NameLabel->SetText(FText::FromString(TEXT("Enter your name")));
	{
		FSlateFontInfo Font = NameLabel->GetFont();
		Font.Size = 20;
		NameLabel->SetFont(Font);
	}
	NameLabel->SetColorAndOpacity(FSlateColor(Gold));
	AddRow(NameLabel, 26.f);

	NameBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("NameBox"));
	NameBox->SetHintText(FText::FromString(TEXT("Your name")));
	{
		// Dark field + cream text so the typed name is readable (default was light-on-light).
		const FLinearColor InkBg(0.03f, 0.028f, 0.02f, 1.f);
		FEditableTextBoxStyle TS = NameBox->WidgetStyle;
		TS.BackgroundImageNormal = RoundedBrush(InkBg, EveraUI::A(EveraUI::Gold, 0.45f), 6.f, 1.5f);
		TS.BackgroundImageHovered = RoundedBrush(InkBg, EveraUI::A(EveraUI::Gold, 0.6f), 6.f, 1.5f);
		TS.BackgroundImageFocused = RoundedBrush(InkBg, EveraUI::Gold, 6.f, 2.f);
		TS.BackgroundImageReadOnly = RoundedBrush(InkBg, EveraUI::A(EveraUI::Gold, 0.45f), 6.f, 1.5f);
		TS.ForegroundColor = FSlateColor(EveraUI::Text);
		TS.Padding = FMargin(12.f, 8.f);
		NameBox->WidgetStyle = TS;
		NameBox->SetForegroundColor(EveraUI::Text);
	}
	if (UVerticalBoxSlot* NBSlot = VB->AddChildToVerticalBox(NameBox))
	{
		NBSlot->SetHorizontalAlignment(HAlign_Fill);
		NBSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));
	}

	// Avatar choice.
	UTextBlock* PickLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	PickLabel->SetText(FText::FromString(TEXT("Choose your avatar")));
	{
		FSlateFontInfo Font = PickLabel->GetFont();
		Font.Size = 20;
		PickLabel->SetFont(Font);
	}
	PickLabel->SetColorAndOpacity(FSlateColor(Gold));
	AddRow(PickLabel, 30.f);

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	AvatarButtons.Reset();
	const int32 Count = UEveraGameInstance::NumAvatars();
	for (int32 i = 0; i < Count; ++i)
	{
		UButton* Button = MakeButton(Row, UEveraGameInstance::AvatarName(i), 16.f, *FString::Printf(TEXT("Avatar%d"), i), 108.f, 58.f);
		AvatarButtons.Add(Button);
	}
	if (AvatarButtons.IsValidIndex(0)) AvatarButtons[0]->OnClicked.AddDynamic(this, &UEveraMainMenu::OnAvatar0);
	if (AvatarButtons.IsValidIndex(1)) AvatarButtons[1]->OnClicked.AddDynamic(this, &UEveraMainMenu::OnAvatar1);
	if (AvatarButtons.IsValidIndex(2)) AvatarButtons[2]->OnClicked.AddDynamic(this, &UEveraMainMenu::OnAvatar2);
	if (AvatarButtons.IsValidIndex(3)) AvatarButtons[3]->OnClicked.AddDynamic(this, &UEveraMainMenu::OnAvatar3);
	AddRow(Row, 12.f);

	AvatarLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	{
		FSlateFontInfo Font = AvatarLabel->GetFont();
		Font.Size = 18;
		AvatarLabel->SetFont(Font);
	}
	AvatarLabel->SetColorAndOpacity(FSlateColor(Cream));
	AddRow(AvatarLabel, 10.f);

	// Play.
	UButton* Play = MakeButton(VB, TEXT("PLAY"), 24.f, TEXT("PlayButton"), 200.f, 96.f);
	Play->OnClicked.AddDynamic(this, &UEveraMainMenu::OnPlayClicked);
	if (UWidget* PlayBox = Play->GetParent())
	{
		if (UVerticalBoxSlot* PlaySlot = Cast<UVerticalBoxSlot>(PlayBox->Slot))
		{
			PlaySlot->SetPadding(FMargin(0.f, 14.f, 0.f, 0.f));
		}
	}

	SelectAvatar(0);
}

void UEveraMainMenu::SelectAvatar(int32 Index)
{
	SelectedAvatar = FMath::Clamp(Index, 0, UEveraGameInstance::NumAvatars() - 1);

	// Highlight the chosen avatar with the ornate "selected" (olive/gold) button.
	for (int32 i = 0; i < AvatarButtons.Num(); ++i)
	{
		if (!AvatarButtons[i]) continue;
		if (BtnNormalTex && BtnSelectedTex)
		{
			AvatarButtons[i]->SetStyle(i == SelectedAvatar
				? OrnateStyle(BtnSelectedTex, BtnSelectedTex)
				: OrnateStyle(BtnNormalTex, BtnHoverTex));
		}
		else
		{
			AvatarButtons[i]->SetStyle(i == SelectedAvatar
				? ButtonStyle(FLinearColor(0.20f, 0.24f, 0.11f, 1.f), FLinearColor(0.26f, 0.30f, 0.14f, 1.f))
				: ButtonStyle(SlotDark, SlotHover));
		}
	}

	if (AvatarLabel)
	{
		AvatarLabel->SetText(FText::FromString(FString::Printf(TEXT("Selected:  %s"),
			*UEveraGameInstance::AvatarName(SelectedAvatar))));
	}
}

void UEveraMainMenu::OnAvatar0() { SelectAvatar(0); }
void UEveraMainMenu::OnAvatar1() { SelectAvatar(1); }
void UEveraMainMenu::OnAvatar2() { SelectAvatar(2); }
void UEveraMainMenu::OnAvatar3() { SelectAvatar(3); }

void UEveraMainMenu::OnPlayClicked()
{
	if (UEveraGameInstance* GI = GetGameInstance<UEveraGameInstance>())
	{
		if (NameBox)
		{
			const FString Typed = NameBox->GetText().ToString().TrimStartAndEnd();
			if (!Typed.IsEmpty())
			{
				GI->PlayerName = Typed;
			}
		}
		GI->AvatarIndex = SelectedAvatar;
	}

	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Evera/Maps/EveraForest")));
}
