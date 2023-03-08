#pragma once

#include "BuildingActor.h"

enum class EVolumeState : uint8_t
{
	Uninitialized = 0,
	ReadOnly = 1,
	Initializing = 2,
	Ready = 3,
	EVolumeState_MAX = 4
};

class AFortVolume : public ABuildingActor // ABuildingGameplayActor
{
public:
	EVolumeState& GetVolumeState()
	{
		static auto VolumeStateOffset = GetOffset("VolumeState");
		return Get<EVolumeState>(VolumeStateOffset);
	}
};