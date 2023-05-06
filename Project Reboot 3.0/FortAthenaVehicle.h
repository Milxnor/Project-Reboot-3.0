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

	int FindSeatIndex(class AFortPlayerPawn* PlayerPawn)
	{
		static auto FindSeatIndexFn = FindObject<UFunction>("/Script/FortniteGame.FortAthenaVehicle.FindSeatIndex");
		struct { AFortPlayerPawn* PlayerPawn; int ReturnValue; } AFortAthenaVehicle_FindSeatIndex_Params{ PlayerPawn };
		this->ProcessEvent(FindSeatIndexFn, &AFortAthenaVehicle_FindSeatIndex_Params);

		return AFortAthenaVehicle_FindSeatIndex_Params.ReturnValue;
	}

	UFortWeaponItemDefinition* GetVehicleWeaponForSeat(int SeatIdx);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaVehicle");
		return Class;
	}
};