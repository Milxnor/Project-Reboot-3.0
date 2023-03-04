#pragma once

#include "Pawn.h"
#include "FortWeapon.h"
#include "FortWeaponItemDefinition.h"

class AFortPawn : public APawn
{
public:
	AFortWeapon* EquipWeaponDefinition(UFortWeaponItemDefinition* WeaponData, const FGuid& ItemEntryGuid);

	AFortWeapon*& GetCurrentWeapon()
	{
		static auto CurrentWeaponOffset = GetOffset("CurrentWeapon");
		return Get<AFortWeapon*>(CurrentWeaponOffset);
	}

	static UClass* StaticClass();
};