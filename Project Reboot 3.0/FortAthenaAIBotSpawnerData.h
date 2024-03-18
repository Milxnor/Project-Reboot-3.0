#pragma once

#include "reboot.h"
#include "FortAthenaAISpawnerDataComponent_SpawnParams.h"
#include "FortAthenaAISpawnerDataComponent_CosmeticLoadout.h"

class UFortAthenaAIBotSpawnerData : public UObject
{
public:
	UFortAthenaAISpawnerDataComponent_SpawnParams*& GetSpawnParamsComponent()
	{
		static auto SpawnParamsComponentOffset = GetOffset("SpawnParamsComponent");
		return Get<UFortAthenaAISpawnerDataComponent_SpawnParams*>(SpawnParamsComponentOffset);
	}

	UFortAthenaAISpawnerDataComponent_CosmeticLoadout*& GetCosmeticComponent()
	{
		static auto CosmeticComponentOffset = GetOffset("CosmeticComponent");
		return Get<UFortAthenaAISpawnerDataComponent_CosmeticLoadout*>(CosmeticComponentOffset);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotSpawnerData");
		return Class;
	}
};