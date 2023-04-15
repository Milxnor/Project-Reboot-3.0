#pragma once

#include "Actor.h"

class AFortWeaponRangedMountedCannon : public AActor // : public AFortWeaponRangedForVehicle
{
public:
	bool FireActorInCannon(FVector LaunchDir, bool bIsServer = true);
	static void ServerFireActorInCannonHook(AFortWeaponRangedMountedCannon* Cannon, FVector LaunchDir);
};