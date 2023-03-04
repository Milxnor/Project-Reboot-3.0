#pragma once

#include "Actor.h"
#include "reboot.h" // we want to prevent this but im to lazy to make cpp file

class ABuildingActor : public AActor
{
public:
	void InitializeBuildingActor(UObject* Controller, ABuildingActor* BuildingOwner, bool bUsePlayerBuildAnimations, UObject* ReplacedBuilding = nullptr)
	{
		struct {
			UObject* BuildingOwner; // ABuildingActor
			UObject* SpawningController;
			bool bUsePlayerBuildAnimations; // I think this is not on some versions
			UObject* ReplacedBuilding; // this also not on like below 18.00
		} IBAParams{ BuildingOwner, Controller, bUsePlayerBuildAnimations, ReplacedBuilding };

		static auto fn = FindObject<UFunction>("/Script/FortniteGame.BuildingActor.InitializeKismetSpawnedBuildingActor");
		this->ProcessEvent(fn, &IBAParams);
	}
};