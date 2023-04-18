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

	void SetCurrentPlayset(class UFortPlaysetItemDefinition* NewPlayset)
	{
		static auto SetCurrentPlaysetFn = FindObject<UFunction>("/Script/FortniteGame.FortVolume.SetCurrentPlayset");
		this->ProcessEvent(SetCurrentPlaysetFn, &NewPlayset);
	}

	void UpdateSize(const FVector& Scale)
	{
		static auto UpdateSizeFn = FindObject<UFunction>("/Script/FortniteGame.FortVolume.UpdateSize");
		this->ProcessEvent(UpdateSizeFn, (FVector*)&Scale);
	}
};