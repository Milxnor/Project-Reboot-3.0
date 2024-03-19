#pragma once

#include "reboot.h"
#include "AIController.h"

class AFortAthenaAIBotController : public AAIController
{
public:
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotController");
		return Class;
	}
};