#include "FortInventoryInterface.h"

#include "reboot.h"

char UFortInventoryInterface::RemoveInventoryItemHook(__int64 a1, FGuid a2, int Count, char bForceRemoveFromQuickBars, char bForceRemoval)
{
	static auto FortPlayerControllerSuperSize = (*(UClass**)(__int64(FindObject<UClass>("/Script/FortniteGame.FortPlayerController")) + Offsets::SuperStruct))->GetPropertiesSize();
	auto Controller = *(UObject**)(__int64(a1) - (FortPlayerControllerSuperSize + 8));

	LOG_INFO(LogDev, "FortPlayerControllerSuperSize: {}", FortPlayerControllerSuperSize);
	LOG_INFO(LogDev, "Controller: {}", Controller->GetFullName());


}