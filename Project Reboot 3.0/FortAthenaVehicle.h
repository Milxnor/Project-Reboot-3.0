#pragma once

#include "Actor.h"

#include "reboot.h"
#include "FortWeaponItemDefinition.h"

class AFortAthenaVehicle : public AActor// : public AFortPhysicsPawn // Super changes based on version
{
public:
	class AFortPlayerPawn* GetPawnAtSeat(int SeatIdx)
	{
		static auto GetPawnAtSeatFn = FindObject<UFunction>("/Script/FortniteGame.FortAthenaVehicle.GetPawnAtSeat");
		struct { int SeatIdx; class AFortPlayerPawn* ReturnValue; } GetPawnAtSeat_Params{SeatIdx};
		this->ProcessEvent(GetPawnAtSeatFn, &GetPawnAtSeat_Params);

		return GetPawnAtSeat_Params.ReturnValue;
	}

	UFortWeaponItemDefinition* GetVehicleWeaponForSeat(int SeatIdx);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaVehicle");
		return Class;
	}
};