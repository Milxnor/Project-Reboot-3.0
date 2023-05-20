#pragma once

#include "reboot.h"

class UFortAthenaAIBotCustomizationData : public UObject // UPrimaryDataAsset
{
public:
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaAIBotCustomizationData");
		return Class;
	}
};