#include "FortOctopusVehicle.h"

#include "reboot.h"

void AFortOctopusVehicle::ServerUpdateTowhookHook(AFortOctopusVehicle* OctopusVehicle, FVector InNetTowhookAimDir)
{
	static auto NetTowhookAimDirOffset = OctopusVehicle->GetOffset("NetTowhookAimDir");
	OctopusVehicle->Get<FVector>(NetTowhookAimDirOffset) = InNetTowhookAimDir;

	static auto OnRep_NetTowhookAimDirFn = FindObject<UFunction>("/Script/FortniteGame.FortOctopusVehicle.OnRep_NetTowhookAimDir");
	OctopusVehicle->ProcessEvent(OnRep_NetTowhookAimDirFn);
}