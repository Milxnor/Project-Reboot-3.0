#include "FortGameModeZone.h"

#include "KismetStringLibrary.h"

#include "reboot.h"

UClass* AFortGameModeZone::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortGameModeZone");
	return Class;
}