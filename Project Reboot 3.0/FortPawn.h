#pragma once

#include "Pawn.h"
#include "FortWeapon.h"
#include "FortDecoItemDefinition.h"

class AFortPawn : public APawn
{
public:
	AFortWeapon* EquipWeaponDefinition(UFortWeaponItemDefinition* WeaponData, const FGuid& ItemEntryGuid);
	bool PickUpActor(AActor* PickupTarget, UFortDecoItemDefinition* PlacementDecoItemDefinition);

	AFortWeapon*& GetCurrentWeapon()
	{
		static auto CurrentWeaponOffset = GetOffset("CurrentWeapon");
		return Get<AFortWeapon*>(CurrentWeaponOffset);
	}

	static UClass* StaticClass();
};