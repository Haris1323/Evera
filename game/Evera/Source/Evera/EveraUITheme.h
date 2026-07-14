// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 *  EVERA UI design tokens — the single source of colour for every widget and the
 *  Canvas HUD, matching the UI Bible. Values are the Bible's sRGB hex as 0..1
 *  components (that is how the existing Canvas HUD already reads colour).
 *
 *  Warm charcoal grounds, aged gold accent, warm beige text, olive selection,
 *  and semantic colours for vitals and feedback.
 */
namespace EveraUI
{
	// Grounds (warm charcoal)
	inline const FLinearColor Ground0(0.055f, 0.047f, 0.035f); // #0E0C09 page
	inline const FLinearColor Ground1(0.090f, 0.078f, 0.059f); // #17140F panel
	inline const FLinearColor Ground2(0.125f, 0.106f, 0.078f); // #201B14 raised
	inline const FLinearColor Ground3(0.169f, 0.141f, 0.102f); // #2B241A hover

	// Aged gold (the one accent)
	inline const FLinearColor Gold(0.788f, 0.635f, 0.294f);       // #C9A24B
	inline const FLinearColor GoldBright(0.894f, 0.784f, 0.467f); // #E4C877
	inline const FLinearColor GoldDeep(0.541f, 0.427f, 0.184f);   // #8A6D2F

	// Text (warm beige)
	inline const FLinearColor Text(0.914f, 0.875f, 0.780f);    // #E9DFC7
	inline const FLinearColor TextDim(0.663f, 0.612f, 0.510f); // #A99C82
	inline const FLinearColor TextMute(0.435f, 0.396f, 0.325f);// #6F6553

	// Semantic — vitals & feedback
	inline const FLinearColor Health(0.753f, 0.314f, 0.227f);  // #C0503A
	inline const FLinearColor Hunger(0.784f, 0.525f, 0.243f);  // #C8863E
	inline const FLinearColor Thirst(0.306f, 0.525f, 0.659f);  // #4E86A8
	inline const FLinearColor Energy(0.498f, 0.635f, 0.294f);  // #7FA24B
	inline const FLinearColor Hygiene(0.404f, 0.612f, 0.682f); // soft water-teal
	inline const FLinearColor XP(0.725f, 0.561f, 0.235f);      // #B98F3C
	inline const FLinearColor Success(0.435f, 0.627f, 0.294f); // #6FA04B
	inline const FLinearColor Warning(0.847f, 0.639f, 0.227f); // #D8A33A
	inline const FLinearColor Danger(0.706f, 0.278f, 0.180f);  // #B4472E
	inline const FLinearColor Selected(0.541f, 0.604f, 0.306f);// #8A9A4E olive

	// Helper: a colour with a specific alpha.
	inline FLinearColor A(const FLinearColor& C, float Alpha)
	{
		return FLinearColor(C.R, C.G, C.B, Alpha);
	}
}
