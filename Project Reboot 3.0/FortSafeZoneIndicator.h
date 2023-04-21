#pragma once

#include "Actor.h"

class AFortSafeZoneIndicator : public AActor
{
public:
	float& GetSafeZoneStartShrinkTime()
	{
		static auto SafeZoneStartShrinkTimeOffset = GetOffset("SafeZoneStartShrinkTime");
		return Get<float>(SafeZoneStartShrinkTimeOffset);
	}

	float& GetSafeZoneFinishShrinkTime()
	{
		static auto SafeZoneFinishShrinkTimeOffset = GetOffset("SafeZoneFinishShrinkTime");
		return Get<float>(SafeZoneFinishShrinkTimeOffset);
	}

	void SkipShrinkSafeZone();
};