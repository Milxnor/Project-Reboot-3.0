#pragma once

#include "BuildingSMActor.h"
#include "FortPawn.h"

class ABuildingContainer : public ABuildingSMActor
{
public:
	bool ShouldDestroyOnSearch()
	{
		static auto bDestroyContainerOnSearchOffset = GetOffset("bDestroyContainerOnSearch");
		static auto bDestroyContainerOnSearchFieldMask = GetFieldMask(GetProperty("bDestroyContainerOnSearch"));
		return this->ReadBitfieldValue(bDestroyContainerOnSearchOffset, bDestroyContainerOnSearchFieldMask);
	}

	bool SpawnLoot(AFortPawn* Pawn);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.BuildingContainer");
		return Class;
	}
};