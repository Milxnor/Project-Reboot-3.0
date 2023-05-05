#pragma once

#include "Actor.h"

enum class EFortSafeZoneState : uint8_t
{
	None = 0,
	Starting = 1,
	Holding = 2,
	Shrinking = 3,
	EFortSafeZoneState_MAX = 4
};

class AFortSafeZoneIndicator : public AActor
{
public:
	static inline void (*OnSafeZoneStateChangeOriginal)(AFortSafeZoneIndicator* SafeZoneIndicator, EFortSafeZoneState NewState, bool bInitial);

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

	static void OnSafeZoneStateChangeHook(AFortSafeZoneIndicator* SafeZoneIndicator, EFortSafeZoneState NewState, bool bInitial);
};