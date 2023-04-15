#include "FortWeaponRangedMountedCannon.h"
#include "FortAthenaVehicle.h"
#include "FortPlayerPawnAthena.h"
#include "FortAthenaSKPushCannon.h"
#include "FortMountedCannon.h"

bool AFortWeaponRangedMountedCannon::FireActorInCannon(FVector LaunchDir, bool bIsServer)
{
    static auto InstigatorOffset = GetOffset("Instigator");
    auto Pawn = Cast<AFortPlayerPawn>(this->Get(InstigatorOffset));

    LOG_INFO(LogDev, "Pawn: {}", __int64(Pawn));
    LOG_INFO(LogDev, "LaunchDir.X: {} LaunchDir.Y: {} LaunchDir.Z: {}", LaunchDir.X, LaunchDir.Y, LaunchDir.Z);

    if (!Pawn)
        return false;

    auto Vehicle = Pawn->GetVehicle();

    LOG_INFO(LogDev, "Vehicle: {}", __int64(Vehicle));

    if (!Vehicle)
        return false;

    auto PushCannon = Cast<AFortAthenaSKPushCannon>(Vehicle);

    LOG_INFO(LogDev, "PushCannon: {}", __int64(PushCannon));

    if (!PushCannon)
    {
        auto MountedCannon = Cast<AFortMountedCannon>(Vehicle);

        LOG_INFO(LogDev, "MountedCannon: {}", __int64(MountedCannon));

        if (MountedCannon)
        {
            if (bIsServer 
                && this->HasAuthority()
                // theres another Pawn check with their vehicle here, not sure what it is.
                )
            {
                MountedCannon->ShootPawnOut();
            }
        }

        return false;
    }

    if (!Vehicle->GetPawnAtSeat(1))
        return false;

    if (bIsServer)
    {
        if (this->HasAuthority())
            PushCannon->ShootPawnOut(LaunchDir);
    }

    return true;
}

void AFortWeaponRangedMountedCannon::ServerFireActorInCannonHook(AFortWeaponRangedMountedCannon* Cannon, FVector LaunchDir)
{
    Cannon->FireActorInCannon(LaunchDir, true);
    return;
}