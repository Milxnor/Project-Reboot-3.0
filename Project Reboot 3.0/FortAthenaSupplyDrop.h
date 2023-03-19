#pragma once

#include "BuildingGameplayActor.h"

#include "FortPickup.h"
#include "Stack.h"

class AFortAthenaSupplyDrop : public ABuildingGameplayActor
{
public:
	static inline AFortPickup* (*SpawnPickupOriginal)(UObject* Context, FFrame& Stack, AFortPickup** Ret);

	static AFortPickup* SpawnPickupHook(UObject* Context, FFrame& Stack, AFortPickup** Ret); // Unfortuanetly this function will never get called since we dont hook picklootdrops
};