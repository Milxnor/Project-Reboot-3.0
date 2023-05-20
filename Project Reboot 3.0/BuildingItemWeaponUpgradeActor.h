#pragma once

#include "BuildingGameplayActor.h"

class ABuildingItemWeaponUpgradeActor : public ABuildingGameplayActor // ABuildingItemCollectorActor
{
public:

	static class UClass* StaticClass()
	{
		static UClass* Class = FindObject<UClass>(L"/Script/FortniteGame.BuildingItemWeaponUpgradeActor");
		return Class;
	}
};