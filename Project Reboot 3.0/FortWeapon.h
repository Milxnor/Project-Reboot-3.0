#pragma once

#include "Actor.h"
#include "GameplayAbilitySpec.h"
#include "Stack.h"

class AFortWeapon : public AActor
{
public:
	static inline void (*ServerReleaseWeaponAbilityOriginal)(UObject* Context, FFrame* Stack, void* Ret);

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

	static void ServerReleaseWeaponAbilityHook(UObject* Context, FFrame* Stack, void* Ret);

	static UClass* StaticClass();
};