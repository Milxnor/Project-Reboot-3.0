#pragma once

#include "Actor.h"
#include "GameplayAbilitySpec.h"
#include "Stack.h"

class AFortWeapon : public AActor
{
public:
	static inline void (*ServerReleaseWeaponAbilityOriginal)(UObject* Context, FFrame* Stack, void* Ret);
	static inline void (*OnPlayImpactFXOriginal)(AFortWeapon* Weapon, __int64 HitResult, uint8_t ImpactPhysicalSurface, UObject* SpawnedPSC);

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

	int& GetAmmoCount()
	{
		static auto AmmoCountOffset = GetOffset("AmmoCount");
		return Get<int>(AmmoCountOffset);
	}

	static void OnPlayImpactFXHook(AFortWeapon* Weapon, __int64 HitResult, uint8_t ImpactPhysicalSurface, UObject* SpawnedPSC);
	static void ServerReleaseWeaponAbilityHook(UObject* Context, FFrame* Stack, void* Ret);

	static UClass* StaticClass();
};