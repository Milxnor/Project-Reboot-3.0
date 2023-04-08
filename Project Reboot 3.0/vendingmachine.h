#pragma once

#include "reboot.h"
#include "BuildingGameplayActor.h"
#include "GameplayStatics.h"

using ABuildingItemCollectorActor = ABuildingGameplayActor;

void FillVendingMachine(ABuildingItemCollectorActor* VendingMachine)
{

}

void FillVendingMachines()
{
	static auto VendingMachineClass = FindObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");
	auto AllVendingMachines = UGameplayStatics::GetAllActorsOfClass(GetWorld(), VendingMachineClass);

	for (int i = 0; i < AllVendingMachines.Num(); i++)
	{
		auto VendingMachine = (ABuildingItemCollectorActor*)AllVendingMachines.at(i);

		if (!VendingMachine)
			continue;

		FillVendingMachine(VendingMachine);
	}

	AllVendingMachines.Free();
}