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

	bool IsDBNO()
	{
		static auto bIsDBNOFieldMask = GetFieldMask(GetProperty("bIsDBNO"));
		static auto bIsDBNOOffset = GetOffset("bIsDBNO");

		return ReadBitfieldValue(bIsDBNOOffset, bIsDBNOFieldMask);
	}

	void SetHealth(float NewHealth);
	void SetShield(float NewShield);

	static UClass* StaticClass();
};