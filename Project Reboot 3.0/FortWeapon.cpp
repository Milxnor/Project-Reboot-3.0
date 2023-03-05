#include "FortWeapon.h"

#include "reboot.h"

UClass* AFortWeapon::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortWeapon");
	return Class;
}