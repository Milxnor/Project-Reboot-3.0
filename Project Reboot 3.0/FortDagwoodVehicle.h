#pragma once

#include "FortAthenaVehicle.h"

class AFortDagwoodVehicle : public AFortAthenaVehicle // AFortAthenaSKMotorVehicle
{
public:
	void SetFuel(float NewFuel)
	{
		static auto SetFuelFn = FindObject<UFunction>(L"/Script/ValetRuntime.FortDagwoodVehicle.SetFuel");
		this->ProcessEvent(SetFuelFn, &NewFuel);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/ValetRuntime.FortDagwoodVehicle");
		return Class;
	}
};