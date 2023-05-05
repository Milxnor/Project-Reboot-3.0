#pragma once

#include "BuildingGameplayActor.h"

#include "FortPickup.h"
#include "Stack.h"

class AFortAthenaSupplyDrop : public ABuildingGameplayActor
{
public:
	static inline AFortPickup* (*SpawnPickupOriginal)(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static inline AFortPickup* (*SpawnGameModePickupOriginal)(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static inline AFortPickup* (*SpawnPickupFromItemEntryOriginal)(UObject* Context, FFrame& Stack, AFortPickup** Ret);

	FVector FindGroundLocationAt(FVector InLocation);

	static AFortPickup* SpawnPickupFromItemEntryHook(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static AFortPickup* SpawnGameModePickupHook(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static AFortPickup* SpawnPickupHook(UObject* Context, FFrame& Stack, AFortPickup** Ret);
};