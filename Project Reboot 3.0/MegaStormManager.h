#pragma once

#include "Actor.h"

#include "reboot.h"

struct FMegaStormCircle
{
	int GetStructSize()
	{
		static auto MegaStormCircleStruct = FindObject<UStruct>("/Script/FortniteGame.MegaStormCircle");
		return MegaStormCircleStruct->GetPropertiesSize();
	}
};

class AMegaStormManager : public AActor
{
public:
	TArray<__int64>& GetMegaStormCircles()
	{
		static auto MegaStormCirclesOffset = GetOffset("MegaStormCircles");
		return Get<TArray<__int64>>(MegaStormCirclesOffset);
	}
};