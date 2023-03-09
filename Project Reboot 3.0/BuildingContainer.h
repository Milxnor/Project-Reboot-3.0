#pragma once

#include "BuildingSMActor.h"

class ABuildingContainer : public ABuildingSMActor
{
public:
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.BuildingContainer");
		return Class;
	}
};