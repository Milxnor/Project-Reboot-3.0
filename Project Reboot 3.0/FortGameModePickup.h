#pragma once

#include "FortPickup.h"

class AFortGameModePickup : public AFortPickup//Athena
{
public:
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortGameModePickup");
		return Class;
	}
};