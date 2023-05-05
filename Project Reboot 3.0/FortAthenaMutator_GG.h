// Gun Game

#pragma once

#include "Actor.h"
#include "CurveTable.h"
#include "GameplayAbilityTypes.h"
#include "FortWeaponItemDefinition.h"
#include "Stack.h"
#include "FortPlayerStateAthena.h"
#include "FortAthenaMutator.h"

struct FGunGameGunEntry
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.GunGameGunEntry");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	UFortWeaponItemDefinition*& GetWeapon()
	{
		static auto WeaponOffset = FindOffsetStruct("/Script/FortniteGame.GunGameGunEntry", "Weapon");
		return *(UFortWeaponItemDefinition**)(__int64(this) + WeaponOffset);
	}

	FScalableFloat& GetEnabled()
	{
		static auto EnabledOffset = FindOffsetStruct("/Script/FortniteGame.GunGameGunEntry", "Enabled");
		return *(FScalableFloat*)(__int64(this) + EnabledOffset);
	}

	FScalableFloat& GetAwardAtElim()
	{
		static auto AwardAtElimOffset = FindOffsetStruct("/Script/FortniteGame.GunGameGunEntry", "AwardAtElim");
		return *(FScalableFloat*)(__int64(this) + AwardAtElimOffset);
	}
};

struct FGunGameGunEntries
{
	TArray<FGunGameGunEntry>                    Entries;                                                  // 0x0000(0x0010) (ZeroConstructor, NativeAccessSpecifierPublic)
};

struct FGunGamePlayerData
{
	TArray<UFortWeaponItemDefinition*>           CurrentlyAssignedWeapons;                                 // 0x0000(0x0010) (ZeroConstructor, NativeAccessSpecifierPublic)
};

class AFortAthenaMutator_GG : public AFortAthenaMutator
{
public:
	TArray<FGunGameGunEntry>& GetWeaponEntries()
	{
		static auto WeaponEntriesOffset = GetOffset("WeaponEntries");
		return Get<TArray<FGunGameGunEntry>>(WeaponEntriesOffset);
	}

	TMap<int, FGunGameGunEntries>& GetAwardEntriesAtElimMap()
	{
		static auto AwardEntriesAtElimMapOffset = GetOffset("AwardEntriesAtElimMap");
		return Get<TMap<int, FGunGameGunEntries>>(AwardEntriesAtElimMapOffset);
	}

	TMap<AFortPlayerStateAthena*, FGunGamePlayerData>& GetPlayerData()
	{
		static auto PlayerDataOffset = GetOffset("PlayerData");
		return Get<TMap<AFortPlayerStateAthena*, FGunGamePlayerData>>(PlayerDataOffset);
	}

	FGunGameGunEntries GetEntriesFromAward(const FScalableFloat& AwardAtElim)
	{
		auto& AwardEntriesAtElimMap = GetAwardEntriesAtElimMap();

		float Value = 0; // TODO Get from AwardAtElim

		for (auto& AwardEntry : AwardEntriesAtElimMap)
		{
			if (AwardEntry.First == Value)
			{
				return AwardEntry.Second;
			}
		}
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaMutator_GG");
		return Class;
	}
};