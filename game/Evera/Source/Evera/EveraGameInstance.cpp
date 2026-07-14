// Copyright NoLimit Developments FZ-LLC. All Rights Reserved.

#include "EveraGameInstance.h"

namespace
{
	struct FAvatarDef
	{
		const TCHAR* Name;
		const TCHAR* Mesh;
		const TCHAR* Idle;
		const TCHAR* Walk;
		const TCHAR* Chop;
	};

	const FAvatarDef GAvatars[] = {
		{ TEXT("Boy"),
		  TEXT("/Game/RPGHeroSquad/Mesh/Character/SK_TinyHeroPolyart.SK_TinyHeroPolyart"),
		  TEXT("/Game/RPGHeroSquad/Animation/TinyHero/Anim_Idle_Normal_TinyHero.Anim_Idle_Normal_TinyHero"),
		  TEXT("/Game/RPGHeroSquad/Animation/TinyHero/InPlace/Anim_MoveFWD_Normal_InPlace_TinyHero.Anim_MoveFWD_Normal_InPlace_TinyHero"),
		  TEXT("/Game/RPGHeroSquad/Animation/TinyHero/Anim_Attack01_TinyHero.Anim_Attack01_TinyHero") },

		{ TEXT("Girl"),
		  TEXT("/Game/RPGHeroSquad/Mesh/Character/SK_TinyHeroGirlPolyart.SK_TinyHeroGirlPolyart"),
		  TEXT("/Game/RPGHeroSquad/Animation/TinyHero/Anim_Idle_Normal_TinyHero.Anim_Idle_Normal_TinyHero"),
		  TEXT("/Game/RPGHeroSquad/Animation/TinyHero/InPlace/Anim_MoveFWD_Normal_InPlace_TinyHero.Anim_MoveFWD_Normal_InPlace_TinyHero"),
		  TEXT("/Game/RPGHeroSquad/Animation/TinyHero/Anim_Attack01_TinyHero.Anim_Attack01_TinyHero") },

		{ TEXT("Knight"),
		  TEXT("/Game/RPGHeroSquad/Mesh/Character/SK_RPGHeroPolyart.SK_RPGHeroPolyart"),
		  TEXT("/Game/RPGHeroSquad/Animation/RPGHero/Anim_Idle_RPGHero.Anim_Idle_RPGHero"),
		  TEXT("/Game/RPGHeroSquad/Animation/RPGHero/InPlace/Anim_Walk_IP_RPGHero.Anim_Walk_IP_RPGHero"),
		  TEXT("/Game/RPGHeroSquad/Animation/RPGHero/Anim_NormalAttack01_RPGHero.Anim_NormalAttack01_RPGHero") },

		{ TEXT("Doggo"),
		  TEXT("/Game/RPGHeroSquad/Mesh/Character/SK_DogPolyart.SK_DogPolyart"),
		  TEXT("/Game/RPGHeroSquad/Animation/AnimalHero/Anim_Idle_Battle_AnimalHero.Anim_Idle_Battle_AnimalHero"),
		  TEXT("/Game/RPGHeroSquad/Animation/AnimalHero/InPlace/Anim_WalkForwardBattle_IP_AnimalHero.Anim_WalkForwardBattle_IP_AnimalHero"),
		  TEXT("/Game/RPGHeroSquad/Animation/AnimalHero/Anim_Attack01_AnimalHero.Anim_Attack01_AnimalHero") },
	};
}

int32 UEveraGameInstance::NumAvatars() { return UE_ARRAY_COUNT(GAvatars); }

FString UEveraGameInstance::AvatarName(int32 Index)
{
	return (Index >= 0 && Index < NumAvatars()) ? FString(GAvatars[Index].Name) : FString();
}

FString UEveraGameInstance::AvatarMeshPath(int32 Index)
{
	return (Index >= 0 && Index < NumAvatars()) ? FString(GAvatars[Index].Mesh) : FString();
}

FString UEveraGameInstance::AvatarIdlePath(int32 Index)
{
	return (Index >= 0 && Index < NumAvatars()) ? FString(GAvatars[Index].Idle) : FString();
}

FString UEveraGameInstance::AvatarWalkPath(int32 Index)
{
	return (Index >= 0 && Index < NumAvatars()) ? FString(GAvatars[Index].Walk) : FString();
}

FString UEveraGameInstance::AvatarChopPath(int32 Index)
{
	return (Index >= 0 && Index < NumAvatars()) ? FString(GAvatars[Index].Chop) : FString();
}
