#include "BuildingWeapons.h"

#include "reboot.h"

UClass* AFortWeap_EditingTool::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/FortniteGame.FortWeap_EditingTool");
	return Class;
}