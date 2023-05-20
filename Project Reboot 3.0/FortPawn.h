#pragma once

#include "Pawn.h"
#include "FortWeapon.h"
#include "FortDecoItemDefinition.h"

class AFortPawn : public APawn
{
public:
	static inline void (*NetMulticast_Athena_BatchedDamageCuesOriginal)(UObject* Context, FFrame* Stack, void* Ret);
	static inline void (*MovingEmoteStoppedOriginal)(UObject* Context, FFrame* Stack, void* Ret);

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

	void SetDBNO(bool IsDBNO)
	{
		static auto bIsDBNOFieldMask = GetFieldMask(GetProperty("bIsDBNO"));
		static auto bIsDBNOOffset = GetOffset("bIsDBNO");

		this->SetBitfieldValue(bIsDBNOOffset, bIsDBNOFieldMask, IsDBNO);
	}

	void SetHasPlayedDying(bool NewValue)
	{
		static auto bPlayedDyingFieldMask = GetFieldMask(GetProperty("bPlayedDying"));
		static auto bPlayedDyingOffset = GetOffset("bPlayedDying");

		this->SetBitfieldValue(bPlayedDyingOffset, bPlayedDyingFieldMask, NewValue);
	}
	
	void OnRep_IsDBNO();
	float GetShield();
	float GetHealth();
	void SetHealth(float NewHealth);
	void SetMaxHealth(float NewHealthVal);
	void SetShield(float NewShield);
	static void NetMulticast_Athena_BatchedDamageCuesHook(UObject* Context, FFrame* Stack, void* Ret);
	static void MovingEmoteStoppedHook(UObject* Context, FFrame* Stack, void* Ret);

	static UClass* StaticClass();
};