#pragma once

#include "Actor.h"

class AFortWeapon : public AActor
{
public:
	template <typename T = class UFortWeaponItemDefinition>
	T* GetWeaponData()
	{
		static auto WeaponDataOffset = GetOffset("WeaponData");
		return Get<T*>(WeaponDataOffset);
	}

	FGuid& GetItemEntryGuid()
	{
		static auto ItemEntryGuidOffset = GetOffset("ItemEntryGuid");
		return Get<FGuid>(ItemEntryGuidOffset);
	}

	static UClass* StaticClass();
};