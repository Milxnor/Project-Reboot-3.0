#pragma once

#include "FortAthenaVehicle.h"

#include "reboot.h"

class AFortMountedCannon : public AFortAthenaVehicle // : public AFortMountedTurret
{
public:

	void OnLaunchPawn(AFortPlayerPawn* Pawn)
	{
		static auto OnLaunchPawnFn = FindObject<UFunction>("/Script/FortniteGame.FortMountedCannon.OnLaunchPawn");
		this->ProcessEvent(OnLaunchPawnFn, &Pawn);
	}

	void ShootPawnOut()
	{
		auto PawnToShoot = this->GetPawnAtSeat(0);

		LOG_INFO(LogDev, "PawnToShoot: {}", __int64(PawnToShoot));

		if (!PawnToShoot)
			return;

		PawnToShoot->ServerOnExitVehicle(ETryExitVehicleBehavior::ForceAlways);
		this->OnLaunchPawn(PawnToShoot);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortMountedCannon");
		return Class;
	}
};