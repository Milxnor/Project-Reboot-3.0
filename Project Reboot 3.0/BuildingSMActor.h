#pragma once

#include "BuildingActor.h"
#include "PlayerState.h"

class ABuildingSMActor : public ABuildingActor
{
public:
	bool IsPlayerPlaced()
	{
		return true; // FOR NOW
	}

	void SetPlayerPlaced(bool NewValue)
	{

	}

	bool IsDestroyed()
	{
		return false; // FOR NOW
	}

	APlayerState*& GetEditingPlayer()
	{
		static auto EditingPlayerOffset = GetOffset("EditingPlayer");
		return Get<APlayerState*>(EditingPlayerOffset);
	}

	int& GetCurrentBuildingLevel()
	{
		static auto CurrentBuildingLevelOffset = GetOffset("CurrentBuildingLevel");
		return Get<int>(CurrentBuildingLevelOffset);
	}
};