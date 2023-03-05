#pragma once

#include "Actor.h"

enum class EFortPickupSourceTypeFlag : uint8_t
{
	Other = 0,
	Player = 1,
	Destruction = 2,
	Container = 3,
	AI = 4,
	Tossed = 5,
	FloorLoot = 6,
	EFortPickupSourceTypeFlag_MAX = 7
};

enum class EFortPickupSpawnSource : uint8_t
{
	Unset = 0,
	PlayerElimination = 1,
	Chest = 2,
	SupplyDrop = 3,
	AmmoBox = 4,
	Drone = 5,
	ItemSpawner = 6,
	EFortPickupSpawnSource_MAX = 7
};

ENUM_CLASS_FLAGS(EFortPickupSourceTypeFlag)

class AFortPickup : public AActor
{
public:
	void TossPickup(FVector FinalLocation, class AFortPawn* ItemOwner, int OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag InPickupSourceTypeFlags, EFortPickupSpawnSource InPickupSpawnSource);

	static AFortPickup* SpawnPickup(class UFortItemDefinition* ItemDef, FVector Location, int Count, 
		EFortPickupSourceTypeFlag PickupSource = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset, 
		int LoadedAmmo = -1, class AFortPawn* Pawn = nullptr);
};