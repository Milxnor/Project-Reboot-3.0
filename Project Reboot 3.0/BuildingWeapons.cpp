#include "BuildingWeapons.h"

#include "reboot.h"

void AFortWeap_EditingTool::OnRep_EditActor()
{
	static auto OnRep_EditActorFn = FindObject<UFunction>("/Script/FortniteGame.FortWeap_EditingTool.OnRep_EditActor");
	this->ProcessEvent(OnRep_EditActorFn);
}

UClass* AFortWeap_EditingTool::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortWeap_EditingTool");
	return Class;
}